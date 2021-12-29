/******************************************************************************
 * sdla_ec.h	
 *
 * Author: 	Alex Feldman  <al.feldman@sangoma.com>
 *
 * Copyright:	(c) 1995-2001 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 ******************************************************************************
 */

#ifndef __SDLA_EC_H
# define __SDLA_EC_H

#define AFT_HWEC_CLEAR_RESET	1
#define AFT_HWEC_SET_RESET	2

#define WAN_OCT6100_RC_OK				0x0000
#define WAN_OCT6100_RC_CPU_INTERFACE_NO_RESPONSE	0x0001

/*
// OCT6100 state machine
enum {
	WAN_OCT6100_STATE_NONE = 0x00,
	WAN_OCT6100_STATE_RESET,
	WAN_OCT6100_STATE_READY,
	WAN_OCT6100_STATE_CHIP_READY
};
#define WAN_OCT6100_STATE_DECODE(state)				\
	(state == WAN_OCT6100_STATE_RESET) ? "Reset" :		\
	(state == WAN_OCT6100_STATE_READY) ? "Ready" :		\
	(state == WAN_OCT6100_STATE_CHIP_READY) ? "Chip Ready" :\
					"Unknown"
*/

/* OCT6100 api command */
enum {
	WAN_OCT6100_CMD_NONE		= 0x00,
	WAN_OCT6100_CMD_GET_INFO,
	WAN_OCT6100_CMD_SET_INFO,
	WAN_OCT6100_CMD_CLEAR_RESET,
	WAN_OCT6100_CMD_SET_RESET,
	WAN_OCT6100_CMD_BYPASS_ENABLE,
	WAN_OCT6100_CMD_BYPASS_DISABLE,
	WAN_OCT6100_CMD_API_WRITE,
	WAN_OCT6100_CMD_API_WRITE_SMEAR,
	WAN_OCT6100_CMD_API_WRITE_BURST,
	WAN_OCT6100_CMD_API_READ,
	WAN_OCT6100_CMD_API_READ_BURST
};

#define WAN_OCT6100_CMD_DECODE(cmd)				\
	(cmd == WAN_OCT6100_CMD_GET_INFO) ? "Get Info" :	\
	(cmd == WAN_OCT6100_CMD_SET_INFO) ? "Set Info" :	\
	(cmd == WAN_OCT6100_CMD_CLEAR_RESET) ? "Clear Reset" :	\
	(cmd == WAN_OCT6100_CMD_SET_RESET) ? "Set Reset" :	\
	(cmd == WAN_OCT6100_CMD_BYPASS_ENABLE) ? "Bypass Enable" :	\
	(cmd == WAN_OCT6100_CMD_BYPASS_DISABLE) ? "Bypass Disable" :	\
	(cmd == WAN_OCT6100_CMD_API) ? "Oct6100 APi" :	\
					"Unknown"

#if 0
enum {
	WAN_OCT6100_BYPASS	= 0x01,
	WAN_OCT6100_BYPASS_FULL,
	WAN_OCT6100_BYPASS_LINE,
};
#endif

/*===========================================================================*\
  User process context - This would probably have to be defined elsewhere.
  This structure must be allocated by the calling process and the parameters
  should be defined prior to open the OCT6100 chip.
\*===========================================================================*/
typedef struct _OCTPCIDRV_USER_PROCESS_CONTEXT_
{
	/* Is main process */
	unsigned int	fMainProcess;

	/* Handle to driver (opened by calling process) */
	unsigned int	hDriver;

	/* Interface name to driver (copied by calling process) */
	unsigned char	devname[WAN_DRVNAME_SZ+1];
	unsigned char	ifname[WAN_IFNAME_SZ+1];

	/* Handle to serialization object used for read and writes */
	unsigned int 	ulUserReadWriteSerObj;

	/* Board index. */
	unsigned int	ulBoardId;

	/* Board type. */
	unsigned int	ulBoardType;

} tOCTPCIDRV_USER_PROCESS_CONTEXT, *tPOCTPCIDRV_USER_PROCESS_CONTEXT;

#define u_ecd			u.ecd
#define u_oct6100_write		u.oct6100_write
#define u_oct6100_write_swear	u.oct6100_write_swear
#define u_oct6100_write_burst	u.oct6100_write_burst
#define u_oct6100_read		u.oct6100_read
#define u_oct6100_read_burst	u.oct6100_read_burst
typedef struct wan_ec_api_ {
	char		name[WAN_DRVNAME_SZ+1];
	char		devname[WAN_DRVNAME_SZ+1];
	char		if_name[WAN_IFNAME_SZ+1];
	int		cmd;
	int		type;
	unsigned int	ulResult;
	
	/* extra arguments for EC */
#if 0
	int		state;
	unsigned char	fe_media;
	int		fe_lineno;
	int		channel;
	int		chip_no;
	int		max_channels;
	unsigned int	ulApiInstanceSize;
#endif
	union {
		struct {
			int		state;
			unsigned char	fe_media;
			int		fe_lineno;
			int		fe_max_channels;
			int		fe_tdmv_law;
			int		channel;
			int		chip_no;
			int		max_channels;
			int		max_ec_channels;
			unsigned int	ulApiInstanceSize;
		} ecd;
		struct {
			unsigned int ulChipIndex;
			unsigned int ulWriteAddress;
			unsigned short usWriteData;
		} oct6100_write;
		struct {
			unsigned int ulChipIndex;
			unsigned int ulWriteAddress;
			unsigned short usWriteData;
			unsigned int ulWriteLength;
		} oct6100_write_swear;
		struct {
			unsigned int ulChipIndex;
			unsigned int ulWriteAddress;
			unsigned int ulWriteLength;
			unsigned short *pWriteData;
		} oct6100_write_burst;
		struct {
			unsigned int ulChipIndex;
			unsigned int ulReadAddress;
			unsigned short usReadData;
		} oct6100_read;
		struct {
			unsigned int ulChipIndex;
			unsigned int ulReadAddress;
			unsigned int ulReadLength;
			unsigned short *pReadData;
		} oct6100_read_burst;
	} u;
} wan_ec_api_t;

typedef struct wan_ec_iface_
{
	int	(*action)(void *pcard, int, int);

} wan_ec_iface_t;

#if defined(WAN_KERNEL)
/* global interface functions */
void *wan_ec_config (void *pcard, int max_channels);
int wan_ec_remove(void*, void *pcard);
int wan_ec_ioctl(void*, struct ifreq*, void *pcard);
#endif

#endif /* __SDLA_EC_H */
