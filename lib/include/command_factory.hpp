#pragma once
#include "i2c_obj.hpp"

enum i2cCmnds {
    i2c_None,
    i2c_scrCng,
    i2c_txtCng,
    i2c_imgLod,
    i2c_imgMov,
    i2c_butGet,
    i2c_txtGet,
    i2c_uiGet,
    i2c_uiCng,
};

i2cObj* createCommandObject(uint8_t cmdId, uint8_t screenId, uint32_t paramBits,
                            const uint8_t* data, size_t len, ScreenManager& screenMgr) {                               
    switch (cmdId) {
        case i2cCmnds::i2c_scrCng:
            return new ScreenChange(screenId, paramBits, data, len,screenMgr);
        case i2cCmnds::i2c_txtCng:
            return new TextChange(screenId, paramBits, data, len,screenMgr);
        default:
            return nullptr;
    }
}

