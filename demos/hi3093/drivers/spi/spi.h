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
#ifndef __SPI_H__
#define __SPI_H__

#include "bm_types.h"
#include "bm_common.h"
#include "bm_spi.h"
#include "spi_regs.h"
#include "io_config.h"
#include "lp_subsys.h"
#include "soc_osal.h"

#ifndef HZ
#define HZ (100000000) /* for jiffies:timer_get_val_turn, HZ is apb clk, 100000000 */
#endif                 /* HZ */

#define SPI_SEM_TIMEOUT_MS 50
#define IO_MUX_R 0
#define IO_MUX_T 1
#define SPI_IO_MUX_MASK 0x7

#define SPI_MAX_CTL 2
#define SPI_MAX_CS 2
#define SPI_CTL_PIN_NUM 3
#define SPI_CS_PIN_NUM 2

#define SPI_TX_MODE 0
#define SPI_RX_MODE 1
#define SPI_TX_RX_MODE 2
#define SPI_CHIP_SELCET_TOTAL_NUM 2

/* Define the union lp_subsys_spi_reg */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int spi0_in_n : 1;         /* [0]  */
        unsigned int spi0_cs_polarity0 : 1; /* [1]  */
        unsigned int spi0_cs_polarity1 : 1; /* [2]  */
        unsigned int spi0_wire_mode : 1;    /* [3]  */
        unsigned int spi0_rx_only : 1;      /* [4]  */
        unsigned int spi1_in_n : 1;         /* [5]  */
        unsigned int spi1_cs_polarity0 : 1; /* [6]  */
        unsigned int spi1_cs_polarity1 : 1; /* [7]  */
        unsigned int spi1_wire_mode : 1;    /* [8]  */
        unsigned int spi1_rx_only : 1;      /* [9]  */
        unsigned int reserved_0 : 22;       /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int val;
} lp_subsys_spi_reg;

/* Define the union spi_sr_reg */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int busy : 1;                /* [0]  */
        unsigned int tfnf : 1;                /* [1]  */
        unsigned int tfe : 1;                 /* [2]  */
        unsigned int rfne : 1;                /* [3]  */
        unsigned int eff : 1;                 /* [4]  */
        unsigned int reserved_0 : 1;          /* [5]  */
        unsigned int dcol : 1;                /* [6]  */
        unsigned int reserved_1 : 25;         /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int val;
} spi_sr_reg;

struct spi_peripheral {
    unsigned int spi_id;
    unsigned long base;
    unsigned int irq;
};

typedef struct {
    unsigned long base;
    unsigned int irq;
    unsigned int spi_id;
    unsigned int cs_id[SPI_MAX_CS];
    unsigned int init_flag[SPI_MAX_CTL];
    unsigned char w_r_flag;
    unsigned char *w_buf;
    unsigned int w_len;
    unsigned char *r_buf;
    unsigned int r_len;
} spi_device;

typedef struct {
    unsigned int offset;
    unsigned int mask;
    unsigned val;
} spi_iomux_info;

struct spi_lock_info {
    int init_flag;
    osal_semaphore sem;
};

struct spi_peripheral g_spi_peripheral_info[] = {
    {0, SPI0_BASE_ADDR, CORE_SYS_SPI0_INTID}, {1, SPI1_BASE_ADDR, CORE_SYS_SPI1_INTID}
};

#define drv_32bit_mask(n1, n2) (BIT((n2) + 1) - BIT(n1))

#define drv_32bit_set_field(v1, n1, n2, v2)                 \
    (((unsigned int)(v1) & (~drv_32bit_mask((n1), (n2)))) | \
        (((unsigned int)(v2) << (unsigned int)(n1)) & drv_32bit_mask((n1), (n2))))

#ifndef drv_32bit_reg_set_field
#define drv_32bit_reg_set_field(reg, n1, n2, v)            \
    do {                                                   \
        unsigned int data;                                 \
        data = spi_readl(reg);                             \
        data = drv_32bit_set_field(data, (n1), (n2), (v)); \
        spi_writel(data, reg);                             \
    } while (0)
#endif

#define spi_readl(addr) (*(volatile unsigned int *)((uintptr_t)(addr)))
#define spi_writel(data, addr) (*(volatile unsigned int *)((uintptr_t)(addr)) = (unsigned int)(data))

