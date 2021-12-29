/*
 * Copyright (c) 2001
 *	Alex Feldman <al.feldman@sangoma.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Alex Feldman.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Alex Feldman AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Alex Feldman OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	$Id: sdla_te1.h,v 1.17 2005/01/24 20:16:42 sangoma Exp $
 */

/*****************************************************************************
 * sdla_te1.h	Sangoma TE1 configuration definitions.
 *
 * Author:      Alex Feldman
 *
 * ============================================================================
 * Aprl 30, 2001	Alex Feldman	Initial version.
 ****************************************************************************
*/
#ifndef	_SDLA_TE1_H
#    define	_SDLA_TE1_H

#ifdef SDLA_TE1
# define EXTERN
#else
# define EXTERN extern
#endif

/************************************************************************
 *			  DEFINES AND MACROS				*
 ***********************************************************************/
#if defined(__NetBSD__) || defined (__FreeBSD__) || defined (__OpenBSD__)
# include <net/sdla_te1_def.h>
#elif defined (__WINDOWS__)
# include "sdla_te1_def.h"
#else
# include <linux/sdla_te1_def.h>
#endif

/*
*************************************************************************
*			  DEFINES AND MACROS				*
*************************************************************************
*/

#define NUM_OF_T1_CHANNELS	24
#define NUM_OF_E1_TIMESLOTS	31
#define NUM_OF_E1_CHANNELS	32
#define ENABLE_ALL_CHANNELS	0xFFFFFFFF

#define E1_FRAMING_TIMESLOT	0
#define E1_SIGNALING_TIMESLOT	16

/* Alram bit mask */
#define WAN_TE_BIT_ALOS_ALARM		0x0001
#define WAN_TE_BIT_LOS_ALARM		0x0002
#define WAN_TE_BIT_ALTLOS_ALARM		0x0004
#define WAN_TE_BIT_OOF_ALARM		0x0008
#define WAN_TE_BIT_RED_ALARM		0x0010
#define WAN_TE_BIT_AIS_ALARM		0x0020
#define WAN_TE_BIT_OOSMF_ALARM		0x0040
#define WAN_TE_BIT_OOCMF_ALARM		0x0080
#define WAN_TE_BIT_OOOF_ALARM		0x0100
#define WAN_TE_BIT_RAI_ALARM		0x0200
#define WAN_TE_BIT_YEL_ALARM		0x0400
#define WAN_TE_BIT_LOOPUP_CODE		0x2000	
#define WAN_TE_BIT_LOOPDOWN_CODE	0x4000	
#define WAN_TE_BIT_TE1_ALARM		0x8000	/* for Windows only */

/* Performamce monitor counter defines */
#define frm_bit_error		pmon1	/* E1/T1   */
#define oof_errors		pmon2	/* T1 only */
#define far_end_blk_errors	pmon2	/* E1 only */
#define bit_errors		pmon3	/* T1 only */
#define crc_errors		pmon3	/* E1 only */
#define lcv			pmon4	/* E1/T1   */

/* For T1 only */
#define WAN_T1_LBO_0_DB      0x01
#define WAN_T1_LBO_75_DB     0x02
#define WAN_T1_LBO_15_DB     0x03
#define WAN_T1_LBO_225_DB    0x04
#define WAN_T1_0_110         0x05
#define WAN_T1_110_220       0x06
#define WAN_T1_220_330       0x07
#define WAN_T1_330_440       0x08
#define WAN_T1_440_550       0x09
#define WAN_T1_550_660       0x0A

/* For E1 only */
#define WAN_E1_120           0x0B
#define WAN_E1_75            0x0C

/* Line loopback modes */
#define WAN_TE1_LINELB_MODE	0x01
#define WAN_TE1_PAYLB_MODE	0x02
#define WAN_TE1_DDLB_MODE	0x03
#define WAN_TE1_TX_LB_MODE	0x04

/* Line loopback activate/deactive modes */
#define WAN_TE1_ACTIVATE_LB	0x01
#define WAN_TE1_DEACTIVATE_LB	0x02

