/***************************************************************************
 * sdla_tdmv.c	WANPIPE(tm) Multiprotocol WAN Link Driver. 
 *				TDM voice board configuration.
 *
 * Author: 	Alex Feldman   <al.feldman@sangoma.com>
 *              Nenad Corbic   <ncorbic@sangoma.com>
 *              David Rokvargh <davidr@sangoma.com>
 *
 * Copyright:	(c) 1995-2005 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 * Jul 22, 2001	Nenad Corbic	Initial version.
 * Oct 01, 2001 Gideon Hack	Modifications for interrupt usage.
 * Aug  9, 2005	David Rokhvarg	Added Echo Detection and Control (EDAC).
 ******************************************************************************
 */
/*
 ******************************************************************************
			   INCLUDE FILES
 ******************************************************************************
*/

#if (defined __FreeBSD__) | (defined __OpenBSD__)
# include <net/wanpipe_includes.h>
# include <net/wanpipe_debug.h>
# include <net/wanpipe_defines.h>
# include <net/wanpipe_abstr.h>
# include <net/wanpipe_common.h>
# include <net/wanpipe.h>
# include <net/sdla_tdmv.h>	/* WANPIPE TDM Voice definitions */
# include <zaptel.h>
#elif (defined __WINDOWS__)
# include <wanpipe\csu_dsu.h>
#else

#if 0
# define CONFIG_ZAPATA_BRI_DCHANS 
#endif

# include <zaptel.h>
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe.h>
# include <linux/sdla_tdmv.h>	/* WANPIPE TDM Voice definitions */
#endif

/*
 ******************************************************************************
			  DEFINES AND MACROS
 ******************************************************************************
*/
//#define DEBUG_ECHO	DEBUG_EVENT
#define DEBUG_ECHO	if(0) DEBUG_EVENT

#define WP_MAX_CARDS		32

#define FIXME_MSG(func)		DEBUG_EVENT("(%s): FIXME: line %d\n", func, __LINE__)
#define DBG_FUNC_START(func)	DEBUG_EVENT("(DEBUG)(TDM Voice): %s - Start\n", func)
#define DBG_FUNC_END(func)	DEBUG_EVENT("(DEBUG)(TDM Voice): %s - End\n", func)

/* flags bits */
#define WP_TDMV_REGISTER	1	/*0x01*/
#define WP_TDMV_RUNNING		2	/*0x02*/
#define WP_TDMV_UP		3	/*0x04*/
#define WP_TDMV_SIG_ENABLE	4	/*0x08*/
#define WP_TDMV_SIG_POLL	5	/*0x10*/
#define WP_TDMV_RBS_READY	6	/*0x20*/
#define WP_TDMV_RBS_BUSY	7	/*0x40*/
#define WP_TDMV_RBS_UPDATE	8	/*0x80*/

#define IS_TDMV_RUNNING(wp)	wan_test_bit(WP_TDMV_RUNNING, &(wp)->flags)
#define IS_TDMV_UP(wp)		wan_test_bit(WP_TDMV_UP, &(wp)->flags)
#define IS_TDMV_SIG_POLL(wp)	wan_test_bit(WP_TDMV_SIG_POLL, &(wp)->flags)
#define IS_TDMV_SIG_ENABLE(wp)	wan_test_bit(WP_TDMV_SIG_ENABLE, &(wp)->flags)
#define IS_TDMV_UP_RUNNING(wp)	(IS_TDMV_UP(wp) && IS_TDMV_RUNNING(wp))
#define IS_TDMV_SIG(wp)		(IS_TDMV_SIG_POLL(wp) && IS_TDMV_SIG_ENABLE(wp))

#define WP_TDMV_ENABLE		0x01
#define WP_TDMV_DISABLE		0x02

#if defined(__FreeBSD__)
extern short *__zt_mulaw;
#endif


#if 0
static unsigned char wp_tdmv_ulaw[] = {
     0,    0,    0,    0,    0,    1,    1,    1,
     1,    2,    2,    2,    2,    3,    3,    3,
     3,    4,    4,    4,    4,    5,    5,    5,
     5,    6,    6,    6,    6,    7,    7,    7,
     7,    8,    8,    8,    8,    9,    9,    9,
     9,   10,   10,   10,   10,   11,   11,   11,
    11,   12,   12,   12,   12,   13,   13,   13,
    13,   14,   14,   14,   14,   15,   15,   15,
    15,   16,   16,   17,   17,   18,   18,   19,
    19,   20,   20,   21,   21,   22,   22,   23,
    23,   24,   24,   25,   25,   26,   26,   27,
    27,   28,   28,   29,   29,   30,   30,   31,
    31,   32,   33,   34,   35,   36,   37,   38,
    39,   40,   41,   42,   43,   44,   45,   46,
    47,   49,   51,   53,   55,   57,   59,   61,
    63,   66,   70,   74,   78,   84,   92,  104,
   254,  231,  219,  211,  205,  201,  197,  193,
   190,  188,  186,  184,  182,  180,  178,  176,
   175,  174,  173,  172,  171,  170,  169,  168,
   167,  166,  165,  164,  163,  162,  161,  160,
   159,  159,  158,  158,  157,  157,  156,  156,
   155,  155,  154,  154,  153,  153,  152,  152,
   151,  151,  150,  150,  149,  149,  148,  148,
   147,  147,  146,  146,  145,  145,  144,  144,
   143,  143,  143,  143,  142,  142,  142,  142,
   141,  141,  141,  141,  140,  140,  140,  140,
   139,  139,  139,  139,  138,  138,  138,  138,
   137,  137,  137,  137,  136,  136,  136,  136,
   135,  135,  135,  135,  134,  134,  134,  134,
   133,  133,  133,  133,  132,  132,  132,  132,
   131,  131,  131,  131,  130,  130,  130,  130,
   129,  129,  129,  129,  128,  128,  128,  128,
};
#endif

/*
 ******************************************************************************
			STRUCTURES AND TYPEDEFS
 ******************************************************************************
*/
typedef struct wp_tdmv_pvt_area
{
	sdla_t*		card;
	char*		devname;
	// FIXME struct pci_dev *dev;
	wan_spinlock_t	lock;
	wan_spinlock_t	tx_rx_lock;
	int		ise1;
	int		num;
	int		spanno;
	int		flags;
	int		lbo;
	int 		lcode;
	int 		frame;
	int		usecount;
	int		sync;
	int		blinktimer;
	int		alarmtimer;
	int		loopupcnt;
	int		loopdowncnt;
#ifdef FANCY_ALARM
	int		alarmpos;
#endif
	/* T1 signalling */
	struct zt_span	span;					/* Span */
	struct zt_chan	chans[31];				/* Channels */
	unsigned char	ec_chunk1[31][ZT_CHUNKSIZE];
	unsigned char	ec_chunk2[31][ZT_CHUNKSIZE];
	unsigned long	config_tsmap;
	unsigned long	timeslot_map;
	int		max_timeslots;		/* T1: 24, E1: 31 */
	int		timeslots;		/* configured timeslots */
	unsigned long	sig_timeslot_map;
	int		sig_timeslots;
	unsigned long	rbs_tx_status;
	unsigned long	rbs_tx1_status;
	unsigned char	rbs_tx[31];
	unsigned char	rbs_tx1[31];
	unsigned char	rbs_rx[31];
	unsigned long	rbs_rx_pending;
	int 		intcount;
	int 		rbscount;
	unsigned int	brt_ctrl;
	unsigned char	echo_off;
	unsigned long	echo_off_map;
	unsigned int	max_rxtx_len;

	unsigned int	channelized;	/* WAN_TRUE or WAN_FALSE */
	unsigned char	*tx_unchan;	/* tx data pointer for unchannelized mode */

	/* AHDLC: Hardware HDLC arguments */
	unsigned int	dchan;
	netdevice_t	*dchan_dev;

} wp_tdmv_softc_t;



/*
*******************************************************************************
**			   GLOBAL VARIABLES
*******************************************************************************
*/
static int		wp_card_no = 0;
static unsigned char	brt[256];
WAN_LIST_HEAD(, wan_tdmv_) wan_tdmv_head = 
			WAN_LIST_HEAD_INITIALIZER(&wan_tdmv_head);
/*
*******************************************************************************
**			  FUNCTION PROTOTYPES
*******************************************************************************
*/

static int wp_tdmv_startup(struct zt_span *span);
static int wp_tdmv_shutdown(struct zt_span *span);
static int wp_tdmv_maint(struct zt_span *span, int cmd);
static int wp_tdmv_chanconfig(struct zt_chan *chan, int sigtype);
static int wp_tdmv_spanconfig(struct zt_span *span, struct zt_lineconfig *lc);
static int wp_tdmv_sigctrl(sdla_t* card, wp_tdmv_softc_t *wp, int status);
static int wp_tdmv_rbsbits(struct zt_chan *chan, int bits);
static int wp_tdmv_tx_rbsbits(wp_tdmv_softc_t *wp);
static int wp_tdmv_open(struct zt_chan *chan);
static int wp_tdmv_close(struct zt_chan *chan);
static void wp_tdmv_release(wp_tdmv_softc_t *wp);
#if defined(__FreeBSD__)
static int wp_tdmv_ioctl(struct zt_chan*, unsigned int, caddr_t);
#else
static int wp_tdmv_ioctl(struct zt_chan*, unsigned int, unsigned long);
#endif
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN)
static int wp_tdmv_tx_dchan(struct zt_chan *chan, int len);
#endif

static int wp_tdmv_rxcopy(wan_tdmv_t *wan_tdmv, unsigned char* rxbuf, int max_len);
static int wp_tdmv_rxprep(wan_tdmv_t* wan_tdmv, unsigned char*, int);
static int wp_tdmv_txcopy(wan_tdmv_t* wan_tdmv, unsigned char*, int);
static int wp_tdmv_txprep(wan_tdmv_t* wan_tdmv, unsigned char*, int);

