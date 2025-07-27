#pragma once

#include <cstdint>
#include <cstddef>
#include "hardware/uart.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "pico/stdlib.h"
#include "display.h"

// Uncomment to enable debug output
#define ENABLE_SERIAL_DEBUG

#ifdef ENABLE_SERIAL_DEBUG

    // --- BASIC DEBUG MACROS ---
    #define DEBUG_PRINT(...)     std::printf(__VA_ARGS__)
    #define DEBUG_PRINTLN(...)   do { std::printf("%s: ", __func__); std::printf(__VA_ARGS__); std::printf("\n"); } while(0)

    // --- CLASS TRACE SUPPORT ---
    #include <string_view>
    #include <type_traits>

    namespace trace_internal {
        // Extract class name from __PRETTY_FUNCTION__
        constexpr std::string_view extract_class_name(std::string_view pretty) {
            // Look for "T = " pattern in template instantiation
            auto start = pretty.find("T = ");
            if (start == std::string_view::npos) return {};
            start += 4;
            auto end = pretty.find(';', start);
            if (end == std::string_view::npos)
                end = pretty.find(']', start);
            return pretty.substr(start, end - start);
        }

        template <typename T>
        constexpr std::string_view class_name_impl() {
            return extract_class_name(__PRETTY_FUNCTION__);
        }
    }

    // Macro to extract current class name (works in class methods)
    #define CLASS_NAME() trace_internal::class_name_impl<std::remove_reference_t<decltype(*this)>>()

    // TRACE macro for class methods: [ClassName] function: message
    #define TRACE(fmt, ...) \
        std::printf("[%.*s] %s: " fmt "\n", (int)CLASS_NAME().size(), CLASS_NAME().data(), __func__, ##__VA_ARGS__)

#else
    #define DEBUG_PRINT(...)     ((void)0)
    #define DEBUG_PRINTLN(...)   ((void)0)
    #define TRACE(fmt, ...)      ((void)0)
    #define CLASS_NAME()         ("")

#endif

// -------------------------------------------------------------
// Debug class
// -------------------------------------------------------------
class Debug {
public:
    Debug();
    void poll();              // Call this frequently from your main loop
    void registerVar(const char* name, void* ptr, size_t size);
    void printHelp();
    void setDisplay(Display* disp);

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
