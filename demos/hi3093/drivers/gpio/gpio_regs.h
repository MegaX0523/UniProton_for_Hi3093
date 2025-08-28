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
#ifndef __GPIO_REGS_H__
#define __GPIO_REGS_H__

#define GPIO_BASE_ADDR 0x0874A000

#define GPIO0_BASE_ADDR 0x0874A000
#define GPIO1_BASE_ADDR 0x0874B000
#define GPIO2_BASE_ADDR 0x0874C000
#define GPIO3_BASE_ADDR 0x0874D000
#define GPIO4_BASE_ADDR 0x0874E000

#define GPIO_OUTPUT_OFFSET_ADDR       0x0  /* OUTPUT register */
#define GPIO_DIR_OFFSET_ADDR          0x4  /* Direction Register 0 - Input 1 - Output */
#define GPIO_PORTCTRL_OFFSET_ADDR     0x8  /* Reserved 0 - Software Control 1 - Hardware Control */
#define GPIO_INTEN_OFFSET_ADDR        0x30 /* 0: normal GPIO signal; 1: interrupt signal */
#define GPIO_INTMASK_OFFSET_ADDR      0x34 /* Interrupt mask register 0 - not masked, 1 - masked */
#define GPIO_INTTYPE_OFFSET_ADDR      0x38 /* Interrupt trigger type. 0: level trigger, 1: edge trigger */
#define GPIO_INTPOLARITY_OFFSET_ADDR  0x3c /* 0 - Low/Falling Edge Trigger, 1 - High/Rising Edge Trigger */
#define GPIO_INTSTATUS_OFFSET_ADDR    0x40 /* Masked status register */
#define GPIO_INTRAWSTATUS_OFFSET_ADDR 0x44 /* Raw interrupt status register */
#define GPIO_INTDEBOUNCE_OFFSET_ADDR  0x48 /* Deglitch control register */
#define GPIO_INTCLR_OFFSET_ADDR       0x4c /* Interrupt clear0: not clear1: clear */
#define GPIO_INPUT_OFFSET_ADDR        0x50 /* INPUT register */
#define GPIO_SYNC_OFFSET_ADDR         0x60 /* Interrupt sync enable register 0 - not sync to PCLK, 1 - sync to PCLK */

#endif /* __GPIO_REGS_H__ */