static inline void start_alarm(wp_tdmv_softc_t* wp);
static inline void stop_alarm(wp_tdmv_softc_t* wp);

/******************************************************************************
** () - 
**
**
*/

/*
**			  FUNCTION DEFINITIONS
**
*/
int wp_tdmv_check_mtu(void* pcard, unsigned long timeslot_map)
{
	sdla_t	*card = (sdla_t*)pcard;
	int	x, num_of_channels = 0, max_channels;

	max_channels = GET_TE_CHANNEL_RANGE(&card->fe);
	for (x = 0; x < max_channels; x++) {
		if (test_bit(x,&timeslot_map)){
			num_of_channels++;
		}
	}
	return (ZT_CHUNKSIZE * num_of_channels);
}

/******************************************************************************
** wp_tdmv_create() - 
**
**	OK
*/
int wp_tdmv_create(void* pcard, wandev_conf_t *conf)
{
	sdla_t		*card = (sdla_t*)pcard;
	wp_tdmv_softc_t	*wp = NULL;
	wan_tdmv_t	*tmp = NULL;

	WAN_ASSERT(card == NULL);
	WAN_ASSERT(conf->u.aft.tdmv_span_no == 0);
	memset(&card->wan_tdmv, 0x0, sizeof(wan_tdmv_t));
	/* We are forcing to register wanpipe devices at the same sequence
	 * that it defines in /etc/zaptel.conf */
   	WAN_LIST_FOREACH(tmp, &wan_tdmv_head, next){
		if (tmp->spanno == conf->u.aft.tdmv_span_no){
			DEBUG_EVENT("%s: Registering device with an incorrect span number!\n",
					card->devname);
			DEBUG_EVENT("%s: Another wanpipe device already configured to span #%d!\n",
					card->devname, conf->u.aft.tdmv_span_no);
			return -EINVAL;
		}
		if (!WAN_LIST_NEXT(tmp, next)){
			break;
		}
	}

	card->wan_tdmv.max_timeslots	= GET_TE_CHANNEL_RANGE(&card->fe);
	card->wan_tdmv.spanno		= conf->u.aft.tdmv_span_no;
	card->wandev.te_report_rbsbits  = wp_tdmv_report_rbsbits;
	card->wandev.te_report_alarms	= wp_tdmv_report_alarms;	

	wp = wan_malloc(sizeof(wp_tdmv_softc_t));
	if (wp == NULL){
		return -ENOMEM;
	}
	memset(wp, 0x0, sizeof(wp_tdmv_softc_t));
	card->wan_tdmv.sc	= wp;
	wp->spanno		= conf->u.aft.tdmv_span_no-1;
	wp->num			= wp_card_no++;
	wp->card		= card;
	wp->devname		= card->devname;
	wp->lcode		= WAN_FE_LCODE(&card->fe);
	wp->frame		= WAN_FE_FRAME(&card->fe);
	wp->lbo			= WAN_TE1_LBO(&card->fe);
	wp->ise1		= IS_E1_FEMEDIA(&card->fe) ? 1 : 0;
	wp->max_timeslots	= IS_E1_FEMEDIA(&card->fe) ? 31: 24;
	wp->max_rxtx_len		= 0;
	wan_spin_lock_init(&wp->lock);
	wan_spin_lock_init(&wp->tx_rx_lock);
	/* AHDLC */
	if (conf->u.aft.tdmv_dchan){
		/* PRI signalling is selected with hw HDLC (dchan is not 0) */
		wp->dchan = conf->u.aft.tdmv_dchan;
	}

	if (tmp){
		WAN_LIST_INSERT_AFTER(tmp, &card->wan_tdmv, next);
	}else{
		WAN_LIST_INSERT_HEAD(&wan_tdmv_head, &card->wan_tdmv, next);
	}

	return 0;
}

/******************************************************************************
** wp_tdmv_reg() - 
**
** Returns: 	0-31	- Return TDM Voice channel number.
**		-EINVAL - otherwise
**	OK
*/
int wp_tdmv_reg(void* pcard, wanif_conf_t *conf, netdevice_t *dev)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t	*wp = NULL;
	int		i, channo = 0, cnt = 0;
	
	WAN_ASSERT(wan_tdmv->sc == NULL);
	wp = wan_tdmv->sc;
		
	if (wan_test_bit(WP_TDMV_REGISTER, &wp->flags)){
		DEBUG_EVENT("%s: Error: Master device has already been configured!\n",
				card->devname);
		return -EINVAL;
	}
		
	/*
	 * T1: 1-24
	 * E1: 0-31 */
	for(i = 0; i<wp->max_timeslots; i++){
		if (wan_test_bit(i, &conf->active_ch)){
			wan_set_bit(i, &wp->timeslot_map);
			if (conf->tdmv_echo_off){
				wan_set_bit(i, &wp->echo_off_map);
			}
			channo = i;
			cnt++;
		}
	}	

	if (!cnt){
		DEBUG_EVENT("%s: Error: TDMV iface %s configured with 0 timeslots!\n",
				card->devname, conf->name);
		return -EINVAL;
	}

	if (cnt > 1){
		/* Unchannelized implementation */
		DEBUG_EVENT("%s:    TDMV Mode     :Unchannelized!\n",
				wp->devname);
		channo = 0;
#if 0
		if (is_last != WAN_TRUE){
			DEBUG_EVENT("%s: Error: Unchannelized interface must be the Master If (slots=%i)!\n",
				wp->devname,cnt);
			return -EINVAL;	
		}
#endif
		if (conf->tdmv_echo_off){
			DEBUG_EVENT("%s:    TDMV Echo Ctrl:Off\n",
					wp->devname);
		}
		wp->channelized = WAN_FALSE;

		if (wp->dchan){
			if (dev == NULL){
				DEBUG_EVENT("%s: ERROR: Device pointer is NULL for D-chan!\n",
							wp->devname);
				return -EINVAL;
			}
			wp->dchan_dev = dev;
		}
	}else{
			
		/* Channelized implementation */
		if (wp->dchan-1 == channo){
			if (dev == NULL){
				DEBUG_EVENT("%s: ERROR: Device pointer is NULL for D-chan!\n",
							wp->devname);
				return -EINVAL;
			}
			wp->dchan_dev = dev;
		}
		if (conf->tdmv_echo_off){
			DEBUG_EVENT("%s:    TDMV Echo Ctrl:Off\n",
					wp->devname);
		}
		memset(wp->chans[channo].sreadchunk, WAN_TDMV_IDLE_FLAG, ZT_CHUNKSIZE);
		memset(wp->chans[channo].swritechunk, WAN_TDMV_IDLE_FLAG, ZT_CHUNKSIZE);
		wp->chans[channo].readchunk = wp->chans[channo].sreadchunk;
		wp->chans[channo].writechunk = wp->chans[channo].swritechunk;
		wp->channelized = WAN_TRUE;
	}
	wp->max_rxtx_len = wp_tdmv_check_mtu(card, conf->active_ch);
	return channo;
}


/******************************************************************************
** wp_tdmv_del() - 
**
**	OK
*/
int wp_tdmv_unreg(void* pcard, unsigned long ts_map)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t	*wp = NULL;
	int		channo = 0;

	WAN_ASSERT(wan_tdmv->sc == NULL);
	wp = wan_tdmv->sc;


	for(channo = 0; channo < wp->max_timeslots; channo++){
		if (wan_test_bit(channo, &wp->timeslot_map)){
			wan_clear_bit(channo, &wp->timeslot_map);
			wan_clear_bit(channo, &wp->echo_off_map);
			memset(wp->chans[channo].sreadchunk, 
					WAN_TDMV_IDLE_FLAG, 
					ZT_CHUNKSIZE);
			memset(wp->chans[channo].swritechunk, 
					WAN_TDMV_IDLE_FLAG, 
					ZT_CHUNKSIZE);
			wp->chans[channo].readchunk = 
					wp->chans[channo].sreadchunk;
			wp->chans[channo].writechunk = 
					wp->chans[channo].swritechunk;
		}
	}
	return 0;
}


/******************************************************************************
** wp_tdmv_running() - 
**
**	OK
*/
int wp_tdmv_running(void* pcard)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t	*wp = NULL;

	wp = wan_tdmv->sc;
	if (wp && wp->usecount){
		DEBUG_EVENT("%s: WARNING: Wanpipe is still used by Asterisk!\n",
				card->devname);
		return -EINVAL;
	}
	return 0;
}

/******************************************************************************
** wp_tdmv_remove() - 
**
**	OK
*/
int wp_tdmv_remove(void* pcard)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t	*wp = NULL;
	wan_smp_flag_t  flags;

	if (!card->wan_tdmv.sc){
		return 0;
	}
	
	wp = wan_tdmv->sc;
	/* Release span, possibly delayed */
	if (wp && wp->timeslot_map){
		DEBUG_EVENT("%s: Some interfaces are not unregistered (%08lX)!\n",
				card->devname, wp->timeslot_map);
		return -EINVAL;
	}
	if (wp && wp->usecount){
		DEBUG_EVENT("%s: ERROR: Wanpipe is still used by Asterisk!\n",
				card->devname);
		return -EINVAL;
	}

	wan_spin_lock_irq(&wp->lock, &flags);
	card->wandev.te_report_rbsbits = NULL;
	card->wandev.te_report_alarms = NULL;	
	wan_spin_unlock_irq(&wp->lock, &flags);
	
	if (wp){
		wan_clear_bit(WP_TDMV_RUNNING, &wp->flags);
		wan_clear_bit(WP_TDMV_UP, &wp->flags);
		wan_tdmv->sc = NULL;
		wp_tdmv_sigctrl(card, wp, WP_TDMV_DISABLE);
		WAN_LIST_REMOVE(wan_tdmv, next);
		wp_tdmv_release(wp);
	}else{
		wan_tdmv->sc = NULL;
	}
	return 0;
}

