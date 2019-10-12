/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   nandv3_drv.c
  @brief  

  Low-level NANDv3 driver interface

  @author Yufei Leung
  @date   2010-11-02
*/

#ifndef __NANDV3_DRV_C__
#define __NANDV3_DRV_C__

#include "nand_drv.h"
#include "yaffs_launcher.h"

#include "includes/mlc.h"
#include "includes/mlc_pa.h"


#ifdef YAFFS_LAUNCHER_COMPILEMODE_MIPS16
#include "mips32_utilfunctions.h"
#undef  RMunshiftBits
#undef  RMmicrosecondsBusySleep
#define RMunshiftBits(data,bits,shift)     mips32_RMunshiftBits((data), (bits), (shift))
#define RMmicrosecondsBusySleep(f_hz, us)  mips32_RMmicrosecondsBusySleep((f_hz), (us))
#else
#define __mips32_sync()  __asm__ __volatile__("sync");
#endif


#define MLC_NAND_NUM_CHIPSEL       2

#define MLC_NAND_PAGESIZE_SHIFT   11
#define MLC_NAND_PAGESIZE         (1 << MLC_NAND_PAGESIZE_SHIFT)
#define MLC_NAND_BLOCKSIZE_SHIFT  17
#define MLC_NAND_BLOCKSIZE        (1 << MLC_NAND_BLOCKSIZE_SHIFT)

#define MLC_XLNAND_PAGESIZE_SHIFT  12
#define MLC_XLNAND_PAGESIZE        (1 << MLC_XLNAND_PAGESIZE_SHIFT)
#define MLC_XLNAND_BLOCKSIZE_SHIFT 19
#define MLC_XLNAND_BLOCKSIZE       (1 << MLC_XLNAND_BLOCKSIZE_SHIFT)

#define MLC_SPNAND_PAGESIZE_SHIFT  9
#define MLC_SPNAND_PAGESIZE        (1 << MLC_SPNAND_PAGESIZE_SHIFT)
#define MLC_SPNAND_BLOCKSIZE_SHIFT 14
#define MLC_SPNAND_BLOCKSIZE       (1 << MLC_SPNAND_BLOCKSIZE_SHIFT)

#define MLC_SCHEME_SPNAND          0
#define MLC_SCHEME_NAND            1
#define MLC_SCHEME_XLNAND          2
#define MLC_SCHEME_XLNAND_EXT      3 /* Extra-Large Page NAND with extended ECC capability */


#define memset_v(ptr, data, length)  memset(ptr, data, length)


static struct gbus *glopgbus = (void *)KSEG1ADDR(REG_BASE_cpu_block + CPU_remap);

static struct mlc_pa_job mlcNand_job;

int nanddrv_isbadblock(int chip_sel, int block_num, int pgs_per_blk)
{
	RMuint8 localPageBuffer[MLC_NAND_PAGESIZE];
	RMuint32 localPageBuffer_ga = kva_to_ga((RMuint32)localPageBuffer);

	if (isbb_simplistic(glopgbus, &mlcNand_job, chip_sel, block_num, localPageBuffer_ga))
		return NANDDRV_STS_FATAL;

	return NANDDRV_STS_OK;
}

int nanddrv_readpage_ecc(int chip_sel, int page_num, unsigned char *databuf, unsigned char *oobbuf)
{
	struct mlc_pa_job *pj = &mlcNand_job;
	RMint32 num_errors;

	pj->ga = kva_to_ga((RMuint32)databuf);
	pj->byteoffset = page_num * MLC_NAND_PAGESIZE; /* <HACK> Hardcoding the page size */
	pj->size = MLC_NAND_PAGESIZE; /* <HACK> Hardcoding the page size */
	pj->vameta = NULL;
	pj->vabb = NULL;
	pj->vareport = NULL;
	pj->crc32=-1; pj->crc32extent=0;

	/* Invalidate cache beforehand, so we can see the data properly afterwards */
	dcache_wi_();

	num_errors = mlc_pa_sync_eccread(glopgbus, pj, GBUS_MUTEX_CPU);
	if ((num_errors == -1) || (num_errors == RM_PARAMETER_OUT_OF_RANGE))
		return NANDDRV_STS_FATAL;

	return (num_errors == 0) ? NANDDRV_STS_OK : NANDDRV_STS_ECC_CORRECTABLE;
}

int nanddrv_init(int chip_sel)
{
	static RMbool isTimingDone = FALSE;
	struct mlc_pa_job *pj = &mlcNand_job;
	struct mlc_cfg *pvch = &pj->vch;
	RMstatus rc;

	RMuint32 l2blocksize;

	/* <HACK> Only supporting Large Page NAND */
	l2blocksize = MLC_NAND_BLOCKSIZE_SHIFT;

	if (!isTimingDone) {
		// CS#0 base + offset for current CS
		volatile char *there=(void *)(0xa0020000);

		// correct for 8654 ES2 @300MHz sysclk or lower. Others: don't know
		*(volatile RMuint32 *)(there+0x834)=0x00330003;
		*(volatile RMuint32 *)(there+0x800)=0x03080202;
		*(volatile RMuint32 *)(there+0x81c)=0x3f3;
		*(volatile RMuint32 *)(0xa002093c) = 0x40; /* <HACK> Fix for 8644.  No documentation on this yet */
		__mips32_sync();
		isTimingDone = TRUE;
	}

	// Setup job
	memset_v((void *)pj, 0, sizeof(struct mlc_pa_job));

	pj->cs = pj->ch = chip_sel;
	pj->l2ps = MLC_NAND_PAGESIZE_SHIFT;	/* <HACK> Hardcoded for Large Page NAND */
	pj->scheme = MLC_SCHEME_NAND; /* Device scheme: spare area, ECC capability.  <HACK> hardcoded for Large Page NAND */
	switch (pj->scheme) {
	case 0:
	case 1:
	case 2:
		pj->spareper128 = 4;
		break;
	case 3:
	case 4:
	default:
		pj->spareper128 = 7;
		break;
	}

	pj->l2blocksize = l2blocksize;
	pj->l2wholesize = 29;	/* <HACK> Hardcoded to 512 MB, should be OK, xmboot doesn't read that far into device */
	/* <TODO> Hardcoded timings.  Are these conservative enough for all devices??? */
	pvch->device_cfg.bits.erase_cycle = 3;
	pvch->device_cfg.bits.tot_cycle = 5;
#ifndef XBOOT2_SMP8646		/* Timings for 333MHz system clock */
	pvch->timing1 = 0x39391d15;
	pvch->timing2 = 0x150e1339;
#else  /* Timings for 400MHz system clock */
	pvch->timing1 = 0x4545231a;
	pvch->timing2 = 0x1a111745;
#endif

	rc = mlc_pa_probe(glopgbus, 333000000, pj);
	if (rc != RM_OK)
		return NANDDRV_STS_FATAL;

	while (mlc_pa_request(glopgbus, pj, GBUS_MUTEX_CPU) == RM_BUSY)
		;
	
	return NANDDRV_STS_OK;
}

#endif /* __NANDV3_DRV_C__ */
