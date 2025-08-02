#include "button.h"
#include <iostream>

Button::Button(const std::string& label,
               const std::string& captionOn,
               const std::string& captionOff,
               int x, int y, int w, int h,
               bool initialState)
    : Widget(label, x, y, w, h),
      captionOn(captionOn),
      captionOff(captionOff),
      state(initialState)
{
    TRACE("");
    caption = state ? captionOn : captionOff;
    selectable = true;
    widgetType = WidgetType::Button;
    Size2 buttSize=Size2(buttWidth,boundingBox.h);
    buttonPos=Vec2(boundingBox.x+boundingBox.w-buttSize.w,boundingBox.y);
    butRect=Rect2(buttonPos,buttSize);   
}

void Button::toggle() {
    state = !state;
    caption = state ? captionOn : captionOff;
}

const std::string& Button::getCaption() const {
    return caption;
}

void Button::draw(Display *disp) const {
    TRACE_CAT(UI,"selected:%d",selected);
    ariel5x8->drawText(disp, label, Vec2(boundingBox.x, boundingBox.y), 255, 1);
    disp->setInverted(selected);
    disp->fillRect(butRect,  0, 255);
    disp->rect(butRect,1);
    //text    
    ariel5x8->drawText(disp, caption, Vec2(buttonPos.x+(buttWidth-(caption.size()*5))/2, buttonPos.y+2), 255, 1);
    disp->setInverted(false);
}

