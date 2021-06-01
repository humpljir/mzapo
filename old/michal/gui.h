typedef enum {WIN1, WIN2, WIN3, WIN_NUM} window_t;

void init_gui(char *filename,
          int filesize,
          int layer_count,
          int x_size,
          int y_size,
          int z_size);

void win_right(void);  // switch to window on right side from the active window (does nothing when in the rightmost one)

void redraw(void);  // redraws display according to disp_state
