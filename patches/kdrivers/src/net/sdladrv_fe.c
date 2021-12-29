/*****************************************************************************
* sdladrv_fe.c	SDLA FE interface support Module.
*
*
* Author:	Alex Feldman
*
* Copyright:	(c) 2006 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Aug 10, 2006	Alex Feldman	Initial version
*****************************************************************************/

/*****************************************************************************
 * Notes:
 * ------
 ****************************************************************************/


#define __SDLA_HW_LEVEL
#define __SDLADRV__

#define SDLADRV_NEW

/***************************************************************************
****		I N C L U D E  		F I L E S			****
***************************************************************************/
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe_version.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <wanpipe_common.h>
# include <wanpipe.h>
# include <sdlasfm.h>
# include <sdlapci.h>
# include <sdladrv.h>
#elif defined(__LINUX__)||defined(__KERNEL__)
# define _K22X_MODULE_FIX_
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_version.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanpipe_common.h>
# include <linux/sdlasfm.h>	/* SDLA firmware module definitions */
# include <linux/sdlapci.h>	/* SDLA PCI hardware definitions */
# include <linux/wanpipe.h>
# include <linux/sdladrv.h>	/* API definitions */
#else
# error "Unsupported Operating System!"
#endif

/***************************************************************************
****                     M A C R O S / D E F I N E S                    ****
***************************************************************************/
#define BIT_DEV_ADDR_CLEAR      0x600
#define BIT_DEV_ADDR_CPLD       0x200
#define XILINX_MCPU_INTERFACE           0x44
#define XILINX_MCPU_INTERFACE_ADDR      0x46

/***************************************************************************
****               F U N C T I O N   P R O T O T Y P E S                ****
***************************************************************************/
int		sdla_te1_write_fe(void* phw, ...);
u_int8_t	sdla_te1_read_fe (void* phw, ...);

static int	__sdla_shark_te1_write_fe(void *phw, ...);
int		sdla_shark_te1_write_fe(void *phw, ...);
static u_int8_t	__sdla_shark_te1_read_fe (void *phw, ...);
u_int8_t	sdla_shark_te1_read_fe (void *phw, ...);

static int	__sdla_shark_rm_write_fe (void* phw, ...);
int		sdla_shark_rm_write_fe (void* phw, ...);
static u_int8_t	__sdla_shark_rm_read_fe (void* phw, ...);
u_int8_t	sdla_shark_rm_read_fe (void* phw, ...);

static int	__sdla_shark_56k_write_fe(void *phw, ...);
int		sdla_shark_56k_write_fe(void *phw, ...);
static u_int8_t	__sdla_shark_56k_read_fe (void *phw, ...);
u_int8_t	sdla_shark_56k_read_fe (void *phw, ...);

extern int sdla_bus_write_1(void* phw, unsigned int offset, u8 value);
extern int sdla_bus_write_2(void* phw, unsigned int offset, u16 value);
extern int sdla_bus_write_4(void* phw, unsigned int offset, u32 value);
extern int sdla_bus_read_1(void* phw, unsigned int offset, u8* value);
extern int sdla_bus_read_2(void* phw, unsigned int offset, u16* value);
extern int sdla_bus_read_4(void* phw, unsigned int offset, u32* value);

extern int sdla_hw_fe_test_and_set_bit(void *phw, int value);
extern int sdla_hw_fe_test_bit(void *phw, int value);
extern int sdla_hw_fe_clear_bit(void *phw, int value);
extern int sdla_hw_fe_set_bit(void *phw, int value);

/***************************************************************************
****                      G L O B A L  D A T A                          ****
***************************************************************************/

/***************************************************************************
****               F U N C T I O N   D E F I N I T I O N                ****
***************************************************************************/



/***************************************************************************
	Front End T1/E1 interface for Normal cards
***************************************************************************/
int sdla_te1_write_fe(void* phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
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

	off &= ~BIT_DEV_ADDR_CLEAR;
       	sdla_bus_write_2(hw, XILINX_MCPU_INTERFACE_ADDR, off);
	/* AF: Sep 10, 2003
	 * IMPORTANT
	 * This delays are required to avoid bridge optimization 
	 * (combining two writes together)
	 */
	if (!qaccess){
		WP_DELAY(5);
	}
        sdla_bus_write_1(hw, XILINX_MCPU_INTERFACE, value);
	if (!qaccess){
		WP_DELAY(5);
	}
        return 0;
}


