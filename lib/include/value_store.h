#pragma once
// value_store.h — single-source-of-truth key/value store with freeze + atomics
// Requires: C++17

#include <cstdint>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <functional> // (usually pulled in via <unordered_map>, but explicit is fine)
#include <array>
#include "value_store.h"          // ⟵ add this so ValueKey/ValueCat/VKey are visible
#include <memory>   // for std::unique_ptr
// -------------------------------
// Addressing / typing
// -------------------------------

enum class VsOrigin : uint8_t {
    Unknown   = 0,   // default
    UI_C0     = 1,   // Core-0 (UI/slave) local writes (e.g., user commit)
    MASTER_C1 = 2,   // Core-1 (master) writes via I²C
    // Reserve room for more writers/slaves/hosts:
    EXT_BASE  = 16,  // 16..127 external writer IDs if you ever need them
};

enum class ValueCat : uint8_t {
    Widget  = 0,  // (a=screenId, b=widgetId) default auto-bind
    Keyboard= 1,
    Led     = 2,
    Audio   = 3,
    System  = 4
};

enum class ValueType : uint8_t {
    Nil  = 0,
    Bool = 1,
    Int  = 2,   // int32_t payload
    U32  = 3    // uint32_t payload
};

/*
const and noexcept appear together in a member function declaration means 
the function does not modify the object's state and 
is guaranteed not to throw exceptions
*/

struct ValueKey {
    ValueCat  cat;
    uint16_t  a;
    uint16_t  b;

    // C++17-friendly equality + hash
    bool operator==(const ValueKey& o) const noexcept {
        return cat == o.cat && a == o.a && b == o.b;
    }
};

struct ValueKeyHash {
    std::size_t operator()(const ValueKey& k) const noexcept {
        // Pack into 64 bits safely, then hash that.
        const uint64_t v =
            (uint64_t(uint8_t(k.cat)) << 32) |
            (uint64_t(k.a)            << 16) |
            (uint64_t(k.b));
        return std::hash<uint64_t>{}(v);
    }
};

inline constexpr ValueKey VKey(ValueCat c, uint16_t a, uint16_t b) noexcept {
    return ValueKey{c, a, b};
}

// -------------------------------
// ValueStore
// -------------------------------
class ValueStore {

    public:
        // Singleton (simple, static storage-duration)
        static ValueStore& instance();
        // True if any slot has been updated since last clear (Phase A1: read-only)
        bool anyDirty() const noexcept {
            return dirtyBanksMask_.load(std::memory_order_acquire) != 0ULL;
        }
        uint32_t bankWidth() const noexcept { return 32; }
        uint32_t numBanks()  const noexcept { return static_cast<uint32_t>(bankDirty_.size()); }

        // -------- Builder phase (call before freeze) --------
        // Declare a key with its type and default raw value (uint32_t bit pattern).
        // Idempotent; first declared type "wins".
        void declare(ValueKey key, ValueType type, uint32_t defaultRaw = 0);

        // Convenience type-safe declares:
        void declareBool(ValueKey key, bool defaultVal);
        void declareInt (ValueKey key, int  defaultVal);
        void declareU32 (ValueKey key, uint32_t defaultVal);

        // Finalize: materialize slots and indices; after this, set/get are lock-free.
        void freeze();

        bool frozen() const noexcept { return frozen_; }
        std::size_t size() const noexcept { return slots_.size(); }

        // -------- Steady-state API (after freeze) --------
        // Typed setters/getters by key
        bool setBool(ValueKey key, bool v) noexcept;
        bool setInt (ValueKey key, int  v) noexcept;
        bool setU32 (ValueKey key, uint32_t v) noexcept;

// new: origin-aware setters
bool setBoolFrom(ValueKey key, bool v, uint8_t origin) noexcept;
bool setIntFrom (ValueKey key, int  v, uint8_t origin) noexcept;
bool setU32From (ValueKey key, uint32_t v, uint8_t origin) noexcept;


// Convenience (cat,a,b) overloads that forward to the ValueKey versions
inline bool setBool(ValueCat c, uint16_t a, uint16_t b, bool v) noexcept {
    return setBool(VKey(c,a,b), v);
}
inline bool setInt(ValueCat c, uint16_t a, uint16_t b, int v) noexcept {
    return setInt(VKey(c,a,b), v);
}
inline bool setU32(ValueCat c, uint16_t a, uint16_t b, uint32_t v) noexcept {
    return setU32(VKey(c,a,b), v);
}
        

