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

#ifndef __BM__CACHE_H__
#define __BM__CACHE_H__

/**
* @defgroup bm_cache bm_cache
*/

/**
*
* @ingroup bm_cache
* @brief invalidate all data cache.
*
* @par description:
* invalidate all data cache.
*
* @param void
*
* @return void
*
*/
void bm_invalidate_dcache_all(void);

/**
*
* @ingroup bm_cache
* @brief invalidate data cache in the range.
*
* @par description:
* invalidate data cache in the range.
*
* @param start [in]: start addr
* @param size  [in]: cache size
*
* @return void
*
*/
void bm_invalidate_dcache_range(unsigned long long start, unsigned int size);

/**
*
* @ingroup bm_cache
* @brief clean & invalidate all data cache.
*
* @par description:
* clean & invalidate all data cache.
*
* @param void
*
* @return void
*
*/
void bm_flush_dcache_all(void);

/**
*
* @ingroup bm_cache
* @brief clean & invalidate data cache in the range.
*
* @par description:
* clean & invalidate data cache in the range.
*
* @param start [in]: start addr
* @param size  [in]: cache size
*
* @return void
*
*/
void bm_flush_dcache_range(unsigned long long start, unsigned int size);

#endif /* __BM__CACHE_H__ */