#ifndef PARLCD_H
#define PARLCD_H

void parlcd_write_layer(unsigned char *parlcd_mem_base);
void parlcd_clean(unsigned char *parlcd_mem_base);
void draw_char(int x, int y, char ch, unsigned short color, font_descriptor_t fdes);

#endif