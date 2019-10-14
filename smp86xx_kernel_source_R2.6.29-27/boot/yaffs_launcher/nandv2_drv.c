/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   nandv2_drv.c
  @brief  

  Low-level NANDv2 driver interface

  @author Yufei Leung
  @date   2010-10-06
*/

#ifndef __NANDV2_DRV_C__
#define __NANDV2_DRV_C__

#include "nand_drv.h"
#include "yaffs_launcher.h"


#ifdef YAFFS_LAUNCHER_COMPILEMODE_MIPS16
#include "mips32_utilfunctions.h"
#undef  RMunshiftBits
#undef  RMmicrosecondsBusySleep
#define RMunshiftBits(data,bits,shift)     mips32_RMunshiftBits((data), (bits), (shift))
#define RMmicrosecondsBusySleep(f_hz, us)  mips32_RMmicrosecondsBusySleep((f_hz), (us))
#else
#define __mips32_sync()  __asm__ __volatile__("sync");
#endif


#define NAND_PAGESIZE_SHIFT   11
#define NAND_PAGESIZE         (1 << NAND_PAGESIZE_SHIFT)
#define NAND_BLOCKSIZE_SHIFT  17
#define NAND_BLOCKSIZE        (1 << NAND_BLOCKSIZE_SHIFT)
#define NAND_NUM_CHIPSEL       2

#define SPNAND_PAGESIZE_SHIFT  9
#define SPNAND_PAGESIZE       (1 << SPNAND_PAGESIZE_SHIFT)
#define SPNAND_BLOCKSIZE_SHIFT  14
#define SPNAND_BLOCKSIZE        (1 << SPNAND_BLOCKSIZE_SHIFT)
#define SPNAND_NUM_CHIPSEL     2

#define NAND_SPAREAREA_SIZE   64
#define SPNAND_SPAREAREA_SIZE 16

#define PB_ECC_CTRL_CLEAR 0x80000008  // Bits[2:0] select the chip select that it is connected to.

#define NAND_PACKETSIZE  512


static struct gbus *glopgbus = (void *)KSEG1ADDR(REG_BASE_cpu_block + CPU_remap);

static RMuint32 nanddrv_f_cpu_hz;

struct unaligned_RMuint32
{
	RMuint32 val __attribute__((packed));
};

static void nanddrv_phyread(RMbool isLargePage, RMuint8 *buf, RMuint32 chip_sel,
			    RMuint32 nfpageno, RMbool readSpareArea)
{
	/* 
	   One regular page = 4096 bytes.
	   One page of large page nfla = 2048 bytes.
	   One page of small page nfla = 512 bytes.

	   However, here we have to specify one power-of-two larger.
	   For large page, 12, not 11.
	   For small page, 10, not 9.
	   that's the trick
	*/
	RMuint32 addr;
	RMuint32 i;
	RMuint32 length;
	volatile char *there = (void *)0xa0020000 + (0x200 * chip_sel);

	if (isLargePage)
		addr = nfpageno << /*DONT CHANGE THE TWELVE */ 12;
	else
		addr = nfpageno << /*DONT CHANGE THE NINE */ 9;

	if (readSpareArea) {
		addr += isLargePage ? NAND_PAGESIZE : 0;
		there[0] = isLargePage ? 0x00 : 0x50; // read/read-spare-area command, depending on NAND flash type
	}
	else {  // We're reading the actual data, we want the ECC to be computed, so clear ECC registers
		gbus_write_uint32(glopgbus, REG_BASE_host_interface + PB_ECC_clear, PB_ECC_CTRL_CLEAR | chip_sel);
		there[0]=0; // read command
	}

	there[4]=RMunshiftBits(addr,8,0);

	if (isLargePage) {
		there[4]=RMunshiftBits(addr,4,8);
		there[4]=RMunshiftBits(addr,8,12);
		there[4]=RMunshiftBits(addr,8,20);
		// that may vary with another ST flash [this is for ST 08GW3B2]
		there[4]=RMunshiftBits(addr,3,28); 
		there[0]=0x30;
	}
	else {
		there[4]=RMunshiftBits(addr,8,9);
		there[4]=RMunshiftBits(addr,8,17);
		there[4]=RMunshiftBits(addr,1,25);
	}

	__mips32_sync();

	/* Poll the status for IO Ready, waiting a max of 27us */
	for (i = 0; i < 27; i++) {
		RMmicrosecondsBusySleep(nanddrv_f_cpu_hz, 1);
		if (gbus_read_uint32_ireg(glopgbus, REG_BASE_host_interface + PB_CS_ctrl) & 0x80000000)
			break;
	}
	
	length = readSpareArea ? NAND_SPAREAREA_SIZE : NAND_PAGESIZE;
	if (!isLargePage)
		length = length / 4;

	for (i = 0; i < length; i++) 
		buf[i]=there[8];
}


static inline RMuint32 get_unaligned_RMuint32(RMuint8 *ptr)
{
	struct unaligned_RMuint32 *s = (struct unaligned_RMuint32 *)ptr;
	return s->val;
}


static RMuint8 mu_count_bits(RMuint32 v)
{
	RMuint8 i, count;
	
	for (count = i = 0; (i < 32) && (v != 0); i++, v >>= 1)
		count += (v & 1);
	return count;
}


