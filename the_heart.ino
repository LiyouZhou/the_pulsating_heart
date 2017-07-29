// Adafruit_NeoMatrix example for single NeoPixel Shield.
// Scrolls 'Howdy' across the matrix in a portrait (vertical) orientation.

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif

#include "5x55pt7b.h"
//#include "Fonts/TomThumb.h"
#define PIN D4

// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)


// Example for NeoPixel Shield.  In this application we'd like to use it
// as a 5x8 tall matrix, with the USB port positioned at the top of the
// Arduino.  When held that way, the first pixel is at the top right, and
// lines are arranged in columns, progressive order.  The shield uses
// 800 KHz (v2) pixels that expect GRB color data.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(9, 9, PIN,
                            NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
                            NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
                            NEO_GRB            + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

uint16_t myRemapFn(uint16_t x_in, uint16_t y_in) {
  int x = (int) x_in;
  int y = 8 - (int) y_in;

//    Serial.print("in: ");
//    Serial.print(x);
//    Serial.print(" ");
//    Serial.println(y);
  //  Serial.print(" ");

  if (x + y < 4 || x + y > 12 || x - y > 4 || x - y < -4) {
    //    Serial.println("out: 100");
    return 100;
  }

  x -= 4;
  int row = -x + y;
  int col = (x + y) / 2;
  /*  Serial.print("row ");
    Serial.print(row);
    Serial.print(" col ");
    Serial.print(col);
    Serial.print(" ");*/

  if (row % 2 == 0) {
    //    Serial.print("out: ");
    //    Serial.println(row/2*9 + col);
    return row / 2 * 9 + col;
  } else {
    //    Serial.print("out: ");
    //    Serial.println(row/2*9 + 5 + (3-col));
    return row / 2 * 9 + 5 + (3 - col);
  }
}
const uint8_t large_heart[] PROGMEM = {
0x36 << 1,
0xff << 1,
0xff << 1,
0xff << 1,
0x3e << 1,
0x1c << 1,
0x08 << 1
};

void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(100);
  matrix.setTextColor(colors[0]);
//  matrix.setFont(&TomThumb);
  matrix.setFont(&Font5x55pt7b);
  matrix.setRemapFunction(myRemapFn);
  matrix.fillScreen(0);
  matrix.drawBitmap(1, 2, large_heart, 7, 7, colors[0]);
  matrix.show();
}

int x    = matrix.width();
int pass = 0;

char text[100];
float n;

#include <Ticker.h>

Ticker flipper;
String s_global;
int16_t X, Y;
void flip()
{
  matrix.fillScreen(0);
  X--;
  Serial.print("coords");
  Serial.print(X);
  Serial.print(" ");
  Serial.print(Y);
  matrix.setCursor(X, Y);
  matrix.print(s_global);

  int16_t x1, y1;
  uint16_t w, h;

  matrix.getTextBounds((char*)s_global.c_str(), X, Y, &x1, &y1, &w, &h);
  Serial.print(" ");
  Serial.print(x1);
  Serial.print(" ");
  Serial.print(y1);
  Serial.print(" ");
  Serial.print(w);
  Serial.print(" ");
  Serial.print(h);
  Serial.println();
  if ( (x1 + w) <= 0 )
  {
    X = 9;
  }
  matrix.show();
}

void scrollText()
{
  X=2;
  Y=6;
  flipper.detach();
  flipper.attach_ms(100, flip);
}

void loop() {
//  n += 0.1;
//  matrix.setBrightness(sin(n)*50 + 50 + 20);
//  matrix.show();
//  delay(50);
  
  if (Serial.available()) {
    s_global = Serial.readString();
    Serial.print(s_global);
    scrollText();
  }

    /*matrix.fillScreen(0);
    //matrix.setCursor(x, 7);
    matrix.drawBitmap(x, 0, (uint8_t *) large_heart, 7, 7, colors[0]);
    if(--x < -7) {
      x = matrix.width();
      if(++pass >= 3) pass = 0;
      matrix.setTextColor(colors[pass]);
    }
    matrix.show();
    delay(100);*/
}
