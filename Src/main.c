#include "clock.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include <stm32f411xe.h>


int main(void) {
  clock_init();
  gpio_init();
  tim2_init();
  usart2_init();
  usb_init();

  while (1)
    ;
}
