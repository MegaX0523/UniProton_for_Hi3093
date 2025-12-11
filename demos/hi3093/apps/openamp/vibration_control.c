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

extern bool state_virabtion_control_flag;
extern bool state_excitation_flag;
extern bool state_control_flag;
extern bool state_secondary_path_identify_flag;

extern double mu_control;

static const uint16_t timer_interval = 1;
static uint16_t ref_array_index = 1;
static uint16_t err_array_index = 0;

int16_t wait_time_for_excit = 5000;  // 等5秒压电驱动器稳定
int32_t wait_time_for_control = 10000;  // 等5秒激励稳定
SensorArray ref_array = {0};
SensorArray err_array = {0};

static long tmp_count = 0;

typedef struct sin_args {
    double phase;
    double excitation_freq;
} sin_arg;

// sin_arg sin_param = {0.0, 87.2};
sin_arg sin_param = {0.0, 14.22};

extern int send_message(unsigned char* message, int len);

static void timer_callback(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4) {
    PRT_SemPost(taskstart_sem);
    tmp_count++;
    return;
}

void change_sin_pram(double freq) {
    sin_param.phase = 0.0;
    sin_param.excitation_freq = freq;
}

double get_sin_value(void) {
#define PI 3.141592653
    double value = sin(sin_param.phase);
    sin_param.phase += 2 * PI * sin_param.excitation_freq * (timer_interval / 1000.0);
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

void virabtion_control(void) {
    static double ref_signal = 0;
    static double err_signal = 0;
    static double exc_signal = 0;
    static double output_signal = 0;
    static int16_t ref_signal_raw = 0, err_signal_raw = 0;
    static uint8_t ad7606_buffer[2] = {0x0};
    // 施加激励
    if (state_excitation_flag) {
        exc_signal = 4.0 * get_sin_value();
        dac8563_setvoltage(DAC_EXCITATION_CHANNEL, exc_signal + OUTPUT_VOLTAGE_OFFSET);
    }

    // 获取参考信号
    adc7606_read_ref_signal(ad7606_buffer);
    ref_signal_raw = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
    ref_signal = (double)ref_signal_raw / 0x7fff;   // 输入的参考信号改为±1范围内

    if (state_control_flag && wait_time_for_control <= 0) {
        // 输出控制信号
        output_signal = output_get(ref_signal);
        output_signal = output_signal > 10.0 ? 10.0 : (output_signal < -10.0 ? -10.0 : output_signal);
        dac8563_setvoltage(DAC_CONTROL_CHANNEL, -1.0 * output_signal + OUTPUT_VOLTAGE_OFFSET);
    }
    // 获取误差信号
    adc7606_read_err_signal(ad7606_buffer);
    err_signal_raw = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
    err_signal = (double)err_signal_raw / 0x7fff;

    if (state_control_flag && wait_time_for_control <= 0) {
        weight_update(err_signal);
    }
    update_sensor_array(ref_signal_raw, err_signal_raw);

    if (wait_time_for_control > 0) {
        wait_time_for_control--;
    }
    static long count = 0;
    if (count % 2000 == 0) {
        // PRT_Printf("mu_control=%.6f\n", mu_control);
        PRT_Printf("%d ", tmp_count - count);
    }
    count++;
}

void secondary_path_identify() {
    double exc_signal;
    double err_signal;
    int16_t err_signal_raw;
    uint8_t ad7606_buffer[2] = {0x0};

    exc_signal = 4.0 * get_sin_value();
    dac8563_setvoltage(DAC_CONTROL_CHANNEL, exc_signal + OUTPUT_VOLTAGE_OFFSET);
    update_input_deque_only(exc_signal / 4);

    adc7606_read_ref_signal(ad7606_buffer);
    adc7606_read_err_signal(ad7606_buffer);
    err_signal_raw = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
    err_signal = (double)err_signal_raw / 0x7fff;
    weight_sep_update(err_signal);

    update_sensor_array((int16_t)(exc_signal * 0x7FFF / 10), err_signal_raw);
    static int count = 1000;
    if (count-- < 1) {
        PRT_Printf("%.6f, %.6f\n ", exc_signal, err_signal);
        count = 1000;
    }
}

void update_sensor_array(int16_t ref_signal_raw, int16_t err_signal_raw) {
    // 输出传感器数据到openEuler端
    if (ref_array_index >= REF_SIGNAL_ARRAY_SIZE) {
        ref_array_index = 0;
        rpmsg_packet pkt = {.msg_type = MSG_REF_ARRAY};
        __real_memcpy(pkt.payload.array, ref_array, sizeof(SensorArray));
        send_message(((uint8_t*)&pkt), sizeof(pkt.msg_type) + sizeof(SensorArray));
    } else {
        int16_t val = ref_signal_raw;
        if ((val & 0xFF) == 0x0A) {
            val++;
        }
        if ((val & 0xFF00) == 0x0A00) {
            val += 0x0100;
        }
        ref_array[ref_array_index++] = val;
    }

    // 输出误差数据到openEuler端
    if (err_array_index >= ERR_SIGNAL_ARRAY_SIZE) {
        err_array_index = 0;
        rpmsg_packet pkt = {.msg_type = MSG_ERR_ARRAY};
        __real_memcpy(pkt.payload.array, err_array, sizeof(SensorArray));
        send_message(((uint8_t*)&pkt), sizeof(pkt.msg_type) + sizeof(SensorArray));
    } else {
        int16_t val = err_signal_raw;
        if ((val & 0xFF) == 0x0A) {
            val++;
        }
        if ((val & 0xFF00) == 0x0A00) {
            val += 0x0100;
        }
        err_array[err_array_index++] = val;
    }
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

    while (1) {
        PRT_SemPend(taskstart_sem, OS_WAIT_FOREVER);
        if (state_virabtion_control_flag) {
            if (wait_time_for_excit > 0) {
                uint8_t ad7606_buffer[2] = {0x0};
                int16_t ref_signal_raw = 0, err_signal_raw = 0;
                adc7606_read_ref_signal(ad7606_buffer);
                ref_signal_raw = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
                adc7606_read_err_signal(ad7606_buffer);
                err_signal_raw = (int16_t)((ad7606_buffer[0] << 8) | ad7606_buffer[1]);
                update_sensor_array(ref_signal_raw, err_signal_raw);
                wait_time_for_excit--;
                continue;
            } else {
                virabtion_control();
            }
        } else if (state_secondary_path_identify_flag) {
            if (wait_time_for_excit > 0) {
                wait_time_for_excit--;
                continue;
            }
            secondary_path_identify();
        }
    }
}
