/******************************************************************************
 * wanec_lip.h	
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

#ifndef __WANEC_LIP_H
# define __WANEC_LIP_H

#if defined(__LINUX__)
# include <linux/wanpipe_events.h>
#elif defined(__WINDOWS__)

#if defined(__KERNEL__)
# define _DEBUG
# include <DebugOut.h>
#else
# include <windows.h>
#endif

# include <wanpipe_defines.h>
# include <wanpipe_includes.h>
# include <wanpipe_events.h>

#elif defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_events.h>
#endif
#include "oct6100api/oct6100_api.h"

#define	WANEC_BYDEFAULT_NORMAL

#define WANEC_DEV_DIR			"/dev/"
#define WANEC_DEV_NAME			"wanec"

#define MAX_CHANNELS_LEN	50

#define MAX_PARAM_LEN		50
#define MAX_VALUE_LEN		50

#define WAN_NUM_DTMF_TONES	16
#define WAN_NUM_PLAYOUT_TONES	16
#define WAN_MAX_TONE_LEN	100

#define WAN_EC_VERBOSE_NONE		0x00
#define WAN_EC_VERBOSE_EXTRA1		0x01
#define WAN_EC_VERBOSE_MASK_EXTRA1	0x01
#define WAN_EC_VERBOSE_EXTRA2		0x02
#define WAN_EC_VERBOSE_MASK_EXTRA2	0x03
#define WAN_EC_VERBOSE(ec_api)	(ec_api)?ec_api->verbose:WAN_EC_VERBOSE_NONE

#if defined(__WINDOWS__)
#define PRINT1	Debug
#define PRINT2	Debug
#else
#define PRINT(v, format,msg...)					\
	DEBUG_EVENT(format, ##msg)
#define PRINT1(v,format,msg...)					\
	if (v & WAN_EC_VERBOSE_EXTRA1) DEBUG_EVENT(format,##msg)
#define PRINT2(v,format,msg...)					\
	if (v & WAN_EC_VERBOSE_EXTRA2) DEBUG_EVENT(format,##msg)
#endif

#define WAN_OCT6100_RC_OK				0x0000
#define WAN_OCT6100_RC_CPU_INTERFACE_NO_RESPONSE	0x0001
#define WAN_OCT6100_RC_MEMORY				0x0002

/* WANPIPE EC API return code */
#define WAN_EC_API_RC_OK		0x0000
#define WAN_EC_API_RC_FAILED		0x0001
#define WAN_EC_API_RC_INVALID_CMD	0x0002
#define WAN_EC_API_RC_INVALID_STATE	0x0003
#define WAN_EC_API_RC_INVALID_DEV	0x0004
#define WAN_EC_API_RC_INVALID_CHANNELS	0x0005
#define WAN_EC_API_RC_BUSY		0x0006
#define WAN_EC_API_RC_NOACTION		0x0007
#define WAN_EC_API_RC_DECODE(err)					\
	(err == WAN_EC_API_RC_OK) 		? "OK" :		\
	(err == WAN_EC_API_RC_FAILED) 		? "Failed" :		\
	(err == WAN_EC_API_RC_INVALID_CMD) 	? "Invalid Cmd" :	\
	(err == WAN_EC_API_RC_INVALID_STATE) 	? "Invalid State" :	\
	(err == WAN_EC_API_RC_INVALID_DEV) 	? "Invalid Device" :	\
	(err == WAN_EC_API_RC_INVALID_CHANNELS) ? "Invalid Channels" :	\
	(err == WAN_EC_API_RC_BUSY) 		? "Busy" :		\
	(err == WAN_EC_API_RC_NOACTION) 	? "No action" :		\
							"Unknown RC"

