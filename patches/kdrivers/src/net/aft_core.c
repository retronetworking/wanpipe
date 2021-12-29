/*****************************************************************************
* aft_core.c
*
* 		WANPIPE(tm) AFT CORE Hardware Support
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003-2008 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Feb 04,2008	Nenad Corbic	Support for RTP TAP, Fixed 56K
* Mar 16,2007	David Rokhvarg	Support for ISDN BRI card.
* Oct 25,2004	Nenad Corbic	Support for QuadPort TE1
* Jan 07,2003	Nenad Corbic	Initial version.
*****************************************************************************/

#include "wanpipe_includes.h"
#include "wanpipe_defines.h"
#include "wanpipe.h"
#include "wanpipe_abstr.h"
#include "wanpipe_snmp.h"
#include "if_wanpipe_common.h"    /* Socket Driver common area */
#include "sdlapci.h"
#include "wanpipe_iface.h"
#include "sdla_tdmv_dummy.h"
#include "wanpipe_tdm_api.h"
#include "aft_core_options.h"

#if defined(CONFIG_WANPIPE_HWEC)
# include "wanec_iface.h"
#endif

#include "aft_core.h"



/*=================================================================
 * Debugging/Feature Defines
 *================================================================*/

#define INIT_FE_ONLY 0

#if 1
#undef AFT_FUNC_DEBUG
#define AFT_FUNC_DEBUG()
#else
#undef AFT_FUNC_DEBUG
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
# pragma message("DISABLED WDT")
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
# if defined(__WINDOWS__)
#  pragma message("AFT_SINGLE_DMA_CHAIN: SET")
# else
#  warning "AFT_SINGLE_DMA_CHAIN: SET"
# endif
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

#ifdef AFT_FE_INTR_DEBUG
# if defined(__WINDOWS__)
#  pragma message("AFT_FE_INTR_DEBUG defined")
# else
#  warning "AFT_FE_INTR_DEBUG defined"
# endif
#endif

#if 0
# warning "AFT_SPAN_SINGLE_IRQ Enabled"
# define AFT_SPAN_SINGLE_IRQ
#else
# undef AFT_SPAN_SINGLE_IRQ
#endif

#if 0
# warning "AFT_SERIAL_DEBUGGING is enabled"
# define AFT_SINGLE_DMA_CHAIN 1
# define AFT_SERIAL_DEBUG
#else
# undef AFT_SERIAL_DEBUG
#endif


#if 0
#define AFT_FIFO_GEN_DEBUGGING
# if defined(__WINDOWS__)
#  pragma message("Warning: AFT_FIFO_GEN_DEBUGGING Enabled")
# else
#  warning "Warning: AFT_FIFO_GEN_DEBUGGING Enabled"
# endif
#else
# undef AFT_FIFO_GEN_DEBUGGING
#endif

#if defined(WANPIPE_64BIT_4G_DMA)
#warning "Wanpipe compiled for 64bit 4G DMA"
#endif


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

#define WP_ZAPTEL_ENABLED	     0
#define WP_ZAPTEL_DCHAN_OPTIMIZATION 1



/*=================================================================
 * Defines & Macros
 *================================================================*/




/*=================================================================
 * Static Defines
 *================================================================*/


/* Static index of hw interface pointers */
aft_hw_dev_t aft_hwdev[MAX_AFT_HW_DEV];

static int aft_rx_copyback=500;

/*=================================================================
 * Function Prototypes
 *================================================================*/


extern wan_iface_t wan_iface;

extern void disable_irq(unsigned int);
extern void enable_irq(unsigned int);

int 		wp_aft_te1default_devcfg(sdla_t* card, wandev_conf_t* conf);
int 		wp_aft_te1default_ifcfg(sdla_t* card, wanif_conf_t* conf);


/*=================================================================
 * Private Function Prototypes
 *================================================================*/


/* WAN link driver entry points. These are called by the WAN router module. */
static int 	update (wan_device_t* wandev);
static int 	new_if (wan_device_t* wandev, netdevice_t* dev, wanif_conf_t* conf);
static int 	del_if(wan_device_t *wandev, netdevice_t *dev);

/* Network device interface */

static int 	if_open   (netdevice_t* dev);
static int 	if_close  (netdevice_t* dev);
static int 	if_do_ioctl(netdevice_t*, struct ifreq*, wan_ioctl_cmd_t);

#if defined(__LINUX__) || defined(__WINDOWS__)
static int 	if_init   (netdevice_t* dev);
static int 	if_send (netskb_t* skb, netdevice_t* dev);
static struct net_device_stats* if_stats (netdevice_t* dev);
extern int if_change_mtu(netdevice_t *dev, int new_mtu);
#else
static int	if_send(netdevice_t*, netskb_t*, struct sockaddr*,struct rtentry*);
#endif

static void 	handle_front_end_state(void* card_id, int lock);
static void 	enable_timer(void* card_id);
static void 	enable_ec_timer(void* card_id);
static void 	if_tx_timeout (netdevice_t *dev);

/* Miscellaneous Functions */
static void 	port_set_state (sdla_t *card, u8);
static void 	disable_comm (sdla_t *card);
static int 		set_chan_state(sdla_t* card, netdevice_t* dev, u8 state);

/* Interrupt handlers */
static WAN_IRQ_RETVAL 	wp_aft_global_isr (sdla_t* card);
static void 	wp_aft_dma_per_port_isr(sdla_t *card);
static void 	wp_aft_tdmv_per_port_isr(sdla_t *card);
static void 	wp_aft_fifo_per_port_isr(sdla_t *card);
static void 	wp_aft_wdt_per_port_isr(sdla_t *card, int wdt_intr);
static void     wp_aft_serial_status_isr(sdla_t *card, u32 status);

static void 	front_end_interrupt(sdla_t *card, unsigned long reg, int lock);
static void  	enable_data_error_intr(sdla_t *card);
static void  	disable_data_error_intr(sdla_t *card, unsigned char);

static void 	aft_tx_fifo_under_recover (sdla_t *card, private_area_t *chan);
static void     aft_rx_fifo_over_recover(sdla_t *card, private_area_t *chan);


/* Bottom half handlers */
#if defined(__LINUX__)
static void 	wp_bh (unsigned long);
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
static void aft_port_task (void * card_ptr);
# else
static void aft_port_task (struct work_struct *work);
# endif
#elif defined(__WINDOWS__)
 static void wp_bh (IN PKDPC Dpc, IN PVOID data, IN PVOID SystemArgument1, IN PVOID SystemArgument2);
 static void aft_port_task (IN PKDPC Dpc, IN PVOID card_ptr, IN PVOID SystemArgument1, IN PVOID SystemArgument2);	
 void __aft_port_task (IN PVOID card_ptr);
 #if defined(WAN_SPINLOCK_IS_FASTMUTEX)
  extern int trigger_aft_port_task(sdla_t *card);
 #endif

extern
void
wan_debug_update_timediff(
	wan_debug_t	*wan_debug_ptr,
	const char *caller_name
	);

#else
static void wp_bh (void*,int);
static void aft_port_task (void * card_ptr, int arg);
#endif


/* Configuration functions */
static int 	aft_global_chip_configuration(sdla_t *card, wandev_conf_t* conf);
static int 	aft_global_chip_disable(sdla_t *card);

static int 	aft_chip_configure(sdla_t *card, wandev_conf_t* conf);
static int 	aft_chip_unconfigure(sdla_t *card);
static int 	aft_dev_configure(sdla_t *card, private_area_t *chan, wanif_conf_t* conf);
static void 	aft_dev_unconfigure(sdla_t *card, private_area_t *chan);

static void 	aft_dev_enable(sdla_t *card, private_area_t *chan);
static void 	aft_dev_close(sdla_t *card, private_area_t *chan);
static void 	aft_dev_open(sdla_t *card, private_area_t *gchan);
static void 	aft_dma_tx_complete (sdla_t *card, private_area_t *chan,int wdt, int reset);

/* Rx/Tx functions */
static int 	aft_dma_rx(sdla_t *card, private_area_t *chan);
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


int 		aft_dma_tx (sdla_t *card,private_area_t *chan);
static int	 	aft_tx_dma_chain_handler(private_area_t *data, int wdt, int reset);
static void 	aft_tx_dma_voice_handler(private_area_t *data, int wdt, int reset);
static void 	aft_tx_dma_chain_init(private_area_t *chan, wan_dma_descr_t *);
static void 	aft_rx_dma_chain_init(private_area_t *chan, wan_dma_descr_t *);
static void 	aft_index_tx_rx_dma_chains(private_area_t *chan);
static void 	aft_init_tx_rx_dma_descr(private_area_t *chan);
static void 	aft_free_rx_complete_list(private_area_t *chan);
static void 	aft_rx_cur_go_test(private_area_t *chan);
static void 	aft_free_rx_descriptors(private_area_t *chan);
static void 	aft_reset_rx_chain_cnt(private_area_t *chan);
static void 	aft_reset_tx_chain_cnt(private_area_t *chan);
static void 	aft_free_tx_descriptors(private_area_t *chan);

static void 	aft_data_mux_cfg(sdla_t *card);
static void 	aft_data_mux_get_cfg(sdla_t *card);

static int 	aft_ss7_tx_mangle(sdla_t *card,private_area_t *chan, netskb_t *skb);
static int 	aft_hdlc_repeat_mangle(sdla_t *card,private_area_t *chan, netskb_t *skb, netskb_t **rkb);

static int 	aft_tdmv_init(sdla_t *card, wandev_conf_t *conf);
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
static int 	aft_tdmv_free(sdla_t *card);
#endif
static int 	aft_tdmv_if_init(sdla_t *card, private_area_t *chan, wanif_conf_t *conf);
static int 	aft_tdmv_if_free(sdla_t *card, private_area_t *chan);

static void 	callback_front_end_state(void *card_id);
/* Procfs functions */
static int 	wan_aft_get_info(void* pcard, struct seq_file* m, int* stop_cnt);

static int 	wan_aft_init (sdla_t *card, wandev_conf_t* conf);
static void aft_critical_shutdown (sdla_t *card);
static void	aft_critical_trigger(sdla_t *card);
static int aft_kickstart_global_tdm_irq(sdla_t *card);

#if 0
static int aft_tx_dma_chain_diff(private_area_t *chan);
#endif

#if defined(NETGRAPH)
extern void 	wan_ng_link_state(wanpipe_common_t *common, int state);
#endif

#if defined(__WINDOWS__)
 int set_netdev_state(sdla_t* card, netdevice_t* dev, int state);
#endif

/*=================================================================
 * Public Functions
 *================================================================*/


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
                WP_SCHEDULE(10,"aft_delay");
        }
#endif
}
#endif


int aft_global_hw_device_init(void)
{
	memset(aft_hwdev,0,sizeof(aft_hwdev));

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_TE1)
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
#endif

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_SERIAL)
	aft_hwdev[WANOPT_AFT_SERIAL].init				= 1;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_global_chip_config 	= a104_global_chip_config;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_global_chip_unconfig	= a104_global_chip_unconfig;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_chip_config		= a104_chip_config;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_chip_unconfig		= a104_chip_unconfig;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_chan_config		= a104_chan_dev_config;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_chan_unconfig 		= a104_chan_dev_unconfig;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_led_ctrl 			= a104_led_ctrl;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_test_sync 			= a104_test_sync;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_read_cpld			= aft_te1_read_cpld;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_write_cpld			= aft_te1_write_cpld;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_fifo_adjust		= a104_fifo_adjust;
	aft_hwdev[WANOPT_AFT_SERIAL].aft_check_ec_security	= a104_check_ec_security;
#endif

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_RM)
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
#endif

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)
	aft_hwdev[WANOPT_AFT_ISDN].init				= 1;
	aft_hwdev[WANOPT_AFT_ISDN].aft_global_chip_config 	= aft_bri_global_chip_config;
	aft_hwdev[WANOPT_AFT_ISDN].aft_global_chip_unconfig	= aft_bri_global_chip_unconfig;
	aft_hwdev[WANOPT_AFT_ISDN].aft_chip_config		= aft_bri_chip_config;
	aft_hwdev[WANOPT_AFT_ISDN].aft_chip_unconfig		= aft_bri_chip_unconfig;
	aft_hwdev[WANOPT_AFT_ISDN].aft_chan_config		= aft_bri_chan_dev_config;
	aft_hwdev[WANOPT_AFT_ISDN].aft_chan_unconfig 		= aft_bri_chan_dev_unconfig;
	aft_hwdev[WANOPT_AFT_ISDN].aft_led_ctrl 		= aft_bri_led_ctrl;
	aft_hwdev[WANOPT_AFT_ISDN].aft_test_sync 		= aft_bri_test_sync;
	aft_hwdev[WANOPT_AFT_ISDN].aft_read_cpld		= aft_bri_read_cpld;
	aft_hwdev[WANOPT_AFT_ISDN].aft_write_cpld		= aft_bri_write_cpld;
	aft_hwdev[WANOPT_AFT_ISDN].aft_fifo_adjust		= aft_bri_fifo_adjust;
	aft_hwdev[WANOPT_AFT_ISDN].aft_check_ec_security	= bri_check_ec_security;
#endif

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_56K)
	aft_hwdev[WANOPT_AFT_56K].init				= 1;
	aft_hwdev[WANOPT_AFT_56K].aft_global_chip_config 	= a104_global_chip_config;
	aft_hwdev[WANOPT_AFT_56K].aft_global_chip_unconfig	= a104_global_chip_unconfig;
	aft_hwdev[WANOPT_AFT_56K].aft_chip_config		= a104_chip_config;
	aft_hwdev[WANOPT_AFT_56K].aft_chip_unconfig		= a104_chip_unconfig;
	aft_hwdev[WANOPT_AFT_56K].aft_chan_config		= a104_chan_dev_config;
	aft_hwdev[WANOPT_AFT_56K].aft_chan_unconfig 		= a104_chan_dev_unconfig;
	aft_hwdev[WANOPT_AFT_56K].aft_led_ctrl 			= a104_led_ctrl;
	aft_hwdev[WANOPT_AFT_56K].aft_test_sync 		= a104_test_sync;
	aft_hwdev[WANOPT_AFT_56K].aft_read_cpld			= aft_56k_read_cpld;
	aft_hwdev[WANOPT_AFT_56K].aft_write_cpld		= aft_56k_write_cpld;
	aft_hwdev[WANOPT_AFT_56K].aft_fifo_adjust		= a104_fifo_adjust;
	aft_hwdev[WANOPT_AFT_56K].aft_check_ec_security		= a104_check_ec_security;
#endif

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

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_RM)
int wp_aft_analog_init (sdla_t *card, wandev_conf_t* conf)
{
	u32 adptr_type;
	/* Verify configuration ID */
	if (card->wandev.config_id != WANCONFIG_AFT_ANALOG) {
		DEBUG_EVENT( "%s: invalid configuration ID %u!\n",
				  card->devname, card->wandev.config_id);
		return -EINVAL;
	}

	ASSERT_AFT_HWDEV(card->wandev.card_type);

	if (card->adptr_type != A200_ADPTR_ANALOG &&
        card->adptr_type != A400_ADPTR_ANALOG &&
        card->adptr_type != AFT_ADPTR_FLEXBRI) {
		DEBUG_EVENT( "%s: Error: Attempting to configure for Analog on non A200/A400/B700 analog hw!\n",
				  card->devname);
		return -EINVAL;
	}

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
	memset(&card->fe, 0, sizeof(sdla_fe_t));
	memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));

	adptr_type = 0x00;
	card->hw_iface.getcfg(card->hw, SDLA_ADAPTERTYPE, &adptr_type);
	
	wp_remora_iface_init(&card->fe, &card->wandev.fe_iface);

	card->fe.name		= card->devname;
	card->fe.card		= card;
	card->fe.write_fe_reg	= card->hw_iface.fe_write;	/* aft_analog_write_fe;  */
	card->fe.read_fe_reg	= card->hw_iface.fe_read;	/* aft_analog_read_fe;   */
	card->fe.__read_fe_reg	= card->hw_iface.__fe_read;	/* __aft_analog_read_fe; */
	card->fe.reset_fe	= card->hw_iface.reset_fe;

	card->wandev.fe_enable_timer = enable_timer;
	card->wandev.ec_enable_timer = enable_ec_timer;
	card->wandev.te_link_state = callback_front_end_state;

	if (card->wandev.comm_port == WANOPT_PRI){
		conf->clocking = WANOPT_EXTERNAL;
	}

	if (adptr_type == AFT_ADPTR_FLEXBRI) {
		DEBUG_TEST("%s(): conf->comm_port: %d\n", __FUNCTION__, conf->comm_port);
		card->wandev.comm_port=conf->comm_port;
	} else {
		DEBUG_TEST("%s(): card->fe.fe_cfg.line_no: %d\n", __FUNCTION__, card->fe.fe_cfg.line_no);
		card->wandev.comm_port=card->fe.fe_cfg.line_no;
		if (card->wandev.comm_port != 0){
			DEBUG_EVENT("%s: Error: Invalid port selected %d (Port 1)\n",
					card->devname,card->wandev.comm_port);
			return -EINVAL;
		}
	}
	card->u.aft.num_of_time_slots=MAX_REMORA_MODULES;
	return 	wan_aft_init(card,conf);

}
#endif

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_A600)
int wp_aft_a600_init (sdla_t* card, wandev_conf_t* conf)
{
	AFT_FUNC_DEBUG();
	
	/* Verify configuration ID */
	
	if (card->wandev.config_id != WANCONFIG_AFT_ANALOG) {
		DEBUG_EVENT( "%s: invalid configuration ID %u!\n",
			     card->devname, card->wandev.config_id);
		return -EINVAL;
	}

	ASSERT_AFT_HWDEV(card->wandev.card_type);

	if (card->adptr_type != AFT_ADPTR_A600) {
		DEBUG_EVENT( "%s: Error: Attempting to configure for Analog on non B600 analog hw!\n",
				  card->devname);
		return -EINVAL;
	}

	card->hw_iface.getcfg(card->hw, SDLA_COREREV, &card->u.aft.firm_ver);
	card->hw_iface.getcfg(card->hw, SDLA_COREID, &card->u.aft.firm_id);

	if (card->u.aft.firm_ver < AFT_MIN_A600_FRMW_VER){
		DEBUG_EVENT( "%s: Invalid/Obsolete AFT A600 firmware version %X (not >= %X)!\n",
			     card->devname, card->u.aft.firm_ver,AFT_MIN_A600_FRMW_VER);
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

	/* Make special hardware initialization for A600 board */
	memset(&card->fe, 0, sizeof(sdla_fe_t));
	memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));
	
	wp_remora_iface_init(&card->fe, &card->wandev.fe_iface);
	
	card->fe.name		= card->devname;
	card->fe.card		= card;
	card->fe.write_fe_reg	= card->hw_iface.fe_write;	/* aft_a600_write_fe;  */
	card->fe.read_fe_reg	= card->hw_iface.fe_read;	/* aft_a600_read_fe;   */
	card->fe.__read_fe_reg	= card->hw_iface.__fe_read;	/* __aft_a600_read_fe; */
	card->fe.reset_fe	= card->hw_iface.reset_fe;

	card->wandev.fe_enable_timer = enable_timer;
	card->wandev.ec_enable_timer = enable_ec_timer;
	card->wandev.te_link_state = callback_front_end_state;
		
	card->wandev.comm_port=card->fe.fe_cfg.line_no;
	if (card->wandev.comm_port == 0){
		DEBUG_A600("Configuring A600 analog ports\n");
		card->u.aft.num_of_time_slots = NUM_A600_ANALOG_PORTS;
	} else if (card->wandev.comm_port == 1){
		DEBUG_A600("Configuring A600 daughter card - Not implemented yet\n");
	} else {
		DEBUG_A600("%s: Error: Invalid port selected %d\n",
			    card->devname,card->wandev.comm_port);	
	}

	return 	wan_aft_init(card,conf);
}
#endif

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)
int wp_aft_bri_init (sdla_t *card, wandev_conf_t* conf)
{
	/* Verify configuration ID */
	if (card->wandev.config_id != WANCONFIG_AFT_ISDN_BRI) {
		DEBUG_EVENT( "%s: invalid configuration ID %u!\n",
				  card->devname, card->wandev.config_id);
		return -EINVAL;
	}

	ASSERT_AFT_HWDEV(card->wandev.card_type);

	if (card->wandev.card_type != WANOPT_AFT_ISDN) {
		DEBUG_EVENT( "%s: Error: Attempting to configure for BRI on non A500/B700 hw!\n",
				  card->devname);
		return -EINVAL;
	}


	card->hw_iface.getcfg(card->hw, SDLA_COREREV, &card->u.aft.firm_ver);
	card->hw_iface.getcfg(card->hw, SDLA_COREID, &card->u.aft.firm_id);

	/* FIXME:hardcoded!! */
	card->u.aft.firm_id = AFT_DS_FE_CORE_ID;

	if (conf == NULL){
		DEBUG_EVENT("%s: Bad configuration structre!\n",
				card->devname);
		return -EINVAL;
	}

	/* Make special hardware initialization for ISDN BRI board */
	memset(&card->fe, 0, sizeof(sdla_fe_t));
	memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));
	wp_bri_iface_init(&card->wandev.fe_iface);
	card->fe.name		= card->devname;
	card->fe.card		= card;
	card->fe.write_fe_reg	= aft_bri_write_fe;
	card->fe.read_fe_reg	= aft_bri_read_fe;
	card->fe.__read_fe_reg	= __aft_bri_read_fe;

	card->wandev.fe_enable_timer = enable_timer;
	card->wandev.ec_enable_timer = enable_ec_timer;
	card->wandev.te_link_state = callback_front_end_state;

	if (card->wandev.comm_port == WANOPT_PRI){
		conf->clocking = WANOPT_EXTERNAL;
	}

#if 1
	card->wandev.comm_port=0;
	if (card->fe.fe_cfg.line_no >= MAX_BRI_MODULES) {
		card->wandev.comm_port=1;
	}
#else
	card->wandev.comm_port=card->fe.fe_cfg.line_no;
#endif

	DEBUG_EVENT("%s: BRI CARD Line=%d  Port=%d\n",
			card->devname, card->wandev.comm_port, card->fe.fe_cfg.line_no);

	/* Set 'num_of_time_slots' to 31. This is needed for the d-chan,
	   which is always at the otherwise unused timeslot 31.
	*/
	card->u.aft.num_of_time_slots = MAX_TIMESLOTS;

	return 	wan_aft_init(card,conf);
}
#endif



#if defined(CONFIG_PRODUCT_WANPIPE_AFT_SERIAL)

int wp_aft_serial_init (sdla_t *card, wandev_conf_t* conf)
{
	/* Verify configuration ID */
	if (card->wandev.config_id != WANCONFIG_AFT_SERIAL) {
		DEBUG_EVENT( "%s: invalid configuration ID %u!\n",
				  card->devname, card->wandev.config_id);
		return -EINVAL;
	}

	ASSERT_AFT_HWDEV(card->wandev.card_type);

	if (card->wandev.card_type != WANOPT_AFT_SERIAL) {
		DEBUG_EVENT( "%s: Error: Attempting to configure for SERIAL on non A14X hw!\n",
				  card->devname);
		return -EINVAL;
	}

	card->hw_iface.getcfg(card->hw, SDLA_COREREV, &card->u.aft.firm_ver);
	card->hw_iface.getcfg(card->hw, SDLA_COREID, &card->u.aft.firm_id);

	if (card->u.aft.firm_ver < AFT_SERIAL_MIN_FRMW_VER){
		DEBUG_EVENT( "%s: Invalid/Obselete AFT firmware version %X (not >= %X)!\n",
				  card->devname, card->u.aft.firm_ver,AFT_SERIAL_MIN_FRMW_VER);
		DEBUG_EVENT( "%s  You seem to be running BETA hardware that is not supported any more.\n",
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

	/* Make special hardware initialization for Serial board */
	memset(&card->fe, 0, sizeof(sdla_fe_t));
	memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));

	FE_MEDIA(&(card->fe.fe_cfg)) = WAN_MEDIA_SERIAL;

	wp_serial_iface_init(&card->wandev.fe_iface);
	card->fe.name		= card->devname;
	card->fe.card		= card;
	card->fe.write_fe_reg	= card->hw_iface.fe_write;	/*aft_serial_write_fe;*/
	card->fe.read_fe_reg	= card->hw_iface.fe_read;	/*aft_serial_read_fe;*/
	card->fe.__read_fe_reg	= card->hw_iface.__fe_read;	/*__aft_serial_read_fe;*/

	card->wandev.fe_enable_timer = enable_timer;
	card->wandev.ec_enable_timer = enable_ec_timer;
	card->wandev.te_link_state = callback_front_end_state;

	card->wandev.comm_port=card->fe.fe_cfg.line_no;

	DEBUG_EVENT("%s: Serial A140 CARD Line=%d  Port=%d Firm=0x%02X ID=0x%X\n",
			card->devname, card->wandev.comm_port, card->fe.fe_cfg.line_no,
			card->u.aft.firm_ver,card->u.aft.firm_id);

	card->u.aft.num_of_time_slots = 1;

	return wan_aft_init(card,conf);
}
#endif


#if defined(CONFIG_PRODUCT_WANPIPE_AFT_TE1)
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

	ASSERT_AFT_HWDEV(card->wandev.card_type);
  
  switch (card->adptr_type){
    case A101_ADPTR_1TE1:		/* 1 Channel T1/E1  */
    case A101_ADPTR_2TE1:	/* 2 Channels T1/E1 */
    case A104_ADPTR_4TE1:	/* Quad line T1/E1 */
    case A104_ADPTR_4TE1_PCIX:		/* Quad line T1/E1 PCI Express */
    case A108_ADPTR_8TE1:
      break;
  default:
	  	DEBUG_EVENT( "%s: Error: Attempting to configure for T1/E1 on non AFT A101/2/4/8 hw (%d)!\n",
				  card->devname,card->adptr_type);
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

	memset(&card->fe, 0, sizeof(sdla_fe_t));
	memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));

	/* TE1 Make special hardware initialization for T1/E1 board */
	if (IS_TE1_MEDIA(&conf->fe_cfg)){
		int max_ports = 4;

		if (conf->fe_cfg.cfg.te_cfg.active_ch == 0){
			conf->fe_cfg.cfg.te_cfg.active_ch = -1;
		}

		memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));
		if (card->u.aft.firm_id == AFT_DS_FE_CORE_ID) {
			max_ports = 8;
			sdla_ds_te1_iface_init(&card->fe, &card->wandev.fe_iface);
		}else{
			sdla_te_iface_init(&card->fe, &card->wandev.fe_iface);
		}
		card->fe.name		= card->devname;
		card->fe.card		= card;
		card->fe.write_fe_reg	= card->hw_iface.fe_write;	/*a104_write_fe;*/
		card->fe.read_fe_reg	= card->hw_iface.fe_read;	/*a104_read_fe;*/
		card->fe.__read_fe_reg	= card->hw_iface.__fe_read;	/*__a104_read_fe;*/

		card->wandev.fe_enable_timer = enable_timer;
		card->wandev.ec_enable_timer = enable_ec_timer;
		card->wandev.te_link_state = callback_front_end_state;
		conf->electrical_interface =
			IS_T1_CARD(card) ? WANOPT_V35 : WANOPT_RS232;

		if (card->wandev.comm_port == WANOPT_PRI){
			conf->clocking = WANOPT_EXTERNAL;
		}

		card->wandev.comm_port=card->fe.fe_cfg.line_no;

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
#endif

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_56K)
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

	ASSERT_AFT_HWDEV(card->wandev.card_type);

	if (card->wandev.card_type != WANOPT_AFT_56K) {
		DEBUG_EVENT( "%s: Error: Attempting to configure for 56K on non A056K hw!\n",
				  card->devname);
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

	memset(&card->fe, 0, sizeof(sdla_fe_t));
	memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));

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
		sdla_56k_iface_init(&card->fe, &card->wandev.fe_iface);

		card->fe.name		= card->devname;
		card->fe.card		= card;

		card->fe.write_fe_reg	= card->hw_iface.fe_write;	/* a56k_write_fe;*/
		card->fe.read_fe_reg	= card->hw_iface.fe_read;	/* a56k_read_fe; */
		card->fe.__read_fe_reg	= card->hw_iface.__fe_read;	/* __a56k_read_fe; */

		card->wandev.fe_enable_timer = enable_timer;
		card->wandev.ec_enable_timer = enable_ec_timer;
		card->wandev.te_link_state = callback_front_end_state;

		card->wandev.comm_port=0;

		card->u.aft.num_of_time_slots=1;

	}else{
		DEBUG_EVENT("%s: Invalid Front-End media type!!\n",
				card->devname);
		return -EINVAL;
	}

	return 	wan_aft_init(card,conf);

}
#endif

