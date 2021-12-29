/*****************************************************************************
 * sdla_tdmv.c	WANPIPE(tm) Multiprotocol WAN Link Driver. 
 *				TDM voice board configuration.
 *
 * Author: 	Alex Feldman  <al.feldman@sangoma.com>
 *
 * Copyright:	(c) 1995-2001 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 * Jul 22, 2001	Nenad Corbic	Initial version.
 * Oct 01, 2001 Gideon Hack	Modifications for interrupt usage.
 ******************************************************************************
 */


/*
 ******************************************************************************
			   INCLUDE FILES
 ******************************************************************************
*/

#if (defined __FreeBSD__) | (defined __OpenBSD__)
# include <net/wanpipe_config.h>
# include <net/wanpipe_includes.h>
# include <net/wanpipe.h>
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
# include <linux/wanpipe.h>	/* WANPIPE common user API definitions */
#endif

/*
 ******************************************************************************
			  DEFINES AND MACROS
 ******************************************************************************
*/

#define WP_MAX_CARDS	32

#define FIXME_MSG(func)		DEBUG_EVENT("(%s): FIXME: line %d\n", func, __LINE__)
#define DBG_FUNC_START(func)	DEBUG_EVENT("(DEBUG)(TDM Voice): %s - Start\n", func)
#define DBG_FUNC_END(func)	DEBUG_EVENT("(DEBUG)(TDM Voice): %s - End\n", func)

#define WP_TDMV_STOP			0x01
#define WP_TDMV_RUNNING			0x02
#define WP_TDMV_UP			0x04

#define WP_SIG_NONE			0x00
#define WP_SIG_CONFIGURING		0x01
#define WP_SIG_CONFIGURED		0x02

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
	spinlock_t	lock;
	spinlock_t	tx_rx_lock;
	int		ise1;
	int		num;
	int		flags;
	int		lbo;
	int 		lcode;
	int 		frame;
	int		sig_ready;
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
	unsigned long	timeslot_map;
	int		timeslots;
	int		first_channo;
	int 		intcount;
	unsigned int	brt_ctrl;
	unsigned char	echo_off;
} wp_tdmv_softc_t;


/*
*******************************************************************************
**			   GLOBAL VARIABLES
*******************************************************************************
*/
static unsigned long	wp_cards_tdmv_map = 0;
static int		wp_last_channo = 0;
static unsigned char	brt[256];

