#include <math.h>
#include "test.h"
#include "prt_task.h"
#include "rpmsg_backend.h"
#include "stdio.h"
#include "hi_spi.h"
#include "prt_timer.h"
#include "prt_sem.h"
#include "ctltask.h"
#include "gpio_def.h"
#include "rpmsg_protocol.h"

#define OUTPUT_VOLTAGE_OFFSET 5.0

static U32 spi_timer_id;
static SemHandle taskstart_sem;
extern TskHandle g_testTskHandle;
extern TskHandle g_CtlTskHandel;

static int timer_interval = 500;

static bool state_idle_flag = true;        // 空闲状态
static bool state_excitation_flag = false; // 激励状态
static bool state_control_flag = false;
static bool state_secondary_path_identify_flag = false;

static uint sensor_array_index = 0;
static int sensor_array[200] = {0};

struct sin_args
{
    double phase = 0.0;
    double excitation_freq = 66.07;
} sin_arg;

void timer_init(void);

static void timer_callback(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    PRT_SemPost(taskstart_sem);
    return;
}

static double get_sin_value(void)
{
    double value = sin(sin_arg.phase);
    sin_arg.phase += 2 * PI * sin_arg.excitation_freq / timer_interval;
    while (sin_arg.phase >= 2 * PI)
    {
        sin_arg.phase -= 2 * PI;
    }
    return value;
}

void timer_init(void)
{
    struct TimerCreatePara spi_timer = {0};
    spi_timer.type = OS_TIMER_SOFTWARE;
    spi_timer.mode = OS_TIMER_LOOP;
    spi_timer.interval = 500;
    spi_timer.timerGroupId = 0;
    spi_timer.callBackFunc = timer_callback;
    spi_timer.arg1 = 0;

    PRT_SemCreate(0, &taskstart_sem);
    PRT_TimerCreate(&spi_timer, &spi_timer_id);
    PRT_TimerStart(0, spi_timer_id);
}

void ControlTaskEntry()
{
    timer_init();
    filter_init();
    spi0_init();
    ad7606_init();
    dac8563_init();

    while (1)
    {
        PRT_SemPend(taskstart_sem, OS_WAIT_FOREVER);

        if (state_idle_flag)
        {
            vibration_control();
        }
        else if (state_secondary_path_identify_flag)
        {
            secondary_path_identify();
        }
    }
}

void vibration_control()
{
    double ref_signal = 0;
    double err_signal = 0;
    double exc_signal = 0;
    double output_signal = 0;
    int16_t ad_ch0_value, ad_ch1_value;
    uint8_t ad7606_buffer[2] = {0x0};

    // 施加激励
    if (state_excitation_flag)
    {
        exc_signal = get_sin_value();
        DAC8563_SetVoltage(EXCITATION_CHANNEL, exc_signal + OUTPUT_VOLTAGE_OFFSET);
    }

    // 获取参考信号
    AD7606_read_channel_1(ad7606_buffer);
    ad_ch0_value = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
    ref_signal = (double)ad_ch0_value * 10 / 0x7fff;

    // 输出控制信号
    if (state_control_flag)
    {
        output_signal = output_get(ref_signal);
        DAC8563_SetVoltage(CONTROL_CHANNEL, output_signal + OUTPUT_VOLTAGE_OFFSET);

        AD7606_read_channel_2(ad7606_buffer);
        ad_ch1_value = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
        err_signal = (double)ad_ch1_value * 10 / 0x7fff;
        weight_update(err_signal);
        if (sensor_array_index >= SENSOR_ARRAY_SIZE)
        {
            sensor_array_index = 0;
            send_sensor_array();
        }
        else
        {
            sensor_array[sensor_array_index++] = ad_ch1_value;
        }
    }
}

