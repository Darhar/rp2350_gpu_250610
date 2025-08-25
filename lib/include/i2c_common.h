#pragma once
#include <cstdint>
#include <cstddef>
#include "pico/types.h"

#define FLAG_VALUE 123

class ScreenManager; // fwd declare (don’t pull heavy headers here)
class KeyBoard;

namespace i2c_common {

    struct Params {
        int i2c_index;          // 0 or 1
        uint sda_pin;
        uint scl_pin;
        uint baud = 100000;
        uint8_t slave_addr = 0x42;
    };

    // Provide the ScreenManager to the slave once at startup.
    void bind(ScreenManager& mgr);
    void setKeyboard(KeyBoard& kbd);
    // Configure I2C hardware as a slave.
    void init(const Params& p);

    // Optional: if you use a polling model for some work.
    // If everything is IRQ-driven you can leave this empty.
    void poll();

} // namespace i2c_common
