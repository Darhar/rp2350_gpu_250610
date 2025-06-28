#include <common.h>
#include <screen.h>
#include <keyboard.h>
#include <label.h>
#include <button.h>
#include <edit.h>
#include <vector>

class TestScreen : public Screen
{
	public:
        TestScreen(ScreenManager& mgr) : mgr(mgr)
        {
            screenId = ScreenEnum::TESTSCREEN;
            scrEnum=ScreenEnum::TESTSCREEN;
            // build widgets once from the descriptor
            rebuildFromDescriptor();
            title =  "Test Screen";
            duration=0; 
            funkyV16->setClearSpace(true);
            refresh=Rect2(0,0,158,64);
            printf("[test] Started\n");
        }
		~TestScreen();
		
		void addWidget(Widget* widget,uint32_t widgetId);
		void update(uint16_t deltaTimeMS);
		void draw(Display *display);
		int keyPressed(uint8_t key);
		int keyReleased(uint8_t key);
		int keyDown(uint8_t key);
		
	private:
		uint16_t duration, lastTime,accDeltaTimeMS;
		int selectedIndex = -1;
		std::string title;
		ScreenManager& mgr;
		void rebuildFromDescriptor();
};
