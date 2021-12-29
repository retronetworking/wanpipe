#ifndef __WAN_OCT6100_H__
# define __WAN_OCT6100_H__


#if defined(__LINUX__)
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanpipe_common.h>
# include <linux/wanpipe_cfg.h>
# include <linux/if_wanpipe.h>
# include <linux/wanpipe.h>
# include <linux/sdla_ec.h>
#else
# include <net/wanpipe_defines.h>
# include <net/wanpipe_debug.h>
# include <net/wanpipe_common.h>
# include <net/wanpipe_cfg.h>
# include <net/wanpipe.h>
# include <net/sdla_ec.h>
#endif
#include "oct6100api/oct6100_api.h"
#include "oct6100_version.h"

#define WAN_EC_DIR	"/etc/wanpipe/wan_ec"
#define WAN_EC_BUFFERS	WAN_EC_DIR "/buffers"

#if 0
#define WAN_EC_SOCKET	WAN_EC_DIR "/wan_ec_socket"
#define WAN_EC_PID	WAN_EC_DIR "/wan_ec_pid"
#endif
#define WAN_EC_SOCKET	"/var/run/wan_ec.socket"
#define WAN_EC_PID	"/var/run/wan_ec.pid"
#define WAN_EC_DEBUG	"wan_ec_sangoma"

#define MAX_PARAM_LEN	50
#define MAX_VALUE_LEN	50

#define WAN_NUM_DTMF_TONES	32
#define WAN_NUM_PLAYOUT_TONES	16
 
enum {
	WAN_EC_CMD_NONE	= 0x00,
	WAN_EC_CMD_LOAD,
	WAN_EC_CMD_UNLOAD,
	WAN_EC_CMD_CONFIG,
	WAN_EC_CMD_ENABLE,
	WAN_EC_CMD_DISABLE,
	WAN_EC_CMD_BYPASS_ENABLE,
	WAN_EC_CMD_BYPASS_DISABLE,
	WAN_EC_CMD_MODE_NORMAL,
	WAN_EC_CMD_MODE_POWERDOWN,
	WAN_EC_CMD_STATS,
	WAN_EC_CMD_STATS_FULL,
	WAN_EC_CMD_PLAYOUT,
	WAN_EC_CMD_RELEASE,
	WAN_EC_CMD_RELEASE_ALL,
	WAN_EC_CMD_ACK,
	WAN_EC_CMD_POLL,
	WAN_EC_CMD_HELP,
	WAN_EC_CMD_HELP1,
	WAN_EC_CMD_HELP2,
	WAN_EC_CMD_TEST,
	WAN_EC_CMD_MONITOR,
	WAN_EC_CMD_MODIFY_CHANNEL,
	WAN_EC_CMD_DTMF_ENABLE,
	WAN_EC_CMD_DTMF_DISABLE
};
#define WAN_EC_CMD_DECODE(cmd)					\
	(cmd == WAN_EC_CMD_LOAD) ? "Load" :			\
	(cmd == WAN_EC_CMD_UNLOAD) ? "Unload" :			\
	(cmd == WAN_EC_CMD_CONFIG) ? "Config" :			\
	(cmd == WAN_EC_CMD_ENABLE) ? "Enable" :			\
	(cmd == WAN_EC_CMD_DISABLE) ? "Disable" :		\
	(cmd == WAN_EC_CMD_BYPASS_ENABLE) ? "Enable bypass" :	\
	(cmd == WAN_EC_CMD_BYPASS_DISABLE) ? "Disable bypass" :	\
	(cmd == WAN_EC_CMD_MODE_NORMAL) ? "Mode Normal" :	\
	(cmd == WAN_EC_CMD_MODE_POWERDOWN) ? "Mode PowerDown" :	\
	(cmd == WAN_EC_CMD_STATS) ? "Get stats" :		\
	(cmd == WAN_EC_CMD_STATS_FULL) ? "Get stats" :		\
	(cmd == WAN_EC_CMD_PLAYOUT) ? "Playout" :		\
	(cmd == WAN_EC_CMD_RELEASE) ? "Release" :		\
	(cmd == WAN_EC_CMD_RELEASE_ALL) ? "Release all" :	\
	(cmd == WAN_EC_CMD_POLL) ? "Poll" :			\
	(cmd == WAN_EC_CMD_ACK) ? "ACK" :			\
	(cmd == WAN_EC_CMD_MONITOR) ? "MONITOR" :		\
	(cmd == WAN_EC_CMD_MODIFY_CHANNEL) ? "MODIFY" :		\
	(cmd == WAN_EC_CMD_DTMF_ENABLE) ? "Enable DTMF" :	\
	(cmd == WAN_EC_CMD_DTMF_DISABLE) ? "Disable DTMF" :	\
					"Unknown"

