#ifndef HDR_BACKLIGHT_DRIVER_H
#define HDR_BACKLIGHT_DRIVER_H

#include <iostream>  // std::cerr
#include <cstdlib>   // exit()

#if defined(__MINGW32__) || defined(_WIN32)
#define USING_SERIAL_WINDOWS_LIBRARY
// use the library for Windows
#include "serialWindows/serialWindows.hpp"
#define DEFAULT_SERIAL_PORT "\\\\.\\COM4"
#else
#define DEFAULT_SERIAL_PORT "/dev/cu.usbmodem3118791"
#endif

#define SCREEN_SIZE_X 9
#define SCREEN_SIZE_Y 16

#define TLC_COUNT 3
#define LED_CHANNELS_PER_CHIP 16
#define COLOR_CHANNEL_COUNT 3

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
    const uint8_t _gsIndexChip[SCREEN_SIZE_X][SCREEN_SIZE_Y] = {
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

    const uint8_t _gsIndexChannel[SCREEN_SIZE_X][SCREEN_SIZE_Y] = {
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
    const uint8_t _gsIndexColor[SCREEN_SIZE_X][SCREEN_SIZE_Y] = {
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
    // ctor: Open serial port and verify the conversion matrices with checksum()
    TLCdriver(const char* serialport = DEFAULT_SERIAL_PORT, int baud = 9600);

    // dtor: Close serial port
    ~TLCdriver();

    // Accessor methods
    auto get_fd() const {
        return serialport_fd;
    }

    // Update state variables
    void setLED(uint8_t x, uint8_t y, uint16_t bright);
    void setAllLED(uint16_t bright);

    // Send data to Teensy
    void updateFrame();

   private:
    bool add_to_buffer(uint8_t byte);
#ifdef USING_SERIAL_WINDOWS_LIBRARY
    HANDLE serialport_fd;
#else
    int serialport_fd;
#endif
    void verify_coordinate(uint8_t x, uint8_t y);
    void checksum();
};
}
#endif  // !HDR_BACKLIGHT_DRIVER_H
