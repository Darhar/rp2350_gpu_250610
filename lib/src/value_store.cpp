// value_store.cpp
#include "value_store.h"

// -------------------------------
// Singleton
// -------------------------------
ValueStore& ValueStore::instance() {
    static ValueStore s;
    return s;
}

// -------------------------------
// Builder phase
// -------------------------------
void ValueStore::declare(ValueKey key, ValueType type, uint32_t defaultRaw) {
    if (frozen_) return; // ignore late declares

    auto it = pending_.find(key);
    if (it == pending_.end()) {
        pending_.emplace(key, std::make_pair(type, defaultRaw));
        return;
    }

    // Merge: keep first non-Nil type; keep first non-zero default (simple policy)
    auto& [t, def] = it->second;
    if (t == ValueType::Nil && type != ValueType::Nil) t = type;
    if (def == 0u) def = defaultRaw;
}

void ValueStore::declareBool(ValueKey key, bool defaultVal) {
    declare(key, ValueType::Bool, to_raw_bool(defaultVal));
}
void ValueStore::declareInt(ValueKey key, int defaultVal) {
    declare(key, ValueType::Int, to_raw_int(defaultVal));
}
void ValueStore::declareU32(ValueKey key, uint32_t defaultVal) {
    declare(key, ValueType::U32, to_raw_u32(defaultVal));
}

void ValueStore::freeze() {
    if (frozen_) return;

    const std::size_t count = pending_.size();

    slots_.clear();
    slots_.resize(count);      // default-construct N Slots
    index_.clear();
    index_.reserve(count);

    std::size_t idx = 0;
    for (const auto& kv : pending_) {
        const ValueKey  key   = kv.first;
        const ValueType type  = kv.second.first;
        const uint32_t  def   = kv.second.second;

        Slot& s = slots_[idx];
        s.key   = key;
        s.type  = type;
        s.raw.store(def, std::memory_order_relaxed);

        index_.emplace(key, idx);
        ++idx;
    }

    pending_.clear();

    const uint32_t slots  = static_cast<uint32_t>(slots_.size());
    const uint32_t nbanks = (slots + 31u) >> 5;

    versions_.clear();
    versions_.resize(slots_.size());
    for (auto& vc : versions_) vc.v.store(0u, std::memory_order_relaxed);

    bankDirty_.clear();
    bankDirty_.resize(nbanks);
    for (auto& b : bankDirty_) {
        b.mask.store(0u, std::memory_order_relaxed);  // <-- .mask
    }
    dirtyBanksMask_.store(0ULL, std::memory_order_relaxed);
    // A4: clear ring + seq
    ringHead_.store(0u, std::memory_order_relaxed);
    seq_.store(0u, std::memory_order_relaxed);
    for (auto& a : ring_) a.store(0u, std::memory_order_relaxed);



    frozen_ = true; // keep as plain bool
}


// -------------------------------
// Lookup
// -------------------------------
ValueStore::Slot* ValueStore::find(ValueKey key) noexcept {
    if (!frozen_) return nullptr;
    auto it = index_.find(key);
    if (it == index_.end()) return nullptr;
    return &slots_[it->second];
}

const ValueStore::Slot* ValueStore::find(ValueKey key) const noexcept {
    if (!frozen_) return nullptr;
    auto it = index_.find(key);
    if (it == index_.end()) return nullptr;
    return &slots_[it->second];
}

// -------------------------------
// Steady-state setters
// -------------------------------
bool ValueStore::setBool(ValueKey key, bool v) noexcept {
    if (!frozen_) return false;

    auto it = index_.find(key);
    if (it == index_.end()) return false;
    const uint32_t idx = static_cast<uint32_t>(it->second);
    Slot& s = slots_[idx];
    if (s.type != ValueType::Bool) return false;

    const uint32_t new_raw = to_raw_bool(v);
    const uint32_t old_raw = s.raw.load(std::memory_order_acquire);
    if (old_raw == new_raw) return true;                 // no change → no dirty

    s.raw.store(new_raw, std::memory_order_release);
    markDirty(idx);
    versions_[idx].v.fetch_add(1u, std::memory_order_acq_rel);    
    recordChange(idx);
    //seq_.fetch_add(1, std::memory_order_acq_rel);
    return true;
}

bool ValueStore::setInt(ValueKey key, int v) noexcept {
    if (!frozen_) return false;

    auto it = index_.find(key);
    if (it == index_.end()) return false;
    const uint32_t idx = static_cast<uint32_t>(it->second);
    Slot& s = slots_[idx];
    if (s.type != ValueType::Int) return false;

    const uint32_t new_raw = to_raw_int(v);
    const uint32_t old_raw = s.raw.load(std::memory_order_acquire);
    if (old_raw == new_raw) return true;

    s.raw.store(new_raw, std::memory_order_release);
    markDirty(idx);
    versions_[idx].v.fetch_add(1u, std::memory_order_acq_rel);    
    recordChange(idx);
    //seq_.fetch_add(1, std::memory_order_acq_rel);
    return true;
}

