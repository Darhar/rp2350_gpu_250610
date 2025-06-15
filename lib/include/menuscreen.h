#include "common.h"
#include "screen.h"

#define MENUCOUNT 4

class MenuScreen : public Screen
{
private:
    const uint8_t menuCount = MENUCOUNT;
    const std::string menuItemNames[MENUCOUNT][7] = {
        {"Test", "", "", "", "", "", ""},
        {"Settings", "", "", "", "", "", ""},
        {"About", "", "", "", "", "", ""},
        {"Basic", "", "", "", "", "", ""}
    };
    uint16_t duration, lastTime,accDeltaTimeMS;
    uint8_t selectedMenuItem, currentMenuItem, animationCounter;
    uint8_t currentOptionItem;
    bool isAnimating = false;
    const uint8_t menuItemGap = 120;
    uint8_t option;

public:
    MenuScreen();
    ~MenuScreen();

    void update(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
