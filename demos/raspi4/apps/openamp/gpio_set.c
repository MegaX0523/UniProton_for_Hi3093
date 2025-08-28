#include <stdint.h>
#include "gpio_def.h"

// Register access macros
#define REG(addr) (*(volatile uint32_t *)(GPIO_BASE + addr))

// Function to initialize GPIO pin
void GPIO_INIT(int pin, int mode)
{
    int reg_offset = (pin / 10) * 4;     // Calculate GPFSEL register offset
    int bit_offset = (pin % 10) * 3;     // Calculate bit position within register
    
    uint32_t reg_value = REG(reg_offset);
    reg_value &= ~(0x7 << bit_offset);     // Clear current function
    reg_value |= (mode << bit_offset);    // Set new function
    REG(reg_offset) = reg_value;
}

// Function to set GPIO output value
void GPIO_SET(int pin, int value)
{
    if (value)
        GPSET0 |= (1 << pin); // Set the pin if value is 1
    else
        GPCLR0 |= (1 << pin); // Clear the pin if value is 0
}

// Function to read GPIO input value
int GPIO_GETVALUE(int pin)
{
    uint32_t level = REG(0x34);      // GPLEV0 register
    return (level & (1 << pin)) ? 1 : 0;
}