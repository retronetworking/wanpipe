/***************************************************************************
 * sdla_serial.c	WANPIPE(tm) 
 *		Serial A140 support module for A140 Serial FE.
 *
 * Author(s): 	Nenad Corbic   <ncorbic@sangoma.com>
 *
 * Copyright:	(c) 2007 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * ============================================================================
 * Nov  1, 2007	Nenad Corbic	Initial version.
 ******************************************************************************
 */

/*******************************************************************************
**			   INCLUDE FILES
*******************************************************************************/

#if defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <wanpipe_abstr.h>
# include <wanpipe_common.h>
# include <wanpipe_events.h>
# include <wanpipe.h>
# include <sdla_serial.h>
# include <wanpipe_events.h>
# include <if_wanpipe_common.h>	/* for 'wanpipe_common_t' used in 'sdla_aft_te1.h'*/
# include <sdla_aft_te1.h>	/* for 'private_area_t' */
#elif (defined __WINDOWS__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <wanpipe_common.h>
# include <wanpipe_events.h>
# include <wanpipe.h>
# include <sdla_serial.h>
# include <wanpipe_events.h>
# include <if_wanpipe_common.h>	/* for 'wanpipe_common_t' used in 'sdla_aft_te1.h'*/
# include <sdla_aft_te1.h>	/* for 'private_area_t' */
#else
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanpipe_common.h>
# include <linux/wanpipe_events.h>
# include <linux/wanpipe.h>
# include <linux/sdla_serial.h>
# include <linux/wanpipe_events.h>
# include <linux/if_wanpipe_common.h>	/* for 'wanpipe_common_t' used in 'sdla_aft_te1.h'*/
# include <linux/sdla_aft_te1.h>	/* for 'private_area_t' */
#endif

#undef	DEBUG_SERIAL
#define DEBUG_SERIAL		if(0)DEBUG_EVENT

/* DEBUG macro definitions */
#define DEBUG_HFC_INIT		if(0)DEBUG_EVENT
#define DEBUG_HFC_MODE		if(0)DEBUG_EVENT
#define DEBUG_HFC_S0_STATES	if(0)DEBUG_EVENT
#define DEBUG_HFC_IRQ		if(0)DEBUG_EVENT
#define DEBUG_HFC_SU_IRQ	if(0)DEBUG_EVENT

#define DEBUG_HFC_TX		if(0)DEBUG_EVENT
#define DEBUG_TX_DATA		if(0)DEBUG_EVENT
#define TX_EXTRA_DBG		if(0)DEBUG_EVENT
#define TX_FAST_DBG		if(0)DEBUG_EVENT

#define DEBUG_HFC_RX		if(0)DEBUG_EVENT
#define RX_EXTRA_DBG		if(0)DEBUG_EVENT
#define DEBUG_RX1		if(0)DEBUG_EVENT
#define DBG_RX_DATA		if(0)DEBUG_EVENT

#define DEBUG_HFC_CLOCK		if(0)DEBUG_EVENT

#define BUILD_MOD_TESTER	0	/* for Production Test */
#define DBG_MODULE_TESTER	if(0)DEBUG_EVENT

#define NT_STATE_FUNC()		if(0)DEBUG_EVENT("%s(): line: %d\n", __FUNCTION__, __LINE__)
#define CLOCK_FUNC()		if(0)DEBUG_EVENT("%s(): line: %d\n", __FUNCTION__, __LINE__)

#define DBG_SPI			if(0)DEBUG_EVENT

#define DEBUG_FE_STATUS		if(0)DEBUG_EVENT

#define SERIAL_FUNC()		if(0)DEBUG_EVENT("%s(): line: %d\n", __FUNCTION__, __LINE__)

/* Timer interrupt counter - used by activation timer T3 */
#define HFC_TIMER_COUNTER_T3	2

#undef NT_T1_COUNT

#define FIFO_THRESHOLD_INDEX	1

#define LINE_STABILITY_THRESHOLD 3

enum {
	WAITING_TO_STABILIZE=1,
	LINE_STABLE,
	LINE_DISCONNECTED
};

