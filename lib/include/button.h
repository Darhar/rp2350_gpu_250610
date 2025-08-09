#pragma once

#include <string>
#include "widget.h"

class Button : public Widget {

public:

    Button(const std::string& label,
           const std::string& captionOn,
           const std::string& captionOff,
           int x, int y, int w, int h,
           bool initialState = false);

    void toggle();
    bool getState() const { return state; }
    const std::string& getCaption() const;
    void draw(Display* display) const override;

private:
    Vec2 buttonPos;
    Rect2 butRect;
    const uint8_t buttWidth=40;  
    std::string caption;
    std::string captionOn;
    std::string captionOff;
    bool state = false;
};


