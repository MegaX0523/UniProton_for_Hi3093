/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: interrupt
 * Author: AuthorNameMagicTag
 * Create: 2021-12-16
 */

#include "prt_buildef.h"
#include "prt_config.h"
#include "prt_irq_internal.h"
#include "prt_hwi.h"
#include "prt_typedef.h"
#include "prt_lib_external.h"
#include "prt_sys_external.h"
#include "prt_irq_external.h"
#include "soc_osal.h"
#include "osal_errno.h"
#include "osal_inner.h"

struct osal_irq_info {
    osal_irq_handler handler;
    void *dev;
    unsigned int irq;
};

static struct osal_irq_info g_osal_irq_array[OS_HWI_MAX_NUM_CONFIG + 1];

static void osal_callback(void* args)
{
    if (args == NULL) {
        return;
    }
    struct osal_irq_info* handle = (struct osal_irq_info*)args;
    if (handle->handler != NULL) {
        handle->handler(handle->irq, handle->dev);
    }
}

int osal_irq_request(unsigned int irq, osal_irq_handler handler, osal_irq_handler thread_fn,
    const char *name, void *dev)
{
    osal_unused(name);
    osal_unused(thread_fn);

    if (handler == NULL) {
        osal_log("handler is NULL !\n");
        return OSAL_FAILURE;
    }

    if (irq >= OS_HWI_MAX_NUM) {
        return OSAL_FAILURE;
    }

    g_osal_irq_array[irq].irq = irq;
    g_osal_irq_array[irq].dev = dev;
    g_osal_irq_array[irq].handler = handler;
    g_hwiModeForm[irq].mode = OS_HWI_MODE_ENGROSS;

    unsigned int ret = PRT_HwiCreate(irq, (HwiProcFunc)osal_callback, (HwiArg)&g_osal_irq_array[irq]);
    if (ret != OS_OK) {
        osal_log("PRT_HwiCreate failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}

void osal_irq_free(unsigned int irq, void *dev)
{
    osal_unused(dev);

    if (irq > OS_HWI_MAX_NUM) {
        return;
    }

    g_osal_irq_array[irq].irq = 0;
    g_osal_irq_array[irq].dev = NULL;
    g_osal_irq_array[irq].handler = NULL;

    unsigned int ret = PRT_HwiDelete(irq);
    if (ret != OS_OK) {
        osal_log("PRT_HwiDelete failed! ret = %#x.\n", ret);
    }
}

int osal_in_interrupt(void)
{
    return (OS_INT_ACTIVE);
}

void osal_irq_enable(unsigned int irq)
{
    unsigned long long ret = PRT_HwiEnable(irq);
    if (ret != OS_OK) {
        osal_log("PRT_HwiEnable failed!\n");
    }
}

void osal_irq_disable(unsigned int irq)
{
    unsigned long long ret = PRT_HwiDisable(irq);
    if (ret != OS_OK) {
        osal_log("PRT_HwiDisable failed!\n");
    }
}

unsigned int osal_irq_lock(void)
{
    return PRT_HwiLock();
}

unsigned int osal_irq_unlock(void)
{
    return PRT_HwiUnLock();
}

void osal_irq_restore(unsigned int irq_status)
{
    PRT_HwiRestore(irq_status);
}

int osal_irq_set_priority(unsigned int irq, unsigned short priority)
{
    unsigned int ret;
    ret = PRT_HwiSetAttr(irq, priority, OS_HWI_MODE_ENGROSS);
    return ret == OS_OK ? OSAL_SUCCESS : OSAL_FAILURE;
}

/*
 * 描述: 计算一个32bit非0数字的最右位, e.g. 0x01000020 ----> 结果返回 5
 */
static inline unsigned int os_getrmb(unsigned int bit)
{
    unsigned int rev = bit - 1;
    unsigned int result;

    OS_EMBED_ASM("EOR %0, %1, %2" : "=r"(result) : "r"(rev), "r"(bit));
    OS_EMBED_ASM("CLZ %0, %1" : "=r"(rev) : "r"(result));

    return (OS_DWORD_BIT_NUM - rev - 1);
}

int osal_irq_set_affinity(unsigned int irq, const char *name, int cpu_mask)
{
    osal_unused(name);
    unsigned int target_core;

    if ((cpu_mask & (cpu_mask - 1)) != 0) { // SPI中断目前不支持1-N，只支持1-1
        return OSAL_FAILURE;
    }

    target_core = os_getrmb(cpu_mask);
    OsGicSetTargetId(irq, target_core);

    return OSAL_SUCCESS;
}