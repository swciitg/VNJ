/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

#include "yaffs_launcher.h"
#include "includes/pio.h"

// to enable or disable the debug messages of this source file, put 1 or 0 below
#if 0
#define LOCALDBG ENABLE
#else
#define LOCALDBG DISABLE
#endif

static inline void _mal_nozsc(RMuint32 *dst,const RMuint32 *src, RMuint32 size)
{
	__asm__ __volatile__(ASMPROLOGUE
"	addu	$13, %1, %2\n"
"1:\n"
"	lw	$15, 0(%1)\n"
"	addu	%1, 4\n"
"	addu	%0, 4\n"
"	bne	%1, $13, 1b\n"
"	sw	$15, -4(%0)\n"
"2:\n" ASMEPILOGUE
: "=&r" (dst), "=&r" (src)
: "r" (size), "0" (dst), "1" (src)
: "t7", "t5");
}

static inline void _mual_nozsc(RMuint32 *dst,const RMuint8 *src, RMuint32 size)
{
	__asm__ __volatile__(ASMPROLOGUE
"	addu	$12, %1, %2\n"
"1:\n"
"	lwl	$11, 3(%1)\n"
"	addu	%0, 4\n"
"	addu	%1, 4\n"
"	lwr	$11, -4(%1)\n"
"	bne	%1, $12, 1b\n"
"	sw	$11, -4(%0)\n"
"2:\n" ASMEPILOGUE
: "=&r" (dst), "=&r" (src)
: "r" (size), "0" (dst), "1" (src)
: "t3", "t4");
}

void *memcpy_v2v(RMuint8 *dst, const RMuint8 *src, RMuint32 n)
{
	RMuint8 *keepdst=dst;
	RMuint32 na;

	RMDBGLOG((LOCALDBG,"memcpy_v2v %p<-%p %d\n",dst,src,n));

	// byte-per-byte start until dst aligned
	while (1) {
		if (n<1) 
			break;
		
		if (((RMuint32)dst&3)==0)
			break;
		
		RMDBGLOG((LOCALDBG," %ds\n",n));
		__asm__ __volatile__(
				     "lbu %0, 0(%1)\n"
				     "addu %1, 1\n"
				     "subu %2, 1\n"
				     : "=&r" (dst[0]), "=&r" (src), "=&r" (n)
				     : "1" (src), "2" (n));
		dst++; 
	}
	
	// word-per-word now. if src unaligned: ulw. else, lw
	na=RMALIGNTO(n,2);
	
	if (na>0) {
		if (((RMuint32)src&3)==0) {
			RMDBGLOG((LOCALDBG," %da-%d\n",n,na));
			_mal_nozsc((RMuint32 *)dst,(RMuint32 *)src,na);
		}
		else {
			RMDBGLOG((LOCALDBG," %du-%d\n",n,na));
			_mual_nozsc((RMuint32 *)dst,src,na);
		}
		
		n-=na;
		dst+=na;
		src+=na;
	}

	// byte-per-byte finish
	while (1) {
		if (n<1) 
			break;
		
		RMDBGLOG((LOCALDBG," %df\n",n));
		__asm__ __volatile__(
				     "lbu %0, 0(%1)\n"
				     "addu %1, 1\n"
				     "subu %2, 1\n"
				     : "=&r" (dst[0]), "=&r" (src), "=&r" (n)
				     : "1" (src), "2" (n));
		dst++; 
	}

	return keepdst;
}

