#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stm32f411xe.h>

#define USART_TX_BUF_SIZE 256
char usart_tx_buf[USART_TX_BUF_SIZE];

void usart2_init(void) {
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
  USART2->BRR = SystemCoreClock / 2 / 115200;
  USART2->CR1 |= USART_CR1_UE | USART_CR1_TE;
}

void printf_usart2(const char *fmt, ...) {
  va_list args;
  uint16_t len;
  va_start(args, fmt);
  len = vsnprintf(usart_tx_buf, USART_TX_BUF_SIZE, fmt, args);
  va_end(args);
  for (uint16_t i = 0; i < len; i++) {
    while (!(USART2->SR & USART_SR_TXE))
      ;
    USART2->DR = usart_tx_buf[i];
  }
}
