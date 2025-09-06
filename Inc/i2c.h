#pragma once

#include <stdint.h>

void i2c1_init(void);
uint8_t i2c1_read_reg(uint8_t addr, uint8_t reg);
void i2c1_write_reg(uint8_t addr, uint8_t reg, uint8_t data);
