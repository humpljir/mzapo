#include <stdint.h>

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
void lcd_print_frame_buffer(void);  // send content of frame_buffer to display
void lcd_print_from_file(FILE *file);
