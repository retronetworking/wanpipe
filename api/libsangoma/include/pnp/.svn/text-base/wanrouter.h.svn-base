/*****************************************************************************
* wanrouter.h	Definitions for the WAN Multiprotocol Router Module.
*		This module provides API and common services for WAN Link
*		Drivers and is completely hardware-independent.
*
* Author: 	Nenad Corbic <ncorbic@sangoma.com>
*		Gideon Hack 	
* Additions:    Arnaldo Melo
*
* Copyright:	(c) 1995-2000 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* May 25, 2001  Alex Feldman	Added T1/E1 support  (TE1).
* Jul 21, 2000  Nenad Corbic	Added WAN_FT1_READY State
* Feb 24, 2000  Nenad Corbic    Added support for socket based x25api
* Jan 28, 2000  Nenad Corbic    Added support for the ASYNC protocol.
* Oct 04, 1999  Nenad Corbic 	Updated for 2.1.0 release
* Jun 02, 1999  Gideon Hack	Added support for the S514 adapter.
* May 23, 1999  Arnaldo Melo    Added local_addr to wanif_conf_t
*                               WAN_DISCONNECTING state added
* Jul 20, 1998	David Fong	Added Inverse ARP options to 'wanif_conf_t'
* Jun 12, 1998	David Fong	Added Cisco HDLC support.
* Dec 16, 1997	Jaspreet Singh	Moved 'enable_IPX' and 'network_number' to
*				'wanif_conf_t'
* Dec 05, 1997	Jaspreet Singh	Added 'pap', 'chap' to 'wanif_conf_t'
*				Added 'authenticator' to 'wan_ppp_conf_t'
* Nov 06, 1997	Jaspreet Singh	Changed Router Driver version to 1.1 from 1.0
* Oct 20, 1997	Jaspreet Singh	Added 'cir','bc','be' and 'mc' to 'wanif_conf_t'
*				Added 'enable_IPX' and 'network_number' to 
*				'wan_device_t'.  Also added defines for
*				UDP PACKET TYPE, Interrupt test, critical values
*				for RACE conditions.
* Oct 05, 1997	Jaspreet Singh	Added 'dlci_num' and 'dlci[100]' to 
*				'wan_fr_conf_t' to configure a list of dlci(s)
*				for a NODE 
* Jul 07, 1997	Jaspreet Singh	Added 'ttl' to 'wandev_conf_t' & 'wan_device_t'
* May 29, 1997 	Jaspreet Singh	Added 'tx_int_enabled' to 'wan_device_t'
* May 21, 1997	Jaspreet Singh	Added 'udp_port' to 'wan_device_t'
* Apr 25, 1997  Farhan Thawar   Added 'udp_port' to 'wandev_conf_t'
* Jan 16, 1997	Gene Kozin	router_devlist made public
* Jan 02, 1997	Gene Kozin	Initial version (based on wanpipe.h).
*****************************************************************************/

#ifndef	_ROUTER_H
#define	_ROUTER_H

#if defined(__LINUX__)
 #include <linux/wanpipe_includes.h>
 #include <linux/wanpipe_version.h>
 #include <linux/wanpipe_defines.h>
 #include <linux/sdla_front_end.h>
 #include <linux/sdla_te1.h>
 #include <linux/sdla_56k.h>
 #ifdef CONFIG_PRODUCT_WANPIPE_VOIP
 # include <linux/sdla_voip.h>
 #endif

 #include <linux/wanpipe_cfg.h>
#elif defined(__WINDOWS__)

 #include <wanpipe_includes.h>
 #include <wanpipe_version.h>

 #include <wanpipe_cfg.h>
 #include <wanpipe_abstr.h>
 #include <wanpipe_common.h> 

// #include <sdla_ec.h>
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__)
# define SYSTEM_TICKS   ticks
# define HZ             hz
#elif defined(__LINUX__)
# define SYSTEM_TICKS   jiffies
#elif defined(__WINDOWS__)

# define jiffies		wpabs_get_systemticks()
# define SYSTEM_TICKS	jiffies

#else
# error "Undefined SYSTEM_TICKS macro!"
#endif


#define	ROUTER_NAME	"wanrouter"	/* in case we ever change it */
#define	ROUTER_IOCTL	'W'		/* for IOCTL calls */
#define	ROUTER_MAGIC	0x524D4157L	/* signature: 'WANR' reversed */

