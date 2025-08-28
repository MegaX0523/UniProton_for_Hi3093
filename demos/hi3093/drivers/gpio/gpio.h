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
#include <bm_common.h>
#include "bm_types.h"

#ifndef __GPIO_H__
#define __GPIO_H__
#include "gpio_regs.h"
#include "bm_gpio.h"
#include "bm_common.h"
#include "io_config.h"

#define GPIO_MAX_GROUP_NUM      4
#define GPIO_GROUP_MAX_PIN_NUM  32
#define GPIO_GROUP3_MAX_PIN_NUM 31

#define GROUP3_INVALID_MASK (0x80000000)
#define GROUP4_INVALID_MASK (0x3FFFFFFF)

#define MAX_FUNC_CNT 8
#define GPIO_NAME_LEN 20

#define IO_MULTI_REG_TYPE_IOCFG_R 0
#define IO_MULTI_REG_TYPE_IOCFG_T 1

#define GPIO_TOTAL_MAX_PIN_NUM 160
#define GPIO_IRQ_TYPE 3

#define MAX_GPIO_CNT 127
#define FUNC_GPIO 1

#define GPIO_BIT30 BIT(30)
#define GPIO_BIT0 1U

#define REG_TYPE_IOR IO_CONFIG_R
#define REG_TYPE_IOT IO_CONFIG_T

#define clear_bit(val, mask) ((val) & (~(mask)))
#define set_bit(val, mask) ((val) | (mask))
#define gpio_clear_reg_bit(group, reg, pin_map) \
    do {                                        \
        unsigned int val;                       \
        val = gpio_read_reg(group, reg);        \
        val = clear_bit(val, pin_map);          \
        gpio_write_reg(group, reg, val);        \
    } while (0)
#define gpio_set_reg_bit(group, reg, pin_map) \
    do {                                      \
        unsigned int val;                     \
        val = gpio_read_reg(group, reg);      \
        val = set_bit(val, pin_map);          \
        gpio_write_reg(group, reg, val);      \
    } while (0)

#define gpio_set_reg_val(group, reg, pin_map, bitval) \
    do {                                                  \
        if ((bitval) == 0) {                              \
            gpio_clear_reg_bit(group, reg, pin_map);      \
        } else {                                          \
            gpio_set_reg_bit(group, reg, pin_map);        \
        }                                                 \
    } while (0)
#define gpio_get_reg_val(group, reg, pin_map) ((gpio_read_reg(group, reg) & (pin_map)) ? 1 : 0)

#define GRADE_BIT 4
#define GRADE4_MASK (0x3 << GRADE_BIT)
#define GRADE8_MASK (0x7 << GRADE_BIT)

#define IO_R_GPIO_NUM (64)
#define IO_T_GPIO_NUM (63)

#define IO_MAX_ID (126)

#define BYTE_NUM 4

#define GRADE4_MAX_VAL (0x3)
#define GRADE8_MAX_VAL (0x7)

#define GPIO_DIR_INPUT 0
#define GPIO_DIR_OUTPUT 1

#define LPSC_CLKEN_CTRL0 0x10
#define LPSC_CLKSTA_CTRL0 0x01BC

#define LPSC_GPIO_MASK 0x7C000000
#define LPSC_GPIO0_BIT 26

typedef struct {
    unsigned int pin;
    unsigned int s_pin_start;
    unsigned int s_pin_end;
    unsigned int func_cnt;
    unsigned int func[MAX_FUNC_CNT];
    unsigned int reg_type[MAX_FUNC_CNT];
    unsigned int reg_off[MAX_FUNC_CNT];
    unsigned int reg_mask[MAX_FUNC_CNT];
    unsigned int reg_val[MAX_FUNC_CNT];
} gpio_func_info;

typedef struct {
    gpio_isr isr;
    void *data;
} gpio_isr_info;

typedef struct {
    unsigned int init_flag;
    gpio_isr_info gpio_isr_hanler[GPIO_TOTAL_MAX_PIN_NUM];
} gpio_int_info;

typedef struct {
    unsigned long gpio_base;
    unsigned int irq[GPIO_MAX_GROUP_NUM];
    unsigned int irq_flag[GPIO_MAX_GROUP_NUM];
} gpio_device;

extern gpio_func_info g_gpio_func_tbl[MAX_GPIO_CNT];

#endif /* __GPIO_H__ */
