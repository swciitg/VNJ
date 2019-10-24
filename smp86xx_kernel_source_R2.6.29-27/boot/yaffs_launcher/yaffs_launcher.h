/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   yaffs_launcher.h
  @brief  

  Centralized place to include common headers

  @author Yufei Leung
  @date   2010-10-29
*/

#ifndef __YAFFS_LAUNCHER_H__
#define __YAFFS_LAUNCHER_H__

/* Cache operation definitions and functions */
#define CRO_Hit_Writeback_Inv_D (28+10*8)
#define CRO_Hit_Writeback_Inv_S (28+11*8)

/* Used by MLC HW NAND controller driver, to determine which processor it runs on.  Here we
   hardcode to indicate that we run on the CPU */
#define GBUS_MUTEX__PU GBUS_MUTEX_CPU

#define ALLOW_OS_CODE 1

#include "includes/rmdef.h"
#include "includes/rmmipsgcc.h"

#include "includes/emhwlib_registers.h"
#include "includes/emhwlib_lram.h"
#include "includes/uart.h"

#include "includes/addrspace.h"
#include "includes/util.h"
#include "includes/gbus.h"

#include "includes/pio.h"

#endif /* __YAFFS_LAUNCHER_H__ */
