/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   rmmacros.h
  @brief  

  This file defines static inline function and macros.

  Macros may suffer name collision (in this case we use __hidden_rv for rv).
  Some macros have a good optimization in MIPS assembly.
  These optimizations are specific to 4KE (MIPS32 release 2) and little endian (tango2).

  The assembly optimizations are valid for mipsel-linux-gcc and sde-gcc, some of them 
  only for little endian (e.g. tangox), and only for 32bit mode.
  Since constrained __asm__ syntax is only for gcc, it makes sense there to also use ({ ... })
  http://gcc.gnu.org/onlinedocs/gcc-2.95.3/gcc_4.html#SEC62 (see RMALIGNTO/RMALIGNTOVar).

  All other cases implement static inline functions, or macros without ({ ... }),
  that are safe with Microsoft compiler.

  See test suite in llad_xtest/example.c

  @author Emmanuel Michon
  @date   2002-10-17
*/

#ifndef __RMMACROS_H__
#define __RMMACROS_H__

RM_EXTERN_C_BLOCKSTART

// Linux coding style chapter 17
#define RM_ARRAY_ENTRYCOUNT(x) (sizeof(x) / sizeof((x)[0]))
#define RM_BUILD_ASSERT(cond) do { (void) sizeof(RMuint32 [1 - 2*!(cond)]); } while(0)
#define RM_FIELD_OFFSET(x,y) ((RMuint32)(&((x *)NULL)->y))
#define RM_FIELD_SIZE(x,y) sizeof(((x *)NULL)->y)

#define FIELD64(f) (*(RMuint64 *)&(f##lo))

/**
   Standard min macro.
*/
#define RMmin(x,y) (((x) < (y)) ? (x) : (y))


/**
   Standard max macro.
*/
#define RMmax(x,y) (((x) < (y)) ? (y) : (x))

/**
   Standard abs macro.
 */
#define RMabs(x) (((x) >= 0) ? (x) : (-(x)))

/**
   sign macro: returns +1 if positive, -1 if negative
 */


#define RMsign(x) (((x) >= 0) ? (1) : (-1))


/// 
/**
   @param l 64 bit unsigned integer.
   @return the lowest 32 bits from a 64 bits long unsigned integer.
*/
#define RMuint32LSBfromRMuint64(l)     ( (RMuint32)   (l)                      )

/// 
/**
   @param l 64 bit unsigned integer.
   @return the highest 32 bits from a 64 bits long unsigned integer.
*/
#define RMuint32MSBfromRMuint64(l)     ( (RMuint32) ( ((l)>>32) & 0xffffffff ) )

/// 
/**
   @param MSB high-order 32 bits.
   @param LSB low-order 32 bits.
   @return the 64 bits unsigned integer such that its 32 high-order bits are MSB and
           its 32 low-order bits are LSB.
*/
#define RMuint64from2RMuint32(MSB,LSB) ( (RMuint64) ( (((RMuint64)(MSB))<<32) + ((RMuint64)(LSB)) )      )

/// 
/**
   @param l 32 bit unsigned integer.
   @param n 32 bit unsigned integer.
   @return 64 bits unsigned integer resulting from left shift of l.
*/
#define RMuint64fromRMuint32ShiftLeft(l,n)  ( ((RMuint64)(l))<<(n) )

/// 
/**
   @param l 64 bit unsigned integer.
   @param n 32 bit unsigned integer.
   @return 32 bits unsigned integer resulting from right shift of l.
*/
#define RMuint32fromRMuint64ShiftRight(l,n)  ( (RMuint32) ((RMuint64)(l)>>(n)) )

/**
   the two following won't call gcc intrinsics
*/
#define srl64to64_le32(x,s)					\
	({							\
		RMuint32 rhi=RMuint32MSBfromRMuint64(x),	\
			rlo=RMuint32LSBfromRMuint64(x);	\
								\
		rlo>>=(s);	\
		rlo|=rhi<<(32-(s)); \
		rhi>>=(s);	\
								\
		RMuint64from2RMuint32(rhi,rlo);			\
	})

#define sll64to64_le32(x,s)					\
	({							\
		RMuint32 rhi=RMuint32MSBfromRMuint64(x),	\
			rlo=RMuint32LSBfromRMuint64(x);	\
								\
		rhi<<=(s);	\
		rhi|=rlo>>(32-(s)); \
		rlo<<=(s);	\
								\
		RMuint64from2RMuint32(rhi,rlo);			\
	})

/**
   --------------- e.m. use this one! --------------------

   Insert the bits 0..bits-1 from data starting at bit number `shift' in *ptr, 
   replacing whatever was there.
   
   In the (rare) cases where bits or shift are variable arguments (not
   known at compile time), use RMinsShiftBitsVar.

   @param ptr: pointer to RMuint32 to be manipulated
   @param data: source data
   @param bits: number of bits to use from data
   @param shift: number of bits to shift data by to the left
   @return the new value of *ptr
*/
#if RMCOMPILERID==RMCOMPILERID_MIPSEL_GCC & !defined(XOS_COMPACT)
#define RMinsShiftBits(ptr, data, bits, shift) \
do __asm__ __volatile__( \
	"ins %0, %1, " STR(shift) ", " STR(bits) \
	: "=r" (*(ptr)) /* may be same reg */ \
	: "r" (data), "0" (*(ptr))); while (0)
#else
#define RMinsShiftBits RMinsShiftBitsVar
#endif

#define RMinsShiftBitsVar(ptr, data, bits, shift) \
	*(ptr) = ( \
		((*(ptr)) & (~(((1 << (bits)) - 1) << (shift)))) | \
		(((RMuint32)(data) & ((1 << (bits)) - 1)) << (shift)) \
	)