/* IOCTL codes for /proc/router/<device> entries (up to 255) */
enum router_ioctls
{
	ROUTER_SETUP	= ROUTER_IOCTL<<8,	/* configure device */
	ROUTER_DOWN,				/* shut down device */
	ROUTER_STAT,				/* get device status */
	ROUTER_IFNEW,				/* add interface */
	ROUTER_IFDEL,				/* delete interface */
	ROUTER_IFSTAT,				/* get interface status */
	ROUTER_VER,				/* get router version */
	ROUTER_DEBUGGING,			/* get router version */
	ROUTER_DEBUG_READ,			/* get router version */

	ROUTER_IFNEW_LAPB,			/* add new lapb interface */
	ROUTER_IFDEL_LAPB,			/* delete a lapb interface */
	ROUTER_IFNEW_X25,			/* add new x25 interface */
	ROUTER_IFDEL_X25,			/* delete a x25 interface */
	ROUTER_IFNEW_DSP,			/* add new dsp interface */
	ROUTER_IFDEL_DSP,			/* delete a dsp interface */

	ROUTER_USER	= (ROUTER_IOCTL<<8)+16,	/* driver-specific calls */
	ROUTER_USER_MAX	= (ROUTER_IOCTL<<8)+31
};

/* identifiers for displaying proc file data for dual port adapters */
#define PROC_DATA_PORT_0 0x8000	/* the data is for port 0 */
#define PROC_DATA_PORT_1 0x8001	/* the data is for port 1 */

/* NLPID for packet encapsulation (ISO/IEC TR 9577) */
#define	NLPID_IP	0xCC	/* Internet Protocol Datagram */
#define NLPID_CISCO_IP	0x00	/* Internet Protocol Datagram - CISCO only */
#define	NLPID_SNAP	0x80	/* IEEE Subnetwork Access Protocol */
#define	NLPID_CLNP	0x81	/* ISO/IEC 8473 */
#define	NLPID_ESIS	0x82	/* ISO/IEC 9542 */
#define	NLPID_ISIS	0x83	/* ISO/IEC ISIS */
#define	NLPID_Q933	0x08	/* CCITT Q.933 */



/****** Data Types **********************************************************/

/*----------------------------------------------------------------------------
 * WAN Link Status Info (for ROUTER_STAT IOCTL).
 */
typedef struct wandev_stat
{
	unsigned state;		/* link state */
	unsigned ndev;		/* number of configured interfaces */

	/* link/interface configuration */
	unsigned connection;	/* permanent/switched/on-demand */
	unsigned media_type;	/* Frame relay/PPP/X.25/SDLC, etc. */
	unsigned mtu;		/* max. transmit unit for this device */

	/* physical level statistics */
	unsigned modem_status;	/* modem status */
	unsigned rx_frames;	/* received frames count */
	unsigned rx_overruns;	/* receiver overrun error count */
	unsigned rx_crc_err;	/* receive CRC error count */
	unsigned rx_aborts;	/* received aborted frames count */
	unsigned rx_bad_length;	/* unexpetedly long/short frames count */
	unsigned rx_dropped;	/* frames discarded at device level */
	unsigned tx_frames;	/* transmitted frames count */
	unsigned tx_underruns;	/* aborted transmissions (underruns) count */
	unsigned tx_timeouts;	/* transmission timeouts */
	unsigned tx_rejects;	/* other transmit errors */

	/* media level statistics */
	unsigned rx_bad_format;	/* frames with invalid format */
	unsigned rx_bad_addr;	/* frames with invalid media address */
	unsigned tx_retries;	/* frames re-transmitted */
	unsigned reserved[16];	/* reserved for future use */
} wandev_stat_t;


/* Front-End status */
enum fe_status {
	FE_UNITIALIZED = 0x00,
	FE_DISCONNECTED,
	FE_CONNECTED
};

/* 'modem_status' masks */
#define	WAN_MODEM_CTS	0x0001	/* CTS line active */
#define	WAN_MODEM_DCD	0x0002	/* DCD line active */
#define	WAN_MODEM_DTR	0x0010	/* DTR line active */
#define	WAN_MODEM_RTS	0x0020	/* RTS line active */

/* modem status changes */
#define WAN_DCD_HIGH			0x08
#define WAN_CTS_HIGH			0x20


#if defined(__KERNEL__)

