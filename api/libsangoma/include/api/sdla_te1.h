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
 *	$Id: sdla_te1.h,v 1.46 2006/10/03 16:45:59 sangoma Exp $
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
#if 0
#if defined(__NetBSD__) || defined (__FreeBSD__) || defined (__OpenBSD__)
# include <sdla_te1_pmc.h>
#elif defined (__WINDOWS__)
# include "sdla_te1_pmc.h"
#else
# include <linux/sdla_te1_pmc.h>
#endif
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

#define WAN_TE_SIG_POLL		0x01
#define WAN_TE_SIG_INTR		0x02

/* Framer Alarm bit mask */
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
#define IS_TE_ALARM(alarm, mask)	(alarm & mask)
#define IS_TE_ALOS_ALARM(alarm)		IS_TE_ALARM(alarm, WAN_TE_BIT_ALOS_ALARM)
#define IS_TE_LOS_ALARM(alarm)		IS_TE_ALARM(alarm, WAN_TE_BIT_LOS_ALARM)
#define IS_TE_OOF_ALARM(alarm)		IS_TE_ALARM(alarm, WAN_TE_BIT_OOF_ALARM)
#define IS_TE_RED_ALARM(alarm)		IS_TE_ALARM(alarm, WAN_TE_BIT_RED_ALARM)
#define IS_TE_AIS_ALARM(alarm)		IS_TE_ALARM(alarm, WAN_TE_BIT_AIS_ALARM)
#define IS_TE_OOSMF_ALARM(alarm)	IS_TE_ALARM(alarm, WAN_TE_BIT_OOSMF_ALARM)
#define IS_TE_OOCMF_ALARM(alarm)	IS_TE_ALARM(alarm, WAN_TE_BIT_OOCMF_ALARM)
#define IS_TE_OOOF_ALARM(alarm)		IS_TE_ALARM(alarm, WAN_TE_BIT_OOOF_ALARM)
#define IS_TE_RAI_ALARM(alarm)		IS_TE_ALARM(alarm, WAN_TE_BIT_RAI_ALARM)
#define IS_TE_YEL_ALARM(alarm)		IS_TE_ALARM(alarm, WAN_TE_BIT_YEL_ALARM)

/* Performance monitor counters bit mask */
#define WAN_TE_BIT_PMON_LCV		0x01	/* line code violation counter */
#define WAN_TE_BIT_PMON_BEE		0x02	/* bit errror event (T1) */
#define WAN_TE_BIT_PMON_OOF		0x04	/* frame out of sync counter */
#define WAN_TE_BIT_PMON_FEB		0x08	/* far end block counter */
#define WAN_TE_BIT_PMON_CRC4		0x10	/* crc4 error counter (E1) */
#define WAN_TE_BIT_PMON_FER		0x20	/* framing bit error (T1-pmc) */
#define WAN_TE_BIT_PMON_FAS		0x40	/* Frame Alginment signal (E1) */

/* LIU Alarm bit mask */
#define WAN_TE_BIT_LIU_ALARM		0x8000	/* print liu status */
#define WAN_TE_BIT_LIU_ALARM_SC		0x0001
#define WAN_TE_BIT_LIU_ALARM_OC		0x0002
#define WAN_TE_BIT_LIU_ALARM_LOS	0x0004

/* For T1 only */
#define WAN_T1_LBO_0_DB		0x01
#define WAN_T1_LBO_75_DB	0x02
#define WAN_T1_LBO_15_DB	0x03
#define WAN_T1_LBO_225_DB	0x04
#define WAN_T1_0_110		0x05
#define WAN_T1_110_220		0x06
#define WAN_T1_220_330		0x07
#define WAN_T1_330_440		0x08
#define WAN_T1_440_550		0x09
#define WAN_T1_550_660		0x0A
/* For E1 only */
#define WAN_E1_120		0x0B
#define WAN_E1_75		0x0C
/* T1/E1 8 ports */
#define WAN_T1_0_133		0x0D
#define WAN_T1_133_266		0x0E
#define WAN_T1_266_399		0x0F
#define WAN_T1_399_533		0x10
#define WAN_T1_533_655		0x11


/* For T1 only (long or short haul) */
#define WAN_T1_LONG_HAUL	0x01
#define WAN_T1_SHORT_HAUL	0x02

