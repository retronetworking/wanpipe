/*************************************************************************
 sdla_temp.h	Template Header 
 
 Author:      	Nenad Corbic <ncorbic@sangoma.com>	

 Copyright:	(c) 1995-2002 Sangoma Technologies Inc.

		This program is free software; you can redistribute it and/or
		modify it under the term of the GNU General Public License
		as published by the Free Software Foundation; either version
		2 of the License, or (at your option) any later version.

===========================================================================
Jan 08, 2003	Nenad Corbic    Initial Version
===========================================================================

**************************************************************************/



#ifndef __SDLA_TEMPLATE__
#define __SDLA_TEMPlATE__


#define SHARED_MEMORY_INFO_STRUCT	void
#define CONFIGURATION_STRUCT		void
#define INTERRUPT_INFORMATION_STRUCT	void
#define TRIGGERS_STRUCT			void
#define DATA_RX_STATUS_EL_STRUCT	void
#define DATA_TX_STATUS_EL_STRUCT	void
#define INT_TRIGGERS_STRUCT		void
#define TX_STATUS_EL_CFG_STRUCT		void
#define RX_STATUS_EL_CFG_STRUCT		void
#define TRACE_STATUS_EL_CFG_STRUCT	void
#define TRACE_STATUS_ELEMENT_STRUCT	void
#define LINE_TRACE_CONFIG_STRUCT	void
#define COMMS_ERROR_STATS_STRUCT	void
#define OPERATIONAL_STATS_STRUCT	void

#define DFLT_TEMPLATE_VALUE		0

#define WANCONFIG_FRMW			DFLT_TEMPLATE_VALUE	

#define COMMAND_OK			DFLT_TEMPLATE_VALUE

#define APP_INT_ON_TIMER		DFLT_TEMPLATE_VALUE
#define APP_INT_ON_TX_FRAME		DFLT_TEMPLATE_VALUE
#define APP_INT_ON_RX_FRAME		DFLT_TEMPLATE_VALUE
#define APP_INT_ON_GLOBAL_EXCEP_COND	DFLT_TEMPLATE_VALUE
#define APP_INT_ON_EXCEP_COND		DFLT_TEMPLATE_VALUE
#define APP_INT_ON_COMMAND_COMPLETE	DFLT_TEMPLATE_VALUE

#define READ_CODE_VERSION		DFLT_TEMPLATE_VALUE

#define READ_CONFIGURATION		DFLT_TEMPLATE_VALUE
#define SET_CONFIGURATION		DFLT_TEMPLATE_VALUE

#define INTERRUPT_TRIGGERS		DFLT_TEMPLATE_VALUE

#define DISABLE_COMMUNICATIONS		DFLT_TEMPLATE_VALUE
#define ENABLE_COMMUNICATIONS		DFLT_TEMPLATE_VALUE

#define SET_INTERRUPT_TRIGGERS		DFLT_TEMPLATE_VALUE

#define UDPMGMT_SIGNATURE		DFLT_TEMPLATE_VALUE

#define FT1_MONITOR_STATUS_CTRL		DFLT_TEMPLATE_VALUE
#define ENABLE_READ_FT1_STATUS		DFLT_TEMPLATE_VALUE
#define ENABLE_READ_FT1_OP_STATS	DFLT_TEMPLATE_VALUE		
#define CPIPE_FT1_READ_STATUS		DFLT_TEMPLATE_VALUE
#define TRACE_INACTIVE			DFLT_TEMPLATE_VALUE

#define READ_GLOBAL_STATISTICS		DFLT_TEMPLATE_VALUE
#define READ_MODEM_STATUS		DFLT_TEMPLATE_VALUE
#define READ_LINK_STATUS		DFLT_TEMPLATE_VALUE
#define ROUTER_UP_TIME			DFLT_TEMPLATE_VALUE
#define READ_COMMS_ERROR_STATS		DFLT_TEMPLATE_VALUE
#define READ_OPERATIONAL_STATS		DFLT_TEMPLATE_VALUE
#define READ_TRACE_CONFIGURATION	DFLT_TEMPLATE_VALUE	

#define	GET_TRACE_INFO			DFLT_TEMPLATE_VALUE
#define SET_TRACE_CONFIGURATION		DFLT_TEMPLATE_VALUE

#define FT1_READ_STATUS			DFLT_TEMPLATE_VALUE
#define UDP_TYPE			DFLT_TEMPLATE_VALUE
#define ENABLE_TRACING			DFLT_TEMPLATE_VALUE
#define TRACE_ACTIVE			DFLT_TEMPLATE_VALUE	

#define EXCEP_TRC_DISABLED		DFLT_TEMPLATE_VALUE
#define EXCEP_IRQ_TIMEOUT		DFLT_TEMPLATE_VALUE
#define READ_EXCEPTION_CONDITION	DFLT_TEMPLATE_VALUE
#define EXCEP_LINK_ACTIVE		DFLT_TEMPLATE_VALUE
#define EXCEP_LINK_INACTIVE_MODEM	DFLT_TEMPLATE_VALUE
#define EXCEP_LINK_INACTIVE_KPALV	DFLT_TEMPLATE_VALUE
#define EXCEP_IP_ADDRESS_DISCOVERED	DFLT_TEMPLATE_VALUE
#define EXCEP_LOOPBACK_CONDITION	DFLT_TEMPLATE_VALUE
#define NO_EXCEP_COND_TO_REPORT		DFLT_TEMPLATE_VALUE
#define DISABLE_TRACING			DFLT_TEMPLATE_VALUE



