# XPT2046_Touchscreen_TT change log
---

## Table of Contents

* [Release 1.7.1](#release-171)
* [Release 1.6.1](#release-161)
* [Release 1.5.1](#release-151)
---

### Release 1.7.1

1. Some changes were made to remove an implied dependence on the ILI9341 display controller, which is actually not required as long as the display controller class is derived from graphics class Adafruit_GFX. This involved some renaming of files and classes. Files TS_ILI9341.h/.cpp were renamed to TS_Display.h/.cpp, and their class TS_ILI9341 was renamed to TS_Display. Example files ILI9341Calibrate.ino, ILI9341Events.ino, and ILI9341Test.ino were renamed to TS_DisplayCalibrate.ino, TS_DisplayEvents.ino, and TS_DisplayTest.ino, respectively.

The argument named *tft* to the TS_Display class begin() function was renamed to *disp* and its type is changed to a pointer to class Adafruit_GFX rather than class Adafruit_ILI9341. Note that an object of type Adafruit_ILI9341 can be used as a pointer to an Adafruit_GFX class because it is derived (indirectly) from that class. Within programs, you still declare the pointer as a pointer to an Adafruit_ILI9341 class and allocate it that way, then pass that pointer to the TS_Display::begin() function.

It is possible that you have a different type of display with a different controller, and it will still work with this library as long as there is a class derived from Adafruit_GFX that handles the controller interaction. In that case you would create a pointer to that class and allocate an instance of it and assign it to the pointer, then use the pointer as the *disp* argument to the TS_Display::begin() function.

### Release 1.6.1

1. Renamed class TS_ILI9341_map to simply TS_ILI9341, with .h/.cpp files also renamed, and added enum eTouchEvent and class functions getTouchEvent() and setTouchReleaseParams(), to support touchscreen touch and release events.

### Release 1.5.1

1. Initial release of this library, cloned from version 1.4 of Paul Stoffregen's XPT2046_Touchscreen library at https://github.com/PaulStoffregen/XPT2046_Touchscreen.
