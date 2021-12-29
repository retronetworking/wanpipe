/*****************************************************************************
* sdla_ppp.h	Sangoma PPP firmware API definitions.
*
* Author:	Nenad Corbic	<ncorbic@sangoma.com>
*
* Copyright:	(c) 1995-1997 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Feb 24, 2000  Nenad Corbic    v2.1.2
* Jan 06, 1997	Gene Kozin	v2.0
* Apr 11, 1996	Gene Kozin	Initial version.
*****************************************************************************/
#ifndef	_SDLA_PPP_H
#define	_SDLA_PPP_H

/*----------------------------------------------------------------------------
 * Notes:
 * ------
 * 1. All structures defined in this file are byte-alined.  
 *
 *	Compiler	Platform
 *	--------	--------
 *	GNU C		Linux		
 */

#ifndef	PACKED
#    define	PACKED	__attribute__((packed))
#endif	/* PACKED */


#include <linux/wanpipe_sppp_iface.h>

/* Adapter memory layout and important constants */
#define	PPP508_MB_VECT	0xE000	/* mailbox window vector */
#define	PPP508_MB_OFFS	0		/* mailbox offset */
#define	PPP508_FLG_OFFS	0x1000	/* status flags offset */
#define	PPP508_BUF_OFFS	0x1100	/* buffer info block offset */
#define PPP514_MB_OFFS  0xE000  /* mailbox offset */
#define PPP514_FLG_OFFS 0xF000  /* status flags offset */
#define PPP514_BUF_OFFS 0xF100  /* buffer info block offset */

#define PPP_MAX_DATA	1008	/* command block data buffer length */

/****** Data Structures *****************************************************/
/*----------------------------------------------------------------------------
 * PPP Command Block.
 */
typedef struct ppp_cmd{
	unsigned char  command	PACKED;	/* command code */
	unsigned short length	PACKED;	/* length of data buffer */
	unsigned char  result	PACKED;	/* return code */
	unsigned char  rsrv[11]	PACKED;	/* reserved for future use */
} ppp_cmd_t;

typedef struct {
	unsigned char	status		PACKED;
	unsigned char	data_avail	PACKED;
	unsigned short	real_length	PACKED;
	unsigned short	time_stamp	PACKED;
	unsigned char	data[1]		PACKED;
} trace_pkt_t;


typedef struct {
	unsigned char 	opp_flag	PACKED;
	unsigned char	trace_type	PACKED;
	unsigned short 	trace_length	PACKED;
	unsigned short 	trace_data_ptr	PACKED;
	unsigned short  trace_time_stamp PACKED;
} trace_element_t;



/* 'result' field defines */
#define PPPRES_OK		0x00	/* command executed successfully */
#define	PPPRES_INVALID_STATE	0x09	/* invalid command in this context */

#if 0
/*----------------------------------------------------------------------------
 * PPP Mailbox.
 *	This structure is located at offset PPP???_MB_OFFS into PPP???_MB_VECT
 */
typedef struct ppp_mbox
{
	unsigned char flag	PACKED;	/* 00h: command execution flag */
	ppp_cmd_t     cmd	PACKED; /* 01h: command block */
	unsigned char data[1]	PACKED;	/* 10h: variable length data buffer */
} ppp_mbox_t;
#endif
/*----------------------------------------------------------------------------
 * PPP Status Flags.
 *	This structure is located at offset PPP???_FLG_OFFS into
 *	PPP???_MB_VECT.
 */
typedef struct	ppp_flags
{
	unsigned char iflag		PACKED;	/* 00: interrupt flag */
	unsigned char imask		PACKED;	/* 01: interrupt mask */
	unsigned char resrv		PACKED;
	unsigned char mstatus		PACKED;	/* 03: modem status */
	unsigned char lcp_state		PACKED; /* 04: LCP state */
	unsigned char ppp_phase		PACKED;	/* 05: PPP phase */
	unsigned char ip_state		PACKED; /* 06: IPCP state */
	unsigned char ipx_state		PACKED; /* 07: IPXCP state */
	unsigned char pap_state		PACKED; /* 08: PAP state */
	unsigned char chap_state	PACKED; /* 09: CHAP state */
	unsigned short disc_cause	PACKED;	/* 0A: disconnection cause */
} ppp_flags_t;

/* 'iflag' defines */
#define	PPP_INTR_RXRDY		0x01	/* Rx ready */
#define	PPP_INTR_TXRDY		0x02	/* Tx ready */
#define	PPP_INTR_MODEM		0x04	/* modem status change (DCD, CTS) */
#define	PPP_INTR_CMD		0x08	/* interface command completed */
#define	PPP_INTR_DISC		0x10	/* data link disconnected */
#define	PPP_INTR_OPEN		0x20	/* data link open */
#define	PPP_INTR_DROP_DTR	0x40	/* DTR drop timeout expired */
#define PPP_INTR_TIMER          0x80    /* timer interrupt */


