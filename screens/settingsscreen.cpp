#include "settingsscreen.h"

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

SettingsScreen::SettingsScreen() {
    printf("[SettingsScreen] loading...\n");
    screenId = ScreenEnum::SETTINGSSCREEN;
    //returnCallBack = rcb;
    option = option;
    title =  "Splash Screen";
    menu1=new Menu(menuA,40);
    refresh=Rect2(0,0,158,64);

    printf("[SettingsScreen] Done\n");
}

SettingsScreen::~SettingsScreen() {}

void SettingsScreen::update(uint16_t deltaTimeMS) {

}

void SettingsScreen::draw(Display *disp) {
    menu1->draw(disp);
}

int SettingsScreen::keyPressed(uint8_t key) {
    if(key == KEY_BACK)
        return encodeKeyReturn(KeyReturn::SCRSELECT, ScreenEnum::MENUSCREEN);

    else if(key == KEY_UP){
        menu1->changeSelection(1);
        refresh=Rect2(0,0,158,64);
    }
    else if(key == KEY_DOWN){
        menu1->changeSelection(0);
        refresh=Rect2(0,0,158,64);
    }
    else if(key == KEY_LEFT){
        menu1->showMenu(false); 
        refresh=Rect2(0,0,158,64);
    }

    else if(key == KEY_OK)
        menu1->showMenu(true); 
        refresh=Rect2(0,0,158,64);
       
        return 0;
    }

int SettingsScreen::keyReleased(uint8_t key) {
    return 0;
}

int SettingsScreen::keyDown(uint8_t key){
    return 0;
}
