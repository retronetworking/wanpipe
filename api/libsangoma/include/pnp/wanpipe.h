/*****************************************************************************
* wanpipe.h	WANPIPE(tm) Multiprotocol WAN Link Driver.
*		User-level API definitions.
*
* Author: 	Nenad Corbic <ncorbic@sangoma.com>
*		Gideon Hack  	
*
* Copyright:	(c) 1995-2000 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Nov 3,  2000  Nenad Corbic    Added config_id to sdla_t structure.
*                               Used to determine the protocol running.
* Jul 13, 2000  Nenad Corbic	Added SyncPPP Support
* Feb 24, 2000  Nenad Corbic    Added support for x25api driver
* Oct 04, 1999  Nenad Corbic    New CHDLC and FRAME RELAY code, SMP support
* Jun 02, 1999  Gideon Hack	Added 'update_call_count' for Cisco HDLC 
*				support
* Jun 26, 1998	David Fong	Added 'ip_mode' in sdla_t.u.p for dynamic IP
*				routing mode configuration
* Jun 12, 1998	David Fong	Added Cisco HDLC union member in sdla_t
* Dec 08, 1997	Jaspreet Singh  Added 'authenticator' in union of 'sdla_t' 
* Nov 26, 1997	Jaspreet Singh	Added 'load_sharing' structure.  Also added 
*				'devs_struct','dev_to_devtint_next' to 'sdla_t'	
* Nov 24, 1997	Jaspreet Singh	Added 'irq_dis_if_send_count', 
*				'irq_dis_poll_count' to 'sdla_t'.
* Nov 06, 1997	Jaspreet Singh	Added a define called 'INTR_TEST_MODE'
* Oct 20, 1997	Jaspreet Singh	Added 'buff_intr_mode_unbusy' and 
*				'dlci_intr_mode_unbusy' to 'sdla_t'
* Oct 18, 1997	Jaspreet Singh	Added structure to maintain global driver
*				statistics.
* Jan 15, 1997	Gene Kozin	Version 3.1.0
*				 o added UDP management stuff
* Jan 02, 1997	Gene Kozin	Version 3.0.0
*****************************************************************************/
#ifndef	_WANPIPE_H
#define	_WANPIPE_H

#if defined(__LINUX__)
# include <linux/wanrouter.h>
# include <linux/wanpipe_kernel.h>
#elif defined(__WINDOWS__)

# include <wanrouter.h>
# include <sdladrv.h>
#endif

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

/* Due to changes between 2.4.9 and 2.4.13,
* I decided to write my own min() and max()
* functions */
#if !defined(__WINDOWS__)

# define wp_min(x,y) \
({ unsigned int __x = (x); unsigned int __y = (y); __x < __y ? __x: __y; })
# define wp_max(x,y) \
({ unsigned int __x = (x); unsigned int __y = (y); __x > __y ? __x: __y; })

#else
# define wp_min(x,y) ((unsigned int)x < (unsigned int)y ? x : y)
# define wp_max(x,y) ((unsigned int)x > (unsigned int)y ? x : y)
#endif

#ifdef LINUX_2_4
#ifndef AF_WANPIPE
#define AF_WANPIPE 25
#ifndef PF_WANPIPE
#define PF_WANPIPE AF_WANPIPE
#endif
#endif

#else
#ifndef AF_WANPIPE
#define AF_WANPIPE 24
#ifndef PF_WANPIPE
#define PF_WANPIPE AF_WANPIPE
#endif
#endif
#endif

#define AF_ANNEXG_WANPIPE AF_WANPIPE
#define PF_ANNEXG_WANPIPE AF_ANNEXG_WANPIPE


/* Defines */

#if defined(__WINDOWS__)
# define	PACKED
#else
# ifndef	PACKED
#  define	PACKED	__attribute__((packed))
# endif
#endif

#define	WANPIPE_MAGIC	0x414C4453L	/* signature: 'SDLA' reversed */

/* IOCTL numbers (up to 16) */
#define	WANPIPE_DUMP	(ROUTER_USER+0)	/* dump adapter's memory */
#define	WANPIPE_EXEC	(ROUTER_USER+1)	/* execute firmware command */

#define TRACE_ALL                       0x00
#define TRACE_PROT			0x01
#define TRACE_DATA			0x02

