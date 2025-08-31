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
    frozen_ = true;
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
    Slot* s = find(key);
    if (!s || s->type != ValueType::Bool) return false;
    s->raw.store(to_raw_bool(v), std::memory_order_release);
    return true;
}

bool ValueStore::setInt(ValueKey key, int v) noexcept {
    Slot* s = find(key);
    if (!s || s->type != ValueType::Int) return false;
    s->raw.store(to_raw_int(v), std::memory_order_release);
    return true;
}

bool ValueStore::setU32(ValueKey key, uint32_t v) noexcept {
    Slot* s = find(key);
    if (!s || s->type != ValueType::U32) return false;
    s->raw.store(to_raw_u32(v), std::memory_order_release);
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

