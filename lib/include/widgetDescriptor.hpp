#include "common.h"
#include "widget.h"

struct WidgetDescriptor {
    // === Common to all widgets ===
    WidgetType type;
    uint32_t widgetId;
    std::string label;
    int x, y, width, height;

    // === Optional fields ===
    bool selectable = false;  // if you want to make this part of the descriptor

    // === Widget-specific ===
    // For Edit
    int initialValue = 0;
    int minValue = 0;
    int maxValue = 100;

    // For Button (ToggleButton)
    bool toggleState = false;
    std::string captionOn;
    std::string captionOff;

};