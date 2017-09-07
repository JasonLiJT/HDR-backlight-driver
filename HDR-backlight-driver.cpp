// The driver program to run on your computer
// Use serial communication to control the Teensy 3.2,
// which controls the three TLC5955 LED drivers
#include <iostream>
#include <string>

#include "HDR-backlight-driver.h"
#include "arduino-serial/arduino-serial-lib.c"

using std::cerr;
using std::endl;

TLCdriver::TLCdriver() {
    // Constructor:
    // Verify the conversion matrices
    checksum();
}

void TLCdriver::verify_coordinate(uint8_t x, uint8_t y) {
    if (x < 0 || x >= SCREEN_SIZE_X || y < 0 || y >= SCREEN_SIZE_Y) {
        cerr << "TLC5955converter::to_gsIndex(): index out of range" << endl;
        exit(1);
    }
}

void TLCdriver::checksum() {
    int c[LED_CHANNELS_PER_CHIP] = {0};
    for (int i = 0; i < SCREEN_SIZE_X; i++) {
        for (int j = 0; j < SCREEN_SIZE_Y; j++) {
            c[_gsIndexChannel[i][j]] += _gsIndexColor[i][j] + _gsIndexChip[i][j];
        }
    }

    // Checksum: expected LED_CHANNELS_PER_CHIP output of 18
    for (int i = 0; i < LED_CHANNELS_PER_CHIP; i++) {
        if (c[i] != 18) {
            cerr << "TLCdriver::checksum(): Error:\n";
            cerr << "\tExpected all values to be 18.\n";
            cerr << "\tValues obtained: ";
            for (int i = 0; i < LED_CHANNELS_PER_CHIP; i++) {
                cerr << c[i] << ' ';
            }
            cerr << '\n';
            exit(1);
        }
        // cerr << "Checksum OK." << endl;
    }
}

void TLCdriver::changeBrightness(uint8_t x, uint8_t y, uint16_t bright) {
    verify_coordinate(x, y);
    _gsData[_gsIndexChip[x][y]][_gsIndexChannel[x][y]][_gsIndexColor[x][y]] = bright;
}

TLCdriver TLCteensy;

int main() {
    int tlc = serialport_init("/dev/cu.usbmodem3118791", 9600);

    serialport_write(tlc, "\\");
    serialport_close(tlc);
    return 0;
}
