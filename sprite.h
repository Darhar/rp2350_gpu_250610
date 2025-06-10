#include "common.h"
#include "display.h"

#ifndef _GAME_TIGER_SPRITE_H
#define _GAME_TIGER_SPRITE_H
    class Sprite {
        private:
            Vec2 pos;
            Vec2 dir;
            Image *image;
            Display *display;

        public:
            Sprite(Image *im,Vec2 po,Vec2 dir);
            ~Sprite();
            void draw(Display *display);
            void setPosition(Vec2 pos);
            Vec2 getPosition();
    };
#endif
