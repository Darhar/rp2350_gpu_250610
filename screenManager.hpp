#pragma once
#include "common.h"
#include <vector>
#include "screen.h"

#define SCREEN_COUNT 5

using ScreenFactoryFunc = Screen* (*)();

class ScreenManager {
    private:
        std::map<ScreenEnum, ScreenFactoryFunc> factories;
        Screen* activeScreen = nullptr;
        ScreenEnum currentScreen;
        Rect2 refresh;
        int keyReturn;//

    public:
        ~ScreenManager();
    
        void registerFactory(ScreenEnum id, ScreenFactoryFunc func);
        void setActiveScreen(ScreenEnum id);
        void update(uint16_t deltaTimeMS);
        void draw(Display* display);
        void keyPressed(uint8_t key);
        void keyDown(uint8_t key);
        void keyReleased(uint8_t key);
        Screen* getActiveScreen();
        void nextScreen();
        void previousScreen(); 
        void setRefreshRect(Rect2 refRect);  
        void disableRefresh(); 
        bool needRefresh();
    };
    