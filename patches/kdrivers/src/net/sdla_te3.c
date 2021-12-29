/******************************************************************************
 * 
 * sdla_te3.c	Sangoma T3/E3 front end.
 *
 * 		Alex Feldman <al.feldman@sangoma.com>
 * 		
 * 	Copyright Sangoma Technologies Inc. 1999, 2000,2001, 2002, 2003, 2004
 *
 * 	This program is provided subject to the Software License included in 
 * 	this package in the file license.txt. By using this program you agree
 * 	to be bound bythe terms of this license. 
 *
 * 	Should you not have a copy of the file license.txt, or wish to obtain
 * 	a hard copy of the Software License, please contact Sangoma
 * 	technologies Corporation.
 *
 * 	Contact: Sangoma Technologies Inc. 905-474-1990, info@sangoma.com
 *
 *****************************************************************************/


/******************************************************************************
**			   INCLUDE FILES
******************************************************************************/

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
#  include <wanpipe_snmp.h>
# endif
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <wanproc.h>
# include <wanpipe.h>	/* WANPIPE common user API definitions */
# include <sdla_te3_reg.h>
#elif (defined __LINUX__) || (defined __KERNEL__)
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanproc.h>
# include <linux/sdla_te3_reg.h>
# include <linux/wanpipe.h>	/* WANPIPE common user API definitions */
#else
# error "No OS Defined"
#endif

/******************************************************************************
**			  DEFINES AND MACROS
******************************************************************************/

/******************************************************************************
**			  DEFINES AND MACROS
******************************************************************************/

#if defined(DEBUG)
# define WRITE_CPLD(reg,val)						\
	DEBUG_EVENT("%s: Write to CPLD reg %d value %X\n", 		\
			fe->name, reg, val);

# define WRITE_REG(reg,val) 						\
	DEBUG_EVENT("%s: Write to Framer off %X value %X\n", 		\
			fe->name, reg, val);
#else

# define WRITE_CPLD(reg,val)						\
	(fe->write_cpld) ? fe->write_cpld(fe->card, reg, val) : -EINVAL

# define WRITE_EXAR_CPLD(reg,val)						\
	(fe->write_fe_cpld) ? fe->write_fe_cpld(fe->card, reg, val) : -EINVAL

# define WRITE_REG(reg,val)						\
	(fe->write_framer) ? fe->write_framer(fe->card, reg, val) : -EINVAL
# define READ_REG(reg)						\
	(fe->read_framer) ? fe->read_framer(fe->card, reg) : 0
#endif

#define WAN_FE_SWAP_BIT(value, mask)					\
	if ((value) & mask){						\
		(value) &= ~mask;					\
	}else{								\
		(value) |= mask;					\
	}

/* DS3: Define DS3 alarm states: LOS OOF YEL */
#define IS_DS3_ALARM(alarm)	((alarm) &				\
					(WAN_TE3_BIT_LOS_ALARM |	\
					 WAN_TE3_BIT_YEL_ALARM |	\
					 WAN_TE3_BIT_OOF_ALARM))

/* DS3: Define E3 alarm states: LOS OOF YEL */
#define IS_E3_ALARM(alarm)	((alarm) &				\
					(WAN_TE3_BIT_LOS_ALARM |	\
					 WAN_TE3_BIT_YEL_ALARM |	\
					 WAN_TE3_BIT_OOF_ALARM))

/******************************************************************************
**			  FUNCTION DEFINITIONS
******************************************************************************/

static int sdla_te3_config(void *p_fe);
static int sdla_te3_unconfig(void *p_fe);
static int sdla_te3_get_fe_status(sdla_fe_t *fe, unsigned char *status);
static int sdla_te3_polling(sdla_fe_t *fe);
static int sdla_ds3_isr(sdla_fe_t *fe);
static int sdla_e3_isr(sdla_fe_t *fe);
static int sdla_te3_isr(sdla_fe_t *fe);
static int sdla_te3_udp(sdla_fe_t *fe, void*, unsigned char*);
static unsigned int sdla_te3_alarm(sdla_fe_t *fe, int);
static int sdla_te3_read_pmon(sdla_fe_t *fe, int);

static int sdla_te3_update_alarm_info(sdla_fe_t* fe, struct seq_file* m, int* stop_cnt);
static int sdla_te3_update_pmon_info(sdla_fe_t* fe, struct seq_file* m, int* stop_cnt);

/******************************************************************************
 *				sdla_te3_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static char* sdla_te3_get_fe_media_string(void)
{
	return ("AFT T3/E3");
}

/******************************************************************************
 *				sdla_te3_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static unsigned char sdla_te3_get_fe_media(sdla_fe_t *fe)
{
	return fe->fe_cfg.media;
}

/******************************************************************************
 *				sdla_te3_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_te3_get_fe_status(sdla_fe_t *fe, unsigned char *status)
{
	*status = fe->fe_status;
	return 0;
}

/******************************************************************************
 *				sdla_te3_polling()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_te3_polling(sdla_fe_t *fe)
{
	DEBUG_EVENT("%s: %s: This function is still not supported!\n",
			fe->name, __FUNCTION__);
	return -EINVAL;
}

/******************************************************************************
 *				sdla_te3_set_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_te3_set_status(sdla_fe_t *fe)
{
	if (IS_DS3(&fe->fe_cfg)){
		if (IS_DS3_ALARM(fe->fe_alarm)){
			if (fe->fe_status != FE_DISCONNECTED){
				DEBUG_EVENT("%s: DS3 disconnected!\n",
						fe->name);
				fe->fe_status = FE_DISCONNECTED;
			}
		}else{
			if (fe->fe_status != FE_CONNECTED){
				DEBUG_EVENT("%s: DS3 connected!\n",
						fe->name);
				fe->fe_status = FE_CONNECTED;
			}
		}
	}else if (IS_E3(&fe->fe_cfg)){
		if (IS_E3_ALARM(fe->fe_alarm)){
			if (fe->fe_status != FE_DISCONNECTED){
				DEBUG_EVENT("%s: E3 disconnected!\n",
						fe->name);
				fe->fe_status = FE_DISCONNECTED;
			}
		}else{
			if (fe->fe_status != FE_CONNECTED){
				DEBUG_EVENT("%s: E3 connected!\n",
						fe->name);
				fe->fe_status = FE_CONNECTED;
			}
		}
	}else{
		return -EINVAL;
	}
	return 0;
}

/******************************************************************************
**				sdla_ds3_tx_isr()
**
** Description:
** Arguments:	
** Returns:
******************************************************************************/
static int sdla_ds3_tx_isr(sdla_fe_t *fe)
{
	unsigned char	value;
	
	value = READ_REG(REG_TxDS3_LAPD_STATUS);
	if (value & BIT_TxDS3_LAPD_STATUS_INT){
		DEBUG_EVENT("%s: LAPD Interrupt!\n",
				fe->name);
	}
	return 0;
}

