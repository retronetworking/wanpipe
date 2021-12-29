/*****************************************************************************
* aft_core_utils.c 
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

#include "wanpipe_includes.h"
#include "wanpipe_defines.h"
#include "wanpipe.h"

#include "wanpipe_abstr.h"
#include "if_wanpipe_common.h"    /* Socket Driver common area */
#include "sdlapci.h"
#include "aft_core.h"
#include "wanpipe_iface.h"
#include "wanpipe_tdm_api.h"
#include "sdla_tdmv_dummy.h"

#if defined(__WINDOWS__)
extern int wanec_dev_ioctl(void *data, char *card_devname);
#endif

#ifdef __WINDOWS__
#define wp_os_lock_irq(card,flags)   wan_spin_lock_irq(card,flags)
#define wp_os_unlock_irq(card,flags) wan_spin_unlock_irq(card,flags)
#else
#define wp_os_lock_irq(card,flags)
#define wp_os_unlock_irq(card,flags)
#endif

int aft_read_security(sdla_t *card)
{
	int adptr_security;
	wan_smp_flag_t flags,smp_flags;

	if (card->adptr_subtype == AFT_SUBTYPE_SHARK){
		/* Shark cards are always channelized */
		card->u.aft.security_id=0x01;
		return 0;
	}
	
	card->hw_iface.hw_lock(card->hw,&smp_flags);
	wan_spin_lock_irq(&card->wandev.lock,&flags);
	adptr_security=aft_read_cpld(card,0x09);	
	wan_spin_unlock_irq(&card->wandev.lock,&flags);
	card->hw_iface.hw_unlock(card->hw,&smp_flags);

	adptr_security=(adptr_security>>2)&0x03;
	card->u.aft.security_id=adptr_security;
	
	if (adptr_security == 0x00){
		DEBUG_EVENT("%s: AFT Security: UnChannelised\n",
				card->devname);
	}else if (adptr_security == 0x01){
		DEBUG_EVENT("%s: AFT Security: Channelised\n",
				card->devname);
	}else{
		DEBUG_EVENT("%s: AFT Security: Unknown\n",
				card->devname);		
		return -EINVAL;
	}

	card->u.aft.security_cnt=0;

	return 0;
}


int aft_front_end_mismatch_check(sdla_t * card)
{
	u32 reg;
	
        if (card->u.aft.firm_id == AFT_PMC_FE_CORE_ID) {
		card->hw_iface.bus_read_4(card->hw,AFT_CHIP_CFG_REG, &reg);
	
		if (IS_T1_CARD(card)){
			if (wan_test_bit(AFT_CHIPCFG_TE1_CFG_BIT,&reg)){
				DEBUG_EVENT("%s: Global Cfg Error: Initial front end cfg: E1\n",
					card->devname);
				return -EINVAL;
			}
		}else{
			if (!wan_test_bit(AFT_CHIPCFG_TE1_CFG_BIT,&reg)){
				DEBUG_EVENT("%s: Global Cfg Error: Initial front end cfg: T1\n",
					card->devname);
				return -EINVAL;
			}
		}
	}
	
	return 0;
}

int aft_realign_skb_pkt(private_area_t *chan, netskb_t *skb)
{
	unsigned char *data=wan_skb_data(skb);
	int len = wan_skb_len(skb);
	
	if (len > chan->dma_mru){
		DEBUG_EVENT("%s: Critical error: Tx unalign pkt(%d) > MTU buf(%d)!\n",
				chan->if_name,len,chan->dma_mru);
		return -ENOMEM;
	}

	if (!chan->tx_realign_buf){
		chan->tx_realign_buf=wan_malloc(chan->dma_mru);
		if (!chan->tx_realign_buf){
			DEBUG_EVENT("%s: Error: Failed to allocate tx memory buf\n",
						chan->if_name);
			return -ENOMEM;
		}else{
			DEBUG_EVENT("%s: AFT Realign buffer allocated Len=%d\n",
						chan->if_name,chan->dma_mru);

		}
	}

	memcpy(chan->tx_realign_buf,data,len);

	wan_skb_init(skb,0);
	wan_skb_trim(skb,0);

	if (wan_skb_tailroom(skb) < len){
		DEBUG_EVENT("%s: Critical error: Tx unalign pkt tail room(%i) < unalign len(%i)!\n",
				chan->if_name,wan_skb_tailroom(skb),len);
		
		return -ENOMEM;
	}

	data=wan_skb_put(skb,len);

	if ((ulong_ptr_t)data & 0x03){
		/* At this point pkt should be realigned. If not
		 * there is something really wrong! */
		return -EINVAL;
	}
	
	memcpy(data,chan->tx_realign_buf,len);

	chan->opstats.Data_frames_Tx_realign_count++;

	return 0;	
}

void aft_wdt_set(sdla_t *card, unsigned char val)
{
#ifndef WAN_NO_INTR
	u8 reg;
	u32 wdt_ctrl_reg=AFT_WDT_1TO4_CTRL_REG+card->wandev.comm_port;
	
	if (card->wandev.comm_port > 3) {
		wdt_ctrl_reg=AFT_WDT_4TO8_CTRL_REG+(card->wandev.comm_port%4);	
	}
	
	card->hw_iface.bus_read_1(card->hw,AFT_PORT_REG(card,wdt_ctrl_reg), &reg);		
	aft_wdt_ctrl_set(&reg,val);
	card->hw_iface.bus_write_1(card->hw,AFT_PORT_REG(card,wdt_ctrl_reg), reg);
#endif
}

void aft_wdt_reset(sdla_t *card)
{
	u8 reg;
	u32 wdt_ctrl_reg=AFT_WDT_1TO4_CTRL_REG+card->wandev.comm_port;
	
	if (card->wandev.comm_port > 3) {
		wdt_ctrl_reg=AFT_WDT_4TO8_CTRL_REG+(card->wandev.comm_port%4);	
	}
	
	card->hw_iface.bus_read_1(card->hw, AFT_PORT_REG(card,wdt_ctrl_reg), &reg);		
	aft_wdt_ctrl_reset(&reg);
	card->hw_iface.bus_write_1(card->hw, AFT_PORT_REG(card,wdt_ctrl_reg), reg);
}


void aft_channel_txdma_ctrl(sdla_t *card, private_area_t *chan, int on)
{
	u32 reg;
	/* Enable TX DMA for Logic Channel */
	card->hw_iface.bus_read_4(card->hw,
                                  AFT_PORT_REG(card,AFT_TX_DMA_CTRL_REG), &reg);
	if (on){
		wan_set_bit(chan->logic_ch_num,&reg);
	}else{
		wan_clear_bit(chan->logic_ch_num,&reg);
	}
	
	card->hw_iface.bus_write_4(card->hw,
                                  AFT_PORT_REG(card,AFT_TX_DMA_CTRL_REG), reg);
	
}
void aft_channel_rxdma_ctrl(sdla_t *card, private_area_t *chan, int on)
{
	u32 reg;
	/* Enable TX DMA for Logic Channel */
	card->hw_iface.bus_read_4(card->hw,
                                  AFT_PORT_REG(card,AFT_RX_DMA_CTRL_REG), &reg);
	if (on){
		wan_set_bit(chan->logic_ch_num,&reg);
	}else{
		wan_clear_bit(chan->logic_ch_num,&reg);
	}
	
	card->hw_iface.bus_write_4(card->hw,
                                  AFT_PORT_REG(card,AFT_RX_DMA_CTRL_REG), reg);
	
}

void aft_channel_txintr_ctrl(sdla_t *card, private_area_t *chan, int on)
{
	u32 reg;
	
	/* Enable Logic Channel TX Interrupts */
	card->hw_iface.bus_read_4(card->hw,
                                  AFT_PORT_REG(card,AFT_TX_DMA_INTR_MASK_REG), &reg);
	if (on){
		wan_set_bit(chan->logic_ch_num,&reg);
	}else{
		wan_clear_bit(chan->logic_ch_num,&reg);

	}
	card->hw_iface.bus_write_4(card->hw,
                                  AFT_PORT_REG(card,AFT_TX_DMA_INTR_MASK_REG), reg);
}

void aft_channel_rxintr_ctrl(sdla_t *card, private_area_t *chan, int on)
{
	u32 reg;
	
	/* Enable Logic Channel TX Interrupts */
	card->hw_iface.bus_read_4(card->hw,
                                  AFT_PORT_REG(card,AFT_RX_DMA_INTR_MASK_REG), &reg);
	if (on){
		wan_set_bit(chan->logic_ch_num,&reg);
	}else{
		wan_clear_bit(chan->logic_ch_num,&reg);

	}
	card->hw_iface.bus_write_4(card->hw,
                                  AFT_PORT_REG(card,AFT_RX_DMA_INTR_MASK_REG), reg);
}


int aft_tslot_sync_ctrl(sdla_t *card, private_area_t *chan, int mode)
{
	u32 dma_ram_reg,reg;

	if (chan->hdlc_eng){
		return 0;
	}
	

	dma_ram_reg=AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	dma_ram_reg+=(chan->logic_ch_num*4);

	card->hw_iface.bus_read_4(card->hw, dma_ram_reg, &reg);

	if (mode){
		wan_set_bit(AFT_DMACHAIN_RX_SYNC_BIT,&reg);
	}else{
		wan_clear_bit(AFT_DMACHAIN_RX_SYNC_BIT,&reg);
	}

	card->hw_iface.bus_write_4(card->hw, dma_ram_reg, reg);
	
	return 0;
}

int aft_init_requeue_free_skb(private_area_t *chan, netskb_t *skb)
{
	WAN_ASSERT(skb == NULL);
	
	wan_skb_init(skb,sizeof(wp_api_hdr_t));
	wan_skb_trim(skb,0);

	wan_skb_queue_tail(&chan->wp_rx_free_list,skb);

	return 0;
}

void aft_tx_dma_skb_init(private_area_t *chan, netskb_t *skb)
{

	if (chan->tx_idle_skb == skb) {
		return;
	}

	if (chan->common.usedby == XMTP2_API) {
		/* Requeue the tx buffer because it came from rx_free_list */
		aft_init_requeue_free_skb(chan, skb);

	} else if (chan->channelized_cfg && !chan->hdlc_eng){
		/* Requeue the tx buffer because it came from rx_free_list */
		aft_init_requeue_free_skb(chan, skb);
	}else{
		wan_aft_skb_defered_dealloc(chan,skb);
	}
}


int aft_alloc_rx_dma_buff(sdla_t *card, private_area_t *chan, int num, int irq)
{
	int i;
	netskb_t *skb;
	wan_smp_flag_t flags;

	for (i=0;i<num;i++){
#if defined(__WINDOWS__)
		if (irq) {
			return -EFAULT;
		} else {
			/* allocate 'regular' skb buffers (NOT for DMA!!) */
			skb = wan_skb_alloc(chan->dma_mru);
		}
#else
		if (chan->channelized_cfg && !chan->hdlc_eng){
#if defined(WANPIPE_64BIT_4G_DMA)
			/* On 64bit Systems greater than 4GB we must
			 * allocated our DMA buffers using GFP_DMA 
			 * flag */
			/* DMA memory must be allocated with ATOMIC flag */
      			skb=__dev_alloc_skb(chan->dma_mru+sizeof(wp_api_hdr_t),GFP_DMA|GFP_ATOMIC);
#else
			if (irq) {
				skb=wan_skb_alloc(chan->dma_mru);
			} else {
				skb=wan_skb_kalloc(chan->dma_mru);
			}
#endif	
		} else {
			if (irq) {
					skb=wan_skb_alloc(chan->dma_mru);
			} else {
			        skb=wan_skb_kalloc(chan->dma_mru); 	
			}	
		}
#endif

		if (!skb){
			DEBUG_EVENT("%s: %s  no rx memory\n",
					chan->if_name,__FUNCTION__);
			return -ENOMEM;
		}

		wan_spin_lock_irq(&card->wandev.lock,&flags);
		wan_skb_queue_tail(&chan->wp_rx_free_list,skb);
		wan_spin_unlock_irq(&card->wandev.lock,&flags);
	}

	return 0;
}


