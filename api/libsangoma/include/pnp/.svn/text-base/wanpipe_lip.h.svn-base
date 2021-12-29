/* $Header: /usr/local/cvsroot/wanpipe_windows/include/pnp/wanpipe_lip.h,v 1.1 2006/07/28 20:58:19 sangoma Exp $ */

#ifndef _WANPIPE_LIP_HEADER_
#define _WANPIPE_LIP_HEADER_


#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <net/wanpipe_includes.h>
# include <net/wanpipe_defines.h>
# include <net/wanpipe_debug.h>
# include <net/wanpipe_defines.h>
# include <net/wanpipe_common.h>
# include <net/wanpipe_abstr.h>
# include <net/wanpipe_snmp.h>
# include <net/wanproc.h>
# include <net/wanpipe_cfg.h>
# include <net/if_wanpipe_common.h>
# include <net/wanpipe_iface.h>
# include <net/wanpipe_lip_kernel.h>
# include <net/wanpipe_fr_iface.h>
# include <net/wanpipe_sppp_iface.h>
# if defined(CONFIG_PRODUCT_WANPIPE_LAPB)
#  include <net/wanpipe_lapb_iface.h>
# endif
# if defined(CONFIG_PRODUCT_WANPIPE_XDLC)
#  include <net/wanpipe_xdlc_iface.h>
# endif
#elif defined(__LINUX__)
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanpipe_common.h>
# include <linux/wanpipe_abstr.h>
# include <linux/wanpipe_snmp.h>
# include <linux/wanproc.h>
# include <linux/wanpipe.h>
# include <linux/wanpipe_cfg.h>
# include <linux/if_wanpipe.h>
# include <linux/if_wanpipe_common.h>
# include <linux/wanpipe_fr_iface.h>
# include <linux/wanpipe_sppp_iface.h>
# if defined(CONFIG_PRODUCT_WANPIPE_LAPB)
#  include <linux/wanpipe_lapb_iface.h>
# endif 
# if defined(CONFIG_PRODUCT_WANPIPE_XDLC)
#  include <linux/wanpipe_xdlc_iface.h>
# endif 
# include <linux/wanpipe_lip_kernel.h>
# include <linux/wanpipe_iface.h>

# ifdef WPLIP_TTY_SUPPORT
# include <linux/tty.h>
# include <linux/tty_driver.h>
# include <linux/tty_flip.h>
# endif

#elif defined(__WINDOWS__)

# include <wanpipe_version.h>
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>

# include <wanpipe_abstr.h>
# include <if_wanpipe_common.h>

# include <wanpipe.h>
# include <wanpipe_cfg.h>

# include <wanpipe_sppp_iface.h>
# include <wanpipe_fr_iface.h>

# include <wanpipe_lip_kernel.h>
# include <wanpipe_iface.h>
# include <linux_if_ether.h>

//#define CONFIG_PRODUCT_WANPIPE_CHDLC

//Prototypes for interface between LIP and 'sprotocol' code.
//in virt_adap_enum.c:
int sdla_tx_down(void* dev, void *tx_skb);
int sdla_data_rx_up(void* sdla_net_dev, void *rx_skb);

#endif//#elif defined(__WINDOWS__)


#ifdef WAN_KERNEL

/*
 ***********************************************************************************
 *																			                          *
 * X25HDR.C is the 'C' header file for the Sangoma X.25 code for the S508 adapter. *
 *																				                       *
 ***********************************************************************************
*/

#define LIP_OK  0
#define TCPIP 0 

#define MAX_CALL_REQ_ASYNC_PKT 512

#define NON_D_OPT 3
#define D_OPT 2

#define FLOW_CONTROL_MASK  0x1F

#define ONE_BYTE 1
#define MAX_TOKENS 31

#define MODNAME "wanpipe_lip"

#define MAX_LINK_RX_Q_LEN 10
#define MAX_TAIL_ROOM 16
#define MAX_LIP_LINKS 255