/******************************************************************************
**				sdla_ds3_rx_isr()	
**
** Description:
** Arguments:	
** Returns:
******************************************************************************/
static int sdla_ds3_rx_isr(sdla_fe_t *fe)
{
	unsigned char	value, status;

	/* RxDS3 Interrupt status register (0x13) */
	value = READ_REG(REG_RxDS3_INT_STATUS);
	status = READ_REG(REG_RxDS3_CFG_STATUS);
	if (fe->fe_cfg.frame == WAN_FR_DS3_Cbit && value & BIT_RxDS3_INT_STATUS_CPBIT_ERR){
		if (WAN_NET_RATELIMIT()){
			DEBUG_TE3("%s: CP Bit Error interrupt detected!\n",
						fe->name);
		}
	}
	if (value & BIT_RxDS3_INT_STATUS_LOS){
		if (status & BIT_RxDS3_CFG_STATUS_RxLOS){
			if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s: LOS Status ON!\n",
						fe->name);
			}
			fe->fe_alarm |= WAN_TE3_BIT_LOS_ALARM;
		}else{
			if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s: LOS Status OFF!\n",
						fe->name);
			}
			fe->fe_alarm &= ~WAN_TE3_BIT_LOS_ALARM;
		}
	}
	if (value & BIT_RxDS3_INT_STATUS_AIS){
		if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s: AIS status %s!\n",
				fe->name,
				(status & BIT_RxDS3_CFG_STATUS_RxAIS) ? "ON" : "OFF");
		}
	}
	if (value & BIT_RxDS3_INT_STATUS_IDLE){
		if (WAN_NET_RATELIMIT()){
		DEBUG_TE3("%s: IDLE condition status %s!\n",
				fe->name,
				(status & BIT_RxDS3_CFG_STATUS_RxIDLE) ? "ON" : "OFF");
		}
	}
	if (value & BIT_RxDS3_INT_STATUS_OOF){
		if (status & BIT_RxDS3_CFG_STATUS_RxLOS){
			DEBUG_EVENT("%s: OOF Alarm ON!\n",
						fe->name);
			fe->fe_alarm |= WAN_TE3_BIT_OOF_ALARM;
		}else{
			DEBUG_EVENT("%s: OOF Alarm OFF!\n",
						fe->name);
			fe->fe_alarm &= ~WAN_TE3_BIT_OOF_ALARM;
		}
	}
	status = READ_REG(REG_RxDS3_STATUS);
	if (value & BIT_RxDS3_INT_STATUS_FERF){
		if (status & BIT_RxDS3_STATUS_RxFERF){
			DEBUG_EVENT("%s: Rx FERF status is ON (YELLOW)!\n",
					fe->name);
			fe->fe_alarm |= WAN_TE3_BIT_YEL_ALARM;
		}else{
			DEBUG_EVENT("%s: Rx FERF status is OFF!\n",
					fe->name);
			fe->fe_alarm &= ~WAN_TE3_BIT_YEL_ALARM;
		}
	}
	if (fe->fe_cfg.frame == WAN_FR_DS3_Cbit && value & BIT_RxDS3_INT_STATUS_AIC){
		if (WAN_NET_RATELIMIT()){
		DEBUG_TE3("%s: AIC bit-field status %s!\n",
				fe->name,
				(status & BIT_RxDS3_STATUS_RxAIC) ? "ON" : "OFF");
		}
	}
	if (value & BIT_RxDS3_INT_STATUS_PBIT_ERR){
		if (WAN_NET_RATELIMIT()){
		DEBUG_TE3("%s: P-Bit error interrupt!\n",
					fe->name);
		}
	}

	/* RxDS3 FEAC Interrupt (0x17) */
	value = READ_REG(REG_RxDS3_FEAC_INT);
	if (value & BIT_RxDS3_FEAC_REMOVE_INT_STATUS){
		DEBUG_TE3("%s: RxFEAC Remove Interrupt!\n",
				fe->name);
	}
	if (value & BIT_RxDS3_FEAC_VALID_INT_STATUS){
		DEBUG_TE3("%s: RxFEAC Valid Interrupt!\n",
				fe->name);
	}

	return 0;
}

/******************************************************************************
**				sdla_ds3_isr()	
**
** Description:
** Arguments:	
** Returns:
******************************************************************************/
static int sdla_ds3_isr(sdla_fe_t *fe)
{
	unsigned char	value;
	
	value = READ_REG(REG_BLOCK_INT_STATUS);
	if (value & BIT_BLOCK_INT_STATUS_RxDS3_E3){
		sdla_ds3_rx_isr(fe);
	}
	if (value & BIT_BLOCK_INT_STATUS_TxDS3_E3){
		sdla_ds3_tx_isr(fe);
	}
		
	return 0;
}

/******************************************************************************
**				sdla_e3_tx_isr()
**
** Description:
** Arguments:	
** Returns:
******************************************************************************/
static int sdla_e3_tx_isr(sdla_fe_t *fe)
{
	DEBUG_EVENT("%s: %s: This function is still not supported!\n",
			fe->name, __FUNCTION__);

	return 0;
}


