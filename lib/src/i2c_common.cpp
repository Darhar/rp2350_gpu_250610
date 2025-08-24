// lib/src/i2c_common.cpp
#include "i2c_common.h"
#include "screenManager.hpp"         // full type only in .cpp
#include "command_factory.hpp"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <pico/i2c_slave.h>

namespace i2c_common {

static ScreenManager* g_mgr = nullptr;
static i2c_inst_t*    g_i2c = nullptr;

static constexpr int MAX_BUFFER = 64;
static uint8_t  s_buf[MAX_BUFFER];
static volatile int s_idx = 0;

// Forward so we can reference it in init()
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);

void bind(ScreenManager& mgr) { g_mgr = &mgr; }

void init(const Params& p) {
    g_i2c = (p.i2c_index == 0) ? i2c0 : i2c1;

    i2c_init(g_i2c, p.baud);
    gpio_set_function(p.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(p.scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(p.sda_pin);
    gpio_pull_up(p.scl_pin);

    // Replace with your chosen address
    i2c_slave_init(g_i2c, p.slave_addr, &i2c_slave_handler);

    s_idx = 0;
}

void poll() {
    // If you handle nothing in polling, leave empty.
    // If you have non-IRQ work to do, do it here.
}

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
        case I2C_SLAVE_RECEIVE:
            if (s_idx < MAX_BUFFER) {
                s_buf[s_idx++] = i2c_read_byte_raw(i2c);
            }
            break;

        case I2C_SLAVE_REQUEST:
            // Optionally provide bytes back to the master
            i2c_write_byte_raw(i2c, 0);
            break;

        case I2C_SLAVE_FINISH:
            if (s_idx >= 4 && g_mgr) {
                const uint32_t cmdWord =  (uint32_t)s_buf[0]
                                         |((uint32_t)s_buf[1] << 8)
                                         |((uint32_t)s_buf[2] << 16)
                                         |((uint32_t)s_buf[3] << 24);

                uint8_t  cmdId      =  cmdWord        & 0x1F;
                uint8_t  flags      = (cmdWord >> 5)  & 0x07;
                uint8_t  screenId   = (cmdWord >> 8)  & 0x3F;
                uint32_t paramBits  = (cmdWord >> 14) & 0x3FFFF;

                const bool hasParams = (flags & 0b001) != 0;
                const bool useActive = (flags & 0b010) != 0;

                if (useActive) {
                    if (auto* scr = g_mgr->getActiveScreen()) {
                        screenId = scr->screenId;
                    }
                }

                const uint8_t* extraData = (s_idx > 4) ? &s_buf[4] : nullptr;
                size_t         extraLen  = (s_idx > 4) ? (s_idx - 4) : 0;

                // Your factory creates a command bound to the manager
                // (adjust signature if needed)
                i2cObj* cmd = createCommandObject(
                    cmdId, screenId, paramBits, extraData, extraLen, *g_mgr
                );
                if (cmd) {
                    // Do whatever you do with the command (execute or enqueue)
                    // cmd->execute(*g_mgr);
                    delete cmd;
                }
            }
            s_idx = 0;
            break;
    }
}

} // namespace i2c_common
