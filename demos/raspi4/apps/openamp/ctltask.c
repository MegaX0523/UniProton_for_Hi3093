#include <math.h>
#include "test.h"
#include "prt_task.h"
#include "rpmsg_backend.h"
#include "stdio.h"
#include "spi.h"
#include "prt_timer.h"
#include "prt_sem.h"
#include "ctltask.h"
#include "gpio_def.h"

volatile double ref_signal = 0;
volatile double err_signal = 0;
volatile int counts = 0;
static int task_flag = 0;
// static int ctlstart_flag = 0;
static U32 spitmierID;
static SemHandle taskstart_sem;
uint8_t AD7606_Data[16] = {0x0}; // Buffer to hold AD7606 data
// static SemHandle taskend_sem;

extern TskHandle g_testTskHandle;
extern TskHandle g_CtlTskHandel;
extern struct rpmsg_endpoint tty_ept;
extern double *InputArray;

void Timer_Init(void);
void UART_Init(int bard_rate);

static void Timer_Func(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    PRT_SemPost(taskstart_sem);
    return;
}

static double next_sine_sample_static(void)
{
    static double phase = 0.0;
    double value = sin(phase);
    phase += 2 * PI * CONT_Hz / 1000.0;
    // 归一化相位到0~2π之间，避免溢出
    while (phase >= 2 * PI)
    {
        phase -= 2 * PI;
    }
    return value;
}

void UART_Init(int bard_rate)
{
    // Set the UART control register to disable the UART
    UART_CR_ADDR = 0x00;

    if (bard_rate == BARD_RATE115200)
    {
        UART_IBRD_ADDR = 0x1a; 
        UART_FBRD_ADDR = 0x03; 
    }
    else if (bard_rate == BARD_RATE921600)
    {
        UART_IBRD_ADDR = 0x03; 
        UART_FBRD_ADDR = 0x10; 
    }
    UART_LCRH_ADDR = 0x70; // Set the line control register to 8 data bits, no parity, 1 stop bit, and FIFO enabled

    // Set the UART control register to enable the UART with 8 data bits, no parity, and 1 stop bit
    UART_CR_ADDR = 0x301; // Enable UART, 8 data bits, no parity, 1 stop bit

}

void Timer_Init(void)
{
    struct TimerCreatePara spitimer = {0};
    spitimer.type = OS_TIMER_SOFTWARE;
    spitimer.mode = OS_TIMER_LOOP;
    spitimer.interval = 1; 
    spitimer.timerGroupId = 0;
    spitimer.callBackFunc = Timer_Func;
    spitimer.arg1 = 0;

    PRT_SemCreate(0, &taskstart_sem);
    // PRT_SemMutexCreate(&taskstart_sem);
    PRT_TimerCreate(&spitimer, &spitmierID);
    PRT_TimerStart(0, spitmierID);
}

void ControlTaskEntry()
{
    int i = 0;
    int ret;
    U8 txbuff[4] = {0x00, 0x55, 0xAA, 0xFF};
    U8 rxbuff[4] = {0};
    U8 dummy[4] = {0x11, 0x22, 0x33, 0x44};
    U8 tx_buff[BUFF_LEN];
    int16_t AD_CH0, AD_CH1;
    double output = 0;
    double votlage0;
    double votlage1;
    double tmp_vol = 0;

    UART_Init(BARD_RATE921600);
    Timer_Init();
    FilterInit();
    SPI0_Init();
    AD7606_Init();
    DAC8563_Init();

    task_flag = 0;
    while (1)
    {
        PRT_SemPend(taskstart_sem, OS_WAIT_FOREVER);
        if (task_flag)
        {
            // send_message(tx_buff, ret);
            if (task_flag > 1)
            {
                SPI0_Set_CPHA1();
                tmp_vol = next_sine_sample_static();
                DAC8563_SetVoltage(1, 4 * tmp_vol + 5.0);
            }

            SPI0_Set_CPHA0();
            ret = AD7606_ReadAllChannels(AD7606_Data);
            AD_CH0 = (int16_t)((AD7606_Data[0] << 8) | AD7606_Data[1]);
            AD_CH1 = (int16_t)((AD7606_Data[2] << 8) | AD7606_Data[3]);
            votlage0 = (double)AD_CH0 * 10 / 0x7fff; // Assuming 16-bit ADC and 10.0V reference
            votlage1 = (double)AD_CH1 * 10 / 0x7fff; // Assuming 16-bit ADC and 10.0V reference

            if (task_flag > 2)
            {
                output = outputget(votlage0, votlage1);
                SPI0_Set_CPHA1();
                DAC8563_SetVoltage(0, (-1.0 * output) + 5.0);
            }

            PRT_Printf("$%.4f %.4f %.4f;", votlage0, votlage1, (-1.0 * output));
        }
    }
}

int rec_msg_proc(void *data, int len)
{
    char tx_buff[BUFF_LEN];
    int ret;
    int id;
    int count;
    double data1;
    double data2;
    // PRT_Printf("rec_msg_proc: %s\n\0", (char *)data);
    memcpy(tx_buff, data, len);
    tx_buff[len] = '\0';
    ret = sscanf(tx_buff, MSG_FORMAT, &id, &count, &data1, &data2);
    if (ret != 4)
    {
        return -1;
    }
    // PRT_Printf("rec_msg_proc: id=%d, count=%d, data1=%lf, data2=%lf\n", id, count, data1, data2);
    switch (id)
    {
    case MSG_CONTROL_START:
        if (task_flag == 0)
        {
            DAC8563_SetVoltage(0, 5.0);
            DAC8563_SetVoltage(1, 5.0);
        }
        task_flag++;
        PRT_TimerStart(0, spitmierID);
        break;
    case MSG_CONTROL_STOP:
        PRT_TimerStop(0, spitmierID);
        DAC8563_SetVoltage(0, 0.0);
        DAC8563_SetVoltage(1, 0.0);
        task_flag = 0;
        break;
    case MSG_CONTROL_EXIT:
        PRT_TimerStop(0, spitmierID);
        task_flag = 0;
        DAC8563_SetVoltage(0, 0.0);
        DAC8563_SetVoltage(1, 0.0);
        UART_Init(BARD_RATE115200);
        clear_env();
        return 1;
    case MSG_INPUTDATA:
        ref_signal = data1;
        err_signal = data2;
        counts = count;
        break;
    default:
        break;
    }
    return 0;
}

void clear_env()
{
    PRT_TimerStop(0, spitmierID);
    PRT_TimerDelete(0, spitmierID);
    PRT_SemDelete(taskstart_sem);
    PRT_TaskDelete(g_CtlTskHandel);
    rpmsg_backend_remove();
}
