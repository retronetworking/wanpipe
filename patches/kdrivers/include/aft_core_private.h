
/* aft_core_private.h */

#ifndef _AFT_CORE_PRIVATE_H
#define _AFT_CORE_PRIVATE_H


#ifdef WAN_KERNEL

# include "aft_core_options.h"
# include "if_wanpipe_common.h"	/* wanpipe_common_t */
# include "aft_core_user.h"		/* aft_op_stats_t */
# include "wanpipe_tdm_api.h"	/* wanpipe_tdm_api_dev_t */
# include "aft_core_bert.h"		/* wp_bert_t */

#if defined(__WINDOWS__)
# include "sdladrv_private.h"
# include "wanpipe_cdev_iface.h" /* wanpipe_cdev_ops_t */
#endif

/*=================================================================
 * Defines
 *================================================================*/

#define AFT_MIN_FRMW_VER 0x11
#define AFT_TDMV_FRM_VER 0x11
#define AFT_TDMV_FRM_CLK_SYNC_VER 0x14
#define AFT_TDMV_SHARK_FRM_CLK_SYNC_VER 0x17
#define AFT_TDMV_SHARK_A108_FRM_CLK_SYNC_VER 0x25
#define AFT_56K_MIN_FRMW_VER	0x00
#define AFT_SERIAL_MIN_FRMW_VER	0x04

#define AFT_MIN_ANALOG_FRMW_VER 0x05
#define AFT_MIN_A600_FRMW_VER 	0x01
#define AFT_MIN_B601_FRMW_VER  0x03

#define A500_MAX_EC_CHANS 64



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


#define MAX_IP_ERRORS	10

#define PORT(x)   (x == 0 ? "PRIMARY" : "SECONDARY" )

/* Route Status options */
#define NO_ROUTE	0x00
#define ADD_ROUTE	0x01
#define ROUTE_ADDED	0x02
#define REMOVE_ROUTE	0x03

#define WP_WAIT 	0
#define WP_NO_WAIT	1


/*=================================================================
 * Enum  Defines
 *================================================================*/


enum {
        TDM_RUNNING,
	TDM_PENDING,
};

/* Private critical flags */
enum {
	POLL_CRIT = PRIV_CRIT,
	CARD_DOWN,
	TE_CFG,
	CARD_HW_EC,
	CARD_MASTER_CLOCK,
	CARD_PORT_TASK_DOWN,
	CARD_PORT_TASK_RUNNING
};

enum { 
	LINK_DOWN = 0,
	DEVICE_DOWN,
	CRITICAL_DOWN
};


enum {
	TX_DMA_BUSY = 0,
	TX_HANDLER_BUSY,
	TX_INTR_PENDING,

	RX_HANDLER_BUSY,
	RX_DMA_BUSY,
	RX_INTR_PENDING
};

enum {
	AFT_FE_CFG_ERR = 0,
	AFT_FE_CFG,
	AFT_FE_INTR,
	AFT_FE_POLL,
	AFT_FE_TDM_RBS,
	AFT_FE_LED,
	AFT_FE_EC_POLL,
	AFT_FE_RESTART,
	AFT_RTP_TAP_Q,
	AFT_SERIAL_STATUS,
	AFT_CRITICAL_DOWN
};


enum {
	MASTER_CLOCK_CHECK = 0,
	MASTER_CLOCK_SET
};



enum {
	TX_DMA_BUF_INIT =0,		
	TX_DMA_BUF_USED
};

enum {
	WP_DEV_CONFIG = 0,
	WP_DEV_UP,
};


enum {
	WAN_AFT_DMA_CHAIN = 0,
	WAN_AFT_DMA_CHAIN_IRQ_ALL,
	WAN_AFT_DMA_CHAIN_SINGLE
};

enum {
	AFT_BG_TIMER_RUNNING,
	AFT_BG_TIMER_KILL,

	AFT_BG_TIMER_CMD_NONE
};

/*=================================================================
 * Private structures
 *================================================================*/


