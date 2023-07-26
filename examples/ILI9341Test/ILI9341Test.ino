/*
  ILI9341Test.ino - A program to test the touchscreen library functions.
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

  To use this, set the _PIN #defines below for your system, compile, load, and run.
*/
#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans12pt7b.h> // From Adafruit-GFX-Library
#include <XPT2046_Touchscreen_TT.h>
#include <SPI.h>

#define TFT_CS_PIN    10
#define TFT_DC_PIN    2
#define TFT_LED_PIN   A2
#define TOUCH_CS_PIN  10
#define TOUCH_IRQ_PIN 2
// TOUCH_MOSI_PIN=11, TOUCH_MISO_PIN=12, TOUCH_SCK_PIN=13

// Touchscreen object.
int xxx[400];
XPT2046_Touchscreen* ts;
int yyy[400];

// TFT LCD display object.
Adafruit_ILI9341* tft;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial && (millis() <= 1000));
  delay(200);
  tft = new Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN);
  tft->begin();
  tft->setRotation(1);
  tft->fillScreen(ILI9341_BLACK);
  pinMode(TFT_LED_PIN, OUTPUT);
  digitalWrite(TFT_LED_PIN, LOW);

  XPT2046_Touchscreen* ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);
  //XPT2046_Touchscreen* ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);  // Param 2 - NULL - No interrupts
  //XPT2046_Touchscreen* ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, 255);  // Param 2 - 255 - No interrupts
  //XPT2046_Touchscreen* ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, TOUCH_IRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling
  ts->begin();
  ts->setRotation(1);
  ts->setThresholds(Z_THRESHOLD*6);

  Serial.print("Z_Threshold = ");
  Serial.print(ts->Zthreshold());
  Serial.print(", Z_Threshold_Int = ");
  Serial.println(ts->Zthreshold_Int());
  Serial.println();
}

boolean wastouched = true;
boolean firstTime = true;

void loop() {
  if (firstTime) {
    Serial.print("First time main loop, Z_Threshold = ");
    Serial.print(ts->Zthreshold());
    Serial.print(", Z_Threshold_Int = ");
    Serial.println(ts->Zthreshold_Int());
    Serial.println();
    firstTime = false;
  }
  boolean istouched = ts->touched();
  if (istouched) {
    TS_Point p = ts->getPoint();
    if (!wastouched) {
      tft->fillScreen(ILI9341_BLACK);
      tft->setTextColor(ILI9341_YELLOW);
      tft->setFont(&FreeSans12pt7b);
      tft->setCursor(60, 80);
      tft->print("Touch");
    }
    tft->fillRect(100, 150, 140, 60, ILI9341_BLACK);
    tft->setTextColor(ILI9341_GREEN);
    tft->setFont(&FreeSans12pt7b);
    tft->setCursor(100, 150);
    tft->print("X = ");
    tft->print(p.x);
    tft->setCursor(100, 180);
    tft->print("Y = ");
    tft->print(p.y);
    Serial.print("Touch");
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    Serial.print(", z = ");
    Serial.println(p.z);
  } else {
    if (wastouched) {
      tft->fillScreen(ILI9341_BLACK);
      tft->setTextColor(ILI9341_RED);
      tft->setFont(&FreeSans12pt7b);
      tft->setCursor(120, 50);
      tft->print("No");
      tft->setCursor(80, 120);
      tft->print("Touch");
    }
    Serial.println("no touch");
  }
  wastouched = istouched;
  delay(100);
}