static int wan_aft_init (sdla_t *card, wandev_conf_t* conf)
{
	int err;
	int used_cnt;
	int used_type_cnt;

	/* Obtain hardware configuration parameters */
	card->wandev.clocking 			= conf->clocking;
	card->wandev.ignore_front_end_status 	= conf->ignore_front_end_status;
	card->wandev.ttl 			= conf->ttl;
	card->wandev.electrical_interface 	= conf->electrical_interface;
	card->wandev.udp_port   		= conf->udp_port;
	card->wandev.new_if_cnt 		= 0;
	wan_atomic_set(&card->wandev.if_cnt,0);
	card->u.aft.chip_security_cnt=0;

	memcpy(&card->u.aft.cfg,&conf->u.aft,sizeof(wan_xilinx_conf_t));
	memcpy(&card->rtp_conf,&conf->rtp_conf,sizeof(conf->rtp_conf));
	memset(card->u.aft.dev_to_ch_map,0,sizeof(card->u.aft.dev_to_ch_map));
	memcpy(&card->tdmv_conf,&conf->tdmv_conf,sizeof(wan_tdmv_conf_t));
	memcpy(&card->hwec_conf,&conf->hwec_conf,sizeof(wan_hwec_conf_t));

	card->u.aft.cfg.dma_per_ch = MAX_RX_BUF;
	card->u.aft.tdmv_api_rx = NULL;
	card->u.aft.tdmv_api_tx = NULL;
	card->u.aft.tdmv_dchan=0;
	wan_skb_queue_init(&card->u.aft.tdmv_api_tx_list);
	wan_skb_queue_init(&card->u.aft.rtp_tap_list);


	/* Set the span_no by default to card number, unless set by user */
#if 0
	if (card->tdmv_conf.span_no == 0) {
		card->tdmv_conf.span_no = card->card_no;
		if (!card->card_no) {
			DEBUG_EVENT("%s: Error: Internal Driver Error: Card card_no filed is ZERO!\n",
					card->devname);
			return -1;
		}
	}
#endif

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


#ifdef CONFIG_PRODUCT_WANPIPE_ANNEXG
	card->wandev.bind_annexg	= &bind_annexg;
	card->wandev.un_bind_annexg	= &un_bind_annexg;
	card->wandev.get_map		= &get_map;
	card->wandev.get_active_inactive= &get_active_inactive;
#endif


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
		card->u.aft.cfg.mru = (u16)card->wandev.mtu;
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

	port_set_state(card,WAN_DISCONNECTED);

	WAN_TASKQ_INIT((&card->u.aft.port_task),0,aft_port_task,card);

	card->u.aft.chip_cfg_status=0;
	card->hw_iface.getcfg(card->hw, SDLA_HWCPU_USEDCNT, &used_cnt);

	/* ======================================================================
         * After this point we must call disable_comm() if we fail to configure
  	 * =====================================================================*/

	wan_clear_bit(CARD_DOWN,&card->wandev.critical);

	__sdla_push_ptr_isr_array(card->hw,card,WAN_FE_LINENO(&card->fe));

        card->isr = &wp_aft_global_isr;

#if 1
	card->hw_iface.getcfg(card->hw, SDLA_HWTYPE_USEDCNT, &used_type_cnt);
	if (used_type_cnt == 1) {
#else
	if (used_cnt==1){
#endif
		DEBUG_EVENT("%s: Global Chip Configuration: used=%d used_type=%d\n",
				card->devname,used_cnt, used_type_cnt);

		err=aft_global_chip_configuration(card, conf);

		if (err){
			disable_comm(card);
			wan_set_bit(CARD_DOWN,&card->wandev.critical);
			return err;
		}

		aft_data_mux_cfg(card);
	}else{

		aft_data_mux_get_cfg(card);

		err=aft_front_end_mismatch_check(card);
		if (err){
			disable_comm(card);
			wan_set_bit(CARD_DOWN,&card->wandev.critical);
			return err;
		}

		DEBUG_EVENT("%s: Global Chip Configuration skiped: used=%d used_type=%d\n",
				card->devname,used_cnt, used_type_cnt);
	}
	card->wandev.ec_intmask=SYSTEM_TICKS;

	aft_read_security(card);


	err=aft_chip_configure(card,conf);
	if (err){
		AFT_FUNC_DEBUG();

		aft_chip_unconfigure(card);
		disable_comm(card);
		wan_set_bit(CARD_DOWN,&card->wandev.critical);
		return err;
	}
	wan_set_bit(AFT_CHIP_CONFIGURED,&card->u.aft.chip_cfg_status);

	if (wan_test_bit(AFT_FRONT_END_UP,&card->u.aft.chip_cfg_status)){
		wan_smp_flag_t smp_flags;
		DEBUG_TEST("%s: Front end up, retrying enable front end!\n",
				card->devname);
		card->hw_iface.hw_lock(card->hw,&smp_flags);
		handle_front_end_state(card,1);
		card->hw_iface.hw_unlock(card->hw,&smp_flags);

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
	DEBUG_EVENT("%s:    Rx CRC Bytes   = %d\n",
			card->devname,
			card->u.aft.cfg.rx_crc_bytes);

	DEBUG_EVENT("%s:    Memory: Card=%d Wandev=%d Card Union=%d\n",
			card->devname,sizeof(sdla_t),sizeof(wan_device_t),sizeof(card->u));

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

	if(IS_BRI_CARD(card) || IS_A700_CARD(card)){
		wan_set_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status);
		wan_set_bit(AFT_TDM_RING_BUF,&card->u.aft.chip_cfg_status);
	} else if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
		wan_set_bit(AFT_TDM_SW_RING_BUF,&card->u.aft.chip_cfg_status);
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
		if (conf->tdmv_conf.hw_dtmf){
			card->u.aft.tdmv_hw_tone = WANOPT_YES;
		}
	}else{
		card->u.aft.tdmv_hw_tone = WANOPT_NO;
	}
	DEBUG_EVENT("%s:    TDM HW TONE    = %s\n",
			card->devname,
			(card->u.aft.tdmv_hw_tone == WANOPT_YES) ?
				"Enabled" : "Disabled");

	err=aft_tdmv_init(card,conf);
	if (err){
		disable_comm(card);
		wan_set_bit(CARD_DOWN,&card->wandev.critical);
		return err;
	}

	card->disable_comm = &disable_comm;

#if defined(AFT_RTP_SUPPORT)
        err=aft_rtp_config(card);
	DEBUG_EVENT("%s:    RTP TAP        = %s\n",
			card->devname,
			err == 0 ? "Enabled" : "Disabled");
#endif

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
	sdla_t* card = wandev->priv;
 	netdevice_t* dev;
        volatile private_area_t* chan;

	/* sanity checks */
	if((wandev == NULL) || (wandev->priv == NULL))
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




static int aft_chan_if_init(sdla_t *card, netdevice_t *dev, private_area_t *chan)
{
	chan->first_time_slot=-1;
	chan->last_time_slot=-1;
	chan->logic_ch_num=-1;
#if defined(AFT_SINGLE_DMA_CHAIN)
	chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;
	chan->max_tx_bufs=MAX_AFT_DMA_CHAINS;
#else
	chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN;
	chan->max_tx_bufs=MAX_TX_BUF;
#endif
	chan->tslot_sync=0;

	strncpy(chan->if_name, wan_netif_name(dev), WAN_IFNAME_SZ);

	chan->card = card;
	chan->common.card = card;

	WAN_IFQ_INIT(&chan->wp_tx_pending_list,0);
	WAN_IFQ_INIT(&chan->wp_tx_complete_list,0);
	WAN_IFQ_INIT(&chan->wp_tx_hdlc_rpt_list,0);

	WAN_IFQ_INIT(&chan->wp_rx_free_list,0);
	WAN_IFQ_INIT(&chan->wp_rx_complete_list,0);

	WAN_IFQ_INIT(&chan->wp_rx_stack_complete_list, 0);

	WAN_IFQ_INIT(&chan->wp_rx_bri_dchan_complete_list, 0);

	WAN_IFQ_INIT(&chan->wp_dealloc_list,0);

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
	chan->xmtp2_api_index = -1;

	if (chan->common.usedby == API && chan->cfg.ss7_enable){

		wan_smp_flag_t smp_flags;
		u32 lcfg_reg;

		DEBUG_EVENT("%s:    SS7 Mode      :%s\n",
					card->devname,
					chan->cfg.ss7_mode?"4096":"128");

		DEBUG_EVENT("%s:    SS7 LSSU Size :%d\n",
					card->devname,
					chan->cfg.ss7_lssu_size);

		wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
		card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG),&lcfg_reg);
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
		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;

	} else {
		 chan->cfg.ss7_enable = 0;

#if defined(AFT_XMTP2_API_SUPPORT)
		if (chan->common.usedby == XMTP2_API) {
			chan->xmtp2_api_index = xmtp2km_register(chan, chan->if_name, wp_xmtp2_callback);
			if (chan->xmtp2_api_index < 0) {
				chan->xmtp2_api_index = -1;
				return -EINVAL;
			}
		}
#endif
	}

	return 0;
}


