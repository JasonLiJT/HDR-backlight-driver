/* The HDR backlight driver library for C++

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

#ifndef HDR_BACKLIGHT_DRIVER_H
#define HDR_BACKLIGHT_DRIVER_H

#include <iostream>  // std::cerr, std::clog, std::endl
#include <cstdlib>   // exit()
#include <ctime>     // clock()

#if defined(__MINGW32__) || defined(_WIN32)
#define USING_SERIAL_WINDOWS_LIBRARY
// use the library for Windows
#include "serialWindows/serialWindows.cpp"

//*****************************************************
// Change the default serial port name here (for Windows)
// Note that if the port number is larger than 9, e.g. COM10,
// "\\\\.\\COM10" has to be used.
// "COM10" won't work. (Well, it's Windows)
#define DEFAULT_SERIAL_PORT "\\\\.\\COM4"
//*****************************************************

#else

#include "arduino-serial/arduino-serial-lib.c"
#define INVALID_HANDLE_VALUE -1

//*****************************************************
// Change the default serial port name here (for Mac OS X / Linux)
#define DEFAULT_SERIAL_PORT "/dev/cu.usbmodem3118791"
//*****************************************************

#endif

#define SCREEN_SIZE_X 9
#define SCREEN_SIZE_Y 16

#define TLC_COUNT 3
#define LED_CHANNELS_PER_CHIP 16
#define COLOR_CHANNEL_COUNT 3

// Class interface
namespace hdrbacklightdriverjli {

class TLCdriver
#ifdef USING_SERIAL_WINDOWS_LIBRARY
    // Inherite from the serialWindows library class
    // Use the same function signatures as arduino-serial-lib
    : public SerialPortWindows
#endif
{
    // The array for all grayscale pixel values
    // Initialize all to 0
    uint16_t _gsData[TLC_COUNT][LED_CHANNELS_PER_CHIP][COLOR_CHANNEL_COUNT] = {{{0}}};

    uint8_t* write_buffer;
    int write_buffer_size;

    // Used to allocate memory for *write_buffer
    // Allocate 144*2 more for safety
    const int MAX_write_buffer_size = 2 * TLC_COUNT * LED_CHANNELS_PER_CHIP * COLOR_CHANNEL_COUNT + 144 * 2;

    // Convert PCB LED coordinate to the indices of _gsData[]
    const size_t _gsIndexChip[SCREEN_SIZE_X][SCREEN_SIZE_Y] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0},
    };

    const size_t _gsIndexChannel[SCREEN_SIZE_X][SCREEN_SIZE_Y] = {
        {9, 9, 9, 12, 12, 12, 8, 8, 4, 4, 0, 2, 6, 11, 11, 11},
        {10, 14, 14, 14, 13, 13, 13, 8, 4, 0, 0, 6, 6, 15, 15, 15},
        {10, 10, 15, 15, 15, 11, 11, 11, 7, 5, 5, 3, 3, 10, 10, 10},
        {9, 12, 12, 12, 0, 0, 0, 5, 7, 1, 5, 7, 3, 14, 14, 14},
        {13, 9, 9, 8, 4, 5, 5, 1, 7, 2, 1, 7, 7, 13, 13, 13},
        {14, 13, 13, 8, 4, 1, 1, 2, 3, 2, 1, 4, 4, 9, 9, 9},
        {10, 14, 14, 8, 4, 2, 2, 6, 3, 2, 2, 5, 0, 4, 12, 12},
        {15, 10, 10, 11, 7, 6, 6, 3, 3, 6, 2, 5, 5, 0, 12, 8},
        {15, 15, 11, 11, 7, 7, 3, 3, 6, 6, 1, 1, 1, 0, 8, 8},
    };

    // Quick check: Adjacent LEDs in the same channel must have different color
    const size_t _gsIndexColor[SCREEN_SIZE_X][SCREEN_SIZE_Y] = {
        {1, 0, 2, 1, 0, 2, 1, 0, 0, 1, 2, 1, 2, 1, 0, 2},
        {2, 1, 0, 2, 1, 0, 2, 2, 2, 1, 0, 0, 1, 1, 0, 2},
        {0, 1, 2, 0, 1, 2, 0, 1, 1, 1, 2, 0, 2, 1, 0, 2},
        {2, 1, 2, 0, 2, 0, 1, 2, 0, 2, 0, 2, 1, 1, 0, 2},
        {2, 1, 0, 1, 1, 0, 1, 2, 2, 2, 0, 1, 0, 1, 0, 2},
        {2, 1, 0, 0, 0, 0, 1, 2, 1, 0, 1, 2, 0, 1, 0, 2},
        {2, 1, 0, 2, 2, 0, 1, 2, 0, 1, 0, 2, 1, 1, 1, 0},
        {2, 1, 0, 1, 1, 0, 1, 2, 2, 2, 2, 1, 0, 2, 2, 1},
        {0, 1, 2, 0, 0, 2, 1, 0, 1, 0, 1, 0, 2, 0, 0, 2},
    };

   public:
    // ctor: Open serial port, allocate memory and verify the conversion matrices with checksum()
    TLCdriver(const char* serialport = DEFAULT_SERIAL_PORT, int baud = 9600);

    // dtor: Free memory space and close serial port
    ~TLCdriver();

    // Accessor methods
    auto get_fd() const {  // deduced return types are a C++14 extension
        // The return types are different on Windows and POSIX systems
        return serialport_fd;
    }
    void print_index(size_t x, size_t y) {
        std::clog << "Internal data indices of (" << x << ", " << y << "):\n\t" << _gsIndexChip[x][y] << " " << _gsIndexChannel[x][y] << " " << _gsIndexColor[x][y] << std::endl;
    }

    // Update state variables
    void setLED(size_t x, size_t y, uint16_t bright);  // Set the brightness of the LED at (x, y) to bright
    void setAllLED(uint16_t bright);
    // Debug: set the brightness of the LEDs of a specific chip
    void setLEDChip(size_t chip_index, uint16_t bright);

    // Send data to Teensy
    void updateFrame();

   private:
    bool add_to_buffer(uint8_t byte);
#ifdef USING_SERIAL_WINDOWS_LIBRARY
    HANDLE serialport_fd;
#else
    int serialport_fd;
#endif
    void verify_coordinate(size_t x, size_t y);
    void checksum();
};
}  //namespace: hdrbacklightdriverjli

// Implementation
namespace hdrbacklightdriverjli {

using std::cerr;
using std::clog;
using std::endl;

TLCdriver::TLCdriver(const char* serialport, int baud) {
    // Constructor
    serialport_fd = serialport_init(serialport, baud);
    if (serialport_fd == INVALID_HANDLE_VALUE) {
        // Error occurred
        // Error handled in serialport_init()
        exit(1);
    }

    clog << "Port \"" << serialport << "\" successfully opened :)" << endl;

    // Tell teensy to reset
    serialport_writebyte(serialport_fd, 'R');
    serialport_writebyte(serialport_fd, 'T');

    clog << "Resetting Teensy..." << endl;

    serialport_close(serialport_fd);

    int connect_count = 0;
    while (1) {
        clock_t timer_start = clock();
        while (clock() - timer_start < 0.1 * CLOCKS_PER_SEC)
            // Wait 0.1s before reconnecting
            ;

        serialport_fd = serialport_init(serialport, baud);
        connect_count++;
        if (serialport_fd != INVALID_HANDLE_VALUE) {
            if (connect_count > 1) {
                // Expected at least one reconnection attempt
                break;
            } else {
                clog << "\n\nReboot failed. Please retry." << endl;
                exit(1);
            }
        } else {
            clog << "\n\nTeensy still rebooting..." << endl;
        }
    }

    clog << "\n\nThe error messages above are expected.\n";
    clog << "Reboot complete!" << endl;

    // Verify the conversion matrices
    checksum();

    // Allocate memory for *write_buffer
    write_buffer = new uint8_t[MAX_write_buffer_size];
    write_buffer_size = 0;
}

TLCdriver::~TLCdriver() {
    // Destructor
    // Free memory space
    delete[] write_buffer;

    // Close the serial port
    serialport_close(serialport_fd);
}

void TLCdriver::verify_coordinate(size_t x, size_t y) {
    if (x >= SCREEN_SIZE_X || y >= SCREEN_SIZE_Y) {  // size_t is always unsigned: no need to check sign
        cerr << "TLC5955converter::to_gsIndex(): index out of range" << endl;
        exit(1);
    }
}

void TLCdriver::checksum() {
    // Checksum: each channel is expected to have a checksum of:
    //   TLC_COUNT * COLOR_CHANNEL_COUNT * (COLOR_CHANNEL_COUNT - 1) / 2
    //           + COLOR_CHANNEL_COUNT * TLC_COUNT * (TLC_COUNT - 1) / 2
    // = TLC_COUNT * COLOR_CHANNEL_COUNT * (COLOR_CHANNEL_COUNT + TLC_COUNT - 2) / 2
    int expected_sum = TLC_COUNT * COLOR_CHANNEL_COUNT * (COLOR_CHANNEL_COUNT + TLC_COUNT - 2) / 2;

    int c[LED_CHANNELS_PER_CHIP] = {0};
    for (int i = 0; i < SCREEN_SIZE_X; i++) {
        for (int j = 0; j < SCREEN_SIZE_Y; j++) {
            c[_gsIndexChannel[i][j]] += _gsIndexColor[i][j] + _gsIndexChip[i][j];
        }
    }

    for (int i = 0; i < LED_CHANNELS_PER_CHIP; i++) {
        if (c[i] != expected_sum) {
            cerr << "TLCdriver::checksum(): Error:\n";
            cerr << "\tExpected all checksums to be " << expected_sum << ".\n";
            cerr << "\tValues obtained: ";
            for (int i = 0; i < LED_CHANNELS_PER_CHIP; i++) {
                cerr << c[i] << ' ';
            }
            cerr << '\n';
            exit(1);
        }
    }
    clog << "Conversion matrices checksum OK." << endl;
}

void TLCdriver::setLED(size_t x, size_t y, uint16_t bright) {
    // Set the brightness of the LED at (x, y) to bright
    verify_coordinate(x, y);
    _gsData[_gsIndexChip[x][y]][_gsIndexChannel[x][y]][_gsIndexColor[x][y]] = bright;
}

void TLCdriver::setAllLED(uint16_t bright) {
    for (int i = 0; i < TLC_COUNT; i++) {
        for (int j = 0; j < LED_CHANNELS_PER_CHIP; j++) {
            for (int k = 0; k < COLOR_CHANNEL_COUNT; k++) {
                _gsData[i][j][k] = bright;
            }
        }
    }
}

void TLCdriver::setLEDChip(size_t chip_index, uint16_t bright) {
    // DEBUG Chip problems
    // Set the brightness of the LEDs of a specific chip
    if (chip_index >= TLC_COUNT) {
        cerr << "TLCdriver::setLEDChip(): chip_index out of range!" << endl;
    }
    for (int j = 0; j < LED_CHANNELS_PER_CHIP; j++) {
        for (int k = 0; k < COLOR_CHANNEL_COUNT; k++) {
            _gsData[chip_index][j][k] = bright;
        }
    }
}

bool TLCdriver::add_to_buffer(uint8_t byte) {
    if (write_buffer_size >= MAX_write_buffer_size) {
        return false;
    }
    write_buffer[write_buffer_size++] = byte;
    return true;
}

void TLCdriver::updateFrame() {
    ////////////////////////////////////////////////////
    //Write and send data

    // 0xFF, 0x00 mark the start
    add_to_buffer('G');
    add_to_buffer('O');
    for (int i = 0; i < TLC_COUNT; i++) {
        for (int j = 0; j < LED_CHANNELS_PER_CHIP; j++) {
            for (int k = 0; k < COLOR_CHANNEL_COUNT; k++) {
                add_to_buffer((uint8_t)(_gsData[i][j][k] >> 8));  // Send the higher byte first
                add_to_buffer((uint8_t)(_gsData[i][j][k]));       // Then the lower byte
            }
        }
    }
    serialport_writeBuffer(serialport_fd, write_buffer, write_buffer_size);
    write_buffer_size = 0;

    ///////////////////////////////////////////////////
    // Read feedback
    int feedback_byte_0, feedback_byte_1;
    feedback_byte_0 = serialport_readByte(serialport_fd, 90);  // 90 ms timeout
    feedback_byte_1 = serialport_readByte(serialport_fd, 10);  // 10 ms timeout
    // Total 100 ms timeout, which means minimum 10 FPS

    if (feedback_byte_0 == -1 || feedback_byte_1 == -1) {
        cerr << "TLCdriver::updateFrame():\n\tError: couldn't read feedback bytes" << endl;
    } else if (feedback_byte_0 == -2 || feedback_byte_1 == -2) {
        cerr << "TLCdriver::updateFrame():\n\tError: feedback bytes reading timeout" << endl;
    } else if (!(feedback_byte_0 == 'D' && feedback_byte_1 == 'N')) {
        // Wrong feedback byte
        cerr << "TLCdriver::updateFrame():\n\tError: feedback bytes wrong" << endl;
    }
}
}  //namespace: hdrbacklightdriverjli

#endif  // !HDR_BACKLIGHT_DRIVER_H
