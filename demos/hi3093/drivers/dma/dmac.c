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

#include "dmac.h"
#include "bm_cache.h"

static osal_semaphore g_sem;
static dmac_callback_info g_callback_info[DMAC_CHANNEL_CNT] = { 0 };
static int g_dmac_start_flag[DMAC_CHANNEL_CNT] = { 0 };
static int g_dmac_sync_complete_flag[DMAC_CHANNEL_CNT] = { 0 };
static int g_dmac_sync_status[DMAC_CHANNEL_CNT] = { 0 };
static int g_sem_init = 0;

static const int g_dma_exist_array[DMAC_CHANNEL_CNT] = {
    dt_device_exists(DMAC, CHANNEL0),
    dt_device_exists(DMAC, CHANNEL1),
    dt_device_exists(DMAC, CHANNEL2),
    dt_device_exists(DMAC, CHANNEL3),
    dt_device_exists(DMAC, CHANNEL4),
    dt_device_exists(DMAC, CHANNEL5),
    dt_device_exists(DMAC, CHANNEL6),
    dt_device_exists(DMAC, CHANNEL7),
    dt_device_exists(DMAC, CHANNEL8),
    dt_device_exists(DMAC, CHANNEL9),
    dt_device_exists(DMAC, CHANNEL10),
    dt_device_exists(DMAC, CHANNEL11),
    dt_device_exists(DMAC, CHANNEL12),
    dt_device_exists(DMAC, CHANNEL13),
    dt_device_exists(DMAC, CHANNEL14),
    dt_device_exists(DMAC, CHANNEL15),
};

static const dmac_req_addr g_dma_req_addr_list[] = {
    {
        .req  = DMAC_REQ_SEL_SPI0_TX,
        .addr = 0x8713060,
    },
    {
        .req  = DMAC_REQ_SEL_SPI0_RX,
        .addr = 0x8713060,
    },
    {
        .req  = DMAC_REQ_SEL_I2C0_TX,
        .addr = 0x8707010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C0_RX,
        .addr = 0x8707010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C1_TX,
        .addr = 0x8708010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C1_RX,
        .addr = 0x8708010,
    },
    {
        .req  = DMAC_REQ_SEL_UART2_TX,
        .addr = 0x08710000,
    },
    {
        .req  = DMAC_REQ_SEL_UART2_RX,
        .addr = 0x08710000,
    },
    {
        .req  = DMAC_REQ_SEL_UART3_TX,
        .addr = 0x08711000,
    },
    {
        .req  = DMAC_REQ_SEL_UART3_RX,
        .addr = 0x08711000,
    },
    {
        .req  = DMAC_REQ_SEL_UART4_TX,
        .addr = 0x08743000,
    },
    {
        .req  = DMAC_REQ_SEL_UART4_RX,
        .addr = 0x08743000,
    },
    {
        .req  = DMAC_REQ_SEL_I2C2_TX,
        .addr = 0x8709010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C2_RX,
        .addr = 0x8709010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C3_TX,
        .addr = 0x870a010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C3_RX,
        .addr = 0x870a010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C4_TX,
        .addr = 0x870b010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C4_RX,
        .addr = 0x870b010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C5_TX,
        .addr = 0x870c010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C5_RX,
        .addr = 0x870c010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C6_TX,
        .addr = 0x870d010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C6_RX,
        .addr = 0x870d010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C7_TX,
        .addr = 0x870e010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C7_RX,
        .addr = 0x870e010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C8_TX,
        .addr = 0x8750010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C8_RX,
        .addr = 0x8750010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C9_TX,
        .addr = 0x8751010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C9_RX,
        .addr = 0x8751010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C10_TX,
        .addr = 0x8752010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C10_RX,
        .addr = 0x8752010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C11_TX,
        .addr = 0x8753010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C11_RX,
        .addr = 0x8753010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C12_TX,
        .addr = 0x8754010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C12_RX,
        .addr = 0x8754010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C13_TX,
        .addr = 0x8755010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C13_RX,
        .addr = 0x8755010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C14_TX,
        .addr = 0x8756010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C14_RX,
        .addr = 0x8756010
    },
    {
        .req  = DMAC_REQ_SEL_I2C15_TX,
        .addr = 0x8757010,
    },
    {
        .req  = DMAC_REQ_SEL_I2C15_RX,
        .addr = 0x8757010,
    },
    {
        .req  = DMAC_REQ_SEL_UART0_TX,
        .addr = 0x0872A000,
    },
    {
        .req  = DMAC_REQ_SEL_UART0_RX,
        .addr = 0x0872A000,
    },
    {
        .req  = DMAC_REQ_SEL_UART1_TX,
        .addr = 0x0872B000,
    },
    {
        .req  = DMAC_REQ_SEL_UART1_RX,
        .addr = 0x0872B000,
    },
    {
        .req  = DMAC_REQ_SEL_UART5_TX,
        .addr = 0x08744000,
    },
    {
        .req  = DMAC_REQ_SEL_UART5_RX,
        .addr = 0x08744000,
    },
    {
        .req  = DMAC_REQ_SEL_UART6_TX,
        .addr = 0x0875D000,
    },
    {
        .req  = DMAC_REQ_SEL_UART6_RX,
        .addr = 0x0875D000,
    },
    {
        .req  = DMAC_REQ_SEL_UART7_TX,
        .addr = 0x0875E000,
    },
    {
        .req  = DMAC_REQ_SEL_UART7_RX,
        .addr = 0x0875E000,
    },
    {
        .req  = DMAC_REQ_SEL_SPI1_TX,
        .addr = 0x8715060
    },
    {
        .req  = DMAC_REQ_SEL_SPI1_RX,
        .addr = 0x8715060
    },
};

