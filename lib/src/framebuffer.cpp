#include "framebuffer.h"

FrameBuffer::FrameBuffer(uint8_t width, uint8_t height) {

    buffer = new uint8_t[width*height];
    inverted=false;

    dmaCopyChannel = dma_claim_unused_channel(true);
    dmaCopyConfig = dma_channel_get_default_config(dmaCopyChannel);
    channel_config_set_transfer_data_size(&dmaCopyConfig, DMA_SIZE_8);
    channel_config_set_read_increment(&dmaCopyConfig, true);
    channel_config_set_write_increment(&dmaCopyConfig, true);

    dmaFillChannel = dma_claim_unused_channel(true);
    dmaFillConfig = dma_channel_get_default_config(dmaFillChannel);
    channel_config_set_transfer_data_size(&dmaFillConfig, DMA_SIZE_8);
    channel_config_set_read_increment(&dmaFillConfig, false);
    channel_config_set_write_increment(&dmaFillConfig, true);
}

FrameBuffer::~FrameBuffer() {}

uint8_t* FrameBuffer::getBuffer(){
    return buffer;
}

void FrameBuffer::clear(uint8_t c) {

    static uint8_t fill_value;
    fill_value = c;    

    dma_channel_configure(dmaFillChannel, &dmaFillConfig, 
		buffer, 
		&c, 
		DISPLAY_HEIGHT*DISPLAY_WIDTH, 
		true
    );
    dma_channel_wait_for_finish_blocking(dmaFillChannel);
}

void FrameBuffer::setPixel(Vec2 pos, uint8_t c, uint8_t alpha) {

    if ( alpha==0||pos.x < 0 || 
        pos.x >= DISPLAY_WIDTH || 
        pos.y < 0 || 
        pos.y >= DISPLAY_HEIGHT)
        return;

    uint8_t bitmask = 1 << (pos.y % 8);    
    uint index = ((pos.y/8) * DISPLAY_WIDTH) + pos.x;

    if(c==0){
        if(inverted){
            buffer[index] |= bitmask; // set pixel
        }else{
            buffer[index] &= ~bitmask; // clear pixel        
        }

    }else{
        if(inverted){
            buffer[index] &= ~bitmask; // clear pixel        
        }else{
            buffer[index] |= bitmask; // set pixel
        }        
    }
}

void FrameBuffer::setInverted(bool inv){
    inverted=inv;
}

void FrameBuffer::drawBitmapRow(Vec2 pos, int width, uint8_t *c) {
    int index = (pos.y/8 * DISPLAY_WIDTH) + pos.x;
    dma_channel_configure(dmaCopyChannel, &dmaCopyConfig, 
        &buffer[index], 
        c, 
        width, 
        true
    );
    dma_channel_wait_for_finish_blocking(dmaCopyChannel);
}

void FrameBuffer::fillRect(Rect2 rect, uint8_t c, uint8_t alpha) {
    if(rect.x >= DISPLAY_WIDTH || 
        rect.y >= DISPLAY_HEIGHT || 
        rect.x + rect.w < 0 || 
        rect.y + rect.h < 0)
        return;

    rect.w = std::min(rect.w, (Index)(DISPLAY_WIDTH - rect.x));
    rect.h = std::min(rect.h, (Index)(DISPLAY_HEIGHT - rect.y));
    rect.x = std::max(rect.x, (Unit)0);
    rect.y = std::max(rect.y, (Unit)0);


    for (int i = rect.y; i < rect.y + rect.h; i++)
        for (int j = rect.x; j < rect.x + rect.w; j++)
            setPixel(Vec2(j, i), c, alpha);

}

void FrameBuffer::hLine(Vec2 pos, int width, uint8_t c, uint8_t alpha) {
    fillRect(Rect2(pos.x, pos.y, width, 1), c, alpha);
}

void FrameBuffer::vLine(Vec2 pos, int height, uint8_t c, uint8_t alpha) {
    fillRect(Rect2(pos.x, pos.y, 1, height), c, alpha);
}

void FrameBuffer::rect(Rect2 rect, uint8_t c, uint8_t alpha) {
    hLine(Vec2(rect.x, rect.y), rect.w, c, alpha);
    hLine(Vec2(rect.x, rect.y + rect.h), rect.w, c, alpha);
    vLine(Vec2(rect.x, rect.y), rect.h, c, alpha);
    vLine(Vec2(rect.x + rect.w, rect.y), rect.h, c, alpha);
}