/******************************************************************************
**				sdla_e3_rx_isr()
**
** Description:
** Arguments:	
** Returns:
******************************************************************************/
static int sdla_e3_rx_isr(sdla_fe_t *fe)
{
	unsigned char	int_status1, int_status2;
	unsigned char	status;

	int_status1 = READ_REG(REG_RxE3_INT_STATUS_1);
	int_status2 = READ_REG(REG_RxE3_INT_STATUS_2);
	status = READ_REG(REG_RxE3_CFG_STATUS_2);
	if (int_status1 & BIT_RxE3_INT_STATUS_OOF){
		if (status & BIT_RxE3_CFG_STATUS_RxOOF){
			DEBUG_EVENT("%s: OOF Alarm ON!\n",
						fe->name);
			fe->fe_alarm |= WAN_TE3_BIT_OOF_ALARM;
		}else{
			DEBUG_EVENT("%s: OOF Alarm OFF!\n",
						fe->name);
			fe->fe_alarm &= ~WAN_TE3_BIT_OOF_ALARM;
		}
	}

	if (int_status1 & BIT_RxE3_INT_STATUS_LOF){
		if (status & BIT_RxE3_CFG_STATUS_RxLOF){
			DEBUG_EVENT("%s: LOF Alarm ON!\n",
						fe->name);
			fe->fe_alarm |= WAN_TE3_BIT_LOF_ALARM;
		}else{
			DEBUG_EVENT("%s: LOF Alarm OFF!\n",
						fe->name);
			fe->fe_alarm &= ~WAN_TE3_BIT_LOF_ALARM;
		}
	}

	if (int_status1 & BIT_RxE3_INT_STATUS_LOS){
		if (status & BIT_RxE3_CFG_STATUS_RxLOS){
			DEBUG_EVENT("%s: LOS Alarm ON!\n",
						fe->name);
			fe->fe_alarm |= WAN_TE3_BIT_LOS_ALARM;
		}else{
			DEBUG_EVENT("%s: LOS Alarm OFF!\n",
						fe->name);
			fe->fe_alarm &= ~WAN_TE3_BIT_LOS_ALARM;
		}
	}
	
	if (int_status1 & BIT_RxE3_INT_STATUS_AIS){
		if (status & BIT_RxE3_CFG_STATUS_RxAIS){
			DEBUG_EVENT("%s: AIS Alarm ON!\n",
						fe->name);
			fe->fe_alarm |= WAN_TE3_BIT_AIS_ALARM;
		}else{
			DEBUG_EVENT("%s: AIS Alarm OFF!\n",
						fe->name);
			fe->fe_alarm &= ~WAN_TE3_BIT_AIS_ALARM;
		}
	}
	
	if (int_status2 & BIT_RxE3_INT_STATUS_FERF){
		if (status & BIT_RxE3_CFG_STATUS_RxFERF){
			DEBUG_EVENT("%s: Rx FERF status is ON (YELLOW)!\n",
					fe->name);
			fe->fe_alarm |= WAN_TE3_BIT_YEL_ALARM;
		}else{
			DEBUG_EVENT("%s: Rx FERF status is OFF!\n",
					fe->name);
			fe->fe_alarm &= ~WAN_TE3_BIT_YEL_ALARM;
		}
	}

	return 0;
}

