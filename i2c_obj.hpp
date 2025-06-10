#include <cstdint>
#include <cstring>
#include "screenManager.hpp"

#ifndef I2COBJ
    #define I2COBJ

class i2cObj {
public:
    static constexpr size_t MAX_DATA = 32;

    uint8_t data[MAX_DATA];
    size_t size;

    i2cObj(const uint8_t* srcData, size_t len) : size(len) {
        memcpy(data, srcData, len);
    }

    virtual ~i2cObj() = default;

    // Override in derived class if it needs to respond with data
    virtual const uint8_t* getResponse(size_t& responseSize) {
        responseSize = 0;
        return nullptr;
    }
};

class ScreenChange : public i2cObj {
    uint8_t screenId;
    uint32_t paramBits;

public:
    ScreenChange(uint8_t scr, uint32_t params, const uint8_t* srcData, size_t len, ScreenManager& screenMgr)
        : i2cObj(srcData, len), screenId(scr), paramBits(params) {
        
        printf("ScreenChange Created: screen=%d, paramBits=0x%X\n", scr, params);
        screenMgr.setActiveScreen((ScreenEnum)scr);
    }

    const uint8_t* getResponse(size_t& responseSize) override {
        responseSize = size;
        return data;
    }
};


class TextChange : public i2cObj {
    uint8_t screenId;
    uint32_t paramBits;

public:
    TextChange(uint8_t scr, uint32_t params, const uint8_t* srcData, size_t len, ScreenManager& screenMgr)
        : i2cObj(srcData, len), screenId(scr), paramBits(params) {
        printf("Bar Created: screen=%d, paramBits=0x%X\n", scr, params);
        //check flags for active screen
        //if not active screen use screen id supplied 
    }

    const uint8_t* getResponse(size_t& responseSize) override {
        responseSize = size;
        return data;
    }
};


#endif
