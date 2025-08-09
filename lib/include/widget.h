#pragma once
#include <cstdint>
#include "common.h"
#include <variant>   // if you’re still using std::variant elsewhere
#include <vector>    // <— for std::vector
#include <string>    // <— for std::string

enum class WidgetType {
    Label,
    Button,
    Edit,
    Menu
};

struct WidgetConfig {
    WidgetType      type;
    uint32_t        widgetId;
    std::string     label;
    uint16_t        x, y, width, height;
    bool            selectable;

    // only for Edit:
    int             initialValue, minValue, maxValue;
    // only for Button:
    bool            toggleOn;
    std::string     captionOn, captionOff;

    // for Menu:
    std::vector<std::string> menuItems;
    int                      initialSelection;
};

struct WidgetState {
    uint32_t                widgetId;
    // only ever holds the live value you need to persist:
    std::variant<
      std::monostate,  // unused
      bool,            // Button state
      int              // Menu selection or Edit value
    > data;
};

class Widget {
    public:
        Widget(const std::string& text, int x, int y, int width, int height);
        std::string label;

        WidgetType getWidgetType() const {return widgetType;} 
        uint32_t getWidgetId() const { return id; }
        void setId(uint32_t wi){id=wi;}  
        virtual void draw(Display *display) const = 0; // Pure virtual function
        virtual ~Widget();
        const std::string& getLabel() const { return label; }
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
    protected:
        Rect2 boundingBox;
        uint32_t id;
        WidgetType widgetType;
        bool selectable = false;   // default: non-selectable
        bool selected   = false;   // is this currently focused?
        bool active     = false;   // if widget is controllable, select and press OK  


};