/******************************************************************************
 *				sdla_e3_isr()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_e3_isr(sdla_fe_t *fe)
{
	unsigned char	value;
	
	value = READ_REG(REG_BLOCK_INT_STATUS);
	if (value & BIT_BLOCK_INT_STATUS_RxDS3_E3){
		sdla_e3_rx_isr(fe);
	}
	if (value & BIT_BLOCK_INT_STATUS_TxDS3_E3){
		sdla_e3_tx_isr(fe);
	}
	return -EINVAL;
}
 
/******************************************************************************
 *				sdla_te3_isr()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_te3_isr(sdla_fe_t *fe)
{
	sdla_fe_cfg_t	*fe_cfg = &fe->fe_cfg;
	unsigned char	value;
	int		err = 0;
	
	value = READ_REG(REG_BLOCK_INT_STATUS);
	switch(fe_cfg->media){
	case WAN_MEDIA_DS3:
		err = sdla_ds3_isr(fe);
		break;
	case WAN_MEDIA_E3:
		err = sdla_e3_isr(fe);
		break;
	}
	fe->fe_alarm = sdla_te3_alarm(fe, 1);
	return err;
}

/******************************************************************************
 *				sdla_te3_alarm()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static unsigned int sdla_te3_alarm(sdla_fe_t *fe, int update)
{
	sdla_fe_cfg_t	*fe_cfg = &fe->fe_cfg;
	unsigned int	alarm = 0;
	unsigned char	value;
	
	if (fe_cfg->media == WAN_MEDIA_DS3){
		value = READ_REG(REG_RxDS3_CFG_STATUS);
		if (value & BIT_RxDS3_CFG_STATUS_RxAIS){
			alarm |= WAN_TE3_BIT_AIS_ALARM;
			DEBUG_TE3("%s: (T3/E3) AIS Alarm is ON\n", fe->name);
		}else{
			alarm &= ~WAN_TE3_BIT_AIS_ALARM;
			DEBUG_TE3("%s: (T3/E3) AIS Alarm is OFF\n", fe->name);
		}	
		if (value & BIT_RxDS3_CFG_STATUS_RxLOS){
			alarm |= WAN_TE3_BIT_LOS_ALARM;
			DEBUG_TE3("%s: (T3/E3) LOS Alarm is ON\n", fe->name);
		}else{
			alarm &= ~WAN_TE3_BIT_LOS_ALARM;
			DEBUG_TE3("%s: (T3/E3) LOS Alarm is OFF\n", fe->name);
		}
		if (value & BIT_RxDS3_CFG_STATUS_RxOOF){
			alarm |= WAN_TE3_BIT_OOF_ALARM;
			DEBUG_TE3("%s: (T3/E3) OOF Alarm is ON\n", fe->name);
		}else{
			alarm &= ~WAN_TE3_BIT_OOF_ALARM;
			DEBUG_TE3("%s: (T3/E3) OOF Alarm is OFF\n", fe->name);
		}
		value = READ_REG(REG_RxDS3_STATUS);
		if (value & BIT_RxDS3_STATUS_RxFERF){
			alarm |= WAN_TE3_BIT_YEL_ALARM;
			DEBUG_TE3("%s: (T3/E3) YEL Alarm is ON\n", fe->name);
		}else{
			alarm &= ~WAN_TE3_BIT_YEL_ALARM;
			DEBUG_TE3("%s: (T3/E3) YEL Alarm is OFF\n", fe->name);
		}
	}else{
		value = READ_REG(REG_RxE3_CFG_STATUS_2);
		if (value & BIT_RxE3_CFG_STATUS_RxOOF){
			DEBUG_TE3("%s: (T3/E3) OOF Alarm ON!\n",
						fe->name);
			alarm |= WAN_TE3_BIT_OOF_ALARM;
		}else{
			DEBUG_TE3("%s: (T3/E3) OOF Alarm OFF!\n",
						fe->name);
			alarm &= ~WAN_TE3_BIT_OOF_ALARM;
		}

		if (value & BIT_RxE3_CFG_STATUS_RxLOF){
			DEBUG_TE3("%s: (T3/E3) LOF Alarm ON!\n",
						fe->name);
			alarm |= WAN_TE3_BIT_LOF_ALARM;
		}else{
			DEBUG_TE3("%s: (T3/E3) LOF Alarm OFF!\n",
						fe->name);
			alarm &= ~WAN_TE3_BIT_LOF_ALARM;
		}

		if (value & BIT_RxE3_CFG_STATUS_RxLOS){
			DEBUG_TE3("%s: (T3/E3) LOS Alarm ON!\n",
						fe->name);
			alarm |= WAN_TE3_BIT_LOS_ALARM;
		}else{
			DEBUG_TE3("%s: (T3/E3) LOS Alarm OFF!\n",
						fe->name);
			alarm &= ~WAN_TE3_BIT_LOS_ALARM;
		}
	
		if (value & BIT_RxE3_CFG_STATUS_RxAIS){
			DEBUG_TE3("%s: (T3/E3) AIS Alarm ON!\n",
						fe->name);
			alarm |= WAN_TE3_BIT_AIS_ALARM;
		}else{
			DEBUG_TE3("%s: (T3/E3) AIS Alarm OFF!\n",
						fe->name);
			alarm &= ~WAN_TE3_BIT_AIS_ALARM;
		}
	
		if (value & BIT_RxE3_CFG_STATUS_RxFERF){
			DEBUG_TE3("%s: (T3/E3) Rx FERF status is ON (YELLOW)!\n",
					fe->name);
			alarm |= WAN_TE3_BIT_YEL_ALARM;
		}else{
			DEBUG_TE3("%s: (T3/E3) Rx FERF status is OFF!\n",
					fe->name);
			alarm &= ~WAN_TE3_BIT_YEL_ALARM;
		}
	}

	fe->fe_alarm = alarm;
	if (update){
		sdla_te3_set_status(fe);
	}
	return alarm;
}


/******************************************************************************
 *				sdla_te3_set_alarm()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_te3_set_alarm(sdla_fe_t *fe, unsigned int alarm)
{
	DEBUG_EVENT("%s: %s: This function is still not supported!\n",
			fe->name, __FUNCTION__);
	return -EINVAL;
}

/******************************************************************************
 *				sdla_te3_read_pmon()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_te3_read_pmon(sdla_fe_t *fe, int action)
{
	sdla_te3_pmon_t	*pmon = (sdla_te3_pmon_t*)&fe->fe_stats.u.te3_pmon;
	unsigned char value_msb, value_lsb;

	value_msb = READ_REG(REG_PMON_LCV_MSB);
	value_lsb = READ_REG(REG_PMON_LCV_LSB);
	pmon->pmon_lcv += ((value_msb << 8) | value_lsb);

	value_msb = READ_REG(REG_PMON_FRAMING_ERR_CNT_MSB);
	value_lsb = READ_REG(REG_PMON_FRAMING_ERR_CNT_LSB);
	pmon->pmon_framing += ((value_msb << 8) | value_lsb); 

	value_msb = READ_REG(REG_PMON_PARITY_ERR_CNT_MSB);
	value_lsb = READ_REG(REG_PMON_PARITY_ERR_CNT_LSB);
	pmon->pmon_parity += ((value_msb << 8) | value_lsb); 

	value_msb = READ_REG(REG_PMON_FEBE_EVENT_CNT_MSB);
	value_lsb = READ_REG(REG_PMON_FEBE_EVENT_CNT_LSB);
	pmon->pmon_febe += ((value_msb << 8) | value_lsb); 

	value_msb = READ_REG(REG_PMON_CPBIT_ERROR_CNT_MSB);
	value_lsb = READ_REG(REG_PMON_CPBIT_ERROR_CNT_LSB);
	pmon->pmon_cpbit += ((value_msb << 8) | value_lsb); 
	return 0;
}



/******************************************************************************
*				sdla_te3_flush_pmon()	
*
* Description:
* Arguments:
* Returns:
******************************************************************************/
static int sdla_te3_flush_pmon(sdla_fe_t *fe)
{
	sdla_te3_pmon_t	*pmon = (sdla_te3_pmon_t*)&fe->fe_stats.u.te3_pmon;

	pmon->pmon_lcv		= 0;
	pmon->pmon_framing	= 0; 
	pmon->pmon_parity	= 0; 
	pmon->pmon_febe		= 0; 
	pmon->pmon_cpbit  	= 0; 
	return 0;
}

