#include "testscreen.h"
#include "display.h"
#include "edit.h"
#include "button.h"
#include "menu.h"
#include "debug.h"

void TestScreen::onUpdate(uint16_t deltaTimeMS) {
    duration += deltaTimeMS;
    accDeltaTimeMS += deltaTimeMS;
    if (accDeltaTimeMS > 200) {
        // your per-screen logic
        accDeltaTimeMS = 0;
    }
}


TestScreen::TestScreen(ScreenManager& mgr)
: Screen(mgr, ScreenEnum::TESTSCREEN)
{
    TRACE("");
    seedConfig();
    // If you still use legacy state seeding, keep this; otherwise remove.
    // seedState(mgr);
    rebuildFromDescriptor(mgr);   // or rebuildFromDescriptor(); if you added the no-arg forwarder
    title = "Test Screen";
    duration = 0;
    // funkyV16->setClearSpace(true);  // if you still need this line
    refresh = Rect2(0,0,158,64);
}

TestScreen::~TestScreen() {
    for (Widget* widget : widgets) {
        delete widget;
    }
}


void TestScreen::draw(Display* disp) {


int typeVal = (selectedIndex >= 0 && selectedIndex < (int)widgets.size())
                ? static_cast<int>(widgets[selectedIndex]->getWidgetType())
                : -1;

TRACE_CAT(UI, "selectedIndex:%d, type:%d", selectedIndex, typeVal);

    disp->setInverted(false);
    drawWidgets(disp);
}

void TestScreen::seedConfig() {
    auto& cfg = mgr.getConfig(screenId);
    cfg.clear();
    cfg.push_back( 
        WidgetConfig{
            WidgetType::Label,  // type
            1,                  // widgetId
            "First Label",      // initialText    
            0, 1, 100, 10,       // x, y, w, h
            0                   // selectable
        } 
    );
    cfg.push_back( 
        WidgetConfig{
            WidgetType::Edit,
            2,
            "Count",
            0, 12, 80, 12,
            true,
            55, 0, 100,  // initialValue, minValue, maxValue
            false, "", "" // irrelevant Button fields        
        } 
    );  
    cfg.push_back( 
        WidgetConfig{
            WidgetType::Button,
            3,
            "Direct",      // label
            0, 26, 80, 12,
            true,        // selectable
            0, 0, 0,     // irrelevant Edit fields
            true,        // toggleState
            "Up", "Down"           
        } 
    );        
}

int TestScreen::keyPressed(uint8_t key) {
    if (key == KEY_UP || key == KEY_DOWN) {
        if (widgets.empty()) return 0;
        bool normNavig=true;

        int dir = (key == KEY_DOWN) ? -1 : 1;
        //test if current item is active
        //if active use arrow keys to change value, ok deactivates
        if(widgets[selectedIndex]->getWidgetType() == WidgetType::Edit){
            if(static_cast<Edit*>(widgets[selectedIndex])->isActive()){
                normNavig=false;
                int editVal=static_cast<Edit*>(widgets[selectedIndex])->getValue();
                editVal=editVal+dir;
                static_cast<Edit*>(widgets[selectedIndex])->setValue(editVal);
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

    else if (key == KEY_OK && selectedIndex != -1) {
        Widget* w = widgets[selectedIndex];
        if (w->getWidgetType() == WidgetType::Button) {
            Button* btn = static_cast<Button*>(w);
            btn->toggle();
            commitWidgetValues();
            refresh = Rect2(0, 0, 158, 64);
        }

        if (w->getWidgetType() == WidgetType::Edit) {
            Edit* edit = static_cast<Edit*>(w);
            if (edit->isActive()) {
                // Commit value to descriptor
                commitWidgetValues(); 
                edit->setActive(false);
            } else {
                // Activate it for editing
                edit->setActive(true);
            }
            refresh = Rect2(0, 0, 158, 64);  // mark for redraw
        }
    }
  
    else if(key == KEY_BACK){
        return encodeKeyReturn(KeyReturn::SCRSELECT, static_cast<uint8_t>(ScreenEnum::MENUSCREEN));
    }
    return 0;
}

int TestScreen::keyReleased(uint8_t key) {
	return 0;
}

int TestScreen::keyDown(uint8_t key){
    return 0;
}
