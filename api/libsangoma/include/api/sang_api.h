
#ifndef _SANG_API_H
# define _SANG_API_H

#ifndef __WINDOWS__
# define __WINDOWS__
#endif

/***********************************************************************************
	sang_api.h - API Interface Header. Should be used with "sang_status_defines.h".

Versions:
	v 1,0,0,0	March  11 2005		David Rokhvarg	<davidr@sangoma.com>
		For use with A101/A102 cards, Wanpipe version 3.0.0.0

	v 1,0,0,1	August 24 2005		David Rokhvarg	<davidr@sangoma.com>
		For use with A101/A102/A104 cards, Wanpipe version 4.0.0.0
		and later.
		The first two bytes of the "reserved" area will be set to CRC
		of a recieved HDLC frame.

	v 1,0,0,2	March  1 2006		David Rokhvarg	<davidr@sangoma.com>
		For use with A101/A102/A104/A104D cards, Wanpipe version 4.0.1.1
		and later.
		Added IOCTL_SET_IDLE_TX_BUFFER.

	v 1,0,0,3	June  29 2006		David Rokhvarg	<davidr@sangoma.com>
		For use with A101/A102/A104/A104D/A200 cards, Wanpipe version 6.0.0.0
		and later.
		Added A200 and 'if_cfg_t'.

************************************************************************************/

#if !defined(__KERNEL__)
# include <winioctl.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////

#define API_HEADER_RESERVED_LEN	10

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)
#pragma pack(1)
#endif

typedef struct _api_header{

	unsigned char	operation_status;	// operation completion status 
	unsigned short	data_length;		// set to length of data in "data" area
	unsigned int	misc_bits;			// various data-related bits
	unsigned int	time_stamp;			// timestamp of data in milliseconds
	union{
		unsigned short	crc;
		unsigned char	reserved[API_HEADER_RESERVED_LEN];
	};

}api_header_t;

#ifdef _MSC_VER
#pragma warning(pop)
#pragma pack()
#endif

///////////////////////////////////////////////////////////////////////////////////////
//Structure used to transmit and receive data
#define MAX_NO_DATA_BYTES_IN_FRAME	8188	//maximum length of data

typedef struct _TX_RX_DATA_STRUCT{

	api_header_t	api_header;
	unsigned char   data[MAX_NO_DATA_BYTES_IN_FRAME];

}TX_RX_DATA_STRUCT;

#define TX_DATA_STRUCT TX_RX_DATA_STRUCT
#define RX_DATA_STRUCT TX_RX_DATA_STRUCT

///////////////////////////////////////////////////////////////////////////////////////
// DeviceIoControl command codes. Must include "winioctl.h" in app for
// definition of CTL_CODE and defines.
//
enum _IOCTL_CODE{
	IOCTL_WRITE=1,
	IOCTL_READ,
	IOCTL_MGMT,
	IOCTL_SET_IDLE_TX_BUFFER,
	IOCTL_API_POLL
};


///////////////////////////////////////////////////////////////////////////////////////
// Command to send data:
// Input : ptr to TX_DATA_STRUCT structure
// Output: ptr to TX_DATA_STRUCT structure (may be same as input)
// Uses Direct I/O (Irp->MdlAddress). Blocking call on device handle
// to avoid busy loops.
// Note: Since this command blocks all further I/O requests, it must be done
//       on a separate device handle, in a dedicated execution thread.
//
//	This command will fail if previous write request is not complete,
//	it may happen if several user threads attempt to send data simultaneously.
#define IoctlWriteCommand	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_WRITE, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define WRITE_CMD_TIMEOUT	1000	//in milliseconds, or 1 second.

///////////////////////////////////////////////////////////////////////////////////////
// Command to receive data:
// Input : NULL
// Output: ptr to RX_DATA_STRUCT structure
// Uses Direct I/O (Irp->MdlAddress). Blocking call on device handle
// to avoid busy loops.
// Note: Since this command blocks all further I/O requests, it must be done
//       on a separate device handle, in a dedicated execution thread.
//
//	This command will fail if previous read request is not complete,
//	it may happen if several user threads attempt to receive data simultaneously.
#define IoctlReadCommand	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_READ, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define READ_CMD_TIMEOUT	1000	//in milliseconds, or 1 second.

