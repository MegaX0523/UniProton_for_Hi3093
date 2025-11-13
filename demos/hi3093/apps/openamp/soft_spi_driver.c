#include "soft_spi_driver.h"

#include <stdint.h>
#include <unistd.h>

#include "bm_gpio.h"
#include "gpio_driver.h"
#include "gpio_pin_define.h"

#define BUFFER_SIZE 32

#define SPI0_CE0_HIGH GPIO_SET_PIN(SPI0_CE0)
#define SPI0_CE0_LOW GPIO_CLEAR_PIN(SPI0_CE0)
#define SPI0_CE1_HIGH GPIO_SET_PIN(SPI0_CE1)
#define SPI0_CE1_LOW GPIO_CLEAR_PIN(SPI0_CE1)
#define SPI0_SCK_HIGH GPIO_SET_PIN(SPI0_SCLK)
#define SPI0_SCK_LOW GPIO_CLEAR_PIN(SPI0_SCLK)
#define SPI0_MOSI_HIGH GPIO_SET_PIN(SPI0_MOSI)
#define SPI0_MOSI_LOW GPIO_CLEAR_PIN(SPI0_MOSI)

#define SPI0_MISO_READ gpio_getvalue(SPI0_MISO)

// SPI初始化函数
void spi0_init(void) {
    gpio_init(GPIO_GROUP1, DAC8563_CS_PIN, GPIO_OUTPUT);  // DAC8563 CS pin
    gpio_init(GPIO_GROUP1, AD7606_CS_PIN, GPIO_OUTPUT);   // AD7606 CS pin
    gpio_init(GPIO_GROUP1, SPI0_SCLK, GPIO_OUTPUT);       // SPI0
    gpio_init(GPIO_GROUP1, SPI0_MOSI, GPIO_OUTPUT);       // SPI0
    gpio_init(GPIO_GROUP1, SPI0_MISO, GPIO_INPUT);        // SPI0
}

// ================== SPI模式3 驱动 ADC ==================
void spi0_adc_receive(uint8_t* rx_buf, int length) {
    // 片选使能
    SPI0_SCK_HIGH;
    SPI0_CE0_LOW;
    SPI0_MOSI_HIGH;
    for (int byte_idx = 0; byte_idx < length; byte_idx++) {
        uint8_t rx_byte = 0;
        // 传输8位数据（MSB first）
        for (int bit_idx = 7; bit_idx >= 0; bit_idx--) {
            // 时钟下降沿（从机采样）
            SPI0_SCK_LOW;
            // 主机读取
            if (SPI0_MISO_READ) {
                rx_byte |= (1 << bit_idx);
            }
            // 时钟上升沿（为下一位准备）
            SPI0_SCK_HIGH;
        }
        // 存储接收到的字节
        if (rx_buf) {
            rx_buf[byte_idx] = rx_byte;
        }
    }
    // 片选禁用
    SPI0_CE0_HIGH;
    SPI0_MOSI_LOW;
}

// ================== SPI模式3 驱动 DAC ==================
void spi0_dac_transfer(const uint8_t* tx_buf, int length) {
    static uint8_t tx_byte = 0x0;

    SPI0_CE1_LOW;
    SPI0_SCK_HIGH;
    for (int byte_idx = 0; byte_idx < length; byte_idx++) {
        tx_byte = tx_buf[byte_idx];
        // 传输8位数据（MSB first）
        for (int bit_idx = 7; bit_idx >= 0; bit_idx--) {
            // 设置MOSI（在时钟上升前准备数据）
            if (tx_byte & (1 << bit_idx)) {
                SPI0_MOSI_HIGH;  // 发送1
            } else {
                SPI0_MOSI_LOW;  // 发送0
            }
            SPI0_SCK_LOW;
            SPI0_SCK_HIGH;
        }
    }

    // 片选禁用
    SPI0_CE1_HIGH;
    SPI0_MOSI_HIGH;
}