/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   memcpy.h
  @brief  

  @author Yufei Leung
  @date   2009-06-16
*/

#ifndef __MEMCPY_H__
#define __MEMCPY_H__

void mips_memcopy_std(int *p_src, int *p_dst, int loopCnt);
void mips_memcopy_std_ua(int *p_uasrc, int *p_dst, int loopCnt);

#endif /* __MEMCPY_H__ */
