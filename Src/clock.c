#include "clock.h"
#include <stm32f411xe.h>

void clock_init(void) {
  RCC->CR |= RCC_CR_HSEON;
  while (!(RCC->CR & RCC_CR_HSERDY))
    ;

  RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;
  RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP |
                    RCC_PLLCFGR_PLLQ);
  RCC->PLLCFGR |= (4 << RCC_PLLCFGR_PLLM_Pos) | (96 << RCC_PLLCFGR_PLLN_Pos) |
                  (4 << RCC_PLLCFGR_PLLQ_Pos);
  RCC->CR |= RCC_CR_PLLON;
  while (!(RCC->CR & RCC_CR_PLLRDY))
    ;

  FLASH->ACR |= FLASH_ACR_LATENCY_3WS;

  RCC->CFGR &= RCC_CFGR_SW;
  RCC->CFGR |= RCC_CFGR_SW_PLL;

  RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

  RCC->PLLI2SCFGR &= ~(RCC_PLLI2SCFGR_PLLI2SM | RCC_PLLI2SCFGR_PLLI2SN |
                       RCC_PLLI2SCFGR_PLLI2SR);
  RCC->PLLI2SCFGR |= (8 << RCC_PLLI2SCFGR_PLLI2SM_Pos) |
                     (258 << RCC_PLLI2SCFGR_PLLI2SN_Pos) |
                     (3 << RCC_PLLI2SCFGR_PLLI2SR_Pos);
  RCC->CR |= RCC_CR_PLLI2SON;
  while (!(RCC->CR & RCC_CR_PLLI2SRDY))
    ;
  SystemCoreClockUpdate();
}
