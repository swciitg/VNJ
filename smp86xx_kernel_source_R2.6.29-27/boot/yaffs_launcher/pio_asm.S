#include "includes/rmdef.h"
#include "includes/rmmipsgcc.h"
#include "includes/emhwlib_registers_tango3.h"
	
.set noreorder
.set noat

LEAF(getcount)
	jr	$ra
	mfc0	$v0, RM_C0_COUNT
END(getcount)
	
/* RMuint32 get_remap(struct gbus *pgbus,RMuint32 ga); */
LEAF(get_remap)
	srl	$v0, $a1, 26
	bnez	$v0, 0f
	move	$v0, $a0
	jr	$ra
	ins	$v0, $a1, 0, 26

0:	
	addu	$t2, $a0, 0x14
1:	
	subu	$v0, $a0, 4
5:	
	beq	$v0, $t2, 1b # bbt
	addu	$v0, 4
3:
	ll	$t1, 8($v0)
	bnez	$t1, 5b      # bbt
	move	$t1, $a1
	sc	$t1, 8($v0)  # bb not tested. Must be tested. em61
	beqz	$t1, 5b
	srl	$t0, $v0, 2  # 2lsb of t0 are now: 5 if remap7, .., 0 if remap2

	lw	$zero, 8($v0)
	addu	$t0, 1
	ins	$v0, $t0, 26, 3
	jr	$ra
	ins	$v0, $a1, 0, 26
END(get_remap)
	
/* void release_remap(struct gbus *pgbus,RMuint32 remapped_ga); */
LEAF(release_remap)
	ext	$t0, $a1, 26, 3
	beqz	$t0, 1f
	# ok so t0 is 1, 2, ..6
	sll	$t0, 2
	addu	$t0, $a0
	sw	$zero, 4($t0)
1:	
	jr	$ra
	sync
END(release_remap)


.set at
	
/* RMuint64 RMu64divu32(RMuint64 dividend,RMuint32 *r,RMuint32 divisor); */
LEAF(RMu64divu32_9831_10362)
subu $t6, $a3, 1
#ifdef XBOOT2_SMP8652
move $v1, $a1
move $v0, $a0
beqz $t6, epi
li $a0, 0
#else
bnez $t6, 0f
move $v1, $a1
jr $ra
move $v0, $a0
#endif

0:
srl $t6, $a3, 16
beqz $t6, knuth
#ifdef XBOOT2_SMP8652
move $a0, $v0 # fix #10362
#else
nop		
#endif
	
divu $zero, $a1, $a3
mflo $v1
mfhi $a1

	# 0xffff00000000 * 0x10000 / 0xffffffff triggers this
srl $t2, $a1, 16
beq $t2, $t6, 1f
li $t7, 0xffff

divu $zero, $a1, $t6
mflo $t7

1:
multu $t7, $a3

srl $t2, $a0, 16
ins $t2, $a1, 16, 16
srl $t3, $a1, 16

mfhi $t5
mflo $t4
 
bgtu $t5, $t3, 1b
subu $t7, 1
bltu $t5, $t3, 2f
nop
bgtu $t4, $t2, 1b
nop

2:
addu $t7, 1
sll $t2, $t7, 16
mthi $a1
mtlo $a0
msubu $a3, $t2
mfhi $a1
mflo $a0

# x<0? Knuth 4.3.1 D6. Proba extremely low
bgez $a1, 3f
lui $t2, 1
maddu $a3, $t2
mfhi $a1
mflo $a0
		
3:	
and $t2, $a1, 0xffff # $a1 is on 16bit now. could be a move
beq $t2, $t6, 1f
li $v0, 0xffff

srl $t2, $a0, 16 
ins $t2, $a1, 16, 16
divu $zero, $t2, $t6
mflo $v0

1:
multu $v0, $a3

# we will use directly $a1$a0 instead of $t3$t2 because $a1 is now on 16bit

mfhi $t5
mflo $t4
 
bgtu $t5, $a1, 1b
subu $v0, 1
bltu $t5, $a1, 2f
nop
bgtu $t4, $a0, 1b
nop

2:
addu $v0, 1
mthi $a1
mtlo $a0
msubu $a3, $v0
mfhi $a1
mflo $a0

# x<0?
bgez $a1, 3f
li $t2, 1
maddu $a3, $t2
# mfhi $a1 --- we don't need it
mflo $a0 
	
3:	
b epi
ins $v0, $t7, 16, 16

knuth:

srl $t5, $a1, 16		
divu $zero, $t5, $a3
srl $t3, $a0, 16
mflo $t6
mfhi $t7	

ins $a1, $t7, 16, 16
divu $zero, $a1, $a3
mflo $v1
mfhi $t7

ins $t3, $t7, 16, 16
divu $zero, $t3, $a3
ins $v1, $t6, 16, 16
mflo $t6
mfhi $t7
	
ins $a0, $t7, 16, 16
divu $zero, $a0, $a3
mflo $v0
mfhi $a0 

b epi
ins $v0, $t6, 16, 16	

epi:

beqz $a2, 4f
nop
sw $a0, 0($a2)	
4:	
jr $ra
nop
END(RMu64divu32_9831_10362)

.set noat