// sys_status.hpp  (shared between cores)
#pragma once
#include "value_store.h"

namespace VSIDs {
  inline constexpr ValueKey K_SYS_STATUS = VKey(ValueCat::System, 0, 0x30); // U32
  inline constexpr ValueKey K_SYS_EPOCH  = VKey(ValueCat::System, 0, 0x31); // U32 (optional)
  inline constexpr ValueKey K_ACTIVE_SCREEN = VKey(ValueCat::System, 0, 12);
  inline constexpr ValueKey K_UI_COMMIT = VKey(ValueCat::System, 0, 50);
  inline constexpr ValueKey K_EXT_READY = VKey(ValueCat::System, 0, 60); // bool 
  // (keep K_C0_HB / K_C1_HB as separate U32 if you want heartbeats later)
}

// Bit layout for K_SYS_STATUS (U32)
namespace SysStat {
  // Low byte (also the quick ACK byte)
  inline constexpr uint32_t C0_READY     = 1u << 0; // owned by Core0
  inline constexpr uint32_t C1_READY     = 1u << 1; // owned by Core1
  inline constexpr uint32_t C0_BUSY      = 1u << 2; // owned by Core0
  inline constexpr uint32_t C1_BUSY      = 1u << 3; // owned by Core1
  inline constexpr uint32_t UI_ACTIVE    = 1u << 4; // owned by Core0 (UI loop running)
  inline constexpr uint32_t VS_FROZEN    = 1u << 5; // owned by Core0 (set after freeze)
  inline constexpr uint32_t ANY_DIRTY    = 1u << 6; // owned by Core0 (mirror of anyDirty)
  inline constexpr uint32_t I2C_OVF      = 1u << 7; // owned by Core0 (slave overflow/error)

  // Next nibble: first dirty bank index (0..15; 0xF means none)
  inline constexpr uint32_t FDB_SHIFT    = 8;
  inline constexpr uint32_t FDB_MASK     = 0xFu << FDB_SHIFT; // owned by Core0

  // Misc high bits
  inline constexpr uint32_t LINK_OK      = 1u << 12; // owned by Core1 (or Core0) if recent comms ok
  inline constexpr uint32_t CMD_INFLIGHT = 1u << 13; // owned by Core1 (multi-part tx in progress)
  inline constexpr uint32_t ERROR_ANY    = 1u << 14; // owned by Core0
  inline constexpr uint32_t RESERVED15   = 1u << 15;

  // Protocol version in bits 16..23
  inline constexpr uint32_t PROTO_SHIFT  = 16;
  inline constexpr uint32_t PROTO_MASK   = 0xFFu << PROTO_SHIFT;
  inline constexpr uint32_t PROTO_VER    = (1u << PROTO_SHIFT); // v1

  // Epoch LSB (optional) in 24..31 (owned by Core0)
  inline constexpr uint32_t EPOCH_SHIFT  = 24;
  inline constexpr uint32_t EPOCH_MASK   = 0xFFu << EPOCH_SHIFT;

  // Ownership masks to prevent RMW clashes
  inline constexpr uint32_t OWNED_BY_C0 =
      C0_READY | C0_BUSY | UI_ACTIVE | VS_FROZEN | ANY_DIRTY | I2C_OVF |
      FDB_MASK | ERROR_ANY | PROTO_MASK | EPOCH_MASK;

  inline constexpr uint32_t OWNED_BY_C1 =
      C1_READY | C1_BUSY | LINK_OK | CMD_INFLIGHT;
}
