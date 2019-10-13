/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

#include "addrspace.h"
#include "rmem86xxid.h"

#include "emhwlib_registers_tango3.h"

#define TANGOX_BASE_FREQUENCY	27000000

#ifndef __ASSEMBLY__

void *memset(void *s, const int c, int n);
void *memcpy(void *d, const void *s, unsigned n);
int memcmp(const void *ptr1, const void *ptr2, unsigned long n);

int strlen(const char *s);
int strnlen(const char *s, int maxlen);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, int num);
int isdigit(int ch);
char *strtok(char *str, const char *delim);

char *strncpy(char *dst, const char *src, int num);
char *strcpy(char *dst, const char *src);
char *strcat(char *dst, const char *src);

void tangox_msleep(int msec);
void tangox_usleep(int usec);

unsigned long get_xtal_in_cnt(void);
unsigned long time_delta_usec(unsigned long prev_cnt);

unsigned int swapl(unsigned int x);

unsigned long string2num(const char *str, int size);

unsigned char gbus_readb(unsigned long addr);
unsigned short gbus_readw(unsigned long addr);
void gbus_writeb(unsigned long addr, unsigned char val);
void gbus_writew(unsigned long addr, unsigned short val);
unsigned long gbus_readl(unsigned long addr);
void gbus_writel(unsigned long addr, unsigned long val);

#define gbus_write_uint32_ireg(h,a,v) gbus_writel(a,v)
#define gbus_read_uint32_ireg(h,a) gbus_readl(a)

#if 0
RMuint8 gbus_read_uint8(struct gbus *pgbus, RMuint32 addr);
RMuint16 gbus_read_uint16(struct gbus *pgbus, RMuint32 addr);
RMuint32 gbus_read_uint32(struct gbus *pgbus, RMuint32 addr);
void gbus_write_uint8(struct gbus *pgbus, RMuint32 addr, RMuint8 data);
void gbus_write_uint16(struct gbus *pgbus, RMuint32 addr, RMuint16 data);
void gbus_write_uint32(struct gbus *pgbus, RMuint32 addr, RMuint32 data);
#endif

RMuint32 kva_to_ga(RMuint32 kva);

RMuint32 xlate_va2ga(RMuint32 va);

unsigned long tangox_get_sysclock(void);
unsigned long tangox_get_cpuclock(void);

void __sync(void);
#define sync()	__sync()

RMuint32 icache_i_(void);
RMuint32 dcache_wi_(void);
RMuint32 l2cache_wi_(void);
void cache_range_op(void *p,RMuint32 size,RMuint32 op);

#endif /* !__ASSEMBLY__ */

#endif /* !_UTIL_H_ */

