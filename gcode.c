#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gcode.h"

struct
{
  double x_coord;  // coords in mm
  double y_coord;
  double z_coord;
  double e_coord;
  double e_temp;  // extruder temperature
  double b_temp;  // bed temperature
  double fan_power;  // is fan turned on

  double x_coord_min;  // to be set while parsing whole gcode file
  double y_coord_min;
  double z_coord_min;

  double x_coord_max;  // to be set while parsing whole gcode file
  double y_coord_max;
  double z_coord_max;
} machine = {0, 0, 0, 0, 0};

struct
{
  bool mm;  // true if set to mm, false if set to inches
  bool absolute;   // true if receiving absolute coords
  bool e_absolute;   // true if receiving absolute extruder coords
} setting = {true, true, true};

void init_cmd(command *cmd)
{
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
}

bool parse_line(command *cmd, char *line, int size)
{
  int index = 0;
  init_cmd(cmd);
  char *cmd_buf = (char *) malloc(sizeof(char) * size);
  if (cmd_buf == NULL) return false;
  while (line[index] != '\0' && line[index] != '\n' && line[index] != '\r' &&
         line[index] != ';')
    {
      field *field_p = NULL;
      //fprintf(stderr, "field identifier: %c, neboli %d\n", line[index], line[index]);
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
          default:
              fprintf(stderr, "Error: parse_line(): Unknown field identifier\n");
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
  //fprintf(stderr, "info: parse_line(): parsed \n");
  //fprintf(stderr, "Error: parse_line(): Unknown command number\n");
  free(cmd_buf);
  if (index == 0) return false;
  return true;
}


void process_cmd(command *cmd)  // get args and simulate machine
{
  if (cmd->G.flag)
    {
      switch ((int) cmd->G.decimal) // get needed args
        {
          case G0:  // move
          case G1:  // common for G0 and G1 //TODO: extruder movement
              move(cmd);
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
          case G92:
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

void move(command *cmd)  // expecting command to be G0 or G1
{
  float conversion_ratio = setting.mm ? 1 : 2.54;
  if (setting.absolute)
    {
      if (cmd->X.flag) machine.x_coord = cmd->X.decimal * conversion_ratio;
      if (cmd->Y.flag) machine.y_coord = cmd->Y.decimal * conversion_ratio;
      if (cmd->Z.flag) machine.z_coord = cmd->Z.decimal * conversion_ratio;
    }
  else
    {
      if (cmd->X.flag) machine.x_coord += cmd->X.decimal * conversion_ratio;
      if (cmd->Y.flag) machine.y_coord += cmd->Y.decimal * conversion_ratio;
      if (cmd->Z.flag) machine.z_coord += cmd->Z.decimal * conversion_ratio;
    }
  if (machine.x_coord > machine.x_coord_max) machine.x_coord_max = machine.x_coord;
  if (machine.x_coord < machine.x_coord_min) machine.x_coord_min = machine.x_coord;

  if (machine.y_coord > machine.y_coord_max) machine.y_coord_max = machine.y_coord;
  if (machine.y_coord < machine.y_coord_min) machine.y_coord_min = machine.y_coord;

  if (machine.z_coord > machine.z_coord_max) machine.z_coord_max = machine.z_coord;
  if (machine.z_coord < machine.z_coord_min) machine.z_coord_min = machine.z_coord;
}

void print_cmd(command *cmd)
{
  fprintf(stderr,
          "printing command: G%g M%g X%g Y%g Z%g F%g E%g S%g T%g\n",
          cmd->G.flag ? cmd->G.decimal : -1,cmd->M.flag ? cmd->M.decimal : -1,
          cmd->X.decimal, cmd->Y.decimal, cmd->Z.decimal, cmd->F.decimal,
          cmd->E.decimal, cmd->S.decimal, cmd->T.decimal);
}

void print_stats(void)
{
  fprintf(stderr, "X_min: %g X_max: %g\n", machine.x_coord_min, machine.x_coord_max);
  fprintf(stderr, "Y_min: %g Y_max: %g\n", machine.y_coord_min, machine.y_coord_max);
  fprintf(stderr, "Z_min: %g Z_max: %g\n", machine.z_coord_min, machine.z_coord_max);
}