#if 0
typedef struct aft_dma_chain
{
	unsigned long	init;
	sdla_dma_addr_t	dma_addr;
	u32		dma_len;
	u32		dma_map_len;
	netskb_t 	*skb;
	u32		index;

	u32		dma_descr;
	u32		len_align;
	u32		reg;

	u8		pkt_error;
	void*		dma_virt;
	u32		dma_offset;
	u32		dma_toggle;
#if defined(__FreeBSD__)
	bus_dma_tag_t	dma_tag;
	bus_dmamap_t	dmamap;
	int		dma_ready;
#endif
}aft_dma_chain_t;
#endif


typedef struct wp_rx_element
{
	unsigned int dma_addr;
	unsigned int reg;
	unsigned int align;
	unsigned short  len;
	unsigned short  pkt_error;
}wp_rx_element_t;


typedef struct aft_config
{
	unsigned int aft_chip_cfg_reg;
	unsigned int aft_dma_control_reg; 
}aft_config_t;


static __inline u32 AFT_PORT_REG(sdla_t *card, u32 reg)
{
        if (card->adptr_type == AFT_ADPTR_A600 || 
			card->adptr_type == AFT_ADPTR_B601) {
        //A600 CASE
                if (reg < 0x100) {
                        return (reg+0x1000);
                } else {
                        return (reg+0x2000)+(0x8000*card->wandev.comm_port);
                }
        } else {
                if (reg < 0x100) {
                        return reg;
                }
                return  (reg+(0x4000*card->wandev.comm_port));
        }
}

typedef struct dma_history{
	u8	end;
	u8	cur;
	u8	begin;
	u8	status;
	u8 	loc;
}dma_history_t;

#define MAX_DMA_HIST_SIZE 	10
#define MAX_AFT_DMA_CHAINS 	16
#define MAX_TX_BUF		MAX_AFT_DMA_CHAINS*2+1
#define MAX_RX_BUF		MAX_AFT_DMA_CHAINS*4+1
#define AFT_DMA_INDEX_OFFSET	0x200


typedef struct aft_dma_ring
{
	unsigned char rxdata[128];
	unsigned char txdata[128];
}aft_dma_ring_t;

#define AFT_DMA_RING_MAX 4

typedef struct aft_dma_swring {
	int tx_toggle;
	int rx_toggle;
	aft_dma_ring_t  rbuf[AFT_DMA_RING_MAX];
}aft_dma_swring_t;


/* List of Maintenance modes for 'maintenance_mode_bitmap'
 * in 'private_area_t'. 
 * Currently only BERT is implemented. */
enum wp_maintenance_modes {
	WP_MAINTENANCE_MODE_BERT=0
};



