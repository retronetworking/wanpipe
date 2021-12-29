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
*****************************************************************************/


typedef struct wanpipe_tdm_api_ec {
	echo_can_state_t 			*ec;
	echo_can_disable_detector_state_t 	txecdis;
	echo_can_disable_detector_state_t 	rxecdis;
	unsigned int 				echocancel;
	unsigned int 				echostate;
	unsigned int 				echolastupdate;
	unsigned int 				echotimer;
	netskb_t 				*tx_skb;
}wanpipe_tdm_api_ec_t;


static __inline void wanpipe_tdm_api_ec_init(wanpipe_tdm_api_ec_t *ec_eng)
{
	netskb_t *skb = ec_eng->tx_skb;

	ec_eng->echostate = ECHO_STATE_IDLE;
	ec_eng->echolastupdate = 0;
	ec_eng->echotimer = 0;
	ec_eng->tx_skb=NULL;

	if (skb) {
		wan_skb_free(skb);
	}

	return;
}

static __inline int wanpipe_tdm_api_ec_create (wanpipe_tdm_api_ec_t *ec_eng, int tap_size)
{
	ec_eng->ec = echo_can_create(tap_size, 0);
	if (!ec_eng->ec) {
		return -EINVAL;
	}
	ec_eng->echocancel = tap_size;
	ec_eng->tx_skb = NULL;

	wanpipe_tdm_api_ec_init (ec_eng);

	echo_can_disable_detector_init(&chan->txecdis);
	echo_can_disable_detector_init(&chan->rxecdis);

	return 0;
}

static __inline int wanpipe_tdm_api_ec_free (wanpipe_tdm_api_ec_t *ec_eng, int tap_size)
{
	echo_can_state_t *ec_tmp=ec_eng->ec;

	wanpipe_tdm_api_ec_init (ec_eng);
	ec_eng->echocancel = 0;
	ec_eng->ec=NULL;

	if (ec_tmp){
		echo_can_free(ec);	
	}

	return 0;
}


static __inline int wanpipe_tdm_api_ec_train (wanpipe_tdm_api_ec_t *ec_eng, int t_period)
{
	if (ec_eng->ec){
		ec_eng->echostate = ECHO_STATE_PRETRAINING;
		ec_eng->echotimer = t_period;
		return 0;
	}		

	return -ENODEV;
}

static __inline int wanpipe_tdm_api_ec_tx(wanpipe_tdm_api_ec_t *ec_eng, netskb_t *skb)
{
	if (ec_eng->ec){ 
		if (ec_eng->echostate == ECHO_STATE_STARTTRAINING) {
			/* Transmit impulse now */
			*(short*)(wan_skb_data(skb)) = 16384;
			ec_eng->echostate = ECHO_STATE_AWAITINGECHO;
		}
	}
		
	return 0;
}
