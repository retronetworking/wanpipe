/*
 ************************************************************************
 * wanpipe_defines.h							*
 *		WANPIPE(tm) 	Global definition for Sangoma 		*
 *				Mailbox/API/UDP	structures.		*
 *									*
 * Author:	Alex Feldman <al.feldman@sangoma.com>			*
 *======================================================================*
 *	May 10 2002		Alex Feldman	Initial version						*
 *																		*
 *	v 1,0,0,0	June 10 2004		David Rokhvarg						*
 *		Additions and changes to use with new Windows driver			*
 *	v 1,0,0,1	July 14 2006		David Rokhvarg						*
 *		Additions for A200 Analog card									*
 ************************************************************************
 */

#ifndef __WANPIPE_DEFINES_H
# define __WANPIPE_DEFINES_H

/************************************************
 *	SET COMMON KERNEL DEFINE					*
 ************************************************/
#if defined (__KERNEL__) || defined (KERNEL) || defined (_KERNEL)
# define WAN_KERNEL
#endif

#if defined(__WINDOWS__)

/* Miscellaneous */
#define	WAN_IFNAME_SZ	256				/* max length of the interface name */
#define	WAN_DRVNAME_SZ	WAN_IFNAME_SZ	/* max length of the link driver name */
#define	WAN_ADDRESS_SZ	31				/* max length of the WAN media address */

#define USED_BY_FIELD	128	/* max length of the used by field */

#define WAN_AUTHNAMELEN 64

//this is the maximum number of interfaces that any protocol may have.
//i.g. Number of DLCIs.
#define MAX_NUMBER_OF_PROTOCOL_INTERFACES	1023

#ifndef inline
#define inline _inline
#endif

#define strlcpy strncpy

#endif//(__WINDOWS__)

#ifdef __LINUX__
# include <linux/wanpipe_kernel.h>
#endif

/************************************************
 *	GLOBAL SANGOMA PLATFORM DEFINITIONS			*
 ************************************************/
#define WAN_LINUX_PLATFORM	0x01
#define WAN_WIN98_PLATFORM	0x02
#define WAN_WINNT_PLATFORM	0x03
#define WAN_WIN2K_PLATFORM	0x04
#define WAN_FREEBSD_PLATFORM	0x05
#define WAN_OPENBSD_PLATFORM	0x06
#define WAN_SOLARIS_PLATFORM	0x07
#define WAN_SCO_PLATFORM	0x08


/************************************************
 *	GLOBAL SANGOMA COMMAND RANGES				*
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
# define WANPIPE_VERSION	WANPIPE_VERSION_FreeBSD
# define WANPIPE_VERSION_BETA	WANPIPE_VERSION_BETA_FreeBSD
#elif defined(__OpenBSD__)
# undef WANPIPE_VERSION
# undef WANPIPE_VERSION_BETA
# define WANPIPE_VERSION	WANPIPE_VERSION_FreeBSD
# define WANPIPE_VERSION_BETA	WANPIPE_VERSION_BETA_FreeBSD
#elif defined(__WINDOWS__)
# undef WANPIPE_VERSION
# undef WANPIPE_SUB_VERSION
# undef WANPIPE_VERSION_BETA
# define WANPIPE_VERSION		WANPIPE_VERSION_Windows
# define WANPIPE_SUB_VERSION	WANPIPE_SUB_VERSION_Windows
# define WANPIPE_VERSION_BETA	WANPIPE_VERSION_BETA_Windows
#endif

/*
*************************************************
**	GLOBAL SANGOMA TYPEDEF
*************************************************
*/
#if defined(__LINUX__)
typedef	struct iphdr		iphdr_t;
typedef	struct udphdr		udphdr_t;
# define w_ip_v	version
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
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
typedef	struct ip		iphdr_t;
typedef	struct udphdr		udphdr_t;
# define w_ip_v	ip_v
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
#elif defined(__WINDOWS__)
typedef UCHAR	u8;
#define __u8	u8
typedef USHORT	u16;
#define __u16	u16
typedef unsigned int	u32;
#define __u32	u32
typedef ULONG	u64;

typedef UCHAR	u_char;
typedef char*	caddr_t;

typedef signed __int8		int8_t;
typedef unsigned __int8		u_int8_t;
typedef unsigned __int8		uint8_t;
typedef unsigned __int16	u_int16_t;
typedef unsigned __int32	u_int32_t;
typedef unsigned __int32	uint32_t;

