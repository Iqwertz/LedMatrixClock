///////////////////////Include Libs////////////////////////
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "DS3231.h"

RTClib RTC;

///////////////////////Display Settings///////////////////
const int Hohe = 16;  //Height (in px)
const int Breite = 16;  // Width (in px)
const bool invert = true; //Set depending on the wiring
const bool rightstart = true;  //Check when the start is on the right side
const int Display = 6; //Pin of the display
const int ldrPin = A1; //Pin for the ldr Sensor
byte Rotate = 0; //Rotates Display / 0  = 0° / 1 = 90° / 2=180° / 3 = 270°
const int MaxLight = 700;  //Ldr Value when enviroment Light is maximum
const int MinLight = 350;  //Ldr Value when Light is off
const byte MaxBrightness = 100;  //Max Brightness of the Display
byte Mode = 1; // 0 = Clock / 1 = Numbers / 2 = off

/////////////////Audio Settings///////////////////////
const int ThresholdDiffrence = 10;  //Diffrence from the standard to count as spike
const int ClappingDelay = 100;     //Min Delay between Spikes
const int ClappingIntervall = 2000;    //Max time between Clapping
const int MeasuringIntervall = 10;  //ms between measurments

//////////////////Settings for the Clock Mode////////////////
const byte ClockOutlineColor[3] = {77, 255, 160};   
const byte QuarterOutlineLength = 11;
const byte QuarterOutline[QuarterOutlineLength][2] = {{1, 8}, {2, 8}, {3, 7}, {4, 7}, {5, 7}, {6, 6}, {7, 5}, {7, 4}, {7, 3}, {8, 2}, {8, 1}};

const byte HourColor[3] = {200, 280, 16};
const byte MinuteColor[3] = {77, 255, 2550};
const byte SecondColor[3] = {0, 255, 0};

//////////////////Settings for the Number Mode////////////////
const byte NumbersSecondColor[3] = {0, 200, 100};
const byte NumbersMinuteColor[3] = {255, 255, 255};
const byte NumbersHourColor[3] = {255, 255, 255};

///////////////Numbers Font Storage////////////
const byte N0 [16][2] = {{3, 7}, {4, 7}, {5, 7}, {6, 6}, {6, 5}, {2, 4}, {6, 4}, {2, 3}, {2, 2}, {3, 1}, {4, 1}, {5, 1}, {6, 2}, {6, 3}, {2, 5}, {2, 6}};
const byte N1 [7][2]  = {{4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7}};
const byte N2 [14][2] = {{2, 6}, {3, 7}, {4, 7}, {5, 7}, {6, 6}, {6, 5}, {5, 4}, {4, 3}, {3, 2}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}};
const byte N3 [14][2] = {{2, 6}, {3, 7}, {4, 7}, {5, 7}, {6, 6}, {6, 5}, {5, 4}, {4, 4}, {6, 3}, {6, 2}, {5, 1}, {4, 1}, {3, 1}, {2, 2}};
const byte N4 [11][2] = {{5, 7}, {4, 6}, {3, 5}, {2, 4}, {2, 3}, {3, 3}, {4, 3}, {5, 4}, {5, 3}, {5, 2}, {5, 1}};
const byte N5 [17][2] = {{6, 7}, {5, 7}, {4, 7}, {3, 7}, {2, 7}, {2, 6}, {2, 5}, {2, 4}, {3, 4}, {4, 4}, {5, 4}, {6, 3}, {6, 2}, {5, 1}, {4, 1}, {3, 1}, {2, 2}};
const byte N6 [14][2] = {{5, 7}, {4, 6}, {3, 5}, {2, 4}, {2, 3}, {2, 2}, {3, 1}, {4, 1}, {5, 1}, {6, 2}, {6, 3}, {5, 4}, {4, 4}, {3, 4}};
const byte N7 [11][2] = {{6, 7}, {5, 7}, {4, 7}, {3, 7}, {2, 7}, {6, 6}, {5, 5}, {4, 4}, {4, 3}, {3, 2}, {3, 1}};
const byte N8 [17][2] = {{3, 7}, {4, 7}, {5, 7}, {6, 6}, {6, 5}, {5, 4}, {4, 4}, {3, 4}, {2, 3}, {2, 2}, {3, 1}, {4, 1}, {5, 1}, {6, 2}, {6, 3}, {2, 5}, {2, 6}};
const byte N9 [14][2] = {{3, 7}, {4, 7}, {5, 7}, {6, 6}, {6, 5}, {6, 4}, {5, 4}, {4, 4}, {3, 4}, {5, 3}, {4, 2}, {3, 1}, {2, 5}, {2, 6}};

//////////////////System Vars/////////////////////////
byte Seconds = 0;
long Milliseconds = 0;
int LastSecond = 0;
byte LastMinute = 0;
long LastMilliseconds = 0;
bool Status = true;

