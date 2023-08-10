/*
  TS_DisplayCalibrate.ino - A program to calibrate the touchscreen-to-display mapping.
  Created by Ted Toal, July 26, 2023.
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
  tap the centers of the displayed +'s, and make note of the calibration values
  displayed on the TFT display and written to the serial monitor and used in a
  call to setTS_calibration() to change the touchscreen calibration parameters.
  Use those parameter values in your project (calling setTS_calibration() with
  them) or incorporate similar touchscreen calibration logic to this into your
  project.

  Also, in your project, allocate and initialize a 'ts_display' object the same
  as done here, and use it for mapping touchscreen point coordinates to TFT
  display coordinates.

  If the SAMD architecture is being used (ARDUINO_ARCH_SAMD), this stores the
  calibration values in the microprocessor EEPROM using the library module
  FlashStorage_SAMD. If you restart the program (without reloading it), it
  should initialize with the last calibration values rather than the defaults.
  When you reload the program, that erases the EEPROM so it has to start anew at
  storing calibrated values after you do a new calibration.
*/
#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include <Adafruit_ILI9341.h>
#include <TS_Display.h>
#include <Fonts/FreeSans9pt7b.h> // From Adafruit_GFX_Library
#include <monitor_printf.h>

// EEPROM support currently only if SAMD architecture.
#ifdef ARDUINO_ARCH_SAMD

// It appears (page 29 of Atmel SAM D21E / SAM D21G / SAM D21J data sheet) that
// the EEPROM page size is 64, and 4 pages must be erased at one time, giving an
// effective page size of 4*64 = 256.  This seems to be confirmed by the fact
// that FlashStorage_SAMD.hpp aligns the PPCAT variable to 256 bytes.
#define EEPROM_EMULATION_SIZE     (4 * 64)

// Use 0-2. Larger for more debugging messages
#define FLASH_DEBUG       0

// Now include flash storage library header file.
// To be included only in one file to avoid `Multiple Definitions` Linker Error.
#include <FlashStorage_SAMD.h>

// Signature used at start of a flash memory block to mark the block as
// containing valid data written by the application.
const int WRITTEN_SIGNATURE = 0xBEEFDEED;

#endif

/////////////////////////////////////////////////////////////////////////////////////////////
// Constants.
/////////////////////////////////////////////////////////////////////////////////////////////

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

// Length of each arm of "+" sign.
#define PLUS_ARM_LEN 10

// Color of background, text, calibration plus signs, and test plus signs.
#define COLOR_BKGD ILI9341_BLACK
#define COLOR_TEXT ILI9341_YELLOW
#define COLOR_PLUS_CALIB ILI9341_BLUE
#define COLOR_PLUS_TEST ILI9341_GREEN

// Text for user instructions.
#define TEXT_TAP_PLUS "Tap the +"
#define TEXT_TAP_RESULT "Tap to show calibration"
#define TEXT_TAP_TEST "Tap to test calibration"

// Size of TFT printf buffer in characters.
#define TFT_PRINTF_BUF_SIZE 80

/////////////////////////////////////////////////////////////////////////////////////////////
// Enums.
/////////////////////////////////////////////////////////////////////////////////////////////

// States during calibration and subsequent showing of tapped points.
typedef enum _eState {
  STATE_WAIT_UL,            // Wait for user to tap + at upper-left
  STATE_WAIT_UL_RELEASE,    // Wait for him to release the tap
  STATE_WAIT_LR,            // Wait for user to tap + at lower-right
  STATE_WAIT_LR_RELEASE,    // Wait for him to release the tap
  STATE_WAIT_SHOW_CALIB,    // Wait for user to tap anywhere before showing calibration results
  STATE_WAIT_RELEASE,       // Wait for him to release the tap
  STATE_WAIT_POINT_SHOW_IT  // Wait for user to tap anywhere, then draw "+" there
} eState;

/////////////////////////////////////////////////////////////////////////////////////////////
// Structs.
/////////////////////////////////////////////////////////////////////////////////////////////

