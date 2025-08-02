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
    TRACE("");
    seedDescriptor(mgr);
    screenId = scrEnum;
    rebuildFromDescriptor(mgr);
    funkyV16->setClearSpace(true);
    title =  "Settings Screen";
    refresh=Rect2(0,0,158,64);
    selectedIndex =0;     
}

SettingsScreen::~SettingsScreen() {
    for (Widget* widget : widgets) {
        delete widget;
    }    
}

void SettingsScreen::update(uint16_t deltaTimeMS) {}

void SettingsScreen::draw(Display *disp) {
    TRACE_CAT(UI,"selectedIndex:%d, type:%d",selectedIndex,widgets[selectedIndex]->getWidgetType());
    disp->setInverted(false);
    drawWidgets(disp);
}

void SettingsScreen::seedDescriptor(ScreenManager& mgr) {
    TRACE_CAT(UI,"");
    auto& desc = mgr.getDescriptor(ScreenEnum::SETTINGSSCREEN);
    if (desc.widgets.empty()) {
        desc.widgets.push_back({
            WidgetType::Menu,        // new MENU type
            /*widgetId=*/ 1,
            /*label=*/"Choice",
            2, 0, 100, 12,           // one row of 12px height
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
        desc.widgets.push_back({
            WidgetType::Button,
            0,
            "WiFi",      // label
            2, 12, 100, 12,
            true,        // selectable
            0, 0, 0,     // irrelevant Edit fields
            true,        // toggleState
            "ON", "OFF"
        });        

    }
}
void SettingsScreen::commitActiveMenuValue() {
    TRACE_CAT(UI,"");

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

int SettingsScreen::keyPressed(uint8_t key) {
    TRACE_CAT(KEY,"selectedIndex:%d, type:%d",selectedIndex,widgets[selectedIndex]->getWidgetType());

    if(key == KEY_BACK)
        return encodeKeyReturn(KeyReturn::SCRSELECT, ScreenEnum::MENUSCREEN);

    else if (key == KEY_UP || key == KEY_DOWN) {
        if (widgets.empty()) return 0;
        bool normNavig=true;

        int dir = (key == KEY_DOWN) ? -1 : 1;
        TRACE_CAT(KEY,"KEY_UP || KEY_DOWN dir: %d",dir);
        //test if current item is active

        if(widgets[selectedIndex]->getWidgetType() == WidgetType::Menu){
            TRACE_CAT(KEY,"is Menu");
            Menu* menu = static_cast<Menu*>(widgets[selectedIndex]);

            if(menu->isActive()){
                TRACE_CAT(KEY,"key up/dwn Menu active");
                normNavig=false;
                menu->changeMenuSelection(dir);
            }
        }            
        if(normNavig){
            TRACE_CAT(KEY,"normNavig sel Indx:%d",selectedIndex);

            int original = selectedIndex;
            do {
                selectedIndex = (selectedIndex + dir + widgets.size()) % widgets.size();
            } while (!widgets[selectedIndex]->isSelectable() && selectedIndex != original);

            for (size_t i = 0; i < widgets.size(); ++i) {//need to change widget selected field
                widgets[i]->setSelected(i == selectedIndex);

                TRACE_CAT(KEY,"widget %d is a %d",i, widgets[i]->getWidgetType());
                if(i == selectedIndex) TRACE_CAT(KEY," and selected"); else TRACE_CAT(KEY,"");
            }
        }
        refresh=Rect2(0,0,158,64);
    } 
    else if(key == KEY_LEFT){ 
        refresh=Rect2(0,0,158,64);
    }
    else if(key == KEY_OK){
        TRACE_CAT(KEY,"key OK");

        if(widgets[selectedIndex]->getWidgetType() == WidgetType::Menu){

            Menu* menu = static_cast<Menu*>(widgets[selectedIndex]);
            //Could we do the rest of this in the class
            if (menu->isActive()) {
                // Commit value to descriptor
                commitActiveMenuValue(); //will set menu inactive
            } else {
                TRACE_CAT(KEY,"not Active ");
                // Activate it for selecting
                //widgetToTop(selectedIndex);
                menu->setActive(true);
            }
        } 
        TRACE_CAT(KEY,"selectedIndex:%d, type:%d",selectedIndex,widgets[selectedIndex]->getWidgetType());
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
