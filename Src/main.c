#include "clock.h"
#include "gpio.h"
#include "log.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include <stm32f411xe.h>

int main(void) {
  clock_init();
  gpio_init();
  tim2_init();
  usart2_init();

  // ログレベル設定（デバッグ時はDEBUG、本番時はINFO）
  log_set_level(LOG_DEBUG);

  usb_init();

  printf_usart2("--------------------------------\r\n");
  printf_usart2("Program start\r\n");
  printf_usart2("Log level: %d\r\n", log_get_level());
  printf_usart2("--------------------------------\r\n");

  while (1)
    ;
}
