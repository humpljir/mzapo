#include <stdbool.h>
#include <stdio.h>

#define MIN_FRAME -5 //mm  // when negative not used
#define MAX_LAYERS 2000 //  supposing model will not hold more layers

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
  double x;  // x coord
  double y;  // y coord
} pos_t;

typedef struct
{
  long file_seek;  // pointer into start of the layer
  int length;  // length of the layer in move instructions
  pos_t start_point;  //first point of the layer, can be from leveling or the last point of previous layer 
  pos_t *points;  // pointer to an array of extruder's pathpoints
} layer_t;

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

typedef struct  // machine
{
  double x_coord;  // coords in mm
  double y_coord;
  double z_coord;
  double e_coord;
  double e_temp;  // extruder temperature
  double b_temp;  // bed temperature
  double fan_power;  // is fan turned on
} machine_t;

typedef struct  // model
{
  double x_coord_min;  // to be set while parsing whole gcode file
  double y_coord_min;
  double z_coord_min;

  double x_coord_max;  // to be set while parsing whole gcode file
  double y_coord_max;
  double z_coord_max;

  int layer_count;
  layer_t layers[MAX_LAYERS];  // holds pointers into file according to each leveling
  double layer_thickness;
} model_t;

typedef struct  // setting
{
  bool mm;  // true if set to mm, false if set to inches
  bool absolute;   // true if receiving absolute coords
  bool e_absolute;   // true if receiving absolute extruder coords
} setting_t;

typedef struct  // file
{
  char *name;
  FILE *fd;
  long size; // in bytes
} file_t;

bool parse_line (command *cmd, char *line, int size);
void process_cmd(command *cmd);  // get args and simulate machine
void print_cmd(command *cmd);
void move(command *cmd);  // expecting command to be G0 or G1
void print_stats(void);
bool init_file(char *filename);  //returns true on success, false on failure
void close_file(void);
void set_defualt_vals(void);
bool set_layer(layer_t *layer);  // take layer->file_seek and layer->length and allocate and write points
bool set_layer_by_num(int layer_num);  // takes index to model.layers[]
layer_t *get_layer(int layer_num);
void free_layer(layer_t *layer);
void print_layer(layer_t *layer);
bool get_cmd(command *cmd, bool move_only);  // reads next command from file.fd into cmd, if move_only, writes next G1 or G0, true on success, false othrwise
