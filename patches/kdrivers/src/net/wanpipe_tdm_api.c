/*****************************************************************************
* wanpipe_tdm_api.c 
* 		
* 		WANPIPE(tm) AFT TE1 Hardware Support
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003-2005 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Oct 04, 2005	Nenad Corbic	Initial version.
* Jul 27, 2006	David Rokhvarg	<davidr@sangoma.com>	
*				Ported to Windows.
* Mar 10, 2008	David Rokhvarg	<davidr@sangoma.com>
*				Added BRI LoopBack control.
*****************************************************************************/

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe.h>
# include <if_wanpipe.h> 
# include <wanpipe_tdm_api.h>
#elif defined(__WINDOWS__)
# include <wanpipe_includes.h>
# include <wanpipe_tdm_api.h>

int 
aft_te1_insert_tdm_api_event_in_to_rx_queue(
	wanpipe_tdm_api_dev_t	*tdm_api_dev,
	wp_tdm_api_event_t		*pevent	
	);

int 
queue_tdm_api_rx_dpc(
	wanpipe_tdm_api_dev_t	*tdm_api_dev
	);

#define DBG_TDMCODEC	if(0)DbgPrint
#define DBG_TDM_RX	if(0)DbgPrint
#define DBG_RBS		if(0)DbgPrint

#define BUILD_TDMV_API

#elif defined(__LINUX__)
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe.h>
# include <linux/if_wanpipe.h> 
# include <linux/wanpipe_cfg.h>
# include <linux/wanpipe_tdm_api.h>
#endif


/*==============================================================
  Defines
 */

#define WP_TDMAPI_MAJOR 241
#define WP_TDMAPI_MINOR_OFFSET 0
#define WP_TDMAPI_MAX_MINORS 1024

#if !defined(__WINDOWS__)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
#define WP_CLASS_DEV_CREATE(class, devt, device, name) \
        class_device_create(class, NULL, devt, device, name)
#else
#define WP_CLASS_DEV_CREATE(class, devt, device, name) \
        class_device_create(class, devt, device, name)
#endif    

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
static struct class *wp_tdmapi_class = NULL;
#else
static struct class_simple *wp_tdmapi_class = NULL;
#define class_create class_simple_create
#define class_destroy class_simple_destroy
#define class_device_create class_simple_device_add
#define class_device_destroy(a, b) class_simple_device_remove(b)
#endif

#define	UNIT(file) MINOR(file->f_dentry->d_inode->i_rdev)
#define BUILD_TDMV_API

#endif/* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
#endif/* #if !defined(__WINDOWS__) */

#if defined(BUILD_TDMV_API)

#define WP_TDMAPI_SPAN_MASK	0xFFFF
#define WP_TMDAPI_SPAN_SHIFT	5 //8
#define WP_TDMAPI_CHAN_MASK	0x1F //0xFF
		
#define WP_TDMAPI_SET_MINOR(span,chan) ((((span-1)&WP_TDMAPI_SPAN_MASK)<<WP_TMDAPI_SPAN_SHIFT)|(chan&WP_TDMAPI_CHAN_MASK))

#define WP_TDMAPI_GET_SPAN_FROM_MINOR(minor) ((((minor)>>WP_TMDAPI_SPAN_SHIFT)&WP_TDMAPI_SPAN_MASK)+1)

#define WP_TDMAPI_GET_CHAN_FROM_MINOR(minor) ((minor)&WP_TDMAPI_CHAN_MASK)

#define WP_TDM_API_MAX_PERIOD 50 /* 50ms */

#define WP_TDM_MAX_RX_Q_LEN 10
#define WP_TDM_MAX_TX_Q_LEN 5
#define WP_TDM_MAX_HDLC_TX_Q_LEN 17
#define WP_TDM_MAX_EVENT_Q_LEN 10
#define WP_TDM_MAX_RX_FREE_Q_LEN 10

#define WP_TDMAPI_MAX_SPANS 255


#undef  DEBUG_API_WAKEUP
#undef  DEBUG_API_READ
#undef  DEBUG_API_WRITE
#undef  DEBUG_API_POLL


static void *wp_tdmapi_hash[WP_TDMAPI_HASH_SZ];
static wan_spinlock_t wp_tdmapi_hash_lock;
static int wp_tdmapi_global_cnt=0;
static u8 *rx_gains;
static u8 *tx_gains;
/*==============================================================
  Prototypes
 */

static int wp_tdmapi_open(struct inode *inode, struct file *file);
static ssize_t wp_tdmapi_read(struct file *file, char *usrbuf, size_t count, loff_t *ppos);
static ssize_t wp_tdmapi_write(struct file *file, const char *usrbuf, size_t count, loff_t *ppos);
static int wp_tdmapi_release(struct inode *inode, struct file *file);
static int wp_tdmapi_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long data);

static void wanpipe_tdm_api_rbs_poll(wanpipe_tdm_api_dev_t *tdm_api);
static void wanpipe_tdm_api_fe_alarm_event(wanpipe_tdm_api_dev_t *tdm_api, int state);

static void wp_tdmapi_rbsbits(void* card_id, int channel, unsigned char rbsbits);
static void wp_tdmapi_alarms(void* card_id, unsigned long alarams);
static void wp_tdmapi_dtmf(void* card_id, wan_event_t*);	
static void wp_tdmapi_hook(void* card_id, wan_event_t*);	
static void wp_tdmapi_ringtrip(void* card_id, wan_event_t*);
static void wp_tdmapi_ringdetect (void* card_id, wan_event_t *event);

#if defined(__WINDOWS__)
static int store_tdm_api_pointer_in_card(sdla_t *card, wanpipe_tdm_api_dev_t *tdm_api);
static int remove_tdm_api_pointer_from_card(wanpipe_tdm_api_dev_t *tdm_api);
int wanpipe_tdm_api_ioctl(wanpipe_tdm_api_dev_t *tdm_api, struct ifreq *ifr);
#else
static unsigned int wp_tdmapi_poll(struct file *file, struct poll_table_struct *wait_table); 
static int wanpipe_tdm_api_ioctl(wanpipe_tdm_api_dev_t *tdm_api, struct ifreq *ifr);                   
#endif
static int wanpipe_tdm_api_event_ioctl(wanpipe_tdm_api_dev_t*, wanpipe_tdm_api_cmd_t*);
 
/*==============================================================
  Global Variables
 */
#if defined(__WINDOWS__)
//NOTE: initialization order here MUST be the same as in structure declaration!!!
/*
//not used, don't do anything.
static struct file_operations wp_tdmapi_fops = {
	wp_tdmapi_open,
	wp_tdmapi_release,
	wp_tdmapi_ioctl,
	wp_tdmapi_read,
	wp_tdmapi_write,
	wp_tdmapi_poll
};
*/
#else
static struct file_operations wp_tdmapi_fops = {
	owner: THIS_MODULE,
	llseek: NULL,
	open: wp_tdmapi_open,
	release: wp_tdmapi_release,
	ioctl: wp_tdmapi_ioctl,
	read: wp_tdmapi_read,
	write: wp_tdmapi_write,
	poll: wp_tdmapi_poll,
	mmap: NULL,
	flush: NULL,
	fsync: NULL,
	fasync: NULL,
};
#endif

static void wp_tdmapi_init_buffs(wanpipe_tdm_api_dev_t *tdm_api)
{	
	wan_skb_queue_purge(&tdm_api->wp_rx_list);
	wan_skb_queue_purge(&tdm_api->wp_rx_free_list);
	wan_skb_queue_purge(&tdm_api->wp_tx_free_list);
	wan_skb_queue_purge(&tdm_api->wp_tx_list);
	wan_skb_queue_purge(&tdm_api->wp_event_list);
	
	if (tdm_api->rx_skb){
		wan_skb_free(tdm_api->rx_skb);
		tdm_api->rx_skb=NULL;
	}
	if (tdm_api->tx_skb){
		wan_skb_free(tdm_api->tx_skb);
		tdm_api->tx_skb=NULL;
	}

	tdm_api->rx_gain=NULL;
	tdm_api->tx_gain=NULL;

}


#ifdef DEBUG_API_WAKEUP
static int gwake_cnt=0;
#endif
#if !defined(__WINDOWS__)
static void wp_wakeup_tdmapi(wanpipe_tdm_api_dev_t *tdm_api)
{
#ifdef DEBUG_API_WAKEUP 
	if (tdm_api->tdm_span == 1 && tdm_api->tdm_chan == 1) {
		gwake_cnt++;
		if (gwake_cnt % 1000 == 0) {
			DEBUG_EVENT("%s: TDMAPI WAKING DEV \n",tdm_api->name);
		}
	}
#endif	
	if (waitqueue_active(&tdm_api->poll_wait)){
		wake_up_interruptible(&tdm_api->poll_wait);
	}
}

static struct cdev wptdm_cdev = {
	.kobj	=	{.name = "wptdm", },
	.owner	=	THIS_MODULE,
};
#endif

static int wp_tdmapi_reg_globals(void)
{
	int err=0;
	
	rx_gains=NULL;
	tx_gains=NULL;
	wan_spin_lock_init(&wp_tdmapi_hash_lock, "wan_tdmapi_hash_lock");
	DEBUG_TDMAPI("%s: Registering Wanpipe TDM Device!\n",__FUNCTION__);
#if !defined(__WINDOWS__)
	{
#ifdef LINUX_2_4
		if ((err = register_chrdev(WP_TDMAPI_MAJOR, "wptdm", &wp_tdmapi_fops))) {
			DEBUG_EVENT("Unable to register tor device on %d\n", WP_TDMAPI_MAJOR);
			return err;
		}    
		
		wp_tdmapi_class = class_create(THIS_MODULE, "wptdm");
		 
#else
		dev_t dev = MKDEV(WP_TDMAPI_MAJOR, 0);
		
		if ((err=register_chrdev_region(dev, WP_TDMAPI_MAX_MINORS, "wptdm"))) {
			DEBUG_EVENT("Unable to register tor device on %d\n", WP_TDMAPI_MAJOR);
			return err;
		}

		cdev_init(&wptdm_cdev, &wp_tdmapi_fops);
		if (cdev_add(&wptdm_cdev, dev, WP_TDMAPI_MAX_MINORS)) {
			kobject_put(&wptdm_cdev.kobj);
			unregister_chrdev_region(dev, WP_TDMAPI_MAX_MINORS);
			return -EINVAL;
		}      

		wp_tdmapi_class = class_create(THIS_MODULE, "wptdm");
		if (IS_ERR(wp_tdmapi_class)) {
			DEBUG_EVENT("Error creating wptdm class.\n");
			cdev_del(&wptdm_cdev);
			unregister_chrdev_region(dev, WP_TDMAPI_MAX_MINORS);
			return -EINVAL;
		}
#endif
	}
#endif
	return err;
}
 
static int wp_tdmapi_unreg_globals(void)
{
	DEBUG_EVENT("%s: Unregistering Wanpipe TDM Device!\n",__FUNCTION__);
#if !defined(__WINDOWS__)
	class_destroy(wp_tdmapi_class);
# ifdef LINUX_2_4
	unregister_chrdev(WP_TDMAPI_MAJOR, "wptdm");
# else
	cdev_del(&wptdm_cdev);
  	unregister_chrdev_region(MKDEV(WP_TDMAPI_MAJOR, 0), WP_TDMAPI_MAX_MINORS);   
# endif
#endif

	if (tx_gains) {
		wan_free(tx_gains);
		tx_gains=NULL;
	}
	if (rx_gains) {
		wan_free(rx_gains);
		rx_gains=NULL;
	}
	return 0;
}

