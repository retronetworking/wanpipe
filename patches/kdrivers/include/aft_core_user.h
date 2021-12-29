#ifndef  __AFT_CORE_USER__
#define  __AFT_CORE_USER__

/*================================================================
 * Includes
 *================================================================*/

# include "wanpipe_includes.h"
# include "wanpipe_debug.h"
# include "wanpipe_defines.h"
# include "wanpipe_common.h"
# include "wanpipe_cfg.h"
# include "wanrouter.h"
# include "wanpipe_api_iface.h"
# include "sdlasfm.h"
# include "sdla_front_end.h"

#if defined(__LINUX__)
# include "if_wanpipe.h"	/* Linux SIOC_ IOCTL Calls */
#endif

#if defined(__WINDOWS__) && !defined(WAN_KERNEL)
# include <winioctl.h>	/* CTL_CODE  */
#endif


#define MAX_TRACE_BUFFER	(MAX_LGTH_UDP_MGNT_PKT - 	\
				 sizeof(iphdr_t) - 		\
	 			 sizeof(udphdr_t) - 		\
				 sizeof(wan_mgmt_t) - 		\
				 sizeof(wan_trace_info_t) - 	\
		 		 sizeof(wan_cmd_t))


#define UDPMGMT_SIGNATURE		"AFTPIPEA"


/*================================================================
 * Global Sangoma Commands
 *================================================================*/

#define WAN_PROTOCOL_CMD_START	0x01
#define WAN_PROTOCOL_CMD_END	0x4F

#define WAN_UDP_CMD_START		WANPIPEMON_PROTOCOL_PRIVATE + 0x01
#define WAN_GET_PROTOCOL		(WAN_UDP_CMD_START+0)
#define WAN_GET_PLATFORM		(WAN_UDP_CMD_START+1)
#define WAN_GET_MEDIA_TYPE		(WAN_UDP_CMD_START+2)
#define WAN_GET_MASTER_DEV_NAME	(WAN_UDP_CMD_START+3)
#define WAN_UDP_CMD_END			WANPIPEMON_PROTOCOL_PRIVATE + 0x1F
#define WANPIPEMON_DRIVER_PRIVATE	WAN_UDP_CMD_END+1

#define WAN_FE_CMD_START	0x90
#define WAN_FE_CMD_END		0x9F

#define WAN_INTERFACE_CMD_START	0xA0
#define WAN_INTERFACE_CMD_END	0xAF

#define WAN_FE_UDP_CMD_START	0xB0
#define WAN_FE_UDP_CMD_END	0xBF

#define TRACE_ALL                       0x00
#define TRACE_PROT			0x01
#define TRACE_DATA			0x02

/* values for request/reply byte */
#define UDPMGMT_REQUEST	0x01
#define UDPMGMT_REPLY	0x02
#define UDP_OFFSET	12


#define AFT_SERIAL_MODEM_RTS  	1
#define AFT_SERIAL_MODEM_DTR  	2
#define AFT_SERIAL_MODEM_CTS  	4
#define AFT_SERIAL_MODEM_DCD  	8


#if defined(__LINUX__)
enum {
	SIOC_AFT_CUSTOMER_ID = SIOC_WANPIPE_DEVPRIVATE,
	SIOC_AFT_SS7_FORCE_RX,
    SIOC_WANPIPE_API
};
#endif

/* Possible error_flagRX packet errors */ 
enum {
	WP_FIFO_ERROR_BIT,
	WP_CRC_ERROR_BIT,
	WP_ABORT_ERROR_BIT,
};

/*================================================================
 * Trace Structure
 *================================================================*/

/*================================================================
 * Global Sangoma Commands
 *================================================================*/

#define WAN_PROTOCOL_CMD_START	0x01
#define WAN_PROTOCOL_CMD_END	0x4F

#define WAN_FE_CMD_START	0x90
#define WAN_FE_CMD_END		0x9F

