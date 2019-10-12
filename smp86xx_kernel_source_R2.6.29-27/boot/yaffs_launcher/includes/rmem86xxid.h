/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   rmem86xxid.h
  @brief  

  long description

  @author Emmanuel Michon
  @date   2004-09-22
*/

#ifndef __RMEM86XXID_H__
#define __RMEM86XXID_H__

/*
  the main chip ids 
  
  tango3 is for asic development (should be tango\infty)

  Usually, users do not set by hand, but thru `rmcflags' helper
*/
#define EM86XX_CHIPID_MAMBO      1000
#define EM86XX_CHIPID_MAMBOLIGHT 2000
#define EM86XX_CHIPID_TANGO      3000
#define EM86XX_CHIPID_TANGOLIGHT 4000
#define EM86XX_CHIPID_TANGO15    4500
#define EM86XX_CHIPID_TANGO2     5000
#define EM86XX_CHIPID_TANGO3    10000
#define EM86XX_CHIPID_TANGOHEAD 20000

#if (EM86XX_CHIP==EM86XX_CHIPID_MAMBO)
#define S_EM86XX_CHIPID "mambo"
#elif (EM86XX_CHIP==EM86XX_CHIPID_MAMBOLIGHT)
#define S_EM86XX_CHIPID "mambolight"
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGOLIGHT)
#define S_EM86XX_CHIPID "tangolight"
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGO15)
#define S_EM86XX_CHIPID "tango15"
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGO2)
#define S_EM86XX_CHIPID "tango2"
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGO3)
#define S_EM86XX_CHIPID "tango3"
#elif (EM86XX_CHIP==EM86XX_CHIPID_TANGOHEAD)
#define S_EM86XX_CHIPID "head"
#else
#warning EM86XX_CHIP is not set --- proceeding with tango2 ES6 as a reasonable setting
#define EM86XX_CHIP EM86XX_CHIPID_TANGO2
#define EM86XX_REVISION 6
#endif

/* 
  revisions...
  
  Referring to whatever is written at the surface of the BGA,
  not the PCI revid / subid / etc. This detail is important for some chips
  are ambiguous software wise.
  
  1: ES1
  2: ES2
  3: ES3
  4: ES4 
  5: ES5 
  6: ES6 
  65: revA
  66: revB
  67: revC
  
  No ID, but numbers. For a 8630ES2 for instance: build with
  RMCFLAGS="... -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=2 ..."

  --------------------------------------------------------------------------
  package writing          ES1  ES2  ES3  ES4     ES5     ES6  ES7  ES8  ES9 revA revB revC

  EM86XX_REVISION            1    2    3    4       5       6    7    8    9  'A'  'B'  'C'

  8600 `mambo' series                                                           1    2    3

  8620 `tangolight' series                                                    (a)  (b) 0x82
  8622 `tango15' series   0x81                (d)0x81                   0x82
                                                                           ^this is the PCI revID

  863x `tango2' series (c)0x81 0x81 0x81 0x82 (e)0x82 (f)0x83 0x84 0x85 0x86                


  `tango3' series:

  T3shuttle (flipchip)       1    1(g)

  8644                                 3(m)

  8654                       1    2(h) 3(i) 4(k)                              
                                        (j)  (l)      

  8652                       1(n) 2(o)

  8646...

  Read the PCI revID with GR8 0x2feec

  Used xotp is pci subid (GR8 0x2fef0).

  (a) don't remember
  (b) no such chip
  (c) 8630: FibbedES1 ES1 ES2 ES3 are the same chip
  (d) 8622: ES1 and revA cannot be distinguished from revID. Software test impossible in practice
  (e) 8630: ES4 and ES5 cannot be distinguished from revID. Software test with 0x6c900 bit12
  (f) 8634: ES6 and RevA have the same revID (just different bonding option)
      8634: ES7 and RevB have the same revID (just different bonding option)
      8634: ES9 and RevC have the same revID (just different bonding option)

  <shuttles of early 2008>
  (g) fixing sflash die connection. main die is the same.
  (h) see /designs/tango3/doc/ECO/ES2/eco.xls

  <prod chips candidates jul 2008>
  (i) http://web.asic.sdesigns.com/twiki/bin/view/Main/TangoThreeIPTVStatus#Changes_from_ES2_to_ES3_Verifica
      change to rev3                                         The package marking is S3_CPE3 (DEV)
  (j) same as (i) but PROD.                                  The package marking is S4_CPE3 (PROD)
  (k) fix sata. fix rom timing. change to rev4.              The package has no marking, or handscraped S5
  (l) same as (k) but PROD.                                  The package has no marking, or handscraped S6
  `Newer' S5 and S6 also exist with non-software-differentiable minor fixes in process and masks. 
  On those, the label is machine printed.

  (m) resynthesis. no pci. has rev3. Package marking is SMP8644-S2-CBE3.
  hwbug#424: has no pci subid, use GR8 0x20814 instead.

  (n) rom rc3b, has a bug on division (9831/10362)
  (o) rom rc3c.

  See also: http://web.asic.sdesigns.com/twiki/bin/view/Main/RomIntegration
  --------------------------------------------------------------------------

  Usually, users do not set by hand, but thru `rmcflags' helper
*/
#ifndef EM86XX_REVISION
#error EM86XX_REVISION is not set
#endif