int wanpipe_tdm_api_reg(wanpipe_tdm_api_dev_t *tdm_api)
{
	sdla_t	*card = NULL;
	u8 tmp_name[50];
	int err;
	
	if (wp_tdmapi_global_cnt == 0){
		wp_tdmapi_reg_globals();
	}
	wp_tdmapi_global_cnt++;
	
	WAN_ASSERT(tdm_api == NULL);
	card = (sdla_t*)tdm_api->card;
	
	wan_skb_queue_init(&tdm_api->wp_rx_list);
	wan_skb_queue_init(&tdm_api->wp_rx_free_list);
	wan_skb_queue_init(&tdm_api->wp_tx_free_list);
	wan_skb_queue_init(&tdm_api->wp_tx_list);
	wan_skb_queue_init(&tdm_api->wp_event_list);

	tdm_api->cfg.rbs_poll=0;
	
	if (tdm_api->hdlc_framing) {
	
		if (IS_BRI_CARD(card)) {
			tdm_api->cfg.hw_mtu_mru		=300;	
			tdm_api->cfg.usr_mtu_mru	=300;	
		} else {
			tdm_api->cfg.hw_mtu_mru		=1500;	
			tdm_api->cfg.usr_mtu_mru	=1500;	
		}

		tdm_api->cfg.usr_period		=0;	
		tdm_api->cfg.tdm_codec		=WP_NONE;		
		tdm_api->cfg.power_level	=0;	
		tdm_api->cfg.rx_disable		=0;	
		tdm_api->cfg.tx_disable		=0;			
		tdm_api->cfg.ec_tap		=0;	
		tdm_api->cfg.rbs_rx_bits	=-1;
		tdm_api->cfg.hdlc		=1;

		/* We are expecting tx_q_len for hdlc
  		 * to be configured from upper layer */
		
	} else { 
		tdm_api->cfg.hw_mtu_mru		=8;	
		tdm_api->cfg.usr_period		=10;	
		tdm_api->cfg.tdm_codec		=WP_NONE;		
		tdm_api->cfg.power_level	=0;	
		tdm_api->cfg.rx_disable		=0;	
		tdm_api->cfg.tx_disable		=0;			
		tdm_api->cfg.usr_mtu_mru	=tdm_api->cfg.usr_period*tdm_api->cfg.hw_mtu_mru;	
		tdm_api->cfg.ec_tap		=0;	
		tdm_api->cfg.rbs_rx_bits	=-1;
		tdm_api->cfg.hdlc		=0;
		tdm_api->tx_q_len		= WP_TDM_MAX_TX_Q_LEN;
	
		if (tdm_api->cfg.idle_flag == 0) {
        		tdm_api->cfg.idle_flag=0xFF; 	
		} 
	}
	
	
	tdm_api->critical=0;
	wan_clear_bit(0,&tdm_api->used);

#if !defined(__WINDOWS__)	
	init_waitqueue_head(&tdm_api->poll_wait);
	err=wphash_tdm_api_dev(wp_tdmapi_hash,tdm_api,
			   WP_TDMAPI_SET_MINOR(tdm_api->tdm_span,tdm_api->tdm_chan));
#else
	err=store_tdm_api_pointer_in_card(card, tdm_api);
#endif

	if (err){
		wp_tdmapi_global_cnt--;
		if (wp_tdmapi_global_cnt == 0){
			wp_tdmapi_unreg_globals();
		}
		return err;
	}
	wan_spin_lock_init(&tdm_api->lock, "wan_tdmapi_lock");	
	sprintf(tmp_name,"wptdm_s%dc%d",tdm_api->tdm_span,tdm_api->tdm_chan);
	
	DEBUG_TDMAPI("%s: Configuring TDM API NAME=%s Qlen=%i\n",
			card->devname,tmp_name, tdm_api->tx_q_len);

	/* Initialize Event Callback functions */
	card->wandev.event_callback.rbsbits		= wp_tdmapi_rbsbits; 
	card->wandev.event_callback.alarms		= wp_tdmapi_alarms; 
	card->wandev.event_callback.hook		= wp_tdmapi_hook; 
	card->wandev.event_callback.ringdetect	= wp_tdmapi_ringdetect;
	card->wandev.event_callback.ringtrip	= wp_tdmapi_ringtrip;

#if defined(__WINDOWS__)
	/* Analog always supports DTMF detection */
	tdm_api->dtmfsupport = WANOPT_YES;
#endif

	/* Always initialize the callback pointer */
	card->wandev.event_callback.dtmf = wp_tdmapi_dtmf; 

	if (tdm_api->cfg.rbs_tx_bits) {
		DEBUG_EVENT("%s: Setting Tx RBS/CAS Idle Bits = 0x%02X\n",
			       	        tmp_name, 
					tdm_api->cfg.rbs_tx_bits);
		tdm_api->write_rbs_bits(tdm_api->chan, 
		                        tdm_api->tdm_chan, 
					(u8)tdm_api->cfg.rbs_tx_bits); 
	}	   

#if !defined(__WINDOWS__)	
	WP_CLASS_DEV_CREATE(wp_tdmapi_class, 
			    MKDEV(WP_TDMAPI_MAJOR, 
			    	  WP_TDMAPI_SET_MINOR(tdm_api->tdm_span,tdm_api->tdm_chan)), 
			    NULL, tmp_name);
#endif
	return err;
}

int wanpipe_tdm_api_unreg(wanpipe_tdm_api_dev_t *tdm_api)
{
	wan_smp_flag_t flags;	

	wan_set_bit(WP_TDM_DOWN,&tdm_api->critical);
	
	if (wan_test_bit(0,&tdm_api->used)) {
		DEBUG_EVENT("%s Failed to unreg Span=%i Chan=%i: BUSY!\n",
			tdm_api->name,tdm_api->tdm_span,tdm_api->tdm_chan);
		return -EBUSY;
	}
	
	wan_spin_lock_irq(&wp_tdmapi_hash_lock,&flags);
#if !defined(__WINDOWS__)	
	wpunhash_tdm_api_dev(tdm_api);
#else
	remove_tdm_api_pointer_from_card(tdm_api);
#endif
	
	wp_tdmapi_init_buffs(tdm_api);
	
	wan_spin_unlock_irq(&wp_tdmapi_hash_lock,&flags);
#if !defined(__WINDOWS__)		
	class_device_destroy(wp_tdmapi_class, 
			     MKDEV(WP_TDMAPI_MAJOR, 
			     WP_TDMAPI_SET_MINOR(tdm_api->tdm_span,tdm_api->tdm_chan))); 
#endif
	wp_tdmapi_global_cnt--;
	if (wp_tdmapi_global_cnt == 0) {
		wp_tdmapi_unreg_globals();
	}

	return 0;
}
 
int wanpipe_tdm_api_update_state(wanpipe_tdm_api_dev_t *tdm_api, int state)
{
	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init)){
		return -ENODEV;
	}
		
       	tdm_api->state = (u8)state;
       	tdm_api->cfg.fe_alarms = (state == WAN_CONNECTED ? 0 : 1);

       	if (wan_test_bit(0,&tdm_api->used)) {
       		wanpipe_tdm_api_fe_alarm_event(tdm_api,state);
#if !defined(__WINDOWS__)		
       		wp_wakeup_tdmapi(tdm_api);
#endif
       	}
	
	return 0;
}

int wanpipe_tdm_api_kick(wanpipe_tdm_api_dev_t *tdm_api)
{
	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init)){
		return -ENODEV;
	}
	
	if (is_tdm_api_stopped(tdm_api) || (unsigned)wan_skb_queue_len(&tdm_api->wp_tx_list) > tdm_api->tx_q_len){
		wp_tdm_api_start(tdm_api);
		if (wan_test_bit(0,&tdm_api->used)) {
#if !defined(__WINDOWS__)
			wp_wakeup_tdmapi(tdm_api);
#endif
		} 
	} 
	
	return 0;
}



static int wp_tdmapi_open(struct inode *inode, struct file *file)
{
	wanpipe_tdm_api_dev_t *tdm_api;
	wan_smp_flag_t flags;

	
#if !defined(__WINDOWS__)
	u32 tdm_span = WP_TDMAPI_GET_SPAN_FROM_MINOR(UNIT(file));
	u32 tdm_chan = WP_TDMAPI_GET_CHAN_FROM_MINOR(UNIT(file));
#else
	u32 tdm_span = file->span;
	u32 tdm_chan = file->chan;
#endif
	
	wan_spin_lock_irq(&wp_tdmapi_hash_lock,&flags);
	tdm_api = wp_find_tdm_api_dev(wp_tdmapi_hash,
#if !defined(__WINDOWS__)
	       		UNIT(file),
#else
		      	(unsigned int)file,
#endif
		       	tdm_span,
		       	tdm_chan);

	if (!tdm_api){
		wan_spin_unlock_irq(&wp_tdmapi_hash_lock,&flags);
		return -ENODEV;
	}
	
	if (wan_test_bit(WP_TDM_DOWN,&tdm_api->critical)) {
		wan_spin_unlock_irq(&wp_tdmapi_hash_lock,&flags);
		return -ENODEV;
	}
	
	wp_tdmapi_init_buffs(tdm_api);
	
	wan_spin_lock(&tdm_api->lock);
	if (wan_test_and_set_bit(0,&tdm_api->used)){
		wan_spin_unlock(&tdm_api->lock);
		wan_spin_unlock_irq(&wp_tdmapi_hash_lock,&flags);
		
		DEBUG_EVENT ("%s: Device S/C(%i/%i) is already open!\n",
				tdm_api->name, tdm_span,tdm_span);
		/* This device is already busy */
		return -EBUSY;
	}
	
	file->private_data = tdm_api; 
	
	wan_spin_unlock(&tdm_api->lock);
	wan_spin_unlock_irq(&wp_tdmapi_hash_lock,&flags);
	
	
	DEBUG_TDMAPI ("%s: DRIVER OPEN S/C(%i/%i) API Ptr=%p\n",
		__FUNCTION__, tdm_span, tdm_chan, tdm_api);
	
	return 0;
}

#ifdef DEBUG_API_READ	
static int gread_cnt=0;
#endif

static ssize_t wp_tdmapi_read_msg(struct file *file, struct msghdr *msg, size_t count)
{
	wanpipe_tdm_api_dev_t *tdm_api;
	netskb_t *skb;
	int err=0, len=0;

#if !defined(__WINDOWS__)
	WAN_ASSERT((file==NULL));
	
	tdm_api=file->private_data;
	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init) || !wan_test_bit(0,&tdm_api->used)) {
		return -ENODEV;
	}
	
	if (wan_test_bit(WP_TDM_DOWN,&tdm_api->critical)) {
		return -ENODEV;
	}
	
	
	skb=wan_skb_dequeue(&tdm_api->wp_rx_list);
	if (!skb){
		return -ENOBUFS;
	}

#ifdef DEBUG_API_READ	
	if (tdm_api->tdm_span == 1 && 	
	    tdm_api->tdm_chan == 1) {
	    gread_cnt++;
	    if (gread_cnt && 
	        gread_cnt % 1000 == 0) {
		DEBUG_EVENT("%s: WP_TDM API READ %i Len=%i\n", 
				tdm_api->name, gread_cnt,
				wan_skb_len(skb)-sizeof(wp_tdm_api_rx_hdr_t));
	    }
	}
