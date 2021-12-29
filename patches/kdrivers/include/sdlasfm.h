/*****************************************************************************
* sdlasfm.h	WANPIPE(tm) Multiprotocol WAN Link Driver.
*		Definitions for the SDLA Firmware Module (SFM).
*
* Author: 	Gideon Hack 	
*
* Copyright:	(c) 1995-1999 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jun 02, 1999  Gideon Hack	Added support for the S514 adapter.
* Dec 11, 1996	Gene Kozin	Cosmetic changes
* Apr 16, 1996	Gene Kozin	Changed adapter & firmware IDs. Version 2
* Dec 15, 1995	Gene Kozin	Structures chaned
* Nov 09, 1995	Gene Kozin	Initial version.
*****************************************************************************/
#ifndef	_SDLASFM_H
#define	_SDLASFM_H

/****** Defines *************************************************************/

#define	SFM_VERSION	2
#define	SFM_SIGNATURE	"SFM - Sangoma SDLA Firmware Module"

/* min/max */
#define	SFM_IMAGE_SIZE	0x8000	/* max size of SDLA code image file */
#define	SFM_DESCR_LEN	256	/* max length of description string */
#define	SFM_MAX_SDLA	16	/* max number of compatible adapters */

/* Adapter types */
#define SDLA_S502A	5020
#define SDLA_S502E	5021
#define SDLA_S503	5030
#define SDLA_S508	5080
#define SDLA_S507	5070
#define SDLA_S509	5090
#define SDLA_S514	5140
#define SDLA_ADSL	6000
#define SDLA_AFT	7000

/* S514 PCI adapter CPU numbers */
#define S514_CPU_A	'A'
#define S514_CPU_B	'B'
#define SDLA_CPU_A	1
#define SDLA_CPU_B	2
#define SDLA_GET_CPU(cpu_no)	(cpu_no==SDLA_CPU_A)?S514_CPU_A:S514_CPU_B


/* Firmware identification numbers:
 *    0  ..  999	Test & Diagnostics
 *  1000 .. 1999	Streaming HDLC
 *  2000 .. 2999	Bisync
 *  3000 .. 3999	SDLC
 *  4000 .. 4999	HDLC
 *  5000 .. 5999	X.25
 *  6000 .. 6999	Frame Relay
 *  7000 .. 7999	PPP
 *  8000 .. 8999        Cisco HDLC
 */
#define	SFID_CALIB502	 200
#define	SFID_STRM502	1200
#define	SFID_STRM508	1800
#define	SFID_BSC502	2200
#define SFID_BSCMP514	2201
#define	SFID_SDLC502	3200
#define	SFID_HDLC502	4200
#define	SFID_HDLC508	4800
#define	SFID_X25_502	5200
#define	SFID_X25_508	5800
#define	SFID_FR502	6200
#define	SFID_FR508	6800
#define	SFID_PPP502	7200
#define	SFID_PPP508	7800
#define SFID_PPP514	7140
#define	SFID_CHDLC508	8800
#define SFID_CHDLC514	8140
#define SFID_BITSTRM   10000
#define SFID_EDU_KIT    8141
#define SFID_SS7514	9000
#define SFID_BSCSTRM    2205
#define SFID_ADSL      20000
#define SFID_SDLC514    3300
#define SFID_ATM       11000	
#define SFID_POS       12000
#define SFID_ADCCP     13000
#define SFID_AFT       30000

/****** Data Types **********************************************************/

typedef struct	sfm_info		/* firmware module information */
{
	unsigned short	codeid;		/* firmware ID */
	unsigned short	version;	/* firmaware version number */
	unsigned short	adapter[SFM_MAX_SDLA]; /* compatible adapter types */
	unsigned long	memsize;	/* minimum memory size */
	unsigned short	reserved[2];	/* reserved */
	unsigned short	startoffs;	/* entry point offset */
	unsigned short	winoffs;	/* dual-port memory window offset */
	unsigned short	codeoffs;	/* code load offset */
	unsigned short	codesize;	/* code size */
	unsigned short	dataoffs;	/* configuration data load offset */
	unsigned short	datasize;	/* configuration data size */
} sfm_info_t;

typedef struct sfm			/* SDLA firmware file structire */
{
	char		signature[80];	/* SFM file signature */
	unsigned short	version;	/* file format version */
	unsigned short	checksum;	/* info + image */
	unsigned short	reserved[6];	/* reserved */
	char		descr[SFM_DESCR_LEN]; /* description string */
	sfm_info_t	info;		/* firmware module info */
	unsigned char	image[1];	/* code image (variable size) */
} sfm_t;

/* settings for the 'adapter_type' */
#define S5141_ADPTR_1_CPU_SERIAL	0x0011	/* S5141, single CPU, serial */
#define S5142_ADPTR_2_CPU_SERIAL	0x0012	/* S5142, dual CPU, serial */
#define S5143_ADPTR_1_CPU_FT1		0x0013	/* S5143, single CPU, FT1 */
#define S5144_ADPTR_1_CPU_T1E1		0x0014	/* S5144, single CPU, T1/E1 */
#define S5145_ADPTR_1_CPU_56K		0x0015	/* S5145, single CPU, 56K */
#define S5147_ADPTR_2_CPU_T1E1		0x0017  /* S5147, dual CPU, T1/E1 */
#define S5148_ADPTR_1_CPU_T1E1		0x0018	/* S5148, single CPU, T1/E1 */

