#include "includes/rmdef.h"
#include "includes/rmmipsgcc.h"
#include "includes/emhwlib_registers_tango3.h"
	
.set noreorder
.set noat

/*
	These are not leaves because we call get_remap and release_remap inside.
	
	We do an assumption on their implementation: do not change a0 a1 a2 
	and v0 (get_remap). Do not use t3.	
*/
	
/* RMuint8  gbus_read_uint8 (struct gbus *h, RMuint32 byte_address) */
LEAF(gbus_read_uint8)
	move $t3, $ra
	jal get_remap
	nop
	
	move $a1, $v0
	jal release_remap
	lbu $v0, 0($a1)
		
	move $ra, $t3
	jr $ra
	nop
END(gbus_read_uint8)

/* RMuint16 gbus_read_uint16(struct gbus *h, RMuint32 byte_address) */
LEAF(gbus_read_uint16)
	move $t3, $ra
	jal get_remap
	nop
	
	move $a1, $v0
	jal release_remap
	lhu $v0, 0($a1)
		
	move $ra, $t3
	jr $ra
	nop
END(gbus_read_uint16)

/* RMuint32 gbus_read_uint32(struct gbus *h, RMuint32 byte_address) */
LEAF(gbus_read_uint32)
	move $t3, $ra
	jal get_remap
	nop
	
	move $a1, $v0
	jal release_remap
	lw $v0, 0($a1)
		
	move $ra, $t3
	jr $ra
	nop
END(gbus_read_uint32)

/* void gbus_write_uint8 (struct gbus *h, RMuint32 byte_address, RMuint8  data) */
LEAF(gbus_write_uint8)	
	move $t3, $ra
	jal get_remap
	nop
	
	move $a1, $v0
	jal release_remap
	sb $a2, 0($a1)
		
	move $ra, $t3
	jr $ra
	nop
END(gbus_write_uint8)	

/* void gbus_write_uint16(struct gbus *h, RMuint32 byte_address, RMuint16 data) */
LEAF(gbus_write_uint16)	
	move $t3, $ra
	jal get_remap
	nop
	
	move $a1, $v0
	jal release_remap
	sh $a2, 0($a1)
		
	move $ra, $t3
	jr $ra
	nop
END(gbus_write_uint16)	

/* void gbus_write_uint32(struct gbus *h, RMuint32 byte_address, RMuint32 data) */
LEAF(gbus_write_uint32)	
	move $t3, $ra
	jal get_remap
	nop	
	
	move $a1, $v0
	jal release_remap
	sw $a2, 0($a1)
		
	move $ra, $t3
	jr $ra
	nop
END(gbus_write_uint32)	
