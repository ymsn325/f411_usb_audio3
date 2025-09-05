#include "tim.h"
#include <stm32f411xe.h>

void tim2_init(void) {
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  TIM2->PSC = 9600;
  TIM2->ARR = 5000;
  TIM2->DIER |= TIM_DIER_UIE;
  NVIC_SetPriority(TIM2_IRQn, 1);
  NVIC_EnableIRQ(TIM2_IRQn);
  TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2_IRQHandler(void) {
  if (TIM2->SR & TIM_SR_UIF) {
    TIM2->SR &= ~TIM_SR_UIF;
    GPIOD->ODR ^= 1 << GPIO_ODR_OD15_Pos;
  }
}
