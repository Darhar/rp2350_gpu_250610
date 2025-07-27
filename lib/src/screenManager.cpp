
#include "screenManager.hpp"
#include <label.h>
#include <button.h>
#include <edit.h>
#include <menu.h>

ScreenManager::~ScreenManager() {
    delete activeScreen;
    TRACE("");
}

void ScreenManager::registerScreen(ScreenEnum id, ScreenFactoryFunc factory) {
    screenObjects[id] = std::move(factory);
    //screenData[id].id = id;      
    ++screenCount;
}

Widget* ScreenManager::createWidgetFromDescriptor(const WidgetDescriptor& wd) {
    TRACE("");
    switch (wd.type) {
        case WidgetType::Label:
            // new Label(text, x, y, width, height)
            return new Label(
                wd.label,
                wd.x, wd.y,
                wd.width, wd.height
            );

        case WidgetType::Button:
            return new Button(
                wd.label, 
                wd.captionOn, 
                wd.captionOff,
                wd.x, wd.y, wd.width, wd.height,
                wd.toggleState);

        case WidgetType::Edit:
            return new Edit(
                wd.label,
                wd.x, wd.y, wd.width, wd.height,
                wd.initialValue,
                wd.minValue, wd.maxValue);

        case WidgetType::Menu:
            return new Menu(
                wd.label,
                wd.menuItems,
                wd.initialSelection,
                wd.x, wd.y,wd.width, wd.height);
        // … handle other widget types …
        default:
            // Unknown widget type—return a nullptr or a placeholder
            return nullptr;
    }
}

Screen* ScreenManager::buildScreenFromDescriptor(ScreenEnum id) {
    TRACE("screen:%d",id);
    auto curFactory = screenObjects.find(id);

    if (curFactory == screenObjects.end()) {
        return nullptr;    // or throw, or fallback
    }
    Screen* screen = curFactory->second();  
    return screen;
}

void ScreenManager::registerFactory(ScreenEnum id, ScreenFactoryFunc func) {
    screenObjects[id] = func;
}

void ScreenManager::setActiveScreen(ScreenEnum id) {
    TRACE("screen : %d",id);
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
    TRACE("");
    if (activeScreen) {
        keyReturn=activeScreen->keyPressed(key);
        KeyReturn cmd=decodeKeyCommand((KeyReturn)keyReturn);
        int retID=decodeKeyID(keyReturn);

        if(cmd==KeyReturn::SCRSELECT){
            setActiveScreen((ScreenEnum)retID);
        }else if(cmd==KeyReturn::SCRBACK){
            setActiveScreen(MENUSCREEN);
        }
        TRACE("return:%d",keyReturn);
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
    currentScreen = static_cast<ScreenEnum>((currentScreen + 1) % screenCount);
    setActiveScreen(currentScreen);
}

void ScreenManager::previousScreen() {
    currentScreen = static_cast<ScreenEnum>((currentScreen + screenCount - 1) % screenCount);
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