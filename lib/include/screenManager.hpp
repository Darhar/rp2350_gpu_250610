#pragma once
#include "common.h"
#include <vector>
#include "screen.h"
#include <functional>

#define SCREEN_COUNT 2

using ScreenFactoryFunc = std::function<Screen*()>;
 
struct WidgetDescriptor {
    WidgetType type;
    uint32_t widgetId;
    std::string initialText;
    int x, y, width, height;
    // additional persistent state…
    int initialValue;   // for EDITs
};

struct ScreenDescriptor {
    ScreenEnum id;
    std::vector<WidgetDescriptor> widgets;
    // e.g., selected option, scroll offsets, etc.
};


class ScreenManager {
    private:
        std::map<ScreenEnum, ScreenFactoryFunc> screenObjects;
        std::map<ScreenEnum, ScreenDescriptor>    screenData;
        Screen* activeScreen = nullptr;
        ScreenEnum currentScreen;
        Rect2 refresh;
        int keyReturn;//
    public:
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
    