#define S518_ADPTR_1_CPU_ADSL		0x0018	/* S518, adsl card */

#define A101_ADPTR_TE1_MASK		0x0040	/* Single T1/E1 type mask  */
#define A101_ADPTR_1TE1			0x0041	/* 1 Channel T1/E1  */
#define A101_ADPTR_2TE1			0x0042	/* 2 Channels T1/E1 */

#define A104_ADPTR_TE1_MASK		0x0080	/* Quad T1/E1 type mask  */
#define A104_ADPTR_4TE1			0x0081	/* Quad line T1/E1 */

#define A100_ADPTR_T3E3_MASK		0x0100	/* T3/E3  type mask */
#define A100_ADPTR_U_1TE3		0x0101	/* 1 Channel T3/E3 (Proto) */
#define A300_ADPTR_U_1TE3		0x0102	/* 1 Channel T3/E3 */

#define OPERATE_T1E1_AS_SERIAL		0x8000  /* For bitstreaming only 
						 * Allow the applicatoin to 
						 * E1 front end */

/* settings for the 'adapter_security' */
#define AFT_SECURITY_NONE	0x00
#define AFT_SECURITY_CHAN	0x01
#define AFT_SECURITY_UNCHAN	0x02

/* CPLD definitions */
#define AFT_SECURITY_1LINE_UNCH		0x00
#define AFT_SECURITY_1LINE_CH		0x01
#define AFT_SECURITY_2LINE_UNCH		0x02
#define AFT_SECURITY_2LINE_CH		0x03

#define AFT_BIT_DEV_ADDR_CLEAR  	0x600
#define AFT_BIT_DEV_ADDR_CPLD  		0x200
#define AFT4_BIT_DEV_ADDR_CLEAR		0x800
#define AFT4_BIT_DEV_ADDR_CPLD		0x800

#define AFT_SECURITY_CPLD_REG		0x09
#define AFT_SECURITY_CPLD_SHIFT		0x02
#define AFT_SECURITY_CPLD_MASK		0x03
#define AFT_MCPU_INTERFACE_ADDR		0x46
#define AFT_MCPU_INTERFACE		0x44

#define SDLA_ADPTR_NAME(adapter_type)			\
		(adapter_type == S5141_ADPTR_1_CPU_SERIAL) ? "S514-1-PCI" : \
		(adapter_type == S5142_ADPTR_2_CPU_SERIAL) ? "S514-2-PCI" : \
		(adapter_type == S5143_ADPTR_1_CPU_FT1)    ? "S514-3-PCI" : \
		(adapter_type == S5144_ADPTR_1_CPU_T1E1)   ? "S514-4-PCI" : \
		(adapter_type == S5145_ADPTR_1_CPU_56K)    ? "S514-5-PCI" : \
		(adapter_type == S5147_ADPTR_2_CPU_T1E1)   ? "S514-7-PCI" : \
		(adapter_type == S518_ADPTR_1_CPU_ADSL)    ? "S518-PCI" : \
		(adapter_type == A101_ADPTR_1TE1) 	   ? "AFT-A101" : \
		(adapter_type == A101_ADPTR_2TE1)	   ? "AFT-A102" : \
		(adapter_type == A104_ADPTR_4TE1)	   ? "AFT-A104" : \
		(adapter_type == A300_ADPTR_U_1TE3) 	   ? "AFT-A300" : \
							     "UNKNOWN"

#if 0
#define SDLA_ADPTR_DECODE(adapter_type)			\
		(adapter_type == S5141_ADPTR_1_CPU_SERIAL) ? "S514-1-PCI" : \
		(adapter_type == S5142_ADPTR_2_CPU_SERIAL) ? "S514-2-PCI" : \
		(adapter_type == S5143_ADPTR_1_CPU_FT1)    ? "S514-3-PCI" : \
		(adapter_type == S5144_ADPTR_1_CPU_T1E1)   ? "S514-4-PCI" : \
		(adapter_type == S5145_ADPTR_1_CPU_56K)    ? "S514-5-PCI" : \
		(adapter_type == S5147_ADPTR_2_CPU_T1E1)   ? "S514-7-PCI" : \
		(adapter_type == S518_ADPTR_1_CPU_ADSL)    ? "S518-PCI  " : \
		(adapter_type == A101_ADPTR_1TE1) 	   ? "AFT-A101  " : \
		(adapter_type == A101_ADPTR_2TE1)	   ? "AFT-A102  " : \
		(adapter_type == A104_ADPTR_4TE1)	   ? "AFT-A104  " : \
		(adapter_type == A300_ADPTR_U_1TE3) 	   ? "AFT-A300  " : \
							     "UNKNOWN   "
#endif

#define AFT_GET_SECURITY(security)		\
		((security >> AFT_SECURITY_CPLD_SHIFT) & AFT_SECURITY_CPLD_MASK)

#define AFT_SECURITY(adapter_security)					\
		(adapter_security == AFT_SECURITY_CHAN) 	? "c" : 	\
		(adapter_security == AFT_SECURITY_UNCHAN) 	? "u" : ""

#define AFT_SECURITY_DECODE(adapter_security)					\
		(adapter_security == AFT_SECURITY_CHAN) 	? "Channelized" : 	\
		(adapter_security == AFT_SECURITY_UNCHAN) 	? "Unchannelized" : ""

#endif	/* _SDLASFM_H */