/******************************************************************************
*				sdla_te3_old_set_lb_modes()	
*
* Description:
* Arguments:
* Returns:
******************************************************************************/
static int 
sdla_te3_old_set_lb_modes(sdla_fe_t *fe, unsigned char type, unsigned char mode)
{

	WAN_ASSERT(fe->write_cpld == NULL);
	DEBUG_EVENT("%s: %s %s mode...\n",
			fe->name,
			WAN_TE3_LB_MODE_DECODE(mode),
			WAN_TE3_LB_TYPE_DECODE(type));

	if (mode == WAN_TE3_DEACTIVATE_LB){
		fe->te3_param.cpld_status &= ~BIT_CPLD_STATUS_LLB;
		fe->te3_param.cpld_status &= ~BIT_CPLD_STATUS_RLB;
	}else{
		switch(type){
		case WAN_TE3_LIU_LB_ANALOG:
			fe->te3_param.cpld_status |= BIT_CPLD_STATUS_LLB;
			fe->te3_param.cpld_status &= ~BIT_CPLD_STATUS_RLB;
			break;
		case WAN_TE3_LIU_LB_REMOTE:
			fe->te3_param.cpld_status &= ~BIT_CPLD_STATUS_LLB;
			fe->te3_param.cpld_status |= BIT_CPLD_STATUS_RLB;
			break;
		case WAN_TE3_LIU_LB_DIGITAL:
			fe->te3_param.cpld_status |= BIT_CPLD_STATUS_LLB;
			fe->te3_param.cpld_status |= BIT_CPLD_STATUS_RLB;
			break;
		default :
			DEBUG_EVENT("%s: (T3/E3) Unknown loopback mode!\n",
					fe->name);
			break;
		}		
	}
	/* Write value to CPLD Status/Control register */
	WRITE_CPLD(REG_CPLD_STATUS, fe->te3_param.cpld_status);
	return 0;
}
 
static int 
sdla_te3_set_lb_modes(sdla_fe_t *fe, unsigned char type, unsigned char mode) 
{
	unsigned char	data = 0x00;

	DEBUG_EVENT("%s: %s %s mode...\n",
			fe->name,
			WAN_TE3_LB_MODE_DECODE(mode),
			WAN_TE3_LB_TYPE_DECODE(type));

	data = READ_REG(REG_LINE_INTERFACE_DRIVE);
	if (mode == WAN_TE3_DEACTIVATE_LB){
		data &= ~BIT_LINE_INTERFACE_DRIVE_LLOOP;
		data &= ~BIT_LINE_INTERFACE_DRIVE_RLOOP;
	}else{
		switch(type){
		case WAN_TE3_LIU_LB_NORMAL:
			break;
		case WAN_TE3_LIU_LB_ANALOG:
			data |= BIT_LINE_INTERFACE_DRIVE_LLOOP;
			data &= ~BIT_LINE_INTERFACE_DRIVE_RLOOP;
			break;
		case WAN_TE3_LIU_LB_REMOTE:
			data &= ~BIT_LINE_INTERFACE_DRIVE_LLOOP;
			data |= BIT_LINE_INTERFACE_DRIVE_RLOOP;
			break;
		case WAN_TE3_LIU_LB_DIGITAL:
			data |= BIT_LINE_INTERFACE_DRIVE_LLOOP;
			data |= BIT_LINE_INTERFACE_DRIVE_RLOOP;
			break;
		default :
			DEBUG_EVENT("%s: (T3/E3) Unknown loopback mode!\n",
					fe->name);
			break;
		}		
	}
	WRITE_REG(REG_LINE_INTERFACE_DRIVE, data);
	return 0;
}


/******************************************************************************
 *				sdla_te3_udp()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_te3_udp(sdla_fe_t *fe, void *pudp_cmd, unsigned char *data)
{
	sdla_t		*card = (sdla_t*)fe->card;
	wan_cmd_t	*udp_cmd = (wan_cmd_t*)pudp_cmd;
	int		err = -EINVAL;

	switch(udp_cmd->wan_cmd_command){

	case WAN_GET_MEDIA_TYPE:
		data[0] = fe->fe_cfg.media;
		udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		udp_cmd->wan_cmd_data_len = sizeof(unsigned char); 
		break;

	case WAN_FE_SET_LB_MODE:
		/* Activate/Deactivate Line Loopback modes */
		if (card->adptr_subtype == AFT_SUBTYPE_NORMAL){
			err = sdla_te3_old_set_lb_modes(fe, data[0], data[1]); 
		}else if (card->adptr_subtype == AFT_SUBTYPE_SHARK){
			err = sdla_te3_set_lb_modes(fe, data[0], data[1]); 
		}
	    	udp_cmd->wan_cmd_return_code = 
				(!err) ? WAN_CMD_OK : WAN_UDP_FAILED_CMD;
	    	udp_cmd->wan_cmd_data_len = 0x00;
		break;

	case WAN_FE_GET_STAT:
 	        /* Read T3/E3 alarms */
    		sdla_te3_read_pmon(fe, 0);
		if (udp_cmd->wan_cmd_fe_force){
			/* force to read FE alarms */
			DEBUG_EVENT("%s: Force to read Front-End alarms\n",
						fe->name);
			fe->fe_stats.alarms = 
				sdla_te3_alarm(fe, 1);
		}
	        memcpy(&data[0], &fe->fe_stats, sizeof(sdla_fe_stats_t));
	        udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
	    	udp_cmd->wan_cmd_data_len = sizeof(sdla_fe_stats_t); 
		break;

 	case WAN_FE_FLUSH_PMON:
		/* TE1 Flush T1/E1 pmon counters */
		sdla_te3_flush_pmon(fe);
	        udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		break;
 
	case WAN_FE_GET_CFG:
		/* Read T1/E1 configuration */
	       	memcpy(&data[0],
	    		&fe->fe_cfg,
		      	sizeof(sdla_fe_cfg_t));
	    	udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
    	    	udp_cmd->wan_cmd_data_len = sizeof(sdla_te_cfg_t);
	    	break;

	default:
		udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
	    	udp_cmd->wan_cmd_data_len = 0;
		break;
	}
	return 0;
}