#define CHECK_DATA 0


int aft_serial_write_cpld(void *card, unsigned short off,u_int16_t data);
unsigned char aft_serial_read_cpld(void *card, unsigned short cpld_off);

/*******************************************************************************
**			  DEFINES AND MACROS
*******************************************************************************/
#define REPORT_MOD_NO	(mod_no+1)

static u8 validate_fe_line_no(sdla_fe_t *fe, const char *caller_name)
{
	if ((int32_t)WAN_FE_LINENO(fe) < 0 || WAN_FE_LINENO(fe) > MAX_SERIAL_LINES){
		DEBUG_EVENT("%s(): %s: SERIAL: Invalid FE line number %d (Min=1 Max=%d)\n",
			caller_name, fe->name, WAN_FE_LINENO(fe)+1, MAX_SERIAL_LINES);
		return 1;
	}
	return 0;
}

#define FE_LINENO_TO_MODULENO_AND_PORTNO(fe)	\
{						\
	mod_no = fe_line_no_to_physical_mod_no(fe);	\
	port = fe_line_no_to_port_no(fe);	\
}

#if 0
/* wait for "milliseconds * 1/1000" of sec */
static void	WP_MDELAY(u32 milliseconds)
{
	for (; milliseconds > 0; --milliseconds){
		WP_DELAY(1000);
 	}
}
#endif

/*******************************************************************************/
/* Register Write/Read debugging funcitons */

/* Enabling/Disabling register debugging */
#define WAN_DEBUG_SERIAL_REG	0

#if WAN_DEBUG_SERIAL_REG

#define HFC_WRITE 1
#define HFC_READ  2

#endif /* WAN_DEBUG_SERIAL_REG */
/*******************************************************************************/

/*******************************************************************************/

/*******************************************************************************
**			STRUCTURES AND TYPEDEFS
*******************************************************************************/

/*******************************************************************************
**			   GLOBAL VARIABLES
*******************************************************************************/
#if !defined(__WINDOWS__)
extern WAN_LIST_HEAD(, wan_tdmv_) wan_tdmv_head;
#endif

/*******************************************************************************
**			  FUNCTION PROTOTYPES
*******************************************************************************/
static int32_t serial_global_config(void* pfe);
static int32_t serial_global_unconfig(void* pfe);
static int32_t wp_serial_config(void *pfe);
static int32_t wp_serial_unconfig(void *pfe);
static int32_t wp_serial_post_init(void *pfe);
static int32_t wp_serial_if_config(void *pfe, u32 mod_map, u8);
static int32_t wp_serial_if_unconfig(void *pfe, u32 mod_map, u8);
//static int32_t wp_serial_disable_irq(sdla_fe_t *fe, u32 mod_no, u8 port);
//static void serial_enable_interrupts(sdla_fe_t *fe, u32 mod_no, u8 port);
static int32_t wp_serial_intr(sdla_fe_t *); 
static int32_t wp_serial_check_intr(sdla_fe_t *); 
static int32_t wp_serial_polling(sdla_fe_t*);
static int32_t wp_serial_udp(sdla_fe_t*, void*, u8*);
static u32 wp_serial_active_map(sdla_fe_t* fe, u8 line_no);
static u8 wp_serial_fe_media(sdla_fe_t *fe);
static int32_t wp_serial_set_dtmf(sdla_fe_t*, int32_t, u8);
static int wp_serial_intr_ctrl(sdla_fe_t *fe, int mod_no, u_int8_t, u_int8_t mode, unsigned int ts_map);
static int32_t wp_serial_event_ctrl(sdla_fe_t*, wan_event_ctrl_t*);

static int wp_serial_get_fe_status(sdla_fe_t *fe, unsigned char *status);
static int wp_serial_set_fe_status(sdla_fe_t *fe, unsigned char status);

#if 0
static void wp_serial_enable_timer(sdla_fe_t* fe, u8 mod_no, u8 cmd, u32 delay);

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
static void wp_serial_timer(void*);
#elif defined(__WINDOWS__)
static void wp_serial_timer(IN PKDPC,void*,void*,void*);
#else
static void wp_serial_timer(void*);
#endif
#endif /* if 0 */

