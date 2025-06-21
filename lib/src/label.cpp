#include "label.h"


Label::Label(const std::string& text, int x, int y, int width, int height)
    : Widget(text, x, y, width, height) {
        widgetType = WidgetType::Label;
    }

void Label::draw(Display *disp) const {
    printf("draw label");
    if (selected) {
        disp->setInverted(true);
    } else {
        disp->setInverted(false);
    }
    term6x9->drawText(disp, label, Vec2(boundingBox.x, boundingBox.y), 255, 1);        

}

void Label::activate() {
    
}
