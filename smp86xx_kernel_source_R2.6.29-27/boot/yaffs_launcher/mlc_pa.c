/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

#include "yaffs_launcher.h"

#include "includes/mlc.h"
#include "includes/mlc_pa.h"


#if 0
#define LOCALDBG TRUE
#else
#define LOCALDBG FALSE
#endif

#define RAWPACKETSIZE (512+4*pj->spareper128)
#define MAXPACKETSIZE 1023
#define MAXPAGES 16

static struct mif *w_channels[] = { 
	(struct mif *)(REG_BASE_host_interface + MIF_W0_ADD),
	(struct mif *)(REG_BASE_host_interface + MIF_W1_ADD),
	(struct mif *)(REG_BASE_host_interface + MIF_W2_ADD)
};

static struct mif *r_channels[] = { 
	(struct mif *)(REG_BASE_host_interface + MIF_R0_ADD),
	(struct mif *)(REG_BASE_host_interface + MIF_R1_ADD),
	(struct mif *)(REG_BASE_host_interface + MIF_R2_ADD)
};

// indexed by scheme#
static const struct {
	RMuint32 ecccapa,ecclen,bbstart,bblen,bbpof;
} q[]={
	
	{ // 0
		ECCCAPA0,
		C2L(ECCCAPA0),
		RM_FIELD_OFFSET(struct page0,l.bb),
		RM_FIELD_SIZE(struct page0,l.bb),
		RM_FIELD_OFFSET(struct page0,l.bb),
	}, 

	{ // 1
		ECCCAPA1,
		C2L(ECCCAPA1),
		RM_FIELD_OFFSET(struct page1,l.bb),
		RM_FIELD_SIZE(struct page1,l.bb),
		RM_FIELD_SIZE(struct page1,l.databeg),
	},

	{ // 2
		ECCCAPA2,
		C2L(ECCCAPA2),
		RM_FIELD_OFFSET(struct page2,l.bb),
		RM_FIELD_SIZE(struct page2,l.bb),
		RM_FIELD_SIZE(struct page2,l.databeg),
	},

	{ // 3
		ECCCAPA3,
		C2L(ECCCAPA3),
		RM_FIELD_OFFSET(struct page3,l.bb),
		RM_FIELD_SIZE(struct page3,l.bb),
		RM_FIELD_SIZE(struct page3,l.databeg),
	},

	{ // 4
		ECCCAPA4,
		C2L(ECCCAPA4),
		RM_FIELD_OFFSET(struct page4,l.bb),
		RM_FIELD_SIZE(struct page4,l.bb),
		RM_FIELD_SIZE(struct page4,l.databeg),
	},
};
#define PQ (&q[pj->scheme])

#define PSWS ((RMuint32)WITHSPARE(1<<pj->l2ps))

#define COMMIT(field) do { gbus_write_uint32_ireg(pgbus,(RMuint32)&gch->field,pvch->field); } while (0)
#define COMMITU(field) do { gbus_write_uint32_ireg(pgbus,(RMuint32)&gch->field,pvch->field.value); } while (0)

#ifndef MLC_PA_NODDR
static void spystart(struct gbus *pgbus,RMuint32 crc32,RMuint32 crc32extent,int w)
{
	gbus_write_uint32_ireg(pgbus,REG_BASE_host_interface+host_CRC32_spy_crc32,crc32);
	gbus_write_uint32_ireg(pgbus,REG_BASE_host_interface+host_CRC32_spy_count,crc32extent);
	gbus_write_uint32_ireg(pgbus,REG_BASE_host_interface+host_CRC32_spy_conf,w);
}
#endif

