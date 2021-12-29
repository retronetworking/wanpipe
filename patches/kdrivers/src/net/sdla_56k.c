/******************************************************************************
 * sdla_56k.c	WANPIPE(tm) Multiprotocol WAN Link Driver. 
 *				56K board configuration.
 *
 * Author: 	Nenad Corbic  <ncorbic@sangoma.com>
 *
 * Copyright:	(c) 1995-2001 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 * Jul 22, 2001	Nenad Corbic	Initial version.
 * Oct 01, 2001 Gideon Hack	Modifications for interrupt usage.
 ******************************************************************************
 */


/*
 ******************************************************************************
			   INCLUDE FILES
 ******************************************************************************
*/

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe.h>
# include <wanproc.h>
#elif defined(__WINDOWS__)
# include <wanpipe_includes.h>
# include <wanpipe.h>	/* WANPIPE common user API definitions */
#else
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanproc.h>
# include <linux/wanpipe.h>	/* WANPIPE common user API definitions */
# include <linux/aft_a104.h>/* for aft_56k_write_cpld() declaration  */
#endif

/******************************************************************************
			  DEFINES AND MACROS
******************************************************************************/
#define WRITE_REG(reg,val) fe->write_fe_reg(fe->card, 0, (u32)(reg),(u32)(val))
#define READ_REG(reg)	   fe->read_fe_reg(fe->card, 0, (u32)(reg))

#if 0
#define AFT_FUNC_DEBUG()  DEBUG_EVENT("%s:%d\n",__FUNCTION__,__LINE__)
#else
#define AFT_FUNC_DEBUG()  
#endif


/******************************************************************************
			STRUCTURES AND TYPEDEFS
******************************************************************************/


/******************************************************************************
			   GLOBAL VARIABLES
******************************************************************************/

/******************************************************************************
			  FUNCTION PROTOTYPES
******************************************************************************/


static int sdla_56k_global_config(void* pfe);
static int sdla_56k_global_unconfig(void* pfe);


static unsigned int sdla_56k_alarm(sdla_fe_t *fe, int manual_read);


static int sdla_56k_config(void* pfe);
static int sdla_56k_unconfig(void* pfe);
static int sdla_56k_intr(sdla_fe_t *fe);
static int sdla_56k_check_intr(sdla_fe_t *fe);

static unsigned int sdla_56k_alarm(sdla_fe_t *fe, int manual_read);
static int sdla_56k_udp(sdla_fe_t*, void*, unsigned char*);
static void display_Rx_code_condition(sdla_fe_t* fe);
static int sdla_56k_print_alarm(sdla_fe_t* fe, unsigned int);
static int sdla_56k_update_alarm_info(sdla_fe_t *fe, struct seq_file* m, int* stop_cnt);


/* enable 56k chip reset state */
static unsigned int reset_on_LXT441PE(sdla_t *card);
/* disable 56k chip reset state */
static unsigned int reset_off_LXT441PE(sdla_t *card);




/******************************************************************************
			  FUNCTION DEFINITIONS
******************************************************************************/

