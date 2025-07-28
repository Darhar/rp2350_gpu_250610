#pragma once
#include "common.h"
#include <vector>
#include "screen.h"
#include <functional>
#include "widgetDescriptor.hpp"

using ScreenFactoryFunc = std::function<Screen*()>;

struct ScreenDescriptor {
    std::vector<WidgetDescriptor> widgets;
    //for future screen related persistant data
};

class ScreenManager {
    private:
        std::map<ScreenEnum, ScreenFactoryFunc> screenObjects;
        std::map<ScreenEnum, ScreenDescriptor>    screenData;
        int screenCount=0;
        Screen* activeScreen = nullptr;
        ScreenEnum currentScreen;
        Rect2 refresh;
        int keyReturn;//
    public:
        ScreenManager();
        ~ScreenManager();

        void registerScreen(ScreenEnum id, ScreenFactoryFunc factory);
        void registerFactory(ScreenEnum id, ScreenFactoryFunc func);
        void setActiveScreen(ScreenEnum id);
        void update(uint16_t deltaTimeMS);
        void draw(Display* display);
        void keyPressed(uint8_t key);
        void keyDown(uint8_t key);
        void keyReleased(uint8_t key);
        Screen* getActiveScreen();
        Screen* buildScreenFromDescriptor(ScreenEnum id);   
        Widget* createWidgetFromDescriptor(const WidgetDescriptor& wd);     
        void nextScreen();
        void previousScreen(); 
        void setRefreshRect(Rect2 refRect);  
        void disableRefresh(); 
        bool needRefresh();
        ScreenDescriptor& getDescriptor(ScreenEnum id);
    };
    