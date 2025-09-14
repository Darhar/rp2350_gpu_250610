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


public:
    SplashScreen(ScreenManager& mgr);
    ~SplashScreen();
    void onUpdate(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
