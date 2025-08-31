#include <cstdint>
#include <cstring>
#include <cstddef>
#include "screenManager.hpp"
#include "keyboard.h"
#include "value_store.h"

#pragma once

// 5-byte wire shapes (only declare once project-wide)
struct __attribute__((packed)) VsAddr  { uint8_t cat; uint16_t a; uint16_t b; };
struct __attribute__((packed)) VsValue { uint8_t type; uint32_t v; };

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

class GetKeyboardStatus : public i2cObj {
    public:
        GetKeyboardStatus(KeyBoard& kb, const uint8_t* src, size_t len) : i2cObj(src, len), kbd(kb) {
            KeyBoard::KeyReport rep{};
            kbd.fillReport(rep);
            static_assert(sizeof(rep) == 8, "KeyReport size changed; update resp buffer");
            std::memcpy(resp.data(), &rep, sizeof(rep));
        }

        const uint8_t* getResponse(size_t& responseSize) override {
            responseSize = resp.size();
            return resp.data();
        }

    private:
        KeyBoard& kbd;
        std::array<uint8_t, sizeof(KeyBoard::KeyReport)> resp{};
};

class GetAckStatus : public i2cObj {
    public:
        GetAckStatus(ScreenManager* mgr, KeyBoard* kbd, const uint8_t* data, size_t len)
        : i2cObj(data, len)   // <-- call the required base ctor
        {
            uint8_t s = 0;
            s |= 1u << 0; // alive
            if (ValueStore::instance().frozen()) s |= 1u << 1;
            if (mgr) s |= 1u << 2;
            if (kbd) s |= 1u << 3;
            if (ValueStore::instance().anyDirty()) s |= 1u << 5;  // NEW: any-dirty            
            status_ = s;
        }

        const uint8_t* getResponse(size_t& outLen) override {
            outLen = 1;
            return &status_;
        }
    private:
        uint8_t status_{0};
};

class VsSetValue : public i2cObj {
    public:
        VsSetValue(const uint8_t* data, size_t len);   // <-- EXACT signature
        const uint8_t* getResponse(size_t& outLen) override { outLen = 0; return nullptr; }
};

class VsGetValue : public i2cObj {
    public:
        VsGetValue(const uint8_t* data, size_t len);   // <-- EXACT signature
        const uint8_t* getResponse(size_t& outLen) override {
            outLen = rlen_; return rlen_ ? resp_ : nullptr;
        }
    private:
        uint8_t resp_[5]{};
        size_t  rlen_ = 0;
};

class DirtySummary : public i2cObj {
public:
    DirtySummary(ScreenManager* mgr, KeyBoard* kbd, const uint8_t* data, size_t len);
    const uint8_t* getResponse(size_t& outLen) override { outLen = 8; return resp_; }
private:
    uint8_t resp_[8]{};
};