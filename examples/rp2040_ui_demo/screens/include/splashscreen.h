#include <common.h>
#include <screen.h>
#include <keyboard.h>

class SplashScreen : public Screen
{
private:

    uint16_t duration, lastTime,accDeltaTimeMS;

    uint8_t posX;
    uint8_t posY;
    uint8_t dirX;
    uint8_t dirY;
    std::string title;
    std::string title1;
    std::string title2;
    Sprite *ball00;
    int selectedIndex = 0;
    ScreenManager& mgr;

public:
    SplashScreen(ScreenManager& mgr) : mgr(mgr){
        printf("[SplashScreen] loading...\n");
        screenId = ScreenEnum::SPLASHSCREEN;
        posX=10;
        posY=10;
        dirX=2;
        dirY=2;
        title =  "Splash Screen";
        title1 = "Arial5x8 Font";
        title2 = "Terminal 6x9 Font";
        ball00=new Sprite(logoSprite,Vec2(10,10),Vec2(1,1));
        duration=0; 
        refresh=Rect2(0,0,158,64);    
        funkyV16->setClearSpace(true);
    }
    ~SplashScreen();
	//void addWidget(Widget* widget,uint32_t widgetId);
    void update(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
