/*
 * Copyright (c) 2006
 *	Alex Feldman <al.feldman@sangoma.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Alex Feldman.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Alex Feldman AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Alex Feldman OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	$Id: sdla_8te1.c,v 1.66 2007/04/10 16:28:49 sangoma Exp $
 */

/*
 ******************************************************************************
 * sdla_8te1.c	WANPIPE(tm) Multiprotocol WAN Link Driver. 
 *				8 ports T1/E1 board configuration.
 *
 * Author: 	Alex Feldman  <al.feldman@sangoma.com>
 *
 * ============================================================================
 * Feb 18, 2006	Alex Feldma	Initial version.
 ******************************************************************************
 */

/******************************************************************************
*			   INCLUDE FILES
******************************************************************************/
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
#  include <wanpipe_snmp.h>
# endif
# include <sdla_te1_ds.h>
# include <wanpipe.h>	/* WANPIPE common user API definitions */
# include <wanproc.h>
#elif (defined __WINDOWS__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <sdla_te1_ds.h>
# include <wanpipe.h>	/* WANPIPE common user API definitions */

#define _DEBUG_EVENT	DBG_8TE1

#elif (defined __LINUX__) || (defined __KERNEL__)
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanproc.h>
# if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
#  include <linux/wanpipe_snmp.h>
# endif
# include <linux/sdla_te1_ds.h>
# include <linux/wanpipe.h>	/* WANPIPE common user API definitions */
#else
# error "No OS Defined"
#endif

/******************************************************************************
*			  DEFINES AND MACROS
******************************************************************************/
#define CLEAR_REG(sreg,ereg) {				\
	unsigned short reg;				\
	for(reg = sreg; reg < ereg; reg++){		\
		WRITE_REG(reg, 0x00);			\
	}						\
	}

#define IS_GLREG(reg)	((reg) >= 0xF0 && (reg) <= 0xFF)
#define IS_FRREG(reg)	((reg) <= 0x1F0)
#define IS_LIUREG(reg)	((reg) >= 0x1000 && (reg) <= 0x101F)
#define IS_BERTREG(reg)	((reg) >= 0x1100 && (reg) <= 0x110F)

#define DLS_PORT_DELTA(reg)			\
		IS_GLREG(reg)	? 0x000 :	\
		IS_FRREG(reg)	? 0x200 :	\
		IS_LIUREG(reg)	? 0x020 :	\
		IS_BERTREG(reg)	? 0x010 : 0x001

#if 1
/* Read/Write to front-end register */
#define WRITE_REG(reg,val)						\
	fe->write_fe_reg(						\
		fe->card, 						\
		fe->fe_cfg.line_no,					\
		(int)sdla_ds_te1_address(fe,fe->fe_cfg.line_no,(reg)),	\
		(int)(val))

#define WRITE_REG_LINE(fe_line_no, reg,val)				\
	fe->write_fe_reg(						\
		fe->card,						\
		fe_line_no,						\
		(int)sdla_ds_te1_address(fe,fe_line_no,(reg)),		\
		(int)(val))
	
#define READ_REG(reg)							\
	fe->read_fe_reg(						\
		fe->card,						\
		fe->fe_cfg.line_no,					\
		(int)sdla_ds_te1_address(fe,fe->fe_cfg.line_no,(reg)))
	
#define __READ_REG(reg)							\
	fe->__read_fe_reg(						\
		fe->card,						\
		fe->fe_cfg.line_no,					\
		(int)sdla_ds_te1_address(fe,fe->fe_cfg.line_no,(reg)))
	
#define READ_REG_LINE(fe_line_no, reg)					\
	fe->read_fe_reg(						\
		fe->card,						\
		fe_line_no,						\
		(int)sdla_ds_te1_address(fe,fe_line_no,(reg)))

#else		
/* Read/Write to front-end register */
#define WRITE_REG(reg,val)						\
	fe->write_fe_reg(						\
		fe->card,						\
		(int)((reg) + ((fe->fe_cfg.line_no)*(DLS_PORT_DELTA(reg)))),\
		(int)(val))

#define WRITE_REG_LINE(fe_line_no, reg,val)				\
	fe->write_fe_reg(						\
		fe->card,						\
			(int)((reg) + (fe_line_no)*(DLS_PORT_DELTA(reg))),	\
		(int)(val))
	
#define READ_REG(reg)							\
	fe->read_fe_reg(						\
		fe->card,						\
		(int)((reg) + ((fe->fe_cfg.line_no)*(DLS_PORT_DELTA(reg)))))
	
#define __READ_REG(reg)							\
	fe->__read_fe_reg(						\
		fe->card,						\
		(int)((reg) + ((fe->fe_cfg.line_no)*(DLS_PORT_DELTA(reg)))))
	
#define READ_REG_LINE(fe_line_no, reg)					\
	fe->read_fe_reg(						\
		fe->card,						\
		(int)((reg) + (fe_line_no)*(DLS_PORT_DELTA(reg))))
#endif


#define WAN_TE1_FRAMED_ALARMS		(WAN_TE_BIT_RED_ALARM |	WAN_TE_BIT_OOF_ALARM)
#define WAN_TE1_UNFRAMED_ALARMS		(WAN_TE_BIT_RED_ALARM)

#define IS_T1_ALARM(alarm)			\
		(alarm & 			\
			(			\
			 WAN_TE_BIT_RED_ALARM |	\
			 WAN_TE_BIT_AIS_ALARM |	\
			 WAN_TE_BIT_OOF_ALARM |	\
			 WAN_TE_BIT_LOS_ALARM |	\
			 WAN_TE_BIT_ALOS_ALARM	\
			 )) 

#define IS_E1_ALARM(alarm)			\
		(alarm &	 		\
			(			\
			 WAN_TE_BIT_RED_ALARM | \
			 WAN_TE_BIT_AIS_ALARM | \
			 WAN_TE_BIT_OOF_ALARM | \
			 WAN_TE_BIT_LOS_ALARM | \
			 WAN_TE_BIT_ALOS_ALARM 	\
			 ))



#define WAN_DS_REGBITMAP(fe)	(((fe)->fe_chip_id==DEVICE_ID_DS26521)?0:WAN_FE_LINENO((fe)))

/******************************************************************************
*			  STRUCTURES AND TYPEDEFS
******************************************************************************/


/******************************************************************************
*			  GLOBAL VERIABLES
******************************************************************************/
char *wan_t1_ds_rxlevel[] = {
	"> -2.5db",	
	"-2.5db to -5db",	
	"-5db to -7.5db",	
	"-7.5db to -10db",	
	"-10db to -12.5db",	
	"-12.5db to -15db",	
	"-15db to -17.5db",	
	"-17.5db to -20db",	
	"-20db to -23db",	
	"-23db to -26db",	
	"-26db to -29db",	
	"-29db to -32db",	
	"-32db to -36db",	
	"< -36db",	
	"",	
	""	
};

char *wan_e1_ds_rxlevel[] = {
	"> -2.5db",	
	"-2.5db to -5db",	
	"-5db to -7.5db",	
	"-7.5db to -10db",	
	"-10db to -12.5db",	
	"-12.5db to -15db",	
	"-15db to -17.5db",	
	"-17.5db to -20db",	
	"-20db to -23db",	
	"-23db to -26db",	
	"-26db to -29db",	
	"-29db to -32db",	
	"-32db to -36db",	
	"-36db to -40db",	
	"-40db to -44db",	
	"< -44db"
};

/******************************************************************************
*			  FUNCTION PROTOTYPES
******************************************************************************/
static int sdla_ds_te1_reset(void* pfe, int port_no, int reset);
static int sdla_ds_te1_global_config(void* pfe);	/* Change to static */
static int sdla_ds_te1_global_unconfig(void* pfe);	/* Change to static */
static int sdla_ds_te1_chip_config(void* pfe);
static int sdla_ds_te1_config(void* pfe);	/* Change to static */
static int sdla_ds_te1_reconfig(sdla_fe_t* fe);
static int sdla_ds_te1_post_init(void *pfe);
static int sdla_ds_te1_unconfig(void* pfe);	/* Change to static */
static int sdla_ds_te1_pre_release(void* pfe);
static int sdla_ds_te1_TxChanCtrl(sdla_fe_t* fe, int channel, int enable);
static int sdla_ds_te1_RxChanCtrl(sdla_fe_t* fe, int channel, int enable);
static int sdla_ds_te1_disable_irq(void* pfe);	/* Change to static */
static int sdla_ds_te1_intr_ctrl(sdla_fe_t*, int, int, int, unsigned int); 
static int sdla_ds_te1_check_intr(sdla_fe_t *fe); 
static int sdla_ds_te1_intr(sdla_fe_t *fe); 
static int sdla_ds_te1_udp(sdla_fe_t *fe, void* p_udp_cmd, unsigned char* data);
static int sdla_ds_te1_flush_pmon(sdla_fe_t *fe);
static int sdla_ds_te1_pmon(sdla_fe_t *fe, int action);
static int sdla_ds_te1_rxlevel(sdla_fe_t* fe);
static int sdla_ds_te1_polling(sdla_fe_t* fe);
static unsigned int sdla_ds_te1_read_alarms(sdla_fe_t *fe, int read);
static int sdla_ds_te1_set_alarms(sdla_fe_t* fe, unsigned long alarms);
static int sdla_ds_te1_clear_alarms(sdla_fe_t* fe, unsigned long alarms);
static void sdla_ds_te1_set_status(sdla_fe_t* fe, unsigned long alarms);
static int sdla_ds_te1_print_alarms(sdla_fe_t*, unsigned int);
static int sdla_ds_te1_set_lbmode(sdla_fe_t*, unsigned char, unsigned char); 
static int sdla_ds_te1_rbs_update(sdla_fe_t* fe, int, unsigned char);
static int sdla_ds_te1_set_rbsbits(sdla_fe_t *fe, int, unsigned char);
static int sdla_ds_te1_rbs_report(sdla_fe_t* fe);
static int sdla_ds_te1_check_rbsbits(sdla_fe_t* fe, int, unsigned int, int);
static unsigned char sdla_ds_te1_read_rbsbits(sdla_fe_t* fe, int, int);
static int sdla_ds_te1_add_event(sdla_fe_t*, sdla_fe_timer_event_t*);
static int sdla_ds_te1_add_timer(sdla_fe_t*, unsigned long);

//static void sdla_ds_te1_enable_timer(sdla_fe_t*, unsigned char, unsigned long);
static int sdla_ds_te1_sigctrl(sdla_fe_t *fe, int, unsigned long, int);

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static void sdla_ds_te1_timer(void* pfe);
#elif defined(__WINDOWS__)
static void sdla_ds_te1_timer(IN PKDPC Dpc, void* pfe, void* arg2, void* arg3);
#else
static void sdla_ds_te1_timer(unsigned long pfe);
#endif

static int sdla_ds_te1_update_alarm_info(sdla_fe_t*, struct seq_file*, int*);
static int sdla_ds_te1_update_pmon_info(sdla_fe_t*, struct seq_file*, int*);

/******************************************************************************
*			  FUNCTION DEFINITIONS
******************************************************************************/

/******************************************************************************
 *				sdla_te3_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static char* sdla_ds_te1_get_fe_media_string(void)
{
	return ("AFT-A108 T1/E1");
}

/******************************************************************************
 *				sdla_ds_te1_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static unsigned char sdla_ds_te1_get_fe_media(sdla_fe_t *fe)
{
	return fe->fe_cfg.media;
}

/******************************************************************************
 *				sdla_ds_te1_get_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_get_fe_status(sdla_fe_t *fe, unsigned char *status)
{
	*status = fe->fe_status;
	return 0;
}

/******************************************************************************
 *				sdla_te1_ds_te1_address()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_address(sdla_fe_t *fe, int port_no, int reg)
{
	/* for a102, replace port number of second chip to 1 (1->0) */
	if (fe->fe_chip_id == DEVICE_ID_DS26521){
		port_no = 0;
	}
	return (int)((reg) + ((port_no)*(DLS_PORT_DELTA(reg))));
}

/*
 ******************************************************************************
 *				sdla_ds_te1_TxChanCtrl()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_TxChanCtrl(sdla_fe_t* fe, int channel, int enable)	
{
	int		off = channel / 8;
	int		bit = channel % 8;
	unsigned char	value;

	value = READ_REG(REG_TGCCS1 + off);
	if (enable){
		value &= ~(1 << (bit-1));
	}else{
		value |= (1 << (bit-1));
	}
	WRITE_REG(REG_TGCCS1 + off, value);
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_RxChanCtrl()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_RxChanCtrl(sdla_fe_t* fe, int channel, int enable)
{
	int		off = channel / 8;
	int		bit = channel % 8;
	unsigned char	value;

	value = READ_REG(REG_RGCCS1 + off);
	if (enable){
		value &= ~(1 << (bit-1));
	}else{
		value |= (1 << (bit-1));
	}
	WRITE_REG(REG_RGCCS1 + off, value);
	return 0;
}


int sdla_ds_te1_iface_init(void *p_fe_iface)
{
	sdla_fe_iface_t	*fe_iface = (sdla_fe_iface_t*)p_fe_iface;

	fe_iface->reset			= &sdla_ds_te1_reset;
	fe_iface->global_config		= &sdla_ds_te1_global_config;
	fe_iface->global_unconfig	= &sdla_ds_te1_global_unconfig;
	fe_iface->chip_config		= &sdla_ds_te1_chip_config;
	fe_iface->config		= &sdla_ds_te1_config;
	fe_iface->post_init		= &sdla_ds_te1_post_init;
	fe_iface->reconfig		= &sdla_ds_te1_reconfig;
	fe_iface->unconfig		= &sdla_ds_te1_unconfig;
	fe_iface->pre_release		= &sdla_ds_te1_pre_release;
	fe_iface->disable_irq		= &sdla_ds_te1_disable_irq;
	fe_iface->isr			= &sdla_ds_te1_intr;
	fe_iface->check_isr		= &sdla_ds_te1_check_intr;
	fe_iface->intr_ctrl		= &sdla_ds_te1_intr_ctrl;
	fe_iface->polling		= &sdla_ds_te1_polling;
	fe_iface->process_udp		= &sdla_ds_te1_udp;

	fe_iface->print_fe_alarm	= &sdla_ds_te1_print_alarms;
	/*fe_iface->print_fe_act_channels	= &sdla_te_print_channels;*/
	fe_iface->read_alarm		= &sdla_ds_te1_read_alarms;
	/*fe_iface->set_fe_alarm		= &sdla_te_set_alarms;*/
	fe_iface->read_pmon		= &sdla_ds_te1_pmon;
	fe_iface->flush_pmon		= &sdla_ds_te1_flush_pmon;
	fe_iface->get_fe_status		= &sdla_ds_te1_get_fe_status;
	fe_iface->get_fe_media		= &sdla_ds_te1_get_fe_media;
	fe_iface->get_fe_media_string	= &sdla_ds_te1_get_fe_media_string;
	fe_iface->update_alarm_info	= &sdla_ds_te1_update_alarm_info;
	fe_iface->update_pmon_info	= &sdla_ds_te1_update_pmon_info;
	fe_iface->set_fe_lbmode		= &sdla_ds_te1_set_lbmode;
	fe_iface->read_rbsbits		= &sdla_ds_te1_read_rbsbits;
	fe_iface->check_rbsbits		= &sdla_ds_te1_check_rbsbits;
	fe_iface->report_rbsbits	= &sdla_ds_te1_rbs_report;
	fe_iface->set_rbsbits		= &sdla_ds_te1_set_rbsbits;
	fe_iface->set_fe_sigctrl	= &sdla_ds_te1_sigctrl;

