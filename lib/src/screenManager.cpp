
#include "screenManager.hpp"
#include <label.h>
#include <button.h>
#include <edit.h>
#include <menu.h>
#include "value_store.h"
#include "sys_status.hpp"

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
    TRACE_CAT(UI,"screen:%d", id);
    auto it = screenObjects.find(id);
    if (it == screenObjects.end()) return nullptr;

    buildingSid_ = static_cast<uint16_t>(id);   // ← set build context
    Screen* screen = it->second();
    buildingSid_ = 0xFFFF;                      // ← clear after build

    TRACE_CAT(UI,"out");
    return screen;
}


void ScreenManager::registerFactory(ScreenEnum id, ScreenFactoryFunc func) {
    screenObjects[id] = func;
}

Widget* ScreenManager::createWidgetFromConfigAndState(
    const WidgetConfig& c, uint16_t sid, WidgetState* /*st unused*/)
{
    TRACE_CAT(UI,"");

    const ValueKey k = VKey(ValueCat::Widget, sid, static_cast<uint16_t>(c.widgetId));
    auto& vs = ValueStore::instance();
    \
    switch (c.type) {
      case WidgetType::Menu: {
        int selected = vs.getInt(k).value_or(c.initialSelection);
        if (!c.menuItems.empty()) {
            if (selected < 0) selected = 0;
            if (selected >= (int)c.menuItems.size()) selected = (int)c.menuItems.size() - 1;
        }
        std::vector<std::string> items; items.reserve(c.menuItems.size());
        for (const auto& s : c.menuItems) items.emplace_back(s);
        return new Menu(std::string(c.label), items, selected, c.x, c.y, c.width, c.height);
      }
      case WidgetType::Edit: {
        int initValue = vs.getInt(k).value_or(c.initialValue);
        TRACE_CAT(UI, "Init %s wid=%u sid=%u -> %s=%d(fromVS=%d)",
                (c.type==WidgetType::Edit?"Edit":c.type==WidgetType::Button?"Button":"Other"),
                c.widgetId, sid, "val",
                initValue /*or toggled/selected*/,
                vs.getInt(k).has_value() /*or getBool*/ );

        if (initValue < c.minValue) initValue = c.minValue;
        if (initValue > c.maxValue) initValue = c.maxValue;
        return new Edit(c.label, c.x, c.y, c.width, c.height, initValue, c.minValue, c.maxValue);
      }
      case WidgetType::Button: {
        bool toggled = vs.getBool(k).value_or(c.toggleOn);
        return new Button(std::string(c.label), std::string(c.captionOn), std::string(c.captionOff),
                          c.x, c.y, c.width, c.height, toggled);
        TRACE_CAT(UI, "Init %s wid=%u sid=%u -> %s=%d(fromVS=%d)",
                (c.type==WidgetType::Edit?"Edit":c.type==WidgetType::Button?"Button":"Other"),
                c.widgetId, sid, "val",
                toggled,
                vs.getInt(k).has_value() /*or getBool*/ );

      }
      case WidgetType::Label:
        return new Label(std::string(c.label), c.x, c.y, c.width, c.height);
      default:
        return nullptr;
    }
}


/*
Widget* ScreenManager::createWidgetFromConfigAndState(
    const WidgetConfig& c, WidgetState* st)
{
    TRACE_CAT(UI,"");    
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
*/


void ScreenManager::setActiveScreen(ScreenEnum id) {
    TRACE_CAT(UI, "");
    delete activeScreen;
    activeScreen = buildScreenFromDescriptor(id);
    TRACE_CAT(UI, "End");
    uint32_t old = ValueStore::instance().getU32(VSIDs::K_ACTIVE_SCREEN).value_or(0xFFFFu);
    (void)ValueStore::instance().setU32(VSIDs::K_ACTIVE_SCREEN, static_cast<uint32_t>(id));
    uint32_t now = ValueStore::instance().getU32(VSIDs::K_ACTIVE_SCREEN).value_or(0xFFFFu);
    printf("[C0] ACTIVE_SCREEN old=%u new=%u\n", (unsigned)old, (unsigned)now);

    // Reflect active screen into VS so Core-1 can see it
    (void)ValueStore::instance().setU32(VSIDs::K_ACTIVE_SCREEN, static_cast<uint32_t>(id));

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
    TRACE_CAT(KEY,"key:%d",key);
    if (activeScreen) {
        keyReturn=activeScreen->keyPressed(key);
        KeyReturn cmd=decodeKeyCommand((KeyReturn)keyReturn);
        int retID=decodeKeyID(keyReturn);

        if(cmd==KeyReturn::SCRSELECT){
            setActiveScreen((ScreenEnum)retID);
        }else if(cmd==KeyReturn::SCRBACK){
            setActiveScreen(ScreenEnum::MENUSCREEN);
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
    if (screenCount == 0) return;
    using U = std::underlying_type_t<ScreenEnum>;
    U idx = static_cast<U>(currentScreen);
    idx = static_cast<U>((idx + 1) % static_cast<U>(screenCount));
    currentScreen = static_cast<ScreenEnum>(idx);
    setActiveScreen(currentScreen);
}

void ScreenManager::previousScreen() {
    if (screenCount == 0) return;
    using U = std::underlying_type_t<ScreenEnum>;
    U idx = static_cast<U>(currentScreen);
    idx = static_cast<U>((idx + static_cast<U>(screenCount) - 1) % static_cast<U>(screenCount));
    currentScreen = static_cast<ScreenEnum>(idx);
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

static inline ValueKey widgetKey(ValueCat cat, uint16_t sid, uint32_t wid) {
    return VKey(cat, sid, static_cast<uint16_t>(wid));
}

void ScreenManager::declareWidgetKeysFor(ScreenEnum id, bool ensureConfig) {
    auto& vs = ValueStore::instance();
    if (vs.frozen()) {
        // Too late to declare new keys; just return quietly.
        return;
    }

    // If the config for this screen hasn't been seeded yet, optionally build
    // a temporary instance to let its ctor run seedConfig().
    bool builtTemp = false;
    {
        auto& cfg = getConfig(id);
        if (ensureConfig && cfg.empty()) {
            if (auto it = screenObjects.find(id); it != screenObjects.end()) {
                Screen* tmp = it->second(); // ctor will call seedConfig()/etc.
                delete tmp;                  // we only needed its config seeded
                builtTemp = true;
            }
        }
    }

    // Now declare keys from the (possibly newly) seeded config.
    const uint16_t sid = static_cast<uint16_t>(id);
    for (const auto& c : getConfig(id)) {
        switch (c.type) {
            case WidgetType::Button: {
                vs.declareBool(widgetKey(ValueCat::Widget, sid, c.widgetId),
                               /*default*/ c.toggleOn);
            } break;
            case WidgetType::Edit: {
                vs.declareInt(widgetKey(ValueCat::Widget, sid, c.widgetId),
                              /*default*/ c.initialValue);
            } break;
            case WidgetType::Menu: {
                vs.declareInt(widgetKey(ValueCat::Widget, sid, c.widgetId),
                              /*default*/ c.initialSelection);
            } break;
            default: /* Label etc. */ break;
        }
    }
}

void ScreenManager::declareWidgetKeysForAll(bool ensureConfig) {
    for (const auto& kv : screenObjects) {
        declareWidgetKeysFor(kv.first, ensureConfig);
    }
}

void ScreenManager::declareWidgetKeysForActive() {
    if (activeScreen) {
        declareWidgetKeysFor(activeScreen->screenId, /*ensureConfig=*/false);
    }
}