#endif
	
	if (count < wan_skb_len(skb) || 
	    wan_skb_len(skb) < sizeof(wp_tdm_api_rx_hdr_t)){
		DEBUG_TDMAPI("%s:%d TDMAPI READ: Error: Count=%i < Skb=%i < HDR=%i Critical Error\n",
			__FUNCTION__,__LINE__,count,wan_skb_len(skb),sizeof(wp_tdm_api_rx_hdr_t));
		wan_skb_free(skb);
		return -EFAULT;
	}
	
	len=wan_skb_len(skb);

	if (tdm_api->cfg.tdm_codec == WP_NONE) {
		err = wan_memcpy_toiovec(msg->msg_iov, 
					 wan_skb_data(skb),
					 wan_skb_len(skb));
		if (err){
			wan_skb_free(skb);
			return err;
		}
	
	} else {
		u8 *data_ptr;
		wanpipe_codec_ops_t *wp_codec_ops;
		u32 power;
		
		
		wp_codec_ops=WANPIPE_CODEC_OPS[tdm_api->cfg.hw_tdm_coding][tdm_api->cfg.tdm_codec];
		if (!wp_codec_ops || !wp_codec_ops->init){
			return -EINVAL;
		}
	
		data_ptr=(u8*)wan_skb_data(skb);
		
#if 0
		/* OPTIMIZATION: Dont copy the header.
		 * Not used now */
		err = wan_memcpy_toiovec(msg->msg_iov, 
					 wan_skb_data(skb),
					 sizeof(wp_tdm_api_rx_hdr_t));
		if (err){
			wan_skb_free(skb);
			return err;
		}
#endif		
		len = wp_codec_ops->encode(&data_ptr[sizeof(wp_tdm_api_rx_hdr_t)],
					wan_skb_len(skb)-sizeof(wp_tdm_api_rx_hdr_t),
					(u16*)msg->msg_iov[1].iov_base,
					&power, tdm_api->rx_gain, 1);
			
			
#ifdef DEBUG_API_READ		
		if (tdm_api->tdm_span == 1 && 	
		    tdm_api->tdm_chan == 1) {
			if (gread_cnt && 
				gread_cnt % 1000 == 0) {
				DEBUG_EVENT("%s: WP_TDM API READ CODEC %i Len=%i\n", 
						tdm_api->name, gread_cnt,
						len);
			}
		}
#endif		
				
		if (len <= 0){
			wan_skb_free(skb);
			return -EINVAL;
		}
		
		len+=sizeof(wp_tdm_api_rx_hdr_t);
	}
	
	if (tdm_api->hdlc_framing || 
	    wan_skb_queue_len(&tdm_api->wp_rx_free_list) > WP_TDM_MAX_RX_FREE_Q_LEN) {
		wan_skb_free(skb);
	} else {
		wan_skb_init(skb,16);
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_rx_free_list,skb);
	}

#endif	/* #if !defined(__WINDOWS__) */
	return len;
}

#define WP_UIO_MAX_SZ 5

static ssize_t wp_tdmapi_read(struct file *file, char *usrbuf, size_t count, loff_t *ppos)
{
	struct iovec iovstack[WP_UIO_MAX_SZ];
	struct iovec *iov=iovstack;
	struct msghdr msg_sys;
	struct msghdr *msg = (struct msghdr*)usrbuf;
	int err;
	
	WAN_ASSERT((file==NULL));
	WAN_ASSERT((usrbuf==NULL)); 
	
#if !defined(__WINDOWS__)
	if (copy_from_user(&msg_sys,msg,sizeof(struct msghdr)))
		return -EFAULT;
                                                
	if (msg_sys.msg_iovlen > WP_UIO_MAX_SZ)
		return -EFAULT;    
		
	msg_sys.msg_namelen=0;
	
	err=wan_verify_iovec(&msg_sys, iov, NULL, 0);
	if (err < 0) {
		return err;
	}
#endif
	return wp_tdmapi_read_msg(file, &msg_sys, count);
}
          

static int wp_tdmapi_tx(wanpipe_tdm_api_dev_t *tdm_api)
{
	netskb_t *skb;
	int err; 
	
	if (wan_test_and_set_bit(WP_TDM_HDLC_TX,&tdm_api->critical)){
		return 0;
	}
	
	while ((skb=wan_skb_dequeue(&tdm_api->wp_tx_list)) != NULL) {
		err=tdm_api->write_hdlc_frame(tdm_api->chan,skb);
		if (err) {
			wan_skb_queue_head(&tdm_api->wp_tx_list, skb);
			break;
		}
		tdm_api->cfg.stats.tx_packets++;
		skb=NULL;
	}
	
	wan_clear_bit(WP_TDM_HDLC_TX,&tdm_api->critical);
	return 0;
}

#ifdef DEBUG_API_WRITE	
static int gwrite_cnt=0;
#endif
static ssize_t wp_tdmapi_write_msg(struct file *file, struct msghdr *msg, size_t count)
{
	wanpipe_tdm_api_dev_t *tdm_api;
	netskb_t *skb;
	int err;
	int skb_len = WP_TDM_API_MAX_LEN;
	u8 *buf;

#if !defined(__WINDOWS__)	
	WAN_ASSERT((file==NULL));

	tdm_api = file->private_data;
	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init) || !wan_test_bit(0,&tdm_api->used)){
		return -ENODEV;
	}
	if (wan_test_bit(WP_TDM_DOWN,&tdm_api->critical)) {
		return -ENODEV;
	}
	
	if (tdm_api->hdlc_framing) {
		skb_len=count+100;
		if (count > (tdm_api->cfg.usr_mtu_mru + sizeof(wp_tdm_api_tx_hdr_t))) {
			return -EFBIG;
		}
		if (count <= sizeof(wp_tdm_api_tx_hdr_t)) {
			return -EINVAL;
		}
	} else {
		/* FIXME: Should have some sort of a check invalid len 
		 *        In reality not needed */ 
#if 0
		if ((count-sizeof(wp_tdm_api_tx_hdr_t)) % 8){
			DEBUG_EVENT("%s: Error: Tx packet not divisible by 8\n");
			return -EINVAL;
		}
#endif
		skb_len=WP_TDM_API_MAX_LEN;
	}

	DEBUG_TEST("%s: TX FRAME List=%i\n",
			tdm_api->name, wan_skb_queue_len(&tdm_api->wp_tx_list));	

	if (wan_skb_queue_len(&tdm_api->wp_tx_list) > tdm_api->tx_q_len){
		wp_tdm_api_stop(tdm_api);
		return -EBUSY;
	}
	
	skb=wan_skb_dequeue(&tdm_api->wp_tx_free_list);
	if (!skb) {
		skb=wan_skb_alloc(skb_len);
		if (!skb){
			return -ENOMEM;
		}
	}
	
	if (tdm_api->hdlc_framing) {

		buf = skb_put(skb,count);
		err = memcpy_fromiovec(buf, msg->msg_iov, count);
		if (err){
			wan_skb_free(skb);
			return -ENOMEM;
		}
	
                if (wan_skb_len(skb) <= sizeof(wp_tdm_api_tx_hdr_t)) {
			wan_skb_free(skb);
                	return -EINVAL;
		}
				
		wan_skb_pull(skb,sizeof(wp_tdm_api_tx_hdr_t));
		wan_skb_queue_tail(&tdm_api->wp_tx_list, skb);
		
		wp_tdmapi_tx(tdm_api);
		
		return count;
	}
	
	if (tdm_api->cfg.tdm_codec == WP_NONE) {
	
		buf = skb_put(skb,count);
		err = memcpy_fromiovec(buf, msg->msg_iov, count);
		if (err){
			wan_skb_free(skb);
			return -ENOMEM;
		}
		
	} else {
	
		int len;	
		u8 *buf;
		wanpipe_codec_ops_t *wp_codec_ops;
	
		buf = skb_put(skb,sizeof(wp_tdm_api_tx_hdr_t));

#if 0
		/* OPTIMIZATION: Do not copy the Write Header
		 * Not needed at this time */
		err = memcpy_fromiovec(buf, msg->msg_iov, sizeof(wp_tdm_api_tx_hdr_t));
		if (err){
			wan_skb_free(skb);
			return -ENOMEM;
		}
#endif		
			
		wp_codec_ops=WANPIPE_CODEC_OPS[tdm_api->cfg.hw_tdm_coding][tdm_api->cfg.tdm_codec];
		if (!wp_codec_ops || !wp_codec_ops->init){
			wan_skb_free(skb);
			return -EINVAL;	
		}
	
		len = wp_codec_ops->decode((u16*)msg->msg_iov[1].iov_base,
					   count-sizeof(wp_tdm_api_tx_hdr_t),
					   wan_skb_tail(skb),
					   tdm_api->tx_gain, 1);
		if (len <= 0) {
			wan_skb_free(skb);
			return -EFAULT;
		}	
		
		if (len > wan_skb_tailroom(skb)){
			DEBUG_EVENT("%s:%d TDM WRITE Critical Error: Len=%i TailRoom=%i\n",
				__FUNCTION__,__LINE__, len, wan_skb_tailroom(skb));
			wan_skb_free(skb);
			tdm_api->cfg.stats.tx_errors++;
			return -EIO;
		}
		
		buf=wan_skb_put(skb,len);
	}

#ifdef DEBUG_API_WRITE	
	if (tdm_api->tdm_span == 1 && 	
	    tdm_api->tdm_chan == 1) {
		gwrite_cnt++;
		if (gwrite_cnt && 
		    gwrite_cnt % 1000 == 0) {
			DEBUG_EVENT("%s: WP_TDM API WRITE CODEC %i Len=%i\n", 
					tdm_api->name, gwrite_cnt,
					wan_skb_len(skb)-sizeof(wp_tdm_api_tx_hdr_t));
		}
	}
#endif	
		
	wan_skb_queue_tail(&tdm_api->wp_tx_list, skb);

#endif	/* #if !defined(__WINDOWS__) */
	return count;
}


static ssize_t wp_tdmapi_write(struct file *file, const char *usrbuf, size_t count, loff_t *ppos)
{
	struct iovec iovstack[WP_UIO_MAX_SZ];
	struct iovec *iov=iovstack;
	struct msghdr msg_sys;
	struct msghdr *msg = (struct msghdr*)usrbuf;
	int err;

#if !defined(__WINDOWS__)	
	WAN_ASSERT((file==NULL));
	WAN_ASSERT((usrbuf==NULL)); 
	
	if (copy_from_user(&msg_sys,msg,sizeof(struct msghdr)))
		return -EFAULT;
                                                
	if (msg_sys.msg_iovlen > WP_UIO_MAX_SZ)
		return -EFAULT;    
		
	msg_sys.msg_namelen=0;
	
	err=wan_verify_iovec(&msg_sys, iov, NULL, 0);
	if (err < 0) {
		return err;
	}
#endif
	return wp_tdmapi_write_msg(file, &msg_sys, count);
}



static int wp_tdmapi_release(struct inode *inode, struct file *file)
{
	wanpipe_tdm_api_dev_t *tdm_api = file->private_data;

#if !defined(__WINDOWS__)
	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init)){
		return -ENODEV;
	}
	
	wan_clear_bit(0,&tdm_api->used);
	wp_wakeup_tdmapi(tdm_api);
	
	wan_spin_lock(&tdm_api->lock);

	tdm_api->cfg.rbs_rx_bits=-1;
	tdm_api->cfg.rbs_tx_bits=-1;
	
	tdm_api->cfg.rbs_poll=0;
	
	file->private_data=NULL;
	
	wan_spin_unlock(&tdm_api->lock);
#endif
	
	return 0;
}

static int wp_tdmapi_ioctl(struct inode *inode, struct file *file, 
		      unsigned int cmd, unsigned long data)
{
	struct ifreq ifr;
	wanpipe_tdm_api_dev_t *tdm_api;

	WAN_ASSERT((file==NULL));
	
	tdm_api = file->private_data;
	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init) || !wan_test_bit(0,&tdm_api->used)){
		return -ENODEV;
	}
	if (wan_test_bit(WP_TDM_DOWN,&tdm_api->critical)) {
		return -ENODEV;
	}
	
	if (data == 0){
		return -EINVAL;
	}
	
	ifr.ifr_data=(void*)data;
	
	return wanpipe_tdm_api_ioctl(tdm_api, &ifr);
	
}