#define TX_BYTE_COUNT_STAT		DFLT_TEMPLATE_VALUE	
#define RX_BYTE_COUNT_STAT		DFLT_TEMPLATE_VALUE
#define TX_THROUGHPUT_STAT		DFLT_TEMPLATE_VALUE
#define RX_THROUGHPUT_STAT		DFLT_TEMPLATE_VALUE
#define INCL_UNDERRUN_TX_THRUPUT	DFLT_TEMPLATE_VALUE
#define INCL_DISC_RX_THRUPUT		DFLT_TEMPLATE_VALUE
#define START_BAUD_CALIBRATION		DFLT_TEMPLATE_VALUE
#define READ_BAUD_CALIBRATION_RESULT	DFLT_TEMPLATE_VALUE
#define BAUD_CALIBRATION_NOT_DONE	DFLT_TEMPLATE_VALUE
#define BUSY_WITH_BAUD_CALIBRATION	DFLT_TEMPLATE_VALUE
#define BAUD_CAL_FAILED_NO_TX_CLK	DFLT_TEMPLATE_VALUE
#define BAUD_CAL_FAILED_BAUD_HI		DFLT_TEMPLATE_VALUE
#define CANNOT_DO_BAUD_CAL		DFLT_TEMPLATE_VALUE
#define READ_GLOBAL_EXCEPTION_CONDITION	DFLT_TEMPLATE_VALUE
#define EXCEP_MODEM_STATUS_CHANGE	DFLT_TEMPLATE_VALUE
#define DCD_HIGH			DFLT_TEMPLATE_VALUE
#define CTS_HIGH			DFLT_TEMPLATE_VALUE

#define INTERFACE_LEVEL_RS232		DFLT_TEMPLATE_VALUE
#define INTERFACE_LEVEL_V35		DFLT_TEMPLATE_VALUE
#define TRANSPARENT_TX_RX_CELLS		DFLT_TEMPLATE_VALUE
#define UNI				DFLT_TEMPLATE_VALUE
#define DISABLE_RX_HEC_CHECK		DFLT_TEMPLATE_VALUE
#define MAX_CELLS_IN_TX_BLOCK		DFLT_TEMPLATE_VALUE
#define MAX_RX_COMPLETE_CELL_COUNT	DFLT_TEMPLATE_VALUE



#define COMMAND_COMPLETE_APP_INT_PEND	DFLT_TEMPLATE_VALUE
#define RX_APP_INT_PEND			DFLT_TEMPLATE_VALUE
#define TX_APP_INT_PEND			DFLT_TEMPLATE_VALUE
#define EXCEP_COND_APP_INT_PEND		DFLT_TEMPLATE_VALUE
#define GLOBAL_EXCEP_COND_APP_INT_PEND	DFLT_TEMPLATE_VALUE
#define TIMER_APP_INT_PEND		DFLT_TEMPLATE_VALUE

#define MIN_WP_PRI_MTU 		1500
#define MAX_WP_PRI_MTU 		MIN_WP_PRI_MTU 
#define DEFAULT_WP_PRI_MTU 	MIN_WP_PRI_MTU

#define MIN_WP_SEC_MTU 		1500
#define MAX_WP_SEC_MTU 		MIN_WP_SEC_MTU 
#define DEFAULT_WP_SEC_MTU 	MIN_WP_SEC_MTU


/* reasons for enabling the timer interrupt on the adapter */
#define TMR_INT_ENABLED_UDP   		0x01
#define TMR_INT_ENABLED_UPDATE		0x02
#define TMR_INT_ENABLED_CONFIG		0x10
#define TMR_INT_ENABLED_TE		0x20


#define PRI_BASE_ADDR_MB_STRUCT		DFLT_TEMPLATE_VALUE
static __inline void init_card_mailbox(sdla_t *card)
{
	/* Initialize protocol-specific fields */
	if(card->hw.type != SDLA_S514){
		card->mbox  = (void *) card->hw.dpmbase;
	}else{ 
		/* for a S514 adapter, set a pointer to the actual mailbox in the */
		/* allocated virtual memory area */
		card->mbox = (void *) card->hw.dpmbase + PRI_BASE_ADDR_MB_STRUCT;
	}
}

static __inline wan_mbox_t *get_card_mailbox(sdla_t *card)
{
	return (wan_mbox_t*)card->mbox;
}

static __inline void *get_card_flags(sdla_t *card)
{
	return card->u.c.flags;
}

static __inline void set_card_flags(sdla_t *card, void *ptr)
{
	card->u.c.flags = ptr;
}
static __inline void *get_card_rxmb(sdla_t *card)
{
	return card->u.c.rxmb;
}

static __inline void set_card_rxmb(sdla_t *card, void *ptr)
{
	card->u.c.rxmb = ptr;
	if((void*)ptr > card->u.c.rxbuf_last){
		card->u.c.rxmb = card->u.c.rxbuf_base;
	}
}

static __inline unsigned long get_rxtop_buf(sdla_t *card)
{
	return card->u.c.rx_top;
}
static __inline unsigned long get_rxbase_buf(sdla_t *card)
{
	return card->u.c.rx_base;
}

static __inline void *get_card_txbuf(sdla_t *card)
{
	return card->u.c.txbuf;
}

static __inline void set_card_txbuf(sdla_t *card, void *ptr)
{
	card->u.c.txbuf = ptr;
	if ((void*)ptr > card->u.c.txbuf_last)
		card->u.c.txbuf = card->u.c.txbuf_base;
}


#endif
