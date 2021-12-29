/***************************************************************************
 * sdla_bri_tdmv.c	WANPIPE(tm) Multiprotocol WAN Link Driver. 
 *					AFT BRI support.
 *
 * Author: 	David Rokhvarg   <davidr@sangoma.com>
 *
 * Copyright:	(c) 1984-2007 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * ============================================================================
 * June 5, 2007	David Rokhvarg	Initial version.
 * Sep 06, 2008	Moises Silva    DAHDI support.
 ******************************************************************************
 */

/*******************************************************************************
**			   INCLUDE FILES
*******************************************************************************/

# include "wanpipe_includes.h"
# include "wanpipe_defines.h"
# include "wanpipe_debug.h"
# include "wanpipe_abstr.h"
# include "wanpipe_common.h"
# include "wanpipe_events.h"
# include "wanpipe.h"
# include "wanpipe_events.h"
# include "if_wanpipe_common.h"	/* for 'wanpipe_common_t' used in 'aft_core.h'*/
# include "sdla_bri.h"
# include "zapcompat.h"
# include "sdla_tdmv.h"


/*******************************************************************************
**			  DEFINES AND MACROS
*******************************************************************************/

#define DEBUG_TDMV_BRI	if(0)DEBUG_EVENT

#define REG_SHADOW
#define REG_WRITE_SHADOW
#undef  PULSE_DIALING

/* flags bits */
#define WP_TDMV_REGISTER	1	/*0x01*/
#define WP_TDMV_RUNNING		2	/*0x02*/
#define WP_TDMV_UP		3	/*0x04*/

#define IS_TDMV_RUNNING(wr)		wan_test_bit(WP_TDMV_RUNNING, &(wr)->flags)
#define IS_TDMV_UP(wr)			wan_test_bit(WP_TDMV_UP, &(wr)->flags)
#define IS_TDMV_UP_RUNNING(wr)	(IS_TDMV_UP(wr) && IS_TDMV_RUNNING(wr))

/*******************************************************************************
**			STRUCTURES AND TYPEDEFS
*******************************************************************************/
typedef struct {
	int	ready;
	int	offhook;
	int	lastpol;
	int	polarity;
	int	polaritydebounce;
	int	battery;
	int	battdebounce;
	int	ringdebounce;
	int	nobatttimer;
	int	wasringing;
	
	int				echotune;	/* echo tune */
	struct wan_rm_echo_coefs	echoregs;	/* echo tune */
} tdmv_fxo_t;

typedef struct {
	int	ready;
	int	lasttxhook;
	int	lasttxhook_update;
	int	lastrxhook;
	int	oldrxhook;
	int	debouncehook;
	int	debounce;
	int	palarms;
	int	ohttimer;
} tdmv_fxs_t;

typedef struct wp_tdmv_bri_ {
	void		*card;
	char		*devname;
	int		num;
	int		flags;
	wan_spinlock_t	lock;
	wan_spinlock_t	tx_rx_lock;
	union {
		tdmv_fxo_t	fxo;
		tdmv_fxs_t	fxs;
	} mod[MAX_BRI_LINES];

	int		spanno;
	struct zt_span	span;
#ifdef DAHDI_ISSUES
	struct zt_chan *chans_ptrs[MAX_BRI_LINES];
#endif
	struct zt_chan	chans[MAX_BRI_LINES];
	unsigned long	reg_module_map;	/* Registered modules */

	u32		timeslot_map;

	/* Global configuration */

	u32		intcount;
	int		pollcount;
	unsigned char	ec_chunk1[31][ZT_CHUNKSIZE];
	unsigned char	ec_chunk2[31][ZT_CHUNKSIZE];
	int		usecount;
	u16		max_timeslots;		/* up to MAX_BRI_LINES */
	int		max_rxtx_len;
	int		channelized;
	unsigned long	echo_off_map;
	int		rxsig_state[MAX_BRI_LINES];
	int		txsig_state[MAX_BRI_LINES];	/* not used */
	
	u_int8_t	tonesupport;
	unsigned int	toneactive;
	unsigned int	tonemask;
	unsigned int	tonemutemask;

	/* BRI D-chan */
	unsigned int	dchan;
	netdevice_t	*dchan_dev;

} wp_tdmv_bri_t;

/*******************************************************************************
**			   GLOBAL VARIABLES
*******************************************************************************/
static int	wp_remora_no = 0;
extern WAN_LIST_HEAD(, wan_tdmv_) wan_tdmv_head;
//static int battdebounce = DEFAULT_BATT_DEBOUNCE;
//static int battthresh = DEFAULT_BATT_THRESH;

/*******************************************************************************
**			  FUNCTION PROTOTYPES
*******************************************************************************/
static int wp_tdmv_bri_check_mtu(void* pcard, unsigned long timeslot_map, int *mtu);
static int wp_tdmv_bri_create(void* pcard, wan_tdmv_conf_t *tdmv_conf);
static int wp_tdmv_bri_remove(void* pcard);

static int wp_tdmv_bri_reg(
		void			*pcard, 
		wan_tdmv_if_conf_t	*tdmv_conf,
		unsigned int		active_ch,
		u8			ec_enable,
		netdevice_t 		*dev);
static int wp_tdmv_bri_unreg(void* pcard, unsigned long ts_map);
static int wp_tdmv_bri_software_init(wan_tdmv_t *wan_tdmv);
static int wp_tdmv_bri_state(void* pcard, int state);
static int wp_tdmv_bri_running(void* pcard);
static int wp_tdmv_bri_is_rbsbits(wan_tdmv_t *wan_tdmv);
static int wp_tdmv_bri_rx_tx_span(void *pcard);
static int wp_tdmv_bri_rx_chan(wan_tdmv_t*, int,unsigned char*,unsigned char*); 
static int wp_tdmv_bri_ec_span(void *pcard);

