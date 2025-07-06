#include <common.h>
#include <screen.h>
#include <keyboard.h>
#include <label.h>
#include <button.h>
#include <edit.h>
#include <vector>
#include "screenManager.hpp"
//#include "WidgetDescriptor.hpp"

class TestScreen : public Screen
{
	public:
        TestScreen(ScreenManager& mgr);
		~TestScreen();
		void commitActiveEditValue();
		//void addWidget(Widget* widget,uint32_t widgetId);
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
		ScreenEnum     scrEnum;
		void rebuildFromDescriptor();
		//void buildFromDescriptor();
		void seedDescriptor(ScreenManager& mgr);
};