/* Loopback commands (T1.107-1995 p.44) */
#define LINELB_TE1_TIMER	40	/* 40ms */
#define LINELB_CODE_CNT		10	/* no. of repetitions for lb_code */
#define LINELB_CHANNEL_CNT	10	/* no. of repetitions for channel */
#define LINELB_ACTIVATE_CODE	0x07
#define LINELB_DEACTIVATE_CODE	0x1C
#define LINELB_DS3LINE		0x1B
#define LINELB_DS1LINE_1	0x21	
#define LINELB_DS1LINE_2	0x22
#define LINELB_DS1LINE_3	0x23
#define LINELB_DS1LINE_4	0x24
#define LINELB_DS1LINE_5	0x25
#define LINELB_DS1LINE_6	0x26
#define LINELB_DS1LINE_7	0x27
#define LINELB_DS1LINE_8	0x28
#define LINELB_DS1LINE_9	0x29
#define LINELB_DS1LINE_10	0x2A
#define LINELB_DS1LINE_11	0x2B
#define LINELB_DS1LINE_12	0x2C
#define LINELB_DS1LINE_13	0x2D
#define LINELB_DS1LINE_14	0x2E
#define LINELB_DS1LINE_15	0x2F
#define LINELB_DS1LINE_16	0x30
#define LINELB_DS1LINE_17	0x31
#define LINELB_DS1LINE_18	0x32
#define LINELB_DS1LINE_19	0x33
#define LINELB_DS1LINE_20	0x34
#define LINELB_DS1LINE_21	0x35
#define LINELB_DS1LINE_22	0x36
#define LINELB_DS1LINE_23	0x37
#define LINELB_DS1LINE_24	0x38
#define LINELB_DS1LINE_25	0x39
#define LINELB_DS1LINE_26	0x3A
#define LINELB_DS1LINE_27	0x3B
#define LINELB_DS1LINE_28	0x3C
#define LINELB_DS1LINE_ALL	0x13
#define LINELB_DS1LINE_MASK	0x1F

/* Interrupt polling delay */
#define POLLING_TE1_TIMER	1000	/* 1 sec */

/* TE1 critical flag */
#define TE_TIMER_RUNNING 	0x01
#define TE_TIMER_KILL 		0x02
#define LINELB_WAITING		0x03
#define LINELB_CODE_BIT		0x04
#define LINELB_CHANNEL_BIT	0x05
#define TE_CONFIGURED		0x06

#if 0
#define TE_TIMER_RUNNING 	0x01
#define TE_TIMER_KILL 		0x02
#define LINELB_WAITING		0x04
#define LINELB_CODE_BIT		0x08
#define LINELB_CHANNEL_BIT	0x10
#endif

/* TE1 timer flags */
#define TE_LINELB_TIMER		0x01
#define TE_LINKDOWN_TIMER	0x02
#define TE_SET_INTR		0x03
#define TE_ABCD_UPDATE		0x04
#define TE_LINKUP_TIMER		0x05

/* TE1 T1/E1 interrupt setting delay */
#define INTR_TE1_TIMER		150	/* 50 ms */

#define IS_T1_CARD(card)	IS_T1_MEDIA(card->fe.fe_cfg.media)
#define IS_E1_CARD(card)	IS_E1_MEDIA(card->fe.fe_cfg.media)
#define IS_TE1_CARD(card)	IS_TE1_MEDIA(card->fe.fe_cfg.media)

#define IS_TE1_UNFRAMED(fe)   					\
	(((sdla_fe_t*)(fe))->fe_cfg.frame == WAN_FR_UNFRAMED)
 
#define GET_TE_CHANNEL_RANGE(fe)				\
	(IS_T1_MEDIA(((sdla_fe_t*)(fe))->fe_cfg.media) ? NUM_OF_T1_CHANNELS :\
	 IS_E1_MEDIA(((sdla_fe_t*)(fe))->fe_cfg.media) ? NUM_OF_E1_CHANNELS :0)
			

#define WAN_TE_ALARM(alarm, bit)	((alarm) & (bit)) ? "ON" : "OFF"

#define WAN_TE_ALOS_ALARM(alarm)	WAN_TE_ALARM(alarm, WAN_TE_BIT_ALOS_ALARM)
#define WAN_TE_LOS_ALARM(alarm)		WAN_TE_ALARM(alarm, WAN_TE_BIT_LOS_ALARM)
#define WAN_TE_OOF_ALARM(alarm)		WAN_TE_ALARM(alarm, WAN_TE_BIT_OOF_ALARM)
#define WAN_TE_RED_ALARM(alarm)		WAN_TE_ALARM(alarm, WAN_TE_BIT_RED_ALARM)
#define WAN_TE_AIS_ALARM(alarm)		WAN_TE_ALARM(alarm, WAN_TE_BIT_AIS_ALARM)
#define WAN_TE_OOSMF_ALARM(alarm)	WAN_TE_ALARM(alarm, WAN_TE_BIT_OOSMF_ALARM)
#define WAN_TE_OOCMF_ALARM(alarm)	WAN_TE_ALARM(alarm, WAN_TE_BIT_OOCMF_ALARM)
#define WAN_TE_OOOF_ALARM(alarm)	WAN_TE_ALARM(alarm, WAN_TE_BIT_OOOF_ALARM)
#define WAN_TE_RAI_ALARM(alarm)		WAN_TE_ALARM(alarm, WAN_TE_BIT_RAI_ALARM)
#define WAN_TE_YEL_ALARM(alarm)		WAN_TE_ALARM(alarm, WAN_TE_BIT_YEL_ALARM)

