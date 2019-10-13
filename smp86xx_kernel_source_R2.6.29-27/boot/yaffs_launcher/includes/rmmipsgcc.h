/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   rmmipsgcc.h
  @brief  

  This file gathers a series of inline functions / macros to
  do some MIPS specific operations (registers, coprocessor zero, cache, ...)

  some of them only allowed in kernel mode.

  It is under rmdef/ but is not included by default with rmdef.h because:
  * it is mostly gcc stuff (inline assembly is gcc specific)
  * end[] and such could conflict with other compilers.

  It works for sde-gcc and mipsel-gcc.
  To achieve this, you need to use human unfriendly register names ($2, $3, etc);
  the CP0 regs are redefined here (RM_C0_*). On sde-gcc that would be C0_* and CP0_* on mipsel-gcc.
  Same goes for cache maintenance bits.
  Refer to #7319 for more details on this specific topic.

  @author Emmanuel Michon
  @date   2007-03-27
*/

#ifndef __RMMIPSGCC_H__
#define __RMMIPSGCC_H__

/* most probably already there */
#include "rmdef.h"

#define RM_C0_INDEX        $0
#define RM_C0_RANDOM       $1
#define RM_C0_ENTRYLO0     $2
#define RM_C0_ENTRYLO1     $3
#define RM_C0_CONTEXT      $4
#define RM_C0_PAGEMASK     $5
#define RM_C0_PAGEGRAIN    $5,1
#define RM_C0_WIRED        $6
#define RM_C0_HWRENA       $7
#define RM_C0_BADVADDR     $8
#define RM_C0_COUNT        $9
#define RM_C0_ENTRYHI      $10
#define RM_C0_COMPARE      $11
#define RM_C0_STATUS       $12
#define RM_C0_INTCTL       $12,1
#define RM_C0_SRSCTL       $12,2
#define RM_C0_SRSMAP       $12,3
#define RM_C0_CAUSE        $13
#define RM_C0_EPC          $14
#define RM_C0_PRID         $15
#define RM_C0_EBASE        $15,1
#define RM_C0_CONFIG       $16
#define RM_C0_CONFIG0      $16,0
#define RM_C0_CONFIG1      $16,1
#define RM_C0_CONFIG2      $16,2
#define RM_C0_CONFIG3      $16,3
#define RM_C0_CONFIG7      $16,7
#define RM_C0_LLADDR       $17
#define RM_C0_WATCHLO      $18
#define RM_C0_WATCHLO1     $18,1
#define RM_C0_WATCHLO2     $18,2
#define RM_C0_WATCHLO3     $18,3
#define RM_C0_WATCHHI      $19
#define RM_C0_WATCHHI1     $19,1
#define RM_C0_WATCHHI2     $19,2
#define RM_C0_WATCHHI3     $19,3
#define RM_C0_DEBUG        $23
#define RM_C0_DEPC         $24
#define RM_C0_PERFCNT      $25
#define RM_C0_ERRCTL       $26
#define RM_C0_CACHEERR     $27

/* 4KE */
#define RM_C0_TAGLO  $28   
#define RM_C0_DATALO $28,1 
/* provisioned by 4KE cache setup routine but actually not implemented on 4KE */
#define RM_C0_TAGHI  $29   
#define RM_C0_DATAHI $29,1 

/* 24K */
#define RM_C0_ITAGLO  $28
#define RM_C0_IDATALO $28,1
#define RM_C0_DTAGLO  $28,2
#define RM_C0_DDATALO $28,3

#define RM_C0_IDATAHI $29,1

/* 74K */
#define RM_C0_ITAGHI $29
#define RM_C0_DTAGHI $29,2

#define RM_C0_ERRPC        $30
#define RM_C0_DESAVE       $31

/* L2 (e.g. mostly for 1004K) */
#define RM_C0_L23TAGLO $28,4
#define RM_C0_L23DATALO $28,5

/* this one is provisioned by the l2 cache setup routine but actually absent on 24K, 74K, 1004K L2 access routines */
#define RM_C0_L23TAGHI $29,4

#define RM_C0_L23DATAHI $29,5

/*
  no dollar
 */