static int sdla_te3_set_intr(sdla_fe_t *fe)
{
	sdla_fe_cfg_t	*fe_cfg = &fe->fe_cfg;

	DEBUG_EVENT("%s: Enabling interrupts for %s (%d)!\n",
					fe->name, FE_MEDIA_DECODE(fe), IS_DS3(fe_cfg));
	/* Enable Framer Interrupts */
	/* 1. Block Interrupt Enable */
	WRITE_REG(REG_BLOCK_INT_ENABLE,
			BIT_BLOCK_INT_ENABLE_RxDS3_E3 |
			BIT_BLOCK_INT_ENABLE_TxDS3_E3);
	if (IS_DS3(fe_cfg)){
		/* 1. RxDS3 Interrupt Enable */
		WRITE_REG(REG_RxDS3_INT_ENABLE, 
				BIT_RxDS3_INT_ENABLE_CPBIT_ERR	|
				BIT_RxDS3_INT_ENABLE_LOS 	|
				BIT_RxDS3_INT_ENABLE_OOF	|
				BIT_RxDS3_INT_ENABLE_AIS	|
				BIT_RxDS3_INT_ENABLE_IDLE	|
				BIT_RxDS3_INT_ENABLE_FERF	|
				BIT_RxDS3_INT_ENABLE_AIC	|
				BIT_RxDS3_INT_ENABLE_PBIT_ERR	);

		/* RxDS3 FEAC */
		WRITE_REG(REG_RxDS3_FEAC_INT, 
				BIT_RxDS3_FEAC_REMOVE_INT_EN	|
				BIT_RxDS3_FEAC_VALID_INT_EN);

		/* RxDS3 LAPD */
		WRITE_REG(REG_TxDS3_LAPD_STATUS, 
				BIT_TxDS3_LAPD_STATUS_INT_EN);

	}else if (IS_E3(fe_cfg)){
		/* RxE3 Interrupt Enable 1 (0x12) */
		WRITE_REG(REG_RxE3_INT_ENABLE_1,
				BIT_RxE3_INT_ENABLE_OOF	|
				BIT_RxE3_INT_ENABLE_LOS	|
				BIT_RxE3_INT_ENABLE_LOF	|
				BIT_RxE3_INT_ENABLE_AIS);

		/* RxE3 Interrupt Enable 2 (0x13) */
		WRITE_REG(REG_RxE3_INT_ENABLE_2,
				BIT_RxE3_INT_ENABLE_FERF	|
				BIT_RxE3_INT_ENABLE_FRAMING);
	}else{
		return -EINVAL;
	}

	return 0;
}


/******************************************************************************
 *				sdla_te3_liu_config()	
 *
 * Description: Configure Sangoma TE3 board
 * Arguments:	
 * Returns:	WAN_TRUE - TE3 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
static int 
sdla_te3_liu_config(sdla_fe_t *fe, sdla_te3_liu_cfg_t *liu, char *name)
{
	sdla_fe_cfg_t	*fe_cfg = &fe->fe_cfg;
	unsigned char	data = 0x00;

	if (fe_cfg->media == WAN_MEDIA_E3){
		data |= BIT_CPLD_CNTRL_E3;
	}
	if (liu->rx_equal == WAN_TRUE){
		DEBUG_TE3("%s: (T3/E3) Enable Receive Equalizer\n",
				name);
		data |= BIT_CPLD_CNTRL_REQEN;
	}else{
		DEBUG_TE3("%s: (T3/E3) Disable Receive Equalizer\n",
				name);
		data &= ~BIT_CPLD_CNTRL_REQEN;
	}
	if (liu->tx_lbo == WAN_TRUE){
		DEBUG_TE3("%s: (T3/E2) Enable Transmit Build-out\n",
				name);
		data |= BIT_CPLD_CNTRL_TxLEV;
	}else{
		DEBUG_TE3("%s: (T3/E3) Disable Transmit Build-out\n",
				name);
		data &= ~BIT_CPLD_CNTRL_TxLEV;
	}
	/* Write value to CPLD Control register */
	WRITE_CPLD(REG_CPLD_CNTRL, data);
	
	data = 0x00;
	if (liu->taos == WAN_TRUE){
		DEBUG_TE3("%s: (T3/E3) Enable Transmit All Ones\n",
				name);
		data |= BIT_CPLD_STATUS_TAOS;
	}else{
		DEBUG_TE3("%s: (T3/E3) Disable Transmit All Ones\n",
				name);
		data &= ~BIT_CPLD_STATUS_TAOS;
	}
	switch(liu->lb_mode){
	case WAN_TE3_LIU_LB_NORMAL:
		break;
	case WAN_TE3_LIU_LB_ANALOG:
		DEBUG_TE3("%s: (T3/E3) Enable Analog Loopback mode!\n",
				name);
		data |= BIT_CPLD_STATUS_LLB;
		data &= ~BIT_CPLD_STATUS_RLB;
		break;
	case WAN_TE3_LIU_LB_REMOTE:
		DEBUG_TE3("%s: (T3/E3) Enable Remote Loopback mode!\n",
				name);
		data &= ~BIT_CPLD_STATUS_LLB;
		data |= BIT_CPLD_STATUS_RLB;
		break;
	case WAN_TE3_LIU_LB_DIGITAL:
		DEBUG_TE3("%s: (T3/E3) Enable Digital Loopback mode!\n",
				name);
		data |= BIT_CPLD_STATUS_LLB;
		data |= BIT_CPLD_STATUS_RLB;
		break;
	default :
		DEBUG_EVENT("%s: (T3/E3) Unknown loopback mode!\n",
				name);
		break;
	}		
	/* Write value to CPLD Status/Control register */
	WRITE_CPLD(REG_CPLD_STATUS, data);
	return 0;
}

