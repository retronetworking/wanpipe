/******************************************************************************
 * zapcompat.h	
 *
 * Author: 	Moises Silva <moises.silva@gmail.com>
 *
 * Copyright:	(c) 2008 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 * Sep 06,  2008 Moises Silva   Initial Version
 * Nov 20,  2008 Alex Feldman   Added ZT_XLAW
 ******************************************************************************
 */

// Simple compat header to compile with Zaptel or DAHDI
#ifndef __ZAPCOMPAT_H
# define __ZAPCOMPAT_H

// for DAHDI we need to map values and functions from ZT_XX to DAHDI_XX
#if defined (DAHDI_ISSUES)

#ifdef WAN_KERNEL
# include <dahdi/kernel.h> // this will bring dahdi kernel stuff plus dahdi/user.h and friends
#else
# include <dahdi/user.h> // this will bring dahdi user stuff
#endif


#ifdef DAHDI_ECHOCANCEL_FAX_MODE 
#define DAHDI_22
#else
#undef DAHDI_22
#endif

// defines 
#define ZT_CODE DAHDI_CODE


#define ZT_ONHOOKTRANSFER DAHDI_ONHOOKTRANSFER
#define ZT_SETPOLARITY DAHDI_SETPOLARITY
#define ZT_CHUNKSIZE DAHDI_CHUNKSIZE
#define ZT_MAX_NUM_BUFS DAHDI_MAX_NUM_BUFS

#define ZT_EVENT_DTMFDOWN DAHDI_EVENT_DTMFDOWN
#define ZT_EVENT_DTMFUP DAHDI_EVENT_DTMFUP
#define ZT_EVENT_POLARITY DAHDI_EVENT_POLARITY

#define ZT_TXSIG_START DAHDI_TXSIG_START
#define ZT_TXSIG_OFFHOOK DAHDI_TXSIG_OFFHOOK
#define ZT_TXSIG_ONHOOK DAHDI_TXSIG_ONHOOK
#define ZT_TXSIG_KEWL DAHDI_TXSIG_KEWL

#define ZT_RXSIG_INITIAL DAHDI_RXSIG_INITIAL
#define ZT_RXSIG_OFFHOOK DAHDI_RXSIG_OFFHOOK
#define ZT_RXSIG_ONHOOK DAHDI_RXSIG_ONHOOK
#define ZT_RXSIG_RING DAHDI_RXSIG_RING

#define ZT_TONEDETECT DAHDI_TONEDETECT
#define ZT_TONEDETECT_ON DAHDI_TONEDETECT_ON
#define ZT_TONEDETECT_MUTE DAHDI_TONEDETECT_MUTE

#define ZT_IOMUX_READ DAHDI_IOMUX_READ

#define ZT_FLAG_NOSTDTXRX DAHDI_FLAG_NOSTDTXRX
#define ZT_FLAG_HDLC DAHDI_FLAG_HDLC
#define ZT_FLAG_OPEN DAHDI_FLAG_OPEN
#define ZT_FLAG_NETDEV DAHDI_FLAG_NETDEV
#define ZT_FLAG_RBS DAHDI_FLAG_RBS
#define ZT_FLAG_RUNNING DAHDI_FLAG_RUNNING

#define ZT_CONFIG_NOTOPEN DAHDI_CONFIG_NOTOPEN
#define ZT_CONFIG_HDB3 DAHDI_CONFIG_HDB3
#define ZT_CONFIG_CCS DAHDI_CONFIG_CCS
#define ZT_CONFIG_CRC4 DAHDI_CONFIG_CRC4
#define ZT_CONFIG_AMI DAHDI_CONFIG_AMI
#define ZT_CONFIG_B8ZS DAHDI_CONFIG_B8ZS
#define ZT_CONFIG_D4 DAHDI_CONFIG_D4
#define ZT_CONFIG_ESF DAHDI_CONFIG_ESF

#define ZT_ABIT DAHDI_ABIT
#define ZT_BBIT DAHDI_BBIT
#define ZT_CBIT DAHDI_CBIT
#define ZT_DBIT DAHDI_DBIT