#if 0
	fe_iface->led_ctrl		= &sdla_te_led_ctrl;
#endif
	return 0;
}

/******************************************************************************
*				sdla_ds_te1_device_id()	
*
* Description: Verify device id
* Arguments:	
* Returns:	0 - device is supported, otherwise - device is not supported
******************************************************************************/
static int sdla_ds_te1_device_id(sdla_fe_t* fe)
{
	u_int8_t	value;
	
	/* Revision/Chip ID (Reg. 0x0D) */
	value = READ_REG_LINE(0, REG_IDR);
	fe->fe_chip_id = DEVICE_ID_DS(value);
	switch(fe->fe_chip_id){
	case DEVICE_ID_DS26528:
		fe->fe_max_ports = 8;
		break;
	case DEVICE_ID_DS26524:
		fe->fe_max_ports = 4;
		break;
	case DEVICE_ID_DS26521:
		fe->fe_max_ports = 1;
		break;
	case DEVICE_ID_DS26522:
		fe->fe_max_ports = 2;
		break;
	default:
		DEBUG_EVENT("%s: ERROR: Unsupported DS %s CHIP (%02X:%02X)\n",
				fe->name, 
				FE_MEDIA_DECODE(fe),
				fe->fe_chip_id,
				value);
		return -EINVAL;
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_reset()	
 *
 * Description: Global configuration for Sangoma TE1 DS board.
 * 		Note: 	These register should be program only once for AFT-QUAD
 * 			cards.
 * Arguments:	fe	- front-end structure
 *		port_no	- 0 - global set/clear reset, 1-8 - set/clear reset per port
 *		reset	- 0 - clear reset, 1 - set reset
 * Returns:	WANTRUE - TE1 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
static int sdla_ds_te1_reset(void* pfe, int port_no, int reset)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	u_int8_t	mask = 0x00, value, liu_status, fr_be_status;

	if (sdla_ds_te1_device_id(fe)) return -EINVAL;
	
	if (port_no){
		DEBUG_EVENT("%s: %s Front End Reset for port %d\n", 
					fe->name,
					(reset) ? "Set" : "Clear",
					port_no);
	}else{
		DEBUG_EVENT("%s: %s Global Front End Reset\n", 
				fe->name, (reset) ? "Set" : "Clear");
	}
	if (port_no){
		mask = (0x01 << WAN_DS_REGBITMAP(fe));
	}else{
		int	i = 0;
		
		mask = 0x00;
		for(i=0;i<fe->fe_max_ports;i++){
			mask |= (1<<i);
		}
	}
	/* Set reset first */
	liu_status = READ_REG(REG_GLSRR);
	liu_status |= mask;
	WRITE_REG(REG_GLSRR, liu_status);
	fr_be_status = READ_REG(REG_GFSRR);
	fr_be_status |= mask;
	WRITE_REG(REG_GFSRR, fr_be_status);
	if (fe->fe_chip_id == DEVICE_ID_DS26521 && !port_no){
		value = READ_REG_LINE(1, REG_GLSRR);
		value |= 0x01;
		WRITE_REG_LINE(1, REG_GLSRR, value);
		value = READ_REG_LINE(1, REG_GFSRR);
		value |= 0x01;
		WRITE_REG_LINE(1, REG_GFSRR, value);	
	}
	
	if (!reset){
		WP_DELAY(1000);
	
		/* Clear reset */
		liu_status &= ~mask;
		WRITE_REG(REG_GLSRR, liu_status);
		fr_be_status &= ~mask;
		WRITE_REG(REG_GFSRR, fr_be_status);	
		if (fe->fe_chip_id == DEVICE_ID_DS26521 && !port_no){
			value = READ_REG_LINE(1, REG_GLSRR);
			value &= ~0x01;
			WRITE_REG_LINE(1, REG_GLSRR, value);
			value = READ_REG_LINE(1, REG_GFSRR);
			value &= ~0x01;
			WRITE_REG_LINE(1, REG_GFSRR, value);	
		}
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_e1_set_sig_mode()	
 *
 * Description: Set E1 signalling mode for A101/A102/A104/A108 DallasMaxim board.
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_e1_set_sig_mode(sdla_fe_t *fe, int verbose)
{
	unsigned char	value = 0x00;

	if (WAN_TE1_SIG_MODE(fe) == WAN_TE1_SIG_CAS){
			
		/* CAS signalling mode */
		if (verbose){
			DEBUG_EVENT("%s: Enabling CAS Signalling mode!\n",
							fe->name);
		}
		value = READ_REG(REG_RCR1);
		WRITE_REG(REG_RCR1, value & ~BIT_RCR1_E1_RSIGM);
		//value = READ_REG(REG_RSIGC);
		//WRITE_REG(REG_RSIGC, value | BIT_RSIGC_CASMS);			
		value = READ_REG(REG_TCR1);
		WRITE_REG(REG_TCR1, value | BIT_TCR1_E1_T16S);
	}else{
		
		/* CCS signalling mode */
		if (verbose){
			DEBUG_TEST("%s: Enabling CCS Signalling mode!\n",
							fe->name);
		}
		WAN_TE1_SIG_MODE(fe) = WAN_TE1_SIG_CCS;
		value = READ_REG(REG_RCR1);
		WRITE_REG(REG_RCR1, value | BIT_RCR1_E1_RSIGM);
	}	
	return 0;
}
	
/*
 ******************************************************************************
 *				sdla_8te_global_config()	
 *
 * Description: Global configuration for Sangoma TE1 DS board.
 * 		Note: 	These register should be program only once for AFT-OCTAL
 * 			cards.
 * Arguments:	
 * Returns:	WANTRUE - TE1 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
static int sdla_ds_te1_global_config(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;

	if (sdla_ds_te1_device_id(fe)) return -EINVAL;
	
	DEBUG_EVENT("%s: Global %s Front End configuration\n", 
				fe->name, FE_MEDIA_DECODE(fe));
	
	WRITE_REG_LINE(0, REG_GTCCR, 0x00);
	WRITE_REG_LINE(0, REG_GTCR1, 0x00);  

	WRITE_REG_LINE(0, REG_GLSRR, 0xFF);  
	WRITE_REG_LINE(0, REG_GFSRR, 0xFF);  
	WP_DELAY(1000);
	WRITE_REG_LINE(0, REG_GLSRR, 0x00);  
	WRITE_REG_LINE(0, REG_GFSRR, 0x00);  

	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_global_unconfig()	
 *
 * Description: Global configuration for Sangoma TE1 DS board.
 * 		Note: 	These register should be program only once for AFT-QUAD
 * 			cards.
 * Arguments:	
 * Returns:	WANTRUE - TE1 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
static int sdla_ds_te1_global_unconfig(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;

	DEBUG_EVENT("%s: Global %s Front End unconfigation!\n",
				fe->name, FE_MEDIA_DECODE(fe));
	
	WRITE_REG_LINE(0, REG_GFIMR, 0x00);
	WRITE_REG_LINE(0, REG_GLIMR, 0x00);  
	WRITE_REG_LINE(0, REG_GBIMR, 0x00);  
	
	WRITE_REG_LINE(0, REG_GLSRR, 0xFF);  
	WRITE_REG_LINE(0, REG_GFSRR, 0xFF);  
		
	WP_DELAY(1000);
	return 0;
}

/******************************************************************************
**				sdla_ds_te1_chip_config()	
**
** Description: Configure Dallas Front-End chip
** Arguments:	
** Returns:	0 - successfully, otherwise -EINVAL.
*******************************************************************************/
static int sdla_ds_te1_chip_config(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	unsigned char	value = 0x00;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	/* Init Rx Framer registers */
	CLEAR_REG(0x0000, 0x00F0);
	/* Init Tx Framer registers */
	CLEAR_REG(0x0100, 0x01F0);
	/* Init LIU registers */
	CLEAR_REG(0x1000, 0x1020);
	/* Init BERT registers */
	CLEAR_REG(0x1100, 0x1110);

	/* Set Rx Framer soft reset */
	WRITE_REG(REG_RMMR, BIT_RMMR_SFTRST);
	/* Set Tx Framer soft reset */
	WRITE_REG(REG_TMMR, BIT_RMMR_SFTRST);
	if (IS_T1_FEMEDIA(fe)){
		/* Clear Rx Framer soft reset */
		WRITE_REG(REG_RMMR, 0x00);
		/* Clear Tx Framer soft reset */
		WRITE_REG(REG_TMMR, 0x00);
		/* Enable Rx Framer */
		WRITE_REG(REG_RMMR, BIT_RMMR_FRM_EN);
		/* Enable Tx Framer */
		WRITE_REG(REG_TMMR, BIT_TMMR_FRM_EN);
	}else{
		/* Clear Rx Framer soft reset */
		WRITE_REG(REG_RMMR, BIT_RMMR_T1E1);
		/* Clear Tx Framer soft reset */
		WRITE_REG(REG_TMMR, BIT_TMMR_T1E1);
		/* Enable Rx Framer */
		WRITE_REG(REG_RMMR, (BIT_RMMR_FRM_EN | BIT_RMMR_T1E1));
		/* Enable Tx Framer */
		WRITE_REG(REG_TMMR, (BIT_TMMR_FRM_EN | BIT_TMMR_T1E1));
	}

	if (IS_T1_FEMEDIA(fe)){
		WRITE_REG(REG_RCR1, BIT_RCR1_T1_SYNCT); 
	}
	switch(WAN_FE_FRAME(fe)){
	case WAN_FR_D4:
		value = READ_REG(REG_RCR1);
		WRITE_REG(REG_RCR1, value | BIT_RCR1_T1_RFM | BIT_RCR1_T1_SYNCC);

		value = READ_REG(REG_TCR3);
		WRITE_REG(REG_TCR3, value | BIT_TCR3_TFM);
		break;

	case WAN_FR_ESF:
		value = READ_REG(REG_RCR1);
		value |= BIT_RCR1_T1_SYNCC;
		value &= ~BIT_RCR1_T1_RFM;
		WRITE_REG(REG_RCR1, value);

		value = READ_REG(REG_TCR3);
		value &= ~BIT_TCR3_TFM;
		WRITE_REG(REG_TCR3, value);
		break;
	case WAN_FR_NCRC4:
		break;
	case WAN_FR_CRC4:
		value = READ_REG(REG_RCR1);
		WRITE_REG(REG_RCR1, value | BIT_RCR1_E1_RCRC4);
		value = READ_REG(REG_TCR1);
		WRITE_REG(REG_TCR1, value | BIT_TCR1_E1_TCRC4);
		break;
	default:
		DEBUG_EVENT("%s: Unsupported DS Frame mode (%X)\n",
				fe->name, WAN_FE_FRAME(fe));
		return -EINVAL;
	}
	
	if (IS_E1_FEMEDIA(fe)){
		switch(WAN_FE_FRAME(fe)){
		case WAN_FR_CRC4:
			value = READ_REG(REG_TCR2);
			WRITE_REG(REG_TCR2, value | BIT_TCR2_E1_AEBE);
			break;
		}
	}

	if (IS_E1_FEMEDIA(fe)){
		sdla_ds_e1_set_sig_mode(fe, 0);
	}

	switch(WAN_FE_LCODE(fe)){
	case WAN_LCODE_B8ZS:
		value = READ_REG(REG_RCR1);
		WRITE_REG(REG_RCR1, value | BIT_RCR1_T1_RB8ZS);
		value = READ_REG(REG_TCR1);
		WRITE_REG(REG_TCR1, value | BIT_TCR1_T1_TB8ZS);
		break;

	case WAN_LCODE_HDB3:
		value = READ_REG(REG_RCR1);
		WRITE_REG(REG_RCR1, value | BIT_RCR1_E1_RHDB3);
		value = READ_REG(REG_TCR1);
		WRITE_REG(REG_TCR1, value | BIT_TCR1_E1_THDB3);
		break;

	case WAN_LCODE_AMI:
		if (IS_T1_FEMEDIA(fe)){
			value = READ_REG(REG_RCR1);
			WRITE_REG(REG_RCR1, value & ~BIT_RCR1_T1_RB8ZS);
			value = READ_REG(REG_TCR1);
			WRITE_REG(REG_TCR1, value & ~BIT_TCR1_T1_TB8ZS);
		}else{
			value = READ_REG(REG_RCR1);
			WRITE_REG(REG_RCR1, value & ~BIT_RCR1_E1_RHDB3);
			value = READ_REG(REG_TCR1);
			WRITE_REG(REG_TCR1, value & ~BIT_TCR1_E1_THDB3);
		}
		break;

	default:
		DEBUG_EVENT("%s: Unsupported DS Line code mode (%X)\n",
				fe->name, WAN_FE_LCODE(fe));
		return -EINVAL;
	}

	/* RSYSCLK output */
	WRITE_REG(REG_RIOCR, BIT_RIOCR_RSCLKM);
	/* TSYSCLK input */
	if (IS_T1_FEMEDIA(fe)){
		WRITE_REG(REG_TIOCR, 0x00);
	}else{
		WRITE_REG(REG_TIOCR, BIT_TIOCR_TSCLKM);
	}
#if 0	
	if (WAN_TE1_CLK(fe) == WAN_MASTER_CLK){
		/* RSYNC as input */
		value = READ_REG(REG_RIOCR);
		value |= BIT_RIOCR_RSIO;
		WRITE_REG(REG_RIOCR, value);
		/* RESE enable */
		value = READ_REG(REG_RESCR);
		value |= BIT_RESCR_RESE;
		WRITE_REG(REG_RESCR, value);

		/* TSYNC as output */
		value = READ_REG(REG_TIOCR);
		value |= BIT_TIOCR_TSIO;
		WRITE_REG(REG_TIOCR, value);
	}
#endif
	if (IS_E1_FEMEDIA(fe)){
		WRITE_REG(REG_E1TAF, 0x1B);
		WRITE_REG(REG_E1TNAF, 0x40);
	}

	/* Set INIT_DONE */
	value = READ_REG(REG_RMMR);
	WRITE_REG(REG_RMMR, value | BIT_RMMR_INIT_DONE);
	/* Set INIT_DONE */
	value = READ_REG(REG_TMMR);
	WRITE_REG(REG_TMMR, value | BIT_TMMR_INIT_DONE);

	/* T1/J1 or E1 */
	if (IS_T1_FEMEDIA(fe)){
		WRITE_REG(REG_LTRCR, BIT_LTRCR_T1J1E1S);
	}else{
		/* E1 | G.775 LOS */
		WRITE_REG(REG_LTRCR, 0x00);
	}

	value = 0x00;
	switch(WAN_TE1_LBO(fe)) {
	case WAN_T1_LBO_0_DB:
		value = 0x00;
		break;
	case WAN_T1_LBO_75_DB:
		value = BIT_LTITSR_L2 | BIT_LTITSR_L0;
		break;
	case WAN_T1_LBO_15_DB:
		value = BIT_LTITSR_L2 | BIT_LTITSR_L1;
		break;
	case WAN_T1_LBO_225_DB:
		value = BIT_LTITSR_L2 | BIT_LTITSR_L1 | BIT_LTITSR_L0;
		break;
	case WAN_T1_0_133:
		value = 0x00;
		break;
	case WAN_T1_133_266:
		value = BIT_LTITSR_L0;
		break;
	case WAN_T1_266_399:
		value = BIT_LTITSR_L1;
		break;
	case WAN_T1_399_533:
		value = BIT_LTITSR_L1 | BIT_LTITSR_L0;
		break;
	case WAN_T1_533_655:
		value = BIT_LTITSR_L2;
		break;
	case WAN_E1_120:
		value = BIT_LTITSR_L0;
		break;
	case WAN_E1_75:
		value = 0x00;
		break;
	default:
		if (IS_E1_FEMEDIA(fe)){
			value = BIT_LTITSR_L0;
		}
		break;
	}
	if (IS_T1_FEMEDIA(fe)){
		WRITE_REG(REG_LTITSR, value | BIT_LTITSR_TIMPL0);
	}else if (IS_E1_FEMEDIA(fe)){
		if (WAN_TE1_LBO(fe) == WAN_E1_120){
			value |= (BIT_LTITSR_TIMPL1 | BIT_LTITSR_TIMPL0);
		}
		WRITE_REG(REG_LTITSR, value);
	}else if (IS_J1_FEMEDIA(fe)){
		WRITE_REG(REG_LTITSR,
			value | BIT_LTITSR_TIMPL0);
	}

	value = BIT_LRISMR_RSMS1 | BIT_LRISMR_RSMS0;
	if (WAN_TE1_HI_MODE(fe)){
		value |= BIT_LRISMR_RMONEN;
	}
	if (IS_T1_FEMEDIA(fe)){
		WRITE_REG(REG_LRISMR, value | BIT_LRISMR_RIMPM0);
	}else{
		value |= BIT_LRISMR_RIMPOFF;		
		if (WAN_TE1_LBO(fe) == WAN_E1_120){
			value |= BIT_LRISMR_RIMPM1 | BIT_LRISMR_RIMPM0;
		}
		WRITE_REG(REG_LRISMR, value);
	}
		
	/* Additional front-end settings */
	value = READ_REG(REG_ERCNT);
	if (WAN_FE_LCODE(fe) == WAN_LCODE_AMI){
		value &= ~BIT_ERCNT_LCVCRF;
	}else{
		value |= BIT_ERCNT_LCVCRF;
	}
	value |= BIT_ERCNT_EAMS;		/* manual mode select */
	WRITE_REG(REG_ERCNT, value);

#if 1
	if (WAN_TE1_ACTIVE_CH(fe) != ENABLE_ALL_CHANNELS){
		unsigned long	active_ch = WAN_TE1_ACTIVE_CH(fe);
		int channel_range = (IS_T1_FEMEDIA(fe)) ? 
				NUM_OF_T1_CHANNELS : NUM_OF_E1_TIMESLOTS;
		unsigned char	rescr, tescr, gfcr;
		int i = 0;

		DEBUG_EVENT("%s: %s:%d: Disable channels: ", 
					fe->name,
					FE_MEDIA_DECODE(fe), WAN_FE_LINENO(fe)+1);
 		for(i = 1; i <= channel_range; i++){
			if (!(active_ch & (1 << (i-1)))){
				_DEBUG_EVENT("%d ", i);
				sdla_ds_te1_TxChanCtrl(fe, i, 0);
				sdla_ds_te1_RxChanCtrl(fe, i, 0);
			}
		}
		_DEBUG_EVENT("\n");
		gfcr = READ_REG(REG_GFCR);
		WRITE_REG(REG_GFCR, gfcr | BIT_GFCR_TCBCS | BIT_GFCR_RCBCS);
		rescr = READ_REG(REG_RESCR);
		WRITE_REG(REG_RESCR, rescr | BIT_RESCR_RGCLKEN);
		tescr = READ_REG(REG_TESCR);
		WRITE_REG(REG_TESCR, tescr | BIT_TESCR_TGPCKEN);
	}
#endif

	/* Turn on LIU output */
	WRITE_REG(REG_LMCR, BIT_LMCR_TE);

	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_config()	
 *
 * Description: Configure Sangoma 8 ports T1/E1 board
 * Arguments:	
 * Returns:	WANTRUE - TE1 configred successfully, otherwise WAN_FALSE.
 ******************************************************************************
 */
static int sdla_ds_te1_config(void* pfe)
{

	sdla_fe_t	*fe = (sdla_fe_t*)pfe;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	/* Initial FE state */
	fe->fe_status = FE_UNITIALIZED;	//FE_DISCONNECTED;
	WAN_LIST_INIT(&fe->event);
	wan_spin_lock_init(&fe->lock);

	/* Revision/Chip ID (Reg. 0x0D) */
	if (sdla_ds_te1_device_id(fe)) return -EINVAL;
	switch(fe->fe_chip_id){
	case DEVICE_ID_DS26528:
		if ((int)WAN_FE_LINENO(fe) < 0 || WAN_FE_LINENO(fe) > 8){
			DEBUG_EVENT(
			"%s: TE Config: Invalid Port selected %d (Min=1 Max=8)\n",
					fe->name,
					WAN_FE_LINENO(fe)+1);
			return -EINVAL;
		}
		break;
	case DEVICE_ID_DS26524:
		if ((int)WAN_FE_LINENO(fe) < 0 || WAN_FE_LINENO(fe) > 4){
			DEBUG_EVENT(
			"%s: TE Config: Invalid Port selected %d (Min=1 Max=4)\n",
					fe->name,
					WAN_FE_LINENO(fe)+1);
			return -EINVAL;
		}
		break;
	case DEVICE_ID_DS26521:
	case DEVICE_ID_DS26522:
		if ((int)WAN_FE_LINENO(fe) < 0 || WAN_FE_LINENO(fe) > 1){
			DEBUG_EVENT(
			"%s: TE Config: Invalid Port selected %d (Min=1 Max=2)\n",
					fe->name,
					WAN_FE_LINENO(fe)+1);
			return -EINVAL;
		}
		break;		
	}

	DEBUG_EVENT("%s: Configuring DS %s %s FE\n", 
				fe->name, 
				DECODE_CHIPID(fe->fe_chip_id),
				FE_MEDIA_DECODE(fe));
	DEBUG_EVENT("%s:    Port %d,%s,%s,%s\n", 
				fe->name, 
				WAN_FE_LINENO(fe)+1,
				FE_LCODE_DECODE(fe),
				FE_FRAME_DECODE(fe),
				TE_LBO_DECODE(fe));
	DEBUG_EVENT("%s:    Clk %s:%d, Ch %X\n",
				fe->name, 
				TE_CLK_DECODE(fe),
				WAN_TE1_REFCLK(fe),
				WAN_TE1_ACTIVE_CH(fe));
						
	if (IS_E1_FEMEDIA(fe)){				
		DEBUG_EVENT("%s:    Sig Mode %s\n",
					fe->name, 
					WAN_TE1_SIG_DECODE(fe));
	}

	if (sdla_ds_te1_chip_config(fe)){
		return -EINVAL;
	}
	
	fe->te_param.max_channels = 
		(IS_E1_FEMEDIA(fe)) ? NUM_OF_E1_TIMESLOTS: NUM_OF_T1_CHANNELS;

	sdla_ds_te1_flush_pmon(fe);

	/* LIU alarms are available in A108 */
	fe->fe_stats.liu_alarms	= WAN_TE_BIT_LIU_ALARM;

	wan_set_bit(TE_CONFIGURED,(void*)&fe->te_param.critical);

	/* Enable interrupts */
	sdla_ds_te1_intr_ctrl(fe, 0, WAN_TE_INTR_BASIC, WAN_FE_INTR_ENABLE, 0x00); 
	/* Enable manual update pmon counter */
	sdla_ds_te1_intr_ctrl(fe, 0, WAN_TE_INTR_PMON, WAN_FE_INTR_MASK, 0x00);
	return 0;
}

/*
 ******************************************************************************
 *			sdla_ds_te1_post_init()	
 *
 * Description: T1/E1 post init.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_post_init(void* pfe)
{
	sdla_fe_t		*fe = (sdla_fe_t*)pfe;
	sdla_fe_timer_event_t	event;
	
	/* Initialize and start T1/E1 timer */
	wan_set_bit(TE_TIMER_KILL,(void*)&fe->te_param.critical);
	
	wan_init_timer(
		&fe->timer, 
		sdla_ds_te1_timer,
	       	(wan_timer_arg_t)fe);

	/* Initialize T1/E1 timer */
	wan_clear_bit(TE_TIMER_KILL,(void*)&fe->te_param.critical);

	/* Start T1/E1 timer */
	
	event.type	= TE_LINKDOWN_TIMER;
	event.delay	= POLLING_TE1_TIMER;
	sdla_ds_te1_add_event(fe, &event);
	sdla_ds_te1_add_timer(fe, HZ);
	return 0;
}

/*
 ******************************************************************************
 *			sdla_ds_te1_pre_release()	
 *
 * Description: T1/E1 pre release function (not locked routines)
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_pre_release(void* pfe)
{
	sdla_fe_t		*fe = (sdla_fe_t*)pfe;
	sdla_fe_timer_event_t	*fe_event;
	wan_smp_flag_t		smp_flags;
	
	/* Kill TE timer poll command */
	wan_set_bit(TE_TIMER_KILL,(void*)&fe->te_param.critical);
	if (wan_test_bit(TE_TIMER_RUNNING,(void*)&fe->te_param.critical)){
		wan_del_timer(&fe->timer);
	}
	wan_clear_bit(TE_TIMER_RUNNING,(void*)&fe->te_param.critical);
	wan_spin_lock_irq(&fe->lock,&smp_flags);
	while(!WAN_LIST_EMPTY(&fe->event)){
		fe_event = WAN_LIST_FIRST(&fe->event);
		WAN_LIST_REMOVE(fe_event, next);
		if (fe_event) wan_free(fe_event);
	}
	wan_spin_unlock_irq(&fe->lock,&smp_flags);
	return 0;
}

/*
 ******************************************************************************
 *			sdla_ds_te1_unconfig()	
 *
 * Description: T1/E1 unconfig.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_unconfig(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;

	/* Verify if FE timer is stopped */
	if (!wan_test_bit(TE_TIMER_KILL,(void*)&fe->te_param.critical)){
		DEBUG_EVENT("%s: Front-End timer is not stopped!\n",
					fe->name);
		return -EINVAL;
	}
	
	DEBUG_EVENT("%s: %s Front End unconfigation!\n",
				fe->name, FE_MEDIA_DECODE(fe));	

			
	/* FIXME: Alex to disable interrupts here */
	sdla_ds_te1_disable_irq(fe);
	
	/* Clear configuration flag */
	wan_clear_bit(TE_CONFIGURED,(void*)&fe->te_param.critical);

	//sdla_ds_te1_reset(fe, WAN_FE_LINENO(fe)+1, 1);
	
	//if (fe->fe_chip_id == DEVICE_ID_DS26521/* && fe->fe_cfg.line_no == 1*/){
	//	sdla_ds_te1_global_unconfig(fe);
	//}

	return 0;
}

/*
 ******************************************************************************
 *			sdla_ds_te1_disable_irq()	
 *
 * Description: T1/E1 unconfig.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_disable_irq(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;

	/* FIXME: Alex to disable interrupts here */
	sdla_ds_te1_intr_ctrl(fe, 0, WAN_TE_INTR_GLOBAL, WAN_FE_INTR_MASK, 0x00);
	sdla_ds_te1_intr_ctrl(fe, 0, WAN_TE_INTR_BASIC, WAN_FE_INTR_MASK, 0x00);
	sdla_ds_te1_intr_ctrl(fe, 0, WAN_TE_INTR_PMON, WAN_FE_INTR_MASK, 0x00);

	return 0;
}

