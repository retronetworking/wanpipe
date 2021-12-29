/*****************************************************************************
* sdla_temp.c	WANPIPE(tm) Multiprotocol WAN Link Driver.
* 		
* 		Template Driver
* 		
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 1995-2003 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jan 07, 2002	Nenad Corbic	Initial version.
*****************************************************************************/

#include <linux/wanpipe_includes.h>
#include <linux/wanpipe_defines.h>
#include <linux/wanpipe.h>	
#include <linux/wanproc.h>

#error   "-> INCLUDE PROTOCOL SPECIFIC HEADER HERE"
#include <linux/sdla_template.h>

#include <linux/if_wanpipe_common.h>    /* Socket Driver common area */
#include <linux/if_wanpipe.h>		


/****** Defines & Macros ****************************************************/

/* Private critical flags */
enum { 
	POLL_CRIT = PRIV_CRIT, 
	TX_INTR,
	TASK_POLL
};

#define MAX_IP_ERRORS	10

#define PORT(x)   (x == 0 ? "PRIMARY" : "SECONDARY" )
#define MAX_BH_BUFF	10

/******Data Structures*****************************************************/

/* This structure is placed in the private data area of the device structure.
 * The card structure used to occupy the private area but now the following 
 * structure will incorporate the card structure along with Protocol specific data
 */

typedef struct private_area
{
	wanpipe_common_t common;
	sdla_t		*card;
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
	unsigned long 	tick_counter;		/* For 5s timeout counter */
	unsigned long 	router_up_time;
        u32             IP_address;		/* IP addressing */
        u32             IP_netmask;
	u32		ip_local;
	u32		ip_remote;
	u32 		ip_local_tmp;
	u32		ip_remote_tmp;
	u8		ip_error;
	u8		config_frmw;
	u8 		config_frmw_timeout;
	unsigned char  	mc;			/* Mulitcast support on/off */
	unsigned char 	udp_pkt_src;		/* udp packet processing */
	unsigned short 	timer_int_enabled;

	bh_data_t 	*bh_head;	  	  /* Circular buffer for bh */
	unsigned long  	tq_working;
	volatile int  	bh_write;
	volatile int  	bh_read;
	atomic_t  	bh_buff_used;
	
	unsigned char 	interface_down;

	/* Polling task queue. Each interface
         * has its own task queue, which is used
         * to defer events from the interrupt */
	struct tq_struct 	poll_task;
	struct timer_list 	poll_delay_timer;

	u8 		gateway;
	u8 		true_if_encoding;
	
	//FIXME: add driver stats as per frame relay!

	/* Entry in proc fs per each interface */
	struct proc_dir_entry	*dent;

	unsigned char 	udp_pkt_data[sizeof(wan_udp_pkt_t)+10];
	atomic_t 	udp_pkt_len;

	char 		if_name[WAN_IFNAME_SZ+1];

}private_area_t;

/* Route Status options */
#define NO_ROUTE	0x00
#define ADD_ROUTE	0x01
#define ROUTE_ADDED	0x02
#define REMOVE_ROUTE	0x03


/* variable for keeping track of enabling/disabling FT1 monitor status */
static int rCount;
static int Intr_test_counter;

extern void disable_irq(unsigned int);
extern void enable_irq(unsigned int);

/**SECTOIN**************************************************
 *
 * Function Prototypes 
 *
 ***********************************************************/

/* WAN link driver entry points. These are called by the WAN router module. */
static int 	update (wan_device_t* wandev);
static int 	new_if (wan_device_t* wandev, struct net_device* dev, wanif_conf_t* conf);
static int 	del_if(wan_device_t *wandev, struct net_device *dev);

/* Network device interface */
static int 	if_init   (struct net_device* dev);
static int 	if_open   (struct net_device* dev);
static int 	if_close  (struct net_device* dev);
static int 	if_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);

static struct net_device_stats* if_stats (struct net_device* dev);
  
static int 	if_send (struct sk_buff* skb, struct net_device* dev);

/* Firmware interface functions */
static int 	frmw_configure 	(sdla_t* card, void* data);
static int 	frmw_comm_enable 	(sdla_t* card);
static int 	frmw_read_version 	(sdla_t* card, char* str);
static int 	frmw_set_intr_mode 	(sdla_t* card, unsigned mode);
static int 	set_adapter_config 	(sdla_t* card);
static int 	frmw_send (sdla_t* card, void* data, unsigned len, unsigned char tx_bits);
static int 	frmw_read_comm_err_stats (sdla_t* card);
static int 	frmw_read_op_stats (sdla_t* card);
static int 	frmw_error (sdla_t *card, int err, wan_mbox_t *mb);
static void 	handle_front_end_state(sdla_t *card);
static void 	enable_timer(void* card_id);
static int 	disable_comm_shutdown (sdla_t *card);
static int	init_interrupt_status_ptrs(sdla_t *card);
#ifdef LINUX_2_4
static void 	if_tx_timeout (struct net_device *dev);
#endif

/* Miscellaneous Functions */
static int 	set_frmw_config (sdla_t* card);
static void 	init_tx_rx_buff( sdla_t* card);
static int 	process_exception(sdla_t *card);
static int 	process_global_exception(sdla_t *card);
static int 	update_comms_stats(sdla_t* card,private_area_t*);
static void 	port_set_state (sdla_t *card, int);

static int 	config_frmw (sdla_t *card);
static void 	disable_comm (sdla_t *card);

static void 	trigger_poll (struct net_device *);
static void 	frmw_poll (struct net_device *);
static void 	frmw_poll_delay (unsigned long dev_ptr);
static int 	frmw_read_baud_calibration (sdla_t *card);

/* Interrupt handlers */
static void 	wpc_isr (sdla_t* card);
static void 	rx_intr (sdla_t* card);
static void 	timer_intr(sdla_t *);

/* Bottom half handlers */
static void 	wp_bh (struct net_device *);
static int 	wp_bh_cleanup (struct net_device *);
static int 	bh_enqueue (struct net_device *, struct sk_buff *);
static void 	wakeup_sk_bh (struct net_device *dev);

/* Miscellaneous functions */
static int 	chk_bcast_mcast_addr(sdla_t* card, struct net_device* dev,
				struct sk_buff *skb);
static int 	reply_udp( unsigned char *data, unsigned int mbox_len );
static int 	intr_test( sdla_t* card);
static int 	udp_pkt_type( struct sk_buff *skb , sdla_t* card);
static int 	store_udp_mgmt_pkt(char udp_pkt_src, sdla_t* card,
                                struct sk_buff *skb, struct net_device* dev,
                                private_area_t*);
static int 	process_udp_mgmt_pkt(sdla_t* card, struct net_device* dev,  
				private_area_t*,
				int local_dev);

static void 	s508_lock (sdla_t *card, unsigned long *smp_flags);
static void 	s508_unlock (sdla_t *card, unsigned long *smp_flags);


#ifdef WANPIPE_ENABLE_PROC_FILE_HOOKS
# warning "Enabling Proc File System Hooks"
static int 	get_config_info(void* priv, char* buf, int cnt, int, int, int*);
static int 	get_status_info(void* priv, char* buf, int cnt, int, int, int*);
# ifdef LINUX_2_4
static int 	get_dev_config_info(char* buf, char** start, off_t offs, int len);
static int 	get_if_info(char* buf, char** start, off_t offs, int len);
# else
static int 	get_dev_config_info(char* buf, char** start, off_t offs, int len, int dummy);
static int 	get_if_info(char* buf, char** start, off_t offs, int len, int dummy);
# endif
static int 	set_dev_config(struct file*, const char*, unsigned long, void *);
static int 	set_if_info(struct file*, const char*, unsigned long, void *);
#else
# warning "Disabling Proc File System Hooks"
#endif

#ifdef WANPIPE_ENABLE_DYNAMIC_IP_ADDRESSING
# warning "Enabling dynamic ip addressing support"
static int 	configure_ip (sdla_t* card);
static int 	unconfigure_ip (sdla_t* card);
static void 	process_route(sdla_t *card);
#else
# warning "Disabling dynamic ip addressing support"
# define 	configure_ip(x)
# define 	unconfigure_ip(x)
# define 	process_route(x)
#endif

/* TE1 Control registers  */
static WRITE_FRONT_END_REG_T write_front_end_reg;
static READ_FRONT_END_REG_T  read_front_end_reg;


/**SECTION********************************************************* 
 * 
 * Public Functions 
 *
 ******************************************************************/


/*============================================================================
 * wpc_init - Cisco HDLC protocol initialization routine.
 *
 * @card:	Wanpipe card pointer
 * @conf:	User hardware/firmware/general protocol configuration
 *              pointer. 
 * 
 * This routine is called by the main WANPIPE module 
 * during setup: ROUTER_SETUP ioctl().  
 *
 * At this point adapter is completely initialized 
 * and firmware is running.
 *  o read firmware version (to make sure it's alive)
 *  o configure adapter
 *  o initialize protocol-specific fields of the adapter data space.
 *
 * Return:	0	o.k.
 *		< 0	failure.
 */

#error "->CHANGE THE INITIALIZATION NAME TO SUTE YOUR PROTOCOL"

int wp_frmw_init (sdla_t* card, wandev_conf_t* conf)
{
	unsigned char port_num;
	int err;
	unsigned long max_permitted_baud = 0;
	SHARED_MEMORY_INFO_STRUCT *flags;

	union{
		char str[80];
	} u;
	volatile wan_mbox_t* mb;
	unsigned long timeout;

	/* Verify configuration ID */

	if (conf->config_id != WANCONFIG_FRMW) {
		DEBUG_EVENT( "%s: invalid configuration ID %u!\n",
				  card->devname, conf->config_id);
		return -EINVAL;
	}

	/* Initialize the card mailbox and obtain the mailbox
	 * pointer */
	init_card_mailbox(card);
	mb = get_card_mailbox(card);

	if (!card->configured){

		/* The board will place an 'I' in the return 
		 * code to indicate that it is ready to accept 
		 * commands.  We expect this to be completed 
		 * in less than 1 second. */

		timeout = jiffies;
		while (mb->wan_return_code != 'I')	/* Wait 1s for board to initialize */
			if ((jiffies - timeout) > 1*HZ) break;

		if (mb->wan_return_code != 'I') {
			DEBUG_EVENT(
				"%s: Initialization not completed by adapter\n",
				card->devname);
			DEBUG_EVENT( "Please contact Sangoma representative.\n");
			return -EIO;
		}
	}

	/* Obtain hardware configuration parameters */
	card->wandev.clocking 			= conf->clocking;
	card->wandev.ignore_front_end_status 	= conf->ignore_front_end_status;
	card->wandev.ttl 			= conf->ttl;
	card->wandev.electrical_interface 			= conf->electrical_interface; 
	card->wandev.comm_port 			= conf->comm_port;
	card->wandev.udp_port   		= conf->udp_port;
	card->wandev.new_if_cnt 		= 0;
	atomic_set(&card->wandev.if_cnt,0);
	
#error "USING U.C  CHANGE TO YOUR PROTOCOL"
	port_num = card->wandev.comm_port;

	err = (card->hw_iface.check_mismatch) ? 
			card->hw_iface.check_mismatch(card->hw,conf->te_cfg.media) : -EINVAL;
	if (err){
		return err;
	}
	
	/* TE1 Make special hardware initialization for T1/E1 board */
	if (IS_TE1(conf->te_cfg)){
		
		memcpy(&card->wandev.te_cfg, &conf->te_cfg, sizeof(sdla_te_cfg_t));
		card->fe.name		= card->devname;
		card->fe.card		= card;
		card->fe.write_fe_reg	= write_front_end_reg;
		card->fe.read_fe_reg	= read_front_end_reg;

		card->wandev.write_front_end_reg = write_front_end_reg;
		card->wandev.read_front_end_reg = read_front_end_reg;
		card->wandev.fe_enable_timer = enable_timer;
		conf->electrical_interface = 
			(card->wandev.te_cfg.media == WAN_MEDIA_T1) ? WANOPT_V35 : WANOPT_RS232;

		if (card->wandev.comm_port == WANOPT_PRI){
			conf->clocking = WANOPT_EXTERNAL;
		}

	}else if (IS_56K(conf->te_cfg)){

		memcpy(&card->wandev.te_cfg, &conf->te_cfg, sizeof(sdla_te_cfg_t));
		card->fe.name		= card->devname;
		card->fe.card		= card;
		card->fe.write_fe_reg	= write_front_end_reg;
		card->fe.read_fe_reg	= read_front_end_reg;

		card->wandev.write_front_end_reg = write_front_end_reg;
		card->wandev.read_front_end_reg = read_front_end_reg;
		
		if (card->wandev.comm_port == WANOPT_PRI){
			conf->clocking = WANOPT_EXTERNAL;
		}

	}else{
		card->wandev.front_end_status = FE_CONNECTED;
	}

	if (card->wandev.ignore_front_end_status == WANOPT_NO){
		DEBUG_EVENT( 
		  "%s: Enabling front end link monitor\n",
				card->devname);
	}else{
		DEBUG_EVENT( 
		"%s: Disabling front end link monitor\n",
				card->devname);
	}


	/* Read firmware version.  Note that when adapter initializes, it
	 * clears the mailbox, so it may appear that the first command was
	 * executed successfully when in fact it was merely erased. To work
	 * around this, we execute the first command twice.
	 */

	if (frmw_read_version(card, u.str))
		return -EIO;

#error "CHANGE THE NAME FROM TEMPLATE TO PROTOCOL RUNNING"

	DEBUG_EVENT( "%s: Running Template firmware v%s\n",
		card->devname, u.str); 

	if (set_adapter_config(card)) {
		return -EIO;
	}

	card->isr			= &wpc_isr;
	card->wandev.update		= &update;
 	card->wandev.new_if		= &new_if;
	card->wandev.del_if		= &del_if;
	card->disable_comm		= &disable_comm;

#ifdef WANPIPE_ENABLE_PROC_FILE_HOOKS
	/* Proc fs functions hooks */
	card->wandev.get_config_info 	= &get_config_info;
	card->wandev.get_status_info 	= &get_status_info;
	card->wandev.get_dev_config_info= &get_dev_config_info;
	card->wandev.get_if_info     	= &get_if_info;
	card->wandev.set_dev_config    	= &set_dev_config;
	card->wandev.set_if_info     	= &set_if_info;
#endif


	/* Setup Port Bps */
	if(card->wandev.clocking) {
		card->wandev.bps = conf->bps;
	}else{
        	card->wandev.bps = 0;
  	}

#error "->USE YOUR OWN MIN AND MAX VALUES FOR PRI/SEC MTU"
	
	/* Setup the Port MTU */
	if((card->wandev.comm_port == WANOPT_PRI)) {
		/* For Primary Port 0 */
		card->wandev.mtu =
			(conf->mtu >= MIN_WP_PRI_MTU) ? 
			 wp_min(conf->mtu, MAX_WP_PRI_MTU) : DEFAULT_WP_PRI_MTU;
		
	}else{
		/* For Secondary Port 1 */
		card->wandev.mtu =
			(conf->mtu >= MIN_WP_SEC_MTU) ?
			wp_min(conf->mtu, MAX_WP_SEC_MTU) :
			DEFAULT_WP_SEC_MTU;
	}

	if ((err=init_interrupt_status_ptrs(card)) != COMMAND_OK){
		return err;
	}

	flags = get_card_flags(card);
	
	if (!card->wandev.piggyback){	
		int err;

		/* Perform interrupt testing */
		err = intr_test(card);

		if(err || (Intr_test_counter < MAX_INTR_TEST_COUNTER)) { 
			DEBUG_EVENT( "%s: Interrupt test failed (%i)\n",
					card->devname, Intr_test_counter);
			DEBUG_EVENT( "%s: Please choose another interrupt\n",
					card->devname);
			return -EIO;
		}
		
		DEBUG_EVENT( "%s: Interrupt test passed (%i)\n", 
				card->devname, Intr_test_counter);
		card->configured = 1;
	}

	if (frmw_set_intr_mode(card, APP_INT_ON_TIMER)){
		DEBUG_EVENT( "%s: Failed to set interrupt triggers!\n",
			card->devname);
		return -EIO;	
       	}
	
	/* Mask the Timer interrupt */
	flags->interrupt_info_struct.interrupt_permission &= 
		~APP_INT_ON_TIMER;

	/* If we are using backup mode, this flag will
	 * indicate not to look for IP addresses in config_frmw()*/
	card->backup = conf->backup;

	/* Set protocol link state to disconnected,
	 * After seting the state to DISCONNECTED this
	 * function must return 0 i.e. success */
	card->wandev.state = WAN_DISCONNECTED;

	DEBUG_EVENT( "\n");
	return 0;
}



/**SECTION************************************************************** 
 *
 * 	WANPIPE Device Driver Entry Points 
 *
 * *********************************************************************/



/*============================================================================
 * update - Update wanpipe device status & statistics
 * 
 * @wandev:	Wanpipe device pointer
 *
 * This procedure is called when updating the PROC file system.
 * It returns various communications statistics.
 * 
 * cat /proc/net/wanrouter/wanpipe#  (where #=1,2,3...)
 * 
 * These statistics are accumulated from 3 
 * different locations:
 * 	1) The 'if_stats' recorded for the device.
 * 	2) Communication error statistics on the adapter.
 *      3) Operational statistics on the adapter.
 * 
 * The board level statistics are read during a timer interrupt. 
 * Note that we read the error and operational statistics 
 * during consecitive timer ticks so as to minimize the time 
 * that we are inside the interrupt handler.
 *
 */
