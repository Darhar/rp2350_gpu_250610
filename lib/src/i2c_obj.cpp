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
// ChangesSince: [lastSeq(u32 LE), maxN(u8)] -> [curSeq(u32) | flags(u8) | count(u8) | count * slotIdx(u16 LE)]
ChangesSince::ChangesSince(const uint8_t* data, size_t len)
: i2cObj(data, len)
{
    uint32_t lastSeq = 0;
    uint8_t  want    = MAX_ENTRIES;

    if (data && len >= 5) {
        lastSeq = (uint32_t)data[0] | ((uint32_t)data[1] << 8)
                | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
        want    = data[4];
        if (want == 0) want = 1;
        if (want > MAX_ENTRIES) want = MAX_ENTRIES;
    }

    uint16_t tmp[MAX_ENTRIES]{};
    uint32_t curSeq = 0;
    bool overflow = false;

    auto& vs = ValueStore::instance();
    const std::size_t got = vs.copyChangesSince(lastSeq, tmp, want, curSeq, overflow);

    // Build response
    resp_[0] = (uint8_t)(curSeq >> 0);
    resp_[1] = (uint8_t)(curSeq >> 8);
    resp_[2] = (uint8_t)(curSeq >> 16);
    resp_[3] = (uint8_t)(curSeq >> 24);

    uint8_t flags = 0;
    if (overflow) flags |= 0x01; // bit0 = overflow
    resp_[4] = flags;

    resp_[5] = (uint8_t)got;     // count

    // entries
    for (std::size_t i = 0; i < got; ++i) {
        const uint16_t slot = tmp[i];
        const std::size_t off = 6 + i*2;
        resp_[off + 0] = (uint8_t)(slot & 0xFF);
        resp_[off + 1] = (uint8_t)(slot >> 8);
    }

    rlen_ = 6 + got*2;
}

// ---- STATUS: header + [addr(5) + options(1)] -> [type(1) + u32(4) + version(4) + dirty(1)] ----
VsStatus::VsStatus(const uint8_t* data, size_t len)
: i2cObj(data, len)
{
    VsAddr addr{};
    uint8_t opts = 0;
    if (!data || len < 6 || !parse_addr(data, 5, addr)) {
        // invalid → return Nil/0s
        resp_[0] = static_cast<uint8_t>(ValueType::Nil);
        /* rest already zero */
        return;
    }
    opts = data[5];

    auto& vs = ValueStore::instance();
    const ValueCat cat = static_cast<ValueCat>(addr.cat);
    const ValueKey key{cat, addr.a, addr.b};

    // Determine type and current raw value
    ValueType t = vs.typeOf(key).value_or(ValueType::Nil);
    uint32_t v  = 0;

    switch (t) {
        case ValueType::Bool: v = vs.getBool(key).value_or(false) ? 1u : 0u; break;
        case ValueType::Int:  v = static_cast<uint32_t>(vs.getInt(key).value_or(0)); break;
        case ValueType::U32:  v = vs.getU32(key).value_or(0); break;
        default: t = ValueType::Nil; v = 0; break;
    }

    // Read version + dirty
    uint32_t ver = vs.versionOf(key).value_or(0);
    bool dirty = false;
    if (auto idx = vs.indexOf(key)) dirty = vs.slotDirty(*idx);

    // Optional CLEAR (after reading)
    if ((opts & 0x01) && dirty) {
        (void)vs.clearDirty(key);
        // dirty bit might now be clear; we keep the pre-clear dirty state in the reply
    }

    // Build response
    resp_[0] = static_cast<uint8_t>(t);
    resp_[1] = (uint8_t)(v >> 0);
    resp_[2] = (uint8_t)(v >> 8);
    resp_[3] = (uint8_t)(v >> 16);
    resp_[4] = (uint8_t)(v >> 24);

    resp_[5] = (uint8_t)(ver >> 0);
    resp_[6] = (uint8_t)(ver >> 8);
    resp_[7] = (uint8_t)(ver >> 16);
    resp_[8] = (uint8_t)(ver >> 24);

    resp_[9] = dirty ? 1 : 0;
}
