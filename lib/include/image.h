#include "common.h"
#include "display.h"

#ifndef IMAGE_H
#define IMAGE_H

enum Em_Sprite_Data { e_spr_X,e_spr_y,e_spr_wid,e_spr_hei };

class Image
{
private:

    bool hasIndexedColors;
    uint8_t *palette;
    uint8_t *alphas;
    uint8_t *pixelData;
    uint16_t colorCount;
    uint16_t *spriteData;
    bool rawData;
    Size2 size;
    std::string ref;
    int8_t spaceX;
    bool clearSpc;

    uint16_t getSpriteX(uint16_t index);
    uint16_t getSpriteY(uint16_t index);
    void draw(Display *display, Rect2 destRect, Rect2 spriteRect, uint8_t alpha, bool flipH, bool flipV);
public:

    Image(Size2 size, uint16_t colorCount, uint8_t *palette, uint8_t *alp, uint8_t *pixelData);
    Image(Size2 size, uint16_t colorCount, uint8_t *palette, uint8_t *alp, uint8_t *pixelData, uint16_t *spriteData);
    Image(Size2 size, uint16_t colorCount, uint8_t *palette, uint8_t *alp, uint8_t *pixelData, uint16_t *spriteData,std::string re,int8_t spx);
    Image(Size2 size, uint8_t *pixelData, uint16_t *spriteData);
    ~Image();


    uint16_t getTextWidth(std::string text);
    uint16_t getSpriteWidth(uint16_t index); 
    void drawText(Display *display, std::string text, Vec2 destPos, uint8_t alpha, uint8_t scaleRatio) ;
    void setClearSpace(bool clrSpc);
    uint16_t getSpriteHeight(uint16_t index);   
    void drawSprite(Display *display, uint16_t index, Rect2 destRect, uint8_t alpha, bool flipH = false, bool flipV = false);

    /*
    void drawText(Display *display, std::string text, Vec2 destPos);   
    void drawText(Display *display, std::string text, Vec2 destPos, uint8_t alpha);

    uint16_t getTextWidth(std::string text, uint8_t scaleRatio);

    void drawSprite(Display *display, uint16_t index, Vec2 destPos, bool flipH = false, bool flipV = false);
    void drawSprite(Display *display, uint16_t index, Vec2 destPos, uint8_t alpha, bool flipH = false, bool flipV = false);
    void drawSprite(Display *display, uint16_t index, Rect2 destRect, uint8_t alpha, bool flipH = false, bool flipV = false);
    uint16_t getSpriteHeight(uint16_t index);   
 
    */

};

#endif