static void wp_tdmv_bri_tone (void* card_id, wan_event_t *event);

extern int wp_init_proslic(sdla_fe_t *fe, int mod_no, int fast, int sane);
extern int wp_init_voicedaa(sdla_fe_t *fe, int mod_no, int fast, int sane);

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN) && defined(ZT_DCHAN_TX)
static int wp_tdmv_tx_dchan(struct zt_chan *chan, int len);
#endif
static int wp_tdmv_rx_dchan(wan_tdmv_t*, int, unsigned char*, unsigned int); 

static int wp_tdmv_span_buf_rotate(void *pcard, u32, unsigned long);

static void wp_tdmv_report_alarms(void* pcard, unsigned long te_alarm);

/*******************************************************************************
**			  FUNCTION DEFINITIONS
*******************************************************************************/

static int
#if defined(__FreeBSD__) || defined(__OpenBSD__)
wp_bri_zap_ioctl(struct zt_chan *chan, unsigned int cmd, caddr_t data)
#else
wp_bri_zap_ioctl(struct zt_chan *chan, unsigned int cmd, unsigned long data)
#endif
{
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN) && defined(ZT_DCHAN_TX)
	wp_tdmv_bri_t	*wp = NULL;
#endif
	int		err = -ENOTTY;

	DEBUG_TDMV_BRI("%s(): line: %d\n", __FUNCTION__, __LINE__);

	switch(cmd) 
	{
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN) && defined(ZT_DCHAN_TX)
	case ZT_DCHAN_TX:

		DEBUG_TDMV_BRI("chan: 0x%p\n", chan);

		if(chan){
			DEBUG_TDMV_BRI("chan->pvt: 0x%p\n", chan->pvt);
		}

		WAN_ASSERT(chan == NULL || chan->pvt == NULL);
		wp = chan->pvt;	

		if (wp->dchan_dev && wp->dchan_dev->hard_start_xmit){
			wp_tdmv_tx_dchan(chan, (int)data);
			err=0;
		}else{
			DEBUG_TDMV_BRI("%s(): line: %d\n", __FUNCTION__, __LINE__);

			DEBUG_TDMV_BRI("wp->dchan_dev: 0x%p\n",	wp->dchan_dev);
			if(wp->dchan_dev){
				DEBUG_TDMV_BRI("wp->dchan_dev->hard_start_xmit: 0x%p\n",
					wp->dchan_dev->hard_start_xmit);
			}
			err=-EOPNOTSUPP;
		}
		break;

#endif

	default:
		DEBUG_EVENT("%s(): uknown cmd!\n", __FUNCTION__);
		err = -ENOTTY;
		break;
	}
	return err;
}


static int wp_bri_zap_open(struct zt_chan *chan)
{
	wp_tdmv_bri_t	*wr = NULL;
	
	BRI_FUNC();

	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wr = chan->pvt;
	wr->usecount++;
	wan_set_bit(WP_TDMV_RUNNING, &wr->flags);
	DEBUG_EVENT("%s: Open (usecount=%d, channo=%d, chanpos=%d)...\n", 
				wr->devname,
				wr->usecount,
				chan->channo,
				chan->chanpos);
	return 0;
}

static int wp_bri_zap_close(struct zt_chan *chan)
{
	sdla_t		*card = NULL;	
	wp_tdmv_bri_t*	wr = NULL;
	sdla_fe_t	*fe = NULL;
	
	BRI_FUNC();

	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wr	= chan->pvt;
	card	= wr->card;
	fe	= &card->fe;
	wr->usecount--;
	wan_clear_bit(WP_TDMV_RUNNING, &wr->flags);

	return 0;
}

static int wp_bri_zap_watchdog(struct zt_span *span, int event)
{
	BRI_FUNC();

#if 0
	printk("TDM: Restarting DMA\n");
	wctdm_restart_dma(span->pvt);
#endif
	return 0;
}

/******************************************************************************
** wp_remora_zap_hwec() - 
**
**	OK
*/
static int wp_bri_zap_hwec(struct zt_chan *chan, int enable)
{
	wp_tdmv_bri_t	*wr = NULL;
	sdla_t		*card = NULL;
	int		fe_chan = chan->chanpos;
	int		err = -ENODEV;
	sdla_fe_t	*fe = NULL;

	BRI_FUNC();	

	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wr = chan->pvt;
	WAN_ASSERT2(wr->card == NULL, -ENODEV);
	card	= wr->card;
	fe	= &card->fe;

	if (card->wandev.ec_enable){
		DEBUG_EVENT("[TDMV_BRI]: %s: %s(): channel %d\n",
			wr->devname, __FUNCTION__, fe_chan);

		if(fe_chan == 1 || fe_chan == 2){
			err = card->wandev.ec_enable(card, enable, fe_chan);
		}else{
			DEBUG_EVENT("[TDMV_BRI]: %s: %s(): Warning: invalid fe_channel %d!!\n",
				wr->devname, __FUNCTION__, fe_chan);
			err = 0;
		}
	}else{
		DEBUG_EVENT("[TDMV_BRI]: %s: %s(): card->wandev.ec_enable == NULL!!!!!!\n",
			wr->devname, __FUNCTION__);
	}
	return err;
}


/******************************************************************************
** wp_tdmv_rbsbits_poll() -
**
**	DONE
*/
static int wp_tdmv_rbsbits_poll(wan_tdmv_t *wan_tdmv, void *card1)
{
	wp_tdmv_bri_t	*wp = NULL;

	BRI_FUNC();	
	
	WAN_ASSERT(wan_tdmv->sc == NULL);
	wp = wan_tdmv->sc;

	return 0;
}