void wp_tdmv_state(void* pcard, int state)
{
	sdla_t*	card = (sdla_t*)pcard;
	wan_tdmv_t*	wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t*	wp = NULL;

	WAN_ASSERT1(wan_tdmv->sc == NULL);
	wp = (wp_tdmv_softc_t*)wan_tdmv->sc;
	
	switch(state){
	case WAN_CONNECTED:
		wan_set_bit(WP_TDMV_UP, &wp->flags);
		break;

	case WAN_DISCONNECTED:
		wan_clear_bit(WP_TDMV_UP, &wp->flags);
		wp->rbs_rx_pending = wp->sig_timeslot_map;
		break;
	}
	return;
}

static int wp_tdmv_sigctrl(sdla_t* card, wp_tdmv_softc_t *wp, int status)
{

	WAN_ASSERT(wp == NULL);
	WAN_ASSERT(card == NULL);

#if 0
	if (wp->sig_timeslots == wp->span.channels){
		/* All channels are configured to FXO/FXS.
		 * Only in this case we can use Signalling interrupt */
   		if (!wan_test_bit(WP_TDMV_SIG_POLL, &wp->flags)){
			if (status == WP_TDMV_ENABLE){
				DEBUG_EVENT("%s: Enable Signalling interrupt (span=%d)!\n",
					card->devname, 
					wp->spanno);
				card->wandev.fe_iface.set_fe_sigcfg(&card->fe, 0);
   				wan_set_bit(WP_TDMV_SIG_ENABLE, &wp->flags);
			}
		}else{
			if (status == WP_TDMV_DISABLE){
				DEBUG_EVENT("%s: Disable Signalling interrupt (span=%d)!\n",
					card->devname, 
					wp->spanno);
				card->wandev.fe_iface.set_fe_sigcfg(&card->fe, 0);
				wp->flags &= ~WP_TDMV_SIG_NONE;
			}
		}
	}else{
		wp->sig_status = (status == WP_TDMV_ENABLE) ? 
						WP_TDMV_SIG_POLL : WP_TDMV_SIG_NONE;
	}
#else
	switch(status){
	case WP_TDMV_ENABLE:
   		wan_set_bit(WP_TDMV_SIG_ENABLE, &wp->flags);
   		wan_set_bit(WP_TDMV_RBS_READY, &wp->flags);
		if (!wan_test_bit(WP_TDMV_SIG_POLL, &wp->flags)){
			card->wandev.fe_iface.set_fe_sigctrl(
					&card->fe, WAN_TE_SIG_INTR);
		}
		break;
	case WP_TDMV_DISABLE:
   		wan_clear_bit(WP_TDMV_RBS_READY, &wp->flags);
   		wan_clear_bit(WP_TDMV_SIG_ENABLE, &wp->flags);
		break;
	default:
		DEBUG_EVENT("%s: Unknown signalling mode (%d)\n", 
				wp->devname, status);
		return -EINVAL;
	}
#endif
	return 0;
}

/******************************************************************************
** wp_tdmv_report_rbsbits() - 	Report A,B bit status changes to TDM Voice
**				requests. 
**
**	DONE
*/
void wp_tdmv_report_rbsbits(void* pcard, int channel, unsigned char status)
{
	sdla_t*			card = (sdla_t*)pcard;
	wan_tdmv_t*		wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t*	wp = NULL;
	int 			rxs = 0, i = 0;
	// int a = 0, b = 0, i = 0, y = 0;

	WAN_ASSERT1(wan_tdmv->sc == NULL);
	wp = (wp_tdmv_softc_t*)wan_tdmv->sc;

	if (!test_bit(channel-1, &wp->timeslot_map)){
		return;
	}

	if (status & BIT_SIGX_A) rxs |= ZT_ABIT;
	if (status & BIT_SIGX_B) rxs |= ZT_BBIT;
	if (status & BIT_SIGX_C) rxs |= ZT_CBIT;
	if (status & BIT_SIGX_D) rxs |= ZT_DBIT;
#if 0
	if (wp->ise1){
		FIXME_MSG("wp_tdmv_report_rbsbits");
		/* Read 5 registers at a time, loading 10 channels at a time */
		for (i = (x *5); i < (x * 5) + 5; i++) {
			// FIXME a = __t1_get_reg(wp, 0x31 + i);
			/* Get high channel in low bits */
			rxs = (a & 0xf);
			if (!(wp->chans[i+15].sig & ZT_SIG_CLEAR)) {
				if (wp->chans[i+15].rxsig != rxs)
					zt_rbsbits(&wp->chans[i+15], rxs);
			}
			rxs = (a >> 4) & 0xf;
			if (!(wp->chans[i].sig & ZT_SIG_CLEAR)) {
				if (wp->chans[i].rxsig != rxs)
					zt_rbsbits(&wp->chans[i], rxs);
			}
		}
	}
#endif
	for(i=0; i < wp->span.channels;i++){
		if (wp->chans[i].chanpos == channel)
			break;
	}
	if (i == wp->span.channels){
		return;
	}
	if (!(wp->chans[i].sig & ZT_SIG_CLEAR) &&
	    (wp->chans[i].rxsig != rxs)){
		zt_rbsbits(&wp->chans[i], rxs);
#if 0
		DEBUG_EVENT("[TDMV] %s: %s:%02d(%d) RX RBS: A:%1d B:%1d C:%1d D:%1d\n",
				wp->devname, 
				(wp->ise1) ? "E1" : "T1",
				channel, wp->chans[i].channo,
				(rxs & ZT_ABIT) ? 1 : 0,
				(rxs & ZT_BBIT) ? 1 : 0,
				(rxs & ZT_CBIT) ? 1 : 0,
				(rxs & ZT_DBIT) ? 1 : 0);
#endif
	}
}

/******************************************************************************
** wp_tdmv_report_alarms() - 
**
**	DONE
*/
void wp_tdmv_report_alarms(void* pcard, unsigned long te_alarm)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t	*wp = NULL;
	int		alarms = 0, prev_alarms;
	int		x,j;

	/* The sc pointer can be NULL, on shutdown. In this
	 * case don't generate error, just get out */
	wp = wan_tdmv->sc;
	if (!wp){
		return;
	}

	/* And consider only carrier alarms */
	wp->span.alarms &= (ZT_ALARM_RED | ZT_ALARM_BLUE | ZT_ALARM_NOTOPEN);
	prev_alarms = wp->span.alarms;

	if (wp->ise1){
		/* XXX Implement me XXX */
	}else{
		/* Detect loopup code if we're not sending one */
		if (!wp->span.mainttimer && (te_alarm & WAN_TE_BIT_LOOPUP_CODE)){
			/* Loop-up code detected */
			if ((wp->loopupcnt++ > 80)  && (wp->span.maintstat != ZT_MAINT_REMOTELOOP)){
				card->wandev.fe_iface.set_fe_lbmode(
						&wp->card->fe,
					       	WAN_TE1_DDLB_MODE,
					       	WAN_TE1_DEACTIVATE_LB);
				card->wandev.fe_iface.set_fe_lbmode(
						&wp->card->fe,
						WAN_TE1_LINELB_MODE,
					       	WAN_TE1_ACTIVATE_LB);
				wp->span.maintstat = ZT_MAINT_REMOTELOOP;
			}
		}else{
			wp->loopupcnt = 0;
		}
		/* Same for loopdown code */
		if (!wp->span.mainttimer && (te_alarm & WAN_TE_BIT_LOOPDOWN_CODE)){
			/* Loop-down code detected */
			if ((wp->loopdowncnt++ > 80)  && (wp->span.maintstat == ZT_MAINT_REMOTELOOP)){
				card->wandev.fe_iface.set_fe_lbmode(
						&wp->card->fe,
						WAN_TE1_DDLB_MODE,
					       	WAN_TE1_DEACTIVATE_LB);
				card->wandev.fe_iface.set_fe_lbmode(
						&wp->card->fe,
						WAN_TE1_LINELB_MODE,
					       	WAN_TE1_DEACTIVATE_LB);
				wp->span.maintstat = ZT_MAINT_NONE;
			}
		}else{
			wp->loopdowncnt = 0;
		}
	}

	if (wp->span.lineconfig & ZT_CONFIG_NOTOPEN) {
		for (x=0,j=0;x < wp->span.channels;x++){
			if ((wp->chans[x].flags & ZT_FLAG_OPEN) ||
			    (wp->chans[x].flags & ZT_FLAG_NETDEV)){
				j++;
			}
		}
		if (!j){
			alarms |= ZT_ALARM_NOTOPEN;
		}
	}

	if (wp->ise1) {
		if (te_alarm & WAN_TE_BIT_RED_ALARM) 
			alarms |= ZT_ALARM_RED;
		if (te_alarm & WAN_TE_BIT_AIS_ALARM)
			alarms |= ZT_ALARM_BLUE;
	} else {
		/* Check actual alarm status */
		if (te_alarm & WAN_TE_BIT_RED_ALARM) 
			alarms |= ZT_ALARM_RED;
		if (te_alarm & WAN_TE_BIT_AIS_ALARM)
			alarms |= ZT_ALARM_BLUE;
	}
	/* Keep track of recovering */
	if ((!alarms) && wp->span.alarms)
		wp->alarmtimer = ZT_ALARMSETTLE_TIME;

#if 0
	/* If receiving alarms, go into Yellow alarm state */
	if (alarms && (!wp->span.alarms)) {
		DEBUG_TDMV("%s: Going into yellow alarm\n",
				wp->devname);
		if (card->wandev.fe_iface.set_fe_alarm){
			card->wandev.fe_iface.set_fe_alarm(&card->fe, WAN_TE_BIT_YEL_ALARM);
		}
	}
#endif

	if (wp->span.alarms != alarms) {
		// FIXME: Alarm status changed!!!
	 	DEBUG_TDMV("%s: Alarm status changed %X!\n",
			       	wp->devname,alarms);
	}