static int aft_ss7_if_unreg(sdla_t *card, private_area_t *chan)
{

#if defined(AFT_XMTP2_API_SUPPORT)
		if (chan->common.usedby == XMTP2_API && chan->xmtp2_api_index >= 0 ) {
			xmtp2km_unregister(chan->xmtp2_api_index);
			chan->xmtp2_api_index = -1;
		}
#endif

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
	
  	chan->max_idle_size=(u16)chan->mtu;

	if (!chan->channelized_cfg && chan->num_of_time_slots > 1){  
   	 	if (conf->u.aft.mtu_idle == 0) {
			if (chan->tdm_api_chunk > 40) {
				chan->max_idle_size=40*chan->num_of_time_slots;
			}
		} else {
			switch (conf->u.aft.mtu_idle) {
				case 40:
				case 80:
				case 160:
				case 340:
					chan->max_idle_size=conf->u.aft.mtu_idle*chan->num_of_time_slots;
					break;
				default:
                    DEBUG_EVENT("%s:%s: Warning: Invalid mtu idle lenght %i defaulting to 40bytes (5ms)\n",
								card->devname,chan->if_name,conf->u.aft.mtu_idle);
					chan->max_idle_size=40*chan->num_of_time_slots;
					break;
			}
		}
	}
	
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
	
	
	DEBUG_TEST("%s:%s: Config for Transparent mode: Idle Flag=0x%02X, Max Idle Len=%d, dma_mru=%d\n",
		card->devname, chan->if_name, chan->idle_flag, chan->max_idle_size, chan->dma_mru);
	
	
	if(conf->u.aft.idle_flag){
		chan->idle_flag=conf->u.aft.idle_flag;
		DEBUG_EVENT("%s:    Idle flag     :0x%02x\n", card->devname, chan->idle_flag);
	} else {
		chan->idle_flag=0x7E;
	}
	
	
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
		
		/* reset the tx idle buffer to the actual mtu size */
		wan_skb_init(chan->tx_idle_skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(chan->tx_idle_skb,0);
		buf=wan_skb_put(chan->tx_idle_skb,chan->max_idle_size);
		
		memset(buf,chan->idle_flag,chan->max_idle_size);
		
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
		
		wan_skb_init(chan->tx_idle_skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(chan->tx_idle_skb,0);
		wan_skb_put(chan->tx_idle_skb,chan->max_idle_size);
	}
	
	return 0;
}




static int new_if_private (wan_device_t* wandev, netdevice_t* dev, wanif_conf_t* conf, u8 channelized, int dchan, int if_seq)
{
	sdla_t* card = wandev->priv;
	private_area_t* chan;
	int dma_per_ch=card->u.aft.cfg.dma_per_ch;
	int	err = 0, dma_alignment = 4, i =0;
	int if_cnt=0;
	int silent=if_seq;
	
	/* DCHAN interface should not be log suppressed */
	if (dchan>=0) {
		silent=0;
	}

	if_cnt = wan_atomic_read(&card->wandev.if_cnt)+1;

	if (silent) {
		DEBUG_EVENT( "%s: Configuring Interface: %s (log supress)\n",
				card->devname, wan_netif_name(dev));
	} else {
		DEBUG_EVENT( "%s: Configuring Interface: %s\n",
				card->devname, wan_netif_name(dev));
	}

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

	chan->cfg_active_ch = conf->cfg_active_ch;
	chan->true_if_encoding=conf->true_if_encoding;
	chan->if_cnt = if_cnt;

	aft_chan_if_init(card,dev,chan);

	if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;
		conf->hdlc_streaming=0;
	}

	if (card->wandev.config_id == WANCONFIG_AFT_ISDN_BRI) {
		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;
	}

	/* If 'dchan_time_slot' is less than zero, it is NOT a dchan.
	   If 'dchan_time_slot' is greater or equal to zero it is a dchan.
	   NOTE: 'dchan' is NOT the same as 'hdlc_eng'. The 'hdlc_eng' is
		a flag for AFT hardware to use it's HDLC core.
	*/
	chan->dchan_time_slot = dchan;


	if(IS_56K_CARD(card)){
		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;
		conf->hdlc_streaming=1;
	}

	if (chan->cfg.hdlc_repeat) {
		if (!conf->hdlc_streaming) {
			DEBUG_EVENT("%s: HDLC Repeat configured for non hdlc channel!\n",
				chan->if_name);
			err= -EINVAL;
			goto new_if_error;
		}
		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;
		conf->hdlc_streaming=1;
	}

	chan->channelized_cfg=channelized;
	chan->wp_api_iface_mode=0;

	/* ======================================
	 * Configure chan MTU and MRU Values
	 * And setup E1 timeslots
	 * =====================================*/
	chan->mtu = card->wandev.mtu;

	DEBUG_TEST("%s(): %d: conf->u.aft.mtu: %d, conf->u.aft.mru: %d\n", __FUNCTION__, __LINE__, conf->u.aft.mtu, conf->u.aft.mru);

	DEBUG_TEST("%s(): %d: chan->mtu: %d chan->mru: %d\n", __FUNCTION__, __LINE__, chan->mtu, chan->mru);

	if (conf->u.aft.mtu){
		chan->mtu=conf->u.aft.mtu;
			
		DEBUG_TEST("%s(): %d: chan->mtu: %d\n", __FUNCTION__, __LINE__, chan->mtu);

		if (chan->mtu > MAX_WP_PRI_MTU ||
	    	    chan->mtu < MIN_WP_PRI_MTU){
			DEBUG_EVENT("%s: Error Invalid %s MTU %d (Min=%d, Max=%d)\n",
				card->devname,chan->if_name,chan->mtu,
				MIN_WP_PRI_MTU,MAX_WP_PRI_MTU);

			err= -EINVAL;
			goto new_if_error;
		}
	}

	/* Initialize MRU based on Current MTU value */
	chan->mru = chan->mtu;

	/* Initialize MRU based on Global MRU Value */
	if (card->u.aft.cfg.mru) {
		chan->mru = card->u.aft.cfg.mru;
	}

	/* Initialize MRU based on Interface MRU Value */
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


	if (channelized) {
		if (dchan >= 0) {
			chan->mtu = card->wandev.mtu;
			chan->mru = chan->mtu;
			if (chan->mtu > MAX_WP_PRI_MTU ||
					chan->mtu < MIN_WP_PRI_MTU){
					DEBUG_EVENT("%s: Error Invalid %s MTU %d (Min=%d, Max=%d)\n",
						card->devname,chan->if_name,chan->mtu,
						MIN_WP_PRI_MTU,MAX_WP_PRI_MTU);
		
					err= -EINVAL;
					goto new_if_error;
			}

			chan->tdm_api_chunk=chan->mtu;
			chan->tdm_api_period=8;
		} else {

			/* To keep backward compatibility if iface MTU is not set
			   set it to 80 */
			if (conf->u.aft.mtu == 0) {
				chan->mtu=80;
			}
	
			if (chan->mtu % 8 != 0) {
				DEBUG_EVENT("%s:%s: Error: MTU %d is not 8 byte aligned!\n",
					card->devname, chan->if_name, chan->mtu);
				err= -EINVAL;
				goto new_if_error;
			}

			chan->mru = chan->mtu;
			chan->tdm_api_chunk=chan->mtu;
			chan->tdm_api_period=chan->tdm_api_chunk/8;

			switch (chan->tdm_api_period) {
			case 1:
			case 2:
			case 5:
			case 10:
			case 20:
			case 30:
			case 40:
				break;
			default:
				DEBUG_EVENT("%s:%s: Error: Invalid MTU %d - Supported: 8/16/40/80/160/240/320!\n",
					card->devname, chan->if_name, chan->mtu);
					err= -EINVAL;
					goto new_if_error;
				break;
			}
		}
	} else {
		if (dchan >= 0 || conf->hdlc_streaming) {

			chan->mtu = card->wandev.mtu;
			chan->mru = chan->mtu;

			/* In this case DCHAN is being configured with TDMV_DCHAN
				we must use MTU based on global configuraiton */
			if (chan->mtu > MAX_WP_PRI_MTU ||
				chan->mtu < MIN_WP_PRI_MTU){
				DEBUG_EVENT("%s: Error Invalid %s MTU %d (Min=%d, Max=%d)\n",
					card->devname,chan->if_name,chan->mtu,
					MIN_WP_PRI_MTU,MAX_WP_PRI_MTU);

				err= -EINVAL;
				goto new_if_error;
			}

			chan->tdm_api_chunk=chan->mtu;
			chan->tdm_api_period=8;

		} else {

			if (chan->mtu % 4 != 0) {
				DEBUG_EVENT("%s:%s: Error: MTU %d is not 4 byte aligned!\n",
					card->devname, chan->if_name, chan->mtu);
				err= -EINVAL;
				goto new_if_error;
			}

			if (chan->mtu <= 320) {
				int scnt=0,mtu=0;

				chan->tdm_api_chunk=chan->mtu;
				chan->tdm_api_period=chan->tdm_api_chunk/8;

				for (scnt=0;scnt<32;scnt++) {
					if (wan_test_bit(scnt,&conf->active_ch)) {
						mtu+=chan->mtu;
					}
				}
	
				chan->mtu=mtu;
				chan->mru=mtu;
			} else {
				int scnt=0,timeslots=0;   
				for (scnt=0;scnt<32;scnt++) {
					if (wan_test_bit(scnt,&conf->active_ch)) {
						timeslots++;
					}
				}

				if (chan->mtu % timeslots != 0 || chan->mtu&0x03) {
					DEBUG_EVENT("%s:%s Error: MTU %d is not timeslot (%d) aligned!\n",
					card->devname, chan->if_name, chan->mtu,timeslots);
					err= -EINVAL;
					goto new_if_error;
				}

				chan->tdm_api_chunk=chan->mtu/timeslots;
				chan->tdm_api_period=chan->tdm_api_chunk/8;
			}
		}
	}


	if (conf->u.aft.mru == 0){
		chan->mru=chan->mtu;
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
	 *    XMTP2_API		= MTP2 API
	 *===================================================*/

	/* If we are running zaptel with another mode, we must
  	 * disable zaptel dchan optimization */	
	if (strcmp(conf->usedby, "TDM_VOICE") != 0) {
		if (card->u.aft.tdmv_zaptel_cfg) {
			DEBUG_EVENT("%s: Disabling ZAPTEL DCHAN OPTIMIZATION !\n",chan->if_name);
			wan_clear_bit(WP_ZAPTEL_DCHAN_OPTIMIZATION,&card->u.aft.tdmv_zaptel_cfg);
		}
	}

	if(strcmp(conf->usedby, "WANPIPE") == 0) {

		DEBUG_EVENT( "%s: Running in WANPIPE mode\n",
			wandev->name);
		chan->common.usedby = WANPIPE;
		chan->usedby_cfg = chan->common.usedby;

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

	} else if( strcmp(conf->usedby, "DATA_API") == 0 || strcmp(conf->usedby, "API") == 0  || strcmp(conf->usedby, "API_LEGACY") == 0)  {

		chan->common.usedby = API;
		chan->usedby_cfg = chan->common.usedby;
		DEBUG_EVENT( "%s:%s: Running in %s mode\n",
			wandev->name,chan->if_name, conf->usedby);

		/* Nenad: FIXME
		 * Convert API mode to CDEV based API, for now
		 * only WINDOWS is running in this mode */
#if defined(__WINDOWS__)
		chan->wp_api_op_mode=WP_TDM_OPMODE_SPAN;
		chan->wp_api_iface_mode=0;

		if (strcmp(conf->usedby, "API_LEGACY") == 0) {
			chan->usedby_cfg = API_LEGACY;
			chan->wp_api_iface_mode=WP_TDM_API_MODE_LEGACY_WIN_API;
		}
#else
		if (strcmp(conf->usedby, "DATA_API") == 0) {
			chan->wp_api_op_mode=WP_TDM_OPMODE_SPAN;
			chan->wp_api_iface_mode=0;
		
			if (card->tdmv_conf.span_no == 0) {
				card->tdmv_conf.span_no = card->card_no;
				if (!card->card_no) {
					DEBUG_EVENT("%s: Error: Internal Driver Error: Card card_no filed is ZERO!\n",
							card->devname);
					return -EINVAL;
				}
			}

		} else {
			wan_reg_api(chan, dev, card->devname);
			/* These are legacy non windows Event system
			* should be removed on Linux when all APIs move
			* to cdev */
			err=aft_core_api_event_init(card);
			if (err) {
				return err;
			}
		}
#endif/* __WINDOWS__ */

#endif/* AFT_API_SUPPORT */

#if defined (__WINDOWS__)

	} else if( strcmp(conf->usedby, "TDM_VOICE_DCHAN") == 0) {
		chan->common.usedby = API;
		chan->usedby_cfg = TDM_VOICE_DCHAN;
		DEBUG_EVENT( "%s:%s: Running in %s mode\n",
			wandev->name,chan->if_name, conf->usedby);
		wan_reg_api(chan, dev, card->devname);

		chan->wp_api_op_mode=WP_TDM_OPMODE_SPAN;
		chan->wp_api_iface_mode=WP_TDM_API_MODE_LEGACY_WIN_API;

		chan->cfg.data_mux=0;
		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN;
		conf->hdlc_streaming=1;
#endif/* __WINDOWS__ */

#ifdef AFT_TDM_API_SUPPORT
	} else if (strcmp(conf->usedby, "TDM_SPAN_VOICE_API") == 0) {

		chan->common.usedby = API;
		chan->usedby_cfg = TDM_SPAN_VOICE_API;

		chan->wp_api_op_mode=WP_TDM_OPMODE_SPAN;

		if (dchan >= 0){

			sprintf(chan->if_name,"%s-dchan",chan->if_name);

			DEBUG_EVENT( "%s:%s: Running in TDM_SPAN_VOICE_DCHAN mode\n",
					wandev->name,chan->if_name);

		
			chan->common.usedby = TDM_VOICE_DCHAN;
			chan->usedby_cfg = TDM_VOICE_DCHAN;
			chan->cfg.data_mux=0;
			chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN;
			conf->hdlc_streaming=1;

		} else {

			DEBUG_EVENT( "%s:%s: Running in %s mode\n",
					wandev->name,chan->if_name, conf->usedby);


			chan->common.usedby = API;
			chan->cfg.data_mux=1;
			conf->hdlc_streaming=0;
			chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_IRQ_ALL;
			chan->max_tx_bufs=MAX_AFT_DMA_CHAINS;			

		}
#endif

#if defined(AFT_XMTP2_API_SUPPORT)
	} else if (strcmp(conf->usedby, "XMTP2_API") == 0) {
		chan->common.usedby = XMTP2_API;
		chan->usedby_cfg = chan->common.usedby;
		conf->hdlc_streaming=0;
		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_IRQ_ALL;
		conf->mtu=80;
#endif

#if defined(__LINUX__)
	}else if (strcmp(conf->usedby, "BRIDGE") == 0) {
		chan->common.usedby = BRIDGE;
		chan->usedby_cfg = chan->common.usedby;
		DEBUG_EVENT( "%s:%s: Running in WANPIPE (BRIDGE) mode.\n",
				card->devname,chan->if_name);

	} else if (strcmp(conf->usedby, "BRIDGE_NODE") == 0) {
		chan->common.usedby = BRIDGE_NODE;
		chan->usedby_cfg = chan->common.usedby;
		DEBUG_EVENT( "%s:%s: Running in WANPIPE (BRIDGE_NODE) mode.\n",
				card->devname,chan->if_name);
#endif

	} else if (strcmp(conf->usedby, "TDM_VOICE") == 0) {

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
		chan->common.usedby = TDM_VOICE;
		chan->usedby_cfg = chan->common.usedby;

		chan->tdmv_zaptel_cfg=1;
		card->u.aft.tdmv_zaptel_cfg=1;
		conf->hdlc_streaming=0;
		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;
		chan->max_tx_bufs=MAX_AFT_DMA_CHAINS;

		/* Eanble Zaptel Optimization */
		wan_set_bit(WP_ZAPTEL_ENABLED,&card->u.aft.tdmv_zaptel_cfg);		
		wan_set_bit(WP_ZAPTEL_DCHAN_OPTIMIZATION,&card->u.aft.tdmv_zaptel_cfg);


		if (dchan >= 0){
# ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN
		       	chan->common.usedby = TDM_VOICE_DCHAN;
				chan->usedby_cfg = chan->common.usedby;
		       	conf->hdlc_streaming=1;
		       	chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN;
		       	chan->mru=chan->mtu=1500;
# else
			DEBUG_EVENT("%s: Error: TDMV_DCHAN Option not compiled into the driver!\n",
					card->devname);
			err=-EINVAL;
			goto new_if_error;
# endif
		}

		if (!silent) {
			DEBUG_EVENT( "%s:%s: Running in TDM %sVoice Zaptel Mode.\n",
				card->devname,chan->if_name,
				chan->common.usedby == TDM_VOICE_DCHAN?"DCHAN ":"");
		}
#else
		DEBUG_EVENT("\n");
		DEBUG_EVENT("%s:%s: Error: TDM VOICE prot not compiled\n",
				card->devname,chan->if_name);
		DEBUG_EVENT("%s:%s:        during installation process!\n",
				card->devname,chan->if_name);
		err=-EINVAL;
		goto new_if_error;
#endif

	
	} else if (strcmp(conf->usedby, "WIN_TDM_LEGACY_SPAN_API") == 0) {

		/* This is WINDOWS mode only!
		 * Used to support legacy Windows API 
		 * TDM_VOICE_API for non analog is mapped to WIN_TDM_LEGACY_SPAN_API in new_if()
		 * TDM_VOICE_API for analog is handled in TDM_VOICE_API section below
		 *
		 * This section will configure core for SPAN mode and API interface mode to Legacy.
		 */

		chan->wp_api_op_mode=WP_TDM_OPMODE_SPAN;
		chan->wp_api_iface_mode=WP_TDM_API_MODE_LEGACY_WIN_API;

		if (dchan >= 0){

			if (!silent) {
				DEBUG_EVENT( "%s:%s: Running in TDM Voice DCHAN API Mode\n",
					wandev->name,chan->if_name);
			}

			chan->common.usedby = TDM_VOICE_DCHAN;
			chan->usedby_cfg = chan->common.usedby;
			chan->cfg.data_mux=0;
   			chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN;
			conf->hdlc_streaming=1;
		
		} else {

			if (!silent) {
				DEBUG_EVENT( "%s:%s: Running in TDM Voice API Mode.\n",
						card->devname,chan->if_name);
			}

			chan->common.usedby = API;
			chan->usedby_cfg = TDM_VOICE_API;
			chan->cfg.data_mux=1;
			conf->hdlc_streaming=0;
			chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_IRQ_ALL;
			chan->tdmv_zaptel_cfg=0;
		}

	} else if (strcmp(conf->usedby, "TDM_VOICE_API") == 0 || strcmp(conf->usedby, "TDM_CHAN_VOICE_API") == 0) {

		chan->common.usedby = TDM_VOICE_API;
		chan->usedby_cfg = TDM_CHAN_VOICE_API;
		chan->cfg.data_mux=1;
		conf->hdlc_streaming=0;
		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;
		chan->tdmv_zaptel_cfg=0;


#if defined(__WINDOWS__)
		/* TDM_VOICE_API for non analog is mapped to WIN_TDM_LEGACY_SPAN_API in new_if()
		 * TDM_VOICE_API for analog is handled here */
		if (strcmp(conf->usedby, "TDM_VOICE_API") == 0) {
			chan->wp_api_iface_mode=WP_TDM_API_MODE_LEGACY_WIN_API;
		}
#endif

		chan->wp_api_op_mode=WP_TDM_OPMODE_CHAN;

		if (dchan >= 0){
		       	chan->common.usedby = TDM_VOICE_DCHAN;
				chan->usedby_cfg = chan->common.usedby;
		        chan->cfg.data_mux=0;
		       	chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN;
		       	conf->hdlc_streaming=1;
		}

		if (!silent) {
			if (chan->common.usedby == TDM_VOICE_DCHAN) {
				DEBUG_EVENT( "%s:%s: Running in TDM Voice DCHAN API Mode.\n",
					card->devname,chan->if_name);
			} else {
				DEBUG_EVENT( "%s:%s: Running in TDM Voice API Mode.\n",
					card->devname,chan->if_name);
			}
		}

#ifdef CONFIG_PRODUCT_WANPIPE_ANNEXG
	} else	if (strcmp(conf->usedby, "ANNEXG") == 0) {
			DEBUG_EVENT("%s:%s: Interface running in ANNEXG mode!\n",
				wandev->name,chan->if_name);
			chan->common.usedby=ANNEXG;
			chan->usedby_cfg = chan->common.usedby;

			if (strlen(conf->label)){
				strncpy(chan->label,conf->label,WAN_IF_LABEL_SZ);
			}

#endif

	}else if (strcmp(conf->usedby, "STACK") == 0) {
		chan->common.usedby = STACK;
		chan->usedby_cfg = chan->common.usedby;
		DEBUG_EVENT( "%s:%s: Running in Stack mode.\n",
				card->devname,chan->if_name);

	}else if (strcmp(conf->usedby, "NETGRAPH") == 0) {
		chan->common.usedby = WP_NETGRAPH;
		chan->usedby_cfg = chan->common.usedby;
		DEBUG_EVENT( "%s:%s: Running in Netgraph mode.\n",
				card->devname,chan->if_name);

	}else{
		DEBUG_EVENT( "%s:%s: Error: Invalid IF operation mode %s\n",
				card->devname,chan->if_name,conf->usedby);
		err=-EINVAL;
		goto new_if_error;
	}



	if (chan->channelized_cfg || chan->wp_api_op_mode) {
		if (wan_netif_priv(dev)) {
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
		} else {
			wan_netif_set_priv(dev, chan);
		}
	} else {
		chan->channelized_cfg=0;
		wan_netif_set_priv(dev, chan);
	}



	/*===============================================
	 * Interface Operation Setup
	 *==============================================*/

	if (conf->hwec.enable){
		if (card->wandev.config_id == WANCONFIG_AFT_ISDN_BRI) {
			card->wandev.ec_enable_map = 0x3;
		}else{
			card->wandev.ec_enable_map |= conf->active_ch;
		}
	}

	/* Read user specified active_ch, we must do it
	 * here because the active_ch value might change
	 * for different user modes*/
	chan->time_slot_map=conf->active_ch;
	chan->num_of_time_slots=
		(u8)aft_get_num_of_slots(card->u.aft.num_of_time_slots,
			             chan->time_slot_map);

	if (!chan->num_of_time_slots){
		DEBUG_EVENT("%s: Error: Invalid number of timeslots in map 0x%08X!\n",
				chan->if_name,chan->time_slot_map);
		return -EINVAL;
	}

	if (card->wandev.config_id == WANCONFIG_AFT_ANALOG && chan->num_of_time_slots > 1) {
		DEBUG_EVENT(
		"%s: Error: Invalid Analog number of timeslots in map 0x%08X: (Valid=1)\n",
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


	if (!silent) {
		/* Print out the current configuration */
		if (!conf->hdlc_streaming) {
			DEBUG_EVENT("%s:    MRU           :%d (Chunk/Period=%d/%d)\n",
				card->devname,
				chan->mru,chan->tdm_api_chunk,chan->tdm_api_period);

			DEBUG_EVENT("%s:    MTU           :%d (Chunk/Period=%d/%d)\n",
				card->devname,
				chan->mtu,chan->tdm_api_chunk,chan->tdm_api_period);
		} else {
			DEBUG_EVENT("%s:    MRU           :%d\n",
				card->devname,
				chan->mru);

			DEBUG_EVENT("%s:    MTU           :%d\n",
				card->devname,
				chan->mtu);
		}


		chan->hdlc_eng = conf->hdlc_streaming;
#if defined(__WINDOWS__)
		dev->hdlc_eng = conf->hdlc_streaming;
		dev->udp_mgmt = &process_udp_mgmt_pkt;
//		dev->set_tx_idle_data_in_priv = &aft_te1_set_tx_idle_data_in_priv;
		dev->trace_info = &chan->trace_info;
#endif

		DEBUG_EVENT("%s:    HDLC Eng      :%s | %s\n",
			card->devname,
			chan->hdlc_eng?"On":"Off (Transparent)",
			chan->cfg.hdlc_repeat?"Repeat":"N/A");
	}

	/* Obtain the DMA MRU size based on user confgured
	 * MRU.  The DMA MRU must be of size 2^x */

	chan->dma_mru = chan->mtu;

	chan->dma_mru = aft_valid_mtu((u16)chan->dma_mru);
	if (!chan->dma_mru){
		DEBUG_EVENT("%s:%s: Error invalid MTU %d  MRU %d\n",
			card->devname,
			chan->if_name,
			chan->mtu,chan->mru);
		err= -EINVAL;
		goto new_if_error;
	}

	/* Note that A101/A102/BRI cards do not have any
       DMA Chains so disable dma chains for them */
	if (conf->single_tx_buf ||
	    IS_BRI_CARD(card) ||
	    ((card->adptr_type == A101_ADPTR_2TE1 ||
	      card->adptr_type == A101_ADPTR_1TE1) &&
	      card->u.aft.firm_id == AFT_DS_FE_CORE_ID)){
       		chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;
			chan->max_tx_bufs=MAX_AFT_DMA_CHAINS;
        	dma_per_ch=MAX_AFT_DMA_CHAINS;
	}


	if (!chan->hdlc_eng){

		/* If hardware HDLC engine is disabled:
		 *  1. Configure DMA chains for SINGLE DMA
		 *  2. Enable Timeslot Synchronization
		 *  3. Configure Interface for Transparent Operation */

		/* If we misconfigured for DMA CHAIN mode reset it back to single */
		if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN) {
			chan->dma_chain_opmode = WAN_AFT_DMA_CHAIN_SINGLE;
		}

		chan->max_tx_bufs=MAX_AFT_DMA_CHAINS;
		

		if (chan->channelized_cfg) {
			dma_per_ch=MAX_AFT_DMA_CHAINS;
		}else{
			dma_per_ch= (MAX_AFT_DMA_CHAINS*1500) / chan->mtu;
			if (dma_per_ch <  MAX_AFT_DMA_CHAINS) {
				dma_per_ch=MAX_AFT_DMA_CHAINS;
			}
		}

		if (chan->common.usedby == XMTP2_API) {
			dma_per_ch += 1024 * chan->num_of_time_slots;
		}

		if (chan->wp_api_op_mode ||
  			chan->dma_chain_opmode != WAN_AFT_DMA_CHAIN_SINGLE) {
			dma_per_ch += 32;
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

	if (!silent){
		DEBUG_EVENT("%s:    Data Mux Ctrl :%s\n",
			card->devname,
			chan->cfg.data_mux?"On":"Off");
	}



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

	if (wan_test_bit(WP_ZAPTEL_DCHAN_OPTIMIZATION,&card->u.aft.tdmv_zaptel_cfg)) {
		int ch=chan->logic_ch_num;
		if (IS_E1_CARD(card)) {
			ch++;
		}
		if (!wan_test_bit(ch,&chan->time_slot_map)) {
            DEBUG_EVENT("%s: Error: Logic channel %i does not map to timeslot map 0x%08X\n",
				   card->devname,chan->logic_ch_num,chan->time_slot_map); 	
			goto new_if_error;
		}	
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


#ifdef AFT_TDM_API_SUPPORT
	err=aft_tdm_api_init(card,chan,conf);
	if (err){
		goto new_if_error;
	}
#endif

	err=aft_hwec_config(card,chan,conf,1);
	if (err){
		goto new_if_error;
	}

	if (chan->channelized_cfg && !chan->hdlc_eng) {
		chan->dma_mru = 1024;
		dma_per_ch = 4;
	}

	if (!silent) {
		DEBUG_EVENT("%s:    DMA/Len/Idle/Chain/EC :%d/%d/%d/%s/%s\n",
			card->devname,
			dma_per_ch,
			chan->dma_mru,
			chan->max_idle_size,
			chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE ? "Off":
			chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN ? "On" :
			chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_IRQ_ALL ? "All" : "Unknown",
			card->wandev.ec_enable_map?"On":"Off");
	}

	/* New DMA support A-DMA */
	dma_alignment = 4;
	if (chan->channelized_cfg && !chan->hdlc_eng){
		dma_alignment = 0x200;
	}

	err = card->hw_iface.busdma_tag_create(	card->hw,
						&chan->rx_dma_chain_table[0],
						dma_alignment,
						chan->dma_mru,
						MAX_AFT_DMA_CHAINS);
	if (err) {
		DEBUG_EVENT("%s: Failed to allocate DMA Rx mtag!\n",
					card->devname);
		err = -EINVAL;
		goto new_if_error;
	}
	err = card->hw_iface.busdma_tag_create(	card->hw,
						&chan->tx_dma_chain_table[0],
						dma_alignment,
						chan->dma_mru,
						MAX_AFT_DMA_CHAINS);
	if (err) {
		DEBUG_EVENT("%s: Failed to allocate DMA Tx mtag!\n",
					card->devname);
		err = card->hw_iface.busdma_tag_destroy(
						card->hw,
						&chan->rx_dma_chain_table[0],
						MAX_AFT_DMA_CHAINS);
		err = -EINVAL;
		goto new_if_error;
	}

	for (i=0;i<MAX_AFT_DMA_CHAINS;i++){

#ifdef __WINDOWS__
		/*	initialize wan_dma_descr_t descriptors before calling
			hw_iface.busdma_alloc() and hw_iface.busdma_free()	*/
		chan->tx_dma_chain_table[i].DmaAdapterObject = 
		chan->rx_dma_chain_table[i].DmaAdapterObject = card->DmaAdapterObject;
#endif
		err = card->hw_iface.busdma_alloc(
						card->hw,
						&chan->tx_dma_chain_table[i]);
		if (err){
			DEBUG_EVENT(
			"%s:%s: Unable to load TX DMA buffer %d (%d)!\n",
					card->devname, chan->if_name, i, err);
			err = -EINVAL;
			break;
		}
		DEBUG_DMA("%s:%s: Alloc DMA TX buffer %d virt=%p len=%d\n",
					card->devname, chan->if_name, i,
					chan->tx_dma_chain_table[i].dma_virt,
					chan->dma_mru);

		err = card->hw_iface.busdma_alloc(
						card->hw,
						&chan->rx_dma_chain_table[i]);
		if (err){
			DEBUG_EVENT(
			"%s:%s: Unable to load RX DMA buffer %d (%d)!\n",
					card->devname, chan->if_name, i, err);
			err = -EINVAL;
			break;
		}
		DEBUG_DMA("%s:%s: Alloc DMA RX buffer %d virt=%p len=%d\n",
					card->devname, chan->if_name, i,
					chan->rx_dma_chain_table[i].dma_virt,
					chan->dma_mru);
	}
	if (err){

		for (i=0;i<MAX_AFT_DMA_CHAINS;i++){

			card->hw_iface.busdma_free(
						card->hw,
						&chan->rx_dma_chain_table[i]);
			card->hw_iface.busdma_free(
						card->hw,
						&chan->tx_dma_chain_table[i]);
		}
		err = card->hw_iface.busdma_tag_destroy(
						card->hw,
						&chan->rx_dma_chain_table[0],
						MAX_AFT_DMA_CHAINS);
		err = card->hw_iface.busdma_tag_destroy(
						card->hw,
						&chan->tx_dma_chain_table[0],
						MAX_AFT_DMA_CHAINS);
		err = -EINVAL;
		goto new_if_error;
	}

	if (if_cnt == 1) {
		DEBUG_EVENT("%s:    Memory: Chan=%d\n",
				card->devname, sizeof(private_area_t));
	}



	err=aft_alloc_rx_dma_buff(card, chan, dma_per_ch,0);
	if (err){
		goto new_if_error;
	}

	chan->dma_per_ch = (u16)dma_per_ch;

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
#if defined(__LINUX__) || defined(__WINDOWS__)
	dev->init = &if_init;
# if defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
	if_init(dev);
# endif
#else
	if (chan->common.usedby != TDM_VOICE &&
	    chan->common.usedby != TDM_VOICE_API){
		chan->common.is_netdev = 1;
	}
	chan->common.iface.open      = &if_open;
    chan->common.iface.close     = &if_close;
    chan->common.iface.output    = &if_send;
    chan->common.iface.ioctl     = &if_do_ioctl;
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

	/* Just make sure that we did not go connected for any reason
	   during startup. This is just a sanity check */
	if (chan->common.state != WAN_CONNECTED) {
		set_chan_state(card, dev, WAN_DISCONNECTED);
	}

	DEBUG_EVENT( "\n");
	return 0;

new_if_error:
	FUNC_NEWDRV();
	return err;


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

static int new_if (wan_device_t* wandev, netdevice_t* dev, wanif_conf_t* conf)
{
	int err=-EINVAL;
	sdla_t *card=wandev->priv;

	wan_netif_set_priv(dev, NULL);

	conf->cfg_active_ch = conf->active_ch;
	if (IS_E1_CARD(card) && !(WAN_FE_FRAME(&card->fe) == WAN_FR_UNFRAMED)) {
		conf->active_ch = conf->active_ch << 1;
		wan_clear_bit(0,&conf->active_ch);
	}else if (IS_T1_CARD(card)) {
		conf->active_ch = conf->active_ch&~(0xFF000000);
		conf->cfg_active_ch =  conf->cfg_active_ch&~(0xFF000000);
	}else if (IS_56K_CARD(card)) {
		conf->active_ch = 1;
		conf->cfg_active_ch = 1;
	}

	if (strcmp(conf->usedby, "TDM_VOICE") == 0 ) {
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
		if (card->tdmv_conf.span_no){

			switch(card->wandev.config_id){
			case WANCONFIG_AFT_ANALOG:
				err = wp_tdmv_remora_init(&card->tdmv_iface);
				break;
#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)
			case WANCONFIG_AFT_ISDN_BRI:
				err = wp_tdmv_bri_init(&card->tdmv_iface);
				break;
#endif
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

#if defined(__WINDOWS__)
	/* TDM_VOICE_API for T1/E1 is always SPAN mode, for all other  
	 * cards the TDM_VOICE_API is channelized just like in Linux */
	if (strcmp(conf->usedby, "TDM_VOICE_API") == 0) {
		if (IS_TE1_CARD(card) || IS_BRI_CARD(card)) {
			sprintf(conf->usedby, "WIN_TDM_LEGACY_SPAN_API");
		}
	}
#endif

	if (strcmp(conf->usedby, "TDM_VOICE") == 0 ||
		strcmp(conf->usedby, "TDM_VOICE_API") == 0 ||
		strcmp(conf->usedby, "TDM_CHAN_VOICE_API") == 0 ||
		strcmp(conf->usedby, "TDM_SPAN_VOICE_API") ==0){

		int i=0,master_if=-1;
		int if_cnt=0;
		u32 active_ch=conf->active_ch;


		if (!card->tdmv_conf.span_no){
				DEBUG_EVENT("\n");
				DEBUG_EVENT("%s: Error: TDM SPAN not configured! Failed to start channel!\n",
						card->devname);
				return -EINVAL;
		}

		if(IS_BRI_CARD(card)) {
			wan_set_bit(BRI_DCHAN_LOGIC_CHAN,&card->u.aft.tdmv_dchan);
		}

		if (card->wandev.fe_iface.active_map){
			conf->active_ch = card->wandev.fe_iface.active_map(&card->fe, 0);

			if(IS_BRI_CARD(card)) {
				wan_set_bit(BRI_DCHAN_LOGIC_CHAN,&conf->active_ch);
				wan_set_bit(BRI_DCHAN_LOGIC_CHAN,&card->tdmv_conf.dchan);
			}

			active_ch=conf->active_ch;
		}

		err=aft_find_master_if_and_dchan(card,&master_if,active_ch);
		if (err < 0) {
			return err;
		}

		DEBUG_TEST("%s: TDM VOICE: DCHAN CHAN in 0x%08X Timeslots=%d CFG DCHAN=0x%08X MasterIF=%d\n",
				card->devname, active_ch, card->u.aft.num_of_time_slots,
				card->tdmv_conf.dchan,master_if);

		if (strcmp(conf->usedby, "TDM_SPAN_VOICE_API") ==0) {
			
			conf->active_ch=active_ch&~(card->tdmv_conf.dchan);
			conf->u.aft.tdmv_master_if=1;
			err=new_if_private(wandev,dev,conf,0,-1,if_cnt);
			if (!err){

				/* Setup DCHAN if any */
				conf->u.aft.tdmv_master_if=0;
				for (i=0;i<card->u.aft.num_of_time_slots;i++){
					if (wan_test_bit(i,&card->tdmv_conf.dchan)){
						int dchan=-1;
	
						conf->active_ch=0;
						conf->u.aft.tdmv_master_if=0;
						wan_set_bit(i,&conf->active_ch);
						dchan=i;
						if_cnt++;
						err=new_if_private(wandev,dev,conf,0,dchan,if_cnt);
						if (err){
							break;
						}
					}
				}
			}

		} else {

			for (i=0;i<card->u.aft.num_of_time_slots;i++){
	
				if (wan_test_bit(i,&active_ch)){
					int dchan=-1;
					conf->active_ch=0;
					conf->u.aft.tdmv_master_if=0;
					wan_set_bit(i,&conf->active_ch);

					card->u.aft.global_tdm_irq=1;

					if (wan_test_bit(i,&card->tdmv_conf.dchan)){
						dchan=i;
					}
					if (i==master_if){
						conf->u.aft.tdmv_master_if=1;
					}

					err=new_if_private(wandev,dev,conf,1,dchan,if_cnt);
					if (err){
						break;
					}
					if_cnt++;
				}
			}
		}

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		if (!err && card->u.aft.tdmv_zaptel_cfg){
			WAN_TDMV_CALL(software_init, (&card->wan_tdmv), err);
		}
#endif
	}else{

		if(IS_BRI_CARD(card)) {

			if (conf->active_ch & ~(0x07)) {
				DEBUG_EVENT("%s: Error: BRI Active Channels range 1 to 3: Range 0x%08X invalid\n",
						card->devname,conf->active_ch);
				err=-EINVAL;
				goto new_if_cfg_skip;
			}

			if (wan_test_bit(BRI_DCHAN_ACTIVE_CFG_CHAN, &conf->active_ch)) {
				/* if bit 2 is set, user wants to run on the bri dchan */
				wan_set_bit(BRI_DCHAN_LOGIC_CHAN, &card->u.aft.tdmv_dchan);
				card->u.aft.tdmv_dchan = BRI_DCHAN_LOGIC_CHAN;
				err=new_if_private(wandev,dev,conf,0, BRI_DCHAN_LOGIC_CHAN,0);
			} else {
				/* BRI B-Channels */
				card->u.aft.tdmv_dchan = 0;
				conf->active_ch = conf->active_ch << (2*WAN_FE_LINENO(&card->fe));
				DEBUG_EVENT("%s: Configure BRI Active CH 0x%08X\n",
					 card->devname,conf->active_ch);
				err=new_if_private(wandev,dev,conf,0,-1,0);
			}

		} else {

			card->tdmv_conf.dchan=0;
			err=new_if_private(wandev,dev,conf,0,-1,0);
		}
	}

new_if_cfg_skip:

	if (err == 0 && wan_netif_priv(dev)) {
		wan_smp_flag_t flags;
		int card_use_counter;

		card->hw_iface.getcfg(card->hw, SDLA_HWCPU_USEDCNT, &card_use_counter);

	       /* If FRONT End is down, it means that the DMA
		* is disabled.  In this case don't try to
		* reset fifo.  Let the enable_data_error_intr()
		* function do this, after front end has come up */

		wan_spin_lock_irq(&card->wandev.lock,&flags);
		aft_dev_open(card, wan_netif_priv(dev));
		wan_spin_unlock_irq(&card->wandev.lock,&flags);

		if (card->wandev.state == WAN_CONNECTED){
			set_chan_state(card, dev, WAN_CONNECTED);
		}

		AFT_FUNC_DEBUG();

		if (card->wandev.config_id == WANCONFIG_AFT_ANALOG ||
		    card->wandev.config_id == WANCONFIG_AFT_SERIAL) {

			card->hw_iface.hw_lock(card->hw,&flags);
			
			card->fe.fe_status = FE_CONNECTED;
			handle_front_end_state(card,1);
			set_chan_state(card, dev, WAN_CONNECTED);

			if (card->wandev.config_id == WANCONFIG_AFT_SERIAL) {
				u32 reg=0;
				__sdla_bus_read_4(card->hw, AFT_PORT_REG(card,AFT_SERIAL_LINE_CFG_REG), &reg);
				card->u.aft.serial_status=reg&AFT_SERIAL_LCFG_CTRL_BIT_MASK;
			}

			card->hw_iface.hw_unlock(card->hw,&flags);

		} else if (card->wandev.config_id == WANCONFIG_AFT_ISDN_BRI) {
			wan_smp_flag_t smp_flags;

			card->hw_iface.hw_lock(card->hw,&smp_flags);

			wan_spin_lock_irq(&card->wandev.lock,&flags);
			enable_data_error_intr(card);

			wan_spin_unlock_irq(&card->wandev.lock,&flags);

#if defined (BUILD_MOD_TESTER)
			handle_front_end_state(card,1);
#endif     
			
			card->hw_iface.hw_unlock(card->hw,&smp_flags);
		}

	} else if (err && wan_netif_priv(dev)){
		del_if(wandev,dev);
		if (wan_netif_priv(dev)){
			wan_free(wan_netif_priv(dev));
			wan_netif_set_priv(dev, NULL);
		}
	}

	AFT_FUNC_DEBUG();

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
	wan_smp_flag_t 		flags;
	int	i;

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


#ifdef CONFIG_PRODUCT_WANPIPE_ANNEXG
	if (chan->common.usedby == ANNEXG && chan->annexg_dev){
		netdevice_t *tmp_dev;
		int err;

		DEBUG_EVENT("%s: Unregistering Lapb Protocol\n",wandev->name);

		if (!IS_FUNC_CALL(lapb_protocol,lapb_unregister)){
			wan_spin_lock_irq(&wandev->lock, &flags);
			chan->annexg_dev = NULL;
			wan_spin_unlock_irq(&wandev->lock, &flags);
			return 0;
		}

		wan_spin_lock_irq(&wandev->lock, &flags);
		tmp_dev=chan->annexg_dev;
		chan->annexg_dev=NULL;
		wan_spin_unlock_irq(&wandev->lock, &flags);

		if ((err=lapb_protocol.lapb_unregister(tmp_dev))){
			wan_spin_lock_irq(&wandev->lock, &flags);
			chan->annexg_dev=tmp_dev;
			wan_spin_unlock_irq(&wandev->lock, &flags);
			return err;
		}
	}
#endif

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

	aft_ss7_if_unreg(card,chan);

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

    wan_spin_unlock_irq(&card->wandev.lock,&flags);

	for (i=0;i<MAX_AFT_DMA_CHAINS;i++){

		card->hw_iface.busdma_free(
					card->hw,
					&chan->rx_dma_chain_table[i]);
		card->hw_iface.busdma_free(
					card->hw,
					&chan->tx_dma_chain_table[i]);
	}
	card->hw_iface.busdma_tag_destroy(
					card->hw,
					&chan->rx_dma_chain_table[0],
					MAX_AFT_DMA_CHAINS);
	card->hw_iface.busdma_tag_destroy(
					card->hw,
					&chan->tx_dma_chain_table[0],
					MAX_AFT_DMA_CHAINS);

	WAN_IFQ_DMA_PURGE(&chan->wp_rx_free_list);
	WAN_IFQ_DESTROY(&chan->wp_rx_free_list);

	WAN_IFQ_PURGE(&chan->wp_rx_complete_list);
	WAN_IFQ_DESTROY(&chan->wp_rx_complete_list);

	WAN_IFQ_PURGE(&chan->wp_rx_stack_complete_list);
	WAN_IFQ_DESTROY(&chan->wp_rx_stack_complete_list);

	WAN_IFQ_PURGE(&chan->wp_tx_pending_list);
	WAN_IFQ_DESTROY(&chan->wp_tx_pending_list);

	WAN_IFQ_PURGE(&chan->wp_rx_bri_dchan_complete_list);
	WAN_IFQ_DESTROY(&chan->wp_rx_bri_dchan_complete_list);

	WAN_IFQ_PURGE(&chan->wp_tx_complete_list);
	WAN_IFQ_DESTROY(&chan->wp_tx_complete_list);

	WAN_IFQ_PURGE(&chan->wp_dealloc_list);
	WAN_IFQ_DESTROY(&chan->wp_dealloc_list);

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

	if (chan->udp_pkt_data) {
		wan_free(chan->udp_pkt_data);
	}

	chan->logic_ch_num=-1;

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
	int card_use_cnt=0;
	int err=0;

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


	card->hw_iface.getcfg(card->hw, SDLA_HWCPU_USEDCNT, &card_use_cnt);

	wan_spin_lock_irq(&card->wandev.lock,&flags);
	aft_dev_close(card,chan);
	wan_spin_unlock_irq(&card->wandev.lock,&flags);

	if (chan->channelized_cfg || chan->wp_api_op_mode) {

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

		if (chan->channelized_cfg) {
			if (IS_BRI_CARD(card)) {
				int card_use_cnt;
	
				card->wandev.state=WAN_DISCONNECTED;
	
				card->hw_iface.getcfg(card->hw, SDLA_HWTYPE_USEDCNT, &card_use_cnt);
				if (card_use_cnt == 1) {
					DEBUG_TEST("%s: BRI Disabling TDMV INTR\n",
						card->devname);
					aft_tdm_intr_ctrl(card,0);
					aft_fifo_intr_ctrl(card, 0);
				} else {
					u32 dmareg;
					/* By removing timeslot out of the global
					interrupt we can disable the tdm global isr.
					This code re-triggers it just in case */
					card->hw_iface.bus_read_4(card->hw,
						AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&dmareg);
						wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&dmareg);
						wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&dmareg);
						card->hw_iface.bus_write_4(card->hw,
						AFT_PORT_REG(card,AFT_DMA_CTRL_REG),dmareg);
				}
			} else {
				aft_tdm_intr_ctrl(card,0);
				aft_fifo_intr_ctrl(card, 0);
			}
		}

		/* Disable RTP Tap */
		card->wandev.rtp_len=0;

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
			wan_netif_set_priv(dev, chan->next);
			wan_free(chan);
			chan = wan_netif_priv(dev);
		}

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		if (card->wan_tdmv.sc) {
			aft_tdmv_free(card);
		}

		if (card->sdla_tdmv_dummy) {
			sdla_tdmv_dummy_unregister(card->sdla_tdmv_dummy);
		}
#endif

		err=0;

	} else {
		err=del_if_private(wandev,dev);
		wan_netif_set_priv(dev, NULL);
		wan_free(chan);
	}

	return err;

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

/* Wait for 100 ms hardcoded */
static void disable_comm_delay(void)
{
        unsigned long timeout=SYSTEM_TICKS;
        while ((SYSTEM_TICKS-timeout)< (HZ/10)){
                WP_SCHEDULE(10,"disable_comm_delay");
        }
}

static void disable_comm (sdla_t *card)
{
	wan_smp_flag_t smp_flags,smp_flags1;
	int used_cnt, used_type_cnt;
	int port_task_timeout=0;
	int err=0;

	AFT_FUNC_DEBUG();
#if INIT_FE_ONLY
...
	aft_chip_unconfigure(card);
#else

	/* Make sure that Task is disabled for this card */
	wan_set_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical);

	while (wan_test_bit(CARD_PORT_TASK_RUNNING,&card->wandev.critical)) {
		disable_comm_delay();	/* 100 ms */
		port_task_timeout++;
		if (port_task_timeout > 20) {
			DEBUG_EVENT("%s: Warning: Port Task stuck! timing out!\n",card->devname);
			break;
		}
	}

#if defined(WAN_TASKQ_STOP)
	err=WAN_TASKQ_STOP((&card->u.aft.port_task));
	DEBUG_EVENT("%s: %s\n",card->devname, err?"TASKQ Successfully Stopped":"TASKQ Not Running");
#endif


   	/* Unconfiging, only on shutdown */
	if (card->wandev.fe_iface.pre_release){
		card->wandev.fe_iface.pre_release(&card->fe);
	}
	card->hw_iface.hw_lock(card->hw,&smp_flags1);
	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
	if (card->wandev.fe_iface.unconfig){
		card->wandev.fe_iface.unconfig(&card->fe);
	}
	wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
	card->hw_iface.hw_unlock(card->hw,&smp_flags1);

	/* Disable DMA ENGINE before we perform
         * core reset.  Otherwise, we will receive
         * rx fifo errors on subsequent resetart. */
	wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
	disable_data_error_intr(card,DEVICE_DOWN);

	aft_handle_clock_master(card);

#if defined(AFT_RTP_SUPPORT)
	aft_rtp_unconfig(card);
#endif


	__aft_fe_intr_ctrl(card, 0);
	wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);

	aft_chip_unconfigure(card);

	/* Only disable the irq completely once the
	   chip unconfigure is executed */
	wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
        disable_data_error_intr(card,DEVICE_DOWN);
	wan_set_bit(CARD_DOWN,&card->wandev.critical);
	wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);

	WP_DELAY(10);

	card->hw_iface.getcfg(card->hw, SDLA_HWCPU_USEDCNT, &used_cnt);
	card->hw_iface.getcfg(card->hw, SDLA_HWTYPE_USEDCNT, &used_type_cnt);

	card->hw_iface.hw_lock(card->hw,&smp_flags1);
	wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
	aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_RED, 0,WAN_AFT_ON);
	aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_GREEN, 0, WAN_AFT_ON);
	
	__sdla_pull_ptr_isr_array(card->hw,card,WAN_FE_LINENO(&card->fe));
	
	wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);
	card->hw_iface.hw_unlock(card->hw,&smp_flags1);
	
	wp_timer_device_unreg(card);

	if (used_type_cnt<=1){
		DEBUG_EVENT("%s: Global Chip Shutdown Usage=%d, Used_type_cnt=%d\n",
				card->devname,used_cnt, used_type_cnt);

		card->hw_iface.hw_lock(card->hw,&smp_flags1);
		wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
		aft_global_chip_disable(card);
		wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);
		card->hw_iface.hw_unlock(card->hw,&smp_flags1);
	} else {
		/* only re-enable front end interrupt if this
		 * card is not last card */
		aft_fe_intr_ctrl(card, 1);
	}

	/* Initialize the AFT operation structure so on subsequent restart
           the card->u.aft strcutre is fully initialized */
	memset(&card->u.aft,0,sizeof(card->u.aft));
#endif
	return;
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
#if defined(__LINUX__) || defined(__WINDOWS__)
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
	    chan->common.usedby == TDM_VOICE_API){
		dev->tx_timeout		= NULL;
	}else{
		dev->tx_timeout		= &if_tx_timeout;
	}
	dev->watchdog_timeo	= 2*HZ;

#endif
	dev->do_ioctl		= if_do_ioctl;
	dev->change_mtu		= if_change_mtu;

	if (chan->common.usedby == BRIDGE ||
            chan->common.usedby == BRIDGE_NODE){
#if defined(__WINDOWS__)
		FUNC_NOT_IMPL();
#else
		/* Setup the interface for Bridging */
		int hw_addr=0;
		ether_setup(dev);

		/* Use a random number to generate the MAC address */
		memcpy(dev->dev_addr, "\xFE\xFC\x00\x00\x00\x00", 6);
		get_random_bytes(&hw_addr, sizeof(hw_addr));
		*(int *)(dev->dev_addr + 2) += hw_addr;
#endif
	}else{
#if !defined(__WINDOWS__)
		dev->flags     |= IFF_POINTOPOINT;
		dev->flags     |= IFF_NOARP;
		dev->type	= ARPHRD_PPP;
#endif
		dev->mtu		= chan->mtu;
		dev->hard_header_len	= 0;

		if (chan->common.usedby == API || chan->common.usedby == STACK){
			if (chan->hdlc_eng) {
				dev->mtu = chan->dma_mru+sizeof(wp_api_hdr_t);
			}else{
				dev->mtu = chan->mtu+sizeof(wp_api_hdr_t);
			}
		}

#if !defined(__WINDOWS__)
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
#endif
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


	FUNC_NEWDRV();

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
	
	wan_set_bit(WP_DEV_UP,&chan->up);

#if !defined(WANPIPE_IFNET_QUEUE_POLICY_INIT_OFF)
	WAN_NETIF_START_QUEUE(dev);
	wan_chan_dev_start(chan);
#endif

	if (card->wandev.state == WAN_CONNECTED){
		set_chan_state(card, dev, WAN_CONNECTED);
   		WAN_NETIF_CARRIER_ON(dev);
	} else {
   		WAN_NETIF_CARRIER_OFF(dev);
	}

	/* Increment the module usage count */
	wanpipe_open(card);


	if (card->wandev.config_id == WANCONFIG_AFT_56K) {
		FUNC_NEWDRV();
		if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
			wan_set_bit(AFT_FE_POLL,&card->u.aft.port_task_cmd);
			WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
		}
	}

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

	wan_clear_bit(WP_DEV_UP,&chan->up);

	WAN_NETIF_STOP_QUEUE(dev);
	wan_chan_dev_stop(chan);

#if defined(LINUX_2_1)
	dev->start=0;
#endif
	protocol_stop(card,dev);

	wanpipe_close(card);

	return 0;
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

	WAN_NETIF_STATS_INC_COLLISIONS(&chan->common);	//++chan->if_stats.collisions;
	WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,collisions);

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
			WAN_NETIF_STATS_TX_PACKETS(&chan->common));

	if (wan_test_bit(TX_DMA_BUSY,&chan->dma_status)){
		wan_clear_bit(TX_DMA_BUSY,&chan->dma_status);
	}

	wan_netif_set_ticks(dev, SYSTEM_TICKS);

#ifdef AFT_TX_FIFO_DEBUG
	aft_list_tx_descriptors(chan);