/* 'mstatus' defines */
#define	PPP_MDM_DCD		0x08	/* mdm_status: DCD */
#define	PPP_MDM_CTS		0x20	/* mdm_status: CTS */

/* 'disc_cause' defines */
#define PPP_LOCAL_TERMINATION   0x0001	/* Local Request by PPP termination phase */
#define PPP_DCD_CTS_DROP        0x0002  /* DCD and/or CTS dropped. Link down */
#define PPP_REMOTE_TERMINATION	0x0800	/* Remote Request by PPP termination phase */

/* 'misc_config_bits' defines */
#define DONT_RE_TX_ABORTED_I_FRAMES 	0x01
#define TX_FRM_BYTE_COUNT_STATS         0x02
#define RX_FRM_BYTE_COUNT_STATS         0x04
#define TIME_STAMP_IN_RX_FRAMES         0x08
#define NON_STD_ADPTR_FREQ              0x10
#define INTERFACE_LEVEL_RS232           0x20
#define AUTO_LINK_RECOVERY              0x100
#define DONT_TERMINATE_LNK_MAX_CONFIG   0x200                    

/* 'authentication options' defines */
#define NO_AUTHENTICATION	0x00
#define INBOUND_AUTH		0x80
#define PAP_AUTH		0x01
#define CHAP_AUTH		0x02		

/* 'ip options' defines */
#define L_AND_R_IP_NO_ASSIG	0x00
#define L_IP_LOCAL_ASSIG    	0x01
#define L_IP_REMOTE_ASSIG   	0x02
#define R_IP_LOCAL_ASSIG        0x04
#define R_IP_REMOTE_ASSIG       0x08
#define ENABLE_IP		0x80

/* 'ipx options' defines */
#define ROUTING_PROT_DEFAULT    0x20
#define ENABLE_IPX		0x80
#define DISABLE_IPX		0x00

/*----------------------------------------------------------------------------
 * PPP Buffer Info.
 *	This structure is located at offset PPP508_BUF_OFFS into
 *	PPP508_MB_VECT.
 */
typedef struct	ppp508_buf_info
{
	unsigned short txb_num	PACKED;	/* 00: number of transmit buffers */
	unsigned int  txb_ptr	PACKED;	/* 02: pointer to the buffer ctl. */
	unsigned int  txb_nxt  PACKED;
	unsigned char  rsrv1[22] PACKED;
	unsigned short rxb_num	PACKED;	/* 20: number of receive buffers */
	unsigned int  rxb_ptr	PACKED;	/* 22: pointer to the buffer ctl. */
	unsigned int  rxb1_ptr	PACKED;	/* 26: pointer to the first buf.ctl. */
	unsigned int  rxb_base	PACKED;	/* 2A: pointer to the buffer base */
	unsigned char  rsrv2[2]	PACKED;
	unsigned int  rxb_end	PACKED;	/* 30: pointer to the buffer end */
} ppp508_buf_info_t;

/*----------------------------------------------------------------------------
 * Transmit/Receive Buffer Control Block.
 */
typedef struct	ppp_buf_ctl
{
	unsigned char  flag		PACKED;	/* 00: 'buffer ready' flag */
	unsigned short length		PACKED;	/* 01: length of data */
	unsigned char  reserved1[1]	PACKED;	/* 03: */
	unsigned char  proto		PACKED;	/* 04: protocol */
	unsigned short timestamp	PACKED;	/* 05: time stamp (Rx only) */
	unsigned char  reserved2[5]	PACKED;	/* 07: */
	union
	{
		unsigned short o_p[2];	/* 1C: buffer offset & page (S502) */
		unsigned int  ptr;	/* 1C: buffer pointer (S508) */
	} buf				PACKED;
} ppp_buf_ctl_t;

/*----------------------------------------------------------------------------
 * S508 Adapter Configuration Block (passed to the PPP_SET_CONFIG command).
 */
