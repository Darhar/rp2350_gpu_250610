
// ---------------- Core-1 (master) demo driver ----------------
// Drives TestScreen widgets via ValueStore over I²C only.
// - Edit wid=2 (int) starts at 30, Button wid=3 (bool) starts ON
// - While TestScreen active: Edit ticks 1/sec; 30..70 sawtooth, Button flips at edges
// - Adopts user commits immediately, pauses briefly so UI "wins"
// - Stops when another screen is active; resumes where it left off

#include <cstdint>
#include <cstdio>
#include "pico/stdlib.h"
#include <hardware/i2c.h>
#include "screenManager.hpp"        // full type here, not in the header
#include "command_factory.hpp"      // for command IDs, if needed
#include "i2c_common.h"     // I2C_SLAVE_ADDRESS, pins, baud
#include "sys_status.hpp"   // SysStat bits (VS_FROZEN, C1_READY, etc.)
#include <climits>
#include <testscreen.h>
#include <menuscreen.h>
#include <aboutscreen.h>
#include <settingsscreen.h>
#include <basicscreen.h>
#include <splashscreen.h>

#define COUNTPACE 1000

namespace vs_demo {

    // ---------- Wire types (must match ValueType) ----------
    enum : uint8_t { WT_Nil=0, WT_Bool=1, WT_Int=2, WT_U32=3 };

    // ---------- VSIDs (adjust b-values if your project uses different ones) ----------
    static constexpr uint8_t  CAT_SYS          = 4;   // ValueCat::System
    static constexpr uint8_t  CAT_WIDGET       = 0;   // ValueCat::Widget

    static constexpr uint16_t K_SYS_STATUS_A   = 0;
    static constexpr uint16_t K_SYS_STATUS_B   = 0x30;

    static constexpr uint16_t K_ACTIVE_SCR_A   = 0;
    static constexpr uint16_t K_ACTIVE_SCR_B   = 12;

    static constexpr uint16_t K_UI_COMMIT_A    = 0;
    static constexpr uint16_t K_UI_COMMIT_B    = 50;

    // TestScreen widget IDs (adjust if different)
    static constexpr uint16_t SID_TEST         = 1;   // ScreenEnum::TESTSCREEN
    static constexpr uint16_t WID_EDIT         = 2;
    static constexpr uint16_t WID_BTN          = 3;

    // ---------- Demo state ----------
    static int      g_edit          = 30;       // starts at 30
    static bool     g_btn           = true;     // ON means count up
    static uint32_t g_ver_edit      = 0;
    static uint32_t g_ver_btn       = 0;
    static int      g_last_sent_edit= INT_MIN;
    static int      g_last_sent_btn = -1;

    static uint32_t g_last_ui_commit= 0;
    static uint32_t g_hold_until_ms = 0;        // quiet window after user commit
    static uint32_t g_last_tick_ms  = 0;        // 1 Hz pacing

    // ---------- Command header encoder (matches your existing 32-bit header layout) ----------
    static inline void encodeCommand(uint8_t cmdId, uint8_t flags, uint8_t screenId, uint32_t paramBits, uint8_t(&bytes)[4]) {
        cmdId     &= 0x1F;    // 5 bits
        flags     &= 0x07;    // 3 bits
        screenId  &= 0x3F;    // 6 bits
        paramBits &= 0x3FFFF; // 18 bits
        const uint32_t u = (uint32_t)cmdId | ((uint32_t)flags << 5) | ((uint32_t)screenId << 8) | (paramBits << 14);
        bytes[0] = (uint8_t)(u >> 0);
        bytes[1] = (uint8_t)(u >> 8);
        bytes[2] = (uint8_t)(u >> 16);
        bytes[3] = (uint8_t)(u >> 24);
    }

    // ---------- Low-level I2C helpers ----------
    static inline uint32_t now_ms() { return to_ms_since_boot(get_absolute_time()); }

