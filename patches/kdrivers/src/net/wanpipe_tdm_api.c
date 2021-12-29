/*****************************************************************************
* wanpipe_tdm_api.c
*
* 		WANPIPE(tm) AFT TE1 Hardware Support
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003-2009 Sangoma Technologies Inc.
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
* Nov 19, 2008	David Rokhvarg	<davidr@sangoma.com>
*				Added RBS Chan/Span poll.
*****************************************************************************/


#include "wanpipe_includes.h"
#include "wanpipe_defines.h"
#include "wanpipe_debug.h"
#include "wanpipe_common.h"
#include "wanpipe_cfg.h"
#include "wanpipe.h"
#include "wanpipe_tdm_api.h"
#include "wanpipe_cdev_iface.h"
#include "wanpipe_timer_iface.h"

#if defined(__WINDOWS__)

#define POLLWRNORM	0
#define POLLRDNORM	0

#endif/* __WINDOWS__ */


/*==============================================================
  Defines
 */

#if !defined(__WINDOWS__)
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#  define BUILD_TDMV_API
# endif/* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
#endif/* #if !defined(__WINDOWS__) */


#if defined(BUILD_TDMV_API)

#define WP_TDM_API_MAX_PERIOD 50 /* 50ms */

#define WP_TDM_MAX_RX_Q_LEN 10
#define WP_TDM_MAX_TX_Q_LEN 5
#define WP_TDM_MAX_HDLC_TX_Q_LEN 17
#define WP_TDM_MAX_EVENT_Q_LEN 200
#define WP_TDM_MAX_CTRL_EVENT_Q_LEN 20000 /* 500 channels * 40 events (At same time) */
#define WP_TDM_MAX_RX_FREE_Q_LEN 10


#undef  DEBUG_API_WAKEUP
#undef  DEBUG_API_READ
#undef  DEBUG_API_WRITE
#undef  DEBUG_API_POLL

#ifdef __WINDOWS__
#define wptdm_os_lock_irq(card,flags)   wan_spin_lock_irq(card,flags)
#define wptdm_os_unlock_irq(card,flags) wan_spin_unlock_irq(card,flags)
#else
#define wptdm_os_lock_irq(card,flags)
#define wptdm_os_unlock_irq(card,flags)
#endif

#define MAX_TDM_API_CHANNELS 32

static int wp_tdmapi_global_cnt=0;
static u8 *rx_gains;
static u8 *tx_gains;
static wanpipe_tdm_api_dev_t tdmapi_ctrl;
/*==============================================================
  Prototypes
 */

static int wp_tdmapi_open(void *obj);
static int wp_tdmapi_read_msg(void *obj , netskb_t **skb,  wp_api_hdr_t *hdr, int count);
static int wp_tdmapi_write_msg(void *obj , netskb_t *skb, wp_api_hdr_t *hdr);
static int wp_tdmapi_release(void *obj);

static void wp_tdmapi_report_rbsbits(void* card_id, int channel, unsigned char rbsbits);
static void wp_tdmapi_report_alarms(void* card_id, unsigned long te_alarm);
static void wp_tdmapi_tone(void* card_id, wan_event_t*);
static void wp_tdmapi_hook(void* card_id, wan_event_t*);
static void wp_tdmapi_ringtrip(void* card_id, wan_event_t*);
static void wp_tdmapi_ringdetect (void* card_id, wan_event_t *event);
static void wp_tdmapi_linkstatus(void* card_id, wan_event_t*);
static void wp_tdmapi_polarityreverse(void* card_id, wan_event_t*);


static int store_tdm_api_pointer_in_card(sdla_t *card, wanpipe_tdm_api_dev_t *tdm_api);
static int remove_tdm_api_pointer_from_card(wanpipe_tdm_api_dev_t *tdm_api);


static int wanpipe_tdm_api_ioctl(void *obj, int cmd, void *udata);
static u_int32_t wp_tdmapi_poll(void *obj);
static int wanpipe_tdm_api_event_ioctl(wanpipe_tdm_api_dev_t*, wanpipe_api_cmd_t*);
static wanpipe_tdm_api_dev_t *wp_tdmapi_search(sdla_t *card, int fe_chan);

static int wanpipe_tdm_api_handle_event(wanpipe_tdm_api_dev_t *tdm_api, netskb_t *skb);

/*==============================================================
  Global Variables
 */

enum {
	WP_API_FLUSH_INVAL,
	WP_API_FLUSH_ALL,
	WP_API_FLUSH_RX,
	WP_API_FLUSH_TX,
	WP_API_FLUSH_EVENT,
	WP_API_FLUSH_LOCK,
	WP_API_FLUSH_NO_LOCK
};

static wanpipe_cdev_ops_t wp_tdmapi_fops;


static void wp_tdmapi_init_buffs(wanpipe_tdm_api_dev_t *tdm_api, int option)
{

#define wptdm_queue_lock_irq(card,flags)    if (lock) wan_spin_lock_irq(card,flags)
#define wptdm_queue_unlock_irq(card,flags)  if (lock) wan_spin_unlock_irq(card,flags)

	netskb_t *skb;
	int hdr_sz=sizeof(wp_api_hdr_t);
	sdla_t *card; 
	wan_smp_flag_t flags;
	int lock=1;

/* FIXME: Cause random crashes */
	return;

	card = (sdla_t*)tdm_api->card;
	if (!card) {
     	lock=0;
	}
	flags=0; 

	wptdm_queue_lock_irq(&card->wandev.lock,&flags);

	if (option == WP_API_FLUSH_ALL || option == WP_API_FLUSH_RX) {

		if (WPTDM_SPAN_OP_MODE(tdm_api) || tdm_api->hdlc_framing) {
			do {
				skb=wan_skb_dequeue(&tdm_api->wp_rx_list);		
				if (skb) {
					wan_skb_queue_tail(&tdm_api->wp_dealloc_list,skb);
					continue;
				}
			} while(skb);

			do {
				skb=wan_skb_dequeue(&tdm_api->wp_rx_free_list);
				if (skb) {
					wan_skb_queue_tail(&tdm_api->wp_dealloc_list,skb);
					continue;
				}
			} while(skb);

			
			skb=tdm_api->rx_skb;
			tdm_api->rx_skb=NULL;
			if (skb) {
				wan_skb_queue_tail(&tdm_api->wp_dealloc_list,skb);
			}

		} else {
			 
			while((skb=wan_skb_dequeue(&tdm_api->wp_rx_list))) {
				wan_skb_init(skb,hdr_sz);
				wan_skb_trim(skb,0);
				wan_skb_queue_tail(&tdm_api->wp_rx_free_list,skb);
			}
			skb=tdm_api->rx_skb;
			tdm_api->rx_skb=NULL;
			if (skb) {
				wan_skb_init(skb,hdr_sz);
				wan_skb_trim(skb,0);
				wan_skb_queue_tail(&tdm_api->wp_rx_free_list,skb);
			}
			
		}
	}

	if (option == WP_API_FLUSH_ALL || option == WP_API_FLUSH_TX) {

		do {
			skb=wan_skb_dequeue(&tdm_api->wp_tx_free_list);
			if (skb) {
				wan_skb_queue_tail(&tdm_api->wp_dealloc_list,skb);
				continue;
			}
		} while(skb);

		do {
			skb=wan_skb_dequeue(&tdm_api->wp_tx_list);
			if (skb) {
				wan_skb_queue_tail(&tdm_api->wp_dealloc_list,skb);
				continue;
			}
		} while(skb);

		skb=tdm_api->tx_skb;
		tdm_api->tx_skb=NULL;
		if (skb){
			wan_skb_queue_tail(&tdm_api->wp_dealloc_list,skb);
		}

	}

	if (option == WP_API_FLUSH_ALL || option == WP_API_FLUSH_EVENT) {
		while((skb=wan_skb_dequeue(&tdm_api->wp_event_list))) {
			wan_skb_init(skb,hdr_sz);
			wan_skb_trim(skb,0);
			wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
		}
	}

	wptdm_queue_unlock_irq(&card->wandev.lock,&flags);


	do {
		wptdm_queue_lock_irq(&card->wandev.lock,&flags);
		skb=wan_skb_dequeue(&tdm_api->wp_dealloc_list);
		wptdm_queue_unlock_irq(&card->wandev.lock,&flags);
		if (skb) {
			wan_skb_free(skb);
			continue;
		}
	} while(skb);
	

#undef wptdm_queue_lock_irq
#undef wptdm_queue_unlock_irq

}

