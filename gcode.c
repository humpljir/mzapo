#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "gcode.h"

#define MIN_FRAME -5 //mm  // when negative not used
#define MAX_LAYERS 2000 //  supposing model will not hold more layers


machine_t machine = {0};
model_t model = {.x_coord_min = 0, .y_coord_min = 0, .z_coord_min = 0};
setting_t setting = {true, true, true};
file_t file = {NULL};

void gcode_init_cmd(command *cmd)
{
  *cmd = (command) {0};
  return;
  /*
  cmd->G.flag = false;
  cmd->G.decimal = 0;
  cmd->M.flag = false;
  cmd->M.decimal = 0;
  cmd->X.flag = false;
  cmd->X.decimal = 0;
  cmd->Y.flag = false;
  cmd->Y.decimal = 0;
  cmd->Z.flag = false;
  cmd->Z.decimal = 0;
  cmd->F.flag = false;
  cmd->F.decimal = 0;
  cmd->E.flag = false;
  cmd->E.decimal = 0;
  cmd->S.flag = false;
  cmd->S.decimal = 0;
  cmd->T.flag = false;
  cmd->T.decimal = 0;
  */
}

bool gcode_parse_line(command *cmd, char *line, int size)
{
  int index = 0;
  gcode_init_cmd(cmd);
  char *cmd_buf = (char *) malloc(sizeof(char) * size);
  if (cmd_buf == NULL) return false;
  while (line[index] != '\0' && line[index] != '\n' && line[index] != '\r' &&
         line[index] != ';')
    {
      field *field_p = NULL;
      switch (line[index])
        {
          case 'G':
              field_p = &cmd->G;
              break;
          case 'M':
              field_p = &cmd->M;
              break;
          case 'X':
              field_p = &cmd->X;
              break;
          case 'Y':
              field_p = &cmd->Y;
              break;
          case 'Z':
              field_p = &cmd->Z;
              break;
          case 'F':
              field_p = &cmd->F;
              break;
          case 'E':
              field_p = &cmd->E;
              break;
          case 'S':
              field_p = &cmd->S;
              break;
          case 'T':
              field_p = &cmd->T;
              break;
          default:
              fprintf(stderr, "Error: gcode_parse_line(): Unknown field identifier\n");
              fprintf(stderr, "field identifier: %c, neboli %d\n", line[index], line[index]);
              free(cmd_buf);
              return false;
        }
      index++;  // set index to the begining of the data
      int cmd_buf_index = 0;
      while ((line[index] >= '0' && line[index] <= '9') ||
              line[index] == '.' || line[index] == '-')
        {
          cmd_buf[cmd_buf_index++] = line[index++];
        }
      cmd_buf[cmd_buf_index] = '\0';  // end for atof()
      field_p->flag = true;  // flag is present
      if (cmd_buf_index != 0)
        {
          field_p->decimal = atof(cmd_buf);
        }
      while(line[index] == ' ') index++; // skip whitespace
    }
  //fprintf(stderr, "info: gcode_parse_line(): parsed \n");
  //fprintf(stderr, "Error: gcode_parse_line(): Unknown command number\n");
  free(cmd_buf);
  if (index == 0) return false;
  return true;
}

void gcode_process_cmd(command *cmd)  // get args and simulate machine
{
  if (cmd->G.flag)
    {
      switch ((int) cmd->G.decimal) // get needed args
        {
          case G0:  // move
          case G1:  // common for G0 and G1 //TODO: extruder movement
              gcode_move(cmd);
              break;
          case G20:
              setting.mm = false;  // -> must be inches
              break;
          case G21:
              setting.mm = true;
              break;
          case G28:  // home
              if (cmd->X.flag) machine.x_coord = 0;
              if (cmd->Y.flag) machine.y_coord = 0;
              if (cmd->Z.flag) machine.z_coord = 0;
              if (cmd->E.flag) machine.e_coord = 0;
              break;
          case G92:  //set position
              // TODO
              break;
          case G90:
              setting.absolute = true;
              break;
          case G91:
              setting.absolute = false;
              break;
        }
    }
  else if (cmd->M.flag)
    {
      switch ((int) cmd->M.decimal)
        {
          case M82:
              setting.e_absolute = true;
              break;
          case M104:
              machine.e_temp = cmd->S.decimal;
              break;
          case M106:
              machine.fan_power = cmd->S.decimal;
              break;
          case M140:
              machine.b_temp = cmd->S.decimal;
              break;
        }
    }
}

