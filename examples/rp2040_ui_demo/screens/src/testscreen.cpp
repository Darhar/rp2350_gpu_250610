#include "testscreen.h"

TestScreen::~TestScreen() {
    printf("deleting widgets\n");
    for (Widget* widget : widgets) {
        delete widget;
    }
}

void TestScreen::addWidget(Widget* widget,uint32_t widgetId) {
    printf("TestScreen Adding widget %d\n",widgetId);
    widget->setId(widgetId);
    widgets.push_back(widget);
    if (selectedIndex == -1 && widget->isSelectable()) {
        selectedIndex = widgets.size() - 1;
        widget->setSelected(false);
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
    printf("drawing %d widgets\n",widgets.size());
    //int widgNo=0;
    for (auto* w : widgets){
        //printf("%d\n",widgNo++);
        w->draw(display);  
    }
}

int TestScreen::keyPressed(uint8_t key) {
    if (key == KEY_UP || key == KEY_DOWN) {
        if (widgets.empty()) return 0;
        bool normNavig=true;

        int dir = (key == KEY_DOWN) ? -1 : 1;
        //test if current item is active
        //if active use arrow keys to change value, ok deactivates
        if(widgets[selectedIndex]->getWidgetType() == WidgetType::Edit){
            if(static_cast<Edit*>(widgets[selectedIndex])->getActive()){
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
            }
        }
        refresh=Rect2(0,0,158,64);
    } 
    
    else if (key == KEY_OK && selectedIndex != -1) {
        //if(widgets[selectedIndex]->getWidgetType()==WidgetType::Edit){
        if(static_cast<Edit*>(widgets[selectedIndex])->getActive()){
            printf("[Test] store the value\n");
            int newVal=static_cast<Edit*>(widgets[selectedIndex])->getValue();            
            auto& desc = mgr.getDescriptor(scrEnum);
            for (auto& wd : desc.widgets) {
                if (wd.widgetId == static_cast<Edit*>(widgets[selectedIndex])->getWidgetId()) {
                    printf("[Test] storing %d in %d\n",newVal,static_cast<Edit*>(widgets[selectedIndex])->getWidgetId());
                    wd.initialValue = newVal;
                    break;
                }
            }
            widgets[selectedIndex]->activate(false); 
                      
        }else{
            widgets[selectedIndex]->activate(true);            
        }
        printf("[Test]ok done\n");
        refresh=Rect2(0,0,158,64);
        //}
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

void TestScreen::rebuildFromDescriptor() {
    printf("[test] rebuildFromDescriptor");
	widgets.clear();
	auto& desc = mgr.getDescriptor(scrEnum);
	for (auto& wd : desc.widgets) {
		Widget* w = mgr.createWidgetFromDescriptor(wd);
		widgets.push_back(w);
	}
}