#endif

	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
	if (chan->channelized_cfg || chan->wp_api_op_mode){
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

	wanpipe_wake_stack(chan);

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

#if defined(__LINUX__) || defined(__WINDOWS__)
static int if_send (netskb_t* skb, netdevice_t* dev)
#else
static int if_send(netdevice_t *dev, netskb_t *skb, struct sockaddr *dst,struct rtentry *rt)
#endif
{
	private_area_t *chan = wan_netif_priv(dev);
	sdla_t *card = chan->card;
	int err;
	netskb_t *rskb=NULL;
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

		wan_chan_dev_start(chan);
		WAN_NETIF_WAKE_QUEUE(dev);
		return 0;
	}

	DEBUG_TX("%s: Sending %d bytes\n", wan_netif_name(dev), wan_skb_len(skb));
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
        WAN_NETIF_STATS_COLLISIONS(&chan->common);	//++chan->if_stats.collisions;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,collisions);
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
		wan_chan_dev_stop(chan);
		wan_netif_set_ticks(dev, SYSTEM_TICKS);
		WAN_NETIF_STATS_INC_TX_CARRIER_ERRORS(&chan->common);	//++chan->if_stats.tx_carrier_errors;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_carrier_errors);
		return 1;
#else
		WAN_NETIF_STATS_INC_TX_CARRIER_ERRORS(&chan->common);	//++chan->if_stats.tx_carrier_errors;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_carrier_errors);
		wan_skb_free(skb);
		WAN_NETIF_START_QUEUE(dev);
		wan_chan_dev_start(chan);
		err=0;
		goto if_send_exit_crit;
#endif
	}


	if (chan->channelized_cfg) {

		private_area_t *top_chan=wan_netif_priv(chan->common.dev);

		DEBUG_TEST("%s:%ld Prev: Zaptel HDLC Tt TDMV_DCHAN=%d\n",
				chan->if_name,chan->logic_ch_num,
				card->u.aft.tdmv_dchan-1);

		if (!card->u.aft.tdmv_dchan || card->u.aft.tdmv_dchan>32){

			DEBUG_EVENT("%s: DCHAN TX No DCHAN Configured!\n",
					card->devname);

			wan_skb_free(skb);
			WAN_NETIF_START_QUEUE(dev);
			wan_chan_dev_start(chan);
			err=0;
			goto if_send_exit_crit;
		}

		if(IS_BRI_CARD(card)) {
			chan=(private_area_t*)card->u.aft.dev_to_ch_map[BRI_DCHAN_LOGIC_CHAN];
		}else{
			chan=(private_area_t*)card->u.aft.dev_to_ch_map[card->u.aft.tdmv_dchan-1];
		}

		if (!chan){
			DEBUG_EVENT("%s: Warning: DCHAN TX: No DCHAN Configured (not preset)! Discarding data.\n",
					card->devname);
			wan_skb_free(skb);
			WAN_NETIF_START_QUEUE(dev);
			wan_chan_dev_start(chan);
			err=0;
			goto if_send_exit_crit;
		}

		if (!chan->hdlc_eng || (IS_BRI_CARD(card) && chan->dchan_time_slot < 0)){
			wan_skb_free(skb);
			WAN_NETIF_START_QUEUE(dev);
			wan_chan_dev_start(chan);
			err=0;
			goto if_send_exit_crit;
		}

		wan_capture_trace_packet(chan->card, &top_chan->trace_info,
					     skb,TRC_OUTGOING_FRM);

		DEBUG_TEST("%s:%ld Zaptel HDLC Tt TDMV_DCHAN=%d\n",
				chan->if_name,chan->logic_ch_num,
				card->u.aft.tdmv_dchan-1);
	}

	/* For TDM_VOICE_API no tx is supported in if_send */
	if (chan->common.usedby == TDM_VOICE_API || chan->wp_api_op_mode){
		DEBUG_EVENT("%s: Warning: IF SEND TX: Chan in VOICE API Mode\n",
					card->devname);
		WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
		WAN_NETIF_START_QUEUE(dev);
		wan_chan_dev_start(chan);
		err=0;
		goto if_send_exit_crit;
	}

	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
	if (wan_skb_queue_len(&chan->wp_tx_pending_list) > chan->max_tx_bufs){
		DEBUG_TEST("%s(): line:%d: wp_tx_pending_list: %u, max_tx_bufs:%u\n",
			__FUNCTION__, __LINE__, 
			wan_skb_queue_len(&chan->wp_tx_pending_list),
			chan->max_tx_bufs);
		WAN_NETIF_STOP_QUEUE(dev);
		wan_chan_dev_stop(chan);
		aft_dma_tx(card,chan);
		wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
		return 1;
	}
	wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);

	if (chan->common.usedby == API){

		if (sizeof(wp_api_hdr_t) >= wan_skb_len(skb)){

			wan_skb_free(skb);
			WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
            WAN_NETIF_START_QUEUE(dev);
			wan_chan_dev_start(chan);
			err=0;
			goto if_send_exit_crit;
		}

		if (chan->cfg.ss7_enable) {
			err=aft_ss7_tx_mangle(card,chan,skb);
			if (err){
				wan_skb_free(skb);
				WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
				WAN_NETIF_START_QUEUE(dev);
				wan_chan_dev_start(chan);
				err=0;
				goto if_send_exit_crit;
			}
		} else if (chan->cfg.hdlc_repeat) {
			err=aft_hdlc_repeat_mangle(card,chan,skb,&rskb);
			if (err){
				wan_skb_free(skb);
				WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
				WAN_NETIF_START_QUEUE(dev);
				wan_chan_dev_start(chan);
				err=0;
				goto if_send_exit_crit;
			}
		} else {
			wan_skb_pull(skb,sizeof(wp_api_hdr_t));
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
			WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//++chan->if_stats.tx_errors;
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
			WAN_NETIF_START_QUEUE(dev);
			wan_chan_dev_start(chan);
			err=0;
			goto if_send_exit_crit;
		}
	}

	if (!chan->hdlc_eng && !chan->lip_atm && (wan_skb_len(skb)%4)){
		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s: Tx Error: Tx Length %d is not 32bit divisible\n",
				chan->if_name,wan_skb_len(skb));
		}
		wan_skb_free(skb);
		WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//++chan->if_stats.tx_errors;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
		WAN_NETIF_START_QUEUE(dev);
		wan_chan_dev_start(chan);
		err=0;
		goto if_send_exit_crit;
	}

#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)
	if(IS_BRI_CARD(card)){
		if(chan->dchan_time_slot >= 0){

			/* NOTE: BRI dchan tx has to be done inside hw lock.
				It allows to synchronize access to SPI on the card.
			*/
			card->hw_iface.hw_lock(card->hw,&smp_flags);

			err=aft_bri_dchan_transmit(card, chan,
						wan_skb_data(skb),
						wan_skb_len(skb));

			card->hw_iface.hw_unlock(card->hw,&smp_flags);

			if (err == 0 ) {
				WAN_NETIF_START_QUEUE(dev);
				wan_chan_dev_start(chan);
				wan_skb_free(skb);
				err=0;
				goto if_send_exit_crit;
			}else{
				err=1;
				WAN_NETIF_STOP_QUEUE(dev);
				wan_chan_dev_stop(chan);
				goto if_send_exit_crit;
			}
		} else {
			/* On b-channel data is transmitted using AFT DMA.
			   Drop down to code below */
		}
	}
#endif

	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);

	wan_skb_unlink(skb);

	wan_skb_queue_tail(&chan->wp_tx_pending_list,skb);

	if(!chan->lip_atm){
		aft_dma_tx(card,chan);	/*not needed for LIP_ATM!!*/
	}

	if (rskb) {
		wan_skb_queue_tail(&chan->wp_tx_hdlc_rpt_list,rskb);
	}

	wan_netif_set_ticks(dev, SYSTEM_TICKS);
	WAN_NETIF_START_QUEUE(dev);
	wan_chan_dev_start(chan);
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
         	 DEBUG_EVENT("%s: Resizing Tx Queue Len to %d\n",
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
#if defined(__LINUX__) || defined(__WINDOWS__)
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
			wan_test_bit(WP_ZAPTEL_DCHAN_OPTIMIZATION,&card->u.aft.tdmv_zaptel_cfg) &&
		    card->wandev.state == WAN_CONNECTED &&
		    card->wandev.config_id != WANCONFIG_AFT_ANALOG &&
		    chan->common.usedby == TDM_VOICE) {
			chan->common.if_stats.rx_packets = card->wandev.stats.rx_packets;
			chan->common.if_stats.tx_packets = card->wandev.stats.tx_packets;
		} 
#endif
	}
#endif

#if 0
#warning "DMA CHAIN DEBUGGING"
	aft_tx_dma_chain_diff(chan);
#endif

	return &chan->common.if_stats;

}
#endif

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
static int
if_do_ioctl(netdevice_t *dev, struct ifreq *ifr, wan_ioctl_cmd_t cmd)
{
	private_area_t* chan= (private_area_t*)wan_netif_priv(dev);
	sdla_t *card;
	wan_smp_flag_t smp_flags;
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
			WAN_NETIF_STATS_RX_DROPPED(&chan->common)=0;	//chan->if_stats.rx_dropped=0;
			if (!chan->hdlc_eng){
				WAN_NETIF_STATS_TX_CARRIER_ERRORS(&chan->common)=0;	//chan->if_stats.tx_carrier_errors=0;
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

#if !defined(__WINDOWS__)
#if defined(AFT_API_SUPPORT)
		case SIOC_WANPIPE_API:
			DEBUG_TEST("%s: WANPIPE API IOCTL!\n", card->devname);
			err = wan_aft_api_ioctl(card,chan,ifr->ifr_data);
			break;
#endif
#endif
		case SIOC_WAN_DEVEL_IOCTL:
			err = aft_devel_ioctl(card, ifr);
			break;

#if !defined(__WINDOWS__)
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
#endif

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
			err=wan_user_process_udp_mgmt_pkt(card,chan,ifr->ifr_data);
			break;

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
			if (card->wandev.hdlc_ioctl){
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
	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE){
		dma_descr=(chan->logic_ch_num<<4) + AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
		aft_dmachain_set_rx_dma_addr(&reg,0);
		card->hw_iface.bus_write_4(card->hw,dma_ram_desc,reg);
	}else {
		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
		cur_dma_ptr=aft_dmachain_get_rx_dma_addr(reg);

		dma_descr=(chan->logic_ch_num<<4) + (cur_dma_ptr*AFT_DMA_INDEX_OFFSET) +
					AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);
	}

	reg=0;
	wan_set_bit(AFT_RXDMA_HI_DMA_CMD_BIT,&reg);

	DEBUG_TEST("%s: Clearing RX Fifo %s Ch=%ld DmaDescr=(0x%X) Reg=(0x%X) WAIT=%s\n",
							__FUNCTION__,chan->if_name,chan->logic_ch_num,
							dma_descr,reg,wait == WP_WAIT?"YES":"NO");

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
			int max_logic_ch;
			u32 dmareg;
			card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&dmareg);
			max_logic_ch = aft_dmactrl_get_max_logic_ch(dmareg);

			DEBUG_EVENT("%s:%s: Error: Rx fifo reset timedout %u us (ch=%d)  DMA CTRL=0x%08X  DMA MAX=%d\n",
				card->devname,
				chan->if_name,
				i*FIFO_RESET_TIMEOUT_US,
				chan->logic_ch_num,
				dmareg,max_logic_ch);
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

	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE){
		dma_descr=(chan->logic_ch_num<<4) + AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);

		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
		aft_dmachain_set_tx_dma_addr(&reg,0);
		card->hw_iface.bus_write_4(card->hw,dma_ram_desc,reg);
	}else {
		dma_ram_desc=chan->logic_ch_num*4 + AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
		card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
		cur_dma_ptr=aft_dmachain_get_tx_dma_addr(reg);

		/* Clean TX DMA fifo */
		dma_descr=(chan->logic_ch_num<<4) + (cur_dma_ptr*AFT_DMA_INDEX_OFFSET) +
			  AFT_PORT_REG(card,AFT_TX_DMA_HI_DESCR_BASE_REG);
	}

	reg=0;
	wan_set_bit(AFT_TXDMA_HI_DMA_CMD_BIT,&reg);


	DEBUG_TEST("%s: Clearing TX Fifo %s Ch=%ld DmaDescr=(0x%X) Reg=(0x%X) WAIT=%s\n",
							__FUNCTION__,chan->if_name,chan->logic_ch_num,
							dma_descr,reg,wait == WP_WAIT?"YES":"NO");

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
               	 	DEBUG_EVENT("%s:%s: Error: Tx fifo reset timedout %u us (ch=%d)\n",
				card->devname,
				chan->if_name,
				i*FIFO_RESET_TIMEOUT_US,
				chan->logic_ch_num);
        	}else{
                	DEBUG_TEST("%s:%s: Tx Fifo Reset Successful\n",
                                card->devname,chan->if_name);
        	}
	}else{
		timeout=0;
	}

	return timeout;
}

static void aft_dev_enable(sdla_t *card, private_area_t *chan)
{

	DEBUG_CFG("%s: Enabling Global Inter Mask !\n",chan->if_name);

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

		DEBUG_CFG("%s: Enabling FOR NON CHANNELIZED !\n",chan->if_name);

#ifdef AFT_SPAN_SINGLE_IRQ
		if (chan->wp_api_op_mode && !chan->hdlc_eng) {
			aft_channel_txintr_ctrl(card,chan,0);
		} else {
			aft_channel_txintr_ctrl(card,chan,1);
		}
#else
		aft_channel_txintr_ctrl(card,chan,1);
#endif
		aft_channel_rxintr_ctrl(card,chan,1);
	}

	wan_set_bit(chan->logic_ch_num,&card->u.aft.active_ch_map);


	DEBUG_CFG("%s: ACTIVE CH MAP !\n",chan->if_name);
}

static void aft_dev_open_private(sdla_t *card, private_area_t *chan)
{

	if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
		/* BRI dchan does not use DMA thus
		   skip the dma config code below */
		return;
	}

	if (card->wandev.state == WAN_CONNECTED &&
	    wan_test_bit(0,&card->u.aft.comm_enabled)){

		DEBUG_TEST("%s: OPEN reseting fifo Channel = %li\n",
					chan->if_name,chan->logic_ch_num);

		aft_tslot_sync_ctrl(card,chan,0);

		aft_init_rx_dev_fifo(card,chan,WP_NO_WAIT);
		aft_init_tx_dev_fifo(card,chan,WP_NO_WAIT);

		aft_dev_enable(card,chan);

		aft_init_rx_dev_fifo(card,chan,WP_WAIT);
		aft_init_tx_dev_fifo(card,chan,WP_WAIT);

#ifdef AFT_DMA_HISTORY_DEBUG
		chan->dma_index=0;
		memset(chan->dma_history,0,sizeof(chan->dma_history));
#endif

		aft_reset_rx_chain_cnt(chan);
		aft_dma_rx(card,chan);

		aft_tslot_sync_ctrl(card,chan,1);

		if (!chan->hdlc_eng){
			aft_reset_tx_chain_cnt(chan);

			aft_dma_tx(card,chan);
			if (!chan->channelized_cfg) {
				/* Add 2 idle buffers into channel */
				aft_dma_tx(card,chan);
			}
		}
	}else{
		aft_dev_enable(card,chan);
	}

}

static void aft_dev_open(sdla_t *card, private_area_t *gchan)
{
	private_area_t *chan=gchan;

	if (chan->channelized_cfg || chan->wp_api_op_mode){

		for (chan=gchan; chan != NULL; chan=chan->next){

			aft_dev_open_private(card,chan);

			wan_set_bit(WP_DEV_CONFIG,&chan->up);

		}

		if (gchan->common.usedby == TDM_VOICE_API){
			/* Set the global mtu value which is
			 * the sum of all timeslots mtus */
			wan_netif_set_mtu(gchan->common.dev,
							  card->u.aft.tdmv_mtu+sizeof(wp_api_hdr_t));

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
		wan_set_bit(WP_DEV_CONFIG,&chan->up);
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

	if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
		/* BRI dchan does not use DMA thus
		   skip the dma config code below */
		return;
	}

	DEBUG_TEST("%s: Chan=%d\n",__FUNCTION__,chan->logic_ch_num);

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

	if (chan->channelized_cfg || chan->wp_api_op_mode){
		if (IS_BRI_CARD(card)) {
			int card_use_cnt;
			card->hw_iface.getcfg(card->hw, SDLA_HWTYPE_USEDCNT, &card_use_cnt);
			if (card_use_cnt == 1) {
				DEBUG_TEST("%s: BRI Disabling TDMV INTR\n",
					card->devname);
				aft_tdm_intr_ctrl(card,0);
				aft_fifo_intr_ctrl(card, 0);
			}
		} else {
			aft_tdm_intr_ctrl(card,0);
			aft_fifo_intr_ctrl(card, 0);
		}

		for (chan=gchan; chan != NULL; chan=chan->next){

			aft_dev_close_private(card,chan);

			DEBUG_TEST("%s: Closing Ch=%ld\n",
					chan->if_name,chan->logic_ch_num);

			wan_clear_bit(WP_DEV_CONFIG,&chan->up);
			wan_set_bit(0,&chan->interface_down);
			if (chan->cfg.tdmv_master_if){
				wan_clear_bit(0,&card->u.aft.tdmv_master_if_up);
			}

			DEBUG_TEST("%s: Closing Ch=%ld MasterUP=%d\n",
					chan->if_name,chan->logic_ch_num,
					wan_test_bit(0,&card->u.aft.tdmv_master_if_up));
		}
	}else{
		wan_set_bit(0,&chan->interface_down);
		wan_clear_bit(WP_DEV_CONFIG,&chan->up);
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
	int err=0;
	DEBUG_TEST("%s: Tx interrupt wdt=%d\n",chan->if_name,wdt);

	if (!wdt){
		wan_clear_bit(TX_INTR_PENDING,&chan->dma_chain_status);
	}

	if (chan->channelized_cfg && !chan->hdlc_eng){
		aft_tx_dma_voice_handler(chan,wdt,reset);
	}else{
		err=aft_tx_dma_chain_handler(chan,wdt,reset);
	}

	wan_set_bit(0,&chan->idle_start);

	if (reset){
		return;
	} 

	aft_dma_tx(card,chan);

#if 0
	if (WAN_NETIF_QUEUE_STOPPED(chan->common.dev)){
#else
	if (wan_chan_dev_stopped(chan)) {
#endif
		/* Wakeup stack also wakes up DCHAN,
  		   however, for DCHAN we must always
		   call the upper layer and let it decide
		   what to do */

		DEBUG_TEST("%s: STACK STOPPED Pending=%d Max=%d Max/2=%d\n",
				chan->if_name,wan_skb_queue_len(&chan->wp_tx_pending_list),
				chan->max_tx_bufs,chan->max_tx_bufs/2);
				

		if (wan_skb_queue_len(&chan->wp_tx_pending_list) <= (chan->max_tx_bufs/2)) {
			wanpipe_wake_stack(chan);
		}
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
			if (WAN_NET_RATELIMIT()){
               		DEBUG_EVENT("%s:%s: Error: TxDMA Length not equal 0 (reg=0x%08X)\n",
                   		card->devname,chan->if_name,reg);
			}
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
		WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//chan->if_stats.tx_errors++;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
		goto tx_post_exit;
	}

tx_post_ok:

	chan->opstats.Data_frames_Tx_count++;
	chan->opstats.Data_bytes_Tx_count+=wan_skb_len(skb);
	chan->chan_stats.tx_packets++;
	chan->chan_stats.tx_bytes+=wan_skb_len(skb);
	WAN_NETIF_STATS_INC_TX_PACKETS(&chan->common);	//chan->if_stats.tx_packets++;
	WAN_NETIF_STATS_INC_TX_BYTES(&chan->common, wan_skb_len(skb));	//chan->if_stats.tx_bytes+=wan_skb_len(skb);

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
	wan_smp_flag_t flags;

	rx_el=(wp_rx_element_t *)wan_skb_data(skb);

	DEBUG_RX("%s:%s: RX HI=0x%X  LO=0x%X DMA=0x%X",
				__FUNCTION__,
				chan->if_name,
				rx_el->reg,
				rx_el->align,
				rx_el->dma_addr);

#if 0
	/* debugging */
	WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//chan->if_stats.rx_errors++;
#endif

	rx_el->align&=AFT_RXDMA_LO_ALIGN_MASK;
	*pkt_error=0;
	*new_skb=NULL;

	dma_status=aft_rxdma_hi_get_dma_status(rx_el->reg);

    	/* Checking Rx DMA Go bit. Has to be '0' */
	if (wan_test_bit(AFT_RXDMA_HI_GO_BIT,&rx_el->reg)){
        	DEBUG_TEST("%s:%s: Error: RxDMA Intr: GO bit set on Rx intr\n",
				card->devname,chan->if_name);
		WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//chan->if_stats.rx_errors++;
		chan->errstats.Rx_dma_descr_err++;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_errors);
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
		WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//chan->if_stats.rx_errors++;
		card->wandev.stats.rx_errors++;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_errors);
		goto rx_comp_error;
	}

	if (chan->hdlc_eng){

		/* Checking Rx DMA Frame start bit. (information for api) */
		if (!wan_test_bit(AFT_RXDMA_HI_START_BIT,&rx_el->reg)){
			DEBUG_TEST("%s:%s RxDMA Intr: Start flag missing: MTU Mismatch! Reg=0x%X\n",
					card->devname,chan->if_name,rx_el->reg);
			WAN_NETIF_STATS_INC_RX_FRAME_ERRORS(&chan->common);	//chan->if_stats.rx_frame_errors++;
			chan->opstats.Rx_Data_discard_long_count++;
			chan->errstats.Rx_hdlc_corrupiton++;
			card->wandev.stats.rx_errors++;
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_frame_errors);
			goto rx_comp_error;
		}

		/* Checking Rx DMA Frame end bit. (information for api) */
		if (!wan_test_bit(AFT_RXDMA_HI_EOF_BIT,&rx_el->reg)){
			DEBUG_TEST("%s:%s: RxDMA Intr: End flag missing: MTU Mismatch! Reg=0x%X\n",
					card->devname,chan->if_name,rx_el->reg);
			WAN_NETIF_STATS_INC_RX_FRAME_ERRORS(&chan->common);	//chan->if_stats.rx_frame_errors++;
			chan->opstats.Rx_Data_discard_long_count++;
			chan->errstats.Rx_hdlc_corrupiton++;
			card->wandev.stats.rx_errors++;
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_frame_errors);
			goto rx_comp_error;

       	 	} else {  /* Check CRC error flag only if this is the end of Frame */

			if (wan_test_bit(AFT_RXDMA_HI_FCS_ERR_BIT,&rx_el->reg)){
                   		DEBUG_TEST("%s:%s: RxDMA Intr: CRC Error! Reg=0x%X Len=%d\n",
                                		card->devname,chan->if_name,rx_el->reg,
						(rx_el->reg&AFT_RXDMA_HI_DMA_LENGTH_MASK)>>2);
				WAN_NETIF_STATS_INC_RX_FRAME_ERRORS(&chan->common);	//chan->if_stats.rx_frame_errors++;
				chan->errstats.Rx_crc_err_count++;
				card->wandev.stats.rx_crc_errors++;
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_crc_errors);
				wan_set_bit(WP_CRC_ERROR_BIT,&rx_el->pkt_error);
				data_error = 1;
			}

			/* Check if this frame is an abort, if it is
                 	 * drop it and continue receiving */
			if (wan_test_bit(AFT_RXDMA_HI_FRM_ABORT_BIT,&rx_el->reg)){
				DEBUG_TEST("%s:%s: RxDMA Intr: Abort! Reg=0x%X\n",
						card->devname,chan->if_name,rx_el->reg);
				WAN_NETIF_STATS_INC_RX_FRAME_ERRORS(&chan->common);	//chan->if_stats.rx_frame_errors++;
				chan->errstats.Rx_hdlc_corrupiton++;
				card->wandev.stats.rx_frame_errors++;
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_frame_errors);
				wan_set_bit(WP_ABORT_ERROR_BIT,&rx_el->pkt_error);
				data_error = 1;
			}

			if (chan->common.usedby != API && data_error){
				goto rx_comp_error;
			}
		}
	}

	len = rx_el->len;

	*pkt_error=(u8)rx_el->pkt_error;

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

#if !defined(__WINDOWS__)
	/* Note for Windows: this will always be true */
	if (len > aft_rx_copyback){
		/* The rx size is big enough, thus
		 * send this buffer up the stack
		 * and allocate another one */
		memset(wan_skb_data(skb),0,sizeof(wp_rx_element_t));
#if defined(__FreeBSD__)
		wan_skb_trim(skb,sizeof(wp_rx_element_t));
#endif
		wan_skb_put(skb,len);
		wan_skb_pull(skb, sizeof(wp_rx_element_t));
		*new_skb=skb;

		aft_alloc_rx_dma_buff(card,chan,1,1);
	}else{
#endif
	
		/* The rx packet is very
		 * small thus, allocate a new
		 * buffer and pass it up */
		*new_skb=wan_skb_alloc(len + 20);
		if (!*new_skb){
			DEBUG_EVENT(
			"%s:%s: Failed to allocate rx skb pkt (len=%d)!\n",
					card->devname,chan->if_name,(len+20));
			WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common);	//chan->if_stats.rx_dropped++;
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
			goto rx_comp_error;
		}

		buf=wan_skb_put((*new_skb),len);
#if defined(__FreeBSD__)
		wan_skb_trim(skb,sizeof(wp_rx_element_t));
#endif
		memcpy(buf,wan_skb_tail(skb),len);

		wan_spin_lock_irq(&card->wandev.lock,&flags);
		aft_init_requeue_free_skb(chan, skb);
		wan_spin_unlock_irq(&card->wandev.lock,&flags);

#if !defined (__WINDOWS__)
	}
#endif

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

	wan_spin_lock_irq(&card->wandev.lock,&flags);
	aft_init_requeue_free_skb(chan, skb);
	wan_spin_unlock_irq(&card->wandev.lock,&flags);

    return;
}



/**SECTION**************************************************
 *
 * 	Logic Channel Registration Support and
 * 	Utility funcitons
 *
 **********************************************************/

/*============================================================================
 * Enable timer interrupt
 */
static void enable_timer (void* card_id)
{
	sdla_t*	card = (sdla_t*)card_id;
#if !defined(WAN_IS_TASKQ_SCHEDULE)
	wan_smp_flag_t	smp_flags;
	wan_smp_flag_t	smp_flags1;
	int		delay = 0;
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
	if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
		wan_set_bit(AFT_FE_POLL,&card->u.aft.port_task_cmd);
		WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
	}
#else
	card->hw_iface.hw_lock(card->hw,&smp_flags1);
	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);
	if (card->wandev.fe_iface.polling){
		delay = card->wandev.fe_iface.polling(&card->fe);
	}
	wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
	if (delay){
		card->wandev.fe_iface.add_timer(&card->fe, delay);
	}
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

	DEBUG_HWEC("%s(): %s Sdla EC Polling !\n",__FUNCTION__, card->devname);

# if defined(WAN_IS_TASKQ_SCHEDULE)
	if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
		wan_set_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd);
		WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
	}
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

#if defined(__WINDOWS__)
static void wp_bh_rx(private_area_t* chan, netskb_t *new_skb, u8 pkt_error, int len)
{
	sdla_t *card = chan->card;

	if (chan->hdlc_eng){
		if (card->u.aft.cfg.rx_crc_bytes == 3){
			wan_skb_put(new_skb,3);
		}else if (card->u.aft.cfg.rx_crc_bytes == 2){
			wan_skb_put(new_skb,2); 
		}
	}

	if(chan->common.usedby == WANPIPE || chan->common.usedby == STACK){

		rx_dpc(chan->common.dev, new_skb, pkt_error, len);
	}else{

		int err;
		wp_api_hdr_t *rx_hdr=NULL;



		if (wan_skb_headroom(new_skb) >= sizeof(wp_api_hdr_t)){
			rx_hdr=(wp_api_hdr_t*)skb_push(new_skb,sizeof(wp_api_hdr_t));
			memset(rx_hdr,0,sizeof(wp_api_hdr_t));
			
		}else{
			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: Error Rx pkt headroom %u < %u\n",
					chan->if_name,
					(u32)wan_skb_headroom(new_skb),
					(u32)sizeof(wp_api_hdr_t));
			}
			WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
			wan_skb_free(new_skb);
			return;
		}

		err=wanpipe_tdm_api_is_rbsbits(card);
		if (err == 1) {
			if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
				wan_set_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
				WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
			}
		}

		rx_hdr->rx_h.crc=pkt_error;
		rx_hdr->rx_h.current_number_of_frames_in_rx_queue = (u8)wan_skb_queue_len(&chan->wp_rx_complete_list);
		rx_hdr->rx_h.max_rx_queue_length = (u8)chan->dma_per_ch;

		err=wanpipe_tdm_api_span_rx(&chan->wp_tdm_api_dev, new_skb);
		if (err) {
			WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
			wan_skb_free(new_skb);
			return;
		}


	}

	chan->opstats.Data_frames_Rx_count++;
	chan->opstats.Data_bytes_Rx_count+=len;
	chan->chan_stats.rx_packets++;
	chan->chan_stats.rx_bytes+=len;
	WAN_NETIF_STATS_INC_RX_PACKETS(&chan->common);		/* chan->if_stats.rx_packets++; */
	WAN_NETIF_STATS_INC_RX_BYTES(&chan->common, len);	/* chan->if_stats.rx_bytes+=len; */
}
#else
static void wp_bh_rx(private_area_t* chan, netskb_t *new_skb, u8 pkt_error, int len)
{
	sdla_t *card = chan->card;

	if (chan->common.usedby == API){

		if (chan->wp_api_op_mode) {
			int err;
			wp_api_hdr_t *rx_hdr=NULL;

			if (wan_skb_headroom(new_skb) >= sizeof(wp_api_hdr_t)){
				rx_hdr=(wp_api_hdr_t*)skb_push(new_skb,sizeof(wp_api_hdr_t));
				memset(rx_hdr,0,sizeof(wp_api_hdr_t));
				
			}else{
				if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: Error Rx pkt headroom %u < %u\n",
						chan->if_name,
						(u32)wan_skb_headroom(new_skb),
						(u32)sizeof(wp_api_hdr_t));
				}
				WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
				wan_skb_free(new_skb);
				return;
			}

			err=wanpipe_tdm_api_is_rbsbits(card);
			if (err == 1) {
				if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
					wan_set_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
					WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
				}
			}

			rx_hdr->rx_h.crc=pkt_error;
			rx_hdr->rx_h.current_number_of_frames_in_rx_queue = wan_skb_queue_len(&chan->wp_rx_complete_list);
			rx_hdr->rx_h.max_rx_queue_length = chan->dma_per_ch;

			err=wanpipe_tdm_api_span_rx(&chan->wp_tdm_api_dev, new_skb);
			if (err) {
				WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
				wan_skb_free(new_skb);
				return;
			}



		} else {

			if (chan->hdlc_eng){
				if (card->u.aft.cfg.rx_crc_bytes == 3){
					wan_skb_put(new_skb,3);
				}else if (card->u.aft.cfg.rx_crc_bytes == 2){
					wan_skb_put(new_skb,2);
				}
			}
	
			if (chan->common.sk == NULL){
				DEBUG_TEST("%s: No sock bound to channel rx dropping!\n",
					chan->if_name);
	
				WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
				wan_skb_free(new_skb);
				return;
			}

#if defined(__LINUX__)
# ifndef CONFIG_PRODUCT_WANPIPE_GENERIC
	
			/* Only for API, we insert packet status
			* byte to indicate a packet error. Take
			* this byte and put it in the api header */
	
			if (wan_skb_headroom(new_skb) >= sizeof(wp_api_hdr_t)){
				wp_api_hdr_t *rx_hdr=
					(wp_api_hdr_t*)skb_push(new_skb,sizeof(wp_api_hdr_t));
				memset(rx_hdr,0,sizeof(wp_api_hdr_t));
				rx_hdr->wp_api_rx_hdr_error_flag=pkt_error;
			}else{
				if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: Error Rx pkt headroom %u < %u\n",
						chan->if_name,
						(u32)wan_skb_headroom(new_skb),
						(u32)sizeof(wp_api_hdr_t));
				}
				WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
				wan_skb_free(new_skb);
				return;
			}
	
			new_skb->protocol = htons(PVC_PROT);
			wan_skb_reset_mac_header(new_skb);
			new_skb->dev	  = chan->common.dev;
			new_skb->pkt_type = WAN_PACKET_DATA;

