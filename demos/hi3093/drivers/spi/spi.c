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
#include "spi.h"
#include "bm_atomic.h"
#include "securec.h"
#include "bm_dmac.h"
#include "bm_spi.h"

/* iomux_reg */
#define IO_CONFIG_SPI0_CLK_ADDR 0x1c
#define IO_CONFIG_SPI0_CS0_ADDR 0x20
#define IO_CONFIG_SPI0_CS1_ADDR 0x24
#define IO_CONFIG_SPI0_DO_ADDR 0x28
#define IO_CONFIG_SPI0_DI_ADDR 0x2C

#ifdef USE_SPI1_SFC_PIN
#define IO_CONFIG_SPI1_CLK_ADDR 0x18
#define IO_CONFIG_SPI1_CS0_ADDR 0x1c
#define IO_CONFIG_SPI1_CS1_ADDR 0x20
#define IO_CONFIG_SPI1_DO_ADDR 0x24
#define IO_CONFIG_SPI1_DI_ADDR 0x24
#else
#define IO_CONFIG_SPI1_CLK_ADDR 0x30
#define IO_CONFIG_SPI1_CS0_ADDR 0x34
#define IO_CONFIG_SPI1_CS1_ADDR 0x38
#define IO_CONFIG_SPI1_DO_ADDR 0x3C
#define IO_CONFIG_SPI1_DI_ADDR 0x40
#endif

spi_device g_spi_devices_info[SPI_MAX_CTL];

static const spi_iomux_info g_spi_iomux_tbl[] = {
    {IO_CONFIG_SPI0_CLK_ADDR, SPI_IO_MUX_MASK, 0x1},
    {IO_CONFIG_SPI0_DO_ADDR, SPI_IO_MUX_MASK, 0x1},
    {IO_CONFIG_SPI0_DI_ADDR, SPI_IO_MUX_MASK, 0x1},
    {IO_CONFIG_SPI1_CLK_ADDR, SPI_IO_MUX_MASK, 0x1},
    {IO_CONFIG_SPI1_DO_ADDR, SPI_IO_MUX_MASK, 0x1},
    {IO_CONFIG_SPI1_DI_ADDR, SPI_IO_MUX_MASK, 0x1},
};

static const spi_iomux_info g_spi_cs_iomux_tbl[] = {
    {IO_CONFIG_SPI0_CS0_ADDR, SPI_IO_MUX_MASK, 0x1},
    {IO_CONFIG_SPI0_CS1_ADDR, SPI_IO_MUX_MASK, 0x1},
    {IO_CONFIG_SPI1_CS0_ADDR, SPI_IO_MUX_MASK, 0x1},
    {IO_CONFIG_SPI1_CS1_ADDR, SPI_IO_MUX_MASK, 0x1},
};

static const int spi_exist_array[SPI_MAX_CTL] = {
    dt_device_exists(SPI, SPI0),
    dt_device_exists(SPI, SPI1),
};

static const unsigned int spi_frame_len_tbl[] = {0x3, 0x7, 0xf}; /* 0x3:11, 0x7:111, 0xf:1111 */

struct bm_spi_ops {
    unsigned char *tx_text_pos;
    unsigned char *rx_text_pos;
    unsigned int tx_text_len;
    unsigned int rx_text_len;
    unsigned int tx_count;
    unsigned int rx_count;
    bm_spi_callback int_tx_callback;
    bm_spi_callback int_rx_callback;
    bm_spi_callback dma_tx_callback;
    bm_spi_callback dma_rx_callback;
};
struct bm_spi_ops g_spi_ops_array[SPI_MAX_CTL];
static const unsigned int g_irq_num_array[SPI_MAX_CTL] = {CORE_SYS_SPI0_INTID, CORE_SYS_SPI1_INTID};
static struct spi_lock_info g_spi_lock_array[SPI_MAX_CTL];

static int read_data(unsigned int id);
static int write_data(unsigned int id, unsigned int cs);
static void bm_spi_disable(unsigned int spi_id)
{
    unsigned long base_addr = g_spi_devices_info[spi_id].base;
    spi_writel(SPI_DISEN, base_addr + SPI_SSIENR_OFFSET_ADDR);
}

static void bm_spi_enable(unsigned int spi_id)
{
    unsigned long base_addr = g_spi_devices_info[spi_id].base;
    spi_writel(SPI_EN, base_addr + SPI_SSIENR_OFFSET_ADDR);
}

static void bm_spi_disable_intr(unsigned int spi_id, unsigned int mask)
{
    unsigned int temp;
    unsigned long base_addr = g_spi_devices_info[spi_id].base;

    temp = spi_readl(base_addr + SPI_IMR_OFFSET_ADDR);
    temp &= ~mask;
    spi_writel(temp, (base_addr + SPI_IMR_OFFSET_ADDR));

    return;
}

static void bm_spi_enable_intr(unsigned int spi_id, unsigned int mask)
{
    unsigned int temp;
    unsigned long base_addr = g_spi_devices_info[spi_id].base;

    temp = spi_readl(base_addr + SPI_IMR_OFFSET_ADDR);
    temp |= mask;
    spi_writel(temp, (base_addr + SPI_IMR_OFFSET_ADDR));

    return;
}

static int bm_spi_baudrate_cfg(unsigned int spi_id, unsigned int baud_rate)
{
    unsigned int freq_div;

    if (!baud_rate) {
        bm_log("baud_rate(%u) must not be zero\n", baud_rate);
        return BM_FAIL;
    }

    freq_div = SYS_AHB_CLOCK_FREQ / baud_rate;

    /* Sets the SPI bus clock frequency. */
    if ((freq_div < SPI_MIN_DIV) || (freq_div > SPI_MAX_DIV)) {
        bm_log("freq_div out of range");
        return BM_FAIL;
    }

    if ((freq_div & 0x1) != 0) {
        freq_div++;
    }

    spi_writel(freq_div, g_spi_devices_info[spi_id].base + SPI_BAUDR_OFFSET_ADDR);

    return BM_OK;
}


