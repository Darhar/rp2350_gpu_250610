#ifndef W2_DRIVE
    #define W2_DRIVE 1   // turn OFF while testing user commit
#endif
// Keep ScreenManager reference private to this TU
static ScreenManager* s_mgr = nullptr;

// --- wire helpers ---
static inline void pack_addr(uint8_t* out, uint8_t cat, uint16_t a, uint16_t b) {
    out[0] = cat; 
    out[1] = (uint8_t)(a>>0); out[2] = (uint8_t)(a>>8);
    out[3] = (uint8_t)(b>>0); out[4] = (uint8_t)(b>>8);
}
static inline void pack_val(uint8_t* out, uint8_t type, uint32_t v) {
    out[0] = type; 
    out[1] = (uint8_t)(v>>0); out[2] = (uint8_t)(v>>8);
    out[3] = (uint8_t)(v>>16); out[4] = (uint8_t)(v>>24);
}

namespace vs_demo {

    // ValueStore wire types (must match ValueType)
    enum : uint8_t { WT_Nil=0, WT_Bool=1, WT_Int=2, WT_U32=3 };

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

    // --- send SET: header + addr(5) + val(5) in ONE write (14 bytes) ---
    static bool vs_set_w(uint8_t cat, uint16_t a, uint16_t b, uint8_t type, uint32_t v) {
        uint8_t frame[4 + 5 + 5]; uint8_t* p = frame;
        uint8_t hdr[4];
        encodeCommand(static_cast<uint8_t>(i2cCmnds::i2c_vs_set), 0, 0, 0, hdr);
        p[0]=hdr[0]; p[1]=hdr[1]; p[2]=hdr[2]; p[3]=hdr[3]; p+=4;
        pack_addr(p, cat, a, b); p+=5;
        pack_val (p, type, v);   p+=5;
        int w = i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, frame, sizeof(frame), /*nostop=*/false);
        return w == (int)sizeof(frame);
    }

    // --- send GET: write [header+addr] (9 bytes), then read 5 bytes reply ---
    static bool vs_get_r(uint8_t cat, uint16_t a, uint16_t b, uint8_t& outType, uint32_t& outVal) {
        uint8_t frame[4 + 5]; uint8_t* p = frame;
        uint8_t hdr[4];
        encodeCommand(static_cast<uint8_t>(i2cCmnds::i2c_vs_get), 0, 0, 0, hdr);
        p[0]=hdr[0]; p[1]=hdr[1]; p[2]=hdr[2]; p[3]=hdr[3]; p+=4;
        pack_addr(p, cat, a, b); p+=5;

        int w = i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, frame, sizeof(frame), /*nostop=*/false);
        if (w != (int)sizeof(frame)) return false;

        uint8_t resp[5]{};
        int r = i2c_read_blocking(i2c1, I2C_SLAVE_ADDRESS, resp, sizeof(resp), /*nostop=*/false);
        if (r != (int)sizeof(resp)) return false;

        outType = resp[0];
        outVal  = (uint32_t)resp[1] | ((uint32_t)resp[2] << 8)
                | ((uint32_t)resp[3] << 16) | ((uint32_t)resp[4] << 24);
        return true;
    }


    static void checkVS() {
        // Demo pattern
        static uint32_t kb = 0x00000001;
        static bool     wifi = false;
        static int32_t  mode = 0;

        // 1) Push three values
        bool ok_kb   = vs_set_w(static_cast<uint8_t>(ValueCat::Keyboard), 0, 0, WT_U32, kb);
        bool ok_wifi = vs_set_w(static_cast<uint8_t>(ValueCat::System),   0, 1, WT_Bool, wifi ? 1u : 0u);
        bool ok_mode = vs_set_w(static_cast<uint8_t>(ValueCat::System),   0, 2, WT_Int,  (uint32_t)mode);

        // 2) Read one back to verify end-to-end
        uint8_t  t=0; uint32_t v=0; bool okr = vs_get_r(static_cast<uint8_t>(ValueCat::System), 0, 2, t, v);

        printf("[CORE1] VS set: kb=0x%08lx wifi=%d mode=%ld  (%c%c%c)  get(mode)=(t=%u v=%ld) %c\n",
            (unsigned long)kb, wifi?1:0, (long)mode,
            ok_kb?'K':'k', ok_wifi?'W':'w', ok_mode?'M':'m',
            (unsigned)t, (long)(int32_t)v, okr?'Y':'N');

        // advance pattern
        kb <<= 1; if (kb == 0 || kb > 0x00000080) kb = 0x00000001;
        wifi = !wifi;
        mode = (mode + 1) % 3;
    }

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

    // Same packed shape used on the slave
    struct __attribute__((packed)) KeyReport {
        uint8_t  key;        // 0..5
        uint8_t  stateBits;  // bit0 = DOWN
        uint8_t  edgeBits;   // bit0 = PRESS, bit1 = RELEASE (latched; cleared on read)
        uint8_t  analog;     // adc_read()/50 bucket
        uint32_t eventCount; // increments on edges
    };

    void checkKeys(){
        if (!s_mgr) return;

        // Build the 4-byte "get keyboard" command
        uint8_t cmd[4];
        encodeCommand(static_cast<uint8_t>(i2cCmnds::i2c_getKb), /*flags=*/0, /*screenId=*/0, /*paramBits=*/0, cmd);

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

    static bool request_ack(uint8_t& out_status) {
        uint8_t hdr[4];
        encodeCommand(static_cast<uint8_t>(i2cCmnds::i2c_ack), /*flags*/0, /*screenId*/0, /*paramBits*/0, hdr);

        int w = i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, hdr, sizeof(hdr), /*nostop=*/false);
        if (w != (int)sizeof(hdr)) return false;

        uint8_t st = 0xFF;
        int r = i2c_read_blocking(i2c1, I2C_SLAVE_ADDRESS, &st, 1, /*nostop=*/false);
        if (r != 1) return false;

        out_status = st;
        return true;
    }

    static void checkAck() {
        for (int i = 0; i < 10; ++i) {
            uint8_t st = 0xFF;
            bool ok = request_ack(st);
            if (!ok) {
                DEBUG_PRINTLN("ACK: write/read failed");
            } else {
                // Decode bits: 0=alive,1=vsFrozen,2=hasMgr,3=hasKbd,4=rxOverflow (optional)
                DEBUG_PRINTLN("ACK: 0x%02X  alive=%u vsFrozen=%u mgr=%u kbd=%u ovf=%u",
                            st, (st>>0)&1, (st>>1)&1, (st>>2)&1, (st>>3)&1, (st>>4)&1);
            }
            sleep_ms(500);
        }
    }
    static bool read_ack(uint8_t& out) {
        uint8_t hdr[4];
        encodeCommand(static_cast<uint8_t>(i2cCmnds::i2c_ack), 0, 0, 0, hdr);
        if (i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, hdr, sizeof(hdr), false) != 4) return false;
        return i2c_read_blocking(i2c1, I2C_SLAVE_ADDRESS, &out, 1, false) == 1;
    }
      


    static bool vs_status_req(uint8_t cat, uint16_t a, uint16_t b, uint8_t options,
                            uint8_t& type, uint32_t& val, uint32_t& ver, uint8_t& dirty)
    {
        uint8_t hdr[4];
        encodeCommand(static_cast<uint8_t>(i2cCmnds::i2c_vs_status), 0, 0, 0, hdr);

        uint8_t frame[4 + 6] = {
            hdr[0], hdr[1], hdr[2], hdr[3],
            cat,
            (uint8_t)(a & 0xFF), (uint8_t)(a >> 8),
            (uint8_t)(b & 0xFF), (uint8_t)(b >> 8),
            options
        };

        if (i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, frame, sizeof(frame), false) != (int)sizeof(frame))
            return false;

        uint8_t resp[10]{};
        if (i2c_read_blocking(i2c1, I2C_SLAVE_ADDRESS, resp, sizeof(resp), false) != (int)sizeof(resp))
            return false;

        type  = resp[0];
        val   = (uint32_t)resp[1] | ((uint32_t)resp[2] << 8) |
                ((uint32_t)resp[3] << 16) | ((uint32_t)resp[4] << 24);
        ver   = (uint32_t)resp[5] | ((uint32_t)resp[6] << 8) |
                ((uint32_t)resp[7] << 16) | ((uint32_t)resp[8] << 24);
        dirty = resp[9];
        return true;
    }