#if 0
			WAN_NETIF_STATS_INC_RX_FRAME_ERRORS(&chan->common); /* chan->if_stats.rx_frame_errors++; */
#endif
	
			if (wan_api_rx(chan,new_skb) != 0){
				WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
				wan_skb_free(new_skb);
				return;
			}

# endif
#endif
		}

	}else if (chan->common.usedby == TDM_VOICE_DCHAN){

#ifdef AFT_TDM_API_SUPPORT
		if (is_tdm_api(chan,&chan->wp_tdm_api_dev)) {
			int err;
			wp_api_hdr_t *rx_hdr=NULL;

			DEBUG_RX("%s: RX TDM API DCHAN %d\n",chan->if_name, wan_skb_len(new_skb));

			if (wan_skb_headroom(new_skb) >= sizeof(wp_api_hdr_t)){
				rx_hdr =(wp_api_hdr_t*)skb_push(new_skb,sizeof(wp_api_hdr_t));
				memset(rx_hdr,0,sizeof(wp_api_hdr_t));
			}else{
				if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: Error Rx pkt headroom %u < %u\n",
						chan->if_name,
						(u32)wan_skb_headroom(new_skb),
						(u32)sizeof(wp_api_hdr_t));
				}
				WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
				wan_skb_free(new_skb);
				return;
			}

			rx_hdr->rx_h.crc=pkt_error;
			rx_hdr->rx_h.current_number_of_frames_in_rx_queue = wan_skb_queue_len(&chan->wp_rx_complete_list);
			rx_hdr->rx_h.max_rx_queue_length = chan->dma_per_ch;

			err=wanpipe_tdm_api_span_rx(&chan->wp_tdm_api_dev,new_skb);
			if (err){
				WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
				wan_skb_free(new_skb);
				return;
			}

		}else
#endif
			if (chan->tdmv_zaptel_cfg){

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN) && defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
				int	err;


				/* ADEBUG */
				WAN_TDMV_CALL(rx_dchan,
					(&card->wan_tdmv,chan->tdmv_chan,
					wan_skb_data(new_skb),wan_skb_len(new_skb)),
					err);
				DEBUG_RX("%s TDM DCHAN VOICE Rx Pkt Len=%d Chan=%d\n",
					card->devname,wan_skb_len(new_skb),
					chan->tdmv_chan);
#else
				DEBUG_EVENT("%s: DCHAN Rx Packet critical error TDMV not compiled!\n",card->devname);
#endif

				wan_skb_free(new_skb);
				/* Continue through since the above
				* function returns void */

			} else {
				if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: DCHAN Rx Packet critical error op not supported\n",card->devname);
				}
				WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
				wan_skb_free(new_skb);
				return;
			}

	}else if (chan->common.usedby == TDM_VOICE){

		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s: TDM VOICE CRITICAL: IN BH!!!!\n",card->devname);
		}
		WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
		wan_skb_free(new_skb);
		return;

	}else if (chan->common.usedby == STACK){

#if defined(__WINDOWS__)		
		FUNC_NOT_IMPL();
#else
		wan_skb_set_csum(new_skb,0);

		if (wanpipe_lip_rx(chan,new_skb) != 0){
			WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
			wan_skb_free(new_skb);
			return;
		}
#endif		

#if defined(AFT_XMTP2_API_SUPPORT)
	}else if (chan->common.usedby == XMTP2_API){
		netskb_t *tskb;

		wan_skb_set_csum(new_skb,0);

		if (chan->xmtp2_api_index < 0) {
			if (WAN_NET_RATELIMIT()) {
				DEBUG_EVENT("%s: Error: MTP2 Link not configured!\n",
						chan->if_name);
			}
			WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common); /* ++chan->if_stats.rx_dropped; */
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
			wan_skb_free(new_skb);
			return;
		}

		tskb=wan_skb_dequeue(&chan->wp_rx_free_list);
		if (!tskb) {
			WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_errors);
			wan_skb_free(new_skb);
			return;
		}

		wan_skb_put(tskb,wan_skb_len(new_skb));

		xmtp2km_bs_handler (chan->xmtp2_api_index,
                            		wan_skb_len(new_skb), wan_skb_data(new_skb), wan_skb_data(tskb));

		wan_skb_free(new_skb);

		wan_skb_queue_tail(&chan->wp_tx_pending_list,tskb);

		aft_dma_tx(card,chan);

#endif

	}else if (chan->common.usedby == ANNEXG){

#ifdef CONFIG_PRODUCT_WANPIPE_ANNEXG
		if (chan->annexg_dev){
			new_skb->protocol = htons(ETH_P_X25);
			new_skb->dev = chan->annexg_dev;
			wan_skb_reset_mac_header(new_skb);

			if (IS_FUNC_CALL(lapb_protocol,lapb_rx)){
				lapb_protocol.lapb_rx(chan->annexg_dev,new_skb);
				card->wandev.stats.rx_packets++;
				card->wandev.stats.rx_bytes += len;
			}else{
				wan_skb_free(new_skb);
				WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common);
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
			}
		}else
#endif
		{
			wan_skb_free(new_skb);
			WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common);
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
		}

	}else{
		protocol_recv(chan->card,chan,new_skb);
	}

	chan->opstats.Data_frames_Rx_count++;
	chan->opstats.Data_bytes_Rx_count+=len;
	chan->chan_stats.rx_packets++;
	chan->chan_stats.rx_bytes+=len;
	WAN_NETIF_STATS_INC_RX_PACKETS(&chan->common);		/* chan->if_stats.rx_packets++; */
	WAN_NETIF_STATS_INC_RX_BYTES(&chan->common, len);	/* chan->if_stats.rx_bytes+=len; */
	return;
}
#endif

#if defined(__LINUX__)
static void wp_bh (unsigned long data)
#elif defined(__WINDOWS__)
static void wp_bh (IN PKDPC Dpc, IN PVOID data, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
#else
static void wp_bh (void *data, int pending)
#endif
{
	private_area_t	*chan = (private_area_t *)data;
	sdla_t		*card=chan->card;
	netskb_t	*new_skb,*skb;
	unsigned char	pkt_error;
	wan_ticks_t	timeout=SYSTEM_TICKS;
	private_area_t	*top_chan;
	int		len;
	wan_smp_flag_t flags;

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

	DEBUG_TEST("%s: ------------ BEGIN --------------: %ld\n",
			__FUNCTION__,(u_int64_t)SYSTEM_TICKS);

	if (!wan_test_bit(WP_DEV_CONFIG,&chan->up)){
		if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s: wp_bh() chan not up!\n",
                                chan->if_name);
		}
		WAN_TASKLET_END((&chan->common.bh_task));
		return;
	}

	do {
		if (SYSTEM_TICKS-timeout > 2){
#if 0
			DEBUG_EVENT("%s: BH Squeeze - breaking out of wp_bh loop!\n",chan->if_name);
#if defined(__WINDOWS__)
			DEBUG_TEST("%s: BH Squeeze - breaking out of wp_bh loop!\n",chan->if_name);
#endif
#endif
			break;
		}

		wan_spin_lock_irq(&card->wandev.lock,&flags);
		skb=wan_skb_dequeue(&chan->wp_rx_complete_list);
		wan_spin_unlock_irq(&card->wandev.lock,&flags);
		if (!skb) {
			break;
		}

#if 0
		WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//chan->if_stats.rx_errors++;
#endif

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
					WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//++chan->if_stats.rx_errors;
					WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_errors);
					wan_skb_free(new_skb);
					continue;
				}

				wan_skb_trim(new_skb,wan_skb_len(new_skb)-3);
				len-=3;
			}

			wan_capture_trace_packet(chan->card, &top_chan->trace_info,
						 new_skb,TRC_INCOMING_FRM);

			wp_bh_rx(chan, new_skb, pkt_error, len);
		}else{
			FUNC_NEWDRV();
		}
	}while(skb);

	do {
		wan_spin_lock_irq(&card->wandev.lock,&flags);
		skb=wan_skb_dequeue(&chan->wp_rx_stack_complete_list);
		wan_spin_unlock_irq(&card->wandev.lock,&flags);
		if (!skb) {
			break;
		}

		len=wan_skb_len(skb);
		wan_capture_trace_packet(chan->card, &top_chan->trace_info,
                                             skb,TRC_INCOMING_FRM);
#if defined(__WINDOWS__)		
		FUNC_NOT_IMPL();
#else
		if (wanpipe_lip_rx(chan,skb) != 0){
			WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common);	//++chan->if_stats.rx_dropped;
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
			wan_skb_free(skb);
		}else{
			chan->opstats.Data_frames_Rx_count++;
			chan->opstats.Data_bytes_Rx_count+=len;
			chan->chan_stats.rx_packets++;
			chan->chan_stats.rx_bytes+=len;
			WAN_NETIF_STATS_INC_RX_PACKETS(&chan->common);	//chan->if_stats.rx_packets++;
			WAN_NETIF_STATS_INC_RX_BYTES(&chan->common,len);	//chan->if_stats.rx_bytes+=len;
		}
#endif
	}while(skb);

	do {
		wan_spin_lock_irq(&card->wandev.lock,&flags);
		skb=wan_skb_dequeue(&chan->wp_rx_bri_dchan_complete_list);
		wan_spin_unlock_irq(&card->wandev.lock,&flags);
		if (!skb) {
			break;
		}
		/* for BRI the rx data on D-chan is in 'wp_rx_bri_dchan_complete_list'. */
		wan_capture_trace_packet(chan->card, 
								 &top_chan->trace_info,
                                 skb,TRC_INCOMING_FRM);

		wp_bh_rx(chan, skb, 0, wan_skb_len(skb));
	}while(skb);

	/* This case is only used for HDLC DMA Chain mode
  	   Not for transparent */
	do {
		wan_spin_lock_irq(&card->wandev.lock,&flags);
		skb=wan_skb_dequeue(&chan->wp_tx_complete_list);
		wan_spin_unlock_irq(&card->wandev.lock,&flags);
		if (!skb) {
			break;
		}
		aft_tx_post_complete (chan->card,chan,skb);

#ifdef __WINDOWS__
		wan_capture_trace_packet(card, &chan->trace_info, skb, TRC_OUTGOING_FRM);
#endif
		wan_skb_free(skb);
	}while(skb);

	/* This case is only used for HDLC DMA Chain mode
  	   Not for transparent */
	do {
		wan_spin_lock_irq(&card->wandev.lock,&flags);
		skb=wan_skb_dequeue(&chan->wp_dealloc_list);
		wan_spin_unlock_irq(&card->wandev.lock,&flags);
		if (!skb) {
			break;
		}
		wan_skb_free(skb);
	}while(skb);

	WAN_TASKLET_END((&chan->common.bh_task));
	/* FIXME: If wanpipe goes down, do not schedule again */
	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		return;
	}

#if 1
	if ((len=wan_skb_queue_len(&chan->wp_rx_complete_list))){
		WAN_TASKLET_SCHEDULE((&chan->common.bh_task));
	}else if ((len=wan_skb_queue_len(&chan->wp_tx_complete_list))){
		WAN_TASKLET_SCHEDULE((&chan->common.bh_task));
        }else if ((len=wan_skb_queue_len(&chan->wp_rx_stack_complete_list))){
		WAN_TASKLET_SCHEDULE((&chan->common.bh_task));
	}else if ((len=wan_skb_queue_len(&chan->wp_rx_bri_dchan_complete_list))){
		WAN_TASKLET_SCHEDULE((&chan->common.bh_task));
	}
#endif

	DEBUG_TEST("%s: ------------ END -----------------: %ld\n",
                        __FUNCTION__,(u_int64_t)SYSTEM_TICKS);
	return;
}


/**SECTION***************************************************************
 *
 * 	HARDWARE Interrupt Handlers
 *
 ***********************************************************************/

static void __wp_aft_fifo_per_port_isr(sdla_t *card, u32 rx_status, u32 tx_status)
{
	int num_of_logic_ch;
	u32 tmp_fifo_reg;
	private_area_t *chan, *top_chan=NULL;
	int i,fifo=0;

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
				if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
					continue;
				}

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
				aft_critical_trigger(card);
				break;

#endif

}
#endif

				top_chan=wan_netif_priv(chan->common.dev);
				aft_tx_fifo_under_recover(card,chan);
				fifo++;

				WAN_NETIF_STATS_INC_TX_FIFO_ERRORS(&chan->common);	//++chan->if_stats.tx_fifo_errors;
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_fifo_errors);
				card->wandev.stats.tx_aborted_errors++;
        		__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG),&tmp_fifo_reg);
			}
		}

		if (fifo) {
			if (card->u.aft.global_tdm_irq) {
				if (top_chan) {
					WAN_NETIF_STATS_INC_TX_FIFO_ERRORS(&top_chan->common);	//++chan->if_stats.tx_fifo_errors;
				}
			}
		}
	}

	fifo=0;


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

				if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
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
				fifo++;
				top_chan=wan_netif_priv(chan->common.dev);

				WAN_NETIF_STATS_INC_RX_FIFO_ERRORS(&chan->common);	//++chan->if_stats.rx_fifo_errors;
				WAN_NETIF_STATS_INC_RX_OVER_ERRORS(&chan->common);	//++chan->if_stats.rx_over_errors;
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_fifo_errors);
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_over_errors);
				chan->errstats.Rx_overrun_err_count++;
				card->wandev.stats.rx_over_errors++;

				aft_rx_fifo_over_recover(card,chan);
				wan_set_bit(WP_FIFO_ERROR_BIT, &chan->pkt_error);

        		__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG),&tmp_fifo_reg);
#if 0
				/* Debuging Code used to stop the line in
				 * case of fifo errors */
				aft_list_descriptors(chan);

				aft_critical_trigger(card);
#endif
			}
		}


		if (fifo) {
			if (card->u.aft.global_tdm_irq) {
				if (top_chan) {
					WAN_NETIF_STATS_INC_RX_FIFO_ERRORS(&top_chan->common);	//++chan->if_stats.rx_fifo_errors;
					WAN_NETIF_STATS_INC_RX_OVER_ERRORS(&top_chan->common);	//++chan->if_stats.rx_over_errors;
				}
			}
		}
	}

	return;
}

static void wp_aft_fifo_per_port_isr(sdla_t *card)
{
	u32 rx_status=0, tx_status=0;
	u32 i;

	/* Clear HDLC pending registers */
    __sdla_bus_read_4(card->hw, AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG),&tx_status);
	__sdla_bus_read_4(card->hw, AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG),&rx_status);


	if (IS_BRI_CARD(card)) {
		void **card_list=__sdla_get_ptr_isr_array(card->hw);
		sdla_t *tmp_card;
		for (i=0;i<MAX_BRI_LINES;i++) {
			tmp_card=(sdla_t*)card_list[i];
			if (tmp_card &&
			    !wan_test_bit(CARD_DOWN,&tmp_card->wandev.critical)) {
				__wp_aft_fifo_per_port_isr(tmp_card,rx_status,tx_status);
			}
		}
	} else {
		__wp_aft_fifo_per_port_isr(card,rx_status,tx_status);
	}

	return;
}


static int aft_is_first_card_in_list(sdla_t *card) 
{
    void **card_list;
	u32 max_number_of_ports, i;
	sdla_t	*tmp_card;


	if (IS_BRI_CARD(card)) {
		max_number_of_ports = MAX_BRI_LINES; /* 24 */
	} else {
		max_number_of_ports = 8; /* 24 */
	}
	
	card_list=__sdla_get_ptr_isr_array(card->hw);

	DEBUG_TEST("%s(): card_list ptr: 0x%p\n", __FUNCTION__, card_list);

	for (i=0; i<max_number_of_ports; i++) {
     	tmp_card=(sdla_t*)card_list[i];
		if (tmp_card == NULL || wan_test_bit(CARD_DOWN,&tmp_card->wandev.critical)) {
			continue;
		}

		/* check if first valid card in the list matches give card ptr */
		if (tmp_card == card) {
         	return 0;
		}

		return 1;
	}

	return 1;
}

static void front_end_interrupt(sdla_t *card, unsigned long reg, int lock)
{
	void **card_list;
	u32 max_number_of_ports, i;
	sdla_t	*tmp_card;

	if(IS_FXOFXS_CARD(card)){
		DEBUG_EVENT("%s(): %s: Error: Front End Interrupt on Analog card! Must never happen!\n", __FUNCTION__, card->devname);
		return;
	}

	if (IS_BRI_CARD(card)) {
		max_number_of_ports = MAX_BRI_LINES; /* 24 */
	} else {
		max_number_of_ports = 8; /* 24 */
	}

	card_list=__sdla_get_ptr_isr_array(card->hw);

	DEBUG_TEST("%s(): card_list ptr: 0x%p\n", __FUNCTION__, card_list);

	for (i=0; i<max_number_of_ports; i++) {

		tmp_card=(sdla_t*)card_list[i];
		if (tmp_card == NULL || wan_test_bit(CARD_DOWN,&tmp_card->wandev.critical)) {
			continue;
		}

		DEBUG_TEST("%s(): card ptr: %s, tmp_card ptr: %s\n", __FUNCTION__, card->devname, tmp_card->devname);

		if (tmp_card->wandev.fe_iface.check_isr &&
		    tmp_card->wandev.fe_iface.check_isr(&tmp_card->fe)) {

			if (tmp_card->wandev.fe_iface.isr &&
				tmp_card->wandev.fe_iface.isr(&tmp_card->fe)) {

				handle_front_end_state(tmp_card,lock);
			}
		}
	}

	return;
}



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

#define EC_IRQ_TIMEOUT (HZ/32)

int dydbg_tmp_cnt = 0;

static WAN_IRQ_RETVAL wp_aft_global_isr (sdla_t* card)
{
	u32 reg_sec=0,reg=0;
	u32 a108_reg=0, a56k_reg=0, serial_reg=0;
	u32 fifo_port_intr=0;
	u32 dma_port_intr=0;
	u32 wdt_port_intr=0;
	u32 tdmv_port_intr=0;
	u32 status_port_intr=0;
	u32 fe_intr=0;
	u32 max_ports=IS_BRI_CARD(card)?MAX_BRI_LINES:8;

	WAN_IRQ_RETVAL_DECL(irq_ret);

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		DEBUG_TEST("%s: Card down, ignoring interrupt!!!!!!!\n",
			card->devname);
		WAN_IRQ_RETURN(irq_ret);
	}

#ifdef AFT_IRQ_STAT_DEBUG
	card->wandev.stats.rx_errors++;
#endif

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
	*  -- "FiFo/Line Abort Error"		 interrupt disable
	* --------------------------------------------------*/
	__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG), &reg);
	reg_sec=reg;

	DEBUG_ISR("%s:ISR = 0x%X\n",card->devname,reg);
	
	if (wan_test_bit(AFT_CHIPCFG_FE_INTR_STAT_BIT,&reg)){

#ifdef AFT_IRQ_STAT_DEBUG
		card->wandev.stats.rx_dropped++;
#endif

		if (wan_test_bit(AFT_CHIPCFG_FE_INTR_CFG_BIT,&reg)) {

			DEBUG_ISR("%s: Got Front End Interrupt 0x%08X\n",
					card->devname,reg);

			WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);

#ifdef AFT_IRQ_STAT_DEBUG
			card->wandev.stats.tx_dropped++;
#endif
			fe_intr=1;

#if defined (__LINUX__) || defined(__FreeBSD__) || defined(__WINDOWS__)
			if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
				/* Only execute front end interrupt on very first card
				   In the list. This way we do not get into a race condition
				   between port task on wanpipe1 versus trigger from wanpipe2.
				   Since Front end interrupt is global interrupt per card */
				if (aft_is_first_card_in_list(card) == 0) {  
					
					wan_set_bit(AFT_FE_INTR,&card->u.aft.port_task_cmd);
					WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
					
					__aft_fe_intr_ctrl(card,0);
					
					/* NC: Bug fix we must update the reg value */
					__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG), &reg);
				}
			}
#else
#error "Undefined OS- Front End Interrupt not allowed in IRQ!"
#if 0
			if (card->wandev.fe_iface.check_isr &&
				card->wandev.fe_iface.check_isr(&card->fe)){
				front_end_interrupt(card,reg,0);
			}
#endif
#endif

		}/* if (wan_test_bit(AFT_CHIPCFG_FE_INTR_CFG_BIT,&reg)) */

	}/* if (wan_test_bit(AFT_CHIPCFG_FE_INTR_STAT_BIT,&reg)) */


/* New Octasic implementarion May 16 2006 */
#if defined(CONFIG_WANPIPE_HWEC)
	if (card->wandev.ec_dev && wanpipe_ec_isr(card->wandev.ec_dev)){
		if (!wan_test_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd)){
			/* All work is done from ec_poll routine!!! */
			if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
				wan_set_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd);
				WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
			}
		}
	}
#endif

	if (card->u.aft.firm_id == AFT_DS_FE_CORE_ID || IS_BRI_CARD(card) || IS_A600_CARD(card) || IS_A700_CARD(card)) {

		__sdla_bus_read_4(card->hw,AFT_PORT_REG(card, AFT_CHIP_STAT_REG), &a108_reg);

		fifo_port_intr	= aft_chipcfg_a108_get_fifo_intr_stats(a108_reg);
		dma_port_intr	= aft_chipcfg_a108_get_dma_intr_stats(a108_reg);
		wdt_port_intr	= aft_chipcfg_a108_get_wdt_intr_stats(a108_reg);
		tdmv_port_intr	= aft_chipcfg_a108_get_tdmv_intr_stats(a108_reg);

		if (!IS_A700_CARD(card)) {
			if (tdmv_port_intr) {
				tdmv_port_intr=1;
			}
		}

	} else if (IS_SERIAL_CARD(card)) {
		__sdla_bus_read_4(card->hw,AFT_PORT_REG(card, AFT_CHIP_STAT_REG), &serial_reg);

#ifdef AFT_SERIAL_DEBUG
if (serial_reg) {
	DEBUG_EVENT("%s: SERIAL ISR = 0x%08X\n",
			card->devname,serial_reg);
}
#endif

		fifo_port_intr	= aft_chipcfg_a108_get_fifo_intr_stats(serial_reg);
		dma_port_intr	= aft_chipcfg_a108_get_dma_intr_stats(serial_reg);
		wdt_port_intr	= aft_chipcfg_serial_get_wdt_intr_stats(serial_reg);
		status_port_intr = aft_chipcfg_serial_get_status_intr_stats(serial_reg);

	} else if(IS_56K_CARD(card)) {
       		__sdla_bus_read_4(card->hw,AFT_PORT_REG(card, AFT_CHIP_STAT_REG), &a56k_reg);
		fifo_port_intr	= wan_test_bit(AFT_CHIPCFG_A56K_FIFO_INTR_BIT,&a56k_reg);
		dma_port_intr	= wan_test_bit(AFT_CHIPCFG_A56K_DMA_INTR_BIT,&a56k_reg);
		wdt_port_intr	= wan_test_bit(AFT_CHIPCFG_A56K_WDT_INTR_BIT,&a56k_reg);
	} else {
		fifo_port_intr	= aft_chipcfg_get_hdlc_intr_stats(reg);
		dma_port_intr	= aft_chipcfg_get_dma_intr_stats(reg);
		wdt_port_intr	= aft_chipcfg_get_wdt_intr_stats(reg);
		tdmv_port_intr	= aft_chipcfg_get_tdmv_intr_stats(reg);

		if (wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
			tdmv_port_intr&=0x01;
		}
	}

#if 0
if (0){
static int gcnt=0;
	if (gcnt++ < 100) {
	DEBUG_EVENT("%s: ISR: REG=0x%08X TDM=0x%08X DMA=0x%08X FIFO=0x%08X STAT=0x%08X WDT=0x%08X\n",
			card->devname,reg_sec, tdmv_port_intr,dma_port_intr,fifo_port_intr,
			status_port_intr,wdt_port_intr);
	}
}
#endif


	if (tdmv_port_intr ||
		dma_port_intr  ||
		fifo_port_intr ||
		status_port_intr ||
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
			card->u.aft.global_tdm_irq &&
		    !wan_test_bit(aft_chipcfg_get_fifo_reset_bit(card),&reg)) {

			int ring_buf_enabled=wan_test_bit(AFT_CHIPCFG_A108_A104_TDM_DMA_RINGBUF_BIT,&reg);
			sdla_t	*tmp_card;
			int ring_rsync=0;
			void **card_list;
			u32 i;
	
			WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);

#ifdef AFT_IRQ_STAT_DEBUG
			card->wandev.stats.rx_crc_errors++;
#endif
			if (ring_buf_enabled) {
				/* NC: Bug fix we must read before writting, reg might have changed above */
				__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG), &reg);
				if (card->adptr_type == A104_ADPTR_4TE1 &&
					card->u.aft.firm_id == AFT_PMC_FE_CORE_ID) {
					wan_set_bit(AFT_CHIPCFG_A104_TDM_ACK_BIT,&reg);
				} else {
					for (i=0;i<2;i++) {
						if (wan_test_bit(i,&tdmv_port_intr)) {
							wan_set_bit(aft_chipcfg_get_rx_intr_ack_bit_bymap(i),&reg);
							wan_set_bit(aft_chipcfg_get_tx_intr_ack_bit_bymap(i),&reg);
						}
					}
				}
				__sdla_bus_write_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG),reg);
	
#if 0
				if (IS_A700_ANALOG_CARD(card) && card->hw_iface.fe_test_bit(card->hw,2)) {
						DEBUG_EVENT("%s: Global TDM Ring Resync\n",card->devname);
								ring_rsync=1;
						card->hw_iface.fe_clear_bit(card->hw,2);
				} else 	if (card->hw_iface.fe_test_bit(card->hw,1)) {
						DEBUG_EVENT("%s: Global TDM Ring Resync\n",card->devname);
								ring_rsync=1;
						card->hw_iface.fe_clear_bit(card->hw,1);
				}
#endif
#if 1
				if (card->hw_iface.fe_test_bit(card->hw,1)) {
						DEBUG_EVENT("%s: Global TDM Ring Resync TDM = 0x%X\n",
												card->devname,tdmv_port_intr);
						ring_rsync=1;
				}