RMstatus mlc_pa_request(struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami)
{
	struct mlc_cfg *pvch=&pj->vch,*gch=pj->cs?CHBREG_GA:CHAREG_GA;
	RMuint32 gamem=pj->cs?REG_BASE_host_interface+MLC_CHB_MEM:REG_BASE_host_interface+MLC_CHA_MEM;
	int i;
	int programmedpages;
	int programmedpackets;
	
	if ((int)gbus_read_uint32_ireg(pgbus,(RMuint32)&gch->status_reg)>=0)
		return RM_BUSY;
	
	programmedpages=RMALIGNTONEXT(pj->programmedsize,pj->l2ps)>>pj->l2ps;
	programmedpackets=RMALIGNTONEXT(pj->programmedsize,PACKET_SHIFT)>>PACKET_SHIFT;

	switch (pvch->flash_cmd) {
		struct mif *gmif;

	case CMD_BOOT:
	case CMD_READ:
		if (pvch->flash_cmd==CMD_BOOT) {
			RMuint32 gamemOTHER=pj->cs?REG_BASE_host_interface+MLC_CHA_MEM:REG_BASE_host_interface+MLC_CHB_MEM;
			memcpy_g2g(pgbus,(void *)pj->ga,(void *)(gamemOTHER+RMALIGNOS(pj->byteoffset,PACKET_SHIFT)),pj->programmedsize);
		}
		else {
 			gmif=w_channels[pj->ch];
			while (gbus_read_uint32_ireg(pgbus,(RMuint32)&gmif->cmd)!=MBUS_IDLE);
		}
		
		// collect metadata
		if (pj->vameta) 
			for (i=0;i<programmedpages;i++) {
				*pj->vameta=gbus_read_uint32_ireg(pgbus,gamem+i*16);
				pj->vameta++;
			}
		
		// collect bbdata
		if (pj->vabb) {
			for (i=0;i<programmedpages;i++) {
				memcpy_g2v(pgbus,pj->vabb,(void *)(gamem+MLC_BADBLOCK_OFFSET+i*12),PQ->bblen);
				pj->vabb+=PQ->bblen;
			}
		}
		
		// collect errors
		if (pj->biterrors==0) {
			for (i=0;i<programmedpages;i++) {
				RMuint32 eccreport=gbus_read_uint32_ireg(pgbus,gamem+MLC_ECCREPORT_OFFSET+4*i); 

				if (pj->vareport) {
					*pj->vareport=RMHASIMBIT(eccreport,7)?(char)RMunshiftBitsVar(eccreport,5,0):-1;
					pj->vareport++;
				}

				if (!RMHASIMBIT(eccreport,7)) 
					pj->biterrors=-1;
				else {
					pj->biterrors+=RMunshiftBitsVar(eccreport,5,0); // report on packet0.
					
					if (programmedpackets==1)
						break;
					
					if (!pvch->xfer_cfg.bits.sp) { // .5KB page devices: one packet per page; dont examine eccreport[15:8]
						
						eccreport>>=8;

						if (pj->vareport) {
							int j;
							for (j=0;j<RMmin(programmedpackets,PACKETSPERPAGE-1);j++) {
								*pj->vareport=RMHASIMBIT(eccreport,7)?(char)RMunshiftBitsVar(eccreport,5,0):-1;
								pj->vareport++;
							}
						}
						
						if (!RMHASIMBIT(eccreport,7)) 
							pj->biterrors=-1;
						else
							pj->biterrors+=(PACKETSPERPAGE-1)*RMunshiftBitsVar(eccreport,5,0); // report on the rest. It indicates the worst ever error rate. So the proper majorant is 7* this, for pages made of 8 packets.
					}
				}
			}
		}
	
		pj->ga+=pj->programmedsize;
		if (pvch->flash_cmd==CMD_READ) {
			pj->crc32=gbus_read_uint32_ireg(pgbus,REG_BASE_host_interface+host_CRC32_spy_crc32);
			pj->crc32extent=gbus_read_uint32_ireg(pgbus,REG_BASE_host_interface+host_CRC32_spy_count);
		}
		break;

	case CMD_PAGEPROGRAM:
 		gmif=r_channels[pj->ch];
		while (gbus_read_uint32_ireg(pgbus,(RMuint32)&gmif->cmd)!=MBUS_IDLE);

		if (pj->vameta) 
			pj->vameta+=programmedpages;
		if (pj->vabb) 
			pj->vabb+=PQ->bblen*programmedpages;

		pj->ga+=pj->programmedsize;
		pj->crc32=gbus_read_uint32_ireg(pgbus,REG_BASE_host_interface+host_CRC32_spy_crc32);
		pj->crc32extent=gbus_read_uint32_ireg(pgbus,REG_BASE_host_interface+host_CRC32_spy_count);
		break;

	default:
		break;
	}
		
	// move pointers
	pj->byteoffset+=pj->programmedsize;
	pj->size-=pj->programmedsize;

	return RM_OK;
}

void mlc_pa_yield(struct gbus *pgbus,struct mlc_pa_job *pj) 
{
}

#ifndef MLC_PA_NODDR

int mlc_pa_trigger_rawread (struct gbus *pgbus,struct mlc_pa_job *pj)
{
	struct mlc_cfg *pvch=&pj->vch,*gch=pj->cs?CHBREG_GA:CHAREG_GA;
	struct mif *gmif=w_channels[pj->ch];

	RMuint32 r;
	RMuint32 pgof=RMu64divu32(pj->byteoffset,&r,PSWS); // by design the hardware can't handle more than 4TByte devices
	RMuint32 pages=RMmin(pj->size/PSWS,MAXPAGES);

	if ((pj->vameta!=NULL)||(pj->vabb!=NULL)||(pj->vareport!=NULL))
		return -1;

	if (pj->byteoffset+pj->size>WITHSPARE(sll64to64_le32(1ULL,pj->l2wholesize)))
	    return -1;

	spystart(pgbus,pj->crc32,pj->crc32extent,1+pj->ch);

	pvch->add_page=pgof;
	COMMIT(add_page);
	pvch->add_offset=r;
	COMMIT(add_offset);

	pvch->xfer_cfg.bits.meta_size=0;
	pvch->packet_n_cfg=RAWPACKETSIZE<<16;
	COMMIT(packet_n_cfg);
	pvch->bad_block_cfg=0;
	COMMIT(bad_block_cfg);

	pvch->packet_0_cfg=pvch->packet_n_cfg;

	if ((r!=0)||(pages==0)) {
		RMuint32 shortone;
		// complete the page
		pj->programmedsize=RMmin(pj->size,PSWS-r);

		pvch->xfer_cfg.bits.pagecount=1;
		pvch->xfer_cfg.bits.packetcount=pj->programmedsize/RAWPACKETSIZE;

		shortone=pj->programmedsize%RAWPACKETSIZE;
		if (shortone!=0) {
			pvch->xfer_cfg.bits.packetcount++;
			pvch->packet_0_cfg=shortone<<16;
		}
		
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->add,pj->ga);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cnt,pj->programmedsize);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cmd,MBUS_LINEAR_VOID); // max 4223
	}
	else {	
		pj->programmedsize=pages*PSWS;
		
		pvch->xfer_cfg.bits.pagecount=pages;
		pvch->xfer_cfg.bits.packetcount=PACKETSPERPAGE;
		
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->add,pj->ga);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cnt,(pages<<16)|PSWS);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->skip,PSWS);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cmd,MBUS_RECTANGLE_VOID); // max 16*4224 (67KB)
	}
	
	COMMITU(xfer_cfg);
	COMMIT(packet_0_cfg);
	pvch->flash_cmd=CMD_READ;
	COMMIT(flash_cmd);

	// there will be no error report in raw mode. Don't try to access it, the simulator does not like it (x).
	pj->biterrors=-1;

	return pj->programmedsize;
}

