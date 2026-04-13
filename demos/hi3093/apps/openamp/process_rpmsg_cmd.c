#include "filter.h"
#include "rpmsg_protocol.h"
#include "soft_spi_device.h"
#include "stdbool.h"
#include "vibration_control.h"

#define OUTPUT_VOLTAGE_OFFSET 5.0

bool state_virabtion_control_flag = false;  // ????��???
bool state_excitation_flag = false;         // ?��??��???
bool state_control_flag = false;
bool state_secondary_path_identify_flag = false;

extern SensorArray sensor_array;

extern double get_sin_value(void);
extern void change_sin_pram(double freq);
extern int16_t wait_time_for_excit;
extern int32_t wait_time_for_control;

int rec_msg_proc(void* data, int len) {
    rpmsg_packet* packet = (rpmsg_packet*)data;

    if (len < sizeof(uint16_t)) {
        PRT_Printf("Invalid packet: too short");
        return -1;
    }
    switch (packet->msg_type) {
        case MSG_COMMAND:
            if (len = sizeof(uint16_t) + sizeof(uint16_t)) {
                switch (packet->payload.command) {
                    case START_EXCITATION:  // ?????��??
                        PRT_Printf("Received command: START_EXCITATION\n");
                        state_secondary_path_identify_flag = false;
                        dac8563_setvoltage(DAC_EXCITATION_CHANNEL, OUTPUT_VOLTAGE_OFFSET);
                        dac8563_setvoltage(DAC_CONTROL_CHANNEL, OUTPUT_VOLTAGE_OFFSET);
                        state_virabtion_control_flag = true;
                        state_excitation_flag = true;
                        break;
                    case STOP_EXCITATION:  // ?????��??
                        PRT_Printf("Received command: STOP_EXCITATION\n");
                        state_virabtion_control_flag = false;
                        state_excitation_flag = false;
                        state_control_flag = false;
                        wait_time_for_excit = 5000;
                        wait_time_for_control = 10000;
                        dac8563_setvoltage(DAC_EXCITATION_CHANNEL, 0);
                        dac8563_setvoltage(DAC_CONTROL_CHANNEL, 0);
                        break;
                    case START_CONTROL:  // ????????
                        PRT_Printf("Received command: START_CONTROL\n");
                        filter_reinit();
                        if (state_excitation_flag == true) {
                            state_control_flag = true;
                        }
                        break;
                    case STOP_CONTROL:  //  ????????
                        PRT_Printf("Received command: STOP_CONTROL\n");
                        state_control_flag = 0;
                        wait_time_for_control = 10000;
                        dac8563_setvoltage(DAC_CONTROL_CHANNEL, 0);
                        double* weight_array = get_weight_array();
                        for (int i = 0; i < LMS_M; i++) {
                            PRT_Printf("%.4lf ", weight_array[i]);
                            if ((i + 1) % 16 == 0) {
                                PRT_Printf("\n");
                            }
                        }
                        break;
                    case START_SP_IDENTIFY:
                        PRT_Printf("Received command: START_SP_IDENTIFY\n");
                        state_excitation_flag = false;
                        state_control_flag = false;
                        state_virabtion_control_flag = false;
                        dac8563_setvoltage(DAC_EXCITATION_CHANNEL, 5);
                        dac8563_setvoltage(DAC_CONTROL_CHANNEL, 5);
                        state_secondary_path_identify_flag = true;
                        break;
                    case STOP_SP_IDENTIFY:
                        PRT_Printf("Received command: STOP_SP_IDENTIFY\n");
                        state_secondary_path_identify_flag = false;
                        dac8563_setvoltage(DAC_EXCITATION_CHANNEL, 0);
                        dac8563_setvoltage(DAC_CONTROL_CHANNEL, 0);
                        PRT_Printf("Secondary path identification stopped. Weight array:\n");
                        wait_time_for_excit = 5000;
                        double* weight_sep_array = get_weight_sep_array();
                        for (int i = 0; i < LMS_M; i++) {
                            PRT_Printf("%.8lf ", weight_sep_array[i]);
                            if ((i + 1) % 16 == 0) {
                                PRT_Printf("\n");
                            }
                        }
                        break;
                    case START_DAMPING:
                        PRT_Printf("Sending test array.\n", packet->payload.command);
                        for (uint16_t i = 0; i < REF_SIGNAL_ARRAY_SIZE; i++) {
                            double value = get_sin_value_mix();  // ????????
                            update_sensor_array(value * 0x7FFF / 10, 0x2345);
                            PRT_Printf("%.4lf ", value);
                            if ((i + 1) % 16 == 0) {
                                PRT_Printf("\n");
                            }
                        }
                        change_sin_pram(66.07);
                        PRT_Printf("\n");
                }
            }
            break;
        case MSG_SET_PARAM:
            if (len >= sizeof(uint16_t) + sizeof(ParamPayload)) {
                PRT_Printf("Received param: id=0x%02X, value=%.8f, len=%d\n", packet->payload.param.param_id,
                           packet->payload.param.param_value, len);
                if (packet->payload.param.param_id == PARAM_FREQUENCY) {
                    change_sin_pram(packet->payload.param.param_value);
                }
            }
            break;
        default:
            PRT_Printf("Unknown message type: 0x%02X\n", packet->msg_type);
            break;
    }
    return 0;
}