
/************************************************************************
 *
 *  start.S
 *
 *  Startup code for ZBOOT applet
 *
 ************************************************************************/

#include "includes/rmdef.h"
#include "includes/rmmipsgcc.h"
#include "includes/emhwlib_registers_tango3.h"

#define CRO_Index_Writeback_Inv_D (28+19*8)
#define CRO_Index_Writeback_Inv_S (28+2*8)
#define CRO_Index_Invalidate_I (28+0*8)

#define RESERVED_STACKSIZE      32

.set noreorder
.set noat

/************************************************************************
 *
 *                          __start
 *  Description :
 *  -------------
 *
 *  Entry point for zboot applet
 *
 *  Return values :
 *  ---------------
 *
 *  return value from applet_entry()
 *
 ************************************************************************/
LEAF(__start)

1:
	/* Setup the stack in DRAM */
	la	$t0,_end_stack
	addiu	$t1,$t0,-RESERVED_STACKSIZE

	move	$s0,$t1

	/* Save content -- to be restored later */
	sw	$sp,0($t1)
	sw	$ra,4($t1)
	sw	$a0,8($t1)
	sw	$a1,12($t1)
	sw	$a2,16($t1)
	sw	$a3,20($t1)
	move	$sp,$t1

	/* Clear BSS */
	la	$t0,_edata
	la	$t1,_end_stack
	addiu   $t1,$t1,-RESERVED_STACKSIZE
2:	beq	$t0,$t1,3f
	nop

	sb	$0,0($t0)
	j	2b
	addiu	$t0,$t0,1

3:
	li	$t0,REG_BASE_cpu_block+CPU_UART1_base
	bne	$t0,$a0,4f
	nop

	la	$t1,cpu_uart_base
	sw	$a0,0($t1)

4:
	lw	$a0, 8($s0)
	lw	$a1, 12($s0)
	lw	$a2, 16($s0)
	lw	$a3, 20($s0)

	/* Jump to C entry point for applet_entry(int,int,int) */
	la	$t0,applet_entry
	jalr	$t0
	nop

5:
	/* Restore the stack */
	la	$t0,_end_stack
	addiu	$t1,$t0,-RESERVED_STACKSIZE
	lw	$sp,0($t1)
	lw	$ra,4($t1)
	lw	$a0,8($t1)
	lw	$a1,12($t1)
	lw	$a2,16($t1)
	lw	$a3,20($t1)

6:
	jr	$ra
	nop
	
END(__start)

/* equivalent of l2geo() function for XBOOT3 applet framework, for use by has_l2cache() function below. */
LEAF(afw_l1geo) # clobbers a0 v0 v1 t0
	ext	$v1, $a0, 0, 3 # a
	ext	$t0, $a0, 3, 3 # l
	ext	$a0, $a0, 6, 3 # s
1:
	beqz	$t0, 2f
	li	$v0, 0

	addu	$v1, 1
	sll	$v0, $v1, 7
	sll	$v0, $t0
2:	
	jr	$ra
	sll	$v0, $a0
END(afw_l1geo)

LEAF(afw_l2geo)
	ext	$v1, $a0, 0, 4 # a
	ext	$t0, $a0, 4, 4 # l
	b	1b
	ext	$a0, $a0, 8, 4 # s
END(afw_l2geo)

LEAF(l1geo) # clobbers a0 v0 v1 t0
	ext	$v1, $a0, 0, 3 # a
	ext	$t0, $a0, 3, 3 # l
	ext	$a0, $a0, 6, 3 # s
1:
	beqz	$t0, 2f
	li	$v0, 0

	addu	$v1, 1
	sll	$v0, $v1, 7
	sll	$v0, $t0
2:	
	jr	$ra
	sll	$v0, $a0
END(l1geo)

LEAF(l2geo)
	ext	$v1, $a0, 0, 4 # a
	ext	$t0, $a0, 4, 4 # l
	b	1b
	ext	$a0, $a0, 8, 4 # s
END(l2geo)

