#include "menuscreen.h"
#include <keyboard.h>


MenuScreen::MenuScreen(ScreenManager& mgr)
  : mgr(mgr)   // initializer list
{
    printf("[MenuScreen] loading...\n");
    screenId = ScreenEnum::MENUSCREEN;
    //returnCallBack = rcb;
    currentMenuItem = 0;
    selectedMenuItem = 0;
    isAnimating = false;
    animationCounter = 0;
    currentOptionItem = 1;
    funkyV16->setClearSpace(true);
    option = option;
    refresh=Rect2(0,0,158,64);
    printf("[MenuScreen] Done\n");
}

MenuScreen::~MenuScreen() {
    printf("[MenuScreen] Destructing\n");
}

void MenuScreen::addWidget(Widget* widget,uint32_t widgetId){

}

void MenuScreen::update(uint16_t deltaTimeMS) {
    duration += deltaTimeMS;
    accDeltaTimeMS += deltaTimeMS;
  
    if(accDeltaTimeMS>200){    
        if(isAnimating) {
            animationCounter +=4;
            if(animationCounter > menuItemGap) {
                animationCounter = 0;
                isAnimating = false;
                currentMenuItem = selectedMenuItem;
                currentOptionItem = 1;
            }
            refresh=Rect2(0,0,158,64);
        }
    }  
}

void MenuScreen::draw(Display *display) {
    funkyV16->drawText(display, menuItemNames[currentMenuItem][0],Vec2( animationCounter+10, 10), 255, 1);
}

int MenuScreen::keyPressed(uint8_t key) {
   
    printf("MenuScreen keypressed\n");
    if(isAnimating)
        return 0;

    if(key == KEY_UP) {
        if(selectedMenuItem < menuCount-1) {
            selectedMenuItem++;
            isAnimating = true;
            refresh=Rect2(0,0,158,64);

        }
    } else if (key == KEY_DOWN) {
        if(selectedMenuItem != 0) {
            selectedMenuItem--;
            isAnimating = true;
            refresh=Rect2(0,0,158,64);

        }
    } else if(key == KEY_OK) {
        printf("selectionItem %d, \n",selectedMenuItem);
        refresh=Rect2(0,0,158,64);
        return encodeKeyReturn(KeyReturn::SCRSELECT, selectedMenuItem+1);
    }
    printf("selectionItem %d, \n",selectedMenuItem);
    return 0;
}

int MenuScreen::keyReleased(uint8_t key) {
    // const char c[6] = {'U', 'D', 'L', 'R', 'A', 'B'};
    // printf("Key Released: %c\n", c[key]);
    return 0;
}

int MenuScreen::keyDown(uint8_t key){
    // const char c[6] = {'U', 'D', 'L', 'R', 'A', 'B'};
    // printf("Key Down: %c\n", c[key]);
    return 0;
}
