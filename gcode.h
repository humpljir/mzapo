#include <stdbool.h>

/*
typedef enum
{
  G0,    // rapid movement
  G1,    // movement
  G20,   // set units to inches
  G21,   // set units to mm
  G28,   // move to origin (fields as flags);
  G92,   // set position (used to reset extruder when stepping up to next layer and when absolute extruder coords used)
  G90,   // set absolute positioning
  G91,   // set relative positioning

  M82,   // set extruder to absolute mode
  M104,  // Set Extruder Temperature 
  M106,  // fan on 
  M140,  // set bed temperature
  M190,  // wait for bed temperature A(Snn for min temp to wait for)

} cmd_type;
*/

typedef enum
{
  G0 = 0,    // rapid movement
  G1 = 1,    // movement
  G20 = 20,   // set units to inches
  G21 = 21,   // set units to mm
  G28 = 28,   // move to origin (fields as flags);
  G92 = 92,   // set position (used to reset extruder when stepping up to next layer and when absolute extruder coords used)
  G90 = 90,   // set absolute positioning
  G91 = 91   // set relative positioning
} g_cmd_type;

typedef enum
{
  M82 = 82,   // set extruder to absolute mode
  M104 = 104,  // Set Extruder Temperature 
  M106 = 106,  // fan on 
  M140 = 140  // set bed temperature
} m_cmd_type;
typedef struct
{
  bool flag;
  double decimal;
} field;

typedef struct
{
  field G;
  field M;
  field X;
  field Y;
  field Z;
  field F;
  field E;
  field S;
  field T;
} command;

bool parse_line (command *cmd, char *line, int size);
void process_cmd(command *cmd);  // get args and simulate machine
void print_cmd(command *cmd);
void move(command *cmd);  // expecting command to be G0 or G1
void print_stats(void);
