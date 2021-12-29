/*****************************************************************************
* wanpipe_tdm_api.h 
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
*
* Jul 25, 2006	David Rokhvarg	<davidr@sangoma.com>	Ported to Windows.
*****************************************************************************/

#ifndef __WANPIPE_TDM_API_H_
#define __WANPIPE_TDM_API_H_ 

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <net/wanpipe_includes.h>
# include <net/wanpipe_defines.h>
# include <net/wanpipe.h>
# include <net/wanpipe_codec_iface.h>
#elif defined(__WINDOWS__)
# if defined(__KERNEL__)
#  include <wanpipe_includes.h>
#  include <wanpipe_defines.h>
#  include <wanpipe.h>
# endif
# include <wanpipe_codec_iface.h>
#else
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe.h>
# include <linux/if_wanpipe.h>
# include <linux/wanpipe_codec_iface.h>
#endif


#if defined(__WINDOWS__)
typedef HANDLE sng_fd_t;
#else
typedef int sng_fd_t;
#endif

enum wanpipe_tdm_api_cmds {

	SIOC_WP_TDM_GET_USR_MTU_MRU,	/* 0x00 */

	SIOC_WP_TDM_SET_USR_PERIOD,	/* 0x01 */
	SIOC_WP_TDM_GET_USR_PERIOD,	/* 0x02 */
	
	SIOC_WP_TDM_SET_HW_MTU_MRU,	/* 0x03 */
	SIOC_WP_TDM_GET_HW_MTU_MRU,	/* 0x04 */

	SIOC_WP_TDM_SET_CODEC,		/* 0x05 */
	SIOC_WP_TDM_GET_CODEC,		/* 0x06 */

	SIOC_WP_TDM_SET_POWER_LEVEL,	/* 0x07 */
	SIOC_WP_TDM_GET_POWER_LEVEL,	/* 0x08 */

	SIOC_WP_TDM_TOGGLE_RX,		/* 0x09 */
	SIOC_WP_TDM_TOGGLE_TX,		/* 0x0A */

	SIOC_WP_TDM_GET_HW_CODING,	/* 0x0B */
	SIOC_WP_TDM_SET_HW_CODING,	/* 0x0C */

	SIOC_WP_TDM_GET_FULL_CFG,	/* 0x0D */

	SIOC_WP_TDM_SET_EC_TAP,		/* 0x0E */
	SIOC_WP_TDM_GET_EC_TAP,		/* 0x0F */
	
	SIOC_WP_TDM_ENABLE_RBS_EVENTS,	/* 0x10 */
	SIOC_WP_TDM_DISABLE_RBS_EVENTS,	/* 0x11 */
	SIOC_WP_TDM_WRITE_RBS_BITS,	/* 0x12 */
	
	SIOC_WP_TDM_GET_STATS,		/* 0x13 */
	SIOC_WP_TDM_FLUSH_BUFFERS,	/* 0x14 */
	
	SIOC_WP_TDM_READ_EVENT,		/* 0x15 */
	
	SIOC_WP_TDM_ENABLE_DTMF_EVENTS,	/* 0x16 */
	SIOC_WP_TDM_DISABLE_DTMF_EVENTS,	/* 0x17 */
	
	SIOC_WP_TDM_ENABLE_RM_DTMF_EVENTS,	/* 0x18 */
	SIOC_WP_TDM_DISABLE_RM_DTMF_EVENTS,	/* 0x19 */
	
	SIOC_WP_TDM_ENABLE_RXHOOK_EVENTS,	/* 0x1A */
	SIOC_WP_TDM_DISABLE_RXHOOK_EVENTS,	/* 0x1B */
	
	SIOC_WP_TDM_ENABLE_RING_DETECT_EVENTS,
	SIOC_WP_TDM_DISABLE_RING_DETECT_EVENTS,
	
	SIOC_WP_TDM_ENABLE_RING_TRIP_DETECT_EVENTS,
	SIOC_WP_TDM_DISABLE_RING_TRIP_DETECT_EVENTS,
	
	SIOC_WP_TDM_TXSIG_KEWL,
	SIOC_WP_TDM_EVENT_TXSIG_START,
	SIOC_WP_TDM_EVENT_TXSIG_OFFHOOK,
	SIOC_WP_TDM_EVENT_TXSIG_ONHOOK,
	SIOC_WP_TDM_EVENT_ONHOOKTRANSFER,
	SIOC_WP_TDM_EVENT_SETPOLARITY,