/* values for request/reply byte */
#define UDPMGMT_REQUEST	0x01
#define UDPMGMT_REPLY	0x02
#define UDP_OFFSET	12

#define MAX_CMD_BUFF 	10
#define MAX_X25_LCN 	255	/* Maximum number of x25 channels */
#define MAX_LCN_NUM	4095	/* Maximum lcn number */
#define MAX_FT1_RETRY 	100


#define TX_TIMEOUT 5*HZ

#define MAX_API_RX_QUEUE 10

/* General Critical Flags */
enum {
	SEND_CRIT,
	PERI_CRIT,
	RX_CRIT,
	PRIV_CRIT
};

/* TE timer critical flags */
/* #define LINELB_TIMER_RUNNING 0x04 - define in sdla_te1_def.h */

/* Bit maps for dynamic interface configuration
* DYN_OPT_ON : turns this option on/off 
* DEV_DOWN   : device was shutdown by the driver not
*              by user 
*/
#define DYN_OPT_ON	0x00
#define DEV_DOWN	0x01

/*
* Data structures for IOCTL calls.
*/

typedef struct sdla_dump	/* WANPIPE_DUMP */
{
	unsigned long magic;	/* for verification */
	unsigned long offset;	/* absolute adapter memory address */
	unsigned long length;	/* block length */
	void* ptr;		/* -> buffer */
} sdla_dump_t;

typedef struct sdla_exec	/* WANPIPE_EXEC */
{
	unsigned long magic;	/* for verification */
	void* cmd;		/* -> command structure */
	void* data;		/* -> data buffer */
} sdla_exec_t;

/* UDP management stuff */

typedef struct wum_header
{
	unsigned char signature[8];	/* 00h: signature */
	unsigned char type;		/* 08h: request/reply */
	unsigned char command;		/* 09h: commnand */
	unsigned char reserved[6];	/* 0Ah: reserved */
} wum_header_t;

/*************************************************************************
Data Structure for global statistics
*************************************************************************/

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

#if 1

#pragma pack(1)

typedef struct{
	unsigned short	udp_src_port		PACKED;
	unsigned short	udp_dst_port		PACKED;
	unsigned short	udp_length		PACKED;
	unsigned short	udp_checksum		PACKED;
} udp_pkt_t;


typedef struct {
	unsigned char	ver_inet_hdr_length	PACKED;
	unsigned char	service_type		PACKED;
	unsigned short	total_length		PACKED;
	unsigned short	identifier		PACKED;
	unsigned short	flags_frag_offset	PACKED;
	unsigned char	ttl			PACKED;
	unsigned char	protocol		PACKED;
	unsigned short	hdr_checksum		PACKED;
	unsigned long	ip_src_address		PACKED;
	unsigned long	ip_dst_address		PACKED;
} ip_pkt_t;


typedef struct {
	unsigned char           signature[8]    PACKED;
	unsigned char           request_reply   PACKED;
	unsigned char           id              PACKED;
	unsigned char           reserved[6]     PACKED;
} wp_mgmt_t;

#pragma pack()

#endif

/*************************************************************************
Data Structure for if_send  statistics
*************************************************************************/  
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


typedef struct {
	struct sk_buff *skb;
} bh_data_t, cmd_data_t;

#define MAX_LGTH_UDP_MGNT_PKT 2000


/* This is used for interrupt testing */
#define INTR_TEST_MODE	0x02

#define	WUM_SIGNATURE_L	0x50495046
#define	WUM_SIGNATURE_H	0x444E3845

#define	WUM_KILL	0x50
#define	WUM_EXEC	0x51


#ifdef	__KERNEL__
/****** Kernel Interface ****************************************************/

#if defined(__LINUX__)
# include <linux/wanpipe_debug.h>
# include <linux/sdladrv.h>	/* SDLA support module API definitions */
# include <linux/sdlasfm.h>	/* SDLA firmware module definitions */
# include <linux/tqueue.h>
# include <linux/wanpipe_common.h>
# ifdef LINUX_2_4
# include <linux/serial.h>
# include <linux/serialP.h>
# include <linux/serial_reg.h>
# include <asm/serial.h>
# endif
# include <linux/tty.h>
# include <linux/tty_driver.h>
# include <linux/tty_flip.h>

#elif defined(__WINDOWS__)

