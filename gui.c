#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>  // usleep

#include "gui.h"
#include "gcode.h"
#include "lcd.h"
#include "font_types.h"

#define DISP_RATIO 1.5  // SCREEN_WIDTH/SCREEN_HEIGHT (480/320)
#define GR_INTRO "./graphics/intro.bin"
#define GR_BG_LABEL "./graphics/bg_label.bin"
#define GR_BG_BLANK "./graphics/bg_blank.bin"

#define INTRO_TIME 6 * 1000 * 1000  // us (== 2s)


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
  //layer_t *layer;  // active layer
} disp_state = {WIN1};

void gui_win_right(void)  // switch to the window on the right side from the active window
{
  if (disp_state.active_win == WIN_NUM - 1) return;  //do nothing
  disp_state.active_win += 1;
  gui_redraw();
}

void gui_win_left(void)  // switch to the window on the left side from the active window
{
  if (disp_state.active_win == 0) return;  //do nothing
  disp_state.active_win -= 1;
  gui_redraw();
}

void gui_redraw(void)  // redraws display according to disp_state
{
}

bool gui_start(void)
{
  if (! lcd_init()) return false;
  lcd_print_from_file(GR_INTRO);
  usleep(INTRO_TIME);
  lcd_print_from_file(GR_BG_LABEL);
  disp_pos_t string_pos = {35, 95};
  lcd_print_string("Choose file:", string_pos, &font_winFreeSystem14x16, COLOR_WHITE);
  lcd_print_frame_buffer();
  return true;
}

bool gui_init_file(char *filename)  // init gui for particular file
{
  if (! gcode_init_file(filename)) return false;
  model_t *model = gcode_get_model();
  file_t *file_info = gcode_get_file_info();

  disp_state.filename = file_info->name;
  disp_state.filesize = file_info->size;

  disp_state.layer_count = model->layer_count;
  disp_state.x_size = model->x_coord_max;
  disp_state.y_size = model->y_coord_max;
  disp_state.z_size = model->z_coord_max;
  disp_state.model_ratio = disp_state.x_size / disp_state.y_size;

  disp_state.active_win = WIN2;
  disp_state.active_layer = 2;  // test
  gui_redraw();
  return true;
}

bool gui_destroy(void)
{
  gcode_close_file();
  if (! lcd_destroy()) return false;
  return true;
}

void gui_print_layer(void)  // print active layer
{
  layer_t *layer = gcode_get_layer(disp_state.active_layer);
  if (layer->length < 2)
    {
      fprintf(stderr,
              "gui_print_layer: layer %d length is %d, nothing to print\n",
              disp_state.active_layer, layer->length);
      return;
    }
  gcode_set_layer_by_num(disp_state.active_layer);
  disp_pos_t pos_start;
  disp_pos_t pos_end;
  pos_end = gui_map_extruder_to_disp(layer->points[0]);
  for (int i = 0; i < layer->length - 1; i++)
    {
      pos_start = pos_end; //gui_map_extruder_to_disp(layer->points[i]);
      pos_end = gui_map_extruder_to_disp(layer->points[i + 1]);
      lcd_draw_line(pos_start, pos_end, COLOR_PINK);
      lcd_print_frame_buffer();  // send it to buffer
    }
  gcode_free_layer(layer);
  lcd_print_frame_buffer();  // send it to buffer
}

disp_pos_t gui_map_extruder_to_disp(pos_t pos)  // maps point from printing dimensions into display
{
  disp_pos_t disp_pos;
  double scalar;  // TODO: maybe can be optimised by creating variable for scalar in disp_state
  bool tall = disp_state.model_ratio < DISP_RATIO;
  /*
    __           ______
   |  |         |      |
   |  |<- tall  |______| <- not tall
   |__|          
    
  */
  if (tall) scalar = (SCREEN_HEIGHT - 1) / disp_state.y_size;
  else scalar = (SCREEN_WIDTH - 1) / disp_state.x_size;
  disp_pos.x = round(0                 + scalar * pos.x);
  disp_pos.y = round(SCREEN_HEIGHT - 1 - scalar * pos.y);
  /*
  fprintf(stderr,
          "gui_map_extruder_to_disp(): scalar: %.5f, x: %.5f -> %5d, y: %.5f ->  %5d\n",
          scalar, pos.x, disp_pos.x, pos.y, disp_pos.y);
  */
  return disp_pos;
}