static int spi_irq(int irq, void *args)
{
    unsigned int rx_tmp_value = 0;
    bm_spi_transmit_async_t stat = SPI_TRANSMIT_BUTT;
    bm_spi_callback spi_callback;
    (void)irq;
    unsigned int spi_id = *(unsigned int *)(args);
    /* Obtaining the Interrupt Status */
    unsigned long base_addr = g_spi_devices_info[spi_id].base;
    unsigned int irq_en_tmp = spi_readl(base_addr + SPI_ISR_OFFSET_ADDR);
    if ((irq_en_tmp & ISR_TX_EMPTY) == ISR_TX_EMPTY) {
        stat = SPI_WRITE_OK;
        spi_callback = g_spi_ops_array[spi_id].int_tx_callback;
        if (spi_callback != NULL) {
            spi_callback(spi_id, stat);
        }
        (void)bm_spi_disable_intr(spi_id, INT_MASK_TX_EMPTY);
    } else if (((irq_en_tmp & ISR_RX_FULL) == ISR_RX_FULL) || (((irq_en_tmp & ISR_RX_OVERFLOW) == ISR_RX_OVERFLOW))) {
        int ret = read_data(spi_id);
        stat = (ret == BM_OK ? SPI_READ_OK : SPI_READ_FAIL);
        spi_callback = g_spi_ops_array[spi_id].int_rx_callback;
        if (spi_callback != NULL) {
            spi_callback(spi_id, stat);
        }
        (void)bm_spi_disable_intr(spi_id, INT_MASK_RX_FULL | INT_MASK_RX_OVER);
    } else {
        (void)bm_spi_disable_intr(spi_id, irq_en_tmp);
        /* Clears the TX FIFO overflow interrupt. */
        rx_tmp_value = spi_readl(base_addr + SPI_TXOICR_OFFSET_ADDR);
        (void)rx_tmp_value;
    }
    return BM_OK;
}

static int bm_spi_ctrl_reg_config(const bm_spi_cfg *cfg)
{
    int ret;
    unsigned int tmp_value;
    unsigned int spi_id = g_spi_devices_info[cfg->spi_id].spi_id;

    unsigned long base_addr = g_spi_devices_info[cfg->spi_id].base;

    bm_spi_disable(spi_id);

    /* SPI CTRLR0 control register */
    tmp_value = spi_readl(base_addr + SPI_CTRLR0_OFFSET_ADDR);
    /* Returning the forbidden energy */
    tmp_value = drv_32bit_set_field(tmp_value, CTRLR0_SRL_FIELD_BIT, CTRLR0_SRL_FIELD_BIT, SPI_NOT_LOOP_BACK);
    /* MOTO SPI frame format */
    tmp_value =
        drv_32bit_set_field(tmp_value, CTRLR0_FRF_FIELD_START_BIT, CTRLR0_FRF_FIELD_END_BIT, SPI_DEFAULT_FRAME_FORMAT);
    tmp_value = drv_32bit_set_field(tmp_value, CTRLR0_DFS_FIELD_START_BIT, CTRLR0_DFS_FIELD_END_BIT,
        spi_frame_len_tbl[cfg->frame_len]);

    tmp_value = drv_32bit_set_field(tmp_value, CTRLR0_SCPOL_FIELD_BIT, CTRLR0_SCPOL_FIELD_BIT, cfg->clk_pol);
    tmp_value = drv_32bit_set_field(tmp_value, CTRLR0_SCPH_FIELD_BIT, CTRLR0_SCPH_FIELD_BIT, cfg->clk_edge);

    spi_writel(tmp_value, base_addr + SPI_CTRLR0_OFFSET_ADDR);

    spi_writel(0, base_addr + SPI_CTRLR1_OFFSET_ADDR);

    /* Modifying the RX Interrupt Waterline Depth */
    spi_writel(0, base_addr + SPI_RXFTLR_OFFSET_ADDR);
    spi_writel(0, base_addr + SPI_TXFTLR_OFFSET_ADDR);

    ret = bm_spi_baudrate_cfg(cfg->spi_id, cfg->baud);
    if (ret) {
        return ret;
    }

    /* Do not select any slave server. */
    spi_writel(SER_REG_VALUE_NON_SLAVE, base_addr + SPI_SER_OFFSET_ADDR);

    /* Turn off all interrupts */
    bm_spi_disable_intr(spi_id, 0xffffffff);
    spi_writel(cfg->bm_spi_rx_threshold, base_addr + SPI_RXFTLR_OFFSET_ADDR);
    spi_writel(SEND_LINE_VAL, base_addr + SPI_TXFTLR_OFFSET_ADDR);
    osal_irq_set_priority(g_irq_num_array[spi_id], 6); // pri = 6
    osal_irq_free(g_irq_num_array[spi_id], NULL);
    ret = osal_irq_request(g_irq_num_array[spi_id], spi_irq, NULL, NULL, &g_spi_devices_info[cfg->spi_id].spi_id);
    if (ret) {
        return BM_FAIL;
    }
    /* route */
    ret = osal_irq_set_affinity(g_irq_num_array[spi_id], NULL, (1 << bm_get_coreid()));
    if (ret) {
        return BM_FAIL;
    }
    osal_irq_enable(g_irq_num_array[spi_id]);
    /* Enable SPI */
    bm_spi_enable(spi_id);

    return ret;
}

