#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"

#include <math.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <map>

#define timetype uint32_t

#ifndef COMMON_H
	#define COMMON_H

	#define DISPLAY_WIDTH 168
	#define DISPLAY_HEIGHT 64
	#define DISPLAY_LINES (DISPLAY_HEIGHT/8)+1
	#define DISPBUFSIZE 1412

	#include "debug.h"
	#include "intmath.h"
	#include "logoSprite.h"          
	#include "funkV16_font.h"
	#include "areil5x8_font.h"
	#include "term6x9_font.h"
	#include "image.h"
	#include "Sprite.h"

	timetype getTime();
	uint16_t getTimeDiffMS(timetype start);

	extern Image *alphanumfont;
	extern Image *logoSprite;
	extern Image *funkyV16;
	extern Image *term6x9;
	extern Image *ariel5x8;

 	enum KeyReturn {NONE,SCRSELECT,SCRBACK};

	int encodeKeyReturn(KeyReturn cmd, uint8_t id);
	KeyReturn decodeKeyCommand(int packed);
	uint8_t decodeKeyID(int packed);
/*
*/
	static const uint I2C_SLAVE_ADDRESS = 0x17;
	static const uint I2C_BAUDRATE = 100000; // 100 kHz
	static const uint I2C_SLAVE_SDA_PIN = 12; // 4
	static const uint I2C_SLAVE_SCL_PIN = 13; // 5
	static const uint I2C_MASTER_SDA_PIN = 26;
	static const uint I2C_MASTER_SCL_PIN = 27;



#endif