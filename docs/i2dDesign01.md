# I²C Protocol Design Summary (RP2040 Dual-Core Development)

## Overview

This document outlines the design and implementation of a custom I²C command protocol for an RP2040-based system. The system uses both cores of the RP2350 (Raspberry Pi Pico 2), with one core acting as a virtual master and the other as a slave. The slave handles display updates, widget interaction, and keypad input, while the master sends control and query commands over I²C.

---

## Addressable Targets

- **64 Total Screens** (IDs 0–63)
  - **Last screen ID (63)** reserved for system-level commands.
  - **7 additional screen IDs** reserved for future development.

---

## Keypad Design

- **Key Inputs (1 at a time)**:
  - `KEY_UP` (1)
  - `KEY_DOWN` (2)
  - `KEY_LEFT` (3)
  - `KEY_OK` (4)
  - `KEY_BACK` (5)

- **Polling**:
  - Master must poll the slave to retrieve key states.
  - Slave cannot initiate communication.

---

## Message Format (Standard Command)

- **Total**: 32 bits per standard command message
- **Structure**:


- **Command + Flags**: 8 bits
- **Screen ID**: 6 bits
- **Remaining 18 bits**: Optional parameters (1 or 2 values)

- **Multi-byte extension**: Some commands may include variable-length payloads. These are processed beyond the initial 32-bit header.

---

## Command Types

### Write (Master → Slave)
#### Single-Byte Commands
- `scrOn`: Turn screen on
- `scrOf`: Turn screen off

#### Multi-Byte Commands
- `scrCng`: Change screen → `screen ID`
- `txtCng`: Change label text → `screen ID, widget index, text size, text`
- `imgLod`: Load image → `screen ID, widget index, data size, image data`
- `imgMov`: Move image → `screen ID, x, y`

---

### Read (Master ← Slave)
#### Command Structure
- Sent as I²C reads (no flag needed to indicate response)

#### Commands
- `butGet`: Get button state → `widget index` → returns `state`
- `txtGet`: Get text from widget → `screen ID, widget ID` → returns `text size, text`
- `uiGet`: UI state changed? → returns `bool`
- `uiCng`: Get UI change events → returns `size, change IDs`

---

## Flag Bit Strategy

- **Flags are stored in bits 6–7 of the first byte.**
- Currently used:
- `Bit 6`: Could signal “use active screen”
- `Bit 7`: Reserved for future use

---

## Notes

- Commands targeting the active screen may set a flag instead of providing a screen ID.
- The protocol allows future expansion by reserving bits for new commands and flag definitions.
- Every command has a fixed format, but multi-byte payloads are permitted for commands like `txtCng` or `imgLod`.

---

## Example Slave Handler (Simplified)

```cpp
void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
  switch (event) {
      case I2C_SLAVE_RECEIVE:
          recv_buffer[recv_index++] = i2c_read_byte_raw(i2c);
          break;
      case I2C_SLAVE_REQUEST:
          i2c_write_byte_raw(i2c, getResponseByte());
          break;
      case I2C_SLAVE_FINISH:
          uint8_t cmd = recv_buffer[0];
          activeCommand = createCommandObject(cmd, &recv_buffer[1], recv_index - 1);
          recv_index = 0;
          break;
  }
}


//----------------
case I2C_SLAVE_FINISH:
    if (recv_index >= 4) {
        uint32_t cmdWord = (recv_buffer[0]) |
                           (recv_buffer[1] << 8) |
                           (recv_buffer[2] << 16) |
                           (recv_buffer[3] << 24);

        uint8_t cmdId      =  cmdWord        & 0x1F;
        uint8_t flags      = (cmdWord >> 5)  & 0x07;
        uint8_t screenId   = (cmdWord >> 8)  & 0x3F;
        uint32_t paramBits = (cmdWord >> 14) & 0x3FFFF;

        bool hasParams = flags & 0b001;
        bool useActive = flags & 0b010;

        if (useActive) {
            screenId = screenMgr.getActiveScreenId();
        }

        printf("cmdId=%d screenId=%d param=0x%05X hasParams=%d\n", cmdId, screenId, paramBits, hasParams);

        if (activeCommand) {
            delete activeCommand;
        }

        const uint8_t* extraData = (recv_index > 4) ? &recv_buffer[4] : nullptr;
        size_t extraLen = (recv_index > 4) ? recv_index - 4 : 0;

        // Factory can now take cmdId + screenId + paramBits + extraData
        activeCommand = createCommandObject(cmdId, screenId, paramBits, extraData, extraLen);
    }

    recv_index = 0;
    break;

//-------------------------
i2cObj* createCommandObject(uint8_t cmdId, uint8_t screenId, uint32_t paramBits,
                            const uint8_t* data, size_t len) {
    switch (cmdId) {
        case i2cCmnds::i2c_scrCng:
            return new CommandFoo(screenId, paramBits, data, len);
        case i2cCmnds::i2c_txtCng:
            return new CommandBar(screenId, paramBits, data, len);
        default:
            return nullptr;
    }
}

//------------------------
class CommandBar : public i2cObj {
    uint8_t screenId;
    uint32_t paramBits;

public:
    CommandBar(uint8_t scr, uint32_t params, const uint8_t* srcData, size_t len)
        : i2cObj(srcData, len), screenId(scr), paramBits(params) {
        printf("Bar Created: screen=%d, paramBits=0x%X\n", scr, params);
    }

    const uint8_t* getResponse(size_t& responseSize) override {
        responseSize = size;
        return data;
    }
};
