// The driver program to run on your computer
// Use serial communication to control the Teensy 3.2,
// which controls the three TLC5955 LED drivers
#include <iostream>
#include <string>

#include "HDR-backlight-driver.h"
#include "arduino-serial/arduino-serial-lib.c"

using std::cerr;
using std::endl;

uint8_t PcbCoordinateConverter::to_gsIndex(uint8_t x, uint8_t y) {
    // Look up for the index for _gsData[] in TLC5955 class
    if (x < 0 || x > 8 || y < 0 || y > 15) {
        cerr << "TLC5955converter::to_gsIndex(): index out of range\n";
        exit(1);
    }
    return _gsIndex[x][y];
}

PcbCoordinateConverter PCB_xy;

int main() {
    int tlc = serialport_init("/dev/cu.usbmodem3118791", 9600);
    
    serialport_write(tlc, "\\");
    
    serialport_close(tlc);
    return 0;
}