#define WAN_INTERFACE_CMD_START	0xA0
#define WAN_INTERFACE_CMD_END	0xAF

#define WAN_FE_UDP_CMD_START	0xB0
#define WAN_FE_UDP_CMD_END	0xBF

#define TRACE_ALL                       0x00
#define TRACE_PROT			0x01
#define TRACE_DATA			0x02

/* values for request/reply byte */
#define UDPMGMT_REQUEST	0x01
#define UDPMGMT_REPLY	0x02
#define UDP_OFFSET	12



/*================================================================
 * Trace Structure
 *================================================================*/

/* the line trace status element presented by the frame relay code */
typedef struct {
        unsigned char 	flag	; /* ready flag */
        unsigned short 	length   ; /* trace length */
        unsigned char 	rsrv0[2]  ; /* reserved */
        unsigned char 	attr      ; /* trace attributes */
        unsigned short 	tmstamp  ; /* time stamp */
        unsigned char 	rsrv1[4]  ; /* reserved */
        unsigned int	offset    ; /* buffer absolute address */
}aft_trc_el_t;

typedef struct {
	unsigned char	num_frames;
	unsigned char	ismoredata;
} wan_trace_info_t;


/*================================================================
 * Statistics Structures
 *================================================================*/

#pragma pack(1)

/* the operational statistics structure */
typedef struct {

	/* Data frame transmission statistics */
	unsigned int Data_frames_Tx_count ;
	/* # of frames transmitted */
	unsigned int Data_bytes_Tx_count ;
	/* # of bytes transmitted */
	unsigned int Data_Tx_throughput ;
	/* transmit throughput */
	unsigned int no_ms_for_Data_Tx_thruput_comp ;
	/* millisecond time used for the Tx throughput computation */
	unsigned int Tx_Data_discard_lgth_err_count ;

	/* Data frame reception statistics */
	unsigned int Data_frames_Rx_count ;
	/* number of frames received */
	unsigned int Data_bytes_Rx_count ;
	/* number of bytes received */
	unsigned int Data_Rx_throughput ;
	/* receive throughput */
	unsigned int no_ms_for_Data_Rx_thruput_comp ;
	/* millisecond time used for the Rx throughput computation */
	unsigned int Rx_Data_discard_short_count ;
	/* received Data frames discarded (too short) */
	unsigned int Rx_Data_discard_long_count ;
	/* received Data frames discarded (too long) */
	unsigned int Rx_Data_discard_inactive_count ;
	/* received Data frames discarded (link inactive) */

	/* Incomming frames with a format error statistics */
	unsigned short Rx_frm_incomp_CHDLC_hdr_count ;
	/* frames received of with incomplete Cisco HDLC header */
	unsigned short Rx_frms_too_long_count ;
	/* frames received of excessive length count */

	/* CHDLC link active/inactive and loopback statistics */
	unsigned short link_active_count ;
	/* number of times that the link went active */
	unsigned short link_inactive_modem_count ;
	/* number of times that the link went inactive (modem failure) */
	unsigned short link_inactive_keepalive_count ;
	/* number of times that the link went inactive (keepalive failure) */
	unsigned short link_looped_count ;
	/* link looped count */

	unsigned int Data_frames_Tx_realign_count;

} aft_op_stats_t;

typedef struct {
	unsigned short Rx_overrun_err_count;
	unsigned short Rx_crc_err_count ;		/* receiver CRC error count */
	unsigned short Rx_abort_count ; 	/* abort frames recvd count */
	unsigned short Rx_hdlc_corrupiton;	/* receiver disabled */
	unsigned short Rx_pci_errors;		/* missed tx underrun interrupt count */
	unsigned short Rx_dma_descr_err;   	/*secondary-abort frames tx count */
	unsigned short DCD_state_change_count ; /* DCD state change */
	unsigned short CTS_state_change_count ; /* CTS state change */

	unsigned short Tx_pci_errors;		/* missed tx underrun interrupt count */
	unsigned short Tx_dma_errors;		/* missed tx underrun interrupt count */

	unsigned int Tx_pci_latency;		/* missed tx underrun interrupt count */
	unsigned int Tx_dma_len_nonzero;	/* Tx dma descriptor len not zero */

} aft_comm_err_stats_t;