/* BH flags */
enum{
	WPLIP_BH_RUNNING,
	WPLIP_RX_FSM_RUNNING,
	WPLIP_SMP_CONFLICT,
	WPLIP_BH_AWAITING_KICK,
	WPLIP_LINK_TIMER_EXPIRED,
	WPLIP_MORE_LINK_TX,
	WPLIP_LINK_DOWN,
	WPLIP_KICK,
	WPLIP_TTY_BUSY
		
};


/* LIP DEV Critical */
enum {
	WPLIP_TX_WINDOW_CLOSED,
	WPLIP_SEND_AWAITING_KICK,
	WPLIP_TIMER_EXPIRED,
	WPLIP_DEV_DOWN,
	WPLIP_DEV_UNREGISTER,
	WPLIP_RX,
	WPLIP_IOCTL_SMP,
	WPLIP_DEV_SENDING
};

#define MAX_RX_FACIL_CODE 30

#define BH_DEBUG 0

#define SOFT_INIT 0
#define HARD_INIT 1
#define MAX_SOFTIRQ_TIMEOUT 2

/*
 * Physical X25 Link
 * Each Link can support multiple logical
 * channels (svc).
 */
#define MAX_LCN				255
#define MAX_DECODE_BUF_SZ	1000
#define MAX_PROC_NAME		256

#ifndef MAX_PROC_EVENTS
#define MAX_PROC_EVENTS 20
#endif
#define MAX_PROC_EVENT_SIZE X25_CALL_STR_SZ+200+1


#define MAX_TX_BUF 10
#define MAX_RX_Q 32

#define WPLIP_MAGIC_LINK  	0xDAFE1234
#define WPLIP_MAGIC_DEV   	0xDAFE4321
#define WPLIP_MAGIC_DEV_EL   	0xDAFE4444

#define WPLIP_ASSERT_MAGIC(ptr,magic,ret) \
	if ((*(unsigned long*)ptr) != magic) { \
               	DEBUG_EVENT("%s:%d: Error Invalid Magic number in Link dev!\n", \
				__FUNCTION__,__LINE__); \
		return ret ; \
	} 
#define WPLIP_ASSERT_MAGIC_VOID(ptr,magic) \
	if ((*(unsigned long*)ptr) != magic) { \
               	DEBUG_EVENT("%s:%d: Error Invalid Magic number in Link dev!\n", \
				__FUNCTION__,__LINE__); \
		return; \
	}

struct wplip_dev;

WAN_LIST_HEAD(wplip_link_list,wplip_link);

//i.g. per-protocol structure
typedef struct wplip_link
{
	unsigned long			magic;

	unsigned int			dev_cnt;

	WAN_LIST_ENTRY(wplip_link)	list_entry;
	
	/* List of all Logic channel
	 * devices attached to the link 
	 *
	 * Packet direction UP
	 * */
#if !defined(__WINDOWS__)
	WAN_LIST_HEAD(,wplip_dev)	list_head_ifdev;
#else
	//MS compiler needs structure name
	WAN_LIST_HEAD(list_link_up, wplip_dev)	list_head_ifdev;
#endif
	wan_rwlock_t			dev_list_lock; 

	
	/* List of Tx Devices attached
	 * to the Link. 
	 *
	 * Packet direction DOWN
	 *
	 * Eg. Load balancing over multiple
	 *     links */
#if !defined(__WINDOWS__)
	WAN_LIST_HEAD(,wplip_dev_list)	list_head_tx_ifdev;
#else
	//MS compiler needs structure name
	WAN_LIST_HEAD(list_tx_dev_down,wplip_dev_list)	list_head_tx_ifdev;

	void* sdla_card;//pointer to 'card' - for wplip_link_callback_tx_down()

#endif
	unsigned int			tx_dev_cnt;
	wan_rwlock_t			tx_dev_list_lock;
	
	unsigned char			state;
	unsigned char			carrier_state;
	unsigned char			prot_state;

	wan_timer_t				prot_timer;
	unsigned char			protocol;	//this is 'config_id'
	//void					*prot;		//this is pointer to protocol object
	//david - renamed to 'prot_obj'
	void					*prot_obj;	//this is pointer to protocol object

	unsigned long			tq_working;

	/* Internal control information */
	wan_timer_info_t		timer;
	
	wan_rwlock_t			map_lock;	
	void					*api_sk_id;

	wan_spinlock_t			bh_lock;

	unsigned char			name [MAX_PROC_NAME];
#if 0
	struct proc_dir_entry *proc_dir;
	unsigned char *proc_event_log [MAX_PROC_EVENTS];
	atomic_t  proc_event_offset;
#endif
	
	wan_skb_queue_t			tx_queue;
	wan_skb_queue_t			rx_queue;

	wan_tasklet_t			task;

	struct wplip_dev		*cur_tx;

	int						link_num;
	
	atomic_t				refcnt;

	unsigned char 			tty_opt;

#ifdef WPLIP_TTY_SUPPORT
	struct tty_struct 		*tty;
	unsigned int 			tty_minor;
	unsigned int 			tty_open;
	unsigned char 			*tty_buf;
	wan_skb_queue_t 		tty_rx;
	wan_taskq_t				tty_task_queue;
	unsigned char			async_mode;
#endif

} wplip_link_t;