static void wp_tdmapi_free_buffs(wanpipe_tdm_api_dev_t *tdm_api)
{
	wan_skb_queue_purge(&tdm_api->wp_rx_list);
	wan_skb_queue_purge(&tdm_api->wp_rx_free_list);
	wan_skb_queue_purge(&tdm_api->wp_tx_free_list);
	wan_skb_queue_purge(&tdm_api->wp_tx_list);
	wan_skb_queue_purge(&tdm_api->wp_event_list);
	wan_skb_queue_purge(&tdm_api->wp_event_free_list);
	wan_skb_queue_purge(&tdm_api->wp_dealloc_list);

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

static inline void wp_wakeup_rx_tdmapi(wanpipe_tdm_api_dev_t *tdm_api)
{
#ifdef DEBUG_API_WAKEUP
	if (tdm_api->tdm_span == 1 && tdm_api->tdm_chan == 1) {
		gwake_cnt++;
		if (gwake_cnt % 1000 == 0) {
			DEBUG_EVENT("%s: TDMAPI WAKING DEV \n",tdm_api->name);
		}
	}
#endif

	if (tdm_api->cdev) {
		wanpipe_cdev_rx_wake(tdm_api->cdev);
	}
}

static inline void wp_wakeup_event_tdmapi(wanpipe_tdm_api_dev_t *tdm_api)
{
	if (tdm_api->cdev) {
		wanpipe_cdev_event_wake(tdm_api->cdev);
	}
}

static inline void wp_wakeup_tx_tdmapi(wanpipe_tdm_api_dev_t *tdm_api)
{
#ifdef DEBUG_API_WAKEUP
	if (tdm_api->tdm_span == 1 && tdm_api->tdm_chan == 1) {
		gwake_cnt++;
		if (gwake_cnt % 1000 == 0) {
			DEBUG_EVENT("%s: TDMAPI WAKING DEV \n",tdm_api->name);
		}
	}
#endif

	if (tdm_api->cdev) {
		wanpipe_cdev_tx_wake(tdm_api->cdev);
	}
}

static int wp_tdmapi_reg_globals(void)
{
	int err=0;
	wanpipe_tdm_api_dev_t *tdm_api;
	wanpipe_cdev_t *cdev;

	rx_gains=NULL;
	tx_gains=NULL;

	/* Internal Sanity Check */
	if (WANPIPE_API_CMD_SZ != sizeof(wanpipe_api_cmd_t)) {
		DEBUG_EVENT("%s: Internal Error: Wanpipe API CMD Structure Exceeds Limit=%i Size=%i!\n",
				__FUNCTION__, WANPIPE_API_CMD_SZ,sizeof(wanpipe_api_cmd_t));
		return -EINVAL;
	}

	/* Internal Sanity Check */
	if (WAN_MAX_EVENT_SZ != sizeof(wp_api_event_t)) {
		DEBUG_EVENT("%s: Internal Error: Wanpipe Event Structure Exceeds Limit=%i Size=%i!\n",
				__FUNCTION__, WAN_MAX_EVENT_SZ,sizeof(wp_api_event_t));
		return EINVAL;
	}

	/* Internal Sanity Check */
	if (WAN_MAX_HDR_SZ != sizeof(wp_api_hdr_t)) {
		DEBUG_EVENT("%s: Internal Error: Wanpipe Header Structure Exceeds Limit=%i Size=%i!\n",
				__FUNCTION__, WAN_MAX_HDR_SZ,sizeof(wp_api_event_t));
		return EINVAL;
	}

#if !defined(__WINDOWS__)
	cdev=wan_kmalloc(sizeof(wanpipe_cdev_t));
	if (!cdev) {
		return -ENOMEM;
	}

	memset(cdev,0,sizeof(wanpipe_cdev_t));
#endif

	DEBUG_TDMAPI("%s: Registering Wanpipe TDM Device!\n",__FUNCTION__);

	wp_tdmapi_fops.open		= wp_tdmapi_open;
	wp_tdmapi_fops.close	= wp_tdmapi_release;
	wp_tdmapi_fops.ioctl	= wanpipe_tdm_api_ioctl;
	wp_tdmapi_fops.read		= wp_tdmapi_read_msg;
	wp_tdmapi_fops.write	= wp_tdmapi_write_msg;
	wp_tdmapi_fops.poll		= wp_tdmapi_poll;

#if !defined(__WINDOWS__)
	{
		tdm_api=&tdmapi_ctrl;
		memset(tdm_api,0,sizeof(wanpipe_tdm_api_dev_t));

		wan_skb_queue_init(&tdm_api->wp_rx_list);
		wan_skb_queue_init(&tdm_api->wp_rx_free_list);
		wan_skb_queue_init(&tdm_api->wp_tx_free_list);
		wan_skb_queue_init(&tdm_api->wp_tx_list);
		wan_skb_queue_init(&tdm_api->wp_event_list);
		wan_skb_queue_init(&tdm_api->wp_event_free_list);
		wan_skb_queue_init(&tdm_api->wp_dealloc_list);

		err=wp_tdmapi_alloc_q(tdm_api, &tdm_api->wp_event_free_list, WP_TDM_API_EVENT_MAX_LEN, WP_TDM_MAX_CTRL_EVENT_Q_LEN);
		if (err) {
			wp_tdmapi_free_buffs(tdm_api);
			wan_free(cdev);
			return err;
		}

		wan_set_bit(0,&tdm_api->init);

		wan_spin_lock_init(&tdm_api->lock, "wan_tdmapi_lock");

		cdev->dev_ptr=&tdmapi_ctrl;
		tdm_api->cdev=cdev;

		memcpy(&cdev->ops, &wp_tdmapi_fops, sizeof(wanpipe_cdev_ops_t));
	
		err=wanpipe_cdev_tdm_ctrl_create(cdev);

	}

	wanpipe_wandev_timer_create();
#endif

	return err;
}

static int wp_tdmapi_unreg_globals(void)
{
	unsigned long timeout=SYSTEM_TICKS;
	DEBUG_EVENT("%s: Unregistering Wanpipe TDM Device!\n",__FUNCTION__);

#if !defined(__WINDOWS__)

	wanpipe_wandev_timer_free();

	wan_clear_bit(0,&tdmapi_ctrl.init);

	while(wan_test_bit(0,&tdmapi_ctrl.used)) {
		if (SYSTEM_TICKS - timeout > HZ*2) {
			DEBUG_EVENT("wanpipe_tdm_api: Warning: Ctrl Device still in use on shutdown!\n");
			break;	
		}
		WP_DELAY(1000);
		WP_SCHEDULE("tdmapi_ctlr",10);
		wp_wakeup_rx_tdmapi(&tdmapi_ctrl);
	}

	wp_tdmapi_free_buffs(&tdmapi_ctrl);

	if (tdmapi_ctrl.cdev) {
		wanpipe_cdev_free(tdmapi_ctrl.cdev);
		wan_free(tdmapi_ctrl.cdev);
		tdmapi_ctrl.cdev=NULL;
	}
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
	sdla_t				*card = NULL;
	u8 					tmp_name[50];
	int 				err=-EINVAL;
	wanpipe_cdev_t 		*cdev=NULL;
	wanpipe_api_cmd_t 	wp_cmd;
	wanpipe_api_cmd_t 	*wp_cmd_ptr=NULL;
	sdla_fe_t			*fe =NULL;
	int 				tdm_api_queue_init=0;
	wanpipe_tdm_api_card_dev_t *tdm_card_dev=NULL;

	wan_spin_lock_init(&tdm_api->lock, "wan_tdmapi_lock");
	
	/* Header Sanity Check */
	if ((sizeof(wp_api_hdr_t)!=WAN_MAX_HDR_SZ) ||
		(sizeof(wp_api_hdr_t)!=WAN_MAX_HDR_SZ)) {
		DEBUG_EVENT("%s: Internal Error: RxHDR=%i != Expected RxHDR=%i  TxHDR=%i != Expected TxHDR=%i\n",
				__FUNCTION__,sizeof(wp_api_hdr_t),WAN_MAX_HDR_SZ,sizeof(wp_api_hdr_t),WAN_MAX_HDR_SZ);
		return -EINVAL;
	}

	cdev=wan_kmalloc(sizeof(wanpipe_cdev_t));
	if (!cdev) {
		return -ENOMEM;
	}
	memset(cdev,0,sizeof(wanpipe_cdev_t));

	if (wp_tdmapi_global_cnt == 0){
		err=wp_tdmapi_reg_globals();
		if (err) {
			goto tdm_api_reg_error_exit;
		}
	}
	wp_tdmapi_global_cnt++;

	WAN_ASSERT(tdm_api == NULL);
	card = (sdla_t*)tdm_api->card;
	fe = &card->fe;

	if (!card->tdm_api_dev) {
		
		tdm_card_dev = wan_malloc(sizeof(wanpipe_tdm_api_card_dev_t));
		if (!tdm_card_dev) {
			goto tdm_api_reg_error_exit;
		}
		memset(tdm_card_dev,0,sizeof(wanpipe_tdm_api_card_dev_t));
		card->tdm_api_dev=tdm_card_dev;
		tdm_card_dev->ref_cnt++;

	} else {
		tdm_card_dev = card->tdm_api_dev;
	}

	if (!tdm_api->hdlc_framing) {
		unsigned int i,max_timeslot=0;
		if (!IS_E1_CARD(card)) {
			tdm_card_dev->sig_timeslot_map |= (tdm_api->active_ch >> 1);
		} else {
			tdm_card_dev->sig_timeslot_map |= tdm_api->active_ch;
		}

		for (i=0;i<32;i++) {
			if (wan_test_bit(i,&tdm_card_dev->sig_timeslot_map)) {
				max_timeslot=i+1;
			}
		}
		if (max_timeslot > tdm_card_dev->max_timeslots) {
			tdm_card_dev->max_timeslots=max_timeslot;
		}
	}

	wan_skb_queue_init(&tdm_api->wp_rx_list);
	wan_skb_queue_init(&tdm_api->wp_rx_free_list);
	
	wan_skb_queue_init(&tdm_api->wp_tx_list);
	wan_skb_queue_init(&tdm_api->wp_tx_free_list);

	wan_skb_queue_init(&tdm_api->wp_event_list);
	wan_skb_queue_init(&tdm_api->wp_event_free_list);
	wan_skb_queue_init(&tdm_api->wp_dealloc_list);

	tdm_api_queue_init=1;

	err=wp_tdmapi_alloc_q(tdm_api, &tdm_api->wp_rx_free_list, WP_TDM_API_MAX_LEN, 16);
	if (err) {
		goto tdm_api_reg_error_exit;
	}

	err=wp_tdmapi_alloc_q(tdm_api, &tdm_api->wp_event_free_list, WP_TDM_API_EVENT_MAX_LEN, WP_TDM_MAX_EVENT_Q_LEN);
	if (err) {
		goto tdm_api_reg_error_exit;
	}

	tdm_api->cfg.rx_queue_sz	=WP_TDM_MAX_RX_Q_LEN;
	tdm_api->tx_channelized		=0;

	DEBUG_TDMAPI("%s(): usr_period: %d, hw_mtu_mru: %d\n", __FUNCTION__, 
		tdm_api->cfg.usr_period, tdm_api->cfg.hw_mtu_mru);

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
		if (tdm_api->cfg.tx_queue_sz == 0) {
			DEBUG_EVENT("%s: Internal Error: Tx Q Len not specified by lower layer!\n",
					__FUNCTION__);
			err=-EINVAL;
			goto tdm_api_reg_error_exit;
		}

	} else {

		tdm_api->cfg.tdm_codec		=WP_NONE;
		tdm_api->cfg.power_level	=0;
		tdm_api->cfg.rx_disable		=0;
		tdm_api->cfg.tx_disable		=0;
		tdm_api->cfg.ec_tap			=0;
		tdm_api->cfg.rbs_rx_bits	=-1;
		tdm_api->cfg.hdlc			=0;
		if (!tdm_api->cfg.hw_mtu_mru ||
			!tdm_api->cfg.usr_period ||
			!tdm_api->cfg.usr_mtu_mru) {
			DEBUG_EVENT("%s: Internal Error: API hwmtu=%i period=%i usr_mtu_mru=%i!\n",
						__FUNCTION__, tdm_api->cfg.hw_mtu_mru, tdm_api->cfg.usr_period, tdm_api->cfg.usr_mtu_mru);
					
			err=-EINVAL;
			goto tdm_api_reg_error_exit;
		}

		if (WPTDM_CHAN_OP_MODE(tdm_api)) {
#if 0
			tdm_api->cfg.hw_mtu_mru		= 8;
			tdm_api->cfg.usr_period		= 10;
			tdm_api->cfg.usr_mtu_mru 	= tdm_api->cfg.usr_period*tdm_api->cfg.hw_mtu_mru;
#endif
			tdm_api->cfg.rx_queue_sz 	= WP_TDM_MAX_RX_Q_LEN;
			tdm_api->cfg.tx_queue_sz	= WP_TDM_MAX_TX_Q_LEN;
			tdm_api->tx_channelized		=1;

		} else {

			/* We are expecting tx_q_len for hdlc
			* to be configured from upper layer */
			if (tdm_api->cfg.tx_queue_sz == 0) {
				DEBUG_EVENT("%s: Internal Error: Tx Q Len not specified by lower layer!\n",
						__FUNCTION__);
				err=-EINVAL;
				goto tdm_api_reg_error_exit;
			}
		}

		if (tdm_api->cfg.idle_flag == 0) {
        		tdm_api->cfg.idle_flag=0xFF;
		}
	}


	tdm_api->critical=0;
	wan_clear_bit(0,&tdm_api->used);


	err=store_tdm_api_pointer_in_card(card, tdm_api);
	if (err){
		goto tdm_api_reg_error_exit;
	}
	
	sprintf(tmp_name,"wanpipe%d_if%d",tdm_api->tdm_span,tdm_api->tdm_chan);

	DEBUG_TDMAPI("%s: Configuring TDM API NAME=%s Qlen=%i\n",
			card->devname,tmp_name, tdm_api->cfg.tx_queue_sz);

	/* Initialize Event Callback functions */
	card->wandev.event_callback.rbsbits		= NULL; /*wp_tdmapi_rbsbits;*/
	card->wandev.event_callback.alarms		= NULL; /*wp_tdmapi_alarms;*/
	card->wandev.event_callback.hook		= wp_tdmapi_hook;
	card->wandev.event_callback.ringdetect	= wp_tdmapi_ringdetect;
	card->wandev.event_callback.ringtrip	= wp_tdmapi_ringtrip;
	card->wandev.event_callback.linkstatus	= wp_tdmapi_linkstatus;
	card->wandev.event_callback.polarityreverse	= wp_tdmapi_polarityreverse;

	card->wandev.te_report_rbsbits  = wp_tdmapi_report_rbsbits;
	card->wandev.te_report_alarms   = wp_tdmapi_report_alarms;


	/* Always initialize the callback pointer */
	card->wandev.event_callback.tone 		= wp_tdmapi_tone;

#if defined(__WINDOWS__)
	/* Analog always supports DTMF detection */
	tdm_api->dtmfsupport = WANOPT_YES;
#endif

	cdev->dev_ptr=tdm_api;
	tdm_api->cdev=cdev;

	cdev->span=tdm_api->tdm_span;
	cdev->chan=tdm_api->tdm_chan;
	cdev->operation_mode=tdm_api->api_mode;/* WP_TDM_OPMODE - Legacy...*/
	sprintf(cdev->name,tmp_name,sizeof(cdev->name));
	memcpy(&cdev->ops, &wp_tdmapi_fops, sizeof(wanpipe_cdev_ops_t));

	err=wanpipe_cdev_tdm_create(cdev);
	if (err) {
		goto tdm_api_reg_error_exit;
	}

	/* AT this point the registration should not fail */

	if (tdm_api->cfg.rbs_tx_bits) {
		DEBUG_EVENT("%s: Setting Tx RBS/CAS Idle Bits = 0x%02X\n",
			       	        tmp_name,
					tdm_api->cfg.rbs_tx_bits);
		tdm_api->write_rbs_bits(tdm_api->chan,
		                        tdm_api->tdm_chan,
					(u8)tdm_api->cfg.rbs_tx_bits);
	}

   /* Enable default events for analog */
	if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
		wp_cmd.cmd = WP_API_CMD_SET_EVENT;
		wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
		wp_cmd_ptr=&wp_cmd;

		if (fe->rm_param.mod[(tdm_api->tdm_chan) - 1].type == MOD_TYPE_FXS) {
			/*Intialize timers for FXS Wink-Flash event*/
			fe->rm_param.mod[(tdm_api->tdm_chan) - 1].u.fxs.itimer=0;
			fe->rm_param.mod[(tdm_api->tdm_chan) - 1].u.fxs.rxflashtime=WP_RM_RXFLASHTIME;
			/*hook event*/
			wp_cmd.event.wp_api_event_type = WP_API_EVENT_RXHOOK;
			DEBUG_EVENT("%s: %s Loop Closure Event on Module %d\n",
				fe->name,
				WP_API_EVENT_MODE_DECODE(wp_cmd.event.wp_api_event_mode),
				tdm_api->tdm_chan);
			wanpipe_tdm_api_event_ioctl(tdm_api,wp_cmd_ptr);
			/*Ring/Trip event */
			wp_cmd.event.wp_api_event_type = WP_API_EVENT_RING_TRIP_DETECT;
			DEBUG_EVENT("%s: %s Ring Trip Detection Event on Module %d\n",
				fe->name,
				WP_API_EVENT_MODE_DECODE(wp_cmd.event.wp_api_event_mode),
				tdm_api->tdm_chan);
			wanpipe_tdm_api_event_ioctl(tdm_api,wp_cmd_ptr);
		} else if (fe->rm_param.mod[(tdm_api->tdm_chan) - 1].type == MOD_TYPE_FXO ) {
			/*ring detect event */
			wp_cmd.event.wp_api_event_type = WP_TDMAPI_EVENT_RING_DETECT;
			DEBUG_EVENT("%s: %s Ring Detection Event on module %d\n",
				fe->name,
				WP_API_EVENT_MODE_DECODE(wp_cmd.event.wp_api_event_mode),
				tdm_api->tdm_chan);
			wanpipe_tdm_api_event_ioctl(tdm_api,wp_cmd_ptr);
		}
	}


	return err;


tdm_api_reg_error_exit:

	if (wp_tdmapi_global_cnt > 0) {
		wp_tdmapi_global_cnt--;
	}

	if (wp_tdmapi_global_cnt == 0){
		wp_tdmapi_unreg_globals();
	}

	if (card->tdm_api_dev) {
		wanpipe_tdm_api_card_dev_t *tdm_card_dev = card->tdm_api_dev;

		if (tdm_card_dev->ref_cnt > 0){
			tdm_card_dev->ref_cnt--;
		}
		if (tdm_card_dev->ref_cnt == 0) {
			wan_free(card->tdm_api_dev);
			card->tdm_api_dev=NULL;
		}
	}

	if (tdm_api_queue_init) {
		wp_tdmapi_free_buffs(tdm_api);
	}

	if (cdev) {
		wan_free(cdev);
	}

	return err;
}

int wanpipe_tdm_api_unreg(wanpipe_tdm_api_dev_t *tdm_api)
{
	sdla_t *card = tdm_api->card;

	wan_set_bit(WP_TDM_DOWN,&tdm_api->critical);

	if (wan_test_bit(0,&tdm_api->used)) {
		DEBUG_EVENT("%s Failed to unreg Span=%i Chan=%i: BUSY!\n",
			tdm_api->name,tdm_api->tdm_span,tdm_api->tdm_chan);
		return -EBUSY;
	}

	remove_tdm_api_pointer_from_card(tdm_api);

	wp_tdmapi_free_buffs(tdm_api);

	if (tdm_api->cdev) {
		wanpipe_cdev_free(tdm_api->cdev);
		wan_free(tdm_api->cdev);
		tdm_api->cdev=NULL;
	}

	wp_tdmapi_global_cnt--;
	if (wp_tdmapi_global_cnt == 0) {
		wp_tdmapi_unreg_globals();
	}

	if (card->tdm_api_dev) {
		wanpipe_tdm_api_card_dev_t *tdm_card_dev = card->tdm_api_dev;

		if (tdm_card_dev->ref_cnt > 0){
			tdm_card_dev->ref_cnt--;
		}
		if (tdm_card_dev->ref_cnt == 0) {

			if (tdm_card_dev->rbs_poll_timeout) {
				wan_smp_flag_t smp_flags1;
				card->hw_iface.hw_lock(card->hw,&smp_flags1);
				if (card->wandev.fe_iface.set_fe_sigctrl) {
					card->wandev.fe_iface.set_fe_sigctrl(
							&card->fe,
							WAN_TE_SIG_POLL,
							ENABLE_ALL_CHANNELS,
							WAN_DISABLE);
				}
				card->hw_iface.hw_unlock(card->hw,&smp_flags1);
			}

			wan_free(card->tdm_api_dev);
			card->tdm_api_dev=NULL;
		}
	}

	return 0;
}