int mlc_pa_trigger_rawwrite(struct gbus *pgbus,struct mlc_pa_job *pj) 
{
	struct mlc_cfg *pvch=&pj->vch,*gch=pj->cs?CHBREG_GA:CHAREG_GA;
	struct mif *gmif=r_channels[pj->ch];
	
	RMuint32 r;
	RMuint32 pgof=RMu64divu32(pj->byteoffset,&r,PSWS); // by design the hardware can't handle more than 4TByte devices
	RMuint32 pages=RMmin(pj->size/PSWS,MAXPAGES);

	if ((pj->vameta!=NULL)||(pj->vabb!=NULL)||(pj->vareport!=NULL))
		return -1;

	if (pj->byteoffset+pj->size>WITHSPARE(sll64to64_le32(1ULL,pj->l2wholesize)))
		return -1;

	spystart(pgbus,pj->crc32,pj->crc32extent,4+pj->ch);

	pvch->add_page=pgof;
	COMMIT(add_page);
	pvch->add_offset=r;
	COMMIT(add_offset);

	pvch->xfer_cfg.bits.meta_size=0;
	pvch->packet_n_cfg=RAWPACKETSIZE<<16;
	COMMIT(packet_n_cfg);
	pvch->bad_block_cfg=0;
	COMMIT(bad_block_cfg);

	pvch->packet_0_cfg=pvch->packet_n_cfg;

	if ((r!=0)||(pages==0)) {
		RMuint32 shortone;
		// complete the page
		pj->programmedsize=RMmin(pj->size,PSWS-r);

		pvch->xfer_cfg.bits.pagecount=1;
		pvch->xfer_cfg.bits.packetcount=pj->programmedsize/RAWPACKETSIZE;

		shortone=pj->programmedsize%RAWPACKETSIZE;
		if (shortone!=0) {
			pvch->xfer_cfg.bits.packetcount++;
			pvch->packet_0_cfg=shortone<<16;
		}
		
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->add,pj->ga);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cnt,pj->programmedsize);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cmd,MBUS_LINEAR_VOID); // max 4223
	}
	else {	
		pj->programmedsize=pages*PSWS;
		
		pvch->xfer_cfg.bits.pagecount=pages;
		pvch->xfer_cfg.bits.packetcount=PACKETSPERPAGE;
		
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->add,pj->ga);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cnt,(pages<<16)|PSWS);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->skip,PSWS);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cmd,MBUS_RECTANGLE_VOID); // max 16*4224 (67KB)
	}
	
	COMMITU(xfer_cfg);
	COMMIT(packet_0_cfg);
	pvch->flash_cmd=CMD_PAGEPROGRAM;
	COMMIT(flash_cmd);

	return pj->programmedsize;
}

#ifndef MLC_READONLY
int mlc_pa_trigger_rawblockerase(struct gbus *pgbus,struct mlc_pa_job *pj) 
{
	RMuint32 r,d=WITHSPARE(1<<pj->l2blocksize);
	RMuint64 blockof=RMu64divu32(pj->byteoffset,&r,d);
	RMuint32 blocks;
	if (r!=0)
		return -1;

	blocks=pj->size/d;
	
	if (pj->size%d!=0)
		return -1;

	pj->byteoffset=sll64to64_le32(blockof,pj->l2blocksize);
	pj->size=blocks<<pj->l2blocksize;

	if (mlc_pa_trigger_eccblockerase(pgbus,pj)==-1)
		return -1;
	
	pj->programmedsize=WITHSPARE(pj->programmedsize);
		
	return pj->programmedsize;
}
#endif	/* MLC_READONLY */

int mlc_pa_trigger_eccread (struct gbus *pgbus,struct mlc_pa_job *pj) 
{
	struct mlc_cfg *pvch=&pj->vch,*gch=pj->cs?CHBREG_GA:CHAREG_GA;
	struct mif *gmif=w_channels[pj->ch];
	RMuint32 pages;

	if (RMALIGNTO(pj->byteoffset,pj->l2ps)!=pj->byteoffset)
		return -1;
	
	pages=pj->size>>pj->l2ps;
	
	if (pages<<pj->l2ps!=pj->size)
		return -1;

	if (pj->byteoffset+pj->size>sll64to64_le32(1ULL,pj->l2wholesize-5)*32)
		return -1;

	pvch->add_page=srl64to64_le32(pj->byteoffset,pj->l2ps);
	COMMIT(add_page);
	pvch->add_offset=0;
	COMMIT(add_offset);

	pages=RMmin(pages,MAXPAGES);
	
	pj->programmedsize=pages<<pj->l2ps;

	spystart(pgbus,pj->crc32,pj->crc32extent,1+pj->ch);

	pvch->xfer_cfg.bits.pagecount=pages;
	pvch->xfer_cfg.bits.packetcount=PACKETSPERPAGE;
	pvch->xfer_cfg.bits.meta_size=4;
	COMMITU(xfer_cfg);
	pvch->packet_0_cfg=(((1<<PACKET_SHIFT)+4)<<16)|PQ->ecccapa;
	COMMIT(packet_0_cfg);
	pvch->packet_n_cfg=((1<<PACKET_SHIFT)<<16)|PQ->ecccapa;
	COMMIT(packet_n_cfg);
	pvch->bad_block_cfg=(PQ->bbstart<<16)|PQ->bblen;
	COMMIT(bad_block_cfg);

	pvch->flash_cmd=CMD_READ;
	COMMIT(flash_cmd);

	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->add,pj->ga);
	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cnt,(pages<<16)|(1<<pj->l2ps));
	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->skip,(1<<pj->l2ps)+pj->stride);
	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cmd,MBUS_RECTANGLE_VOID); // max 16*4096 (64KB)

	pj->biterrors=0;
	
	return pj->programmedsize;
}

