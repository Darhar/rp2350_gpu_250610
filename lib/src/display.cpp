#include "display.h"
#include <stdio.h>

Display::Display() {
    printf("[Display] driver loading...\n");
    setupIO();
	sleep_ms(100);  
    frameBuffer = new FrameBuffer(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    initHardware();
	needRefresh=true;
    printf("[Display] Done\n");
}

Display::~Display() {}

void Display::setRefresh(bool refr){
	needRefresh=refr;
}

void Display::setupIO(){

	gpio_init(DIS_CLK);
	gpio_set_dir(DIS_CLK, GPIO_OUT);
	gpio_init(DIS_SI);
	gpio_set_dir(DIS_SI, GPIO_OUT);
	gpio_init(DIS_DC);
	gpio_set_dir(DIS_DC, GPIO_OUT);
	gpio_init(DIS_CS);
	gpio_set_dir(DIS_CS, GPIO_OUT);
	gpio_init(DIS_RE);
	gpio_set_dir(DIS_RE, GPIO_OUT);

	gpio_put(DIS_DC, 0);
	gpio_put(DIS_CS, 0);
	gpio_put(DIS_RE, 0);

    spi_init(SPI_PORT, spiBaurd);
    gpio_set_function(DIS_CLK, GPIO_FUNC_SPI);
    gpio_set_function(DIS_SI, GPIO_FUNC_SPI);  

    dma_tx = dma_claim_unused_channel(true);
}

void Display::setPinU(uint8_t pin,uint8_t state,unsigned int len){
	gpio_put(pin, state);
	sleep_us(len);
}

void Display::setPinM(uint8_t pin,uint8_t state,unsigned int len){
	gpio_put(pin, state);
	sleep_ms(len);
}

void Display::write_byte(uint8_t dat) {
    spi_write_blocking(SPI_PORT, &dat, 1);
}

void Display::sendCommand(uint8_t cmd){
	gpio_put(DIS_DC, 0);  
	write_byte(cmd);
}

void Display::initHardware()
{
	setPinU(DIS_RE,1,220);
	gpio_put(DIS_CLK,1);
	gpio_put(DIS_CLK,0);
	gpio_put(DIS_CLK,1);
	gpio_put(DIS_CS,1);
	gpio_put(DIS_CS,0);
	gpio_put(DIS_CS,1);
	gpio_put(DIS_CS,0);
	setPinM(DIS_CS,1,500);
	setPinM(DIS_RE,0,20);
	setPinM(DIS_RE,1,128);
	gpio_put(DIS_CS,0);

	sendCommand(CMD_SET_ADC_REVERSE);//set display direction a1:a0 normal
	//sendCommand(0xA3);//set voltage bias
	//sendCommand(0xC8);//15-select com out scan direction -normal direction
	sendCommand(CMD_SET_DISP_NORMAL);//sets ons and offs -a7:a6 normal
	sendCommand(CMD_SET_ALLPTS_NORMAL);//display all points-a5:a4 normal display
	sendCommand(CMD_RMW);//column address increment - at write +1, at read 0
	sendCommand(0x23);//select internal resister ratio - 011
	sendCommand(CMD_SET_VOLUME_FIRST);//Electronic Volume Mode Set
	sendCommand(0x6D);//Electronic volume value 101101
	sendCommand(CMD_DISPLAY_ON);//1-display on
	sendCommand(0xAB);//?
	sendCommand(0x2F);//16-power control set- 111
	sendCommand(0x50);//Display start line set - 010000
	sendCommand(CMD_SET_COLUMN_UPPER);//set column addr most significant bits
	sendCommand(CMD_SET_COLUMN_LOWER);//set column addr least significant bits
	//sendCommand(0x30);//?
	//sendCommand(0xE4);//?
	setPinU(DIS_CS,0,32);
    gpio_put(DIS_CS,1);

    c = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));  
}

//------------------------
void Display::clearScr(){
	uint8_t nofRows=(DISPLAY_HEIGHT/8)+1;
  	for(int rowCount=0;rowCount<DISPLAY_HEIGHT;rowCount++){
		for(int byteCount=0;byteCount<DISPLAY_WIDTH;byteCount++){
			drawPixel(byteCount, rowCount, 0);
		}
  	}
}

void Display::drawPixel(uint8_t x, uint8_t y, uint8_t colour)
{
	frameBuffer->setPixel(Vec2(x,y),colour,1);
}

void Display::update(void) {
	if(needRefresh){
        gpio_put(DIS_CS,0);  

        sendCommand(CMD_SET_COLUMN_LOWER);
        sendCommand(CMD_SET_COLUMN_UPPER);
        sendCommand(CMD_SET_PAGE);
        gpio_put(DIS_DC, 1);  
        dma_channel_configure(dma_tx, &c,
            &spi_get_hw(SPI_PORT)->dr, // write address
            frameBuffer->getBuffer(), // read address
            DISPLAY_WIDTH*DISPLAY_LINES, // data transfer count
            true
        );
        dma_channel_wait_for_finish_blocking(dma_tx);
	}
}

void Display::clearBg(){
	uint8_t nofRows=(DISPLAY_HEIGHT/8)+1;
  	for(int rowCount=0;rowCount<DISPLAY_HEIGHT>>3;rowCount++){
		for(int byteCount=0;byteCount<DISPLAY_WIDTH;byteCount++){
			bg01[(rowCount*DISPLAY_WIDTH)+byteCount]=0;//(byteCount, rowCount, 0);
		}
  	}
}

void Display::clear(uint8_t c) {
    frameBuffer->clear(c);
}

void Display::setPixel(Vec2 pos, uint8_t c, uint8_t alpha) {
    frameBuffer->setPixel(pos, c, alpha);
}

void Display::setInverted(bool inv){
	frameBuffer->setInverted(inv);
}

void Display::drawBitmapRow(Vec2 pos, int width, uint8_t *c) {
    frameBuffer->drawBitmapRow(pos, width, c);
}

void Display::fillRect(Rect2 rect, uint8_t c, uint8_t alpha) {
    frameBuffer->fillRect(rect, c, alpha);
}

void Display::hLine(Vec2 pos, int width, uint8_t c, uint8_t alpha) {
    frameBuffer->hLine(pos, width, c, alpha);
}

void Display::vLine(Vec2 pos, int height, uint8_t c, uint8_t alpha) {
    frameBuffer->vLine(pos, height, c, alpha);
}

void Display::drawCircle(int radius, Vec2 pos, uint8_t c, uint8_t alpha) {
    frameBuffer->drawCircle(radius, pos, c, alpha);
}

void Display::drawFillCircle(int radius, Vec2 pos, uint8_t c, uint8_t alpha) {
    frameBuffer->drawFillCircle(radius, pos, c, alpha);
}

void Display::line(Vec2 p0, Vec2 p1, uint8_t c, uint8_t alpha) {
	frameBuffer->line(p0, p1, c, alpha);
}

void Display::rect(Rect2 rect, uint8_t c, uint8_t alpha) {
    frameBuffer->rect(rect, c, alpha);
}

void Display::triangle(Vec2 p0, Vec2 p1, Vec2 p2, uint8_t c, uint8_t alpha) {
	frameBuffer->triangle(p0, p1, p2, c, alpha);
}

void Display::fillTriangle(Vec2 p0, Vec2 p1, Vec2 p2, uint8_t c, uint8_t alpha) {
	frameBuffer->fillTriangle(p0, p1, p2, c, alpha);
}
