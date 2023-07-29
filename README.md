# XPT2046_Touchscreen_TT

## Changes to Paul Stoffregen's XPT_2046_Touchscreen with function for setting thresholds

*XPT2046_Touchscreen_TT* is an Arduino C++ library that supports the XPT2046 resistive touchscreen controllers used on many low cost TFT displays. It is a copy of the library made by Paul Stoffregen named *XPT_2046_Touchscreen*, with new functions and files added to support **mapping touchscreen coordinates to display coordinates, calibrating that mapping, and changing the touch pressure threshold.**

## List of changes to Paul Stoffregen's version of the library

**1.** Add new code files *TS_ILI9341_map.h* and *.cpp*, and new example program file *ILI9341Calibrate.ino*, to support **mapping touchscreen coordinates to/from TFT LCD display coordinates** and **provide support for calibrating the touchscreen-to-TFT mapping**.

**2.** New function *setThresholds()* allows **dynamically setting the pressure thresholds** previously hard-coded as Z_THRESHOLD and Z_THRESHOLD_INT, now stored in variables Z_Threshold and Z_Threshold_Int. (The defines of Z_THRESHOLD and Z_THRESHOLD_INT were moved from *XPT2046_Touchscreen_TT.cpp* to *XPT2046_Touchscreen_TT.h*.)

**3.** New functions *Zthreshold()* (returns the pressure threshold Z_Threshold) and *Zthreshold_Int()* (returns the pressure threshold Z_Threshold_Int).

**4.** Function header comments in Doxygen style have been added to the XPT2046_Touchscreen_TT.h file.

**5.** Example programs have been changed:

> Use *Adafruit-GFX-Library* and *Adafruit_ILI9341* libraries in place of *ILI9341_t3* library.
>
> Use *Adafruit-GFX-library* *FreeSans12pt7b* font in place of Arial fonts from *ILI9341_t3* library.
>
> The example program *ILI9341Test.ino* now uses new files *TS_ILI9341.h/.cpp* to display a "+" on the display at the tapped point, and it doesn't repeatedly write data to monitor and display, but only during a tap or untap.
>
> Set serial monitor speed to 115,200 bps instead of 38,400 bps.
>
> Change pin assignments to match my own system and add usage comment reminding user to set the constants for his system.
>
> Use longer names for _PIN #defines.
>
> Allocate tft and touchscreen objects with *new*.
>
> Adjust Serial initialization to start up better.

## Using XPT2036_Touchscreen in your project

### Include files

At the start of your project .ino file, where you #include other files, #include *XPT2046_Touchscreen_TT.h*:

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

The *begin()* function initializes the touchscreen object and device controller, the *setRotation()* function indicates what orientation the touchscreen is in (0-3 for north, east, south, west, and this must match the rotation setting for the ILI9341 library display when it is used), and setThresholds() is optional and allows you to alter the pressure threshold for detecting a press.

The begin() function can also specify communication on a different SPI
port.

```
  ts.begin(SPI1); // use SPI1 rather than SPI
```

### Reading touch information

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

## Using BOTH XPT2036_Touchscreen AND Adafruit_ILI9341 (TFT LCD display) in your project

The pair of files *TS_ILI9341_map.h* and *.cpp* provide mapping services to map between touchscreen coordinates and TFT LCD display coordinates. The display must use an ILI9341 controller, and the library requires the use of additional libraries *Adafruit_ILI9341* and *Adafruit-GFX-Library* to support the TFT display. If you want to use the touchscreen this way and you haven't done so already, add those libraries to your Arduino IDE.

### Include files

At the start of your project .ino file, where you #include other files, #include *TS_ILI9341_map.h*:

```
#include <TS_ILI9341_map.h>
```

Including the above file also causes the following to automatically be included:

```
#include <SPI.h>
#include <XPT2046_Touchscreen_TT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
```

### Define pins

Next, near the start of your project .ino file, use #define statements to define the pins used by the touchscreen and TFT display in your system.

For the touchscreen, define the pins used by the touch controller CS and IRQ signals. For example:

```
#define TOUCH_CS_PIN  10
#define TOUCH_IRQ_PIN A7
// TOUCH_MOSI_PIN=11, TOUCH_MISO_PIN=12, TOUCH_SCK_PIN=13
```

For the TFT display, define the pins used by the display controller CS and DC signals and the digital output used to control the LCD backlight LED. For example:

```
#define TFT_CS_PIN    10
#define TFT_DC_PIN    2
#define TFT_LED_PIN   A2
```

### Create file-level variables for the touchscreen, display, and touchscreen/display map objects

Before the start of function definitions in your project .ino file, define three pointer variables that point to XPT2046_Touchscreen, Adafruit_ILI9341, and TS_ILI9341_map objects. The objects will be created in setup() and assigned to these pointer variables. For example:

```
// Pointer to touchscreen object.
XPT2046_Touchscreen* ts;
// Pointer to TFT LCD display object.
Adafruit_ILI9341* tft;
// Pointer to touchscreen-TFT mapping object.
TS_ILI9341_map* tsmap;
```

### Initialization in setup()

In your Arduino setup() function, use *new* to allocate the three objects and assign them to the pointer variables, then call the begin() functions for the objects to initialize them, and call additional object functions as needed to configure them. For example:

