/*************************************************************
 * wanpipe_lip.c   WANPIPE Link Interface Protocol Layer (LIP)
 *
 *
 *
 * ===========================================================
 *
 * Dec 02 2003	Nenad Corbic	Initial Driver
 */


/*=============================================================
 * Includes
 */

#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <net/wanpipe_lip.h>
#elif defined(__LINUX__)
#include <linux/wanpipe_lip.h>
#endif

/*=============================================================
 * Definitions
 */


/*=============================================================
 * Global Parameters
 */
/* Function interface between LIP layer and kernel */
extern wan_iface_t wan_iface;

struct wplip_link_list list_head_link;
wan_rwlock_t wplip_link_lock;
unsigned char wplip_link_num[MAX_LIP_LINKS];
#if 0
int gdbg_flag=0;
#endif

/*=============================================================
 * Function Prototypes
 */

static int wplip_if_unreg (netdevice_t *dev);
static int wplip_bind_link(void *lip_id,netdevice_t *dev);
static int wplip_unbind_link(void *lip_id,netdevice_t *dev);
static void wplip_kick(void *wplip_id,int reason);
static void wplip_disconnect(void *wplip_id,int reason);
static void wplip_connect(void *wplip_id,int reason);
static int wplip_rx(void *wplip_id, void *skb);
static int wplip_unreg(void *reg_ptr);

extern int register_wanpipe_lip_protocol (wplip_reg_t *lip_reg);
extern void unregister_wanpipe_lip_protocol (void);


/*=============================================================
 * Global Module Interface Functions
 */


/*=============================================================
 * wplip_register
 *
 * Description:
 *	
 * Usedby:
 */
/* EXPORT_SYMBOL(wplip_register); */
static int wplip_register (void **lip_link_ptr, wanif_conf_t *conf, char *devname)
{
	wplip_link_t *lip_link = (wplip_link_t*)*lip_link_ptr;

	/* Create the new X25 link for this Lapb 
	 * connection */
	if (lip_link){
		return -EEXIST;
	}

	lip_link = wplip_create_link(devname);
	if (!lip_link){
		DEBUG_EVENT("%s: LIP register: Failed to create link\n",
				MODNAME);
		return -ENOMEM;
	}
	
	DEBUG_TEST("%s: Registering LIP Link\n",lip_link->name);

	
	if (conf){
		int err=wplip_reg_link_prot(lip_link,conf);
		if (err){
			wplip_free_link(lip_link);	
			return err;
		}
	}
	
	wplip_insert_link(lip_link);

	lip_link->state = WAN_DISCONNECTED;
	lip_link->carrier_state = WAN_DISCONNECTED;

	*lip_link_ptr = lip_link;

	return 0;	
}

/*==============================================================
 * wplip_unreg
 *
 * Description:
 *	This function is called during system setup to 
 *	remove the whole x25 link and all x25 svc defined
 *	within the x25 link.
 *
 *	For each x25 link and x25 svc the proc file
 *	entry is removed.
 *	
 * Usedby:
 * 	Lapb layer. 
 */

static int wplip_unreg(void *lip_link_ptr)
{
	wplip_link_t *lip_link = (wplip_link_t*)lip_link_ptr;
	wplip_dev_t *lip_dev;
	wplip_dev_list_t *lip_dev_list_el; 
	int err;
	
	if (wplip_link_exists(lip_link) != 0){
		return -ENODEV;
	}

#ifdef WPLIP_TTY_SUPPORT
	if (lip_link->tty_opt && lip_link->tty_open){
		tty_hangup(lip_link->tty);
		return -EBUSY;
	}
#endif
	
	wan_del_timer(&lip_link->prot_timer);

	wan_set_bit(WPLIP_LINK_DOWN,&lip_link->tq_working);

	while((lip_dev = WAN_LIST_FIRST(&lip_link->list_head_ifdev)) != NULL){
		
		DEBUG_EVENT("%s: Unregistering dev %s\n",
				lip_link->name,lip_dev->name);
		
		err=wplip_if_unreg(lip_dev->common.dev);
		if (err<0){
			wan_clear_bit(WPLIP_LINK_DOWN,&lip_link->tq_working);
			return err;
		}
	}

	
	while((lip_dev_list_el = WAN_LIST_FIRST(&lip_link->list_head_tx_ifdev)) != NULL){
	
		DEBUG_EVENT("%s: Unregistering master dev %s\n",
				lip_link->name,
				wan_netif_name(lip_dev_list_el->dev));

		WAN_DEV_PUT(lip_dev_list_el->dev);
		lip_dev_list_el->dev=NULL;
		
		WAN_LIST_REMOVE(lip_dev_list_el,list_entry);
		lip_link->tx_dev_cnt--;

		wan_free(lip_dev_list_el);
		lip_dev_list_el=NULL;
	}


	if (lip_link->protocol){
		wplip_unreg_link_prot(lip_link);
	}
	
	wplip_remove_link(lip_link);
	wplip_free_link(lip_link);

	return 0;
}


