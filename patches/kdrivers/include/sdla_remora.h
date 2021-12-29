/*******************************************************************************
** sdla_remora.h	
**
** Author: 	Alex Feldman  <al.feldman@sangoma.com>
**
** Copyright:	(c) 2005 Sangoma Technologies Inc.
**
**		This program is free software; you can redistribute it and/or
**		modify it under the terms of the GNU General Public License
**		as published by the Free Software Foundation; either version
**		2 of the License, or (at your option) any later version.
** ============================================================================
** Oct 6, 2005	Alex Feldman	Initial version.
*******************************************************************************/

#ifndef __SDLA_REMORA_H
# define __SDLA_REMORA_H

#ifdef __SDLA_REMORA_SRC
# define EXTERN
#else
# define EXTERN extern
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <net/sdla_remora_proslic.h>
#else
# include <linux/sdla_remora_proslic.h>
#endif

/*******************************************************************************
**			  DEFINES and MACROS
*******************************************************************************/
#define FXOFXS_CHANNEL_RANGE(fe)	16

/*******************************************************************************
**			  TYPEDEF STRUCTURE
*******************************************************************************/
typedef struct sdla_remora_cfg_ {
	int	not_used;
	int	fxo_opermode;
//	int	tdmv_law;	/* WAN_TDMV_ALAW or WAN_TDMV_MULAW */
	int	reversepolarity;
} sdla_remora_cfg_t;


#if defined(WAN_KERNEL)

#define MAX_REMORA_MODULES	16

enum proslic_power_warn {
	PROSLIC_POWER_UNKNOWN = 0,
	PROSLIC_POWER_ON,
	PROSLIC_POWER_WARNED,
};


typedef struct {
	int	ready;
	
} fxo_t;

typedef struct {
	int	ready;
	int	idletxhookstate;
	enum proslic_power_warn proslic_power;
} fxs_t;

typedef struct sdla_remora_param {
	int	not_used;

	union {
		fxo_t	fxo;
		fxs_t	fxs;
	} mod[MAX_REMORA_MODULES];
	int		type[MAX_REMORA_MODULES];	
	int		chain[MAX_REMORA_MODULES];
	unsigned long	module_map;	/* Map of available module */

} sdla_remora_param_t;


#endif /* WAN_KERNEL */

/*******************************************************************************
**			  FUNCTION PROTOTYPES
*******************************************************************************/
extern int	wp_remora_config(void *, void*);
extern int	wp_remora_unconfig(void *);

#undef EXTERN
#endif	/* __SDLA_REMORA_H */
