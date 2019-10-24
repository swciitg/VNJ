/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   boot_auth.c
  @ingroup application
  @brief  Boot Authentication tool

  @author David Bryson
  @date   02-17-2006
*/

#define ALLOW_OS_CODE 1
#define LOCALDBG DISABLE

#include "includes/rmdef.h"
#include "includes/rmdbg.h"
#include "includes/util.h"
#include "includes/uart.h"

#include "fastsha.h"
#include "rsa_enc_dec.h"

#include "boot_auth.h"
#include "debug.h"

#define VERBOSE(fmt, ...) uart_printf(fmt, ##__VA_ARGS__); 

static struct runtime_args __rtime_args;
static unsigned int __sig[MAX_KEY_WORD_SIZE*4/sizeof(unsigned int)];
static const unsigned char asn_sha1_sig[ASN_SHA1_SIG_LEN] = {0x30, 0x21, 0x30, 0x09,
						0x06, 0x05, 0x2b, 0x0e,
						0x03, 0x02, 0x1a, 0x05,
						0x00, 0x04, 0x14};

/**
  Copies data from a PKCS_v1.5 padded buffer into a smaller buffer without
  padding.
  
  @param data : a buffer to copy to data to
  @param data_len : size of the buffer to copy the data to, in bytes
  @param pad_buffer : the buffer containing the padded data
  @param pad_buffer_len : size of the buffer containing padded data
  @return -1 if there was an error copying, or reading the buffer.
  Otherwise returns a value > 0 that is the length of the unpadded data.
 */
static int pkcs15_unpad(unsigned char *data, int data_len, unsigned char* pad_buffer, int pad_buffer_len) {
	int i;
	if ((pad_buffer[0] == 0x00) && (pad_buffer[1] == 0x01)) {
		DEBUG_LOG(0,"byte 0 = %02x\nbyte 1 = %02x\n", pad_buffer[0], pad_buffer[1]);
		// PKCS v1.5 01 standard, this is the padding we support
		// moves over 0xff until it his the 0x00 denoting end of pad
		for(i=2; (pad_buffer[i] != 0x00) && (i < pad_buffer_len) ; i++) {};
		DEBUG_LOG(0,"Found padding in message until byte %d.\n", i);
		DEBUG_LOG(0,"Checking for ASN SHA1 signature.\n", i);

		if (memcmp(asn_sha1_sig, pad_buffer + i + 1, ASN_SHA1_SIG_LEN) != 0) {
			DEBUG_LOG(0,"Did not find ASN encoded SHA1 signature!\n");	
			return -1;
		}

		
		if (data_len < (pad_buffer_len - i)) {
			DEBUG_LOG(0,"Not enough room to copy %d bytes to %d bytes.\n", pad_buffer - i, data_len);
			return -1;
		}
		// +1 offset for the last 00 pad byte
		memcpy(data, pad_buffer + i + 1 + ASN_SHA1_SIG_LEN, pad_buffer_len - i);

	} else {
		VERBOSE("File is not PKCS 1.5 padded\nOnly PKCS v1.5 padding supported\n");
		return -1;
	}
	return pad_buffer_len - i - 1;
}

/**
 Swaps the bytes at a, b.
  
  @param a : byte a
  @param b : byte b
  @return returns 0 in all cases
 */
static int byteswap(unsigned char* a, unsigned char* b) {
	unsigned char temp;

	temp = *a;
	*a = *b;
	*b = temp;

	return 0;
}

/**
  Reverses the byte ordering for a buffer of memory.  Used to convert a
  memory block from BigEndian to LittleEndian.
  
  @param bottom : bottom of the buffer(lower memory address)
  @param top : top of the buffer(higher memory address)
  @return 0 in all cases
 */
static int revbytes(unsigned char* bottom, unsigned char* top) {
	byteswap(bottom, top);
	if (bottom+1 == top)
		return 0;
	else
		revbytes(bottom+1, top-1);

	return 0;
}

/**
   Authenticates the given payload, against the given signature.

   @param sig:    Signature image
   @param sig_sz: Size of signature in bytes
   @param payload:    Payload image
   @param payload_sz: Size of payload in bytes
   @return 0 on success, -1 on failure.
*/
int boot_auth(const unsigned char *sig, const unsigned int sig_sz,
		const unsigned char *payload, const unsigned int payload_sz) {
	int	 	i;
	int		ret_val = 0;	
	int		buf_data;
	unsigned char 	run_hash[MAX_BUFFER_BYTE_SIZE];
	unsigned char	buf[MAX_BUFFER_BYTE_SIZE];
	struct sha1_ctx	sha1_ctx;
	struct runtime_args *args = &__rtime_args;
	extern const char __mykey_start, __mykey_end;
	
	if (sig_sz > (MAX_KEY_WORD_SIZE*4)) {
		ret_val = -1;
		goto cleanup;
	}

	memset(args, 0, sizeof(struct runtime_args));
	args->e = 65537;

	args->n = (unsigned long *)&__mykey_start;
	args->n_len = (unsigned int)(&__mykey_end) - (unsigned int)(&__mykey_start);

	args->sig = (unsigned char *)__sig;
	args->sig_len = sig_sz;
	memcpy(args->sig, sig, sig_sz);
	revbytes(args->sig, args->sig + (args->sig_len - 1));

	args->e_len = payload_sz;

	sha1_init(&sha1_ctx);
	sha1_input(&sha1_ctx, payload, payload_sz);
	sha1_done(&sha1_ctx);

	memcpy(run_hash, sha1_ctx.digest, SHA1_DIGEST_SIZE);

	revbytes(run_hash, run_hash + (SHA1_DIGEST_SIZE - 1));

	VERBOSE("Calculated Hash: ");
	for(i = 0; i < SHA1_DIGEST_SIZE; i++) {
		VERBOSE("%02x", run_hash[i]);
	}
	VERBOSE("\n");

	VERBOSE("Signed Hash:  ");
	for(i = 0; i < args->sig_len; i++) {
		if (!(i % 32)) VERBOSE("\n");
		VERBOSE("%02x", args->sig[i]);
	}
	VERBOSE("\n");

	rsa_enc_dec((unsigned long*)args->sig, (const unsigned long*)args->n,
			(args->n_len / 4), (const unsigned long*)&args->e, 1);

	revbytes(args->sig, args->sig + (args->sig_len -1));

	// how many bytes of unpadded data do we have ?
	if ((buf_data = pkcs15_unpad(buf, MAX_BUFFER_BYTE_SIZE, args->sig, args->sig_len)) == -1) {
		ret_val = -1;
		goto cleanup;
	}
	
	VERBOSE("Decrypted Hash:  ");
	for(i = 0; i < SHA1_DIGEST_SIZE; i++) {
		if (!(i % 32)) VERBOSE("\n");
		VERBOSE("%02x", buf[i]);
	}
	VERBOSE("\n");
	
	if (memcmp(run_hash, buf, SHA1_DIGEST_SIZE) == 0) {
		VERBOSE("Payload authenticated\n");
	} else { 
		VERBOSE("Payload did not authenticate!\n");
		ret_val = -1;
	}

cleanup:	
	return ret_val;
}