static int wplip_if_reg(void *lip_link_ptr, char *dev_name, wanif_conf_t *conf)
{		
	wplip_link_t *lip_link = (wplip_link_t*)lip_link_ptr;
	wplip_dev_t *lip_dev;

#ifdef WPLIP_TTY_SUPPORT
	if (lip_link->tty_opt){
		return 0;
	}
#endif
	
	if (wplip_link_exists(lip_link) != 0){
		DEBUG_EVENT("%s: LIP: Invalid Link !\n",
				dev_name);
		return -EINVAL;
	}
		
	if (dev_name == NULL){
		DEBUG_EVENT("%s: LIP: Invalid device name : NULL!\n",
				lip_link->name);
		return -EINVAL;
	}

	if (!wplip_lipdev_exists(lip_link,dev_name)){
		DEBUG_EVENT("%s: LIP: Invalid lip link device %s!\n",
				__FUNCTION__,dev_name);
		return -EEXIST;
	}
	

	if ((lip_dev = wplip_create_lipdev(dev_name, conf->protocol)) == NULL){
		DEBUG_EVENT("%s: LIP: Failed to create lip priv device %s\n",
				lip_link->name,dev_name);
		return -ENOMEM;
	}

	WAN_HOLD(lip_link);
	lip_dev->lip_link 	= lip_link;
	lip_dev->common.usedby 	= WANPIPE;
	lip_dev->common.state	= WAN_DISCONNECTED;

#if defined(__LINUX__)
	if (conf->true_if_encoding){
		DEBUG_EVENT("%s: LIP: Setting IF Type to Broadcast\n",dev_name);
		lip_dev->common.dev->flags &= ~IFF_POINTOPOINT;		
		lip_dev->common.dev->flags |= IFF_BROADCAST;		
	}
#endif

	if (conf){
		int err;

		if(strcmp(conf->usedby, "API") == 0) {
			lip_dev->common.usedby = API;
			wan_reg_api(lip_dev, lip_dev->common.dev, lip_link->name);
			DEBUG_EVENT( "%s: Running in API mode\n",
					lip_dev->name);
		}else{
			lip_dev->common.usedby = WANPIPE;
			DEBUG_EVENT( "%s: Running in WANPIPE mode\n",
					lip_dev->name);
		}

		lip_dev->ipx_net_num = conf->network_number;

		DEBUG_EVENT("%s: IPX Network Number = 0x%lX\n",
				lip_dev->name, lip_dev->ipx_net_num);
		
		err=wplip_reg_lipdev_prot(lip_dev,conf);
		if (err){
			__WAN_PUT(lip_link);
			lip_dev->lip_link = NULL;
			wplip_free_lipdev(lip_dev);
			return err;
		}
	}
	
	wplip_insert_lipdev(lip_link,lip_dev);

	DEBUG_TEST("%s: LIP LIPDEV Created %p Magic 0x%lX\n",
			lip_link->name,
			lip_dev,
			lip_dev->magic);
	return 0;
}

/*==============================================================
 * wplip_if_unreg
 *
 * Description:
 *	This function is called during system setup to 
 *	remove the x25 link and each x25 svc defined
 *	in the wanpipe configuration file.
 *
 *	For each x25 link and x25 svc the proc file
 *	entry is removed.
 *	
 * Usedby:
 * 	Lapb layer. 
 */