/*
*******************************************************************************
**			  FUNCTION PROTOTYPES
*******************************************************************************
*/
static int wp_tdmv_software_init(wp_tdmv_softc_t* wp);
static int wp_tdmv_startup(struct zt_span *span);
static int wp_tdmv_shutdown(struct zt_span *span);
static int wp_tdmv_maint(struct zt_span *span, int cmd);
static int wp_tdmv_chanconfig(struct zt_chan *chan, int sigtype);
static int wp_tdmv_spanconfig(struct zt_span *span, struct zt_lineconfig *lc);
static int wp_tdmv_sigenable(sdla_t* card, wp_tdmv_softc_t *wp);
static int wp_tdmv_rbsbits(struct zt_chan *chan, int bits);
static int wp_tdmv_open(struct zt_chan *chan);
static int wp_tdmv_close(struct zt_chan *chan);
static void wp_tdmv_release(wp_tdmv_softc_t *wp);
static int wp_tdmv_ioctl(struct zt_chan *chan, unsigned int cmd, unsigned long data);
static int wp_tdmv_receiveprep(wan_tdmv_t* wan_tdmv, unsigned char*, int);
static int  wp_tdmv_transmitprep(wan_tdmv_t* wan_tdmv, unsigned char*, int);

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
** wp_tdmv_init() - 
**
**	OK
*/
int wp_tdmv_init(void* pcard, wanif_conf_t *conf)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t	*wp = NULL;
	int		err;
	
	memset(&card->wan_tdmv, 0x0, sizeof(wan_tdmv_t));
	/* We are forcing to register wanpipe devices at the same sequence
	 * that it defines in /etc/zaptel.conf */
	/*if (conf->spanno && conf->spanno != wp_cards_tdmv_no + 1){*/
	if (conf->spanno){
	       if (wan_test_bit(conf->spanno-1, &wp_cards_tdmv_map)){
			DEBUG_EVENT("%s: Registering device with an incorrect span number!\n",
						card->devname);
			DEBUG_EVENT("%s: Another wanpipe device already configured to span #%d!\n",
						card->devname, conf->spanno);
			return -EINVAL;
	       }
	}else{
		int	i = 1;
		for(i = 1; i <= 32; i++){
			if (!wan_test_bit(i-1, &wp_cards_tdmv_map)){
				conf->spanno = i;
				break;
			}
		}
	}

	card->wan_tdmv.max_timeslots = GET_TE_CHANNEL_RANGE(&card->fe);
	card->wan_tdmv.max_tx_len = card->wandev.mtu;
	card->wandev.te_report_rbsbits = NULL;
	card->wandev.te_report_alarms = wp_tdmv_report_alarms;	

	wp = wan_malloc(sizeof(wp_tdmv_softc_t));
	if (wp == NULL){
		DEBUG_EVENT("%s: %s:%d: Error: Failed to allocate Memory\n",
				card->devname,__FUNCTION__,__LINE__);
		err = -ENOMEM;
		goto wp_tdmv_init_error;
	}
	memset(wp, 0x0, sizeof(wp_tdmv_softc_t));

	wp->num			= conf->spanno-1;
	wp->card		= card;
	wp->devname		= card->devname;
	wp->lcode		= card->fe.fe_cfg.lcode;
	wp->frame		= card->fe.fe_cfg.frame;
	wp->lbo			= card->fe.fe_cfg.cfg.te_cfg.lbo;
	wp->timeslot_map	= conf->active_ch;
	wp->ise1		= IS_E1_CARD(card) ? 1 : 0;
	wp->echo_off		= conf->tdmv_echo_off;
	if (wp->ise1){
		/* Timeslot map for E1 includes ts0. TDM voice does never
		** use ts0, so i will shift map 1 bit right to get 
		** everything alignment as T1 */
		wp->timeslot_map = wp->timeslot_map >> 1;
	}
	wan_tdmv->sc = wp;
	spin_lock_init(&wp->lock);
	spin_lock_init(&wp->tx_rx_lock);

	/* Misc. software stuff */
	if ((err = wp_tdmv_software_init(wp))){
		wan_tdmv->sc = NULL;
		wan_free(wp);
		goto wp_tdmv_init_error;
		
	}
	/* Mark this span as used */
	wan_set_bit(wp->num, &wp_cards_tdmv_map);

	if (wp->echo_off){
		DEBUG_EVENT("%s:    TDMV Echo Ctrl:Off\n",
			wp->devname);
	}

	wp->flags = WP_TDMV_RUNNING;
	return 0;

wp_tdmv_init_error:
	wp_tdmv_remove(pcard);
	return err;
}

int wp_tdmv_running(void *pcard)
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
** wp_stop_remove() - 
**
**	OK
*/
int wp_tdmv_remove(void* pcard)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_tdmv_t	*wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t	*wp = NULL;

	wp = wan_tdmv->sc;
	/* Release span, possibly delayed */
	if (wp && wp->usecount){
		DEBUG_EVENT("%s: ERROR: Wanpipe is still used by Asterisk!\n",
				card->devname);
		return -EINVAL;
	}
	wan_tdmv->sc = NULL;
	if (wp){
		wp->flags &= ~(WP_TDMV_RUNNING | WP_TDMV_UP);
		/* Mark this span as unused */
		wan_clear_bit(wp->num, &wp_cards_tdmv_map);
		wp_tdmv_release(wp);
	}
	card->wandev.te_report_rbsbits = NULL;
	card->wandev.te_report_alarms = NULL;	
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
		if (wp->sig_ready == WP_SIG_CONFIGURING){
			wp_tdmv_sigenable(card, wp);
		}
		wp->flags |= WP_TDMV_UP;
		break;

	case WAN_DISCONNECTED:
		wp->flags &= ~WP_TDMV_UP;
		break;
	}
	return;
}

