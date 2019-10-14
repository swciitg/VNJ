/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   mips32_utilfunctions.c
  @brief  

  Some implementations of functions with mips32-only code.

  @author Yufei Leung
  @date   2010-10-28
*/

#ifndef __MIPS32_UTILFUNCTIONS_C__
#define __MIPS32_UTILFUNCTIONS_C__

#include "yaffs_launcher.h"

void mips32_RMmicrosecondsBusySleep(RMuint32 f_hz, RMuint32 us)
{
	RMmicrosecondsBusySleep(f_hz, us);
}

RMuint32 mips32_RMunshiftBits(RMuint32 data, RMuint32 bits, RMuint32 shift)
{
	return RMunshiftBitsVar(data, bits, shift);
}

RMint32 mips32_RMlog2(RMuint32 a)
{
	return RMlog2(a);
}

#endif /* __MIPS32_UTILFUNCTIONS_C__ */
