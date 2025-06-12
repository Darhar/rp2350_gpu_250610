
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "common.h"
#include"intmath.h"

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

class FrameBuffer {
    private:
        uint dmaCopyChannel;
        dma_channel_config dmaCopyConfig;
        uint dmaFillChannel;
        dma_channel_config dmaFillConfig;
        bool inverted;
        uint8_t  *buffer;

    public:

    FrameBuffer(uint8_t width, uint8_t height);
    ~FrameBuffer();
    uint8_t* getBuffer();
    void clear(uint8_t  c);
    void setPixel(Vec2 pos, uint8_t  c, uint8_t alpha);
    void setInverted(bool inv);
    void drawBitmapRow(Vec2 pos, int width, uint8_t  *c);
    void fillRect(Rect2 rect, uint8_t  c, uint8_t alpha = 255);
    void hLine(Vec2 pos, int width, uint8_t  c, uint8_t alpha = 255);
    void vLine(Vec2 pos, int height, uint8_t  c, uint8_t alpha = 255);
    void rect(Rect2 rect, uint8_t  c, uint8_t alpha = 255);
    void line(Vec2 p0, Vec2 p1, uint8_t  c, uint8_t alpha = 255);
	void drawCircle(int radius, Vec2 pos, uint8_t  c, uint8_t alpha = 255);
	void drawFillCircle(int radius, Vec2 pos, uint8_t  c, uint8_t alpha = 255);
	//void drawSphere(int radius, Vec2 pos, uint8_t  &c, uint8_t alpha = 255);
    void triangle(Vec2 p0, Vec2 p1, Vec2 p2, uint8_t  c, uint8_t alpha = 255);
    void fillTriangle(Vec2 p0, Vec2 p1, Vec2 p2, uint8_t  c, uint8_t alpha = 255);
   /* 
    */

};

#endif