# include <sdlasfm.h>	/* SDLA firmware module definitions */
# include <wanpipe_abstr.h>

#endif


#define MAX_E1_CHANNELS 32
#define MAX_FR_CHANNELS (1007+1)

#define WAN_ENABLE	0x01
#define WAN_DISABLE	0x02

/*
for T1/E1 card maxium is 32, for A200 maximum is 24 modules,
using the highest value
*/
#define MAX_TDM_API_CHANNELS	32

#ifndef	min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef	max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#define	is_digit(ch) (((ch)>=(unsigned)'0'&&(ch)<=(unsigned)'9')?1:0)
#define	is_alpha(ch) ((((ch)>=(unsigned)'a'&&(ch)<=(unsigned)'z')||\
((ch)>=(unsigned)'A'&&(ch)<=(unsigned)'Z'))?1:0)
#define	is_hex_digit(ch) ((((ch)>=(unsigned)'0'&&(ch)<=(unsigned)'9')||\
	((ch)>=(unsigned)'a'&&(ch)<=(unsigned)'f')||\
((ch)>=(unsigned)'A'&&(ch)<=(unsigned)'F'))?1:0)


/****** Data Structures *****************************************************/

#if defined(__WINDOWS__)

enum DEVICE_CONFIGURATION_RETURN_CODE{

	DEVICE_CFG_OK = 1,
	DEVICE_CFG_FAILED_COMMS_ENABLED,
	DEVICE_CFG_FAILED_INVALID_LENGTH_OF_DATA_QUEUE,	//must be between MINIMUM_LENGTH_OF_DATA_QUEUE
													//and MAXIMUM_LENGTH_OF_DATA_QUEUE

	DEVICE_CFG_FAILED_INVALID_DATA_LENGTH,		//must be at least	MINIMUM_LENGTH_OF_DATA
	DEVICE_CFG_FAILED_MEMORY_ALLOCATION_ERR
};

//Declarations for queueing DPCs from XILINX ISR.
//It should be in sdla_xilinx.h but 'api_rx_hdr_t'
//declared there will fail compilation.

//used to create bitmap of last interrupt
enum XILINX_INTERRUPT_TYPE{
	X_SECURITY_CONFLICT,
	X_FRONT_END_INTR,
	X_FIFO_ERR_INTR,
	X_RX_DMA_INTR,
	X_TX_DMA_INTR
};

typedef struct{

	unsigned int interrupt_type;	//bitmap

	unsigned long xilinx_chip_cfg_reg;	//copy of tx interrupt register

	unsigned long rx_interrupt_reg;	//copy of rx interrupt register
	unsigned long tx_interrupt_reg;	//copy of tx interrupt register

}xilinx_interrupt_t;


VOID xilinx_security_err_dpc(
	IN PKDPC	Dpc, 
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP		someIrp, 
	IN PVOID	Context
	);

enum _XILINX_FE_DPC_TYPE{
	FE_DPC_TASK=1,
	ENABLE_HWEC_TIMER
};

typedef struct{

	unsigned long te_timer_delay;

}xilinx_front_end_dpc_t;


VOID xilinx_front_end_dpc(
	IN PKDPC	Dpc, 
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP		someIrp, 
	IN PVOID	Context
	);

VOID xilinx_fifo_error_dpc(
	IN PKDPC	Dpc, 
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP		someIrp, 
	IN PVOID	Context
	);

VOID rx_dpc_func(
	IN PKDPC	Dpc, 
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP		someIrp, 
	IN PVOID	Context
	);

VOID tx_dpc_func(
	IN PKDPC	Dpc, 
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP		someIrp, 
	IN PVOID	Context
	);

VOID aft_te1_front_end_dpc(
	IN PKDPC	Dpc, 
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP		someIrp, 
	IN PVOID	Context
	);

VOID aft_te1_fifo_error_dpc(
	IN PKDPC	Dpc, 
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP		someIrp, 
	IN PVOID	Context
	);

unsigned char copy_rx_data_to_wanpipe_rx_buffer(
	void *sdla_net_dev
	);

////////////////////////////////////////////////////////////////
//functions in pnp.c
int allocate_dma_buffs(	PDEVICE_OBJECT DeviceObject,
						netdevice_t * sdla_net_device);

void deallocate_dma_buffs(netdevice_t * sdla_net_device);

