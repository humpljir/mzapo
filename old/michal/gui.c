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
             **R**                 **G**                 **B**   select layer by rotating the knob                               
             *****                 *****                 *****  |                               
               *                     *                     *   /                                
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
  layer_t *layer;
} disp_state = {WIN1};

void win_right(void)  // switch to window on right side from the active window (does nothing when in the rightmost one)
{
  if (disp_state.active_win == WIN_NUM - 1) return;
  disp_state.active_win += 1;
  redraw();
}

void redraw(void)  // redraws display according to disp_state
{
}

void init_gui(char *filename,
          int filesize,
          int layer_count,
          int x_size,
          int y_size,
          int z_size)
{
  disp_state.active_win = WIN1;
  disp_state.layer_count = layer_count;
  disp_state.x_size = x_size;
  disp_state.y_size = y_size;
  disp_state.z_size = z_size;
}