/*============================================================================
 * Read TE1/56K Front end registers
 */
u_int8_t sdla_te1_read_fe (void* phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	u_int16_t	qaccess, line_no, off;
	u_int8_t	tmp;
//	u8	qaccess = card->wandev.state == WAN_CONNECTED ? 1 : 0;

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int8_t)va_arg(args, int);
	va_end(args);

        off &= ~BIT_DEV_ADDR_CLEAR;
        sdla_bus_write_2(hw, XILINX_MCPU_INTERFACE_ADDR, off);
        sdla_bus_read_1(hw,XILINX_MCPU_INTERFACE, &tmp);
	
	if (!qaccess){
		WP_DELAY(5);
	}
        return tmp;
}

/***************************************************************************
	Front End T1/E1 interface for Shark subtype cards
***************************************************************************/
static int __sdla_shark_te1_write_fe (void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	u_int16_t 	org_off, qaccess, line_no, off;
	u_int8_t	value;
//	u8		qaccess = card->wandev.state == WAN_CONNECTED ? 1 : 0;

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int16_t)va_arg(args, int);
	value	= (u_int8_t)va_arg(args, int);
	va_end(args);

	if (hw->hwcard->core_id == AFT_PMC_FE_CORE_ID){
       	off &= ~AFT4_BIT_DEV_ADDR_CLEAR;	
	}else if (hw->hwcard->core_id == AFT_DS_FE_CORE_ID){
		if (off & 0x800)  off |= 0x2000;
		if (off & 0x1000) off |= 0x4000;
		off &= ~AFT8_BIT_DEV_ADDR_CLEAR;
		if (hw->hwcard->adptr_type == A101_ADPTR_2TE1 && line_no == 1){
			off |= AFT8_BIT_DEV_MAXIM_ADDR_CPLD;
		}
	}

	sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, (u16*)&org_off);
	
       	sdla_bus_write_2(hw,AFT_MCPU_INTERFACE_ADDR, (u16)off);

	/* AF: Sep 10, 2003
	 * IMPORTANT
	 * This delays are required to avoid bridge optimization 
	 * (combining two writes together)
	 */
	if (!qaccess){
		WP_DELAY(5);
	}

	sdla_bus_write_1(hw, AFT_MCPU_INTERFACE, (u8)value);
	if (!qaccess){
		WP_DELAY(5);
	}
	
	sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);	

	if (!qaccess){
		WP_DELAY(5);
	}
        return 0;
}

int sdla_shark_te1_write_fe (void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	u_int16_t	qaccess, line_no, off;
	u_int8_t	value;

	if (sdla_hw_fe_test_and_set_bit(hw,0)){
		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT(
			"%s: %s:%d: Critical Error: Re-entry in FE!\n",
					hw->devname,
					__FUNCTION__,__LINE__);
		}
		return -EINVAL;
	}

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int16_t)va_arg(args, int);
	value	= (u_int8_t)va_arg(args, int);
	va_end(args);

	__sdla_shark_te1_write_fe(hw, qaccess, line_no, off, value);

	sdla_hw_fe_clear_bit(hw,0);
        return 0;
}

/*============================================================================
 * Read TE1 Front end registers
 */
