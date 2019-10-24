/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

#include "yaffs_launcher.h"

#include "includes/addrspace.h"
#include "includes/rmem86xxid.h"

#include "includes/emhwlib_registers_tango3.h"

#include "includes/util.h"
#include "includes/mips_memcopy.h"

#define TMP_REMAPPED_REG   CPU_remap2
#define TMP_REMAPPED_BASE  CPU_remap2_address

int strlen(const char *s)
{
	int i;
	for (i = 0; (*s != '\0'); s++, i++);
	return(i);
}

int strnlen(const char *s, int maxlen)
{
	int i;
	for (i = 0; (*s != '\0') && i < maxlen; s++, i++);
	return(i);
}

int strcmp(const char *str1, const char *str2)
{
    int i;

    for (i = 0; (str1[i] == str2[i]) && (str1[i] != 0); ++i)
        ;
    return(str1[i] - str2[i]);
}

int strncmp(const char *str1, const char *str2, int num)
{
    int i;

    for (i = 0; (i < num - 1) && (str1[i] == str2[i]) && (str1[i] != 0); ++i)
        ;
    return str1[i] - str2[i];
}

char *strtok(char *str, const char *delim)
{
    char *ptr;
    const char *mptr;

    for (ptr = str, mptr = delim; *ptr != '\0';) {
	if (*ptr == *mptr) {
	    for (; (*ptr != '\0') && (*mptr != '\0') && (*ptr == *mptr); ptr++, mptr++)
		;
	    if (*mptr == '\0') 
		return(ptr);
	    else 
	        mptr = delim; /* Reset */
	} else
	    ptr++;
    }
    return(NULL);
}

char *strcpy(char *dst, const char *src)
{
	char *tmp = dst;

	while ((*tmp++ = *src++) != '\0');

	return dst;
}

char *strncpy(char *dst, const char *src, int num)
{
	char *tmp = dst;

	while (num) {
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		num--;
	}
	return dst;
}

char *strcat(char *dst, const char *src)
{
	char *tmp = dst;

	while (*tmp)
		tmp++;
	while ((*tmp++ = *src++) != '\0')
		;
	return dst;
}

unsigned int swapl(unsigned int x)
{
    return ((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) |
        (((unsigned long int)(x) & 0x0000ff00U) <<  8) |
        (((unsigned long int)(x) & 0x00ff0000U) >>  8) |
        (((unsigned long int)(x) & 0xff000000U) >> 24)));
}

int isdigit(int ch)
{
	return (ch >= '0' && ch <= '9') ? 1 : 0;
}

void tangox_usleep(int usec)
{
    unsigned long start, end;
    start = gbus_readl(REG_BASE_system_block + SYS_xtal_in_cnt);
    end = start + (usec * 27);
                                                                                
    if (end <= start) 
    	while (gbus_readl(REG_BASE_system_block + SYS_xtal_in_cnt) > start);
    while (gbus_readl(REG_BASE_system_block + SYS_xtal_in_cnt) < end);
}

void tangox_msleep(int msec)
{
    int i;
    for (i = 0; i < msec; i++)
	tangox_usleep(1000);
}

unsigned long get_xtal_in_cnt(void)
{
    return(gbus_readl(REG_BASE_system_block + SYS_xtal_in_cnt));
}

unsigned long time_delta_usec(unsigned long prev_cnt)
{
   unsigned long cnt = gbus_readl(REG_BASE_system_block + SYS_xtal_in_cnt);

   return (cnt - prev_cnt) / (TANGOX_BASE_FREQUENCY / 1000000);
}

void *memcpy(void *_dest, const void *_src, unsigned _n)
{
	char *dest=_dest,*src=(void *)_src;
	int n=_n;
	int lines;

	// align dest to cache line

	while ((n>0)&&((int)dest&31)) {
		*dest=*src;
		n--;
		dest++;
		src++;
	}
	
	// do as many lines as possible

	lines=n>>5;
	if (lines>=2) {
		if ((int)src&3) {
			mips_memcopy_std_ua((int *)src,(int *)dest,lines-2);
			src+=lines<<5;
			dest+=lines<<5;
			n-=lines<<5;
		}
		else {
			mips_memcopy_std((int *)src,(int *)dest,lines-2);
			src+=lines<<5;
			dest+=lines<<5;
			n-=lines<<5;
		}
	}
	
	// finish quietly

	while (n>0) {
		*dest=*src;
		n--;
		dest++;
		src++;
	}
	
	return _dest;
}

