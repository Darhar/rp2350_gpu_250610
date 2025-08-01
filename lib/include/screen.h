#include "common.h"
#include "display.h"
#include "widget.h"
#include <vector>
#include <algorithm>
#include "screenManager.hpp"

#ifndef SCREEN_H
    #define SCREEN_H
    class ScreenManager;
    enum ScreenEnum {  MENUSCREEN, TESTSCREEN, SETTINGSSCREEN, ABOUTSCREEN,BASICSCREEN,SPLASHSCREEN };

    class Screen {

    public:
        virtual void update(uint16_t deltaTimeMS) = 0;
        virtual void draw(Display *display) = 0;
        virtual int keyPressed(uint8_t key) = 0;
        virtual int keyReleased(uint8_t key) = 0;
        virtual int keyDown(uint8_t key) = 0;
        virtual ~Screen() {}
        //uint8_t screenId = 0;
        ScreenEnum screenId;
        //ScreenEnum scrEnum;
        Rect2 refresh;
        std::vector<Widget*> widgets;
        int selectedIndex=-1;
        void addWidget(Widget* w, uint32_t id);
 
    protected:

        void rebuildFromDescriptor(ScreenManager& mgr); 
        void drawWidgets(Display *disp) {
            // First pass: draw all inactive widgets
            for (auto* w : widgets) {
                if (!w->isActive()) {
                    w->draw(disp);
                }
            }

            // Second pass: draw all active widgets on top
            for (auto* w : widgets) {
                if (w->isActive()) {
                    w->draw(disp);
                }
            }
        }  
        
    };

#endif