#include "command_factory.hpp"
#include "screenManager.hpp"
#include "i2c_obj.hpp"

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