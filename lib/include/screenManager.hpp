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
};

class ScreenManager {
    uint16_t buildingSid_ = 0xFFFF; // invalid means “not building”
    public:
        ScreenManager();
        ~ScreenManager();
        Widget* createWidgetFromConfigAndState(const WidgetConfig& c,
                                            uint16_t sid,
                                            WidgetState* st /*unused*/);
        auto& getConfig(ScreenEnum id) { return configs[id]; }
        auto& getState(ScreenEnum id)  { return states[id];  }
        void registerScreen(ScreenEnum id, ScreenFactoryFunc factory);
        void registerFactory(ScreenEnum id, ScreenFactoryFunc func);
        Screen* buildScreenFromDescriptor(ScreenEnum id);       
        void setActiveScreen(ScreenEnum id);
        Screen* getActiveScreen();
        void update(uint16_t deltaTimeMS);
        void draw(Display* display);
        void keyPressed(uint8_t key);
        void keyDown(uint8_t key);
        void keyReleased(uint8_t key);
        void nextScreen();
        void previousScreen(); 
        void setRefreshRect(Rect2 refRect);  
        void disableRefresh(); 
        bool needRefresh();
        // Declare VS keys for a single screen. If ensureConfig=true and the config
        // is empty, we temporarily build the screen to let it seed its config.
        void declareWidgetKeysFor(ScreenEnum id, bool ensureConfig = true);

        // Declare VS keys for all registered screens.
        void declareWidgetKeysForAll(bool ensureConfig = true);

        // (Optional convenience) declare for the current active screen only.
        void declareWidgetKeysForActive();
        void beginWidgetBuild(uint16_t sid) { buildingSid_ = sid; }
        void endWidgetBuild()               { buildingSid_ = 0xFFFF; }        
    private:
        std::map<ScreenEnum, ScreenFactoryFunc> screenObjects_;  // instance registry
        // per‐screen lists, keyed by screenId:
        std::map<ScreenEnum, std::vector<WidgetConfig>> configs;
        std::map<ScreenEnum, std::vector<WidgetState>> states;
        int screenCount=0;
        Screen* activeScreen = nullptr;
        ScreenEnum currentScreen;
        Rect2 refresh;
        int keyReturn;
    };
    