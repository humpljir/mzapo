#include "parlcd.h"
#include "font_types.h"
#include <stdio.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define MAX_LINE 32
#define MAIN_COLOR "ffffff"
#define FONT_SPACING 0
#define BACKGROUND_COLOR "000000"

void parlcd_write_layer(unsigned char *parlcd_mem_base)
{
    //probably getting pointer to fst point of movement in layer as a parameter
    bool lcd[SCREEN_WIDTH][SCREEN_HEIGHT] = {0};
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (int j = 0; j < SCREEN_WIDTH; j++)
        {
            if (lcd[SCREEN_WIDTH][SCREEN_HEIGHT])
            {
                unsigned short c = MAIN_COLOR;
                parlcd_write_data(parlcd_mem_base, c);
            }
        }
    }
}

void draw_char(int x, int y, char ch, unsigned short color, font_descriptor_t *fdes)
{
    int w = char_width(ch);
    const font_bits_t *ptr;
    if ((ch >= fdes->firstchar) &&
        (ch - fdes->firstchar < fdes->size))
    {
        if (fdes->offset)
        {
            ptr = &fdes->bits[fdes->offset[ch - fdes->firstchar]];
        }
        else
        {
            int bw = (fdes->maxwidth + 15) / 16;
            ptr = &fdes->bits[(ch - fdes->firstchar) * bw * fdes->height];
        }
        int i, j;
        for (i = 0; i < fdes->height; i++)
        {
            font_bits_t val = *ptr;
            for (j = 0; j < w; j++)
            {
                if ((val & 0x08000) != 0)
                {
                    draw_pixel(x + j, y + i, color);
                }
            }
        }
    }
}

void draw_string(int x, int y, char string[], unsigned short color, font_descriptor_t *fdes)
{
    int i=0;
    while (string[i]!='\0')
    {
        draw_char(x,y,string[i],color,fdes);
        x+=(fdes->maxwidth+FONT_SPACING);
        i++;
    }
}

void parlcd_clean(unsigned char *parlcd_mem_base)
{
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (int j = 0; j < SCREEN_WIDTH; j++)
        {
            unsigned int c = BACKGROUND_COLOR;
            parlcd_write_data(parlcd_mem_base, c);
        }
    }
}