#endif
			}

			card_list=__sdla_get_ptr_isr_array(card->hw);

			DEBUG_TEST("%s: TDM IRQ MAP=0x%X\n", card->devname, tdmv_port_intr);

			//FIXME: Use value pre card type
			for (i=0;i<max_ports;i++) { /* for TE1 maximum is 8, but for BRI is 12*/
				tmp_card=(sdla_t*)card_list[i];
	
				if (!tmp_card ||
					!tmp_card->u.aft.global_tdm_irq || 
					!wan_test_bit(AFT_LCFG_TDMV_INTR_BIT,&tmp_card->u.aft.lcfg_reg)) {
					continue;
				}

#ifdef AFT_IRQ_STAT_DEBUG
				tmp_card->wandev.stats.rx_crc_errors++;
#endif

				if (IS_A700_CARD(tmp_card)) {
					if (!wan_test_bit(tmp_card->wandev.comm_port,&tdmv_port_intr)) {
						continue;
					}
				}

				if (ring_buf_enabled) {
	
					if (ring_rsync) {
						aft_tdm_ring_rsync(tmp_card);
					} else {
#if 0
						for (i=0; i<card->u.aft.num_of_time_slots;i++){
	
							if (!wan_test_bit(i,&card->u.aft.tdm_logic_ch_map)){
								continue;
							}
	
							tmp_card->u.aft.tdm_rx_dma_toggle[i]++;
							if (tmp_card->u.aft.tdm_rx_dma_toggle[i] >= AFT_TDMV_CIRC_BUF_LEN) {
								tmp_card->u.aft.tdm_rx_dma_toggle[i]=0;
							}
	
							tmp_card->u.aft.tdm_tx_dma_toggle[i]++;
							if (tmp_card->u.aft.tdm_tx_dma_toggle[i] >= AFT_TDMV_CIRC_BUF_LEN) {
								tmp_card->u.aft.tdm_tx_dma_toggle[i]=0;
							}
					}
#endif
					}
				}
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
				if (tmp_card->wan_tdmv.sc &&
					wan_test_bit(WP_ZAPTEL_DCHAN_OPTIMIZATION,&card->u.aft.tdmv_zaptel_cfg) &&
						!card->wandev.rtp_len &&
						card->wandev.config_id != WANCONFIG_AFT_ANALOG) {
	
					aft_voice_span_rx_tx(tmp_card,
									ring_buf_enabled);
				}else
#endif
				{
					wp_aft_tdmv_per_port_isr(tmp_card);
				}
			}

			if (!ring_buf_enabled) {
				/* NC: Bug fix we must read before writting, reg might have changed above */
				__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG), &reg);
				if (card->adptr_type == A104_ADPTR_4TE1 &&
					card->u.aft.firm_id == AFT_PMC_FE_CORE_ID) {
					wan_set_bit(AFT_CHIPCFG_A104_TDM_ACK_BIT,&reg);
				} else {
					for (i=0;i<2;i++) {
						if (wan_test_bit(i,&tdmv_port_intr)) {
							wan_set_bit(aft_chipcfg_get_rx_intr_ack_bit_bymap(i),&reg);
							wan_set_bit(aft_chipcfg_get_tx_intr_ack_bit_bymap(i),&reg);
						}
					}
				}
				__sdla_bus_write_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG),reg);
			}

			if (ring_rsync) {
				card->hw_iface.fe_clear_bit(card->hw,1);
				ring_rsync=0;
			}

		} else if (tdmv_port_intr &&
		    	   wan_test_bit(aft_chipcfg_get_fifo_reset_bit(card),&reg)) {
			DEBUG_EVENT("%s: TDM GLOBAL ISR - Skipped due to RESET BIT SET!\n",card->devname);
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
 			    wan_test_bit(WP_ZAPTEL_DCHAN_OPTIMIZATION,&card->u.aft.tdmv_zaptel_cfg) &&
			    !card->wandev.rtp_len &&
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

	if (status_port_intr) {
		wp_aft_serial_status_isr(card, status_port_intr);
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
	 *	  Finish of the interupt handler
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

			aft_critical_trigger(card);
		}

	} else if (aft_hwdev[card->wandev.card_type].aft_check_ec_security(card)){
		WAN_IRQ_RETVAL_SET(irq_ret, WAN_IRQ_HANDLED);
		if (++card->u.aft.chip_security_cnt > AFT_MAX_CHIP_SECURITY_CNT){
			DEBUG_EVENT("%s: Critical: Echo Canceller Chip Security Compromised: Disabling Driver!\n",
				card->devname);
			DEBUG_EVENT("%s: Please call Sangoma Tech Support (www.sangoma.com)!\n",
				card->devname);

			card->u.aft.chip_security_cnt=0;
			aft_critical_trigger(card);
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

			aft_critical_trigger(card);
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



static void wp_aft_serial_status_isr(sdla_t *card, u32 serial_intr_status)
{
 	u32 reg;

	__sdla_bus_read_4(card->hw, AFT_PORT_REG(card,AFT_SERIAL_LINE_CFG_REG), &reg);

	if (card->u.aft.serial_status == reg) {
		/* No change in status */
		return;
	}

	card->u.aft.serial_status=reg;

	if (card->wandev.ignore_front_end_status == WANOPT_YES) {
		return;
	}

	if (wan_test_bit(AFT_CHIPCFG_SERIAL_CTS_STATUS_INTR_BIT,&serial_intr_status)) {
		if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s: CTS ISR Status 0x%02X\n",
				card->devname,
				wan_test_bit(AFT_SERIAL_LCFG_CTS_BIT,&reg));
		}
	}

	if (wan_test_bit(AFT_CHIPCFG_SERIAL_DCD_STATUS_INTR_BIT,&serial_intr_status)) {
		if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s: DCS ISR Status 0x%02X\n",
				card->devname,
				wan_test_bit(AFT_SERIAL_LCFG_DCD_BIT,&reg));
		}
	}

	if (wan_test_bit(AFT_CHIPCFG_SERIAL_RTS_STATUS_INTR_BIT,&serial_intr_status)) {
		if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s: RTS ISR Status 0x%02X\n",
				card->devname,
				wan_test_bit(AFT_SERIAL_LCFG_RTS_BIT,&reg));
		}
	}
}



static void __wp_aft_per_per_port_isr(sdla_t *card, u32 dma_rx_reg, u32 dma_tx_reg)
{
	private_area_t *chan;
	u32 dma_tx_voice=0;
	int i;
	int timer_wakeup=0;


	dma_rx_reg&=card->u.aft.active_ch_map;
	if (!dma_rx_reg) {
		goto isr_skb_rx;
	}

	dma_rx_reg &= card->u.aft.logic_ch_map;
	dma_rx_reg &= ~(card->u.aft.tdm_logic_ch_map);

	for (i=0; i<card->u.aft.num_of_time_slots;i++){

		if (wan_test_bit(i,&dma_rx_reg)) {
			chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
			if (!chan){
				DEBUG_EVENT("%s: Error: No Dev for Rx logical ch=%d\n",
						card->devname,i);
				continue;
			}

			if (!wan_test_bit(WP_DEV_CONFIG,&chan->up)){
				continue;
			}

			if (chan->channelized_cfg && !chan->hdlc_eng){
				wan_set_bit(i,&dma_tx_voice);

				if (timer_wakeup==0){
					timer_wakeup++;
					/* Updated the Timer device based on our DMA */
					wp_timer_device_reg_tick(card);
				}
				continue;
			}

			if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
				continue;
			}

#if 0
			WAN_NETIF_STATS_INC_RX_FRAME_ERRORS(&chan->common);	//chan->if_stats.rx_frame_errors++;
#endif

			DEBUG_TEST("%s: RX Interrupt pend. \n",	card->devname);

#if defined(__WINDOWS__)
			wan_debug_update_timediff(&card->wan_debug_rx_interrupt_timing, __FUNCTION__);
#endif
			aft_dma_rx_complete(card,chan,0);

#ifdef AFT_SPAN_SINGLE_IRQ
			if (chan->wp_api_op_mode && !chan->hdlc_eng) {
				wan_set_bit(i,&dma_tx_voice);
				aft_dma_tx_complete(card,chan,0,0);
			}
#endif

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

	dma_tx_reg&=card->u.aft.active_ch_map;
	dma_tx_reg&=~dma_tx_voice;

	if (dma_tx_reg == 0){
		goto isr_skb_tx;
	}

	dma_tx_reg &= card->u.aft.logic_ch_map;
	dma_tx_reg &= ~(card->u.aft.tdm_logic_ch_map);


	for (i=0; i<card->u.aft.num_of_time_slots ;i++){
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

			if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
				continue;
			}

			DEBUG_ISR("---- TX Interrupt pend. --\n");
#if defined(__WINDOWS__)
			wan_debug_update_timediff(&card->wan_debug_tx_interrupt_timing, __FUNCTION__);
#endif
			aft_dma_tx_complete(card,chan,0,0);
		}
	}

isr_skb_tx:
    	DEBUG_ISR("---- ISR SKB TX end.-------------------\n");

	return;
}


static void wp_aft_dma_per_port_isr(sdla_t *card)
{
	int i;
	u32 dma_tx_reg=0,dma_rx_reg=0;
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
	private_area_t *chan;
#endif

       /* -----------------2/6/2003 9:37AM------------------
      	* Checking for Interrupt source:
      	* 1. Receive DMA Engine
      	* 2. Transmit DMA Engine
      	* 3. Error conditions.
      	* --------------------------------------------------*/

        /* Zaptel optimization. Dont waist time looking at
	   channels, when we know that only a single DCHAN
	   will use this code */
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
	if (card->wan_tdmv.sc &&
		wan_test_bit(WP_ZAPTEL_DCHAN_OPTIMIZATION,&card->u.aft.tdmv_zaptel_cfg)) {
		__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_DMA_INTR_PENDING_REG),&dma_rx_reg);
	    	__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_DMA_INTR_PENDING_REG),&dma_tx_reg);
	   	if (card->u.aft.tdmv_dchan) {
			chan=(private_area_t*)card->u.aft.dev_to_ch_map[card->u.aft.tdmv_dchan-1];
	   		if (chan && wan_test_bit(WP_DEV_CONFIG,&chan->up)) {
           		aft_dma_rx_complete(card,chan,0);
				aft_dma_tx_complete(card,chan,0,0);
			}
		}
		return;
	}
#endif



#ifdef AFT_FIFO_GEN_DEBUGGING
#ifndef __WINDOWS__
#warning "AFT_FIFO_GEN_DEBUGGING Enabled"
#endif
	if (card->wp_debug_gen_fifo_err) {
		if (card->wp_debug_gen_fifo_err++ < 10) {
			DEBUG_EVENT("%s:FIFO ERROR: Debugging Enabled ... causing fifo error!\n",card->devname); 
		}
     	return;
	}
#endif

	/* Receive DMA Engine */
	__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_DMA_INTR_PENDING_REG),&dma_rx_reg);
	__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_DMA_INTR_PENDING_REG),&dma_tx_reg);

	DEBUG_TEST("Line: %d: dma_rx_reg: 0x%X\n", __LINE__, dma_rx_reg);

	if (IS_BRI_CARD(card)) {
		void **card_list=__sdla_get_ptr_isr_array(card->hw);
		sdla_t *tmp_card;
		for (i=0;i<MAX_BRI_LINES;i++) {
			tmp_card=(sdla_t*)card_list[i];
			if (tmp_card &&
			    !wan_test_bit(CARD_DOWN,&tmp_card->wandev.critical) &&
			    !wan_test_bit(AFT_LCFG_TDMV_INTR_BIT,&tmp_card->u.aft.lcfg_reg)) {

				__wp_aft_per_per_port_isr(tmp_card,dma_rx_reg,dma_tx_reg);
			}
		}
	} else {
		__wp_aft_per_per_port_isr(card,dma_rx_reg,dma_tx_reg);
	}
}

static void wp_aft_tdmv_per_port_isr(sdla_t *card)
{
	int i;
	private_area_t *chan;

#if 0
	DEBUG_EVENT("%s: TDMV Interrupt LogicCh=%d\n",
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

		if (!wan_test_bit(WP_DEV_CONFIG,&chan->up)){
			continue;
		}

		if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
			continue;
		}

		if (chan->channelized_cfg && !chan->hdlc_eng){
			aft_dma_rx_tdmv(card,chan);
		}

#if 0
		WAN_NETIF_STATS_INC_RX_FRAME_ERRORS(&chan->common);	//chan->if_stats.rx_frame_errors++;
#endif

		DEBUG_ISR("%s: RX Interrupt pend. \n",
				card->devname);

	}
}



static void __wp_aft_wdt_per_port_isr (sdla_t *card, int wdt_intr, int *wdt_disable, int *timeout)
{
	int i;

	/* If TDM interrupt does not start in time
         * we have to re-start it */
	if (card->rsync_timeout){
		if (!IS_BRI_CARD(card)) {
			if (SYSTEM_TICKS - card->rsync_timeout > 2*HZ) {
				card->rsync_timeout=0;
				if (card->fe.fe_status == FE_CONNECTED) {
#if 0
#warning "TDM IRQ Timeout disabled - debugging"
					u32 reg=0,dma_rx_reg=0,dma_tx_reg=0;
					__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_RX_DMA_INTR_PENDING_REG),&dma_rx_reg);
					__sdla_bus_read_4(card->hw,AFT_PORT_REG(card,AFT_TX_DMA_INTR_PENDING_REG),&dma_tx_reg);
					__sdla_bus_read_4(card->hw,AFT_PORT_REG(card, AFT_CHIP_STAT_REG), &reg);
					DEBUG_EVENT("%s: TDM IRQ Timeout  0x%08X  rx=0x%08X  tx=0x%08X\n",card->devname,reg,dma_rx_reg,dma_tx_reg);
#else
					DEBUG_EVENT("%s: TDM IRQ Timeout\n",card->devname);
#endif
					card->hw_iface.fe_clear_bit(card->hw,1);
					if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
						wan_set_bit(AFT_FE_RESTART,&card->u.aft.port_task_cmd);
						WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
					}
				}
			}
		} else {
			if (SYSTEM_TICKS - card->rsync_timeout > 1*HZ) {
				int x;
				void **card_list=__sdla_get_ptr_isr_array(card->hw);
				sdla_t *first_card=NULL;
				card->rsync_timeout=0;

				for (x=0;x<MAX_BRI_LINES;x++) {
					first_card=(sdla_t*)card_list[x];
					if (first_card && wan_test_bit(AFT_LCFG_TDMV_INTR_BIT,&first_card->u.aft.lcfg_reg)) {
						break;
					}
					first_card=NULL;
				}

				if (first_card) {
					DEBUG_EVENT("%s: BRI TDM IRQ Timeout \n",
								first_card->devname);
					card->hw_iface.fe_clear_bit(card->hw,1);
					if (!wan_test_bit(AFT_FE_RESTART,&first_card->u.aft.port_task_cmd)) {
						if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
							wan_set_bit(AFT_FE_RESTART,&first_card->u.aft.port_task_cmd);
							WAN_TASKQ_SCHEDULE((&first_card->u.aft.port_task));
						}
					}
				}
			}
		}
               	return;
	}

	if (card->front_end_irq_timeout) {
		if ((SYSTEM_TICKS - card->front_end_irq_timeout) > HZ) {
			card->front_end_irq_timeout=0;
			if (!wan_test_bit(CARD_DOWN,&card->wandev.critical)) {
				DEBUG_EVENT("%s: Wanpipe Front End Interrupt Restart Timeout \n",
							card->devname);
				__aft_fe_intr_ctrl(card, 1);
			}				
		}
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

		if (!wan_test_bit(WP_DEV_CONFIG,&chan->up) ||
                    wan_test_bit(0,&chan->interface_down)){
			continue;
		}

		if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
			continue;
		}

#if 0
		if (wdt_intr){
			WAN_NETIF_STATS_INC_TX_DROPPED(&chan->common);	//++chan->if_stats.tx_dropped;
		}else{
			WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//++chan->if_stats.tx_errors;
		}
#endif

		if (card->wandev.state == WAN_CONNECTED){

			if (chan->dma_chain_opmode != WAN_AFT_DMA_CHAIN){
				*wdt_disable=1;
				continue;
			}
#if 0
			WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//++chan->if_stats.tx_errors;
#endif
			aft_dma_tx_complete (card,chan,1,0);

		}

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
		else{
			if (chan->tdmv_zaptel_cfg &&
			    wan_test_bit(WP_DEV_CONFIG,&chan->up) &&
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
						if (!tmp_card || !tmp_card->u.aft.global_tdm_irq) {
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
					WAN_NETIF_STATS_INC_TX_DROPPED(&chan->common);	//++chan->if_stats.tx_dropped;
#endif

#if 1
					*timeout=1;
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

	return;

}

static void wp_aft_wdt_per_port_isr(sdla_t *card, int wdt_intr)
{
	int wdt_disable = 0;
	int timeout=AFT_WDTCTRL_TIMEOUT;

	aft_wdt_reset(card);

	if (IS_BRI_CARD(card)) {
		int x;
		void **card_list=__sdla_get_ptr_isr_array(card->hw);
		sdla_t *first_card;
		for (x=0;x<MAX_BRI_LINES;x++) {
			first_card=(sdla_t *)card_list[x];
			if (first_card && !wan_test_bit(CARD_DOWN,&first_card->wandev.critical)) {
				__wp_aft_wdt_per_port_isr(first_card,wdt_intr,&wdt_disable,&timeout);
			}
		}

	} else {
		__wp_aft_wdt_per_port_isr(card,wdt_intr,&wdt_disable,&timeout);
	}



#ifdef AFT_WDT_ENABLE
	/* Since this fucntion can be called via interrupt or
	 * via interrupt poll, only re-enable wdt interrupt
	 * if the function was called from the wdt_intr
	 * not from wdt poll */
	if (!wdt_disable && wdt_intr){
		aft_wdt_set(card,(u8)timeout);
	}
#endif

	return;

}




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
static void port_set_state (sdla_t *card, u8 state)
{
	struct wan_dev_le	*devle;
	netdevice_t *dev;

	FUNC_NEWDRV();


	card->wandev.state = (char)state;
#if defined(__WINDOWS__)
	{
		int i;
		for (i = 0; i < NUM_OF_E1_CHANNELS; i++){
			if(card->sdla_net_device[i] != NULL && wan_test_bit(0,&card->up[i]) ){
				set_chan_state(card, card->sdla_net_device[i], state);
			}
		}
	}
#else
	WAN_LIST_FOREACH(devle, &card->wandev.dev_head, dev_link){
		dev = WAN_DEVLE2DEV(devle);
		if (!dev) continue;
		set_chan_state(card, dev, state);
	}
#endif

}




/*============================================================
 * callback_front_end_state
 *
 * Called by front end code to indicate that state has
 * changed. We will call the poll task to update the state.
 */

static void callback_front_end_state(void *card_id)
{
	sdla_t		*card = (sdla_t*)card_id;

	FUNC_NEWDRV();

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		return;
	}

	/* Call the poll task to update the state */
	if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
		wan_set_bit(AFT_FE_POLL,&card->u.aft.port_task_cmd);
		WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
	}

	return;
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

static void handle_front_end_state(void *card_id,int lock)
{
	sdla_t			*card = (sdla_t*)card_id;
	wan_smp_flag_t	flags=0;

	if (!wan_test_bit(AFT_CHIP_CONFIGURED,&card->u.aft.chip_cfg_status) &&
	    card->fe.fe_status == FE_CONNECTED) {
		DEBUG_TEST("%s: Skipping Front Front End State = %x\n",
				card->devname,card->fe.fe_status);

		wan_set_bit(AFT_FRONT_END_UP,&card->u.aft.chip_cfg_status);
		return;
	}

	if (card->fe.fe_status == FE_CONNECTED || card->wandev.ignore_front_end_status == WANOPT_YES){
		if (card->wandev.state != WAN_CONNECTED){

				if (lock) {
					wan_spin_lock_irq(&card->wandev.lock,&flags);
				}

			    /* FIXME: Only the T1/E1 front end will call back again.
				   This needs to be fixed in the core */
				if (IS_TE1_CARD(card) &&
					card->u.aft.global_tdm_irq &&
					card->hw_iface.fe_test_bit(card->hw,1)) {

					card->fe.fe_status = FE_DISCONNECTED;

					if (lock) {
						wan_spin_unlock_irq(&card->wandev.lock,&flags);
					}

					DEBUG_EVENT("%s: Skipping AFT Communication wait for ReSync...\n",
									card->devname);
					return;
				}

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
				if (card->wan_tdmv.sc){
					int	err;
					WAN_TDMV_CALL(state, (card, WAN_CONNECTED), err);
				}
#endif

				if (card->u.aft.global_tdm_irq &&
				    !wan_test_bit(0,&card->u.aft.tdmv_master_if_up)){

					/* FIXME: Only the T1/E1 front end will call back again.
					   This needs to be fixed in the core */
					if (IS_TE1_CARD(card)) {
						card->fe.fe_status = FE_DISCONNECTED;
					}

					if (lock) {
						wan_spin_unlock_irq(&card->wandev.lock,&flags);
					}

					DEBUG_EVENT("%s: Skipping AFT Communication wait for MasterIF\n",
							card->devname);
					return;
				}

				if (!IS_BRI_CARD(card) || !wan_test_bit(0,&card->u.aft.comm_enabled)) {
					enable_data_error_intr(card);
				}

				card->wandev.state = WAN_CONNECTED;

				aft_handle_clock_master(card);

				if (lock) {
					wan_spin_unlock_irq(&card->wandev.lock,&flags);
				}

				port_set_state(card,WAN_CONNECTED);

				if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
					wan_set_bit(AFT_FE_LED,&card->u.aft.port_task_cmd);
					WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
				}

				

		}
	}else{

		if (card->wandev.state == WAN_CONNECTED){

			port_set_state(card,WAN_DISCONNECTED);

			if (lock) {
				wan_spin_lock_irq(&card->wandev.lock,&flags);
			}

			if (!IS_BRI_CARD(card)) {

				/* Interrupts for non BRI cards stop
				   so tell timer device that this card is down */	
				wp_timer_device_unreg(card);

				/* Never disable interrupts for BRI since
				   all cards are on same timslot map */				
				disable_data_error_intr(card,LINK_DOWN);
			}

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
			if (card->wan_tdmv.sc){
				int	err;
				WAN_TDMV_CALL(state, (card, WAN_DISCONNECTED), err);
			}
#endif

			if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
				wan_set_bit(AFT_FE_LED,&card->u.aft.port_task_cmd);
				WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
			}

			aft_handle_clock_master(card);

			
			if (lock) {
				wan_spin_unlock_irq(&card->wandev.lock,&flags);
			}
		}
	}
}

static int aft_kickstart_global_tdm_irq(sdla_t *card)
{
	u32 reg;
	int fifo_reset_bit = aft_chipcfg_get_fifo_reset_bit(card);
	int rx_intr_ack_bit = aft_chipcfg_get_rx_intr_ack_bit(card);
	int tx_intr_ack_bit = aft_chipcfg_get_tx_intr_ack_bit(card);


	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG),&reg);

	/* Reset Global Fifo for the whole card */
	wan_set_bit(fifo_reset_bit,&reg);
	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG),reg); 
	WP_DELAY(2000);

	/* Clear Global Card Fifo reset */
	wan_clear_bit(fifo_reset_bit,&reg);


	/* Enable TDM Quad DMA Ring buffer */
	if (wan_test_bit(AFT_TDM_RING_BUF,&card->u.aft.chip_cfg_status)) {
		wan_set_bit(AFT_CHIPCFG_A108_A104_TDM_DMA_RINGBUF_BIT,&reg);

		card->hw_iface.fe_set_bit(card->hw,1);
	}else{
		wan_clear_bit(AFT_CHIPCFG_A108_A104_TDM_DMA_RINGBUF_BIT,&reg);
	}

	/* Global Acknowledge TDM Interrupt  (Kickstart) */
	if (card->adptr_type == A104_ADPTR_4TE1 &&
		card->u.aft.firm_id == AFT_PMC_FE_CORE_ID) {
			wan_set_bit(AFT_CHIPCFG_A104_TDM_ACK_BIT,&reg);
	} else {
		wan_set_bit(rx_intr_ack_bit,&reg);
		wan_set_bit(tx_intr_ack_bit,&reg);
	}

	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_CHIP_CFG_REG),reg);

	card->rsync_timeout=SYSTEM_TICKS;

	DEBUG_EVENT("%s: AFT Global TDM Intr\n",
			card->devname);

	return 0;
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
	int card_use_cnt;


	card->hw_iface.getcfg(card->hw, SDLA_HWTYPE_USEDCNT, &card_use_cnt);

	DEBUG_TEST("%s: %s()  Card Port =%d Use Cnt = %d \n",
			card->devname,__FUNCTION__,card->wandev.comm_port,card_use_cnt);


	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);
	if (wan_test_bit(AFT_LCFG_FE_IFACE_RESET_BIT,&reg)){
		DEBUG_EVENT("%s: Warning: Skipping data enable wait for cfg!\n",
				card->devname);
		return;
	}

#if 0
	aft_list_dma_chain_regs(card);
#endif

	if (card->u.aft.global_tdm_irq &&
	    !wan_test_bit(0,&card->u.aft.tdmv_master_if_up)){
		DEBUG_EVENT("%s: Critical error: Enable Card while Master If Not up!\n",
				card->devname);
	}

	if (wan_test_bit(0,&card->u.aft.comm_enabled)){
		disable_data_error_intr(card,LINK_DOWN);
	}

	aft_wdt_reset(card);

	if (!IS_BRI_CARD(card) || (IS_BRI_CARD(card) && card_use_cnt == 1)) {
		/* Clean Tx/Rx DMA interrupts */
		card->hw_iface.bus_read_4(card->hw,
					AFT_PORT_REG(card,AFT_TX_DMA_INTR_PENDING_REG),
					&reg);

		card->hw_iface.bus_read_4(card->hw,
					AFT_PORT_REG(card,AFT_RX_DMA_INTR_PENDING_REG),
					&reg);
	}

	err=aft_hwdev[card->wandev.card_type].aft_test_sync(card,0);
	if (err){
		card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);
		DEBUG_EVENT("%s: Error: Front End Interface out of sync! (0x%X)\n",
				card->devname,reg);
		/*FIXME: How to recover from here, should never happen */
	}

	if (card->u.aft.global_tdm_irq){
		card->hw_iface.bus_read_4(card->hw, AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);
		wan_set_bit(AFT_LCFG_TDMV_INTR_BIT,&reg);
		card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), reg);
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

		if (!wan_test_bit(WP_DEV_CONFIG,&chan->up)){
			continue;
		}

		if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
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
			aft_tx_dma_voice_handler(chan,0,1);
		}else{
			aft_tx_dma_chain_handler(chan,0,1);
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

		if (!wan_test_bit(WP_DEV_CONFIG,&chan->up)){
			continue;
		}

		if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
			continue;
		}

		DEBUG_TEST("%s: 3) Init interface fifo %s Logic Ch=%li\n",
				card->devname,chan->if_name,chan->logic_ch_num);

		aft_init_rx_dev_fifo(card, chan, WP_WAIT);
		aft_init_tx_dev_fifo(card, chan, WP_WAIT);

		DEBUG_TEST("%s: 4) Clearing Fifo and idle_flag %s Logic Ch=%li\n",
				card->devname,chan->if_name,chan->logic_ch_num);
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

		if (!wan_test_bit(WP_DEV_CONFIG,&chan->up)){
			continue;
		}

		if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
			continue;
		}

		DEBUG_TEST("%s: 4) Init interface %s\n",
				card->devname,chan->if_name);

#ifdef AFT_DMA_HISTORY_DEBUG
		chan->dma_index=0;
		memset(chan->dma_history,0,sizeof(chan->dma_history));
#endif
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



	for (i=0; i<card->u.aft.num_of_time_slots;i++){
		private_area_t *chan;

		if (!wan_test_bit(i,&card->u.aft.logic_ch_map)){
			continue;
		}

		chan=(private_area_t*)card->u.aft.dev_to_ch_map[i];
		if (!chan){
			continue;
		}

		card->u.aft.tdm_rx_dma_toggle[i]=0;
		card->u.aft.tdm_tx_dma_toggle[i]=0;

		memset(&chan->swring,0,sizeof(chan->swring));

		if (!wan_test_bit(WP_DEV_CONFIG,&chan->up)){
			continue;
		}

		if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
			continue;
		}

		if (!chan->hdlc_eng){
			aft_reset_tx_chain_cnt(chan);
			aft_dma_tx(card,chan);
			if (!chan->channelized_cfg) {
				/* Add 2 idle buffers into channel */
				aft_dma_tx(card,chan);
			}
		}

		if (chan->cfg.ss7_enable){
			aft_clear_ss7_force_rx(card,chan);
		}

		/* In zaptel mode we have to setup/init channel dma buffers
 		   therefore, we have to simulate a received data frame
		   from the hardare. This only happeds on connect. */
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

	if (card->u.aft.global_tdm_irq){
	      	wan_set_bit(AFT_LCFG_TDMV_INTR_BIT,&reg);
	}


	card->hw_iface.bus_write_4(card->hw,
				AFT_PORT_REG(card,AFT_LINE_CFG_REG), reg);

	card->u.aft.lcfg_reg=reg;


	wan_set_bit(0,&card->u.aft.comm_enabled);
	DEBUG_EVENT("%s: AFT communications enabled!\n",
			card->devname);

	/* Enable Channelized Driver if configured */
	if (card->u.aft.global_tdm_irq) {

		card->hw_iface.bus_read_4(card->hw,
			AFT_PORT_REG(card,AFT_RX_FIFO_INTR_PENDING_REG),
			&reg);
		card->hw_iface.bus_read_4(card->hw,
			AFT_PORT_REG(card,AFT_TX_FIFO_INTR_PENDING_REG),
			&reg);
  
	
		if (wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {

			aft_kickstart_global_tdm_irq(card);

		} else {

			card->hw_iface.bus_read_4(card->hw,
				AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
			wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&reg);
			wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&reg);
			card->hw_iface.bus_write_4(card->hw,
				AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
			
			if (wan_test_bit(AFT_TDM_SW_RING_BUF,&card->u.aft.chip_cfg_status)) {
				DEBUG_EVENT("%s: AFT Per Port TDM Intr (swring)\n",card->devname);
			} else {
				DEBUG_EVENT("%s: AFT Per Port TDM Intr\n",card->devname);
			}

		}

	}/* if (card->u.aft.global_tdm_irq) */

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
	int type_use_cnt;

	card->hw_iface.getcfg(card->hw, SDLA_HWTYPE_USEDCNT, &type_use_cnt);

	if(IS_BRI_CARD(card) && type_use_cnt > 1 && event < CRITICAL_DOWN){
		/* BRI card loaded with multiple ports should igore all
		   DOWN events EXCEPT CRITICAL */
		return;
	}

	DEBUG_EVENT("%s: AFT communications disabled! (Dev Cnt: %d Cause: %s)\n",
			card->devname, type_use_cnt,
			event==DEVICE_DOWN?"Device Down":
			event==CRITICAL_DOWN?"Critical Down" : "Link Down");

	aft_wdt_reset(card);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), &reg);

	wan_clear_bit(AFT_LCFG_DMA_INTR_BIT,&reg);
	wan_clear_bit(AFT_LCFG_FIFO_INTR_BIT,&reg);
	wan_clear_bit(AFT_LCFG_TDMV_INTR_BIT,&reg);


	if (IS_BRI_CARD(card)) {
		/* Disable Front end on BRI if its last device, OR if we are being shut down
                   CRITICALLY due to chip security error */
		if ((type_use_cnt == 1 && event == DEVICE_DOWN) || event == CRITICAL_DOWN){
			/* Disable Front End Interface */
			wan_set_bit(AFT_LCFG_FE_IFACE_RESET_BIT,&reg);
		}
	} else {
		if (event >= DEVICE_DOWN) {
			/* Disable Front End Interface */
			wan_set_bit(AFT_LCFG_FE_IFACE_RESET_BIT,&reg);
		}
	}
	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_LINE_CFG_REG), reg);
	card->u.aft.lcfg_reg=reg;

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
	wan_clear_bit(AFT_DMACTRL_GLOBAL_INTR_BIT,&reg);
	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);

	if (event == CRITICAL_DOWN) {

		card->fe_no_intr=1;
		__aft_fe_intr_ctrl(card, 0);

	} else 	if (IS_TE1_CARD(card) && event == LINK_DOWN){
		/* Only on Link down start zaptel timing watchdog interrupt.
                   Zaptel timing only applies to T1/E1 cards, all other cards
		   do not disable DMA on link down */
		if (card->u.aft.global_tdm_irq){
			DEBUG_EVENT("%s: Starting TDMV 1ms Timer\n",
					card->devname);

#ifdef AFT_WDT_ENABLE
			aft_wdt_set(card,1);
#endif
		}
	}

	wan_clear_bit(0,&card->u.aft.comm_enabled);
}


