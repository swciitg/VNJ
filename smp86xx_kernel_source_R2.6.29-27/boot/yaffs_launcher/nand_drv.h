/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   nand_drv.h
  @brief  

  Low-level NAND driver interface for yaffs_nand_if.c

  @author Yufei Leung
  @date   2010-10-06
*/

#ifndef __NAND_DRV_H__
#define __NAND_DRV_H__

/*
 * Various error codes for the return values here, such as correctable ECC, uncorrectable ECC, etc.
 */
#define NANDDRV_STS_OK                0
#define NANDDRV_STS_FATAL            -1
#define NANDDRV_STS_ECC_CORRECTABLE   1

#define NANDDRV_PAGESIZE_MAX  2048
#define NANDDRV_OOBSIZE_MAX   64


/// 
/**
   Read a page from NAND, performing ECC checks, and automatically attempting ECC correction if necessary.

   @param chip_sel:      Chip select to read from
   @param page_num:      Page number to read from
   @param databuf:       Pointer to data buffer to hold the page data
   @param oobbuf:        Pointer to buffer to hold the OOB data (spare area data)
   @return NANDDRV_STS_OK on success.
           NANDDRV_STS_ECC_CORRECTABLE if there was a correctable ECC error.
           NANDDRV_STS_FATAL on failure.
*/
int nanddrv_readpage_ecc(int chip_sel, int page_num, unsigned char *databuf, unsigned char *oobbuf);


/// 
/**
   Read spare area from NAND.

   @param chip_sel:      Chip select to read from
   @param page_num:      Page number to read from
   @param oobbuf:        Pointer to buffer to hold the OOB data (spare area data)
   @return NANDDRV_STS_OK on success.
           NANDDRV_STS_FATAL on failure.
*/
int nanddrv_readoob(int chip_sel, int page_num, unsigned char *oobbuf);


/// 
/**
   Checks if the block is good or bad.

   @param chip_sel:      Chip select
   @param block_num:     Block number to check
   @param pgs_per_blk:   Number of pages per block on this device
   @return NANDDRV_STS_OK if the block is good.
           NANDDRV_STS_FATAL if the block is bad.
*/
int nanddrv_isbadblock(int chip_sel, int block_num, int pgs_per_blk);


/// 
/**
   Initialize driver

   @param chip_sel      
   @return 0 on success, -1 on failure
*/
int nanddrv_init(int chip_sel);

#endif /* __NAND_DRV_H__ */