/* OCT6100 state machine */
enum {
	WAN_OCT6100_STATE_NONE = 0x00,
	WAN_OCT6100_STATE_RESET,
	WAN_OCT6100_STATE_READY,
	WAN_OCT6100_STATE_CHIP_OPEN_PENDING,
	WAN_OCT6100_STATE_CHIP_OPEN,
	WAN_OCT6100_STATE_CHIP_READY
};
#define WAN_OCT6100_STATE_DECODE(state)						\
	(state == WAN_OCT6100_STATE_RESET)		? "Reset" :		\
	(state == WAN_OCT6100_STATE_READY)		? "Ready" :		\
	(state == WAN_OCT6100_STATE_CHIP_OPEN_PENDING)	? "Chip Open Pending" :	\
	(state == WAN_OCT6100_STATE_CHIP_OPEN)		? "Chip Open" :		\
	(state == WAN_OCT6100_STATE_CHIP_READY)		? "Chip Ready" :	\
					"Unknown"

#define WAN_EC_SOUT_DTMF_1	0x40000011
#define WAN_EC_SOUT_DTMF_2	0x40000012
#define WAN_EC_SOUT_DTMF_3	0x40000013
#define WAN_EC_SOUT_DTMF_A	0x4000001A
#define WAN_EC_SOUT_DTMF_4	0x40000014
#define WAN_EC_SOUT_DTMF_5	0x40000015
#define WAN_EC_SOUT_DTMF_6	0x40000016
#define WAN_EC_SOUT_DTMF_B	0x4000001B
#define WAN_EC_SOUT_DTMF_7	0x40000017
#define WAN_EC_SOUT_DTMF_8	0x40000018
#define WAN_EC_SOUT_DTMF_9	0x40000019
#define WAN_EC_SOUT_DTMF_C	0x4000001C
#define WAN_EC_SOUT_DTMF_STAR	0x4000001E
#define WAN_EC_SOUT_DTMF_0	0x40000010
#define WAN_EC_SOUT_DTMF_POUND	0x4000001F
#define WAN_EC_SOUT_DTMF_D	0x4000001D
#define WAN_EC_ROUT_DTMF_1	0x10000011
#define WAN_EC_ROUT_DTMF_2	0x10000012
#define WAN_EC_ROUT_DTMF_3	0x10000013
#define WAN_EC_ROUT_DTMF_A	0x1000001A
#define WAN_EC_ROUT_DTMF_4	0x10000014
#define WAN_EC_ROUT_DTMF_5	0x10000015
#define WAN_EC_ROUT_DTMF_6	0x10000016
#define WAN_EC_ROUT_DTMF_B	0x1000001B
#define WAN_EC_ROUT_DTMF_7	0x10000017
#define WAN_EC_ROUT_DTMF_8	0x10000018
#define WAN_EC_ROUT_DTMF_9	0x10000019
#define WAN_EC_ROUT_DTMF_C	0x1000001C
#define WAN_EC_ROUT_DTMF_STAR	0x1000001E
#define WAN_EC_ROUT_DTMF_0	0x10000010
#define WAN_EC_ROUT_DTMF_POUND 	0x1000001F
#define WAN_EC_ROUT_DTMF_D	0x1000001D
#define WAN_EC_DECODE_DTMF_ID(id)				\
		(id == WAN_EC_SOUT_DTMF_0)	? "SOUT_0" :	\
		(id == WAN_EC_SOUT_DTMF_1)	? "SOUT_1" :	\
		(id == WAN_EC_SOUT_DTMF_2)	? "SOUT_2" :	\
		(id == WAN_EC_SOUT_DTMF_3)	? "SOUT_3" :	\
		(id == WAN_EC_SOUT_DTMF_4)	? "SOUT_4" :	\
		(id == WAN_EC_SOUT_DTMF_5)	? "SOUT_5" :	\
		(id == WAN_EC_SOUT_DTMF_6)	? "SOUT_6" :	\
		(id == WAN_EC_SOUT_DTMF_7)	? "SOUT_7" :	\
		(id == WAN_EC_SOUT_DTMF_8)	? "SOUT_8" :	\
		(id == WAN_EC_SOUT_DTMF_9)	? "SOUT_9" :	\
		(id == WAN_EC_SOUT_DTMF_A)	? "SOUT_A" :	\
		(id == WAN_EC_SOUT_DTMF_B)	? "SOUT_B" :	\
		(id == WAN_EC_SOUT_DTMF_C)	? "SOUT_C" :	\
		(id == WAN_EC_SOUT_DTMF_D)	? "SOUT_D" :	\
		(id == WAN_EC_SOUT_DTMF_STAR)	? "SOUT_STAR" :	\
		(id == WAN_EC_SOUT_DTMF_POUND)	? "SOUT_POUND" :\
		(id == WAN_EC_ROUT_DTMF_0)	? "ROUT_0" :	\
		(id == WAN_EC_ROUT_DTMF_1)	? "ROUT_1" :	\
		(id == WAN_EC_ROUT_DTMF_2)	? "ROUT_2" :	\
		(id == WAN_EC_ROUT_DTMF_3)	? "ROUT_3" :	\
		(id == WAN_EC_ROUT_DTMF_4)	? "ROUT_4" :	\
		(id == WAN_EC_ROUT_DTMF_5)	? "ROUT_5" :	\
		(id == WAN_EC_ROUT_DTMF_6)	? "ROUT_6" :	\
		(id == WAN_EC_ROUT_DTMF_7)	? "ROUT_7" :	\
		(id == WAN_EC_ROUT_DTMF_8)	? "ROUT_8" :	\
		(id == WAN_EC_ROUT_DTMF_9)	? "ROUT_9" :	\
		(id == WAN_EC_ROUT_DTMF_A)	? "ROUT_A" :	\
		(id == WAN_EC_ROUT_DTMF_B)	? "ROUT_B" :	\
		(id == WAN_EC_ROUT_DTMF_C)	? "ROUT_C" :	\
		(id == WAN_EC_ROUT_DTMF_D)	? "ROUT_D" :	\
		(id == WAN_EC_ROUT_DTMF_STAR)	? "ROUT_STAR" :	\
		(id == WAN_EC_ROUT_DTMF_POUND)	? "ROUT_POUND" :\
						"Unknown"