#if !defined(__WINDOWS__)

#include <linux/version.h>

#ifndef KERNEL_VERSION
  #define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
 #define LINUX_2_4
 #define netdevice_t struct net_device
 #include <linux/spinlock.h>       /* Support for SMP Locking */

 #define ADMIN_CHECK()  {if (!capable(CAP_SYS_ADMIN)) {\
                             DEBUG_EVENT("wanpipe: ADMIN_CHECK: Failed Cap=0x%X Fsuid=0x%X Euid=0x%X\n", \
				 current->cap_effective,current->fsuid,current->euid);\
	                     return -EPERM; \
 		             }\
                        }

 #define NET_ADMIN_CHECK()  {if (!capable(CAP_NET_ADMIN)){\
	                          DEBUG_EVENT("wanpipe: NET_ADMIN_CHECK: Failed Cap=0x%X Fsuid=0x%X Euid=0x%X\n", \
					 current->cap_effective,current->fsuid,current->euid);\
	                          return -EPERM; \
                                 }\
                            }

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,1,0)
 #define LINUX_2_1
 #define netdevice_t struct device
 #include <asm/spinlock.h>       /* Support for SMP Locking */

 #define ADMIN_CHECK()  {if (!capable(CAP_SYS_ADMIN)) return -EPERM;}
 #define NET_ADMIN_CHECK()  {if (!capable(CAP_NET_ADMIN)) return -EPERM;}

#else
 #define LINUX_2_0
 #define netdevice_t struct device
 #define spinlock_t int

 #define ADMIN_CHECK()
 #define NET_ADMIN_CHECK()  
#endif



#define IS_FUNC_CALL(str, func)					\
	(IS_PROTOCOL_FUNC(str) && str.func) ? 1 : 0


/****** Kernel Interface ****************************************************/

#include <linux/fs.h>		/* support for device drivers */
#include <linux/proc_fs.h>	/* proc filesystem pragmatics */
#include <linux/inet.h>		/* in_aton(), in_ntoa() prototypes */
#include <linux/netdevice.h>	/* support for network drivers */

#ifndef LINUX_2_4
typedef int (get_info_t)(char *, char **, off_t, int, int);
#endif

#ifdef LINUX_2_0
typedef int (write_proc_t)(char *, char **, off_t, int, int);
#endif

#endif //#if !defined(__WINDOWS__)

#define REG_PROTOCOL_FUNC(str)		wan_set_bit(0, (unsigned long*)&str.init)
#define UNREG_PROTOCOL_FUNC(str)	wan_clear_bit(0, (unsigned long*)&str.init)
#define IS_PROTOCOL_FUNC(str)		wan_test_bit(0, (unsigned long*)&str.init)

#define IS_FUNC_CALL(str, func)					\
	(IS_PROTOCOL_FUNC(str) && str.func) ? 1 : 0

/*----------------------------------------------------------------------------
 * WAN device data space.
 */
