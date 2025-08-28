#ifndef GPIO_DEF_H
#define GPIO_DEF_H

/*my set*/

#define GPIO32 0    // 32 - 32  
#define GPIO33 1    // 33 - 32
#define GPIO47 15   // 47 - 32
#define GPIO48 16   // 48 - 32
#define GPIO49 17   // 49 - 32
#define GPIO51 19   // 51 - 32
#define GPIO52 20   // 52 - 32
#define GPIO53 21   // 53 - 32
#define GPIO54 22   // 54 - 32

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
#define SPI0_CE1 GPIO32
#define SPI0_CE0 GPIO33
#define SPI0_MISO GPIO47
#define SPI0_MOSI GPIO48
#define SPI0_SCLK GPIO49

#define AD7606_CS_PIN SPI0_CE0   // Chip Select pin for AD7606
#define AD7606_CONVST_PIN GPIO51 // Conversion start pin
#define AD7606_RESET_PIN GPIO52  // Reset pin
#define AD7606_BUSY_PIN GPIO53   // Busy signal pin

#define DAC8563_CS_PIN SPI0_CE1 // Chip Select pin for DAC8563
#define DAC8563_SYNC_PIN SPI0_CE1 // Sync pin for DAC8563
#define DAC8563_LDAC_PIN GPIO54 // LDAC pin for DAC8563

#define GPIO_BASE_ADDR 0x0874A000
#define GPIO0_BASE_ADDR 0x0874A000
#define GPIO1_BASE_ADDR 0x0874B000
#define GPIO2_BASE_ADDR 0x0874C000
#define GPIO3_BASE_ADDR 0x0874D000
#define GPIO4_BASE_ADDR 0x0874E000

#define GPIO_OUTPUT_OFFSET_ADDR       0x0  /* OUTPUT register */
#define GPIO_DIR_OFFSET_ADDR          0x4  /* Direction Register 0 - Input 1 - Output */
#define GPIO_INPUT_OFFSET_ADDR        0x50 /* INPUT register */

#define GPIO1_OUTPUT_REG *(volatile uint32_t*)(GPIO1_BASE_ADDR + GPIO_OUTPUT_OFFSET_ADDR)
#define GPIO1_INPUT_REG *(volatile uint32_t*)(GPIO1_BASE_ADDR + GPIO_INPUT_OFFSET_ADDR)

void GPIO_INIT(int group, int pin, int mode);
// void GPIO_SET_PIN(int pin);
// void GPIO_CLEAR_PIN(int pin);
int GPIO_GETVALUE(int pin);

#define GPIO_SET_PIN(pin)                      \
    do                                     \
    {                                      \
        GPIO1_OUTPUT_REG |= (1U << (pin)); \
    } while (0)

#define GPIO_CLEAR_PIN(pin)                  \
    do                                   \
    {                                    \
        GPIO1_OUTPUT_REG &= ~(1 << pin); \
    } while (0)

#endif