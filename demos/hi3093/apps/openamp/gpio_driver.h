#pragma once

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