/******************************************************************************
 *				sdla_te3_shark_liu_config()	
 *
 * Description: Configure Sangoma TE3 Shark board
 * Arguments:	
 * Returns:	WAN_TRUE - TE3 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
static int 
sdla_te3_shark_liu_config(sdla_fe_t *fe, sdla_te3_liu_cfg_t *liu, char *name)
{
	sdla_fe_cfg_t	*fe_cfg = &fe->fe_cfg;
	unsigned char	data = 0x00;

	if (fe_cfg->media == WAN_MEDIA_E3){
		data |= BIT_EXAR_CPLD_CNTRL_E3;
	}
	/* Write value to CPLD Control register */
	WRITE_EXAR_CPLD(REG_EXAR_CPLD_CNTRL, data);
	
	data = 0x00;
	if (liu->rx_equal == WAN_TRUE){
		DEBUG_TE3("%s: (T3/E3) Enable Receive Equalizer\n",
				name);
		data |= BIT_LINE_INTERFACE_DRIVE_REQB;
	}else{
		DEBUG_TE3("%s: (T3/E3) Disable Receive Equalizer\n",
				name);
		data &= ~BIT_LINE_INTERFACE_DRIVE_REQB;
	}
	if (liu->tx_lbo == WAN_TRUE){
		DEBUG_TE3("%s: (T3/E2) Enable Transmit Build-out\n",
				name);
		data |= BIT_LINE_INTERFACE_DRIVE_TxLEV;
	}else{
		DEBUG_TE3("%s: (T3/E3) Disable Transmit Build-out\n",
				name);
		data &= ~BIT_LINE_INTERFACE_DRIVE_TxLEV;
	}
	if (liu->taos == WAN_TRUE){
		DEBUG_TE3("%s: (T3/E3) Enable Transmit All Ones\n",
				name);
		data |= BIT_LINE_INTERFACE_DRIVE_TAOS;
	}else{
		DEBUG_TE3("%s: (T3/E3) Disable Transmit All Ones\n",
				name);
		data &= ~BIT_LINE_INTERFACE_DRIVE_TAOS;
	}
	
	switch(liu->lb_mode){
	case WAN_TE3_LIU_LB_NORMAL:
		break;
	case WAN_TE3_LIU_LB_ANALOG:
		DEBUG_TE3("%s: (T3/E3) Enable Analog Loopback mode!\n",
				name);
		data |= BIT_LINE_INTERFACE_DRIVE_LLOOP;
		data &= ~BIT_LINE_INTERFACE_DRIVE_RLOOP;
		break;
	case WAN_TE3_LIU_LB_REMOTE:
		DEBUG_TE3("%s: (T3/E3) Enable Remote Loopback mode!\n",
				name);
		data &= ~BIT_LINE_INTERFACE_DRIVE_LLOOP;
		data |= BIT_LINE_INTERFACE_DRIVE_RLOOP;
		break;
	case WAN_TE3_LIU_LB_DIGITAL:
		DEBUG_TE3("%s: (T3/E3) Enable Digital Loopback mode!\n",
				name);
		data |= BIT_LINE_INTERFACE_DRIVE_LLOOP;
		data |= BIT_LINE_INTERFACE_DRIVE_RLOOP;
		break;
	default :
		DEBUG_EVENT("%s: (T3/E3) Unknown loopback mode!\n",
				name);
		break;
	}		
	WRITE_REG(REG_LINE_INTERFACE_DRIVE, data);
	
	return 0;
}


int sdla_te3_iface_init(void *p_fe_iface)
{
	sdla_fe_iface_t	*fe_iface = (sdla_fe_iface_t*)p_fe_iface;

	/* Inialize Front-End interface functions */
	fe_iface->config		= &sdla_te3_config;
	fe_iface->unconfig		= &sdla_te3_unconfig;
	fe_iface->polling		= &sdla_te3_polling;
	fe_iface->isr			= &sdla_te3_isr;
	fe_iface->process_udp		= &sdla_te3_udp;
	fe_iface->read_alarm		= &sdla_te3_alarm;
	fe_iface->read_pmon		= &sdla_te3_read_pmon;
	fe_iface->set_fe_alarm		= &sdla_te3_set_alarm;
	fe_iface->get_fe_status		= &sdla_te3_get_fe_status;
	fe_iface->get_fe_media		= &sdla_te3_get_fe_media;
	fe_iface->get_fe_media_string	= &sdla_te3_get_fe_media_string;
	fe_iface->update_alarm_info	= &sdla_te3_update_alarm_info;
	fe_iface->update_pmon_info	= &sdla_te3_update_pmon_info;

	return 0;
}
static int sdla_te3_config(void *p_fe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)p_fe;
	sdla_t*		card = (sdla_t*)fe->card;
	sdla_fe_cfg_t	*fe_cfg = &fe->fe_cfg;
	sdla_te3_cfg_t	*te3_cfg = &fe_cfg->cfg.te3_cfg;
	u16		adptr_subtype;
	unsigned char	data = 0x00;
	
	card->hw_iface.getcfg(card->hw, SDLA_ADAPTERSUBTYPE, &adptr_subtype);
	
	data = READ_REG(0x02);
	
	/* configure Line Interface Unit */
	if (card->adptr_subtype == AFT_SUBTYPE_NORMAL){
		sdla_te3_liu_config(fe, &te3_cfg->liu_cfg, fe->name);
	}else if (card->adptr_subtype == AFT_SUBTYPE_SHARK){
		sdla_te3_shark_liu_config(fe, &te3_cfg->liu_cfg, fe->name);	
	}else{
		DEBUG_EVENT("%s: Unknown Adapter Subtype (%X)\n",
				fe->name, card->adptr_subtype);
		return -EINVAL;
	}

	switch(fe_cfg->media){
	case WAN_MEDIA_DS3:
		DEBUG_TE3("%s: (T3/E3) Media type DS3\n",
				fe->name);
		data |= BIT_OPMODE_DS3;
		break;
	case WAN_MEDIA_E3:
		DEBUG_TE3("%s: (T3/E3) Media type E3\n",
				fe->name);
		data &= ~BIT_OPMODE_DS3;
		break;
	default:
		DEBUG_EVENT("%s: Invalid Media type 0x%X\n",
				fe->name,fe_cfg->media);
		return -EINVAL;
	}

	switch(fe_cfg->frame){
	case WAN_FR_E3_G751:
		if (fe_cfg->media != WAN_MEDIA_E3){
			DEBUG_EVENT("%s: (T3/E3) Invalid Frame Format!\n",
					fe->name);
			return -EINVAL;
		}
		DEBUG_TE3("%s: (T3/E3) Frame type G.751\n",
				fe->name);
		data &= ~BIT_OPMODE_FRAME_FRMT;
		break;
	case WAN_FR_E3_G832:
		if (fe_cfg->media != WAN_MEDIA_E3){
			DEBUG_EVENT("%s: (T3/E3) Invalid Frame Format!\n",
					fe->name);
			return -EINVAL;
		}
		DEBUG_TE3("%s: (T3/E3) Frame type G.832\n",
				fe->name);
		data |= BIT_OPMODE_FRAME_FRMT;
		break;
	case WAN_FR_DS3_Cbit:
		if (fe_cfg->media != WAN_MEDIA_DS3){
			DEBUG_EVENT("%s: (T3/E3) Invalid Frame Format!\n",
					fe->name);
			return -EINVAL;
		}
		DEBUG_TE3("%s: (T3/E3) Frame type C-bit Parity\n",
				fe->name);
		data &= ~BIT_OPMODE_FRAME_FRMT;
		break;
	case WAN_FR_DS3_M13:
		if (fe_cfg->media != WAN_MEDIA_DS3){
			DEBUG_EVENT("%s: (T3/E3) Invalid Frame Format!\n",
					fe->name);
			return -EINVAL;
		}
		DEBUG_TE3("%s: (T3/E3) Frame type M13\n",
				fe->name);
		data |= BIT_OPMODE_FRAME_FRMT;
		break;
	default:
		DEBUG_EVENT("%s: Invalid Frame type 0x%X\n",
				fe->name,fe_cfg->frame);
		return -EINVAL;
	}
	data |= BIT_OPMODE_INTERNAL_LOS;
	data |= (BIT_OPMODE_TIMREFSEL1 | BIT_OPMODE_TIMREFSEL0);
	WRITE_REG(REG_OPMODE, data);

	data = 0x00;
	switch(fe_cfg->lcode){
	case WAN_LCODE_AMI: 
		DEBUG_TE3("%s: (T3/E3) Line code AMI\n",
				fe->name);
		data |= BIT_IO_CONTROL_AMI;
		break;
	case WAN_LCODE_HDB3:
		DEBUG_TE3("%s: (T3/E3) Line code HDB3\n",
				fe->name);
		data &= ~BIT_IO_CONTROL_AMI;
		break;
	case WAN_LCODE_B3ZS:
		DEBUG_TE3("%s: (T3/E3) Line code B3ZS\n",
				fe->name);
		data &= ~BIT_IO_CONTROL_AMI;
		break;
	default:
		DEBUG_EVENT("%s: Invalid Lcode 0x%X\n",
				fe->name,fe_cfg->lcode);
		return -EINVAL;
	}
	data |= BIT_IO_CONTROL_DISABLE_TXLOC;
	data |= BIT_IO_CONTROL_DISABLE_RXLOC;
	data |= BIT_IO_CONTROL_RxLINECLK;
	WRITE_REG(REG_IO_CONTROL, data);

	/* Initialize Front-End parameters */
	fe->fe_status	= FE_DISCONNECTED;
	DEBUG_EVENT("%s: %s disconnected!\n",
					fe->name, FE_MEDIA_DECODE(fe));
	sdla_te3_alarm(fe, 1);

	sdla_te3_set_intr(fe);
	return 0;
}