#if defined(AFT_TDM_API_SUPPORT)
static int32_t wp_serial_watchdog(sdla_fe_t *fe);
#endif


/* for selecting PCM direction */
#define XHFC_DIRECTION_TX 0
#define XHFC_DIRECTION_RX 1

#if 0
static void xhfc_select_xhfc_channel(sdla_fe_t *fe, u32 mod_no,
				  u8 channel, u8 direction, u8 vrout_bitmap);
#endif

//static int32_t check_f0cl_increment(sdla_fe_t *fe, u8 old_f0cl, u8 new_f0cl, int32_t *diff);

/*******************************************************************************
**			  FUNCTION DEFINITIONS
*******************************************************************************/


/*******************************************************************************/




/******************************************************************************
** wp_serial_iface_init) - 
**
**	OK
*/
int32_t wp_serial_iface_init(void *pfe_iface)
{
	sdla_fe_iface_t	*fe_iface = (sdla_fe_iface_t*)pfe_iface;

	SERIAL_FUNC();
	fe_iface->global_config		= &serial_global_config;	/* not used in remora */
	fe_iface->global_unconfig	= &serial_global_unconfig;	/* not used in remora */

	fe_iface->config		= &wp_serial_config;
	fe_iface->unconfig		= &wp_serial_unconfig;

	fe_iface->post_init		= &wp_serial_post_init;

	fe_iface->if_config		= &wp_serial_if_config;
	fe_iface->if_unconfig		= &wp_serial_if_unconfig;

	fe_iface->active_map		= &wp_serial_active_map;

	fe_iface->set_fe_status		= &wp_serial_set_fe_status;
	fe_iface->get_fe_status		= &wp_serial_get_fe_status;

	fe_iface->isr			= &wp_serial_intr;
//	fe_iface->disable_irq		= &wp_serial_disable_irq;
	fe_iface->check_isr		= &wp_serial_check_intr;

	fe_iface->polling		= &wp_serial_polling;
	fe_iface->process_udp		= &wp_serial_udp;
	fe_iface->get_fe_media		= &wp_serial_fe_media;

	fe_iface->set_dtmf		= &wp_serial_set_dtmf;
	fe_iface->intr_ctrl		= &wp_serial_intr_ctrl;
	fe_iface->event_ctrl		= &wp_serial_event_ctrl;


#if defined(AFT_TDM_API_SUPPORT)
	fe_iface->watchdog	= &wp_serial_watchdog;
#endif

	return 0;
}

#if defined(NOTUSED)
/* Should be done only ONCE per card. */
static int32_t wp_serial_spi_bus_reset(sdla_fe_t	*fe)
{
	sdla_t	*card = (sdla_t*)fe->card;

	SERIAL_FUNC();

	DEBUG_EVENT("%s: Executing SPI bus reset....\n", fe->name);

	card->hw_iface.bus_write_4(	card->hw,
					SPI_INTERFACE_REG,
					MOD_SPI_RESET);
	WP_DELAY(1000);
	card->hw_iface.bus_write_4(	card->hw,
					SPI_INTERFACE_REG,
					0x00000000);
	WP_DELAY(1000);
	return 0;
}
#endif

/******************************************************************************
*				serial_global_config()	
*
* Description	: Global configuration for Sangoma SERIAL board.
*
* 		Notes	: 1. This routine runs only ONCE for a physical 'base' CARD,
*					 not for 'additional' cards. 
*				  2. reset card's SPI.
*				  3. Scan for installed Remoras and modules.
*
* Arguments	: pfe - pointer to Front End structure.	
*
* Returns	: 0 - configred successfully, otherwise non-zero value.
*******************************************************************************/
static int32_t serial_global_config(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;

	SERIAL_FUNC();

	DEBUG_EVENT("%s: %s Global Front End configuration\n", 
			fe->name, FE_MEDIA_DECODE(fe));

	return 0;
}