/////Audio Vars//////
int Spikes = 0;
int AverageSound[2] = {0, 0};
long FirstSpikeMillis = 0;
long LastSound = 0;

////////Ini Pixel
#define NUMPIXELS Hohe*Breite
Adafruit_NeoPixel Matrix(NUMPIXELS, Display, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  /////Matrix Setup//////
  Matrix.begin();
  Matrix.setBrightness(MaxBrightness);
  Matrix.clear();
  Matrix.show();

  LastMilliseconds = millis();
  LastSound = millis();

  GetAverage(30);   //Get Average Value of the sound
}

void loop() {
  SetBrightness();

  if (Status) {
    DateTime now = RTC.now();  //Get Time
    Sound();   //Check Sound
    if (Mode == 0) {
      if (now.second() - LastSecond >= 1) {  //Check if Second changed
        Rotate = 2; 
        Milliseconds = 0;
        LastSecond = now.second();

        if (LastSecond == 60) {
          LastSecond = 0;
        } else if (LastSecond == 59) {
          LastSecond = -1;
        }
      }

      if (millis() - LastMilliseconds >= 100) {   //Can be improved (removed Feature)
        Matrix.clear();
        Milliseconds += 100;
        LastMilliseconds = millis();
        DrawSecond(now.second(), Milliseconds);


        DrawMinute(now.minute());
        DrawHour(now.hour(), now.minute());
        DrawOutline();

        DrawPixel(0, 0, true, 255, 25, 16);
        DrawPixel(1, 0, true, 255, 25, 16);
        DrawPixel(1, 1, true, 255, 25, 16);
        DrawPixel(0, 1, true, 255, 25, 16);
        Matrix.show();
      }
    } else if (Mode == 1) {
      if (millis() - LastMilliseconds >= 100) {
        Rotate = 3;
        Matrix.clear();
        LastMilliseconds = millis();
        Milliseconds += 100;
        if (Milliseconds >= 60000) {
          Milliseconds = 0;
        }
        if (LastMinute != now.minute()) {
          LastMinute = now.minute();
          Milliseconds = 0;
        }
        DrawRectSecond(Milliseconds);
        DrawNumbers(floor(now.hour() / 10), now.hour() % 10, floor(now.minute() / 10), now.minute() % 10);
        Matrix.show();
      }
    }
  }
}

void Sound() {   //Check for Sound & Detect Double Clap
  if (millis() - LastSound >= MeasuringIntervall) {
    LastSound = millis();
    Serial.print(analogRead(A2));
    Serial.print(",");
    Serial.print(AverageSound[0]);
    Serial.print(",");
    Serial.println(AverageSound[1]);
    int ReadValue = analogRead(A2);
    if (millis() - FirstSpikeMillis >= ClappingDelay) {
      if ( ReadValue - AverageSound[1] >= ThresholdDiffrence || AverageSound[0] - ReadValue >= ThresholdDiffrence) {
        Spikes++;
      }
    }
    if (Spikes == 1) {
      FirstSpikeMillis = millis();
    }

    if ( millis() - FirstSpikeMillis >= ClappingIntervall  && Spikes >= 1) {
      if (Spikes >= 2 && Spikes <= 2) {
        if (Spikes >= 1 && Spikes <= 4) {
          Mode++;
          Serial.println(0);
          if (Mode == 3) {
            Mode = 0;
          } else if (Mode == 2) {
            Matrix.clear();
            Matrix.show();
          }
        }
      }
      //Serial.println(Spikes);
      Spikes = 0;
    }
  }
}

void DrawRectSecond(long ms) {  //Draw Rectangles with fade depending on ms
  ms = round(ms * 1.33334); //Convert ms (0 - 60000) to a range of 0 to  80000
  int s = floor(ms / 10000);  //Get Seconds)
  for (int i = 1; i <= s; i++) {
    DrawQuad(i, NumbersSecondColor[0], NumbersSecondColor[1], NumbersSecondColor[2]);  //Draw Quad for every Second
  }
  int Rest = ms % 10000; //Get ms for outer fading Square
  DrawQuad(s + 1, AdjustBrightness(NumbersSecondColor[0], Rest, 10000), AdjustBrightness(NumbersSecondColor[1], Rest, 10000), AdjustBrightness(NumbersSecondColor[2], Rest, 10000));   //Draw Outest QUad with a fade depending on the brightness
}

void DrawQuad(byte dist, byte r, byte g, byte b) {  //Draw a Quadrat around the Center with a given Distance
  int px, py;
  px = py = dist;
  for (int i = 0; i < dist * 2; i++) {
    DrawPixel(px - i, py, true, r, g, b);
    DrawPixel(px, py - i, true, r, g, b);
    DrawPixel(px * -1 + i + 1, py * -1 + 1, true, r, g, b);
    DrawPixel(px * -1 + 1, py * -1 + i + 1, true, r, g, b);
  }
}

