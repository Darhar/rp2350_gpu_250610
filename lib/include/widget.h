#ifndef WIDGET_H
#define WIDGET_H

#include "common.h"
enum class WidgetType {
    Label,
    Button,
    Edit
};

class Widget {
protected:
    Rect2 boundingBox;
    bool selected = false;
    //std::string label;
    WidgetType widgetType;

public:
    Widget(const std::string& text, int x, int y, int width, int height);
    std::string label;

    virtual void activate() {}
    virtual bool isSelectable() const { return false; }
    void setSelected(bool sel) { selected = sel; }
    WidgetType getWidgetType() const {
        return widgetType;
    } 
       
    virtual void draw(Display *display) const = 0; // Pure virtual function
    virtual ~Widget();
};

#endif // WIDGET_H