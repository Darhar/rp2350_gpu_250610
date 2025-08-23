#include "ui_demo.h"
#include "i2c_master.h"
#include "common.h"
#include "display.h"
#include <ctime>
#include <time.h>
#include <hardware/i2c.h>
#include "pico/multicore.h"
#include <pico/i2c_slave.h>
#include "i2c_obj.hpp"
#include "keyboard.h"
#include "command_factory.hpp"
#include "screenManager.hpp"

#define MAX_BUFFER 64

uint8_t recv_buffer[MAX_BUFFER];
volatile int recv_index = 0;
i2cObj* activeCommand = nullptr;

ScreenManager screenMgr;
Debug debug;

void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
        case I2C_SLAVE_RECEIVE:
            if (recv_index < MAX_BUFFER) {
                recv_buffer[recv_index++] =  i2c_read_byte_raw(i2c);
            }
            break;
        case I2C_SLAVE_REQUEST: // Master is requesting data (not used here)
                i2c_write_byte_raw(i2c,0);            
            break;

        case I2C_SLAVE_FINISH:
            if (recv_index >= 4) {
                uint32_t cmdWord = (recv_buffer[0]) |
                                (recv_buffer[1] << 8) |
                                (recv_buffer[2] << 16) |
                                (recv_buffer[3] << 24);

                uint8_t cmdId      =  cmdWord        & 0x1F;
                uint8_t flags      = (cmdWord >> 5)  & 0x07;
                uint8_t screenId   = (cmdWord >> 8)  & 0x3F;
                uint32_t paramBits = (cmdWord >> 14) & 0x3FFFF;

                bool hasParams = flags & 0b001;
                bool useActive = flags & 0b010;

                if (useActive) {
                    Screen* thisScreen=screenMgr.getActiveScreen();
                    screenId = thisScreen->screenId;
                }

                DEBUG_PRINTLN("i2c_slave_handler cmdId=%d screenId=%d param=0x%05X hasParams=%d\n", cmdId, screenId, paramBits, hasParams);

                if (activeCommand) {
                    delete activeCommand;
                }

                const uint8_t* extraData = (recv_index > 4) ? &recv_buffer[4] : nullptr;
                size_t extraLen = (recv_index > 4) ? recv_index - 4 : 0;

                // Factory can now take cmdId + screenId + paramBits + extraData
                activeCommand = createCommandObject(cmdId, screenId, paramBits, extraData, extraLen,screenMgr);
            }

            recv_index = 0;
            break;
    }
}

void setup_slave() {
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
    i2c_slave_init (i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
}

int main()
{

    stdio_init_all();
    srand((unsigned int)time(0));
	uint32_t msSinceBoot=to_ms_since_boot(get_absolute_time());
    timetype lastUpdate = getTime();
    while (!stdio_usb_connected() & WAITFORSERIAL) {
        sleep_ms(10);  // Wait for USB host to open the port
    }

	sleep_ms(500);
    printf("ui_demo\n");
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
