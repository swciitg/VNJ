/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   mlc_pa.h
  @brief  

  Fixed ECC formatting specified in
  http://web.soft.sdesigns.com/twiki/bin/view/Main/ApplicationNote30MLCT4Boot

  MLC spec tango02_host.doc cvs 1.53 section 1.14

  This page-size- and block-size- agnostic API is entirely written in byte offsets and byte sizes.
  Up to the caller to write wrappers to operate on page# or block# should he feel more comfortable.
  This API does not use the small page / large page / ultra large page terminology but byte sizes and shifts (``log2'').
  Devices larger than 4GB impose the usage of RMuint64 for offsets.

  The concepts of _channel_ (chA(0) or chB(1)) and _chip select_ (cs# 0 or 1) are distinct so user should not alias 
  chA - cs#0 and chB - cs#1
  
  ***
  
  *** Concurrent access to a cs# is out of scope of this API and locking has to be written in an upper software layer. ***

  Concurrent access to chA (resp chB) is enforced by this library implementation using AUDIO_0_CHA_MUTEX (resp AUDIO_0_CHB_MUTEX).

  Pseudo code would do this, for instance, for a probe-and-several-eccread:

  -------------------------
  wanna operate on cs#0 

  request(A)... busy
  request(B)... busy
  request(A)... busy
  request(B)... idle! [AUDIO_0_CHB_MUTEX is now taken]

  probe(cs#0,B,...) -> writes timings to chB, reset chB, returns device size, block size, page size
  
  suppose l2ps is 11.

  wanna read well aligned data:

  offset=0x1000000
  size=0xf000
  ga=...
  while (size!=0) {
  trigger_eccread(cs#0,B,11,&ga,&offset,&size,NULL,NULL,NULL);
   --- DO SOMETHING ELSE --- TYPICALLY LINUX WOULD GET BACK TO YOU UPON MBUS INTERRUPT RIGHT ON TIME ---
  while (request(B,&errors)==BUSY);
  if (errors==-1)
        bailout;
  }

  wanna read badly aligned data:

  offset=0x1357003
  size=0xf3ff7
  ga=...
  -> process the [offset,RMALIGNTONEXT(offset,l2ps)[  with sparseeccread (sllllow)
  -> process the middle large section                 with       eccread (fast)
  -> process the [RMALIGNTO(offset+size),offset+size[ with sparseeccread (sllllow)

  yield(B) [AUDIO_0_CHB_MUTEX is now released]
  -------------------------

  ***

  There are two manners of accessing: 

  raw (read/write/erase) 
  -> excepted `erase', those are mostly for maintenance and debugging
  -> is suitable for devices where data is unformatted, or formatted a different way than Sigma
  -> gets/puts weird sized buffers where metadata data ecc bb unused are all mixed
  -> if those are known to come from a device that respects Sigma formatting though, and lie on
     even page boundaries, it is a good idea to decapsulate them with the provided struct page0, page1 or page2.
     
  with ecc (read/write)
  -> limited to the formatting defined by Sigma, indicated by the l2ps parameter:
    9 for .5KB pages,
    11 for 2KB pages,
    12 for 4KB pages.
  -> efficient functions operate on integral pages and get/put data/metadata/bb to/from separate arrays
  -> one function allows sparse read

  ***

  - sparse read with stride?

  ***

  Since 2010 Jan 19 this library has a non-ddr build mode where the function set varies

Normal build:

0000001c T mlc_pa_request
00000294 T mlc_pa_yield
000002bc T mlc_pa_trigger_rawread
00000604 T mlc_pa_trigger_rawwrite
00000948 T mlc_pa_trigger_eccblockerase
00000a48 T mlc_pa_trigger_rawblockerase
00000b00 T mlc_pa_trigger_eccread
00000d20 T mlc_pa_trigger_eccwrite
00000fec T mlc_pa_trigger_sparseeccread
0000133c T mlc_pa_probe
00001458 T mlc_pa_sync_rawread
00001494 T mlc_pa_sync_rawwrite
000014d0 T mlc_pa_sync_eccread
00001554 T mlc_pa_sync_eccwrite
0000159c T mlc_pa_sync_eccblockerase
00001610 T isbb
00001734 T bba2p
0000175c T bbaeccread_PBLOCK

-DMLC_PA_NODDR=1 build: This library cannot write (well rawwrite could be is not very useful)
-DMLC_ECCFAILGIVESUP=1 build: out of ECC capability on one packet during a read, will abort the remaining packets

00000000 T mlc_pa_request
00000278 T mlc_pa_yield

000002a0 T mlc_pa_trigger_eccblockerase

000003a0 T mlc_pa_trigger_sparseeccread

00000588 T mlc_pa_probe

000006a4 T mlc_pa_sync_eccread

00000704 T mlc_pa_sync_eccblockerase
00000778 T isbb
00000894 T bba2p
000008bc T bbaeccread_PBLOCK

  @author Emmanuel Michon
  @date   2009-11-25
*/

#ifndef __MLC_PA_H__
#define __MLC_PA_H__

#include "mlc.h"

/* formatting of ecc'd packets */

// ecc capability bit count gives ecc length: multiply by 13, align to next multiple of 8, result is in bits [mlc spec 1.14.3]
#define C2L(x) (RMALIGNTONEXT((x)*13,3)>>3)

#define ECCCAPA0 4
struct packet0_last {
	char data[512];
	char bb[2];
	char ecc[C2L(ECCCAPA0)];
} __attribute__((packed));

#define ECCCAPA1 8
struct packet1_n {
	char data[512];
	char ecc[C2L(ECCCAPA1)];
} __attribute__((packed));
struct packet1_last {
	char databeg[469];
	char bb[6];
	char dataend[43];
	char ecc[C2L(ECCCAPA1)];
} __attribute__((packed));

#define ECCCAPA2 9
struct packet2_n {
	char data[512];
	char ecc[C2L(ECCCAPA2)];
} __attribute__((packed));
struct packet2_last {
	char databeg[403];
	char bb[1];
	char dataend[109];
	char ecc[C2L(ECCCAPA2)];
} __attribute__((packed));

#define ECCCAPA3 16
struct packet3_n {
	char data[512];
	char ecc[C2L(ECCCAPA3)];
} __attribute__((packed));
struct packet3_last {
	char databeg[326];
	char bb[1];
	char dataend[186];
	char ecc[C2L(ECCCAPA3)];
} __attribute__((packed));

#define ECCCAPA4 16
struct packet4_n {
	char data[512];
	char ecc[C2L(ECCCAPA4)];
} __attribute__((packed));
struct packet4_last {
	char databeg[118];
	char bb[1];
	char dataend[394];
	char ecc[C2L(ECCCAPA4)];
} __attribute__((packed));

/* formatting of ecc'd pages */

struct page0 {
	char metadata[4];
	struct packet0_last l;
	char unused[3];
} __attribute__((packed));

struct page1 {
	char metadata[4];
	struct packet1_n n[3];
	struct packet1_last l;
	char unused[2];
} __attribute__((packed));

struct page2 {
	char metadata[4];
	struct packet2_n n[7];
	struct packet2_last l;
	char unused[3];
} __attribute__((packed));

struct page3 {
	char metadata[4];
	struct packet3_n n[7];
	struct packet3_last l;
	char unused[11];
} __attribute__((packed));

struct page4 {
	char metadata[4];
	struct packet4_n n[15];
	struct packet4_last l;
	char unused[27];
} __attribute__((packed));

struct mlc_pa_job {
	int ch;
	int cs; // irrelevant for initial request()

	// filled by probe()
	RMuint32 jdecid;

	// http://web.soft.sdesigns.com/twiki/bin/view/Main/ApplicationNote30MLC
	RMuint32 scheme;

	/*
	  128 bytes of spare per 4096 bytes is spareper128=4
	  224 bytes of spare per 4096 bytes is spareper128=7
	 */
	RMuint32 spareper128;

	RMuint32 l2wholesize; // not including spare.
	RMuint32 l2blocksize; // not including spare.
	RMuint32 l2ps;

	// the ongoing request. Fields are input by trigger() and autoincremented on successful request()
	RMuint32 ga; // no alignment concern
	RMuint64 byteoffset;
	RMuint32 size;
	
	// updated if: request()==OK, previous operation was a read, and input value is 0.
	RMuint32 biterrors;

	// if non NULL, must be sized to (size>>l2ps) *4 
	// updated if request()==OK and autoincremented
	RMuint32 *vameta;

	/*
	  For vabb:

          if non NULL, 
	  must be sized to (size>>l2ps) *2 (l2ps=9)
          must be sized to (size>>l2ps) *6 (l2ps=11)
          must be sized to (size>>l2ps) *1 (l2ps=12)

	  updated if request()==OK and autoincremented

	  For vareport:
	  if non NULL,
	  must be sized to the max of requested packets, autoincremented.
	*/
	char *vabb,*vareport;

	/*
	  byte amount to skip between pages, and at which offset to write to
	  normal cases use 0 and 0.

	  Now if you have two similar 4KB page devices on cs#0 and cs#1, you may want to stripe the storage
	  and use stride 4096 and offset 0 resp 4096. Proceeding so you read twice faster.
	*/
	RMuint32 stride;

	RMuint32 crc32,crc32extent; // input in trigger, output if request()==OK. User must set to -1 before a crc32 computation campaign

	//// private fields
	RMuint32 programmedsize;
	struct mlc_cfg vch;
};

#define WITHSPARE(x) ( (RMuint64)(x) + srl64to64_le32((RMuint64)((x)*pj->spareper128),7) )

#define PACKETSPERPAGE (1<<(pj->l2ps-PACKET_SHIFT))

#define PACKET_SHIFT 9

#define MAXCORRECTABLEBITSPERPACKET(pj) (	\
(pj->scheme==0)?ECCCAPA0:			\
(pj->scheme==1)?ECCCAPA1:			\
(pj->scheme==2)?ECCCAPA2:			\
(pj->scheme==3)?ECCCAPA3:			\
(pj->scheme==4)?ECCCAPA4:			\
-1)

// Requests usage of the given channel: RM_OK (now it's yours), RM_ACCESS_VIOLATION (not yours), RM_BUSY (working for you)
// Use whoami GBUS_MUTEX_CPU if you're the cpu
RMstatus mlc_pa_request(struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami);

// Give back the resource
void mlc_pa_yield(struct gbus *pgbus,struct mlc_pa_job *pj);

/// 
/**
   You know what cs# you're considering, you know your driver is idle on it,
   you ran mlc_pa_request() with success.
   And you know the mbus interface is idle.

   Now you want to trigger a transfer.

   This function will do as much as possible in one DMA shot.

   The programmed transfer size is returned by the function.
   buf, byteoffset, size come back updated accordingly.

   All offsets and sizes can be odd.
   
   The vameta and vabb arrays must be NULL.

   When programming it for even page sizes (e.g. 528, 1056, or 4224), 
   it makes more sense to store ga as a ``struct page1 *'' for easier decapsulation
   but this is left to the caller.

   Since it's uncorrected, don't be surprised if you don't read twice the same thing.

   If you write, of course, you paid attention to erase enough around beforehand.
*/
int mlc_pa_trigger_rawread (struct gbus *pgbus,struct mlc_pa_job *pj);
int mlc_pa_trigger_rawwrite(struct gbus *pgbus,struct mlc_pa_job *pj);

/// 
/**
   This is a bizarre function that takes raw sizes and offsets (e.g. including spare)
   checks it matches an integral block# and returns erased byte count (including spare).

   The vameta and vabb arrays must be NULL.
   Can only process one block at a time.

   Use WITHSPARE() macro, or
   mlc_pa_trigger_eccblockerase() is probably more natural to use and more adapted in most cases.

   Be careful though: asuming l2blocksize=17, 
    the raw offset 0x420000 is the start of block #32 (0x420000/0x21000).
    computation 0x420000>>17 yields 33 and is incorrect.

   @param pgbus 
   @param pj    
   @return -ReturnValue-
*/
int mlc_pa_trigger_rawblockerase(struct gbus *pgbus,struct mlc_pa_job *pj);

/// 
/**
   You know what cs# you're considering, you know your driver is idle on it,
   you ran mlc_pa_request() with success.
   And you know the mbus interface is idle.

   This function will do as much as possible in one DMA shot.

   The programmed transfer size is returned by the function.
   ga, byteoffset, size come back updated accordingly.

   If byteoffset, size are not an integral page count, returns -1.

   By design this function can't reach `unused' bytes (use rawread/rawwrite).

   If you write, of course, you paid attention to erase enough around beforehand.
*/
int mlc_pa_trigger_eccread (struct gbus *pgbus,struct mlc_pa_job *pj);
int mlc_pa_trigger_eccwrite(struct gbus *pgbus,struct mlc_pa_job *pj);

/// 
/**
   Same as mlc_pa_trigger_rawblockerase() but operating
   on ecc'd byte offset, count

   The vameta and vabb arrays must be NULL.
   Can only process one block at a time.

   The sync* variant can do multiple but it will poll.

   @param pgbus 
   @param pj    
   @return -ReturnValue-
*/
int mlc_pa_trigger_eccblockerase(struct gbus *pgbus,struct mlc_pa_job *pj);

/// 
/**
   ok, mlc_pa_trigger_eccread() was meant for integral pages.
   now you'd like to be able to read sparse amounts without reading the whole stuff and trashing.

   The vameta and vabb arrays must be NULL.

   This function will do little by little with unefficient dma shots _and busy polling_

   All offsets and sizes can be odd.
*/
int mlc_pa_trigger_sparseeccread(struct gbus *pgbus,struct mlc_pa_job *pj);

/*
  The noddr variants WILL HIJACK THE OTHER's channel. No check from software, use with care. Meant to be used when ddr is not setup yet (bootloaders)
  Per hardware design you cannot eccwrite without doing DMA.

  The CRC32 hardware computation also, won't work.
 */
int mlc_pa_trigger_sparseeccread_noddr(struct gbus *pgbus,struct mlc_pa_job *pj);

/// 
/**
   This function probably will probably rely on code that implements
   a decapsulation of the jdec id to get the figures and set timings etc.

   As such it has little to do with the MLC hardware since you have to use 
   the 0x20000/4/8 interface to get that info.
   But it's a good idea to enrich the `MLC supported part database' inside this library.

   User is supposed to input a zero'd out pj excepted for those fields:
   pj-> : cs, l2ps, device_cfg, timing1, timing2.

   Software NOT PART OF THE LIBRARY might decapsulate the JDEC ID to give sensible values to

   pj-> : scheme, spareper128, l2bs, l2ws

   Until all mentioned fields have non-dummy values, the rest of the library will NOT work.
*/
RMstatus mlc_pa_probe(struct gbus *pgbus,RMuint32 f_sys,struct mlc_pa_job *pj);

/*
  the following are synchronous wrappers. 
  they will busy poll and never short read or short write in normal cases.

  Repeated shots of 16 pages max. So vameta should be sized 64 bytes and 
  vabb 6*16 = 96 bytes.

  vareport should be sized by max number of packets.

  rawread() rawwrite() eccread() support odd byteoffset and size.
    
  the eccwrite() will return error if non-entire-page byteoffset or size
  the eccerase() will return error if non-entire-block byteoffset or size
*/
#define MLC_PA_SYNC_VAMETA_SIZE 64
#define MLC_PA_SYNC_VABB_SIZE 96
#define MLC_PA_SYNC_VAREPORT_SIZE 128
RMstatus mlc_pa_sync_rawread (struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami);
RMstatus mlc_pa_sync_rawwrite(struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami);
int mlc_pa_sync_eccread (struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami);
int mlc_pa_sync_eccread_noddr (struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami);
RMstatus mlc_pa_sync_eccwrite(struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami);
RMstatus mlc_pa_sync_eccblockerase(struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami);

typedef RMbool (*isbb)(struct gbus *pgbus,struct mlc_pa_job *_pj,RMuint32 cs,RMuint32 block,RMuint32 gascratch);
RMbool isbb_simplistic(struct gbus *pgbus,struct mlc_pa_job *_pj,RMuint32 cs,RMuint32 block,RMuint32 gascratch);

// take a bad-block-aware block# and return the physical block# (obviously the latter is >=)
int bba2p(struct gbus *pgbus,struct mlc_pa_job *pj,isbb f,RMuint32 cs,RMuint32 gascratch,RMuint32 bbablock);
int bbaeccread_PBLOCK(struct gbus *pgbus,struct mlc_pa_job *pj,isbb f,RMuint32 cs,RMuint32 ga,RMuint32 pblock,RMuint32 blockcount);

#endif /* __MLC_PA_H__ */
