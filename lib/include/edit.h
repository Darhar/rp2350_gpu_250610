#pragma once

#include <string>
#include "widget.h"
#include <functional>

class Edit : public Widget {
    public:
        Edit(const std::string& text, int x, int y, int width, int height, int value_, int mn, int mx);
        
        void setMin(int mnVal){minValue=mnVal;}	
        void setMax(int mxVal){ maxValue=mxVal; } 
        void setValue(int val);
        int  getValue() const { return value; }        
        void draw(Display *display) const override;
	
    private:
        Vec2 buttonPos;
        Rect2 butRect;
        const uint8_t buttWidth=40;
        int minValue;
        int maxValue;
        int value;
};

