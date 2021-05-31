#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // sleep

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#include "gcode.h"
#include "lcd.h"

#define PENIS "./penis.RGB565.bin"

#define ARGS_ERR 100
#define FILE_ERR 101
#define MMAP_ERR 102
#define LCD_INIT_ERR 103

int main(int argc, char **argv)
{
  /*
  if (argc < 2) return ARGS_ERR;
  char *filename = argv[1];
  if (! init_file(filename)) return FILE_ERR;
  close_file();
  */
  FILE *penis = fopen(PENIS, "r");
  fprintf(stderr, "penis opened %s\n", penis ? "successfully" : "no good");
  if (! lcd_init()) return LCD_INIT_ERR;
  lcd_test(0x001F);
  sleep(2);
  lcd_print_from_file(penis);
  lcd_destroy();
  return 0;
}
