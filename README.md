# HDR backlight driver program

This driver is designed to use serial communication protocol to control TLC5955 LED drivers via Teensy 3.2.

The *Teensy_TLC_Control* sketch is the Teensy 3.2 client, and the *TLC_driver.cpp* is the driver to run on your computer.

Now it works on both **Windows** and **POSIX** systems (e.g. **most Linux systems and Mac OS X**).

## Requirements
- Teensyduino
- g++