#define SYS_AHB_CLOCK_FREQ (200000000)

#define SPI_MIN_DIV 6
#define SPI_MAX_DIV 65534

#define SPI_LOOP_BACK 1
#define SPI_NOT_LOOP_BACK 0

#define SPI_TXFTLR 0x0018 /* SPI TX FIFO threshold register */
#define SPI_RXFTLR 0x001c /* SPI RX FIFO threshold register */
#define SPI_TX_NUM 0x0020 /* SPI TX FIFO depth */
#define SPI_RX_NUM 0x0024 /* SPI RX FIFO depth */

#define SPI_TRAS_MODE_DUPLEX 0
#define SPI_TRAS_MODE_SEND 1
#define SPI_TRAS_MODE_RECEIVE 2
#define SPI_TRAS_MODE_EEPROM_RD 3

#define SEND_LINE_VAL 0

#define SPI_EN 0x1
#define SPI_DISEN 0x0

#define TX_FIFO_SIZE 32

#define SPI_START_RECEIVE 0x5a5a

#define INT_MASK_TX_EMPTY 0x01
#define INT_MASK_RX_FULL 0x10
#define INT_MASK_RX_OVER 0x08

#define ISR_TX_EMPTY 0x01
#define ISR_RX_OVERFLOW 0x08
#define ISR_RX_FULL 0x10

#define SPI_DEFAULT_FRAME_FORMAT 0 /* Motorola SPI Frame Format */

#define SER_REG_VALUE_NON_SLAVE 0    /* When no slave device is selected */
#define SER_REG_VALUE_SLAVE_0 BIT(0) /* Slave device 0 */
#define SER_REG_VALUE_SLAVE_1 BIT(1) /* Slave device 1 */

#define SPI_CS_SELECT 1
#define SPI_CS_DESELECT 0
#define SPI_ID0  0
#define SPI_ID1  1

/* CTRLR0 */
#define ctrlr0_reserved1(value) ((value) << 16) /* 31~16 Reserved bit */
#define ctrlr0_cfs(value) ((value) << 12)       /* 15~12 Control frame size selection */
#define ctrlr0_srl(value) ((value) << 11)       /* 11 Shift register loopback */
#define ctrlr0_reserved2(value) ((value) << 10) /* 10 Reserved bit */
#define ctrlr0_tmod(value) ((value) << 8)       /* 9~8 Transmission mode */
#define ctrlr0_scpol(value) ((value) << 7)      /* 7 Serial clock polarity */
#define ctrlr0_scph(value) ((value) << 6)       /* 6 Serial Clock Phase */
#define ctrlr0_frf(value) ((value) << 4)        /* 5~4 Frame format */
#define ctrlr0_dfs(value) ((value) << 0)        /* 3~0 Data size selection */

/* CTRLR0 srl */
#define CTRLR0_SRL_FIELD_BIT 11 /* Srl CTRLR0[11] */

/* CTRLR0 tmod */
#define CTRLR0_TMOD_FIELD_START_BIT 8 /* tmod CTRLR0[8:9] */
#define CTRLR0_TMOD_FIELD_END_BIT 9   /* tmod CTRLR0[8:9] */

/* CTRLR0 scpol */
#define CTRLR0_SCPOL_FIELD_BIT 7 /* scpol CTRLR0[7] */

/* CTRLR0 scph */
#define CTRLR0_SCPH_FIELD_BIT 6 /* scph CTRLR0[6] */

/* CTRLR0 frf */
#define CTRLR0_FRF_FIELD_START_BIT 4 /* frf CTRLR0[4:5] */
#define CTRLR0_FRF_FIELD_END_BIT 5   /* frf CTRLR0[4:5] */

/* CTRLR0 dfs */
#define CTRLR0_DFS_FIELD_START_BIT 0 /* dfs CTRLR0[0:3] */
#define CTRLR0_DFS_FIELD_END_BIT 3   /* dfs CTRLR0[0:3] */

#define WRITE_DELAY_TIME 160 // 2560//160//1710的参数 //经过验证，写后等待150微秒不会导致异常，为保险起见，设为160微秒

#endif /* __SPI_H__ */