#pragma once

#include <string>
#include "widget.h"
#include "display.h"
#include <vector>

class Menu : public Widget {
    public:

        Menu(
            const std::string& text,
            const std::vector<std::string> items,
            int selectedMenuItem,
            int x, int y, int width, int height
        );

        void draw(Display *display) const override;
        void changeMenuSelection(int dirctn);
        int  getValue() const {
            TRACE_CAT(UI,"selectedMenuItem:%d",selectedMenuItem);
            return selectedMenuItem;
        }        

    private:
        std::vector<std::string> items;
        int itemCount;
        int selectedMenuItem;
        int  fontHeight=10;
        int  fontWidth=5;
        bool show=false;
        int menuOffs=50;
   
};


