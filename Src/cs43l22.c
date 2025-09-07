#include "cs43l22.h"
#include "i2c.h"
#include "log.h"
#include "usart.h"
#include <stm32f411xe.h>

void cs43l22_init(void) {
  GPIOD->ODR |= GPIO_ODR_OD4;
  uint8_t tmp;

  i2c1_write_reg(CS43L22_ADDR, 0x04, 0xaf);
  i2c1_write_reg(CS43L22_ADDR, 0x00, 0x99);
  i2c1_write_reg(CS43L22_ADDR, 0x47, 0x80);
  tmp = i2c1_read_reg(CS43L22_ADDR, 0x32);
  tmp |= 1 << 7;
  i2c1_write_reg(CS43L22_ADDR, 0x32, tmp);
  tmp = i2c1_read_reg(CS43L22_ADDR, 0x32);
  tmp &= ~(1 << 7);
  i2c1_write_reg(CS43L22_ADDR, 0x32, tmp);
  i2c1_write_reg(CS43L22_ADDR, 0x00, 0x00);
  i2c1_write_reg(CS43L22_ADDR, 0x06, (1 << 2));
  i2c1_write_reg(CS43L22_ADDR, 0x02, 0x9e);

  LOG_DEBUG("%s", "CS43L22 initialized\r\n");
}
