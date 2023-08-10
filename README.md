# XPT2046_Touchscreen_TT

## Changes to Paul Stoffregen's XPT_2046_Touchscreen with function for setting thresholds

*XPT2046_Touchscreen_TT* is an Arduino C++ library that supports the XPT2046 resistive touchscreen controllers used on many low cost pixel-based displays. It is a copy of the library made by Paul Stoffregen named *XPT_2046_Touchscreen*, with new functions and files added to support **touch and release events, mapping touchscreen coordinates to display coordinates, calibrating that mapping, and changing the touch pressure threshold.**

## List of changes to Paul Stoffregen's version of the library

**1.** Add new code files *TS_Display.h* and *.cpp*, and new example program files *TS_DisplayEvents.ino and TS_DisplayCalibrate.ino*, to support **touch and release events, mapping touchscreen coordinates to/from display coordinates, and calibrating the touchscreen-to-display mapping**.

**2.** New function *setThresholds()* allows **dynamically setting the pressure thresholds** previously hard-coded as Z_THRESHOLD and Z_THRESHOLD_INT, now stored in variables Z_Threshold and Z_Threshold_Int. (The defines of Z_THRESHOLD and Z_THRESHOLD_INT were moved from *XPT2046_Touchscreen_TT.cpp* to *XPT2046_Touchscreen_TT.h*.)

**3.** New functions *Zthreshold()* (returns the pressure threshold Z_Threshold) and *Zthreshold_Int()* (returns the pressure threshold Z_Threshold_Int).

**4.** Function header comments in Doxygen style have been added to the XPT2046_Touchscreen_TT.h file.

**5.** Example programs have been changed:

> Use *Adafruit_GFX_Library* and *Adafruit_ILI9341* libraries in place of *ILI9341_t3* library.
>
> Use *Adafruit_GFX_library* *FreeSans12pt7b* font in place of Arial fonts from *ILI9341_t3* library.
>
> The example program *ILI9341Test.ino* is renamed to *TS_DisplayTest.ino* and it now uses new files *TS_Display.h/.cpp* to display a "+" on the display at the tapped point, and it doesn't repeatedly write data to monitor and display, but only during a tap or untap.
>
> Set serial monitor speed to 115,200 bps instead of 38,400 bps.
>
> Change pin assignments to match my own system and add usage comment reminding user to set the constants for his system.
>
> Use longer names for _PIN #defines.
>
> Allocate display and touchscreen objects with *new*.
>
> Adjust Serial initialization to start up better.

## Adding basic touchscreen functionality to your project

### Include files

At the start of your project .ino file, #include *XPT2046_Touchscreen_TT.h*:

```
#include <XPT2046_Touchscreen_TT.h>
```

Including the above file also causes the following to automatically be included:

```
#include <SPI.h>
```

### Define pins

Next, near the start of your project .ino file, use #define statements to define the pins used by the touchscreen in your system. Define pins for the touch controller device CS (chip select) and IRQ (interrupt request) signals. You probably don't need to define values for the MOSI, MISO, and SCK pins because these are standardized on Arduino processors. For example:

```
#define TOUCH_CS_PIN  10
#define TOUCH_IRQ_PIN A7
// TOUCH_MOSI_PIN=11, TOUCH_MISO_PIN=12, TOUCH_SCK_PIN=13
```

### Create a file-level variable that will point to the touchscreen object

Before the start of function definitions in your project .ino file, define a variable that points to a XPT2046_Touchscreen object. The object will be created in setup() and assigned to this pointer variable. For example:

```
// Pointer to touchscreen object.
XPT2046_Touchscreen* ts;
```

### Initialize touchscreen in setup()

In your Arduino setup() function, use *new* to allocate a touchscreen object and assign it to your pointer variable, call the object's begin() function to initialize it, and call additional object functions as needed to configure it. For example:

```
  // Allocate the touchscreen object, then initialize it.
  ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);  // Param 2 - NULL - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, 255);  // Param 2 - 255 - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, TOUCH_IRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling
  ts->begin();
  ts->setRotation(1);
  ts->setThresholds(Z_THRESHOLD/3); // Change the pressure threshold if needed.
```

Three ways to allocate the object are shown in the comments above, depending on whether or not your want to use the IRQ pin and interrupts to improve touchscreen performance and reduce the time required when touchscreen functions are called to test for presses. The first two methods shown are equivalent, while the third uses interrupts (recommended method).

