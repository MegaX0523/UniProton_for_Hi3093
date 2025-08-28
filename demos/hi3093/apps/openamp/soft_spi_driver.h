#ifndef HI_SPI_H
#define HI_SPI_H
#include <stdint.h>
#include <unistd.h>

void spi0_init(void);
void spi0_adc_receive(uint8_t *rx_buf, size_t length);
void spi0_dac_transfer(const uint8_t *tx_buf, size_t length);

#endif