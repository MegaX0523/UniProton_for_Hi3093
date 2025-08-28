/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: debug
 * Author: AuthorNameMagicTag
 * Create: 2021-12-16
 */
#include <stdarg.h>
#include "soc_osal.h"

extern int TestPrintf(const char *format, va_list vaList);

void osal_printk(const char *fmt, ...)
{
    va_list args;

    if (fmt == NULL) {
        return;
    }

    va_start(args, fmt);
    (void)TestPrintf(fmt, args);
    va_end(args);
}