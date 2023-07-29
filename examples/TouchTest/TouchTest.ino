/*
  TouchTest.ino - A program to show touchscreen touch coordinates.
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
#include <SPI.h>
#include <XPT2046_Touchscreen_TT.h>

#define TOUCH_CS_PIN  10
#define TOUCH_IRQ_PIN A7
// TOUCH_MOSI_PIN=11, TOUCH_MISO_PIN=12, TOUCH_SCK_PIN=13

// Pointer to touchscreen object.
XPT2046_Touchscreen* ts;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial && (millis() <= 1000));
  delay(200);

  ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);  // Param 2 - NULL - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, 255);  // Param 2 - 255 - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, TOUCH_IRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling
  ts->begin();
  ts->setRotation(1);
}

void loop() {
  if (ts->touched()) {
    TS_Point p = ts->getPoint();
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    delay(30);
    Serial.println();
  }
}