int memcmp(const void *ptr1, const void *ptr2, unsigned long n)
{
    unsigned long i;
                                                                                
    unsigned char *_ptr1 = (unsigned char *) ptr1;
    unsigned char *_ptr2 = (unsigned char *) ptr2;
                                                                                
    if (n == 0)
        return 0;
                                                                                
    for (i = 0; i < (n - 1) && _ptr1[i] == _ptr2[i]; ++i)
        ;
                                                                                
    return _ptr1[i] - _ptr2[i];
}

void *memset(void *s, const int c, int n)
{
	char *cptr = (char *)s;
	for (; (n > 0); n--)
		*cptr++ = (char)c;
	return(s);
}

unsigned long string2num(const char *str, int size)
{
    unsigned long res = 0;
    int base = 10;
    int i;

    if ((*str == '0') && ((*(str + 1) == 'x') || (*(str + 1) == 'X'))) {
	 base = 16;
	 str += 2;
    }

    for (i = 0; (*str != '\0') && (i < size); str++, i++) {
	 if ((*str >= '0') && (*str <= '9'))
	     res = (res * base) + (*str - '0');
	 else if (base == 16) {
		 if ((*str >= 'a') && (*str <= 'f'))
			 res = (res * base) + 10 + (*str - 'a');
		 else if ((*str >= 'A') && (*str <= 'F'))
			 res = (res * base) + 10 + (*str - 'A');
		 else
			 return(res);
	 } else 
		 return(res);
    }
    return(res);
}

static unsigned long set_remap(unsigned long remap_reg, unsigned long value)
{
	unsigned long orig = *((volatile unsigned long *)KSEG1ADDR(REG_BASE_cpu_block + remap_reg));
	if (orig != value) {
		*((volatile unsigned long *)KSEG1ADDR(REG_BASE_cpu_block + remap_reg)) = value;
		sync();
	}
	return(orig);
}

unsigned char gbus_readb(unsigned long addr)
{
	unsigned long remap;
	unsigned char tmp;
	if (addr < 0x20000000) // KSEG1 accessible
		return(*((volatile unsigned char *)KSEG1ADDR(addr)) & 0xff);
	else {
		// Use CPU_remapx to temporarily map the address
		remap = set_remap(TMP_REMAPPED_REG, addr & 0xfc000000);
		tmp = *((volatile unsigned char *)KSEG1ADDR(TMP_REMAPPED_BASE + (addr & 0x03ffffff)));
		set_remap(TMP_REMAPPED_REG, remap);
		return(tmp);
	}
}

unsigned short gbus_readw(unsigned long addr)
{
	unsigned long remap;
	unsigned short tmp;
	if (addr < 0x20000000) // KSEG1 accessible
		return(*((volatile unsigned short *)KSEG1ADDR(addr)) & 0xffff);
	else {
		// Use CPU_remapx to temporarily map the address
		remap = set_remap(TMP_REMAPPED_REG, addr & 0xfc000000);
		tmp = *((volatile unsigned short *)KSEG1ADDR(TMP_REMAPPED_BASE + (addr & 0x03ffffff)));
		set_remap(TMP_REMAPPED_REG, remap);
		return(tmp);
	}
}

unsigned long gbus_readl(unsigned long addr)
{
	unsigned long remap;
	unsigned long tmp;
	if (addr < 0x20000000) // KSEG1 accessible
		return(*((volatile unsigned long *)KSEG1ADDR(addr)));
	else {
		// Use CPU_remapx to temporarily map the address
		remap = set_remap(TMP_REMAPPED_REG, addr & 0xfc000000);
		tmp = *((volatile unsigned long *)KSEG1ADDR(TMP_REMAPPED_BASE + (addr & 0x03ffffff)));
		set_remap(TMP_REMAPPED_REG, remap);
		return(tmp);
	}
}