#define C0_INDEX        0
#define C0_RANDOM       1
#define C0_ENTRYLO0     2
#define C0_ENTRYLO1     3
#define C0_CONTEXT      4
#define C0_PAGEMASK     5
#define C0_WIRED        6
#define C0_HWRENA       7
#define C0_BADVADDR     8
#define C0_COUNT        9
#define C0_ENTRYHI      10
#define C0_COMPARE      11
#define C0_STATUS       12
#define C0_CAUSE        13
#define C0_EPC          14
#define C0_PRID         15
#define C0_CONFIG       16
#define C0_LLADDR       17
#define C0_WATCHLO      18
#define C0_WATCHHI      19
#define C0_DEBUG        23
#define C0_DEPC         24
#define C0_PERFCNT      25
#define C0_ERRCTL       26
#define C0_CACHEERR     27
#define C0_TAGLO  28   
#define C0_TAGHI  29   
#define C0_ITAGLO  28
#define C0_ITAGHI 29
#define C0_ERRPC        30
#define C0_DESAVE       31

/*
 * Exception Codes
 */
#define RM_EXC_INTR     0
#define RM_EXC_MOD      1
#define RM_EXC_TLBL     2
#define RM_EXC_TLBS     3
#define RM_EXC_ADEL     4
#define RM_EXC_ADES     5
#define RM_EXC_IBE      6
#define RM_EXC_DBE      7
#define RM_EXC_SYS      8
#define RM_EXC_BP       9
#define RM_EXC_RI       10
#define RM_EXC_CPU      11
#define RM_EXC_OVF      12
#define RM_EXC_TRAP     13
#define RM_EXC_RES14    14
#define RM_EXC_FPE      15
#define RM_EXC_IS1      16
#define RM_EXC_IS2      17
#define RM_EXC_C2E      18
#define RM_EXC_RES19    19
#define RM_EXC_RES20    20
#define RM_EXC_RES21    21
#define RM_EXC_MDMX     22
#define RM_EXC_WATCH    23
#define RM_EXC_MCHECK   24
#define RM_EXC_THREAD   25
#define RM_EXC_DSPU     26
#define RM_EXC_RES27    27
#define RM_EXC_RES28    28
#define RM_EXC_RES29    29
#define RM_EXC_RES30    30
#define RM_EXC_RES31    31

#define RM_CR_BD        0x80000000
#define RM_CR_IMASK     0x0000ff00

#define LOG2_RM_SR_IE      0
#define LOG2_RM_SR_EXL     1
#define LOG2_RM_SR_UM      4
#define LOG2_RM_SR_IM0     8
#define LOG2_RM_SR_IM1     9
#define LOG2_RM_SR_IM2     10
#define LOG2_RM_SR_IM3     11
#define LOG2_RM_SR_IM4     12
#define LOG2_RM_SR_IM5     13
#define LOG2_RM_SR_IM6     14
#define LOG2_RM_SR_IM7     15
#define LOG2_RM_SR_BEV     22
#define LOG2_RM_SR_MX      24
#define LOG2_RM_SR_FR      26
#define LOG2_RM_SR_CU0     28
#define LOG2_RM_SR_CU1     29 /* e.g. FPU */

#define RM_SR_IE           (1<<LOG2_RM_SR_IE)      /* interrupt enable */
#define RM_SR_EXL          (1<<LOG2_RM_SR_EXL)
#define RM_SR_UM           (1<<LOG2_RM_SR_UM) 
#define RM_SR_IM0          (1<<LOG2_RM_SR_IM0)
#define RM_SR_IM1          (1<<LOG2_RM_SR_IM1)
#define RM_SR_IM2          (1<<LOG2_RM_SR_IM2)
#define RM_SR_IM3          (1<<LOG2_RM_SR_IM3)
#define RM_SR_IM4          (1<<LOG2_RM_SR_IM4)
#define RM_SR_IM5          (1<<LOG2_RM_SR_IM5)
#define RM_SR_IM6          (1<<LOG2_RM_SR_IM6)
#define RM_SR_IM7          (1<<LOG2_RM_SR_IM7)
#define RM_SR_BEV          (1<<LOG2_RM_SR_BEV)
#define RM_SR_MX          (1<<LOG2_RM_SR_MX)
#define RM_SR_FR          (1<<LOG2_RM_SR_FR)
#define RM_SR_CU0          (1<<LOG2_RM_SR_CU0)
#define RM_SR_CU1          (1<<LOG2_RM_SR_CU1)

#define RM_CFG_C_UNCACHED             2
#define RM_CFG_C_WBACK                3
#define RM_CFG_C_MASK                 7 

#define RM_TLB_PFNMASK       0x3fffffc0
#define RM_TLB_PFNSHIFT      6
#define RM_TLB_FLAGS         0x0000003f
#define RM_TLB_CMASK         0x00000038
#define RM_TLB_CSHIFT        3

