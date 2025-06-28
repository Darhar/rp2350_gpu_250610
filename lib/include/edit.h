#ifndef EDIT_H
#define EDIT_H

#include <string>
#include "widget.h"
#include <functional>

class Edit : public Widget {

public:

    Edit(const std::string& text, int x, int y, int width, int height, int value_, int mn, int mx);
	
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
	
    void setValue(int val);
    bool isSelectable() const override { return true; }
    void draw(Display *display) const override;
    void activate(bool activ);
    void activateToggle(); 	
	int  getValue()   const;
	
	private:
    Size2 buttSize;
    Vec2 buttonPos;
    Rect2 butRect;
    const uint8_t buttWidth=40;
    bool active=false;
    int minValue;
    int maxValue;
	int value;

};

#endif 
