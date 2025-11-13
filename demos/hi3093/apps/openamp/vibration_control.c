#include "vibration_control.h"

#include <math.h>

#include "filter.h"
#include "prt_sem.h"
#include "prt_task.h"
#include "prt_timer.h"
#include "rpmsg_backend.h"
#include "rpmsg_protocol.h"
#include "soft_spi_device.h"
#include "test.h"

#define OUTPUT_VOLTAGE_OFFSET 5.0

static U32 spi_timer_id;
static SemHandle taskstart_sem;
extern TskHandle g_testTskHandle;
extern TskHandle g_CtlTskHandel;

extern bool state_idle_flag;
extern bool state_excitation_flag;
extern bool state_control_flag;
extern bool state_secondary_path_identify_flag;

static uint16_t timer_interval = 1;
static uint16_t sensor_array_index = 0;
static uint16_t error_array_index = 0;
SensorArray sensor_array = {0};
SensorArray error_array = {0};

typedef struct sin_args {
    double phase;
    double excitation_freq;
} sin_arg;

sin_arg sin_param = {0.0, 66.07};

extern int send_message(unsigned char* message, int len);

static void timer_callback(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4) {
    PRT_SemPost(taskstart_sem);
    return;
}

void change_sin_pram(double freq) {
    sin_param.phase = 0.0;
    sin_param.excitation_freq = freq;
}

double get_sin_value(void) {
#define PI 3.141592653

    double value = sin(sin_param.phase);
    sin_param.phase += 2 * PI * sin_param.excitation_freq / timer_interval;
    while (sin_param.phase >= 2 * PI) {
        sin_param.phase -= 2 * PI;
    }
    return value;
}

void timer_init(void) {
    struct TimerCreatePara spi_timer = {0};
    spi_timer.type = OS_TIMER_SOFTWARE;
    spi_timer.mode = OS_TIMER_LOOP;
    spi_timer.interval = timer_interval;
    spi_timer.timerGroupId = 0;
    spi_timer.callBackFunc = timer_callback;
    spi_timer.arg1 = 0;

    PRT_SemCreate(0, &taskstart_sem);
    PRT_TimerCreate(&spi_timer, &spi_timer_id);
    PRT_TimerStart(0, spi_timer_id);
}

void secondary_path_identify() {
    double exc_signal;
    double err_signal;
    int16_t err_signal_raw;
    uint8_t ad7606_buffer[2] = {0x0};

    exc_signal = get_sin_value();
    dac8563_setvoltage(DAC_CONTROL_CHANNEL,
                       exc_signal);  // 次级通道辨识把激励信号发控制作动器
    update_input_deque_only(exc_signal);

    adc7606_read_err_signal(ad7606_buffer);
    err_signal_raw = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
    err_signal = (double)err_signal_raw * 10 / 0x7fff;
    weight_sec_p_update(err_signal);
}

void send_sensor_array() {
    rpmsg_packet pkt = {.msg_type = MSG_SENSOR_ARRAY};
    __real_memcpy(pkt.payload.array, sensor_array, sizeof(SensorArray));
    send_message(((uint8_t*)&pkt), sizeof(pkt.msg_type) + sizeof(SensorArray));
}

void clear_env() {
    PRT_TimerStop(0, spi_timer_id);
    PRT_TimerDelete(0, spi_timer_id);
    PRT_SemDelete(taskstart_sem);
    PRT_TaskDelete(g_CtlTskHandel);
    rpmsg_backend_remove();
}

void control_task_entry() {
    filter_init();
    spi0_init();
    adc7606_init();
    dac8563_init();
    timer_init();

    static double ref_signal = 0;
    static double err_signal = 0;
    static double exc_signal = 0;
    static double output_signal = 0;
    static int16_t ref_signal_raw, err_signal_raw;
    static uint8_t ad7606_buffer[2] = {0x0};

    while (1) {
        PRT_SemPend(taskstart_sem, OS_WAIT_FOREVER);
        if (state_idle_flag) {  // vibration control
            // 施加激励
            if (state_excitation_flag) {
                exc_signal = 4.0 * get_sin_value();
                dac8563_setvoltage(DAC_EXCITATION_CHANNEL, exc_signal + OUTPUT_VOLTAGE_OFFSET);
            }

            // 获取参考信号
            adc7606_read_ref_signal(ad7606_buffer);
            ref_signal_raw = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
            ref_signal = (double)ref_signal_raw * 10 / 0x7fff;

            if (state_control_flag) {
                // 输出控制信号
                output_signal = output_get(ref_signal);
                if (output_signal > 10 || output_signal < -10) {
                    if (output_signal > 15 || output_signal < -15) {
                        PRT_Printf("\noutput_signal = %f, weight reinit!!!\n", output_signal);
                        filter_init();
                        continue;
                    } else {
                        output_signal = output_signal > 0 ? 10.0 : (output_signal < 0 ? -10.0 : output_signal);
                    }
                }
                dac8563_setvoltage(DAC_CONTROL_CHANNEL, output_signal + OUTPUT_VOLTAGE_OFFSET);

                // 获取误差信号
                adc7606_read_err_signal(ad7606_buffer);
                err_signal_raw = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
                err_signal = (double)err_signal_raw * 10 / 0x7fff;
                // weight_update(err_signal);
                weight_update(ref_signal - output_signal);
            }

            // 输出传感器数据到openEuler端
            if (sensor_array_index >= SENSOR_ARRAY_SIZE) {
                sensor_array_index = 0;
                send_sensor_array();
            } else {
                int16_t val = ref_signal_raw;
                if ((val & 0xFF) == 0x0A) {
                    val++;
                }
                if ((val & 0xFF00) == 0x0A00) {
                    val += 0x0100;
                }
                sensor_array[sensor_array_index++] = val;
            }

            // 输出误差数据到openEuler端
            static double sum = 0;
            if (error_array_index >= SENSOR_ARRAY_SIZE) {
                // PRT_Printf("ref_signal = %f, output_signal = %f, error value = %f\n",
                // ref_signal, output_signal, ref_signal - output_signal);
                PRT_Printf("ref_signal = %f, err_signal = %f\n", ref_signal, err_signal);
                error_array_index = 0;
                sum = 0;
            } else if (state_control_flag) {
                double val = ref_signal - output_signal;
                sum += val;
                error_array_index++;
            }
        } else if (state_secondary_path_identify_flag) {
            secondary_path_identify();
        }
    }
}