typedef struct wan_device
{
	unsigned magic;			/* magic number */
	char* name;				/* -> WAN device name (ASCIIZ) */
	void* drv_private;		/* -> driver private data */
	unsigned config_id;		/* Configuration ID */
					/****** hardware configuration ******/
	unsigned ioport;		/* adapter I/O port base #1 */
	char S514_cpu_no[1];	/* PCI CPU Number */
	unsigned char S514_slot_no;	/* PCI Slot Number */
#if 0
	//ALEX_TODAY maddr not really used
	unsigned long maddr;		/* dual-port memory address */
	unsigned msize;			/* dual-port memory size */
#endif
	int irq;			/* interrupt request level */
	int dma;			/* DMA request level */
	unsigned bps;			/* data transfer rate */
	unsigned mtu;			/* max physical transmit unit size */
	unsigned udp_port;      /* UDP port for management */
    unsigned char ttl;		/* Time To Live for UDP security */
	unsigned enable_tx_int; 	/* Transmit Interrupt enabled or not */
	char electrical_interface;	/* RS-232/V.35, etc. */
	char clocking;			/* external/internal */

	unsigned char 		comm_port;

	char line_coding;		/* NRZ/NRZI/FM0/FM1, etc. */
	char station;			/* DTE/DCE, primary/secondary, etc. */
	char connection;		/* permanent/switched/on-demand */
	char signalling;		/* Signalling RS232 or V35 */
	char read_mode;			/* read mode: Polling or interrupt */
	char new_if_cnt;        /* Number of interfaces per wanpipe */ 
	char del_if_cnt;		/* Number of times del_if() gets called */
	unsigned char piggyback;/* Piggibacking a port */

#if 0
	//ALEX_TODAY hw_opt[0] -> card->type
	unsigned hw_opt[4];		/* other hardware options */
#endif
					/****** status and statistics *******/
	char state;			/* device state */
	char api_status;		/* device api status */
#if defined(LINUX_2_1) || defined(LINUX_2_4)
	struct net_device_stats stats; 	/* interface statistics */

#elif !defined(__WINDOWS__)
	struct enet_statistics stats;	/* interface statistics */

	/****** maintained by the router ****/
	struct wan_device* next;	/* -> next device */
	netdevice_t* dev;		/* list of network interfaces */
	unsigned ndev;			/* number of interfaces */

	struct proc_dir_entry *dent;	/* proc filesystem entry */
	struct proc_dir_entry *link;	/* proc filesystem entry per link */

	// Proc fs functions
	int (*get_config_info) 		(void*, char*, int, int, int, int*);
	int (*get_status_info) 		(void*, char*, int, int, int, int*);
	
	get_info_t*	get_dev_config_info;
	get_info_t*	get_if_info;
	write_proc_t*	set_dev_config;
	write_proc_t*	set_if_info;

#else //if __WINDOWS__

	/****** maintained by the router ****/
	unsigned reserved[16];		/* reserved for future use */
	unsigned long critical;		/* critical section flag */
	//spinlock_t lock;            /* Support for SMP Locking */
	KIRQL	lock;

	struct net_device_stats stats; 	/* interface statistics */

					/****** device management methods ***/
	int (*setup) (struct wan_device *wandev, wandev_conf_t *conf);
	int (*shutdown) (struct wan_device *wandev, wandev_conf_t* conf);
//	int (*update) (struct wan_device *wandev);
	int (*ioctl) (struct wan_device *wandev, unsigned cmd,
		unsigned long arg);
	int (*new_if) (struct wan_device *wandev, netdevice_t *dev,
		wanif_conf_t *conf);
	int (*del_if) (struct wan_device *wandev, netdevice_t *dev);

#endif

//	sdla_te_cfg_t		te_cfg;		/* TE1 hardware configuration */
	sdla_fe_cfg_t		fe_cfg;	/* Front end configurations */

	unsigned long 		te_alarm;	/* TE1 alarm */
	unsigned long 		cur_te_alarm;	/* TE1 alarm (proc fs) */
	//pmc_pmon_t			te_pmon;	/* TE PMON counters */
	unsigned char 		te_lb_cmd;	/* Received loopback command */	
	unsigned long 		te_lb_time;	/* Time when loopback command received */	

	unsigned char 		te_lb_tx_cmd;	/* Received loopback command */	
	unsigned long 		te_lb_tx_cnt;	/* Time when loopback command received */
	unsigned char 		te_critical;	/* T1/E1 critical flag */	

	unsigned char		te_timer_cmd;	// time command

//	unsigned long		k56_alarm;	// 56K alarms
//	unsigned char 		front_end_status;
//	unsigned char 		RLOS_56k;
	unsigned char 		RR8_reg_56k;
	unsigned char 		RRA_reg_56k;
	unsigned char 		RRC_reg_56k;
//	unsigned char 		prev_RRC_reg_56k;
//	unsigned char 		delta_RRC_reg_56k;
//	WRITE_FRONT_END_REG_T* 	write_front_end_reg;
//	READ_FRONT_END_REG_T* 	read_front_end_reg;

//	int 	(*get_info)(void*, struct seq_file* m, int *);
	void	(*fe_enable_timer) (void* card_id);
//	void (*te_enable_timer) (void* card_id);

	void (*te_report_rbsbits) (void* card_id, int channel, unsigned char rbsbits);
	void (*te_report_alarms) (void* card_id, unsigned long alarams);
	void (*te_link_state)  (void* card_id);
	int (*te_signaling_config) (void* card_id, unsigned long);
	int (*te_disable_signaling) (void* card_id);
	int (*te_read_signaling_config) (void* card_id);
	int (*report_dtmf) (void* card_id, int, unsigned char);
	void	(*ec_enable_timer) (void* card_id);

	struct {
		void	(*rbsbits) (void* card_id, int channel, unsigned char rbsbits);
		void	(*alarms) (void* card_id, unsigned long alarams);
		void	(*dtmf) (void* card_id, wan_event_t*);	
		void	(*hook) (void* card_id, wan_event_t*);	
		void	(*ringtrip) (void* card_id, wan_event_t*);	
		void	(*ringdetect) (void* card_id, wan_event_t*);	
	} event_callback;

	unsigned char 		ignore_front_end_status;
	unsigned char		line_idle;
	unsigned short		card_type;

	atomic_t		if_cnt;
	atomic_t		if_up_cnt;

#if !defined(__WINDOWS__)

	wan_sdlc_conf_t		sdlc_cfg;
	wan_bscstrm_conf_t  	bscstrm_cfg;
	int (*debugging) (struct wan_device *wandev);
	unsigned char 		comm_port;

	spinlock_t		get_map_lock;
	int (*get_map)(struct wan_device*,netdevice_t*,char*, int, int, int, int*);

	int (*bind_annexg) (netdevice_t *dev, netdevice_t *adev);
	netdevice_t *(*un_bind_annexg) (struct wan_device *wandev,netdevice_t *adev);
	void (*get_active_inactive)(struct wan_device*,netdevice_t*,void*);

	int (*debug_read) (void*, void*);
#endif

	sdla_fe_iface_t	fe_iface;
	sdla_fe_notify_iface_t	fe_notify_iface;

	//void			*ec;
	void			*ec_dev;
	unsigned long		ec_map;
//	wan_ec_iface_t		ec_iface;
	unsigned long		ec_enable_map;
	unsigned long		ec_intmask;
	int			(*ec_enable)(void *pcard, int, int);

	unsigned char	(*write_ec)(void*, unsigned short, unsigned char);
	unsigned char	(*read_ec)(void*, unsigned short);
	int		(*hwec_reset)(void* card_id, int);
	int		(*hwec_enable)(void* card_id, int, int);
	//int				(*hwec_action)(void* card_id, int, int);

} wan_device_t;