void FrameBuffer::line(Vec2 p0, Vec2 p1, uint8_t c, uint8_t alpha) {
   	bool yLonger = false;
	int shortLen = p1.y - p0.y;
	int longLen = p1.x - p0.x;
	if(abs(shortLen) > abs(longLen)) {
		int temp = shortLen;
		shortLen = longLen;
		longLen = temp;
		yLonger = true;
	}

	int decInc = (longLen == 0) ? 0 : ((shortLen << 16) / longLen);
	if (yLonger) {
		if (longLen > 0) {
			longLen += p0.y;
			for (int j = 0x8000 + (p0.x<<16); p0.y <= longLen; ++p0.y) {
                setPixel(Vec2(j >> 16, p0.y), c, alpha);
				j += decInc;
			}
			return;
		}
		longLen += p0.y;
		for (int j = 0x8000 + (p0.x<<16); p0.y >= longLen; --p0.y) {
            setPixel(Vec2(j >> 16, p0.y), c, alpha);
			j -= decInc;
		}
		return;	
	}

	if (longLen > 0) {
		longLen += p0.x;
		for (int j = 0x8000 + (p0.y<<16); p0.x <= longLen; ++p0.x) {
            setPixel(Vec2(p0.x, j >> 16), c, alpha);
			j += decInc;
		}
		return;
	}
	longLen += p0.x;
	for (int j = 0x8000 + (p0.y<<16); p0.x >= longLen; --p0.x) {
        setPixel(Vec2(p0.x, j >> 16), c, alpha);
		j -= decInc;
	}
}

void FrameBuffer::drawCircle(int radius, Vec2 pos, uint8_t c, uint8_t alpha) {

    int x = 0, y = radius;
    int d = 3 - 2 * radius;
    setPixel(Vec2(pos.x+x, pos.y+y), c, alpha);
    setPixel(Vec2(pos.x-x, pos.y+y), c, alpha);
    setPixel(Vec2(pos.x+x, pos.y-y), c, alpha);
    setPixel(Vec2(pos.x-x, pos.y-y), c, alpha);
    setPixel(Vec2(pos.x+y, pos.y+x), c, alpha);
    setPixel(Vec2(pos.x-y, pos.y+x), c, alpha);
    setPixel(Vec2(pos.x+y, pos.y-x), c, alpha);
    setPixel(Vec2(pos.x-y, pos.y-x), c, alpha);
    while (y >= x)
    {
        x++;
        if (d > 0)
        {
            y--; 
            d = d + 4 * (x - y) + 10;
        }
        else
            d = d + 4 * x + 6;
		setPixel(Vec2(pos.x+x, pos.y+y), c, alpha);
		setPixel(Vec2(pos.x-x, pos.y+y), c, alpha);
		setPixel(Vec2(pos.x+x, pos.y-y), c, alpha);
		setPixel(Vec2(pos.x-x, pos.y-y), c, alpha);
		setPixel(Vec2(pos.x+y, pos.y+x), c, alpha);
		setPixel(Vec2(pos.x-y, pos.y+x), c, alpha);
		setPixel(Vec2(pos.x+y, pos.y-x), c, alpha);
		setPixel(Vec2(pos.x-y, pos.y-x), c, alpha);
    }
}

void FrameBuffer::drawFillCircle(int radius, Vec2 pos, uint8_t c, uint8_t alpha) {

    int x = radius, y = 0, err = -radius;
    while (x >= y)
    {
      int lastY = y;
      err += y; y++; err += y;

      //h_span(Point(c.x - x, c.y + lastY), x * 2 + 1);
	  hLine(Vec2(pos.x - x, pos.y + lastY), x * 2 + 1, c, alpha);
      if (lastY != 0) {
        //h_span(Point(c.x - x, c.y - lastY), x * 2 + 1);
		hLine(Vec2(pos.x - x, pos.y - lastY), x * 2 + 1, c, alpha);
      }
      if (err >= 0) {
        if (x != lastY) {
          //h_span(Point(c.x - lastY, c.y + x), lastY * 2 + 1);
		  hLine(Vec2(pos.x - lastY, pos.y + x), lastY * 2 + 1, c, alpha);

          if (x != 0) {
            //h_span(Point(c.x - lastY, c.y - x), lastY * 2 + 1);
			hLine(Vec2(pos.x - lastY, pos.y - x), lastY * 2 + 1, c, alpha);
          }
          err -= x; x--; err -= x;
        }
      }
    }
}


void FrameBuffer::triangle(Vec2 p0, Vec2 p1, Vec2 p2, uint8_t c, uint8_t alpha) {
    line(p0, p1, c, alpha);
    line(p1, p2, c, alpha);
    line(p2, p0, c, alpha);
}

void FrameBuffer::fillTriangle(Vec2 p0, Vec2 p1, Vec2 p2, uint8_t c, uint8_t alpha) {
    if(p0.y == p1.y && p0.y == p2.y) return;
    if(p0.y > p1.y) std::swap(p0, p1);
    if(p0.y > p2.y) std::swap(p0, p2);
    if(p1.y > p2.y) std::swap(p1, p2);

    Unit total_height = p2.y - p0.y;
    for (Unit i = 0; i < total_height; i++) { 
        bool second_half = i > p1.y-p0.y || p1.y == p0.y; 
        Unit segment_height = second_half ? p2.y-p1.y : p1.y-p0.y;
        Vec2 A = p0 + ((p2-p0) * i) / total_height;
        Vec2 B; 
        if(second_half)
            B = p1 + ((p2-p1) * (i-p1.y+p0.y)) / segment_height;
        else
            B = p0 + ((p1-p0) * i) / segment_height;

        if (A.x > B.x) std::swap(A, B); 
        hLine(Vec2(A.x, p0.y+i), B.x - A.x, c, alpha);
    } 
}
