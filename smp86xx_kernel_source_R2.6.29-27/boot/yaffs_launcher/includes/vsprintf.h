/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/* This file is part of the boot loader */

/*
 * vsprintf.h
 *
 * This is the porting of <stdarg.h> for ARM processor
 * also including function prototypes defined in vsprintf.c
 */

#ifndef __VSPRINTF_H
#define __VSPRINTF_H

// stdarg
typedef void *va_list;

// size of memory storing the data with specified type
// data size must be multiple of sizeof(int) : 4 bytes
#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

// stdarg standard macros
#define va_start(AP, LASTARG)   (AP = (va_list) __builtin_next_arg(LASTARG))
#define va_end(AP)
#define va_arg(AP, TYPE)        (AP = (va_list) ((char *) (AP) + __va_rounded_size (TYPE)), *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size(TYPE))))

/* function prototypes */
        
int sprintf(char * buf, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int vsprintf(char *buf, const char *fmt, va_list args);

#endif 

