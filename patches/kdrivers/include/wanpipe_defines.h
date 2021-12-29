
/*
 ************************************************************************
 * wanpipe_defines.h							*
 *		WANPIPE(tm) 	Global definition for Sangoma 		*
 *				Mailbox/API/UDP	structures.		*
 *									*
 * Author:	Alex Feldman <al.feldman@sangoma.com>			*
 *======================================================================*
 *	May 10 2002		Alex Feldman	Initial version		*
 *									*
 ************************************************************************
 */

#ifndef __WANPIPE_DEFINES_H
# define __WANPIPE_DEFINES_H

/************************************************
 *	SET COMMON KERNEL DEFINE		*
 ************************************************/
#if defined (__KERNEL__) || defined (KERNEL) || defined (_KERNEL)
# ifndef WAN_KERNEL
#  define WAN_KERNEL
# endif
#endif

#if defined(__LINUX__)
# include <linux/wanpipe_version.h>
# include <linux/wanpipe_kernel.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <wanpipe_version.h>
# if defined(WAN_KERNEL)
#  include <wanpipe_kernel.h>
# endif
#endif

/************************************************
 *	GLOBAL SANGOMA PLATFORM DEFINITIONS	*
 ************************************************/
#define WAN_LINUX_PLATFORM	0x01
#define WAN_WIN98_PLATFORM	0x02
#define WAN_WINNT_PLATFORM	0x03
#define WAN_WIN2K_PLATFORM	0x04
#define WAN_FREEBSD_PLATFORM	0x05
#define WAN_OPENBSD_PLATFORM	0x06
#define WAN_SOLARIS_PLATFORM	0x07
#define WAN_SCO_PLATFORM	0x08
#define WAN_NETBSD_PLATFORM	0x09

#if defined(__FreeBSD__)
# define WAN_PLATFORM_ID	WAN_FREEBSD_PLATFORM
#elif defined(__OpenBSD__)
# define WAN_PLATFORM_ID	WAN_OPENBSD_PLATFORM
#elif defined(__NetBSD__)
# define WAN_PLATFORM_ID	WAN_NETBSD_PLATFORM
#elif defined(__LINUX__)
# define WAN_PLATFORM_ID	WAN_LINUX_PLATFORM
#elif defined(__WINDOWS__)
# define WAN_PLATFORM_ID	WAN_WIN2K_PLATFORM
#endif

/************************************************
 *	GLOBAL SANGOMA COMMAND RANGES		*
 ************************************************/
#define WAN_PROTOCOL_CMD_START	0x01
#define WAN_PROTOCOL_CMD_END	0x4F

#define WAN_UDP_CMD_START	0x60
#define WAN_GET_PROTOCOL	(WAN_UDP_CMD_START+0)
#define WAN_GET_PLATFORM	(WAN_UDP_CMD_START+1)
#define WAN_GET_MEDIA_TYPE	(WAN_UDP_CMD_START+2)
#define WAN_GET_MASTER_DEV_NAME	(WAN_UDP_CMD_START+3)
#define WAN_UDP_CMD_END		0x6F	

#define WAN_FE_CMD_START	0x90
#define WAN_FE_CMD_END		0x9F

#define WAN_INTERFACE_CMD_START	0xA0
#define WAN_INTERFACE_CMD_END	0xAF

#define WAN_FE_UDP_CMD_START	0xB0
#define WAN_FE_UDP_CMD_END	0xBF	

/*
************************************************
**	GLOBAL SANGOMA DEFINITIONS			
************************************************
*/
#define WAN_FALSE	0
#define WAN_TRUE	1

#if defined(__FreeBSD__)
# undef WANPIPE_VERSION
# undef WANPIPE_VERSION_BETA
# undef WANPIPE_SUB_VERSION
# undef WANPIPE_LITE_VERSION
# define WANPIPE_VERSION	WANPIPE_VERSION_FreeBSD
# define WANPIPE_VERSION_BETA	WANPIPE_VERSION_BETA_FreeBSD
# define WANPIPE_SUB_VERSION	WANPIPE_SUB_VERSION_FreeBSD
# define WANPIPE_LITE_VERSION	WANPIPE_LITE_VERSION_FreeBSD
#elif defined(__OpenBSD__)
# undef WANPIPE_VERSION
# undef WANPIPE_VERSION_BETA
# undef WANPIPE_SUB_VERSION
# undef WANPIPE_LITE_VERSION
# define WANPIPE_VERSION	WANPIPE_VERSION_OpenBSD
# define WANPIPE_VERSION_BETA	WANPIPE_VERSION_BETA_OpenBSD
# define WANPIPE_SUB_VERSION	WANPIPE_SUB_VERSION_OpenBSD
# define WANPIPE_LITE_VERSION	WANPIPE_LITE_VERSION_OpenBSD
#elif defined(__NetBSD__)
# undef WANPIPE_VERSION
# undef WANPIPE_VERSION_BETA
# undef WANPIPE_SUB_VERSION
# undef WANPIPE_LITE_VERSION
# define WANPIPE_VERSION	WANPIPE_VERSION_NetBSD
# define WANPIPE_VERSION_BETA	WANPIPE_VERSION_BETA_NetBSD
# define WANPIPE_SUB_VERSION	WANPIPE_SUB_VERSION_NetBSD
# define WANPIPE_LITE_VERSION	WANPIPE_LITE_VERSION_NetBSD
#elif defined(__WINDOWS__)
# undef WANPIPE_VERSION
# undef WANPIPE_VERSION_BETA
# undef WANPIPE_SUB_VERSION
# define WANPIPE_VERSION	WANPIPE_VERSION_Windows
# define WANPIPE_VERSION_BETA	WANPIPE_VERSION_BETA_Windows
# define WANPIPE_SUB_VERSION	WANPIPE_SUB_VERSION_Windows
#endif

#define WANROUTER_MAJOR_VER	2
#define WANROUTER_MINOR_VER	1

#define WANPIPE_MAJOR_VER	1
#define WANPIPE_MINOR_VER	1
/*
*************************************************
**	GLOBAL SANGOMA TYPEDEF
*************************************************
*/
#if defined(__LINUX__)
typedef struct ethhdr		ethhdr_t;
typedef	struct iphdr		iphdr_t;
typedef	struct udphdr		udphdr_t;
typedef	struct tcphdr		tcphdr_t;
# define w_eth_dest	h_dest
# define w_eth_src	h_source
# define w_eth_proto	h_proto
# define w_ip_v		version
# define w_ip_hl	ihl
# define w_ip_tos	tos
# define w_ip_len	tot_len
# define w_ip_id	id
# define w_ip_off	frag_off
# define w_ip_ttl	ttl
# define w_ip_p		protocol
# define w_ip_sum	check
# define w_ip_src	saddr
# define w_ip_dst	daddr
# define w_udp_sport	source
# define w_udp_dport	dest
# define w_udp_len	len
# define w_udp_sum	check
# define w_tcp_sport	source
# define w_tcp_dport	dest
# define w_tcp_seq	seq
# define w_tcp_ack_seq	ack_seq
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
typedef	struct ip		iphdr_t;
typedef	struct udphdr		udphdr_t;
typedef	struct tcphdr		tcphdr_t;
# define w_ip_v		ip_v
# define w_ip_hl	ip_hl
# define w_ip_tos	ip_tos
# define w_ip_len	ip_len
# define w_ip_id	ip_id
# define w_ip_off	ip_off
# define w_ip_ttl	ip_ttl
# define w_ip_p		ip_p
# define w_ip_sum	ip_sum
# define w_ip_src	ip_src.s_addr
# define w_ip_dst	ip_dst.s_addr
# define w_udp_sport	uh_sport
# define w_udp_dport	uh_dport
# define w_udp_len	uh_ulen
# define w_udp_sum	uh_sum
# define w_tcp_sport	th_sport
# define w_tcp_dport	th_dport
# define w_tcp_seq	th_seq
# define w_tcp_ack_seq	th_ack
#elif defined(__WINDOWS__)
typedef	void*		iphdr_t;
typedef	void*		udphdr_t;
#else
# error "Unknown OS system!"
#endif

