/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   yaffs_utils.c
  @brief  

  Providing utility functions for YAFFS for builds without OS support

  @author Yufei Leung
  @date   2010-10-06
*/

#ifndef __YAFFS_UTILS_C__
#define __YAFFS_UTILS_C__

#include "stdio.h"
#include "stdlib.h"
#include "string.h"


#ifndef APPLET_ADDR
#define APPLET_ADDR 0x87840000
#endif

#define HEAP_OFFSET  0x00100000
#define HEAP_SIZE    0x00100000


static unsigned long util_heap_start = APPLET_ADDR + HEAP_OFFSET;
static unsigned long util_heap_end = (APPLET_ADDR + HEAP_OFFSET) + HEAP_SIZE;
static unsigned long util_heap_cur = APPLET_ADDR + HEAP_OFFSET;


void *malloc(size_t size)
{
	if (util_heap_cur + size <= util_heap_end) {
		void *ptr = (void *) util_heap_cur;
		
		// align on 32 bytes boundary
		util_heap_cur += size + 31;
		util_heap_cur &= ~0x1f;
		
		memset(ptr, 0, size);
		
		return ptr;
	} else
		return NULL;
}


void free(void *ptr)
{
	/* <HACK> !! No free() */
}


// Return the absolute value
int abs(int num)
{
	return (num < 0) ? (num * (-1)) : num;
}


void __assert_fail (__const char *__assertion, __const char *__file, unsigned int __line, __const char *__function)
{
	uart_printf("assert(%s) failed in file %s, line%d, function %s.\n", __assertion, __file, __line, __function);
	uart_printf("Hanging myself...\n");
	while (1);
}


#endif /* __YAFFS_UTILS_C__ */
