
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
#define EEPROM_SIZE		0xFF

#define EEPROM_BUSY		19
#define EEPROM_CS_ENABLE	18
#define EEPROM_BYTE_READ_START	17
#define EEPROM_READ_DATA	8
#define EEPROM_WRITE_DATA	0
#define EEPROM_BYTE_WRITE_START 16


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
int EE_WaitIdle(void *info);
void EE_Off(void *info);
unsigned char EE_ReadByte(void *info);
void EE_WriteByte(void *info, unsigned char val);

unsigned char read_eeprom_status(void *info);
unsigned char read_eeprom_data8(void *info, short address);
void write_eeprom_data8(void *info, short address, unsigned char data);

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
	for (ii = 0; ii < 100; ii++)
	{
	       	/* read current value in EECTL */
		PEX_8111Read(info, EECTL, &eeCtl);
		/* loop until idle */
		if ((eeCtl & (1 << EEPROM_BUSY)) == 0)
			return(eeCtl);
	}
	printf("ERROR: EEPROM Busy timeout!\n");
	while(1);
}


///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////



void EE_Off(void *info)
{	
	int t = 0;
	
	EE_WaitIdle(info); /* make sure EEPROM is idle */
	PEX_8111Write(info, EECTL, t); /* turn off everything (especially EEPROM_CS_ENABLE)*/
}

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


unsigned char EE_ReadByte(void *info)
{
       	/* make sure EEPROM is idle */
	int eeCtl = EE_WaitIdle(info);

	eeCtl |=	(1 << EEPROM_CS_ENABLE) |
	       		(1 << EEPROM_BYTE_READ_START);
	PEX_8111Write(info, EECTL, eeCtl); /* start reading */
	eeCtl = EE_WaitIdle(info); /* wait until read is done */
	
	/* extract read data from EECTL */
	return (unsigned char)((eeCtl >> EEPROM_READ_DATA) & 0xff);
}

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void EE_WriteByte(void *info, unsigned char val)
{
	int eeCtl = EE_WaitIdle(info); /* make sure EEPROM is idle */	
	
	/* clear current WRITE value */
	eeCtl &= ~(0xff << EEPROM_WRITE_DATA);
	eeCtl |= 	(1 << EEPROM_CS_ENABLE) |
			(1 << EEPROM_BYTE_WRITE_START) |
			((val & 0xff) << EEPROM_WRITE_DATA);

	PEX_8111Write(info, EECTL, eeCtl);
}
///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//These are the high level functions

unsigned char read_eeprom_status(void *info)
{	
	unsigned char status = 0;

    	EE_WriteByte(info, READ_STATUS_EE_OPCODE); /* read status opcode */
	status = EE_ReadByte(info); /* get EEPROM status */
	EE_Off(info); /* turn off EEPROM */

	return status;
}


void wan_plxctrl_write_ebyte(void *info, unsigned char addr, unsigned char data)
{
	int	i = 0, eeCtl = 0;
	
	/* busy verification */
	PEX_8111Read(info, EECTL, &eeCtl);
	if ((eeCtl & (1 << 19)) != 0){
	 	printf("ERROR: EEPROM is busy %08X (init)!\n", eeCtl);
		return;
	}
	eeCtl = 0x00000000;
	eeCtl |= (1 << EEPROM_CS_ENABLE);
	PEX_8111Write(info, EECTL, eeCtl);
		
	eeCtl &= ~(0xff << EEPROM_WRITE_DATA);
	eeCtl |= 	(1 << EEPROM_CS_ENABLE) |
			(1 << EEPROM_BYTE_WRITE_START) |
			(6 << EEPROM_WRITE_DATA);
	PEX_8111Write(info, EECTL, eeCtl);
	
	for(i=0; i<1000;i++){
		PEX_8111Read(info, EECTL, &eeCtl);
		if ((eeCtl & (1 << 16)) == 0){
			break;
		}
	}	
	if ((eeCtl & (1 << 16)) != 0){
	 	printf("ERROR: EEPROM timeout (wren cmd)!\n");
		return;
	}
	
	EE_Off(info); /* turn off EEPROM */

	/* busy verification */
	PEX_8111Read(info, EECTL, &eeCtl);
	if ((eeCtl & (1 << 19)) != 0){
	 	printf("ERROR: EEPROM is busy!\n");
		return;
	}
	eeCtl = 0x00000000;
	eeCtl |= (1 << EEPROM_CS_ENABLE);
	PEX_8111Write(info, EECTL, eeCtl);
		
	eeCtl &= ~(0xff << EEPROM_WRITE_DATA);
	eeCtl |= 	(1 << EEPROM_CS_ENABLE) |
		  	(1 << EEPROM_BYTE_WRITE_START) |
			(2 << EEPROM_WRITE_DATA);
	PEX_8111Write(info, EECTL, eeCtl);
	for(i=0; i<1000;i++){
		PEX_8111Read(info, EECTL, &eeCtl);
		if ((eeCtl & (1 << 16)) == 0){
			break;
		}
	}	
	if ((eeCtl & (1 << 16)) != 0){
	 	printf("ERROR: EEPROM timeout (write cmd)!\n");
		return;
	}
	
	eeCtl = 0x00000000;
	eeCtl |= (1 << EEPROM_CS_ENABLE);
	PEX_8111Write(info, EECTL, eeCtl);
		
	eeCtl &= ~(0xff << EEPROM_WRITE_DATA);
	eeCtl |= 	(1 << EEPROM_CS_ENABLE) |
		  	(1 << EEPROM_BYTE_WRITE_START) |
			(addr << EEPROM_WRITE_DATA);
	PEX_8111Write(info, EECTL, eeCtl);
	for(i=0; i<1000;i++){
		PEX_8111Read(info, EECTL, &eeCtl);
		if ((eeCtl & (1 << 16)) == 0){
			break;
		}
	}	
	if ((eeCtl & (1 << 16)) != 0){
	 	printf("ERROR: EEPROM timeout (addr)!\n");
		return;
	}
	
	eeCtl = 0x00000000;
	eeCtl |= (1 << EEPROM_CS_ENABLE);
	PEX_8111Write(info, EECTL, eeCtl);
		
	eeCtl &= ~(0xff << EEPROM_WRITE_DATA);
	eeCtl |= 	(1 << EEPROM_CS_ENABLE) |
		  	(1 << EEPROM_BYTE_WRITE_START) |
			(data << EEPROM_WRITE_DATA);
	PEX_8111Write(info, EECTL, eeCtl);
	for(i=0; i<1000;i++){
		PEX_8111Read(info, EECTL, &eeCtl);
		if ((eeCtl & (1 << 16)) == 0){
			break;
		}
	}	
	if ((eeCtl & (1 << 16)) != 0){
	 	printf("ERROR: EEPROM timeout (0x5A)!\n");
		return;
	}
	
	EE_Off(info); /* turn off EEPROM */
	return;
}


