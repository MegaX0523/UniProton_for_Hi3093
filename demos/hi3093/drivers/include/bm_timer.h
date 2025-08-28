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
#ifndef __BM_TIMER_H__
#define __BM_TIMER_H__

#include "bm_types.h"

typedef enum {
    TIMER_ID_0 = 0,
    TIMER_ID_1,
    TIMER_ID_2,
    TIMER_ID_3,
    TIMER_ID_4,
    TIMER_ID_5,
    TIMER_ID_BUTT,
} bm_timer_ids;

#define TIMER_FRQ 100000000
#define TIMER_BASE_IRQ 175
/**
 * @brief Period min value
 */
#define PERIOD_MIN_VALUE 1

/**
 * @brief  timer operating mode definition
 */
typedef enum {
    TIMER_MODE_RUN_FREE = 0x00000000U,
    TIMER_MODE_RUN_PERIODIC = 0x00000001U,
    TIMER_MODE_RUN_ONTSHOT = 0x00000002U,
} timer_mode;

/**
 * @brief  timer division factor definition
 */
typedef enum {
    TIMERPRESCALER_NO_DIV = 0x00000000U,
    TIMERPRESCALER_DIV_16 = 0x00000001U,
    TIMERPRESCALER_DIV_256 = 0x00000002U,
    TIMERPRESCALER_DIV_RSVD = 0x00000003U,
} timer_prescaler_factor;

/**
 * @brief  timer couter size definition
 */
typedef enum {
    TIMER_SIZE_32BIT = 0x00000000U,
    TIMER_SIZE_64BIT = 0x00000001U,
} timer_size;

/**
 * @brief  typedef callback function of timer
 */
typedef void (*timer_callback_func)(int irq, uintptr_t param);

/**
 * @brief  time base addr and configuration structure definition
 */
typedef struct {
    bm_timer_ids id;                  /* *<timer id */
    unsigned int load_l;        /* *< period, set the timer_load low 32bit */
    unsigned int load_h;        /* *< period, set the timer_load high 32bit */
    timer_mode mode;                  /* *< timer counting mode */
    unsigned int interrupten;         /* *< interrupt enable or disable */
    timer_prescaler_factor prescaler; /* *< timer prescaler */
    timer_size size;                  /* *< timer size 32 or 64 bits */
    unsigned int bgload_l;      /* *< Backgroud period, set the timer_bgload low 32bit */
    unsigned int bgload_h;      /* *< Backgroud period, set the timer_bgload high 32bit */
    timer_callback_func callback;     /* *< callback function of timer */
    uintptr_t param;                  /* *< callback param */
} bm_timer_cfg;

/**
 * @brief   init the timer
 * @param   timer cfg
 * @retval  BM_OK    success
 * @retval  BM_FAIL parameter check fail
 */
int bm_timer_init(const bm_timer_cfg *cfg);

/**
 * @brief   deinit the timer
 * @param   timer id
 * @retval  BM_OK    success
 * @retval  BM_FAIL parameter check fail
 */
int bm_timer_deinit(bm_timer_ids id);

/**
 * @brief   start timer.
 * @param   timer id
 * @retval  BM_OK    success
 * @retval  BM_FAIL parameter check fail
 */
int bm_timer_start(bm_timer_ids id);

/**
 * @brief   stop timer.
 * @param   timer id
 * @retval  BM_OK    success
 * @retval  BM_FAIL parameter check fail
 */
int bm_timer_stop(bm_timer_ids id);

/**
 * @brief   modify timer load value
 * @param   timer id
 * @param   load_l value [!= 0]
 * @param   load_h value
 * @retval  BM_OK    success
 * @retval  BM_FAIL parameter check fail
 */
int bm_timer_modify_load_value(bm_timer_ids id, unsigned int load_l, unsigned int load_h);

/**
 * @brief   modify timer bgload value
 * @param   timer id
 * @param   bgload_l value
 * @param   bgload_h value
 * @retval  BM_OK    success
 * @retval  BM_FAIL parameter check fail
 */
int bm_timer_modify_bgload_value(bm_timer_ids id, unsigned int bgload_l, unsigned int bgload_h);

/**
 * @brief   get timer value
 * @in param   timer id
 * @out param  timer cnt value
 * @retval  BM_OK    success
 * @retval  BM_FAIL parameter check fail
 */
int bm_timer_get_cnt_value(bm_timer_ids id, unsigned int *val_l, unsigned int *val_h);

/**
 * @brief   clear timer interrrupt.
 * @param   timer id
 * @retval  BM_OK    success
 * @retval  BM_FAIL parameter check fail
 */
int bm_timer_irq_clear(bm_timer_ids id);
#endif /* __BM_TIMER_H__ */
