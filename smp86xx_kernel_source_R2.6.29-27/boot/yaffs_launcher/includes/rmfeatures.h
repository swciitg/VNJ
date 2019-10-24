/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   rmfeatures.h
  @brief  

  Some CHIPID dependent stuff are properly described by XML.  
  However this is not enough when this impacts code

  @author Emmanuel Michon
  @date   2005-02-16
*/

#ifndef __RMFEATURES_H__
#define __RMFEATURES_H__

#define XTAL_HZ 27000000
#define DEMUX_DSP_TIMER_HZ 5000  /* Frequency of the DSP timer interrupt, in Hz */
#define UART_LCR_8N1 0x03    /* None, 8 bit, 1 bit */

#define LOG2_RM_PAGE_SIZE 12
#define RM_PAGE_SIZE (1<<LOG2_RM_PAGE_SIZE)

/*******************************************************************
************************************** decoding capabilities
*******************************************************************/

#ifndef EM86XX_CHIP
/* Well, duh, warnings are considered errors in most of the build */
/* #warning Ignoring contents of rmfeatures.h (EM86XX_CHIP not defined) */
#else /* EM86XX_CHIP */

#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO)
#define RMFEATURE_HAS_WMV9 1
#endif

#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO15)
#define RMFEATURE_HAS_H264 1
#endif

/* hack: maybe audio wmapro here? */

/*******************************************************************
************************************** sizes of the DSP PMEM and DMEM
*******************************************************************/

#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO3)

#define CPU_ICACHE_SIZE (32*1024)
#define CPU_DCACHE_SIZE (32*1024)

#define IPU_ICACHE_SIZE (16*1024)
#define IPU_DCACHE_SIZE (16*1024)

#define XPU_ICACHE_SIZE (12*1024)
#define XPU_DCACHE_SIZE (12*1024)

#define CPU_LOCALRAM_SIZE 0x2000

#define MPEG_PM_SIZE 0x3000
#define MPEG_DM_SIZE 0x1000

#define DEMUX_PM_SIZE 0x6000 /* 24kB = 6k x 32bits */
#ifdef XBOOT2_SMP8656
#define DEMUX_DM_SIZE 0x8000 /* 32kB = 8k x 32bits */
#else
#define DEMUX_DM_SIZE 0x4000 /* 16kB = 4k x 32bits */
#endif

#ifdef XBOOT2_SMP865X
#define AUDIO_PM_SIZE 0x3000
#define AUDIO_DM_SIZE 0xc000

#define FG_SIZE 0x5000
#else
#define AUDIO_PM_SIZE 0x8000
#define AUDIO_DM_SIZE 0xf000

#define FG_SIZE 0xa000
#endif

/* respin will take time. See bug#8931 */
#define IRQ_HANDLER_CONFIG_MAXSIZE (64*1024)

#else

#define CPU_LOCALRAM_SIZE 0x2000
#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO3)
#define MPEG_PM_SIZE 0x3000
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION>=4)
#define MPEG_PM_SIZE 0x3000
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGO15) && (EM86XX_REVISION>='B')
#define MPEG_PM_SIZE 0x2800
#else
#define MPEG_PM_SIZE 0x2000
#endif
#define MPEG_DM_SIZE 0x1000

#define DEMUX_PM_SIZE 0x4000 /* 16kB = 4k x 32bits */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO15)
#define DEMUX_DM_SIZE 0x4000 /* 16kB = 4k x 32bits */
#else
#define DEMUX_DM_SIZE 0x2000 /*  8kB = 2k x 32bits */
#endif

#if ((EM86XX_CHIP>=EM86XX_CHIPID_TANGO2) ||((EM86XX_CHIP==EM86XX_CHIPID_TANGO15) && (EM86XX_REVISION>='B')))
#define AUDIO_PM_SIZE 0x3000
#else
#define AUDIO_PM_SIZE 0x2800
#endif
#define AUDIO_DM_SIZE 0xc000

#define MAX_DDR2_BANK_SIZE (512*1024*1024)

#endif

#define ZBOOTXENV_MAXSIZE 0x4000
#define HB_PERIOD_MS 1000