// Structure containing non-volatile data to be stored in flash memory (with
// copy in regular memory). We use this structure even if we don't have the SAMD
// architecture support for storing it in EEPROM.
struct nonvolatileSettings {
  int16_t TS_LR_X;
  int16_t TS_LR_Y;
  int16_t TS_UL_X;
  int16_t TS_UL_Y;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Variables.
/////////////////////////////////////////////////////////////////////////////////////////////

// Pointer to touchscreen object.
XPT2046_Touchscreen* ts;

// Pointer to TFT LCD display object.
Adafruit_ILI9341* tft;

// Pointer to touchscreen/TFT LCD display object.
TS_Display* ts_display;

// TFT UL and LR corner positions and corresponding touchscreen corner coordinates.
int16_t x_ULcorner, y_ULcorner, x_LRcorner, y_LRcorner;
int16_t TSx_ULcorner, TSy_ULcorner, TSx_LRcorner, TSy_LRcorner;

// TFT display UL and LR calibration positions and corresponding touchscreen
// calibration coordinates.
int16_t x_UL, y_UL, x_LR, y_LR;
int16_t TSx_UL, TSy_UL, TSx_LR, TSy_LR;

// TFT printf buffer.
char TFTprintfBuf[TFT_PRINTF_BUF_SIZE];

// Current state of interaction with user.
eState state;

// The current non-volatile settings initialized from flash-based EEPROM (and
// written back to EEPROM when the data changes).
nonvolatileSettings NVsettings;

/////////////////////////////////////////////////////////////////////////////////////////////
// Functions.
/////////////////////////////////////////////////////////////////////////////////////////////

//**************************************************************************
// Printf to TFT at cursor position (x,y) in specified color.
//**************************************************************************
void tft_printf(int16_t x, int16_t y, int16_t color, const char* format, ...) {
  tft->setCursor(x, y);
  tft->setTextColor(color);
  va_list args;
  va_start(args, format);
  vsnprintf(TFTprintfBuf, TFT_PRINTF_BUF_SIZE, format, args);
  va_end(args);
  tft->print(TFTprintfBuf);
}

//**************************************************************************
// Draw a plus sign at a specified TFT location.
//**************************************************************************
void drawPlus(int16_t x, int16_t y, int16_t color, uint8_t len = PLUS_ARM_LEN) {
  tft->drawFastVLine(x, y-len, 2*len+1, color);
  tft->drawFastHLine(x-len, y, 2*len+1, color);
}

//**************************************************************************
// Show touchscreen coordinate values (TSx, TSy) at TFT cursor position (x, y).
//**************************************************************************
void showTS_XY(int16_t x, int16_t y, int16_t TSx, int16_t TSy) {
  tft_printf(x, y, COLOR_TEXT, "TX = %d,  TY = %d", TSx, TSy);
}

// EEPROM support currently only if SAMD architecture.
#ifdef ARDUINO_ARCH_SAMD

//**************************************************************************
// Read non-volatile settings from flash memory into 'settings'.  If flash
// memory has not yet been initialized, initialize it with 'defaults'.
//**************************************************************************
void readNonvolatileSettings(nonvolatileSettings& settings,
    const nonvolatileSettings& defaults) {
  // Initialize to only commit data when we call the commit function.
  EEPROM.setCommitASAP(false);
  // Check signature at address 0.
  int signature;
  uint16_t storedAddress = 0;
  EEPROM.get(storedAddress, signature);
  // If flash-based EEPROM is empty, write WRITTEN_SIGNATURE and defaults to it.
  if (signature != WRITTEN_SIGNATURE) {
    monitor.printf("EEPROM is uninitialized, writing defaults\n");
    EEPROM.put(storedAddress, WRITTEN_SIGNATURE);
    EEPROM.put(storedAddress + sizeof(signature), defaults);
    EEPROM.commit();
  }

  // Read settings data from flash-based EEPROM.
  EEPROM.get(storedAddress + sizeof(signature), settings);
}

//**************************************************************************
// Write 'settings' to flash memory IF IT HAS CHANGED. Return true if it changed
// and was written, else false.
//**************************************************************************
bool writeNonvolatileSettingsIfChanged(nonvolatileSettings& settings) {
  int signature;
  uint16_t storedAddress = 0;
  nonvolatileSettings tmp;
  EEPROM.get(storedAddress + sizeof(signature), tmp);
  if (memcmp(&settings, &tmp, sizeof(nonvolatileSettings)) == 0)
    return(false);
  EEPROM.put(storedAddress + sizeof(signature), settings);
  EEPROM.commit();
  return(true);
}

#endif

//**************************************************************************
// Show nonvolatile settings on serial monitor.
//**************************************************************************
void showNVsettings(char* title, const nonvolatileSettings& settings) {
  monitor.printf("%s\n", title);
  monitor.printf(" TS_LR_X: %d  TS_LR_Y: %d  TS_UL_X: %d  TS_UL_Y: %d\n",
    settings.TS_LR_X, settings.TS_LR_Y, settings.TS_UL_X, settings.TS_UL_Y);
}

//**************************************************************************
// Standard Arduino setup function.
//**************************************************************************
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
  tft->setTextColor(COLOR_TEXT);