#ifdef DEBUG_API_POLL
#warning "POLL Debugging Enabled"
static int gpoll_cnt=0;
#endif

#if defined(__WINDOWS__)

unsigned char wp_tdmapi_poll(void *tdm_api_ptr)
{
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t *)tdm_api_ptr;

	if (tdm_api->cfg.rbs_poll){
		wanpipe_tdm_api_rbs_poll(tdm_api);
	}
	return 0;
}

#else

static unsigned int wp_tdmapi_poll(struct file *file, struct poll_table_struct *wait_table)
{
	int ret=0;
	wanpipe_tdm_api_dev_t *tdm_api = file->private_data;
	
	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init) || !wan_test_bit(0,&tdm_api->used)){
		return -ENODEV;
	}
	
	if (wan_test_bit(WP_TDM_DOWN,&tdm_api->critical)) {
		return -ENODEV;
	}

	poll_wait(file, &tdm_api->poll_wait, wait_table);
	ret = 0;

#ifdef DEBUG_API_POLL 	
	if (tdm_api->tdm_span == 1 && tdm_api->tdm_chan == 1) {
		gpoll_cnt++;
		if (gpoll_cnt && gpoll_cnt % 1000 == 0) {
			DEBUG_EVENT("%s: WP_TDM POLL WAKEUP %i RxF=%i TxF=%i TxCE=%i TxE=%i TxP=%i\n", 
				tdm_api->name, gpoll_cnt,
				wan_skb_queue_len(&tdm_api->wp_tx_free_list),
				wan_skb_queue_len(&tdm_api->wp_rx_free_list),
				tdm_api->cfg.stats.tx_carrier_errors,
				tdm_api->cfg.stats.tx_errors,
				tdm_api->cfg.stats.tx_packets);
		}
	}
#endif	
	
	/* Tx Poll */
	if (!wan_test_bit(0,&tdm_api->cfg.tx_disable)){
	
		if (tdm_api->hdlc_framing &&
		    wan_skb_queue_len(&tdm_api->wp_tx_list) > tdm_api->tx_q_len &&
		    !is_tdm_api_stopped(tdm_api)) {
			wp_tdmapi_tx(tdm_api);
		}

		
		if (wan_skb_queue_len(&tdm_api->wp_tx_list) <= tdm_api->tx_q_len) {
			ret |= POLLOUT | POLLWRNORM;
		} 
	}
	
	/* Rx Poll */
	if (!wan_test_bit(0,&tdm_api->cfg.rx_disable) &&
	    wan_skb_queue_len(&tdm_api->wp_rx_list)) {
		ret |= POLLIN | POLLRDNORM;
	}
	
	if (tdm_api->cfg.rbs_poll){
		wanpipe_tdm_api_rbs_poll(tdm_api);
	}
	
	if (wan_skb_queue_len(&tdm_api->wp_event_list)) {
		/* Indicate an exception */
		ret |= POLLPRI;
	}

	return ret; 
}

#endif	/* #if defined(__WINDOWS__) */

static void wanpipe_tdm_api_rbs_poll(wanpipe_tdm_api_dev_t *tdm_api)
{
	u8			rbs_bits;
	netskb_t		*skb;
	wp_tdm_api_event_t	*pevent = NULL;
#if defined(__WINDOWS__)
	wp_tdm_api_event_t	event;
#endif

	if (!tdm_api->cfg.rbs_poll) {
		return;
	}
	if (SYSTEM_TICKS - tdm_api->rbs_poll_cnt < tdm_api->cfg.rbs_poll) {
		return;
	}
	tdm_api->rbs_poll_cnt=SYSTEM_TICKS;

	if (!tdm_api->read_rbs_bits) {
		return;
	}
#if 0
	DEBUG_EVENT("%s: READING RBS 0x%lX %i\n",
		tdm_api->name,SYSTEM_TICKS,tdm_api->cfg.stats.rx_errors);
		tdm_api->cfg.stats.rx_errors=0;
#endif

	tdm_api->read_rbs_bits(	tdm_api->chan, 
							tdm_api->tdm_chan, 
							&rbs_bits);
	if (tdm_api->cfg.rbs_rx_bits == rbs_bits) {
		return;
	}

	DEBUG_TDMAPI("%s: RBS BITS CHANGED O=0x%X N=0x%X\n",
			tdm_api->name, tdm_api->cfg.rbs_rx_bits, rbs_bits);

	tdm_api->cfg.rbs_rx_bits = rbs_bits;
				
#if defined(__WINDOWS__)
	pevent = &event;
#else
	if (wan_skb_queue_len(&tdm_api->wp_event_list) > WP_TDM_MAX_EVENT_Q_LEN) {	
		return;
	}
	skb=wan_skb_alloc(sizeof(wp_tdm_api_event_t));
	if (skb == NULL) return;

	pevent = (wp_tdm_api_event_t*)wan_skb_put(skb,sizeof(wp_tdm_api_event_t));
#endif/* #if !defined(__WINDOWS__) */
				
	memset(pevent,0,sizeof(wp_tdm_api_event_t));
	pevent->wp_tdm_api_event_type	= WP_TDMAPI_EVENT_RBS;
	pevent->wp_tdm_api_event_rbs_bits = (u8)tdm_api->cfg.rbs_rx_bits;

#if 0
	/* FIXME: NENAD TO ADD Timestamp  */
	rx_hdr->event_time_stamp = gettimeofday();
#endif					

#if defined(__WINDOWS__)
	pevent->channel = (u_int16_t)tdm_api->tdm_chan;
	aft_te1_insert_tdm_api_event_in_to_rx_queue(tdm_api, pevent);
	queue_tdm_api_rx_dpc(tdm_api);
#else
	wan_skb_queue_tail(&tdm_api->wp_event_list,skb);
#endif
}

static void wanpipe_tdm_api_fe_alarm_event(wanpipe_tdm_api_dev_t *tdm_api, int state)
{
	netskb_t       		*skb;
	wp_tdm_api_event_t	*pevent = NULL;
#if defined(__WINDOWS__)
	wp_tdm_api_event_t	event;
#endif

	DEBUG_TDMAPI("%s: TDM API State Event State=%i\n",
			tdm_api->name, tdm_api->state);

#if defined(__WINDOWS__)
	pevent = &event;
#else
	if (wan_skb_queue_len(&tdm_api->wp_event_list) > WP_TDM_MAX_EVENT_Q_LEN) {	
		return;
	}
	skb=wan_skb_alloc(sizeof(wp_tdm_api_event_t));
	if (skb == NULL) { 
		return;
	}
	pevent = (wp_tdm_api_event_t*)wan_skb_put(skb,sizeof(wp_tdm_api_event_t));
#endif/* #if !defined(__WINDOWS__) */
				
	memset(pevent,0,sizeof(wp_tdm_api_event_t));
	pevent->wp_tdm_api_event_type = WP_TDMAPI_EVENT_ALARM;
	pevent->wp_tdm_api_event_alarm = (state == WAN_CONNECTED) ? 0 : 1;

#if 0
	/* FIXME: NENAD TO ADD Timestamp  */
	rx_hdr->event_time_stamp = gettimeofday();
#endif					
	
	pevent->channel = (u_int16_t)tdm_api->tdm_chan;

#if defined(__WINDOWS__)
	aft_te1_insert_tdm_api_event_in_to_rx_queue(tdm_api, pevent);
	queue_tdm_api_rx_dpc(tdm_api);
#else
	wan_skb_queue_tail(&tdm_api->wp_event_list,skb);
#endif
}        

