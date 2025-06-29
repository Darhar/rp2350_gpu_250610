#pragma once

#include <string>
#include "widget.h"

class Button : public Widget {

public:
    Button(const std::string& text, int x, int y, int width, int height,const std::string& caption_);

    void draw(Display *display) const override;
    std::string caption;

private:
    void buttonGraphic(Display *disp) const;
    Size2 buttSize;
    Vec2 buttonPos;
    Rect2 butRect;
    const uint8_t buttWidth=40;  
};