#if (EM86XX_CHIP==EM86XX_CHIPID_MAMBO)
#define RMFEATURE_HAS_AUDIO_ENGINE_1 1
#define RMFEATURE_HAS_DRAM_CONTROLLER_1 1
#elif ((EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION>=4))
#define RMFEATURE_HAS_AUDIO_ENGINE_1 1
#define RMFEATURE_HAS_VIDEO_ENGINE_1 1
#define RMFEATURE_HAS_DRAM_CONTROLLER_1 1
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGO3)
#define RMFEATURE_HAS_DRAM_CONTROLLER_1 1
#ifdef XBOOT2_SMP865X /* for 8654 = tango3_iptv chip there are two demux engines, one video engine and one audio engine */
#define RMFEATURE_HAS_DEMUX_ENGINE_1 1
#else /* for 8644 = tango3 chip there are one demux engine, two video engines and three audio engines */
#define RMFEATURE_HAS_AUDIO_ENGINE_1 1
#define RMFEATURE_HAS_AUDIO_ENGINE_2 1
#define RMFEATURE_HAS_VIDEO_ENGINE_1 1
#endif /* XBOOT2_SMP865X */
#endif

/*******************************************************************
************************************* sizes of scaler's linebuffers
*******************************************************************/

#define SCALER_MAX_INPUT_HEIGHT    2048

/* Tangolight, Tango2Shuttle */
#if ((EM86XX_CHIP==EM86XX_CHIPID_TANGOLIGHT) || ((EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION<4)))
#define MULTISCALER_LINEBUFFER_SIZE 6144 /* total memory: Y + UV */
#define MULTISCALER_MAX_LINE_WIDTH   512 /* in 32BPP mode */
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGO2)
#define MULTISCALER_LINEBUFFER_SIZE      12288 /* total memory: Y + UV */
#define MULTISCALER_MAX_LINE_WIDTH       1024 /* in 32BPP mode */
#define MULTISCALER_ES9_MAX_LINE_WIDTH   2048 /* in 32BPP mode */
#else /*Tango3 and up */
#define MULTISCALER_LINEBUFFER_SIZE      12288 /* total memory: Y + UV */
#define MULTISCALER_MAX_LINE_WIDTH       2048 /* in 32BPP mode */
#endif

#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define MULTISCALER_MAX_VBUS_UNIT    0x3f
#else
#define MULTISCALER_MAX_VBUS_UNIT    0x1f
#endif

/* for tango2 ES8 and greater. Runtime switch */
#define ES8_SCALER_VBUS_DRAM_PRIORITY 8
#define ES8_GFX_ENGINE_VBUS_DRAM_PRIORITY 9
#define ES8_MBUS_DRAM_PRIORITY 6

/* for all other chips */
#define SCALER_VBUS_DRAM_PRIORITY 3
#define GFX_ENGINE_VBUS_DRAM_PRIORITY 1
#define MBUS_DRAM_PRIORITY 2

/* Tango2Shuttle, Tango2 and up */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define SPUSCALER_LINEBUFFER_SIZE 12288
#define SPUSCALER_MAX_LINE_WIDTH   2048 /* in 32BPP mode */
#define SPUSCALER_MAX_VBUS_UNIT    0x3f
#else
#define SPUSCALER_LINEBUFFER_SIZE  4096
#define SPUSCALER_MAX_LINE_WIDTH   1024 /* in 32BPP mode */
#define SPUSCALER_MAX_VBUS_UNIT    0x1f
#endif

#define OSDSCALER_LINEBUFFER_SIZE 24576
#define OSDSCALER_MAX_LINE_WIDTH   2048 /* in 32BPP mode */
#define OSDSCALER_MAX_VBUS_UNIT    0x7f

#define MAINVIDEOSCALER_LINEBUFFER_SIZE 24576 /* total memory: Y + UV */
/* video scaler does not support 32BPP but fake it */
#define MAINVIDEOSCALER_MAX_LINE_WIDTH 1024

#define MAINVIDEOSCALER_MAX_VBUS_UNIT_LUMA  0x3f
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define MAINVIDEOSCALER_MAX_VBUS_UNIT_CHROMA  0x3f
#else
#define MAINVIDEOSCALER_MAX_VBUS_UNIT_CHROMA 0x1f
#endif

