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

# ---- Demos configuration ----------------------------------------------------
# Put the base names of your demo .cpp files here (without extension)
# Each should exist at: examples/rp2040_ui_demo/demos/<name>.cpp
set(DEMOS
  #ui_demo
  screens_demo
  #perf_demo
)

# Pick which demo becomes the "default" rp2040_ui_demo target so th Run button works
# You need to 
set(SELECTED_DEMO "screens_demo" CACHE STRING "Which demo to expose as rp2040_ui_demo")
set_property(CACHE SELECTED_DEMO PROPERTY STRINGS ${DEMOS})

# ---- Helper function to define a demo executable ---------------------------
function(add_ui_demo DEMO_NAME)
  set(_demo_src ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/demos/${DEMO_NAME}.cpp)

  if (NOT EXISTS ${_demo_src})
    message(FATAL_ERROR "Demo source not found: ${_demo_src}")
  endif()

  add_executable(${DEMO_NAME}
    ${_demo_src}
    ${SCREEN_SRCS}
  )

  target_include_directories(${DEMO_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/include
    ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/screens/include
    ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/demos/include
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

  pico_enable_stdio_uart(${DEMO_NAME} 0)
  pico_enable_stdio_usb(${DEMO_NAME} 1)

  # Emit UF2/ELF/bin for this demo
  pico_add_extra_outputs(${DEMO_NAME})
endfunction()

# ---- Create one target per demo (nice for trying multiple) ------------------
foreach(D IN LISTS DEMOS)
  add_ui_demo(${D})
endforeach()

# ---- Create the canonical "rp2040_ui_demo" target for VS Code --------------
# This is a separate executable that uses the selected demo's .cpp
message(STATUS "Selected demo for rp2040_ui_demo: ${SELECTED_DEMO}")
add_executable(${PROJECT_NAME}
  ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/demos/${SELECTED_DEMO}.cpp
  ${SCREEN_SRCS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/lib/include
  ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/screens/include
  ${CMAKE_SOURCE_DIR}/examples/rp2040_ui_demo/demos/include
)

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

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_add_extra_outputs(${PROJECT_NAME})
