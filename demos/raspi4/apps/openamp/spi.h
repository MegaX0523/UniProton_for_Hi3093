#ifndef SPI_H
#define SPI_H
#include <stdint.h>
#include <unistd.h>
#include "gpio_def.h"

// SPI0寄存器偏移量
typedef struct
{
    volatile uint32_t CS;   // 0x00 Control and Status
    volatile uint32_t FIFO; // 0x04 Data FIFO
    volatile uint32_t CLK;  // 0x08 Clock divider
    volatile uint32_t DLEN; // 0x0C Data Length
    volatile uint32_t LTOH; // 0x10 LOSSI mode TOH
    volatile uint32_t DC;   // 0x14 DMA configuration
} SPI0_Registers;

#define SPI0 ((SPI0_Registers *)SPI0_BASE)

// SPI控制寄存器标志位
#define SPI_CS_TA 0x00000080    // Transfer Active
#define SPI_CS_CLEAR 0x00000030 // Clear FIFO
#define SPI_CS_CPOL0 0x00000000  // CPOL = 0    Rest state of clock = low
#define SPI_CS_CPOL1 0x00000008   // CPOL = 1   Rest state of clock = high.
#define SPI_CS_CPHA0 0x00000000  // CPHA = 0    First SCLK transition at middle of data bit.
#define SPI_CS_CPHA1 0x00000004   // CPHA = 1   First SCLK transition at beginning of data bit
#define SPI_CS_CS0 0x00000000    // Chip select0
#define SPI_CS_CS1 0x00000001  // Chip select1
#define SPI_CS_DONE 0x00010000   // Transfer Done
#define SPI_CS_RXD 0x00020000    //  RX FIFO contains Data
#define SPI_CS_TXD 0x00040000    //  TX FIFO can accept Data

#define CPHA0 0
#define CPHA1 1

void SPI0_Init(void);
uint8_t SPI0_TransferByte(uint8_t CSx, uint8_t data);
void SPI0_TransferBuffer(uint8_t CSx, uint8_t *txBuffer, uint8_t *rxBuffer, uint32_t length);
void SPI0_SelectSlave(uint8_t slave);
void SPI0_DeselectSlave(uint8_t CSx);
void SPI0_Set_CPHA0(void);
void SPI0_Set_CPHA1(void);

#endif