  // Create touchscreen object and initialize it.
  ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);  // Param 2 - NULL - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, 255);  // Param 2 - 255 - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, TOUCH_IRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling
  ts->begin();
  ts->setRotation(tft->getRotation());

  // Create touchscreen-TFT object and initialize it.
  ts_display = new TS_Display();
  ts_display->begin(ts, tft);

  // Make 'defaults' hold the default non-volatile settings when the settings
  // are first initialized. Here, we get the initial calibration settings from
  // ts_display.
  nonvolatileSettings defaults;
  ts_display->getTS_calibration(&defaults.TS_LR_X, &defaults.TS_LR_Y, &defaults.TS_UL_X,
    &defaults.TS_UL_Y);
  showNVsettings("Calibration setting defaults:", defaults);

  // EEPROM support currently only if SAMD architecture.
  #ifdef ARDUINO_ARCH_SAMD
  // Read non-volatile settings from flash-based EEPROM into 'NVsettings', then
  // set them as the current calibration settings in 'ts_display'.
  readNonvolatileSettings(NVsettings, defaults);
  ts_display->setTS_calibration(NVsettings.TS_LR_X, NVsettings.TS_LR_Y, NVsettings.TS_UL_X,
    NVsettings.TS_UL_Y);
  showNVsettings("Non-volatile EEPROM calibration settings:", NVsettings);
  #else
  monitor.printf("Unable to read calibration settings from EEPROM - currently this needs SAMD architecture to do that\n");
  #endif

  // Get position of TFT upper-left and lower-right corner.
  x_ULcorner = 0;
  y_ULcorner = 0;
  x_LRcorner = tft->width() - 1;
  y_LRcorner = tft->height() - 1;

  // Get position of two TFT points at which to draw "+" signs and require that
  // the user click them to calibrate the touchscreen.
  ts_display->GetCalibration_UL_LR(PLUS_ARM_LEN+2, &x_UL, &y_UL, &x_LR, &y_LR);

  // Paint first "+" and wait for user to click that point.
  tft->fillScreen(COLOR_BKGD);
  drawPlus(x_UL, y_UL, COLOR_PLUS_CALIB);
  tft_printf(80, 20, COLOR_TEXT, TEXT_TAP_PLUS);
  state = STATE_WAIT_UL;
  monitor.printf("Waiting for tap at UL corner\n");
}