/*******************************************************************************
*				serial_global_unconfig()	
*
* Description	: Global un-configuration for Sangoma SERIAL board.
* 			Note: This routne runs only ONCE for a physical card.
*
* Arguments	:	
*
* Returns	: 0
*******************************************************************************/
static int32_t serial_global_unconfig(void* pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_t 	*card = (sdla_t*)fe->card;

	SERIAL_FUNC();
        aft_serial_write_cpld(card,0x08,0x00);
        aft_serial_write_cpld(card,0x09,0x00);

	DEBUG_EVENT("%s: %s Global Front End unconfiguration!\n",
				fe->name, FE_MEDIA_DECODE(fe));

	return 0;
}

/******************************************************************************
*			wp_serial_config() - initialise the XHFC ISDN Chip.
*
* Description	:  Configure the PHYSICAL module ONE time. 
*		   On each module 2 lines will be configured 
*		   in exactly the same way.
*
* Arguments	:  pfe - pointer to Front End structure.
*
* Returns	:  0 - configred successfully, otherwise non-zero value.
*******************************************************************************/
static int32_t wp_serial_config(void *pfe)
{
	sdla_fe_t		*fe = (sdla_fe_t*)pfe;
	sdla_t 			*card = (sdla_t*)fe->card;
	u32 reg;
	u8 cpld_reg_val;
	unsigned short  cpld_reg=0;

	SERIAL_FUNC();

	DEBUG_EVENT("%s: %s: Line %d Front End configuration\n", 
			fe->name, FE_MEDIA_DECODE(fe), WAN_FE_LINENO(fe) + 1);

	if(validate_fe_line_no(fe, __FUNCTION__)){
		return 1;
	}

	
	switch (card->wandev.line_coding){ 
	case WANOPT_NRZ:
	case WANOPT_NRZI:
		break;		
	default:
		DEBUG_EVENT("%s: A140: Error: Unsupported line coding mode 0x%X\n",
			card->devname,
			card->wandev.line_coding);
		return -1;
	}

	switch (card->adptr_type) {

	case AFT_ADPTR_2SERIAL_V35X21:
        case AFT_ADPTR_4SERIAL_V35X21:
                DEBUG_EVENT("%s: A140: Configuring for V35\n",
                        card->devname);

                switch(WAN_FE_LINENO(fe)) {

                case 0:
                case 2:
                        cpld_reg=0x08;
                        break;
                case 1:
                case 3:
                        cpld_reg=0x09;
                        break;
                default:
                        DEBUG_EVENT("%s: Error: Invalid Serial Port Number! (%i) \n",
                                card->devname,WAN_FE_LINENO(fe));
                        return -EINVAL;
                };

                cpld_reg_val=aft_serial_read_cpld(card,cpld_reg);

                if(card->wandev.clocking) {

/*FIXME: Must check for case where first port started in external mode
         At this time, if port 1 start in normal & prot 3 in master, the
         port 1 will silently be reconfigured to Master after port 3 starts */

                        aft_serial_write_cpld(card,cpld_reg,0x05);
                } else {
   			if (wan_test_bit(2,&cpld_reg_val)) {
                                DEBUG_EVENT("%s: Error: Clocking configuration mismatch!\n",
                                                card->devname);
                                DEBUG_EVENT("%s:        Ports 1&3 and 2&4 must use same clock source!\n",
                                                card->devname);
                                return -EINVAL;
                        }

                        aft_serial_write_cpld(card,cpld_reg,0x01);
                }
                break;

        case AFT_ADPTR_2SERIAL_RS232:
        case AFT_ADPTR_4SERIAL_RS232:

                DEBUG_EVENT("%s: A140: Configuring for RS232\n",
                        card->devname);

                if (WAN_FE_LINENO(fe) < 2) {
                        cpld_reg=0x08;
                } else {
                        cpld_reg=0x09;
                }

                cpld_reg_val=aft_serial_read_cpld(card,cpld_reg);
                if(card->wandev.clocking) {
                        wan_set_bit((WAN_FE_LINENO(fe)%2), &cpld_reg_val);
                } else {
                        wan_clear_bit((WAN_FE_LINENO(fe)%2), &cpld_reg_val);
                }
                aft_serial_write_cpld(card,cpld_reg,cpld_reg_val);

                break;

        default:
                DEBUG_EVENT("%s: Error: Invalid Serial Card Type 0x%X\n",
                        card->devname,card->adptr_type);
                return -1;
        }

        card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_SERIAL_LINE_CFG_REG),&reg);
	
	DEBUG_EVENT("%s: A140: Configurfed for 0x%08X\n",
			card->devname,
			reg);


	if (card->wandev.clocking) {
		DEBUG_EVENT("%s: A140: Configuring for Internal Clocking: Baud=%i\n",
			card->devname,
			card->wandev.bps);
		wan_set_bit(AFT_SERIAL_LCFG_CLK_SRC_BIT, &reg);
		aft_serial_set_baud_rate(&reg,card->wandev.bps);
	} else {
		DEBUG_EVENT("%s: A140: Configuring for External Clocking: Baud=%i\n",
			card->devname,
			card->wandev.bps);
		wan_clear_bit(AFT_SERIAL_LCFG_CLK_SRC_BIT, &reg);
	}

	switch (card->wandev.line_coding){ 
	case WANOPT_NRZ:
		DEBUG_EVENT("%s: A140: Configuring for NRZ\n",
			card->devname);
		aft_serial_set_lcoding(&reg,WANOPT_NRZ);
		break;
	case WANOPT_NRZI:
		DEBUG_EVENT("%s: A140: Configuring for NRZI\n",
			card->devname);
		aft_serial_set_lcoding(&reg,WANOPT_NRZI);
		break;		
	default:
		/* Should never happen because we check above */
		DEBUG_EVENT("%s: A140: Error: Unsupported line coding mode 0x%X\n",
			card->devname,
			card->wandev.line_coding);
		return -1;
	}

	if (card->wandev.connection == WANOPT_SWITCHED){
		DEBUG_EVENT("%s: A140: Configuring for Switched CTS/RTS\n",card->devname);
		wan_set_bit(AFT_SERIAL_LCFG_SWMODE_BIT, &reg);
		wan_set_bit(AFT_SERIAL_LCFG_IDLE_DET_BIT,&reg);
	} else {
		wan_clear_bit(AFT_SERIAL_LCFG_SWMODE_BIT, &reg);
		wan_clear_bit(AFT_SERIAL_LCFG_IDLE_DET_BIT,&reg);
	}


	/* Hardcode to sync device type */
	wan_clear_bit(AFT_SERIAL_LCFG_IFACE_TYPE_BIT,&reg);

	/* CTS/DCD Interrupt Enable */
	if (card->wandev.ignore_front_end_status == WANOPT_YES) {
		/* If ignore front end is set, do not enable CTS/RTS interrupts */
		wan_clear_bit(AFT_SERIAL_LCFG_CTS_INTR_EN_BIT,&reg);
		wan_clear_bit(AFT_SERIAL_LCFG_DCD_INTR_EN_BIT,&reg);
	} else {
		wan_set_bit(AFT_SERIAL_LCFG_CTS_INTR_EN_BIT,&reg);
		wan_set_bit(AFT_SERIAL_LCFG_DCD_INTR_EN_BIT,&reg);
	}

	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_SERIAL_LINE_CFG_REG),reg);

	DEBUG_EVENT("%s: A140: Configurfed for 0x%08X CTS/DCD ISR=%s\n",
			card->devname,
			reg,card->wandev.ignore_front_end_status == WANOPT_YES?"Off":"On");

	/* Raise RTS and DTR */	
	wan_set_bit(AFT_SERIAL_LCFG_RTS_BIT,&reg);
	wan_set_bit(AFT_SERIAL_LCFG_DTR_BIT,&reg);
	
	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_SERIAL_LINE_CFG_REG),reg);

	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_SERIAL_LINE_CFG_REG),&reg);
	
	DEBUG_EVENT("%s: A140: Configurfed for 0x%08X\n",
			card->devname,
			reg);

	return 0;
}