    static bool read_ack(uint8_t& out_st) {
        DEBUG_PRINTLN("");

        uint8_t hdr[4];
        encodeCommand((uint8_t)i2cCmnds::i2c_ack, 0, 0, 0, hdr);
        if (i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, hdr, 4, false) != 4) return false;
        return i2c_read_blocking(i2c1, I2C_SLAVE_ADDRESS, &out_st, 1, false) == 1;
    }

    // set widget values in VS
    // SET: [header(i2c_vs_set)=4] + [addr=5] + [val=5]
    static bool vs_set_w(uint8_t cat, uint16_t a, uint16_t b, uint8_t type, uint32_t v) {
        uint8_t hdr[4]; 
        encodeCommand((uint8_t)i2cCmnds::i2c_vs_set, 0, 0, 0, hdr);
        uint8_t frame[4+5+5] = {
            hdr[0],hdr[1],hdr[2],hdr[3],
            cat,    (uint8_t)(a>>0),
                    (uint8_t)(a>>8),
                    (uint8_t)(b>>0),
                    (uint8_t)(b>>8),
            type,   (uint8_t)(v>>0),
                    (uint8_t)(v>>8),
                    (uint8_t)(v>>16),
                    (uint8_t)(v>>24)
        };
        int w = i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, frame, sizeof(frame), false);
        return w == (int)sizeof(frame);
    }

    // STATUS: [header(i2c_vs_status)=4] + [addr=5] + [options=1] → read 10B (type,u32,val,ver,dirty)
    //return true if successfully sent and rec status command

    static bool vs_status(uint8_t cat, uint16_t a, uint16_t b,
                        uint8_t& out_type, uint32_t& out_val,
                        uint32_t& out_ver, uint8_t& out_dirty,
                        uint8_t& out_origin,
                        bool clearDirty=false)
    {
        uint8_t hdr[4]; 
        uint8_t rx[10]{}; // type(1)+val(4)+ver(4)+dirty(1)
        encodeCommand((uint8_t)i2cCmnds::i2c_vs_status, 0, 0, 0, hdr);
        uint8_t tx[4+6] = {
            hdr[0],hdr[1],hdr[2],hdr[3],
            cat,(uint8_t)(a>>0),
                (uint8_t)(a>>8),
                (uint8_t)(b>>0),
                (uint8_t)(b>>8),
                (uint8_t)(clearDirty ? 1 : 0)
        };
        if (i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, tx, sizeof(tx), false) != (int)sizeof(tx)){
            printf("[c1] vs_status: couldnt write i2c");
          return false;  
        } 
        if (i2c_read_blocking(i2c1, I2C_SLAVE_ADDRESS, rx, sizeof(rx), false) != (int)sizeof(rx)){
            printf("[c1] vs_status: couldnt resd i2c");
            return false;
        } 
        //set by reference
        out_type  = rx[0];
        out_val   = (uint32_t)rx[1] | ((uint32_t)rx[2]<<8) | ((uint32_t)rx[3]<<16) | ((uint32_t)rx[4]<<24);
        out_ver   = (uint32_t)rx[5] | ((uint32_t)rx[6]<<8) | ((uint32_t)rx[7]<<16) | ((uint32_t)rx[8]<<24);
        // was: out_dirty = rx[9] & 0x01;
        out_dirty = rx[9] & 0x01;
        uint8_t origin = rx[9] >> 1;          // 0=Unknown, 1=UI_C0, 2=MASTER_C1, ...
        out_origin = rx[9] >> 1;
        return true;
    }

    static bool get_sys_status_u32(uint32_t& out_status) {
        uint8_t t=0, d=0, origin=0; 
        uint32_t v=0, ver=0;
        //get status
        if (!vs_status(CAT_SYS, K_SYS_STATUS_A, K_SYS_STATUS_B, t, v, ver, origin,d)) return false;
        if (t != WT_U32) return false;
        out_status = v;
        return true;
    }

    // Restrict writes to C1-owned bits
    static bool set_c1_bits(uint32_t set_mask, uint32_t clear_mask) {
        uint32_t s=0; 
        if (!get_sys_status_u32(s)) return false;
        s &= ~clear_mask;
        s |= (set_mask & SysStat::OWNED_BY_C1);
        return vs_set_w(CAT_SYS, K_SYS_STATUS_A, K_SYS_STATUS_B, WT_U32, s);
    }

    static void getCommit() {
        //DEBUG_PRINTLN("");

        uint8_t t=0,d=0, origin=0; 
        uint32_t v=0,ver=0;
        //get commit
        if (!vs_status(CAT_SYS, K_UI_COMMIT_A, K_UI_COMMIT_B, t, v, ver, origin,d)) return;
        if (v == g_last_ui_commit) return;        // no new commit
        g_last_ui_commit = v;

        // Adopt both widgets now
        d=0; 
        v=0;
        //get edit widget
        if (vs_status(CAT_WIDGET, SID_TEST, WID_EDIT, t, v, ver, origin,d,true)) {
            g_edit     = (int)v;
            printf("[DRV] ADOPT(edit) val=%d dirty=%u\n", g_edit, (unsigned)d);
        }
        // BTN
        d=0; 
        v=0;
        //get button widget
        if (vs_status(CAT_WIDGET, SID_TEST, WID_BTN, t, v, ver, origin,d,true)) {
            g_btn     = (v != 0);
            printf("[DRV] ADOPT(btn) val=%u dirty=%u\n", (unsigned)g_btn, (unsigned)d);
        }

        // Hold off our writes briefly so the UI's value wins visually
        g_hold_until_ms = now_ms() + 350;
        // printf("[DRV] UI_COMMIT=%lu adopt edit=%d btn=%u\n", (unsigned long)v, g_edit, (unsigned)g_btn);
    }


    // ---------- 1Hz driver, gated by active screen ----------
    static void run_master() {

        uint8_t t=0,d=0, origin=0; 
        uint32_t v=0,ver=0;
        
        // get active screen
        if (!vs_status(CAT_SYS, K_ACTIVE_SCR_A, K_ACTIVE_SCR_B, t, v, ver,origin, d) || t != WT_U32) return;
        if (v == !SID_TEST) return;
        v=0;
        //get commit
        if (!vs_status(CAT_SYS, K_UI_COMMIT_A, K_UI_COMMIT_B, t, v, ver, origin,d))return;    

        if (v != g_last_ui_commit){// new commit
    
            g_last_ui_commit = v;
            // Adopt both widgets
            d=0; 
            v=0;
            //get edit widget
            if (vs_status(CAT_WIDGET, SID_TEST, WID_EDIT, t, v, ver, origin,d,true)) {
                g_edit     = (int)v;
                printf("[DRV] ADOPT(edit) val=%d dirty=%u\n", g_edit, (unsigned)d);
            }

            d=0; 
            v=0;
            //get button widget
            if (vs_status(CAT_WIDGET, SID_TEST, WID_BTN, t, v, ver, origin,d,true)) {
                g_btn     = (v != 0);
                printf("[DRV] ADOPT(btn) val=%u dirty=%u\n", (unsigned)g_btn, (unsigned)d);
            }

            // Hold off our writes briefly so the UI's value wins visually
            g_hold_until_ms = now_ms() + 350;
            // printf("[DRV] UI_COMMIT=%lu adopt edit=%d btn=%u\n", (unsigned long)v, g_edit, (unsigned)g_btn);

        }        

        const uint32_t now = now_ms();
        if (now < g_hold_until_ms) return;            


        //Pace
        if (now - g_last_tick_ms < COUNTPACE) return;
        g_last_tick_ms = now;

        //
        g_edit += (g_btn ? +1 : -1);
        if (g_edit > 70) { g_edit = 70; g_btn = false; }
        if (g_edit < 30) { g_edit = 30; g_btn = true;  }

        if (g_edit != g_last_sent_edit) {
            if (vs_set_w(CAT_WIDGET, SID_TEST, WID_EDIT, WT_Int, (uint32_t)g_edit)) {
                g_last_sent_edit = g_edit;
            }
        }

        const int b = g_btn ? 1 : 0;
        if (b != g_last_sent_btn) {
            if (vs_set_w(CAT_WIDGET, SID_TEST, WID_BTN, WT_Bool, (uint32_t)b)) {
                g_last_sent_btn = b;
            }
        }
    }

    // ---------- Public entry points ----------
    void setup_master() {
        gpio_init(I2C_MASTER_SDA_PIN);
        gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_MASTER_SDA_PIN);

        gpio_init(I2C_MASTER_SCL_PIN);
        gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_MASTER_SCL_PIN);

        i2c_init(i2c1, I2C_BAUDRATE);
    }

    void core1_entry() {
        DEBUG_PRINTLN("");
        printf("Core 1 up\n");

        // Peek ACK once (optional sanity)
        if (uint8_t ack; read_ack(ack)) {
            printf("[C1] first ACK=0x%02X (frozen=%u)\n", ack, (ack>>5)&1);
        }

        // Wait until Core-0 seeds SYS_STATUS with VS_FROZEN
        for (;;) {
            uint32_t s=0;
            bool ok = get_sys_status_u32(s);
            if (ok && (s & SysStat::VS_FROZEN)) break;
            sleep_ms(100);
        }

        // Mark C1 ready (optional)
        (void)set_c1_bits(SysStat::C1_READY, 0);

        // Seed initial requested values once (Edit=30, Btn=ON)
        (void)vs_set_w(CAT_WIDGET, SID_TEST, WID_EDIT, WT_Int,  30);
        (void)vs_set_w(CAT_WIDGET, SID_TEST, WID_BTN,  WT_Bool, 1);
        g_edit = 30; g_btn = true;
        g_last_sent_edit = INT_MIN; // force send if needed
        g_last_sent_btn  = -1;
        printf("[c1] start loop\n");

        // Loop
        for (;;) {
            (void)set_c1_bits(SysStat::C1_BUSY, 0);
            run_master();
            (void)set_c1_bits(0, SysStat::C1_BUSY);
            (void)set_c1_bits(SysStat::LINK_OK, 0);
            sleep_ms(20); // ~50 Hz housekeeping; driver is 1 Hz internally
        }
    }

} // namespace vs_demo
void registerAllScreens(ScreenManager& mgr) {
    mgr.registerScreen(ScreenEnum::MENUSCREEN,     [&mgr]{ return new MenuScreen(mgr); });
    mgr.registerScreen(ScreenEnum::TESTSCREEN,     [&mgr]{ return new TestScreen(mgr); });
    mgr.registerScreen(ScreenEnum::ABOUTSCREEN,    [&mgr]{ return new AboutScreen(mgr); });
    mgr.registerScreen(ScreenEnum::SETTINGSSCREEN, [&mgr]{ return new SettingsScreen(mgr); });
    mgr.registerScreen(ScreenEnum::BASICSCREEN,    [&mgr]{ return new BasicScreen(mgr); });
    mgr.registerScreen(ScreenEnum::SPLASHSCREEN,   [&mgr]{ return new SplashScreen(mgr); });
}

