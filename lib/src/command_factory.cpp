#include "command_factory.hpp"
#include "screenManager.hpp"
#include "i2c_obj.hpp"
#include "keyboard.h"
#include "value_store.h"

i2cObj* createCommandObject(
    i2cCmnds cmdId, 
    uint8_t screenId, 
    uint32_t paramBits,
    const uint8_t* data, 
    size_t len,
    ScreenManager& screenMgr, 
    KeyBoard* keyboard
)
{
    switch (cmdId) {
        case i2cCmnds::i2c_scrCng: return new ScreenChange(screenId, paramBits, data, len, screenMgr);
        case i2cCmnds::i2c_txtCng: return new TextChange  (screenId, paramBits, data, len, screenMgr);
        case i2cCmnds::i2c_getKb:  return keyboard ? new GetKeyboardStatus(*keyboard, data, len) : nullptr;
        case i2cCmnds::i2c_ack:    return new GetAckStatus(&screenMgr, keyboard, data, len); 
        case i2cCmnds::i2c_vs_set: return new VsSetValue(data, len);
        case i2cCmnds::i2c_vs_get: return new VsGetValue(data, len);
        case i2cCmnds::i2c_dirty_summary: return new DirtySummary(&screenMgr, keyboard, data, len);

        default: return nullptr;
    }
}