#define TECLK_DECODE(val)			\
	(val == WAN_NORMAL_CLK) ? "Normal" :	\
	(val == WAN_MASTER_CLK) ? "Master" : "Unknown"

#define LBO_DECODE(val)				\
	(val == WAN_T1_LBO_0_DB)	? "0db" :	\
	(val == WAN_T1_LBO_75_DB)	? "7.5db" :	\
	(val == WAN_T1_LBO_15_DB)	? "15dB" :	\
	(val == WAN_T1_LBO_225_DB)	? "22.5dB" :	\
	(val == WAN_T1_0_110)		? "0-110ft" :	\
	(val == WAN_T1_110_220)		? "110-220ft" :	\
	(val == WAN_T1_220_330)		? "220-330ft" :	\
	(val == WAN_T1_330_440)		? "330-440ft" :	\
	(val == WAN_T1_440_550)		? "440-550ft" :	\
	(val == WAN_T1_550_660)		? "550-660ft" : "Unknown"


/*----------------------------------------------------------------------------
 * T1/E1 configuration structures.
 */
typedef struct sdla_te_cfg {
	unsigned char	lbo;
	unsigned char	te_clock;
	unsigned long	active_ch;
	unsigned long	te_rbs_ch;
	unsigned char	high_impedance_mode;
} sdla_te_cfg_t;

/* Performamce monitor counters */
typedef struct {
	unsigned long pmon1;
	unsigned long pmon2;
	unsigned long pmon3;
	unsigned long pmon4;
} sdla_te_pmon_t;


/*
 ******************************************************************************
			STRUCTURES AND TYPEDEFS
 ******************************************************************************
*/

#ifdef WAN_KERNEL


/* -----------------------------------------------------------------------------
 * Constants for the SET_T1_E1_SIGNALING_CFG/READ_T1_E1_SIGNALING_CFG commands
 * ---------------------------------------------------------------------------*/

/* the structure for setting the signaling permission */
#pragma pack(1)
typedef struct {
	unsigned char time_slot[32];
} te_signaling_perm_t;
#pragma pack()

/* settings for the signaling permission structure */
#define TE_SIG_DISABLED		0x00 /* signaling is disabled */
#define TE_RX_SIG_ENABLED	0x01 /* receive signaling is enabled */
#define TE_TX_SIG_ENABLED	0x02 /* transmit signaling is enabled */
#define TE_SET_TX_SIG_BITS	0x80 /* a flag indicating that outgoing 
					signaling bits should be set */

/* the structure used for the 
 * SET_T1_E1_SIGNALING_CFG/READ_T1_E1_SIGNALING_CFG command 
 */
#pragma pack(1)
typedef struct {
	/* signaling permission structure */
	te_signaling_perm_t sig_perm;
	/* loop signaling processing counter */
	unsigned char sig_processing_counter;
	/* pointer to the signaling permission structure */
	unsigned long ptr_te_sig_perm_struct;
	/* pointer to the receive signaling structure */
	unsigned long ptr_te_Rx_sig_struct;
	/* pointer to the transmit signaling structure */
	unsigned long ptr_te_Tx_sig_struct;
} te_signaling_cfg_t;
#pragma pack()

/* the structure used for reading and setting the signaling bits */
#pragma pack(1)
typedef struct {
	unsigned char time_slot[32];
} te_signaling_status_t;
#pragma pack()

typedef struct {
	unsigned char	lb_cmd;
	unsigned long	lb_time;
	
	unsigned char	lb_tx_cmd;
	unsigned long	lb_tx_cnt;

	unsigned char	critical;
	wan_timer_t	timer;
	unsigned char	timer_cmd;
		
	unsigned char	SIGX_chg_30_25;
	unsigned char	SIGX_chg_24_17;
	unsigned char	SIGX_chg_16_9;
	unsigned char	SIGX_chg_8_1;

	unsigned long	ptr_te_sig_perm_off;
	unsigned long	ptr_te_Rx_sig_off;
	unsigned long	ptr_te_Tx_sig_off;

} sdla_te_param_t;



/*
 ******************************************************************************
			  FUNCTION PROTOTYPES
 ******************************************************************************
*/

EXTERN int sdla_te_default_cfg(void* pfe, void* fe_cfg, int media);
EXTERN int sdla_te_copycfg(void* pfe, void* fe_cfg);
EXTERN int sdla_te_global_config(void *pfe);
EXTERN short sdla_te_config(void *pfe,void *pfe_iface);
EXTERN int sdla_te_global_unconfig(void *pfe);
EXTERN void sdla_te_unconfig(void *pfe);

#endif /* WAN_KERNEL */

#undef EXTERN


#endif /* _SDLA_TE1_H */
