#include "image.h"
/*
 size : size of the whole image  
 colorCount : how many colours are used in the image
 palette : the indexes of these colours
 alphas    : which indexes are transparent
 pixelData   : the pixels as colour indexes
 spriteData   : data of each image xstart,ystart,width,height
*/
Image::Image(Size2 s, uint16_t cc, uint8_t *plt, uint8_t *alp, uint8_t *pd) {
    hasIndexedColors = true;
    size = s;
    colorCount = cc;
    palette = plt;
    alphas = alp;
    pixelData = pd;
}

Image::Image(Size2 s, uint16_t cc, uint8_t *plt, uint8_t *alp, uint8_t *pd, uint16_t *sd) {
    hasIndexedColors = true;
    size = s;
    colorCount = cc;
    palette = plt;
    alphas = alp;
    pixelData = pd;
    spriteData = sd;
}

Image::Image(Size2 s, uint16_t cc, uint8_t *plt, uint8_t *alp, uint8_t *pd, uint16_t *sd,std::string re,int8_t spx) {
    hasIndexedColors = true;
    size = s;
    colorCount = cc;
    palette = plt;
    alphas = alp;
    pixelData = pd;
    spriteData = sd;
    ref=re;
    spaceX=spx;
    clearSpc=false;
}

Image::Image(Size2 s, uint8_t *pd, uint16_t *sd) {
    hasIndexedColors = false;
    size = s;
    palette = pd;
    spriteData = sd;
}
Image::~Image(){}

uint16_t Image::getTextWidth(std::string text) {
    uint16_t width = 0;
    for(char& c : text) {
        uint16_t i = ref.find(c);
        width += getSpriteWidth(i) - 1;
    }
    return width;
}
uint16_t Image::getSpriteWidth(uint16_t index) {
    return spriteData[index*4+2];
}

void Image::setClearSpace(bool clrSpc){
    clearSpc=clrSpc;
}

void Image::drawText(Display *display, std::string text, Vec2 destPos, uint8_t alpha, uint8_t scaleRatio) {
    for(char& c : text) {
        uint16_t i = ref.find(c);
        Size2 spriteSize = Size2(getSpriteWidth(i), getSpriteHeight(i));
        if(clearSpc){
            if(c != ' '){
                drawSprite(display, i, Rect2(destPos, spriteSize*scaleRatio), alpha);
            }
        }else{
            drawSprite(display, i, Rect2(destPos, spriteSize*scaleRatio), alpha);
        }
        destPos.x += (spriteSize.w+spaceX ) * scaleRatio;
    }
}

void Image::drawSprite(Display *display, uint16_t index, Rect2 destRect, uint8_t alpha, bool flipH, bool flipV) {
    Rect2 spriteRect = Rect2(
        getSpriteX(index), 
        getSpriteY(index), 
        getSpriteWidth(index), 
        getSpriteHeight(index)
    );
    draw(display, destRect, spriteRect, alpha, flipH, flipV);
}

uint16_t Image::getSpriteHeight(uint16_t index) {
    return spriteData[index*4+3];
}


uint16_t Image::getSpriteX(uint16_t index) {
    return spriteData[index*4+0];
}