static void spi_io_config(spi_device *dev)
{
    unsigned int spi_id = dev->spi_id;
    for (int i = 0; i < SPI_CTL_PIN_NUM; i++) {
        if (spi_id == SPI_ID0) {
            bm_io_config_r_read_write(g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + i].offset,
                g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + i].mask, g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + i].val);
        } else {
#ifdef USE_SPI1_SFC_PIN
            bm_io_config_t_read_write(g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + i].offset,
                g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + i].mask, g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + i].val);
#else
            bm_io_config_r_read_write(g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + i].offset,
                g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + i].mask, g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + i].val);
#endif
        }
    }
    for (int i = 0; i < SPI_CS_PIN_NUM; i++) {
        if (dev->cs_id[i]) {
            if (spi_id == SPI_ID0) {
                bm_io_config_r_read_write(g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + i].offset,
                    g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + i].mask,
                    g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + i].val);
            } else {
#ifdef USE_SPI1_SFC_PIN
                bm_io_config_t_read_write(g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + i].offset,
                    g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + i].mask,
                    g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + i].val);
#else
                bm_io_config_r_read_write(g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + i].offset,
                    g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + i].mask,
                    g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + i].val);
#endif
            }
        }
    }
}
/* Spi iomux and lp_subsys arrangement */
static int bm_spi_iomux_lp_config(spi_device *dev)
{
    unsigned int spi_id = dev->spi_id;
    /* Setting the IOMUX */
    spi_io_config(dev);
    /* Set LP_SUBSYS */
    unsigned int lp_spi_val;
    lp_subsys_spi_reg spi_reg;
    spi_reg.val = 0;
    if (spi_id == 0) {
        spi_reg.bits.spi0_in_n = 1;
        spi_reg.bits.spi0_rx_only = 1;
        lp_spi_val = BIT(0);
    } else {
        spi_reg.bits.spi1_in_n = 1;
        spi_reg.bits.spi1_rx_only = 1;
        lp_spi_val = BIT(5); /* 5: bit */
    }

    bm_lp_subsys_reg_read_write(LP_SUBSYS_CSR_LPSC_SPI_CTRL_OFFSET_ADDR, spi_reg.val, lp_spi_val);
#ifdef USE_SPI1_SFC_PIN
    bm_lp_subsys_reg_set_bits(LP_SUBSYS_CSR_LPSC_IOMUX_CTRL_OFFSET_ADDR, 1, 1);
#else
    bm_lp_subsys_reg_set_bits(LP_SUBSYS_CSR_LPSC_IOMUX_CTRL_OFFSET_ADDR, 1, 0);
#endif
    return 0;
}

static int bm_spi_init_param_check(const bm_spi_cfg *cfg)
{
    if (cfg == NULL) {
        return BM_FAIL;
    }

    unsigned int spi_id = cfg->spi_id;
    unsigned int frame_len = cfg->frame_len;
    unsigned int clk_edge = cfg->clk_edge;
    unsigned int clk_pol = cfg->clk_pol;
    bm_spi_cs_select cs_map = cfg->cs_map;

    if ((cs_map > HISPI_CS_ALL_SELECT) || (cs_map < HISPI_CS0_SELECT)) {
        bm_log("cs_map(%u) error\n", cs_map);
        return BM_FAIL;
    }
    if (spi_id >= SPI_MAX_CTL) {
        bm_log("spi_id(%u) error\n", spi_id);
        return BM_FAIL;
    }

    if (!spi_exist_array[spi_id]) {
        bm_log("spi_id(%u) error, forbidden to access\n", spi_id);
        return BM_FAIL;
    }

    if ((frame_len < HISPI_FRAME_LENGTH_4_BIT) || (frame_len > HISPI_FRAME_LENGTH_16_BIT)) {
        bm_log("parameter error: frame_len(%u) out of range\n", frame_len);
        return BM_FAIL;
    }

    if (clk_edge >= HISPI_CLOCK_EDGEERR) {
        bm_log("parameter error: clk_edge = %u\n", clk_edge);
        return BM_FAIL;
    }

    if (clk_pol >= HISPI_IDLE_CLOCK_POLARITYERR) {
        bm_log("parameter error: clk_pol = %u\n", clk_pol);
        return BM_FAIL;
    }

    if (cfg->bm_spi_rx_threshold > (SPI_RX_FIFO_SIZE - 1)) {
        bm_log("parameter error: rx_threshold = %u\n", cfg->bm_spi_rx_threshold);
        return BM_FAIL;
    }
    return BM_OK;
}

int bm_spi_init(const bm_spi_cfg *cfg)
{
    unsigned int ret = bm_spi_init_param_check(cfg);
    if (ret != BM_OK) {
        return BM_FAIL;
    }

    spi_device *dev = &g_spi_devices_info[cfg->spi_id];

    if (dev->init_flag[cfg->spi_id]) {
        return BM_FAIL;
    }

    if (memset_s(dev, sizeof(spi_device), 0, sizeof(spi_device)) != EOK) {
        return BM_FAIL;
    }

    if (!g_spi_lock_array[cfg->spi_id].init_flag) {
        if (osal_sem_init(&g_spi_lock_array[cfg->spi_id].sem, 1) == OSAL_SUCCESS) {
            g_spi_lock_array[cfg->spi_id].init_flag = 1;
        } else {
            return BM_FAIL;
        }
    }
    dev->base = g_spi_peripheral_info[cfg->spi_id].base;
    dev->irq = g_spi_peripheral_info[cfg->spi_id].irq;
    dev->spi_id = g_spi_peripheral_info[cfg->spi_id].spi_id;
    for (int i = 0; i < SPI_MAX_CS; i++) {
        dev->cs_id[i] = ((cfg->cs_map >> i) & 0x1);
    }
    dev->init_flag[cfg->spi_id] = 1;

    /* Spi iomux and lp_subsys arrangement */
    bm_spi_iomux_lp_config(dev);
    /* SPI control register configuration */
    ret = bm_spi_ctrl_reg_config(cfg);
    return ret;
}

