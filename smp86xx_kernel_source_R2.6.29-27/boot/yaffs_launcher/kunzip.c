/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

#define ALLOW_OS_CODE 1
#include "yaffs_launcher.h"

#ifdef YAFFS_LAUNCHER_COMPILEMODE_MIPS16
#include "mips32_utilfunctions.h"
#undef  RMlog2
#define RMlog2(a) mips32_RMlog2((a))
#endif


static RMascii *heap_base = NULL;
static RMascii *heap_top = NULL;

static unsigned inptr = 0;		    /* index of next byte to be processed in inbuf */
static unsigned outcnt = 0;		    /* bytes in output buffer */
static RMascii *slide = NULL;
static RMascii *inbuf = NULL;

#define WSIZE (0x04000000/2)	/* default window size */

static RMuint32 space_remaining; /* Number of bytes of space that remains available to decompress to */
static RMuint32 window_size;	 /* Window size based on space_remaining, taken to the next power of two. */

static inline void *memcpy_gz(void *dest, const void *src, RMuint32 count)
{
	RMascii *tmp = dest;
	const RMascii *s = src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}

static void *memset_gz(void *s, RMint32 c, RMuint32 count)
{
	RMascii *xs = s;

	while (count--)
		*xs++ = c;
	return s;
}

static inline RMuint8 get_byte(void)
{
	RMuint8 b = inbuf[inptr++];

	return b;
}

static inline RMuint16 get_halfword(void)
{
	RMuint16 b = ((RMuint16 *) inbuf)[inptr / 2];

	inptr += 2;
	return b;
}

static inline void *__fake_malloc(RMint32 size)
{
	void *ret;

	ret = (void *) heap_top;
	heap_top += size;
	return ret;
}

static inline void __fake_free(void *p)
{
	heap_top = heap_base;
}

/* inflate.h end */

/* 
   inflate.c -- Not copyrighted 1992 by Mark Adler
   version c10p1, 10 January 1993 
 */

struct huft {
	RMuint8 e;			    /* number of extra bits or operation */
	RMuint8 b;			    /* number of bits in this code or subcode */
	union {
		RMuint16 n;		    /* literal, length base, or distance base */
		struct huft *t;		    /* pointer to next level of table */
	} v;
};

/* Function prototypes */
static RMint32 huft_build(unsigned *, unsigned, unsigned, RMuint16 *, RMuint16 *, struct huft **, RMint32 *);
static RMint32 huft_free(struct huft *);
static RMint32 inflate_codes(struct huft *, struct huft *, RMint32, RMint32);
static RMint32 inflate_stored(void);
static RMint32 inflate_fixed(void);
static RMint32 inflate_dynamic(void);
static RMint32 inflate_block(RMint32 *);
static RMint32 kinflate(void);

#define wp outcnt

/* Tables for deflate from PKZIP's appnote.txt. */
static const unsigned border[] = {	/* Order of the bit length code lengths */
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};
static const RMuint16 cplens[] = {	/* Copy lengths for literal codes 257..285 */
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};

	/* note: see note #13 above about the 258 in this list. */
static const RMuint16 cplext[] = {	/* Extra bits for literal codes 257..285 */
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99
};				/* 99==invalid */
static const RMuint16 cpdist[] = {	/* Copy offsets for distance codes 0..29 */
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
	8193, 12289, 16385, 24577
};
static const RMuint16 cpdext[] = {	/* Extra bits for distance codes */
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
	12, 12, 13, 13
};

#define INIT_BITBUFFER()						\
	{								\
		RMuint32 addr = (RMuint32)(&inbuf[inptr]);	\
		while ( addr & 3 ) {					\
			b |= ((RMuint32)get_byte()) << k;		\
			k += 8;						\
			addr++;						\
		}							\
	}
#define NEEDBITS(n)						\
	{							\
		while (k < (n)) {				\
			b |= ((RMuint32)get_halfword())<<k;	\
			k += 16;				\
		}						\
	}
#define DUMPBITS(n) {b>>=(n);k-=(n);}

#define lbits 9			/* bits in base literal/length lookup table */
#define dbits 6			/* bits in base distance lookup table */

/* If BMAX needs to be larger than 16, then h and x[] should be unsigned int. */
#define BMAX 16			/* maximum bit length of any code (16 for explode) */
#define N_MAX 288		/* maximum number of codes in any set */

static RMuint32 bb;			    /* bit buffer */
static unsigned bk;			    /* bits in bit buffer */