//	if (wp->alarmtimer)
//		alarms |= ZT_ALARM_RECOVER;
	if (te_alarm & WAN_TE_BIT_YEL_ALARM)
		alarms |= ZT_ALARM_YELLOW;

	wp->span.alarms = alarms;
	if (wan_test_bit(WP_TDMV_RUNNING, &wp->flags)){
#if 0
		/* If receiving alarms, go into Yellow alarm state */
		if (alarms && (!prev_alarms)) {
			DEBUG_TDMV("%s: Going into yellow alarm\n",
					wp->devname);
			if (card->wandev.fe_iface.set_fe_alarm){
				card->wandev.fe_iface.set_fe_alarm(
							&card->fe,
							WAN_TE_BIT_YEL_ALARM);
			}
		}
#endif
		zt_alarm_notify(&wp->span);
	}
	return;
}

/******************************************************************************
** wp_tdmv_txcopy() - 
**
**	OK
*/
static int 
wp_tdmv_txcopy(wan_tdmv_t *wan_tdmv, unsigned char* txbuf, int max_len)
{
	wp_tdmv_softc_t	*wp = wan_tdmv->sc;
	int		x = 0, y = 0, offset = 0;

	WAN_ASSERT(wp == NULL);
	for (y=0;y<ZT_CHUNKSIZE;y++) {
		for (x=0;x<wp->span.channels;x++){
			if (!test_bit(x,&wp->timeslot_map)){
				continue;
			}
			txbuf[offset++] = (wan_tdmv->brt_enable) ?
						brt[wp->chans[x].writechunk[y]] :
						wp->chans[x].writechunk[y];
		}
	}
	return (offset+1);
}

/******************************************************************************
** wp_tdmv_txprep() - 
**
**	OK
*/
static int 
wp_tdmv_txprep(wan_tdmv_t *wan_tdmv, unsigned char* txbuf, int max_len)
{
	wp_tdmv_softc_t	*wp = wan_tdmv->sc;

	WAN_ASSERT2(wp == NULL, 0);
	zt_transmit(&wp->span);
	return wp_tdmv_txcopy(wan_tdmv, txbuf, max_len);
}

/******************************************************************************
** wp_tdmv_rxcopy() - 
**
**	OK
*/
static int 
wp_tdmv_rxcopy(wan_tdmv_t *wan_tdmv, unsigned char* rxbuf, int max_len)
{
	wp_tdmv_softc_t	*wp = wan_tdmv->sc;
	int		x, y, offset = 0;
	int		channels = wp->span.channels;
	unsigned char	value;

	WAN_ASSERT(wp == NULL);
	for (y=0;y<ZT_CHUNKSIZE;y++){
		for (x=0;x<channels;x++){
			if (test_bit(x,&wp->timeslot_map)){
				value = rxbuf[offset++];
			}else{
				value = WAN_TDMV_IDLE_FLAG;	//0xFE;
			}
			wp->chans[x].readchunk[y] = 
				(wan_tdmv->brt_enable) ? brt[value] : value;
		}
	}
	return offset;
}

/******************************************************************************
** wp_tdmv_rxprep() - 
**
**	OK
*/
static int 
wp_tdmv_rxprep(wan_tdmv_t *wan_tdmv, unsigned char* rxbuf, int max_len)
{
	wp_tdmv_softc_t	*wp = wan_tdmv->sc;
	int		x, offset = 0, channels = wp->span.channels;
	
	WAN_ASSERT2(wp == NULL, 0);

	offset = wp_tdmv_rxcopy(wan_tdmv, rxbuf, max_len);

	if (!wp->echo_off_map){
		for (x = 0; x < channels; x++){
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
			wan_tdmv_rxtx_pwr_t *pwr_rxtx = &wan_tdmv->chan_pwr[x];
			wp_tdmv_echo_check(wan_tdmv, &wp->chans[0], x);
			if(pwr_rxtx->current_state != ECHO_ABSENT){
#endif
				zt_ec_chunk(
					&wp->chans[x], 
					wp->chans[x].readchunk, 
					wp->chans[x].writechunk);
#if 0
				zt_ec_chunk(
					&wp->chans[x], 
					wp->chans[x].readchunk, 
					wp->ec_chunk1[x]);
				memcpy(
					wp->ec_chunk1[x],
					wp->chans[x].writechunk,
					ZT_CHUNKSIZE);
#endif

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
			}/* if() */
#endif
		}/* for() */
	}/* if() */

	zt_receive(&wp->span);
	return (offset);
}


/******************************************************************************
** wp_tdmv_rx_tx() - 
**
**	OK
*/
int wp_tdmv_rx_tx(void* pcard, netskb_t* skb)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t	*wp = NULL;
	int 		len;

	WAN_ASSERT(wan_tdmv->sc == NULL);
	WAN_ASSERT(skb == NULL);
	wp = wan_tdmv->sc;

	if (wan_skb_len(skb) != wp->max_rxtx_len){
		if (WAN_NET_RATELIMIT()) {   
                DEBUG_EVENT("%s: Internal Error[%s:%d]: Wrong buffer lenght %d (%d)!\n", 
					wp->devname, 
					__FUNCTION__,__LINE__,
					wan_skb_len(skb),
					wp->max_rxtx_len);
		}
                return 0;
        }

	wp->intcount ++;

	len = wp_tdmv_rxprep(
			wan_tdmv,
			wan_skb_data(skb),
			wan_skb_len(skb)); 
	len = wp_tdmv_txprep(
			wan_tdmv,
			wan_skb_data(skb),
			wan_skb_len(skb)); 

	if (wan_test_bit(WP_TDMV_RBS_UPDATE, &wp->flags)){
		if (card->wandev.fe_iface.report_rbsbits){
			card->wandev.fe_iface.report_rbsbits(&card->fe);
		}
		wan_clear_bit(WP_TDMV_RBS_UPDATE, &wp->flags);
		wan_clear_bit(WP_TDMV_RBS_BUSY, &wp->flags);
	}

	return wp->max_rxtx_len;
}



/*
**		STATIC FUNCTION DEFINITIONS
**
*/

static void wp_tdmv_generate_bit_rev_table(void)
{
	unsigned char util_char;
	unsigned char misc_status_byte;
	int i;

	/* generate the bit-reversing table for all unsigned characters */
	for(util_char = 0;; util_char ++) {
		misc_status_byte = 0;			/* zero the character to be 'built' */
		/* process all 8 bits of the source byte and generate the */
		for(i = 0; i <= 7; i ++) {
		      	/* corresponding 'bit-flipped' character */
			if(util_char & (1 << i)) {
				misc_status_byte |= (1 << (7 - i));
			}
		}
		/* insert the 'bit-flipped' character into the table at the appropriate location */
		brt[util_char] = misc_status_byte; 

		/* exit when all unsigned characters have been processed */
		if(util_char == 0xFF) {
			break;
		}
	}
}

/******************************************************************************
** wp_tdmv_software_init() - 
**
** OK
*/
int wp_tdmv_software_init(wan_tdmv_t *wan_tdmv)
{
	sdla_t		*card = NULL;
	wp_tdmv_softc_t	*wp = wan_tdmv->sc;
	int		x = 0;

	WAN_ASSERT(wp == NULL);
	WAN_ASSERT(wp->card == NULL);
	card = wp->card;
	
	if (wan_test_bit(WP_TDMV_REGISTER, &wp->flags)){
		DEBUG_EVENT("%s: Wanpipe device is already registered to Zaptel span # %d!\n", 
					wp->devname, wp->span.spanno);
		return 0;
	}
#if 0
	if (wp->ise1 && wp->channelized == WAN_FALSE){
		/* Timeslot map for E1 includes ts0. TDM voice does never
		** use ts0, so i will shift map 1 bit right to get 
		** everything alignment as T1 */
		wp->timeslot_map = wp->timeslot_map >> 1;
		wp->echo_off_map = wp->echo_off_map >> 1;
	}
#endif
	if (wp->ise1){
		sprintf(wp->span.name, "WPE1/%d", wp->num);
	}else{
		sprintf(wp->span.name, "WPT1/%d", wp->num);
	}
	sprintf(wp->span.desc, "%s card %d", wp->devname, wp->num);
	wp->span.spanconfig = wp_tdmv_spanconfig;
	wp->span.chanconfig = wp_tdmv_chanconfig;
	wp->span.startup = wp_tdmv_startup;
	wp->span.shutdown = wp_tdmv_shutdown;
	wp->span.rbsbits = wp_tdmv_rbsbits;
	wp->span.maint = wp_tdmv_maint;
	wp->span.open = wp_tdmv_open;
	wp->span.close = wp_tdmv_close;
	wp->span.channels = wp->max_timeslots;
	wp->span.chans = wp->chans;
	wp->span.flags = ZT_FLAG_RBS;
	wp->span.linecompat = ZT_CONFIG_AMI | ZT_CONFIG_B8ZS | ZT_CONFIG_D4 | ZT_CONFIG_ESF;
	wp->span.ioctl = wp_tdmv_ioctl;
	wp->span.pvt = wp;
	if (wp->ise1)
		wp->span.deflaw = ZT_LAW_ALAW;
	else
		wp->span.deflaw = ZT_LAW_MULAW;
#if !defined(__FreeBSD__)
	init_waitqueue_head(&wp->span.maintq);
#endif
	for (x=0;x<wp->span.channels;x++) {
		sprintf(wp->chans[x].name, "%s/%d", wp->span.name, x+1);
		if (test_bit(x,&wp->timeslot_map)){
			DEBUG_TEST("%s: Configure channel %d for voice (%s)!\n", 
					wp->devname, 
					x + 1,
					wp->chans[x].name);
			wp->chans[x].sigcap = 
				ZT_SIG_EM | ZT_SIG_CLEAR | ZT_SIG_EM_E1 | 
				ZT_SIG_FXSLS | ZT_SIG_FXSGS | 
				ZT_SIG_FXSKS | ZT_SIG_FXOLS | 
				ZT_SIG_FXOGS | ZT_SIG_FXOKS | ZT_SIG_CAS;
		}else{
			wp->chans[x].sigcap = ZT_SIG_NONE;
		}
		wp->chans[x].pvt = wp;
		wp->chans[x].chanpos = x + 1;
		wp->chans[x].rxsig = 0x00;
	}

	if (zt_register(&wp->span, 0)) {
		DEBUG_EVENT("%s: Unable to register span with zaptel\n",
					wp->devname);
		return -EINVAL;
	}
	
	if (wp->span.spanno != wp->spanno +1){
		DEBUG_EVENT("\n");
		DEBUG_EVENT("WARNING: Span number %d is already used by another device!\n",
						wp->spanno + 1);
		DEBUG_EVENT("         Possible cause: Another TDM driver already loaded!\n");
		DEBUG_EVENT("         Solution:       Unload wanpipe and check currently\n");
		DEBUG_EVENT("                         used spans in /proc/zaptel directory.\n");
		DEBUG_EVENT("         Reconfiguring device %s to new span number # %d\n",
						wp->devname,wp->span.spanno);
		DEBUG_EVENT("\n");
		wp->spanno = wp->span.spanno-1;
	}else{
		DEBUG_EVENT("%s: Wanpipe device is registered to Zaptel span # %d!\n", 
					wp->devname, wp->span.spanno);
	}
	wp_tdmv_generate_bit_rev_table();
	if (wp->channelized == WAN_FALSE && wp->dchan){
		/* Apr 15, 2005
		 * For unchannelized mode, if HW HDLC protocol is selected
		 * by using dchan configuration option, remove dchan timeslot
		 * from timeslot and echo map. */
		wan_clear_bit(wp->dchan-1, &wp->timeslot_map);
		wan_clear_bit(wp->dchan-1, &wp->echo_off_map);
	}
	wp->max_rxtx_len = wp_tdmv_check_mtu(card, wp->timeslot_map);
	wan_set_bit(WP_TDMV_REGISTER, &wp->flags);
	wan_set_bit(WP_TDMV_SIG_POLL, &wp->flags);

	return 0;
}







