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
#include "reg_sysctrl.h"
#include "bm_common.h"
#include "bm_ops.h"

int bm_sysctrl_reg_read(unsigned int offset, unsigned int *val)
{
    if (val == NULL) {
        return BM_FAIL;
    }

    *val = bm_readl(SYSCTRL_CSR_BASE_ADDR + offset);

    return BM_OK;
}

int bm_sysctrl_reg_write(unsigned int offset, unsigned int val)
{
    bm_writel(val, SYSCTRL_CSR_BASE_ADDR + offset);

    return BM_OK;
}

int bm_sysctrl_reg_read_write(unsigned int offset, unsigned int mask, unsigned int val)
{
    unsigned int value;

    value = bm_readl(SYSCTRL_CSR_BASE_ADDR + offset);
    value &= ~mask;
    value |= (val & mask);
    bm_writel(value, SYSCTRL_CSR_BASE_ADDR + offset);

    return BM_OK;
}

int bm_sysctrl_reg_set_bits(unsigned int offset, unsigned int mask, unsigned int set_bit)
{
    unsigned int value;

    value = bm_readl(SYSCTRL_CSR_BASE_ADDR + offset);
    value &= ~mask;

    if (set_bit) {
        value |= mask;
    }

    bm_writel(value, SYSCTRL_CSR_BASE_ADDR + offset);

    return BM_OK;
}
