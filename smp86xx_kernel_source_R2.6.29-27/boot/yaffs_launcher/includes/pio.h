/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   pio.h
  @brief  

  -long description-

  @author Emmanuel Michon
  @date   2007-02-13
*/

#ifndef __PIO_H__
#define __PIO_H__

struct gbus;

extern RMascii rom__fpublic[];
extern RMascii rom_etext[];

/******** THE FOLLOWING ACTUALLY IMPLEMENTED IN flow/xboot2.S *******/
/******** THE FOLLOWING ACTUALLY IMPLEMENTED IN flow/xboot2.S *******/
/******** THE FOLLOWING ACTUALLY IMPLEMENTED IN flow/xboot2.S *******/

/*
  start must be set to 0x80000000+cachesizeinbytes-0x10.

  Ex: xpu d-cache: 0x80002ff0
 */
void icache_i(RMuint32 start);
void dcache_wi(RMuint32 start);

/* must be run from uncached location... return i-cache size in bytes */
RMuint32 icacheinit(void);
/* returns d-cache size in bytes */
RMuint32 dcacheinit(void);
/* returns tlb size in entrypairs */
RMuint32 tlbinit(void);

void setentryhi(RMuint32 saved);

RMuint32 getentryhi(void);

int getasid(void);

/**
   Because you may want to access data immediately (cache i / wb), typical usage is:

   {
   RMuint32 keep=getentryhi();
   createtlbentry(...)
   access data
   getentryhi(keep);
   }

   Entries are created valid (TLB_V)
		
   No check on parameters.

   @param entry   in 0..TLB_PAIRS-1
   @param va      must be 1<<pgshift aligned 
   @param pa      must be 1<<pgshift aligned
   @param pgshift in 12..29 (4KB to 512MB) (if even, one entry in the pair will be invalid)
   @param asid    ignored if TLB_G (4KE SUM table 5-5)
   @param attr    cacheability: TLB_UNCACHED/TLB_WBACK, writable: TLB_D, ignore asid: TLB_G
*/
void createtlbentry(RMuint32 entry,RMuint32 va,RMuint32 pa,RMuint32 pgshift,RMuint32 asid,RMuint32 attr);

/**
   Find va+asid in tlb, deduce which entry in the pair we are targeting with va,
   then set the bit(field) to the value provided

   No error check if no entry match (failure on tlbp)

   The call restores C0_ENTRYHI
   
   Ex:
   vasetbit(0x10018030,0,TLB_D,0);
   vasetbit(0x1001c050,0,TLB_CMASK,TLB_WBACK);

   @param va    
   @param asid  
   @param bit   
   @param value	
*/
void vasetbit(RMuint32 va,RMuint32 asid,RMuint32 bit,RMuint32 value);

/**
   Find va+asid in tlb, deduce which entry in the pair we are targeting with va,
   then return entrylo --- with the attr bits (4KE figure 5-3)

   The call restores C0_ENTRYHI

   @param va    
   @param asid  
   @return -ReturnValue-
*/
RMuint32 vagetentrylo(RMuint32 va,RMuint32 asid);

/*
  Returns val+pc
  Needed for PIC implementation
 */
RMuint32 addpc(RMuint32 val);

/*
  Read C0_COUNT. Useful for mips16 caller
 */
RMuint32 getcount(void);

/*
  Read C0_RANDOM. Useful for mips16 caller. 
 */
RMuint32 getpoorrandom(void);

/******** THE FOLLOWING ACTUALLY IMPLEMENTED IN flow/xboot2.S *******/
/******** THE FOLLOWING ACTUALLY IMPLEMENTED IN flow/xboot2.S *******/
/******** THE FOLLOWING ACTUALLY IMPLEMENTED IN flow/xboot2.S *****DONE**/

/* in pio_asm.S */

void init_remap(struct gbus *pgbus);

