/*
  TS_Display.h - Defines C++ class TS_Display, which supports systems with an
  XPT2046-controlled touchscreen and a pixel-based display, with functions for
  monitoring for touch events, mapping coordinates between touchscreen and
  display, and touchscreen/display mapping calibration.
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


  This file defines C++ class TS_Display, useful in systems with an
  XPT2046-controlled touchscreen and a pixel-based display. The only requirement
  for the display is that it is controlled by an instance of a class that is
  derived from the graphics support class Adafruit_GFX. The TS_disp class has
  member functions for generating touch and release events, mapping coordinates
  between touchscreen and display, and calibrating the mapping.

  This class provides three kinds of functions:
    1. Functions to map between touchscreen and display coordinates
    2. Functions to test for touch and release events and set touch pressure
        thresholds for them.
    3. Functions to get and set calibration parameters that control that mapping

  The first of the above is useful in any program that makes use of a
  pixel-based display and XPT2046-controlled touchscreen, for converting between
  the coordinates used by those two devices.

  The second of the above adds additional functionality to simple tests for
  touches of the screen, providing "debouncing" and informing the user code of
  touch events and release events and the display and touchscreen coordinates.

  The third of the above can be used together with user GUI code to provide a
  screen where the user can touch two opposite corners of the display, so the
  touchscreen coordinates of those points can be used to calibrate the mapping.
  Although the default mapping parameter values work well in most cases,
  calibration may be desirable because it seems that factory calibration is not
  highly accurate.

  The TS_Display class stores the current calibrated values of these parameters
  in class variables. See example program TS_DisplayCalibrate.ino for code that
  uses both types of functions and performs calibration, storing the calibration
  values in EEPROM memory.

  This class assumes that the screen rotation is fixed at one of the four
  rotation values. The touchscreen must be set to the same rotation as the
  display, using their setRotation() functions.

  Rotation 0 is normal upright portrait orientation, 1 is normal upright
  landscape orientation 90° CCW from that, then 2 is an additional 90° CCW and
  is upside-down portrait orientation, and 3 is an additional 90° CCW and is
  upside-down landscape orientation.
*/
/**************************************************************************/

#ifndef ILI9341_h
#define ILI9341_h

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <XPT2046_Touchscreen_TT.h>

// Default milliseconds of touch before touch recognized, or absence of touch
// before release recognized.
#define DEF_DEBOUNCE_MS_TR  20

// Default minimum pressure for touch event, maximum for release event.
#define DEF_MIN_TOUCH_PRES    5
#define DEF_MAX_RELEASE_PRES  0

/**************************************************************************/
/*!
  @brief    Enum eTouchEvent has five TS_ constants, two for touch and release
            events and three for touched, released, or ambiguous states.

  @param  TS_UNCERTAIN
*/
/**************************************************************************/
typedef enum _eTouchEvent {
  TS_UNCERTAIN,       /*! State, not event: ambiguous, maybe touched, maybe not. */
  TS_NO_TOUCH,        /*! State, not event: screen not being touched. */
  TS_TOUCH_PRESENT,   /*! State, not event: screen is being touched. */
  TS_TOUCH_EVENT,     /*! Event: debounced touch, next event will be release. */
  TS_RELEASE_EVENT    /*! Event: debounced release, next event will be touch. */
} eTouchEvent;

/**************************************************************************/
/*!
  @brief    Class TS_Display manages the interface between a touchscreen
            controlled by an XPT_2046 controller and a pixel-based display
            whose controller is an instance of a class derived from the
            graphics class Adafruit_GFX. The class provides functions for
            mapping coordinates, generating touch and release events, and
            mapping calibration.
*/
/**************************************************************************/
class TS_Display {

protected:

  // The touchscreen object associated with the class instance using begin().
  XPT2046_Touchscreen* _ts;

  // The display object associated with the class instance using begin().
  Adafruit_GFX* _disp;

  // The four _TS_ variables below are the calibration parameters, used to map
  // between touchscreen and display coordinates. The values are actually the
  // minimum and maximum x- and y-coordinates returned by the touchscreen's
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

  // Milliseconds of touch before touch recognized, or absence of touch before
  // release recognized.
  uint32_t _debounceMS_TR;

  // Minimum pressure for touch event, maximum for release event.
  int16_t _minTouchPres, _maxReleasePres;

private:

  // true if last touchscreen event was a touch event, false if release event.
  bool _lastEventWasTouch;

  // Timer for debouncing.
  uint32_t _msTime;

  // Display screen size in pixel (varies depending on rotation).
  int16_t _pixelsX;
  int16_t _pixelsY;

public:

  /**************************************************************************/
  /*!
    @brief  Constructor.
  */
  /**************************************************************************/
  TS_Display() : _ts(nullptr), _disp(nullptr), _TS_LR_X(0), _TS_LR_Y(0),
      _TS_UL_X(0), _TS_UL_Y(0), _debounceMS_TR(DEF_DEBOUNCE_MS_TR),
      _minTouchPres(DEF_MIN_TOUCH_PRES), _maxReleasePres(DEF_MAX_RELEASE_PRES),
      _lastEventWasTouch(false), _msTime(millis()), _pixelsX(0), _pixelsY(0) {}