typedef struct private_area
{
	wanpipe_common_t 	common;/* MUST be at the top */
	sdla_t				*card;
	u32 				busy;

	/************************************//**
     High priority - DMA Stuff
     ****************************************/
	aft_dma_swring_t 	swring;

#if defined(__LINUX__) || defined(__WINDOWS__)
	wanpipe_tdm_api_dev_t	*wp_tdm_api_dev;
#endif

	u32					dma_status;
	unsigned char		hdlc_eng;
	unsigned char		tx_chain_indx,tx_pending_chain_indx,tx_chain_data_sz,tx_chain_sz;
	wan_dma_descr_t 	tx_dma_chain_table[MAX_AFT_DMA_CHAINS];

	unsigned char		rx_chain_indx,rx_pending_chain_indx,rx_chain_sz;
	wan_dma_descr_t 	rx_dma_chain_table[MAX_AFT_DMA_CHAINS];

	wan_skb_queue_t 	wp_tx_pending_list;
	wan_skb_queue_t 	wp_tx_complete_list;
	wan_skb_queue_t		wp_tx_hdlc_rpt_list;
	netskb_t 			*tx_dma_skb;
	u8					tx_dma_cnt;

	wan_skb_queue_t 	wp_rx_free_list;
	wan_skb_queue_t 	wp_rx_complete_list;

	wan_skb_queue_t 	wp_rx_stack_complete_list;
	wan_skb_queue_t 	wp_rx_bri_dchan_complete_list;

	wan_skb_queue_t		wp_dealloc_list;

	u32	 				time_slot_map;
	unsigned char 		num_of_time_slots;
	int          		logic_ch_num;

	unsigned char 		interface_down;
	unsigned char		channelized_cfg;
	unsigned char		tdmv_zaptel_cfg;

#if defined(__FreeBSD__)
//	int		dma_ready;
	bus_dma_tag_t		dma_rx_mtag;
	bus_dma_tag_t		dma_tx_mtag;
#endif

	unsigned char 		dma_chain_opmode;

	wp_tdm_chan_stats_t	chan_stats;

	/************************************//**
     Medium priority - Operational Stuff
     ****************************************/

	/* Polling task queue. Each interface	
	* has its own task queue, which is used
	* to defer events from the interrupt */
	wan_taskq_t 		poll_task;
	wan_timer_info_t 	poll_delay_timer;

	u8 					gateway;
	u8 					true_if_encoding;

	u8					idle_flag;
	u16					max_idle_size;
	u8					idle_start;

	u8					pkt_error;
	u8					rx_fifo_err_cnt;

	int					first_time_slot;
	int					last_time_slot;
	
	netskb_t			*tx_idle_skb;
	netskb_t			*tx_hdlc_rpt_skb;

	unsigned char		rx_dma;
	unsigned char   	pci_retry;
	
	unsigned char		fifo_size_code;
	unsigned char		fifo_base_addr;
	unsigned char 		fifo_size;

	int					dma_mru;
	int					mru,mtu;

	void 				*prot_ch;
	int	 				prot_state;

	wan_trace_t			trace_info;

	int					rx_no_data_cnt;

	u32					dma_chain_status;
	u32 				up;
	int					tx_attempts;
	
	unsigned char   	*tx_realign_buf;

	unsigned char		tslot_sync;

	
	unsigned int		dma_index;

	/* Used by ss7 mangle code */
	wp_api_hdr_t 		tx_api_hdr;
	unsigned char   	*tx_ss7_realign_buf;

	int					tdmv_chan;
	unsigned int		tdmv_irq_cfg;

	unsigned int		tdmv_rx_delay;
	unsigned char		tdmv_rx_delay_cfg;
	unsigned short		max_tx_bufs;
	unsigned short		max_tx_bufs_orig;

	unsigned int		ss7_force_rx;
	
	unsigned char		lip_atm;



	int					dchan_time_slot;
	int 				xmtp2_api_index;

	netdevice_t 		*annexg_dev;
	unsigned char 		label[WAN_IF_LABEL_SZ+1];

	unsigned char 		*udp_pkt_data;
	atomic_t 			udp_pkt_len;

	struct private_area *next;

	/************************************//**
     Low priority - Config Stuff
     ****************************************/


	int					rx_api_crc_bytes;
	unsigned char 		wp_api_op_mode;
	unsigned char 		wp_api_iface_mode;

	unsigned int  		tdm_api_period;
	unsigned int  		tdm_api_chunk;
	
	unsigned char		protocol;
	unsigned short 		dma_per_ch;

	int 				tracing_enabled;	/* For enabling Tracing */
	wan_time_t			router_start_time;	/*unsigned long 	router_start_time;*/
	wan_ticks_t			trace_timeout;

	unsigned long 		tick_counter;		/* For 5s timeout counter */
	wan_time_t			router_up_time;		/*unsigned long router_up_time;*/
	unsigned char  		mc;			/* Mulitcast support on/off */

	int						if_cnt;
	char 					if_name[WAN_IFNAME_SZ+1];
	aft_op_stats_t  		opstats;
	aft_comm_err_stats_t	errstats;

	wan_xilinx_conf_if_t 	cfg;
	unsigned char			usedby_cfg;
	unsigned int			cfg_active_ch;

	unsigned char			rx_seq_char;
	unsigned char			tx_seq_char;
	

#if defined(__LINUX__)
	struct timeval			timing_tv;
	/* Entry in proc fs per each interface */
	struct proc_dir_entry	*dent;
#endif	

#ifdef AFT_DMA_HISTORY_DEBUG
#warning "DMA History Enabled"
	dma_history_t 		dma_history[MAX_DMA_HIST_SIZE];
#endif

	u32		maintenance_mode_bitmap;
	wp_bert_t	wp_bert;
	netskb_t	*tx_bert_skb;
	u32 bert_data_length;

}private_area_t;


