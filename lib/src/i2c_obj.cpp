#include "i2c_obj.hpp"
#include "keyboard.h"
#include "screenManager.hpp"
#include <cstring>

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
    uint8_t f = 0;
    f |= 1u << 0;                         // alive
    if (vs.frozen()) f |= 1u << 1;
    if (mgr)         f |= 1u << 2;
    if (kbd)         f |= 1u << 3;
    // if you track RX overflow, keep it as bit4
    if (vs.anyDirty()) f |= 1u << 5;

    const uint8_t  bw   = static_cast<uint8_t>(vs.bankWidth());    // 32
    const uint8_t  nb   = static_cast<uint8_t>(vs.numBanks());     // fits 0..255 here
    const uint8_t  fdb  = vs.firstDirtyBank().value_or(0xFF);
    const uint32_t seq  = vs.seq();

    resp_[0] = f;
    resp_[1] = bw;
    resp_[2] = nb;
    resp_[3] = fdb;
    resp_[4] = static_cast<uint8_t>(seq >> 0);
    resp_[5] = static_cast<uint8_t>(seq >> 8);
    resp_[6] = static_cast<uint8_t>(seq >> 16);
    resp_[7] = static_cast<uint8_t>(seq >> 24);
}

