#include "edit.h"
#include <iostream>

Edit::Edit(const std::string& text, int x, int y, int width, int height,int value_,int mn, int mx)
    : Widget(text,x, y, width, height), value(value_),minValue(mn),maxValue(mx) {

    widgetType = WidgetType::Edit;
    buttSize=Size2(buttWidth,boundingBox.h);
    buttonPos=Vec2(boundingBox.x+boundingBox.w-buttSize.w,boundingBox.y);
    butRect=Rect2(buttonPos,buttSize);
    active=false;
}

void Edit::draw(Display *disp) const {

    //label
    ariel5x8->drawText(disp, label, Vec2(boundingBox.x, boundingBox.y), 255, 1);
    disp->setInverted(selected);
    disp->fillRect(butRect,  0, 255);
    disp->rect(butRect,1);
    //text
    char intBuf[10];
    sprintf(intBuf,"%d",value);  
    ariel5x8->drawText(disp, intBuf, Vec2(buttonPos.x+3, buttonPos.y+2), 255, 1);
    disp->setInverted(false);
    if(active){
        ariel5x8->drawText(disp, ">", Vec2(buttonPos.x-6, buttonPos.y+2), 255, 1);
    }    
}

void Edit::setValue(int val){
    if(val>maxValue) val=maxValue;
        else if(val<minValue) val=minValue;
        else value=val;
}

void Edit::activate() { 
    printf("[Edit] %d active:%d,\n",widgetType,getActive());
    setActive(!getActive());
    printf("[Edit] %d active:%d,\n",widgetType,getActive());
 
}