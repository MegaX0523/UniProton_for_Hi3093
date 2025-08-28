#ifndef HI_SPI_H
#define HI_SPI_H
#include <stdint.h>
#include <unistd.h>

#define BUFFER_SIZE 32

#define SPI0_CE0_HIGH GPIO_SET_PIN(SPI0_CE0)
#define SPI0_CE0_LOW GPIO_CLEAR_PIN(SPI0_CE0)
#define SPI0_CE1_HIGH GPIO_SET_PIN(SPI0_CE1)
#define SPI0_CE1_LOW GPIO_CLEAR_PIN(SPI0_CE1)
#define SPI0_SCK_HIGH GPIO_SET_PIN(SPI0_SCLK)
#define SPI0_SCK_LOW GPIO_CLEAR_PIN(SPI0_SCLK)
#define SPI0_MOSI_HIGH GPIO_SET_PIN(SPI0_MOSI)
#define SPI0_MOSI_LOW GPIO_CLEAR_PIN(SPI0_MOSI)
#define SPI0_MISO_READ GPIO_GETVALUE(SPI0_MISO)

#define CONTROL_CHANNEL 0
#define EXCITATION_CHANNEL 1

void SPI0_Init(void);
void spi0_AD_receive(uint8_t *rx_buf, size_t length);
void spi0_DA_transfer(const uint8_t *tx_buf, size_t length);


#endif