/******************************************************************************
 *				sdla_56k_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static char* sdla_56k_get_fe_media_string(void)
{
	return ("S514_56K");
}

/******************************************************************************
 *				sdla_56k_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static unsigned char sdla_56k_get_fe_media(sdla_fe_t *fe)
{
	return fe->fe_cfg.media;
}

/******************************************************************************
 *				sdla_56k_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_56k_get_fe_status(sdla_fe_t *fe, unsigned char *status)
{
	*status = fe->fe_status;
	return 0;
}

unsigned int sdla_56k_alarm(sdla_fe_t *fe, int manual_read)
{

	unsigned short status = 0x00;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);
	/* set to manually read the front-end registers */
	if(manual_read) {
		fe->fe_param.k56_param.RR8_reg_56k = 0;
		fe->fe_param.k56_param.RR8_reg_56k = READ_REG(REG_INT_EN_STAT);
		fe->fe_param.k56_param.RRA_reg_56k = READ_REG(REG_DEV_STAT);
		fe->fe_param.k56_param.RRC_reg_56k = READ_REG(REG_RX_CODES);
		status |= fe->fe_param.k56_param.RRA_reg_56k;
		status = (status << 8) & 0xFF00;
		status |= fe->fe_param.k56_param.RRC_reg_56k;
	}
	
	DEBUG_56K("56K registers: 8-%02X,C-%02X,A-%02X\n", 
			fe->fe_param.k56_param.RR8_reg_56k,
			fe->fe_param.k56_param.RRC_reg_56k, 
			fe->fe_param.k56_param.RRA_reg_56k);

	/* discard an invalid Rx code interrupt */
	if((fe->fe_param.k56_param.RR8_reg_56k & BIT_INT_EN_STAT_RX_CODE) && 
		(!fe->fe_param.k56_param.RRC_reg_56k)) {
		return(status);
	}

	/* record an RLOS (Receive Loss of Signal) condition */
	if(fe->fe_param.k56_param.RRA_reg_56k & BIT_DEV_STAT_RLOS) {
		fe->fe_param.k56_param.RLOS_56k = 1;
	}

	/* display Rx code conditions if necessary */
	if(!((fe->fe_param.k56_param.RRC_reg_56k ^ fe->fe_param.k56_param.prev_RRC_reg_56k) & 
		fe->fe_param.k56_param.delta_RRC_reg_56k) ||
		(fe->fe_status == FE_CONNECTED)) {
			display_Rx_code_condition(fe);
	}
	
	/* record the changes that occurred in the Rx code conditions */
	fe->fe_param.k56_param.delta_RRC_reg_56k |= 
		(fe->fe_param.k56_param.RRC_reg_56k ^ fe->fe_param.k56_param.prev_RRC_reg_56k);

	
	if(manual_read || (fe->fe_param.k56_param.RR8_reg_56k & BIT_INT_EN_STAT_ACTIVE)) {

		/* if Insertion Loss is less than 44.4 dB, then we are connected */
		if ((fe->fe_param.k56_param.RRA_reg_56k & 0x0F) > BIT_DEV_STAT_IL_44_dB) {
			if((fe->fe_status != FE_CONNECTED)) {
				
				fe->fe_status = FE_CONNECTED;
				/* reset the Rx code condition changes */
				fe->fe_param.k56_param.delta_RRC_reg_56k = 0;
				
				if(fe->fe_param.k56_param.RLOS_56k == 1) {
					fe->fe_param.k56_param.RLOS_56k = 0;
					DEBUG_EVENT("%s: 56k Receive Signal Recovered\n", 
							fe->name);
				}
				DEBUG_EVENT("%s: 56k Connected\n",
							fe->name);
			}
		}else{
			if((fe->fe_status == FE_CONNECTED) || 
			   (fe->fe_status == FE_UNITIALIZED)) {
				
				fe->fe_status = FE_DISCONNECTED;
				/* reset the Rx code condition changes */
				fe->fe_param.k56_param.delta_RRC_reg_56k = 0;

				if(fe->fe_param.k56_param.RLOS_56k == 1) {
					DEBUG_EVENT("%s: 56k Receive Loss of Signal\n", 
							fe->name);
				}
				DEBUG_EVENT("%s: 56k Disconnected (loopback) (RRA=0x%0X < 0x%0X)\n",
						fe->name,
						fe->fe_param.k56_param.RRA_reg_56k & 0x0F,
						BIT_DEV_STAT_IL_44_dB);
			}
		}
	}
	
	fe->fe_param.k56_param.prev_RRC_reg_56k = 
				fe->fe_param.k56_param.RRC_reg_56k;
	fe->fe_alarm = status;
	return(status);
}


int sdla_56k_default_cfg(void* pcard, void* p56k_cfg)
{
	return 0;
}


int sdla_56k_iface_init(void *p_fe, void* pfe_iface)
{

	sdla_fe_t	*fe = (sdla_fe_t*)p_fe;
	sdla_fe_iface_t	*fe_iface = (sdla_fe_iface_t*)pfe_iface;


	fe_iface->global_config		= &sdla_56k_global_config;
	fe_iface->global_unconfig	= &sdla_56k_global_unconfig;

	fe_iface->config		= &sdla_56k_config;
	fe_iface->unconfig		= &sdla_56k_unconfig;
	
	fe_iface->get_fe_status		= &sdla_56k_get_fe_status;
	fe_iface->get_fe_media		= &sdla_56k_get_fe_media;
	fe_iface->get_fe_media_string	= &sdla_56k_get_fe_media_string;
	fe_iface->print_fe_alarm	= &sdla_56k_print_alarm;
	fe_iface->read_alarm		= &sdla_56k_alarm;
	fe_iface->update_alarm_info	= &sdla_56k_update_alarm_info;
	fe_iface->process_udp		= &sdla_56k_udp;

	fe_iface->isr			= &sdla_56k_intr;
	fe_iface->check_isr		= &sdla_56k_check_intr;

	/* The 56k CSU/DSU front end status has not been initialized  */
	fe->fe_status = FE_UNITIALIZED;

	return 0;
}

