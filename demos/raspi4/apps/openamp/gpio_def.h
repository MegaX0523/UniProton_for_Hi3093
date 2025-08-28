#ifndef GPIO_DEF_H
#define GPIO_DEF_H

/*my set*/
// 外设基地址（树莓派4B）
#define BCM2711_PERI_BASE 0xFE000000
#define GPIO_BASE (BCM2711_PERI_BASE + 0x200000)
#define UART0_BASE (BCM2711_PERI_BASE + 0x201000)
#define SPI0_BASE (BCM2711_PERI_BASE + 0x204000)

#define GPFSEL0 *(volatile uint32_t *)(GPIO_BASE + 0x00) // GPIO功能选择寄存器0
#define GPFSEL1 *(volatile uint32_t *)(GPIO_BASE + 0x04) // GPIO功能选择寄存器1
#define GPFSEL2 *(volatile uint32_t *)(GPIO_BASE + 0x08) // GPIO功能选择寄存器2
#define GPSET0 *(volatile uint32_t *)(GPIO_BASE + 0x1C)  // GPIO设置寄存器0
#define GPCLR0 *(volatile uint32_t *)(GPIO_BASE + 0x28)  // GPIO清除寄存器0

#define GPIO7 7
#define GPIO8 8
#define GPIO9 9
#define GPIO10 10
#define GPIO11 11
#define GPIO17 17
#define GPIO18 18
#define GPIO23 23
#define GPIO22 22
#define GPIO27 27

// GPIO pin modes
#define GPIO_INPUT 0b000
#define GPIO_OUTPUT 0b001 // Output mode, default low
#define GPIO_ALT0 0b100
#define GPIO_ALT1 0b101
#define GPIO_ALT2 0b110
#define GPIO_ALT3 0b111
#define GPIO_ALT4 0b011
#define GPIO_ALT5 0b010

// GPIO pin levels
#define GPIO_LEVEL_HIGH 1
#define GPIO_LEVEL_LOW 0

// Define GPIO pins (arbitrary assignments)
#define SPI0_CE1 GPIO7
#define SPI0_CE0 GPIO8
#define SPI0_MISO GPIO9
#define SPI0_MOSI GPIO10
#define SPI0_SCLK GPIO11

#define AD7606_CS_PIN SPI0_CE0   // Chip Select pin for AD7606
#define AD7606_CONVST_PIN GPIO17 // Conversion start pin
#define AD7606_RESET_PIN GPIO18  // Reset pin
#define AD7606_BUSY_PIN GPIO27   // Busy signal pin

#define DAC8563_CS_PIN SPI0_CE1 // Chip Select pin for DAC8563
#define DAC8563_SYNC_PIN GPIO22 // Sync pin for DAC8563
#define DAC8563_LDAC_PIN GPIO23 // LDAC pin for DAC8563

extern void GPIO_INIT(int pin, int mode);
extern void GPIO_SET(int pin, int value);
extern int GPIO_GETVALUE(int pin);

#endif