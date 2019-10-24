/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

#include "includes/rmem86xxid.h"

#include "includes/emhwlib_registers_tango3.h"
#include "includes/emhwlib_lram.h"

unsigned long cpu_uart_base = REG_BASE_cpu_block + CPU_UART0_base; /* default */

