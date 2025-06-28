#include "common.h"
#include "display.h"
#include "widget.h"
#include <vector>

#ifndef SCREEN_H
    #define SCREEN_H

    enum ScreenEnum {  MENUSCREEN, TESTSCREEN, SETTINGSSCREEN, ABOUTSCREEN,BASICSCREEN };

    class Screen {

    public:
        virtual void update(uint16_t deltaTimeMS) = 0;
        virtual void draw(Display *display) = 0;
        virtual int keyPressed(uint8_t key) = 0;
        virtual int keyReleased(uint8_t key) = 0;
        virtual int keyDown(uint8_t key) = 0;
        virtual  void addWidget(Widget* widget,uint32_t widgetId)=0;
        virtual ~Screen() {}
        uint8_t screenId = 0;
        ScreenEnum scrEnum;
        Rect2 refresh;
        std::vector<Widget*> widgets;
    };

#endif