#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION=='A') 
#error inconsistent: 863x revA is actually -DWITH_PROD=1 -DEM86XX_REVISION=6
#endif

#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION=='B') 
#error inconsistent: 863x revB is actually -DWITH_PROD=1 -DEM86XX_REVISION=7
#endif

#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (EM86XX_REVISION=='C') 
#error inconsistent: 863x revC is actually -DWITH_PROD=1 -DEM86XX_REVISION=9
#endif

#if (EM86XX_REVISION==1)
#define S_EM86XX_REVISION "ES1"
#elif (EM86XX_REVISION==2)
#define S_EM86XX_REVISION "ES2"
#elif (EM86XX_REVISION==3)
#define S_EM86XX_REVISION "ES3"
#elif (EM86XX_REVISION==4)
#define S_EM86XX_REVISION "ES4"
#elif (EM86XX_REVISION==5)
#define S_EM86XX_REVISION "ES5"
#elif (EM86XX_REVISION==6)
#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (defined(WITH_PROD) || defined(WITH_FACSPROD))
#define S_EM86XX_REVISION "revA"
#else
#define S_EM86XX_REVISION "ES6"
#endif
#elif (EM86XX_REVISION==7)
#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (defined(WITH_PROD) || defined(WITH_FACSPROD))
#define S_EM86XX_REVISION "revB"
#else
#define S_EM86XX_REVISION "ES7"
#endif
#elif (EM86XX_REVISION==8)
#define S_EM86XX_REVISION "ES8"
#elif (EM86XX_REVISION==9)
#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2) && (defined(WITH_PROD) || defined(WITH_FACSPROD))
#define S_EM86XX_REVISION "revC"
#else
#define S_EM86XX_REVISION "ES9"
#endif

#elif (EM86XX_REVISION=='A')
#define S_EM86XX_REVISION "revA"
#elif (EM86XX_REVISION=='B')
#define S_EM86XX_REVISION "revB"
#elif (EM86XX_REVISION=='C')
#define S_EM86XX_REVISION "revC"
#else
#error complete revision strings
#endif

/* the compilation modes */
#define EM86XX_MODEID_WITHHOST   1000
#define EM86XX_MODEID_STANDALONE 2000

/* the dsps */
#define EM86XX_ENGINEID_MPEG0 10
#define EM86XX_ENGINEID_MPEG1 11
#define EM86XX_ENGINEID_AUDIO0 20
#define EM86XX_ENGINEID_AUDIO1 21
#define EM86XX_ENGINEID_AUDIO2 22
#define EM86XX_ENGINEID_DEMUX 30
#define EM86XX_ENGINEID_DEMUX0 30
#define EM86XX_ENGINEID_DEMUX1 31

/* user does not have to set an engine id. This makes sense for mu only */
#ifdef EM86XX_ENGINE

#if (EM86XX_ENGINE<EM86XX_ENGINEID_AUDIO0)
#define ON_MPEG_ENGINE 1
#define DSP16 1
#elif (EM86XX_ENGINEID_DEMUX0<=EM86XX_ENGINE)
#define ON_DEMUX_ENGINE 1
#else
#define ON_AUDIO_ENGINE 1
#endif

#if (EM86XX_ENGINE==EM86XX_ENGINEID_MPEG0)
#define SENG "mpeg0"
#elif (EM86XX_ENGINE==EM86XX_ENGINEID_MPEG1)
#define SENG "mpeg1"
#elif (EM86XX_ENGINE==EM86XX_ENGINEID_AUDIO0)
#define SENG "audio0"
#elif (EM86XX_ENGINE==EM86XX_ENGINEID_AUDIO1)
#define SENG "audio1"
#elif (EM86XX_ENGINE==EM86XX_ENGINEID_DEMUX0)
#define SENG "demux0"
#elif (EM86XX_ENGINE==EM86XX_ENGINEID_DEMUX1)
#define SENG "demux1"
#else
#endif // end of engine dependent stuff
#endif

#if (EM86XX_CHIP==EM86XX_CHIPID_TANGO2)
/*
#if (defined(WITH_PROD) || defined(WITH_FACSPROD)) && !defined WITH_XLOADED_UCODE
#error inconsistent flag combination.
#endif

#if (defined(WITH_PROD) || defined(WITH_FACSPROD)) && !defined WITH_IRQHANDLER_BOOTLOADER
#error inconsistent flag combination.
#endif
*/
#ifdef WITH_UCODE_BOOTLOADER
#error inconsistent flag combination. You probably want WITH_XLOADED_UCODE
#endif

#endif

#if defined XBOOT2_SMP8656 && !defined XBOOT2_SMP8652
#error inconsistent flags
#endif

/* the microcode debug mode */

#define EM86XX_DEBUG_CHIP	1000
#define EM86XX_DEBUG_SIMU	2000

/*
  must be ok for C and for gas. Be careful.
  Answer is 1 or -1, RMsignImmediate(0)=1.

  ?, < cannot be used
  0xffffffff because gas computes on 64bit

  ...CHECK ANY CHANGE...
 */
#define RMsignImmediate(x) ((1|((x)>>31))&0xffffffff)

#define RMHEXDIGIT(x,n) ('0'+ ( ((x)>>((n)*4))&0xf ) + ((RMsignImmediate( ( ((x)>>((n)*4))&0xf ) -10)+1)&0xffffffff)/2*('a'-'9'-1))

#endif // __RMEM86XXID_H__