/*
 ******************************************************************************
 *			sdla_ds_te1_reconfig()	
 *
 * Description: T1/E1 post configuration.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_reconfig(sdla_fe_t* fe)
{

	if (IS_E1_FEMEDIA(fe)){
		sdla_ds_e1_set_sig_mode(fe, 1);
	}
	return 0;
}

static int 
sdla_ds_te1_sigctrl(sdla_fe_t *fe, int sig_mode, unsigned long ch_map, int mode)
{	
	sdla_fe_timer_event_t	event;
	int			err;
	
	event.type		= (mode == WAN_ENABLE) ? 
					TE_RBS_ENABLE : TE_RBS_DISABLE;
	event.delay		= POLLING_TE1_TIMER;
	event.te_event.ch_map	= ch_map;
	err = sdla_ds_te1_add_event(fe, &event);
	if (err){
		DEBUG_EVENT("%s: Failed to add new fe event %02X ch_map=%08lX!\n",
					fe->name,
					event.type, event.te_event.ch_map);
		return -EINVAL;
	}
	return 0;

}


/******************************************************************************
**			sdla_ds_t1_is_connected()	
**
** Description: Verify T1 status.
** Arguments:
** Returns:	1 - the port is connected
**		0 - the port is disconnected
******************************************************************************/
static int sdla_ds_t1_is_connected(sdla_fe_t *fe, unsigned long alarms)
{

	if (alarms & WAN_TE1_FRAMED_ALARMS) return 0;
	return 1;
}
/******************************************************************************
**			sdla_ds_e1_is_connected()	
**
** Description: Verify E1 status.
** Arguments:
** Returns:	1 - the port is connected
**		0 - the port is disconnected
******************************************************************************/
static int sdla_ds_e1_is_connected(sdla_fe_t *fe, unsigned long alarms)
{
	if (alarms & WAN_TE1_FRAMED_ALARMS) return 0;
	return 1;
}