#define WAN_EC_VERBOSE_EXTRA1		0x01
#define WAN_EC_VERBOSE_MASK_EXTRA1	0x01
#define WAN_EC_VERBOSE_EXTRA2		0x02
#define WAN_EC_VERBOSE_MASK_EXTRA2	0x03
#define PRINT(v, format,msg...)					\
	syslog(LOG_INFO, format, ##msg)
#define PRINT1(v,format,msg...)					\
	if (v & WAN_EC_VERBOSE_EXTRA1) syslog(LOG_INFO,format,##msg)
#define PRINT2(v,format,msg...)					\
	if (v & WAN_EC_VERBOSE_EXTRA2) syslog(LOG_INFO,format,##msg)

struct wan_ec_dev_;
typedef struct wan_ecd_ {
	char	name[WAN_DRVNAME_SZ+1];
	int	usage;
	int	dev;
	int	state;
	int	chip_no;
	int	max_channels;

	tOCTPCIDRV_USER_PROCESS_CONTEXT	f_Context;
	tPOCT6100_INSTANCE_API		pChipInstance;
	UINT32				ulApiInstanceSize;
	UINT32				*pEchoChannelHndl;
	UINT32				pToneBufferIndexes[WAN_NUM_DTMF_TONES];
	UINT32				ulDebugChannelHndl;
	INT				DebugChannel;
	UINT32				ulDebugDataMode;

	struct wan_ec_dev_		**pEcDevMap;
	WAN_LIST_HEAD(wan_ec_dev_head_, wan_ec_dev_)	ec_dev_head;
	WAN_LIST_ENTRY(wan_ecd_)			next;
} wan_ecd_t;

typedef struct wan_ec_dev_ {
	char		if_name[WAN_IFNAME_SZ+1];
	char		devname[WAN_DRVNAME_SZ+1];
	int		dev;
	unsigned char	fe_media;
	int		fe_lineno;
	int		fe_max_channels;
	int		fe_tdmv_law;

	wan_ecd_t	*ec;
	WAN_LIST_ENTRY(wan_ec_dev_)	next;
} wan_ec_dev_t;

#define MAX_CHANNELS_LEN	20
typedef struct wan_ecd_event_ {
/*	INT	first_channel;*/
/*	INT	last_channel;*/
	INT	channel;
	UINT32	channel_map;
	CHAR	channels[MAX_CHANNELS_LEN];
	INT	reset;
} wan_ecd_event_t;

#define MAX_PLAYOUT_LEN	20
typedef struct wan_ecd_event_playout_ {
	INT	channel;
	CHAR	str[MAX_PLAYOUT_LEN];
	INT	delay;
} wan_ecd_event_playout_t;

typedef struct wan_ecd_event_conf_ {
	UINT32	hndl;
} wan_ecd_event_conf_t;

typedef struct wan_ecd_chip_stats_ {
	tOCT6100_CHIP_STATS		f_ChipStats;
	tOCT6100_CHIP_IMAGE_INFO	f_ChipImageInfo;
} wan_ecd_chip_stats_t;

typedef struct wan_ecd_chan_stats_ {
	tOCT6100_CHANNEL_STATS	f_ChannelStats;
} wan_ecd_chan_stats_t;

#define MAX_MONITOR_DATA_LEN	1024
typedef struct wan_ecd_chan_monitor_ {
	UINT32			remain_len;
	UINT32			data_len;
	UINT32			max_len;
	UINT8			data[MAX_MONITOR_DATA_LEN+1];
} wan_ecd_chan_monitor_t;

typedef struct wan_ecd_param_ {
	char	key[MAX_PARAM_LEN];
	char	value[MAX_VALUE_LEN];
} wan_ecd_param_t;

typedef struct wan_ec_msg_ {
	char	if_name[WAN_IFNAME_SZ+1];
	char	devname[WAN_DRVNAME_SZ+1];
	
	int	verbose;
	int	cmd;
	int	err;
#define msg_event	u_sdata.event
#define msg_playout	u_sdata.playout
	union {
		wan_ecd_event_t		event;
		wan_ecd_event_playout_t	playout;
	} u_sdata;	/* send data */
#define msg_chip_stat		u_rdata.chip_stat
#define msg_chan_stat		u_rdata.chan_stat
#define msg_chan_monitor	u_rdata.chan_monitor
	union {
		wan_ecd_chip_stats_t	chip_stat;
		wan_ecd_chan_stats_t	chan_stat;
		wan_ecd_chan_monitor_t	chan_monitor;
	} u_rdata;	/* return data */
	int		param_no;
	wan_ecd_param_t	param[10];
} wan_ec_msg_t;

extern int wan_ecd_dev_ioctl( int, wan_ec_api_t* );

#endif /* __WAN_OCT6100_H__ */