#define	atomic_t int
typedef ULONG	pid_t;

typedef unsigned long clock_t;

typedef unsigned int	wan_smp_flag_t;

//////////////////////////////////////////////////////
enum{
	TDMAPI_BUFFER_NO_CODEC=1,
	TDMAPI_BUFFER_HDLC_DATA,
	TDMAPI_BUFFER_ERROR,
	TDMAPI_BUFFER_READY,
	TDMAPI_BUFFER_ACCEPTED
};

int lib_sang_api_decode(void *tdm_api_ptr,
						void *original_databuf,	int original_datalen,
						void *new_databuf,		int *new_datalen);

int lib_sang_api_encode(void *tdm_api_ptr,
						void *original_databuf,	int original_datalen,
						void *new_databuf,		int *new_datalen);

int lib_sang_api_rx(void *tdm_api_ptr,
					void *rx_data, int rx_data_len,
					char *destination_buf, unsigned int *destination_buf_datalen);

unsigned char wp_tdmapi_poll(void *tdm_api_ptr);

//////////////////////////////////////////////////////
//FIXME: used header file : #include <ip.h>
//instead of defining here 'iphdr':
//////////////////////////////////////////////////////
//on Intel X86
#define __LITTLE_ENDIAN_BITFIELD

struct iphdr {
	
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8    ihl:4,
			version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	__u8    version:4,
			ihl:4;
#else
# error  "unknown byteorder!"
#endif
	__u8    tos;
	__u16   tot_len;
	__u16   id;
	__u16   frag_off;
	__u8    ttl;
	__u8    protocol;
	__u16   check;
	__u32   saddr;
	__u32   daddr;
	/*The options start here. */
};
/////////////////////////////////////////////////////

struct udphdr {
	__u16   source;
	__u16   dest;
	__u16   len;
	__u16   check;
};

typedef	struct	iphdr	iphdr_t;
typedef	struct	udphdr	udphdr_t;

# define w_ip_v	version
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

#define DRV_MODE_NORMAL	0
#define DRV_MODE_AFTUP	1

#else
# error "Unknown OS system!"
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
					unsigned long	rxlost2;	/* frames discarded at app. level */
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
					unsigned char sdlc_address;
					unsigned char PF_bit;
					unsigned short poll_interval;
					unsigned char general_mailbox_byte;
				} sdlc;
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
#pragma pack()

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
			unsigned char   station;
		}sdlc;
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
        unsigned char  		data[1];//[0]
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
	unsigned char		data[1];//[0];
}wan_api_tx_element_t;

typedef struct wan_cmd_api_
{
	unsigned int	cmd;
	unsigned short	len;
	unsigned char	bar;
	unsigned short	offset;
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
		unsigned char data[WAN_MAX_DATA_SIZE];
	} wan_udphdr_u;

#define wan_udphdr_signature		wan_mgmt.signature
#define wan_udphdr_request_reply	wan_mgmt.request_reply
#define wan_udphdr_id				wan_mgmt.id
#define wan_udphdr_opp_flag			wan_cmd.wan_cmd_opp_flag
#define wan_udphdr_command			wan_cmd.wan_cmd_command
#define wan_udphdr_data_len			wan_cmd.wan_cmd_data_len
#define wan_udphdr_return_code		wan_cmd.wan_cmd_return_code
#define wan_udphdr_hdlc_PF_bit		wan_cmd.wan_cmd_hdlc_PF_bit
#define wan_udphdr_fr_dlci			wan_cmd.wan_cmd_fr_dlci
#define wan_udphdr_fr_attr			wan_cmd.wan_cmd_fr_attr	
#define wan_udphdr_fr_rxlost1		wan_cmd.wan_cmd_fr_rxlost1
#define wan_udphdr_fr_rxlost2		wan_cmd.wan_cmd_fr_rxlost2
#define wan_udphdr_x25_pf			wan_cmd.wan_cmd_x25_pf
#define wan_udphdr_x25_lcn			wan_cmd.wan_cmd_x25_lcn			
#define wan_udphdr_x25_qdm				wan_cmd.wan_cmd_x25_qdm	
#define wan_udphdr_x25_cause			wan_cmd.wan_cmd_x25_cause
#define wan_udphdr_x25_diagn			wan_cmd.wan_cmd_x25_diagn
#define wan_udphdr_x25_pktType			wan_cmd.wan_cmd_x25_pktType
#define wan_udphdr_bscstrm_misc_bits		wan_cmd.wan_cmd_bscstrm_misc_bits
#define wan_udphdr_bscstrm_Rx_err_bits		wan_cmd.wan_cmd_bscstrm_Rx_err_bits
#define wan_udphdr_bscstrm_Rx_time_stamp	wan_cmd.wan_cmd_bscstrm_Rx_time_stamp
#define wan_udphdr_bsc_misc_bits			wan_cmd.wan_cmd_bsc_misc_bit
#define wan_udphdr_bsc_misc_heading_len		wan_cmd.wan_cmd_bsc_misc_heading_len
#define wan_udphdr_bsc_misc_notify			wan_cmd.wan_cmd_bsc_misc_notify
#define wan_udphdr_bsc_misc_station			wan_cmd.wan_cmd_bsc_misc_station
#define wan_udphdr_bsc_misc_poll_add		wan_cmd.wan_cmd_bsc_misc_poll_addr
#define wan_udphdr_bsc_misc_select_addr		wan_cmd.wan_cmd_bsc_misc_select_addr
#define wan_udphdr_bsc_misc_device_addr		wan_cmd.wan_cmd_bsc_misc_device_addr