        std::optional<bool>     getBool(ValueKey key) const noexcept;
        std::optional<int>      getInt (ValueKey key) const noexcept;
        std::optional<uint32_t> getU32 (ValueKey key) const noexcept;

        inline std::optional<bool>     getBool(ValueCat c, uint16_t a, uint16_t b) const noexcept { return getBool(VKey(c,a,b)); }
        inline std::optional<int>      getInt (ValueCat c, uint16_t a, uint16_t b) const noexcept { return getInt (VKey(c,a,b)); }
        inline std::optional<uint32_t> getU32 (ValueCat c, uint16_t a, uint16_t b) const noexcept { return getU32(VKey(c,a,b)); }




        std::optional<uint32_t> firstDirtyBank() const noexcept;
        uint32_t seq() const noexcept { return seq_.load(std::memory_order_acquire); }      
        // Return current 32-bit dirty mask for a bank (0 if out of range)
        uint32_t loadBankMask(uint32_t bank) const noexcept;
        // Atomically fetch and clear a bank; also clears global bit if mask becomes 0
        uint32_t fetchAndClearBank(uint32_t bank) noexcept;  
        // Copy up to maxN changed slot indices since lastSeq into out[].
        // Returns the number copied, sets curSeq to the latest sequence, and overflow=true
        // if there were more than the ring can hold since lastSeq.
        std::size_t copyChangesSince(uint32_t lastSeq,
                                    uint16_t* out, std::size_t maxN,
                                    uint32_t& curSeq, bool& overflow) const noexcept;

        // A5: per-slot status helpers
        std::optional<uint32_t> indexOf(ValueKey key) const noexcept;
        std::optional<ValueType> typeOf(ValueKey key) const noexcept;  // if you don’t already have it
        std::optional<uint32_t> versionOf(ValueKey key) const noexcept;

        // Returns true if the bit is set for this slot
        bool slotDirty(uint32_t slotIdx) const noexcept;

        // Clear just this slot’s dirty bit (and global bank bit if that bank becomes empty)
        bool clearDirty(ValueKey key) noexcept;
        void clearAllDirty() noexcept;




    // New: query the last writer id for a key (0 if unknown / not found)
    std::optional<uint8_t> lastWriterOf(ValueKey key) const noexcept;
    
    
    private:
        // Storage slot — 32-bit atomic raw payload for lock-free access
        struct Slot {
            ValueKey              key{};
            ValueType             type{ValueType::Nil};
            std::atomic<uint32_t> raw{0};

            Slot() = default;
            Slot(const Slot&) = delete;
            Slot& operator=(const Slot&) = delete;

            // Move by value-copying the atomic contents
            Slot(Slot&& other) noexcept
                : key(other.key), type(other.type) {
                raw.store(other.raw.load(std::memory_order_relaxed),
                        std::memory_order_relaxed);
            }
            Slot& operator=(Slot&&) = delete;
        };
        std::atomic<uint32_t> seq_{0}; // increments on each successful value change
        // Bank mask wrapper so vector can move/resize safely
        struct BankMask {
            std::atomic<uint32_t> mask{0};
            BankMask() = default;
            BankMask(const BankMask&) = delete;
            BankMask& operator=(const BankMask&) = delete;
            BankMask(BankMask&& other) noexcept {
                mask.store(other.mask.load(std::memory_order_relaxed),
                        std::memory_order_relaxed);
            }
            BankMask& operator=(BankMask&&) = delete;
        };