static int update (wan_device_t* wandev)
{
	sdla_t* card = wandev->priv;
 	struct net_device* dev;
        volatile private_area_t* priv_area;
        SHARED_MEMORY_INFO_STRUCT *flags;
	unsigned long timeout;

	/* sanity checks */
	if((wandev == NULL) || (wandev->priv == NULL))
		return -EFAULT;
	
	if(wandev->state == WAN_UNCONFIGURED)
		return -ENODEV;

	/* more sanity checks */
        if(!get_card_flags(card))
                return -ENODEV;

	if(test_bit(PERI_CRIT, (void*)&card->wandev.critical))
                return -EAGAIN;

	if((dev=card->wandev.dev) == NULL)
		return -ENODEV;

	if((priv_area=dev->priv) == NULL)
		return -ENODEV;

      	flags = get_card_flags(card);

       	if(card->update_comms_stats){
		return -EAGAIN;
	}
			
	/* TE1	Change the update_comms_stats variable to 3,
	 * 	only for T1/E1 card, otherwise 2 for regular
	 *	S514/S508 card.
	 *	Each timer interrupt will update only one type
	 *	of statistics.
	 */
	card->update_comms_stats = (IS_TE1_CARD(card) || IS_56K_CARD(card)) ? 3 : 2;

       	flags->interrupt_info_struct.interrupt_permission |= APP_INT_ON_TIMER;
	card->timer_int_enabled = TMR_INT_ENABLED_UPDATE;
  
	/* wait a maximum of 1 second for the statistics to be updated */ 
        timeout = jiffies;
        for(;;) {
		if(card->update_comms_stats == 0)
			break;

                if ((jiffies - timeout) > (1 * HZ)){
    			card->update_comms_stats = 0;
 			card->timer_int_enabled &=
				~TMR_INT_ENABLED_UPDATE; 
 			return -EAGAIN;
		}
        }

	return 0;
}


/*============================================================================
 * new_if - Create new logical channel.
 * 
 * &wandev: 	Wanpipe device pointer
 * &dev:	Network device pointer
 * &conf:	User configuration options pointer
 *
 * This routine is called by the ROUTER_IFNEW ioctl,
 * in wanmain.c.  The ioctl passes us the user configuration
 * options which we use to configure the driver and
 * firmware.
 *
 * This functions main purpose is to allocate the
 * private structure for protocol and bind it 
 * to dev->priv pointer.  
 *
 * Also the dev->init pointer should also be initialized
 * to the if_init() function.
 *
 * Any allocation necessary for the private strucutre
 * should be done here, as well as proc/ file initializetion
 * for the network interface.
 * 
 * o parse media- and hardware-specific configuration
 * o make sure that a new channel can be created
 * o allocate resources, if necessary
 * o prepare network device structure for registaration.
 * o add network interface to the /proc/net/wanrouter
 * 
 * The opposite of this function is del_if()
 *
 * Return:	0	o.k.
 *		< 0	failure (channel will not be created)
 */
static int new_if (wan_device_t* wandev, struct net_device* dev, wanif_conf_t* conf)
{
	sdla_t* card = wandev->priv;
	private_area_t* priv_area;
	int err = 0;

	DEBUG_EVENT( "%s: Configuring Interface: %s\n",
			card->devname, conf->name);
 
	if ((conf->name[0] == '\0') || (strlen(conf->name) > WAN_IFNAME_SZ)) {
		DEBUG_EVENT( "%s: Invalid interface name!\n",
			card->devname);
		return -EINVAL;
	}

	/* This protocol only supports a single network interface
	 * If developing a multi-interface protocol, one should
	 * alter this check to allow multiple interfaces */
	if (atomic_read(&card->wandev.if_cnt) > 0){
		return -EEXIST;
	}
		
	/* allocate and initialize private data */
	priv_area = wan_malloc(sizeof(private_area_t));
	if(priv_area == NULL){ 
		WAN_MEM_ASSERT(card->devname);
		return -ENOMEM;
	}
	memset(priv_area, 0, sizeof(private_area_t));
	
	strncpy(priv_area->if_name, conf->name, WAN_IFNAME_SZ);
	
	priv_area->card = card; 

	/* Initialize the socket binding information
	 * These hooks are used by the API sockets to
	 * bind into the network interface */
	priv_area->common.sk = NULL;
	priv_area->common.func = NULL;	
	priv_area->common.state = WAN_CONNECTING;

	priv_area->TracingEnabled = 0;
	priv_area->route_status = NO_ROUTE;
	priv_area->route_removed = 0;

        /* Setup interface as:
	 *    WANPIPE 	  = IP over Protocol (Firmware)
	 *    API     	  = Raw Socket access to Protocol (Firmware)
	 *    BRIDGE  	  = Ethernet over Protocol, no ip info
	 *    BRIDGE_NODE = Ethernet over Protocol, with ip info
	 */
	if(strcmp(conf->usedby, "WANPIPE") == 0) {

		DEBUG_EVENT( "%s: Running in WANPIPE mode!\n",
			wandev->name);
		priv_area->common.usedby = WANPIPE;

		/* Option to bring down the interface when 
        	 * the link goes down */
		if (conf->if_down){
			set_bit(DYN_OPT_ON,&priv_area->interface_down);
			DEBUG_EVENT( 
			 "%s:%s: Dynamic interface configuration enabled\n",
			   card->devname,priv_area->if_name);
		} 

	} else if( strcmp(conf->usedby, "API") == 0) {
		priv_area->common.usedby = API;
		DEBUG_EVENT( "%s:%s: Running in API mode !\n",
			wandev->name,priv_area->if_name);

	}else if (strcmp(conf->usedby, "BRIDGE") == 0) {
		priv_area->common.usedby = BRIDGE;
		DEBUG_EVENT( "%s:%s: Running in WANPIPE (BRIDGE) mode.\n", 
				card->devname,priv_area->if_name);
	
	}else if (strcmp(conf->usedby, "BRIDGE_N") == 0) {
		priv_area->common.usedby = BRIDGE_NODE;
		DEBUG_EVENT( "%s:%s: Running in WANPIPE (BRIDGE_NODE) mode.\n", 
				card->devname,priv_area->if_name);
	
	}else{
		DEBUG_EVENT( "%s:%s: Error: Invalid operation mode [WANPIPE|API|BRIDGE|BRIDGE_NODE]\n",
				card->devname,priv_area->if_name);
		err=-EINVAL;
		goto new_if_error;
	}

	/* BH Circular buffers are used only in API mode.
	 * The BH handler offloads the Rx interrupt and
	 * passes the rx packets up the wanpipe custom
	 * sockets.  
	 *
	 * Allocate and initialize BH circular buffer
	 * Add 1 to MAX_BH_BUFF so we don't have 
	 * test with (MAX_BH_BUFF-1) 
	 */
	priv_area->bh_head = wan_malloc((sizeof(bh_data_t)*(MAX_BH_BUFF+1)));
	if (!priv_area->bh_head){
		WAN_MEM_ASSERT(card->devname);
		err=-ENOMEM;
		goto new_if_error;
	}
	memset(priv_area->bh_head,0,(sizeof(bh_data_t)*(MAX_BH_BUFF+1)));
	atomic_set(&priv_area->bh_buff_used, 0);

	
	/* If gateway option is set, then this interface is the
	 * default gateway on this system. We must know that information
	 * in case DYNAMIC interface configuration is enabled. 
	 *
	 * I.E. If the interface is brought down by the driver, the
	 *      default route will also be removed.  Once the interface
	 *      is brought back up, we must know to re-astablish the
	 *      default route.
	 */
	if ((priv_area->gateway = conf->gateway) == WANOPT_YES){
		DEBUG_EVENT( "%s: Interface %s is set as a gateway.\n",
			card->devname,priv_area->if_name);
	}

	/* Get Multicast Information from the user
	 * FIXME: This option is not clearly defined
	 */
	priv_area->mc = conf->mc;

	
	/* The network interface "dev" has been passed as
	 * an argument from the above layer. We must initialize
	 * it so it can be registered into the kernel.
	 *
	 * The "dev" structure is the link between the kernel
	 * stack and the wanpipe driver.  It contains all 
	 * access hooks that kernel uses to communicate to 
	 * the our driver.
	 *
	 * For now, just set the "dev" name to the user
	 * defined name and initialize:
	 * 	dev->if_init : function that will be called
	 * 	               to further initialize
	 * 	               dev structure on "ifconfig up"
	 *
	 * 	dev->priv    : private structure allocated above
	 * 	
	 */
	
#ifdef LINUX_2_4
	strcpy(dev->name,priv_area->if_name);
#else
	dev->name = (char *)wan_malloc(strlen(priv_area->if_name) + 2); 
	if (!dev->name){
		err=-ENOMEM;
		goto new_if_error;
	}
	sprintf(dev->name, "%s", priv_area->if_name);
#endif

	/* Initialize the polling task routine 
	 * used to defer tasks from interrupt context.
	 * Also it is used to implement dynamic 
	 * interface configuration i.e. bringing interfaces
	 * up and down.
	 */
	
#ifndef LINUX_2_4
	priv_area->poll_task.next = NULL;
#endif
	priv_area->poll_task.sync=0;
	priv_area->poll_task.routine = (void*)(void*)frmw_poll;
	priv_area->poll_task.data = dev;

	/* Initialize the polling delay timer */
	init_timer(&priv_area->poll_delay_timer);
	priv_area->poll_delay_timer.data = (unsigned long)dev;
	priv_area->poll_delay_timer.function = frmw_poll_delay;
	
	/* Create interface file in proc fs.
	 * Once the proc file system is created, the new_if() function
	 * should exit successfuly.  
	 *
	 * DO NOT place code under this function that can return 
	 * anything else but 0.
	 */
	err = wanrouter_proc_add_interface(wandev, 
					   &priv_area->dent, 
					   priv_area->if_name, 
					   dev);
	if (err){	
		DEBUG_EVENT(
			"%s: can't create /proc/net/router/frmw/%s entry!\n",
			card->devname, priv_area->if_name);
		goto new_if_error;
	}

	/* Only setup the dev pointer once the new_if function has
	 * finished successfully.  DO NOT place any code below that
	 * can return an error */
	dev->init = &if_init;
	dev->priv = priv_area;

	/* Indicate to the configuration task frmw_config(), 
	 * that the firmware needs to be configured.  
	 * The configuration task may be called multiple 
	 * times during driver operation.   This flag 
	 * indicates that the firmware is being configured 
	 * for the first time.
	 */
	set_bit(0,&priv_area->config_frmw);

	/* Increment the number of network interfaces 
	 * configured on this card.  
	 */
	atomic_inc(&card->wandev.if_cnt);

	DEBUG_EVENT( "\n");

	return 0;
	
new_if_error:

	if (priv_area->bh_head){
		wan_free(priv_area->bh_head);
	}

	wan_free(priv_area);

#ifndef LINUX_2_4
	if (dev->name){
		wan_free(dev->name);
		dev->name=NULL;
	}
#endif
	dev->priv=NULL;

	return err;
}

/*============================================================================
 * del_if - Delete logical channel.
 *
 * @wandev: 	Wanpipe private device pointer
 * @dev:	Netowrk interface pointer
 *
 * This function is called by ROUTER_DELIF ioctl call
 * to deallocate the network interface.
 * 
 * The network interface and the private structure are
 * about to be deallocated by the upper layer. 
 * We have to clean and deallocate any allocated memory.
 *
 * NOTE: DO NOT deallocate dev->priv here! It will be
 *       done by the upper layer.
 * 
 */
static int del_if (wan_device_t* wandev, struct net_device* dev)
{
	private_area_t* 	priv_area = dev->priv;
	sdla_t*			card = priv_area->card;
	unsigned long smp_flags;

	/* Delete interface name from proc fs. */
	wanrouter_proc_delete_interface(wandev, priv_area->if_name);

	/* Deallocate BH circular buffer for this card
	 * We must do this in a critical area since the
	 * BH handler might be running */
	if (priv_area->bh_head){
		int i;
		struct sk_buff *skb;

		wan_spin_lock_irq(&card->wandev.lock,&smp_flags);		
		
		for (i=0; i<(MAX_BH_BUFF+1); i++){
			skb = ((bh_data_t *)&priv_area->bh_head[i])->skb;
			if (skb != NULL){
                		wan_skb_free(skb);
			}
		}
		wan_free(priv_area->bh_head);
		priv_area->bh_head=NULL;
		
		wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);	
	}


	/* Decrement the number of network interfaces 
	 * configured on this card.  
	 */
	atomic_dec(&card->wandev.if_cnt);
	return 0;
}


/**SECTION***********************************************************
 *
 * 	KERNEL Device Entry Interfaces 
 *
 ********************************************************************/


 
/*============================================================================
 * if_init - Initialize Linux network interface.
 *
 * @dev:	Network interface pointer
 * 
 * During "ifconfig up" the upper layer calls this function
 * to initialize dev access pointers.  Such as transmit,
 * stats and header.
 * 
 * It is called only once for each interface, 
 * during Linux network interface registration.  
 *
 * Returning anything but zero will fail interface
 * registration.
 */
static int if_init (struct net_device* dev)
{
	private_area_t* priv_area = dev->priv;
	sdla_t* card = priv_area->card;
	wan_device_t* wandev = &card->wandev;

	/* Initialize device driver entry points */
	dev->open		= &if_open;
	dev->stop		= &if_close;
	dev->hard_start_xmit	= &if_send;
	dev->get_stats		= &if_stats;
#ifdef LINUX_2_4
	dev->tx_timeout		= &if_tx_timeout;
	dev->watchdog_timeo	= TX_TIMEOUT;
#endif
	dev->do_ioctl		= if_do_ioctl;

	if (priv_area->common.usedby == BRIDGE || 
            priv_area->common.usedby == BRIDGE_NODE){

		/* Setup the interface for Bridging */
		int hw_addr=0;
		ether_setup(dev);
		
		/* Use a random number to generate the MAC address */
		memcpy(dev->dev_addr, "\xFE\xFC\x00\x00\x00\x00", 6);
		get_random_bytes(&hw_addr, sizeof(hw_addr));
		*(int *)(dev->dev_addr + 2) += hw_addr;

	}else{
		/* Initialize media-specific parameters */
		dev->flags		|= IFF_POINTOPOINT;
		dev->flags		|= IFF_NOARP;

		/* Enable Mulitcasting if user selected */
		if (priv_area->mc == WANOPT_YES){
			dev->flags 	|= IFF_MULTICAST;
		}
		
		if (priv_area->true_if_encoding){
			dev->type	= ARPHRD_HDLC; /* This breaks the tcpdump */
		}else{
			dev->type	= ARPHRD_PPP;
		}
		
		dev->mtu		= card->wandev.mtu;
		/* for API usage, add the API header size to the requested MTU size */
		if(priv_area->common.usedby == API) {
			dev->mtu += sizeof(api_tx_hdr_t);
		}
	 
		dev->hard_header_len	= 0;
	}
	
	/* Initialize hardware parameters */
	dev->irq	= wandev->irq;
	dev->dma	= wandev->dma;
	dev->base_addr	= wandev->ioport;
	card->hw_iface.getcfg(card->hw, SDLA_MEMBASE, &dev->mem_start);
	card->hw_iface.getcfg(card->hw, SDLA_MEMEND, &dev->mem_end);

	/* Set transmit buffer queue length 
	 * If too low packets will not be retransmitted 
         * by stack.
	 */
        dev->tx_queue_len = 100;
   
	return 0;
}

/*============================================================================
 * if_open - Open network interface.
 *
 * @dev: Network device pointer
 *
 * On ifconfig up, this function gets called in order
 * to initialize and configure the private area.  
 * Driver should be configured to send and receive data.
 *
 * This functions starts a timer that will call
 * frmw_config() function. This function must be called
 * because the IP addresses could have been changed
 * for this interface.
 *
 * Return 0 if O.k. or errno.
 */
static int if_open (struct net_device* dev)
{
	private_area_t* priv_area = dev->priv;
	sdla_t* card = priv_area->card;
	struct timeval tv;
	int err = 0;

	/* Only one open per interface is allowed */
	if (open_dev_check(dev))
		return -EBUSY;

	/* Initialize the BH handler task queue flag 
	 * indicating that the BH handler is not
	 * running */
	priv_area->tq_working=0;

	/* Initialize the BH handler task queue
	 * Function and argument */
#ifndef LINUX_2_4
	priv_area->common.wanpipe_task.next = NULL;
#endif
	priv_area->common.wanpipe_task.sync = 0;
	priv_area->common.wanpipe_task.routine = (void *)(void *)wp_bh;
	priv_area->common.wanpipe_task.data = dev;

	/* Initialize the router start time.
	 * Used by wanpipemon debugger to indicate
	 * how long has the interface been up */
	do_gettimeofday(&tv);
	priv_area->router_start_time = tv.tv_sec;

#ifdef LINUX_2_4
	netif_start_queue(dev);
#else
	dev->interrupt = 0;
	dev->tbusy = 0;
	dev->start = 1;
#endif

	/* Increment the module usage count */
	wanpipe_open(card);

	/* Each time we bring the interface up we must
	 * run frwm_config() because the ip addresses might
	 * have changed and we must report those to
	 * firmware (if supported). 
	 * 
	 * Thus, we delay the config until the
	 * interface starts up and all ip information has
	 * been setup */
	
	set_bit(0,&priv_area->config_frmw);
	priv_area->config_frmw_timeout=jiffies;
	del_timer(&priv_area->poll_delay_timer);

	/* Start the configuration after 1sec delay.
	 * This will give the interface initilization time
	 * to finish its configuration */
	priv_area->poll_delay_timer.expires=jiffies+HZ;
	add_timer(&priv_area->poll_delay_timer);
	return err;
}

