#include <stdint.h>
#include "gpio_pin_define.h"
#include "gpio_driver.h"
#include "soft_spi_driver.h"
#include "bm_gpio.h"

// Initialize AD7606
void adc7606_init(void)
{
    // Initialize GPIO pins for AD7606
    gpio_init(GPIO_GROUP1, AD7606_CONVST_PIN, GPIO_OUTPUT);  // Initialize CONVST pin as output
    gpio_init(GPIO_GROUP1, AD7606_RESET_PIN, GPIO_OUTPUT);   // Initialize RESET pin as output
    gpio_init(GPIO_GROUP1,AD7606_BUSY_PIN, GPIO_INPUT);     // Initialize BUSY pin as input (optional, if needed)
    
    GPIO_SET_PIN(AD7606_RESET_PIN);   // Reset high
    for (volatile int i = 0; i < 100; i++);  // Simple delay loop (adjust as needed)
    GPIO_CLEAR_PIN(AD7606_RESET_PIN);   // Reset low
}

void adc7606_read_ref_signal(uint8_t* data)
{
    int timeout = 10;

    // start conversion
    GPIO_CLEAR_PIN(AD7606_CONVST_PIN);  // CONVST low to start conversion
    // for (volatile int i = 0; i < 100; i++);  // Simple delay loop (adjust as needed)
    GPIO_SET_PIN(AD7606_CONVST_PIN);  // CONVST back to high

    while (gpio_getvalue(AD7606_BUSY_PIN) == GPIO_LEVEL_HIGH)
    {
        if (timeout-- == 0) {
            // return;
            break;
        }
    }
    spi0_adc_receive(data, 2);
}

// Read all channels at once
void adc7606_read_err_signal(uint8_t* data)
{
    spi0_adc_receive(data, 2);
}