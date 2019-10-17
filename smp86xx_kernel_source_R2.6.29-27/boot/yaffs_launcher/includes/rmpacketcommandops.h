/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   rmpacketcommandops.h

  @author Emmanuel Michon
  @date   2002-12-06
*/

#ifndef __RMPACKETCOMMANDOPS_H__
#define __RMPACKETCOMMANDOPS_H__

RM_EXTERN_C_BLOCKSTART

typedef struct {
	RMuint8 senseKey;
	RMuint8 asc;
	RMuint8 ascq;
} RMpacketError;

typedef enum {
        RM_PACKET_DATA_WRITE = 1,
	RM_PACKET_DATA_READ  = 2,
} RMpacketDataDirection;

typedef struct {
	RMuint8 cmd[12];
	RMuint8 *buffer;
	RMuint32 buflen;
	RMpacketError packetError;
	RMpacketDataDirection packetDataDirection;
} RMpacketCommand;

typedef RMint32 (*PacketCommandOp)(void *handle,RMpacketCommand *pP);

RM_EXTERN_C_BLOCKEND

#endif // __RMPACKETCOMMANDOPS_H__
