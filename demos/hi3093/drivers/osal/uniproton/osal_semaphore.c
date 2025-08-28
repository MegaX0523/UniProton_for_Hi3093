/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: semaphore
 * Author: AuthorNameMagicTag
 * Create: 2021-12-16
 */

#include "prt_mem.h"
#include "prt_sem.h"
#include "prt_sys_external.h"
#include "prt_sem_external.h"
#include "prt_typedef.h"
#include "soc_osal.h"
#include "osal_errno.h"
#include "osal_inner.h"
#include "prt_config.h"

int osal_sem_init(osal_semaphore *sem, int val)
{
    if (sem == NULL || val < 0) {
        osal_log("val=%d parameter invalid!\n", val);
        return OSAL_FAILURE;
    }

    unsigned int ret = PRT_SemCreate(val, (SemHandle *)&(sem->sem)); /* semid may be zero */
    if (ret != OS_OK) {
        osal_log("PRT_SemCreate failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

void osal_sem_destroy(osal_semaphore *sem)
{
    if (sem == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }
    unsigned int ret = PRT_SemDelete((unsigned int)(uintptr_t)sem->sem);
    if (ret != OS_OK) {
        osal_log("PRT_SemDelete failed! ret = %#x.\n", ret);
    }
    sem->sem = (void *)(uintptr_t)g_maxSem;
}

int osal_sem_down(osal_semaphore *sem)
{
    if (sem == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    unsigned int ret = PRT_SemPend((unsigned int)(uintptr_t)sem->sem, OS_WAIT_FOREVER);
    if (ret == OS_ERRNO_SEM_TIMEOUT) {
        osal_log("OS_ERRNO_SEM_TIMEOUT!\n");
        return OSAL_FAILURE;
    } else if (ret != OS_OK) {
        osal_log("PRT_SemPend failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    } else {
        return OSAL_SUCCESS;
    }
}

int osal_sem_down_timeout(osal_semaphore *sem, unsigned int timeout)
{
    if (sem == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    unsigned int ticks = (timeout == OS_WAIT_FOREVER) ? timeout :
     (U32)DIV64(((U64)g_tickModInfo.tickPerSecond * timeout), OS_SYS_MS_PER_SECOND);
    unsigned int ret = PRT_SemPend((unsigned int)(uintptr_t)sem->sem, ticks);
    if (ret == OS_ERRNO_SEM_TIMEOUT) {
        osal_log("OS_ERRNO_SEM_TIMEOUT!\n");
        return OSAL_ETIME;
    } else if (ret != OSAL_SUCCESS) {
        osal_log("PRT_SemPend failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    } else {
        return OSAL_SUCCESS;
    }
}

int osal_sem_trydown(osal_semaphore *sem)
{
    if (sem == NULL) {
        osal_log("parameter invalid!\n");
        return 1;
    }

    unsigned int ret = PRT_SemPend((unsigned int)(uintptr_t)sem->sem, OS_NO_WAIT);
    if (ret != OS_OK) {
        osal_log("PRT_SemPend failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    } else {
        return OSAL_SUCCESS;
    }
}

void osal_sem_up(osal_semaphore *sem)
{
    if (sem == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

    unsigned int ret = PRT_SemPost((unsigned int)(uintptr_t)sem->sem);
    if (ret != OS_OK) {
        osal_log("PRT_SemPost failed! ret = %#x.\n", ret);
    }
}