/******************************************************************************
** wp_serial_unconfig() - 
**
**	OK
*/
static int32_t wp_serial_unconfig(void *pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_t 		*card = (sdla_t*)fe->card;
	u32 reg=0;
	unsigned short cpld_reg;
	unsigned char cpld_reg_val;

	SERIAL_FUNC();

	if(validate_fe_line_no(fe, __FUNCTION__)){
		return 1;
	}

	switch (card->adptr_type) {
	case AFT_ADPTR_2SERIAL_RS232:
	case AFT_ADPTR_4SERIAL_RS232:

		if (WAN_FE_LINENO(fe) < 2) {
			cpld_reg=0x08;
		} else {
			cpld_reg=0x09;
		}

		cpld_reg_val=aft_serial_read_cpld(card,cpld_reg);
		wan_clear_bit((WAN_FE_LINENO(fe)%2), &cpld_reg_val);
		aft_serial_write_cpld(card,cpld_reg,cpld_reg_val);
		break;
	}

	card->hw_iface.bus_write_4(card->hw,AFT_PORT_REG(card,AFT_SERIAL_LINE_CFG_REG),reg);

	return 0;
}

/******************************************************************************
** wp_serial_post_init() - 
**
**	OK
*/
static int32_t wp_serial_post_init(void *pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	
	SERIAL_FUNC();

	DEBUG_EVENT("%s: Running post initialization...\n", fe->name);
	/* return sdla_rm_add_timer(fe, HZ); */
	return 0;
}