int wanpipe_tdm_api_kick(wanpipe_tdm_api_dev_t *tdm_api)
{
	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init)){
		return -ENODEV;
	}

	if (tdm_api->tx_channelized) {
		if (is_tdm_api_stopped(tdm_api) || wan_skb_queue_len(&tdm_api->wp_tx_list) >= (int)tdm_api->cfg.tx_queue_sz){
			wp_tdm_api_start(tdm_api);
			if (wan_test_bit(0,&tdm_api->used)) {
				wp_wakeup_tx_tdmapi(tdm_api);
			}
		}
	} else {
			wp_tdm_api_start(tdm_api);
			if (wan_test_bit(0,&tdm_api->used)) {
				wp_wakeup_tx_tdmapi(tdm_api);
			}
	}

	return 0;
}


int wanpipe_tdm_api_is_rbsbits(sdla_t *card)
{
	wanpipe_tdm_api_card_dev_t *tdm_card_dev;

	WAN_ASSERT(card == NULL);

	if (!card->tdm_api_dev) {
		return 0;
	}

	tdm_card_dev = card->tdm_api_dev;

	if (!tdm_card_dev || !tdm_card_dev->rbs_poll_timeout) {
		return 0;
	}

	tdm_card_dev->rbscount++;

	if ((SYSTEM_TICKS - tdm_card_dev->rbs_poll_cnt) < tdm_card_dev->rbs_poll_timeout) {
		return 0;
	}

	tdm_card_dev->rbs_poll_cnt=SYSTEM_TICKS;

	return 1;
}



/* Note: This functin is hw_locked from the core */
int wanpipe_tdm_api_rbsbits_poll(sdla_t * card)
{
	wanpipe_tdm_api_card_dev_t *tdm_card_dev;
	unsigned int x;

	WAN_ASSERT(card == NULL);

	if (!card->tdm_api_dev) {
		return 0;
	}

	tdm_card_dev = card->tdm_api_dev;

	if ((SYSTEM_TICKS - tdm_card_dev->rbs_long_poll_cnt) > HZ*2) {

		tdm_card_dev->rbs_long_poll_cnt=SYSTEM_TICKS;

		DEBUG_TEST("%s: RBS POLL MaxSlots=%i MAP=0x%08X\n",card->devname,tdm_card_dev->max_timeslots,tdm_card_dev->sig_timeslot_map);

		for(x = 0; x < tdm_card_dev->max_timeslots; x++){
			if (wan_test_bit(x, &tdm_card_dev->sig_timeslot_map)){
				if (card->wandev.fe_iface.read_rbsbits) {
					card->wandev.fe_iface.read_rbsbits(
						&card->fe,
						x,
						WAN_TE_RBS_UPDATE|WAN_TE_RBS_REPORT);
				} else {
					DEBUG_EVENT("%s: Internal Error [%s:%d]!\n", card->devname,	__FUNCTION__,__LINE__);
				}
			}
		}

	} else {

		if (card->wandev.fe_iface.check_rbsbits == NULL){
                	DEBUG_EVENT("%s: Internal Error [%s:%d]!\n",
					card->devname,
					__FUNCTION__,__LINE__); 
			return -EINVAL;
		}

		card->wandev.fe_iface.check_rbsbits(
				&card->fe, 
				1, tdm_card_dev->sig_timeslot_map, 1);
		card->wandev.fe_iface.check_rbsbits(
				&card->fe, 
				9, tdm_card_dev->sig_timeslot_map, 1);
		card->wandev.fe_iface.check_rbsbits(
				&card->fe, 
				17, tdm_card_dev->sig_timeslot_map, 1);
		if (IS_E1_CARD(card)){
			card->wandev.fe_iface.check_rbsbits(
					&card->fe, 
					25, tdm_card_dev->sig_timeslot_map, 1);
		}
	}

	return 0;
}


static int wp_tdmapi_open(void *obj)
{
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t*)obj;
	int cnt;

	if (!tdm_api) {
		return -ENODEV;
	}

	if (wan_test_bit(WP_TDM_DOWN,&tdm_api->critical)) {
		return -ENODEV;
	}

	if (tdm_api != &tdmapi_ctrl && !tdm_api->card) {
     	DEBUG_EVENT("%s: Error: Wanpipe API Device does not have a card pointer! Internal error!\n",tdm_api->name);
		return -ENODEV;
	}

	cnt = wp_tdm_inc_open_cnt(tdm_api);
	if (cnt == 1) {
		wp_tdmapi_init_buffs(tdm_api, WP_API_FLUSH_ALL);
	
		tdm_api->rx_gain=NULL;
		tdm_api->tx_gain=NULL;
	}

	wan_set_bit(0,&tdm_api->used);
	wp_tdm_api_start(tdm_api);

	DEBUG_TDMAPI ("%s: DRIVER OPEN S/C(%i/%i) API Ptr=%p\n",
		__FUNCTION__, tdm_api->tdm_span, tdm_api->tdm_chan, tdm_api);

	return 0;
}

#ifdef DEBUG_API_READ
static int gread_cnt=0;
#endif

static int wp_tdmapi_read_msg(void *obj , netskb_t **skb_ptr, wp_api_hdr_t *hdr, int count)
{
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t*)obj;
	netskb_t *skb;
	wan_smp_flag_t irq_flags;
	sdla_t *card;
	int rx_q_len=0;
	u8 *buf;

	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init) || !wan_test_bit(0,&tdm_api->used) || !hdr) {
		return -ENODEV;
	}

	if (tdm_api == &tdmapi_ctrl) {
		hdr->wp_api_hdr_operation_status = SANG_STATUS_INVALID_DEVICE;
		return -EINVAL;
	}

	if (wan_test_bit(WP_TDM_DOWN,&tdm_api->critical)) {
		hdr->wp_api_hdr_operation_status = SANG_STATUS_GENERAL_ERROR;
		return -ENODEV;
	}

	card=(sdla_t*)tdm_api->card;

	if (!card) {
		hdr->wp_api_hdr_operation_status = SANG_STATUS_GENERAL_ERROR;
		return -ENODEV;
	}

	irq_flags=0;

	wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
	skb=wan_skb_dequeue(&tdm_api->wp_rx_list);
	rx_q_len = wan_skb_queue_len(&tdm_api->wp_rx_list);
	wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
	if (!skb){
		hdr->wp_api_hdr_operation_status = SANG_STATUS_NO_DATA_AVAILABLE;
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
				wan_skb_len(skb)-sizeof(wp_api_hdr_t));
	    }
	}
#endif

	if (count < (int)(wan_skb_len(skb) - sizeof(wp_api_hdr_t)) ||

	    wan_skb_len(skb) < sizeof(wp_api_hdr_t)){

		DEBUG_EVENT("%s:%d TDMAPI READ: Error: Count=%i < Skb=%i < HDR=%i Critical Error\n",
			__FUNCTION__,__LINE__,count,wan_skb_len(skb),sizeof(wp_api_hdr_t));
		wan_skb_free(skb);
		hdr->wp_api_hdr_operation_status = SANG_STATUS_BUFFER_TOO_SMALL;
		return -EFAULT;
	}

	
	buf=(u8*)wan_skb_data(skb);

	if (WPTDM_SPAN_OP_MODE(tdm_api) || tdm_api->hdlc_framing) {

		memcpy(hdr,buf,sizeof(wp_api_hdr_t));
		
	} else {
		netskb_t *tmp_skb;

		/* Allocate a rx another free skb frame */
		tmp_skb=wan_skb_alloc(WP_TDM_API_MAX_LEN);
		if (tmp_skb) {
			wan_skb_init(tmp_skb,sizeof(wp_api_hdr_t));
			wan_skb_trim(tmp_skb,0);
			wan_skb_queue_tail(&tdm_api->wp_rx_free_list,tmp_skb);
		}
	}

	hdr->wp_api_rx_hdr_number_of_frames_in_queue = (u8)rx_q_len;
	hdr->wp_api_rx_hdr_max_queue_length = (u8)tdm_api->cfg.rx_queue_sz;

	hdr->wp_api_hdr_operation_status = SANG_STATUS_RX_DATA_AVAILABLE;
	hdr->wp_api_hdr_data_length = wan_skb_len(skb)-sizeof(wp_api_hdr_t);

	if (wan_skb_len(skb) >= sizeof(wp_api_hdr_t)) {
		memcpy(buf,hdr,sizeof(wp_api_hdr_t));
	} else {
		DEBUG_EVENT("%s: Internal Error: Rx Data Invalid %i\n",tdm_api->name,wan_skb_len(skb));
	}

	*skb_ptr = skb;

//NENAD
	DEBUG_TEST("%s: RX Q=%i  RX Q MAX=%i SIZE=%i\n",
				tdm_api->name,
				hdr->wp_api_rx_hdr_number_of_frames_in_queue,
				hdr->wp_api_rx_hdr_max_queue_length,
				wan_skb_len(skb));

	return 0;

#undef wptdm_queue_lock_irq
#undef wptdm_queue_unlock_irq
}


static int wp_tdmapi_tx(wanpipe_tdm_api_dev_t *tdm_api, netskb_t *skb,  wp_api_hdr_t *hdr)
{
	int err=-EINVAL;

	if (wan_test_and_set_bit(WP_TDM_HDLC_TX,&tdm_api->critical)){
		hdr->wp_api_hdr_operation_status = SANG_STATUS_GENERAL_ERROR;
		return -EINVAL;
	}

	if (tdm_api->write_hdlc_frame) {
		err=tdm_api->write_hdlc_frame(tdm_api->chan,skb, hdr);
	} else {
		return -EINVAL;
	}

	if (err == -EBUSY) {
		/* Busy condition */
		hdr->wp_api_hdr_operation_status = SANG_STATUS_DEVICE_BUSY;
		wp_tdm_api_stop(tdm_api);
		err=1;
	} else if (err == 1) {
		/* Successful tx but now queue is full */
		wp_tdm_api_stop(tdm_api);
		err=0;
	} else if (err == 0) {
		/* Successful tx queue is still NOT full */
		wp_tdm_api_start(tdm_api);
		err=0;
	} else {
		wp_tdm_api_start(tdm_api);
		hdr->wp_api_hdr_operation_status = SANG_STATUS_IO_ERROR;
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,tx_errors);
	}

#if 0
	while ((skb=wan_skb_dequeue(&tdm_api->wp_tx_list)) != NULL) {
		err=tdm_api->write_hdlc_frame(tdm_api->chan,skb);
		if (err) {
			wan_skb_queue_head(&tdm_api->wp_tx_list, skb);
			break;
		}
		tdm_api->cfg.stats.tx_packets++;
		skb=NULL;
	}
#endif

	wan_clear_bit(WP_TDM_HDLC_TX,&tdm_api->critical);
	return err;
}

#ifdef DEBUG_API_WRITE
static int gwrite_cnt=0;
#endif
static int wp_tdmapi_write_msg(void *obj, netskb_t *skb, wp_api_hdr_t *hdr)
{
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t*)obj;
	wan_smp_flag_t irq_flags;
	int err=-EINVAL;
	sdla_t *card;

	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init) || !wan_test_bit(0,&tdm_api->used)){
		return -ENODEV;
	}
	if (tdm_api == &tdmapi_ctrl) {
		hdr->wp_api_hdr_operation_status = SANG_STATUS_INVALID_DEVICE;
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,tx_errors);
		DEBUG_EVENT("Error: Wanpipe API: wp_tdmapi_write_msg() on ctrl device!\n");
		return -EINVAL;
	}

	card=tdm_api->card;
	irq_flags=0;

	if (wan_test_bit(WP_TDM_DOWN,&tdm_api->critical)) {
		hdr->wp_api_hdr_operation_status = SANG_STATUS_GENERAL_ERROR;
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,tx_errors);
		DEBUG_EVENT("Error: Wanpipe API: wp_tdmapi_write_msg() WP_TDM_DOWN");
		return -ENODEV;
	}

	if (wan_skb_len(skb) <= sizeof(wp_api_hdr_t)) {
		hdr->wp_api_hdr_operation_status = SANG_STATUS_BUFFER_TOO_SMALL;
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,tx_errors);
		DEBUG_EVENT("Error: Wanpipe API: wp_tdmapi_write_msg() datalen=%i < hdrlen=%i\n",
				wan_skb_len(skb),sizeof(wp_api_hdr_t));
		return -EINVAL;
	}

	if (WPTDM_SPAN_OP_MODE(tdm_api) || tdm_api->hdlc_framing) {
		if (wan_skb_len(skb) > (int)(tdm_api->cfg.usr_mtu_mru + sizeof(wp_api_hdr_t))) {
			DEBUG_EVENT("%s: Error: TDM API Tx packet too big %d Max=%d\n",
				tdm_api->name,wan_skb_len(skb), (tdm_api->cfg.usr_mtu_mru + sizeof(wp_api_hdr_t)));
			hdr->wp_api_hdr_operation_status =SANG_STATUS_TX_DATA_TOO_LONG;
			WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,tx_errors);
			return -EFBIG;
		}
	} else {
		if (wan_skb_len(skb) > (WP_TDM_API_MAX_LEN+sizeof(wp_api_hdr_t))) {
			DEBUG_EVENT("%s: Error: TDM API Tx packet too big %d\n",
				tdm_api->name, wan_skb_len(skb));
			hdr->wp_api_hdr_operation_status =SANG_STATUS_TX_DATA_TOO_LONG;
			WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,tx_errors);
			return -EFBIG;
		}
	}

	hdr->wp_api_hdr_data_length = wan_skb_len(skb)-sizeof(wp_api_hdr_t);

	if (WPTDM_SPAN_OP_MODE(tdm_api) || tdm_api->hdlc_framing) {
		
		wan_skb_pull(skb,sizeof(wp_api_hdr_t));

		err=wp_tdmapi_tx(tdm_api, skb, hdr);
		if (err) {
			wan_skb_push(skb,sizeof(wp_api_hdr_t));
			goto wp_tdmapi_write_msg_exit;
		} else {
			tdm_api->cfg.stats.tx_packets++;
			hdr->wp_api_hdr_operation_status=SANG_STATUS_SUCCESS;
		}

	} else {

		if (wan_skb_queue_len(&tdm_api->wp_tx_list) >= (int)tdm_api->cfg.tx_queue_sz){
			wp_tdm_api_stop(tdm_api);
			err=1;
			DEBUG_TDMAPI("Error: Wanpipe API: wp_tdmapi_write_msg() TxList=%i >= MaxTxList=%i\n",
				wan_skb_queue_len(&tdm_api->wp_tx_list), tdm_api->cfg.tx_queue_sz);
			goto wp_tdmapi_write_msg_exit;
		}

		/* The code below implements the channelized tx mode */
	
#ifdef DEBUG_API_WRITE
		if (tdm_api->tdm_span == 1 &&
			tdm_api->tdm_chan == 1) {
			gwrite_cnt++;
			if (gwrite_cnt &&
				gwrite_cnt % 1000 == 0) {
				DEBUG_EVENT("%s: WP_TDM API WRITE CODEC %i Len=%i\n",
						tdm_api->name, gwrite_cnt,
						wan_skb_len(skb)-sizeof(wp_api_hdr_t));
			}
		}
#endif
	
		card = tdm_api->card;

		wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
		wan_skb_queue_tail(&tdm_api->wp_tx_list, skb);
		hdr->tx_h.current_number_of_frames_in_tx_queue = (u8)wan_skb_queue_len(&tdm_api->wp_tx_list);
		wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);

		hdr->tx_h.max_tx_queue_length = (u8)tdm_api->cfg.tx_queue_sz;
		
		
		/* Free up previously used tx buffers */
		skb=NULL;
		if (wan_skb_queue_len(&tdm_api->wp_tx_free_list)) {
			do {
				wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
				skb=wan_skb_dequeue(&tdm_api->wp_tx_free_list);
				wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
				if (skb) {
					wan_skb_free(skb);
					continue;
				}

			}while(skb);
		}

		hdr->wp_api_hdr_operation_status=SANG_STATUS_SUCCESS;
		err=0;
	}

	DEBUG_TEST("%s: TX Q=%i  TX Q MAX=%i\n",
				tdm_api->name,
				hdr->tx_h.current_number_of_frames_in_tx_queue,hdr->tx_h.max_tx_queue_length);