int bm_dmac_config_io_clock(bm_dmac_clock_t clock)
{
    unsigned int reg_val;
    int clock_status;

    bm_lp_subsys_reg_read(LP_SUBSYS_CSR_LPSC_CLKSTA_CTRL1_OFFSET_ADDR, &reg_val);

    /* Shift rightwards by 16 bits to obtain the clock gating information. */
    clock_status = (DMAC_CLOCK_MASK & reg_val) >> 16;

    if ((clock == DMAC_CLOCK_OPEN)) {
        if (!clock_status) {
            bm_lp_subsys_reg_read_write(LP_SUBSYS_CSR_LPSC_CLKEN_CTRL1_OFFSET_ADDR,
                DMAC_CLOCK_MASK, DMAC_CLOCK_ENABLE);
        }
    } else if ((clock == DMAC_CLOCK_CLOSE)) {
        if (clock_status) {
            bm_lp_subsys_reg_read_write(LP_SUBSYS_CSR_LPSC_CLKDIS_CTRL1_OFFSET_ADDR,
                DMAC_CLOCK_MASK, DMAC_CLOCK_DISABLE);
        }
    } else {
        bm_log("Failed to configure the DMA clock, clock val = (0x%x)\n", clock);
        return BM_FAIL;
    }

    return BM_OK;
}

static int bm_dmac_init_check(void)
{
    unsigned int reg_val;
    int clock_status;
    u_dmac_g_en dmac_g_en;

    bm_lp_subsys_reg_read(LP_SUBSYS_CSR_LPSC_CLKSTA_CTRL1_OFFSET_ADDR, &reg_val);

    /* Shift rightwards by 16 bits to obtain the clock gating information. */
    clock_status = (DMAC_CLOCK_MASK & reg_val) >> 16;
    if (!clock_status) {
        return BM_FAIL;
    }

    dmac_g_en.value = dmac_readl(DMAC_BASE_ADDR + DMAC_G_EN_OFFSET_ADDR);
    if (dmac_g_en.bits.dmac_g_en != 1) {
        return BM_FAIL;
    }

    return BM_OK;
}