static void aft_rx_fifo_over_recover(sdla_t *card, private_area_t *chan)
{

#ifdef AFT_FIFO_GEN_DEBUGGING
	card->wp_debug_gen_fifo_err=0;
#endif

	/* Igore fifo errors in transpared mode. There is nothing
  	   that we can do to make it better. */
	if (chan->channelized_cfg && !chan->hdlc_eng){
		aft_tdm_chan_ring_rsyinc(card,chan);
		return;
	}

	/* Stream re-synchronization is not necessary when 
	   running on single time slot bitstreaming. Equivalent to channelized mode. */
	if (!chan->hdlc_eng && chan->num_of_time_slots == 1) {
		DEBUG_TEST("%s:%s FIFO ERROR: NON Channelize fifo!\n",card->devname,chan->if_name);
		aft_free_rx_descriptors(chan);
		aft_dma_rx(card,chan);
		wanpipe_wake_stack(chan);
     	return;
	}

	/* If running in mixed mode DATA + Voice do not disable DMA */
	if (chan->hdlc_eng && card->u.aft.global_tdm_irq) {
		DEBUG_TEST("%s:%s FIFO ERROR: Rx Fifo on DCHAN!\n",card->devname,chan->if_name);
		aft_free_rx_descriptors(chan);
		aft_dma_rx(card,chan);
		wanpipe_wake_stack(chan);
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

#ifdef AFT_DMA_HISTORY_DEBUG
	chan->dma_index=0;
	memset(chan->dma_history,0,sizeof(chan->dma_history));
#endif

	aft_reset_rx_chain_cnt(chan);

	aft_dma_rx(card,chan);

	aft_tslot_sync_ctrl(card,chan,1);

	wanpipe_wake_stack(chan);
}

static void aft_tx_fifo_under_recover (sdla_t *card, private_area_t *chan)
{  
	/* Igore fifo errors in transpared channalized mode. 
	   There is nothing that we can do to make it better. */
	if (chan->channelized_cfg && !chan->hdlc_eng){
   		return;
	}

	/* Stream re-synchronization is not necessary when 
	   running on single time slot bitstreaming. Equivalent to channelized mode. */
	if (!chan->hdlc_eng && chan->num_of_time_slots == 1) {
		aft_dma_tx_complete(card,chan,0, 1);
		aft_free_tx_descriptors(chan);
		aft_dma_tx(card,chan);
		wanpipe_wake_stack(chan);
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
   	aft_reset_tx_chain_cnt(chan);

	wan_clear_bit(0,&chan->idle_start);

	aft_dma_tx(card,chan);
	wanpipe_wake_stack(chan);
}

static int set_chan_state(sdla_t* card, netdevice_t* dev, u8 state)
{
	private_area_t *chan = wan_netif_priv(dev);
	private_area_t *ch_ptr;
#if 0
	wan_smp_flag_t flags;
#endif

	FUNC_NEWDRV();

	if (!chan){
		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s: %s:%d No chan ptr!\n",card->devname,__FUNCTION__,__LINE__);
		}
		return -EINVAL;
	}

	if (chan->common.state == state) {
		return 0;
	}
		 
#if defined(__WINDOWS__)
	set_netdev_state(card, dev, state);
#endif

#ifdef AFT_TDM_API_SUPPORT
	if (is_tdm_api(chan,&chan->wp_tdm_api_dev) && chan->common.state != state) {
		if (card->wandev.config_id != WANCONFIG_AFT_ANALOG) {
			wan_event_t	event;

			event.type	= WAN_EVENT_LINK_STATUS;
			event.channel	= 0;
			if (state == WAN_CONNECTED) {
				event.link_status= WAN_EVENT_LINK_STATUS_CONNECTED;
			} else {
				event.link_status= WAN_EVENT_LINK_STATUS_DISCONNECTED;
			}

			if (card->wandev.event_callback.linkstatus) {
				card->wandev.event_callback.linkstatus(card, &event);
			}
		}
	}
#endif


#if 0
	wan_spin_lock_irq(&card->wandev.lock,&irq_flags);
#endif

	for (ch_ptr=chan; ch_ptr != NULL; ch_ptr=ch_ptr->next){

		if (ch_ptr->common.state==state) {
			continue;
		}

		ch_ptr->common.state=state;	
		
		if (ch_ptr->tdmv_zaptel_cfg) {
			continue;
		}

#ifdef CONFIG_PRODUCT_WANPIPE_ANNEXG
		if (ch_ptr->common.usedby == ANNEXG &&
			ch_ptr->annexg_dev){
			if (state == WAN_CONNECTED){
				if (IS_FUNC_CALL(lapb_protocol,lapb_link_up)){
					lapb_protocol.lapb_link_up(ch_ptr->annexg_dev);
				}
			} else {
				if (IS_FUNC_CALL(lapb_protocol,lapb_link_down)){
					lapb_protocol.lapb_link_down(ch_ptr->annexg_dev);
				}
			}
		}
#endif
	}

	chan->common.state = state;

	/* On some LINUX kernels calling network device wake before
 	   device is up can cause a panic. So make sure that interface
  	   is up before you attempt to call interface WAKE */
	if (wan_test_bit(WP_DEV_UP,&chan->up)) {
		if (state == WAN_CONNECTED){
			wan_clear_bit(0,&chan->idle_start);
			WAN_NETIF_START_QUEUE(dev);
			wan_chan_dev_start(chan);
			chan->opstats.link_active_count++;
			WAN_NETIF_CARRIER_ON(dev);
			WAN_NETIF_WAKE_QUEUE(dev);
		}else{
			chan->opstats.link_inactive_modem_count++;
			WAN_NETIF_CARRIER_OFF(dev);
			WAN_NETIF_STOP_QUEUE(dev);
			wan_chan_dev_stop(chan);
		}
	}

#if defined(__LINUX__)
# if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
	if (chan->common.usedby == API) {
		wan_update_api_state(chan);
	}
#endif
#endif

	if (chan->common.usedby == STACK){
#if defined(__WINDOWS__)
		FUNC_NOT_IMPL();
#else
		if (state == WAN_CONNECTED){
			wanpipe_lip_connect(chan,0);
		}else{
			wanpipe_lip_disconnect(chan,0);
		}
#endif
	}

#if defined(NETGRAPH)
	if (chan->common.usedby == WP_NETGRAPH){
		wan_ng_link_state(&chan->common, state);
	}
#endif

#if defined(AFT_XMTP2_API_SUPPORT)
	if (chan->common.usedby == XMTP2_API) {
		if (state == WAN_CONNECTED){
			xmtp2km_facility_state_change(chan->xmtp2_api_index, 1);
		} else {
			xmtp2km_facility_state_change(chan->xmtp2_api_index, 0);
		}
	}
#endif

#if 0
	wan_spin_unlock_irq(&card->wandev.lock,&irq_flags);
#endif
	FUNC_NEWDRV();
	return 0;
}





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
static void aft_tx_dma_voice_handler(private_area_t *chan, int wdt, int reset)
{
	sdla_t *card = chan->card;
	u32 reg,dma_descr,dma_status;
	wan_dma_descr_t *dma_chain;

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
		WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//chan->if_stats.tx_errors++;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
	}


	chan->opstats.Data_frames_Tx_count++;
	chan->opstats.Data_bytes_Tx_count+=wan_skb_len(dma_chain->skb);
	chan->chan_stats.tx_packets++;
	chan->chan_stats.tx_bytes+=wan_skb_len(dma_chain->skb);
	WAN_NETIF_STATS_INC_TX_PACKETS(&chan->common);	//chan->if_stats.tx_packets++;
	WAN_NETIF_STATS_INC_TX_BYTES(&chan->common,wan_skb_len(dma_chain->skb));	//chan->if_stats.tx_bytes+=wan_skb_len(dma_chain->skb);

	wan_clear_bit(0,&dma_chain->init);

aft_tx_dma_voice_handler_exit:
	wan_clear_bit(TX_HANDLER_BUSY,&chan->dma_status);

	return;
}

#if 0
static int aft_tx_dma_chain_diff(private_area_t *chan) 
{
	u32 dma_ram_desc,reg,cur_dma_ptr;
	sdla_t *card=chan->card;

	dma_ram_desc=chan->logic_ch_num*4+
			AFT_PORT_REG(card,AFT_DMA_CHAIN_RAM_BASE_REG);
	card->hw_iface.bus_read_4(card->hw,dma_ram_desc,&reg);
	cur_dma_ptr=aft_dmachain_get_tx_dma_addr(reg);

	DEBUG_EVENT("%s:  Tx Chains Curr=%i Pending=%i HW=%i Diff=%i\n",
			   chan->if_name,chan->tx_chain_indx, chan->tx_pending_chain_indx, cur_dma_ptr,
			   aft_tx_dma_chain_chain_len(chan) 
			   );
		
	return 0;
}
#endif


/*===============================================
 * aft_tx_dma_chain_handler
 *
 */
static int aft_tx_dma_chain_handler(private_area_t *chan, int wdt, int reset)
{
	sdla_t *card = chan->card;
	u32 reg,dma_descr;
	wan_dma_descr_t *dma_chain;
	int tx_complete=0;

	if (wan_test_and_set_bit(TX_HANDLER_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);
		return tx_complete;
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

		tx_complete++;

		DEBUG_TEST("%s: TX DMA Handler Chain %d\n",chan->if_name,dma_chain->index);

		if (chan->hdlc_eng){

			if (dma_chain->skb == chan->tx_hdlc_rpt_skb) {
				if (wan_skb_queue_len(&chan->wp_tx_hdlc_rpt_list) == 0) {
					wan_skb_queue_tail(&chan->wp_tx_hdlc_rpt_list,dma_chain->skb);
					dma_chain->skb=NULL;
				} else {
					/* Drop down to init where packet will freed */
				}
				chan->tx_hdlc_rpt_skb=NULL;

			} else if (dma_chain->skb){
				wan_skb_set_csum(dma_chain->skb, reg);
				wan_skb_queue_tail(&chan->wp_tx_complete_list,dma_chain->skb);
				dma_chain->skb=NULL;
			}
		} else {
			if (dma_chain->skb != chan->tx_idle_skb){

				wan_skb_set_csum(dma_chain->skb, reg);
				aft_tx_post_complete(chan->card,chan,dma_chain->skb);
				aft_tx_dma_skb_init(chan,dma_chain->skb);

				dma_chain->skb=NULL;
			}
		}

		aft_tx_dma_chain_init(chan,dma_chain);

		if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE){
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

	return tx_complete;
}

/*===============================================
 * aft_dma_chain_tx
 *
 */
static int aft_dma_chain_tx(wan_dma_descr_t *dma_chain,private_area_t *chan, int intr,int fifo)
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

	DEBUG_DMA("%s: %s:%s: LogicCh=%ld ChIndex=%d DmaDesc=0x%x set\n",
                    		__FUNCTION__, card->devname, chan->if_name,
				chan->logic_ch_num,dma_ch_indx,dma_descr);

	card->hw_iface.bus_read_4(card->hw,dma_descr,&reg);

	if (wan_test_bit(AFT_TXDMA_HI_GO_BIT,&reg)){
		/* This can happen during tx fifo error */
		DEBUG_EVENT("%s: Warning: TxDMA GO Ready bit set on dma (chain=0x%X) Desc=0x%X Tx 0x%X\n",
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

	DEBUG_DMA("%s: %s:%s: TXDMA_LO=0x%X PhyAddr=0x%X DmaDescr=0x%X Len=%d\n",
			__FUNCTION__,card->devname,chan->if_name,
			reg,dma_chain->dma_addr,dma_descr,len);

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

	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE){
		wan_clear_bit(AFT_TXDMA_HI_LAST_DESC_BIT,&reg);
		wan_clear_bit(AFT_TXDMA_HI_INTR_DISABLE_BIT,&reg);

		if (chan->channelized_cfg && !chan->hdlc_eng){
			wan_set_bit(AFT_TXDMA_HI_LAST_DESC_BIT,&reg);
		}
	} else if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_IRQ_ALL){
		wan_set_bit(AFT_TXDMA_HI_LAST_DESC_BIT,&reg);
		wan_clear_bit(AFT_TXDMA_HI_INTR_DISABLE_BIT,&reg);

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

	DEBUG_DMA("%s:: %s:%s: TXDMA_HI=0x%X DmaDescr=0x%X Len=%d Intr=%d\n",
			__FUNCTION__,card->devname,chan->if_name,
			reg,dma_descr,len,intr);

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
static void
aft_tx_dma_chain_init(private_area_t *chan, wan_dma_descr_t *dma_chain)
{
#define card	    chan->card

	card->hw_iface.busdma_sync(	card->hw,
					dma_chain,
					1, 1,
					SDLA_DMA_POSTWRITE);
	card->hw_iface.busdma_unmap(	card->hw,
					dma_chain,
					SDLA_DMA_POSTWRITE);

	if (dma_chain->skb){
		if (!chan->hdlc_eng){
			aft_tx_dma_skb_init(chan,dma_chain->skb);
			dma_chain->skb=NULL;
		}else{
			wan_aft_skb_defered_dealloc(chan,dma_chain->skb);
			dma_chain->skb=NULL;
		}
	}
	wan_clear_bit(0,&dma_chain->init);
#undef card
}

static void
aft_rx_dma_chain_init(private_area_t *chan, wan_dma_descr_t *dma_chain)
{
#define card	chan->card

	card->hw_iface.busdma_sync(	card->hw,
					dma_chain,
					1, 1,
					SDLA_DMA_POSTREAD);
	card->hw_iface.busdma_unmap(	card->hw,
					dma_chain,
					SDLA_DMA_POSTREAD);

	if (dma_chain->skb){
		aft_init_requeue_free_skb(chan,dma_chain->skb);
		dma_chain->skb = NULL;
	}
	wan_clear_bit(0,&dma_chain->init);
#undef card
}



static int aft_dma_voice_tx(sdla_t *card, private_area_t *chan)
{
	int err=0;
	wan_dma_descr_t *dma_chain;
	u32 reg, dma_ram_desc;

	if (wan_test_and_set_bit(TX_DMA_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);

		return -EBUSY;
	}

	dma_chain = &chan->tx_dma_chain_table[0];

	DEBUG_DMA("%s: %s:%s:: Chain %d  Used %ld\n",
			__FUNCTION__,card->devname,chan->if_name,
			dma_chain->index,dma_chain->init);

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

		wan_skb_init(dma_chain->skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(dma_chain->skb,0);

		/*NC: We must set the initial value of the
		 *    frist DMA TX transfer to 2*MTU.  This is
		 *    to avoid potential Tx FIFO underrun.
		 *
		 *    This is equivalent of transmitting twice
		 *    very fist time. */


		buf=wan_skb_put(dma_chain->skb,chan->mtu*2);
		memset(buf,chan->idle_flag,chan->mtu*2);

		/* A-DMA */
		card->hw_iface.busdma_map(	card->hw,
						dma_chain,
						wan_skb_data(dma_chain->skb),
						wan_skb_len(dma_chain->skb),
						chan->dma_mru,
						SDLA_DMA_PREWRITE,
						dma_chain->skb);
		card->hw_iface.busdma_sync(	card->hw,
						dma_chain,
						1, 1,
						SDLA_DMA_PREWRITE);
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
		WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//chan->if_stats.tx_errors++;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
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
int aft_dma_tx (sdla_t *card,private_area_t *chan)
{
	int err=0, intr=0, cnt=0;
	wan_dma_descr_t *dma_chain;
	int tx_data_cnt=0;
	netskb_t *skb=NULL;

	if (wan_test_bit(0,&chan->interface_down)) {
		return -EBUSY;
	}

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

				if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_IRQ_ALL){
					int chain_diff=aft_tx_dma_chain_chain_len(chan);
					if (chain_diff >= 2 || tx_data_cnt) {
						wan_clear_bit(0,&dma_chain->init);
						wan_clear_bit(TX_DMA_BUSY,&chan->dma_status);
						break;
					}
					tx_data_cnt++;
				}

					
				skb=chan->tx_idle_skb;
				if (!skb){
					if (WAN_NET_RATELIMIT()){
					DEBUG_EVENT("%s: Critical Error: Transparent tx no skb!\n",
							chan->if_name);
					}
					wan_clear_bit(0,&dma_chain->init);
					wan_clear_bit(TX_DMA_BUSY,&chan->dma_status);
					break;
				}

				WAN_NETIF_STATS_INC_TX_CARRIER_ERRORS(&chan->common);
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_carrier_errors);
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_idle_packets);

			} else {
				if (chan->cfg.hdlc_repeat) {
					/* Pull the latest repeat frame out of the
					   repeat queue */
					for (;;) {
						skb=wan_skb_dequeue(&chan->wp_tx_hdlc_rpt_list);
						if (!skb) {
							break;
						}
						if (wan_skb_queue_len(&chan->wp_tx_hdlc_rpt_list)){
							wan_aft_skb_defered_dealloc(chan,skb);
						} else {
							break;
						}
					}

					if (skb) {
						chan->tx_hdlc_rpt_skb=skb;
					} else {
						WAN_NETIF_STATS_INC_TX_CARRIER_ERRORS(&chan->common);
						WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_carrier_errors);
						WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_idle_packets);
						/* Pass throught to no skb condition */
					}
				}

				if (!skb) {
					wan_clear_bit(0,&dma_chain->init);
					wan_clear_bit(TX_DMA_BUSY,&chan->dma_status);
					break;
				}
			}
		} else {
         	tx_data_cnt++;
		}

		if ((ulong_ptr_t)wan_skb_data(skb) & 0x03){

			err=aft_realign_skb_pkt(chan,skb);
			if (err){
				if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: Tx Error: Non Aligned packet %p: dropping...\n",
						chan->if_name,wan_skb_data(skb));
				}
				wan_aft_skb_defered_dealloc(chan,skb);
				aft_tx_dma_chain_init(chan,dma_chain);
				WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//chan->if_stats.tx_errors++;
				WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
				chan->opstats.Tx_Data_discard_lgth_err_count++;
				continue;
			}
		}


		if (!chan->hdlc_eng && (wan_skb_len(skb)%4)){

			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: Tx Error: Tx Length %d not 32bit aligned: dropping...\n",
						chan->if_name,wan_skb_len(skb));
			}
			wan_aft_skb_defered_dealloc(chan,skb);
			aft_tx_dma_chain_init(chan,dma_chain);
			WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//chan->if_stats.tx_errors++;
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
			chan->opstats.Tx_Data_discard_lgth_err_count++;
			continue;
		}

		dma_chain->skb=skb;

		/* A-DMA */
		card->hw_iface.busdma_map(	card->hw,
						dma_chain,
						wan_skb_data(dma_chain->skb),
						wan_skb_len(dma_chain->skb),
						wan_skb_len(dma_chain->skb),
						SDLA_DMA_PREWRITE,
						dma_chain->skb);
		card->hw_iface.busdma_sync(	card->hw,
						dma_chain,
						1, (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE),
						SDLA_DMA_PREWRITE);

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
			aft_critical_trigger(card);
			break;
#endif

			/* Drop the tx packet here */
			aft_tx_dma_chain_init(chan,dma_chain);
			WAN_NETIF_STATS_INC_TX_ERRORS(&chan->common);	//chan->if_stats.tx_errors++;
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,tx_errors);
			break;
		}

#ifndef __WINDOWS__
		if (skb){
			wan_capture_trace_packet(card, &chan->trace_info, skb, TRC_OUTGOING_FRM);
		}
#endif

		if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE){
			break;
		}

		if (++chan->tx_chain_indx >= MAX_AFT_DMA_CHAINS){
		 	chan->tx_chain_indx=0;
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

static int aft_dma_chain_rx(wan_dma_descr_t *dma_chain, private_area_t *chan, int intr, int fifo)
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

	DEBUG_DMA("%s: %s:%s: RxDMA_LO(%ld) = 0x%X, PhyAddr:%X DmaDescr=0x%X (%p)\n",
				__FUNCTION__,card->devname,chan->if_name,
				chan->logic_ch_num,reg,
				dma_chain->dma_addr, dma_descr,dma_chain);

	card->hw_iface.bus_write_4(card->hw,dma_descr,reg);

	dma_descr=(chan->logic_ch_num<<4) + (dma_ch_indx*AFT_DMA_INDEX_OFFSET) +
		  AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

    reg =0;

	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE){
		wan_clear_bit(AFT_RXDMA_HI_LAST_DESC_BIT,&reg);
		wan_clear_bit(AFT_RXDMA_HI_INTR_DISABLE_BIT,&reg);

		if (chan->channelized_cfg && !chan->hdlc_eng){
			wan_set_bit(AFT_RXDMA_HI_LAST_DESC_BIT,&reg);
		}

	} else if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_IRQ_ALL){
		wan_set_bit(AFT_RXDMA_HI_LAST_DESC_BIT,&reg);
		wan_clear_bit(AFT_RXDMA_HI_INTR_DISABLE_BIT,&reg);
	
	} else {
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

	DEBUG_DMA("%s: %s:%s: RXDMA_HI(%ld) = 0x%X, DmaDescr=0x%X\n",
			__FUNCTION__,card->devname,chan->if_name,
			chan->logic_ch_num,reg,dma_descr);

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
	wan_dma_descr_t *dma_chain;
	u32 reg, dma_ram_desc;

	FUNC_NEWDRV();

	if (wan_test_and_set_bit(RX_DMA_BUSY,&chan->dma_status)){
		DEBUG_EVENT("%s: SMP Critical in %s\n",
				chan->if_name,__FUNCTION__);
		return -EBUSY;
	}

	dma_chain = &chan->rx_dma_chain_table[0];

	DEBUG_DMA("%s: %s:%s: Chain %d  Used %ld\n",
			__FUNCTION__,card->devname,chan->if_name,
			dma_chain->index,dma_chain->init);

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

		wan_skb_init(dma_chain->skb,sizeof(wp_api_hdr_t));
		wan_skb_trim(dma_chain->skb,0);

		/* A-DMA */
		card->hw_iface.busdma_map(
				card->hw,
				dma_chain,
				wan_skb_tail(dma_chain->skb),
				chan->dma_mru,
				chan->dma_mru,
				SDLA_DMA_PREREAD,
				dma_chain->skb);
		card->hw_iface.busdma_sync(
				card->hw,
				dma_chain,
				MAX_AFT_DMA_CHAINS, (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE),
				SDLA_DMA_PREREAD);
		DEBUG_DMA("%s: RXDMA PHY = 0x%08X VIRT = %p \n",
				chan->if_name,
				dma_chain->dma_addr,wan_skb_tail(dma_chain->skb)+dma_chain->dma_offset);
	}else{
		wan_skb_init(dma_chain->skb,sizeof(wp_api_hdr_t));
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
		WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//chan->if_stats.rx_errors++;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_errors);
	}

aft_dma_single_chain_rx_exit:

	wan_clear_bit(RX_DMA_BUSY,&chan->dma_status);
	return err;
}


static int aft_dma_rx(sdla_t *card, private_area_t *chan)
{
	int err=0, intr=0;
	wan_dma_descr_t *dma_chain;
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

	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE) {
      	/* keep going */
	} else if (free_queue_len < MAX_AFT_DMA_CHAINS){
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

	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE) {
      	/* keep going */
	}else if (free_queue_len < max_dma_cnt){
		max_dma_cnt = free_queue_len;
	}

	DEBUG_RX("%s: (line: %d) DMA RX: CBoardPtr=%d  Driver=%d MaxDMA=%d\n",
			card->devname, __LINE__, cur_dma_ptr,chan->rx_chain_indx,max_dma_cnt);

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
			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: Rx driver buffering overrun rxf=%d rxc=%d !\n",
					chan->if_name,
					wan_skb_queue_len(&chan->wp_rx_free_list),
					wan_skb_queue_len(&chan->wp_rx_complete_list));
			}

			aft_free_rx_complete_list(chan);

			dma_chain->skb=wan_skb_dequeue(&chan->wp_rx_free_list);
			if (!dma_chain->skb) {
				DEBUG_EVENT("%s: Critical Rx chain = %d: no free rx bufs (Free=%d Comp=%d)\n",
					chan->if_name,dma_chain->index,
					wan_skb_queue_len(&chan->wp_rx_free_list),
					wan_skb_queue_len(&chan->wp_rx_complete_list));
				wan_clear_bit(0,&dma_chain->init);

				aft_critical_trigger(card);
				err=-EINVAL;
				break;
			}

		}

		card->hw_iface.busdma_map(
					card->hw,
					dma_chain,
					wan_skb_tail(dma_chain->skb),
					chan->dma_mru,
					chan->dma_mru,
					SDLA_DMA_PREREAD,
					dma_chain->skb);
		card->hw_iface.busdma_sync(
					card->hw,
					dma_chain,
					1, (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE),
					SDLA_DMA_PREREAD);
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

		DEBUG_TEST("%s: Setting Buffer on Rx Chain = %d Intr=%d HeadRoom=%d Len=%d\n",
					chan->if_name,dma_chain->index, intr, wan_skb_headroom(dma_chain->skb), wan_skb_len(dma_chain	->skb));

		err=aft_dma_chain_rx(dma_chain,chan,intr,0);
		if (err){
			DEBUG_EVENT("%s: Rx dma chain %d overrun error: should never happen!\n",
					chan->if_name,dma_chain->index);
			aft_rx_dma_chain_init(chan,dma_chain);
			WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//chan->if_stats.rx_errors++;
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_errors);
			break;
		}

		if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE) {
			break;
		} else{
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
	wan_dma_descr_t *dma_chain;
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

	DEBUG_RX("%s: (line: %d) DMA RX: CBoardPtr=%d  Driver=%d MaxDMA=%d\n",
			card->devname, __LINE__,
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
		if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE) {
				DEBUG_EVENT("%s: CRITICAL (%s) Pending chain %d Go bit still set!\n",
					chan->if_name,__FUNCTION__,dma_chain->index);
				WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//++chan->if_stats.rx_errors;
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

		if (sizeof(wp_rx_element_t) > wan_skb_headroom(dma_chain->skb)){
			if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: Rx error: rx_el=%u > max head room=%u\n",
						chan->if_name,
						(u32)sizeof(wp_rx_element_t),
						(u32)wan_skb_headroom(dma_chain->skb));
			}

			aft_init_requeue_free_skb(chan, dma_chain->skb);
			WAN_NETIF_STATS_INC_RX_ERRORS(&chan->common);	//++chan->if_stats.rx_errors;
			WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_errors);
			goto rx_hndlr_skip_rx;
		}else{
			rx_el=(wp_rx_element_t *)wan_skb_push(dma_chain->skb,
							      sizeof(wp_rx_element_t));
			memset(rx_el,0,sizeof(wp_rx_element_t));
		}
#if 0
		WAN_NETIF_STATS_INC_RX_FRAME_ERRORS(&chan->common);	//chan->if_stats.rx_frame_errors++;
#endif

       		/* Reading Rx DMA descriptor information */
		dma_descr=(chan->logic_ch_num<<4) +(dma_chain->index*AFT_DMA_INDEX_OFFSET) +
			  AFT_PORT_REG(card,AFT_RX_DMA_LO_DESCR_BASE_REG);

		card->hw_iface.bus_read_4(card->hw,dma_descr, &rx_el->align);
		rx_el->align&=AFT_RXDMA_LO_ALIGN_MASK;

       		dma_descr=(chan->logic_ch_num<<4) +(dma_chain->index*AFT_DMA_INDEX_OFFSET) +
			  AFT_PORT_REG(card,AFT_RX_DMA_HI_DESCR_BASE_REG);

		card->hw_iface.bus_read_4(card->hw,dma_descr, &rx_el->reg);

		/* New for FreeBSD/Solaris */
		rx_el->len=rx_el->reg&AFT_RXDMA_HI_DMA_LENGTH_MASK;
		if (chan->hdlc_eng){
			/* In HDLC mode, calculate rx length based
			* on alignment value, received from DMA */
			rx_el->len=((((chan->dma_mru>>2)-1)-rx_el->len)<<2) - (~(rx_el->align)&AFT_RXDMA_LO_ALIGN_MASK);
		}else{
			/* In Transparent mode, our RX buffer will always be
			* aligned to the 32bit (word) boundary, because
			* the RX buffers are all of equal length  */
			rx_el->len=(((chan->mru>>2)-rx_el->len)<<2) - (~(0x03)&AFT_RXDMA_LO_ALIGN_MASK);
		}

		rx_el->pkt_error= dma_chain->pkt_error;
		rx_el->dma_addr = dma_chain->dma_addr;

		/* A-DMA */
		card->hw_iface.busdma_sync(	card->hw,
						dma_chain,
						1, (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE),
						SDLA_DMA_POSTREAD);
		dma_chain->dma_len = rx_el->len;	/* update real dma len*/
		card->hw_iface.busdma_unmap(	card->hw,
						dma_chain,
						SDLA_DMA_POSTREAD);

#if defined(__FreeBSD__)
		wan_skb_put(dma_chain->skb, rx_el->len);
#endif
		wan_skb_queue_tail(&chan->wp_rx_complete_list,dma_chain->skb);
		rx_data_available=1;

		DEBUG_TEST("%s: RxInt Pending chain %d Rxlist=%d LO:0x%X HI:0x%X Len=%d!\n",
				chan->if_name,dma_chain->index,
				wan_skb_queue_len(&chan->wp_rx_complete_list),
				rx_el->align,rx_el->reg, rx_el->len);

rx_hndlr_skip_rx:
		dma_chain->skb=NULL;
		dma_chain->pkt_error=0;
		wan_clear_bit(0,&dma_chain->init);

		if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE) {
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
	u32 i;
	u32 reg=0;
	sdla_t *card=chan->card;
	unsigned long tx_dma_descr,rx_dma_descr;
	unsigned int dma_cnt=MAX_AFT_DMA_CHAINS;

	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE) {
		dma_cnt=1;
	}

	if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
		return;
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
		WAN_NETIF_STATS_INC_RX_DROPPED(&chan->common);	//chan->if_stats.rx_dropped++;
		WP_AFT_CHAN_ERROR_STATS(chan->chan_stats,rx_dropped);
		aft_init_requeue_free_skb(chan, skb);
	}
}



