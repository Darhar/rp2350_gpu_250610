#include <common.h>
#include <screen.h>
#include <keyboard.h>

class BasicScreen : public Screen
{
private:
    uint8_t option; 
    std::string title =  "Basic Screen";  
    int selectedIndex = -1;
    
public:
    BasicScreen(ScreenManager& mgr);
    ~BasicScreen();
    void onUpdate(uint16_t deltaTimeMS);
    void draw(Display *display);
    int keyPressed(uint8_t key);
    int keyReleased(uint8_t key);
    int keyDown(uint8_t key);
};
