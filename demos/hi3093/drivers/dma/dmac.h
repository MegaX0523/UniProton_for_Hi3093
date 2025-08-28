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

#ifndef __DMAC_H__
#define __DMAC_H__

#include <bm_common.h>
#include "bm_ops.h"
#include "lp_subsys.h"
#include "bm_dmac.h"
#include "dmac_regs.h"
#include "dmac_union.h"
#include "soc_osal.h"

/* DMAC request port x, peripheral select register */
#define DMAC_REQ_SEL0_ADDR(src_dma_id)  (LP_SUBSYS_CSR_LPSC_DMAC_REQ_SEL0_OFFSET_ADDR + (src_dma_id) * 0x4)

/* Source addr register for channel m */
#define DMAC_SRC_ADDR_L(channel_id)     (DMAC_CMSRCADDR_0_OFFSET_ADDR + (channel_id) * 0x10)

/* Channel m destination addr register */
#define DMAC_DEST_ADDR_L(channel_id)    (DMAC_CMDESTADDR_0_OFFSET_ADDR + (channel_id) * 0x10)

/* Linked list entry register of channel m */
#define DMAC_LLI_ADDR(channel_id)       (DMAC_CMLLI_0_OFFSET_ADDR + (channel_id) * 0x10)

/* Control register for channel m */
#define DMAC_CONTROL_ADDR(channel_id)   (DMAC_CMCONTROL_0_OFFSET_ADDR + (channel_id) * 0x10)

/* Channel m configuration register */
#define DMAC_CONFIG_ADDR(channel_id)    (DMAC_CMCONFIG_0_OFFSET_ADDR + (channel_id) * 0x4)

/* The DMA supports only 32 peripherals. The peripheral ID ranges from 0 to 31.
    If the peripheral ID is greater than 31,
    you need to reconfigure the connection between the peripherals and the DMA. */
#define DMAC_PERIPHERAL_ID(ID)  (((ID) > 31) ? ((ID) % 32) : (ID))

#define DMAC_MASK_TRANSMISSION_INT 0x0
#define DMAC_CLEAN_TRANSMISSION_INT 0x0000FFFF

#define DMAC_CLOCK_MASK       BIT(16)
#define DMAC_CLOCK_ENABLE     BIT(16)
#define DMAC_CLOCK_DISABLE    BIT(16)

#define DMAC_INT_DISABLE                    0
#define DMAC_INT_ENABLE                     1

#define DMAC_CHANNEL_CNT       16          /* 16 DMA channels */
#define DMAC_COMPLETE_TIMEOUT  1000000

#define DMAC_READ 0
#define DMAC_WRITE 1

#define DMAC_COMPLETE                       1
#define DMAC_OK                             0
#define DMAC_FAIL                           (-1)
#define DMAC_TIMEOUT                        (-2)
#define DMAC_NO_CHANNEL                     (-3)

#define TIME_MS_50 50

typedef struct {
    bm_dmac_req src_req;
    bm_dmac_req dest_req;
    unsigned int src_addr;
    unsigned int dest_addr;
    unsigned int data_len;
    transfer_type trans_type;
} local_dmac_channel_cfg;

typedef struct {
    bm_dmac_req req;
    bm_dmac_callback callback;
} dmac_callback_info;

typedef struct {
    bm_dmac_req req;
    unsigned int addr;
} dmac_req_addr;

typedef struct {
    u_dmac_cmlli dmac_cmlli;
    u_dmac_cmcontrol dmac_cmcontrol;
    u_dmac_cmconfig dmac_cmconfig;
    local_dmac_channel_cfg local_channel_cfg;
} dmac_reg_info_s;

static inline unsigned int dmac_readl(int addr)
{
    return bm_readl(addr);
}

static inline void dmac_writel(unsigned int val, int addr)
{
    bm_writel(val, addr);
}

#endif /* __DMAC_H__ */