/******************************************************************************
** wp_tdmv_bri_init() - 
**
**	OK
*/
int wp_tdmv_bri_init(wan_tdmv_iface_t *iface)
{
	BRI_FUNC();	

	WAN_ASSERT(iface == NULL);
	
	memset(iface, 0, sizeof(wan_tdmv_iface_t));
	iface->check_mtu	= wp_tdmv_bri_check_mtu;
	iface->create		= wp_tdmv_bri_create;
	iface->remove		= wp_tdmv_bri_remove;
	iface->reg		= wp_tdmv_bri_reg;
	iface->unreg		= wp_tdmv_bri_unreg;
	iface->software_init	= wp_tdmv_bri_software_init;
	iface->state		= wp_tdmv_bri_state;
	iface->running		= wp_tdmv_bri_running;
	iface->is_rbsbits	= wp_tdmv_bri_is_rbsbits;
	iface->rx_tx_span	= wp_tdmv_bri_rx_tx_span;
	iface->rx_chan		= wp_tdmv_bri_rx_chan;
	iface->ec_span		= wp_tdmv_bri_ec_span;  

	iface->rbsbits_poll	= wp_tdmv_rbsbits_poll;	//?????

	iface->rx_dchan		= wp_tdmv_rx_dchan;

	iface->buf_rotate	= wp_tdmv_span_buf_rotate;
	return 0;
}


static int wp_tdmv_bri_software_init(wan_tdmv_t *wan_tdmv)
{
	sdla_t		*card = NULL;
	sdla_fe_t	*fe = NULL;
	wp_tdmv_bri_t	*wr = wan_tdmv->sc;
	int		x = 0, num = 0;
	
	BRI_FUNC();	

	WAN_ASSERT(wr == NULL);
	WAN_ASSERT(wr->card == NULL);
	card = wr->card;
	fe = &card->fe;

	if (wan_test_bit(WP_TDMV_REGISTER, &wr->flags)){
	
		WP_DELAY(1000);	
		DEBUG_EVENT(
		"%s: Wanpipe device is already registered to Zaptel span # %d!\n",
					wr->devname, wr->span.spanno);
		return 0;
	}

	/* Zapata stuff */
	sprintf(wr->span.name, "WPBRI/%d", wr->num);
	sprintf(wr->span.desc, "wrtdm Board %d", wr->num + 1);
	switch(fe->fe_cfg.tdmv_law){
	case WAN_TDMV_ALAW:
		DEBUG_EVENT(
		"%s: ALAW override parameter detected. Device will be operating in ALAW\n",
					wr->devname);
		wr->span.deflaw = ZT_LAW_ALAW;
		break;
	case WAN_TDMV_MULAW:
		wr->span.deflaw = ZT_LAW_MULAW;
		break;
	}

wr->span.deflaw = ZT_LAW_ALAW;//FIXME: hardcoded

	wr->tonesupport = card->u.aft.tdmv_hw_tone;
	
	for (x = 0; x < MAX_BRI_TIMESLOTS; x++) {
			
		sprintf(wr->chans[x].name, "WPBRI/%d/%d", wr->num, x);

		wr->chans[x].sigcap =  ZT_SIG_EM | ZT_SIG_CLEAR | ZT_SIG_FXSLS |
		                          ZT_SIG_FXSGS | ZT_SIG_FXSKS | ZT_SIG_FXOLS |
		                          ZT_SIG_FXOGS | ZT_SIG_FXOKS | ZT_SIG_CAS |
		                          ZT_SIG_SF;
		switch(x)
		{
		case 0:
		case 1:
			DEBUG_EVENT("%s: Port %d Configure B-chan %d for voice (%s, type %s)!\n", 
					wr->devname, 
					WAN_FE_LINENO(fe) + 1,
					x + 1,
					wr->chans[x].name,
					WP_BRI_DECODE_MOD_TYPE(fe->bri_param.mod[WAN_FE_LINENO(fe)].type));
			break;

		case 2:
			DEBUG_EVENT("%s: Port %d Configure BRI D-chan (%s, type %s)!\n", 
					wr->devname, 
					WAN_FE_LINENO(fe) + 1,
					wr->chans[x].name,
					WP_BRI_DECODE_MOD_TYPE(fe->bri_param.mod[WAN_FE_LINENO(fe)].type));/* fe->bri_param.mod[2], there is no [2] */
			break;
		}/* switch() */

		wr->chans[x].chanpos = x+1;
		wr->chans[x].pvt = wr;
		num++;

		//wr->rxsig_state[x] = ZT_RXSIG_INITIAL;
	}/* for() */

	wr->span.pvt		= wr;
#ifdef DAHDI_ISSUES
	wr->span.chans		= wr->chans_ptrs;
#else
	wr->span.chans		= wr->chans;
#endif
	wr->span.channels	= MAX_BRI_TIMESLOTS;/* this is the number of b-chans (2) and the d-chan on one BRI line. */;
	wr->span.linecompat	= ZT_CONFIG_AMI | ZT_CONFIG_CCS; /* <--- this is really BS */

	wr->span.open		= wp_bri_zap_open;
	wr->span.close		= wp_bri_zap_close;

	//wr->span.flags	= ZT_FLAG_RBS;

	wr->span.ioctl		= wp_bri_zap_ioctl;
	wr->span.watchdog	= wp_bri_zap_watchdog;
	/* Set this pointer only if card has hw echo canceller module */
	if (card->wandev.ec_dev){
		wr->span.echocan = wp_bri_zap_hwec;
	}
#if defined(__LINUX__)
	init_waitqueue_head(&wr->span.maintq);
#endif
	
	WP_DELAY(1000);	

	if (zt_register(&wr->span, 0)) {
	
		BRI_FUNC();	
		DEBUG_EVENT("%s: Unable to register span with zaptel\n",
				wr->devname);
	
		WP_DELAY(1000);	
		return -EINVAL;
	}

	if (wr->span.spanno != wr->spanno +1){
		DEBUG_EVENT("\n");
		DEBUG_EVENT("WARNING: Span number %d is already used by another device!\n",
						wr->spanno + 1);
		DEBUG_EVENT("         Possible cause: Another TDM driver already loaded!\n");
		DEBUG_EVENT("         Solution:       Unload wanpipe and check currently\n");
		DEBUG_EVENT("                         used spans in /proc/zaptel directory.\n");
		DEBUG_EVENT("         Reconfiguring device %s to new span number # %d\n",
						wr->devname,wr->span.spanno);
		DEBUG_EVENT("\n");
		wr->spanno = wr->span.spanno-1;
	}else{
		DEBUG_EVENT("%s: Wanpipe device is registered to Zaptel span # %d!\n", 
					wr->devname, wr->span.spanno);
	}
	wp_tdmv_bri_check_mtu(card, wr->reg_module_map, &wr->max_rxtx_len);
	wan_set_bit(WP_TDMV_REGISTER, &wr->flags);

	/* Initialize Callback event function pointers */	
	if (wr->tonesupport == WANOPT_YES){
		DEBUG_EVENT("%s:Updating event callback tone\n", wr->devname);
		card->wandev.event_callback.tone = wp_tdmv_bri_tone;
	}

	WP_DELAY(1000);	

	wr->span.alarms = ZT_ALARM_NONE;//FIXME: report real line state, also report alarms on line connect/disconnect.
	zt_alarm_notify(&wr->span);

	return 0;
}