#define RM_TLBHI_VPN2MASK    0xffffe000
#define RM_TLBHI_VPN2SHIFT   13
#define RM_TLBHI_VPNMASK     0xfffff000
#define RM_TLBHI_VPNSHIFT    12
#define RM_TLBHI_PIDMASK     0x000000ff
#define RM_TLBHI_PIDSHIFT    0x00000000

#define RM_TLBHIVPN(hi)      ((hi)>>RM_TLBHI_VPNSHIFT)
#define RM_TLBHIID(hi)       ((hi)&RM_TLBHI_PIDMASK)
#define RM_TLBLOPN(lo)       ((lo)>>RM_TLB_PFNSHIFT)
#define RM_TLBLOFLAGS(lo)    ((lo)&RM_TLB_FLAGS)

#define RM_TLB_G             0x00000001
#define RM_TLB_V             0x00000002
#define RM_TLB_D             0x00000004
#define RM_TLB_UNCACHED      (RM_CFG_C_UNCACHED<<RM_TLB_CSHIFT)
#define RM_TLB_WBACK         (RM_CFG_C_WBACK<<RM_TLB_CSHIFT)

#define WATCH_G (1<<14)
#define ERRCTL_WST	(1<<29) /* way select / tag array */
#define ERRCTL_SPR	(1<<28) /* scratchpad ram select */

/*
  Index_* operate on a cache index (global operation on the cache or explicit feed)
  Hit_* operate on a virtual address.

  Some features like Writeback_D do not exist in the first flavour. For instance you can't
  writeback the whole d-cache without doing _wi (RM_Index_Writeback_Inv_D).
 */

#define RM_Index_Invalidate_I               0x0         /* 0       0 */
#define RM_Index_Writeback_Inv_D            0x1         /* 0       1 */
#define RM_Index_Writeback_Inv_S            0x3         /* 0       3 */

#define RM_Index_Load_Tag_I                 0x4         /* 1       0 */
#define RM_Index_Load_Tag_D                 0x5         /* 1       1 */
#define RM_Index_Load_Tag_S                 0x7

#define RM_Index_Store_Tag_I                0x8         /* 2       0 */
#define RM_Index_Store_Tag_D                0x9         /* 2       1 */
#define RM_Index_Store_Tag_S                0xb

#define RM_Index_Store_Data_I               0xc

#define RM_Hit_Invalidate_I                 0x10        /* 4       0 */
#define RM_Hit_Invalidate_D                 0x11        /* 4       1 */
#define RM_Hit_Invalidate_S                 0x13        /* 4       3 */

#define RM_Fill_I                           0x14        /* 5       0 */
#define RM_Hit_Writeback_Inv_D              0x15        /* 5       1 */
#define RM_Hit_Writeback_Inv_S              0x17        /* 5       3 */

#define RM_Hit_Writeback_D                  0x19        /* 6       1 */
#define RM_Hit_Writeback_S                  0x1b        /* 6       3 */

#define RM_Fetch_Lock_I                     0x1c        /* 7       0 */
#define RM_Fetch_Lock_D                     0x1d        /* 7       1 */
#define RM_Fetch_Lock_S                     0x1f        /* 7       3 */

/*
   Coprocessor2 registers
WARNING: this is implementation specific and subject to change from one
version to another. There is currently a single version... Don't forget
to update those definitions with #ifdefs the day we change registers
mapping... Subject to change!

There is currently two registers set:
   - the first set (set 0) is reserved for remapped registers
       the mapping is currently fixed with each register from
   the first set pointing to the same register in the second
   coprocessor2 registers set.

   - the second set (set 1) contains the following registers map:
       regs 0...15 = inflate registers area
       regs 16..23 = png registers area
       regs 24..27 = backup/restore registers area
       regs 28..31 = backup management registers zone
*/

/*
   Coprocessor2 registers: implementation specific
no dollar
 */