#if defined(__FreeBSD__)
typedef u_int8_t		u8;
typedef u_int16_t		u16;
typedef u_int32_t		u32;
#elif defined(__OpenBSD__)
typedef u_int8_t		u8;
typedef u_int16_t		u16;
typedef u_int32_t		u32;
typedef u_int64_t		u64;
#elif defined(__NetBSD__)
typedef u_int8_t		u8;
typedef u_int16_t		u16;
typedef u_int32_t		u32;
typedef u_int64_t		u64;
#endif

/************************************************
**	GLOBAL SANGOMA MACROS
************************************************/
#if defined(__LINUX__)
# define strlcpy(d,s,l)	strcpy(d,s)
#elif defined(__FreeBSD__)
# define strlcpy(d,s,l)	strcpy(d,s)
#endif

/************************************************
 *	GLOBAL DEFINITION FOR SANGOMA MAILBOX	*
 ************************************************/
#define WAN_MAILBOX_SIZE	16
#define WAN_MAX_DATA_SIZE	2032
#define WAN_MAX_POS_DATA_SIZE	1030

#pragma pack(1)
typedef struct {
	union {
		struct {
			unsigned char  opp_flag;
			unsigned char  command;
			unsigned short data_len;	
			unsigned char  return_code;
			union {
				struct {
					unsigned char	PF_bit;		/* the HDLC P/F bit */
				} hdlc;
				struct {
					unsigned short	dlci;		/* DLCI number */
					unsigned char	attr;		/* FECN, BECN, DE and C/R bits */
					unsigned short	rxlost1;	/* frames discarded at int. level */
					u_int32_t	rxlost2;	/* frames discarded at app. level */
				} fr;
				struct {
					unsigned char	pf;			/* P/F bit */
					unsigned short	lcn;		/* logical channel */
					unsigned char	qdm;		/* Q/D/M bits */
					unsigned char	cause;		/* cause field */
					unsigned char	diagn;		/* diagnostics */
					unsigned char	pktType;	/* packet type */
				} x25;
				struct {
					unsigned char	misc_Tx_Rx_bits; /* miscellaneous transmit and receive bits */
					unsigned char	Rx_error_bits; /* an indication of a block received with an error */
					unsigned short	Rx_time_stamp; /* a millisecond receive time stamp */
					unsigned char	port;		/* comm port */
				} bscstrm;
				struct {
					unsigned char 	misc_tx_rx_bits;
					unsigned short 	heading_length;
					unsigned short 	notify;
					unsigned char 	station;
					unsigned char 	poll_address;
					unsigned char 	select_address;
					unsigned char 	device_address;
					unsigned char 	notify_extended;
				} bsc;
				struct {
					unsigned char	sdlc_address;
					unsigned char	PF_bit;
					unsigned short	poll_interval;
					unsigned char	general_mailbox_byte;
				} sdlc;
				struct {
					unsigned char	force;
				} fe;
			} wan_protocol;
		} wan_p_cmd;
		struct {
			unsigned char opp_flag;
			unsigned char pos_state;
			unsigned char async_state;
		} wan_pos_cmd;
		unsigned char mbox[WAN_MAILBOX_SIZE];
	} wan_cmd_u;
#define wan_cmd_opp_flag		wan_cmd_u.wan_p_cmd.opp_flag
#define wan_cmd_command			wan_cmd_u.wan_p_cmd.command
#define wan_cmd_data_len		wan_cmd_u.wan_p_cmd.data_len
#define wan_cmd_return_code		wan_cmd_u.wan_p_cmd.return_code
#define wan_cmd_hdlc_PF_bit		wan_cmd_u.wan_p_cmd.wan_protocol.hdlc.PF_bit
#define wan_cmd_fe_force		wan_cmd_u.wan_p_cmd.wan_protocol.fe.force
#define wan_cmd_fr_dlci			wan_cmd_u.wan_p_cmd.wan_protocol.fr.dlci
#define wan_cmd_fr_attr			wan_cmd_u.wan_p_cmd.wan_protocol.fr.attr
#define wan_cmd_fr_rxlost1		wan_cmd_u.wan_p_cmd.wan_protocol.fr.rxlost1
#define wan_cmd_fr_rxlost2		wan_cmd_u.wan_p_cmd.wan_protocol.fr.rxlost2
#define wan_cmd_x25_pf			wan_cmd_u.wan_p_cmd.wan_protocol.x25.pf
#define wan_cmd_x25_lcn			wan_cmd_u.wan_p_cmd.wan_protocol.x25.lcn
#define wan_cmd_x25_qdm			wan_cmd_u.wan_p_cmd.wan_protocol.x25.qdm
#define wan_cmd_x25_cause		wan_cmd_u.wan_p_cmd.wan_protocol.x25.cause
#define wan_cmd_x25_diagn		wan_cmd_u.wan_p_cmd.wan_protocol.x25.diagn
#define wan_cmd_x25_pktType		wan_cmd_u.wan_p_cmd.wan_protocol.x25.pktType
#define wan_cmd_bscstrm_misc_bits	wan_cmd_u.wan_p_cmd.wan_protocol.bscstrm.misc_Tx_Rx_bits
#define wan_cmd_bscstrm_Rx_err_bits	wan_cmd_u.wan_p_cmd.wan_protocol.bscstrm.Rx_error_bits
#define wan_cmd_bscstrm_Rx_time_stamp	wan_cmd_u.wan_p_cmd.wan_protocol.bscstrm.Rx_time_stamp
#define wan_cmd_bscstrm_port		wan_cmd_u.wan_p_cmd.wan_protocol.bscstrm.port
#define wan_cmd_bsc_misc_bits		wan_cmd_u.wan_p_cmd.wan_protocol.bsc.misc_tx_rx_bits
#define wan_cmd_bsc_heading_len		wan_cmd_u.wan_p_cmd.wan_protocol.bsc.heading_length
#define wan_cmd_bsc_notify		wan_cmd_u.wan_p_cmd.wan_protocol.bsc.notify
#define wan_cmd_bsc_station		wan_cmd_u.wan_p_cmd.wan_protocol.bsc.station
#define wan_cmd_bsc_poll_addr		wan_cmd_u.wan_p_cmd.wan_protocol.bsc.poll_address
#define wan_cmd_bsc_select_addr		wan_cmd_u.wan_p_cmd.wan_protocol.bsc.select_address
#define wan_cmd_bsc_device_addr		wan_cmd_u.wan_p_cmd.wan_protocol.bsc.device_address
#define wan_cmd_bsc_notify_ext		wan_cmd_u.wan_p_cmd.wan_protocol.bsc.notify_extended
#define wan_cmd_sdlc_address		wan_cmd_u.wan_p_cmd.wan_protocol.sdlc.sdlc_address
#define wan_cmd_sdlc_pf			wan_cmd_u.wan_p_cmd.wan_protocol.sdlc.PF_bit
#define wan_cmd_sdlc_poll_interval	wan_cmd_u.wan_p_cmd.wan_protocol.sdlc.poll_interval
#define wan_cmd_sdlc_general_mb_byte	wan_cmd_u.wan_p_cmd.wan_protocol.sdlc.general_mailbox_byte

#define wan_cmd_pos_opp_flag		wan_cmd_u.wan_pos_cmd.opp_flag
#define wan_cmd_pos_pos_state		wan_cmd_u.wan_pos_cmd.pos_state
#define wan_cmd_pos_async_state		wan_cmd_u.wan_pos_cmd.async_state
} wan_cmd_t;