/******************************************************************************
** wp_tdmv_startup() - 
**
**	OK
*/
static int wp_tdmv_startup(struct zt_span *span)
{
	wp_tdmv_softc_t*	wp = NULL;
	int			i;

	WAN_ASSERT2(span == NULL, -ENODEV);
	WAN_ASSERT2(span->pvt == NULL, -ENODEV);
	wp		= span->pvt;

	/* initialize the start value for the entire chunk of last ec buffer */
	for(i = 0; i < span->channels; i++){
		memset(wp->ec_chunk1[i],
			ZT_LIN2X(0,&span->chans[i]),ZT_CHUNKSIZE);
		memset(wp->ec_chunk2[i],
			ZT_LIN2X(0,&span->chans[i]),ZT_CHUNKSIZE);
	}


	if (!(span->flags & ZT_FLAG_RUNNING)) {
		/* Only if we're not already going */
		span->flags |= ZT_FLAG_RUNNING;
	}

	return 0;
}

/******************************************************************************
** wp_tdmv_shutdown() - 
**
**	OK
*/
static int wp_tdmv_shutdown(struct zt_span *span)
{
	wp_tdmv_softc_t*	wp = NULL;
	wan_smp_flag_t		flags;

	WAN_ASSERT2(span == NULL, -ENODEV);
	WAN_ASSERT2(span->pvt == NULL, -ENODEV);
	wp		= span->pvt;
	wan_spin_lock_irq(&wp->lock, &flags);
	span->flags &= ~ZT_FLAG_RUNNING;
	wan_spin_unlock_irq(&wp->lock, &flags);
	return 0;
}

/******************************************************************************
** wp_tdmv_maint() - 
**
**	OK
*/
static int wp_tdmv_maint(struct zt_span *span, int cmd)
{
	wp_tdmv_softc_t	*wp = span->pvt;
	sdla_t		*card = wp->card;
	int		res = 0;
	wan_smp_flag_t	flags;
	
	WAN_ASSERT2(span == NULL, -ENODEV);
	WAN_ASSERT2(span->pvt == NULL, -ENODEV);
	wp		= span->pvt;
	wan_spin_lock_irq(&wp->lock, &flags);
	if (wp->ise1) {
		// FIXME: Support E1
#if 0
		switch(cmd) {
		case ZT_MAINT_NONE:
			DEBUG_EVENT("%s: E1: Set to normal mode (no local/remote loops)\n",
				       wp->card->devname);	
			// FIXME __t1_set_reg(wp,0xa8,0); /* no loops */
			break;
		case ZT_MAINT_LOCALLOOP:
			DEBUG_EVENT("%s: E1: Set to local loopback mode\n",
				       wp->card->devname);	
			// FIXME __t1_set_reg(wp,0xa8,0x40); /* local loop */
			break;
		case ZT_MAINT_REMOTELOOP:
			DEBUG_EVENT("%s: E1: Set to remote loopback mode\n",
				       wp->card->devname);	
			// FIXME __t1_set_reg(wp,0xa8,0x80); /* remote loop */
			break;
		case ZT_MAINT_LOOPUP:
		case ZT_MAINT_LOOPDOWN:
		case ZT_MAINT_LOOPSTOP:
			res = -ENOSYS;
			break;
		default:
			DEBUG_EVENT("%s: E1: Unknown maintenance mode (%d)\n", 
					wp->card->devname, cmd);
			res = -EINVAL;
			break;
		}
#endif
	}else{
		switch(cmd) {
		case ZT_MAINT_NONE:
			DEBUG_EVENT("%s: T1: Set to normal mode (no local/remote loop)\n",
				       wp->card->devname);	
			card->wandev.fe_iface.set_fe_lbmode(
					&wp->card->fe,
					WAN_TE1_DDLB_MODE,
				       	WAN_TE1_DEACTIVATE_LB);
			card->wandev.fe_iface.set_fe_lbmode(
					&wp->card->fe,
					WAN_TE1_LINELB_MODE,
				       	WAN_TE1_DEACTIVATE_LB);
			break;
	    	case ZT_MAINT_LOCALLOOP:
			DEBUG_EVENT("%s: T1: Set to local loopback mode (local/no remote loop)\n",
				       wp->card->devname);	
			card->wandev.fe_iface.set_fe_lbmode(
					&wp->card->fe,
					WAN_TE1_LINELB_MODE,
				       	WAN_TE1_DEACTIVATE_LB);
			card->wandev.fe_iface.set_fe_lbmode(
					&wp->card->fe,
					WAN_TE1_DDLB_MODE,
				       	WAN_TE1_ACTIVATE_LB);
			break;
	    	case ZT_MAINT_REMOTELOOP:
			DEBUG_EVENT("%s: T1: Set to remote loopback mode (no local/remote loop)\n",
				       wp->card->devname);	
			card->wandev.fe_iface.set_fe_lbmode(
					&wp->card->fe,
					WAN_TE1_LINELB_MODE,
				       	WAN_TE1_ACTIVATE_LB);
			card->wandev.fe_iface.set_fe_lbmode(
					&wp->card->fe,
					WAN_TE1_LINELB_MODE,
				       	WAN_TE1_DEACTIVATE_LB);
			break;
	    	case ZT_MAINT_LOOPUP:
			DEBUG_EVENT("%s: T1: Send loopup code\n",
				       wp->card->devname);	
			card->wandev.fe_iface.set_fe_lbmode(
					&wp->card->fe,
					WAN_TE1_TX_LB_MODE,
				       	WAN_TE1_ACTIVATE_LB);
			break;
	    	case ZT_MAINT_LOOPDOWN:
			DEBUG_EVENT("%s: T1: Send loopdown code\n",
				       wp->card->devname);	
			card->wandev.fe_iface.set_fe_lbmode(
					&wp->card->fe,
					WAN_TE1_TX_LB_MODE,
				       	WAN_TE1_DEACTIVATE_LB);
			break;
	    	case ZT_MAINT_LOOPSTOP:
			DEBUG_EVENT("%s: T1: Stop sending loop code\n",
				       wp->card->devname);	
			// FIXME __t1_set_reg(wp,0x30,0);	/* stop sending loopup code */
			break;
	    	default:
			DEBUG_EVENT("%s: T1: Unknown maintenance mode (%d)\n", 
					wp->card->devname, cmd);
			res = -EINVAL;
	   	}
	}
	wan_spin_unlock_irq(&wp->lock, &flags);
	return res;
}

static void wp_tdmv_set_clear(wp_tdmv_softc_t* wp)
{
	WAN_ASSERT1(wp == NULL);

	// FIXME: Add action for this function 
}

/******************************************************************************
** sigstr() - 
**
**	OK	
*/
#if 0
static char *wp_tdmv_sigstr(int sig)
{
	switch (sig) {
		case ZT_SIG_FXSLS:
			return "FXSLS";
		case ZT_SIG_FXSKS:
			return "FXSKS";
		case ZT_SIG_FXSGS:
			return "FXSGS";
		case ZT_SIG_FXOLS:
			return "FXOLS";
		case ZT_SIG_FXOKS:
			return "FXOKS";
		case ZT_SIG_FXOGS:
			return "FXOGS";
		case ZT_SIG_EM:
			return "E&M";
		case ZT_SIG_CLEAR:
			return "Clear";
		case ZT_SIG_HDLCRAW:
			return "HDLCRAW";
		case ZT_SIG_HDLCFCS:
			return "HDLCFCS";
		case ZT_SIG_HDLCNET:
			return "HDLCNET";
		case ZT_SIG_SLAVE:
			return "Slave";
		case ZT_SIG_CAS:
			return "CAS";
		case ZT_SIG_DACS:
			return "DACS";
		case ZT_SIG_SF:
			return "SF (ToneOnly)";
		case ZT_SIG_NONE:
		default:
			return "Unconfigured";
	}

}
#endif

