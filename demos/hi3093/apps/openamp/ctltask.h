#ifndef __CTL_TASK_H
#define __CTL_TASK_H

#include "filter.h"


#define BUFF_LEN 100
#define PI 3.141592653
// #define CONT_Hz 13.67
// #define CONT_Hz 66.02
#define MSG_FORMAT "%d %d %lf %lf\r\n"

#define BARD_RATE115200 1
#define BARD_RATE921600 2
#define UART_IBRD_ADDR               (*(unsigned int *)0xFE201024ULL)
#define UART_FBRD_ADDR               (*(unsigned int *)0xFE201028ULL)
#define UART_LCRH_ADDR               (*(unsigned int *)0xFE20102CULL)
#define UART_CR_ADDR                 (*(unsigned int *)0xFE201030ULL)

void ControlTaskEntry();
int rec_msg_proc(void *data, int len);
extern void AD7606_Init(void);
extern void AD7606_StartConversion(void);
extern void AD7606_read_channel_1(uint8_t* data);
extern void AD7606_read_channel_2(uint8_t* data);
extern void DAC8563_Init(void);
extern void DAC8563_SetVoltage(uint8_t channel, double value);

#endif