/*============================================================================
 * if_close - Close network interface.
 * 
 * @dev: Network device pointer
 * 
 * On ifconfig down, this function gets called in order
 * to cleanup interace private area.  
 *
 * IMPORTANT: 
 *
 * No deallocation or unconfiguration should ever occur in this 
 * function, because the interface can come back up 
 * (via ifconfig up).  
 * 
 * Furthermore, in dynamic interfacace configuration mode, the 
 * interface will come up and down to reflect the protocol state.
 *
 * Any deallocation and cleanup can occur in del_if() 
 * function.  That function is called before the dev interface
 * itself is deallocated.
 *
 * Thus, we should only stop the net queue and decrement
 * the wanpipe usage counter via wanpipe_close() function.
 */

static int if_close (struct net_device* dev)
{
	private_area_t* priv_area = dev->priv;
	sdla_t* card = priv_area->card;

	stop_net_queue(dev);

#ifndef LINUX_2_4
	dev->start=0;
#endif
	wanpipe_close(card);
	del_timer(&priv_area->poll_delay_timer);
	return 0;
}


/*=============================================================
 * disable_comm - Main shutdown function
 *
 * @card: Wanpipe device pointer
 *
 * The command 'wanrouter stop' has been called
 * and the whole wanpipe device is going down.
 * This is the last function called to disable 
 * all comunications and deallocate any memory
 * that is still allocated.
 *
 * o Disable communications, turn off interrupts
 * o Deallocate memory used, if any
 * o Unconfigure TE1 card
 */ 

static void disable_comm (sdla_t *card)
{
	SHARED_MEMORY_INFO_STRUCT *flags = get_card_flags(card);
	unsigned long smp_flags;

	wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
	if (card->comm_enabled){
		disable_comm_shutdown (card);
	}else{
		flags->interrupt_info_struct.interrupt_permission = 0;	
	}
	wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);

	/* TE1 - Unconfiging, only on shutdown */
	if (IS_TE1_CARD(card)) {
		sdla_te_unconfig(&card->fe);
	}

	return;
}



#ifdef LINUX_2_4
/*============================================================================
 * if_tx_timeout
 *
 * Kernel networking stack calls this function in case
 * the interface has been stopped for TX_TIMEOUT seconds.
 *
 * This would occur if we lost TX interrupts or the
 * card has stopped working for some reason.  
 * 
 * Handle transmit timeout event from netif watchdog
 */
static void if_tx_timeout (struct net_device *dev)
{
    	private_area_t* chan = dev->priv;
	sdla_t *card = chan->card;
	
	/* If our device stays busy for at least 5 seconds then we will
	 * kick start the device by making dev->tbusy = 0.  We expect
	 * that our device never stays busy more than 5 seconds. So this                 
	 * is only used as a last resort.
	 */

	++chan->if_stats.collisions;

	DEBUG_EVENT( "%s: Transmit timed out on %s\n", card->devname,dev->name);
	netif_wake_queue (dev);
}
#endif


/*============================================================================
 * if_send - Send a packet on a network interface.
 * 
 * @dev:	Network interface pointer
 * @skb:	Packet obtained from the stack or API
 *              that should be sent out the port.
 *              
 * o Mark interface as stopped
 * 	(marks start of the transmission) to indicate 
 * 	to the stack that the interface is busy. 
 *
 * o Check link state. 
 * 	If link is not up, then drop the packet.
 * 
 * o Copy the tx packet into the protocol tx buffers on
 *   the adapter.
 *   
 * o If tx successful:
 * 	Free the skb buffer and mark interface as running
 * 	and return 0.
 *
 * o If tx failed, busy:
 * 	Keep interface marked as busy 
 * 	Do not free skb buffer
 * 	Enable Tx interrupt (which will tell the stack
 * 	                     that interace is not busy)
 * 	Return a non-zero value to tell the stack
 * 	that the tx should be retried.
 * 	
 * Return:	0	complete (socket buffer must be freed)
 *		non-0	packet may be re-transmitted 
 *
 */
static int if_send (struct sk_buff* skb, struct net_device* dev)
{
	private_area_t *priv_area = dev->priv;
	sdla_t *card = priv_area->card;
	SHARED_MEMORY_INFO_STRUCT *flags = get_card_flags(card);
	INTERRUPT_INFORMATION_STRUCT *frmw_int = &flags->interrupt_info_struct;
	int udp_type = 0;
	unsigned long smp_flags;
	int err=0;
	unsigned char misc_Tx_bits = 0;

	/* Mark interface as busy. The kernel will not
	 * attempt to send any more packets until we clear
	 * this condition */
#ifdef LINUX_2_4
	netif_stop_queue(dev);
#endif
	
	if (skb == NULL){
		/* This should never happen. Just a sanity check.
		 */
		DEBUG_EVENT( "%s: interface %s got kicked!\n",
			card->devname, dev->name);

		WAN_NETIF_WAKE_QUEUE(dev);
		return 0;
	}

	/* Non 2.4 kernels used to call if_send() 
	 * after TX_TIMEOUT seconds have passed of interface 
	 * being busy. Same as if_tx_timeout() in 2.4 kernels */
#ifndef LINUX_2_4
	if (dev->tbusy){

		/* If our device stays busy for at least 5 seconds then we will
		 * kick start the device by making dev->tbusy = 0.  We expect 
		 * that our device never stays busy more than 5 seconds. So this
		 * is only used as a last resort. 
		 */
                ++chan->if_stats.collisions;
		if((jiffies - priv_area->tick_counter) < (5 * HZ)) {
			return 1;
		}

		DEBUG_EVENT( "%s: Transmit timeout !\n",
			card->devname);

		/* unbusy the interface */
		clear_bit(0,&dev->tbusy);
	}
#endif

	/* PVC_PROT protocol is used by the API sockets
	 * to indicate to the driver that this packet
	 * is a custom RAW packet and no protocol checks
	 * should be done on it */
   	if (ntohs(skb->protocol) != htons(PVC_PROT)){

		/* check the udp packet type */
		
		udp_type = udp_pkt_type(skb, card);

		if (udp_type == UDP_TYPE){
                        if(store_udp_mgmt_pkt(UDP_PKT_FRM_STACK, card, skb, dev,
                                priv_area)){
	                	frmw_int->interrupt_permission |=
					APP_INT_ON_TIMER;
			}
			start_net_queue(dev);
			return 0;
		}

		/* check to see if the source IP address is a broadcast or */
		/* multicast IP address */
                if(priv_area->common.usedby == WANPIPE && chk_bcast_mcast_addr(card, dev, skb)){
			++chan->if_stats.tx_dropped;
			wan_skb_free(skb);
			start_net_queue(dev);
			return 0;
		}
        }

	/* Lock the 508 Card: SMP is supported 
	 * We only lock for 508 card since the S508 ISA
	 * card could only see 8K of memory at the time
	 * and racing conditions were possible with
	 * the interrupts */
      	if(card->type != SDLA_S514){
		s508_lock(card,&smp_flags);
	} 

	/* Sanity check, which blocks the if_send() from being
	 * re-entrant.  The kernel will never do this to us
	 * but its better to be safe */
    	if(test_and_set_bit(SEND_CRIT, (void*)&card->wandev.critical)) {
	
		DEBUG_EVENT( "%s: Critical in if_send: %lx\n",
					card->wandev.name,card->wandev.critical);
                ++chan->if_stats.tx_dropped;
		start_net_queue(dev);
		goto if_send_exit_crit;
	}

	if(card->wandev.state != WAN_CONNECTED){
       		++chan->if_stats.tx_dropped;
		start_net_queue(dev);

	}else if(!skb->protocol){
		/* Skb buffer without protocol should never
		 * occur in normal operation */
        	++chan->if_stats.tx_errors;
		start_net_queue(dev);
		
	}else {
		void* data = skb->data;
		unsigned len = skb->len;
		unsigned char attr;

		/* If it's an API packet pull off the API
		 * header. Also check that the packet size
		 * is larger than the API header
	         */
		if (priv_area->common.usedby == API){
			api_tx_hdr_t* api_tx_hdr;

			/* discard the frame if we are configured for */
			/* 'receive only' mode or if there is no data */
			if (len <= sizeof(api_tx_hdr_t)) {
				
				++chan->if_stats.tx_dropped;
				start_net_queue(dev);
				goto if_send_exit_crit;
			}
				
			api_tx_hdr = (api_tx_hdr_t *)data;
			attr = api_tx_hdr->attr;
			misc_Tx_bits = api_tx_hdr->misc_Tx_bits;
			data += sizeof(api_tx_hdr_t);
			len -= sizeof(api_tx_hdr_t);
		}

		if(frmw_send(card, data, len, misc_Tx_bits)) {

			/* Failed to send, mark queue as busy
			 * and let the stack retry */
			stop_net_queue(dev);
		}else{
			/* Send successful, update stats
			 * and mark queue as ready */
			++chan->if_stats.tx_packets;
                        chan->if_stats.tx_bytes += len;
			start_net_queue(dev);
			
#ifdef LINUX_2_4
		 	dev->trans_start = jiffies;
#endif
		}	
	}

if_send_exit_crit:
	
	/* If the queue is still stopped here, then we
	 * have failed to send! Turn on interrutps and
	 * return the skb buffer to the stack by
	 * exiting with non-zero value.  
	 *
	 * Otherwise, free the skb buffer and return 0 
	 */
	if (!(err=WAN_NETIF_QUEUE_STOPPED(dev))) {
		wan_skb_free(skb);
	}else{
		priv_area->tick_counter = jiffies;
		frmw_int->interrupt_permission |= APP_INT_ON_TX_FRAME;
	}

	/* End of critical area for re-entry and for S508 card */
	clear_bit(SEND_CRIT, (void*)&card->wandev.critical);
	if(card->type != SDLA_S514){
		s508_unlock(card,&smp_flags);
	}
	
	return err;
}


/*============================================================================
 * chk_bcast_mcast_addr - Check for source broadcast addresses
 *
 * Check to see if the packet to be transmitted contains a broadcast or
 * multicast source IP address.
 */

static int chk_bcast_mcast_addr(sdla_t *card, struct net_device* dev,
				struct sk_buff *skb)
{
	u32 src_ip_addr;
        u32 broadcast_ip_addr = 0;
	private_area_t *priv_area=dev->priv;
        struct in_device *in_dev;
        /* read the IP source address from the outgoing packet */
        src_ip_addr = *(u32 *)(skb->data + 12);

	if (priv_area->common.usedby != WANPIPE){
		return 0;
	}
	
	/* read the IP broadcast address for the device */
        in_dev = dev->ip_ptr;
        if(in_dev != NULL) {
                struct in_ifaddr *ifa= in_dev->ifa_list;
                if(ifa != NULL)
                        broadcast_ip_addr = ifa->ifa_broadcast;
                else
                        return 0;
        }
 
        /* check if the IP Source Address is a Broadcast address */
        if((dev->flags & IFF_BROADCAST) && (src_ip_addr == broadcast_ip_addr)) {
                DEBUG_EVENT( "%s: Broadcast Source Address silently discarded\n",
				card->devname);
                return 1;
        } 

        /* check if the IP Source Address is a Multicast address */
        if((ntohl(src_ip_addr) >= 0xE0000001) &&
		(ntohl(src_ip_addr) <= 0xFFFFFFFE)) {
                DEBUG_EVENT( "%s: Multicast Source Address silently discarded\n",
				card->devname);
                return 1;
        }

        return 0;
}

/*============================================================================
 * if_stats
 *
 * Used by /proc/net/dev and ifconfig to obtain interface
 * statistics.
 *
 * Return a pointer to struct net_device_stats.
 */
static struct net_device_stats* if_stats (struct net_device* dev)
{
	private_area_t* priv_area;

	if ((priv_area=dev->priv) == NULL)
		return NULL;

	return &priv_area->if_stats;
}




/*========================================================================
 * 
 * if_do_ioctl - Ioctl handler for fr
 *
 * 	@dev: Device subject to ioctl
 * 	@ifr: Interface request block from the user
 *	@cmd: Command that is being issued
 *	
 *	This function handles the ioctls that may be issued by the user
 *	to control or debug the protocol or hardware . 
 *
 *	It does both busy and security checks. 
 *	This function is intended to be wrapped by callers who wish to 
 *	add additional ioctl calls of their own.
 *
 * Used by:  SNMP Mibs
 * 	     wanpipemon debugger
 *
 */
static int if_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	private_area_t* chan= (private_area_t*)dev->priv;
	unsigned long smp_flags;
	sdla_t *card;
	wan_udp_pkt_t *wan_udp_pkt;
	
	if (!chan){
		return -ENODEV;
	}
	card=chan->card;

	NET_ADMIN_CHECK();

	switch(cmd)
	{
		case SIOC_WANPIPE_PIPEMON: 
			
			if (atomic_read(&chan->udp_pkt_len) != 0){
				return -EBUSY;
			}
	
			atomic_set(&chan->udp_pkt_len,MAX_LGTH_UDP_MGNT_PKT);
			
			/* For performance reasons test the critical
			 * here before spin lock */
			if (test_bit(0,&card->in_isr)){
				atomic_set(&chan->udp_pkt_len,0);
				return -EBUSY;
			}
		
			
			wan_udp_pkt=(wan_udp_pkt_t*)chan->udp_pkt_data;
			if (copy_from_user(&wan_udp_pkt->wan_udp_hdr,ifr->ifr_data,sizeof(wan_udp_hdr_t))){
				atomic_set(&chan->udp_pkt_len,0);
				return -EFAULT;
			}

			spin_lock_irqsave(&card->wandev.lock, smp_flags);

			/* We have to check here again because we don't know
			 * what happened during spin_lock */
			if (test_bit(0,&card->in_isr)) {
				DEBUG_EVENT( "%s:%s Pipemon command failed, Driver busy: try again.\n",
						card->devname,dev->name);
				atomic_set(&chan->udp_pkt_len,0);
				spin_unlock_irqrestore(&card->wandev.lock, smp_flags);
				return -EBUSY;
			}
			
			process_udp_mgmt_pkt(card,dev,chan,1);
			
			spin_unlock_irqrestore(&card->wandev.lock, smp_flags);

			/* This area will still be critical to other
			 * PIPEMON commands due to udp_pkt_len
			 * thus we can release the irq */
			
			if (atomic_read(&chan->udp_pkt_len) > sizeof(wan_udp_pkt_t)){
				DEBUG_EVENT( "%s: Error: Pipemon buf too bit on the way up! %i\n",
						card->devname,atomic_read(&chan->udp_pkt_len));
				atomic_set(&chan->udp_pkt_len,0);
				return -EINVAL;
			}

			if (copy_to_user(ifr->ifr_data,&wan_udp_pkt->wan_udp_hdr,sizeof(wan_udp_hdr_t))){
				atomic_set(&chan->udp_pkt_len,0);
				return -EFAULT;
			}
			
			atomic_set(&chan->udp_pkt_len,0);
			return 0;
	
			
		default:
			return -EOPNOTSUPP;
	}
	return 0;
}





/**SECTION**********************************************************
 *
 * 	FIRMWARE Specific Interface Functions 
 *
 *******************************************************************/

static int init_interrupt_status_ptrs(sdla_t *card)
{
	wan_mbox_t *mb1;
	int err;

	/* Set up the interrupt status area */
	/* Read the Configuration and obtain: 
	 *	Ptr to shared memory infor struct
         * Use this pointer to calculate the value of get_card_flags(card) !
 	 */
	mb1->wan_data_len = 0;
	mb1->wan_command = READ_CONFIGURATION;
	err = sdla_exec(mb1) ? mb1->wan_return_code : CMD_TIMEOUT;
	if(err != COMMAND_OK) {
		frmw_error(card, err, mb1);
		return -EIO;
	}

	/* Alex Apr 8 2004 Sangom ISA card */
	set_card_flags (card,
			(void *)(card->hw.dpmbase +
             			         (((CONFIGURATION_STRUCT *)mb1->wan_data)->
				           ptr_shared_mem_info_struct))
			);
	return 0;
}



/*============================================================
 * set_adapter_config
 * 
 * Set adapter config configures the firmware for
 * the specific front end hardware.  
 * Front end hardware: T1/E1/56K/V32/RS232
 *
 */ 

static int set_adapter_config (sdla_t* card)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	char* data = mb->wan_data;
	int err;

	((ADAPTER_CONFIGURATION_STRUCT *)data)->adapter_type = 
			sdla_get(&card->hw, SDLA_ADAPTERTYPE); 
	((ADAPTER_CONFIGURATION_STRUCT *)data)->adapter_config = 0x00; 
	((ADAPTER_CONFIGURATION_STRUCT *)data)->operating_frequency = 00; 
	mb->wan_data_len = sizeof(ADAPTER_CONFIGURATION_STRUCT);
	mb->wan_command = SET_ADAPTER_CONFIGURATION;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;

	if(err != COMMAND_OK) {
		frmw_error(card,err,mb);
	}
	return (err);
}


/*============================================================================
 * frmw_read_version
 * 
 * Read firmware code version.
 * Put code version as ASCII string in str. 
 */
static int frmw_read_version (sdla_t* card, char* str)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	int len;
	char err;
	mb->wan_data_len = 0;
	mb->wan_command = READ_CODE_VERSION;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;

	if(err != COMMAND_OK) {
		frmw_error(card,err,mb);
	}
	else if (str) {  /* is not null */
		len = mb->wan_data_len;
		memcpy(str, mb->wan_data, len);
		str[len] = '\0';
	}
	return (err);
}

/*-----------------------------------------------------------------------------
 *  Configure firmware.
 */
static int frmw_configure (sdla_t* card, void* data)
{
	int err;
	wan_mbox_t *mb = get_card_mailbox(card);
	int data_length = sizeof(CONFIGURATION_STRUCT);
	
	mb->wan_data_len = data_length;  
	memcpy(mb->wan_data, data, data_length);
	mb->wan_command = SET_CONFIGURATION;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
	
	if (err != COMMAND_OK) frmw_error (card, err, mb);
                           
	return err;
}