/* called from TASK */
static int sdla_56k_intr(sdla_fe_t *fe) 
{
	/*AFT_FUNC_DEBUG();*/

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	sdla_56k_alarm(fe, 1);

	return 0;
}

/*	Called from ISR. On AFT card there is only on 56 port, it 
	means the interrupt is always ours.
	Returns: 1
 */
static int sdla_56k_check_intr(sdla_fe_t *fe) 
{
	/*AFT_FUNC_DEBUG();*/

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	return 1;
}

static int sdla_56k_global_config(void* pfe)
{
	DEBUG_56K("%s: %s Global Front End configuration\n", 
			fe->name, FE_MEDIA_DECODE(fe));
	return 0;
}

static int sdla_56k_global_unconfig(void* pfe)
{
	DEBUG_56K("%s: %s Global unconfiguration!\n",
				fe->name,
				FE_MEDIA_DECODE(fe));
	return 0;
}




static int sdla_56k_config(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_t		*card = (sdla_t *)fe->card;
	u16			adapter_type;

	/*AFT_FUNC_DEBUG();*/


	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	card->hw_iface.getcfg(card->hw, SDLA_ADAPTERTYPE, &adapter_type);

#if 0
	/* The 56k CSU/DSU front end status has not been initialized  */
	fe->fe_status = FE_UNITIALIZED;
#endif

	/* Zero the previously read RRC register value */
	fe->fe_param.k56_param.prev_RRC_reg_56k = 0;
	
	/* Zero the RRC register changes */ 
	fe->fe_param.k56_param.delta_RRC_reg_56k = 0;
	
		
	if(adapter_type == AFT_ADPTR_56K){
		reset_on_LXT441PE(card);
		reset_off_LXT441PE(card);
	}

	if(WRITE_REG(REG_INT_EN_STAT, (BIT_INT_EN_STAT_IDEL | 
		BIT_INT_EN_STAT_RX_CODE | BIT_INT_EN_STAT_ACTIVE))) {
		return 1;
	} 

	if(WRITE_REG(REG_RX_CODES, 0xFF & ~(BIT_RX_CODES_ZSC))) {
		return 1;
	}

	if (card->wandev.clocking == WANOPT_EXTERNAL){
		DEBUG_EVENT("%s: Configuring 56K CSU/DSU for External Clocking\n",
				card->devname);
		if(WRITE_REG(REG_DEV_CTRL, 
			(BIT_DEV_CTRL_SCT_E_OUT | BIT_DEV_CTRL_DDS_PRI))) {
			return 1;
		}
	}else{
		DEBUG_EVENT("%s: Configuring 56K CSU/DSU for Internal Clocking\n",
				fe->name);
		if(WRITE_REG(REG_DEV_CTRL, (BIT_DEV_CTRL_XTALI_INT | 
			BIT_DEV_CTRL_SCT_E_OUT | BIT_DEV_CTRL_DDS_PRI))) {
			return 1;
		}
 	}

	if(WRITE_REG(REG_TX_CTRL, 0x00)) {
		return 1;
	}

	if(WRITE_REG(REG_RX_CTRL, 0x00)) {
		return 1;
	}

	if(WRITE_REG(REG_EIA_SEL, 0x00)) {
		return 1;
	}

	if(WRITE_REG(REG_EIA_TX_DATA, 0x00)) {
		return 1;
	}

	if(WRITE_REG(REG_EIA_CTRL, (BIT_EIA_CTRL_RTS_ACTIVE | 
		BIT_EIA_CTRL_DTR_ACTIVE | BIT_EIA_CTRL_DTE_ENABLE))) {
		return 1; 
	}

	return 0;
}

static int sdla_56k_unconfig(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_t		*card = (sdla_t *)fe->card;
	u16			adapter_type;

	/*AFT_FUNC_DEBUG();*/

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	card->hw_iface.getcfg(card->hw, SDLA_ADAPTERTYPE, &adapter_type);

	if(adapter_type == AFT_ADPTR_56K){
		reset_on_LXT441PE(card);
	}
	return 0;
}




