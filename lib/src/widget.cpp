#include "widget.h"

Widget::Widget(const std::string& text, int x, int y, int width, int height)
    : boundingBox(x, y, width, height), label(text) {TRACE("");}

Widget::~Widget() = default;