unsigned char aft_read_cpld(sdla_t *card, unsigned short cpld_off)
{
	return aft_hwdev[card->wandev.card_type].aft_read_cpld(card,cpld_off);
}

int aft_write_cpld(void *pcard, unsigned short off,unsigned char data)
{
	sdla_t *card = (sdla_t *)pcard;
	return 	aft_hwdev[card->wandev.card_type].aft_write_cpld(card,off,data);
}

unsigned char aft_write_ec (void *pcard, unsigned short off, unsigned char value)
{
#if defined(WAN_DEBUG_TEST)
	sdla_t	*card = (sdla_t*)pcard;
	DEBUG_TEST("%s: Write Octasic Offset %04X Value %02X!\n",
				card->devname, off, value);
#endif
        return 0;
}

/*============================================================================
 * Read from Octasic board
 */
unsigned char aft_read_ec (void *pcard, unsigned short off)
{
	unsigned char	value = 0x00;
#if defined(WAN_DEBUG_TEST)
	sdla_t	*card = (sdla_t*)pcard;

	DEBUG_TEST("%s: Read Octasic offset %04X Value %02X (temp)!\n",
				card->devname, off, value);
#endif
	return value;
}



int aft_read(sdla_t *card, wan_cmd_api_t *api_cmd)
{
	WAN_ASSERT(card == NULL);
	WAN_ASSERT(api_cmd == NULL);

	if(api_cmd->len == 1){
		if (api_cmd->offset <= 0x3C){
			card->hw_iface.pci_read_config_byte(
					card->hw,
					api_cmd->offset,
					(u8*)&api_cmd->data[0]); 
		}else{
			card->hw_iface.bus_read_1(
					card->hw,
				    api_cmd->offset,
			       	(u8*)&api_cmd->data[0]);
		}
	}else if (api_cmd->len == 2){
		if (api_cmd->offset <= 0x3C){
			card->hw_iface.pci_read_config_word(
					card->hw,
					api_cmd->offset,
					(u16*)&api_cmd->data[0]); 
		}else{
			card->hw_iface.bus_read_2(
					card->hw,
				    api_cmd->offset,
			       	(u16*)&api_cmd->data[0]);
		}
	}else if (api_cmd->len == 4){
		if (api_cmd->offset <= 0x3C){
			card->hw_iface.pci_read_config_dword(
					card->hw,
					api_cmd->offset,
					(u32*)&api_cmd->data[0]); 
		}else{
			WAN_ASSERT(card->hw_iface.bus_read_4 == NULL);
			card->hw_iface.bus_read_4(
					card->hw,
				    api_cmd->offset,
			       	(u32*)&api_cmd->data[0]);
		}
	}else{
		card->hw_iface.peek(card->hw,
					api_cmd->offset,
					&api_cmd->data[0],
					api_cmd->len);
	}		

	DEBUG_REG("%s: Reading Bar%d Offset=0x%X Data=%08X Len=%d\n",
				card->devname,
				api_cmd->bar,
				api_cmd->offset,
				*(u32*)&api_cmd->data[0],
				api_cmd->len);

	return 0;
}

int aft_fe_read(sdla_t *card, wan_cmd_api_t *api_cmd)
{
	wan_smp_flag_t smp_flags;

	card->hw_iface.hw_lock(card->hw,&smp_flags);
	api_cmd->data[0] = (u8)card->fe.read_fe_reg(card, (int)api_cmd->offset);
	card->hw_iface.hw_unlock(card->hw,&smp_flags);

#ifdef DEB_XILINX
	DEBUG_EVENT("%s: Reading Bar%d Offset=0x%X Len=%d Val=%02X\n",
			card->devname,api_cmd->bar,api_cmd->offset,api_cmd->len, api_cmd->data[0]);
#endif

	return 0;
}

int aft_write(sdla_t *card, wan_cmd_api_t *api_cmd)
{

	if (api_cmd->len == 1){
		card->hw_iface.bus_write_1(
				card->hw,
				api_cmd->offset,
				*(u8*)&api_cmd->data[0]);
		DEBUG_REG("%s: Write  Offset=0x%08X Data=0x%02X\n",
				card->devname,api_cmd->offset,
				*(u8*)&api_cmd->data[0]);
	}else if (api_cmd->len == 2){
		card->hw_iface.bus_write_2(
				card->hw,
				api_cmd->offset,
				*(u16*)&api_cmd->data[0]);
		DEBUG_REG("%s: Write  Offset=0x%08X Data=0x%04X\n",
				card->devname,api_cmd->offset,
				*(unsigned short*)&api_cmd->data[0]);
	}else if (api_cmd->len == 4){
		card->hw_iface.bus_write_4(
				card->hw,
				api_cmd->offset,
				*(unsigned int*)&api_cmd->data[0]);
		DEBUG_REG("ADEBUG: %s: Write  Offset=0x%08X Data=0x%08X\n",
			card->devname,api_cmd->offset,
			*(u32*)&api_cmd->data[0]);
	}else{
		card->hw_iface.poke(
				card->hw,
				api_cmd->offset,
				(u8*)&api_cmd->data[0],
				api_cmd->len);
#if 0
		memcpy_toio((unsigned char*)vector,
			(unsigned char*)&api_cmd->data[0], api_cmd->len);
#endif
	}

	return 0;
}

int aft_fe_write(sdla_t *card, wan_cmd_api_t *api_cmd)
{
	wan_smp_flag_t smp_flags;

#ifdef DEB_XILINX
	DEBUG_EVENT("%s: Writting Bar%d Offset=0x%X Len=%d Val=%02X\n",
			card->devname,
			api_cmd->bar,
			api_cmd->offset,
			api_cmd->len,
			api_cmd->data[0]);
#endif


	card->hw_iface.hw_lock(card->hw,&smp_flags);
	card->fe.write_fe_reg (card, (int)api_cmd->offset, (int)api_cmd->data[0]);
	card->hw_iface.hw_unlock(card->hw,&smp_flags);
	
	return 0;

}



int aft_write_bios(sdla_t *card, wan_cmd_api_t *api_cmd)
{

#ifdef DEB_XILINX
	DEBUG_EVENT("Setting PCI 0xX=0x%08lX   0x3C=0x%08X\n",
			(card->wandev.S514_cpu_no[0] == SDLA_CPU_A) ? 0x10 : 0x14,
			card->u.aft.bar,card->wandev.irq);
#endif
	card->hw_iface.pci_write_config_dword(card->hw, 
			(card->wandev.S514_cpu_no[0] == SDLA_CPU_A) ? 0x10 : 0x14,
			(u32)card->u.aft.bar);
	card->hw_iface.pci_write_config_dword(card->hw, 0x3C, card->wandev.irq);
	card->hw_iface.pci_write_config_dword(card->hw, 0x0C, 0x0000ff00);

	return 0;
}

#if 0
extern int OctDrvIoctl(sdla_t*, int cmd, void*);
#endif

int aft_hwec(sdla_t *card, wan_cmd_api_t *api_cmd)
{

#if 0
	if (api_cmd->offset){
		/* Use direct read/write to/from octasic chip */
		if (api_cmd->len){
			/* Write */ 
			aft_write_ec (card, api_cmd->offset, api_cmd->data[0]);
		}else{
			/* Read */
			api_cmd->data[0] = aft_read_ec (card, api_cmd->offset);
			api_cmd->len = 1;
		}
	}else
#endif
	{
#if 0
		OctDrvIoctl(card, api_cmd->cmd, api_cmd->data);
#endif
	}
	
	return 0;
}

int aft_devel_ioctl(sdla_t *card, struct ifreq *ifr)
{
	wan_cmd_api_t*	api_cmd;
	wan_cmd_api_t   api_cmd_struct;
	int 		err = -EINVAL;
	void 		*ifr_data_ptr;

#if defined (__WINDOWS__)
	ifr_data_ptr = ifr;
#else
	if (!ifr || !ifr->ifr_data){
		DEBUG_EVENT("%s: Error: No ifr or ifr_data\n",__FUNCTION__);
		return -EFAULT;
	}
	ifr_data_ptr = ifr->ifr_data;
#endif

	api_cmd = &api_cmd_struct;

	memset(api_cmd, 0, sizeof(wan_cmd_api_t));	

	if(WAN_COPY_FROM_USER(api_cmd, ifr_data_ptr, sizeof(wan_cmd_api_t))){
		return -EFAULT;
	}

	switch(api_cmd->cmd){
	case SIOC_WAN_READ_REG:
		err=aft_read(card, api_cmd);
		break;
	case SIOC_WAN_WRITE_REG:
		err=aft_write(card, api_cmd);
		break;

	case SIOC_WAN_FE_READ_REG:
		err=aft_fe_read(card, api_cmd);
		break;

	case SIOC_WAN_FE_WRITE_REG:
		err=aft_fe_write(card, api_cmd);
		break;

	case SIOC_WAN_SET_PCI_BIOS:
		err=aft_write_bios(card, api_cmd);
		break;

	case SIOC_WAN_EC_REG:
		err = aft_hwec(card, api_cmd);
		break;
#if defined(__WINDOWS__)
	case SIOC_WAN_GET_CARD_TYPE:

		card->hw_iface.getcfg(card->hw, SDLA_ADAPTERTYPE,	 (u16*)&api_cmd->data[0]);
		card->hw_iface.getcfg(card->hw, SDLA_ADAPTERSUBTYPE, (u16*)&api_cmd->data[sizeof(u16)]);

		api_cmd->len = 2*sizeof(u16);
		break;
#endif
	default:
		DEBUG_EVENT("%s(): Error: unsupported cmd: %d\n",__FUNCTION__, api_cmd->cmd);
		break;
	}

	if (WAN_COPY_TO_USER(ifr_data_ptr, api_cmd,sizeof(wan_cmd_api_t))){
		return -EFAULT;
	}
	return err;
}



int aft_tdmapi_mtu_check(sdla_t *card_ptr, unsigned int *mtu)
{
	void **card_list;
	u32 max_number_of_ports, i;
	sdla_t	*card;
	int used_cnt;
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
	int zt_chunksize;
#endif

	card_ptr->hw_iface.getcfg(card_ptr->hw, SDLA_HWCPU_USEDCNT, &used_cnt);
	
	if (used_cnt == 1) {
		card=card_ptr;

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		if (card->sdla_tdmv_dummy) {
			zt_chunksize = sdla_tdmv_dummy_get_zt_chunksize();
			*mtu = zt_chunksize;
			DEBUG_EVENT("%s: TDMV Dummy Enabled setting MTU to %d bytes!\n",
				card->devname,zt_chunksize);
			return 0;
		} 
#endif

		/* If SDLA TDMV dummy is not used, then allow the first
		   card mtu to be configured by the user */
		return -1;		
	}

	if (IS_BRI_CARD(card_ptr)) {
		max_number_of_ports = MAX_BRI_LINES; /* 24 */
	} else {
		max_number_of_ports = 8; 
	}
	
	card_list=__sdla_get_ptr_isr_array(card_ptr->hw);

	for (i=0; i<max_number_of_ports; i++) {

		card=(sdla_t*)card_list[i];
		if (card == NULL || wan_test_bit(CARD_DOWN,&card->wandev.critical)) {
			continue;
		}

		/* NC: Bug fix: if we have mixed mode where some cards are not in
		   global_tdm_irq mode we should ignore them */
		if (!card->u.aft.global_tdm_irq) {
			continue;
		}

		if (card_ptr == card) {
			continue;
		}
	
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		if (card->sdla_tdmv_dummy) {
			zt_chunksize = sdla_tdmv_dummy_get_zt_chunksize();
			*mtu = zt_chunksize;
			DEBUG_EVENT("%s: TDMV Dummy Enabled setting MTU to %d bytes!\n",
				card->devname,zt_chunksize);
			return 0;
		} 
#endif
		
		if (wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
			/* Make sure that all MTUs are the same */
			*mtu = card->u.aft.tdmv_mtu;	
			return 0;
		}
		
	}

	return -1;
}



