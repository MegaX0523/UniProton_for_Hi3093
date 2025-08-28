#include "rpmsg_protocol.h"
#include "soft_spi_device.h"
#include "vibration_control.h"

#define OUTPUT_VOLTAGE_OFFSET 5.0

bool state_idle_flag = true;        // ¿ÕÏÐ×´Ì¬
bool state_excitation_flag = false; // ¼¤Àø×´Ì¬
bool state_control_flag = false;
bool state_secondary_path_identify_flag = false;

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
            case START_EXCITATION: // ¿ªÊ¼¼¤Àø
                PRT_Printf("Received command: START_EXCITATION\n");
                dac8563_setvoltage(DAC_EXCITATION_CHANNEL, OUTPUT_VOLTAGE_OFFSET);
                dac8563_setvoltage(DAC_CONTROL_CHANNEL, OUTPUT_VOLTAGE_OFFSET);
                state_excitation_flag = 1;
                break;
            case STOP_EXCITATION: // Í£Ö¹¼¤Àø
                PRT_Printf("Received command: STOP_EXCITATION\n");
                state_excitation_flag = 0;
                state_control_flag = 0;
                dac8563_setvoltage(DAC_EXCITATION_CHANNEL, 0);
                dac8563_setvoltage(DAC_CONTROL_CHANNEL, 0);
                break;
            case START_CONTROL: // ¿ªÊ¼¿ØÖÆ
                PRT_Printf("Received command: START_CONTROL\n");
                if (state_excitation_flag == 1)
                {
                    state_control_flag = 1;
                }
            case STOP_CONTROL: //  Í£Ö¹¿ØÖÆ
                PRT_Printf("Received command: STOP_CONTROL\n");
                state_control_flag = 0;
                dac8563_setvoltage(DAC_CONTROL_CHANNEL, 0);
            case START_DAMPING:
                PRT_Printf("Sending test array.\n", packet->payload.command);
                for (uint16_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
                {
                    sensor_array[i] = (int16_t)(get_sin_value(i) * 0x7FFF); // Ê¾ÀýÊý¾Ý
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
                sin_arg.phase = 0;
                sin_arg.excitation_freq = packet->payload.param.param_value;
            }
        }
        break;
    default:
        PRT_Printf("Unknown message type: 0x%02X\n", packet->msg_type);
        break;
    }
    return 0;
}