#include "includes/rmdef.h"
#include "includes/rmmipsgcc.h"
	
.set noreorder

	
LEAF(__mips32_sync)
	sync
	jr	$ra
	nop
END(__mips32_sync)