/* inflate registers: set 0,1 */
#define C2_INFLATE_CMDSTAT       0
#define C2_INFLATE_LVLCFG        1
#define C2_INFLATE_SYMCFG        2
#define C2_INFLATE_IN_CTL        3
#define C2_INFLATE_IN_DATA       4
#define C2_INFLATE_OUT_STATUS    5
#define C2_INFLATE_OUT_LITERALS  6
#define C2_INFLATE_OUT_DISTANCES 7
/* png registers: set 0,1 */
#define C2_PNG_CONFIG            16
#define C2_PNG_IN_DATA           17
#define C2_PNG_OUT_DATA          18
#define C2_PNG_PREV_DATA_UP      19
#define C2_PNG_PREV_DATA_UP_LEFT 20
#define C2_PNG_PREV_DATA_LEFT    21
/* backup registers: set 0,1 */
#define C2_BACKUP_ADDRESS        24
#define C2_BACKUP_DATA           25
/* coprocessor2 registers: set 0,1 */
#define C2_IMPLEMENTATION        28

/* TODO: defines masks and bits ops */

/*
   Coprocessor2 registers: implementation specific
dollar declarations
we don't specifiy any registers set here since we wanna
be able to access those registers with swc2/lwc2 instructions
as well. Thus we only use the registers included in the first
registers set, that're currently mapped onto the second register
set. This is subject to change!
 */

/* inflate registers */
#define RM_C2_INFLATE_CMDSTAT       $0
#define RM_C2_INFLATE_LVLCFG        $1
#define RM_C2_INFLATE_SYMCFG        $2
#define RM_C2_INFLATE_IN_CTL        $3
#define RM_C2_INFLATE_IN_DATA       $4
#define RM_C2_INFLATE_OUT_STATUS    $5
#define RM_C2_INFLATE_OUT_LITERALS  $6
#define RM_C2_INFLATE_OUT_DISTANCES $7
/* png registers */
#define RM_C2_PNG_CONFIG            $16
#define RM_C2_PNG_IN_DATA           $17
#define RM_C2_PNG_OUT_DATA          $18
#define RM_C2_PNG_PREV_DATA_UP      $19
#define RM_C2_PNG_PREV_DATA_UP_LEFT $20
#define RM_C2_PNG_PREV_DATA_LEFT    $21
/* backup registers */
#define RM_C2_BACKUP_ADDRESS        $24
#define RM_C2_BACKUP_DATA           $25
/* coprocessor2 registers */
#define RM_C2_IMPLEMENTATION        $28

#define RM_HWR_CPUNUM 0
#define RM_HWR_SYNCI  1
#define RM_HWR_COUNT  2  /* CP0 Count register */
#define RM_HWR_CCRES  3

#ifdef __ASSEMBLY__

/*
  That's a common trap of mips assembly.
  You should use addu and subu instead.

  Those might set some flags you don't want and were meant for COBOL! See MIPS run second edition 8.7.4
 */
#define add add_dont_use_overflow_variants_see_mips_run_8_7_4
#define sud sub_dont_use_overflow_variants_see_mips_run_8_7_4

/*
  code sourcery adds `frame sp...' something that fails to build because sp is written $sp.
 */
#undef LEAF

/*
  bug in assembler (?) you cannot pad mips16 asm with a sequence like 0x65 0x00 (nop).
  So I had to choose a byte-repeating pattern.
  This one is roughly safe

  7777      	cmpi	a3,119
*/
#define LEAF(symbol)                                    \
                .globl  symbol;                         \
	.align 2,0x77;				\
                .type   symbol,@function;               \
                .ent    symbol,0;                       \
symbol:

#ifndef END
#define END(function)                                   \
                .end    function;                       \
                .size   function,.-function
#endif

/*
  This one is always ok when address is in kseg0 or kseg1
 */
#define RM_KVA_TO_PA(x) ((x)&((1<<29)-1))

/*
  These ones only when address is <0x20000000 (and has possible side effects with remap regs)
 */
#define RM_PA_TO_KVA0(x) ((x)|(4<<29))
#define RM_PA_TO_KVA1(x) ((x)|(5<<29))

#else

#define RM_KVA_TO_PA(x) RMunshiftBits(x,29,0)
#define RM_KVA_TO_PAalt(x) ((RMuint32)(x)&((1<<29)-1))
#define RM_PA_TO_KVA0(x) (RMuint32)((RMuint32)(x)|(4<<29))
#define RM_PA_TO_KVA1(x) (RMuint32)((RMuint32)(x)|(5<<29))

/*
  See mips run second edition 9.5.1
  You can only take pointers to those obviously

  Don't assume they're 32bit aligned
  _ftext <= etext
  _fdata <= edata
  _fbss <= end

  Do not do assumptions on the location of _frodata. When using harvard model, rodata can't be in text.
*/
extern RMascii _ftext[], _frodata[], etext[], _fdata[], edata[], _fbss[], end[], _gp[];