#define CALL_REQUEST_INFO_SZ 512

/*
 * The logic channel per link connection control structure.
 */
//i.g. per DLCI structure.
typedef struct wplip_dev{
	
	wanpipe_common_t		common;
	
	unsigned long			magic;

	WAN_LIST_ENTRY(wplip_dev)	list_entry;

	unsigned short			critical;

	/* Internal control information */
	wan_skb_queue_t			tx_queue;

	/* The link we are part of */
	wplip_link_t			*lip_link;

	void					*sk_id;
	unsigned char			api_state;
#if defined(__LINUX__)
	struct proc_dir_entry		*dent;
#endif
	struct net_device_stats		ifstats;

	unsigned char			used;
	unsigned char			protocol;

	unsigned char			name[MAX_PROC_NAME];

	unsigned char			udp_pkt_data[sizeof(wan_udp_pkt_t)+10];
	unsigned int			udp_pkt_len;

	unsigned long			ipx_net_num;
	
	atomic_t			refcnt;
	pid_t				pid;

} wplip_dev_t;


typedef struct wplip_dev_list
{
	unsigned long			magic;
	netdevice_t			*dev;
	WAN_LIST_ENTRY(wplip_dev_list)	list_entry;
}wplip_dev_list_t;


typedef struct wplip_prot_iface
{
	unsigned int	 init;
	wplip_prot_reg_t reg;
	void*(*prot_link_register)(	void *link_ptr, 
								char *devname, 
								void *cfg, 
								wplip_prot_reg_t *reg);

	int (*prot_link_unregister)(void *prot_ptr);
	
	void*(*prot_chan_register)(	void *if_ptr, 
								void *prot_ptr, 
								char *devname,
								void *cfg,
								unsigned char type);
	
	int (*prot_chan_unregister)(void *chan_ptr);

	int (*open_chan) (void *chan_ptr);
	int (*close_chan)(void *chan_ptr);
	
	int (*tx)     (void *chan_ptr, void *skb, int type);
	int (*ioctl)  (void *chan_ptr, int cmd, void *arg);
	int (*pipemon)(void *chan, 
		       int cmd, 
		       int dlci, 
		       unsigned char* data, 
		       unsigned int *len);

	int (*rx)     (void *prot_ptr, void *rx_pkt);
	int (*timer)  (void *prot_ptr, unsigned int *period);
	int (*bh)     (void *);
	int (*snmp)   (void *, void *);

}wplip_prot_iface_t;

#define MAX_LIP_PROTOCOLS 255

#define WPLIP_PROT_ASSERT(prot,ret) \
	if (prot >= MAX_LIP_PROTOCOLS){ \
		DEBUG_EVENT("%s:%d: Lip Error: Invalid Protocol 0x%X\n",\
			__FUNCTION__,__LINE__,prot);\
		return ret; \
	}