void *memcpy_g2g(struct gbus *pgbus,RMuint8 *dst, const RMuint8 *src, RMuint32 n)
{
	RMuint8 *keepdst=dst;
	RMuint32 dstb,srcb;
	RMuint32 dstv,srcv;
	RMuint32 nv;
	
	RMDBGLOG((LOCALDBG,"memcpy_g2g %p<-%p %d\n",dst,src,n));

	while (1) {
		if (n<1)
			break;
		
		dstb=RMALIGNTONEXT(dst+1,26); // next remap boundary
		srcb=RMALIGNTONEXT(src+1,26); // next remap boundary
		RMDBGLOG((LOCALDBG,"distance to dstb %d\n",dstb-(RMuint32)dst));
		nv=RMmin(n,dstb-(RMuint32)dst);
		RMDBGLOG((LOCALDBG,"distance to srcb %d\n",srcb-(RMuint32)src));
		nv=RMmin(nv,srcb-(RMuint32)src);

		dstv=get_remap(pgbus,(RMuint32)dst);
		srcv=get_remap(pgbus,(RMuint32)src);
		memcpy_v2v((RMuint8 *)dstv,(RMuint8 *)srcv,nv);
		release_remap(pgbus,dstv);
		release_remap(pgbus,srcv);
		
		dst+=nv;
		src+=nv;
		n-=nv;
	}
	
	return keepdst;
}

void *memcpy_g2v(struct gbus *pgbus,RMuint8 *dst, const RMuint8 *src, RMuint32 n)
{
	RMuint8 *keepdst=dst;
	RMuint32 srcb;
	RMuint32 srcv;
	RMuint32 nv;
	
	RMDBGLOG((LOCALDBG,"memcpy_g2v %p<-%p %d\n",dst,src,n));

	while (1) {
		if (n<1)
			break;
		
		srcb=RMALIGNTONEXT(src+1,26); // next remap boundary
		RMDBGLOG((LOCALDBG,"distance to srcb %d\n",srcb-(RMuint32)src));
		nv=RMmin(n,srcb-(RMuint32)src);
		
		srcv=get_remap(pgbus,(RMuint32)src);
		memcpy_v2v(dst,(RMuint8 *)srcv,nv);
		release_remap(pgbus,srcv);
		
		dst+=nv;
		src+=nv;
		n-=nv;
	}
	
	return keepdst;
}

void *memcpy_v2g(struct gbus *pgbus,RMuint8 *dst, const RMuint8 *src, RMuint32 n)
{
	RMuint8 *keepdst=dst;
	RMuint32 dstb;
	RMuint32 dstv;
	RMuint32 nv;
	
	RMDBGLOG((LOCALDBG,"memcpy_v2g %p<-%p %d\n",dst,src,n));

	while (1) {
		if (n<1)
			break;
		
		dstb=RMALIGNTONEXT(dst+1,26); // next remap boundary
		RMDBGLOG((LOCALDBG,"distance to dstb %d\n",dstb-(RMuint32)dst));
		nv=RMmin(n,dstb-(RMuint32)dst);

		dstv=get_remap(pgbus,(RMuint32)dst);
		memcpy_v2v((RMuint8 *)dstv,src,nv);
		release_remap(pgbus,dstv);
		
		dst+=nv;
		src+=nv;
		n-=nv;
	}
	
	return keepdst;
}

#undef REGU