static const RMuint16 mask_bits[] = {
	0x0000,
	0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

static unsigned hufts;			    /* track memory usage */

static RMint32 huft_build(b, n, s, d, e, t, m)
     unsigned *b;			    /* code lengths in bits (all assumed <= BMAX) */
     unsigned n;			    /* number of codes (assumed <= N_MAX) */
     unsigned s;			    /* number of simple-valued codes (0..s-1) */
     RMuint16 *d;			    /* list of base values for non-simple codes */
     RMuint16 *e;			    /* list of extra bits for non-simple codes */
     struct huft **t;			    /* result: starting table */
     RMint32 *m;			    /* maximum lookup bits, returns actual */

/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.  Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory. */
{
	unsigned a;			    /* counter for codes of length k */
	unsigned c[BMAX + 1];		    /* bit length count table */
	unsigned f;			    /* i repeats in table every f entries */
	RMint32 g;			    /* maximum code length */
	RMint32 h;			    /* table level */
	register unsigned i;		    /* counter, current code */
	register unsigned j;		    /* counter */
	register RMint32 k;		    /* number of bits in current code */
	RMint32 l;			    /* bits per table (returned in m) */
	register unsigned *p;		    /* pointer into c[], b[], or v[] */
	register struct huft *q;	    /* points to current table */
	struct huft r;			    /* table entry for structure assignment */
	struct huft *u[BMAX];		    /* table stack */
	unsigned v[N_MAX];		    /* values in order of bit length */
	register RMint32 w;		    /* bits before this table == (l * h) */
	unsigned x[BMAX + 1];		    /* bit offsets, then code stack */
	unsigned *xp;			    /* pointer into x */
	RMint32 y;			    /* number of dummy codes added */
	unsigned z;			    /* number of entries in current table */

	/* Generate counts for each bit length */
	memset_gz((RMascii *) c, 0, sizeof(c));
	p = b;
	i = n;
	do {
		c[*p]++;	/* assume all entries <= BMAX */
		p++;		/* Can't combine with above line (Solaris bug) */
	} while (--i);
	if (c[0] == n) {	/* null input--all zero length codes */
		*t = (struct huft *) NULL;
		*m = 0;
		return 0;
	}

	/* Find minimum and maximum length, bound *m by those */
	l = *m;
	for (j = 1; j <= BMAX; j++)
		if (c[j])
			break;
	k = j;			/* minimum code length */
	if ((unsigned) l < j)
		l = j;
	for (i = BMAX; i; i--)
		if (c[i])
			break;
	g = i;			/* maximum code length */
	if ((unsigned) l > i)
		l = i;
	*m = l;

	/* Adjust last length count to fill out codes, if needed */
	for (y = 1 << j; j < i; j++, y <<= 1)
		if ((y -= c[j]) < 0)
			return 2;	/* bad input: more codes than bits */
	if ((y -= c[i]) < 0)
		return 2;
	c[i] += y;

	/* Generate starting offsets into the value table for each length */
	x[1] = j = 0;
	p = c + 1;
	xp = x + 2;
	while (--i) {		/* note that i == g from above */
		*xp++ = (j += *p++);
	}

	/* Make a table of values in order of bit lengths */
	p = b;
	i = 0;
	do {
		if ((j = *p++) != 0)
			v[x[j]++] = i;
	} while (++i < n);
	n = x[g];		/* set n to length of v */

	/* Generate the Huffman codes and for each, make the table entries */
	x[0] = i = 0;		/* first Huffman code is zero */
	p = v;			/* grab values in bit order */
	h = -1;			/* no tables yet--level -1 */
	w = -l;			/* bits decoded == (l * h) */
	u[0] = (struct huft *) NULL;	/* just to keep compilers happy */
	q = (struct huft *) NULL;	/* ditto */
	z = 0;			/* ditto */

	/* go through the bit lengths (k already is bits in shortest code) */
	for (; k <= g; k++) {
		a = c[k];
		while (a--) {
			/* here i is the Huffman code of length k bits for value *p */
			/* make tables up to required level */
			while (k > w + l) {
				h++;
				w += l;	/* previous table always l bits */

				/* compute minimum size table less than or equal to l bits */
				z = ((z = (unsigned) (g - w)) > (unsigned) l) ? (unsigned) l : z;	/* upper limit on table size */
				if ((f = 1 << (j = k - w)) > a + 1) {	/* try a k-w bit table *//* too few codes for k-w bit table */
					f -= a + 1;	/* deduct codes from patterns left */
					xp = c + k;
					if (j < z)
						while (++j < z) {	/* try smaller tables up to z bits */
							if ((f <<= 1) <= *++xp)
								break;	/* enough codes to use up j bits */
							f -= *xp;	/* else deduct codes from patterns */
						}
				}
				z = 1 << j;	/* table entries for j-bit table */

				/* allocate and link in new table */
				if ((q =
				     (struct huft *) __fake_malloc((z + 1) * sizeof(struct huft))) ==
				    (struct huft *) NULL) {
					if (h)
						huft_free(u[0]);
					return 3;	/* not enough memory */
				}
				hufts += z + 1;	/* track memory usage */
				*t = q + 1;	/* link to list for huft_free() */
				*(t = &(q->v.t)) = (struct huft *) NULL;
				u[h] = ++q;	/* table starts after link */

				/* connect to last table, if there is one */
				if (h) {
					x[h] = i;	/* save pattern for backing up */
					r.b = (RMuint8) l;	/* bits to dump before this table */
					r.e = (RMuint8) (16 + j);	/* bits in this table */
					r.v.t = q;	/* pointer to this table */
					j = i >> (w - l);	/* (get around Turbo C bug) */
					u[h - 1][j] = r;	/* connect to last table */
				}
			}

			/* set up table entry in r */
			r.b = (RMuint8) (k - w);
			if (p >= v + n)
				r.e = 99;	/* out of values--invalid code */
			else if (*p < s) {
				r.e = (RMuint8) (*p < 256 ? 16 : 15);	/* 256 is end-of-block code */
				r.v.n = (RMuint16) (*p);	/* simple code is just the value */
				p++;	/* one compiler does not like *p++ */
			}
			else {
				r.e = (RMuint8) e[*p - s];	/* non-simple--look up in lists */
				r.v.n = d[*p++ - s];
			}

			/* fill code-like entries with r */
			f = 1 << (k - w);
			for (j = i >> w; j < z; j += f)
				q[j] = r;

			/* backwards increment the k-bit code i */
			for (j = 1 << (k - 1); i & j; j >>= 1)
				i ^= j;
			i ^= j;

			/* backup over finished tables */
			while ((i & ((1 << w) - 1)) != x[h]) {
				h--;	/* don't need to update q */
				w -= l;
			}
		}
	}
	/* Return true (1) if we were given an incomplete table */
	return y != 0 && g != 1;
}

static RMint32 huft_free(t)
     struct huft *t;			    /* table to free */

/* Free the malloc'ed tables built by huft_build(), which makes a linked
   list of the tables it made, with the links in a dummy first entry of
   each table. */
{
	register struct huft *p, *q;

	/* Go through linked list, freeing from the malloced (t[-1]) address. */
	p = t;
	while (p != (struct huft *) NULL) {
		q = (--p)->v.t;
		__fake_free((RMascii *) p);
		p = q;
	}
	return 0;
}

static RMint32 inflate_codes(tl, td, bl, bd)
     struct huft *tl, *td;		    /* literal/length and distance decoder tables */
     RMint32 bl, bd;			    /* number of bits decoded by tl[] and td[] */

/* inflate (decompress) the codes in a deflated (compressed) block.
   Return an error code or zero if it all goes ok. */
{
	register unsigned e;		    /* table entry flag/number of extra bits */
	unsigned n, d;			    /* length and index for copy */
	unsigned w;			    /* current window position */
	struct huft *t;			    /* pointer to table entry */
	unsigned ml, md;		    /* masks for bl and bd bits */
	register RMuint32 b;		    /* bit buffer */
	register unsigned k;		    /* number of bits in bit buffer */

	/* make local copies of globals */
	b = bb;			/* initialize bit buffer */
	k = bk;
	w = wp;			/* initialize window position */

	/* inflate the coded data */
	ml = mask_bits[bl];	/* precompute masks for speed */
	md = mask_bits[bd];

	for (;;) {		/* do until end of block */

		NEEDBITS((unsigned) bl);
		if ((e = (t = tl + ((unsigned) b & ml))->e) > 16)
			do {
				if (e == 99)
					return 1;
				DUMPBITS(t->b);
				e -= 16;
				NEEDBITS(e);
			} while ((e = (t = t->v.t + ((unsigned) b & mask_bits[e]))->e) > 16);
		DUMPBITS(t->b);
		if (e == 16) {	/* then it's a literal */
			if (space_remaining-- == 0)
				return 1;
			slide[w++] = (RMuint8) t->v.n;
		}
		else {		/* it's an EOB or a length */

			/* exit if end of block */
			if (e == 15)
				break;
			/* get length of block to copy */
			NEEDBITS(e);
			n = t->v.n + ((unsigned) b & mask_bits[e]);
			DUMPBITS(e);

			/* decode distance of block to copy */
			NEEDBITS((unsigned) bd);
			if ((e = (t = td + ((unsigned) b & md))->e) > 16)
				do {
					if (e == 99)
						return 1;
					DUMPBITS(t->b);
					e -= 16;
					NEEDBITS(e);
				} while ((e = (t = t->v.t + ((unsigned) b & mask_bits[e]))->e) > 16);
			DUMPBITS(t->b);
			NEEDBITS(e);
			d = w - t->v.n - ((unsigned) b & mask_bits[e]);
			DUMPBITS(e);

			/* do the copy */
			do {
				d &= window_size - 1;
				e = window_size - (d > w ? d : w);
				e = e > n ? n : e;
				n -= e;
				
				if (space_remaining < e)
					return 1;

				memcpy_gz(slide + w, slide + d, e);
				space_remaining -= e;
				w += e;
				d += e;
				if (w == window_size) {
					w = 0;
				}
			} while (n);
		}
	}

	/* restore the globals from the locals */
	wp = w;			/* restore global window pointer */
	bb = b;			/* restore global bit buffer */
	bk = k;

	/* done */
	return 0;
}

static RMint32 inflate_stored()
/* "decompress" an inflated type 0 (stored) block. */
{
	unsigned n;			    /* number of bytes in block */
	unsigned w;			    /* current window position */
	register RMuint32 b;		    /* bit buffer */
	register unsigned k;		    /* number of bits in bit buffer */

	/* make local copies of globals */
	b = bb;			/* initialize bit buffer */
	k = bk;
	w = wp;			/* initialize window position */

	/* go to byte boundary */
	n = k & 7;
	DUMPBITS(n);

	/* get the length and its complement */
	NEEDBITS(16);
	n = ((unsigned) b & 0xffff);
	DUMPBITS(16);
	NEEDBITS(16);
	if (n != (unsigned) ((~b) & 0xffff))
		return 1;	/* error in compressed data */
	DUMPBITS(16);
	
	/* read and output the compressed data */
	while (n--) {
		NEEDBITS(8);
		if (space_remaining-- == 0)
			return 1;
		slide[w++] = (RMuint8) b;
		DUMPBITS(8);
	}

	/* restore the globals from the locals */
	wp = w;			/* restore global window pointer */
	bb = b;			/* restore global bit buffer */
	bk = k;
	return 0;
}

static RMint32 inflate_fixed()
/* decompress an inflated type 1 (fixed Huffman codes) block.  We should
   either replace this with a custom decoder, or at least precompute the
   Huffman tables. */
{
	RMint32 i;			    /* temporary variable */
	struct huft *tl;		    /* literal/length code table */
	struct huft *td;		    /* distance code table */
	RMint32 bl;			    /* lookup bits for tl */
	RMint32 bd;			    /* lookup bits for td */
	unsigned l[288];		    /* length list for huft_build */

	/* set up literal table */
	for (i = 0; i < 144; i++)
		l[i] = 8;
	for (; i < 256; i++)
		l[i] = 9;
	for (; i < 280; i++)
		l[i] = 7;
	for (; i < 288; i++)	/* make a complete, but wrong code set */
		l[i] = 8;
	bl = 7;
	if ((i = huft_build(l, 288, 257, (RMuint16 *) cplens, (RMuint16 *) cplext, &tl, &bl)) != 0)
		return i;

	/* set up distance table */
	for (i = 0; i < 30; i++)	/* make an incomplete code set */
		l[i] = 5;
	bd = 5;
	if ((i = huft_build(l, 30, 0, (RMuint16 *) cpdist, (RMuint16 *) cpdext, &td, &bd)) > 1) {
		huft_free(tl);
		return i;
	}

	/* decompress until an end-of-block code */
	if (inflate_codes(tl, td, bl, bd))
		return 1;

	/* free the decoding tables, return */
	huft_free(tl);
	huft_free(td);
	return 0;
}

static RMint32 inflate_dynamic()
/* decompress an inflated type 2 (dynamic Huffman codes) block. */
{
	RMint32 i;			    /* temporary variables */
	unsigned j;
	unsigned l;			    /* last length */
	unsigned m;			    /* mask for bit lengths table */
	unsigned n;			    /* number of lengths to get */
	struct huft *tl;		    /* literal/length code table */
	struct huft *td;		    /* distance code table */
	RMint32 bl;			    /* lookup bits for tl */
	RMint32 bd;			    /* lookup bits for td */
	unsigned nb;			    /* number of bit length codes */
	unsigned nl;			    /* number of literal/length codes */
	unsigned nd;			    /* number of distance codes */
	unsigned ll[286 + 30];		    /* literal/length and distance code lengths */
	register RMuint32 b;		    /* bit buffer */
	register unsigned k;		    /* number of bits in bit buffer */

	/* make local bit buffer */
	b = bb;
	k = bk;

	/* read in table lengths */
	NEEDBITS(5);
	nl = 257 + ((unsigned) b & 0x1f);	/* number of literal/length codes */
	DUMPBITS(5);
	NEEDBITS(5);
	nd = 1 + ((unsigned) b & 0x1f);	/* number of distance codes */
	DUMPBITS(5);
	NEEDBITS(4);
	nb = 4 + ((unsigned) b & 0xf);	/* number of bit length codes */
	DUMPBITS(4);
	if (nl > 286 || nd > 30)
		return 1;	/* bad lengths */
	
	/* read in bit-length-code lengths */
	for (j = 0; j < nb; j++) {
		NEEDBITS(3);
		ll[border[j]] = (unsigned) b & 7;
		DUMPBITS(3);
	}
	for (; j < 19; j++)
		ll[border[j]] = 0;

	/* build decoding table for trees--single level, 7 bit lookup */
	bl = 7;
	if ((i = huft_build(ll, 19, 19, NULL, NULL, &tl, &bl)) != 0) {
		if (i == 1)
			huft_free(tl);
		return i;	/* incomplete code set */
	}

	if (tl == NULL)		/* Grrrhhh */
		return 2;

	/* read in literal and distance code lengths */
	n = nl + nd;
	m = mask_bits[bl];
	i = l = 0;
	while ((unsigned) i < n) {
		NEEDBITS((unsigned) bl);
		j = (td = tl + ((unsigned) b & m))->b;
		DUMPBITS(j);
		j = td->v.n;
		if (j < 16)	/* length of code in bits (0..15) */
			ll[i++] = l = j;	/* save last length in l */
		else if (j == 16) {	/* repeat last length 3 to 6 times */
			NEEDBITS(2);
			j = 3 + ((unsigned) b & 3);
			DUMPBITS(2);
			if ((unsigned) i + j > n)
				return 1;
			while (j--)
				ll[i++] = l;
		}
		else if (j == 17) {	/* 3 to 10 zero length codes */
			NEEDBITS(3);
			j = 3 + ((unsigned) b & 7);
			DUMPBITS(3);
			if ((unsigned) i + j > n)
				return 1;
			while (j--)
				ll[i++] = 0;
			l = 0;
		}
		else {		/* j == 18: 11 to 138 zero length codes */
			NEEDBITS(7);
			j = 11 + ((unsigned) b & 0x7f);
			DUMPBITS(7);
			if ((unsigned) i + j > n)
				return 1;
			while (j--)
				ll[i++] = 0;
			l = 0;
		}
	}

	/* free decoding table for trees */
	huft_free(tl);

	/* restore the global bit buffer */
	bb = b;
	bk = k;

	/* build the decoding tables for literal/length and distance codes */
	bl = lbits;
	if ((i = huft_build(ll, nl, 257, (RMuint16 *) cplens, (RMuint16 *) cplext, &tl, &bl)) != 0) {
		if (i == 1) {
			huft_free(tl);
		}
		return i;	/* incomplete code set */
	}
	bd = dbits;
	if ((i = huft_build(ll + nl, nd, 0, (RMuint16 *) cpdist, (RMuint16 *) cpdext, &td, &bd)) != 0) {
		if (i == 1) {
			huft_free(td);
		}
		huft_free(tl);
		return i;	/* incomplete code set */
	}

	/* decompress until an end-of-block code */
	if (inflate_codes(tl, td, bl, bd))
		return 1;

	/* free the decoding tables, return */
	huft_free(tl);
	huft_free(td);
	return 0;
}

static RMint32 inflate_block(e)
     RMint32 *e;			    /* last block flag */

/* decompress an inflated block */
{
	unsigned t;			    /* block type */
	register RMuint32 b;		    /* bit buffer */
	register unsigned k;		    /* number of bits in bit buffer */

	/* make local bit buffer */
	b = bb;
	k = bk;

	/* read in last block bit */
	INIT_BITBUFFER();
	NEEDBITS(1);
	* e = (RMint32) b & 1;
	DUMPBITS(1);
	
	/* read in block type */
	NEEDBITS(2);
	t = (unsigned) b & 3;
	DUMPBITS(2);
	
	/* restore the global bit buffer */
	bb = b;
	bk = k;
	
	/* inflate that block type */
	if (t == 2)
		return inflate_dynamic();
	if (t == 0)
		return inflate_stored();
	if (t == 1)
		return inflate_fixed();

	/* bad block type */
	return 2;
}

static RMint32 kinflate()
/* decompress an inflated entry */
{
	RMint32 e;			    /* last block flag */
	RMint32 r;			    /* result code */
	unsigned h;			    /* maximum struct huft's malloc'ed */

	/* initialize slide(window), bit buffer */
	wp = 0;
	bk = 0;
	bb = 0;

	/* decompress until the last block */
	h = 0;
	do {
		hufts = 0;
		if ((r = inflate_block(&e)) != 0)
			return r;
		if (hufts > h)
			h = hufts;
	} while (!e);

	/* Undo too much lookahead. The next read will be byte aligned so we
	 * can discard unused bits in the last meaningful byte.
	 */
	while (bk >= 8) {
		bk -= 8;
		inptr--;
	}

	/* return success */
	return 0;
}

/* kunzip.c */
/* ========================================================================
 * Table of CRC-32's of all single-byte values (made by makecrc.c)
 */
static const RMuint32 crc_32_tab[] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};