void gbus_writeb(unsigned long addr, unsigned char val)
{
	unsigned long remap;
	if (addr < 0x20000000) // KSEG1 accessible
		*((volatile unsigned char *)KSEG1ADDR(addr)) = (val & 0xff);
	else {
		// Use CPU_remapx to temporarily map the address
		remap = set_remap(TMP_REMAPPED_REG, addr & 0xfc000000);
		*((volatile unsigned char *)KSEG1ADDR(TMP_REMAPPED_BASE + (addr & 0x03ffffff))) = (val & 0xff);
		set_remap(TMP_REMAPPED_REG, remap);
	}
	sync();
}

void gbus_writew(unsigned long addr, unsigned short val)
{
	unsigned long remap;
	if (addr < 0x20000000) // KSEG1 accessible
		*((volatile unsigned short *)KSEG1ADDR(addr)) = (val & 0xffff);
	else {
		// Use CPU_remapx to temporarily map the address
		remap = set_remap(TMP_REMAPPED_REG, addr & 0xfc000000);
		*((volatile unsigned short *)KSEG1ADDR(TMP_REMAPPED_BASE + (addr & 0x03ffffff))) = (val & 0xffff);
		set_remap(TMP_REMAPPED_REG, remap);
	}
	sync();
}

void gbus_writel(unsigned long addr, unsigned long val)
{
	unsigned long remap;
	if (addr < 0x20000000) // KSEG1 accessible
		*((volatile unsigned long *)KSEG1ADDR(addr)) = val;
	else {
		// Use CPU_remapx to temporarily map the address
		remap = set_remap(TMP_REMAPPED_REG, addr & 0xfc000000);
		*((volatile unsigned long *)KSEG1ADDR(TMP_REMAPPED_BASE + (addr & 0x03ffffff))) = val;
		set_remap(TMP_REMAPPED_REG, remap);
	}
	sync();
}

unsigned long tangox_get_sysclock(void)
{
#if (EM86XX_REVISION > 3) || (EM86XX_CHIP >= EM86XX_CHIPID_TANGO3)
	unsigned long sys_clkgen_pll, sysclk_mux, sysclk_premux, n, m, freq, div, k, mux;

	k = m = sys_clkgen_pll = 0;
 	sysclk_mux = gbus_readl(REG_BASE_system_block + SYS_sysclk_mux);
 	sysclk_premux = gbus_readl(REG_BASE_system_block + SYS_sysclk_premux);
	switch(sysclk_premux & 0x3) {
		case 0:
			sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen0_pll);
			m = (sys_clkgen_pll >> 16) & 0x1f;
			k = (sys_clkgen_pll >> 14) & 0x3;
			break;
		case 1:
			sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen1_pll);
			m = (sys_clkgen_pll >> 16) & 0x7f;
			break;
		case 2:
			sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen2_pll);
			m = (sys_clkgen_pll >> 16) & 0x7f;
			break;
		case 3:
			sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen3_pll);
			m = (sys_clkgen_pll >> 16) & 0x7f;
			break;
	}
        n = sys_clkgen_pll & 0x000003ff;

	/* Not using XTAL_IN, cannot calculate */
	if ((sys_clkgen_pll & 0x07000000) != 0x01000000)
		return(0);

	/* Calculate the divider */
	mux = (sysclk_mux >> 8) & 0xf;
	if (mux == 0) /* Get system clock frequency */
		div = 2;
	else if ((mux == 1) || ((mux >= 8) && (mux < 0xc)))
		div = 4;
	else if ((mux >= 2) && (mux < 8))
		div = 3;
	else
		return(0); /* Wrong divider setting */

	if (sysclk_mux & 0x1) 	/* PLL is used */
		freq = ((TANGOX_BASE_FREQUENCY / (m + 2)) * (n + 2)) / (div * (1 << k));
	else
		freq = TANGOX_BASE_FREQUENCY / div;
