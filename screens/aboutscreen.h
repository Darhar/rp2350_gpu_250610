#include "../common.h"
#include "../screen.h"
#include "../keyboard.h"

class AboutScreen : public Screen
{
private:
    uint8_t option;
    uint16_t scroll;
    timetype lastUpdate;    
    std::string aboutText[10][5] = {
        {"One", "Scrolling "},
        {"Two", "Screen"},
        {"Three", "Scrolling"},
        {"Four", "Screen"},
        {"Five", "This", "is", "Scrolling", "Screen"},
        {"Six", "line 1", "Line 2", "line 3"},
    };

public:
    AboutScreen();
    ~AboutScreen();

    void update(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
