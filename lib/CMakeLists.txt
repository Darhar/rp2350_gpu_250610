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
)
target_include_directories(i2c_ui_lib PUBLIC include)
target_include_directories(i2c_ui_lib PUBLIC ${CMAKE_SOURCE_DIR}/lib/include)