/*============================================================================
 * Set interrupt mode -- HDLC Version.
 */

static int frmw_set_intr_mode (sdla_t* card, unsigned mode)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	INT_TRIGGERS_STRUCT* int_data =
		 (INT_TRIGGERS_STRUCT *)mb->wan_data;
	int err;

	int_data->interrupt_triggers 	= mode;
	int_data->IRQ				= card->hw.irq;
	int_data->interrupt_timer               = 1;
   
	mb->wan_data_len = sizeof(INT_TRIGGERS_STRUCT);
	mb->wan_command = SET_INTERRUPT_TRIGGERS;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
	if (err != COMMAND_OK)
		frmw_error (card, err, mb);
	return err;
}


/*===========================================================
 * disable_comm_shutdown
 *
 * Shutdown() disables the communications. We must
 * have a sparate functions, because we must not
 * call frmw_error() hander since the private
 * area has already been replaced */

static int disable_comm_shutdown (sdla_t *card)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	INT_TRIGGERS_STRUCT* int_data =
		 (INT_TRIGGERS_STRUCT *)mb->wan_data;
	int err;

	/* Disable Interrutps */
	int_data->interrupt_triggers 		= 0;
	int_data->IRQ				= card->hw.irq;
	int_data->interrupt_timer               = 1;
   
	mb->wan_data_len = sizeof(INT_TRIGGERS_STRUCT);
	mb->wan_command = SET_INTERRUPT_TRIGGERS;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;

	/* Disable Communications */

	mb->wan_command = DISABLE_COMMUNICATIONS;
	
	mb->wan_data_len = 0;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
	
	card->comm_enabled = 0;

	/* TE1 - Unconfiging, only on shutdown */
	if (IS_TE1_CARD(card)) {
		sdla_te_unconfig(&card->fe);
	}

	return 0;
}

/*============================================================================
 * Enable communications.
 */

static int frmw_comm_enable (sdla_t* card)
{
	int err;
	wan_mbox_t* mb = get_card_mailbox(card);

	mb->wan_data_len = 0;
	mb->wan_command = ENABLE_COMMUNICATIONS;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
	if (err != COMMAND_OK)
		frmw_error(card, err, mb);
	else
		card->comm_enabled = 1;
	
	return err;
}

/*============================================================================
 * Read communication error statistics.
 */
static int frmw_read_comm_err_stats (sdla_t* card)
{
        int err;
        wan_mbox_t* mb = get_card_mailbox(card);

        mb->wan_data_len = 0;
        mb->wan_command = READ_COMMS_ERROR_STATS;
        err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
        if (err != COMMAND_OK)
                frmw_error(card,err,mb);
        return err;
}


/*============================================================================
 * Read operational statistics.
 */
static int frmw_read_op_stats (sdla_t* card)
{
        int err;
        wan_mbox_t* mb = get_card_mailbox(card);

        mb->wan_data_len = 0;
        mb->wan_command = READ_OPERATIONAL_STATS;
        err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
        if (err != COMMAND_OK)
                frmw_error(card,err,mb);
        return err;
}


/*============================================================================
 * Update communications error and general packet statistics.
 */
static int update_comms_stats(sdla_t* card, private_area_t* priv_area)
{
        wan_mbox_t* mb = get_card_mailbox(card);
  	COMMS_ERROR_STATS_STRUCT* err_stats;
        OPERATIONAL_STATS_STRUCT* op_stats;

	if(card->update_comms_stats == 3) {
		/* 1. On the first timer interrupt, update T1/E1 alarms 
		 * and PMON counters (only for T1/E1 card) (TE1) 
		 */
		/* TE1 Update T1/E1 alarms */
		if (IS_TE1_CARD(card)) {	
			card->wandev.fe_iface.read_alarm(&card->fe, WAN_FE_ALARM_READ|WAN_FE_ALARM_UPDATE); 
			/* TE1 Update T1/E1 perfomance counters */
			sdla_te_pmon(card);

		}else if (IS_56K_CARD(card)) {
			/* 56K Update CSU/DSU alarms */
			card->wandev.k56_alarm = sdla_56k_alarm(card,1);
		}
	 } else { 
		/* 2. On the second timer interrupt, read the comms error 
	 	 * statistics 
	 	 */
		if(card->update_comms_stats == 2) {
			if(frmw_read_comm_err_stats(card))
				return 1;
			err_stats = (COMMS_ERROR_STATS_STRUCT *)mb->wan_data;
			card->wandev.stats.rx_over_errors = 
				err_stats->Rx_overrun_err_count;
			card->wandev.stats.rx_crc_errors = 
				err_stats->CRC_err_count;
			card->wandev.stats.rx_frame_errors = 
				err_stats->Rx_abort_count;
			card->wandev.stats.rx_fifo_errors = 
				err_stats->Rx_dis_pri_bfrs_full_count; 
			card->wandev.stats.rx_missed_errors =
				card->wandev.stats.rx_fifo_errors;
			card->wandev.stats.tx_aborted_errors =
				err_stats->sec_Tx_abort_count;
		} else {

        		/* on the third timer interrupt, read the operational 
			 * statistics 
		 	 */
        		if(frmw_read_op_stats(card))
                		return 1;
			op_stats = (OPERATIONAL_STATS_STRUCT *)mb->wan_data;
			card->wandev.stats.rx_length_errors =
				(op_stats->Rx_Data_discard_short_count +
				op_stats->Rx_Data_discard_long_count);
		}
	}

	return 0;
}

/*============================================================================
 * Send packet.
 *	Return:	0 - o.k.
 *		1 - no transmit buffers available
 */
static int frmw_send (sdla_t* card, void* data, unsigned len, unsigned char tx_bits)
{
	DATA_TX_STATUS_EL_STRUCT *txbuf = get_card_txbuf(card);

	if (txbuf->opp_flag)
		return 1;
	
	sdla_poke(&card->hw, txbuf->ptr_data_bfr, data, len);

	txbuf->frame_length = len;
	txbuf->misc_Tx_bits = tx_bits;
	txbuf->opp_flag = 1;		/* start transmission */
	
	/* Update transmit buffer control fields */
	set_card_txbuf(card,++txbuf);
	
	return 0;
}

/*============================================================================
 * Read TE1/56K Front end registers
 */
static unsigned char read_front_end_reg (void* card1, ...)
{
	va_list		args;
	sdla_t		*card = (sdla_t*)card1;
        wan_mbox_t* mb = get_card_mailbox(card);
	char* data = mb->wan_data;
	u16		reg, line_no;
        int err;

	va_start(args, card1);
	line_no	= (u16)va_arg(args, int);
	reg	= (u16)va_arg(args, int);
	va_end(args);
	
	((FRONT_END_REG_STRUCT *)data)->register_number = (unsigned short)reg;
	mb->wan_data_len = sizeof(FRONT_END_REG_STRUCT);
        mb->wan_command = READ_FRONT_END_REGISTER;
        err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
        if (err != COMMAND_OK)
                frmw_error(card,err,mb);

	return(((FRONT_END_REG_STRUCT *)data)->register_value);
}

/*============================================================================
 * Write to TE1/56K Front end registers  
 */
static unsigned char write_front_end_reg (void* card1, ...)
{
	va_list		args;
	sdla_t		*card = (sdla_t*)card1;
        wan_mbox_t* mb = get_card_mailbox(card);
	char* data = mb->wan_data;
	u16		reg, line_no;
	u8		value;
        int		err, retry=15;
	
	va_start(args, card1);
	line_no	= (u16)va_arg(args, int);
	reg	= (u16)va_arg(args, int);
	value	= (u8)va_arg(args, int);
	va_end(args);
	
	do {
		((FRONT_END_REG_STRUCT *)data)->register_number = (unsigned short)reg;
		((FRONT_END_REG_STRUCT *)data)->register_value = value;
		mb->wan_data_len = sizeof(FRONT_END_REG_STRUCT);
		mb->wan_command = WRITE_FRONT_END_REGISTER;
		err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
		if (err != COMMAND_OK)
			frmw_error(card,err,mb);
	}while(err && --retry);
	
        return err;
}

/*============================================================================
 * Enable timer interrupt  
 */
static void enable_timer (void* card_id)
{
	sdla_t* 		card = (sdla_t*)card_id;
	SHARED_MEMORY_INFO_STRUCT* flags = get_card_flags(card);

	DEBUG_EVENT( "%s: enabling timer %s\n",card->devname,
			card->wandev.dev?card->wandev.dev->name:"No DEV");
		
	card->timer_int_enabled |= TMR_INT_ENABLED_TE;
	flags->interrupt_info_struct.interrupt_permission |= APP_INT_ON_TIMER;
	return;
}

/*============================================================================
 * Firmware error handler.
 *	This routine is called whenever firmware command returns non-zero
 *	return code.
 *
 * Return zero if previous command has to be cancelled.
 */
static int frmw_error (sdla_t *card, int err, wan_mbox_t *mb)
{
	unsigned cmd = mb->wan_command;

	switch (err) {

	default:
		DEBUG_EVENT( "%s: command 0x%02X returned 0x%02X!\n",
			card->devname, cmd, err);
	}

	return 0;
}


/*===============================================================
 * set_frmw_config() 
 *
 * used to set configuration options on the board
 *
 */

static int set_frmw_config(sdla_t* card)
{
	CONFIGURATION_STRUCT cfg;

	memset(&cfg, 0, sizeof(CONFIGURATION_STRUCT));

	if(card->wandev.clocking){
		cfg.baud_rate = card->wandev.bps;
	}
		
#error  "->PROTOCOL SPECIFIC CONFIGURATION"

	return frmw_configure(card, &cfg);
}

static int frmw_calibrate_baud (sdla_t *card)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	int err;
	
	mb->wan_data_len = 0;
	mb->wan_command = START_BAUD_CALIBRATION;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;

	if (err != COMMAND_OK) 
		frmw_error (card, err, mb);

	return err;
}

static int frmw_read_baud_calibration (sdla_t *card)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	int err;
	
read_baud_again:
	
	mb->wan_data_len = 0;
	mb->wan_command = READ_BAUD_CALIBRATION_RESULT;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;

	switch (err){

		case COMMAND_OK:
			mb->wan_data_len = 0;
			mb->wan_command = READ_CONFIGURATION;
			err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
			if (err == COMMAND_OK){
				card->wandev.bps = ((CONFIGURATION_STRUCT*)mb->wan_data)->baud_rate;
				DEBUG_EVENT( "%s: Baud Rate calibrated at: %i bps\n",
					card->devname,card->wandev.bps);
			}
			break;
		
		case BAUD_CALIBRATION_NOT_DONE:
		case BUSY_WITH_BAUD_CALIBRATION:	
			goto read_baud_again;
			
		case BAUD_CAL_FAILED_NO_TX_CLK:
			DEBUG_EVENT( "%s: Baud Rate calibration failed: No Tx Clock!\n",
					card->devname);
			return -EINVAL;

		case BAUD_CAL_FAILED_BAUD_HI:
			DEBUG_EVENT( "%s: Baud Rate calibration failed: Baud to High!\n",
					card->devname);
			return -EINVAL;

		case CANNOT_DO_BAUD_CAL:
			DEBUG_EVENT( "%s: Baud Rate calibration cannot be performed!\n",
					card->devname);
			return -EINVAL;
		default:
			frmw_error(card,err,mb);
			return -EINVAL;
	}
	return err;
}


/*============================================================================
 * init_tx_rx_buff
 *
 * Initialize hardware Receive and Transmit Buffers.
 *
 */

static void init_tx_rx_buff( sdla_t* card)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	TX_STATUS_EL_CFG_STRUCT *tx_config;
	RX_STATUS_EL_CFG_STRUCT *rx_config;
	char err;
	
	mb->wan_data_len = 0;
	mb->wan_command = READ_CONFIGURATION;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;

	if(err != COMMAND_OK) {
		if (card->wandev.dev){
			frmw_error(card,err,mb);
		}
		return;
	}

	/* Alex Apr 8 2004 Sangoma ISA card */
	tx_config = 
		(TX_STATUS_EL_CFG_STRUCT *)(card->hw.dpmbase +
                (((CONFIGURATION_STRUCT *)mb->wan_data)->
                	            ptr_Tx_stat_el_cfg_struct));
       	rx_config = 
		(RX_STATUS_EL_CFG_STRUCT *)(card->hw.dpmbase +
                (((CONFIGURATION_STRUCT *)mb->wan_data)->
                        	    ptr_Rx_stat_el_cfg_struct));

       	/* Setup Head and Tails for buffers */
       	card->u.c.txbuf_base = 
		(void *)(card->hw.dpmbase +
                tx_config->base_addr_Tx_status_elements);
       	card->u.c.txbuf_last = 
		(DATA_TX_STATUS_EL_STRUCT *)card->u.c.txbuf_base +
		(tx_config->number_Tx_status_elements - 1);

       	card->u.c.rxbuf_base = 
		(void *)(card->hw.dpmbase +
                rx_config->base_addr_Rx_status_elements);
        card->u.c.rxbuf_last =
		(DATA_RX_STATUS_EL_STRUCT *)card->u.c.rxbuf_base +
		(rx_config->number_Rx_status_elements - 1);

	/* Set up next pointer to be used */
        card->u.c.txbuf = 
		(void *)(card->hw.dpmbase +
                tx_config->next_Tx_status_element_to_use);
        card->u.c.rxmb = 
		(void *)(card->hw.dpmbase +
                rx_config->next_Rx_status_element_to_use);

        /* Setup Actual Buffer Start and end addresses */
        card->u.c.rx_base = rx_config->base_addr_Rx_buffer;
        card->u.c.rx_top  = rx_config->end_addr_Rx_buffer;

}

/*=============================================================================
 * intr_test
 * 
 * Perform Interrupt Test by running READ_CODE_VERSION command MAX_INTR
 * _TEST_COUNTER times.
 * 
 */
static int intr_test( sdla_t* card)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	int err,i;
	SHARED_MEMORY_INFO_STRUCT* flags = get_card_flags(card);

	Intr_test_counter = 0;
	
	err = frmw_set_intr_mode(card, APP_INT_ON_COMMAND_COMPLETE);

	if (err == CMD_OK) { 
		for (i = 0; i < MAX_INTR_TEST_COUNTER; i ++) {	
			mb->wan_data_len  = 0;
			mb->wan_command = READ_CODE_VERSION;
			err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
			if (err != CMD_OK) 
				frmw_error(card, err, mb);
		}
	}
	else {
		return err;
	}

	flags->interrupt_info_struct.interrupt_type = 0;
	err = frmw_set_intr_mode(card, 0);

	if (err != CMD_OK)
		return err;

	return 0;
}





/**SECTION************************************************** 
 *
 * 	API Bottom Half Handlers 
 *
 **********************************************************/


static void wp_bh (struct net_device * dev)
{
	private_area_t* chan = dev->priv;
	sdla_t *card = chan->card;
	struct sk_buff *skb;

	if (!chan->bh_head || atomic_read(&chan->bh_buff_used) == 0){
		clear_bit(0, &chan->tq_working);
		return;
	}

	while (atomic_read(&chan->bh_buff_used)){

		skb  = ((bh_data_t *)&chan->bh_head[chan->bh_read])->skb;

		if (skb != NULL){

			if (chan->common.sk == NULL || chan->common.func == NULL){
				++chan->if_stats.rx_dropped;
				wan_skb_free(skb);
				wp_bh_cleanup(dev);
				continue;
			}

			if (chan->common.func(skb,dev,chan->common.sk) != 0){
				++chan->if_stats.rx_dropped;
				wan_skb_free(skb);
			}
		}
		wp_bh_cleanup(dev);
	}	
	clear_bit(0, &chan->tq_working);

	return;
}

static int wp_bh_cleanup (struct net_device *dev)
{
	private_area_t* chan = dev->priv;

	((bh_data_t *)&chan->bh_head[chan->bh_read])->skb = NULL;

	if (chan->bh_read == MAX_BH_BUFF){
		chan->bh_read=0;
	}else{
		++chan->bh_read;	
	}

	atomic_dec(&chan->bh_buff_used);
	return 0;
}



static int bh_enqueue (struct net_device *dev, struct sk_buff *skb)
{
	/* Check for full */
	private_area_t* chan = dev->priv;
	sdla_t *card = chan->card;

	if (!chan->bh_head || atomic_read(&chan->bh_buff_used) == (MAX_BH_BUFF+1)){
		++chan->if_stats.rx_dropped;
		wan_skb_free(skb);
		return 1; 
	}

	((bh_data_t *)&chan->bh_head[chan->bh_write])->skb = skb;

	if (chan->bh_write == MAX_BH_BUFF){
		chan->bh_write=0;
	}else{
		++chan->bh_write;
	}

	atomic_inc(&chan->bh_buff_used);

	return 0;
}


static void wakeup_sk_bh (struct net_device *dev)
{

	private_area_t *chan = dev->priv;

	/* If the sock is in the process of unlinking the
	 * driver from the socket, we must get out. 
	 * This never happends but is a sanity check. */
	if (test_bit(0,&chan->common.common_critical)){
		return;
	}

	if (!chan->common.sk || !chan->common.func){
		return;
	}

	chan->common.func(NULL,dev,chan->common.sk);
	return;
}



/**SECTION*************************************************************** 
 * 
 * 	HARDWARE Interrupt Handlers 
 *
 ***********************************************************************/


#error "->CHANGE THE ISR NAME "
/*============================================================================
 * wpfw_isr
 *
 * Main interrupt service routine. 
 * Determin the interrupt received and handle it.
 *
 */
