/*****************************************************************************
* sdladrv_utils.c	SDLADRV utils functions.
*
*
* Author:	Alex Feldman
*
* Copyright:	(c) 2007 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jan 20, 2007	Alex Feldman	Initial version
*
*****************************************************************************/


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
# include <wanpipe.h>
# include <sdladrv.h>
#elif defined(__WINDOWS__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe_version.h>
# include <wanpipe_debug.h>
# include <wanpipe_common.h>
# include <sdlasfm.h>	/* SDLA firmware module definitions */
# include <sdlapci.h>	/* SDLA PCI hardware definitions */
# include <wanpipe.h>
# include <sdladrv.h>	/* API definitions */
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
//This is the address of the ECCTL register , page 149 of PEX8111 datasheet.
#define SDLA_PLX_EECTL			0x1004	

//Change this accordingly.!!!
#define SDLA_PLX_EEPROM_SIZE		0xFF

#define SDLA_PLX_EEPROM_BUSY		19
#define SDLA_PLX_EEPROM_CS_ENABLE	18
#define SDLA_PLX_EEPROM_BYTE_READ_START	17
#define SDLA_PLX_EEPROM_READ_DATA	8
#define SDLA_PLX_EEPROM_WRITE_DATA	0
#define SDLA_PLX_EEPROM_BYTE_WRITE_START 16

//EEPROM COMMANDS
#define SDLA_PLX_READ_STATUS_EE_OPCODE	0x05
#define SDLA_PLX_WREN_EE_OPCODE 	0x06
#define SDLA_PLX_WRITE_EE_OPCODE	0x02
#define SDLA_PLX_READ_EE_OPCODE		0x03


/***************************************************************************
****               F U N C T I O N   P R O T O T Y P E S                ****
***************************************************************************/

/***************************************************************************
****                      G L O B A L  D A T A                          ****
***************************************************************************/

/***************************************************************************
****               F U N C T I O N   D E F I N I T I O N                ****
***************************************************************************/
static void		sdla_plx_8111Read(void *phw, int addr, int *data);
static void		sdla_plx_8111Write(void *phw, int addr, int data);
static int		sdla_plx_EE_waitidle(void *phw);
static int		sdla_plx_EE_off(void *phw);
static unsigned char	sdla_plx_EE_readbyte(void *phw);
static int		sdla_plx_EE_writebyte(void *phw, unsigned char);

unsigned char	sdla_plxctrl_status(void *phw);
unsigned char	sdla_plxctrl_read8(void *phw, short);
void		sdla_plxctrl_write8(void *phw, short, unsigned char);
void		sdla_plxctrl_erase(void *phw);

extern int	sdla_pci_bridge_read_config_dword(void*, int, u_int32_t*);
extern int	sdla_pci_bridge_write_config_dword(void*, int, u_int32_t);

/************************************************************************/
/*			GLOBAL VARIABLES				*/
/************************************************************************/

/************************************************************************/
/*			FUNCTION PROTOTYPES				*/
/************************************************************************/

///////////////////////////////////////////////////////////////////
//Write the value read into the dst pointer.
static void sdla_plx_8111Read(void *phw, int addr, int *data)
{	
	WAN_ASSERT_VOID(phw == NULL);
	if (addr < 0x1000){
		sdla_pci_bridge_read_config_dword(phw, addr, data);
	}else if (addr >= 0x1000 && addr <= 0x1FFF){
		sdla_pci_bridge_write_config_dword(phw, 0x84, addr);
		sdla_pci_bridge_read_config_dword(phw, 0x88, data);
	}	
}

///////////////////////////////////////////////////////////////////
static void sdla_plx_8111Write(void *phw, int addr, int data)
{
	WAN_ASSERT_VOID(phw == NULL);
	if (addr < 0x1000){
		sdla_pci_bridge_write_config_dword(phw, addr, data);
	}else if (addr >= 0x1000 && addr <= 0x1FFF){			
		sdla_pci_bridge_write_config_dword(phw, 0x84, addr);
		sdla_pci_bridge_write_config_dword(phw, 0x88, data);
	}
}

///////////////////////////////////////////////////////////////////
static int sdla_plx_EE_waitidle(void *phw)
{	
	sdlahw_t	*hw = (sdlahw_t*)phw;
	int 		eeCtl, ii;

	WAN_ASSERT(phw == NULL);
	for (ii = 0; ii < 100; ii++){
	       	/* read current value in EECTL */
		sdla_plx_8111Read(phw, SDLA_PLX_EECTL, &eeCtl);
		/* loop until idle */
		if ((eeCtl & (1 << SDLA_PLX_EEPROM_BUSY)) == 0){
			return(eeCtl);
		}
	}
	DEBUG_EVENT("%s: ERROR: EEPROM Busy timeout!\n",
				hw->devname);
	return -EINVAL;
}

///////////////////////////////////////////////////////////////////
static int sdla_plx_EE_off(void *phw)
{	
	
	WAN_ASSERT(phw == NULL);
	 /* make sure EEPROM is idle */
	sdla_plx_EE_waitidle(phw);
	/* turn off everything (especially SDLA_PLX_EEPROM_CS_ENABLE)*/
	sdla_plx_8111Write(phw, SDLA_PLX_EECTL, 0);
	return 0; 
}