///////////////////////////////////////////////////////////////////////////////////////
// Command to get T1/E1/56K alarms, activate/deactivate line test modes...
// Input : ptr to wan_udp_hdr_t structure.
// Output: ptr to wan_udp_hdr_t structure (may be same as input)
// Uses Buffered I/O. Synchronous call for most commands. GET_TRACE_INFO may
// block if there is no trace data.
#define IoctlManagementCommand \
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_MGMT, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define MGMT_CMD_TIMEOUT	1000	//in milliseconds, or 1 second.

// commands for IoctlManagementCommand()
enum {
	SIOC_WAN_DEVEL_IOCTL,
	SIOC_WAN_READ_REG,
	SIOC_WAN_WRITE_REG,
	SIOC_WAN_SET_PCI_BIOS,
	SIOC_WAN_COREREV,
	SIOC_WAN_FE_READ_REG,
	SIOC_WAN_FE_WRITE_REG,
	SIOC_WAN_EC_REG,
	SIOC_WAN_GET_CARD_TYPE,
	WAN_TDMV_API_IOCTL,
	SIOC_WANPIPE_API,
	GET_OPEN_HANDLES_COUNTER,

	AFT_HWEC_STATUS = 0x40,
	ROUTER_UP_TIME = 0x50,
	ENABLE_TRACING,	
	DISABLE_TRACING,
	GET_TRACE_INFO,
	READ_CODE_VERSION,
	FLUSH_OPERATIONAL_STATS,
	OPERATIONAL_STATS,
	READ_OPERATIONAL_STATS,
	READ_CONFIGURATION,
	READ_COMMS_ERROR_STATS,
	FLUSH_COMMS_ERROR_STATS,
	AFT_LINK_STATUS,
	AFT_MODEM_STATUS,
	WAN_EC_IOCTL,
	WAN_SET_RBS_BITS,
	WAN_GET_RBS_BITS
};

///////////////////////////////////////////////////////////////////////////////////////
// Command to poll API for Data/Events:
// Input : API_POLL_STRUCT
// Output: ptr to API_POLL_STRUCT structure
// Uses Direct I/O (Irp->MdlAddress). 
// Call will block, until:
//			1: Out-of-band (OOB) event: link state change (TE1 connected/disconnected)
//			2: Rx data available
//			3: Interface able to Tx (transmit buffer available)
//			4: Timeout expired
//			5: Telephony Event: DTMF, On/Off hook
//		 
// Command can be used for writing single-thread applications.
//
// Note: Since this command blocks all further I/O requests, it must be done
//       on a separate device handle, in a dedicated execution thread.
//
//	This command will fail if previous read request is not complete,
//	it may happen if several user threads attempt to receive data simultaneously.
#define IoctlApiPoll	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_API_POLL, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

typedef struct _API_POLL_STRUCT
{
	unsigned char	operation_status;	// operation completion status, check on return
	u_int32_t		user_flags_bitmap;	// bitmap of events API user is interested to receive
	u_int32_t		poll_events_bitmap;	// bitmap of events available for API user
	u_int32_t		timeout;			// time to wait for an event before returning POLL_EVENT_TIMEOUT
}API_POLL_STRUCT;

//Definitions for 'poll_events_bitmap':
#define POLL_EVENT_LINK_STATE			  1
#define POLL_EVENT_RX_DATA			(1 << 2)
#define POLL_EVENT_TX_READY			(1 << 3)
#define POLL_EVENT_TIMEOUT			(1 << 4)
//#define POLL_EVENT_TELEPHONY		(1 << 5)
#define POLL_EVENT_LINK_CONNECT		(1 << 6)
#define POLL_EVENT_LINK_DISCONNECT	(1 << 7)

#define POLLIN		(POLL_EVENT_RX_DATA | POLL_EVENT_TIMEOUT)
#define POLLOUT		(POLL_EVENT_TX_READY | POLL_EVENT_TIMEOUT)
#define POLLERR		(POLL_EVENT_LINK_STATE)
#define POLLHUP		(POLL_EVENT_LINK_STATE)


