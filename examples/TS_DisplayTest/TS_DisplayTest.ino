/*
  TS_DisplayTest.ino - A program to test the touchscreen library functions,
  excluding testing of touch/release events.
  Copied from Paul Stoffregen's XPT2046_Touchscreen project.

  Copyright (c) 2015, Paul Stoffregen, paul@pjrc.com

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice, development funding notice, and this permission
  notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.


  Usage:

  To use this, set the _PIN #defines below for your system, compile, load, run,
  and watch the serial monitor window at 115200 bps.
*/
#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <TS_Display.h>
#include <Fonts/FreeSans9pt7b.h> // From Adafruit_GFX_Library

#define TFT_CS_PIN    10
#define TFT_DC_PIN    2
#define TFT_LED_PIN   A2
#define TOUCH_CS_PIN  A0
#define TOUCH_IRQ_PIN A7
// TOUCH_MOSI_PIN=11, TOUCH_MISO_PIN=12, TOUCH_SCK_PIN=13

// Screen rotation to use. 0=north, 1=east, 2=south, 3=west; 0/2=portrait, 1/3=landscape.
#define ROTATION 2

// TFT screen size in pixels.
#define PIXELS_SHORT 240
#define PIXELS_LONG  320

// Pointer to touchscreen object.
XPT2046_Touchscreen* ts;

// Pointer to TFT LCD display object.
Adafruit_ILI9341* tft;

// Pointer to touchscreen-TFT object.
TS_Display* ts_display;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial && (millis() <= 1000));
  delay(200);

  // Allocate and initialize the TFT display object.
  tft = new Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN);
  pinMode(TFT_LED_PIN, OUTPUT);
  digitalWrite(TFT_LED_PIN, LOW);
  tft->begin();
  tft->setRotation(ROTATION);   // portrait mode
  tft->setTextSize(1);
  tft->setTextWrap(false);
  tft->setFont(&FreeSans9pt7b);

  // Allocate and initialize the touchscreen object.
  ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);  // Param 2 - NULL - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, 255);  // Param 2 - 255 - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, TOUCH_IRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling
  ts->begin();
  ts->setRotation(tft->getRotation());

  // Allocate and initialize the touchscreen-tft object.
  ts_display = new TS_Display();
  ts_display->begin(ts, tft);

  // Set and show pressure threshold.
  ts->setThresholds(Z_THRESHOLD/3);
  Serial.print("Z_Threshold = ");
  Serial.print(ts->Zthreshold());
  Serial.print(", Z_Threshold_Int = ");
  Serial.println(ts->Zthreshold_Int());
  Serial.println();

  // Paint initial display.
  tft->fillScreen(ILI9341_BLACK);
}

boolean wasTouched = true;

void loop() {
  boolean isTouched = ts->touched();
  if (isTouched) {
    TS_Point p = ts->getPoint();
    if (!wasTouched) {
      tft->fillScreen(ILI9341_BLACK);
      tft->setTextColor(ILI9341_YELLOW);
      tft->setCursor(60, 40);
      tft->print("Touch");
      tft->fillRect(100, 150, 140, 60, ILI9341_BLACK);
      tft->setTextColor(ILI9341_GREEN);
      tft->setCursor(10, 100);
      tft->print("X = ");
      tft->print(p.x);
      tft->setCursor(10, 150);
      tft->print("Y = ");
      tft->print(p.y);
      Serial.print("Touch");
      Serial.print(", x = ");
      Serial.print(p.x);
      Serial.print(", y = ");
      Serial.print(p.y);
      Serial.print(", z = ");
      Serial.println(p.z);
      int16_t x, y;
      ts_display->mapTStoDisplay(p.x, p.y, &x, &y);
      tft->drawFastVLine(x, y-10, 20, ILI9341_BLUE);
      tft->drawFastHLine(x-10, y, 20, ILI9341_BLUE);
    }
  } else {
    if (wasTouched) {
      tft->fillScreen(ILI9341_BLACK);
      tft->setTextColor(ILI9341_RED);
      tft->setCursor(120, 40);
      tft->print("No");
      tft->setCursor(80, 100);
      tft->print("Touch");
      Serial.println("no touch");
    }
  }
  wasTouched = isTouched;
  delay(100);
}
