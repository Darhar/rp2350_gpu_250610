#include "common.h"

#include "screens_demo.h"
#include "display.h"

#include <ctime>
#include <time.h>
#include <hardware/i2c.h>
#include "pico/multicore.h"
#include <pico/i2c_slave.h>
#include "i2c_obj.hpp"
#include "keyboard.h"


int main()
{

    stdio_init_all();
    srand((unsigned int)time(0));
	uint32_t msSinceBoot=to_ms_since_boot(get_absolute_time());
    timetype lastUpdate = getTime();
    while (!stdio_usb_connected()) {
        sleep_ms(10);  // Wait for USB host to open the port
    }
	sleep_ms(500);

    Display *display=new Display();
    debug.setDisplay(display);
    debug.registerVar("fb", display->getFrameBufferPtr(), DISPLAY_WIDTH*DISPLAY_HEIGHT);    

    //display->clearBg();
    display->clear(0);
    debug.printHelp();
    KeyBoard *keyboard = new KeyBoard();
    
    multicore_launch_core1(core1_entry);
    setup_slave();
    setup_master();
    uint32_t g = multicore_fifo_pop_blocking(); 
 
    registerAllScreens(screenMgr);
    screenMgr.setActiveScreen(ScreenEnum::MENUSCREEN);
    sleep_ms(1000);

    if (g != FLAG_VALUE)
        printf("Problem with core 0!\n");
    else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("Core 0 up\n");
    }
    DEBUG_PRINTLN(" main loop");

    while (true) {
        uint16_t deltaTimeMS = getTimeDiffMS(lastUpdate);
        lastUpdate = getTime();
        debug.poll();
        keyboard->checkKeyState(&screenMgr);
        screenMgr.update(deltaTimeMS);
        if(screenMgr.needRefresh()){
            //DEBUG_PRINTLN("needRefresh");
            //display->drawBitmapRow(Vec2(0,0), DISPBUFSIZE, bg01);  
            display->clear(0);
            screenMgr.draw(display);
            display->update();
            screenMgr.setRefreshRect(Rect2(0,0,0,0));         
        }else{
            sleep_ms(20);
        }
        msSinceBoot=to_ms_since_boot(get_absolute_time());
     }
}
