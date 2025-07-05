#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "common.h"
#include "framebuffer.h"

#ifndef DISPLAY_H
    #define DISPLAY_H
    #define CMD_SET_COLUMN_LOWER       0x00
    #define CMD_SET_COLUMN_UPPER       0x10
    #define CMD_SET_RESISTOR_RATIO     0x20
    #define CMD_SET_POWER_CONTROL      0x28
    #define CMD_SET_DISP_START_LINE    0x40
    #define CMD_SET_VOLUME_FIRST       0x81
    #define CMD_SET_ADC_NORMAL         0xA0
    #define CMD_SET_ADC_REVERSE        0xA1
    #define CMD_SET_BIAS_9             0xA2 
    #define CMD_SET_BIAS_7             0xA3
    #define CMD_SET_ALLPTS_NORMAL      0xA4
    #define CMD_SET_ALLPTS_ON          0xA5
    #define CMD_SET_DISP_NORMAL        0xA6
    #define CMD_SET_DISP_REVERSE       0xA7
    #define CMD_SET_STATIC_OFF         0xAC
    #define CMD_SET_STATIC_ON          0xAD
    #define CMD_DISPLAY_OFF            0xAE
    #define CMD_DISPLAY_ON             0xAF
    #define CMD_SET_PAGE               0xB0
    #define CMD_SET_COM_NORMAL         0xC0
    #define CMD_SET_COM_REVERSE        0xC8
    #define CMD_RMW                    0xE0
    #define CMD_RMW_CLEAR              0xEE
    #define CMD_INTERNAL_RESET         0xE2
    #define CMD_NOP                    0xE3
    #define CMD_TEST                   0xF0
    #define CMD_SET_BOOSTER_FIRST      0xF8
    #define CMD_SET_VOLUME_SECOND      0
    #define CMD_SET_STATIC_REG         0x0
    #define CMD_SET_BOOSTER_234        0
    #define CMD_SET_BOOSTER_5          1
    #define CMD_SET_BOOSTER_6          3

    #define DIS_RE 3    //9-22
    #define DIS_DC 4   //6-25
    #define DIS_CS 5  //7-24
    #define DIS_CLK 6  //4-27
    #define DIS_SI 7   //5-26

    #define SPI_PORT spi0

    static uint8_t bg01[DISPBUFSIZE]; 

    class Display {
        private:
            uint dma_tx;
            dma_channel_config c;

            const uint8_t DC_PIN = DIS_DC;
            const uint8_t CS_PIN = DIS_CS;
            const uint8_t SCK_PIN = DIS_CLK;
            const uint8_t MOSI_PIN = DIS_SI;
            const uint8_t RST_PIN = DIS_RE;
            const uint spiBaurd=5*1000*100;
            FrameBuffer *frameBuffer;
            bool needRefresh;

        public:
            Display();
            ~Display();
            uint8_t* getFrameBufferPtr() const {
                return frameBuffer ? frameBuffer->getBuffer() : nullptr;
            }            
            void setRefresh(bool refr);
            void setupIO();
            void setPinU(uint8_t pin,uint8_t state,unsigned int len);
            void setPinM(uint8_t pin,uint8_t state,unsigned int len);
            void write_byte(uint8_t dat);
            void sendCommand(uint8_t cmd);
            void initHardware();
            void clearScr();
            void clearBg();            
            void drawPixel(uint8_t x, uint8_t y, uint8_t colour);
            void update(void);
            void clear(uint8_t c);
            void setPixel(Vec2 pos, uint8_t c, uint8_t alpha);
            void setInverted(bool inv);
            void drawBitmapRow(Vec2 pos, int width, uint8_t *c);
            void fillRect(Rect2 rect, uint8_t c, uint8_t alpha = 255);
            void hLine(Vec2 pos, int width, uint8_t c, uint8_t alpha = 255);
            void vLine(Vec2 pos, int height, uint8_t c, uint8_t alpha = 255);
            void rect(Rect2 rect, uint8_t c, uint8_t alpha = 255);
            void line(Vec2 p0, Vec2 p1, uint8_t c, uint8_t alpha = 255);
            void drawCircle(int radius, Vec2 pos, uint8_t c, uint8_t alpha = 255);
            void drawFillCircle(int radius, Vec2 pos, uint8_t c, uint8_t alpha = 255);

            void triangle(Vec2 p0, Vec2 p1, Vec2 p2, uint8_t c, uint8_t alpha = 255);
            void fillTriangle(Vec2 p0, Vec2 p1, Vec2 p2, uint8_t c, uint8_t alpha = 255);

    };

#endif