# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# ---- \CMakeLists.txt (top level) ----
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == VS Code Pico extension boilerplate ==
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
# =====================================

# Pick your board here (change to tiny-2350 if/when you make a custom definition)
set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(rp2040_ui_demo C CXX ASM)
pico_sdk_init()

# ---- Core library -----------------------------------------------------------
add_subdirectory(lib)

# ---- Collect screen sources (shared by all demos) ---------------------------
file(GLOB SCREEN_SRCS
  ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/screens/src/*.cpp
)
# Paths
set(LIB_INC  ${CMAKE_SOURCE_DIR}/lib/include)
set(SCR_INC  ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/screens/include)
set(DEMO_DIR ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/demos)
# ---- Demos configuration ----------------------------------------------------
# Put the base names of your demo .cpp files here (without extension)
# Each should exist at: examples/rp2040_ui_demo/demos/<name>.cpp
set(DEMOS
  ui_demo
  #screens_demo
  #perf_demo
)
set(DEMO_DIR ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/demos)  # ensure you have this

# Pick which demo becomes the "default" rp2040_ui_demo target so th Run button works
# You need to 
set(SELECTED_DEMO "ui_demo" CACHE STRING "Which demo to expose as rp2040_ui_demo")
set_property(CACHE SELECTED_DEMO PROPERTY STRINGS ${DEMOS})

# Let us flip this from presets or VS Code later:
option(WAIT_FOR_SERIAL "Block until USB CDC opens" ON)

# Helper to convert ON/OFF -> 1/0 at compile time
set(WAITFORSERIAL_EXPR "$<IF:$<BOOL:${WAIT_FOR_SERIAL}>,1,0>")

function(add_ui_demo DEMO_NAME)
  # Collect this demo's sources:
  #  - ui_demo.cpp
  #  - ui_demo_impl.cpp (and any other ui_demo*.cpp)
  file(GLOB DEMO_SRCS
    ${DEMO_DIR}/${DEMO_NAME}.cpp
    ${DEMO_DIR}/${DEMO_NAME}_*.cpp
    ${DEMO_DIR}/${DEMO_NAME}*.cpp
  )
  if (NOT DEMO_SRCS)
    message(FATAL_ERROR
      "No sources for demo '${DEMO_NAME}'. "
      "Expected at least: ${DEMO_DIR}/${DEMO_NAME}.cpp (and optionally ${DEMO_DIR}/${DEMO_NAME}_*.cpp)")
  endif()

  add_executable(${DEMO_NAME}
    ${DEMO_SRCS}
    ${SCREEN_SRCS}
  )

  # Which demo is this (handy for printf at runtime)
  target_compile_definitions(${DEMO_NAME} PRIVATE
    RP_DEMO_NAME="${DEMO_NAME}"
    WAITFORSERIAL=${WAITFORSERIAL_EXPR}
  )

  target_include_directories(${DEMO_NAME} PRIVATE
    ${LIB_INC}
    ${SCR_INC}
    ${DEMO_DIR}/include
  )

  target_link_libraries(${DEMO_NAME} PUBLIC
    i2c_ui_lib
    pico_stdlib
    pico_i2c_slave
    hardware_spi
    hardware_dma
    hardware_adc
    hardware_i2c
    pico_multicore
  )

  # Handy at runtime: printf("Running demo: %s\n", RP_DEMO_NAME);
  target_compile_definitions(${DEMO_NAME} PRIVATE RP_DEMO_NAME="${DEMO_NAME}")

  pico_enable_stdio_uart(${DEMO_NAME} 0)
  pico_enable_stdio_usb(${DEMO_NAME} 1)
  pico_add_extra_outputs(${DEMO_NAME})
endfunction()

# For the canonical VS Code "Run" target, also glob all files for the selected demo:
file(GLOB SELECTED_DEMO_SRCS ${DEMO_DIR}/${SELECTED_DEMO}*.cpp)

add_executable(${PROJECT_NAME}
  ${SELECTED_DEMO_SRCS}
  ${SCREEN_SRCS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  RP_DEMO_NAME="${SELECTED_DEMO}"
  WAITFORSERIAL=${WAITFORSERIAL_EXPR}
)

# >>> Add include paths so headers resolve
target_include_directories(${PROJECT_NAME} PRIVATE
  ${LIB_INC}
  ${SCR_INC}
  ${DEMO_DIR}/include
  # Optional fallback if some headers live directly under screens/ (no include/):
  ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/screens
)

# >>> Link Pico + your core lib so pico/stdlib.h & friends resolve
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

# >>> Same stdio/outputs as other demos
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_add_extra_outputs(${PROJECT_NAME})