typedef struct {
	wan_cmd_t	wan_cmd;
	union {
		struct {
			unsigned char  command;
			unsigned short data_len;	
			unsigned char  return_code;
			unsigned char  port_num;
			unsigned char  attr;
			unsigned char  reserved[10];
			unsigned char  data[WAN_MAX_POS_DATA_SIZE];
		} pos_data;
		unsigned char data[WAN_MAX_DATA_SIZE];
	} wan_u_data;
#define wan_opp_flag			wan_cmd.wan_cmd_opp_flag
#define wan_command			wan_cmd.wan_cmd_command
#define wan_data_len			wan_cmd.wan_cmd_data_len
#define wan_return_code			wan_cmd.wan_cmd_return_code
#define wan_hdlc_PF_bit			wan_cmd.wan_cmd_hdlc_PF_bit
#define wan_fr_dlci			wan_cmd.wan_cmd_fr_dlci
#define wan_fr_attr			wan_cmd.wan_cmd_fr_attr
#define wan_fr_rxlost1			wan_cmd.wan_cmd_fr_rxlost1
#define wan_fr_rxlost2			wan_cmd.wan_cmd_fr_rxlost2
#define wan_x25_pf			wan_cmd.wan_cmd_x25_pf
#define wan_x25_lcn			wan_cmd.wan_cmd_x25_lcn
#define wan_x25_qdm			wan_cmd.wan_cmd_x25_qdm
#define wan_x25_cause			wan_cmd.wan_cmd_x25_cause
#define wan_x25_diagn			wan_cmd.wan_cmd_x25_diagn
#define wan_x25_pktType			wan_cmd.wan_cmd_x25_pktType
#define wan_bscstrm_misc_bits		wan_cmd.wan_cmd_bscstrm_misc_bits
#define wan_bscstrm_Rx_err_bits		wan_cmd.wan_cmd_bscstrm_Rx_error_bits
#define wan_bscstrm_Rx_time_stamp	wan_cmd.wan_cmd_bscstrm_Rx_time_stamp
#define wan_bscstrm_port		wan_cmd.wan_cmd_bscstrm_port
#define wan_bsc_misc_bits		wan_cmd.wan_cmd_bsc_misc_bits
#define wan_bsc_heading_len		wan_cmd.wan_cmd_bsc_heading_length
#define wan_bsc_notify			wan_cmd.wan_cmd_bsc_notify
#define wan_bsc_station			wan_cmd.wan_cmd_bsc_station
#define wan_bsc_poll_addr		wan_cmd.wan_cmd_bsc_poll_address
#define wan_bsc_select_addr		wan_cmd.wan_cmd_bsc_select_address
#define wan_bsc_device_addr		wan_cmd.wan_cmd_bsc_device_address
#define wan_bsc_notify_ext		wan_cmd.wan_cmd_bsc_notify_extended
#define wan_sdlc_address		wan_cmd.wan_cmd_sdlc_address		
#define wan_sdlc_pf			wan_cmd.wan_cmd_sdlc_pf			
#define wan_sdlc_poll_interval		wan_cmd.wan_cmd_sdlc_poll_interval	
#define wan_sdlc_general_mb_byte	wan_cmd.wan_cmd_sdlc_general_mb_byte	
#define wan_data			wan_u_data.data	

#define wan_pos_opp_flag		wan_cmd.wan_cmd_pos_opp_flag
#define wan_pos_pos_state		wan_cmd.wan_cmd_pos_pos_state
#define wan_pos_async_state		wan_cmd.wan_cmd_pos_async_state
#define wan_pos_command			wan_u_data.pos_data.command
#define wan_pos_data_len		wan_u_data.pos_data.data_len
#define wan_pos_return_code		wan_u_data.pos_data.return_code
#define wan_pos_port_num		wan_u_data.pos_data.port_num
#define wan_pos_attr			wan_u_data.pos_data.attr
#define wan_pos_data			wan_u_data.pos_data.data
} wan_mbox_t;

#define WAN_MBOX_INIT(mbox)	memset(mbox, 0, sizeof(wan_cmd_t));

/********************************************************
 *	GLOBAL DEFINITION FOR SANGOMA API STRUCTURE	*
 *******************************************************/
#define WAN_API_MAX_DATA	2048
typedef struct{
	unsigned char	pktType;
	unsigned short	length;
	unsigned char	result;
	union {
		struct {
			unsigned char	arg1;
			unsigned short	time_stamp;
		} chdlc;
		struct {
	        	unsigned char   attr;
	        	unsigned short  time_stamp;
		} fr;
		struct {
			unsigned char	qdm;
			unsigned char	cause;
			unsigned char	diagn;
			unsigned short	lcn;
		} x25;
		struct {
			unsigned char  station;
			unsigned char  PF_bit;
			unsigned short poll_interval;
			unsigned char  general_mailbox_byte;
		}sdlc;
		struct {
			unsigned char  exception;
		}xdlc;
	} wan_protocol;
#define wan_apihdr_chdlc_error_flag	wan_protocol.chdlc.arg1
#define wan_apihdr_chdlc_attr		wan_protocol.chdlc.arg1
#define wan_apihdr_chdlc_time_stamp	wan_protocol.chdlc.time_stamp
#define wan_apihdr_fr_attr		wan_protocol.fr.attr
#define wan_apihdr_fr_time_stamp	wan_protocol.fr.time_stamp
#define wan_apihdr_x25_qdm		wan_protocol.x25.qdm
#define wan_apihdr_x25_cause		wan_protocol.x25.cause
#define wan_apihdr_x25_diagn		wan_protocol.x25.diagn
#define wan_apihdr_x25_lcn		wan_protocol.x25.lcn

#define wan_apihdr_sdlc_station		wan_protocol.sdlc.station
#define wan_apihdr_sdlc_pf		wan_protocol.sdlc.PF_bit
#define wan_apihdr_sdlc_poll_interval	wan_protocol.sdlc.poll_interval
#define wan_apihdr_sdlc_general_mb_byte	wan_protocol.sdlc.general_mailbox_byte

#define wan_apihdr_xdlc_exception	wan_protocol.xdlc.exception
} wan_api_hdr_t;


