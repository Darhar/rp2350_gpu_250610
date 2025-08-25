#pragma once

#include <hardware/adc.h>
#include <cstdint>

//#include "screenManager.hpp"

#define PINRHT 6
#define PINLFT 23
#define PINSEL 40
#define PINUP 54
#define PINDWN 70

#define butPin 29

#define KEY_UP 1
#define KEY_DOWN 2
#define KEY_LEFT 3
#define KEY_OK 4
#define KEY_BACK 5
/*
class KeyBoard
{
    private:
        uint8_t prevKey;

    public:
        KeyBoard();
        ~KeyBoard();

        void checkKeyState(ScreenManager* screen);
};
*/

class KeyBoard {
    public:
        // State bits (current)
        static constexpr uint8_t ST_DOWN    = 1 << 0; // some key held
        // Edge bits (latched, cleared by fillReport)
        static constexpr uint8_t ED_PRESS   = 1 << 0; // transition to >0 since last snapshot
        static constexpr uint8_t ED_RELEASE = 1 << 1; // transition to 0 since last snapshot

        struct __attribute__((packed)) KeyReport {
            uint8_t  key;        // 0..5 (your mapping)
            uint8_t  stateBits;  // ST_DOWN
            uint8_t  edgeBits;   // ED_PRESS | ED_RELEASE (latched)
            uint8_t  analog;     // quantized adc_read()/50 bucket for debugging/telemetry
            uint32_t eventCount; // increments on every edge (press/release)
        };

        KeyBoard();
        ~KeyBoard();

        void checkKeyState(class ScreenManager* sm);
        void fillReport(KeyReport& out); // copies + clears edge bits

    private:
        uint8_t  prevKey = 0;
        uint8_t  currKey = 0;
        uint8_t  stateBits = 0;
        uint8_t  edgeBits  = 0; // latched until fillReport()
        uint8_t  lastAnalog = 0;
        uint32_t eventCount = 0;
};