#if defined(__WINDOWS__)
# define WAN_EC_CMD_NONE			0
# define WAN_EC_CMD_GETINFO			1
# define WAN_EC_CMD_CONFIG			2
# define WAN_EC_CMD_CHANNEL_OPEN	3
# define WAN_EC_CMD_RELEASE			4
# define WAN_EC_CMD_ENABLE			5
# define WAN_EC_CMD_DISABLE			6
# define WAN_EC_CMD_BYPASS_ENABLE	7
# define WAN_EC_CMD_BYPASS_DISABLE	8
# define WAN_EC_CMD_MODE_NORMAL		9
# define WAN_EC_CMD_MODE_POWERDOWN	10
# define WAN_EC_CMD_MODIFY_CHANNEL	11
# define WAN_EC_CMD_DTMF_ENABLE		12
# define WAN_EC_CMD_DTMF_DISABLE	13
# define WAN_EC_CMD_STATS			14
# define WAN_EC_CMD_STATS_FULL		15
# define WAN_EC_CMD_TONE_LOAD		16
# define WAN_EC_CMD_TONE_UNLOAD		17
# define WAN_EC_CMD_PLAYOUT_START	18
# define WAN_EC_CMD_PLAYOUT_STOP	19
# define WAN_EC_CMD_MONITOR			20
# define WAN_EC_CMD_RELEASE_ALL		21
#else
# define WAN_EC_CMD_NONE		_IOWR('E', 0, struct wan_ec_api_)
# define WAN_EC_CMD_GETINFO		_IOWR('E', 1, wan_ec_api_t)
# define WAN_EC_CMD_CONFIG		_IOWR('E', 2, struct wan_ec_api_)
# define WAN_EC_CMD_CHANNEL_OPEN	_IOWR('E', 3, struct wan_ec_api_)
# define WAN_EC_CMD_RELEASE		_IOWR('E', 4, struct wan_ec_api_)
# define WAN_EC_CMD_ENABLE		_IOWR('E', 5, struct wan_ec_api_)
# define WAN_EC_CMD_DISABLE		_IOWR('E', 6, struct wan_ec_api_)
# define WAN_EC_CMD_BYPASS_ENABLE	_IOWR('E', 7, struct wan_ec_api_)
# define WAN_EC_CMD_BYPASS_DISABLE	_IOWR('E', 8, struct wan_ec_api_)
# define WAN_EC_CMD_MODE_NORMAL		_IOWR('E', 9, struct wan_ec_api_)
# define WAN_EC_CMD_MODE_POWERDOWN	_IOWR('E', 10, struct wan_ec_api_)
# define WAN_EC_CMD_MODIFY_CHANNEL	_IOWR('E', 11, struct wan_ec_api_)
# define WAN_EC_CMD_DTMF_ENABLE		_IOWR('E', 12, struct wan_ec_api_)
# define WAN_EC_CMD_DTMF_DISABLE	_IOWR('E', 13, struct wan_ec_api_)
# define WAN_EC_CMD_STATS		_IOWR('E', 14, struct wan_ec_api_)
# define WAN_EC_CMD_STATS_FULL		_IOWR('E', 15, struct wan_ec_api_)
# define WAN_EC_CMD_TONE_LOAD		_IOWR('E', 16, struct wan_ec_api_)
# define WAN_EC_CMD_TONE_UNLOAD		_IOWR('E', 17, struct wan_ec_api_)
# define WAN_EC_CMD_PLAYOUT_START	_IOWR('E', 18, struct wan_ec_api_)
# define WAN_EC_CMD_PLAYOUT_STOP	_IOWR('E', 19, struct wan_ec_api_)
# define WAN_EC_CMD_MONITOR		_IOWR('E', 20, struct wan_ec_api_)
# define WAN_EC_CMD_RELEASE_ALL		_IOWR('E', 21, struct wan_ec_api_)
#endif

