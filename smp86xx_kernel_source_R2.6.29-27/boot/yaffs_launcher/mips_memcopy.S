/*****************************************
 Copyright © 2005 - 2009
 Sigma Designs, Inc. All Rights Reserved

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *****************************************/

/**
  @file   memcpy.S
  @brief  

	Some better optimized memory copying routines.
	
  @author Emmanuel Michon
  @date   2009-06-16
*/

#include "includes/rmdef.h"
#include "includes/rmmipsgcc.h"
#include "includes/emhwlib_registers_tango3.h"

	
.set noreorder
.set noat

	
LEAF(mips_memcopy_std)
# void mips_memcopy_std(int *p_src, int *p_dst, int loopCnt)
#
# $a0 - p_src, source read pointer A
# $a1 - p_dst, dest write pointer
# $a2 - loopCnt
	pref	30, 0x0($a1)	# start setting up the dest
	sll	$a2, 5
	addu	$a2, $a1

1:
	/*
		those two prefetch are only useful when data is not in d-cache (when `streaming') ; where they bring 30%
		 improvement. when data is in cache already, performance drops from 2.34 ccy/w (libc) to 2.49 ccy/w
		(this code).
		removing the `pref' goes to 2.25 ccy/w (faster than libc).
	*/	
	pref	0, 0x60($a0)	# bring the next lines of src
	pref	30, 0x20($a1)	# start setting up the dest

	lw 	$t0, 0x0($a0)
	lw 	$t1, 0x4($a0)
	lw 	$t2, 0x8($a0)
	lw 	$t3, 0xc($a0)
	lw 	$t4, 0x10($a0)
	lw 	$t5, 0x14($a0)
	lw 	$t6, 0x18($a0)
	lw 	$t7, 0x1c($a0)

	sw 	$t0, 0x0($a1)
	sw 	$t1, 0x4($a1)
	sw 	$t2, 0x8($a1)
	sw 	$t3, 0xc($a1)
	sw 	$t4, 0x10($a1)
	sw 	$t5, 0x14($a1)
	sw 	$t6, 0x18($a1)
	sw 	$t7, 0x1c($a1)

	addu	$a0, 0x20
	bne	$a1, $a2, 1b
	addu	$a1, 0x20

	lw 	$t0, 0x0($a0)
	lw 	$t1, 0x4($a0)
	lw 	$t2, 0x8($a0)
	lw 	$t3, 0xc($a0)
	lw 	$t4, 0x10($a0)
	lw 	$t5, 0x14($a0)
	lw 	$t6, 0x18($a0)
	lw 	$t7, 0x1c($a0)

	sw 	$t0, 0x0($a1)
	sw 	$t1, 0x4($a1)
	sw 	$t2, 0x8($a1)
	sw 	$t3, 0xc($a1)
	sw 	$t4, 0x10($a1)
	sw 	$t5, 0x14($a1)
	sw 	$t6, 0x18($a1)
	
	jr	$ra
	sw 	$t7, 0x1c($a1)
END(mips_memcopy_std)

LEAF(mips_memcopy_std_ua)
# void mips_memcopy_std_ua(int *p_uasrc, int *p_dst, int loopCnt)
#
# $a0 - p_src, source read pointer A
# $a1 - p_dst, dest write pointer
# $a2 - loopCnt
	pref	30, 0x0($a1)	# start setting up the dest
	sll	$a2, 5
	addu	$a2, $a1

1:
	pref	0, 0x60($a0)	# bring the next lines of src
	pref	30, 0x20($a1)	# start setting up the dest

	ulw 	$t0, 0x0($a0)
	ulw 	$t1, 0x4($a0)
	ulw 	$t2, 0x8($a0)
	ulw 	$t3, 0xc($a0)
	ulw 	$t4, 0x10($a0)
	ulw 	$t5, 0x14($a0)
	ulw 	$t6, 0x18($a0)
	ulw 	$t7, 0x1c($a0)

	sw 	$t0, 0x0($a1)
	sw 	$t1, 0x4($a1)
	sw 	$t2, 0x8($a1)
	sw 	$t3, 0xc($a1)
	sw 	$t4, 0x10($a1)
	sw 	$t5, 0x14($a1)
	sw 	$t6, 0x18($a1)
	sw 	$t7, 0x1c($a1)

	addu	$a0, 0x20
	bne	$a1, $a2, 1b
	addu	$a1, 0x20

	ulw 	$t0, 0x0($a0)
	ulw 	$t1, 0x4($a0)
	ulw 	$t2, 0x8($a0)
	ulw 	$t3, 0xc($a0)
	ulw 	$t4, 0x10($a0)
	ulw 	$t5, 0x14($a0)
	ulw 	$t6, 0x18($a0)
	ulw 	$t7, 0x1c($a0)

	sw 	$t0, 0x0($a1)
	sw 	$t1, 0x4($a1)
	sw 	$t2, 0x8($a1)
	sw 	$t3, 0xc($a1)
	sw 	$t4, 0x10($a1)
	sw 	$t5, 0x14($a1)
	sw 	$t6, 0x18($a1)

	jr	$ra
	sw 	$t7, 0x1c($a1)
END(mips_memcopy_std_ua)
