#pragma once

#include <stdint.h>

#define DAC_CONTROL_CHANNEL 0
#define DAC_EXCITATION_CHANNEL 1

extern void spi0_init(void);

extern void adc7606_init(void);
extern void adc7606_read_ref_signal(uint8_t* data)
extern void adc7606_read_err_signal(uint8_t* data);

extern void dac8563_init(void);
extern void dac8563_write(uint8_t cmd, uint16_t data);
extern void dac8563_setvoltage(uint8_t channel, double voltage);
