#include "clock.h"
#include "cs43l22.h"
#include "gpio.h"
#include "i2c.h"
#include "i2s.h"
#include "log.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "usb_audio.h"
#include <stm32f411xe.h>

extern uint64_t global_time_us;
extern uint64_t callback_time_history[CALLBACK_TIME_HISTORY_SIZE];

int main(void) {
  clock_init();
  gpio_init();
  usart2_init();
  // log_set_level(LOG_DEBUG);
  log_set_level(LOG_INFO);
  tim2_init();
  tim3_init();
  i2c1_init();
  cs43l22_init();
  i2s3_init();
  usb_init();

  printf_usart2("--------------------------------\r\n");
  printf_usart2("Program start\r\n");
  printf_usart2("Log level: %d\r\n", log_get_level());
  printf_usart2("--------------------------------\r\n");

  while (1) {
    if (global_time_us % 1000000 == 0) {
      for (uint32_t i = 0; i < CALLBACK_TIME_HISTORY_SIZE; i++) {
        LOG_INFO("Callback time history[%d]: %d\r\n", i,
                 callback_time_history[i]);
      }
    }
  }
}
