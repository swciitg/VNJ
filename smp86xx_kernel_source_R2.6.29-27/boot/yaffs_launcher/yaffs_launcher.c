/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/*
 * TangoX YAFFS launcher applet
 */

#include "yaffs_launcher.h"

#include "yaffs_client.h"

#include "zboot_yaffs_params.h"

#include "boot_auth.h"

#define REMAPPED_REG	   CPU_remap4
#define REMAPPED_BASE	   CPU_remap4_address

#define TANGOX_BASE_FREQUENCY 27000000

#define LOG2_RM_PAGE_SIZE 12
#define RM_PAGE_SIZE (1<<LOG2_RM_PAGE_SIZE)

#define LR_XOS2K_C2X             0x00000900
#define LR_XOS2K_X2C             0x00000a00

#define LRRW_XENV_LOC        0xa0061a00
#define LRRO_XENV_LOC        0xa0061d00

#ifndef PHY_NUM_RSVD_BLKS
#ifndef XBOOT3
#define PHY_NUM_RSVD_BLKS 4
#else
#define PHY_NUM_RSVD_BLKS 8
#endif	/* XBOOT3 */
#endif	/* PHY_NUM_RSVD_BLKS */

#define NAND_LARGEPAGE_BLOCKSIZE 0x20000
#define NAND_SMALLPAGE_BLOCKSIZE 0x4000

#ifndef XBOOT3
#define NAND_MINIMUM_RSVD_SPACE 0x80000
#define NAND_SMALLPAGE_MINIMUM_RSVD_PBLKS 32
#define NAND_LARGEPAGE_MINIMUM_RSVD_PBLKS 4
#define NAND_MINIMUM_RSVD_PBLKS 4 /* Need to have at least 4 blocks, for 4 separate copies of phyblock image */
#else
#define NAND_MINIMUM_RSVD_SPACE 0x100000
#define NAND_SMALLPAGE_MINIMUM_RSVD_PBLKS 64
#define NAND_LARGEPAGE_MINIMUM_RSVD_PBLKS 8
#define NAND_MINIMUM_RSVD_PBLKS 6 /* Need to have at least 6 blocks, see Smp8656BootMap twiki doc. */
#endif


#define YAFFS_BASE_PARTITION_PATH  "/yaffs2"

typedef RMint32 (*FUNC_PTR) (void *, void *, void *, void *);

extern RMuint32 cpu_uart_base;
struct gbus *pgbus = (void *) KSEG1ADDR(REG_BASE_cpu_block + CPU_remap);

RMuint32 g_yaffs_launcher_heap;
RMuint32 g_yaffs_launcher_readbuffer;
RMuint32 g_yaffs_launcher_decryptbuffer;

int yaffs_launcher_chip_sel;

int yaffs_launcher_start_blk;
int yaffs_launcher_end_blk;


RMint32 kunzip(RMascii *zaddr, RMascii *loadaddr, RMuint32 heapaddr, RMint32 do_crc);


static void cachedcodechange(void)
{
	dcache_wi_();
	icache_i_();
}

static void onems(void)
{
	RMuint32 x0, x1;

	x0 = gbus_readl(REG_BASE_system_block + SYS_xtal_in_cnt);
	do {
		x1 = gbus_readl(REG_BASE_system_block + SYS_xtal_in_cnt);
	}
	while ((x1 - x0) < 27000);
}


static void pbi_init(void)
{
	RMuint32 i;

	// CS#0 base + offset for current CS
	volatile char *there=(void *)(0xa0020000);

	// correct for 8654 ES2 @300MHz sysclk or lower. Others: don't know
	*(RMuint32 *)(there+0x834)=0x00330003;
	*(RMuint32 *)(there+0x800)=0x03080202;
	*(RMuint32 *)(there+0x81c)=0x3f3;
	
	*(RMuint32 *)(0xa002093c) = 0x40; /* <HACK> Fix for 8644.  No documentation on this yet */

	for (i = 0; i < 2; i++) {
		volatile char *cs_rst = there + (0x200 * i);

		// Reset
		*(RMuint32 *)(cs_rst) = 0xff;
		/* Must wait >= 5us for NAND flash(es) to settle after issuing Reset */
		onems();
	}
}


/// 
/**
   We assume that this applet is built and placed in a position above the space ezboot occupied,
   and we'll re-use the heap/read/decrypt buffers of ezboot.  We assume that ezboot was loaded
   to virtual address 0x85600000 and its code+stack occupies 256KB, and thus based on the applet's
   own virtual address, we'll divide up the available space.

   The heap gets 1/17th of the space, the read buffer and decrypt buffer each get 8/17ths of the
   available space.

   The buffers we setup here must be 16-byte aligned.

   @param void
*/