/*******************************************************************
************************************** features of PLLs
*******************************************************************/

/* Tango3 and up */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO3)

#if (((EM86XX_CHIP==EM86XX_CHIPID_TANGO3) && (EM86XX_REVISION>=3)) || (EM86XX_CHIP>EM86XX_CHIPID_TANGO3))  /* Tango3 ES3 and up */
#define RMFEATURE_HAS_MNK_PLL 1   /* PLL with (N+1)/(M+1)/(2^K) factor instead of (N+2)/(M+2) */
#define RMFEATURE_PLL_READONLY 1  /* PLL registers are set by boot loader only and shall not be changed */
#endif // EM86XX_REVISION

#define RMFEATURE_HAS_PLL_CD_CONTROL 1  /* Clean Divider can be disabled in control register */
#define RMFEATURE_PLL_SYSCLK_ON_DSP 1   /* System Clock is used as the video DSP clock */

#endif // EM86XX_CHIP

/*******************************************************************
************************************** usage of PLLs
*******************************************************************/

/* Selection of System clock PLL */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define PLL_SYSTEM PLLGen_pll_3
#else
#define PLL_SYSTEM PLLGen_pll_0
#endif

/* Selection of CD input clock source */
#if (EM86XX_CHIP<EM86XX_CHIPID_TANGO2)
#define PLL_CD_PLL PLLGen_pll_0 /* hard wired to system clock */
#define PLL_CD_PLLOUT PLLOut_0
#define PLL_CD_FREQ 0  /* don't touch CD input */
#elif ((EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION<4))
#define PLL_CD_PLL PLLGen_pll_3
#define PLL_CD_PLLOUT PLLOut_0
#define PLL_CD_FREQ 0  /* don't touch CD input */
#else
#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2)
#define PLL_CD_PLL PLLGen_pll_1  /* Tango2: SysClk from PLL2, CDClk from PLL1 */
#else
#define PLL_CD_PLL PLLGen_pll_2  /* Tango3: SysClk from PLL1, CDClk from PLL2 */
#endif
#define PLL_CD_PLLOUT PLLOut_0
#define PLL_CD_FREQ 405000000
#endif

/* Selection of Audio Engine clock source */
#if (EM86XX_CHIP<EM86XX_CHIPID_TANGO)
#define PLL_AUDIO PLLGen_pll_3  /* use normal PLL on Mambo */
#else
#define PLL_AUDIO PLLGen_cd_0  /* use clean divider on Tango and up */
#endif

/* Selection of Video clock sources */
#if (EM86XX_CHIP<EM86XX_CHIPID_TANGO2)
#define PLL_VIDEO_PRI PLLGen_pll_1  /* use PLL 1 for primary route */
#define PLL_VIDEO_SEC PLLGen_pll_2  /* use PLL 2 for secondary route */
#define PLL_VIDEO_OUT PLLOut_1
#else
#define PLL_VIDEO_PRI PLLGen_cd_8  /* use CD 8 for primary/digital route */
#define PLL_VIDEO_SEC PLLGen_cd_9  /* use CD 9 for secondary route */
#define PLL_VIDEO_TER PLLGen_cd_10  /* use CD 10 for third route */
#define PLL_VIDEO_OUT PLLOut_0
#endif

#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO15)
#define PLL_AUDIO_ENGINE_0  PLLGen_cd_0
#define PLL_AUDIO_ENGINE_1  PLLGen_cd_1
#define PLL_VIDEO_DIGITAL   PLLGen_pll_1
#define PLL_VIDEO_ANALOG    PLLGen_pll_2
#endif
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define PLL_AUDIO_ENGINE_0  PLLGen_cd_0
#define PLL_AUDIO_ENGINE_1  PLLGen_cd_1
#define PLL_VIDEO_DIGITAL   PLLGen_cd_8
#define PLL_VIDEO_ANALOG    PLLGen_cd_9
#define PLL_VIDEO_COMPONENT PLLGen_cd_10
#endif

/*******************************************************************
************************************** within audio block
*******************************************************************/

