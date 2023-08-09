/*
  TS_Display.cpp - Support mapping coordinates between an XPT2046-controlled
  touchscreen and a pixel-based display, with calibration.
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
#include <TS_Display.h>

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
void TS_Display::begin(XPT2046_Touchscreen* ts, Adafruit_GFX* disp) {
  _ts = ts;
  _disp = disp;

  switch (_disp->getRotation()) {

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

  _debounceMS_TR = DEF_DEBOUNCE_MS_TR;
  _minTouchPres = DEF_MIN_TOUCH_PRES;
  _maxReleasePres = DEF_MAX_RELEASE_PRES;

  _lastEventWasTouch = false;
  _msTime = millis();

  _pixelsX = _disp->width();
  _pixelsY = _disp->height();
}

/**************************************************************************/
eTouchEvent TS_Display::getTouchEvent(int16_t& x, int16_t& y, int16_t& pres,
    int16_t* px, int16_t* py) {

  eTouchEvent ret = TS_UNCERTAIN;
  TS_Point p = _ts->getPoint();

  if (px != NULL)
    *px = p.x;
  if (py != NULL)
    *py = p.y;

  mapTStoDisplay(p.x, p.y, &x, &y);
  pres = p.z;

  bool currentTSeventIsTouch = _lastEventWasTouch;
  if (pres >= _minTouchPres) {
    currentTSeventIsTouch = true;
    ret = TS_TOUCH_PRESENT;
  } else if (pres <= _maxReleasePres) {
    currentTSeventIsTouch = false;
    ret = TS_NO_TOUCH;
  }

  // If no change from last detected event, restart debounce timer.
  if (_lastEventWasTouch == currentTSeventIsTouch) {
    _msTime = millis();
    return(ret);
  }

  // A change since the last event has occurred, don't register it until debounce timer has expired.
  if (millis() - _msTime < _debounceMS_TR)
    return(ret);

  // Event occurred and debounce time expired.  Restart debounce timer for timing the opposite event.
  _msTime = millis();
  _lastEventWasTouch = currentTSeventIsTouch;
  return(currentTSeventIsTouch ? TS_TOUCH_EVENT : TS_RELEASE_EVENT);
}

/**************************************************************************/
void TS_Display::mapTStoDisplay(int16_t TSx, int16_t TSy, int16_t* x,
    int16_t* y) {
  *x = map(TSx, _TS_UL_X, _TS_LR_X, 0, _pixelsX);
  *y = map(TSy, _TS_UL_Y, _TS_LR_Y, 0, _pixelsY);
}

/**************************************************************************/
void TS_Display::mapDisplayToTS(int16_t x, int16_t y, int16_t* TSx,
    int16_t* TSy) {
  *TSx = map(x, 0, _pixelsX, _TS_UL_X, _TS_LR_X);
  *TSy = map(y, 0, _pixelsY, _TS_UL_Y, _TS_LR_Y);
}
/**************************************************************************/
void TS_Display::GetCalibration_UL_LR(int16_t pixelOffset, int16_t* x_UL,
    int16_t* y_UL, int16_t* x_LR, int16_t* y_LR) {
  *x_UL = pixelOffset;
  *y_UL = pixelOffset;
  *x_LR = _pixelsX - pixelOffset - 1;
  *y_LR = _pixelsY - pixelOffset - 1;
}

/**************************************************************************/
void TS_Display::findTS_calibration(int16_t x_UL, int16_t y_UL, int16_t x_LR,
    int16_t y_LR, int16_t TSx_UL, int16_t TSy_UL, int16_t TSx_LR,
    int16_t TSy_LR, int16_t* TS_LR_X, int16_t* TS_LR_Y, int16_t* TS_UL_X,
    int16_t* TS_UL_Y) {

  float sx = (float)(TSx_LR - TSx_UL)/(x_LR - x_UL);
  float sy = (float)(TSy_LR - TSy_UL)/(y_LR - y_UL);

  *TS_UL_X = (int16_t) (TSx_UL + (0 - x_UL)*sx);
  *TS_LR_X = (int16_t) (TSx_UL + (_pixelsX - x_UL)*sx);
  *TS_UL_Y = (int16_t) (TSy_UL + (0 - y_UL)*sy);
  *TS_LR_Y = (int16_t) (TSy_UL + (_pixelsY - y_UL)*sy);
}

// -------------------------------------------------------------------------