#define ASMPROLOGUE ".set push\n.set noreorder\n"
#define ASMEPILOGUE ".set pop\n"

/*
  Basically this is

  x=c?a:b;

  but forcing this assembly implementation is for cryptography microanalysis resistance
  both branches are garanteed to take the same time
 */
#define RMmovn(c,a,b)							\
({									\
	RMuint32 __hidden_rv;						\
									\
	__asm__ __volatile__ (ASMPROLOGUE				\
			      "move %0, %2\n"				\
			      "movn %0, %1, %3\n"			\
			      ASMEPILOGUE				\
			      : "=&r" (__hidden_rv)			\
			      : "r" ((RMuint32)(a)), "r" ((RMuint32)(b)), "r" ((RMuint32)(c)));	\
									\
	__hidden_rv;							\
})

/*
  obviously conflicting with gcc register allocation
  useful in some cases, like getting $sp, or using $k0/$k1 to make spaghettis.
 */
#define RMgetReg(reg) \
({ \
	RMuint32 __hidden_rv;					   \
	__asm__ __volatile__ (ASMPROLOGUE "move %0, " reg "\n" ASMEPILOGUE \
			    : "=r" (__hidden_rv) );			\
	__hidden_rv;							\
})

#define RMsetReg(reg,val)	\
do { \
	__asm__ __volatile__ (ASMPROLOGUE "move " reg ", %0\n" ASMEPILOGUE \
			      : : "r" ((RMuint32)(val)) );		\
} while (0)

/*
  beyond this point, this is only for kernel mode.
 */

#define RMmfc0(reg, sel) \
({ \
	RMuint32 __hidden_rv;						\
	__asm__ __volatile__ (ASMPROLOGUE "mfc0 %0, " STR(reg) ", " #sel "\n" ASMEPILOGUE \
			    : "=r" (__hidden_rv));			\
	__hidden_rv;							\
})

/*
  The memory is a clobber here because you may change a TLB setting, e.g. have a part of addressing space
  move under gcc's feet.
*/
#define RMmtc0(reg, sel, val) \
do { \
	__asm__ __volatile__ (ASMPROLOGUE "mtc0 %0, " STR(reg) ", " #sel "\n" ASMEPILOGUE \
                        : \
			      : "r" ((RMuint32)(val))	\
                        : "memory"); \
} while (0)

/*
  RMrdhwr allows reading the following registers from user mode.
  Access to these registers must have been previously enabled in RM_C0_HWRENA
  (bug#9046 and See MIPS run second edition section 8.5.12)
 */
#define RMrdhwr(reg) \
({ \
        RMuint32 __hidden_rv;                                           \
        __asm__ __volatile__ (ASMPROLOGUE "rdhwr %0, $" STR(reg) "\n" ASMEPILOGUE \
                            : "=r" (__hidden_rv));                      \
        __hidden_rv;                                                    \
})

/***************************************************/
/* timing functions --- in clocks ******************/
/***************************************************/

/*
  val is a previously measured C0_COUNT value.
  By nice effect of 32bit arith, is ok when counter just looped.
  Safe as long as 32bit counter has not looped multiple times.
  At 200MHz, rollover is 21 seconds.
  
  Ex. Measure a duration in clocks
  
  volatile RMuint32 mark=RMmfc0(RM_C0_COUNT,0);
  .. do something ..
  d_clk=RMclocksElapsedSince(mark);
*/
#ifdef XBOOT3
#define GETCOUNT() getcount()
#else
#define GETCOUNT() RMmfc0(RM_C0_COUNT,0)
#endif

#define RMclocksElapsedSince(val) (RMuint32)( /*every other*/ 2* /* <--- ! */ (GETCOUNT()-(val)))

/*
  expect this macro to sleep too long when clocks <= 50 
  if you need so short delays, you should use dedicated hardware, not a general purpose cpu
*/
#define RMclocksBusySleep(clocks)			\
do {						\
	RMuint32 clocks_nosideeffect=(clocks);	\
	RMuint32 mark=GETCOUNT();	\
	while (RMclocksElapsedSince(mark)<(clocks_nosideeffect));	\
} while (0)