#if defined(__LINUX__) || defined(__WINDOWS__)

int if_change_mtu(netdevice_t *dev, int new_mtu)
{
	private_area_t* chan= (private_area_t*)wan_netif_priv(dev);

	if (!chan || wan_test_bit(0,&chan->interface_down)) {
		return -ENODEV;
	}

	if (!chan->hdlc_eng) {
		return -EINVAL;
	}

	if (new_mtu < 16) {
		return -EINVAL;
	}

	if (chan->common.usedby == API){
		new_mtu+=sizeof(wp_api_hdr_t);
	}else if (chan->common.usedby == STACK){
		new_mtu+=32;
	}

	if (new_mtu > chan->dma_mru) {
		return -EINVAL;
	}

	dev->mtu = new_mtu;
	
	return 0;
}
#endif


static int digital_loop_test(sdla_t* card,wan_udp_pkt_t* wan_udp_pkt)
{
	netskb_t* skb;
	netdevice_t* dev;
	char* buf;
	private_area_t *chan;

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if (dev == NULL) {
		return 1;
	}
	chan = wan_netif_priv(dev);
	if (chan == NULL) {
		return 1;
	}
	
	if (chan->common.state != WAN_CONNECTED) {
		DEBUG_EVENT("%s: Loop test failed: dev not connected!\n",
		                        card->devname);
		return 2;
	}
	 
	skb = wan_skb_alloc(wan_udp_pkt->wan_udp_data_len+100);
	if (skb == NULL) {
		return 3;
	}

	switch (chan->common.usedby) {

	case API:
		wan_skb_push(skb, sizeof(wp_api_hdr_t));
		break;

	case STACK:
	case WANPIPE:
		break;

	case TDM_VOICE:
	case TDM_VOICE_API:
	case TDM_VOICE_DCHAN:
		if (card->u.aft.tdmv_dchan) {
			break;
		} else {
			DEBUG_EVENT("%s: Loop test failed: no dchan in TDMV mode!\n",
		                        card->devname);
		}
		/* Fall into the default case */

	default:
		DEBUG_EVENT("%s: Loop test failed: invalid operation mode!\n",
			card->devname);
		wan_skb_free(skb);
		return 4;
	}

	buf = wan_skb_put(skb, wan_udp_pkt->wan_udp_data_len);
	memcpy(buf, wan_udp_pkt->wan_udp_data, wan_udp_pkt->wan_udp_data_len);

#if defined(__LINUX__)
	skb->next = skb->prev = NULL;
        skb->dev = dev;
        skb->protocol = htons(ETH_P_IP);
	wan_skb_reset_mac_header(skb);
        dev_queue_xmit(skb);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	DEBUG_EVENT("%s: WARNING: Digital loop test mode is not supported!\n",
				card->devname);
#endif
	return 0;
}



/*=============================================================================
 * process_udp_mgmt_pkt
 *
 * Process all "wanpipemon" debugger commands.  This function
 * performs all debugging tasks:
 *
 * 	Line Tracing
 * 	Line/Hardware Statistics
 * 	Protocol Statistics
 *
 * "wanpipemon" utility is a user-space program that
 * is used to debug the WANPIPE product.
 *
 */

