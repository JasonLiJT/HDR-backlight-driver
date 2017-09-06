// The driver program to run on your computer
// Use serial communication to control the Teensy 3.2,
// which controls the three TLC5955 LED drivers
#include <iostream>
#include <string>

#include "arduino-serial/arduino-serial-lib.c"

using namespace std;

int main() {
    int tlc = serialport_init("/dev/cu.usbmodem3118791", 9600);
    serialport_write(tlc, "\\");
    serialport_close(tlc);
    return 0;
}