static int wplip_if_unreg (netdevice_t *dev)
{
	wplip_dev_t *lip_dev = (wplip_dev_t *)wan_netif_priv(dev);
	wplip_link_t *lip_link = NULL;
	wan_smp_flag_t flags;

	if (!lip_dev)
		return -ENODEV;
	
	if (WAN_NETIF_UP(dev)){
		DEBUG_EVENT("%s: Failed to unregister: Device UP!\n",
				wan_netif_name(dev));
		return -EBUSY;
	}
	
	lip_link = lip_dev->lip_link;
	
	if (wplip_link_exists(lip_link) != 0){
		DEBUG_EVENT("%s: Failed to unregister: no link device\n",
				wan_netif_name(dev));
		return -ENODEV;
	}

	wan_set_bit(WPLIP_DEV_UNREGISTER,&lip_dev->critical);
	

	wan_spin_lock_irq(&lip_link->bh_lock,&flags);
	lip_link->cur_tx=NULL;
	wan_skb_queue_purge(&lip_dev->tx_queue);
	wan_spin_unlock_irq(&lip_link->bh_lock,&flags);
	
	DEBUG_EVENT("%s: Unregistering LIP device\n",
				wan_netif_name(dev));

	if (lip_dev->common.prot_ptr){
		wplip_unreg_lipdev_prot(lip_dev);	
	}

	if (lip_dev->common.usedby == API){
		wan_unreg_api(lip_dev, lip_link->name);
	}

	wplip_remove_lipdev(lip_link,lip_dev);
	
	__WAN_PUT(lip_dev->lip_link);
	lip_dev->lip_link=NULL;
	
	wplip_free_lipdev(lip_dev);
	
	return 0;
}



static int wplip_bind_link(void *lip_id,netdevice_t *dev)
{

	wplip_link_t *lip_link = (wplip_link_t*)lip_id;
	wplip_dev_list_t  *lip_dev_list_el;
	wan_smp_flag_t	flags;
	
	if (!lip_id){
		return -ENODEV;
	}

	if (wplip_link_exists(lip_link) != 0){
		return -ENODEV;
	}
	
	if (wan_test_bit(WPLIP_LINK_DOWN,&lip_link->tq_working)){
		return -ENETDOWN;
	}

	lip_dev_list_el=wan_malloc(sizeof(wplip_dev_list_t));
	memset(lip_dev_list_el,0,sizeof(wplip_dev_list_t));

	lip_dev_list_el->magic=WPLIP_MAGIC_DEV_EL;
	
	WAN_DEV_HOLD(dev);

	lip_dev_list_el->dev=dev;
	
	wan_spin_lock_irq(&lip_link->bh_lock,&flags);

	WAN_LIST_INSERT_HEAD(&lip_link->list_head_tx_ifdev,lip_dev_list_el,list_entry);
	lip_link->tx_dev_cnt++;

	wan_spin_unlock_irq(&lip_link->bh_lock,&flags);

	return 0;
}

static int wplip_unbind_link(void *lip_id,netdevice_t *dev)
{
	wplip_link_t *lip_link = (wplip_link_t*)lip_id;
	wplip_dev_list_t *lip_dev_list_el=NULL;
	wan_smp_flag_t	flags;
	int err=-ENODEV;
	
	if (!lip_id){
		return -EFAULT;
	}

	if (wplip_link_exists(lip_link) != 0){
		return -EFAULT;
	}
	

	wan_spin_lock_irq(&lip_link->bh_lock,&flags);

	WAN_LIST_FOREACH(lip_dev_list_el,&lip_link->list_head_tx_ifdev,list_entry){
		if (lip_dev_list_el->dev == dev){
			WAN_LIST_REMOVE(lip_dev_list_el,list_entry);
			lip_link->tx_dev_cnt--;
			err=0;
			break;
		}
	}	
	
	wan_spin_unlock_irq(&lip_link->bh_lock,&flags);

	if (err==0){
		WAN_DEV_PUT(lip_dev_list_el->dev);
		lip_dev_list_el->dev=NULL;
		wan_free(lip_dev_list_el);
	}
	
	return err;
}





