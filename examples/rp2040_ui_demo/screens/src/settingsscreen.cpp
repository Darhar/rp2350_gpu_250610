#include "settingsscreen.h"
std::vector<std::string> menuA
{
    "Item A" ,
    "Item B" ,
    "Item C" ,
    "Item D" ,
    "Item i"
}; 

SettingsScreen::SettingsScreen(ScreenManager& mgr) : mgr(mgr), scrEnum(ScreenEnum::SETTINGSSCREEN){
    printf("[SettingsScreen] loading...\n");

    seedDescriptor(mgr);
    screenId = scrEnum;
    rebuildFromDescriptor();
    funkyV16->setClearSpace(true);
    title =  "Settings Screen";
    refresh=Rect2(0,0,158,64);
    selectedIndex =0;

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
}

void SettingsScreen::seedDescriptor(ScreenManager& mgr) {
    auto& desc = mgr.getDescriptor(ScreenEnum::SETTINGSSCREEN);
    if (desc.widgets.empty()) {
        desc.widgets.push_back({
            WidgetType::Button,
            0,
            "WiFi",      // label
            10, 20, 80, 12,
            true,        // selectable
            0, 0, 0,     // irrelevant Edit fields
            true,        // toggleState
            "ON", "OFF"
        });        
        desc.widgets.push_back({
            WidgetType::Menu,        // new MENU type
            /*widgetId=*/ 1,
            /*label=*/"Choice",
            0, 0, 100, 12,           // one row of 12px height
            /*selectable =*/ true,
            /*initialValue =*/ 0,
            /*minValue=*/ 0,
            /*maxValue= */ 0,
            /*toggleState =*/ false,
            /*captionOn=*/ "",
            /*captionOff=*/ "",
            /*menuItems=*/ menuA, 
            /*initialSelection=*/ 0
        });
    }
    printf("widgets : %d \n",desc.widgets.size());
}
void SettingsScreen::commitActiveMenuValue() {
    auto* w = widgets[selectedIndex];
    auto& desc = mgr.getDescriptor(scrEnum);

    if (w->getWidgetType() == WidgetType::Button) {
        Button* btn = static_cast<Button*>(w);
        for (auto& wd : desc.widgets) {
            if (wd.widgetId == btn->getWidgetId()) {
                wd.toggleState = btn->getState();
            }
        }
    }else if (w->getWidgetType() == WidgetType::Menu){
        Menu* menu = static_cast<Menu*>(w);
        if (!menu->isActive()) return;

        int val = menu->getValue();

        for (auto& wd : desc.widgets) {
            if (wd.widgetId == menu->getWidgetId()) {
                wd.initialSelection = val;              
                break;
            }
        }
       
        menu->setActive(false);   

    }else return;
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

    else if (key == KEY_UP || key == KEY_DOWN) {
        if (widgets.empty()) return 0;
        bool normNavig=true;

        int dir = (key == KEY_DOWN) ? -1 : 1;
        printf("[sm]keypressed,dir: %d, selectedItem:%d, type:%d\n",dir,selectedIndex,widgets[selectedIndex]->getWidgetType());
        //test if current item is active
        if(widgets[selectedIndex]->getWidgetType() == WidgetType::Menu){
            Menu* menu = static_cast<Menu*>(widgets[selectedIndex]);

            printf("[sm] widget Menu\n");
            if(menu->isActive()){
                normNavig=false;
                menu->changeMenuSelection(dir);
            }
        }            
        if(normNavig){
            int original = selectedIndex;
            do {
                selectedIndex = (selectedIndex + dir + widgets.size()) % widgets.size();
            } while (!widgets[selectedIndex]->isSelectable() && selectedIndex != original);

            for (size_t i = 0; i < widgets.size(); ++i) {
                widgets[i]->setSelected(i == selectedIndex);
                printf("widget %d is a %d",i, widgets[i]->getWidgetType());
                if(i == selectedIndex){printf(" and selected\n");}else{printf("\n");}
            }
        }
        refresh=Rect2(0,0,158,64);
    } 
    else if(key == KEY_LEFT){ 
        refresh=Rect2(0,0,158,64);
    }
    else if(key == KEY_OK){
        if(widgets[selectedIndex]->getWidgetType() == WidgetType::Menu){

            Menu* menu = static_cast<Menu*>(widgets[selectedIndex]);
            //Could we do the rest of this in the class
            if (menu->isActive()) {
                // Commit value to descriptor
                commitActiveMenuValue(); //will set menu inactive
            } else {
                // Activate it for editing
                menu->setActive(true);
            }
            //menu->toggleMenu();
        }        
    }

    refresh=Rect2(0,0,158,64);
    return 0;
}

int SettingsScreen::keyReleased(uint8_t key) {
    return 0;
}

int SettingsScreen::keyDown(uint8_t key){
    return 0;
}
