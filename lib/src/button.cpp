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
    TRACE_CAT(UI,"buttSize w,h:%d,%d buttonPos x,y:%d,%d butRect w,h:%d,%d",buttSize.w,buttSize.h,buttonPos.x,buttonPos.y,butRect.w,butRect.h);

}
void Button::setState(bool on) {
    if (state == on) return;
    state   = on;
    caption = state ? captionOn : captionOff;
    // (optional) trigger redraw upstream if you have a mechanism
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

