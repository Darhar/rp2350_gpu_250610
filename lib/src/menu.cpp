#include "menu.h"

Menu::Menu(const std::string& text, int x, int y, int width, int height)
    : Widget(text, x, y, width, height) {
    printf("[menu] const start\n");
    widgetType = WidgetType::Menu;
    selectable = false;    // explicit but optional
    printf("[menu] constr fin\n");
}

void Menu::draw(Display *disp) const {
    printf("[menu] draw\n");
    if (selected) {
        disp->setInverted(true);
    } else {
        disp->setInverted(false);
    }
    term6x9->drawText(disp, label, Vec2(boundingBox.x, boundingBox.y), 255, 1);        
}

/*
Menu::Menu(std::vector<std::string> menuDat,uint8_t wid){
    data=menuDat;
    width=wid;
    length=50;
    itemStartIndx=0;
    selection=0;
    lastIndex=menuDat.size(); 
    fontHeight=10;
    displayItems=5;
    show=true;
}

Menu::~Menu(){}

void Menu::update(uint16_t deltaTimeMS){}

void Menu::showMenu(bool sm){
    show=sm;
}

uint8_t Menu::getSelection(){
    return selection;
}

void Menu::draw(Display *display){
    int positionIndx=0;
    char buff[30];

    uint8_t menuheight=displayItems*fontHeight;
    if(show){
        display->fillRect(Rect2(3,3,width,menuheight),  0, 255);
        display->rect(Rect2(3,3,width,menuheight),1);
        for(std::vector<std::string>::size_type itemINdx = itemStartIndx; positionIndx != 5; itemINdx++) {
            if(itemINdx==selection){
                display->setInverted(true);
            }
            ariel5x8->drawText(display, data[itemINdx].c_str(), Vec2(6, positionIndx*fontHeight+5), 255, 1);
            display->setInverted(false);
            positionIndx++;
        }        
    }


    sprintf (buff, " %d:%d ", selection,itemStartIndx);
    ariel5x8->drawText(display, buff, Vec2(90, 10), 255, 1);
}

void Menu::changeSelection(uint8_t dirctn){
    int firstItemPos=selection-itemStartIndx;
    printf("STA firstItemPos:%d,selection:%d,lastIndex:%d\n",firstItemPos,selection,lastIndex);
    if(dirctn==0){ //back
        if(selection>0){
            printf("A");
            if(firstItemPos<=0){
                printf("00");
                itemStartIndx--;
            }            
           selection--; 
        }  
    }else{//forward
        if(selection<lastIndex-1){
            printf("B");
            if(firstItemPos>=(displayItems-1)){
                printf("11");
                itemStartIndx++;
            }
            selection++;    
        } 
    }
    printf("\nFIN firstItemPos:%d,selection:%d,lastIndex:%d\n",firstItemPos,selection,lastIndex);

}

*/
