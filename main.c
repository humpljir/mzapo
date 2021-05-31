#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // sleep

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#include "gui.h"
#include "gcode.h"
#include "lcd.h"

#define PENIS "./penis2.bin"

#define ARGS_ERR 100
#define FILE_ERR 101
#define MMAP_ERR 102
#define INIT_ERR 103
#define GUI_START_ERR 104

int main(int argc, char **argv)
{
  
  if (argc < 2) return ARGS_ERR;
  char *filename = argv[1];
  if (! gui_start()) return GUI_START_ERR;
  if (! gui_init_file(filename)) return INIT_ERR;
  fprintf(stderr, "main(): file initialized ok\n");
  fprintf(stderr, "main(): printing layer:\n");
  gui_print_layer();
  gui_destroy();
  
  
  /*
  if (! lcd_init()) return INIT_ERR;
  lcd_print_from_file(PENIS);
  //sleep(2);
  //lcd_test(0x001F);
  lcd_destroy();
  */
  return 0;
}