/*==============================================================
 * wplip_rx
 *
 * Description:
 *
 *	
 * Usedby:
 * 	Lower layer to pass us an rx packet.
 */

static int wplip_rx(void *wplip_id, void *skb)
{
	wplip_link_t *lip_link = (wplip_link_t*)wplip_id;

	DEBUG_RX("%s: LIP LINK %s() pkt=%d %p\n",
			lip_link->name,__FUNCTION__,
			wan_skb_len(skb),
			skb);

	if (wan_test_bit(WPLIP_LINK_DOWN,&lip_link->tq_working)){
		return -ENODEV;
	}

#ifdef WPLIP_TTY_SUPPORT
	if (lip_link->tty_opt){
		if (lip_link->tty && lip_link->tty_open){
			wplip_tty_receive(lip_link,skb);		
			wanpipe_tty_trigger_poll(lip_link);
			return 0;
		}else{
			return -ENODEV;
		}
	}
#endif	
	
	if (wan_skb_queue_len(&lip_link->rx_queue) > MAX_RX_Q){
		DEBUG_TEST("%s: Critical Rx Error: Rx buf overflow 0x%lX!\n",
				lip_link->name,
				lip_link->tq_working);
		wplip_trigger_bh(lip_link);
		return -ENOBUFS;
	}
	
	wan_skb_queue_tail(&lip_link->rx_queue,skb);	
	wplip_trigger_bh(lip_link);
	
	return 0;
}

/*==============================================================
 * wplip_data_rx_up
 *
 * Description:
 *	This function is used to pass data to the upper 
 *	layer. If the lip dev is used by TCP/IP the packet
 *	is passed up the the TCP/IP stack, otherwise
 *	it is passed to the upper protocol layer.
 *	
 * Locking:
 * 	This function will ALWAYS get called from
 * 	the BH handler with the bh_lock, thus
 * 	its protected.
 *
 * Usedby:
 */

int wplip_data_rx_up(wplip_dev_t* lip_dev, void *skb)
{
	int len=wan_skb_len(skb);

	DEBUG_TEST("LIP LINK %s() pkt=%i\n",
			__FUNCTION__,wan_skb_len(skb));

#if 0
	DEBUG_EVENT("%s: %s() Packet Len=%d (DEBUG DROPPED)\n",
			lip_dev->name, __FUNCTION__,wan_skb_len(skb));			

	wan_skb_free(skb);
	return 0;
#endif

	switch (lip_dev->common.usedby){

#if defined(__LINUX__)	
	case API:
		{
		unsigned char *buf;
		int err;

		if (wan_skb_headroom(skb) < sizeof(wan_api_rx_hdr_t)){
			DEBUG_EVENT("%s: Critical Error Rx pkt Hdrm=%d  < ApiHdrm=%d\n",
					lip_dev->name,wan_skb_headroom(skb),
					sizeof(wan_api_rx_hdr_t));
			wan_skb_free(skb);
			break;
		}
		
		buf=wan_skb_push(skb,sizeof(wan_api_rx_hdr_t));	
		memset(buf,0,sizeof(wan_api_rx_hdr_t));
	
		((netskb_t *)skb)->protocol = htons(PVC_PROT);
		((netskb_t *)skb)->mac.raw  = wan_skb_data(skb);
		((netskb_t *)skb)->dev      = lip_dev->common.dev;
		((netskb_t *)skb)->pkt_type = WAN_PACKET_DATA;		

		if ((err=wan_api_rx(lip_dev,skb)) != 0){
#if 0
			if (net_ratelimit()){
				DEBUG_EVENT("%s: Error: Rx Socket busy err=%i!\n",
					lip_dev->name,err);
			}
#endif
			wan_skb_pull(skb,sizeof(wan_api_rx_hdr_t));
			return 1;
		}else{
			lip_dev->ifstats.rx_packets++;
			lip_dev->ifstats.rx_bytes+=len;
		}
		}
		break;
#endif	
	default:
		if (wan_iface.input && wan_iface.input(lip_dev->common.dev, skb) == 0){
			lip_dev->ifstats.rx_packets++;
			lip_dev->ifstats.rx_bytes += len;
		}else{
			wan_skb_free(skb);
			lip_dev->ifstats.rx_dropped++;
		}
			
		break;
	}
	return 0;
}