static RMstatus nanddrv_correct_ecc(RMuint32 mem, RMuint32 reg, RMuint8 *buf)
{
	RMuint16 pe, po;

	// Must re-arrange the bits for the formula to work properly
	RMuint32 ecc_mem = (mem & 0x00000fff) | ((mem & 0x00fff000) << 4);
	RMuint32 ecc_reg = (reg & 0x00000fff) | ((reg & 0x00fff000) << 4);

	ecc_mem = ecc_mem ^ ecc_reg;

	switch (mu_count_bits(ecc_mem)) {
	case 0: // shall never get there since ecc equal, no call in
		return RM_OK;
	case 1: // dont correct, it is an error in the ECC itself
		return RM_OK;
	case 12: // Potentially a correctable error
		po = (RMuint16)(ecc_mem & 0x0000ffff);
		pe = (RMuint16)((ecc_mem >> 16) & 0x0000ffff);
		po = (pe ^ po);
		if (0x0fff == po) { // Single bit correctable error.  Fix the data and return OK
			buf[pe >> 3] ^= (1 << (pe & 7));
			return RM_OK;
		}
		else	// Uncorrectable error
			return RM_ERROR;
	default:
		return RM_ERROR;
	}
}


/*
  Check if ECC in spare area matches the ECC in the registers.

  For Large Page NAND, that starts from spare_area[10], each group of 3 bytes corresponding to an ECC code register:
      spare_area[10:12] <=> PBx_ECC0_CODE0
      spare_area[13:15] <=> PBx_ECC0_CODE1
      spare_area[16:18] <=> PBx_ECC0_CODE2
      spare_area[19:21] <=> PBx_ECC0_CODE3
  For Small Page NAND, that starts from spare_area[10], one group of 3 bytes corresponding to one ECC code register:
      spare_area[10:12] <=> PBx_ECC0_CODE0
 */
static int nanddrv_check_ecc(RMbool isLargePage, RMuint8 *spare_area, RMuint8 *buf)
{
	RMuint32 i;
	RMuint8 *ecc_oob;
	RMbool ecc_correction_occurred = FALSE;

	ecc_oob = &(spare_area[10]);
	for (i = 0; i < (isLargePage ? 4 : 1); i++) {
		RMuint32 mem, reg;
		mem = get_unaligned_RMuint32(&(ecc_oob[i * 3])) & 0x00ffffff;
		reg = gbus_read_uint32(glopgbus, REG_BASE_host_interface + PB_ECC_code0 + (i * 4)) & 0x00ffffff;
		if (mem != reg) {
			/* If it's all 0xff's, it's probably a bad/empty block, no ECC stored, so just return, don't
			   attempt to correct, you might end up screwing up the data. */
			if (mem == 0x00ffffff) 
				return NANDDRV_STS_FATAL;
			if (nanddrv_correct_ecc(mem, reg, buf + (i * NAND_PACKETSIZE)) != RM_OK)
				return NANDDRV_STS_FATAL;
			else
				ecc_correction_occurred = TRUE;
		}
	}

	return ecc_correction_occurred ? NANDDRV_STS_ECC_CORRECTABLE : NANDDRV_STS_OK;
}


int nanddrv_readpage_ecc(int chip_sel, int page_num, unsigned char *databuf, unsigned char *oobbuf)
{
	/* <HACK> For now, hardcoded to always treat it like Large Page NAND */
	// Read the spare area first, we'll check ECC later
	nanddrv_phyread(TRUE, oobbuf, chip_sel, page_num, TRUE);
	// Read the data
	nanddrv_phyread(TRUE, databuf, chip_sel, page_num, FALSE);
	
	/* Now compare ECC against the ECC HW registers. */
	return nanddrv_check_ecc(TRUE, oobbuf, databuf);
}

int nanddrv_readoob(int chip_sel, int page_num, unsigned char *oobbuf)
{
	nanddrv_phyread(TRUE, oobbuf, chip_sel, page_num, TRUE);

	return NANDDRV_STS_OK;
}

int nanddrv_isbadblock(int chip_sel, int block_num, int pgs_per_blk)
{
	RMuint8 sparebuf[NAND_SPAREAREA_SIZE];

	nanddrv_phyread(TRUE, sparebuf, chip_sel, block_num * pgs_per_blk, TRUE);

	if (sparebuf[0] != 0xff)
		return NANDDRV_STS_FATAL;

	return NANDDRV_STS_OK;
}

int nanddrv_init(int chip_sel)
{
	RMuint32 f_sys_hz;
	RMuint32 mux = gbus_read_uint32(glopgbus,REG_BASE_system_block+SYS_sysclk_mux);
	RMuint32 ratio = RMunshiftBits(mux,4,8);
	
	/* from system block spec 1.5.3 */
	RMuint8 costt[12]={ // cpufreq/sysfreq times 2. 
		2,4,3, 2,2,3, 2,3,2, 4,2,4
	};
	
	// don't muck around to guess the sysclk. xos2 is maintaining it there:
	f_sys_hz = gbus_read_uint32(glopgbus,REG_BASE_cpu_block+LR_XOS_F_SYS_HZ);
	nanddrv_f_cpu_hz = f_sys_hz*costt[ratio]/2;

	return NANDDRV_STS_OK;
}

#endif /* __NANDV2_DRV_C__ */
