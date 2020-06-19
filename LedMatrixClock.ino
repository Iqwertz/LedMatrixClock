#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "DS3231.h"

RTClib RTC;

const int Hohe = 16;
const int Breite = 16;
const bool invert = true;
const bool rightstart = true;
const int Display = 6;
const int ldrPin = A1;
const byte Rotate = 3; //Rotates Display / 0  = 0° / 1 = 90° / 2=180° / 3 = 270°
const int MaxLight = 700;
const int MinLight = 350;
const byte MaxBrightness = 155;

const byte ClockOutlineColor[3] = {77, 255, 160};
const byte QuarterOutlineLength = 11;
const byte QuarterOutline[QuarterOutlineLength][2] = {{1, 8}, {2, 8}, {3, 7}, {4, 7}, {5, 7}, {6, 6}, {7, 5}, {7, 4}, {7, 3}, {8, 2}, {8, 1}};

const byte HourColor[3] = {200, 280, 16};
const byte MinuteColor[3] = {77, 255, 2550};
const byte SecondColor[3] = {0, 255, 0};

const byte N1 [7][2]={{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7}};
const byte N2 [14][2]={{2,6},{3,7},{4,7},{5,7},{6,6},{6,5},{5,4},{4,3},{3,2},{2,1},{3,1},{4,1},{5,1},{6,1}};
const byte N3 [14][2]={{2,6},{3,7},{4,7},{5,7},{6,6},{6,5},{5,4},{4,4},{6,3},{6,2},{5,1},{4,1},{3,1},{2,2}};
const byte N4 [11][2]={{5,7},{4,6},{3,5},{2,4},{2,3},{3,3},{4,3},{5,4},{5,3},{5,2},{5,1}};
const byte N5 [17][2]={{6,7},{5,7},{4,7},{3,7},{2,7},{2,6},{2,5},{2,4},{3,4},{4,4},{5,4},{6,3},{6,2},{5,1},{4,1},{3,1},{2,2}};
const byte N6 [14][2]={{5,7},{4,6},{3,5},{2,4},{2,3},{2,2},{3,1},{4,1},{5,1},{6,2},{6,3},{5,4},{4,4},{3,4}};
const byte N7 [11][2]={{6,7},{5,7},{4,7},{3,7},{2,7},{6,6},{5,5},{4,4},{4,3},{3,2},{3,1}};
const byte N8 [17][2]={{3,7},{4,7},{5,7},{6,6},{6,5},{5,4},{4,4},{3,4},{2,3},{2,2},{3,1},{4,1},{5,1},{6,2},{6,3},{2,5},{2,6}};
const byte N8 [17][2]={{3,7},{4,7},{5,7},{6,6},{6,5},{5,4},{4,4},{3,4},{2,3},{2,2},{3,1},{4,1},{5,1},{6,2},{6,3},{2,5},{2,6}};

byte Seconds = 0;
int Milliseconds = 0;
int LastSecond = 0;
long LastMilliseconds = 0;
bool Status = true;
byte Mode = 1; // 0 = Clock / 1 = Numbers / 2 = off

#define NUMPIXELS Hohe*Breite
Adafruit_NeoPixel Matrix(NUMPIXELS, Display, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  Wire.begin();

  Matrix.begin();
  Matrix.setBrightness(MaxBrightness);
  Matrix.clear();
  Matrix.show();

  DrawOutline();

  LastMilliseconds = millis();
}

void loop() {
  SetBrightness();

  if (Status) {
    DateTime now = RTC.now();

    if (Mode == 0) {
      if (now.second() - LastSecond >= 1) {

        Milliseconds = 0;
        LastSecond = now.second();

        if (LastSecond == 60) {
          LastSecond = 0;
        } else if (LastSecond == 59) {
          LastSecond = -1;
        }
      }

      if (millis() - LastMilliseconds >= 100) {
        Matrix.clear();

        Milliseconds += 100;
        LastMilliseconds = millis();
        DrawSecond(now.second(), Milliseconds);


        DrawMinute(now.minute());
        DrawHour(now.hour());
        DrawOutline();

        DrawPixel(0, 0, true, 255, 25, 16);
        DrawPixel(1, 0, true, 255, 25, 16);
        DrawPixel(1, 1, true, 255, 25, 16);
        DrawPixel(0, 1, true, 255, 25, 16);
        Matrix.show();
      }
    }
  } else if (Mode == 1) {

  }
}