/* Line loopback modes */
#define WAN_TE1_LINELB_MODE	0x01
#define WAN_TE1_PAYLB_MODE	0x02
#define WAN_TE1_DDLB_MODE	0x03
#define WAN_TE1_TX_LB_MODE	0x04
#define WAN_TE1_LIU_ALB_MODE	0x05
#define WAN_TE1_LIU_LLB_MODE	0x06
#define WAN_TE1_LIU_RLB_MODE	0x07
#define WAN_TE1_LIU_DLB_MODE	0x08
#define WAN_TE1_FR_FLB_MODE	0x09
#define WAN_TE1_FR_PLB_MODE	0x0A
#define WAN_TE1_FR_RLB_MODE	0x0B
#define WAN_TE1_LB_TYPE_DECODE(type)						\
		(type == WAN_TE1_LINELB_MODE) ? "Line Loopback" :		\
		(type == WAN_TE1_PAYLB_MODE) ? "Payload Loopback" :		\
		(type == WAN_TE1_DDLB_MODE) ? "Diagnostic Digital Loopback" :	\
		(type == WAN_TE1_TX_LB_MODE) ? "TX Loopback" :			\
		(type == WAN_TE1_LIU_ALB_MODE) ? "Analog LIU Loopback" :	\
		(type == WAN_TE1_LIU_LLB_MODE) ? "Local LIU Loopback" :		\
		(type == WAN_TE1_LIU_RLB_MODE) ? "Remote LIU Loopback" :	\
		(type == WAN_TE1_LIU_DLB_MODE) ? "Dual LIU Loopback" :		\
		(type == WAN_TE1_FR_FLB_MODE) ? "Framer Loopback" :		\
		(type == WAN_TE1_FR_RLB_MODE) ? "Remote Framer Loopback" :	\
		(type == WAN_TE1_FR_PLB_MODE) ? "Payload Framer Loopback" :	\
						"Unknown Loopback"

/* Line loopback activate/deactive modes */
#define WAN_TE1_ACTIVATE_LB	0x01
#define WAN_TE1_DEACTIVATE_LB	0x02
#define WAN_TE1_LB_MODE_DECODE(mode)				\
		(mode == WAN_TE1_ACTIVATE_LB) ? "Activate" :	\
		(mode == WAN_TE1_DEACTIVATE_LB) ? "Deactivate" :\
						"Unknown"

/* T1/E1 front end Master clock source */
#define WAN_TE1_REFCLK_OSC	0x00
#define WAN_TE1_REFCLK_LINE1	0x01
#define WAN_TE1_REFCLK_LINE2	0x02
#define WAN_TE1_REFCLK_LINE3	0x03
#define WAN_TE1_REFCLK_LINE4	0x04

/* E1 signalling insertion mode */
#define WAN_TE1_SIG_NONE	0x00	/* default */
#define WAN_TE1_SIG_CCS		0x01	/* E1 CCS - default */
#define WAN_TE1_SIG_CAS		0x02	/* E1 CAS */

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
#define LINELB_DS1LINE_DISABLE	0x3F
#define LINELB_DS1LINE_MASK	0x3F

/* Interrupt polling delay */
#define POLLING_TE1_TIMER	1000	/* 1 sec */

/* TE1 critical flag */
#define TE_TIMER_RUNNING 	0x01
#define TE_TIMER_KILL 		0x02
#define LINELB_WAITING		0x03
#define LINELB_CODE_BIT		0x04
#define LINELB_CHANNEL_BIT	0x05
#define TE_CONFIGURED		0x06

/* TE1 timer flags (polling) */
#define TE_LINELB_TIMER		0x01
#define TE_LINKDOWN_TIMER	0x02
#define TE_SET_INTR		0x03
#define TE_RBS_READ		0x04
#define TE_LINKUP_TIMER		0x05
#define TE_SET_RBS		0x06
#define TE_UPDATE_PMON		0x07
#define TE_SET_LB_MODE		0x08
#define TE_RBS_ENABLE		0x09
#define TE_RBS_DISABLE		0x0A
#define TE_POLL_CFG		0x0B

/* TE1 T1/E1 interrupt setting delay */
#define INTR_TE1_TIMER		150	/* 50 ms */

/* T1/E1 RBS flags (bit-map) */
#define WAN_TE_RBS_NONE		0x00
#define WAN_TE_RBS_UPDATE	0x01
#define WAN_TE_RBS_REPORT	0x02


#define IS_T1_CARD(card)	IS_T1_FEMEDIA(&(card)->fe)
#define IS_E1_CARD(card)	IS_E1_FEMEDIA(&(card)->fe)
#define IS_TE1_CARD(card)	IS_TE1_FEMEDIA(&(card)->fe)

#define FE_LBO(fe_cfg)		(fe_cfg)->cfg.te_cfg.lbo	
#define FE_CLK(fe_cfg)		(fe_cfg)->cfg.te_cfg.te_clock	
#define FE_REFCLK(fe_cfg)	(fe_cfg)->cfg.te_cfg.te_ref_clock	
#define HIMPEDANCE_MODE(fe_cfg)	(fe_cfg)->cfg.te_cfg.high_impedance_mode
#define FE_ACTIVE_CH(fe_cfg)	(fe_cfg)->cfg.te_cfg.active_ch
#define FE_SIG_MODE(fe_cfg)	(fe_cfg)->cfg.te_cfg.sig_mode

