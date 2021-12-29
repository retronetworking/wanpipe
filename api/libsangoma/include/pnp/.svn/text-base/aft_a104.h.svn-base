/*****************************************************************************
* aft_a104.h	WANPIPE(tm) S51XX Xilinx Hardware Support
* 		
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Oct 18, 2005	Nenad Corbic	Initial version.
*****************************************************************************/


#ifndef __AFT_A104_H_
#define __AFT_A104_H_

#ifdef WAN_KERNEL

#if 0
#define A104_SECURITY_32_ECCHAN		0x00
#define A104_SECURITY_64_ECCHAN		0x01
#define A104_SECURITY_96_ECCHAN		0x02
#define A104_SECURITY_128_ECCHAN	0x03
#define A104_SECURITY_256_ECCHAN	0x04
#define A104_SECURITY_PROTO_128_ECCHAN	0x05
#define A104_SECURITY_0_ECCHAN		0x07

#define A104_ECCHAN(val)				\
	((val) == A104_SECURITY_32_ECCHAN)  	? 32 :	\
	((val) == A104_SECURITY_64_ECCHAN)  	? 64 :	\
	((val) == A104_SECURITY_96_ECCHAN)  	? 96 :	\
	((val) == A104_SECURITY_128_ECCHAN)	? 128 :	\
	((val) == A104_SECURITY_PROTO_128_ECCHAN) ? 128 :	\
	((val) == A104_SECURITY_256_ECCHAN)	? 256 : 0
#endif

int a104_global_chip_config(sdla_t *card);
int a104_global_chip_unconfig(sdla_t *card);
int a104_chip_config(sdla_t *card);
int a104_chip_unconfig(sdla_t *card);
int a104_chan_dev_config(sdla_t *card, void *chan);
int a104_chan_dev_unconfig(sdla_t *card, void *chan);
int a104_led_ctrl(sdla_t *card, int color, int led_pos, int on);
int a104_test_sync(sdla_t *card, int tx_only);
int a104_check_ec_security(sdla_t *card);


//int a104_write_fe (sdla_t* card, unsigned short off, unsigned char value);
//unsigned char a104_read_fe (sdla_t* card, unsigned short off);
int a104_write_fe (void *pcard, ...);
unsigned char __a104_read_fe (void *pcard, ...);
unsigned char a104_read_fe (void *pcard, ...);

int aft_te1_write_cpld(sdla_t *card, unsigned short off,unsigned char data);
unsigned char aft_te1_read_cpld(sdla_t *card, unsigned short cpld_off);

int a108m_write_cpld(sdla_t *card, unsigned short off,unsigned char data);
unsigned char a108m_read_cpld(sdla_t *card, unsigned short cpld_off);

void a104_fifo_adjust(sdla_t *card,u32 level);



#endif

#endif