//**************************************************************************
// Standard Arduino loop() function.
//**************************************************************************
void loop() {
  boolean isTouched = ts->touched();
  TS_Point p;
  if (isTouched)
    p = ts->getPoint();

  switch (state) {

  case STATE_WAIT_UL:
    if (isTouched) {
      TSx_UL = p.x;
      TSy_UL = p.y;
      state = STATE_WAIT_UL_RELEASE;
      // Show tapped position.
      showTS_XY(10, 50, TSx_UL, TSy_UL);
      monitor.printf("UL corner tapped at position (%d, %d)\n", TSx_UL, TSy_UL);
    }
    break;

  case STATE_WAIT_UL_RELEASE:
    if (!isTouched) {
      state = STATE_WAIT_LR;
      // Erase the first plus and instructions.
      drawPlus(x_UL, y_UL, COLOR_BKGD);
      tft_printf(80, 20, COLOR_BKGD, TEXT_TAP_PLUS);
      // Paint second + and wait for user to click that point.
      drawPlus(x_LR, y_LR, COLOR_PLUS_CALIB);
      tft_printf(80, tft->height()-20, COLOR_TEXT, TEXT_TAP_PLUS);
      monitor.printf("Waiting for tap at LR corner\n");
    }
    break;

  case STATE_WAIT_LR:
    if (isTouched) {
      TSx_LR = p.x;
      TSy_LR = p.y;
      state = STATE_WAIT_LR_RELEASE;
      // Show tapped position.
      showTS_XY(10, 70, TSx_LR, TSy_LR);
      monitor.printf("LR corner tapped at position (%d, %d)\n", TSx_LR, TSy_LR);
    }
    break;

  case STATE_WAIT_LR_RELEASE:
    if (!isTouched) {
      state = STATE_WAIT_SHOW_CALIB;
      // Erase the second plus and instructions.
      drawPlus(x_LR, y_LR, COLOR_BKGD);
      tft_printf(80, tft->height()-20, COLOR_BKGD, TEXT_TAP_PLUS);
      tft->setCursor(10, 90);
      tft_printf(10, 90, COLOR_TEXT, TEXT_TAP_RESULT);
    }
    break;

  case STATE_WAIT_SHOW_CALIB:
    if (isTouched) {
      state = STATE_WAIT_RELEASE;
      // Map the two touchscreen points to the correct calibration values at the
      // extreme ends of the display. Put resulting calibration parameters into
      // NVsettings and set them as the new calibration parameters in ts_display,
      // as well as writing them to the EEPROM.
      ts_display->findTS_calibration(x_UL, y_UL, x_LR, y_LR, TSx_UL, TSy_UL, TSx_LR,
        TSy_LR, &NVsettings.TS_LR_X, &NVsettings.TS_LR_Y, &NVsettings.TS_UL_X,
        &NVsettings.TS_UL_Y);
      ts_display->setTS_calibration(NVsettings.TS_LR_X, NVsettings.TS_LR_Y,
        NVsettings.TS_UL_X, NVsettings.TS_UL_Y);
      // EEPROM support currently only if SAMD architecture.
      #ifdef ARDUINO_ARCH_SAMD
      writeNonvolatileSettingsIfChanged(NVsettings);
      #else
      monitor.printf("Unable to store calibration settings in EEPROM - currently this needs SAMD architecture to do that\n");
      #endif
      showNVsettings("Calibration results:", NVsettings);

      // Show the display corner points calibration mapping on the display.
      ts_display->mapDisplayToTS(x_ULcorner, y_ULcorner, &TSx_ULcorner, &TSy_ULcorner);
      ts_display->mapDisplayToTS(x_LRcorner, y_LRcorner, &TSx_LRcorner, &TSy_LRcorner);
      tft_printf(10, 110, COLOR_TEXT, "UL (%d, %d) maps to:", x_ULcorner, y_ULcorner);
      showTS_XY(10, 130, TSx_ULcorner, TSy_ULcorner);
      tft_printf(10, 150, COLOR_TEXT, "LR (%d, %d) maps to:", x_LRcorner, y_LRcorner);
      showTS_XY(10, 170, TSx_LRcorner, TSy_LRcorner);
      monitor.printf("  UL corner (%d, %d) maps to touchscreen (%d, %d)\n",
        x_ULcorner, y_ULcorner, TSx_ULcorner, TSy_ULcorner);
      monitor.printf("  LR corner (%d, %d) maps to touchscreen (%d, %d)\n",
        x_LRcorner, y_LRcorner, TSx_LRcorner, TSy_LRcorner);
    }
    break;

  case STATE_WAIT_RELEASE:
    if (!isTouched) {
      state = STATE_WAIT_POINT_SHOW_IT;
      tft_printf(10, 200, COLOR_TEXT, TEXT_TAP_TEST);
      monitor.printf("Waiting for random tap\n");
    }
    break;

  case STATE_WAIT_POINT_SHOW_IT:
    if (isTouched) {
      state = STATE_WAIT_RELEASE;
      // Map touched point to TFT, erase screen, and draw a green "+" at that point.
      int16_t x, y;
      ts_display->mapTStoDisplay(p.x, p.y, &x, &y);
      tft->fillRect(x_ULcorner, y_ULcorner, tft->width(), tft->height(), COLOR_BKGD);
      drawPlus(x, y, COLOR_PLUS_TEST);
      monitor.printf("Random touch at x = %d, y = %d maps to (%d, %d)\n", p.x, p.y, x, y);
    }
    break;
  }
  delay(10);
}