///////////////////////////////////////////////////////////////////
static unsigned char sdla_plx_EE_readbyte(void *phw)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	int		i, eeCtl = 0x00;
	unsigned char	data = 0x00;

	WAN_ASSERT_RC(phw == NULL, 0xFF);
	eeCtl = sdla_plx_EE_waitidle(phw);

	eeCtl = 0;
	eeCtl |= (1 << SDLA_PLX_EEPROM_CS_ENABLE) |
	       	 (1 << SDLA_PLX_EEPROM_BYTE_READ_START);
	sdla_plx_8111Write(phw, SDLA_PLX_EECTL, eeCtl); /* start reading */
	for (i=0;i<1000;i++){
		sdla_plx_8111Read(phw, SDLA_PLX_EECTL, &eeCtl);
		if ((eeCtl & (1 << SDLA_PLX_EEPROM_BYTE_READ_START)) == 0){
			break;
		}
	}
	if ((eeCtl & (1 << SDLA_PLX_EEPROM_BYTE_READ_START)) != 0){
		DEBUG_EVENT("%s: Timeout on PLX READ!\n",
					hw->devname);
		return 0xFF;
	}
	
	eeCtl = sdla_plx_EE_waitidle(phw); /* wait until read is done */
	sdla_plx_8111Read(phw, SDLA_PLX_EECTL, &eeCtl);
	data = (eeCtl >> SDLA_PLX_EEPROM_READ_DATA) & 0xFF;
	
	/* extract read data from EECTL */
	return data;
}

///////////////////////////////////////////////////////////////////
static int sdla_plx_EE_writebyte(void *phw, unsigned char val)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	int 		i, eeCtl = 0; /* make sure EEPROM is idle */	
	
	WAN_ASSERT(phw == NULL);
	eeCtl = sdla_plx_EE_waitidle(phw); /* make sure EEPROM is idle */	
	/* clear current WRITE value */
	eeCtl = 0;
	eeCtl &= ~(0xff << SDLA_PLX_EEPROM_WRITE_DATA);
	eeCtl |= 	(1 << SDLA_PLX_EEPROM_CS_ENABLE) |
			(1 << SDLA_PLX_EEPROM_BYTE_WRITE_START) |
			((val & 0xff) << SDLA_PLX_EEPROM_WRITE_DATA);
	sdla_plx_8111Write(phw, SDLA_PLX_EECTL, eeCtl);
	
	for (i=0;i<1000;i++){
		sdla_plx_8111Read(phw, SDLA_PLX_EECTL, &eeCtl);
		if ((eeCtl & (1 << SDLA_PLX_EEPROM_BYTE_WRITE_START)) == 0){
			break;
		}
	}
	if ((eeCtl & (1 << SDLA_PLX_EEPROM_BYTE_WRITE_START)) != 0){
		DEBUG_EVENT("%s: Timeout on PLX write!\n",
						hw->devname);
		return -EINVAL;
	}
	return 0;
}


/***************************************************************************
****			These are the high level functions		****
***************************************************************************/

///////////////////////////////////////////////////////////////////
unsigned char sdla_plxctrl_status(void *phw)
{	
	unsigned char status = 0;

	WAN_ASSERT_RC(phw == NULL, 0xFF);
    	sdla_plx_EE_writebyte(phw, SDLA_PLX_READ_STATUS_EE_OPCODE);
	status = sdla_plx_EE_readbyte(phw); /* get EEPROM status */
	sdla_plx_EE_off(phw); /* turn off EEPROM */

	return status;
}

void sdla_plxctrl_write8(void *phw, short addr, unsigned char data)
{

	WAN_ASSERT_VOID(phw == NULL);
	sdla_plx_EE_writebyte(phw, SDLA_PLX_WREN_EE_OPCODE); /* must first write-enable */
	sdla_plx_EE_off(phw); /* turn off EEPROM */
	sdla_plx_EE_writebyte(phw, SDLA_PLX_WRITE_EE_OPCODE); /* opcode to write bytes */

	/* Send low byte of address */
	sdla_plx_EE_writebyte(phw, (unsigned char)(addr & 0xFF));
	
	sdla_plx_EE_writebyte(phw, 0xFF & data); /* send data to be written */
	
	sdla_plx_EE_off(phw); /* turn off EEPROM */
	return;
}

///////////////////////////////////////////////////////////////////
unsigned char sdla_plxctrl_read8(void *phw, short addr)
{	
	unsigned char	ch;
    
	WAN_ASSERT_RC(phw == NULL, 0xFF);
    	sdla_plx_EE_writebyte(phw, SDLA_PLX_READ_EE_OPCODE);
	sdla_plx_EE_writebyte(phw, (unsigned char)(addr & 0xFF));

	ch = sdla_plx_EE_readbyte(phw);
    	sdla_plx_EE_off(phw);
	return ch;
}

void sdla_plxctrl_erase(void *phw)
{	
	int t;

	WAN_ASSERT_VOID(phw == NULL);
	for(t = 0; t < SDLA_PLX_EEPROM_SIZE; t++){
		sdla_plxctrl_write8(phw, t, 0xFF);
	}
	return;
}