#define ZT_ALARM_NONE DAHDI_ALARM_NONE
#define ZT_ALARM_RED DAHDI_ALARM_RED
#define ZT_ALARM_YELLOW DAHDI_ALARM_YELLOW
#define ZT_ALARM_BLUE DAHDI_ALARM_BLUE
#define ZT_ALARM_NOTOPEN DAHDI_ALARM_NOTOPEN
#define ZT_ALARMSETTLE_TIME DAHDI_ALARMSETTLE_TIME
#define ZT_ALARM_RECOVER DAHDI_ALARM_RECOVER

#define ZT_LAW_ALAW DAHDI_LAW_ALAW
#define ZT_LAW_MULAW DAHDI_LAW_MULAW
#define ZT_XLAW	DAHDI_XLAW

#define ZT_MAINT_REMOTELOOP DAHDI_MAINT_REMOTELOOP
#define ZT_MAINT_NONE DAHDI_MAINT_NONE
#define ZT_MAINT_LOCALLOOP DAHDI_MAINT_LOCALLOOP
#define ZT_MAINT_LOOPUP DAHDI_MAINT_LOOPUP
#define ZT_MAINT_LOOPDOWN DAHDI_MAINT_LOOPDOWN
#define ZT_MAINT_LOOPSTOP DAHDI_MAINT_LOOPSTOP

#define ZT_SIG_NONE DAHDI_SIG_NONE
#define ZT_SIG_CLEAR DAHDI_SIG_CLEAR
#define ZT_SIG_EM DAHDI_SIG_EM
#define ZT_SIG_EM_E1 DAHDI_SIG_EM_E1
#define ZT_SIG_FXSLS DAHDI_SIG_FXSLS
#define ZT_SIG_FXSGS DAHDI_SIG_FXSGS
#define ZT_SIG_FXSKS DAHDI_SIG_FXSKS
#define ZT_SIG_FXOLS DAHDI_SIG_FXOLS
#define ZT_SIG_FXOGS DAHDI_SIG_FXOGS
#define ZT_SIG_FXOKS DAHDI_SIG_FXOKS
#define ZT_SIG_CAS DAHDI_SIG_CAS
#define ZT_SIG_DACS_RBS DAHDI_SIG_DACS_RBS
#define ZT_SIG_HARDHDLC DAHDI_SIG_HARDHDLC
#define ZT_SIG_HDLCRAW DAHDI_SIG_HDLCRAW
#define ZT_SIG_HDLCFCS DAHDI_SIG_HDLCFCS
#define ZT_SIG_HDLCNET DAHDI_SIG_HDLCNET
#define ZT_SIG_SLAVE DAHDI_SIG_SLAVE
#define ZT_SIG_DACS DAHDI_SIG_DACS
#define ZT_SIG_SF DAHDI_SIG_SF
#define ZT_SIG_MTP2 DAHDI_SIG_MTP2

#define ZT_LIN2X DAHDI_LIN2X

// data types
#define __zt_mulaw __dahdi_mulaw
#define zt_span dahdi_span
#define zt_chan dahdi_chan
#define zt_lineconfig dahdi_lineconfig
#define zt_txsig_t enum dahdi_txsig

// functions
#define zt_rbsbits dahdi_rbsbits
#define zt_alarm_notify dahdi_alarm_notify
#define zt_receive dahdi_receive
#define zt_transmit dahdi_transmit
#define zt_ec_chunk dahdi_ec_chunk
#define zt_register dahdi_register
#define zt_unregister dahdi_unregister
#define zt_hdlc_getbuf dahdi_hdlc_getbuf
#define zt_hooksig dahdi_hooksig
#define zt_ec_span dahdi_ec_span
#define zt_qevent_lock dahdi_qevent_lock


# define WP_ZT_QEVENT_LOCK(chan, event)	dahdi_qevent_lock((chan),(event))

#else
// zaptel is present
// we will keep the same old names in wanpipe code, I thought of changing them
// to something like WP_XX instead of ZT_XX, but I don't see any benefit on it
// and would make this file bigger 
#include <zaptel.h>


# define WP_ZT_QEVENT_LOCK(chan, event)	zt_qevent_lock(&(chan),(event))
#endif

#endif	/* __ZAPCOMPAT_H */