The *begin()* function initializes the touchscreen object and device controller, the *setRotation()* function indicates what orientation the touchscreen is in (0-3 for north, east, south, west, and this must match the rotation setting for the display), and setThresholds() is optional and allows you to alter the pressure threshold for detecting a press.

The begin() function can also specify communication on a different SPI
port.

```
  ts.begin(SPI1); // use SPI1 rather than SPI
```

### Reading touch information

A later section will describe the easiest way to interact with the touch screen when using a display, with touch and release events. However, you can always interact at a lower level by reading the touch information at any time.

The touched() function tells if the display is currently being touched, returning true or false:

```
  if (ts.touched()) {
    // do something....
  }
```

You can read the touch coordinates with readData():

```
  uint16_t x, y, z;
  ts.readData(&x, &y, &z);
````

or with getPoint(), which returns a TS_Point object:

```
  TS_Point p = ts.getPoint();
  Serial.print("x = ");
  Serial.print(p.x);
  Serial.print(", y = ");
  Serial.print(p.y);
  Serial.print(", z = ");
  Serial.println(p.z);
```

The Z-coordinate represents the amount of pressure applied to the screen.

## Adding mapping between touchscreen and display

The pair of files *TS_Display.h* and *.cpp* provide touch and release event services for responding to touch and release events, and mapping services to map between touchscreen coordinates and display coordinates. This library requires the use of additional graphics library *Adafruit_GFX_Library* to support the display, and the display controller interface must be a C++ class derived from that library's class *Adafruit_GFX*. For example, popular displays that use an ILI9341 controller can use the library *Adafruit_ILI9341*, which has a class by the same name that is derived from *Adafruit_GFX* and so will work with this library. If you want to use the touchscreen this way and you haven't done so already, add those libraries to your Arduino IDE.

Touch and release events, which are the easiest way to interact with the touchscreen and display, are described in a later section. However, you can always use the mapping functions independently to map coordinates between the touchscreen and display, as follows.

### Include files

At the start of your project .ino file, include the header file *TS_Display.h*:

```
#include <TS_Display.h>
```

Including the above file also causes the following to automatically be included, so you can skip including these, although it won't hurt to do so:

```
#include <SPI.h>
#include <XPT2046_Touchscreen_TT.h>
#include <Adafruit_GFX.h>
```

Also include the header file for the display controller class that is derived from class Adafruit_GFX, typically *Adafruit_ILI9341.h*:

```
#include <Adafruit_ILI9341.h>
```

If you use a display that has a different (compatible) library, you simply replace *Adafruit_ILI9341* with it. The sample programs that use *TS_Display.h* all use the *Adafruit_ILI9341* library, but if you have a different library, just change the three occurrences of *Adafruit_ILI9341* in the sample program to your library name, that is all that is required to switch to a different type of display. A library is compatible when its C++ class is derived from class *Adafruit_GFX*.

### Define pins

Next, near the start of your project .ino file, use #define statements to define the pins used by the touchscreen and display in your system.

For the touchscreen, define the pins used by the touch controller CS and IRQ signals. For example:

```
#define TOUCH_CS_PIN  10
#define TOUCH_IRQ_PIN A7
// TOUCH_MOSI_PIN=11, TOUCH_MISO_PIN=12, TOUCH_SCK_PIN=13
```

For the display, define the pins used by the display controller CS and DC signals and the digital output used to control the backlight LED which is typically present on most displays. For example:

```
#define LCD_CS_PIN    10
#define LCD_DC_PIN    2
#define LCD_LED_PIN   A2
```

### Create file-level variables for the touchscreen, display, and touchscreen/display objects

Before the start of function definitions in your project .ino file, define three pointer variables that point to XPT2046_Touchscreen, TS_Display, and  display controller (typically an Adafruit_ILI9341 instance) objects. The objects will be created in setup() and assigned to these pointer variables. For example:

```
// Pointer to display object.
Adafruit_ILI9341* lcd;
// Pointer to touchscreen object.
XPT2046_Touchscreen* ts;
// Pointer to touchscreen-display object.
TS_Display* ts_display;
```

### Initialization in setup()

In your Arduino setup() function, use *new* to allocate the three objects and assign them to the pointer variables, then call the begin() functions for the objects to initialize them, and call additional object functions as needed to configure them. For example:

```
  // Allocate and initialize the display object.
  lcd = new Adafruit_ILI9341(LCD_CS_PIN, LCD_DC_PIN);
  pinMode(LCD_LED_PIN, OUTPUT);
  digitalWrite(LCD_LED_PIN, LOW); // Backlight on.
  lcd->begin();
  lcd->setRotation(2);   // upside-down portrait mode

  // Allocate and initialize the touchscreen object.
  ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, TOUCH_IRQ_PIN);
  ts->begin();
  ts->setRotation(lcd->getRotation()); // Use same rotation as display.

  // Allocate and initialize the touchscreen-display object.
  ts_display = new TS_Display();
  ts_display->begin(ts, lcd);

  // Additional display function calls would appear below to create the initial display screen.