        // Builder storage: key -> (type, defaultRaw)
        std::unordered_map<ValueKey, std::pair<ValueType,uint32_t>, ValueKeyHash> pending_;

        // Frozen storage
        std::vector<Slot>                                       slots_;
        std::unordered_map<ValueKey, std::size_t, ValueKeyHash> index_;
        bool                                                    frozen_ = false;

        // Phase A1 state
        std::vector<BankMask>           bankDirty_;            // one u32 mask per 32 slots
        std::atomic<uint64_t>           dirtyBanksMask_{0};

        // Lookup (nullptr if not found or not frozen)
        Slot*       find(ValueKey key) noexcept;
        const Slot* find(ValueKey key) const noexcept;

        // Mark a slot as dirty (called only when value actually changes)
        inline void markDirty(uint32_t slotIdx) noexcept {
            const uint32_t bank = slotIdx >> 5;   // /32
            const uint32_t bit  = slotIdx & 31;   // %32
            if (bank < bankDirty_.size()) {
                bankDirty_[bank].mask.fetch_or(1u << bit, std::memory_order_release);
                dirtyBanksMask_.fetch_or(1ULL << bank, std::memory_order_release);
            }
        }

        // Bitcasts
        static inline uint32_t to_raw_bool(bool v)      noexcept { return v ? 1u : 0u; }
        static inline uint32_t to_raw_int (int v)       noexcept { return static_cast<uint32_t>(static_cast<int32_t>(v)); }
        static inline uint32_t to_raw_u32 (uint32_t v)  noexcept { return v; }

        static inline bool     from_raw_bool(uint32_t r) noexcept { return (r & 1u) != 0; }
        static inline int      from_raw_int (uint32_t r) noexcept { return static_cast<int32_t>(r); }
        static inline uint32_t from_raw_u32 (uint32_t r) noexcept { return r; }
        // was: std::vector<std::atomic<uint8_t>> lastWriter_;
        std::unique_ptr<std::atomic<uint8_t>[]> lastWriter_;
        std::size_t lastWriterCount_ = 0;

        // ---- A4 ring buffer (power-of-2) ----
        //static constexpr uint32_t RING_LG2  = 7;                 // 128 entries
        //static constexpr uint32_t RING_SIZE = 1u << RING_LG2;
        //static constexpr uint32_t RING_MASK = RING_SIZE - 1;

        // Internal head used by writers to reserve the next slot before publishing
        //std::atomic<uint32_t>   ringHead_{0};
        //std::array<std::atomic<uint16_t>, RING_SIZE> ring_{};

        // Record a change: write ring slot, then publish new seq
        /*
        inline void recordChange(uint32_t slotIdx) noexcept {
            const uint32_t s = ringHead_.fetch_add(1, std::memory_order_acq_rel) + 1u;
            ring_[s & RING_MASK].store(static_cast<uint16_t>(slotIdx), std::memory_order_release);
            seq_.store(s, std::memory_order_release); // publish after entry is written
        }
        


        struct Version {
            std::atomic<uint32_t> v{0};
            Version() = default;
            Version(const Version&) = delete;
            Version& operator=(const Version&) = delete;
            Version(Version&& other) noexcept {
                v.store(other.v.load(std::memory_order_relaxed), std::memory_order_relaxed);
            }
            Version& operator=(Version&&) = delete;
        };

        std::vector<Version> versions_;  // one counter per slot
        */
        inline bool clearSlotDirty(uint32_t slotIdx) noexcept {
            const uint32_t bank = slotIdx >> 5;
            const uint32_t bit  = slotIdx & 31;
            if (bank >= bankDirty_.size()) return false;
            const uint32_t mask = (1u << bit);
            const uint32_t old  = bankDirty_[bank].mask.fetch_and(~mask, std::memory_order_acq_rel);
            if ((old & mask) == 0) return false;  // wasn’t set
            if ((old & ~mask) == 0) {
                // this bank became empty → clear its bit in the global mask
                dirtyBanksMask_.fetch_and(~(1ull << bank), std::memory_order_acq_rel);
            }
            return true;
        }


};