static int bm_dmac_interrupt_handle(int irq, void *arg)
{
    unsigned int channel_status;
    unsigned int dmac_intr_err;
    unsigned int dmac_intr_complete;
    int int_status;
    dmac_callback_info callback_info;
    (void)irq;
    (void)arg;
    /* The DMAC has three groups of interrupt registers. The first group is used. */
    channel_status = dmac_readl(DMAC_BASE_ADDR + DMAC_INT_1_OFFSET_ADDR);
    dmac_intr_err = dmac_readl(DMAC_BASE_ADDR + DMAC_INT_ERR_1_OFFSET_ADDR);
    dmac_intr_complete = dmac_readl(DMAC_BASE_ADDR + DMAC_INT_TC_1_OFFSET_ADDR);

    for (int i = 0; i < DMAC_CHANNEL_CNT; i++) {
        /* Check the interrupt status of each channel. */
        if ((channel_status >> i) & 0x1) {
            if (((dmac_intr_complete >> i) & 0x1) ^ ((dmac_intr_err >> i) & 0x1)) {
                int_status = (((dmac_intr_complete >> i) & 0x1) ? DMAC_TRANSMIT_OK : DMAC_TRANSMIT_FAIL);
            } else {
                int_status = DMAC_TRANSMIT_FAIL;
            }
            /* Clear interrupt */
            dmac_writel((0x1u << i) & DMAC_CLEAN_TRANSMISSION_INT, DMAC_BASE_ADDR + DMAC_INT_TC_CLR_OFFSET_ADDR);
            dmac_writel((0x1u << i) & DMAC_CLEAN_TRANSMISSION_INT, DMAC_BASE_ADDR + DMAC_INT_ERR_CLR_OFFSET_ADDR);
            callback_info = g_callback_info[i];
            if (callback_info.callback != NULL) {
                bm_flush_dcache_all();
                callback_info.callback(callback_info.req, int_status);
                g_callback_info[i].callback = NULL;
            }
        }
    }

    return BM_OK;
}

static void bm_dmac_flush_cache(const bm_dmac_channel_cfg *channel_cfg)
{
    int trans_type = channel_cfg->trans_type;
    unsigned int src_addr = channel_cfg->src_addr;
    unsigned int dest_addr = channel_cfg->dest_addr;
    unsigned int data_len = channel_cfg->data_len;

    if (trans_type == TRASFER_TYPE_M2M) {
        bm_flush_dcache_range(src_addr, data_len);
        bm_flush_dcache_range(dest_addr, data_len);
    } else if (trans_type == TRASFER_TYPE_M2P) {
        bm_flush_dcache_range(src_addr, data_len);
    } else if (trans_type == TRASFER_TYPE_P2M) {
        bm_flush_dcache_range(dest_addr, data_len);
    } else {
    }
}

int bm_dmac_init(void)
{
    /* Turn on the clock */
    bm_dmac_config_io_clock(DMAC_CLOCK_OPEN);

    /* Enable DMAC and set channel priority */
    u_dmac_g_en dmac_g_en;
    dmac_g_en.value = dmac_readl(DMAC_BASE_ADDR + DMAC_G_EN_OFFSET_ADDR);
    dmac_g_en.bits.dmac_g_en = 1;
    dmac_g_en.bits.prioirty_mode = 0;
    dmac_writel(dmac_g_en.value, DMAC_BASE_ADDR + DMAC_G_EN_OFFSET_ADDR);

    /* The DMAC masks the transfer completion interrupt. */
    dmac_writel(DMAC_MASK_TRANSMISSION_INT, DMAC_BASE_ADDR + DMAC_ITC_MASK_INT_1_OFFSET_ADDR);

    /* The DMAC masks the transfer error interrupt. */
    dmac_writel(DMAC_MASK_TRANSMISSION_INT, DMAC_BASE_ADDR + DMAC_IE_MASK_INT_1_OFFSET_ADDR);

    /* Transfer completion interrupt clear register */
    dmac_writel(DMAC_CLEAN_TRANSMISSION_INT, DMAC_BASE_ADDR + DMAC_INT_TC_CLR_OFFSET_ADDR);

    /* DMAC channel transfer error interrupt clear */
    dmac_writel(DMAC_CLEAN_TRANSMISSION_INT, DMAC_BASE_ADDR + DMAC_INT_ERR_CLR_OFFSET_ADDR);

    /* Initialization callback */
    for (int i = 0; i < DMAC_CHANNEL_CNT; i++) {
        if (g_dma_exist_array[i]) {
            g_callback_info[i].callback = NULL;
        }
    }

     /* Request interrupt */
    osal_irq_set_priority(DMAC_INTID, 6); // pri = 6
    osal_irq_request(DMAC_INTID, bm_dmac_interrupt_handle, NULL, NULL, NULL);

    if (g_sem_init) {
        return BM_OK;
    }
    int ret = osal_sem_init(&g_sem, 1);
    if (ret == OSAL_SUCCESS) {
        g_sem_init = 1;
    }

    return BM_OK;
}

