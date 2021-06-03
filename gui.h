#include "gcode.h"
#include "lcd.h"

#ifndef GUI_H
#define GUI_H

typedef enum {WIN_MAN = -1, WIN1, WIN2, WIN3, WIN_NUM} window_t;


// input process functions ----------------------------------------------------
void gui_win_right(void);  // switch to window on right side from the active window (does nothing when in the rightmost one)
void gui_win_left(void);  // switch to the window on the left side from the active window

void gui_win_manual(void);  // switch to the window with manual
void gui_win_manual_exit(void);  // switch to the window with manual

void gui_layer_up(void);
void gui_layer_down(void);
void gui_layer_up_4x(void);
void gui_layer_down_4x(void);
void gui_layer_up_16x(void);
void gui_layer_down_16x(void);

void gui_apply_state(void);  // redraws display according to disp_state
// end of input process functions =============================================


// calls for input procedures--------------------------------------------------
void gui_r_click(void);
void gui_r_incr(void);
void gui_r_decr(void);
void gui_g_click(void);
void gui_g_incr(void);
void gui_g_decr(void);
void gui_b_click(void);
void gui_b_incr(void);
void gui_b_decr(void);
void gui_set_quit(void);
// end of calls for input procedures===========================================

// window print functions------------------------------------------------------
void print_win1(void);
void print_win2(void);
void print_win3(void);
void print_win_man(void);
// end of window print functions===============================================

// general functions-----------------------------------------------------------
void gui_refresh_ledstrip(void);

bool gui_start(char *filename);

bool gui_init_file(char *filename);  // init gui for particular file

bool gui_destroy(void);

void gui_print_layer(void);  // print active layer

disp_pos_t gui_map_extruder_to_disp(pos_t pos);  // maps point from printing dimensions into display

bool gui_quit(void);
// end of general functions====================================================

#endif