#else
	unsigned long sys_clkgen_pll, sysclk_mux, n, m, freq, div;
 	sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen_pll);
 	sysclk_mux = gbus_readl(REG_BASE_system_block + SYS_sysclk_mux);
        n = sys_clkgen_pll & 0x000003ff;
        m = (sys_clkgen_pll & 0x003f0000) >> 16;

	/* Calculate the divider */
	if ((sysclk_mux & 0x300) == 0x000) /* Get system clock frequency */
		div = 2;
	else if ((sysclk_mux & 0x300) == 0x100)
		div = 4;
	else
		div = 3;

	if (sysclk_mux & 0x1) 	/* PLL is used */
		freq = ((TANGOX_BASE_FREQUENCY / (m + 2)) * (n + 2)) / div;
	else
		freq = TANGOX_BASE_FREQUENCY / div;
#endif
	return(freq);
}

unsigned long tangox_get_cpuclock(void)
{
#if (EM86XX_REVISION > 3) || (EM86XX_CHIP >= EM86XX_CHIPID_TANGO3)
	unsigned long sys_clkgen_pll, sysclk_mux, sysclk_premux, n, m, freq, div, k, mux;

	k = m = sys_clkgen_pll = 0;
 	sysclk_mux = gbus_readl(REG_BASE_system_block + SYS_sysclk_mux);
 	sysclk_premux = gbus_readl(REG_BASE_system_block + SYS_sysclk_premux);
	switch(sysclk_premux & 0x3) {
		case 0:
			sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen0_pll);
			m = (sys_clkgen_pll >> 16) & 0x1f;
			k = (sys_clkgen_pll >> 14) & 0x3;
			break;
		case 1:
			sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen1_pll);
			m = (sys_clkgen_pll >> 16) & 0x7f;
			break;
		case 2:
			sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen2_pll);
			m = (sys_clkgen_pll >> 16) & 0x7f;
			break;
		case 3:
			sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen3_pll);
			m = (sys_clkgen_pll >> 16) & 0x7f;
			break;
	}
        n = sys_clkgen_pll & 0x000003ff;

	/* Not using XTAL_IN, cannot calculate */
	if ((sys_clkgen_pll & 0x07000000) != 0x01000000)
		return(0);

	/* Calculate the divider */
	mux = (sysclk_mux >> 8) & 0xf;
	if ((mux == 3) || (mux == 4) || (mux == 6)) /* Get CPU frequency */
		div = 3;
	else if ((mux == 8) || (mux == 0xa))
		div = 4;
	else if ((mux == 0) || (mux == 1) || (mux == 2) || (mux == 5) || (mux == 7) || 
			(mux == 9) || (mux == 0xb))
		div = 2;
	else
		return(0); /* Wrong divider setting */

	if (sysclk_mux & 0x1) 	/* PLL is used */
		freq = ((TANGOX_BASE_FREQUENCY / (m + 2)) * (n + 2)) / (div * (1 << k));
	else
		freq = TANGOX_BASE_FREQUENCY / div;
#else
	unsigned long sys_clkgen_pll, sysclk_mux, n, m, freq, div;
 	sys_clkgen_pll = gbus_readl(REG_BASE_system_block + SYS_clkgen_pll);
 	sysclk_mux = gbus_readl(REG_BASE_system_block + SYS_sysclk_mux);
        n = sys_clkgen_pll & 0x000003ff;
        m = (sys_clkgen_pll & 0x003f0000) >> 16;

	if ((sysclk_mux & 0x300) == 0x300) /* Get CPU frequency */
		div = 3;
	else
		div = 2;

	/* Calculate the divider */
	if (sysclk_mux & 0x1) 	/* PLL is used */
		freq = ((TANGOX_BASE_FREQUENCY / (m + 2)) * (n + 2)) / div;
	else
		freq = TANGOX_BASE_FREQUENCY / div;
#endif
	return(freq);
}


#define L1SIZEMAJ (32*1024)
//#ifndef XBOOT3
/* <HACK> In fact this is in XBOOT3 ROM, so should not need to define it here, but if I
   call the XBOOT3 ROM version of this, it will hang, so we use this instead. */
void l1_dcache_range_wi(void *p,RMuint32 size)
{
	/* let's say size is 4MByte, you're not gonna make 1 million iterations thru L1. */

	if (size>L1SIZEMAJ)
		dcache_wi_();
	else
		cache_range_op(p,size,CRO_Hit_Writeback_Inv_D);
}