static void print_poll_event_bitmap(u_int32_t bitmap)
{
	char known_event = 0;

	if(bitmap & POLL_EVENT_LINK_STATE){
		known_event = 1;
		printf("POLL_EVENT_LINK_STATE\n");
	}
	if(bitmap & POLL_EVENT_RX_DATA){
		known_event = 1;
		printf("POLL_EVENT_RX_DATA\n");
	}
	if(bitmap & POLL_EVENT_TX_READY){
		known_event = 1;
		printf("POLL_EVENT_TX_READY\n");
	}
	if(bitmap & POLL_EVENT_TIMEOUT){
		known_event = 1;
		printf("POLL_EVENT_TIMEOUT\n");
	}
/*
	if(bitmap & POLL_EVENT_TELEPHONY){
		known_event = 1;
		printf("POLL_EVENT_TELEPHONY\n");
	}
*/
	if(bitmap & POLL_EVENT_LINK_CONNECT){
		known_event = 1;
		printf("POLL_EVENT_LINK_CONNECT\n");
	}
	if(bitmap & POLL_EVENT_LINK_DISCONNECT){
		known_event = 1;
		printf("POLL_EVENT_LINK_DISCONNECT\n");
	}

	if(known_event == 0){
		printf("Unknown event!\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//Structure used for reading device (API driver) statistics
//
typedef struct net_device_stats
{
	//transmission stats
	unsigned long tx_dropped;	//number of packets dropped for some reason
								//check messages log

	unsigned long tx_packets;	//number of packetes successfully trasmitted
	unsigned long tx_bytes;		//number of bytes successfully trasmitted

	//receive stats
	unsigned long rx_packets;	//number of packets received without an error
	unsigned long rx_bytes;		//number of bytes received without an error

	unsigned long rx_errors;	//number of packets received with CRC error
	unsigned long rx_frame_errors;	//number of packets received with Abort error

	unsigned long rx_packets_discarded_rx_q_full;	//rx queue is full.
	unsigned long rx_packets_discarded_excessive_length; //rx data is longer than the 
														 //pre-configured maximum
	
	unsigned long rx_packets_discarded_too_short;	//if length is less than:
													//	1 byte  for Primary   port
													//	2 bytes for Secondary port
	unsigned long rx_dropped;	//unrecognized packet 'type' for the Lip Layer
	//hardware level stats
	//AFT card
	unsigned long rx_fifo_errors;
	unsigned long tx_fifo_errors;
	//S508, S514
	unsigned long number_of_times_rx_buffers_out_of_synch_with_card;

	//BitStream mode
	unsigned long tx_idle_data;	//number of times idle data was transmitted

	unsigned long trace_packet_disc_len;
	unsigned long trace_packet_disc_q_full;

	//A104 additions
#define tx_carrier_errors	tx_dropped
#define tx_aborted_errors	tx_dropped
#define collisions			tx_dropped
#define rx_over_errors		rx_fifo_errors

}net_device_stats_t;

///////////////////////////////////////////////////////////////////////////////////////
// Command to Set data in Idle Transmit buffer of the driver:
// Input : ptr to TX_DATA_STRUCT structure
// Output: ptr to TX_DATA_STRUCT structure (may be same as input)
// Uses Direct I/O (Irp->MdlAddress). Synchronous call.
//
#define IoctlSetIdleTxBuffer	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_SET_IDLE_TX_BUFFER, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////
//definitions for the line trace
//
typedef struct {
	//the API header is reused for trace data
	api_header_t	trace_header;
	unsigned char	data[1];//1-st byte of trace data
}trace_data_t;

//values for 'operation_status' in 'trace_header'
#define TRC_INCOMING_DATA	0x00
#define TRC_OUTGOING_DATA   0x01

//end of definitions for the line trace
///////////////////////////////////////////////////////////////////////////////////////

#define OID_WANPIPEMON_IOCTL	19720123

/*
#define MEM_TEST_BUFFER_LEN	100
typedef struct _user_buff{
	char	*user_buff_ptr;
	int		user_buff_length;
}user_buff_t;
*/
typedef struct _rbs_management{
	int				channel;
	unsigned char	ABCD_bits;
}rbs_management_t;

typedef struct {
	char usedby[USED_BY_FIELD];
	unsigned long	active_ch;
	unsigned char	media;
	unsigned int	interface_number;
	unsigned int	sub_media;
}if_cfg_t;

typedef struct _DRIVER_VERSION {
	unsigned int major;
	unsigned int minor;
	unsigned int minor1; 
	unsigned int minor2; 
}DRIVER_VERSION, *PDRIVER_VERSION;

#endif//_SANG_API_H