static void aft_rx_cur_go_test(private_area_t *chan)
{
#if 0
	u32 reg,cur_dma_ptr;
	sdla_t *card=chan->card;
	wan_dma_descr_t *dma_chain;
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



static void aft_free_rx_descriptors(private_area_t *chan)
{
	u32 reg;
	sdla_t *card=chan->card;
	wan_dma_descr_t *dma_chain;
	u32 dma_descr;
	u32 i;
	unsigned int dma_cnt=MAX_AFT_DMA_CHAINS;

	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE) {
		dma_cnt=1;
	}

	if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
		return;
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

		card->hw_iface.busdma_sync(	card->hw,
						dma_chain,
						1, (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE),
						SDLA_DMA_POSTREAD);
		card->hw_iface.busdma_unmap(	card->hw,
						dma_chain,
						SDLA_DMA_POSTREAD);

		if (dma_chain->skb){
			aft_init_requeue_free_skb(chan, dma_chain->skb);
		}

		dma_chain->skb=NULL;
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
	chan->rx_pending_chain_indx = chan->rx_chain_indx = (u8)cur_dma_ptr;
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
	chan->tx_pending_chain_indx = chan->tx_chain_indx = (u8)cur_dma_ptr;
	DEBUG_TEST("%s:  Setting Tx Index to %d\n",
		chan->if_name,cur_dma_ptr);

}



static void aft_free_tx_descriptors(private_area_t *chan)
{
	u32 reg,dma_descr;
	sdla_t *card=chan->card;
	wan_dma_descr_t *dma_chain;
	u32 i;
	void *skb;
	unsigned int dma_cnt=MAX_AFT_DMA_CHAINS;

	if (chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE) {
		dma_cnt=1;
	}

	if (IS_BRI_CARD(card) && chan->dchan_time_slot >= 0){
		return;
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
		if (!chan->hdlc_eng) {
			aft_tx_dma_skb_init(chan,skb);
		} else {
			wan_aft_skb_defered_dealloc(chan,skb);
		}
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
				card->u.aft.top_logic_ch=(char)i;
			}
		}


		aft_dma_max_logic_ch(card);
	}

}

void aft_dma_max_logic_ch(sdla_t *card)
{
	u32 reg;
	u32 max_logic_ch;


	reg=0;
	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
	max_logic_ch = aft_dmactrl_get_max_logic_ch(reg);

	DEBUG_TEST("%s: Maximum Logic Ch (current %d) set to %d \n",
			card->devname,
			max_logic_ch,
			card->u.aft.top_logic_ch);

	if (card->u.aft.top_logic_ch > max_logic_ch) {
		aft_dmactrl_set_max_logic_ch(&reg,card->u.aft.top_logic_ch);
		card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
	}
}

#if defined(__WINDOWS__)
static void aft_port_task (IN PKDPC Dpc, IN PVOID card_ptr, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
#if defined(WAN_SPINLOCK_IS_FASTMUTEX)
	/* this will cause __aft_port_task() to run in a low-priority thread */
	trigger_aft_port_task((sdla_t *)card_ptr);
#else
	/* this will run DPC priority */
	__aft_port_task (card_ptr);
#endif
}
#endif


#if defined(__LINUX__)
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
static void aft_port_task (void * card_ptr)
# else
static void aft_port_task (struct work_struct *work)
# endif
#elif defined(__WINDOWS__)
/* running in a low priority thread */
void __aft_port_task (IN PVOID card_ptr)
#else
static void aft_port_task (void * card_ptr, int arg)
#endif
{
#if defined(__LINUX__)
# if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
        sdla_t 		*card = (sdla_t *)container_of(work, sdla_t, u.aft.port_task);
# else
	sdla_t 		*card = (sdla_t *)card_ptr;
# endif
#else
	sdla_t 		*card = (sdla_t *)card_ptr;
#endif
	wan_smp_flag_t smp_flags;
	wan_smp_flag_t smp_irq_flags;

	wan_set_bit(CARD_PORT_TASK_RUNNING,&card->wandev.critical);

	if (wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
		wan_clear_bit(CARD_PORT_TASK_RUNNING,&card->wandev.critical);
		return;
	}
	
	if (wan_test_bit(AFT_CRITICAL_DOWN, &card->u.aft.port_task_cmd)) {
		aft_critical_shutdown(card);
		wan_clear_bit(CARD_PORT_TASK_RUNNING,&card->wandev.critical);
		return;
	}

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		wan_clear_bit(CARD_PORT_TASK_RUNNING,&card->wandev.critical);
		return;
	}


	DEBUG_TEST("%s: PORT TASK: 0x%X\n", card->devname,card->u.aft.port_task_cmd);

#ifdef AFT_IRQ_STAT_DEBUG
      	card->wandev.stats.rx_missed_errors++;
#endif

	if (wan_test_bit(AFT_FE_INTR,&card->u.aft.port_task_cmd)){

		DEBUG_TEST("%s: PORT TASK: FE INTER\n", card->devname);

		card->hw_iface.hw_lock(card->hw,&smp_flags);
		aft_fe_intr_ctrl(card, 0);
		front_end_interrupt(card,0,1);


		wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
		wan_clear_bit(AFT_FE_INTR,&card->u.aft.port_task_cmd);
		__aft_fe_intr_ctrl(card, 1);
		wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);

		card->hw_iface.hw_unlock(card->hw,&smp_flags);

	}

	if (wan_test_bit(AFT_FE_POLL,&card->u.aft.port_task_cmd)){

		DEBUG_TEST("%s: PORT TASK: FE POLL\n", card->devname);
		card->hw_iface.hw_lock(card->hw,&smp_flags);

		if (card->wandev.fe_iface.polling){
			card->wandev.fe_iface.polling(&card->fe);
		}

		handle_front_end_state(card,1);

		wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
		wan_clear_bit(AFT_FE_POLL,&card->u.aft.port_task_cmd);
		wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);

		card->hw_iface.hw_unlock(card->hw,&smp_flags);
	}

	if (wan_test_bit(AFT_FE_LED,&card->u.aft.port_task_cmd)){
		DEBUG_TEST("%s: PORT TASK: FE LED\n", card->devname);
		card->hw_iface.hw_lock(card->hw,&smp_flags);
		if (card->wandev.state == WAN_CONNECTED){
			aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_RED, 0,WAN_AFT_OFF);
			aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_GREEN, 0, WAN_AFT_ON);
		}else{
			aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_RED, 0,WAN_AFT_ON);
			aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_GREEN, 0, WAN_AFT_OFF);
		}

		wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
		wan_clear_bit(AFT_FE_LED,&card->u.aft.port_task_cmd);
		wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);

		card->hw_iface.hw_unlock(card->hw,&smp_flags);
	}


	if (wan_test_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd)){
		int	err;
		DEBUG_TEST("%s: PORT TASK: FE RBS\n", card->devname);
		card->hw_iface.hw_lock(card->hw,&smp_flags);
		err=0;

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
		if (card->wan_tdmv.sc) {
			WAN_TDMV_CALL(rbsbits_poll, (&card->wan_tdmv, card), err);
		}
#endif

#ifdef AFT_TDM_API_SUPPORT
		if (card->tdm_api_dev) {
			wanpipe_tdm_api_rbsbits_poll(card);
		}
#endif

		wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
		wan_clear_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
		wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);

		card->hw_iface.hw_unlock(card->hw,&smp_flags);
	}

#if defined(CONFIG_WANPIPE_HWEC)
	if (wan_test_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd)){
		DEBUG_TEST("%s: PORT TASK: FE EC INTR\n", card->devname);
		if (card->wandev.ec_dev){
			int	pending = 0;
			pending = wanpipe_ec_poll(card->wandev.ec_dev, card);
			/* Aug 10, 2007
			** If EC poll return 1 (pending), re-schedule port_task again
			** (more dtmf events are available) */
			if (pending == 1){
				WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
			}else{
				wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
				wan_clear_bit(AFT_FE_EC_POLL,&card->u.aft.port_task_cmd);
				wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);
			}
		}
	}
#endif

	if (wan_test_bit(AFT_FE_RESTART,&card->u.aft.port_task_cmd)){

		wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
		wan_clear_bit(AFT_FE_RESTART,&card->u.aft.port_task_cmd);
		wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);

		if (IS_BRI_CARD(card)) {

			DEBUG_EVENT("%s: BRI IRQ Restart\n",
                                       card->devname);

			card->hw_iface.hw_lock(card->hw,&smp_flags);
			wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
			wan_clear_bit(0,&card->u.aft.comm_enabled);
			enable_data_error_intr(card);
			wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);
			card->hw_iface.hw_unlock(card->hw,&smp_flags);

		} else if (card->fe.fe_status == FE_CONNECTED) {

			DEBUG_EVENT("%s: TDM IRQ Restart\n",
							card->devname);

			card->hw_iface.hw_lock(card->hw,&smp_flags);
		
			card->fe.fe_status = FE_DISCONNECTED;
			handle_front_end_state(card,1);

			/* BRI has special behaviour, we need to make sure
                         * that restart will trigger interrupts */
			wan_clear_bit(0,&card->u.aft.comm_enabled);

			card->fe.fe_status = FE_CONNECTED;
			handle_front_end_state(card,1);
	
			card->hw_iface.hw_unlock(card->hw,&smp_flags);
		}
	}


#if defined(AFT_RTP_SUPPORT)
	if (wan_test_bit(AFT_RTP_TAP_Q,&card->u.aft.port_task_cmd)){
		netskb_t *skb;
		wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
		wan_clear_bit(AFT_FE_RESTART,&card->u.aft.port_task_cmd);
		wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);
		while ((skb=wan_skb_dequeue(&card->u.aft.rtp_tap_list))) {
			dev_queue_xmit(skb);
		}
	}
#endif

	if (wan_test_bit(AFT_SERIAL_STATUS,&card->u.aft.port_task_cmd)){
		wan_spin_lock_irq(&card->wandev.lock,&smp_irq_flags);
		wan_clear_bit(AFT_SERIAL_STATUS,&card->u.aft.port_task_cmd);
		wan_spin_unlock_irq(&card->wandev.lock,&smp_irq_flags);
		aft_core_send_serial_oob_msg(card);
	}


	wan_clear_bit(CARD_PORT_TASK_RUNNING,&card->wandev.critical);

	if (wan_test_bit(CARD_DOWN,&card->wandev.critical)){
		return;	
	}
	
	/* If there are more commands pending requeue the task */
	if (card->u.aft.port_task_cmd) {
		WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));	
	}
	return;

}
 
#ifdef AFT_FE_INTR_DEBUG
void ___aft_fe_intr_ctrl(sdla_t *card, int status, char *func, int line)
#else
void __aft_fe_intr_ctrl(sdla_t *card, int status)
#endif
{
	u32 reg;
	int retry=5;

	/* if fe_no_intr is set then only allow disabling of fe interrupt */
#ifdef AFT_FE_INTR_DEBUG
	DEBUG_EVENT("%s:%d: __aft_fe_intr_ctrl  card=%s  status=%i\n",
				func,line,card->devname,status);
#endif

	if (!card->fe_no_intr || !status){
		card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card, AFT_CHIP_CFG_REG),&reg);
		if (status){
			card->front_end_irq_timeout=0;
			wan_set_bit(AFT_CHIPCFG_FE_INTR_CFG_BIT,&reg);
		}else{
			if (!card->fe_no_intr) {
				/* Only start the front end irq timeout if fe interrupts
				are enabled */
				card->front_end_irq_timeout=SYSTEM_TICKS;
			}
			wan_clear_bit(AFT_CHIPCFG_FE_INTR_CFG_BIT,&reg);
		}

		do {
			card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card, AFT_CHIP_CFG_REG),reg);

			if (status) {
				u32 treg;
				card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card, AFT_CHIP_CFG_REG),&treg);
				if (wan_test_bit(AFT_CHIPCFG_FE_INTR_CFG_BIT,&treg)) {
					break;
				}
				DEBUG_EVENT("%s: Warning failed to enable front end ... retry %i!\n",card->devname,retry);
			}
		}while(--retry);
	}
}

#ifdef AFT_FE_INTR_DEBUG
  /* Defined in aft_core_private.h */
#else
void aft_fe_intr_ctrl(sdla_t *card, int status)
{
	wan_smp_flag_t smp_flags;

	wan_spin_lock_irq(&card->wandev.lock,&smp_flags);
	__aft_fe_intr_ctrl(card,status);
	wan_spin_unlock_irq(&card->wandev.lock,&smp_flags);

}
#endif

static void aft_data_mux_cfg(sdla_t *card)
{
	if (!card->u.aft.cfg.data_mux_map){
		card->u.aft.cfg.data_mux_map=AFT_DEFAULT_DATA_MUX_MAP;
	}
	
	card->hw_iface.bus_write_4(card->hw, AFT_PORT_REG(card,AFT_DATA_MUX_REG), 
				   card->u.aft.cfg.data_mux_map);		

	DEBUG_EVENT("%s: AFT Data Mux Bit Map: 0x%08X\n",
		card->devname,card->u.aft.cfg.data_mux_map);
}

static void aft_data_mux_get_cfg(sdla_t *card)
{
	card->hw_iface.bus_read_4(card->hw, 
				  AFT_PORT_REG(card,AFT_DATA_MUX_REG), 
				  &card->u.aft.cfg.data_mux_map);		

}

static int aft_hdlc_repeat_mangle(sdla_t *card,private_area_t *chan, netskb_t *skb, netskb_t **rskb)
{
	unsigned char *buf;
	wp_api_hdr_t *tx_hdr = (wp_api_hdr_t *)wan_skb_data(skb);

	if (tx_hdr->wp_api_tx_hdr_hdlc_rpt_len == 0 || tx_hdr->wp_api_tx_hdr_hdlc_rpt_len > 8) {
		return -1;
	}

	*rskb=wan_skb_alloc(128);
	if (!rskb) {
		return -1;
	}

	buf=wan_skb_put(*rskb,tx_hdr->wp_api_tx_hdr_hdlc_rpt_len);
	memcpy(buf,tx_hdr->wp_api_tx_hdr_hdlc_rpt_data,tx_hdr->wp_api_tx_hdr_hdlc_rpt_len);
	wan_skb_pull(skb,sizeof(wp_api_hdr_t));

	return 0;
}

static int aft_ss7_tx_mangle(sdla_t *card,private_area_t *chan, netskb_t *skb)
{
	int ss7_len=0,len=0;
	unsigned int ss7_ctrl=0;
	unsigned char *buf;
	wp_api_hdr_t *tx_hdr = &chan->tx_api_hdr;

	memcpy(&chan->tx_api_hdr,wan_skb_data(skb),sizeof(wp_api_hdr_t));
	wan_skb_pull(skb,sizeof(wp_api_hdr_t));

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

	if (tx_hdr->wp_api_tx_hdr_aft_ss7_type == WANOPT_SS7_FISU){
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

	if (tx_hdr->wp_api_tx_hdr_aft_ss7_force_tx){
		wan_set_bit(AFT_SS7_CTRL_FORCE_BIT,&ss7_ctrl);
	}else{
		wan_clear_bit(AFT_SS7_CTRL_FORCE_BIT,&ss7_ctrl);
	}

	DEBUG_TEST("%s: SS7 DATA = 0x%X 0x%X 0x%X\n",
			chan->if_name,
			tx_hdr->wp_api_tx_hdr_aft_ss7_data[0],
			tx_hdr->wp_api_tx_hdr_aft_ss7_data[1],
			tx_hdr->wp_api_tx_hdr_aft_ss7_data[2]);

	memcpy(&chan->tx_ss7_realign_buf[len],tx_hdr->wp_api_tx_hdr_aft_ss7_data,ss7_len);

	len+=ss7_len;

	wan_skb_init(skb,0);
	wan_skb_trim(skb,0);

	if (wan_skb_tailroom(skb) < len){
		DEBUG_EVENT("%s: Critical error: SS7 Tx unalign pkt tail room(%d) < len(%d)!\n",
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

#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
        DEBUG_EVENT("%s:    TDMV Dummy     = %s\n",
			card->devname,
			(conf->tdmv_conf.sdla_tdmv_dummy_enable == WANOPT_YES)? "Enabled":"Disabled");


	/* Initialize sdla_tdmv_dummy field */
	card->sdla_tdmv_dummy = NULL;

	/* If SDLA TDMV DUMMY option is enabled, register for zaptel timing */
	if (conf->tdmv_conf.sdla_tdmv_dummy_enable == WANOPT_YES) {
		int used_cnt;
		card->hw_iface.getcfg(card->hw, SDLA_HWCPU_USEDCNT, &used_cnt);
		if (used_cnt > 1) {
			DEBUG_EVENT("%s: Error: TDMV Dummy must be configured for first TDM SPAN device\n",
					card->devname);
			return -EINVAL;
		}

		card->sdla_tdmv_dummy = sdla_tdmv_dummy_register();

		if (card->sdla_tdmv_dummy == NULL) {
			DEBUG_EVENT("%s: Failed to register sdla_tdmv_dummy\n", card->devname);
			return -EINVAL;
		}
	}
#else
	if (conf->tdmv_conf.sdla_tdmv_dummy_enable == WANOPT_YES) {
		DEBUG_EVENT("%s: TDMV Dummy support not compiled in \n", card->devname);
		return -EINVAL;
	}

#endif


	if (card->u.aft.global_tdm_irq) {
		if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
			if (IS_A600_CARD(card)) {
				valid_firmware_ver=AFT_MIN_A600_FRMW_VER;
			} else {
				valid_firmware_ver=AFT_MIN_ANALOG_FRMW_VER;
			}
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
	if (card->u.aft.global_tdm_irq && card->wan_tdmv.sc){
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

	if (!card->u.aft.global_tdm_irq){
		DEBUG_EVENT("%s: Error: TDMV Span No is not set!\n",
				card->devname);
		return -EINVAL;
	}

	if (chan->cfg.tdmv_master_if){
		DEBUG_EVENT("%s: Configuring TDMV Master dev %s\n",
				card->devname,chan->if_name);
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

			/* If AFT DUMMY is enabled all devices must be configured
                         * with same MTU */

			if (aft_tdmapi_mtu_check(card, &chan->mtu) != 0){
				switch (chan->mtu) {
				case 8:
				case 16:
					break;
				case 40:
				case 80:
					if (!wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
						/* If Global TDM Feature is not enable
						then 40 and 80 bytes TDM are not available */
						chan->mtu=8;
					}
					break;
				default:
					if (card->wandev.config_id == WANCONFIG_AFT_ANALOG) {
						chan->mtu=8;
					} else if (wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
						chan->mtu=80;
					} else {
						chan->mtu=8;
					}
					break;
				}
			}
		}

		chan->mru = chan->mtu;
		card->u.aft.tdmv_mtu = chan->mtu;

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

		if(IS_BRI_CARD(card)){
			if(chan->dchan_time_slot >= 0){
				conf->active_ch = 0x4<<(WAN_FE_LINENO(&card->fe)*2);
				/* For the d-chan MUST set ONLY bit 2!! */
			}
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
	wan_dma_descr_t	*tx_dma_chain;
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
	 		WAN_TDMV_CALL(buf_rotate, (card,AFT_TDMV_CIRC_BUF,AFT_TDMV_BUF_MASK,AFT_TDMV_CIRC_BUF_LEN), err);
		}

		if (!card->wandev.ec_enable || card->wandev.ec_enable_map == 0){
			WAN_TDMV_CALL(ec_span, (card), err);
		}

		WAN_TDMV_CALL(rx_tx_span, (card), err);

		WAN_TDMV_CALL(is_rbsbits, (&card->wan_tdmv), err);
		if (err == 1){
			if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
				wan_set_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
				WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
			}
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

	wan_dma_descr_t *tx_dma_chain;
	wan_dma_descr_t *rx_dma_chain;

	u8 	*txbuf, *rxbuf;

	tx_dma_chain = &chan->tx_dma_chain_table[0];
	rx_dma_chain = &chan->rx_dma_chain_table[0];

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


	rxbuf = (unsigned char*)rx_dma_chain->dma_virt+rx_offset;
	txbuf =	(unsigned char*)tx_dma_chain->dma_virt+tx_offset;

	if (wan_test_bit(AFT_TDM_RING_BUF,&card->u.aft.chip_cfg_status)) {

		rx_offset= AFT_TDMV_CIRC_BUF * card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num];
		tx_offset= AFT_TDMV_CIRC_BUF * card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num];


		card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]++;
		if (card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num] >= AFT_TDMV_CIRC_BUF_LEN) {
			card->u.aft.tdm_rx_dma_toggle[chan->logic_ch_num]=0;
		}
		card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]++;
		if (card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num] >= AFT_TDMV_CIRC_BUF_LEN) {
			card->u.aft.tdm_tx_dma_toggle[chan->logic_ch_num]=0;
		}

		rxbuf = (unsigned char*)rx_dma_chain->dma_virt+rx_offset;
		txbuf =	(unsigned char*)tx_dma_chain->dma_virt+tx_offset;

	} else if (wan_test_bit(AFT_TDM_SW_RING_BUF,&card->u.aft.chip_cfg_status)) {

		chan->swring.rx_toggle = (chan->swring.rx_toggle + 1) %  AFT_DMA_RING_MAX;
		memcpy(chan->swring.rbuf[chan->swring.rx_toggle].rxdata,
		       rxbuf,
		       chan->mtu);
		rxbuf= chan->swring.rbuf[chan->swring.rx_toggle].rxdata;


		memcpy(txbuf, chan->swring.rbuf[chan->swring.tx_toggle].txdata,
		       chan->mtu);
		chan->swring.tx_toggle = (chan->swring.tx_toggle + 1) %  AFT_DMA_RING_MAX;
		txbuf = chan->swring.rbuf[chan->swring.tx_toggle].txdata;

	}


	err=0;


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
			wan_dma_descr_t *tx_bh_dma_chain = &chan->tx_dma_chain_table[1];
			wan_dma_descr_t *rx_bh_dma_chain = &chan->rx_dma_chain_table[1];

			if (!rx_bh_dma_chain->skb){
				rx_bh_dma_chain->skb=wan_skb_dequeue(&chan->wp_rx_free_list);
				if (!rx_bh_dma_chain->skb){
					if (WAN_NET_RATELIMIT()){
						DEBUG_EVENT("%s: Critical TDM BH no free skb\n",
							chan->if_name);
						goto aft_tdm_bh_skip;
					}
				}
				wan_skb_init(rx_bh_dma_chain->skb,sizeof(wp_api_hdr_t));
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
				wan_skb_init(tx_bh_dma_chain->skb,sizeof(wp_api_hdr_t));
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

			DEBUG_TEST ("%s: Calling Rx Chan=%d TdmvChan=%d\n",
					card->devname,chan->logic_ch_num,
					chan->tdmv_chan);

			if (card->wandev.rtp_len && card->wandev.rtp_tap) {
                                card->wandev.rtp_tap(card,
						     IS_T1_CARD(card) ? chan->first_time_slot :
						     			chan->first_time_slot-1,
						     rxbuf,
						     txbuf,
                                                     chan->mtu);
                        }

#if 1
			WAN_TDMV_CALL(rx_chan,
					(&card->wan_tdmv,chan->tdmv_chan,
					rxbuf,
					txbuf),
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

		if (card->wp_debug_chan_seq) {
			private_area_t *top_chan=wan_netif_priv(chan->common.dev);
			
			if (memcmp(txbuf,rxbuf,chan->mtu) != 0) {
				WAN_NETIF_STATS_INC_RX_ERRORS(&top_chan->common);
				top_chan->common.if_stats.rx_dropped=chan->logic_ch_num;
				top_chan->common.if_stats.rx_frame_errors=rxbuf[0];
			}
		}

		wanpipe_tdm_api_rx_tx(&chan->wp_tdm_api_dev,
			    rxbuf,
			    txbuf,
			    chan->mtu);

		if (card->wp_debug_chan_seq) {
			memset(txbuf,(unsigned char)chan->logic_ch_num,chan->mtu);
		}

#endif
	}

	if (chan->cfg.tdmv_master_if){
		u32	reg;
		int	err;
		err=0;
#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
		if (chan->tdmv_zaptel_cfg){
			DEBUG_TEST ("%s: Calling Master Rx Tx Chan=%d\n",
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

			if (wan_test_bit(AFT_TDM_SW_RING_BUF,&card->u.aft.chip_cfg_status)) {
				if (!wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
					card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
					wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&reg);
					wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&reg);
					card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
				}
			}

#if 1
			card->hw_iface.busdma_sync(
						card->hw,
						&chan->tx_dma_chain_table[0],
						MAX_AFT_DMA_CHAINS,
						(chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE),
						SDLA_DMA_POSTREAD);

			WAN_TDMV_CALL(rx_tx_span, (card), err);

			card->hw_iface.busdma_sync(
						card->hw,
						&chan->tx_dma_chain_table[0],
						MAX_AFT_DMA_CHAINS,
						(chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE),
						SDLA_DMA_PREWRITE);
			card->hw_iface.busdma_sync(
						card->hw,
						&chan->rx_dma_chain_table[0],
						MAX_AFT_DMA_CHAINS,
						(chan->dma_chain_opmode == WAN_AFT_DMA_CHAIN_SINGLE),
						SDLA_DMA_PREREAD);

#else
#warning "NCDEBUG: rx_tx_span disabled irq"
#endif

			if (!wan_test_bit(AFT_TDM_SW_RING_BUF,&card->u.aft.chip_cfg_status) &&
         		    !wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
				card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
				wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&reg);
				wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&reg);
				card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
			}

			if (card->wan_tdmv.sc){
				WAN_TDMV_CALL(is_rbsbits, (&card->wan_tdmv), err);
				if (err == 1){
					if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
						wan_set_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
						WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
					}
				}
			}
#endif
		}else{
#else
		if (!chan->tdmv_zaptel_cfg){

#endif

#ifdef CONFIG_PRODUCT_WANPIPE_TDM_VOICE
			if (card->sdla_tdmv_dummy) {
				err = sdla_tdmv_dummy_tick(card->sdla_tdmv_dummy);
			}
#endif


			if (!wan_test_bit(AFT_TDM_GLOBAL_ISR,&card->u.aft.chip_cfg_status)) {
				card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),&reg);
				wan_set_bit(AFT_DMACTRL_TDMV_RX_TOGGLE,&reg);
				wan_set_bit(AFT_DMACTRL_TDMV_TX_TOGGLE,&reg);
				card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_DMA_CTRL_REG),reg);
			}

			if (card->wandev.fe_iface.watchdog) {
				err = card->wandev.fe_iface.watchdog(card);
			}

			/* FIXME: Make this more abstract */
			err=wanpipe_tdm_api_is_rbsbits(card);
			if (err == 1) {
				if (!wan_test_bit(CARD_PORT_TASK_DOWN,&card->wandev.critical)){
					wan_set_bit(AFT_FE_TDM_RBS,&card->u.aft.port_task_cmd);
					WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));
				}
			}

		}

		DEBUG_TEST("%s: Master device tx rx %d!\n",
				card->devname,chan->logic_ch_num);
	}

	if (card->wandev.state == WAN_CONNECTED) {
		chan->opstats.Data_frames_Rx_count++;
		chan->opstats.Data_bytes_Rx_count+=chan->mru;
		chan->opstats.Data_frames_Tx_count++;
		chan->opstats.Data_bytes_Tx_count+=chan->mtu;
		chan->chan_stats.rx_packets++;
		chan->chan_stats.rx_bytes += chan->mru;
		chan->chan_stats.tx_packets++;
		chan->chan_stats.tx_bytes += chan->mtu;
		WAN_NETIF_STATS_INC_RX_PACKETS(&chan->common);	//chan->if_stats.rx_packets++;
		WAN_NETIF_STATS_INC_RX_BYTES(&chan->common,chan->mru);	//chan->if_stats.rx_bytes += chan->mru;
		WAN_NETIF_STATS_INC_TX_PACKETS(&chan->common);	//chan->if_stats.tx_packets++;
		WAN_NETIF_STATS_INC_TX_BYTES(&chan->common,chan->mtu);	//chan->if_stats.tx_bytes += chan->mtu;
	}

	return 0;
}


static void aft_critical_trigger(sdla_t *card)
{
	disable_data_error_intr(card,CRITICAL_DOWN);

	wan_set_bit(AFT_CRITICAL_DOWN,&card->u.aft.port_task_cmd);
	WAN_TASKQ_SCHEDULE((&card->u.aft.port_task));

	return;
}

static void aft_critical_shutdown (sdla_t *card)
{
	wan_smp_flag_t smp_flags,smp_flags1;

	DEBUG_EVENT("%s: Error: Card Critically Shutdown!\n",
			card->devname);

	if (card->wandev.fe_iface.pre_release){
		card->wandev.fe_iface.pre_release(&card->fe);
	}

	card->hw_iface.hw_lock(card->hw,&smp_flags1);
	wan_spin_lock_irq(&card->wandev.lock, &smp_flags);

	if (card->wandev.fe_iface.disable_irq){
		card->wandev.fe_iface.disable_irq(&card->fe);
	}

	if (card->wandev.fe_iface.unconfig){
		card->wandev.fe_iface.unconfig(&card->fe);
	}
	wan_spin_unlock_irq(&card->wandev.lock, &smp_flags);
	card->hw_iface.hw_unlock(card->hw,&smp_flags1);

	disable_data_error_intr(card,CRITICAL_DOWN);
	wan_set_bit(CARD_DOWN,&card->wandev.critical);
	port_set_state(card,WAN_DISCONNECTED);

	if (IS_BRI_CARD(card)) {
		void **card_list=__sdla_get_ptr_isr_array(card->hw);
		sdla_t *tmp_card;
		int i;
		for (i=0;i<MAX_BRI_LINES;i++) {
			tmp_card=(sdla_t*)card_list[i];
			if (tmp_card &&
			    !wan_test_bit(CARD_DOWN,&tmp_card->wandev.critical)) {

				if (tmp_card->wandev.fe_iface.pre_release){
					tmp_card->wandev.fe_iface.pre_release(&tmp_card->fe);
				}

				if (tmp_card->wandev.fe_iface.disable_irq){
					tmp_card->wandev.fe_iface.disable_irq(&tmp_card->fe);
				}

				if (tmp_card->wandev.fe_iface.unconfig){
					tmp_card->wandev.fe_iface.unconfig(&tmp_card->fe);
				}

				disable_data_error_intr(tmp_card,CRITICAL_DOWN);
				wan_set_bit(CARD_DOWN,&tmp_card->wandev.critical);
				port_set_state(tmp_card,WAN_DISCONNECTED);
			}
		}
	}

	aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_RED, 0,WAN_AFT_ON);
	aft_hwdev[card->wandev.card_type].aft_led_ctrl(card, WAN_AFT_GREEN, 0, WAN_AFT_OFF);
			
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

	return aft_hwdev[card->wandev.card_type].aft_chip_config(card, conf);
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
		DEBUG_EVENT("%s: Invalid Channel Selection 0x%X\n",
				card->devname,chan->time_slot_map);
		return -EINVAL;
	}


	DEBUG_EVENT("%s:    Active Ch Map :0x%08X\n",
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



/****** End ****************************************************************/
