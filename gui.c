#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>  // usleep
#include <string.h>

#include "gui.h"
#include "gcode.h"
#include "lcd.h"
#include "font_types.h"
#include "hwio.h"

#define DISP_RATIO 1.5  // SCREEN_WIDTH/SCREEN_HEIGHT (480/320)
#define GR_INTRO "./graphics/intro.bin"
#define GR_BG_LABEL "./graphics/bg_label.bin"
#define GR_BG_BLANK "./graphics/bg_blank.bin"

#define INTRO_TIME 4 * 1000 * 1000  // us
#define OUTRO_TIME 1 * 1000 * 1000  // us

#define INNER_FRAME 1//0.9  // scale model image on screen

#define STRING_MAXWIDTH 30


/*_______________________ ________________________ ________________________  
|                        |                        |                        |  
|                        |                        |                        |
|       FILE INFO        |       MODEL INFO       |      LEYER RENDER      |
|                        |                        |                        |
|       filename         |       x_size           |                        |
|       size             |       y_size           |                        |
|                        |       z_size           |                        |
|                        |       number of layers |    (mozna) CURRENT/ALL |
|________________________|________________________|________________________|
         WIN1                       WIN2                     WIN3               

                                                                                
               *                     *                     *   \^                                 
             *****                 *****                 *****  |                               
             **R**                 **G**                 **B**   select layer by 
             *****                 *****                 *****  |rotating the knob                                                             
               *                     *                     *   / when in WIN3                  
            <------                EXIT                 ------>                   
             CLICK               ON CLICK                CLICK                     
                                                                                
 |o|o|o|o||o|o|o|o| |o|o|o|o||o|o|o|o| |o|o|o|o||o|o|o|o| |o|o|o|o||o|o|o|o| 
  ^-LED strip indicates the relative position in layers
*/

struct
{
  window_t active_win;
  int layer_count;
  int active_layer;  //index to gcode.c - model.layers[]
  char *filename;
  int filesize;
  double x_size;
  double y_size;
  double z_size;
  double model_ratio;  // model size ratio x_size/y_size
  double scalar;  // stores information for mapping machine choords to disp_choords
  //layer_t *layer;  // active layer
  bool quit;  // quit?
} gui_state = {WIN1};

struct  // procedures are dependent on active_win
{
  void (*r_click)(void);
  void (*r_incr)(void);
  void (*r_decr)(void);

  void (*g_click)(void);
  void (*g_incr)(void);
  void (*g_decr)(void);

  void (*b_click)(void);
  void (*b_incr)(void);
  void (*b_decr)(void);
} gui_input_procedures;

void gui_win_right(void)  // switch to the window on the right side from the active window
{
  if (gui_state.active_win == WIN_NUM - 1) return;  //do nothing
  gui_state.active_win += 1;
  gui_apply_state();
}

void gui_win_left(void)  // switch to the window on the left side from the active window
{
  if (gui_state.active_win == 0) return;  //do nothing
  gui_state.active_win -= 1;
  gui_apply_state();
}

void gui_layer_up(void)
{
  assert(gui_state.active_win == WIN3);
  if (gui_state.active_layer >= gui_state.layer_count - 1)
    {
      fprintf(stderr, "gui_layer_up(): already top layer can't go higher\n");
      return;
    }
  if (gui_state.active_layer == 0) hw_write_led1(0, 0, 0);
  gui_state.active_layer++;
  gui_refresh_ledstrip();
  if (gui_state.active_layer == gui_state.layer_count - 1) hw_write_led2(255, 255, 255);
  gui_apply_state();
}

void gui_layer_down(void)
{
  assert(gui_state.active_win == WIN3);
  if (gui_state.active_layer <= 0)
    {
      fprintf(stderr, "gui_layer_down(): already bottom layer can't go lower\n");
      return;
    }
  if (gui_state.active_layer == gui_state.layer_count - 1) hw_write_led2(0, 0, 0);
  gui_state.active_layer--;
  gui_refresh_ledstrip();
  if (gui_state.active_layer == 0) hw_write_led1(255, 255, 255);
  gui_apply_state();
}