int process_udp_mgmt_pkt(sdla_t* card, netdevice_t* dev, private_area_t* chan, int local_dev )
{
	unsigned short buffer_length;
	wan_udp_pkt_t *wan_udp_pkt;
	wan_trace_t *trace_info=NULL;
	wan_if_cfg_t *if_cfg;
	wan_smp_flag_t smp_flags;

	smp_flags=0;

	if (wan_atomic_read(&chan->udp_pkt_len) == 0){
		return -ENODEV;
	}

	if (!chan->udp_pkt_data) {
		chan->udp_pkt_data=wan_malloc(sizeof(wan_udp_pkt_t)+128);
		if (!chan->udp_pkt_data) {
			return -ENOMEM;
		}
	}

	trace_info=&chan->trace_info;
	wan_udp_pkt = (wan_udp_pkt_t *)chan->udp_pkt_data;

   	{
		netskb_t *skb;

		wan_udp_pkt->wan_udp_opp_flag = 0;

		DEBUG_UDP("wan_udp_pkt->wan_udp_command: 0x%X\n", wan_udp_pkt->wan_udp_command);

		switch(wan_udp_pkt->wan_udp_command) {

		case WANPIPEMON_READ_CONFIGURATION:
 			if_cfg = (wan_if_cfg_t*)&wan_udp_pkt->wan_udp_data[0];
                        
			memset(if_cfg,0,sizeof(wan_if_cfg_t));
            if_cfg->usedby = chan->usedby_cfg;			
            if_cfg->media=card->wandev.fe_iface.get_fe_media(&card->fe);

			if_cfg->active_ch=chan->time_slot_map;
			if_cfg->cfg_active_ch=chan->cfg_active_ch;

			/* EC API always expects first channel to start from 1
		       however in driver, we use first channel 0. Since E1
			   is already shifted and 0 channel is not used, we dont
			   have to shif for E1 */
			if(IS_E1_CARD(card)) {
          	 	if_cfg->ec_active_ch=chan->time_slot_map;
			} else {
				if_cfg->ec_active_ch=chan->time_slot_map<<1;
			}

            if_cfg->chunk_sz=chan->mru/chan->num_of_time_slots;
            if_cfg->interface_number=chan->logic_ch_num;

            if (chan->hdlc_eng) {
                    if_cfg->hw_coding=WAN_TDMV_HDLC;
			} else {
                    if_cfg->hw_coding=(u8)card->fe.fe_cfg.tdmv_law;
                    if (IS_T1_CARD(card)){
                            if_cfg->hw_coding=WAN_TDMV_MULAW;
                    } else if (IS_E1_CARD(card)) {
                            if_cfg->hw_coding=WAN_TDMV_ALAW;
                    }
            }

            memcpy(&if_cfg->fe_cfg, &card->fe.fe_cfg, sizeof(sdla_fe_cfg_t));

			/* line_mode: MODE_OPTION_HDLC/MODE_OPTION_BITSTRM/ISDN_BRI_DCHAN */
			if (IS_BRI_CARD(card) && (chan->dchan_time_slot >= 0)){
				_snprintf(if_cfg->line_mode, USED_BY_FIELD, "%s", ISDN_BRI_DCHAN);
			} else if (chan->hdlc_eng) {
				_snprintf(if_cfg->line_mode, USED_BY_FIELD, "%s", MODE_OPTION_HDLC);
			} else {
				_snprintf(if_cfg->line_mode, USED_BY_FIELD, "%s", MODE_OPTION_BITSTRM);
			}

			if(IS_FXOFXS_MEDIA(&card->fe.fe_cfg)){
				//find the module number
				int mod_num = chan->first_time_slot;
				//check not outside of array boundary!!
				if(mod_num < MAX_REMORA_MODULES){
					if_cfg->sub_media = card->fe.rm_param.mod[mod_num].type;
				}else{
					DEBUG_EVENT("%s(): %s: Error: READ_CONFIGURATION: Invalid mod_num: %d\n", __FUNCTION__, card->devname, mod_num);
				}
			}else if(IS_BRI_CARD(card)){
				if(aft_is_bri_te_card(card)){
					if_cfg->sub_media = MOD_TYPE_TE;
				}else if(aft_is_bri_nt_card(card)){
					if_cfg->sub_media = MOD_TYPE_NT;
				}else{
					if_cfg->sub_media = MOD_TYPE_NONE;/* this should never happen */
				}
			} else if (IS_SERIAL_CARD(card)) {
				switch (card->adptr_type) 
				{
				case AFT_ADPTR_2SERIAL_V35X21:
				case AFT_ADPTR_4SERIAL_V35X21:
					if_cfg->sub_media = WANOPT_V35;
					break;
				case AFT_ADPTR_2SERIAL_RS232:
				case AFT_ADPTR_4SERIAL_RS232:
					if_cfg->sub_media = WANOPT_RS232;
					break;
				}
			}


            wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
            wan_udp_pkt->wan_udp_data_len=sizeof(wan_if_cfg_t);
            break;

		case WANPIPEMON_READ_CODE_VERSION:
#if defined(__WINDOWS__)
			wpabs_memcpy(wan_udp_pkt->wan_udp_data, &drv_version, sizeof(DRIVER_VERSION));
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			wan_udp_pkt->wan_udp_data_len = sizeof(DRIVER_VERSION);
#else
			wan_udp_pkt->wan_udp_return_code = 0;
			wan_udp_pkt->wan_udp_data[0]=card->u.aft.firm_ver;
			wan_udp_pkt->wan_udp_data_len=1;
#endif
			break;
	
		case WANPIPEMON_AFT_LINK_STATUS:
			wan_udp_pkt->wan_udp_return_code = 0;
			if (card->wandev.state == WAN_CONNECTED){
				wan_udp_pkt->wan_udp_data[0]=1;
			}else{
				wan_udp_pkt->wan_udp_data[0]=0;
			}
			wan_udp_pkt->wan_udp_data_len=1;
			break;

		case WANPIPEMON_AFT_MODEM_STATUS:
			wan_udp_pkt->wan_udp_return_code = 0;
			wan_udp_pkt->wan_udp_data[0]=0;

			if (IS_SERIAL_CARD(card)) {
				if (wan_test_bit(AFT_SERIAL_LCFG_DCD_BIT,&card->u.aft.serial_status)) {
					wan_udp_pkt->wan_udp_data[0]|=0x08;
				}
				if (wan_test_bit(AFT_SERIAL_LCFG_CTS_BIT,&card->u.aft.serial_status)) {
					wan_udp_pkt->wan_udp_data[0]|=0x20;
				}
			} else {
				if (card->wandev.state == WAN_CONNECTED){
					wan_udp_pkt->wan_udp_data[0]=0x28;
				}else{
					wan_udp_pkt->wan_udp_data[0]=0;
				}
			}

			wan_udp_pkt->wan_udp_data_len=1;
			break;

		case WANPIPEMON_DIGITAL_LOOPTEST:
			wan_udp_pkt->wan_udp_return_code = 
				(u8)digital_loop_test(card,wan_udp_pkt);
			break;
				
		case WANPIPEMON_READ_OPERATIONAL_STATS:
			memcpy(wan_udp_pkt->wan_udp_data,&chan->chan_stats,sizeof(wp_tdm_chan_stats_t));
			wan_udp_pkt->wan_udp_data_len=sizeof(wp_tdm_chan_stats_t);
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			break;

		case WANPIPEMON_FLUSH_OPERATIONAL_STATS:
			memset(&chan->chan_stats,0,sizeof(wp_tdm_chan_stats_t));
			memset(&chan->common.if_stats,0,sizeof(chan->common.if_stats));
			wan_udp_pkt->wan_udp_data_len=0;
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			break;

		case WANPIPEMON_READ_COMMS_ERROR_STATS:
			wan_udp_pkt->wan_udp_return_code = 0;
			memcpy(wan_udp_pkt->wan_udp_data,&chan->errstats,sizeof(aft_comm_err_stats_t));
			wan_udp_pkt->wan_udp_data_len=sizeof(aft_comm_err_stats_t);
			break;
	
		case WANPIPEMON_FLUSH_COMMS_ERROR_STATS:
			wan_udp_pkt->wan_udp_return_code = 0;
			memset(&chan->common.if_stats,0,sizeof(chan->common.if_stats));
			memset(&chan->errstats,0,sizeof(aft_comm_err_stats_t));
			wan_udp_pkt->wan_udp_data_len=0;
			break;

		case WANPIPEMON_CHAN_SEQ_DEBUGGING:

			card->wp_debug_chan_seq = wan_udp_pkt->wan_udp_data[0];
			DEBUG_EVENT("%s: Span %i channel sequence debugging %s !\n",
				card->devname, card->tdmv_conf.span_no,card->wp_debug_chan_seq ? "Enabled":"Disabled");
			wan_udp_pkt->wan_udp_return_code = 0;
			wan_udp_pkt->wan_udp_data_len=0;
			break;

		case WANPIPEMON_ENABLE_TRACING:
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			wan_udp_pkt->wan_udp_data_len = 0;
			
			if (!wan_test_bit(0,&trace_info->tracing_enabled)){
						
				trace_info->trace_timeout = SYSTEM_TICKS;
					
				wan_trace_purge(trace_info);
					
				if (wan_udp_pkt->wan_udp_data[0] == 0){
					wan_clear_bit(1,&trace_info->tracing_enabled);
					DEBUG_UDP("%s: ADSL L3 trace enabled!\n",
						card->devname);
				}else if (wan_udp_pkt->wan_udp_data[0] == 1){
					wan_clear_bit(2,&trace_info->tracing_enabled);
					wan_set_bit(1,&trace_info->tracing_enabled);
					DEBUG_UDP("%s: ADSL L2 trace enabled!\n",
							card->devname);
				}else{
					wan_clear_bit(1,&trace_info->tracing_enabled);
					wan_set_bit(2,&trace_info->tracing_enabled);
					DEBUG_UDP("%s: ADSL L1 trace enabled!\n",
							card->devname);
				}
				wan_set_bit (0,&trace_info->tracing_enabled);

			}else{
				DEBUG_EVENT("%s: Error: ATM trace running!\n",
						card->devname);
				wan_udp_pkt->wan_udp_return_code = 2;
			}
			break;

		case WANPIPEMON_DISABLE_TRACING:
			
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			
			if(wan_test_bit(0,&trace_info->tracing_enabled)) {
					
				wan_clear_bit(0,&trace_info->tracing_enabled);
				wan_clear_bit(1,&trace_info->tracing_enabled);
				wan_clear_bit(2,&trace_info->tracing_enabled);
				
				wan_trace_purge(trace_info);
				
				DEBUG_UDP("%s: Disabling AFT trace\n",
							card->devname);
					
			}else{
				/* set return code to line trace already 
				   disabled */
				wan_udp_pkt->wan_udp_return_code = 1;
			}
			break;

		case WANPIPEMON_GET_TRACE_INFO:

			if(wan_test_bit(0,&trace_info->tracing_enabled)){
				trace_info->trace_timeout = SYSTEM_TICKS;
			}else{
				DEBUG_EVENT("%s: Error ATM trace not enabled\n",
						card->devname);
				/* set return code */
				wan_udp_pkt->wan_udp_return_code = 1;
				break;
			}

			buffer_length = 0;
			wan_udp_pkt->wan_udp_aft_num_frames = 0;	
			wan_udp_pkt->wan_udp_aft_ismoredata = 0;
					
#if defined(__FreeBSD__) || defined(__OpenBSD__)
			while (wan_skb_queue_len(&trace_info->trace_queue)){
				WAN_IFQ_POLL(&trace_info->trace_queue, skb);
				if (skb == NULL){	
					DEBUG_EVENT("%s: No more trace packets in trace queue!\n",
								card->devname);
					break;
				}
				if ((WAN_MAX_DATA_SIZE - buffer_length) < skb->m_pkthdr.len){
					/* indicate there are more frames on board & exit */
					wan_udp_pkt->wan_udp_aft_num_frames = 0x01;
					break;
				}

				m_copydata(skb, 
					   0, 
					   skb->m_pkthdr.len, 
					   &wan_udp_pkt->wan_udp_data[buffer_length]);
				buffer_length += skb->m_pkthdr.len;
				WAN_IFQ_DEQUEUE(&trace_info->trace_queue, skb);
				if (skb){
					wan_skb_free(skb);
				}
				wan_udp_pkt->wan_udp_aft_num_frames++;
			}
#elif defined(__LINUX__)
			while ((skb=skb_dequeue(&trace_info->trace_queue)) != NULL){

				if((MAX_TRACE_BUFFER - buffer_length) < wan_skb_len(skb)){
					/* indicate there are more frames on board & exit */
					wan_udp_pkt->wan_udp_aft_ismoredata = 0x01;
					if (buffer_length != 0){
						wan_skb_queue_head(&trace_info->trace_queue, skb);
					}else{
						/* If rx buffer length is greater than the
						 * whole udp buffer copy only the trace
						 * header and drop the trace packet */

						memcpy(&wan_udp_pkt->wan_udp_data[buffer_length], 
							wan_skb_data(skb),
							sizeof(wan_trace_pkt_t));

						buffer_length = sizeof(wan_trace_pkt_t);
						wan_udp_pkt->wan_udp_aft_num_frames++;
						wan_skb_free(skb);	
					}
					break;
				}

				memcpy(&wan_udp_pkt->wan_udp_data[buffer_length], 
				       wan_skb_data(skb),
				       wan_skb_len(skb));
		     
				buffer_length += wan_skb_len(skb);
				wan_skb_free(skb);
				wan_udp_pkt->wan_udp_aft_num_frames++;
			}
#elif defined(__WINDOWS__)
#if 1
			/* for CMD line Wanpipemon */
			wp_os_lock_irq(&card->wandev.lock,&smp_flags);
			skb=skb_dequeue(&trace_info->trace_queue);
			wp_os_unlock_irq(&card->wandev.lock,&smp_flags);

			if(skb){
				memcpy(&wan_udp_pkt->wan_udp_data[buffer_length], 
				       wan_skb_data(skb),
				       wan_skb_len(skb));
		     
				buffer_length += (unsigned short)wan_skb_len(skb);
				wan_skb_free(skb);
				wan_udp_pkt->wan_udp_aft_num_frames++;

				/* NOT locking check of queue length intentionally! */
				if(wan_skb_queue_len(&trace_info->trace_queue) > 0){

					if(wan_skb_queue_len(&trace_info->trace_queue)){
						/* indicate there are more frames on board & exit */
						wan_udp_pkt->wan_udp_aft_ismoredata = 0x01;
					}else{
						wan_udp_pkt->wan_udp_aft_ismoredata = 0x00;
					}
				}
			}/* if(skb) */
#else
			/* for GUI Wanpipemon */
			buffer_length = 0;
			wp_os_lock_irq(&card->wandev.lock,&smp_flags);
			skb=skb_dequeue(&trace_info->trace_queue);
			wp_os_unlock_irq(&card->wandev.lock,&smp_flags);

			if(skb){
				wan_trace_info_t *aft_trace_info;
				unsigned char	 *user_data_buffer;

				user_data_buffer	= wan_udp_pkt->wan_udp_data;
				aft_trace_info		= &wan_udp_pkt->wan_udp_aft_trace_info;

				buffer_length = (u16)wan_skb_len(skb); /* this includes sizeof(wan_trace_pkt_t) */

				memcpy(user_data_buffer, wan_skb_data(skb), buffer_length);

				wan_skb_free(skb);

				/* NOT locking check of queue length intentionally! */
				if(wan_skb_queue_len(&trace_info->trace_queue) > 0){

					if(wan_skb_queue_len(&trace_info->trace_queue)){
						/* indicate there are more frames on board & exit */
						aft_trace_info->ismoredata = 0x01;
					}else{
						aft_trace_info->ismoredata = 0x00;
					}
				}
			}/* if(skb) */
#endif
#endif                      
			/* set the data length and return code */
			wan_udp_pkt->wan_udp_data_len = buffer_length;
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			break;

		case WANPIPEMON_ROUTER_UP_TIME:
			wan_getcurrenttime(&chan->router_up_time, NULL);
			chan->router_up_time -= chan->router_start_time;
			*(u32 *)&wan_udp_pkt->wan_udp_data = chan->router_up_time;	
			wan_udp_pkt->wan_udp_data_len = sizeof(u32);
	    	wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			break;

		case WAN_GET_PROTOCOL:
			wan_udp_pkt->wan_udp_data[0] = (u8)card->wandev.config_id;
	    	wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
	    	wan_udp_pkt->wan_udp_data_len = 1;
	    	break;

		case WAN_GET_PLATFORM:
		    wan_udp_pkt->wan_udp_data[0] = WAN_PLATFORM_ID;
		    wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
		    wan_udp_pkt->wan_udp_data_len = 1;
		    break;

		case WAN_GET_MASTER_DEV_NAME:
			wan_udp_pkt->wan_udp_data_len = 0;
			wan_udp_pkt->wan_udp_return_code = WAN_UDP_INVALID_NET_CMD;
			break;
			
		case WANPIPEMON_AFT_HWEC_STATUS:
			*(u32 *)&wan_udp_pkt->wan_udp_data[0] = card->wandev.fe_ec_map;
			wan_udp_pkt->wan_udp_data_len = sizeof(u32);
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			break;

		case WANPIPEMON_GET_OPEN_HANDLES_COUNTER:
#if defined(__WINDOWS__)			
			*(int*)&wan_udp_pkt->wan_udp_data[0] = dev->open_handle_counter;
#else
			*(int*)&wan_udp_pkt->wan_udp_data[0] = 1;
#endif
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			wan_udp_pkt->wan_udp_data_len = sizeof(int);
			break;

#if defined(__WINDOWS__)			
		case WANPIPEMON_EC_IOCTL:
			if (wan_test_and_set_bit(CARD_HW_EC,&card->wandev.critical)){
				DEBUG_EVENT("%s: Error: EC IOCTL Reentrant!\n", card->devname);
				return -EBUSY;
			}

			if (card->wandev.ec_dev){
				wan_udp_pkt->wan_udp_return_code = 
					(unsigned char)wanec_dev_ioctl((void*)wan_udp_pkt->wan_udp_data, card->devname);
			}else{
				DEBUG_HWEC("card->wandev.ec_dev is NULL!!!!!\n");
				wan_udp_pkt->wan_udp_return_code = WAN_UDP_INVALID_CMD;
			}

			wan_clear_bit(CARD_HW_EC,&card->wandev.critical);
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			break;

		case WANPIPEMON_SET_RBS_BITS:
			{
				rbs_management_t *rbs_management_ptr = (rbs_management_t *)&wan_udp_pkt->wan_udp_data[0];
				
				DEBUG_UDP("channel: %d, ABCD_bits: 0x%02X\n",
					rbs_management_ptr->channel,
					rbs_management_ptr->ABCD_bits);

				if(card->wandev.fe_iface.set_rbsbits){
					//card->fe.fe_debug |= WAN_FE_DEBUG_RBS;
					chan->card->hw_iface.hw_lock(chan->card->hw,&smp_flags);
					wan_udp_pkt->wan_udp_data[0] = 
						(unsigned char)card->wandev.fe_iface.set_rbsbits(
								&card->fe,
								rbs_management_ptr->channel,
								rbs_management_ptr->ABCD_bits);
					chan->card->hw_iface.hw_unlock(chan->card->hw,&smp_flags);
					wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
					wan_udp_pkt->wan_udp_data_len = 1;
				}else{
					wan_udp_pkt->wan_udp_return_code = EPFNOSUPPORT;
				}
			}
			break;

		case WANPIPEMON_GET_RBS_BITS:
			{
				rbs_management_t *rbs_management_ptr = (rbs_management_t *)&wan_udp_pkt->wan_udp_data[0];
			
				DEBUG_UDP("rbs_management_ptr->channel: %d\n", rbs_management_ptr->channel);
		
				if(card->wandev.fe_iface.read_rbsbits){
					chan->card->hw_iface.hw_lock(chan->card->hw,&smp_flags);
					rbs_management_ptr->ABCD_bits = 
						card->wandev.fe_iface.read_rbsbits(	
								&card->fe,
								rbs_management_ptr->channel,
								0);
					chan->card->hw_iface.hw_unlock(chan->card->hw,&smp_flags);

					wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
					wan_udp_pkt->wan_udp_data_len = sizeof(rbs_management_t);
				}else{
					wan_udp_pkt->wan_udp_return_code = EPFNOSUPPORT;
				}
			}
			break;
/*
		case WAN_TDMV_API_IOCTL:
			wan_udp_pkt->wan_udp_return_code = 
				(unsigned char)chan->wanpipe_cdev_ops.ioctl(&chan->wp_tdm_api_dev, WANPIPE_IOCTL_TDM_API,
								(struct ifreq*)wan_udp_pkt->wan_udp_data);
			break;
*/
		case SIOC_WAN_DEVEL_IOCTL:
			aft_devel_ioctl(card, (struct ifreq*)wan_udp_pkt->wan_udp_data);
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			break;

		case WAN_GET_HW_MAC_ADDR:
			{
				/*	There is no MAC address on AFT hardware, simply create a random MAC.
					Only S518 ADSL provides built-in MAC addrr (sdla_adsl.c). */
				int i;
				for(i = 0; i < ETHER_ADDR_LEN; i++){
					wan_udp_pkt->wan_udp_data[i] = (u8)SYSTEM_TICKS+i;
				}
			}
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			wan_udp_pkt->wan_udp_data_len = ETHER_ADDR_LEN;
			break;

#endif
		case WANPIPEMON_AFT_CUSTOMER_ID:
			{
			unsigned char cid=0;
			wan_smp_flag_t smp_flags1;
			card->hw_iface.hw_lock(card->hw,&smp_flags1);
			wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
#if defined(__WINDOWS__)
			/* FIXME: should it be done for Linux too? */
#define WIN_CUSTOMER_CPLD_ID_REG		0xA0
#define CUSTOMER_CPLD_ID_TE1_OFFSET		0x02
#define CUSTOMER_CPLD_ID_ANALOG_OFFSET	0x0A
			if (IS_TE1_CARD(card)) {
				cid = aft_read_cpld(card,WIN_CUSTOMER_CPLD_ID_REG + CUSTOMER_CPLD_ID_TE1_OFFSET);
			}else if(IS_FXOFXS_CARD(card)){
				cid = aft_read_cpld(card,WIN_CUSTOMER_CPLD_ID_REG + CUSTOMER_CPLD_ID_ANALOG_OFFSET);
			}else{
				cid = aft_read_cpld(card,WIN_CUSTOMER_CPLD_ID_REG);
			}
#else
			cid=aft_read_cpld(card,CUSTOMER_CPLD_ID_REG);
#endif
			wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
			card->hw_iface.hw_unlock(card->hw,&smp_flags1);
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			wan_udp_pkt->wan_udp_data[0]=cid;
			wan_udp_pkt->wan_udp_data_len=sizeof(cid);
			}
			break;

		case WANPIPEMON_FLUSH_TX_BUFFERS:
			DEBUG_UDP("WANPIPEMON_FLUSH_TX_BUFFERS\n");
			{
				netskb_t *skb=NULL;
				do {
					wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
					skb=wan_skb_dequeue(&chan->wp_tx_pending_list);
					wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);
					if (skb) {
						wan_skb_free(skb);
					}
				} while (skb);
			}
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			break;

		default:
			if ((wan_udp_pkt->wan_udp_command == WAN_GET_MEDIA_TYPE) ||
			    ((wan_udp_pkt->wan_udp_command & 0xF0) == WAN_FE_UDP_CMD_START)){
				/* FE udp calls */

				if (card->wandev.fe_iface.process_udp) {

					card->hw_iface.hw_lock(card->hw,&smp_flags);
					card->wandev.fe_iface.process_udp(
							&card->fe, 
							&wan_udp_pkt->wan_udp_cmd,
							&wan_udp_pkt->wan_udp_data[0]
							);
					card->hw_iface.hw_unlock(card->hw,&smp_flags);
								
					break;
				} 
				/* Drop down to error case */
			}
			wan_udp_pkt->wan_udp_data_len = 0;
			wan_udp_pkt->wan_udp_return_code = WAN_UDP_INVALID_NET_CMD;
	
			DEBUG_EVENT("%s: %s: Error: Unsupported Management command: %x\n",
						__FUNCTION__,card->devname,wan_udp_pkt->wan_udp_command);
			break;
		} /* end of switch */
   	} /* end of else */