////////////////////////////////////////////////////////////////
void translate_te1_structures(sdla_fe_t *fe, void *p_udp_cmd, unsigned char* data);

#endif


typedef struct 
{	/* Cisco HDLC-specific data */
	char if_name[WAN_IFNAME_SZ+1];	/* interface name */
	int	comm_port;/* Communication Port O or 1 */
	unsigned char usedby;  /* Used by WANPIPE or API */
	unsigned long	rxmb_off;	/* Receive mail box */
	/*unsigned long	flags_off;*/	/* flags */
	unsigned long	txbuf_off;	/* -> current Tx buffer */
	unsigned long	txbuf_base_off;	/* -> first Tx buffer */
	unsigned long	txbuf_last_off;	/* -> last Tx buffer */
	unsigned long	rxbuf_base_off;	/* -> first Rx buffer */
	unsigned long	rxbuf_last_off;	/* -> last Rx buffer */
	unsigned long	rx_base_off;	/* S508 receive buffer base */
	unsigned long	rx_top_off;	/* S508 receive buffer end */
	void* tx_status;	/* Tx status element */
	void* rx_status;	/* Rx status element */
	unsigned char receive_only; /* high speed receivers */
	unsigned short protocol_options;
	unsigned short kpalv_tx;	/* Tx kpalv timer */
	unsigned short kpalv_rx;	/* Rx kpalv timer */
	unsigned short kpalv_err;	/* Error tolerance */
	unsigned short slarp_timer;	/* SLARP req timer */
	unsigned state;			/* state of the link */
	unsigned char api_status;
	unsigned char update_call_count;
	unsigned short api_options;	/* for async config */
	unsigned char  async_mode;
        unsigned short tx_bits_per_char;
        unsigned short rx_bits_per_char;
        unsigned short stop_bits;
        unsigned short parity;
	unsigned short break_timer;
        unsigned short inter_char_timer;
        unsigned short rx_complete_length;
        unsigned short xon_char;
        unsigned short xoff_char;
	/* FIXME: Move this out of the union */
	unsigned char comm_enabled; /* Is comm enabled or not */
	/* FIXME: Move this out of the union */
	unsigned char backup;
	int 		TracingEnabled;		/* For enabling Tracing */
	unsigned long 	curr_trace_addr;	/* Used for Tracing */
	unsigned long 	start_trace_addr;
	unsigned long 	end_trace_addr;
	unsigned long 	base_addr_trace_buffer;
	unsigned long 	end_addr_trace_buffer;
	unsigned short 	number_trace_elements;
	unsigned  	available_buffer_space;
	unsigned long 	router_start_time;
	unsigned char 	route_status;
	unsigned char 	route_removed;
	unsigned long 	tick_counter;	
	/* FIXME: Move this out of the union */
	unsigned short 	timer_int_enabled;
	unsigned long 	router_up_time;
#if defined(__LINUX__)
	spinlock_t if_send_lock;
#endif
	void * prot;

} sdla_chdlc_t;

typedef struct 
{
	unsigned long time_slot_map;
	unsigned long logic_ch_map;
	unsigned char num_of_time_slots;
	unsigned char top_logic_ch;
	unsigned long bar;
	void *trace_info;
	void *dev_to_ch_map[MAX_E1_CHANNELS];
	void *rx_dma_ptr;
	void *tx_dma_ptr;
	wan_xilinx_conf_t	cfg;
	unsigned long  	dma_mtu_off;
	unsigned short 	dma_mtu;
	unsigned char 	state_change_exit_isr;
	unsigned long 	active_ch_map;
	unsigned long 	fifo_addr_map;
//	wan_timer_t 	led_timer;
	unsigned char 	tdmv_sync;
	unsigned int	chip_cfg_status;	
	wan_taskq_t 	port_task;
	unsigned int 	port_task_cmd;
	unsigned long	wdt_rx_cnt;
	unsigned long	wdt_tx_cnt;
	unsigned int	security_id;
	unsigned int	security_cnt;
	unsigned char	firm_ver;
	unsigned char	firm_id;
	unsigned int	chip_security_cnt;
	unsigned long	rx_timeout,gtimeout;
	unsigned int	comm_enabled;
	unsigned int	lcfg_reg;
	unsigned int    tdmv_master_if_up;
	unsigned int	tdmv_mtu;
	unsigned int	tdmv_zaptel_cfg;
	netskb_t	*tdmv_api_rx;
	netskb_t	*tdmv_api_tx;
	wan_skb_queue_t	tdmv_api_tx_list;

	unsigned char	led_ctrl;
	unsigned int	tdm_logic_ch_map;
	void 		*bar_virt;
} sdla_xilinx_t;

