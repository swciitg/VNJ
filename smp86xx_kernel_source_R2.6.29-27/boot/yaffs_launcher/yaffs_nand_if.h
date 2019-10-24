/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   yaffs_nand_if.h
  @brief  

  NAND flash driver interface layer for YAFFS

  @author Yufei Leung
  @date   2010-10-05
*/

#ifndef __YAFFS_NAND_IF_H__
#define __YAFFS_NAND_IF_H__

#include "yaffs2/yaffs_guts.h"

struct ynandv2_ChipAttrib {
	int chipsel;
};

int ynandv2_GetChipSelect(void);

int ynandv2_GetStartBlock(void);
int ynandv2_GetEndBlock(void);

int ynandv2_EraseBlockInNAND(yaffs_dev_t *dev, int blockNumber);
int ynandv2_WriteChunkToNAND(yaffs_dev_t *dev,int chunkInNAND,const __u8 *data, const yaffs_spare *spare);
int ynandv2_WriteChunkWithTagsToNAND(yaffs_dev_t *dev,int chunkInNAND,const __u8 *data, const yaffs_ext_tags *tags);
int ynandv2_ReadChunkFromNAND(yaffs_dev_t *dev,int chunkInNAND, __u8 *data, yaffs_spare *spare);
int ynandv2_ReadChunkWithTagsFromNAND(yaffs_dev_t *dev,int chunkInNAND, __u8 *data, yaffs_ext_tags *tags);
int ynandv2_InitialiseNAND(yaffs_dev_t *dev);
int ynandv2_MarkNANDBlockBad(struct yaffs_dev_s *dev, int blockNo);
int ynandv2_QueryNANDBlock(struct yaffs_dev_s *dev, int blockNo, yaffs_block_state_t *state, __u32 *seq_number);


#endif /* __YAFFS_NAND_IF_H__ */