typedef struct wan_if_cfg{
	unsigned char   usedby;
    unsigned int    active_ch;		/* Active channels/timslots configured */
	unsigned int    ec_active_ch;	/* Echo Canceller configured active channels/timeslots */
	unsigned int	cfg_active_ch;  /* User specified active channel configuration */
    unsigned char   media;
    unsigned int    interface_number;
    unsigned int    sub_media;
    unsigned char   hw_coding;
    unsigned int    chunk_sz;
    sdla_fe_cfg_t   fe_cfg;
	char line_mode[USED_BY_FIELD];/* HDLC, BitStream, BRI D-chan... */
}wan_if_cfg_t;

#define if_cfg_t wan_if_cfg_t

#if 0
#define IFNAMSIZ	256
#define USED_BY_FIELD	128	/* max length of the used by field */
typedef struct {
	char usedby[USED_BY_FIELD];
	unsigned long	active_ch;
	unsigned char	media;
	unsigned int	interface_number;
	unsigned int	sub_media;
	char line_mode[USED_BY_FIELD];/* HDLC, BitStream */
}if_cfg_t;
#endif

#pragma pack()


typedef struct global_stats
{
	unsigned long isr_entry;
	unsigned long isr_already_critical;
	unsigned long isr_rx;
	unsigned long isr_tx;
	unsigned long isr_intr_test;
	unsigned long isr_spurious;
	unsigned long isr_enable_tx_int;
	unsigned long rx_intr_corrupt_rx_bfr;
	unsigned long rx_intr_on_orphaned_DLCI;
	unsigned long rx_intr_dev_not_started;
	unsigned long tx_intr_dev_not_started;
	unsigned long poll_entry;
	unsigned long poll_already_critical;
	unsigned long poll_processed;
	unsigned long poll_tbusy_bad_status;
	unsigned long poll_host_disable_irq;
	unsigned long poll_host_enable_irq;

} global_stats_t;


typedef struct if_send_stat{
	unsigned long if_send_entry;
	unsigned long if_send_skb_null;
	unsigned long if_send_broadcast;
	unsigned long if_send_multicast;
	unsigned long if_send_critical_ISR;
	unsigned long if_send_critical_non_ISR;
	unsigned long if_send_tbusy;
	unsigned long if_send_tbusy_timeout;
	unsigned long if_send_PIPE_request;
	unsigned long if_send_wan_disconnected;
	unsigned long if_send_dlci_disconnected;
	unsigned long if_send_no_bfrs;
	unsigned long if_send_adptr_bfrs_full;
	unsigned long if_send_bfr_passed_to_adptr;
	unsigned long if_send_protocol_error;
       	unsigned long if_send_bfr_not_passed_to_adptr;
       	unsigned long if_send_tx_int_enabled;
        unsigned long if_send_consec_send_fail;
} if_send_stat_t;

typedef struct rx_intr_stat{
	unsigned long rx_intr_no_socket;
	unsigned long rx_intr_dev_not_started;
	unsigned long rx_intr_PIPE_request;
	unsigned long rx_intr_bfr_not_passed_to_stack;
	unsigned long rx_intr_bfr_passed_to_stack;
} rx_intr_stat_t;