int bm_spi_deinit(unsigned int spi_id)
{
    if (spi_id >= SPI_MAX_CTL) {
        bm_log("spi_id(%u) error\n", spi_id);
        return BM_FAIL;
    }

    if (!spi_exist_array[spi_id]) {
        bm_log("spi_id(%u) error, forbidden to access\n", spi_id);
        return BM_FAIL;
    }

    spi_device *dev = &g_spi_devices_info[spi_id];
    dev->init_flag[spi_id] = 0;

    if (bm_cmp_xchg32bits((unsigned int *)(uintptr_t)(&(g_spi_lock_array[spi_id].init_flag)), 0, 1) != 0) {
        return BM_FAIL;
    }

    osal_sem_destroy(&g_spi_lock_array[spi_id].sem);

    osal_irq_free(dev->irq, NULL);
    if (memset_s(dev, sizeof(spi_device), 0, sizeof(spi_device)) != EOK) {
        return BM_FAIL;
    }

    return BM_OK;
}

int bm_spi_config(const bm_spi_cfg *cfg)
{
    unsigned int ret = bm_spi_init_param_check(cfg);
    if (ret != BM_OK) {
        return BM_FAIL;
    }

    spi_device *dev = &g_spi_devices_info[cfg->spi_id];
    if (dev->init_flag[cfg->spi_id] == 0) {
        return BM_FAIL;
    }

    if (osal_sem_down_timeout(&g_spi_lock_array[cfg->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }
    ret = bm_spi_ctrl_reg_config(cfg);
    osal_sem_up(&g_spi_lock_array[cfg->spi_id].sem);
    return ret;
}

static int bm_spi_handle_para_check(const bm_spi_handle *handle)
{
    if (handle == NULL) {
        return BM_FAIL;
    }

    if (handle->spi_id >= SPI_MAX_CTL || handle->cs_id >= SPI_MAX_CS) {
        return BM_FAIL;
    }

    if (g_spi_devices_info[handle->spi_id].init_flag[handle->spi_id] == 0) {
        return BM_FAIL;
    }

    if (!spi_exist_array[handle->spi_id]) {
        bm_log("spi_id(%u) error, forbidden to access\n", handle->spi_id);
        return BM_FAIL;
    }

    return BM_OK;
}

int bm_spi_config_io_reuse(const bm_spi_handle *handle)
{
    if (bm_spi_handle_para_check(handle) == BM_FAIL) {
        return BM_FAIL;
    }
    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }
    unsigned int spi_id = handle->spi_id;
    unsigned int cs_id = handle->cs_id;
    /* Setting the IOMUX */
    for (int x = 0; x < SPI_CTL_PIN_NUM; x++) {
        if (spi_id == SPI_ID0) {
            bm_io_config_r_read_write(g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + x].offset,
                g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + x].mask, g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + x].val);
        } else {
#ifdef USE_SPI1_SFC_PIN
            bm_io_config_t_read_write(g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + x].offset,
                g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + x].mask, g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + x].val);
#else
            bm_io_config_r_read_write(g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + x].offset,
                g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + x].mask, g_spi_iomux_tbl[spi_id * SPI_CTL_PIN_NUM + x].val);
#endif
        }
    }

    if (spi_id == SPI_ID0) {
        bm_io_config_r_read_write(g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + cs_id].offset,
            g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + cs_id].mask,
            g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + cs_id].val);
    } else {
#ifdef USE_SPI1_SFC_PIN
        bm_io_config_t_read_write(g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + cs_id].offset,
            g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + cs_id].mask,
            g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + cs_id].val);
#else
        bm_io_config_r_read_write(g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + cs_id].offset,
            g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + cs_id].mask,
            g_spi_cs_iomux_tbl[spi_id * SPI_CS_PIN_NUM + cs_id].val);
#endif
    }
    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return BM_OK;
}

int bm_spi_config_io_clock(const bm_spi_handle *handle)
{
    if (bm_spi_handle_para_check(handle) == BM_FAIL) {
        return BM_FAIL;
    }
    unsigned int spi_id = handle->spi_id;
    /* Set LP_SUBSYS */
    unsigned int lp_spi_value;
    lp_subsys_spi_reg spi_reg;
    spi_reg.val = 0;
    if (spi_id == 0) {
        spi_reg.bits.spi0_in_n = 1;
        spi_reg.bits.spi0_rx_only = 1;
        lp_spi_value = BIT(0);
    } else {
        spi_reg.bits.spi1_in_n = 1;
        spi_reg.bits.spi1_rx_only = 1;
        lp_spi_value = BIT(5); /* 5: bit */
    }
    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }
    bm_lp_subsys_reg_read_write(LP_SUBSYS_CSR_LPSC_SPI_CTRL_OFFSET_ADDR, spi_reg.val, lp_spi_value);
    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return BM_OK;
}

static int bm_spi_frame_len_check(const bm_spi_handle *handle, unsigned len)
{
    unsigned long base_addr = g_spi_devices_info[handle->spi_id].base;
    unsigned int frame_len = (spi_readl(base_addr + SPI_CTRLR0_OFFSET_ADDR)) & 0xf;
    unsigned int length = len;
    if (frame_len == 0xf) {
        if (length % 2) { // frame length is 16bit, input length must be multiple of 2
            bm_log("frame length is 16bit, input length(%u) must be multiple of 2\n", length);
            return BM_FAIL;
        }
    }
    return BM_OK;
}