static void wpc_isr (sdla_t* card)
{
	struct net_device* dev;
	SHARED_MEMORY_INFO_STRUCT* flags = NULL;
	int i;
	sdla_t *my_card;


	/* Check for which port the interrupt has been generated
	 * Since one Port is piggybacking on the other,
	 * the check must be performed to see which prot is the
	 * interrupt owner.
	 */
	flags = get_card_flags(card);
	if (!flags->interrupt_info_struct.interrupt_type){
		/* Check for a second port (piggybacking) */
		if ((my_card = card->next) != NULL){
			flags = get_card_flags(my_card);
			if (flags->interrupt_info_struct.interrupt_type){
				card = my_card;
				card->isr(card);
				return;
			}
		}
	}

	flags = get_card_flags(card);
	card->in_isr = 1;
	dev = card->wandev.dev;
	
	/* If we get an interrupt with no network device, stop the interrupts
	 * and issue an error */
	if (!dev && flags->interrupt_info_struct.interrupt_type != 
	    	COMMAND_COMPLETE_APP_INT_PEND){
		goto isr_done;
	}
	
	/* if critical due to peripheral operations
	 * ie. update() or getstats() then reset the interrupt and
	 * wait for the board to retrigger.
	 */
	if(test_bit(PERI_CRIT, (void*)&card->wandev.critical)) {
		DEBUG_EVENT( "%s: ISR:  Critical with PERI_CRIT!\n",
				card->devname);
		goto isr_done;
	}

	/* On a 508 Card, if critical due to if_send 
         * Major Error. The critical section in if_send()
	 * should have avoided this situation. */
	if(card->type != SDLA_S514) {
		if(test_bit(SEND_CRIT, (void*)&card->wandev.critical)) {
			DEBUG_EVENT( "%s: ISR: Critical with SEND_CRIT!\n",
				card->devname);
			card->in_isr = 0;
			flags->interrupt_info_struct.interrupt_type = 0;
			return;
		}
	}

	switch(flags->interrupt_info_struct.interrupt_type) {

	case RX_APP_INT_PEND:	/* 0x01: receive interrupt */
		rx_intr(card);
		break;

	case TX_APP_INT_PEND:	/* 0x02: transmit interrupt */
		flags->interrupt_info_struct.interrupt_permission &=
			 ~APP_INT_ON_TX_FRAME;

		if (dev && WAN_NETIF_QUEUE_STOPPED(dev)){
			private_area_t* priv_area=dev->priv;
			if (priv_area->common.usedby == API){
				WAN_NETIF_START_QUEUE(dev);
				wakeup_sk_bh(dev);
			}else{
				WAN_NETIF_WAKE_QUEUE(dev);
			}
		}
		break;

	case COMMAND_COMPLETE_APP_INT_PEND:/* 0x04: cmd complete */
		++ Intr_test_counter;
		break;

	case EXCEP_COND_APP_INT_PEND:	/* 0x20: Exception */
		process_exception(card);
		break;

	case GLOBAL_EXCEP_COND_APP_INT_PEND:  
		process_global_exception(card);
		
		/* Reset the 56k or T1/E1 front end exception condition */
		if(IS_56K_CARD(card) || IS_TE1_CARD(card)) {
			FRONT_END_STATUS_STRUCT *FE_status =
				(FRONT_END_STATUS_STRUCT *)&flags->FT1_info_struct.parallel_port_A_input;
			FE_status->opp_flag = 0x01;	
		}
		break;

	case TIMER_APP_INT_PEND:	/* Timer interrupt */
		timer_intr(card);
		break;

	default:
		DEBUG_EVENT( "%s: spurious interrupt 0x%02X!\n", 
			card->devname,
			flags->interrupt_info_struct.interrupt_type);
		DEBUG_EVENT( "Code name: ");
		for(i = 0; i < 4; i ++){
			printk("%c",
				flags->global_info_struct.codename[i]); 
		}
		printk("\n");
		DEBUG_EVENT( "Code version: ");
	 	for(i = 0; i < 4; i ++){
			printk("%c", 
				flags->global_info_struct.codeversion[i]); 
		}
		printk("\n");	
		break;
	}

isr_done:

	card->in_isr = 0;
	flags->interrupt_info_struct.interrupt_type = 0;
	return;
}

/*============================================================================
 * rx_intr
 *
 * Receive interrupt handler.
 */
static void rx_intr (sdla_t* card)
{
	struct net_device *dev;
	private_area_t *priv_area;
	SHARED_MEMORY_INFO_STRUCT *flags = get_card_flags(card);
	DATA_RX_STATUS_EL_STRUCT *rxbuf = get_card_rxmb(card);
	struct sk_buff *skb;
	unsigned len;
	unsigned addr = rxbuf->ptr_data_bfr;
	void *buf;
	int i,udp_type;

	if (rxbuf->opp_flag != 0x01) {
		DEBUG_EVENT( 
			"%s: corrupted Rx buffer @ 0x%X, flag = 0x%02X!\n", 
			card->devname, (unsigned)rxbuf, rxbuf->opp_flag);
                DEBUG_EVENT( "Code name: ");
                for(i = 0; i < 4; i ++)
                        DEBUG_EVENT( "%c",
                                flags->global_info_struct.codename[i]);
		DEBUG_EVENT( "\n");
                DEBUG_EVENT( "Code version: ");
                for(i = 0; i < 4; i ++)
                        DEBUG_EVENT( "%c",
                                flags->global_info_struct.codeversion[i]);
                DEBUG_EVENT( "\n");


		/* Bug Fix: Mar 6 2000
                 * If we get a corrupted mailbox, it measn that driver 
                 * is out of sync with the firmware. There is no recovery.
                 * If we don't turn off all interrupts for this card
                 * the machine will crash. 
                 */
		DEBUG_EVENT( "%s: Critical router failure ...!!!\n", card->devname);
		DEBUG_EVENT( "Please contact Sangoma Technologies !\n");
		frmw_set_intr_mode(card,0);	
		return;
	}

	len  = rxbuf->frame_length;

	dev = card->wandev.dev;

	if (!dev){
		goto rx_exit;
	}

	if (!is_dev_running(dev))
		goto rx_exit;

	priv_area = dev->priv;

	
	/* Allocate socket buffer */
	skb = wan_skb_alloc(len+2);

	if (skb == NULL) {
		DEBUG_EVENT( "%s: no socket buffers available!\n",
					card->devname);
		++chan->if_stats.rx_dropped;
		goto rx_exit;
	}

	/* Align IP on 16 byte */
	wan_skb_reserve(skb,2);

	/* Copy data to the socket buffer */
	rx_top_buf=get_rxtop_buf(card);
	if((addr + len) > rx_top_buf  + 1) {
		unsigned tmp = rx_top_buf - addr + 1;
		buf = wan_skb_put(skb, tmp);
		sdla_peek(&card->hw, addr, buf, tmp);
		addr = get_rxbase_buf(card);
		len -= tmp;
	}
		
	buf = wan_skb_put(skb, len);
	sdla_peek(&card->hw, addr, buf, len);

	skb->protocol = htons(ETH_P_IP);

	chan->if_stats.rx_packets ++;
	chan->if_stats.rx_bytes += skb->len;
	udp_type = udp_pkt_type( skb, card );

	if(udp_type == UDP_TYPE) {
		if(store_udp_mgmt_pkt(UDP_PKT_FRM_NETWORK,
   				      card, skb, dev, priv_area)) {
     		        flags->interrupt_info_struct.
						interrupt_permission |= 
							APP_INT_ON_TIMER; 
		}
	} else if(priv_area->common.usedby == API) {

		api_rx_hdr_t* api_rx_hdr;
       		wan_skb_push(skb, sizeof(api_rx_hdr_t));
                api_rx_hdr = (api_rx_hdr_t*)&skb->data[0x00];
		api_rx_hdr->error_flag = rxbuf->error_flag;
     		api_rx_hdr->time_stamp = rxbuf->time_stamp;

                skb->protocol = htons(PVC_PROT);
     		skb->mac.raw  = skb->data;
		skb->dev      = dev;
               	skb->pkt_type = WAN_PACKET_DATA;

		bh_enqueue(dev, skb);

		/* FIXME
		 * If we fail to queue the task, then we must wait
		 * for another packet to retry! */
		if (!test_and_set_bit(0,&priv_area->tq_working)){
			if (wanpipe_queue_tq(&priv_area->common.wanpipe_task) != 0){
				clear_bit(0,&priv_area->tq_working);
			}
			if (wanpipe_mark_bh() != 0){
				clear_bit(0,&priv_area->tq_working);
			}
		}

	}else if (priv_area->common.usedby == BRIDGE ||
		  priv_area->common.usedby == BRIDGE_NODE){
		
		/* Make sure it's an Ethernet frame, otherwise drop it */
		if (skb->len <= ETH_ALEN) {
			wan_skb_free(skb);
			++chan->if_stats.rx_errors;
			goto rx_exit;
		}
	
		skb->dev = dev;
		skb->protocol=eth_type_trans(skb,dev);
		netif_rx(skb);
	}else{
		/* FIXME: we should check to see if the received packet is a 
                          multicast packet so that we can increment the multicast 
                          statistic
                          ++ priv_area->if_stats.multicast;
		*/
               	/* Pass it up the protocol stack */

		skb->protocol = htons(ETH_P_IP);
                skb->dev = dev;
                skb->mac.raw  = skb->data;
                netif_rx(skb);
	}

rx_exit:
	/* Release buffer element and calculate a pointer to the next one */
	rxbuf->opp_flag = 0x00;
	set_card_rxmb(card,++rxbuf);
}

/*============================================================================
 * Timer interrupt handler.
 * The timer interrupt is used for two purposes:
 *    1) Processing udp calls from 'cpipemon'.
 *    2) Reading board-level statistics for updating the proc file system.
 */
void timer_intr(sdla_t *card)
{
        struct net_device* dev=NULL;
        private_area_t* priv_area = NULL;
        SHARED_MEMORY_INFO_STRUCT* flags = get_card_flags(card);

	/* TE timer interrupt */
	if (card->timer_int_enabled & TMR_INT_ENABLED_TE) {
		card->wandev.fe_iface.polling(&card->fe);
		card->timer_int_enabled &= ~TMR_INT_ENABLED_TE;
	}

        if ((dev = card->wandev.dev)==NULL){
		card->timer_int_enabled=0;
		goto timer_isr_exit;
	}
	
        priv_area = dev->priv;

	/* Configure hardware */
	if (card->timer_int_enabled & TMR_INT_ENABLED_CONFIG) {
		config_frmw(card);
		card->timer_int_enabled &= ~TMR_INT_ENABLED_CONFIG;
	}
	
	/* process a udp call if pending */
       	if(card->timer_int_enabled & TMR_INT_ENABLED_UDP) {
               	process_udp_mgmt_pkt(card, dev,
                       priv_area,0);
		card->timer_int_enabled &= ~TMR_INT_ENABLED_UDP;
        }

	/* read the communications statistics if required */
	if(card->timer_int_enabled & TMR_INT_ENABLED_UPDATE) {
		update_comms_stats(card, priv_area);
                if(!(-- card->update_comms_stats)) {
			card->timer_int_enabled &= 
				~TMR_INT_ENABLED_UPDATE;
		}
        }

timer_isr_exit:

	/* only disable the timer interrupt if there are no udp or statistic */
	/* updates pending */
        if(!card->timer_int_enabled) {
                flags = get_card_flags(card);
                flags->interrupt_info_struct.interrupt_permission &=
                        ~APP_INT_ON_TIMER;
        }
}

/*============================================================================
 * Process global exception condition
 */
static int process_global_exception(sdla_t *card)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	int err;

	mb->wan_data_len = 0;
	mb->wan_command = READ_GLOBAL_EXCEPTION_CONDITION;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;

	if(err != CMD_TIMEOUT ){
	
		switch(mb->wan_return_code) {
         
	      	case EXCEP_MODEM_STATUS_CHANGE:

			if (IS_56K_CARD(card)) {

				SHARED_MEMORY_INFO_STRUCT *flags = get_card_flags(card);
				FRONT_END_STATUS_STRUCT *FE_status =
				(FRONT_END_STATUS_STRUCT *)&flags->FT1_info_struct.parallel_port_A_input;
				card->wandev.RR8_reg_56k = 
					FE_status->FE_U.stat_56k.RR8_56k;	
				card->wandev.RRA_reg_56k = 
					FE_status->FE_U.stat_56k.RRA_56k;	
				card->wandev.RRC_reg_56k = 
					FE_status->FE_U.stat_56k.RRC_56k;	
				sdla_56k_alarm(card, 0);

				handle_front_end_state(card);
				break;
			
			}
			
			if (IS_TE1_CARD(card)) {
				/* TE1 T1/E1 interrupt */
				card->wandev.fe_iface.isr(&card->fe);
				handle_front_end_state(card);
				break;
			}	

			if ((mb->wan_data[0] & (DCD_HIGH | CTS_HIGH)) == (DCD_HIGH | CTS_HIGH)){
				card->wandev.front_end_status = FE_CONNECTED;
			}else{
				card->wandev.front_end_status = FE_DISCONNECTED;
			}
			
			DEBUG_EVENT( "%s: Modem status change\n",
				card->devname);

			switch(mb->wan_data[0] & (DCD_HIGH | CTS_HIGH)) {
				
				case ((DCD_HIGH | CTS_HIGH)):
                                        DEBUG_EVENT( "%s: DCD high, CTS high\n",card->devname);
                                        break;
				
				case (DCD_HIGH):
					DEBUG_EVENT( "%s: DCD high, CTS low\n",card->devname);
					break;
					
				case (CTS_HIGH):
                                        DEBUG_EVENT( "%s: DCD low, CTS high\n",card->devname); 
					break;
				
				default:
                                        DEBUG_EVENT( "%s: DCD low, CTS low\n",card->devname);
                                        break;
			}

			handle_front_end_state(card);
			break;

#if 0
                case EXCEP_TRC_DISABLED:
                        DEBUG_EVENT( "%s: Line trace disabled\n",
				card->devname);
                        break;

		case EXCEP_IRQ_TIMEOUT:
			DEBUG_EVENT( "%s: IRQ timeout occurred\n",
				card->devname); 
			break;
#endif

                default:
                        DEBUG_EVENT( "%s: Global exception %x\n",
				card->devname, mb->wan_return_code);
                        break;
                }
	}
	return 0;
}


/*============================================================================
 * Process  exception condition
 */
static int process_exception(sdla_t *card)
{
	wan_mbox_t* mb = get_card_mailbox(card);
	int err;

	mb->wan_data_len = 0;
	mb->wan_command = READ_EXCEPTION_CONDITION;
	err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;

	if(err != CMD_TIMEOUT) {
	
		switch (err) {

#if 0
		case EXCEP_LINK_ACTIVE:
			card->u.c.state = WAN_CONNECTED;
			if (card->wandev.ignore_front_end_status == WANOPT_YES ||
			    card->wandev.front_end_status == FE_CONNECTED){
				port_set_state(card, WAN_CONNECTED);
				trigger_poll(card->wandev.dev);
			}
			break;

		case EXCEP_LINK_INACTIVE_MODEM:
			card->u.c.state = WAN_DISCONNECTED;
			port_set_state(card, WAN_DISCONNECTED);
			unconfigure_ip(card);
			trigger_poll(card->wandev.dev);
			break;

		case EXCEP_LINK_INACTIVE_KPALV:
			card->u.c.state = WAN_DISCONNECTED;
			port_set_state(card, WAN_DISCONNECTED);
			DEBUG_EVENT( "%s: Keepalive timer expired.\n",
				 		card->devname);
			unconfigure_ip(card);
			trigger_poll(card->wandev.dev);
			break;

		case EXCEP_IP_ADDRESS_DISCOVERED:
			if (configure_ip(card)) 
				return -1;
			break;
			
		case EXCEP_LOOPBACK_CONDITION:
			DEBUG_EVENT( "%s: Loopback Condition Detected.\n",
						card->devname);
			break;

		case NO_EXCEP_COND_TO_REPORT:
			DEBUG_EVENT( "%s: No exceptions reported.\n",
						card->devname);
			break;
		}
#endif
	}
	return 0;
}




/**SECTION***********************************************************
 * 
 * WANPIPE Debugging Interfaces
 *
 ********************************************************************/



/*=============================================================================
 * store_udp_mgmt_pkt
 *
 * Store a UDP management packet for later processing.
 *
 * When isr or tx task receives a UDP debug packet from 
 * wanpipemon utility, or from the remote network, the 
 * debug process must be defered to a kernel task 
 * which will execute the debug command at a later time. 
 *
 */

static int store_udp_mgmt_pkt(char udp_pkt_src, sdla_t* card,
                                struct sk_buff *skb, struct net_device* dev,
                                private_area_t* priv_area )
{
	int udp_pkt_stored = 0;

	if(!atomic_read(&priv_area->udp_pkt_len) &&
	  (skb->len <= MAX_LGTH_UDP_MGNT_PKT)) {
		atomic_set(&priv_area->udp_pkt_len, skb->len);
		priv_area->udp_pkt_src = udp_pkt_src;
       		memcpy(priv_area->udp_pkt_data, skb->data, skb->len);
		card->timer_int_enabled = TMR_INT_ENABLED_UDP;
		udp_pkt_stored = 1;
	}

	wan_skb_free(skb);
		
	return(udp_pkt_stored);
}


/*=============================================================================
 * process_udp_mgmt_pkt
 * 
 * Process all "wanpipemon" debugger commands.  This function
 * performs all debugging tasks: 
 *
 * 	Line Tracing
 * 	Line/Hardware Statistics
 * 	Protocol Statistics
 * 
 * "wanpipemon" utility is a user-space program that 
 * is used to debug the WANPIPE product.
 *
 */

