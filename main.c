#include <stdio.h>
#include <stdlib.h>

#include "gcode.h"

#define ARGS_ERR 100
#define FILE_ERR 101

int main(int argc, char **argv)
{
  if (argc < 2) return ARGS_ERR;
  char *filename = argv[1];
  FILE *gcode_file = fopen(filename, "r");
  if (gcode_file == NULL) return FILE_ERR;

  //get size
  fseek(gcode_file, 0, SEEK_END);
  long filesize = ftell(gcode_file);  // in bytes
  filesize /= 1000;   // kB
  if (filesize < 1000) fprintf(stderr, "Filename: %s Size: %lu kB\n", filename, filesize);
  else fprintf(stderr, "Filename: %s Size: %g MB\n", filename, (double) filesize / 1000);
  fseek(gcode_file, 0, SEEK_SET);

  char *line = NULL;
  size_t size = 0;
  command cmd;
  while (getline(&line, &size, gcode_file) != -1)
    {
      if (parse_line(&cmd, line, size))
        {
  //        print_cmd(&cmd);
          process_cmd(&cmd);
        }
      free(line);
      line = NULL;
      size = 0;
    }
  print_stats();
  return false;
}