int bm_dmac_deinit(void)
{
    u_dmac_g_en dmac_g_en;

    if (bm_dmac_init_check()) {
        bm_log("dmac is not enabled!\n");
        return BM_FAIL;
    }
    dmac_g_en.value = dmac_readl(DMAC_BASE_ADDR + DMAC_G_EN_OFFSET_ADDR);
    dmac_g_en.bits.dmac_g_en = 0;
    dmac_writel(dmac_g_en.value, DMAC_BASE_ADDR + DMAC_G_EN_OFFSET_ADDR);

    osal_irq_free(DMAC_INTID, NULL);
    osal_irq_disable(DMAC_INTID);

    for (int i = 0; i < DMAC_CHANNEL_CNT; i++) {
        g_dmac_start_flag[i] = 0;
        g_dmac_sync_complete_flag[i] = 0;
        g_dmac_sync_status[i] = 0;
    }

    if (g_sem_init) {
        osal_sem_destroy(&g_sem);
        g_sem_init = 0;
    }

    return BM_OK;
}

static unsigned int bm_dmac_get_ddr_relative_addr(unsigned int phy_addr, int type)
{
    unsigned int dmac_addr;
    unsigned int relative_addr;
    u_lpsc_addr_extend1 lpsc_addr_extend1;
    u_lpsc_addr_sel lpsc_addr_sel;

    (void)bm_lp_subsys_reg_read(LP_SUBSYS_CSR_LPSC_ADDR_EXTEND1_OFFSET_ADDR, &lpsc_addr_extend1.value);
    (void)bm_lp_subsys_reg_read(LP_SUBSYS_CSR_LPSC_ADDR_SEL_OFFSET_ADDR, &lpsc_addr_sel.value);

    switch (lpsc_addr_extend1.bits.dmac_addr_remap_mode) {
        case 0x1:
            /* Shift right by 28 bits valid */
            dmac_addr = (((unsigned long long)(lpsc_addr_extend1.bits.dmac_araddr_offset)) << 28);
            if (type) {
                /* Shift right by 28 bits valid */
                dmac_addr = (((unsigned long long)(lpsc_addr_extend1.bits.dmac_awaddr_offset)) << 28);
            }
            break;

        case 0x2:
            /* Shift right by 32 bits valid */
            dmac_addr = (((unsigned long long)(lpsc_addr_sel.bits.dmac_araddr_sel)) << 32);
            if (type) {
                /* Shift right by 32 bits valid */
                dmac_addr = (((unsigned long long)(lpsc_addr_sel.bits.dmac_awaddr_sel)) << 32);
            }
            break;

        default:
            dmac_addr = 0;
            break;
    }

    if (phy_addr < dmac_addr) {
        bm_log("dmac LPSC_ADDR_EXTEND1(0x%x) is inval!\n", lpsc_addr_extend1.value);
        return phy_addr;
    }

    relative_addr = phy_addr - dmac_addr;
    return relative_addr;
}

static int find_idle_channel(void)
{
    unsigned int index;
    u_dmac_en_ch_status dmac_en_ch_status;

    if (osal_sem_down_timeout(&g_sem, TIME_MS_50) != OSAL_SUCCESS) {
        return BM_FAIL;
    }

    dmac_en_ch_status.value = dmac_readl(DMAC_BASE_ADDR + DMAC_EN_CH_STATUS_OFFSET_ADDR);
    for (index = 0; index < DMAC_CHANNEL_CNT; index++) {
        if (!g_dma_exist_array[index] || g_dmac_start_flag[index]) {
            continue;
        }

        /* Check whether the channel is idle. */
        if (!((dmac_en_ch_status.bits.en_ch_status >> index) & 0x1)) {
                g_dmac_start_flag[index] = 1;
                break;
        }
    }

    if (index >= DMAC_CHANNEL_CNT) {
        osal_sem_up(&g_sem);
        return BM_FAIL;
    }

    osal_sem_up(&g_sem);

    return index;
}

