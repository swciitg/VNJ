/***********************************************************************
 * Copyright (C) 2005-2011 Sigma Designs, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 **********************************************************************/

/**
  @file   yaffs_client.h
  @brief  

  Interface to the client of the YAFFS library.  Acts mainly as a wrapper.

  @author Yufei Leung
  @date   2010-10-19
*/

#ifndef __YAFFS_CLIENT_H__
#define __YAFFS_CLIENT_H__


struct yaffsclient_stat {
	int           st_dev;      /* device */
	int           st_ino;      /* inode */
	unsigned      st_mode;     /* protection */
	int           st_nlink;    /* number of hard links */
	int           st_uid;      /* user ID of owner */
	int           st_gid;      /* group ID of owner */
	unsigned      st_rdev;     /* device type (if inode device) */
	unsigned long st_size;     /* total size, in bytes */
	unsigned long st_blksize;  /* blocksize for filesystem I/O */
	unsigned long st_blocks;   /* number of blocks allocated */
#ifdef CONFIG_YAFFS_WINCE
	/* Special 64-bit times for WinCE */
	unsigned long yst_wince_atime[2];
	unsigned long yst_wince_mtime[2];
	unsigned long yst_wince_ctime[2];
#else
	unsigned long yst_atime;    /* time of last access */
	unsigned long yst_mtime;    /* time of last modification */
	unsigned long yst_ctime;    /* time of last change */
#endif
};


/// 
/**
   Mount a YAFFS partition

   @param path  
   @param readOnly      
   @return 0 on success, -1 on failure.
*/
int yaffsclient_mount2(const char *path, int readOnly);


/// 
/**
   Unmount a YAFFS partition

   @param path  
   @param force 
   @return 0 on success, -1 on failure.
*/
int yaffsclient_unmount2(const char *path, int force);


/// 
/**
   Get file statistics

   @param path  
   @param buf   
   @return 0 on success, -1 on failure.
*/
int yaffsclient_stat(const char *path, struct yaffsclient_stat *buf);


/// 
/**
   Open a file, get a file handle.

   @param path  
   @param oflag 
   @param mode  
   @return A file descriptor, or <0 on failure.
*/
int yaffsclient_open(const char *path, int oflag, int mode);


/// 
/**
   Read the contents of a file

   @param fd    
   @param buf   
   @param nbyte 
   @return the number of bytes actually read, or <0 on error.
*/
int yaffsclient_read(int fd, void *buf, unsigned int nbyte);


/// 
/**
   Close a file handle.

   @param fd    
   @return 0 on success, -1 on failure.
*/
int yaffsclient_close(int fd);


/*
 * Mode and flags definitions
 */

#ifndef O_RDONLY
#define O_RDONLY        00
#endif

#ifndef O_WRONLY
#define O_WRONLY	01
#endif

#ifndef O_RDWR
#define O_RDWR		02
#endif

#ifndef O_CREAT		
#define O_CREAT 	0100
#endif

#ifndef O_EXCL
#define O_EXCL		0200
#endif

#ifndef O_TRUNC
#define O_TRUNC		01000
#endif

#ifndef O_APPEND
#define O_APPEND	02000
#endif

#ifndef SEEK_SET
#define SEEK_SET	0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif

#ifndef SEEK_END
#define SEEK_END	2
#endif

#ifndef EBUSY
#define EBUSY	16
#endif

#ifndef ENODEV
#define ENODEV	19
#endif

#ifndef EINVAL
#define EINVAL	22
#endif

#ifndef EBADF
#define EBADF	9
#endif

#ifndef EACCES
#define EACCES	13
#endif

#ifndef EXDEV	
#define EXDEV	18
#endif

#ifndef ENOENT
#define ENOENT	2
#endif

#ifndef ENOSPC
#define ENOSPC	28
#endif

#ifndef ERANGE
#define ERANGE 34
#endif

#ifndef ENODATA
#define ENODATA 61
#endif

#ifndef ENOTEMPTY
#define ENOTEMPTY 39
#endif

#ifndef ENAMETOOLONG
#define ENAMETOOLONG 36
#endif

#ifndef ENOMEM
#define ENOMEM 12
#endif

#ifndef EEXIST
#define EEXIST 17
#endif

#ifndef ENOTDIR
#define ENOTDIR 20
#endif

#ifndef EISDIR
#define EISDIR 21
#endif


// Mode flags

#ifndef S_IFMT
#define S_IFMT		0170000
#endif

#ifndef S_IFLNK
#define S_IFLNK		0120000
#endif

#ifndef S_IFDIR
#define S_IFDIR		0040000
#endif

#ifndef S_IFREG
#define S_IFREG		0100000
#endif

#ifndef S_IREAD 
#define S_IREAD		0000400
#endif

#ifndef S_IWRITE
#define	S_IWRITE	0000200
#endif

#ifndef S_IEXEC
#define	S_IEXEC	0000100
#endif

#ifndef XATTR_CREATE
#define XATTR_CREATE 1
#endif

#ifndef XATTR_REPLACE
#define XATTR_REPLACE 2
#endif

#ifndef R_OK
#define R_OK	4
#define W_OK	2
#define X_OK	1
#define F_OK	0
#endif


#endif /* __YAFFS_CLIENT_H__ */