static u_int8_t __sdla_shark_te1_read_fe (void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	u_int16_t     	org_off, qaccess, line_no, off;
	u_int8_t	tmp;
//	u8		qaccess = card->wandev.state == WAN_CONNECTED ? 1 : 0;

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int8_t)va_arg(args, int);
	va_end(args);

	if (hw->hwcard->core_id == AFT_PMC_FE_CORE_ID){
       		off &= ~AFT4_BIT_DEV_ADDR_CLEAR;	
	}else if (hw->hwcard->core_id == AFT_DS_FE_CORE_ID){
		if (off & 0x800)  off |= 0x2000;
		if (off & 0x1000) off |= 0x4000;
		off &= ~AFT8_BIT_DEV_ADDR_CLEAR;	
		if (hw->hwcard->adptr_type == A101_ADPTR_2TE1 && line_no == 1){
			off |= AFT8_BIT_DEV_MAXIM_ADDR_CPLD;
		}
	}
	
	sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, (u16*)&org_off);
	
    sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, (u16)off);

   	sdla_bus_read_1(hw,AFT_MCPU_INTERFACE, (u8*)&tmp);
	if (!qaccess){
		WP_DELAY(5);
	}
	
	sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, (u16)org_off);	

	if (!qaccess){
		WP_DELAY(5);
	}
        return tmp;
}

u_int8_t sdla_shark_te1_read_fe (void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	u_int16_t	qaccess, line_no, off;
	u_int8_t	tmp;

	if (sdla_hw_fe_test_and_set_bit(hw,0)){
		if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s: %s:%d: Critical Error: Re-entry in FE!\n",
			hw->devname, __FUNCTION__,__LINE__);
		}
		return 0x00;
	}

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int16_t)va_arg(args, int);
	va_end(args);

	tmp = __sdla_shark_te1_read_fe(hw, qaccess, line_no, off);

	sdla_hw_fe_clear_bit(hw,0);
        return tmp;
}

/***************************************************************************
	56K Front End interface for Shark subtype cards
***************************************************************************/

/*============================================================================
 * Write 56k Front end registers
 */
static int __sdla_shark_56k_write_fe (void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	u_int16_t 	qaccess, line_no, off;
	u_int8_t	value;
//	u8		qaccess = card->wandev.state == WAN_CONNECTED ? 1 : 0;

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int16_t)va_arg(args, int);
	value	= (u_int8_t)va_arg(args, int);
	va_end(args);

	off &= ~AFT8_BIT_DEV_ADDR_CLEAR;	

   	sdla_bus_write_2(hw,0x46, (u16)off);

	sdla_bus_write_2(hw,0x44, (u16)value);

	if (!qaccess){
		WP_DELAY(5);
	}
   	
	return 0;
}

int sdla_shark_56k_write_fe (void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	u_int16_t	qaccess, line_no, off;
	u_int8_t	value;

	if (sdla_hw_fe_test_and_set_bit(hw,0)){
		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT(
			"%s: %s:%d: Critical Error: Re-entry in FE!\n",
					hw->devname,
					__FUNCTION__,__LINE__);
		}
		return -EINVAL;
	}

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int16_t)va_arg(args, int);
	value	= (u_int8_t)va_arg(args, int);
	va_end(args);

	__sdla_shark_56k_write_fe(hw, qaccess, line_no, off, value);

	sdla_hw_fe_clear_bit(hw,0);
     return 0;
}

/*============================================================================
 * Read 56k Front end registers
 */
static u_int8_t __sdla_shark_56k_read_fe (void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	u_int16_t  	qaccess, line_no, off;
	u_int32_t	tmp;
//	u8		qaccess = card->wandev.state == WAN_CONNECTED ? 1 : 0;

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int8_t)va_arg(args, int);
	va_end(args);

	off &= ~AFT8_BIT_DEV_ADDR_CLEAR;	

   	sdla_bus_write_2(hw,0x46, (u16)off);

   	sdla_bus_read_4(hw,0x44, &tmp);

	if (!qaccess){
		WP_DELAY(5);
	}

	return (u_int8_t)tmp;
}

u_int8_t sdla_shark_56k_read_fe (void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	u_int16_t	qaccess, line_no, off;
	u_int8_t	tmp;

	if (sdla_hw_fe_test_and_set_bit(hw,0)){
		if (WAN_NET_RATELIMIT()){
		DEBUG_EVENT("%s: %s:%d: Critical Error: Re-entry in FE!\n",
			hw->devname, __FUNCTION__,__LINE__);
		}
		return 0x00;
	}

	va_start(args, phw);
	qaccess = (u_int16_t)va_arg(args, int);
	line_no = (u_int16_t)va_arg(args, int);
	off	= (u_int16_t)va_arg(args, int);
	va_end(args);

	tmp = __sdla_shark_56k_read_fe(hw, qaccess, line_no, off);

	sdla_hw_fe_clear_bit(hw,0);
        return tmp;
}


