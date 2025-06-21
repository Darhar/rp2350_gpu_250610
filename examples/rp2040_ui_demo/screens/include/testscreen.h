#include <common.h>
#include <screen.h>
#include <keyboard.h>
#include <label.h>
#include <button.h>
#include <edit.h>

#include <vector>

class TestScreen : public Screen
{
private:

    uint16_t duration, lastTime,accDeltaTimeMS;
    int selectedIndex = -1;
    std::string title;
    //std::vector<Widget*> widgets;

public:
    TestScreen();
    ~TestScreen();
    
    void addWidget(Widget* widget,uint32_t widgetId);
    void update(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