#if !defined(__WINDOWS__)

struct wanpipe_fw_register_struct
{
	unsigned char init;
	
	int (*bind_api_to_svc)(char *devname, void *sk_id);
	int (*bind_listen_to_link)(char *devname, void *sk_id, unsigned short protocol);
	int (*unbind_listen_from_link)(void *sk_id,unsigned short protocol);
};


/* Public functions available for device drivers */
extern int register_wan_device(wan_device_t *wandev);
extern int unregister_wan_device(char *name);
unsigned short wanrouter_type_trans(struct sk_buff *skb, netdevice_t *dev);
int wanrouter_encapsulate(struct sk_buff *skb, netdevice_t *dev,unsigned short type);

/* Proc interface functions. These must not be called by the drivers! */
extern int wanrouter_proc_init(void);
extern void wanrouter_proc_cleanup(void);
extern int wanrouter_proc_add(wan_device_t *wandev);
extern int wanrouter_proc_delete(wan_device_t *wandev);
extern int wanrouter_proc_add_protocol(wan_device_t* wandev);
extern int wanrouter_proc_delete_protocol(wan_device_t* wandev);
extern int wanrouter_proc_add_interface(wan_device_t*,struct proc_dir_entry**,
					char*,void*);
extern int wanrouter_proc_delete_interface(wan_device_t*, char*);
extern int wanrouter_ioctl( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

extern void *sdla_get_hw_probe(void);
extern int wanrouter_proc_usage_check(void);
extern int wan_run_wanrouter(char* hwdevname, char *devname, char *action);

/* Public Data */
extern wan_device_t *router_devlist;	/* list of registered devices */

extern int register_wanpipe_fw_protocol (struct wanpipe_fw_register_struct *wp_fw_reg);
extern void unregister_wanpipe_fw_protocol (void);
extern void wan_skb_destructor (struct sk_buff *skb);

#endif	/* !(__WINDOWS__) */

void *wanpipe_ec_register(void*, int);
int wanpipe_ec_unregister(void*,void*);
int wanpipe_ec_isr(void*,void*);
int wanpipe_ec_poll(void*,void*);
int wanpipe_ec_event_ctrl(void*,void*,wan_event_ctrl_t*);

#endif	/* __KERNEL__ */
#endif	/* _ROUTER_H */
