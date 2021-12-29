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
# include <net/wanpipe_tdm_api_iface.h>
#elif defined(__WINDOWS__)
# if defined(__KERNEL__)
#  include <wanpipe_includes.h>
#  include <wanpipe_defines.h>
#  include <wanpipe_tdm_api_iface.h>
#  include <wanpipe.h>

enum{
	TDMAPI_BUFFER_NO_CODEC=1,
	TDMAPI_BUFFER_HDLC_DATA,
	TDMAPI_BUFFER_ERROR,
	TDMAPI_BUFFER_READY,
	TDMAPI_BUFFER_ACCEPTED
};

#define inline __inline
# endif
# include <wanpipe_codec_iface.h>

#else
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe.h>
# include <linux/if_wanpipe.h>
# include <linux/wanpipe_codec_iface.h>
# include <linux/wanpipe_tdm_api_iface.h>
#endif



#ifdef WAN_KERNEL

#if !defined(__WINDOWS__)
#define WP_TDM_API_MAX_LEN 	8*200
#endif

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
	
	wan_ticks_t	 	rbs_poll_cnt;
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

	uint8_t		dtmfsupport;
	uint8_t		loop;
	
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

static inline int wp_tdmapi_get_span(sdla_t *card)
{
	return card->tdmv_conf.span_no;
}
		
#endif

#endif