#define RMinsShiftBool(ptr, val, shift) RMinsShiftBits(ptr, (val) ? 1 : 0, 1, shift)
#define RMinsShiftBoolVar(ptr, val, shift) RMinsShiftBitsVar(ptr, (val) ? 1 : 0, 1, shift)

// e.m. deprecated --- use RMinsShiftBitsVar()
#define RMsetConsecutiveBitsVar(target,begin,end,value) RMinsShiftBitsVar(target, value, (end)-(begin)+1, begin)

#ifndef STR
#define __STR(x) #x
#define STR(x) __STR(x)
#endif /* STR */

// e.m. deprecated --- use RMinsShiftBits()
#if RMCOMPILERID==RMCOMPILERID_MIPSEL_GCC & !defined(XOS_COMPACT)

#define RMsetConsecutiveBits(target,begin,end,value)			\
do __asm__ __volatile__(							\
		     "ins %0, %1, " STR(begin) ", (" STR(end) "-(" STR(begin) ")+1)"	\
		     : "=r" (*(target)) /* may be same reg */		\
		     : "r" (value), "0" (*(target))); while (0)

#define RMclearBits(ptr, bits, shift)				\
do __asm__ __volatile__(					\
		     "ins %0, $0, " STR(shift) ", " STR(bits)	\
		     : "=r" (*(ptr)) /* may be same reg */ 	\
		     : "0" (*(ptr))); while (0)

#else
#define RMsetConsecutiveBits RMsetConsecutiveBitsVar
#define RMclearBits(ptr, bits, shift) RMsetConsecutiveBits(ptr,shift,(shift)+(bits)-1,0)
#endif

// e.m. deprecated --- use RMinsShiftBits()
#define RMshiftBits(data, bits, shift) ( \
        (((RMuint32)(data)) & ((1 << (bits)) - 1)) << (shift) )

// e.m. deprecated --- use RMinsShiftBool()
#define RMshiftBool(data, shift) ( \
        (RMuint32)((data) ? (1 << (shift)) : 0) )

/**
   --------------- e.m. use this one! --------------------

   Bit manipulation operation.

   [RMunshiftBits(data, 3, 9) maps directly in mips to `ext $y, $x, 9, 3']

   In the (rare) cases where bits or shift are variable arguments (not
   known at compile time), use RMinsShiftBitsVar.

   Use of __hidden_rv is to make very unlikely one of the provided variables has the same name. This is a trick,
   but Linux kernel headers do no better for this (see uaccess.h).

   @param data source data.
   @param bits: number of bits to use from data
   @param shift: number bits to shift data by to the right
   @return 'bits' bits from position 'shift' of 'data'.
   @remark for example, if 'data' is '00110100', RMunshiftBits(data, 3, 2) returns '101'
*/
#if RMCOMPILERID==RMCOMPILERID_MIPSEL_GCC & !defined(XOS_COMPACT)
#define RMunshiftBits(data, bits, shift)				\
({									\
	RMuint32 __hidden_rv;						\
									\
	__asm__ __volatile__(						\
			     "ext %0, %1, (" STR(shift) "), (" STR(bits)")"	\
			     : "=r" (__hidden_rv) /* may be same reg */	\
			     : "r" (data));				\
									\
	__hidden_rv;							\
})
#else
#define RMunshiftBits RMunshiftBitsVar
#endif

#define RMunshiftBitsVar(data, bits, shift) ( \
        (((RMuint32)(data)) >> (shift)) & ((1 << (bits)) - 1) )

// e.m. deprecated --- use RMunshiftBits()
#define RMunshiftBool(data, shift) RMunshiftBits(data, 1, shift)
// e.m. deprecated --- use RMunshiftBits()
#define RMgetConsecutiveBits(target,begin,end) RMunshiftBits(target, (end)-(begin)+1, begin)

/* 0x10111213 -> 0x13121110 */
#if RMCOMPILERID==RMCOMPILERID_MIPSEL_GCC & !defined(XOS_COMPACT)
#define RMswapBytesUint32(a)				\
({							\
	RMuint32 __hidden_rv;				\
							\
	__asm__ __volatile__(				\
			     "wsbh %0, %1\n"		\
			     "rotr %0, %0, 16\n"	\
			     : "=r" (__hidden_rv) /* may be same reg */ \
			     : "r" (a));		\
							\
	__hidden_rv;					\
})
#else

static inline RMuint32 RMswapBytesUint32(RMuint32 a)						
{									
	RMuint32 tmp=a;				
	
	tmp = (((tmp >> 8) & 0x00ff00ff) | ((tmp & 0x00ff00ff) << 8)); 
	tmp = ((tmp >> 16) | (tmp << 16));	
	
	return tmp;							
}

#endif

/* 0x10111213 -> 0x088848c8 */
static inline RMuint32 RMswapBitsInByteUint32(RMuint32 x)
{
	RMuint32 y = 0x55555555;
	x = (((x >> 1) & y) | ((x & y) << 1));
	y = 0x33333333;
	x = (((x >> 2) & y) | ((x & y) << 2));
	y = 0x0f0f0f0f;
	x = (((x >> 4) & y) | ((x & y) << 4));
	return x;
}

/* 0x10111213 -> 0xc8488808 */
static inline RMuint32 RMswapBitsUint32(RMuint32 x)
{
	return RMswapBytesUint32(RMswapBitsInByteUint32(x));
}