/******************************************************************************
** wp_tdmv_release() - 
**
**	OK
*/
static void wp_tdmv_release(wp_tdmv_bri_t *wr)
{
	BRI_FUNC();	

	WAN_ASSERT1(wr == NULL);
	if (wan_test_bit(WP_TDMV_REGISTER, &wr->flags)){
		DEBUG_EVENT("%s: Unregister WAN FXS/FXO device from Zaptel!\n",
				wr->devname);
		wan_clear_bit(WP_TDMV_REGISTER, &wr->flags);
		zt_unregister(&wr->span);
		wan_clear_bit(WP_TDMV_REGISTER, &wr->flags);
	}
	wan_free(wr);
	return;
}


static wp_tdmv_bri_t *wan_remora_search(sdla_t * card)
{
	BRI_FUNC();	

	return NULL;
}

/******************************************************************************
** wp_tdmv_bri_check_mtu() - 
**
**	OK
*/
static int wp_tdmv_bri_check_mtu(void* pcard, unsigned long timeslot_map, int *mtu)
{
	BRI_FUNC();	

	*mtu = ZT_CHUNKSIZE;
	return 0;

}

/******************************************************************************
** wp_tdmv_bri_create() - 
*tdmv_*
**	OK
*/
static int wp_tdmv_bri_create(void* pcard, wan_tdmv_conf_t *tdmv_conf)
{
	sdla_t		*card = (sdla_t*)pcard;
	wp_tdmv_bri_t	*wr = NULL;
	wan_tdmv_t	*tmp = NULL;
#ifdef DAHDI_ISSUES
	int i;
#endif
	
	BRI_FUNC();	

	WAN_ASSERT(card == NULL);
	WAN_ASSERT(tdmv_conf->span_no == 0);

	wr = wan_remora_search(card);
	if (wr){
		DEBUG_EVENT("%s: AFT remora card already configured!\n",
					card->devname);
		return -EINVAL;
	}

	memset(&card->wan_tdmv, 0x0, sizeof(wan_tdmv_t));
	/* We are forcing to register wanpipe devices at the same sequence
	 * that it defines in /etc/zaptel.conf */
   	WAN_LIST_FOREACH(tmp, &wan_tdmv_head, next){
		if (tmp->spanno == tdmv_conf->span_no){
			DEBUG_EVENT("%s: Registering device with an incorrect span number!\n",
					card->devname);
			DEBUG_EVENT("%s: Another wanpipe device already configured to span #%d!\n",
					card->devname, tdmv_conf->span_no);
			return -EINVAL;
		}
		if (!WAN_LIST_NEXT(tmp, next)){
			break;
		}
	}

	memset(&card->wan_tdmv, 0x0, sizeof(wan_tdmv_t));
	card->wan_tdmv.max_timeslots	= card->fe.bri_param.max_fe_channels;
	card->wan_tdmv.spanno		= tdmv_conf->span_no;
	card->wandev.te_report_alarms	= wp_tdmv_report_alarms;	

	wr = wan_malloc(sizeof(wp_tdmv_bri_t));
	if (wr == NULL){
		return -ENOMEM;
	}
	memset(wr, 0x0, sizeof(wp_tdmv_bri_t));
	card->wan_tdmv.sc	= wr;
	wr->spanno		= tdmv_conf->span_no-1;
	wr->num			= wp_remora_no++;
	wr->card		= card;
	wr->devname		= card->devname;
	wr->max_timeslots	= card->fe.bri_param.max_fe_channels;
	wr->max_rxtx_len	= 0;
	wan_spin_lock_init(&wr->lock, "wan_britdmv_lock");
	wan_spin_lock_init(&wr->tx_rx_lock, "wan_britdmv_txrx_lock");
#ifdef DAHDI_ISSUES
	for (i = 0; i < sizeof(wr->chans)/sizeof(wr->chans[0]); i++) {
		wr->chans_ptrs[i] = &wr->chans[i];
	}
#endif

	/* BRI signalling is selected with hw HDLC (dchan is not 0) */
	wr->dchan = 3;/* MUST be 3! */

	if (tmp){
		WAN_LIST_INSERT_AFTER(tmp, &card->wan_tdmv, next);
	}else{
		WAN_LIST_INSERT_HEAD(&wan_tdmv_head, &card->wan_tdmv, next);
	}
	return 0;
}