uint16_t Image::getSpriteY(uint16_t index) {
    return spriteData[index*4+1];
}
void Image::draw(Display *display, Rect2 destRect, Rect2 spriteRect, uint8_t alpha, bool flipH, bool flipV) {
    if(alpha == 0 || 
        (int)destRect.x+(int)destRect.w <= 0 || 
        destRect.x >= DISPLAY_WIDTH || 
        (int)destRect.y+(int)destRect.h <= 0 || 
        destRect.y >= DISPLAY_HEIGHT)
        return;

    uint32_t xRatio = (destRect.w == spriteRect.w) ? 1 : (uint32_t)((spriteRect.w << 8) / destRect.w);
    uint32_t yRatio = (destRect.h == spriteRect.h) ? 1 : (uint32_t)((spriteRect.h << 8) / destRect.h);

    uint16_t fx = 0, fw = destRect.w;
    if(destRect.x < 0) {
        fx = abs(destRect.x) * spriteRect.w / destRect.w;
        fw += destRect.x;
        destRect.x = 0;
    }

    uint16_t fy = 0, fh = destRect.h;
    if(destRect.y < 0) {
        fy = abs(destRect.y) * spriteRect.h / destRect.h;
        fh += destRect.y;
        destRect.y = 0;
    }

    if(!hasIndexedColors && !flipH && !flipV && xRatio == yRatio == 1) {
        if(destRect.x+spriteRect.w > DISPLAY_WIDTH)
            fw -= destRect.x+spriteRect.w-DISPLAY_WIDTH;
        if(destRect.y+spriteRect.h > DISPLAY_HEIGHT)
            fh -= destRect.y+spriteRect.h-DISPLAY_HEIGHT;

        for (int y = 0; y < fh; y++) {
            int pixIndex = (fy+spriteRect.y+y) * size.w + fx+spriteRect.x;
            display->drawBitmapRow(Vec2(destRect.x, destRect.y + y), fw, &palette[pixIndex]);
        }
    } else {
        for (int y = 0; y < fh; y++) {
            for (int x = 0; x < fw; x++) {
                uint16_t cx = (xRatio == 1) ? x : ((x * xRatio) >> 8);
                uint16_t cy = (yRatio == 1) ? y : ((y * yRatio) >> 8);
                uint16_t px = (flipV ? (spriteRect.y+(fh-cy-1)) : (spriteRect.y+fy+cy));
                uint16_t py = (flipH ? (spriteRect.x+(fw-cx-1)) : (spriteRect.x+fx+cx));
                int pixIndex = px * size.w + py;
                int colIndex = hasIndexedColors ? pixelData[pixIndex] : pixIndex;
                uint8_t a = hasIndexedColors ? alphas[colIndex] : 255;
                a = a > alpha ? alpha : a;
                display->setPixel(Vec2(destRect.x + x, destRect.y + y), palette[colIndex], a);
            }
        }
    }
}
/*

void Image::draw(Display *display, Rect2 destRect, Rect2 spriteRect, uint8_t alpha, bool flipH, bool flipV) {
    if(alpha == 0 || (int)destRect.x+(int)destRect.w <= 0 || destRect.x >= DISPLAY_WIDTH || (int)destRect.y+(int)destRect.h <= 0 || destRect.y >= DISPLAY_HEIGHT)
        return;

    uint32_t xRatio = (destRect.w == spriteRect.w) ? 1 : (uint32_t)((spriteRect.w << 16) / destRect.w);
    uint32_t yRatio = (destRect.h == spriteRect.h) ? 1 : (uint32_t)((spriteRect.h << 16) / destRect.h);

    uint16_t fx = 0, fw = destRect.w;
    if(destRect.x < 0) {
        fx = abs(destRect.x) * spriteRect.w / destRect.w;
        fw += destRect.x;
        destRect.x = 0;
    }

    uint16_t fy = 0, fh = destRect.h;
    if(destRect.y < 0) {
        fy = abs(destRect.y) * spriteRect.h / destRect.h;
        fh += destRect.y;
        destRect.y = 0;
    }

    if(!this->hasIndexedColors && !flipH && !flipV && xRatio == yRatio == 1) {
        if(destRect.x+spriteRect.w > DISPLAY_WIDTH)
            fw -= destRect.x+spriteRect.w-DISPLAY_WIDTH;
        if(destRect.y+spriteRect.h > DISPLAY_HEIGHT)
            fh -= destRect.y+spriteRect.h-DISPLAY_HEIGHT;

        for (int y = 0; y < fh; y++) {
            int pixIndex = (fy+spriteRect.y+y) * this->size.w + fx+spriteRect.x;
            display->drawBitmapRow(Vec2(destRect.x, destRect.y + y), fw, &this->palette[pixIndex]);
        }
    } else {
        for (int y = 0; y < fh; y++) {
            for (int x = 0; x < fw; x++) {
                uint16_t cx = (xRatio == 1) ? x : ((x * xRatio) >> 16);
                uint16_t cy = (yRatio == 1) ? y : ((y * yRatio) >> 16);
                uint16_t px = (flipV ? (spriteRect.y+(fh-cy-1)) : (spriteRect.y+fy+cy));
                uint16_t py = (flipH ? (spriteRect.x+(fw-cx-1)) : (spriteRect.x+fx+cx));
                int pixIndex = px * this->size.w + py;
                int colIndex = this->hasIndexedColors ? this->pixelData[pixIndex] : pixIndex;
                uint8_t a = this->hasIndexedColors ? this->alphas[colIndex] : 255;
                a = a > alpha ? alpha : a;
                display->setPixel(Vec2(destRect.x + x, destRect.y + y), this->palette[colIndex], a);
            }
        }
    }
}

void Image::drawSprite(Display *display, uint16_t index, Vec2 destPos, bool flipH, bool flipV) {
    this->drawSprite(display, index, destPos, 255, flipH, flipV);
}

void Image::drawSprite(Display *display, uint16_t index, Vec2 destPos, uint8_t alpha, bool flipH, bool flipV) {
    Size2 spriteSize = Size2(this->getSpriteWidth(index), this->getSpriteHeight(index));
    this->drawSprite(display, index, Rect2(destPos, spriteSize), alpha, flipH, flipV);
}

void Image::drawText(Display *display, std::string text, Vec2 destPos) {
    this->drawText(display, text, destPos, 255);
}

void Image::drawText(Display *display, std::string text, Vec2 destPos, uint8_t alpha) {
    this->drawText(display, text, destPos, alpha, 1);
}



uint16_t Image::getTextWidth(std::string text) {
    uint16_t width = 0;
    for(char& c : text) {
        uint16_t i = ref.find(c);
        width += this->getSpriteWidth(i) - 1;
    }
    return width;
}

uint16_t Image::getTextWidth(std::string text, uint8_t scaleRatio) {
    uint16_t width = 0;
    for(char& c : text) {
        uint16_t i = ref.find(c);
        width += (this->getSpriteWidth(i) - 1)*scaleRatio;
    }
    return width;
}


*/
