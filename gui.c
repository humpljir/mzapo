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

#define RED_KNOB_POS 120  // approximate x position of the red knob
#define GREEN_KNOB_POS 290  // approximate x position of the green knob
#define BLUE_KNOB_POS 420  // approximate x position of the blue knob

#define STRING_MAXWIDTH 30

/* DISPLAY SCHEME
 ________________________ ________________________ ________________________  
|                        |                        |                        |  
|                        |                        |                        |
|       MENU             |       MODEL INFO       |      LEYER RENDER      |
|                        |                        |                        |
|       proceed to file  |       x_size           |                        |
|       manual           |       y_size           |                        |
|       quit             |       z_size           |                        |
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

struct  // gui_state
{
  window_t active_win;
  int layer_count;
  int active_layer;  //index to gcode.c - model.layers[]
  char *filename;
  int filesize;
  bool file_loaded;
  double x_size;
  double y_size;
  double z_size;
  double model_ratio;  // model size ratio x_size/y_size
  double scalar;  // stores information for mapping machine choords to disp_choords
  //layer_t *layer;  // active layer
  bool quit;  // quit?
} gui_state = {WIN1};

struct  // gui_input_procedures, procedures are dependent on active_win
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

struct  // menu
{
  enum {MENU_PROCEED, MENU_MANUAL, MENU_QUIT, MENU_NUM} state;
} menu;

// input process functions ----------------------------------------------------
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

void gui_menu_up(void) // zero is top
{
  assert(gui_state.active_win == WIN1);
  usleep(200 * 1000);  // debounce rotation for 200 ms
  hw_check();
  if (menu.state == 0) return;
  menu.state--;
  print_win1();
}

void gui_menu_down(void) // zero is top
{
  assert(gui_state.active_win == WIN1);
  usleep(200 * 1000);  // debounce rotation for 200 ms
  hw_check();
  if (menu.state == MENU_NUM - 1) return;
  menu.state++;
  print_win1();
}

void gui_menu_proceed(void)
{
  assert(gui_state.active_win == WIN1);
  switch (menu.state)
    {
      case MENU_PROCEED:
          if (! gui_state.file_loaded)
            {
              if (! gui_init_file(gui_state.filename))
                {
                  fprintf(stderr, "gui_menu_proceed(): error loading file\n");
                  usleep(1*1000*1000);  // 1s
                  gui_destroy();
                  exit(200);
                }
              else
                {
                  gui_state.file_loaded = true;
                }
            }
          gui_win_right();  // go to file info
          break;
      case MENU_MANUAL:
          gui_win_manual();
          break;
      case MENU_QUIT:
          gui_set_quit();
          break;
      default:
          fprintf(stderr, "gui_menu_proceed(): unknown menu state\n");
          break;
    }
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
  if (gui_state.active_layer == gui_state.layer_count - 1) hw_write_led2(32, 32, 32);
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
  if (gui_state.active_layer == 0) hw_write_led1(32, 32, 32);
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
  if (gui_state.active_layer == gui_state.layer_count - 1) hw_write_led2(32, 32, 32);
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
  if (gui_state.active_layer == 0) hw_write_led1(32, 32, 32);
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
  if (gui_state.active_layer == gui_state.layer_count - 1) hw_write_led2(32, 32, 32);
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
  if (gui_state.active_layer == 0) hw_write_led1(32, 32, 32);
  gui_apply_state();
}

void gui_set_quit(void)
{
  gui_state.quit = true;
}

void gui_win_manual(void)
{
  assert(gui_state.active_win == WIN1);
  gui_state.active_win = WIN_MAN;
  gui_apply_state();
}

void gui_win_manual_exit(void)
{
  assert(gui_state.active_win == WIN_MAN);
  gui_state.active_win = WIN1;
  gui_apply_state();
}

void gui_apply_state(void)
{
  /*

   * IMPORTANT FUNCTION:
  set functions of individual input elements (knob-click, knob-increment, knob-decrement)
  then redraw screen for particular WIN
  (r_click and b_click are reserved for swithing windows)

  */
  switch (gui_state.active_win)
    {
      case WIN1:
          // (setting is edited in menu in win_1)
          gui_input_procedures.r_click = NULL;
          gui_input_procedures.g_click = NULL;
          gui_input_procedures.b_click = gui_menu_proceed;

          gui_input_procedures.r_incr = NULL;
          gui_input_procedures.g_incr = NULL;
          gui_input_procedures.b_incr = gui_menu_up;

          gui_input_procedures.r_decr = NULL;
          gui_input_procedures.g_decr = NULL;
          gui_input_procedures.b_decr = gui_menu_down;

          print_win1();
          break;
      case WIN2:
          gui_input_procedures.r_click = gui_win_left;
          gui_input_procedures.g_click = NULL;
          gui_input_procedures.b_click = gui_win_right;

          gui_input_procedures.r_incr = NULL;
          gui_input_procedures.g_incr = NULL;
          gui_input_procedures.b_incr = NULL;

          gui_input_procedures.r_decr = NULL;
          gui_input_procedures.g_decr = NULL;
          gui_input_procedures.b_decr = NULL;

          print_win2();
          break;
      case WIN3:
          gui_input_procedures.r_click = gui_win_left;
          gui_input_procedures.g_click = NULL;
          gui_input_procedures.b_click = gui_win_right;

          gui_input_procedures.r_incr = gui_layer_up_16x;
          gui_input_procedures.g_incr = gui_layer_up_4x;
          gui_input_procedures.b_incr = gui_layer_up;

          gui_input_procedures.r_decr = gui_layer_down_16x;
          gui_input_procedures.g_decr = gui_layer_down_4x;
          gui_input_procedures.b_decr = gui_layer_down;

          print_win3();
          break;
      case WIN_MAN:
          gui_input_procedures.r_click = NULL;
          gui_input_procedures.g_click = NULL;
          gui_input_procedures.b_click = gui_win_manual_exit;

          gui_input_procedures.r_incr = NULL;
          gui_input_procedures.g_incr = NULL;
          gui_input_procedures.b_incr = NULL;

          gui_input_procedures.r_decr = NULL;
          gui_input_procedures.g_decr = NULL;
          gui_input_procedures.b_decr = NULL;

          print_win_man();
          break;
      default:
          fprintf(stderr, "gui_apply_state(): warning: unknown window number\n");
          break;
    }
}
// end of input process functions =============================================