static RMuint32 updcrc(RMuint8 *s, unsigned n)
    /* unsigned char *s;                  pointer to bytes to pump through */
    /* unsigned n;              number of bytes in s[] */
{
	register RMuint32 c;		    /* temporary variable */

	static RMuint32 crc = (RMuint32) 0xffffffffL;	/* shift register contents */

	if (s == NULL) {
		c = 0xffffffffL;
	}
	else {
		c = crc;
		if (n)
			do {
				c = crc_32_tab[((RMint32) c ^ (*s++)) & 0xff] ^ (c >> 8);
			} while (--n);
	}
	crc = c;
	return c ^ 0xffffffffL;	/* (instead of ~c for 64-bit machines) */
}

static RMuint32 get_unaligned_long(RMascii *addr)
{
	RMuint32 l;

	l = ((addr[0] & 0xff)
	     | ((addr[1] & 0xff) << 8)
	     | ((addr[2] & 0xff) << 16)
	     | ((addr[3] & 0xff) << 24));

	return l;
}

RMint32 kunzip(RMascii *zaddr, RMascii *loadaddr, RMuint32 heapaddr, RMint32 do_crc)
{
	RMuint32 orig_crc, crc;
	RMascii flag;

	space_remaining = WSIZE;
	if ((RMuint32)(1 << RMlog2(space_remaining)) == space_remaining)
		window_size = space_remaining;
	else
		window_size = 1 << (RMlog2(space_remaining) + 1);

	// Initialize globals
	outcnt = 0;
	inptr = 0;

	inbuf = zaddr;
	slide = loadaddr;

	heap_base = (RMascii *) heapaddr;

	heap_top = heap_base;

	if (do_crc)
		updcrc(NULL, 0);

	if ((*(inbuf + inptr + 2)) != 8) {
		return -1;
	}

	flag = *(inbuf + inptr + 3);
	inptr += 10;

	if (flag & 4) {		/* check FEXTRA */
		unsigned xlen = *(inbuf + inptr) | (*(inbuf + inptr + 1) << 8);

		inptr += (xlen + 2);	/* skip extra */
	}
	if (flag & 8) {		/* check FNAME */
		while (*(inbuf + inptr))
			inptr++;	/* skip file name */
		inptr++;
	}
	if (flag & 16) {	/* check FCOMMENT */
		while (*(inbuf + inptr))
			inptr++;	/* skip comment */
		inptr++;
	}
	if (flag & 2) {		/* check FHCRC */
		inptr += 2;	/* skip CRC16 */
	}

	if (kinflate() != 0)
		return -1;

	if (do_crc) {
		orig_crc = get_unaligned_long(inbuf + inptr);

		crc = updcrc((RMuint8 *) slide, outcnt);

		if (orig_crc != crc) {
			return -1;
		}
	}

	return outcnt;
}