static int bm_spi_transfer_para_check(const bm_spi_handle *handle, bm_spi_data *data, unsigned int mod)
{
    if (bm_spi_handle_para_check(handle) == BM_FAIL) {
        return BM_FAIL;
    }
    if (data == NULL) {
        return BM_FAIL;
    }

    int ret;

    switch (mod) {
        case SPI_TX_MODE:
            ret = (data->w_buf == NULL || data->w_len == 0 || bm_spi_frame_len_check(handle, data->w_len) ||
                   is_invalid_addr_len((unsigned int)(uintptr_t)(data->w_buf), data->w_len));
            break;
        case SPI_RX_MODE:
            ret = (data->r_buf == NULL || data->r_len == 0 || bm_spi_frame_len_check(handle, data->r_len) ||
                is_invalid_addr_len((unsigned int)(uintptr_t)(data->r_buf), data->r_len));
            break;
        case SPI_TX_RX_MODE:
            ret = (data->w_buf == NULL || data->w_len == 0 || data->r_buf == NULL || data->r_len == 0 ||
                bm_spi_frame_len_check(handle, data->w_len) || bm_spi_frame_len_check(handle, data->r_len) ||
                is_invalid_addr_len((unsigned int)(uintptr_t)(data->w_buf), data->w_len) ||
                is_invalid_addr_len((unsigned int)(uintptr_t)(data->r_buf), data->r_len));
            break;
        default:
            return BM_FAIL;
            break;
    }

    if (ret) {
        return BM_FAIL;
    }
    return BM_OK;
}

static int bm_spi_cs_control(const bm_spi_handle *handle, unsigned int select_status)
{
    if (bm_spi_handle_para_check(handle) == BM_FAIL) {
        return BM_FAIL;
    }
    unsigned long base_addr = g_spi_devices_info[handle->spi_id].base;
    if (select_status == SPI_CS_SELECT) {
        spi_writel((1 << (handle->cs_id)), base_addr + SPI_SER_OFFSET_ADDR);
    } else {
        spi_writel(SER_REG_VALUE_NON_SLAVE, base_addr + SPI_SER_OFFSET_ADDR);
    }
    return BM_OK;
}

static void bm_spi_write_once_by_frame_len(const bm_spi_handle *handle, unsigned int frame_len, unsigned char *w_buf)
{
    unsigned long base_addr = g_spi_devices_info[handle->spi_id].base;
    unsigned int tx_tmp_value;
    if (frame_len == 0xf) {
        tx_tmp_value = ((*(unsigned short *)w_buf) & 0xffff);
    } else {
        tx_tmp_value = ((*w_buf) & 0xff);
    }
    spi_writel(tx_tmp_value, base_addr + SPI_DR0_OFFSET_ADDR);
}

static void bm_spi_read_once_by_frame_len(const bm_spi_handle *handle, unsigned int frame_len, unsigned char *r_buf)
{
    unsigned long base_addr = g_spi_devices_info[handle->spi_id].base;
    unsigned int rx_tmp_value = spi_readl(base_addr + SPI_DR0_OFFSET_ADDR);

    if (frame_len == 0xf) {
        r_buf[0] = (rx_tmp_value & 0xff);
        r_buf[1] = ((rx_tmp_value >> 8) & 0xff); // shift 8 bits
    } else {
        r_buf[0] = (rx_tmp_value & 0xff);
    }
}

static int bm_spi_wait_idle(unsigned int spi_id, unsigned int time_out)
{
    unsigned int u_timeout = time_out * 1000; // 1000 us
    unsigned long base_addr = g_spi_devices_info[spi_id].base;

    /* Check whether the SPI is in the idle state. */
    while ((u_timeout != 0) && (spi_readl(base_addr + SPI_SR_OFFSET_ADDR) & BIT(0))) {
        u_timeout--;
        udelay(1); /* usleep 1~2 us */
    }
    if (u_timeout == 0) {
        bm_log("wait spi idle timeout\n");
        return BM_FAIL;
    }

    return BM_OK;
}

int bm_spi_write(const bm_spi_handle *handle, bm_spi_data *w_data)
{
    if (bm_spi_transfer_para_check(handle, w_data, SPI_TX_MODE) == BM_FAIL) {
        return BM_FAIL;
    }

    unsigned int length = w_data->w_len;
    unsigned long base_addr = g_spi_devices_info[handle->spi_id].base;
    unsigned int frame_len = (spi_readl(base_addr + SPI_CTRLR0_OFFSET_ADDR)) & 0xf;

    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }

    bm_spi_cs_control(handle, SPI_CS_SELECT);
    int i = 0;
    while (length) {
        if (spi_readl(base_addr + SPI_TXFLR_OFFSET_ADDR) == 0) {
            bm_spi_write_once_by_frame_len(handle, frame_len, w_data->w_buf + i);
            length = (frame_len == 0xf) ? (length - 2) : (length - 1); // a frame contains 2 or 1 byte
            i = (frame_len == 0xf) ? (i + 2) : (i + 1);                // a frame contains 2 or 1 byte
        }
    }
    int ret = bm_spi_wait_idle(handle->spi_id, w_data->time_out);
    bm_spi_cs_control(handle, SPI_CS_DESELECT);
    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return ret;
}

