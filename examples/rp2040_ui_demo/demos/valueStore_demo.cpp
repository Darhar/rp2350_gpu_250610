
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
#include "value_store.h"  // add this include

#ifndef WAITFORSERIAL
  #define WAITFORSERIAL 0
#endif
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
        i2c_common::poll();

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