/******************************************************************************
** wp_tdmv_bri_reg()  - calculate 'channo' based on 'activ_ch' for the
**			interface.
**
** 
** Returns: 	0-31	- Return TDM Voice channel number.
**		-EINVAL - otherwise
**	OK
*/
static int wp_tdmv_bri_reg(
		void			*pcard, 
		wan_tdmv_if_conf_t	*tdmv_conf,
		unsigned int		active_ch,
		u8			ec_enable,
		netdevice_t 		*dev)
{
	sdla_t			*card = (sdla_t*)pcard;
	sdla_fe_t		*fe = &card->fe;
	wan_tdmv_t		*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_bri_t		*wr = NULL;
	int			i, channo = 0;
	
	BRI_FUNC();	

	WAN_ASSERT(wan_tdmv->sc == NULL);
	wr = wan_tdmv->sc;


	DEBUG_BRI("%s(): FE line no: %d, active_ch: 0x%X\n", 
		__FUNCTION__, WAN_FE_LINENO(fe), active_ch);
	
	if(WAN_FE_LINENO(fe) >= MAX_BRI_LINES){
		DEBUG_EVENT(
		"%s: %s(): Error: invalid FE line number (%d)!\n",
			card->devname, __FUNCTION__, WAN_FE_LINENO(fe));
		return -EINVAL;
	}

	if (wan_test_bit(WP_TDMV_REGISTER, &wr->flags)){
		DEBUG_EVENT(
		"%s: Error: Master device has already been configured!\n",
				card->devname);
		return -EINVAL;
	}

	DEBUG_BRI("%s(): wr->max_timeslots: %d\n", __FUNCTION__, wr->max_timeslots);

	/* The Zaptel Channel Number for BRI must be adjusted based on 
	   the module number to start from 0 */
	active_ch = active_ch>>(WAN_FE_LINENO(fe)*2);

	for(i = 0; i < wr->max_timeslots; i++){
		if (wan_test_bit(i, &active_ch)){
			if (tdmv_conf->tdmv_echo_off){
				wan_set_bit(i, &wr->echo_off_map);
			}
			channo = i;
			break;
		}
	}

	DEBUG_BRI("1  %s(): channo: %d, wr->timeslot_map: 0x%X\n", 
		__FUNCTION__, channo, wr->timeslot_map);

	if(channo > 2){
		DEBUG_EVENT(
		"%s: Error: TDMV iface %s failed to configure for channo %d! Must be between 0 and 2 (including).\n",
					card->devname,
					wan_netif_name(dev),
					channo);
		return -EINVAL;
	}

	/* for BRI only bits 0 and 1 can be set! */
	wan_set_bit(channo, &wr->timeslot_map);
	
	if (i == wr->max_timeslots){
		DEBUG_EVENT("%s: Error: TDMV iface %s failed to configure for 0x%08X timeslots!\n",
					card->devname,
					wan_netif_name(dev),
					active_ch);
		return -EINVAL;
	}

   	DEBUG_EVENT("%s: Registering TDMV to %s module %d!\n",
			card->devname,
			WP_BRI_DECODE_MOD_TYPE(fe->bri_param.mod[WAN_FE_LINENO(fe)].type),
			WAN_FE_LINENO(fe) + 1);

	wan_set_bit(channo, &wr->reg_module_map);

	if (tdmv_conf->tdmv_echo_off){
		DEBUG_EVENT("%s:    TDMV Echo Ctrl:Off\n",
				wr->devname);
	}
	memset(wr->chans[channo].sreadchunk, WAN_TDMV_IDLE_FLAG, ZT_CHUNKSIZE);
	memset(wr->chans[channo].swritechunk, WAN_TDMV_IDLE_FLAG, ZT_CHUNKSIZE);
	wr->chans[channo].readchunk = wr->chans[channo].sreadchunk;
	wr->chans[channo].writechunk = wr->chans[channo].swritechunk;
	wr->channelized = WAN_TRUE;

	wp_tdmv_bri_check_mtu(card, active_ch, &wr->max_rxtx_len);

	DEBUG_BRI("%s(): card->u.aft.tdmv_dchan: %d, channo: %d, wr->dchan: %d\n", 
		__FUNCTION__, card->u.aft.tdmv_dchan, channo, wr->dchan);
	
	if(wr->dchan != 3){
		DEBUG_EVENT("%s:%s: Error: 'dchan' (%d) not equal 3!\n",
				card->devname, wan_netif_name(dev), wr->dchan);
		return -EINVAL;
	}

	if((channo + 1) == wr->dchan){ /* 3 */
		DEBUG_BRI("%s(): registering BRI D-chan 'dev' pointer\n", __FUNCTION__);
		wr->dchan_dev = dev;
		card->u.aft.tdmv_dchan = BRI_DCHAN_LOGIC_CHAN;
	}
	
	return channo;
}

/******************************************************************************
** wp_tdmv_unreg() - 
**
**	OK
*/
static int wp_tdmv_bri_unreg(void* pcard, unsigned long ts_map)
{
	sdla_t			*card = (sdla_t*)pcard;
	sdla_fe_t		*fe = &card->fe;
	wan_tdmv_t		*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_bri_t	*wr = NULL;
	int			channo = 0;

	BRI_FUNC();	

	WAN_ASSERT(wan_tdmv->sc == NULL);
	wr = wan_tdmv->sc;

	for(channo = 0; channo < wr->max_timeslots; channo++){
		if (wan_test_bit(channo, &wr->reg_module_map)){
			DEBUG_EVENT(
			"%s: Unregistering TDMV %s iface from module %d!\n",
				card->devname,
				WP_BRI_DECODE_MOD_TYPE(fe->bri_param.mod[0].type),
				channo+1);
			wan_clear_bit(channo, &wr->reg_module_map);
			wan_clear_bit(channo, &wr->echo_off_map);
			memset(wr->chans[channo].sreadchunk, 
					WAN_TDMV_IDLE_FLAG, 
					ZT_CHUNKSIZE);
			memset(wr->chans[channo].swritechunk, 
					WAN_TDMV_IDLE_FLAG, 
					ZT_CHUNKSIZE);
			wr->chans[channo].readchunk = 
					wr->chans[channo].sreadchunk;
			wr->chans[channo].writechunk = 
					wr->chans[channo].swritechunk;
		}
	}
	return 0;
}