# define WAN_EC_CMD_DECODE(cmd)						\
	(cmd == WAN_EC_CMD_GETINFO)		? "Get Info" :		\
	(cmd == WAN_EC_CMD_CONFIG)		? "Config" :		\
	(cmd == WAN_EC_CMD_CHANNEL_OPEN)	? "Channel Open" :	\
	(cmd == WAN_EC_CMD_ENABLE)		? "Enable" :		\
	(cmd == WAN_EC_CMD_DISABLE)		? "Disable" :		\
	(cmd == WAN_EC_CMD_BYPASS_ENABLE)	? "Enable bypass" :	\
	(cmd == WAN_EC_CMD_BYPASS_DISABLE)	? "Disable bypass" :	\
	(cmd == WAN_EC_CMD_MODE_NORMAL)		? "Mode Normal" :	\
	(cmd == WAN_EC_CMD_MODE_POWERDOWN)	? "Mode PowerDown" :	\
	(cmd == WAN_EC_CMD_STATS)		? "Get stats" :		\
	(cmd == WAN_EC_CMD_STATS_FULL)		? "Get stats" :		\
	(cmd == WAN_EC_CMD_TONE_LOAD)		? "Tone load" :		\
	(cmd == WAN_EC_CMD_TONE_UNLOAD)		? "Tone unload" :	\
	(cmd == WAN_EC_CMD_PLAYOUT_START)	? "Playout start" :	\
	(cmd == WAN_EC_CMD_PLAYOUT_STOP)	? "Playout stop" :	\
	(cmd == WAN_EC_CMD_RELEASE)		? "Release" :		\
	(cmd == WAN_EC_CMD_RELEASE_ALL)		? "Release all" :	\
	(cmd == WAN_EC_CMD_MONITOR)		? "MONITOR" :		\
	(cmd == WAN_EC_CMD_MODIFY_CHANNEL)	? "MODIFY" :		\
	(cmd == WAN_EC_CMD_DTMF_ENABLE)		? "Enable DTMF" :	\
	(cmd == WAN_EC_CMD_DTMF_DISABLE)	? "Disable DTMF" :	\
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
#if defined(WAN_KERNEL)
	/* Interface name to driver (copied by calling process) */
	unsigned char	devname[WAN_DRVNAME_SZ+1];
	/*unsigned char	ifname[WAN_IFNAME_SZ+1];*/
	
	/* Board index. */
	unsigned int	ulBoardId;

	void		*ec_dev;
