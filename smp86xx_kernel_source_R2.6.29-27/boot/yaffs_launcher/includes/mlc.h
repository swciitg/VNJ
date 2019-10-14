/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   mlc.h
  @brief  

  -long description-

  @author Emmanuel Michon
  @date   2009-08-27
*/

#ifndef __MLC_H__
#define __MLC_H__

#define BFLOG(x...) do {} while (0)
#define BFPROTO
#define BFCLASS extern inline __attribute__((nomips16))

#include "mlcstatus.h"
#include "mlcdevicecfg.h"
#include "mlcxfercfg.h"
#include "pbspictrl.h"

struct mlc_cfg {
	union MLCSTATUSReg status_reg;
	RMuint32 flash_cmd;
	union MLCDEVICECFGReg device_cfg;
	RMuint32 timing1,timing2;
	union MLCXFERCFGReg xfer_cfg;
	RMuint32 packet_0_cfg,packet_n_cfg,bad_block_cfg,add_page,add_offset;
};

struct mif {
	RMuint32 add,cnt,skip,cmd;
	RMuint32 pad[48/4];
};
	
/* i can't believe that's the first time it's used */
#define host_CRC32_spy_crc32 0x9800
#define host_CRC32_spy_count 0x9804
#define host_CRC32_spy_conf  0x9808

#define MLC_CHA_REG 0x4400
#define MLC_CHB_REG 0x4440
#define MLC_CHA_MEM 0x4800
#define MLC_CHB_MEM 0x4a00
#define MLC_MEM_SZ 0x200
#define MLC_BADBLOCK_OFFSET 0x100
#define MLC_ECCREPORT_OFFSET 0x1c0
#define CHAREG_GA ((struct mlc_cfg *)(REG_BASE_host_interface+MLC_CHA_REG))
#define CHBREG_GA ((struct mlc_cfg *)(REG_BASE_host_interface+MLC_CHB_REG))
#define NAND_CS0 0
#define NAND_CS1 0x200

#define CMD_READ 1
#define CMD_PAGEPROGRAM 2
#define CMD_BLOCKERASE 3
#define CMD_BOOT 4
#define CMD_RESET 15
	
#endif /* __MLC_H__ */
