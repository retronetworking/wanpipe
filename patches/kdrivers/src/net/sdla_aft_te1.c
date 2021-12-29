/*****************************************************************************
* sdla_aft_te1.c 
* 		
* 		WANPIPE(tm) AFT TE1 Hardware Support
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003-2006 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jan 07, 2003	Nenad Corbic	Initial version.
* Oct 25, 2004  Nenad Corbic	Support for QuadPort TE1
*****************************************************************************/

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe.h>
# include <wanpipe_abstr.h> 
# include <wanpipe_snmp.h> 
# include <if_wanpipe_common.h>    /* Socket Driver common area */
# include <sdlapci.h>
# include <sdla_aft_te1.h>
# include <wanpipe_iface.h>
#else
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe.h>
# include <linux/wanproc.h>
# include <linux/wanpipe_abstr.h>
# include <linux/if_wanpipe_common.h>    /* Socket Driver common area */
# include <linux/if_wanpipe.h>
# include <linux/sdlapci.h>
# include <linux/sdla_aft_te1.h>
# include <linux/wanpipe_iface.h>
# include <linux/wanpipe_tdm_api.h>
#endif

#if defined(CONFIG_WANPIPE_HWEC) 
# include <wanec_iface.h>
#endif

#define INIT_FE_ONLY 0

#undef DEBUG_REG

#if 1
#define AFT_FUNC_DEBUG()
#else
#define AFT_FUNC_DEBUG()  DEBUG_EVENT("%s:%d\n",__FUNCTION__,__LINE__)
#endif



#if 0
# define AFT_XTEST_UPDATE 1
#else
# undef AFT_XTEST_UPDATE
#endif


#if 1
# define AFT_SECURITY_CHECK 1
#else
# undef AFT_SECURITY_CHECK
# warning "AFT_SECURITY_CHECK disabled"
#endif

#if 1
# define AFT_WDT_ENABLE 1
#else
# warning "DISABLED WDT"
# undef AFT_WDT_ENABLE
#endif

#if 0
# define AFT_RX_FIFO_DEBUG 1
# warning "AFT_RX_FIFO_DEBUG Flag used"
#else
# undef AFT_RX_FIFO_DEBUG
#endif

#if 0
# define AFT_TX_FIFO_DEBUG 1
# warning "AFT_TX_FIFO_DEBUG Flag used"
#else
# undef AFT_TX_FIFO_DEBUG
#endif

#if 0
# define AFT_SINGLE_DMA_CHAIN 1
# warning "AFT_SINGLE_DMA_CHAIN: SET"
#else
# undef AFT_SINGLE_DMA_CHAIN
#endif

#if 1
# define AFT_IFT_INTR_ENABLE 1
#else
# warning "AFT_IFT_INTR_ENABLE NOT ENABLED"
# undef AFT_IFT_INTR_ENABLE 
#endif

#if 0
# warning "IRQ INTR DEBUGGIN ON"
# define AFT_IRQ_DEBUG 1
#else
# undef AFT_IRQ_DEBUG
#endif

#if 0
# warning "IRQ STAT DEBUGGIN ON"
# define AFT_IRQ_STAT_DEBUG 1
#else
# undef AFT_IRQ_STAT_DEBUG
#endif


#if 0
# define AFT_TDMV_BH_ENABLE 1
# error "AFT_TDMV_BH_ENABLE flag used"
#else
# undef AFT_TDMV_BH_ENABLE 
#endif

#if 1
# define AFT_TDMV_CHANNELIZATION 1
#else
# undef AFT_TDMV_CHANNELIZATION
#endif

#if 1
# define AFT_CLOCK_SYNC 1
#else
# undef AFT_CLOCK_SYNC
#endif

#if defined(__LINUX__)
#define AFT_TDM_API_SUPPORT 1    
#else
#undef AFT_TDM_API_SUPPORT
#endif

#if defined(__LINUX__)
#define AFT_API_SUPPORT 1    
#else
#undef AFT_API_SUPPORT
#endif


/* Trigger on Number of transactions 
 * 1= 1x8 byte transactions
 * 2= 2x8 byte transactions
 * 3= 3x8 byte transactions
 * 4= 4x8 byte transactions
 */
#define AFT_TDMV_FIFO_LEVEL 1	
#define AFT_TDMV_CIRC_BUF   128
#define AFT_TDMV_CIRC_BUF_LEN 4
#define AFT_TDMV_BUF_MASK   0x1FF

#define AFT_SS7_CTRL_LEN_MASK  0x0F
#define AFT_SS7_CTRL_TYPE_BIT  4
#define AFT_SS7_CTRL_FORCE_BIT 5

#define AFT_MAX_CHIP_SECURITY_CNT 100

#define AFT_FE_FIX_FIRM_VER    100

aft_hw_dev_t aft_hwdev[MAX_AFT_HW_DEV];

enum {
        TDM_RUNNING,
	TDM_PENDING,
};

/****** Defines & Macros ****************************************************/

/* Private critical flags */
enum {
	POLL_CRIT = PRIV_CRIT,
	CARD_DOWN,
	TE_CFG,
	CARD_HW_EC
};

enum { 
	LINK_DOWN,
	DEVICE_DOWN
};

enum {
	AFT_CHIP_CONFIGURED,
	AFT_FRONT_END_UP,
	AFT_TDM_GLOBAL_ISR,
	AFT_TDM_RING_BUF,
	AFT_TDM_FAST_ISR
};

enum {
	TX_DMA_BUSY,
	TX_HANDLER_BUSY,
	TX_INTR_PENDING,

	RX_HANDLER_BUSY,
	RX_DMA_BUSY,
	RX_INTR_PENDING
};

enum {
	AFT_FE_CFG_ERR,
	AFT_FE_CFG,
	AFT_FE_INTR,
	AFT_FE_POLL,
	AFT_FE_TDM_RBS,
	AFT_FE_LED,
	AFT_FE_EC_POLL
};

#define MAX_IP_ERRORS	10

#define PORT(x)   (x == 0 ? "PRIMARY" : "SECONDARY" )


#if 1 
# define TRUE_FIFO_SIZE 1
#else
# undef  TRUE_FIFO_SIZE
# define HARD_FIFO_CODE 0x1F
#endif


/* Remove HDLC Address 
 * 1=Remove Enabled
 * 0=Remove Disabled
 */

#if 0
#define WANPIPE_CODEC_CONVERTER 1 
#else
#undef WANPIPE_CODEC_CONVERTER
#endif

static int aft_rx_copyback=500;
/******Data Structures*****************************************************/

/* This structure is placed in the private data area of the device structure.
 * The card structure used to occupy the private area but now the following
 * structure will incorporate the card structure along with Protocol specific data
 */


/* Route Status options */
#define NO_ROUTE	0x00
#define ADD_ROUTE	0x01
#define ROUTE_ADDED	0x02
#define REMOVE_ROUTE	0x03

#define WP_WAIT 	0
#define WP_NO_WAIT	1

/* variable for keeping track of enabling/disabling FT1 monitor status */
/* static int rCount; */

/* Function interface between WANPIPE layer and kernel */
extern wan_iface_t wan_iface;

extern void disable_irq(unsigned int);
extern void enable_irq(unsigned int);

extern sdla_t* card_list;

/**SECTOIN**************************************************
 *
 * Function Prototypes
 *
 ***********************************************************/
int wp_aft_te1default_devcfg(sdla_t* card, wandev_conf_t* conf);
int wp_aft_te1default_ifcfg(sdla_t* card, wanif_conf_t* conf);

/* WAN link driver entry points. These are called by the WAN router module. */
static int 	update (wan_device_t* wandev);
static int 	new_if (wan_device_t* wandev, netdevice_t* dev, wanif_conf_t* conf);
static int 	del_if(wan_device_t *wandev, netdevice_t *dev);

/* Network device interface */
#if defined(__LINUX__)
static int 	if_init   (netdevice_t* dev);
#endif
static int 	if_open   (netdevice_t* dev);
static int 	if_close  (netdevice_t* dev);
static int 	if_do_ioctl(netdevice_t *dev, struct ifreq *ifr, int cmd);

static struct net_device_stats* if_stats (netdevice_t* dev);

#if defined(__LINUX__)
static int 	if_send (netskb_t* skb, netdevice_t* dev);
#else
static int	if_send(netdevice_t*, netskb_t*, struct sockaddr*,struct rtentry*);
#endif 

static void 	handle_front_end_state(void* card_id);
static void 	enable_timer(void* card_id);
static void 	enable_ec_timer(void* card_id);
static void 	if_tx_timeout (netdevice_t *dev);

/* Miscellaneous Functions */
static void 	port_set_state (sdla_t *card, int);

static void 	disable_comm (sdla_t *card);

/* Interrupt handlers */
static WAN_IRQ_RETVAL 	wp_aft_global_isr (sdla_t* card);
static void 	wp_aft_dma_per_port_isr(sdla_t *card);
static void 	wp_aft_tdmv_per_port_isr(sdla_t *card);
static void 	wp_aft_fifo_per_port_isr(sdla_t *card);
static void 	wp_aft_wdt_per_port_isr(sdla_t *card, int wdt_intr);	

/* Bottom half handlers */
#if defined(__LINUX__)
static void 	wp_bh (unsigned long);
static void	wp_tdm_bh (unsigned long);
#else
static void 	wp_bh (void*,int);
static void	wp_tdm_bh (void*,int);
#endif

/* Miscellaneous functions */
static int 	process_udp_mgmt_pkt(sdla_t* card, netdevice_t* dev,
				private_area_t*,
				int local_dev);

static int 	aft_global_chip_configuration(sdla_t *card, wandev_conf_t* conf);
static int 	aft_global_chip_disable(sdla_t *card);

static int 	aft_chip_configure(sdla_t *card, wandev_conf_t* conf);
static int 	aft_chip_unconfigure(sdla_t *card);
static int 	aft_dev_configure(sdla_t *card, private_area_t *chan, wanif_conf_t* conf);
static void 	aft_dev_unconfigure(sdla_t *card, private_area_t *chan);

static int 	aft_dma_rx(sdla_t *card, private_area_t *chan);
static void 	aft_dev_enable(sdla_t *card, private_area_t *chan);
static void 	aft_dev_close(sdla_t *card, private_area_t *chan);
static void 	aft_dev_open(sdla_t *card, private_area_t *gchan);
static void 	aft_dma_tx_complete (sdla_t *card, private_area_t *chan,int wdt, int reset);

static int 	aft_dma_rx_complete(sdla_t *card, private_area_t *chan, int reset);
static int 	aft_init_rx_dev_fifo(sdla_t *card, private_area_t *chan, unsigned char);
static int 	aft_init_tx_dev_fifo(sdla_t *card, private_area_t *chan, unsigned char);
static void 	aft_tx_post_complete (sdla_t *card, private_area_t *chan, netskb_t *skb);
static void 	aft_rx_post_complete (sdla_t *card, private_area_t *chan,
                                     netskb_t *skb,
                                     netskb_t **new_skb,
                                     unsigned char *pkt_error);
static int 	aft_dma_rx_tdmv(sdla_t *card, private_area_t *chan);
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
static int	aft_voice_span_rx_tx(sdla_t *card, int rotate);
#endif
 
static void 	aft_channel_txdma_ctrl(sdla_t *card, private_area_t *chan, int on);
static void 	aft_channel_rxdma_ctrl(sdla_t *card, private_area_t *chan, int on);
static void 	aft_channel_txintr_ctrl(sdla_t *card, private_area_t *chan, int on);
static void 	aft_channel_rxintr_ctrl(sdla_t *card, private_area_t *chan, int on);

static int 	aft_read_security(sdla_t *card);
static int 	aft_front_end_mismatch_check(sdla_t * card);
static int 	aft_tslot_sync_ctrl(sdla_t *card, private_area_t *chan, int mode);

#if defined(__LINUX__)
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))     
static void aft_port_task (void * card_ptr);
# else
static void aft_port_task (struct work_struct *work);	
# endif
#else
static void aft_port_task (void * card_ptr, int arg);
#endif  

#if defined(__LINUX__)
static void 	aft_rtp_config(sdla_t *card);
static void 	aft_rtp_unconfig(sdla_t *card); 
#if 0
static void 	aft_rtp_tap(sdla_t *card, private_area_t *chan, u8* rx, u8* tx, u32 len);   
#endif
#endif

static int 	aft_devel_ioctl(sdla_t *card,struct ifreq *ifr);
static int 	aft_write_bios(sdla_t *card, wan_cmd_api_t *api_cmd);
static int 	aft_write(sdla_t *card, wan_cmd_api_t *api_cmd);
static int 	aft_read(sdla_t *card, wan_cmd_api_t *api_cmd);
static int 	aft_fe_write(sdla_t *card, wan_cmd_api_t *api_cmd);
static int 	aft_fe_read(sdla_t *card, wan_cmd_api_t *api_cmd);

static void 	front_end_interrupt(sdla_t *card, unsigned long reg, int lock);
static void  	enable_data_error_intr(sdla_t *card);
static void  	disable_data_error_intr(sdla_t *card, unsigned char);

static void 	aft_tx_fifo_under_recover (sdla_t *card, private_area_t *chan);
static void     aft_rx_fifo_over_recover(sdla_t *card, private_area_t *chan);

static int 	set_chan_state(sdla_t* card, netdevice_t* dev, int state);

static int 	update_comms_stats(sdla_t* card);

static int 	protocol_init (sdla_t*card,netdevice_t *dev,
		               private_area_t *chan, wanif_conf_t* conf);
static int 	protocol_stop (sdla_t *card, netdevice_t *dev);
static int 	protocol_shutdown (sdla_t *card, netdevice_t *dev);
static void 	protocol_recv(sdla_t *card, private_area_t *chan, netskb_t *skb);

static int 	aft_alloc_rx_dma_buff(sdla_t *card, private_area_t *chan, int num, int irq);
static int 	aft_init_requeue_free_skb(private_area_t *chan, netskb_t *skb);


static int 	aft_dma_tx (sdla_t *card,private_area_t *chan);
static void 	aft_tx_dma_chain_handler(unsigned long data, int wdt, int reset);
static void 	aft_tx_dma_voice_handler(unsigned long data, int wdt, int reset);
static void 	aft_tx_dma_chain_init(private_area_t *chan, aft_dma_chain_t *);
static void 	aft_rx_dma_chain_init(private_area_t *chan, aft_dma_chain_t *);
static void 	aft_index_tx_rx_dma_chains(private_area_t *chan);
static void 	aft_init_tx_rx_dma_descr(private_area_t *chan);
static void 	aft_free_rx_complete_list(private_area_t *chan);
static void 	aft_rx_cur_go_test(private_area_t *chan);
static void 	aft_free_rx_descriptors(private_area_t *chan);
static void 	aft_reset_rx_chain_cnt(private_area_t *chan);
static void 	aft_reset_tx_chain_cnt(private_area_t *chan);
static void 	aft_free_tx_descriptors(private_area_t *chan);


static int 	aft_realign_skb_pkt(private_area_t *chan, netskb_t *skb);

static void 	aft_data_mux_cfg(sdla_t *card);
static void 	aft_data_mux_get_cfg(sdla_t *card);

static int 	aft_ss7_tx_mangle(sdla_t *card,private_area_t *chan, netskb_t *skb);

static int 	aft_tdmv_init(sdla_t *card, wandev_conf_t *conf);
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
static int 	aft_tdmv_free(sdla_t *card);
#endif
static int 	aft_tdmv_if_init(sdla_t *card, private_area_t *chan, wanif_conf_t *conf);
static int 	aft_tdmv_if_free(sdla_t *card, private_area_t *chan);

static int 	digital_loop_test(sdla_t* card,wan_udp_pkt_t* wan_udp_pkt);

#if 0
static void 	wp_tdmv_api_chan_rx_tx(sdla_t *card, 
			      private_area_t *chan,
			      unsigned char *rxdata, unsigned char *tx_data);
static void 	wp_tdmv_api_rx_tx (sdla_t *card, private_area_t *chan);
#endif

static int 	aft_fifo_intr_ctrl(sdla_t *card, int ctrl);
static int 	aft_tdm_intr_ctrl(sdla_t *card, int ctrl);

#if defined(__LINUX__)
static void 	aft_set_ss7_force_rx(sdla_t *card, private_area_t *chan);
#endif
static void 	aft_clear_ss7_force_rx(sdla_t *card, private_area_t *chan);

#if defined(AFT_API_SUPPORT) || defined(AFT_TDM_API_SUPPORT)
static int aft_event_ctrl(void *chan_ptr, wan_event_ctrl_t *ctrl);
#endif

#ifdef AFT_TDM_API_SUPPORT
static int aft_read_rbs_bits(void *chan_ptr, u32 ch, u8 *rbs_bits);
static int aft_write_rbs_bits(void *chan_ptr, u32 ch, u8 rbs_bits);
static int aft_write_hdlc_frame(void *chan_ptr,  netskb_t *skb);
#endif
static int aft_tdm_ring_rsync(sdla_t *card);
static void aft_critical_shutdown(sdla_t *card);

/* API VoIP event */
#if defined(AFT_API_SUPPORT)
static int wan_aft_api_ioctl(sdla_t *card, private_area_t *chan, char *user_data);
static void wan_aft_api_dtmf (void* card_id, wan_event_t *event);
static void wan_aft_api_hook (void* card_id, wan_event_t *event);
static void wan_aft_api_ringtrip (void* card_id, wan_event_t *event);
static void wan_aft_api_ringdetect (void* card_id, wan_event_t *event);
#endif

#if 0
static void 	aft_list_descriptors(private_area_t *chan);
#endif
#if 0
static void	aft_list_dma_chain_regs(sdla_t *card);
#endif

#if 0
static void 	aft_list_tx_descriptors(private_area_t *chan);
#endif
#if 0
static void 	aft_display_chain_history(private_area_t *chan);
static void 	aft_chain_history(private_area_t *chan,u8 end, u8 cur, u8 begin, u8 loc);
#endif 


/* TE1 Control registers  */
#if 0
static WRITE_FRONT_END_REG_T write_front_end_reg;
static READ_FRONT_END_REG_T  read_front_end_reg;
#endif

unsigned char aft_read_cpld(sdla_t *card, unsigned short cpld_off);
int aft_write_cpld(void *pcard, unsigned short off,unsigned char data);

/* Procfs functions */
static int wan_aft_get_info(void* pcard, struct seq_file* m, int* stop_cnt); 

static int wan_aft_init (sdla_t *card, wandev_conf_t* conf);


static unsigned char aft_write_ec (void*, unsigned short, unsigned char);
static unsigned char aft_read_ec (void*, unsigned short);

static int aft_hwec_config(sdla_t *card, private_area_t *chan, wanif_conf_t *conf, int ctrl);
static int aft_find_master_if_and_dchan(sdla_t *card, int *master_if,u32 active_ch);


/**SECTION*********************************************************
 *
 * Public Functions
 *
 ******************************************************************/

int wp_aft_te1default_devcfg(sdla_t* card, wandev_conf_t* conf)
{
	conf->config_id			= WANCONFIG_AFT_TE1;
	conf->u.aft.dma_per_ch	= MAX_RX_BUF;
	conf->u.aft.mru	= 1500;
	return 0;
}

int wp_aft_te1default_ifcfg(sdla_t* card, wanif_conf_t* conf)
{
	conf->protocol = WANCONFIG_HDLC;
	memcpy(conf->usedby, "WANPIPE", 7);
	conf->if_down = 0;
	conf->ignore_dcd = WANOPT_NO;
	conf->ignore_cts = WANOPT_NO;
	conf->hdlc_streaming = WANOPT_NO;
	conf->mc = 0;
	conf->gateway = 0;
	conf->active_ch = ENABLE_ALL_CHANNELS;

	return 0;
}

#if 0
static void aft_delay(int sec)
{
#if 1
        unsigned long timeout=SYSTEM_TICKS;
        while ((SYSTEM_TICKS-timeout)<(sec*HZ)){
                schedule();
        }
#endif
}
#endif


int aft_global_hw_device_init(void)
{
	memset(aft_hwdev,0,sizeof(aft_hwdev));

	aft_hwdev[WANOPT_AFT104].init				= 1;
	aft_hwdev[WANOPT_AFT104].aft_global_chip_config 	= a104_global_chip_config;
	aft_hwdev[WANOPT_AFT104].aft_global_chip_unconfig	= a104_global_chip_unconfig;	
	aft_hwdev[WANOPT_AFT104].aft_chip_config		= a104_chip_config;
	aft_hwdev[WANOPT_AFT104].aft_chip_unconfig		= a104_chip_unconfig; 
	aft_hwdev[WANOPT_AFT104].aft_chan_config		= a104_chan_dev_config; 
	aft_hwdev[WANOPT_AFT104].aft_chan_unconfig 		= a104_chan_dev_unconfig;
	aft_hwdev[WANOPT_AFT104].aft_led_ctrl 			= a104_led_ctrl;
	aft_hwdev[WANOPT_AFT104].aft_test_sync 			= a104_test_sync;
	aft_hwdev[WANOPT_AFT104].aft_read_cpld			= aft_te1_read_cpld;
	aft_hwdev[WANOPT_AFT104].aft_write_cpld			= aft_te1_write_cpld;
	aft_hwdev[WANOPT_AFT104].aft_fifo_adjust		= a104_fifo_adjust;
	aft_hwdev[WANOPT_AFT104].aft_check_ec_security	= a104_check_ec_security; 

	aft_hwdev[WANOPT_AFT_ANALOG].init			= 1;
	aft_hwdev[WANOPT_AFT_ANALOG].aft_global_chip_config 	= aft_analog_global_chip_config;
	aft_hwdev[WANOPT_AFT_ANALOG].aft_global_chip_unconfig	= aft_analog_global_chip_unconfig;	
	aft_hwdev[WANOPT_AFT_ANALOG].aft_chip_config		= aft_analog_chip_config;
	aft_hwdev[WANOPT_AFT_ANALOG].aft_chip_unconfig		= aft_analog_chip_unconfig; 
	aft_hwdev[WANOPT_AFT_ANALOG].aft_chan_config		= aft_analog_chan_dev_config; 
	aft_hwdev[WANOPT_AFT_ANALOG].aft_chan_unconfig 		= aft_analog_chan_dev_unconfig;
	aft_hwdev[WANOPT_AFT_ANALOG].aft_led_ctrl 		= aft_analog_led_ctrl;
	aft_hwdev[WANOPT_AFT_ANALOG].aft_test_sync 		= aft_analog_test_sync;
	aft_hwdev[WANOPT_AFT_ANALOG].aft_read_cpld		= aft_analog_read_cpld;
	aft_hwdev[WANOPT_AFT_ANALOG].aft_write_cpld		= aft_analog_write_cpld;
	aft_hwdev[WANOPT_AFT_ANALOG].aft_fifo_adjust		= aft_analog_fifo_adjust;
    	aft_hwdev[WANOPT_AFT_ANALOG].aft_check_ec_security	= a200_check_ec_security;     

	aft_hwdev[WANOPT_AFT_56K].init				= 1;
	aft_hwdev[WANOPT_AFT_56K].aft_global_chip_config 	= a104_global_chip_config;
	aft_hwdev[WANOPT_AFT_56K].aft_global_chip_unconfig	= a104_global_chip_unconfig;	
	aft_hwdev[WANOPT_AFT_56K].aft_chip_config		= a104_chip_config;
	aft_hwdev[WANOPT_AFT_56K].aft_chip_unconfig		= a104_chip_unconfig; 
	aft_hwdev[WANOPT_AFT_56K].aft_chan_config		= a104_chan_dev_config; 
	aft_hwdev[WANOPT_AFT_56K].aft_chan_unconfig 		= a104_chan_dev_unconfig;
	aft_hwdev[WANOPT_AFT_56K].aft_led_ctrl 			= a104_led_ctrl;
	aft_hwdev[WANOPT_AFT_56K].aft_test_sync 			= a104_test_sync;
	aft_hwdev[WANOPT_AFT_56K].aft_read_cpld			= aft_56k_read_cpld;
	aft_hwdev[WANOPT_AFT_56K].aft_write_cpld			= aft_56k_write_cpld;
	aft_hwdev[WANOPT_AFT_56K].aft_fifo_adjust		= a104_fifo_adjust;
	aft_hwdev[WANOPT_AFT_56K].aft_check_ec_security	= a104_check_ec_security; 

	return 0;
}


/*============================================================================
 * wp_aft_te1_init - Cisco HDLC protocol initialization routine.
 *
 * @card:	Wanpipe card pointer
 * @conf:	User hardware/firmware/general protocol configuration
 *              pointer.
 *
 * This routine is called by the main WANPIPE module
 * during setup: ROUTER_SETUP ioctl().
 *
 * At this point adapter is completely initialized
 * and firmware is running.
 *  o read firmware version (to make sure it's alive)
 *  o configure adapter
 *  o initialize protocol-specific fields of the adapter data space.
 *
 * Return:	0	o.k.
 *		< 0	failure.
 */

int wp_aft_analog_init (sdla_t *card, wandev_conf_t* conf)
{
	/* Verify configuration ID */
	if (card->wandev.config_id != WANCONFIG_AFT_ANALOG) {
		DEBUG_EVENT( "%s: invalid configuration ID %u!\n",
				  card->devname, card->wandev.config_id);
		return -EINVAL;
	}

	ASSERT_AFT_HWDEV(card->wandev.card_type);

	card->hw_iface.getcfg(card->hw, SDLA_COREREV, &card->u.aft.firm_ver);
	card->hw_iface.getcfg(card->hw, SDLA_COREID, &card->u.aft.firm_id);
	if (card->u.aft.firm_ver < AFT_MIN_ANALOG_FRMW_VER){
		DEBUG_EVENT( "%s: Invalid/Obselete AFT ANALOG firmware version %X (not >= %X)!\n",
				  card->devname, card->u.aft.firm_ver,AFT_MIN_ANALOG_FRMW_VER);
		DEBUG_EVENT( "%s  Refer to /usr/share/doc/wanpipe/README.aft_firm_update\n",
				  card->devname);
		DEBUG_EVENT( "%s: Please contact Sangoma Technologies for more info.\n",
				  card->devname);
		return -EINVAL;
	}

	if (conf == NULL){
		DEBUG_EVENT("%s: Bad configuration structre!\n",
				card->devname);
		return -EINVAL;
	}

	/* Make special hardware initialization for Analog board */
	memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));
	wp_remora_iface_init(&card->wandev.fe_iface);
	card->fe.name		= card->devname;
	card->fe.card		= card;
	card->fe.write_fe_reg	= aft_analog_write_fe;
	card->fe.read_fe_reg	= aft_analog_read_fe;
	card->fe.__read_fe_reg	= __aft_analog_read_fe;

	card->wandev.fe_enable_timer = enable_timer;
	card->wandev.ec_enable_timer = enable_ec_timer;
	card->wandev.te_link_state = handle_front_end_state;

	if (card->wandev.comm_port == WANOPT_PRI){
		conf->clocking = WANOPT_EXTERNAL;
	}

	card->wandev.comm_port=card->fe.fe_cfg.line_no;
	if (card->wandev.comm_port != 0){
		DEBUG_EVENT("%s: Error: Invalid port selected %d (Port 1)\n",
				card->devname,card->wandev.comm_port);
		return -EINVAL;
	}

	card->u.aft.num_of_time_slots=MAX_REMORA_MODULES;

	return 	wan_aft_init(card,conf);

}


int wp_aft_te1_init (sdla_t* card, wandev_conf_t* conf)
{

	AFT_FUNC_DEBUG();

	wan_set_bit(CARD_DOWN,&card->wandev.critical);
	
	/* Verify configuration ID */
	if (card->wandev.config_id != WANCONFIG_AFT_TE1) {
		DEBUG_EVENT( "%s: invalid configuration ID %u!\n",
				  card->devname, card->wandev.config_id);
		return -EINVAL;
	}


	card->hw_iface.getcfg(card->hw, SDLA_COREREV, &card->u.aft.firm_ver);
	card->hw_iface.getcfg(card->hw, SDLA_COREID, &card->u.aft.firm_id);

	if (card->u.aft.firm_ver < AFT_MIN_FRMW_VER){
		DEBUG_EVENT( "%s: Invalid/Obselete AFT firmware version %X (not >= %X)!\n",
				  card->devname, card->u.aft.firm_ver,AFT_MIN_FRMW_VER);
		DEBUG_EVENT( "%s  Refer to /usr/share/doc/wanpipe/README.aft_firm_update\n",
				  card->devname);
		DEBUG_EVENT( "%s: Please contact Sangoma Technologies for more info.\n",
				  card->devname);
		return -EINVAL;
	}

	ASSERT_AFT_HWDEV(card->wandev.card_type);
	
	if (conf == NULL){
		DEBUG_EVENT("%s: Bad configuration structre!\n",
				card->devname);
		return -EINVAL;
	}

#if defined(WAN_DEBUG_MEM)
        DEBUG_EVENT("%s: Total Mem %d\n",__FUNCTION__,wan_atomic_read(&wan_debug_mem));
#endif

	/* TE1 Make special hardware initialization for T1/E1 board */
	if (IS_TE1_MEDIA(&conf->fe_cfg)){
		int max_ports = 4;
	
		if (conf->fe_cfg.cfg.te_cfg.active_ch == 0){
			conf->fe_cfg.cfg.te_cfg.active_ch = -1;
		}

		memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));
		if (card->u.aft.firm_id == AFT_DS_FE_CORE_ID) {
			max_ports = 8;
			sdla_ds_te1_iface_init(&card->wandev.fe_iface);
		}else{
			sdla_te_iface_init(&card->wandev.fe_iface);
		}
		card->fe.name		= card->devname;
		card->fe.card		= card;
		card->fe.write_fe_reg	= a104_write_fe;
		card->fe.read_fe_reg	= a104_read_fe;
		card->fe.__read_fe_reg	= __a104_read_fe;

		card->wandev.fe_enable_timer = enable_timer;
		card->wandev.ec_enable_timer = enable_ec_timer;
		card->wandev.te_link_state = handle_front_end_state;
		conf->interface =
			IS_T1_CARD(card) ? WANOPT_V35 : WANOPT_RS232;

		if (card->wandev.comm_port == WANOPT_PRI){
			conf->clocking = WANOPT_EXTERNAL;
		}

		card->wandev.comm_port=card->fe.fe_cfg.line_no;
#if 0		
/* ALEX: This will check during t1/e1 configuration */
		if (card->wandev.comm_port < 0 || card->wandev.comm_port > max_ports-1){
			DEBUG_EVENT("%s: Error: Invalid port selected %d (Min=1 Max=%d)\n",
					card->devname,card->wandev.comm_port, max_ports);
			return -EINVAL;
		}
#endif

		if (IS_T1_CARD(card)){
			card->u.aft.num_of_time_slots=NUM_OF_T1_CHANNELS;
		}else{
			card->u.aft.num_of_time_slots=NUM_OF_E1_CHANNELS;
		}

	}else{
		DEBUG_EVENT("%s: Invalid Front-End media type!!\n",
				card->devname);
		return -EINVAL;
	}

	return 	wan_aft_init(card,conf);

}

int wp_aft_56k_init (sdla_t* card, wandev_conf_t* conf)
{

	AFT_FUNC_DEBUG();

	wan_set_bit(CARD_DOWN,&card->wandev.critical);
	
	/* Verify configuration ID */
	if (card->wandev.config_id != WANCONFIG_AFT_56K) {
		DEBUG_EVENT( "%s: invalid configuration ID %u!\n",
				  card->devname, card->wandev.config_id);
		return -EINVAL;
	}


	card->hw_iface.getcfg(card->hw, SDLA_COREREV, &card->u.aft.firm_ver);
	card->hw_iface.getcfg(card->hw, SDLA_COREID, &card->u.aft.firm_id);
#if 0
	if (card->u.aft.firm_ver < AFT_56K_MIN_FRMW_VER){
		DEBUG_EVENT( "%s: Invalid/Obselete AFT firmware version %X (not >= %X)!\n",
				  card->devname, card->u.aft.firm_ver,AFT_56K_MIN_FRMW_VER);
		DEBUG_EVENT( "%s  Refer to /usr/share/doc/wanpipe/README.aft_firm_update\n",
				  card->devname);
		DEBUG_EVENT( "%s: Please contact Sangoma Technologies for more info.\n",
				  card->devname);
		return -EINVAL;
	}
#endif
	ASSERT_AFT_HWDEV(card->wandev.card_type);
	
	if (conf == NULL){
		DEBUG_EVENT("%s: Bad configuration structre!\n",
				card->devname);
		return -EINVAL;
	}

#if defined(WAN_DEBUG_MEM)
        DEBUG_EVENT("%s: Total Mem %d\n",__FUNCTION__,wan_atomic_read(&wan_debug_mem));
#endif

	if (IS_56K_MEDIA(&conf->fe_cfg)){
	
		conf->fe_cfg.cfg.te_cfg.active_ch = 1;

		memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));

		DEBUG_56K("card->u.aft.firm_id: 0x%X\n", card->u.aft.firm_id);
/*
		if(card->u.aft.firm_id != AFT_56K_FE_CORE_ID){
			DEBUG_EVENT("%s: Invalid (56K) Firmware ID: 0x%X!\n",
				card->devname, card->u.aft.firm_id);
			return -EINVAL;
		}
*/
		sdla_56k_iface_init(&card->wandev.fe_iface);

		card->fe.name		= card->devname;
		card->fe.card		= card;
#if 1
		card->fe.write_fe_reg	= a56k_write_fe;
		card->fe.read_fe_reg	= a56k_read_fe;
		card->fe.__read_fe_reg	= __a56k_read_fe;
#else
		card->fe.write_fe_reg	= a104_write_fe;
		card->fe.read_fe_reg	= a104_read_fe;
		card->fe.__read_fe_reg	= __a104_read_fe;
#endif
		card->wandev.fe_enable_timer = enable_timer;
		card->wandev.ec_enable_timer = enable_ec_timer;
		card->wandev.te_link_state = handle_front_end_state;

		card->wandev.comm_port=1;

		card->u.aft.num_of_time_slots=1;

	}else{
		DEBUG_EVENT("%s: Invalid Front-End media type!!\n",
				card->devname);
		return -EINVAL;
	}

	return 	wan_aft_init(card,conf);

}

static int wan_aft_init (sdla_t *card, wandev_conf_t* conf)
{
	int err;
	int used_cnt;

	/* Obtain hardware configuration parameters */
	card->wandev.clocking 			= conf->clocking;
	card->wandev.ignore_front_end_status 	= conf->ignore_front_end_status;
	card->wandev.ttl 			= conf->ttl;
	card->wandev.interface 			= conf->interface;
	card->wandev.comm_port 			= conf->comm_port;
	card->wandev.udp_port   		= conf->udp_port;
	card->wandev.new_if_cnt 		= 0;
	wan_atomic_set(&card->wandev.if_cnt,0);
	card->u.aft.chip_security_cnt=0;

	memcpy(&card->u.aft.cfg,&conf->u.aft,sizeof(wan_xilinx_conf_t));
	memset(card->u.aft.dev_to_ch_map,0,sizeof(card->u.aft.dev_to_ch_map));
	memcpy(&card->tdmv_conf,&conf->tdmv_conf,sizeof(wan_tdmv_conf_t));
	memcpy(&card->hwec_conf,&conf->hwec_conf,sizeof(wan_hwec_conf_t));

	card->u.aft.cfg.dma_per_ch = MAX_RX_BUF;
	card->u.aft.tdmv_api_rx = NULL;
	card->u.aft.tdmv_api_tx = NULL;
	card->u.aft.tdmv_dchan=0;
	wan_skb_queue_init(&card->u.aft.tdmv_api_tx_list);

	if (card->wandev.ignore_front_end_status == WANOPT_NO){
		DEBUG_EVENT(
		  "%s: Enabling front end link monitor\n",
				card->devname);
	}else{
		DEBUG_EVENT(
		"%s: Disabling front end link monitor\n",
				card->devname);
	}

	AFT_FUNC_DEBUG();

	/* WARNING: After this point the init function
	 * must return with 0.  The following bind
	 * functions will cause problems if structures
	 * below are not initialized */
	
        card->wandev.update             = &update;
        card->wandev.new_if             = &new_if;
        card->wandev.del_if             = &del_if;
        card->disable_comm              = NULL;
	

#ifdef WANPIPE_ENABLE_PROC_FILE_HOOKS
	/* Proc fs functions hooks */
	card->wandev.get_config_info 	= &get_config_info;
	card->wandev.get_status_info 	= &get_status_info;
	card->wandev.get_dev_config_info= &get_dev_config_info;
	card->wandev.get_if_info     	= &get_if_info;
	card->wandev.set_dev_config    	= &set_dev_config;
	card->wandev.set_if_info     	= &set_if_info;
#endif
	card->wandev.get_info 		= &wan_aft_get_info;

	/* Setup Port Bps */
	if(card->wandev.clocking) {
		card->wandev.bps = conf->bps;
	}else{
        	card->wandev.bps = 0;
  	}

        /* For Primary Port 0 */
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
        card->wandev.mtu = conf->mtu;
	card->wan_tdmv.sc = NULL;
#else
	card->wandev.mtu=conf->mtu;
	if (card->wandev.mtu > MAX_WP_PRI_MTU ||
	    card->wandev.mtu < MIN_WP_PRI_MTU){
		DEBUG_EVENT("%s: Error Invalid Global MTU %d (Min=%d, Max=%d)\n",
				card->devname,card->wandev.mtu,
				MIN_WP_PRI_MTU,MAX_WP_PRI_MTU);

		return -EINVAL;
	}
#endif


	if (!card->u.aft.cfg.mru){
		card->u.aft.cfg.mru = card->wandev.mtu;
	}


	
	if (card->u.aft.cfg.mru > MAX_WP_PRI_MTU ||
	    card->u.aft.cfg.mru < MIN_WP_PRI_MTU){
		DEBUG_EVENT("%s: Error Invalid Global MRU %d (Min=%d, Max=%d)\n",
				card->devname,card->u.aft.cfg.mru,
				MIN_WP_PRI_MTU,MAX_WP_PRI_MTU);

		return -EINVAL;
	}

	card->hw_iface.getcfg(card->hw, SDLA_BASEADDR, &card->u.aft.bar);
	card->hw_iface.getcfg(card->hw, SDLA_MEMBASE, &card->u.aft.bar_virt);

	port_set_state(card,WAN_CONNECTING);

	AFT_FUNC_DEBUG();

	WAN_TASKQ_INIT((&card->u.aft.port_task),0,aft_port_task,card);
	
	card->u.aft.chip_cfg_status=0;
	card->hw_iface.getcfg(card->hw, SDLA_USEDCNT, &used_cnt);

	wan_clear_bit(CARD_DOWN,&card->wandev.critical);

	__sdla_push_ptr_isr_array(card->hw,card,card->wandev.comm_port);
	
        card->isr = &wp_aft_global_isr;
	
	if (used_cnt==1){
		DEBUG_EVENT("%s: Global Chip Configuration: used=%d\n",
				card->devname,used_cnt);

		err=aft_global_chip_configuration(card, conf);
		if (err){
			aft_global_chip_disable(card);
			return err;
		}

		aft_data_mux_cfg(card);

	}else{

		aft_data_mux_get_cfg(card);

		err=aft_front_end_mismatch_check(card);
		if (err){
			return err;
		}
		
		DEBUG_EVENT("%s: Global Chip Configuration skiped: used=%d\n",
				card->devname,used_cnt);
	}
	card->wandev.ec_intmask=SYSTEM_TICKS;

	aft_read_security(card);
	
	err=aft_chip_configure(card,conf);
	if (err){
		AFT_FUNC_DEBUG();

		aft_chip_unconfigure(card);
		if (used_cnt==1){
			aft_global_chip_disable(card);
		}
		return err;
	}
	wan_set_bit(AFT_CHIP_CONFIGURED,&card->u.aft.chip_cfg_status);

	if (wan_test_bit(AFT_FRONT_END_UP,&card->u.aft.chip_cfg_status)){
		wan_smp_flag_t smp_flags;
		DEBUG_TEST("%s: Front end up, retrying enable front end!\n",
				card->devname);
		wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
		handle_front_end_state(card);
		wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);

		wan_clear_bit(AFT_FRONT_END_UP,&card->u.aft.chip_cfg_status);
	}
	
	AFT_FUNC_DEBUG();

	aft_read_security(card);


	DEBUG_EVENT("%s: Configuring Device   :%s  FrmVr=%02X\n",
			card->devname,card->devname,card->u.aft.firm_ver);
	DEBUG_EVENT("%s:    Global MTU     = %d\n", 
			card->devname, 
			card->wandev.mtu);
	DEBUG_EVENT("%s:    Global MRU     = %d\n", 
			card->devname, 
			card->u.aft.cfg.mru);
	DEBUG_EVENT("%s:    Data Mux Map   = 0x%08X\n",
			card->devname,
			card->u.aft.cfg.data_mux_map);
	DEBUG_EVENT("%s:    Rx CRC Bytes   = %i\n",
			card->devname,
			card->u.aft.cfg.rx_crc_bytes);

        wan_clear_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status);
        wan_clear_bit(AFT_TDM_RING_BUF,&card->u.aft.chip_cfg_status);    

	if (card->u.aft.firm_id == AFT_DS_FE_CORE_ID) {
		if ((card->adptr_type == A108_ADPTR_8TE1 &&
		     card->u.aft.firm_ver >= 0x27) ||
	            (card->adptr_type == A104_ADPTR_4TE1 &&
		     card->u.aft.firm_ver >= 0x26) ||
		    (card->adptr_type == A101_ADPTR_2TE1 &&
		     card->u.aft.firm_ver >= 0x26) ||
		    (card->adptr_type == A101_ADPTR_1TE1 &&
		     card->u.aft.firm_ver >= 0x26)) {
	    	     	wan_set_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status);
               		wan_set_bit(AFT_TDM_RING_BUF,&card->u.aft.chip_cfg_status);      	    
		}
	} else {
            	if ((card->adptr_type == A104_ADPTR_4TE1 &&
		     card->adptr_subtype == AFT_SUBTYPE_SHARK  &&
	     	     card->u.aft.firm_ver >= 0x23)) {
                        wan_set_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status);
               		wan_set_bit(AFT_TDM_RING_BUF,&card->u.aft.chip_cfg_status);  
		}	
	}
	
	DEBUG_EVENT("%s:    Global TDM Int = %s\n",
			card->devname,
			wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status) ?
			"Enabled" : "Disabled");

	DEBUG_EVENT("%s:    Global TDM Ring= %s\n",
			card->devname,
			wan_test_bit(AFT_TDM_RING_BUF,&card->u.aft.chip_cfg_status) ?
			"Enabled" : "Disabled");

	if (card->wandev.ec_dev){
		card->u.aft.tdmv_hw_dtmf = conf->tdmv_conf.hw_dtmf;
	}else{
		card->u.aft.tdmv_hw_dtmf = WANOPT_NO;	
	}
	DEBUG_EVENT("%s:    Global TDM HW DTMF = %s\n",
			card->devname,
			(card->u.aft.tdmv_hw_dtmf == WANOPT_YES) ?
				"Enabled" : "Disabled");
	
	err=aft_tdmv_init(card,conf);
	if (err){
		disable_comm(card);
		return err;
	}

	card->disable_comm = &disable_comm;

        aft_rtp_config(card); 

	
	card->wandev.read_ec = aft_read_ec;
	card->wandev.write_ec = aft_write_ec;
	return 0;

}




/**SECTION**************************************************************
 *
 * 	WANPIPE Device Driver Entry Points
 *
 * *********************************************************************/



/*============================================================================
 * update - Update wanpipe device status & statistics
 *
 * @wandev:	Wanpipe device pointer
 *
 * This procedure is called when updating the PROC file system.
 * It returns various communications statistics.
 *
 * cat /proc/net/wanrouter/wanpipe#  (where #=1,2,3...)
 *
 * These statistics are accumulated from 3
 * different locations:
 * 	1) The 'if_stats' recorded for the device.
 * 	2) Communication error statistics on the adapter.
 *      3) Operational statistics on the adapter.
 *
 * The board level statistics are read during a timer interrupt.
 * Note that we read the error and operational statistics
 * during consecitive timer ticks so as to minimize the time
 * that we are inside the interrupt handler.
 *
 */
static int update (wan_device_t* wandev)
{
	sdla_t* card = wandev->private;
 	netdevice_t* dev;
        volatile private_area_t* chan;

	/* sanity checks */
	if((wandev == NULL) || (wandev->private == NULL))
		return -EFAULT;

	if(wandev->state == WAN_UNCONFIGURED)
		return -ENODEV;

	if(wan_test_bit(PERI_CRIT, (void*)&card->wandev.critical))
                return -EAGAIN;

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if(dev == NULL)
		return -ENODEV;

	if((chan=wan_netif_priv(dev)) == NULL)
		return -ENODEV;

       	if(card->update_comms_stats){
		return -EAGAIN;
	}

	DEBUG_TEST("%s: Chain Dma Status=0x%lX, TxCur=%d, TxPend=%d RxCur=%d RxPend=%d\n",
			chan->if_name, 
			chan->dma_chain_status,
			chan->tx_chain_indx,
			chan->tx_pending_chain_indx,
			chan->rx_chain_indx,
			chan->rx_pending_chain_indx);

#if 1
	update_comms_stats(card);
#else
	#warning "COMM STATS DISABLED"
#endif
	return 0;
}



/*============================================================================
 * new_if - Create new logical channel.
 *
 * &wandev: 	Wanpipe device pointer
 * &dev:	Network device pointer
 * &conf:	User configuration options pointer
 *
 * This routine is called by the ROUTER_IFNEW ioctl,
 * in wanmain.c.  The ioctl passes us the user configuration
 * options which we use to configure the driver and
 * firmware.
 *
 * This functions main purpose is to allocate the
 * private structure for protocol and bind it
 * to dev->priv pointer.
 *
 * Also the dev->init pointer should also be initialized
 * to the if_init() function.
 *
 * Any allocation necessary for the private strucutre
 * should be done here, as well as proc/ file initializetion
 * for the network interface.
 *
 * o parse media- and hardware-specific configuration
 * o make sure that a new channel can be created
 * o allocate resources, if necessary
 * o prepare network device structure for registaration.
 * o add network interface to the /proc/net/wanrouter
 *
 * The opposite of this function is del_if()
 *
 * Return:	0	o.k.
 *		< 0	failure (channel will not be created)
 */
 
static int
aft_tdm_api_init(sdla_t *card, private_area_t *chan, wanif_conf_t *conf)
{
#ifdef AFT_TDM_API_SUPPORT
	int err=0;
#endif
	
	if (chan->common.usedby != TDM_VOICE_API &&
	    chan->common.usedby != TDM_VOICE_DCHAN) {
	    	return 0;
	}
	
#ifdef AFT_TDM_API_SUPPORT
	if (chan->tdmv_zaptel_cfg) {
		return 0;
	}
	
	/* Initilaize TDM API Parameters */
	chan->wp_tdm_api_dev.chan = chan;
	chan->wp_tdm_api_dev.card = card;
	wan_spin_lock_init(&chan->wp_tdm_api_dev.lock);
	strncpy(chan->wp_tdm_api_dev.name,chan->if_name,WAN_IFNAME_SZ);

	if (conf->hdlc_streaming) {
        	chan->wp_tdm_api_dev.hdlc_framing=1;
	}
	
	chan->wp_tdm_api_dev.event_ctrl		= aft_event_ctrl;
	chan->wp_tdm_api_dev.read_rbs_bits	= aft_read_rbs_bits;
	chan->wp_tdm_api_dev.write_rbs_bits	= aft_write_rbs_bits;
	chan->wp_tdm_api_dev.write_hdlc_frame	= aft_write_hdlc_frame;
	
	chan->wp_tdm_api_dev.cfg.rx_disable = 0;
	chan->wp_tdm_api_dev.cfg.tx_disable = 0;
 
        if (IS_TE1_CARD(card)) {
		if (IS_T1_CARD(card)){
			chan->wp_tdm_api_dev.cfg.hw_tdm_coding=WP_MULAW;	
			chan->wp_tdm_api_dev.tdm_chan = chan->first_time_slot+1;
		}else{
			chan->wp_tdm_api_dev.cfg.hw_tdm_coding=WP_ALAW;
			chan->wp_tdm_api_dev.tdm_chan = chan->first_time_slot;
		}
	} else {
        	if (card->fe.fe_cfg.tdmv_law == WAN_TDMV_MULAW){ 
		       	chan->wp_tdm_api_dev.cfg.hw_tdm_coding=WP_MULAW;	
			chan->wp_tdm_api_dev.tdm_chan = chan->first_time_slot+1; 
		} else {
                        chan->wp_tdm_api_dev.cfg.hw_tdm_coding=WP_ALAW;	
			chan->wp_tdm_api_dev.tdm_chan = chan->first_time_slot+1;
		}	
	}
	
	if (IS_T1_CARD(card) || IS_FXOFXS_CARD(card)){
		/* Convert active_ch bit map to user */
		chan->wp_tdm_api_dev.active_ch = conf->active_ch << 1;
	}else{
		chan->wp_tdm_api_dev.active_ch = conf->active_ch;
	}

	chan->wp_tdm_api_dev.cfg.idle_flag = conf->u.aft.idle_flag;
      	chan->wp_tdm_api_dev.cfg.rbs_tx_bits = conf->u.aft.rbs_cas_idle;
	
    	chan->wp_tdm_api_dev.tdm_span = card->tdmv_conf.span_no;
	
	err=wanpipe_tdm_api_reg(&chan->wp_tdm_api_dev);
	if (err){
		return err;
	}

	wan_set_bit(0,&chan->wp_tdm_api_dev.init);
	return err;
#else
	DEBUG_EVENT("%s: TDM API support not compiled in\n",
			card->devname);
      	return -EINVAL;
#endif
	
}

static int aft_tdm_api_free(sdla_t *card, private_area_t *chan)
{
#ifdef AFT_TDM_API_SUPPORT 
	int err=0;
	if (wan_test_bit(0,&chan->wp_tdm_api_dev.init)){
		wan_clear_bit(0,&chan->wp_tdm_api_dev.init);
		err=wanpipe_tdm_api_unreg(&chan->wp_tdm_api_dev);
		if (err){
			wan_set_bit(0,&chan->wp_tdm_api_dev.init);
			return err;
		}
	}
#endif
	return 0;
}



static int aft_chan_if_init(sdla_t *card, netdevice_t *dev, private_area_t *chan)
{
	chan->first_time_slot=-1;
	chan->last_time_slot=-1;
	chan->logic_ch_num=-1;
#if defined(AFT_SINGLE_DMA_CHAIN)
	chan->single_dma_chain=1;
	chan->max_tx_bufs=MAX_AFT_DMA_CHAINS;
#else
	chan->single_dma_chain=0;
	chan->max_tx_bufs=MAX_TX_BUF;
#endif
	chan->tslot_sync=0;

	strncpy(chan->if_name, wan_netif_name(dev), WAN_IFNAME_SZ);

	chan->card = card;
	chan->common.card = card;

	WAN_IFQ_INIT(&chan->wp_tx_pending_list,0);
	WAN_IFQ_INIT(&chan->wp_tx_complete_list,0);
	
	WAN_IFQ_INIT(&chan->wp_rx_free_list,0);
	WAN_IFQ_INIT(&chan->wp_rx_complete_list,0);

	WAN_IFQ_INIT(&chan->wp_rx_stack_complete_list, 0);

	wan_trace_info_init(&chan->trace_info,MAX_TRACE_QUEUE);

	/* Initiaize Tx/Rx DMA Chains */
	aft_index_tx_rx_dma_chains(chan);
	
	/* Initialize the socket binding information
	 * These hooks are used by the API sockets to
	 * bind into the network interface */
	WAN_TASKLET_INIT((&chan->common.bh_task),0,wp_bh,chan);
	chan->common.dev = dev;
	chan->tracing_enabled = 0;

	return 0;
}



static int aft_ss7_if_init(sdla_t *card, private_area_t *chan, wanif_conf_t *conf)
{
	if (chan->common.usedby != API){
		/* SS7 Support not supported in non API mode */
		chan->cfg.ss7_enable = 0;
		return 0;
	}
		
	DEBUG_EVENT("%s:    SS7 Support   :%s\n",
				card->devname,
				chan->cfg.ss7_enable?"On":"Off");
	
	if (chan->cfg.ss7_enable){
		
		wan_smp_flag_t smp_flags;
		u32 lcfg_reg;
		
		DEBUG_EVENT("%s:    SS7 Mode      :%s\n",
					card->devname,
					chan->cfg.ss7_mode?"4096":"128");
		
		DEBUG_EVENT("%s:    SS7 LSSU Size :%d\n",
					card->devname,
					chan->cfg.ss7_lssu_size);

		wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
		card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), 
					  &lcfg_reg);
		if (chan->cfg.ss7_mode){
			aft_lcfg_ss7_mode4096_cfg(&lcfg_reg,chan->cfg.ss7_lssu_size);
		}else{
			aft_lcfg_ss7_mode128_cfg(&lcfg_reg,chan->cfg.ss7_lssu_size);
		}
		card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), 
					   lcfg_reg);
		card->u.aft.lcfg_reg=lcfg_reg;
		wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);

		aft_hwdev[card->wandev.card_type].aft_fifo_adjust(card,AFT_TDMV_FIFO_LEVEL);
		chan->single_dma_chain=1;
	}

	return 0;
}

static int aft_transp_if_init(sdla_t *card, private_area_t *chan, wanif_conf_t *conf)
{
	unsigned char *buf;

	if (chan->mtu&0x03){
		DEBUG_EVENT("%s:%s: Error, Transparent MTU must be word aligned!\n",
				card->devname,chan->if_name);
		return -EINVAL;
	}

	chan->max_idle_size=chan->mtu;

	if (chan->tslot_sync && chan->mtu%chan->num_of_time_slots){
		DEBUG_EVENT("%s:%s: Error, Sync Transparent MTU must be timeslot aligned!\n",
				card->devname,chan->if_name);

		DEBUG_EVENT("%s:%s: Error, MTU=%d not multiple of %d timeslots!\n",
				card->devname,chan->if_name,
				chan->mtu,chan->num_of_time_slots);

		return -EINVAL;
	}

	if (conf->protocol != WANCONFIG_LIP_ATM &&
	    conf->protocol != WANCONFIG_LIP_KATM &&           
	    chan->mru%chan->num_of_time_slots){
		DEBUG_EVENT("%s:%s: Error, Transparent MRU must be timeslot aligned!\n",
				card->devname,chan->if_name);

		DEBUG_EVENT("%s:%s: Error, MRU=%d not multiple of %d timeslots!\n",
				card->devname,chan->if_name,
				chan->mru,chan->num_of_time_slots);

		return -EINVAL;
	}


	DEBUG_TEST("%s:%s: Config for Transparent mode: Idle=%X Len=%u\n",
		card->devname,chan->if_name,
		chan->idle_flag,chan->max_idle_size);

	chan->idle_flag=0x7E;     

	if (chan->tdmv_zaptel_cfg){
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		chan->idle_flag = WAN_TDMV_IDLE_FLAG;
#endif
	}

	/* We use the dma_mru value here, just because it will
         * be easier to change the idle tx size on the fly */
	chan->tx_idle_skb = wan_skb_alloc(chan->dma_mru); 
	if (!chan->tx_idle_skb){
		return -EINVAL;
	}
	buf = wan_skb_put(chan->tx_idle_skb,chan->dma_mru);

	if(conf->protocol != WANCONFIG_LIP_ATM &&
	   conf->protocol != WANCONFIG_LIP_KATM){
		memset(buf,chan->idle_flag,chan->dma_mru);
	}else{
		chan->lip_atm = 1;	
		/* if running below LIP ATM, transmit idle cells */
		if(init_atm_idle_buffer((unsigned char*)buf,
				wan_skb_len(chan->tx_idle_skb),
				chan->if_name,
				chan->cfg.data_mux )){

			wan_skb_free(chan->tx_idle_skb);
			chan->tx_idle_skb = NULL;
			return -EINVAL;
		}
	}

	/* reset the tx idle buffer to the actual mtu size */
	wan_skb_init(chan->tx_idle_skb,16);
	wan_skb_trim(chan->tx_idle_skb,0);
	wan_skb_put(chan->tx_idle_skb,chan->max_idle_size);

	return 0;
}


static int new_if_private (wan_device_t* wandev, netdevice_t* dev, wanif_conf_t* conf, int channelized, int dchan)
{
	sdla_t* card = wandev->private;
	private_area_t* chan;
	int dma_per_ch=card->u.aft.cfg.dma_per_ch;
	int err = 0;

	DEBUG_EVENT( "%s: Configuring Interface: %s\n",
			card->devname, wan_netif_name(dev));

	if ((conf->name[0] == '\0') || (strlen(conf->name) > WAN_IFNAME_SZ)){
		DEBUG_EVENT( "%s: Invalid interface name!\n",
			card->devname);
		return -EINVAL;
	}


	if (card->adptr_subtype != AFT_SUBTYPE_SHARK){
		if (card->u.aft.security_id != 0x01  && 
		    card->u.aft.security_cnt >= 2){
			DEBUG_EVENT("%s: Error: Security: Max HDLC channels(2) exceeded!\n",
					card->devname);
			DEBUG_EVENT("%s: Un-Channelised AFT supports 2 HDLC ifaces!\n",
					card->devname);
			return -EINVAL;
		}
	}

	/* ======================================
	 * Allocate and initialize private data 
	 * =====================================*/
	
	chan = wan_kmalloc(sizeof(private_area_t));
	if(chan == NULL){
		WAN_MEM_ASSERT(card->devname);
		return -ENOMEM;
	}
	
	memset(chan, 0, sizeof(private_area_t));
	memcpy(&chan->cfg,&conf->u.aft,sizeof(chan->cfg));


	chan->true_if_encoding=conf->true_if_encoding;

	aft_chan_if_init(card,dev,chan);

	if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
		chan->single_dma_chain = 1;
		conf->hdlc_streaming=0;
	}

	if(IS_56K_CARD(card)){
		chan->single_dma_chain = 1;
		conf->hdlc_streaming=1;
	}

	if (channelized){ 
		chan->channelized_cfg=1;
		if (wan_netif_priv(dev)){ 
#if 1
			private_area_t *cptr;
			for (cptr=wan_netif_priv(dev);cptr->next!=NULL;cptr=cptr->next);
			cptr->next=chan;
			chan->next=NULL;
#else
#warning "DEBUG: Chan list backwards!"
			chan->next = wan_netif_priv(dev);
			wan_netif_set_priv(dev, chan);
#endif
		}else{
			wan_netif_set_priv(dev, chan);
		}
	}else{
		chan->channelized_cfg=0;
		wan_netif_set_priv(dev, chan);
	}

	/* ======================================
	 * Configure chan MTU and MRU Values
	 * And setup E1 timeslots
	 * =====================================*/	
	chan->mtu = card->wandev.mtu;
	if (conf->u.aft.mtu){
		chan->mtu=conf->u.aft.mtu;
		if (chan->mtu > MAX_WP_PRI_MTU ||
	    	    chan->mtu < MIN_WP_PRI_MTU){
			DEBUG_EVENT("%s: Error Invalid %s MTU %d (Min=%d, Max=%d)\n",
				card->devname,chan->if_name,chan->mtu,
				MIN_WP_PRI_MTU,MAX_WP_PRI_MTU);

			err= -EINVAL;
			goto new_if_error;
		}

	}

	chan->mru = card->u.aft.cfg.mru;
	if (conf->u.aft.mru){
		chan->mru = conf->u.aft.mru;
		if (chan->mru > MAX_WP_PRI_MTU ||
	    	    chan->mru < MIN_WP_PRI_MTU){
			DEBUG_EVENT("%s: Error Invalid %s MRU %d (Min=%d, Max=%d)\n",
				card->devname,chan->if_name,chan->mru,
				MIN_WP_PRI_MTU,MAX_WP_PRI_MTU);

			err= -EINVAL;
			goto new_if_error;
		}
	}

	/*====================================================
	 * Interface connects top services to driver
	 * Interface can be used by the following services:
	 *    WANPIPE 	  	= TCP/IP             -> Driver
	 *    API     	  	= Raw Socket Access  -> Driver
	 *    BRIDGE  	  	= Bridge to Ethernet -> Driver
	 *    BRIDGE_NODE 	= TCP/IP to Ethernet -> Driver
	 *    STACK	  	= LIP        -> Driver
	 *    TDM_VOICE  	= Zaptel     -> Trans Ch Driver
	 *    TDM_VOICE_DCHAN   = Zaptel     -> Hdlc Driver (PRIVATE)
	 *    TDM_VOICE_API	= Raw Socket -> Transp Ch Driver
	 *    TMD_API           = Raw Socket -> Transp Channelized API
	 *===================================================*/
	
	if(strcmp(conf->usedby, "WANPIPE") == 0) {

		DEBUG_EVENT( "%s: Running in WANPIPE mode\n",
			wandev->name);
		chan->common.usedby = WANPIPE;

		/* Used by GENERIC driver only otherwise protocols
		 * are in LIP layer */
		if (conf->protocol != WANOPT_NO){
			wan_netif_set_priv(dev, chan);
			if ((err=protocol_init(card,dev,chan,conf)) != 0){
				wan_netif_set_priv(dev, NULL);
				goto new_if_error;
			}
		}

#if defined(__LINUX__)

	} else if( strcmp(conf->usedby, "TDM_API") == 0) {

		DEBUG_EVENT("%s:%s: Error: TDM API mode is not supported!\n",
				card->devname,chan->if_name);
		err=-EINVAL;
		goto new_if_error;
#endif

#if defined(AFT_API_SUPPORT)
	} else if( strcmp(conf->usedby, "API") == 0) {
		chan->common.usedby = API;
		DEBUG_EVENT( "%s:%s: Running in API mode\n",
			wandev->name,chan->if_name);
		wan_reg_api(chan, dev, card->devname);
		card->wandev.event_callback.dtmf	= wan_aft_api_dtmf; 
		card->wandev.event_callback.hook	= wan_aft_api_hook; 
		card->wandev.event_callback.ringtrip	= wan_aft_api_ringtrip; 
		card->wandev.event_callback.ringdetect	= wan_aft_api_ringdetect; 
#endif

#if defined(__LINUX__)
	}else if (strcmp(conf->usedby, "BRIDGE") == 0) {
		chan->common.usedby = BRIDGE;
		DEBUG_EVENT( "%s:%s: Running in WANPIPE (BRIDGE) mode.\n",
				card->devname,chan->if_name);
#endif

#if defined(__LINUX__)
	}else if (strcmp(conf->usedby, "BRIDGE_N") == 0) {
		chan->common.usedby = BRIDGE_NODE;
		DEBUG_EVENT( "%s:%s: Running in WANPIPE (BRIDGE_NODE) mode.\n",
				card->devname,chan->if_name);
#endif

	}else if (strcmp(conf->usedby, "TDM_VOICE") == 0) {

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
		chan->common.usedby = TDM_VOICE;

		chan->tdmv_zaptel_cfg=1;
		card->u.aft.tdmv_zaptel_cfg=1;
		conf->hdlc_streaming=0;
		chan->single_dma_chain=1;
		chan->max_tx_bufs=MAX_AFT_DMA_CHAINS;
		
		if (dchan >= 0){
# ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN
		       	chan->common.usedby = TDM_VOICE_DCHAN;
		       	conf->hdlc_streaming=1;
		       	chan->single_dma_chain=0;
		       	chan->mru=chan->mtu=1500;
# else
			DEBUG_EVENT("%s: Error: TDMV_DCHAN Option not compiled into the driver!\n",
					card->devname);
			err=-EINVAL;
			goto new_if_error;
# endif
		}

		DEBUG_EVENT( "%s:%s: Running in TDM %sVoice Zaptel Mode.\n",
				card->devname,chan->if_name,
				chan->common.usedby == TDM_VOICE_DCHAN?"DCHAN ":"");
#else
		DEBUG_EVENT("\n");
		DEBUG_EVENT("%s:%s: Error: TDM VOICE prot not compiled\n",
				card->devname,chan->if_name);
		DEBUG_EVENT("%s:%s:        during installation process!\n",
				card->devname,chan->if_name);
		err=-EINVAL;
		goto new_if_error;
#endif

#if defined(__LINUX__)
	}else if (strcmp(conf->usedby, "TDM_VOICE_API") == 0) {
	
		chan->common.usedby = TDM_VOICE_API;
		chan->cfg.data_mux=1;
		conf->hdlc_streaming=0;
		chan->single_dma_chain=1;
		chan->tdmv_zaptel_cfg=0;
		
		if (dchan >= 0){
		       	chan->common.usedby = TDM_VOICE_DCHAN;
		        chan->cfg.data_mux=0;
		       	chan->single_dma_chain=0;
		       	conf->hdlc_streaming=1;
		       	chan->mru=chan->mtu=1500;
		}


		if (chan->common.usedby == TDM_VOICE_DCHAN) {
			DEBUG_EVENT( "%s:%s: Running in TDM Voice DCHAN API Mode.\n",
				card->devname,chan->if_name);
		} else {
			DEBUG_EVENT( "%s:%s: Running in TDM Voice API Mode.\n",
				card->devname,chan->if_name);
		}
#endif		

	}else if (strcmp(conf->usedby, "STACK") == 0) {
		chan->common.usedby = STACK;
		DEBUG_EVENT( "%s:%s: Running in Stack mode.\n",
				card->devname,chan->if_name);

	}else{
		DEBUG_EVENT( "%s:%s: Error: Invalid IF operation mode %s\n",
				card->devname,chan->if_name,conf->usedby);
		err=-EINVAL;
		goto new_if_error;
	}


	/*===============================================
	 * Interface Operation Setup
	 *==============================================*/

	if (conf->hwec.enable){
		card->wandev.ec_enable_map |= conf->active_ch;
	}

	/* Read user specified active_ch, we must do it
	 * here because the active_ch value might change
	 * for different user modes*/
	chan->time_slot_map=conf->active_ch;
	chan->num_of_time_slots=
		aft_get_num_of_slots(card->u.aft.num_of_time_slots,
			             chan->time_slot_map);

	if (!chan->num_of_time_slots){
		DEBUG_EVENT("%s: Error: Invalid number of timeslots in map 0x%08lX!\n",
				chan->if_name,chan->time_slot_map);
		return -EINVAL;
	}

	if (card->wandev.config_id == WANCONFIG_AFT_ANALOG && chan->num_of_time_slots > 1) {
		DEBUG_EVENT(
		"%s: Error: Invalid Analog number of timeslots in map 0x%08lX: (Valid=1)\n",
				chan->if_name,chan->time_slot_map);
		return -EINVAL;
	}	

	
	/* =====================
	 * Interaface TDMV Setup
	 * 
	 * Initialize the interface for TDMV 
	 * operation, if TDMV is not used this
	 * function will just return */
	err=aft_tdmv_if_init(card,chan,conf);
	if (err){
		err=-EINVAL;
		goto new_if_error;
	}


	/* =====================
	 * Interaface SS7 Setup
	 * 
	 * Initialize the interface for TDMV 
	 * operation, if TDMV is not used this
	 * function will just return */
	err=aft_ss7_if_init(card,chan,conf);
	if (err){
		err=-EINVAL;
		goto new_if_error;
	}

	
	/* Print out the current configuration */
	DEBUG_EVENT("%s:    MRU           :%d\n",
			card->devname,
			chan->mru);

	DEBUG_EVENT("%s:    MTU           :%d\n",
			card->devname,
			chan->mtu);


	chan->hdlc_eng = conf->hdlc_streaming;

	DEBUG_EVENT("%s:    HDLC Eng      :%s\n",
			card->devname,
			chan->hdlc_eng?"On":"Off (Transparent)");


	/* Obtain the DMA MRU size based on user confgured
	 * MRU.  The DMA MRU must be of size 2^x */
		
       	chan->dma_mru = chan->mtu;

#if defined(__LINUX__)
	chan->dma_mru = aft_valid_mtu(chan->dma_mru);
#else
	chan->dma_mru = aft_valid_mtu(chan->dma_mru);
#endif
	if (!chan->dma_mru){
		DEBUG_EVENT("%s:%s: Error invalid MTU %d  MRU %d\n",
			card->devname,
			chan->if_name,
			chan->mtu,chan->mru);
		err= -EINVAL;
		goto new_if_error;
	}

    	if (conf->single_tx_buf || 
	    ((card->adptr_type == A101_ADPTR_2TE1 || card->adptr_type == A101_ADPTR_1TE1) && 
	     card->u.aft.firm_id == AFT_DS_FE_CORE_ID)){
       		chan->single_dma_chain=1;
		chan->max_tx_bufs=MAX_AFT_DMA_CHAINS;  
        	dma_per_ch=MAX_AFT_DMA_CHAINS;
	}
	

	if (!chan->hdlc_eng){
		/* If hardware HDLC engine is disabled:
		 *  1. Configure DMA chains for SINGLE DMA
		 *  2. Enable Timeslot Synchronization 
		 *  3. Configure Interface for Transparent Operation */
		chan->single_dma_chain=1;
		chan->max_tx_bufs=MAX_AFT_DMA_CHAINS;
		
		if (chan->channelized_cfg) {
                        dma_per_ch=MAX_AFT_DMA_CHAINS;
		}else{
			dma_per_ch= (MAX_AFT_DMA_CHAINS*1500) / chan->mtu;
			if (dma_per_ch <  MAX_AFT_DMA_CHAINS) {
                		dma_per_ch=MAX_AFT_DMA_CHAINS; 	
	     		}
		}

		chan->tslot_sync=1;
		
		if(conf->protocol == WANCONFIG_LIP_ATM ||
		   conf->protocol == WANCONFIG_LIP_KATM){
			chan->tslot_sync=0;
		}

		err=aft_transp_if_init(card,chan,conf);
		if (err){
			goto new_if_error;
		}
		
	}else{
		/* If hardware HDLC engine is enabled:
		 *  1. Force Disable DATA MUX option 
		 *     just in case user made a mistake
		 */
		chan->cfg.data_mux=0;
	}
	DEBUG_EVENT("%s:    Data Mux Ctrl :%s\n",
			card->devname,
			chan->cfg.data_mux?"On":"Off");


	

	/*=================================================
	 * AFT CHANNEL Configuration
	 *
	 * Configure the AFT Hardware for this 
	 * logic channel.  Enable the above selected
	 * operation modes.
	 *================================================*/

	err=aft_dev_configure(card,chan,conf);
	if (err){
		goto new_if_error;
	}

	/*Set the actual logic ch number of this chan
     	 *as the dchan. Due to HDLC security issue, the
	 *HDLC channels are mapped on first TWO logic channels */
	if (chan->common.usedby == TDM_VOICE_DCHAN){
     		card->u.aft.tdmv_dchan=chan->logic_ch_num+1;
	}

	/* Configure the DCHAN on LAST Master interface.
         * We will use the master interface information, until
         * the next interface with the current DCHAN info is 
         * configured.  This must be done in order to register
         * the DCHAN in zaptel.  */
	if (card->u.aft.tdmv_dchan_cfg_on_master &&
            card->u.aft.tdmv_dchan){ 
		int dchan=card->u.aft.tdmv_dchan;
		if (IS_T1_CARD(card)){
			dchan--;
		}
		if (wan_test_bit(dchan,&conf->active_ch)){
			DEBUG_EVENT("%s:    TDMV DCHAN    :%d\n",
					card->devname,dchan); 
			card->u.aft.tdmv_chan_ptr=chan;
			card->u.aft.tdmv_dchan=chan->logic_ch_num+1;
		}
	}

      
	err=aft_tdm_api_init(card,chan,conf);
	if (err){
		goto new_if_error;
	}	   

	err=aft_hwec_config(card,chan,conf,1);
	if (err){
		goto new_if_error;
	}

       	if (chan->channelized_cfg && !chan->hdlc_eng) {
       	 	chan->dma_mru = 1024;
		dma_per_ch = 4;
	}

	DEBUG_EVENT("%s:    DMA/Len/Chain/EC :%d/%d/%s/%s\n",
			card->devname,
			dma_per_ch,
			chan->dma_mru,
			chan->single_dma_chain?"Off":"On",
			card->wandev.ec_enable_map?"On":"Off");

          
	
	err=aft_alloc_rx_dma_buff(card, chan, dma_per_ch,0);
	if (err){
		goto new_if_error;
	}

	/*=======================================================
	 * Interface OS Specific Configuration
	 *======================================================*/

	
	/* If gateway option is set, then this interface is the
	 * default gateway on this system. We must know that information
	 * in case DYNAMIC interface configuration is enabled.
	 *
	 * I.E. If the interface is brought down by the driver, the
	 *      default route will also be removed.  Once the interface
	 *      is brought back up, we must know to re-astablish the
	 *      default route.
	 */
	if ((chan->gateway = conf->gateway) == WANOPT_YES){
		DEBUG_EVENT( "%s: Interface %s is set as a gateway.\n",
			card->devname,chan->if_name);
	}

	/* Get Multicast Information from the user 
	 * FIXME: This is IP relevant, since this is now
	 *        a hardware interface this option should't 
	 *        be here */
	chan->mc = conf->mc;


	/* The network interface "dev" has been passed as
	 * an argument from the above layer. We must initialize
	 * it so it can be registered into the kernel.
	 *
	 * The "dev" structure is the link between the kernel
	 * stack and the wanpipe driver.  It contains all
	 * access hooks that kernel uses to communicate to
	 * the our driver.
	 *
	 * For now, just set the "dev" name to the user
	 * defined name and initialize:
	 * 	dev->if_init : function that will be called
	 * 	               to further initialize
	 * 	               dev structure on "ifconfig up"
	 *
	 * 	dev->priv    : private structure allocated above
	 *
	 */

	
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 * 
	 * DO NOT PLACE ANY CODE BELOW THAT COULD RETURN ERROR 
	 *
	 *!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


	/* Only setup the dev pointer once the new_if function has
	 * finished successfully.  DO NOT place any code below that
	 * can return an error */
#if defined(__LINUX__)
	dev->init = &if_init;
# if defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
	if_init(dev);
# endif
#else
	chan->common.is_netdev = 1;
	chan->common.iface.open      = &if_open;
        chan->common.iface.close     = &if_close;
        chan->common.iface.output    = &if_send;
        chan->common.iface.ioctl     = &if_do_ioctl;
        chan->common.iface.get_stats = &if_stats;
        chan->common.iface.tx_timeout= &if_tx_timeout;
	if (wan_iface.attach){
		if (!ifunit(wan_netif_name(dev))){
			wan_iface.attach(dev, NULL, chan->common.is_netdev);
		}
	}else{
		DEBUG_EVENT("%s: Failed to attach interface %s!\n",
				card->devname, wan_netif_name(dev));
		wan_netif_set_priv(dev, NULL);
		err = -EINVAL;
		goto new_if_error;
	}
	wan_netif_set_mtu(dev, chan->mtu);
#endif

	/* 
	 * Increment the number of network interfaces
	 * configured on this card.
	 */
	wan_atomic_inc(&card->wandev.if_cnt);
	if (chan->hdlc_eng){
		++card->u.aft.security_cnt;
	}



	/* Keep the original tx queue len in case 
	   we have to go back to it */
	chan->max_tx_bufs_orig = chan->max_tx_bufs;

	chan->common.state = WAN_CONNECTING;

	DEBUG_EVENT( "\n");

	return 0;

new_if_error:

	return err;
}

static int new_if (wan_device_t* wandev, netdevice_t* dev, wanif_conf_t* conf)
{
	int err=-EINVAL;
	sdla_t *card=wandev->private;

        wan_netif_set_priv(dev, NULL); 

	if (IS_E1_CARD(card) && !(WAN_FE_FRAME(&card->fe) == WAN_FR_UNFRAMED)) {
		conf->active_ch = conf->active_ch << 1;
		wan_clear_bit(0,&conf->active_ch);
	}else if (IS_56K_CARD(card)) {
		conf->active_ch = 1;
	}


	if (strcmp(conf->usedby, "TDM_VOICE") == 0) {
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE	
		if (card->tdmv_conf.span_no){

			switch(card->wandev.config_id){
			case WANCONFIG_AFT_ANALOG:
				err = wp_tdmv_remora_init(&card->tdmv_iface);
				break;
			default:
				err = wp_tdmv_te1_init(&card->tdmv_iface);
				break;
			}
			if (err){
				DEBUG_EVENT("%s: Error: Failed to initialize tdmv functions!\n",
						card->devname);
				return -EINVAL;
			}

			WAN_TDMV_CALL(create, (card, &card->tdmv_conf), err);
			if (err){
				DEBUG_EVENT("%s: Error: Failed to create tdmv span!\n",
						card->devname);
				return err;
			}
		}
#else
		DEBUG_EVENT("\n");
		DEBUG_EVENT("%s: Error: TDM VOICE prot not compiled\n",
				card->devname);
		DEBUG_EVENT("%s:        during installation process!\n",
				card->devname);
		return -EINVAL;
#endif                 
	}

	err=-EINVAL;
	
	if (strcmp(conf->usedby, "TDM_VOICE") == 0 ||
	    strcmp(conf->usedby, "TDM_VOICE_API") == 0){

		int i=0,master_if=-1;
		u32 active_ch=conf->active_ch;            

		if (card->wandev.fe_iface.active_map){
			conf->active_ch = card->wandev.fe_iface.active_map(&card->fe);
			active_ch=conf->active_ch;
		}

	       	err=aft_find_master_if_and_dchan(card,&master_if,active_ch);
	       	if (err < 0) {
                       	return err;
	       	}

		DEBUG_TEST("%s: TDM VOICE: TESTING FOR DCHAN CHAN in 0x%08X Timeslots=%i CFG DCHAN=0x%08X MasterIF=%i\n",
				card->devname, active_ch, card->u.aft.num_of_time_slots,
				card->tdmv_conf.dchan,master_if);

		
		for (i=0;i<card->u.aft.num_of_time_slots;i++){
			if (wan_test_bit(i,&active_ch)){
				int dchan=-1;
				conf->active_ch=0;
				conf->u.aft.tdmv_master_if=0;
				wan_set_bit(i,&conf->active_ch);
				if (wan_test_bit(i,&card->tdmv_conf.dchan)){
                                    	dchan=i;    
				}
				if (i==master_if){
					conf->u.aft.tdmv_master_if=1;
				}

				err=new_if_private(wandev,dev,conf,1,dchan);
				if (err){
					break;
				}
			}
		}		

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		if (!err && card->u.aft.tdmv_zaptel_cfg){
			WAN_TDMV_CALL(software_init, (&card->wan_tdmv), err);
		}
#endif
		
	}else{
		card->tdmv_conf.dchan=0;
		err=new_if_private(wandev,dev,conf,0,-1);
	}

	
	if (err == 0 && wan_netif_priv(dev)) {
		wan_smp_flag_t flags;
	
	       /* If FRONT End is down, it means that the DMA
		* is disabled.  In this case don't try to
		* reset fifo.  Let the enable_data_error_intr()
		* function do this, after front end has come up */
	
		wan_spin_lock_irq(&card->wandev.lock,&flags);
	
		aft_dev_open(card, wan_netif_priv(dev));
	
		if (card->wandev.state == WAN_CONNECTED){
			set_chan_state(card, dev, WAN_CONNECTED);
		}
		wan_spin_unlock_irq(&card->wandev.lock,&flags);

		if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
			wan_spin_lock_irq(&card->wandev.lock,&flags);
			card->fe.fe_status = FE_CONNECTED;
			handle_front_end_state(card);
			set_chan_state(card, dev, WAN_CONNECTED);
			wan_spin_unlock_irq(&card->wandev.lock,&flags);
		}
	
	} else if (err && wan_netif_priv(dev)){
		del_if(wandev,dev);
		if (wan_netif_priv(dev)){
			wan_free(wan_netif_priv(dev));
			wan_netif_set_priv(dev, NULL);
		}
	}
	
	return err;
}


/*============================================================================
 * del_if - Delete logical channel.
 *
 * @wandev: 	Wanpipe private device pointer
 * @dev:	Netowrk interface pointer
 *
 * This function is called by ROUTER_DELIF ioctl call
 * to deallocate the network interface.
 *
 * The network interface and the private structure are
 * about to be deallocated by the upper layer.
 * We have to clean and deallocate any allocated memory.
 *
 * NOTE: DO NOT deallocate dev->priv here! It will be
 *       done by the upper layer.
 *
 */
static int del_if_private (wan_device_t* wandev, netdevice_t* dev)
{
	private_area_t* 	chan = wan_netif_priv(dev);
	sdla_t*			card;
	netskb_t 		*skb;
	wan_smp_flag_t 		flags;

	if (!chan){
		DEBUG_EVENT("%s: Critical Error del_if_private() chan=NULL!\n",
			  wan_netif_name(dev));	
		return 0;
	}

	card = chan->card;
	if (!card){
		DEBUG_EVENT("%s: Critical Error del_if_private() chan=NULL!\n",
			  wan_netif_name(dev));
		return 0;
	}

	aft_hwec_config(card,chan,NULL,0);

        wan_spin_lock_irq(&card->wandev.lock,&flags);
	aft_dev_unconfigure(card,chan);
        wan_spin_unlock_irq(&card->wandev.lock,&flags);

	AFT_FUNC_DEBUG();
#if INIT_FE_ONLY
	return 0;
#endif

	WAN_TASKLET_KILL(&chan->common.bh_task);
	
	if (chan->common.usedby == API){
		wan_unreg_api(chan, card->devname);
		
	}

	if (aft_tdm_api_free(card,chan)) {
		DEBUG_EVENT("%s: Error: Failed to del iface: TDM API Device in use!\n",
				chan->if_name);
		return -EBUSY;
	}
	
	wan_spin_lock_irq(&card->wandev.lock,&flags);
	aft_tdmv_if_free(card,chan);
	wan_spin_unlock_irq(&card->wandev.lock,&flags);

	protocol_shutdown(card,dev);

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	if (wan_iface.detach){
		wan_iface.detach(dev, chan->common.is_netdev);
	}
#endif

	/* We must set used by to API because
	 * the free_tx and free_rx are not allowed
	 * for TDM_VOICE mode in regular operation */


        wan_spin_lock_irq(&card->wandev.lock,&flags);

	chan->common.usedby = API;

	aft_free_tx_descriptors(chan);
	aft_free_rx_descriptors(chan);
	
	while ((skb=wan_skb_dequeue(&chan->wp_rx_free_list)) != NULL) {
#ifdef __LINUX__
        	if (skb_shinfo(skb)->frag_list ||
	            skb_shinfo(skb)->nr_frags) {
	        	DEBUG_EVENT("%s: Warning: SKB Corruption ch=%li!\n",
		                   chan->if_name,chan->logic_ch_num);
	        	continue;
		}
#endif
		wan_skb_free(skb);
	}

	while ((skb=wan_skb_dequeue(&chan->wp_rx_complete_list)) != NULL){
		wan_skb_free(skb);
	}
	
	while ((skb=wan_skb_dequeue(&chan->wp_rx_stack_complete_list)) != NULL){
		wan_skb_free(skb);
	}

	while ((skb=wan_skb_dequeue(&chan->wp_tx_pending_list)) != NULL){
		wan_skb_free(skb);
	}

        while ((skb=wan_skb_dequeue(&chan->wp_tx_complete_list)) != NULL){
		wan_skb_free(skb);
	} 
	
	
	if (chan->tx_idle_skb){
		wan_skb_free(chan->tx_idle_skb);
		chan->tx_idle_skb=NULL;
	}
	
	if (chan->tx_realign_buf){
		wan_free(chan->tx_realign_buf);
		chan->tx_realign_buf=NULL;
	}

	if (chan->tx_ss7_realign_buf){
		wan_free(chan->tx_ss7_realign_buf);
		chan->tx_ss7_realign_buf=NULL;
	}

	if (card->u.aft.tdmv_chan_ptr == chan){
		card->u.aft.tdmv_chan_ptr=NULL;
	}

	chan->logic_ch_num=-1; 
        wan_spin_unlock_irq(&card->wandev.lock,&flags);

	/* Delete interface name from proc fs. */
#if 0
	wanrouter_proc_delete_interface(wandev, chan->if_name);
#endif

	/* Decrement the number of network interfaces
	 * configured on this card.
	 */
	wan_atomic_dec(&card->wandev.if_cnt);
	if (chan->hdlc_eng){
		--card->u.aft.security_cnt;
	}

	DEBUG_SUB_MEM(sizeof(private_area_t));
	return 0;
}

static int del_if (wan_device_t* wandev, netdevice_t* dev)
{
	private_area_t*	chan=wan_netif_priv(dev);
	wan_smp_flag_t flags;
	sdla_t *card;
	
	if (!chan){
		DEBUG_EVENT("%s: Critical Error del_if() chan=NULL!\n",
			  wan_netif_name(dev));	
		return 0;
	}

	if (!(card=chan->card)){
		DEBUG_EVENT("%s: Critical Error del_if() chan=NULL!\n",
			  wan_netif_name(dev));	
		return 0;
	}
	
	wan_spin_lock_irq(&card->wandev.lock,&flags);
	aft_dev_close(card,chan);
	wan_spin_unlock_irq(&card->wandev.lock,&flags);
	
	if (chan->channelized_cfg) {

		sdla_t *card=chan->card;

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		if (chan->tdmv_zaptel_cfg) {
			sdla_t *card=chan->card;
			int	err;
			WAN_TDMV_CALL(running, (card), err);
			if (err){
				return -EBUSY;
			}
		}
#endif

		/* Disable the TDMV Interrupt first, before
		 * shutting down all TDMV channels */
	        wan_spin_lock_irq(&card->wandev.lock,&flags);

		aft_tdm_intr_ctrl(card,0);
		aft_fifo_intr_ctrl(card, 0);
		
		/* Disable RTP Tap */
		card->u.aft.rtp_len=0;

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		if (card->wan_tdmv.sc) {
			int	err;
			WAN_TDMV_CALL(state, (card, WAN_DISCONNECTED), err);
		}
#endif
        	wan_spin_unlock_irq(&card->wandev.lock,&flags);

		while(chan){
			int err=del_if_private(wandev,dev);		
			if (err) {
				return err;
			}	
			if (chan->next) {
				wan_netif_set_priv(dev, chan->next);
				wan_free(chan);
				chan = wan_netif_priv(dev);
			} else {
				/* Leave the last chan dev
				 * in dev->priv.  It will get
				 * deallocated normally */
				break;
			}
		}

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		if (card->wan_tdmv.sc) {
	       	 	aft_tdmv_free(card);
		}
#endif
		return 0;
	} else {
		return del_if_private(wandev,dev);
	}
}


/**SECTION***********************************************************
 *
 * 	KERNEL Device Entry Interfaces
 *
 ********************************************************************/



/*============================================================================
 * if_init - Initialize Linux network interface.
 *
 * @dev:	Network interface pointer
 *
 * During "ifconfig up" the upper layer calls this function
 * to initialize dev access pointers.  Such as transmit,
 * stats and header.
 *
 * It is called only once for each interface,
 * during Linux network interface registration.
 *
 * Returning anything but zero will fail interface
 * registration.
 */
#if defined(__LINUX__)
static int if_init (netdevice_t* dev)
{
	private_area_t* chan = wan_netif_priv(dev);
	sdla_t*		card = chan->card;
	wan_device_t* 	wandev = &card->wandev;
#if defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
	hdlc_device*	hdlc;
#endif
	
	/* Initialize device driver entry points */
	dev->open		= &if_open;
	dev->stop		= &if_close;
#if defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
	hdlc		= dev_to_hdlc(dev);
	hdlc->xmit 	= if_send;
#else
	dev->hard_start_xmit	= &if_send;
#endif
	dev->get_stats		= &if_stats;

#if 0
	dev->tx_timeout		= &if_tx_timeout;
	dev->watchdog_timeo	= 2*HZ;
#else
	if (chan->common.usedby == TDM_VOICE || 
	    chan->common.usedby == TDM_VOICE_DCHAN ||
	    chan->common.usedby == TDM_VOICE_API){
		dev->tx_timeout		= NULL;
	}else{
		dev->tx_timeout		= &if_tx_timeout;
	}
	dev->watchdog_timeo	= 2*HZ;
		
#endif	
	dev->do_ioctl		= if_do_ioctl;

	if (chan->common.usedby == BRIDGE ||
            chan->common.usedby == BRIDGE_NODE){

		/* Setup the interface for Bridging */
		int hw_addr=0;
		ether_setup(dev);

		/* Use a random number to generate the MAC address */
		memcpy(dev->dev_addr, "\xFE\xFC\x00\x00\x00\x00", 6);
		get_random_bytes(&hw_addr, sizeof(hw_addr));
		*(int *)(dev->dev_addr + 2) += hw_addr;

	}else{

		dev->flags     |= IFF_POINTOPOINT;
		dev->flags     |= IFF_NOARP;
		dev->type	= ARPHRD_PPP;
		dev->mtu		= chan->mtu;
		dev->hard_header_len	= 0;
		dev->hard_header	= NULL; 
		dev->rebuild_header	= NULL;

		if (chan->common.usedby == API || chan->common.usedby == STACK){
			if (chan->hdlc_eng) {
				dev->mtu = chan->dma_mru+sizeof(api_tx_hdr_t);
			}else{
				dev->mtu = chan->mtu+sizeof(api_tx_hdr_t);
			}
		}

		/* Enable Mulitcasting if user selected */
		if (chan->mc == WANOPT_YES){
			dev->flags 	|= IFF_MULTICAST;
		}

		if (chan->true_if_encoding){
			DEBUG_EVENT("%s: Setting IF Type to Broadcast\n",chan->if_name);
			dev->type	= ARPHRD_PPP; /* This breaks the tcpdump */
			dev->flags     &= ~IFF_POINTOPOINT;
			dev->flags     |=  IFF_BROADCAST;
		}else{
			dev->type	= ARPHRD_PPP;
		}
	}

	/* Initialize hardware parameters */
	dev->irq	= wandev->irq;
	dev->dma	= wandev->dma;
	dev->base_addr	= wandev->ioport;
	card->hw_iface.getcfg(card->hw, SDLA_MEMBASE, &dev->mem_start);
	card->hw_iface.getcfg(card->hw, SDLA_MEMEND, &dev->mem_end);

	/* Set transmit buffer queue length
	 * If too low packets will not be retransmitted
         * by stack.
	 */
        dev->tx_queue_len = 100;
	return 0;
}
#endif

/*============================================================================
 * if_open - Open network interface.
 *
 * @dev: Network device pointer
 *
 * On ifconfig up, this function gets called in order
 * to initialize and configure the private area.
 * Driver should be configured to send and receive data.
 *
 * This functions starts a timer that will call
 * frmw_config() function. This function must be called
 * because the IP addresses could have been changed
 * for this interface.
 *
 * Return 0 if O.k. or errno.
 */
static int if_open (netdevice_t* dev)
{
	private_area_t* chan = wan_netif_priv(dev);
	sdla_t* card = chan->card;



#if defined(__LINUX__)
	/* Only one open per interface is allowed */
	if (open_dev_check(dev)){
		DEBUG_EVENT("%s: Open dev check failed!\n",
				wan_netif_name(dev));
		return -EBUSY;
	}
#endif

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		DEBUG_EVENT("%s:%s: Card down: Failed to open interface!\n",
			card->devname,chan->if_name);	
		return -EINVAL;
	}


	/* Initialize the router start time.
	 * Used by wanpipemon debugger to indicate
	 * how long has the interface been up */
	wan_getcurrenttime(&chan->router_start_time, NULL);

	WAN_NETIF_START_QUEUE(dev);
	
	if (card->wandev.state == WAN_CONNECTED){
		set_chan_state(card, dev, WAN_CONNECTED);
       		WAN_NETIF_CARRIER_ON(dev);
	} else {
       		WAN_NETIF_CARRIER_OFF(dev);
	}

	/* Increment the module usage count */
	wanpipe_open(card);


	/* Wait for the front end interrupt 
	 * before enabling the card */
	return 0;
}

/*============================================================================
 * if_close - Close network interface.
 *
 * @dev: Network device pointer
 *
 * On ifconfig down, this function gets called in order
 * to cleanup interace private area.
 *
 * IMPORTANT:
 *
 * No deallocation or unconfiguration should ever occur in this
 * function, because the interface can come back up
 * (via ifconfig up).
 *
 * Furthermore, in dynamic interfacace configuration mode, the
 * interface will come up and down to reflect the protocol state.
 *
 * Any deallocation and cleanup can occur in del_if()
 * function.  That function is called before the dev interface
 * itself is deallocated.
 *
 * Thus, we should only stop the net queue and decrement
 * the wanpipe usage counter via wanpipe_close() function.
 */

static int if_close (netdevice_t* dev)
{
	private_area_t* chan = wan_netif_priv(dev);
	sdla_t* card = chan->card;

	WAN_NETIF_STOP_QUEUE(dev);

#if defined(LINUX_2_1)
	dev->start=0;
#endif
	protocol_stop(card,dev);

	wanpipe_close(card);
	
	return 0;
}


/*=============================================================
 * disable_comm - Main shutdown function
 *
 * @card: Wanpipe device pointer
 *
 * The command 'wanrouter stop' has been called
 * and the whole wanpipe device is going down.
 * This is the last function called to disable
 * all comunications and deallocate any memory
 * that is still allocated.
 *
 * o Disable communications, turn off interrupts
 * o Deallocate memory used, if any
 * o Unconfigure TE1 card
 */

static void disable_comm (sdla_t *card)
{
	wan_smp_flag_t flags,flags1;
	int used_cnt;

	AFT_FUNC_DEBUG();
#if INIT_FE_ONLY
	aft_chip_unconfigure(card);
#else

   	/* Unconfiging, only on shutdown */
	if (IS_TE1_CARD(card) || IS_56K_CARD(card)) {
		wan_smp_flag_t smp_flags,smp_flags1;
		if (card->wandev.fe_iface.pre_release){
			card->wandev.fe_iface.pre_release(&card->fe);
		}
		card->hw_iface.hw_lock(card->hw,&smp_flags1);
		wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
		__aft_fe_intr_ctrl(card, 0);
		if (card->wandev.fe_iface.unconfig){
			card->wandev.fe_iface.unconfig(&card->fe);
		}
		__aft_fe_intr_ctrl(card, 1);
		wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
		card->hw_iface.hw_unlock(card->hw,&smp_flags1);

	}
	
	wan_spin_lock_irq(&card->wandev.lock,&flags);

	wan_set_bit(CARD_DOWN,&card->wandev.critical);

	/* Disable DMA ENGINE before we perform 
         * core reset.  Otherwise, we will receive
         * rx fifo errors on subsequent resetart. */
	disable_data_error_intr(card,DEVICE_DOWN);
	
	aft_rtp_unconfig(card);

	wan_spin_unlock_irq(&card->wandev.lock,&flags);

	aft_chip_unconfigure(card);

	WP_DELAY(10);
	

	card->hw_iface.getcfg(card->hw, SDLA_USEDCNT, &used_cnt);
	
	card->hw_iface.hw_lock(card->hw,&flags1);
	wan_spin_lock_irq(&card->wandev.lock,&flags);
       	__aft_fe_intr_ctrl(card, 0);
	aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_RED, 0,WAN_AFT_ON);
	aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_GREEN, 0, WAN_AFT_ON);
       	__aft_fe_intr_ctrl(card, 1);
	wan_spin_unlock_irq(&card->wandev.lock,&flags);
	card->hw_iface.hw_unlock(card->hw,&flags1);

	__sdla_pull_ptr_isr_array(card->hw,card,card->wandev.comm_port);

	if (used_cnt<=1){
		DEBUG_EVENT("%s: Global Chip Shutdown Usage=%d\n",
				card->devname,used_cnt);

		wan_spin_lock_irq(&card->wandev.lock,&flags);
		aft_global_chip_disable(card);		
		wan_spin_unlock_irq(&card->wandev.lock,&flags);
	}

	
#if defined(WAN_DEBUG_MEM)
        DEBUG_EVENT("%s: Total Mem %d\n",__FUNCTION__,wan_atomic_read(&wan_debug_mem));
#endif
#endif
	return;
}



/*============================================================================
 * if_tx_timeout
 *
 * Kernel networking stack calls this function in case
 * the interface has been stopped for TX_TIMEOUT seconds.
 *
 * This would occur if we lost TX interrupts or the
 * card has stopped working for some reason.
 *
 * Handle transmit timeout event from netif watchdog
 */
static void if_tx_timeout (netdevice_t *dev)
{
    	private_area_t* chan = wan_netif_priv(dev);
    	private_area_t* ch_ptr;
	sdla_t *card = chan->card;
	unsigned int cur_dma_ptr;
	u32 reg,dma_ram_desc;
	wan_smp_flag_t smp_flags;

	/* If our device stays busy for at least 5 seconds then we will
	 * kick start the device by making dev->tbusy = 0.  We expect
	 * that our device never stays busy more than 5 seconds. So this
	 * is only used as a last resort.
	 */

	++chan->if_stats.collisions;

	DEBUG_EVENT( "%s: Transmit timed out on %s\n",
					card->devname,
					wan_netif_name(dev));

	dma_ram_desc=chan->logic_ch_num*4+AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	cur_dma_ptr=aft_dmachain_get_tx_dma_addr(reg);

	DEBUG_EVENT("%s: Chain TxPend=%d, TxCur=%d, TxPend=%d HwCur=%d TxA=%d TxC=%ld\n",
			chan->if_name, 
			wan_test_bit(TX_INTR_PENDING,&chan->dma_chain_status),
			chan->tx_chain_indx,
			chan->tx_pending_chain_indx,
			cur_dma_ptr,
			chan->tx_attempts,
			chan->if_stats.tx_packets);

	if (wan_test_bit(TX_DMA_BUSY,&chan->dma_status)){
		wan_clear_bit(TX_DMA_BUSY,&chan->dma_status);
	}

	wan_netif_set_ticks(dev, SYSTEM_TICKS);

#ifdef AFT_TX_FIFO_DEBUG
	aft_list_tx_descriptors(chan);
#endif

	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
	if (chan->channelized_cfg){
		for (ch_ptr=chan;ch_ptr;ch_ptr=ch_ptr->next){
			aft_tx_fifo_under_recover(card,chan);
		}
	}else{
		aft_tx_fifo_under_recover(card,chan);
	}
	wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);

#ifdef AFT_TX_FIFO_DEBUG
	aft_list_tx_descriptors(chan);
#endif
	
	WAN_NETIF_WAKE_QUEUE(dev);
	if (chan->common.usedby == API){
		wan_wakeup_api(chan);
	}else if (chan->common.usedby == STACK){
		wanpipe_lip_kick(chan,0);
	}else if (chan->common.usedby == TDM_VOICE_DCHAN){
#ifdef AFT_TDM_API_SUPPORT
		if (is_tdm_api(chan,&chan->wp_tdm_api_dev)){
			wanpipe_tdm_api_kick(&chan->wp_tdm_api_dev);
		}
#endif
	}
}


/*============================================================================
 * if_send - Send a packet on a network interface.
 *
 * @dev:	Network interface pointer
 * @skb:	Packet obtained from the stack or API
 *              that should be sent out the port.
 *
 * o Mark interface as stopped
 * 	(marks start of the transmission) to indicate
 * 	to the stack that the interface is busy.
 *
 * o Check link state.
 * 	If link is not up, then drop the packet.
 *
 * o Copy the tx packet into the protocol tx buffers on
 *   the adapter.
 *
 * o If tx successful:
 * 	Free the skb buffer and mark interface as running
 * 	and return 0.
 *
 * o If tx failed, busy:
 * 	Keep interface marked as busy
 * 	Do not free skb buffer
 * 	Enable Tx interrupt (which will tell the stack
 * 	                     that interace is not busy)
 * 	Return a non-zero value to tell the stack
 * 	that the tx should be retried.
 *
 * Return:	0	complete (socket buffer must be freed)
 *		non-0	packet may be re-transmitted
 *
 */
#if defined(__LINUX__)
static int if_send (netskb_t* skb, netdevice_t* dev)
#else
static int if_send(netdevice_t *dev, netskb_t *skb, struct sockaddr *dst,struct rtentry *rt)
#endif
{
	private_area_t *chan = wan_netif_priv(dev);
	sdla_t *card = chan->card;
	int err;
	wan_smp_flag_t smp_flags;

	/* Mark interface as busy. The kernel will not
	 * attempt to send any more packets until we clear
	 * this condition */

	if (skb == NULL){
		/* This should never happen. Just a sanity check.
		 */
		DEBUG_EVENT( "%s: interface %s got kicked!\n",
					card->devname,
					wan_netif_name(dev));

		WAN_NETIF_WAKE_QUEUE(dev);
		return 0;
	}

	/* Non 2.4 kernels used to call if_send()
	 * after TX_TIMEOUT seconds have passed of interface
	 * being busy. Same as if_tx_timeout() in 2.4 kernels */
#if defined(LINUX_2_1)
	if (dev->tbusy){

		/* If our device stays busy for at least 5 seconds then we will
		 * kick start the device by making dev->tbusy = 0.  We expect
		 * that our device never stays busy more than 5 seconds. So this
		 * is only used as a last resort.
		 */
                ++chan->if_stats.collisions;
		if((SYSTEM_TICKS - chan->tick_counter) < (5 * HZ)) {
			return 1;
		}

		if_tx_timeout(dev);
	}
#endif
	err=0;
		
	if (chan->common.state != WAN_CONNECTED){
#if 1
		WAN_NETIF_STOP_QUEUE(dev);
		wan_netif_set_ticks(dev, SYSTEM_TICKS);
		++chan->if_stats.tx_carrier_errors;
		return 1;
#else
		++chan->if_stats.tx_carrier_errors;
		wan_skb_free(skb);
              	WAN_NETIF_START_QUEUE(dev);
		err=0;
		goto if_send_exit_crit;
#endif
	}

	if (chan->channelized_cfg) {
	
		private_area_t *top_chan=wan_netif_priv(chan->common.dev);

		DEBUG_TEST("%s:%ld Prev: Zaptel HDLC Tt TDMV_DCHAN=%i\n",
				chan->if_name,chan->logic_ch_num,
				card->u.aft.tdmv_dchan-1);

		if (!card->u.aft.tdmv_dchan || card->u.aft.tdmv_dchan>32){

			DEBUG_EVENT("%s: DCHAN TX No DCHAN Configured!\n",
					card->devname);

			wan_skb_free(skb);
              		WAN_NETIF_START_QUEUE(dev);
			err=0;
			goto if_send_exit_crit;
		}
		
		chan=(private_area_t*)card->u.aft.dev_to_ch_map[card->u.aft.tdmv_dchan-1];
		if (!chan){
			DEBUG_EVENT("%s: DCHAN TX No DCHAN Configured by not preset!\n",
					card->devname);
			wan_skb_free(skb);
              		WAN_NETIF_START_QUEUE(dev);
			err=0;
			goto if_send_exit_crit;
		}

		if (!chan->hdlc_eng){
			wan_skb_free(skb);
              		WAN_NETIF_START_QUEUE(dev);
			err=0;
			goto if_send_exit_crit;
		}

		wan_capture_trace_packet(chan->card, &top_chan->trace_info,
					     skb,TRC_OUTGOING_FRM);
	
		DEBUG_TEST("%s:%ld Zaptel HDLC Tt TDMV_DCHAN=%i\n",
				chan->if_name,chan->logic_ch_num,
				card->u.aft.tdmv_dchan-1);
	}

	/* For TDM_VOICE_API no tx is supported in if_send */
	if (chan->common.usedby == TDM_VOICE_API){
		++chan->if_stats.tx_errors;
              	WAN_NETIF_START_QUEUE(dev);
		err=0;
		goto if_send_exit_crit;
	}
	
	if (chan->common.usedby == API){ 
		
		if (sizeof(api_tx_hdr_t) >= wan_skb_len(skb)){
			wan_skb_free(skb);
			++chan->if_stats.tx_errors;
              		WAN_NETIF_START_QUEUE(dev);
			err=0;
			goto if_send_exit_crit;
		}
		
		if (chan->cfg.ss7_enable){
			err=aft_ss7_tx_mangle(card,chan,skb);
			if (err){
				wan_skb_free(skb);
				++chan->if_stats.tx_errors;
              			WAN_NETIF_START_QUEUE(dev);
				err=0;
				goto if_send_exit_crit;
			}
		}else{
			wan_skb_pull(skb,sizeof(api_tx_hdr_t));
		}		
	}

	if (!chan->hdlc_eng && chan->tslot_sync){
		if (wan_skb_len(skb)%chan->num_of_time_slots){
			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s:%s: Tx Error pkt len(%d) not multiple of timeslots(%d)\n",
						card->devname,
						chan->if_name,
						wan_skb_len(skb),
						chan->num_of_time_slots);
			}
			wan_skb_free(skb);
			++chan->if_stats.tx_errors;
              		WAN_NETIF_START_QUEUE(dev);
			err=0;
			goto if_send_exit_crit;
		}
	}
	
	if (!chan->hdlc_eng && !chan->lip_atm && (wan_skb_len(skb)%4)){
		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s: Tx Error: Tx Length %i is not 32bit divisible\n",
				chan->if_name,wan_skb_len(skb));
		}
		wan_skb_free(skb);
		++chan->if_stats.tx_errors;
              	WAN_NETIF_START_QUEUE(dev);
		err=0;
		goto if_send_exit_crit;
	}

	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);

	if (wan_skb_queue_len(&chan->wp_tx_pending_list) > chan->max_tx_bufs){
		WAN_NETIF_STOP_QUEUE(dev);
		aft_dma_tx(card,chan);	
		wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
		return 1;
		
	}

	wan_skb_unlink(skb);

	wan_skb_queue_tail(&chan->wp_tx_pending_list,skb);

	if(!chan->lip_atm){
		aft_dma_tx(card,chan);	/*not needed for LIP_ATM!!*/
	}
	

	wan_netif_set_ticks(dev, SYSTEM_TICKS);
	WAN_NETIF_START_QUEUE(dev);
	err=0;
	wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);

#if defined(__LINUX__)
	if (dev->tx_queue_len < chan->max_tx_bufs && 
	    dev->tx_queue_len > 0) {
        	DEBUG_EVENT("%s: Resizing Tx Queue Len to %li\n",
				chan->if_name,dev->tx_queue_len);
		chan->max_tx_bufs = dev->tx_queue_len;      	  
	}

	if (dev->tx_queue_len > chan->max_tx_bufs &&
	    chan->max_tx_bufs != chan->max_tx_bufs_orig) {
         	 DEBUG_EVENT("%s: Resizing Tx Queue Len to %i\n",
				chan->if_name,chan->max_tx_bufs_orig);
		chan->max_tx_bufs = chan->max_tx_bufs_orig; 
	}
#endif

if_send_exit_crit:

	return err;
}


/*============================================================================
 * if_stats
 *
 * Used by /proc/net/dev and ifconfig to obtain interface
 * statistics.
 *
 * Return a pointer to struct net_device_stats.
 */
static struct net_device_stats gstats;
static struct net_device_stats* if_stats (netdevice_t* dev)
{
	private_area_t* chan;
        sdla_t *card;
	
	if ((chan=wan_netif_priv(dev)) == NULL)
		return &gstats;
	
	card=chan->card;

#if !defined(AFT_IRQ_DEBUG)
	if (card) {
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
      		if (card->wan_tdmv.sc &&
		    !card->u.aft.rtp_len &&
		    card->wandev.config_id != WANCONFIG_AFT_ANALOG && 
		    chan->common.usedby == TDM_VOICE) {
			chan->if_stats.rx_packets = card->wandev.stats.rx_packets;
			chan->if_stats.tx_packets = card->wandev.stats.tx_packets;
		}
#endif
	}
#endif

	return &chan->if_stats;

}

/*========================================================================
 *
 * if_do_ioctl - Ioctl handler for fr
 *
 * 	@dev: Device subject to ioctl
 * 	@ifr: Interface request block from the user
 *	@cmd: Command that is being issued
 *
 *	This function handles the ioctls that may be issued by the user
 *	to control or debug the protocol or hardware .
 *
 *	It does both busy and security checks.
 *	This function is intended to be wrapped by callers who wish to
 *	add additional ioctl calls of their own.
 *
 * Used by:  SNMP Mibs
 * 	     wanpipemon debugger
 *
 */
static int if_do_ioctl(netdevice_t *dev, struct ifreq *ifr, int cmd)
{
	private_area_t* chan= (private_area_t*)wan_netif_priv(dev);
	sdla_t *card;
#if defined(__LINUX__)
	wan_smp_flag_t smp_flags;
#endif
	wan_udp_pkt_t *wan_udp_pkt;
	int err=-EOPNOTSUPP;

	if (!chan || !chan->card){
		DEBUG_EVENT("%s:%d: No Chan of card ptr\n",
				__FUNCTION__,__LINE__);
		return -ENODEV;
	}
	card=chan->card;

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		DEBUG_EVENT("%s: Card down: Ignoring Ioctl call!\n",
			card->devname);	
		return -ENODEV;
	}

	switch(cmd)
	{
#if defined(__LINUX__)
		case SIOC_WANPIPE_BIND_SK:
			if (!ifr){
				err= -EINVAL;
				break;
			}
	
			wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
			err=wan_bind_api_to_svc(chan,ifr->ifr_data);
			chan->if_stats.rx_dropped=0;
			if (!chan->hdlc_eng){
				chan->if_stats.tx_carrier_errors=0;
			}
			wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
			break;

		case SIOC_WANPIPE_UNBIND_SK:
			if (!ifr){
				err= -EINVAL;
				break;
			}

			wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
			err=wan_unbind_api_from_svc(chan,ifr->ifr_data);
			wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);

			break;

		case SIOC_WANPIPE_CHECK_TX:
		case SIOC_ANNEXG_CHECK_TX:
			err=0;
			break;

		case SIOC_WANPIPE_DEV_STATE:
			err = chan->common.state;
			break;

		case SIOC_ANNEXG_KICK:
			err=0;
			break;

		case SIOC_AFT_SS7_FORCE_RX:
			wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
			aft_set_ss7_force_rx(card,chan);	
			wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
			break;
#endif

#if defined(AFT_API_SUPPORT)
		case SIOC_WANPIPE_API:
			DEBUG_TEST("%s: WANPIPE API IOCTL!\n", card->devname);
			err = wan_aft_api_ioctl(card,chan,ifr->ifr_data);	
			break;
#endif

		case SIOC_WAN_DEVEL_IOCTL:
			err = aft_devel_ioctl(card, ifr);
			break;

		case SIOC_AFT_CUSTOMER_ID:
			if (!ifr){
				return -EINVAL;
			} else {
				unsigned char cid=0;
				wan_smp_flag_t smp_flags1;
				card->hw_iface.hw_lock(card->hw,&smp_flags1);
				wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
				cid=aft_read_cpld(card,CUSTOMER_CPLD_ID_REG);
				wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
				card->hw_iface.hw_unlock(card->hw,&smp_flags1);
				return WAN_COPY_TO_USER(ifr->ifr_data,&cid,sizeof(unsigned char));
			}
			err=0;
			break;
	
#if defined(__LINUX__)	
		case SIOC_WANPIPE_GET_DEVICE_CONFIG_ID:
			err=card->wandev.config_id;
			break;
#endif
		case SIOC_WANPIPE_SNMP:
		case SIOC_WANPIPE_SNMP_IFSPEED:
			return wan_snmp_data(card, dev, cmd, ifr);

		case SIOC_WANPIPE_PIPEMON:
			
			NET_ADMIN_CHECK();

			if (wan_atomic_read(&chan->udp_pkt_len) != 0){
				return -EBUSY;
			}

			wan_atomic_set(&chan->udp_pkt_len,MAX_LGTH_UDP_MGNT_PKT);

			/* For performance reasons test the critical
			 * here before spin lock */
			if (wan_test_bit(0,&card->in_isr)){
				wan_atomic_set(&chan->udp_pkt_len,0);
				return -EBUSY;
			}


			wan_udp_pkt=(wan_udp_pkt_t*)chan->udp_pkt_data;
			if (WAN_COPY_FROM_USER(
					&wan_udp_pkt->wan_udp_hdr,
					ifr->ifr_data,
					sizeof(wan_udp_hdr_t))){
				wan_atomic_set(&chan->udp_pkt_len,0);
				return -EFAULT;
			}

			/* We have to check here again because we don't know
			 * what happened during spin_lock */
			if (wan_test_bit(0,&card->in_isr)) {
				DEBUG_EVENT( "%s:%s Pipemon command failed, Driver busy: try again.\n",
						card->devname,
						wan_netif_name(dev));
				wan_atomic_set(&chan->udp_pkt_len,0);
				return -EBUSY;
			}

			process_udp_mgmt_pkt(card,dev,chan,1);

			/* This area will still be critical to other
			 * PIPEMON commands due to udp_pkt_len
			 * thus we can release the irq */

			if (wan_atomic_read(&chan->udp_pkt_len) > sizeof(wan_udp_pkt_t)){
				DEBUG_EVENT( "%s: Error: Pipemon buf too bit on the way up! %d\n",
						card->devname,wan_atomic_read(&chan->udp_pkt_len));
				wan_atomic_set(&chan->udp_pkt_len,0);
				return -EINVAL;
			}

			if (WAN_COPY_TO_USER(
					ifr->ifr_data,
					&wan_udp_pkt->wan_udp_hdr,
					sizeof(wan_udp_hdr_t))){
				wan_atomic_set(&chan->udp_pkt_len,0);
				return -EFAULT;
			}

			wan_atomic_set(&chan->udp_pkt_len,0);
			return 0;

#if 0
		case SIOC_WAN_EC_IOCTL:
			if (wan_test_and_set_bit(CARD_HW_EC,&card->wandev.critical)){
				DEBUG_EVENT("%s: Error: EC IOCTL Reentrant!\n",
						card->devname);
				return -EBUSY;
			}
			if (card->wandev.ec){
				err = wan_ec_ioctl(card->wandev.ec, ifr, card);
			}else{
				err = -EINVAL;
			}
			wan_clear_bit(CARD_HW_EC,&card->wandev.critical);
			break;
#endif

/*
		case SIOC_WAN_FE_IOCTL:
			DEBUG_TEST("%s: Command %x not supported!\n",
				card->devname,cmd);
			return -EOPNOTSUPP;
			break;
*/
		default:
#ifndef WANPIPE_GENERIC
			DEBUG_TEST("%s: Command %x not supported!\n",
				card->devname,cmd);
			return -EOPNOTSUPP;
#else
			if (card->wandev.ioctl){
				err = card->wandev.hdlc_ioctl(card, dev, ifr, cmd);
			}
#endif
	}

	return err;
}


/**SECTION**********************************************************
 *
 * 	FIRMWARE Specific Interface Functions
 *
 *******************************************************************/


#define FIFO_RESET_TIMEOUT_CNT 1000
#define FIFO_RESET_TIMEOUT_US  10
static int aft_init_rx_dev_fifo(sdla_t *card, private_area_t *chan, unsigned char wait)
{

        u32 reg;
        u32 dma_descr;
	u8  timeout=1;
	u16 i;
	unsigned int cur_dma_ptr;
	u32 dma_ram_desc;
	
        /* Clean RX DMA fifo */

	if (chan->single_dma_chain){
		dma_descr=(chan->logic_ch_num<<4) + AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);
		
		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
		aft_dmachain_set_rx_dma_addr(&reg,0);	
		card->hw_iface.bus_write_4(card->hw,dma_ram_desc,reg);
	}else{
		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
		cur_dma_ptr=aft_dmachain_get_rx_dma_addr(reg);
	
     	   	dma_descr=(chan->logic_ch_num<<4) + (cur_dma_ptr*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);
	}
	
        reg=0;
        wan_set_bit(AFT_RXDMA_HI_DMA_CMD_BIT,&reg);

        DEBUG_TEST("%s: Clearing RX Fifo %s Ch=%ld DmaDescr=(0x%X) Reg=(0x%X)\n",
                                __FUNCTION__,chan->if_name,chan->logic_ch_num,
                                dma_descr,reg);

       	card->hw_iface.bus_write_4(card->hw,dma_descr,reg);

	if (wait == WP_WAIT){
		for(i=0;i<FIFO_RESET_TIMEOUT_CNT;i++){
			card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);
			if (wan_test_bit(AFT_RXDMA_HI_DMA_CMD_BIT,&reg)){
				WP_DELAY(FIFO_RESET_TIMEOUT_US);
				continue;
			}
			timeout=0;
			break;
		} 

		if (timeout){
			DEBUG_EVENT("%s:%s: Error: Rx fifo reset timedout %u us\n",
				card->devname,chan->if_name,i*FIFO_RESET_TIMEOUT_US);
		}else{
			DEBUG_TEST("%s:%s: Rx Fifo Reset Successful\n",
				card->devname,chan->if_name); 
		}
	}else{
		timeout=0;
	}

	return timeout;
}

static int aft_init_tx_dev_fifo(sdla_t *card, private_area_t *chan, unsigned char wait)
{
	u32 reg;
        u32 dma_descr,dma_ram_desc;
        u8  timeout=1;
	u16 i;
	unsigned int cur_dma_ptr;

	if (chan->single_dma_chain){
		dma_descr=(chan->logic_ch_num<<4) + AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);

		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
		aft_dmachain_set_tx_dma_addr(&reg,0);	
		card->hw_iface.bus_write_4(card->hw,dma_ram_desc,reg);
	}else{
		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
		cur_dma_ptr=aft_dmachain_get_tx_dma_addr(reg);

		/* Clean TX DMA fifo */
		dma_descr=(chan->logic_ch_num<<4) + (cur_dma_ptr*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);
	}

        reg=0;
        wan_set_bit(AFT_TXDMA_HI_DMA_CMD_BIT,&reg);

        DEBUG_TEST("%s: Clearing TX Fifo %s DmaDescr=(0x%X) Reg=(0x%X)\n",
                                __FUNCTION__,chan->if_name,
                                dma_descr,reg);

        card->hw_iface.bus_write_4(card->hw,dma_descr,reg);

	if (wait == WP_WAIT){
        	for(i=0;i<FIFO_RESET_TIMEOUT_CNT;i++){
                	card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);
                	if (wan_test_bit(AFT_TXDMA_HI_DMA_CMD_BIT,&reg)){
                        	WP_DELAY(FIFO_RESET_TIMEOUT_US);
                        	continue;
                	}
               		timeout=0;
               		break;
        	}

        	if (timeout){
                	DEBUG_EVENT("%s:%s: Error: Tx fifo reset timedout %u us\n",
                                card->devname,chan->if_name,i*FIFO_RESET_TIMEOUT_US);
        	}else{
                	DEBUG_TEST("%s:%s: Tx Fifo Reset Successful\n",
                                card->devname,chan->if_name);
        	}
	}else{
		timeout=0;
	}

	return timeout;
}

static void aft_channel_txdma_ctrl(sdla_t *card, private_area_t *chan, int on)
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
static void aft_channel_rxdma_ctrl(sdla_t *card, private_area_t *chan, int on)
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

static void aft_channel_txintr_ctrl(sdla_t *card, private_area_t *chan, int on)
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

static void aft_channel_rxintr_ctrl(sdla_t *card, private_area_t *chan, int on)
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


static void aft_dev_enable(sdla_t *card, private_area_t *chan)
{
	DEBUG_TEST("%s: Enabling Global Inter Mask !\n",chan->if_name);

	/* Enable TX DMA for Logic Channel */
	aft_channel_txdma_ctrl(card,chan,1);
	
	/* Enable RX DMA for Logic Channel */
	aft_channel_rxdma_ctrl(card,chan,1);

	/* Enable Logic Channel TX Interrupts */
	if (chan->channelized_cfg && !chan->hdlc_eng){
		aft_channel_txintr_ctrl(card,chan,0);
		aft_channel_rxintr_ctrl(card,chan,0);
		chan->tdmv_irq_cfg=1;
	}else{
		aft_channel_txintr_ctrl(card,chan,1);
		aft_channel_rxintr_ctrl(card,chan,1);
	}

	wan_set_bit(chan->logic_ch_num,&card->u.aft.active_ch_map);
}

static void aft_dev_open_private(sdla_t *card, private_area_t *chan)
{
	if (card->wandev.state == WAN_CONNECTED &&
	    wan_test_bit(0,&card->u.aft.comm_enabled)){

		DEBUG_TEST("%s: OPEN reseting fifo\n",
					chan->if_name);

		aft_tslot_sync_ctrl(card,chan,0);

		aft_init_rx_dev_fifo(card,chan,WP_NO_WAIT);
		aft_init_tx_dev_fifo(card,chan,WP_NO_WAIT);
		
		aft_dev_enable(card,chan);

		aft_init_rx_dev_fifo(card,chan,WP_WAIT);
		aft_init_tx_dev_fifo(card,chan,WP_WAIT);

		chan->dma_index=0;
		memset(chan->dma_history,0,sizeof(chan->dma_history));

		aft_reset_rx_chain_cnt(chan);
		aft_dma_rx(card,chan);

		aft_tslot_sync_ctrl(card,chan,1);
		
		if (!chan->hdlc_eng){
			aft_reset_tx_chain_cnt(chan);
			aft_dma_tx(card,chan);
		}
	}else{
		aft_dev_enable(card,chan);
	}

}

static void aft_dev_open(sdla_t *card, private_area_t *gchan)
{
	private_area_t *chan=gchan;

	if (chan->channelized_cfg){

		for (chan=gchan; chan != NULL; chan=chan->next){
			
			aft_dev_open_private(card,chan);

			wan_set_bit(0,&chan->up);
               	
		}

		if (gchan->common.usedby == TDM_VOICE_API){
			/* Set the global mtu value which is 
			 * the sum of all timeslots mtus */
			wan_netif_set_mtu(
				gchan->common.dev,
				card->u.aft.tdmv_mtu+sizeof(api_tx_hdr_t));
					  
		}
	
		wan_set_bit(0,&card->u.aft.tdmv_master_if_up);
	
		if (card->wandev.state == WAN_CONNECTED &&
    		    !wan_test_bit(0,&card->u.aft.comm_enabled)){ 
			DEBUG_EVENT("%s: Master IF Starting %s Communications\n",
				gchan->if_name,card->devname);
			enable_data_error_intr(card);
		}
	}else{
		aft_dev_open_private(card,chan);
		wan_set_bit(0,&chan->up);
	}
	
	
	if (gchan->cfg.ss7_enable){
		aft_clear_ss7_force_rx(card,gchan);
	}
	
	return;
}

static void aft_dev_close_private(sdla_t *card, private_area_t *chan)
{

	if (chan->logic_ch_num < 0){
		return;
	}
		
	/* Disable Logic Channel TX Interrupts */
	aft_channel_txintr_ctrl(card,chan,0);

	/* Disable Logic Channel RX Interrupts */
	aft_channel_rxintr_ctrl(card,chan,0);
	
	/* Disable TX DMA for Logic Channel */
	aft_channel_txdma_ctrl(card,chan,0);
	
	/* Disable RX DMA for Logic Channel */
	aft_channel_rxdma_ctrl(card,chan,0);
	
	/* Initialize DMA descriptors and DMA Chains */
	aft_init_tx_rx_dma_descr(chan);

}


static void aft_dev_close(sdla_t *card, private_area_t *gchan)
{
	private_area_t *chan=gchan;

	if (chan->channelized_cfg){
		
		aft_tdm_intr_ctrl(card,0);
		aft_fifo_intr_ctrl(card, 0);

		for (chan=gchan; chan != NULL; chan=chan->next){
			
			aft_dev_close_private(card,chan);

			DEBUG_TEST("%s: Closing Ch=%ld\n",
					chan->if_name,chan->logic_ch_num);

			wan_clear_bit(0,&chan->up);
			wan_set_bit(0,&chan->interface_down);
			if (chan->cfg.tdmv_master_if){
				wan_clear_bit(0,&card->u.aft.tdmv_master_if_up);
			}
		}
	}else{
		wan_set_bit(0,&chan->interface_down);
		wan_clear_bit(0,&chan->up);
		aft_dev_close_private(card,chan);
		
		
	}
	return;
}

/**SECTION*************************************************************
 *
 * 	TX Handlers
 *
 **********************************************************************/


/*===============================================
 * aft_dma_tx_complete
 *
 */
static void aft_dma_tx_complete (sdla_t *card, private_area_t *chan, int wdt, int reset)
{
	DEBUG_TEST("%s: Tx interrupt wdt=%d\n",chan->if_name,wdt);

	if (!wdt){
		wan_clear_bit(TX_INTR_PENDING,&chan->dma_chain_status);
	}

	if (chan->channelized_cfg && !chan->hdlc_eng){
		aft_tx_dma_voice_handler((unsigned long)chan,wdt,reset);
	}else{
		aft_tx_dma_chain_handler((unsigned long)chan,wdt,reset);
	}
	
	wan_set_bit(0,&chan->idle_start);

	if (reset){
		return;
	}
	
	aft_dma_tx(card,chan);

	if (WAN_NETIF_QUEUE_STOPPED(chan->common.dev)){
		WAN_NETIF_WAKE_QUEUE(chan->common.dev);
#ifndef CONFIG_PRODUCT_WANPIPE_GENERIC
		if (chan->common.usedby == API){
			wan_wakeup_api(chan);
		}else if (chan->common.usedby == STACK){
			wanpipe_lip_kick(chan,0);
		}
#endif
	}
	
	if (chan->common.usedby == TDM_VOICE_DCHAN){
#ifdef AFT_TDM_API_SUPPORT
		if (is_tdm_api(chan,&chan->wp_tdm_api_dev)){
			wanpipe_tdm_api_kick(&chan->wp_tdm_api_dev);
		}
#endif
	}

	return;
}

/*===============================================
 * aft_tx_post_complete
 *
 */
static void aft_tx_post_complete (sdla_t *card, private_area_t *chan, netskb_t *skb)
{
	unsigned int reg =  wan_skb_csum(skb);
	u32 dma_status = aft_txdma_hi_get_dma_status(reg);

	wan_skb_set_csum(skb,0);

	if (reg & AFT_TXDMA_HI_DMA_LENGTH_MASK){
		chan->errstats.Tx_dma_len_nonzero++;
	}
	
	if ((wan_test_bit(AFT_TXDMA_HI_GO_BIT,&reg)) ||
	    (reg & AFT_TXDMA_HI_DMA_LENGTH_MASK) ||
	    dma_status){

		DEBUG_TEST("%s:%s: Tx DMA Descriptor=0x%X\n",
			card->devname,chan->if_name,reg);

		/* Checking Tx DMA Go bit. Has to be '0' */
		if (wan_test_bit(AFT_TXDMA_HI_GO_BIT,&reg)){
        		DEBUG_TEST("%s:%s: Error: TxDMA Intr: GO bit set on Tx intr\n",
                   		card->devname,chan->if_name);
			chan->errstats.Tx_dma_errors++;
		}

		if (reg & AFT_TXDMA_HI_DMA_LENGTH_MASK){
               		DEBUG_EVENT("%s:%s: Error: TxDMA Length not equal 0 (reg=0x%08X)\n",
                   		card->devname,chan->if_name,reg);
			chan->errstats.Tx_dma_errors++;
	        }   
 
    		/* Checking Tx DMA PCI error status. Has to be '0's */
		if (dma_status){
                	     	
			chan->errstats.Tx_pci_errors++;
			if (wan_test_bit(AFT_TXDMA_HIDMASTATUS_PCI_M_ABRT,&dma_status)){
				if (WAN_NET_RATELIMIT()){
        			DEBUG_EVENT("%s:%s: Tx Error: Abort from Master: pci fatal error!\n",
                	     		card->devname,chan->if_name);
				}
				
			}
			if (wan_test_bit(AFT_TXDMA_HIDMASTATUS_PCI_T_ABRT,&dma_status)){
				if (WAN_NET_RATELIMIT()){
        			DEBUG_EVENT("%s:%s: Tx Error: Abort from Target: pci fatal error!\n",
                	     		card->devname,chan->if_name);
				}
			}
			if (wan_test_bit(AFT_TXDMA_HIDMASTATUS_PCI_DS_TOUT,&dma_status)){
        			DEBUG_TEST("%s:%s: Tx Warning: PCI Latency Timeout!\n",
                	     		card->devname,chan->if_name);
				chan->errstats.Tx_pci_latency++;
				goto tx_post_ok;
			}
			if (wan_test_bit(AFT_TXDMA_HIDMASTATUS_PCI_RETRY,&dma_status)){
				if (WAN_NET_RATELIMIT()){
        			DEBUG_EVENT("%s:%s: Tx Error: 'Retry' exceeds maximum (64k): pci fatal error!\n",
                	     		card->devname,chan->if_name);
				}
			}
		}
		chan->if_stats.tx_errors++;
		goto tx_post_exit;
	}

tx_post_ok:

	chan->opstats.Data_frames_Tx_count++;
	chan->opstats.Data_bytes_Tx_count+=wan_skb_len(skb);
	chan->if_stats.tx_packets++;
	chan->if_stats.tx_bytes+=wan_skb_len(skb);

#if 0
	if (chan->common.usedby != TDM_VOICE){
		wan_capture_trace_packet(card, &chan->trace_info, skb, TRC_OUTGOING_FRM);
	}
#endif

tx_post_exit:

	return;
}



/**SECTION*************************************************************
 *
 * 	RX Handlers
 *
 **********************************************************************/


/*===============================================
 * aft_rx_post_complete
 *
 */
static void aft_rx_post_complete (sdla_t *card, private_area_t *chan, 
				     netskb_t *skb, 
				     netskb_t **new_skb,
				     unsigned char *pkt_error)
{

    	unsigned int len,data_error = 0;
	unsigned char *buf;
	wp_rx_element_t *rx_el;
	u32 dma_status;
	rx_el=(wp_rx_element_t *)wan_skb_data(skb);

	DEBUG_RX("%s:%s: RX HI=0x%X  LO=0x%X\n DMA=0x%X",
				__FUNCTION__,
				chan->if_name,
				rx_el->reg,
				rx_el->align,
				rx_el->dma_addr);   
	
#if 0
	/* debugging */
	chan->if_stats.rx_errors++;
#endif
	
	rx_el->align&=AFT_RXDMA_LO_ALIGN_MASK;
	*pkt_error=0;
	*new_skb=NULL;

	dma_status=aft_rxdma_hi_get_dma_status(rx_el->reg);
	
    	/* Checking Rx DMA Go bit. Has to be '0' */
	if (wan_test_bit(AFT_RXDMA_HI_GO_BIT,&rx_el->reg)){
        	DEBUG_TEST("%s:%s: Error: RxDMA Intr: GO bit set on Rx intr\n",
				card->devname,chan->if_name);
		chan->if_stats.rx_errors++;
		chan->errstats.Rx_dma_descr_err++;
		goto rx_comp_error;
	}
    
	/* Checking Rx DMA PCI error status. Has to be '0's */
	if (dma_status){

		if (wan_test_bit(AFT_RXDMA_HIDMASTATUS_PCI_M_ABRT,&dma_status)){
			if (WAN_NET_RATELIMIT()){
                	DEBUG_EVENT("%s:%s: Rx Error: Abort from Master: pci fatal error 0x%X!\n",
                                   card->devname,chan->if_name,rx_el->reg);
			}
                }
		if (wan_test_bit(AFT_RXDMA_HIDMASTATUS_PCI_T_ABRT,&dma_status)){
                        if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s:%s: Rx Error: Abort from Target: pci fatal error 0x%X!\n",
                                   card->devname,chan->if_name,rx_el->reg);
			}
                }
		if (wan_test_bit(AFT_RXDMA_HIDMASTATUS_PCI_DS_TOUT,&dma_status)){
                        if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s:%s: Rx Error: No 'DeviceSelect' from target: pci fatal error 0x%X!\n",
                                    card->devname,chan->if_name,rx_el->reg);
			}
                }
		if (wan_test_bit(AFT_RXDMA_HIDMASTATUS_PCI_RETRY,&dma_status)){
                        if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s:%s: Rx Error: 'Retry' exceeds maximum (64k): pci fatal error 0x%X!\n",
                                    card->devname,chan->if_name,rx_el->reg);
			}
                }

		chan->errstats.Rx_pci_errors++;
		chan->if_stats.rx_errors++;
		card->wandev.stats.rx_errors++;
		goto rx_comp_error;
	}

	if (chan->hdlc_eng){
 
		/* Checking Rx DMA Frame start bit. (information for api) */
		if (!wan_test_bit(AFT_RXDMA_HI_START_BIT,&rx_el->reg)){
			DEBUG_TEST("%s:%s RxDMA Intr: Start flag missing: MTU Mismatch! Reg=0x%X\n",
					card->devname,chan->if_name,rx_el->reg);
			chan->if_stats.rx_frame_errors++;
			chan->opstats.Rx_Data_discard_long_count++;
			chan->errstats.Rx_hdlc_corrupiton++;
			card->wandev.stats.rx_errors++;
			goto rx_comp_error;
		}
    
		/* Checking Rx DMA Frame end bit. (information for api) */
		if (!wan_test_bit(AFT_RXDMA_HI_EOF_BIT,&rx_el->reg)){
			DEBUG_TEST("%s:%s: RxDMA Intr: End flag missing: MTU Mismatch! Reg=0x%X\n",
					card->devname,chan->if_name,rx_el->reg);
			chan->if_stats.rx_frame_errors++;
			chan->opstats.Rx_Data_discard_long_count++;
			chan->errstats.Rx_hdlc_corrupiton++;
			card->wandev.stats.rx_errors++;
			goto rx_comp_error;
		
       	 	} else {  /* Check CRC error flag only if this is the end of Frame */
        	
			if (wan_test_bit(AFT_RXDMA_HI_FCS_ERR_BIT,&rx_el->reg)){
                   		DEBUG_TEST("%s:%s: RxDMA Intr: CRC Error! Reg=0x%X Len=%d\n",
                                		card->devname,chan->if_name,rx_el->reg,
						(rx_el->reg&AFT_RXDMA_HI_DMA_LENGTH_MASK)>>2);
				chan->if_stats.rx_frame_errors++;
				chan->errstats.Rx_crc_err_count++;
				card->wandev.stats.rx_crc_errors++;
				wan_set_bit(WP_CRC_ERROR_BIT,&rx_el->pkt_error);	
                   		data_error = 1;
               		}

			/* Check if this frame is an abort, if it is
                 	 * drop it and continue receiving */
			if (wan_test_bit(AFT_RXDMA_HI_FRM_ABORT_BIT,&rx_el->reg)){
				DEBUG_TEST("%s:%s: RxDMA Intr: Abort! Reg=0x%X\n",
						card->devname,chan->if_name,rx_el->reg);
				chan->if_stats.rx_frame_errors++;
				chan->errstats.Rx_hdlc_corrupiton++;
				card->wandev.stats.rx_frame_errors++;
				wan_set_bit(WP_ABORT_ERROR_BIT,&rx_el->pkt_error);
				data_error = 1;
			}
			
			if (chan->common.usedby != API && data_error){
				goto rx_comp_error;
			}	

		}
	}

	len=rx_el->reg&AFT_RXDMA_HI_DMA_LENGTH_MASK;
	
	if (chan->hdlc_eng){
		/* In HDLC mode, calculate rx length based
                 * on alignment value, received from DMA */
		len=((((chan->dma_mru>>2)-1)-len)<<2) - (~(rx_el->align)&AFT_RXDMA_LO_ALIGN_MASK);

		if (len < 3 || len > chan->dma_mru){
			chan->if_stats.rx_frame_errors++;
			chan->errstats.Rx_hdlc_corrupiton++;
			card->wandev.stats.rx_frame_errors++;
			goto rx_comp_error;
		}
	}else{
		/* In Transparent mode, our RX buffer will always be
		 * aligned to the 32bit (word) boundary, because
                 * the RX buffers are all of equal length  */
		len=(((chan->mru>>2)-len)<<2) - (~(0x03)&AFT_RXDMA_LO_ALIGN_MASK);

		if (len < 1 || len > chan->mru){
			chan->if_stats.rx_frame_errors++;
			card->wandev.stats.rx_frame_errors++;
			goto rx_comp_error;
		}
	}



	*pkt_error=rx_el->pkt_error;

	/* After a RX FIFO overflow, we must mark max 7 
         * subsequent frames since firmware, cannot 
         * guarantee the contents of the fifo */

	if (wan_test_bit(WP_FIFO_ERROR_BIT,&rx_el->pkt_error)){
		if (chan->hdlc_eng){
			if (++chan->rx_fifo_err_cnt >= WP_MAX_FIFO_FRAMES){
				chan->rx_fifo_err_cnt=0;
			}
		}else{
			chan->rx_fifo_err_cnt=0;
		}
		wan_set_bit(WP_FIFO_ERROR_BIT,pkt_error);
	}else{
		if (chan->rx_fifo_err_cnt){
			if (++chan->rx_fifo_err_cnt >= WP_MAX_FIFO_FRAMES){
                        	chan->rx_fifo_err_cnt=0;
			}
			wan_set_bit(WP_FIFO_ERROR_BIT,pkt_error);
		}
	}


	if (len > aft_rx_copyback){
		/* The rx size is big enough, thus
		 * send this buffer up the stack
		 * and allocate another one */
		memset(wan_skb_data(skb),0,sizeof(wp_rx_element_t));
		wan_skb_put(skb,len);	
		wan_skb_pull(skb, sizeof(wp_rx_element_t));
		*new_skb=skb;

		aft_alloc_rx_dma_buff(card,chan,1,1);
	}else{

		/* The rx packet is very
		 * small thus, allocate a new 
		 * buffer and pass it up */
		*new_skb=wan_skb_alloc(len + 20);
		if (!*new_skb){
			DEBUG_EVENT("%s:%s: Failed to allocate rx skb pkt (len=%d)!\n",
				card->devname,chan->if_name,(len+20));
			chan->if_stats.rx_dropped++;
			goto rx_comp_error;
		}

		buf=wan_skb_put((*new_skb),len);
		memcpy(buf,wan_skb_tail(skb),len);

		aft_init_requeue_free_skb(chan, skb);
	}

#if 0
	if (chan->hdlc_eng){
		buf=wan_skb_data(*new_skb);
		if (buf[wan_skb_len(*new_skb)-1] != 0x7E &&
		    buf[wan_skb_len(*new_skb)-1] != 0x7F){
			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: Rx: Invalid packet len=%d: 0x%X 0x%X 0x%X\n",
						card->devname,
						wan_skb_len(*new_skb),
						buf[wan_skb_len(*new_skb)-3],
						buf[wan_skb_len(*new_skb)-2],
						buf[wan_skb_len(*new_skb)-1]);
			}
		}
	}
#endif

	return;

rx_comp_error:

	aft_init_requeue_free_skb(chan, skb);
    	return;
}



/**SECTION**************************************************
 *
 * 	Logic Channel Registration Support and
 * 	Utility funcitons
 *
 **********************************************************/

static int aft_init_requeue_free_skb(private_area_t *chan, netskb_t *skb)
{
	WAN_ASSERT(skb == NULL);
	
	wan_skb_init(skb,16);
	wan_skb_trim(skb,0);
	wan_skb_queue_tail(&chan->wp_rx_free_list,skb);

	return 0;
}

static int aft_alloc_rx_dma_buff(sdla_t *card, private_area_t *chan, int num, int irq)
{
	int i;
	netskb_t *skb;
	
	for (i=0;i<num;i++){
		if (chan->channelized_cfg && !chan->hdlc_eng){
#if defined(WANPIPE_64BIT_4G_DMA)
#warning "Wanpipe compiled for 64bit 4G DMA"
			/* On 64bit Systems greater than 4GB we must
			 * allocated our DMA buffers using GFP_DMA 
			 * flag */
			if (irq) {
      				skb=__dev_alloc_skb(chan->dma_mru,GFP_DMA|GFP_ATOMIC);  	
			} else {
                         	skb=__dev_alloc_skb(chan->dma_mru,GFP_DMA|GFP_KERNEL);
			}
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
		if (!skb){
			DEBUG_EVENT("%s: %s  no rx memory\n",
					chan->if_name,__FUNCTION__);
			return -ENOMEM;
		}
		wan_skb_queue_tail(&chan->wp_rx_free_list,skb);
	}


		
	return 0;
}


/*============================================================================
 * Enable timer interrupt
 */
static void enable_timer (void* card_id)
{
	sdla_t*	card = (sdla_t*)card_id;
#if !defined(WAN_IS_TASKQ_SCHEDULE)
	wan_smp_flag_t	smp_flags; 
	wan_smp_flag_t	smp_flags1; 
	int		err = 0;
#endif

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		DEBUG_EVENT("%s: Card down: Ignoring enable_timer!\n",
			card->devname);	
		return;
	}

	DEBUG_56K("%s: %s Sdla Polling %p!\n",__FUNCTION__,
			card->devname,
			card->wandev.fe_iface.polling);
	
#if defined(WAN_IS_TASKQ_SCHEDULE)
	wan_set_bit(AFT_FE_POLL,&card->u.aft.port_task_cmd);
	WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
#else
	card->hw_iface.hw_lock(card->hw,&smp_flags1);
	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
	if (card->wandev.fe_iface.polling){
		err = card->wandev.fe_iface.polling(&card->fe);
	}
	wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
	card->hw_iface.hw_unlock(card->hw,&smp_flags1);
#endif

	return;
}

static void enable_ec_timer (void* card_id)
{
#if defined(CONFIG_WANPIPE_HWEC)
	sdla_t*	card = (sdla_t*)card_id;
# if !defined(WAN_IS_TASKQ_SCHEDULE)
	wan_smp_flag_t smp_flags; 
	wan_smp_flag_t smp_flags1; 
# endif

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		DEBUG_EVENT("%s: Card down: Ignoring enable_timer!\n",
			card->devname);	
		return;
	}

	DEBUG_TEST("%s: %s Sdla EC Polling !\n",__FUNCTION__,
			card->devname);
	
# if defined(WAN_IS_TASKQ_SCHEDULE)
	wan_set_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd);
	WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
# else
# error "TASK Q Not defined"
	card->hw_iface.hw_lock(card->hw,&smp_flags1);
	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
	
	wanpipe_ec_poll(card->wandev.ec_dev, card);
	
	wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
	card->hw_iface.hw_unlock(card->hw,&smp_flags1);
# endif
#endif
	return;
}
/**SECTION**************************************************
 *
 * 	API Bottom Half Handlers
 *
 **********************************************************/

static int tdm_check=0;

#if defined(__LINUX__)
static void wp_tdm_bh (unsigned long data)
#else
static void wp_tdm_bh (void *data, int pending)
#endif
{
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
	private_area_t* chan = (private_area_t *)data;
	sdla_t *card=chan->card;
	int	err;

#if 0
	if (WAN_NET_RATELIMIT()){
	DEBUG_EVENT("%s: TDM BH Running !\n",
		chan->if_name);
	}
#endif

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		goto wp_tdm_bh_exit;
	}

	if (!wan_test_bit(0,&chan->up)){
		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s: wp_tdm_bh() chan not up!\n",
                            	    chan->if_name);
		}	
		goto wp_tdm_bh_exit;
	}

#if 0
	if (tdm_check){
		if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s: TDM BH Already Running 0x%02X... cool!\n",
			chan->if_name,tdm_check);
		}
	}
	wan_set_bit(card->tdmv_conf.span_no,&tdm_check); 
#endif
	WAN_TDMV_CALL(rx_tx_span, (card), err);
	
	WAN_TASKLET_END((&chan->common.bh_task));
	
	if (card->wan_tdmv.sc){
		WAN_TDMV_CALL(is_rbsbits, (&card->wan_tdmv), err);
		if (err == 1){
			wan_set_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
			WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
		}
	}

#if 0
	wan_clear_bit(card->tdmv_conf.span_no,&tdm_check);
#endif
	tdm_check++;		

	return;

wp_tdm_bh_exit:
	WAN_TASKLET_END((&chan->common.bh_task));
#endif
	return;
}


#if defined(__LINUX__)
static void wp_bh (unsigned long data)
#else
static void wp_bh (void *data, int pending)
#endif
{
	private_area_t* chan = (private_area_t *)data;
	sdla_t *card=chan->card;
	netskb_t *new_skb,*skb;
	unsigned char pkt_error;
	unsigned long timeout=SYSTEM_TICKS;
	private_area_t *top_chan;
	int len;

#ifdef AFT_IRQ_STAT_DEBUG
      	card->wandev.stats.collisions++;
#endif	     
	
	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		WAN_TASKLET_END((&chan->common.bh_task));
		return;
	}
	
	if (card->u.aft.tdmv_dchan){
		top_chan=wan_netif_priv(chan->common.dev);
	}else{
		top_chan=chan;
	}
	
	DEBUG_TEST("%s: ------------ BEGIN --------------: %u\n",
			__FUNCTION__,SYSTEM_TICKS);

	if (!wan_test_bit(0,&chan->up)){
		if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s: wp_bh() chan not up!\n",
                                chan->if_name);
		}
		WAN_TASKLET_END((&chan->common.bh_task));
		return;
	}

	while((skb=wan_skb_dequeue(&chan->wp_rx_complete_list)) != NULL){

#if 0
		chan->if_stats.rx_errors++;
#endif

		if (SYSTEM_TICKS-timeout > 1){
			wan_skb_queue_head(&chan->wp_rx_complete_list,skb);
			break;
		}
		
		new_skb=NULL;
		pkt_error=0;
	

		
		/* The post function will take care
		 * of the skb and new_skb buffer.
		 * If new_skb buffer exists, driver
		 * must pass it up the stack, or free it */
		aft_rx_post_complete (chan->card, chan,
                                   	 skb,
                                     	 &new_skb,
                                     	 &pkt_error);
		if (new_skb){
	
			len=wan_skb_len(new_skb);

			if (chan->hdlc_eng){
				/* HDLC packets contain 2 byte crc and 1 byte
				 * flag. If data is not greater than 3, then
				 * we have a 0 length frame. Thus discard 
				 * (only if HDLC engine enabled) */
				if (len <= 3){
					++chan->if_stats.rx_errors;
					wan_skb_free(new_skb);
					continue;
				}

				wan_skb_trim(new_skb,wan_skb_len(new_skb)-3);	
				len-=3;
			}
			
			wan_capture_trace_packet(chan->card, &top_chan->trace_info,
					     new_skb,TRC_INCOMING_FRM);


			if (chan->common.usedby == API){

                                 if (chan->common.sk == NULL){
				 	DEBUG_TEST("%s: No sock bound to channel rx dropping!\n",
				 		chan->if_name);
				 	chan->if_stats.rx_dropped++;
					wan_skb_free(new_skb);
				 	continue; 
				 }

				if (chan->hdlc_eng){
					if (card->u.aft.cfg.rx_crc_bytes == 3){
						wan_skb_put(new_skb,3);
					}else if (card->u.aft.cfg.rx_crc_bytes == 2){
						wan_skb_put(new_skb,2);	
					}
				}

#if defined(__LINUX__)
# ifndef CONFIG_PRODUCT_WANPIPE_GENERIC


				/* Only for API, we insert packet status
				 * byte to indicate a packet error. Take
			         * this byte and put it in the api header */
				 
				if (wan_skb_headroom(new_skb) >= sizeof(api_rx_hdr_t)){
					api_rx_hdr_t *rx_hdr=
						(api_rx_hdr_t*)skb_push(new_skb,sizeof(api_rx_hdr_t));	
					memset(rx_hdr,0,sizeof(api_rx_hdr_t));
					rx_hdr->error_flag=pkt_error;
				}else{
					if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: Error Rx pkt headroom %u < %u\n",
							chan->if_name,
							(u32)wan_skb_headroom(new_skb),
							(u32)sizeof(api_rx_hdr_t));
					}
					++chan->if_stats.rx_dropped;
					wan_skb_free(new_skb);
					continue;
				}

				new_skb->protocol = htons(PVC_PROT);
				new_skb->mac.raw  = new_skb->data;
				new_skb->dev      = chan->common.dev;
				new_skb->pkt_type = WAN_PACKET_DATA;	
				
#if 0	
				chan->if_stats.rx_frame_errors++;
#endif

				if (wan_api_rx(chan,new_skb) != 0){
					++chan->if_stats.rx_dropped;
					wan_skb_free(new_skb);
					continue;
				}
				
# endif
#endif

			}else if (chan->common.usedby == TDM_VOICE_DCHAN){

#ifdef AFT_TDM_API_SUPPORT
				if (is_tdm_api(chan,&chan->wp_tdm_api_dev)) {
					int err;
					
					if (wan_skb_headroom(new_skb) >= sizeof(api_rx_hdr_t)){
						api_rx_hdr_t *rx_hdr =
						 (api_rx_hdr_t*)skb_push(new_skb,sizeof(api_rx_hdr_t));	
						memset(rx_hdr,0,sizeof(api_rx_hdr_t));
						//rx_hdr->error_flag=pkt_error;
					}else{
						if (WAN_NET_RATELIMIT()){
						DEBUG_EVENT("%s: Error Rx pkt headroom %u < %u\n",
								chan->if_name,
								(u32)wan_skb_headroom(new_skb),
								(u32)sizeof(api_rx_hdr_t));
						}
						++chan->if_stats.rx_dropped;
						wan_skb_free(new_skb);
						continue;
					}
					err=wanpipe_tdm_api_rx_hdlc(&chan->wp_tdm_api_dev,new_skb);	
					if (err){
						++chan->if_stats.rx_dropped;
						wan_skb_free(new_skb);
						continue;
					}
					
				}else
#endif
				if (chan->tdmv_zaptel_cfg){
				
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE) && defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN)
					int	err;
/* ADEBUG */ 
					WAN_TDMV_CALL(rx_dchan,
						(&card->wan_tdmv,chan->tdmv_chan,
					        wan_skb_data(new_skb),wan_skb_len(new_skb)),
						err);

					DEBUG_TEST("%s TDM DCHAN VOICE Rx Pkt Len=%i Chan=%i\n",
						card->devname,wan_skb_len(new_skb),
						chan->tdmv_chan);
#else
					DEBUG_EVENT("%s: DCHAN Rx Packet critical error TDMV not compiled!\n",card->devname);
#endif
			
					wan_skb_free(new_skb);
					/* Continue through since the above
					 * function returns void */
				
				} else {
					DEBUG_EVENT("%s: DCHAN Rx Packet critical error op not supported\n",card->devname);
					++chan->if_stats.rx_dropped;
					wan_skb_free(new_skb);
					continue;
				}				
				
			}else if (chan->common.usedby == TDM_VOICE){

				DEBUG_EVENT("%s: TDM VOICE CRITICAL: IN BH!!!!\n",card->devname);
				++chan->if_stats.rx_dropped;
				wan_skb_free(new_skb);
				continue;
				
			}else if (chan->common.usedby == STACK){

				wan_skb_set_csum(new_skb,0);

				if (wanpipe_lip_rx(chan,new_skb) != 0){
					++chan->if_stats.rx_dropped;
					wan_skb_free(new_skb);
					continue;
				}
				
			}else{
				protocol_recv(chan->card,chan,new_skb);
			}

			chan->opstats.Data_frames_Rx_count++;
			chan->opstats.Data_bytes_Rx_count+=len;
			chan->if_stats.rx_packets++;
			chan->if_stats.rx_bytes+=len;
		}

	}

	while((skb=wan_skb_dequeue(&chan->wp_rx_stack_complete_list)) != NULL){
		len=wan_skb_len(skb);
		if (wanpipe_lip_rx(chan,skb) != 0){
			++chan->if_stats.rx_dropped;
			wan_skb_free(skb);
		}else{
			chan->opstats.Data_frames_Rx_count++;
			chan->opstats.Data_bytes_Rx_count+=len;
			chan->if_stats.rx_packets++;
			chan->if_stats.rx_bytes+=len;
		}
	}

	while((skb=wan_skb_dequeue(&chan->wp_tx_complete_list)) != NULL){
		aft_tx_post_complete (chan->card,chan,skb);
		wan_skb_free(skb);
	}


	WAN_TASKLET_END((&chan->common.bh_task));
		
	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		WAN_TASKLET_END((&chan->common.bh_task));
		return;
	}

#if 1	
	{

	if ((len=wan_skb_queue_len(&chan->wp_rx_complete_list))){
		WAN_TASKLET_SCHEDULE((&chan->common.bh_task));	
	}else if ((len=wan_skb_queue_len(&chan->wp_tx_complete_list))){
		WAN_TASKLET_SCHEDULE((&chan->common.bh_task));	
        }else if ((len=wan_skb_queue_len(&chan->wp_rx_stack_complete_list))){
		WAN_TASKLET_SCHEDULE((&chan->common.bh_task));	
	}

	}
#endif

	DEBUG_TEST("%s: ------------ END -----------------: %u\n",
                        __FUNCTION__,SYSTEM_TICKS);

	return;
}

/**SECTION**************************************************
 *
 * 	Interrupt Support Functions
 *
 **********************************************************/
static void wp_aft_fifo_per_port_isr(sdla_t *card)
{
        u32 rx_status, tx_status;
	u32 i;
	private_area_t *chan;
	int num_of_logic_ch;
	u32 tmp_fifo_reg;

        /* Clear HDLC pending registers */
        __sdla_bus_read_4(card->hw, AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG),&tx_status);
	__sdla_bus_read_4(card->hw, AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG),&rx_status);

	tx_status&=card->u.aft.active_ch_map;
	rx_status&=card->u.aft.active_ch_map;

	num_of_logic_ch=card->u.aft.num_of_time_slots;

	if (!wan_test_bit(0,&card->u.aft.comm_enabled)){
		if (tx_status){
			card->wandev.stats.tx_aborted_errors++;
		}
		if (rx_status){
			card->wandev.stats.rx_over_errors++;
		}
		return;
	}
	
        if (tx_status != 0){
		for (i=0;i<num_of_logic_ch;i++){
			if (wan_test_bit(i,&tx_status) && wan_test_bit(i,&card->u.aft.logic_ch_map)){
				
				chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
				if (!chan){
					DEBUG_EVENT("Warning: ignoring tx fifo intr: no dev!\n");
					continue;
				}

				if (wan_test_bit(0,&chan->interface_down)){
					continue;
				}
#if 1
				if (!chan->hdlc_eng && !wan_test_bit(0,&chan->idle_start)){
					DEBUG_TEST("%s: Warning: ignoring tx fifo: dev idle start!\n",
                                                chan->if_name);
                                        continue;
				}
#endif
                		DEBUG_TEST("%s:%s: Warning TX Fifo Error on LogicCh=%ld Slot=%d!\n",
                           		card->devname,chan->if_name,chan->logic_ch_num,i);

#if 0				
{
				u32 dma_descr,tmp_reg;
				dma_descr=(chan->logic_ch_num<<4) + 
					   AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);


        			card->hw_iface.bus_read_4(card->hw,dma_descr, &tmp_reg);

				DEBUG_EVENT("%s:%s: Warning TX Fifo Error on LogicCh=%ld Slot=%d Reg=0x%X!\n",
                           		card->devname,chan->if_name,chan->logic_ch_num,i,tmp_reg);

#if 1
			aft_list_tx_descriptors(chan);
			aft_critical_shutdown(card);
			break;
			
#endif

}
#endif

				aft_tx_fifo_under_recover(card,chan);
				++chan->if_stats.tx_fifo_errors;
				card->wandev.stats.tx_aborted_errors++;
        			__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG),&tmp_fifo_reg);
			}
		}
        }


        if (rx_status != 0){
		for (i=0;i<num_of_logic_ch;i++){
			if (wan_test_bit(i,&rx_status) && wan_test_bit(i,&card->u.aft.logic_ch_map)){
				chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
				if (!chan){
					continue;
				}

				if (wan_test_bit(0,&chan->interface_down)){
					continue;
				}

#ifdef AFT_RX_FIFO_DEBUG				
{
				u32 dma_descr,tmp1_reg,tmp_reg,cur_dma_ptr;
				u32 dma_ram_desc=chan->logic_ch_num*4 + 
						AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		
				card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&tmp_reg);
				cur_dma_ptr=aft_dmachain_get_rx_dma_addr(tmp_reg);

				dma_descr=(chan->logic_ch_num<<4) + cur_dma_ptr*AFT_DMA_INDEX_OFFSET + 
					   AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

        			card->hw_iface.bus_read_4(card->hw,dma_descr, &tmp_reg);
        			card->hw_iface.bus_read_4(card->hw,(dma_descr-4), &tmp1_reg);


				if (wan_test_bit(AFT_RXDMA_HI_GO_BIT,&tmp_reg)){
					DEBUG_EVENT("%s: Rx Fifo Go Bit Set DMA=%d Addr=0x%X : HI=0x%08X LO=0x%08X OLO=0x%08X Cfg=0x%08X!\n",
							card->devname,
							cur_dma_ptr,
							dma_descr,
							tmp_reg,tmp1_reg,
chan->rx_dma_chain_table[chan->rx_chain_indx].dma_addr,
0);
				}

				DEBUG_EVENT("%s:%s: Warning RX Fifo Error on Ch=%ld End=%d Cur=%d: Reg=0x%X Addr=0x%X!\n",
                           		card->devname,chan->if_name,chan->logic_ch_num,
					chan->rx_chain_indx,cur_dma_ptr,tmp_reg,dma_descr);
				
}
#if 0
				aft_display_chain_history(chan);	
				aft_list_descriptors(chan);
#endif
#endif
				++chan->if_stats.rx_fifo_errors;
				++chan->if_stats.rx_over_errors;
				chan->errstats.Rx_overrun_err_count++;
				card->wandev.stats.rx_over_errors++;

				aft_rx_fifo_over_recover(card,chan);
				wan_set_bit(WP_FIFO_ERROR_BIT, &chan->pkt_error);
			
        			__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG),&tmp_fifo_reg);
#if 0
				/* Debuging Code used to stop the line in 
				 * case of fifo errors */
				aft_list_descriptors(chan);

				aft_critical_shutdown(card);    
#endif
			}
		}
        }

	return;
}


static void front_end_interrupt(sdla_t *card, unsigned long reg, int lock)
{
	if (card->wandev.fe_iface.isr){
		card->wandev.fe_iface.isr(&card->fe);

		if (lock){
			wan_smp_flag_t smp_flags;
			wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
			handle_front_end_state(card);
			wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);
		}else{
			handle_front_end_state(card);
		}
	}
	return;
}
/**SECTION***************************************************************
 *
 * 	HARDWARE Interrupt Handlers
 *
 ***********************************************************************/


/*============================================================================
 * wpfw_isr
 *
 * Main interrupt service routine.
 * Determin the interrupt received and handle it.
 *
 */
#if 0
static u32 aft_shared_irq=0;
static u32 aft_master_dev=0xF;
#endif
#if 0
static int gdma_cnt=0;
#endif

#define EC_IRQ_TIMEOUT (HZ)

static WAN_IRQ_RETVAL wp_aft_global_isr (sdla_t* card)
{
	u32 reg_sec=0,reg=0;
	u32 a108_reg=0, a56k_reg=0;
	u32 fifo_port_intr=0;
	u32 dma_port_intr=0;
	u32 wdt_port_intr=0;
	u32 tdmv_port_intr=0;
	u32 fe_intr=0;

	WAN_IRQ_RETVAL_DECL(irq_ret);
	
	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		DEBUG_TEST("%s: Card down, ignoring interrupt!!!!!!!\n",
			card->devname);	
		WAN_IRQ_RETURN(irq_ret); 
	}

#ifdef AFT_IRQ_STAT_DEBUG
	card->wandev.stats.rx_errors++;
#endif

	if (tdm_check > 0){
		card->wandev.stats.rx_errors=tdm_check;
	}

#ifdef AFT_IRQ_DEBUG
	card->wandev.stats.rx_packets++;
	if (SYSTEM_TICKS-card->u.aft.gtimeout >= HZ){
		card->wandev.stats.tx_packets=card->wandev.stats.rx_packets;
		card->wandev.stats.rx_packets=0;
		card->u.aft.gtimeout=SYSTEM_TICKS;
	}
#endif	
	
    	wan_set_bit(0,&card->in_isr);

       /* -----------------2/6/2003 9:02AM------------------
     	* Disable all chip Interrupts  (offset 0x040)
     	*  -- "Transmit/Receive DMA Engine"  interrupt disable
     	*  -- "FiFo/Line Abort Error"        interrupt disable
	* --------------------------------------------------*/
       	__sdla_bus_read_4(card->hw,AFT_CHIP_CFG_REG, &reg);
	reg_sec=reg;

       	if (wan_test_bit(AFT_CHIPCFG_FE_INTR_STAT_BIT,&reg)){

#ifdef AFT_IRQ_STAT_DEBUG
		card->wandev.stats.rx_dropped++;
#endif       

		if (wan_test_bit(AFT_CHIPCFG_FE_INTR_CFG_BIT,&reg)) {
		
			DEBUG_TEST("%s: Got Front End Interrupt 0x%08X\n",
					card->devname,reg);

			WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);

#ifdef AFT_IRQ_STAT_DEBUG
			card->wandev.stats.tx_dropped++;
#endif

			fe_intr=1;
			/*FIXME: Give Alex the reg to acknowledge remoras */
			if (card->wandev.fe_iface.check_isr &&
			    card->wandev.fe_iface.check_isr(&card->fe)){
#if defined(__LINUX__)
				wan_set_bit(AFT_FE_INTR,&card->u.aft.port_task_cmd);
				WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));	
				__aft_fe_intr_ctrl(card,0);
#else
              			front_end_interrupt(card,reg,0);
#endif
			}
		}
        }

/* New Octasic implementarion May 16 2006 */
#if defined(CONFIG_WANPIPE_HWEC)
	if (card->wandev.ec_dev && 
	    SYSTEM_TICKS-card->wandev.ec_intmask  > EC_IRQ_TIMEOUT) {
		card->wandev.ec_intmask=SYSTEM_TICKS;
		if (!wan_test_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd)){
			/* All work is done from ec_poll routine!!! */
			wan_set_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd);
			WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));	
		}
	}
#endif
	
	if (card->u.aft.firm_id == AFT_DS_FE_CORE_ID) {
		
       		__sdla_bus_read_4(card->hw,AFT_CHIP_STAT_REG, &a108_reg);
		fifo_port_intr	= aft_chipcfg_a108_get_fifo_intr_stats(a108_reg);
		dma_port_intr	= aft_chipcfg_a108_get_dma_intr_stats(a108_reg);
		wdt_port_intr	= aft_chipcfg_a108_get_wdt_intr_stats(a108_reg);
		tdmv_port_intr	= aft_chipcfg_a108_get_tdmv_intr_stats(a108_reg);	

	}else if(IS_56K_CARD(card)){
       	__sdla_bus_read_4(card->hw,AFT_CHIP_STAT_REG, &a56k_reg);
		fifo_port_intr	= wan_test_bit(AFT_CHIPCFG_A56K_FIFO_INTR_BIT,&a56k_reg);
		dma_port_intr	= wan_test_bit(AFT_CHIPCFG_A56K_DMA_INTR_BIT,&a56k_reg);
		wdt_port_intr	= wan_test_bit(AFT_CHIPCFG_A56K_WDT_INTR_BIT,&a56k_reg);
	}else{
		fifo_port_intr	= aft_chipcfg_get_hdlc_intr_stats(reg);
		dma_port_intr	= aft_chipcfg_get_dma_intr_stats(reg);
		wdt_port_intr	= aft_chipcfg_get_wdt_intr_stats(reg);
		tdmv_port_intr	= aft_chipcfg_get_tdmv_intr_stats(reg);

      		if (wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
			tdmv_port_intr&=0x01;
		}
	}

	if (tdmv_port_intr || 
	    dma_port_intr  ||
	    fifo_port_intr ||
	    dma_port_intr  ||
	    wdt_port_intr) {
         	/* Pass Through */
	} else {
		/* No more interrupts for us */
        	goto aft_global_isr_exit;
	}
	    
	if (wan_test_bit(AFT_LCFG_FIFO_INTR_BIT,&card->u.aft.lcfg_reg) && 
	    wan_test_bit(card->wandev.comm_port,&fifo_port_intr)){
#ifdef AFT_IRQ_STAT_DEBUG
		card->wandev.stats.multicast++;
#endif
		wp_aft_fifo_per_port_isr(card);
		WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);
	}

#if 1
	if (wan_test_bit(AFT_LCFG_DMA_INTR_BIT,&card->u.aft.lcfg_reg) &&
	    wan_test_bit(card->wandev.comm_port,&dma_port_intr)){
        	
		WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);
		
		wp_aft_dma_per_port_isr(card);

		/* Only enable fifo interrupts after a first
		 * successful DMA interrupt */
#ifdef AFT_IRQ_STAT_DEBUG
		card->wandev.stats.rx_length_errors++;
#endif

#if 1
		if (wan_test_bit(0,&card->u.aft.comm_enabled) && 
		    !wan_test_bit(AFT_LCFG_FIFO_INTR_BIT,&card->u.aft.lcfg_reg)){
				aft_fifo_intr_ctrl(card, 1);
		}
#else
#warning "FIFO Interrupt Disabled"
#endif
	}
#else
#warning "NCDEBUG DMA IGNORED"
#endif

      	if (wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
	       
	    if (tdmv_port_intr &&
	        !wan_test_bit(AFT_CHIPCFG_A108_A104_TDM_FIFO_SYNC_BIT,&reg)) {

		int ring_buf_enabled=wan_test_bit(AFT_CHIPCFG_A108_A104_TDM_DMA_RINGBUF_BIT,&reg);
		sdla_t	*tmp_card;
		int ring_rsync=0;
		void **card_list;
		int i;

		
		WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);

#ifdef AFT_IRQ_STAT_DEBUG
		card->wandev.stats.rx_crc_errors++;
#endif

		if (ring_buf_enabled) {
			if (card->adptr_type == A104_ADPTR_4TE1 &&
			    card->u.aft.firm_id == AFT_PMC_FE_CORE_ID) {
			        wan_set_bit(AFT_CHIPCFG_A104_TDM_ACK_BIT,&reg);  
			} else {
				wan_set_bit(AFT_CHIPCFG_A108_TDM_GLOBAL_RX_INTR_ACK,&reg);
	       			wan_set_bit(AFT_CHIPCFG_A108_TDM_GLOBAL_TX_INTR_ACK,&reg);
			}
       			__sdla_bus_write_4(card->hw,AFT_CHIP_CFG_REG,reg);    
		
			if (card->hw_iface.fe_test_bit(card->hw,1)) {
		    	    DEBUG_EVENT("%s: Global TDM Ring Resync\n",card->devname);
                    	    ring_rsync=1;
		    	    card->hw_iface.fe_clear_bit(card->hw,1);	
			}
		}
		
		card_list=__sdla_get_ptr_isr_array(card->hw);
		for (i=0;i<8;i++){
			tmp_card=(sdla_t*)card_list[i];
			if (tmp_card &&
			    tmp_card->wandev.state == WAN_CONNECTED &&
			    wan_test_bit(AFT_LCFG_TDMV_INTR_BIT,&tmp_card->u.aft.lcfg_reg)) {
#ifdef AFT_IRQ_STAT_DEBUG
				tmp_card->wandev.stats.rx_crc_errors++;
#endif

				if (ring_buf_enabled) {
					if (ring_rsync) {
						aft_tdm_ring_rsync(tmp_card);
					} else {
						tmp_card->u.aft.tdm_rx_dma_toggle++;
						if (tmp_card->u.aft.tdm_rx_dma_toggle >= AFT_TDMV_CIRC_BUF_LEN) {
							tmp_card->u.aft.tdm_rx_dma_toggle=0; 
						}
						
                                                tmp_card->u.aft.tdm_tx_dma_toggle++;
						if (tmp_card->u.aft.tdm_tx_dma_toggle >= AFT_TDMV_CIRC_BUF_LEN) {
							tmp_card->u.aft.tdm_tx_dma_toggle=0; 
						}     
					}	
				}
				
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
				if (tmp_card->wan_tdmv.sc &&
		    		    !tmp_card->u.aft.rtp_len) {
					aft_voice_span_rx_tx(tmp_card,
							     ring_buf_enabled);
				}else
#endif 
				{
       					wp_aft_tdmv_per_port_isr(tmp_card);
				}
			}
		}

		if (!ring_buf_enabled) {
			if (card->adptr_type == A104_ADPTR_4TE1 &&
			    card->u.aft.firm_id == AFT_PMC_FE_CORE_ID) {
			        wan_set_bit(AFT_CHIPCFG_A104_TDM_ACK_BIT,&reg);  
			} else {
				wan_set_bit(AFT_CHIPCFG_A108_TDM_GLOBAL_RX_INTR_ACK,&reg);
	       			wan_set_bit(AFT_CHIPCFG_A108_TDM_GLOBAL_TX_INTR_ACK,&reg);
			}
       			__sdla_bus_write_4(card->hw,AFT_CHIP_CFG_REG,reg);
		}
	    }

	} else {
		
	    if (wan_test_bit(AFT_LCFG_TDMV_INTR_BIT,&card->u.aft.lcfg_reg) &&
	        wan_test_bit(card->wandev.comm_port,&tdmv_port_intr)){
		
		WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);

#ifdef AFT_IRQ_STAT_DEBUG
		card->wandev.stats.rx_crc_errors++;
#endif
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		if (card->wan_tdmv.sc &&
		    !card->u.aft.rtp_len &&
		    card->wandev.config_id != WANCONFIG_AFT_ANALOG) {
			u32 dmareg;
		        aft_voice_span_rx_tx(card, 0);
			card->hw_iface.bus_read_4(card->hw,
					AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&dmareg);
	       		wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&dmareg);
	       		wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&dmareg);
	       		card->hw_iface.bus_write_4(card->hw,
					AFT_PORT_REG(card,AFT_DMA_CTRL_REG),dmareg);
		} else
#endif
	       	{
	                wp_aft_tdmv_per_port_isr(card);
		}
	    }
	}

	if (wan_test_bit(card->wandev.comm_port,&wdt_port_intr)){
		WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);
		wp_aft_wdt_per_port_isr(card,1);			
		card->u.aft.wdt_tx_cnt=SYSTEM_TICKS;
#ifdef AFT_IRQ_STAT_DEBUG
		card->wandev.stats.rx_fifo_errors++;
#endif
	}

#ifdef AFT_WDT_ENABLE
	else if (card->wandev.state == WAN_CONNECTED &&
		 SYSTEM_TICKS-card->u.aft.wdt_tx_cnt > (HZ>>2)){
		wp_aft_wdt_per_port_isr(card,0);
		card->u.aft.wdt_tx_cnt=SYSTEM_TICKS;
#ifdef AFT_IRQ_STAT_DEBUG
		card->wandev.stats.tx_aborted_errors++;
#endif
	}
#endif

	/* -----------------2/6/2003 10:36AM-----------------
	 *    Finish of the interupt handler
	 * --------------------------------------------------*/


#if AFT_SECURITY_CHECK

	reg=reg_sec;	
	if (wan_test_bit(AFT_CHIPCFG_SECURITY_STAT_BIT,&reg)){
		WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);
		if (++card->u.aft.chip_security_cnt > AFT_MAX_CHIP_SECURITY_CNT){
			DEBUG_EVENT("%s: Critical: AFT Chip Security Compromised: Disabling Driver!(%08X)\n",
				card->devname, reg);
			DEBUG_EVENT("%s: Please call Sangoma Tech Support (www.sangoma.com)!\n",
				card->devname);

			aft_critical_shutdown(card);
		}

	} else if (aft_hwdev[card->wandev.card_type].aft_check_ec_security(card)){
		WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);

		if (++card->u.aft.chip_security_cnt > AFT_MAX_CHIP_SECURITY_CNT){
        	DEBUG_EVENT("%s: Critical: Echo Canceller Chip Security Compromised: Disabling Driver!\n",
				card->devname);
			DEBUG_EVENT("%s: Please call Sangoma Tech Support (www.sangoma.com)!\n",
				card->devname);

			card->u.aft.chip_security_cnt=0;
			aft_critical_shutdown(card);
		}
	
        } else if (card->u.aft.firm_id == AFT_DS_FE_CORE_ID &&
	           card->wandev.state == WAN_CONNECTED &&
	           SYSTEM_TICKS-card->u.aft.sec_chk_cnt > HZ) {
		
		u32 lcfg_reg;

		card->u.aft.sec_chk_cnt=SYSTEM_TICKS;
		__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), &lcfg_reg);
		card->u.aft.lcfg_reg=lcfg_reg;
		if (wan_test_bit(AFT_LCFG_TX_FE_SYNC_STAT_BIT,&lcfg_reg) ||
		    wan_test_bit(AFT_LCFG_RX_FE_SYNC_STAT_BIT,&lcfg_reg)){	
		    if (++card->u.aft.chip_security_cnt > AFT_MAX_CHIP_SECURITY_CNT){
		        DEBUG_EVENT("%s: Critical: A108 Lost Sync with Front End: Disabling Driver (0x%08X : A108S=0x%08X)!\n",
				card->devname,
				lcfg_reg,a108_reg);
			DEBUG_EVENT("%s: Please call Sangoma Tech Support (www.sangoma.com)!\n",
				card->devname);
			
			aft_critical_shutdown(card);
		    }
		} else {
			card->u.aft.chip_security_cnt=0;
		}
        } else {
       		card->u.aft.chip_security_cnt=0; 
	}    	
#endif
	
    	DEBUG_TEST("---- ISR end.-------------------\n");

aft_global_isr_exit:
	
    	wan_clear_bit(0,&card->in_isr);
	WAN_IRQ_RETURN(irq_ret); 
}

static void wp_aft_dma_per_port_isr(sdla_t *card)
{
	int i;
	u32 dma_tx_reg,dma_rx_reg;
	private_area_t *chan;
	u32 dma_tx_voice=0;

       /* -----------------2/6/2003 9:37AM------------------
      	* Checking for Interrupt source:
      	* 1. Receive DMA Engine
      	* 2. Transmit DMA Engine
      	* 3. Error conditions.
      	* --------------------------------------------------*/

	int num_of_logic_ch;
	num_of_logic_ch=card->u.aft.num_of_time_slots;
 
        /* Zaptel optimization. Dont waist time looking at
	   channels, when we know that only a single DCHAN
	   will use this code */
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
        if (card->wan_tdmv.sc) {
		__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_DMA_INTR_PENDING_REG),&dma_rx_reg);  
	        __sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_DMA_INTR_PENDING_REG),&dma_tx_reg);	
	       	if (card->u.aft.tdmv_dchan) {
			chan=(private_area_t*)card->u.aft.dev_to_ch_map[card->u.aft.tdmv_dchan-1];
	    		if (chan && wan_test_bit(0,&chan->up)) {
                		aft_dma_rx_complete(card,chan,0);
				aft_dma_tx_complete(card,chan,0,0);   
			}
		}
		goto isr_skb_rx;
	}  
#endif	
		
	/* Receive DMA Engine */
	__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_DMA_INTR_PENDING_REG),&dma_rx_reg);

	dma_rx_reg&=card->u.aft.active_ch_map;

	if (dma_rx_reg == 0){
		goto isr_skb_rx;
	}

	dma_rx_reg &= card->u.aft.logic_ch_map;
	dma_rx_reg &= ~(card->u.aft.tdm_logic_ch_map);


	for (i=0; i<num_of_logic_ch;i++){
		if (wan_test_bit(i,&dma_rx_reg)) {

			chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
			if (!chan){
				DEBUG_EVENT("%s: Error: No Dev for Rx logical ch=%d\n",
						card->devname,i);
				continue;
			}

			if (!wan_test_bit(0,&chan->up)){
				continue;
			}

			if (chan->channelized_cfg && !chan->hdlc_eng){
				wan_set_bit(i,&dma_tx_voice);
				continue;
			}

#if 0	
			chan->if_stats.rx_frame_errors++;
#endif

			DEBUG_ISR("%s: RX Interrupt pend. \n",
					card->devname);

			aft_dma_rx_complete(card,chan,0);


#if 0
			if (chan->cfg.tdmv_master_if && !chan->tdmv_irq_cfg){
				aft_channel_rxintr_ctrl(card,chan,1);
				DEBUG_EVENT("%s: Master dev %s Synched to master irq\n",
						card->devname,chan->if_name);
				chan->tdmv_irq_cfg=1;
			}
#endif
		}
	}


isr_skb_rx:

	/* Transmit DMA Engine */
	__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_DMA_INTR_PENDING_REG),&dma_tx_reg);

	dma_tx_reg&=card->u.aft.active_ch_map;

	dma_tx_reg&=~dma_tx_voice;

	if (dma_tx_reg == 0){
		goto isr_skb_tx;
	}
	
	dma_tx_reg &= card->u.aft.logic_ch_map;
	dma_tx_reg &= ~(card->u.aft.tdm_logic_ch_map);


	for (i=0; i<num_of_logic_ch ;i++){
		if (wan_test_bit(i,&dma_tx_reg)) {

			chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
			if (!chan){
				DEBUG_EVENT("%s: Error: No Dev for Tx logical ch=%d\n",
						card->devname,i);
				continue;
			}

			if (chan->channelized_cfg && !chan->hdlc_eng){
				continue;
			}

			DEBUG_ISR("---- TX Interrupt pend. --\n");
			aft_dma_tx_complete(card,chan,0,0);
		}
	}

isr_skb_tx:
    	DEBUG_ISR("---- ISR SKB TX end.-------------------\n");


}

static void wp_aft_tdmv_per_port_isr(sdla_t *card)
{
	int i;
	private_area_t *chan;

#if 0
	DEBUG_EVENT("%s: TDMV Interrupt LogicCh=%i\n",
			card->devname,card->u.aft.num_of_time_slots);
#endif
       /* -----------------2/6/2003 9:37AM------------------
      	* Checking for Interrupt source:
      	* 1. Receive DMA Engine
      	* 2. Transmit DMA Engine
      	* 3. Error conditions.
      	* --------------------------------------------------*/


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

		if (!wan_test_bit(0,&chan->up)){
			continue;
		}


		if (chan->channelized_cfg && !chan->hdlc_eng){
			aft_dma_rx_tdmv(card,chan);
		}

#if 0	
		chan->if_stats.rx_frame_errors++;
#endif

		DEBUG_ISR("%s: RX Interrupt pend. \n",
				card->devname);

	}
}





static void wp_aft_wdt_per_port_isr(sdla_t *card, int wdt_intr)
{
	int i,wdt_disable = 0;
	int timeout=AFT_WDTCTRL_TIMEOUT;

	aft_wdt_reset(card);

	for (i=0; i<card->u.aft.num_of_time_slots;i++){

		private_area_t *chan;

		if (!wan_test_bit(i,&card->u.aft.logic_ch_map)){
			continue;
		}

		chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
		if (!chan){
			continue;
		}

		if (!wan_test_bit(0,&chan->up) || 
                    wan_test_bit(0,&chan->interface_down)){
			continue;
		}

#if 0
		if (wdt_intr){
			++chan->if_stats.tx_dropped;
		}else{
			++chan->if_stats.tx_errors;
		}
#endif

		if (card->wandev.state == WAN_CONNECTED){
			
			if (chan->single_dma_chain){
				wdt_disable=1;
				continue;
			}
#if 0
			++chan->if_stats.tx_errors;
#endif                 
			aft_dma_tx_complete (card,chan,1,0);	
		
		}
		
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		else{
			if (chan->tdmv_zaptel_cfg &&  
			    wan_test_bit(0,&chan->up) && 
			    chan->cfg.tdmv_master_if){
				/* If the line becomes disconnected 
				 * keep calling TDMV zaptel in order to 
				 * provide timing */
				if (wdt_intr && chan->cfg.tdmv_master_if){
					int    i, err;
					void **card_list=__sdla_get_ptr_isr_array(card->hw);    
					sdla_t *tmp_card;
					for (i=0;i<8;i++) {
                                        	tmp_card=card_list[i];
						if (!tmp_card) {
                                                 	continue;
						}
						if (tmp_card != card) {
							/* Only run timing from a first
							   configured single card */
							DEBUG_TEST("%s: Disabling zaptel timing ! \n",card->devname);
                                                	return; 	
						} else {
                                                 	break;
						}	
					}
#if 0
					++chan->if_stats.tx_dropped;
#endif

#if 1
					timeout=1;
					aft_wdt_set(card,timeout);
					WAN_TDMV_CALL(rx_tx_span, (card), err);
#else
#warning "NCDEBUG: rx_tx_span disabled poll"
#endif
					return;
				}
			}
		}
#endif
	}

#ifdef AFT_WDT_ENABLE
	/* Since this fucntion can be called via interrupt or
	 * via interrupt poll, only re-enable wdt interrupt
	 * if the function was called from the wdt_intr
	 * not from wdt poll */
	if (!wdt_disable && wdt_intr){
		aft_wdt_set(card,timeout);
	}
#endif

	return;

}

/**SECTION***********************************************************
 *
 * WANPIPE Debugging Interfaces
 *
 ********************************************************************/



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
#if 1
static int process_udp_mgmt_pkt(sdla_t* card, netdevice_t* dev,
				private_area_t* chan, int local_dev )
{
	unsigned short buffer_length;
	wan_udp_pkt_t *wan_udp_pkt;
	wan_trace_t *trace_info=NULL;

	wan_udp_pkt = (wan_udp_pkt_t *)chan->udp_pkt_data;

	if (wan_atomic_read(&chan->udp_pkt_len) == 0){
		return -ENODEV;
	}

	trace_info=&chan->trace_info;
	wan_udp_pkt = (wan_udp_pkt_t *)chan->udp_pkt_data;

   	{

		netskb_t *skb;

		wan_udp_pkt->wan_udp_opp_flag = 0;

		switch(wan_udp_pkt->wan_udp_command) {

		case READ_CONFIGURATION:
			wan_udp_pkt->wan_udp_return_code = 0;
			wan_udp_pkt->wan_udp_data_len=0;
			break;

		case READ_CODE_VERSION:
			wan_udp_pkt->wan_udp_return_code = 0;
			wan_udp_pkt->wan_udp_data[0]=card->u.aft.firm_ver;
			wan_udp_pkt->wan_udp_data_len=1;
			break;
	
		case AFT_LINK_STATUS:
			wan_udp_pkt->wan_udp_return_code = 0;
			if (card->wandev.state == WAN_CONNECTED){
				wan_udp_pkt->wan_udp_data[0]=1;
			}else{
				wan_udp_pkt->wan_udp_data[0]=0;
			}
			wan_udp_pkt->wan_udp_data_len=1;
			break;

		case AFT_MODEM_STATUS:
			wan_udp_pkt->wan_udp_return_code = 0;
			if (card->wandev.state == WAN_CONNECTED){
				wan_udp_pkt->wan_udp_data[0]=0x28;
			}else{
				wan_udp_pkt->wan_udp_data[0]=0;
			}
			wan_udp_pkt->wan_udp_data_len=1;
			break;

		case DIGITAL_LOOPTEST:
			wan_udp_pkt->wan_udp_return_code = 
				digital_loop_test(card,wan_udp_pkt);
			break;
				
		case READ_OPERATIONAL_STATS:
			wan_udp_pkt->wan_udp_return_code = 0;
			memcpy(wan_udp_pkt->wan_udp_data,&chan->opstats,sizeof(aft_op_stats_t));
			wan_udp_pkt->wan_udp_data_len=sizeof(aft_op_stats_t);
			break;

		case FLUSH_OPERATIONAL_STATS:
			wan_udp_pkt->wan_udp_return_code = 0;
			memset(&chan->opstats,0,sizeof(aft_op_stats_t));
			wan_udp_pkt->wan_udp_data_len=0;
			break;

		case READ_COMMS_ERROR_STATS:
			wan_udp_pkt->wan_udp_return_code = 0;
			memcpy(wan_udp_pkt->wan_udp_data,&chan->errstats,sizeof(aft_comm_err_stats_t));
			wan_udp_pkt->wan_udp_data_len=sizeof(aft_comm_err_stats_t);
			break;
	
		case FLUSH_COMMS_ERROR_STATS:
			wan_udp_pkt->wan_udp_return_code = 0;
			memset(&chan->errstats,0,sizeof(aft_comm_err_stats_t));
			wan_udp_pkt->wan_udp_data_len=0;
			break;
	

		case ENABLE_TRACING:
	
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

		case DISABLE_TRACING:
			
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

	        case GET_TRACE_INFO:

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
			wan_udp_pkt->wan_udp_atm_num_frames = 0;	
			wan_udp_pkt->wan_udp_atm_ismoredata = 0;
					
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
					wan_udp_pkt->wan_udp_atm_ismoredata = 0x01;
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
				wan_udp_pkt->wan_udp_atm_num_frames++;
			}
#elif defined(__LINUX__)
			while ((skb=skb_dequeue(&trace_info->trace_queue)) != NULL){

				if((MAX_TRACE_BUFFER - buffer_length) < wan_skb_len(skb)){
					/* indicate there are more frames on board & exit */
					wan_udp_pkt->wan_udp_atm_ismoredata = 0x01;
					if (buffer_length != 0){
						wan_skb_queue_head(&trace_info->trace_queue, skb);
					}else{
						/* If rx buffer length is greater than the
						 * whole udp buffer copy only the trace
						 * header and drop the trace packet */

						memcpy(&wan_udp_pkt->wan_udp_atm_data[buffer_length], 
							wan_skb_data(skb),
							sizeof(wan_trace_pkt_t));

						buffer_length = sizeof(wan_trace_pkt_t);
						wan_udp_pkt->wan_udp_atm_num_frames++;
						wan_skb_free(skb);	
					}
					break;
				}

				memcpy(&wan_udp_pkt->wan_udp_atm_data[buffer_length], 
				       wan_skb_data(skb),
				       wan_skb_len(skb));
		     
				buffer_length += wan_skb_len(skb);
				wan_skb_free(skb);
				wan_udp_pkt->wan_udp_atm_num_frames++;
			}
#endif                      
			/* set the data length and return code */
			wan_udp_pkt->wan_udp_data_len = buffer_length;
			wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
			break;

		case ROUTER_UP_TIME:
			wan_getcurrenttime(&chan->router_up_time, NULL);
			chan->router_up_time -= chan->router_start_time;
			*(unsigned long *)&wan_udp_pkt->wan_udp_data = 
					chan->router_up_time;	
			wan_udp_pkt->wan_udp_data_len = sizeof(unsigned long);
			wan_udp_pkt->wan_udp_return_code = 0;
			break;
	
		case WAN_GET_MEDIA_TYPE:
			if (card->wandev.fe_iface.get_fe_media){
				wan_udp_pkt->wan_udp_data[0] = 
					card->wandev.fe_iface.get_fe_media(&card->fe);
				wan_udp_pkt->wan_udp_return_code = WAN_CMD_OK;
				wan_udp_pkt->wan_udp_data_len = sizeof(unsigned char); 
			}else{
				wan_udp_pkt->wan_udp_return_code = WAN_UDP_INVALID_CMD;
			}
			break;
#if 0
		case WAN_FE_GET_STAT:
		case WAN_FE_SET_LB_MODE:
 		case WAN_FE_FLUSH_PMON:
		case WAN_FE_GET_CFG:
		case WAN_FE_SET_DEBUG_MODE:
		case WAN_FE_TX_MODE:

			if (IS_TE1_CARD(card)){
				wan_smp_flag_t smp_flags;
				card->hw_iface.hw_lock(card->hw,&smp_flags);
				card->wandev.fe_iface.process_udp(
						&card->fe, 
						&wan_udp_pkt->wan_udp_cmd,
						&wan_udp_pkt->wan_udp_data[0]);
				card->hw_iface.hw_unlock(card->hw,&smp_flags);
			}else{
				wan_udp_pkt->wan_udp_return_code = WAN_UDP_INVALID_CMD;
			}
			break;
#endif

		case WAN_GET_PROTOCOL:
		   	wan_udp_pkt->wan_udp_aft_num_frames = card->wandev.config_id;
		    	wan_udp_pkt->wan_udp_return_code = CMD_OK;
		    	wan_udp_pkt->wan_udp_data_len = 1;
		    	break;

		case WAN_GET_PLATFORM:
		    	wan_udp_pkt->wan_udp_data[0] = WAN_PLATFORM_ID;
		    	wan_udp_pkt->wan_udp_return_code = CMD_OK;
		    	wan_udp_pkt->wan_udp_data_len = 1;
		    	break;

		case WAN_GET_MASTER_DEV_NAME:
			wan_udp_pkt->wan_udp_data_len = 0;
			wan_udp_pkt->wan_udp_return_code = 0xCD;
			break;
			
		case AFT_HWEC_STATUS:
			*(unsigned long *)&wan_udp_pkt->wan_udp_data[0] = 
				IS_E1_CARD(card) ? card->wandev.ec_map:
					card->wandev.ec_map << 1;
			wan_udp_pkt->wan_udp_data_len = sizeof(unsigned long);
			wan_udp_pkt->wan_udp_return_code = 0;
			break;
	
		default:
			if ((wan_udp_pkt->wan_udp_command & 0xF0) == WAN_FE_UDP_CMD_START){
				/* FE udp calls */
				wan_smp_flag_t smp_flags,smp_flags1;
				
				card->hw_iface.hw_lock(card->hw,&smp_flags);
				wan_spin_lock_irq(&card->wandev.lock,&smp_flags1);
				card->wandev.fe_iface.process_udp(
						&card->fe, 
						&wan_udp_pkt->wan_udp_cmd,
						&wan_udp_pkt->wan_udp_data[0]);
				wan_spin_unlock_irq(&card->wandev.lock,&smp_flags1);
				card->hw_iface.hw_unlock(card->hw,&smp_flags);
				
				break;
			}
			wan_udp_pkt->wan_udp_data_len = 0;
			wan_udp_pkt->wan_udp_return_code = 0xCD;
	
			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT(
				"%s: Warning, Illegal UDP command attempted from network: %x\n",
				card->devname,wan_udp_pkt->wan_udp_command);
			}
			break;
		} /* end of switch */
     	} /* end of else */

     	/* Fill UDP TTL */
	wan_udp_pkt->wan_ip_ttl= card->wandev.ttl;

	wan_udp_pkt->wan_udp_request_reply = UDPMGMT_REPLY;
	return 1;

}
#endif



/**SECTION*************************************************************
 *
 * 	TASK Functions and Triggers
 *
 **********************************************************************/


/*============================================================================
 * port_set_state
 *
 * Set PORT state.
 *
 */
static void port_set_state (sdla_t *card, int state)
{
	struct wan_dev_le	*devle;
	netdevice_t *dev;

        if (card->wandev.state != state)
        {
#if 0
                switch (state)
                {
                case WAN_CONNECTED:
                        DEBUG_EVENT( "%s: Front End Link connected!\n",
                                card->devname);
                      	break;

                case WAN_CONNECTING:
                        DEBUG_EVENT( "%s: Front End Link connecting...\n",
                                card->devname);
                        break;

                case WAN_DISCONNECTED:
                        DEBUG_EVENT( "%s: Front End Link disconnected!\n",
                                card->devname);
                        break;
                }
#endif
                card->wandev.state = state;
		WAN_LIST_FOREACH(devle, &card->wandev.dev_head, dev_link){
			dev = WAN_DEVLE2DEV(devle);
			if (!dev) continue;
			set_chan_state(card, dev, state);
		}
        }
}

/*============================================================
 * handle_front_end_state
 *
 * Front end state indicates the physical medium that
 * the Z80 backend connects to.
 *
 * S514-1/2/3:		V32/RS232/FT1 Front End
 * 	  		Front end state is determined via
 * 	 		Modem/Status.
 * S514-4/5/7/8:	56K/T1/E1 Front End
 * 			Front end state is determined via
 * 			link status interrupt received
 * 			from the front end hardware.
 *
 * If the front end state handler is enabed by the
 * user.  The interface state will follow the
 * front end state. I.E. If the front end goes down
 * the protocol and interface will be declared down.
 *
 * If the front end state is UP, then the interface
 * and protocol will be up ONLY if the protocol is
 * also UP.
 *
 * Therefore, we must have three state variables
 * 1. Front End State (card->wandev.front_end_status)
 * 2. Protocol State  (card->wandev.state)
 * 3. Interface State (dev->flags & IFF_UP)
 *
 */

static void handle_front_end_state(void *card_id)
{
	sdla_t		*card = (sdla_t*)card_id;
	
	if (!wan_test_bit(AFT_CHIP_CONFIGURED,&card->u.aft.chip_cfg_status) &&
	    card->fe.fe_status == FE_CONNECTED) {
		DEBUG_TEST("%s: Skipping Front Front End State = %x\n",
				card->devname,card->fe.fe_status);
				
		wan_set_bit(AFT_FRONT_END_UP,&card->u.aft.chip_cfg_status);
		return;
	}

        
	if (card->wandev.ignore_front_end_status == WANOPT_YES) {
		if (card->wandev.state != WAN_CONNECTED){
			enable_data_error_intr(card);	
			port_set_state(card,WAN_CONNECTED);
			wan_set_bit(AFT_FE_LED,&card->u.aft.port_task_cmd);
		}
		return;
	}         

	if (card->fe.fe_status == FE_CONNECTED){
		if (card->wandev.state != WAN_CONNECTED){

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
				if (card->wan_tdmv.sc){
					int	err;
					WAN_TDMV_CALL(state, (card, WAN_CONNECTED), err);
				}
#endif
				/* WAN_TASKQ_SCHEDULE((&card->u.aft.port_task)); */

				if (card->tdmv_conf.span_no &&
				    !wan_test_bit(0,&card->u.aft.tdmv_master_if_up)){
					DEBUG_EVENT("%s: Skipping AFT Communication wait for MasterIF\n",
							card->devname);
					return;
				}

				enable_data_error_intr(card);
				port_set_state(card,WAN_CONNECTED);
				wan_set_bit(AFT_FE_LED,&card->u.aft.port_task_cmd);
				
		}
	}else{
		if (card->wandev.state == WAN_CONNECTED){
			port_set_state(card,WAN_DISCONNECTED);
			disable_data_error_intr(card,LINK_DOWN);

			wan_set_bit(AFT_FE_LED,&card->u.aft.port_task_cmd);
			/* We are already in the poll task here so 
			 * no need to schedule. because next check in the
			 * poll routine would be the AFT_FE_LED */
			/* WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));	*/

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
			if (card->wan_tdmv.sc){
				int	err;
				WAN_TDMV_CALL(state, (card, WAN_DISCONNECTED), err);
			}
#endif
		}
	}

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

#if 0
/*============================================================================
 * Read TE1/56K Front end registers
 */
static unsigned char
write_front_end_reg (void* card1, unsigned short off, unsigned char value)
{
	sdla_t* card = (sdla_t*)card1;

	if (card->wandev.card_type == WANOPT_AFT_ANALOG){
		DEBUG_EVENT("%s: Internal Error (%s:%d)\n",
					card->devname, __FUNCTION__,__LINE__);
		return 0x00;
	}
	return aft_hwdev[card->wandev.card_type].aft_write_fe(card1,off,value);
}

/*============================================================================
 * Read TE1/56K Front end registers
 */
static unsigned char
read_front_end_reg (void* card1, unsigned short off)
{
	sdla_t* card = (sdla_t*)card1;

	if (card->wandev.card_type == WANOPT_AFT_ANALOG){
		DEBUG_EVENT("%s: Internal Error (%s:%d)\n",
					card->devname, __FUNCTION__,__LINE__);
		return 0x00;
	}
	return aft_hwdev[card->wandev.card_type].aft_read_fe(card1,off);
}
#endif

static unsigned char
aft_write_ec (void* card1, unsigned short off, unsigned char value)
{
	DEBUG_EVENT("ADEBUG: Write Octasic Offset %04X Value %02X!\n",
					off, value);
        return 0;
}

/*============================================================================
 * Read from Octasic board
 */
static unsigned char
aft_read_ec (void* card1, unsigned short off)
{
	u8	value = 0x00;
	
	DEBUG_EVENT("ADEBUG: Read Octasic offset %04X Value %02X (temp)!\n",
					off, value);
        return value;
}



static int aft_read(sdla_t *card, wan_cmd_api_t *api_cmd)
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
			card->hw_iface.pci_read_config_dword(card->hw,
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

#if defined(DEBUG_REG)
	DEBUG_EVENT("%s: Reading Bar%d Offset=0x%X Data=%08X Len=%d\n",
				card->devname,
				api_cmd->bar,
				api_cmd->offset,
				*(u32*)&api_cmd->data[0],
				api_cmd->len);
#endif

	return 0;
}

static int aft_fe_read(sdla_t *card, wan_cmd_api_t *api_cmd)
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

static int aft_write(sdla_t *card, wan_cmd_api_t *api_cmd)
{

	if (api_cmd->len == 1){
		card->hw_iface.bus_write_1(
				card->hw,
				api_cmd->offset,
				*(u8*)&api_cmd->data[0]);
#if defined(DEBUG_REG)
		DEBUG_EVENT("%s: Write  Offset=0x%08X Data=0x%02X\n",
				card->devname,api_cmd->offset,
				*(u8*)&api_cmd->data[0]);
#endif
	}else if (api_cmd->len == 2){
		card->hw_iface.bus_write_2(
				card->hw,
				api_cmd->offset,
				*(u16*)&api_cmd->data[0]);
#if defined(DEBUG_REG)
		DEBUG_EVENT("%s: Write  Offset=0x%08X Data=0x%04X\n",
				card->devname,api_cmd->offset,
				*(unsigned short*)&api_cmd->data[0]);
#endif
	}else if (api_cmd->len == 4){
		card->hw_iface.bus_write_4(
				card->hw,
				api_cmd->offset,
				*(unsigned int*)&api_cmd->data[0]);
#if defined(DEBUG_REG)
		DEBUG_EVENT("ADEBUG: %s: Write  Offset=0x%08X Data=0x%08X\n",
			card->devname,api_cmd->offset,
			*(u32*)&api_cmd->data[0]);
#endif
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

static int aft_fe_write(sdla_t *card, wan_cmd_api_t *api_cmd)
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



static int aft_write_bios(sdla_t *card, wan_cmd_api_t *api_cmd)
{

#ifdef DEB_XILINX
	DEBUG_EVENT("Setting PCI 0xX=0x%08lX   0x3C=0x%08X\n",
			(card->wandev.S514_cpu_no[0] == SDLA_CPU_A) ? 0x10 : 0x14,
			card->u.aft.bar,card->wandev.irq);
#endif
	card->hw_iface.pci_write_config_dword(card->hw, 
			(card->wandev.S514_cpu_no[0] == SDLA_CPU_A) ? 0x10 : 0x14,
			card->u.aft.bar);
	card->hw_iface.pci_write_config_dword(card->hw, 0x3C, card->wandev.irq);
	card->hw_iface.pci_write_config_dword(card->hw, 0x0C, 0x0000ff00);

	return 0;
}

#if 0
extern int OctDrvIoctl(sdla_t*, int cmd, void*);
#endif

static int aft_hwec(sdla_t *card, wan_cmd_api_t *api_cmd)
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

static int aft_devel_ioctl(sdla_t *card, struct ifreq *ifr)
{
	wan_cmd_api_t	api_cmd;
	int 		err = -EINVAL;

	if (!ifr || !ifr->ifr_data){
		DEBUG_EVENT("%s: Error: No ifr or ifr_data\n",__FUNCTION__);
		return -EFAULT;
	}

	if (WAN_COPY_FROM_USER(&api_cmd,ifr->ifr_data,sizeof(wan_cmd_api_t))){
		return -EFAULT;
	}

	switch(api_cmd.cmd){
	case SIOC_WAN_READ_REG:
		err=aft_read(card, &api_cmd);
		break;
	case SIOC_WAN_WRITE_REG:
		err=aft_write(card, &api_cmd);
		break;

	case SIOC_WAN_FE_READ_REG:
		err=aft_fe_read(card, &api_cmd);
		break;

	case SIOC_WAN_FE_WRITE_REG:
		err=aft_fe_write(card, &api_cmd);
		break;

	case SIOC_WAN_SET_PCI_BIOS:
		err=aft_write_bios(card, &api_cmd);
		break;

	case SIOC_WAN_EC_REG:
		err = aft_hwec(card, &api_cmd);
		break;
	}
	if (WAN_COPY_TO_USER(ifr->ifr_data,&api_cmd,sizeof(wan_cmd_api_t))){
		return -EFAULT;
	}
	return err;
}


/*=========================================
 * enable_data_error_intr
 *
 * Description:
 *	
 *    Run only after the front end comes
 *    up from down state.
 *
 *    Clean the DMA Tx/Rx pending interrupts.
 *       (Ignore since we will reconfigure
 *        all dma descriptors. DMA controler
 *        was already disabled on link down)
 *
 *    For all channels clean Tx/Rx Fifo
 *
 *    Enable DMA controler
 *        (This starts the fifo cleaning
 *         process)
 *
 *    For all channels reprogram Tx/Rx DMA
 *    descriptors.
 * 
 *    Clean the Tx/Rx Error pending interrupts.
 *        (Since dma fifo's are now empty)
 *   
 *    Enable global DMA and Error interrutps.    
 *
 */

static void enable_data_error_intr(sdla_t *card)
{
	u32 reg;
	int i,err;

	DEBUG_TEST("%s: %s()\n",card->devname,__FUNCTION__);


	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);
	if (wan_test_bit(AFT_LCFG_FE_IFACE_RESET_BIT,&reg)){
		DEBUG_EVENT("%s: Warning: Skipping data enable wait for cfg!\n",
				card->devname);
		return;
	}

#if 0
	aft_list_dma_chain_regs(card);
#endif

	if (card->tdmv_conf.span_no &&
	    !wan_test_bit(0,&card->u.aft.tdmv_master_if_up)){
		DEBUG_EVENT("%s: Critical error: Enable Card while Master If Not up!\n",
				card->devname);
	}

	if (wan_test_bit(0,&card->u.aft.comm_enabled)){
		disable_data_error_intr(card,LINK_DOWN);		
	}
	
	aft_wdt_reset(card);
	
	/* Clean Tx/Rx DMA interrupts */
	card->hw_iface.bus_read_4(card->hw,
                                  AFT_PORT_REG(card,AFT_TX_DMA_INTR_PENDING_REG), 
				  &reg);
	
        card->hw_iface.bus_read_4(card->hw,
				  AFT_PORT_REG(card,AFT_RX_DMA_INTR_PENDING_REG),
                                  &reg);



	err=aft_hwdev[card->wandev.card_type].aft_test_sync(card,0);
	if (err){
		card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);
		DEBUG_EVENT("%s: Error: Front End Interface out of sync! (0x%X)\n",
				card->devname,reg);

		/*FIXME: How to recover from here, should never happen */
	}


	if (card->tdmv_conf.span_no){
        	card->hw_iface.bus_read_4(card->hw,
			AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);
	      	wan_set_bit(AFT_LCFG_TDMV_INTR_BIT,&reg);
		card->hw_iface.bus_write_4(card->hw,
				AFT_PORT_REG(card,AFT_LINE_CFG_REG), reg);        
	}
	
	for (i=0; i<card->u.aft.num_of_time_slots;i++){
		private_area_t *chan;

		if (!wan_test_bit(i,&card->u.aft.logic_ch_map)){
			continue;
		}

		chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
		if (!chan){
			continue;
		}

		if (!wan_test_bit(0,&chan->up)){
			continue;
		}


		DEBUG_TEST("%s: 1) Free Used DMA CHAINS %s\n",
				card->devname,chan->if_name);

		aft_free_rx_complete_list(chan);
		aft_free_rx_descriptors(chan);

		DEBUG_TEST("%s: 1) Free UNUSED DMA CHAINS %s\n",
				card->devname,chan->if_name);

		wan_clear_bit(TX_INTR_PENDING,&chan->dma_chain_status);

		if (chan->channelized_cfg && !chan->hdlc_eng){
			aft_tx_dma_voice_handler((unsigned long)chan,0,1);
		}else{
			aft_tx_dma_chain_handler((unsigned long)chan,0,1);
		}
	
		aft_free_tx_descriptors(chan);
		
		DEBUG_TEST("%s: 2) Init interface fifo no wait %s\n",
				card->devname,chan->if_name);

		aft_tslot_sync_ctrl(card,chan,0);
	
                aft_init_rx_dev_fifo(card, chan, WP_NO_WAIT);
                aft_init_tx_dev_fifo(card, chan, WP_NO_WAIT);

        }


        /* Enable Global DMA controler, in order to start the
         * fifo cleaning */
	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
	wan_set_bit(AFT_DMACTRL_GLOBAL_INTR_BIT,&reg);
	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);

	/* For all channels clean Tx/Rx fifos */

	for (i=0; i<card->u.aft.num_of_time_slots;i++){
		private_area_t *chan;
	
		if (!wan_test_bit(i,&card->u.aft.logic_ch_map)){
			continue;
		}

		chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
		if (!chan){
			continue;
		}

		if (!wan_test_bit(0,&chan->up)){
			continue;
		}


		DEBUG_TEST("%s: 3) Init interface fifo %s\n",
				card->devname,chan->if_name);

		aft_init_rx_dev_fifo(card, chan, WP_WAIT);
		aft_init_tx_dev_fifo(card, chan, WP_WAIT);

		DEBUG_TEST("%s: Clearing Fifo and idle_flag %s\n",
				card->devname,chan->if_name);
		wan_clear_bit(0,&chan->idle_start);
	}
#if 0
	aft_list_dma_chain_regs(card);
#endif
	/* For all channels, reprogram Tx/Rx DMA descriptors.
         * For Tx also make sure that the BUSY flag is clear
         * and previoulsy Tx packet is deallocated */

	for (i=0; i<card->u.aft.num_of_time_slots;i++){
		private_area_t *chan;
	
		if (!wan_test_bit(i,&card->u.aft.logic_ch_map)){
			continue;
		}

		chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
		if (!chan){
			continue;
		}

		if (!wan_test_bit(0,&chan->up)){
			continue;
		}

		DEBUG_TEST("%s: 4) Init interface %s\n",
				card->devname,chan->if_name);

		chan->dma_index=0;
		memset(chan->dma_history,0,sizeof(chan->dma_history));

		aft_reset_rx_chain_cnt(chan);

#if 0
		aft_list_descriptors(chan);
#endif

                aft_dma_rx(card,chan);
		aft_tslot_sync_ctrl(card,chan,1);

                DEBUG_TEST("%s: DMA RX SETUP %s\n",
                                card->devname,chan->if_name);
#if 0
		aft_list_descriptors(chan);
#endif
        }

	/* Clean Tx/Rx Error interrupts, since fifos are now
         * empty, and Tx fifo may generate an underrun which
         * we want to ignore :) */

	card->u.aft.tdm_rx_dma_toggle=0;
	card->u.aft.tdm_tx_dma_toggle=0;

	for (i=0; i<card->u.aft.num_of_time_slots;i++){
		private_area_t *chan;
	
		if (!wan_test_bit(i,&card->u.aft.logic_ch_map)){
			continue;
		}

		chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
		if (!chan){
			continue;
		}

		if (!wan_test_bit(0,&chan->up)){
			continue;
		}

		if (!chan->hdlc_eng){
			aft_reset_tx_chain_cnt(chan);
			aft_dma_tx(card,chan);
		}
	
		if (chan->cfg.ss7_enable){
			aft_clear_ss7_force_rx(card,chan);
		}

		if (chan->tdmv_zaptel_cfg && !chan->hdlc_eng){
			aft_dma_rx_tdmv(card,chan);
		} 	
	}

	card->hw_iface.bus_read_4(card->hw,
                              AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG), 
			      &reg);
	card->hw_iface.bus_read_4(card->hw,
                              AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG), 
			      &reg);


	/* Enable Global DMA and Error Interrupts */
	card->hw_iface.bus_read_4(card->hw,
				AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);
	
	wan_set_bit(AFT_LCFG_DMA_INTR_BIT,&reg);
	wan_clear_bit(AFT_LCFG_FIFO_INTR_BIT,&reg);
	if (card->tdmv_conf.span_no){
	      	wan_set_bit(AFT_LCFG_TDMV_INTR_BIT,&reg);
	}
	card->hw_iface.bus_write_4(card->hw,
				AFT_PORT_REG(card,AFT_LINE_CFG_REG), reg);

	card->u.aft.lcfg_reg=reg;

	
	wan_set_bit(0,&card->u.aft.comm_enabled);
	DEBUG_EVENT("%s: AFT communications enabled!\n",
			card->devname);

	/* Enable Channelized Driver if configured */
	if (card->tdmv_conf.span_no) {

		card->hw_iface.bus_read_4(card->hw,
                              AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG), 
			      &reg);
		card->hw_iface.bus_read_4(card->hw,
                              AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG), 
			      &reg);
     
#if 1

		
         	if (wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {

			/* Reset Global Fifo for the whole card */
			card->hw_iface.bus_read_4(card->hw,AFT_CHIP_CFG_REG,&reg);
			wan_set_bit(AFT_CHIPCFG_A108_A104_TDM_FIFO_SYNC_BIT,&reg);
	       		card->hw_iface.bus_write_4(card->hw,AFT_CHIP_CFG_REG,reg);
                	
			/* Wait for Global Fifo Reset 1ms */
			WP_DELAY(1000);
	       		
			/* Clear Global Card Fifo reset */
			wan_clear_bit(AFT_CHIPCFG_A108_A104_TDM_FIFO_SYNC_BIT,&reg);

			/* Enable TDM Quad DMA Ring buffer */
               		if (wan_test_bit(AFT_TDM_RING_BUF,&card->u.aft.chip_cfg_status)) {
				wan_set_bit(AFT_CHIPCFG_A108_A104_TDM_DMA_RINGBUF_BIT,&reg);
				card->hw_iface.fe_set_bit(card->hw,1);
			}else{
				wan_clear_bit(AFT_CHIPCFG_A108_A104_TDM_DMA_RINGBUF_BIT,&reg);
			}
	       		
#if 1
			/* Global Acknowledge TDM Interrupt  (Kickstart) */
			if (card->adptr_type == A104_ADPTR_4TE1 &&
			    card->u.aft.firm_id == AFT_PMC_FE_CORE_ID) {
			        wan_set_bit(AFT_CHIPCFG_A104_TDM_ACK_BIT,&reg);  
			} else {
				wan_set_bit(AFT_CHIPCFG_A108_TDM_GLOBAL_RX_INTR_ACK,&reg);
	       			wan_set_bit(AFT_CHIPCFG_A108_TDM_GLOBAL_TX_INTR_ACK,&reg);
			}

#endif
	       		card->hw_iface.bus_write_4(card->hw,AFT_CHIP_CFG_REG,reg);

			DEBUG_EVENT("%s: AFT Global TDM Intr\n",
					card->devname);

		} else {

	       		card->hw_iface.bus_read_4(card->hw,
					AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
	       		wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&reg);
	       		wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&reg);
	       		card->hw_iface.bus_write_4(card->hw,
					AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
			DEBUG_EVENT("%s: AFT Per Port TDM Intr\n",card->devname);
		}
		
#endif
	}
	

#ifdef AFT_WDT_ENABLE
	aft_wdt_set(card,AFT_WDTCTRL_TIMEOUT);
#endif

	DEBUG_TEST("%s: %s() end: reg=0x%X!\n"
			,card->devname,__FUNCTION__,reg);
	AFT_FUNC_DEBUG();

}

static void disable_data_error_intr(sdla_t *card, unsigned char event)
{
	u32 reg;
	
	DEBUG_TEST("%s: Event = %s\n",__FUNCTION__,
			event==DEVICE_DOWN?"Device Down": "Link Down");

	DEBUG_EVENT("%s: AFT communications disabled!\n",
			card->devname);

	aft_wdt_reset(card);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);

	wan_clear_bit(AFT_LCFG_DMA_INTR_BIT,&reg);
	wan_clear_bit(AFT_LCFG_FIFO_INTR_BIT,&reg);
	wan_clear_bit(AFT_LCFG_TDMV_INTR_BIT,&reg);

	if (event==DEVICE_DOWN){
		/* Disable Front End Interface */
		wan_set_bit(AFT_LCFG_FE_IFACE_RESET_BIT,&reg);
	}
	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), reg);
	card->u.aft.lcfg_reg=reg;

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
	wan_clear_bit(AFT_DMACTRL_GLOBAL_INTR_BIT,&reg);
	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);


	if (event==DEVICE_DOWN){
		wan_set_bit(CARD_DOWN,&card->wandev.critical);
	}else{
		if (card->tdmv_conf.span_no){
			DEBUG_EVENT("%s: Starting TDMV 1ms Timer\n",
					card->devname);
#ifdef AFT_WDT_ENABLE			
			aft_wdt_set(card,1);
#endif
		}
	}

	wan_clear_bit(0,&card->u.aft.comm_enabled);
}


/*============================================================================
 * Update communications error and general packet statistics.
 */
static int update_comms_stats(sdla_t* card)
{
	/* 1. On the first timer interrupt, update T1/E1 alarms
         * and PMON counters (only for T1/E1 card) (TE1)
         */

	 /* TE1 Update T1/E1 alarms */
        if (IS_TE1_CARD(card) && card->hw) {
	       wan_smp_flag_t smp_flags;

	       card->hw_iface.hw_lock(card->hw,&smp_flags);
	
	       if (card->wandev.fe_iface.read_alarm) {
               		card->wandev.fe_iface.read_alarm(&card->fe, 0); 
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

static void aft_rx_fifo_over_recover(sdla_t *card, private_area_t *chan)
{

	if (chan->channelized_cfg && !chan->hdlc_eng){
		return;
	}
	
#if 0
	if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s:%s Rx Fifo Recovery!\n",
				card->devname,chan->if_name);
	}
#endif

	aft_channel_rxdma_ctrl(card, chan, 0);

	aft_tslot_sync_ctrl(card,chan,0);

	aft_free_rx_complete_list(chan);
	aft_free_rx_descriptors(chan);

	aft_init_rx_dev_fifo(card, chan, WP_NO_WAIT);

	aft_channel_rxdma_ctrl(card, chan, 1);

        aft_init_rx_dev_fifo(card, chan, WP_WAIT);

	chan->dma_index=0;
	memset(chan->dma_history,0,sizeof(chan->dma_history));

	aft_reset_rx_chain_cnt(chan);
	aft_dma_rx(card,chan);

	aft_tslot_sync_ctrl(card,chan,1);

}

static void aft_tx_fifo_under_recover (sdla_t *card, private_area_t *chan)
{
	if (chan->channelized_cfg && !chan->hdlc_eng){
		return;
	}
	
#if 0
	if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s:%s Tx Fifo Recovery!\n",
				card->devname,chan->if_name);
	}
#endif
	/* Enable DMA controler, in order to start the
         * fifo cleaning */

	aft_channel_txdma_ctrl(card, chan, 0);

#if 0
	aft_list_tx_descriptors(chan);
#endif
	aft_dma_tx_complete(card,chan,0, 1);

	aft_free_tx_descriptors(chan);
	aft_init_tx_dev_fifo(card,chan,WP_NO_WAIT);

	aft_channel_txdma_ctrl(card, chan, 1);

	aft_init_tx_dev_fifo(card,chan,WP_WAIT);
	wan_clear_bit(0,&chan->idle_start);

	aft_reset_tx_chain_cnt(chan);
	aft_dma_tx(card,chan);

}

static int set_chan_state(sdla_t* card, netdevice_t* dev, int state)
{
	private_area_t *chan = wan_netif_priv(dev);
	private_area_t *ch_ptr;

	if (!chan){
                if (WAN_NET_RATELIMIT()){
                DEBUG_EVENT("%s: %s:%d No chan ptr!\n",
                               card->devname,__FUNCTION__,__LINE__);
                }	
		return -EINVAL;
	}

       	chan->common.state = state;
       	for (ch_ptr=chan; ch_ptr != NULL; ch_ptr=ch_ptr->next){
		ch_ptr->common.state=state;	 
		
		if (ch_ptr->tdmv_zaptel_cfg) {
			continue;
		}
		
		if (ch_ptr->common.usedby == TDM_VOICE_API || 
	            ch_ptr->common.usedby == TDM_VOICE_DCHAN) {
#ifdef AFT_TDM_API_SUPPORT
			if (is_tdm_api(ch_ptr,&ch_ptr->wp_tdm_api_dev)) {
				wanpipe_tdm_api_update_state(&ch_ptr->wp_tdm_api_dev, state);
			}
#endif
		}      
	}
       
	if (state == WAN_CONNECTED){
		wan_clear_bit(0,&chan->idle_start);
		WAN_NETIF_START_QUEUE(dev);
		chan->opstats.link_active_count++;
		WAN_NETIF_CARRIER_ON(dev);
		WAN_NETIF_WAKE_QUEUE(dev);
	}else{
		chan->opstats.link_inactive_modem_count++;
		WAN_NETIF_CARRIER_OFF(dev);
		WAN_NETIF_STOP_QUEUE(dev);
	}
	      
#if defined(__LINUX__)
# if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
	if (chan->common.usedby == API) {
		wan_update_api_state(chan);
	}
#endif 
#endif

	if (chan->common.usedby == STACK){
		if (state == WAN_CONNECTED){
			wanpipe_lip_connect(chan,0);
		}else{
			wanpipe_lip_disconnect(chan,0);
		}
	}
       return 0;
}



/**SECTION*************************************************************
 *
 * 	Protocol API Support Functions
 *
 **********************************************************************/


static int protocol_init (sdla_t *card, netdevice_t *dev,
		          private_area_t *chan,
			  wanif_conf_t* conf)
{

	chan->common.protocol = conf->protocol;

	DEBUG_TEST("%s: Protocol init 0x%X PPP=0x0%x FR=0x0%X\n",
			wan_netif_name(dev), chan->common.protocol,
			WANCONFIG_PPP,
			WANCONFIG_FR);

#ifndef CONFIG_PRODUCT_WANPIPE_GENERIC
	DEBUG_EVENT("%s: AFT Driver doesn't directly support any protocols!\n",
			chan->if_name);
	return -1;

#else
	if (chan->common.protocol == WANCONFIG_PPP || 
	    chan->common.protocol == WANCONFIG_CHDLC){

		struct ifreq		ifr;
		struct if_settings	ifsettings;
		
		wanpipe_generic_register(card, dev, wan_netif_name(dev));
		chan->common.prot_ptr = dev;

		if (chan->common.protocol == WANCONFIG_CHDLC){
			DEBUG_EVENT("%s: Starting Kernel CISCO HDLC protocol\n",
					chan->if_name);
			ifsettings.type = IF_PROTO_CISCO;
		}else{
			DEBUG_EVENT("%s: Starting Kernel Sync PPP protocol\n",
					chan->if_name);
			ifsettings.type = IF_PROTO_PPP;

		}
		ifr.ifr_data = (caddr_t)&ifsettings;
		if (wp_lite_set_proto(dev, &ifr)){
			wanpipe_generic_unregister(dev);
			return -EINVAL;
		}			
		
	}else if (chan->common.protocol == WANCONFIG_GENERIC){
		chan->common.prot_ptr = dev;
		
	}else{
		DEBUG_EVENT("%s:%s: Unsupported protocol %d\n",
				card->devname,chan->if_name,chan->common.protocol);
		return -EPROTONOSUPPORT;
	}
#endif
	
	return 0;
}


static int protocol_stop (sdla_t *card, netdevice_t *dev)
{
	private_area_t *chan=wan_netif_priv(dev);
	int err = 0;
	
	if (!chan)
		return 0;

	return err;
}

static int protocol_shutdown (sdla_t *card, netdevice_t *dev)
{
	private_area_t *chan=wan_netif_priv(dev);

	if (!chan)
		return 0;

#ifndef CONFIG_PRODUCT_WANPIPE_GENERIC	

	return 0;
#else
	
	if (chan->common.protocol == WANCONFIG_PPP || 
	    chan->common.protocol == WANCONFIG_CHDLC){

		chan->common.prot_ptr = NULL;
		wanpipe_generic_unregister(dev);

	}else if (chan->common.protocol == WANCONFIG_GENERIC){
		DEBUG_EVENT("%s:%s Protocol shutdown... \n",
				card->devname, chan->if_name);
	}
#endif
	return 0;
}

void protocol_recv(sdla_t *card, private_area_t *chan, netskb_t *skb)
{

#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC
	if (chan->common.protocol == WANCONFIG_PPP || 
	    chan->common.protocol == WANCONFIG_CHDLC){
		wanpipe_generic_input(chan->common.dev, skb);
		return 0;
	}
	
#if defined(__LINUX__) 
	if (chan->common.protocol == WANCONFIG_GENERIC){
		skb->protocol = htons(ETH_P_HDLC);
		skb->dev = chan->common.dev;
		skb->mac.raw  = wan_netif_data(skb);
		netif_rx(skb);
		return 0;
	}
#endif

#endif

#if defined(__LINUX__)
	skb->protocol = htons(ETH_P_IP);
	skb->dev = chan->common.dev;
	skb->mac.raw  = wan_skb_data(skb);
	netif_rx(skb);
#else
	DEBUG_EVENT("%s: Action not supported (IP)!\n",
				card->devname);
	wan_skb_free(skb);
#endif

	return;
}


/**SECTION*************************************************************
 *
 * 	TE1 Config Code
 *
 **********************************************************************/
static int aft_global_chip_configuration(sdla_t *card, wandev_conf_t* conf)
{
	int err=0;
	
	err = aft_hwdev[card->wandev.card_type].aft_global_chip_config(card);
	return err;
}

static int aft_global_chip_disable(sdla_t *card)
{

	aft_hwdev[card->wandev.card_type].aft_global_chip_unconfig(card);

	return 0;	
}

/*=========================================================
 * aft_chip_configure
 *
 */

static int aft_chip_configure(sdla_t *card, wandev_conf_t* conf)
{

	return aft_hwdev[card->wandev.card_type].aft_chip_config(card);
}

static int aft_chip_unconfigure(sdla_t *card)
{
	u32 reg=0;

	AFT_FUNC_DEBUG();

	wan_set_bit(CARD_DOWN,&card->wandev.critical);

	aft_hwdev[card->wandev.card_type].aft_chip_unconfig(card);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG),&card->u.aft.lcfg_reg);
	card->u.aft.lcfg_reg=reg;
	return 0;
}


static int aft_dev_configure(sdla_t *card, private_area_t *chan, wanif_conf_t* conf)
{
	chan->logic_ch_num=-1;
		
	/* Channel definition section. If not channels defined
	 * return error */
	if (chan->time_slot_map == 0){
		DEBUG_EVENT("%s: Invalid Channel Selection 0x%lX\n",
				card->devname,chan->time_slot_map);
		return -EINVAL;
	}


	DEBUG_EVENT("%s:    Active Ch Map :0x%08lX\n",
		card->devname,chan->time_slot_map);


	DEBUG_TEST("%s:%d: GOT Logic ch %ld Base 0x%X  Size=0x%X\n",
		__FUNCTION__,__LINE__,chan->logic_ch_num,
		chan->fifo_base_addr, chan->fifo_size_code);


	return aft_hwdev[card->wandev.card_type].aft_chan_config(card,chan);
}

static void aft_dev_unconfigure(sdla_t *card, private_area_t *chan)
{
	aft_hwdev[card->wandev.card_type].aft_chan_unconfig(card,chan);
	return ;
}


#define BIT_DEV_ADDR_CLEAR      0x600



/**SECTION*************************************************************
 *
 * 	TE1 Tx Functions
 * 	DMA Chains
 *
 **********************************************************************/


/*===============================================
 * aft_tx_dma_voice_handler
 *
 */
static void aft_tx_dma_voice_handler(unsigned long data, int wdt, int reset)
{
	private_area_t *chan = (private_area_t *)data;
	sdla_t *card = chan->card;
	u32 reg,dma_descr,dma_status;
	aft_dma_chain_t *dma_chain;

	if (wan_test_and_set_bit(TX_HANDLER_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);
		return;
	}

	dma_chain = &chan->tx_dma_chain_table[0];

	if (reset){
		wan_clear_bit(0,&dma_chain->init);
		dma_descr=(chan->logic_ch_num<<4) + AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);
		card->hw_iface.bus_write_4(card->hw,dma_descr,0);
		goto aft_tx_dma_voice_handler_exit;
	}

	/* If the current DMA chain is in use,then
	 * all chains are busy */
	if (!wan_test_bit(0,&dma_chain->init)){
		goto aft_tx_dma_voice_handler_exit;
	}

	dma_descr=(chan->logic_ch_num<<4) + AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);

	/* If GO bit is set, then the current DMA chain
	 * is in process of being transmitted, thus
	 * all are busy */
	if (wan_test_bit(AFT_TXDMA_HI_GO_BIT,&reg)){
		goto aft_tx_dma_voice_handler_exit;
	}

	dma_status = aft_txdma_hi_get_dma_status(reg);

	if (reg & AFT_TXDMA_HI_DMA_LENGTH_MASK){
		chan->errstats.Tx_dma_len_nonzero++;
		chan->errstats.Tx_dma_errors++;
	}
	
	if (dma_status){

		DEBUG_TEST("%s:%s: Tx DMA Descriptor=0x%X\n",
			card->devname,chan->if_name,reg);

 
    		/* Checking Tx DMA PCI error status. Has to be '0's */
		if (dma_status){
                	     	
			chan->errstats.Tx_pci_errors++;
			if (wan_test_bit(AFT_TXDMA_HIDMASTATUS_PCI_M_ABRT,&dma_status)){
				if (WAN_NET_RATELIMIT()){
        			DEBUG_EVENT("%s:%s: Tx Error: Abort from Master: pci fatal error!\n",
                	     		card->devname,chan->if_name);
				}
				
			}
			if (wan_test_bit(AFT_TXDMA_HIDMASTATUS_PCI_T_ABRT,&dma_status)){
				if (WAN_NET_RATELIMIT()){
        			DEBUG_EVENT("%s:%s: Tx Error: Abort from Target: pci fatal error!\n",
                	     		card->devname,chan->if_name);
				}
			}
			if (wan_test_bit(AFT_TXDMA_HIDMASTATUS_PCI_DS_TOUT,&dma_status)){
        			DEBUG_TEST("%s:%s: Tx Warning: PCI Latency Timeout!\n",
                	     		card->devname,chan->if_name);
				chan->errstats.Tx_pci_latency++;
				goto aft_tx_dma_voice_handler_exit;
			}
			if (wan_test_bit(AFT_TXDMA_HIDMASTATUS_PCI_RETRY,&dma_status)){
				if (WAN_NET_RATELIMIT()){
        			DEBUG_EVENT("%s:%s: Tx Error: 'Retry' exceeds maximum (64k): pci fatal error!\n",
                	     		card->devname,chan->if_name);
				}
			}
		}
		chan->if_stats.tx_errors++;
	}


	chan->opstats.Data_frames_Tx_count++;
	chan->opstats.Data_bytes_Tx_count+=wan_skb_len(dma_chain->skb);
	chan->if_stats.tx_packets++;
	chan->if_stats.tx_bytes+=wan_skb_len(dma_chain->skb);

	wan_clear_bit(0,&dma_chain->init);

aft_tx_dma_voice_handler_exit:
	wan_clear_bit(TX_HANDLER_BUSY,&chan->dma_status);

	return;	
}



/*===============================================
 * aft_tx_dma_chain_handler
 *
 */
static void aft_tx_dma_chain_handler(unsigned long data, int wdt, int reset)
{
	private_area_t *chan = (private_area_t *)data;
	sdla_t *card = chan->card;
	u32 reg,dma_descr;
	aft_dma_chain_t *dma_chain;

	if (wan_test_and_set_bit(TX_HANDLER_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);
		return;
	}

	dma_chain = &chan->tx_dma_chain_table[chan->tx_pending_chain_indx];
	
	for (;;){

		/* If the current DMA chain is in use,then
		 * all chains are busy */
		if (!wan_test_bit(0,&dma_chain->init)){
			break;
		}

		dma_descr=(chan->logic_ch_num<<4) + (chan->tx_pending_chain_indx*AFT_DMA_INDEX_OFFSET) + 
				AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);
		
		card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);

		/* If GO bit is set, then the current DMA chain
		 * is in process of being transmitted, thus
		 * all are busy */
		if (wan_test_bit(AFT_TXDMA_HI_GO_BIT,&reg)){
			break;
		}

		if (!wan_test_bit(AFT_TXDMA_HI_INTR_DISABLE_BIT,&reg)){
			wan_clear_bit(TX_INTR_PENDING,&chan->dma_chain_status);	
			if (wdt){
				DEBUG_TEST("%s:%s TX WDT Timer got Interrtup pkt!\n",
						card->devname,chan->if_name);
			}
		}

	
		DEBUG_TEST("%s: TX DMA Handler Chain %d\n",chan->if_name,dma_chain->index);

		if (chan->hdlc_eng){
			if (dma_chain->skb){
				wan_skb_set_csum(dma_chain->skb, reg);
				wan_skb_queue_tail(&chan->wp_tx_complete_list,dma_chain->skb);	
				dma_chain->skb=NULL;
			}
		}else{
			if (dma_chain->skb != chan->tx_idle_skb){

				wan_skb_set_csum(dma_chain->skb, reg);
				aft_tx_post_complete(chan->card,chan,dma_chain->skb);

				if (chan->channelized_cfg && !chan->hdlc_eng){
					/* Voice code uses the rx buffer to
					 * transmit! So put the rx buffer back
					 * into the rx queue */
					aft_init_requeue_free_skb(chan, dma_chain->skb);
				}else{
					wan_skb_free(dma_chain->skb);
				}

				dma_chain->skb=NULL;
			}
		}

		aft_tx_dma_chain_init(chan,dma_chain);

		if (chan->single_dma_chain){
			break;
		}
		
		if (++chan->tx_pending_chain_indx >= MAX_AFT_DMA_CHAINS){
			chan->tx_pending_chain_indx=0;
		}
		
		dma_chain = &chan->tx_dma_chain_table[chan->tx_pending_chain_indx];
		
	}

	wan_clear_bit(TX_HANDLER_BUSY,&chan->dma_status);

	if (wan_skb_queue_len(&chan->wp_tx_complete_list)){
		WAN_TASKLET_SCHEDULE((&chan->common.bh_task));	
	}

	return;	
}

/*===============================================
 * aft_dma_chain_tx
 *
 */
static int aft_dma_chain_tx(aft_dma_chain_t *dma_chain,private_area_t *chan, int intr,int fifo)
{

#define dma_descr   dma_chain->dma_descr
#define reg	    dma_chain->reg
#define dma_ch_indx dma_chain->index	
#define len_align   dma_chain->len_align	
#define card	    chan->card

	unsigned int len = dma_chain->dma_len;
	unsigned int ss7_ctrl=0;
	
	dma_descr=(chan->logic_ch_num<<4) + (dma_ch_indx*AFT_DMA_INDEX_OFFSET) + 
		  AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);

	DEBUG_DMA("%s:%d: chan logic ch=%ld chain=%d dma_descr=0x%x set!\n",
                    __FUNCTION__,__LINE__,chan->logic_ch_num,dma_ch_indx,dma_descr);

	card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);

	if (wan_test_bit(AFT_TXDMA_HI_GO_BIT,&reg)){
		DEBUG_EVENT("%s: Error: TxDMA GO Ready bit set on dma (chain=0x%X) Desc=0x%X Tx 0x%X\n",
				card->devname,dma_descr,dma_ch_indx,reg);
                /* Nothing we can do here, just reset
                   descriptor and keep going */
                card->hw_iface.bus_write_4(card->hw,dma_descr,0);
                card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);
	}
	
	dma_descr=(chan->logic_ch_num<<4) + (dma_ch_indx*AFT_DMA_INDEX_OFFSET) + 
		   AFT_PORT_REG(card,AFT_TX_DMA_LO_DESCR_BASE_REG);

	/* Write the pointer of the data packet to the
	 * DMA address register */
	reg=dma_chain->dma_addr;

	
	if (chan->cfg.ss7_enable){
		ss7_ctrl=wan_skb_csum(dma_chain->skb);
		wan_skb_set_csum(dma_chain->skb,0);
		if (ss7_ctrl&AFT_SS7_CTRL_LEN_MASK){
			len-=4;
			len+=ss7_ctrl&AFT_SS7_CTRL_LEN_MASK;
		}
		if (!wan_test_bit(AFT_SS7_CTRL_TYPE_BIT,&ss7_ctrl)){
			/*FISU*/
			if (chan->cfg.ss7_mode == WANOPT_SS7_MODE_4096){
				len-=WANOPT_SS7_FISU_4096_SZ;
			}else{
				len-=WANOPT_SS7_FISU_128_SZ;
			}
		}else{
			/*LSSU*/
			len-=chan->cfg.ss7_lssu_size;
		}
	}

	/* Set the 32bit alignment of the data length.
	 * Used to pad the tx packet to the 32 bit
	 * boundary */
	aft_txdma_lo_set_alignment(&reg,len);

	len_align=0;
	if (len&0x03){
		len_align=1;
	}

	DEBUG_DMA("%s: TXDMA_LO=0x%X PhyAddr=0x%X DmaDescr=0x%X Len=%i\n",
			__FUNCTION__,reg,(int)dma_chain->dma_addr,dma_descr,len);

	card->hw_iface.bus_write_4(card->hw,dma_descr,reg);

	dma_descr=(chan->logic_ch_num<<4) + (dma_ch_indx*AFT_DMA_INDEX_OFFSET) + 
		  AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);

	reg=0;

	if (chan->cfg.ss7_enable){
		if (wan_test_bit(AFT_SS7_CTRL_TYPE_BIT,&ss7_ctrl)){
			wan_set_bit(AFT_TXDMA_HI_SS7_FISU_OR_LSSU_BIT,&reg);
		}else{
			wan_clear_bit(AFT_TXDMA_HI_SS7_FISU_OR_LSSU_BIT,&reg);
		}
		if (wan_test_bit(AFT_SS7_CTRL_FORCE_BIT,&ss7_ctrl)){
			wan_set_bit(AFT_TXDMA_HI_SS7_FI_LS_FORCE_TX_BIT,&reg);
		}else{
			wan_clear_bit(AFT_TXDMA_HI_SS7_FI_LS_FORCE_TX_BIT,&reg);
		}
	}
	
	aft_txdma_hi_set_dma_length(&reg,len,len_align);
	
	if (chan->single_dma_chain){
		wan_clear_bit(AFT_TXDMA_HI_LAST_DESC_BIT,&reg);
		wan_clear_bit(AFT_TXDMA_HI_INTR_DISABLE_BIT,&reg);

		if (chan->channelized_cfg && !chan->hdlc_eng){
			wan_set_bit(AFT_TXDMA_HI_LAST_DESC_BIT,&reg);
		}
	}else{
		wan_set_bit(AFT_TXDMA_HI_LAST_DESC_BIT,&reg);

		if (intr){
			DEBUG_TEST("%s: Setting Interrupt on index=%d\n",
					chan->if_name,dma_ch_indx);
			wan_clear_bit(AFT_TXDMA_HI_INTR_DISABLE_BIT,&reg);
		}else{
			wan_set_bit(AFT_TXDMA_HI_INTR_DISABLE_BIT,&reg);
		}
	}
	
	if (chan->hdlc_eng){
		/* Only enable the Frame Start/Stop on
                 * non-transparent hdlc configuration */
		wan_set_bit(AFT_TXDMA_HI_START_BIT,&reg);
		wan_set_bit(AFT_TXDMA_HI_EOF_BIT,&reg);
	}else{
		/* Used for transparent time slot 
		 * synchronization */
		
		if (chan->tslot_sync){
			wan_set_bit(AFT_TXDMA_HI_START_BIT,&reg);
		}
	}

	wan_set_bit(AFT_TXDMA_HI_GO_BIT,&reg);
	if (fifo){
		/* Clear fifo command */
		wan_set_bit(AFT_TXDMA_HI_DMA_CMD_BIT,&reg);
	}

	DEBUG_DMA("%s: TXDMA_HI=0x%X DmaDescr=0x%X Len=%d Intr=%d\n",
			__FUNCTION__,reg,dma_descr,len,intr);

	card->hw_iface.bus_write_4(card->hw,dma_descr,reg);

#if 1
	++chan->tx_attempts;
#endif

	return 0;

#undef dma_descr  
#undef reg	 
#undef dma_ch_indx
#undef len_align 
#undef card
}

/*===============================================
 * aft_dma_chain_init
 *
 */
static void aft_tx_dma_chain_init(private_area_t *chan, aft_dma_chain_t *dma_chain)
{

	if (dma_chain->dma_addr){
		chan->card->hw_iface.pci_unmap_dma(chan->card->hw,
			 dma_chain->dma_addr-dma_chain->dma_offset,
	 		 dma_chain->dma_map_len,
	 		 PCI_DMA_TODEVICE);
	}

	if (dma_chain->skb){
		if (!chan->hdlc_eng){
			if (dma_chain->skb != chan->tx_idle_skb){
				if (chan->channelized_cfg && !chan->hdlc_eng){
					aft_init_requeue_free_skb(chan, dma_chain->skb);
				}else{
					wan_skb_free(dma_chain->skb);
				}
			}
			dma_chain->skb=NULL;
		}else{
			wan_skb_free(dma_chain->skb);
			dma_chain->skb=NULL;
		}
	}
	
	dma_chain->dma_addr=0;
	dma_chain->dma_len=0;
	dma_chain->dma_map_len=dma_chain->dma_len;
			
	wan_clear_bit(0,&dma_chain->init);
}

static void aft_rx_dma_chain_init(private_area_t *chan, aft_dma_chain_t *dma_chain)
{

	if (dma_chain->dma_addr){
		chan->card->hw_iface.pci_unmap_dma(chan->card->hw,
			 dma_chain->dma_addr-dma_chain->dma_offset,
	 		 dma_chain->dma_map_len,
	 		 PCI_DMA_FROMDEVICE);
	}

	if (dma_chain->skb){
		aft_init_requeue_free_skb(chan,dma_chain->skb);
		dma_chain->skb=NULL;
	}
	
	dma_chain->dma_addr=0;
	dma_chain->dma_len=0;
	dma_chain->dma_map_len=0;
			
	wan_clear_bit(0,&dma_chain->init);
}



static int aft_dma_voice_tx(sdla_t *card, private_area_t *chan)
{
	int err=0;
	aft_dma_chain_t *dma_chain;
	u32 reg, dma_ram_desc;

	if (wan_test_and_set_bit(TX_DMA_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);
		
		return -EBUSY;
	}
	
	dma_chain = &chan->tx_dma_chain_table[0];

	/* If the current DMA chain is in use,then
	 * all chains are busy */
	if (wan_test_and_set_bit(0,&dma_chain->init)){
		err=-EBUSY;
		goto aft_dma_voice_tx_exit;
	}

	if (!dma_chain->skb){
		unsigned char *buf;
		
		/* Take already preallocated buffer from rx queue.
		 * We are only using a single buffer for rx and tx */
		dma_chain->skb=wan_skb_dequeue(&chan->wp_rx_free_list);
		if (!dma_chain->skb){
			DEBUG_EVENT("%s: Warning Tx chain = %d: no free tx bufs\n",
					chan->if_name,dma_chain->index);
			wan_clear_bit(0,&dma_chain->init);
			err=-EINVAL;
			goto aft_dma_voice_tx_exit;
		}

		wan_skb_init(dma_chain->skb,16);
		wan_skb_trim(dma_chain->skb,0);

		/*NC: We must set the initial value of the
		 *    frist DMA TX transfer to 2*MTU.  This is
		 *    to avoid potential Tx FIFO underrun.
		 *    
		 *    This is equivalent of transmitting twice
		 *    very fist time. */
	
		
		buf=wan_skb_put(dma_chain->skb,chan->mtu*2);
		memset(buf,chan->idle_flag,chan->mtu*2);

		dma_chain->dma_addr = card->hw_iface.pci_map_dma(card->hw,
                                           wan_skb_data(dma_chain->skb),
					   chan->dma_mru,
					   PCI_DMA_TODEVICE);
		
        	if (dma_chain->dma_addr & AFT_TDMV_BUF_MASK) {
                	dma_chain->dma_offset = 
				AFT_TDMV_BUF_MASK - (dma_chain->dma_addr & AFT_TDMV_BUF_MASK) + 1;

			dma_chain->dma_virt = wan_skb_data(dma_chain->skb) + dma_chain->dma_offset;
			dma_chain->dma_addr += dma_chain->dma_offset;

		} else {
                       dma_chain->dma_offset=0; 
                       dma_chain->dma_virt = wan_skb_data(dma_chain->skb);
		}               	
		
		dma_chain->dma_len = wan_skb_len(dma_chain->skb);
		dma_chain->dma_map_len=chan->dma_mru;

	}


	dma_ram_desc=chan->logic_ch_num*4 + 
			AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	aft_dmachain_set_tx_dma_addr(&reg,1);	
	card->hw_iface.bus_write_4(card->hw,dma_ram_desc,reg);
	
	/* We set inital TX DMA with FIFO Reset option. This funciton
	 * will ONLY run once in TDM mode. After the inital TX the
	 * DMA Reload will be used to tx the Voice frame */
	err=aft_dma_chain_tx(dma_chain,chan,1,1);
	if (err){
		DEBUG_EVENT("%s: Tx dma chain %d overrun error: should never happen!\n",
				chan->if_name,dma_chain->index);

		/* Drop the tx packet here */
		aft_tx_dma_chain_init(chan,dma_chain);
		chan->if_stats.tx_errors++;
		err=-EINVAL;
		goto aft_dma_voice_tx_exit;
	}

aft_dma_voice_tx_exit:
	wan_clear_bit(TX_DMA_BUSY,&chan->dma_status);

	return 0;
}

/*===============================================
 * aft_dma_tx
 *
 */
static int aft_dma_tx (sdla_t *card,private_area_t *chan)
{
	int err=0, intr=0, cnt=0;
	aft_dma_chain_t *dma_chain;
	netskb_t *skb=NULL;

	if (chan->channelized_cfg && !chan->hdlc_eng){
		return aft_dma_voice_tx(card,chan);
	}
	
	if (wan_test_and_set_bit(TX_DMA_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);
		
		return -EBUSY;
	}

	if (chan->tx_chain_indx >= MAX_AFT_DMA_CHAINS){
		DEBUG_EVENT("%s: MAJOR ERROR: TE1 Tx: Dma tx chain = %d\n",
				chan->if_name,chan->tx_chain_indx);
		return -EBUSY;
	}


	/* The cnt is not used, its used only as a
	 * sanity check. The code below should break
	 * out of the loop before MAX_TX_BUF runs out! */
	for (cnt=0;cnt<MAX_AFT_DMA_CHAINS;cnt++){
		
		dma_chain = &chan->tx_dma_chain_table[chan->tx_chain_indx];

		/* FIXME: Reset TX Watchdog */
		/* aft_reset_tx_watchdog(card); */

		/* If the current DMA chain is in use,then
		 * all chains are busy */
		if (wan_test_and_set_bit(0,&dma_chain->init)){
			wan_clear_bit(TX_DMA_BUSY,&chan->dma_status);
			break;
		}

		if(!chan->lip_atm){
			skb=wan_skb_dequeue(&chan->wp_tx_pending_list);
		}else{
			skb=atm_tx_skb_dequeue(&chan->wp_tx_pending_list, chan->tx_idle_skb, chan->if_name);
		}

		if (!skb){
			if (!chan->hdlc_eng){
				
				skb=chan->tx_idle_skb;
				if (!skb){
					if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: Critical Error: Transparent tx no skb!\n",
							chan->if_name);
					}
					break;
				}
				
				++chan->if_stats.tx_carrier_errors;
			}else{
				wan_clear_bit(0,&dma_chain->init);
				wan_clear_bit(TX_DMA_BUSY,&chan->dma_status);
				break;
			}
		}

		if ((unsigned long)wan_skb_data(skb) & 0x03){
			
			err=aft_realign_skb_pkt(chan,skb);
			if (err){
				if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: Tx Error: Non Aligned packet %p: dropping...\n",
						chan->if_name,wan_skb_data(skb));
				}
				wan_skb_free(skb);
				aft_tx_dma_chain_init(chan,dma_chain);
				chan->if_stats.tx_errors++;	
				chan->opstats.Tx_Data_discard_lgth_err_count++;
				continue;
			}
		}
		
		
		if (!chan->hdlc_eng && (wan_skb_len(skb)%4)){
			
			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: Tx Error: Tx Length %i not 32bit aligned: dropping...\n",
						chan->if_name,wan_skb_len(skb));
			}
			wan_skb_free(skb);
			aft_tx_dma_chain_init(chan,dma_chain);
			chan->if_stats.tx_errors++;	
			chan->opstats.Tx_Data_discard_lgth_err_count++;
			continue;
		}
			
		dma_chain->skb=skb;
		
		dma_chain->dma_addr = card->hw_iface.pci_map_dma(card->hw,
						wan_skb_data(dma_chain->skb),
						wan_skb_len(dma_chain->skb),
						PCI_DMA_TODEVICE); 	

		dma_chain->dma_len = wan_skb_len(dma_chain->skb);
		dma_chain->dma_map_len=dma_chain->dma_len;
		dma_chain->dma_offset=0;
		
	
		DEBUG_TEST("%s: DMA Chain %d:  Cur=%d Pend=%d\n",
				chan->if_name,dma_chain->index,
				chan->tx_chain_indx,chan->tx_pending_chain_indx);

		intr=0;
		if (!wan_test_bit(TX_INTR_PENDING,&chan->dma_chain_status)){
			int pending_indx=chan->tx_pending_chain_indx;
			if (chan->tx_chain_indx >= pending_indx){
				intr = ((MAX_AFT_DMA_CHAINS-(chan->tx_chain_indx - 
							     pending_indx))<=2);
			}else{
				intr = ((pending_indx - chan->tx_chain_indx)<=2);
			}

			if (intr){
				DEBUG_TEST("%s: Setting tx interrupt on chain=%d\n",
						chan->if_name,dma_chain->index);
				wan_set_bit(TX_INTR_PENDING,&chan->dma_chain_status);
			}
		}
			

		err=aft_dma_chain_tx(dma_chain,chan,intr,0);
		if (err){
			DEBUG_EVENT("%s: Tx dma chain %d overrun error: should never happen!\n",
					chan->if_name,dma_chain->index);

			
#if 0
			aft_list_tx_descriptors(chan);
			aft_critical_shutdown(card);
			break;
#endif

			/* Drop the tx packet here */
			aft_tx_dma_chain_init(chan,dma_chain);
			chan->if_stats.tx_errors++;
			break;
		}
		
		if (skb){
			wan_capture_trace_packet(card, &chan->trace_info, skb, TRC_OUTGOING_FRM);
		}

		if (chan->single_dma_chain){
			break;
		}else{
			if (++chan->tx_chain_indx >= MAX_AFT_DMA_CHAINS){
				chan->tx_chain_indx=0;
			}
		}
	}
	
	wan_clear_bit(TX_DMA_BUSY,&chan->dma_status);

	return 0;
}



/**SECTION*************************************************************
 *
 * 	TE1 Rx Functions
 * 	DMA Chains
 *
 **********************************************************************/

static int aft_dma_chain_rx(aft_dma_chain_t *dma_chain, private_area_t *chan, int intr, int fifo)
{
#define dma_descr   dma_chain->dma_descr
#define reg	    dma_chain->reg
#define len	    dma_chain->dma_len
#define dma_ch_indx dma_chain->index	
#define len_align   dma_chain->len_align	
#define card	    chan->card

	/* Write the pointer of the data packet to the
	 * DMA address register */
	reg=dma_chain->dma_addr;

	/* Set the 32bit alignment of the data length.
	 * Since we are setting up for rx, set this value
	 * to Zero */
	aft_rxdma_lo_set_alignment(&reg,0);

    	dma_descr=(chan->logic_ch_num<<4) + (dma_ch_indx*AFT_DMA_INDEX_OFFSET) + 
		   AFT_PORT_REG(card,AFT_RX_DMA_LO_DESCR_BASE_REG);

	DEBUG_DMA("%s: RxDMA_LO(%ld) = 0x%X, DmaDescr=0x%X\n",
		__FUNCTION__,chan->logic_ch_num,reg,dma_descr);

	card->hw_iface.bus_write_4(card->hw,dma_descr,reg);

	dma_descr=(chan->logic_ch_num<<4) + (dma_ch_indx*AFT_DMA_INDEX_OFFSET) + 
		  AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

    	reg =0;

	if (chan->single_dma_chain){
		wan_clear_bit(AFT_RXDMA_HI_LAST_DESC_BIT,&reg);
		wan_clear_bit(AFT_RXDMA_HI_INTR_DISABLE_BIT,&reg);

		if (chan->channelized_cfg && !chan->hdlc_eng){
			wan_set_bit(AFT_RXDMA_HI_LAST_DESC_BIT,&reg);
		}
		
	}else{
		wan_set_bit(AFT_RXDMA_HI_LAST_DESC_BIT,&reg);
		
#if AFT_IFT_INTR_ENABLE
		wan_set_bit(AFT_RXDMA_HI_IFT_INTR_ENB_BIT,&reg);
#else
		wan_clear_bit(AFT_RXDMA_HI_IFT_INTR_ENB_BIT,&reg);
#endif
		
		if (intr){
			DEBUG_TEST("%s: Setting Rx Interrupt on index=%d\n",
					chan->if_name,dma_ch_indx);
			wan_clear_bit(AFT_RXDMA_HI_INTR_DISABLE_BIT,&reg);
		}else{
			wan_set_bit(AFT_RXDMA_HI_INTR_DISABLE_BIT,&reg);
		}
	}

	if (chan->hdlc_eng){
		aft_rxdma_hi_set_dma_length(&reg,chan->dma_mru,1);
	}else{
		aft_rxdma_hi_set_dma_length(&reg,chan->mru,0);
	}

	wan_set_bit(AFT_RXDMA_HI_GO_BIT,&reg);
	if (fifo){
		wan_set_bit(AFT_RXDMA_HI_DMA_CMD_BIT,&reg);
	}

	DEBUG_DMA("%s: RXDMA_HI(%ld) = 0x%X, DmaDescr=0x%X\n",
 	             __FUNCTION__,chan->logic_ch_num,reg,dma_descr);

	card->hw_iface.bus_write_4(card->hw,dma_descr,reg);

	return 0;

#undef dma_descr  
#undef reg	 
#undef len	 
#undef dma_ch_indx
#undef len_align 
#undef card
}

static int aft_dma_voice_rx(sdla_t *card, private_area_t *chan)
{
	int err=0;
	aft_dma_chain_t *dma_chain;
	u32 reg, dma_ram_desc;

	if (wan_test_and_set_bit(RX_DMA_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);
		return -EBUSY;
	}

	dma_chain = &chan->rx_dma_chain_table[0];

	
	/* If the current DMA chain is in use,then
	 * all chains are busy */
	if (wan_test_and_set_bit(0,&dma_chain->init)){
		DEBUG_TEST("%s: Warning: %s():%d dma chain busy %d!\n",
				card->devname, __FUNCTION__, __LINE__,
				dma_chain->index);
		err=-EBUSY;
		goto aft_dma_single_chain_rx_exit;
	}

	/* This will only be done on the first time.  The dma_chain
	 * skb will be re-used all the time, thus no need for
	 * rx_free_list any more */
	if (!dma_chain->skb){
		dma_chain->skb=wan_skb_dequeue(&chan->wp_rx_free_list);
		if (!dma_chain->skb){
			DEBUG_EVENT("%s: Warning Rx Voice chain = %d: no free rx bufs\n",
					chan->if_name,dma_chain->index);
			wan_clear_bit(0,&dma_chain->init);
			err=-EINVAL;
			goto aft_dma_single_chain_rx_exit;
		}

		wan_skb_init(dma_chain->skb,16);
		wan_skb_trim(dma_chain->skb,0);
		
#if defined(__LINUX__)
		dma_chain->dma_addr = card->hw_iface.pci_map_dma(card->hw,
						wan_skb_tail(dma_chain->skb),
						chan->dma_mru,
		    		       		PCI_DMA_FROMDEVICE); 

		if (dma_chain->dma_addr & AFT_TDMV_BUF_MASK) {
                	dma_chain->dma_offset = 
				AFT_TDMV_BUF_MASK - (dma_chain->dma_addr & AFT_TDMV_BUF_MASK) + 1;

			dma_chain->dma_virt = wan_skb_tail(dma_chain->skb) + dma_chain->dma_offset;
			dma_chain->dma_addr += dma_chain->dma_offset;

		} else {
                       	dma_chain->dma_offset=0; 
                       	dma_chain->dma_virt = wan_skb_tail(dma_chain->skb);
		}
		
		dma_chain->dma_len  = chan->dma_mru-dma_chain->dma_offset;
		dma_chain->dma_map_len=dma_chain->dma_len;

		DEBUG_TEST("%s: RXDMA PHY = 0x%08X VIRT = %p \n", 
				chan->if_name,
				dma_chain->dma_addr,wan_skb_tail(dma_chain->skb)+dma_chain->dma_offset);
#else
		dma_chain->dma_addr = 
			       	virt_to_phys(wan_skb_tail(dma_chain->skb));

                if (dma_chain->dma_addr & AFT_TDMV_BUF_MASK) {
		      	dma_chain->dma_offset = 
				AFT_TDMV_BUF_MASK - 
				(dma_chain->dma_addr & AFT_TDMV_BUF_MASK) + 1;
			dma_chain->dma_virt = wan_skb_tail(dma_chain->skb) + dma_chain->dma_offset;
			dma_chain->dma_addr = virt_to_phys(wan_skb_tail(dma_chain->dma_virt));
		} else {
			dma_chain->dma_offset=0; 
                        dma_chain->dma_virt = wan_skb_tail(dma_chain->skb); 
		}
		
		dma_chain->dma_len  = chan->dma_mru;
		dma_chain->dma_map_len=dma_chain->dma_len;
		
#endif
	}else{
		wan_skb_init(dma_chain->skb,16);
		wan_skb_trim(dma_chain->skb,0);
	}

	dma_ram_desc=chan->logic_ch_num*4 + 
		AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	aft_dmachain_set_rx_dma_addr(&reg,1);	
	card->hw_iface.bus_write_4(card->hw,dma_ram_desc,reg);
	
	err=aft_dma_chain_rx(dma_chain,chan,1,1);
	if (err){
		DEBUG_EVENT("%s: Rx dma chain %d overrun error: should never happen!\n",
				chan->if_name,dma_chain->index);
		aft_rx_dma_chain_init(chan,dma_chain);
		chan->if_stats.rx_errors++;
	}

aft_dma_single_chain_rx_exit:

	wan_clear_bit(RX_DMA_BUSY,&chan->dma_status);
	return err;
}


static int aft_dma_rx(sdla_t *card, private_area_t *chan)
{
	int err=0, intr=0;
	aft_dma_chain_t *dma_chain;
	int cur_dma_ptr, i,max_dma_cnt,free_queue_len;
	u32 reg, dma_ram_desc;


	if (chan->channelized_cfg && !chan->hdlc_eng){
		return aft_dma_voice_rx(card,chan);
	}
	
	if (wan_test_and_set_bit(RX_DMA_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);
		return -EBUSY;
	}

	free_queue_len=wan_skb_queue_len(&chan->wp_rx_free_list);
	if (!chan->single_dma_chain &&
	    free_queue_len < MAX_AFT_DMA_CHAINS){
		aft_free_rx_complete_list(chan);
		free_queue_len=wan_skb_queue_len(&chan->wp_rx_free_list);
		if (free_queue_len < MAX_AFT_DMA_CHAINS){
			DEBUG_EVENT("%s: %s() CRITICAL ERROR: No free rx buffers\n",
					card->devname,__FUNCTION__);
			goto te1rx_skip;
		}
	}

	dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	cur_dma_ptr=aft_dmachain_get_rx_dma_addr(reg);

#if 0
	aft_chain_history(chan,chan->rx_chain_indx, cur_dma_ptr, chan->rx_pending_chain_indx,1);
#endif
	max_dma_cnt = MAX_AFT_DMA_CHAINS;

	if (!chan->single_dma_chain && 
	    free_queue_len < max_dma_cnt){
		max_dma_cnt = free_queue_len;
	}

	
	DEBUG_TEST("%s: DMA RX: CBoardPtr=%d  Driver=%d MaxDMA=%d\n",
			card->devname,cur_dma_ptr,chan->rx_chain_indx,max_dma_cnt);

	for (i=0;i<max_dma_cnt;i++){

		dma_chain = &chan->rx_dma_chain_table[chan->rx_chain_indx];

		/* If the current DMA chain is in use,then
		 * all chains are busy */
		if (wan_test_and_set_bit(0,&dma_chain->init)){
			DEBUG_TEST("%s: Warning: %s():%d dma chain busy %d!\n",
					card->devname, __FUNCTION__, __LINE__,
					dma_chain->index);
		
			err=-EBUSY;
			break;
		}
		
		dma_chain->skb=wan_skb_dequeue(&chan->wp_rx_free_list);
		if (!dma_chain->skb){
			/* If this ever happends, it means that wp_bh is stuck for some
			 * reason, thus start using the completed buffers, thus 
			 * overflow the data */
			dma_chain->skb=wan_skb_dequeue(&chan->wp_rx_complete_list);
			if (dma_chain->skb) {
				chan->if_stats.rx_dropped++;
				wan_skb_init(dma_chain->skb,16);
				wan_skb_trim(dma_chain->skb,0);
			}else{
				DEBUG_EVENT("%s: Critical Rx chain = %d: no free rx bufs (Free=%i Comp=%i)\n",
					chan->if_name,dma_chain->index,
					wan_skb_queue_len(&chan->wp_rx_free_list),
					wan_skb_queue_len(&chan->wp_rx_complete_list));
				wan_clear_bit(0,&dma_chain->init);

				aft_critical_shutdown(card);
				err=-EINVAL;
				break;
			}
			
		}
		
		dma_chain->dma_addr = card->hw_iface.pci_map_dma(card->hw,
			      	       		wan_skb_tail(dma_chain->skb),
						chan->dma_mru,
		    		       		PCI_DMA_FROMDEVICE);

		dma_chain->dma_len  = chan->dma_mru;
		dma_chain->dma_map_len=dma_chain->dma_len;
		dma_chain->dma_offset=0;

		intr=0;
		if (!wan_test_bit(RX_INTR_PENDING,&chan->dma_chain_status)){
			
			free_queue_len--;

			if (free_queue_len <= 2){
				DEBUG_TEST("%s: DBG: Setting intr queue size low\n",
					card->devname);
				intr=1;
			}else{
				if (chan->rx_chain_indx >= cur_dma_ptr){
					intr = ((MAX_AFT_DMA_CHAINS - 
						(chan->rx_chain_indx-cur_dma_ptr)) <=4);
				}else{
					intr = ((cur_dma_ptr - chan->rx_chain_indx)<=4);
				}
			}

			if (intr){
				DEBUG_TEST("%s: Setting Rx interrupt on chain=%d\n",
					chan->if_name,dma_chain->index);
				wan_set_bit(RX_INTR_PENDING,&chan->dma_chain_status);
			}
		}

		DEBUG_TEST("%s: Setting Buffer on Rx Chain = %d Intr=%d\n",
					chan->if_name,dma_chain->index, intr);

		err=aft_dma_chain_rx(dma_chain,chan,intr,0);
		if (err){
			DEBUG_EVENT("%s: Rx dma chain %d overrun error: should never happen!\n",
					chan->if_name,dma_chain->index);
			aft_rx_dma_chain_init(chan,dma_chain);
			chan->if_stats.rx_errors++;
			break;
		}

		if (chan->single_dma_chain){
			break;
		}else{
			if (++chan->rx_chain_indx >= MAX_AFT_DMA_CHAINS){
				chan->rx_chain_indx=0;
			}
		}
	}

#if 0
	aft_chain_history(chan,chan->rx_chain_indx, cur_dma_ptr, chan->rx_pending_chain_indx,2);

	if (chan->rx_chain_indx == cur_dma_ptr &&
            chan->rx_pending_chain_indx == cur_dma_ptr &&
	    cur_dma_ptr != 0){
		DEBUG_EVENT("%s :Location 1 Listing Descr!\n",
			chan->if_name);
		aft_list_descriptors(chan);
	}
#endif
	aft_rx_cur_go_test(chan);

te1rx_skip:

	wan_clear_bit(RX_DMA_BUSY,&chan->dma_status);
	
	return err;
}

/*===============================================
 * aft_rx_dma_chain_handler
 *
 */
/*N1*/
static int aft_rx_dma_chain_handler(private_area_t *chan, int reset)
{
	sdla_t *card = chan->card;
	u32 reg,dma_descr;
	wp_rx_element_t *rx_el;
	aft_dma_chain_t *dma_chain;
	int i,max_dma_cnt, cur_dma_ptr;
	int rx_data_available=0;
	u32 dma_ram_desc;

	if (wan_test_and_set_bit(RX_HANDLER_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);
		return rx_data_available;
	}

	dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	cur_dma_ptr=aft_dmachain_get_rx_dma_addr(reg);

	max_dma_cnt = MAX_AFT_DMA_CHAINS;

	DEBUG_TEST("%s: DMA RX: CBoardPtr=%d  Driver=%d MaxDMA=%d\n",
			card->devname,
			cur_dma_ptr,
			chan->rx_chain_indx,max_dma_cnt);

#if 0
	aft_chain_history(chan,chan->rx_chain_indx, cur_dma_ptr, chan->rx_pending_chain_indx,3);
#endif

	
	for (i=0;i<max_dma_cnt;i++){

		dma_chain = &chan->rx_dma_chain_table[chan->rx_pending_chain_indx];

		if (i>0 && chan->rx_pending_chain_indx == cur_dma_ptr){
			break;
		}

		if (!dma_chain){
			DEBUG_EVENT("%s:%d Assertion Error !!!!\n",
				__FUNCTION__,__LINE__);
			break;
		}

		/* If the current DMA chain is in use,then
		 * all chains are busy */
		if (!wan_test_bit(0,&dma_chain->init)){
			DEBUG_TEST("%s: Warning (%s) Pending chain %d empty!\n",
				chan->if_name,__FUNCTION__,dma_chain->index);

			break;
		}

		dma_descr=(chan->logic_ch_num<<4) + (dma_chain->index*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

		card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);

		/* If GO bit is set, then the current DMA chain
		 * is in process of being transmitted, thus
		 * all are busy */
		if (wan_test_bit(AFT_RXDMA_HI_GO_BIT,&reg)){
			
			if (wan_test_bit(WP_FIFO_ERROR_BIT, &chan->pkt_error)){
				break;
			}

#if 0
			if (chan->single_dma_chain){
			DEBUG_EVENT("%s: CRITICAL (%s) Pending chain %d Go bit still set!\n",
				chan->if_name,__FUNCTION__,dma_chain->index);
				++chan->if_stats.rx_errors;
			}else{	
			DEBUG_TEST("%s: Warning (%s) Pending chain %d Go bit still set!\n",
				chan->if_name,__FUNCTION__,dma_chain->index);
			}
#endif
			break;
		}

		if (!wan_test_bit(AFT_RXDMA_HI_INTR_DISABLE_BIT,&reg)){
			wan_clear_bit(RX_INTR_PENDING,&chan->dma_chain_status);	
		}

		card->hw_iface.pci_unmap_dma(card->hw,
				 dma_chain->dma_addr-dma_chain->dma_offset,
				 dma_chain->dma_map_len,
				 PCI_DMA_FROMDEVICE);

		if (sizeof(wp_rx_element_t) > wan_skb_headroom(dma_chain->skb)){
			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: Rx error: rx_el=%u > max head room=%u\n",
						chan->if_name,
						(u32)sizeof(wp_rx_element_t),
						(u32)wan_skb_headroom(dma_chain->skb));
			}

			aft_init_requeue_free_skb(chan, dma_chain->skb);
			++chan->if_stats.rx_errors;
			goto rx_hndlr_skip_rx;
		}else{
			rx_el=(wp_rx_element_t *)wan_skb_push(dma_chain->skb, 
							      sizeof(wp_rx_element_t));
			memset(rx_el,0,sizeof(wp_rx_element_t));
		}
#if 0
		chan->if_stats.rx_frame_errors++;
#endif

       		/* Reading Rx DMA descriptor information */
		dma_descr=(chan->logic_ch_num<<4) +(dma_chain->index*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_RX_DMA_LO_DESCR_BASE_REG);

		card->hw_iface.bus_read_4(card->hw,dma_descr, &rx_el->align);
		rx_el->align&=AFT_RXDMA_LO_ALIGN_MASK;

       		dma_descr=(chan->logic_ch_num<<4) +(dma_chain->index*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

		card->hw_iface.bus_read_4(card->hw,dma_descr, &rx_el->reg);

		rx_el->pkt_error= dma_chain->pkt_error;
		rx_el->dma_addr = dma_chain->dma_addr;

		wan_skb_queue_tail(&chan->wp_rx_complete_list,dma_chain->skb);
		rx_data_available=1;

		DEBUG_TEST("%s: RxInt Pending chain %d Rxlist=%d LO:0x%X HI:0x%X Data=0x%X Len=%d!\n",
				chan->if_name,dma_chain->index,
				wan_skb_queue_len(&chan->wp_rx_complete_list),
				rx_el->align,rx_el->reg,
				(*(unsigned char*)wan_skb_data(dma_chain->skb)),
				wan_skb_len(dma_chain->skb));

rx_hndlr_skip_rx:
		dma_chain->skb=NULL;
		dma_chain->dma_addr=0;
		dma_chain->dma_len=0;
		dma_chain->dma_map_len=dma_chain->dma_len;
		dma_chain->pkt_error=0;
		wan_clear_bit(0,&dma_chain->init);

		if (chan->single_dma_chain){
			break;
		}

		if (++chan->rx_pending_chain_indx >= MAX_AFT_DMA_CHAINS){
			chan->rx_pending_chain_indx=0;
		}
	}

	if (reset){
		goto reset_skip_rx_setup;
	}

	aft_dma_rx(card,chan);


	if (wan_skb_queue_len(&chan->wp_rx_complete_list)){
		DEBUG_TEST("%s: Rx Queued list triggering\n",chan->if_name);
		WAN_TASKLET_SCHEDULE((&chan->common.bh_task));
	}

reset_skip_rx_setup:

	wan_clear_bit(RX_HANDLER_BUSY,&chan->dma_status);


	return rx_data_available;	
}

static int aft_dma_rx_complete(sdla_t *card, private_area_t *chan, int reset)
{
	if (chan->cfg.ss7_enable){
		aft_clear_ss7_force_rx(card,chan);
	}
	return aft_rx_dma_chain_handler(chan,reset);
}

/*===============================================
 *  TE1 DMA Chains Utility Funcitons
 *
 */


static void aft_index_tx_rx_dma_chains(private_area_t *chan)
{
	int i;

	for (i=0;i<MAX_AFT_DMA_CHAINS;i++){
		chan->tx_dma_chain_table[i].index=i;
		chan->rx_dma_chain_table[i].index=i;
	}
}


static void aft_init_tx_rx_dma_descr(private_area_t *chan)
{
	int i;
	u32 reg=0;
	sdla_t *card=chan->card;
	unsigned long tx_dma_descr,rx_dma_descr;
	unsigned int dma_cnt=MAX_AFT_DMA_CHAINS;

	if (chan->single_dma_chain){
		dma_cnt=1;
	}

	for (i=0;i<dma_cnt;i++){

		tx_dma_descr=(chan->logic_ch_num<<4) + 
			     (i*AFT_DMA_INDEX_OFFSET) + 
			     AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);
		
		rx_dma_descr=(chan->logic_ch_num<<4) + 
			     (i*AFT_DMA_INDEX_OFFSET) + 
			     AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);
        	
		card->hw_iface.bus_write_4(card->hw,tx_dma_descr,reg);
		card->hw_iface.bus_write_4(card->hw,rx_dma_descr,reg);

		aft_tx_dma_chain_init(chan,&chan->tx_dma_chain_table[i]);
		aft_rx_dma_chain_init(chan,&chan->rx_dma_chain_table[i]);
	}
}

static void aft_free_rx_complete_list(private_area_t *chan)
{
	void *skb;

	while((skb=wan_skb_dequeue(&chan->wp_rx_complete_list)) != NULL){
		DEBUG_TEST("%s: aft_free_rx_complete_list dropped\n",
				chan->if_name);
		chan->if_stats.rx_dropped++;
		aft_init_requeue_free_skb(chan, skb);
	}
}

static void aft_rx_cur_go_test(private_area_t *chan)
{
#if 0
	u32 reg,cur_dma_ptr;
	sdla_t *card=chan->card;
	aft_dma_chain_t *dma_chain;
	u32 dma_descr;
	int i;
	u32 dma_ram_desc;
	unsigned int dma_cnt=MAX_AFT_DMA_CHAINS;

	dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	cur_dma_ptr=aft_dmachain_get_rx_dma_addr(reg);

	dma_descr=(chan->logic_ch_num<<4) +(cur_dma_ptr*AFT_DMA_INDEX_OFFSET) + 
		  AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

	card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);

	if (!wan_test_bit(AFT_RXDMA_HI_GO_BIT,&reg)){
		DEBUG_EVENT("%s: CRITICAL Cur =%d not Got bit set!\n",
				chan->if_name,
				cur_dma_ptr);


		aft_list_descriptors(chan);
	}
#endif
}


#if 0
static void aft_list_descriptors(private_area_t *chan)
{
	u32 reg,cur_dma_ptr,lo_reg;
	sdla_t *card=chan->card;
	aft_dma_chain_t *dma_chain;
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

	if (chan->single_dma_chain){
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

#if 0
static void aft_list_tx_descriptors(private_area_t *chan)
{
	u32 reg,cur_dma_ptr,lo_reg;
	sdla_t *card=chan->card;
	aft_dma_chain_t *dma_chain;
	u32 dma_descr;
	int i;
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

	if (chan->single_dma_chain){
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

		if (!wan_test_bit(0,&chan->up)){
			continue;
		}
		
		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);

		DEBUG_EVENT("%s: DMA CHAIN: %i: 0x%08X\n",
				card->devname,i,reg);

	}
}
#endif


static void aft_free_rx_descriptors(private_area_t *chan)
{
	u32 reg;
	sdla_t *card=chan->card;
	aft_dma_chain_t *dma_chain;
	u32 dma_descr;
	int i;
	unsigned int dma_cnt=MAX_AFT_DMA_CHAINS;

	if (chan->single_dma_chain){
		dma_cnt=1;
	}
	
	for (i=0;i<dma_cnt;i++){

		dma_chain = &chan->rx_dma_chain_table[i];
		if (!dma_chain){
			DEBUG_EVENT("%s:%d Assertion Error !!!!\n",
				__FUNCTION__,__LINE__);
			break;
		}

		dma_descr=(chan->logic_ch_num<<4) +
			   (dma_chain->index*AFT_DMA_INDEX_OFFSET) + 
			   AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

		DEBUG_TEST("%s:%s: Rx: Freeing Descripors Ch=%ld Desc=0x%X\n",
				card->devname,chan->if_name,chan->logic_ch_num,dma_descr);

		card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);

		/* If GO bit is set, then the current DMA chain
		 * is in process of being transmitted, thus
		 * all are busy */
		reg=0;
		card->hw_iface.bus_write_4(card->hw,dma_descr,reg);

		card->hw_iface.pci_unmap_dma(card->hw,
				dma_chain->dma_addr-dma_chain->dma_offset,
				dma_chain->dma_map_len,
				PCI_DMA_FROMDEVICE);

		if (dma_chain->skb){
			aft_init_requeue_free_skb(chan, dma_chain->skb);
		}

		dma_chain->skb=NULL;
		dma_chain->dma_addr=0;
		dma_chain->dma_len=0;
		dma_chain->dma_map_len=dma_chain->dma_len;
		dma_chain->pkt_error=0;
		wan_clear_bit(0,&dma_chain->init);
	}

}

static void aft_reset_rx_chain_cnt(private_area_t *chan)
{
	u32 dma_ram_desc,reg,cur_dma_ptr;
	sdla_t *card=chan->card;

	dma_ram_desc=chan->logic_ch_num*4+
			AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	cur_dma_ptr=aft_dmachain_get_rx_dma_addr(reg);
	chan->rx_pending_chain_indx = chan->rx_chain_indx = cur_dma_ptr;
	DEBUG_TEST("%s:  Setting Rx Index to %d\n",
		chan->if_name,cur_dma_ptr);
	
}

static void aft_reset_tx_chain_cnt(private_area_t *chan)
{
	u32 dma_ram_desc,reg,cur_dma_ptr;
	sdla_t *card=chan->card;

	dma_ram_desc=chan->logic_ch_num*4+
			AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	cur_dma_ptr=aft_dmachain_get_tx_dma_addr(reg);
	chan->tx_pending_chain_indx = chan->tx_chain_indx = cur_dma_ptr;
	DEBUG_TEST("%s:  Setting Tx Index to %d\n",
		chan->if_name,cur_dma_ptr);
	
}



static void aft_free_tx_descriptors(private_area_t *chan)
{
	u32 reg,dma_descr;
	sdla_t *card=chan->card;
	aft_dma_chain_t *dma_chain;
	int i;
	void *skb;
	unsigned int dma_cnt=MAX_AFT_DMA_CHAINS;
	
	if (chan->single_dma_chain){
		dma_cnt=1;
	}

	DEBUG_TEST("%s:%s: Tx: Freeing Descripors\n",card->devname,chan->if_name);

	for (i=0;i<dma_cnt;i++){

		dma_chain = &chan->tx_dma_chain_table[i];
		if (!dma_chain){
			DEBUG_EVENT("%s:%d Assertion Error !!!!\n",
				__FUNCTION__,__LINE__);
			break;
		}

		dma_descr=(chan->logic_ch_num<<4) +(dma_chain->index*AFT_DMA_INDEX_OFFSET) + 
			  AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);

		DEBUG_TEST("%s:%s: Tx: Freeing Descripors Ch=%ld Desc=0x%X\n",
				card->devname,chan->if_name,chan->logic_ch_num,dma_descr);
		reg=0;
		card->hw_iface.bus_write_4(card->hw,dma_descr,reg);

		aft_tx_dma_chain_init(chan, dma_chain);
	}

	chan->tx_chain_indx = chan->tx_pending_chain_indx;

	while((skb=wan_skb_dequeue(&chan->wp_tx_complete_list)) != NULL){
		wan_skb_free(skb);
	}
}


/*=====================================================
 * Chanalization/Logic Channel utilites
 *
 */
void aft_free_logical_channel_num (sdla_t *card, int logic_ch)
{
	wan_clear_bit (logic_ch,&card->u.aft.logic_ch_map);
	card->u.aft.dev_to_ch_map[logic_ch]=NULL;

	if (logic_ch >= card->u.aft.top_logic_ch){
		int i;

		card->u.aft.top_logic_ch=AFT_DEFLT_ACTIVE_CH;

		for (i=0;i<card->u.aft.num_of_time_slots;i++){
			if (card->u.aft.dev_to_ch_map[i]){
				card->u.aft.top_logic_ch=i;
			}
		}

		
		aft_dma_max_logic_ch(card);
	}

}

void aft_dma_max_logic_ch(sdla_t *card)
{
	u32 reg;

	DEBUG_TEST("%s: Maximum Logic Ch set to %d \n",
			card->devname,card->u.aft.top_logic_ch);

	reg=0;
	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
	aft_dmactrl_set_max_logic_ch(&reg,card->u.aft.top_logic_ch);
	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
}

static int aft_tslot_sync_ctrl(sdla_t *card, private_area_t *chan, int mode)
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



static int aft_read_security(sdla_t *card)
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


static int aft_front_end_mismatch_check(sdla_t * card)
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

static int aft_realign_skb_pkt(private_area_t *chan, netskb_t *skb)
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

	if ((unsigned long)data & 0x03){
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
	u8 reg;
	u32 wdt_ctrl_reg=AFT_WDT_1TO4_CTRL_REG+card->wandev.comm_port;
	
	if (card->wandev.comm_port > 3) {
		wdt_ctrl_reg=AFT_WDT_4TO8_CTRL_REG+(card->wandev.comm_port%4);	
	}
	
	card->hw_iface.bus_read_1(card->hw,wdt_ctrl_reg, &reg);		
	aft_wdt_ctrl_set(&reg,val);
	card->hw_iface.bus_write_1(card->hw,wdt_ctrl_reg, reg);
}
void aft_wdt_reset(sdla_t *card)
{
	u8 reg;
	u32 wdt_ctrl_reg=AFT_WDT_1TO4_CTRL_REG+card->wandev.comm_port;
	
	if (card->wandev.comm_port > 3) {
		wdt_ctrl_reg=AFT_WDT_4TO8_CTRL_REG+(card->wandev.comm_port%4);	
	}
	
	card->hw_iface.bus_read_1(card->hw, wdt_ctrl_reg, &reg);		
	aft_wdt_ctrl_reset(&reg);
	card->hw_iface.bus_write_1(card->hw, wdt_ctrl_reg, reg);
}

#if defined(__LINUX__)
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))     
static void aft_port_task (void * card_ptr)
# else
static void aft_port_task (struct work_struct *work)	
# endif
#else
static void aft_port_task (void * card_ptr, int arg)
#endif
{
#if defined(__LINUX__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))   
        sdla_t 		*card = (sdla_t *)container_of(work, sdla_t, u.aft.port_task);
#else
	sdla_t 		*card = (sdla_t *)card_ptr;
#endif 
	wan_smp_flag_t smp_flags;

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		return;
	}

	DEBUG_56K("%s: PORT TASK: 0x%X\n", card->devname,card->u.aft.port_task_cmd);

#ifdef AFT_IRQ_STAT_DEBUG
      	card->wandev.stats.rx_missed_errors++;
#endif	


	if (wan_test_bit(AFT_FE_INTR,&card->u.aft.port_task_cmd)){

		DEBUG_TEST("%s: PORT TASK: FE INTER\n", card->devname);

		card->hw_iface.hw_lock(card->hw,&smp_flags);
		aft_fe_intr_ctrl(card, 0);
		front_end_interrupt(card,0,1);
		wan_clear_bit(AFT_FE_INTR,&card->u.aft.port_task_cmd);
		aft_fe_intr_ctrl(card, 1);
		card->hw_iface.hw_unlock(card->hw,&smp_flags);

	}

	if (wan_test_bit(AFT_FE_POLL,&card->u.aft.port_task_cmd)){
		DEBUG_TEST("%s: PORT TASK: FE POLL\n", card->devname);
		card->hw_iface.hw_lock(card->hw,&smp_flags);
		aft_fe_intr_ctrl(card, 0);
		if (card->wandev.fe_iface.polling){
			wan_smp_flag_t	smp_irq_flags;
			int		err = 0;
			err = card->wandev.fe_iface.polling(&card->fe);

			wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
			handle_front_end_state(card);
			wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);
		}
		wan_clear_bit(AFT_FE_POLL,&card->u.aft.port_task_cmd);
		aft_fe_intr_ctrl(card, 1);
		card->hw_iface.hw_unlock(card->hw,&smp_flags);
	}

	if (wan_test_bit(AFT_FE_LED,&card->u.aft.port_task_cmd)){
		DEBUG_TEST("%s: PORT TASK: FE LED\n", card->devname);
		card->hw_iface.hw_lock(card->hw,&smp_flags);
		aft_fe_intr_ctrl(card, 0);
		if (card->wandev.state == WAN_CONNECTED){
			aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_RED, 0,WAN_AFT_OFF);
			aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_GREEN, 0, WAN_AFT_ON);
		}else{
			aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_RED, 0,WAN_AFT_ON);
			aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_GREEN, 0, WAN_AFT_OFF);
		}
		wan_clear_bit(AFT_FE_LED,&card->u.aft.port_task_cmd);
		aft_fe_intr_ctrl(card, 1);
		card->hw_iface.hw_unlock(card->hw,&smp_flags);
	}

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
	if (wan_test_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd)){
		int	err;
		DEBUG_TEST("%s: PORT TASK: FE RBS\n", card->devname);
		card->hw_iface.hw_lock(card->hw,&smp_flags);

		WAN_TDMV_CALL(rbsbits_poll, (&card->wan_tdmv, card), err);

		wan_clear_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
		card->hw_iface.hw_unlock(card->hw,&smp_flags);
	}
#endif

#if defined(CONFIG_WANPIPE_HWEC)
	if (wan_test_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd)){

		DEBUG_TEST("%s: PORT TASK: FE EC INTR\n", card->devname);

		if (card->wandev.ec_dev){
			card->hw_iface.hw_ec_lock(card->hw,&smp_flags);
			wanpipe_ec_poll(card->wandev.ec_dev, card);
			wan_clear_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd);
			card->hw_iface.hw_ec_unlock(card->hw,&smp_flags);
		}
	}
#endif
}

void __aft_fe_intr_ctrl(sdla_t *card, int status)
{
	u32 reg;

	card->hw_iface.bus_read_4(card->hw,AFT_CHIP_CFG_REG,&reg);
	if (status){
	        wan_set_bit(AFT_CHIPCFG_FE_INTR_CFG_BIT,&reg);
	}else{
		wan_clear_bit(AFT_CHIPCFG_FE_INTR_CFG_BIT,&reg);
	}
	card->hw_iface.bus_write_4(card->hw,AFT_CHIP_CFG_REG,reg);

}

void aft_fe_intr_ctrl(sdla_t *card, int status)
{
	wan_smp_flag_t smp_flags;

	wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
	__aft_fe_intr_ctrl(card,status);
	wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);

}

static void aft_data_mux_cfg(sdla_t *card)
{
	if (!card->u.aft.cfg.data_mux_map){
		card->u.aft.cfg.data_mux_map=AFT_DEFAULT_DATA_MUX_MAP;
	}
	
	card->hw_iface.bus_write_4(card->hw, AFT_DATA_MUX_REG, 
				   card->u.aft.cfg.data_mux_map);		

	DEBUG_EVENT("%s: AFT Data Mux Bit Map: 0x%08X\n",
		card->devname,card->u.aft.cfg.data_mux_map);
}

static void aft_data_mux_get_cfg(sdla_t *card)
{
	card->hw_iface.bus_read_4(card->hw, AFT_DATA_MUX_REG, 
				   &card->u.aft.cfg.data_mux_map);		

}


static int aft_ss7_tx_mangle(sdla_t *card,private_area_t *chan, netskb_t *skb)
{
	int ss7_len=0,len=0;
	unsigned int ss7_ctrl=0;
	unsigned char *buf;
	api_tx_hdr_t *tx_hdr = &chan->tx_api_hdr;

	memcpy(&chan->tx_api_hdr,wan_skb_data(skb),sizeof(api_tx_hdr_t));
	wan_skb_pull(skb,sizeof(api_tx_hdr_t));

	if (!chan->tx_ss7_realign_buf){
		chan->tx_ss7_realign_buf=wan_malloc(chan->dma_mru);
		if (!chan->tx_ss7_realign_buf){
			DEBUG_EVENT("%s: Error: Failed to allocate ss7 tx memory buf\n",
						chan->if_name);
			return -ENOMEM;
		}else{
			DEBUG_TEST("%s: AFT SS7 Realign buffer allocated Len=%d\n",
						chan->if_name,chan->dma_mru);
		}
	}

	memset(chan->tx_ss7_realign_buf,0,chan->dma_mru);
	memcpy(chan->tx_ss7_realign_buf,wan_skb_data(skb),wan_skb_len(skb));
	len=wan_skb_len(skb);

	/* Align the end of the frame to 32 byte boundary */
	ss7_ctrl=(len%4)&AFT_SS7_CTRL_LEN_MASK;
	if (ss7_ctrl != 0){
		len-=len%4;
		len+=4;
	}

	if (tx_hdr->u.ss7.type == WANOPT_SS7_FISU){
		if (chan->cfg.ss7_mode == WANOPT_SS7_MODE_4096){
			ss7_len=WANOPT_SS7_FISU_4096_SZ;
		}else{
			ss7_len=WANOPT_SS7_FISU_128_SZ;
		}
		wan_clear_bit(AFT_SS7_CTRL_TYPE_BIT,&ss7_ctrl);
	}else{
		ss7_len=chan->cfg.ss7_lssu_size;
		wan_set_bit(AFT_SS7_CTRL_TYPE_BIT,&ss7_ctrl);
	}

	if (tx_hdr->u.ss7.force_tx){
		wan_set_bit(AFT_SS7_CTRL_FORCE_BIT,&ss7_ctrl);
	}else{
		wan_clear_bit(AFT_SS7_CTRL_FORCE_BIT,&ss7_ctrl);
	}

	DEBUG_TEST("%s: SS7 DATA = 0x%X 0x%X 0x%X\n",
			chan->if_name,
			tx_hdr->u.ss7.data[0],
			tx_hdr->u.ss7.data[1],
			tx_hdr->u.ss7.data[2]);
	
	memcpy(&chan->tx_ss7_realign_buf[len],tx_hdr->u.ss7.data,ss7_len);

	len+=ss7_len;

	wan_skb_init(skb,0);
	wan_skb_trim(skb,0);

	if (wan_skb_tailroom(skb) < len){
		DEBUG_EVENT("%s: Critical error: SS7 Tx unalign pkt tail room(%i) < len(%i)!\n",
				chan->if_name,wan_skb_tailroom(skb),len);
		
		return -ENOMEM;
	}
	
	buf=wan_skb_put(skb,len);
	memcpy(buf,chan->tx_ss7_realign_buf,len);
	
	wan_skb_set_csum(skb, ss7_ctrl);

#if 0
	debug_print_skb_pkt(chan->if_name, wan_skb_data(skb), wan_skb_len(skb), 0);
#endif
	return 0;
}




#if 0
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

/*
 * ******************************************************************
 * Proc FS function 
 */
static int wan_aft_get_info(void* pcard, struct seq_file *m, int *stop_cnt)
{
	sdla_t	*card = (sdla_t*)pcard;

	if (card->wandev.fe_iface.update_alarm_info){
		m->count = 
			WAN_FECALL(
				&card->wandev, 
				update_alarm_info, 
				(&card->fe, m, stop_cnt)); 
	}
	if (card->wandev.fe_iface.update_pmon_info){
		m->count = 
			WAN_FECALL(
				&card->wandev, 
				update_pmon_info, 
				(&card->fe, m, stop_cnt)); 
	}

	return m->count;
}

static int aft_tdmv_init(sdla_t *card, wandev_conf_t *conf)
{

	int err;
	int valid_firmware_ver=AFT_TDMV_FRM_VER;

	err=0;
	DEBUG_EVENT("%s:    TDMV Span      = %d : %s\n",
			card->devname,
			card->tdmv_conf.span_no,
			card->tdmv_conf.span_no?"Enabled":"Disabled");

	if (card->tdmv_conf.span_no) {
		if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
			valid_firmware_ver=AFT_MIN_ANALOG_FRMW_VER;
		}
	
		if (card->u.aft.firm_ver < valid_firmware_ver){
			DEBUG_EVENT("%s: Error: Obselete AFT Firmware version: %X\n",
					card->devname,card->u.aft.firm_ver);
			DEBUG_EVENT("%s: Error: AFT TDMV Support depends on Firmware Ver >= %X\n",
					card->devname,valid_firmware_ver);
			return -EINVAL;
		}	
	}
			


	return 0;

}


/**************************************************************
 * TDMV VOICE Functions
 **************************************************************/
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
static int aft_tdmv_free(sdla_t *card)
{
	if (card->tdmv_conf.span_no && card->wan_tdmv.sc){
		int	err;
		WAN_TDMV_CALL(remove, (card), err);
	}
	return 0;
}
#endif


static int aft_tdmv_if_init(sdla_t *card, private_area_t *chan, wanif_conf_t *conf)
{
	
	if (!chan->channelized_cfg){
		return 0;
	}
	
	if (!card->tdmv_conf.span_no){
		DEBUG_EVENT("%s: Error: TDMV Span No is not set!\n",
				card->devname);
		return -EINVAL;
	}
	
	if (chan->cfg.tdmv_master_if){
		DEBUG_EVENT("%s: Configuring TDMV Master dev %s\n",
				card->devname,chan->if_name);

		/* Initialize a TDM bottom half handler 
		 * Optionally used */
		WAN_TASKLET_INIT((&chan->common.bh_task),0,wp_tdm_bh,chan);
	}

	if (conf->hdlc_streaming == 0){

		int	err;
		
		err=0;

		aft_hwdev[card->wandev.card_type].aft_fifo_adjust(card,AFT_TDMV_FIFO_LEVEL);

                if (chan->common.usedby == TDM_VOICE) {
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE	
			WAN_TDMV_CALL(check_mtu, (card, conf->active_ch, &chan->mtu), err);
			if (err){
				DEBUG_EVENT("Error: TMDV mtu check failed!");
				return -EINVAL;
			}
#endif
		}
		
		if (chan->common.usedby == TDM_VOICE_API) {
			switch (chan->mtu) {
			case 8:
			case 16:
				break;
			case 40:
			case 80:
         			if (!wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
					/* If Global TDM Feature is not enable
					   then 40 and 80 bytes TDM are not available */
                                      chan->mtu=16;
				}
			       	break;
			default:
			       	chan->mtu=16;
				break;
			}
		}

		chan->mru = chan->mtu;

		card->u.aft.tdmv_mtu += chan->mtu;

		if (chan->tdmv_zaptel_cfg){
			chan->cfg.data_mux=1;
		}

		conf->hdlc_streaming=0;
		chan->tx_realign_buf = NULL;
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE	
		card->wan_tdmv.brt_enable=0;
#endif

	}else{
		chan->cfg.data_mux=0;
	}
	
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE	
	if (chan->tdmv_zaptel_cfg){
		int channel;

		/* The TDMV drivers always starts from number
		 * ZERO. Wanpipe driver doesn't allow timeslot
		 * ZERO. Thus, the active_ch map must me adjusted
		 * before calling tdmv_reg */
		if (IS_E1_CARD(card)){
			conf->active_ch=conf->active_ch>>1;
		}

		WAN_TDMV_CALL(reg, 
				(card, 
				&conf->tdmv, 
				conf->active_ch, 
				conf->hwec.enable, 
				chan->common.dev), channel);
		if (channel < 0){
			DEBUG_EVENT("%s: Error: Failed to register TDMV channel!\n",
					chan->if_name);

			return -EINVAL;
		}
		chan->tdmv_chan=channel;


		if (card->u.aft.tdmv_dchan_cfg_on_master && chan->cfg.tdmv_master_if){
			u32 orig_ch=conf->active_ch;

			conf->active_ch=card->u.aft.tdmv_dchan_cfg_on_master;

			WAN_TDMV_CALL(reg, 
					(card, 
					&conf->tdmv, 
					conf->active_ch, 
					conf->hwec.enable, 
					chan->common.dev), channel);
			if (channel < 0){
				DEBUG_EVENT("%s: Error: Failed to register TDMV channel!\n",
						chan->if_name);
	
				return -EINVAL;
			}	

			card->u.aft.tdmv_chan=channel;
			card->u.aft.tdmv_dchan_active_ch=conf->active_ch;
			conf->active_ch=orig_ch;
		}
	}
#endif

	return 0;

}


static int aft_tdmv_if_free(sdla_t *card, private_area_t *chan)
{
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
	if (chan->tdmv_zaptel_cfg){
		int err;
		WAN_TDMV_CALL(unreg, (card, chan->time_slot_map), err);
		if (err){
			return err;
		}

		if (card->u.aft.tdmv_dchan_cfg_on_master && chan->cfg.tdmv_master_if){
			DEBUG_EVENT("%s: Card Unregistering DCHAN\n",
					card->devname);
			WAN_TDMV_CALL(unreg, (card, card->u.aft.tdmv_dchan_active_ch), err);
			card->u.aft.tdmv_dchan_cfg_on_master=0;
		}
	}
#endif
	return 0;
}

#if 0
/* NCDEBUG */
static int gtmp_cnt=0;
static void aft_set_channel(sdla_t *card, int ch)
{
	aft_dma_chain_t	*tx_dma_chain;
	u8		*buf;
	private_area_t	*chan=(private_area_t*)card->u.aft.dev_to_ch_map[ch];
	
	if (!chan) return;

	tx_dma_chain = &chan->tx_dma_chain_table[0];
	if (!tx_dma_chain){
		return;
	}
	buf = (u8*)wan_skb_data(tx_dma_chain->skb);
	buf[0]=gtmp_cnt++;
	buf[1]=gtmp_cnt++;
	buf[2]=gtmp_cnt++;
	buf[3]=gtmp_cnt++;
	buf[4]=gtmp_cnt++;
	buf[5]=gtmp_cnt++;
	buf[6]=gtmp_cnt++;
	buf[7]=gtmp_cnt++;

	card->wandev.stats.tx_packets++;
}
#endif


#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
static int aft_voice_span_rx_tx(sdla_t *card, int rotate)
{
	int err;
	err=0;
	
	if (card->wan_tdmv.sc){

		if (rotate) {
	 		WAN_TDMV_CALL(buf_rotate, (card,AFT_TDMV_CIRC_BUF,AFT_TDMV_BUF_MASK), err);
		}

		if (!card->wandev.ec_enable || card->wandev.ec_enable_map == 0){
			WAN_TDMV_CALL(ec_span, (card), err);
		}

		WAN_TDMV_CALL(rx_tx_span, (card), err);
		
		WAN_TDMV_CALL(is_rbsbits, (&card->wan_tdmv), err);
		if (err == 1){
			wan_set_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
			WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));	
		}
	
#if !defined(AFT_IRQ_DEBUG)
	        card->wandev.stats.rx_packets++;
		card->wandev.stats.tx_packets++;
#endif
		
	}    
	return 0;
}
#endif

static int aft_dma_rx_tdmv(sdla_t *card, private_area_t *chan)
{
	int	err;
	u32	rx_offset=0;
	u32	tx_offset=0;

	aft_dma_chain_t *tx_dma_chain;
	aft_dma_chain_t *rx_dma_chain;

	tx_dma_chain = &chan->tx_dma_chain_table[0];
	rx_dma_chain = &chan->rx_dma_chain_table[0]; 


       	if (wan_test_bit(AFT_TDM_RING_BUF,&card->u.aft.chip_cfg_status)) {
	       	rx_offset= AFT_TDMV_CIRC_BUF * card->u.aft.tdm_rx_dma_toggle;
	       	tx_offset= AFT_TDMV_CIRC_BUF * card->u.aft.tdm_tx_dma_toggle;
	}

	err=0;

	if (!tx_dma_chain || !rx_dma_chain){
		DEBUG_EVENT("%s: %s:%d ASSERT ERROR TxDma=%p RxDma=%p\n",
				card->devname,__FUNCTION__,__LINE__,
				tx_dma_chain,rx_dma_chain);
		return -EINVAL;
	}

	if (!rx_dma_chain->skb || !tx_dma_chain->skb){
		DEBUG_TEST("%s: %s:%d ASSERT ERROR TxSkb=%p RxSkb=%p\n",
				card->devname,__FUNCTION__,__LINE__,
				rx_dma_chain->skb,tx_dma_chain->skb);
		return -EINVAL;
	}

#if 0
	/*Measure the round trip delay*/
	if (!chan->tdmv_rx_delay_cfg){
		int i;
		unsigned char *buf=rx_dma_chain->dma_virt+offset;
		for (i=0;i<8;i++){
			if (buf[i]==0x55){
				chan->tdmv_rx_delay_cfg=1;
				DEBUG_EVENT("%s: Chan=%ld Delay=%d\n",
						chan->if_name,chan->logic_ch_num,
						chan->tdmv_rx_delay);
				break;
			}
			chan->tdmv_rx_delay++;
		}
	}
#endif


	if (chan->tdmv_zaptel_cfg){
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
		if (card->wan_tdmv.sc){

#if 0 
defined(AFT_TDMV_BH_ENABLE) 
			aft_dma_chain_t *tx_bh_dma_chain = &chan->tx_dma_chain_table[1];
			aft_dma_chain_t *rx_bh_dma_chain = &chan->rx_dma_chain_table[1];

			if (!rx_bh_dma_chain->skb){
				rx_bh_dma_chain->skb=wan_skb_dequeue(&chan->wp_rx_free_list);
				if (!rx_bh_dma_chain->skb){
					if (WAN_NET_RATELIMIT()){
						DEBUG_EVENT("%s: Critical TDM BH no free skb\n",
							chan->if_name);
						goto aft_tdm_bh_skip;
					}
				}
				wan_skb_init(rx_bh_dma_chain->skb,16);
				wan_skb_trim(rx_bh_dma_chain->skb,0);
			}

			if (!tx_bh_dma_chain->skb){
				tx_bh_dma_chain->skb=wan_skb_dequeue(&chan->wp_rx_free_list);
				if (!tx_bh_dma_chain->skb){
					if (WAN_NET_RATELIMIT()){
						DEBUG_EVENT("%s: Critical TDM BH no free skb\n",
							chan->if_name);
						goto aft_tdm_bh_skip;
					}
				}
				wan_skb_init(tx_bh_dma_chain->skb,16);
				wan_skb_trim(tx_bh_dma_chain->skb,0);
			}

			memcpy(wan_skb_data(rx_bh_dma_chain->skb),
			       wan_skb_data(rx_dma_chain->skb),8);
		
			memcpy(wan_skb_data(tx_dma_chain->skb),
		       		wan_skb_data(tx_bh_dma_chain->skb),8);

			rx_dma_chain=rx_bh_dma_chain;
			tx_dma_chain=tx_bh_dma_chain;
aft_tdm_bh_skip:
#endif	

			DEBUG_TEST ("%s: Calling Rx Chan=%i TdmvChan=%i\n",
					card->devname,chan->logic_ch_num,
					chan->tdmv_chan);
#if 1

#if 0			
#warning "RTP TAP ENABLED Not Finished"
                        if (card->u.aft.rtp_len) { 
		       		aft_rtp_tap(card,chan,
				    	rx_dma_chain->dma_virt+rx_offset,
				    	tx_dma_chain->dma_virt+tx_offset, 
				    	chan->mtu);
			}	
#endif   		
			WAN_TDMV_CALL(rx_chan,
					(&card->wan_tdmv,chan->tdmv_chan,
					rx_dma_chain->dma_virt+rx_offset,
					tx_dma_chain->dma_virt+tx_offset),
					err);
#else
#warning "NCDEBUG rx_chan disabled irq"
#endif
			
#if 0
			if (((u8*)(rx_dma_chain->dma_virt+offset))[0] != 0xFF && 
                            ((u8*)(rx_dma_chain->dma_virt+offset))[0] != 0x7F &&
                             tx_debug_cnt < 100){
			DEBUG_EVENT("%s: %02X %02X %02X %02X %02X %02X %02X %02X\n",
				card->devname,
				((u8*)(rx_dma_chain->dma_virt+offset))[0],
				((u8*)(rx_dma_chain->dma_virt+offset))[1],
				((u8*)(rx_dma_chain->dma_virt+offset))[2],
				((u8*)(rx_dma_chain->dma_virt+offset))[3],
				((u8*)(rx_dma_chain->dma_virt+offset))[4],
				((u8*)(rx_dma_chain->dma_virt+offset))[5],
				((u8*)(rx_dma_chain->dma_virt+offset))[6],
				((u8*)(rx_dma_chain->dma_virt+offset))[7]);
				tx_debug_cnt++;
			}
#endif			  
		}else{
			return 1;
		}
#endif
	}else{
	
#ifdef AFT_TDM_API_SUPPORT 

		if (card->wandev.fe_iface.watchdog){
			err = card->wandev.fe_iface.watchdog(&card->fe);
		}
		
		wanpipe_tdm_api_rx_tx(&chan->wp_tdm_api_dev,
			    rx_dma_chain->dma_virt+rx_offset,
			    tx_dma_chain->dma_virt+tx_offset,
			    chan->mtu);
#endif
	}

	if (chan->cfg.tdmv_master_if){
		u32	reg;
		int	err;
		err=0;
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
		if (chan->tdmv_zaptel_cfg){
			DEBUG_TEST ("%s: Calling Master Rx Tx Chan=%i\n",
					card->devname,chan->logic_ch_num);
#if 0 
defined(AFT_TDMV_BH_ENABLE) 
#warning "AFT A104: TDM Driver compiled in BH mode!"			
		
			if (WAN_TASKLET_RUNNING((&chan->common.bh_task))){
				if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: Critical Error: TDMV BH Overrun!\n",
						card->devname);
				}
			}
			
			WAN_WP_TASKLET_SCHEDULE_PER_CPU((&chan->common.bh_task),
							card->tdmv_conf.span_no);	

			card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
			wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&reg);
			wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&reg);
			card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
	
#else
#if 1
			WAN_TDMV_CALL(rx_tx_span, (card), err);
#else
#warning "NCDEBUG: rx_tx_span disabled irq"
#endif   


         		if (!wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
				card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
				wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&reg);
				wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&reg);
				card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
			}
			
			if (card->wan_tdmv.sc){
				WAN_TDMV_CALL(is_rbsbits, (&card->wan_tdmv), err);
				if (err == 1){
					wan_set_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
					WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));	
				}
			}
#endif
		}else{
#else
		if (!chan->tdmv_zaptel_cfg){

#endif
         		if (!wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
				card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
				wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&reg);
				wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&reg);
				card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
			}
		}

		DEBUG_TEST("%s: Master device tx rx %i!\n",
				card->devname,chan->logic_ch_num);
	}

	chan->opstats.Data_frames_Rx_count++;
	chan->opstats.Data_bytes_Rx_count+=chan->mru;
	chan->opstats.Data_frames_Tx_count++;
	chan->opstats.Data_bytes_Tx_count+=chan->mtu;
	chan->if_stats.rx_packets++;
	chan->if_stats.rx_bytes += chan->mru;
	chan->if_stats.tx_packets++;
	chan->if_stats.tx_bytes += chan->mtu;
	
	return 0;
}

static int aft_fifo_intr_ctrl(sdla_t *card, int ctrl)
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

static int aft_tdm_intr_ctrl(sdla_t *card, int ctrl)
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

#if defined(AFT_API_SUPPORT)
static int wan_aft_api_ioctl(sdla_t *card, private_area_t *chan, char *user_data)
{
	api_tx_hdr_t		tx_hdr;
	wan_event_ctrl_t	event_ctrl;
	int			err = -EINVAL;
	
	if (WAN_COPY_FROM_USER(&tx_hdr, user_data, sizeof(api_tx_hdr_t))){
		DEBUG_EVENT("%s: Failed to copy data from user space!\n",
					card->devname);
		return -EFAULT;
	}
	memset(&event_ctrl, 0, sizeof(wan_event_ctrl_t));
	switch(tx_hdr.wp_api_tx_hdr_event_type){
	case WP_API_EVENT_DTMF:
		DEBUG_TEST("%s: %s HW DTMF events!\n",
			card->devname,
			(tx_hdr.wp_api_tx_hdr_event_mode==WP_API_EVENT_ENABLE)?
						"Enable": "Disable");
		event_ctrl.type   = WAN_EVENT_EC_DTMF;	
		event_ctrl.ts_map = tx_hdr.wp_api_tx_hdr_event_channel;
		if (tx_hdr.wp_api_tx_hdr_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
		
	case WP_API_EVENT_RM_DTMF:
		DEBUG_TEST("%s: %s RM DTMF events!\n",
			card->devname,
			(tx_hdr.wp_api_tx_hdr_event_mode==WP_API_EVENT_ENABLE)?
						"Enable": "Disable");
		event_ctrl.type   = WAN_EVENT_RM_DTMF;	
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		if (tx_hdr.wp_api_tx_hdr_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
		
	case WP_API_EVENT_RXHOOK:
		DEBUG_TEST("%s: %s OFFHOOK/ONHOOK events!\n",
			card->devname,
			(tx_hdr.wp_api_tx_hdr_event_mode==WP_API_EVENT_ENABLE)?
						"Enable": "Disable");
		event_ctrl.type   = WAN_EVENT_RM_LC;	
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		if (tx_hdr.wp_api_tx_hdr_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
	case WP_API_EVENT_RING:
		DEBUG_TEST("%s: %s RING events!\n",
			card->devname,
			(tx_hdr.wp_api_tx_hdr_event_mode==WP_API_EVENT_ENABLE)?
						"Enable": "Disable");
		event_ctrl.type   = WAN_EVENT_RM_RING;	
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		if (tx_hdr.wp_api_tx_hdr_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
	case WP_API_EVENT_TONE:
		DEBUG_TEST("%s: %s TONE events!\n",
			card->devname,
			(tx_hdr.wp_api_tx_hdr_event_mode==WP_API_EVENT_ENABLE)?
						"Enable": "Disable");
		event_ctrl.type   = WAN_EVENT_RM_TONE;	
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		switch(tx_hdr.wp_api_tx_hdr_event_tone){
		case WP_API_EVENT_TONE_DIAL:
			event_ctrl.tone	= WAN_EVENT_TONE_DIAL;
			break;
		case WP_API_EVENT_TONE_BUSY:
			event_ctrl.tone	= WAN_EVENT_TONE_BUSY;
			break;
		case WP_API_EVENT_TONE_RING:
			event_ctrl.tone	= WAN_EVENT_TONE_RING;
			break;
		case WP_API_EVENT_TONE_CONGESTION:
			event_ctrl.tone	= WAN_EVENT_TONE_CONGESTION;
			break;
		default:
			if (tx_hdr.wp_api_tx_hdr_event_mode == WP_API_EVENT_ENABLE){
				DEBUG_EVENT("%s: Unsupported tone type %d!\n",
						card->devname,
						tx_hdr.wp_api_tx_hdr_event_tone);
				return -EINVAL;
			}
			break;
		}
		if (tx_hdr.wp_api_tx_hdr_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode	= WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode	= WAN_EVENT_DISABLE;	
		}
		err = aft_event_ctrl(chan, &event_ctrl);
		break;

	case WP_API_EVENT_TXSIG_KEWL:
		DEBUG_EVENT("%s: TXSIG KEWL for module %d!\n",
				card->devname, 
				tx_hdr.wp_api_tx_hdr_event_channel);
		event_ctrl.type   = WAN_EVENT_RM_TXSIG_KEWL;	
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
	case WP_API_EVENT_TXSIG_START:
		DEBUG_EVENT("%s: TXSIG START for module %d!\n",
				card->devname, 
				tx_hdr.wp_api_tx_hdr_event_channel);
		event_ctrl.type   = WAN_EVENT_RM_TXSIG_START;
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
	case WP_API_EVENT_TXSIG_OFFHOOK:
		DEBUG_EVENT("%s: RM TXSIG OFFHOOK for module %d!\n",
				card->devname,
				tx_hdr.wp_api_tx_hdr_event_channel);
		event_ctrl.type   = WAN_EVENT_RM_TXSIG_OFFHOOK;
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
	case WP_API_EVENT_TXSIG_ONHOOK:
		DEBUG_EVENT("%s: RM TXSIG ONHOOK for module %d!\n",
				card->devname,
				tx_hdr.wp_api_tx_hdr_event_channel);
		event_ctrl.type   = WAN_EVENT_RM_TXSIG_ONHOOK;
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
	case WP_API_EVENT_ONHOOKTRANSFER:
		DEBUG_EVENT("%s: RM ONHOOKTRANSFER for module %d!\n",
				card->devname,
				tx_hdr.wp_api_tx_hdr_event_channel);
		event_ctrl.type   = WAN_EVENT_RM_ONHOOKTRANSFER;
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		event_ctrl.ohttimer = tx_hdr.wp_api_tx_hdr_event_ohttimer;
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
	case WP_API_EVENT_SETPOLARITY:
		DEBUG_EVENT("%s: RM SETPOLARITY for module %d!\n",
				card->devname,
				tx_hdr.wp_api_tx_hdr_event_channel);
		event_ctrl.type   = WAN_EVENT_RM_SETPOLARITY;
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		event_ctrl.polarity = tx_hdr.wp_api_tx_hdr_event_polarity;
		err = aft_event_ctrl(chan, &event_ctrl);
		break;
	case WP_API_EVENT_RING_DETECT:
		DEBUG_EVENT("%s: %s: RM RING DETECT events for module %d!\n",
			card->devname,
			WP_API_EVENT_MODE_DECODE(tx_hdr.wp_api_tx_hdr_event_mode),
			tx_hdr.wp_api_tx_hdr_event_channel);
		event_ctrl.type   = WAN_EVENT_RM_RING_DETECT;	
		event_ctrl.mod_no = tx_hdr.wp_api_tx_hdr_event_channel;
		if (tx_hdr.wp_api_tx_hdr_event_mode == WP_API_EVENT_ENABLE){
			event_ctrl.mode = WAN_EVENT_ENABLE;	
		}else{
			event_ctrl.mode = WAN_EVENT_DISABLE;	
		}
		err = aft_event_ctrl(chan, &event_ctrl);
		break;

	default:
		DEBUG_EVENT("%s: Unknown event type %02X!\n",
				card->devname,
				tx_hdr.wp_api_tx_hdr_event_type);
		err = -EINVAL;
		break;
	}
	return err;
}
#endif

#if defined(AFT_API_SUPPORT)
static void wan_aft_api_dtmf (void* card_id, wan_event_t *event)
{
	sdla_t		*card = (sdla_t*)card_id;
	private_area_t	*chan = NULL;
	netskb_t	*new_skb = NULL;
	api_rx_hdr_t 	*rx_hdr;
	int		i;
	
	if (event->type == WAN_EVENT_EC_DTMF){
		DEBUG_TEST("%s: Received DTMF Event at AFT API (%d:%c:%s:%s)!\n",
			card->devname,
			event->channel,
			event->digit,
			(event->dtmf_port == WAN_EC_CHANNEL_PORT_ROUT)?"ROUT":"SOUT",
			(event->dtmf_type == WAN_EC_TONE_PRESENT)?"PRESENT":"STOP");
	}else if (event->type == WAN_EVENT_RM_DTMF){
		DEBUG_TEST("%s: Received DTMF Event at AFT API (%d:%c)!\n",
			card->devname,
			event->channel,
			event->digit);	
	}

	for (i=0;i<card->u.aft.num_of_time_slots;i++){
		if (wan_test_bit(i,&card->u.aft.logic_ch_map)){
			unsigned long	ts_map;
			chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
			ts_map = chan->time_slot_map;
			if (IS_T1_CARD(card) || IS_FXOFXS_CARD(card)){
				/* Convert active_ch bit map to user */
				ts_map	= ts_map << 1;
			}			
			if (wan_test_bit(event->channel,&ts_map)){
				break;
			}
			chan = NULL;
		}
	}
	if (chan == NULL){
	 	DEBUG_EVENT("%s: Failed to find channel device (channel=%d)!\n",
					card->devname, event->channel);
		return;
	}
#if defined(__LINUX__)
	new_skb=wan_skb_alloc(sizeof(api_rx_element_t));
	if (new_skb == NULL) return;
		
	rx_hdr=(api_rx_hdr_t*)wan_skb_put(new_skb,sizeof(api_rx_element_t));
	memset(rx_hdr,0,sizeof(api_rx_hdr_t));
		
	rx_hdr->error_flag 			= 0;
	rx_hdr->event_type 			= WP_API_EVENT_DTMF;	
	rx_hdr->wp_api_rx_hdr_event_channel	= event->channel;
	rx_hdr->wp_api_rx_hdr_event_dtmf_digit	= event->digit;
	rx_hdr->wp_api_rx_hdr_event_dtmf_type	= event->dtmf_type;
	rx_hdr->wp_api_rx_hdr_event_dtmf_port	= event->dtmf_port;

	new_skb->protocol = htons(PVC_PROT);
	new_skb->mac.raw  = new_skb->data;
	new_skb->dev      = chan->common.dev;
	new_skb->pkt_type = WAN_PACKET_DATA;	
	
	//if (wan_api_rx_dtmf(chan,new_skb) != 0){
	if (wan_api_rx(chan,new_skb) != 0){
		DEBUG_EVENT("%s: Failed to send up DTMF event!\n",
						card->devname);
		wan_skb_free(new_skb);
	}
#else
	DEBUG_EVENT("%s:%s: DTMF Event report is not supported!\n",
				card->devname, chan->if_name);
	new_skb = NULL;
	rx_hdr = NULL;
#endif
	return;
}
#endif

#if defined(AFT_API_SUPPORT)
static void wan_aft_api_hook (void* card_id, wan_event_t *event)
{
	sdla_t		*card = (sdla_t*)card_id;
	private_area_t	*chan = NULL;
	netskb_t	*new_skb = NULL;
	api_rx_hdr_t 	*rx_hdr;
	int		i;
	
	if (event->type != WAN_EVENT_RM_LC){
		DEBUG_EVENT("ERROR: %s: Invalid Event type (%04X)!\n",
				card->devname, event->type);
		return;
	}
	DEBUG_EVENT("%s: Received %s (%d) Event at AFT API (%d)!\n",
			card->devname,
			WAN_EVENT_RXHOOK_DECODE(event->rxhook), event->rxhook,
			event->channel);

	for (i=0;i<card->u.aft.num_of_time_slots;i++){
		if (wan_test_bit(i,&card->u.aft.logic_ch_map)){
			unsigned long	ts_map;
			chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
			ts_map = chan->time_slot_map;
			if (IS_T1_CARD(card) || IS_FXOFXS_CARD(card)){
				/* Convert active_ch bit map to user */
				ts_map	= ts_map << 1;
			}			
			if (wan_test_bit(event->channel,&ts_map)){
				break;
			}
			chan = NULL;
		}
	}
	if (chan == NULL){
	 	DEBUG_EVENT("%s: Failed to find channel device (channel=%d)!\n",
					card->devname, event->channel);
		return;
	}
#if defined(__LINUX__)
	new_skb=wan_skb_alloc(sizeof(api_rx_element_t));
	if (new_skb == NULL) return;
		
	rx_hdr=(api_rx_hdr_t*)wan_skb_put(new_skb,sizeof(api_rx_element_t));
	memset(rx_hdr,0,sizeof(api_rx_hdr_t));
		
	rx_hdr->error_flag 			= 0;
	rx_hdr->event_type 			= WP_API_EVENT_RXHOOK;	
	rx_hdr->wp_api_rx_hdr_event_channel	= event->channel;
	if (event->rxhook == WAN_EVENT_RXHOOK_OFF){
		rx_hdr->wp_api_rx_hdr_event_rxhook_state = WP_API_EVENT_RXHOOK_OFF;
	}else if (event->rxhook == WAN_EVENT_RXHOOK_ON){
		rx_hdr->wp_api_rx_hdr_event_rxhook_state = WP_API_EVENT_RXHOOK_ON;	
	}

	new_skb->protocol = htons(PVC_PROT);
	new_skb->mac.raw  = new_skb->data;
	new_skb->dev      = chan->common.dev;
	new_skb->pkt_type = WAN_PACKET_DATA;	
	
	if (wan_api_rx(chan,new_skb) != 0){
		DEBUG_EVENT("%s: Failed to send up HOOK event!\n",
						card->devname);
		wan_skb_free(new_skb);
	}
#else
	DEBUG_EVENT("%s:%s: RXHOOK Event report is not supported!\n",
				card->devname, chan->if_name);
	new_skb = NULL;
	rx_hdr = NULL;
#endif
	return;
}
#endif

#if defined(AFT_API_SUPPORT)
static void wan_aft_api_ringtrip (void* card_id, wan_event_t *event)
{
	sdla_t		*card = (sdla_t*)card_id;
	
	DEBUG_EVENT("%s: Unsupported event!\n", card->devname);
	return;
}
#endif

#if defined(AFT_API_SUPPORT)
static void wan_aft_api_ringdetect (void* card_id, wan_event_t *event)
{
	sdla_t		*card = (sdla_t*)card_id;
	private_area_t	*chan = NULL;
	netskb_t	*new_skb = NULL;
	api_rx_hdr_t 	*rx_hdr;
	int		i;
	
	if (event->type != WAN_EVENT_RM_RING_DETECT){
		DEBUG_EVENT("ERROR: %s: Invalid Event type (%04X)!\n",
				card->devname, event->type);
		return;
	}
	DEBUG_EVENT("%s: Received Ring Detect %s (%d) Event at AFT API (%d)!\n",
			card->devname,
			WAN_EVENT_RING_DECODE(event->ring_mode), event->ring_mode,
			event->channel);

	for (i=0;i<card->u.aft.num_of_time_slots;i++){
		if (wan_test_bit(i,&card->u.aft.logic_ch_map)){
			unsigned long	ts_map;
			chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
			ts_map = chan->time_slot_map;
			if (IS_T1_CARD(card) || IS_FXOFXS_CARD(card)){
				/* Convert active_ch bit map to user */
				ts_map	= ts_map << 1;
			}			
			if (wan_test_bit(event->channel,&ts_map)){
				break;
			}
			chan = NULL;
		}
	}
	if (chan == NULL){
	 	DEBUG_EVENT("%s: Failed to find channel device (channel=%d)!\n",
					card->devname, event->channel);
		return;
	}
#if defined(__LINUX__)
	new_skb=wan_skb_alloc(sizeof(api_rx_element_t));
	if (new_skb == NULL) return;
		
	rx_hdr=(api_rx_hdr_t*)wan_skb_put(new_skb,sizeof(api_rx_element_t));
	memset(rx_hdr,0,sizeof(api_rx_hdr_t));

	rx_hdr->error_flag 			= 0;
	rx_hdr->event_type 			= WP_API_EVENT_RING_DETECT;	
	rx_hdr->wp_api_rx_hdr_event_channel	= event->channel;
	if (event->ring_mode == WAN_EVENT_RING_PRESENT){
		rx_hdr->wp_api_rx_hdr_event_ringdetect_state = WAN_EVENT_RING_PRESENT;
	}else if (event->ring_mode == WAN_EVENT_RING_STOP){
		rx_hdr->wp_api_rx_hdr_event_ringdetect_state = WAN_EVENT_RING_STOP;	
	}else{
	 	DEBUG_EVENT("%s: Invalid Rind Detect mode (%d)!\n",
					card->devname, event->ring_mode);
		wan_skb_free(new_skb);
		return ;
	}

	new_skb->protocol = htons(PVC_PROT);
	new_skb->mac.raw  = new_skb->data;
	new_skb->dev      = chan->common.dev;
	new_skb->pkt_type = WAN_PACKET_DATA;	
	
	if (wan_api_rx(chan,new_skb) != 0){
		DEBUG_EVENT("%s: Failed to send up HOOK event!\n",
						card->devname);
		wan_skb_free(new_skb);
	}
#else
	DEBUG_EVENT("%s:%s: RXHOOK Event report is not supported!\n",
				card->devname, chan->if_name);
	new_skb = NULL;
	rx_hdr = NULL;
#endif
	return;
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
			++chan->if_stats.rx_errors;
			goto wp_tdmv_api_rx_tx_chan_skip_rx;
		}
	}

	if (wan_skb_len(card->u.aft.tdmv_api_rx) > (card->u.aft.tdmv_mtu-chan->mru)){
		/* CRITICAL ERROR: We cannot fit the all timeslots into this
		 * packet */
		++chan->if_stats.rx_errors;
		goto wp_tdmv_api_rx_tx_chan_skip_rx;
	}

	buf=wan_skb_put(card->u.aft.tdmv_api_rx, chan->mru);
	memcpy(buf,rxdata,chan->mru);
	++chan->if_stats.rx_dropped;

wp_tdmv_api_rx_tx_chan_skip_rx:

	if (!card->u.aft.tdmv_api_tx){
		card->u.aft.tdmv_api_tx=wan_skb_dequeue(&card->u.aft.tdmv_api_tx_list);
		if (!card->u.aft.tdmv_api_tx){
			/* No tx packet, send idle frames */
			++chan->if_stats.tx_carrier_errors;
			memset(txdata,chan->idle_flag,chan->mtu);
			return;
		}
	}

	if (wan_skb_len(card->u.aft.tdmv_api_tx) < chan->mtu){
		/* CRITICAL ERROR: 
		 * The tx api packet must have info for all
		 * timeslots */
		memset(txdata,chan->idle_flag,chan->mtu);
		++chan->if_stats.tx_errors;
		return;
	}
	
	buf=wan_skb_data(card->u.aft.tdmv_api_tx);
	memcpy(txdata,buf,chan->mtu);
	wan_skb_pull(card->u.aft.tdmv_api_tx,chan->mtu);
	++chan->if_stats.tx_dropped;

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

	if (wan_skb_headroom(card->u.aft.tdmv_api_rx) >= sizeof(api_rx_hdr_t)){
		api_rx_hdr_t *rx_hdr=
			(api_rx_hdr_t*)skb_push(card->u.aft.tdmv_api_rx,
						sizeof(api_rx_hdr_t));	
			memset(rx_hdr,0,sizeof(api_rx_hdr_t));
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

#if defined(__LINUX__)
/*=============================================
 * aft_set_ss7_force_rx
 * 
 * Force the firmware to pass up a single
 * ss7 packet.  Otherwise, the ss7 engine
 * will wait for the next different packet.
 */
static void aft_set_ss7_force_rx(sdla_t *card, private_area_t *chan)
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
#endif

/*=============================================
 * aft_clear_ss7_force_rx
 * 
 * Reset the ss7 force rx logic.
 * This must be done before trying to enable
 * force rx again. 
 * Note: That first_time_slot must be cleared LAST.
 *       Thus the reverse clear order.
 */
static void aft_clear_ss7_force_rx(sdla_t *card, private_area_t *chan)
{
	wan_clear_bit(0,&chan->ss7_force_rx);
}


#if defined(AFT_API_SUPPORT) || defined(AFT_TDM_API_SUPPORT)
static int aft_event_ctrl(void *chan_ptr, wan_event_ctrl_t *p_event)
{
	sdla_t		*card = NULL;
	private_area_t 	*chan = (private_area_t *)chan_ptr;
	wan_event_ctrl_t *event_ctrl;
	int		err = -EINVAL;
	
	card = chan->card;
	event_ctrl = wan_malloc(sizeof(wan_event_ctrl_t));
	if (event_ctrl == NULL){
		DEBUG_EVENT("%s: Failed to allocate event control!\n",
					card->devname);
		return -EFAULT;	
	}
	memset(event_ctrl, 0, sizeof(wan_event_ctrl_t));
	memcpy(event_ctrl, p_event, sizeof(wan_event_ctrl_t));
	
	if (event_ctrl->type == WAN_EVENT_EC_DTMF && card->wandev.ec_dev){
#if defined(CONFIG_WANPIPE_HWEC)
		DEBUG_TEST("%s: Event control request EC_DTMF...\n",
					chan->if_name);	
		err = wanpipe_ec_event_ctrl(card->wandev.ec_dev, card, event_ctrl);	
#endif
	}else if (chan->card->wandev.fe_iface.event_ctrl){
		
		DEBUG_TEST("%s: FE Event control request...\n",
					chan->if_name);		
		err = chan->card->wandev.fe_iface.event_ctrl(
					&chan->card->fe, event_ctrl);
	}else{
		DEBUG_TEST("%s: Unsupported event control request (%lX)\n",
					chan->if_name, event_ctrl->type);
	}	
	if (err){
		if (event_ctrl) wan_free(event_ctrl);
	}
	return err;
}
#endif

#ifdef AFT_TDM_API_SUPPORT
static int aft_read_rbs_bits(void *chan_ptr, u32 ch, u8 *rbs_bits)
{
	private_area_t *chan = (private_area_t *)chan_ptr;
	wan_smp_flag_t flags;
	
	if (!chan_ptr){
		return -EINVAL;
	}
	
	chan->card->hw_iface.hw_lock(chan->card->hw,&flags);
	*rbs_bits = chan->card->wandev.fe_iface.read_rbsbits(
						&chan->card->fe, 
						chan->logic_ch_num+1,
						WAN_TE_RBS_UPDATE);
	chan->card->hw_iface.hw_unlock(chan->card->hw,&flags);
	
	return 0; 
	
}
 
static int aft_write_rbs_bits(void *chan_ptr, u32 ch, u8 rbs_bits)
{
	private_area_t *chan = (private_area_t *)chan_ptr;
	wan_smp_flag_t flags;
	int err;
	
	if (!chan_ptr){
		return -EINVAL;
	}
	 
	chan->card->hw_iface.hw_lock(chan->card->hw,&flags);
	err = chan->card->wandev.fe_iface.set_rbsbits(&chan->card->fe, 
					  chan->logic_ch_num+1, 
					  rbs_bits);
	chan->card->hw_iface.hw_unlock(chan->card->hw,&flags);	
	
	return err;
}

 
static int aft_write_hdlc_frame(void *chan_ptr, netskb_t *skb)
{
	private_area_t *chan = (private_area_t *)chan_ptr;
	sdla_t *card=chan->card;
	wan_smp_flag_t smp_flags;
	int err=-EINVAL;
	
	if (!chan_ptr || !chan->common.dev || !card){
		WAN_ASSERT(1);
		return -EINVAL;
	}
	
	if (wan_skb_len(skb) > chan->mtu) {
		return -EINVAL;
	}
	
	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);

	if (wan_skb_queue_len(&chan->wp_tx_pending_list) > chan->max_tx_bufs){
		aft_dma_tx(card,chan);	
		wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
		return -EBUSY;
		
	}

	wan_skb_unlink(skb);
	wan_skb_queue_tail(&chan->wp_tx_pending_list,skb);
	aft_dma_tx(card,chan);


	err=0;
	wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);

       

	return err;
}
#endif


static int aft_tdm_ring_rsync(sdla_t *card)
{
        int i; 
	private_area_t *chan;
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

		if (chan->channelized_cfg && !chan->hdlc_eng && chan->cfg.tdmv_master_if){

                        u32 lo_reg;
			u32 dma_descr=(chan->logic_ch_num<<4) + 
			  			AFT_PORT_REG(card,AFT_RX_DMA_LO_DESCR_BASE_REG);
			
			card->hw_iface.bus_read_4(card->hw,dma_descr,&lo_reg); 

			lo_reg=(lo_reg&AFT_TDMV_BUF_MASK)/AFT_TDMV_CIRC_BUF;
			
			if (card->wandev.ec_enable){
				/* HW EC standard */
				if (lo_reg > 0) {
		        		card->u.aft.tdm_rx_dma_toggle = lo_reg-1; 
		       	 	} else {
                         		card->u.aft.tdm_rx_dma_toggle = 3;
				}
			} else {
				/* Software ec moves spike to 8bytes */ 
				card->u.aft.tdm_rx_dma_toggle=lo_reg+1;
				if (card->u.aft.tdm_rx_dma_toggle > 3) {
                        		card->u.aft.tdm_rx_dma_toggle=0; 	
				}
			}

			if (lo_reg < 3) {
		        	card->u.aft.tdm_tx_dma_toggle = lo_reg+1; 
			} else {
                         	card->u.aft.tdm_tx_dma_toggle = 0;
			}

			DEBUG_EVENT("%s: Card TDM Rsync Rx=%i Tx=%i\n",
					   card->devname,
					   card->u.aft.tdm_rx_dma_toggle,
					   card->u.aft.tdm_tx_dma_toggle);
		}
	}                   

	return 0;
}

static void aft_critical_shutdown (sdla_t *card)
{
	DEBUG_EVENT("%s: Error: Card Critically Shutdown!\n",
			card->devname);
	
	if (card->wandev.fe_iface.pre_release){
		card->wandev.fe_iface.pre_release(&card->fe);
	}
	if (card->wandev.fe_iface.unconfig){
		card->wandev.fe_iface.unconfig(&card->fe);
	}    
	disable_data_error_intr(card,DEVICE_DOWN);
	wan_set_bit(CARD_DOWN,&card->wandev.critical);	  
       	port_set_state(card,WAN_DISCONNECTED);

	aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_RED, 0,WAN_AFT_ON);
	aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_GREEN, 0, WAN_AFT_OFF);
}

enum {
	WAN_TDM_RTP_NO_CHANGE,
	WAN_TDM_RTP_CALL_START,
	WAN_TDM_RTP_CALL_STOP
};


static int aft_hwec_config (sdla_t *card, private_area_t *chan, wanif_conf_t *conf, int ctrl)
{
	int err = 0;
	unsigned int channel=0;
	unsigned int tdmv_hwec_option=0;
	
	if (conf) {
        	tdmv_hwec_option=conf->hwec.enable;
	}

	if (chan->common.usedby == TDM_VOICE_API ||
	    chan->common.usedby == TDM_VOICE){

		if (IS_TE1_CARD(card)) {
			if (IS_T1_CARD(card)){
				channel = chan->first_time_slot;
			}else{
				channel = chan->first_time_slot;
			}
		} else {
		       	channel = chan->first_time_slot; 
		}              
		
#if defined(CONFIG_WANPIPE_HWEC)  
		if (ctrl == 0 && card->wandev.ec_enable){ 
                	card->wandev.ec_enable(card, 0, channel);
			
		} else if (tdmv_hwec_option && card->wandev.ec_enable){
			DEBUG_HWEC("%s: HW echo canceller Enabled on channel %d\n",
					chan->if_name,
					channel);
			err = card->wandev.ec_enable(card, 1, channel);
			if (err) {
				DEBUG_EVENT("%s: Failed to enable HWEC on channel %d\n",
						chan->if_name,channel);
			 	return err;       		
			}
		}          
#endif	
	}

	return err;
}               


#if defined(__LINUX__)

static void aft_rtp_unconfig(sdla_t *card)  
{
        netskb_t *skb;
	
	card->u.aft.cfg.rtp_ip=0;
	card->u.aft.cfg.rtp_sample=0;
	card->u.aft.rtp_len=0;    
	if (card->u.aft.rtp_dev) {
              	dev_put(card->u.aft.rtp_dev);
		card->u.aft.rtp_dev=NULL;
	}

	if ((skb=card->u.aft.rx_rtp_skb)) {
		card->u.aft.rx_rtp_skb=NULL;
         	wan_skb_free(skb);
	}

}           

static void aft_rtp_config(sdla_t *card)
{

/* RTP TAP Not finished yet */
	return;

	
#if 0
	if (!card->u.aft.cfg.rtp_ip || !card->u.aft.cfg.rtp_sample) {
		goto aft_rtp_init_exit;
	}

	switch (card->u.aft.cfg.rtp_sample) {
		case 10:
		case 20:
		case 30:
		case 40:
			/* Supported Sample Sizes */
			break;
		default:
			goto aft_rtp_init_exit;
	}
#endif

	card->u.aft.cfg.rtp_port=9000;
	card->u.aft.cfg.rtp_ip=0x0100007F;
	card->u.aft.cfg.rtp_sample=100;
	card->u.aft.rtp_len = (card->u.aft.cfg.rtp_sample * 8) + sizeof(wan_rtp_pkt_t); 
	sprintf(card->u.aft.cfg.rtp_devname,"lo");

	if ((card->u.aft.rtp_dev=dev_get_by_name(card->u.aft.cfg.rtp_devname)) == NULL){
		DEBUG_EVENT("%s: Failed to open rtp tx device %s\n", 
				card->devname,
				card->u.aft.cfg.rtp_devname);
        	goto aft_rtp_init_exit;	
	}	

	DEBUG_EVENT("%s: Configured rtp tap!\n",card->devname);
	return;

aft_rtp_init_exit:

	aft_rtp_unconfig(card);

	DEBUG_EVENT("%s: Failed to configure rtp tap!\n",card->devname);
	return;

}


#if 0
static void aft_rtp_tap(sdla_t *card, private_area_t *chan, u8* rx, u8* tx, u32 len) 
{
	wan_rtp_pkt_t *pkt;
	u8 *buf;
	netskb_t *skb;	
	u32 ecmap, call_status=WAN_TDM_RTP_NO_CHANGE;
	
	if (!card->u.aft.cfg.rtp_ip || 
	    !card->u.aft.cfg.rtp_sample ||
	    !card->u.aft.rtp_len ||
	    !card->u.aft.rtp_dev) {
         	return;
	}

	if (IS_T1_CARD(card)) {
	 	ecmap=card->wandev.ec_map;
        } else {
		ecmap=card->wandev.ec_map << 1;    
	}

	if (1) { //wan_test_bit(chan->first_time_slot,&ecmap)) {
		if (!wan_test_and_set_bit(0,&chan->tdm_call_status)) {
        		/* Start of the call */
			call_status=WAN_TDM_RTP_CALL_START;
			DEBUG_TEST("%s: CALL Start on ch %i\n",
					chan->if_name,chan->first_time_slot);
		}
	} else {
               	if (!wan_test_bit(0,&chan->tdm_call_status)) {  
			/* Call not up */
        	     	return;   	
		} 
		call_status=WAN_TDM_RTP_CALL_STOP; 		
	       	DEBUG_TEST("%s: CALL Stop on ch %i\n",
	       		chan->if_name,chan->first_time_slot);
	}

	 	
	if (!(skb=chan->rx_rtp_skb)) {
		card->u.aft.rtp_len = (card->u.aft.cfg.rtp_sample * 8) + sizeof(wan_rtp_pkt_t);
		chan->rx_rtp_skb=wan_skb_alloc(card->u.aft.rtp_len+128);
		if (!chan->rx_rtp_skb) {
                 	return;
		}
		skb=chan->rx_rtp_skb;
		pkt = (wan_rtp_pkt_t*)wan_skb_put(skb,sizeof(wan_rtp_pkt_t));
		memset(pkt,0,sizeof(wan_rtp_pkt_t));
		pkt->rtp_hdr.version=2;
		if (IS_T1_CARD(card)) {
                	pkt->rtp_hdr.pt=0;
		} else {
                        pkt->rtp_hdr.pt=1;
		}	
	} 
	
	pkt = (wan_rtp_pkt_t*)wan_skb_data(skb);

	if (call_status == WAN_TDM_RTP_CALL_START) {
        	pkt->rtp_hdr.seq=0;
		pkt->rtp_hdr.ts=0;
	}

	buf=wan_skb_put(skb,len);
	memcpy(buf,rx,len);
	pkt->rtp_hdr.ts++;

        if (wan_skb_len(skb) >= card->u.aft.rtp_len || 
	    call_status==WAN_TDM_RTP_CALL_STOP) {
	        netskb_t *nskb;
		wan_ip_udp_setup(card,
				 card->u.aft.cfg.rtp_ip,
				 card->u.aft.cfg.rtp_port+chan->first_time_slot,
				 wan_skb_data(skb),
				 wan_skb_len(skb)-sizeof(wan_rtp_pkt_t)); 
		nskb=wan_skb_clone(skb);
		if (nskb) {
                	nskb->next = nskb->prev = NULL;
			nskb->dev = card->u.aft.rtp_dev;
			nskb->protocol = htons(ETH_P_802_2);
			nskb->mac.raw  = wan_skb_data(nskb);  
			//nskb->nh.raw   = wan_skb_data(nskb);
			dev_queue_xmit(nskb);     
		}
		wan_skb_trim(skb,sizeof(wan_rtp_pkt_t));
                pkt->rtp_hdr.seq++;

		#if 0
		/* Disable rtp (debugging) */
		card->u.aft.rtp_len=0;
		#endif
		
	}	
}
#endif
#endif


static int aft_find_master_if_and_dchan(sdla_t *card, int *master_if, u32 active_ch)
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
		wan_skb_push(skb, sizeof(api_rx_hdr_t));
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


	skb->next = skb->prev = NULL;
        skb->dev = dev;
        skb->protocol = htons(ETH_P_IP);
        skb->mac.raw  = wan_skb_data(skb);
        dev_queue_xmit(skb);

	return 0;
}
/****** End ****************************************************************/
