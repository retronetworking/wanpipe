/******************************************************************************
 * sdla_tdmv.h	
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
 ******************************************************************************
 */

#ifndef __SDLA_TDMV_H
# define __SDLA_TDMV_H

#ifdef __SDLA_TDMV_SRC
# define EXTERN
#else
# define EXTERN extern
#endif

#include <linux/wanpipe_edac_iface.h>

/*
*******************************************************************************
**			  DEFINES and MACROS
*******************************************************************************
*/
#define WAN_TDMV_IDLE_FLAG	0x7F

/*
*******************************************************************************
**			  TYPEDEF STRUCTURE
*******************************************************************************
*/



typedef struct wan_tdmv_ {
	void	*sc;
	int	max_tx_len;
	int	max_timeslots;
	int	brt_enable;
	int	spanno;
	wan_tdmv_rxtx_pwr_t chan_pwr[31];	
	WAN_LIST_ENTRY(wan_tdmv_)	next;
} wan_tdmv_t;

/*
*******************************************************************************
**			  FUNCTION PROTOTYPES
*******************************************************************************
*/
EXTERN int  wp_tdmv_check_mtu(void*, unsigned long);
EXTERN int  wp_tdmv_init(void*, wanif_conf_t*);
EXTERN void wp_tdmv_free(wan_tdmv_t*);
EXTERN int  wp_tdmv_software_init(wan_tdmv_t*);
EXTERN void wp_tdmv_state(void*, int);
EXTERN int  wp_tdmv_running(void*);
EXTERN int  wp_tdmv_rx_tx(void*, netskb_t*);
EXTERN void wp_tdmv_report_rbsbits(void*, int, unsigned char);
EXTERN void wp_tdmv_report_alarms(void*, unsigned long);

EXTERN int wp_tdmv_create(void* pcard, wandev_conf_t *conf);
EXTERN int wp_tdmv_reg(void*, wanif_conf_t*, netdevice_t*);
EXTERN int wp_tdmv_unreg(void* pcard, unsigned long ts_map);
EXTERN int wp_tdmv_remove(void* pcard);
EXTERN int wp_tdmv_rx_chan(wan_tdmv_t *wan_tdmv, int channo, 
			   unsigned char *rxbuf, unsigned char *txbuf);
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN)
EXTERN int  wp_tdmv_rx_dchan(wan_tdmv_t *wan_tdmv, int channo, 
			   unsigned char *rxbuf, unsigned int len);
#endif
EXTERN int  wp_tdmv_rx_tx_span(void *pcard);

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_ECHOMASTER
EXTERN int wp_tdmv_echo_check(wan_tdmv_t *wan_tdmv, void *current_ztchan, int channo);
#endif


EXTERN int wanpipe_codec_convert_2s(u8 *data, int len, netskb_t *nskb, 
                                    u16 *power_ptr, int is_alaw);

EXTERN int wanpipe_codec_convert_s2ulaw(netskb_t *skb, netskb_t *nskb, int is_alaw);

EXTERN int wp_tdmv_is_rbsbits(wan_tdmv_t *);
EXTERN int wp_tdmv_rbsbits_poll(wan_tdmv_t *, void*);

#undef EXTERN
#endif	/* __SDLA_VOIP_H */