void gui_layer_up_4x(void)
{
  assert(gui_state.active_win == WIN3);
  if (gui_state.active_layer >= gui_state.layer_count - 1)
    {
      fprintf(stderr, "gui_layer_up(): already top layer can't go higher\n");
      return;
    }
  if (gui_state.active_layer == 0) hw_write_led1(0, 0, 0);
  gui_state.active_layer += 4;
  if (gui_state.active_layer >= gui_state.layer_count - 1)
    {
      gui_state.active_layer = gui_state.layer_count - 1;
    }
  gui_refresh_ledstrip();
  if (gui_state.active_layer == gui_state.layer_count - 1) hw_write_led2(255, 255, 255);
  gui_apply_state();
}

void gui_layer_down_4x(void)
{
  assert(gui_state.active_win == WIN3);
  if (gui_state.active_layer <= 0)
    {
      fprintf(stderr, "gui_layer_down(): already bottom layer can't go lower\n");
      return;
    }
  if (gui_state.active_layer == gui_state.layer_count - 1) hw_write_led2(0, 0, 0);
  gui_state.active_layer -= 4;
  if (gui_state.active_layer < 0) gui_state.active_layer = 0;
  gui_refresh_ledstrip();
  if (gui_state.active_layer == 0) hw_write_led1(255, 255, 255);
  gui_apply_state();
}

void gui_layer_up_16x(void)
{
  assert(gui_state.active_win == WIN3);
  if (gui_state.active_layer >= gui_state.layer_count - 1)
    {
      fprintf(stderr, "gui_layer_up(): already top layer can't go higher\n");
      return;
    }
  if (gui_state.active_layer == 0) hw_write_led1(0, 0, 0);
  gui_state.active_layer += 16;
  if (gui_state.active_layer >= gui_state.layer_count - 1)
    {
      gui_state.active_layer = gui_state.layer_count - 1;
    }
  gui_refresh_ledstrip();
  if (gui_state.active_layer == gui_state.layer_count - 1) hw_write_led2(255, 255, 255);
  gui_apply_state();
}

void gui_layer_down_16x(void)
{
  assert(gui_state.active_win == WIN3);
  if (gui_state.active_layer <= 0)
    {
      fprintf(stderr, "gui_layer_down(): already bottom layer can't go lower\n");
      return;
    }
  if (gui_state.active_layer == gui_state.layer_count - 1) hw_write_led2(0, 0, 0);
  gui_state.active_layer -= 16;
  if (gui_state.active_layer < 0) gui_state.active_layer = 0;
  gui_refresh_ledstrip();
  if (gui_state.active_layer == 0) hw_write_led1(255, 255, 255);
  gui_apply_state();
}

void gui_refresh_ledstrip(void)
{
  int led_shift = round(32 * (gui_state.active_layer / ((double) gui_state.layer_count - 1)));
  led_shift = 32 - led_shift;  // flip
  hw_write_ledstrip(0xFFFFFFFF << led_shift);
}

void gui_apply_state(void)  // redraws display according to gui_state
{
  //set functions of individual input elements (knob-click, knob-increment, knob-decrement)
  //then redraw screen for particular WIN
  switch (gui_state.active_win)
    {
      case WIN1:
          gui_input_procedures.g_click = gui_set_quit;  // quit when green button pressed
          fprintf(stderr, "gui_apply_state(): WIN1 not yet done\n");
          print_win1();
          break;
      case WIN2:
          gui_input_procedures.g_click = NULL;

          gui_input_procedures.r_incr = NULL;
          gui_input_procedures.g_incr = NULL;
          gui_input_procedures.b_incr = NULL;

          gui_input_procedures.r_decr = NULL;
          gui_input_procedures.g_decr = NULL;
          gui_input_procedures.b_decr = NULL;
          print_win2();
          break;
      case WIN3:
          gui_input_procedures.g_click = NULL;

          gui_input_procedures.r_incr = gui_layer_up_16x;
          gui_input_procedures.g_incr = gui_layer_up_4x;
          gui_input_procedures.b_incr = gui_layer_up;

          gui_input_procedures.r_decr = gui_layer_down_16x;
          gui_input_procedures.g_decr = gui_layer_down_4x;
          gui_input_procedures.b_decr = gui_layer_down;
          print_win3();
          break;
      default:
          fprintf(stderr, "gui_apply_state(): warning: unknown window number\n");
          break;
    }
}