static int wp_tdmv_sigenable(sdla_t* card, wp_tdmv_softc_t *wp)
{
	wp->sig_ready = WP_SIG_CONFIGURED;
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
	if (!(wp->flags & WP_TDMV_RUNNING)) return;
	if (!test_bit(channel-1, &wp->timeslot_map)){
		return;
	}
	if (status & BIT_SIGX_SIGDATA_A) rxs |= ZT_ABIT;
	if (status & BIT_SIGX_SIGDATA_B) rxs |= ZT_BBIT;
	if (status & BIT_SIGX_SIGDATA_C) rxs |= ZT_CBIT;
	if (status & BIT_SIGX_SIGDATA_D) rxs |= ZT_DBIT;
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
		if (wp->chans[i].channo == wp->first_channo + channel)
			break;
	}
	if (i == wp->span.channels){
		return;
	}
//	if (test_bit(wp->chans[i].chanpos-1, &wp->timeslot_map)){
	if (!(wp->chans[i].sig & ZT_SIG_CLEAR) &&
	    (wp->chans[i].rxsig != rxs)){
		zt_rbsbits(&wp->chans[i], rxs);
		DEBUG_TDMV("%s: %s:%d RX RBS: A:%1d B:%1d C:%1d D:%1d\n",
				wp->devname, 
				(wp->ise1) ? "E1" : "T1",
				wp->chans[i].channo,
				(rxs & ZT_ABIT) ? 1 : 0,
				(rxs & ZT_BBIT) ? 1 : 0,
				(rxs & ZT_CBIT) ? 1 : 0,
				(rxs & ZT_DBIT) ? 1 : 0);
	}
}

/******************************************************************************
** wp_tdmv_report_alarms() - 
**
**	DONE
*/
void wp_tdmv_report_alarms(void* pcard, unsigned long te_alarm)
{
	sdla_t*	card = (sdla_t*)pcard;
	wan_tdmv_t*	wan_tdmv = &card->wan_tdmv;
	wp_tdmv_softc_t*	wp = NULL;
	int		alarms;
	int		x,j;

	WAN_ASSERT1(wan_tdmv->sc == NULL);
	wp = wan_tdmv->sc;
	if (!(wp->flags & WP_TDMV_RUNNING)) return;
	/* Assume no alarms */
	alarms = 0;

	/* And consider only carrier alarms */
	wp->span.alarms &= (ZT_ALARM_RED | ZT_ALARM_BLUE | ZT_ALARM_NOTOPEN);

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

	/* If receiving alarms, go into Yellow alarm state */
	if (alarms && (!wp->span.alarms)) {
		DEBUG_TDMV("%s: Going into yellow alarm\n",
				wp->devname);
		if (card->wandev.fe_iface.set_fe_alarm){
			card->wandev.fe_iface.set_fe_alarm(&card->fe, WAN_TE_BIT_YEL_ALARM);
		}
	}

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
	zt_alarm_notify(&wp->span);
}