/***************************************************************************
	Front End FXS/FXO interface for Shark subtype cards
***************************************************************************/
/*============================================================================
 * Read TE1/56K Front end registers
 */
static int __sdla_shark_rm_write_fe (void* phw, ...)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	va_list		args;
	int		mod_no, type, chain;
	int		reg, value;
	u32		data = 0;
	unsigned char	cs = 0x00, ctrl_byte = 0x00;
	int		i;

	va_start(args, phw);
	mod_no	= va_arg(args, int);
	type	= va_arg(args, int);
	chain	= va_arg(args, int);
	reg	= va_arg(args, int);
	value	= va_arg(args, int);
	va_end(args);
#if 0
	if (!wan_test_bit(mod_no, card->fe.fe_param.remora.module_map)){
		DEBUG_EVENT("%s: %s:%d: Internal Error: Module %d\n",
			card->devname, __FUNCTION__,__LINE__,mod_no);
		return -EINVAL;
	}
#endif
	DEBUG_RM("%s:%d: Module %d: Write RM FE code (reg %d, value %02X)!\n",
				__FUNCTION__,__LINE__,
				/*FIXME: hw->devname,*/mod_no, reg, (u8)value);
	
	/* bit 0-7: data byte */
	data = value & 0xFF;
	if (type == MOD_TYPE_FXO){

		/* bit 8-15: register number */
		data |= (reg & 0xFF) << 8;

		/* bit 16-23: chip select byte
		** bit 16
		**
		**
		**			*/
		cs = 0x20;
		cs |= MOD_SPI_CS_FXO_WRITE;
		if (mod_no % 2 == 0){
			/* Select second chip in a chain */
			cs |= MOD_SPI_CS_FXO_CHIP_1;
		}
		data |= (cs & 0xFF) << 16;

		/* bit 24-31: ctrl byte
		** bit 24
		**
		**
		**			*/
		ctrl_byte = mod_no / 2;
#if !defined(SPI2STEP)
		if (hw->hwcard->core_rev > 3){
			ctrl_byte |= MOD_SPI_CTRL_START;
		}else{
			ctrl_byte |= MOD_SPI_CTRL_V3_START;
		}
#endif
		ctrl_byte |= MOD_SPI_CTRL_CHAIN;	/* always chain */
		data |= ctrl_byte << 24;

	}else if (type == MOD_TYPE_FXS){

		/* bit 8-15: register byte */
		reg = reg & 0x7F;
		reg |= MOD_SPI_ADDR_FXS_WRITE; 
		data |= (reg & 0xFF) << 8;
		
		/* bit 16-23: chip select byte
		** bit 16
		**
		**
		**			*/
		if (mod_no % 2){
			/* Select first chip in a chain */
			cs = MOD_SPI_CS_FXS_CHIP_0;
		}else{
			/* Select second chip in a chain */
			cs = MOD_SPI_CS_FXS_CHIP_1;
		}
		data |= cs << 16;

		/* bit 24-31: ctrl byte
		** bit 24
		**
		**
		**			*/
		ctrl_byte = mod_no / 2;
#if !defined(SPI2STEP)
		if (hw->hwcard->core_rev > 3){
			ctrl_byte |= MOD_SPI_CTRL_START;
		}else{
			ctrl_byte |= MOD_SPI_CTRL_V3_START;
		}
#endif
		ctrl_byte |= MOD_SPI_CTRL_FXS;
		if (chain){
			ctrl_byte |= MOD_SPI_CTRL_CHAIN;
		}
		data |= ctrl_byte << 24;

	}else{
		DEBUG_EVENT("%s: Module %d: Unsupported module type %d!\n",
				hw->devname, mod_no, type);
		return -EINVAL;
	}

	sdla_bus_write_4(hw, SPI_INTERFACE_REG, data);	