  /**************************************************************************/
  /*!
    @brief  Class instance initialization function.
    @param  ts    Pointer to the instance of the touchscreen object.
    @param  disp  Pointer to the instance of the display object, which must be
                  an display controller class that has been derived from the
                  graphics support class Adafruit_GFX.
    @note         Calibration parameters are reset to their default values.
    @note         _pixelsX and _pixelsY are set appropriately.
  */
  /**************************************************************************/
  void begin(XPT2046_Touchscreen* ts, Adafruit_GFX* disp);

  /**************************************************************************/
  /*!
    @brief  Get current touchscreen state OR last touch or release event. An
            event TS_TOUCH_EVENT or TS_RELEASE_EVENT is returned one time only
            upon occurrence and debouncing of that event, and these alternate,
            a release event ALWAYS (eventually) following a touch event. If
            there is no event, the returned value indicates the current touch
            state (touch present, touch absent, or uncertain).
    @param  x     Reference to a variable in which to return the display
                  x-coordinate corresponding to current touch position if any.
    @param  y     Reference to a variable in which to return the display
                  y-coordinate corresponding to current touch position if any.
    @param  pres  Reference to a variable in which to return the current touch
                  pressure, 0 if none.
    @param  px    nullptr if not used, else a pointer to a variable to receive
                  the current touchscreen x-coordinate.
    @param  py    nullptr if not used, else a pointer to a variable to receive
                  the current touchscreen y-coordinate.
    @returns  A TS_ constant indicating a touch or release event if any, else
              indicating the current touch state.
    @note     A minimum time called the debounce time elapses before each touch
              or release event.
  */
  /**************************************************************************/
  eTouchEvent getTouchEvent(int16_t& x, int16_t& y, int16_t& pres,
    int16_t* px=nullptr, int16_t* py=nullptr);

  /**************************************************************************/
  /*!
    @brief  Set parameters for touch/release event detection.
    @param  debounceMS_TR       Number of milliseconds after start of a touch or
                                release before the event is generated.
    @param  minTouchPressure    Minimum pressure for a touch to be recognized.
    @param  maxReleasePressure  Maximum pressure for a release to be recognized.
    @note   A touch pressure > maxReleasePressure and < minTouchPressure is
            considered to be ambiguous and is the TS_UNCERTAIN state.
    @note   Generally the default values for these parameters should be okay.
  */
  /**************************************************************************/
  void setTouchReleaseParams(uint32_t debounceMS_TR, int16_t minTouchPressure,
      int16_t maxReleasePressure) {
    _debounceMS_TR = debounceMS_TR;
    _minTouchPres = minTouchPressure;
    _maxReleasePres = maxReleasePressure;
  }

  /**************************************************************************/
  /*!
    @brief  Map a touchscreen point (TSx, TSy) to a display point (x, y).
    @param  TSx   Touchscreen x-coordinate to map.
    @param  TSy   Touchscreen y-coordinate to map.
    @param  x     Pointer to variable to receive display x-coordinate.
    @param  y     Pointer to variable to receive display y-coordinate.
    @note   Mapping depends on the screen rotation, assumed to be fixed.
  */
  /**************************************************************************/
  void mapTStoDisplay(int16_t TSx, int16_t TSy, int16_t* x, int16_t* y);

  /**************************************************************************/
  /*!
    @brief  Reverse map a display point (x, y) to a touchscreen point (TSx, TSy).
    @param  x     Display x-coordinate to map.
    @param  y     Display y-coordinate to map.
    @param  TSx   Pointer to variable to receive touchscreen x-coordinate.
    @param  TSy   Pointer to variable to receive touchscreen y-coordinate.
    @note   Mapping depends on the screen rotation, assumed to be fixed.
  */
  /**************************************************************************/
  void mapDisplayToTS(int16_t x, int16_t y, int16_t* TSx, int16_t* TSy);

  /**************************************************************************/
  /*!
    @brief  Compute two display coordinate pairs located near the upper-left
            and lower-right corners of the screen, to be used for displaying
            graphics (such as a "+" sign) to mark points the user is to click
            to calibrate the touchscreen.
    @param  pixelOffset   Number of pixels from edge of display at which the
                          returned points should be located.
    @param  x_UL          Pointer to variable to receive upper-left x-coord.
    @param  y_UL          Pointer to variable to receive upper-left y-coord.
    @param  x_LR          Pointer to variable to receive lower-right x-coord.
    @param  y_LR          Pointer to variable to receive lower-right y-coord.
    @note   The two display points need not be located the same distance from
            the display edge or at upper-left and lower-right. Actually, any two
            points can be used. Accurancy is increased, though, by separating
            them as much as possible.
  */
  /**************************************************************************/
  void GetCalibration_UL_LR(int16_t pixelOffset, int16_t* x_UL, int16_t* y_UL,
    int16_t* x_LR, int16_t* y_LR);

  /**************************************************************************/
  /*!
    @brief  Use two touchscreen coordinate pairs at specified display
            coordinates to compute new touchscreen calibration parameter values.
    @param  x_UL      Display upper-left x-coord.
    @param  y_UL      Display upper-left y-coord.
    @param  x_LR      Display lower-right x-coord.
    @param  y_LR      Display lower-right y-coord.
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
};

#endif // ILI9341_h
