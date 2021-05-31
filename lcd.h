#include <stdint.h>
#include "font_types.h"

#ifndef LCD_H
#define LCD_H

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0x0000
#define COLOR_RED   0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE  0x111F
#define COLOR_PINK  0xF816

typedef struct
{
  int x;  // x coord (starting on the left)
  int y;  // y coord (starting on the top)
} disp_pos_t;

bool lcd_init(void);
bool lcd_destroy(void);
void lcd_test(uint16_t color);
void lcd_paint(uint16_t color);
void lcd_write_pixel(disp_pos_t pixel, uint16_t color);
void lcd_paint_buffer(uint16_t color);
void lcd_draw_line(disp_pos_t start_point, disp_pos_t end_point, uint16_t color);
void lcd_print_char(char c, disp_pos_t pos, font_descriptor_t *font, uint16_t color);
void lcd_print_string(char *string, disp_pos_t pos, font_descriptor_t *font, uint16_t color);
void lcd_print_frame_buffer(void);  // send content of frame_buffer to display
bool lcd_print_from_file(char *filename);

#endif