#if !defined(__WINDOWS__) 
static 
#endif
int wanpipe_tdm_api_ioctl(wanpipe_tdm_api_dev_t *tdm_api, struct ifreq *ifr)
{
	wanpipe_tdm_api_cmd_t usr_tdm_api, *utdmapi;
	int err=0;
	u32 cmd;
	wanpipe_codec_ops_t *wp_codec_ops;
	netskb_t *skb;
       	sdla_t *card = (sdla_t*)tdm_api->card;  

	utdmapi = (wanpipe_tdm_api_cmd_t*)ifr->ifr_data;
	
	if (!tdm_api->chan){
		DEBUG_EVENT("%s:%d Error: TDM API Not initialized! chan=NULL!\n",
			__FUNCTION__,__LINE__);
		return -EFAULT;
	}	
	
	if (!ifr){
		return -EINVAL;
	}
#if defined(__WINDOWS__)
	memcpy(&usr_tdm_api, ifr, sizeof(wanpipe_tdm_api_cmd_t));
#else
	if (WAN_COPY_FROM_USER(&usr_tdm_api,
			       utdmapi,
			       sizeof(wanpipe_tdm_api_cmd_t))){
	       return -EFAULT;
	}
#endif
	cmd=usr_tdm_api.cmd;

	DEBUG_TDMAPI("%s: TDM API CMD: %i\n",tdm_api->name,cmd);

	wan_spin_lock(&tdm_api->lock);

	if (tdm_api->hdlc_framing) {
		switch (cmd) {
		case SIOC_WP_TDM_GET_USR_MTU_MRU:
		case SIOC_WP_TDM_GET_STATS:
		case SIOC_WP_TDM_GET_FULL_CFG:
		case SIOC_WP_TDM_GET_FE_STATUS:
		case SIOC_WP_TDM_SET_FE_STATUS:
                case SIOC_WP_TDM_READ_EVENT:
                case SIOC_WP_TDM_GET_FE_ALARMS:
			break;
		default:
			DEBUG_EVENT("%s: Invalid TDM API HDLC CMD %i\n", tdm_api->name,cmd);
			err=-EOPNOTSUPP;
			goto tdm_api_exit;
		}
	}
	
	switch (cmd) {

	case SIOC_WP_TDM_SET_HW_MTU_MRU:
		err=-EOPNOTSUPP;
		break;

	case SIOC_WP_TDM_GET_HW_MTU_MRU:
		usr_tdm_api.hw_mtu_mru = tdm_api->cfg.hw_mtu_mru;
		break;
	
	case SIOC_WP_TDM_SET_USR_PERIOD:

		if (usr_tdm_api.usr_period >= 10 && 
		    (usr_tdm_api.usr_period % 10) == 0 &&
		    usr_tdm_api.usr_period <= 1000) {

			usr_tdm_api.usr_mtu_mru = usr_tdm_api.usr_period*tdm_api->cfg.hw_mtu_mru;

		} else {
			err = -EINVAL;
			DEBUG_EVENT("%s: TDM API: Invalid HW Period %i!\n",
					tdm_api->name,usr_tdm_api.usr_period);
			goto tdm_api_exit;
		}

		usr_tdm_api.usr_mtu_mru = wanpipe_codec_calc_new_mtu(tdm_api->cfg.tdm_codec,
			              				     usr_tdm_api.usr_mtu_mru);
		tdm_api->cfg.usr_period = usr_tdm_api.usr_period;
		tdm_api->cfg.usr_mtu_mru = usr_tdm_api.usr_mtu_mru;
		break;

	case SIOC_WP_TDM_GET_USR_PERIOD:

		usr_tdm_api.usr_period = tdm_api->cfg.usr_period;
		break;

	case SIOC_WP_TDM_GET_USR_MTU_MRU:

		usr_tdm_api.usr_mtu_mru = tdm_api->cfg.usr_mtu_mru;
		break;

	case SIOC_WP_TDM_SET_CODEC:
		
		if (usr_tdm_api.tdm_codec == tdm_api->cfg.tdm_codec){
			err = 0;
			goto tdm_api_exit;
		}

		if (usr_tdm_api.tdm_codec >= WP_TDM_CODEC_MAX){
			err = -EINVAL;
			goto tdm_api_exit;
		} 

		if (usr_tdm_api.tdm_codec == WP_NONE) {
			
			usr_tdm_api.usr_mtu_mru = tdm_api->cfg.hw_mtu_mru * tdm_api->cfg.usr_period;
			
		} else {
			wp_codec_ops = WANPIPE_CODEC_OPS[tdm_api->cfg.hw_tdm_coding][usr_tdm_api.tdm_codec];
			if (!wp_codec_ops || !wp_codec_ops->init){

				err = -EINVAL;
				DEBUG_EVENT("%s: TDM API: Codec %i is unsupported!\n",
						tdm_api->name,usr_tdm_api.tdm_codec);
				goto tdm_api_exit;
			}
			usr_tdm_api.usr_mtu_mru = wanpipe_codec_calc_new_mtu(usr_tdm_api.tdm_codec,
								      tdm_api->cfg.usr_mtu_mru);
		}

		tdm_api->cfg.usr_mtu_mru=usr_tdm_api.usr_mtu_mru;
		tdm_api->cfg.tdm_codec = usr_tdm_api.tdm_codec;
		break;

	case SIOC_WP_TDM_GET_CODEC:
		usr_tdm_api.tdm_codec = tdm_api->cfg.tdm_codec;
		break;

	case SIOC_WP_TDM_SET_POWER_LEVEL:
		tdm_api->cfg.power_level = usr_tdm_api.power_level;
		break;

	case SIOC_WP_TDM_GET_POWER_LEVEL:
		usr_tdm_api.power_level = tdm_api->cfg.power_level;
		break;

	case SIOC_WP_TDM_TOGGLE_RX:
		if (tdm_api->cfg.tx_disable){
			wan_clear_bit(0,&tdm_api->cfg.rx_disable);
		}else{
			wan_set_bit(0,&tdm_api->cfg.rx_disable);
		}
		break;

	case SIOC_WP_TDM_TOGGLE_TX:
		if (tdm_api->cfg.tx_disable){
			wan_clear_bit(0,&tdm_api->cfg.tx_disable);
		}else{
			wan_set_bit(0,&tdm_api->cfg.tx_disable);
		}
		break;

	case SIOC_WP_TDM_SET_EC_TAP:

		switch (usr_tdm_api.ec_tap){
			case 0:
			case 32:
			case 64:
			case 128:
				tdm_api->cfg.ec_tap = usr_tdm_api.ec_tap;
				break;

			default:
				DEBUG_EVENT("%s: Illegal Echo Cancellation Tap \n",
						tdm_api->name);	
				err = -EINVAL;
				goto tdm_api_exit;
		}		 
		break;

	case SIOC_WP_TDM_GET_EC_TAP:
		usr_tdm_api.ec_tap = tdm_api->cfg.ec_tap;
		break;


	case SIOC_WP_TDM_ENABLE_HWEC:
		if (card->wandev.ec_enable) {
			wan_smp_flag_t smp_flags1;
			card->hw_iface.hw_lock(card->hw,&smp_flags1);   
                	card->wandev.ec_enable(card, 1, tdm_api->tdm_chan);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);   
		}
		break;

	case SIOC_WP_TDM_DISABLE_HWEC:
		if (card->wandev.ec_enable) {
			wan_smp_flag_t smp_flags1;
			card->hw_iface.hw_lock(card->hw,&smp_flags1);   
                	card->wandev.ec_enable(card, 0, tdm_api->tdm_chan);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);   
		}
		break;	
		
	case SIOC_WP_TDM_GET_STATS:
		memcpy(&usr_tdm_api.stats,&tdm_api->cfg.stats,sizeof(tdm_api->cfg.stats));
		break;
		
	case SIOC_WP_TDM_GET_FULL_CFG:
		memcpy(&usr_tdm_api,&tdm_api->cfg,sizeof(wanpipe_tdm_api_cmd_t));
		break;

	case SIOC_WP_TDM_ENABLE_RBS_EVENTS:
		/* 'usr_tdm_api.rbs_poll' is the user provided 'number of polls per second' */
		if (usr_tdm_api.rbs_poll < 20 || usr_tdm_api.rbs_poll > 100) {
			DEBUG_EVENT("%s: Error: Invalid RBS Poll Count Min=20 Max=100\n",
					tdm_api->name);
			usr_tdm_api.rbs_poll=20;
		}
		usr_tdm_api.rbs_poll=HZ/usr_tdm_api.rbs_poll;
		
		tdm_api->cfg.rbs_poll = usr_tdm_api.rbs_poll;
		if (card->wandev.fe_iface.set_fe_sigctrl){
                 	card->wandev.fe_iface.set_fe_sigctrl(
					&card->fe,
					WAN_TE_SIG_POLL,
					ENABLE_ALL_CHANNELS,
					WAN_ENABLE);
		}
		break;
		
	case SIOC_WP_TDM_DISABLE_RBS_EVENTS:	
		tdm_api->cfg.rbs_poll=0;
		if (card->wandev.fe_iface.set_fe_sigctrl){
                 	card->wandev.fe_iface.set_fe_sigctrl(
					&card->fe,
					WAN_TE_SIG_POLL,
					ENABLE_ALL_CHANNELS,
					WAN_DISABLE);
		}
		break;

	case SIOC_WP_TDM_WRITE_RBS_BITS:
		wan_spin_unlock(&tdm_api->lock);
		 
		err=tdm_api->write_rbs_bits(
						tdm_api->chan, 
					    tdm_api->tdm_chan,
					    (u8)usr_tdm_api.rbs_tx_bits);
		if (err) {
			DEBUG_EVENT("%s: WRITE RBS Error (%i)\n",tdm_api->name,err);
		}
		goto tdm_api_unlocked_exit;
		break;

	case SIOC_WP_TDM_GET_FE_STATUS:
		if (card->wandev.fe_iface.get_fe_status){
			wan_smp_flag_t smp_flags1;
			card->hw_iface.hw_lock(card->hw,&smp_flags1);   
                 	card->wandev.fe_iface.get_fe_status(
					&card->fe, &usr_tdm_api.fe_status);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);   
		}
		break;
		
	case SIOC_WP_TDM_SET_FE_STATUS:	
		if (card->wandev.fe_iface.set_fe_status){
			wan_smp_flag_t smp_flags1;
			card->hw_iface.hw_lock(card->hw,&smp_flags1);   
                 	card->wandev.fe_iface.set_fe_status(
					&card->fe, usr_tdm_api.fe_status);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);   
		}
		break;

	case SIOC_WP_TDM_SET_EVENT:
		err = wanpipe_tdm_api_event_ioctl(tdm_api, &usr_tdm_api);
		break;
		
	case SIOC_WP_TDM_READ_EVENT:
		skb=wan_skb_dequeue(&tdm_api->wp_event_list);
		if (!skb){
			err=-ENOBUFS;
			break;
		}
		memcpy(&usr_tdm_api.event,wan_skb_data(skb),sizeof(wp_tdm_api_event_t));
		wan_skb_free(skb);
		break;

	case SIOC_WP_TDM_GET_HW_CODING:
		usr_tdm_api.hw_tdm_coding = tdm_api->cfg.hw_tdm_coding;
		break;
		
	case SIOC_WP_TDM_SET_RX_GAINS:

		if (usr_tdm_api.data_len && utdmapi->data) {
                        if (usr_tdm_api.data_len != 256) {
                         	err=-EINVAL;
				break;
			}
			
			wan_spin_unlock(&tdm_api->lock); 	
			
			if (!rx_gains) {
				rx_gains = wan_malloc(usr_tdm_api.data_len);
				if (!rx_gains) {
                         		err=-ENOMEM;
					goto tdm_api_unlocked_exit;
				}
			}
                	
#if defined(__WINDOWS__)
			/*FIXME: test the memcpy() here
			memcpy(rx_gains, utdmapi->data, usr_tdm_api.data_len);*/
#else
       			if (WAN_COPY_FROM_USER(rx_gains,
					       utdmapi->data,
					       usr_tdm_api.data_len)){
	       			err=-EFAULT;
				goto tdm_api_unlocked_exit;
			}
#endif      
		       	wan_spin_lock(&tdm_api->lock); 	

		       	tdm_api->rx_gain = rx_gains;

		} else {
	       	       	err=-EFAULT;
		}
		
		break;

	case SIOC_WP_TDM_GET_FE_ALARMS:
		usr_tdm_api.fe_alarms = (tdm_api->state == WAN_CONNECTED ? 0 : 1); 
		break;

	case SIOC_WP_TDM_SET_TX_GAINS:

		if (usr_tdm_api.data_len && utdmapi->data) {
                        if (usr_tdm_api.data_len != 256) {
                         	err=-EINVAL;
				break;
			}
			
			wan_spin_unlock(&tdm_api->lock); 	
			
			if (!tx_gains) {
				tx_gains = wan_malloc(usr_tdm_api.data_len);
				if (!tx_gains) {
                         		err=-ENOMEM;
					goto tdm_api_unlocked_exit;
				}
			}
                	
#if defined(__WINDOWS__)
			/*FIXME: test the memcpy() here
			memcpy(tx_gains, utdmapi->data, usr_tdm_api.data_len);*/
#else
       			if (WAN_COPY_FROM_USER(tx_gains,
					       utdmapi->data,
					       usr_tdm_api.data_len)){
	       			err=-EFAULT;
				goto tdm_api_unlocked_exit;
			}
#endif      
		       	wan_spin_lock(&tdm_api->lock); 	

		       	tdm_api->tx_gain = tx_gains;

		} else {
	       	       	err=-EFAULT;
		}
		
		break;   

	case SIOC_WP_TDM_CLEAR_RX_GAINS:
		tdm_api->rx_gain = NULL;
		break;
	
	case SIOC_WP_TDM_CLEAR_TX_GAINS: 
		tdm_api->tx_gain = NULL;
		break;
		
	default:
		DEBUG_EVENT("%s: Invalid TDM API CMD %i\n", tdm_api->name,cmd);
		err=-EOPNOTSUPP;
		break;
	} 

tdm_api_exit:
	wan_spin_unlock(&tdm_api->lock);
	
tdm_api_unlocked_exit:

#if defined(__WINDOWS__)
	memcpy(ifr, &usr_tdm_api, sizeof(wanpipe_tdm_api_cmd_t));
#else
	if (WAN_COPY_TO_USER(ifr->ifr_data,
			     &usr_tdm_api,
			     sizeof(wanpipe_tdm_api_cmd_t))){
	       return -EFAULT;
	}
#endif
	return err;
}

