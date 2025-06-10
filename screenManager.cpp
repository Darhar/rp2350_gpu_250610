
#include "screenManager.hpp"

ScreenManager::~ScreenManager() {
    delete activeScreen;
}

void ScreenManager::registerFactory(ScreenEnum id, ScreenFactoryFunc func) {
    factories[id] = func;
}

void ScreenManager::setActiveScreen(ScreenEnum id) {
    printf("Setting %d\n",id);
    auto it = factories.find(id);
    if (it != factories.end()) {
        delete activeScreen;
        activeScreen = it->second();  // Create new screen with no arguments
        refresh=Rect2(0,0,DISPLAY_WIDTH,DISPLAY_HEIGHT);

    }
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