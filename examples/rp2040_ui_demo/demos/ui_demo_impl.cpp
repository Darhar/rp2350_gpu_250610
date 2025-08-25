#include "ui_demo.h"
#include "screenManager.hpp"        // full type here, not in the header
#include "i2c_common.h"
#include "command_factory.hpp"      // for command IDs, if needed
#include <pico/i2c_slave.h>

// demo-specific screen headers:
#include <testscreen.h>
#include <menuscreen.h>
#include <aboutscreen.h>
#include <settingsscreen.h>
#include <basicscreen.h>
#include <splashscreen.h>
#include "pico/multicore.h"

#include "pico/stdlib.h"
#include <hardware/i2c.h>

// Keep ScreenManager reference private to this TU
static ScreenManager* s_mgr = nullptr;

namespace ui_demo {
    void bind(ScreenManager& mgr) { s_mgr = &mgr; }

    void setup_master() {
        // set up i2c1 as master (dev only)
        gpio_init(I2C_MASTER_SDA_PIN);
        gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_MASTER_SDA_PIN);

        gpio_init(I2C_MASTER_SCL_PIN);
        gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_MASTER_SCL_PIN);

        i2c_init(i2c1, I2C_BAUDRATE);
    }


    void encodeCommand(uint8_t cmdId, uint8_t flags, uint8_t screenId, uint32_t paramBits, uint8_t (&bytes)[4]) {
        // Apply bit limits
        cmdId     &= 0x1F;       // 5 bits
        flags     &= 0x07;       // 3 bits
        screenId  &= 0x3F;       // 6 bits
        paramBits &= 0x3FFFF;    // 18 bits

        // Encode into 32-bit value
        uint32_t command = (cmdId)
                            | (flags << 5)
                            | (screenId << 8)
                            | (paramBits << 14);

        // Split into 4 bytes (big-endian order)
        bytes[0] = static_cast<uint8_t>(command & 0xFF);
        bytes[1] = static_cast<uint8_t>((command >> 8) & 0xFF);
        bytes[2] = static_cast<uint8_t>((command >> 16) & 0xFF);
        bytes[3] = static_cast<uint8_t>((command >> 24) & 0xFF);
    }


// Same packed shape used on the slave
struct __attribute__((packed)) KeyReport {
    uint8_t  key;        // 0..5
    uint8_t  stateBits;  // bit0 = DOWN
    uint8_t  edgeBits;   // bit0 = PRESS, bit1 = RELEASE (latched; cleared on read)
    uint8_t  analog;     // adc_read()/50 bucket
    uint32_t eventCount; // increments on edges
};

