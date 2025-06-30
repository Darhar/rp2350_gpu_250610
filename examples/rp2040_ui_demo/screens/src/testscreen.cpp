#include "testscreen.h"

TestScreen::~TestScreen() {
    //printf("deleting widgets\n");
    for (Widget* widget : widgets) {
        delete widget;
    }
}

void TestScreen::addWidget(Widget* widget,uint32_t widgetId) {
    //printf("TestScreen Adding widget %d\n",widgetId);
    widget->setId(widgetId);
    widgets.push_back(widget);
    if (selectedIndex == -1 && widget->isSelectable()) {
        selectedIndex = widgets.size() - 1;
        widget->setSelected(true);
    }
    refresh=Rect2(0,0,158,64);
    printf("TestScreen Added widget,size now %d\n",widgets.size());
}

void TestScreen::update(uint16_t deltaTimeMS) {
    duration += deltaTimeMS;
    accDeltaTimeMS += deltaTimeMS;
    if(accDeltaTimeMS>200){}
}

void TestScreen::draw(Display* display) {
    //printf("drawing %d widgets\n",widgets.size());
    //int widgNo=0;
    for (auto* w : widgets){
        //printf("%d\n",widgNo++);
        w->draw(display);  
    }
}

void TestScreen::buildFromDescriptor() {
    printf("[test] buildFromDescriptor\n");
	widgets.clear();
    selectedIndex = -1;

	auto& desc = mgr.getDescriptor(scrEnum);

	for (auto& wd : desc.widgets) {
		Widget* w = mgr.createWidgetFromDescriptor(wd);
        if (!w) continue;
        addWidget(w, wd.widgetId);
	}
}

void TestScreen::commitActiveEditValue() {
    auto* w = widgets[selectedIndex];
    auto& desc = mgr.getDescriptor(scrEnum);

    if (w->getWidgetType() == WidgetType::Button) {
        Button* btn = static_cast<Button*>(w);
        for (auto& wd : desc.widgets) {
            if (wd.widgetId == btn->getWidgetId()) {
                wd.toggleState = btn->getState();
            }
        }
    }else if (w->getWidgetType() == WidgetType::Edit){
        Edit* edit = static_cast<Edit*>(w);
        if (!edit->isActive()) return;

        int val = edit->getValue();
        for (auto& wd : desc.widgets) {
            if (wd.widgetId == edit->getWidgetId()) {
                wd.initialValue = val;
                break;
            }
        }
        edit->setActive(false);        
    }else return;
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
        commitActiveEditValue();
        refresh = Rect2(0, 0, 158, 64);
    }

    if (w->getWidgetType() == WidgetType::Edit) {
        Edit* edit = static_cast<Edit*>(w);

        if (edit->isActive()) {
            // Commit value to descriptor
            commitActiveEditValue(); 
        } else {
            // Activate it for editing
            edit->setActive(true);
        }

        refresh = Rect2(0, 0, 158, 64);  // mark for redraw
    }
}
  
    
    else if(key == KEY_BACK){
        return encodeKeyReturn(KeyReturn::SCRSELECT, ScreenEnum::MENUSCREEN);
    }
    return 0;
}

int TestScreen::keyReleased(uint8_t key) {
	return 0;
}

int TestScreen::keyDown(uint8_t key){
    return 0;
}
