# HDR backlight driver

This driver library is designed to control TLC5955 LED drivers via Teensy 3.2 with serial communication protocol.

The *Teensy_TLC_Control* sketch is the Teensy 3.2 client, and the *HDR-backlight-driver.hpp* is the driver library for your C++ program to run on your computer.

Now it works on both **Windows** and **POSIX** systems (e.g. **most Linux systems and Mac OS X**). It has been tested on **ubuntu 16.04 LTS**, **Mac OS X El Capitan 10.11.6** and **Windows 10 1703**, each reaching a frame rate of over 115 frames per second.

## Requirements

- A C++ compiler with C++14 support (for deduced return types for cross-platform support)
- Only required for Teensy board setup:
    - Arduino 1.8.3 or later
    - Teensyduino (an Arduino add-on for Teensy support):
        - https://www.pjrc.com/teensy/td_download.html

## Usage

The HDR backlight driver library can be included as a single file:

```C++
#include "HDR-backlight-driver.hpp"

using hdrbacklightdriverjli::TLCdriver;
```

The `TLCdriver` class is in namespace `hdrbacklightdriverjli`.

Have a look at *demo.cpp* for examples. The demo program can be compiled as a single C++ file:

```
g++ -Wall -std=c++14 HDR-backlight-driver.cpp -o HDR-backlight-driver
```

## Teensy Board Setup (only needs to be done once)

1. Make sure you have downloaded and installed Arduino and Teensyduino
1. Copy `Arduino/libraries/TLC5955` to your Arduino library:
    - Open your Arduino IDE, click "File - Preferences" (or "Arduino - Preferences" for Mac OS X) in the menu
    - Find the "Sketchbook location" (usually the first line) e.g. `yourPath/Arduino`
    - Find the `Arduino/libraries/TLC5955` folder in this repository and copy it to `yourPath/Arduino/libraries`
2. Upload the Teensy_TLC_control.ino program to your Teensy board:
    - Restart your Arduino IDE to load the TLC5955 library
    - Open `Teensy_TLC_Control/Teensy_TLC_Control.ino` in your Arduino IDE
    - Connect your Teensy board to your computer
    - Click "Tools - Board" and select "Teensy 3.2 / 3.1"
    - Click "Tools - Ports" and select the port ending with "(Teensy)", and write down the port name (or take a screenshot, because you cannot copy it). The driver will need it to open the port.
        - For example, if the port name is `"/dev/someport233 (Teensy)"`, you'll need to use:
            ```C++
            TLCdriver myTeensyBoard("/dev/someport233", 9600);
            ```
            to open the serial port.
        - For Windows port numbers larger than 9, e.g. `"COM10"`, you'll need to use:
            ```C++
            TLCdriver myTeensyBoard("\\\\.\\COM10", 9600);
            ```
            to open the serial port. (Well, it's Windows.)
        - For smaller Windows port numbers like "COM4", you can get away with
            ```C++
            TLCdriver myTeensyBoard("COM4", 9600);
            ```
    - Leave everything else as default. Make sure "Tools - USB Type" is "Serial".
    - Click "Upload" and wait for the sketch to be compiled and uploaded to your Teensy board.

3. The Teensy board is now ready to communicate with the C++ program.
