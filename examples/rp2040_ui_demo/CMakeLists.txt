

#include(pico_sdk_import.cmake)
project(rp2040_ui_demo)
pico_sdk_init()
add_executable(rp2040_ui_demo main.cpp)
target_link_libraries(rp2040_ui_demo PUBLIC i2c_ui_lib pico_stdlib hardware_i2c)
pico_add_extra_outputs(rp2040_ui_demo)