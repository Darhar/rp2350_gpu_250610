#include "screen.h"
#include "ScreenManager.hpp"
//#include <algorithm>//dont know if needed
#include "menu.h"
#include "edit.h"
#include "button.h"
#include "value_store.h"
#include "sys_status.hpp"

#include <climits>   // for INT_MIN
// --- OPTIONAL knobs for the base update’s VS→widget sync ---
#ifndef VS_WIDGET_POLL_ENABLE
    #define VS_WIDGET_POLL_ENABLE 1    // set to 0 to disable
#endif

#ifndef VS_WIDGET_POLL_MS
    #define VS_WIDGET_POLL_MS 100      // poll ~10 Hz
#endif

#if !defined(VS_WIDGET_POLL_LOG)
    #define VS_WIDGET_POLL_LOG 1
#endif

#ifndef VS_OVERRIDE_COOLDOWN_MS
    #define VS_OVERRIDE_COOLDOWN_MS 600   // user override window after commit
#endif

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

    const uint16_t sid = static_cast<uint16_t>(screenId);

    for (auto& c : cfgs) {
        auto it = std::find_if(sts.begin(), sts.end(),
            [&](auto& s){ return s.widgetId == c.widgetId; });
        WidgetState* state = (it != sts.end()) ? &*it : nullptr;

        // PASS sid EXPLICITLY
        Widget* w = mgr.createWidgetFromConfigAndState(c, sid, state);
        if (w) addWidget(w, c.widgetId);
    }

    ensureSelection();
    rebuilding = false;
    refresh = Rect2(0,0,158,64);
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
    TRACE_CAT(UI, "");

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


#include <climits>  // for INT_MIN

void Screen::commitWidgetValues() {

    printf("commitWidgetValues\n");
    auto& vs = ValueStore::instance();
    const uint16_t sid = static_cast<uint16_t>(screenId);

    bool touched = false;  // track if anything changed

    for (auto* w : widgets) {
        const uint16_t wid = static_cast<uint16_t>(w->getWidgetId());
        const ValueKey k   = VKey(ValueCat::Widget, sid, wid);

        switch (w->getWidgetType()) {
            case WidgetType::Menu: {
                const int v = static_cast<Menu*>(w)->getValue();
                (void)vs.setInt(k, v);
                overrideUntil_[wid] = nowMs_ + VS_OVERRIDE_COOLDOWN_MS;  // let user “win” briefly
                w->setActive(false);
                touched = true;
                break;
            }

            case WidgetType::Edit: {
                const int v  = static_cast<Edit*>(w)->getValue();
                const bool ok = vs.setInt(k, v);
                const auto rb = vs.getInt(k).value_or(INT_MIN);
                printf("[COMMIT→VS] sid=%u wid=%u v=%d ok=%d readback=%ld\n",
                       (unsigned)sid, (unsigned)wid, v, (int)ok, (long)rb);

                overrideUntil_[wid] = nowMs_ + VS_OVERRIDE_COOLDOWN_MS;
                //touched = true;
                break;
            }

            case WidgetType::Button: {
                const bool v = static_cast<Button*>(w)->getState();
                (void)vs.setBool(k, v);
                overrideUntil_[wid] = nowMs_ + VS_OVERRIDE_COOLDOWN_MS;
                //touched = true;
                break;
            }

            default:
                break;
        }
    }

    // IMPORTANT: bump AFTER all writes so Core-1 adopts the committed values
    static uint32_t uiCommit = 0;  // local monotonic bump on Core-0
    printf("send commit\n");
    (void)vs.setU32(VSIDs::K_UI_COMMIT, ++uiCommit);
    const auto commitVal=vs.getU32(VSIDs::K_UI_COMMIT);
    printf("K_UI_COMMIT:%d",commitVal);
}