// Adapter Data Space.
typedef struct sdla
{
#if defined(__WINDOWS__)
	unsigned char	device_type; /* must be first member of the structure!! */
#endif

	char devname[WAN_DRVNAME_SZ+1];	/* card name */
	//void*	hw;						/* hardware configuration */ /* points to 'hw_struct' */
	sdlahw_t * hw;
	
	wan_device_t	wandev;	/* WAN device data space */
	sdla_fe_t		fe;		/* front end structures */
	
	unsigned open_cnt;			/* number of open interfaces */
	unsigned long state_tick;	/* link state timestamp */
	unsigned intr_mode;			/* Type of Interrupt Mode */
	char in_isr;				/* interrupt-in-service flag */
	char buff_int_mode_unbusy;	/* flag for carrying out dev_tint */  
	char dlci_int_mode_unbusy;	/* flag for carrying out dev_tint */
	char configured;			/* flag for previous configurations */
	
	unsigned short irq_dis_if_send_count; /* Disabling irqs in if_send*/
	unsigned short irq_dis_poll_count;   /* Disabling irqs in poll routine*/
	unsigned short force_enable_irq;
	char TracingEnabled;		/* flag for enabling trace */
	global_stats_t statistics;	/* global statistics */
	unsigned long	mbox_off;	/* -> mailbox offset */
	wan_mbox_t	wan_mbox;	/* mailbox structure */
	unsigned long	rxmb_off;	/* -> receive mailbox */
	wan_mbox_t	wan_rxmb;	/* rx mailbox structure */
	unsigned long	flags_off;	/* -> adapter status flags */
	unsigned long	fe_status_off;	/* FE status structure offset */

#if defined(__WINDOWS__)	
	int (*isr)(struct sdla* card);	/* interrupt service routine */
#else
	void (*isr)(struct sdla* card);	/* interrupt service routine */
#endif

	void (*poll)(struct sdla* card); /* polling routine */
	int (*exec)(struct sdla* card, void* u_cmd, void* u_data);
	/* Used by the listen() system call */		
	/* Wanpipe Socket Interface */
	int   (*func) (struct sk_buff *, struct sock *);
	struct sock *sk;
	
	/* Shutdown function */
	void (*disable_comm) (struct sdla *card);
	
	/* Secondary Port Device: Piggibacking */
	struct sdla *next;
	struct sdla *list;
	
	union
	{
		sdla_chdlc_t	c;
		sdla_xilinx_t	aft;
	} u;
	
	//????????????????/
	//Should be in wandev
	unsigned int 	type;			/* adapter type */
	unsigned char 	adptr_subtype;	/* adapter subtype */

	unsigned char	wan_debugging_state;		/* WAN debugging state */
	int				wan_debug_last_msg;			/* Last WAN debug message */
	int 			(*wan_debugging)(struct sdla*);/* link debugging routine */
	unsigned long	(*get_crc_frames)(struct sdla*);/* get no of CRC frames */
	unsigned long	(*get_abort_frames)(struct sdla*);/* get no of Abort frames */
	unsigned long	(*get_tx_underun_frames)(struct sdla*);/* get no of TX underun frames */
	unsigned short 	timer_int_enabled;
	unsigned char 	backup;
	unsigned char 	comm_enabled;
	unsigned char  	update_comms_stats;
	/*
	compile problem
	sdla_fe_iface_t	fe_iface;
	union {
		sdla_te_iface_t	te_iface;
	} u_fe;
	*/
	unsigned char	debug_running;
	unsigned int	rCount;
	
	/* Wanpipe Socket Interface */
	int   (*get_snmp_data)(struct sdla*, netdevice_t*, void*);
	
	unsigned long intr_perm_off;
	unsigned long intr_type_off;
	
	/* Hardware interface function pointers */
	sdlahw_iface_t	hw_iface;
	
	int (*bind_api_to_svc)(struct sdla*, void *sk_id);
	
#ifdef CONFIG_PRODUCT_WANPIPE_VOIP
	wan_voip_t	wan_voip;
#endif
	unsigned int	spurious;

#if defined(__WINDOWS__)
	void* fdo;		// Device object this extension belongs to
	void* bus_fdo;	// Pointer to funcional dev. extension of virtual bus
	void* pdoData;	// Pointer to physical dev. extension of virtual bus

	sdlahw_card_t sdlahw_card_struct;
//	sdlahw_card_t *p_sdlahw_card_struct;
	sdlahw_t hw_struct; /* actual hardware configuration */

	UNICODE_STRING linkString;	//device name accessible from user mode

	wandev_conf_t	wandev_conf;//configuration of the actual hardware/firmware,
								//partial on startup.

	//
	//NOTE FOR sdla_net_device[] and up[]:
	//at hardware level only up to 31 will be used, at LIP layer it
	//can be more (i.g. DLCIs)
	//
	//logical interface(s). accessable from user space.
	netdevice_t * sdla_net_device[MAX_NUMBER_OF_PROTOCOL_INTERFACES]; 
	//This array will contain "compacted" list of Protocol Interfaces
	//from the 'sdla_net_device[]'. It will save a lot of time in sprotocol.sys
	//tx_complete_indicate_from_below() routine.
	//There will be no NULL pointers from start of the array up to 
	//'number_of_compacted_interfaces'.
	netdevice_t * compact_sdla_net_device_arr[MAX_NUMBER_OF_PROTOCOL_INTERFACES]; 
	int number_of_compacted_interfaces;
	int tx_complete_indicate_index;

	//each 'netdevice_t * sdla_net_device[]' has it's own initialization state.
	unsigned long up[MAX_NUMBER_OF_PROTOCOL_INTERFACES];

	char init_flag;	//initialization completion flag for this card.

	PKINTERRUPT	InterruptObject;// address of interrupt object

	int (*config_firmware)(void *card);

	///////////////////////////////////////
	KDPC	front_end_dpc_obj;
	KDPC	rx_dpc_obj;
	KDPC	tx_dpc_obj;

	xilinx_front_end_dpc_t xilinx_fe_dpc;

	KSPIN_LOCK	spinlock_ISR;	//to synchronize access to data shared with ISR

	KSPIN_LOCK	ioctl_management;//only one Mangement call should be running

	//this is for communicting with driver below
	sdla_wanpipe_interface_t sdla_wanpipe_if_down;
	//this is for communicting with driver above
	sdla_wanpipe_interface_t sdla_wanpipe_if_up;

	//for tx down 
	TX_DATA_STRUCT tx_data;
	int open_handle_counter;

	struct net_device_stats	if_stats;

	//////////////////////////////////////////////////////////////
	//Protocol related members
	KTIMER			ProtocolTimer;
	KDPC			ProtocolTimerDpcObject;
	LARGE_INTEGER	ProtocolTimerDueTime;
	//End of Protocol related members
	//////////////////////////////////////////////////////////////
	void* lip_link;//pointer to 'wplip_link_t'
#endif

#if defined(A104D_CODE)
	//A104 additions
	unsigned int 	adptr_type;		/* adapter type */
	void			*ec_dev_ptr;	//Echo Canceller device
	char			initialization_complete;
#endif

	//array of pointers to TDM API devices
	void *wp_tdmapi_hash[MAX_TDM_API_CHANNELS];
	unsigned int intcount;

} sdla_t;