```
  // Allocate and initialize the TFT display object.
  tft = new Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN);
  pinMode(TFT_LED_PIN, OUTPUT);
  digitalWrite(TFT_LED_PIN, LOW); // Backlight on.
  tft->begin();
  tft->setRotation(2);   // upside-down portrait mode

  // Allocate and initialize the touchscreen object.
  ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, TOUCH_IRQ_PIN);
  ts->begin();
  ts->setRotation(tft->getRotation()); // Use same rotation as TFT display.

  // Allocate and initialize the touchscreen-tft mapping object.
  tsmap = new TS_ILI9341_map();
  tsmap->begin(ts, tft);

  // Additional tft function calls would appear below to create the initial display screen.
```

It is recommended you follow the above order, doing the TFT display *before* the touchscreen.

After the above initializations, you will also want to call TFT display functions to create the initial screen on the display.

### Reading touch information and finding its display coordinates

In your Arduino *loop()* function, you will monitor for screen touches and handle them. Define a global *boolean* variable *wasTouched* to keep track of whether or not the screen was touched but the touch has not yet been released (true) or has been released (false).

```
boolean wasTouched = false;
```

In the *loop()* function, call the touchscreen *touched()* function to test for a screen touch. If a touch occurred, ignore it if *wasTouched* is true (because the touch was already handled on a previous pass through the loop). If *wasTouched* is false, this is the first time you've seen this touch, so handle it. First, call the *getPoint()* function to get the touched point coordinates, and then pass those coordinates to the *TS_ILI9341_map* class function *mapTS_to_TFT()* to map them to TFT display coordinates. You are then ready to use those coordinates to decide what you should do based on a user screen tap at that (x, y) position. For example:

```
void loop() {
  boolean isTouched = ts->touched();
  if (isTouched) {
    // Handle only if it wasn't previously touched.
    if (!wasTouched) {
      // Get the touched point.
      TS_Point p = ts->getPoint();
      // Map it to TFT display coordinates (x,y).
      int16_t x, y;
      tsmap->mapTS_to_TFT(p.x, p.y, &x, &y);
      // Here you can use (x,y) to either plot something at that position on the TFT
      // or perhaps check to see if (x,y) lies within something you've displayed on
      // the screen, such as a button.
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

## Calibrating the touchscreen

The mapping class *TS_ILI9341_map* introduced in the previous section also includes functions for calibrating the relationship between touchscreen coordinates and TFT display coordinates. Although the default calibration is okay, it isn't as ideal as it could be. Touchscreens seem to vary a bit from one to another, and the different rotations also behave differently.

An example program is provided that illustrates calibration of the touchscreen. It also shows how to save the calibration values in EEPROM so they can be retrieved following a power-down and powering back up. It is described in a following section.

## Example programs

Four example programs are provided in the library's *examples* subfolder. All four programs require that you set #define values near the start of the file to define the pin numbers connected to the touchscreen and in some cases to a TFT LCD display.

### TouchTest.ino

Example program *TouchTest.ino* assumes only a touchscreen with XPT2346 controller for hardware. It initializes the touchscreen software and controller, then waits for touches and displays their coordinates on the IDE serial monitor window.

### TouchTestIRQ.ino

Example program *TouchTestIRQ.ino* is just like *TouchTest.ino* except it uses interrupts whereas the latter does not.

### ILI9341Test.ino

Example program *ILI9341Test.ino* assumes a touchscreen with XPT2346 controller connected to a TFT LCD display with an ILI9341 controller. It initializes the TFT and the touchscreen software and controllers, then waits for touches, displays their coordinates on the screen and in the IDE serial monitor window, and draws a green "+" at the display position to which the touched position maps (using the mapping class *TS_ILI9341_map*).

### ILI9341Calibrate.ino

Example program *ILI9341Calibrate.ino* illustrates how to provide a user with a screen that lets him calibrate the relationship between the touchscreen coordinates and the TFT display coordinates. It also assumes a touchscreen with XPT2346 controller connected to a TFT LCD display with an ILI9341 controller. If you are using SAMD architecture, it uses the FlashStorage_SAMD library to store calibration data permanently across power-downs. It initializes the TFT and the touchscreen software and controllers and the mapping object, then waits for two touches, displays their coordinates on the screen and in the IDE serial monitor window, and computes new calibration parameters and displays them on the serial monitor. If the SAMD architecture is being used, it writes the calibration parameters to EEPROM non-volatile memory. They are read back when the program is restarted, and used as the initial calibration setting.

You can also use the calibration program to determine the ideal calibration parameters for your touchscreen and its rotation. You can then call *tsmap->setTS_calibration()* with those ideal parameters, after calling *tsmap->begin()* in *setup()*.

You can model your own calibration screen after the one shown in this example program. If you do not have the SAMD architecture, this is a problem, because you don't want to force the user to calibrate the screen at every startup. Instead, you want to store the calibration parameters in non-volatile memory and retrieve them at startup. (A calibration screen would only be displayed when the user invoked it.) There are other non-volatile memory libraries available, and you will want to choose one and make the necessary changes to the example program to use it.

## Adafruit Library Compatibility

XPT2046_Touchscreen is meant to be compatible with sketches written for Adafruit_STMPE610, offering the same functions, parameters and numerical ranges as Adafruit's library.

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

