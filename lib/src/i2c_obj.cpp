#include "i2c_obj.hpp"
#include "keyboard.h"
#include "screenManager.hpp"
#include <cstring>
#include "value_store.h"
#include "sys_status.hpp"

static inline bool parse_addr(const uint8_t* d, size_t n, VsAddr& o) {
    if (n < 5) return false;
    o.cat = d[0];
    o.a   = (uint16_t)(d[1] | (uint16_t(d[2]) << 8));
    o.b   = (uint16_t)(d[3] | (uint16_t(d[4]) << 8));
    return true;
}
static inline bool parse_val(const uint8_t* d, size_t n, VsValue& o) {
    if (n < 5) return false;
    o.type = d[0];
    o.v    = (uint32_t)d[1] | ((uint32_t)d[2] << 8)
           | ((uint32_t)d[3] << 16) | ((uint32_t)d[4] << 24);
    return true;
}

// ---- SET ----
VsSetValue::VsSetValue(const uint8_t* data, size_t len)
: i2cObj(data, len)  // <-- required if i2cObj has no default ctor
{
    if (!data || len < 10) return;
    VsAddr addr{}; VsValue val{};
    if (!parse_addr(data, 5, addr)) return;
    if (!parse_val (data + 5, len - 5, val)) return;

    auto& vs  = ValueStore::instance();
    auto  cat = static_cast<ValueCat>(addr.cat);
    auto  typ = static_cast<ValueType>(val.type);

    switch (typ) {
        case ValueType::Bool: (void)vs.setBool(cat, addr.a, addr.b, val.v != 0); break;
        case ValueType::Int:  (void)vs.setInt (cat, addr.a, addr.b, (int32_t)val.v); break;
        case ValueType::U32:  (void)vs.setU32 (cat, addr.a, addr.b, val.v); break;
        default: break;
    }
}

// ---- GET: header + [addr(5)] -> reply [type(1)+u32(4)] ----
VsGetValue::VsGetValue(const uint8_t* data, size_t len)
: i2cObj(data, len)
{
    if (!data || len < 5) return;

    VsAddr addr{};
    if (!parse_addr(data, len, addr)) return;

    auto& vs  = ValueStore::instance();
    auto  cat = static_cast<ValueCat>(addr.cat);

    // Infer type by probing getters
    ValueType t = ValueType::Nil;
    uint32_t  v = 0;

    if (auto b = vs.getBool(cat, addr.a, addr.b); b.has_value()) {
        t = ValueType::Bool;
        v = *b ? 1u : 0u;
    } else if (auto i = vs.getInt(cat, addr.a, addr.b); i.has_value()) {
        t = ValueType::Int;
        v = static_cast<uint32_t>(*i);  // two's complement preserved
    } else if (auto u = vs.getU32(cat, addr.a, addr.b); u.has_value()) {
        t = ValueType::U32;
        v = *u;
    } else {
        t = ValueType::Nil;
        v = 0;
    }

    resp_[0] = static_cast<uint8_t>(t);
    resp_[1] = static_cast<uint8_t>(v >> 0);
    resp_[2] = static_cast<uint8_t>(v >> 8);
    resp_[3] = static_cast<uint8_t>(v >> 16);
    resp_[4] = static_cast<uint8_t>(v >> 24);
    rlen_ = 5;
}
// --- Dirty Summary: header-only request -> 8-byte reply

DirtySummary::DirtySummary(ScreenManager* mgr, KeyBoard* kbd, const uint8_t* data, size_t len)
: i2cObj(data, len) {
    auto& vs = ValueStore::instance();
    // flags (match ACK + anyDirty)
    uint8_t flg = 0;
    flg |= 1u << 0;                         // alive
    if (vs.frozen()) flg |= 1u << 1;//frozen
    if (mgr)         flg |= 1u << 2;//manager running
    if (kbd)         flg |= 1u << 3;//keyboard running
    // if you track RX overflow, keep it as bit4
    if (vs.anyDirty()) flg |= 1u << 5;// change of any slot

    resp_[0] = flg;
    resp_[1] = static_cast<uint8_t>(vs.bankWidth());
    resp_[2] = static_cast<uint8_t>(vs.numBanks());
    resp_[3] =  vs.firstDirtyBank().value_or(0xFF);
    resp_[4] = 0;
    resp_[5] = 0;
    resp_[6] = 0;
    resp_[7] = 0;
}