void gcode_move(command *cmd)  // expecting command to be G0 or G1 TODO: vymyslet separaci vrstev
{
  if (! (cmd->X.flag || cmd->Y.flag || cmd->Z.flag)) return;  // don't care about other than xyz movements
  float conversion_ratio = setting.mm ? 1 : 2.54;
  //model.layers[model.layer_count].length++;  //increment movements in current layer
  if (setting.absolute)
    {
      if (cmd->X.flag) machine.x_coord = cmd->X.decimal * conversion_ratio;
      if (cmd->Y.flag) machine.y_coord = cmd->Y.decimal * conversion_ratio;
      if (cmd->Z.flag && cmd->Z.decimal != machine.z_coord)
        {
          //fprintf(stderr, "layer %d done, starting new layer\n", model.layer_count);
          //fprintf(stderr, "layer_length of layer %d is: %d, starting new layer\n\n", model.layer_count, model.layers[model.layer_count].length);
          machine.z_coord = cmd->Z.decimal * conversion_ratio;
          model.layers[++model.layer_count].file_seek = ftell(file.fd);
          pos_t new_start_point;
          new_start_point.x = machine.x_coord;
          new_start_point.y = machine.y_coord;
          model.layers[model.layer_count].start_point = new_start_point;
          //fprintf(stderr, "start point of the new layer is: X: %f Y: %f\n",new_start_point.x, new_start_point.y);
        }
      else  // Z flag is not set
        {
          //model.layers[model.layer_count].length++;  //increment movements in current layer
          //fprintf(stderr, "increasing layer_count at layer num: %d\n", model.layer_count);
        }
    }
  else
    {
      if (cmd->X.flag) machine.x_coord += cmd->X.decimal * conversion_ratio;
      if (cmd->Y.flag) machine.y_coord += cmd->Y.decimal * conversion_ratio;
      if (cmd->Z.flag && cmd->Z.decimal != machine.z_coord)
        {
          //fprintf(stderr, "layer %d done, starting new layer\n", model.layer_count);
          //fprintf(stderr, "layer_length of layer %d is: %d, starting new layer\n\n", model.layer_count, model.layers[model.layer_count].length);
          machine.z_coord += cmd->Z.decimal * conversion_ratio;
          model.layers[++model.layer_count].file_seek = ftell(file.fd);
          pos_t new_start_point;
          new_start_point.x = machine.x_coord;
          new_start_point.y = machine.y_coord;
          model.layers[model.layer_count].start_point = new_start_point;
          //fprintf(stderr, "start point of the new layer is: X: %f Y: %f\n",new_start_point.x, new_start_point.y);
        }
      else
        {
          //model.layers[model.layer_count].length++;  //increment movements in current layer
          //fprintf(stderr, "increasing layer.length at layer num: %d\n", model.layer_count);
        }
    }
  if (machine.x_coord > model.x_coord_max) model.x_coord_max = machine.x_coord;
  if (machine.x_coord < model.x_coord_min && machine.x_coord > MIN_FRAME) model.x_coord_min = machine.x_coord;

  if (machine.y_coord > model.y_coord_max) model.y_coord_max = machine.y_coord;
  if (machine.y_coord < model.y_coord_min && machine.y_coord >= MIN_FRAME) model.y_coord_min = machine.y_coord;

  if (machine.z_coord > model.z_coord_max) model.z_coord_max = machine.z_coord;
  if (machine.z_coord < model.z_coord_min && machine.y_coord) model.z_coord_min = machine.z_coord;
  //if (cmd->X.flag || cmd->Y.flag || cmd->Z.flag)
    {
      //fprintf(stderr, "increasing layer.length at layer num: %d\n", model.layer_count);
      model.layers[model.layer_count].length++;  //increment movements in current layer
    }
}

void gcode_print_cmd(command *cmd)
{
  fprintf(stderr,
          "printing command: G%g M%g X%g Y%g Z%g F%g E%g S%g T%g\n",
          cmd->G.flag ? cmd->G.decimal : -1,
          cmd->M.flag ? cmd->M.decimal : -1,
          cmd->X.flag ? cmd->X.decimal : -1,
          cmd->Y.flag ? cmd->Y.decimal : -1,
          cmd->Z.flag ? cmd->Z.decimal : -1,
          cmd->F.flag ? cmd->F.decimal : -1,
          cmd->E.flag ? cmd->E.decimal : -1,
          cmd->S.flag ? cmd->S.decimal : -1,
          cmd->T.flag ? cmd->T.decimal : -1);
  /*
  fprintf(stderr,
          "command flags: G: %s M: %s X: %s Y: %s Z: %s F: %s E: %s S: %s T: %s\n",
          cmd->G.flag ? "true" : "false",
          cmd->M.flag ? "true" : "false",
          cmd->X.flag ? "true" : "false",
          cmd->Y.flag ? "true" : "false",
          cmd->Z.flag ? "true" : "false",
          cmd->F.flag ? "true" : "false",
          cmd->E.flag ? "true" : "false",
          cmd->S.flag ? "true" : "false",
          cmd->T.flag ? "true" : "false");
          */
}