#define GET_TE_CHANNEL_RANGE(fe)				\
	(IS_T1_FEMEDIA(fe) ? NUM_OF_T1_CHANNELS :\
	 IS_E1_FEMEDIA(fe) ? NUM_OF_E1_CHANNELS :0)
			

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

#define WAN_TE_LIU_ALARM_SC(alarm)	WAN_TE_ALARM(alarm, WAN_TE_BIT_LIU_ALARM_SC)
#define WAN_TE_LIU_ALARM_OC(alarm)	WAN_TE_ALARM(alarm, WAN_TE_BIT_LIU_ALARM_OC)
#define WAN_TE_LIU_ALARM_LOS(alarm)	WAN_TE_ALARM(alarm, WAN_TE_BIT_LIU_ALARM_LOS)

#define TECLK_DECODE(fe_cfg)			\
	(FE_CLK(fe_cfg) == WAN_NORMAL_CLK) ? "Normal" :	\
	(FE_CLK(fe_cfg) == WAN_MASTER_CLK) ? "Master" : "Unknown"

#define LBO_DECODE(fe_cfg)				\
	(FE_LBO(fe_cfg) == WAN_T1_LBO_0_DB)	? "0db" :	\
	(FE_LBO(fe_cfg) == WAN_T1_LBO_75_DB)	? "7.5db" :	\
	(FE_LBO(fe_cfg) == WAN_T1_LBO_15_DB)	? "15dB" :	\
	(FE_LBO(fe_cfg) == WAN_T1_LBO_225_DB)	? "22.5dB" :	\
	(FE_LBO(fe_cfg) == WAN_T1_0_110)	? "0-110ft" :	\
	(FE_LBO(fe_cfg) == WAN_T1_110_220)	? "110-220ft" :	\
	(FE_LBO(fe_cfg) == WAN_T1_220_330)	? "220-330ft" :	\
	(FE_LBO(fe_cfg) == WAN_T1_330_440)	? "330-440ft" :	\
	(FE_LBO(fe_cfg) == WAN_T1_440_550)	? "440-550ft" :	\
	(FE_LBO(fe_cfg) == WAN_T1_550_660)	? "550-660ft" : \
	(FE_LBO(fe_cfg) == WAN_T1_0_133)	? "0-133ft"   :	\
	(FE_LBO(fe_cfg) == WAN_T1_133_266)	? "133-266ft" :	\
	(FE_LBO(fe_cfg) == WAN_T1_266_399)	? "266-399ft" :	\
	(FE_LBO(fe_cfg) == WAN_T1_399_533)	? "399-533ft" :	\
	(FE_LBO(fe_cfg) == WAN_T1_533_655)	? "5330-599ft":	\
	(FE_LBO(fe_cfg) == WAN_E1_120)		? "120OH"     :	\
	(FE_LBO(fe_cfg) == WAN_E1_75)		? "75OH"      :	\
							"Unknown"

/* Front-End UDP command */
#define WAN_FE_GET_STAT		(WAN_FE_UDP_CMD_START + 0)
#define WAN_FE_SET_LB_MODE	(WAN_FE_UDP_CMD_START + 1)
#define WAN_FE_FLUSH_PMON	(WAN_FE_UDP_CMD_START + 2)
#define WAN_FE_GET_CFG		(WAN_FE_UDP_CMD_START + 3)
#define WAN_FE_SET_DEBUG_MODE	(WAN_FE_UDP_CMD_START + 4)
#define WAN_FE_TX_MODE		(WAN_FE_UDP_CMD_START + 5)

/* FE interrupt types */
#define WAN_TE_INTR_NONE	0x00
#define WAN_TE_INTR_GLOBAL	0x01
#define WAN_TE_INTR_BASIC	0x02
#define WAN_TE_INTR_SIGNALLING	0x03
#define WAN_TE_INTR_FXS_DTMF	0x04
#define WAN_TE_INTR_PMON	0x05

/*----------------------------------------------------------------------------
 * T1/E1 configuration structures.
 */
typedef struct sdla_te_cfg {
	unsigned char	lbo;
	unsigned char	te_clock;
	unsigned long	active_ch;
	unsigned long	te_rbs_ch;
	unsigned char	high_impedance_mode;
	unsigned char	te_ref_clock;
	unsigned char	sig_mode;
} sdla_te_cfg_t;

