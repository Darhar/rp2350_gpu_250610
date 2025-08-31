// lib/src/i2c_common.cpp
#include "i2c_common.h"
#include "screenManager.hpp"         // full type only in .cpp
#include "command_factory.hpp"
#include "keyboard.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <pico/i2c_slave.h>
#include "debug.h"

namespace i2c_common {

    static ScreenManager* g_mgr = nullptr;
    static KeyBoard*      g_kbd = nullptr; 
    static i2c_inst_t*    g_i2c = nullptr;

    static constexpr int MAX_BUFFER = 64;
    static uint8_t  s_buf[MAX_BUFFER];
    static volatile int s_idx = 0;

    // Simple TX buffer for the response to the next read
    static uint8_t  s_tx[MAX_BUFFER];
    static volatile int s_tx_len = 0;
    static volatile int s_tx_pos = 0;

    // Forward so we can reference it in init()
    static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);

    void bind(ScreenManager& mgr) { g_mgr = &mgr; }
    void setKeyboard(KeyBoard& kbd)      { g_kbd = &kbd; }   // <— NEW overload
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

            case I2C_SLAVE_REQUEST:{
                // Optionally provide bytes back to the master
                uint8_t b = 0x00;
                if (s_tx_pos < s_tx_len) {
                    b = s_tx[s_tx_pos++];
                }
                i2c_write_byte_raw(i2c, b);
                break;
            }

            case I2C_SLAVE_FINISH:{
                if (s_idx >= 4 && g_mgr) {
                    const uint32_t cmdWord =  
                        (uint32_t)s_buf[0]
                        |((uint32_t)s_buf[1] << 8)
                        |((uint32_t)s_buf[2] << 16)
                        |((uint32_t)s_buf[3] << 24);

                    //uint8_t  cmdId      =  cmdWord        & 0x1F;
                    // decode header...
                    uint8_t cmdIdRaw =  cmdWord        & 0x1F;
                    // Cast once to your scoped enum
                    i2cCmnds cmdId   = static_cast<i2cCmnds>(cmdIdRaw);

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
//DEBUG_PRINTLN("I2C FINISH: bytes=%u cmdIdRaw=%u extraLen=%u",(unsigned)s_idx, (unsigned)cmdIdRaw, (unsigned)extraLen);                

                    // pass enum to factory
                    i2cObj* obj = createCommandObject(
                        cmdId, screenId, paramBits, extraData, extraLen, *g_mgr, g_kbd); 

                    /*
                        i2cObj* obj = createCommandObject(
                            cmdId, screenId, paramBits, extraData, extraLen, *g_mgr,
                            (g_kbd ? *g_kbd : *(KeyBoard*)nullptr)  // will crash if null; better: guard below
                        );
                    */

                    // Stage response bytes for subsequent REQUEST events
                    s_tx_len = 0;
                    s_tx_pos = 0;

                    if (obj) {
                        size_t respLen = 0;
                        const uint8_t* resp = obj->getResponse(respLen);
                        if (resp && respLen > 0) {
                            if (respLen > (size_t)MAX_BUFFER) respLen = MAX_BUFFER;
                            memcpy(s_tx, resp, respLen);
                            s_tx_len = (int)respLen;
                            s_tx_pos = 0;
                        }
                        delete obj;
                    }
                }
                s_idx = 0;
                break;
            }
        }//switch
    }//i2c_slave_handler

} // namespace i2c_common


  