void DrawNumbers(int z1, int z2, int z3, int z4) {  //Draws 4 numbers to the display
  SelectNumber(z1, 0);
  SelectNumber(z2, 1);
  SelectNumber(z3, 2);
  SelectNumber(z4, 3);
}

void SelectNumber(int n, byte mode) { //Selcts the Array var corresponding number and Sets it
  switch (n) {
    case 0:
      SetNumber(N0, sizeof(N0) / sizeof(N0[0]), mode);
      break;
    case 1:
      SetNumber(N1, sizeof(N1) / sizeof(N1[0]), mode);
      break;
    case 2:
      SetNumber(N2, sizeof(N2) / sizeof(N2[0]), mode);
      break;
    case 3:
      SetNumber(N3, sizeof(N3) / sizeof(N3[0]), mode);
      break;
    case 4:
      SetNumber(N4, sizeof(N4) / sizeof(N4[0]), mode);
      break;
    case 5:
      SetNumber(N5, sizeof(N5) / sizeof(N5[0]), mode);
      break;
    case 6:
      SetNumber(N6, sizeof(N6) / sizeof(N6[0]), mode);
      break;
    case 7:
      SetNumber(N7, sizeof(N7) / sizeof(N7[0]), mode);
      break;
    case 8:
      SetNumber(N8, sizeof(N8) / sizeof(N8[0]), mode);
      break;
    case 9:
      SetNumber(N9, sizeof(N9) / sizeof(N9[0]), mode);
      break;
  }
}

void SetNumber(byte arr[][2], byte Size, byte M) {  //Draws a number to the display / arr = array containig the displayed data / Size = the size of the passed array / M = Quater of the Matrix to draw the number on (0 = top left, 1 = top right, 2 = bottom left, 3 = bottom right)
  for (int i = 0; i < Size; i++) {
    switch (M) {
      case 0:
        DrawPixel(arr[i][0] - Breite / 2 + 1, arr[i][1] + 1, true, NumbersHourColor[0], NumbersHourColor[1], NumbersHourColor[2]);
        break;
      case 1:
        DrawPixel(arr[i][0], arr[i][1] + 1, true, NumbersHourColor[0], NumbersHourColor[1], NumbersHourColor[2]);
        break;
      case 2:
        DrawPixel(arr[i][0] - Breite / 2 + 1, arr[i][1] - Hohe / 2, true, NumbersMinuteColor[0], NumbersMinuteColor[1], NumbersMinuteColor[2]);
        break;
      case 3:
        DrawPixel(arr[i][0], arr[i][1] - Hohe / 2, true, NumbersMinuteColor[0], NumbersMinuteColor[1], NumbersMinuteColor[2]);
        break;

    }

  }
}

void SetBrightness() {   //Set the Brightness of the Display depending on the ldr readings / if the value drops below a definde value the display is turned off
  int ldrStatus = analogRead(ldrPin);
  if (ldrStatus <= MinLight) {
    Matrix.clear();
    Matrix.show();
    Status = false;
    Mode = 1;
  } else {
    Status = true;
    Matrix.setBrightness(map(ldrStatus, MinLight, MaxLight, 0, MaxBrightness));
  }
}

void DrawOutline() {  //Draws the Outline of the Clock
  for (int i = 0; i < QuarterOutlineLength; i++) {
    Matrix.setPixelColor(MatrixConvert(QuarterOutline[i][0], QuarterOutline[i][1], true), Matrix.Color(ClockOutlineColor[0], ClockOutlineColor[1], ClockOutlineColor[2]));
    Matrix.setPixelColor(MatrixConvert(QuarterOutline[i][0] * -1 + 1, QuarterOutline[i][1], true), Matrix.Color(ClockOutlineColor[0], ClockOutlineColor[1], ClockOutlineColor[2]));
    Matrix.setPixelColor(MatrixConvert(QuarterOutline[i][0], QuarterOutline[i][1] * -1 + 1, true), Matrix.Color(ClockOutlineColor[0], ClockOutlineColor[1], ClockOutlineColor[2]));
    Matrix.setPixelColor(MatrixConvert(QuarterOutline[i][0] * -1 + 1, QuarterOutline[i][1] * -1 + 1, true), Matrix.Color(ClockOutlineColor[0], ClockOutlineColor[1], ClockOutlineColor[2]));
  }
}

