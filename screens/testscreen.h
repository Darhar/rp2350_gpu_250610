#include "../common.h"
#include "../screen.h"
#include "../keyboard.h"
#include "../label.h"
#include "../button.h"
#include "../edit.h"

#include <vector>

class TestScreen : public Screen
{
private:

    uint16_t duration, lastTime,accDeltaTimeMS;
    int selectedIndex = -1;
    std::string title;
    uint8_t option;
    std::vector<Widget*> widgets;

public:
    TestScreen();
    ~TestScreen();
    
    void addWidget(Widget* widget);
    void update(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
