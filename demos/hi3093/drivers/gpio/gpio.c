/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2024-2024. All rights reserved.
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 */

#include "gpio.h"
#include "lp_subsys.h"
#include "soc_osal.h"
#include "bm_ops.h"

static gpio_int_info g_gpio_int_obj = { 0 };

static const unsigned char g_grade[IO_MAX_ID + 1] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static const unsigned char g_epad_r[IO_R_GPIO_NUM] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19, 20, 21, 22, 23, 24, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 40, 41, 42, 43, 44, 45, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 64,
    66, 67, 68, 69
};

static const unsigned char g_epad_t[IO_T_GPIO_NUM] = {
    0, 1, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 56, 57, 58, 59, 60, 61, 62, 63, 64, 69, 70,
    71, 72, 73
};

static unsigned int g_gpio_int_tbl[GPIO_MAX_GROUP_NUM] = {
    CORE_SYS_GPIO0_INTID, CORE_SYS_GPIO1_INTID, CORE_SYS_GPIO2_INTID, CORE_SYS_GPIO3_INTID
};

static const int gpio_exist_array[GPIO_GROUP_BUTT] = {
    dt_device_exists(GPIO, GPIO0),
    dt_device_exists(GPIO, GPIO1),
    dt_device_exists(GPIO, GPIO2),
    dt_device_exists(GPIO, GPIO3),
};

static unsigned int g_gpio_pin_tbl[GPIO_MAX_GROUP_NUM] = {32, 32, 32, 31};

static unsigned int g_group_int_count[MAX_GPIO_CNT] = {0};

unsigned int gpio_read_reg(unsigned int group, unsigned int offset)
{
    return bm_readl(GPIO_BASE_ADDR + group * 0x1000 + offset);
}

void gpio_write_reg(unsigned int group, unsigned int offset, unsigned int val)
{
    bm_writel(val, GPIO_BASE_ADDR + group * 0x1000 + offset);
}

static int io_cfg_reg_read_write_by_type(unsigned int reg_type, unsigned int off, unsigned int mask, unsigned int val)
{
    int ret;
    switch (reg_type) {
        case REG_TYPE_IOR:
            ret = bm_io_config_r_read_write(off, mask, val);
            break;
        case REG_TYPE_IOT:
            ret = bm_io_config_t_read_write(off, mask, val);
            break;
        default:
            ret = BM_FAIL;
            bm_log("cfg reg type 0x%x err!\n", reg_type);
            break;
    }

    return ret;
}

static int gpio_group_pin_check(unsigned int group, unsigned int pin)
{
    if (group >= GPIO_MAX_GROUP_NUM) {
        return BM_FAIL;
    }

    if (pin > (g_gpio_pin_tbl[group] - 1)) {
        return BM_FAIL;
    }

    if (!gpio_exist_array[group]) {
        bm_log("gpio group(%u), pin(%u) error, forbidden to access\n", group, pin);
        return BM_FAIL;
    }
    return BM_OK;
}

int gpio_get_pin_map(unsigned int group, unsigned int pin, unsigned int *pin_map, unsigned int *pin_linear)
{
    if (gpio_group_pin_check(group, pin)) {
        return BM_FAIL;
    }

    if (pin_linear == NULL || pin_map == NULL) {
        return BM_FAIL;
    }

    for (unsigned int i = 0; i < group; i++) {
        *pin_linear += g_gpio_pin_tbl[i];
    }
    *pin_linear += pin;

    *pin_map = (1U << pin);

    return 0;
}

static int io_cfg_get_func_index(gpio_func_info *pin_set, unsigned int func_type, unsigned int *func_index)
{
    unsigned int i;

    /* Traverse the feature set to search for the specified feature configuration. */
    for (i = 0; i < pin_set->func_cnt; i++) {
        if (func_type == pin_set->func[i]) {
            break;
        }
    }

    if (pin_set->func_cnt == i) {
        bm_log("pin 0x%x func 0x%x not support cfg!\n", pin_set->pin, func_type);
        return BM_FAIL;
    }

    *func_index = i;

    return 0;
}

