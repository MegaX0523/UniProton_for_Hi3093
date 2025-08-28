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

#include "bm_common.h"

#define bm_core_mpid_cpuid(mpid) (((mpid) | ((mpid) >> 8)) & 0xFFU)

unsigned int bm_get_coreid(void)
{
    unsigned long long mpid;
    asm volatile("MRS  %0, MPIDR_EL1" : "=r"(mpid)::"memory", "cc");
    return bm_core_mpid_cpuid(mpid);
}

unsigned long long bm_get_tick(void)
{
    unsigned long long value;
    asm volatile("mrs %0, cntvct_el0" : "=r" (value));
    return value;
}

void udelay(unsigned long usec)
{
    volatile unsigned long long cur, end;
    unsigned long long cur_freq;
    unsigned long long cycles;

    /* calculate tick */
    asm volatile("mrs %0, cntfrq_el0" : "=r"(cur_freq));
    cycles = usec * cur_freq / 1000000; // 1s = 1000000us

    cur = bm_get_tick();
    end = cur + cycles;

    if (cur < end) {
        while (cur < end) {
            cur = bm_get_tick();
        }
    } else {
        /* Consider the problem of periodic count inversion. Although the count inversion can be implemented only
        in 64, 584, or 64 years, the inversion protection is required to ensure the software strictness. */
        /* Scenario where the current value is greater than the end value */
        while (cur > end) {
            cur = bm_get_tick();
        }
        /* The scene of chasing after coming over */
        while (cur < end) {
            cur = bm_get_tick();
        }
    }
}

