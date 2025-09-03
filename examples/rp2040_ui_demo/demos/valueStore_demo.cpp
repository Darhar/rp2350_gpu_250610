
#include "pico/stdlib.h"
#include "common.h"
#include "screenManager.hpp"
#include "i2c_common.h"
#include "valueStore_demo.h"
#include "common.h"
#include "display.h"

#include "pico/multicore.h"
#include "debug.h"
#include "keyboard.h"
#include "value_store.h" 

#ifndef WAITFORSERIAL
  #define WAITFORSERIAL 1
#endif

#ifndef TEST_LOCAL_VS_WRITES
#define TEST_LOCAL_VS_WRITES 0
#endif

static constexpr ValueKey K_KB_MASK = VKey(ValueCat::Keyboard,0,0);
static constexpr ValueKey K_WIFI    = VKey(ValueCat::System,  0,1);
static constexpr ValueKey K_MODE    = VKey(ValueCat::System,  0,2);

Debug debug;

static void build_and_freeze_values() {
    auto& vs = ValueStore::instance();
    vs.declareU32(VKey(ValueCat::Keyboard, 0, 0), 0);   // kb mask
    vs.declareBool(VKey(ValueCat::System,   0, 1), false); // wifi
    vs.declareInt (VKey(ValueCat::System,   0, 2), 0);     // mode
    vs.freeze();
}

int main() 
{
    stdio_init_all();
    srand((unsigned int)time(0));

    uint32_t msSinceBoot = to_ms_since_boot(get_absolute_time());
    timetype lastUpdate  = getTime();

    // FIX: use logical && so WAITFORSERIAL acts as a guard
    while (!stdio_usb_connected() && WAITFORSERIAL) {
        sleep_ms(10);  // Wait for USB host to open the port
    }

    sleep_ms(500);
    printf("Running demo: %s\n", RP_DEMO_NAME);//RP_DEMO_NAME from CMakeLists.txt file

    Display* display = new Display();

    display->clear(0);
    debug.printHelp();

    KeyBoard* keyboard = new KeyBoard();

    // NEW: make the manager local to main (no header-defined globals)
    ScreenManager screenMgr;

    // NEW: demo-specific bindings & registrations live in ui_demo_impl.cpp
    vs_demo::bind(screenMgr);
    i2c_common::bind(screenMgr);
    i2c_common::setKeyboard(*keyboard);   
    i2c_common::Params p{
        .i2c_index = 0,
        .sda_pin   = I2C_SLAVE_SDA_PIN,
        .scl_pin   = I2C_SLAVE_SCL_PIN,
        .baud      = I2C_BAUDRATE,
        .slave_addr= I2C_SLAVE_ADDRESS
    };
    i2c_common::init(p);
    build_and_freeze_values();   // <-- add this line (before launching Core1)

    registerAllScreens(screenMgr);
    screenMgr.setActiveScreen(ScreenEnum::MENUSCREEN);
    // NEW: demo-only I2C MASTER (dev helper)
    vs_demo::setup_master();

    // CHANGED: launch core1 entry provided by the demo implementation
    multicore_launch_core1(vs_demo::core1_entry);

    uint32_t g = multicore_fifo_pop_blocking(); 
    sleep_ms(1000);

    if (g != FLAG_VALUE) {
        printf("Problem with core 0!\n");
    } else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("Core 0 up\n");
    }

    DEBUG_PRINTLN(" main loop");

    while (true) {
        uint16_t deltaTimeMS = getTimeDiffMS(lastUpdate);
        lastUpdate = getTime();

        debug.poll();
        keyboard->checkKeyState(&screenMgr);
        screenMgr.update(deltaTimeMS);

        // NEW: give the slave a chance to do any polling work (noop if IRQ-only)
        //i2c_common::poll();


        #if TEST_LOCAL_VS_WRITES
        {
            static uint32_t tick_ms = 0;
            tick_ms += deltaTimeMS;                 // you already compute deltaTimeMS each loop
            if (tick_ms >= 1000) {                  // every 1s
                tick_ms = 0;

                static bool     wifi = false;
                static int      mode = 0;           // 0..2
                static uint32_t kb   = 0x00000001;  // walking bit

                auto& vs = ValueStore::instance();
                if (vs.frozen()) {
                    // Use your shared demo keys (from valueStore_demo.h)
                    vs.setBool(K_WIFI,    wifi);
                    vs.setInt (K_MODE,    mode);
                    vs.setU32 (K_KB_MASK, kb);

                    printf("[CORE0] local set: wifi=%d mode=%d kb=0x%08lx\n",
                        wifi ? 1 : 0, mode, (unsigned long)kb);
                }

                // advance patterns for next tick
                wifi = !wifi;
                mode = (mode + 1) % 3;
                kb <<= 1;
                if ((kb & 0xFF) == 0) kb = 0x01;    // keep it in low byte for easy viewing
            }
        }
        #endif

        if (screenMgr.needRefresh()) {
            display->clear(0);
            screenMgr.draw(display);
            display->update();
            screenMgr.setRefreshRect(Rect2(0,0,0,0));
        } else {
            sleep_ms(20);
        }

        msSinceBoot = to_ms_since_boot(get_absolute_time());
    }
}
