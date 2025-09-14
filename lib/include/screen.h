#pragma once

#include "common.h"
#include "display.h"
#include "widget.h"
#include <vector>
#include <algorithm>
#include <unordered_set>  // std::unordered_set
#include <cstdint>        // uint32_t
#include <unordered_map>
// forward-declare to avoid circular include
/*
class ScreenManager;
enum ScreenEnum { MENUSCREEN, TESTSCREEN, SETTINGSSCREEN, ABOUTSCREEN, BASICSCREEN, SPLASHSCREEN };

class Screen {
public:
    Screen(ScreenManager& m, ScreenEnum id) : mgr(m), screenId(id) {}
    virtual void update(uint16_t deltaTimeMS) = 0;
    virtual void draw(Display *display) = 0;
    virtual int keyPressed(uint8_t key) = 0;
    virtual int keyReleased(uint8_t key) = 0;
    virtual int keyDown(uint8_t key) = 0;
    virtual ~Screen() {}

    ScreenEnum screenId;
    Rect2 refresh;
    std::vector<Widget*> widgets;
    int selectedIndex = -1;
    bool rebuilding = false;

    void addWidget(Widget* w, uint32_t id);
    // convenience wrappers use stored mgr
    void seedState()             { seedState(mgr); }
    void rebuildFromDescriptor() { rebuildFromDescriptor(mgr); }
    // centralizes Menu/Edit/Button → state commits
    void commitWidgetValues();
    
protected:
    ScreenManager& mgr;
    void seedState(ScreenManager& mgr);
    void rebuildFromDescriptor(ScreenManager& mgr);
    void drawWidgets(Display *disp) {
        // First pass: draw all inactive widgets
        for (auto* w : widgets) {
            if (!w->isActive()) {
                w->draw(disp);
            }
        }

        // Second pass: draw all active widgets on top
        for (auto* w : widgets) {
            if (w->isActive()) {
                w->draw(disp);
            }
        }
    }
    void ensureSelection();
};


*/

// screen.h
#pragma once
#include <cstdint>
#include <vector>
#include "common.h"

class Display;
class ScreenManager;
class Widget;

enum class ScreenEnum : uint16_t {
  MENUSCREEN,
  TESTSCREEN,
  ABOUTSCREEN,
  SETTINGSSCREEN,
  BASICSCREEN,
  SPLASHSCREEN,
  // ...
};


class Screen {

    static constexpr uint32_t VS_OVERRIDE_COOLDOWN_MS = 500; // or whatever you chose    
public:
    Screen(ScreenManager& m, ScreenEnum id) : mgr(m), screenId(id) {}
    virtual ~Screen() = default;

    // Option B: not virtual. Always runs base logic, then onUpdate().
    void update(uint16_t deltaTimeMS);

    virtual void draw(Display* display) = 0;

    // focus helpers you already have:
    void addWidget(Widget* widg, uint32_t id);
    void ensureSelection();

    void rebuildFromDescriptor(ScreenManager& mgr);
    void seedState(ScreenManager& mgr);   // you can keep this for now

    inline void seedState()                { seedState(mgr); }
    inline void rebuildFromDescriptor()    { rebuildFromDescriptor(mgr); }
    void commitWidgetValues();

    // input
    virtual int  keyPressed(uint8_t key)   { (void)key; return 0; }
    virtual int  keyReleased(uint8_t key)  { (void)key; return 0; }
    virtual int  keyDown(uint8_t key)      { (void)key; return 0; }

    // public members you already use:
    ScreenEnum screenId;
    Rect2      refresh{0,0,0,0};

protected:
    // Option B hook: derived screens override this instead of update()
    virtual void onUpdate(uint16_t /*deltaTimeMS*/) {}
    void drawWidgets(Display *disp) {
        // First pass: draw all inactive widgets
        for (auto* w : widgets) {
            if (!w->isActive()) {
                w->draw(disp);
            }
        }

        // Second pass: draw all active widgets on top
        for (auto* w : widgets) {
            if (w->isActive()) {
                w->draw(disp);
            }
        }
    }
    ScreenManager&          mgr;
    std::vector<Widget*>    widgets;
    int                     selectedIndex = -1;
    bool                    rebuilding    = false;

private:
    // tiny timer for VS polling in base update (safe to leave even if unused)
    uint16_t vsPollTick_ = 0;
    uint32_t nowMs_      = 0;  // monotonic ms accumulated from update()
    // Per-widget cooldown: widgetId -> ms timestamp until which user overrides external writes
    std::unordered_map<uint16_t, uint32_t> overrideUntil_;
    void syncWidgetsFromVSIfIdle();
};