wp_tdmapi_write_msg_exit:

	return err;
}



static int wp_tdmapi_release(void *obj)
{
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t*)obj;
	wan_smp_flag_t flag;
	u32 cnt;

	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init)) {
		if (tdm_api) {
			wan_clear_bit(0,&tdm_api->used);
		}
		return -ENODEV;
	}

	cnt = wp_tdm_dec_open_cnt(tdm_api);
	if (cnt == 0) {
		wan_clear_bit(0,&tdm_api->used);
		wp_wakeup_rx_tdmapi(tdm_api);

		wan_spin_lock(&tdm_api->lock,&flag);
	
		tdm_api->cfg.rbs_rx_bits=-1;
		tdm_api->cfg.rbs_tx_bits=-1;
	
		wan_spin_unlock(&tdm_api->lock,&flag);
	}

	return 0;
}

static int wan_skb_push_to_ctrl_event (netskb_t *skb)
{
	wanpipe_tdm_api_dev_t *tdm_api = &tdmapi_ctrl;
	netskb_t *ctrl_skb;

	if (!skb || !tdm_api) {
		DEBUG_EVENT("TDMAPI: Error: Null skb argument!\n");
		return -ENODEV;
	}

	if (!wan_test_bit(0,&tdm_api->used)) {
		return -EBUSY;
	}

	ctrl_skb=wan_skb_dequeue(&tdm_api->wp_event_free_list);
	if (ctrl_skb){
		int len = wan_skb_len(skb);
		u8 *buf;
		if (len > wan_skb_tailroom(ctrl_skb)) {
			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("TDMAPI: Error: TDM API CTRL Event Buffer Overflow Elen=%i Max=%i!\n",
							len,wan_skb_tailroom(ctrl_skb));
			}
			WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
			return -EFAULT;
		}
		buf=wan_skb_put(ctrl_skb,len);
		memcpy(buf,wan_skb_data(skb),len);

		wanpipe_tdm_api_handle_event(tdm_api,ctrl_skb);
	} else {
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
	}

	return 0;
}

#ifdef DEBUG_API_POLL
#if defined(__WINDOWS__)
#pragma message ("POLL Debugging Enabled")
#else
#warning "POLL Debugging Enabled"
#endif
static int gpoll_cnt=0;
#endif

static unsigned int wp_tdmapi_poll(void *obj)
{
	int ret=0;
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t*)obj;
	sdla_t *card;
	wan_smp_flag_t irq_flags;	

	irq_flags=0;

	if (tdm_api == NULL || !wan_test_bit(0,&tdm_api->init) || !wan_test_bit(0,&tdm_api->used)){
		return -ENODEV;
	}

	if (wan_test_bit(WP_TDM_DOWN,&tdm_api->critical)) {
		return -ENODEV;
	}


	if (tdm_api == &tdmapi_ctrl) {
    	if (wan_skb_queue_len(&tdm_api->wp_event_list)) {
			/* Indicate an exception */
			ret |= POLLPRI;
		}         
     	return ret;	
	}


	card = tdm_api->card;
	if (!card) {
		return -ENODEV;
	}

#ifdef DEBUG_API_POLL
	if (tdm_api->tdm_span == 1 && tdm_api->tdm_chan == 1) {
		gpoll_cnt++;
		if (gpoll_cnt && gpoll_cnt % 1000 == 0) {
			DEBUG_EVENT("%s: WP_TDM POLL WAKEUP %i RxF=%i TxF=%i TxCE=%i TxE=%i TxP=%i\n",
				tdm_api->name, gpoll_cnt,
				wan_skb_queue_len(&tdm_api->wp_tx_free_list),
				wan_skb_queue_len(&tdm_api->wp_rx_free_list),
				tdm_api->cfg.stats.tx_idle_packets,
				tdm_api->cfg.stats.tx_errors,
				tdm_api->cfg.stats.tx_packets);
		}
	}
#endif

	wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);

	/* Tx Poll */
	if (!wan_test_bit(0,&tdm_api->cfg.tx_disable)){
		if (tdm_api->tx_channelized) {
			if (wan_skb_queue_len(&tdm_api->wp_tx_list) < (int)tdm_api->cfg.tx_queue_sz) {
				wp_tdm_api_start(tdm_api);
				ret |= POLLOUT | POLLWRNORM;
			}
		} else {
			if (!is_tdm_api_stopped(tdm_api)){
				wp_tdm_api_start(tdm_api);
				ret |= POLLOUT | POLLWRNORM;
			}
		}
	}

	/* Rx Poll */
	if (!wan_test_bit(0,&tdm_api->cfg.rx_disable) &&
	    wan_skb_queue_len(&tdm_api->wp_rx_list)) {
		ret |= POLLIN | POLLRDNORM;
	}

	if (wan_skb_queue_len(&tdm_api->wp_event_list)) {
		/* Indicate an exception */
		ret |= POLLPRI;
	}

	wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);

	return ret;
}

static int wanpipe_tdm_api_handle_event(wanpipe_tdm_api_dev_t *tdm_api, netskb_t *skb)
{
	wan_skb_queue_tail(&tdm_api->wp_event_list,skb);
	tdm_api->cfg.stats.rx_events++;
	wp_wakeup_event_tdmapi(tdm_api);
	return 0;
}

