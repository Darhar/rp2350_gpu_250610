#include "screen.h"
#include "ScreenManager.hpp"

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
}

void Screen::rebuildFromDescriptor(ScreenManager& mgr) {
    widgets.clear();
    selectedIndex = -1;
    auto& desc = mgr.getDescriptor(screenId);
    for (auto& wd : desc.widgets) {
        Widget* w = mgr.createWidgetFromDescriptor(wd);
        if (w) addWidget(w, wd.widgetId);
    }
}


