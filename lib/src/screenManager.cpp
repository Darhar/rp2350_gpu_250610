
#include "screenManager.hpp"
#include <label.h>
#include <button.h>
#include <edit.h>
#include <menu.h>

inline std::map<ScreenEnum, ScreenFactoryFunc> screenObjects;

ScreenManager::ScreenManager() {
    TRACE("");
}

ScreenManager::~ScreenManager() {
    delete activeScreen;
}

void ScreenManager::registerScreen(ScreenEnum id, ScreenFactoryFunc factory) {
    screenObjects[id] = std::move(factory);     
    ++screenCount;
}

Screen* ScreenManager::buildScreenFromDescriptor(ScreenEnum id) {
    TRACE_CAT(UI,"screen:%d",id);
    auto curFactory = screenObjects.find(id);

    if (curFactory == screenObjects.end()) {
        return nullptr;    // or throw, or fallback
    }
    Screen* screen = curFactory->second();  
    TRACE_CAT(UI,"out");

    return screen;
}

void ScreenManager::registerFactory(ScreenEnum id, ScreenFactoryFunc func) {
    screenObjects[id] = func;
}

Widget* ScreenManager::createWidgetFromConfigAndState(
    const WidgetConfig& c, WidgetState* st)
{
    switch (c.type) {
        case WidgetType::Menu: {
            // Default to config’s initialSelection
            int selected = c.initialSelection;

            if (st) {
                if (auto p = std::get_if<int>(&st->data)) {
                    selected = *p;
                } else {
                    // Type mismatch (e.g., bool/monostate) — don’t crash, just log
                    TRACE_CAT(UI,
                        "Menu state type mismatch: widgetId=%u stateIndex=%zu (expected int). Falling back to initial=%d",
                        c.widgetId, st->data.index(), c.initialSelection);
                }
            } else {
                TRACE_CAT(UI,
                    "Menu has no saved state (widgetId=%u). Using initial=%d",
                    c.widgetId, c.initialSelection);
            }

            TRACE_CAT(UI,
                "Menu ctor: widgetId=%u using selected=%d",
                c.widgetId, selected);

            // Build items vector from config
            std::vector<std::string> items;
            items.reserve(c.menuItems.size());
            for (const auto& s : c.menuItems) items.emplace_back(s);

            return new Menu(
                std::string(c.label),
                items,
                selected,
                c.x, c.y, c.width, c.height
            );
        }
        case WidgetType::Edit: {
            int initValue = st
            ? std::get<int>(st->data)
            : c.initialValue;

            return new Edit(
                c.label,
                c.x, c.y, c.width, c.height,
                initValue,
                c.minValue, c.maxValue
            );
        }
        case WidgetType::Button: {
            bool toggled = c.toggleOn;
            if (st) {
                if (auto pb = std::get_if<bool>(&st->data)) toggled = *pb;
                else TRACE_CAT(UI, "Button state type mismatch id=%u idx=%zu; using default=%d",
                            c.widgetId, st->data.index(), (int)c.toggleOn);
            }

            // adapt parameter order to your Button ctor
            return new Button(
                std::string(c.label),
                std::string(c.captionOn),
                std::string(c.captionOff),
                c.x, c.y, c.width, c.height,
                toggled
            );
        }

        case WidgetType::Label: {
            return new Label(
                std::string(c.label), 
                c.x, c.y, c.width, c.height
            );
        }        
        default:
        return nullptr;
    }
}

void ScreenManager::setActiveScreen(ScreenEnum id) {
    TRACE_CAT(UI, "");
    delete activeScreen;
    activeScreen = buildScreenFromDescriptor(id);
    TRACE_CAT(UI, "End");

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
    TRACE_CAT(KEY,"");
    if (activeScreen) {
        keyReturn=activeScreen->keyPressed(key);
        KeyReturn cmd=decodeKeyCommand((KeyReturn)keyReturn);
        int retID=decodeKeyID(keyReturn);

        if(cmd==KeyReturn::SCRSELECT){
            setActiveScreen((ScreenEnum)retID);
        }else if(cmd==KeyReturn::SCRBACK){
            setActiveScreen(MENUSCREEN);
        }
        TRACE_CAT(KEY,"return:%d",keyReturn);
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