static int
wanpipe_tdm_api_event_ioctl(wanpipe_tdm_api_dev_t *tdm_api, wanpipe_tdm_api_cmd_t *tdm_cmd)
{
	wp_tdm_api_event_t	*tdm_event;
	wan_event_ctrl_t	event_ctrl;

	if (tdm_api->event_ctrl == NULL){	
		DEBUG_EVENT("%s: Error: Event control interface doesn't initialized!\n",
				tdm_api->name); 
		return -EINVAL;
	}
		
	tdm_event = &tdm_cmd->event;
	memset(&event_ctrl, 0, sizeof(wan_event_ctrl_t));

	switch(tdm_event->wp_tdm_api_event_type){		
	case WP_TDMAPI_EVENT_DTMF:
		// Octasic DTMF event
		DEBUG_TDMAPI("%s: %s HW EC DTMF event %X!\n",
			tdm_api->name, 
			WP_TDMAPI_EVENT_MODE_DECODE(tdm_event->wp_tdm_api_event_mode),
			tdm_api->active_ch);
		event_ctrl.type		= WAN_EVENT_EC_DTMF;	
		if (tdm_event->wp_tdm_api_event_mode == WP_TDMAPI_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
#if defined(__WINDOWS__)
		if(tdm_event->channel < 1 || tdm_event->channel > NUM_OF_E1_CHANNELS - 1/* 31 */){
			DEBUG_TDMAPI("%s(): %s: Warning: DTMF control requested on invalid channel %u!\n", 
				__FUNCTION__, tdm_api->name, tdm_event->channel);
			tdm_event->channel = 1;/* */
		}
		event_ctrl.channel	= tdm_event->channel;
#else
		event_ctrl.channel	= tdm_api->tdm_chan;
#endif
		break;
		
	case WP_TDMAPI_EVENT_RM_DTMF:
		// A200-Remora DTMF event
		DEBUG_TDMAPI("%s: %s A200-Remora DTMF event!\n",
			tdm_api->name, 
			WP_TDMAPI_EVENT_MODE_DECODE(tdm_event->wp_tdm_api_event_mode));
		event_ctrl.type		= WAN_EVENT_RM_DTMF;	
		if (tdm_event->wp_tdm_api_event_mode == WP_TDMAPI_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;			
		
	case WP_TDMAPI_EVENT_RXHOOK:
		DEBUG_TDMAPI("%s: %s A200-Remora Loop Closure event!\n",
			tdm_api->name,
			WP_TDMAPI_EVENT_MODE_DECODE(tdm_event->wp_tdm_api_event_mode));
		event_ctrl.type		= WAN_EVENT_RM_LC;	
		if (tdm_event->wp_tdm_api_event_mode == WP_TDMAPI_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;
		
	case WP_TDMAPI_EVENT_RING:
		DEBUG_TDMAPI("%s: %s Ring Event on module %d!\n",
			tdm_api->name,
			WP_TDMAPI_EVENT_MODE_DECODE(tdm_event->wp_tdm_api_event_mode),
			tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_RING;
		if (tdm_event->wp_tdm_api_event_mode == WP_TDMAPI_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;
	
	case WP_TDMAPI_EVENT_RING_DETECT:
		DEBUG_TDMAPI("%s: %s Ring Detection Event on module %d!\n",
			tdm_api->name,
			WP_TDMAPI_EVENT_MODE_DECODE(tdm_event->wp_tdm_api_event_mode),
			tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_RING_DETECT;
		if (tdm_event->wp_tdm_api_event_mode == WP_TDMAPI_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;
		
	case WP_TDMAPI_EVENT_RING_TRIP_DETECT:
		DEBUG_TDMAPI("%s: %s Ring Trip Detection Event on module %d!\n",
			tdm_api->name,
			WP_TDMAPI_EVENT_MODE_DECODE(tdm_event->wp_tdm_api_event_mode),
			tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_RING_TRIP;
		if (tdm_event->wp_tdm_api_event_mode == WP_TDMAPI_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;
		
	case WP_TDMAPI_EVENT_TONE:

		DEBUG_TDMAPI("%s: %s Tone Event (%d)on module %d!\n",
			tdm_api->name, 
			WP_TDMAPI_EVENT_MODE_DECODE(tdm_event->wp_tdm_api_event_mode),
			tdm_event->wp_tdm_api_event_tone_type,
			tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_TONE;
		if (tdm_event->wp_tdm_api_event_mode == WP_TDMAPI_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
			switch(tdm_event->wp_tdm_api_event_tone_type){
			case WP_TDMAPI_EVENT_TONE_DIAL:
				event_ctrl.tone	= WAN_EVENT_TONE_DIAL;
				break;
			case WP_TDMAPI_EVENT_TONE_BUSY:
				event_ctrl.tone	= WAN_EVENT_TONE_BUSY;
				break;
			case WP_TDMAPI_EVENT_TONE_RING:
				event_ctrl.tone	= WAN_EVENT_TONE_RING;
				break;
			case WP_TDMAPI_EVENT_TONE_CONGESTION:
				event_ctrl.tone	= WAN_EVENT_TONE_CONGESTION;
				break;
			default:
				DEBUG_EVENT("%s: Unsupported TDM API Tone Type  %d!\n",
						tdm_api->name, 
						tdm_event->wp_tdm_api_event_tone_type);
				return -EINVAL;
			}
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;
		
	case WP_TDMAPI_EVENT_TXSIG_KEWL:
		DEBUG_TDMAPI("%s: TX Signalling KEWL on module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_TXSIG_KEWL;	
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;
	
	case WP_TDMAPI_EVENT_TXSIG_START:
		DEBUG_TDMAPI("%s: TX Signalling START for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_TXSIG_START;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;
	
	case WP_TDMAPI_EVENT_TXSIG_OFFHOOK:
		DEBUG_TDMAPI("%s: TX Signalling OFFHOOK for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_TXSIG_OFFHOOK;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;
	
	case WP_TDMAPI_EVENT_TXSIG_ONHOOK:
		DEBUG_TDMAPI("%s: TX Signalling ONHOOK for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_TXSIG_ONHOOK;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;
	
	case WP_TDMAPI_EVENT_ONHOOKTRANSFER:
		DEBUG_TDMAPI("%s: RM ONHOOKTRANSFER for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_ONHOOKTRANSFER;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		event_ctrl.ohttimer	= tdm_event->wp_tdm_api_event_ohttimer;
		break;
	
	case WP_TDMAPI_EVENT_SETPOLARITY:
		DEBUG_EVENT("%s: RM SETPOLARITY for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_SETPOLARITY;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		event_ctrl.polarity	= tdm_event->wp_tdm_api_event_polarity;
		break;

	case WP_TDMAPI_EVENT_BRI_CHAN_LOOPBACK:
		event_ctrl.type		= WAN_EVENT_BRI_CHAN_LOOPBACK;
		event_ctrl.channel	= tdm_event->channel;

		if (tdm_event->wp_tdm_api_event_mode == WP_TDMAPI_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}

		DEBUG_TDMAPI("%s: BRI_BCHAN_LOOPBACK: %s for channel %d!\n",
			tdm_api->name, 
			(event_ctrl.mode == WAN_EVENT_ENABLE ? "Enable" : "Disable"),
			event_ctrl.channel);
		break;

	default:
		DEBUG_EVENT("%s: Unknown TDM API Event Type %02X!\n",
				tdm_api->name,
				tdm_event->type);
		return -EINVAL;
	}


	switch(tdm_event->wp_tdm_api_event_type){
	case WP_TDMAPI_EVENT_BRI_CHAN_LOOPBACK:
		/* BRI FE access must be locked */
		{
			sdla_t *card = (sdla_t*)tdm_api->card;
			wan_smp_flag_t smp_flags1;
			int rc;

			card->hw_iface.hw_lock(card->hw,&smp_flags1);   
			rc = tdm_api->event_ctrl(tdm_api->chan, &event_ctrl);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);   
			return rc;
		}
		break;

	default:
		return tdm_api->event_ctrl(tdm_api->chan, &event_ctrl);
	}
}

static int wanpipe_tdm_api_tx (wanpipe_tdm_api_dev_t *tdm_api, u8 *tx_data, int len)
{
	u8 *buf;
	wp_tdm_api_tx_hdr_t *tx_hdr;
	
	if (wan_test_bit(0,&tdm_api->cfg.tx_disable)){
		return 0;
	}

#if !defined(__WINDOWS__)	 
	if (!tdm_api->tx_skb) {
		tdm_api->tx_skb=wan_skb_dequeue(&tdm_api->wp_tx_list);	
		if (!tdm_api->tx_skb){	
			memset(tx_data,tdm_api->cfg.idle_flag,len);
			tdm_api->cfg.stats.tx_carrier_errors++;
			return -ENOBUFS;
		}
		
		wp_wakeup_tdmapi(tdm_api);
		buf=wan_skb_pull(tdm_api->tx_skb,sizeof(wp_tdm_api_tx_hdr_t));
		memcpy(&tdm_api->tx_hdr, buf, sizeof(wp_tdm_api_tx_hdr_t));
		
		if (wan_skb_len(tdm_api->tx_skb) % len) {
			wan_skb_free(tdm_api->tx_skb);
			tdm_api->tx_skb=NULL;
			tdm_api->cfg.stats.tx_errors++;
			return -EINVAL;
		}
	}
	
	tx_hdr=&tdm_api->tx_hdr;
	
	buf=(u8*)wan_skb_data(tdm_api->tx_skb);
	memcpy(tx_data,buf,len);
	wan_skb_pull(tdm_api->tx_skb,len);
	
	if (wan_skb_len(tdm_api->tx_skb) <= 0) {
	
		/* Recycle the buffer so we dont have to realloc */
		wan_skb_push(tdm_api->tx_skb,sizeof(wp_tdm_api_tx_hdr_t));
		wan_skb_init(tdm_api->tx_skb,16);
		wan_skb_trim(tdm_api->tx_skb,0);
		wan_skb_queue_tail(&tdm_api->wp_tx_free_list,tdm_api->tx_skb);
		
	
		tdm_api->tx_skb=NULL;
		tdm_api->cfg.stats.tx_packets++;
	}
#endif	
	return 0;
}

static int wanpipe_tdm_api_rx (wanpipe_tdm_api_dev_t *tdm_api, u8 *rx_data, u8 *tx_data, int len)
{
	u8 *data_ptr;
	wp_tdm_api_rx_hdr_t *rx_hdr;
	int err=-EINVAL;

#if !defined(__WINDOWS__)	
	if (wan_test_bit(0,&tdm_api->cfg.rx_disable)) {
		err=0;
		goto wanpipe_tdm_api_rx_error;
	}

	if (wan_skb_queue_len(&tdm_api->wp_rx_list) > WP_TDM_MAX_RX_Q_LEN) {
		tdm_api->cfg.stats.rx_fifo_errors++;
		err=0;
		goto wanpipe_tdm_api_rx_error;
	}
	
	if (!tdm_api->rx_skb) {
		tdm_api->rx_skb=wan_skb_dequeue(&tdm_api->wp_rx_free_list);
		if (!tdm_api->rx_skb) {
			tdm_api->rx_skb=wan_skb_alloc(WP_TDM_API_MAX_LEN);	
			if (!tdm_api->rx_skb){
				err=-ENOMEM;
				tdm_api->cfg.stats.rx_errors++;
				goto wanpipe_tdm_api_rx_error;
			}
		}
		data_ptr=wan_skb_put(tdm_api->rx_skb,sizeof(wp_tdm_api_rx_hdr_t));
	}
	
	if (wan_skb_len(tdm_api->rx_skb)+len >= WP_TDM_API_MAX_LEN) {
		err=-EINVAL;
		tdm_api->cfg.stats.rx_errors++;
		goto wanpipe_tdm_api_rx_error;
	}
	
	rx_hdr=(wp_tdm_api_rx_hdr_t*)wan_skb_data(tdm_api->rx_skb);
	
	data_ptr=wan_skb_put(tdm_api->rx_skb,len);
	memcpy((u8*)data_ptr,rx_data,len);
	
#if 0
	if (tdm_api->cfg.tdm_codec == WP_NONE) {
	
		data_ptr=wan_skb_put(tdm_api->rx_skb,len);
		memcpy((u8*)data_ptr,rx_data,len);
		
	} else {
		wanpipe_codec_ops_t *wp_codec_ops;
		wan_smp_flag_t smp_flags;
		u32 power;
		
		wan_spin_lock_irq(&tdm_api->lock,&smp_flags);
	
		wp_codec_ops=WANPIPE_CODEC_OPS[tdm_api->cfg.hw_tdm_coding][tdm_api->cfg.tdm_codec];
		if (!wp_codec_ops || !wp_codec_ops->init){
			wan_spin_unlock_irq(&tdm_api->lock,&smp_flags);
			err=-EINVAL;
			tdm_api->cfg.stats.rx_errors++;
			goto wanpipe_tdm_api_rx_error;	
		}
	
		wan_spin_unlock_irq(&tdm_api->lock,&smp_flags);
	
		data_ptr=(u8*)wan_skb_data(tdm_api->rx_skb);
		
		len = wp_codec_ops->encode(rx_data,
					   len,
					   (u16*)&data_ptr[wan_skb_len(tdm_api->rx_skb)],
					   &power, 0);	
		  
		if (len < 0){
			err=len;
			tdm_api->cfg.stats.rx_errors++;
			goto wanpipe_tdm_api_rx_error;
		}
		
		wan_skb_put(tdm_api->rx_skb,len);
	}
#endif	
	
	if (wan_skb_len(tdm_api->rx_skb) >= 
	   (tdm_api->cfg.usr_period*tdm_api->cfg.hw_mtu_mru + sizeof(wp_tdm_api_rx_hdr_t))) {
		wan_skb_queue_tail(&tdm_api->wp_rx_list,tdm_api->rx_skb);				
		tdm_api->rx_skb=NULL;
		wp_wakeup_tdmapi(tdm_api);
		tdm_api->cfg.stats.rx_packets++;
	}
	 
	
	return 0;
	
wanpipe_tdm_api_rx_error:

	if (tdm_api->rx_skb) {
		wan_skb_free(tdm_api->rx_skb);
	}
	
	tdm_api->rx_skb=NULL;

#endif /* #if !defined(__WINDOWS__)	*/
	return err;
}


int wanpipe_tdm_api_rx_tx (wanpipe_tdm_api_dev_t *tdm_api, u8 *rx_data, u8 *tx_data, int len)
{
	if (!tdm_api->chan || !wan_test_bit(0,&tdm_api->init)){
		return 0;
	}
	
	if (!wan_test_bit(0,&tdm_api->used)) {
		if (tx_data[0] != tdm_api->cfg.idle_flag) {
			memset(tx_data,tdm_api->cfg.idle_flag,len);
		}
		return 0;
	}
	
	wanpipe_tdm_api_rx (tdm_api, rx_data, tx_data, len);
	wanpipe_tdm_api_tx (tdm_api, tx_data, len);

	
	return 0;
}

int wanpipe_tdm_api_rx_hdlc (wanpipe_tdm_api_dev_t *tdm_api, netskb_t *skb)
{
#if !defined(__WINDOWS__)
	if (wan_skb_queue_len(&tdm_api->wp_rx_list) > WP_TDM_MAX_RX_Q_LEN) {
		tdm_api->cfg.stats.rx_fifo_errors++;
		return -EBUSY;
	}
	
	DEBUG_TDMAPI("%s: TDM API RX HDLC FRAME %i\n",tdm_api->name, wan_skb_len(skb));

	wan_skb_queue_tail(&tdm_api->wp_rx_list,skb);
	wp_wakeup_tdmapi(tdm_api);
	tdm_api->cfg.stats.rx_packets++;
#endif
	return 0;
}

static wanpipe_tdm_api_dev_t *wp_tdmapi_search(sdla_t *card, int fe_chan)
{
	wanpipe_tdm_api_dev_t	*tdm_api;
	int	i = 0;
	
#if defined(__WINDOWS__)

	DEBUG_TDMAPI("%s(): fe_chan: %d\n", __FUNCTION__, fe_chan);

	if(fe_chan < 0 || fe_chan >= MAX_TDM_API_CHANNELS){
		DEBUG_EVENT("%s(): TDM API Error: Invalid Channel Number=%i!\n",
			__FUNCTION__, fe_chan);
		return NULL;
	}

	tdm_api = card->wp_tdmapi_hash[fe_chan];
	if(tdm_api == NULL){
		DEBUG_EVENT("%s(): TDM API Warning: No TDM API registered for Channel Number=%i!\n",
			__FUNCTION__, fe_chan);
	}
	return tdm_api;
#else
	for(i = 0; i < WP_TDMAPI_HASH_SZ; i++){
		tdm_api = wp_tdmapi_hash[i];
		
		if (tdm_api == NULL || tdm_api->card != card){
			continue;
		}
		if (wan_test_bit(fe_chan, &tdm_api->active_ch)){
			return tdm_api;		 
		}
	}
	return NULL;
#endif
}

static void wp_tdmapi_rbsbits(void* card_id, int channel, unsigned char rbsbits)
{
	sdla_t	*card = (sdla_t*)card_id;
	
	DEBUG_EVENT("%s: Received RBS Event at TDM_API (not supported)!\n",
					card->devname);
	return;
}

static void wp_tdmapi_alarms(void* card_id, unsigned long alarams)
{
	sdla_t	*card = (sdla_t*)card_id;
	
	DEBUG_EVENT("%s: Received RBS Event at TDM_API (not supported)!\n",
					card->devname);
	return;
}

static void wp_tdmapi_dtmf (void* card_id, wan_event_t *event)
{
	netskb_t			*skb = NULL;
	wanpipe_tdm_api_dev_t	*tdm_api = NULL;
	sdla_t				*card = (sdla_t*)card_id;
	wp_tdm_api_event_t	*p_tdmapi_event = NULL;
#if defined(__WINDOWS__)
	wp_tdm_api_event_t	tdmapi_event;
#endif

	if (event->type == WAN_EVENT_EC_DTMF){
		DEBUG_TDMAPI("%s: Received DTMF Event at TDM API (%d:%c:%s:%s)!\n",
			card->devname,
			event->channel,
			event->digit,
			(event->dtmf_port == WAN_EC_CHANNEL_PORT_ROUT)?"ROUT":"SOUT",
			(event->dtmf_type == WAN_EC_TONE_PRESENT)?"PRESENT":"STOP");
	}else if (event->type == WAN_EVENT_RM_DTMF){
		DEBUG_TDMAPI("%s: Received DTMF Event at TDM API (%d:%c)!\n",
			card->devname,
			event->channel,
			event->digit);	
	}
					
	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}

#if defined(__WINDOWS__)
	p_tdmapi_event = &tdmapi_event;
#else
	if (wan_skb_queue_len(&tdm_api->wp_event_list) > WP_TDM_MAX_EVENT_Q_LEN) {
		return;
	}
	skb=wan_skb_alloc(sizeof(wp_tdm_api_event_t));
	if (skb == NULL) return;
	
	p_tdmapi_event = (wp_tdm_api_event_t*)wan_skb_put(skb,sizeof(wp_tdm_api_event_t));
#endif

	memset(p_tdmapi_event,0,sizeof(wp_tdm_api_event_t));
	p_tdmapi_event->type				= WP_TDMAPI_EVENT_DTMF;
	p_tdmapi_event->wp_tdm_api_event_dtmf_digit	= event->digit;
	p_tdmapi_event->wp_tdm_api_event_dtmf_type	= event->dtmf_type;
	p_tdmapi_event->wp_tdm_api_event_dtmf_port	= event->dtmf_port;
#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif					

#if defined(__WINDOWS__)
	p_tdmapi_event->channel = (u_int16_t)event->channel; 
	aft_te1_insert_tdm_api_event_in_to_rx_queue(tdm_api, p_tdmapi_event);
	queue_tdm_api_rx_dpc(tdm_api);
#else
	wan_skb_queue_tail(&tdm_api->wp_event_list,skb);
	wp_wakeup_tdmapi(tdm_api);
#endif
	return;
}

static void wp_tdmapi_hook (void* card_id, wan_event_t *event)
{
	netskb_t		*skb;
	wanpipe_tdm_api_dev_t	*tdm_api = NULL;
	sdla_t			*card = (sdla_t*)card_id;
	wp_tdm_api_event_t	*p_tdmapi_event = NULL;
#if defined(__WINDOWS__)
	wp_tdm_api_event_t	tdmapi_event;
#endif
	
	DEBUG_TDMAPI("%s: Received RM LC Event at TDM_API (%d:%s)!\n",
			card->devname,
			event->channel,
			(event->rxhook==WAN_EVENT_RXHOOK_OFF)?"OFF-HOOK":"ON-HOOK");

	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}
					
#if defined(__WINDOWS__)
	p_tdmapi_event = &tdmapi_event;
#else
	if (wan_skb_queue_len(&tdm_api->wp_event_list) > WP_TDM_MAX_EVENT_Q_LEN) {
		return;
	}
	skb=wan_skb_alloc(sizeof(wp_tdm_api_event_t));
	if (skb == NULL) return;
	
	p_tdmapi_event = (wp_tdm_api_event_t*)wan_skb_put(skb,sizeof(wp_tdm_api_event_t));
#endif

	memset(p_tdmapi_event, 0, sizeof(wp_tdm_api_event_t));
	p_tdmapi_event->type	= WP_TDMAPI_EVENT_RXHOOK;
	p_tdmapi_event->channel	= (u_int16_t)event->channel;
	switch(event->rxhook){
	case WAN_EVENT_RXHOOK_ON:
		p_tdmapi_event->wp_tdm_api_event_hook_state = 
					WP_TDMAPI_EVENT_RXHOOK_ON;
		break;
	case WAN_EVENT_RXHOOK_OFF:
		p_tdmapi_event->wp_tdm_api_event_hook_state = 
					WP_TDMAPI_EVENT_RXHOOK_OFF;
		break;
	}

#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif					

#if defined(__WINDOWS__)
	aft_te1_insert_tdm_api_event_in_to_rx_queue(tdm_api, p_tdmapi_event);
	queue_tdm_api_rx_dpc(tdm_api);
#else
	wan_skb_queue_tail(&tdm_api->wp_event_list,skb);
	wp_wakeup_tdmapi(tdm_api);
#endif
	return;
}

/*
FXS:
A ring trip event signals that the terminal equipment has gone
off-hook during the ringing state.
At this point the application should stop the ring because the
call was answered.
*/
static void wp_tdmapi_ringtrip (void* card_id, wan_event_t *event)
{
	netskb_t		*skb;
	wanpipe_tdm_api_dev_t	*tdm_api = NULL;
	sdla_t			*card = (sdla_t*)card_id;
	wp_tdm_api_event_t	*p_tdmapi_event = NULL;
#if defined(__WINDOWS__)
	wp_tdm_api_event_t	tdmapi_event;
#endif
	
	DEBUG_TDMAPI("%s: Received RM RING TRIP Event at TDM_API (%d:%s)!\n",
			card->devname,
			event->channel,
			WAN_EVENT_RING_TRIP_DECODE(event->ring_mode));
	
	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}
					
#if defined(__WINDOWS__)
	p_tdmapi_event = &tdmapi_event;
#else
	if (wan_skb_queue_len(&tdm_api->wp_event_list) > WP_TDM_MAX_EVENT_Q_LEN) {
		return;
	}
	skb=wan_skb_alloc(sizeof(wp_tdm_api_event_t));
	if (skb == NULL) return;
	
	p_tdmapi_event = (wp_tdm_api_event_t*)wan_skb_put(skb,sizeof(wp_tdm_api_event_t));
#endif

	memset(p_tdmapi_event, 0, sizeof(wp_tdm_api_event_t));
	p_tdmapi_event->type	= WP_TDMAPI_EVENT_RING_TRIP_DETECT;
	p_tdmapi_event->channel	= (u_int16_t)event->channel; 
	if (event->ring_mode == WAN_EVENT_RING_TRIP_STOP){
		p_tdmapi_event->wp_tdm_api_event_ring_state = 
				WP_TDMAPI_EVENT_RING_TRIP_STOP;
	}else if (event->ring_mode == WAN_EVENT_RING_TRIP_PRESENT){
		p_tdmapi_event->wp_tdm_api_event_ring_state = 
				WP_TDMAPI_EVENT_RING_TRIP_PRESENT;	
	}

#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif					

#if defined(__WINDOWS__)
	aft_te1_insert_tdm_api_event_in_to_rx_queue(tdm_api, p_tdmapi_event);
	queue_tdm_api_rx_dpc(tdm_api);
#else
	wan_skb_queue_tail(&tdm_api->wp_event_list,skb);
	wp_wakeup_tdmapi(tdm_api);
#endif
	return;
}

/*
FXO:
received a ring Start/Stop from the other side.
*/
static void wp_tdmapi_ringdetect (void* card_id, wan_event_t *event)
{
	netskb_t		*skb;
	wanpipe_tdm_api_dev_t	*tdm_api = NULL;
	sdla_t			*card = (sdla_t*)card_id;
	wp_tdm_api_event_t	*p_tdmapi_event = NULL;
#if defined(__WINDOWS__)
	wp_tdm_api_event_t	tdmapi_event;
#endif
	
	DEBUG_TDMAPI("%s: Received RM RING DETECT Event at TDM_API (%d:%s)!\n",
			card->devname,
			event->channel,
			WAN_EVENT_RING_DECODE(event->ring_mode));
	
	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}
					
#if defined(__WINDOWS__)
	p_tdmapi_event = &tdmapi_event;
#else
	if (wan_skb_queue_len(&tdm_api->wp_event_list) > WP_TDM_MAX_EVENT_Q_LEN) {
		return;
	}
	skb=wan_skb_alloc(sizeof(wp_tdm_api_event_t));
	if (skb == NULL) return;
	
	p_tdmapi_event = (wp_tdm_api_event_t*)wan_skb_put(skb,sizeof(wp_tdm_api_event_t));
#endif

	memset(p_tdmapi_event, 0, sizeof(wp_tdm_api_event_t));

	p_tdmapi_event->type	= WP_TDMAPI_EVENT_RING_DETECT;
	p_tdmapi_event->channel	= (u_int16_t)event->channel; 
	switch(event->ring_mode){
	case WAN_EVENT_RING_PRESENT:
		p_tdmapi_event->wp_tdm_api_event_ring_state = 
					WP_TDMAPI_EVENT_RING_PRESENT;
		break;
	case WAN_EVENT_RING_STOP:
		p_tdmapi_event->wp_tdm_api_event_ring_state = 
					WP_TDMAPI_EVENT_RING_STOP;
		break;
	}

#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif					

#if defined(__WINDOWS__)
	aft_te1_insert_tdm_api_event_in_to_rx_queue(tdm_api, p_tdmapi_event);
	queue_tdm_api_rx_dpc(tdm_api);
#else
	wan_skb_queue_tail(&tdm_api->wp_event_list,skb);
	wp_wakeup_tdmapi(tdm_api);
#endif
	return;
}

#if defined(__WINDOWS__)

static int store_tdm_api_pointer_in_card(sdla_t *card, wanpipe_tdm_api_dev_t *tdm_api)
{
	int		i;

	TDM_FUNC_DBG();

	DEBUG_TDMAPI("%s(): original_active_ch: 0x%X\n", __FUNCTION__, tdm_api->original_active_ch);

	for(i = 0; i < NUM_OF_E1_CHANNELS; i++){
		if(tdm_api->original_active_ch & (1 << i)){

			DEBUG_TDMAPI("%s(): setting channel: %d\n", __FUNCTION__, i);

			if(i >= MAX_TDM_API_CHANNELS){
				DEBUG_EVENT("%s(): TDM API Error (TE1): Invalid Channel Number=%i (Span=%d)!\n",
					__FUNCTION__, i, tdm_api->tdm_span);
				return 1;
			}

			if(card->wp_tdmapi_hash[i] != NULL){
				DEBUG_EVENT("%s(): TDM API Error (TE1): device SPAN=%i CHAN=%i already in use!\n",
					__FUNCTION__, tdm_api->tdm_span, i);
				return 1;
			}
			card->wp_tdmapi_hash[i] = tdm_api;
		}//if()
	}//for()
	return 0;
}

static int remove_tdm_api_pointer_from_card(wanpipe_tdm_api_dev_t *tdm_api)
{
	sdla_t	*card = NULL;
	int		i;

	TDM_FUNC_DBG();

	WAN_ASSERT(tdm_api == NULL);
	card = (sdla_t*)tdm_api->card;

	if(card == NULL){
		DEBUG_EVENT("%s(): TDM API Error: Invalid 'card' pointer!\n",
			__FUNCTION__);
		return 1;
	}

	DEBUG_TDMAPI("%s(): original_active_ch: 0x%X\n", __FUNCTION__, tdm_api->original_active_ch);

	for(i = 0; i < NUM_OF_E1_CHANNELS; i++){
		if(tdm_api->original_active_ch & (1 << i)){

			DEBUG_TDMAPI("%s(): RE-setting channel: %d\n", __FUNCTION__, i);

			if(i >= MAX_TDM_API_CHANNELS){
				DEBUG_EVENT("%s(): TDM API Error (TE1): Invalid Channel Number=%i (Span=%d)!\n",
					__FUNCTION__, i, tdm_api->tdm_span);
				return 1;
			}

			if(card->wp_tdmapi_hash[i] == NULL){
				DEBUG_EVENT("%s: TDM API Warning (TE1): device SPAN=%i CHAN=%i was NOT in use!\n",
					__FUNCTION__, tdm_api->tdm_span, tdm_api->tdm_chan);
			}
			card->wp_tdmapi_hash[i] = NULL;
		}//if()
	}//for()

	return 0;
}

//when transmitting: convert from s-linear to MuLaw/ALaw.
//decoded data length will be 1/2 of original data length.
int lib_sang_api_decode(void *tdm_api_ptr,
						void *original_databuf,	int original_datalen,
						void *new_databuf,		int *new_datalen)
{
	int len;	
	wanpipe_codec_ops_t *wp_codec_ops;
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t *)tdm_api_ptr;

	DBG_TDMCODEC("%s(): IN original_datalen: %d\n", __FUNCTION__, original_datalen);

	if (tdm_api->hdlc_framing){
		DBG_TDMCODEC("%s(): HDLC framing\n", __FUNCTION__);
		return TDMAPI_BUFFER_HDLC_DATA;
	}

	if(tdm_api->cfg.tdm_codec == WP_NONE) {
		//nothing to do
		DBG_TDMCODEC("%s(): no codec\n", __FUNCTION__);
		return TDMAPI_BUFFER_NO_CODEC;		
	}

	wp_codec_ops=WANPIPE_CODEC_OPS[tdm_api->cfg.hw_tdm_coding][tdm_api->cfg.tdm_codec];
	if (!wp_codec_ops || !wp_codec_ops->init){
		DBG_TDMCODEC("%s(): Error: !wp_codec_ops || !wp_codec_ops->init !!!\n", __FUNCTION__);
		return TDMAPI_BUFFER_ERROR;	
	}
	
	len = wp_codec_ops->decode((u16*)original_databuf,
				   original_datalen,
				   new_databuf, 
				   tdm_api->tx_gain,
				   0);
	if (len <= 0) {
		DBG_TDMCODEC("%s(): Error: len: %d !!!\n", __FUNCTION__, len);
		return TDMAPI_BUFFER_ERROR;
	}	

	*new_datalen = len;
	return TDMAPI_BUFFER_READY;
}

//when receiving: convert from MuLaw/ALaw to s-linear.
//decoded data length will be 1/2 of original data length.
int lib_sang_api_encode(void *tdm_api_ptr,
						void *original_databuf,	int original_datalen,
						void *new_databuf,		int *new_datalen)
{
	int len;	
	wanpipe_codec_ops_t *wp_codec_ops;
	u32 power;
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t *)tdm_api_ptr;

	DBG_TDMCODEC("%s(): IN original_datalen: %d\n", __FUNCTION__, original_datalen);

	if (tdm_api->hdlc_framing){
		//nothing to do
		DBG_TDMCODEC("%s(): HDLC data\n", __FUNCTION__);
		return TDMAPI_BUFFER_HDLC_DATA;
	}
	
	if(tdm_api->cfg.tdm_codec == WP_NONE) {
		//nothing to do
		DBG_TDMCODEC("%s(): no codec\n", __FUNCTION__);
		return TDMAPI_BUFFER_NO_CODEC;
	}

	wp_codec_ops=WANPIPE_CODEC_OPS[tdm_api->cfg.hw_tdm_coding][tdm_api->cfg.tdm_codec];
	if (!wp_codec_ops || !wp_codec_ops->init){
		DBG_TDMCODEC("%s(): Error: !wp_codec_ops || !wp_codec_ops->init !!!\n", __FUNCTION__);
		return TDMAPI_BUFFER_ERROR;	
	}

	len = wp_codec_ops->encode(original_databuf,
				   original_datalen,
				   (u16*)new_databuf,
				   &power,
				   tdm_api->rx_gain,
				   0);
	if (len <= 0) {
		DG_TDMCODEC("%s(): Error: len: %d !!!\n", __FUNCTION__, len);
		return TDMAPI_BUFFER_ERROR;
	}	

	*new_datalen = len;
	return 0;
}

int lib_sang_api_rx(void *tdm_api_ptr,
			   void *rx_data, int rx_data_len,
			   char *destination_buf, unsigned int *destination_buf_datalen)
{
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t *)tdm_api_ptr;

	if (tdm_api->hdlc_framing) {
		//nothing to do
		DBG_TDM_RX("%s(): HDLC data\n", __FUNCTION__);
		return TDMAPI_BUFFER_HDLC_DATA;
	}

	//check length 
	if(	*destination_buf_datalen + rx_data_len >= WP_TDM_API_MAX_LEN){
		DEBUG_EVENT("%s(): TDM API Error : SPAN=%i CHAN=%i: data too long!\n",
				__FUNCTION__, tdm_api->tdm_span, tdm_api->tdm_chan);
		return TDMAPI_BUFFER_ERROR;
	}

	memcpy(&destination_buf[*destination_buf_datalen], rx_data, rx_data_len);
	*destination_buf_datalen += rx_data_len;

	if(	*destination_buf_datalen >= tdm_api->cfg.usr_period*tdm_api->cfg.hw_mtu_mru){
		//data reached desired length
		DBG_TDM_RX("%s(): data reached desired length: %d\n",
			__FUNCTION__, *destination_buf_datalen);
		return TDMAPI_BUFFER_READY;
	}

	DBG_TDM_RX("%s(): TDMAPI_BUFFER_ACCEPTED, len: %d\n",
			__FUNCTION__, *destination_buf_datalen);

	return TDMAPI_BUFFER_ACCEPTED;
}
#endif/*#if defined(__WINDOWS__)*/

#else

int wanpipe_tdm_api_reg(wanpipe_tdm_api_dev_t *tdm_api)
{
	return -EINVAL;
}

int wanpipe_tdm_api_unreg(wanpipe_tdm_api_dev_t *tdm_api)
{
	return -EINVAL;
}
 
int wanpipe_tdm_api_update_state(wanpipe_tdm_api_dev_t *tdm_api, int state)
{
	return -ENODEV;
}

int wanpipe_tdm_api_kick(wanpipe_tdm_api_dev_t *tdm_api)
{
	return -EINVAL;
}                                            

int wanpipe_tdm_api_rx_hdlc (wanpipe_tdm_api_dev_t *tdm_api, netskb_t *skb)
{
 	return -EINVAL;
}

int wanpipe_tdm_api_rx_tx (wanpipe_tdm_api_dev_t *tdm_api, u8 *rx_data, u8 *tx_data, int len)
{
	return -EINVAL;
}

#endif /* #if defined(BUILD_TDMV_API) */