// Helper: apply a single widget’s VS value if its slot is dirty.
// Clears the slot’s dirty bit after applying. Respects Edit::isActive() to avoid stomping user edits.
static bool applyVSIfDirty(Widget* w,
                           uint16_t screenId,
                           uint32_t nowMs,
                           const std::unordered_map<uint16_t, uint32_t>& overrideUntil) {
    auto& vs = ValueStore::instance();
    if (!vs.frozen()) return false;

    const uint16_t wid = static_cast<uint16_t>(w->getWidgetId());

    // Respect recent local commits (“user wins”)
    if (auto it = overrideUntil.find(wid); it != overrideUntil.end()) {
        if (nowMs < it->second) {
            return false; // keep dirty set; UI hasn't “consumed” it yet
        }
    }

    const ValueKey k = VKey(ValueCat::Widget, screenId, wid);
    auto idx = vs.indexOf(k);
    if (!idx || !vs.slotDirty(*idx)) return false;

    bool changed = false;

    switch (w->getWidgetType()) {
        case WidgetType::Edit: {
            auto* ed = static_cast<Edit*>(w);
            if (!ed->isActive()) {
                int cur = ed->getValue();
                int v   = vs.getInt(k).value_or(cur);
                if (v != cur) { ed->setValue(v); changed = true; }
                else {
                    // UI already matches store: consume dirty so ACK can drop
                    (void)vs.clearDirty(k);
                    return false;
                }
            } else {
                return false; // don’t stomp active edits
            }
        } break;

        case WidgetType::Button: {
            auto* bt = static_cast<Button*>(w);
            bool cur = bt->getState();
            bool v   = vs.getBool(k).value_or(cur);
            if (v != cur) { bt->toggle(); changed = true; }
            else {
                (void)vs.clearDirty(k);
                return false;
            }
        } break;

        case WidgetType::Menu: {
            // Add a setter when available; until then, just consume if equal:
            // int cur = static_cast<Menu*>(w)->getValue();
            // int v   = vs.getInt(k).value_or(cur);
            // if (v == cur) { (void)vs.clearDirty(k); return false; }
            // else { static_cast<Menu*>(w)->setValue(v); changed = true; }
            (void)vs.clearDirty(k);
            return false;
        } break;

        default: {
            (void)vs.clearDirty(k);
            return false;
        }
    }

    if (changed) (void)vs.clearDirty(k);
    return changed;
}
// screen.cpp
void Screen::syncWidgetsFromVSIfIdle() {
    auto& vs = ValueStore::instance();
    const uint16_t sid = static_cast<uint16_t>(screenId);

    for (auto* w : widgets) {
        if (!w) continue;
        if (w->isActive()) continue;  // while editing, show local buffer

        const uint16_t wid = static_cast<uint16_t>(w->getWidgetId());
        // UI commit cooldown: skip VS adoption if still within grace window
        auto it = overrideUntil_.find(wid);
        if (it != overrideUntil_.end() && nowMs_ < it->second) continue;

        const ValueKey k = VKey(ValueCat::Widget, sid, wid);
        bool applied = false;

        switch (w->getWidgetType()) {
          case WidgetType::Edit: {
              if (auto v = vs.getInt(k)) {
                  Edit* e = static_cast<Edit*>(w);
                  if (e->getValue() != *v) {
                      e->setValue(*v);
                      applied = true;
                  }
              }
              break;
          }
          case WidgetType::Button: {
              if (auto b = vs.getBool(k)) {
                  Button* bt = static_cast<Button*>(w);
                  if (bt->getState() != *b) {
                      bt->setState(*b);
                      applied = true;
                  }
              }
              break;
          }
          case WidgetType::Menu: {
              if (auto s = vs.getInt(k)) {
                  Menu* m = static_cast<Menu*>(w);
                  if (m->getValue() != *s) {
                      m->setValue(*s);
                      applied = true;
                  }
              }
              break;
          }
          default: break;
        }

        if (applied) {
            // simplest: redraw the whole screen
            refresh = Rect2(0, 0, 158, 64);
            // (or, if you prefer: union with this widget’s rect)
        }
    }
}



// Option B: base update always runs, then calls onUpdate().
void Screen::update(uint16_t deltaTimeMS) {
     nowMs_ += deltaTimeMS; 
    syncWidgetsFromVSIfIdle();   // mirror VS for all non-active widgets
    #if VS_WIDGET_POLL_ENABLE
        vsPollTick_ += deltaTimeMS;
        if (vsPollTick_ >= VS_WIDGET_POLL_MS) {
            vsPollTick_ = 0;

            auto& vs = ValueStore::instance();
            if (vs.anyDirty()) {                          // ← new gate
                bool any = false;
                for (auto* w : widgets) {
                    any |= applyVSIfDirty(w,
                                        static_cast<uint16_t>(screenId),
                                        nowMs_,
                                        overrideUntil_);
                }
                if (any) {
                    refresh = Rect2(0,0,158,64);
                }
            }
        }
    #endif

    // Give the derived screen a turn
    onUpdate(deltaTimeMS);
}