void gcode_print_stats(void)
{
  if (file.size/1000 < 1000) fprintf(stderr, "Filename: %s Size: %lu kB\n", file.name, file.size);
  else fprintf(stderr, "Filename: %s Size: %g MB\n", file.name, (double) file.size / 1000000);
  fprintf(stderr, "X_min: %g X_max: %g\n", model.x_coord_min, model.x_coord_max);
  fprintf(stderr, "Y_min: %g Y_max: %g\n", model.y_coord_min, model.y_coord_max);
  fprintf(stderr, "Z_min: %g Z_max: %g\n", model.z_coord_min, model.z_coord_max);
  fprintf(stderr, "Layer count: %d\n", model.layer_count);
  fprintf(stderr, "Layer thickness: %f\n", model.z_coord_max / model.layer_count);
}

void gcode_set_default_vals(void) // TODO: vymyslet separaci vrstev
{
  machine = (machine_t){0};
  model = (model_t){.layer_count = 0, .layers[0].file_seek = 0, .x_coord_min = 1000, .y_coord_min = 1000, .z_coord_min = 1000};
  setting = (setting_t){true, true, true};
  file = (file_t){NULL};
}

bool gcode_init_file(char *filename)  //returns true on success, false on fialure
{
  gcode_set_default_vals();
  file.name = filename;
  file.fd = fopen(file.name, "r");
  if (file.fd == NULL)
    {
      return false;
    }

  char *line = NULL;
  size_t size = 0;
  command cmd; // = {.Z.flag = true, .Z.decimal = 0};  // so the first layer is initialized
  //gcode_move(&cmd);
  while (getline(&line, &size, file.fd) != -1)
    {
      //fprintf(stderr, "%s", line);
      if (gcode_parse_line(&cmd, line, size))
        {
          //print_cmd(&cmd);
          gcode_process_cmd(&cmd);
        }
      free(line);
      line = NULL;
      size = 0;
    }
  free(line);  // getline failed but line has to be freed
  file.size = ftell(file.fd);  // in bytes
  fseek(file.fd, 0, SEEK_SET);
  /*
  gcode_set_layer(&model.layers[2]);
  gcode_print_layer(&model.layers[2]);
  for (int i = 0; i < 10; i++)
    {
      fprintf(stderr, "layer: %d, move_count: %d, file seek: %ld\n", i, model.layers[i].length, model.layers[i].file_seek);
    }
  gcode_free_layer(&model.layers[2]);
  */
  gcode_print_stats();  // print info about model and file
  return true;
}

void gcode_close_file(void)
{
  fclose(file.fd);
}

bool gcode_set_layer(layer_t *layer)  // take layer->file_seek and layer->length and allocate and write points
{
  fseek(file.fd, layer->file_seek, SEEK_SET);
  layer->points = (pos_t *) malloc(sizeof(pos_t) * layer->length);
  if (layer->points == NULL)
    {
      fprintf(stderr, "Error: gcode_set_layer: failed to malloc\n");
      return false;
    }
  int move_count = 0;
  layer->points[move_count].x = layer->start_point.x;
  layer->points[move_count].y = layer->start_point.y;
  move_count++;
  command cmd;
  while (move_count < layer->length)
    {
      gcode_get_cmd(&cmd, true);  //get next move command
      layer->points[move_count].x = cmd.X.decimal;
      layer->points[move_count].y = cmd.Y.decimal;
      move_count++;
    }
  return true;
}

bool gcode_set_layer_by_num(int layer_num)  // take layer->file_seek and layer->length and allocate and write points
{
  return gcode_set_layer(&model.layers[layer_num]);
}

layer_t *gcode_get_layer(int layer_num)
{
  return &model.layers[layer_num];
}

void gcode_free_layer(layer_t *layer)
{
  if (layer->points != NULL) free(layer->points);
}

void gcode_print_layer(layer_t *layer)
{
  fprintf(stderr, "layer length: %d\n", layer->length);
  for (int i = 0; i < layer->length; i++)
    {
      fprintf(stderr, "X: %f, Y: %f\n", layer->points[i].x, layer->points[i].y);
    }
}

bool gcode_get_cmd(command *cmd, bool move_only)  // reads next command from file.fd into cmd, if move_only, writes next G1 or G0, true on success, false othrwise
{
  char *line = NULL;
  size_t size = 0;
  while (getline(&line, &size, file.fd) != -1)
    {
      if (gcode_parse_line(cmd, line, size))
        {
          if (! move_only)
            {
              free(line);
              return true;
            }
          else if (cmd->G.flag &&
                  ((int) cmd->G.decimal == G0 || (int) cmd->G.decimal == G1) &&
                  (cmd->X.flag || cmd->Y.flag))
            {
              free(line);
              return true;
            }
        }
    }
  free(line);
  return false;
}

model_t *gcode_get_model(void)
{
  return &model;
}

file_t *gcode_get_file_info(void)
{
  return &file;
}