/**
   @param buf 4 bytes-long buffer.
   @return a RMuint32 from 4 bytes of a bigendian buffer
*/
static inline RMuint32 RMbeBufToUint32(const RMuint8 *buf)
{
#if RMCOMPILERID==RMCOMPILERID_MIPSEL_GCC & !defined(XOS_COMPACT) && RMENDIANNESS==RMLITTLEENDIAN
	RMuint32 rv;
	
	__asm__ __volatile__(
			     "ulw %0, 0(%1)"
			     : "=&r" (rv) /* may NOT be the same reg because ulw expands to two insn. Thanks Bertrand */
			     : "r" (buf));

	return RMswapBytesUint32(rv);
#else
	return (((RMuint32) buf[0] << 24) + \
		((RMuint32) buf[1] << 16) + \
		((RMuint32) buf[2] << 8) + \
		(RMuint32)  buf[3]);
#endif
}

/**
   @param buf 4 bytes-long buffer.
   @return a RMuint32 from 4 bytes of a littleendian buffer
*/
static inline RMuint32 RMleBufToUint32(const RMuint8 *buf)
{
#if RMCOMPILERID==RMCOMPILERID_MIPSEL_GCC & !defined(XOS_COMPACT) && RMENDIANNESS==RMLITTLEENDIAN
	RMuint32 rv;
	
	__asm__ __volatile__(
			     "ulw %0, 0(%1)"
			     : "=&r" (rv) /* may NOT be the same reg */
			     : "r" (buf));

	return rv;
#else
	return (((RMuint32) buf[3] << 24) + \
		((RMuint32) buf[2] << 16) + \
		((RMuint32) buf[1] << 8) + \
		(RMuint32)  buf[0]);
#endif
}

/**
   @param buf 8 bytes-long buffer.
   @return a RMuint64 from 8 bytes of a littleendian buffer
*/
static inline RMuint64 RMbeBufToUint64(const RMuint8 *buf)
{
#if RMCOMPILERID==RMCOMPILERID_MIPSEL_GCC & !defined(XOS_COMPACT) && RMENDIANNESS==RMLITTLEENDIAN
	RMuint32 rvhi,rvlo;
	
	__asm__ __volatile__(
			     "ulw %0, 0(%2)\n"
			     "ulw %1, 4(%2)\n"
			     : "=&r" (rvhi), "=&r" (rvlo)
			     : "r" (buf));

	rvhi=RMswapBytesUint32(rvhi);
	rvlo=RMswapBytesUint32(rvlo);
	return ((RMuint64)rvhi<<32)+(RMuint64)rvlo;
#else
	return (((RMuint64) buf[0] << 56) + \
		((RMuint64) buf[1] << 48) + \
		((RMuint64) buf[2] << 40) + \
		((RMuint64) buf[3] << 32) + \
		((RMuint64) buf[4] << 24) + \
		((RMuint64) buf[5] << 16) + \
		((RMuint64) buf[6] << 8) + \
		(RMuint64)  buf[7]);
#endif
}

/**
   @param buf 8 bytes-long buffer.
   @return a RMuint64 from 8 bytes of a littleendian buffer
*/
static inline RMuint64 RMleBufToUint64(const RMuint8 *buf)
{
#if RMCOMPILERID==RMCOMPILERID_MIPSEL_GCC & !defined(XOS_COMPACT) && RMENDIANNESS==RMLITTLEENDIAN
	RMuint32 rvhi,rvlo;
	
	__asm__ __volatile__(
			     "ulw %0, 0(%2)\n"
			     "ulw %1, 4(%2)\n"
			     : "=&r" (rvlo), "=&r" (rvhi)
			     : "r" (buf));

	return ((RMuint64)rvhi<<32)+(RMuint64)rvlo;
#else
	return (((RMuint64) buf[7] << 56) + \
		((RMuint64) buf[6] << 48) + \
		((RMuint64) buf[5] << 40) + \
		((RMuint64) buf[4] << 32) + \
		((RMuint64) buf[3] << 24) + \
		((RMuint64) buf[2] << 16) + \
		((RMuint64) buf[1] << 8) + \
		(RMuint64)  buf[0]);
#endif
}