#if defined(SPI2STEP)
	WP_DELAY(1);
	if (hw->hwcard->core_rev > 3){
		data |= MOD_SPI_START;
	}else{
		data |= MOD_SPI_V3_START;
	}
	sdla_bus_write_4(hw, SPI_INTERFACE_REG, data);	
#endif
#if 0
	DEBUG_EVENT("%s: %s: Module %d - Execute SPI command %08X\n",
					card->fe.name,
					__FUNCTION__,
					mod_no,
					data);
#endif

	for (i=0;i<10;i++){	
		WP_DELAY(10);
		sdla_bus_read_4(hw, SPI_INTERFACE_REG, &data);

		if (data & MOD_SPI_BUSY){
			continue;
		}
	}

	if (data & MOD_SPI_BUSY) {
		DEBUG_EVENT("%s: Module %d: Critical Error (%s:%d)!\n",
					hw->devname, mod_no,
					__FUNCTION__,__LINE__);
		return -EINVAL;
	}
        return 0;
}

int sdla_shark_rm_write_fe (void* phw, ...)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	va_list		args;
	int		mod_no, type, chain, reg, value;
#if defined(WAN_DEBUG_FE)
	char		*fname;	
	int		fline;
#endif

	va_start(args, phw);
	mod_no	= va_arg(args, int);
	type	= va_arg(args, int);
	chain	= va_arg(args, int);
	reg	= va_arg(args, int);
	value	= va_arg(args, int);
#if defined(WAN_DEBUG_FE)
	fname	= va_arg(args, char*);
	fline	= va_arg(args, int);
#endif
	va_end(args);

	if (sdla_hw_fe_test_and_set_bit(hw,0)){
#if defined(WAN_DEBUG_FE)
		DEBUG_EVENT("%s: %s:%d: Critical Error: Re-entry in FE (%s:%d)!\n",
			hw->devname, __FUNCTION__,__LINE__, fname, fline);
#else
		DEBUG_EVENT("%s: %s:%d: Critical Error: Re-entry in FE!\n",
			hw->devname, __FUNCTION__,__LINE__);
#endif			
		return -EINVAL;
	}

	__sdla_shark_rm_write_fe(hw, mod_no, type, chain, reg, value);

	sdla_hw_fe_clear_bit(hw,0);
        return 0;
}

static u_int8_t __sdla_shark_rm_read_fe (void* phw, ...)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	va_list		args;
	int		mod_no, type, chain, reg;
	u32		data = 0;
	unsigned char	cs = 0x00, ctrl_byte = 0x00;
	int		i;

	va_start(args, phw);
	mod_no	= va_arg(args, int);
	type	= va_arg(args, int);
	chain	= va_arg(args, int);
	reg	= va_arg(args, int);
	va_end(args);
#if 0
	if (!wan_test_bit(mod_no, card->fe.fe_param.remora.module_map)){
		DEBUG_EVENT("%s: %s:%d: Internal Error: Module %d\n",
			card->devname, __FUNCTION__,__LINE__,mod_no);
		return 0x00;
	}