/* Tango15, Tango2Shuttle, Tango2 and up */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO15)
#define RMFEATURE_HAS_SPDIF_INPUT 1  /* separate pin for SPDIF capture, not shared with I2S */
#endif

/* Tango 3 and up features */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO3)
#define RMFEATURE_SWAP_I2S1 1  /* Swap I2S1 (LFE now on I2S1L, C now on I2S1R) to match HDMI channel mapping */
#endif

#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define RMFEATURE_HAS_AUDIOOUTPADSCTRL  /* control for I2S out and SPDIF out pads in bits [25:22] of SO_CH_CTRL1 */
#endif

/*******************************************************************
************************************** within display block
*******************************************************************/

/* Full, Light or Extra Light Display Block? */
#if ( \
	(EM86XX_CHIP==EM86XX_CHIPID_TANGOLIGHT) || \
	(EM86XX_CHIP==EM86XX_CHIPID_TANGO15) || \
	((EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION<4)) \
)
#define RMFEATURE_HAS_EXTRALIGHT_DISPLAY 1
#else
#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2)
#define RMFEATURE_HAS_LIGHT_DISPLAY 1
#endif
#endif

/* Specific display features */
#define RMFEATURE_HAS_VIDEO_IN 1
#define RMFEATURE_HAS_DIGITAL_OUT 1
#define RMFEATURE_HAS_MAIN_ANALOG_OUT 1
#define RMFEATURE_HAS_HARD_GFX_SCALER 1
#define RMFEATURE_HAS_MAIN_MIXER 1
#define RMFEATURE_HAS_BLACKSTRIP_BITS_COUNT 9

/* Mambo, MamboLight, "Tango" */
#if (EM86XX_CHIP<EM86XX_CHIPID_TANGOLIGHT)
#define RMFEATURE_HAS_GRAPHIC_IN 1
#define RMFEATURE_HAS_COMPONENT_OUT 1
#define RMFEATURE_HAS_COMPOSITE_OUT 1
#define RMFEATURE_HAS_VCR_SCALER 1
#define RMFEATURE_HAS_CRT_SCALER 1
#define RMFEATURE_HAS_VCR_MIXER 1
#endif

/* Tango15, Tango2Shuttle, Tango2 and up */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO15)
#define RMFEATURE_HAS_GRAPHIC_IN 1
#endif

/* Tango2 and up */
#if (((EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION>=4)) || (EM86XX_CHIP>EM86XX_CHIPID_TANGO2))
#define RMFEATURE_HAS_GRAPHIC_IN 1
#define RMFEATURE_HAS_VCR_SCALER 1
#define RMFEATURE_HAS_VCR_CHANNEL 1
#define RMFEATURE_HAS_VIDEO_PLANE 1
#define RMFEATURE_HAS_COMPONENT_OUT 1
#define RMFEATURE_HAS_BCS2 1
#undef RMFEATURE_HAS_BLACKSTRIP_BITS_COUNT
#define RMFEATURE_HAS_BLACKSTRIP_BITS_COUNT 11
#define RMFEATURE_HAS_HDMI 1
#define RMFEATURE_HAS_GRAPHACC_CSCONV 1 /* colorspace and sampling mode conversion */
#define RMFEATURE_HAS_GRAPHACC_FULLBLEND 1
#define RMFEATURE_HAS_GRAPHACC_GRADIENTS 1
#define RMFEATURE_HAS_COMPONENT_COMPOSITE_MODE 1 /* composite mode bits in component out config register (for 625i modes) */
#define RMFEATURE_HAS_HDSD_CONVERTER 1
#endif

/* Tango3 and up */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO3)

#define RMFEATURE_T3BC 1
#define RMFEATURE_COMPONENT_OUT_HAS_TELETEXT 1
#define RMFEATURE_CONFIGURABLE_HWC_LAYER_ORDER 1
#define RMFEATURE_HAS_CONSTRAINED_IMAGE 1
#define RMFEATURE_HAS_DAC_MUX 1  /* Also for "PAL Nc" (Argentina) config2 bit */

#if ! (defined XBOOT2_SMP865X) 
#define RMFEATURE_HAS_CRT_SCALER 1  /* 8644 only */
#endif

