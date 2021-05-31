#include "gcode.h"

#ifndef GUI_H
#define GUI_H

typedef enum {WIN1, WIN2, WIN3, WIN_NUM} window_t;

void init_gui(file_t *file, model_t *model);

void win_right(void);  // switch to window on right side from the active window (does nothing when in the rightmost one)

void redraw(void);  // redraws display according to disp_state

#endif
