#pragma once

#include "common.h"
#include "screen.h" 
#include <vector>
#include <functional>
#include <map>
#include "widgetDescriptor.hpp"
#include "widget.h"

using ScreenFactoryFunc = std::function<Screen*()>;

extern std::map<ScreenEnum, ScreenFactoryFunc> screenObjects;

struct ScreenDescriptor {
    std::vector<WidgetDescriptor> widgets;
    // for future screen-related persistent data
};

class ScreenManager {
    private:
        std::map<ScreenEnum, ScreenFactoryFunc> screenObjects_;  // instance registry
        std::map<ScreenEnum, ScreenDescriptor>    screenData;
        // per‐screen lists, keyed by screenId:
        std::map<ScreenEnum, std::vector<WidgetConfig>> configs;
        std::map<ScreenEnum, std::vector<WidgetState>> states;


        int screenCount=0;
        Screen* activeScreen = nullptr;
        ScreenEnum currentScreen;
        Rect2 refresh;
        int keyReturn;//

    public:
        ScreenManager();
        ~ScreenManager();

        
        auto& getConfig(ScreenEnum id) { return configs[id]; }
        auto& getState(ScreenEnum id)  { return states[id];  }
        Widget* createWidgetFromConfigAndState( const WidgetConfig& c, WidgetState* st);
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
    