typedef struct	ppp508_conf
{
	unsigned int  line_speed	PACKED;	/* 00: baud rate, bps */
	unsigned short txbuf_percent	PACKED;	/* 04: % of Tx buffer */
	unsigned short conf_flags	PACKED;	/* 06: configuration bits */
	unsigned short mtu_local	PACKED;	/* 08: local MTU */
	unsigned short mtu_remote	PACKED;	/* 0A: remote MTU */
	unsigned short restart_tmr	PACKED;	/* 0C: restart timer */
	unsigned short auth_rsrt_tmr	PACKED;	/* 0E: authentication timer */
	unsigned short auth_wait_tmr	PACKED;	/* 10: authentication timer */
	unsigned short mdm_fail_tmr	PACKED;	/* 12: modem failure timer */
	unsigned short dtr_drop_tmr	PACKED;	/* 14: DTR drop timer */
	unsigned short connect_tmout	PACKED;	/* 16: connection timeout */
	unsigned short conf_retry	PACKED;	/* 18: max. retry */
	unsigned short term_retry	PACKED;	/* 1A: max. retry */
	unsigned short fail_retry	PACKED;	/* 1C: max. retry */
	unsigned short auth_retry	PACKED;	/* 1E: max. retry */
	unsigned char  auth_options	PACKED;	/* 20: authentication opt. */
	unsigned char  ip_options	PACKED;	/* 21: IP options */
	unsigned int  ip_local		PACKED;	/* 22: local IP address */
	unsigned int  ip_remote	PACKED;	/* 26: remote IP address */
	unsigned char  ipx_options	PACKED;	/* 2A: IPX options */
	unsigned char  ipx_netno[4]	PACKED;	/* 2B: IPX net number */
	unsigned char  ipx_local[6]	PACKED;	/* 2F: local IPX node number*/
	unsigned char  ipx_remote[6]	PACKED;	/* 35: remote IPX node num.*/
	unsigned char  ipx_router[48]	PACKED;	/* 3B: IPX router name*/
	unsigned int  alt_cpu_clock	PACKED;	/* 6B:  */
} ppp508_conf_t;

/*----------------------------------------------------------------------------
 * S508 Adapter Read Connection Information Block 
 *    Returned by the PPP_GET_CONNECTION_INFO command
 */
typedef struct	ppp508_connect_info
{
	unsigned short 	mru		PACKED;	/* 00-01 Remote Max Rec' Unit */
	unsigned char  	ip_options 	PACKED; /* 02: Negotiated ip options  */
	unsigned int  	ip_local	PACKED;	/* 03-06: local IP address    */
	unsigned int  	ip_remote	PACKED;	/* 07-0A: remote IP address   */
	unsigned char	ipx_options	PACKED; /* 0B: Negotiated ipx options */
	unsigned char  	ipx_netno[4]	PACKED;	/* 0C-0F: IPX net number      */
	unsigned char  	ipx_local[6]	PACKED;	/* 10-1F: local IPX node #    */
	unsigned char  	ipx_remote[6]	PACKED;	/* 16-1B: remote IPX node #   */
	unsigned char  	ipx_router[48]	PACKED;	/* 1C-4B: IPX router name     */
	unsigned char	auth_status	PACKED; /* 4C: Authentication Status  */
	unsigned char 	inbd_auth_peerID[1] PACKED; /* 4D: variable length inbound authenticated peer ID */
} ppp508_connect_info_t;

/* 'line_speed' field */
#define	PPP_BITRATE_1200	0x01
#define	PPP_BITRATE_2400	0x02
#define	PPP_BITRATE_4800	0x03
#define	PPP_BITRATE_9600	0x04
#define	PPP_BITRATE_19200	0x05
#define	PPP_BITRATE_38400	0x06
#define	PPP_BITRATE_45000	0x07
#define	PPP_BITRATE_56000	0x08
#define	PPP_BITRATE_64000	0x09
#define	PPP_BITRATE_74000	0x0A
#define	PPP_BITRATE_112000	0x0B
#define	PPP_BITRATE_128000	0x0C
#define	PPP_BITRATE_156000	0x0D

/* Defines for the 'conf_flags' field */
#define	PPP_IGNORE_TX_ABORT	0x01	/* don't re-transmit aborted frames */
#define	PPP_ENABLE_TX_STATS	0x02	/* enable Tx statistics */
#define	PPP_ENABLE_RX_STATS	0x04	/* enable Rx statistics */
#define	PPP_ENABLE_TIMESTAMP	0x08	/* enable timestamp */

/* 'ip_options' defines */
#define	PPP_LOCAL_IP_LOCAL	0x01
#define	PPP_LOCAL_IP_REMOTE	0x02
#define	PPP_REMOTE_IP_LOCAL	0x04
#define	PPP_REMOTE_IP_REMOTE	0x08

/* 'ipx_options' defines */
#define	PPP_REMOTE_IPX_NETNO	0x01
#define	PPP_REMOTE_IPX_LOCAL	0x02
#define	PPP_REMOTE_IPX_REMOTE	0x04
#define	PPP_IPX_ROUTE_RIP_SAP	0x08
#define	PPP_IPX_ROUTE_NLSP	0x10
#define	PPP_IPX_ROUTE_DEFAULT	0x20
#define	PPP_IPX_CONF_COMPLETE	0x40
#define	PPP_IPX_ENABLE		0x80