/*
  The following macro returns an approximate value in Hz slightly greater (*) than 
  the processor core frequency based on those assumptions:

  - SYS_xtal_in_cnt is revolving @27MHz [so this function does not work on palladium]

  - use probe_length_ms >= 1. The longer the better though, if you like to wait

  - accuracy is affected by: interrupts (better run that with interrupts disabled), 
  caching, gbus latency. Note that all these perturbations will make the measure larger, hence (*).

  Such a function that polls crazy on gbus should be avoided. Use it once per time you change
  the clock settings, better with interrupts disabled.

  (*) It is important when writing a facility such as RMmicrosecondsBusySleep() that it sleeps no less
  than the specified amount (but maybe a little more). 
  The fact RMclocksCalibrate() returns a value that is known to be slightly in excess serves this purpose.

  em. 2007 dec 17th
*/
#define RMclocksCalibrate(pgbus,probe_length_ms)			\
({									\
	RMuint32 __f_hz,x0,x1,mark;                                     \
									\
	mark = GETCOUNT();                                   \
	x0=gbus_read_uint32(pgbus,REG_BASE_system_block+SYS_xtal_in_cnt); \
	do {                                                            \
		x1=gbus_read_uint32(pgbus,REG_BASE_system_block+SYS_xtal_in_cnt); \
	}                                                               \
	while ((x1-x0) < (27000*(probe_length_ms)));                    \
									\
	__f_hz =RMclocksElapsedSince(mark)*(1000/(probe_length_ms));	\
									\
	__f_hz;								\
})

#define RMclocksCalibrateAlt(pgbus,probe_length_clk)			\
({									\
	RMuint32 __f_hz,dx,mark;					\
									\
	mark = gbus_read_uint32(pgbus,REG_BASE_system_block+SYS_xtal_in_cnt); \
	RMclocksBusySleep(probe_length_clk); \
	dx=gbus_read_uint32(pgbus,REG_BASE_system_block+SYS_xtal_in_cnt)-mark; \
	__f_hz =(27000*probe_length_clk/dx)*1000; \
									\
        dx?__f_hz:13500000;							\
})

#define RMclocksCalibrateSys(pgbus,probe_length_ms)			\
({									\
	RMuint32 __f_hz,x0,x1,mark;                                     \
									\
	mark = gbus_read_uint32(pgbus,REG_BASE_system_block+SYS_clk_cnt); \
	x0=gbus_read_uint32(pgbus,REG_BASE_system_block+SYS_xtal_in_cnt); \
	do {                                                            \
		x1=gbus_read_uint32(pgbus,REG_BASE_system_block+SYS_xtal_in_cnt); \
	}                                                               \
	while ((x1-x0) < (27000*(probe_length_ms)));                    \
									\
	__f_hz =(gbus_read_uint32(pgbus,REG_BASE_system_block+SYS_clk_cnt)-mark)*(1000/(probe_length_ms)); \
									\
	__f_hz;								\
})

/*
  #define RMnanosecondsBusySleep(f_hz,ns) does not exist, on purpose. 
*/

/*
  Could be the same if we were not plagued by 32bit mul overflow
 */
#define RMmicrosecondsBusySleep(f_hz,us) RMclocksBusySleep((RMuint32)(us)*((f_hz)/1000000))
#define RMmillisecondsBusySleep(f_hz,ms) RMclocksBusySleep((RMuint32)(ms)*((f_hz)/1000))

/***************************************************/
/* timing functions --- end ************************/
/***************************************************/

/**
   Setup a watchpoint on [start&(~mask), start|mask[ area

   Example: setwatch(5,3,0x90100010,0x1ff,3);

   sets the watchpoint #5 to monitor load&store in 0x90100000..0x901001ff
   on asid 3

   Refer to MIPS software manual section 5.2.26

   This has to be implemented as a macro because there is no
   way to transmit the first parameter nr to the third operand of `mtc0'

   @param nr: 0..7
   @param mode: 1=sw 2=lw 4=instruction fetch (can be or'ed)
   @param start: will be aligned to 8 byte boundary
   @param mask: 2^n-1 with 7 <= mask <= 0xfff
   @param asid: asid, or WATCH_G for `global'
*/
#define RMsetWatch(nr,mode,start,mask,asid)				\
do {									\
	__asm__ __volatile__(ASMPROLOGUE				\
			     "mtc0 %0, " STR(RM_C0_WATCHHI) ", " #nr "\n"	\
			     "mtc0 %1, " STR(RM_C0_WATCHLO) ", " #nr "\n"	\
			     ASMEPILOGUE				\
			     :						\
			     : "r" (((asid)<<16)|((mask)&~7)), "r" (((RMuint32)(start)&~7)|(mode))); \
}									\
while (0)
#define RMclearWatch(nr) \
do {									\
	__asm__ __volatile__(ASMPROLOGUE				\
			     "mtc0 $0, " STR(RM_C0_WATCHHI) ", " #nr "\n"	\
			     "mtc0 $0, " STR(RM_C0_WATCHLO) ", " #nr "\n"	\
			     ASMEPILOGUE);				\
}									\
while (0)

