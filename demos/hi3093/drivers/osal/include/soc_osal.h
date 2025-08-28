/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: OS Abstract Layer.
 */

#ifndef __SOC_OSAL_H__
#define __SOC_OSAL_H__

#include "osal_list.h"
#include "osal_errno.h"

/* osal api header begin */

#if defined(__FREERTOS__)
#include "module/osal_atomic.h"
#include "module/osal_debug.h"
#include "module/osal_interrupt.h"
#include "module/osal_mutex.h"
#include "module/osal_task.h"
#include "module/osal_addr.h"
#include "module/osal_cache.h"
#include "module/osal_drvbox.h"
#include "module/osal_event.h"
#include "module/osal_msgqueue.h"
#include "module/osal_semaphore.h"
#include "module/osal_spinlock.h"
#include "module/osal_string.h"
#include "module/osal_timer.h"
#include "module/osal_completion.h"
#endif

#if defined(__linux__)
#include "module/osal_atomic.h"
#include "module/osal_debug.h"
#include "module/osal_interrupt.h"
#include "module/osal_mutex.h"
#include "module/osal_task.h"
#include "module/osal_addr.h"
#include "module/osal_barrier.h"
#include "module/osal_device.h"
#include "module/osal_fileops.h"
#include "module/osal_math.h"
#include "module/osal_proc.h"
#include "module/osal_semaphore.h"
#include "module/osal_spinlock.h"
#include "module/osal_string.h"
#include "module/osal_timer.h"
#include "module/osal_rtc.h"
#include "module/osal_wait.h"
#include "module/osal_workqueue.h"
#if defined(__KERNEL__)
#include "module/osal_cache.h"
#include "module/osal_bitmap.h"
#include "module/osal_completion.h"
#include "module/osal_delaywork.h"
#include "module/osal_rwlock.h"
#endif
#endif

#if defined(__LITEOS__)
#include "module/osal_atomic.h"
#include "module/osal_debug.h"
#include "module/osal_interrupt.h"
#include "module/osal_mutex.h"
#include "module/osal_task.h"
#include "module/osal_addr.h"
#include "module/osal_barrier.h"
#include "module/osal_cache.h"
#include "module/osal_completion.h"
#include "module/osal_device.h"
#include "module/osal_drvbox.h"
#include "module/osal_event.h"
#include "module/osal_fileops.h"
#include "module/osal_math.h"
#include "module/osal_msgqueue.h"
#include "module/osal_proc.h"
#include "module/osal_rtc.h"
#include "module/osal_semaphore.h"
#include "module/osal_spinlock.h"
#include "module/osal_string.h"
#include "module/osal_timer.h"
#include "module/osal_wait.h"
#include "module/osal_workqueue.h"
#endif

#if defined(__NONOS__)
#include "module/osal_atomic.h"
#include "module/osal_debug.h"
#include "module/osal_interrupt.h"
#include "module/osal_mutex.h"
#include "module/osal_task.h"
#include "module/osal_spinlock.h"
#include "module/osal_semaphore.h"
#include "module/osal_timer.h"
#include "module/osal_wait.h"
#endif

#if defined(__LIBCK__)
#include "module/osal_debug.h"
#include "module/osal_interrupt.h"
#include "module/osal_task.h"
#include "module/osal_addr.h"
#include "module/osal_event.h"
#include "module/osal_msgqueue.h"
#include "module/osal_semaphore.h"
#include "module/osal_timer.h"
#endif

#if defined(__UNIPROTON__)
#include "module/osal_interrupt.h"
#include "module/osal_semaphore.h"
#include "module/osal_debug.h"
#endif

#if defined(CONFIG_SEC_CORE)
#include "module/osal_atomic.h"
#include "module/osal_debug.h"
#include "module/osal_interrupt.h"
#include "module/osal_mutex.h"
#include "module/osal_task.h"
#include "module/osal_addr.h"
#include "module/osal_cache.h"
#include "module/osal_drvbox.h"
#include "module/osal_fileops.h"
#include "module/osal_wait.h"
#endif
/* osal api header end */

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OSAL_SUCCESS 0
#define OSAL_FAILURE (-1)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* __SOC_OSAL_H__ */