#if !defined(__WINDOWS__)
     	/* Fill UDP TTL */
	wan_udp_pkt->wan_ip_ttl= card->wandev.ttl;
#endif
	wan_udp_pkt->wan_udp_request_reply = UDPMGMT_REPLY;

	return 1;

}


int wan_user_process_udp_mgmt_pkt(void* card_ptr, void* chan_ptr, void *udata)
{
	sdla_t *card = (sdla_t *)card_ptr;
	private_area_t *chan = (private_area_t*)chan_ptr;
	wan_udp_pkt_t *wan_udp_pkt;

	if (wan_atomic_read(&chan->udp_pkt_len) != 0){
		return -EBUSY;
	}

	wan_atomic_set(&chan->udp_pkt_len, MAX_LGTH_UDP_MGNT_PKT);

	if (!chan->udp_pkt_data) {
		chan->udp_pkt_data=wan_malloc(sizeof(wan_udp_pkt_t)+128);
		if (!chan->udp_pkt_data) {
			return -ENOMEM;
		}
	}

	wan_udp_pkt=(wan_udp_pkt_t*)chan->udp_pkt_data;

#if defined (__WINDOWS__)
	/* udata IS a pointer to wan_udp_hdr_t. copy data from user's buffer */
	wpabs_memcpy(&wan_udp_pkt->wan_udp_hdr, udata, sizeof(wan_udp_hdr_t));
#else
	if (WAN_COPY_FROM_USER(
			&wan_udp_pkt->wan_udp_hdr,
			udata,
			sizeof(wan_udp_hdr_t))){
		wan_atomic_set(&chan->udp_pkt_len,0);
		return -EFAULT;
	}
#endif

	process_udp_mgmt_pkt(card,chan->common.dev,chan,1);

	/* This area will still be critical to other
		* PIPEMON commands due to udp_pkt_len
		* thus we can release the irq */

	if (wan_atomic_read(&chan->udp_pkt_len) > sizeof(wan_udp_pkt_t)){
		DEBUG_EVENT( "%s: Error: Pipemon buf too bit on the way up! %d\n",
				card->devname,wan_atomic_read(&chan->udp_pkt_len));
		wan_atomic_set(&chan->udp_pkt_len,0);
		return -EINVAL;
	}

#if defined (__WINDOWS__)
	/* udata IS a pointer to wan_udp_hdr_t. copy data into user's buffer */
	wpabs_memcpy(udata, &wan_udp_pkt->wan_udp_hdr, sizeof(wan_udp_hdr_t));
#else
	if (WAN_COPY_TO_USER(
			udata,
			&wan_udp_pkt->wan_udp_hdr,
			sizeof(wan_udp_hdr_t))){
		wan_atomic_set(&chan->udp_pkt_len,0);
		return -EFAULT;
	}
#endif

	wan_atomic_set(&chan->udp_pkt_len,0);

	return 0;
}

/*============================================================================
 * Update communications error and general packet statistics.
 */
int update_comms_stats(sdla_t* card)
{
	/* 1. On the first timer interrupt, update T1/E1 alarms
         * and PMON counters (only for T1/E1 card) (TE1)
         */

	 /* TE1 Update T1/E1 alarms */
        if (IS_TE1_CARD(card) && card->hw) {
	       wan_smp_flag_t smp_flags;

	       card->hw_iface.hw_lock(card->hw,&smp_flags);
	
	       if (card->wandev.fe_iface.read_alarm) {
               		card->wandev.fe_iface.read_alarm(&card->fe, WAN_FE_ALARM_READ|WAN_FE_ALARM_UPDATE); 
	       }
               /* TE1 Update T1/E1 perfomance counters */
#if 0
#warning "PMON DISABLED DUE TO ERROR"
#else
	       if (card->wandev.fe_iface.read_pmon) {
		       	wan_smp_flag_t flags;   
			wan_spin_lock_irq(&card->wandev.lock,&flags);
	         	card->wandev.fe_iface.read_pmon(&card->fe, 0);
			wan_spin_unlock_irq(&card->wandev.lock,&flags);
	       }
#endif
		
	       card->hw_iface.hw_unlock(card->hw,&smp_flags);
        }

	return 0;
}



static int aft_hwec_config_ch (sdla_t *card, private_area_t *chan, wanif_conf_t *conf, int fe_chan, int ctrl)
{
	int err = 0;
	unsigned int tdmv_hwec_option=0;
	
	if (conf) {
        	tdmv_hwec_option=conf->hwec.enable;
	}

#if defined(CONFIG_WANPIPE_HWEC)  
	if (ctrl == 0) { 

		if (IS_BRI_CARD(card)) {
			DEBUG_HWEC("%s(): original fe_chan: %d\n", __FUNCTION__, fe_chan);
			/* translate channel to be 1 or 2, nothing else!! */
			fe_chan = (fe_chan % 2);
			if (fe_chan == 0) {
				fe_chan=2;
			}
			DEBUG_HWEC("%s():      new fe_chan: %d\n", __FUNCTION__, fe_chan);
		}
               	card->wandev.ec_enable(card, 0, fe_chan);
			
	} else if (tdmv_hwec_option) {

		if (IS_BRI_CARD(card)){
			DEBUG_HWEC("%s(): original fe_chan: %d\n", __FUNCTION__, fe_chan);

			/* translate channel to be 1 or 2, nothing else!! */
			fe_chan = (fe_chan % 2);
			if (fe_chan == 0) {
				fe_chan=2;
			}
			DEBUG_HWEC("%s():      new fe_chan: %d\n", __FUNCTION__, fe_chan);
		}

		DEBUG_HWEC("[HWEC] %s: Enable Echo Canceller on fe_chan %d\n",
				chan->if_name,
				fe_chan);

		err = card->wandev.ec_enable(card, 1, fe_chan);
		if (err) {
			DEBUG_EVENT("%s: Failed to enable HWEC on fe chan %d\n",
					chan->if_name,fe_chan);
		 	return err;       		
		}
	} 
#endif	

	return err;
}               


int aft_hwec_config (sdla_t *card, private_area_t *chan, wanif_conf_t *conf, int ctrl)
{
	int err = 0;
	int fe_chan = 0;
	int i;
	
	/* If not hardware echo nothing to configure */
	if (!card->wandev.ec_enable) {
		return 0;
	}

	if (chan->common.usedby == TDM_VOICE_DCHAN) {
		return 0;
	}

	if (chan->wp_api_op_mode == WP_TDM_OPMODE_SPAN) {

		u32 map=chan->time_slot_map;
		for (i=0;i<31;i++) {
			if (wan_test_bit(i,&map)) {
				if (IS_TE1_CARD(card)) {
					if (IS_T1_CARD(card)){
							fe_chan = i+1;
					}else{
							fe_chan = i;
					}
				} else if (IS_FXOFXS_CARD(card)) {
					fe_chan = i+1;
				} else {
						fe_chan = i+1;
				}
				err=aft_hwec_config_ch(card,chan,conf,fe_chan,ctrl);
				if (err) {
					return err;
				}
			}
		}

	} else if (chan->common.usedby == TDM_VOICE_API ||
	           chan->common.usedby == TDM_VOICE){

		/* Nov 6, 2007 Calling EC function with FE channel number. */
		if (IS_TE1_CARD(card)) {
			if (IS_T1_CARD(card)){
				fe_chan = chan->first_time_slot+1;
			}else{
				fe_chan = chan->first_time_slot;
			}
		} else if (IS_FXOFXS_CARD(card)) {
			fe_chan = chan->first_time_slot+1;
		} else {
		    fe_chan = chan->first_time_slot+1; 
		}              

		err=aft_hwec_config_ch(card,chan,conf,fe_chan,ctrl);	
		if (err) {
			return err;
		}
	}

	return err;
}




#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)
static int32_t aft_bri_clock_control(sdla_t *card, u8 line_state)
{
	u_int32_t		reg;

	/* Due to a hardware bug, only first 0x100 register 
         * controls the clock routing */
	card->hw_iface.bus_read_4(card->hw,AFT_LINE_CFG_REG,&reg);
	
	if (line_state == 1){
		/* The 'Reference' line is connected, enable sync from line interface */
		
		if (wan_test_bit(A500_LINE_SYNC_MASTER_BIT, &reg)) {
			return 0;
		}

		wan_set_bit(A500_LINE_SYNC_MASTER_BIT, &reg);
	}

	if (line_state == 0){
		/* The 'Reference' line is DISconnected, disable sync from line interface */
		
		if (!wan_test_bit(A500_LINE_SYNC_MASTER_BIT, &reg)) {
			return 0;
		}

		
		wan_clear_bit(A500_LINE_SYNC_MASTER_BIT, &reg);
	}

	card->hw_iface.bus_write_4(card->hw,AFT_LINE_CFG_REG,reg);

	return 0;
}
#endif