int mlc_pa_trigger_eccwrite(struct gbus *pgbus,struct mlc_pa_job *pj) 
{
	struct mlc_cfg *pvch=&pj->vch,*gch=pj->cs?CHBREG_GA:CHAREG_GA;
	struct mif *gmif=r_channels[pj->ch];
	RMuint32 gamem=pj->cs?REG_BASE_host_interface+MLC_CHB_MEM:REG_BASE_host_interface+MLC_CHA_MEM;
	int i;
	RMuint32 pages;

	if (RMALIGNTO(pj->byteoffset,pj->l2ps)!=pj->byteoffset)
		return -1;
	
	pages=pj->size>>pj->l2ps;
	
	if (pages<<pj->l2ps!=pj->size)
		return -1;

	if (pj->byteoffset+pj->size>sll64to64_le32(1ULL,pj->l2wholesize-5)*32ULL)
		return -1;

	pvch->add_page=srl64to64_le32(pj->byteoffset,pj->l2ps);
	COMMIT(add_page);
	pvch->add_offset=0;
	COMMIT(add_offset);

	pages=RMmin(pages,MAXPAGES);

	// send metadata. hwbug#522: in simulation only: if you leave x here, bizarre things will happen.
	if (pj->vameta) 
		for (i=0;i<(signed)pages;i++) 
			gbus_write_uint32_ireg(pgbus,gamem+i*16,pj->vameta[i]);
	
	// send bbdata
	if (pj->vabb) {
		void *keep=pj->vabb;
		for (i=0;i<(signed)pages;i++) {
			memcpy_v2g(pgbus,(void *)(gamem+MLC_BADBLOCK_OFFSET+i*12),pj->vabb,PQ->bblen);
			pj->vabb+=PQ->bblen;
		}
		pj->vabb=keep;
	}
	else {
		/*
		  though the bad block area is writable, it's a bad idea to let random data land there.
		  If user did not explicitly give a vabb buffer, burn 0xff e.g. leave it as-is.
		 */

		for (i=0;i<(signed)pages*12;i+=4) 
			gbus_write_uint32_ireg(pgbus,gamem+MLC_BADBLOCK_OFFSET+i,-1);
	}
	
	pj->programmedsize=pages<<pj->l2ps;

	spystart(pgbus,pj->crc32,pj->crc32extent,4+pj->ch);

	pvch->xfer_cfg.bits.pagecount=pages;
	pvch->xfer_cfg.bits.packetcount=PACKETSPERPAGE;
	pvch->xfer_cfg.bits.meta_size=4;
	COMMITU(xfer_cfg);
	pvch->packet_0_cfg=(((1<<PACKET_SHIFT)+4)<<16)|PQ->ecccapa;
	COMMIT(packet_0_cfg);
	pvch->packet_n_cfg=((1<<PACKET_SHIFT)<<16)|PQ->ecccapa;
	COMMIT(packet_n_cfg);
	pvch->bad_block_cfg=(PQ->bbstart<<16)|PQ->bblen;
	COMMIT(bad_block_cfg);

	pvch->flash_cmd=CMD_PAGEPROGRAM;
	COMMIT(flash_cmd);

	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->add,pj->ga);
	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cnt,(pages<<16)|(1<<pj->l2ps));
	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->skip,(1<<pj->l2ps)+pj->stride);
	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cmd,MBUS_RECTANGLE_VOID); // max 16*4096 (64KB)

	return pj->programmedsize;
}

#endif

#ifndef MLC_READONLY
int mlc_pa_trigger_eccblockerase(struct gbus *pgbus,struct mlc_pa_job *pj)
{
	struct mlc_cfg *pvch=&pj->vch,*gch=pj->cs?CHBREG_GA:CHAREG_GA;

	if ((pj->vameta!=NULL)||(pj->vabb!=NULL)||(pj->vareport!=NULL))
		return -1;
	if (pj->byteoffset+pj->size>sll64to64_le32(1ULL,pj->l2wholesize-5)*32ULL)
		return -1;
	if (RMALIGNTO(pj->byteoffset,pj->l2blocksize)!=pj->byteoffset)
		return -1;
	if ((signed)pj->size!=1<<pj->l2blocksize) // hardware can't erase more than one block at a time
		return -1;

	pj->programmedsize=1<<pj->l2blocksize;
	
	pvch->add_page=srl64to64_le32(pj->byteoffset,pj->l2ps);
	COMMIT(add_page);
	pvch->flash_cmd=CMD_BLOCKERASE;
	COMMIT(flash_cmd);

	return pj->programmedsize;
}
#endif	/* MLC_READONLY */

