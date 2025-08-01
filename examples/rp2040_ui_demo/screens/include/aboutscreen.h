#include <common.h>
#include <screen.h>
#include <keyboard.h>

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
    int selectedIndex = -1;
    std::string title;
    ScreenManager& mgr;
    void buildFromDescriptor();

public:
    AboutScreen(ScreenManager& mgr) : mgr(mgr){
        printf("[AboutScreen] loading...\n");
        screenId = ScreenEnum::ABOUTSCREEN;
        scroll = 0;
        refresh=Rect2(0,0,158,64);
        printf("[AboutScreen] Done\n");
    }
    ~AboutScreen();
	//void addWidget(Widget* widget,uint32_t widgetId);
    void update(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
