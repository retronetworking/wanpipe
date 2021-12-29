
/***********************************************************************
* wan_plxctr.c	Sangoma PLX Control Utility.
*
* Copyright:	(c) 2005 AMFELTEC Corp.
*
* ----------------------------------------------------------------------
* Nov 1, 2006	Alex Feldman	Initial version.
***********************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

//This is the address of the ECCTL register , page 149 of PEX8111 datasheet.
#define EECTL			0x1004	

//Change this accordingly.!!!
#define PLXE_SIZE		0xFF

#define PLXE_READ_DATA_SHIFT		8
#define PLXE_WRITE_DATA_SHIFT		0

#define PLXE_BUSY_MASK			(1<<19)
#define PLXE_CS_ENABLE_MASK		(1<<18)
#define PLXE_BYTE_READ_START_MASK	(1<<17)
#define PLXE_BYTE_WRITE_START_MASK	(1<<16)

//EEPROM COMMANDS
#define READ_STATUS_EE_OPCODE	0x05
#define WREN_EE_OPCODE 		0x06
#define WRITE_EE_OPCODE		0x02
#define READ_EE_OPCODE		0x03


/************************************************************************/
/*			GLOBAL VARIABLES				*/
/************************************************************************/

/************************************************************************/
/*			FUNCTION PROTOTYPES				*/
/************************************************************************/
void PEX_8111Read(void *info, int addr, int *data);
void PEX_8111Write(void *info, int addr, int data);
int	EE_WaitIdle(void *info);
int	EE_Off(void *info);
int	EE_ReadByte(void *info, unsigned char*);
int	EE_WriteByte(void *info, unsigned char val);

unsigned char wan_plxctrl_status(void *info);

int wan_plxctrl_write8(void *info, unsigned char addr, unsigned char data);
int wan_plxctrl_read8(void *info, unsigned char, unsigned char*);
int wan_plxctrl_erase(void *info);

extern int exec_read_cmd(void*, unsigned int, unsigned int, unsigned int*);
extern int exec_write_cmd(void*, unsigned int, unsigned int, unsigned int);

/***********************************************
****************FUNCTION PROTOTYPES*************
************************************************/
//These functions are taken from page 37 of PEX8111 datasheet.


//Write the value read into the dst pointer.
void PEX_8111Read(void *info, int addr, int *data)
{	
	if (addr < 0x1000){
		exec_read_cmd(info, addr, 4, (unsigned int*)data);
	}else if (addr >= 0x1000 && addr <= 0x1FFF){
			
		exec_write_cmd(info, 0x84, 4, addr);
		exec_read_cmd(info, 0x88, 4, (unsigned int*)data);
	}	
}
 

void PEX_8111Write(void *info, int addr, int data)
{
	if (addr < 0x1000){
		exec_write_cmd(info, addr, 4, data);
	}else if (addr >= 0x1000 && addr <= 0x1FFF){
			
		exec_write_cmd(info, 0x84, 4, addr);
		exec_write_cmd(info, 0x88, 4, data);
	}
}

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


int EE_WaitIdle(void *info)
{	
	int eeCtl, ii;
	for (ii = 0; ii < 1000; ii++)
	{
	       	/* read current value in EECTL */
		PEX_8111Read(info, EECTL, &eeCtl);
		/* loop until idle */
		if ((eeCtl & PLXE_BUSY_MASK) == 0)
			return(eeCtl);
	}
	printf("ERROR: EEPROM Busy timeout!\n");
	return PLXE_BUSY_MASK;
}


///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////



int EE_Off(void *info)
{	
	int t = 0;
	
	/* make sure EEPROM is idle */
	EE_WaitIdle(info);
	/* turn off everything (especially PLXE_CS_ENABLE_MASK)*/
	PEX_8111Write(info, EECTL, t);
	return 0;
}

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


int EE_ReadByte(void *info, unsigned char *data)
{
       	/* make sure EEPROM is idle */
	int		eeCtl,i;

	*data = 0x00;
	eeCtl = EE_WaitIdle(info);

	eeCtl = 0;
	eeCtl |= PLXE_CS_ENABLE_MASK | PLXE_BYTE_READ_START_MASK;
	PEX_8111Write(info, EECTL, eeCtl); /* start reading */
	
	for (i=0;i<1000;i++){
		PEX_8111Read(info, EECTL, &eeCtl);
		if ((eeCtl & PLXE_BYTE_READ_START_MASK) == 0){
			break;
		}
	}
	if ((eeCtl & PLXE_BYTE_READ_START_MASK) != 0){
		printf("ERROR: EEPROM is still reading byte (busy)!\n");
		return -EBUSY;
	}

	EE_WaitIdle(info); /* wait until read is done */
	PEX_8111Read(info, EECTL, &eeCtl);
	*data = (eeCtl >> PLXE_READ_DATA_SHIFT) & 0xFF;
	
	/* extract read data from EECTL */
	return 0;
}

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

int EE_WriteByte(void *info, unsigned char val)
{
	int eeCtl,i;	
	
	eeCtl = EE_WaitIdle(info); /* make sure EEPROM is idle */
	
	/* clear current WRITE value */
	eeCtl = 0;
	eeCtl &= ~(0xff << PLXE_WRITE_DATA_SHIFT);
	eeCtl |= PLXE_CS_ENABLE_MASK | PLXE_BYTE_WRITE_START_MASK |
		 ((val & 0xff) << PLXE_WRITE_DATA_SHIFT);
	PEX_8111Write(info, EECTL, eeCtl);
	
	for (i=0;i<1000;i++){
		PEX_8111Read(info, EECTL, &eeCtl);
		if ((eeCtl & PLXE_BYTE_WRITE_START_MASK) == 0){
			break;
		}
	}
	if ((eeCtl & PLXE_BYTE_WRITE_START_MASK) != 0){
		printf("ERROR: EEPROM is still writting byte (busy)!\n");
		return -EBUSY;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//These are the high level functions

unsigned char wan_plxctrl_status(void *info)
{	
	unsigned char status = 0;

    	EE_WriteByte(info, READ_STATUS_EE_OPCODE); /* read status opcode */
	EE_ReadByte(info, &status); /* get EEPROM status */
	EE_Off(info); /* turn off EEPROM */

	return status;
}

int wan_plxctrl_write8(void *info, unsigned char addr, unsigned char data)
{
	EE_WriteByte(info, WREN_EE_OPCODE); /* must first write-enable */
	EE_Off(info); /* turn off EEPROM */
	EE_WriteByte(info, WRITE_EE_OPCODE); /* opcode to write bytes */

	/* Send low byte of address */
	EE_WriteByte(info, (unsigned char)(addr & 0xFF));
	
	EE_WriteByte(info, 0xFF & data); /* send data to be written */
	
	EE_Off(info); /* turn off EEPROM */
	return 0;
}

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

int wan_plxctrl_read8(void *info, unsigned char addr, unsigned char *data)
{	

	*data = 0x00;    
    	EE_WriteByte(info, READ_EE_OPCODE);
	EE_WriteByte(info, (unsigned char)(addr & 0xFF));

	EE_ReadByte(info, data);
    	EE_Off(info);
	return 0;
}

int wan_plxctrl_erase(void *info)
{	
	int t;

	for(t = 0; t < PLXE_SIZE; t++){
		wan_plxctrl_write8(info, t, 0xFF);
	}
	return 0;
}
