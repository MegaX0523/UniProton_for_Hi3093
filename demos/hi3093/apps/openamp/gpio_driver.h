#pragma once

#define GPIO1_OUTPUT_REG *(volatile uint32_t*)(GPIO1_BASE_ADDR + GPIO_OUTPUT_OFFSET_ADDR)
#define GPIO1_INPUT_REG *(volatile uint32_t*)(GPIO1_BASE_ADDR + GPIO_INPUT_OFFSET_ADDR)

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

void gpio_init(int group, int pin, int mode);
int gpio_getvalue(int pin);