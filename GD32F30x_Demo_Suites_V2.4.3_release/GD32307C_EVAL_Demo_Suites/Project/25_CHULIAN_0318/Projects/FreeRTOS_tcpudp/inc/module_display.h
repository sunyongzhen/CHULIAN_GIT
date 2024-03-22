#ifndef _MODULE_DISPLAY_H
#define _MODULE_DISPLAY_H

// #define DRAW_PICTURE
#ifdef DRAW_PICTURE
const unsigned char display_buffer[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x0F,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x1F,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x3F,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x03,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x0F,0xFC,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x3F,0xF8,0x7F,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xFF,0xC3,0x0F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0xFF,0xC7,0x8F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x07,0xFE,0x3F,0xF1,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x07,0xFC,0x3F,0xF0,0xFF,0x80,0x03,0x00,0x00,0x00,0x30,0x60,0x00,0x00,0x00,0x00,
0x07,0xE3,0xFF,0xFF,0x1F,0x80,0x03,0x00,0x63,0xFC,0x30,0xE0,0x30,0xC4,0x08,0x60,
0x07,0xE3,0xFF,0xFF,0x8F,0x80,0xFF,0xFC,0x63,0x0C,0x30,0xC0,0xC6,0xFC,0x7C,0xF8,
0x07,0x8F,0xFC,0xFF,0xE7,0x80,0xC0,0x0C,0x63,0x0C,0xFD,0xFC,0xFF,0xC8,0xF8,0x5C,
0x07,0x8F,0xF0,0x3F,0xE7,0x80,0xC6,0x0D,0xFB,0x0C,0xCD,0x8C,0x02,0xCC,0x4F,0xE0,
0x07,0x8F,0xC0,0x0F,0xE7,0x80,0x06,0x00,0x5B,0xFC,0xCF,0x8C,0xFE,0xFC,0x0F,0xE0,
0x07,0x8F,0x80,0x07,0xE7,0x80,0x0E,0x00,0xDB,0x00,0xCD,0x0C,0xC6,0xC5,0xFF,0xFE,
0x07,0x8F,0xC0,0x0F,0xE7,0x81,0xFF,0xFE,0xDB,0x60,0xCC,0x8C,0xFE,0xDC,0xF0,0x1C,
0x07,0x8F,0xF0,0x3F,0xE7,0x80,0x18,0x70,0xDB,0x6C,0xFD,0xCC,0xC6,0xF8,0x7F,0xF8,
0x07,0x8F,0xFC,0xFF,0xE7,0x80,0x38,0x60,0xDB,0x7C,0xCC,0xCC,0xFE,0xC4,0x63,0x18,
0x07,0x8F,0xFF,0xFF,0xE7,0x80,0x3E,0xE0,0xF3,0x78,0xCC,0xEC,0xC6,0xCC,0x63,0x18,
0x07,0x8F,0xFF,0xFF,0xE7,0x80,0x0F,0xC0,0x3F,0x70,0xCC,0x6C,0xDE,0xFC,0x7F,0xF8,
0x07,0x8F,0xFF,0xFF,0xE7,0x80,0x07,0xC0,0x7E,0x64,0xCC,0x4C,0x4C,0x98,0x60,0x0E,
0x07,0x8F,0x3F,0xF3,0xE7,0x80,0x1E,0xF0,0xF6,0x66,0xCC,0x0C,0xCC,0xCC,0x60,0x0C,
0x01,0x8F,0x0F,0xC3,0xE6,0x00,0x7C,0x39,0xCE,0x6C,0xFC,0x0C,0xCC,0xCC,0x70,0x1C,
0x00,0x8F,0x03,0x03,0xE4,0x00,0xE0,0x0C,0x84,0x7C,0xCC,0xF8,0x80,0x04,0x3F,0xF8,
0x06,0x0F,0x00,0x03,0xE1,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x07,0x0F,0x00,0x03,0xC3,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x07,0xE3,0x00,0x03,0x1F,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x07,0xF1,0x00,0x02,0x3F,0x81,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,
0x07,0xFE,0x00,0x01,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x07,0xFF,0x00,0x03,0xFF,0x81,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,
0x01,0xFF,0xC0,0x0F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x7F,0xE0,0x1F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x1F,0xF8,0x7F,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x0F,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x01,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x3F,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x1F,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x07,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
#endif

void lcd_display(unsigned char row, unsigned char col, const char *string);
int display_process_cmd(int cmd_id, int * param, int paramLen);
int display_init(void);

#endif