#define wan_udphdr_chdlc_num_frames		wan_udphdr_u.chdlc.trace_info.num_frames
#define wan_udphdr_chdlc_ismoredata		wan_udphdr_u.chdlc.trace_info.ismoredata
#define wan_udphdr_chdlc_data			wan_udphdr_u.chdlc.data

#define wan_udphdr_bitstrm_num_frames	wan_udphdr_u.bitstrm.trace_info.num_frames
#define wan_udphdr_bitstrm_ismoredata	wan_udphdr_u.bitstrm.trace_info.ismoredata
#define wan_udphdr_bitstrm_data			wan_udphdr_u.bitstrm.data

#define wan_udphdr_adsl_num_frames		wan_udphdr_u.adsl.trace_info.num_frames
#define wan_udphdr_adsl_ismoredata		wan_udphdr_u.adsl.trace_info.ismoredata
#define wan_udphdr_adsl_data			wan_udphdr_u.adsl.data

#define wan_udphdr_atm_num_frames		wan_udphdr_u.atm.trace_info.num_frames
#define wan_udphdr_atm_ismoredata		wan_udphdr_u.atm.trace_info.ismoredata
#define wan_udphdr_atm_data				wan_udphdr_u.atm.data

#define wan_udphdr_ss7_num_frames		wan_udphdr_u.ss7.trace_info.num_frames
#define wan_udphdr_ss7_ismoredata		wan_udphdr_u.ss7.trace_info.ismoredata
#define wan_udphdr_ss7_data				wan_udphdr_u.ss7.data

#define wan_udphdr_aft_num_frames		wan_udphdr_u.aft.trace_info.num_frames
#define wan_udphdr_aft_ismoredata		wan_udphdr_u.aft.trace_info.ismoredata
#define wan_udphdr_aft_data				wan_udphdr_u.aft.data

///////////////////////////////////////////////////////////
//old version. not used.
//#define wan_udphdr_data				wan_udphdr_u.data
///////////////////////////////////////////////////////////

#define wan_udphdr_data					wan_udphdr_aft_data

} wan_udp_hdr_t;

# define PATH_MAX	1024	//for octasic code
# define MAXPATHLEN	PATH_MAX

#ifdef EFAULT
#undef EFAULT
#endif
#ifdef EBUSY
#undef EBUSY
#endif
#ifdef ENODEV
#undef ENODEV
#endif
#ifdef EINVAL
#undef EINVAL
#endif
#ifdef EIO
#undef EIO
#endif
#ifdef ENOMEM
#undef ENOMEM
#endif
#ifdef EEXIST
#undef EEXIST
#endif
#ifdef ENXIO
#undef ENXIO
#endif
# define EFAULT				SANG_STATUS_GENERAL_ERROR
# define EBUSY				SANG_STATUS_DEVICE_BUSY
# define ENODEV				SANG_STATUS_INVALID_DEVICE
# define EINVAL				SANG_STATUS_INVALID_PARAMETER
# define EIO				SANG_STATUS_IO_ERROR
# define EPFNOSUPPORT		SANG_STATUS_UNSUPPORTED_FUNCTION
# define EPROTONOSUPPORT	SANG_STATUS_UNSUPPORTED_PROTOCOL
# define ENOMEM				SANG_STATUS_FAILED_ALLOCATE_MEMORY
# define EEXIST				SANG_STATUS_DEVICE_ALREADY_EXIST
# define ENOBUFS			ENOMEM
# define EOPNOTSUPP			SANG_STATUS_OPTION_NOT_SUPPORTED
# define ENXIO				EFAULT