static void bm_dmac_transmission_int_clean(unsigned int dmac_idle_channel)
{
    u_dmac_int_tc_clr dmac_int_tc_clr;
    u_dmac_int_err_clr dmac_int_err_clr;

    /* Clears the channel transfer completion interrupt status. */
    dmac_int_tc_clr.value = dmac_readl(DMAC_BASE_ADDR + DMAC_INT_TC_CLR_OFFSET_ADDR);
    dmac_int_tc_clr.bits.int_tc_clr = BIT(dmac_idle_channel);
    dmac_writel(dmac_int_tc_clr.value, DMAC_BASE_ADDR + DMAC_INT_TC_CLR_OFFSET_ADDR);

    /* Interrupt status for clearing channel transfer errors */
    dmac_int_err_clr.value = dmac_readl(DMAC_BASE_ADDR + DMAC_INT_ERR_CLR_OFFSET_ADDR);
    dmac_int_err_clr.bits.int_err_clr = BIT(dmac_idle_channel);
    dmac_writel(dmac_int_err_clr.value, DMAC_BASE_ADDR + DMAC_INT_ERR_CLR_OFFSET_ADDR);
}

static void bm_dmac_cmaddr_config(int dmac_idle_channel,
    local_dmac_channel_cfg *local_channel_cfg, u_dmac_cmconfig *dmac_cmconfig)
{
    unsigned int src_dma_id;
    unsigned int dest_dma_id;
    unsigned int src_dma_addr;
    unsigned int dest_dma_addr;

    /* Mapping between dma_req_sel and dmac_channel */
    src_dma_id = DMAC_PERIPHERAL_ID(local_channel_cfg->src_req);
    dest_dma_id = DMAC_PERIPHERAL_ID(local_channel_cfg->dest_req);

    bm_lp_subsys_reg_write(DMAC_REQ_SEL0_ADDR(src_dma_id), local_channel_cfg->src_req);
    bm_lp_subsys_reg_write(DMAC_REQ_SEL0_ADDR(dest_dma_id), local_channel_cfg->dest_req);

    if ((dmac_cmconfig->bits.flowctrl == 0) || (dmac_cmconfig->bits.flowctrl == 0x1)) {
        src_dma_addr = bm_dmac_get_ddr_relative_addr(local_channel_cfg->src_addr, DMAC_READ);
    } else {
        src_dma_addr = local_channel_cfg->src_addr;
    }

    if ((dmac_cmconfig->bits.flowctrl == 0) || (dmac_cmconfig->bits.flowctrl == 0x2)) {
        dest_dma_addr = bm_dmac_get_ddr_relative_addr(local_channel_cfg->dest_addr, DMAC_WRITE);
    } else {
        dest_dma_addr = local_channel_cfg->dest_addr;
    }

    /* Configure the source physical addr. */
    dmac_writel(src_dma_addr, DMAC_BASE_ADDR + DMAC_SRC_ADDR_L(dmac_idle_channel));

    /* Configure the destination physical addr. */
    dmac_writel(dest_dma_addr, DMAC_BASE_ADDR + DMAC_DEST_ADDR_L(dmac_idle_channel));
}

static void bm_dmac_channel_int_switch(int channel_id, int enable)
{
    unsigned int dmac_itc_mask_int;
    unsigned int dmac_ie_mask_int;

    dmac_itc_mask_int = dmac_readl(DMAC_BASE_ADDR + DMAC_ITC_MASK_INT_1_OFFSET_ADDR);
    dmac_ie_mask_int = dmac_readl(DMAC_BASE_ADDR + DMAC_IE_MASK_INT_1_OFFSET_ADDR);

    if (enable) { /* Open Channel Interruption */
        dmac_itc_mask_int |= (0x1u << channel_id);
        dmac_ie_mask_int |= (0x1u << channel_id);
    } else { /* Masking Channel Interrupts */
        dmac_itc_mask_int &= ~(0x1u << channel_id);
        dmac_ie_mask_int &= ~(0x1u << channel_id);
    }

    dmac_writel(dmac_itc_mask_int, DMAC_BASE_ADDR + DMAC_ITC_MASK_INT_1_OFFSET_ADDR);
    dmac_writel(dmac_ie_mask_int, DMAC_BASE_ADDR + DMAC_IE_MASK_INT_1_OFFSET_ADDR);
}