/******************************************************************************
** wp_serial_if_config() - 
**
**	OK
*/
static int32_t wp_serial_if_config(void *pfe, u32 mod_map, u8 usedby)
{
	SERIAL_FUNC();
	return 0;
}


/******************************************************************************
** wp_serial_if_unconfig() - 
**
**	OK
*/
static int32_t wp_serial_if_unconfig(void *pfe, u32 mod_map, u8 usedby)
{
	SERIAL_FUNC();
	return 0;
}



#if defined(NOTUSED)
/*******************************************************************************
*			serial_enable_interrupts()	
*
* Description: Enable SERIAL interrupts - start interrupt and set interrupt mask.
*
* Arguments:
*
* Returns:
*******************************************************************************/
static void serial_enable_interrupts(sdla_fe_t *fe, u32 mod_no, u8 port)
{
	

	SERIAL_FUNC();

}
#endif

#if defined(NOTUSED)
/******************************************************************************
** wp_serial_disable_irq() - disable all interrupts by disabling M_GLOB_IRQ_EN
**
**	OK
*/
static int32_t wp_serial_disable_irq(sdla_fe_t *fe, u32 mod_no, u8 port)
{
	
	return 0;
}
#endif

static u32 wp_serial_active_map(sdla_fe_t* fe, u8 line_no)
{
	SERIAL_FUNC();

	return 0x01;
}

/******************************************************************************
*				wp_serial_fe_status()	
*
* Description:
* Arguments:	
* Returns:
*******************************************************************************/
static u8 wp_serial_fe_media(sdla_fe_t *fe)
{
	SERIAL_FUNC();
	return fe->fe_cfg.media;
}

/******************************************************************************
*				wp_serial_set_dtmf()	
*
* Description:
* Arguments:	
* Returns:
*******************************************************************************/
static int32_t wp_serial_set_dtmf(sdla_fe_t *fe, int32_t mod_no, u8 val)
{
	SERIAL_FUNC();

	return -EINVAL;
}

#if 0

/*******************************************************************************
*				sdla_serial_timer()	
*
* Description:
* Arguments:
* Returns:
*******************************************************************************/
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static void wp_serial_timer(void* pfe)
#elif defined(__WINDOWS__)
static void wp_serial_timer(IN PKDPC Dpc, void* pfe, void* arg2, void* arg3)
#else
static void wp_serial_timer(void *pfe)
#endif
{
	SERIAL_FUNC();
	return;
}

/*******************************************************************************
*				wp_serial_enable_timer()	
*
* Description: Enable software timer interrupt in delay ms.
* Arguments:
* Returns:
*******************************************************************************/
static void wp_serial_enable_timer(sdla_fe_t* fe, u8 mod_no, u8 cmd, u32 delay)
{
	SERIAL_FUNC();
	return;	
}