// calls for input procedures--------------------------------------------------
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
// end of calls for input procedures===========================================

// window print functions------------------------------------------------------
void print_win1(void)  // menu
{
  char color_line[] = {0xdb,0xdb,0xdb,0xdb,0xdb,0xdb,0xdb,0xdb,0x00};  // pozor na zakonceni
 //0xdb is full character 
  lcd_print_from_file(GR_BG_LABEL);
  disp_pos_t label_pos = {35, 95};
  lcd_print_string("MENU", label_pos, &font_winFreeSystem14x16, COLOR_PINK);
  disp_pos_t elements_pos = label_pos;
  // print color lines and text into them
  elements_pos.y += 2 * font_rom8x16.height;
  lcd_print_string(color_line, elements_pos, &font_rom8x16, menu.state == MENU_PROCEED ? COLOR_WHITE : COLOR_BLACK); 
  elements_pos.y += 1;
  lcd_print_string("PROCEED", elements_pos, &font_rom8x16, menu.state == MENU_PROCEED ? COLOR_BLACK : COLOR_WHITE); 
  elements_pos.y += -1 + font_rom8x16.height;
  lcd_print_string(color_line, elements_pos, &font_rom8x16, menu.state == MENU_MANUAL ? COLOR_WHITE : COLOR_BLACK); 
  elements_pos.y += 1;
  lcd_print_string("MANUAL", elements_pos, &font_rom8x16, menu.state == MENU_MANUAL ? COLOR_BLACK : COLOR_WHITE); 
  elements_pos.y += -1 + font_rom8x16.height;
  lcd_print_string(color_line, elements_pos, &font_rom8x16, menu.state == MENU_QUIT ? COLOR_WHITE : COLOR_BLACK); 
  elements_pos.y += 1;
  lcd_print_string("QUIT", elements_pos, &font_rom8x16, menu.state == MENU_QUIT ? COLOR_BLACK : COLOR_WHITE); 

  disp_pos_t pos;
  pos.x = BLUE_KNOB_POS;
  pos.y = SCREEN_HEIGHT - font_rom8x16.height;
  lcd_print_string("SELECT", pos, &font_rom8x16, COLOR_BLUE);
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

  pos.x = RED_KNOB_POS;
  pos.y = SCREEN_HEIGHT - font_rom8x16.height;
  lcd_print_string("MENU", pos, &font_rom8x16, COLOR_RED);

  pos.x = BLUE_KNOB_POS;
  pos.y = SCREEN_HEIGHT - font_rom8x16.height;
  lcd_print_string("LAYERS", pos, &font_rom8x16, COLOR_BLUE);
  lcd_print_frame_buffer();
}

