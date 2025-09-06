#include "i2c.h"
#include "log.h"
#include "usart.h"
#include <stdint.h>
#include <stm32f411xe.h>

#define I2C_CLK_FREQ 100000

void i2c1_init(void) {
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
  I2C1->CR2 |= (SystemCoreClock / 2 / 1000000) << I2C_CR2_FREQ_Pos;
  I2C1->CCR |= (SystemCoreClock / 2 / (2 * I2C_CLK_FREQ)) << I2C_CCR_CCR_Pos;
  I2C1->TRISE |= (1000 / (1000000000 / SystemCoreClock) + 1)
                 << I2C_TRISE_TRISE_Pos;
  I2C1->CR1 |= I2C_CR1_PE;

  // 計算値の確認
  uint32_t freq_val = SystemCoreClock / 2 / 1000000;
  uint32_t ccr_val = SystemCoreClock / 2 / (2 * I2C_CLK_FREQ);
  uint32_t trise_val = 1000 / (1000000000 / (SystemCoreClock / 2)) + 1;
}

static void i2c1_start(void) {
  I2C1->CR1 |= I2C_CR1_START;
  while (!(I2C1->SR1 & I2C_SR1_SB))
    ;
}

static void i2c1_stop(void) { I2C1->CR1 |= I2C_CR1_STOP; }

static void i2c1_send_addr(uint8_t addr) {
  I2C1->DR = addr;
  while (!(I2C1->SR1 & I2C_SR1_ADDR))
    ;
  (void)I2C1->SR1;
  (void)I2C1->SR2;
}

static void i2c1_write(uint8_t data) {
  while (!(I2C1->SR1 & I2C_SR1_TXE))
    ;
  I2C1->DR = data;
}

static uint8_t i2c1_read(void) {
  while (!(I2C1->SR1 & I2C_SR1_RXNE))
    ;
  return I2C1->DR;
}

uint8_t i2c1_read_reg(uint8_t addr, uint8_t reg) {
  uint8_t data;

  i2c1_start();
  i2c1_send_addr(addr | 0);
  i2c1_write(reg);
  i2c1_start();
  i2c1_send_addr(addr | 1);
  data = i2c1_read();
  i2c1_stop();
  return data;
}

void i2c1_write_reg(uint8_t addr, uint8_t reg, uint8_t data) {
  i2c1_start();
  i2c1_send_addr(addr | 0);
  i2c1_write(reg);
  i2c1_write(data);
  while (!(I2C1->SR1 & I2C_SR1_BTF))
    ;
  i2c1_stop();
}