#endif

/* Tango3 only */
#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO3)
#define RMFEATURE_FILTER_COEFF_BUG 1  /* All Tango3: some filter coefficients have incorrect sum of 257 instead of 256 */
#endif

/* Tangolight, Tango15, Tango2Shuttle, Tango3 */
#if ( \
	((EM86XX_CHIP>=EM86XX_CHIPID_TANGOLIGHT) && (EM86XX_CHIP<EM86XX_CHIPID_TANGO2)) || \
	((EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION<4)) || \
	 (EM86XX_CHIP==EM86XX_CHIPID_TANGO3) \
)
#define RMFEATURE_HAS_DUAL_MAINANALOGOUT 1 /* Main Analog Out has 3 extra DACs for CVBS/S-Video out */
#endif

/* "Tango", Tangolight, Tango15, Tango2Shuttle, Tango2 and up */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO)
#define RMFEATURE_HAS_CAPTURE_COUNTER 1  /* chip provides line and pixel counters for input */
#define RMFEATURE_HAS_VBI_CAPTURE 1  /* chip has raw VBI capture capability */
#define RMFEATURE_HAS_GRAPHIC_IN 1
#define RMFEATURE_HAS_HWI2C 1  /* chip has hardware i2c interface (master and slave) */
#endif

/* Tango15, Tango2Shuttle, Tango2 and up */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO15)
#define RMFEATURE_HAS_ANC_CAPTURE 1  /* chip can capture ANC-formatted (sliced) VBI data */
#define RMFEATURE_HAS_SPU_SCALER 1  /* chip has working SPU scaler */
#define RMFEATURE_HAS_CAPTURE_COUNTER2 1  /* chip provides active line count for 656 input */
#endif

/* Tango2Shuttle, Tango2 and up */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define RMFEATURE_HAS_GENERIC_SPU_SCALER /* chip has a 8bit graphic scaler for SPU */
#define RMFEATURE_HAS_DOUBLE_RATE
#endif

/* Tangolight, Tango2Shuttle */
#if ((EM86XX_CHIP==EM86XX_CHIPID_TANGOLIGHT) || ((EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION<4)))
#define RMFEATURE_HAS_REDUCED_MULTISCALERBUFFER 1  /* size of multi-scaler input buffer is reduced */
#endif

#if (RMFEATURE_HAS_HWI2C)  /* backwards compatibility, used to be defined for Tango2 before ES8 only. Err on safe side */
#define RMFEATURE_HAS_FAULTY_I2C_BURST_WRITE 1 /* OBSOLETE: (Hardware I2C burst write is sending MSBit of previous data byte when starvation occurs) Use I2C:BurstPatch property instead! */
#endif

#define RMFEATURE_XOFFSET_DIG_COMPENSATE 3        /* sync-to-video delay in the digital output block */
#define RMFEATURE_XOFFSET_HDMI_COMPENSATE 2       /* sync-to-video delay for internal SiI9030 HDMI chip */
#define RMFEATURE_XOFFSET_VESA_COMPENSATE 7       /* sync-to-video delay for analog VESA modes */
#define RMFEATURE_XOFFSET_HDTV_COMPENSATE 13      /* sync-to-video delay for analog HDTV modes */
#define RMFEATURE_XOFFSET_SDTV525_COMPENSATE 32   /* sync-to-video delay for analog 525 line modes (4X oversampled) */
#define RMFEATURE_XOFFSET_SDTV625_COMPENSATE 80   /* sync-to-video delay for analog 625 line modes (4X oversampled) */

#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define WITH_NEW_VCXO
#define RMFEATURE_WITH_BWSAMPLING
#endif

/* hack tbd by Christian W. */
/* sizes of some buffers (IN BYTES) */
/* etc. */
/* use format RMFEATURE_HAS_XXX */

#endif /* EM86XX_CHIP */