/*
  Policy:

  the remap registers remap2, remap3, remap4 cannot be used directly anymore.
  We will use them like `semaphores' where the meaning is: zero, free, and
  non-zero, in use.

  We use read-modify-write primitives of the MIPS for this purpose: ll/sc (see mips run 8.5.2)

  This prevents to use them to remap close to gbus address 0. 
  Accessing there is never an issue anyways, does not require remapping.

  This policy does not apply to remap`0' (defined as CPU_remap) and remap1.

  ***

  You must run init_remap() at first so get_remap/release_remap work as specified here.

  ***

  This function is meant for cpu, ipu, xpu.
  
  This function supposes you have an uncached view on 
  the gbus range 0x00000000..0x1fffffff (512MB, 29bits)
  using a base address UB where access is simply sw/lw reg, 0(UB|ga).

  This is the case with access thru kseg1 (UB=0xa0000000) and xpu-style static MMU entry for gbus access (UB=0).
  kseg1 mapping is 512MB which is more than the required for this function.
  
  You need to indicate as pgbus how you access the remap registers with sw. UB will be deduced from pgbus.
  Ex. when using kseg1 and the cpu, pgbus=(struct gbus *)0xa006f000

  ***

  if ga < 64MB 

  return UB|ga
  
  else
  
  find a remap register, set proper value (because ga is >= 64MB, it becomes reserved),
  and return the remapped adress.

  IN OTHER WORDS, SAFE INTERRUPT WISE UP TO THREE LEVELS OF NESTING

  Notes:

  - may loop infinitely if remap1 and 2 and 3 and 4 are taken already.

  - since `sc' will fail when load-store consistency `might be bad', this function may be undeterministic in remap register choice.
	
  - since this function has an underlying side-effect a reserving a resource, it is not suitable for
    usage in a macro. Must always be paired with release_remap().

  - in the ROM, gbus_write_uint32() etc use these functions

  - to move larger areas, use memcpy_g2g _g2v _v2g. In this case, you must enforce non-interruption yourself

  - if you know that a buffer does not overlap 64MB boundaries, then you may use the facility this way so that
    the MIPS accesses data directly (here, for printf and software SHA-256, through kseg0):

  {
        RMuint32 _ba=get_remap(pgbus,(RMuint32)src_dram);

	RMDBGLOG((LOCALDBG,"%z\n",RM_PA_TO_KVA0(_ba),size[0]));
	mark0=RMmfc0(RM_C0_COUNT,0);
	sha256_init(&c);
	sha256_input(&c,RM_PA_TO_KVA0(_ba),size[0]);
	sha256_done(&c);
	mark1=RMmfc0(RM_C0_COUNT,0);

	release_remap(pgbus,_ba);
  }
 */
RMuint32 get_remap(struct gbus *pgbus,RMuint32 ga);

/*
  Release an address obtained by previous function
 */
void release_remap(struct gbus *pgbus,RMuint32 remapped_ga);

RMuint32 remapped_to_ga(struct gbus *pgbus,RMuint32 remapped_ga);

/* rb is the uart reg base. Ex. RM_PA_TO_KVA1(REG_BASE_cpu_block+CPU_UART0_base). Inits to 1152008N1 */
void uart_init(RMuint32 rb);
void uart_tx(RMuint32 rb,RMuint32 c);
RMuint32 uart_rx(RMuint32 rb);

/* s must be allocated to n bytes; no zero-marker written */
void uart_rxs(RMuint32 rb,RMuint8 *s,RMuint32 n);
/* 
   s must be allocated to n bytes; no zero-marker written; 
   Ctrl-H and Enter supported. Enter (0xd) will be part of the answer 

   Should be safe from buffer overflow pov but if Ctrl-H is the first char input it will be s[0]
*/
RMuint32 uart_rxsed(RMuint32 rb,RMuint8 *s,RMuint32 n);

/*
  translate unsigned x to lowercase radix rdx in buf, reverse order of natural reading; return number of digits
  0 translates to '0'
  does not append final \0
  buf must be allocated by user depending on rdx. Ex 8 bytes for worst case rdx=16. 32 for rdx=1, etc
*/
RMuint32 u2rdxrev(RMuint32 x,RMuint8 *buf,RMuint32 rdx);

/*
  translate null-terminated string to unsigned number in radix rdx.
  empty string translates to zero.
  characters before '0', resp between '9' and 'a', resp after 'z' will garbage output with -1, resp -2, resp -3.
 */
RMuint32 rdx2u(RMuint8 *s,RMuint32 rdx);

/*
  uncached data execute one 32bit instruction insn. Typically insn returns something useful in $v0.

  a.k.a. self-modifying code

  Ex 40020800        mfc0    $v0,$random
  {
    RMuint32 ud[2];
    RMuint32 q=udx(0x40020800,ud);
  }

  unpredictable results if ud is i-cached or in a location that cannot be fetched (d-cache)
  Do not change $ra in insn; also the executed instruction cannot be a branch

  Function should work cached if ud[] buffer does not overlap a d-cache line (32bytes on 74K)
 */
RMuint32 udx(RMuint32 insn,RMuint32 *ud);

/*
  write val to ad, when ad is a location that is not byte-enabled
  (e.g. read the 32bit word around it, mask, or, write 32bit)
 */
void bew(RMuint8 *ad,RMuint8 val); 

/*
  read byte per byte from i-cache
 */