void secondary_path_identify()
{
    double exc_signal;
    double err_signal;
    int16_t ad_ch1_value;
    uint8_t ad7606_buffer[2] = {0x0};

    exc_signal = get_sin_value();
    DAC8563_SetVoltage(CONTROL_CHANNEL, exc_signal); // 次级通道辨识把激励信号发控制作动器
    update_input_deque_only(exc_signal);

    AD7606_read_channel_2(ad7606_buffer);
    ad_ch1_value = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
    err_signal = (double)ad_ch1_value * 10 / 0x7fff;
    weight_sec_p_update(err_signal);
}

void send_sensor_array()
{
    rpmsg_packet pkt = {
        .msg_type = MSG_SENSOR_ARRAY};
    __real_memcpy(pkt.payload.array, sensor_array, sizeof(SensorArray));
    send_message(((uint8_t *)&pkt), sizeof(pkt.msg_type) + sizeof(SensorArray));
}

int rec_msg_proc(void *data, int len)
{
    rpmsg_packet *packet = (rpmsg_packet *)data;

    if (len < sizeof(uint16_t))
    {
        PRT_Printf("Invalid packet: too short");
        return -1;
    }
    switch (packet->msg_type)
    {
    case MSG_COMMAND:
        if (len = sizeof(uint16_t) + sizeof(uint16_t))
        {
            switch (packet->payload.command)
            {
            case START_EXCITATION: // 开始激励
                PRT_Printf("Received command: START_EXCITATION\n");
                DAC8563_SetVoltage(EXCITATION_CHANNEL, OUTPUT_VOLTAGE_OFFSET);
                DAC8563_SetVoltage(CONTROL_CHANNEL, OUTPUT_VOLTAGE_OFFSET);
                state_excitation_flag = 1;
                break;
            case STOP_EXCITATION: // 停止激励
                PRT_Printf("Received command: STOP_EXCITATION\n");
                state_excitation_flag = 0;
                state_control_flag = 0;
                DAC8563_SetVoltage(EXCITATION_CHANNEL, 0);
                DAC8563_SetVoltage(CONTROL_CHANNEL, 0);
                break;
            case START_CONTROL: // 开始控制
                PRT_Printf("Received command: START_CONTROL\n");
                if (state_excitation_flag == 1)
                {
                    state_control_flag = 1;
                }
            case STOP_CONTROL: //  停止控制
                PRT_Printf("Received command: STOP_CONTROL\n");
                state_control_flag = 0;
                DAC8563_SetVoltage(CONTROL_CHANNEL, 0);
            case START_DAMPING:
                PRT_Printf("Sending test array.\n", packet->payload.command);
                for (uint16_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
                {
                    sensor_array[i] = (int16_t)(get_sin_value(i) * 0x7FFF); // 示例数据
                    PRT_Printf("%d", sensor_array[i]);
                    if ((i + 1) % 16 == 0)
                        PRT_Printf("\n");
                }
                PRT_Printf("\n");
                send_sensor_array();
            }
        }
    case MSG_SET_PARAM:
        if (len >= sizeof(uint16_t) + sizeof(ParamPayload))
        {
            PRT_Printf("Received param: id=0x%02X, value=%.8f, len=%d\n",
                       packet->payload.param.param_id,
                       packet->payload.param.param_value,
                       len);
            if (packet->payload.param.param_id == PARAM_FREQUENCY)
            {
                PRT_TimerStop(0, spi_timer_id);
                sin_arg.phase = 0;
                sin_arg.excitation_freq = packet->payload.param.param_value;
                PRT_TimerStart(0, spi_timer_id);
            }
        }
        break;
    default:
        PRT_Printf("Unknown message type: 0x%02X\n", packet->msg_type);
        break;
    }
    return 0;
}

void clear_env()
{
    PRT_TimerStop(0, spi_timer_id);
    PRT_TimerDelete(0, spi_timer_id);
    PRT_SemDelete(taskstart_sem);
    PRT_TaskDelete(g_CtlTskHandel);
    rpmsg_backend_remove();
}