void DrawNumbers(int z1, int z2, int z3, int z4){
  
}

void SetBrightness() {
  int ldrStatus = analogRead(ldrPin);
  if (ldrStatus <= MinLight) {
    Matrix.clear();
    Matrix.show();
    Status = false;
  } else {
    Status = true;
    Matrix.setBrightness(map(ldrStatus, MinLight, MaxLight, 0, MaxBrightness));
  }
}

void DrawOutline() {
  for (int i = 0; i < QuarterOutlineLength; i++) {
    Matrix.setPixelColor(MatrixConvert(QuarterOutline[i][0], QuarterOutline[i][1], true), Matrix.Color(ClockOutlineColor[0], ClockOutlineColor[1], ClockOutlineColor[2]));
    Matrix.setPixelColor(MatrixConvert(QuarterOutline[i][0] * -1 + 1, QuarterOutline[i][1], true), Matrix.Color(ClockOutlineColor[0], ClockOutlineColor[1], ClockOutlineColor[2]));
    Matrix.setPixelColor(MatrixConvert(QuarterOutline[i][0], QuarterOutline[i][1] * -1 + 1, true), Matrix.Color(ClockOutlineColor[0], ClockOutlineColor[1], ClockOutlineColor[2]));
    Matrix.setPixelColor(MatrixConvert(QuarterOutline[i][0] * -1 + 1, QuarterOutline[i][1] * -1 + 1, true), Matrix.Color(ClockOutlineColor[0], ClockOutlineColor[1], ClockOutlineColor[2]));
  }
}

void DrawSecond(byte s, int ms) {
  /* s++;
    DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, AdjustBrightness(SecondColor[0], ms), AdjustBrightness(SecondColor[1], ms), AdjustBrightness(SecondColor[2], ms));
    s++;
    DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, AdjustBrightness(SecondColor[0], ms), AdjustBrightness(SecondColor[1], ms), AdjustBrightness(SecondColor[2], ms));
    s -= 2;
    DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, AdjustBrightness(SecondColor[0], 2000 - ms), AdjustBrightness(SecondColor[1], 2000 - ms), AdjustBrightness(SecondColor[2], 2000 - ms));
  */

  DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, SecondColor[0], SecondColor[1], SecondColor[2]);
}

void DrawMinute(byte m) {
  DrawFinger(map(m * 6, 0, 360, 360, 0), Breite / 2 - 3, MinuteColor[0], MinuteColor[1], MinuteColor[2]);
}

void DrawHour(byte h) {
  if (h > 12) {
    h -= 12;
  }
  DrawFinger(map(h * 30, 0, 360, 360, 0), Breite / 2 - 4, HourColor[0], HourColor[1], HourColor[2]);
}

byte AdjustBrightness(byte v, int brightness) { //Full Brightness = 2000
  return round(v * (brightness / 2000.0));
}
void DrawFinger(int angle, int rad, byte r, byte g, byte b) {
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

void DrawLine(int x0, int y0, int x1, int y1, byte r, byte g, byte b)
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

void DrawPixel(int px, int py, bool c, byte r, byte g, byte b) {
  Matrix.setPixelColor(MatrixConvert(px, py, c), Matrix.Color(r, g, b));
}

int MatrixConvert(int x, int y, bool center) {
  int nummer = 0;

  if (center) {
    x += round(Breite / 2);
    y += round(Hohe / 2);
  }

  if (Rotate == 1) {
    int a = x;
    x = y;
    y = a;

    a = x;
    x = y;
    y = a;

    a = x;
    x = y;
    y = a;

  } else if (Rotate == 2) {
    int a = x;
    x = y;
    y = a;

    a = x;
    x = y;
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

void Error(String Code) {
  Serial.println(Code);
}
