#include "button.h"
#include <iostream>

Button::Button(const std::string& text, int x, int y, int width, int height,const std::string& caption_)
    : Widget(text,x, y, width, height), caption(caption_) {
    printf("Size of caption %s=%d\n",caption.c_str(),caption.size());
    widgetType = WidgetType::Button;
    buttSize=Size2(buttWidth,boundingBox.h);
    buttonPos=Vec2(boundingBox.x+boundingBox.w-buttSize.w,boundingBox.y);
    butRect=Rect2(buttonPos,buttSize);
}

void Button::draw(Display *disp) const {
    printf("draw Button\n");
    buttonGraphic(disp);
}

void Button::buttonGraphic(Display *disp) const{
    //label
    printf("%s : pos %d,%d\n",label,boundingBox.x, boundingBox.y);
    ariel5x8->drawText(disp, label, Vec2(boundingBox.x, boundingBox.y), 255, 1);
    disp->setInverted(selected);
    disp->fillRect(butRect,  0, 255);
    disp->rect(butRect,1);
    //text    
    ariel5x8->drawText(disp, caption, Vec2(buttonPos.x+(buttWidth-(caption.size()*5))/2, buttonPos.y+2), 255, 1);
    disp->setInverted(false);
}

void Button::activate() {
    
}
