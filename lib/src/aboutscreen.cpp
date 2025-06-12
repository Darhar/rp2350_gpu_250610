#include "aboutscreen.h"
int scrollMax=420;
AboutScreen::AboutScreen() {
    printf("[AboutScreen] loading...\n");
    screenId = ScreenEnum::ABOUTSCREEN;
    //returnCallBack = rcb;
    option = option;
    scroll = 0;
    refresh=Rect2(0,0,158,64);

    printf("[AboutScreen] Done\n");
}

AboutScreen::~AboutScreen() {}

void AboutScreen::update(uint16_t deltaTimeMS) {
    uint16_t timeDiff = getTimeDiffMS(lastUpdate);
    if(timeDiff > 100) {
        scroll+=1;
        if(scroll >= scrollMax)
            scroll = 0;
        lastUpdate = getTime();
        refresh=Rect2(0,0,158,64);

    }
}

void AboutScreen::draw(Display *display) {
    int16_t y = DISPLAY_HEIGHT;
    for (int titleIndx = 0; titleIndx < 10; titleIndx++) {
        std::string title = aboutText[titleIndx][0];
        uint16_t width = funkyV16->getTextWidth(title);
        funkyV16->drawText(display, title, Vec2((DISPLAY_WIDTH - width)/2, y - scroll), 255, 1);
        y += 18;

        for (int textIndx = 1; textIndx < 5; textIndx++) {
            std::string text = aboutText[titleIndx][textIndx];
            if(text.length() > 0) {
                uint16_t width = funkyV16->getTextWidth(text);
                funkyV16->drawText(display, text, Vec2((DISPLAY_WIDTH - width)/2, y - scroll), 255, 1);
                y += 18;
            }
        }
        y += 20;
    }
}

int AboutScreen::keyPressed(uint8_t key) {
    if(key == KEY_BACK){
        return encodeKeyReturn(KeyReturn::SCRSELECT, ScreenEnum::MENUSCREEN);
    }
    return 0;
}

int AboutScreen::keyReleased(uint8_t key) {
    return 0;
}

int AboutScreen::keyDown(uint8_t key){
    return 0;
}