bool gui_start(void)
{
  if (! lcd_init()) return false;
  if (! hw_init()) return false;
  hw_write_ledstrip(0x00000000);
  hw_write_led1(0, 0, 0);
  hw_write_led2(0, 0, 0);
  lcd_print_from_file(GR_INTRO);
  usleep(INTRO_TIME);
  gui_input_procedures.r_click = gui_win_left;  // always
  gui_input_procedures.b_click = gui_win_right; // always
  gui_state.active_win = WIN1;
  gui_apply_state();
  return true;
}

bool gui_init_file(char *filename)  // init gui for particular file
{
  lcd_print_from_file(GR_BG_LABEL);
  disp_pos_t pos = {100, 100};
  lcd_print_string("LOADING FILE...", pos, &font_wArial_44, COLOR_WHITE);
  lcd_print_frame_buffer();
  if (! gcode_init_file(filename))
    { 
      pos.y += font_wArial_44.height;
      lcd_print_string("FAILED", pos, &font_wArial_44, COLOR_WHITE);
      lcd_print_frame_buffer();
      return false;
    }
  model_t *model = gcode_get_model();
  file_t *file_info = gcode_get_file_info();

  gui_state.filename = file_info->name;
  gui_state.filesize = file_info->size;

  gui_state.layer_count = model->layer_count;
  gui_state.x_size = model->x_coord_max;
  gui_state.y_size = model->y_coord_max;
  gui_state.z_size = model->z_coord_max;
  gui_state.model_ratio = gui_state.x_size / gui_state.y_size;
  /*
    __           ______
   |  |         |      |
   |  |<- tall  |______| <- not tall
   |__|          
    
  */
  bool tall = gui_state.model_ratio < DISP_RATIO;
  if (tall) gui_state.scalar = (SCREEN_HEIGHT - 1) / gui_state.y_size;
  else gui_state.scalar = (SCREEN_WIDTH - 1) / gui_state.x_size;
  gui_state.scalar = gui_state.scalar * INNER_FRAME;  //TODO: play with this

  gui_state.active_win = WIN2;
  gui_state.active_layer = 0;  // test
  while (gui_state.active_layer < gui_state.layer_count &&
              gcode_get_layer(gui_state.active_layer)->length < 2)
    {
      fprintf(stderr, "gui_init_file(): shifting to next layer\n");
      gui_state.active_layer++;  // set active_layer to point on the first printable layer
    }
  gui_apply_state();
  return true;
}

bool gui_destroy(void)
{
  hw_write_ledstrip(0x00000000);
  hw_write_led1(0, 0, 0);
  hw_write_led2(0, 0, 0);
  lcd_print_from_file(GR_INTRO);
  usleep(OUTRO_TIME);
  lcd_paint_buffer(COLOR_BLACK);
  lcd_print_frame_buffer();

  gcode_close_file();
  if (! lcd_destroy()) return false;
  return true;
}

void gui_print_layer(void)  // print active layer
{
  layer_t *layer = gcode_get_layer(gui_state.active_layer);

  //print active layer num
  disp_pos_t text_pos = {400, 10};
  char text_string[STRING_MAXWIDTH];
  sprintf(text_string, "%d/%d", gui_state.active_layer, gui_state.layer_count - 1);
  lcd_print_string(text_string, text_pos, &font_winFreeSystem14x16, COLOR_WHITE);

  if (layer->length < 2)
    {
      fprintf(stderr,
              "gui_print_layer: layer %d length is %d, nothing to print\n",
              gui_state.active_layer, layer->length);
      text_pos = (disp_pos_t) {10, 10};
      sprintf(text_string, "no print data for this layer");
      lcd_print_string(text_string, text_pos, &font_winFreeSystem14x16, COLOR_WHITE);
      lcd_print_frame_buffer();  // send it to display
      return;
    }
  gcode_set_layer_by_num(gui_state.active_layer);
  disp_pos_t pos_start;
  disp_pos_t pos_end;
  pos_end = gui_map_extruder_to_disp(layer->points[0]);
  for (int i = 0; i < layer->length - 1; i++)
    {
      pos_start = pos_end; //gui_map_extruder_to_disp(layer->points[i]);
      pos_end = gui_map_extruder_to_disp(layer->points[i + 1]);
      lcd_draw_line(pos_start, pos_end, COLOR_PINK);
      //lcd_print_frame_buffer();  // send it to buffer
    }
  gcode_free_layer(layer);
  lcd_print_frame_buffer();  // send it to display
}