/****** Public Functions ****************************************************/

void wanpipe_open      (sdla_t* card);			/* wpmain.c */
void wanpipe_close     (sdla_t* card);			/* wpmain.c */

int wpx_init (sdla_t* card, wandev_conf_t* conf);	/* wpx.c */
int wpf_init (sdla_t* card, wandev_conf_t* conf);	/* wpf.c */
int wpp_init (sdla_t* card, wandev_conf_t* conf);	/* wpp.c */
int wpc_init (sdla_t* card, wandev_conf_t* conf); 	/* Cisco HDLC */
int wpbsc_init (sdla_t* card, wandev_conf_t* conf);	/* BSC streaming */
int hdlc_init(sdla_t* card, wandev_conf_t* conf);	/* HDLC support */
int wpft1_init (sdla_t* card, wandev_conf_t* conf);     /* FT1 Config support */
int wp_mprot_init(sdla_t* card, wandev_conf_t* conf);	/* Sync PPP on top of RAW CHDLC */
int wpbit_init (sdla_t* card, wandev_conf_t* conf);	/* Bit Stream driver */
int wpedu_init(sdla_t* card, wandev_conf_t* conf);	/* Educational driver */
int wpss7_init(sdla_t* card, wandev_conf_t* conf);	/* SS7 driver */
int wp_bscstrm_init(sdla_t* card, wandev_conf_t* conf);	/* BiSync Streaming Nasdaq */
int wp_hdlc_fr_init(sdla_t* card, wandev_conf_t* conf);	/* Frame Relay over HDLC RAW Streaming */
int wp_adsl_init(sdla_t* card, wandev_conf_t* conf);	/* ADSL Driver */
int wp_sdlc_init(sdla_t* card, wandev_conf_t* conf);	/* SDLC Driver */
int wp_atm_init(sdla_t* card, wandev_conf_t* conf);	/* ATM Driver */
int wp_pos_init(sdla_t* card, wandev_conf_t* conf);	/* POS Driver */	
int wp_xilinx_init(sdla_t* card, wandev_conf_t* conf);	/* Xilinx Dual Port Hardware */
int wp_aft_te1_init(sdla_t* card, wandev_conf_t* conf);	/* Xilinx Quad Port Hardware */
int wp_aft_analog_init(sdla_t* card, wandev_conf_t* conf);	/* Xilinx Analog Hardware */
int wp_adccp_init(sdla_t* card, wandev_conf_t* conf);	
int wp_aft_firmware_up_init (sdla_t* card, wandev_conf_t* conf);

