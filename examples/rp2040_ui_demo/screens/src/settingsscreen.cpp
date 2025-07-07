#include "settingsscreen.h"

SettingsScreen::SettingsScreen(ScreenManager& mgr) : mgr(mgr), scrEnum(ScreenEnum::SETTINGSSCREEN){
    printf("[SettingsScreen] loading...\n");

    seedDescriptor(mgr);
    screenId = scrEnum;
    rebuildFromDescriptor();
    funkyV16->setClearSpace(true);
    
    title =  "Settings Screen";
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
    refresh=Rect2(0,0,158,64);
    printf("[SettingsScreen] Done\n");        
}

SettingsScreen::~SettingsScreen() {
    //printf("deleting widgets\n");
    for (Widget* widget : widgets) {
        delete widget;
    }    
}

void SettingsScreen::update(uint16_t deltaTimeMS) {}

void SettingsScreen::draw(Display *disp) {
    printf("[settings] draw\n");
    for (auto* w : widgets){
        w->draw(disp);  
    }    
    //menu1->draw(disp);
}

void SettingsScreen::seedDescriptor(ScreenManager& mgr) {
    printf("[settings] seedDescriptor\n");
    auto& desc = mgr.getDescriptor(scrEnum);
    if (desc.widgets.empty()) {
        desc.widgets.push_back({
            WidgetType::Menu,  // type
            1,                  // widgetId
            "Menu",             // initialText    
            50, 10, 100, 10,    // x, y, w, h
            0                   // selectable
        });
    }
    printf("widgets : %d \n",desc.widgets.size());
}

void SettingsScreen::rebuildFromDescriptor() {
    printf("[settings]rebuild\n");
    widgets.clear();
    auto& desc = mgr.getDescriptor(scrEnum);
    for (auto& wd : desc.widgets) {
        Widget* w = mgr.createWidgetFromDescriptor(wd);
        addWidget(w, wd.widgetId);
    }
}

int SettingsScreen::keyPressed(uint8_t key) {
    if(key == KEY_BACK)
        return encodeKeyReturn(KeyReturn::SCRSELECT, ScreenEnum::MENUSCREEN);

    else if(key == KEY_UP){
        //menu1->changeSelection(1);
        refresh=Rect2(0,0,158,64);
    }
    else if(key == KEY_DOWN){
        //menu1->changeSelection(0);
        refresh=Rect2(0,0,158,64);
    }
    else if(key == KEY_LEFT){
        //menu1->showMenu(false); 
        refresh=Rect2(0,0,158,64);
    }
    else if(key == KEY_OK)
        //menu1->showMenu(true); 
    refresh=Rect2(0,0,158,64);
    return 0;
}

int SettingsScreen::keyReleased(uint8_t key) {
    return 0;
}

int SettingsScreen::keyDown(uint8_t key){
    return 0;
}