#ifdef WAN_KERNEL

/*
******************************************************************
**	D E F I N E S
******************************************************************
*/
#if defined(__FreeBSD__)
# define WAN_MOD_LOAD			MOD_LOAD
# define WAN_MOD_UNLOAD			MOD_UNLOAD
# define atomic_set(name, val)		*(int*)name = val
# define atomic_read(name)		*(int*)name
# define atomic_inc(name)		atomic_add_int(name, 1)
# define atomic_dec(name)		atomic_substract_int(name, 1)
# define SYSTEM_TICKS	ticks
# define HZ		hz
#elif defined(__OpenBSD__)
# define WAN_MOD_LOAD			LKM_E_LOAD
# define WAN_MOD_UNLOAD			LKM_E_UNLOAD
# define clear_bit(bit, name)		bit_clear((unsigned char*)name, bit)
# define set_bit(bit, name)		bit_set((unsigned char*)name, bit)
# define test_bit(bit, name)		bit_test((unsigned char*)name, bit)
# define atomic_set(name, val)		*(int*)name = val
# define atomic_set_int(name, val)	*(unsigned int*)name = val
# define atomic_read(name)		*(int*)name
# define SYSTEM_TICKS	ticks
# define HZ		hz
/* Unsafe sprintf and vsprintf function removed from the kernel */
# define sprintf(a,b...)		snprintf(a, sizeof(a), ##b)
# define vsprintf(a,b...)		vsnprintf(a, sizeof(a), ##b)
#elif defined(__LINUX__)
# define ETHER_ADDR_LEN			ETH_ALEN
# define DELAY(usecs)			udelay(usecs)
# define atomic_set_int(name, val)	atomic_set(name, val)
# define SYSTEM_TICKS	jiffies

#elif defined(__WINDOWS__)

# define RW_LOCK_UNLOCKED	0
# define ETHER_ADDR_LEN		6//??? ETH_ALEN
# define DELAY(usecs)		KeStallExecutionProcessor(usecs);//usecs in MicroSeconds
# define WP_DELAY(usecs)	DELAY(usecs)
# define udelay(usecs)		DELAY(usecs)

//
//all of the bit macros defined in <wanpipe\bit_win.h>
//
//# define clear_bit(bit, name)		bit_clear((unsigned char*)name, bit)
//# define set_bit(bit, name)		bit_set((unsigned char*)name, bit)
//# define test_bit(bit, name)		bit_test((unsigned char*)name, bit)

//Note: these macros are not atomic really. make sure to use "priority" protection.
# define atomic_set(name, val)		*(int*)name = val
# define atomic_set_int(name, val)	*(unsigned int*)name = val
# define atomic_read(name)			*(int*)name

# define atomic_inc(name)			(*(int*)name)++
# define atomic_dec(name)			(*(int*)name)--

////////////////////////////////////////////////
# define wan_atomic_set		atomic_set
# define wan_atomic_set_int	atomic_set_int
# define wan_atomic_read	atomic_read

# define wan_atomic_inc		atomic_inc
# define wan_atomic_dec		atomic_dec
////////////////////////////////////////////////

//The __FUNCTION__ macro is not defined in Windows 2000 DDK compiler,
//but defined in Windows XP DDK compiler.
#if !defined(__FUNCTION__)
static char* function_macro(char* function_name, int line_number)
{
#define MAX_PATH_LEN	512
	static char path_buff[MAX_PATH_LEN];
	_snprintf(path_buff, MAX_PATH_LEN, "{File: %s, Line: %d}", 
		function_name, line_number);
	return path_buff;
}

#define __FUNCTION__ function_macro(__FILE__, __LINE__)
#endif//#if !defined(__FUNCTION__)

#define HW_ERROR	0xFE
#define NUM_OF_CRC_AND_FLAG_BYTES	3

#endif//#elif defined(__WINDOWS__)


/*
******************************************************************
**	T Y P E D E F
******************************************************************
*/