typedef struct pipe_mgmt_stat{
	unsigned long UDP_PIPE_mgmt_kmalloc_err;
	unsigned long UDP_PIPE_mgmt_direction_err;
	unsigned long UDP_PIPE_mgmt_adptr_type_err;
	unsigned long UDP_PIPE_mgmt_adptr_cmnd_OK;
	unsigned long UDP_PIPE_mgmt_adptr_cmnd_timeout;
	unsigned long UDP_PIPE_mgmt_adptr_send_passed;
	unsigned long UDP_PIPE_mgmt_adptr_send_failed;
	unsigned long UDP_PIPE_mgmt_not_passed_to_stack;
	unsigned long UDP_PIPE_mgmt_passed_to_stack;
	unsigned long UDP_PIPE_mgmt_no_socket;
        unsigned long UDP_PIPE_mgmt_passed_to_adptr;
} pipe_mgmt_stat_t;



#if defined(__WINDOWS__)

///////////////////////////////////////////////////////////////////////////////////////
// Command to send data:
// Input : ptr to TX_DATA_STRUCT structure
// Output: ptr to TX_DATA_STRUCT structure (may be same as input)
// Uses Direct I/O (Irp->MdlAddress). Blocking call on device handle
// to avoid busy loops.
// Note: Since this command blocks all further I/O requests, it must be done
//       on a separate device handle, in a dedicated execution thread.
//
//	This command is thread safe.
#define IoctlWriteCommand	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_WRITE, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

// The same as IoctlWriteCommand, but will NOT block.
#define IoctlWriteCommandNonBlocking	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, WANPIPE_IOCTL_WRITE_NON_BLOCKING, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////
// Command to receive data:
// Input : NULL
// Output: ptr to RX_DATA_STRUCT structure
// Uses Direct I/O (Irp->MdlAddress). Blocking call on device handle
// to avoid busy loops.
// Note: Since this command blocks all further I/O requests, it must be done
//       on a separate device handle, in a dedicated execution thread.
//
//	This command is thread safe.
#define IoctlReadCommand	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_READ, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// The same as IoctlReadCommand, but will NOT block.
#define IoctlReadCommandNonBlocking	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, WANPIPE_IOCTL_READ_NON_BLOCKING, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////
// Command to control TDM API:
// Input : ptr to wanpipe_tdm_api_cmd_t structure.
// Output: ptr to wanpipe_tdm_api_cmd_t structure (may be same as input)
// Uses Buffered I/O, Synchronous call.
// This command is thread safe.
#define IoctlTdmApiCommand	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, WANPIPE_IOCTL_TDM_API, METHOD_BUFFERED, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////
// Command to get T1/E1/56K alarms, activate/deactivate line test modes...
// Input : ptr to wan_udp_hdr_t structure.
// Output: ptr to wan_udp_hdr_t structure (may be same as input)
// Uses Buffered I/O, Synchronous call.
// This command is thread safe.
#define IoctlManagementCommand \
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_MGMT, METHOD_BUFFERED, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
// Command to poll API for Data/Events:
// Input : API_POLL_STRUCT
// Output: ptr to API_POLL_STRUCT structure
// Uses Direct I/O (Irp->MdlAddress).
// Call will block, until:
//			1: Out-of-band (OOB) event: link state change (TE1 connected/disconnected)
//			2: Rx data available
//			3: Interface able to Tx (transmit buffer available)
//			4: Telephony Event: DTMF, On/Off hook
//
// Command can be used for writing single-thread applications.
//
// Note: Since this command blocks all further I/O requests, it must be done
//       on a separate device handle, in a dedicated execution thread.
//
// This command is thread safe.
#define IoctlApiPoll	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_API_POLL, METHOD_IN_DIRECT, FILE_ANY_ACCESS)


//Definitions for 'poll_events_bitmap' and 'user_flags_bitmap' in API_POLL_STRUCT:
#define POLL_EVENT_RX_DATA			1
#define POLL_EVENT_TX_READY			(1 << 2)
#define POLL_EVENT_OOB				(1 << 3) /* Out-Of-Band events such as Line Connect/Disconnect,
												RBS change, Ring, On/Off Hook, DTMF... */

#define POLLIN		(POLL_EVENT_RX_DATA)
#define POLLOUT		(POLL_EVENT_TX_READY)
#define POLLPRI		(POLL_EVENT_OOB)
#define POLLHUP		POLLPRI
#define POLLERR		POLLPRI

