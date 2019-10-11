/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/*
 * addrspace.h
 *
 * MIPS address space definitions
 */

#ifndef __ADDRSPACE_H__
#define __ADDRSPACE_H__

#ifndef CPHYSADDR
#define CPHYSADDR(x)    ((x) & 0x1fffffff)
#endif

#ifndef KSEG1
#define KSEG1           0xa0000000
#endif

#ifndef KSEG1ADDR
#define KSEG1ADDR(x)    (CPHYSADDR(x) | KSEG1)
#endif

#ifndef KSEG0
#define KSEG0           0x80000000
#endif

#ifndef KSEG0ADDR
#define KSEG0ADDR(x)    (CPHYSADDR(x) | KSEG0)
#endif

#endif /* __ADDRSPACE_H__ */