	SIOC_WP_TDM_SET_RX_GAINS,
	SIOC_WP_TDM_SET_TX_GAINS,
	SIOC_WP_TDM_CLEAR_RX_GAINS,
	SIOC_WP_TDM_CLEAR_TX_GAINS,

	SIOC_WP_TDM_GET_FE_ALARMS,
	SIOC_WP_TDM_ENABLE_HWEC,
	SIOC_WP_TDM_DISABLE_HWEC,
	
	SIOC_WP_TDM_NOTSUPP		/*  */


};

enum wanpipe_tdm_api_events {
	WP_TDM_EVENT_RBS,
	WP_TDM_EVENT_DTMF,	
	WP_TDM_EVENT_NONE,
	WP_TDM_EVENT_RXHOOK,
	WP_TDM_EVENT_RING,
	WP_TDM_EVENT_TONE,
	WP_TDM_EVENT_RING_DETECT,
	WP_TDM_EVENT_RING_TRIP,
	WP_TDM_EVENT_TXSIG_KEWL,
	WP_TDM_EVENT_TXSIG_START,
	WP_TDM_EVENT_TXSIG_OFFHOOK,
	WP_TDM_EVENT_TXSIG_ONHOOK,
	WP_TDM_EVENT_ONHOOKTRANSFER,
	WP_TDM_EVENT_SETPOLARITY,
	WP_TDM_EVENT_FE_ALARM
};

#define WPTDM_A_BIT 			WAN_RBS_SIG_A
#define WPTDM_B_BIT 			WAN_RBS_SIG_B
#define WPTDM_C_BIT 			WAN_RBS_SIG_C
#define WPTDM_D_BIT 			WAN_RBS_SIG_D
 
#define WP_TDMAPI_EVENT_RXHOOK_OFF	0x01
#define WP_TDMAPI_EVENT_RXHOOK_ON	0x02

#define WP_TDMAPI_EVENT_RING_PRESENT	0x01
#define WP_TDMAPI_EVENT_RING_STOP	0x02


typedef struct {
	union {
		struct {
			unsigned char	_event_type;
			unsigned char	_rbs_rx_bits;
			unsigned int	_time_stamp;
			u_int16_t	channel;
			union {
				struct {
                                	u_int8_t	alarm;
				}fe;
				struct {
					u_int8_t	rbs_bits;
				}rbs;
				struct {
					u_int8_t	state;
				}rxhook;
				struct {
					u_int8_t	state;
				}ring;
				struct {
					u_int8_t	digit;	/* DTMF: digit  */
					u_int8_t	port;	/* DTMF: SOUT/ROUT */
					u_int8_t	type;	/* DTMF: PRESET/STOP */
				}dtmf;
				u_int16_t	polarity;
				u_int16_t	ohttimer;
			} u_event;
		}wp_event;
		struct {
			unsigned char	_rbs_rx_bits;
			unsigned int	_time_stamp;
		} wp_rx;
		unsigned char	reserved[16];
	}wp_rx_hdr_u;
#define wp_tdm_api_event_type		wp_rx_hdr_u.wp_event._event_type
#define wp_tdm_api_event_rbs_rx_bits 	wp_rx_hdr_u.wp_event._rbs_rx_bits
#define wp_tdm_api_event_time_stamp 	wp_rx_hdr_u.wp_event._time_stamp
#define wp_tdm_api_event_channel 	wp_rx_hdr_u.wp_event.channel
#define wp_tdm_api_event_rxhook_state 	wp_rx_hdr_u.wp_event.u_event.rxhook.state
#define wp_tdm_api_event_ring_state 	wp_rx_hdr_u.wp_event.u_event.ring.state
#define wp_tdm_api_event_dtmf_digit 	wp_rx_hdr_u.wp_event.u_event.dtmf.digit
#define wp_tdm_api_event_dtmf_type 	wp_rx_hdr_u.wp_event.u_event.dtmf.type
#define wp_tdm_api_event_dtmf_port 	wp_rx_hdr_u.wp_event.u_event.dtmf.port
#define wp_tdm_api_event_ohttimer 	wp_rx_hdr_u.wp_event.u_event.ohttimer
#define wp_tdm_api_event_polarity 	wp_rx_hdr_u.wp_event.u_event.polarity
#define wp_tdm_api_event_fe_alarm 	wp_rx_hdr_u.wp_event.u_event.fe.alarm
} wp_tdm_api_rx_hdr_t;