#if defined(__KERNEL__)
#define PRINT_BITMAP	DEBUG_TDMAPI
#else
#define PRINT_BITMAP	printf
#endif

static void print_poll_event_bitmap(u_int32_t bitmap)
{
	char known_event = 0;

	if(bitmap & POLL_EVENT_RX_DATA){
		known_event = 1;
		PRINT_BITMAP("POLL_EVENT_RX_DATA\n");
	}
	if(bitmap & POLL_EVENT_TX_READY){
		known_event = 1;
		PRINT_BITMAP("POLL_EVENT_TX_READY\n");
	}
	if(bitmap & POLL_EVENT_OOB){
		known_event = 1;
		PRINT_BITMAP("POLL_EVENT_OOB\n");
	}
	if(known_event == 0){
		PRINT_BITMAP("Unknown event!\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Command to Set data in Idle Transmit buffer of the driver:
// Input : ptr to TX_DATA_STRUCT structure
// Output: ptr to TX_DATA_STRUCT structure (may be same as input)
// Uses Direct I/O (Irp->MdlAddress). Synchronous call.
//
#define IoctlSetIdleTxBuffer	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_SET_IDLE_TX_BUFFER, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////
// This IOCTL for use only by Sangoma Technical Support
// Command to control cdev:
// Input : ptr to wanpipe_tdm_api_cmd_t structure.
// Output: ptr to wanpipe_tdm_api_cmd_t structure (may be same as input)
// Uses Buffered I/O, Synchronous call.
// This command is thread safe.
#define IoctlCdevControlCommand	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, WANPIPE_IOCTL_CDEV_CTRL, METHOD_BUFFERED, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////
// Command to Set a "Shared Event" which can be used for
//	WaitForSingleObject() and WaitForMultipleObjects() functions before
//	calling the blocking IoctlReadCommand. Once the "Shared Object" is set
//	to signalled state by the API driver, call IoctlReadCommand.
//
// Input : ptr to REGISTER_EVENT structure
// Output: ptr to REGISTER_EVENT structure (may be same as input)
// Uses Direct I/O (Irp->MdlAddress). Synchronous call.
//
typedef struct _REGISTER_EVENT
{
	unsigned char	operation_status;	// operation completion status
	HANDLE			hEvent;
} REGISTER_EVENT , *PREGISTER_EVENT ;

#define SIZEOF_REGISTER_EVENT  sizeof(REGISTER_EVENT)

#define IoctlSetSharedEvent	\
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_SET_SHARED_EVENT, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////
//this IOCTL is for 'Port', for example: WANPIPE1.
#define IoctlPortConfigurationCommand \
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_PORT_CONFIG, METHOD_BUFFERED, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////
//this IOCTL is for 'Port', for example: WANPIPE1.
#define IoctlPortManagementCommand \
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_PORT_MGMT, METHOD_BUFFERED, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////////////
#define WP_CMD_TIMEOUT		1000	//in milliseconds, or 1 second.
#define MGMT_CMD_TIMEOUT	WP_CMD_TIMEOUT
#define WRITE_CMD_TIMEOUT	WP_CMD_TIMEOUT
#define READ_CMD_TIMEOUT	WP_CMD_TIMEOUT

#endif /* __WINDOWS__ */


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


// Command codes for IoctlPortManagementCommand():
enum {
	GET_HARDWARE_INFO=1,		//Fill in "hardware_info_t" structure.

	STOP_PORT,					//Stop Port. Prior to calling this command all 'handles'
								//to communication interfaces such as WANPIPE1_IF0 must
								//be closed using CloseHandle() system call.

	START_PORT_VOLATILE_CONFIG,	//Start Port. Use configuration stored in a Port's Driver memory buffer.
								//This command runs faster than START_PORT_REGISTRY_CFG.
								//Recommended for use if Port is restarted *a lot*.

	START_PORT_REGISTRY_CONFIG,	//Start Port. Use configuration stored in the Port's Registry key.
								//This command runs slower than START_PORT_VOLATILE_CFG.
								//Recommended for use if Port is *not* restarted often (most cases).

	GET_DRIVER_VERSION,			//Fill in "DRIVER_VERSION" structure.

	GET_PORT_OPERATIONAL_STATS,	//Fill in "port_stats_t" structure.

	FLUSH_PORT_OPERATIONAL_STATS//Reset port's statistics counters in API driver.
};

/* port statistics for low-level debugging and techsupport */
typedef struct {
	unsigned int rx_fifo_interrupt_counter;/* receive fifo error interrupt counter (total for all channels groups) */
	unsigned int tx_fifo_interrupt_counter;/* transmit fifo error interrupt counter (total for all channels groups)*/
	unsigned int fifo_err_interrupt_counter;/* total number of fifo error interrupts */
	unsigned int chip_security_interrupt_counter;/* chip security 'bit set' counter */
	unsigned int front_end_interrupt_counter;
	unsigned int isr_entry_counter;/* Interrupt Service Routine entry counter */
	unsigned int recognized_interrupt_counter;/* interrupt was generated by this port */
	unsigned int unrecognized_interrupt_counter;/* caused by interrupt sharing with other ports and cards */
	unsigned int isr_critical_flag_set_counter;
	unsigned int isr_hwec_poll_counter;
	unsigned int wdt_interrupt_counter;
	unsigned int dma_interrupt_counter;/* DMA for data complete interrupt counter */
	unsigned int hwec_chip_security_interrupt_counter;/* hwec chip security 'bit set' counter */
	unsigned int synch_loss_with_fe_chip_interrupt_counter;
}port_stats_t;


#define CARD_SERIAL_NUMBER_LENGTH 64
// data structure for GET_HARDWARE_INFO command
typedef struct {
	int	card_model;		/* A101/102/104/108/200... */
	int	firmware_version;
	int	pci_bus_number;
	int	pci_slot_number;
	/* Number of HW Echo Canceller channels.
	    Zero means HW Echo Canceller not installed the card. */
	int	max_hw_ec_chans;

	/* Port's number (zero based). */
	int port_number;
	char serial_number[CARD_SERIAL_NUMBER_LENGTH];	/* Not implemented, for future use. */
}hardware_info_t;

typedef struct{
	int	wanpipe_number;
	hardware_info_t hardware_info;
}wanpipe_instance_info_t;


///////////////////////////////////////////////////////////////////////////////////////
//Structure used with IoctlPortManagementCommand
#define PORT_MGMT_MAX_DATA_SIZE	8188

typedef struct {

	unsigned int	command_code;		/* Management Command Code */
	unsigned int	operation_status;	/* operation completion status */
	unsigned short	port_no;			/* port number */
	unsigned char	data[PORT_MGMT_MAX_DATA_SIZE]; /* data buffer passed from/to caller */
}port_management_struct_t;


// Commands for IoctlPortConfigurationCommand().
// Used together with 'port_cfg_t' from wanpipe_cfg.h
enum {

	GET_PORT_VOLATILE_CONFIG=1,	//Get current Port configuration stored in a Port's Driver memory buffer.

	SET_PORT_VOLATILE_CONFIG,	//Set new Port configuration in a Port's Driver memory buffer.
								//Prior to using this command Port must be stopped with STOP_PORT command.
								//This command will not update Port's Property Pages in the Device Manager.
								//Recommended for use if Port is restarted *a lot*.
};



#if defined(__WINDOWS__)
static wan_driver_version_t drv_version = {	WANPIPE_VERSION_MAJOR,
					WANPIPE_VERSION_MINOR,
					WANPIPE_VERSION_MINOR1,
					WANPIPE_VERSION_MINOR2
				};
#endif

#endif