/* video tokens */
#define LOG2_IPATOKEN_VIDEO_MPEG12  15
#define LOG2_IPATOKEN_VIDEO_MPEG4   14
#define LOG2_IPATOKEN_VIDEO_H264    13
#define LOG2_IPATOKEN_VIDEO_VC1     12
#define LOG2_IPATOKEN_VIDEO_WMV9    11
#define LOG2_IPATOKEN_VIDEO_JPEG    10
#define LOG2_IPATOKEN_VIDEO_MSMPEG4 9
#define LOG2_IPATOKEN_VIDEO_AVS     8
#define LOG2_IPATOKEN_VIDEO_H261    7

/* audio tokens */
#define LOG2_IPATOKEN_AUDIO_MPEGL12   31
#define LOG2_IPATOKEN_AUDIO_MPEGL3    30
#define LOG2_IPATOKEN_AUDIO_MPEG4AAC  29
#define LOG2_IPATOKEN_AUDIO_MPEG4BSAC 28
#define LOG2_IPATOKEN_AUDIO_MPEG4HE   27
#define LOG2_IPATOKEN_AUDIO_DD        26
#define LOG2_IPATOKEN_AUDIO_DDP       25
#define LOG2_IPATOKEN_AUDIO_TRUEHD    24
#define LOG2_IPATOKEN_AUDIO_DTS       23
#define LOG2_IPATOKEN_AUDIO_DTSNEO6	  22
#define LOG2_IPATOKEN_AUDIO_DTSHD     21
#define LOG2_IPATOKEN_AUDIO_DTSES	  20
#define LOG2_IPATOKEN_AUDIO_DTSHDMA   19
#define LOG2_IPATOKEN_AUDIO_DTSHDLBR  18
#define LOG2_IPATOKEN_AUDIO_WMA9      17
#define LOG2_IPATOKEN_AUDIO_WMA9PRO   16
#define LOG2_IPATOKEN_AUDIO_ATRAC3    15
#define LOG2_IPATOKEN_AUDIO_DTSRE     14
#define LOG2_IPATOKEN_AUDIO_DDRE      13
#define LOG2_IPATOKEN_AUDIO_DDPL2X    12
#define LOG2_IPATOKEN_AUDIO_DRA    	  11
#define LOG2_IPATOKEN_AUDIO_DDPL2	  10
#define LOG2_IPATOKEN_AUDIO_WOWHD	  9
#define LOG2_IPATOKEN_AUDIO_TRUSUR	  8
#define LOG2_IPATOKEN_AUDIO_TRUVOL	  7

/* demux tokens */
#define LOG2_IPATOKEN_DEMUX_C2        31
#define LOG2_IPATOKEN_DEMUX_DVBCSA    30
#define LOG2_IPATOKEN_DEMUX_MULTI2    29
#define LOG2_IPATOKEN_DEMUX_CSS       28
#define LOG2_IPATOKEN_DEMUX_RC4       27
#define LOG2_IPATOKEN_DEMUX_AES       26
#define LOG2_IPATOKEN_DEMUX_DES       25

/* RMFEATURE_OUTPORT_COUNT */
#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO15)
#define RMFEATURE_OUTPORT_COUNT 2
#endif
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define RMFEATURE_OUTPORT_COUNT 3
#endif


/* RMFEATURE_HAS_OUTPORT_RESET_DELAY  */
#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define RMFEATURE_HAS_OUTPORT_RESET_DELAY 1
#endif

/* From TANGO3, no more DEBUG_PROBEx locations in LR */
#if (EM86XX_CHIP<=EM86XX_CHIPID_TANGO2)
#define RMFEATURE_HAS_LR_DEBUG_PROBE
#endif


#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO2)
#define RMFEATURE_HAS_MULTISCALER_LUMA_KEYING 1
#endif


#if (EM86XX_CHIP<EM86XX_CHIPID_TANGO3)

/* up to Tango2 */
#define RMTILE_WIDTH_SHIFT  7
#define RMTILE_HEIGHT_SHIFT 5
#define RMFEATURE_DIGITAL_MAT_BITS 11
#define RMFEATURE_DIGITAL_CST_BITS  3
#define RMFEATURE_ANALOG_MAT_BITS  12
#define RMFEATURE_ANALOG_CST_BITS   4
#define RMFEATURE_ANALOG_TOTAL_SIZE_BITS 12

#else /* EM86XX_CHIP<EM86XX_CHIPID_TANGO3 */

