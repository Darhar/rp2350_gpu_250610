#include "menuscreen.h"
#include <keyboard.h>


MenuScreen::MenuScreen(ScreenManager& mgr)
  : mgr(mgr)   // initializer list
{
    TRACE("");
    screenId = ScreenEnum::MENUSCREEN;
    currentMenuItem = 0;
    selectedMenuItem = 0;
    isAnimating = false;
    animationCounter = 0;
    currentOptionItem = 1;
    funkyV16->setClearSpace(true);
    option = option;
    refresh=Rect2(0,0,158,64);
}

MenuScreen::~MenuScreen() {}

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
    TRACE("");
    display->setInverted(false);
    funkyV16->drawText(display, menuItemNames[currentMenuItem][0],Vec2( animationCounter+10, 10), 255, 1);
}

int MenuScreen::keyPressed(uint8_t key) {
    TRACE("");
    if(isAnimating)
        return 0;

    if(key == KEY_UP) {
        TRACE("key Up");
        if(selectedMenuItem < menuCount-1) {
            selectedMenuItem++;
            isAnimating = true;
            refresh=Rect2(0,0,158,64);

        }
    } else if (key == KEY_DOWN) {
        TRACE("key Dwn");
        if(selectedMenuItem != 0) {
            selectedMenuItem--;
            isAnimating = true;
            refresh=Rect2(0,0,158,64);

        }
    } else if(key == KEY_OK) {
        TRACE("key OK");
        refresh=Rect2(0,0,158,64);
        return encodeKeyReturn(KeyReturn::SCRSELECT, selectedMenuItem+1);
    }
    TRACE("scr selectionItem %d",selectedMenuItem);
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
