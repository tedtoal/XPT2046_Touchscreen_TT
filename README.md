# XPT2046_Touchscreen_TT

## Changes PaulStoffregen/XPT_2046_Touchscreen with functions for setting and getting thresholds

*XPT2046_Touchscreen_TT* is an Arduino C++ library that supports the XPT2046 resistive touchscreen controllers used on many low cost TFT displays. It is a copy of the library made by Paul Stoffregen named *XPT_2046_Touchscreen*. The changes to it are:

**1.** The defines of Z_THRESHOLD and Z_THRESHOLD_INT were moved from *XPT2046_Touchscreen_TT.cpp* to *XPT2046_Touchscreen_TT.h*.

**2.** New function *setThresholds()* allows dynamically setting the pressure thresholds previously hard-coded as Z_THRESHOLD and Z_THRESHOLD_INT, now stored in variables Z_Threshold and Z_Threshold_Int.

**3.** New function *Zthreshold()* returns the pressure threshold Z_Threshold.

**4.** New function *Zthreshold_Int()* returns the pressure threshold Z_Threshold_Int.

**5.** Function header comments in Doxygen style have been added to the XPT2046_Touchscreen_TT.h file.

**6.** Example programs have been changed:

> Use *Adafruit-GFX-Library* and *Adafruit_ILI9341* libraries in place of *ILI9341_t3* library.
>
> Use *Adafruit-GFX-library* *FreeSans12pt7b* font in place of Arial fonts from *ILI9341_t3* library.
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
> 
> Call the new set and get threshold functions.

## Using XPT2036_Touchscreen in your project

### Include files

At the start of your project .ino file, where you #include other files, #include *SPI.h* and *XPT2046_Touchscreen_TT.h*:

```
#include <SPI.h>
#include <XPT2046_Touchscreen_TT.h>
```

### Define pins

Next, near the start of your project .ino file, use #define statements to define the pins used by the touchscreen in your system. Define a value for the touch controller device CS (chip select) and IRQ (interrupt request) pins. You probably don't need to define values for the MOSI, MISO, and SCK pins because these are standardized on Arduino processors. For example:

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

### Initialization in setup()

In your Arduino setup() function, use *new* to allocate a touchscreen object and assign it to your pointer variable, call the object's begin() function to initialize it, and call additional object functions as needed to configure it. For example:

```
  // Allocate the touchscreen object, then initialize it.
  ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN);  // Param 2 - NULL - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, 255);  // Param 2 - 255 - No interrupts
  //ts = new XPT2046_Touchscreen(TOUCH_CS_PIN, TOUCH_IRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling
  ts->begin();
  ts->setRotation(1);
  ts->setThresholds(Z_THRESHOLD/3);
```

Three ways to allocate the object are shown in the comments above, depending on whether or not your want to use the IRQ pin and interrupts to improve touchscreen performance and reduce the time required when touchscreen functions are called to test for presses. The first two methods shown are equivalent, while the third uses interrupts (recommended method).

The *begin()* function initializes the touchscreen object and device controller, the *setRotation()* function indicates what orientation the touchscreen is in (0-3 for north, east, south, west, matching the rotation setting for the display as used with the ILI9341 library), and setThresholds() is optional and allows you to alter the pressure threshold for detecting a press.

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

## Example programs

Three example programs are provided in the library's *examples* subfolder.

### ILI9341Test.ino

Example program *ILI9341Test.ino* assumes a touchscreen with XPT2346 controller connected to a TFT LCD display with an ILI9341 controller. It initializes the TFT and the touchscreen software and controllers, then waits for touches and displays their coordinates on the screen and in the IDE serial monitor window.

### TouchTest.ino

Example program *TouchTest.ino* assumes only a touchscreen with XPT2346 controller for hardware. It initializes the touchscreen software and controller, then waits for touches and displays their coordinates on the IDE serial monitor window.

### TouchTestIRQ.ino

Example program *TouchTestIRQ.ino* is just like *TouchTest.ino* except it uses interrupts whereas the latter does not.

## Adafruit Library Compatibility

XPT2046_Touchscreen is meant to be a compatible with sketches written for Adafruit_STMPE610, offering the same functions, parameters and numerical ranges as Adafruit's library.

## Using The Interrupt Pin

When using interrupts as specified above, there is built-in support when connected, nothing else is needed.

When interrupts are used, no SPI calls are made unless a Touch was detected, which increases program efficiency.

The following warnings are provided should you decide to do a custom interrupt setup with the touchscreen.

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

Here are the two GitHub repositories for this touch controller project:

**Paul Stoffregen repository:** https://github.com/PaulStoffregen/XPT2046_Touchscreen

**My (Ted Toal) repository:** https://github.com/tedtoal/XPT2046_Touchscreen_TT

Paul's email as listed in the code files is: *paul@pjrc.com*

My email is: *ted@tedtoal.net*

