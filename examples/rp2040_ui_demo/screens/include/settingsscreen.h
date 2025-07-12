#pragma once

#include <common.h>
#include <screen.h>
#include <keyboard.h>
//#include <label.h>
#include <button.h>
#include <menu.h>
#include <vector>
#include "screenManager.hpp"

class SettingsScreen : public Screen
{
    private:
        std::string title;   
		ScreenManager& mgr;
		uint16_t duration, lastTime,accDeltaTimeMS;
		ScreenEnum     scrEnum;
        int selectedIndex = -1;
		void rebuildFromDescriptor();
		void seedDescriptor(ScreenManager& mgr);

    public:
        SettingsScreen(ScreenManager& mgr);
        ~SettingsScreen();
            // called by the screen when an arrow key navigates
        void commitActiveMenuValue(); 
        void update(uint16_t deltaTimeMS);
        void draw(Display *display);
        int keyPressed(uint8_t key);
        int keyReleased(uint8_t key);
        int keyDown(uint8_t key);
};
