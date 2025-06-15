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

project(rp2040_ui_demo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(lib)

# 2) Gather all screen sources
file(GLOB SCREEN_SRCS
  ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/screens/src/*.cpp
)

# 3) Define the demo executable *at the root level*
add_executable(${PROJECT_NAME}
  examples/rp2040_ui_demo/main.cpp
  ${SCREEN_SRCS}
)

# 4) Include paths for core library and screens
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/lib/include
  ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/screens/include
)

# 5) Link against libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
  i2c_ui_lib
  pico_stdlib
  pico_i2c_slave
  hardware_spi
  hardware_dma
  hardware_adc
  hardware_i2c
  pico_multicore
)

# 6) Enable stdio and USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# 7) Generate the UF2 in build/ so “Run Project” finds it
pico_add_extra_outputs(${PROJECT_NAME})