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

#ifndef __DMAC_UNION_H__
#define __DMAC_UNION_H__

/* Define the union u_dmac_g_en */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    dmac_g_en             : 1   ; /* [0]  */
        unsigned int    prioirty_mode         : 1   ; /* [1]  */
        unsigned int    reserved              : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_g_en;

/* Define the union u_dmac_itc_mask_int_1 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    itc_mark_int1         : 16  ; /* [15..0]  */
        unsigned int    reserved              : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_itc_mask_int_1;

/* Define the union u_dmac_itc_mask_int_2 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    itc_mark_int2         : 16  ; /* [15..0]  */
        unsigned int    reserved              : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_itc_mask_int_2;

/* Define the union u_dmac_itc_mask_int_3 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    itc_mark_int3         : 16  ; /* [15..0]  */
        unsigned int    reserved              : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_itc_mask_int_3;

/* Define the union u_dmac_ie_mask_int_1 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    ie_mark_int1          : 16  ; /* [15..0]  */
        unsigned int    reserved              : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_ie_mask_int_1;

/* Define the union u_dmac_ie_mask_int_2 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    ie_mark_int2          : 16  ; /* [15..0]  */
        unsigned int    reserved              : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_ie_mask_int_2;

/* Define the union u_dmac_ie_mask_int_3 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    ie_mark_int3          : 16  ; /* [15..0]  */
        unsigned int    reserved              : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_ie_mask_int_3;

/* Define the union u_dmac_en_ch_status */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    en_ch_status          : 16  ; /* [15..0]  */
        unsigned int    reserved              : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_en_ch_status;

/* Define the union u_dmac_int_tc_clr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    int_tc_clr            : 16  ; /* [15..0]  */
        unsigned int    reserved              : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_int_tc_clr;

/* Define the union u_dmac_int_err_clr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    int_err_clr           : 16  ; /* [15..0]  */
        unsigned int    reserved              : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_int_err_clr;

/* Define the union u_dmac_cmconfig */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    ce                    : 1   ; /* [0]  */
        unsigned int    srcperipheral         : 6   ; /* [6..1]  */
        unsigned int    destperipheral        : 6   ; /* [12..7]  */
        unsigned int    flowctrl              : 3   ; /* [15..13]  */
        unsigned int    swidth                : 2   ; /* [17..16]  */
        unsigned int    reserved_0            : 1   ; /* [18]  */
        unsigned int    dwidth                : 2   ; /* [20..19]  */
        unsigned int    destination_addr_stride_mode : 1   ; /* [21]  */
        unsigned int    source_address_stride_mode : 1   ; /* [22]  */
        unsigned int    fifo_active           : 1   ; /* [23]  */
        unsigned int    halt                  : 1   ; /* [24]  */
        unsigned int    destination_endianness : 1   ; /* [25]  */
        unsigned int    source_endianness     : 1   ; /* [26]  */
        unsigned int    audio_process_jump_mode : 1   ; /* [27]  */
        unsigned int    reserved_1            : 3   ; /* [30..28]  */
        unsigned int    itc_inten             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_cmconfig;

/* Define the union u_lpsc_addr_extend1 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    dmac_awaddr_offset    : 4   ; /* [3..0]  */
        unsigned int    dmac_araddr_offset    : 4   ; /* [7..4]  */
        unsigned int    dmac_addr_remap_mode  : 2   ; /* [9..8]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_lpsc_addr_extend1;

/* Define the union u_lpsc_addr_sel */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    m3_ibus_addr_sel      : 2   ; /* [1..0]  */
        unsigned int    m3_dbus_addr_sel      : 2   ; /* [3..2]  */
        unsigned int    m3_sbus_addr_sel      : 2   ; /* [5..4]  */
        unsigned int    sfc1_addr_sel         : 2   ; /* [7..6]  */
        unsigned int    emmc_addr_sel         : 2   ; /* [9..8]  */
        unsigned int    sdio_addr_sel         : 2   ; /* [11..10]  */
        unsigned int    lpc_sol_addr_sel      : 2   ; /* [13..12]  */
        unsigned int    espi_sol_addr_sel     : 2   ; /* [15..14]  */
        unsigned int    uart2_sol_addr_sel    : 2   ; /* [17..16]  */
        unsigned int    uart3_sol_addr_sel    : 2   ; /* [19..18]  */
        unsigned int    uart4_sol_addr_sel    : 2   ; /* [21..20]  */
        unsigned int    espi_dma_addr_sel     : 2   ; /* [23..22]  */
        unsigned int    sfc0_addr_sel         : 2   ; /* [25..24]  */
        unsigned int    dmac_awaddr_sel       : 2   ; /* [27..26]  */
        unsigned int    dmac_araddr_sel       : 2   ; /* [29..28]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_lpsc_addr_sel;

/* Define the union u_dmac_cmlli */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    reserved_0            : 1   ; /* [0]  */
        unsigned int    nextllien             : 1   ; /* [1]  */
        unsigned int    lli                   : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_cmlli;

/* Define the union u_dmac_cmcontrol */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    transfersize          : 19  ; /* [18..0]  */
        unsigned int    sburst_length         : 4   ; /* [22..19]  */
        unsigned int    dburst_length         : 4   ; /* [26..23]  */
        unsigned int    si                    : 1   ; /* [27]  */
        unsigned int    di                    : 1   ; /* [28]  */
        unsigned int    protection_pri        : 1   ; /* [29]  */
        unsigned int    protection_buf        : 1   ; /* [30]  */
        unsigned int    protection_cache      : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    value;
} u_dmac_cmcontrol;

#endif /* __DMAC_UNION_H__ */