/******************************************************************************
** wp_tdmv_remove() - 
**
**	OK
*/
static int wp_tdmv_bri_remove(void* pcard)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_bri_t	*wr = NULL;

	BRI_FUNC();	

	if (!card->wan_tdmv.sc){
		return 0;
	}
	
	wr = wan_tdmv->sc;
	/* Release span, possibly delayed */
	if (wr && wr->reg_module_map){
		DEBUG_EVENT(
		"%s: Some interfaces are not unregistered (%08lX)!\n",
				card->devname,
				wr->reg_module_map);
		return -EINVAL;
	}
	if (wr && wr->usecount){
		DEBUG_EVENT("%s: ERROR: Wanpipe is still used by Asterisk!\n",
				card->devname);
		return -EINVAL;
	}

	if (wr){
		wan_clear_bit(WP_TDMV_RUNNING, &wr->flags);
		wan_clear_bit(WP_TDMV_UP, &wr->flags);
		wan_tdmv->sc = NULL;
		card->wandev.te_report_alarms = NULL;	
		WAN_LIST_REMOVE(wan_tdmv, next);
		wp_tdmv_release(wr);
	}else{
		wan_tdmv->sc = NULL;
	}
	return 0;
}

static int wp_tdmv_span_buf_rotate(void *pcard, u32 buf_sz, unsigned long mask)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_bri_t	*wp = NULL;
	int x;
	unsigned int rx_offset, tx_offset;
	void *ptr;
	wan_smp_flag_t flag;

	WAN_ASSERT(wan_tdmv->sc == NULL);
	wp = wan_tdmv->sc; 

	rx_offset = buf_sz * card->u.aft.tdm_rx_dma_toggle;
	tx_offset = buf_sz * card->u.aft.tdm_tx_dma_toggle;

#define BRI_NUM_OF_BCHANNELS 2
	for (x = 0; x < BRI_NUM_OF_BCHANNELS; x ++) {
	/*for (x = 0; x < 32; x ++) {*/
		if (wan_test_bit(x,&wp->timeslot_map)) {

			wan_spin_lock(&wp->chans[x].lock,&flag);
				
                        ptr=(void*)((((unsigned long)wp->chans[x].readchunk) & ~(mask)) + rx_offset);
			wp->chans[x].readchunk = ptr;
                        ptr=(void*)((((unsigned long)wp->chans[x].writechunk) & ~(mask)) + tx_offset);
			wp->chans[x].writechunk = ptr;
			
			wan_spin_unlock(&wp->chans[x].lock,&flag);

#if defined(__LINUX__)
      			prefetch(wp->chans[x].readchunk);
      			prefetch(wp->chans[x].writechunk);
#endif

			if (card->wandev.rtp_len && card->wandev.rtp_tap) {
				card->wandev.rtp_tap(card,x,
						     wp->chans[x].readchunk,
						     wp->chans[x].writechunk,
						     ZT_CHUNKSIZE);
			}
		}
	}          

	return 0;
}

static int wp_tdmv_bri_state(void* pcard, int state)
{
	sdla_t			*card = (sdla_t*)pcard;
	wan_tdmv_t		*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_bri_t	*wr = NULL;

	BRI_FUNC();	

	WAN_ASSERT(wan_tdmv->sc == NULL);
	wr = (wp_tdmv_bri_t*)wan_tdmv->sc;
	
	switch(state){
	case WAN_CONNECTED:
		DEBUG_TDMV("%s: TDMV Remora state is CONNECTED!\n",
					wr->devname);
		wan_set_bit(WP_TDMV_UP, &wr->flags);
		break;

	case WAN_DISCONNECTED:
		DEBUG_TDMV("%s: TDMV Remora state is DISCONNECTED!\n",
					wr->devname);
		wan_clear_bit(WP_TDMV_UP, &wr->flags);
		break;
	}
	return 0;
}

/******************************************************************************
** wp_tdmv_running() - 
**
**	OK
*/
static int wp_tdmv_bri_running(void* pcard)
{
	sdla_t			*card = (sdla_t*)pcard;
	wan_tdmv_t		*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_bri_t	*wr = NULL;

	BRI_FUNC();	

	wr = wan_tdmv->sc;
	if (wr && wr->usecount){
		DEBUG_EVENT("%s: WARNING: Wanpipe is still used by Asterisk!\n",
				card->devname);
		return -EINVAL;
	}
	return 0;
}

/******************************************************************************
** wp_tdmv_bri_is_rbsbits() - 
**
**	OK
*/
static int wp_tdmv_bri_is_rbsbits(wan_tdmv_t *wan_tdmv)
{
	return 0;
}

