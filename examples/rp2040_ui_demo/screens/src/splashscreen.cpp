#include "splashscreen.h"

SplashScreen::~SplashScreen() {
    printf("[SplashScreen] Destructing\n");
}

void SplashScreen::update(uint16_t deltaTimeMS) {

    duration += deltaTimeMS;
    accDeltaTimeMS += deltaTimeMS;
  
    if(accDeltaTimeMS>100){
        lastTime++;
        uint8_t offs=DISPLAY_HEIGHT-16; 
        posX += dirX;
        posY += dirY; 
        
        if(posX>DISPLAY_WIDTH-16){
             posX=DISPLAY_WIDTH-16;
            dirX=-4;
        }
        if(posX<1){
            posX=1;
            dirX=4;
        }
        if(posY>offs){
            posY=offs;
            dirY=-2;
        }
        if(posY<1){
            posY=1;  
            dirY=2;  
        } 

        accDeltaTimeMS=0;
        title2="         ";
        refresh=Rect2(0,0,158,64);
    }
}

void SplashScreen::draw(Display *display) {
    char outStr[30]="";
    sprintf(outStr,"%d",lastTime);
    title1=outStr;

    ball00->setPosition(Vec2(posX,posY));
    ball00->draw(display);    

    funkyV16->drawText(display, title, Vec2((DISPLAY_WIDTH-(funkyV16->getTextWidth(title)))/2, 5), 255, 1);
    ariel5x8->drawText(display, title1, Vec2(10, 28), 255, 1);
    term6x9->drawText(display, title2, Vec2(10, 45), 255, 1);

}

int SplashScreen::keyPressed(uint8_t key) {
    if(key == KEY_BACK){
        return encodeKeyReturn(KeyReturn::SCRSELECT, ScreenEnum::MENUSCREEN);
    }
    return 0;
}

int SplashScreen::keyReleased(uint8_t key) {
    char outStr[30]="";
    sprintf(outStr,"%d released",key);
    title2=outStr;
    refresh=Rect2(0,0,158,64);
    return 0;
}

int SplashScreen::keyDown(uint8_t key){
    char outStr[30]="";
    sprintf(outStr,"%d down",key);
    title2=outStr;
    refresh=Rect2(0,0,158,64);
    return 0;
}