static int process_udp_mgmt_pkt(sdla_t* card, struct net_device* dev,
				private_area_t* priv_area, int local_dev ) 
{
	unsigned char *buf;
	unsigned int frames, len;
	struct sk_buff *new_skb;
	unsigned short buffer_length, real_len;
	volatile unsigned long data_ptr;
	unsigned data_length;
	int udp_mgmt_req_valid = 1;
	wan_mbox_t *mb = get_card_mailbox(card);
	SHARED_MEMORY_INFO_STRUCT *flags = get_card_flags(card);
	wan_udp_pkt_t *wan_udp_pkt;
	struct timeval tv;
	int err;
	char ut_char;

	wan_udp_pkt = (wan_udp_pkt_t *) priv_area->udp_pkt_data;

	if (!local_dev){

		if(priv_area->udp_pkt_src == UDP_PKT_FRM_NETWORK){

			/* Only these commands are support for remote debugging.
			 * All others are not */
			switch(wan_udp_pkt->wan_udp_command) {

				case READ_GLOBAL_STATISTICS:
				case READ_MODEM_STATUS:  
				case READ_LINK_STATUS:
				case ROUTER_UP_TIME:
				case READ_COMMS_ERROR_STATS:
				case READ_OPERATIONAL_STATS:
				/* These two commands are executed for
				 * each request */
				case READ_CONFIGURATION:
				case READ_CODE_VERSION:
				case WAN_GET_MEDIA_TYPE:
				case WAN_FE_GET_STAT:
					udp_mgmt_req_valid = 1;
					break;
				default:
					udp_mgmt_req_valid = 0;
					break;
			} 
		}
	}

  	if(!udp_mgmt_req_valid) {

		/* set length to 0 */
		wan_udp_pkt->wan_udp_data_len = 0;

    		/* set return code */
		wan_udp_pkt->wan_udp_return_code = 0xCD;

		if (net_ratelimit()){	
			DEBUG_EVENT( 
			"%s: Warning, Illegal UDP command attempted from network: %x\n",
			card->devname,wan_udp_pkt->wan_udp_command);
		}

   	} else {
	   	unsigned long trace_status_cfg_addr = 0;
		TRACE_STATUS_EL_CFG_STRUCT trace_cfg_struct;
		TRACE_STATUS_ELEMENT_STRUCT trace_element_struct;

		wan_udp_pkt->wan_udp_opp_flag = 0;

		switch(wan_udp_pkt->wan_udp_command) {

		case ENABLE_TRACING:
		     if (!priv_area->TracingEnabled) {

			/* OPERATE_DATALINE_MONITOR */
			mb->wan_data_len = sizeof(LINE_TRACE_CONFIG_STRUCT);
			mb->wan_command = SET_TRACE_CONFIGURATION;

    			((LINE_TRACE_CONFIG_STRUCT *)mb->wan_data)->
				trace_config = TRACE_ACTIVE;
			/* Trace delay mode is not used because it slows
			   down transfer and results in a standoff situation
			   when there is a lot of data */

			/* Configure the Trace based on user inputs */
			((LINE_TRACE_CONFIG_STRUCT *)mb->wan_data)->trace_config |= 
					wan_udp_pkt->wan_udp_data[0];

			((LINE_TRACE_CONFIG_STRUCT *)mb->wan_data)->
			   trace_deactivation_timer = 4000;


			err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
			if (err != COMMAND_OK) {
				frmw_error(card,err,mb);
				card->TracingEnabled = 0;
				wan_udp_pkt->wan_udp_return_code = err;
				mb->wan_data_len = 0;
				break;
	    		} 

			/* Get the base address of the trace element list */
			mb->wan_data_len = 0;
			mb->wan_command = READ_TRACE_CONFIGURATION;
			err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;

			if (err != COMMAND_OK) {
				frmw_error(card,err,mb);
				priv_area->TracingEnabled = 0;
				wan_udp_pkt->wan_udp_return_code = err;
				mb->wan_data_len = 0;
				break;
	    		} 	

	   		trace_status_cfg_addr =((LINE_TRACE_CONFIG_STRUCT *)
				mb->wan_data) -> ptr_trace_stat_el_cfg_struct;

			sdla_peek(&card->hw, trace_status_cfg_addr,
				 &trace_cfg_struct, sizeof(trace_cfg_struct));
		    
			priv_area->start_trace_addr = trace_cfg_struct.
				base_addr_trace_status_elements;

			priv_area->number_trace_elements = 
					trace_cfg_struct.number_trace_status_elements;

			priv_area->end_trace_addr = (unsigned long)
					((TRACE_STATUS_ELEMENT_STRUCT *)
					 priv_area->start_trace_addr + 
					 (priv_area->number_trace_elements - 1));

			priv_area->base_addr_trace_buffer = 
					trace_cfg_struct.base_addr_trace_buffer;

			priv_area->end_addr_trace_buffer = 
					trace_cfg_struct.end_addr_trace_buffer;

		    	priv_area->curr_trace_addr = 
					trace_cfg_struct.next_trace_element_to_use;

	    		priv_area->available_buffer_space = 2000 - 
								  sizeof(struct iphdr) -
								  sizeof(struct udphdr) -
							      	  sizeof(wan_mgmt_t)-
								  sizeof(wan_cmd_t)-
								  sizeof(wan_trace_info_t);	
	       	     }

		     wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
		     mb->wan_data_len = 0;
	       	     priv_area->TracingEnabled = 1;
	       	     break;
	   

		case DISABLE_TRACING:
		     if (priv_area->TracingEnabled) {

			/* OPERATE_DATALINE_MONITOR */
			mb->wan_data_len = sizeof(LINE_TRACE_CONFIG_STRUCT);
			mb->wan_command = SET_TRACE_CONFIGURATION;
    			((LINE_TRACE_CONFIG_STRUCT *)mb->wan_data)->
				trace_config = TRACE_INACTIVE;
			err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
		     }		

		     priv_area->TracingEnabled = 0;
		     wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
		     mb->wan_data_len = 0;
		     break;
	   

		case GET_TRACE_INFO:

		     if (!priv_area->TracingEnabled) {
			wan_udp_pkt->wan_udp_return_code = 1;
			mb->wan_data_len = 0;
			break;
		     }

  		     wan_udp_pkt->wan_udp_ismoredata = 0x00;
		     buffer_length = 0;	/* offset of packet already occupied */

		     for (frames=0; frames < priv_area->number_trace_elements; frames++){

			trace_pkt_t *trace_pkt = (trace_pkt_t *)
				&wan_udp_pkt->wan_udp_data[buffer_length];

			sdla_peek(&card->hw, priv_area->curr_trace_addr,
			   	  (unsigned char *)&trace_element_struct,
			   	  sizeof(TRACE_STATUS_ELEMENT_STRUCT));

     			if (trace_element_struct.opp_flag == 0x00) {
			 	break;
			}

			/* get pointer to real data */
			data_ptr = trace_element_struct.ptr_data_bfr;

			/* See if there is actual data on the trace buffer */
			if (data_ptr){
				data_length = trace_element_struct.trace_length;
			}else{
				data_length = 0;
				wan_udp_pkt->wan_udp_ismoredata = 0x01;
			}
	
   			if( (priv_area->available_buffer_space - buffer_length)
				< ( sizeof(trace_pkt_t) + data_length) ) {

                            /* indicate there are more frames on board & exit */
				wan_udp_pkt->wan_udp_ismoredata = 0x01;
                               	break;
                         }

			trace_pkt->status = trace_element_struct.trace_type;

			trace_pkt->time_stamp =
				trace_element_struct.trace_time_stamp;

			trace_pkt->real_length =
				trace_element_struct.trace_length;

			/* see if we can fit the frame into the user buffer */
			real_len = trace_pkt->real_length;

			if (data_ptr == 0) {
			     	trace_pkt->data_avail = 0x00;
			} else {
				unsigned tmp = 0;

				/* get the data from circular buffer
				    must check for end of buffer */
			        trace_pkt->data_avail = 0x01;

				if ((data_ptr + real_len) >
					     priv_area->end_addr_trace_buffer + 1){

				    	tmp = priv_area->end_addr_trace_buffer - data_ptr + 1;
				    	sdla_peek(&card->hw, data_ptr,
					       	  trace_pkt->data,tmp);
				    	data_ptr = priv_area->base_addr_trace_buffer;
				}
	
		        	sdla_peek(&card->hw, data_ptr,
					  &trace_pkt->data[tmp], real_len - tmp);
			}	

			/* zero the opp flag to show we got the frame */
			ut_char = 0x00;
			sdla_poke(&card->hw, priv_area->curr_trace_addr, &ut_char, 1);

       			/* now move onto the next frame */
       			priv_area->curr_trace_addr += sizeof(TRACE_STATUS_ELEMENT_STRUCT);

       			/* check if we went over the last address */
			if ( priv_area->curr_trace_addr > priv_area->end_trace_addr ) {
				priv_area->curr_trace_addr = priv_area->start_trace_addr;
       			}

            		if(trace_pkt->data_avail == 0x01) {
				buffer_length += real_len - 1;
			}
	 
	       	    	/* for the header */
	            	buffer_length += sizeof(trace_pkt_t);

		     }  /* For Loop */

		     if (frames == priv_area->number_trace_elements){
			wan_udp_pkt->wan_udp_ismoredata = 0x01;
	             }
 		     wan_udp_pkt->wan_udp_num_frames = frames;
		 
    		     mb->wan_data_len = buffer_length;
		     wan_udp_pkt->wan_udp_data_len = buffer_length; 
		 
		     wan_udp_pkt->wan_udp_return_code = COMMAND_OK; 
		     
		     break;


		case FT1_READ_STATUS:
			((unsigned char *)wan_udp_pkt->wan_udp_data )[0] =
				flags->FT1_info_struct.parallel_port_A_input;

			((unsigned char *)wan_udp_pkt->wan_udp_data )[1] =
				flags->FT1_info_struct.parallel_port_B_input;
				
			wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
			wan_udp_pkt->wan_udp_data_len = 2;
			mb->wan_data_len = 2;
			break;

		case ROUTER_UP_TIME:
			do_gettimeofday( &tv );
			priv_area->router_up_time = tv.tv_sec - 
					priv_area->router_start_time;
			*(unsigned long *)&wan_udp_pkt->wan_udp_data = 
					priv_area->router_up_time;	
			mb->wan_data_len = sizeof(unsigned long);
			wan_udp_pkt->wan_udp_data_len = sizeof(unsigned long);
			wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
			break;

   		case FT1_MONITOR_STATUS_CTRL:
			/* Enable FT1 MONITOR STATUS */
	        	if ((wan_udp_pkt->wan_udp_data[0] & ENABLE_READ_FT1_STATUS) ||  
				(wan_udp_pkt->wan_udp_data[0] & ENABLE_READ_FT1_OP_STATS)) {
			
			     	if( rCount++ != 0 ) {
					wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
					mb->wan_data_len = 1;
		  			break;
		    	     	}
	      		}

	      		/* Disable FT1 MONITOR STATUS */
	      		if( wan_udp_pkt->wan_udp_data[0] == 0) {

	      	   	     	if( --rCount != 0) {
		  			wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
					mb->wan_data_len = 1;
		  			break;
	   	    	     	} 
	      		} 	
			goto dflt_1;

		case WAN_GET_MEDIA_TYPE:
		case WAN_FE_GET_STAT:
		case WAN_FE_SET_LB_MODE:
 		case WAN_FE_FLUSH_PMON:
		case WAN_FE_GET_CFG:
			if (IS_TE1_CARD(card)){
				card->wandev.fe_iface.process_udp(
						&card->fe, 
						&wan_udp_pkt->wan_udp_cmd,
						&wan_udp_pkt->wan_udp_data[0]);
			}else if (IS_56K_CARD(card)){
				card->wandev.fe_iface.process_udp(
						&card->fe, 
						&wan_udp_pkt->wan_udp_cmd,
						&wan_udp_pkt->wan_udp_data[0]);
			}else{
				if (wan_udp_pkt->wan_udp_command == WAN_GET_MEDIA_TYPE){
		    			wan_udp_pkt->wan_udp_data_len = sizeof(unsigned char); 
					wan_udp_pkt->wan_udp_return_code = CMD_OK;
				}else{
					wan_udp_pkt->wan_udp_return_code = WAN_UDP_INVALID_CMD;
				}
			}
			mb->wan_data_len = wan_udp_pkt->wan_udp_data_len;
			break;
#if 0
		case WAN_FE_SET_LB_MODE:
		    /* Activate/Deactivate Line Loopback modes */
		    if (IS_TE1_CARD(card)){
			err = sdla_te_set_lbmodes(
					&card->fe, 
					wan_udp_pkt->wan_udp_data[0], 
					wan_udp_pkt->wan_udp_data[1]);
			wan_udp_pkt->wan_udp_return_code = 
					(!err) ? CMD_OK : WAN_UDP_FAILED_CMD;
		    }else{
			wan_udp_pkt->wan_udp_return_code = WAN_UDP_INVALID_CMD;
		    }
		    wan_udp_pkt->wan_udp_data_len = 0x00;
		    break;

		case WAN_GET_MEDIA_TYPE:
	 		wan_udp_pkt->wan_udp_data[0] = 
				(IS_T1_CARD(card) ? WAN_MEDIA_T1 :
				 IS_E1_CARD(card) ? WAN_MEDIA_E1 :
				 IS_56K_CARD(card) ? WAN_MEDIA_56K : 
				 				WAN_MEDIA_NONE);
		    	wan_udp_pkt->wan_udp_data_len = sizeof(unsigned char); 
			wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
			mb->wan_data_len = sizeof(unsigned char);
			break;

		case WAN_FE_GET_STAT:
		    	if (IS_TE1_CARD(card)) {	
	 	    		/* TE1_56K Read T1/E1/56K alarms */
			  	*(unsigned long *)&wan_udp_pkt->wan_udp_data = 
		                        sdla_te_alarm(card, 0);
				/* TE1 Update T1/E1 perfomance counters */
    				sdla_te_pmon(card);
	        		memcpy(&wan_udp_pkt->wan_udp_data[sizeof(unsigned long)],
					&card->wandev.te_pmon,
					sizeof(sdla_te_pmon_t));
		        	wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
		    		wan_udp_pkt->wan_udp_data_len = 
					sizeof(unsigned long) + sizeof(sdla_te_pmon_t); 
				mb->wan_data_len = wan_udp_pkt->wan_udp_data_len;
			}else if (IS_56K_CARD(card)){
				/* 56K Update CSU/DSU alarms */
				card->wandev.k56_alarm = sdla_56k_alarm(card, 1); 
			 	*(unsigned long *)&wan_udp_pkt->wan_udp_data = 
			                        card->wandev.k56_alarm;
				wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
	    			wan_udp_pkt->wan_udp_data_len = sizeof(unsigned long);
				mb->wan_data_len = wan_udp_pkt->wan_udp_data_len;
			}
		    	break;

 		case WAN_FE_FLUSH_PMON:
 	    		/* TE1 Flush T1/E1 pmon counters */
	    		if (IS_TE1_CARD(card)){	
				card->wandev.fe_iface.flush_pmon(&card->fe);
	        		wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
	    		}
	    		break;

		case WAN_FE_GET_CFG:
			/* Read T1/E1 configuration */
			DEBUG_EVENT("Read T1/E1 configuration!\n");
	    		if (IS_TE1_CARD(card)){	
        			memcpy(&wan_udp_pkt->wan_udp_data[0],
					&card->wandev.te_cfg,
					sizeof(sdla_te_cfg_t));
		        	wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
	    			wan_udp_pkt->wan_udp_data_len = sizeof(sdla_te_cfg_t);
				mb->wan_data_len = wan_udp_pkt->wan_udp_data_len;
			}
			break;
#endif
			
		case WAN_GET_PROTOCOL:
		   	wan_udp_pkt->wan_udp_num_frames = card->wandev.config_id;
		    	wan_udp_pkt->wan_udp_return_code = CMD_OK;
		    	mb->wan_data_len = wan_udp_pkt->wan_udp_data_len = 1;
		    	break;

		case WAN_GET_PLATFORM:
		    	wan_udp_pkt->wan_udp_data[0] = WAN_LINUX_PLATFORM;
		    	wan_udp_pkt->wan_udp_return_code = CMD_OK;
		    	mb->wan_data_len = wan_udp_pkt->wan_udp_data_len = 1;
		    	break;

		default:
dflt_1:
			/* it's a board command */
			mb->wan_command = wan_udp_pkt->wan_udp_command;
			mb->wan_data_len = wan_udp_pkt->wan_udp_data_len;
			if (mb->wan_data_len) {
				memcpy(&mb->wan_data, (unsigned char *) wan_udp_pkt->
							wan_udp_data, mb->wan_data_len);
	      		} 

			/* run the command on the board */
			err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
			if (err != COMMAND_OK) {
				frmw_error(card,err,mb);
				wan_udp_pkt->wan_udp_return_code = mb->wan_return_code;
				break;
			}
			
			/* copy the result back to our buffer */
	         	memcpy(&wan_udp_pkt->wan_udp_hdr.wan_cmd, &mb->wan_command, sizeof(wan_cmd_t)); 
			
			if (mb->wan_data_len) {
	         		memcpy(&wan_udp_pkt->wan_udp_data, &mb->wan_data, 
								mb->wan_data_len); 
	      		}

		} /* end of switch */
     	} /* end of else */

     	/* Fill UDP TTL */
	wan_udp_pkt->wan_ip_ttl= card->wandev.ttl; 

	if (local_dev){
		wan_udp_pkt->wan_udp_request_reply = UDPMGMT_REPLY;
		return 1;
	}
	
	len = reply_udp(priv_area->udp_pkt_data, mb->wan_data_len);

     	if(priv_area->udp_pkt_src == UDP_PKT_FRM_NETWORK){

		/* Must check if we interrupted if_send() routine. The
		 * tx buffers might be used. If so drop the packet */
	   	if (!test_bit(SEND_CRIT,&card->wandev.critical)) {
		
			if(!frmw_send(card, priv_area->udp_pkt_data, len, 0)) {
				++ chan->if_stats.tx_packets;
				chan->if_stats.tx_bytes += len;
			}
		}
	} else {	
	
		/* Pass it up the stack
    		   Allocate socket buffer */
		if ((new_skb = wan_skb_alloc(len)) != NULL) {
			/* copy data into new_skb */

 	    		buf = wan_skb_put(new_skb, len);
  	    		memcpy(buf, priv_area->udp_pkt_data, len);

            		/* Decapsulate pkt and pass it up the protocol stack */
	    		new_skb->protocol = htons(ETH_P_IP);
            		new_skb->dev = dev;
	    		new_skb->mac.raw  = new_skb->data;

			netif_rx(new_skb);
		} else {
	    	
			DEBUG_EVENT( "%s: no socket buffers available!\n",
					card->devname);
  		}
    	}

	atomic_set(&priv_area->udp_pkt_len,0);
 	
	return 0;
}