static int wanpipe_tdm_api_ioctl_handle_tdm_api_cmd(wanpipe_tdm_api_dev_t *tdm_api, void *udata)
{
	wanpipe_api_cmd_t usr_tdm_api, *utdmapi;
	int err=0, cmd;
	wanpipe_codec_ops_t *wp_codec_ops;
	netskb_t *skb;
	sdla_fe_t			*fe = NULL;
	sdla_t *card=NULL;
	wanpipe_tdm_api_card_dev_t *tdm_card_dev=NULL;
	wan_smp_flag_t flags,irq_flags;	

	flags=0;
	irq_flags=0;

	/* This is a klooge because global ctrl device does not have card pointer */
    card = (sdla_t*)tdm_api->card;  
	if (card) {
		tdm_card_dev=card->tdm_api_dev;
		fe = &card->fe;
	}

	utdmapi = (wanpipe_api_cmd_t*)udata;

#if defined(__WINDOWS__)
	/* udata is a pointer to wanpipe_api_cmd_t */
	memcpy(&usr_tdm_api, udata, sizeof(wanpipe_api_cmd_t));
#else
	if (WAN_COPY_FROM_USER(&usr_tdm_api,
			       utdmapi,
			       sizeof(wanpipe_api_cmd_t))){
	       return -EFAULT;
	}
#endif
	cmd=usr_tdm_api.cmd;

	usr_tdm_api.result=SANG_STATUS_SUCCESS;

	DEBUG_TDMAPI("%s: TDM API CMD: %i CMD Size=%i HDR Size=%i Even Size=%i\n",
			tdm_api->name,cmd,sizeof(wanpipe_api_cmd_t),sizeof(wp_api_hdr_t), sizeof(wp_api_event_t));

	wan_spin_lock(&tdm_api->lock,&flags);

	if (tdm_api->hdlc_framing) {
		switch (cmd) {
		case WP_API_CMD_OPEN_CNT:
		case WP_API_CMD_GET_USR_MTU_MRU:
		case WP_API_CMD_GET_STATS:
		case WP_API_CMD_GET_FULL_CFG:
		case WP_API_CMD_GET_FE_STATUS:
		case WP_API_CMD_SET_FE_STATUS:
       	case WP_API_CMD_READ_EVENT:
       	case WP_API_CMD_GET_FE_ALARMS:
		case WP_API_CMD_RESET_STATS:
		case WP_API_CMD_SET_EVENT:
		case WP_API_CMD_SET_RM_RXFLASHTIME:
		case WP_API_CMD_DRIVER_VERSION:
		case WP_API_CMD_FIRMWARE_VERSION:
		case WP_API_CMD_CPLD_VERSION:
			break;
		default:
			DEBUG_EVENT("%s: Invalid TDM API HDLC CMD %i\n", tdm_api->name,cmd);
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
			err=-EOPNOTSUPP;
			goto tdm_api_exit;
		}
	}

	if (tdm_api == &tdmapi_ctrl) {
		switch (cmd) {
		case WP_API_CMD_READ_EVENT:
			break;
		default:
			DEBUG_EVENT("%s: Invalid TDM API for CTRL DEVICE %i\n", tdm_api->name,cmd);
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
			err=-EOPNOTSUPP;
			goto tdm_api_exit;
		}
	}

	switch (cmd) {

	case WP_API_CMD_OPEN_CNT:
		usr_tdm_api.open_cnt = __wp_tdm_get_open_cnt(tdm_api);
		break;

	case WP_API_CMD_SET_HW_MTU_MRU:
		usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
		err=-EOPNOTSUPP;
		break;

	case WP_API_CMD_GET_HW_MTU_MRU:
		usr_tdm_api.hw_mtu_mru = tdm_api->cfg.hw_mtu_mru;
		break;

	case WP_API_CMD_SET_USR_PERIOD:

		if (WPTDM_SPAN_OP_MODE(tdm_api)) {
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
			err=-EOPNOTSUPP;
			goto tdm_api_exit;
		}

		if (usr_tdm_api.usr_period >= 10 &&
		    (usr_tdm_api.usr_period % 10) == 0 &&
		    usr_tdm_api.usr_period <= 1000) {

			usr_tdm_api.usr_mtu_mru = usr_tdm_api.usr_period*tdm_api->cfg.hw_mtu_mru;

		} else {
			usr_tdm_api.result=SANG_STATUS_INVALID_PARAMETER;
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

	case WP_API_CMD_GET_USR_PERIOD:

		usr_tdm_api.usr_period = tdm_api->cfg.usr_period;
		break;

	case WP_API_CMD_GET_USR_MTU_MRU:

		usr_tdm_api.usr_mtu_mru = tdm_api->cfg.usr_mtu_mru;
		break;

	case WP_API_CMD_SET_CODEC:

		if (WPTDM_SPAN_OP_MODE(tdm_api)) {
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
			err=-EOPNOTSUPP;
			goto tdm_api_exit;
		}

		if (usr_tdm_api.tdm_codec == tdm_api->cfg.tdm_codec){
			err = 0;
			goto tdm_api_exit;
		}

		if (usr_tdm_api.tdm_codec >= WP_TDM_CODEC_MAX){
			usr_tdm_api.result=SANG_STATUS_INVALID_PARAMETER;
			err = -EINVAL;
			goto tdm_api_exit;
		}

		if (usr_tdm_api.tdm_codec == WP_NONE) {

			usr_tdm_api.usr_mtu_mru = tdm_api->cfg.hw_mtu_mru * tdm_api->cfg.usr_period;

		} else {
			wp_codec_ops = WANPIPE_CODEC_OPS[tdm_api->cfg.hw_tdm_coding][usr_tdm_api.tdm_codec];
			if (!wp_codec_ops || !wp_codec_ops->init){

				usr_tdm_api.result=SANG_STATUS_INVALID_PARAMETER;
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

	case WP_API_CMD_GET_CODEC:
		usr_tdm_api.tdm_codec = tdm_api->cfg.tdm_codec;
		break;

	case WP_API_CMD_SET_POWER_LEVEL:
		tdm_api->cfg.power_level = usr_tdm_api.power_level;
		break;

	case WP_API_CMD_GET_POWER_LEVEL:
		usr_tdm_api.power_level = tdm_api->cfg.power_level;
		break;

	case WP_API_CMD_TOGGLE_RX:
		if (tdm_api->cfg.tx_disable){
			wan_clear_bit(0,&tdm_api->cfg.rx_disable);
		}else{
			wan_set_bit(0,&tdm_api->cfg.rx_disable);
		}
		break;

	case WP_API_CMD_TOGGLE_TX:
		if (tdm_api->cfg.tx_disable){
			wan_clear_bit(0,&tdm_api->cfg.tx_disable);
		}else{
			wan_set_bit(0,&tdm_api->cfg.tx_disable);
		}
		break;

	case WP_API_CMD_SET_EC_TAP:

		switch (usr_tdm_api.ec_tap){
			case 0:
			case 32:
			case 64:
			case 128:
				tdm_api->cfg.ec_tap = usr_tdm_api.ec_tap;
				break;

			default:
				usr_tdm_api.result=SANG_STATUS_INVALID_PARAMETER;
				DEBUG_EVENT("%s: Illegal Echo Cancellation Tap \n",
						tdm_api->name);
				err = -EINVAL;
				goto tdm_api_exit;
		}
		break;

	case WP_API_CMD_GET_EC_TAP:
		usr_tdm_api.ec_tap = tdm_api->cfg.ec_tap;
		break;


	case WP_API_CMD_ENABLE_HWEC:
		if (card && card->wandev.ec_enable) {
			wan_smp_flag_t smp_flags1;
			card->hw_iface.hw_lock(card->hw,&smp_flags1);
                	card->wandev.ec_enable(card, 1, tdm_api->tdm_chan);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);
		}
		break;

	case WP_API_CMD_DISABLE_HWEC:
		if (card && card->wandev.ec_enable) {
			wan_smp_flag_t smp_flags1;
			card->hw_iface.hw_lock(card->hw,&smp_flags1);
                	card->wandev.ec_enable(card, 0, tdm_api->tdm_chan);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);
		} else {
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
			err = -EOPNOTSUPP;
		}
		break;

	case WP_API_CMD_GET_HW_DTMF:
		if (card && card->wandev.ec_enable && card->u.aft.tdmv_hw_tone == WANOPT_YES) {
			usr_tdm_api.hw_dtmf = WANOPT_YES;
		} else {
			usr_tdm_api.hw_dtmf = WANOPT_NO;
		}
		break;

	case WP_API_CMD_GET_STATS:
		if (WPTDM_SPAN_OP_MODE(tdm_api) || tdm_api->hdlc_framing) {
			if (tdm_api->driver_ctrl) {
					err = tdm_api->driver_ctrl(tdm_api->chan,usr_tdm_api.cmd,&usr_tdm_api);

					/* Overwrite using the TDM API values for rx only */
					if (err == 0) {
						usr_tdm_api.stats.max_rx_queue_length =  (u8)tdm_api->cfg.rx_queue_sz;
						wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
						usr_tdm_api.stats.current_number_of_frames_in_rx_queue =  wan_skb_queue_len(&tdm_api->wp_rx_list);
						wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
						
					}
			} else {
				usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
				err = -EOPNOTSUPP;
			}
		} else {
			memcpy(&usr_tdm_api.stats,&tdm_api->cfg.stats,sizeof(tdm_api->cfg.stats));
			usr_tdm_api.stats.max_rx_queue_length =  (u8)tdm_api->cfg.rx_queue_sz; 
			usr_tdm_api.stats.max_tx_queue_length = (u8)tdm_api->cfg.tx_queue_sz;
			wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
			usr_tdm_api.stats.current_number_of_frames_in_tx_queue = wan_skb_queue_len(&tdm_api->wp_tx_list);
			usr_tdm_api.stats.current_number_of_frames_in_rx_queue = wan_skb_queue_len(&tdm_api->wp_rx_list);
			wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
		}

		if (err == 0) {
			wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
			usr_tdm_api.stats.max_event_queue_length = wan_skb_queue_len(&tdm_api->wp_event_list)+wan_skb_queue_len(&tdm_api->wp_event_free_list);
			usr_tdm_api.stats.current_number_of_events_in_event_queue = wan_skb_queue_len(&tdm_api->wp_event_list);
			wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);

			usr_tdm_api.stats.rx_events_dropped	= tdm_api->cfg.stats.rx_events_dropped;
			usr_tdm_api.stats.rx_events 		= tdm_api->cfg.stats.rx_events;
			usr_tdm_api.stats.rx_events_tone 	= tdm_api->cfg.stats.rx_events_tone;
		}
		break;

	case WP_API_CMD_RESET_STATS:
		if (WPTDM_SPAN_OP_MODE(tdm_api) || tdm_api->hdlc_framing) {
			if (tdm_api->driver_ctrl) {
					err = tdm_api->driver_ctrl(tdm_api->chan,usr_tdm_api.cmd,&usr_tdm_api);
			} else {
				usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
				err = -EOPNOTSUPP;
			}
		}
		/* Alwasy flush the tdm_api stats SPAN or CHAN mode*/
		memset(&tdm_api->cfg.stats, 0x00, sizeof(tdm_api->cfg.stats));
		break;

	case WP_API_CMD_SET_IDLE_FLAG:
		if (WPTDM_SPAN_OP_MODE(tdm_api) || tdm_api->hdlc_framing) {
			if (tdm_api->driver_ctrl) {
				err = tdm_api->driver_ctrl(tdm_api->chan, usr_tdm_api.cmd, &usr_tdm_api);
			} else {
				usr_tdm_api.result = SANG_STATUS_OPTION_NOT_SUPPORTED;
				err = -EOPNOTSUPP;
			}
		}
		/* Set the new idle_flag, for both SPAN or CHAN mode. */
		tdm_api->cfg.idle_flag = usr_tdm_api.idle_flag;
		break;

	case WP_API_CMD_GET_FULL_CFG:
		memcpy(&usr_tdm_api.data[0],&tdm_api->cfg,sizeof(wanpipe_api_dev_cfg_t));
		break;

	case WP_API_CMD_ENABLE_RBS_EVENTS:
		/* 'usr_tdm_api.rbs_poll' is the user provided 'number of polls per second' */
		if (usr_tdm_api.rbs_poll < 20 || usr_tdm_api.rbs_poll > 100) {
			DEBUG_EVENT("%s: Error: Invalid RBS Poll Count Min=20 Max=100\n",
					tdm_api->name);
			
			usr_tdm_api.result=SANG_STATUS_INVALID_PARAMETER;
			err=-EINVAL;
		} else {

			err=-EOPNOTSUPP;
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;

			if (tdm_card_dev->rbs_enable_cnt == 0) {
				if (card && card->wandev.fe_iface.set_fe_sigctrl){
						wan_smp_flag_t smp_flags1;
						card->hw_iface.hw_lock(card->hw,&smp_flags1);
						err=card->wandev.fe_iface.set_fe_sigctrl(
								&card->fe,
								WAN_TE_SIG_POLL,
								ENABLE_ALL_CHANNELS,
								WAN_ENABLE);
						card->hw_iface.hw_unlock(card->hw,&smp_flags1);
				}

				if (err == 0) {
					tdm_card_dev->rbs_enable_cnt++;
					tdm_card_dev->rbs_poll_timeout =  (HZ * usr_tdm_api.rbs_poll) / 1000;
					tdm_api->cfg.rbs_poll = usr_tdm_api.rbs_poll;
					usr_tdm_api.result=SANG_STATUS_SUCCESS;
					err=0;
				}

			} else {
				usr_tdm_api.result=SANG_STATUS_SUCCESS;
				err=0;
			}

		}
		break;

	case WP_API_CMD_DISABLE_RBS_EVENTS:

		if (tdm_card_dev->rbs_enable_cnt == 1) {

			tdm_card_dev->rbs_poll_timeout=0;
			tdm_api->cfg.rbs_poll=0;
	
			if (card && card->wandev.fe_iface.set_fe_sigctrl){
				wan_smp_flag_t smp_flags1;
				card->hw_iface.hw_lock(card->hw,&smp_flags1);				
				err=card->wandev.fe_iface.set_fe_sigctrl(
							&card->fe,
							WAN_TE_SIG_POLL,
							ENABLE_ALL_CHANNELS,
							WAN_DISABLE);
				card->hw_iface.hw_unlock(card->hw,&smp_flags1);
			}else{
				usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
				err=-EOPNOTSUPP;
			}
		}

		if (tdm_card_dev->rbs_enable_cnt > 0) {
			tdm_card_dev->rbs_enable_cnt--;
		}

		break;

	case WP_API_CMD_WRITE_RBS_BITS:

		if (tdm_api->write_rbs_bits) {
			err=tdm_api->write_rbs_bits(
							tdm_api->chan,
							usr_tdm_api.chan,
							(u8)usr_tdm_api.rbs_tx_bits);
			if (err) {
				usr_tdm_api.result=SANG_STATUS_IO_ERROR;
				DEBUG_EVENT("%s: WRITE RBS Error (%i)\n",tdm_api->name,err);
			}
		} else {
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
			err=-EOPNOTSUPP;
		}
		break;

	case WP_API_CMD_READ_RBS_BITS:

		if (tdm_api->read_rbs_bits) {
			err=tdm_api->read_rbs_bits(
							tdm_api->chan,
							usr_tdm_api.chan,
							(u8*)&usr_tdm_api.rbs_rx_bits);
			if (err) {
				usr_tdm_api.result=SANG_STATUS_IO_ERROR;
				DEBUG_EVENT("%s: READ RBS Error (%d)\n",tdm_api->name,err);
			}
			DEBUG_TEST("%s: READ RBS  (0x%X)\n",tdm_api->name,usr_tdm_api.rbs_rx_bits);
		} else {
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
			err=-EOPNOTSUPP;
		}
		break;

	case WP_API_CMD_GET_FE_STATUS:
		if (card && card->wandev.fe_iface.get_fe_status){
			wan_smp_flag_t smp_flags1;
			card->hw_iface.hw_lock(card->hw,&smp_flags1);
			card->wandev.fe_iface.get_fe_status(
					&card->fe, &usr_tdm_api.fe_status,tdm_api->tdm_chan);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);
		}else{
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
			err=-EOPNOTSUPP;
			DEBUG_EVENT("%s: Warning: WP_API_CMD_GET_FE_STATUS request is not supported!\n",
						tdm_api->name);
		}
		break;

	case WP_API_CMD_SET_FE_STATUS:
		if (card && card->wandev.fe_iface.set_fe_status){
			wan_smp_flag_t smp_flags1;
			card->hw_iface.hw_lock(card->hw,&smp_flags1);
                 	card->wandev.fe_iface.set_fe_status(
					&card->fe, usr_tdm_api.fe_status);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);
		}else{
			usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
			err=-EOPNOTSUPP;
			DEBUG_EVENT("%s: Warning: WP_API_CMD_SET_FE_STATUS request is not supported!\n",
						tdm_api->name);
		}
		break;

	case WP_API_CMD_SET_EVENT:
		err = wanpipe_tdm_api_event_ioctl(tdm_api, &usr_tdm_api);
		if (err) {
			if (err==-EBUSY) {
				usr_tdm_api.result=SANG_STATUS_DEVICE_BUSY;
			} else if (err==-EINVAL) {
				usr_tdm_api.result=SANG_STATUS_INVALID_PARAMETER;
			} else {
				usr_tdm_api.result=SANG_STATUS_IO_ERROR;
			}
		}
		break;

	case WP_API_CMD_READ_EVENT:
		wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
		skb=wan_skb_dequeue(&tdm_api->wp_event_list);
		wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
		if (!skb){
			usr_tdm_api.result=SANG_STATUS_NO_DATA_AVAILABLE;
			err=-ENOBUFS;
			break;
		}

		err = 0;
		memcpy(&usr_tdm_api.event,wan_skb_data(skb),sizeof(wp_api_event_t));

		wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
		wan_skb_init(skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
		wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
		break;

	case WP_API_CMD_GET_HW_CODING:
		usr_tdm_api.hw_tdm_coding = tdm_api->cfg.hw_tdm_coding;
		break;

	case WP_API_CMD_SET_RX_GAINS:

		if (usr_tdm_api.data_len && utdmapi->data) {

			if (usr_tdm_api.data_len != 256) {
				usr_tdm_api.result=SANG_STATUS_INVALID_PARAMETER;
				err=-EINVAL;
				break;
			}
			
			wan_spin_unlock(&tdm_api->lock, &flags); 	

			if (!rx_gains) {
				rx_gains = wan_malloc(usr_tdm_api.data_len);
				if (!rx_gains) {
					usr_tdm_api.result=SANG_STATUS_FAILED_ALLOCATE_MEMORY;
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
					usr_tdm_api.result=SANG_STATUS_FAILED_TO_LOCK_USER_MEMORY;
	       			err=-EFAULT;
				goto tdm_api_unlocked_exit;
			}
#endif
			wan_spin_lock(&tdm_api->lock,&flags);

			tdm_api->rx_gain = rx_gains;

		} else {
			usr_tdm_api.result=SANG_STATUS_FAILED_TO_LOCK_USER_MEMORY;
			err=-EFAULT;
		}

		break;

	case WP_API_CMD_GET_FE_ALARMS:
		usr_tdm_api.fe_alarms = tdm_api->cfg.fe_alarms;
		break;

	case WP_API_CMD_SET_TX_GAINS:

		if (usr_tdm_api.data_len && utdmapi->data) {
			if (usr_tdm_api.data_len != 256) {
				usr_tdm_api.result=SANG_STATUS_INVALID_PARAMETER;
				err=-EINVAL;
				break;
			}

			wan_spin_unlock(&tdm_api->lock, &flags);

			if (!tx_gains) {
				tx_gains = wan_malloc(usr_tdm_api.data_len);
				if (!tx_gains) {
					usr_tdm_api.result=SANG_STATUS_FAILED_ALLOCATE_MEMORY;
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
				usr_tdm_api.result=SANG_STATUS_FAILED_TO_LOCK_USER_MEMORY;
				err=-EFAULT;
				goto tdm_api_unlocked_exit;
			}
#endif
		       	wan_spin_lock(&tdm_api->lock,&flags);
		       	tdm_api->tx_gain = tx_gains;

		} else {
				usr_tdm_api.result=SANG_STATUS_FAILED_TO_LOCK_USER_MEMORY;
				err=-EFAULT;
		}

		break;



	case WP_API_CMD_CLEAR_RX_GAINS:
		tdm_api->rx_gain = NULL;
		break;

	case WP_API_CMD_CLEAR_TX_GAINS:
		tdm_api->tx_gain = NULL;
		break;

	case WP_API_CMD_FLUSH_BUFFERS:
		wp_tdmapi_init_buffs(tdm_api, WP_API_FLUSH_ALL);
		break;

	case WP_API_CMD_FLUSH_RX_BUFFERS:
		wp_tdmapi_init_buffs(tdm_api, WP_API_FLUSH_RX);
		break;

	case WP_API_CMD_FLUSH_TX_BUFFERS:
		wp_tdmapi_init_buffs(tdm_api, WP_API_FLUSH_TX);
		break;

	case WP_API_CMD_FLUSH_EVENT_BUFFERS:
		wp_tdmapi_init_buffs(tdm_api, WP_API_FLUSH_EVENT);
		break;

	case WP_API_CMD_SET_TX_Q_SIZE:

		if (usr_tdm_api.tx_queue_sz) {
			if (WPTDM_SPAN_OP_MODE(tdm_api) || tdm_api->hdlc_framing) {
				if (tdm_api->driver_ctrl) {
					err = tdm_api->driver_ctrl(tdm_api->chan,usr_tdm_api.cmd,&usr_tdm_api);
				} else {
					err=-EFAULT;
				}
			} else {
				tdm_api->cfg.tx_queue_sz =usr_tdm_api.tx_queue_sz;
			}
		} else {
			err=-EINVAL;
		}

		break;

	case WP_API_CMD_GET_TX_Q_SIZE:

		if (WPTDM_SPAN_OP_MODE(tdm_api) || tdm_api->hdlc_framing) {
			if (tdm_api->driver_ctrl) {
				err = tdm_api->driver_ctrl(tdm_api->chan,usr_tdm_api.cmd,&usr_tdm_api);
			} else {
				err=-EFAULT;
			}
		} else {
			usr_tdm_api.tx_queue_sz = tdm_api->cfg.tx_queue_sz;
		}

		break;

	case WP_API_CMD_SET_RX_Q_SIZE:

		if (usr_tdm_api.rx_queue_sz) {
			tdm_api->cfg.rx_queue_sz =usr_tdm_api.rx_queue_sz;
		} else {
			err=-EINVAL;
		}
		break;

	case WP_API_CMD_GET_RX_Q_SIZE:
		usr_tdm_api.rx_queue_sz = tdm_api->cfg.rx_queue_sz;
		break;

	case WP_API_CMD_DRIVER_VERSION:
	case WP_API_CMD_FIRMWARE_VERSION:
	case WP_API_CMD_CPLD_VERSION:
	case WP_API_CMD_GEN_FIFO_ERR:
		if (tdm_api->driver_ctrl) {
			err = tdm_api->driver_ctrl(tdm_api->chan,usr_tdm_api.cmd,&usr_tdm_api);
		} else {
			err=-EFAULT;
		}
		break;

	case WP_API_CMD_SET_RM_RXFLASHTIME:
		if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
			if (fe->rm_param.mod[(tdm_api->tdm_chan) - 1].type == MOD_TYPE_FXS) {
				DEBUG_EVENT("%s: Module %d: rxflashtime changed (%d -> %d)\n",
					fe->name,
					tdm_api->tdm_chan,
					fe->rm_param.mod[(tdm_api->tdm_chan) - 1].u.fxs.rxflashtime,
					usr_tdm_api.rxflashtime);
				fe->rm_param.mod[(tdm_api->tdm_chan) - 1].u.fxs.rxflashtime=usr_tdm_api.rxflashtime;						
			} else {
				DEBUG_EVENT("Warning %s: Module %d: WP_API_CMD_SET_RM_RXFLASHTIME is only valid for FXS module!\n", 
					fe->name,tdm_api->tdm_chan);
				err=-EOPNOTSUPP;
			}
		} else {
				DEBUG_EVENT("Warning %s : Unsupported Protocol/Card %s WP_API_CMD_SET_RM_RXFLASHTIME is only valid for analog!\n", 
					fe->name,SDLA_DECODE_PROTOCOL(card->wandev.config_id));
				err=-EOPNOTSUPP;
		}
		break;

	default:
		DEBUG_EVENT("%s: Invalid TDM API CMD %i\n", tdm_api->name,cmd);
		usr_tdm_api.result=SANG_STATUS_OPTION_NOT_SUPPORTED;
		err=-EOPNOTSUPP;
		break;
	}

tdm_api_exit:
	wan_spin_unlock(&tdm_api->lock, &flags);

tdm_api_unlocked_exit:

	usr_tdm_api.result = err;

#if defined(__WINDOWS__)
	/* udata is a pointer to wanpipe_api_cmd_t */
	memcpy(udata, &usr_tdm_api, sizeof(wanpipe_api_cmd_t));
#else
	if (WAN_COPY_TO_USER(udata,
			     &usr_tdm_api,
			     sizeof(wanpipe_api_cmd_t))){
		usr_tdm_api.result=SANG_STATUS_FAILED_TO_LOCK_USER_MEMORY;
	    return -EFAULT;
	}
#endif

	return err;

}