static int32_t wp_serial_regdump(sdla_fe_t* fe, u8 *data)
{
	SERIAL_FUNC();
	return 0;
}

#endif /* if 0*/

static int32_t wp_serial_polling(sdla_fe_t* fe)
{
	SERIAL_FUNC();
	return 0;
}

/*******************************************************************************
*				wp_serial_udp()	
*
* Description:
* Arguments:
* Returns:
*******************************************************************************/
static int32_t wp_serial_udp(sdla_fe_t *fe, void* p_udp_cmd, u8* data)
{
	int32_t		err = -EINVAL;

	SERIAL_FUNC();
#if 0
	wan_cmd_t	*udp_cmd = (wan_cmd_t*)p_udp_cmd;

	switch(udp_cmd->wan_cmd_command){
	case WAN_FE_REGDUMP:
		err = wp_serial_regdump(fe, data);
		if (err){
			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			udp_cmd->wan_cmd_data_len = (u16)err; 
		}
		break;

	default:
		udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
	    	udp_cmd->wan_cmd_data_len = 0;
		break;
	}
#endif
	return err;
}

/******************************************************************************
*				wp_serial_get_fe_status()	
*
* Description	: Get current FE line state - is it Connected or Disconnected
*
* Arguments	: fe - pointer to Front End structure.	
*		  status - pointer to location where the FE line state will
*			be stored.
*
* Returns	: always zero.
*******************************************************************************/
static int wp_serial_get_fe_status(sdla_fe_t *fe, unsigned char *status)
{
	*status = fe->fe_status;
	return 0;
}

/******************************************************************************
*				wp_serial_set_fe_status()	
*
* Description	: Set FE line state to Connected or Disconnected.
*		  In SERIAL this means Activate or Deactivate the line.
*
* Arguments	: fe - pointer to Front End structure.	
*		  new_status - the new FE line state.
*
* Returns	: 0 - success.
*		  1 - failure.
*******************************************************************************/
static int wp_serial_set_fe_status(sdla_fe_t *fe, unsigned char new_status)
{
	int rc=0;
	SERIAL_FUNC();
	return rc;
}



/******************************************************************************
*				wp_serial_event_ctrl()	
*
* Description: Enable/Disable event types
* Arguments: mod_no -  Module number (1,2,3,... MAX_REMORA_MODULES)
* Returns:
******************************************************************************/
static int32_t
wp_serial_event_ctrl(sdla_fe_t *fe, wan_event_ctrl_t *ectrl)
{
	int32_t err = 0;

	SERIAL_FUNC();

	WAN_ASSERT(ectrl == NULL);

	return err;
}

#if defined(AFT_TDM_API_SUPPORT)
/******************************************************************************
*			wp_serial_watchdog()	
*
* Description:
* Arguments: mod_no -  Module number (1,2,3,... MAX_REMORA_MODULES)
* Returns:
******************************************************************************/
static int32_t wp_serial_watchdog(sdla_fe_t *fe)
{
	int32_t	mod_no;

	SERIAL_FUNC();
	
	return 0;
}
#endif

/******************************************************************************
*				wp_serial_intr_ctrl()	
*
* Description: Enable/Disable extra interrupt types
* Arguments: mod_no -  Module number (1,2,3,... MAX_REMORA_MODULES)
* Returns:
******************************************************************************/
static int wp_serial_intr_ctrl(sdla_fe_t *fe, int mod_no, u_int8_t type, u_int8_t mode, unsigned int ts_map)
{
	
	SERIAL_FUNC();

	return 0;

}

