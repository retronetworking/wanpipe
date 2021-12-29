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
} wan_tdmv_t;

/*
*******************************************************************************
**			  FUNCTION PROTOTYPES
*******************************************************************************
*/
EXTERN int wp_tdmv_check_mtu(void*, unsigned long);
EXTERN int wp_tdmv_init(void*, wanif_conf_t*);
EXTERN void wp_tdmv_free(wan_tdmv_t*);
EXTERN int wp_tdmv_remove(void*);
EXTERN int wp_tdmv_running(void*);
EXTERN void wp_tdmv_state(void*, int);
EXTERN int wp_tdmv_rx_tx(void*, netskb_t*);
EXTERN void wp_tdmv_report_rbsbits(void*, int, unsigned char);
EXTERN void wp_tdmv_report_alarms(void*, unsigned long);

#undef EXTERN
#endif	/* __SDLA_VOIP_H */