static void initialize_buffers(void)
{
#define EZBOOT_VA          0x85600000
#define EZBOOT_ITSELF_SIZE 0x40000
#define EZBOOT_ITSELF_END  ((EZBOOT_VA) + (EZBOOT_ITSELF_SIZE))
	RMuint32 total_space;
	RMuint32 heap_size, readbuffer_size, decryptbuffer_size;

	total_space = APPLET_ADDR - EZBOOT_ITSELF_END;

	heap_size = RMALIGNTO(total_space / 17, 4);
	readbuffer_size = decryptbuffer_size = RMALIGNTO((total_space * 8) / 17, 4);

	g_yaffs_launcher_heap = EZBOOT_ITSELF_END;
	g_yaffs_launcher_readbuffer = g_yaffs_launcher_heap + heap_size;
	g_yaffs_launcher_decryptbuffer = g_yaffs_launcher_readbuffer + readbuffer_size;

	uart_printf("Initialized buffers:\n");
	uart_printf("  Heap@0x%08lx, size=0x%08lx\n", g_yaffs_launcher_heap, heap_size);
	uart_printf("  Read Buffer@0x%08lx, size=0x%08lx\n", g_yaffs_launcher_readbuffer, readbuffer_size);
	uart_printf("  Decrypt Buffer@0x%08lx, size=0x%08lx\n", g_yaffs_launcher_decryptbuffer, decryptbuffer_size);
}


static RMstatus yaffs_launcher_readfile(RMascii *filepath, RMuint8 *buffer, RMuint32 *r_filesize)
{
	int fd;
	RMuint32 file_size;

	// Get size of file
	{
		struct yaffsclient_stat stats;
		if (yaffsclient_stat(filepath, &stats)) {
			uart_printf("Unable to get file size of %s!  Aborting!\n", filepath);
			return RM_ERROR;
		}
		file_size = stats.st_size;
		uart_printf("File size of %s retrieved, size is %d bytes.\n", filepath, file_size);
	}
	
	fd = yaffsclient_open(filepath, O_RDONLY, 0);
	if (fd < 0) {
		uart_printf("Failed to open %s !  Aborting!\n", filepath);
		return RM_ERROR;
	}
	uart_printf("Opened %s for read-only...\n", filepath);
	
	if (yaffsclient_read(fd, (void *)buffer, file_size) != file_size) {
		uart_printf("Error reading the file contents!  Aborting!\n");
		yaffsclient_close(fd);
		return RM_ERROR;
	}
	uart_printf("Successfully read the contents of %s to 0x%08lx\n", filepath, buffer);

	yaffsclient_close(fd);

	*r_filesize = file_size;

	return RM_OK;
}