int wanpipe_globals_util_init(void); /* Initialize All Global Tables */

#if !defined(__WINDOWS__)
extern int wanpipe_queue_tq (struct tq_struct *);
extern int wanpipe_mark_bh (void);
extern int change_dev_flags (netdevice_t *, unsigned); 
extern unsigned long get_ip_address (netdevice_t *dev, int option);
extern void add_gateway(sdla_t *, netdevice_t *);

//FIXME: Take it out
//extern int wan_reply_udp( unsigned char *data, unsigned int mbox_len, int trace_opt);
//extern int wan_udp_pkt_type(sdla_t* card,unsigned char *data);

extern int wanpipe_sdlc_unregister(netdevice_t *dev);
extern int wanpipe_sdlc_register(netdevice_t *dev, void *wp_sdlc_reg);
//ALEX_TODAY extern int check_conf_hw_mismatch(sdla_t *card, unsigned char media);

void adsl_vcivpi_update(sdla_t* card, wandev_conf_t* conf);

#ifdef CONFIG_PRODUCT_WANPIPE_ANNEXG
extern struct wanpipe_lapb_register_struct lapb_protocol;
#endif

int wan_snmp_data(sdla_t* card, netdevice_t* dev, struct ifreq* ifr);

#endif

/*==================================================
* WANPIPE API FUNCTIONS
*/
#if !defined(__WINDOWS__)
extern int 	wan_unbind_api_from_svc(sdla_t *card,  void *chan_ptr, void *sk_id);
extern void 	wan_update_api_state(sdla_t *card, void *chan_ptr);
extern int 	wan_bind_api_to_svc(sdla_t *card,  void *chan_ptr, void *sk_id);
extern int 	wan_api_rx(void *chan_ptr,struct sk_buff *skb);
extern void 	wan_wakeup_api(void *chan_ptr);
extern int 	wan_reg_api(sdla_t *card, void *chan_ptr, 
						void *dev, void (*func)(unsigned long data));
extern int	wan_unreg_api(sdla_t *card, void *chan_ptr);
extern void 	wan_trigger_api_bh(void *chan_ptr);
extern void 	wan_api_bh_end(void *chan_ptr);
extern int 	wan_api_enqueue_skb(void *chan_ptr,struct sk_buff *skb);
extern struct sk_buff * wan_api_dequeue_skb(void *chan_ptr);
#endif

int wan_verify_iovec(struct msghdr *m, void *iov, char *address, int mode);
int wan_memcpy_fromiovec(unsigned char *kdata, void *iov, int len);
int wan_memcpy_toiovec(void *iov, unsigned char *kdata, int len);

#endif	/* __KERNEL__ */
#endif	/* _WANPIPE_H */