static int wanpipe_tdm_api_ioctl(void *obj, int cmd, void *udata)
{
	wanpipe_tdm_api_dev_t *tdm_api = (wanpipe_tdm_api_dev_t*)obj;
	int err=0;

	if (!tdm_api->chan && tdm_api != &tdmapi_ctrl){
		DEBUG_EVENT("%s:%d Error: TDM API Not initialized! chan=NULL!\n",
			__FUNCTION__,__LINE__);
		return -EFAULT;
	}

	if (!udata){
		return -EINVAL;
	}

	switch (cmd) {

#if defined (__LINUX__)
	case SIOC_WANPIPE_TDM_API:
#endif
	case WANPIPE_IOCTL_API_CMD:
		err=wanpipe_tdm_api_ioctl_handle_tdm_api_cmd(tdm_api,udata);
		break;

	case WANPIPE_IOCTL_PIPEMON:
		err=-EINVAL;
		if (tdm_api->pipemon) {
			err=tdm_api->pipemon(tdm_api->card,tdm_api->chan,udata);
		}
		break;

	default:
		err=-EINVAL;
		break;
	}	

	return err;
}

static int
wanpipe_tdm_api_event_ioctl(wanpipe_tdm_api_dev_t *tdm_api, wanpipe_api_cmd_t *tdm_cmd)
{
	wp_api_event_t	*tdm_event;
	wan_event_ctrl_t	event_ctrl;
	sdla_t *card=tdm_api->card;

	if (tdm_api->event_ctrl == NULL){
		DEBUG_EVENT("%s: Error: Event control interface doesn't initialized!\n",
				tdm_api->name);
		return -EINVAL;
	}

	tdm_event = &tdm_cmd->event;
	memset(&event_ctrl, 0, sizeof(wan_event_ctrl_t));

	switch(tdm_event->wp_api_event_type){


	case WP_API_EVENT_DTMF:

        	event_ctrl.type         = WAN_EVENT_EC_DTMF;

    		if (!card->wandev.ec_enable || card->wandev.ec_enable_map == 0){
              	if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
                       	event_ctrl.type         = WAN_EVENT_RM_DTMF;
                       	event_ctrl.mod_no       = tdm_api->tdm_chan;
                       	DEBUG_EVENT("%s: %s HW RM DTMF event %X!\n",
                                	tdm_api->name,
                                	WP_API_EVENT_MODE_DECODE(tdm_event->wp_api_event_mode),
                                	tdm_api->active_ch);
        		}
    		} else {

                DEBUG_EVENT("%s: %s HW EC DTMF event %X %p !\n",
                          tdm_api->name,
                          WP_API_EVENT_MODE_DECODE(tdm_event->wp_api_event_mode),
                           tdm_api->active_ch,card->wandev.ec_enable);

    		}

			// Octasic DTMF event
			if (tdm_event->wp_api_event_mode == WP_API_EVENT_ENABLE){
					event_ctrl.mode = WAN_EVENT_ENABLE;
			}else{
					event_ctrl.mode = WAN_EVENT_DISABLE;
			}
			if(tdm_event->channel < 1 || tdm_event->channel > NUM_OF_E1_CHANNELS - 1/* 31 */){
					DEBUG_TDMAPI("%s(): %s: Warning: DTMF control requested on invalid channel %u!\n",
							__FUNCTION__, tdm_api->name, tdm_event->channel);
					tdm_event->channel = 1;/* */
			}
			event_ctrl.channel      = tdm_event->channel;
			break;


	case WP_API_EVENT_RM_DTMF:
		// A200-Remora DTMF event
		DEBUG_TDMAPI("%s: %s A200-Remora DTMF event!\n",
			tdm_api->name,
			WP_API_EVENT_MODE_DECODE(tdm_event->wp_api_event_mode));
		event_ctrl.type		= WAN_EVENT_RM_DTMF;
		if (tdm_event->wp_api_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_RXHOOK:
		DEBUG_TDMAPI("%s: %s A200-Remora Loop Closure event!\n",
			tdm_api->name,
			WP_API_EVENT_MODE_DECODE(tdm_event->wp_api_event_mode));
		event_ctrl.type		= WAN_EVENT_RM_LC;
		if (tdm_event->wp_api_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_RING:
		DEBUG_TDMAPI("%s: %s Ring Event on module %d!\n",
			tdm_api->name,
			WP_API_EVENT_MODE_DECODE(tdm_event->wp_api_event_mode),
			tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_RING;
		if (tdm_event->wp_api_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_RING_DETECT:
		DEBUG_TDMAPI("%s: %s Ring Detection Event on module %d!\n",
			tdm_api->name,
			WP_API_EVENT_MODE_DECODE(tdm_event->wp_api_event_mode),
			tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_RING_DETECT;
		if (tdm_event->wp_api_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_RING_TRIP_DETECT:
		DEBUG_TDMAPI("%s: %s Ring Trip Detection Event on module %d!\n",
			tdm_api->name,
			WP_API_EVENT_MODE_DECODE(tdm_event->wp_api_event_mode),
			tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_RING_TRIP;
		if (tdm_event->wp_api_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_TONE:

		DEBUG_TDMAPI("%s: %s Tone Event (%d)on module %d!\n",
			tdm_api->name,
			WP_API_EVENT_MODE_DECODE(tdm_event->wp_api_event_mode),
			tdm_event->wp_api_event_tone_type,
			tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_TONE;
		if (tdm_event->wp_api_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;
			switch(tdm_event->wp_api_event_tone_type){
			case WP_API_EVENT_TONE_DIAL:
				event_ctrl.tone	= WAN_EVENT_RM_TONE_TYPE_DIAL;
				break;
			case WP_API_EVENT_TONE_BUSY:
				event_ctrl.tone	= WAN_EVENT_RM_TONE_TYPE_BUSY;
				break;
			case WP_API_EVENT_TONE_RING:
				event_ctrl.tone	= WAN_EVENT_RM_TONE_TYPE_RING;
				break;
			case WP_API_EVENT_TONE_CONGESTION:
				event_ctrl.tone	= WAN_EVENT_RM_TONE_TYPE_CONGESTION;
				break;
			default:
				DEBUG_EVENT("%s: Unsupported TDM API Tone Type  %d!\n",
						tdm_api->name,
						tdm_event->wp_api_event_tone_type);
				return -EINVAL;
			}
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;
		}
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_TXSIG_KEWL:
		DEBUG_TDMAPI("%s: TX Signalling KEWL on module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type   	= WAN_EVENT_RM_TXSIG_KEWL;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_TXSIG_START:
		DEBUG_TDMAPI("%s: TX Signalling START for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_TXSIG_START;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_TXSIG_OFFHOOK:
		DEBUG_TDMAPI("%s: TX Signalling OFFHOOK for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_TXSIG_OFFHOOK;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_TXSIG_ONHOOK:
		DEBUG_TDMAPI("%s: TX Signalling ONHOOK for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_TXSIG_ONHOOK;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		break;

	case WP_API_EVENT_ONHOOKTRANSFER:
		DEBUG_TDMAPI("%s: RM ONHOOKTRANSFER for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_ONHOOKTRANSFER;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		event_ctrl.ohttimer	= tdm_event->wp_api_event_ohttimer;
		break;

	case WP_API_EVENT_SETPOLARITY:
		DEBUG_EVENT("%s: RM SETPOLARITY for module %d!\n",
				tdm_api->name, tdm_api->tdm_chan);
		event_ctrl.type		= WAN_EVENT_RM_SETPOLARITY;
		event_ctrl.mod_no	= tdm_api->tdm_chan;
		event_ctrl.polarity	= tdm_event->wp_api_event_polarity;
		break;

	case WP_API_EVENT_BRI_CHAN_LOOPBACK:
		event_ctrl.type		= WAN_EVENT_BRI_CHAN_LOOPBACK;
		event_ctrl.channel	= tdm_event->channel;

		if (tdm_event->wp_api_event_mode == WP_API_EVENT_ENABLE){
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
				tdm_event->wp_api_event_type);
		return -EINVAL;
	}

	return tdm_api->event_ctrl(tdm_api->chan, &event_ctrl);
}

static int wanpipe_tdm_api_channelized_tx (wanpipe_tdm_api_dev_t *tdm_api, u8 *tx_data, int len)
{
	u8 *buf;
	wp_api_hdr_t *tx_hdr;

	if (wan_test_bit(0,&tdm_api->cfg.tx_disable)){
		return 0;
	}

	if (!tdm_api->tx_skb) {
		tdm_api->tx_skb=wan_skb_dequeue(&tdm_api->wp_tx_list);
		if (!tdm_api->tx_skb){
			memset(tx_data,tdm_api->cfg.idle_flag,len);
			tdm_api->cfg.stats.tx_carrier_errors++;
			tdm_api->cfg.stats.tx_idle_packets++;
			return -ENOBUFS;
		}

		wp_wakeup_tx_tdmapi(tdm_api);
		buf=wan_skb_pull(tdm_api->tx_skb,sizeof(wp_api_hdr_t));
		memcpy(&tdm_api->tx_hdr, buf, sizeof(wp_api_hdr_t));
		tdm_api->cfg.stats.tx_bytes+=wan_skb_len(tdm_api->tx_skb);

		if (wan_skb_len(tdm_api->tx_skb) % len) {
			WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,tx_errors);
			goto wanpipe_tdm_api_tx_error;
		}
	}

	tx_hdr=&tdm_api->tx_hdr;
	
	if (wan_skb_len(tdm_api->tx_skb) < len) {
#if 0
			if (WAN_NET_RATELIMIT()) {
				DEBUG_EVENT("%s: TX LEN %i Less then Expecting %i\n",
						tdm_api->name, wan_skb_len(tdm_api->tx_skb) , len);
			}
#endif
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,tx_errors);
		goto wanpipe_tdm_api_tx_error;
	}

	buf=(u8*)wan_skb_data(tdm_api->tx_skb);
	memcpy(tx_data,buf,len);
	wan_skb_pull(tdm_api->tx_skb,len);

	if (wan_skb_len(tdm_api->tx_skb) <= 0) {
		tdm_api->cfg.stats.tx_packets++;
		wan_skb_init(tdm_api->tx_skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(tdm_api->tx_skb,0);
		wan_skb_queue_tail(&tdm_api->wp_tx_free_list,tdm_api->tx_skb);
		tdm_api->tx_skb=NULL;
	}

	return 0;

wanpipe_tdm_api_tx_error:
			
	memset(tx_data,tdm_api->cfg.idle_flag,len);
	tdm_api->cfg.stats.tx_carrier_errors++;
	tdm_api->cfg.stats.tx_idle_packets++;

	if (tdm_api->tx_skb) {
		wan_skb_init(tdm_api->tx_skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(tdm_api->tx_skb,0);
		wan_skb_queue_tail(&tdm_api->wp_tx_free_list,tdm_api->tx_skb);
		tdm_api->tx_skb=NULL;
	}

	tdm_api->tx_skb=NULL;
	return -1;

}

static int wanpipe_tdm_api_channelized_rx (wanpipe_tdm_api_dev_t *tdm_api, u8 *rx_data, u8 *tx_data, int len)
{
	u8 *data_ptr;
	wp_api_hdr_t *rx_hdr;
	int err=-EINVAL;

	if (wan_test_bit(0,&tdm_api->cfg.rx_disable)) {
		err=0;
		goto wanpipe_tdm_api_rx_error;
	}

	if (wan_skb_queue_len(&tdm_api->wp_rx_list) >= (int)tdm_api->cfg.rx_queue_sz) {
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_dropped);
		wp_wakeup_rx_tdmapi(tdm_api);
		err=-EBUSY;
		goto wanpipe_tdm_api_rx_error;
	}

	if (!tdm_api->rx_skb) {
		tdm_api->rx_skb=wan_skb_dequeue(&tdm_api->wp_rx_free_list);
		if (!tdm_api->rx_skb) {
			err=-ENOMEM;
			WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_errors);
			goto wanpipe_tdm_api_rx_error;
		}

		if (wan_skb_headroom(tdm_api->rx_skb) != sizeof(wp_api_hdr_t)) {
#if 0
			if (WAN_NET_RATELIMIT()) {
				DEBUG_EVENT("%s: RX HEAD ROOM =%i Expecting %i\n",
						tdm_api->name, wan_skb_headroom(tdm_api->rx_skb), sizeof(wp_api_hdr_t));
			}
#endif
			WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_errors);
			goto wanpipe_tdm_api_rx_error;
		}

		data_ptr=wan_skb_put(tdm_api->rx_skb,sizeof(wp_api_hdr_t));
	}

	if (len > wan_skb_tailroom(tdm_api->rx_skb)) {
		err=-EINVAL;
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_errors);
#if 0
		if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: RX TAIL ROOM =%i Less than Len %i\n",
						tdm_api->name, wan_skb_tailroom(tdm_api->rx_skb), len);
		}
#endif
		goto wanpipe_tdm_api_rx_error;
	}

	rx_hdr=(wp_api_hdr_t*)wan_skb_data(tdm_api->rx_skb);

	data_ptr=wan_skb_put(tdm_api->rx_skb,len);
	memcpy((u8*)data_ptr,rx_data,len);

	if (wan_skb_len(tdm_api->rx_skb) >=
	   (int)(tdm_api->cfg.usr_period*tdm_api->cfg.hw_mtu_mru + sizeof(wp_api_hdr_t))) {
		tdm_api->cfg.stats.rx_bytes+=wan_skb_len(tdm_api->rx_skb)-sizeof(wp_api_hdr_t);
		wan_skb_queue_tail(&tdm_api->wp_rx_list,tdm_api->rx_skb);
		tdm_api->rx_skb=NULL;
		wp_wakeup_rx_tdmapi(tdm_api);
		tdm_api->cfg.stats.rx_packets++;
		
	}

	return 0;

wanpipe_tdm_api_rx_error:

	if (tdm_api->rx_skb) {
		netskb_t *skb = tdm_api->rx_skb;
		tdm_api->rx_skb = NULL;

		wan_skb_init(skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_rx_free_list,skb);
	}

	tdm_api->rx_skb=NULL;

	return err;
}


int wanpipe_tdm_api_rx_tx (wanpipe_tdm_api_dev_t *tdm_api, u8 *rx_data, u8 *tx_data, int len)
{

	if (!tdm_api || !rx_data || !tx_data || len == 0) {
		if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s:%d Internal Error: tdm_api=%p rx_data=%p tx_data=%p len=%i\n",
					__FUNCTION__,__LINE__,tdm_api,rx_data,tx_data,len);
		}
		return 0;
	}

	if (!tdm_api->chan || !wan_test_bit(0,&tdm_api->init)){
		return 0;
	}

	if (!wan_test_bit(0,&tdm_api->used)) {
		/* Always use 'cfg.idle_flag' because it may be it was set by WP_API_CMD_SET_IDLE_FLAG */
		memset(tx_data,tdm_api->cfg.idle_flag,len);
		return 0;
	}

	wanpipe_tdm_api_channelized_rx (tdm_api, rx_data, tx_data, len);
	wanpipe_tdm_api_channelized_tx (tdm_api, tx_data, len);


	return 0;
}

int wanpipe_tdm_api_span_rx (wanpipe_tdm_api_dev_t *tdm_api, netskb_t *skb)
{
	sdla_t *card;
	wan_smp_flag_t flag;

	flag=0;
	
	if (!tdm_api->chan || !wan_test_bit(0,&tdm_api->init) || !tdm_api->card){
		return -ENODEV;
	}

	if (!wan_test_bit(0,&tdm_api->used)) {
		return -ENODEV;
	}

	card=tdm_api->card;

	if (wan_skb_len(skb) <= sizeof(wp_api_hdr_t)) {
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_errors);
		if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: Internal Error RX Data has no rx header!\n",
					__FUNCTION__);	
		}
		return -EINVAL;
	}

	wptdm_os_lock_irq(&card->wandev.lock,&flag);
	if (wan_skb_queue_len(&tdm_api->wp_rx_list) >= (int)tdm_api->cfg.rx_queue_sz) {
		wptdm_os_unlock_irq(&card->wandev.lock,&flag);
		wp_wakeup_rx_tdmapi(tdm_api);
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_dropped);
		if (WAN_NET_RATELIMIT()) {
			DEBUG_TEST("%s: Error RX Buffer Overrun!\n",
					__FUNCTION__);	
		}
		return -EBUSY;
	}

	wan_skb_queue_tail(&tdm_api->wp_rx_list,skb);
	tdm_api->cfg.stats.rx_packets++;
	wptdm_os_unlock_irq(&card->wandev.lock,&flag);

	wp_wakeup_rx_tdmapi(tdm_api);

	return 0;
}


static wanpipe_tdm_api_dev_t *wp_tdmapi_search(sdla_t *card, int fe_chan)
{
	wanpipe_tdm_api_dev_t	*tdm_api;

	if(fe_chan < 0 || fe_chan >= MAX_TDM_API_CHANNELS){
		DEBUG_EVENT("%s(): TDM API Error: Invalid Channel Number=%i!\n",
			__FUNCTION__, fe_chan);
		return NULL;
	}

	tdm_api = card->wp_tdmapi_hash[fe_chan];
	if(tdm_api == NULL){
		DEBUG_TEST("%s(): TDM API Warning: No TDM API registered for Channel Number=%i!\n",
			__FUNCTION__, fe_chan);
		return NULL;
	}


	if (!wan_test_bit(0,&tdm_api->init)) {
		return NULL;
	}

	return tdm_api;
}

static void wp_tdmapi_report_rbsbits(void* card_id, int channel, unsigned char rbsbits)
{
	netskb_t                *skb;
	wanpipe_tdm_api_dev_t   *tdm_api = NULL;
	sdla_t                  *card = (sdla_t*)card_id;
	wp_api_event_t  *p_tdmapi_event = NULL;
	wan_smp_flag_t irq_flags;

	irq_flags=0;

	tdm_api = wp_tdmapi_search(card, channel);
	if (tdm_api == NULL){
		return;
	}

	if (tdm_api->rbs_rx_bits[channel] == rbsbits) {
		return;
	}

	DEBUG_TEST("%s: Received RBS Bits Event at TDM_API channel=%d rbs=0x%02X!\n",
					card->devname,
					channel,rbsbits);

	tdm_api->rbs_rx_bits[channel] = rbsbits;

	wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);

	skb=wan_skb_dequeue(&tdm_api->wp_event_free_list);
	if (skb == NULL) {
			wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
			WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
			return;
	}

	p_tdmapi_event = (wp_api_event_t*)wan_skb_put(skb,sizeof(wp_api_event_t));

	memset(p_tdmapi_event, 0, sizeof(wp_api_event_t));



	p_tdmapi_event->wp_api_event_type = WP_API_EVENT_RBS;
	p_tdmapi_event->wp_api_event_channel = (u8)channel;
	p_tdmapi_event->wp_api_event_span = wp_tdmapi_get_span(card);

	p_tdmapi_event->wp_api_event_rbs_bits	= (u8)rbsbits;

	wan_skb_push_to_ctrl_event(skb);
	
	if (!wan_test_bit(0,&tdm_api->used)) {
			wan_skb_init(skb,sizeof(wp_api_hdr_t));
			wan_skb_trim(skb,0);
			wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
			wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
			return;
	}

	wanpipe_tdm_api_handle_event(tdm_api,skb);

	wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);


	return;

}




