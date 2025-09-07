#include "i2s.h"
#include "log.h"
#include "usart.h"
#include "usb_audio.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32f411xe.h>

extern int16_t audio_rx_samples_0[AUDIO_BUFFER_SIZE / 2];
extern int16_t audio_rx_samples_1[AUDIO_BUFFER_SIZE / 2];

void i2s3_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
  DMA1_Stream5->CR |= (0 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MSIZE_0 |
                      DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 |
                      DMA_SxCR_DBM;
  DMA1_Stream5->PAR = (uint32_t)&SPI3->DR;
  DMA1_Stream5->M0AR = (uint32_t)audio_rx_samples_0;
  DMA1_Stream5->M1AR = (uint32_t)audio_rx_samples_1;
  DMA1_Stream5->NDTR = AUDIO_BUFFER_SIZE / 2;

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
