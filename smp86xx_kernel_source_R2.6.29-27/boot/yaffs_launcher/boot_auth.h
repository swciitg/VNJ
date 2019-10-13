/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   boot_auth.h
  @ingroup application
  @brief  Boot Authentication tool

  @author David Bryson
  @date   02-17-2006
*/

#ifndef __BOOT_AUTH_H__
#define __BOOT_AUTH_H__

#define MAX_BUFFER_BYTE_SIZE		256
#define MAX_KEY_WORD_SIZE		64
#define MAX_FILENAME_SIZE		128
#define	ASN_SHA1_SIG_LEN		15
#define SHA1_DIGEST_SIZE		20

struct runtime_args {
	int		n_len;
	int		e_len;
	int		sig_len;
	unsigned long	*n; 		// public key (modulo)
	unsigned long	e; 		// public exponent, hardcoded 65537
	unsigned char	*sig;  		// SHA1 signature
};

/// 
/**
   Authenticates the given payload, against the given signature.

   @param sig:    Signature image
   @param sig_sz: Size of signature in bytes
   @param payload:    Payload image
   @param payload_sz: Size of payload in bytes
   @return 0 on success, -1 on failure.
*/
int boot_auth(const unsigned char *sig, const unsigned int sig_sz,
	      const unsigned char *payload, const unsigned int payload_sz);

#endif // __BOOT_AUTH_H__
