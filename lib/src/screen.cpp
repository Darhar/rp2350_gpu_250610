#include "screen.h"
#include "ScreenManager.hpp"

void Screen::addWidget(Widget* w, uint32_t id) {
  w->setId(id);
  widgets.push_back(w);
  if (selectedIndex == -1 && w->isSelectable()) {
    selectedIndex = widgets.size() - 1;
    w->setSelected(true);
  }
  refresh = Rect2(0,0,158,64);
}

void Screen::buildFromDescriptor(ScreenManager& mgr) {
  printf("[Screen] buildScreenFromDescriptor\n");
  widgets.clear();
  selectedIndex = -1;
  auto& desc = mgr.getDescriptor(scrEnum);
  for (auto& wd : desc.widgets) {
    Widget* w = mgr.createWidgetFromDescriptor(wd);
    if (!w) continue;
    addWidget(w, wd.widgetId);
  }
}