void 	aft_free_logical_channel_num (sdla_t *card, int logic_ch);
void 	aft_dma_max_logic_ch(sdla_t *card);

#undef AFT_FE_INTR_DEBUG 

#ifdef AFT_FE_INTR_DEBUG
void 	___aft_fe_intr_ctrl(sdla_t *card, int status, char *func, int line);
static __inline int ___aft_fe_intr_ctrl_locked(sdla_t *card, int status, char* func, int line)
{
	wan_smp_flag_t smp_flags; 
	wan_spin_lock_irq(&card->wandev.lock,&smp_flags); 
	___aft_fe_intr_ctrl(card,status,func,line);
	wan_spin_unlock_irq(&card->wandev.lock,&smp_flags); 
	return 0;
}
#define __aft_fe_intr_ctrl(card, status) ___aft_fe_intr_ctrl(card, status, (char*)__FUNCTION__,__LINE__)
#define aft_fe_intr_ctrl(card, status) ___aft_fe_intr_ctrl_locked(card, status,(char*)__FUNCTION__,__LINE__)
#else
void 	aft_fe_intr_ctrl(sdla_t *card, int status);
void 	__aft_fe_intr_ctrl(sdla_t *card, int status);
#endif

void 	aft_wdt_set(sdla_t *card, unsigned char val);
void 	aft_wdt_reset(sdla_t *card);
int	 	aft_free_running_timer_set_enable(sdla_t *card, u32 ms);
int 	aft_free_running_timer_disable(sdla_t *card);
void 	wanpipe_wake_stack(private_area_t* chan);
int 	aft_core_api_event_init(sdla_t *card);
int 	aft_event_ctrl(void *chan_ptr, wan_event_ctrl_t *p_event);
int 	aft_core_tdmapi_event_init(private_area_t *chan);
int 	wan_aft_api_ioctl(sdla_t *card, private_area_t *chan, char *user_data);
int 	aft_dma_tx (sdla_t *card,private_area_t *chan);
int 	aft_tdm_chan_ring_rsyinc(sdla_t * card, private_area_t *chan, int log );


static __inline void wan_aft_skb_defered_dealloc(private_area_t *chan, netskb_t *skb)
{
	if(chan->tx_bert_skb == skb){
		return;
	}

	wan_skb_queue_tail(&chan->wp_dealloc_list,skb);
	WAN_TASKLET_SCHEDULE((&chan->common.bh_task));
}

static __inline void wan_chan_dev_stop(private_area_t *chan)
{
	wan_set_bit(0,&chan->busy);
}
static __inline void wan_chan_dev_start(private_area_t *chan)
{
	wan_clear_bit(0,&chan->busy);
}
static __inline int wan_chan_dev_stopped(private_area_t *chan)
{
	return wan_test_bit(0,&chan->busy);
}

int aft_background_timer_kill(sdla_t* card);
int aft_background_timer_add(sdla_t* card, unsigned long delay);

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
void aft_background_timer_expire(void* pcard);
#elif defined(__WINDOWS__)
void aft_background_timer_expire(IN PKDPC Dpc, void* pcard, void* arg2, void* arg3);
#else
void aft_background_timer_expire(unsigned long pcard);
#endif

int aft_fe_loop_back_status(sdla_t *card);

#endif /* WAN_KERNEL */


#endif

