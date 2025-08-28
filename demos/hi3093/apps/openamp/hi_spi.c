#include <stdint.h>
#include <unistd.h>
#include "hi_spi.h"
#include "gpio_def.h"
#include "bm_gpio.h"
#include "prt_hwi.h"
#include "test.h"

// SPI初始化函数
void SPI0_Init(void)
{
    GPIO_INIT(GPIO_GROUP1, DAC8563_CS_PIN, GPIO_OUTPUT); // DAC8563 CS pin
    GPIO_INIT(GPIO_GROUP1, AD7606_CS_PIN, GPIO_OUTPUT);  // AD7606 CS pin
    GPIO_INIT(GPIO_GROUP1, SPI0_SCLK, GPIO_OUTPUT);      // SPI0
    GPIO_INIT(GPIO_GROUP1, SPI0_MOSI, GPIO_OUTPUT);      // SPI0
    GPIO_INIT(GPIO_GROUP1, SPI0_MISO, GPIO_INPUT);       // SPI0
}

// ================== SPI模式0 (CPHA=0) 驱动 ADC ==================
void spi0_AD_receive(uint8_t *rx_buf, size_t length)
{
    // 片选使能
    SPI0_CE0_LOW;
    SPI0_MOSI_HIGH;
    for (size_t byte_idx = 0; byte_idx < length; byte_idx++)
    {
        uint8_t rx_byte = 0;
        // 传输8位数据（MSB first）
        for (int bit_idx = 7; bit_idx >= 0; bit_idx--)
        {
            // 时钟上升沿（从机采样）
            SPI0_SCK_HIGH;
            // 主机读取（从机在上升沿时已准备好数据）
            if (SPI0_MISO_READ)
            {
                rx_byte |= (1 << bit_idx);
            }
            // 时钟下降沿（为下一位准备）
            SPI0_SCK_LOW;
        }
        // 存储接收到的字节
        if (rx_buf)
        {
            rx_buf[byte_idx] = rx_byte;
        }
    }
    // 片选禁用
    SPI0_CE0_HIGH;
}

// ================== SPI模式1 (CPHA=1) 驱动 DAC ==================
void spi0_DA_transfer(const uint8_t *tx_buf, size_t length)
{
    static byte_idx = 0;
    static uint8_t tx_byte = 0x0;
    static int bit_idx = 7;

    SPI0_CE1_LOW;
    for (byte_idx = 0; byte_idx < length; byte_idx++)
    {
        tx_byte = tx_buf[byte_idx];

        // 初始时钟为低（空闲状态）
        SPI0_SCK_LOW;

        // 传输8位数据（MSB first）
        for (bit_idx = 7; bit_idx >= 0; bit_idx--)
        {
            // 设置MOSI（在时钟上升前准备数据）
            if(tx_byte & (1 << bit_idx))
            {
                SPI0_MOSI_HIGH; // 发送1
            }
            else
            {
                SPI0_MOSI_LOW; // 发送0
            }
            // 时钟上升沿（从机在此刻改变输出）
            SPI0_SCK_HIGH;
            // 时钟下降沿（从机采样输入）
            SPI0_SCK_LOW;
        }
    }

    // 片选禁用
    SPI0_CE1_HIGH;
    SPI0_MOSI_HIGH;
}