#include "command_factory.hpp"
#include "screenManager.hpp"
#include "i2c_obj.hpp"

/*
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
*/


#include "keyboard.h"

i2cObj* createCommandObject(uint8_t cmdId, uint8_t screenId, uint32_t paramBits,
                            const uint8_t* data, size_t len,
                            ScreenManager& screenMgr,
                            KeyBoard& keyboard)   // <— added
{
    switch (cmdId) {
        case i2cCmnds::i2c_scrCng:
            return new ScreenChange(screenId, paramBits, data, len, screenMgr);

        case i2cCmnds::i2c_txtCng:
            return new TextChange(screenId, paramBits, data, len, screenMgr);

        case i2cCmnds::i2c_getKb: {            // <— NEW
            return new GetKeyboardStatus(keyboard, data, len);
        }

        default:
            return nullptr;
    }
}