#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>  // usleep

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "lcd.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0x0000
#define COLOR_BLUE  0x111F
#define COLOR_PINK  0xF816

#define DRAW_LINE_L_D 1/(SCREEN_WIDTH * 2.0)  // resolution of position

struct
{
  bool initialized;
  uint16_t **frame_buffer;
  //ATTENTION: due to cache performance while printing buffer, frame buffer is
  //addressed in (row, column) style!!
  unsigned char *mem_base;
} display = {false, NULL, NULL};

bool lcd_init(void)
{
  display.mem_base = map_phys_address(PARLCD_REG_BASE_PHYS,
                                                 PARLCD_REG_SIZE,
                                                 0);
  if (display.mem_base == NULL) return false;
  display.frame_buffer = (uint16_t **) malloc(sizeof(uint16_t *) * SCREEN_HEIGHT);
  if (display.frame_buffer == NULL) return false;
  for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
      display.frame_buffer[i] = (uint16_t *) malloc(sizeof(uint16_t) * SCREEN_WIDTH);
      if (display.frame_buffer[i] == NULL)
        {
          for (int j = 0; j < i; j++) free(display.frame_buffer[j]);
          return false;
        }
    }
  parlcd_hx8357_init(display.mem_base);
  display.initialized = true;
  return true;
}

bool lcd_destroy(void)
{
  if (display.frame_buffer == NULL) return false;
  for (int i = 0; i < SCREEN_HEIGHT; i++) free(display.frame_buffer[i]);
  free(display.frame_buffer);
  display.frame_buffer = NULL;
  display.initialized = false;
  return true;
}

void lcd_test(uint16_t color)
{
  assert(display.initialized);
  //paint left half of the top row with pink, the rigth half of the top row with white, the rest paint with the given color
  parlcd_write_cmd(display.mem_base, 0x2c);
  for (int i = 0; i < SCREEN_WIDTH/2; i++)
    {
      parlcd_write_data(display.mem_base, COLOR_PINK);  // pink
    }
  for (int i = 0; i < SCREEN_WIDTH/2; i++)
    {
      parlcd_write_data(display.mem_base, COLOR_WHITE);  // white
    }
  for (int i = 1; i < SCREEN_HEIGHT; i++)
    {
      for (int j = 0; j < SCREEN_WIDTH; j++)
        {
          parlcd_write_data(display.mem_base, color);
        }
    }
  //wait 
  fprintf(stderr, "Falling asleep\n");
  sleep (1);  // sleep for 1 sec
  fprintf(stderr, "Waking up\n");
  //test lcd_draw_line
  disp_pos_t start = {0, 0};
  disp_pos_t end = {SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1};
  lcd_paint_buffer(COLOR_BLACK);
  lcd_draw_line(start, end, COLOR_WHITE);
  start.x = 0; start.y = SCREEN_HEIGHT - 1;
  end.x = SCREEN_WIDTH - 1; end.y = 0;
  lcd_draw_line(start, end, COLOR_PINK);
  start.x = SCREEN_WIDTH / 2 - 1; start.y = SCREEN_HEIGHT / 4 - 1;
  end.x = SCREEN_WIDTH / 2- 1; end.y = SCREEN_HEIGHT * 3 / 4 - 1;
  lcd_draw_line(start, end, COLOR_BLUE);
  lcd_print_frame_buffer();
}

void lcd_paint(uint16_t color)
{
  // paint whole display with given color
  assert(display.initialized);
  parlcd_write_cmd(display.mem_base, 0x2c);

  for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
      for (int j = 0; j < SCREEN_WIDTH; j++)
        {
          parlcd_write_data(display.mem_base, color);
        }
    }
}

void lcd_write_pixel(disp_pos_t pixel, uint16_t color)
{
  // write pixel of color color to frame_buffer
  assert(display.initialized);
  display.frame_buffer[pixel.y][pixel.x] = color;  //frame buffer is addressed (row, column)
}

void lcd_paint_buffer(uint16_t color)
{
  // paint whole frame buffer with given color 
  for (int  i = 0; i < SCREEN_HEIGHT; i++)
    {
      for (int j = 0; j < SCREEN_WIDTH; j++)
        {
          display.frame_buffer[i][j] = color;
        }
    }
}

void lcd_draw_line(disp_pos_t start_point, disp_pos_t end_point, uint16_t color)
{
  /*
 / pos.x \     / x1 \          / x2 - x1 \
 |       |  =  |    |  +  l *  |         |
 \ pos.y /     \ y1 /          \ y2 - y1 /

l is element of <0, 1>
  */
  double l = 0;  // scalar
  int x1 = start_point.x;
  int y1 = start_point.y;
  int x2 = end_point.x;
  int y2 = end_point.y;
  disp_pos_t pos;
  //int count = 0;
  while (l < 1)
    {
      /*
      count++;
      fprintf(stderr, "pos.x: %f, pos.y: %f, l: %f, count: %d\n",
              x1 + l * (x2 - x1),
              y1 + l * (y2 - y1),
              l,
              count);
      */
      pos.x = round(x1 + l * (x2 - x1));
      pos.y = round(y1 + l * (y2 - y1));
      lcd_write_pixel(pos, color);
      l += DRAW_LINE_L_D;  //TODO: variable size of DRAW_LINE_L_D (could be more effective for shorter lines
    }
}

void lcd_print_frame_buffer(void)  // send content of frame_buffer to display
{
  assert(display.initialized);
  parlcd_write_cmd(display.mem_base, 0x2c);

  for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
      for (int j = 0; j < SCREEN_WIDTH; j++)
        {
          parlcd_write_data(display.mem_base, display.frame_buffer[i][j]);
        }
    }
}

void lcd_print_from_file(FILE *file)
    /* file must be 480x320x2 Bytes long,
       expected to be of RGB565 format */
{
  for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
      for (int j = 0; j < SCREEN_WIDTH; j++)
        {
          unsigned char c1 = fgetc(file);
          unsigned char c2 = fgetc(file);
          uint16_t color = c1;
          color = color << 8;
          color = color | c2;
          disp_pos_t pos;
          pos.x = j;
          pos.y = i;
          lcd_write_pixel(pos, color);
        }
     // lcd_print_frame_buffer();
    }
  lcd_print_frame_buffer();
}