static inline uint32_t now_ms() {
    return to_ms_since_boot(get_absolute_time());
}

// Coupled controller state
struct EditButtonDriver {
    // Key (Widget, screenId, widgetId)
    uint16_t sid      = 1;   // updated from active screen each tick
    uint16_t wid_edit = 2;   // Edit widget id
    uint16_t wid_btn  = 3;   // Button widget id

    // Targets we try to achieve/maintain
    int32_t  target_edit = 40;
    bool     target_btn  = true;

    // Last observed versions to detect external/user changes
    uint32_t last_ver_edit = 0;
    uint32_t last_ver_btn  = 0;

    // Avoid fighting the UI immediately after adoption
    uint32_t quiet_until_ms = 0;

    bool initialized = false;
};

static EditButtonDriver g_drv{};

static void tick_edit_button_driver() {
    // Track active screen so the driver follows whichever screen is visible
    if (s_mgr) {
        if (auto* scr = s_mgr->getActiveScreen()) {
            g_drv.sid = (uint16_t)scr->screenId;
        }
    }

    // Pull current store state (no clear)
    uint8_t  t_e=0, d_e=0, t_b=0, d_b=0;
    uint32_t v_e=0, ver_e=0, v_b=0, ver_b=0;

    const uint8_t CAT = (uint8_t)ValueCat::Widget;
    //vs_status_req(cat, a, b, options, &type, &val, &ver, &dirty)
    const bool ok_e = vs_status_req(CAT, g_drv.sid, g_drv.wid_edit, /*clear=*/0, t_e, v_e, ver_e, d_e);
    const bool ok_b = vs_status_req(CAT, g_drv.sid, g_drv.wid_btn,  /*clear=*/0, t_b, v_b, ver_b, d_b);
    if (!ok_e || !ok_b) return;

    // One-time initialization to requested demo values
    if (!g_drv.initialized) {
        bool need_set = false;

        if (t_e != 2 || (int32_t)v_e != 40) { g_drv.target_edit = 40; need_set = true; }
        if (t_b != 1 || (v_b != 1))         { g_drv.target_btn  = true; need_set = true; }

        if (need_set) {
            (void)vs_set_w(CAT, g_drv.sid, g_drv.wid_edit, /*WT_Int=*/2,  (uint32_t)g_drv.target_edit);
            (void)vs_set_w(CAT, g_drv.sid, g_drv.wid_btn,  /*WT_Bool=*/1, g_drv.target_btn ? 1u : 0u);
            printf("[DRV] init sid=%u edit=40 btn=1\n", (unsigned)g_drv.sid);
            // The store will bump versions; we’ll see them on the next tick

            (void)vs_set_w((uint8_t)ValueCat::Widget, /*sid*/(uint16_t)ScreenEnum::TESTSCREEN,
                        /*wid*/2, /*WT_Int=*/2, 40);
            (void)vs_set_w((uint8_t)ValueCat::Widget, /*sid*/(uint16_t)ScreenEnum::TESTSCREEN,
                        /*wid*/3, /*WT_Bool=*/1, 1);

            // tell the UI it can render real screens
            (void)vs_set_w((uint8_t)ValueCat::System, 0, 60, /*WT_Bool=*/1, 1);
            printf("Core1: K_EXT_READY set\n");
        }

        g_drv.last_ver_edit = ver_e;
        g_drv.last_ver_btn  = ver_b;
        g_drv.quiet_until_ms = now_ms();   // no pause needed beyond init
        g_drv.initialized    = true;
        return;
    }

    // Detect user/external edit change → ADOPT value & pause briefly
    if (ver_e != g_drv.last_ver_edit && (int32_t)v_e != g_drv.target_edit) {
        g_drv.target_edit   = (int32_t)v_e;
        g_drv.last_ver_edit = ver_e;
        g_drv.quiet_until_ms = now_ms() + 400;
        printf("[DRV] adopt EDIT=%ld ver=%lu\n", (long)g_drv.target_edit, (unsigned long)ver_e);
    }

    // Detect user/external button change → ADOPT state & pause briefly
    const bool store_btn = (v_b != 0);
    if (ver_b != g_drv.last_ver_btn && store_btn != g_drv.target_btn) {
        g_drv.target_btn    = store_btn;
        g_drv.last_ver_btn  = ver_b;
        g_drv.quiet_until_ms = now_ms() + 400;
        printf("[DRV] adopt BTN=%u ver=%lu\n", (unsigned)g_drv.target_btn, (unsigned long)ver_b);
    }

    // Respect the quiet window
    if (now_ms() < g_drv.quiet_until_ms) return;

    // Current store snapshot
    const int32_t cur_edit = (int32_t)v_e;
    const bool    cur_btn  = store_btn;

    // If store is aligned with our target, move the target one step in the current direction
    if (cur_edit == g_drv.target_edit) {
        g_drv.target_edit += (g_drv.target_btn ? +1 : -1);
    }

    // Bounds logic flips the button target
    if (g_drv.target_edit > 70) { g_drv.target_edit = 70; g_drv.target_btn = false; }
    if (g_drv.target_edit < 30) { g_drv.target_edit = 30; g_drv.target_btn = true;  }

    // Drive EDIT toward target
    if (cur_edit != g_drv.target_edit) {
        if (vs_set_w(CAT, g_drv.sid, g_drv.wid_edit, /*WT_Int=*/2, (uint32_t)g_drv.target_edit)) {
            printf("[DRV] set EDIT -> %ld\n", (long)g_drv.target_edit);
        }
    }

    // Drive BUTTON toward target
    if (cur_btn != g_drv.target_btn) {
        if (vs_set_w(CAT, g_drv.sid, g_drv.wid_btn, /*WT_Bool=*/1, g_drv.target_btn ? 1u : 0u)) {
            printf("[DRV] set BTN -> %u\n", (unsigned)g_drv.target_btn);
        }
    }
}

// Call this frequently so adoption feels instant
void run_master() {
    tick_edit_button_driver();
    sleep_ms(150);
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
        // Optional: quick ACK peek so logs show vsFrozen=1 at start
        uint8_t ack = 0;
        read_ack(ack);
        printf("Core1 start ACK=0x%02X anyDirty=%u\n", ack, (ack>>5)&1);
        while (true) {
            run_master();       // your dev traffic
            // you can also peek s_mgr->getActiveScreen() if you kept a pointer
            sleep_ms(1000);
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
