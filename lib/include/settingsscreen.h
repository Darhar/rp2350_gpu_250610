#include "common.h"
#include "screen.h"
#include "keyboard.h"
#include "menu.h"
#include <vector>


class SettingsScreen : public Screen
{
private:
    uint8_t option; 
    std::string title =  "Splash Screen";   
    Menu *menu1;

    uint16_t duration;
    uint16_t accDeltaTimeMS;

public:
    SettingsScreen();
    ~SettingsScreen();
    void printMenu(Display *display,const std::vector<std::string> &menu);
    void update(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