typedef struct {
        wp_tdm_api_rx_hdr_t	hdr;
        unsigned char  		data[1];
} wp_tdm_api_rx_element_t;

typedef struct {
	union {
		struct {
			unsigned char	_rbs_rx_bits;
			unsigned int	_time_stamp;
		}wp_tx;
		unsigned char	reserved[16];
	}wp_tx_hdr_u;
#define wp_api_time_stamp 	wp_tx_hdr_u.wp_tx._time_stamp
} wp_tdm_api_tx_hdr_t;

typedef struct {
        wp_tdm_api_tx_hdr_t	hdr;
        unsigned char  		data[1];
} wp_tdm_api_tx_element_t;



typedef struct wp_tdm_chan_stats
{
	unsigned int	rx_packets;		/* total packets received	*/
	unsigned int	tx_packets;		/* total packets transmitted	*/
	unsigned int	rx_bytes;		/* total bytes received 	*/
	unsigned int	tx_bytes;		/* total bytes transmitted	*/
	unsigned int	rx_errors;		/* bad packets received		*/
	unsigned int	tx_errors;		/* packet transmit problems	*/
	unsigned int	rx_dropped;		/* no space in linux buffers	*/
	unsigned int	tx_dropped;		/* no space available in linux	*/
	unsigned int	multicast;		/* multicast packets received	*/
#if !defined(__WINDOWS__)
	unsigned int	collisions;
#endif
	/* detailed rx_errors: */
	unsigned int	rx_length_errors;
	unsigned int	rx_over_errors;		/* receiver ring buff overflow	*/
	unsigned int	rx_crc_errors;		/* recved pkt with crc error	*/
	unsigned int	rx_frame_errors;	/* recv'd frame alignment error */
#if !defined(__WINDOWS__)
	unsigned int	rx_fifo_errors;		/* recv'r fifo overrun		*/
#endif
	unsigned int	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
#if !defined(__WINDOWS__)
	unsigned int	tx_aborted_errors;
	unsigned int	tx_carrier_errors;
#endif
	unsigned int	tx_fifo_errors;
	unsigned int	tx_heartbeat_errors;
	unsigned int	tx_window_errors;
	
}wp_tdm_chan_stats_t;          


 
typedef struct wanpipe_tdm_api_cmd{
	unsigned int cmd;
	unsigned int hw_tdm_coding;	/* Set/Get HW TDM coding: uLaw muLaw */
	unsigned int hw_mtu_mru;	/* Set/Get HW TDM MTU/MRU */
	unsigned int usr_period;	/* Set/Get User Period in ms */
	unsigned int tdm_codec;		/* Set/Get TDM Codec: SLinear */
	unsigned int power_level;	/* Set/Get Power level treshold */
	unsigned int rx_disable;	/* Enable/Disable Rx */
	unsigned int tx_disable;	/* Enable/Disable Tx */		
	unsigned int usr_mtu_mru;	/* Set/Get User TDM MTU/MRU */
	unsigned int ec_tap;		/* Echo Cancellation Tap */
	unsigned int rbs_poll;		/* Enable/Disable RBS Polling */
	unsigned int rbs_rx_bits;	/* Rx RBS Bits */
	unsigned int rbs_tx_bits;	/* Tx RBS Bits */
	unsigned int hdlc;		/* HDLC based device */
	unsigned int idle_flag;		/* IDLE flag to Tx */
	unsigned int fe_alarms;		/* FE Alarms detected */
	wp_tdm_chan_stats_t stats;	/* TDM Statistics */
	wp_tdm_api_rx_hdr_t event;	/* TDM Event */
	unsigned int data_len;
        void *data;	
}wanpipe_tdm_api_cmd_t;

typedef struct wanpipe_tdm_api_event{
	int (*wp_rbs_event)(sng_fd_t fd, unsigned char rbs_bits);
	int (*wp_dtmf_event)(sng_fd_t fd, unsigned char dtmf, unsigned char type, unsigned char port);
	int (*wp_rxhook_event)(sng_fd_t fd, unsigned char hook_state);
	int (*wp_rxring_event)(sng_fd_t fd, unsigned char ring_state);
	int (*wp_ringtrip_event)(sng_fd_t fd, unsigned char ring_state);
	int (*wp_fe_alarm_event)(sng_fd_t fd, unsigned char fe_alarm_event);
}wanpipe_tdm_api_event_t; 

