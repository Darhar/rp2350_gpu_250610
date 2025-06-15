
#project(rp2040_ui_demo)
pico_sdk_init()

file(GLOB SCREEN_SRCS screens/*.cpp)
add_executable(rp2040_ui_demo main.cpp ${SCREEN_SRCS})

target_include_directories(rp2040_ui_demo 
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/screens
)

target_link_libraries(rp2040_ui_demo 
  PUBLIC 
    i2c_ui_lib 
    pico_stdlib 
    hardware_i2c
)

# <-- This line makes the UF2 appear in build/, not build/examples/…
#set(PICO_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#pico_add_extra_outputs(rp2040_ui_demo)
#  add_custom_target(run
#      COMMENT "Flashing rp2040_ui_demo.uf2 to your RP2040 via picotool"
#      DEPENDS rp2040_ui_demo  # ensure the UF2 is up to date
#      COMMAND picotool load
#          $<TARGET_FILE:rp2040_ui_demo>.uf2
#)
# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(rp2040_ui_demo 0)
pico_enable_stdio_usb(rp2040_ui_demo 1)

pico_add_extra_outputs(rp2040_ui_demo)
file(COPY C:/Users/dlind/Documents/pico/vs_2/rp2350_gpu/build/examples/rp2040_ui_demo/rp2040_ui_demo.uf2 
  DESTINATION C:/Users/dlind/Documents/pico/vs_2/rp2350_gpu/build)