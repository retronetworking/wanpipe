/*************************************************************************
 ss7_linux.h	Sangoma SS7 firmware API definitions

 Author:      	Gideon Hack
		Nenad Corbic <ncorbic@sangoma.com>	

 Copyright:	(c) 1995-2000 Sangoma Technologies Inc.

		This program is free software; you can redistribute it and/or
		modify it under the term of the GNU General Public License
		as published by the Free Software Foundation; either version
		2 of the License, or (at your option) any later version.

===========================================================================
 Dec 22, 2000	Gideon Hack 	Initial Verison
 Jan 16, 2001   Gideon Hack     Header updates
===========================================================================

Descripiton:
	The 'C' header file for the Sangoma S508/S514 SS7 code API. 

****************************************************************************/

#ifndef _SS7_LINUX_H
#define _SS7_LINUX_H

#ifndef	PACKED
#define	PACKED __attribute__((packed))
#endif

#include <linux/sdla_ss7.h>
#include <linux/if_wanpipe.h>

enum {
	SIOCC_PC_RESERVED = (SIOC_WANPIPE_DEVPRIVATE),
	SIOCS_GENERAL_CMD,
	SIOCS_CHECK_FRONT_STATE,
	SIOC_RECEIVE,
	SIOC_SEND,
	SIOC_RECEIVE_WAIT,
	SIOC_RECEIVE_AVAILABLE,
	SIOC_RETRIEVE_MSU_BUFS
};

#define SS7_CMD_BLOCK_SZ (sizeof(wan_mbox_t)-1-SIZEOF_MB_DATA_BFR)


typedef struct {
	unsigned char	status		PACKED;
	unsigned char	data_avail	PACKED;
	unsigned short	real_length	PACKED;
	unsigned short	time_stamp	PACKED;
	unsigned char	data[1]		PACKED;
} trace_pkt_t;

typedef struct {
	unsigned char	SIO	PACKED;
	unsigned short	time_stamp	PACKED;
	unsigned char	reserved[13]	PACKED;
} api_rx_hdr_t;

typedef struct {
        api_rx_hdr_t	api_rx_hdr      PACKED;
        void *   	data    	PACKED;
} api_rx_element_t;

typedef struct {
	unsigned char 	SIO		PACKED;
	unsigned char  	reserved[15]	PACKED;
} api_tx_hdr_t;

typedef struct {
	api_tx_hdr_t 	api_tx_hdr	PACKED;
	void *		data		PACKED;
} api_tx_element_t;


/* The embedded control block for UDP mgmt 
   This is essentially a mailbox structure, without the large data field */
#if 0
typedef struct {
        unsigned char  opp_flag PACKED;                  /* the opp flag */
        unsigned char  command PACKED;                   /* the user command */
        unsigned short buffer_length PACKED;             /* the data length */
        unsigned char  return_code PACKED;               /* the return code */
	unsigned char  MB_reserved[NUMBER_MB_RESERVED_BYTES] PACKED;	/* reserved for later */
} cblock_t;


typedef struct {
	unsigned char		num_frames	PACKED;
	unsigned char		ismoredata	PACKED;
} trace_info_t;

typedef struct {
	ip_pkt_t 		ip_pkt		PACKED;
	udp_pkt_t		udp_pkt		PACKED;
	wp_mgmt_t		wp_mgmt		PACKED;
	cblock_t                cblock          PACKED;
	trace_info_t       	trace_info      PACKED;
	unsigned char           data[SIZEOF_MB_DATA_BFR]      PACKED;
} ss7_udp_pkt_t;
#endif

/* modem status changes */
#define DCD_HIGH	0x08
#define CTS_HIGH	0x20

/* Special UDP drivers management commands */
#define SPIPE_ENABLE_TRACING				0x50
#define SPIPE_DISABLE_TRACING				0x51
#define SPIPE_GET_TRACE_INFO				0x52
#define SPIPE_GET_IBA_DATA				0x53
#define SPIPE_FT1_READ_STATUS				0x54
#define SPIPE_DRIVER_STAT_IFSEND			0x55
#define SPIPE_DRIVER_STAT_INTR				0x56
#define SPIPE_DRIVER_STAT_GEN				0x57
#define SPIPE_FLUSH_DRIVER_STATS			0x58
#define SPIPE_ROUTER_UP_TIME				0x59

/* Driver specific commands for API */
#define	CHDLC_READ_TRACE_DATA		0xE4	/* read trace data */
#define TRACE_ALL                       0x00
#define TRACE_PROT			0x01
#define TRACE_DATA			0x02

#define UDPMGMT_UDP_PROTOCOL 0x11
#define UDPMGMT_SIGNATURE    "CTPIPEAB"   /* "STPIPEAB" */


#endif