static int bm_dmac_channel_config(dmac_reg_info_s *dmac_reg_info)
{
    int dmac_idle_channel;

    u_dmac_cmlli *dmac_cmlli = &(dmac_reg_info->dmac_cmlli);
    u_dmac_cmcontrol *dmac_cmcontrol = &(dmac_reg_info->dmac_cmcontrol);
    u_dmac_cmconfig *dmac_cmconfig = &(dmac_reg_info->dmac_cmconfig);
    local_dmac_channel_cfg *local_channel_cfg = &(dmac_reg_info->local_channel_cfg);

    dmac_idle_channel = find_idle_channel();
    if (dmac_idle_channel == BM_FAIL) {
        bm_log("dmac has no idle channel!\n");
        return BM_FAIL;
    }

    bm_dmac_transmission_int_clean(dmac_idle_channel);

    bm_dmac_cmaddr_config(dmac_idle_channel, local_channel_cfg, dmac_cmconfig);

    /* Enable Channel Interrupt */
    bm_dmac_channel_int_switch(dmac_idle_channel, DMAC_INT_ENABLE);

    /* Configuring the Channel Linked List Register */
    dmac_writel(dmac_cmlli->value, DMAC_BASE_ADDR + DMAC_LLI_ADDR(dmac_idle_channel));

    /* Configure the channel control register. */
    dmac_writel(dmac_cmcontrol->value, DMAC_BASE_ADDR + DMAC_CONTROL_ADDR(dmac_idle_channel));

    /* Configure the channel configuration register. */
    dmac_writel(dmac_cmconfig->value, DMAC_BASE_ADDR + DMAC_CONFIG_ADDR(dmac_idle_channel));

    return dmac_idle_channel;
}

static int bm_dmac_channel_cfg_check(const bm_dmac_channel_cfg *channel_cfg)
{
    if (channel_cfg == NULL) {
        bm_log("bm_dmac_cfg is NULL!\n");
        return BM_FAIL;
    }

    if (channel_cfg->req < DMAC_REQ_SEL_SPI0_TX || channel_cfg->req > DMAC_REQ_SEL_SPI1_RX) {
        bm_log("req (0x%x) is inval!\n", channel_cfg->req);
        return BM_FAIL;
    }

    if (!channel_cfg->data_len) {
        bm_log("data_len (%d) is inval!\n", channel_cfg->data_len);
        return BM_FAIL;
    }
    unsigned int src_addr = channel_cfg->src_addr;
    unsigned int dest_addr = channel_cfg->dest_addr;
    unsigned int len = channel_cfg->data_len;
    if (channel_cfg->trans_type == TRASFER_TYPE_M2P) {
        unsigned int addr = g_dma_req_addr_list[channel_cfg->req].addr;
        if (addr != channel_cfg->dest_addr || is_invalid_addr_len(src_addr, len)) {
            bm_log("addr is inval!\n");
            return BM_FAIL;
        }
    } else if (channel_cfg->trans_type == TRASFER_TYPE_P2M) {
        unsigned int addr = g_dma_req_addr_list[channel_cfg->req].addr;
        if (addr != channel_cfg->src_addr || is_invalid_addr_len(dest_addr, len)) {
            bm_log("addr is inval!\n");
            return BM_FAIL;
        }
    } else if (channel_cfg->trans_type == TRASFER_TYPE_M2M) {
        if (is_invalid_addr_len(src_addr, len) || is_invalid_addr_len(dest_addr, len)) {
            bm_log("addr is inval!\n");
            return BM_FAIL;
        }
    } else {
        bm_log("trans_type (0x%x) is inval!\n", channel_cfg->trans_type);
        return BM_FAIL;
    }

    return BM_OK;
}

static void bm_dmac_common_template(u_dmac_cmcontrol *dmac_cmcontrol, u_dmac_cmconfig *dmac_cmconfig,
    u_dmac_cmlli *dmac_cmlli, const bm_dmac_channel_cfg *channel_cfg)
{
    dmac_cmlli->bits.nextllien = 0;
    dmac_cmlli->bits.lli = 0;

    dmac_cmcontrol->bits.transfersize = channel_cfg->data_len;
    dmac_cmcontrol->bits.sburst_length = 0;
    dmac_cmcontrol->bits.dburst_length = 0;
    dmac_cmcontrol->bits.protection_pri = 0;
    dmac_cmcontrol->bits.protection_buf = 0;
    dmac_cmcontrol->bits.protection_cache = 0;

    dmac_cmconfig->bits.itc_inten = 0;
    dmac_cmconfig->bits.destination_endianness = 0;  // Small end
    dmac_cmconfig->bits.source_endianness = 0;       // Small end
    dmac_cmconfig->bits.ce = 0;
}

