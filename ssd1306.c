#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "ssd1306.h"
#include "font8x8_basic.h"



void ssd1306_display_text(SSD1306_t * dev, int page, char * text, int text_len, bool invert) {
	uint8_t seg = 0;
  uint8_t chars[8];
	for (uint8_t i = 0; i < text_len; i++) {
    memcpy(chars,font8x8_basic_tr[(uint8_t)text[i]],8);
		spi_display_text(dev, page, seg, chars);
		seg = seg + 8;
	}
}


void ssd1306_clear_screen(SSD1306_t * dev, bool invert) {
    char space[16];
    memset(space, 0x20, sizeof(space));
    for (int page = 0; page < dev->_pages; page++) {
        ssd1306_display_text(dev, page, space, sizeof(space), invert);
    }
}

void ssd1306_clear_line(SSD1306_t * dev, int page, bool invert) {
    char space[16];
    memset(space, 0x20, sizeof(space));
    ssd1306_display_text(dev, page, space, sizeof(space), invert);
}

void ssd1306_contrast(SSD1306_t * dev, int contrast) {
	if (dev->_address == SPIAddress) {
		spi_contrast(dev, contrast);
	} 
}

void ssd1306_software_scroll(SSD1306_t * dev, int start, int end) {
	
	if (start < 0 || end < 0) {
		dev->_scEnable = false;
	} else if (start >= dev->_pages || end >= dev->_pages) {
		dev->_scEnable = false;
	} else {
		dev->_scEnable = true;
		dev->_scStart = start;
		dev->_scEnd = end;
		dev->_scDirection = 1;
		if (start > end ) dev->_scDirection = -1;
		for (int i=0;i<dev->_pages;i++) {
			dev->_page[i]._valid = false;
			dev->_page[i]._segLen = 0;
		}
	}
}

/*
void ssd1306_scroll_text(SSD1306_t * dev, char * text, int text_len, bool invert) {
	
	if (dev->_scEnable == false) return;

	void (*func)(SSD1306_t * dev, int page, int seg, uint8_t images);
	if (dev->_address == SPIAddress) {
		func = spi_display_text;
	} 

	int srcIndex = dev->_scEnd - dev->_scDirection;
	while(1) {
		int dstIndex = srcIndex + dev->_scDirection;
		dev->_page[dstIndex]._valid = dev->_page[srcIndex]._valid;
		dev->_page[dstIndex]._segLen = dev->_page[srcIndex]._segLen;
		for(int seg = 0; seg < dev->_width; seg++) {
			dev->_page[dstIndex]._segs[seg] = dev->_page[srcIndex]._segs[seg];
		}
		
		if (dev->_page[dstIndex]._valid) (*func)(dev, dstIndex, 0,dev->_page[dstIndex]);
		if (srcIndex == dev->_scStart) break;
		srcIndex = srcIndex - dev->_scDirection;
	}
	
	int _text_len = text_len;
	if (_text_len > 16) _text_len = 16;
	
	uint8_t seg = 0;
	uint8_t image[8];
	for (uint8_t i = 0; i < _text_len; i++) {
		memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
		if (invert) ssd1306_invert(image, 8);
		if (dev->_flip) ssd1306_flip(image, 8);
		(*func)(dev, srcIndex, seg, image);
		for(int j=0;j<8;j++) dev->_page[srcIndex]._segs[seg+j] = image[j];
		seg = seg + 8;
	}
	dev->_page[srcIndex]._valid = true;
	dev->_page[srcIndex]._segLen = seg;
}
*/

void ssd1306_scroll_clear(SSD1306_t * dev) {
	
	if (dev->_scEnable == false) return;

	int srcIndex = dev->_scEnd - dev->_scDirection;
	while(1) {
		int dstIndex = srcIndex + dev->_scDirection;
		ssd1306_clear_line(dev, dstIndex, false);
		dev->_page[dstIndex]._valid = false;
		if (dstIndex == dev->_scStart) break;
		srcIndex = srcIndex - dev->_scDirection;
	}
}


void ssd1306_hardware_scroll(SSD1306_t * dev, ssd1306_scroll_type_t scroll) {
	if (dev->_address == SPIAddress) {
		spi_hardware_scroll(dev, scroll);
	} 
}

void ssd1306_invert(uint8_t *buf, size_t blen) {
	uint8_t wk;
	for(int i=0; i<blen; i++){
		wk = buf[i];
		buf[i] = ~wk;
	}
}

// Flip upside down
void ssd1306_flip(uint8_t *buf, size_t blen) {
	for(int i=0; i<blen; i++){
		buf[i] = ssd1306_rotate(buf[i]);
	}
}

// Rotate 8-bit data
// 0x12-->0x48
uint8_t ssd1306_rotate(uint8_t ch1) {
	uint8_t ch2 = 0;
	for (int j=0;j<8;j++) {
		ch2 = (ch2 << 1) + (ch1 & 0x01);
		ch1 = ch1 >> 1;
	}
	return ch2;
}


void ssd1306_fadeout(SSD1306_t * dev) {
	void (*func)(SSD1306_t * dev, int page, int seg, uint8_t images[8]);
	if (dev->_address == SPIAddress) {
		func = spi_display_text;
	}

	uint8_t image[8];
  for (int x=0; x<8; x++) {
    image[x] = 0XFF;
  }
	for(int page=0; page<dev->_pages; page++) {
		for(int line=0; line<8; line++) {
			if (dev->_flip) {
				image[0] = image[0] >> 1;
			} else {
				image[0] = image[0] << 1;
			}
			for(int seg=0; seg<128; seg++) {
				(*func)(dev, page, seg, image);
			}
		}
	}
}