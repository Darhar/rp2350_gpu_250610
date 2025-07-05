#pragma once

#include <cstdint>
#include <cstddef>
#include "hardware/uart.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "pico/stdlib.h"
#include "display.h"

class Debug {
public:
    Debug();
    void poll();              // Call this frequently from your main loop
    void registerVar(const char* name, void* ptr, size_t size);
    void printHelp();
    void setDisplay(Display* disp);
    // Optional: attach references to system components (display, screen, etc.)
    // void setDisplay(Display& disp);

private:
    struct DebugVar {
        const char* name;
        void* ptr;
        size_t size;
    };
    Display* display = nullptr;
    static constexpr size_t MAX_VARS = 32;
    DebugVar vars[MAX_VARS];
    size_t varCount = 0;
    static constexpr size_t MAX_INPUT = 64;
    char inputBuffer[MAX_INPUT];
    size_t inputIndex;

    void processCommand(const char* cmd);
    void printMemoryUsage();
    void viewMemory(uintptr_t addr, size_t len);
    void printVarByName(const char* name);
    void viewFrameBuffer1bpp(uint8_t* buf, uint8_t width, uint8_t height);

};