/**
   Transforms a 32 bits integer into 4 bytes in big endian order

   @param val 
   @param buf   
*/
static inline void RMuint32ToBeBuf(RMuint32 val, RMuint8 *buf)
{
#if (defined WITH_32BIT_BUF_ALIGNMENT) && (RMPLATFORMID == RMPLATFORMID_JASPERMAMBO) 
        /* Standalone ARM platform : optimized swapping - as WMAPro lib uses this intensively now... */
        *(RMuint32 *) buf = ((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >>  8) |
                            ((val & 0x0000ff00) <<  8) | ((val & 0x000000ff) << 24); 
#else
	/* default other platforms */
	buf[0] = (RMuint8)(val >> 24) & 0xff;
	buf[1] = (RMuint8)(val >> 16) & 0xff;
	buf[2] = (RMuint8)(val >>  8) & 0xff;
	buf[3] = (RMuint8)val & 0xff;
#endif /* WITH_32BIT_BUF_ALIGNMENT && RMPLATFORMID == */ 
}

/**
   Transforms a 32 bits integer int o 4 bytes in little endian order

   @param val 
   @param buf   
*/
static inline void RMuint32ToLeBuf(RMuint32 val, RMuint8 *buf)
{
#if (defined WITH_32BIT_BUF_ALIGNMENT) && (RMPLATFORMID == RMPLATFORMID_JASPERMAMBO) 
	/* Standalone platform : optimized swapping - as WMAPro lib uses this intensively now... */
        *(RMuint32 *) buf = ((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >>  8) |
                            ((val & 0x0000ff00) <<  8) | ((val & 0x000000ff) << 24); 
#else 
	/* default other platforms */
	buf[3] = (RMuint8)(val >> 24);
	buf[2] = (RMuint8)(val >> 16);
	buf[1] = (RMuint8)(val >>  8);
	buf[0] = (RMuint8)val;
#endif /* WITH_32BIT_BUF_ALIGNMENT && RMPLATFORMID == */ 
}


/**
   Transforms a 64 bits integer int to 8 bytes in little endian order

   @param val 
   @param buf   
*/
static inline void RMuint64ToLeBuf(RMuint64 val, RMuint8 *buf)
{
	buf[7] = (RMuint8)(val >> 56);
	buf[6] = (RMuint8)(val >> 48);
	buf[5] = (RMuint8)(val >> 40);
	buf[4] = (RMuint8)(val >> 32);
	buf[3] = (RMuint8)(val >> 24);
	buf[2] = (RMuint8)(val >> 16);
	buf[1] = (RMuint8)(val >>  8);
	buf[0] = (RMuint8)val;

}
/**
   Transforms aarray of 16 bits integer into an RMuint64

   @param array 
   @param val   
*/

static inline void RMuint16ArrayToRMuint64(RMuint16 * array, RMuint64 *val)
{
	*val =  (RMuint64) (array[0])
	| (RMuint64)(array[1]) << 16
	| (RMuint64)(array[2]) << 32
	| (RMuint64)(array[3]) << 48;
}


/**
   Transforms a 64 bits integer int to 8 bytes in big endian order

   @param val 
   @param buf   
*/
static inline void RMuint64ToBeBuf(RMuint64 val, RMuint8 *buf)
{
	buf[0] = (RMuint8)(val >> 56) & 0xff;
	buf[1] = (RMuint8)(val >> 48) & 0xff;
	buf[2] = (RMuint8)(val >> 40) & 0xff;
	buf[3] = (RMuint8)(val >> 32) & 0xff;
	buf[4] = (RMuint8)(val >> 24) & 0xff;
	buf[5] = (RMuint8)(val >> 16) & 0xff;
	buf[6] = (RMuint8)(val >>  8) & 0xff;
	buf[7] = (RMuint8)val & 0xff;

}


/**
   @param buf 3 bytes-long buffer.
   @return a RMuint24 from 3 bytes of a bigendian buffer
*/
static inline RMuint32 RMbeBufToUint24(const RMuint8 *buf)
{
	return (((RMuint32) buf[0] << 16) + \
		((RMuint32) buf[1] << 8) + \
		(RMuint32) buf[2]);
}

/**
   @param buf 6 bytes-long buffer.
   @return a RMuint48 from 6 bytes of a bigendian buffer
*/
static inline RMuint64 RMbeBufToUint48(const RMuint8 *buf)
{
	return (((RMuint64) buf[0] << 40) + \
		((RMuint64) buf[1] << 32) + \
		((RMuint64) buf[2] << 24) + \
		((RMuint64) buf[3] << 16) + \
		((RMuint64) buf[4] << 8) + \
		(RMuint64) buf[5]);
}

/**
   @param buf 3 bytes-long buffer.
   @return a RMuint24 from 4 bytes of a littleendian buffer
*/
static inline RMuint32 RMleBufToUint24(const RMuint8 *buf)
{
	return (((RMuint32) buf[2] << 16) + \
		((RMuint32) buf[1] << 8) + \
		(RMuint32) buf[0]);
}

/**
   Transform a 24 bits integer (stored in a RMuint32) into 3 bytes in big endian order
   
   @param val
   @param buf
*/
static inline void RMuint24ToBeBuf(RMuint32 val, RMuint8 *buf)
{
	buf[0] = (RMuint8)(val >>  16);
	buf[1] = (RMuint8)(val >>  8);
	buf[2] = (RMuint8)val;
}

/**
   Transforms a 24 bits integer into 2 bytes in little endian order

   @param val 
   @param buf   
*/
static inline void RMuint24ToLeBuf(RMuint32 val, RMuint8 *buf)
{
	buf[2] = (RMuint8)(val >>  16);
	buf[1] = (RMuint8)(val >>  8);
	buf[0] = (RMuint8)val;
}

/**
   @param buf 2 bytes-long buffer.
   @return a RMuint16 from 2 bytes of a bigendian buffer
*/
static inline RMuint16 RMbeBufToUint16(const RMuint8 *buf)
{
	return (((RMuint16) buf[0] << 8) + \
		(RMuint16)  buf[1]);
}

/**
   @param buf 2 bytes-long buffer.
   @return a RMuint16 from 2 bytes of a littleendian buffer
*/
static inline RMuint16 RMleBufToUint16(RMuint8 *buf)
{
 	return 	(((RMuint16) buf[1] << 8) + \
		(RMuint16)  buf[0]);
}

/**
   Transforms a 16 bits integer into 2 bytes in big endian order

   @param val 
   @param buf   
*/
static inline void RMuint16ToBeBuf(RMuint16 val, RMuint8 *buf)
{
	buf[0] = (RMuint8)(val >>  8);
	buf[1] = (RMuint8)val;
}

/**
   Transforms a 16 bits integer into 2 bytes in little endian order

   @param val 
   @param buf   
*/
static inline void RMuint16ToLeBuf(RMuint16 val, RMuint8 *buf)
{
	buf[1] = (RMuint8)(val >>  8);
	buf[0] = (RMuint8)val;
}

// sometimes RMMemset is remapped on a complex operation. Rather use this to zero memory at location:
#define RMclearmemory(location,size) 				\
do {								\
	RMuint32 _rmmacros_i;							\
	for (_rmmacros_i=0;_rmmacros_i<(RMuint32)(size);_rmmacros_i++) *((RMuint8 *)(location)+_rmmacros_i)=0;	\
} while (0)

#define RMcopymemory(dest, src, size) \
do { \
	RMuint32 _rmmacros_i; \
	for (_rmmacros_i=0;_rmmacros_i<(RMuint32)(size);_rmmacros_i++) *((RMuint8 *)(dest)+_rmmacros_i)=*((RMuint8 *)(src)+_rmmacros_i); \
} while (0)

/* Bug# 8632 : Those are deprecated in favor of RMclearmemory and RMcopymemory
 * Keep them for backward compatibility until most places are cleaned up */
#define clearmemory RMclearmemory
#define copymemory RMcopymemory

// integer division a * b / c
#define RM64mult32div32(a,b,c) ( (RMuint64)(a) * (RMuint32)(b) / (RMuint32)(c) )

// rounded integer division a * b / c
#define RM64mult32divrnd32(a,b,c) ( ((RMuint64)(a) * (RMuint32)(b) + ((RMuint32)(c) >> 1)) / (RMuint32)(c) )

/* 

   Example
   RMALIGNTO(0x12345600,2) = 0x12345600 (prev 32bit word align)
   RMALIGNTO(0x12345601,2) = 0x12345600 (prev 32bit word align)
   RMALIGNTO(0x12345609,4) = 0x12345600 (prev 128bit word align)

   With the current implementation, the `Var' form is not different.
*/
#define RMALIGNTOVar(x,nbits) ((((RMuint32)(x))>>(nbits))<<(nbits))
#define RMALIGNTO(x,nbits) RMALIGNTOVar(x,nbits)

/*
  Alignment over shoot

  how many bytes you're from the boundary.
 */
// that was the previous implem. #define RMALIGNOS(x,nbits) ((x)-RMALIGNTO(x,nbits)) [typically x - ((x>>7)<<7))] but this is easier with 64bit: x&127
#define RMALIGNOS(x,nbits) ((RMuint32)(x)&((1<<(nbits))-1))
#define RMALIGNOSNEXT(x,nbits) (RMALIGNTONEXT((x)+1/*think twice*/,nbits)-(x))

/* 
   process x as a 32bit integer, align it to next 2^nbits byte boundary.

   Example
   RMALIGNTONEXT(0x12345600,2) = 0x12345600 (next 32bit word align)
   RMALIGNTONEXT(0x12345601,2) = 0x12345604 (next 32bit word align)
   RMALIGNTONEXT(0x12345601,4) = 0x12345610 (next 128bit word align)
*/
#define RMALIGNTONEXT(x,nbits) RMALIGNTO( (RMuint32)(x) + ((1<<(nbits))-1) , nbits )
#define RMALIGNTONEXTVar(x,nbits) RMALIGNTOVar( (RMuint32)(x) + ((1<<(nbits))-1) , nbits )

#define RMALIGNTO64Var(x,nbits) ((((RMuint64)(x))>>(nbits))<<(nbits))
#define RMALIGNTO64(x,nbits) RMALIGNTOVar(x,nbits)
#define RMALIGNTO64NEXT(x,nbits) RMALIGNTO( (RMuint64)(x) + ((1ULL<<(nbits))-1ULL) , nbits )

/*
  check if [p, p+size[ crosses a 2^nbits boundary.

  [0x803,0x803 + 0x20[ does cross a 2^5 boundary.

  [0x800,0x800 + 0x20[ does not cross a 2^5 boundary.
  [0x800,0x800 + 0[ does not cross a 2^5 boundary.

  The alternate with ((RMALIGNTONEXT(p+size,nbits)-RMALIGNTO(p,nbits))>>nbits)<=1 is too much calculus.
 */
#define RMCROSSES(p,size,nbits) (RMALIGNTO(p,nbits)!=RMALIGNTO((RMuint32)(p)+(RMuint32)(size)-1,nbits))

/// 
/**
   Returns the largest page aligned address <= x.

   @param x     
   @return -ReturnValue-
*/
#define RM_PAGE_ALIGN(x) RMALIGNTO(x,LOG2_RM_PAGE_SIZE)

/// 
/**
   Classic variant of the above, not to be mismatched

   Typically: p=RMMalloc(size+RM_PAGE_SIZE-1); Give out RM_NEXT_PAGE_ALIGN(p).

   @param x     
   @return -ReturnValue-
*/
#define RM_NEXT_PAGE_ALIGN(x) RMALIGNTONEXT(x,LOG2_RM_PAGE_SIZE)

/// 
/**
   Returns the next tile aligned address. RMTILE_SIZE_SHIFT defined in rmfeatures.h

   @param x     
   @return -ReturnValue-
*/
#define RM_NEXT_TILE_ALIGN(x) RMALIGNTONEXT(x,RMTILE_SIZE_SHIFT)

/*
  returns the number of the highest bit set
  
  log2 0xffffffff = 31
  log2 0x1003 = 12
  log2 0x1000 = 12
  log2 0x1 = 0
  log2 (nil) = -1
 */
#if RMCOMPILERID==RMCOMPILERID_MIPSEL_GCC && !defined(XOS_COMPACT)

#define RMlog2(a)							\
({									\
	RMint32 __hidden_rv; \
									\
	__asm__ __volatile__(						\
			     "clz %0, %1\n"				\
			     "neg %0, %0\n"				\
			     "addu %0, 31\n"			\
			     : "=r" (__hidden_rv) \
			     : "r" (a));				\
									\
	__hidden_rv;							\
})

#else

static inline RMint32 RMlog2(RMuint32 a)
{
	RMint32 rv = -1;
	
	while (a) {
		rv++;
		a >>= 1;
	}
	
	return rv;
}

#endif

/*
  returns the number of the lowest bit set
  
  0: -1
  1: 0
  4: 2
  0x8C: 2
  0x8000: 15
 */
static inline RMint32 RMlbs(RMuint32 a)
{
	return RMlog2(a - (a & (a - 1)));
}

/*
  Is a power of two?

  0: FALSE
  1: TRUE
  8: TRUE
  0x1000: TRUE
  0x1003: FALSE
 */
static inline RMbool RMisPot(RMuint32 a) 
{
	return (!(a & (a - 1)) && a);  // check if non-zero after removing least significant set bit
}

/* 
  Count the number of set bits
  
  0: 0
  1: 1
  8: 1
  0x5A5A5A5A: 16
  0xFFFFFFFF: 32
 */
static inline RMuint32 RMcountBits(RMuint32 x)
{
	RMuint32 _rmmacros_i;
	
	for (_rmmacros_i = 0; x; _rmmacros_i++) {
		x &= x - 1;  // remove least significant set bit
	}
	
	return _rmmacros_i;
}

/*
  only for 32bit; mips assembly small if log2im is immediate (resolved at compile time)
 */
#define RMHASIMBIT(x,log2im) ((int)( ((RMuint32)(x)<<(31-(log2im))) )<0)

#if (EM86XX_CHIP>=EM86XX_CHIPID_TANGO3)
#define RM_DRAM_CONTROLLER_ID(x) (((x)&0x80000000) ? (((x)>>30) - 2):(((x)>>28) - 1))
#else
#define RM_DRAM_CONTROLLER_ID(x) (((x)>>28) - 1)
#endif

/*
It divides two RMuint64 and returns quotient = a / b and reminder */
static inline RMuint64 RM64div64rem64(RMuint64 a, RMuint64 b, RMuint64 *rem)
{
	RMuint64 remainder = 0, quotient = 0;
	RMuint32 r32, d, b_low;
	RMint32 i;
	RMuint8 nbits = sizeof(RMuint64) * 8;
	
	if(rem)
		*rem = 0;

	if (! b) {
		/* RMDBGLOG((ENABLE, "RM64div64() DIVIDE BY 0!\n" )); */
		return a;
	}
	
	if (a == b) {
		return 1;
	}
	
	if (((b >> 32) == 0) && ((a >> 32) == 0)) {
		/* Simplest Case, 32 bits division */
		quotient = (RMuint64)((RMuint32)a / (RMuint32)b);
		if(rem)
			*rem = a - quotient * (RMuint32)b;
		return quotient;
	}
	
	if ((b >> 28) == 0) {
		/* Divisor is less than 28bit length */
		b_low = (RMuint32)b;
		d = (RMuint32)(a >> 32);
		quotient = (d / b_low);
		
		for (i = 0; i < 8; i++) {
			r32 = d % b_low;
			d = (RMuint32)((r32 << 4) | ((a >> (7 - i) * 4) & 0xf));
			quotient = (quotient << 4) | (d / b_low);
		}
		if(rem)
			*rem =  d % b_low;
		return quotient;
	}
	
	/* Skip first zeros */
	quotient = (((RMuint64)1) << (nbits - 1));
	while (quotient && ((a & quotient) == 0)) {
		nbits--;
		quotient >>= 1;
	}
	
	/* Compute, simple 2-radix */
	quotient = 0;
	for (i = (nbits - 1); i >= 0; i--) {
		remainder = remainder << 1 | ((a >> i) & 1);
		quotient <<= 1;
		if (remainder >= b) {
			remainder -= b;
			quotient |= 1;
		}
	}
	if(rem)
		*rem = remainder;
	return quotient;
}

static inline RMuint64 RM64div64(RMuint64 a, RMuint64 b)
{
	return RM64div64rem64(a, b, (RMuint64 *)NULL );
}

/*
  DEPRECATED. See #5142. em2007apr17 
  
  This funtion divides two signed 64 bit integer, 
   and writes the resulting integer quotient, the
   remainder and the first 64 bits of the fractional 
   part into 3 64 bit integer, whose references are 
   passed to the function. Each of the references
   can be NULL, if the result is not desired.
*/

static inline RMstatus RM64divfrac64rem64(
	RMint64 numer,   /* enumerator                  */
	RMint64 denom,   /* denominator                 */
	RMint64 *pQuot,  /* result of integer division  */
	RMint64 *pRem,   /* remainder of division       */
	RMint64 *pFrac)  /* fractional part of division */
{
	RMint64 quot;
	RMint64 frac;
	RMint32 shift;
	RMint64 sign = 1;
	
	/* division by zero? */
	if (!denom) return RM_ERROR;
	
	/* remove and remember sign */
	if (numer < 0) {
		numer = -numer;
		sign = -sign;
	}
	if (denom < 0) {
		denom = -denom;
		sign = -sign;
	}
	
	/* align denom with numer */
	quot = frac = 0;
	shift = 0;
	while ((denom < numer) && ((denom & 0x8000000000000000ll) == 0)) {
		denom <<= 1;
		shift++;
	}
	
	/* divide numer by denom */
	while (1) {
		if (numer >= denom) {
			numer -= denom;
			quot |= 1;
		}
		if (! shift) break;
		denom >>= 1;
		quot <<= 1;
		shift--;
	}
	
	/* write results and check if fractional part is needed */
	if (pQuot) *pQuot = quot * sign;
	if (pRem) *pRem = numer * sign;
	if (!pFrac) return RM_OK;
	
	/* keep dividing remainder for fractional part */
	while (1) {
		if (numer & 0x8000000000000000ll) {
			if ( ((numer == (denom >> 1)) && (!(denom & 1))) || (numer > (denom >> 1))) {
				numer -= (denom >> 1);
				frac |= 1;
			}
			numer <<= 1;
			if (denom & 1) numer -= 1;
		} else {
			numer <<= 1;
			if (numer >= denom) {
				numer -= denom;
				frac |= 1;
			}
		}
		if (shift <= -63) break;
		frac <<= 1;
		shift--;
	}
	
	/* write fractional part and return */
	*pFrac = frac * sign;
	
	return RM_OK;
}

/*
   Knuth's implementation to divide a 96bits integer by a 32bits integer (Donald E. Knuth, "The Art Of Computer Programming", chapter 4.3.1, algorithm D).
   divides a 64 bit integer (6 x 16) with a 32 bit integer. Stores Quotient in a 96 bits integer (6 x 16) and Rest in a 32 bit integer (2 x 16).
   N = M*Q + R where all operands are positive. 
   
   @param N 96 bits unsigned integer.
   @param M 32 bits unsigned integer.
   @param Q 96 bits unsigned integer.
   @param R 32 bits unsigned integer.
*/
static inline RMstatus RMuint96div32(RMuint16 N[6], RMuint16 M[2], RMuint16 Q[6], RMuint16 R[2])
{
	RMint32 i;
	RMuint32 r, q;
	RMuint32 n;
	RMuint32 D;
	RMuint32 ND[7];
	RMuint32 MD[2];
	RMint64 u,v;
	
	if (M[1] == 0 && M[0] == 0) {
/* 		RMDBGLOG((ENABLE, "Division by zero\n")); */
		return RM_ERROR;
	}

	/* simplest case, one digit on the divisor */
	if (M[1] == 0) {
		R[1] = 0;
		r = 0;
		
		/* 5 is digits(N) - digits(M) = 6-1, where digits return the number of digit (in 2^16 base) of a number */
		for (i=5 ; i>=0 ; i--) {
			n = (r << 16) + N[i];
			Q[i] = n / M[0];
			r = n - (RMuint32)(Q[i])*(RMuint32)(M[0]);
		}
		R[0] = r;
	}
	else {
		/* first normalize : ND = N<<D and MD = M << D, so that MD[1] >= 2^16 / 2 */
		D=0;
		while ((M[1] << D) < (1 << 15)) D++;

		if (D > 0) {
			MD[0] = (((RMuint32) M[0]) << D) & 0xffff;
			MD[1] = (((RMuint32) M[1]) << D) | (M[0] >> (16-D));
			
			ND[0] = (((RMuint32) N[0]) << D) & 0xffff;
			for (i=1 ; i<6 ; i++) 
				ND[i] = ((((RMuint32) N[i]) << D) | (N[i-1] >> (16-D))) & 0xffff;
			ND[6] = N[5] >> (16-D);
		} else {
			for (i=0 ; i<2 ; i++)
				MD[i] = M[i];
			
			for (i=0 ; i<6 ; i++)
				ND[i] = N[i];
			ND[6] = 0;
		}

/* 		RMDBGLOG((DISABLE,"N -> %d:%d:%d:%d:%d:%d, M -> %d:%d\n", N[5], N[4], N[3], N[2], N[1], N[0], M[1], M[0])); */
/* 		RMDBGLOG((DISABLE,"D: %lu, N -> %lu:%lu:%lu:%lu:%lu:%lu:%lu, M -> %lu:%lu\n", D, ND[6], ND[5], ND[4], ND[3], ND[2], ND[1], ND[0], MD[1], MD[0])); */

		v = (((RMuint64) MD[1]) << 16) + MD[0];

		Q[5] = 0;
		
		/* 4 is digits(N) - digits(M) = 6-2, where digits return the number of digit (in 2^16 base) of a number */
		for (i=4 ; i>=0 ; i--) {
			n = (ND[i+2]<<16) + ND[i+1];
			q = n / MD[1];

			if (q >= (1<<16)) {
				q = (1<<16) - 1;
			} else {
				r = n - q*MD[1];
				while (q * MD[0] > (r << 16) + ND[i]) {
					q --;
					r += MD[1];
					if (r >= 1<<16)
						break;
				}
			}
			
			/* no need to use multiprecision multiplication and substraction due to the limited range of the operands */
			u = (((RMuint64) ND[i+2]) << 32) + (((RMuint64) ND[i+1]) << 16) + ((RMuint64) ND[i]);  
			u = u - q*v;
			if (u < 0) {
				u += ((RMuint64) 1) << 48;
				q --;
				u += v;
			}
			
/* 			RMDBGLOG((DISABLE,"ND[%lu]: %lu, ND[%lu]: %lu, Q[%lu]: %lu\n", i+2, ND[i+2], i+1, ND[i+1], i, q)); */
			
			Q[i] = q;
			
			ND[i+2] = (u >> 32) & 0xffff;
			ND[i+1] = (u >> 16) & 0xffff;
			ND[i+0] = (u >>  0) & 0xffff;
		}
		
		u = (ND[1] << 16) + ND[0];
		r = u >> D;
/* 		RMDBGLOG((DISABLE,"u: %lld, r: %lu, N1: %lx, N0: %lx\n", u, r, ND[1], ND[0])); */

		R[1] = (r >> 16) & 0xffff;
		R[0] = (r >>  0) & 0xffff;
	} 

	return RM_OK;
}

/**
   Knuth's implementation to divide a 64bits integer by a 32bits integer (Donald E. Knuth, "The Art Of Computer Programming", chapter 4.3.1, algorithm D).
   divides a 64 bit integer (4 x 16) with a 32 bit integer. Stores Quotient in a 64 bits integer (4 x 16) and Rest in a 32 bit integer (2 x 16).
   N = M*Q + R where all operands are positive. 

   @param N 64 bits unsigned integer.
   @param M 32 bits unsigned integer.
   @param Q 64 bits unsigned integer.
   @param R 32 bits unsigned integer.
*/


static inline RMstatus RMuint64div32(RMuint16 N[4], RMuint16 M[2], RMuint16 Q[4], RMuint16 R[2])
{
	RMint32 i;
	RMuint32 r, q;
	RMuint32 n;
	RMuint32 D;
	RMuint32 ND[5];
	RMuint32 MD[2];
	RMint64 u,v;

	if (M[1] == 0 && M[0] == 0) {
/* 		RMDBGLOG((ENABLE, "Division by zero\n")); */
		return RM_ERROR;
	}

	/* simplest case, one digit on the divisor */
	if (M[1] == 0) {
		R[1] = 0;
		r = 0;

		/* 3 is digits(N) - digits(M) = 4-1, where digits return the number of digit (in 2^16 base) of a number */
		for (i=3 ; i>=0 ; i--) {
			n = (r << 16) + N[i];
			Q[i] = n / M[0];
			r = n - (RMuint32)(Q[i])*(RMuint32)(M[0]);
		}
		R[0] = r;
	}
	else {
		/* first normalize : ND = N<<D and MD = M << D, so that MD[1] >= 2^16 / 2 */
		D=0;
		while ((M[1] << D) < (1 << 15)) D++;

		if (D > 0) {
			MD[0] = (((RMuint32) M[0]) << D) & 0xffff;
			MD[1] = (((RMuint32) M[1]) << D) | (M[0] >> (16-D));
			
			ND[0] = (((RMuint32) N[0]) << D) & 0xffff;
			for (i=1 ; i<4 ; i++) 
				ND[i] = ((((RMuint32) N[i]) << D) | (N[i-1] >> (16-D))) & 0xffff;
			ND[4] = N[3] >> (16-D);
		} else {
			for (i=0 ; i<2 ; i++)
				MD[i] = M[i];
			
			for (i=0 ; i<4 ; i++)
				ND[i] = N[i];
			ND[4] = 0;
		}

/* 		RMDBGLOG((DISABLE,"N -> %d:%d:%d:%d, M -> %d:%d\n", N[3], N[2], N[1], N[0], M[1], M[0])); */
/* 		RMDBGLOG((DISABLE,"D: %lu, N -> %lu:%lu:%lu:%lu:%lu, M -> %lu:%lu\n", D, ND[4], ND[3], ND[2], ND[1], ND[0], MD[1], MD[0])); */

		v = (((RMuint64) MD[1]) << 16) + MD[0];
		
		Q[3] = 0;

		/* 2 is digits(N) - digits(M) = 4-2, where digits return the number of digit (in 2^16 base) of a number */
		for (i=2 ; i>=0 ; i--) {
			n = (ND[i+2]<<16) + ND[i+1];
			q = n / MD[1];

			if (q >= (1<<16)) {
				q = (1<<16) - 1;
			} else {
				r = n - q*MD[1];
				while (q * MD[0] > (r << 16) + ND[i]) {
					q --;
					r += MD[1];
					if (r >= 1<<16)
						break;
				}
			}
			
			/* no need to use multiprecision multiplication and substraction due to the limited range of the operands */
			u = (((RMuint64) ND[i+2]) << 32) + (((RMuint64) ND[i+1]) << 16) + ((RMuint64) ND[i]);  
			u = u - q*v;
			if (u < 0) {
				u += ((RMuint64) 1) << 48;
				q --;
				u += v;
			}
			
/* 			RMDBGLOG((DISABLE,"ND[%lu]: %lu, ND[%lu]: %lu, Q[%lu]: %lu\n", i+2, ND[i+2], i+1, ND[i+1], i, q)); */

			Q[i] = q;
			
			ND[i+2] = (u >> 32) & 0xffff;
			ND[i+1] = (u >> 16) & 0xffff;
			ND[i+0] = (u >>  0) & 0xffff;
		}
		
		u = (ND[1] << 16) + ND[0];
		r = u >> D;
/* 		RMDBGLOG((DISABLE,"u: %lld, r: %lu, N1: %lx, N0: %lx\n", u, r, ND[1], ND[0])); */

		R[1] = (r >> 16) & 0xffff;
		R[0] = (r >>  0) & 0xffff;
	} 
	
	return RM_OK;
}

/**
 multiplies a 64 bit integer (2 x 32) with a 32 bit integer and store it a 96 bit integer (3 x 32).
   @param N 64 bits integer.
   @param m 32 bits integer.
   @param P 96 bits integer.
*/

static inline void RMuint64mul32(RMuint32 N[2], RMuint32 m, RMuint32 P[3])

{
	RMuint64 n1m=(RMuint64)(N[1])*(RMuint64)m;
	RMuint64 n0m=(RMuint64)(N[0])*(RMuint64)m;
	P[0]=n0m;
	P[1]=(n0m>>32)+(RMuint32)n1m;
	P[2]=n1m>>32;
	if (P[1]<(RMuint32)n1m)
		P[2]++;
}

RM_EXTERN_C_BLOCKEND

#endif // __RMMACROS_H__
