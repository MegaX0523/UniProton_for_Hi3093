#include <stdint.h>
#include "gpio_pin_define.h"
#include "bm_gpio.h"

#define GPIO1_OUTPUT_REG *(volatile uint32_t*)(GPIO1_BASE_ADDR + GPIO_OUTPUT_OFFSET_ADDR)
#define GPIO1_INPUT_REG *(volatile uint32_t*)(GPIO1_BASE_ADDR + GPIO_INPUT_OFFSET_ADDR)

// Function to initialize GPIO pin
void gpio_init(int group, int pin, int mode)
{
    static int tmp;
    bm_gpio_init(group, pin);
    if (mode == GPIO_INPUT) {
        bm_gpio_get_level(group, pin, &tmp); // Set pin low for input
    } else if (mode == GPIO_OUTPUT) {
        if (pin == SPI0_SCLK || pin == AD7606_RESET_PIN)
        {
            bm_gpio_set_level(group, pin, GPIO_LEVEL_LOW);
        }
        else
        {
            bm_gpio_set_level(group, pin, GPIO_LEVEL_HIGH); // Set pin high for output
        }
    }
}

// Function to read GPIO input value
inline int gpio_getvalue(int pin)
{
    return (GPIO1_INPUT_REG & (1 << pin)) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW; // Read the specified pin value
}