static void bm_dmac_transmit_config(const bm_dmac_channel_cfg *channel_cfg, dmac_reg_info_s *dmac_reg_info)
{
    u_dmac_cmlli *dmac_cmlli = &(dmac_reg_info->dmac_cmlli);
    u_dmac_cmcontrol *dmac_cmcontrol = &(dmac_reg_info->dmac_cmcontrol);
    u_dmac_cmconfig *dmac_cmconfig = &(dmac_reg_info->dmac_cmconfig);
    local_dmac_channel_cfg *local_channel_cfg = &(dmac_reg_info->local_channel_cfg);

    dmac_cmlli->value = 0;
    dmac_cmcontrol->value = 0;
    dmac_cmconfig->value = 0;

    local_channel_cfg->src_addr  = channel_cfg->src_addr;
    local_channel_cfg->dest_addr = channel_cfg->dest_addr;
    bm_dmac_common_template(dmac_cmcontrol, dmac_cmconfig, dmac_cmlli, channel_cfg);

    if (channel_cfg->trans_type == TRASFER_TYPE_M2M) {
        local_channel_cfg->src_req = 0;
        local_channel_cfg->dest_req = 0;

        dmac_cmcontrol->bits.si = 1;                         // Addr auto-increment
        dmac_cmcontrol->bits.di = 1;                         // Addr auto-increment

        dmac_cmconfig->bits.srcperipheral = 0;               // The DDR does not care.
        dmac_cmconfig->bits.destperipheral = 0;              // The DDR does not care.
        dmac_cmconfig->bits.flowctrl = 0;                    // Memory-to-memory
        dmac_cmconfig->bits.swidth = 0;                    // width bits, 0: 8bit; 1: 16bit; 2: 32bit; 3: 64bit
        dmac_cmconfig->bits.dwidth = 0;
    } else if (channel_cfg->trans_type == TRASFER_TYPE_M2P) {
        local_channel_cfg->src_req = 0;
        /* The DMA supports only 32 peripherals. The peripheral ID ranges from 0 to 31.
            If the peripheral ID is greater than 31,
            you need to reconfigure the connection between the peripherals and the DMA. */
        local_channel_cfg->dest_req = channel_cfg->req;
        dmac_cmcontrol->bits.si = 1;                     // Addr auto-increment
        dmac_cmcontrol->bits.di = 0;                     // The addr is not auto-incremented.

        dmac_cmconfig->bits.srcperipheral = 0;           // The DDR does not care.
        dmac_cmconfig->bits.destperipheral = DMAC_PERIPHERAL_ID(channel_cfg->req);
        dmac_cmconfig->bits.flowctrl = 0x1;              // Memory to peripherals
        dmac_cmconfig->bits.swidth = 0;                // 8bits
        dmac_cmconfig->bits.dwidth = 0;                // 8bits
    } else if (channel_cfg->trans_type == TRASFER_TYPE_P2M) {
        /* The DMA supports only 32 peripherals. The peripheral ID ranges from 0 to 31.
            If the peripheral ID is greater than 31,
            you need to reconfigure the connection between the peripherals and the DMA. */
        local_channel_cfg->src_req  = channel_cfg->req;

        local_channel_cfg->dest_req = 0;

        dmac_cmcontrol->bits.si = 0;                     // The addr is not auto-incremented.
        dmac_cmcontrol->bits.di = 1;                     // Addr auto-increment

        dmac_cmconfig->bits.srcperipheral = DMAC_PERIPHERAL_ID(channel_cfg->req);
        dmac_cmconfig->bits.destperipheral = 0;          // The DDR does not care.
        dmac_cmconfig->bits.flowctrl = 0x2;              // Peripheral to memory
        dmac_cmconfig->bits.swidth = 0;                  // 8bits
        dmac_cmconfig->bits.dwidth = 0;                  // 8bits
    }
}

static void bm_dmac_transmit_start(int channel_id)
{
    u_dmac_cmconfig dmac_cmconfig;
    dmac_cmconfig.value = dmac_readl(DMAC_BASE_ADDR + DMAC_CONFIG_ADDR(channel_id));
    dmac_cmconfig.bits.ce = 1;
    dmac_writel(dmac_cmconfig.value, DMAC_BASE_ADDR + DMAC_CONFIG_ADDR(channel_id));
}

