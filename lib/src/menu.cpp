#include "menu.h"
#include <algorithm>   // for std::max_element

Menu::Menu(const std::string& text, const std::vector<std::string> _its,int _selId, int _x, int _y, int _w, int _h)
        : Widget(text, _x,_y,_w,_h), items(std::move(_its)) ,selectedMenuItem(_selId)
 {
    TRACE("");
    widgetType = WidgetType::Menu;
    selectable = true;    // explicit but optional
    itemCount=(int)items.size();

    auto it = std::max_element(
        items.begin(), items.end(),
        [](auto const& a, auto const& b) {
            return a.size() < b.size();
        }
    );
    boundingBox.w = fontWidth*static_cast<int>( it->size() );
}

void Menu::draw(Display *disp) const {
    TRACE_CAT(UI,"selected:%d, active:%d",selected,active);

    int noofItems=(int)items.size();
    term6x9->drawText(disp, label, Vec2(boundingBox.x, boundingBox.y), 255, 1);     
    disp->setInverted(selected);
    if(active){
        disp->fillRect(Rect2(boundingBox.x+menuOffs, boundingBox.y, boundingBox.w+4, noofItems*fontHeight),1,255);
        disp->rect(Rect2(boundingBox.x+menuOffs-1, boundingBox.y, boundingBox.w+5, noofItems*fontHeight),0,255);

        for (int i = 0; i < noofItems; ++i) {
            bool isSel = (i == selectedMenuItem);
            // draw highlight if selected
            if (isSel) disp->fillRect(Rect2(boundingBox.x+menuOffs-1, boundingBox.y + i*fontHeight, boundingBox.w+4, fontHeight), 0,255);
            // draw the text
            disp->setInverted(isSel);
            ariel5x8->drawText(disp, items[i], Vec2(boundingBox.x+2+menuOffs, boundingBox.y + i*fontHeight +2),255,1);
        }  
    }else{
        disp->fillRect(Rect2(boundingBox.x+menuOffs, boundingBox.y, boundingBox.w+4, fontHeight), 0,255);
        ariel5x8->drawText(disp, items[selectedMenuItem], Vec2(boundingBox.x+2+menuOffs, boundingBox.y+2), 255, 1);    
    }
    disp->setInverted(false);  
}

//should really used a "value" field in the widget class and initialvalue descriptor field instead of selectedMenuitem
void Menu::changeMenuSelection(int dirctn){
    TRACE_CAT(KEY,"direction:%d",dirctn);

    if(dirctn==1){ //forward
        if(selectedMenuItem<(itemCount-1)){
            selectedMenuItem++;
        }
    }else{//back
        if(selectedMenuItem>0){
            selectedMenuItem--;
        }            
    }
}

