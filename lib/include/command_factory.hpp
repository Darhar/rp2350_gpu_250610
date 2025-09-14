#pragma once
#include "i2c_obj.hpp"
#include <cstddef>
#include <cstdint>
#include "keyboard.h"

/*
namespace i2cCmnds {
    enum : uint8_t {
        i2c_scrCng   = 1,
        i2c_txtCng   = 2,
        i2c_getKb    = 3,   // <— NEW: get keyboard status snapshot
    };
}

*/

enum class i2cCmnds : uint8_t {
    i2c_None   = 0x00,
    i2c_scrCng = 0x01,
    i2c_txtCng = 0x02,
    i2c_getKb  = 0x03,
    i2c_vs_set = 0x04,  // value store
    i2c_vs_get = 0x05,  
    i2c_ack    = 0x06,
    i2c_dirty_summary = 0x07,
    i2c_dirty_bank  = 0x08, 
    i2c_changes_since = 0x09,
    i2c_vs_status = 0x0A,
    i2c_dirty_clear_all = 0x0B,
          
/*
    i2c_imgLod,
    i2c_imgMov,
    i2c_butGet,
    i2c_txtGet,
    i2c_uiGet,
    i2c_uiCng,
*/

};
static_assert(static_cast<uint8_t>(i2cCmnds::i2c_vs_get) < 32, "cmdId must fit 5 bits");
static_assert(static_cast<uint8_t>(i2cCmnds::i2c_dirty_summary) < 32, "cmdId fits 5 bits");
static_assert(static_cast<uint8_t>(i2cCmnds::i2c_dirty_clear_all) < 32, "fits in 5 bits");

class ScreenManager;
class i2cObj;

// If you publish command IDs here, keep them as enum/constexpr (OK in headers).
// enum : uint8_t { i2c_scrCng = 0x01, /* ... */ };

i2cObj* createCommandObject(
    i2cCmnds cmdId, 
    uint8_t screenId, 
    uint32_t paramBits,
    const uint8_t* data, 
    size_t len,
    ScreenManager& screenMgr, 
    KeyBoard* keyboard
);



