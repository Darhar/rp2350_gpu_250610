#include <common.h>
#include <screen.h>
#include <keyboard.h>

class BasicScreen : public Screen
{
private:
    uint8_t option; 
    std::string title =  "Basic Screen";  
    ScreenManager& mgr;
    int selectedIndex = -1;
    
public:
    BasicScreen(ScreenManager& mgr) : mgr(mgr){
        printf("[BasicScreen] loading...\n");
        screenId = ScreenEnum::BASICSCREEN;
        option = option;
        title =  "Basic Screen";
        refresh=Rect2(0,0,158,64);
        printf("[BasicScreen] Done\n");
    }
    ~BasicScreen();
	//void addWidget(Widget* widget,uint32_t widgetId);
    void update(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
