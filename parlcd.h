#include "font_types.h"
#include "gcode.h"

#ifndef PARLCD_H
#define PARLCD_H

void parlcd_write_layer(unsigned char *parlcd_mem_base, layer_t layer, uint16_t color);  // michal
void parlcd_clean(unsigned char *parlcd_mem_base);
void draw_char(int x, int y, char ch, uint16_t color, font_descriptor_t *fdes);
void draw_string(int x, int y, char string[], uint16_t color, font_descriptor_t *fdes);
void draw_line(int x1, int y1, int x2, int y2, uint16_t color);
void draw_pixel(int x1, int y1, uint16_t color);  // michal

#endif