/******************************************************************************
** wp_tdmv_rx_chan() - 
**
**	OK
*/
static int wp_tdmv_bri_rx_chan(wan_tdmv_t *wan_tdmv, int channo, 
			unsigned char *rxbuf,
			unsigned char *txbuf)
{
	wp_tdmv_bri_t	*wr = wan_tdmv->sc;
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
	wan_tdmv_rxtx_pwr_t	*pwr_rxtx = NULL;
#endif
	
	//DEBUG_TDMV_BRI("channo: %d\n", channo);

	//return 0;

	WAN_ASSERT2(wr == NULL, -EINVAL);
	WAN_ASSERT2(channo < 0, -EINVAL);
	WAN_ASSERT2(channo > 31, -EINVAL);

	if (!IS_TDMV_UP(wr)){
		return -EINVAL;
	}

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
	pwr_rxtx = &wan_tdmv->chan_pwr[channo];
#endif

#if 0
DEBUG_EVENT("Module %d: RX: %02X %02X %02X %02X %02X %02X %02X %02X\n",
		channo,
					rxbuf[0],
					rxbuf[1],
					rxbuf[2],
					rxbuf[3],
					rxbuf[4],
					rxbuf[5],
					rxbuf[6],
					rxbuf[7]
					);
#endif
	wr->chans[channo].readchunk = rxbuf;	
	wr->chans[channo].writechunk = txbuf;	

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
	wp_tdmv_echo_check(wan_tdmv, &wr->chans[channo], channo);
#endif		

	if (!wan_test_bit(channo, &wr->echo_off_map)){
/*Echo spike starts at 25bytes*/
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
		if(pwr_rxtx->current_state != ECHO_ABSENT){
#endif
#if 0
/* Echo spike starts at 16 bytes */	

			zt_ec_chunk(
				&wr->chans[channo], 
				wr->chans[channo].readchunk, 
				wr->chans[channo].writechunk);
#endif

#if 1			
/*Echo spike starts at 9 bytes*/
			zt_ec_chunk(
				&wr->chans[channo], 
				wr->chans[channo].readchunk, 
				wr->ec_chunk1[channo]);
			memcpy(
				wr->ec_chunk1[channo],
				wr->chans[channo].writechunk,
				ZT_CHUNKSIZE);
#endif

#if 0			
/*Echo spike starts at bytes*/
			zt_ec_chunk(
				&wr->chans[channo], 
				wr->chans[channo].readchunk, 
				wr->ec_chunk1[channo]);
			memcpy(
				wr->ec_chunk1[channo],
				wr->ec_chunk2[channo],
				ZT_CHUNKSIZE);

			memcpy(
				wr->ec_chunk2[channo],
				wr->chans[channo].writechunk,
				ZT_CHUNKSIZE);
#endif

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
		} /*if(pwr_rxtx->current_state != ECHO_ABSENT) */
#endif
	} /* if (!wan_test_bit(channo, &wr->echo_off_map)) */

	return 0;
}

static int wp_tdmv_bri_rx_tx_span(void *pcard)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_bri_t	*wr = NULL;
	
	/*BRI_FUNC();*/

	WAN_ASSERT(wan_tdmv->sc == NULL);
	wr = wan_tdmv->sc;

	wr->intcount++;

	zt_receive(&wr->span);
	zt_transmit(&wr->span);

	return 0;
}

static int wp_tdmv_bri_ec_span(void *pcard)
{
	sdla_t          *card = (sdla_t*)pcard;
	wan_tdmv_t      *wan_tdmv = &card->wan_tdmv;
	wp_tdmv_bri_t	*wr = NULL;

	//BRI_FUNC();	

	WAN_ASSERT(wan_tdmv->sc == NULL);
    
	wr = wan_tdmv->sc;

	zt_ec_span(&wr->span);

	return 0;

}

static void wp_tdmv_bri_tone (void* card_id, wan_event_t *event)
{
	sdla_t	*card = (sdla_t*)card_id;
        wan_tdmv_t      *wan_tdmv = &card->wan_tdmv;
        wp_tdmv_bri_t	*wr = NULL;

	BRI_FUNC();	

        WAN_ASSERT1(wan_tdmv->sc == NULL);
        wr = wan_tdmv->sc;

	if (event->type == WAN_EVENT_RM_DTMF){
		DEBUG_EVENT("%s: Received DTMF Event at TDM BRI (%d:%c)!\n",
			card->devname,
			event->channel,
			event->digit);	
	}else{
		DEBUG_EVENT("%s: Received Tone Event at TDM BRI (%d:%c:%s:%s)!\n",
			card->devname,
			event->channel,
			event->digit,
			(event->tone_port == WAN_EC_CHANNEL_PORT_ROUT)?"ROUT":"SOUT",
			(event->tone_type == WAN_EC_TONE_PRESENT)?"PRESENT":"STOP");
	}
					
	if (!(wr->tonemask & (1 << (event->channel-1)))){
		DEBUG_EVENT("%s: Tone detection is not enabled for the channel %d\n",
					card->devname,
					event->channel);
		return;
	}
	if (event->tone_type == WAN_EC_TONE_PRESENT){
		wr->toneactive |= (1 << event->channel);
#ifdef DAHDI_ISSUES
		zt_qevent_lock(
				wr->span.chans[event->channel-1],
				(ZT_EVENT_DTMFDOWN | event->digit));
#else
		zt_qevent_lock(
				&wr->span.chans[event->channel-1],
				(ZT_EVENT_DTMFDOWN | event->digit));
#endif
	}else{
		wr->toneactive &= ~(1 << event->channel);
#ifdef DAHDI_ISSUES
		zt_qevent_lock(
				wr->span.chans[event->channel-1],
				(ZT_EVENT_DTMFUP | event->digit));
#else
		zt_qevent_lock(
				&wr->span.chans[event->channel-1],
				(ZT_EVENT_DTMFUP | event->digit));
#endif
	}
	return;
}

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN) && defined(ZT_DCHAN_TX)
static int wp_tdmv_tx_dchan(struct zt_chan *chan, int len)
{
	wp_tdmv_bri_t	*wp = NULL;
	netskb_t	*skb = NULL;
	wan_smp_flag_t	smp_flags;
	unsigned char	*data = NULL;
	int		err = 0;

	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wp	= chan->pvt;
	WAN_ASSERT(wp->dchan_dev == NULL);

	if (len <= 2){
		return -EINVAL;
	}
	len -= 2; /* Remove checksum */
	skb = wan_skb_alloc(len+1);
	if (skb == NULL){
        	return -ENOMEM;
	}
	data = wan_skb_put(skb, len);
	wan_spin_lock_irq(&chan->lock, &smp_flags);
	memcpy(data, chan->writebuf[chan->inwritebuf], len);
	wan_spin_unlock_irq(&chan->lock, &smp_flags);
#if 0
	{
		int i;
		DEBUG_EVENT("%s: TX DCHAN len: %d\n", chan->name, len);
		/*
		for(i = 0; i < len; i++){
			_DEBUG_EVENT("%02X ", data[i]);
		}
		_DEBUG_EVENT("\n");
		*/
	}
#endif
	if (skb){
		err = wp->dchan_dev->hard_start_xmit(skb, wp->dchan_dev);
		if (err){
			wan_skb_free(skb);
		}
	}

	return err;
}
#endif

