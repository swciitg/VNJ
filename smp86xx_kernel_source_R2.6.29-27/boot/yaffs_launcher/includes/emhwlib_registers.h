/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   emhwlib_registers.h
  @brief  

  long description

  @author Emmanuel Michon
  @date   2004-01-23
*/

#ifndef __EMHWLIB_REGISTERS_H__
#define __EMHWLIB_REGISTERS_H__

#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2)
#include "tango2/emhwlib_registers_tango2.h"
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGO3)
#include "emhwlib_registers_tango3.h"
#else
#error EM86XX_CHIP is not set in RMCFLAGS: refer to rmdef/rmem86xxid.h.
#endif

#endif // __EMHWLIB_REGISTERS_H__
