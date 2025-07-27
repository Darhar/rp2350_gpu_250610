#include "screen.h"
#include "ScreenManager.hpp"

void Screen::addWidget(Widget* widg, uint32_t id) {
  TRACE("id:%d",id);

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

void Screen::widgetToBottom(uint32_t widgetId) {
  // find it, remove it, then push_back so it draws last (on top)
  auto it = std::find_if(widgets.begin(), widgets.end(),
      [&](Widget* w){ return w->getWidgetId() == widgetId; });
  if (it != widgets.end()) {
    Widget* w = *it;
    widgets.erase(it);
    widgets.push_back(w);
  }
}

void Screen::widgetToTop(uint32_t widgetId) {
  TRACE("");
  auto it = std::find_if(widgets.begin(), widgets.end(),
    [&](Widget* w){ return w->getWidgetId() == widgetId; });
  if (it != widgets.end()) {
    Widget* w = *it;
    widgets.erase(it);
    widgets.insert(widgets.begin(), w);
  }
}


