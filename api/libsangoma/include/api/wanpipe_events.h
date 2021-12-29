/******************************************************************************
 * wanpipe_events.h	
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

#ifndef __WANPIPE_EVENTS_H__
# define __WANPIPE_EVENTS_H__

/* DTMF event tone type: present or stop */
#define WAN_EC_TONE_PRESENT		0x01
#define WAN_EC_TONE_STOP		0x02
#define WAN_EC_DECODE_TONE_TYPE(type)					\
		(type == WAN_EC_TONE_PRESENT)	? "Present"	:	\
		(type == WAN_EC_TONE_STOP)	? "Stop" 	:	\
							"Unknown"

/* channel port (sout, rout, sin, rin) */
#define WAN_EC_CHANNEL_PORT_SOUT	0x01
#define WAN_EC_CHANNEL_PORT_SIN		0x02
#define WAN_EC_CHANNEL_PORT_ROUT	0x03
#define WAN_EC_CHANNEL_PORT_RIN		0x04

#define WAN_EVENT_RXHOOK_OFF		0x01
#define WAN_EVENT_RXHOOK_ON		0x02
#define WAN_EVENT_RXHOOK_DECODE(hook)					\
		((hook) == WAN_EVENT_RXHOOK_OFF) ? "Off-hook" :		\
		((hook) == WAN_EVENT_RXHOOK_ON)  ? "On-hook" :		\
							"Unknown"

#define WAN_EVENT_RING_PRESENT		0x01
#define WAN_EVENT_RING_STOP		0x02
#define WAN_EVENT_RING_DECODE(ring)					\
		((ring) == WAN_EVENT_RING_PRESENT) ? "Ring Present" :	\
		((ring) == WAN_EVENT_RING_STOP)	   ? "Ring Stop" :	\
							"Unknown"
#define WAN_EVENT_RING_TRIP_PRESENT		0x01
#define WAN_EVENT_RING_TRIP_STOP		0x02
#define WAN_EVENT_RING_TRIP_DECODE(ring)					\
		((ring) == WAN_EVENT_RING_TRIP_PRESENT) ? "RingTrip Present" :	\
		((ring) == WAN_EVENT_RING_TRIP_STOP)	? "RingTrip Stop" :	\
							"Unknown"

#if defined(WAN_KERNEL)

/* Global Event defines 			*/
#define WAN_EVENT_ENABLE	0x01
#define WAN_EVENT_DISABLE	0x02
#define WAN_EVENT_MODE_DECODE(mode)					\
		((mode) == WAN_EVENT_ENABLE) ? "Enable" :		\
		((mode) == WAN_EVENT_DISABLE) ? "Disable" :		\
						"(Unknown mode)"

/* Event type list */
#define WAN_EVENT_EC_DTMF		0x0001
#define WAN_EVENT_RM_POWER		0x0002
#define WAN_EVENT_RM_LC			0x0003
#define WAN_EVENT_RM_RING_TRIP		0x0004
#define WAN_EVENT_RM_DTMF		0x0005
#define WAN_EVENT_TE_RBS		0x0006
#define WAN_EVENT_RM_RING		0x0007
#define WAN_EVENT_RM_TONE		0x0008
#define WAN_EVENT_RM_RING_DETECT	0x0009
#define WAN_EVENT_RM_TXSIG_START	0x000A
#define WAN_EVENT_RM_TXSIG_OFFHOOK	0x000B
#define WAN_EVENT_RM_TXSIG_ONHOOK	0x000C
#define WAN_EVENT_RM_TXSIG_KEWL		0x000D
#define WAN_EVENT_RM_ONHOOKTRANSFER	0x000E
#define WAN_EVENT_RM_SETPOLARITY	0x000F
#define WAN_EVENT_RM_SET_ECHOTUNE	0x0010

#define WAN_EVENT_TYPE_DECODE(type)					\
		((type) == WAN_EVENT_EC_DTMF)		? "EC DTMF"  :		\
		((type) == WAN_EVENT_RM_POWER)		? "RM Power Alarm" :	\
		((type) == WAN_EVENT_RM_LC)		? "RM Loop Closure" :	\
		((type) == WAN_EVENT_RM_RING_TRIP)	? "RM Ring Trip" :	\
		((type) == WAN_EVENT_RM_DTMF)		? "RM DTMF" :		\
		((type) == WAN_EVENT_TE_RBS)		? "TE RBS" :		\
		((type) == WAN_EVENT_RM_RING)		? "RM Ring" :		\
		((type) == WAN_EVENT_RM_TONE)		? "RM Tone" :		\
		((type) == WAN_EVENT_RM_RING_DETECT)	? "RM Ring Detect" :	\
		((type) == WAN_EVENT_RM_TXSIG_START)	? "RM TXSIG Start" :	\
		((type) == WAN_EVENT_RM_TXSIG_OFFHOOK)	? "RM TXSIG Off-hook" :	\
		((type) == WAN_EVENT_RM_TXSIG_ONHOOK)	? "RM TXSIG On-hook" :	\
		((type) == WAN_EVENT_RM_TXSIG_KEWL)	? "RM TXSIG kewlfs" :	\
		((type) == WAN_EVENT_RM_ONHOOKTRANSFER)	? "RM On-hook transfer" :	\
		((type) == WAN_EVENT_RM_SETPOLARITY)	? "RM Set polarity" :	\
		((type) == WAN_EVENT_RM_SET_ECHOTUNE)	? "RM Set echotune" :	\
						"(Unknown type)"

/* tone type list */						
#define	WAN_EVENT_TONE_DIAL		0x01
#define	WAN_EVENT_TONE_BUSY		0x02
#define	WAN_EVENT_TONE_RING		0x03
#define	WAN_EVENT_TONE_CONGESTION	0x04
#define WAN_EVENT_TONE_DECODE(tone)					\
		((tone) == WAN_EVENT_TONE_DIAL)		? "Dial tone" :	\
		((tone) == WAN_EVENT_TONE_BUSY)		? "Busy tone" :	\
		((tone) == WAN_EVENT_TONE_RING)		? "Ring tone" :	\
		((tone) == WAN_EVENT_TONE_CONGESTION)	? "Congestion tone" :	\
						"(Unknown tone)"

/* Event information			*/
typedef struct wan_event_
{
	u_int16_t	type;
	u_int8_t	mode;		/* Enable/Disable */
	int		channel;	/* A200-mod_no, T1/E1-fe chan  */
	unsigned char	digit;		/* DTMF: digit  */
	unsigned char	dtmf_type;	/* DTMF: PRESETN/STOP */
	unsigned char	dtmf_port;	/* DTMF: ROUT/SOUT */

	unsigned char	rxhook;		/* LC: OFF-HOOK or ON-HOOK */

	unsigned char	ring_mode;	/* RingDetect: Present/Stop */
		
} wan_event_t;

/* Event control 			*/
typedef struct wan_event_ctrl_
{
	u_int16_t	type;
	u_int8_t	mode;
	int		mod_no;		/* A200-Remora */	
	unsigned char	ec_dtmf_port;	/* EC DTMF: SOUT or ROUT */
	unsigned long	ts_map;
	u_int8_t	tone;
	int		ohttimer;	/* On-hook transfer */
	int		polarity;	/* SETPOLARITY */
#if !defined(__WINDOWS__)
	WAN_LIST_ENTRY(wan_event_ctrl_)	next;
#endif
} wan_event_ctrl_t;

#endif	/* WAN_KERNEL */

#endif	/* __WANPIPE_EVENTS_H__ */