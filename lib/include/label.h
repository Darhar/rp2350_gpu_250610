#ifndef LABEL_H
#define LABEL_H

#include <string>
#include "widget.h"
#include "display.h"

class Label : public Widget {
public:

    Label(const std::string& text, int x, int y, int width, int height);
    void draw(Display *display) const override;

};

#endif // LABEL_H