#define L2SIZEMAJ (512*1024)
void l1_l2_dcache_range_wi(void *p,RMuint32 size)
{
	/* let's say size is 4MByte, you're not gonna make 1 million iterations thru L2. */

	// circulate l1 to l2.
	l1_dcache_range_wi(p,size);

	// circulate l2 to memory.
	if (size>L2SIZEMAJ)
		l2cache_wi_();
	else
		cache_range_op(p,size,CRO_Hit_Writeback_Inv_S);
}
//#endif	/* XBOOT3 */


RMuint32 kva_to_ga(RMuint32 kva)
{
	struct gbus *pgbus = (void *) KSEG1ADDR(REG_BASE_cpu_block + CPU_remap);
	RMuint32 ga_base = 0;
#ifdef YAFFS_LAUNCHER_COMPILEMODE_MIPS16
	RMuint32 pa = RMunshiftBitsVar(kva, 29, 0);
	RMuint32 pa_offset = RMunshiftBitsVar(pa, 26, 0);
	RMuint32 remap_idx = RMunshiftBitsVar(pa, 3, 26);
#else
	RMuint32 pa = RMunshiftBits(kva, 29, 0);
	RMuint32 pa_offset = RMunshiftBits(pa, 26, 0);
	RMuint32 remap_idx = RMunshiftBits(pa, 3, 26);
#endif

	switch (remap_idx) {
	case 0:		/*
				 * The 1st 64MB (pa: 0x00000000-0x03ffffff) has no DRAM 
				 * mapping hence returning the offset directly.
				 */
		break;
	case 7:		/*
				 * The 8th 64MB (pa: 0x1c000000-0x1fffffff) is hardwired
				 * to the 8th 64MB in DRAM0 space.
				 */
		ga_base = 0x9c000000;	/* 0x80000000 + 7 * 64MB */
		break;
	default:		/*
				 * From 2nd - 6th 64MB (pa: 0x04000000-0x1bffffff) are 
				 * mapped by CPU_remap2~CPU_remap7.
				 */
		ga_base = gbus_read_uint32(pgbus, REG_BASE_cpu_block + CPU_remap1 + (remap_idx * 4));
		break;
	}

	return ga_base + pa_offset;
}

RMuint32 xlate_va2ga(RMuint32 va)
{
	if (va >= 0x80000000 && va < 0xc0000000)
		return kva_to_ga(va);
	else
		return 0;	/* Failed to translate */
}

static inline RMuint32 sigmaremap(RMuint32 address)
{
	RMuint32 address31_16=address&(~((1<<16)-1)),
		address15_0=address&((1<<16)-1),
		address31_26,address25_0;
	
#define RB REG_BASE_cpu_block

	if (address31_16==*(volatile RMuint32 *)RM_PA_TO_KVA1(RB+CPU_remap1)) return CPU_remap1_address|address15_0;

	address31_26 = address & (~((1<<26)-1));
	address25_0=address&((1<<26)-1);

	if (address31_26 == 0)
	    return address;
	if (address31_26==*(volatile RMuint32 *)RM_PA_TO_KVA1(RB+CPU_remap2)) return CPU_remap2_address|address25_0;
	if (address31_26==*(volatile RMuint32 *)RM_PA_TO_KVA1(RB+CPU_remap3)) return CPU_remap3_address|address25_0;
	if (address31_26==*(volatile RMuint32 *)RM_PA_TO_KVA1(RB+CPU_remap4)) return CPU_remap4_address|address25_0;
	if (address31_26==*(volatile RMuint32 *)RM_PA_TO_KVA1(RB+CPU_remap5)) return CPU_remap5_address|address25_0;
	if (address31_26==*(volatile RMuint32 *)RM_PA_TO_KVA1(RB+CPU_remap6)) return CPU_remap6_address|address25_0;
	if (address31_26==*(volatile RMuint32 *)RM_PA_TO_KVA1(RB+CPU_remap7)) return CPU_remap7_address|address25_0;

#undef RB

	return 0;
}

RMuint32 xlate_ga2va(RMuint32 ga)
{
	RMuint32 phys_addr = sigmaremap(ga);
	if (phys_addr == 0)
		return 0;	/* Failed to translate */

	return KSEG1ADDR(phys_addr);
}


