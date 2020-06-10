#include <Adafruit_NeoPixel.h>

const int Hohe = 16;
const int Breite = 16;
const bool invert = true;
const bool rightstart = true;
const int Display = 6;

const byte ClockOutlineColor[3] = {77, 255, 160};
const byte QuarterOutlineLength = 11;
const byte QuarterOutline[QuarterOutlineLength][2] = {{1, 8}, {2, 8}, {3, 7}, {4, 7}, {5, 7}, {6, 6}, {7, 5}, {7, 4}, {7, 3}, {8, 2}, {8, 1}};

const byte HourColor[3] = {255, 25, 16};
const byte MinuteColor[3] = {77, 255, 160};
const byte SecondColor[3] = {0, 255, 0};

byte Seconds = 0;
int Milliseconds = 0;

#define NUMPIXELS Hohe*Breite
Adafruit_NeoPixel Matrix(NUMPIXELS, Display, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);

  Matrix.begin();
  Matrix.setBrightness(100);
  Matrix.clear();
  Matrix.show();

  DrawOutline();
}

void loop() {

  Matrix.clear();
  Milliseconds += 100;
  if (Milliseconds >= 1000) {
    Seconds += 2;
  }
  if (Seconds > 60) {
    Seconds = 0;
  }
  if (Milliseconds >= 1000) {
    Milliseconds = 0;
  }
  DrawSecond(Seconds, Milliseconds);
  DrawOutline();

  
    DrawPixel(0, 0, true, 255, 25, 16);
    DrawPixel(1, 0, true, 255, 25, 16);
    DrawPixel(1, 1, true, 255, 25, 16);
    DrawPixel(0, 1, true, 255, 25, 16);
  Matrix.show();
  delay(100);
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
  s++;
  DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, AdjustBrightness(SecondColor[0], ms), AdjustBrightness(SecondColor[1], ms), AdjustBrightness(SecondColor[2], ms));
  s++;
  DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, AdjustBrightness(SecondColor[0], ms), AdjustBrightness(SecondColor[1], ms), AdjustBrightness(SecondColor[2], ms));
  s -= 2;
  DrawFinger(map(s * 6, 0, 360, 360, 0), Breite / 2 - 2, AdjustBrightness(SecondColor[0], 1000 - ms), AdjustBrightness(SecondColor[1], 1000 - ms), AdjustBrightness(SecondColor[2], 1000 - ms));

}

byte AdjustBrightness(byte v, int brightness) { //Full Brightness = 1000
  return round(v * (brightness / 1000.0));
}
void DrawFinger(int angle, int rad, byte r, byte g, byte b) {
  if (angle <6) {
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