/* Tango3 and up */
#define RMTILE_WIDTH_SHIFT  8
#define RMTILE_HEIGHT_SHIFT 5
#define RMFEATURE_DIGITAL_MAT_BITS 13
#define RMFEATURE_DIGITAL_CST_BITS  6
#define RMFEATURE_ANALOG_MAT_BITS  12
#define RMFEATURE_ANALOG_CST_BITS   5
#define RMFEATURE_ANALOG_TOTAL_SIZE_BITS 13


/* New features of Tango 3 and up */
#define RMFEATURE_HAS_DEEP_COLOR 1
#define RMFEATURE_HAS_BLACK_STRIP_COLOR 1
#define RMFEATURE_HAS_VBUS_PALETTE 1
#define RMFEATURE_HAS_DEDICATED_DEINTERLACING 1
#define RMFEATURE_HAS_POSTPROCESSING
#define RMFEATURE_SOFT_GFX_SCALER_IS_HARD_CRT_SCALER 1 
#define RMFEATURE_HAS_MP_SCALER 1
#define RMFEATURE_HARD_CRT_SCALER_IS_HARD_MP_SCALER 1  
#define RMFEATURE_VIDEO_PLANE_HAS_MIXER_INDEX_0 1

#if (EM86XX_REVISION>=3)
#define RMFEATURE_VIDEO_PLANE_CAN_DO_BCS_CSC 1
#define RMFEATURE_MIXER_CAN_BLEND_WITH_BACKGROUND 1 
#endif

#if defined(XBOOT2_SMP865X)  /* 8654 ? */
#undef RMFEATURE_HAS_HARD_GFX_SCALER  /* 8654 has no hw gfx scaler */
#if 0
#define RMFEATURE_USE_VCR_SCALER
#endif 
#else /* 8644 ? */
#define RMFEATURE_SOFT_CRT_SCALER_IS_HARD_GFX_SCALER 1 /* 8644: soft crt is mapped to hw gfx scaler */
#endif

#ifdef RMFEATURE_HAS_MP_SCALER
#define MPSCALER_MAX_LINE_WIDTH   2048 
#define MPSCALER_LINEBUFFER_SIZE   32768  /* in 32BPP mode: 4 lines x 2048 pixels x 32bits */
#define RMFEATURE_MPSCALER_8BPP_MAT_BITS 9   /* 8bpp synthesized CSC matrix coefficients have 9 bits for floating point precision */ 
#define RMFEATURE_MPSCALER_8BPP_CST_BITS 1   /* 8bpp synthesized CSC cst coefficients have 1 bit for floating point precision */ 
#define RMFEATURE_MPSCALER_12BPP_MAT_BITS 13 /* 12bpp synthesized CSC matrix coefficients have 13 bits for floating point precision */ 
#define RMFEATURE_MPSCALER_12BPP_CST_BITS 5  /* 12bpp synthesized CSC matrix coefficients have 1 bits for floating point precision +4 for 8bpp->12bpp pixels */ 
#endif

#define RMFEATURE_HAS_XENV_IH_API_GA 1

#define RMFEATURE_HAS_HDMI_DDC_BYPASS 1
#define RMFEATURE_HAS_PADS_REF 1
#define RMFEATURE_HAS_HDTV_2X_OVERSAMPLING 1
#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO3)
#define RMFEATURE_HAS_HDTV_BROAD_PULSE_OFFSET  /* When not in 2X oversampling, the component output broad pulse start value has an additional built-in offset of 45 pixel */
#endif /* EM86XX_CHIP==EM86XX_CHIPID_TANGO3 */
#define RMFEATURE_HAS_CEC 1

#endif /* EM86XX_CHIP<EM86XX_CHIPID_TANGO3 */

#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO3) && (EM86XX_REVISION>=3) /* number of bits per mixer layer */
#define MAIN_MIXER_LAYER_OFFSET 4
#else
#define MAIN_MIXER_LAYER_OFFSET 3
#endif


#define RMTILE_SIZE_SHIFT (RMTILE_WIDTH_SHIFT + RMTILE_HEIGHT_SHIFT)


#endif /* __RMFEATURES_H__ */
