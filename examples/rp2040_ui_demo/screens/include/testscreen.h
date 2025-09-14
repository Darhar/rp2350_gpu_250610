#include <common.h>
#include <screen.h>
#include <keyboard.h>
#include <label.h>
#include <button.h>
#include <edit.h>
#include <vector>
#include "screenManager.hpp"

class TestScreen : public Screen
{
	public:
		explicit TestScreen(ScreenManager& mgr);   // <- declaration only (no body)
		~TestScreen();

		void draw(Display *display) override;
		int keyPressed(uint8_t key);
		int keyReleased(uint8_t key);
		int keyDown(uint8_t key);
		
	private:
		uint32_t duration = 0;
		uint32_t accDeltaTimeMS = 0;
		uint16_t lastTime;
		std::string title;
		ScreenEnum     scrEnum;
		void seedConfig();

	protected:
		// Option B: rename old update(...) override to onUpdate(...)
		void onUpdate(uint16_t deltaTimeMS) override;
};