int bm_spi_read(const bm_spi_handle *handle, bm_spi_data *r_data)
{
    if (bm_spi_transfer_para_check(handle, r_data, SPI_RX_MODE) == BM_FAIL) {
        return BM_FAIL;
    }
    unsigned int length = r_data->r_len;
    unsigned long base_addr = g_spi_devices_info[handle->spi_id].base;
    unsigned int frame_len = (spi_readl(base_addr + SPI_CTRLR0_OFFSET_ADDR)) & 0xf;

    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }

    bm_spi_cs_control(handle, SPI_CS_SELECT);
    int i = 0;
    while (length) {
        if (spi_readl(base_addr + SPI_RXFLR_OFFSET_ADDR)) {
            bm_spi_read_once_by_frame_len(handle, frame_len, r_data->r_buf + i);
            length = (frame_len == 0xf) ? (length - 2) : (length - 1); // a frame contains 2 or 1 byte
            i = (frame_len == 0xf) ? (i + 2) : (i + 1);                // a frame contains 2 or 1 byte
        }
    }
    int ret = bm_spi_wait_idle(handle->spi_id, r_data->time_out);
    bm_spi_cs_control(handle, SPI_CS_DESELECT);
    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return ret;
}

int bm_spi_write_read(const bm_spi_handle *handle, bm_spi_data *w_r_data)
{
    if (bm_spi_transfer_para_check(handle, w_r_data, SPI_TX_RX_MODE) == BM_FAIL) {
        return BM_FAIL;
    }

    unsigned int w_len = w_r_data->w_len;
    unsigned int r_len = w_r_data->r_len;
    unsigned long base_addr = g_spi_devices_info[handle->spi_id].base;
    unsigned int frame_len = (spi_readl(base_addr + SPI_CTRLR0_OFFSET_ADDR)) & 0xf;

    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }

    /* clear tx_fifo and rx_fifo */
    bm_spi_disable(handle->spi_id);
    bm_spi_enable(handle->spi_id);
    bm_spi_cs_control(handle, SPI_CS_SELECT);

    int i = 0;
    /* write */
    while (w_len) {
        if (spi_readl(base_addr + SPI_TXFLR_OFFSET_ADDR) == 0) {
            bm_spi_write_once_by_frame_len(handle, frame_len, w_r_data->w_buf + i);
            w_len = (frame_len == 0xf) ? (w_len - 2) : (w_len - 1); // a frame contains 2 or 1 byte
            i = (frame_len == 0xf) ? (i + 2) : (i + 1);             // a frame contains 2 or 1 byte
        }
    }
    int ret = bm_spi_wait_idle(handle->spi_id, w_r_data->time_out);
    if (ret) {
        osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
        return ret;
    }

    /* read */
    i = 0;
    while (r_len) {
        if (spi_readl(base_addr + SPI_RXFLR_OFFSET_ADDR)) {
            bm_spi_read_once_by_frame_len(handle, frame_len, w_r_data->r_buf + i);
            r_len = (frame_len == 0xf) ? (r_len - 2) : (r_len - 1); // a frame contains 2 or 1 byte
            i = (frame_len == 0xf) ? (i + 2) : (i + 1);             // a frame contains 2 or 1 byte
        }
    }
    ret = bm_spi_wait_idle(handle->spi_id, w_r_data->time_out);
    bm_spi_cs_control(handle, SPI_CS_DESELECT);
    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return ret;
}

static void bm_spi_disable_dma(unsigned int id, unsigned int mask)
{
    unsigned int temp;
    unsigned long base_addr = g_spi_devices_info[id].base;

    temp = spi_readl(base_addr + SPI_DMACR_OFFSET_ADDR);
    temp &= ~mask;
    spi_writel(temp, (base_addr + SPI_DMACR_OFFSET_ADDR));

    return;
}

static void bm_spi_enable_dma(unsigned int id, unsigned int mask)
{
    unsigned int temp;
    unsigned long base_addr = g_spi_devices_info[id].base;

    temp = spi_readl(base_addr + SPI_DMACR_OFFSET_ADDR);
    temp |= mask;
    spi_writel(temp, (base_addr + SPI_DMACR_OFFSET_ADDR));

    return;
}

static void dma_finish_func(bm_dmac_req req, bm_dmac_transmit_async_t type)
{
    bm_spi_callback spi_callback = NULL;
    bm_spi_transmit_async_t state = SPI_TRANSMIT_BUTT;
    int spi_id = 0;

    if (req == DMAC_REQ_SEL_SPI1_TX) {
        spi_id = SPI_ID1;
        if (type != DMAC_TRANSMIT_OK)
            state = SPI_WRITE_FAIL;
        else
            state = SPI_WRITE_OK;
        spi_callback = g_spi_ops_array[spi_id].dma_tx_callback;
        bm_spi_disable_dma(spi_id, 0x2);
    } else if (req == DMAC_REQ_SEL_SPI1_RX) {
        spi_id = SPI_ID1;
        if (type != DMAC_TRANSMIT_OK)
            state = SPI_READ_FAIL;
        else
            state = SPI_READ_OK;
        spi_callback = g_spi_ops_array[spi_id].dma_rx_callback;
        bm_spi_disable_dma(spi_id, 0x1);
    } else if (req == DMAC_REQ_SEL_SPI0_TX) {
        spi_id = SPI_ID0;
        if (type != DMAC_TRANSMIT_OK)
            state = SPI_WRITE_FAIL;
        else
            state = SPI_WRITE_OK;
        spi_callback = g_spi_ops_array[spi_id].dma_tx_callback;
        bm_spi_disable_dma(spi_id, 0x2);
    } else if (req == DMAC_REQ_SEL_SPI0_RX) {
        spi_id = SPI_ID0;
        if (type != DMAC_TRANSMIT_OK)
            state = SPI_READ_FAIL;
        else
            state = SPI_READ_OK;
        spi_callback = g_spi_ops_array[spi_id].dma_rx_callback;
        bm_spi_disable_dma(spi_id, 0x1);
    }
    if (spi_callback) {
        spi_callback(spi_id, state);
    }
}