typedef struct wanpipe_tdm_api{
	wanpipe_tdm_api_cmd_t	wp_tdm_cmd;
	wanpipe_tdm_api_event_t wp_tdm_event;
}wanpipe_tdm_api_t;




#ifdef WAN_KERNEL

/* Maximum API Len = 200ms = 1600 */
#define WP_TDM_API_MAX_LEN 	8*200 
#define WP_TDM_API_CHUNK_SZ 	8

enum {
	WP_TDM_HDLC_TX,
	WP_TDM_DOWN
};

#if defined(__WINDOWS__)

#define	ssize_t unsigned int
#define loff_t	unsigned int

struct file{
	void *private_data;
	unsigned int span;
	unsigned int chan;
};

struct inode{
	char dummy;
};

struct poll_table_struct{
	char dummy;
};

struct file_operations {
    int		(*open)			(struct inode *, struct file *);
	int		(*release)		(struct inode *, struct file *);
    int		(*ioctl)		(struct inode *, struct file *, unsigned int, unsigned long);
    ssize_t (*read)			(struct file *, char *, size_t, loff_t *);
    ssize_t (*write)		(struct file *, const char *, size_t, loff_t *);
    unsigned int (*poll)	(struct file *, struct poll_table_struct *);
};

struct msghdr{
	void *msg_iov;
	int msg_namelen;
	int msg_iovlen;
};

struct iovec{
	char dummy;
};

#endif/* #if defined(__WINDOWS__) */

typedef struct wanpipe_tdm_api_dev {

	u32 	init;
	void 	*chan;
	void 	*card;
	char 	name[WAN_IFNAME_SZ];
	wan_spinlock_t lock;
	
	u32	used;
	u32	tdm_span;
	u32	tdm_chan;
	u8	state;
	u8	hdlc_framing;
	u32	active_ch;		/* ALEX */
	
	wanpipe_tdm_api_cmd_t	cfg;

	wan_skb_queue_t 	wp_rx_list;
	wan_skb_queue_t 	wp_rx_free_list;
	netskb_t 		*rx_skb;
	
	wan_skb_queue_t 	wp_tx_list;
	wan_skb_queue_t 	wp_tx_free_list;
	netskb_t 		*tx_skb; 
	
	wan_skb_queue_t 	wp_event_list;
	
	wp_tdm_api_rx_hdr_t	*rx_hdr;
	wp_tdm_api_tx_hdr_t	tx_hdr;
	
	u32			poll_event;
#if !defined(__WINDOWS__)
	wait_queue_head_t 	poll_wait;
#endif
	
	u32 	rbs_poll_cnt;
	u32			busy;
	u32			tx_q_len;
	u32 			critical;
	u32			master;
	
	/* Used For non channelized drivers */
	u8			rx_data[WP_TDM_API_CHUNK_SZ];
	u8			tx_data[WP_TDM_API_CHUNK_SZ];

	u8* rx_gain;
	u8* tx_gain;
	
#if 0
	int (*get_hw_mtu_mru)(void *chan);
	int (*set_hw_mtu_mru)(void *chan, u32 hw_mtu_mru);
	int (*get_usr_mtu_mru)(void *chan);
	int (*set_usr_mtu_mru)(void *chan, u32 usr_mtu_mru);
#endif	
	 
	int (*event_ctrl)(void *chan, wan_event_ctrl_t*);
	int (*read_rbs_bits)(void *chan, u32 ch, u8 *rbs_bits);
	int (*write_rbs_bits)(void *chan, u32 ch, u8 rbs_bits);
	int (*write_hdlc_frame)(void *chan, netskb_t *skb);


	 /* Hash pointers used to implement
	 * SPAN/CHAN to Device mapping */
	u32 hash_init;
	struct wanpipe_tdm_api_dev **hash_pprev;
	struct wanpipe_tdm_api_dev *hash_next; 

#if defined(__WINDOWS__)
	u32 original_active_ch;
#endif
}wanpipe_tdm_api_dev_t;

