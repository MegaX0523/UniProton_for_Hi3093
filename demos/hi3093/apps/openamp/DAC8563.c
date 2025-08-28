#include <stdint.h>
#include "gpio_def.h"
#include "hi_spi.h"
#include "bm_gpio.h"

// DAC8563 Commands and addresses
#define CMD_POWER_UP 0x20
#define CMD_LDAC_INACTIVE 0x30
#define CMD_RESET_GAIN  0x38
#define CMD_UPDATE_DAC_A 0x18
#define CMD_UPDATE_DAC_B 0x19

void DAC8563_Init(void);
void DAC8563_Write(uint8_t cmd, uint16_t data);
void DAC8563_SetVoltage(uint8_t channel, double value);

void DAC8563_Init(void)
{
    // SPI0_Init();  // Initialize GPIO pins for DAC8563

    GPIO_INIT(GPIO_GROUP1, DAC8563_SYNC_PIN, 1);  // Output mode
    GPIO_INIT(GPIO_GROUP1, DAC8563_LDAC_PIN, 1);
    GPIO_SET_PIN(DAC8563_SYNC_PIN);
    GPIO_SET_PIN(DAC8563_LDAC_PIN);
    
    // Power up DAC-A and DAC-B
    DAC8563_Write(CMD_POWER_UP, 0x0003);
    // Set LDAC inactive
    DAC8563_Write(CMD_LDAC_INACTIVE, 0x0003);
    // Update DAC-A and DAC-B
    DAC8563_SetVoltage(0, 0.0); // Set initial voltage for DAC-A
    DAC8563_SetVoltage(1, 0.0); // Set initial voltage for DAC-B
    // Reset gain to 2x
    DAC8563_Write(CMD_RESET_GAIN, 0x0001);
}

void DAC8563_Write(uint8_t cmd, uint16_t data)
{
    uint8_t transferdata[3];
    uint8_t receivedata[3];
    transferdata[0] = cmd; // Command byte
    transferdata[1] = (data >> 8) & 0xFF; // High byte of data
    transferdata[2] = data & 0xFF; // Low byte of data
    spi0_DA_transfer(transferdata, 3);
}

void DAC8563_SetVoltage(uint8_t channel, double voltage)
{
    static uint16_t write_val = 0x0000;

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
        DAC8563_Write(CMD_UPDATE_DAC_A, write_val);
    }
    else if (channel == 1)
    {
        DAC8563_Write(CMD_UPDATE_DAC_B, write_val);
    }
}