static void yaffs_launcher_process_entry(struct zboot_yaffs_params *yaffs_params)
{
	RMuint32 file_size;

	RMascii *path_name;
	RMuint32 flags;
	RMuint32 load_addr, start_addr;

	RMuint32 tmpbuf_addr;
	RMuint32 sigbuf_addr;
	
	yaffs_launcher_chip_sel = yaffs_params->chip_sel;
	yaffs_launcher_start_blk = yaffs_params->start_block;
	yaffs_launcher_end_blk = yaffs_params->end_block;
	path_name = yaffs_params->path_name;
	flags = yaffs_params->flags;
	load_addr = yaffs_params->load_addr;
	start_addr = yaffs_params->start_addr;
	
	uart_printf("  Chip Select = %d\n", yaffs_launcher_chip_sel);
	uart_printf("  Start Block = %d\n", yaffs_launcher_start_blk);
	uart_printf("  End Block   = %d\n", yaffs_launcher_end_blk);
	uart_printf("  path_name   = %s\n", path_name);
	uart_printf("  flags       = 0x%08lx\n", flags);
	uart_printf("  load addr   = 0x%08lx\n", load_addr);
	uart_printf("  start addr  = 0x%08lx\n", start_addr);
	
	/* If it's zipped or encrypted, we need to read to the readbuffer first, otherwise read it
	   directly to the load address. */
	tmpbuf_addr = (flags & (ZBOOT_YAFFS_FLAGS_GZIP | ZBOOT_YAFFS_FLAGS_ENCRYPT)) ?
		g_yaffs_launcher_readbuffer : load_addr;
	sigbuf_addr = g_yaffs_launcher_decryptbuffer;

	/* Use YAFFS, read out a file, and launch it */
	// Mount as read-only
	if (yaffsclient_mount2(YAFFS_BASE_PARTITION_PATH, TRUE)) {
		uart_printf("Failed to mount " YAFFS_BASE_PARTITION_PATH " !  Aborting!\n");
		return;
	}
	uart_printf("Mounted " YAFFS_BASE_PARTITION_PATH " ...\n");

	if (yaffs_launcher_readfile(path_name, (RMuint8 *)tmpbuf_addr, &file_size) != RM_OK)
		goto unmount_partition;

#ifdef USE_BOOT_AUTH
	/* If execution flag is set, but payload isn't encrypted, deny and abort */
	if ((flags & ZBOOT_YAFFS_FLAGS_EXEC) && ((flags & ZBOOT_YAFFS_FLAGS_ENCRYPT) == 0)) {
		uart_printf("Execution flag is set, but not the encrypt flag, aborting!");
		goto unmount_partition;
	}
#endif

	if (flags & ZBOOT_YAFFS_FLAGS_ENCRYPT) {
#ifdef USE_BOOT_AUTH
		RMascii sigfilename[260];
		RMuint32 sigfile_size;

		/* <NOTE> Users have two ways to proceed for an encrypted/signed payload.  One is to use boot_auth, which
		   uses RSA on the SHA1 hash of the payload.  That is what is built into the applet by default.

		   We don't support payloads encrypted/signed using Sigma's authentication mechanisms.
		   A customer wishing to use Sigma's authentication mechanisms within this applet would need
		   to acquire a license for YAFFS, and then include code here to carry out the
		   decryption/signature verification.  That code would be drawn from ezboot's source code, which is non-GPL.
		*/

		// Read the signature file.  Filename of sig file is implied to be the name of the payload file with
		// ".sig" appended to it.
		strncpy(sigfilename, path_name, sizeof(yaffs_params->path_name));
		strcat(sigfilename, ".sig");
		if (yaffs_launcher_readfile(sigfilename, (RMuint8 *)sigbuf_addr, &sigfile_size) != RM_OK)
			goto unmount_partition;

		if (boot_auth((RMuint8 *)sigbuf_addr, sigfile_size, (RMuint8 *)tmpbuf_addr, file_size) != 0) {
			uart_printf("Authentication failed!  Aborting!\n");
			goto unmount_partition;
		}
#else
		uart_printf("Encryption/Authentication is disabled.\n");
		goto unmount_partition;
#endif
	}
		
	if (flags & ZBOOT_YAFFS_FLAGS_GZIP) {
		uart_printf("Decompressing image from 0x%08lx to 0x%08lx\n", tmpbuf_addr, load_addr);
		if (kunzip((RMascii *)tmpbuf_addr, (RMascii *)load_addr, g_yaffs_launcher_heap, 1) < 0) {
			uart_printf("Decompression failed!  Aborting...\n");
			goto unmount_partition;
		}
		uart_printf("Decompression OK!\n");
	}

#ifndef USE_BOOT_AUTH
	/* authentication is not used, so we launch it if needed */
	if (flags & ZBOOT_YAFFS_FLAGS_EXEC) {
		uart_printf("Going to launch image @0x%08lx\n", start_addr);
		
		/* File successfully authenticated, launch it! */
		// Flush caches
		cachedcodechange();
		// Go!
		(*((FUNC_PTR) start_addr)) (NULL, NULL, NULL, NULL);
	}
#else
	if (flags & ZBOOT_YAFFS_FLAGS_EXEC) {
		/* Only allow execution if encrypted flag was set.  If we get here, authentication was
		   performed and succeeded */
		if (flags & ZBOOT_YAFFS_FLAGS_ENCRYPT) {
			uart_printf("Going to launch image @0x%08lx\n", start_addr);
			
			/* File successfully authenticated, launch it! */
			// Flush caches
			cachedcodechange();
			// Go!
			(*((FUNC_PTR) start_addr)) (NULL, NULL, NULL, NULL);
		}
		else {
			uart_printf("Execution not allowed for unencrypted or unsigned payloads!  Aborting!\n");
			goto unmount_partition;
		}
	}
#endif

 unmount_partition:
	yaffsclient_unmount2(YAFFS_BASE_PARTITION_PATH, 1);
}


/* Entry point of the applet */
int applet_entry(RMuint32 container_addr, void *dummy1, void *dummy2)
{
	RMuint32 yaffs_params_ga = container_addr;
	struct zboot_yaffs_params yaffs_params;

	RMuint32 param_entry_idx;

	{			/* If the chip is 8656 or 8658, change the UART0 base */
		RMuint32 chip_id = gbus_read_uint32(pgbus, REG_BASE_host_interface + PCI_REG0) & 0xfffe;
		if ((chip_id == 0x8656) || (chip_id == 0x8658))
			cpu_uart_base = REG_BASE_system_block + 0x700;
	}

	uart_printf("**** YAFFS launcher applet (version %s) starts ****\n", APPLET_VERSION);
	uart_printf("CPU uart base = 0x%08lx\n", cpu_uart_base);

	pbi_init();		/* Setup PBI with some conservative settings */

	initialize_buffers();

	if (!yaffs_params_ga) {
		uart_printf("No YAFFS parameters passed in, aborting!\n");
		goto endless_sleep;
	}

	uart_printf("YAFFS parameters passed in @ga=0x%08lx...\n", yaffs_params_ga);

	for (param_entry_idx = 0; TRUE; param_entry_idx++) {
		RMuint32 i;
		RMuint32 param_entry_offset = param_entry_idx * sizeof(struct zboot_yaffs_params);
		RMuint32 param_entry_ga = yaffs_params_ga + param_entry_offset;
		for (i = 0; i < sizeof(struct zboot_yaffs_params); i++) {
			((RMuint8 *)(&yaffs_params))[i] = gbus_read_uint8(pgbus, (RMuint32)(param_entry_ga + i));
		}

		yaffs_launcher_process_entry(&yaffs_params);

		if ((yaffs_params.flags & ZBOOT_YAFFS_FLAGS_CONT_NOTLAST) == 0)
			break;
	}

	uart_printf("**** YAFFS launcher applet ends ****\n");

 endless_sleep:
	while (1);		/* Hang here, never return! */

	return RM_OK;
}