```

It is recommended you follow the above order, initializing the display *before* the touchscreen, with the TS_Display object initialization coming last.

After the above initializations, you will also want to call display functions to create the initial screen on the display.

### Getting touch display coordinates with getPoint() and mapTStoDisplay()

There are two ways to monitor for screen touches and map them to display coordinates. The harder way is shown in this section, by using the *getPoint()* and *mapTStoDisplay()* functions. The next section shows the easier way using touch and release events.

Initialization of the touchscreen is as shown above. To monitor for touch events in your Arduino *loop()* function, first define a global *boolean* variable *wasTouched* to keep track of whether or not the screen was touched but the touch has not yet been released (true) or has been released (false).

```
boolean wasTouched = false;
```

Next, in the Arduino standard *loop()* function, call the touchscreen *touched()* function to test for a screen touch. If a touch occurred, ignore it if *wasTouched* is true (because the touch was already handled on a previous pass through the loop). If *wasTouched* is false, this is the first time you've seen this touch, so handle it. First, call the *getPoint()* function to get the touched point coordinates, and then pass those coordinates to the *TS_Display* class function *mapTStoDisplay()* to map them to display coordinates. You are then ready to use those coordinates to decide what you should do based on a user screen tap at that (x, y) position. For example:

```
void loop() {
  boolean isTouched = ts->touched();
  if (isTouched) {
    // Handle only if it wasn't previously touched.
    if (!wasTouched) {
      // Get the touched point.
      TS_Point p = ts->getPoint();
      // Map it to display coordinates (x,y).
      int16_t x, y;
      ts_display->mapTStoDisplay(p.x, p.y, &x, &y);
      // Here you can use (x,y) to either plot something at that position on the
      // display or perhaps check to see if (x,y) lies within something you've
      // displayed, such as a button.
    }
  } else {
    // Handle release of a touch here.
    if (wasTouched) {
      // Here you can do anything you might need to do when the user releases a screen tap.
      // There may be nothing to do, or perhaps you will choose to display something when he releases the tap.
    }
  }

  // Set wasTouched for next time through the loop.
  wasTouched = isTouched;
}
```

## Using touch and release events

The easiest way to monitor touchscreen touches (and the end of the touch, also called a release) is to use touch and release events, supported by the TS_Display class, of which the touchscreen-display object *(ts_display)* is an instance.

Initialization is as shown in the previous sections above, including initializing the display *(lcd)*, touchscreen *(ts)*, and touchscreen-display objects *(ts_display)*. Then, in the standard Arduino *loop()* function, call the *ts_display* object's *getTouchEvent()* function to test for a touch or release event and handle it accordingly. For example:

```
void loop() {
  int16_t x, y, pres, px, py;
  eTouchEvent touchEvent = ts_display->getTouchEvent(x, y, pres, &px, &py);

  switch (touchEvent) {

  case TS_TOUCH_EVENT:
    // TOUCH EVENT at (x, y), pressure=pres, touchscreen at (px, py)
    // Here you would do whatever you might need to do when the user taps the (x,y) position on the screen.
    break;

  case TS_RELEASE_EVENT:
    // RELEASE EVENT, you would usually ignore x, y, pres, px, and py.
    // Here you would do whatever you might need to do when the user releases the tap.
    break;

  }
}
```

## Calibrating the touchscreen

The class *TS_Display* introduced above also includes functions for calibrating the relationship between touchscreen coordinates and display coordinates. Although the default calibration is okay, it isn't as ideal as it could be. Touchscreens seem to vary a bit from one to another, and the different rotations also behave differently.

An example program (*TS_DisplayCalibrate.ino*) is provided that illustrates calibration of the touchscreen. It also shows how to save the calibration values in EEPROM so they can be retrieved following a power-down and powering back up. It is described in a following section.

## Example programs

Five example programs are provided in the library's *examples* subfolder. All of these programs require that you set #define values near the start of the file to define the pin numbers connected to the touchscreen and, in some programs, to the display.

### TouchTest.ino

Example program *TouchTest.ino* assumes only a touchscreen with XPT2346 controller for hardware. It initializes the touchscreen software and controller, then waits for touches and displays their coordinates on the IDE serial monitor window.

### TouchTestIRQ.ino

Example program *TouchTestIRQ.ino* is just like *TouchTest.ino* except it uses interrupts whereas the latter does not.

### TS_DisplayTest.ino

Example program *TS_DisplayTest.ino* assumes a touchscreen with XPT2346 controller connected to a display with an ILI9341 controller. It initializes the display and the touchscreen software and controllers, then waits for touches, displays their coordinates on the screen and in the IDE serial monitor window, and draws a green "+" at the display position to which the touched position maps (using the class *TS_Display* to map the points).

### TS_DisplayEvents.ino

Example program *TS_DisplayEvents.ino* illustrates how to use the simple touch and release event service of class *TS_Display* to interact with the user's screen touches.

### TS_DisplayCalibrate.ino

Example program *TS_DisplayCalibrate.ino* illustrates how to provide a user with a screen that lets him calibrate the relationship between the touchscreen coordinates and the display coordinates. It also assumes a touchscreen with XPT2346 controller connected to a display with an ILI9341 controller. If you are using SAMD architecture, it uses the FlashStorage_SAMD library to store calibration data permanently across power-downs. It initializes the display and the touchscreen software and controllers and the touchscreen-display object, then waits for two touches, displays their coordinates on the screen and in the IDE serial monitor window, and computes new calibration parameters and displays them on the serial monitor. If the SAMD architecture is being used, it writes the calibration parameters to EEPROM non-volatile memory. They are read back when the program is restarted, and used as the initial calibration setting.

You can also use the calibration program to determine the ideal calibration parameters for your touchscreen and its rotation. You can then call *ts_display->setTS_calibration()* with those ideal parameters, after calling *ts_display->begin()* in *setup()*.

You can model your own calibration screen after the one shown in this example program. If you do not have the SAMD architecture, this is a problem, because you don't want to force the user to calibrate the screen at every startup. Instead, you want to store the calibration parameters in non-volatile memory and retrieve them at startup. (A calibration screen would only be displayed when the user invoked it.) There are other non-volatile memory libraries available, and you will want to choose one and make the necessary changes to the example program to use it.

## Adafruit Library Compatibility

XPT2046_Touchscreen is meant to be compatible with sketches written for Adafruit_STMPE610, offering the same functions, parameters and numerical ranges as Adafruit's library. It is also meant to be compatible with any display using the Adafruit_GFX library.

## Using The Interrupt Pin

When using interrupts as specified above, there is built-in support when connected, nothing else is needed.

When interrupts are used, no SPI calls are made unless a touch was detected, which increases program efficiency.

Pay attention to the following warnings should you decide to do a custom interrupt setup with the touchscreen.

The XPT2046 chip has an interrupt output, which is typically labeled T_IRQ on many low cost TFT displays. No special software support is needed in this library. The interrupt pin always outputs a digital signal related to the touch controller signals, which is LOW when the display is touched. It also is driven low while software reads the touch position.

The interrupt can be used as a wakeup signal, if you put your microcontroller into a deep sleep mode. Normally, you would stop reading the touch data, then enable the interrupt pin with attachInterrupt(), and then configure your processor to wake when the interrupt occurs, before enter a deep sleep mode. Upon waking, you would normally disable the interrupt before reading the display, to prevent false interrupts caused by the process of reading touch positions.

You can also use the interrupt to respond to touch events.  Setup might look similar to this:

```
  SPI.usingInterrupt(digitalPinToInterrupt(pin))
  attachInterrupt(digitalPinToInterrupt(pin), myFunction, FALLING);
```

However, inside your interrupt function, if the display is no longer being touched, any attempt to read the touch position will cause the interrupt pin to create another falling edge. This can lead to an infinite loop of falsely triggered interrupts. Special care is needed to avoid triggering more interrupts on the low signal due to reading the touch position.

For most applications, regularly reading the touch position from the main program is much simpler.

## Contact

There are the two GitHub repositories related to this project:

**ORIGINAL Paul Stoffregen repository:** https://github.com/PaulStoffregen/XPT2046_Touchscreen

**MY (Ted Toal) repository for code described herein:** https://github.com/tedtoal/XPT2046_Touchscreen_TT

Paul's email is listed in the code files as: *paul@pjrc.com*

My email is: *ted@tedtoal.net*