static int 
wp_tdmv_transmitprep(wan_tdmv_t *wan_tdmv, unsigned char* txbuf, int max_len)
{
	wp_tdmv_softc_t	*wp = wan_tdmv->sc;
	int		x = 0, y = 0, offset = 0;

	WAN_ASSERT2(wp == NULL, 0);
	zt_transmit(&wp->span);
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
** wp_tdmv_receiveprep() - 
**
**	OK
*/
static int 
wp_tdmv_receiveprep(wan_tdmv_t *wan_tdmv, unsigned char* rxbuf, int max_len)
{
	wp_tdmv_softc_t	*wp = wan_tdmv->sc;
	int		x, y, offset = 0;
	int		channels = wp->span.channels;
	unsigned char	value;
	
	WAN_ASSERT2(wp == NULL, 0);
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

	if (wp->echo_off){
		goto wp_skip_echo;
	}
	
	for (x = 0; x < channels; x++){
#if 1
		zt_ec_chunk(
			&wp->chans[x], 
			wp->chans[x].readchunk, 
			wp->chans[x].writechunk);

#endif

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

#if 0
		zt_ec_chunk(
			&wp->chans[x], 
			wp->chans[x].readchunk, 
			wp->ec_chunk2[x]);
		memcpy(
			wp->ec_chunk2[x],
			wp->ec_chunk1[x],
			ZT_CHUNKSIZE);
		memcpy(
			wp->ec_chunk1[x],
			wp->chans[x].writechunk,
			ZT_CHUNKSIZE);
#endif
	}

wp_skip_echo:

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
	int 		len, max_len = wan_tdmv->max_tx_len;
	int 		x;

	WAN_ASSERT(wan_tdmv->sc == NULL);
	WAN_ASSERT(skb == NULL);
	wp = wan_tdmv->sc;

	if (wan_skb_len(skb) != max_len){
                DEBUG_EVENT("%s: Internal Error[%s:%d]: Wrong buffer lenght %d (%d)!\n", 
					wp->devname, 
					__FUNCTION__,__LINE__,
					wan_skb_len(skb),
					max_len);
                return 0;
        }
	if (!(wp->flags & (WP_TDMV_RUNNING|WP_TDMV_UP))){
		return 0;
	}

	wp->intcount ++;

	len = wp_tdmv_receiveprep(
			wan_tdmv, 
			wan_skb_data(skb), 
			wan_skb_len(skb)); 
#if 0
	if (len != max_len){
		/* Should never happened! */
		DEBUG_EVENT("%s: Internal Error [%s:%d]: Wrong RX lenght %d (%d)!\n",
					wp->devname,
					__FUNCTION__, __LINE__,
					len, max_len);	
	}
#endif
	len = wp_tdmv_transmitprep(
			wan_tdmv, 
			wan_skb_data(skb), 
			wan_skb_len(skb)); 
#if 0
	if (len != max_len){
		/* Should never happen! */
		DEBUG_EVENT("%s: Internal Error [%s:%d]: Wrong TX lenght %d (%d)!\n",
					wp->devname,
					__FUNCTION__, __LINE__,
					len, max_len);	
	}
#endif

	/* Do not read signalling bits if Asterisk not configured to */
	if (card->wandev.te_report_rbsbits){
	
		/* RBS_POLL
		** Update RBS bits now (we don't have to do very often) */
		x = wp->intcount & 0xF;
		switch(x) {
		case 0:
		case 1:
		case 2:
			if (card->wandev.fe_iface.read_rbsbits){
				card->wandev.fe_iface.read_rbsbits(
						&card->fe, 
						x * 8 + 1);
				if (wp->ise1 && x == 2){
					card->wandev.fe_iface.read_rbsbits(
						&card->fe, 
						(x+1) * 8 + 1);
				}
			}
			break;
		}
	}

	return max_len;
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
static int wp_tdmv_software_init(wp_tdmv_softc_t* wp)
{
	int 	x = 0;

	WAN_ASSERT(wp == NULL);
	if (wp->ise1){
		sprintf(wp->span.name, "WPE1/%d", wp->num);
	}else{
		sprintf(wp->span.name, "WPT1/%d", wp->num);
	}
	sprintf(wp->span.desc, "%s card %d", wp->card->devname, wp->num);
	wp->span.spanconfig = wp_tdmv_spanconfig;
	wp->span.chanconfig = wp_tdmv_chanconfig;
	wp->span.startup = wp_tdmv_startup;
	wp->span.shutdown = wp_tdmv_shutdown;
	wp->span.rbsbits = wp_tdmv_rbsbits;
	wp->span.maint = wp_tdmv_maint;
	wp->span.open = wp_tdmv_open;
	wp->span.close = wp_tdmv_close;
	wp->span.channels = (wp->ise1) ? 31 : 24;
	wp->span.chans = wp->chans;
	wp->span.flags = ZT_FLAG_RBS;
	wp->span.linecompat = ZT_CONFIG_AMI | ZT_CONFIG_B8ZS | ZT_CONFIG_D4 | ZT_CONFIG_ESF;
	wp->span.ioctl = wp_tdmv_ioctl;
	wp->span.pvt = wp;
	if (wp->ise1)
		wp->span.deflaw = ZT_LAW_ALAW;
	else
		wp->span.deflaw = ZT_LAW_MULAW;
	init_waitqueue_head(&wp->span.maintq);
	wp->first_channo = wp_last_channo;
	wp_last_channo += wp->span.channels;
	for (x=0;x<wp->span.channels;x++) {
		if (wp->ise1){
			sprintf(wp->chans[x].name, "WPE1/%d/%d", wp->num, x+1);
		}else{
			sprintf(wp->chans[x].name, "WPT1/%d/%d", wp->num, x+1);
		}
		if (test_bit(x,&wp->timeslot_map)){
			DEBUG_TEST("%s: Configure channel %d (%d) for voice (%s)!\n", 
					wp->card->devname, 
					x + 1,
					wp->first_channo + x + 1,
					wp->chans[x].name);
			wp->chans[x].sigcap = 
				ZT_SIG_EM | ZT_SIG_CLEAR | ZT_SIG_EM_E1 | 
				ZT_SIG_FXSLS | ZT_SIG_FXSGS | 
				ZT_SIG_FXSKS | ZT_SIG_FXOLS | 
				ZT_SIG_FXOGS | ZT_SIG_FXOKS | 
				ZT_SIG_CAS | ZT_SIG_SF;
			wp->timeslots++;
		}else{
			wp->chans[x].sigcap = ZT_SIG_NONE;
		}
		wp->chans[x].pvt = wp;
		wp->chans[x].chanpos = x + 1;
	}

	DEBUG_EVENT("%s: Registering interface to Zaptel span # %d!\n", 
					wp->devname, wp->num + 1);
	if (zt_register(&wp->span, 0)) {
		DEBUG_EVENT("%s: Unable to register span with zaptel\n",
					wp->devname);
		return -EINVAL;
	}
	wp_tdmv_generate_bit_rev_table();

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
	spin_lock_irqsave(&wp->lock, flags);
	span->flags &= ~ZT_FLAG_RUNNING;
	spin_unlock_irqrestore(&wp->lock, flags);
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
	spin_lock_irqsave(&wp->lock, flags);
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
	spin_unlock_irqrestore(&wp->lock, flags);
	return res;
}

static void wp_tdmv_set_clear(wp_tdmv_softc_t* wp)
{
	WAN_ASSERT1(wp == NULL);
	/* No such thing under E1 */
	if (wp->ise1) {
		DEBUG_EVENT("Can't set clear mode on an E1!\n");
		return;
	}

	// FIXME: Add action for this function 
}

/******************************************************************************
** wp_tdmv_chanconfig() - 
**
**	OK	
*/
#if 0
static char *sigstr(int sig)
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
	sdla_t*		card;
	wp_tdmv_softc_t*	wp = NULL;

	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wp	= chan->pvt;
	card	= (sdla_t*)wp->card;

	if (chan->span->flags & ZT_FLAG_RUNNING){
		wp_tdmv_set_clear(wp);
	}
	//if (sigtype & __ZT_SIG_FXO || sigtype & __ZT_SIG_FXS){
	if (!(sigtype & ZT_SIG_CLEAR)){
		card->wandev.te_report_rbsbits = wp_tdmv_report_rbsbits;
		if (wp->sig_ready == WP_SIG_NONE){
			wp->sig_ready = WP_SIG_CONFIGURING;
		}
		if (wp->flags & WP_TDMV_UP){
			wp_tdmv_sigenable(card, wp);
		}
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
	wp_tdmv_softc_t*	wp = NULL;
	sdla_t*		card = NULL;
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
	switch(wp->lbo){
	case WAN_T1_LBO_0_DB:  
        	DEBUG_TE1("%s: LBO 0 dB\n", card->devname);
		span->lineconfig = 0;
		break;
	case WAN_T1_LBO_75_DB:
        	DEBUG_TE1("%s: LBO 7.5 dB\n", card->devname);
		span->lineconfig = 5;
		break;
	case WAN_T1_LBO_15_DB: 
        	DEBUG_TE1("%s: LBO 15 dB\n", card->devname);
		span->lineconfig = 6;
		break;
 	case WAN_T1_LBO_225_DB:
        	DEBUG_TE1("%s: LBO 22.5 dB\n", card->devname);
		span->lineconfig = 7;
		break;
	case WAN_T1_0_110:    
        	DEBUG_TE1("%s: LBO 0-110 ft.\n", card->devname);
		span->lineconfig = 0;
		break;
 	case WAN_T1_110_220:   
        	DEBUG_TE1("%s: LBO 110-220 ft.\n", card->devname);
		span->lineconfig = 1;
		break;
 	case WAN_T1_220_330:   
        	DEBUG_TE1("%s: LBO 220-330 ft.\n", card->devname);
		span->lineconfig = 2;
		break;
 	case WAN_T1_330_440:   
        	DEBUG_TE1("%s: LBO 330-440 ft.\n", card->devname);
		span->lineconfig = 3;
		break;
 	case WAN_T1_440_550:   
        	DEBUG_TE1("%s: LBO 440-550 ft.\n", card->devname);
		span->lineconfig = 4;
		break;
 	case WAN_T1_550_660:  
        	DEBUG_TE1("%s: LBO 550-660 ft.\n", card->devname);
		span->lineconfig = 5;
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
	if (wp->sig_ready != WP_SIG_CONFIGURED){
		return 0;
	}
	if (bits & ZT_ABIT) ABCD_bits |= BIT_SIGX_SIGDATA_A;
	if (bits & ZT_BBIT) ABCD_bits |= BIT_SIGX_SIGDATA_B;
	if (bits & ZT_CBIT) ABCD_bits |= BIT_SIGX_SIGDATA_C;
	if (bits & ZT_DBIT) ABCD_bits |= BIT_SIGX_SIGDATA_D;
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
	DEBUG_TDMV("%s: %s:%d(%d) TX RBS: A:%1d B:%1d C:%1d D:%1d\n", 
			wp->devname, 
			(wp->ise1) ? "E1" : "T1",
			chan->channo, wp->first_channo,
			(ABCD_bits & BIT_SIGX_SIGDATA_A) ? 1 : 0,
			(ABCD_bits & BIT_SIGX_SIGDATA_B) ? 1 : 0,
			(ABCD_bits & BIT_SIGX_SIGDATA_C) ? 1 : 0,
			(ABCD_bits & BIT_SIGX_SIGDATA_D) ? 1 : 0);
	// FIXME: Do we need lock here?
	/* Output new values */
	card->wandev.fe_iface.set_rbsbits(
			&wp->card->fe, 
			chan->channo - wp->first_channo, 
			ABCD_bits);
	return 0;
}


/******************************************************************************
** wp_tdmv_ioctl() - 
**
**	OK
*/
static int 
wp_tdmv_ioctl(struct zt_chan *chan, unsigned int cmd, unsigned long data)
{
	switch(cmd) {
	default:
		return -ENOTTY;
	}
}

/******************************************************************************
** wp_tdmv_open() - 
**
**	OK
*/
static int wp_tdmv_open(struct zt_chan *chan)
{
	wp_tdmv_softc_t*	wp = NULL;
	
	WAN_ASSERT2(chan == NULL, -ENODEV);
	WAN_ASSERT2(chan->pvt == NULL, -ENODEV);
	wp = chan->pvt;
	wp->usecount++;
	DEBUG_TDMV("%s: Open (usecount=%d, channo=%d)...\n", 
				wp->devname, wp->usecount, chan->channo);
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
	DEBUG_TDMV("%s: Close (usecount=%d, channo=%d)...\n", 
				wp->devname, wp->usecount, chan->channo);
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
	DEBUG_TDMV("%s: Unregister from Asterisk software!\n",
			wp->devname);
	zt_unregister(&wp->span);
	kfree(wp);
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