/*==============================================================
 * wplip_data_tx_down
 *
 * Description:
 *	This function is used to pass data down to the 
 *	lower layer. 
 *
 * Locking:
 * 	This function will ALWAYS get called from
 * 	the BH handler with the bh_lock, thus
 * 	its protected.
 *
 * Return Codes:
 *
 * 	0: Packet send successful, lower layer
 * 	   will deallocate packet
 *
 * 	Non 0: Packet send failed, upper layer
 * 	       must handle the packet
 *	
 */

int wplip_data_tx_down(wplip_link_t *lip_link, void *skb)
{
	wplip_dev_list_t *lip_dev_list_el; 	
	netdevice_t *dev;
	
	DEBUG_TEST("%s: LIP LINK %s() pkt=%d\n",
		lip_link->name,__FUNCTION__,wan_skb_len(skb));

	
	if (!lip_link->tx_dev_cnt){ 
		DEBUG_EVENT("%s: %s: Tx Dev List empty! dropping...\n",
				__FUNCTION__,lip_link->name);	
		return -ENODEV;
	}

	/* FIXME:
	 * For now, we can only transmit on a FIRST Tx device */
	lip_dev_list_el=WAN_LIST_FIRST(&lip_link->list_head_tx_ifdev);
	if (!lip_dev_list_el){
		DEBUG_EVENT("%s: %s: Tx Dev List empty! dropping...\n",
				__FUNCTION__,lip_link->name);	
		return -ENODEV;
	}
	
	if (lip_dev_list_el->magic != WPLIP_MAGIC_DEV_EL){
		DEBUG_EVENT("%s: %s: Error: Invalid dev magic number! dropping...\n",
				__FUNCTION__,lip_link->name);
		return -EFAULT;
	}

	dev=lip_dev_list_el->dev;
	if (!dev){
		DEBUG_EVENT("%s: %s: Error: No dev!  dropping...\n",
				__FUNCTION__,lip_link->name);
		return -ENODEV;
	}	

	if (WAN_NETIF_QUEUE_STOPPED(dev)){
		return -EBUSY;
	}

#if defined(__LINUX__)
	return dev->hard_start_xmit(skb,dev);
#else
 	if (dev->if_output) return dev->if_output(dev, skb, NULL,NULL);
	return 0;
#endif
}

/*==============================================================
 * wplip_link_prot_change_state
 *
 * Description:
 * 	The lowyer layer calls this function, when it
 * 	becomes disconnected.  
 */

int wplip_link_prot_change_state(void *wplip_id,int state, unsigned char *data, int len)
{
	wplip_link_t *lip_link = (wplip_link_t *)wplip_id;

	if (lip_link->prot_state != state){
		lip_link->prot_state=state;
	
		DEBUG_EVENT("%s: Lip Link Protocol State %s!\n",
			lip_link->name, STATE_DECODE(state));
		
		if (lip_link->prot_state == WAN_CONNECTED &&
		    lip_link->carrier_state == WAN_CONNECTED){
			DEBUG_EVENT("%s: Lip Link Connected!\n",
				lip_link->name);
			lip_link->state = WAN_CONNECTED;
		}

		if (lip_link->prot_state != WAN_CONNECTED){
			DEBUG_EVENT("%s: Lip Link Disconnected!\n",
				lip_link->name);
			lip_link->state = WAN_DISCONNECTED;
		}
	}
	
	return 0;
}

