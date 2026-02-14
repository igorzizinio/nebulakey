#ifndef BITMAPS_H
#define BITMAPS_H

#include <Arduino.h>

////////////////////////////////////////////////////
// 8x8 Icons for SSD1306 (U8G2 drawXBMP compatible)
////////////////////////////////////////////////////

// ▶ PLAY
const unsigned char icon_play_8x8[] PROGMEM = {
    0x08, 0x18, 0x38, 0x78, 0x38, 0x18, 0x08, 0x00};

// ⏸ PAUSE
const unsigned char icon_pause_8x8[] PROGMEM = {
    0x66,
    0x66,
    0x66,
    0x66,
    0x66,
    0x66,
    0x66,
    0x00};

// ⏭ NEXT
const unsigned char icon_next_8x8[] PROGMEM = {
    0x12,
    0x36,
    0x7E,
    0xFE,
    0x7E,
    0x36,
    0x12,
    0x02,
};

// ⏮ PREVIOUS
const unsigned char icon_prev_8x8[] PROGMEM = {
    0x48,
    0x6C,
    0x7E,
    0x7F,
    0x7E,
    0x6C,
    0x48,
    0x40};

////////////////////////////////////////////////////
// Dimensions (useful for future scaling)
////////////////////////////////////////////////////
constexpr uint8_t ICON_8_WIDTH = 8;
constexpr uint8_t ICON_8_HEIGHT = 8;

#endif