#if defined(__LINUX__)
typedef struct sk_buff		netskb_t;
typedef struct sk_buff_head	wan_skb_queue_t;
typedef struct ethhdr		ethhdr_t;
typedef struct timer_list	wan_timer_info_t;
typedef void 			(*wan_timer_func_t)(unsigned long);
typedef void 			(*wan_tasklet_func_t)(unsigned long);
typedef void 			(*wan_taskq_func_t)(void *);
typedef void*			virt_addr_t;
typedef unsigned long		phys_addr_t;

typedef void			(*TASKQ_FUNC)(void *);
typedef struct tty_driver	ttydriver_t;
typedef struct tty_struct	ttystruct_t;
typedef struct termios		termios_t;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,10)
# define vsnprintf(a,b,c,d)	vsprintf(a,c,d)
#endif
typedef void*			wan_dma_tag_t;
#elif defined(__FreeBSD__)
typedef u_int8_t		u8;
typedef u_int16_t		u16;
typedef u_int32_t		u32;
typedef u_int64_t		u64;
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
typedef task_fn_t*		wan_tasklet_func_t;
typedef task_fn_t*		wan_taskq_func_t;
typedef caddr_t			virt_addr_t;
typedef u_int32_t		phys_addr_t;
typedef int			atomic_t;
typedef dev_t			ttydriver_t;
typedef struct tty		ttystruct_t;
typedef struct termios		termios_t;
typedef int 			(get_info_t)(char *, char **, off_t, int, int);
# if (__FreeBSD_version < 450000)
typedef struct proc		wan_dev_thread_t;
# else
typedef d_thread_t		wan_dev_thread_t;
# endif
typedef bus_dma_tag_t		wan_dma_tag_t;
#elif defined(__OpenBSD__)
typedef u_int8_t		u8;
typedef u_int16_t		u16;
typedef u_int32_t		u32;
typedef u_int64_t		u64;
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
typedef void 			(*wan_tasklet_func_t)(void*, int);
typedef void 			(*wan_taskq_func_t)(void*, int);
typedef caddr_t			virt_addr_t;
typedef u_int32_t		phys_addr_t;
typedef int			atomic_t;
typedef dev_t			ttydriver_t;
typedef struct tty		ttystruct_t;
typedef struct termios		termios_t;
typedef int 			(get_info_t)(char *, char **, off_t, int, int);
typedef bus_dma_tag_t		wan_dma_tag_t;

#elif defined(__WINDOWS__)

typedef void*			wan_timer_arg_t;

typedef struct _wan_timer_info_t{

	KTIMER				Timer;
	KDPC				TimerDpcObject;
	LARGE_INTEGER		TimerDueTime;
	PKDEFERRED_ROUTINE  function;
	wan_timer_arg_t		context;
}wan_timer_info_t;

struct sk_buff {

	unsigned char   *head;	/* Originally allocated buffer pointer. Do NOT change it!! */
    unsigned char   *end;   /* End of originally allocated buffer. Do NOT change it!!*/

	unsigned char   *data;  /* current 1-st byte of user data pointer       */
    unsigned char   *tail;  /* current last byte of user data pointer		*/
	unsigned int	len;	/* current length of user data at 'data' */

	unsigned char	protocol;
	unsigned int	data_len;
	unsigned char	cloned;

	void* list;
	int csum;
	char* cb;
};

///////////////////////////////////////////////////////////////////////////

#define DBG_NET_DEV 1

#define MAX_SKB_QUEUE_SIZE	100

typedef struct skb_queue_element
{
	void * previous_element;
	struct sk_buff* data;

}skb_queue_element_t;

typedef struct wan_skb_queue
{
	//maximum allowed number of nodes in the list
	//unsigned int maximum_size;
	//current number of nodes in the list
	unsigned int size;
	//insert at tail
	skb_queue_element_t * tail;
	//remove from head
	skb_queue_element_t * head;

	//The "skb queue full" message flag - should not be repeated on each discarded
	//buffer, or log file may get very big. Write it once and set this flag.
	//When queue gets at least one empty space again, reset this flag.
	unsigned char skb_queue_full_message_printed;

}wan_skb_queue_t;


//////////////////////////////////////////////////////

typedef void (*wan_timer_func_t)(IN PKDPC Dpc, void* context, void * arg2, void * arg3);

typedef	KSPIN_LOCK spinlock_t;