int wplip_lipdev_prot_change_state(void *wplip_id,int state, 
		                   unsigned char *data, int len)
{
	wplip_dev_t *lip_dev = (wplip_dev_t *)wplip_id;

	DEBUG_EVENT("%s: Lip Dev Prot State %s!\n",
		lip_dev->name,STATE_DECODE(state));

	lip_dev->common.state = state;

	if (lip_dev->common.usedby == API){

		if (data && len){
			wplip_prot_oob(lip_dev,data,len);
		}

		if (lip_dev->common.state == WAN_CONNECTED){
			WAN_NETIF_CARRIER_ON(lip_dev->common.dev);
			WAN_NETIF_START_QUEUE(lip_dev->common.dev);
			wan_update_api_state(lip_dev);
		}else{
			WAN_NETIF_CARRIER_OFF(lip_dev->common.dev);
			WAN_NETIF_STOP_QUEUE(lip_dev->common.dev);
		}

		wplip_trigger_bh(lip_dev->lip_link);
	}else{
		if (lip_dev->common.state == WAN_CONNECTED){
			WAN_NETIF_CARRIER_ON(lip_dev->common.dev);
			WAN_NETIF_WAKE_QUEUE(lip_dev->common.dev);
			wplip_trigger_bh(lip_dev->lip_link);
		}else{
			WAN_NETIF_CARRIER_OFF(lip_dev->common.dev);
			WAN_NETIF_STOP_QUEUE(lip_dev->common.dev);
		}
	}
	
	return 0;
}


/*==============================================================
 * wplip_connect
 *
 * Description:
 * 	The lowyer layer calls this function, when it
 * 	becomes connected.  
 */

static void wplip_connect(void *wplip_id,int reason)
{
	wplip_link_t *lip_link = (wplip_link_t *)wplip_id;

	if (lip_link->carrier_state != WAN_CONNECTED){
		wan_smp_flag_t flags;

		DEBUG_EVENT("%s: Lip Link Carrier Connected! \n",
			lip_link->name);

		wan_spin_lock_irq(&lip_link->bh_lock,&flags);
		wan_skb_queue_purge(&lip_link->tx_queue);
		wan_skb_queue_purge(&lip_link->rx_queue);
		wan_spin_unlock_irq(&lip_link->bh_lock,&flags);

		wplip_kick(lip_link,0);
		
		lip_link->carrier_state = WAN_CONNECTED;
	}
	
	if (lip_link->prot_state == WAN_CONNECTED){

		DEBUG_EVENT("%s: Lip Link Connected! \n",
			lip_link->name);

		lip_link->state = WAN_CONNECTED;
	}
}

/*==============================================================
 * wplip_disconnect
 *
 * Description:
 * 	The lowyer layer calls this function, when it
 * 	becomes disconnected.  
 */

static void wplip_disconnect(void *wplip_id,int reason)
{
	wplip_link_t *lip_link = (wplip_link_t *)wplip_id;

	if (lip_link->carrier_state != WAN_DISCONNECTED){
		DEBUG_EVENT("%s: Lip Link Carrier Disconnected!\n",
			lip_link->name);
		lip_link->carrier_state = WAN_DISCONNECTED;
	}

	/* state = carrier_state & prot_state 
	 * Therefore, the overall state is down */
	if (lip_link->state != WAN_DISCONNECTED){
		lip_link->state = WAN_DISCONNECTED;
		DEBUG_EVENT("%s: Lip Link Disconnected!\n",lip_link->name);
	}
}



/*==============================================================
 * wplip_kick
 *
 * Description:
 */

static void wplip_kick(void *wplip_id,int reason)
{
	wplip_link_t *lip_link = (wplip_link_t *)wplip_id;
	
	DEBUG_TEST("%s: LIP Kick!\n",
			lip_link->name);

#ifdef WPLIP_TTY_SUPPORT
	if (lip_link->tty_opt){ 
		wanpipe_tty_trigger_poll(lip_link);
	}else
#endif
	{	
		wan_clear_bit(WPLIP_BH_AWAITING_KICK,&lip_link->tq_working);
		wan_set_bit(WPLIP_KICK,&lip_link->tq_working);
		wplip_kick_trigger_bh(lip_link);
	}
}

#define INTERFACES_FRM	 "%-15s| %-12s| %-6u| %-18s|\n"
static int wplip_get_if_status(void *wplip_id, void *mptr)
{
#if defined(__LINUX__)
	wplip_link_t *lip_link = (wplip_link_t *)wplip_id;
	struct seq_file *m = (struct seq_file *)mptr;
	wplip_dev_t *cur_dev;
	unsigned long flag;

	WP_READ_LOCK(&lip_link->dev_list_lock,flag);

	WAN_LIST_FOREACH(cur_dev,&lip_link->list_head_ifdev,list_entry){
		PROC_ADD_LINE(m, 
			      INTERFACES_FRM,
			      cur_dev->name, lip_link->name, 
			      cur_dev->prot_addr,
			      STATE_DECODE(cur_dev->common.state));
	}

	WP_READ_UNLOCK(&lip_link->dev_list_lock,flag);

	return m->count;
#else
	return -EINVAL;
#endif
}