#else
	/* Is main process */
	unsigned int	fMainProcess;

	/* Handle to driver (opened by calling process) */
	void *ec_dev;

	/* Interface name to driver (copied by calling process) */
	unsigned char	devname[WAN_DRVNAME_SZ+1];
	unsigned char	ifname[WAN_IFNAME_SZ+1];

	/* Handle to serialization object used for read and writes */
	unsigned int 	ulUserReadWriteSerObj;

	/* Board index. */
	unsigned int	ulBoardId;

	/* Board type. */
	unsigned int	ulBoardType;
#endif
} tOCTPCIDRV_USER_PROCESS_CONTEXT, *tPOCTPCIDRV_USER_PROCESS_CONTEXT;


typedef struct wanec_config_ {
	int	max_channels;
	int	memory_chip_size;
	UINT32	debug_data_mode;
	PUINT8	imageData;	
	UINT32	imageSize;
	int	imageLast;
} wanec_config_t;

typedef struct wanec_param_ {
	char	key[MAX_PARAM_LEN];
	char	value[MAX_VALUE_LEN];
	UINT32	value1;
} wanec_param_t;

typedef struct wanec_chip_stats_ {
	int				reset;
	tOCT6100_CHIP_STATS		f_ChipStats;
	tOCT6100_CHIP_IMAGE_INFO	*f_ChipImageInfo;
} wanec_chip_stats_t;

typedef struct wanec_chan_stats_ {
	int				reset;
	tOCT6100_CHANNEL_STATS		f_ChannelStats;
} wanec_chan_stats_t;

#define MAX_MONITOR_DATA_LEN	1024
typedef struct wanec_chan_monitor_ {
	UINT32			remain_len;
	UINT32			data_len;
	UINT32			max_len;
	UINT8			data[MAX_MONITOR_DATA_LEN+1];
} wanec_chan_monitor_t;

typedef struct wanec_tone_config_ {
	UINT8	tone[WAN_MAX_TONE_LEN];	
	PUINT8	data;
	UINT32	size;
	UINT32	buffer_index;		/* value return by ec */
} wanec_tone_config_t;

typedef struct wanec_dtmf_config {
	u_int8_t	port;		/* SOUT or ROUT */
	u_int8_t	type;		/* PRESENT or STOP */
} wanec_dtmf_config_t;

#define MAX_EC_PLAYOUT_LEN	20
typedef struct wanec_playout_ {
	UINT32	index;
	UINT32	duration;
	BOOL	repeat;
	UINT32	repeat_count;
	UINT32	buffer_length;
	INT32	gaindb;
	
	CHAR	str[MAX_EC_PLAYOUT_LEN];
	UINT32	delay;
} wanec_playout_t;

typedef struct wan_ec_api_ {
	char		devname[WAN_DRVNAME_SZ+1];
	unsigned long	cmd;
	unsigned int	type;
	int		err;
	int		verbose;
	int		state;
	
	int		channel;
	unsigned long	channel_map;
	
	union {
#define u_info		u_ec.info		
#define u_config	u_ec.config		
#define u_chip_stats	u_ec.chip_stats		
#define u_chan_stats	u_ec.chan_stats		
#define u_chan_monitor	u_ec.chan_monitor	
#define u_tone_config	u_ec.tone_config	
#define u_dtmf_config	u_ec.dtmf_config
#define u_playout	u_ec.playout	
		struct info_ {
			unsigned int	max_channels;
		} info;
		wanec_config_t		config;
		wanec_chip_stats_t	chip_stats;
		wanec_chan_stats_t	chan_stats;
		wanec_chan_monitor_t	chan_monitor;
		wanec_tone_config_t	tone_config;
		wanec_dtmf_config_t	dtmf_config;
		wanec_playout_t		playout;
	} u_ec;
	int		param_no;
	wanec_param_t	param[2];
} wan_ec_api_t;


