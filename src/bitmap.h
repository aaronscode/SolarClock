#ifndef BITMAP_H
#define BITMAP_H

#include <Arduino.h>

// generated using https://javl.github.io/image2cpp/


// 'BackArrow', 5x7px
const unsigned char epd_bitmap_BackArrow [] PROGMEM = {
	0x18, 0x08, 0x28, 0x68, 0xf8, 0x60, 0x20
};
// 'ForwardArrow', 5x7px
const unsigned char epd_bitmap_ForwardArrow [] PROGMEM = {
	0x00, 0x20, 0x30, 0xf8, 0x30, 0x20, 0x00
};

#endif