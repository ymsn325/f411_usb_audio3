#include "clock.h"
#include "cs43l22.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include <stm32f411xe.h>

int main(void) {
  // ログレベル設定（デバッグ時はDEBUG、本番時はINFO）
  clock_init();
  gpio_init();
  usart2_init();
  log_set_level(LOG_DEBUG);
  tim2_init();
  i2c1_init();
  cs43l22_init();

  // usb_init();

  printf_usart2("--------------------------------\r\n");
  printf_usart2("Program start\r\n");
  printf_usart2("Log level: %d\r\n", log_get_level());
  printf_usart2("--------------------------------\r\n");

  uint8_t data = i2c1_read_reg(0x94, 0x01);
  LOG_DEBUG("%x", data);
  while (1)
    ;
}