static int wp_tdmv_chanconfig(struct zt_chan *chan, int sigtype)
{
	sdla_t		*card;
	wp_tdmv_softc_t	*wp = NULL;

	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wp	= chan->pvt;
	card	= (sdla_t*)wp->card;

	if (chan->span->flags & ZT_FLAG_RUNNING){
		wp_tdmv_set_clear(wp);
	}

	if (!wan_test_and_set_bit(chan->chanpos, &wp->config_tsmap)){
		wp->timeslots++;
	}
	if (!(sigtype & ZT_SIG_CLEAR)){
		/* Set Signalling channel map */
		if (!wan_test_and_set_bit(chan->chanpos-1, &wp->sig_timeslot_map)){
			wp->sig_timeslots ++;
			wan_set_bit(chan->chanpos-1, &wp->rbs_rx_pending);
		}
		wp_tdmv_sigctrl(card, wp, WP_TDMV_ENABLE);
	}
	return 0;
}

/******************************************************************************
** wp_tdmv_spanconfig() - 
**
**	OK
*/
static int wp_tdmv_spanconfig(struct zt_span *span, struct zt_lineconfig *lc)
{
	wp_tdmv_softc_t	*wp = NULL;
	sdla_t		*card = NULL;
	int		err = 0;

	WAN_ASSERT2(span == NULL, -ENODEV);
	WAN_ASSERT2(span->pvt == NULL, -ENODEV);
	wp		= span->pvt;
	card = (sdla_t*)wp->card;
	switch(wp->lcode){
	case WAN_LCODE_AMI:
		span->lineconfig |= ZT_CONFIG_AMI;
		break;			
	case WAN_LCODE_B8ZS:
		span->lineconfig |= ZT_CONFIG_B8ZS;
		break;			
	case WAN_LCODE_HDB3:
		span->lineconfig |= ZT_CONFIG_HDB3;
		break;	
	}		
	switch(wp->frame){
	case WAN_FR_ESF:
		span->lineconfig |= ZT_CONFIG_ESF;
		break;			
	case WAN_FR_D4:
		span->lineconfig |= ZT_CONFIG_D4;
		break;			
	case WAN_FR_CRC4:
		span->lineconfig |= ZT_CONFIG_CRC4;
		break;	
	}
	if (wp->ise1){
	       if (lc->lineconfig & ZT_CONFIG_CCS){
			span->lineconfig |= ZT_CONFIG_CCS;
			card->fe.fe_cfg.cfg.te_cfg.sig_mode = WAN_TE1_SIG_CCS;
		}else{
			card->fe.fe_cfg.cfg.te_cfg.sig_mode = WAN_TE1_SIG_CAS;
		}
		card->wandev.fe_iface.post_config(&card->fe);
	}
	span->txlevel = 0;
	switch(wp->lbo){
	case WAN_T1_LBO_0_DB:  
       		DEBUG_TE1("%s: LBO 0 dB\n", card->devname);
		span->txlevel = 0;
		break;
	case WAN_T1_LBO_75_DB:
       		DEBUG_TE1("%s: LBO 7.5 dB\n", card->devname);
		span->txlevel = 5;
		break;
	case WAN_T1_LBO_15_DB: 
       		DEBUG_TE1("%s: LBO 15 dB\n", card->devname);
		span->txlevel = 6;
		break;
 	case WAN_T1_LBO_225_DB:
       		DEBUG_TE1("%s: LBO 22.5 dB\n", card->devname);
		span->txlevel = 7;
		break;
	case WAN_T1_0_110:    
        	DEBUG_TE1("%s: LBO 0-110 ft.\n", card->devname);
		span->txlevel = 0;
		break;
 	case WAN_T1_110_220:   
       		DEBUG_TE1("%s: LBO 110-220 ft.\n", card->devname);
		span->txlevel = 1;
		break;
 	case WAN_T1_220_330:   
       		DEBUG_TE1("%s: LBO 220-330 ft.\n", card->devname);
		span->txlevel = 2;
		break;
 	case WAN_T1_330_440:   
       		DEBUG_TE1("%s: LBO 330-440 ft.\n", card->devname);
		span->txlevel = 3;
		break;
 	case WAN_T1_440_550:   
       		DEBUG_TE1("%s: LBO 440-550 ft.\n", card->devname);
		span->txlevel = 3;
		break;
 	case WAN_T1_550_660:  
       		DEBUG_TE1("%s: LBO 550-660 ft.\n", card->devname);
		span->txlevel = 4;
		break;
	}

	span->rxlevel = 0;
	/* Do we want to SYNC on receive or not */
	wp->sync = lc->sync;
	/* */
	/* If already running, apply changes immediately */
	if (span->flags & ZT_FLAG_RUNNING){
		err = wp_tdmv_startup(span);
	}

	return err;
}