int aft_handle_clock_master (sdla_t *card_ptr)
{

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)

	void **card_list;
	u32 max_number_of_ports, i;
	sdla_t	*card;
	int master_clock_src_found=0;


	/* This feature only works for BRI cards for now */
	if (!IS_BRI_CARD(card_ptr)) {
		return 0;	
	}

	if (card_ptr->wandev.fe_iface.clock_ctrl == NULL){
		return 0;
	}

	max_number_of_ports = MAX_BRI_LINES; /* 24 */
	
	DEBUG_TEST("%s:aft_handle_clock_master !\n",
							card_ptr->devname);

	card_list=__sdla_get_ptr_isr_array(card_ptr->hw);

	for (i=0; i<max_number_of_ports; i++) {
		card=(sdla_t*)card_list[i];
		if (card == NULL || wan_test_bit(CARD_DOWN,&card->wandev.critical)) {
			continue;
		}
		
#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)
		DEBUG_TEST("%s: Device Type TE=%i NT=%i!\n",card->devname,
				aft_is_bri_te_card(card),aft_is_bri_nt_card(card));


		if (IS_BRI_CARD(card)) {
			if (aft_is_bri_te_card(card) && wan_test_bit(CARD_MASTER_CLOCK,&card->wandev.critical)) {		
				if (card->wandev.state != WAN_CONNECTED) {
					if (card->wandev.fe_iface.clock_ctrl){
						DEBUG_EVENT("%s: Clearing Master Clock!\n",
							card->devname);
						card->wandev.fe_iface.clock_ctrl(&card->fe, 0);
						aft_bri_clock_control(card,0);
					
					} else {
						DEBUG_EVENT("%s: Internal Error: ctrl_clock feature not available!\n",
							card->devname);
						aft_bri_clock_control(card,0);
					}
					
					wan_clear_bit(CARD_MASTER_CLOCK,&card->wandev.critical);

				} else {
					DEBUG_TEST("%s: Master Clock Found!\n",
							card->devname);
					/* Master Clock port is still OK */
					master_clock_src_found=1;
				}
			}
		}
#endif
	}

	if (master_clock_src_found) {
		return 0;
	}

	/* At this point the recovery clock must be set on any connected port */
	for (i=0; i<max_number_of_ports; i++) {

		card=(sdla_t*)card_list[i];
		if (card == NULL || wan_test_bit(CARD_DOWN,&card->wandev.critical)) {
			continue;
		}

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)
		if (IS_BRI_CARD(card)) {

			/* Check if this module is forced configured in oscillator mode */
			if (BRI_CARD_CLK(card) == WAN_MASTER_CLK) {
				continue;
			}

			/* Only enable clock recovery on cards with new CPLD */
			if (!aft_is_bri_512khz_card(card)) {
				continue;	
			}

			if (aft_is_bri_te_card(card) && !wan_test_bit(CARD_MASTER_CLOCK,&card->wandev.critical)) {	
				if (card->wandev.state == WAN_CONNECTED) {
					if (card->wandev.fe_iface.clock_ctrl){
						DEBUG_EVENT("%s: Setting Master Clock!\n",
							card->devname);
						card->wandev.fe_iface.clock_ctrl(&card->fe, 1);
						aft_bri_clock_control(card,1);
					
						wan_set_bit(CARD_MASTER_CLOCK,&card->wandev.critical);
						master_clock_src_found=1;
					}
					break;
				} 
			}
		}
#endif
	}
#endif

	return 0;
}



void wanpipe_wake_stack(private_area_t* chan)
{
	WAN_NETIF_WAKE_QUEUE(chan->common.dev);
	wan_chan_dev_start(chan);

	if (chan->common.usedby == API){

		if (chan->wp_api_op_mode){
#ifdef AFT_TDM_API_SUPPORT
			if (is_tdm_api(chan,&chan->wp_tdm_api_dev)){
				wanpipe_tdm_api_kick(&chan->wp_tdm_api_dev);
			}
#endif
		} else {
			wan_wakeup_api(chan);
		}

	} else if (chan->common.usedby == STACK){
#if defined(__WINDOWS__)
		FUNC_NOT_IMPL();
#else
		wanpipe_lip_kick(chan,0);
#endif
	} else if (chan->wp_api_op_mode || chan->common.usedby == TDM_VOICE_DCHAN){
#ifdef AFT_TDM_API_SUPPORT
		if (is_tdm_api(chan,&chan->wp_tdm_api_dev)){
			wanpipe_tdm_api_kick(&chan->wp_tdm_api_dev);
		}
#endif
	} else if (chan->common.usedby == ANNEXG) {
#ifdef CONFIG_PRODUCT_WANPIPE_ANNEXG
		if (chan->annexg_dev){
			if (IS_FUNC_CALL(lapb_protocol,lapb_mark_bh)){
				lapb_protocol.lapb_mark_bh(chan->annexg_dev);
			}
		}
#endif
	}

}


int aft_find_master_if_and_dchan(sdla_t *card, int *master_if, u32 active_ch)
{
     	int dchan_found=0;
	int i;
	
        if (card->tdmv_conf.dchan) {
		if (IS_E1_CARD(card) && !(WAN_FE_FRAME(&card->fe) == WAN_FR_UNFRAMED)) {
			card->tdmv_conf.dchan = card->tdmv_conf.dchan << 1;
			wan_clear_bit(0,&card->tdmv_conf.dchan);
		}     
	}

	for (i=card->u.aft.num_of_time_slots-1;i>=0;i--){
		if (wan_test_bit(i,&active_ch)){

			if (wan_test_bit(i,&card->tdmv_conf.dchan)){
			       	dchan_found=1;
				card->u.aft.tdmv_dchan=i;
			       	continue;
			}
		
			/* Find the TOP timeslot.  This timeslot will be
                         * considered MASTER since it is the last timeslot
                         * to rx data from the T1 line */	
			if (*master_if < 0){	
				*master_if=i;
			}
		}
	}

	if (card->tdmv_conf.dchan && !dchan_found){
		/* We configured for dchan however, this interface
                 * was not configued with the DCHAN timeslot.
                 * It IS now possible that another interface has
                 * this time slot */
	       	for (i=card->u.aft.num_of_time_slots-1;i>=0;i--){
                       	if (wan_test_bit(i,&card->tdmv_conf.dchan)){
	       		       	dchan_found=1;
	       			card->u.aft.tdmv_dchan=i;
	       			wan_set_bit(i,&card->u.aft.tdmv_dchan_cfg_on_master);
	       			continue;
	       		}	
	       	}

	       	if (!dchan_found) {
	       		DEBUG_EVENT("%s: Error: TDM DCHAN is out of range 0x%08X\n",
	       			card->devname,card->tdmv_conf.dchan);
	       		return -EINVAL;       
	       	}
			
	       	/* We have found a DCHAN outside the
	       	   Voice active channels */
  	}                             

	return 0;
}


int aft_fifo_intr_ctrl(sdla_t *card, int ctrl)
{
	u32 reg;
	card->hw_iface.bus_read_4(card->hw,
	      AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG), 
	      &reg);

	card->hw_iface.bus_read_4(card->hw,
	      AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG), 
	      &reg);

	card->hw_iface.bus_read_4(card->hw,
			AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);

	if (ctrl){
		wan_set_bit(AFT_LCFG_FIFO_INTR_BIT,&reg);
	}else{
		wan_clear_bit(AFT_LCFG_FIFO_INTR_BIT,&reg);
	}

	card->hw_iface.bus_write_4(card->hw,
			AFT_PORT_REG(card,AFT_LINE_CFG_REG), reg);

	card->u.aft.lcfg_reg=reg;

	
	if (!ctrl){
		card->hw_iface.bus_read_4(card->hw,
	 	     AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG), 
	 	     &reg);

		card->hw_iface.bus_read_4(card->hw,
		      AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG), 
		      &reg);
	}
	
	return 0;
}

int aft_tdm_intr_ctrl(sdla_t *card, int ctrl)
{
	u32 reg;
	card->hw_iface.bus_read_4(card->hw,
			AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);

	if (ctrl){
                wan_set_bit(AFT_LCFG_TDMV_INTR_BIT,&reg);

	}else{
		wan_clear_bit(AFT_LCFG_TDMV_INTR_BIT,&reg);
	}

	card->hw_iface.bus_write_4(card->hw,
			AFT_PORT_REG(card,AFT_LINE_CFG_REG), reg);

	card->u.aft.lcfg_reg=reg;

	return 0;
}



int aft_tdm_chan_ring_rsyinc(sdla_t * card, private_area_t *chan)
{
	u32 lo_reg,lo_reg_tx;
	u32 dma_descr;

	dma_descr=(chan->logic_ch_num<<4) +
				AFT_PORT_REG(card,AFT_RX_DMA_LO_DESCR_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_descr,&lo_reg);

	dma_descr=(chan->logic_ch_num<<4) +
				AFT_PORT_REG(card,AFT_TX_DMA_LO_DESCR_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_descr,&lo_reg_tx);

	lo_reg=(lo_reg&AFT_TDMV_BUF_MASK)/AFT_TDMV_CIRC_BUF;
	lo_reg_tx=(lo_reg_tx&AFT_TDMV_BUF_MASK)/AFT_TDMV_CIRC_BUF;

	DEBUG_TEST("%s: Chan[%i] TDM Rsync RxHw=%i TxHw=%i Rx=%i Tx=%i\n",
					   card->devname,chan->logic_ch_num,
					   lo_reg,lo_reg_tx,
					   card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num],
					   card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]);

	switch (lo_reg) {
	case 0:
		card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]=2;
		break;
	case 1:
		card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]=3;
		break;
	case 2:
		card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]=0;
		break;
	case 3:
		card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]=1;
		break;
	}

	switch (lo_reg_tx) {
	case 0:
		card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]=2;
		break;
	case 1:
		card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]=3;
		break;
	case 2:
		card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]=0;
		break;
	case 3:
		card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]=1;
		break;
	}

	DEBUG_TEST("%s: Card TDM Rsync RxHw=%i TxHw=%i Rx=%i Tx=%i\n",
					   card->devname,
					   lo_reg,lo_reg_tx,
					   card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num],
					   card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]);

	return 0;
}

