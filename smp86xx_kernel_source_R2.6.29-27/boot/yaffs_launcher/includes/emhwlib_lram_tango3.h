/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   emhwlib_lram_tango3.h
  @brief  

  Map of the localram (8KBytes)

  Traditionnally the start of localram is used to setup
  a few kilobytes bootstrap routine code+data
  (cache init, tlb init, load something bigger to DRAM, jump there).

  Fixed offsets are defined in this file as communication devices
  between hardware blocks.
  Even debug locations must be present here.

  The bootstrap routine is expected to preserve these and setup
  its stack under LR_STACKTOP.

  Keep addresses increasing in this file.

  See emhwlib_resources_shared.h how some resources bw. 0 and 0x100 are used already
  only when uCLinux is up with irq handler running

  @author Sebastien Beysserie
  @date   2007-06-26
*/

#ifndef __EMHWLIB_LRAM_TANGO3_H__
#define __EMHWLIB_LRAM_TANGO3_H__

/*
  Leaving 2.3KB to for some startup code and stack
  
  as long as the value of this symbol moves only up with time, backward compatibility is ok */
#define LR_STACKTOP              0x00000900

#define LR_XOS2K_C2X             0x00000900
#define LR_XOS2K_X2C             0x00000a00
#define LR_XOS2K_I2X             0x00000b00
#define LR_XOS2K_X2I             0x00000c00
#define LR_XOS2K_END             0x00000d00

#define LR_VSYNC_STRUCT          0x00000d00
/* between those irq handler world */
#define LR_VSYNC_STRUCT_END      0x00001900

#define LR_HB_CPU1               0x00001944
#define LR_HB_CPU2               0x00001948
#define LR_HB_CPU3               0x0000194c
#define LR_CPU_BOOTSEL           0x00001950 /* relying on the fact the soft reset & cpu bist does not clear lram */
#define LR_CPU_PERF0_CTRL        0x00001954
#define LR_CPU_PERF0_CNT         0x00001958
#define LR_CPU_PERF1_CTRL        0x0000195c
#define LR_CPU_PERF1_CNT         0x00001960
#define LR_HB_AUDIO2             0x00001964 /* to check if AUDIO2 will coexist with DEMUX1 */
#define LR_SUSPEND_ACK_AUDIO2    0x00001968
#define LR_HB_DEMUX1             0x0000196c
#define LR_SUSPEND_ACK_DEMUX1    0x00001970
/* 0x20 = 32 bytes unused */

#define LR_ZBOOTXENV_LOCATION    0x00001994
#define LR_BAT_D0                0x00001998
#define LR_BAT_D1                0x0000199c

/* rather use the http://bugs.soft.sdesigns.com/twiki/bin/view/Main/SevenSegmentDisplay if you can */
#define LR_ZBOOT_STAGE           0x000019a0 

#define LR_CHANNEL_INDEX         0x000019a4
#define LR_HB_IPU                0x000019a8
#define LR_PCI_INTERRUPT_ENABLE  0x000019ac
#define LR_HOST_INTERRUPT_STATUS 0x000019b0
#define LR_DRAM_DMA_SUSPEND      0x000019b4
#define LR_SUSPEND_ACK_MPEG0     0x000019b8
#define LR_SUSPEND_ACK_MPEG1     0x000019bc
#define LR_SUSPEND_ACK_AUDIO0    0x000019c0
#define LR_SUSPEND_ACK_AUDIO1    0x000019c4
#define LR_SUSPEND_ACK_DEMUX0    0x000019c8
#define LR_SUSPEND_ACK_IH        0x000019cc
#define LR_HB_IH                 0x000019d0
#define LR_HB_HOST               0x000019d4
#define LR_HB_CPU                0x000019d8
#define LR_HB_MPEG0              0x000019dc
#define LR_HB_MPEG1              0x000019e0
#define LR_HB_AUDIO0             0x000019e4
#define LR_HB_AUDIO1             0x000019e8
#define LR_HB_DEMUX0             0x000019ec
#define LR_IREJECT_STATUS        0x000019f0
#define LR_HB_VSYNC              0x000019f4
#define LR_SW_VAL_VSYNC_COUNT    0x000019f8 /* this location is used to count captured VSYNC */
#define LR_SW_VAL_PIXEL_ADDR     0x000019fc /* this location is used to store a pixel address to write the frame count */

#define LR_XENV2_RW              0x00001a00 /* up to 628 bytes */

#define LR_XOS_DUMP              0x00001c74 /* xtask dump --- might change */
#define LR_XENV2_RO              0x00001d00 /* up to the end, 628 bytes. This area is written by xpu, r.o. for others */

#define LR_IPU_BORG              0x00001f70 /* where ipu stage0 was pulled from */
#define LR_XOS_SECOND_COUNT      0x00001f74 /* second counter */
#define LR_XOS_SECOND_COUNT_ATX  0x00001f78 /* value of xtal cnt at last update of second counter */
#define LR_XOS_F_SYS_HZ          0x00001f7c /* system frequency in Hz */
#define LR_XOS_C2X_RO            0x00001f80 /* xos trusted copies of channel structures */
#define LR_XOS_X2C_RO            0x00001fa0
#define LR_XOS_I2X_RO            0x00001fc0
#define LR_XOS_X2I_RO            0x00001fe0

/* for backward compatibility */
#define LR_HB_DEMUX              LR_HB_DEMUX0
#define LR_SUSPEND_ACK_DEMUX     LR_SUSPEND_ACK_DEMUX0

#define LR_HB_XPU LR_XOS_SECOND_COUNT

#define LR_HB_CPU0 LR_HB_CPU

#define LRRO_SIZE (LR_XOS_SECOND_COUNT-LR_XENV2_RO)
#define LRRW_SIZE (LR_XOS_DUMP-LR_XENV2_RW)

#define XMJ_VAX (FG_SIZE-20)

#endif // __EMHWLIB_LRAM_TANGO3_H__