typedef struct{
	wan_api_hdr_t	api_hdr;
	unsigned char	data[WAN_API_MAX_DATA];
#define wan_api_pktType			api_hdr.pktType
#define wan_api_length			api_hdr.length
#define wan_api_result			api_hdr.result
#define wan_api_chdlc_error_flag	api_hdr.wan_apihdr_chdlc_error_flag
#define wan_api_chdlc_time_stamp	api_hdr.wan_apihdr_chdlc_time_stamp
#define wan_api_chdlc_attr		api_hdr.wan_apihdr_chdlc_attr
#define wan_api_chdlc_misc_Tx_bits	api_hdr.wan_apihdr_chdlc_misc_Tx_bits
#define wan_api_fr_attr			api_hdr.wan_apihdr_fr_attr
#define wan_api_fr_time_stamp		api_hdr.wan_apihdr_fr_time_stamp
#define wan_api_x25_qdm			api_hdr.wan_apihdr_x25_qdm
#define wan_api_x25_cause		api_hdr.wan_apihdr_x25_cause
#define wan_api_x25_diagn		api_hdr.wan_apihdr_x25_diagn
#define wan_api_x25_lcn			api_hdr.wan_apihdr_x25_lcn
#define	wan_api_sdlc_station		api_hdr.wan_apihdr_sdlc_station
#define wan_api_sdlc_pf			api_hdr.wan_apihdr_sdlc_pf
#define wan_api_sdlc_poll_interval	api_hdr.wan_apihdr_sdlc_poll_interval
#define wan_api_sdlc_general_mb_byte	api_hdr.wan_apihdr_sdlc_general_mb_byte
#define wan_api_xdlc_exception		api_hdr.wan_apihdr_xdlc_exception
} wan_api_t;


typedef struct {
	union {
		struct {
			unsigned char	error_flag;
			unsigned short	time_stamp;
		}chdlc,hdlc;
		struct {
			unsigned char   exception;
			unsigned char 	pf;
		}lapb;
		struct {
			unsigned char	state;
			unsigned char	address;
			unsigned short  exception;
		}xdlc;
		unsigned char	reserved[16];
	}u;

#define wan_hdr_xdlc_state	u.xdlc.state
#define wan_hdr_xdlc_address	u.xdlc.address
#define wan_hdr_xdlc_exception	u.xdlc.exception
} wan_api_rx_hdr_t;

typedef struct {
        wan_api_rx_hdr_t	api_rx_hdr;
        unsigned char  		data[0];
#define wan_rxapi_xdlc_state		api_rx_hdr.wan_hdr_xdlc_state
#define wan_rxapi_xdlc_address		api_rx_hdr.wan_hdr_xdlc_address
#define wan_rxapi_xdlc_exception	api_rx_hdr.wan_hdr_xdlc_exception
} wan_api_rx_element_t;

typedef struct {
	union{
		struct {
			unsigned char 	attr;
			unsigned char   misc_Tx_bits;
		}chdlc,hdlc;
		struct {
			unsigned char 	pf;
		}lapb;
		struct {
			unsigned char  pf;
		}xdlc;
		unsigned char  	reserved[16];
	}u;
} wan_api_tx_hdr_t;

typedef struct {
	wan_api_tx_hdr_t 	api_tx_hdr;
	unsigned char		data[0];
}wan_api_tx_element_t;

#pragma pack()

enum {
	SIOC_WAN_READ_REG = 0x01,
	SIOC_WAN_WRITE_REG,
	SIOC_WAN_HWPROBE,
	SIOC_WAN_ALL_HWPROBE,
	SIOC_WAN_ALL_READ_REG,
	SIOC_WAN_ALL_WRITE_REG,
	SIOC_WAN_ALL_SET_PCI_BIOS,
	SIOC_WAN_SET_PCI_BIOS,
	SIOC_WAN_COREREV,
	SIOC_WAN_GET_CFG,
	SIOC_WAN_FE_READ_REG,
	SIOC_WAN_FE_WRITE_REG,
	SIOC_WAN_EC_REG,
	SIOC_WAN_READ_PCIBRIDGE_REG,
	SIOC_WAN_ALL_READ_PCIBRIDGE_REG,
	SIOC_WAN_WRITE_PCIBRIDGE_REG,
	SIOC_WAN_ALL_WRITE_PCIBRIDGE_REG
};

typedef struct wan_cmd_api_
{
	unsigned int	cmd;
	unsigned short	len;
	unsigned char	bar;
	u_int32_t	offset;
	unsigned char	data[WAN_MAX_DATA_SIZE];
} wan_cmd_api_t;


/********************************************************
 *	GLOBAL DEFINITION FOR SANGOMA UDP STRUCTURE	*
 *******************************************************/
#define GLOBAL_UDP_SIGNATURE		"WANPIPE"
#define GLOBAL_UDP_SIGNATURE_LEN	7
#define UDPMGMT_UDP_PROTOCOL 		0x11

typedef struct {
	unsigned char	signature[8];
	unsigned char	request_reply;
	unsigned char	id;
	unsigned char	reserved[6];
} wan_mgmt_t;


/****** DEFINITION OF UDP HEADER AND STRUCTURE PER PROTOCOL ******/
typedef struct {
	unsigned char	num_frames;
	unsigned char	ismoredata;
} wan_trace_info_t;

typedef struct wan_udp_hdr{
	wan_mgmt_t	wan_mgmt;
	wan_cmd_t	wan_cmd;
	union {
		struct {
			wan_trace_info_t	trace_info;
			unsigned char		data[WAN_MAX_DATA_SIZE];
		} chdlc, adsl, atm, ss7,bitstrm,aft;
#define xilinx aft
		unsigned char data[WAN_MAX_DATA_SIZE];
	} wan_udphdr_u;
#define wan_udphdr_signature			wan_mgmt.signature
#define wan_udphdr_request_reply		wan_mgmt.request_reply
#define wan_udphdr_id				wan_mgmt.id
#define wan_udphdr_opp_flag			wan_cmd.wan_cmd_opp_flag
#define wan_udphdr_command			wan_cmd.wan_cmd_command
#define wan_udphdr_data_len			wan_cmd.wan_cmd_data_len
#define wan_udphdr_return_code			wan_cmd.wan_cmd_return_code
#define wan_udphdr_fe_force			wan_cmd.wan_cmd_fe_force
#define wan_udphdr_hdlc_PF_bit			wan_cmd.wan_cmd_hdlc_PF_bit
#define wan_udphdr_fr_dlci			wan_cmd.wan_cmd_fr_dlci
#define wan_udphdr_fr_attr			wan_cmd.wan_cmd_fr_attr	
#define wan_udphdr_fr_rxlost1			wan_cmd.wan_cmd_fr_rxlost1
#define wan_udphdr_fr_rxlost2			wan_cmd.wan_cmd_fr_rxlost2
#define wan_udphdr_x25_pf			wan_cmd.wan_cmd_x25_pf
#define wan_udphdr_x25_lcn			wan_cmd.wan_cmd_x25_lcn			
#define wan_udphdr_x25_qdm			wan_cmd.wan_cmd_x25_qdm	
#define wan_udphdr_x25_cause			wan_cmd.wan_cmd_x25_cause
#define wan_udphdr_x25_diagn			wan_cmd.wan_cmd_x25_diagn
#define wan_udphdr_x25_pktType			wan_cmd.wan_cmd_x25_pktType
#define wan_udphdr_bscstrm_misc_bits		wan_cmd.wan_cmd_bscstrm_misc_bits
#define wan_udphdr_bscstrm_Rx_err_bits		wan_cmd.wan_cmd_bscstrm_Rx_err_bits
#define wan_udphdr_bscstrm_Rx_time_stamp	wan_cmd.wan_cmd_bscstrm_Rx_time_stamp
#define wan_udphdr_bscstrm_port			wan_cmd.wan_cmd_bscstrm_port
#define wan_udphdr_bsc_misc_bits		wan_cmd.wan_cmd_bsc_misc_bit
#define wan_udphdr_bsc_misc_heading_len		wan_cmd.wan_cmd_bsc_misc_heading_len
#define wan_udphdr_bsc_misc_notify		wan_cmd.wan_cmd_bsc_misc_notify
#define wan_udphdr_bsc_misc_station		wan_cmd.wan_cmd_bsc_misc_station
#define wan_udphdr_bsc_misc_poll_add		wan_cmd.wan_cmd_bsc_misc_poll_addr
#define wan_udphdr_bsc_misc_select_addr		wan_cmd.wan_cmd_bsc_misc_select_addr
#define wan_udphdr_bsc_misc_device_addr		wan_cmd.wan_cmd_bsc_misc_device_addr
#define wan_udphdr_chdlc_num_frames		wan_udphdr_u.chdlc.trace_info.num_frames
#define wan_udphdr_chdlc_ismoredata		wan_udphdr_u.chdlc.trace_info.ismoredata
#define wan_udphdr_chdlc_data			wan_udphdr_u.chdlc.data

#define wan_udphdr_bitstrm_num_frames		wan_udphdr_u.bitstrm.trace_info.num_frames
#define wan_udphdr_bitstrm_ismoredata		wan_udphdr_u.bitstrm.trace_info.ismoredata
#define wan_udphdr_bitstrm_data			wan_udphdr_u.bitstrm.data

#define wan_udphdr_adsl_num_frames		wan_udphdr_u.adsl.trace_info.num_frames
#define wan_udphdr_adsl_ismoredata		wan_udphdr_u.adsl.trace_info.ismoredata
#define wan_udphdr_adsl_data			wan_udphdr_u.adsl.data
#define wan_udphdr_atm_num_frames		wan_udphdr_u.atm.trace_info.num_frames
#define wan_udphdr_atm_ismoredata		wan_udphdr_u.atm.trace_info.ismoredata
#define wan_udphdr_atm_data			wan_udphdr_u.atm.data
#define wan_udphdr_ss7_num_frames		wan_udphdr_u.ss7.trace_info.num_frames
#define wan_udphdr_ss7_ismoredata		wan_udphdr_u.ss7.trace_info.ismoredata
#define wan_udphdr_ss7_data			wan_udphdr_u.ss7.data
#define wan_udphdr_aft_num_frames		wan_udphdr_u.aft.trace_info.num_frames
#define wan_udphdr_aft_ismoredata		wan_udphdr_u.aft.trace_info.ismoredata
#define wan_udphdr_aft_data			wan_udphdr_u.aft.data	
#define wan_udphdr_data				wan_udphdr_u.data
} wan_udp_hdr_t;


