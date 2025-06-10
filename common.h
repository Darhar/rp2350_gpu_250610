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

	typedef struct {
		float x, y, z;
	} Vector3;

	struct vec3 { 
		float x; 
		float y; 
		float z; 
	
		vec3 operator+(const vec3& other) const { 
			return { x + other.x, y + other.y, z + other.z }; 
		} 
	
		vec3 operator-(const vec3& other) const { 
			return { x - other.x, y - other.y, z - other.z }; 
		} 
	
		vec3 operator-() const { 
			return { -x, -y, -z }; 
		} 
	
		vec3 operator*(const float& scalar) const { 
			return { x * scalar, y * scalar, z * scalar }; 
		} 
	
		vec3 operator/(const float& scalar) const { 
			return { x / scalar, y / scalar, z / scalar }; 
		} 
	
		float dot(const vec3& other) const { 
			return x * other.x + y * other.y + z * other.z; 
		} 
	
		float len() const { 
			return sqrtf(this->dot(*this)); 
		} 
	
		void normalize() { 
			float len = this->len(); 
			x /= len; 
			y /= len; 
			z /= len; 
		} 
	}; 

	enum KeyReturn {NONE,SCRSELECT,SCRBACK};

	int encodeKeyReturn(KeyReturn cmd, uint8_t id);
	KeyReturn decodeKeyCommand(int packed);
	uint8_t decodeKeyID(int packed);

#endif