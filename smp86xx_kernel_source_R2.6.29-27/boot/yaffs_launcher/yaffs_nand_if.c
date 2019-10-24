/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   yaffs_nand_if.c
  @brief  

  Implementation of NAND flash driver interface layer for YAFFS

  @author Yufei Leung
  @date   2010-10-05
*/

#ifndef __YAFFS_NAND_IF_C__
#define __YAFFS_NAND_IF_C__

#include "yaffs_nand_if.h"

#include "nand_drv.h"

#include "yaffs_packedtags2.h"


extern int yaffs_launcher_chip_sel;

extern int yaffs_launcher_start_blk;
extern int yaffs_launcher_end_blk;


int ynandv2_GetChipSelect(void)
{
	if (yaffs_launcher_chip_sel)
		return yaffs_launcher_chip_sel;

	return YAFFS_FAIL;
}

int ynandv2_GetStartBlock(void)
{
	if (yaffs_launcher_start_blk)
		return yaffs_launcher_start_blk;

	return YAFFS_FAIL;
}

int ynandv2_GetEndBlock(void)
{
	if (yaffs_launcher_end_blk)
		return yaffs_launcher_end_blk;

	return YAFFS_FAIL;
}

int ynandv2_EraseBlockInNAND(yaffs_dev_t *dev, int blockNumber)
{
	/* <HACK> Stubbed for now */
	return YAFFS_FAIL;
}

int ynandv2_WriteChunkToNAND(yaffs_dev_t *dev,int chunkInNAND,const __u8 *data, const yaffs_spare *spare)
{
	/* No need to implement this if ynandv2_WriteChunkWithTagsToNAND() is implemented */
	return YAFFS_FAIL;
}

int ynandv2_WriteChunkWithTagsToNAND(yaffs_dev_t *dev,int chunkInNAND,const __u8 *data, const yaffs_ext_tags *tags)
{
	/* <HACK> Stubbed for now */
	return YAFFS_FAIL;
}

int ynandv2_ReadChunkFromNAND(yaffs_dev_t *dev,int chunkInNAND, __u8 *data, yaffs_spare *spare)
{
	/* No need to implement this if ynandv2_ReadChunkWithTagsFromNAND() is implemented */
	return YAFFS_FAIL;
}

int ynandv2_ReadChunkWithTagsFromNAND(yaffs_dev_t *dev,int chunkInNAND, __u8 *data, yaffs_ext_tags *tags)
{
	/* Note that either of data or tags can be NULL!!! */
	int readpage_retcode;
	struct ynandv2_ChipAttrib *chipAttr = (struct ynandv2_ChipAttrib *)dev->driver_context;
	__u8 localDataBuf[NANDDRV_PAGESIZE_MAX];
	__u8 oobBuf[NANDDRV_OOBSIZE_MAX];

	__u8 *buffer = data ? data : localDataBuf;

	readpage_retcode = nanddrv_readpage_ecc(chipAttr->chipsel, chunkInNAND, buffer, oobBuf);

	if (tags) {
		if (dev->param.inband_tags) {
			yaffs_PackedTags2TagsPart *pt2tp;
			pt2tp = (yaffs_PackedTags2TagsPart *)&buffer[dev->data_bytes_per_chunk];
			yaffs_unpack_tags2tags_part(tags, pt2tp);
		}
		else {
			yaffs_PackedTags2 pt;
			memcpy(&pt, oobBuf, sizeof(pt));
			yaffs_unpack_tags2(tags, &pt, !(dev->param.no_tags_ecc));
		}

		/* If the result from nanddrv_readpage_ecc() is worse than the current tags->ecc_result,
		   update tags->ecc_result with that result. */
		switch (readpage_retcode) {
		case NANDDRV_STS_ECC_CORRECTABLE:
			if (tags->ecc_result <= YAFFS_ECC_RESULT_NO_ERROR)
				tags->ecc_result = YAFFS_ECC_RESULT_FIXED;
			break;
		case NANDDRV_STS_FATAL:
			if (tags->ecc_result <= YAFFS_ECC_RESULT_FIXED)
				tags->ecc_result = YAFFS_ECC_RESULT_UNFIXED;
			break;
		default:
			break;
		}
	}

	return (readpage_retcode != NANDDRV_STS_FATAL) ? YAFFS_OK : YAFFS_FAIL;
}

int ynandv2_InitialiseNAND(yaffs_dev_t *dev)
{
	struct ynandv2_ChipAttrib *chipAttr = (struct ynandv2_ChipAttrib *)dev->driver_context;
	/* <HACK> This should probably do more work, but for now this will do. */
	if (nanddrv_init(chipAttr->chipsel) != NANDDRV_STS_OK)
		return YAFFS_FAIL;
	return YAFFS_OK;
}

int ynandv2_MarkNANDBlockBad(struct yaffs_dev_s *dev, int blockNo)
{
	/* <HACK> Stubbed for now */
	return YAFFS_FAIL;
}

int ynandv2_QueryNANDBlock(struct yaffs_dev_s *dev, int blockNo, yaffs_block_state_t *state, __u32 *seq_number)
{

	struct ynandv2_ChipAttrib *chipAttr = (struct ynandv2_ChipAttrib *)dev->driver_context;

	if (nanddrv_isbadblock(chipAttr->chipsel, blockNo, dev->param.chunks_per_block) == NANDDRV_STS_FATAL) {
		*state = YAFFS_BLOCK_STATE_DEAD;
		*seq_number = 0;
		return YAFFS_FAIL;
	}
	
	{
		int chunkInNAND = blockNo * dev->param.chunks_per_block;
		yaffs_ext_tags tags;

		/* Read it out, ignoring return code, because it could fail if block is empty, but that's
		   not relevant to what we want to check. */
		ynandv2_ReadChunkWithTagsFromNAND(dev, chunkInNAND, NULL, &tags);
		if (tags.chunk_used) {
			*seq_number = tags.seq_number;
			*state = YAFFS_BLOCK_STATE_NEEDS_SCANNING;
		}
		else {
			*seq_number = 0;
			*state = YAFFS_BLOCK_STATE_EMPTY;
		}
	}

	return YAFFS_OK;
}


#endif /* __YAFFS_NAND_IF_C__ */
