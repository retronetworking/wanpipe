/*****************************************************************************
* aft_core_utils.h
* 		
* 		WANPIPE(tm) AFT CORE Hardware Support - Utilities
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003-2008 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================*/


#ifndef __AFT_CORE_UTILS_H_
#define __AFT_CORE_UTILS_H_

#include "aft_core_private.h"

int 	aft_read_security(sdla_t *card);
int 	aft_front_end_mismatch_check(sdla_t * card);
int 	aft_realign_skb_pkt(private_area_t *chan, netskb_t *skb);
void 	aft_wdt_set(sdla_t *card, unsigned char val);
void 	aft_wdt_reset(sdla_t *card);
int 	process_udp_mgmt_pkt(sdla_t* card, netdevice_t* dev,
				private_area_t*,
				int local_dev);

void 	aft_channel_txdma_ctrl(sdla_t *card, private_area_t *chan, int on);
void 	aft_channel_rxdma_ctrl(sdla_t *card, private_area_t *chan, int on);
void 	aft_channel_txintr_ctrl(sdla_t *card, private_area_t *chan, int on);
void 	aft_channel_rxintr_ctrl(sdla_t *card, private_area_t *chan, int on);

int 	aft_alloc_rx_dma_buff(sdla_t *card, private_area_t *chan, int num, int irq);
int 	aft_init_requeue_free_skb(private_area_t *chan, netskb_t *skb);

void	 aft_tx_dma_skb_init(private_area_t *chan, netskb_t *skb);

int 	aft_tslot_sync_ctrl(sdla_t *card, private_area_t *chan, int mode);

int 	aft_tdmapi_mtu_check(sdla_t *card_ptr, unsigned int *mtu);

int 	aft_devel_ioctl(sdla_t *card, struct ifreq *ifr);

unsigned char aft_write_ec (void *pcard, unsigned short off, unsigned char value);
unsigned char aft_read_cpld(sdla_t *card, unsigned short cpld_off);
unsigned char aft_read_ec (void *pcard, unsigned short off);

int 	aft_read(sdla_t *card, wan_cmd_api_t *api_cmd);
int 	aft_write_cpld(void *pcard, unsigned short off,unsigned char data);

#if defined(__LINUX__)
int if_change_mtu(netdevice_t *dev, int new_mtu);
#endif

int 	update_comms_stats(sdla_t* card);
int 	aft_handle_clock_master (sdla_t *card_ptr);
void 	wanpipe_wake_stack(private_area_t* chan);
int 	aft_find_master_if_and_dchan(sdla_t *card, int *master_if, u32 active_ch);

void 	aft_set_ss7_force_rx(sdla_t *card, private_area_t *chan);
void 	aft_clear_ss7_force_rx(sdla_t *card, private_area_t *chan);
int 	update_comms_stats(sdla_t* card);
int 	aft_find_master_if_and_dchan(sdla_t *card, int *master_if,u32 active_ch);
int 	aft_hwec_config (sdla_t *card, private_area_t *chan, wanif_conf_t *conf, int ctrl);
int 	aft_fifo_intr_ctrl(sdla_t *card, int ctrl);
int 	aft_tdm_intr_ctrl(sdla_t *card, int ctrl);
int 	aft_tdm_ring_rsync(sdla_t *card);

int 	aft_core_send_serial_oob_msg (sdla_t *card);
int 	wan_user_process_udp_mgmt_pkt(void* card_ptr, void* chan_ptr, void *udata);




/*=================================================================
 * Used for debugging only
 *================================================================*/


#if 0
void 	wp_tdmv_api_chan_rx_tx(sdla_t *card,
			      private_area_t *chan,
			      unsigned char *rxdata, unsigned char *tx_data);
void 	wp_tdmv_api_rx_tx (sdla_t *card, private_area_t *chan);
#endif

#if 0
void 	aft_list_descriptors(private_area_t *chan);
#endif
#if 0
void	aft_list_dma_chain_regs(sdla_t *card);
#endif

#if 0
void 	aft_list_tx_descriptors(private_area_t *chan);
#endif
#if 0
void 	aft_display_chain_history(private_area_t *chan);
void 	aft_chain_history(private_area_t *chan,u8 end, u8 cur, u8 begin, u8 loc);
#endif 

#endif

