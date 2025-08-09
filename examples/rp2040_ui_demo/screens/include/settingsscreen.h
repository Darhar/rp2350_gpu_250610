#pragma once

#include <common.h>
#include <screen.h>
#include <keyboard.h>
//#include <label.h>
#include <button.h>
#include <menu.h>
#include <edit.h>
#include <vector>
#include "screenManager.hpp"

class SettingsScreen : public Screen
{
    private:
        std::string title;   
		uint16_t duration, lastTime,accDeltaTimeMS;
		ScreenEnum     scrEnum;
        int selectedIndex = -1;
        void seedConfig();

    public:
        SettingsScreen(ScreenManager& mgr);
        ~SettingsScreen(); 
        void update(uint16_t deltaTimeMS);
        void draw(Display *display);
        int keyPressed(uint8_t key);
        int keyReleased(uint8_t key);
        int keyDown(uint8_t key);
};