static bm_dmac_req spi_dma_get_dmac_req(unsigned char id, int is_read)
{
    if (id <= SPI_ID0) {
        if (is_read == TRUE) {
            return DMAC_REQ_SEL_SPI0_RX;
        } else {
            return DMAC_REQ_SEL_SPI0_TX;
        }
    } else {
        if (is_read == TRUE) {
            return DMAC_REQ_SEL_SPI1_RX;
        } else {
            return DMAC_REQ_SEL_SPI1_TX;
        }
    }
}

static int spi_write_dma(unsigned char id, unsigned char cs, unsigned char *tx_buf, unsigned int tx_len)
{
    int ret = 0;
    bm_dmac_channel_cfg channel_cfg = { 0 };
    int dma_channel = spi_dma_get_dmac_req(id, FALSE);
    unsigned long base_addr = g_spi_devices_info[id].base;

    spi_writel(0xf, base_addr + SPI_DMATDLR_OFFSET_ADDR);
    /* Chip select CS */
    spi_writel((1 << cs), base_addr + SPI_SER_OFFSET_ADDR);
    udelay(WRITE_DELAY_TIME);
    /* Configure the DMA to start transmission. */
    channel_cfg.req = dma_channel;
    channel_cfg.src_addr = (uintptr_t)(tx_buf);
    channel_cfg.dest_addr = (uintptr_t)(base_addr + SPI_DR0_OFFSET_ADDR);
    channel_cfg.data_len = tx_len;
    channel_cfg.trans_type = TRASFER_TYPE_M2P;
    ret = bm_dmac_transmit_async(&channel_cfg, dma_finish_func);
    /* DMA Enable Start */
    bm_spi_enable_dma(id, 0x2);
    return ret;
}

static int spi_read_dma(unsigned char id, unsigned char cs, unsigned char *rx_buf, unsigned int rx_len)
{
    int ret = 0;
    bm_dmac_channel_cfg channel_cfg = { 0 };
    int dma_channel = spi_dma_get_dmac_req(id, TRUE);
    unsigned long base_addr = g_spi_devices_info[id].base;

    /* Chip select CS */
    spi_writel((1 << cs), base_addr + SPI_SER_OFFSET_ADDR);
    /* Configure the DMA to start transmission. */
    channel_cfg.req = dma_channel;
    channel_cfg.src_addr = (uintptr_t)(base_addr + SPI_DR0_OFFSET_ADDR);
    ;
    channel_cfg.dest_addr = (uintptr_t)(rx_buf);
    channel_cfg.data_len = rx_len;
    channel_cfg.trans_type = TRASFER_TYPE_P2M;

    ret = bm_dmac_transmit_async(&channel_cfg, dma_finish_func);
    /* DMA Enable Start */
    bm_spi_enable_dma(id, 0x1);
    return ret;
}

int bm_spi_write_dma(const bm_spi_handle *handle, bm_spi_data *w_data, bm_spi_callback callback)
{
    int ret;
    if (bm_spi_transfer_para_check(handle, w_data, SPI_TX_MODE) == BM_FAIL) {
        return BM_FAIL;
    }

    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }
    g_spi_ops_array[handle->spi_id].dma_tx_callback = callback;
    ret = spi_write_dma(handle->spi_id, handle->cs_id, w_data->w_buf, w_data->w_len);
    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return ret;
}

int bm_spi_read_dma(const bm_spi_handle *handle, bm_spi_data *r_data, bm_spi_callback callback)
{
    int ret;
    if (bm_spi_transfer_para_check(handle, r_data, SPI_RX_MODE) == BM_FAIL) {
        return BM_FAIL;
    }

    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }
    g_spi_ops_array[handle->spi_id].dma_rx_callback = callback;
    ret = spi_read_dma(handle->spi_id, handle->cs_id, r_data->r_buf, r_data->r_len);
    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return ret;
}

static int read_data(unsigned int id)
{
    int ret;
    unsigned int rx_tmp_value = 0;
    unsigned long base_addr = g_spi_devices_info[id].base;
    unsigned int bit_value = (spi_readl(base_addr + SPI_CTRLR0_OFFSET_ADDR)) & 0xf;
    bm_spi_frame_length frame_len = (bit_value == 0xf) ? HISPI_FRAME_LENGTH_16_BIT : HISPI_FRAME_LENGTH_8_BIT;
    spi_sr_reg *sr_reg = (spi_sr_reg *)(uintptr_t)(base_addr + SPI_SR_OFFSET_ADDR);
    while (g_spi_ops_array[id].rx_text_len > g_spi_ops_array[id].rx_count && g_spi_ops_array[id].rx_text_pos &&
        sr_reg->bits.rfne) {
        rx_tmp_value = spi_readl(base_addr + SPI_DR0_OFFSET_ADDR);
        if (frame_len > HISPI_FRAME_LENGTH_8_BIT) {
            *(unsigned short *)g_spi_ops_array[id].rx_text_pos = rx_tmp_value;
        } else {
            *(unsigned char *)g_spi_ops_array[id].rx_text_pos = rx_tmp_value & 0xff;
        }
        g_spi_ops_array[id].rx_text_pos += frame_len;
        g_spi_ops_array[id].rx_count += frame_len;
    }
    if (g_spi_ops_array[id].rx_text_len > g_spi_ops_array[id].rx_count)
        ret = BM_FAIL;
    else
        ret = BM_OK;
    unsigned char count = spi_readl(base_addr + SPI_RXFLR_OFFSET_ADDR);
    while (count--) {
        rx_tmp_value = spi_readl(base_addr + SPI_DR0_OFFSET_ADDR);
        (void)rx_tmp_value;
    }
    g_spi_ops_array[id].rx_text_len = 0;
    g_spi_ops_array[id].rx_count = 0;
    return ret;
}

