#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include "widget.h"

class Button : public Widget {

private:
    void buttonGraphic(Display *disp) const;
    Size2 buttSize;
    Vec2 buttonPos;
    Rect2 butRect;
    const uint8_t buttWidth=40;

public:
    std::string caption;
    Button(const std::string& text, int x, int y, int width, int height,const std::string& caption_);
    
    bool isSelectable() const override { return true; }
    void draw(Display *display) const override;
    void activate();
};

#endif // BUTTON_H
