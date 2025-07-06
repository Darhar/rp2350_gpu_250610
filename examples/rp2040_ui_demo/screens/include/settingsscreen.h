#pragma once

#include <common.h>
#include <screen.h>
#include <keyboard.h>
#include <menu.h>
#include <vector>


class SettingsScreen : public Screen
{
    private:
        uint8_t option; 
        std::string title =  "Splash Screen";   
        Menu *menu1;

        uint16_t duration;
        uint16_t accDeltaTimeMS;

    public:
        SettingsScreen(ScreenManager& mgr) : mgr(mgr){
            printf("[SettingsScreen] loading...\n");
            screenId = ScreenEnum::SETTINGSSCREEN;
            //returnCallBack = rcb;
            option = option;
            title =  "Splash Screen";
            std::vector<std::string> menuA
            {
                "Item A" ,
                "Item B" ,
                "Item C" ,
                "Item D" ,
                "Item E" ,
                "Item f", 
                "Item g",
                "Item h",
                "Item i"
            };              
            menu1=new Menu(menuA,40);
            refresh=Rect2(0,0,158,64);

            printf("[SettingsScreen] Done\n");        
        }
        ~SettingsScreen();
        //void addWidget(Widget* widget,uint32_t widgetId);
        void printMenu(Display *display,const std::vector<std::string> &menu);
        void update(uint16_t deltaTimeMS);
        void draw(Display *display);
        int keyPressed(uint8_t key);
        int keyReleased(uint8_t key);
        int keyDown(uint8_t key);

    private:
		ScreenManager& mgr;
        int selectedIndex = -1;
		void buildFromDescriptor();
};
