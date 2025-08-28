#include <stdint.h>
#include <unistd.h>
#include "spi.h"
#include "gpio_def.h"

// SPI初始化函数
void SPI0_Init(void)
{
    
    GPIO_INIT(SPI0_CE0, GPIO_ALT0); // GPIO8 (CE0) 设置为ALT0
    GPIO_INIT(SPI0_CE1, GPIO_ALT0); // GPIO7 (CE1) 设置为ALT0
    // GPIO_INIT(SPI0_CE0, GPIO_OUTPUT); // GPIO8 (CE0) 设置为输出
    // GPIO_INIT(SPI0_CE1, GPIO_OUTPUT); // GPIO7 (CE1) 设置为输出
    // GPIO_SET(SPI0_CE0, GPIO_LEVEL_HIGH); // 设置CE0为高电平
    // GPIO_SET(SPI0_CE1, GPIO_LEVEL_HIGH); // 设置CE1为高电平

    // 配置GPIO引脚为SPI功能
    GPIO_INIT(SPI0_MISO, GPIO_ALT0); // GPIO9 (MISO) 设置为ALT0
    GPIO_INIT(SPI0_MOSI, GPIO_ALT0); // GPIO10 (MOSI) 设置为ALT0
    GPIO_INIT(SPI0_SCLK, GPIO_ALT0); // GPIO11 (SCLK) 设置为ALT0
    // 清除FIFO
    SPI0->CS = SPI_CS_CLEAR;
    // 配置SPI模式0 (CPOL=0, CPHA=0)
    // SPI0->CS = SPI_CS_CPOL0 | SPI_CS_CPHA0;
    SPI0->CS = SPI_CS_CPOL0 | SPI_CS_CPHA1;
    // 设置时钟分频器 (假设设置为250MHz/32=7.8MHz)
    SPI0->CLK = 32;
    SPI0_SelectSlave(SPI_CS_CS0); // 选择片选0
}

// SPI发送和接收一个字节
uint8_t SPI0_TransferByte(uint8_t CSx, uint8_t data)
{
    SPI0_SelectSlave(CSx);
    // 启动传输
    SPI0->CS |= SPI_CS_TA;
    // 等待TX FIFO有空间
    while (!(SPI0->CS & SPI_CS_TXD))
        ;
    // 发送数据
    SPI0->FIFO = data;
    
    // 等待接收数据
    while (!(SPI0->CS & SPI_CS_RXD))
        ;
    // 读取接收到的数据
    uint8_t received = SPI0->FIFO;
    // 等待传输完成
    while (!(SPI0->CS & SPI_CS_DONE))
        ;
    // 关闭传输
    SPI0->CS &= ~SPI_CS_TA;

    SPI0_DeselectSlave(CSx); // 取消选择片选

    return received;
}

// SPI发送多个字节
void SPI0_TransferBuffer(uint8_t CSx, uint8_t *txBuffer, uint8_t *rxBuffer, uint32_t length)
{
    uint32_t i;
    // 选择片选0
    SPI0_SelectSlave(CSx);
    // 启动传输
    SPI0->CS |= SPI_CS_TA;
    for (i = 0; i < length; i++)
    {
        // 等待TX FIFO有空间
        while (!(SPI0->CS & SPI_CS_TXD))
            ;
        // 发送数据
        SPI0->FIFO = txBuffer[i];
        // 等待接收数据
        while (!(SPI0->CS & SPI_CS_RXD))
            ;
        // 读取接收到的数据
        if (rxBuffer)
        {
            rxBuffer[i] = SPI0->FIFO;
        }
        else
        {
            // 如果不需要接收数据，仍然需要读取FIFO以清空它
            (void)SPI0->FIFO;
        }
    }
    // 等待传输完成
    while (!(SPI0->CS & SPI_CS_DONE))
        ;
    // 关闭传输
    SPI0->CS &= ~SPI_CS_TA;
}

// 选择从设备
void SPI0_SelectSlave(uint8_t CSx)
{
    if (CSx == SPI0_CE0)
    {
        SPI0->CS = (SPI0->CS & ~0x03) | SPI_CS_CS0;
        // GPIO_SET(SPI0_CE0, GPIO_LEVEL_LOW); // 设置CE0为低电平
    }
    else if(CSx == SPI0_CE1)
    {
        SPI0->CS = (SPI0->CS & ~0x03) | SPI_CS_CS1;
        // GPIO_SET(SPI0_CE1, GPIO_LEVEL_LOW); // 设置CE1为低电平
    }
}

void SPI0_DeselectSlave(uint8_t CSx)
{
    if (CSx == SPI0_CE0)
    {
        GPIO_SET(SPI0_CE0, GPIO_LEVEL_HIGH); // 设置CE0为高电平
    }
    else if(CSx == SPI0_CE1)
    {
        GPIO_SET(SPI0_CE1, GPIO_LEVEL_HIGH); // 设置CE1为高电平
    }
}

void SPI0_Set_CPHA0(void)
{
    SPI0->CS &= ~SPI_CS_CPHA1; // 设置CPHA为0
}

void SPI0_Set_CPHA1(void)
{
    SPI0->CS |= SPI_CS_CPHA1; // 设置CPHA为1
}