#if defined(NOTUSED)
/******************************************************************************
*				sdla_serial_set_status()	
*
* Description: handle interrupt on a physical module
* Arguments: fe, mod_no
* Returns:	1 - interrupt recognized and handled
*		0 - interrupt not recognized (not generated by this module)
******************************************************************************/
static void sdla_serial_set_status(sdla_fe_t* fe, u8 mod_no, u8 port_no, u8 status)
{
	sdla_t			*card = (sdla_t*)fe->card;
	u8			old_fe_status = fe->fe_status;


	SERIAL_FUNC();


#if 0
	if(old_fe_status == status){
		return;
	}
#endif

	fe->fe_status = status;
	
	if (old_fe_status != fe->fe_status){
		if (fe->fe_status == FE_CONNECTED){
			DEBUG_EVENT("%s: %s Module: %d connected!\n", 
					fe->name,
					FE_MEDIA_DECODE(fe), REPORT_MOD_NO + port_no);

			if (card->wandev.te_report_alarms){
				card->wandev.te_report_alarms(card, 0);
			}

		}else{
			DEBUG_EVENT("%s: %s Module: %d disconnected!\n", 
					fe->name,
					FE_MEDIA_DECODE(fe), REPORT_MOD_NO + port_no);
			if (card->wandev.te_report_alarms){
				card->wandev.te_report_alarms(card, 1);
			} 
		}
	}

	return;
}
#endif


static int32_t wp_serial_check_intr(sdla_fe_t *fe)
{
	/* must return 1! */
	return 1;
} 

static int32_t wp_serial_intr(sdla_fe_t *fe)
{
	sdla_t 		*card = (sdla_t*)fe->card;
	u32 			reg;

	SERIAL_FUNC();
	card->hw_iface.bus_read_4(card->hw,AFT_PORT_REG(card,AFT_SERIAL_LINE_CFG_REG),&reg);
	DEBUG_EVENT("%s: DCD/CTS VALUES = 0x%02X\n",card->devname,(reg>>2)&0x03);
	
	return 0;
}

/***************************************************************************
	Front End T1/E1 interface for Normal cards
***************************************************************************/
int aft_serial_write_fe(void* phw, ...)
{
	va_list		args;
	u16		qaccess, off, line_no;
	u8		value;
//	u8	qaccess = card->wandev.state == WAN_CONNECTED ? 1 : 0;
	       	 
	va_start(args, phw);
	qaccess	= (u16)va_arg(args, int);
	line_no = (u16)va_arg(args, int);
	off	= (u16)va_arg(args, int);
	value	= (u8)va_arg(args, int);
	va_end(args);

	SERIAL_FUNC();

        return 0;
}


/*============================================================================
 * Read TE1/56K Front end registers
 */


u32 aft_serial_read_fe (void* phw, ...)
{
	va_list		args;
	u_int16_t	qaccess, line_no, off;
	u_int8_t	tmp=0;
//	u8	qaccess = card->wandev.state == WAN_CONNECTED ? 1 : 0;

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int8_t)va_arg(args, int);
	va_end(args);

	SERIAL_FUNC();

        return tmp;
}

int aft_serial_write_cpld(void *pcard, unsigned short off,u_int16_t data)
{
	int	err = -EINVAL;
	sdla_t *card = (sdla_t*)pcard;

	if (card->hw_iface.fe_test_and_set_bit(card->hw,0)){
		DEBUG_EVENT("%s: %s:%d: Critical Error: Re-entry in FE!\n",
			card->devname, __FUNCTION__,__LINE__);
		return 0x00;
	}

	if (card->hw_iface.write_cpld){
		err = card->hw_iface.write_cpld(card->hw, (u16)off, (u8)data);
	}

	card->hw_iface.fe_clear_bit(card->hw,0);
        return 0;
}

unsigned char aft_serial_read_cpld(void *pcard, unsigned short cpld_off)
{
        u8      tmp=0;
	int	err = -EINVAL;
	sdla_t *card = (sdla_t*)pcard;

	if (card->hw_iface.fe_test_and_set_bit(card->hw,0)){
		DEBUG_EVENT("%s: %s:%d: Critical Error: Re-entry in FE!\n",
			card->devname, __FUNCTION__,__LINE__);
		return 0x00;
	}

	if (card->hw_iface.read_cpld){
		err = card->hw_iface.read_cpld(card->hw, (u16)cpld_off, &tmp);
	}

	card->hw_iface.fe_clear_bit(card->hw,0);
        return tmp;
}
