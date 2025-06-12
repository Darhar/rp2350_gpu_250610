# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(i2c_ui_lib C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


# Add executable. Default name is the project name, version 0.1

#add_executable(dt300cpp_00 
#    common.cpp
#    framebuffer.cpp
#    display.cpp
#    keyboard.cpp
#    image.cpp
#    sprite.cpp
#    splashscreen.cpp
#    testscreen.cpp
#    menuscreen.cpp
#    settingsscreen.cpp 
#    aboutscreen.cpp
#    basicscreen.cpp
#    menu.cpp
#    widget.cpp
#    label.cpp
#    button.cpp
#    edit.cpp 
#    screenManager.cpp
#    dt300cpp_00.cpp 
#)

add_subdirectory(lib)
add_subdirectory(examples/rp2040_ui_demo)


#pico_set_program_name( "dt300cpp_00")
#pico_set_program_version(dt300cpp_00 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(i2c_ui_lib 0)
pico_enable_stdio_usb(i2c_ui_lib 1)

# Add the standard library to the build
target_link_libraries(i2c_ui_lib
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(i2c_ui_lib PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(i2c_ui_lib 
    pico_i2c_slave
    hardware_spi
    hardware_dma
    hardware_adc
    hardware_i2c  
    pico_multicore         
    )

#pico_add_extra_outputs(i2c_ui_lib)


