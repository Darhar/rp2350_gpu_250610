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

enum i2cCmnds {
    i2c_None,
    i2c_scrCng,
    i2c_txtCng,
    i2c_getKb,
    i2c_imgLod,
    i2c_imgMov,
    i2c_butGet,
    i2c_txtGet,
    i2c_uiGet,
    i2c_uiCng,
};

class ScreenManager;
class i2cObj;

// If you publish command IDs here, keep them as enum/constexpr (OK in headers).
// enum : uint8_t { i2c_scrCng = 0x01, /* ... */ };

i2cObj* createCommandObject(
    uint8_t  cmdId,
    uint8_t  screenId,
    uint32_t paramBits,
    const uint8_t* extraData,
    std::size_t extraLen,
    ScreenManager& mgr,
    KeyBoard& keyboard   
);



