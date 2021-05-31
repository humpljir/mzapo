#include "gcode.h"
#include "lcd.h"

#ifndef GUI_H
#define GUI_H

typedef enum {WIN1, WIN2, WIN3, WIN_NUM} window_t;

bool gui_start(void);

bool gui_init_file(char *filename);  // init gui for particular file

void gui_win_right(void);  // switch to window on right side from the active window (does nothing when in the rightmost one)

void gui_redraw(void);  // redraws display according to disp_state

disp_pos_t gui_map_extruder_to_disp(pos_t pos);  // maps point from printing dimensions into display

bool gui_destroy(void);

void gui_print_layer(void);  // print active layer

disp_pos_t gui_map_extruder_to_disp(pos_t pos);  // maps point from printing dimensions into display

#endif
