#include "rpmsg_protocol.h"
#include "soft_spi_device.h"
#include "vibration_control.h"
#include "prt_sem.h"
#include "stdbool.h"

#define OUTPUT_VOLTAGE_OFFSET 5.0

bool state_idle_flag = true;        // 空闲状态
bool state_excitation_flag = false; // 激励状态
bool state_control_flag = false;
bool state_secondary_path_identify_flag = false;

static SemHandle send_array_sem;
static SemHandle receive_cmd_sem;

extern SensorArray sensor_array;

int cmd_len;
rpmsg_packet *packet;


extern double get_sin_value(void);
extern void change_sin_pram(double freq);

void rec_msg_proc(void *data, int len)
{
    packet = (rpmsg_packet *)data;
    cmd_len = len;
    PRT_SemPost(receive_cmd_sem);
}

void receive_msg_entry(void)
{
    PRT_SemCreate(0, receive_cmd_sem);

    while (1)
    {
        PRT_SemPend(receive_cmd_sem, OS_WAIT_FOREVER);
        if (cmd_len < sizeof(uint16_t))
        {
            PRT_Printf("Invalid packet: too short");
            return -1;
        }
        switch (packet->msg_type)
        {
        case MSG_COMMAND:
            if (cmd_len = sizeof(uint16_t) + sizeof(uint16_t))
            {
                switch (packet->payload.command)
                {
                case START_EXCITATION: // 开始激励
                    PRT_Printf("Received command: START_EXCITATION\n");
                    dac8563_setvoltage(DAC_EXCITATION_CHANNEL, OUTPUT_VOLTAGE_OFFSET);
                    dac8563_setvoltage(DAC_CONTROL_CHANNEL, OUTPUT_VOLTAGE_OFFSET);
                    state_excitation_flag = 1;
                    break;
                case STOP_EXCITATION: // 停止激励
                    PRT_Printf("Received command: STOP_EXCITATION\n");
                    state_excitation_flag = 0;
                    state_control_flag = 0;
                    dac8563_setvoltage(DAC_EXCITATION_CHANNEL, 0);
                    dac8563_setvoltage(DAC_CONTROL_CHANNEL, 0);
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
                    dac8563_setvoltage(DAC_CONTROL_CHANNEL, 0);
                case START_DAMPING:
                    PRT_Printf("Sending test array.\n", packet->payload.command);
                    for (uint16_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
                    {
                        double value = get_sin_value(); // 示例数据
                        sensor_array[i] = (int16_t)(value * 0x7FFF);
                        PRT_Printf("%.4lf ", value);
                        if ((i + 1) % 16 == 0)
                        {
                            PRT_Printf("\n");
                        }
                    }
                    change_sin_pram(66.07);
                    PRT_Printf("\n");
                    PRT_SemPost(send_array_sem);
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
                    change_sin_pram(packet->payload.param.param_value);
                }
            }
            break;
        default:
            PRT_Printf("Unknown message type: 0x%02X\n", packet->msg_type);
            break;
        }
    }
}


void sendarray_task_entry(void)
{
    PRT_SemCreate(0, send_array_sem);
    while (1)
    {
        PRT_SemPend(send_array_sem, OS_WAIT_FOREVER);
        rpmsg_packet pkt = {
            .msg_type = MSG_SENSOR_ARRAY};
        __real_memcpy(pkt.payload.array, sensor_array, sizeof(SensorArray));
        send_message(((uint8_t *)&pkt), sizeof(pkt.msg_type) + sizeof(SensorArray));
    }
}