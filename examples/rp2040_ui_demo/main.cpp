#include "common.h"
#include "display.h"
#include <testscreen.h>
#include <menuscreen.h>
#include <aboutscreen.h>
#include <settingsscreen.h>
#include <basicscreen.h>
#include <ctime>
#include <time.h>
#include "keyboard.h"
#include <hardware/i2c.h>
#include "pico/multicore.h"
#include <pico/i2c_slave.h>
#include "i2c_obj.hpp"
#include "command_factory.hpp"
#include "screenManager.hpp"

#define FLAG_VALUE 123

uint8_t buf[32];
static const uint I2C_SLAVE_ADDRESS = 0x17;
static const uint I2C_BAUDRATE = 100000; // 100 kHz
static const uint I2C_SLAVE_SDA_PIN = 12; // 4
static const uint I2C_SLAVE_SCL_PIN = 13; // 5
static const uint I2C_MASTER_SDA_PIN = 26;
static const uint I2C_MASTER_SCL_PIN = 27;

/*
static uint8_t registers[20] = {
    0xA1, 0x00, 0x01, 0x12, 0x34, 0x56, 0x78, 0xFF,
    0xA1, 0x00, 0x01, 0x12, 0x34, 0x56, 0x78, 0xFF,
    0xA1, 0x00, 0x01, 0x12
};
static uint8_t selected_register = 0;
*/


//bool shouldSwitchScreen;
uint8_t newScreenId, newOption;

volatile uint8_t rx_buf[256];
volatile uint8_t rx_index = 0;
i2cObj* activeCommand = nullptr;

#define MAX_BUFFER 64

uint8_t recv_buffer[MAX_BUFFER];
volatile int recv_index = 0;

ScreenManager screenMgr;

void setup_master(){
    gpio_init(I2C_MASTER_SDA_PIN);
    gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SDA_PIN);
    gpio_init(I2C_MASTER_SCL_PIN);
    gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SCL_PIN);
    i2c_init(i2c1, I2C_BAUDRATE);
}

void encodeCommand(uint8_t cmdId, uint8_t flags, uint8_t screenId, uint32_t paramBits, uint8_t (&bytes)[4]) {
    // Apply bit limits
    cmdId     &= 0x1F;       // 5 bits
    flags     &= 0x07;       // 3 bits
    screenId  &= 0x3F;       // 6 bits
    paramBits &= 0x3FFFF;    // 18 bits

    // Encode into 32-bit value
    uint32_t command = (cmdId)
                     | (flags << 5)
                     | (screenId << 8)
                     | (paramBits << 14);

    // Split into 4 bytes (big-endian order)
    bytes[0] = static_cast<uint8_t>(command & 0xFF);
    bytes[1] = static_cast<uint8_t>((command >> 8) & 0xFF);
    bytes[2] = static_cast<uint8_t>((command >> 16) & 0xFF);
    bytes[3] = static_cast<uint8_t>((command >> 24) & 0xFF);
}

void run_master() {

    uint8_t buffer[4];
    //encodeCommand(cmdId,flags,screenId,paramBits, uint8_t (&bytes)[4])
    encodeCommand(i2c_scrCng, 0, 2, 0x12345, buffer);

    printf("sending %d bytes\n",sizeof(buffer));

    i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, buffer, 4, false);
    sleep_ms(1000);
    encodeCommand(i2c_scrCng, 0, 1, 0x12345, buffer);

    i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, buffer, 4, false);
    sleep_ms(1000);

}

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

                printf("cmdId=%d screenId=%d param=0x%05X hasParams=%d\n", cmdId, screenId, paramBits, hasParams);

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

void core1_entry() {
   int  dbCount=0;

    multicore_fifo_push_blocking(FLAG_VALUE);
    uint32_t g = multicore_fifo_pop_blocking();

	sleep_ms(200);
    if (g != FLAG_VALUE)
        printf("Problem with core 1!\n");
    else
        printf("Core 1 up\n");

    while (1){
        //run_master();        
        printf("core1 %d\n",dbCount++);  
        Screen* thisScreen=screenMgr.getActiveScreen();
        printf("scr id:%d\n",thisScreen->screenId);
        sleep_ms(5000);
    }
}

void registerAllScreens(ScreenManager& mgr) {
    static ScreenFactoryFunc screenObjects[SCREEN_COUNT] = {
        []() -> Screen* { return new MenuScreen(); },
        []() -> Screen* { return new TestScreen(); },
        //[]() -> Screen* { return new SettingsScreen(); },
        //[]() -> Screen* { return new AboutScreen(); },
        //[]() -> Screen* { return new BasicScreen(); },
    };

    for (int i = 0; i < SCREEN_COUNT; ++i) {

        if (screenObjects[i]) {
            auto id = static_cast<ScreenEnum>(i);
            mgr.registerScreen(id, screenObjects[i]);

            // Optionally, seed the descriptor so buildScreenFromDescriptor()
            // will reconstruct the widgets the first time:
            auto& desc = mgr.getDescriptor(id);
            if (desc.widgets.empty()) {
            // e.g. call a helper to populate default widgets:
            //seedDefaultWidgetsForScreen(id, desc);
            // widgetId must be unique per‐screen

                if(id==ScreenEnum::TESTSCREEN){
                     desc.widgets.push_back({
                        WidgetType::Label,    // type
                        /*widgetId=*/1,
                        /*initialText=*/"First Label",
                        /*x=*/5, /*y=*/15, /*w=*/20, /*h=*/10
                    });
                    desc.widgets.push_back({
                        WidgetType::Button,
                        /*widgetId=*/2,
                        /*initialText=*/"Click",  // button label
                        5, 28, 100, 10
                    });
                    desc.widgets.push_back({
                        WidgetType::Edit,
                        /*widgetId=*/3,
                        /*initialText=*/"",       // initial contents
                        5, 42, 100, 10
                    });
                    // any other screen‐level state:
                    // desc.selectedIndex = 0; 
                    printf("widgets size:%d\n",desc.widgets.size());
                }
            }
        }
    }
}

int main()
{
    stdio_init_all();
    srand((unsigned int)time(0));
	uint32_t msSinceBoot=to_ms_since_boot(get_absolute_time());
    Display *display=new Display();
	sleep_ms(5000);
    printf("Starting\n");
    KeyBoard *keyboard = new KeyBoard();

    timetype lastUpdate = getTime();
    //display->clearBg();
    display->clear(0);
    
    multicore_launch_core1(core1_entry);
    setup_slave();
    setup_master();
	sleep_ms(500);
    uint32_t g = multicore_fifo_pop_blocking(); 

    registerAllScreens(screenMgr);
    screenMgr.setActiveScreen(MENUSCREEN);
    sleep_ms(1000);

    if (g != FLAG_VALUE)
        printf("Problem with core 0!\n");
    else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("Core 0 up\n");
    }

    while (true) {
        
        uint16_t deltaTimeMS = getTimeDiffMS(lastUpdate);
        lastUpdate = getTime();
         
        keyboard->checkKeyState(&screenMgr);
        screenMgr.update(deltaTimeMS);
        if(screenMgr.needRefresh()){
            printf("redraw!\n");
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