/* Performamce monitor counters */
typedef struct {
	unsigned char	mask;
	unsigned long	lcv_errors;	/* Line code violation (T1/E1) */
	unsigned long	bee_errors;	/* Bit errors (T1) */
	unsigned long	oof_errors;	/* Frame out of sync (T1) */
	unsigned long	crc4_errors;	/* CRC4 errors (E1) */
	unsigned long	fas_errors;	/* Frame Aligment Signal (E1)*/
	unsigned long	feb_errors;	/* Far End Block errors (E1) */
	unsigned long	fer_errors;	/* Framing bit errors (T1) */
} sdla_te_pmon_t;

/*
 ******************************************************************************
			STRUCTURES AND TYPEDEFS
 ******************************************************************************
*/

#ifdef WAN_KERNEL

#define WAN_TE1_LBO(fe)		FE_LBO(&((fe)->fe_cfg))
#define WAN_TE1_CLK(fe)		FE_CLK(&((fe)->fe_cfg))
#define WAN_TE1_REFCLK(fe)	FE_REFCLK(&((fe)->fe_cfg))
#define WAN_TE1_HI_MODE(fe)	HIMPEDANCE_MODE(&((fe)->fe_cfg))
#define WAN_TE1_ACTIVE_CH(fe)	FE_ACTIVE_CH(&((fe)->fe_cfg))
#define WAN_TE1_SIG_MODE(fe)	FE_SIG_MODE(&((fe)->fe_cfg))

#define TE_LBO_DECODE(fe)	LBO_DECODE(&((fe)->fe_cfg))
#define TE_CLK_DECODE(fe)	TECLK_DECODE(&((fe)->fe_cfg))

/* Read/Write to front-end register */
#if 0
#if 0
#define READ_REG(reg)		card->wandev.read_front_end_reg(card, reg)
#define WRITE_REG(reg, value)	card->wandev.write_front_end_reg(card, reg, (unsigned char)(value))
#endif
#define WRITE_REG(reg,val)						\
	fe->write_fe_reg(						\
		fe->card,						\
		(int)((reg) + (fe->fe_cfg.line_no*PMC4_LINE_DELTA)),	\
		(int)(val))

#define WRITE_REG_LINE(fe_line_no, reg,val)				\
	fe->write_fe_reg(						\
		fe->card,						\
		(int)((reg) + (fe_line_no)*PMC4_LINE_DELTA),	\
		(int)(val))
	
#define READ_REG(reg)							\
	fe->read_fe_reg(						\
		fe->card,						\
		(int)((reg) + (fe->fe_cfg.line_no*PMC4_LINE_DELTA)))
	
#define READ_REG_LINE(fe_line_no, reg)					\
	fe->read_fe_reg(						\
		fe->card,						\
		(int)((reg) + (fe_line_no)*PMC4_LINE_DELTA))
#endif

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

	unsigned char	lb_rx_code;
	
	unsigned char	critical;
	wan_timer_t	timer;
	unsigned char	timer_cmd;
	unsigned long	timer_ch_map;
	int 		timer_channel;	/* tx rbs per channel */
	unsigned char	timer_abcd;	/* tx rbs pec channel */
		
	unsigned char	SIGX_chg_30_25;
	unsigned char	SIGX_chg_24_17;
	unsigned char	SIGX_chg_16_9;
	unsigned char	SIGX_chg_8_1;

	unsigned long	ptr_te_sig_perm_off;
	unsigned long	ptr_te_Rx_sig_off;
	unsigned long	ptr_te_Tx_sig_off;

	unsigned char	intr_src1;
	unsigned char	intr_src2;
	unsigned char	intr_src3;

	unsigned int	max_channels;

	unsigned long	rx_rbs_status;
	unsigned char	rx_rbs[32];
	unsigned char	tx_rbs[32];

	unsigned long	tx_rbs_A;
	unsigned long	tx_rbs_B;
	unsigned long	tx_rbs_C;
	unsigned long	tx_rbs_D;

	unsigned long	rx_rbs_A;
	unsigned long	rx_rbs_B;
	unsigned long	rx_rbs_C;
	unsigned long	rx_rbs_D;

	unsigned char	xlpg_scale;
} sdla_te_param_t;


/*
 ******************************************************************************
			  FUNCTION PROTOTYPES
 ******************************************************************************
*/

EXTERN int sdla_te_default_cfg(void* pfe, void* fe_cfg, int media);
EXTERN int sdla_te_copycfg(void* pfe, void* fe_cfg);

EXTERN int sdla_te_iface_init(void *p_fe_iface);
EXTERN int sdla_ds_te1_iface_init(void *p_fe_iface);

#endif /* WAN_KERNEL */

#undef EXTERN


#endif /* _SDLA_TE1_H */