#ifndef MLC_PA_NODDR

int mlc_pa_trigger_sparseeccread(struct gbus *pgbus,struct mlc_pa_job *pj) 
{
	struct mlc_cfg *pvch=&pj->vch,*gch=pj->cs?CHBREG_GA:CHAREG_GA;
	struct mif *gmif=w_channels[pj->ch];
	RMuint32 offset_inpacket;
	RMuint64 packetno;
	int packetno_inpage;
	RMuint32 left;

	if (pj->byteoffset+pj->size>sll64to64_le32(1ULL,pj->l2wholesize-5)*32ULL)
		return -1;

	/*
	  Ex l2ps=11

	  0                                               2048
	  |...........|...........|...........|...........|
	                 ^yyyy
                      xxxxxxxxxxxx
                      |--|
                      offset_inpacket

          x: what we must read to get the ecc report
	  y: what we keep
	*/

	pvch->add_page=srl64to64_le32(pj->byteoffset,pj->l2ps);
	COMMIT(add_page);
	pvch->add_offset=RMALIGNTO(RMALIGNOS(pj->byteoffset,pj->l2ps),PACKET_SHIFT);
	
	offset_inpacket=RMALIGNOS(pj->byteoffset,PACKET_SHIFT);
	pj->programmedsize=RMmin(pj->size,(1<<PACKET_SHIFT)-offset_inpacket);
	
	packetno=pj->byteoffset>>PACKET_SHIFT;
	packetno_inpage=packetno&(PACKETSPERPAGE-1);
	
	pvch->xfer_cfg.bits.pagecount=1;
	pvch->xfer_cfg.bits.packetcount=1;
	pvch->packet_0_cfg=((1<<PACKET_SHIFT)<<16)|PQ->ecccapa;
	
	if (packetno_inpage==0) {
		pvch->xfer_cfg.bits.meta_size=4;
		pvch->packet_0_cfg+=4<<16;
	}
	else {
		pvch->xfer_cfg.bits.meta_size=0;
		pvch->add_offset+=4+packetno_inpage*PQ->ecclen;
	}
	
	if (packetno_inpage==PACKETSPERPAGE-1) 
		pvch->bad_block_cfg=(PQ->bbpof<<16)|PQ->bblen;
	else
		pvch->bad_block_cfg=0;
	
	COMMIT(add_offset);
	COMMITU(xfer_cfg);
	COMMIT(packet_0_cfg);
	COMMIT(bad_block_cfg);

	pvch->flash_cmd=CMD_READ;
	COMMIT(flash_cmd);
	
	/*
	  ok the mlc controller is reading and ecc-ing a whole packet.
	  we're gonna: 

	   discard the start with mbus skip=0, 

	   keep the middle, 

	   discard the end with mbus skip=0.
	 */
	left=(1<<PACKET_SHIFT);

	if (offset_inpacket>0) {
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->add,pj->ga);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cnt,(offset_inpacket<<16)|1);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->skip,0);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cmd,MBUS_RECTANGLE_VOID);
		while (gbus_read_uint32_ireg(pgbus,(RMuint32)&gmif->cmd)!=MBUS_IDLE);
		// you don't want to squeeze the VOID so the writes of the trashed bytes are committed before going on
		left-=offset_inpacket;
	}

	spystart(pgbus,pj->crc32,pj->crc32extent,1+pj->ch);

	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->add,pj->ga);
	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cnt,pj->programmedsize);
	gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cmd,MBUS_LINEAR_VOID);
	while (gbus_read_uint32_ireg(pgbus,(RMuint32)&gmif->cmd)!=MBUS_IDLE);
	// you don't want to squeeze the VOID so you can save the first byte
	left-=pj->programmedsize;

	if (left>0) {
		RMuint8 keep;
		pj->crc32=gbus_read_uint32_ireg(pgbus,REG_BASE_host_interface+host_CRC32_spy_crc32);
		pj->crc32extent=gbus_read_uint32_ireg(pgbus,REG_BASE_host_interface+host_CRC32_spy_count);
		
		keep=gbus_read_uint8(pgbus,pj->ga);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->add,pj->ga);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cnt,(left<<16)|1);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->skip,0);
		gbus_write_uint32_ireg(pgbus,(RMuint32)&gmif->cmd,MBUS_RECTANGLE_VOID);
		while (gbus_read_uint32_ireg(pgbus,(RMuint32)&gmif->cmd)!=MBUS_IDLE);
		gbus_write_uint8(pgbus,pj->ga,keep);
		
		spystart(pgbus,pj->crc32,pj->crc32extent,1+pj->ch);
	}

	pj->biterrors=0;

	return pj->programmedsize;
}

#else