static int sdla_te3_unconfig(void *p_fe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)p_fe;
	DEBUG_EVENT("%s: Unconfiguring T3/E3 interface\n",
			fe->name);
	return 0;
}

static int
sdla_te3_update_alarm_info(sdla_fe_t* fe, struct seq_file* m, int* stop_cnt)
{
	if (IS_DS3(&fe->fe_cfg)){
		PROC_ADD_LINE(m,
			"=============================== DS3 Alarms ===============================\n");
		PROC_ADD_LINE(m,
			PROC_STATS_ALARM_FORMAT,
			"AIS", WAN_TE3_AIS_ALARM(fe->fe_alarm), 
			"LOS", WAN_TE3_LOS_ALARM(fe->fe_alarm));
		PROC_ADD_LINE(m,
			PROC_STATS_ALARM_FORMAT,
			"OOF", WAN_TE3_OOF_ALARM(fe->fe_alarm), 
			"YEL", WAN_TE3_YEL_ALARM(fe->fe_alarm));
	}else{
		PROC_ADD_LINE(m,
			"=============================== E3 Alarms ===============================\n");
		PROC_ADD_LINE(m,
			PROC_STATS_ALARM_FORMAT,
			"AIS", WAN_TE3_AIS_ALARM(fe->fe_alarm), 
			"LOS", WAN_TE3_LOS_ALARM(fe->fe_alarm));
		PROC_ADD_LINE(m,
			PROC_STATS_ALARM_FORMAT,
			"OFF", WAN_TE3_OOF_ALARM(fe->fe_alarm), 
			"YEL", WAN_TE3_YEL_ALARM(fe->fe_alarm));
		PROC_ADD_LINE(m,
			PROC_STATS_ALARM_FORMAT,
			"LOF", WAN_TE3_LOF_ALARM(fe->fe_alarm), 
			"", "");
	}
		
	return m->count;
}

static int sdla_te3_update_pmon_info(sdla_fe_t* fe, struct seq_file* m, int* stop_cnt)
{
	PROC_ADD_LINE(m,
		 "=========================== %s PMON counters ============================\n",
		 (IS_DS3(&fe->fe_cfg)) ? "DS3" : "E3");
	PROC_ADD_LINE(m,
		PROC_STATS_PMON_FORMAT,
		"Line Code Violation", fe->fe_stats.u.te3_pmon.pmon_lcv,
		"Framing Bit/Byte Error", fe->fe_stats.u.te3_pmon.pmon_framing);
	if (IS_DS3(&fe->fe_cfg)){
		if (fe->fe_cfg.frame == WAN_FR_DS3_Cbit){
			PROC_ADD_LINE(m,
				PROC_STATS_PMON_FORMAT,
				"Parity Error", fe->fe_stats.u.te3_pmon.pmon_parity,
				"CP-Bit Error Event", fe->fe_stats.u.te3_pmon.pmon_cpbit);
		}else{
			PROC_ADD_LINE(m,
				PROC_STATS_PMON_FORMAT,
				"Parity Error", fe->fe_stats.u.te3_pmon.pmon_parity,
				"FEBE Event", fe->fe_stats.u.te3_pmon.pmon_febe);

		}
	}else{
		PROC_ADD_LINE(m,
			PROC_STATS_PMON_FORMAT,
			"Parity Error", fe->fe_stats.u.te3_pmon.pmon_parity,
			"FEBE Event", fe->fe_stats.u.te3_pmon.pmon_febe);
	}
	
	return m->count;
}
