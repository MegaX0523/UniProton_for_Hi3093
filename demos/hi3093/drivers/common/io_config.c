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

#include "io_config.h"
#include "bm_common.h"
#include "bm_ops.h"

#define IO_CONFIG_REG_NUM 2

static unsigned long g_io_config_base[IO_CONFIG_REG_NUM] = {IO_CONFIG_R_BASE_ADDR, IO_CONFIG_T_BASE_ADDR};

static int bm_io_config_read(unsigned int type, unsigned int offset, unsigned int *val)
{
    if (val == NULL || type > IO_CONFIG_T) {
        return BM_FAIL;
    }

    *val = bm_readl(g_io_config_base[type] + offset);

    return BM_OK;
}

int bm_io_config_r_read(unsigned int offset, unsigned int *val)
{
    return bm_io_config_read(IO_CONFIG_R, offset, val);
}

int bm_io_config_t_read(unsigned int offset, unsigned int *val)
{
    return bm_io_config_read(IO_CONFIG_T, offset, val);
}

static int bm_io_config_write(unsigned int type, unsigned int offset, unsigned int val)
{
    if (type > IO_CONFIG_T) {
        return BM_FAIL;
    }

    bm_writel(val, (void *)(g_io_config_base[type] + offset));

    return BM_OK;
}

int bm_io_config_r_write(unsigned int offset, unsigned int val)
{
    return bm_io_config_write(IO_CONFIG_R, offset, val);
}

int bm_io_config_t_write(unsigned int offset, unsigned int val)
{
    return bm_io_config_write(IO_CONFIG_T, offset, val);
}

static int bm_io_config_read_write(unsigned int type, unsigned int offset, unsigned int mask, unsigned int val)
{
    if (type > IO_CONFIG_T) {
        return BM_FAIL;
    }

    unsigned int temp_val;

    temp_val = bm_readl(g_io_config_base[type] + offset);
    temp_val &= ~mask;
    temp_val |= (val & mask);
    bm_writel(temp_val, (void *)(g_io_config_base[type] + offset));

    return BM_OK;
}

int bm_io_config_r_read_write(unsigned int offset, unsigned int mask, unsigned int val)
{
    return bm_io_config_read_write(IO_CONFIG_R, offset, mask, val);
}

int bm_io_config_t_read_write(unsigned int offset, unsigned int mask, unsigned int val)
{
    return bm_io_config_read_write(IO_CONFIG_T, offset, mask, val);
}

static int bm_io_config_set_bits(unsigned int type, unsigned int offset, unsigned int mask, unsigned int set_bit)
{
    if (type > IO_CONFIG_T) {
        return BM_FAIL;
    }

    unsigned int val;

    val = bm_readl(g_io_config_base[type] + offset);
    val &= ~mask;

    if (set_bit) {
        val |= mask;
    }

    bm_writel(val, g_io_config_base[type] + offset);

    return BM_OK;
}

int bm_io_config_r_set_bits(unsigned int offset, unsigned int mask, unsigned int set_bit)
{
    return bm_io_config_set_bits(IO_CONFIG_R, offset, mask, set_bit);
}

int bm_io_config_t_set_bits(unsigned int offset, unsigned int mask, unsigned int set_bit)
{
    return bm_io_config_set_bits(IO_CONFIG_T, offset, mask, set_bit);
}