static __inline int is_tdm_api(void *chan, wanpipe_tdm_api_dev_t *tdm_api)
{

	if (wan_test_bit(0,&tdm_api->init)) {
		return 1;
	}
	
	if (tdm_api->chan == chan){
		return 1;
	}
	return 0;
}
static __inline int is_tdm_api_stopped(wanpipe_tdm_api_dev_t *tdm_api)
{
	return wan_test_bit(0,&tdm_api->busy);
}

static __inline void wp_tdm_api_stop(wanpipe_tdm_api_dev_t *tdm_api)
{
	wan_set_bit(0,&tdm_api->busy);
}

static __inline void wp_tdm_api_start(wanpipe_tdm_api_dev_t *tdm_api)
{
	wan_clear_bit(0,&tdm_api->busy);
}

extern int wanpipe_tdm_api_rx_tx (wanpipe_tdm_api_dev_t *tdm_api, 
				 u8 *rx_data, u8 *tx_data, int len);	  
extern int wanpipe_tdm_api_reg(wanpipe_tdm_api_dev_t *tdm_api);
extern int wanpipe_tdm_api_unreg(wanpipe_tdm_api_dev_t *tdm_api);
extern int wanpipe_tdm_api_update_state(wanpipe_tdm_api_dev_t *tdm_api, int state);
extern int wanpipe_tdm_api_rx_hdlc (wanpipe_tdm_api_dev_t *tdm_api, netskb_t *skb);
extern int wanpipe_tdm_api_kick(wanpipe_tdm_api_dev_t *tdm_api);

		
/*===============================================================
 * wphash_sar_dev
 * wpunhash_sar_dev
 * wp_find_sar_dev_by_vpivci
 *
 *  Hashing functions used to map SPAN/CHAN to TDM API devices
 */
#define WP_TDMAPI_HASH_SZ 	(4096 >> 2)
#define vpivci_hashfn(x)	((((x) >> 8) ^ (x)) & (WP_TDMAPI_HASH_SZ - 1))    

static inline wanpipe_tdm_api_dev_t *wp_find_tdm_api_dev(void **hash, 
		                               unsigned int key,
					       unsigned int span,
					       unsigned int chan)
{
	wanpipe_tdm_api_dev_t *p, **htable;

	htable = (wanpipe_tdm_api_dev_t **)&hash[vpivci_hashfn(key)];
	
	for(p = *htable; p ;p = p->hash_next){
		if (p->tdm_span == span && p->tdm_chan == chan){
			break;
		}
	}

	return p;
}
 
static inline int wphash_tdm_api_dev(void **hash, wanpipe_tdm_api_dev_t *p, int hashid)
{
	wanpipe_tdm_api_dev_t **htable = (wanpipe_tdm_api_dev_t **)&hash[vpivci_hashfn(hashid)];

	
	if (wan_test_bit(0,&p->hash_init)){
		DEBUG_EVENT("%s: Critical Error: Init Sar in wphash_sar_dev!\n",
				p->name);
		return -1;
	}	

	if (wp_find_tdm_api_dev(hash, hashid, p->tdm_span, p->tdm_chan)){
		DEBUG_EVENT("%s:%s: Error: device SPAN=%i CHAN=%i already in use!\n",
				__FUNCTION__,p->name,p->tdm_span,p->tdm_chan);
		return -1;
	}
	
	if((p->hash_next = *htable) != NULL)
		(*htable)->hash_pprev = &p->hash_next;
	
	*htable = p;
	p->hash_pprev = htable;

	wan_set_bit(0,&p->hash_init);
	return 0;
}

static inline void wpunhash_tdm_api_dev(wanpipe_tdm_api_dev_t *p)
{
	if (p->hash_next)
		p->hash_next->hash_pprev = p->hash_pprev;
	
	*p->hash_pprev = p->hash_next;

	wan_clear_bit(0,&p->init);
}

static inline int wp_tdmapi_check_mtu(void* pcard, unsigned long timeslot_map, int chunksz, int *mtu)
{
	sdla_t	*card = (sdla_t*)pcard;
	int	x, num_of_channels = 0, max_channels;

	max_channels = GET_TE_CHANNEL_RANGE(&card->fe);
	for (x = 0; x < max_channels; x++) {
		if (wan_test_bit(x,&timeslot_map)){
			num_of_channels++;
		}
	}
	*mtu = chunksz * num_of_channels;
	return 0;
}
             
		
#endif

#endif
