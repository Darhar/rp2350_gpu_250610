#include "keyboard.h"

KeyBoard::KeyBoard() {
    printf("[Keyboard] driver loading...\n");
    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(butPin);
    // Select ADC input 3 (GPIO29)
    adc_select_input(3);
    printf("[Keyboard] Done\n");
}

KeyBoard::~KeyBoard() {
}

void KeyBoard::checkKeyState(ScreenManager* screen) {
    uint8_t analogReading = adc_read()/50;
    uint8_t keyIndx;
    if(analogReading<PINRHT){
        keyIndx=1;
    }else if(analogReading<PINLFT){
        keyIndx=2;
    }else if(analogReading<PINSEL){
        keyIndx=3;
    }else if(analogReading<PINUP){
        keyIndx=4;
    }else if(analogReading<PINDWN){
        keyIndx=5;
    }else{
        keyIndx=0;
    }

    if (prevKey != keyIndx) {
        printf("key:%d\n",keyIndx);

        if (keyIndx>0) { 
            screen->keyPressed(keyIndx);
        } else
            screen->keyReleased(keyIndx);
    } else if(keyIndx>0)
        screen->keyDown(keyIndx);

    prevKey = keyIndx;

}