#ifdef WAN_KERNEL

#if 0
#if defined(__LINUX__)
# include <linux/wanpipe_events.h>
#elif defined(__FreeBSD__) || defned(__OpenBSD__)
# include <wanpipe_events.h>
#endif
#endif

/*
******************************************************************
**	D E F I N E S
******************************************************************
*/
#define MAX_PACKET_SIZE		5000
#if defined(__FreeBSD__)
/******************* F R E E B S D ******************************/
# define WAN_MOD_LOAD		MOD_LOAD
# define WAN_MOD_UNLOAD		MOD_UNLOAD
# if (__FreeBSD_version > 503000)
#  define WAN_MOD_SHUTDOWN	MOD_SHUTDOWN
#  define WAN_MOD_QUIESCE	MOD_QUIESCE
# else 
#  define WAN_MOD_SHUTDOWN	WAN_MOD_UNLOAD+1
#  define WAN_MOD_QUIESCE	WAN_MOD_UNLOAD+2
# endif
# define WP_DELAY		DELAY
# define WP_SCHEDULE(arg,name)	tsleep(&(arg),PPAUSE,(name),(arg))
# define SYSTEM_TICKS		ticks
typedef int			wan_ticks_t;
# define HZ			hz
# define RW_LOCK_UNLOCKED	0
# define ETH_P_IP		AF_INET
# define ETH_P_IPV6		AF_INET6
# define ETH_P_IPX		AF_IPX
# define WAN_IFT_OTHER		IFT_OTHER
# define WAN_IFT_ETHER		IFT_ETHER
# define WAN_IFT_PPP		IFT_PPP
# define WAN_MFLAG_PRV		M_PROTO1
#elif defined(__OpenBSD__)
/******************* O P E N B S D ******************************/
# define WAN_MOD_LOAD		LKM_E_LOAD
# define WAN_MOD_UNLOAD		LKM_E_UNLOAD
# define WP_DELAY		DELAY
# define WP_SCHEDULE(arg,name)	tsleep(&(arg),PPAUSE,(name),(arg))
# define SYSTEM_TICKS		ticks
typedef int			wan_ticks_t;
# define HZ			hz
# define RW_LOCK_UNLOCKED	0
# define ETH_P_IP		AF_INET
# define ETH_P_IPV6		AF_INET6
# define ETH_P_IPX		AF_IPX
# define WAN_IFT_OTHER		IFT_OTHER
# define WAN_IFT_ETHER		IFT_ETHER
# define WAN_IFT_PPP		IFT_PPP
# define WAN_MFLAG_PRV		M_PROTO1
#elif defined(__NetBSD__)
/******************* N E T B S D ******************************/
# define WAN_MOD_LOAD		LKM_E_LOAD
# define WAN_MOD_UNLOAD		LKM_E_UNLOAD
# define WP_DELAY		DELAY
# define SYSTEM_TICKS		tick
typedef int			wan_ticks_t;
# define HZ			hz
# define RW_LOCK_UNLOCKED	0
# define WAN_IFT_OTHER		IFT_OTHER
# define WAN_IFT_ETHER		IFT_ETHER
# define WAN_IFT_PPP		IFT_PPP
#elif defined(__LINUX__)
/*********************** L I N U X ******************************/
# define ETHER_ADDR_LEN		ETH_ALEN
# define WP_DELAY(usecs)	udelay(usecs)
# define atomic_set_int(name, val)	atomic_set(name, val)
# define SYSTEM_TICKS		jiffies
typedef unsigned long		wan_ticks_t;
# define WP_SCHEDULE(arg,name)	schedule()
# define wan_atomic_read	atomic_read
# define wan_atomic_set		atomic_set
# define wan_atomic_inc		atomic_inc
# define wan_atomic_dec		atomic_dec
# define WAN_IFT_OTHER		0x00
# define WAN_IFT_ETHER		0x00
# define WAN_IFT_PPP		0x00
#elif defined(__WINDOWS__)
/******************* W I N D O W S ******************************/
# define EINVAL		22
# define IFNAMESIZ	16
#endif

#if defined(__FreeBSD__)
# define WAN_MODULE_VERSION(module, version)			\
	MODULE_VERSION(module, version)
# define WAN_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)	\
	MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)		 