unsigned char wan_plxctrl_read_ebyte(void *info, unsigned char addr)
{
	int	i = 0, eeCtl = 0;
	unsigned char	data;

	/* busy verification */
	PEX_8111Read(info, EECTL, &eeCtl);
	if ((eeCtl & (1 << 19)) != 0){
	 	printf("ERROR: EEPROM is busy!\n");
		return 0xFF;
	}
	eeCtl = 0x00000000;
	eeCtl |= (1 << EEPROM_CS_ENABLE);
	PEX_8111Write(info, EECTL, eeCtl);
		
	eeCtl &= ~(0xff << EEPROM_WRITE_DATA);
	eeCtl |= 	(1 << EEPROM_CS_ENABLE) |
		  	(1 << EEPROM_BYTE_WRITE_START) |
			(3 << EEPROM_WRITE_DATA);
	PEX_8111Write(info, EECTL, eeCtl);
	for(i=0; i<1000;i++){
		PEX_8111Read(info, EECTL, &eeCtl);
		if ((eeCtl & (1 << 16)) == 0){
			break;
		}
	}	
	if ((eeCtl & (1 << 16)) != 0){
	 	printf("ERROR: EEPROM timeout (write cmd)!\n");
		return 0xFF;
	}
	
	eeCtl = 0x00000000;
	eeCtl |= (1 << EEPROM_CS_ENABLE);
	PEX_8111Write(info, EECTL, eeCtl);
		
	eeCtl &= ~(0xff << EEPROM_WRITE_DATA);
	eeCtl |= 	(1 << EEPROM_CS_ENABLE) |
		  	(1 << EEPROM_BYTE_WRITE_START) |
			(addr << EEPROM_WRITE_DATA);
	PEX_8111Write(info, EECTL, eeCtl);
	for(i=0; i<1000;i++){
		PEX_8111Read(info, EECTL, &eeCtl);
		if ((eeCtl & (1 << 16)) == 0){
			break;
		}
	}	
	if ((eeCtl & (1 << 16)) != 0){
	 	printf("ERROR: EEPROM timeout (addr)!\n");
		return 0xFF;
	}
	
	eeCtl = 0x00000000;
	eeCtl |= (1 << EEPROM_CS_ENABLE) |
		 (1 << EEPROM_BYTE_READ_START);
	PEX_8111Write(info, EECTL, eeCtl);
	for(i=0; i<1000;i++){
		PEX_8111Read(info, EECTL, &eeCtl);
		if ((eeCtl & (1 << 17)) == 0){
			break;
		}
	}	
	if ((eeCtl & (1 << 17)) != 0){
	 	printf("ERROR: EEPROM timeout (read)!\n");
		return 0xFF;
	}
	EE_WaitIdle(info);
	PEX_8111Read(info, EECTL, &eeCtl);
	data = (eeCtl >> 8) & 0xFF;
	EE_Off(info); /* turn off EEPROM */
	
	sleep(1);	/* wait until PLX EEPROM finish write command */
	return data;
}

void write_eeprom_data8(void *info, short addr, unsigned char data)
{
    

	EE_WriteByte(info, WREN_EE_OPCODE); /* must first write-enable */
	EE_Off(info); /* turn off EEPROM */
	EE_WriteByte(info, WRITE_EE_OPCODE); /* opcode to write bytes */

	/* Send low byte of address */
	EE_WriteByte(info, (unsigned char)(addr & 0xFF));
	
	EE_WriteByte(info, 0xFF & data); /* send data to be written */
	
	EE_Off(info); /* turn off EEPROM */
}

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

unsigned char read_eeprom_data8(void *info, short addr)
{	
	unsigned char	ch;
    
    	EE_WriteByte(info, READ_EE_OPCODE);
	EE_WriteByte(info, (unsigned char)(addr & 0xFF));

	ch = EE_ReadByte(info);
    	EE_Off(info);
	return ch;
}

void erase_eeprom_data(void *info)
{	
	int t;

	for(t = 0; t < EEPROM_SIZE; t++){
		write_eeprom_data8(info, t, 0xFF);
	}
	return;
}