#define WPLIP_PROT_FUNC_ASSERT(prot,func,ret) \
	if (prot->func == NULL){ \
		DEBUG_EVENT("%s:%d: Lip Error: Protocol function not supported\n",\
			__FUNCTION__,__LINE__);\
		return ret; \
	}

#define WPLIP_PROT_EXIST(prot,ret) \
	if (wplip_prot_ops[prot] == NULL){ \
		DEBUG_EVENT("%s:%d: Lip Error: Unsupported/UnCompiled Protocol 0x%X\n",\
			__FUNCTION__,__LINE__,prot);\
		return ret; \
	}


#define wplip_hold(_dev)  atomic_inc(&(_dev)->refcnt)
#define wplip_put(_dev)   atomic_dec(&(_dev)->refcnt)

#define wplip_get_link(_reg)	(_reg)->wplip_link		 
#define wplip_get_lipdev(_dev)	(wplip_dev_t*)wan_netif_priv((_dev))

#define wplip_liplink_magic(_link)  ((_link)->magic == WPLIP_MAGIC_LINK) 
#define wplip_lipdev_magic(_lipdev) ((_lipdev)->magic == WPLIP_MAGIC_DEV)


/* Function Prototypes */
//???
/*
struct wplip_link_list{

	int tail;
	int head;
};
*/

/* wanpipe_lip_iface.c */
extern unsigned char 	wplip_link_num[];
extern wan_rwlock_t 	wplip_link_lock;
extern struct 		wplip_link_list list_head_link;

extern wplip_reg_t		wplip_protocol;

extern int 		wplip_data_rx_up(wplip_dev_t* lip_dev, void *skb);
extern int 		wplip_data_tx_down(wplip_link_t *lip_link, void *skb);
extern int 		wplip_callback_tx_down(void *lip_dev, void *skb);
extern int 		wplip_link_callback_tx_down(void *lip_link, void *skb);

extern int wanpipe_lip_init(void);
extern int wanpipe_lip_exit(void);

/* wanpipe_lip_sub.c */
extern wplip_link_t*	wplip_create_link(char *devname);
extern void 		wplip_remove_link(wplip_link_t *lip_link);
extern void 		wplip_insert_link(wplip_link_t *lip_link);
extern int 		wplip_link_exists(wplip_link_t *lip_link);
extern void 		wplip_free_link(wplip_link_t *lip_link);


#if 1
extern wplip_dev_t*	wplip_create_lipdev(char *dev_name,int);
#else
extern wplip_dev_t*	wplip_create_lipdev(char *dev_name);
#endif
extern void 		wplip_free_lipdev(wplip_dev_t *lip_dev);
extern int		wplip_lipdev_exists(wplip_link_t *lip_link, char *dev_name);
extern void 		wplip_remove_lipdev(wplip_link_t *lip_link, 
					    wplip_dev_t *lip_dev);
extern void 		wplip_insert_lipdev(wplip_link_t *wplip_link, 
					    wplip_dev_t *wplip_dev);

extern unsigned int 	dec_to_uint (unsigned char* str, int len);

/* wanpipe_lip_netdev.c */
extern int wplip_open_dev(netdevice_t *dev);
extern int wplip_stop_dev(netdevice_t *dev);
extern struct net_device_stats * wplip_ifstats (netdevice_t *dev);
extern int wplip_if_send (netskb_t *skb, netdevice_t *dev);
extern int wplip_if_init(netdevice_t *dev);

# ifdef WPLIP_TTY_SUPPORT
/* wanpipe_lip_tty.c */
extern int wplip_reg_tty(wplip_link_t *lip_link, wanif_conf_t *conf);
extern int wplip_unreg_tty(wplip_link_t *lip_link);
extern int wanpipe_tty_trigger_poll(wplip_link_t *lip_link);
extern void wplip_tty_receive(wplip_link_t *lip_link, void *skb);
#endif