#ifdef __mips16
#define FORCE32 __attribute__((nomips16)) 
#else
#define FORCE32
#endif

/*
  ON 8656ES3 and up all the ?cache_* are obsoleted by cache_range_op() that better handles
  the `cache stepping' with rdhwr $1. It is planned the ipu goes from L1 lines of 16 bytes -> 32 bytes at some point

  Archop hardcodes 16 bytes which works on 16/32 byte lines
 */

/**
   All cache related instructions below operate on cache lines.
   A cache line is 32bytes on 24Kf and 16bytes on 4KE.

   For the *_range_* that come afterwards, the address increment is hardcoded to 16, so they will be
   correct, but twice slower, on 24K.

   When relocating a piece of code, you will have to writeback data cache, invalidate instruction cache.
   As soon as the size exceeds the cache size, the *_range_* functions will work but take too long.
   In this case, the ROM has functions to operate on the entire cache instead.

   When locking a range, first come, first served.

   If hit on address, invalidate the line in instruction cache (16 bytes)
   Refer to 4KE SUM page 250

   @param address 
*/
FORCE32 static inline void icache_line_i(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Hit_Invalidate_I) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/**
   Fetch and lock by address.

   @param address       
*/
FORCE32 static inline void icache_line_fl(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Fetch_Lock_I) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/**
   Fetch and lock by address.

   @param address       
*/
FORCE32 static inline void dcache_line_fl(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Fetch_Lock_D) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/**
   If hit on address, invalidate the line in data cache (16 bytes)
   Refer to 4KE SUM page 250

   This operation unlocks locked lines.
   Intentionally, this does not writeback the dirty lines.

   @param address       
*/
FORCE32 static inline void dcache_line_i(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Hit_Invalidate_D) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/**
   If hit on address, writeback the line in data cache (16 bytes)
   Refer to 4KE SUM page 250

   @param address       
*/
FORCE32 static inline void dcache_line_w(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Hit_Writeback_D) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/**
   If hit on address, writeback and invalidate the line in data cache (16 bytes)
   Refer to 4KE SUM page 250

   Some fuzzy people call this `flush'

   @param address       
*/
FORCE32 static inline void dcache_line_wi(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Hit_Writeback_Inv_D) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/**
   Fetch and lock by address.

   @param address       
*/
FORCE32 static inline void scache_line_fl(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Fetch_Lock_S) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/**
   If hit on address, invalidate the line in secondary cache (32 bytes)
   Refer to 24K SUM page 313

   This operation unlocks locked lines.
   Intentionally, this does not writeback the dirty lines.

   @param address       
*/
FORCE32 static inline void scache_line_i(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Hit_Invalidate_S) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/**
   If hit on address, writeback the line in secondary cache (32 bytes)
   Refer to 24K SUM page 313

   @param address       
*/
FORCE32 static inline void scache_line_w(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Hit_Writeback_S) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/**
   If hit on address, writeback and invalidate the line in secondary cache (32 bytes)
   Refer to 24K SUM page 313

   Some fuzzy people call this `flush'

   @param address       
*/
FORCE32 static inline void scache_line_wi(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Hit_Writeback_Inv_S) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/*
  No prefetch for i-cache. We use Fill I
 */
FORCE32 static inline void icache_prefetch_line_load(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "cache " STR(RM_Fill_I) ", 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

FORCE32 static inline void cache_line_synci(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "synci 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

/*
  2007jul17 em seems there is no prefetch ability for i-cache. 
  For meaning of 0 see table 11-16
 */
FORCE32 static inline void dcache_prefetch_line_load(RMuint32 address)
{
	__asm__ __volatile__(ASMPROLOGUE "pref 0, 0(%0)\n" ASMEPILOGUE : : "r" (address) : "memory");
}

FORCE32 static inline void icache_range_fl(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~15;la<address+size;la+=16) icache_line_fl(la);
}

FORCE32 static inline void dcache_range_fl(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~15;la<address+size;la+=16) dcache_line_fl(la);
}

FORCE32 static inline void icache_range_i(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~15;la<address+size;la+=16) icache_line_i(la);
}

