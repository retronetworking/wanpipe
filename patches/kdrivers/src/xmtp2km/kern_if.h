/****************************************************************************
 *
 * kern_if.h :
 *
 * Protoypes for kernel function wrappers.
 *
 * Copyright (C) 2004  Xygnada Technology, Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 ****************************************************************************/
/* protoypes of functions in the public part of the xmtp2km kernel module */

#ifndef __XMTP2_KERN_IF__
#define __XMTP2_KERN_IF__


enum {
	XMTP2_VERIFY_READ,
	XMTP2_VERIFY_WRITE
};


int xmtp2km_access_ok   (int type, const void *p_addr, unsigned long n);
unsigned long xmtp2km_copy_to_user   (void *p_to, const void *p_from, unsigned long n);
unsigned long xmtp2km_copy_from_user (void *p_to, const void *p_from, unsigned long n);
int xmtp2km_strncmp (const char *p_s1, const char *p_s2, const unsigned int n);
void * xmtp2km_memset(void * p_mb, const int init_value, const unsigned int n);
void * xmtp2km_memcpy(void * p_to, const void * p_from, const unsigned int n);
void   xmtp2_printk(const char * fmt, ...);
void* xmtp2_memset(void *b, int c, int len);

void xmtp2km_spin_lock_init(void);
void xmtp2km_spin_lock_irq(unsigned long *flag);
void xmtp2km_spin_unlock_irq(unsigned long *flag);
void xmtp2km_spin_lock(void);
void xmtp2km_spin_unlock(void);

int xmtp2km_rate_limit(void);
void xmtp2_mdelay(int ms);
int xmtp2km_ioctl_close(void);

#ifdef XMTP2_MEM_DEBUG
void * __xmtp2km_kmalloc (const unsigned int bsize, char *func, int line);
void __xmtp2km_kfree (void * p_buffer, char *func, int line);
#define xmtp2km_kmalloc(size) __xmtp2km_kmalloc(size,(char*)__FUNCTION__,(int)__LINE__)
#define xmtp2km_kfree(ptr) __xmtp2km_kfree(ptr,(char*)__FUNCTION__,(int)__LINE__)
#else
void * __xmtp2km_kmalloc (const unsigned int bsize);
void __xmtp2km_kfree (void * p_buffer);
#define xmtp2km_kmalloc(a) __xmtp2km_kmalloc(a)
#define xmtp2km_kfree(a) __xmtp2km_kfree(a)
#endif

#endif