/* wanpipe_lip_prot.c */
extern int wplip_init_prot(void);
extern int wplip_free_prot(void);
extern int wplip_reg_link_prot(wplip_link_t *lip_link, wanif_conf_t *conf);
extern int wplip_unreg_link_prot(wplip_link_t *lip_link);
extern int wplip_reg_lipdev_prot(wplip_dev_t *lip_dev, wanif_conf_t *conf);
extern int wplip_unreg_lipdev_prot(wplip_dev_t *lip_dev);
extern int wplip_open_lipdev_prot(wplip_dev_t *lip_dev);
extern int wplip_close_lipdev_prot(wplip_dev_t *lip_dev);
extern int wplip_prot_rx(wplip_link_t *lip_link, netskb_t *skb);

#if 0
extern void wplip_prot_rx_kick(wplip_dev_t *lip_dev);
#endif
extern int wplip_prot_kick(wplip_link_t *lip_link, wplip_dev_t *lip_dev);
extern int wplip_prot_tx(wplip_dev_t *lip_dev, wan_api_tx_hdr_t *api_tx_hdr, netskb_t *skb, int type);
extern int wplip_prot_oob(wplip_dev_t *lip_dev, unsigned char*, int reason);
extern int wplip_prot_ioctl(wplip_dev_t *lip_dev, int cmd, void *arg);
extern int wplip_prot_udp_mgmt_pkt(wplip_dev_t * lip_dev, wan_udp_pkt_t *wan_udp_pkt);
extern int wplip_prot_udp_snmp_pkt(wplip_dev_t * lip_dev, int cmd, struct ifreq* ifr);


extern int wplip_link_prot_change_state(void *wplip_id,
		                        int state,
					unsigned char*,int);
extern int wplip_lipdev_prot_change_state(void *wplip_id,
					  int state,
					  unsigned char*,int);


extern unsigned int wplip_get_ipv4_addr (void *wplip_id, int type);
extern int wplip_set_ipv4_addr (void *wplip_id, 
 		         	unsigned int,
		         	unsigned int,
		         	unsigned int,
		         	unsigned int);
extern void wplip_add_gateway(void *wplip_id);

void wplip_ipxwan_switch_net_num(unsigned char *sendpacket, 
		                unsigned long network_number, 
				unsigned long *orig_dnet,
				unsigned long *orig_snet,
			        unsigned char incoming);


void wplip_ipxwan_restore_net_num(unsigned char *sendpacke, 
				  unsigned long orig_dnet,
				  unsigned long orig_snet);


int wplip_handle_ipxwan(wplip_dev_t *lip_dev, void *skb);


extern int gdbg_flag;

#if !defined(__WINDOWS__)
static __inline int wplip_trigger_bh(wplip_link_t *lip_link)
{
	if (wan_test_bit(WPLIP_LINK_DOWN,&lip_link->tq_working)){
		return -ENETDOWN;
	}

	if (wan_skb_queue_len(&lip_link->rx_queue)){
		WAN_TASKLET_SCHEDULE((&lip_link->task));	
		return 0;
	}
	
	if (wan_test_bit(WPLIP_BH_AWAITING_KICK,&lip_link->tq_working)){
		if (gdbg_flag){
			DEBUG_EVENT("%s: Waiting for kick!\n",
					__FUNCTION__);
		}
		return -EBUSY;
	}

	gdbg_flag=0;
	WAN_TASKLET_SCHEDULE((&lip_link->task));	
	return 0;
}
#endif//#if !defined(__WINDOWS__)


static __inline int wplip_kick_trigger_bh(wplip_link_t *lip_link)
{	
#if !defined(__WINDOWS__)
	wan_clear_bit(WPLIP_BH_AWAITING_KICK,&lip_link->tq_working);
	WAN_TASKLET_SCHEDULE((&lip_link->task));	
#endif//#if !defined(__WINDOWS__)
	return 0;
}