RMuint64 RMu64_u64divu64(RMuint64 x,RMuint64 y,RMuint64 *pr)
{
	RMDBGLOG((LOCALDBG,"input 0x%016llx / 0x%016llx\n",x,y));

	if ((y>>32)==0) {
		RMuint64 q;
		RMuint32 r;
		
		q=RMu64divu32_9831_10362(x,&r,(RMuint32)y);
		if (pr)
			*pr=r;

		return q;
	}
	else {
		/* 
		   this is handbook of applied cryptography 14.20 when
		   
		   y normalized,
		   n=2, t=1, b=2^32 (so x=x2x1x0 and y=y1y0 with y1>=2^31)
		*/
		RMuint32 f2,f1,f0;
		RMuint32 x2,x1,x0,y1,y0,q0;
		
#ifdef REGU
		x2=0;
		
		// normalize
		while ((RMint64)y>0) {
			x2<<=1;
			if ((RMint64)x<0) x2|=1;
			x<<=1;
			y<<=1;
		}
		
		x1=x>>32;
		x0=(RMuint32)x;
		y1=y>>32;
		y0=(RMuint32)y;
#else
		x1=x>>32;
		x0=(RMuint32)x;
		y1=y>>32;
		y0=(RMuint32)y;

		__asm__ __volatile__(ASMPROLOGUE
				     "clz $8, %3\n"
				     "neg $9, $8\n"
				     
				     "srl $10, %4, $9\n"
				     "sll %3, $8\n"
				     "or  %3, $10\n"
				     "sll %4, $8\n"
				     
				     "srl %0, %1, $9\n"
				     "sll %1, $8\n"
				     "srl $10, %2, $9\n"
				     "or  %1, $10\n"
				     "sll %2, $8\n"
				     ASMEPILOGUE
				     : "=&r" (x2), "=&r" (x1), "=&r" (x0), "=&r" (y1), "=&r" (y0)
				     :               "1" (x1),   "2" (x0),   "3" (y1),   "4" (y0)
				     : "t0", "t1", "t2");
#endif
		
		RMDBGLOG((LOCALDBG,"brute 0x%08lx:%08lx:%08lx / 0x%08lx:%08lx\n",x2,x1,x0,y1,y0));
				
		/*
		  normalisation was at most 31 shifts because originally y>=b.
		  
		  now y1y0 has bit#63=1
		  and x2x1x0 cannot have bit#95=1 (*)
		*/

		
		// step 1
		//q1=0;
		
		// step 2 per (*): condition does not happen

		// step 3 for i from 2 to 2, e.g. no loop
		
		// step 3.1 per (*): condition does not happen, implement the else{}:
		q0=RMu64divu32_9831_10362(RMuint64from2RMuint32(x2,x1),NULL,y1);

		// let's call f=q0*y
#ifdef REGU
		{
			RMuint64 Y1=y1,Y0=y0;
			
			Y1*=q0;
			Y0*=q0;
			
			f0=(RMuint32)Y0;
			f1=(Y0>>32)+(RMuint32)Y1;
			f2=Y1>>32;
			if (f1<(RMuint32)Y1)
				f2++;
		}
#else
		__asm__ __volatile__(ASMPROLOGUE
				     "multu %5, %3\n"
				     "mflo %2\n"
				     "mfhi %0\n"

				     "mtlo %0\n"
				     "mthi $0\n"
				     "maddu %4, %3\n"
				     "mflo %1\n"
				     "mfhi %0\n"
				     ASMEPILOGUE
				     : "=&r" (f2), "=&r" (f1), "=&r" (f0)
				     : "r" (q0), "r" (y1), "r" (y0));
#endif

		RMDBGLOG((LOCALDBG,"forg= 0x%08lx:%08lx:%08lx 0x%08lx\n",f2,f1,f0,q0));
		
		// step 3.2 while f>x { q0-- f-=y };
		while (1) {
			int gt;

#ifdef REGU
			gt=0;
			
			if (f2>x2)
				gt=1;
			else {
				if ((f2==x2)&&(f1>x1)) 
					gt=1;
				else {
					if ((f1==x1)&&(f0>x0))
						gt=1;
				}
			}
#else
			__asm__ __volatile__(ASMPROLOGUE
					     "sltu %0, %4, %1\n"
					     "bnez %0, 1f\n"
					     "nop\n"
					     "beq %4, %1, 1f\n"
					     "sltu %0, %5, %2\n"
					     "beq %5, %2, 1f\n"
					     "sltu %0, %6, %3\n"
					     "move %0, $zero\n"
					     "1:\n"
					     ASMEPILOGUE
					     : "=&r" (gt)
					     : "r" (f2), "r" (f1), "r" (f0), "r" (x2), "r" (x1), "r" (x0));
#endif
			
			if (!gt)
				break;
			
			q0--;
			
			{
				RMuint32 _f0=f0,_f1=f1;
				
				f0-=y0;
				f1-=y1;
				f1-=(f0>_f0);
				f2-=(f1>_f1);
				RMDBGLOG((LOCALDBG,"fre= 0x%08lx:%08lx:%08lx 0x%08lx\n",f2,f1,f0,q0));
			}
		}
		
		// step 3.3: because t=1, the quantity substracted at 3.2 is the same this one.
		// step 3.4: f<=x so 0<=x-f so x<0 does not happen

		// step 4: we do not give the remainder
		
		// step 5
		if (pr) 
                        *pr=x-q0*y;

		return q0;
	}
}