int aft_tdm_ring_rsync(sdla_t *card)
{
	int i; 
	private_area_t *chan;
	u32 lo_reg,lo_reg_tx,prx=0,ptx=0;
 	u32 dma_descr;

	for (i=0; i<card->u.aft.num_of_time_slots;i++){

		if (!wan_test_bit(i,&card->u.aft.tdm_logic_ch_map)){
			continue;
		}

		chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
		if (!chan){
			DEBUG_EVENT("%s: Error: No Dev for Rx logical ch=%d\n",
					card->devname,i);
			continue;
		}

		dma_descr=(chan->logic_ch_num<<4) +
					AFT_PORT_REG(card,AFT_RX_DMA_LO_DESCR_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_descr,&lo_reg);

		dma_descr=(chan->logic_ch_num<<4) +
					AFT_PORT_REG(card,AFT_TX_DMA_LO_DESCR_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_descr,&lo_reg_tx);

		lo_reg=(lo_reg&AFT_TDMV_BUF_MASK)/AFT_TDMV_CIRC_BUF;
		lo_reg_tx=(lo_reg_tx&AFT_TDMV_BUF_MASK)/AFT_TDMV_CIRC_BUF;

		if (i==0) {
			prx=lo_reg;
			ptx=lo_reg_tx;
		} else {
#if 0
			if (lo_reg != prx) {
				DEBUG_EVENT("%s: %s: channel=%i rx mismatch cur=%i prev=%i\n",
							card->devname,chan->if_name, chan->logic_ch_num, lo_reg, prx);
			}
			if (lo_reg_tx != ptx) {
				DEBUG_EVENT("%s: %s: channel=%i rx mismatch cur=%i prev=%i\n",
							card->devname,chan->if_name, chan->logic_ch_num,lo_reg_tx, ptx);
			}
#endif
			prx=lo_reg;
			ptx=lo_reg_tx;
		}

		switch (lo_reg) {
		case 0:
			card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]=2;
			break;
		case 1:
			card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]=3;
			break;
		case 2:
			card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]=0;
			break;
		case 3:
			card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]=1;
			break;
		}

		switch (lo_reg_tx) {
		case 0:
			card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]=2;
			break;
		case 1:
			card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]=3;
			break;
		case 2:
			card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]=0;
			break;
		case 3:
			card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]=1;
			break;
		}

		if (chan->channelized_cfg && !chan->hdlc_eng && chan->cfg.tdmv_master_if){

#if 0
			/* NC Bug fix: This code caused random data corruption */
			if (card->wandev.ec_enable){

				/* HW EC standard */
				if (lo_reg > 0) {
					card->u.aft.tdm_rx_dma_toggle = lo_reg-1;
				} else {
					card->u.aft.tdm_rx_dma_toggle = 3;
				}	
			} else
#endif

#if 0
			{
				/* Software ec moves spike to 8bytes */ 
				card->u.aft.tdm_rx_dma_toggle=lo_reg+1;
				if (card->u.aft.tdm_rx_dma_toggle > 3) {
					card->u.aft.tdm_rx_dma_toggle=0;
				}
				card->u.aft.tdm_rx_dma_toggle=lo_reg+1;
				if (card->u.aft.tdm_rx_dma_toggle > 3) {
					card->u.aft.tdm_rx_dma_toggle=0;
				}
			}
#endif


	
#if 0
			if (lo_reg_tx < 3) {
				card->u.aft.tdm_tx_dma_toggle = lo_reg_tx+1;
			} else {
				card->u.aft.tdm_tx_dma_toggle = 0;
			}
#endif
			card->rsync_timeout=0;
			DEBUG_EVENT("%s: Card TDM Rsync RxHw=%i TxHw=%i Rx=%i Tx=%i\n",
					   card->devname,
					   lo_reg,lo_reg_tx,
					   card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num],
					   card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]);
		}
	}                   

	return 0;
}



/*=============================================
 * aft_set_ss7_force_rx
 *
 * Force the firmware to pass up a single
 * ss7 packet.  Otherwise, the ss7 engine
 * will wait for the next different packet.
 */
void aft_set_ss7_force_rx(sdla_t *card, private_area_t *chan)
{
	u32 reg, dma_ram_desc;

	if (wan_test_and_set_bit(0,&chan->ss7_force_rx)){
		DEBUG_TEST("%s: FORCE BUSY RX\n",card->devname);
		return;
	}

	dma_ram_desc=chan->logic_ch_num*4+AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	if (wan_test_bit(AFT_DMACHAIN_SS7_FORCE_RX,&reg)){
		wan_clear_bit(AFT_DMACHAIN_SS7_FORCE_RX,&reg);
	}else{
		wan_set_bit(AFT_DMACHAIN_SS7_FORCE_RX,&reg);
	}
	card->hw_iface.bus_write_4(card->hw,dma_ram_desc,reg);
	
	DEBUG_TEST("%s: FORCE RX\n",card->devname);
}

/*=============================================
 * aft_clear_ss7_force_rx
 * 
 * Reset the ss7 force rx logic.
 * This must be done before trying to enable
 * force rx again. 
 * Note: That first_time_slot must be cleared LAST.
 *       Thus the reverse clear order.
 */
void aft_clear_ss7_force_rx(sdla_t *card, private_area_t *chan)
{
	wan_clear_bit(0,&chan->ss7_force_rx);
}


/* Used for Debugging Only */

#if 0
void aft_list_tx_descriptors(private_area_t *chan)
{
	u32 reg,cur_dma_ptr,lo_reg;
	sdla_t *card=chan->card;
	wan_dma_descr_t *dma_chain;
	u32 dma_descr;
	u32 i;
	u32 dma_ram_desc;
	unsigned int dma_cnt=MAX_AFT_DMA_CHAINS;

	dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	cur_dma_ptr=aft_dmachain_get_tx_dma_addr(reg);


	DEBUG_TEST("%s: TX Chain DMA List: Cur(End)=%d, Pend(Begin)=%d HWCur=%d\n",
			chan->if_name, 
			chan->tx_chain_indx,
			chan->tx_pending_chain_indx,
			cur_dma_ptr);

	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE) {
		dma_cnt=1;
	}
	
	for (i=0;i<dma_cnt;i++){

		dma_chain = &chan->tx_dma_chain_table[i];
		if (!dma_chain){
			DEBUG_EVENT("%s:%d Assertion Error !!!!\n",
				__FUNCTION__,__LINE__);
			break;
		}

		dma_descr=(chan->logic_ch_num<<4) + 
			  (dma_chain->index*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);
		
		card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);

		dma_descr=(chan->logic_ch_num<<4) + 
			  (dma_chain->index*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_TX_DMA_LO_DESCR_BASE_REG);
		
		card->hw_iface.bus_read_4(card->hw,dma_descr,&lo_reg);

		DEBUG_EVENT("%s: TX DMA=%d : Go=%u Intr=%u Used=%lu A=0%X R=0x%08X L=0x%08X C=%i\n",
				chan->if_name,
				dma_chain->index,
				wan_test_bit(AFT_TXDMA_HI_GO_BIT,&reg),
				wan_test_bit(AFT_TXDMA_HI_INTR_DISABLE_BIT,&reg) ? 0:1,
				dma_chain->init,
				dma_descr,
				reg,lo_reg,(lo_reg&0x1FF)/128);

	}
}

#endif

#if 0
static void aft_list_dma_chain_regs(sdla_t *card)
{
	u32 reg;
	int i;
	u32 dma_ram_desc;

	for (i=0; i<card->u.aft.num_of_time_slots;i++){

		private_area_t *chan;

		if (!wan_test_bit(i,&card->u.aft.logic_ch_map)){
			continue;
		}

		chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
		if (!chan){
			continue;
		}

		if (!wan_test_bit(WP_DEV_CONFIG,&chan->up)){
			continue;
		}
		
		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);

		DEBUG_EVENT("%s: DMA CHAIN: %i: 0x%08X\n",
				card->devname,i,reg);

	}
}
#endif


#if 0
static void aft_list_descriptors(private_area_t *chan)
{
	u32 reg,cur_dma_ptr,lo_reg;
	sdla_t *card=chan->card;
	wan_dma_descr_t *dma_chain;
	u32 dma_descr;
	int i;
	u32 dma_ram_desc;
	unsigned int dma_cnt=MAX_AFT_DMA_CHAINS;

	dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	cur_dma_ptr=aft_dmachain_get_rx_dma_addr(reg);
	
	DEBUG_TEST("%s: RX Chain DMA List: End=%d Begin=%d Cur=%d \n",
			chan->if_name, 
			chan->rx_chain_indx,
			chan->rx_pending_chain_indx,
			cur_dma_ptr);

	if ((chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE)){
		dma_cnt=1;
	}
	
	for (i=0;i<dma_cnt;i++){

		dma_chain = &chan->rx_dma_chain_table[i];
		if (!dma_chain){
			DEBUG_EVENT("%s:%d Assertion Error !!!!\n",
				__FUNCTION__,__LINE__);
			break;
		}

		dma_descr=(chan->logic_ch_num<<4) +(dma_chain->index*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

		card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);

		dma_descr=(chan->logic_ch_num<<4) +(dma_chain->index*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_RX_DMA_LO_DESCR_BASE_REG);

		card->hw_iface.bus_read_4(card->hw,dma_descr,&lo_reg);     

		DEBUG_EVENT("%s: RX DMA=%d : Go=%u Intr=%u Used=%lu A=0%X R=0x%X L=0x%08X C=%i\n",
				chan->if_name,
				dma_chain->index,
				wan_test_bit(AFT_RXDMA_HI_GO_BIT,&reg),
				!wan_test_bit(AFT_RXDMA_HI_INTR_DISABLE_BIT,&reg),
				dma_chain->init,dma_descr,reg,lo_reg,(lo_reg&0x1FF)/128);
	}
}
#endif


#ifdef AFT_DMA_HISTORY_DEBUG
static void aft_display_chain_history(private_area_t *chan)
{
	int start=chan->dma_index;
	int i;
	dma_history_t *dma_hist = &chan->dma_history[start];

	for (i=0;i<MAX_DMA_HIST_SIZE;i++){

		if (dma_hist->loc == 0){
			continue;
		}

		DEBUG_EVENT("%s: Loc=%02i End=%02d  Cur=%02d  Beg=%02d  St=%s\n",
				chan->if_name,
				dma_hist->loc,
				dma_hist->end,
				dma_hist->cur,
				dma_hist->begin,
				dma_hist->status?"Error":"Ok");

		start++;
		if (start >= MAX_DMA_HIST_SIZE){
			start=0;
		}
		dma_hist = &chan->dma_history[start];
	}
}
#endif


#ifdef AFT_DMA_HISTORY_DEBUG
static void aft_chain_history(private_area_t *chan,u8 end, u8 cur, u8 begin, u8 loc)
{
	dma_history_t *dma_hist = &chan->dma_history[chan->dma_index];

	dma_hist->loc=loc;
	dma_hist->end=end;
	dma_hist->cur=cur;
	dma_hist->begin=begin;
	dma_hist->status=0;
	
	if (end > begin){
		if (cur > end ||
		    cur < begin){
			DEBUG_TEST("%s: Rx: Critical: RxPendChain=%d HWDMA=%d  RxEndChain=%d\n",
					chan->if_name,begin,cur,end);
			dma_hist->status=1;
		}
	}else if (end < begin){
		if (cur < begin &&
		    cur > end){
			DEBUG_TEST("%s: Rx: Critical: RxEndChain=%d HWDMA=%d  RxPendingChain=%d\n",
					chan->if_name,begin,cur,end);
			dma_hist->status=1;
			
		}
	}

	if (++chan->dma_index >= MAX_DMA_HIST_SIZE){
		chan->dma_index=0;
	}
}
#endif




#if 0
static void wp_tdmv_api_chan_rx_tx(sdla_t *card, 
			           private_area_t *chan,
			           unsigned char *rxdata, unsigned char *txdata)
{
#if defined(__LINUX__)
	unsigned char *buf;
	
	if (!card->u.aft.tdmv_api_rx){
		card->u.aft.tdmv_api_rx=wan_skb_alloc(card->u.aft.tdmv_mtu);
		if (!card->u.aft.tdmv_api_rx){
			WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//++chan->if_stats.rx_errors;
			goto wp_tdmv_api_rx_tx_chan_skip_rx;
		}
	}

	if (wan_skb_len(card->u.aft.tdmv_api_rx) > (card->u.aft.tdmv_mtu-chan->mru)){
		/* CRITICAL ERROR: We cannot fit the all timeslots into this
		 * packet */
		WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//++chan->if_stats.rx_errors;
		goto wp_tdmv_api_rx_tx_chan_skip_rx;
	}

	buf=wan_skb_put(card->u.aft.tdmv_api_rx, chan->mru);
	memcpy(buf,rxdata,chan->mru);
	WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common);	//++chan->if_stats.rx_dropped;

