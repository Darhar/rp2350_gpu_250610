
#include "keyboard.h"
#include "pico/stdlib.h"
#include "debug.h"
#include "screenmanager.hpp"

// Your thresholds: PINRHT, PINLFT, PINSEL, PINUP, PINDWN
// and butPin already defined somewhere in your project.

KeyBoard::KeyBoard() {
    TRACE("");
    adc_init();
    adc_gpio_init(butPin);
    adc_select_input(3);
}

KeyBoard::~KeyBoard() {}

void KeyBoard::checkKeyState(ScreenManager* sm) {
    // Quantize reading to a small bucket so it fits in one byte
    uint8_t analogReading = adc_read() / 50;
    lastAnalog = analogReading;

    uint8_t keyIndx;
    if      (analogReading < PINRHT) keyIndx = 1;
    else if (analogReading < PINLFT) keyIndx = 2;
    else if (analogReading < PINSEL) keyIndx = 3;
    else if (analogReading < PINUP ) keyIndx = 4;
    else if (analogReading < PINDWN) keyIndx = 5;
    else                             keyIndx = 0;

    if (prevKey != keyIndx) {
        TRACE("key:%d", keyIndx);
        if (keyIndx > 0) {
            sm->keyPressed(keyIndx);
            edgeBits |= ED_PRESS;
        } else {
            sm->keyReleased(prevKey); // previous key released
            edgeBits |= ED_RELEASE;
        }
        eventCount++;
    } else if (keyIndx > 0) {
        sm->keyDown(keyIndx);
    }

    currKey = keyIndx;
    stateBits = (currKey > 0) ? ST_DOWN : 0;
    prevKey = keyIndx;
}

void KeyBoard::fillReport(KeyReport& out) {
    // If you later sample keys in an ISR, wrap this with critical section.
    out.key        = currKey;
    out.stateBits  = stateBits;
    out.edgeBits   = edgeBits;
    out.analog     = lastAnalog;
    out.eventCount = eventCount;

    // clear latched edges so master gets deltas
    edgeBits = 0;
}

/*

KeyBoard::KeyBoard() {
    //TRACE("");
    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(butPin);
    // Select ADC input 3 (GPIO29)
    adc_select_input(3);
}

KeyBoard::~KeyBoard() {}

void KeyBoard::checkKeyState(ScreenManager* sm) {
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
        //TRACE("key:%d",keyIndx);
        if (keyIndx>0) { 
            sm->keyPressed(keyIndx);
        } else
            sm->keyReleased(keyIndx);
    } else if(keyIndx>0)
        sm->keyDown(keyIndx);

    prevKey = keyIndx;

}

*/