void run_master() {
    if (!s_mgr) return;

    // Build the 4-byte "get keyboard" command
    uint8_t cmd[4];
    encodeCommand(i2c_getKb, /*flags=*/0, /*screenId=*/0, /*paramBits=*/0, cmd);

    // Track last printed state
    static bool       havePrev = false;
    static KeyReport  prev{};

    // Poll quickly here; core1_entry already throttles outer loop if you want
    for (int i = 0; i < 100; ++i) {
        // 1) WRITE: send command and finish with STOP so slave stages reply on FINISH
        int w = i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, cmd, sizeof(cmd), /*nostop=*/false);
        if (w != (int)sizeof(cmd)) {
            // Only print write errors if we never printed one before to avoid spam
            static bool wroteErr = false;
            if (!wroteErr) {
                DEBUG_PRINTLN("i2c write failed: %d", w);
                wroteErr = true;
            }
            sleep_ms(200);
            continue;
        }

        // Tiny gap—usually safe without, but harmless
        sleep_us(1000);

        // 2) READ: fetch the 8-byte KeyReport
        KeyReport rep{};
        int r = i2c_read_blocking(i2c1, I2C_SLAVE_ADDRESS,
                                  reinterpret_cast<uint8_t*>(&rep), sizeof(rep),
                                  /*nostop=*/false);
        if (r != (int)sizeof(rep)) {
            static bool readErr = false;
            if (!readErr) {
                DEBUG_PRINTLN("i2c read failed: rc=%d", r);
                readErr = true;
            }
            sleep_ms(200);
            continue;
        }

        // Decide if we should print:
        // - Always print first sample
        // - Print on any edge (PRESS/RELEASE)
        // - Print if key index changed
        // - Print if DOWN bit changed
        // - Optionally print analog changes when no key is down (filter small jitter)
        bool changed = false;

        if (!havePrev) {
            changed = true;
        } else {
            const bool downNow  = (rep.stateBits & 0x01) != 0;
            const bool downPrev = (prev.stateBits & 0x01) != 0;

            if (rep.edgeBits != 0)                changed = true;                 // an edge happened
            else if (rep.key != prev.key)         changed = true;                 // different key bucket
            else if (downNow != downPrev)         changed = true;                 // up/down flipped
            else if (!downNow) {
                // Only when idle: print if analog bucket moved significantly
                // (1 bucket step can be noise; require >=2 steps)
                uint8_t d = (rep.analog > prev.analog) ? (rep.analog - prev.analog)
                                                       : (prev.analog - rep.analog);
                if (d >= 2) changed = true;
            }
        }

        if (changed) {
            const bool down = (rep.stateBits & 0x01) != 0;
            // Friendly edge text
            const bool pressed  = (rep.edgeBits & 0x01) != 0;
            const bool released = (rep.edgeBits & 0x02) != 0;

            if (pressed || released) {
                DEBUG_PRINTLN("KB edge: key=%u pressed=%u released=%u analog=%u events=%lu",
                              rep.key, pressed ? 1 : 0, released ? 1 : 0,
                              rep.analog, (unsigned long)rep.eventCount);
            } else if (down) {
                DEBUG_PRINTLN("KB hold: key=%u analog=%u events=%lu",
                              rep.key, rep.analog, (unsigned long)rep.eventCount);
            } else {
                DEBUG_PRINTLN("KB idle: analog=%u events=%lu",
                              rep.analog, (unsigned long)rep.eventCount);
            }

            prev = rep;
            havePrev = true;
        }

        sleep_ms(50); // poll period; tweak as needed
    }
}



    void run_master_orig() {
        if (!s_mgr) return;

        uint8_t buffer[4];
        //encodeCommand(cmdId,flags,screenId,paramBits, uint8_t (&bytes)[4])
        encodeCommand(i2c_scrCng, 0, 2, 0x12345, buffer);
        DEBUG_PRINTLN("run_master sending %d bytes\n",sizeof(buffer));

        i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, buffer, 4, false);
        sleep_ms(1000);
        encodeCommand(i2c_scrCng, 0, 1, 0x12345, buffer);

        i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, buffer, 4, false);
        sleep_ms(10000);
    }

    void core1_entry() {
        // Handshake exactly like your old version
        multicore_fifo_push_blocking(FLAG_VALUE);
        uint32_t g = multicore_fifo_pop_blocking();

        sleep_ms(200);
        if (g != FLAG_VALUE)
            printf("Problem with core 1!\n");
        else
            printf("Core 1 up\n");

        while (true) {
            run_master();       // your dev traffic
            // you can also peek s_mgr->getActiveScreen() if you kept a pointer
            sleep_ms(5000);
        }
    }

} // namespace ui_demo

void registerAllScreens(ScreenManager& mgr) {
  mgr.registerScreen(ScreenEnum::MENUSCREEN,     [&mgr]{ return new MenuScreen(mgr); });
  mgr.registerScreen(ScreenEnum::TESTSCREEN,     [&mgr]{ return new TestScreen(mgr); });
  mgr.registerScreen(ScreenEnum::ABOUTSCREEN,    [&mgr]{ return new AboutScreen(mgr); });
  mgr.registerScreen(ScreenEnum::SETTINGSSCREEN, [&mgr]{ return new SettingsScreen(mgr); });
  mgr.registerScreen(ScreenEnum::BASICSCREEN,    [&mgr]{ return new BasicScreen(mgr); });
  mgr.registerScreen(ScreenEnum::SPLASHSCREEN,   [&mgr]{ return new SplashScreen(mgr); });
}
