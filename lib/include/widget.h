#pragma once
#include <cstdint>
#include "common.h"

enum class WidgetType {
    Label,
    Button,
    Edit,
    Menu
};

class Widget {
protected:
    Rect2 boundingBox;
    uint32_t id;
    WidgetType widgetType;
    //int value;
    bool selectable = false;   // default: non-selectable
    bool selected   = false;   // is this currently focused?
    bool active     = false;   // if widget is controllable, select and press OK  

public:
    Widget(const std::string& text, int x, int y, int width, int height);
    std::string label;

    WidgetType getWidgetType() const {return widgetType;} 
    uint32_t getWidgetId() const { return id; }
    void setId(uint32_t wi){id=wi;}  
    virtual void draw(Display *display) const = 0; // Pure virtual function
    virtual ~Widget();
    const std::string& getLabel() const { return label; }
    //virtual int getValue() const {return value;}
    //virtual void setValue(int val){ if (selectable) value = val; }
    virtual bool isSelected() const { if (selectable) return selected; return false;}
    virtual bool isActive() const { if (selectable) return active; return false;}   
    virtual void setSelected(bool s)  { 
        if (selectable) {
            selected = s; 
        }
        if(s) TRACE("%d selected",id); else TRACE("%d not selected",id);
    }
    virtual void setActive(bool a)    { 
        if (selectable) active = a; 
        if(a) TRACE("%d active",id); else TRACE("%d not active",id);
    }
    virtual bool isSelectable() const { return selectable; }
};

