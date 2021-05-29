#include <stdio.h>
#include <stdlib.h>

#include "gcode.h"

#define ARGS_ERR 100
#define FILE_ERR 101

int main(int argc, char **argv)
{
  if (argc < 2) return ARGS_ERR;
  char *filename = argv[1];
  if (! init_file(filename)) return FILE_ERR;
  close_file();
  return 0;
}
