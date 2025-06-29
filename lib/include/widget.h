#pragma once
#include <cstdint>
#include "common.h"

enum class WidgetType {
    Label,
    Button,
    Edit
};

class Widget {
protected:
    Rect2 boundingBox;
    uint32_t id;
    WidgetType widgetType;
    bool selectable = false;   // default: non-selectable
    bool selected   = false;   // is this currently focused?
    bool active     = false;   // only meaningful if selectable==true   

public:
    Widget(const std::string& text, int x, int y, int width, int height);
    std::string label;

    WidgetType getWidgetType() const {return widgetType;} 
    uint32_t getWidgetId() const { return id; }
    void setId(uint32_t wi){id=wi;}  
    virtual void draw(Display *display) const = 0; // Pure virtual function
    virtual ~Widget();


    virtual void setSelected(bool s)  { if (selectable) selected = s; }
    virtual bool isSelected()  const { if (selectable) return selected; return false;}
    virtual void setActive(bool a)    { if (selectable) active = a; }
    virtual bool isActive()   const  { if (selectable) return active; return false;}   
    virtual bool isSelectable() const { return selectable; }



};