// Dirty bank: [bank(u8), options(u8)] -> mask(u32 LE)
DirtyBank::DirtyBank(const uint8_t* data, size_t len)
: i2cObj(data, len)
{
    uint8_t bank = 0, opts = 0;
    if (data && len >= 2) { bank = data[0]; opts = data[1]; }

    auto& vs = ValueStore::instance();
    const bool clear = (opts & 0x01) != 0;

    const uint32_t mask = clear ? vs.fetchAndClearBank(bank)
                                : vs.loadBankMask(bank);

    resp_[0] = (uint8_t)(mask >> 0);
    resp_[1] = (uint8_t)(mask >> 8);
    resp_[2] = (uint8_t)(mask >> 16);
    resp_[3] = (uint8_t)(mask >> 24);
}

// i2c_obj.cpp
VsStatus::VsStatus(const uint8_t* data, size_t len)
: i2cObj(data, len)
{
    auto& vs = ValueStore::instance();

    // Default to SYS_STATUS if no addr provided
    ValueKey key = VSIDs::K_SYS_STATUS;

    // Optional "clear dirty" flag lives after the 5-byte addr
    bool clearDirty = false;
    if (data && len >= 5) {
        VsAddr addr{};
        if (parse_addr(data, len, addr)) {
            key = VKey(static_cast<ValueCat>(addr.cat), addr.a, addr.b);
            if (len >= 6) {
                clearDirty = (data[5] & 0x01) != 0;  // matches your Core-1 tx[5] options byte
            }
        }
    }

    // --- Type + value ---
    ValueType t = vs.typeOf(key).value_or(ValueType::Nil);
    uint32_t  v = 0;
    switch (t) {
        case ValueType::U32: if (auto u = vs.getU32(key)) v = *u;     else t = ValueType::Nil; break;
        case ValueType::Int: if (auto i = vs.getInt (key)) v = (uint32_t)*i; else t = ValueType::Nil; break;
        case ValueType::Bool:if (auto b = vs.getBool(key)) v = *b ? 1u : 0u; else t = ValueType::Nil; break;
        default: t = ValueType::Nil; v = 0; break;
    }

    uint8_t  dirty = 0;
    uint8_t  origin = 0;

    // dirty
    if (auto idx = vs.indexOf(key)) {
        dirty = vs.slotDirty(*idx) ? 1u : 0u;
        if (clearDirty && dirty) {
            vs.clearDirty(key);
            dirty = 0u;
        }
        // origin
        origin = vs.lastWriterOf(key).value_or((uint8_t)VsOrigin::Unknown);
    }

    // pack
    resp_[0] = static_cast<uint8_t>(t);
    resp_[1] = (uint8_t)(v >> 0);
    resp_[2] = (uint8_t)(v >> 8);
    resp_[3] = (uint8_t)(v >> 16);
    resp_[4] = (uint8_t)(v >> 24);
    resp_[5] = 0; 
    resp_[6] = 0; 
    resp_[7] = 0; 
    resp_[8] = 0; // ver now unused
    resp_[9] = (uint8_t)((origin << 1) | (dirty & 0x01));
    rlen_ = 10;

}

// Clear-all: header-only -> 1B status (like ACK), AFTER clearing
DirtyClearAll::DirtyClearAll(ScreenManager* mgr, KeyBoard* kbd, const uint8_t* /*data*/, size_t /*len*/)
: i2cObj(nullptr, 0)
{
    auto& vs = ValueStore::instance();
    vs.clearAllDirty();  // A6 action

    uint8_t s = 0;
    s |= 1u << 0;                       // alive
    if (vs.frozen()) s |= 1u << 1;
    if (mgr)         s |= 1u << 2;
    if (kbd)         s |= 1u << 3;
    // keep your RX overflow on bit4 if you have it
    if (vs.anyDirty()) s |= 1u << 5;    // should be 0 unless races/new writes
    status_ = s;
}