static int bm_dmac_wait_complete(int channel_id, int time_wait)
{
    unsigned int udelay_conut;

    if (channel_id >= DMAC_CHANNEL_CNT || channel_id < 0) {
        bm_log("channel_id(0x%x) is inval!\n", channel_id);
        return BM_FAIL;
    }

    udelay_conut = (time_wait == (int)BM_WAIT_FOREVER) ? (-1) : (time_wait * 1000); /* 1ms = 1000us */
    while (1) {
        if (g_dmac_sync_complete_flag[channel_id] == 1) {
            g_dmac_sync_complete_flag[channel_id] = 0;
            break;
        }

        udelay(1);

        if (udelay_conut == 0) {
            bm_log("dmac channel(0x%x) complete timeout!\n", channel_id);
            return BM_TIMEOUT;
        }

        if (udelay_conut > 0) {
            udelay_conut--;
        }
        dsb();
    }

    if (g_dmac_sync_status[channel_id] == DMAC_TRANSMIT_OK) {
        return BM_OK;
    } else if (g_dmac_sync_status[channel_id] == DMAC_TRANSMIT_FAIL) {
        bm_log("dmac channel(0x%x) complete failed!\n", channel_id);
        return BM_FAIL;
    } else {
        return BM_FAIL;
    }
}

static void dmac_transmit_sync_callback(bm_dmac_req req, bm_dmac_transmit_async_t type)
{
    g_dmac_sync_complete_flag[req] = 1;
    g_dmac_sync_status[req] = type;
}

int bm_dmac_transmit_sync(const bm_dmac_channel_cfg *channel_cfg, int time_wait)
{
    int channel_id;
    dmac_reg_info_s dmac_reg_info;

    if (bm_dmac_channel_cfg_check(channel_cfg)) {
        return BM_FAIL;
    }

    if (bm_dmac_init_check()) {
        bm_log("dmac is not enabled!\n");
        return BM_FAIL;
    }

    if (time_wait < 0 && (time_wait != (int)BM_WAIT_FOREVER)) {
        bm_log("dmac time_wait(0x%x) is inval!\n", time_wait);
        return BM_FAIL;
    }

    bm_dmac_transmit_config(channel_cfg, &dmac_reg_info);
    channel_id = bm_dmac_channel_config(&dmac_reg_info);
    if (channel_id == BM_FAIL) {
        return BM_FAIL;
    }

    g_callback_info[channel_id].req = channel_id;
    g_callback_info[channel_id].callback = dmac_transmit_sync_callback;

    int core = bm_get_coreid();
    osal_irq_set_affinity(DMAC_INTID, NULL, (1 << core));
    osal_irq_enable(DMAC_INTID);

    bm_dmac_flush_cache(channel_cfg);
    bm_dmac_transmit_start(channel_id);

    int ret = bm_dmac_wait_complete(channel_id, time_wait);
    bm_flush_dcache_all();
    g_dmac_start_flag[channel_id] = 0;
    return ret;
}

int bm_dmac_transmit_async(const bm_dmac_channel_cfg *channel_cfg, bm_dmac_callback callback)
{
    int channel_id;
    int core;
    dmac_reg_info_s dmac_reg_info;

    if (bm_dmac_channel_cfg_check(channel_cfg) || callback == NULL) {
        return BM_FAIL;
    }

    if (bm_dmac_init_check()) {
        bm_log("dmac is not enabled!\n");
        return BM_FAIL;
    }

    bm_dmac_transmit_config(channel_cfg, &dmac_reg_info);
    channel_id = bm_dmac_channel_config(&dmac_reg_info);
    if ((channel_id == BM_FAIL) || (channel_id >= DMAC_CHANNEL_CNT) || g_callback_info[channel_id].callback != NULL) {
        return BM_FAIL;
    }

    g_callback_info[channel_id].req = channel_cfg->req;
    g_callback_info[channel_id].callback = callback;

    core = bm_get_coreid();
    osal_irq_set_affinity(DMAC_INTID, NULL, (1 << core));
    osal_irq_enable(DMAC_INTID);

    bm_dmac_flush_cache(channel_cfg);
    bm_dmac_transmit_start(channel_id);
    g_dmac_start_flag[channel_id] = 0;

    return BM_OK;
}