int mlc_pa_trigger_sparseeccread(struct gbus *pgbus,struct mlc_pa_job *pj) 
{
	struct mlc_cfg *pvch=&pj->vch,*gch=pj->cs?CHBREG_GA:CHAREG_GA;
	RMuint32 offset_inpacket;
	RMuint64 packetno;
	int packetno_inpage;

	if (pj->byteoffset+pj->size>sll64to64_le32(1ULL,pj->l2wholesize-5)*32ULL)
		return -1;

	/*
	  Ex l2ps=11

	  0                                               2048
	  |...........|...........|...........|...........|
	                 ^yyyy
                      xxxxxxxxxxxx
                      |--|
                      offset_inpacket

          x: what we must read to get the ecc report
	  y: what we keep
	*/

	pvch->add_page=srl64to64_le32(pj->byteoffset,pj->l2ps);
	COMMIT(add_page);
	pvch->add_offset=RMALIGNTO(RMALIGNOS(pj->byteoffset,pj->l2ps),PACKET_SHIFT);
	
	offset_inpacket=RMALIGNOS(pj->byteoffset,PACKET_SHIFT);
	pj->programmedsize=RMmin(pj->size,(1<<PACKET_SHIFT)-offset_inpacket);
	
	packetno=pj->byteoffset>>PACKET_SHIFT;
	packetno_inpage=packetno&(PACKETSPERPAGE-1);
	
	pvch->xfer_cfg.bits.pagecount=1;
	pvch->xfer_cfg.bits.packetcount=1;
	pvch->packet_0_cfg=((1<<PACKET_SHIFT)<<16)|PQ->ecccapa;
	
	if (packetno_inpage==0) {
		pvch->xfer_cfg.bits.meta_size=4;
		pvch->packet_0_cfg+=4<<16;
	}
	else {
		pvch->xfer_cfg.bits.meta_size=0;
		pvch->add_offset+=4+packetno_inpage*PQ->ecclen;
	}
	
	if (packetno_inpage==PACKETSPERPAGE-1) 
		pvch->bad_block_cfg=(PQ->bbpof<<16)|PQ->bblen;
	else
		pvch->bad_block_cfg=0;
	
	COMMIT(add_offset);
	COMMITU(xfer_cfg);
	COMMIT(packet_0_cfg);
	COMMIT(bad_block_cfg);

	pvch->flash_cmd=CMD_BOOT;
	COMMIT(flash_cmd);

	pj->biterrors=0;

	return pj->programmedsize;
}

#endif

RMstatus mlc_pa_probe(struct gbus *pgbus,RMuint32 f_sys,struct mlc_pa_job *pj) 
{ 
	struct mlc_cfg *pvch=&pj->vch,*gch=pj->cs?CHBREG_GA:CHAREG_GA;
	RMuint32 h=pj->cs?REG_BASE_host_interface+NAND_CS1:REG_BASE_host_interface+NAND_CS0;
	int i;
	char id[5];

	gbus_write_uint32_ireg(pgbus,REG_BASE_host_interface+PB_default_timing,0x20202020); // for the read id command?
	
	// read id
	gbus_write_uint32_ireg(pgbus,h,0x90); // id
	gbus_write_uint32_ireg(pgbus,h+4,0);
	
	for (i=0;i<(signed)sizeof(id);i++) 
		id[i]=gbus_read_uint8(pgbus,h+8);
	
	// 20 dc 10 95 54? can't find the spec
	// 20 d5 14 b6 44 [this is NAND16GW3D2A.pdf)
	PRINTF("jdec id=%z\n",id,sizeof(id));

	memcpy(&pj->jdecid,id,4); // hey it's 5 byte long
	
	COMMITU(device_cfg);
	COMMIT(timing1);
	COMMIT(timing2);

	//pvch->xfer_cfg.bits.dc= ?
	pvch->xfer_cfg.bits.sp=(pj->l2ps==9);
	pvch->xfer_cfg.bits.cesel=pj->cs;
	COMMITU(xfer_cfg);

	pvch->flash_cmd=CMD_RESET;
	COMMIT(flash_cmd);

	return RM_OK;
}

#ifndef MLC_PA_NODDR

RMstatus mlc_pa_sync_rawread (struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami)
{
	pj->vameta=NULL;
	pj->vabb=NULL;
	pj->vareport=NULL;
	
	while (pj->size>0) {
		RMuint32 ps=mlc_pa_trigger_rawread(pgbus,pj);

		if ((signed)ps==-1)
			return RM_PARAMETER_OUT_OF_RANGE; // most probably, reached out of device
		
		while (mlc_pa_request(pgbus,pj,whoami)==RM_BUSY);

		RMDBGLOG((DISABLE,"rawR ga=%p @%p %d bytes complete\n",pj->ga-ps,(RMuint32)pj->byteoffset-ps,ps));
		RMDBGLOG((DISABLE,"%02x %02x %02x %02x\n",gbus_read_uint8(pgbus,pj->ga-ps),gbus_read_uint8(pgbus,pj->ga-ps+1),gbus_read_uint8(pgbus,pj->ga-ps+2),gbus_read_uint8(pgbus,pj->ga-ps+3)));
	}

	return RM_OK;
}

RMstatus mlc_pa_sync_rawwrite(struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami)
{
	pj->vameta=NULL;
	pj->vabb=NULL;
	pj->vareport=NULL;

	while (pj->size>0) {
		RMuint32 ps=mlc_pa_trigger_rawwrite(pgbus,pj);

		if ((signed)ps==-1)
			return RM_PARAMETER_OUT_OF_RANGE; // most probably, reached out of device
		
		while (mlc_pa_request(pgbus,pj,whoami)==RM_BUSY);

		RMDBGLOG((DISABLE,"rawW ga=%p @%p %d bytes complete\n",pj->ga-ps,(RMuint32)pj->byteoffset-ps,ps));
	}

	return RM_OK;
}

