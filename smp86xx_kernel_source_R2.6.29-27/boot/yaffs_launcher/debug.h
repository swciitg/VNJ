/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   debug.h
  @ingroup application
  @brief  some debug macros

  @author David Bryson
  @date   02-23-2006
*/


#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG

#define DEBUG_STRING "%s line %d: "

#define DEBUG_LOG(pri,fmt, ...) \
		uart_printf(DEBUG_STRING fmt, __FILE__, __LINE__, ##__VA_ARGS__ );

#else
#define DEBUG_LOG(pri,fmt, ...)\
		uart_printf("");

#endif /* USE_DEBUG */

#endif