bool ValueStore::setU32(ValueKey key, uint32_t v) noexcept {
    if (!frozen_) return false;

    auto it = index_.find(key);
    if (it == index_.end()) return false;
    const uint32_t idx = static_cast<uint32_t>(it->second);
    Slot& s = slots_[idx];
    if (s.type != ValueType::U32) return false;

    const uint32_t new_raw = to_raw_u32(v);
    const uint32_t old_raw = s.raw.load(std::memory_order_acquire);
    if (old_raw == new_raw) return true;

    s.raw.store(new_raw, std::memory_order_release);
    markDirty(idx);
    versions_[idx].v.fetch_add(1u, std::memory_order_acq_rel);    
    recordChange(idx);
    //seq_.fetch_add(1, std::memory_order_acq_rel);
    return true;
}



// -------------------------------
// Steady-state getters
// -------------------------------
std::optional<bool> ValueStore::getBool(ValueKey key) const noexcept {
    const Slot* s = find(key);
    if (!s || s->type != ValueType::Bool) return std::nullopt;
    return from_raw_bool(s->raw.load(std::memory_order_acquire));
}

std::optional<int> ValueStore::getInt(ValueKey key) const noexcept {
    const Slot* s = find(key);
    if (!s || s->type != ValueType::Int) return std::nullopt;
    return from_raw_int(s->raw.load(std::memory_order_acquire));
}

std::optional<uint32_t> ValueStore::getU32(ValueKey key) const noexcept {
    const Slot* s = find(key);
    if (!s || s->type != ValueType::U32) return std::nullopt;
    return from_raw_u32(s->raw.load(std::memory_order_acquire));
}

std::optional<uint32_t> ValueStore::firstDirtyBank() const noexcept {
    const uint64_t m = dirtyBanksMask_.load(std::memory_order_acquire);
    if (!m) return std::nullopt;
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<uint32_t>(__builtin_ctzll(m));
#else
    for (uint32_t i = 0; i < 64; ++i) if (m & (1ull << i)) return i;
    return std::nullopt;
#endif
}

uint32_t ValueStore::loadBankMask(uint32_t bank) const noexcept {
    if (bank >= bankDirty_.size()) return 0u;
    return bankDirty_[bank].mask.load(std::memory_order_acquire);
}

uint32_t ValueStore::fetchAndClearBank(uint32_t bank) noexcept {
    if (bank >= bankDirty_.size()) return 0u;
    const uint32_t old = bankDirty_[bank].mask.exchange(0u, std::memory_order_acq_rel);
    if (old != 0u) {
        // Clear the bank bit in the global mask; concurrent sets will re-set it.
        const uint64_t bit = ~(1ull << bank);
        dirtyBanksMask_.fetch_and(bit, std::memory_order_acq_rel);
    }
    return old;
}

std::size_t ValueStore::copyChangesSince(
    uint32_t lastSeq,
    uint16_t* out, 
    std::size_t maxN,
    uint32_t& curSeq,
     bool& overflow) const noexcept
{
    curSeq = seq_.load(std::memory_order_acquire);
    if (curSeq == lastSeq || maxN == 0) { overflow = false; return 0; }

    const uint32_t diff = curSeq - lastSeq;          // natural modulo arithmetic on uint32_t
    overflow = (diff > RING_SIZE);

    // Choose the earliest sequence to return (drop oldest if overflow)
    const uint32_t startSeq = overflow ? (curSeq - RING_SIZE + 1u)
                                       : (lastSeq + 1u);
    const uint32_t available = curSeq - startSeq + 1u;               // <= RING_SIZE
    std::size_t n = static_cast<std::size_t>(available);
    if (n > maxN) n = maxN;

    for (std::size_t i = 0; i < n; ++i) {
        const uint32_t seq_i = startSeq + static_cast<uint32_t>(i);
        const uint32_t idx   = seq_i & RING_MASK;
        out[i] = ring_[idx].load(std::memory_order_acquire);
    }
    return n;
}

std::optional<uint32_t> ValueStore::indexOf(ValueKey key) const noexcept {
    auto it = index_.find(key);
    if (it == index_.end()) return std::nullopt;
    return static_cast<uint32_t>(it->second);
}

std::optional<ValueType> ValueStore::typeOf(ValueKey key) const noexcept {
    auto it = index_.find(key);
    if (it == index_.end()) return std::nullopt;
    return slots_[it->second].type;
}

std::optional<uint32_t> ValueStore::versionOf(ValueKey key) const noexcept {
    auto idx = indexOf(key);
    if (!idx) return std::nullopt;
    return versions_[*idx].v.load(std::memory_order_acquire);
}

bool ValueStore::slotDirty(uint32_t slotIdx) const noexcept {
    const uint32_t bank = slotIdx >> 5;
    const uint32_t bit  = slotIdx & 31;
    if (bank >= bankDirty_.size()) return false;
    const uint32_t m = bankDirty_[bank].mask.load(std::memory_order_acquire);
    return (m & (1u << bit)) != 0;
}

bool ValueStore::clearDirty(ValueKey key) noexcept {
    auto idx = indexOf(key);
    if (!idx) return false;
    return clearSlotDirty(*idx);
}