void DrawSecond(byte s, int ms) { //Draws the Second finger
  /* s++;
    DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, AdjustBrightness(SecondColor[0], ms), AdjustBrightness(SecondColor[1], ms), AdjustBrightness(SecondColor[2], ms));
    s++;
    DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, AdjustBrightness(SecondColor[0], ms), AdjustBrightness(SecondColor[1], ms), AdjustBrightness(SecondColor[2], ms));
    s -= 2;
    DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, AdjustBrightness(SecondColor[0], 2000 - ms), AdjustBrightness(SecondColor[1], 2000 - ms), AdjustBrightness(SecondColor[2], 2000 - ms));
  */

  DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, SecondColor[0], SecondColor[1], SecondColor[2]);
}

void DrawMinute(byte m) {  //Draws the minute finger
  DrawFinger(map(m * 6, 0, 360, 360, 0), Breite / 2 - 3, MinuteColor[0], MinuteColor[1], MinuteColor[2]);
}

void DrawHour(int h, int m) {  //Draws the Hour finger (Converted to minutes for smaller Steps)
  if (h > 12) {
    h -= 12;
  }
  h = h * 60;
  h = h + m;
  h = round(h * 0.5);
  DrawFinger(map(h, 0, 360, 360, 0), Breite / 2 - 4, HourColor[0], HourColor[1], HourColor[2]);
}

byte AdjustBrightness(byte v, int brightness, float MaxB) {  //Adjust the Brightness of an rgb value
  return round(v * (brightness / MaxB));
}
void DrawFinger(int angle, int rad, byte r, byte g, byte b) { //Draws a finger at an given Angle
  if (angle < 6) {
    DrawLine(1, 1, rad + 1, 1, r, g, b);
  } else if (angle == 90) {
    DrawLine(0, 0, 0, rad + 1, r, g, b);
  } else if (angle == 180) {
    DrawLine(1, 1, rad * -1, 1, r, g, b);
  } else if (angle == 270) {
    DrawLine(0, 0, 0, rad * -1, r, g, b);
  }

  int MiddleX = 0;
  int MiddleY = 0;

  MiddleY = (angle > 0 && angle < 180) ? 1 : 0;
  MiddleX = (angle > 90 && angle < 270) ? 0 : 1;

  DrawLine(MiddleX, MiddleY, round(rad * cos(radians(angle))) + MiddleX, round(rad * sin(radians(angle))) + MiddleY, r, g, b);
}

void DrawLine(int x0, int y0, int x1, int y1, byte r, byte g, byte b) //Draws a line with the bresenham algorithmus
{
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;
  while (true) {
    DrawPixel(x0, y0, true, r, g, b);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0  += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0  += sy;
    }
  }
}

void DrawPixel(int px, int py, bool c, byte r, byte g, byte b) {  //Draws a pixel on the matrix
  Matrix.setPixelColor(MatrixConvert(px, py, c), Matrix.Color(r, g, b));
}

int MatrixConvert(int x, int y, bool center) {  //Converts an x,y Cordinate to the Position on the Led String / When Center is activated the cordinate origib is in the center
  int nummer = 0;

  if (center) {
    x += round(Breite / 2);
    y += round(Hohe / 2);
  }

  if (Rotate == 1) {
    int a = x;
    x = Breite - y + 1;
    y = a;

    a = x;
    x = Breite - y + 1;
    y = a;

    a = x;
    x = Breite - y + 1;
    y = a;
  } else if (Rotate == 2) {
    int a = x;
    x = Breite - y + 1;
    y = a;

    a = x;
    x = Breite - y + 1;
    y = a;
  } else if (Rotate == 3) {
    int a = x;
    x = Breite - y + 1;
    y = a;
  }

  if (x <= Breite && x > 0 && y <= Hohe && y > 0) {
    if (invert) {
      if (rightstart) {
        if (x % 2 != 0) {
          nummer = 16 * (x - 1) + y;
        } else {
          nummer = 16 * x - y + 1;
        }
      } else {
        if (y % 2 == 0) {
          nummer = 16 * (x - 1) + y;
        } else {
          nummer = 16 * x - y + 1;
        }
      }
    } else {
      nummer = 16 * (y - 1) + x;
    }
  } else {
    Error("Der Wert darf nicht höher als die max Höhe und Breite sein");
  } return nummer - 1;
}

void GetAverage(int Samples) {  //Gets min and max Average of surrounding Sound
  int MinSample;
  int MaxSample;
  int AVal = analogRead(A2);
  MinSample = MaxSample = AVal;

  for (int i = 0; i < Samples; i++) {
    AVal = analogRead(A2);
    if (AVal > MaxSample) {
      MaxSample = AVal;
    }
    if (AVal < MinSample) {
      MinSample = AVal;
    }
    delay(10);
  }
  AverageSound[0] = MinSample;
  AverageSound[1] = MaxSample;
}

void Error(String Code) {  //Prints Errors
  Serial.println(Code);
}
