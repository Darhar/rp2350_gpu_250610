#include "common.h"
#include "display.h"
#include "screenManager.hpp"

#ifndef KEYBOARD_H
    #define KEYBOARD_H
    #define PINRHT 6
    #define PINLFT 23
    #define PINSEL 40
    #define PINUP 54
    #define PINDWN 70
    #define butPin 29
    
    #define KEY_UP 1
    #define KEY_DOWN 2
    #define KEY_LEFT 3
    #define KEY_OK 4
    #define KEY_BACK 5

    class KeyBoard
    {
        private:
            uint8_t prevKey;

        public:
            KeyBoard();
            ~KeyBoard();

            void checkKeyState(ScreenManager* screen);
    };

#endif