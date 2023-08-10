/*
  TS_DisplayEvents.ino - A program to illustrate the touch/release event capability
  of the touchscreen library.
  Created by Ted Toal, August 7, 2023.
  Released into the public domain.


  Software License Agreement (BSD License)

  Copyright (c) 2023 Ted Toal
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  3. Neither the name of the copyright holders nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


  Usage:

  To use this, set the _PIN #defines below for your system, compile, load, run,
  then tap and release on the touchscreen and watch the monitor window to see
  how these events are detected.
*/
#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <TS_Display.h>
#include <Fonts/FreeSans9pt7b.h> // From Adafruit_GFX_Library
#include <monitor_printf.h>

// TFT display and touchscreen I/O pin definitions, using standard Arduino pin
// numbers/IDs.
// These values are set for my system, change them to the appropriate values for
// your own system.
#define TFT_CS_PIN    10
#define TFT_DC_PIN    2
#define TFT_LED_PIN   A2
#define TOUCH_CS_PIN  A0
#define TOUCH_IRQ_PIN A7
// TOUCH_MOSI_PIN=11, TOUCH_MISO_PIN=12, TOUCH_SCK_PIN=13

// Screen rotation to use. 0=north, 1=east, 2=south, 3=west; 0/2=portrait, 1/3=landscape.
#define ROTATION 2

// Pointer to touchscreen object.
XPT2046_Touchscreen* ts;

// Pointer to TFT LCD display object.
Adafruit_ILI9341* tft;

// Pointer to touchscreen/TFT LCD display object.
TS_Display* ts_display;

// Print string S to display at cursor position (x,y).
void tft_print(int16_t x, int16_t y, const char* S) {
  tft->setCursor(x, y);
  tft->print(S);
}

// Standard Arduino setup function.
void setup() {
  // Initialize for printfs to serial monitor.
  monitor.begin(&Serial, 115200);
  monitor.printf("Initializing\n");

  // Create TFT display object and initialize it.
  tft = new Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN);
  pinMode(TFT_LED_PIN, OUTPUT);
  digitalWrite(TFT_LED_PIN, LOW);
  tft->begin();
  tft->setRotation(ROTATION);
  tft->setTextSize(1);
  tft->setTextWrap(false);
  tft->setFont(&FreeSans9pt7b);
  tft->setTextColor(ILI9341_RED);

  // Create touchscreen object and initialize it.
  ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);
  ts->begin();
  ts->setRotation(tft->getRotation());

  // Create touchscreen-TFT object and initialize it.
  ts_display = new TS_Display();
  ts_display->begin(ts, tft);

  // Tell user to tap screen.
  tft->fillScreen(ILI9341_WHITE);
  tft_print(20, 20, "Tap on screen and hold");
  tft_print(30, 50, "then release");
  tft->setTextColor(ILI9341_BLUE);
}

// Standard Arduino loop() function.
void loop() {
  int16_t x, y, pres, px, py;
  eTouchEvent touchEvent = ts_display->getTouchEvent(x, y, pres, &px, &py);
  switch (touchEvent) {
  case TS_TOUCH_EVENT:
    monitor.printf("TOUCH EVENT at (%d,%d), pressure=%d, touchscreen at (%d, %d)\n",
      x, y, pres, px, py);
    tft->fillRect(0, 60, 240, 30, ILI9341_WHITE);
    tft_print(10, 85, "TOUCH EVENT");
    // Draw a plus sign at a specified TFT location.
    tft->drawFastVLine(x, y-10, 21, ILI9341_BLUE);
    tft->drawFastHLine(x-10, y, 21, ILI9341_BLUE);
    break;

  case TS_RELEASE_EVENT:
    tft->fillRect(0, 60, 240, 30, ILI9341_WHITE);
    tft_print(10, 85, "RELEASE EVENT");
    monitor.printf("RELEASE EVENT\n");
    break;
  }
}