/******************************************************************************
** wp_tdmv_rx_dchan() - 
**
**	OK
*/
static int wp_tdmv_rx_dchan(wan_tdmv_t *wan_tdmv, int channo, 
			unsigned char *rxbuf, unsigned int len)
{
	wp_tdmv_bri_t	*wp = wan_tdmv->sc;
	struct zt_chan	*chan = NULL, *ms = NULL;
	wan_smp_flag_t	smp_flags;
	unsigned char	*buf = NULL;
	int		oldbuf;
	int 		i, left;

	DEBUG_TDMV("%s(): channo: %d, wp->dchan: %d, len: %d\n", __FUNCTION__, channo, wp->dchan, len);

	WAN_ASSERT(wp == NULL);
	WAN_ASSERT(channo != wp->dchan-1);
	chan	= &wp->chans[wp->dchan-1];
	WAN_ASSERT(chan == NULL || chan->master == NULL);
	ms = chan->master;

	BRI_FUNC();
	
	if (!IS_TDMV_UP(wp)){
		DEBUG_TDMV("%s: Asterisk is not running!\n",
				wp->devname); 
		return -EINVAL;
	}
	if (!(ms->flags & ZT_FLAG_HDLC)){
		DEBUG_TDMV("%s: ERROR: %s not defined as D-CHAN! Or received HDLC data before 'ztcfg' was run.\n",
				wp->devname, ms->name); 
		return -EINVAL;
	}
	
	if (ms->inreadbuf < 0){
		return -EINVAL;
	}

	if (ms->inreadbuf >= ZT_MAX_NUM_BUFS){
		DEBUG_EVENT("%s: RX buffer (%s) is out of range (%d-%d)!\n",
				wp->devname, ms->name, ms->inreadbuf,ZT_MAX_NUM_BUFS); 
		return -EINVAL;
	}

	/* FIXME wan_spin_lock_irqsave(&wp->tx_rx_lock, smp_flags); */
	wan_spin_lock_irq(&chan->lock, &smp_flags);
	buf = ms->readbuf[ms->inreadbuf];
	left = ms->blocksize - ms->readidx[ms->inreadbuf];
	if (len + 2 > left) {
		DEBUG_EVENT("%s: ERROR: Not ehough space for RX HDLC packet (%d:%d)!\n",
				wp->devname, len+2, left); 
		wan_spin_unlock_irq(&chan->lock, &smp_flags);
		return -EINVAL;
	}
	for(i = 0; i < len; i++){
		buf[ms->readidx[ms->inreadbuf]++] = rxbuf[i];
	}
	/* Add extra 2 bytes for checksum */
	buf[ms->readidx[ms->inreadbuf]++] = 0x00;
	buf[ms->readidx[ms->inreadbuf]++] = 0x00;

	oldbuf = ms->inreadbuf;
	ms->readn[ms->inreadbuf] = ms->readidx[ms->inreadbuf];
	ms->inreadbuf = (ms->inreadbuf + 1) % ms->numbufs;
	if (ms->inreadbuf == ms->outreadbuf) {
		/* Whoops, we're full, and have no where else
		to store into at the moment.  We'll drop it
		until there's a buffer available */
		ms->inreadbuf = -1;
		/* Enable the receiver in case they've got POLICY_WHEN_FULL */
		ms->rxdisable = 0;
	}
	if (ms->outreadbuf < 0) { /* start out buffer if not already */
		ms->outreadbuf = oldbuf;
	}
	/* FIXME wan_spin_unlock_irq(&wp->tx_rx_lock, &smp_flags); */
	wan_spin_unlock_irq(&chan->lock, &smp_flags);
	if (!ms->rxdisable) { /* if receiver enabled */
		DEBUG_TDMV("%s: HDLC block is ready!\n",
					wp->devname);
		/* Notify a blocked reader that there is data available
		to be read, unless we're waiting for it to be full */
#if defined(__LINUX__)
		wake_up_interruptible(&ms->readbufq);
		wake_up_interruptible(&ms->sel);
		if (ms->iomask & ZT_IOMUX_READ)
			wake_up_interruptible(&ms->eventbufq);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
		wakeup(&ms->readbufq);
		wakeup(&ms->sel);
		if (ms->iomask & ZT_IOMUX_READ)
			wakeup(&ms->eventbufq);

#endif
	}
	return 0;
}


/******************************************************************************
** wp_tdmv_report_alarms() - 
**
**	DONE
*/
static void wp_tdmv_report_alarms(void* pcard, unsigned long te_alarm)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_bri_t	*wp = wan_tdmv->sc;

	/* The sc pointer can be NULL, on shutdown. In this
	 * case don't generate error, just get out */
	if (!wp){
		return;
	}

	if(te_alarm == 0){
		wp->span.alarms = ZT_ALARM_NONE;
	}else{
		wp->span.alarms = ZT_ALARM_RED;
	}
	zt_alarm_notify(&wp->span);
	return;
}