RMuint8 icacherd_4k(RMuint8 *ad); 

/*
  same as bew in i-cache
 */
void ibew_4k(RMuint8 *ad,RMuint8 val);

RMuint8 icacherd_24k(RMuint8 *ad); 
void ibew_24k(RMuint8 *ad,RMuint8 val); 

RMuint32 sprset_isp_read_uint32(RMuint32 ad);
void sprset_isp_write_uint32(RMuint32 ad,RMuint32 val);

void *memset_v(void *s, int c, RMuint32 n);

/* 
   THIS FUNCTION IS 32BITWORD ORIENTED. All others are essentially byte oriented.
   It comes handy to work around registers with inverted addressing.

   dest[0]=src[n-1]
   dest[1]=src[n-2]
   ...
   dest[n-1]=src[0]
 */
void RMuint32_memcpy_rev(RMuint32 *dest, const RMuint32 *src, RMuint32 n /*in 32bit words!!!*/);

int memcmp_vv(const void *s0, const void *s1, RMuint32 n); 
RMuint32 strnlen_v(const RMascii *s, RMuint32 maxlen);
int strncmp_vv(const RMascii *s0, const RMascii *s1, RMuint32 n);

RMuint32 dmaparm(RMuint32 ad,RMbool isOutput); 

// careful, all the 8654/44/46 still suffer this (not 8652/56)
#define RMu64divu32 RMu64divu32_9831_10362
#define RMu64_u64divu64 RMu64_u64divu64_9831_10362

/*
  save to give r=NULL if remainder is of no interest

  bug#9831: if divisor=1, *r is not updated. Use something like this to fix it
  #define RMu64divu32_(x,r,y) ({ if ((r)!=0) *(r)=0; RMu64divu32(x,r,y); })

 */
RMuint64 RMu64divu32(RMuint64 dividend,RMuint32 *r,RMuint32 divisor);

/*
  save to give pr=NULL if remainder is of no interest

  bug#9831: if y=1, *pr is not updated. Use something like this to fix it
 */
RMuint64 RMu64_u64divu64(RMuint64 x,RMuint64 y,RMuint64 *pr);

static inline RMuint64 RMu64divu32_substitute(RMuint64 dividend,RMuint32 *r,RMuint32 divisor)
{
	RMuint64 v;
	RMuint16 _r[2];
	// yes this syntax assumes optimistic alignment and endianness. Within mips LE is fine. em 2010may6
	RMuint64div32((void *)&dividend,(void *)&divisor,(void *)&v,_r);
	if (r) 
		*r=RMleBufToUint32((void *)_r);
	return v;
}

/* in pio.c */

/*
  Works for any src / dst / n alignment, byte oriented, and optimized for speed in the core loop.

  If src, dst can sustain the rate (src should be cached), 
  you can expect 266MByte/s (222MByte/s when not aligned) on a 333MHz processor

dsp2dsp 1.25clk/byte
  5c:   8c480000        lw      $t0,0($v0)
  60:   24420004        addiu   $v0,$v0,4
  64:   24630004        addiu   $v1,$v1,4
  68:   144afffc        bne     $v0,$t2,5c <memcpy_v2v+0x5c>
  6c:   ac68fffc        sw      $t0,-4($v1)

udsp2dsp 1.5clk/byte
  80:   88480003        lwl     $t0,3($v0)
  84:   24630004        addiu   $v1,$v1,4
  88:   24420004        addiu   $v0,$v0,4
  8c:   9848fffc        lwr     $t0,-4($v0)
  90:   144afffb        bne     $v0,$t2,80 <memcpy_v2v+0x80>
  94:   ac68fffc        sw      $t0,-4($v1)
*/
void *memcpy_v2v(RMuint8 *dst, const RMuint8 *src, RMuint32 n);

