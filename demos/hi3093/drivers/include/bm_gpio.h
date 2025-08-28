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

#ifndef __BM_GPIO_H__
#define __BM_GPIO_H__
/**
* @defgroup bm_gpio bm_gpio
*/

typedef int (*gpio_isr)(void *);

typedef enum {
    GPIO_INT_ACTIVE_LOW = 0,
    GPIO_INT_ACTIVE_HIGH,
    GPIO_INT_ACTIVE_BUTT,
} bm_gpio_int_polarity;

typedef enum {
    GPIO_INT_LEVEL_TRIGGER = 0,
    GPIO_INT_EDGE_TRIGGER,
    GPIO_INT_TRIGGER_BUTT,
} bm_gpio_int_level;

typedef enum {
    GPIO_DRIVE_LEVEL0 = 0,
    GPIO_DRIVE_LEVEL1,
    GPIO_DRIVE_LEVEL2,
    GPIO_DRIVE_LEVEL3,
    GPIO_DRIVE_LEVEL4,
    GPIO_DRIVE_LEVEL5,
    GPIO_DRIVE_LEVEL6,
    GPIO_DRIVE_LEVEL7,
    GPIO_DRIVE_LEVEL_BUTT,
} bm_gpio_drive_level;

typedef enum {
    GPIO_GROUP0 = 0,
    GPIO_GROUP1,
    GPIO_GROUP2,
    GPIO_GROUP3,
    GPIO_GROUP_BUTT,
} gpio_group;

/* note: GPIO0/GPIO1/GPIO2 has 32 pin, GPIO3 has 31 pin */
typedef enum {
    GPIO_PIN_0 = 0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
    GPIO_PIN_16,
    GPIO_PIN_17,
    GPIO_PIN_18,
    GPIO_PIN_19,
    GPIO_PIN_20,
    GPIO_PIN_21,
    GPIO_PIN_22,
    GPIO_PIN_23,
    GPIO_PIN_24,
    GPIO_PIN_25,
    GPIO_PIN_26,
    GPIO_PIN_27,
    GPIO_PIN_28,
    GPIO_PIN_29,
    GPIO_PIN_30,
    GPIO_PIN_31,
    GPIO_PIN_BUTT,
} gpio_pin;

#define CORE_SYS_GPIO0_INTID 181
#define CORE_SYS_GPIO1_INTID 182
#define CORE_SYS_GPIO2_INTID 183
#define CORE_SYS_GPIO3_INTID 184
#define CORE_SYS_GPIO_INTID_BASE CORE_SYS_GPIO0_INTID

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO initialization.
 *
 * @par description:
 * GPIO initialization.
 *
 * @param group [in]: gpio group num
 * @param pin [in]: pin num
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_init(gpio_group group, gpio_pin pin);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO deinitialization.
 *
 * @par description:
 * GPIO deinitialization.
 *
 * @param group [in]: gpio group num
 * @param pin [in]: pin num
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_deinit(gpio_group group, gpio_pin pin);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO clock config.
 *
 * @par description:
 * GPIO clock config.
 *
 * @param group [in]: gpio group num
 * @param pin [in]: pin num
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_config_io_clock(gpio_group group, gpio_pin pin);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO iomux config.
 *
 * @par description:
 * GPIO iomux config.
 *
 * @param group [in]: gpio group num
 * @param pin [in]: pin num
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_config_io_reuse(gpio_group group, gpio_pin pin);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO set level.
 *
 * @par description:
 * GPIO set level.
 *
 * @param group [in]: gpio group num
 * @param pin [in]: pin num
 * @param value [in]: 0/1
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_set_level(gpio_group group, gpio_pin pin, unsigned int value);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO get level.
 *
 * @par description:
 * GPIO get level.
 *
 * @param group [in]: gpio group num
 * @param pin [in]: pin num
 * @param value [out]: 0/1
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_get_level(gpio_group group, gpio_pin pin, unsigned int *value);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO isr register.
 *
 * @par description:
 * GPIO isr register.
 *
 * @param group [in]: gpio group num
 * @param pin [in]: pin num
 * @param handler [in]: user handler
 * @param data [in]: user data
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_isr_register(gpio_group group, gpio_pin pin, gpio_isr handler, void *data);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO isr unregister.
 *
 * @par description:
 * GPIO isr unregister.
 *
 * @param group [in]: gpio group num
 * @param pin [in]: pin num
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_isr_unregister(gpio_group group, gpio_pin pin);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO interrupts config.
 *
 * @par description:
 * @brief GPIO interrupts config.
 *
 * @param group [in]: group num
 * @param pin [in]: pin num
 * @param trig_level [in]: bm_gpio_int_level
 * @param polarity [in]: bm_gpio_int_polarity
 *
 * @return BM_OK/BM_FAIL
 *
 */
int gpio_int_config(gpio_group group, gpio_pin pin, bm_gpio_int_level trig_level, bm_gpio_int_polarity pol);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO enable interrupts.
 *
 * @par description:
 * @brief GPIO enable interrupts.
 *
 * @param group [in]: group num
 * @param pin [in]: pin num
 * @param trig_level [in]: bm_gpio_int_level
 * @param polarity [in]: bm_gpio_int_polarity
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_enable_int(gpio_group group, gpio_pin pin, bm_gpio_int_level trig_level, bm_gpio_int_polarity pol);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO disable interrupts.
 *
 * @par description:
 * @brief GPIO disable interrupts.
 *
 * @param group [in]: group num
 * @param pin [in]: pin num
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_disable_int(gpio_group group, gpio_pin pin);

/**
 *
 * @ingroup bm_gpio
 * @brief GPIO set drive level.
 *
 * @par description:
 * GPIO set drive level.
 *
 * @param group [in]: gpio group num
 * @param pin [in]: pin num
 * @param drive_level [in]: bm_gpio_drive_level
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_gpio_set_drive_level(gpio_group group, gpio_pin pin, bm_gpio_drive_level drive_level);

int gpio_get_pin_map(unsigned int group, unsigned int pin, unsigned int *pin_map, unsigned int *pin_linear);

#endif /* __BM_GPIO_H__ */