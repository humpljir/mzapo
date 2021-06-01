#include <stdbool.h>

#ifndef HWIO_H
#define HWIO_H

bool hw_init(void);
void hw_check(void);  // reads data from knobs and writes old data to last
bool hw_r_released(void);
bool hw_g_released(void);
bool hw_b_released(void);
bool hw_r_incremented(void);
bool hw_g_incremented(void);
bool hw_b_incremented(void);
bool hw_r_decremented(void);
bool hw_g_decremented(void);
bool hw_b_decremented(void);
void hw_test_out(void);
void hw_write_ledstrip(uint32_t bits);
void hw_write_led1(uint8_t red, uint8_t green, uint8_t blue);
void hw_write_led2(uint8_t red, uint8_t green, uint8_t blue);

#endif
