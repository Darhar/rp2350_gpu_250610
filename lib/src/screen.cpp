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