/*----------------------------------------------------------------------------
 * S508 Adapter Configuration Block (returned by the PPP_READ_CONFIG command).
 */
typedef struct	ppp508_get_conf
{
	unsigned int  bps	PACKED;	/* 00: baud rate, bps */
	ppp508_conf_t  conf	PACKED;	/* 04: requested config. */
	unsigned short txb_num	PACKED;	/* 6F: number of Tx buffers */
	unsigned short rxb_num	PACKED;	/* 71: number of Rx buffers */
} ppp508_get_conf_t;

/*----------------------------------------------------------------------------
 * S508 Operational Statistics (returned by the PPP_READ_STATISTIC command).
 */
typedef struct ppp508_stats
{
	unsigned short reserved1	PACKED;	/* 00: */
	unsigned short rx_bad_len	PACKED;	/* 02: */
	unsigned short reserved2	PACKED;	/* 04: */
	unsigned int  tx_frames	PACKED;	/* 06: */
	unsigned int  tx_bytes	PACKED;	/* 0A: */
	unsigned int  rx_frames	PACKED;	/* 0E: */
	unsigned int  rx_bytes	PACKED;	/* 12: */
} ppp508_stats_t;

/*----------------------------------------------------------------------------
 * Adapter Error Statistics (returned by the PPP_READ_ERROR_STATS command).
 */
typedef struct	ppp_err_stats
{
	unsigned char	 rx_overrun	PACKED;	/* 00: Rx overrun errors */
	unsigned char	 rx_bad_crc	PACKED;	/* 01: Rx CRC errors */
	unsigned char	 rx_abort	PACKED;	/* 02: Rx aborted frames */
	unsigned char	 rx_lost	PACKED;	/* 03: Rx frames lost */
	unsigned char	 tx_abort	PACKED;	/* 04: Tx aborted frames */
	unsigned char	 tx_underrun	PACKED;	/* 05: Tx underrun errors */
	unsigned char	 tx_missed_intr	PACKED;	/* 06: Tx underruns missed */
	unsigned char	 reserved	PACKED;	/* 07: Tx underruns missed */
	unsigned char	 dcd_trans	PACKED;	/* 08: DCD transitions */
	unsigned char	 cts_trans	PACKED;	/* 09: CTS transitions */
} ppp_err_stats_t;

/* Data structure for SET_TRIGGER_INTR command
 */

typedef struct ppp_intr_info{
	unsigned char  i_enable		PACKED; /* 0 Interrupt enable bits */
	unsigned char  irq              PACKED; /* 1 Irq number */
	unsigned short timer_len        PACKED; /* 2 Timer delay */
} ppp_intr_info_t;


#define FT1_MONITOR_STATUS_CTRL                         0x80
#define SET_FT1_MODE                                    0x81



/* Special UDP drivers management commands */
#define PPIPE_ENABLE_TRACING                            0x20
#define PPIPE_DISABLE_TRACING                           0x21
#define PPIPE_GET_TRACE_INFO                            0x22
#define PPIPE_GET_IBA_DATA                              0x23
#define PPIPE_KILL_BOARD     				0x24
#define PPIPE_FT1_READ_STATUS                           0x25
#define PPIPE_DRIVER_STAT_IFSEND                        0x26
#define PPIPE_DRIVER_STAT_INTR                          0x27
#define PPIPE_DRIVER_STAT_GEN                           0x28
#define PPIPE_FLUSH_DRIVER_STATS                        0x29
#define PPIPE_ROUTER_UP_TIME                            0x30
#define PPIPE_TE1_56K_STAT 	    			0x40	/* TE1_56K */
#define PPIPE_GET_MEDIA_TYPE	 	    		0x41	/* TE1_56K */
#define PPIPE_FLUSH_TE1_PMON 	   			0x42	/* TE1     */
#define PPIPE_READ_REGISTER 	   			0x43	/* TE1_56K */
#define PPIPE_TE1_CFG 		   			0x44	/* TE1     */

#define DISABLE_TRACING 				0x00
#define TRACE_SIGNALLING_FRAMES				0x01
#define TRACE_DATA_FRAMES				0x02

#define UDPMGMT_SIGNATURE    "PTPIPEAB"
#define UDPDRV_SIGNATURE     "DRVSTATS"
#define UDPMGMT_UDP_PROTOCOL 0x11

#ifdef		_MSC_
#  pragma	pack()
#endif
#endif	/* _SDLA_PPP_H */