/*==============================================================================
 * udp_pkt_type
 * 
 * Determine what type of UDP call it is. ?
 *
 */
static int udp_pkt_type(struct sk_buff *skb, sdla_t* card)
{
	 wan_udp_pkt_t *wan_udp_pkt = (wan_udp_pkt_t *)skb->data;

#ifdef _WAN_UDP_DEBUG
		DEBUG_EVENT( "SIG %s = %s\n\
				  UPP %x = %x\n\
				  PRT %x = %x\n\
				  REQ %i = %i\n\
				  36 th = %x 37th = %x\n",
				  wan_udp_pkt->wan_udp_signature,
				  UDPMGMT_SIGNATURE,
				  wan_udp_pkt->wan_udp_dport,
				  ntohs(card->wandev.udp_port),
				  wan_udp_pkt->wan_ip_p,
				  UDPMGMT_UDP_PROTOCOL,
				  wan_udp_pkt->wan_udp_request_reply,
				  UDPMGMT_REQUEST,
				  skb->data[36], skb->data[37]);
#endif	
		
	if ((wan_udp_pkt->wan_udp_dport == ntohs(card->wandev.udp_port)) &&
	   (wan_udp_pkt->wan_ip_p == UDPMGMT_UDP_PROTOCOL) &&
	   (wan_udp_pkt->wan_udp_request_reply == UDPMGMT_REQUEST)) {
		if (!strncmp(wan_udp_pkt->wan_udp_signature,UDPMGMT_SIGNATURE,8)){
			return UDP_TYPE;
		}
		if (!strncmp(wan_udp_pkt->wan_udp_signature,GLOBAL_UDP_SIGNATURE,8)){
			return UDP_TYPE;
		}
	}
	return UDP_INVALID_TYPE;
}


/**SECTION*************************************************************
 *
 * 	TASK Functions and Triggers
 *
 **********************************************************************/


/*============================================================================
 * port_set_state
 * 
 * Set PORT state.
 *
 */
static void port_set_state (sdla_t *card, int state)
{
        if (card->wandev.state != state)
        {
                switch (state)
                {
                case WAN_CONNECTED:
                        DEBUG_EVENT( "%s: Link connected!\n",
                                card->devname);
                      	break;

                case WAN_CONNECTING:
                        DEBUG_EVENT( "%s: Link connecting...\n",
                                card->devname);
                        break;

                case WAN_DISCONNECTED:
                        DEBUG_EVENT( "%s: Link disconnected!\n",
                                card->devname);
                        break;
                }

                card->wandev.state = state;
		if (card->wandev.dev){
			struct net_device *dev = card->wandev.dev;
			private_area_t *priv_area = dev->priv;
			priv_area->common.state = state;

			if (priv_area->common.usedby == API){
				wakeup_sk_bh(dev);	
			}
		}
        }
}

/*===========================================================================
 * config_frmw
 *
 *	Configure the protocol and enable communications.		
 *
 *   	The if_open() function binds this function to the poll routine.
 *      Therefore, this function will run every time the interface
 *      is brought up. We cannot run this function from the if_open 
 *      because if_open does not have access to the remote IP address.
 *      
 *	If the communications are not enabled, proceed to configure
 *      the card and enable communications.
 *
 *      If the communications are enabled, it means that the interface
 *      was shutdown by either the user or driver. In this case, we 
 *      have to check that the IP addresses have not changed.  If
 *      the IP addresses have changed, we have to reconfigure the firmware
 *      and update the changed IP addresses.  Otherwise, just exit.
 *
 */

static int config_frmw (sdla_t *card)
{
	struct net_device *dev = card->wandev.dev;
	private_area_t *priv_area = dev->priv;
	SHARED_MEMORY_INFO_STRUCT *flags = get_card_flags(card);

	if (card->comm_enabled){

		/* Jun 20. 2000: NC
		 * IP addresses are not used in the API mode */
		
		if ((priv_area->ip_local_tmp != priv_area->ip_local ||
		     priv_area->ip_remote_tmp != priv_area->ip_remote) && 
		     priv_area->common.usedby == WANPIPE) {
			
			/* The IP addersses have changed, we must
                         * stop the communications and reconfigure
                         * the card. Reason: the firmware must know
                         * the local and remote IP addresses. */
			disable_comm(card);
			card->u.c.state = WAN_DISCONNECTED;
			port_set_state(card, WAN_DISCONNECTED);
			DEBUG_EVENT( 
				"%s: IP addresses changed!\n",
					card->devname);
			DEBUG_EVENT( 
				"%s: Restarting communications ...\n",
					card->devname);
		}else{ 
			/* IP addresses are the same and the link is up, 
                         * we dont have to do anything here. Therefore, exit */
			return 0;
		}
	}

	priv_area->ip_local = priv_area->ip_local_tmp;
	priv_area->ip_remote = priv_area->ip_remote_tmp;
	
	/* Setup the Board for */
	if (set_frmw_config(card)) {
		DEBUG_EVENT( "%s: Failed configuration!\n",
			card->devname);
		return 0;
	}

	flags->interrupt_info_struct.interrupt_type = 0;
	flags->interrupt_info_struct.interrupt_permission=0;
	
	if (IS_TE1_CARD(card)) {
		DEBUG_EVENT( "%s: Configuring onboard %s CSU/DSU\n",
			card->devname, 
			(card->wandev.te_cfg.media==WAN_MEDIA_T1)?"T1":"E1");
		if (sdla_te_config(&card->fe, &card->wandev.fe_iface)){
			DEBUG_EVENT( "%s: Failed %s configuratoin!\n",
					card->devname,
					(card->wandev.te_cfg.media==WAN_MEDIA_T1)?"T1":"E1");
			return -EINVAL;
		}
	}

	 
	if (IS_56K_CARD(card)) {
		DEBUG_EVENT( "%s: Configuring 56K onboard CSU/DSU\n",
			card->devname);

		if(sdla_56k_config(&card->fe, &card->wandev.fe_iface)){
			DEBUG_EVENT( "%s: Failed 56K configuration!\n",
				card->devname);
			return -EINVAL;
		}
	}

	
	/* Set interrupt mode and mask */
        if (frmw_set_intr_mode(card, APP_INT_ON_RX_FRAME |
                		APP_INT_ON_GLOBAL_EXCEP_COND |
                		APP_INT_ON_TX_FRAME |
                		APP_INT_ON_EXCEP_COND | APP_INT_ON_TIMER)){
		DEBUG_EVENT( "%s: Failed to set interrupt triggers!\n",
				card->devname);
		return -EINVAL;	
        }
	
	/* Mask All interrupts  */
	flags->interrupt_info_struct.interrupt_permission &= 
		~(APP_INT_ON_RX_FRAME | APP_INT_ON_TX_FRAME | 
		  APP_INT_ON_TIMER    | APP_INT_ON_GLOBAL_EXCEP_COND | 
		  APP_INT_ON_EXCEP_COND);


	if (frmw_comm_enable(card) != 0) {
		DEBUG_EVENT( "%s: Failed to enable communications!\n",
				card->devname);
		flags->interrupt_info_struct.interrupt_permission = 0;
		card->comm_enabled=0;
		frmw_set_intr_mode(card,0);
		return -EINVAL;
	}

	/* Initialize Rx/Tx buffer control fields */
	init_tx_rx_buff(card);
	card->u.c.state = WAN_CONNECTING;
	port_set_state(card, WAN_CONNECTING);

	/* Manually poll the 56K CSU/DSU to get the status */
	if (IS_56K_CARD(card)) {
		/* 56K Update CSU/DSU alarms */
		sdla_56k_alarm(card, 1);
	}

	/* Unmask all interrupts except the Transmit and Timer interrupts */
	flags->interrupt_info_struct.interrupt_permission |= 
		(APP_INT_ON_RX_FRAME | APP_INT_ON_GLOBAL_EXCEP_COND | 
		APP_INT_ON_EXCEP_COND);

	flags->interrupt_info_struct.interrupt_type = 0;

	return 0; 
}


/*============================================================
 * frmw_poll
 *	
 * Rationale:
 * 	We cannot manipulate the routing tables, or
 *      ip addresses withing the interrupt. Therefore
 *      we must perform such actons outside an interrupt 
 *      at a later time. 
 *
 * Description:	
 *	polling routine, responsible for 
 *     	shutting down interfaces upon disconnect
 *     	and adding/removing routes. 
 *      
 * Usage:        
 * 	This function is executed for each   
 * 	interface through a tq_schedule bottom half.
 *      
 *      trigger_poll() function is used to kick
 *      the chldc_poll routine.  
 *
 *      if_open() calls the timer delay function which
 *      in turn calls the trigger_poll() to kick
 *      this task. 
 */

static void frmw_poll (struct net_device *dev)
{
	private_area_t *priv_area;
	sdla_t *card;
	u8 check_gateway=0;	
	SHARED_MEMORY_INFO_STRUCT* flags;

	
	if (!dev || (priv_area=dev->priv) == NULL){
		return;
	}

	card = priv_area->card;
	flags = get_card_flags(card);
	
	/* (Re)Configuraiton is in progress, stop what you are 
	 * doing and get out */
	if (test_bit(PERI_CRIT,&card->wandev.critical)){
		clear_bit(POLL_CRIT,&card->wandev.critical);
		return;
	}
	
	/* if_open() function has triggered the polling routine
	 * to determine the configured IP addresses.  Once the
	 * addresses are found, trigger the configuration */
	if (test_bit(0,&priv_area->config_frmw)){

		priv_area->ip_local_tmp  = get_ip_address(dev,WAN_LOCAL_IP);
		priv_area->ip_remote_tmp = get_ip_address(dev,WAN_POINTOPOINT_IP);
	
	       /* Jun 20. 2000 Bug Fix
	 	* Only perform this check in WANPIPE mode, since
	 	* IP addresses are not used in the API mode. */
	
		if (priv_area->ip_local_tmp == priv_area->ip_remote_tmp && 
		    !card->backup && 
		    priv_area->common.usedby == WANPIPE){

			if (++priv_area->ip_error > MAX_IP_ERRORS){
				DEBUG_EVENT( "\n");
				DEBUG_EVENT( "%s: --- WARNING ---\n",
						card->devname);
				DEBUG_EVENT( 
				"%s: The local IP address is the same as the\n",
						card->devname);
				DEBUG_EVENT( 
				"%s: Point-to-Point IP address.\n",
						card->devname);
				DEBUG_EVENT( "%s: --- WARNING ---\n\n",
						card->devname);
			}else{
				clear_bit(POLL_CRIT,&card->wandev.critical);
				priv_area->poll_delay_timer.expires = jiffies+HZ;
				add_timer(&priv_area->poll_delay_timer);
				return;
			}
		}

		clear_bit(0,&priv_area->config_frmw);
		clear_bit(POLL_CRIT,&card->wandev.critical);
		
		card->timer_int_enabled |= TMR_INT_ENABLED_CONFIG;
		flags->interrupt_info_struct.interrupt_permission |= APP_INT_ON_TIMER;
		return;
	}
	/* Dynamic interface implementation, as well as dynamic
	 * routing.  */
	
	switch (card->wandev.state){

	case WAN_DISCONNECTED:

		/* If the dynamic interface configuration is on, and interface 
		 * is up, then bring down the network interface */
		
		if (test_bit(DYN_OPT_ON,&priv_area->interface_down) && 
		    !test_bit(DEV_DOWN,  &priv_area->interface_down) &&		
		    card->wandev.dev->flags & IFF_UP){	

			DEBUG_EVENT( "%s: Interface %s down.\n",
				card->devname,card->wandev.dev->name);
			change_dev_flags(card->wandev.dev,(card->wandev.dev->flags&~IFF_UP));
			set_bit(DEV_DOWN,&priv_area->interface_down);
			priv_area->route_status = NO_ROUTE;

		}else{

#ifdef WANPIPE_ENABLE_DYNAMIC_IP_ADDRESSING
		       /* If the protocol is using dynamic ip addressing, then
		 	* remove IP addresses to the network interface
		 	* since protocol has gone down */

			/* We need to check if the local IP address is
               	  	 * zero. If it is, we shouldn't try to remove it.
                 	 */

			if (card->wandev.dev->flags & IFF_UP && 
		    	    get_ip_address(card->wandev.dev,WAN_LOCAL_IP) && 
		    	    priv_area->route_status != NO_ROUTE){

				process_route(card);
			}
#endif
		}
		break;

	case WAN_CONNECTED:

		/* In SMP machine this code can execute before the interface
		 * comes up.  In this case, we must make sure that we do not
		 * try to bring up the interface before dev_open() is finished */


		/* DEV_DOWN will be set only when we bring down the interface
		 * for the very first time. This way we know that it was us
		 * that brought the interface down */
		
		if (test_bit(DYN_OPT_ON,&priv_area->interface_down) &&
		    test_bit(DEV_DOWN,  &priv_area->interface_down) &&
		    !(card->wandev.dev->flags & IFF_UP)){
			
			DEBUG_EVENT( "%s: Interface %s up.\n",
				card->devname,card->wandev.dev->name);
			change_dev_flags(card->wandev.dev,(card->wandev.dev->flags|IFF_UP));
			clear_bit(DEV_DOWN,&priv_area->interface_down);
			check_gateway=1;
		}

#ifdef WANPIPE_ENABLE_DYNAMIC_IP_ADDRESSING

		/* If the protocol is using dynamic routing, then
		 * add the IP addresses to the network interface
		 * since the protocol has come up */

		if (chdlc_priv_area->route_status == ADD_ROUTE && 
		    card->u.c.slarp_timer){ 

			process_route(card);
			check_gateway=1;
		}
#endif

		if (priv_area->gateway && check_gateway)
			add_gateway(card,dev);

		break;
	}	

	clear_bit(POLL_CRIT,&card->wandev.critical);
}

/*============================================================
 * trigger_poll
 *
 * Description:
 * 	Add a frmw_poll() task into a tq_scheduler bh handler
 *      for a specific interface.  This will kick
 *      the fr_poll() routine at a later time. 
 *
 * Usage:
 * 	Interrupts use this to defer a taks to 
 *      a polling routine.
 *
 *      To execute tasks out of interrupt context.
 *
 */	
static void trigger_poll (struct net_device *dev)
{
	private_area_t *priv_area;
	sdla_t *card;

	if (!dev)
		return;
	
	if ((priv_area = dev->priv)==NULL)
		return;

	card = priv_area->card;
	
	if (test_and_set_bit(POLL_CRIT,&card->wandev.critical)){
		return;
	}
	if (test_bit(PERI_CRIT,&card->wandev.critical)){
		return; 
	}
#ifdef LINUX_2_4
	schedule_task(&priv_area->poll_task);
#else
	queue_task(&priv_area->poll_task, &tq_scheduler);
#endif
	return;
}

/*============================================================
 * frmw_poll_delay
 *
 * This is a kernel timer function used to implement
 * a delayed execution of a protocol poll task.
 *
 * The if_open() functions triggers the timer
 * that will run this function after a certain
 * delay.  After user defined delay, this function
 * will trigger the task.
 *
 * Reason: We MUST use the task to run polling commands
 *         because timer run in interrupt mode and we
 *         must not execute poll commands in interrupt mode!
 *
 *         if_open() function doesn't have interface IP
 *         information. Thus, we must not run the polling
 *         task too quickly because IP information might
 *         not be available yet.  
 *
 *         Thus we implement the delay to give kernel 
 *         enough time to bring the network inteface up,
 *         thus, when we run the poll task, we will have
 *         IP information available to us.
 *
 */ 

static void frmw_poll_delay (unsigned long dev_ptr)
{
	struct net_device *dev = (struct net_device *)dev_ptr;
	trigger_poll(dev);
}



/*============================================================
 * handle_front_end_state
 *
 * Front end state indicates the physical medium that
 * the Z80 backend connects to.  
 * 
 * S514-1/2/3:		V32/RS232/FT1 Front End
 * 	  		Front end state is determined via 
 * 	 		Modem/Status.
 * S514-4/5/7/8:	56K/T1/E1 Front End
 * 			Front end state is determined via
 * 			link status interrupt received
 * 			from the front end hardware.
 *
 * If the front end state handler is enabed by the
 * user.  The interface state will follow the 
 * front end state. I.E. If the front end goes down
 * the protocol and interface will be declared down.
 *
 * If the front end state is UP, then the interface
 * and protocol will be up ONLY if the protocol is
 * also UP.
 *
 * Therefore, we must have three state variables
 * 1. Front End State (card->wandev.front_end_status)
 * 2. Protocol State  (card->wandev.state)
 * 3. Interface State (dev->flags & IFF_UP)
 *
 */

