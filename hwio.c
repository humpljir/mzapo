#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>  // sleep

#include "hwio.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#define ONE_SECOND 1 * 1000 * 1000  // us
#define HALF_SECOND 500 * 1000  // us

struct
{
  bool initialized;
  unsigned char *mem_base;
} hw = {false, NULL};

struct
{
  uint8_t r_rot;
  uint8_t g_rot;
  uint8_t b_rot;

  bool r_pushed;
  bool g_pushed;
  bool b_pushed;

  uint8_t r_rot_last;  // values from previous check
  uint8_t g_rot_last;
  uint8_t b_rot_last;

  bool r_pushed_last;
  bool g_pushed_last;
  bool b_pushed_last;
} input = {0}; 

bool hw_init(void)
{
  if (hw.initialized) return false;
  hw.mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (hw.mem_base == NULL) return false;
  hw.initialized = true;
  return true;
}

void hw_check(void)  // reads data from knobs and writes old data to last
{
  assert(hw.initialized);
  input.r_rot_last = input.r_rot;
  input.g_rot_last = input.g_rot;
  input.b_rot_last = input.b_rot;

  input.r_pushed_last = input.r_pushed;
  input.g_pushed_last = input.g_pushed;
  input.b_pushed_last = input.b_pushed;

  uint32_t knobs = *(volatile uint32_t *)(hw.mem_base + SPILED_REG_KNOBS_8BIT_o); 
  uint32_t mask = 0x000000FF;
  input.b_rot = knobs & mask;
  knobs >>= 8;
  input.g_rot = knobs & mask;
  knobs >>= 8;
  input.r_rot = knobs & mask;
  knobs >>= 8;
  mask = 0x00000001;
  input.b_pushed = knobs & mask;
  knobs >>= 1;
  input.g_pushed = knobs & mask;
  knobs >>= 1;
  input.r_pushed = knobs & mask;
 /* 
  fprintf(stderr,
          "r_rot: %d, g_rot: %d, b_rot: %d,\nr_switch: %s, g_switch: %s, b_switch: %s\n",
          input.r_rot, input.g_rot, input.b_rot,
          input.r_pushed ? "down" : "up",
          input.g_pushed ? "down" : "up",
          input.b_pushed ? "down" : "up");
  */
}

bool hw_r_released(void)
{
  return (input.r_pushed == false && input.r_pushed_last == true);
}
bool hw_g_released(void)
{
  return (input.g_pushed == false && input.g_pushed_last == true);
}
bool hw_b_released(void)
{
  return (input.b_pushed == false && input.b_pushed_last == true);
}

bool hw_r_incremented(void)
{
  bool retval = (input.r_rot > input.r_rot_last || input.r_rot_last - input.r_rot > 128);
  //if (retval) fprintf(stderr, "hwio.c: r incr\n");
  return retval;
}
bool hw_g_incremented(void)
{
  bool retval = (input.g_rot > input.g_rot_last || input.g_rot_last - input.g_rot > 128);
  //if (retval) fprintf(stderr, "hwio.c: g incr\n");
  return retval;
}
bool hw_b_incremented(void)
{
  bool retval = (input.b_rot > input.b_rot_last || input.b_rot_last - input.b_rot > 128);
  //if (retval) fprintf(stderr, "hwio.c: b incr\n");
  return retval;
}
bool hw_r_decremented(void)
{
  bool retval = (input.r_rot < input.r_rot_last || input.r_rot - input.r_rot_last >= 128);
  //if (retval) fprintf(stderr, "hwio.c: r decr\n");
  return retval;
}
bool hw_g_decremented(void)
{
  bool retval = (input.g_rot < input.g_rot_last || input.g_rot - input.g_rot_last >= 128);
  //if (retval) fprintf(stderr, "hwio.c: g decr\n");
  return retval;
}
bool hw_b_decremented(void)
{
  bool retval = (input.b_rot < input.b_rot_last || input.b_rot - input.b_rot_last >= 128);
  //if (retval) fprintf(stderr, "hwio.c: b decr\n");
  return retval;
}

void hw_test_out(void)
{
  assert(hw.initialized);
  hw_write_ledstrip(0xFFFFFFFF);
  usleep(ONE_SECOND);
  hw_write_ledstrip(0x0);
  usleep(ONE_SECOND);
  hw_write_ledstrip(0xFFFFFFFF);
  usleep(ONE_SECOND);
  hw_write_ledstrip(0x0);
  usleep(ONE_SECOND);
  uint32_t bits = 0xFFFFFFFF;
  for (int i = 0; i < 32; i++)
    {
      hw_write_ledstrip(bits);
      bits <<= 1;
      usleep(HALF_SECOND);
    }
  hw_write_ledstrip(bits);
  hw_write_led1(255, 0, 0);
  usleep(HALF_SECOND);
  hw_write_led1(0, 255, 0);
  usleep(HALF_SECOND);
  hw_write_led1(0, 0, 255);
  usleep(HALF_SECOND);

  hw_write_led2(255, 0, 0);
  usleep(HALF_SECOND);
  hw_write_led2(0, 255, 0);
  usleep(HALF_SECOND);
  hw_write_led2(0, 0, 255);
  usleep(HALF_SECOND);
  hw_write_led1(0, 0, 0);
  hw_write_led2(0, 0, 0);
}

void hw_write_ledstrip(uint32_t bits)
{
  assert(hw.initialized);
  *(volatile uint32_t *)(hw.mem_base + SPILED_REG_LED_LINE_o) = bits; 
}

void hw_write_led1(uint8_t red, uint8_t green, uint8_t blue)
{
  assert(hw.initialized);
  uint32_t color = red;
  color <<= 8;
  color += green;
  color <<= 8;
  color += blue;
  *(volatile uint32_t *)(hw.mem_base + SPILED_REG_LED_RGB1_o) = color;
}

void hw_write_led2(uint8_t red, uint8_t green, uint8_t blue)
{
  assert(hw.initialized);
  uint32_t color = red;
  color <<= 8;
  color += green;
  color <<= 8;
  color += blue;
  *(volatile uint32_t *)(hw.mem_base + SPILED_REG_LED_RGB2_o) = color;
}