static __inline int wplip_decode_protocol(void *ptr)
{
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	struct sockaddr *sa = (struct sockaddr *)ptr;
	
	switch (sa->sa_family){

	case AF_INET:
		return WPLIP_IP;
	case AF_INET6:
		return WPLIP_IPV6;
	case AF_IPX:
		return WPLIP_IPX;
	default:
		return WPLIP_IP;
	}
		
#elif defined(__LINUX__) 
	struct sk_buff *skb=(struct sk_buff*)ptr;
	
	switch (htons(skb->protocol)){

	case ETH_P_IP:
		return WPLIP_IP;
	case ETH_P_IPV6:
		return WPLIP_IPV6;
	case ETH_P_IPX:
		return WPLIP_IPX;
	}
	return WPLIP_IP;	

#elif defined(__WINDOWS__) 
	struct sk_buff *skb=(struct sk_buff*)ptr;

	DEBUG_LIP("wplip_decode_protocol()\n");
	
	switch (wpabs_htons(skb->protocol)){

	case ETH_P_IP:
		return WPLIP_IP;
	case ETH_P_IPV6:
		return WPLIP_IPV6;
	case ETH_P_IPX:
		return WPLIP_IPX;
	}

	return WPLIP_IP;	
#else
# error ("wplip_decode_protocol: Unknown Protocol!\n");
#endif

}

/*__KERNEL__*/
#endif  

/*_WANPIPE_X25_HEADER_*/
#endif



#define WPLIP_ASSERT(reg,ret)  if (!(reg)) {\
					DEBUG_EVENT("%s:%d Assert Error!\n",	\
							__FUNCTION__,__LINE__);	\
					return ret;	\
			       }

#define WPLIP_ASSERT_VOID(reg) if (!(reg)) {\
					DEBUG_EVENT("%s:%d Assert Error!\n",	\
							__FUNCTION__,__LINE__);	\
					return;	\
			       }

#define FUNC_BEGIN() wpabs_debug_test("%s:%d ---Begin---\n", __FILE__,__LINE__); 
#define FUNC_END() wpabs_debug_test("%s:%d ---End---\n\n",__FILE__,__LINE__); 


#define X25_DEBUG_MEM

#ifdef X25_DEBUG_MEM

 #define X25_SKB_DEC(x)  atomic_sub(x,&x25_skb_alloc)
 #define X25_SKB_INC(x)	 atomic_add(x,&x25_skb_alloc)

 #define ALLOC_SKB(skb,len) { skb = dev_alloc_skb(len);			\
			      if (skb != NULL){ X25_SKB_INC(skb->truesize);}else{ WAN_MEM_ASSERT("X25");} }
 #define KFREE_SKB(skb)     { X25_SKB_DEC(skb->truesize); dev_kfree_skb_any(skb); }


 #define X25_MEM_DEC(x)  atomic_sub(x,&x25_mem_alloc)
 #define X25_MEM_INC(x)	 atomic_add(x,&x25_mem_alloc) 			      
			      
 #define KMALLOC(ptr,len,flag)	{ ptr=kmalloc(len, flag); \
	  			  if (ptr != NULL){ X25_MEM_INC(len);} else {WAN_MEM_ASSERT("X25");}}
 #define KFREE(ptr)		{X25_MEM_DEC(sizeof(*ptr)); kfree(ptr);}
			      
#else
 #define KMALLOC(ptr,len,flag)	ptr=kmalloc(len, flag)
 #define KFREE(ptr)		kfree(ptr)			
			      
 #define ALLOC_SKB(new_skb,len, dsp) new_skb = dev_alloc_skb(len)
 #define KFREE_SKB(skb)	             dev_kfree_skb_any(skb)

 #define X25_SKB_DEC(x) 
 #define X25_SKB_INC(x)	 
 #define X25_MEM_DEC(x) 
 #define X25_MEM_INC(x)	
#endif

#define is_digit(ch) (((ch)>=(unsigned)'0'&&(ch)<=(unsigned)'9')?1:0)
