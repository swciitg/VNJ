/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   mips32_utilfunctions.h
  @brief  

  Interface to functions carrying mips32 code.  We're separating the mips32 code out so
  that the rest of the code can compile in mips16 mode.

  @author Yufei Leung
  @date   2010-10-28
*/

#ifndef __MIPS32_UTILFUNCTIONS_H__
#define __MIPS32_UTILFUNCTIONS_H__

void __mips32_sync(void);

void mips32_RMmicrosecondsBusySleep(RMuint32 f_hz, RMuint32 us);

RMuint32 mips32_RMunshiftBits(RMuint32 data, RMuint32 bits, RMuint32 shift);

RMint32 mips32_RMlog2(RMuint32 a);

#endif /* __MIPS32_UTILFUNCTIONS_H__ */
