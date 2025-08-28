/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __BM_ATOMIC_H__
#define __BM_ATOMIC_H__

static inline int bm_atomic_read(unsigned int *addr)
{
    return *(volatile int *)addr;
}

static inline void bm_atomic_set(unsigned int *addr, int set_val)
{
    *(volatile int *)addr = set_val;
}

static inline int bm_atomic_add(unsigned int *addr, int add_val)
{
    int val;
    unsigned int status;

    do {
        __asm__ __volatile__("ldxr   %w1, %2\n"
                             "add   %w1, %w1, %w3\n"
                             "stxr   %w0, %w1, %2"
                             : "=&r"(status), "=&r"(val), "+Q"(*addr)
                             : "r"(add_val)
                             : "cc");
    } while (__builtin_expect(status != 0, 0));

    return val;
}

static inline int bm_cmp_xchg32bits(unsigned int *addr, int val, int old_val)
{
    int prev_val;
    unsigned int status;

    do {
        __asm__ __volatile__("1: ldxr %w0, %w2\n"
            "    mov %w1, #0\n"
            "    cmp %w0, %w3\n"
            "    b.ne 2f\n"
            "    stxr %w1, %w4, %w2\n"
            "2:"
            : "=&r"(prev_val), "=&r"(status), "+Q"(*addr)
            : "r"(old_val), "r"(val)
            : "cc");
    } while (__builtin_expect(status != 0, 0));

    return prev_val != old_val;
}

#endif