# define WAN_MODULE_DEFINE(name,name_str,author,descr,lic,mod_init,mod_exit,devsw)\
	int load_##name (module_t mod, int cmd, void *arg);	\
	int load_##name (module_t mod, int cmd, void *arg){	\
		switch(cmd){					\
		case WAN_MOD_LOAD: return mod_init((devsw));	\
		case WAN_MOD_UNLOAD: 				\
		case WAN_MOD_SHUTDOWN: return mod_exit((devsw));\
		case WAN_MOD_QUIESCE: return 0;			\
		}						\
		return -EINVAL;					\
	}							\
	DEV_MODULE(name, load_##name, NULL);
#elif defined(__OpenBSD__)
# define WAN_MODULE_VERSION(module, version)
# define WAN_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)
# define WAN_MODULE_DEFINE(name,name_str,author,descr,lic,mod_init,mod_exit,devsw)\
	int (name)(struct lkm_table* lkmtp, int cmd, int ver);\
	MOD_DEV(name_str, LM_DT_CHAR, -1, (devsw));	\
	int load_##name(struct lkm_table* lkm_tp, int cmd){	\
		switch(cmd){					\
		case WAN_MOD_LOAD: return mod_init(NULL);	\
		case WAN_MOD_UNLOAD: return mod_exit(NULL);	\
		}						\
		return -EINVAL;					\
	}							\
	int (name)(struct lkm_table* lkmtp, int cmd, int ver){\
		DISPATCH(lkmtp,cmd,ver,load_##name,load_##name,lkm_nofunc);\
	}
#elif defined(__NetBSD__)
# define WAN_MODULE_VERSION(module, version)
# define WAN_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)
# if (__NetBSD_Version__ < 200000000)
#  define WAN_MOD_DEV(name,devsw) MOD_DEV(name,LM_DT_CHAR,-1,(devsw));
# else
#  define WAN_MOD_DEV(name,devsw) MOD_DEV(name,name,NULL,-1,(devsw),-1);
# endif
# define WAN_MODULE_DEFINE(name,name_str,author,descr,lic,mod_init,mod_exit,devsw)\
	int (##name_lkmentry)(struct lkm_table* lkmtp, int cmd, int ver);\
	WAN_MOD_DEV(name_str, (devsw));				\
	int load_##name(struct lkm_table* lkm_tp, int cmd){	\
		switch(cmd){					\
		case WAN_MOD_LOAD: return mod_init(NULL);	\
		case WAN_MOD_UNLOAD: return mod_exit(NULL);	\
		}						\
		return -EINVAL;					\
	}							\
	int (##name_lkmentry)(struct lkm_table* lkmtp, int cmd, int ver){\
		DISPATCH(lkmtp,cmd,ver,load_##name,load_##name,lkm_nofunc);\
	}
#elif defined(__LINUX__)
# define WAN_MODULE_VERSION(module, version)
# define WAN_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)
# define WAN_MODULE_DEFINE(name,name_str,author,descr,lic,mod_init,mod_exit,devsw)\
	MODULE_AUTHOR (author);					\
	MODULE_DESCRIPTION (descr);				\
	MODULE_LICENSE(lic);					\
	int __init load_##name(void){return mod_init(NULL);}	\
	void __exit unload_##name(void){mod_exit(NULL);}	\
	module_init(load_##name);				\
	module_exit(unload_##name);				

#elif defined(__SOLARIS__)
# define WAN_MODULE_VERSION(module, version)
# define WAN_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)
# define WAN_MODULE_DEFINE(name,name_str,author,descr,lic,mod_init,mod_exit,devsw)\
	int _init(void){\
		int err=mod_init(NULL);				\
		if (err) return err; 				\
		err=mod_install(&modlinkage)			\
		if (err) cmn_err(CE_CONT, "mod_install: failed\n"); \
		return err;					\
	}\
	void _fini(void){					\
		int status					\
		mod_exit(NULL); 				\
		if ((status = mod_remove(&modlinkage)) != 0)	\
        		cmn_err(CE_CONT, "mod_remove: failed\n"); \
		return status;					\
	}\
	int _info(struct modinfo* modinfop)			\
	{							\
    		dcmn_err((CE_CONT, "Get module info!\n"));	\
    		return (mod_info(&modlinkage, modinfop));	\
	}
#endif

/*
******************************************************************
**	T Y P E D E F
******************************************************************
*/
#if !defined(offsetof)
# define offsetof(type, member)	((size_t)(&((type*)0)->member))
#endif

#if defined(__LINUX__)
typedef struct sk_buff		netskb_t;
typedef struct sk_buff_head	wan_skb_queue_t;
typedef struct timer_list	wan_timer_info_t;
typedef void 			(*wan_timer_func_t)(unsigned long);
typedef unsigned long		wan_timer_arg_t;
typedef void 			wan_tasklet_func_t(unsigned long);
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
typedef void 			(*wan_taskq_func_t)(void *);
# else
typedef void 			(*wan_taskq_func_t)(struct work_struct *);
#endif

/* Due to 2.6.20 kernel, the wan_taskq_t must be declared
 * here as a workqueue structre.  The tq_struct is declared
 * as work queue in wanpipe_kernel.h */
typedef struct tq_struct 	wan_taskq_t;

typedef void*			virt_addr_t;
typedef unsigned long		phys_addr_t;
typedef spinlock_t		wan_spinlock_t;
typedef rwlock_t		wan_rwlock_t;
typedef unsigned long		wan_smp_flag_t;
typedef unsigned long 		wan_rwlock_flag_t;

typedef void			(*TASKQ_FUNC)(void *);
typedef struct tty_driver	ttydriver_t;
typedef struct tty_struct	ttystruct_t;
typedef struct termios		termios_t;
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,10)
#  define vsnprintf(a,b,c,d)	vsprintf(a,c,d)
# endif
typedef void*			wan_dma_tag_t;
typedef wait_queue_head_t	wan_waitq_head_t;
typedef void			(wan_pci_ifunc_t)(void*);
#elif defined(__FreeBSD__)
typedef struct ifnet		netdevice_t;
typedef struct mbuf		netskb_t;
# ifdef ALTQ
typedef struct ifaltq		wan_skb_queue_t;
# else
typedef	struct ifqueue		wan_skb_queue_t;
# endif
typedef struct ether_header	ethhdr_t;
typedef struct callout_handle	wan_timer_info_t;
typedef timeout_t*		wan_timer_func_t;
typedef void*			wan_timer_arg_t;
typedef task_fn_t		wan_tasklet_func_t;
typedef task_fn_t*		wan_taskq_func_t;
typedef caddr_t			virt_addr_t;
typedef u_int32_t		phys_addr_t;
typedef int			atomic_t;
typedef dev_t			ttydriver_t;
typedef struct tty		ttystruct_t;
typedef struct termios		termios_t;
typedef int 			(get_info_t)(char *, char **, off_t, int, int);
typedef int			wan_spinlock_t;
typedef int 			wan_rwlock_t;
typedef int			wan_smp_flag_t;
typedef int			wan_rwlock_flag_t;
# if (__FreeBSD_version < 450000)
typedef struct proc		wan_dev_thread_t;
# else
typedef d_thread_t		wan_dev_thread_t;
# endif
typedef bus_dma_tag_t		wan_dma_tag_t;
typedef int			wan_waitq_head_t;
typedef void			(wan_pci_ifunc_t)(void*);
#elif defined(__OpenBSD__)
typedef struct ifnet		netdevice_t;
typedef struct mbuf		netskb_t;
# ifdef ALTQ
typedef struct ifaltq		wan_skb_queue_t;
# else
typedef	struct ifqueue		wan_skb_queue_t;
# endif
typedef struct ether_header	ethhdr_t;
typedef struct timeout		wan_timer_info_t;
typedef void 			(*wan_timer_func_t)(void*);
typedef void*			wan_timer_arg_t;
typedef void 			wan_tasklet_func_t(void*, int);
typedef void 			(*wan_taskq_func_t)(void*, int);
typedef caddr_t			virt_addr_t;
typedef u_int32_t		phys_addr_t;
typedef int			atomic_t;
typedef dev_t			ttydriver_t;
typedef struct tty		ttystruct_t;
typedef struct termios		termios_t;
typedef int 			(get_info_t)(char *, char **, off_t, int, int);
typedef bus_dma_tag_t		wan_dma_tag_t;
typedef int			wan_spinlock_t;
typedef int			wan_smp_flag_t;
typedef int 			wan_rwlock_t;
typedef int			wan_rwlock_flag_t;
typedef int			(wan_pci_ifunc_t)(void*);
#elif defined(__NetBSD__)
typedef struct ifnet		netdevice_t;
typedef struct mbuf		netskb_t;
# ifdef ALTQ
typedef struct ifaltq		wan_skb_queue_t;
# else
typedef	struct ifqueue		wan_skb_queue_t;
# endif
typedef struct ether_header	ethhdr_t;
typedef struct callout		wan_timer_info_t;
typedef void 			(*wan_timer_func_t)(void*);
typedef void*			wan_timer_arg_t;
typedef void 			wan_tasklet_func_t(void*, int);
typedef void 			(*wan_taskq_func_t)(void*, int);
typedef caddr_t			virt_addr_t;
typedef u_int32_t		phys_addr_t;
typedef int			atomic_t;
typedef dev_t			ttydriver_t;
typedef struct tty		ttystruct_t;
typedef struct termios		termios_t;
typedef int 			(get_info_t)(char *, char **, off_t, int, int);
typedef bus_dma_tag_t		wan_dma_tag_t;
typedef int			wan_spinlock_t;
typedef int			wan_smp_flag_t;
typedef int 			wan_rwlock_t;
typedef int			wan_rwlock_flag_t;
typedef void			(wan_pci_ifunc_t)(void*);
#elif defined(__SOLARIS__)
typedef mblk_t			netskb_t;


#elif defined(__WINDOWS__)
typedef UCHAR	u8;
typedef UINT	u16;
typedef USHORT	u32;
typedef ULONG	u64;
typedef char*	caddr_t;
# if defined(NDIS_MINIPORT_DRIVER)
typedef NDIS_MINIPORT_TIMER	wan_timer_info_t;
# else
typedef KTIMER			wan_timer_info_t;
# endif

typedef void	netdevice_t;
typedef void*	wan_skb_queue_t;
typedef void (*wan_timer_func_t)(void*);
#endif

/*
 * Spin Locks
 */
#if 0
typedef struct _wan_spinlock
{
#if defined(__LINUX__)
   	spinlock_t      	slock;
    	unsigned long   	flags;
#elif defined(MAC_OS)
    	ULONG           	slock;
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    	int			slock;
#endif /* OS */
} wan_spinlock_t;
#endif

/*
 * Read Write Locks
 */
#if 0
typedef struct _wan_rwlock
{
#if defined(__LINUX__)
	rwlock_t      	rwlock;
#elif defined(MAC_OS)
 	#error "wan_rwlock_t not defined"
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	volatile unsigned int lock;
#else
	#error "wan_rwlock_t not defined"    
#endif /* OS */
} wan_rwlock_t;
#endif

/*
** FIXME: Redefined from sdla_adsl.c
** DMA structure
*/
typedef struct _wan_dma_descr
{
	unsigned long*		vAddr;
        unsigned long		pAddr;
        unsigned long		length;
        unsigned long		max_length;
#if defined(__FreeBSD__)
	bus_dma_tag_t		dmat;
	bus_dmamap_t		dmamap;
#elif defined(__OpenBSD__)
	bus_dma_tag_t		dmat;
	bus_dma_segment_t	dmaseg;
	int			rsegs;
#elif defined(__NetBSD__)
	bus_dma_tag_t		dmat;
	bus_dma_segment_t	dmaseg;
	int			rsegs;
#else	/* other OS */
#endif
} wan_dma_descr_t;/*, *PDMA_DESCRIPTION;*/

/*
** TASK structure
*/
typedef struct _wan_tasklet
{
	unsigned long		running;
#if defined(__FreeBSD__) && (__FreeBSD_version >= 410000)
	struct task		task_id;
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	wan_tasklet_func_t*	task_func;
	void*			data;	
#elif defined(__LINUX__)
	struct tasklet_struct 	task_id;
#elif  defined(__SOLARIS__)
#error "wan_tasklet: not defined in solaris"
#endif
} wan_tasklet_t;

#ifndef __LINUX__
typedef struct _wan_taskq
{
	unsigned char		running;
#if defined(__FreeBSD__) && (__FreeBSD_version >= 410000)
	struct task		tqueue;
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	wan_taskq_func_t	tfunc;
	void*			data;	
#elif defined(__LINUX__)
/* Due to 2.6.20 kernel, we cannot abstract the
 * wan_taskq_t here, we must declare it as work queue */
# error "Linux doesnt support wan_taskq_t here!"
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)) 
    	struct tq_struct 	tqueue;
# else
        struct work_struct 	tqueue;
# endif
#elif  defined(__SOLARIS__)
#error "_wan_taskq: not defined in solaris"
#endif
} wan_taskq_t;
#endif


typedef struct wan_trace
{
	u_int32_t  		tracing_enabled;
	wan_skb_queue_t		trace_queue;
	unsigned long  		trace_timeout;/* WARNING: has to be 'unsigned long' !!!*/
	unsigned int   		max_trace_queue;
	unsigned char		last_trace_direction;
	u_int32_t		missed_idle_rx_counter;
}wan_trace_t;



/*
** TIMER structure
*/
typedef struct _wan_timer
{
#define NDIS_TIMER_TAG 0xBEEF0005
	unsigned long           Tag;
	wan_timer_func_t        MiniportTimerFunction;
	void *                  MiniportTimerContext;
	void *                  MiniportAdapterHandle;
	wan_timer_info_t	timer_info;
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	wan_timer_func_t	timer_func;
#elif defined(__WINDOWS__)
	KDPC			TimerDpcObject;
#endif
	void*			timer_arg;
} wan_timer_t;

#if !defined(LINUX_2_6)
/* Define this structure for BSDs and not Linux-2.6 */
struct seq_file {
	char*		buf;	/* pointer to buffer	(buf)*/
	size_t		size;	/* total buffer len	(len)*/
	size_t		from;	/* total buffer len	(offs)*/
	size_t		count;	/* offset into buffer	(cnt)*/
# if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	unsigned long	index;	/* iteration index*/
# else
	loff_t		index;	/* iteration index*/
# endif
	int		stop_cnt;/* last stop offset*/
};
#endif



/****** DEFINITION OF UDP HEADER AND STRUCTURE PER PROTOCOL *******/
typedef struct wan_udp_pkt {
	iphdr_t		ip_hdr;
	udphdr_t	udp_hdr;
	wan_udp_hdr_t	wan_udp_hdr;


#define wan_ip				ip_hdr
#define wan_ip_v			ip_hdr.w_ip_v
#define wan_ip_hl			ip_hdr.w_ip_hl
#define wan_ip_tos			ip_hdr.w_ip_tos
#define wan_ip_len			ip_hdr.w_ip_len
#define wan_ip_id			ip_hdr.w_ip_id
#define wan_ip_off			ip_hdr.w_ip_off
#define wan_ip_ttl			ip_hdr.w_ip_ttl
#define wan_ip_p			ip_hdr.w_ip_p
#define wan_ip_sum			ip_hdr.w_ip_sum
#define wan_ip_src			ip_hdr.w_ip_src
#define wan_ip_dst			ip_hdr.w_ip_dst
#define wan_udp_sport			udp_hdr.w_udp_sport
#define wan_udp_dport			udp_hdr.w_udp_dport
#define wan_udp_len			udp_hdr.w_udp_len
#define wan_udp_sum			udp_hdr.w_udp_sum
#define wan_udp_cmd			wan_udp_hdr.wan_cmd
#define wan_udp_signature		wan_udp_hdr.wan_udphdr_signature
#define wan_udp_request_reply		wan_udp_hdr.wan_udphdr_request_reply
#define wan_udp_id			wan_udp_hdr.wan_udphdr_id
#define wan_udp_opp_flag		wan_udp_hdr.wan_udphdr_opp_flag
#define wan_udp_command			wan_udp_hdr.wan_udphdr_command
#define wan_udp_data_len		wan_udp_hdr.wan_udphdr_data_len
#define wan_udp_return_code		wan_udp_hdr.wan_udphdr_return_code
#define wan_udp_hdlc_PF_bit 		wan_udp_hdr.wan_udphdr_hdlc_PF_bit
#define wan_udp_fr_dlci 		wan_udp_hdr.wan_udphdr_fr_dlci
#define wan_udp_fr_attr 		wan_udp_hdr.wan_udphdr_fr_attr
#define wan_udp_fr_rxlost1 		wan_udp_hdr.wan_udphdr_fr_rxlost1
#define wan_udp_fr_rxlost2 		wan_udp_hdr.wan_udphdr_fr_rxlost2
#define wan_udp_x25_pf 			wan_udp_hdr.wan_udphdr_x25_pf	
#define wan_udp_x25_lcn 		wan_udp_hdr.wan_udphdr_x25_lcn	
#define wan_udp_x25_qdm 		wan_udp_hdr.wan_udphdr_x25_qdm
#define wan_udp_x25_cause 		wan_udp_hdr.wan_udphdr_x25_cause
#define wan_udp_x25_diagn 		wan_udp_hdr.wan_udphdr_x25_diagn
#define wan_udp_x25_pktType 		wan_udp_hdr.wan_udphdr_x25_pktType
#define wan_udp_bscstrm_misc_bits 	wan_udp_hdr.wan_udphdr_bscstrm_misc_bits
#define wan_udp_bscstrm_Rx_err_bits 	wan_udp_hdr.wan_udphdr_bscstrm_Rx_err_bits
#define wan_udp_bscstrm_Rx_time_stam 	wan_udp_hdr.wan_udphdr_bscstrm_Rx_time_stamp
#define wan_udp_bscstrm_port 		wan_udp_hdr.wan_udphdr_bscstrm_port
#define wan_udp_bsc_misc_bits 		wan_udp_hdr.wan_udphdr_bsc_misc_bits
#define wan_udp_bsc_misc_heading_len  	wan_udp_hdr.wan_udphdr_bsc_misc_heading_len
#define wan_udp_bsc_misc_notify 	wan_udp_hdr.wan_udphdr_bsc_misc_notify
#define wan_udp_bsc_misc_station 	wan_udp_hdr.wan_udphdr_bsc_misc_station
#define wan_udp_bsc_misc_poll_add 	wan_udp_hdr.wan_udphdr_bsc_misc_poll_add
#define wan_udp_bsc_misc_select_addr 	wan_udp_hdr.wan_udphdr_bsc_misc_select_addr
#define wan_udp_bsc_misc_device_addr 	wan_udp_hdr.wan_udphdr_bsc_misc_device_addr
#define wan_udp_bsc_misc_notify_ext 	wan_udp_hdr.wan_udphdr_bsc_misc_notify_ext
#define wan_udp_chdlc_num_frames	wan_udp_hdr.wan_udphdr_chdlc_num_frames
#define wan_udp_chdlc_ismoredata	wan_udp_hdr.wan_udphdr_chdlc_ismoredata
#define wan_udp_chdlc_data		wan_udp_hdr.wan_udphdr_chdlc_data

#define wan_udp_bitstrm_num_frames	wan_udp_hdr.wan_udphdr_bitstrm_num_frames
#define wan_udp_bitstrm_ismoredata	wan_udp_hdr.wan_udphdr_bitstrm_ismoredata
#define wan_udp_bitstrm_data		wan_udp_hdr.wan_udphdr_bitstrm_data

#define wan_udp_adsl_num_frames		wan_udp_hdr.wan_udphdr_adsl_num_frames
#define wan_udp_adsl_ismoredata		wan_udp_hdr.wan_udphdr_adsl_ismoredata
#define wan_udp_adsl_data		wan_udp_hdr.wan_udphdr_adsl_data
#define wan_udp_atm_num_frames		wan_udp_hdr.wan_udphdr_atm_num_frames
#define wan_udp_atm_ismoredata		wan_udp_hdr.wan_udphdr_atm_ismoredata	
#define wan_udp_atm_data		wan_udp_hdr.wan_udphdr_atm_data
#define wan_udp_ss7_num_frames		wan_udp_hdr.wan_udphdr_ss7_num_frames
#define wan_udp_ss7_ismoredata		wan_udp_hdr.wan_udphdr_ss7_ismoredata	
#define wan_udp_ss7_data		wan_udp_hdr.wan_udphdr_ss7_data
#define wan_udp_aft_num_frames		wan_udp_hdr.wan_udphdr_aft_num_frames
#define wan_udp_aft_ismoredata		wan_udp_hdr.wan_udphdr_aft_ismoredata	
#define wan_udp_data			wan_udp_hdr.wan_udphdr_data
} wan_udp_pkt_t;


#pragma pack(1)
#if defined(WAN_BIG_ENDIAN)

typedef struct {
  uint8_t cc:4;	/* CSRC count             */
  uint8_t x:1;		/* header extension flag  */
  uint8_t p:1;		/* padding flag           */
  uint8_t version:2;	/* protocol version       */
  uint8_t pt:7;	/* payload type           */
  uint8_t m:1;		/* marker bit             */
  uint16_t seq;		/* sequence number        */
  uint32_t ts;		/* timestamp              */
  uint32_t ssrc;	/* synchronization source */
} wan_rtp_hdr_t;

#else /*  BIG_ENDIAN */

typedef struct {
  unsigned version:2;	/* protocol version       */
  unsigned p:1;		/* padding flag           */
  unsigned x:1;		/* header extension flag  */
  unsigned cc:4;	/* CSRC count             */
  unsigned m:1;		/* marker bit             */
  unsigned pt:7;	/* payload type           */
  uint16_t seq;		/* sequence number        */
  uint32_t ts;		/* timestamp              */
  uint32_t ssrc;	/* synchronization source */
} wan_rtp_hdr_t;

#endif

typedef struct wan_rtp_pkt {
	ethhdr_t	eth_hdr;
	iphdr_t		ip_hdr;
	udphdr_t	udp_hdr;       
	wan_rtp_hdr_t	rtp_hdr;
#define wan_eth_dest			eth_hdr.w_eth_dest	
#define wan_eth_src 			eth_hdr.w_eth_src
#define wan_eth_proto  			eth_hdr.w_eth_proto         
} wan_rtp_pkt_t;

#pragma pack()


#endif /* KERNEL */ 

#endif /* __WANPIPE_DEFINES_H */