/*
 ******************************************************************************
 *			sdla_ds_te1_set_status()	
 *
 * Description: Set T1/E1 status. Enable OOF and LCV interrupt (if status 
 * 		changed to disconnected.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static void sdla_ds_te1_set_status(sdla_fe_t* fe, unsigned long alarms)
{
	sdla_t		*card = (sdla_t*)fe->card;
	unsigned char	curr_fe_status = fe->fe_status;

	if (IS_T1_FEMEDIA(fe)){

		if (sdla_ds_t1_is_connected(fe, alarms)){
			sdla_ds_te1_clear_alarms(fe, WAN_TE_BIT_YEL_ALARM);
			if (!(fe->fe_alarm & WAN_TE_BIT_YEL_ALARM)){
				if (fe->fe_status != FE_CONNECTED){
					fe->fe_status = FE_CONNECTED;
				}
			}else{
				if (WAN_NET_RATELIMIT()){
				DEBUG_EVENT("%s: T1 Waiting for Yellow Alarm to clear...\n",
						fe->name);
				}
				fe->fe_status = FE_DISCONNECTED;
			}
		}else{
			if (fe->fe_status != FE_DISCONNECTED){
				fe->fe_status = FE_DISCONNECTED;
			}
		}            

	} else {
		if (sdla_ds_e1_is_connected(fe, alarms)){
			if (fe->fe_status != FE_CONNECTED){
				fe->fe_status = FE_CONNECTED;
			}
		} else {
			if (fe->fe_status != FE_DISCONNECTED){
				fe->fe_status = FE_DISCONNECTED;
			}
		}
	}

	if (curr_fe_status != fe->fe_status){
		if (fe->fe_status == FE_CONNECTED){
			if (fe->te_param.status_cnt > WAN_TE1_STATUS_THRESHOLD){
				DEBUG_EVENT("%s: %s connected!\n", 
						fe->name,
						FE_MEDIA_DECODE(fe));
				if (card->wandev.te_report_alarms){
					card->wandev.te_report_alarms(
							card,
							fe->fe_alarm);
				}
			}else{
				if (!fe->te_param.status_cnt){
					DEBUG_TEST("%s: %s connecting...\n", 
							fe->name,
							FE_MEDIA_DECODE(fe));
				}
				fe->te_param.status_cnt ++;
				fe->fe_status = FE_DISCONNECTED;
				DEBUG_TEST("%s: %s connecting...%d\n", 
							fe->name,
							FE_MEDIA_DECODE(fe),
							fe->te_param.status_cnt);
			}
		}else{
			DEBUG_EVENT("%s: %s disconnected!\n", 
					fe->name,
					FE_MEDIA_DECODE(fe));
			fe->fe_status = FE_DISCONNECTED;
			fe->te_param.status_cnt = 0;
			if (card->wandev.te_report_alarms){
				card->wandev.te_report_alarms(card, fe->fe_alarm);
			}
		}

	}else{
		fe->te_param.status_cnt = 0;	
		DEBUG_TEST("%s: %s %s...%d\n", 
					fe->name,
					FE_MEDIA_DECODE(fe),
					WAN_FE_STATUS_DECODE(fe),
					fe->te_param.status_cnt);
	}

	return;
}


/*
*******************************************************************************
**				sdla_te_alarm_print()	
**
** Description: 
** Arguments:
** Returns:
*/
static int sdla_ds_te1_print_alarms(sdla_fe_t* fe, unsigned int alarms)
{
	if (!alarms){
		alarms = fe->fe_alarm; 
	}

	if (!alarms){
		DEBUG_EVENT("%s: %s Alarms status: No alarms detected!\n",
				fe->name,
				FE_MEDIA_DECODE(fe));
		return 0;
	}
	DEBUG_EVENT("%s: %s Framer Alarms status (%X):\n",
			fe->name,
			FE_MEDIA_DECODE(fe),
			alarms);
	if (alarms & WAN_TE_BIT_RAI_ALARM){
		DEBUG_EVENT("%s:    RAI is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_LOS_ALARM){
		DEBUG_EVENT("%s:    LOS is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_OOF_ALARM){
		DEBUG_EVENT("%s:    OOF is ON\n", fe->name);
	}
	if (alarms & WAN_TE_BIT_RED_ALARM){
		DEBUG_EVENT("%s:    RED is ON\n", fe->name);
	}
	DEBUG_EVENT("%s: %s LIU Alarms status (%X):\n",
			fe->name,
			FE_MEDIA_DECODE(fe),
			fe->liu_alarm);
	if (fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_OC){
		DEBUG_EVENT("%s:    Open Circuit is detected!\n",
				fe->name);
	}
	if (fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_SC){
		DEBUG_EVENT("%s:    Short Circuit is detected!\n",
				fe->name);
	}
	if (fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_LOS){
		DEBUG_EVENT("%s:    Lost of Signal is detected!\n",
				fe->name);
	}

	return 0;
}

/*
*******************************************************************************
**				sdla_te_read_alarms()	
**
** Description: 
** Arguments:
** Returns:
*/
static unsigned int sdla_ds_te1_read_alarms(sdla_fe_t *fe, int action)
{

	if (IS_FE_ALARM_READ(action)){
		unsigned char	rrts1 = READ_REG(REG_RRTS1);
		unsigned char	lrsr = READ_REG(REG_LRSR);

		DEBUG_TE1("%s: Alarm status =  %02X (%X)\n", 
				fe->name, rrts1, fe->fe_alarm);
		/* Framer alarms */
		if (rrts1 & BIT_RRTS1_RRAI){
			if (!(fe->fe_alarm & WAN_TE_BIT_RAI_ALARM)){
				DEBUG_EVENT("%s: RAI alarm is ON\n", 
							fe->name);
			}
			fe->fe_alarm |= WAN_TE_BIT_RAI_ALARM;		
		}else{
			if (fe->fe_alarm & WAN_TE_BIT_RAI_ALARM){
				DEBUG_EVENT("%s: RAI alarm is OFF\n", 
							fe->name);
			}
			fe->fe_alarm &= ~WAN_TE_BIT_RAI_ALARM;		
		}
		if (rrts1 & BIT_RRTS1_RAIS){
			if (!(fe->fe_alarm & WAN_TE_BIT_AIS_ALARM)){
				DEBUG_EVENT("%s: AIS alarm is ON\n", 
							fe->name);
			}
			fe->fe_alarm |= WAN_TE_BIT_AIS_ALARM;		
		}else{
			if (fe->fe_alarm & WAN_TE_BIT_AIS_ALARM){
				DEBUG_EVENT("%s: AIS alarm is OFF\n", 
							fe->name);
			}
			fe->fe_alarm &= ~WAN_TE_BIT_AIS_ALARM;		
		}
		if (rrts1 & BIT_RRTS1_RLOS){
			if (!(fe->fe_alarm & WAN_TE_BIT_LOS_ALARM)){
				DEBUG_EVENT("%s: LOS alarm is ON\n", 
							fe->name);
			}
			fe->fe_alarm |= WAN_TE_BIT_LOS_ALARM;		
		}else{
			if (fe->fe_alarm & WAN_TE_BIT_LOS_ALARM){
				DEBUG_EVENT("%s: LOS alarm is OFF\n", 
							fe->name);
			}
			fe->fe_alarm &= ~WAN_TE_BIT_LOS_ALARM;		
		}
		if (rrts1 & BIT_RRTS1_RLOF){
			if (!(fe->fe_alarm & WAN_TE_BIT_OOF_ALARM)){
				DEBUG_EVENT("%s: OOF alarm is ON\n", 
							fe->name);
			}
			fe->fe_alarm |= WAN_TE_BIT_OOF_ALARM;		
		}else{
			if (fe->fe_alarm & WAN_TE_BIT_OOF_ALARM){
				DEBUG_EVENT("%s: OOF alarm is OFF\n", 
							fe->name);
			}
			fe->fe_alarm &= ~WAN_TE_BIT_OOF_ALARM;		
		}
		/* Aug 30, 2006
		** Red alarm is either LOS or OOF alarms */
		if (IS_TE_OOF_ALARM(fe->fe_alarm) ||
		    IS_TE_LOS_ALARM(fe->fe_alarm)){
			if (!(fe->fe_alarm & WAN_TE_BIT_RED_ALARM)){
				DEBUG_EVENT("%s: RED alarm is ON\n", 
							fe->name);
			}
			fe->fe_alarm |= WAN_TE_BIT_RED_ALARM;
		}else{
			if (fe->fe_alarm & WAN_TE_BIT_RED_ALARM){
				DEBUG_EVENT("%s: RED alarm is OFF\n", 
							fe->name);
			}
			fe->fe_alarm &= ~WAN_TE_BIT_RED_ALARM;
		}

		/* LIU alarms */
		if (lrsr & BIT_LRSR_OCS){
			if (!(fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_OC)){
				DEBUG_EVENT("%s: Open Circuit is detected!\n",
						fe->name);
			}
			fe->liu_alarm |= WAN_TE_BIT_LIU_ALARM_OC;
		}else{
			if (fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_OC){
				DEBUG_EVENT("%s: Open Circuit is cleared!\n",
						fe->name);
			}
			fe->liu_alarm &= ~WAN_TE_BIT_LIU_ALARM_OC;
		}
		if (lrsr & BIT_LRSR_SCS){
			if (!(fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_SC)){
				DEBUG_EVENT("%s: Short Circuit is detected!\n",
						fe->name);
			}
			fe->liu_alarm |= WAN_TE_BIT_LIU_ALARM_SC;
		}else{
			if (fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_SC){
				DEBUG_EVENT("%s: Short Circuit is cleared!\n",
						fe->name);
			}
			fe->liu_alarm &= ~WAN_TE_BIT_LIU_ALARM_SC;	
		}
		if (lrsr & BIT_LRSR_LOSS){
			if (!(fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_LOS)){
				DEBUG_EVENT("%s: Lost of Signal is detected!\n",
						fe->name);
			}
			fe->liu_alarm |= WAN_TE_BIT_LIU_ALARM_LOS;
		}else{
			if (fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_LOS){
				DEBUG_EVENT("%s: Lost of Signal is cleared!\n",
						fe->name);
			}
			fe->liu_alarm &= ~WAN_TE_BIT_LIU_ALARM_LOS;	
		}
	}
	if (IS_FE_ALARM_PRINT(action)){
		sdla_ds_te1_print_alarms(fe, fe->fe_alarm);
	}
	return fe->fe_alarm;
}

/*
*******************************************************************************
**				sdla_ds_te1_set_alarms()	
**
** Description: 
** Arguments:
** Returns:
*/
static int sdla_ds_te1_set_alarms(sdla_fe_t* fe, unsigned long alarms)
{
	if (alarms & WAN_TE_BIT_YEL_ALARM){
		DEBUG_TEST("%s: Setting YELLOW alarm (not supported)!\n",
							fe->name);
	}
	return 0;
}

/*
*******************************************************************************
**				sdla_ds_te1_clear_alarms()	
**
** Description: 
** Arguments:
** Returns:
*/
static int sdla_ds_te1_clear_alarms(sdla_fe_t* fe, unsigned long alarms)
{
	if (alarms & WAN_TE_BIT_YEL_ALARM){
		DEBUG_TEST("%s: Clearing YELLOW alarm (not supported)!\n",
						fe->name);
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_rbs_report()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_rbs_report(sdla_fe_t* fe)
{
	sdla_t*		card = (sdla_t*)fe->card;
	int		ch = 1, max_channels;
	
	max_channels = fe->te_param.max_channels;
	ch = (IS_E1_FEMEDIA(fe)) ? 0 : 1;
	for(; ch <= max_channels; ch++) {
		if (wan_test_bit(ch, &fe->te_param.rx_rbs_status)){
			if (card->wandev.te_report_rbsbits){
				card->wandev.te_report_rbsbits(
					card, 
					ch, 
					fe->te_param.rx_rbs[ch]);
			}
			wan_clear_bit(ch, &fe->te_param.rx_rbs_status);
		}
	}
	return 0;
}


/*
 ******************************************************************************
 *				sdla_ds_te1_read_rbsbits()	
 *
 * Description:
 * Arguments:	channo: 1-24 for T1
 *			1-32 for E1
 * Returns:
 ******************************************************************************
 */
static unsigned char sdla_ds_te1_read_rbsbits(sdla_fe_t* fe, int channo, int mode)
{
	sdla_t*		card = (sdla_t*)fe->card;
	int		rs_offset = 0, range = 0;
	unsigned char	rbsbits = 0x00, status = 0x00;
	
	if (IS_E1_FEMEDIA(fe)){
		rs_offset = channo % 16;
		range = 16;
	}else{
		rs_offset = (channo - 1) % 12;
		range = 12;
	}
	rbsbits = READ_REG(REG_RS1 + rs_offset);
	if (channo <= range){
		rbsbits = (rbsbits >> 4) & 0x0F;
	}else{
		rbsbits &= 0xF;
	}
	
	if (rbsbits & BIT_RS_A) status |= WAN_RBS_SIG_A;
	if (rbsbits & BIT_RS_B) status |= WAN_RBS_SIG_B;
	if (rbsbits & BIT_RS_C) status |= WAN_RBS_SIG_C;
	if (rbsbits & BIT_RS_D) status |= WAN_RBS_SIG_D;

	if (mode & WAN_TE_RBS_UPDATE){
		sdla_ds_te1_rbs_update(fe, channo, status);
	}

	if ((mode & WAN_TE_RBS_REPORT) && card->wandev.te_report_rbsbits){
		card->wandev.te_report_rbsbits(
					card, 
					channo, 
					status);
	}
	return status;
}

/*
 ******************************************************************************
 *				sdla_te_rbs_update()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_ds_te1_rbs_update(sdla_fe_t* fe, int channo, unsigned char status)
{

	if (fe->fe_debug & WAN_FE_DEBUG_RBS_RX_ENABLE && 
	    fe->te_param.rx_rbs[channo] != status){
		DEBUG_EVENT(
		"%s: %s:%-3d RX RBS A:%1d B:%1d C:%1d D:%1d\n",
					fe->name,
					FE_MEDIA_DECODE(fe),
					channo, 
					(status & WAN_RBS_SIG_A) ? 1 : 0,
					(status & WAN_RBS_SIG_B) ? 1 : 0,
					(status & WAN_RBS_SIG_C) ? 1 : 0,
					(status & WAN_RBS_SIG_D) ? 1 : 0);
	}
	
	/* Update rbs value in private structures */
	wan_set_bit(channo, &fe->te_param.rx_rbs_status);
	fe->te_param.rx_rbs[channo] = status;

	if (status & WAN_RBS_SIG_A){
		wan_set_bit(channo,
			(unsigned long*)&fe->te_param.rx_rbs_A);
	}else{
		wan_clear_bit(channo,
			(unsigned long*)&fe->te_param.rx_rbs_A);
	}	
	if (status & WAN_RBS_SIG_B){
		wan_set_bit(channo,
			(unsigned long*)&fe->te_param.rx_rbs_B);
	}else{
		wan_clear_bit(channo,
			(unsigned long*)&fe->te_param.rx_rbs_B);
	}
	if (status & WAN_RBS_SIG_C){
		wan_set_bit(channo,
			(unsigned long*)&fe->te_param.rx_rbs_C);
	}else{
		wan_clear_bit(channo,
			(unsigned long*)&fe->te_param.rx_rbs_C);
	}
	if (status & WAN_RBS_SIG_D){
		wan_set_bit(channo,
			(unsigned long*)&fe->te_param.rx_rbs_D);
	}else{
		wan_clear_bit(channo,
			(unsigned long*)&fe->te_param.rx_rbs_D);
	}

	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_check_rbsbits()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_ds_te1_check_rbsbits(sdla_fe_t* fe, int ch_base, unsigned int ts_map, int report)
{
	sdla_t*		card = (sdla_t*)fe->card;
	unsigned char	rs_status = 0x0, rbsbits = 0x00, bit_mask = 0x00;
	int		rs_reg = 0, rs_offset = 0;
	int		i = 0, channel, range = 12;

	switch(ch_base){
	case 1:
		rs_reg = REG_RSS1;
		break;
	case 9:
		rs_reg = REG_RSS2;
		break;
	case 17:
		rs_reg = REG_RSS3;
		break;
	case 25:
		rs_reg = REG_RSS4;
		break;
	}
	rs_status = READ_REG(rs_reg);
	
	if (rs_status == 0x00){
		return 0;
	}

	if (IS_E1_FEMEDIA(fe)){
		range = 16;
	}
	for(i = 0; i < 8; i ++) {
		channel = ch_base + i;
		if (IS_E1_FEMEDIA(fe)){
			if (channel == 1 || channel == 17){
				continue;
			}
			rs_offset = (channel - 1) % 16;
		}else{
			rs_offset = (channel - 1) % 12;
		}
		/* If this channel/timeslot is not selected, move to 
		 * another channel/timeslot */
		if (!wan_test_bit(channel-1, &ts_map)){
			continue;
		}
		bit_mask = (1 << i);
		if(rs_status & bit_mask) {
			unsigned char	abcd_status = 0x00;

			rbsbits = READ_REG(REG_RS1 + rs_offset);
			DEBUG_TE1("%s: Channel %d: RS=%X+%d Val=%02X\n",
					fe->name, channel, REG_RS1, rs_offset, rbsbits);
			if (channel > range){
				rbsbits &= 0x0F;
			}else{
				rbsbits = (rbsbits >> 4) & 0x0F;
			}
		
			if (rbsbits & BIT_RS_A) abcd_status |= WAN_RBS_SIG_A;
			if (rbsbits & BIT_RS_B) abcd_status |= WAN_RBS_SIG_B;
			if (rbsbits & BIT_RS_C) abcd_status |= WAN_RBS_SIG_C;
			if (rbsbits & BIT_RS_D) abcd_status |= WAN_RBS_SIG_D;
			if (IS_E1_FEMEDIA(fe)){
				channel--;
			}

			sdla_ds_te1_rbs_update(fe, channel, abcd_status);
			if (report && card->wandev.te_report_rbsbits){
				card->wandev.te_report_rbsbits(
							card, 
							channel, 
							abcd_status);
			}
			WRITE_REG(rs_reg, bit_mask);
		}
	}	
	return 0; 
}

/*
 ******************************************************************************
 *				sdla_ds_te1_set_RBS()	
 *
 * Description:
 * Arguments:	T1: 1-24 E1: 0-31
 * Returns:
 ******************************************************************************
 */
static int 
sdla_ds_te1_set_rbsbits(sdla_fe_t *fe, int channel, unsigned char status)
{
	int		ts_off = 0, range = 0;
	unsigned char	rbsbits = 0x00, ts_org;

	if ((unsigned int)channel > fe->te_param.max_channels){
		DEBUG_EVENT("%s: Invalid channel number %d (%d)\n",
				fe->name, channel, fe->te_param.max_channels);
		return -EINVAL;
	}
#if 0
	if (IS_E1_FEMEDIA(fe) && (channel == 0 || channel == 16)){
		DEBUG_EVENT("%s: Invalid channel number %d for E1 Rx SIG\n",
				fe->name, channel);
		return 0;
	}
#endif
	if (status & WAN_RBS_SIG_A) rbsbits |= BIT_TS_A;
	if (status & WAN_RBS_SIG_B) rbsbits |= BIT_TS_B;
	if (!(IS_T1_FEMEDIA(fe) && WAN_FE_FRAME(fe) == WAN_FR_D4)){
		if (status & WAN_RBS_SIG_C) rbsbits |= BIT_TS_C;
		if (status & WAN_RBS_SIG_D) rbsbits |= BIT_TS_D;
	}
	if (fe->fe_debug & WAN_FE_DEBUG_RBS_TX_ENABLE){
		DEBUG_EVENT("%s: %s:%-3d TX RBS A:%1d B:%1d C:%1d D:%1d\n",
				fe->name,
				FE_MEDIA_DECODE(fe),
				channel,	
				(rbsbits & BIT_TS_A) ? 1 : 0,
				(rbsbits & BIT_TS_B) ? 1 : 0,
				(rbsbits & BIT_TS_C) ? 1 : 0,
				(rbsbits & BIT_TS_D) ? 1 : 0);
	}
	if (rbsbits & BIT_TS_A){
		wan_set_bit(channel,(unsigned long*)&fe->te_param.tx_rbs_A);
	}else{
		wan_clear_bit(channel,(unsigned long*)&fe->te_param.tx_rbs_A);
	}
	if (rbsbits & BIT_TS_B){
		wan_set_bit(channel,(unsigned long*)&fe->te_param.tx_rbs_B);
	}else{
		wan_clear_bit(channel,(unsigned long*)&fe->te_param.tx_rbs_B);
	}
	if (rbsbits & BIT_TS_C){
		wan_set_bit(channel,(unsigned long*)&fe->te_param.tx_rbs_C);
	}else{
		wan_clear_bit(channel,(unsigned long*)&fe->te_param.tx_rbs_C);
	}
	if (rbsbits & BIT_TS_D){
		wan_set_bit(channel,(unsigned long*)&fe->te_param.tx_rbs_D);
	}else{
		wan_clear_bit(channel,(unsigned long*)&fe->te_param.tx_rbs_D);
	}
	if (IS_E1_FEMEDIA(fe)){
		ts_off	= channel % 16;
		range	= 15;
	}else{
		ts_off	= (channel - 1) % 12;
		range	= 12;
	}
	ts_org = READ_REG(REG_TS1 + ts_off);
	if (channel <= range){
		rbsbits = rbsbits << 4;
		ts_org &= 0xF;
	}else{
		ts_org &= 0xF0;
	}

	DEBUG_TE1("%s: TS=%02X Val=%02X\n",
				fe->name, 
				REG_TS1+ts_off,
				rbsbits);

	WRITE_REG(REG_TS1 + ts_off, ts_org | rbsbits);
	return 0;
}
/*
 ******************************************************************************
 *				sdla_te_rbs_print_banner()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_ds_te1_rbs_print_banner(sdla_fe_t* fe)
{
	if (IS_T1_FEMEDIA(fe)){
		DEBUG_EVENT("%s:                111111111122222\n",
					fe->name);
		DEBUG_EVENT("%s:       123456789012345678901234\n",
					fe->name);
		DEBUG_EVENT("%s:       ------------------------\n",
					fe->name);
	}else{
		DEBUG_EVENT("%s:                11111111112222222222333\n",
					fe->name);
		DEBUG_EVENT("%s:       12345678901234567890123456789012\n",
					fe->name);
		DEBUG_EVENT("%s:       --------------------------------\n",
					fe->name);
	}	
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_rbs_print_bits()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_ds_te1_rbs_print_bits(sdla_fe_t* fe, unsigned long bits, char *msg)
{
	int 	i, max_channels = fe->te_param.max_channels;
	int	start_chan = 1;

	if (IS_E1_FEMEDIA(fe)){
		start_chan = 0;
	}
	_DEBUG_EVENT("%s: %s ", fe->name, msg);
	for(i=start_chan; i <= max_channels; i++)
		_DEBUG_EVENT("%01d", 
			wan_test_bit(i, &bits) ? 1 : 0);
	_DEBUG_EVENT("\n");
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_rbs_print()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_ds_te1_rbs_print(sdla_fe_t* fe, int last_status)
{
	unsigned long	rx_a = 0x00;
	unsigned long	rx_b = 0x00;
	unsigned long	rx_c = 0x00;
	unsigned long	rx_d = 0x00;
	
	if (last_status){
		rx_a = fe->te_param.rx_rbs_A;	
		rx_b = fe->te_param.rx_rbs_B;
		rx_c = fe->te_param.rx_rbs_C;
		rx_d = fe->te_param.rx_rbs_D;
		
		DEBUG_EVENT("%s: Last Status:\n",
					fe->name);
		sdla_ds_te1_rbs_print_banner(fe);
		sdla_ds_te1_rbs_print_bits(fe, fe->te_param.tx_rbs_A, "TX A:");
		sdla_ds_te1_rbs_print_bits(fe, fe->te_param.tx_rbs_B, "TX B:");
		sdla_ds_te1_rbs_print_bits(fe, fe->te_param.tx_rbs_C, "TX C:");
		sdla_ds_te1_rbs_print_bits(fe, fe->te_param.tx_rbs_D, "TX D:");
		DEBUG_EVENT("%s:\n", fe->name);
	}else{
		unsigned int	i, chan = 0;
		unsigned char	abcd = 0x00;
		for(i = 1; i <= fe->te_param.max_channels; i++) {

			abcd = sdla_ds_te1_read_rbsbits(fe, i, WAN_TE_RBS_NONE);

			chan = (IS_E1_FEMEDIA(fe))? i - 1 : i;
			if (abcd & WAN_RBS_SIG_A){
				wan_set_bit(chan, &rx_a);
			}
			if (abcd & WAN_RBS_SIG_B){
				wan_set_bit(chan, &rx_b);
			}
			if (abcd & WAN_RBS_SIG_C){
				wan_set_bit(chan, &rx_c);
			}
			if (abcd & WAN_RBS_SIG_D){
				wan_set_bit(chan, &rx_d);
			}
		}
		DEBUG_EVENT("%s: Current Status:\n",
					fe->name);
		sdla_ds_te1_rbs_print_banner(fe);
	}

	sdla_ds_te1_rbs_print_bits(fe, rx_a, "RX A:");
	sdla_ds_te1_rbs_print_bits(fe, rx_b, "RX B:");
	sdla_ds_te1_rbs_print_bits(fe, rx_c, "RX C:");
	sdla_ds_te1_rbs_print_bits(fe, rx_d, "RX D:");
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_intr_ctrl()	
 *
 * Description: Check interrupt type.
 * Arguments: 	card 		- pointer to device structure.
 * 		write_register 	- write register function.
 * 		read_register	- read register function.
 * Returns:	None.
 ******************************************************************************
 */
static int
sdla_ds_te1_intr_ctrl(sdla_fe_t *fe, int dummy, int type, int mode, unsigned int ts_map) 
{
	unsigned char	mask, value;
	unsigned char	rscse;
	unsigned int	ch, bit, off;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	if (!wan_test_bit(TE_CONFIGURED,(void*)&fe->te_param.critical)){
		return 0;
	}
	switch(type){
	case WAN_TE_INTR_GLOBAL:
		mask = READ_REG(REG_GFIMR);
		if (mode == WAN_FE_INTR_ENABLE){
			//mask |= (1<<WAN_FE_LINENO(fe));
			mask |= (1<<WAN_DS_REGBITMAP(fe));
		}else{
			//mask &= ~(1<<WAN_FE_LINENO(fe));
			mask &= ~(1<<WAN_DS_REGBITMAP(fe));
		}
		WRITE_REG(REG_GFIMR, mask);

		mask = READ_REG(REG_GLIMR);
		if (mode == WAN_FE_INTR_ENABLE){
			mask |= (1<<WAN_DS_REGBITMAP(fe));
			//mask |= (1<<WAN_FE_LINENO(fe));
		}else{
			mask &= ~(1<<WAN_DS_REGBITMAP(fe));
			//mask |= (1<<WAN_FE_LINENO(fe));
		}
		WRITE_REG(REG_GLIMR, mask);
		
		mask = READ_REG(REG_GBIMR);
		if (mode == WAN_FE_INTR_ENABLE){
			//mask |= (1<<WAN_FE_LINENO(fe));
			mask |= (1<<WAN_DS_REGBITMAP(fe));
		}else{
			//mask &= ~(1<<WAN_FE_LINENO(fe));
			mask &= ~(1<<WAN_DS_REGBITMAP(fe));
		}
		WRITE_REG(REG_GBIMR, mask);
		break;

	case WAN_TE_INTR_BASIC:
		if (mode == WAN_FE_INTR_ENABLE){
			WRITE_REG(REG_RIM1,
				BIT_RIM1_RRAIC | BIT_RIM1_RRAID |
				BIT_RIM1_RAISC | BIT_RIM1_RAISD |
				BIT_RIM1_RLOSC | BIT_RIM1_RLOSD |
				BIT_RIM1_RLOFC | BIT_RIM1_RLOFD);
			/*WRITE_REG(REG_RIM4, BIT_RIM4_TIMER);*/
			WRITE_REG(REG_LSIMR,
				BIT_LSIMR_OCCIM | BIT_LSIMR_OCDIM |
				BIT_LSIMR_SCCIM | BIT_LSIMR_SCCIM |
				BIT_LSIMR_LOSCIM | BIT_LSIMR_LOSDIM |
				BIT_LSIMR_JALTCIM | BIT_LSIMR_JALTSIM);
		}else{
			WRITE_REG(REG_RIM1, 0x00);
			WRITE_REG(REG_RIM2, 0x00);
			WRITE_REG(REG_RIM3, 0x00);
			WRITE_REG(REG_RIM4, 0x00);
			WRITE_REG(REG_RIM5, 0x00);
			WRITE_REG(REG_RIM7, 0x00);

			WRITE_REG(REG_TIM1, 0x00);
			WRITE_REG(REG_TIM2, 0x00);
			WRITE_REG(REG_TIM3, 0x00);

			WRITE_REG(REG_LSIMR, 0x00);
		}
		break;

	case WAN_TE_INTR_SIGNALLING:
		for(ch = 1; ch <= fe->te_param.max_channels; ch++){
			if (!wan_test_bit(ch, &ts_map)){
				continue;
			}
			if (IS_T1_FEMEDIA(fe)){
				bit = (ch-1) % 8;
				off = (ch-1) / 8;
			}else{
				if (ch == 16) continue;
				bit = ch % 8;
				off = ch / 8;
			}
			rscse = READ_REG(REG_RSCSE1+off);
			if (mode == WAN_FE_INTR_ENABLE){
				rscse |= (1<<bit);
			}else{
				rscse &= ~(1<<bit);
			}
			DEBUG_TEST("%s: Ch %d RSCSE=%02X Val=%02X\n",
					fe->name, ch, REG_RSCSE1+off, rscse);
			WRITE_REG(REG_RSCSE1+off, rscse);
		}
		value = READ_REG(REG_RIM4);
		if (mode == WAN_FE_INTR_ENABLE){
			value |= BIT_RIM4_RSCOS;
		}else{
			value &= ~BIT_RIM4_RSCOS;
		}
		WRITE_REG(REG_RIM4, value);
		break;

	case WAN_TE_INTR_PMON:
		value = READ_REG(REG_ERCNT);
		if (mode == WAN_FE_INTR_ENABLE){
			value &= ~BIT_ERCNT_EAMS;
		}else{
			value |= BIT_ERCNT_EAMS;
		}
		WRITE_REG(REG_ERCNT, value); 
		break;

	default:
		DEBUG_EVENT("%s: Unsupported interrupt type %X!\n",
					fe->name, type);
		return -EINVAL;
	}

	return 0;
}


static int sdla_ds_te1_framer_rx_intr(sdla_fe_t *fe) 
{
	unsigned char	istatus;

	istatus = READ_REG(REG_RIIR);
	if (istatus & BIT_RIIR_RLS1){
		unsigned char	rls1 = READ_REG(REG_RLS1);
		unsigned char	rrts1 = READ_REG(REG_RRTS1);

		DEBUG_TE1("%s: RX Latched Status Register 1 %02X\n",
					fe->name, rls1);
		if (rls1 & (BIT_RLS1_RRAIC|BIT_RLS1_RRAID)){
			if (rrts1 & BIT_RRTS1_RRAI){
				fe->fe_alarm |= WAN_TE_BIT_RAI_ALARM;		
				DEBUG_EVENT("%s: RAI alarm is ON\n",
							fe->name);
			}else{
				fe->fe_alarm &= ~WAN_TE_BIT_RAI_ALARM;		
				DEBUG_EVENT("%s: RAI alarm is OFF\n",
							fe->name);
			}
		}
		if (rls1 & (BIT_RLS1_RAISC|BIT_RLS1_RAISD)){
			if (rrts1 & BIT_RRTS1_RAIS){
				fe->fe_alarm |= WAN_TE_BIT_AIS_ALARM;		
				DEBUG_EVENT("%s: AIS alarm is ON\n",
							fe->name);
			}else{
				fe->fe_alarm &= ~WAN_TE_BIT_AIS_ALARM;		
				DEBUG_EVENT("%s: AIS alarm is OFF\n",
							fe->name);
			}
		}
		if (rls1 & (BIT_RLS1_RLOSC|BIT_RLS1_RLOSD)){
			if (rrts1 & BIT_RRTS1_RLOS){
				fe->fe_alarm |= WAN_TE_BIT_LOS_ALARM;		
				DEBUG_EVENT("%s: LOS alarm is ON\n",
							fe->name);
			}else{
				fe->fe_alarm &= ~WAN_TE_BIT_LOS_ALARM;		
				DEBUG_EVENT("%s: LOS alarm is OFF\n",
							fe->name);
			}
		}
		if (rls1 & (BIT_RLS1_RLOFC|BIT_RLS1_RLOFD)){
			if (rrts1 & BIT_RRTS1_RLOF){
				fe->fe_alarm |= WAN_TE_BIT_OOF_ALARM;		
				DEBUG_EVENT("%s: OOF alarm is ON\n",
							fe->name);
			}else{
				fe->fe_alarm &= ~WAN_TE_BIT_OOF_ALARM;		
				DEBUG_EVENT("%s: OOF alarm is OFF\n",
							fe->name);
			}
		}
		WRITE_REG(REG_RLS1, rls1);
		if (IS_T1_FEMEDIA(fe)){
			if (IS_TE_OOF_ALARM(fe->fe_alarm) &&
			    IS_TE_LOS_ALARM(fe->fe_alarm)){
				DEBUG_EVENT("%s: RED alarm is ON\n",
							fe->name);
				fe->fe_alarm |= WAN_TE_BIT_RED_ALARM;
			}else{
				DEBUG_EVENT("%s: RED alarm is OFF\n",
							fe->name);
				fe->fe_alarm &= ~WAN_TE_BIT_RED_ALARM;
			}
		}
	}
	if (istatus & BIT_RIIR_RLS2){
		unsigned char	rls2 = READ_REG(REG_RLS2);
		if (IS_E1_FEMEDIA(fe)){
			DEBUG_TE1("%s: E1 RX Latched Status Register 2 %02X\n",
					fe->name, rls2);
			if (rls2 & BIT_RLS2_E1_RSA1){
				DEBUG_EVENT(
				"%s: Receive Signalling All Ones Event!\n",
						fe->name);
			}
			if (rls2 & BIT_RLS2_E1_RSA0){
				DEBUG_EVENT(
				"%s: Receive Signalling All Ones Event!\n",
						fe->name);
			}
			if (rls2 & BIT_RLS2_E1_RCMF){
				DEBUG_EVENT(
				"%s: Receive CRC4 Multiframe Event!\n",
						fe->name);
			}
			if (rls2 & BIT_RLS2_E1_RAF){
				DEBUG_EVENT(
				"%s: Receive Align Frame Event!\n",
						fe->name);
			}
		}
		WRITE_REG(REG_RLS2, rls2);
	}
	if (istatus & BIT_RIIR_RLS3){
		unsigned char	rls3 = READ_REG(REG_RLS3);
		DEBUG_TE1("%s: RX Latched Status Register 3 %02X\n",
					fe->name, rls3);
		if (IS_T1_FEMEDIA(fe)){
			if (rls3 & BIT_RLS3_T1_LORCC){
				DEBUG_EVENT(
				"%s: Loss of Receive Clock Condition Clear!\n",
						fe->name);
			}
			if (rls3 & BIT_RLS3_T1_LORCD){
				DEBUG_EVENT(
				"%s: Loss of Receive Clock Condition Detect!\n",
						fe->name);
			}
		}else{
			if (rls3 & BIT_RLS3_E1_LORCC){
				DEBUG_EVENT(
				"%s: Loss of Receive Clock Condition Clear!\n",
						fe->name);
			}
			if (rls3 & BIT_RLS3_E1_LORCD){
				DEBUG_EVENT(
				"%s: Loss of Receive Clock Condition Detect!\n",
						fe->name);
			}
		}
		WRITE_REG(REG_RLS3, rls3);
	}
	if (istatus & BIT_RIIR_RLS4){
		unsigned char	rls4 = READ_REG(REG_RLS4);
		DEBUG_TE1("%s: RX Latched Status Register 4 %02X\n",
					fe->name, rls4);
		if (rls4 & BIT_RLS4_RSCOS){
			DEBUG_EVENT(
			"%s: Receive Signalling status changed!\n",
					fe->name);
			sdla_ds_te1_check_rbsbits(fe, 1, ENABLE_ALL_CHANNELS, 1); 
			sdla_ds_te1_check_rbsbits(fe, 9, ENABLE_ALL_CHANNELS, 1);
			sdla_ds_te1_check_rbsbits(fe, 17, ENABLE_ALL_CHANNELS, 1);
			if (IS_E1_FEMEDIA(fe)){
				sdla_ds_te1_check_rbsbits(fe, 25, ENABLE_ALL_CHANNELS, 1);
			}
		}
		if (rls4 & BIT_RLS4_TIMER){
			DEBUG_ISR(
			"%s: Performance monitor counters have been updated!\n",
					fe->name);
			sdla_ds_te1_pmon(fe, WAN_FE_PMON_READ);
		}
		WRITE_REG(REG_RLS4, rls4);
	}
	if (istatus & BIT_RIIR_RLS5){
		unsigned char	rls5 = READ_REG(REG_RLS5);
		DEBUG_TE1("%s: RX Latched Status Register 5 %02X\n",
					fe->name, rls5);
		if (rls5 & BIT_RLS5_ROVR){
			DEBUG_EVENT("%s: Receive FIFO overrun (HDLC)!\n",
					fe->name);
		}					
		if (rls5 & BIT_RLS5_RHOBT){
			DEBUG_EVENT("%s: Receive HDLC Opening Byte Event (HDLC)!\n",
					fe->name);
		}					
		if (rls5 & BIT_RLS5_RPE){
			DEBUG_EVENT("%s: Receive Packet End Event (HDLC)!\n",
					fe->name);
		}					
		if (rls5 & BIT_RLS5_RPS){
			DEBUG_EVENT("%s: Receive Packet Start Event (HDLC)!\n",
					fe->name);
		}					
		if (rls5 & BIT_RLS5_RHWMS){
			DEBUG_EVENT("%s: Receive FIFO Above High Watermark Set Event (HDLC)!\n",
					fe->name);
		}					
		if (rls5 & BIT_RLS5_RNES){
			DEBUG_EVENT("%s: Receive FIFO Not Empty Set Event (HDLC)!\n",
					fe->name);
		}					
		WRITE_REG(REG_RLS5, rls5);
	}
#if 0	
	if (istatus & BIT_RIIR_RLS6){
	}
#endif	
	if (istatus & BIT_RIIR_RLS7){
		unsigned char	rls7 = READ_REG(REG_RLS7);
		DEBUG_TE1("%s: RX Latched Status Register 7 %02X\n",
					fe->name, rls7);
		if (rls7 & BIT_RLS7_RRAI_CI){
			DEBUG_EVENT("%s: Receive RAI-CI Detect!\n",
					fe->name);
		}					
		if (rls7 & BIT_RLS7_RAIS_CI){
			DEBUG_EVENT("%s: Receive RAI-CI Detect!\n",
					fe->name);
		}					
		if (rls7 & BIT_RLS7_RSLC96){
			DEBUG_EVENT("%s: Receive SLC-96 Alignment Event!\n",
					fe->name);
		}					
		if (rls7 & BIT_RLS7_RFDLF){
			DEBUG_EVENT("%s: Receive FDL Register Full Event!\n",
					fe->name);
		}					
		if (rls7 & BIT_RLS7_BC){
			DEBUG_EVENT("%s: BOC Clear Event!\n",
					fe->name);
		}					
		if (rls7 & BIT_RLS7_BD){
			DEBUG_EVENT("%s: BOC Detect Event!\n",
					fe->name);
		}					
		WRITE_REG(REG_RLS7, rls7);
	}
	
	return 0;
}

static int sdla_ds_te1_framer_tx_intr(sdla_fe_t *fe) 
{
	unsigned char	istatus;
	unsigned char	status;

	istatus = READ_REG(REG_TIIR);
	if (istatus & BIT_TIIR_TLS1){
		status = READ_REG(REG_TLS1);
		if (status & BIT_TLS1_TPDV){
			DEBUG_EVENT(
			"%s: Transmit Pulse Density Violation Event!\n",
					fe->name);
		}
		if (status & BIT_TLS1_LOTCC){
			DEBUG_EVENT(
			"%s: Loss of Transmit Clock condition Clear!\n",
					fe->name);
		}
		WRITE_REG(REG_TLS1, status);
	}
	if (istatus & BIT_TIIR_TLS2){
		status = READ_REG(REG_TLS2);
		WRITE_REG(REG_TLS2, status);
	}
	if (istatus & BIT_TIIR_TLS3){
		status = READ_REG(REG_TLS3);
		WRITE_REG(REG_TLS3, status);
	}
	return 0;
}

static int sdla_ds_te1_bert_intr(sdla_fe_t *fe) 
{
	unsigned char	blsr = READ_REG(REG_BLSR);
	
	if (blsr & BIT_BLSR_BBED){
		DEBUG_EVENT("%s: BERT bit error detected!\n",
					fe->name);
	}
	if (blsr & BIT_BLSR_BBCO){
		DEBUG_EVENT("%s: BERT Bit Counter overflows!\n",
					fe->name);
	}
	if (blsr & BIT_BLSR_BECO){
		DEBUG_EVENT("%s: BERT Error Counter overflows!\n",
					fe->name);
	}
	if (blsr & BIT_BLSR_BRA1){
		DEBUG_EVENT("%s: BERT Receive All-Ones Condition!\n",
					fe->name);
	}
	if (blsr & BIT_BLSR_BRA0){
		DEBUG_EVENT("%s: BERT Receive All-Zeros Condition!\n",
					fe->name);	
	}
	if (blsr & BIT_BLSR_BRLOS){
		DEBUG_EVENT("%s: BERT Receive Loss of Synchronization Condition!\n",
					fe->name);	
	}
	if (blsr & BIT_BLSR_BSYNC){
		DEBUG_EVENT("%s: BERT in synchronization Condition!\n",
					fe->name);	
	}

	WRITE_REG(REG_BLSR, blsr);
	return 0;
}

static int sdla_ds_te1_liu_intr(sdla_fe_t *fe) 
{
	unsigned char	llsr = READ_REG(REG_LLSR);
	unsigned char	lrsr = READ_REG(REG_LRSR);

	if (llsr & BIT_LLSR_JALTC){
		DEBUG_TE1("%s: Jitter Attenuator Limit Trip Clear!\n",
					fe->name);
	}
	if (llsr & BIT_LLSR_JALTS){
		DEBUG_TE1("%s: Jitter Attenuator Limit Trip Set!\n",
					fe->name);
	}
	if (llsr & (BIT_LLSR_OCC | BIT_LLSR_OCD)){
#if 0	
		if (IS_T1_FEMEDIA(fe) && WAN_TE1_LBO(fe) == LBO 5/6/7){
			/* Use LRSR register to get real value. LLSR is not
			** valid for these modes */
		}
#endif
		if (lrsr & BIT_LRSR_OCS){
			if (!(fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_OC)){
				DEBUG_TE1("%s: Open Circuit is detected!\n",
					fe->name);
				fe->liu_alarm |= WAN_TE_BIT_LIU_ALARM_OC;
			}
		}else{
			if (fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_OC){
				DEBUG_TE1("%s: Open Circuit is cleared!\n",
					fe->name);
				fe->liu_alarm &= ~WAN_TE_BIT_LIU_ALARM_OC;
			}
		}
	}
	if (llsr & (BIT_LLSR_SCC | BIT_LLSR_SCD)){
		if (lrsr & BIT_LRSR_SCS){
			if (!(fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_SC)){
				DEBUG_EVENT("%s: Short Circuit is detected!\n",
					fe->name);
				fe->liu_alarm |= WAN_TE_BIT_LIU_ALARM_SC;
			}
		}else{
			if (fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_SC){
				DEBUG_EVENT("%s: Short Circuit is cleared!\n",
					fe->name);
				fe->liu_alarm &= ~WAN_TE_BIT_LIU_ALARM_SC;	
			}
		}
	}
	if (llsr & (BIT_LLSR_LOSC | BIT_LLSR_LOSD)){
		if (lrsr & BIT_LRSR_LOSS){
			if (!(fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_LOS)){
				DEBUG_EVENT("%s: Lost of Signal is detected!\n",
					fe->name);
				fe->liu_alarm |= WAN_TE_BIT_LIU_ALARM_LOS;
			}
		}else{
			if (fe->liu_alarm & WAN_TE_BIT_LIU_ALARM_LOS){
				DEBUG_EVENT("%s: Lost of Signal is cleared!\n",
					fe->name);
				fe->liu_alarm &= ~WAN_TE_BIT_LIU_ALARM_LOS;	
			}
		}
	}
	WRITE_REG(REG_LLSR, llsr);
	return 0;
}

static int sdla_ds_te1_check_intr(sdla_fe_t *fe) 
{
	unsigned char	framer_istatus;
	unsigned char	liu_istatus;
	unsigned char	bert_istatus;
	
	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	framer_istatus	= __READ_REG(REG_GFISR);
	liu_istatus	= __READ_REG(REG_GLISR);
	bert_istatus	= __READ_REG(REG_GBISR);

	//if (framer_istatus & (1 << WAN_FE_LINENO(fe))){
	if (framer_istatus & (1 << WAN_DS_REGBITMAP(fe))){
		DEBUG_ISR("%s: Interrupt for line %d (framer)\n",
					fe->name, WAN_FE_LINENO(fe));
		return 1;
	}
	//if (liu_istatus & (1 << WAN_FE_LINENO(fe))){
	if (liu_istatus & (1 << WAN_DS_REGBITMAP(fe))){
		DEBUG_ISR("%s: Interrupt for line %d (liu)\n",
					fe->name, WAN_FE_LINENO(fe));
		return 1;
	}
	//if (bert_istatus & (1 << WAN_FE_LINENO(fe))){
	if (bert_istatus & (1 << WAN_DS_REGBITMAP(fe))){
		DEBUG_ISR("%s: Interrupt for line %d (bert)\n",
					fe->name, WAN_FE_LINENO(fe));
		return 1;
	}
	DEBUG_ISR("%s: This interrupt not for this port %d\n",
				fe->name,
				WAN_FE_LINENO(fe)+1);
	return 0;
}

static int sdla_ds_te1_intr(sdla_fe_t *fe) 
{
	unsigned char	status = fe->fe_status;
	unsigned char	framer_istatus;
	unsigned char	liu_istatus;
	unsigned char	bert_istatus;
	
	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	framer_istatus = READ_REG(REG_GFISR);
	liu_istatus = READ_REG(REG_GLISR);
	bert_istatus = READ_REG(REG_GBISR);

	//if (framer_istatus & (1 << WAN_FE_LINENO(fe))){
	if (framer_istatus & (1 << WAN_DS_REGBITMAP(fe))){
		sdla_ds_te1_framer_rx_intr(fe);
		sdla_ds_te1_framer_tx_intr(fe);
		//WRITE_REG(REG_GFISR, (1<<WAN_FE_LINENO(fe)));
		WRITE_REG(REG_GFISR, (1<<WAN_DS_REGBITMAP(fe)));
	}
	//if (liu_istatus & (1 << WAN_FE_LINENO(fe))){
	if (liu_istatus & (1 << WAN_DS_REGBITMAP(fe))){
		sdla_ds_te1_liu_intr(fe);
		//WRITE_REG(REG_GLISR, (1<<WAN_FE_LINENO(fe)));
		WRITE_REG(REG_GLISR, (1<<WAN_DS_REGBITMAP(fe)));
	}
	//if (bert_istatus & (1 << WAN_FE_LINENO(fe))){
	if (bert_istatus & (1 << WAN_DS_REGBITMAP(fe))){
		sdla_ds_te1_bert_intr(fe);
		//WRITE_REG(REG_GBISR, (1<<WAN_FE_LINENO(fe)));
		WRITE_REG(REG_GBISR, (1<<WAN_DS_REGBITMAP(fe)));
	}

	DEBUG_TE1("%s: FE Interrupt Alarms=0x%X\n",
			fe->name,fe->fe_alarm);

	sdla_ds_te1_set_status(fe, fe->fe_alarm);
	if (status != fe->fe_status){
		if (fe->fe_status != FE_CONNECTED){
			sdla_fe_timer_event_t	event;
			/* AL: March 1, 2006: Mask global FE intr */
			sdla_ds_te1_intr_ctrl(
					fe, 0,
					WAN_TE_INTR_GLOBAL,
					WAN_FE_INTR_MASK,
					0x00);
			/* Disable automatic update */
			sdla_ds_te1_intr_ctrl(
					fe, 0,
					WAN_TE_INTR_PMON,
					WAN_FE_INTR_MASK,
					0x00);
			/* Start LINKDOWN poll */
			event.type	= TE_LINKDOWN_TIMER;
			event.delay	= POLLING_TE1_TIMER*5;
			sdla_ds_te1_add_event(fe, &event);
		}
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_timer()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static void sdla_ds_te1_timer(void* pfe)
#elif defined(__WINDOWS__)
static void sdla_ds_te1_timer(IN PKDPC Dpc, void* pfe, void* arg2, void* arg3)
#else
static void sdla_ds_te1_timer(unsigned long pfe)
#endif
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_t 		*card = (sdla_t*)fe->card;
	wan_device_t	*wandev = &card->wandev;
	wan_smp_flag_t	smp_flags;
	int		empty = 1;

	DEBUG_TEST("[TE1] %s: TE1 timer!\n", fe->name);
	if (wan_test_bit(TE_TIMER_KILL,(void*)&fe->te_param.critical)){
		wan_clear_bit(TE_TIMER_RUNNING,(void*)&fe->te_param.critical);
		return;
	}
	if (!wan_test_bit(TE_TIMER_RUNNING,(void*)&fe->te_param.critical)){
		/* Somebody clear this bit */
		DEBUG_EVENT("WARNING: %s: Timer bit is cleared (should never happened)!\n", 
					fe->name);
		return;
	}
	wan_clear_bit(TE_TIMER_RUNNING,(void*)&fe->te_param.critical);
			
	/* Enable hardware interrupt for TE1 */
	wan_spin_lock_irq(&fe->lock,&smp_flags);	
	empty = WAN_LIST_EMPTY(&fe->event);
	wan_spin_unlock_irq(&fe->lock,&smp_flags);	

	if (!empty){
		if (wan_test_and_set_bit(TE_TIMER_EVENT_PENDING,(void*)&fe->te_param.critical)){
			DEBUG_EVENT("%s: RM timer event is pending!\n", fe->name);
			return;
		}	
		if (wandev->fe_enable_timer){
			wandev->fe_enable_timer(fe->card);
		}else{
			sdla_ds_te1_polling(fe);
		}
	}else{
		sdla_ds_te1_add_timer(fe, 1000);
	}
	return;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_add_timer()	
 *
 * Description: Enable software timer interrupt in delay ms.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_add_timer(sdla_fe_t* fe, unsigned long delay)
{
	int	err;
	
	err = wan_add_timer(&fe->timer, delay * HZ / 1000);
	if (err){
		/* Failed to add timer */
		return -EINVAL;
	}
	wan_set_bit(TE_TIMER_RUNNING,(void*)&fe->te_param.critical);
	return 0;	
}

/*
 ******************************************************************************
 *				sdla_ds_te1_add_event()	
 *
 * Description: Enable software timer interrupt in delay ms.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int
sdla_ds_te1_add_event(sdla_fe_t *fe, sdla_fe_timer_event_t *event)
{
	sdla_t			*card = (sdla_t*)fe->card;
	sdla_fe_timer_event_t	*tevent = NULL;
	wan_smp_flag_t		smp_flags;

	WAN_ASSERT_RC(card == NULL, -EINVAL);
	
	DEBUG_TE1("%s: Add new DS Event=0x%X\n",
			fe->name, event->type);

	/* Creating event timer */	
	tevent = wan_malloc(sizeof(sdla_fe_timer_event_t));
	if (tevent == NULL){
		DEBUG_EVENT(
       		"%s: Failed to allocate memory for timer event!\n",
					fe->name);
		return -EINVAL;
	}
	
	memcpy(tevent, event, sizeof(sdla_fe_timer_event_t));
	
#if 0	
	DEBUG_EVENT("%s: %s:%d: ---------------START ----------------------\n",
				fe->name, __FUNCTION__,__LINE__);
	WARN_ON(1);
	DEBUG_EVENT("%s: %s:%d: ---------------STOP ----------------------\n",
				fe->name, __FUNCTION__,__LINE__);
#endif	
	wan_spin_lock_irq(&fe->lock,&smp_flags);	
	if (WAN_LIST_EMPTY(&fe->event)){
		WAN_LIST_INSERT_HEAD(&fe->event, tevent, next);
	}else{
		sdla_fe_timer_event_t	*tmp;
		WAN_LIST_FOREACH(tmp, &fe->event, next){
			if (!WAN_LIST_NEXT(tmp, next)) break;
		}
		if (tmp == NULL){
			DEBUG_EVENT("%s: Internal Error!!!\n", fe->name);
			wan_spin_unlock_irq(&fe->lock, &smp_flags);	
			return -EINVAL;
		}
		WAN_LIST_INSERT_AFTER(tmp, tevent, next);
	}
	wan_spin_unlock_irq(&fe->lock, &smp_flags);	
	return 0;
}


/*
 ******************************************************************************
 *				sdla_ds_te1_polling()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_polling(sdla_fe_t* fe)
{
	sdla_t			*card = (sdla_t*)fe->card;
	sdla_fe_timer_event_t	*event;
	wan_smp_flag_t		smp_flags;
	u_int8_t	pending = 0;
	unsigned char	value;
	unsigned int	ch, bit, off;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

#if 0	
	DEBUG_EVENT("%s: %s:%d: ---------------START ----------------------\n",
				fe->name, __FUNCTION__,__LINE__);
	WARN_ON(1);
	DEBUG_EVENT("%s: %s:%d: ---------------STOP ----------------------\n",
				fe->name, __FUNCTION__,__LINE__);
#endif
	wan_spin_lock_irq(&fe->lock,&smp_flags);			
	if (WAN_LIST_EMPTY(&fe->event)){
		wan_clear_bit(TE_TIMER_EVENT_PENDING,(void*)&fe->te_param.critical);
		wan_spin_unlock_irq(&fe->lock,&smp_flags);	
		DEBUG_EVENT("%s: WARNING: No FE events in a queue!\n",
					fe->name);
		sdla_ds_te1_add_timer(fe, HZ);
		return 0;
	}
	event = WAN_LIST_FIRST(&fe->event);
	WAN_LIST_REMOVE(event, next);
	wan_spin_unlock_irq(&fe->lock,&smp_flags);
		
	DEBUG_TE1("%s: TE1 Polling State=%s Event=%X!\n", 
			fe->name, WAN_FE_STATUS_DECODE(fe), event->type);	
	switch(event->type){
	case TE_LINKDOWN_TIMER:
		sdla_ds_te1_read_alarms(fe, WAN_FE_ALARM_READ);
		sdla_ds_te1_pmon(fe, WAN_FE_PMON_UPDATE|WAN_FE_PMON_READ);
		sdla_ds_te1_set_status(fe, fe->fe_alarm);
		if (fe->fe_status == FE_CONNECTED){
			event->type	= TE_LINKUP_TIMER;
			event->delay	= POLLING_TE1_TIMER;
			pending	= 1;
		}else{
			event->type	= TE_LINKDOWN_TIMER;
			event->delay	= POLLING_TE1_TIMER;
			pending	= 1;
		}
		break;

	case TE_LINKUP_TIMER:
		/* ALEX: 
		** Do not update protocol front end state from TE_LINKDOWN_TIMER
		** because it cause to stay longer in interrupt handler
	        ** (critical for XILINX code) */
		if (fe->fe_status == FE_CONNECTED){
			/* Enable Basic Interrupt */
			sdla_ds_te1_intr_ctrl(	fe, 0, 
						WAN_TE_INTR_GLOBAL, 
						WAN_FE_INTR_ENABLE, 
						0x00);
			/* Enable automatic update pmon counters */
			sdla_ds_te1_intr_ctrl(	fe, 0, 
						WAN_TE_INTR_PMON, 
						WAN_FE_INTR_ENABLE, 
						0x00);
			if (card->wandev.te_link_state){
				card->wandev.te_link_state(card);
			}
		}else{
			event->type	= TE_LINKDOWN_TIMER;
			event->delay	= POLLING_TE1_TIMER;
			pending	= 1;
		}
		break;

	case TE_RBS_READ:
		/* Physically read RBS status and print */
		sdla_ds_te1_rbs_print(fe, 0);
		break;

	case TE_SET_RBS:
		/* Set RBS bits */
		DEBUG_TE1("%s: Set ABCD bits (%X) for channel %d!\n",
						fe->name,
						event->te_event.rbs_abcd,
						event->te_event.rbs_channel);
		sdla_ds_te1_set_rbsbits(fe, 
					event->te_event.rbs_channel,
					event->te_event.rbs_abcd);
		break;
	
	case TE_RBS_ENABLE:
	case TE_RBS_DISABLE:
		value = READ_REG(REG_TCR1);
		if (IS_T1_FEMEDIA(fe)){
			if (event->type == TE_RBS_ENABLE){		
				value |= BIT_TCR1_T1_TSSE;
			}else{
				value &= ~BIT_TCR1_T1_TSSE;
			}
		}else{
			if (event->type == TE_RBS_ENABLE){		
				value |= BIT_TCR1_E1_T16S;
			}else{
				value &= ~BIT_TCR1_E1_T16S;
			}
		}
		WRITE_REG(REG_TCR1, value);
		
		for(ch = 1; ch <= fe->te_param.max_channels; ch++){
			if (!wan_test_bit(ch, &event->te_event.ch_map)){
				continue;
			}
			if (IS_T1_FEMEDIA(fe)){
				bit = (ch-1) % 8;
				off = (ch-1) / 8;
			}else{
				if (ch == 16) continue;
				bit = ch % 8;
				off = ch / 8;
			}
			value = READ_REG(REG_SSIE1+off);
			if (event->type == TE_RBS_ENABLE){
				value |= (1<<bit);
			}else{
				value &= ~(1<<bit);
			}
			DEBUG_TE1("%s: Channel %d: SSIE=%02X Val=%02X\n",
					fe->name, ch, REG_SSIE1+off, value);
			WRITE_REG(REG_SSIE1+off, value);	
		}
		break;

	case TE_SET_LB_MODE:
		sdla_ds_te1_set_lbmode(fe, event->te_event.lb_type, event->mode); 
		break;
				
	case TE_POLL_CONFIG:			
		DEBUG_EVENT("%s: Re-configuring %s Front-End chip...\n",
						fe->name, FE_MEDIA_DECODE(fe));
		if (sdla_ds_te1_chip_config(fe)){
			DEBUG_EVENT("%s: Failed to re-configuring Front-End chip!\n",
					fe->name);
			return -EINVAL;
		}
		/* Enable interrupts */
		sdla_ds_te1_intr_ctrl(fe, 0, WAN_TE_INTR_BASIC, WAN_FE_INTR_ENABLE, 0x00); 
		/* Enable manual update pmon counter */
		sdla_ds_te1_intr_ctrl(fe, 0, WAN_TE_INTR_PMON, WAN_FE_INTR_MASK, 0x00);
		event->type	= TE_LINKDOWN_TIMER;
		event->delay	= POLLING_TE1_TIMER;
		pending = 1;
		break;
		
	case TE_POLL_READ:
		DEBUG_EVENT("%s: Reading %s Front-End register: Reg[%04X]=%02X\n",
					fe->name, FE_MEDIA_DECODE(fe),
					event->te_event.reg,
					READ_REG(event->te_event.reg));
		break;
		
	case TE_POLL_WRITE:
		DEBUG_EVENT("%s: Writting %s Front-End register: Reg[%04X]=%02X\n",
					fe->name, FE_MEDIA_DECODE(fe),
					event->te_event.reg,
					event->te_event.value);
		WRITE_REG(event->te_event.reg, event->te_event.value);
		break;

	default:
		DEBUG_EVENT("%s: Unknown DS TE1 Polling type %02X\n",
				fe->name, event->type);
		break;
	}
	/* Add new event */
	if (pending){
		sdla_ds_te1_add_event(fe, event);
	}
	wan_clear_bit(TE_TIMER_EVENT_PENDING,(void*)&fe->te_param.critical);
	if (event) wan_free(event);

	/* Add fe timer */
	event = WAN_LIST_FIRST(&fe->event);
	if (event){
		sdla_ds_te1_add_timer(fe, event->delay);	
	}else{
		sdla_ds_te1_add_timer(fe, HZ);	
	}
	return 0;			
}

/*
 ******************************************************************************
 *				sdla_ds_te1_flush_pmon()	
 *
 * Description: Flush Dallas performance monitoring counters
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_flush_pmon(sdla_fe_t *fe)
{
	sdla_te_pmon_t	*pmon = &fe->fe_stats.te_pmon;
	sdla_ds_te1_pmon(fe, WAN_FE_PMON_UPDATE | WAN_FE_PMON_READ);
	pmon->lcv_errors	= 0;	
	pmon->oof_errors	= 0;
	pmon->bee_errors	= 0;
	pmon->crc4_errors	= 0;
	pmon->feb_errors	= 0;
	pmon->fas_errors	= 0;
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_pmon()	
 *
 * Description: Read DLS performance monitoring counters
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_pmon(sdla_fe_t *fe, int action)
{
	sdla_te_pmon_t	*pmon = &fe->fe_stats.te_pmon;
	u16		pmon1 = 0, pmon2 = 0, pmon3 = 0, pmon4 = 0;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	if (IS_FE_PMON_UPDATE(action)){
		unsigned char	ercnt = READ_REG(REG_ERCNT);
		WRITE_REG(REG_ERCNT, ercnt & ~BIT_ERCNT_MECU);
		WP_DELAY(10);
		WRITE_REG(REG_ERCNT, ercnt | BIT_ERCNT_MECU);
		WP_DELAY(250);
		WRITE_REG(REG_ERCNT, ercnt);
	}

	if (IS_FE_PMON_READ(action)){
		pmon->mask = 0x00;
		/* Line code violation count register */	
		pmon1 = READ_REG(REG_LCVCR1) << 8 | READ_REG(REG_LCVCR2);
		/* Path code violation count for E1/T1 */
		pmon2 = READ_REG(REG_PCVCR1) << 8 | READ_REG(REG_PCVCR2);
		/* OOF Error for T1/E1 */
		pmon3 = READ_REG(REG_FOSCR1) << 8 | READ_REG(REG_FOSCR2);
		if (IS_E1_FEMEDIA(fe)){
			/* E-bit counter (Far End Block Errors) */
			pmon4 = READ_REG(REG_E1EBCR1) << 8 | READ_REG(REG_E1EBCR2);
		}
	
		pmon->lcv_diff = pmon1;
		pmon->lcv_errors = pmon->lcv_errors + pmon1;	
		if (IS_T1_FEMEDIA(fe)){
			pmon->mask =	WAN_TE_BIT_PMON_LCV |
					WAN_TE_BIT_PMON_BEE |
					WAN_TE_BIT_PMON_OOF;
			pmon->bee_diff = pmon2;
			pmon->bee_errors = pmon->bee_errors + pmon2;
			pmon->oof_diff = pmon3;
			pmon->oof_errors = pmon->oof_errors + pmon3;
		}else{
			pmon->mask =	WAN_TE_BIT_PMON_LCV |
					WAN_TE_BIT_PMON_CRC4 |
					WAN_TE_BIT_PMON_FAS |
					WAN_TE_BIT_PMON_FEB;
			pmon->crc4_diff = pmon2;
			pmon->crc4_errors = pmon->crc4_errors + pmon2;
			pmon->fas_diff = pmon3;
			pmon->fas_errors = pmon->fas_errors + pmon3;
			pmon->feb_diff = pmon4;
			pmon->feb_errors = pmon->feb_errors + pmon4;
		}
	}
	if (IS_FE_PMON_PRINT(action)){
		DEBUG_EVENT("%s: Line Code Viilation:\t\t%d\n",
				fe->name, pmon->lcv_errors);
		if (IS_T1_FEMEDIA(fe)){
			DEBUG_EVENT("%s: Bit error events:\t\t%d\n",
					fe->name, pmon->bee_errors);
			DEBUG_EVENT("%s: Frames out of sync:\t\t%d\n",
					fe->name, pmon->oof_errors);
		}else{
			DEBUG_EVENT("%s: CRC4 errors:\t\t\t%d\n",
					fe->name, pmon->crc4_errors);
			DEBUG_EVENT("%s: Frame Alignment signal errors:\t%d\n",
					fe->name, pmon->fas_errors);
			DEBUG_EVENT("%s: Far End Block errors:\t\t%d\n",
					fe->name, pmon->feb_errors);
		}
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_rxlevel()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_rxlevel(sdla_fe_t* fe) 
{
	int		index = 0;
	unsigned char	rxlevel;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	rxlevel = READ_REG(REG_LRSL);
	index = (rxlevel >> REG_LRSL_SHIFT) & REG_LRSL_MASK;
	
	memset(fe->fe_stats.u.te1_stats.rxlevel, 0, WAN_TE_RXLEVEL_LEN);
	if (IS_T1_FEMEDIA(fe)){
		memcpy(	fe->fe_stats.u.te1_stats.rxlevel,
			wan_t1_ds_rxlevel[index],
			strlen(wan_t1_ds_rxlevel[index]));
	}else{
		memcpy(	fe->fe_stats.u.te1_stats.rxlevel,
			wan_e1_ds_rxlevel[index],
			strlen(wan_e1_ds_rxlevel[index]));	
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_liu_alb()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_liu_alb(sdla_fe_t* fe, unsigned char mode) 
{
	unsigned char	value;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	value = READ_REG(REG_LMCR);
	if (mode == WAN_TE1_ACTIVATE_LB){
		value |= BIT_LMCR_ALB;
	}else{
		value &= ~BIT_LMCR_ALB;
	}
	WRITE_REG(REG_LMCR, value);
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_liu_llb()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_liu_llb(sdla_fe_t* fe, unsigned char mode) 
{
	unsigned char	value;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	value = READ_REG(REG_LMCR);
	if (mode == WAN_TE1_ACTIVATE_LB){
		value |= BIT_LMCR_LLB;
	}else{
		value &= ~BIT_LMCR_LLB;
	}
	WRITE_REG(REG_LMCR, value);
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_liu_rlb()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_liu_rlb(sdla_fe_t* fe, unsigned char mode) 
{
	unsigned char	value;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	value = READ_REG(REG_LMCR);
	if (mode == WAN_TE1_ACTIVATE_LB){
		value |= BIT_LMCR_RLB;
	}else{
		value &= ~BIT_LMCR_RLB;
	}
	WRITE_REG(REG_LMCR, value);
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_fr_flb()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_fr_flb(sdla_fe_t* fe, unsigned char mode) 
{
	unsigned char	value;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	value = READ_REG(REG_RCR3);
	if (mode == WAN_TE1_ACTIVATE_LB){
		value |= BIT_RCR3_FLB;
	}else{
		value &= ~BIT_RCR3_FLB;
	}
	WRITE_REG(REG_LMCR, value);
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_fr_plb()
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_fr_plb(sdla_fe_t* fe, unsigned char mode) 
{
	unsigned char	value;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	value = READ_REG(REG_RCR3);
	if (mode == WAN_TE1_ACTIVATE_LB){
		value |= BIT_RCR3_PLB;
	}else{
		value &= ~BIT_RCR3_PLB;
	}
	WRITE_REG(REG_LMCR, value);
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_set_lbmode()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int 
sdla_ds_te1_set_lbmode(sdla_fe_t *fe, unsigned char type, unsigned char mode) 
{

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);
	DEBUG_EVENT("%s: %s %s mode...\n",
			fe->name,
			WAN_TE1_LB_MODE_DECODE(mode),
			WAN_TE1_LB_TYPE_DECODE(type));
	switch(type){
	case WAN_TE1_LIU_ALB_MODE:
		sdla_ds_te1_liu_alb(fe, mode);
		break;
	case WAN_TE1_LIU_LLB_MODE:
		sdla_ds_te1_liu_llb(fe, mode);
		break;
	case WAN_TE1_LIU_RLB_MODE:
		sdla_ds_te1_liu_rlb(fe, mode);
		break;
	case WAN_TE1_LIU_DLB_MODE:
		sdla_ds_te1_liu_llb(fe, mode);
		sdla_ds_te1_liu_rlb(fe, mode);
		break;
	case WAN_TE1_FR_FLB_MODE:
		sdla_ds_te1_fr_flb(fe, mode);
		break;
	case WAN_TE1_FR_PLB_MODE:
		sdla_ds_te1_fr_plb(fe, mode);
		break;
	case WAN_TE1_FR_RLB_MODE:
		break;
	}
	return 0;
}

/*
 ******************************************************************************
 *				sdla_ds_te1_udp()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int sdla_ds_te1_udp(sdla_fe_t *fe, void* p_udp_cmd, unsigned char* data)
{
	wan_cmd_t		*udp_cmd = (wan_cmd_t*)p_udp_cmd;
	sdla_fe_debug_t		*fe_debug;
	sdla_fe_timer_event_t	event;
	int			err = 0;

	switch(udp_cmd->wan_cmd_command){
	case WAN_GET_MEDIA_TYPE:
		data[0] = (IS_T1_FEMEDIA(fe) ? WAN_MEDIA_T1 :
			   IS_E1_FEMEDIA(fe) ? WAN_MEDIA_E1 :
					    WAN_MEDIA_NONE);
		udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		udp_cmd->wan_cmd_data_len = sizeof(unsigned char); 
		break;

	case WAN_FE_SET_LB_MODE:
		/* Activate/Deactivate Line Loopback modes */
#if 1
		event.type		= TE_SET_LB_MODE;	
		event.te_event.lb_type	= data[0];		/* LB type */
		event.mode		= data[1];		/* LB mode (activate/deactivate) */
		event.delay		= POLLING_TE1_TIMER;
		err = sdla_ds_te1_add_event(fe, &event);	
#else	    	
		err = sdla_ds_te1_set_lbmode(fe, data[0], data[1]); 
#endif		
	    	udp_cmd->wan_cmd_return_code = 
				(!err) ? WAN_CMD_OK : WAN_UDP_FAILED_CMD;
	    	udp_cmd->wan_cmd_data_len = 0x00;
		break;

	case WAN_FE_GET_STAT:
		/* TE1 Update T1/E1 perfomance counters */
		sdla_ds_te1_pmon(fe, WAN_FE_PMON_UPDATE|WAN_FE_PMON_READ);
		if (udp_cmd->wan_cmd_fe_force){
			/* force to read FE alarms */
			DEBUG_EVENT("%s: Force to read Front-End alarms\n",
						fe->name);
			fe->fe_stats.alarms = 
				sdla_ds_te1_read_alarms(fe, WAN_FE_ALARM_READ);
		}
		sdla_ds_te1_rxlevel(fe);
	        memcpy(&data[0], &fe->fe_stats, sizeof(sdla_fe_stats_t));
	        udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
	    	udp_cmd->wan_cmd_data_len = sizeof(sdla_fe_stats_t); 
		break;

 	case WAN_FE_FLUSH_PMON:
		/* TE1 Flush T1/E1 pmon counters */
		sdla_ds_te1_flush_pmon(fe);
	        udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		break;
 
	case WAN_FE_GET_CFG:
		/* Read T1/E1 configuration */
	       	memcpy(&data[0],
	    		&fe->fe_cfg,
		      	sizeof(sdla_fe_cfg_t));
	    	udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
    	    	udp_cmd->wan_cmd_data_len = sizeof(sdla_fe_cfg_t);
	    	break;

	case WAN_FE_SET_DEBUG_MODE:
		fe_debug = (sdla_fe_debug_t*)&data[0];
		switch(fe_debug->type){
		case WAN_FE_DEBUG_RBS:
			if (fe_debug->mode == WAN_FE_DEBUG_RBS_READ){
				DEBUG_EVENT("%s: Reading RBS status!\n",
					fe->name);
				event.type	= TE_RBS_READ;
				event.delay	= POLLING_TE1_TIMER;
				sdla_ds_te1_add_event(fe, &event);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (fe_debug->mode == WAN_FE_DEBUG_RBS_PRINT){
				/* Enable extra debugging */
				sdla_ds_te1_rbs_print(fe, 1);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (fe_debug->mode == WAN_FE_DEBUG_RBS_RX_ENABLE){
				/* Enable extra debugging */
				DEBUG_EVENT("%s: Enable RBS RX DEBUG mode!\n",
					fe->name);
				fe->fe_debug |= WAN_FE_DEBUG_RBS_RX_ENABLE;
				sdla_ds_te1_rbs_print(fe, 1);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (fe_debug->mode == WAN_FE_DEBUG_RBS_TX_ENABLE){
				/* Enable extra debugging */
				DEBUG_EVENT("%s: Enable RBS TX DEBUG mode!\n",
					fe->name);
				fe->fe_debug |= WAN_FE_DEBUG_RBS_TX_ENABLE;
				sdla_ds_te1_rbs_print(fe, 1);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (fe_debug->mode == WAN_FE_DEBUG_RBS_RX_DISABLE){
				/* Disable extra debugging */
				DEBUG_EVENT("%s: Disable RBS RX DEBUG mode!\n",
					fe->name);
				fe->fe_debug &= ~WAN_FE_DEBUG_RBS_RX_ENABLE;
				sdla_ds_te1_rbs_print(fe, 1);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (fe_debug->mode == WAN_FE_DEBUG_RBS_TX_DISABLE){
				/* Disable extra debugging */
				DEBUG_EVENT("%s: Disable RBS TX DEBUG mode!\n",
					fe->name);
				fe->fe_debug &= ~WAN_FE_DEBUG_RBS_TX_ENABLE;
				sdla_ds_te1_rbs_print(fe, 1);
	    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}else if (fe_debug->mode == WAN_FE_DEBUG_RBS_SET){

				if (IS_T1_FEMEDIA(fe)){
					if (fe_debug->fe_debug_rbs.channel < 1 || 
					    fe_debug->fe_debug_rbs.channel > 24){
						DEBUG_EVENT(
						"%s: Invalid channel number %d\n",
							fe->name,
							fe_debug->fe_debug_rbs.channel);
						break;
					}
				}else{
					if (fe_debug->fe_debug_rbs.channel < 0 || 
					    fe_debug->fe_debug_rbs.channel > 31){
						DEBUG_EVENT(
						"%s: Invalid channel number %d\n",
							fe->name,
							fe_debug->fe_debug_rbs.channel);
						break;
					}
				}

				event.type		= TE_RBS_READ;
				event.delay		= POLLING_TE1_TIMER;
				event.te_event.rbs_channel = fe_debug->fe_debug_rbs.channel;
				event.te_event.rbs_abcd	= fe_debug->fe_debug_rbs.abcd;
				sdla_ds_te1_add_event(fe, &event);
				udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			}
			break;
		case WAN_FE_DEBUG_RECONFIG:
			event.type	= TE_POLL_CONFIG;
			event.delay	= POLLING_TE1_TIMER;
			sdla_ds_te1_add_event(fe, &event);
    			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			break;
		case WAN_FE_DEBUG_REG:
			event.type		= (fe_debug->fe_debug_reg.read) ? 
							TE_POLL_READ : TE_POLL_WRITE;
			event.te_event.reg	= fe_debug->fe_debug_reg.reg;
			event.te_event.value	= fe_debug->fe_debug_reg.value;
			event.delay		= POLLING_TE1_TIMER;
			sdla_ds_te1_add_event(fe, &event);
			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			break;
		
		case WAN_FE_DEBUG_ALARM:
		default:
			udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
			break;
		}
    	    	udp_cmd->wan_cmd_data_len = 0;
		break;
	case WAN_FE_TX_MODE:
		fe_debug = (sdla_fe_debug_t*)&data[0];
		switch(fe_debug->mode){
		case WAN_FE_TXMODE_ENABLE:
			DEBUG_TEST("%s: Enable Transmitter!\n",
					fe->name);
			udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
			break;
		case WAN_FE_TXMODE_DISABLE:
			DEBUG_TEST("%s: Disable Transmitter (tx tri-state mode)!\n",
					fe->name);
			udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
			break;
		default:
			udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
		    	udp_cmd->wan_cmd_data_len = 0;
			break;
		}
		break;
	default:
		udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
	    	udp_cmd->wan_cmd_data_len = 0;
		break;
	}
	return 0;
}

static int 
sdla_ds_te1_update_alarm_info(sdla_fe_t* fe, struct seq_file* m, int* stop_cnt)
{

#if !defined(__WINDOWS__)
	PROC_ADD_LINE(m,
		"=============================== %s Alarms ===============================\n",
		FE_MEDIA_DECODE(fe));
	PROC_ADD_LINE(m,
		PROC_STATS_ALARM_FORMAT,
		"ALOS", WAN_TE_ALOS_ALARM(fe->fe_alarm), 
		"LOS", WAN_TE_LOS_ALARM(fe->fe_alarm));
	PROC_ADD_LINE(m,
		PROC_STATS_ALARM_FORMAT,
		"RED", WAN_TE_RED_ALARM(fe->fe_alarm), 
		"AIS", WAN_TE_AIS_ALARM(fe->fe_alarm));
	if (IS_T1_FEMEDIA(fe)){
		PROC_ADD_LINE(m,
			 PROC_STATS_ALARM_FORMAT,
			 "YEL", WAN_TE_YEL_ALARM(fe->fe_alarm),
			 "OOF", WAN_TE_OOF_ALARM(fe->fe_alarm));
	}else{ 
		PROC_ADD_LINE(m,
			PROC_STATS_ALARM_FORMAT,
			"OOF", WAN_TE_OOF_ALARM(fe->fe_alarm), 
			"", "");
	}
	return m->count;	
#endif
	return 0;
}

static int 
sdla_ds_te1_update_pmon_info(sdla_fe_t* fe, struct seq_file* m, int* stop_cnt)
{
		
#if !defined(__WINDOWS__)
	PROC_ADD_LINE(m,
		 "=========================== %s PMON counters ============================\n",
		 FE_MEDIA_DECODE(fe));
	if (IS_T1_FEMEDIA(fe)){
		PROC_ADD_LINE(m,
			PROC_STATS_PMON_FORMAT,
			"Line Code Violation", fe->fe_stats.te_pmon.lcv_errors,
			"Bit Errors", fe->fe_stats.te_pmon.bee_errors);
		PROC_ADD_LINE(m,
			PROC_STATS_PMON_FORMAT,
			"Out of Frame Errors", fe->fe_stats.te_pmon.oof_errors,
			"", (u_int32_t)0);
	}else{
		PROC_ADD_LINE(m,
			PROC_STATS_PMON_FORMAT,
			"Line Code Violation", fe->fe_stats.te_pmon.lcv_errors,
			"CRC4 Errors", fe->fe_stats.te_pmon.crc4_errors);
		PROC_ADD_LINE(m,
			PROC_STATS_PMON_FORMAT,
			"FAS Errors", fe->fe_stats.te_pmon.fas_errors,
			"Far End Block Errors", fe->fe_stats.te_pmon.feb_errors);
	}
	return m->count;
#endif
	return 0;
}


