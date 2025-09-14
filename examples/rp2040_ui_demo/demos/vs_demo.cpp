
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
#include "sys_status.hpp"

#ifndef WAITFORSERIAL
  #define WAITFORSERIAL 1
#endif

Debug debug;

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

    i2c_common::bind(screenMgr);
    i2c_common::setKeyboard(*keyboard);   

    registerAllScreens(screenMgr);
    // 1) Declare system/status keys and widget keys, then freeze once
    auto& vs = ValueStore::instance();
    const uint16_t SID_TEST = to_screen_id_6(ScreenEnum::TESTSCREEN);
    vs.declareU32(VSIDs::K_SYS_STATUS, 0);
    vs.declareU32(VSIDs::K_SYS_EPOCH,  0);
    vs.declareU32(VSIDs::K_ACTIVE_SCREEN, static_cast<uint32_t>(ScreenEnum::MENUSCREEN));
    vs.declareU32(VSIDs::K_UI_COMMIT,  0);   // ← required so Core-1 can see user commits
    vs.declareInt (VKey(ValueCat::Widget, SID_TEST, 2), 50);   // Edit default (any)
    vs.declareBool(VKey(ValueCat::Widget, SID_TEST, 3), true); // Button default (any)
    // Freeze once
    (void)vs.freeze();

    // 2) Seed SYS_STATUS: VS_FROZEN + PROTO + EPOCH
    uint32_t epoch = vs.getU32(VSIDs::K_SYS_EPOCH).value_or(0) + 1;
    (void)vs.setU32(VSIDs::K_SYS_EPOCH, epoch);

    uint32_t s = 0;
    s |= SysStat::VS_FROZEN;                         // bit 5 (0x20)  ← you printed 0x01010020, good
    s |= SysStat::PROTO_VER;                         // 0x00010000
    s |= (epoch & 0xFFu) << SysStat::EPOCH_SHIFT;    // 0x01000000 if epoch==1
    (void)vs.setU32(VSIDs::K_SYS_STATUS, s);
    printf("[C0] seeded SYS_STATUS=0x%08lX\n", (unsigned long)s);

    // 3) Init I²C slave BEFORE launching Core1, so vs_status can be served
    i2c_common::Params p{
        .i2c_index = 0,
        .sda_pin   = I2C_SLAVE_SDA_PIN,
        .scl_pin   = I2C_SLAVE_SCL_PIN,
        .baud      = I2C_BAUDRATE,
        .slave_addr= I2C_SLAVE_ADDRESS
    };
    i2c_common::init(p);

    // 4) Mark UI ready/active (C0-owned bits), *then* launch Core1
    s = vs.getU32(VSIDs::K_SYS_STATUS).value_or(0);
    s |= (SysStat::C0_READY | SysStat::UI_ACTIVE);
    (void)vs.setU32(VSIDs::K_SYS_STATUS, s);
    printf("[C0] ready SYS_STATUS=0x%08lX\n", (unsigned long)s);

    vs_demo::setup_master();
    printf("Core 0 up\n");

    // 5) Launch Core1 now that I²C+status are live
    multicore_launch_core1(vs_demo::core1_entry);

    // 6) Pick initial screen (e.g. Menu splash that doesn’t need VS)
    screenMgr.setActiveScreen(ScreenEnum::MENUSCREEN);

    // I2C slave, keyboard, etc. are now ready → raise C0_READY + UI_ACTIVE
    s = vs.getU32(VSIDs::K_SYS_STATUS).value_or(0);
    s |= (SysStat::C0_READY | SysStat::UI_ACTIVE);
    (void)vs.setU32(VSIDs::K_SYS_STATUS, s);

    // Launch Core1 — no FIFO handshake needed anymore
    DEBUG_PRINTLN(" main loop");

    while (true) {
        uint16_t deltaTimeMS = getTimeDiffMS(lastUpdate);
        lastUpdate = getTime();

        // Core0 main loop — keep mirrors fresh (cheap)
        static uint32_t last_s = 0;
        uint32_t scur = vs.getU32(VSIDs::K_SYS_STATUS).value_or(0);

        // Mirror anyDirty + first dirty bank (if you have it; else leave 0xF)
        const bool any = vs.anyDirty();
        scur = (scur & ~SysStat::ANY_DIRTY) | (any ? SysStat::ANY_DIRTY : 0u);

        // (optional) uint8_t fdb = any ? first_dirty_bank() : 0xF;
        // scur = (scur & ~SysStat::FDB_MASK) | (uint32_t(fdb) << SysStat::FDB_SHIFT);

        if (scur != last_s) { 
            (void)vs.setU32(VSIDs::K_SYS_STATUS, scur); 
            last_s = scur; 
        }


        debug.poll();
        keyboard->checkKeyState(&screenMgr);
        screenMgr.update(deltaTimeMS);

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