int mlc_pa_sync_eccread (struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami)
{
	RMuint32 cumulated_errors=0;
	char *ovameta=(void *)pj->vameta,*ovabb=pj->vabb,*ovareport=pj->vareport;
	
	while (pj->size>0) {
		RMuint32 ps=-1;
		RMuint32 friendlysize;

		pj->biterrors=0;

		// first, try a whole page read...
		friendlysize=RMALIGNTO(pj->size,pj->l2ps);
		if (friendlysize>0) {
			RMuint32 keep=pj->size;
			pj->size=friendlysize;
			ps=mlc_pa_trigger_eccread(pgbus,pj);
			pj->size=keep;
		}
		
		// else use the sparse function... it's gonna bring you back to well aligned
		if ((signed)ps==-1) 
			ps=mlc_pa_trigger_sparseeccread(pgbus,pj);
		
		if ((signed)ps==-1) 
			return RM_PARAMETER_OUT_OF_RANGE; // most probably, reached out of device

		while (mlc_pa_request(pgbus,pj,whoami)==RM_BUSY);
		
		RMDBGLOG((DISABLE,"eccR ga=%p @%p %d (%d errors)\n",pj->ga-ps,(RMuint32)pj->byteoffset-ps,ps,pj->biterrors));
		RMDBGLOG((DISABLE,"%p %02x %02x %02x %02x\n",pj->ga-ps,gbus_read_uint8(pgbus,pj->ga-ps),gbus_read_uint8(pgbus,pj->ga-ps+1),gbus_read_uint8(pgbus,pj->ga-ps+2),gbus_read_uint8(pgbus,pj->ga-ps+3)));
		RMDBGLOG((DISABLE,"M %z\n",ovameta,(char *)pj->vameta-ovameta));
		pj->vameta=(void *)ovameta;
		RMDBGLOG((DISABLE,"B %z\n",ovabb,pj->vabb-ovabb));
		pj->vabb=(void *)ovabb;
		RMDBGLOG((DISABLE,"R %z\n",ovareport,pj->vareport-ovareport));
		pj->vareport=(void *)ovareport;

		if ((signed)cumulated_errors!=-1) {
			if ((signed)pj->biterrors==-1) {
				cumulated_errors=-1;
#ifdef MLC_ECCFAILGIVESUP
				return cumulated_errors;
#endif
			}
			else 
				cumulated_errors+=pj->biterrors;
		}
	}

	return cumulated_errors;
}


RMstatus mlc_pa_sync_eccwrite(struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami)
{
	char *ovameta=(void *)pj->vameta,*ovabb=pj->vabb;

	while (pj->size>0) {
		
		RMuint32 ps=mlc_pa_trigger_eccwrite(pgbus,pj);
		
		if ((signed)ps==-1)
			return RM_INVALID_PARAMETER; // unaligned, out of device, etc
		
		while (mlc_pa_request(pgbus,pj,whoami)==RM_BUSY);
		
		pj->vameta=(void *)ovameta;
		pj->vabb=(void *)ovabb;

		RMDBGLOG((DISABLE,"eccW ga=%p @%p %d\n",pj->ga-ps,(RMuint32)pj->byteoffset-ps,ps));
	}
	
	return RM_OK;
}

#else

int mlc_pa_sync_eccread (struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami)
{
	RMuint32 cumulated_errors=0;
	char *ovameta=(void *)pj->vameta,*ovabb=pj->vabb;
	
	while (pj->size>0) {
		RMuint32 ps=-1;

		pj->biterrors=0;

		ps=mlc_pa_trigger_sparseeccread(pgbus,pj);
		
		if ((signed)ps==-1) 
			return RM_PARAMETER_OUT_OF_RANGE; // most probably, reached out of device

		while (mlc_pa_request(pgbus,pj,whoami)==RM_BUSY);
		
		RMDBGLOG((ENABLE,"eccR ga=%p @%p %d (%d errors)\n",pj->ga-ps,(RMuint32)pj->byteoffset-ps,ps,pj->biterrors));
		RMDBGLOG((DISABLE,"%p %02x %02x %02x %02x\n",pj->ga-ps,gbus_read_uint8(pgbus,pj->ga-ps),gbus_read_uint8(pgbus,pj->ga-ps+1),gbus_read_uint8(pgbus,pj->ga-ps+2),gbus_read_uint8(pgbus,pj->ga-ps+3)));
		RMDBGLOG((DISABLE,"%z\n",ovameta,(char *)pj->vameta-ovameta));
		pj->vameta=(void *)ovameta;
		RMDBGLOG((DISABLE,"%z\n",ovabb,pj->vabb-ovabb));
		pj->vabb=(void *)ovabb;

		if ((signed)cumulated_errors!=-1) {
			if ((signed)pj->biterrors==-1) {
				cumulated_errors=-1;
#ifdef MLC_ECCFAILGIVESUP
				return cumulated_errors;
#endif
			}
			else 
				cumulated_errors+=pj->biterrors;
		}
		
		cumulated_errors+=pj->biterrors;
	}

	return cumulated_errors;
}

#endif

#ifndef MLC_READONLY
RMstatus mlc_pa_sync_eccblockerase(struct gbus *pgbus,struct mlc_pa_job *pj,RMuint32 whoami)
{
	pj->vameta=NULL;
	pj->vabb=NULL;
	pj->vareport=NULL;

	while (pj->size>0) {
		RMuint32 ps=-1;
		
		// gotta go one by one
		RMuint32 friendlysize=RMmin(pj->size,(unsigned)1<<pj->l2blocksize);
		if (friendlysize>0) {
			RMuint32 keep=pj->size;
			pj->size=friendlysize;
			ps=mlc_pa_trigger_eccblockerase(pgbus,pj);
			pj->size=keep;
		}
		
		if ((signed)ps==-1)
			return RM_INVALID_PARAMETER; // unaligned, out of device, etc
		
		while (mlc_pa_request(pgbus,pj,whoami)==RM_BUSY);

		PRINTF("E");
	}

	return RM_OK;
}
#endif

