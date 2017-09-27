/*
-----------------------Demo Program--------------------------------
A demo driver program to run on your computer.
It uses serial communication to control the Teensy 3.2,
which controls the TLC5955 LED drivers.

Copyright (c) 2017 Junteng (Jason) Li

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <chrono>  // For wall clock, since c++11

#include "HDR-backlight-driver.hpp"

using hdrbacklightdriverjli::TLCdriver;

using std::clog;
using std::endl;

void testBrightness(TLCdriver& TLCteensy) {
    int step = 0x100;
    for (int bright = 0; bright <= 0xFFFF; bright += step) {
        TLCteensy.setAllLED(bright);
        TLCteensy.updateFrame();
    }
    for (int bright = 0xFFFF; bright >= 0; bright -= step) {
        // The following two for loops are equivalent to:
        // TLCteensy.setAllLED(bright);
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            for (int y = 0; y < SCREEN_SIZE_Y; y++) {
                TLCteensy.setLED(x, y, bright);
            }
        }
        TLCteensy.updateFrame();
    }
}

void testLEDs(TLCdriver& TLCteensy) {
    // DO NOT use clock() from <ctime>
    // because the thread (CPU time) sleeps during communication for synchronization
    // Use wall time instead
    auto timer_start = std::chrono::system_clock::now();
    // auto time_step = std::chrono::microseconds(0);
    // auto time_step = std::chrono::microseconds((int)(1.0 / 60 * 1e6));
    TLCteensy.setAllLED(0);
    TLCteensy.updateFrame();
    for (int x = 0; x < SCREEN_SIZE_X; x++) {
        for (int y = 0; y < SCREEN_SIZE_Y; y++) {
            // Delay time_step microseconds
            auto temp_start = std::chrono::system_clock::now();
            // while (std::chrono::system_clock::now() - temp_start < time_step)
            //     ;

            TLCteensy.setAllLED(0);
            TLCteensy.setLED(x, y, 0xFFFF);
            TLCteensy.updateFrame();
            auto temp_end = std::chrono::system_clock::now();
            std::chrono::duration<double> temp_elapsed = temp_end - temp_start;  // In seconds
            if (temp_elapsed > std::chrono::microseconds((int)1e6/120)) {
                // The framerate drops below 120 fps
                // It may cause flickering
                clog << "The framerate drops to: " << 1 / temp_elapsed.count() << " FPS" << endl;
            }
        }
    }
    auto timer_end = std::chrono::system_clock::now();
    std::chrono::duration<double> wall_time_elapsed = timer_end - timer_start;  // In seconds
    clog << (1 + SCREEN_SIZE_X * SCREEN_SIZE_Y) / wall_time_elapsed.count() << " frames per sec." << endl;
}

int main() {
    TLCdriver TLCteensy(DEFAULT_SERIAL_PORT, 9600);

    // For debugging: get the internal array indices of an LED
    TLCteensy.print_index(1, 0);

    while (1) {
        testBrightness(TLCteensy);
        testLEDs(TLCteensy);
        // TODO: Investigate the speed drop from 400 FPS to 150 FPS

        // TODO: separate the latch() from updateLEDs(): write updateLEDsWithoutLatch()
        // TODO: control latch at each frame update
        // TODO: think of a way to synchronize the backlight with the LCD screen
    }
}