static void wp_tdmapi_alarms(void* card_id, wan_event_t *event)
{
	netskb_t		*skb;
	wanpipe_tdm_api_dev_t	*tdm_api = NULL;
	sdla_t			*card = (sdla_t*)card_id;
	wp_api_event_t	*p_tdmapi_event = NULL;
	wan_smp_flag_t irq_flags;

	irq_flags=0;


	DEBUG_TEST("%s: Received ALARM Event at TDM_API channel=%d alarm=0x%X!\n",
			card->devname,
			event->channel,
			event->alarms);

	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}

	wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
	skb=wan_skb_dequeue(&tdm_api->wp_event_free_list);
	if (skb == NULL) {
		wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
		return;
	}	

	p_tdmapi_event = (wp_api_event_t*)wan_skb_put(skb,sizeof(wp_api_event_t));

	memset(p_tdmapi_event, 0, sizeof(wp_api_event_t));

	p_tdmapi_event->wp_api_event_type = WP_API_EVENT_ALARM;
	p_tdmapi_event->wp_api_event_channel = event->channel;
	p_tdmapi_event->wp_api_event_span = wp_tdmapi_get_span(card);

	p_tdmapi_event->wp_api_event_alarm = event->alarms;			
	tdm_api->cfg.fe_alarms = event->alarms;

	wan_skb_push_to_ctrl_event(skb);

	if (!wan_test_bit(0,&tdm_api->used)) {
		wan_skb_init(skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
		wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
		return;
	}

	wanpipe_tdm_api_handle_event(tdm_api,skb);

	wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);


	return;
}

static void wp_tdmapi_report_alarms(void* card_id, unsigned long te_alarm)
{
	wan_event_t event;
	u8 i;

	memset(&event,0,sizeof(event));

	event.alarms=(u32)te_alarm;

	for (i=0; i<NUM_OF_E1_CHANNELS; i++) {
		event.channel=i;
		wp_tdmapi_alarms(card_id, &event);
	}

	return;
}


static void wp_tdmapi_tone (void* card_id, wan_event_t *event)
{
	netskb_t		*skb = NULL;
	wanpipe_tdm_api_dev_t	*tdm_api = NULL;
	sdla_t			*card = (sdla_t*)card_id;
	wp_api_event_t	*p_tdmapi_event = NULL;
	wan_smp_flag_t irq_flags;
	int 			lock=1;

	irq_flags=0;

	if (event->type == WAN_EVENT_EC_DTMF){
		DEBUG_TDMAPI("%s: Received Tone Event at TDM API (%d:%c:%s:%s)!\n",
			card->devname,
			event->channel,
			event->digit,
			(event->tone_port == WAN_EC_CHANNEL_PORT_ROUT)?"ROUT":"SOUT",
			(event->tone_type == WAN_EC_TONE_PRESENT)?"PRESENT":"STOP");
	}else if (event->type == WAN_EVENT_RM_DTMF){
		DEBUG_TDMAPI("%s: Received DTMF Event at TDM API (%d:%c)!\n",
			card->devname,
			event->channel,
			event->digit);
			lock=0;
			/* FIXME: Updated locking architecture */
	}

	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}

	if (lock) wptdm_os_lock_irq(&card->wandev.lock,&irq_flags);
	skb=wan_skb_dequeue(&tdm_api->wp_event_free_list);
	if (skb == NULL) {
		if (lock) wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
		return;
	}
	
	p_tdmapi_event = (wp_api_event_t*)wan_skb_put(skb,sizeof(wp_api_event_t));

	memset(p_tdmapi_event,0,sizeof(wp_api_event_t));
	p_tdmapi_event->wp_api_event_type	= WP_API_EVENT_DTMF;
	p_tdmapi_event->wp_api_event_dtmf_digit	= event->digit;
	p_tdmapi_event->wp_api_event_dtmf_type	= event->tone_type;
	p_tdmapi_event->wp_api_event_dtmf_port	= event->tone_port;

#ifdef WAN_TDMAPI_DTMF_TEST
#ifdef __LINUX__
#warning "Enabling DTMF TEST - Should only be done on debugging"
#endif
	{
		if (event->tone_type == WAN_EC_TONE_PRESENT) {
			u8 ntone=tdm_api->dtmf_check[event->channel].dtmf_tone_present;
			if (ntone != 0) {
				ntone = tdm_api->dtmf_check[event->channel].dtmf_tone_present + 1;
			
				if (ntone != event->digit) {
					DEBUG_EVENT("%s: Error: Digit mismatch Ch=%i Expecting %c  Received %c\n",
						tdm_api->name,event->channel, ntone, event->digit);
				}
			}
			tdm_api->dtmf_check[event->channel].dtmf_tone_present = event->digit;
		} else {
			u8 ntone=tdm_api->dtmf_check[event->channel].dtmf_tone_stop;

			if (ntone != 0) {
				ntone = tdm_api->dtmf_check[event->channel].dtmf_tone_stop + 1;
				
				if (ntone != event->digit) {
					DEBUG_EVENT("%s: Error: Digit mismatch Ch=%i Expecting %c  Received %c\n",
						tdm_api->name,event->channel, ntone, event->digit);
				}
			}
			tdm_api->dtmf_check[event->channel].dtmf_tone_stop = event->digit;
		}
	}
#endif

	p_tdmapi_event->channel = event->channel;
	p_tdmapi_event->span = wp_tdmapi_get_span(card);

#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif

	wan_skb_push_to_ctrl_event(skb);

	if (!wan_test_bit(0,&tdm_api->used)) {
		wan_skb_init(skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
		if (lock)  wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);
		return;
	}

	tdm_api->cfg.stats.rx_events_tone++;
	wanpipe_tdm_api_handle_event(tdm_api,skb);

	if (lock) wptdm_os_unlock_irq(&card->wandev.lock,&irq_flags);


	return;
}


static void wp_tdmapi_hook (void* card_id, wan_event_t *event)
{
	netskb_t		*skb;
	wanpipe_tdm_api_dev_t	*tdm_api = NULL;
	sdla_t			*card = (sdla_t*)card_id;
	wp_api_event_t	*p_tdmapi_event = NULL;

	DEBUG_TDMAPI("%s: Received RM LC Event at TDM_API (%d:%s)!\n",
			card->devname,
			event->channel,
			(event->rxhook==WAN_EVENT_RXHOOK_OFF)?"OFF-HOOK":"ON-HOOK");

	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		DEBUG_EVENT("%s: Error: failed to find API device. Discarding RM LC Event at TDM_API (%d:%s)!\n",
			card->devname,
			event->channel,
			(event->rxhook==WAN_EVENT_RXHOOK_OFF)?"OFF-HOOK":"ON-HOOK");
		return;
	}

	skb=wan_skb_dequeue(&tdm_api->wp_event_free_list);
	if (skb == NULL) {
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
		DEBUG_EVENT("%s: Error: no free buffers for API event. Discarding RM LC Event at TDM_API (%d:%s)!\n",
			card->devname,
			event->channel,
			(event->rxhook==WAN_EVENT_RXHOOK_OFF)?"OFF-HOOK":"ON-HOOK");
		return;
	}

	p_tdmapi_event = (wp_api_event_t*)wan_skb_put(skb,sizeof(wp_api_event_t));

	memset(p_tdmapi_event, 0, sizeof(wp_api_event_t));

	p_tdmapi_event->wp_api_event_type 		= WP_API_EVENT_RXHOOK;
	p_tdmapi_event->wp_api_event_channel	= event->channel;
	p_tdmapi_event->wp_api_event_span		= wp_tdmapi_get_span(card);

	switch(event->rxhook){
	case WAN_EVENT_RXHOOK_ON:
		p_tdmapi_event->wp_api_event_hook_state =
					WP_API_EVENT_RXHOOK_ON;
		break;
	case WAN_EVENT_RXHOOK_OFF:
		p_tdmapi_event->wp_api_event_hook_state =
					WP_API_EVENT_RXHOOK_OFF;
		break;
	case WAN_EVENT_RXHOOK_FLASH:
		p_tdmapi_event->wp_api_event_hook_state =
				WP_API_EVENT_RXHOOK_FLASH;
		break;
		}

