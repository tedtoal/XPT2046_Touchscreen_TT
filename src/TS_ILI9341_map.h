/*
  TS_ILI9341_map.h - Support mapping coordinates between an XPT2046-controlled
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


  This file defines C++ class TS_ILI9341_map, which has member functions for
  mapping coordinates between an XPT2046-controlled touchscreen and an
  ILI9341-controlled TFT LCD display.

  This class provides two kinds of functions:
   1. Functions to map between touchscreen and TFT display coordinates
   2. Functions to get and set calibration parameters that control that mapping

  The first of the above function kinds is useful in any program that makes use
  of an ILI9341-controlled TFT display and XPT2046-controlled touchscreen, for
  converting between the coordinates used by those two devices.

  The second of the above function kinds can be used together with user GUI code
  to provide a screen where the user can touch two opposite corners of the
  display, so the touchscreen coordinates of those points can be used to
  calibrate the mapping. Although the default mapping parameter values work well
  in most cases, calibration may be desirable because it seems that factory
  calibration is not highly accurate.

  The TS_ILI9341_map class stores the current calibrated values of these
  parameters in class variables. See example program ILI9341Calibrate.ino for
  code that uses both types of functions and performs calibration, storing the
  calibration values in EEPROM memory.

  This class assumes that the screen rotation is fixed at one of the four
  rotation values. The touchscreen must be set to the same rotation as the TFT
  display, using their setRotation() functions.

  Rotation 0 is normal upright portrait orientation, 1 is normal upright
  landscape orientation 90° CCW from that, then 2 is an additional 90° CCW and
  is upside-down portrait orientation, and 3 is an additional 90° CCW and is
  upside-down landscape orientation.
*/
/**************************************************************************/

#ifndef TS_ILI9341_map_h
#define TS_ILI9341_map_h

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen_TT.h>

class TS_ILI9341_map {

  protected:

  // The touchscreen object associated with the class instance using begin().
  XPT2046_Touchscreen* _ts;

  // The TFT display object associated with the class instance using begin().
  Adafruit_ILI9341* _tft;

  // The four _TS_ variables below are the calibration parameters, used to map
  // between touchscreen and TFT display coordinates. The values are actually
  // the minimum and maximum x- and y-coordinates returned by the touchscreen's
  // getPoint() function.
  //
  // The "UL" in the variable names below means upper-left, and "LR" means
  // lower-right.
  //
  // Regardless of what the rotation is set to, the upper-left corner of the
  // display (in its specified rotation/orientation) produces the LARGEST values
  // for the touchscreen x- and y- coordinates, while the lower-right corner of
  // the display produces the SMALLEST values, just the opposite of what you
  // would expect. Therefore, the TS_UL_ constants will have larger values than
  // the TS_LR_ constants.
  //
  // The INITIAL value of these parameters when an instance of this class is
  // first created are set to reasonable values that should work fairly well in
  // most cases. Those initial values are DIFFERENT depending on screen
  // orientation.
  int16_t _TS_UL_X, _TS_UL_Y, _TS_LR_X, _TS_LR_Y;

  // TFT display screen size in pixel (varies depending on rotation).
  int16_t pixelsX;
  int16_t pixelsY;

  public:

  /**************************************************************************/
  /*!
    @brief  Constructor.
  */
  /**************************************************************************/
  TS_ILI9341_map() : _ts(nullptr), _tft(nullptr), _TS_LR_X(0), _TS_LR_Y(0),
      _TS_UL_X(0), _TS_UL_Y(0), pixelsX(0), pixelsY(0) {}

  /**************************************************************************/
  /*!
    @brief  Class instance initialization function.
    @param  ts    Pointer to the instance of the touchscreen object.
    @param  tft   Pointer to the instance of the TFT LCD display object.
    @note         Calibration parameters are reset to their default values.
    @note         pixelsX and pixelsY are set appropriately.
  */
  /**************************************************************************/
  void begin(XPT2046_Touchscreen* ts, Adafruit_ILI9341* tft);

  /**************************************************************************/
  /*!
    @brief  Return the current calibration parameter values.
    @param  TS_LR_X    Pointer to variable to receive TS_LR_X value.
    @param  TS_LR_Y    Pointer to variable to receive TS_LR_Y value.
    @param  TS_UL_X    Pointer to variable to receive TS_UL_X value.
    @param  TS_UL_Y    Pointer to variable to receive TS_UL_Y value.
  */
  /**************************************************************************/
  void getTS_calibration(int16_t* TS_LR_X, int16_t* TS_LR_Y, int16_t* TS_UL_X,
      int16_t* TS_UL_Y) {
    *TS_LR_X = _TS_LR_X;
    *TS_LR_Y = _TS_LR_Y;
    *TS_UL_X = _TS_UL_X;
    *TS_UL_Y = _TS_UL_Y;
  }