static unsigned int reset_on_LXT441PE(sdla_t *card)
{	
	AFT_FUNC_DEBUG();
	aft_56k_write_cpld(card, 0x00,0x00);
	WP_DELAY(1000);
	return 0;
}

static unsigned int reset_off_LXT441PE(sdla_t *card)
{	
	AFT_FUNC_DEBUG();
	aft_56k_write_cpld(card, 0x00, 0x03);
	WP_DELAY(1000);
	return 0;
}









static void display_Rx_code_condition(sdla_fe_t* fe)
{
	sdla_56k_param_t	*k56_param = &fe->fe_param.k56_param;

	/* check for a DLP (DSU Non-latching loopback) code condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_DLP) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_DLP) {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k receiving DSU Loopback code\n", 
					fe->name);
			}
		} else {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k DSU Loopback code condition ceased\n", 
					fe->name);
			}
		}
	}

	/* check for an OOF (Out of Frame) code condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_OOF) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_OOF) {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k receiving Out of Frame code\n", 
					fe->name);
			}
		} else {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k Out of Frame code condition ceased\n", 
					fe->name);
			}
		}
	}

	/* check for an OOS (Out of Service) code condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_OOS) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_OOS) {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k receiving Out of Service code\n", 
					fe->name);
			}
		} else {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k Out of Service code condition ceased\n", 
					fe->name);
			}
		}
	}

	/* check for a CMI (Control Mode Idle) condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_CMI) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_CMI) {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k receiving Control Mode Idle\n",
					fe->name);
			}
		} else {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k Control Mode Idle condition ceased\n", 
					fe->name);
			}
		}
	}

	/* check for a ZSC (Zero Suppression Code) condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_ZSC) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_ZSC) {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k receiving Zero Suppression code\n", 
					fe->name);
			}
		} else {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k Zero Suppression code condition ceased\n", 
					fe->name);
			}
		}
	}

	/* check for a DMI (Data Mode Idle) condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_DMI) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_DMI) {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k receiving Data Mode Idle\n",
				fe->name);
			}
		} else {
			if (WAN_NET_RATELIMIT()) {
			DEBUG_EVENT("%s: 56k Data Mode Idle condition ceased\n", 
				fe->name);
			}
		}
	}
}


/*
 ******************************************************************************
 *                              sdla_te_set_lbmode()
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */

static int
sdla_56k_set_lbmode(sdla_fe_t *fe, unsigned char type, unsigned char mode)
{

        unsigned char loop=BIT_RX_CTRL_DSU_LOOP|BIT_RX_CTRL_CSU_LOOP;
        //unsigned char loop=BIT_RX_CTRL_DSU_LOOP;
        //unsigned char loop=BIT_RX_CTRL_CSU_LOOP;
        //unsigned char loop=0x40;

        WAN_ASSERT(fe->write_fe_reg == NULL);
        WAN_ASSERT(fe->read_fe_reg == NULL);

        if (mode == WAN_TE1_ACTIVATE_LB){
                WRITE_REG(REG_RX_CTRL, READ_REG(REG_RX_CTRL) | loop);

                DEBUG_EVENT("%s: %s Diagnostic Line Loopback mode activated (0x%X).\n",
                        fe->name,
                        FE_MEDIA_DECODE(fe),
                        READ_REG(REG_RX_CTRL));
        }else{

                WRITE_REG(REG_RX_CTRL, READ_REG(REG_RX_CTRL) & ~(loop));
                DEBUG_EVENT("%s: %s Diagnostic Line Loopback mode deactivated (0x%X).\n",
                        fe->name,
                        FE_MEDIA_DECODE(fe),
                        READ_REG(REG_RX_CTRL));
        }

        return 0;

}


/*
 ******************************************************************************
 *				sdla_56k_udp()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_56k_udp(sdla_fe_t *fe, void* pudp_cmd, unsigned char* data)
{
	wan_cmd_t	*udp_cmd = (wan_cmd_t*)pudp_cmd;
	int err=0;
	
	switch(udp_cmd->wan_cmd_command){
	case WAN_GET_MEDIA_TYPE:
		data[0] = (IS_56K_FEMEDIA(fe) ? WAN_MEDIA_56K : 
						WAN_MEDIA_NONE);
		udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		udp_cmd->wan_cmd_data_len = sizeof(unsigned char); 
		break;

	case WAN_FE_GET_STAT:
		/* 56K Update CSU/DSU alarms */
		fe->fe_alarm = sdla_56k_alarm(fe, 1); 
		memcpy(&data[0], &fe->fe_stats, sizeof(sdla_fe_stats_t));
		udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
	    	udp_cmd->wan_cmd_data_len = sizeof(sdla_fe_stats_t);
		break;
		
	case WAN_FE_SET_LB_MODE:
                /* Activate/Deactivate Line Loopback modes */
                err = sdla_56k_set_lbmode(fe, data[0], data[1]);
                udp_cmd->wan_cmd_return_code =
                                (!err) ? WAN_CMD_OK : WAN_UDP_FAILED_CMD;
                udp_cmd->wan_cmd_data_len = 0x00;
                break;

 	case WAN_FE_FLUSH_PMON:
	case WAN_FE_GET_CFG:
	default:
		udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
	    	udp_cmd->wan_cmd_data_len = 0;
		break;
	}
	return 0;
}

