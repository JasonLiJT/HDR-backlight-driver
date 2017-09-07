#ifndef HDR_BACKLIGHT_DRIVER_H
#define HDR_BACKLIGHT_DRIVER_H

#include <iostream>  // std::cerr
#include <cstdlib>   // exit()

#define SCREEN_SIZE_X 9
#define SCREEN_SIZE_Y 16

#define LED_CHANNELS_PER_CHIP 16

class TLCdriver {
    uint16_t _gsData[3][LED_CHANNELS_PER_CHIP][3];

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

    const uint8_t _gsIndexColor[SCREEN_SIZE_X][SCREEN_SIZE_Y] = {
        {1, 0, 2, 1, 0, 2, 1, 0, 0, 1, 2, 1, 2, 1, 0, 2},
        {2, 1, 0, 2, 1, 0, 2, 2, 2, 1, 0, 0, 1, 1, 0, 2},
        {0, 1, 2, 0, 1, 2, 0, 1, 1, 1, 2, 0, 2, 1, 0, 2},
        {2, 1, 2, 0, 2, 0, 1, 2, 0, 2, 0, 2, 1, 1, 0, 2},
        {2, 1, 0, 1, 1, 0, 1, 2, 2, 2, 0, 1, 0, 1, 0, 2},
        {2, 1, 0, 0, 0, 0, 1, 2, 1, 0, 1, 1, 0, 1, 0, 2},
        {2, 1, 0, 2, 2, 0, 1, 2, 0, 1, 0, 2, 1, 2, 1, 0},
        {2, 1, 0, 1, 1, 0, 1, 2, 2, 2, 2, 1, 0, 2, 2, 1},
        {0, 1, 2, 0, 0, 2, 1, 0, 1, 0, 1, 0, 2, 0, 0, 2},
    };

   public:
    TLCdriver();
    void changeBrightness(uint8_t x, uint8_t y, uint16_t bright);
    void sendGsData();
    
private:
    void verify_coordinate(uint8_t x, uint8_t y);
    void checksum();
};

#endif  // !HDR_BACKLIGHT_DRIVER_H