wp_tdmv_api_rx_tx_chan_skip_rx:

	if (!card->u.aft.tdmv_api_tx){
		card->u.aft.tdmv_api_tx=wan_skb_dequeue(&card->u.aft.tdmv_api_tx_list);
		if (!card->u.aft.tdmv_api_tx){
			/* No tx packet, send idle frames */
			WAN_NETIF_STATS_INC_TX_CARRIER_ERRORS(&chan->common);	//++chan->if_stats.tx_carrier_errors;
			memset(txdata,chan->idle_flag,chan->mtu);
			return;
		}
	}

	if (wan_skb_len(card->u.aft.tdmv_api_tx) < chan->mtu){
		/* CRITICAL ERROR: 
		 * The tx api packet must have info for all
		 * timeslots */
		memset(txdata,chan->idle_flag,chan->mtu);
		WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//++chan->if_stats.tx_errors;
		return;
	}
	
	buf=wan_skb_data(card->u.aft.tdmv_api_tx);
	memcpy(txdata,buf,chan->mtu);
	wan_skb_pull(card->u.aft.tdmv_api_tx,chan->mtu);
	WAN_NETIF_STATS_INC_TX_DROPPED(&chan->common);	//++chan->if_stats.tx_dropped;

#endif
	return;
}

/*================================================
 * wp_tdmv_api_rx_tx
 *
 *
 */
	
static void wp_tdmv_api_rx_tx (sdla_t *card, private_area_t *chan)
{
#if defined(__LINUX__)
	chan=(private_area_t*)wan_netif_priv(chan->common.dev);
	
	if (!card->u.aft.tdmv_api_rx){
		/* CRITICAL ERROR: 
		 * There should be an rx api packet here */
		goto wp_tdmv_api_rx_tx_skip_rx;
	}
	
	if (wan_skb_len(card->u.aft.tdmv_api_rx) != card->u.aft.tdmv_mtu){
		wan_skb_free(card->u.aft.tdmv_api_rx);
		card->u.aft.tdmv_api_rx=NULL;
		goto wp_tdmv_api_rx_tx_skip_rx;
	}

	if (wan_skb_headroom(card->u.aft.tdmv_api_rx) >= sizeof(wp_api_hdr_t)){
		wp_api_hdr_t *rx_hdr=
			(wp_api_hdr_t*)skb_push(card->u.aft.tdmv_api_rx,
						sizeof(wp_api_hdr_t));
			memset(rx_hdr,0,sizeof(wp_api_hdr_t));
	}else{
		wan_skb_free(card->u.aft.tdmv_api_rx);
		card->u.aft.tdmv_api_rx=NULL;
		goto wp_tdmv_api_rx_tx_skip_rx;
	}
				
	card->u.aft.tdmv_api_rx->protocol = htons(PVC_PROT);
	card->u.aft.tdmv_api_rx->mac.raw  = card->u.aft.tdmv_api_rx->data;
	card->u.aft.tdmv_api_rx->dev      = chan->common.dev;
	card->u.aft.tdmv_api_rx->pkt_type = WAN_PACKET_DATA;	
	
	if (wan_api_rx(chan,card->u.aft.tdmv_api_rx) != 0){
		wan_skb_free(card->u.aft.tdmv_api_rx);
	}

	card->u.aft.tdmv_api_rx=NULL;

wp_tdmv_api_rx_tx_skip_rx:
	
	if (card->u.aft.tdmv_api_tx){
		
		if (wan_skb_len(card->u.aft.tdmv_api_tx) != 0){
			/*CRITICAL ERROR: 
			 * Length must be zero, because we pulled
			 * all timeslots out*/
		}
		
		wan_skb_free(card->u.aft.tdmv_api_tx);
		card->u.aft.tdmv_api_tx=NULL;

		if (WAN_NETIF_QUEUE_STOPPED(chan->common.dev)){
			WAN_NETIF_WAKE_QUEUE(chan->common.dev);
			wan_wakeup_api(chan);
		}
	}
#endif
	return;
}
#endif


int aft_core_send_serial_oob_msg (sdla_t *card)
{
#if defined(__LINUX__)
	wp_api_hdr_t *api_rx_el;
	int err=0, len=5;
	netskb_t *skb;
	wp_api_event_t *api_el;
	private_area_t *chan;

	chan=(private_area_t*)card->u.aft.dev_to_ch_map[0];
	if (!chan) {
		return -ENODEV;
	}

	if (chan->common.usedby != API){
		return -ENODEV;
	}
		
	if (!chan->common.sk){
		return -ENODEV;
	}

	skb=wan_skb_alloc(sizeof(wp_api_hdr_t)+sizeof(wp_api_event_t)+len);
	if (!skb) {
		return -ENOMEM;
	}

	api_rx_el=(wp_api_hdr_t *)wan_skb_put(skb,sizeof(wp_api_hdr_t));
	memset(api_rx_el,0,sizeof(wp_api_hdr_t));

	api_el=(wp_api_event_t *)wan_skb_put(skb,sizeof(wp_api_event_t));
	memset(api_el,0,sizeof(wp_api_event_t));

	api_el->wp_api_event_type = WP_TDMAPI_EVENT_MODEM_STATUS;
	api_el->wp_api_event_serial_status = card->u.aft.serial_status&AFT_SERIAL_LCFG_CTRL_BIT_MASK;

	skb->pkt_type = WAN_PACKET_ERR;
	skb->protocol=htons(PVC_PROT);
	skb->dev=chan->common.dev;
	
	DEBUG_TEST("%s: Sending OOB message len=%i\n",
			chan->if_name, wan_skb_len(skb));

	if (wan_api_rx(chan,skb)!=0){
		err=-ENODEV;
		wan_skb_free(skb);
	}
	return err;

#else
	DEBUG_EVENT("%s(): OOB messages not supported!\n", __FUNCTION__);
	return -EINVAL;
#endif
}

int aft_register_dump(sdla_t *card)
{
	u32 reg,dma_ram_reg,ctrl_ram_reg,dma_descr;
	private_area_t *chan;
	int dma_cnt=1;
	int i,x;

	DEBUG_EVENT("\n");
	DEBUG_EVENT("%s: GLOBAL REGISTER DUMP\n",card->devname);
	DEBUG_EVENT("===========================================\n");

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG),&reg);
	DEBUG_EVENT("%s: AFT_CHIP_CFG_REG                Reg=0x%04X  Val=0x%08X\n",
				card->devname,AFT_PORT_REG(card,AFT_CHIP_CFG_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG),&reg);
	DEBUG_EVENT("%s: AFT_LINE_CFG_REG                Reg=0x%04X  Val=0x%08X\n",
				card->devname,AFT_PORT_REG(card,AFT_LINE_CFG_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
	DEBUG_EVENT("%s: AFT_DMA_CTRL_REG                Reg=0x%04X  Val=0x%08X\n",
				card->devname,
				AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_DMA_CTRL_REG),&reg);
	DEBUG_EVENT("%s: AFT_RX_DMA_CTRL_REG             Reg=0x%04X  Val=0x%08X\n",
				card->devname,
				AFT_PORT_REG(card,AFT_RX_DMA_CTRL_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_DMA_CTRL_REG),&reg);
	DEBUG_EVENT("%s: AFT_TX_DMA_CTRL_REG             Reg=0x%04X  Val=0x%08X\n",
				card->devname,
				AFT_PORT_REG(card,AFT_TX_DMA_CTRL_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_DMA_INTR_MASK_REG),&reg);
	DEBUG_EVENT("%s: AFT_TX_DMA_INTR_MASK_REG        Reg=0x%04X  Val=0x%08X\n",
				card->devname,
				AFT_PORT_REG(card,AFT_TX_DMA_INTR_MASK_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_DMA_INTR_PENDING_REG),&reg);
	DEBUG_EVENT("%s: AFT_TX_DMA_INTR_PENDING_REG     Reg=0x%04X  Val=0x%08X\n",
				card->devname,
				AFT_PORT_REG(card,AFT_TX_DMA_INTR_PENDING_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG),&reg);
	DEBUG_EVENT("%s: AFT_TX_FIFO_INTR_PENDING_REG    Reg=0x%04X  Val=0x%08X\n",
				card->devname,
				AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_DMA_INTR_MASK_REG),&reg);
	DEBUG_EVENT("%s: AFT_RX_DMA_INTR_MASK_REG        Reg=0x%04X  Val=0x%08X\n",
				card->devname,
				AFT_PORT_REG(card,AFT_RX_DMA_INTR_MASK_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_DMA_INTR_PENDING_REG),&reg);
	DEBUG_EVENT("%s: AFT_RX_DMA_INTR_PENDING_REG     Reg=0x%04X  Val=0x%08X\n",
				card->devname,
				AFT_PORT_REG(card,AFT_RX_DMA_INTR_PENDING_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG),&reg);
	DEBUG_EVENT("%s: AFT_RX_FIFO_INTR_PENDING_REG    Reg=0x%04X  Val=0x%08X\n",
				card->devname,
				AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG),reg);	

	DEBUG_EVENT("\n");
	DEBUG_EVENT("%s: DMA REGISTER DUMP\n",card->devname);
	DEBUG_EVENT("===========================================\n");

	for (i=0;i<32;i++) {

		dma_ram_reg=AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		dma_ram_reg+=(i*4);
	
		card->hw_iface.bus_read_4(card->hw,dma_ram_reg,&reg);
		DEBUG_EVENT("%s: Reg=0x%04X  Val=0x0%08X\n",
				card->devname,
				dma_ram_reg,reg);
	}

	DEBUG_EVENT("\n");	
	DEBUG_EVENT("%s: CONTROL REGISTER DUMP\n",card->devname);
	DEBUG_EVENT("===========================================\n");

	for (i=0;i<32;i++) {

		ctrl_ram_reg=AFT_PORT_REG(card,AFT_CONTROL_RAM_ACCESS_BASE_REG);
		ctrl_ram_reg+=(i*4);

		card->hw_iface.bus_read_4(card->hw,ctrl_ram_reg,&reg);
		DEBUG_EVENT("%s: Reg=0x%04X  Val=0x0%08X\n",
				card->devname,
				ctrl_ram_reg,reg);
	}

	DEBUG_EVENT("\n");
	DEBUG_EVENT("%s: DMA DESCR REGISTER DUMP\n",card->devname);
	DEBUG_EVENT("===========================================\n");
	for (i=0;i<32;i++) {

		if (!wan_test_bit(i,&card->u.aft.active_ch_map)) {
			continue;
		}
		chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
		if (!chan){
			DEBUG_EVENT("%s: Error: No Dev for Rx logical ch=%d\n",
					card->devname,i);
			continue;
		}

		if (chan->dma_chain_opmode != WAN_AFT_DMA_CHAIN_SINGLE){
			dma_cnt=16;
		}

		for (x=0;x<dma_cnt;x++) {

			dma_descr=(chan->logic_ch_num<<4) + (x*AFT_DMA_INDEX_OFFSET) +
							AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);
	
			card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);
			DEBUG_EVENT("%s: TX HI Chan=%02i Index=%02i Reg=0x%04X  Val=0x%08X\n",
					card->devname, chan->logic_ch_num, x,dma_descr,reg);

			dma_descr=(chan->logic_ch_num<<4) + (x*AFT_DMA_INDEX_OFFSET) +
							AFT_PORT_REG(card,AFT_TX_DMA_LO_DESCR_BASE_REG);
	
			card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);
			DEBUG_EVENT("%s: TX LO Chan=%02i Index=%02i                                       Reg=0x%04X  Val=0x%08X\n",
					card->devname, chan->logic_ch_num, x,dma_descr,reg);

			dma_descr=(chan->logic_ch_num<<4) + (x*AFT_DMA_INDEX_OFFSET) +
							AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);
	
			card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);
			DEBUG_EVENT("%s: RX HI Chan=%02i Index=%02i Reg=0x%04X  Val=0x%08X\n",
					card->devname, chan->logic_ch_num, x,dma_descr,reg);



			dma_descr=(chan->logic_ch_num<<4) + (x*AFT_DMA_INDEX_OFFSET) +
							AFT_PORT_REG(card,AFT_RX_DMA_LO_DESCR_BASE_REG);
	
			card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);
			DEBUG_EVENT("%s: RX LO Chan=%02i Index=%02i                                       Reg=0x%04X  Val=0x%08X\n",
					card->devname, chan->logic_ch_num, x,dma_descr,reg);

		}

	}

	return 0;
}

