/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   rmcompilerid.h

  @author Emmanuel Michon
  @date   2002-11-04
*/

#ifndef __RMCOMPILERID_H__
#define __RMCOMPILERID_H__

#if (RMCOMPILERID==RMCOMPILERID_VISUALC)

typedef unsigned int RMnewOperatorSize;
#include "rmvisual.h"

#elif (RMCOMPILERID==RMCOMPILERID_ARMELF_GCC)

typedef unsigned long RMnewOperatorSize;
#include "rmgcc.h"

#else

typedef unsigned int RMnewOperatorSize;
#include "rmgcc.h"

#endif // RMCOMPILERID==...

#endif // __RMCOMPILERID_H__