RMbool isbb_simplistic(struct gbus *pgbus,struct mlc_pa_job *_pj,RMuint32 cs,RMuint32 block,RMuint32 gascratch)
{
	struct mlc_pa_job J=*_pj,*pj=&J; // MAKE A COPY we dont want the rawread operations to move around _pj pointers.
	RMuint8 bb[6];
	
	pj->ga=gascratch;
	pj->byteoffset=sll64to64_le32((RMuint64)block,pj->l2blocksize);
	pj->vameta=NULL;
	pj->vabb=bb;
	
	/*
	  let's reach the bb info using eccread to be more friendly with the ddr-less build (no dma).
	  
	  On unformatted devices it's all gonna read wrong, but bb[] will be fetched properly.
	  You need to reach the last packet of the page, the one that has the bb segment.
	*/
	
	switch (pj->l2ps) {
	case 12:
		/*
		  MLC 4KB page MT29F32G08MAAWC spec says

		  System software should initially check the first spare area location on the first page 
		  of each block prior to performing any program or erase operations on the NAND Flash device. 
		  A bad-block table can then be created, enabling system software to map around these areas. 
		  Factory testing is performed under worst-case conditions. Because invalid blocks may be marginal, 
		  it may not be possible to recover the bad-block marking if the block is erased.

		  Location +4096, marker is : byte value is 0x00

		  Remark of Steven jan 11th
		  ... On the contrary for Samsung 16Gbit: K9GAG08B0M locates its bb marker onlast page ...
		*/
		pj->byteoffset+=4096-(1<<PACKET_SHIFT);
		pj->size=1;
		mlc_pa_sync_eccread(pgbus,pj,GBUS_MUTEX__PU);
		if (bb[0]==0x00) 
			return TRUE;
		
		pj->byteoffset+=4096+(1<<pj->l2blocksize)-(1<<pj->l2ps);
		pj->size=1;
		mlc_pa_sync_eccread(pgbus,pj,GBUS_MUTEX__PU);
		if (bb[0]==0x00) 
			return TRUE;
		break;
	case 11:
		/*
		  engineering discussions on this topic going on in
		  http://bugs.soft.sdesigns.com/show_bug.cgi?id=11898
		  
		  to factorize the code, this library might report a block is bad, while it's not bad per a specific part# spec
		  it will never report a block is good, while it's actually bad
		*/
		pj->byteoffset+=2048-(1<<PACKET_SHIFT);
		pj->size=6;
		mlc_pa_sync_eccread(pgbus,pj,GBUS_MUTEX__PU);
		
		if ( ((bb[0]!=0xff)&&(bb[5]!=0xff))
		     || (*(RMuint32 *)bb!=0xffffffff) )
			return TRUE;
		break;
	case 9:
		// one packet per page
		pj->size=1;
		mlc_pa_sync_eccread(pgbus,pj,GBUS_MUTEX__PU);
		if (bb[0]==0x00) 
			return TRUE;
		break;
	}

	return FALSE;
}

int bba2p(struct gbus *pgbus,struct mlc_pa_job *pj,isbb f,RMuint32 cs,RMuint32 gascratch,RMuint32 bbablock)
{
	int i,block;
	
	i=bbablock; // let i be the good block counter
	block=0;
	while (1) {
		if (!f(pgbus,pj,cs,block,gascratch)) {
			if (i==0)
				break; // you always exit this loop facing a good block
			i--;
		}
		
		block++;
	}
	
	return block;
}

int bbaeccread_PBLOCK(struct gbus *pgbus,struct mlc_pa_job *pj,isbb f,RMuint32 cs,RMuint32 ga,RMuint32 pblock,RMuint32 blockcount)
{
	/*
	  THIS FUNCTION TAKES THE PHYSICAL BLOCK# AS STARTING POINT ASSUMED A GOOD BLOCK.

	  When the ROM comes back to stage#0 with boot offset indicator, it is a physical address.

	  test_bbaeccewr() on the other hand takes a bbablock#.
	*/

	RMuint32 cumulated_errors=0;
	RMuint32 gascratch=ga+(blockcount<<pj->l2blocksize);
	RMuint32 left;

	pj->ga=ga;
	pj->byteoffset=sll64to64_le32((RMuint64)pblock,pj->l2blocksize);
	
	left=blockcount<<pj->l2blocksize;
	while (1) {
		RMuint32 berc;
		pj->size=1<<pj->l2blocksize;
		berc=mlc_pa_sync_eccread(pgbus,pj,GBUS_MUTEX__PU);
		//PRINTF("R%p%08x %d\n",P64(pj->byteoffset-(1<<pj->l2blocksize)),berc);
		if ((signed)berc==-1)
			return -1;

		cumulated_errors+=berc;

		left-=1<<pj->l2blocksize;
		if (left==0)
			break;
		
		pblock++;
		while (f(pgbus,pj,cs,pblock,gascratch)) {
			PRINTF("\x1b[34mskip %d!\x1b[0m\n",pblock);
			pj->byteoffset+=1<<pj->l2blocksize;
			pblock++;
		}
	}

	return cumulated_errors;
}
