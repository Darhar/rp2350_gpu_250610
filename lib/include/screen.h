#pragma once

#include "common.h"
#include "display.h"
#include "widget.h"
#include <vector>
#include <algorithm>
#include <unordered_set>  // std::unordered_set
#include <cstdint>        // uint32_t
// forward-declare to avoid circular include
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
