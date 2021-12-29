/*****************************************************************************
 * libsangoma.c	AFT T1/E1: HDLC API Code Library
 *
 * Author(s):	Anthony Minessale II <anthmct@yahoo.com>
 *              Nenad Corbic <ncorbic@sangoma.com>
 *
 * Copyright:	(c) 2005 Anthony Minessale II
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 */

#ifndef _LIBSNAGOMA_H
#define _LIBSNAGOMA_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <linux/if_wanpipe.h>
#include <linux/wanpipe_defines.h>
#include <linux/wanpipe_cfg.h>
#include <linux/wanpipe.h>
#include <linux/sdla_aft_te1.h>

typedef api_rx_hdr_t sangoma_api_hdr_t;

int sangoma_create_socket(int span); 
int sangoma_create_socket_intr(int span, int intr);
int sangoma_write_socket(int sock, void *data, int len);
int sangoma_read_socket(int sock, void *data, int len);
int sangoma_sendmsg_socket(int sock, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag);
int sangoma_readmsg_socket(int sock, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag);
#endif
