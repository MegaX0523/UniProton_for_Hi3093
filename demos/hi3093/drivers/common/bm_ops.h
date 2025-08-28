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

#ifndef __BM_OPS_H__
#define __BM_OPS_H__
#include "bm_types.h"

#define bm_readl(addr) (*(volatile unsigned int *)((uintptr_t)(addr)))
#define bm_writel(data, addr) (*(volatile unsigned int *)((uintptr_t)(addr)) = (unsigned int)(data))

#endif /* __BM_OPS_H__ */