/* Sets the multiplexing mode of the io_mux pin_linear to gpio. */
static int gpio_set_reuse_by_pinfunc(unsigned int pin_linear, unsigned int func_type)
{
    int ret;
    gpio_func_info *pin_set = NULL;
    unsigned int func_index = 0;

    pin_set = &g_gpio_func_tbl[pin_linear];
    if (pin_set->pin != pin_linear) {
        bm_log("set is not order by pin_linear,please check(pin_linear 0x%x)!\n", pin_linear);
        return BM_FAIL;
    }

    ret = io_cfg_get_func_index(pin_set, func_type, &func_index);
    if (ret) {
        bm_log("pin_linear 0x%x func 0x%x not support cfg!\n", pin_linear, func_type);
        return BM_FAIL;
    }

    ret = io_cfg_reg_read_write_by_type(pin_set->reg_type[func_index], pin_set->reg_off[func_index],
        pin_set->reg_mask[func_index], pin_set->reg_val[func_index]);
    if (ret) {
        bm_log("pin_linear 0x%x func 0x%x write cfg err!\n", pin_linear, func_type);
        return ret;
    }

    return ret;
}

/* Sets LP_SUBSYS */
static int gpio_set_lp_subsys_reg(unsigned int group, unsigned int enable)
{
    if (group >= GPIO_MAX_GROUP_NUM) {
        bm_log("invalid gpio_group\n");
        return BM_FAIL;
    }
    unsigned int val = (1 << group);
    if (enable) {
        bm_lp_subsys_reg_read_write(LPSC_CLKEN_CTRL0, (val << LPSC_GPIO0_BIT), 1);
    } else {
        bm_lp_subsys_reg_read_write(LPSC_CLKEN_CTRL0, (val << LPSC_GPIO0_BIT), 0);
    }
    return BM_OK;
}

int bm_gpio_config_io_reuse(gpio_group group, gpio_pin pin)
{
    unsigned int pin_linear = 0;
    unsigned int pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    /* Set this parameter to GPIO multiplexing. */
    if (gpio_set_reuse_by_pinfunc(pin_linear, FUNC_GPIO)) {
        return BM_FAIL;
    }
    return BM_OK;
}

int bm_gpio_config_io_clock(gpio_group group, gpio_pin pin)
{
    unsigned int pin_linear, pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    /* Set LP_SUBSYS */
    if (gpio_set_lp_subsys_reg(group, 1)) {
        return BM_FAIL;
    }

    return BM_OK;
}

/* Sets the output level. */
int bm_gpio_set_level(gpio_group group, gpio_pin pin, unsigned int value)
{
    unsigned int pin_linear, pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    unsigned int output;
    output = ((value == 0) ? 0 : 1);
    gpio_set_reg_val(group, GPIO_INTEN_OFFSET_ADDR, pin_map, 0);
    gpio_set_reg_val(group, GPIO_DIR_OFFSET_ADDR, pin_map, GPIO_DIR_OUTPUT);
    gpio_set_reg_val(group, GPIO_OUTPUT_OFFSET_ADDR, pin_map, output);

    return BM_OK;
}
/* Obtains the input level. */
int bm_gpio_get_level(gpio_group group, gpio_pin pin, unsigned int *value)
{
    unsigned int pin_linear, pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    if (value == NULL) {
        return BM_FAIL;
    }

    gpio_set_reg_val(group, GPIO_INTEN_OFFSET_ADDR, pin_map, 0);
    gpio_set_reg_val(group, GPIO_DIR_OFFSET_ADDR, pin_map, GPIO_DIR_INPUT);
    *value = gpio_get_reg_val(group, GPIO_INPUT_OFFSET_ADDR, pin_map);

    return BM_OK;
}
/* Set Interrupt */
static int gpio_irq_handler(int irq, void *dev_id)
{
    unsigned int i, group_num, count_i, pin_mask, raw_int_status, gpio_num;

    (void)irq;
    group_num = *((unsigned int *)dev_id) - CORE_SYS_GPIO0_INTID;
    /* 2 feet in group 4, 31 feet in group 3, 32 feet in each of the remaining 3 groups */
    if (group_num < GPIO_GROUP3) {
        count_i = GPIO_GROUP_MAX_PIN_NUM;
    } else {
        count_i = GPIO_GROUP3_MAX_PIN_NUM;
    }

    for (i = 0; i < count_i; i++) {
        /* Only bits 30 and 31 of GPIO group 5 are valid. */
        pin_mask = (GPIO_BIT0 << i);
        raw_int_status = gpio_get_reg_val(group_num, GPIO_INTRAWSTATUS_OFFSET_ADDR, pin_mask);
        if (raw_int_status == 1) {
            /* Clear interrupt */
            gpio_set_reg_val(group_num, GPIO_INTCLR_OFFSET_ADDR, pin_mask, 1);

            /* Add the operation on the callback function when the GPIO receives the interrupt function. */
            gpio_num = group_num * GPIO_GROUP_MAX_PIN_NUM + i;
            if (g_gpio_int_obj.gpio_isr_hanler[gpio_num].isr) {
                (void)((g_gpio_int_obj.gpio_isr_hanler[gpio_num]).isr)(g_gpio_int_obj.gpio_isr_hanler[gpio_num].data);
            }
        }
    }
    return BM_OK;
}
int bm_gpio_isr_register(gpio_group group, gpio_pin pin, gpio_isr handler, void *data)
{
    unsigned int pin_linear = 0;
    unsigned int pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    g_gpio_int_obj.gpio_isr_hanler[pin_linear].isr = handler;
    g_gpio_int_obj.gpio_isr_hanler[pin_linear].data = data;
    return BM_OK;
}

