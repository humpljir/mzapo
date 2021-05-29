#include "parlcd.h"
#include "mzapo_parlcd.h"
#include "font_types.h"
#include "gcode.h"
#include <math.h>
#include <stdio.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define MAX_LINE 32
#define MAIN_COLOR 999999
#define FONT_SPACING 0
#define BACKGROUND_COLOR 000000

void parlcd_write_layer(unsigned char *parlcd_mem_base, layer_t layer, unsigned short color)
{
    /*
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
    */

    int i=0;

    for (i=0;i<layer.length;i++)
    {
        draw_line(layer.points[i].x,layer.points[i].y,layer.points[i+1].x,layer.points[i+1].y,color);
    }
}

void draw_char(int x, int y, char ch, unsigned short color, font_descriptor_t *fdes)
{
    int w = fdes->maxwidth;
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
    int i = 0;
    while (string[i] != '\0')
    {
        draw_char(x, y, string[i], color, fdes);
        x += (fdes->maxwidth + FONT_SPACING);
        i++;
    }
}

void calculate_vector(int *x, int *y, int x1, int y1, int x2, int y2)
{
    *x = x2 - x1;
    *y = y2 - y1;
}

int calculate_constant(int x1, int y1, int x2, int y2)
{
    int c = -1 * (x2 * x1 + y2 * y1);
    return c;
}

    void draw_line(int x1, int y1, int x2, int y2, unsigned short color)
{
    int c = 0, a = 0, b= 0;
    calculate_vector(&a, &b, x1, y1, x2, y2);
    c = calculate_constant(a, b, x2, y2);
    if (x1 == x2)
    {
        if (y1 < y2)
        {
            while (y1 < y2)
            {
                draw_pixel(x1, y1, color);
                y1++;
            }
        }
        else if (y1 > y2)
        {
            while (y1 > y2)
            {
                draw_pixel(x1, y1, color);
                y1--;
            }
        }
        else
        {
            draw_pixel(x1, y1, color);
        }
    }
    else if (x1 < x2)
    {
        while (x1 < x2)
        {
            double y_double = (-c - (a * x1)) / b;
            draw_pixel(x1, floor(y_double), color);
            draw_pixel(x1, ceil(y_double), color);
            x1++;
        }
    }
    else
    {
        while (x1 > x2)
        {
            double y_double = (-c - (a * x1)) / b;
            draw_pixel(x1, floor(y_double), color);
            draw_pixel(x1, ceil(y_double), color);
            x1--;
        }
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
