
project(rp2040_ui_demo)
pico_sdk_init()
file(GLOB SCREEN_SRCS screens/*.cpp)
add_executable(rp2040_ui_demo main.cpp ${SCREEN_SRCS})
target_include_directories(rp2040_ui_demo PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/screens
)
target_link_libraries(rp2040_ui_demo PUBLIC i2c_ui_lib pico_stdlib hardware_i2c)
pico_add_extra_outputs(rp2040_ui_demo)