/* RMuint32 icache_i_(void); */
LEAF(icache_i_)
	move	$t3, $ra

	mfc0	$a0, RM_C0_CONFIG1
	bal	l1geo
	srl	$a0, 16

	beqz	$v0, 10f # no i-cache?
	li	$a0, 0x80000000
	move	$a1, $v0
	bal	cache_range_op
	li	$a2, CRO_Index_Invalidate_I

10:	
	move	$ra, $t3
	jr	$ra
	nop
END(icache_i_)

/* RMuint32 dcache_wi_(void); */
LEAF(dcache_wi_)
	move	$t3, $ra

	mfc0	$a0, RM_C0_CONFIG1
	bal	l1geo
	srl	$a0, 7

	beqz	$v0, 10f # no d-cache?
	li	$a0, 0x80000000
	move	$a1, $v0
	bal	cache_range_op
	li	$a2, CRO_Index_Writeback_Inv_D

10:	
	move	$ra, $t3
	jr	$ra
	nop
END(dcache_wi_)

/* RMuint32 l2cache_wi_(void); */
LEAF(l2cache_wi_)
	move	$t3, $ra

	bal	l2geo
	mfc0	$a0, RM_C0_CONFIG2

	beqz	$v0, 10f # no l2 cache?
	li	$a0, 0x80000000
	move	$a1, $v0
	bal	cache_range_op
	li	$a2, CRO_Index_Writeback_Inv_S

10:	
	move	$ra, $t3
	jr	$ra
	nop
END(l2cache_wi_)

/* void cache_range_op(void *p,RMuint32 size,RMuint32 op) */
LEAF(cache_range_op)
	move	$t2, $ra
	
	bal	1f
	addu	$a2, $ra
1:
	rdhwr	$t0, $1
	
	# compute rmaligntonext(p+size,log2ls)
	addu	$a1, $a0
	addu	$a1, $t0
	subu	$a1, 1
	neg	$t1, $t0
	j	$a2
	and	$a1, $t1

	b	2f  
	cache	RM_Index_Invalidate_I, 0($a0)
	b	2f
	pref 30, 0($a0)
	b	2f
	cache	RM_Index_Writeback_Inv_S, 0($a0)

	b	2f
	cache	RM_Index_Store_Tag_I, 0($a0)
	b	2f
	cache	RM_Index_Store_Tag_D, 0($a0)
	b	2f
	cache	RM_Index_Store_Tag_S, 0($a0)

	b	2f
	cache	RM_Hit_Invalidate_I, 0($a0)
	b	2f
	cache	RM_Hit_Invalidate_D, 0($a0)
	b	2f	
	cache	RM_Hit_Invalidate_S, 0($a0)
	
	b	2f
	cache	RM_Fill_I, 0($a0)
	b	2f
	cache	RM_Hit_Writeback_Inv_D, 0($a0)
	b	2f
	cache	RM_Hit_Writeback_Inv_S, 0($a0)
	
	b	2f
	cache	RM_Hit_Writeback_D, 0($a0)
	b	2f
	cache	RM_Hit_Writeback_S, 0($a0)
	
	b	2f
	cache	RM_Fetch_Lock_I, 0($a0)
	b	2f
	cache	RM_Fetch_Lock_D, 0($a0)
	b	2f
	cache	RM_Fetch_Lock_S, 0($a0)

	b	2f	
	synci	0($a0)
	b	2f
	pref 0, 0($a0)

	# let the most common one run bit faster, the last in list doesn't have to jump
	cache	RM_Index_Writeback_Inv_D, 0($a0)
	
2:
	addu	$a0, $t0
	sltu	$t1, $a0, $a1
	movz	$a2, $t2, $t1 # if end of loop restore $ra and get out.
	
	j	$a2
	nop
END(cache_range_op)

LEAF(has_l2cache)
	b	afw_l2geo
	mfc0	$a0, RM_C0_CONFIG2
END(has_l2cache)

LEAF(__sync)
	sync
	jr	$ra
	nop
END(__sync)