FORCE32 static inline void dcache_range_i(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~15;la<address+size;la+=16) dcache_line_i(la);
}

FORCE32 static inline void dcache_range_w(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~15;la<address+size;la+=16) dcache_line_w(la);
}

FORCE32 static inline void dcache_range_wi(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~15;la<address+size;la+=16) dcache_line_wi(la);
}

FORCE32 static inline void scache_range_i(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~31;la<address+size;la+=32) scache_line_i(la);
}

FORCE32 static inline void scache_range_w(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~31;la<address+size;la+=32) scache_line_w(la);
}

FORCE32 static inline void scache_range_wi(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~31;la<address+size;la+=32) scache_line_wi(la);
}

FORCE32 static inline void icache_prefetch_range_load(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~15;la<address+size;la+=16) icache_prefetch_line_load(la);
}

FORCE32 static inline void dcache_prefetch_range_load(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~15;la<address+size;la+=16) dcache_prefetch_line_load(la);
}

FORCE32 static inline void cache_range_synci(RMuint32 address,RMuint32 size) 
{
	RMuint32 la;
	
	for (la=address&~15;la<address+size;la+=16) cache_line_synci(la);
}

#undef FORCE32

struct uaRMuint32 {
	RMuint32 value;
} __attribute__((packed));


/***************************************************/
/* Coprocessor2 definitions ************************/
/***************************************************/
/**
   Move 32 bits FROM a coprocessor2 32 bits register

   Example: value = RMmfc2(15,1) to get the value of the coprocessor2
    register 15 in the second register's set.

   Refer to MIPS32 Architecture For Programmers Volume II, rev2.61 page 161

   @param reg: register implementation index
   @param set: register set index
*/
#define RMmfc2(reg,sel) \
   ({ \
    RMuint32 __hidden_rv;                        \
    __asm__ __volatile__ (ASMPROLOGUE "mfc2 %0, " STR(reg) ", " #sel "\n" ASMEPILOGUE \
        : "=r" (__hidden_rv));            \
    __hidden_rv;                            \
    })

/**
   Move 32 bits GPR stored value TO a coprocessor2 32 bits register

   Example: RMmtc2(15,1,17) to move the value 17 to the coprocessor2
    register 15 in the second register's set.

   Refer to MIPS32 Architecture For Programmers Volume II, rev2.61 page 180

   @param reg: register implementation index
   @param set: register set index
   @param val: immediate 32 bits value to be transferred to the coprocessor2
*/
#define RMmtc2(reg, sel, val) \
   do { \
       __asm__ __volatile__ (ASMPROLOGUE "mtc2 %0, " STR(reg) ", " #sel "\n" ASMEPILOGUE \
               : \
               : "r" ((RMuint32)(val))    \
               : "memory"); \
   } while (0)

/**
   Store 32 bits to memory from coprocessor2 register. This coprocessor2
    register can only be in the first register set (i.e. register 15 means
    register 15 from registers set 0 in the coprocessor2)

   Example: RMswc2(15,a) to move the value to the coprocessor2
    register 15 in the first registers set to the memory operand a.

   Refer to MIPS32 Architecture For Programmers Volume II, rev2.61 page 244

   @param reg: register implementation index
   @param mem: a place in memory address space, just use a variable and let
the compiler make its work
*/
#define RMswc2(reg, mem) \
   do { \
       __asm__ __volatile__ (ASMPROLOGUE "swc2 " STR(reg) ", %0\n" ASMEPILOGUE \
               : "=m" ((RMuint32)(mem))); \
   } while (0)

/**
  Load 32 bits from memory to coprocessor2 register. This coprocessor2
    register can only be in the first register set (i.e. register 15 means
    register 15 from registers set 0 in the coprocessor2)

   Example: RMlwc2(15,a) to move the value of the memory operand a 
to the coprocessor2 register 15 in the first registers set.

   Refer to MIPS32 Architecture For Programmers Volume II, rev2.61 page 150

   @param reg: register implementation index
   @param mem: a place in memory address space, just use a variable and let
the compiler make its work
*/
#define RMlwc2(reg, mem) \
   do { \
       __asm__ __volatile__ (ASMPROLOGUE "lwc2 " STR(reg) ", %0\n" ASMEPILOGUE \
               : \
               : "m" ((RMuint32)(mem))); \
   } while (0)
/***************************************************/
/* Coprocessor2 definitions --- end ****************/
/***************************************************/

#endif /* __ASSEMBLY__ */

#endif /* __RMMIPSGCC_H__ */