typedef struct sk_buff		netskb_t;

#define sk_buff_head wan_skb_queue

#endif//__WINDOWS__

/*
** TIMER structure
*/
typedef struct _wan_timer
{
	wan_timer_info_t	timer_info;

#if defined(__FreeBSD__) || defined(__OpenBSD__)
	wan_timer_func_t	timer_func;
#elif defined(__WINDOWS__)
	KDPC				TimerDpcObject;
#endif

	void*				timer_arg;
} wan_timer_t;

/*
 * Spin Locks
 */
typedef struct _wan_spinlock
{
#if defined(__LINUX__)
   	spinlock_t      	slock;
   	unsigned long   	flags;
#elif defined(MAC_OS)
   	ULONG           	slock;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
   	int			slock;
#elif defined(__WINDOWS__)
	spinlock_t		slock;
	KIRQL			flags;
#endif /* OS */
} wan_spinlock_t;


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
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	volatile unsigned int lock;
#elif defined(__WINDOWS__)
	//???
	int      	rwlock;
#else
	#error "wan_rwlock_t not defined"    
#endif /* OS */
} wan_rwlock_t;
#endif

typedef int		wan_rwlock_t;
typedef int		wan_rwlock_flag_t;
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
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	wan_tasklet_func_t	task_func;
	void*			data;	
#elif defined(__LINUX__)
	struct tasklet_struct 	task_id;
#elif defined(__WINDOWS__)
	KDPC 	tqueue;
#endif
} wan_tasklet_t;


typedef struct _wan_taskq
{
	unsigned char		running;
#if defined(__FreeBSD__) && (__FreeBSD_version >= 410000)
	struct task		tqueue;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	wan_taskq_func_t	tfunc;
	void*			data;	
#elif defined(__LINUX__)
    struct tq_struct 	tqueue;
#elif defined(__WINDOWS__)
	KDPC 	tqueue;
#endif
} wan_taskq_t;

typedef struct wan_trace
{
	unsigned char  		tracing_enabled;
	wan_skb_queue_t		trace_queue;
	unsigned int  		trace_timeout;
	unsigned int   		max_trace_queue;
}wan_trace_t;



/****** DEFINITION OF UDP HEADER AND STRUCTURE PER PROTOCOL *******/
typedef struct wan_udp_pkt {
	iphdr_t		ip_hdr;
	udphdr_t	udp_hdr;
	wan_udp_hdr_t	wan_udp_hdr;
#define wan_ip				ip_hdr
#define wan_ip_v			ip_hdr.w_ip_v
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
#define wan_udp_chdlc_data			wan_udp_hdr.wan_udphdr_chdlc_data

#define wan_udp_bitstrm_num_frames	wan_udp_hdr.wan_udphdr_bitstrm_num_frames
#define wan_udp_bitstrm_ismoredata	wan_udp_hdr.wan_udphdr_bitstrm_ismoredata
#define wan_udp_bitstrm_data		wan_udp_hdr.wan_udphdr_bitstrm_data

#define wan_udp_adsl_num_frames		wan_udp_hdr.wan_udphdr_adsl_num_frames
#define wan_udp_adsl_ismoredata		wan_udp_hdr.wan_udphdr_adsl_ismoredata
#define wan_udp_adsl_data			wan_udp_hdr.wan_udphdr_adsl_data
#define wan_udp_atm_num_frames		wan_udp_hdr.wan_udphdr_atm_num_frames
#define wan_udp_atm_ismoredata		wan_udp_hdr.wan_udphdr_atm_ismoredata	
#define wan_udp_atm_data			wan_udp_hdr.wan_udphdr_atm_data
#define wan_udp_ss7_num_frames		wan_udp_hdr.wan_udphdr_ss7_num_frames
#define wan_udp_ss7_ismoredata		wan_udp_hdr.wan_udphdr_ss7_ismoredata	
#define wan_udp_ss7_data			wan_udp_hdr.wan_udphdr_ss7_data
/*
#define wan_udp_aft_num_frames		wan_udp_hdr.wan_udphdr_aft_num_frames
#define wan_udp_aft_ismoredata		wan_udp_hdr.wan_udphdr_aft_ismoredata	
*/
#define wan_udp_data				wan_udp_hdr.wan_udphdr_data
} wan_udp_pkt_t;

#endif /* KERNEL */ 

#endif /* __WANPIPE_DEFINES_H */