/******************************************************************************
** wp_tdmv_rbsbits() - Set A,B bits according TDM Voice requests. 
**
**	DONE
*/
static int wp_tdmv_rbsbits(struct zt_chan *chan, int bits)
{
	wp_tdmv_softc_t	*wp = NULL;
	sdla_t		*card = NULL;
	unsigned char	ABCD_bits = 0x00;
	
	/* Byte offset */
	WAN_ASSERT2(chan == NULL, 0);
	if ((wp = chan->pvt) == NULL) return 0;
	WAN_ASSERT2(wp->card == NULL, 0);
	card = (sdla_t*)wp->card;
	if (!test_bit(chan->chanpos-1, &wp->timeslot_map)){
		return 0;	
	}
   	if (!wan_test_bit(WP_TDMV_SIG_ENABLE, &wp->flags)){
                return 0;
        }
	if (bits & ZT_ABIT) ABCD_bits |= BIT_TPSC_SIGBYTE_A;
	if (bits & ZT_BBIT) ABCD_bits |= BIT_TPSC_SIGBYTE_B;
	if (bits & ZT_CBIT) ABCD_bits |= BIT_TPSC_SIGBYTE_C;
	if (bits & ZT_DBIT) ABCD_bits |= BIT_TPSC_SIGBYTE_D;
#if 0
	if (wp->ise1){
		FIXME_MSG("wp_tdmv_rbsbits");
		if (chan->chanpos > 15){
			mask = (bits | (wp->chans[chan->chanpos-15].txsig<<4));
			DEBUG_EVENT("%s: E1 Timeslot %d mask=%02x\n", 
					__FUNCTION__,chan->chanpos, mask);
			//__t1_set_reg(wp, 0x41 + chan->chanpos - 15, mask);
		}else if (chan->chanpos < 15){
			mask = ((bits<<4) | wp->chans[chan->chanpos+15].txsig);
			DEBUG_EVENT("%s: E1 Timeslot %d mask=%02x\n",
				       __FUNCTION__,chan->chanpos, mask);
			//__t1_set_reg(wp, 0x41 + chan->chanpos, mask);
		} 
		wp->chans[chan->chanpos - 1].txsig = bits;
	}else{
#endif

	if (chan->flags & ZT_FLAG_HDLC){
		return 0;
	}
#if 0
	DEBUG_EVENT("[TDMV] %s: %s:%02d(%d) TX RBS: A:%1d B:%1d C:%1d D:%1d\n", 
			wp->devname, 
			(wp->ise1) ? "E1" : "T1",
			chan->chanpos, chan->channo,
			(ABCD_bits & BIT_TPSC_SIGBYTE_A) ? 1 : 0,
			(ABCD_bits & BIT_TPSC_SIGBYTE_B) ? 1 : 0,
			(ABCD_bits & BIT_TPSC_SIGBYTE_C) ? 1 : 0,
			(ABCD_bits & BIT_TPSC_SIGBYTE_D) ? 1 : 0);
#endif

	if (wan_test_and_set_bit(chan->chanpos-1, &wp->rbs_tx_status)){
		if (ABCD_bits == wp->rbs_tx[chan->chanpos-1]){
			return 0;
		}
		if (wan_test_and_set_bit(chan->chanpos-1, &wp->rbs_tx1_status)){
			if (ABCD_bits == wp->rbs_tx1[chan->chanpos-1]){
				return 0;
			}
			DEBUG_EVENT("%s: Critical Error: TX RBS for channel %d\n",
						wp->devname,
						chan->chanpos);
		}
		wp->rbs_tx1[chan->chanpos-1] = ABCD_bits;
	}else{
		wp->rbs_tx[chan->chanpos-1] = ABCD_bits;
	}
#if 0
	wan_set_bit(7, &ABCD_bits);
	if (wan_test_and_set_bit(7, &wp->rbs_tx[chan->chanpos-1])){
		if (ABCD_bits == wp->rbs_tx[chan->chanpos-1]){
			return 0;
		}
		if (wan_test_and_set_bit(7, &wp->rbs_tx1[chan->chanpos-1])){
			if (ABCD_bits == wp->rbs_tx1[chan->chanpos-1]){
				return 0;
			}
			DEBUG_EVENT("%s: Critical Error: TX RBS for channel %d\n",
						wp->devname,
						chan->chanpos);
		}
		wp->rbs_tx1[chan->chanpos-1] = ABCD_bits;
	}else{
		wp->rbs_tx[chan->chanpos-1] = ABCD_bits;
	}
#endif
	return 0;
}


/******************************************************************************
** wp_tdmv_is_rbsbits() -
**
**	Returns: 1 - start RBS poll routine, 0 - otherwise
*/
int wp_tdmv_is_rbsbits(wan_tdmv_t *wan_tdmv)
{
	wp_tdmv_softc_t	*wp = NULL;
	
	WAN_ASSERT(wan_tdmv->sc == NULL);
	wp = wan_tdmv->sc;

	/* Do not read signalling bits if Asterisk not configured to */
   	if (!IS_TDMV_SIG(wp)){
		return 0;
	}

	if (wan_test_and_set_bit(WP_TDMV_RBS_BUSY, &wp->flags)){
		/* RBS read still in progress or not ready*/
		return 0;
	}

	if (wp->rbs_tx_status || wp->rbs_tx1_status){
		return 1;
	}

	if (!IS_TDMV_UP(wp)){
		wan_clear_bit(WP_TDMV_RBS_BUSY, &wp->flags);
		return 0;
	}

	/* Increment RX/TX interrupt counter */	
	wp->rbscount++;

	/* RBS_POLL
	** Update RBS bits now (we don't have to do very often) */
	if (!(wp->rbscount & 0xF)){
		return 1;
	}

	/* Wait for the next time */
	wan_clear_bit(WP_TDMV_RBS_BUSY, &wp->flags);
	return 0;
}

/******************************************************************************
** wp_tdmv_read_rbsbits() -
**
**	DONE
*/
int wp_tdmv_rbsbits_poll(wan_tdmv_t *wan_tdmv, void *card1)
{
	sdla_t		*card = (sdla_t*)card1;
	wp_tdmv_softc_t	*wp = NULL;
	int 		i, x;
	
	WAN_ASSERT(wan_tdmv->sc == NULL);
	wp = wan_tdmv->sc;

	/* TX rbsbits */
	if (wp->rbs_tx_status || wp->rbs_tx1_status){
		wp_tdmv_tx_rbsbits(wp);
	}
	
   	if (!IS_TDMV_UP(wp)){
		wan_clear_bit(WP_TDMV_RBS_BUSY, &wp->flags);
		return 0;
	}
	if (wp->rbs_rx_pending){
		DEBUG_TEST("%s: %s:%d: Reading RBS (pending)\n", 
				wp->devname,
				__FUNCTION__,__LINE__);
		for(i=0; i < wp->max_timeslots;i++){
			if (wan_test_bit(i, &wp->rbs_rx_pending)){
				wan_clear_bit(i, &wp->rbs_rx_pending);
				card->wandev.fe_iface.read_rbsbits(
						&card->fe, 
						i+1,
						1);
			}
		}
		wan_set_bit(WP_TDMV_RBS_UPDATE, &wp->flags);
		return 0;
	}

	/* RX rbsbits */
	DEBUG_TEST("%s: %s:%d: Reading RBS (%s)\n", 
				wp->devname,
				__FUNCTION__,__LINE__,
				(wp->rbscount % 1000) ? "Normal" : "Sanity");
	if (wp->rbscount % 1000 == 0){
		for(x = 0; x < wp->max_timeslots; x++){
			if (wan_test_bit(x, &wp->sig_timeslot_map)){
				card->wandev.fe_iface.read_rbsbits(
						&card->fe,
						x+1, 0);
			}
		}
	}else{
		if (card->wandev.fe_iface.check_rbsbits == NULL){
                	DEBUG_EVENT("%s: Internal Error [%s:%d]!\n",
					wp->devname,
					__FUNCTION__,__LINE__); 
			return -EINVAL;
		}
		card->wandev.fe_iface.check_rbsbits(
				&card->fe, 
				1, wp->sig_timeslot_map, 0);
		card->wandev.fe_iface.check_rbsbits(
				&card->fe, 
				9, wp->sig_timeslot_map, 0);
		card->wandev.fe_iface.check_rbsbits(
				&card->fe, 
				17, wp->sig_timeslot_map, 0);
		if (wp->ise1){	
			card->wandev.fe_iface.check_rbsbits(
					&card->fe, 
					25, wp->sig_timeslot_map, 0);
		}
	}
	wan_set_bit(WP_TDMV_RBS_UPDATE, &wp->flags);
	return 0;
}

/******************************************************************************
** wp_tdmv_tx_rbsbits() -
**
**	DONE
*/
static int wp_tdmv_tx_rbsbits(wp_tdmv_softc_t *wp)
{
	sdla_t	*card;
	int	x;

	WAN_ASSERT2(wp->card == NULL, 0);
	card = (sdla_t*)wp->card;
	for(x=0;x<wp->max_timeslots;x++){
		if (wan_test_bit(x, &wp->rbs_tx_status)){
			card->wandev.fe_iface.set_rbsbits(
					&wp->card->fe, 
					x+1, 
					wp->rbs_tx[x]);
			wan_clear_bit(x, &wp->rbs_tx_status);
			if (wan_test_bit(x, &wp->rbs_tx1_status)){
				card->wandev.fe_iface.set_rbsbits(
						&wp->card->fe, 
						x+1, 
						wp->rbs_tx1[x]);
				wan_clear_bit(x, &wp->rbs_tx1_status);
			}
		}
	}
#if 0	
	for(x=0;x<wp->max_timeslots;x++){
		if (wan_test_bit(7, &wp->rbs_tx[x])){
			card->wandev.fe_iface.set_rbsbits(
					&wp->card->fe, 
					x+1, 
					wp->rbs_tx[x]);
			wan_clear_bit(7, &wp->rbs_tx[x]);
			if (wan_test_bit(7, &wp->rbs_tx1[x])){
				card->wandev.fe_iface.set_rbsbits(
						&wp->card->fe, 
						x+1, 
						wp->rbs_tx1[x]);
				wan_clear_bit(7, &wp->rbs_tx1[x]);
			}
		}
	}
#endif
	return 0;
}

/******************************************************************************
** wp_tdmv_ioctl() - 
**
**	OK
*/
static int 
#if defined(__FreeBSD__)
wp_tdmv_ioctl(struct zt_chan *chan, unsigned int cmd, caddr_t data)
#else
wp_tdmv_ioctl(struct zt_chan *chan, unsigned int cmd, unsigned long data)
#endif
{
	int err = -ENOTTY;
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN)
	wp_tdmv_softc_t	*wp = NULL;
#endif
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
	wp_tdmv_softc_t	*echo_detect_wp	= NULL;
	int echo_detect_chan = 0;
#endif

	switch(cmd) 
	{
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN)
	case ZT_DCHAN_TX:

		WAN_ASSERT(chan == NULL || chan->pvt == NULL);
		wp = chan->pvt;	

		if (wp->dchan_dev && wp->dchan_dev->hard_start_xmit){
			wp_tdmv_tx_dchan(chan, (int)data);
			err=0;
		}else{
			err=-EOPNOTSUPP;
		}
		break;
#endif
		
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
	case SANGOMA_GET_ED_STATE:
		DEBUG_ECHO("%s():SANGOMA_GET_ED_STATE\n", __FUNCTION__);

		WAN_ASSERT(chan == NULL || chan->pvt == NULL);

		echo_detect_wp	= chan->pvt;
		echo_detect_chan = chan->chanpos - 1;
		
		DEBUG_ECHO("on span: %d, chanpos: %d\n", echo_detect_wp->spanno, 
			echo_detect_chan);
			
		if(echo_detect_chan > 30 || echo_detect_chan < 0){
			err=-EOPNOTSUPP;
		}else{
			wan_tdmv_t *wan_tdmv = &echo_detect_wp->card->wan_tdmv;
			wan_tdmv_rxtx_pwr_t *pwr_rxtx = &wan_tdmv->chan_pwr[echo_detect_chan];
	
			DEBUG_ECHO("%s():using %s table.\n", __FUNCTION__,
				(chan->xlaw == __zt_mulaw ? "MULAW" : "ALAW"));
						
			/* This will be used when reporting Echo Cancellation state 
			   from Asterisk CLI.
			*/
			chan->echo_detect_struct.echo_state = pwr_rxtx->current_state;
			
			DEBUG_ECHO("echo_state:%s\n",
				TDMV_SAMPLE_STATE_DECODE(chan->echo_detect_struct.echo_state));

			chan->echo_detect_struct.echo_present_samples_number =
				pwr_rxtx->echo_present_samples_number_history;
			chan->echo_detect_struct.echo_absent_samples_number = 
				pwr_rxtx->echo_absent_samples_number_history;
		}

		err = 0;
		break;
#endif /* CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER */
	
	default:
		/*DEBUG_EVENT("%s(): uknown cmd!\n", __FUNCTION__);*/
		err = -ENOTTY;
		break;
	}
	return err;
}


/******************************************************************************
** wp_tdmv_open() - 
**
**	OK
*/
static int wp_tdmv_open(struct zt_chan *chan)
{
	wp_tdmv_softc_t	*wp = NULL;
	sdla_t		*card = NULL;
	
	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wp = chan->pvt;
	WAN_ASSERT2(wp->card == NULL, -ENODEV);
	card = wp->card;
	wp->usecount++;
	if (wp->usecount == wp->timeslots){
		wan_set_bit(WP_TDMV_RUNNING, &wp->flags);
	}
	DEBUG_TDMV("%s: Open (usecount=%d, channo=%d, chanpos=%d)...\n", 
				wp->devname,
				wp->usecount,
				chan->channo,
				chan->chanpos);
	return 0;
}


/******************************************************************************
** wp_tdmv_close() - 
**
**	OK
*/
static int wp_tdmv_close(struct zt_chan *chan)
{
	wp_tdmv_softc_t*	wp = NULL;
	
	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wp = chan->pvt;
	wp->usecount--;
	wan_clear_bit(WP_TDMV_RUNNING, &wp->flags);
	DEBUG_TDMV("%s: Close (usecount=%d, channo=%d, chanpos=%d)...\n", 
				wp->devname,
				wp->usecount,
				chan->channo,
				chan->chanpos);
	return 0;
}

/******************************************************************************
** wp_tdmv_release() - 
**
**	OK
*/
static void wp_tdmv_release(wp_tdmv_softc_t *wp)
{
	WAN_ASSERT1(wp == NULL);
	if (wan_test_bit(WP_TDMV_REGISTER, &wp->flags)){
		DEBUG_EVENT("%s: Unregister Wanpipe device from Zaptel!\n",
				wp->devname);
		wan_clear_bit(WP_TDMV_SIG_POLL, &wp->flags);
		wan_clear_bit(WP_TDMV_REGISTER, &wp->flags);
		zt_unregister(&wp->span);
		wan_clear_bit(WP_TDMV_REGISTER, &wp->flags);
	}
	wan_free(wp);
}

static inline void start_alarm(wp_tdmv_softc_t* wp)
{
	WAN_ASSERT1(wp == NULL);
#ifdef FANCY_ALARM
	wp->alarmpos = 0;
#endif
	wp->blinktimer = 0;
}

