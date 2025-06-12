#include "basicscreen.h"

BasicScreen::BasicScreen() {
    printf("[BasicScreen] loading...\n");
    screenId = ScreenEnum::BASICSCREEN;
    option = option;
    title =  "Basic Screen";
    refresh=Rect2(0,0,158,64);
    printf("[BasicScreen] Done\n");
}

BasicScreen::~BasicScreen() {}

void BasicScreen::update(uint16_t deltaTimeMS) {}

void BasicScreen::draw(Display *display) {
    funkyV16->drawText(display, title, Vec2((DISPLAY_WIDTH-(funkyV16->getTextWidth(title)))/2, 5), 255, 1);
}

int BasicScreen::keyPressed(uint8_t key) {
    if(key == KEY_BACK){
        refresh=Rect2(0,0,158,64);
        return encodeKeyReturn(KeyReturn::SCRSELECT, ScreenEnum::MENUSCREEN);
    }
    return 0;
}

int BasicScreen::keyReleased(uint8_t key) {
    return 0;
}

int BasicScreen::keyDown(uint8_t key){
    return 0;
}
