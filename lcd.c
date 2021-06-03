#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>  // usleep
#include <time.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_types.h"
#include "lcd.h"


//#define DRAW_LINE_L_D 1/(SCREEN_WIDTH * 2.0)  // resolution of position (depricated)

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

void lcd_test(uint16_t color)  // TODO: smazat urazky a nevhodne vyjevy
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
  fprintf(stderr, "Falling asleep\n");
  sleep (1);  // sleep for 1 sec
  fprintf(stderr, "Waking up\n");
  //test printing characters
  lcd_paint_buffer(COLOR_BLACK);
  disp_pos_t char_point = {10, 10};
  lcd_print_char('a', char_point, &font_winFreeSystem14x16, COLOR_WHITE);
  char_point = (disp_pos_t) {20, 10};
  lcd_print_char('G', char_point, &font_winFreeSystem14x16, COLOR_PINK);
  lcd_print_frame_buffer();
  fprintf(stderr, "Falling asleep\n");
  sleep(1);
  fprintf(stderr, "Waking up\n");
  //test printing strings
  lcd_paint_buffer(COLOR_BLACK);
  disp_pos_t string_point = {10, 10};
  lcd_print_string("Pisovi smrdi nohy", string_point, &font_winFreeSystem14x16, COLOR_WHITE);
  string_point = (disp_pos_t) {10, 30};
  lcd_print_string("Pisovi smrdi nohy", string_point, &font_winFreeSystem14x16, COLOR_WHITE);
  lcd_print_frame_buffer();
  // test getting char and string size:
  fprintf(stderr, "Testing getting char and string size\n");
  fprintf(stderr,
          "width of a is: %d, width of aaa is: %d\n",
          lcd_get_char_width('a', &font_winFreeSystem14x16),
          lcd_get_string_width("aaa", &font_winFreeSystem14x16));
  fprintf(stderr,
          "width of i is: %d, width of iii is: %d\n",
          lcd_get_char_width('i', &font_winFreeSystem14x16),
          lcd_get_string_width("iii", &font_winFreeSystem14x16));
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
  if (0 <= pixel.x && pixel.x < SCREEN_WIDTH &&
          0 <= pixel.y && pixel.y < SCREEN_HEIGHT)
    {
      display.frame_buffer[pixel.y][pixel.x] = color;  //frame buffer is addressed (row, column)
    }
  else
    {
      fprintf(stderr, "lcd_write_pixel(): warning: trying to write pixel out of display\n");
    }
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
  double l_d;
  if (x2 != x1 || y2 != y1)
    {
      l_d = 1.0/sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)); // 1/norm(vector)
    }
  else
    {
      l_d = 1;
    }
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
      l += l_d;
    }
}

void lcd_print_char(char c, disp_pos_t pos, font_descriptor_t *font, uint16_t color)
{
  assert((c >= font->firstchar) && (c - font->firstchar < font->size));
  const font_bits_t *ptr;  // pointer into font bits (font_bits_t is uint16_t)
  if (font->offset)
    {
      ptr = &font->bits[font->offset[c - font->firstchar]];
    }
  else
    {
      int bw = (font->maxwidth + 15) / 16;  // width of character in uint16-s
      ptr = &font->bits[(c - font->firstchar) * bw * font->height];
    }
  unsigned char char_width = font->width ? font->width[c - font->firstchar] : font->maxwidth;  // width of printed character
  //fprintf(stderr, "Width of character %c is %d\n", c, char_width);
  for (int i = 0; i < font->height; i++)
    {
      font_bits_t val = *ptr;
      for (int j = 0; j < char_width; j++)
        {
          if ((j % 16) == 0 && j != 0)
            {
              ptr++;
              val = *ptr;
            }
          if ((val & 0x8000) != 0)  // mask all, but the leftmost bit
            {
              lcd_write_pixel(pos, color);
            }
          pos.x++;
          val = val << 1;  // shift row to left
        }
      pos.x -= char_width;
      pos.y++;
      ptr++;
    }
}

void lcd_print_string(char *string, disp_pos_t pos, font_descriptor_t *font, uint16_t color)
{
  char c;
  int i = 0;
  while ((c = string[i]) != '\0')
    {
      assert((c >= font->firstchar) && (c - font->firstchar < font->size));
      unsigned char char_width = font->width ? font->width[c - font->firstchar] : font->maxwidth;
      lcd_print_char(c, pos, font, color);
      pos.x += char_width;
      i++;
    }
}

unsigned char lcd_get_char_width(char c, font_descriptor_t *font)
{
  assert((c >= font->firstchar) && (c - font->firstchar < font->size));
  unsigned char char_width = font->width ? font->width[c - font->firstchar] : font->maxwidth;  // width of printed character
 return char_width;
}

int lcd_get_string_width(char *string, font_descriptor_t *font)
{
  int width = 0;
  int str_index = 0;
  while (string[str_index] != '\0')
    {
      width += lcd_get_char_width(string[str_index], font);
      str_index++;
    }
  return width;
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

bool lcd_print_from_file(char *filename)
    /* file must be 480x320x2 Bytes long,
       expected to be of RGB565 format */
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
    {
      fprintf(stderr, "lcd_print_from_file(): can't open file\n");
      return false;
    }
  for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
      for (int j = 0; j < SCREEN_WIDTH; j++)
        {
          unsigned char c1 = fgetc(file);
          unsigned char c2 = fgetc(file);
          uint16_t color = c2;
          color = color << 8;
          color = color | c1;
          disp_pos_t pos;
          pos.x = j;
          pos.y = i;
          lcd_write_pixel(pos, color);
        }
      //lcd_print_frame_buffer();
    }
  lcd_print_frame_buffer();
  fclose(file);
  return true;
}
