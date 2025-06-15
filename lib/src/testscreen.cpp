#include "testscreen.h"

TestScreen::TestScreen() {
    screenId = ScreenEnum::TESTSCREEN;
    title =  "Test Screen";
    //returnCallBack = rcb;
    duration=0; 
    int textWid=term6x9->getTextWidth("Click Me");
    Widget* label = new Label("First Label", 5, 15, 20, 10);
    Widget* button = new Button("Button1", 5, 28, 100, 10,"Click");
    //Widget* button1 = new Button("b2", 5, 42, 80, 10,"OK");
    Widget* edit1 = new Edit("Edit1", 5, 42, 100, 10,50,0,100);
    widgets = { label, button, edit1 };// Store in vector of Widget pointers
    funkyV16->setClearSpace(true);
    refresh=Rect2(0,0,158,64);

    printf("[test] Started\n");
}

TestScreen::~TestScreen() {
    printf("deleting widgets\n");
    for (Widget* widget : widgets) {
        delete widget;
    }
    
}

void TestScreen::addWidget(Widget* widget) {
    widgets.push_back(widget);
    if (selectedIndex == -1 && widget->isSelectable()) {
        selectedIndex = widgets.size() - 1;
        widget->setSelected(true);
    }
    refresh=Rect2(0,0,158,64);

}

void TestScreen::update(uint16_t deltaTimeMS) {

    duration += deltaTimeMS;
    accDeltaTimeMS += deltaTimeMS;
  
    if(accDeltaTimeMS>200){}

}

void TestScreen::draw(Display *display) {

    funkyV16->drawText(display, title, Vec2((DISPLAY_WIDTH-(funkyV16->getTextWidth(title)))/2, 0), 255, 1);
    //ariel5x8
    //term6x9

    // Draw all widgets
    for (const Widget* widget : widgets) {
        widget->draw(display);
    }
}

int TestScreen::keyPressed(uint8_t key) {

    if (key == KEY_UP || key == KEY_DOWN) {

        if (widgets.empty()) return 0;
        bool normNavig=true;

        int dir = (key == KEY_DOWN) ? 1 : -1;
        //test if current item is active
        //if active use arrow keys to change value, ok deactivates
        //if(dynamic_cast<Edit*>(widgets[selectedIndex])->getActive()){
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
        printf("[TestScreen]Activate\n");
        //if(widgets[selectedIndex]->getWidgetType()==WidgetType::Edit){
            widgets[selectedIndex]->activate();
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