disp_pos_t gui_map_extruder_to_disp(pos_t pos)  // maps point from printing dimensions into display
{
  disp_pos_t disp_pos;
  disp_pos.x = round(0                 + gui_state.scalar * pos.x);
  disp_pos.y = round(SCREEN_HEIGHT - 1 - gui_state.scalar * pos.y);
  /*
  fprintf(stderr,
          "gui_map_extruder_to_disp(): scalar: %.5f, x: %.5f -> %5d, y: %.5f ->  %5d\n",
          scalar, pos.x, disp_pos.x, pos.y, disp_pos.y);
  */
  return disp_pos;
}

bool gui_quit(void)
{
  return gui_state.quit;
}

//input procedures

void gui_r_click(void)
{
  if (gui_input_procedures.r_click) gui_input_procedures.r_click();
}
void gui_r_incr(void)
{
  if (gui_input_procedures.r_incr) gui_input_procedures.r_incr();
}
void gui_r_decr(void)
{
  if (gui_input_procedures.r_decr) gui_input_procedures.r_decr();
}
void gui_g_click(void)
{
  if (gui_input_procedures.g_click) gui_input_procedures.g_click();
}
void gui_g_incr(void)
{
  if (gui_input_procedures.g_incr) gui_input_procedures.g_incr();
}
void gui_g_decr(void)
{
  if (gui_input_procedures.g_decr) gui_input_procedures.g_decr();
}
void gui_b_click(void)
{
  if (gui_input_procedures.b_click) gui_input_procedures.b_click();
}
void gui_b_incr(void)
{
  if (gui_input_procedures.b_incr) gui_input_procedures.b_incr();
}
void gui_b_decr(void)
{
  if (gui_input_procedures.b_decr) gui_input_procedures.b_decr();
}

void gui_set_quit(void)
{
  gui_state.quit = true;
}

void print_win1(void)
{
  lcd_print_from_file(GR_BG_LABEL);
  disp_pos_t pos = {60, 100};
  lcd_print_string("WIN1 NOT DONE, YET", pos, &font_wArial_44, COLOR_WHITE);
  lcd_print_frame_buffer();
}
void print_win2(void)
{
  lcd_print_from_file(GR_BG_LABEL);
  char string[STRING_MAXWIDTH];
  disp_pos_t pos = {35, 95};  // aligned with label

  sprintf(string, "FILENAME: %s", gui_state.filename);
  lcd_print_string(string, pos, &font_winFreeSystem14x16, COLOR_WHITE);
  pos.y += font_winFreeSystem14x16.height;

  sprintf(string, "FILESIZE: %d kB", gui_state.filesize / 1000);
  lcd_print_string(string, pos, &font_winFreeSystem14x16, COLOR_WHITE);
  pos.y += font_winFreeSystem14x16.height;

  sprintf(string, "MODEL:");
  lcd_print_string(string, pos, &font_winFreeSystem14x16, COLOR_WHITE);
  pos.y += font_winFreeSystem14x16.height;

  sprintf(string, "   X SIZE: %.3f mm", gui_state.x_size);
  lcd_print_string(string, pos, &font_winFreeSystem14x16, COLOR_WHITE);
  pos.y += font_winFreeSystem14x16.height;

  sprintf(string, "   Y SIZE: %.3f mm", gui_state.y_size);
  lcd_print_string(string, pos, &font_winFreeSystem14x16, COLOR_WHITE);
  pos.y += font_winFreeSystem14x16.height;

  sprintf(string, "   Z SIZE: %.3f mm", gui_state.z_size);
  lcd_print_string(string, pos, &font_winFreeSystem14x16, COLOR_WHITE);
  pos.y += font_winFreeSystem14x16.height;

  sprintf(string, "   NUMBER OF LAYERS: %d", gui_state.layer_count);
  lcd_print_string(string, pos, &font_winFreeSystem14x16, COLOR_WHITE);
  pos.y += font_winFreeSystem14x16.height;
//TODO: delete s_filesize
  lcd_print_frame_buffer();
}
void print_win3(void)
{
  lcd_paint_buffer(COLOR_BLACK);
  gui_print_layer();
}
