// The driver program to run on your computer
// Use serial communication to control the Teensy 3.2,
// which controls the three TLC5955 LED drivers
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>  // For wall clock, since c++11

#include "HDR-backlight-driver.hpp"

#ifdef USING_SERIAL_WINDOWS_LIBRARY
// use the library for Windows
#include "serialWindows/serialWindows.cpp"
#else
#include "arduino-serial/arduino-serial-lib.c"
#define INVALID_HANDLE_VALUE -1
#endif

using std::cerr;
using std::endl;

namespace hdrbacklightdriverjli {

TLCdriver::TLCdriver(const char *serialport, int baud) {
    // Constructor
    serialport_fd = serialport_init(serialport, baud);
    if (serialport_fd == INVALID_HANDLE_VALUE) {
        // Error occurred
        // Error handled in serialport_init()
        exit(1);
    }

    // Verify the conversion matrices
    checksum();

    // Allocate memory for *write_buffer
    write_buffer = new uint8_t[MAX_write_buffer_size];
    write_buffer_size = 0;
}

TLCdriver::~TLCdriver() {
    // Destructor
    delete[] write_buffer;

    serialport_close(serialport_fd);
}

void TLCdriver::verify_coordinate(uint8_t x, uint8_t y) {
    if (x < 0 || x >= SCREEN_SIZE_X || y < 0 || y >= SCREEN_SIZE_Y) {
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
        // cerr << "Checksum OK." << endl;
    }
}

void TLCdriver::setLED(uint8_t x, uint8_t y, uint16_t bright) {
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

//-----------------------Demo Program--------------------------------

using hdrbacklightdriverjli::TLCdriver;

void blink() {
    TLCdriver TLCteensy;
    clock_t timer_start;
    while (1) {
        timer_start = clock();
        while (clock() - timer_start < CLOCKS_PER_SEC)
            ;
        TLCteensy.setAllLED(0);
        TLCteensy.updateFrame();
        cerr << "Update 0" << endl;

        timer_start = clock();
        while (clock() - timer_start < CLOCKS_PER_SEC)
            ;
        TLCteensy.setAllLED(0xFFFF);
        TLCteensy.updateFrame();
        cerr << "Update 65535" << endl;
    }
}

void speedtest() {
    TLCdriver TLCteensy;
    while (1) {
        // DO NOT use clock() from <ctime>
        // because the thread (CPU time) sleeps during communication for synchronization
        // Use wall time instead
        auto timer_start = std::chrono::system_clock::now();
        int step = 0x100;
        for (int bright = 0; bright < 0x10000; bright += step) {
            // TLCteensy.setAllLED(bright);
            TLCteensy.setAllLED(0);
            TLCteensy.setLED(5, 11, bright);
            TLCteensy.updateFrame();
        }
        for (int bright = 0x10000; bright > 0; bright -= step) {
            // TLCteensy.setAllLED(bright);
            TLCteensy.setAllLED(0);
            TLCteensy.setLED(1, 7, bright);
            TLCteensy.updateFrame();
        }
        auto timer_end = std::chrono::system_clock::now();
        std::chrono::duration<double> wall_time_elapsed = timer_end - timer_start;  // In seconds
        cerr << 2.0 * (0x10000 / step) / wall_time_elapsed.count() << " frames per sec." << endl;
    }
}

void testLEDs() {
    TLCdriver TLCteensy;
    while (1) {
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
        cerr << (1 + SCREEN_SIZE_X * SCREEN_SIZE_Y) / wall_time_elapsed.count() << " frames per sec." << endl;
    }
}

int main() {
    // speedtest();
    testLEDs();
}
