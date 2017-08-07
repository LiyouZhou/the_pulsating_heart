#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Ticker.h>

#include "Font5x55pt7b.h"
#include <ESP8266WiFi.h>
#include "FS.h"#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <IRremoteESP8266.h>

/* for WLAN_SSID, WLAN_PASS, AIO_KEY */
#include "credentials.h"

/* for some handy colour names */
#include "colours.h"

/* Adafruit.io Setup */
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883                   // 8883 for MQTTS
#define AIO_USERNAME    "liyouzhou"

/* global state machine */
typedef enum {
    STATE_CONNECT,
    STATE_LISTEN,
    STATE_EMIT,
    STATE_KEEP_ALIVE
} the_heart_state_t;
the_heart_state_t state = STATE_CONNECT;

/* WiFiFlientSecure for SSL/TLS support */
WiFiClientSecure client;

/* Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. */
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/* io.adafruit.com SHA1 fingerprint */
const char* fingerprint = "AD 4B 64 B3 67 40 B5 FC 0E 51 9B BD 25 E9 7F 88 B6 2A A3 5B";

/* Setup a feed. Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> */
Adafruit_MQTT_Subscribe display_msg_feed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/f/hdisp");

/* workaround for spurious messages on first connect */
bool first_connect = false;

/* buffer to hold incoming mqtt messages */
char msg_buf[100] = {0};

/* neopixel pin */
#define PIN D8

/* globals to be used by text scroller */
Ticker scroller;
char scroll_buf[100] = {0};
int16_t X, Y;

/* declear the display matrix global */
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(9, 9, PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT +
                                               NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
                                               NEO_GRB + NEO_KHZ800);

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
    matrix.print(scroll_buf);

    /* detect if scroll past end of text */
    matrix.getTextBounds((char*)scroll_buf, X, Y, &x1, &y1, &w, &h);
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

void verifyFingerprint()
{
    const char* host = AIO_SERVER;

    Serial.print("Connecting to ");
    Serial.println(host);

    if (!client.connect(host, AIO_SERVERPORT))
    {
        Serial.println("Connection failed. Halting execution.");
        while (1);
    }

    if (client.verify(fingerprint, host))
    {
        Serial.println("Connection secure.");
    }
    else
    {
        Serial.println("Connection insecure! Halting execution.");
        while (1);
    }
}

/* Function to connect and reconnect as necessary to the MQTT server.
 * Should be called in the loop function and it will take care if connecting.
 */
void MQTT_connect()
{
    int8_t ret;

    /* Stop if already connected. */
    if (mqtt.connected())
    {
        return;
    }

    Serial.print("Connecting to MQTT... ");

    uint8_t retries = 10;
    while ((ret = mqtt.connect()) != 0)
    {
        /* connect will return 0 for connected */
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        delay(5000);  // wait 5 seconds
        retries--;
        if (retries == 0)
        {
            /* die and wait for WDT to reset */
            while (1);
        }
    }

    first_connect = true;
    Serial.println("MQTT Connected!");
}

void mqtt_msg_callback(char* buf, uint16_t len)
{
    /* print message */
    Serial.print("Received: ");
    for (uint16_t i = 0; i < len; i++)
    {
        Serial.print(buf[i]);
    }
    Serial.println();

    if (len >= sizeof(msg_buf))
    {
        Serial.println("Message Too Long!!!");
        return;
    }

    memcpy(msg_buf, buf, len);
    msg_buf[len] = 0;

    state = STATE_EMIT;
}

uint16_t colour_name_to_colour(char* buf)
{
    uint16_t color = 0;

    if (strcmp(msg_buf+1, "BLUE") == 0)
    {
        color = BLUE;
    }
    else if (strcmp(msg_buf+1, "GREEN") == 0)
    {
        color = GREEN;
    }
    else if (strcmp(msg_buf+1, "CYAN") == 0)
    {
        color = CYAN;
    }
    else if (strcmp(msg_buf+1, "MAGENTA") == 0)
    {
        color = MAGENTA;
    }
    else if (strcmp(msg_buf+1, "YELLOW") == 0)
    {
        color = YELLOW;
    }
    else if (strcmp(msg_buf+1, "WHITE") == 0)
    {
        color = WHITE;
    }
    else if (strcmp(msg_buf+1, "RED") == 0)
    {
        color = RED;
    }

    return color;
}

void emit()
{
    uint16_t len = strnlen(msg_buf, 100);
    if (len > 0 && len < 100)
    {
        uint8_t intensity, r, g, b;
        uint16_t color;

        switch(msg_buf[0])
        {
            case int('0'):
                intensity = atoi(msg_buf+1);
                matrix.setBrightness(intensity);
                break;

            case int('1'):
                matrix.setTextColor(colour_name_to_colour(msg_buf+1));
                break;

            case int('2'):
                r = atoi(msg_buf+1);
                g = atoi(msg_buf+5);
                b = atoi(msg_buf+9);
                color = uint8_t(r / float(UINT8_MAX) * 0x11111) << 11 |
                        uint8_t(g / float(UINT8_MAX) * 0x111111) << 5 |
                        uint8_t(b / float(UINT8_MAX) * 0x11111);
                Serial.println(r);
                Serial.println(g);
                Serial.println(b);
                Serial.println(color);
                matrix.setTextColor(color);
                break;

            case int('3'):
                /* stop any ongoing scroll */
                scroller.detach();
                /* fill screen */
                matrix.fillScreen(colour_name_to_colour(msg_buf+1));
                break;

            default:
                memcpy(scroll_buf, msg_buf, len);
                scroll_buf[len] = 0;
                scrollText();
                break;
        }

        /* flush to display */
        matrix.show();
    }
}

void setup()
{
    Serial.begin(115200);

    /* Connect to WiFi access point. */
    Serial.print("Connecting to ");
    Serial.println(WLAN_SSID);

    WiFi.begin(WLAN_SSID, WLAN_PASS);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("Unable to connect ot WIFI");
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    /* check the fingerprint of io.adafruit.com's SSL cert */
    verifyFingerprint();

    /* setup display */
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(10);
    matrix.setTextColor(RED);
    matrix.setFont(&Font5x55pt7b);
    matrix.setRemapFunction(myRemapFn);

    /* clear screen */
    matrix.fillScreen(0);

    /* draw a heart */
//    matrix.drawBitmap(1, 2, large_heart, 7, 7, RED);
//    matrix.show();
    char s[] = "Still Hacking Anyway...";
    mqtt_msg_callback(s, strlen(s));

    /* setup feed callback and subscribe */
    display_msg_feed.setCallback(mqtt_msg_callback);
    mqtt.subscribe(&display_msg_feed);
}

void loop()
{
    static unsigned long last_ping = millis();

    switch (state)
    {
        case STATE_CONNECT:
            Serial.println("STATE_CONNECT");
            MQTT_connect();
            state = STATE_LISTEN;
            break;

        case STATE_LISTEN:
            Serial.println("STATE_LISTEN");
            mqtt.processPackets(1000);
            if (state == STATE_LISTEN)
            {
                state = STATE_KEEP_ALIVE;
            }
            break;

        case STATE_EMIT:
            Serial.println("STATE_EMIT");
            emit();
            state = STATE_CONNECT;
            break;

        case STATE_KEEP_ALIVE:
            Serial.println("STATE_KEEP_ALIVE");
            if (millis() - last_ping > 30 * 1000)
            {
                Serial.println("Pinging");
                mqtt.ping();
                last_ping = millis();
            }
            state = STATE_CONNECT;
            break;
    }
}