/* 
   gbus to gbus, must provide remap0 address
   NON SAFE IF INTERRUPTED
   ASSUMES UB is 512MB wide (e.g. as wide as kseg1)

   Ex.
   dst_dram=(RMuint8 *)0x97fffffd; // dc0 +128M -3bytes
   src_dram=(RMuint8 *)0xdffffffa; // dc1 end -6bytes 
   memcpy_g2g(pgbus,dst_dram,src_dram,16);
   
   does:

memcpy_g2g 0x97fffffd<-0xdffffffa 16
distance to dstb 3
distance to srcb 6
memcpy_v2v 0xa000fffd<-0xbfc0fffa 3
 3s
 2s
 1s
distance to dstb 65536
distance to srcb 3
memcpy_v2v 0xa0000000<-0xbfc0fffd 3
 3f
 2f
 1f
distance to dstb 65533
distance to srcb 65536
memcpy_v2v 0xa0000003<-0xbfc00000 10
 10s
 9u
 5u
 1f

   trace:

 >  MID=2  97fffffd -> [000ef004] 
 >  MID=2  dffffffa -> [000ef000] 
 >  MID=2  ..xx.... <- [dffffff8] 
 >  MID=2  ....xx.. -> [97fffffc] 
 >  MID=2  xx...... <- [dffffff8] 
 >  MID=2  ..xx.... -> [97fffffc] 
 >  MID=2  ......xx <- [dffffffc] 
 >  MID=2  xx...... -> [97fffffc]
 
 >  MID=2  98000000 -> [000ef004] 
 >  MID=2  dffffffd -> [000ef000] 
 >  MID=2  ....xx.. <- [dffffffc] 
 >  MID=2  ......xx -> [98000000] 
 >  MID=2  ..xx.... <- [dffffffc] 
 >  MID=2  ....xx.. -> [98000000] 
 >  MID=2  xx...... <- [dffffffc] 
 >  MID=2  ..xx.... -> [98000000] 

 >  MID=2  98000003 -> [000ef004] 
 >  MID=2  e0000000 -> [000ef000] 
 >  MID=2  ......xx <- [e0000000] 
 >  MID=2  xx...... -> [98000000] 
 >  MID=2  ......xx <- [e0000004] 
 >  MID=2  xxxxxxxx <- [e0000000] 
 >  MID=2  xxxxxxxx -> [98000004] 
 >  MID=2  ......xx <- [e0000008] 
 >  MID=2  xxxxxxxx <- [e0000004] 
 >  MID=2  xxxxxxxx -> [98000008] 
 >  MID=2  ....xx.. <- [e0000008] 
 >  MID=2  ......xx -> [9800000c] 

*/
void *memcpy_g2g(struct gbus *pgbus,RMuint8 *dst, const RMuint8 *src, RMuint32 n);

/*
  g2v: gbus to virtual, must provide remap0 address
  v2g: gbus to virtual, must provide remap0 address

  NON SAFE IF INTERRUPTED
  ASSUMES UB is 512MB wide (e.g. as wide as kseg1)

  does:

memcpy_g2v 0xa0300001<-0xdffffffa 16
distance to srcb 6
memcpy_v2v 0xa0300001<-0xbfc0fffa 6
 6s
 5s
 4s
 3f
 2f
 1f
distance to srcb 65536
memcpy_v2v 0xa0300007<-0xbfc00000 10
 10s
 9u
 5u
 1f

 >  MID=2  dffffffa -> [000ef000] 
 >  MID=2  ..xx.... <- [dffffff8] 
 >  MID=2  ....xx.. -> [00300000] 
 >  MID=2  xx...... <- [dffffff8] 
 >  MID=2  ..xx.... -> [00300000] 
 >  MID=2  ......xx <- [dffffffc] 
 >  MID=2  xx...... -> [00300000] 
 >  MID=2  ....xx.. <- [dffffffc] 
 >  MID=2  ......xx -> [00300004] 
 >  MID=2  ..xx.... <- [dffffffc] 
 >  MID=2  ....xx.. -> [00300004] 
 >  MID=2  xx...... <- [dffffffc] 
 >  MID=2  ..xx.... -> [00300004] 

 >  MID=2  e0000000 -> [000ef000] 
 >  MID=2  ......xx <- [e0000000] 
 >  MID=2  xx...... -> [00300004] 
 >  MID=2  ......xx <- [e0000004] 
 >  MID=2  xxxxxxxx <- [e0000000] 
 >  MID=2  xxxxxxxx -> [00300008] 
 >  MID=2  ......xx <- [e0000008] 
 >  MID=2  xxxxxxxx <- [e0000004] 
 >  MID=2  xxxxxxxx -> [0030000c] 
 >  MID=2  ....xx.. <- [e0000008] 
 >  MID=2  ......xx -> [00300010] 

memcpy_v2g 0x97fffffd<-0xa0300001 16
distance to dstb 3
memcpy_v2v 0xa000fffd<-0xa0300001 3
 3s
 2s
 1s
distance to dstb 65536
memcpy_v2v 0xa0000000<-0xa0300004 13
 13a
 9a
 5a
 1f

 >  MID=2  97fffffd -> [000ef004] 
 >  MID=2  ....00.. <- [00300000] 
 >  MID=2  ....00.. -> [97fffffc] 
 >  MID=2  ..00.... <- [00300000] 
 >  MID=2  ..00.... -> [97fffffc] 
 >  MID=2  00...... <- [00300000] 
 >  MID=2  00...... -> [97fffffc]
 
 >  MID=2  98000000 -> [000ef004] 
 >  MID=2  00000000 <- [00300004] 
 >  MID=2  00000000 -> [98000000] 
 >  MID=2  00000000 <- [00300008] 
 >  MID=2  00000000 -> [98000004] 
 >  MID=2  00000000 <- [0030000c] 
 >  MID=2  00000000 -> [98000008] 
 >  MID=2  ......00 <- [00300010] 
 >  MID=2  ......00 -> [9800000c] 
 */