static void handle_front_end_state(sdla_t *card)
{
	if (card->wandev.ignore_front_end_status == WANOPT_YES){
		return;
	}

	if (card->wandev.front_end_status == FE_CONNECTED){
		if (card->u.c.state == WAN_CONNECTED){
			port_set_state(card,WAN_CONNECTED);
			trigger_poll(card->wandev.dev);
		}
	}else{
		port_set_state(card,WAN_DISCONNECTED);
		trigger_poll(card->wandev.dev);
	}
}

/*============================================================
 * s508_lock and s508_unlock
 *
 * Used to lock and unlock critical code areas. By
 * default only Interrupt is allowed to execute
 * board commands.  If the non-interrupt process
 * tries to execute a command these locks must
 * be used to turn off the interrupts.
 *
 * Otherwise race conditions can occur between
 * the interrupt and non-interrupt kernel 
 * processes.
 *
 */

void s508_lock (sdla_t *card, unsigned long *smp_flags)
{
#if defined(CONFIG_SMP) || defined(LINUX_2_4)
	spin_lock_irqsave(&card->wandev.lock, *smp_flags);
        if (card->next){
        	spin_lock(&card->next->wandev.lock);
	}
#else
        disable_irq(card->hw.irq);
#endif                                                                     
}

void s508_unlock (sdla_t *card, unsigned long *smp_flags)
{
#if defined(CONFIG_SMP) || defined(LINUX_2_4)
        if (card->next){
        	spin_unlock(&card->next->wandev.lock);
        }
        spin_unlock_irqrestore(&card->wandev.lock, *smp_flags);
#else
        enable_irq(card->hw.irq);
#endif           
}



/**SECTION*******************************************************************
 *
 *  DYNAMIC IP ADDRESSING SUPPORT
 * 
 ****************************************************************************/




#ifdef WANPIPE_ENABLE_DYNAMIC_IP_ADDRESSING

/*============================================================================
 * configure_ip
 *  
 * Called by the ISR when a dynamic IP address has been 
 * obtained by the firmware.  This call will trigger an
 * event which will handle the newly obtained IP information.

 * Configure IP from SLARP negotiation
 * This adds dynamic routes when SLARP has provided valid addresses
 */

static int configure_ip (sdla_t* card)
{
	struct net_device *dev = card->wandev.dev;
        private_area_t *priv_area;
        char err;

	if (!dev)
		return 0;

	priv_area = dev->priv;
	
	
        /* set to discover */
        if(card->u.c.slarp_timer != 0x00) {
		wan_mbox_t* mb = get_card_mailbox(card);
		CONFIGURATION_STRUCT *cfg;

     		mb->wan_data_len = 0;
		mb->wan_command = READ_CONFIGURATION;
		err = sdla_exec(mb) ? mb->wan_return_code : CMD_TIMEOUT;
	
		if(err != COMMAND_OK) {
			frmw_error(card,err,mb);
			return -1;
		}

		cfg = (CONFIGURATION_STRUCT *)mb->wan_data;
                priv_area->IP_address = cfg->IP_address;
                priv_area->IP_netmask = cfg->IP_netmask;

		/* Set flag to add route */
		priv_area->route_status = ADD_ROUTE;

		/* The idea here is to add the route in the poll routine.
	   	This way, we aren't in interrupt context when adding routes */
		trigger_poll(dev);
        }

	return 0;
}


/*============================================================================
 * unconfigure_ip
 * 
 * Called by the ISR when a dynamic IP address has been 
 * changed or removed by the firmware/protocol.  
 * This call will trigger an event which will handle the 
 * newly obtained IP information.
 *
 * Un-Configure IP negotiated by SLARP
 * This removes dynamic routes when the link becomes inactive.
 *
 */

static int unconfigure_ip (sdla_t* card)
{
	struct net_device *dev = card->wandev.dev;
	private_area_t *priv_area;

	if (!dev)
		return 0;

	priv_area= dev->priv;
	
	if (priv_area->route_status == ROUTE_ADDED) {

		/* Note: If this function is called, the 
                 * port state has been DISCONNECTED.  This state
                 * change will trigger a poll_disconnected 
                 * function, that will check for this condition. 
		 */
		priv_area->route_status = REMOVE_ROUTE;

	}
	return 0;
}

/*============================================================================
 * process_route
 * 
 * This function should be used for protocols that support 
 * dynamic ip addressing, to dynamically change kernel
 * interface ip information. 
 * 
 * Routine to add/remove routes 
 * Called like a polling routine when 
 * Routes are flagged to be added/removed.
 */

static void process_route (sdla_t *card)
{
        struct net_device *dev = card->wandev.dev;
        unsigned char port_num;
        private_area_t *priv_area = NULL;
	u32 local_IP_addr = 0;
	u32 remote_IP_addr = 0;
	u32 IP_netmask, IP_addr;
        int err = 0;
	struct in_device *in_dev;
	mm_segment_t fs;
	struct ifreq if_info;
        struct sockaddr_in *if_data1, *if_data2;
	
        priv_area = dev->priv;
        port_num = card->wandev.comm_port;

	/* Bug Fix Mar 16 2000
	 * AND the IP address to the Mask before checking
         * the last two bits. */

	if((priv_area->route_status == ADD_ROUTE) &&
		((priv_area->IP_address & ~priv_area->IP_netmask) > 2)) {
		unsigned long tmp_ip;
		DEBUG_EVENT( "%s: Dynamic route failure.\n",card->devname);

		tmp_ip=ntohl(priv_area->IP_address);
                
		if(card->u.c.slarp_timer) {
			DEBUG_EVENT( "%s: Bad IP address %u.%u.%u.%u received\n",
				card->devname,
				NIPQUAD(tmp_ip));
                        DEBUG_EVENT( "%s: from remote station.\n",
				card->devname);

                }else{ 
                        DEBUG_EVENT( "%s: Bad IP address %u.%u.%u.%u issued\n",
				card->devname,
				NIPQUAD(tmp_ip));
                        DEBUG_EVENT( "%s: to remote station. Local\n",
				card->devname);
			DEBUG_EVENT( "%s: IP address must be A.B.C.1\n",
				card->devname);
			DEBUG_EVENT( "%s: or A.B.C.2.\n",card->devname);
		}

		/* remove the route due to the IP address error condition */
		priv_area->route_status = REMOVE_ROUTE;
		err = 1;
   	}

	/* If we are removing a route with bad IP addressing, then use the */
	/* locally configured IP addresses */
        if((priv_area->route_status == REMOVE_ROUTE) && err) {

 	        /* do not remove a bad route that has already been removed */
        	if(priv_area->route_removed) {
	                return;
        	}

                in_dev = dev->ip_ptr;

                if(in_dev != NULL) {
                        struct in_ifaddr *ifa = in_dev->ifa_list;
                        if (ifa != NULL ) {
                                local_IP_addr = ifa->ifa_local;
                                IP_netmask  = ifa->ifa_mask;
                        }
                }
	}else{ 
       		/* According to Cisco HDLC, if the point-to-point address is
		   A.B.C.1, then we are the opposite (A.B.C.2), and vice-versa.
		*/
		IP_netmask = ntohl(priv_area->IP_netmask);
	        remote_IP_addr = ntohl(priv_area->IP_address);
	

		/* If Netmask is 255.255.255.255 the local address
                 * calculation will fail. Default it back to 255.255.255.0 */
		if (IP_netmask == 0xffffffff)
			IP_netmask &= 0x00ffffff;

		/* Bug Fix Mar 16 2000
		 * AND the Remote IP address with IP netmask, instead
                 * of static netmask of 255.255.255.0 */
        	local_IP_addr = (remote_IP_addr & IP_netmask) +
                	(~remote_IP_addr & ntohl(0x0003));

	        if(!card->u.c.slarp_timer) {
			IP_addr = local_IP_addr;
			local_IP_addr = remote_IP_addr;
			remote_IP_addr = IP_addr;
       		}
	}

        fs = get_fs();                  /* Save file system  */
        set_fs(get_ds());               /* Get user space block */

        /* Setup a structure for adding/removing routes */
        memset(&if_info, 0, sizeof(if_info));
        strcpy(if_info.ifr_name, dev->name);

	switch (priv_area->route_status) {

	case ADD_ROUTE:

		if(!card->u.c.slarp_timer) {
			if_data2 = (struct sockaddr_in *)&if_info.ifr_dstaddr;
			if_data2->sin_addr.s_addr = remote_IP_addr;
			if_data2->sin_family = AF_INET;
			err = devinet_ioctl(SIOCSIFDSTADDR, &if_info);
		} else { 
			if_data1 = (struct sockaddr_in *)&if_info.ifr_addr;
			if_data1->sin_addr.s_addr = local_IP_addr;
			if_data1->sin_family = AF_INET;
			if(!(err = devinet_ioctl(SIOCSIFADDR, &if_info))){
				if_data2 = (struct sockaddr_in *)&if_info.ifr_dstaddr;
				if_data2->sin_addr.s_addr = remote_IP_addr;
				if_data2->sin_family = AF_INET;
				err = devinet_ioctl(SIOCSIFDSTADDR, &if_info);
			}
		}

               if(err) {
			DEBUG_EVENT( "%s: Add route %u.%u.%u.%u failed (%d)\n", 
				card->devname, NIPQUAD(remote_IP_addr), err);
		} else {
			((private_area_t *)dev->priv)->route_status = ROUTE_ADDED;
			DEBUG_EVENT( "%s: Dynamic route added.\n",
				card->devname);
			DEBUG_EVENT( "%s:    Local IP addr : %u.%u.%u.%u\n",
				card->devname, NIPQUAD(local_IP_addr));
			DEBUG_EVENT( "%s:    Remote IP addr: %u.%u.%u.%u\n",
				card->devname, NIPQUAD(remote_IP_addr));
			priv_area->route_removed = 0;
		}
		break;


	case REMOVE_ROUTE:
	
		/* Change the local ip address of the interface to 0.
		 * This will also delete the destination route.
		 */
		if(!card->u.c.slarp_timer) {
			if_data2 = (struct sockaddr_in *)&if_info.ifr_dstaddr;
			if_data2->sin_addr.s_addr = 0;
			if_data2->sin_family = AF_INET;
			err = devinet_ioctl(SIOCSIFDSTADDR, &if_info);
		} else {
			if_data1 = (struct sockaddr_in *)&if_info.ifr_addr;
			if_data1->sin_addr.s_addr = 0;
			if_data1->sin_family = AF_INET;
			err = devinet_ioctl(SIOCSIFADDR,&if_info);
		
		}
		if(err) {
			DEBUG_EVENT(
				"%s: Remove route %u.%u.%u.%u failed, (err %d)\n",
					card->devname, NIPQUAD(remote_IP_addr),
					err);
		} else {
			((private_area_t *)dev->priv)->route_status =
				NO_ROUTE;
                        DEBUG_EVENT( "%s: Dynamic route removed: %u.%u.%u.%u\n",
                                        card->devname, NIPQUAD(local_IP_addr)); 
			priv_area->route_removed = 1;
		}
		break;
	}

        set_fs(fs);                     /* Restore file system */

}

/* WANPIPE_ENABLE_DYNAMIC_IP_ADDRESSING */
#endif 




/**SECTION**********************************************************
 * 
 * 		PROC FILE SYSTEM SUPPORT 
 * 
 *******************************************************************/



#ifdef WANPIPE_ENABLE_PROC_FILE_HOOKS
#warning "Enabling Proc File System Hooks"


#define PROC_CFG_FRM	"%-15s| %-12s|\n"
#define PROC_STAT_FRM	"%-15s| %-12s| %-14s|\n"
static char config_hdr[] =
	"Interface name | Device name |\n";
static char status_hdr[] =
	"Interface name | Device name | Status        |\n";

static int get_config_info(void* priv, char* buf, int cnt, int len, int offs, int* stop_cnt) 
{
	private_area_t*	priv_area = priv;
	sdla_t*			card = NULL;
	int			size = 0;

	if (priv_area == NULL)
		return cnt;
	card = priv_area->card;

	if ((offs == 0 && cnt == 0) || (offs && offs == *stop_cnt)){
		PROC_ADD_LINE(cnt, (buf, &cnt, len, offs, stop_cnt, &size, "%s", config_hdr));
	}

	PROC_ADD_LINE(cnt, (buf, &cnt, len, offs, stop_cnt, &size,
			    PROC_CFG_FRM, priv_area->if_name, card->devname));
	return cnt;
}

static int get_status_info(void* priv, char* buf, int cnt, int len, int offs, int* stop_cnt)
{
	private_area_t*	priv_area = priv;
	sdla_t*			card = NULL;
	int			size = 0;

	if (priv_area == NULL)
		return cnt;
	card = priv_area->card;

	if ((offs == 0 && cnt == 0) || (offs && offs == *stop_cnt)){
		PROC_ADD_LINE(cnt, (buf, &cnt, len, offs, stop_cnt, &size, "%s", status_hdr));
	}

	PROC_ADD_LINE(cnt, (buf, &cnt, len, offs, stop_cnt, &size, PROC_STAT_FRM, 
			    priv_area->if_name, card->devname, STATE_DECODE(priv_area->common.state)));

	return cnt;
}

#define PROC_DEV_FR_S_FRM	"%-20s| %-14s|\n"
#define PROC_DEV_FR_D_FRM	"%-20s| %-14d|\n"
#define PROC_DEV_SEPARATE	"=====================================\n"

#ifdef LINUX_2_4
static int get_dev_config_info(char* buf, char** start, off_t offs, int len)
#else
static int get_dev_config_info(char* buf, char** start, off_t offs, int len, int dummy)
#endif
{
	int 		cnt = 0;
	wan_device_t*	wandev = (void*)start;
	sdla_t*		card = NULL;
	int 		size = 0;
	PROC_ADD_DECL(stop_cnt);

	if (wandev == NULL)
		return cnt;

	PROC_ADD_INIT(offs, stop_cnt);
	card = (sdla_t*)wandev->priv;

	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_SEPARATE));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, "Configuration for %s device\n",
		 wandev->name));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_SEPARATE));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_FR_S_FRM,
		 "Comm Port", COMPORT_DECODE(card->wandev.comm_port)));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_FR_S_FRM,
		 "Interface", INT_DECODE(wandev->interface)));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_FR_S_FRM,
		 "Clocking", CLK_DECODE(wandev->clocking)));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_FR_D_FRM,
		 "BaudRate",wandev->bps));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_FR_D_FRM,
		 "MTU", wandev->mtu));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_FR_D_FRM,
		 "UDP Port",  wandev->udp_port));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_FR_D_FRM,
		 "TTL", wandev->ttl));
	PROC_ADD_LINE(cnt, 
		(buf, &cnt, len, offs, &stop_cnt, &size, PROC_DEV_SEPARATE));

	PROC_ADD_RET(cnt, offs, stop_cnt);
}

static int set_dev_config(struct file *file, 
			     	const char *buffer,
			     	unsigned long count, 
			     	void *data)
{
	int 		cnt = 0;
	wan_device_t*	wandev = (void*)data;
	sdla_t*		card = NULL;

	if (wandev == NULL)
		return cnt;

	card = (sdla_t*)wandev->priv;

	DEBUG_EVENT( "%s: New device config (%s)\n",
			wandev->name, buffer);
	/* Parse string */

	return count;
}


#define PROC_IF_FR_S_FRM	"%-30s\t%-14s\n"
#define PROC_IF_FR_D_FRM	"%-30s\t%-14d\n"
#define PROC_IF_FR_L_FRM	"%-30s\t%-14ld\n"
#define PROC_IF_SEPARATE	"====================================================\n"

#ifdef LINUX_2_4
static int get_if_info(char* buf, char** start, off_t offs, int len)
#else
static int get_if_info(char* buf, char** start, off_t offs, int len, int dummy)
#endif
{
	int 			cnt = 0;
	struct net_device*	dev = (void*)start;
	private_area_t* 	priv_area = dev->priv;
	sdla_t*			card = priv_area->card;
	int 			size = 0;
	PROC_ADD_DECL(stop_cnt);

	goto get_if_info_end;
	PROC_ADD_INIT(offs, stop_cnt);
	/* Update device statistics */
	if (!offs && card->wandev.update) {
		int rslt = 0;
		rslt = card->wandev.update(&card->wandev);
		if(rslt) {
			switch (rslt) {
			case -EAGAIN:
				PROC_ADD_LINE(cnt, 
					(buf, &cnt, len, offs, &stop_cnt, &size, 
					 "Device is busy!\n"));
				break;

			default:
				PROC_ADD_LINE(cnt, 
					(buf, &cnt, len, offs, &stop_cnt, &size,
					 "Device is not configured!\n"));
				break;
			}
			goto get_if_info_end;
		}
	}

get_if_info_end:
	PROC_ADD_RET(cnt, offs, stop_cnt);
}
	
static int set_if_info(struct file *file, 
		       const char *buffer,
		       unsigned long count, 
		       void *data)
{
	struct net_device*	dev = (void*)data;
	private_area_t* 	priv_area = NULL;

	if (dev == NULL || dev->priv == NULL)
		return count;

	priv_area = (private_area_t*)dev->priv;


	DEBUG_EVENT( "%s: New interface config (%s)\n",
			priv_area->if_name, buffer);
	/* Parse string */

	return count;
}

/* WANPIPE_ENABLE_PROC_FILE_HOOKS */
#endif

/****** End ****************************************************************/
