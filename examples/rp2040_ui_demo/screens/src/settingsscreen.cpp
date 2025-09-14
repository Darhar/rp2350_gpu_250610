#include "settingsscreen.h"

std::vector<std::string> menuA
{
    "Item A" ,
    "Item B" ,
    "Item C" ,
    "Item D" ,
    "Item i"
}; 

SettingsScreen::SettingsScreen(ScreenManager& mgr) : Screen(mgr, ScreenEnum::SETTINGSSCREEN){
    TRACE("");
    seedConfig();
    //seedState();
    rebuildFromDescriptor();
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

void SettingsScreen::onUpdate(uint16_t deltaTimeMS) {}

void SettingsScreen::draw(Display *disp) {
    TRACE_CAT(UI,"selectedIndex:%d, type:%d",selectedIndex,widgets[selectedIndex]->getWidgetType());
    disp->setInverted(false);
    drawWidgets(disp);
}

void SettingsScreen::seedConfig() {
    auto& cfg = mgr.getConfig(screenId);
    cfg.clear();
    cfg.push_back( 
        WidgetConfig{
            WidgetType::Menu,
            1,                     // widgetId
            "Choice",              // label
            2,0,100,12,            // row,col,w,h
            true,                  // selectable
            0,0,0,                 // initialValue,min,max (unused for Menu)
            false,"","",           // toggleOn,captionOn,captionOff
            menuA,                 // now a std::vector<std::string>
            0                      // initialSelection
        } 
    );
}

int SettingsScreen::keyPressed(uint8_t key) {
    TRACE_CAT(KEY,"selectedIndex:%d, type:%d",selectedIndex,widgets[selectedIndex]->getWidgetType());

    if(key == KEY_BACK)
        return encodeKeyReturn(KeyReturn::SCRSELECT, static_cast<uint8_t>(ScreenEnum::MENUSCREEN));

    else if (key == KEY_UP || key == KEY_DOWN) {
        if (widgets.empty()) return 0;
        bool normNavig=true;

        int dir = (key == KEY_DOWN) ? -1 : 1;
        TRACE_CAT(KEY,"KEY_UP || KEY_DOWN dir: %d",dir);

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
                commitWidgetValues(); //will set menu inactive
            } else {
                TRACE_CAT(KEY,"not Active ");
                // Activate it for selecting
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