void *memcpy_g2v(struct gbus *pgbus,RMuint8 *dst, const RMuint8 *src, RMuint32 n);
void *memcpy_v2g(struct gbus *pgbus,RMuint8 *dst, const RMuint8 *src, RMuint32 n);

void dcache_range_wi_cs(RMuint32 address,RMuint32 size,RMuint32 cachesize);

extern const RMuint32 jub[8];

#define XLOAD_CERTTYPE_ZBOOT       0
#define XLOAD_CERTTYPE_CPU         1
#define XLOAD_CERTTYPE_XTASK1      2
#define XLOAD_CERTTYPE_UCODE_VIDEO 3
#define XLOAD_CERTTYPE_UCODE_AUDIO 4
#define XLOAD_CERTTYPE_UCODE_DEMUX 5
#define XLOAD_CERTTYPE_IH          6
#define XLOAD_CERTTYPE_XTASK2      7
#define XLOAD_CERTTYPE_XTASK3      8
#define XLOAD_CERTTYPE_XTASK4      9
#define XLOAD_CERTTYPE_XOSU     0xff

#define XLOAD_SEKID_MAX 14
#define XLOAD_SEKID_CLEAR 0xff

#define XLOAD_CLEAR_HEADERSIZE 772
#define XLOAD_CIPHERED_HEADERSIZE 1044
#define XLOAD_MAXSIZE (20*1024*1024)

#define DSPCOUNT_865X 4
#define DSPCOUNT_864X 6

#ifdef XBOOT2_SMP865X
extern const struct dgpriv_s {
	RMuint16 stepcert,rb,pb,dbBROKEN,ps,ds;
} dgpriv_sixteen[DSPCOUNT_865X];
#else
extern const struct dgpriv_s {
	RMuint16 stepcert,rb,pb,dbBROKEN,ps,ds;
} dgpriv_sixteen[DSPCOUNT_864X];
#endif

__attribute__((mips16)) RMstatus ucode_load_microcode(
	struct gbus *pGBus,
	RMuint32 PMem_Base,  // GBus byte address of DSP's PMem block
	RMuint32 DMem_Base,  // GBus byte address of DSP's DMem block
	RMuint32 DRAM_Base,  // GBus byte address of DRAM memory range for this microcode
	RMuint32 DMem_Width, // DMem width (in bit or byte)
	RMuint8 *ucode_data, // the data of the microcode's .bin file
	RMuint32 ucode_size); // size of data in bytes

__attribute__((mips16)) void pmemdmem_clear_expose(struct gbus *pgbus);

__attribute__((mips16)) void ub_prot(struct gbus *pgbus);

__attribute__((mips16)) RMuint32 pllrandom_DONTUSE(struct gbus *pgbus,RMuint32 nbits);

struct inflate_ctx {
	unsigned b__,k__;
	RMascii *inbuf__;
	unsigned outcnt__; /* bytes in output buffer */
	RMascii *slide__; 
	unsigned bb__;                         /* bit buffer */
	unsigned bk__;                    /* bits in bit buffer */
	unsigned hufts__;         /* track memory usage */
	void *(*fake_malloc__)(struct inflate_ctx *ctx,RMint32 size);
	void (*fake_free__)(struct inflate_ctx *ctx,void *p);
	const RMuint8 *border;
	const RMuint16 *cplens;
	const RMuint8 *cplext;
	const RMuint16 *cpdist;
	const RMuint8 *cpdext;
	const RMuint16 *mask_bits;
	void *cookie0;
	void *cookie1;
};

void needbits(struct inflate_ctx *ctx,unsigned n); 
	     
__attribute__((mips16)) RMint32 inflate(struct inflate_ctx *ctx);

__attribute__((mips16)) void *heap_malloc(struct inflate_ctx *ctx,RMint32 size);
__attribute__((mips16)) void heap_free(struct inflate_ctx *ctx,void *p);

__attribute__((mips16)) RMint32 easy_inflate(RMascii **zinbuf,RMascii **outbuf, RMint32 *poutsize, RMascii *heap);

#endif /* __PIO_H__ */
