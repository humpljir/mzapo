#include <stdbool.h>

#ifndef HWIO_H
#define HWIO_H

bool hw_init(void);
void hw_check(void);  // reads data from knobs and writes old data to last
void hw_test_out(void);
void hw_write_ledstrip(uint32_t bits);
void hw_write_led1(uint8_t red, uint8_t green, uint8_t blue);
void hw_write_led2(uint8_t red, uint8_t green, uint8_t blue);

#endif
