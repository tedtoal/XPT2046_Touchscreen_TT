/*
  TS_ILI9341_map.cpp - Support mapping coordinates between an XPT2046-controlled
  touchscreen and an ILI9341-controlled TFT LCD display, with calibration.
  Created by Ted Toal, July 26, 2023
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
*/
#include <Arduino.h>
#include <TS_ILI9341_map.h>

// The four TS_ constants below are used to set the initial default calibration
// parameter values to reasonable values probably suitable for most touchscreens.
// The screen rotation is taken into account when using the values below to
// initialize the mapping parameter values. In rotation mode 2 the values below
// can be directly assigned to the calibration parameters, but in the other
// rotation modes an offset must be applied.
//
// In the constant names, "SHORT" refers to either x- or y- coordinate,
// depending on which direction is shorter in size for the current rotation,
// and "LONG" refers to the opposite direction that is longer in size.
#define TS_UL_SHORT  3800
#define TS_UL_LONG   3700
#define TS_LR_SHORT  275
#define TS_LR_LONG   165

// Value used to "flip" touchscreen coordinates. This value is used within the
// original touchscreen code, and it is required here in order to properly
// adjust the mapping parameter values in rotation modes 0 and 1.
#define TS_OFFSET 4095

/**************************************************************************/
void TS_ILI9341_map::begin(XPT2046_Touchscreen* ts, Adafruit_ILI9341* tft) {
  _ts = ts;
  _tft = tft;

  switch (_tft->getRotation()) {

  case 0:
    _TS_UL_X = TS_OFFSET - TS_LR_SHORT;
    _TS_UL_Y = TS_OFFSET - TS_LR_LONG;
    _TS_LR_X = TS_OFFSET - TS_UL_SHORT;
    _TS_LR_Y = TS_OFFSET - TS_UL_LONG;
    break;

  case 1:
    _TS_UL_X = TS_OFFSET - TS_LR_SHORT;
    _TS_UL_Y = TS_UL_SHORT;
    _TS_LR_X = TS_OFFSET - TS_UL_SHORT;
    _TS_LR_Y = TS_LR_SHORT;
    break;

  case 2:
    _TS_UL_X = TS_UL_SHORT;
    _TS_UL_Y = TS_UL_SHORT;
    _TS_LR_X = TS_LR_SHORT;
    _TS_LR_Y = TS_LR_SHORT;
    break;

  case 3:
    _TS_UL_X = TS_UL_SHORT;
    _TS_UL_Y = TS_OFFSET - TS_LR_LONG;
    _TS_LR_X = TS_LR_SHORT;
    _TS_LR_Y = TS_OFFSET - TS_UL_LONG;
    break;

  }

  pixelsX = _tft->width();
  pixelsY = _tft->height();
}

/**************************************************************************/
void TS_ILI9341_map::mapTS_to_TFT(int16_t TSx, int16_t TSy, int16_t* x,
    int16_t* y) {
  *x = map(TSx, _TS_UL_X, _TS_LR_X, 0, pixelsX);
  *y = map(TSy, _TS_UL_Y, _TS_LR_Y, 0, pixelsY);
}

/**************************************************************************/
void TS_ILI9341_map::mapTFT_to_TS(int16_t x, int16_t y, int16_t* TSx,
    int16_t* TSy) {
  *TSx = map(x, 0, pixelsX, _TS_UL_X, _TS_LR_X);
  *TSy = map(y, 0, pixelsY, _TS_UL_Y, _TS_LR_Y);
}
/**************************************************************************/
void TS_ILI9341_map::GetCalibration_UL_LR(int16_t pixelOffset, int16_t* x_UL,
    int16_t* y_UL, int16_t* x_LR, int16_t* y_LR) {
  *x_UL = pixelOffset;
  *y_UL = pixelOffset;
  *x_LR = pixelsX - pixelOffset - 1;
  *y_LR = pixelsY - pixelOffset - 1;
}

/**************************************************************************/
void TS_ILI9341_map::findTS_calibration(int16_t x_UL, int16_t y_UL,
    int16_t x_LR, int16_t y_LR, int16_t TSx_UL, int16_t TSy_UL, int16_t TSx_LR,
    int16_t TSy_LR, int16_t* TS_LR_X, int16_t* TS_LR_Y, int16_t* TS_UL_X,
    int16_t* TS_UL_Y) {

  float sx = (float)(TSx_LR - TSx_UL)/(x_LR - x_UL);
  float sy = (float)(TSy_LR - TSy_UL)/(y_LR - y_UL);

  *TS_UL_X = (int16_t) (TSx_UL + (0 - x_UL)*sx);
  *TS_LR_X = (int16_t) (TSx_UL + (pixelsX - x_UL)*sx);
  *TS_UL_Y = (int16_t) (TSy_UL + (0 - y_UL)*sy);
  *TS_LR_Y = (int16_t) (TSy_UL + (pixelsY - y_UL)*sy);
}

// -------------------------------------------------------------------------