int bm_gpio_isr_unregister(gpio_group group, gpio_pin pin)
{
    unsigned int pin_linear = 0;
    unsigned int pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    g_gpio_int_obj.gpio_isr_hanler[pin_linear].isr = NULL;
    g_gpio_int_obj.gpio_isr_hanler[pin_linear].data = NULL;
    return BM_OK;
}

static int gpio_int_config_common(gpio_group group, unsigned int pin_map,
    bm_gpio_int_level trig_level, bm_gpio_int_polarity pol)
{
    if ((trig_level != GPIO_INT_LEVEL_TRIGGER) && (trig_level != GPIO_INT_EDGE_TRIGGER)) {
        return BM_FAIL;
    }

    if ((pol != GPIO_INT_ACTIVE_LOW) && (pol != GPIO_INT_ACTIVE_HIGH)) {
        return BM_FAIL;
    }

    /* Configure the GPIO as a normal GPIO. */
    gpio_set_reg_val(group, GPIO_INTEN_OFFSET_ADDR, pin_map, 0);

    /* The direction is changed to input. */
    gpio_set_reg_val(group, GPIO_DIR_OFFSET_ADDR, pin_map, 0);

    /* Setting Edges */
    gpio_set_reg_val(group, GPIO_INTTYPE_OFFSET_ADDR, pin_map, trig_level);

    /* Set Polarity */
    gpio_set_reg_val(group, GPIO_INTPOLARITY_OFFSET_ADDR, pin_map, pol);

    /* Clear interrupt */
    gpio_set_reg_val(group, GPIO_INTCLR_OFFSET_ADDR, pin_map, 1);

    /* Set Clear Burr */
    gpio_set_reg_val(group, GPIO_INTDEBOUNCE_OFFSET_ADDR, pin_map, 1);

    /* Effect of Clear Deglitch on GPIO */
    udelay(10); // delay 10 us

    return BM_OK;
}

int gpio_int_config(gpio_group group, gpio_pin pin, bm_gpio_int_level trig_level, bm_gpio_int_polarity pol)
{
    unsigned int pin_linear = 0;
    unsigned int pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    if (gpio_int_config_common(group, pin_map, trig_level, pol)) {
        return BM_FAIL;
    }

    return BM_OK;
}

int bm_gpio_enable_int(gpio_group group, gpio_pin pin, bm_gpio_int_level trig_level, bm_gpio_int_polarity pol)
{
    unsigned int pin_linear, pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    if (gpio_int_config_common(group, pin_map, trig_level, pol)) {
        return BM_FAIL;
    }

    /* Interrupt mask. 0: not mask; 1: mask. */
    gpio_set_reg_val(group, GPIO_INTMASK_OFFSET_ADDR, pin_map, 0);

    /* Enable Interrupt */
    gpio_set_reg_val(group, GPIO_INTEN_OFFSET_ADDR, pin_map, 1);
    return BM_OK;
}