  /**************************************************************************/
  /*!
    @brief  Set the current calibration parameter values.
    @param  TS_LR_X    New TS_LR_X value.
    @param  TS_LR_Y    New TS_LR_Y value.
    @param  TS_UL_X    New TS_UL_X value.
    @param  TS_UL_Y    New TS_UL_Y value.
  */
  /**************************************************************************/
  void setTS_calibration(int16_t TS_LR_X, int16_t TS_LR_Y, int16_t TS_UL_X,
      int16_t TS_UL_Y) {
    _TS_LR_X = TS_LR_X;
    _TS_LR_Y = TS_LR_Y;
    _TS_UL_X = TS_UL_X;
    _TS_UL_Y = TS_UL_Y;
  }

  /**************************************************************************/
  /*!
    @brief  Map a touchscreen point (TSx, TSy) to a TFT display point (x, y).
    @param  TSx   Touchscreen x-coordinate to map.
    @param  TSy   Touchscreen y-coordinate to map.
    @param  x     Pointer to variable to receive TFT display x-coordinate.
    @param  y     Pointer to variable to receive TFT display y-coordinate.
    @note   Mapping depends on the screen rotation, assumed to be fixed.
  */
  /**************************************************************************/
  void mapTS_to_TFT(int16_t TSx, int16_t TSy, int16_t* x, int16_t* y);

  /**************************************************************************/
  /*!
    @brief  Reverse map a TFT point (x, y) to a touchscreen point (TSx, TSy).
    @param  x     TFT display x-coordinate to map.
    @param  y     TFT display y-coordinate to map.
    @param  TSx   Pointer to variable to receive touchscreen x-coordinate.
    @param  TSy   Pointer to variable to receive touchscreen y-coordinate.
    @note   Mapping depends on the screen rotation, assumed to be fixed.
  */
  /**************************************************************************/
  void mapTFT_to_TS(int16_t x, int16_t y, int16_t* TSx, int16_t* TSy);

  /**************************************************************************/
  /*!
    @brief  Compute 2 TFT display coordinate pairs located near the upper-left
            and lower-right corners of the screen, to be used for displaying
            graphics (such as a "+" sign) to mark points the user is to click
            to calibrate the touchscreen.
    @param  pixelOffset   Number of pixels from edge of display at which the
                          returned points should be located.
    @param  x_UL          Pointer to variable to receive upper-left x-coord.
    @param  y_UL          Pointer to variable to receive upper-left y-coord.
    @param  x_LR          Pointer to variable to receive lower-right x-coord.
    @param  y_LR          Pointer to variable to receive lower-right y-coord.
    @note   The 2 TFT display points need not be located the same distance from
            the display edge or at upper-left and lower-right. Actually, any two
            points can be used. Accurancy is increased, though, by separating
            them as much as possible.
  */
  /**************************************************************************/
  void GetCalibration_UL_LR(int16_t pixelOffset, int16_t* x_UL, int16_t* y_UL,
    int16_t* x_LR, int16_t* y_LR);

  /**************************************************************************/
  /*!
    @brief  Use 2 touchscreen coordinate pairs at specified TFT display
            coordinates to compute new touchscreen calibration parameter values.
    @param  x_UL      TFT display upper-left x-coord.
    @param  y_UL      TFT display upper-left y-coord.
    @param  x_LR      TFT display lower-right x-coord.
    @param  y_LR      TFT display lower-right y-coord.
    @param  TSx_UL    Corresponding touchscreen upper-left x-coord.
    @param  TSy_UL    Corresponding touchscreen upper-left y-coord.
    @param  TSx_LR    Corresponding touchscreen lower-right x-coord.
    @param  TSy_LR    Corresponding touchscreen lower-right y-coord.
    @param  TS_LR_X    Pointer to variable to receive new TS_LR_X value.
    @param  TS_LR_Y    Pointer to variable to receive new TS_LR_Y value.
    @param  TS_UL_X    Pointer to variable to receive new TS_UL_X value.
    @param  TS_UL_Y    Pointer to variable to receive new TS_UL_Y value.
    @note   The returned calibration values are NOT set as the current values.
            Call setTS_calibration() to do that.
    @note   The points need not be located at upper-left and lower-right, but
            accurancy is increased by separating them as much as possible.
    @note   The particular computations used depend on screen rotation.
  */
  /**************************************************************************/
  void findTS_calibration(int16_t x_UL, int16_t y_UL, int16_t x_LR,
    int16_t y_LR, int16_t TSx_UL, int16_t TSy_UL, int16_t TSx_LR,
    int16_t TSy_LR, int16_t* TS_LR_X, int16_t* TS_LR_Y, int16_t* TS_UL_X,
    int16_t* TS_UL_Y);
};

#endif // TS_ILI9341_map_h
