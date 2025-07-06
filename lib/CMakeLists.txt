add_library(i2c_ui_lib
  src/button.cpp
  src/common.cpp
  src/display.cpp
  src/edit.cpp
  src/framebuffer.cpp
  src/image.cpp
  src/keyboard.cpp
  src/label.cpp
  src/menu.cpp
  src/screenManager.cpp
  src/sprite.cpp
  src/widget.cpp
  src/debug.cpp
  src/screen.cpp
)

target_include_directories(i2c_ui_lib
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

# this wasnt needed before
target_link_libraries(i2c_ui_lib 
    pico_stdlib
    pico_i2c_slave
    hardware_spi
    hardware_dma
    hardware_adc
    hardware_i2c  
    pico_multicore        
)