static int write_data(unsigned int id, unsigned int cs)
{
    unsigned short tx_value_tmp;
    unsigned long base_addr = g_spi_devices_info[id].base;
    unsigned int value = (spi_readl(base_addr + SPI_CTRLR0_OFFSET_ADDR)) & 0xf;
    unsigned int frame_len = (value == 0xf) ? HISPI_FRAME_LENGTH_16_BIT : HISPI_FRAME_LENGTH_8_BIT;
    spi_sr_reg *sr_reg = (spi_sr_reg *)(uintptr_t)(base_addr + SPI_SR_OFFSET_ADDR);
    if (sr_reg->bits.busy) {
        return BM_FAIL;
    }
    /* Cancel chip selection */
    spi_writel(SER_REG_VALUE_NON_SLAVE, base_addr + SPI_SER_OFFSET_ADDR);

    while (g_spi_ops_array[id].tx_text_len > g_spi_ops_array[id].tx_count && sr_reg->bits.tfnf) {
        if (frame_len > HISPI_FRAME_LENGTH_8_BIT) {
            tx_value_tmp = *(unsigned short *)g_spi_ops_array[id].tx_text_pos;
        } else {
            tx_value_tmp = *(unsigned char *)g_spi_ops_array[id].tx_text_pos;
        }
        spi_writel(tx_value_tmp, base_addr + SPI_DR0_OFFSET_ADDR);
        g_spi_ops_array[id].tx_count += frame_len;
        g_spi_ops_array[id].tx_text_pos += frame_len;
    }
    /* Threshold interrupt enable. The interrupt type is TX FIFO empty interrupt. */
    osal_irq_enable(g_irq_num_array[id]);
    (void)bm_spi_enable_intr(id, INT_MASK_TX_EMPTY);
    /* Open Chip Select */
    spi_writel((1 << cs), base_addr + SPI_SER_OFFSET_ADDR);
    return BM_OK;
}
int bm_spi_write_async(const bm_spi_handle *handle, bm_spi_data *w_data, bm_spi_callback callback)
{
    int ret = 0;
    if (bm_spi_transfer_para_check(handle, w_data, SPI_TX_MODE) == BM_FAIL) {
        return BM_FAIL;
    }
    if (w_data->w_len > SPI_TX_FIFO_SIZE) {
        return BM_FAIL;
    }

    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }
    g_spi_ops_array[handle->spi_id].tx_text_pos = w_data->w_buf;
    g_spi_ops_array[handle->spi_id].tx_text_len = w_data->w_len;
    g_spi_ops_array[handle->spi_id].tx_count = 0;
    g_spi_ops_array[handle->spi_id].int_tx_callback = callback;
    ret = write_data(handle->spi_id, handle->cs_id);

    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return ret;
}

int bm_spi_read_async(const bm_spi_handle *handle, bm_spi_data *r_data, bm_spi_callback callback)
{
    if (bm_spi_transfer_para_check(handle, r_data, SPI_RX_MODE) == BM_FAIL) {
        return BM_FAIL;
    }
    if (r_data->r_len > SPI_RX_FIFO_SIZE) {
        return BM_FAIL;
    }

    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }
    g_spi_ops_array[handle->spi_id].rx_text_pos = r_data->r_buf;
    g_spi_ops_array[handle->spi_id].rx_text_len = r_data->r_len;
    g_spi_ops_array[handle->spi_id].rx_count = 0;
    g_spi_ops_array[handle->spi_id].int_rx_callback = callback;

    osal_irq_enable(g_irq_num_array[handle->spi_id]);
    (void)bm_spi_enable_intr(handle->spi_id, INT_MASK_RX_FULL | INT_MASK_RX_OVER);
    unsigned long base_addr = g_spi_devices_info[handle->spi_id].base;
    spi_writel((1 << handle->cs_id), base_addr + SPI_SER_OFFSET_ADDR);
    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return BM_OK;
}

int bm_spi_write_read_async(const bm_spi_handle *handle, bm_spi_data *w_r_data, bm_spi_callback callback)
{
    int ret = 0;
    if (bm_spi_transfer_para_check(handle, w_r_data, SPI_TX_RX_MODE) == BM_FAIL) {
        return BM_FAIL;
    }
    if (w_r_data->w_len > SPI_TX_FIFO_SIZE) {
        return BM_FAIL;
    }
    if (w_r_data->r_len > SPI_RX_FIFO_SIZE) {
        return BM_FAIL;
    }

    if (osal_sem_down_timeout(&g_spi_lock_array[handle->spi_id].sem, SPI_SEM_TIMEOUT_MS) != OSAL_SUCCESS) {
        bm_log("osal_sem_down FAILED\n");
        return BM_FAIL;
    }
    g_spi_ops_array[handle->spi_id].tx_text_pos = w_r_data->w_buf;
    g_spi_ops_array[handle->spi_id].tx_text_len = w_r_data->w_len;
    g_spi_ops_array[handle->spi_id].tx_count = 0;

    g_spi_ops_array[handle->spi_id].rx_text_pos = w_r_data->r_buf;
    g_spi_ops_array[handle->spi_id].rx_text_len = w_r_data->r_len;
    g_spi_ops_array[handle->spi_id].rx_count = 0;
    g_spi_ops_array[handle->spi_id].int_tx_callback = NULL;
    g_spi_ops_array[handle->spi_id].int_rx_callback = callback;

    (void)bm_spi_enable_intr(handle->spi_id, INT_MASK_RX_FULL | INT_MASK_RX_OVER);
    ret = write_data(handle->spi_id, handle->cs_id);
    osal_sem_up(&g_spi_lock_array[handle->spi_id].sem);
    return ret;
}
