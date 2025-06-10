#ifndef EDIT_H
#define EDIT_H

#include <string>
#include "widget.h"

class Edit : public Widget {

private:
    Size2 buttSize;
    Vec2 buttonPos;
    Rect2 butRect;
    const uint8_t buttWidth=40;
    bool active;
    int minValue;
    int maxValue;

public:
    int value;
    Edit(const std::string& text, int x, int y, int width, int height,int value_,int mn,int mx);
    bool getActive(){
        return active;
    };
    void setActive(bool atv){
        active=atv;
    }
    void setMin(int mnVal){
        minValue=mnVal;
    }
    void setMax(int mxVal)
    {
        maxValue=mxVal;
    }   
    int getValue(){
        return value;
    }
    void setValue(int val);
    bool isSelectable() const override { return true; }
    void draw(Display *display) const override;
    void activate();
};

#endif // EDIT_H
