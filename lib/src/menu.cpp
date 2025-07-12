#include "menu.h"
#include <algorithm>   // for std::max_element

Menu::Menu(const std::string& text, const std::vector<std::string> _its,int _selId, int _x, int _y, int _w, int _h)
        : Widget(text, _x,_y,_w,_h), items(std::move(_its)) ,selectedMenuItem(_selId)
 {
    printf("[menu] constr start\n");
    widgetType = WidgetType::Menu;
    selectable = true;    // explicit but optional
    itemCount=(int)items.size();

    auto it = std::max_element(
        items.begin(), items.end(),
        [](auto const& a, auto const& b) {
            return a.size() < b.size();
        }
    );
    boundingBox.w = 5*static_cast<int>( it->size() );

    printf("[menu] constr fin\n");
}

void Menu::draw(Display *disp) const {
    printf("[menu] draw\n");
    int noofItems=(int)items.size();
    disp->setInverted(false);

    term6x9->drawText(disp, label, Vec2(boundingBox.x, boundingBox.y), 255, 1);    
    if (selected) {
        disp->setInverted(true);
    }

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
            //disp->setInverted(false);
        }  
    }else{
        disp->fillRect(Rect2(boundingBox.x+menuOffs, boundingBox.y, boundingBox.w, fontHeight), 0,255);
        term6x9->drawText(disp, "test", Vec2(boundingBox.x+2+menuOffs, boundingBox.y), 255, 1);    
    }
}
//should really used a "value" field in the widget class and initialvalue descriptor field instead of selectedMenuitem
void Menu::changeMenuSelection(int dirctn){
    printf("[Menu] changeMenuSelection:%d\n",dirctn);

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

