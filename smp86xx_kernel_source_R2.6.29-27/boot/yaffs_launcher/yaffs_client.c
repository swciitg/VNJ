/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   yaffs_client.c
  @brief  

  Implementation of the client of the YAFFS library.  Acts mainly as a wrapper.

  @author Yufei Leung
  @date   2010-10-19
*/

#ifndef __YAFFS_CLIENT_C__
#define __YAFFS_CLIENT_C__

#include "yaffs2/yaffs_guts.h"

#include "yaffs_client.h"

int yaffsclient_mount2(const char *path, int readOnly)
{
	static int yaffs_started = 0;

	if (!yaffs_started) {
		if (yaffs_start_up() != 0)
			return -1;
		yaffs_started = 1;
	}

	return yaffs_mount2(path, readOnly);
}


int yaffsclient_unmount2(const char *path, int force)
{
	return yaffs_unmount2(path, force);
}


int yaffsclient_stat(const char *path, struct yaffsclient_stat *buf)
{
	struct yaffs_stat stats;

	if (yaffs_stat(path, &stats))
		return -1;

	buf->st_dev = stats.st_dev;
	buf->st_ino = stats.st_ino;
	buf->st_mode = stats.st_mode;
	buf->st_nlink = stats.st_nlink;
	buf->st_uid = stats.st_uid;
	buf->st_gid = stats.st_gid;
	buf->st_rdev = stats.st_rdev;
	buf->st_size = stats.st_size;

	buf->st_blksize = stats.st_blksize;
	buf->st_blocks = stats.st_blocks;
	buf->st_size = stats.st_size;

#ifdef CONFIG_YAFFS_WINCE
	/* Special 64-bit times for WinCE */
	buf->yst_wince_atime[0] = stats.yst_wince_atime[0];
	buf->yst_wince_atime[1] = stats.yst_wince_atime[1];
	buf->yst_wince_mtime[0] = stats.yst_wince_mtime[0];
	buf->yst_wince_mtime[1] = stats.yst_wince_mtime[1];
	buf->yst_wince_ctime[0] = stats.yst_wince_ctime[0];
	buf->yst_wince_ctime[1] = stats.yst_wince_ctime[1];
#else
	buf->yst_atime = stats.yst_atime;    /* time of last access */
	buf->yst_mtime = stats.yst_mtime;    /* time of last modification */
	buf->yst_ctime = stats.yst_ctime;    /* time of last change */
#endif

	return 0;
}


int yaffsclient_open(const char *path, int oflag, int mode)
{
	return yaffs_open(path, oflag, mode);
}


int yaffsclient_read(int fd, void *buf, unsigned int nbyte)
{
	return yaffs_read(fd, buf, nbyte);
}


int yaffsclient_close(int fd)
{
	return yaffs_close(fd);
}


#endif /* __YAFFS_CLIENT_C__ */
