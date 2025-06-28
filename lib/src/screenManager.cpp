
#include "screenManager.hpp"
#include <label.h>
#include <button.h>
#include <edit.h>

ScreenManager::~ScreenManager() {
    delete activeScreen;
}

void ScreenManager::registerScreen(ScreenEnum id, ScreenFactoryFunc factory) {
    screenObjects[id] = std::move(factory);
    // Also init a blank descriptor if you like:
    if (!screenData.count(id)) {
        screenData[id] = ScreenDescriptor{ id, {} };
    }
}

Widget* ScreenManager::createWidgetFromDescriptor(const WidgetDescriptor& wd) {
    printf("createWidgetFromDescriptor\n");
    switch (wd.type) {
        case WidgetType::Label:
            // new Label(text, x, y, width, height)
            return new Label(
                wd.initialText,
                wd.x, wd.y,
                wd.width, wd.height
            );

        case WidgetType::Button:
            // For a button you might want to store the “action” text
            return new Button(
                wd.initialText,   // button caption
                wd.x, wd.y,
                wd.width, wd.height,
                wd.initialText    // or wd.actionText if you have one
            );

        case WidgetType::Edit:
            // For an edit field you might also persist min/max or cursor pos
            return new Edit(
                wd.initialText,   // initial contents
                wd.x, wd.y,
                wd.width, wd.height,
                /*value_=*/wd.initialValue,// initial value
                /*min=*/0,
                /*max=*/100
            );

        // … handle other widget types …

        default:
            // Unknown widget type—return a nullptr or a placeholder
            return nullptr;
    }
}

Screen* ScreenManager::buildScreenFromDescriptor(ScreenEnum id) {
    printf("buildScreenFromDescriptor :%d\n",id);
    auto curFactory = screenObjects.find(id);

    if (curFactory == screenObjects.end()) {
        return nullptr;    // or throw, or fallback
    }

    Screen* screen = curFactory->second();  
    ScreenDescriptor& desc = screenData.at(id);

    for (auto& wd : desc.widgets) {
        Widget* w = createWidgetFromDescriptor(wd);
        screen->addWidget(w, wd.widgetId);
    }

    // 4) (Optionally) restore any screen-level state:
    //    e.g. screen->setSelected(desc.selectedIndex);

    return screen;
}

void ScreenManager::registerFactory(ScreenEnum id, ScreenFactoryFunc func) {
    screenObjects[id] = func;
}

void ScreenManager::setActiveScreen(ScreenEnum id) {
    printf("Setting Activ screen %d\n",id);
    delete activeScreen;
    activeScreen = buildScreenFromDescriptor(id);

}

ScreenDescriptor& ScreenManager::getDescriptor(ScreenEnum id){
    return screenData[id];
}

void ScreenManager::update(uint16_t deltaTimeMS) {
    if (activeScreen) {
        activeScreen->update(deltaTimeMS);
    }
}

void ScreenManager::draw(Display* display) {
    if (activeScreen) {
        activeScreen->draw(display);
    }
}

void ScreenManager::keyPressed(uint8_t key) {
    if (activeScreen) {
        keyReturn=activeScreen->keyPressed(key);

        KeyReturn cmd=decodeKeyCommand((KeyReturn)keyReturn);
        int retID=decodeKeyID(keyReturn);

        if(cmd==KeyReturn::SCRSELECT){
            setActiveScreen((ScreenEnum)retID);
        }else if(cmd==KeyReturn::SCRBACK){
            setActiveScreen(MENUSCREEN);
        }
        printf("Key return:%d\n",keyReturn);
    }
}

void ScreenManager::keyDown(uint8_t key){
    if (activeScreen) {
        keyReturn=activeScreen->keyDown(key);
    }    
}
void ScreenManager::keyReleased(uint8_t key){
    if (activeScreen) {
        keyReturn=activeScreen->keyReleased(key);
    }    
}

Screen* ScreenManager::getActiveScreen(){
    return activeScreen;
}

void ScreenManager::nextScreen() {
    currentScreen = static_cast<ScreenEnum>((currentScreen + 1) % SCREEN_COUNT);
    setActiveScreen(currentScreen);
}

void ScreenManager::previousScreen() {
    currentScreen = static_cast<ScreenEnum>((currentScreen + SCREEN_COUNT - 1) % SCREEN_COUNT);
    setActiveScreen(currentScreen);
}

void ScreenManager::setRefreshRect(Rect2 refRect){
    refresh=refRect;
    activeScreen->refresh=refresh;
}
  
void ScreenManager::disableRefresh(){
    refresh.w=0;
}

bool ScreenManager::needRefresh(){
    bool needRef=false;
    refresh=activeScreen->refresh;
    if(refresh.w!=0){
        needRef=true;
    }
    return needRef;
}