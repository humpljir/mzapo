#include <assert.h>

#include "gui.h"
#include "gcode.h"


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
  int x_size;
  int y_size;
  int z_size;
  layer_t *layer;  // active layer
} disp_state = {WIN1};

void win_right(void)  // switch to the window on the right side from the active window
{
  if (disp_state.active_win == WIN_NUM - 1) return;  //do nothing
  disp_state.active_win += 1;
  redraw();
}

void win_left(void)  // switch to the window on the left side from the active window
{
  if (disp_state.active_win == 0) return;  //do nothing
  disp_state.active_win -= 1;
  redraw();
}

void redraw(void)  // redraws display according to disp_state
{
}

void init_gui(file_t *file, model_t *model)
{
  disp_state.active_win = WIN1;
  disp_state.layer_count = model->layer_count;
  disp_state.active_layer = 0;
  disp_state.x_size = model->x_coord_min;
  disp_state.y_size = model->y_coord_min;
  disp_state.z_size = model->z_coord_min;
}