#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif

	wan_skb_push_to_ctrl_event(skb);

	if (!wan_test_bit(0,&tdm_api->used)) {
		wan_skb_init(skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
		return;
	}

	wanpipe_tdm_api_handle_event(tdm_api,skb);

	return;
}



static void __wp_tdmapi_linkstatus (void* card_id, wan_event_t *event, int lock)
{
#define wptdm_queue_lock_irq(card,flags)    if (lock) wptdm_os_lock_irq(card,flags)
#define wptdm_queue_unlock_irq(card,flags)  if (lock) wptdm_os_unlock_irq(card,flags)

	netskb_t		*skb;
	wanpipe_tdm_api_dev_t	*tdm_api = NULL;
	sdla_t			*card = (sdla_t*)card_id;
	wp_api_event_t	*p_tdmapi_event = NULL;
	wan_smp_flag_t 	flags;

	flags=0;

	DEBUG_TDMAPI("%s: Received Link Status Event at TDM_API (%d:%s)!\n",
			card->devname,
			event->channel,
			(event->link_status==WAN_EVENT_LINK_STATUS_DISCONNECTED)?"Disconnected":"connected");

	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}

	if (tdm_api->state == event->link_status) {
		return;
	}

   	wptdm_queue_lock_irq(&card->wandev.lock,&flags);
	tdm_api->state = event->link_status;

	skb=wan_skb_dequeue(&tdm_api->wp_event_free_list);
	if (skb == NULL) {
   		wptdm_queue_unlock_irq(&card->wandev.lock,&flags);
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
		return;
	}

	p_tdmapi_event = (wp_api_event_t*)wan_skb_put(skb,sizeof(wp_api_event_t));

	memset(p_tdmapi_event, 0, sizeof(wp_api_event_t));
	p_tdmapi_event->wp_api_event_type		= WP_API_EVENT_LINK_STATUS;
	p_tdmapi_event->wp_api_event_channel	= event->channel;
	p_tdmapi_event->wp_api_event_span		= wp_tdmapi_get_span(card);

	switch(event->link_status){
	case WAN_EVENT_LINK_STATUS_CONNECTED:
		p_tdmapi_event->wp_api_event_link_status =
					WP_API_EVENT_LINK_STATUS_CONNECTED;
		wp_tdm_api_start(tdm_api);		

		break;
	case WAN_EVENT_LINK_STATUS_DISCONNECTED	:
		p_tdmapi_event->wp_api_event_link_status =
					WP_API_EVENT_LINK_STATUS_DISCONNECTED;
		wp_tdm_api_stop(tdm_api);
		break;
	}


	if (!wan_test_bit(0,&tdm_api->used)) {
		wan_skb_init(skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
   		
		wptdm_queue_unlock_irq(&card->wandev.lock,&flags);
	
		wan_skb_push_to_ctrl_event(skb);
		return;
	}

	wanpipe_tdm_api_handle_event(tdm_api,skb);

	wptdm_queue_unlock_irq(&card->wandev.lock,&flags);

#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif

   	wan_skb_push_to_ctrl_event(skb);

	return;
#undef wptdm_queue_lock_irq
#undef wptdm_queue_unlock_irq
}

static void wp_tdmapi_linkstatus (void* card_id, wan_event_t *event)
{
	u8 i;
	sdla_t *card = (sdla_t*)card_id;
	
	if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
		__wp_tdmapi_linkstatus(card_id, event, 0);
		return;
	}

	for (i=0; i<NUM_OF_E1_CHANNELS; i++) {
		event->channel=i;
		__wp_tdmapi_linkstatus(card_id, event, 1);
	}

	return;	

}

/*FXO
	Some analog line use reverse polarity to indicate line hangup 
*/


static void wp_tdmapi_polarityreverse (void* card_id, wan_event_t *event)
{
	netskb_t		*skb;
	wanpipe_tdm_api_dev_t	*tdm_api = NULL;
	sdla_t			*card = (sdla_t*)card_id;
	wp_api_event_t	*p_tdmapi_event = NULL;

	DEBUG_TDMAPI("%s: Received Polarity Reverse Event at TDM_API (%d: %s)!\n",
			card->devname,
			event->channel,(event->polarity_reverse==WAN_EVENT_POLARITY_REV_POSITIVE_NEGATIVE)?"+ve  to -ve":"-ve to +ve");

	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}

	skb=wan_skb_dequeue(&tdm_api->wp_event_free_list);
	if (skb == NULL) {
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
		return;
	}

	p_tdmapi_event = (wp_api_event_t*)wan_skb_put(skb,sizeof(wp_api_event_t));

	memset(p_tdmapi_event, 0, sizeof(wp_api_event_t));
	p_tdmapi_event->wp_api_event_type			= WP_API_EVENT_POLARITY_REVERSE;
	p_tdmapi_event->wp_api_event_channel		= event->channel;
	p_tdmapi_event->wp_api_event_span			= wp_tdmapi_get_span(card);
	
	switch(event->polarity_reverse){
	case WAN_EVENT_POLARITY_REV_POSITIVE_NEGATIVE:
		p_tdmapi_event->wp_api_event_polarity_reverse =
					WP_API_EVENT_POL_REV_POS_TO_NEG;
		wp_tdm_api_start(tdm_api);		

		break;
	case WAN_EVENT_POLARITY_REV_NEGATIVE_POSITIVE	:
		p_tdmapi_event->wp_api_event_polarity_reverse =
					WP_API_EVENT__POL_REV_NEG_TO_POS;
		wp_tdm_api_stop(tdm_api);
		break;
	}

	wan_skb_push_to_ctrl_event(skb);

	if (!wan_test_bit(0,&tdm_api->used)) {
		wan_skb_init(skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
		return;
	}

#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif

	wanpipe_tdm_api_handle_event(tdm_api,skb);

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
	wp_api_event_t	*p_tdmapi_event = NULL;

	DEBUG_TDMAPI("%s: Received RM RING TRIP Event at TDM_API (%d:%s)!\n",
			card->devname,
			event->channel,
			WAN_EVENT_RING_TRIP_DECODE(event->ring_mode));


	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}

	skb=wan_skb_dequeue(&tdm_api->wp_event_free_list);
	if (skb == NULL) {
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
		return;
	}

	p_tdmapi_event = (wp_api_event_t*)wan_skb_put(skb,sizeof(wp_api_event_t));

	memset(p_tdmapi_event, 0, sizeof(wp_api_event_t));
	
	p_tdmapi_event->wp_api_event_type = WP_API_EVENT_RING_TRIP_DETECT;
	p_tdmapi_event->wp_api_event_channel	= event->channel;
	p_tdmapi_event->wp_api_event_span = wp_tdmapi_get_span(card);

	
	if (event->ring_mode == WAN_EVENT_RING_TRIP_STOP){
		p_tdmapi_event->wp_api_event_ring_state =
				WP_API_EVENT_RING_TRIP_STOP;
	}else if (event->ring_mode == WAN_EVENT_RING_TRIP_PRESENT){
		p_tdmapi_event->wp_api_event_ring_state =
				WP_API_EVENT_RING_TRIP_PRESENT;
	}

#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif

	wan_skb_push_to_ctrl_event(skb);

	if (!wan_test_bit(0,&tdm_api->used)) {
		wan_skb_init(skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
		return;
	}

	wanpipe_tdm_api_handle_event(tdm_api,skb);

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
	wp_api_event_t	*p_tdmapi_event = NULL;

	DEBUG_TDMAPI("%s: Received RM RING DETECT Event at TDM_API (%d:%s)!\n",
			card->devname,
			event->channel,
			WAN_EVENT_RING_DECODE(event->ring_mode));


	tdm_api = wp_tdmapi_search(card, event->channel);
	if (tdm_api == NULL){
		return;
	}

	skb=wan_skb_dequeue(&tdm_api->wp_event_free_list);
	if (skb == NULL) {
		WP_AFT_CHAN_ERROR_STATS(tdm_api->cfg.stats,rx_events_dropped);
		return;
	}	

	p_tdmapi_event = (wp_api_event_t*)wan_skb_put(skb,sizeof(wp_api_event_t));

	memset(p_tdmapi_event, 0, sizeof(wp_api_event_t));

	p_tdmapi_event->wp_api_event_type = WP_API_EVENT_RING_DETECT;
	p_tdmapi_event->wp_api_event_channel	= event->channel;
	p_tdmapi_event->wp_api_event_span = wp_tdmapi_get_span(card);

	switch(event->ring_mode){
	case WAN_EVENT_RING_PRESENT:
		p_tdmapi_event->wp_api_event_ring_state =
					WP_API_EVENT_RING_PRESENT;
		break;
	case WAN_EVENT_RING_STOP:
		p_tdmapi_event->wp_api_event_ring_state =
					WP_API_EVENT_RING_STOP;
		break;
	}

#if 0
	rx_hdr->event_time_stamp = gettimeofday();
#endif

	wan_skb_push_to_ctrl_event(skb);

	if (!wan_test_bit(0,&tdm_api->used)) {
		wan_skb_init(skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(skb,0);
		wan_skb_queue_tail(&tdm_api->wp_event_free_list,skb);
		return;
	}

	wanpipe_tdm_api_handle_event(tdm_api,skb);

	return;
}


static int __store_tdm_api_pointer_in_card(sdla_t *card, wanpipe_tdm_api_dev_t *tdm_api, int fe_chan)
{

	if(fe_chan >= MAX_TDM_API_CHANNELS){
		DEBUG_EVENT("%s(): TDM API Error (TE1): Invalid Channel Number=%i (Span=%d)!\n",
				__FUNCTION__, fe_chan, tdm_api->tdm_span);
		return 1;
	}

	if(card->wp_tdmapi_hash[fe_chan] != NULL){
		DEBUG_EVENT("%s(): TDM API Error: device SPAN=%i CHAN=%i already in use!\n",
			__FUNCTION__, tdm_api->tdm_span, fe_chan);
		return 1;
	}

	card->wp_tdmapi_hash[fe_chan] = tdm_api;

#if 0
	DEBUG_TDMAPI("%s(): card->wp_tdmapi_hash[%d]==0x%p\n", __FUNCTION__, fe_chan, tdm_api);
#endif
	return 0;
}

static int store_tdm_api_pointer_in_card(sdla_t *card, wanpipe_tdm_api_dev_t *tdm_api)
{	
	int fe_chan=0, err;
#if 0
	DEBUG_TDMAPI("%s(): tdm_api ptr==0x%p, tdm_api->active_ch: 0x%X\n", __FUNCTION__, tdm_api, tdm_api->active_ch);
#endif

	/* Go through all timeslots belonging to this interface. VERY important for SPAN based API! */
	for(fe_chan = 0; fe_chan < NUM_OF_E1_CHANNELS; fe_chan++){

		/*	Note: 'tdm_api->active_ch' bitmap is 1-based so 'fe_chan' will start from 1.
			It is important because 'channo' in 'read_rbsbits(sdla_fe_t* fe, int channo, int mode)'
			is 1-based. */
		if (!wan_test_bit(fe_chan, &tdm_api->active_ch)) {
			/* timeslot does NOT belong to this interface */
			continue;
		}

		err=__store_tdm_api_pointer_in_card(card, tdm_api, fe_chan);
		if (err) {
			return err;
		}
	}

	return 0;
}


static int __remove_tdm_api_pointer_from_card(wanpipe_tdm_api_dev_t *tdm_api,int fe_chan)
{
	sdla_t	*card = NULL;

	WAN_ASSERT(tdm_api == NULL);
	card = (sdla_t*)tdm_api->card;

#if 1
	DEBUG_TDMAPI("%s(): tdm_api ptr==0x%p, fe_chan: %d\n", __FUNCTION__, tdm_api, fe_chan);
#endif

	if(card == NULL){
		DEBUG_EVENT("%s(): TDM API Error: Invalid 'card' pointer!\n", __FUNCTION__);
		return 1;
	}


	if(fe_chan >= MAX_TDM_API_CHANNELS){
		DEBUG_EVENT("%s(): TDM API Error (TE1): Invalid Channel Number=%i (Span=%d)!\n",
			__FUNCTION__, fe_chan, tdm_api->tdm_span);
		return 1;
	}

	if(card->wp_tdmapi_hash[fe_chan] == NULL){
		DEBUG_EVENT("%s: TDM API Warning: device SPAN=%i CHAN=%i was NOT initialized!\n",
			__FUNCTION__, tdm_api->tdm_span, fe_chan);
	}

	card->wp_tdmapi_hash[fe_chan] = NULL;

	return 0;
}


static int remove_tdm_api_pointer_from_card(wanpipe_tdm_api_dev_t *tdm_api)
{
	sdla_t	*card = NULL;
	int fe_chan=0, err;

	WAN_ASSERT(tdm_api == NULL);

	card = (sdla_t*)tdm_api->card;
	
	/* Go through all timeslots belonging to this interface. VERY important for SPAN based API! */
	for(fe_chan = 0; fe_chan < NUM_OF_E1_CHANNELS; fe_chan++){

		/*	Note: 'tdm_api->active_ch' bitmap is 1-based so 'fe_chan' will start from 1.
			It is important because 'channo' in 'read_rbsbits(sdla_fe_t* fe, int channo, int mode)'
			is 1-based. */
		if (!wan_test_bit(fe_chan, &tdm_api->active_ch)) {
			/* timeslot does NOT belong to this interface */
			continue;
		}

		err=__remove_tdm_api_pointer_from_card(tdm_api, fe_chan);
		if (err) {
			return err;
		}

	}

	return 0;
}



#else

int wanpipe_tdm_api_reg(wanpipe_tdm_api_dev_t *tdm_api)
{
	return -EINVAL;
}

int wanpipe_tdm_api_unreg(wanpipe_tdm_api_dev_t *tdm_api)
{
	return -EINVAL;
}

int wanpipe_tdm_api_kick(wanpipe_tdm_api_dev_t *tdm_api)
{
	return -EINVAL;
}

int wanpipe_tdm_api_rx_tx (wanpipe_tdm_api_dev_t *tdm_api, u8 *rx_data, u8 *tx_data, int len)
{
	return -EINVAL;
}


int wanpipe_tdm_api_span_rx (wanpipe_tdm_api_dev_t *tdm_api, netskb_t *skb)
{
	return -EINVAL;
}

#endif /* #if defined(BUILD_TDMV_API) */
