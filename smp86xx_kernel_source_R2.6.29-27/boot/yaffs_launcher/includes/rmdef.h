/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   rmdef.h
  @brief  

  Refer to design presentation and documentation in
  http://bugs.soft.sdesigns.com/twiki/bin/view/Main/WhatIsThePurposeOfRmdef

  @author Emmanuel Michon
  @date   2002-11-04
*/

/* Note:
 * ----
 * This header file is NOT guarded against multiple inclusions. This is because
 * 'rmforbid.h', which is also not guarded against multiple inclusions and 
 * depends on that behaviour is included exclusively through this header.
 *
 * The fact that this header is unguarded is not without consequences: 
 *  - only preprocessor directives are allowed. Function prototypes, struct
 *    definitions, variable declarations, and in general anything related to
 *    the C or C++ languages is not allowed.
 *  - anything that is #define'd in here must previously be #undef'd in order
 *    to avoid redefinition complaints.
 */
#include "rmem86xxid.h"
/* from the RMPLATFORM value, we set RMCOMPILERID, RMARCHID, RM_MACHINEALIGNMENT and RMENDIANNESS */
#include "rmplatform.h"

#include "rmfeatures.h"

#ifndef __ASSEMBLY__

/* Some decisions are taken after RMARCHID value: */
#include "rmarchid.h"
/* Some decisions are taken after RMCOMPILERID value: */
#include "rmcompilerid.h"
/* hopefully this is the same on all the platforms we have so far: */
#include "rmbtypes.h"
#include "rmforbid.h"
#include "rmstatus.h"
#include "rmpanic.h"
#include "rmmacros.h"
#include "rmstreamid.h"
#include "rmdbg.h"
#include "rmmm.h"
#include "rmsync.h"
#include "rmpacketcommandops.h"
#include "rmnonasciitype.h"

/* inserting this macro in the code makes it not compilable */
#undef NOTCOMPILABLE
#define NOTCOMPILABLE }}}}}}not}}}}}compilable}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

#endif /* __ASSEMBLY__ */