void print_win3(void)
{
  lcd_paint_buffer(COLOR_BLACK);
  gui_print_layer();
}

void print_win_man(void)
{
  /*
  string = "then press red or blue one to move around.";
  string = "In layer mode, turn the blue knob to move";
  string = "up or down by one layer.";
  string = "use green or red knob";
*/
  lcd_print_from_file(GR_BG_LABEL);
  char *string = "MANUAL";
  disp_pos_t pos = {35, 95};
  lcd_print_string(string, pos, &font_winFreeSystem14x16, COLOR_PINK);

  pos.y += font_rom8x16.height;
  string = "Use the blue knob to navigate in the menu,";
  lcd_print_string(string, pos, &font_rom8x16, COLOR_WHITE);

  pos.y += font_rom8x16.height;
  string = "then press red or blue one to move around.";
  lcd_print_string(string, pos, &font_rom8x16, COLOR_WHITE);

  pos.y += font_rom8x16.height;
  string = "In layer mode, turn the blue knob to move";
  lcd_print_string(string, pos, &font_rom8x16, COLOR_WHITE);

  pos.y += font_rom8x16.height;
  string = "up or down by one layer.";
  lcd_print_string(string, pos, &font_rom8x16, COLOR_WHITE);

  pos.y += font_rom8x16.height;
  string = "Use green or red knob to move faster";
  lcd_print_string(string, pos, &font_rom8x16, COLOR_WHITE);

  pos.y += font_rom8x16.height;
  string = "through layers.";
  lcd_print_string(string, pos, &font_rom8x16, COLOR_WHITE);

  pos.x = BLUE_KNOB_POS;
  pos.y = SCREEN_HEIGHT - font_rom8x16.height;
  lcd_print_string("BACK", pos, &font_rom8x16, COLOR_BLUE);
  lcd_print_frame_buffer();
}
// end of window print functions===============================================

// general functions-----------------------------------------------------------
void gui_refresh_ledstrip(void)
{
  int led_shift = round(32 * (gui_state.active_layer / ((double) gui_state.layer_count - 1)));
  led_shift = 32 - led_shift;  // flip
  hw_write_ledstrip(0xFFFFFFFF << led_shift);
}

bool gui_start(char *filename)
{
  if (! lcd_init()) return false;
  if (! hw_init()) return false;
  hw_write_ledstrip(0x00000000);
  hw_write_led1(0, 0, 0);
  hw_write_led2(0, 0, 0);
  lcd_print_from_file(GR_INTRO);
  usleep(INTRO_TIME);
  gui_state.filename = filename;
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
  sprintf(text_string, "%d/%d", gui_state.active_layer + 1, gui_state.layer_count);
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

  disp_pos_t pos;
  pos.y = SCREEN_HEIGHT - font_rom8x16.height;
  pos.x = RED_KNOB_POS;
  char string[] = {0x0f, 'F', 'I', 'L', 'E', ' ',
      'I', 'N', 'F', 'O', ' ', 0x12, '1', '6', 'x', '\0'};
  lcd_print_string(string, pos, &font_rom8x16, COLOR_RED);
  pos.x = GREEN_KNOB_POS;
  char string2[] =  {0x12, '4', 'x', '\0'};
  lcd_print_string(string2 , pos, &font_rom8x16, COLOR_GREEN);
  pos.x = BLUE_KNOB_POS;
  string2[1] = '1';
  lcd_print_string(string2, pos, &font_rom8x16, COLOR_BLUE);
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
// end of general functions====================================================