int wplip_link_callback_tx_down(void *wplink_id, void *skb)
{
	wplip_link_t *lip_link   = (wplip_link_t *)wplink_id;	

	if (!lip_link){
		DEBUG_EVENT("%s: Assertion error lip_dev=NULL!\n",
				__FUNCTION__);
		return 1;
	}

	DEBUG_TEST("%s:%s: Protocol Packet Len=%i\n",
			lip_link->name,__FUNCTION__,wan_skb_len(skb));


	if (lip_link->carrier_state != WAN_CONNECTED){
		DEBUG_TEST("%s: %s() Error Lip Link Carrier not connected !\n",
				lip_dev->name,__FUNCTION__);
		wan_skb_free(skb);
		return 0;
	}

	if (wan_skb_queue_len(&lip_link->tx_queue) >= MAX_TX_BUF){
		DEBUG_TEST("%s: %s() Error Protocol Tx queue full !\n",
				lip_link->name,__FUNCTION__);
		wplip_trigger_bh(lip_link);
		return 1;
	}

	wan_skb_unlink(skb);
	wan_skb_queue_tail(&lip_link->tx_queue,skb);
	wplip_trigger_bh(lip_link);

	return 0;
}

int wplip_callback_tx_down(void *wplip_id, void *skb)
{
	wplip_dev_t *lip_dev   = (wplip_dev_t *)wplip_id;	

	if (!lip_dev){
		DEBUG_EVENT("%s: Assertion error lip_dev=NULL!\n",
				__FUNCTION__);
		return 1;
	}

	DEBUG_TEST("%s:%s: Packet Len=%i\n",
			lip_dev->name,__FUNCTION__,wan_skb_len(skb));


	if (lip_dev->lip_link->carrier_state != WAN_CONNECTED){
		DEBUG_TEST("%s: %s() Error Lip Link Carrier not connected !\n",
				lip_dev->name,__FUNCTION__);
		wan_skb_free(skb);
		lip_dev->ifstats.tx_carrier_errors++;
		return 0;
	}

	if (wan_skb_queue_len(&lip_dev->tx_queue) >= MAX_TX_BUF){
		wplip_trigger_bh(lip_dev->lip_link);
		DEBUG_TEST("%s: %s() Error  Tx queue full Kick=%i!\n",
				lip_dev->name,__FUNCTION__,
				wan_test_bit(WPLIP_BH_AWAITING_KICK,&lip_dev->lip_link->tq_working)
);
		return 1;
	}

	wan_skb_unlink(skb);
	wan_skb_queue_tail(&lip_dev->tx_queue,skb);
	wplip_trigger_bh(lip_dev->lip_link);

	return 0;
}


unsigned int wplip_get_ipv4_addr (void *wplip_id, int type)
{
#ifdef __LINUX__
	wplip_dev_t *lip_dev   = (wplip_dev_t *)wplip_id;	

	struct in_ifaddr *ifaddr;
	struct in_device *in_dev;

	if ((in_dev = __in_dev_get(lip_dev->common.dev)) == NULL){
		return 0;
	}

	if ((ifaddr = in_dev->ifa_list)== NULL ){
		return 0;
	}
	
	switch (type){

	case WAN_LOCAL_IP:
		return ifaddr->ifa_local;
		break;
	
	case WAN_POINTOPOINT_IP:
		return ifaddr->ifa_address;
		break;	

	case WAN_NETMASK_IP:
		return ifaddr->ifa_mask;
		break;

	case WAN_BROADCAST_IP:
		return ifaddr->ifa_broadcast;
		break;
	default:
		return 0;
	}
#endif
	return 0;
}


