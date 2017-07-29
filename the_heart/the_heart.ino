// Adafruit_NeoMatrix example for single NeoPixel Shield.
// Scrolls 'Howdy' across the matrix in a portrait (vertical) orientation.

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Ticker.h>

#include "Font5x55pt7b.h"

#define PIN D4

Ticker scroller;
String s_global;
int16_t X, Y;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(9, 9, PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT +
                                               NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
                                               NEO_GRB + NEO_KHZ800);

const uint16_t colors[] = {
    matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

uint16_t myRemapFn(uint16_t x_in, uint16_t y_in)
{
    int x = (int) x_in;
    int y = 8 - (int) y_in;

    if (x + y < 4 || x + y > 12 || x - y > 4 || x - y < -4)
    {
        /* out of bounds */
        return 100;
    }

    x -= 4;
    int row = -x + y;
    int col = (x + y) / 2;

    if (row % 2 == 0)
    {
        return row / 2 * 9 + col;
    }
    else
    {
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

void scroll()
{
    int16_t x1, y1;
    uint16_t w, h;

    /* clear screen */
    matrix.fillScreen(0);

    /* move text left one pixel */
    X--;
    matrix.setCursor(X, Y);

    /* print text */
    matrix.print(s_global);

    /* detect if scroll past end of text */
    matrix.getTextBounds((char*)s_global.c_str(), X, Y, &x1, &y1, &w, &h);
    if ( (x1 + w) <= 0 )
    {
        /* reset to start from outside of display boundary again */
        X = matrix.width();
    }

    /* flush to display */
    matrix.show();
}

void scrollText()
{
    /* reset cursor to outside of the display */
    X = matrix.width();
    Y = 6;

    /* stop any ongoing scroll */
    scroller.detach();

    /* start stroll update */
    scroller.attach_ms(100, scroll);
}

void setup()
{
    Serial.begin(9600);

    /* setup display */
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(100);
    matrix.setTextColor(colors[0]);
    matrix.setFont(&Font5x55pt7b);
    matrix.setRemapFunction(myRemapFn);

    /* clear screen */
    matrix.fillScreen(0);

    /* draw a heart */
    matrix.drawBitmap(1, 2, large_heart, 7, 7, colors[0]);
    matrix.show();
}

void loop()
{
    if (Serial.available())
    {
        s_global = Serial.readString();
        Serial.print(s_global);
        scrollText();
    }
}
