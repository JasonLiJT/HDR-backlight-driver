//-----------------------Demo Program--------------------------------
// The driver program to run on your computer
// Use serial communication to control the Teensy 3.2,
// which controls the three TLC5955 LED drivers
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
    auto time_step = std::chrono::milliseconds(0);
    TLCteensy.setAllLED(0);
    TLCteensy.updateFrame();
    for (int x = 0; x < SCREEN_SIZE_X; x++) {
        for (int y = 0; y < SCREEN_SIZE_Y; y++) {
            // Delay time_step milliseconds
            auto temp_start = std::chrono::system_clock::now();
            while (std::chrono::system_clock::now() - temp_start < time_step)
                ;

            TLCteensy.setAllLED(0);
            TLCteensy.setLED(x, y, 0xFFFF);
            TLCteensy.updateFrame();
        }
    }
    auto timer_end = std::chrono::system_clock::now();
    std::chrono::duration<double> wall_time_elapsed = timer_end - timer_start;  // In seconds
    clog << (1 + SCREEN_SIZE_X * SCREEN_SIZE_Y) / wall_time_elapsed.count() << " frames per sec." << endl;
}

int main() {
    TLCdriver TLCteensy(DEFAULT_SERIAL_PORT, 9600);

    while (1) {
        testBrightness(TLCteensy);
        testLEDs(TLCteensy);
    }
}
