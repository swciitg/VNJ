/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   zboot_yaffs_params.h
  @brief  

  Interface to pass parameters for the YAFFS applet

  @author Yufei Leung
  @date   2010-10-27
*/

#ifndef __ZBOOT_YAFFS_PARAMS_H__
#define __ZBOOT_YAFFS_PARAMS_H__

#define LOG2_ZBOOT_YAFFS_FLAGS_GZIP    0
#define LOG2_ZBOOT_YAFFS_FLAGS_ENCRYPT 1
#define LOG2_ZBOOT_YAFFS_FLAGS_EXEC    2

#define ZBOOT_YAFFS_FLAGS_GZIP    (1 << LOG2_ZBOOT_YAFFS_FLAGS_GZIP)
#define ZBOOT_YAFFS_FLAGS_ENCRYPT (1 << LOG2_ZBOOT_YAFFS_FLAGS_ENCRYPT)
#define ZBOOT_YAFFS_FLAGS_EXEC    (1 << LOG2_ZBOOT_YAFFS_FLAGS_EXEC)

/* Only defined/used if ezboot version >= 0xba */
#define LOG2_ZBOOT_YAFFS_FLAGS_CONT_VALID    31
#define LOG2_ZBOOT_YAFFS_FLAGS_CONT_NOTLAST  30
#define LOG2_ZBOOT_YAFFS_FLAGS_CONT_IDX      16
#define ZBOOT_YAFFS_FLAGS_CONT_IDX_BITS      14
#define ZBOOT_YAFFS_FLAGS_CONT_VALID    (1 << LOG2_ZBOOT_YAFFS_FLAGS_CONT_VALID)
#define ZBOOT_YAFFS_FLAGS_CONT_NOTLAST  (1 << LOG2_ZBOOT_YAFFS_FLAGS_CONT_NOTLAST)
#define ZBOOT_YAFFS_FLAGS_CONT_IDX      (((1 << ZBOOT_YAFFS_FLAGS_CONT_IDX_BITS) - 1) << LOG2_ZBOOT_YAFFS_FLAGS_CONT_IDX)
/* ... only for ezboot version >= 0xba */

struct zboot_yaffs_params {
	RMuint32 chip_sel;
	RMuint32 start_block;
	RMuint32 end_block;
	RMascii path_name[256];
	RMuint32 flags;		/* For ezboot version >= 0xba, bit31 is the valid bit, indicating that this parameter entry is valid.
				   bit30 indicates this is NOT the last entry, and the YAFFS applet may look further for more entries.
				   if bit30 is NOT set, then this is indeed the final entry, so look no further.
				   bits[30:16] are the container index of this entry.
				   The remainder of the bits are flags as defined above.
				 */
	RMuint32 load_addr;
	RMuint32 start_addr;
};

#endif /* __ZBOOT_YAFFS_PARAMS_H__ */
