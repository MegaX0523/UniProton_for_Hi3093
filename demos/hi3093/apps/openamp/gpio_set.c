#include <stdint.h>
#include "gpio_def.h"
#include "bm_gpio.h"

static int tmp;

// Function to initialize GPIO pin
void GPIO_INIT(int group, int pin, int mode)
{
    
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

// Function to set GPIO output value
// void GPIO_SET_PIN(int pin)
// {
//     GPIO1_OUTPUT_REG |= (1 << pin); // Set the specified pin high
// }

// inline void GPIO_CLEAR_PIN(int pin)
// {
//     GPIO1_OUTPUT_REG &= ~(1 << pin); // Set the specified pin low
// }

// Function to read GPIO input value
inline int GPIO_GETVALUE(int pin)
{
    return (GPIO1_INPUT_REG & (1 << pin)) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW; // Read the specified pin value
}