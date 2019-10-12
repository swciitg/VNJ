/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

struct gbus;

extern struct gbus *pgbus;

RMuint8  gbus_read_uint8 (struct gbus *h, RMuint32 byte_address);
RMuint16 gbus_read_uint16(struct gbus *h, RMuint32 byte_address);
RMuint32 gbus_read_uint32(struct gbus *h, RMuint32 byte_address);
void gbus_write_uint8 (struct gbus *h, RMuint32 byte_address, RMuint8  data);
void gbus_write_uint16(struct gbus *h, RMuint32 byte_address, RMuint16 data);
void gbus_write_uint32(struct gbus *h, RMuint32 byte_address, RMuint32 data);

#define gbus_write_data8(h,ga,va,s) memcpy_v2g(h,(RMuint8 *)(ga),va,s)
#define gbus_write_data16(h,ga,va,s) memcpy_v2g(h,(RMuint8 *)(ga),(RMuint8 *)(va),(s)*2)
#define gbus_write_data32(h,ga,va,s) memcpy_v2g(h,(RMuint8 *)(ga),(RMuint8 *)(va),(s)*4)

#define gbus_read_data8(h,ga,va,s) memcpy_g2v(h,va,(RMuint8 *)ga,s)
#define gbus_read_data16(h,ga,va,s) memcpy_g2v(h,(RMuint8 *)(va),(RMuint8 *)(ga),(s)*2)
#define gbus_read_data32(h,ga,va,s) memcpy_g2v(h,(RMuint8 *)(va),(RMuint8 *)(ga),(s)*4)