int wplip_set_ipv4_addr (void *wplip_id, 
		         unsigned int local,
		         unsigned int remote,
		         unsigned int netmask,
		         unsigned int dns)
{
#if 0
	wplip_dev_t *lip_dev   = (wplip_dev_t *)wplip_id;	
#endif
	return 0;

}

void wplip_add_gateway(void *wplip_id)
{
#if 0
	wplip_dev_t *lip_dev   = (wplip_dev_t *)wplip_id;	
#endif

	return;
}




/***************************************************************
 * Private Device Functions
 */





/***************************************************************
 * Module Interface Functions
 *
 */

/*==============================================================
 * wanpipe_lip_init
 *
 * Description:
 * 	This function is called on module startup.
 * 	(ex: modprobe wanpipe_lip)
 *
 * 	Register the lip callback functions to the
 * 	wanmain which are used by the socket or
 * 	upper layer.
 */


static unsigned char wplip_fullname[]="WANPIPE(tm) L.I.P Network Layer";
static unsigned char wplip_copyright[]="(c) 1995-2004 Sangoma Technologies Inc.";

int wanpipe_lip_init(void*);
int wanpipe_lip_exit(void*);

int wanpipe_lip_init(void *arg)
{
	wplip_reg_t reg;	
	int err;
	
	if (WANPIPE_VERSION_BETA){
		DEBUG_EVENT("%s Beta%s-%s %s\n",
			wplip_fullname, WANPIPE_SUB_VERSION, WANPIPE_VERSION, wplip_copyright);
	}else{
		DEBUG_EVENT("%s Stable %s-%s %s\n",
			wplip_fullname, WANPIPE_VERSION, WANPIPE_SUB_VERSION, wplip_copyright);
	}

	err=wplip_init_prot();
	if (err){
		wplip_free_prot();
		return err;
	}

	wplip_link_lock=RW_LOCK_UNLOCKED;

	memset(&reg,0,sizeof(wplip_reg_t));
	
	reg.wplip_bind_link 	= wplip_bind_link;
	reg.wplip_unbind_link	= wplip_unbind_link;
	
	reg.wplip_if_reg 	= wplip_if_reg;
	reg.wplip_if_unreg   	= wplip_if_unreg;

	reg.wplip_disconnect 	= wplip_disconnect;
	reg.wplip_connect 	= wplip_connect;

	reg.wplip_rx 	 	= wplip_rx;
	reg.wplip_kick 		= wplip_kick;
	
	reg.wplip_register 	= wplip_register;
	reg.wplip_unreg 	= wplip_unreg;

	reg.wplip_get_if_status = wplip_get_if_status;

	register_wanpipe_lip_protocol(&reg);
	 
	memset(&wplip_link_num,0,sizeof(wplip_link_num));
	
	return 0;
}

int wanpipe_lip_exit (void *arg)
{
	if (!WAN_LIST_EMPTY(&list_head_link)){
		DEBUG_EVENT("%s: Major error: List not empty!\n",__FUNCTION__);
	}

	unregister_wanpipe_lip_protocol();
	wplip_free_prot();	
	
	DEBUG_EVENT("WANPIPE L.I.P: Unloaded\n");
	return 0;
}

#if 0
MODULE_AUTHOR("Nenad Corbic <ncorbic@sangoma.com>");
MODULE_DESCRIPTION("Wanpipe L.I.P Network Layer - Sangoma Tech. Copyright 2004");
MODULE_LICENSE("GPL");
module_init(wanpipe_lip_init);
module_exit(wanpipe_lip_exit);
#endif

WAN_MODULE_DEFINE(
		wanpipe_lip,"wanpipe_lip", 
		"Nenad Corbic <ncorbic@sangoma.com>",
		"Wanpipe L.I.P Network Layer - Sangoma Tech. Copyright 2004", 
		"GPL",
		wanpipe_lip_init, wanpipe_lip_exit, NULL);
WAN_MODULE_DEPEND(wanpipe_lip, wanrouter, 1,
			WANROUTER_MAJOR_VER, WANROUTER_MAJOR_VER);
WAN_MODULE_DEPEND(wanpipe_lip, wanpipe, 1,
			WANPIPE_MAJOR_VER, WANPIPE_MAJOR_VER);

