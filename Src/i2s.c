#include "i2s.h"
#include "log.h"
#include "usart.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32f411xe.h>

#define BUF_SIZE 96

static const int16_t sin_1k[BUF_SIZE] = {
    0,      0,      4277,   4277,   8481,   8481,   12539,  12539,  16383,
    16383,  19947,  19947,  23170,  23170,  25996,  25996,  28377,  28377,
    30273,  30273,  31650,  31650,  32487,  32487,  32767,  32767,  32487,
    32487,  31650,  31650,  30273,  30273,  28377,  28377,  25996,  25996,
    23170,  23170,  19947,  19947,  16384,  16384,  12539,  12539,  8481,
    8481,   4277,   4277,   0,      0,      -4277,  -4277,  -8481,  -8481,
    -12539, -12539, -16383, -16383, -19947, -19947, -23170, -23170, -25996,
    -25996, -28377, -28377, -30273, -30273, -31650, -31650, -32487, -32487,
    -32767, -32767, -32487, -32487, -31650, -31650, -30273, -30273, -28377,
    -28377, -25996, -25996, -23170, -23170, -19947, -19947, -16384, -16384,
    -12539, -12539, -8481,  -8481,  -4277,  -4277,
};

void i2s3_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
  DMA1_Stream5->CR |= (0 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MSIZE_0 |
                      DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_CIRC |
                      DMA_SxCR_DIR_0;
  DMA1_Stream5->PAR = (uint32_t)&SPI3->DR;
  DMA1_Stream5->M0AR = (uint32_t)sin_1k;
  DMA1_Stream5->NDTR = BUF_SIZE;

  RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
  SPI3->CR2 |= SPI_CR2_TXDMAEN;
  SPI3->I2SCFGR |= SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG_1;
  SPI3->I2SPR &= ~(SPI_I2SPR_I2SDIV);
  SPI3->I2SPR = (3 << SPI_I2SPR_I2SDIV_Pos) | // 分周比3
                SPI_I2SPR_ODD |               // 奇数補正
                SPI_I2SPR_MCKOE;              // MCK出力
  SPI3->I2SCFGR |= SPI_I2SCFGR_I2SE;

  DMA1_Stream5->CR |= DMA_SxCR_EN;
}