/*
*******************************************************************************
**				sdla_56k_alaram_print()	
**
** Description: 
** Arguments:
** Returns:
*/
static int sdla_56k_print_alarm(sdla_fe_t* fe, unsigned int status)
{
	unsigned int	alarms = (unsigned int)status;

	if (!alarms){
		alarms = sdla_56k_alarm(fe, 0);
	}
	
	DEBUG_EVENT("%s: 56K Alarms:\n", fe->name);
 	DEBUG_EVENT("%s: In Service:            %s Data mode idle:    %s\n",
				fe->name,
			 	INS_ALARM_56K(alarms), 
			 	DMI_ALARM_56K(alarms));
	
 	DEBUG_EVENT("%s: Zero supp. code:       %s Ctrl mode idle:    %s\n",
				fe->name,
			 	ZCS_ALARM_56K(alarms), 
			 	CMI_ALARM_56K(alarms));

 	DEBUG_EVENT("%s: Out of service code:   %s Out of frame code: %s\n",
				fe->name,
			 	OOS_ALARM_56K(alarms), 
			 	OOF_ALARM_56K(alarms));
		
 	DEBUG_EVENT("%s: Valid DSU NL loopback: %s Unsigned mux code: %s\n",
				fe->name,
			 	DLP_ALARM_56K(alarms), 
			 	UMC_ALARM_56K(alarms));

 	DEBUG_EVENT("%s: Rx loss of signal:     %s \n",
				fe->name,
			 	RLOS_ALARM_56K(alarms)); 
	return 0;
}

static int sdla_56k_update_alarm_info(sdla_fe_t* fe, struct seq_file* m, int* stop_cnt)
{
#if !defined(__WINDOWS__)
	PROC_ADD_LINE(m,
		"\n====================== Rx 56K CSU/DSU Alarms ==============================\n");
	PROC_ADD_LINE(m,
		PROC_STATS_ALARM_FORMAT,
		"In service", INS_ALARM_56K(fe->fe_alarm), 
		"Data mode idle", DMI_ALARM_56K(fe->fe_alarm));
		
	PROC_ADD_LINE(m,
		PROC_STATS_ALARM_FORMAT,
		"Zero supp. code", ZCS_ALARM_56K(fe->fe_alarm), 
		"Ctrl mode idle", CMI_ALARM_56K(fe->fe_alarm));

	PROC_ADD_LINE(m,
		PROC_STATS_ALARM_FORMAT,
		"Out of service code", OOS_ALARM_56K(fe->fe_alarm), 
		"Out of frame code", OOF_ALARM_56K(fe->fe_alarm));
		
	PROC_ADD_LINE(m,
		PROC_STATS_ALARM_FORMAT,
		"Valid DSU NL loopback", DLP_ALARM_56K(fe->fe_alarm), 
		"Unsigned mux code", UMC_ALARM_56K(fe->fe_alarm));

	PROC_ADD_LINE(m,
		PROC_STATS_ALARM_FORMAT,
		"Rx loss of signal", RLOS_ALARM_56K(fe->fe_alarm), 
		"N/A", "N/A");
#endif
	return m->count;	
}

