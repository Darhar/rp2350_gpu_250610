#include "common.h"


    Sprite::Sprite(Image *im,Vec2 po,Vec2 di){
        pos=po;
        dir=di;
        image=im;  

    }

    void Sprite::draw(Display *display){
        image->drawSprite(display,0,Rect2(pos.x,pos.y,16,16),1);
    }

    void Sprite::setPosition(Vec2 po){
        pos=po;
    }
    Vec2 Sprite::getPosition(){
        return pos;
    }
    Sprite::~Sprite(){}