static inline void stop_alarm(wp_tdmv_softc_t* wp)
{
	WAN_ASSERT1(wp == NULL);
#ifdef FANCY_ALARM
	wp->alarmpos = 0;
#endif
	wp->blinktimer = 0;
}

/************************************************************************************
 * Channelized code for rx/tx
 * *********************************************************************************/

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN)
/******************************************************************************
** wp_tdmv_rx_dchan() - 
**
**	OK
*/
int wp_tdmv_rx_dchan(wan_tdmv_t *wan_tdmv, int channo, 
			unsigned char *rxbuf, unsigned int len)
{
	wp_tdmv_softc_t	*wp = wan_tdmv->sc;
	struct zt_chan	*chan = NULL, *ms = NULL;
	wan_smp_flag_t	smp_flags;
	unsigned char	*buf = NULL;
	int		oldbuf;
	int 		i, left;

	WAN_ASSERT(wp == NULL);
	WAN_ASSERT(channo != wp->dchan-1);
	chan	= &wp->chans[wp->dchan-1];
	WAN_ASSERT(chan == NULL || chan->master == NULL);
	ms = chan->master;
	
	if (!IS_TDMV_UP(wp)){
		DEBUG_TDMV("%s: Asterisk is not running!\n",
				wp->devname); 
		return -EINVAL;
	}
	if (!(ms->flags & ZT_FLAG_HDLC)){
		DEBUG_TDMV("%s: ERROR: %s not defined as D-CHAN!\n",
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
#elif defined(__FreeBSD__)
		wakeup(&ms->readbufq);
		wakeup(&ms->sel);
		if (ms->iomask & ZT_IOMUX_READ)
			wakeup(&ms->eventbufq);

#endif
	}
	return 0;
}
#endif

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN)
static int wp_tdmv_tx_dchan(struct zt_chan *chan, int len)
{
	wp_tdmv_softc_t	*wp = NULL;
	netskb_t	*skb = NULL;
	wan_smp_flag_t	smp_flags;
	unsigned char	*data = NULL;
	int		err = 0;

	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wp	= chan->pvt;
	WAN_ASSERT(wp->dchan_dev == NULL);

	wan_spin_lock_irq(&chan->lock, &smp_flags);
	if (len <= 2){
		wan_spin_unlock_irq(&chan->lock, &smp_flags);
		return -EINVAL;
	}
	len -= 2; /* Remove checksum */
	skb = wan_skb_alloc(len+1);
	if (skb == NULL){
		wan_spin_unlock_irq(&chan->lock, &smp_flags);
		return -ENOMEM;
	}
	data = wan_skb_put(skb, len);
	memcpy(data, chan->writebuf[chan->inwritebuf], len);
#if 0
	{
	int i;
	DEBUG_EVENT("TX DCHAN: ");
	for(i = 0; i < len; i++){
		_DEBUG_EVENT("%02X ", data[i]);
	}
	_DEBUG_EVENT("\n");
	}
#endif
	wan_spin_unlock_irq(&chan->lock, &smp_flags);
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
** wp_tdmv_rx_chan() - 
**
**	OK
*/
int wp_tdmv_rx_chan(wan_tdmv_t *wan_tdmv, int channo, 
			unsigned char *rxbuf,
			unsigned char *txbuf)
{
	wp_tdmv_softc_t	*wp = wan_tdmv->sc;
	
	WAN_ASSERT2(wp == NULL, -EINVAL);
	WAN_ASSERT2(channo < 0, -EINVAL);
	WAN_ASSERT2(channo > 31, -EINVAL);

	if (!IS_TDMV_UP(wp)){
		return -EINVAL;
	}

	if (wp->channelized == WAN_TRUE){
		int	y;
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
		wan_tdmv_rxtx_pwr_t *pwr_rxtx = &wan_tdmv->chan_pwr[channo];
#endif

		wp->chans[channo].readchunk = rxbuf;	
		wp->chans[channo].writechunk = txbuf;	

		if (wan_tdmv->brt_enable){
			for (y=0;y<ZT_CHUNKSIZE;y++){
				wp->chans[channo].readchunk[y] = 
					brt[wp->chans[channo].readchunk[y]];
			}
		}

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
		wp_tdmv_echo_check(wan_tdmv, &wp->chans[channo], channo);
#endif		

		if (!wan_test_bit(channo, &wp->echo_off_map)){
/*Echo spike starts at 25bytes*/
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
			if(pwr_rxtx->current_state != ECHO_ABSENT){
#endif
#if 0
/* Echo spike starts at 16 bytes */	

				zt_ec_chunk(
					&wp->chans[channo], 
					wp->chans[channo].readchunk, 
					wp->chans[channo].writechunk);
#endif

#if 1			
/*Echo spike starts at 9 bytes*/
				zt_ec_chunk(
					&wp->chans[channo], 
					wp->chans[channo].readchunk, 
					wp->ec_chunk1[channo]);
				memcpy(
					wp->ec_chunk1[channo],
					wp->chans[channo].writechunk,
					ZT_CHUNKSIZE);
#endif

#if 0			
/*Echo spike starts at bytes*/
				zt_ec_chunk(
					&wp->chans[channo], 
					wp->chans[channo].readchunk, 
					wp->ec_chunk1[channo]);
				memcpy(
					wp->ec_chunk1[channo],
					wp->ec_chunk2[channo],
					ZT_CHUNKSIZE);

				memcpy(
					wp->ec_chunk2[channo],
					wp->chans[channo].writechunk,
					ZT_CHUNKSIZE);
#endif

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
			}//if(pwr_rxtx->current_state != ECHO_ABSENT)
#endif
		}//if (!wan_test_bit(channo, &wp->echo_off_map))
	}else{
		int	x, channels = wp->span.channels;

		wp->tx_unchan = txbuf;
		wp_tdmv_rxcopy(wan_tdmv, rxbuf, wp->max_rxtx_len);

		if (!wp->echo_off_map){
			for (x = 0; x < channels; x++){
#if 0
				//This code never runs. Instead wp_tdmv_rx_chan() is called for A101/A102.
				//All Echo Detection is done there for A101/A102.
				wp_tdmv_echo_check(wan_tdmv, &wp->chans[0], x);
#endif

				zt_ec_chunk(
					&wp->chans[x], 
					wp->chans[x].readchunk, 
					wp->chans[x].writechunk);

#if 0
				zt_ec_chunk(
					&wp->chans[x], 
					wp->chans[x].readchunk, 
					wp->ec_chunk1[x]);
				memcpy(
					wp->ec_chunk1[x],
					wp->chans[x].writechunk,
					ZT_CHUNKSIZE);
#endif
			}
		}/* if() */
	}/* if() */

	return 0;
}

/******************************************************************************
** wp_tdmv_rx_tx_span() - 
**
**	OK
*/
int wp_tdmv_rx_tx_span(void *pcard)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t	*wp = NULL;
	
	WAN_ASSERT(wan_tdmv->sc == NULL);
	wp = wan_tdmv->sc;

	wp->intcount++;
	zt_receive(&wp->span);
	zt_transmit(&wp->span);
	
	if (wp->channelized == WAN_FALSE){
		wp_tdmv_txcopy(wan_tdmv,
				wp->tx_unchan,
				wp->max_rxtx_len);
	}

	if (wan_test_bit(WP_TDMV_RBS_UPDATE, &wp->flags)){
		DEBUG_TEST("%s: %s:%d: Updating RBS status \n",
				wp->devname,
				__FUNCTION__,__LINE__);
		if (card->wandev.fe_iface.report_rbsbits){
			card->wandev.fe_iface.report_rbsbits(&card->fe);
		}
		wan_clear_bit(WP_TDMV_RBS_UPDATE, &wp->flags);
		wan_clear_bit(WP_TDMV_RBS_BUSY, &wp->flags);
	}
	return 0;
}


/******************************************************************************
** wp_tdmv_init() - 
*
**	OK
*/
int wp_tdmv_init(void* pcard, wanif_conf_t *conf)
{
	return -EINVAL;
}


#undef WANPIPE_CODEC_CONVERTER

#ifdef WANPIPE_CODEC_CONVERTER

#define ENABLE_BRT 1

#ifdef ENABLE_BRT
static int codec_init=0;
#endif
int wanpipe_codec_convert_2s(u8 *data, int len, netskb_t *nskb, u16 *power_ptr, int is_alaw)
{
	int i;
	u16 	*buf;
	u16 	power=0;
	short   *codec;
	unsigned short *pcodec;
	u8	brtdata;

#ifdef ENABLE_BRT
	if (codec_init==0){
		wp_tdmv_generate_bit_rev_table();
		codec_init=1;
	}
#endif

	if (is_alaw == 0){
		codec=__zt_mulaw;	
		pcodec=u2s;
	}else{
		codec=__zt_alaw;
		pcodec=a2s;
	}

	for (i=0;i<len;i++){
		buf=(short *)wan_skb_put(nskb,2);
#ifdef ENABLE_BRT
		brtdata=brt[data[i]];
#else
		brtdata=data[i];
#endif
		buf[0]=codec[ brtdata ];	
		power+=pcodec[ brtdata & 0x7F ];		
	}

	power = (u16)(power/len);

	if (power_ptr){
		*power_ptr=power;
	}

	return 0;
}


int wanpipe_codec_convert_s2ulaw(netskb_t *skb, netskb_t *nskb, int is_alaw)
{	
	int i;
	u8 *buf;
	int len = wan_skb_len(skb);
	u16 *data = (u16*)wan_skb_data(skb);

	len=len/2;

	for (i=0;i<len;i++){
		buf=(u8 *)wan_skb_put(nskb,1);
		buf[0]=__zt_lin2mu[data[i]>>2];	
#ifdef ENABLE_BRT
		buf[0]=brt[buf[0]];
#endif	
	}	

	return 0;
}

#endif