#if defined(WAN_KERNEL)

/* Critical bit map */
#define WAN_EC_BIT_TIMER_RUNNING	1
#define WAN_EC_BIT_TIMER_KILL 		2
#define WAN_EC_BIT_CRIT_CMD 		3
#define WAN_EC_BIT_CRIT_DOWN 		4
#define WAN_EC_BIT_CRIT_ERROR 	       	5
#define WAN_EC_BIT_CRIT 	       	6

#define WAN_EC_BIT_EVENT_DTMF 		4

#define WAN_EC_POLL_NONE		0x00
#define WAN_EC_POLL_INTR		0x01
#define WAN_EC_POLL_CHIPOPENPENDING	0x02

struct wan_ec_;
typedef struct wan_ec_dev_ {
	char		*name;
	char		devname[WAN_DRVNAME_SZ+1];
	char		ecdev_name[WAN_DRVNAME_SZ+1];
	int		ecdev_no;
	sdla_t		*card;
	
	u_int8_t	fe_media;
	u_int32_t	fe_lineno;
	u_int32_t	fe_max_channels;
	u_int32_t	fe_tdmv_law;
	u_int32_t	channel;
	int		state;

	u_int32_t	critical;
	wan_timer_t	timer;
	u_int8_t	poll_cmd;

	u_int32_t	events;			/* enable events map */
		
	struct wan_ec_	*ec;
	WAN_LIST_ENTRY(wan_ec_dev_)	next;
} wan_ec_dev_t;

typedef struct wan_ec_ {
	char		name[WAN_DRVNAME_SZ+1];
	int		usage;
	int		chip_no;
	int		state;
	int		ec_channels_no;	
	int		max_channels;		/* max number of ec channels (security) */
	void		*ec_dev;
	u_int32_t	intcount;
	u_int32_t	critical;

	//wan_spinlock_t	lock;
	u_int32_t	events;			/* enable events map */
	
	PUINT8				pImageData;
	int				imageLast;
	tOCT6100_CHIP_OPEN		f_OpenChip;

	tOCTPCIDRV_USER_PROCESS_CONTEXT	f_Context;
	tPOCT6100_INSTANCE_API		pChipInstance;
	tOCT6100_INTERRUPT_FLAGS	f_InterruptFlag;
	UINT32				*pEchoChannelHndl;
	PUINT32				pToneBufferIndexes;
	UINT32				ulDebugChannelHndl;
	INT				DebugChannel;
	UINT32				ulDebugDataMode;

	struct wan_ec_dev_		**pEcDevMap;
	WAN_LIST_HEAD(wan_ec_dev_head_, wan_ec_dev_)	ec_dev_head;
	WAN_LIST_ENTRY(wan_ec_)				next;
} wan_ec_t;

#if 0
typedef struct wanec_lip_reg 
{
	unsigned long init;
	
	void* (*reg) 	(void*, int);
	int (*unreg)	(void*, void*);
	
	int (*ioctl)	(void);
	int (*isr)	(void);

}wanec_lip_reg_t;
#endif

/* global interface functions */
void *wan_ec_config (void *pcard, int max_channels);
int wan_ec_remove(void*, void *pcard);
int wan_ec_ioctl(void*, struct ifreq*, void *pcard);
int wan_ec_dev_ioctl(void*, void *pcard);

u32 wan_ec_req_write(void*, u32 write_addr, u16 write_data);
u32 wan_ec_req_write_smear(void*, u32 addr, u16 data, u32 len);
u32 wan_ec_req_write_burst(void*, u32 addr, u16 *data, u32 len);
u32 wan_ec_req_read(void*, u32 addr, u16 *data);
u32 wan_ec_req_read_burst(void*, u32 addr, u16 *data, u32 len);

#endif

#endif /* __WANEC_LIP_H */
