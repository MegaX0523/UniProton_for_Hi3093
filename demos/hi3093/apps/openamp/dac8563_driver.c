#include <stdint.h>
#include "gpio_def.h"
#include "gpio_driver.h"
#include "soft_spi_driver.h"
#include "bm_gpio.h"

// DAC8563 Commands and addresses
#define CMD_POWER_UP 0x20
#define CMD_LDAC_INACTIVE 0x30
#define CMD_RESET_GAIN  0x38
#define CMD_UPDATE_DAC_A 0x18
#define CMD_UPDATE_DAC_B 0x19

void dac8563_write(uint8_t cmd, uint16_t data)
{
    uint8_t transferdata[3];
    uint8_t receivedata[3];
    transferdata[0] = cmd; // Command byte
    transferdata[1] = (data >> 8) & 0xFF; // High byte of data
    transferdata[2] = data & 0xFF; // Low byte of data
    spi0_dac_transfer(transferdata, 3);
}

void dac8563_setvoltage(uint8_t channel, double voltage)
{
    uint16_t write_val = 0x0000;

    if(voltage < 0.0)
    {
        voltage = 0.0; // Ensure voltage is not negative
    }
    else if(voltage > 10.0)
    {
        voltage = 10.0; // Ensure voltage does not exceed 10V
    }
    write_val = voltage * 0x7FFF / 2 / 5;

    if (channel == 0)
    {
        dac8563_write(CMD_UPDATE_DAC_A, write_val);
    }
    else if (channel == 1)
    {
        dac8563_write(CMD_UPDATE_DAC_B, write_val);
    }
}

void dac8563_init(void)
{
    // SPI0_Init();  // Initialize GPIO pins for DAC8563

    gpio_init(GPIO_GROUP1, DAC8563_SYNC_PIN, 1);  // Output mode
    gpio_init(GPIO_GROUP1, DAC8563_LDAC_PIN, 1);
    GPIO_SET_PIN(DAC8563_SYNC_PIN);
    GPIO_SET_PIN(DAC8563_LDAC_PIN);
    
    // Power up DAC-A and DAC-B
    dac8563_write(CMD_POWER_UP, 0x0003);
    // Set LDAC inactive
    dac8563_write(CMD_LDAC_INACTIVE, 0x0003);
    // Update DAC-A and DAC-B
    dac8563_setvoltage(0, 0.0); // Set initial voltage for DAC-A
    dac8563_setvoltage(1, 0.0); // Set initial voltage for DAC-B
    // Reset gain to 2x
    dac8563_write(CMD_RESET_GAIN, 0x0001);
}