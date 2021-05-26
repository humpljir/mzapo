#include "parlcd.h"
#include <stdio.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

void parlcd_write_layer(unsigned char *parlcd_mem_base)
{
    //probably getting pointer to fst point of movement in layer as a parameter
    unsigned int lcd[SCREEN_WIDTH][SCREEN_HEIGHT]={0};
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (int j = 0; j < SCREEN_WIDTH; j++)
        {
            parlcd_write_data(parlcd_mem_base, lcd[SCREEN_WIDTH][SCREEN_HEIGHT]);
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
            unsigned int c = 0;
            parlcd_write_data(parlcd_mem_base, c);
        }
    }
}