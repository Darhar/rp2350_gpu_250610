#include "screen.h"
#include "ScreenManager.hpp"
//#include <algorithm>//dont know if needed
#include "menu.h"
#include "edit.h"
#include "button.h"

void Screen::addWidget(Widget* widg, uint32_t id) {
    TRACE_CAT(UI,"id:%d",id);

    widg->setId(id);
    if (selectedIndex == -1 && widg->isSelectable()) {
        selectedIndex = widgets.size();
        widg->setSelected(true);
    }else{
        widg->setSelected(false);
    }

    widgets.push_back(widg);

    refresh = Rect2(0,0,158,64);
    TRACE("out");
}

void Screen::ensureSelection() {
    if (selectedIndex >= 0 && selectedIndex < (int)widgets.size()) return;

    for (size_t i = 0; i < widgets.size(); ++i) {
        if (widgets[i]->isSelectable()) {
            selectedIndex = (int)i;
            widgets[i]->setSelected(true);
            return;
        }
    }
    // none selectable
    selectedIndex = -1;
}

void Screen::rebuildFromDescriptor(ScreenManager& mgr) {
    rebuilding = true;
    widgets.clear();
    selectedIndex = -1;

    auto& cfgs = mgr.getConfig(screenId);
    auto& sts  = mgr.getState(screenId);

    for (auto& c : cfgs) {
        auto it = std::find_if(sts.begin(), sts.end(),
            [&](auto& s){ return s.widgetId == c.widgetId; });
        WidgetState* state = (it != sts.end()) ? &*it : nullptr;

        Widget* w = mgr.createWidgetFromConfigAndState(c, state);
        if (w) addWidget(w, c.widgetId);
    }

    // ensure something is selected if possible
    ensureSelection();

    rebuilding = false;
    refresh = Rect2(0,0,158,64); // redraw once, after we’re fully built
}

static inline bool needsState(const WidgetConfig& c) {
    switch (c.type) {
        case WidgetType::Menu:
        case WidgetType::Edit:
        case WidgetType::Button:
            return true;
        default: // Label etc.
            return false;
    }
}

void Screen::seedState(ScreenManager& mgr) {
    auto& cfgs = mgr.getConfig(screenId);
    auto& sts  = mgr.getState(screenId);

    // Build a quick lookup of existing state entries by widgetId
    std::unordered_set<uint32_t> have;
    have.reserve(sts.size());
    for (const auto& s : sts) have.insert(s.widgetId);

    // Add missing state entries; keep existing values as-is
    for (const auto& c : cfgs) {
        if (!needsState(c)) continue;
        if (have.count(c.widgetId)) {
            TRACE_CAT(UI, "seedState: keep id=%u", c.widgetId);
            continue;
        }

        WidgetState s{};
        s.widgetId = c.widgetId;
        switch (c.type) {
            case WidgetType::Menu:   s.data = c.initialSelection; break; // int
            case WidgetType::Edit:   s.data = c.initialValue;     break; // int
            case WidgetType::Button: s.data = bool(c.toggleOn);   break; // bool (not int)
            default:                 s.data = std::monostate{};   break;
        }
        TRACE_CAT(UI, "seedState: add id=%u (defaulted)", c.widgetId);
        sts.push_back(std::move(s));
    }

    // Optional: prune states whose widget no longer exists in cfgs
    sts.erase(std::remove_if(sts.begin(), sts.end(),
        [&](const WidgetState& s){
            return std::none_of(cfgs.begin(), cfgs.end(),
                [&](const WidgetConfig& c){
                    return needsState(c) && c.widgetId == s.widgetId;
                });
        }),
        sts.end());
}

void Screen::commitWidgetValues() {
    auto& sts = mgr.getState(screenId);

    for (auto* w : widgets) {
        const uint32_t id = w->getWidgetId();

        auto it = std::find_if(sts.begin(), sts.end(),
            [&](auto& s){ return s.widgetId == id; });
        if (it == sts.end()) {
            TRACE_CAT(UI, "commit: no state entry for widgetId=%u", id);
            continue;
        }

        switch (w->getWidgetType()) {
          case WidgetType::Menu: {
              const int v = static_cast<Menu*>(w)->getValue();
              it->data = v; // store as int
              TRACE_CAT(UI, "commit: Menu widgetId=%u value=%d", id, v);
              w->setActive(false);
              break;
          }
          case WidgetType::Edit:
              TRACE_CAT(UI,"WidgetType::Edit:%d",static_cast<Edit*>(w)->getValue());
              it->data = static_cast<Edit*>(w)->getValue();
              break;
          case WidgetType::Button:
              TRACE_CAT(UI,"WidgetType::Button");
              it->data = static_cast<Button*>(w)->getState();
              break;
          default: break;
        }
    }
  for (auto& s : sts) {
      if (auto p = std::get_if<int>(&s.data))
          TRACE_CAT(UI, "state: id=%u int=%d", s.widgetId, *p);
      else if (auto q = std::get_if<bool>(&s.data))
          TRACE_CAT(UI, "state: id=%u bool=%d", s.widgetId, *q);
      else
          TRACE_CAT(UI, "state: id=%u <monostate>", s.widgetId);
  }    
}