int bm_gpio_disable_int(gpio_group group, gpio_pin pin)
{
    unsigned int pin_linear, pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    /* Interrupt mask. 0: not mask; 1: mask. */
    gpio_set_reg_val(group, GPIO_INTMASK_OFFSET_ADDR, pin_map, 1);
    gpio_set_reg_val(group, GPIO_INTEN_OFFSET_ADDR, pin_map, 0);
    return BM_OK;
}

int bm_gpio_set_drive_level(gpio_group group, gpio_pin pin, bm_gpio_drive_level drive_level)
{
    unsigned int pin_linear = 0;
    unsigned int pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }
    int ret;
    unsigned int offset, mask, type, gpio_id;
    unsigned int val = drive_level;

    if (drive_level >= GPIO_DRIVE_LEVEL_BUTT || drive_level < GPIO_DRIVE_LEVEL0) {
        return BM_FAIL;
    }

    if (g_grade[pin_linear] && (val > GRADE8_MAX_VAL)) {
        bm_log("gpio%u exceeds the configurable(0--7) drive level(%u)\n", pin_linear, val);
        return BM_FAIL;
    } else if (!g_grade[pin_linear] && (val > GRADE4_MAX_VAL)) {
        bm_log("gpio%u exceeds the configurable(0--3) drive level(%u)\n", pin_linear, val);
        return BM_FAIL;
    }

    gpio_id = (pin_linear > IO_T_GPIO_NUM) ? (pin_linear - IO_R_GPIO_NUM) : pin_linear;
    type = (pin_linear >= IO_R_GPIO_NUM) ? IO_MULTI_REG_TYPE_IOCFG_T : IO_MULTI_REG_TYPE_IOCFG_R;
    switch (type) {
        case IO_CONFIG_R:
            offset = BYTE_NUM * g_epad_r[gpio_id] + IO_CONFIG_R_PADMG000_OFFSET_ADDR;
            mask = g_grade[pin_linear] ? GRADE8_MASK : GRADE4_MASK;
            val = (val << GRADE_BIT);
            ret = bm_io_config_r_read_write(offset, mask, val);
            break;

        case IO_CONFIG_T:
            offset = BYTE_NUM * g_epad_t[pin_linear - IO_R_GPIO_NUM] + IO_CONFIG_T_PADMG000_OFFSET_ADDR;
            mask = g_grade[pin_linear] ? GRADE8_MASK : GRADE4_MASK;
            val = (val << GRADE_BIT);
            ret = bm_io_config_t_read_write(offset, mask, val);
            break;

        default:
            ret = BM_FAIL;
    }

    return ret;
}

/* init */
int bm_gpio_init(gpio_group group, gpio_pin pin)
{
    unsigned int pin_linear = 0;
    unsigned int pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    /* Set LP_SUBSYS */
    gpio_set_lp_subsys_reg(group, 1);
    /* Set this parameter to GPIO multiplexing. */
    if (gpio_set_reuse_by_pinfunc(pin_linear, FUNC_GPIO)) {
        return BM_FAIL;
    }

    // osal_irq_set_priority(CORE_SYS_GPIO0_INTID + group, 6); // pri = 6
    // osal_irq_request(CORE_SYS_GPIO0_INTID + group, gpio_irq_handler, NULL, NULL, &g_gpio_int_tbl[group]);
    // g_group_int_count[pin_linear]++;

    // bm_gpio_set_level(group, pin, 0);
    return BM_OK;
}

/* deinit */
int bm_gpio_deinit(gpio_group group, gpio_pin pin)
{
    unsigned int pin_linear = 0;
    unsigned int pin_map;
    if (gpio_get_pin_map(group, pin, &pin_map, &pin_linear)) {
        bm_log("gpio group=%u pin=%u out of range\n", group, pin);
        return BM_FAIL;
    }

    /* set LP_SUBSYS */
    gpio_set_lp_subsys_reg(group, 0);
    /* Disable the interrupt. */
    bm_gpio_disable_int(group, pin);
    if (g_group_int_count[pin_linear]) {
        g_group_int_count[pin_linear]--;
    }
    if (g_group_int_count[pin_linear] == 0) {
        osal_irq_free(CORE_SYS_GPIO0_INTID + group, NULL);
    }
    return BM_OK;
}
