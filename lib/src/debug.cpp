#include "debug.h"


Debug::Debug() : inputIndex(0) {}

void Debug::poll() {

    int c;

    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if (c == '\r' || c == '\n') {
            if (inputIndex > 0) {
                inputBuffer[inputIndex] = '\0';

                // Sanitize: strip trailing CR/LF (safety against double entry)
                size_t len = strlen(inputBuffer);
                while (len > 0 && (inputBuffer[len - 1] == '\r' || inputBuffer[len - 1] == '\n')) {
                    inputBuffer[--len] = '\0';
                }

                processCommand(inputBuffer);
                inputIndex = 0;
            }
        } else if (inputIndex < MAX_INPUT - 1) {
            inputBuffer[inputIndex++] = (char)c;
        } else {
            // Optional: buffer overflow protection
            inputIndex = 0;
            printf("Input too long, clearing buffer.\n");
        }
    }
/*
    int c;
    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if (c == '\r' || c == '\n') {
            inputBuffer[inputIndex] = '\0';
            if (inputIndex > 0) {
                processCommand(inputBuffer);
                inputIndex = 0;
            }
        } else if (inputIndex < MAX_INPUT - 1) {
            inputBuffer[inputIndex++] = (char)c;
        }
    }
*/
}
void Debug::setDisplay(Display* disp) {
    display = disp;
}
void Debug::processCommand(const char* cmd) {
    //char code;
    uintptr_t addr;
    //int len;
    //char arg[32];
    char code = cmd[0];

    // Skip any leading whitespace after the command character
    const char* arg = cmd + 1;
    while (*arg == ' ') arg++;

    // Strip trailing CR/LF and whitespace from arg
    char varName[32];
    strncpy(varName, arg, sizeof(varName) - 1);
    varName[sizeof(varName) - 1] = '\0';

    // Remove trailing whitespace (e.g. \r, \n, or spaces)
    size_t len = strlen(varName);
    while (len > 0 && (varName[len - 1] == '\r' || varName[len - 1] == '\n' || varName[len - 1] == ' ')) {
        varName[--len] = '\0';
    }

    if (sscanf(cmd, "%c", &code) < 1)
        return;

    switch (code) {
        case 'f':
            viewFrameBuffer1bpp(
                display->getFrameBufferPtr(),
                DISPLAY_WIDTH,
                DISPLAY_HEIGHT
            );        
            break;
        case 'm':
            printMemoryUsage();
            break;
        case 'v':
            if (sscanf(cmd, "v %x %d", (unsigned int*)&addr, &len) == 2) {
                printf("addr:%x len:%d\n",addr,len);
                viewMemory(addr, len);
            } else {
                printf("Usage: v <hex_address> <length>\n");
            }
            break;

        case 'p': // print variable
            printVarByName(arg);
            break;
        case 'h': // print help
            printHelp();
            break;
        default:
            printf("Unknown command: %c\n", code);
            break;
    }
}

void Debug::printMemoryUsage() {
    extern char __StackTop, __StackLimit, __bss_end__, __HeapLimit;
    size_t stack_used = &__StackTop - &__StackLimit;
    size_t heap_used = &__HeapLimit - &__bss_end__;
    printf("Stack used       : %lu bytes\n", (unsigned long)stack_used);
    printf("Heap used        :  %lu bytes\n", (unsigned long)heap_used);
    printf("Stack limit      : %p\n",&__StackLimit);
    printf("Stack Top        : %p\n",&__StackTop);
    printf("Heap top         : %p\n",(&__StackLimit - (10 * 1024)));
    printf("Heap limit       : %p\n",&__HeapLimit);
    printf("Heap size        : %lu\n", ((&__StackLimit - (10 * 1024)) - &__HeapLimit));  
   // printf("START_OF_FREE_RAM: %p\n",&__end__);
   // printf("END_OF_FREE_RAM: %p\n",&__HeapLimit__);   
}

void Debug::viewMemory(uintptr_t addr, size_t len) {
    uint8_t* p = reinterpret_cast<uint8_t*>(addr);
    printf("Viewing %zu bytes at 0x%08x:\n", len, (unsigned)addr);
    for (size_t i = 0; i < len; ++i) {
        if (i % 16 == 0) printf("\n%08x: ", (unsigned)(addr + i));
        printf("%02X ", p[i]);
    }
    printf("\n");
}

void Debug::registerVar(const char* name, void* ptr, size_t size) {
    if (varCount < MAX_VARS) {
        vars[varCount++] = { name, ptr, size };
    }
}
void Debug::printHelp(){
    printf("f - view the frame buffer \n");
    printf("h - this screen, Help\n");
    printf("m - show memory usage\n");
    printf("p variableName - show data of a variable\n");
    printf("v hexAddress length - show data at memory address\n");
    printf("\n");
}
void Debug::printVarByName(const char* name) {
    for (size_t i = 0; i < varCount; ++i) {
        if (strcmp(vars[i].name, name) == 0) {
            printf("%s [%p] = ", vars[i].name, vars[i].ptr);
            if (vars[i].size == sizeof(uint8_t))
                printf("%u\n", *(uint8_t*)vars[i].ptr);
            else if (vars[i].size == sizeof(uint16_t))
                printf("%u\n", *(uint16_t*)vars[i].ptr);
            else if (vars[i].size == sizeof(uint32_t))
                printf("%u\n", *(uint32_t*)vars[i].ptr);
            else
                printf("unsupported size (%zu)\n", vars[i].size);
            return;
        }
    }
    printf("Variable '%s' not found\n", name);
}

void Debug::viewFrameBuffer1bpp(uint8_t* buf, uint8_t width, uint8_t height) {
    for (uint8_t y = 0; y < height; ++y) {
        printf("%03u: ", y);
        for (uint8_t x = 0; x < width; ++x) {
            uint index = (y / 8) * width + x;
            uint8_t bitmask = 1 << (y % 8);
            bool pixel_on = buf[index] & bitmask;
            putchar(pixel_on ? '#' : '.');
        }
        printf("\n");
    }
}