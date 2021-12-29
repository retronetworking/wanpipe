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
# include <net/wanpipe_includes.h>
# include <net/wanpipe.h>
# include <net/wanproc.h>
#elif defined(__WINDOWS__)
# include <wanpipe\wanpipe_includes.h>
# include <hdlc_stm_api\driver.h>
#else
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanproc.h>
# include <linux/wanpipe.h>	/* WANPIPE common user API definitions */
#endif

/*
 ******************************************************************************
			  DEFINES AND MACROS
 ******************************************************************************
*/
#if 0
#define WRITE_REG(reg, value)		card->wandev.write_front_end_reg(card, reg, (unsigned char)(value))
#define READ_REG(reg)			card->wandev.read_front_end_reg(card, reg)
#endif

/*
 ******************************************************************************
			STRUCTURES AND TYPEDEFS
 ******************************************************************************
*/


/*
 ******************************************************************************
			   GLOBAL VARIABLES
 ******************************************************************************
*/

/*
 ******************************************************************************
			  FUNCTION PROTOTYPES
 ******************************************************************************
*/
static void display_Rx_code_condition(sdla_fe_t* fe);
static int sdla_56k_print_alarm(sdla_fe_t* fe, unsigned long);
static unsigned long sdla_56k_alarm(sdla_fe_t *fe, int manual_read);
static int sdla_56k_update_alarm_info(sdla_fe_t *fe, struct seq_file* m, int* stop_cnt);
static int sdla_56k_udp(sdla_fe_t*, void*, unsigned char*);

/*
 ******************************************************************************
			  FUNCTION DEFINITIONS
 ******************************************************************************
*/
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

static unsigned long sdla_56k_alarm(sdla_fe_t *fe, int manual_read)
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
			if((fe->fe_status == FE_DISCONNECTED) ||
			 (fe->fe_status == FE_UNITIALIZED)) {
				
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
				DEBUG_EVENT("%s: 56k Disconnected (loopback)\n",
						fe->name);
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


int sdla_56k_config(void* pfe, void* pfe_iface)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_fe_iface_t	*fe_iface = (sdla_fe_iface_t*)pfe_iface;
	sdla_t		*card = (sdla_t *)fe->card;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);
	/* The 56k CSU/DSU front end status has not been initialized  */
	fe->fe_status = FE_UNITIALIZED;

	/* Zero the previously read RRC register value */
	fe->fe_param.k56_param.prev_RRC_reg_56k = 0;
	
	/* Zero the RRC register changes */ 
	fe->fe_param.k56_param.delta_RRC_reg_56k = 0;
	
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

	/* Initialize function pointer for WANPIPE debugging */
	fe_iface->get_fe_status		= &sdla_56k_get_fe_status;
	fe_iface->get_fe_media		= &sdla_56k_get_fe_media;
	fe_iface->get_fe_media_string	= &sdla_56k_get_fe_media_string;
	fe_iface->print_fe_alarm	= &sdla_56k_print_alarm;
	fe_iface->read_alarm		= &sdla_56k_alarm;
	fe_iface->update_alarm_info	= &sdla_56k_update_alarm_info;
	fe_iface->process_udp		= &sdla_56k_udp;
	return 0;
}


static void display_Rx_code_condition(sdla_fe_t* fe)
{
	sdla_56k_param_t	*k56_param = &fe->fe_param.k56_param;

	/* check for a DLP (DSU Non-latching loopback) code condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_DLP) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_DLP) {
			DEBUG_EVENT("%s: 56k receiving DSU Loopback code\n", 
					fe->name);
		} else {
			DEBUG_EVENT("%s: 56k DSU Loopback code condition ceased\n", 
					fe->name);
		}
	}

	/* check for an OOF (Out of Frame) code condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_OOF) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_OOF) {
			DEBUG_EVENT("%s: 56k receiving Out of Frame code\n", 
					fe->name);
		} else {
			DEBUG_EVENT("%s: 56k Out of Frame code condition ceased\n", 
					fe->name);
		}
	}

	/* check for an OOS (Out of Service) code condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_OOS) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_OOS) {
			DEBUG_EVENT("%s: 56k receiving Out of Service code\n", 
					fe->name);
		} else {
			DEBUG_EVENT("%s: 56k Out of Service code condition ceased\n", 
					fe->name);
		}
	}

	/* check for a CMI (Control Mode Idle) condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_CMI) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_CMI) {
			DEBUG_EVENT("%s: 56k receiving Control Mode Idle\n",
					fe->name);
		} else {
			DEBUG_EVENT("%s: 56k Control Mode Idle condition ceased\n", 
					fe->name);
		}
	}

	/* check for a ZSC (Zero Suppression Code) condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_ZSC) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_ZSC) {
			DEBUG_EVENT("%s: 56k receiving Zero Suppression code\n", 
					fe->name);
		} else {
			DEBUG_EVENT("%s: 56k Zero Suppression code condition ceased\n", 
					fe->name);
		}
	}

	/* check for a DMI (Data Mode Idle) condition */
	if((k56_param->RRC_reg_56k ^ k56_param->prev_RRC_reg_56k) &
		BIT_RX_CODES_DMI) {
		if(k56_param->RRC_reg_56k & BIT_RX_CODES_DMI) {
			DEBUG_EVENT("%s: 56k receiving Data Mode Idle\n",
				fe->name);
		} else {
			DEBUG_EVENT("%s: 56k Data Mode Idle condition ceased\n", 
				fe->name);
		}
	}
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

	switch(udp_cmd->wan_cmd_command){
	case WAN_GET_MEDIA_TYPE:
		data[0] = (IS_56K_MEDIA(fe->fe_cfg.media) ? WAN_MEDIA_56K : 
						WAN_MEDIA_NONE);
		udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		udp_cmd->wan_cmd_data_len = sizeof(unsigned char); 
		break;

	case WAN_FE_GET_STAT:
		/* 56K Update CSU/DSU alarms */
		fe->fe_alarm = sdla_56k_alarm(fe, 1); 
	 	*(unsigned long *)&data[0] = fe->fe_alarm;
		udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
	    	udp_cmd->wan_cmd_data_len = sizeof(unsigned long);
		break;

	case WAN_FE_SET_LB_MODE:
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
static int sdla_56k_print_alarm(sdla_fe_t* fe, unsigned long status)
{
	unsigned short	alarms = (unsigned short)status;

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

	return m->count;	
}


