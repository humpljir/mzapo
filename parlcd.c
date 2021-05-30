#include <math.h>
#include <stdio.h>

#include "parlcd.h"
#include "config.h"
#include "mzapo_parlcd.h"
#include "font_types.h"
#include "gcode.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define FONT_SPACING 0

void parlcd_clean(unsigned char *parlcd_mem_base)
{
    //paint whole screen with background color
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (int j = 0; j < SCREEN_WIDTH; j++)
        {
            uint16_t c = global.color_background;
            parlcd_write_data(parlcd_mem_base, c);
        }
    }
}

void draw_char(int x, int y, char ch, uint16_t color, font_descriptor_t *fdes)
{
    //draw char from pixels, fonts are described in font_prop14x16.c a font_rom8x16.c
    //fce prevazne prevzata z videi dr stepana
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

void draw_string(int x, int y, char string[], uint16_t color, font_descriptor_t *fdes)
{
    //call draw_char for each char in a string, currently monospace
    int i = 0;
    while (string[i] != '\0')
    {
        draw_char(x, y, string[i], color, fdes);
        x += (fdes->maxwidth + FONT_SPACING);
        i++;
    }
}

void draw_line(int x1, int y1, int x2, int y2, uint16_t color)
{
    //draw line between points... maybe
    //cela tahle funkce by sla urcite napsat lip, ale pokud bude fungovat, neresil bych to

    int c = 0, a = 0, b = 0;
    a = x2 - x1;
    b = y2 - y1;
    c = -1 * (x2 * x1 + y2 * y1); //vypocet rovnice primky ve formatu ax+by+c=0

    if (x1 == x2)
    {
        // vertical line
        if (y1 < y2)
        {
            while (y1 < y2)
            {
                draw_pixel(x1, y1, color);
                y1++; // vertical line -> just changing y1
            }
        }
        else if (y1 > y2)
        {
            while (y1 > y2)
            {
                draw_pixel(x1, y1, color);
                y1--; // vertical line -> just changing y1
            }
        }
        else
        {
            // point1 == point2
            draw_pixel(x1, y1, color);
        }
    }
    else if (x1 < x2)
    {
        while (x1 < x2)
        {
            double y_double = (-c - (a * x1)) / b; // calculate y value for each x
            draw_pixel(x1, floor(y_double), color); // if y value isn't exact number, round up and down draw two points
            draw_pixel(x1, ceil(y_double), color);
            x1++;
        }
    }
    else
    {
        while (x1 > x2)
        {
            double y_double = (-c - (a * x1)) / b; // calculate y value for each x
            draw_pixel(x1, floor(y_double), color); // if y value isn't exact number, round up and down draw two points
            draw_pixel(x1, ceil(y_double), color);
            x1--;
        }
    }
}

void parlcd_write_layer(unsigned char *parlcd_mem_base, layer_t layer, uint16_t color)
{
   // draw line between each two next points of given layer

    int i = 0;

    for (i = 0; i < layer.length; i++)
    {
        //je opravdu length o jedna vetsi, nez pocet points??
        draw_line(layer.points[i].x, layer.points[i].y, layer.points[i + 1].x, layer.points[i + 1].y, color);
    }
}

void draw_pixel(int x1, int y1, uint16_t color)  // michal
{
}
