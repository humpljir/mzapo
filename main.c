#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // sleep

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#include "gui.h"
#include "gcode.h"
#include "lcd.h"
#include "hwio.h"

#define PENIS "./penis2.bin"

#define ARGS_ERR 100
#define FILE_ERR 101
#define MMAP_ERR 102
#define INIT_ERR 103
#define GUI_START_ERR 104
#define HW_INIT_ERR 105

#define POLL_SLEEP 10 * 1000  // 10 ms

enum  // keyboard input mapping
{
  KB_R_CLICK = 'b',
  KB_R_INCR  = 't',  // increment
  KB_R_DECR  = 'g',  // decrement
  KB_G_CLICK = 'n',
  KB_G_INCR  = 'y',  // increment
  KB_G_DECR  = 'h',  // decrement
  KB_B_CLICK = 'm', 
  KB_B_INCR  = 'u',  // increment
  KB_B_DECR  = 'j',  // decrement
};

int main(int argc, char **argv)
{
  if (argc < 2) return ARGS_ERR;
  char *filename = argv[1];
  if (! gui_start(filename))
    {
      fprintf(stderr, "NO FILE GIVEN");
      return GUI_START_ERR;
    }
  //if (! gui_init_file(filename)) return INIT_ERR;   // still tests, probably will be called by gui.c
  bool quit = false;
  while (! quit)
    {
      //fprintf(stderr, "main(): hw_checking\n");
      hw_check();
      if (hw_r_released())
        {
          gui_r_click();
        }
      if (hw_g_released())
        {
          gui_g_click();
        }
      if (hw_b_released())
        {
          gui_b_click();
        }
      if (hw_r_incremented())
        {
          gui_r_incr();
        }
      if (hw_g_incremented())
        {
          gui_g_incr();
        }
      if (hw_b_incremented())
        {
          gui_b_incr();
        }
      if (hw_r_decremented())
        {
          gui_r_decr();
        }
      if (hw_g_decremented())
        {
          gui_g_decr();
        }
      if (hw_b_decremented())
        {
          gui_b_decr();
        }
      if (gui_quit()) quit = true;
      usleep(POLL_SLEEP);
    }
  gui_destroy();
  /*
  if (argc < 2) return ARGS_ERR;
  char *filename = argv[1];
  if (! hw_init()) return HW_INIT_ERR;
  if (! gui_start()) return GUI_START_ERR;
  if (! gui_init_file(filename)) return INIT_ERR;   // still tests, probably will be called by gui.c
  char c;
  while ((c = getchar()) != 'q')
    {
      switch (c)
        {
          case KB_R_CLICK:
              fprintf(stderr, "main(): signal r_click received from keyboard\n");
              gui_r_click();
              break;
          case KB_R_INCR:
              fprintf(stderr, "main(): signal r_incr received from keyboard\n");
              gui_r_incr();
              break;
          case KB_R_DECR:
              fprintf(stderr, "main(): signal r_decr received from keyboard\n");
              gui_r_decr();
              break;
          case KB_G_CLICK:
              hw_check();
              fprintf(stderr, "main(): signal g_click received from keyboard\n");
              gui_g_click();
              break;
          case KB_G_INCR:
              fprintf(stderr, "main(): signal g_incr received from keyboard\n");
              gui_g_incr();
              break;
          case KB_G_DECR:
              fprintf(stderr, "main(): signal g_decr received from keyboard\n");
              gui_g_decr();
              break;
          case KB_B_CLICK:
              fprintf(stderr, "main(): signal b_click received from keyboard\n");
              gui_b_click();
              break;
          case KB_B_INCR:
              fprintf(stderr, "main(): signal b_incr received from keyboard\n");
              gui_b_incr();
              break;
          case KB_B_DECR:
              fprintf(stderr, "main(): signal b_decr received from keyboard\n");
              gui_b_decr();
              break;
          case '\n':
              break;
          default:
              fprintf(stderr, "main(): no function assigned to %c\n", c);
              break;
        }
    }

  gui_destroy();
  */
  /*
  hw_init();
  hw_test_out();
  */
  /*
  if (! lcd_init()) return INIT_ERR;
  lcd_print_from_file(PENIS);
  //sleep(2);
  //lcd_test(0x001F);
  lcd_destroy();
  */
  return 0;
}