#endif
	DEBUG_RM("%s:%d: Module %d: Read RM FE code (reg %d)!\n",
				__FUNCTION__,__LINE__,
				/*FIXME: hw->devname, */mod_no, reg);

	/* bit 0-7: data byte */
	data = 0x00;
	if (type == MOD_TYPE_FXO){

		/* bit 8-15: register byte */
		data |= (reg & 0xFF) << 8;

		/* bit 16-23: chip select byte
		** bit 16
		**
		**
		**			*/
		cs = 0x20;
		cs |= MOD_SPI_CS_FXO_READ;
		if (mod_no % 2 == 0){
			/* Select second chip in a chain */
			cs |= MOD_SPI_CS_FXO_CHIP_1;
		}
		data |= (cs & 0xFF) << 16;

		/* bit 24-31: ctrl byte
		** bit 24
		**
		**
		**			*/
		ctrl_byte = mod_no / 2;
#if !defined(SPI2STEP)
		if (hw->hwcard->core_rev > 3){
			ctrl_byte |= MOD_SPI_CTRL_START;
		}else{
			ctrl_byte |= MOD_SPI_CTRL_V3_START;
		}
#endif
		ctrl_byte |= MOD_SPI_CTRL_CHAIN;	/* always chain */
		data |= ctrl_byte << 24;

	}else if (type == MOD_TYPE_FXS){

		/* bit 8-15: register byte */
		reg = reg & 0x7F;
		reg |= MOD_SPI_ADDR_FXS_READ; 
		data |= (reg & 0xFF) << 8;
		
		/* bit 16-23: chip select byte
		** bit 16
		**
		**
		**			*/
		if (mod_no % 2){
			/* Select first chip in a chain */
			cs = MOD_SPI_CS_FXS_CHIP_0;
		}else{
			/* Select second chip in a chain */
			cs = MOD_SPI_CS_FXS_CHIP_1;
		}
		data |= cs << 16;

		/* bit 24-31: ctrl byte
		** bit 24
		**
		**
		**			*/
		ctrl_byte = mod_no / 2;
#if !defined(SPI2STEP)
		if (hw->hwcard->core_rev > 3){
			ctrl_byte |= MOD_SPI_CTRL_START;
		}else{
			ctrl_byte |= MOD_SPI_CTRL_V3_START;
		}
#endif
		ctrl_byte |= MOD_SPI_CTRL_FXS;
		if (chain){
			ctrl_byte |= MOD_SPI_CTRL_CHAIN;
		}
		data |= ctrl_byte << 24;

	}else{
		DEBUG_EVENT("%s: Module %d: Unsupported module type %d!\n",
				hw->devname, mod_no, type);
		return -EINVAL;
	}

	sdla_bus_write_4(hw, SPI_INTERFACE_REG, data);	
#if defined(SPI2STEP)
	WP_DELAY(1);
	if (hw->hwcard->core_rev > 3){
		data |= MOD_SPI_START;
	}else{
		data |= MOD_SPI_V3_START;
	}
	sdla_bus_write_4(hw, SPI_INTERFACE_REG, data);	
#endif
	DEBUG_TEST("%s: %s: Module %d - Execute SPI command %08X\n",
					hw->devname,
					__FUNCTION__,
					mod_no,
					data);
	for (i=0;i<10;i++){
		WP_DELAY(10);
		sdla_bus_read_4(hw, SPI_INTERFACE_REG, &data);
		if (data & MOD_SPI_BUSY) {
			continue;
		}
	}

	if (data & MOD_SPI_BUSY){
		DEBUG_EVENT("%s: Module %d: Critical Error (%s:%d) Data=0x%0X!\n",
					hw->devname, mod_no,
					__FUNCTION__,__LINE__,data);
		return 0xFF;
	}

	return (u8)(data & 0xFF);
}

u_int8_t sdla_shark_rm_read_fe (void* phw, ...)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	va_list		args;
	int		mod_no, type, chain, reg;
	unsigned char	data = 0;
#if defined(WAN_DEBUG_FE)
	char		*fname;
	int		fline;
#endif

	va_start(args, phw);
	mod_no	= va_arg(args, int);
	type	= va_arg(args, int);
	chain	= va_arg(args, int);
	reg	= va_arg(args, int);
#if defined(WAN_DEBUG_FE)
	fname	= va_arg(args, char*);
	fline	= va_arg(args, int);
#endif
	va_end(args);

	if (sdla_hw_fe_test_and_set_bit(hw,0)){
#if defined(WAN_DEBUG_FE)
		DEBUG_EVENT("%s: %s:%d: Critical Error: Re-entry in FE (%s:%d)!\n",
			hw->devname, __FUNCTION__,__LINE__,fname,fline);
#else
		DEBUG_EVENT("%s: %s:%d: Critical Error: Re-entry in FE!\n",
			hw->devname, __FUNCTION__,__LINE__);
#endif		
		return 0x00;
	}
	data = __sdla_shark_rm_read_fe (hw, mod_no, type, chain, reg);

	sdla_hw_fe_clear_bit(hw,0);
	return data;
}


