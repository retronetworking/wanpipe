#ifndef __WAN_ECMAIN_H__
# define __WAN_ECMAIN_H__


#if defined(__LINUX__)
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_common.h>
# include <linux/if_wanpipe.h>
# include <linux/wanpipe_cfg.h>
#elif defined(__WINDOWS__)
# include <windows.h>
# include <wanpipe_defines.h>
# include <wanpipe_common.h>
# include <wanpipe_cfg.h>
#else
# include <wanpipe_defines.h>
# include <wanpipe_cfg.h>
# include <wanpipe_common.h>
#endif

#define MAX_FILENAME_LEN		100

#define MAX_EC_CLIENT_CHANNELS_LEN	50
#define MAX_EC_CLIENT_PARAM_LEN		50
#define MAX_EC_CLIENT_VALUE_LEN		50

enum {
	WAN_EC_ACT_CMD = 0,
	WAN_EC_ACT_HELP,
	WAN_EC_ACT_HELP1,
	WAN_EC_ACT_TEST
};

typedef struct {
	char	devname[WAN_DRVNAME_SZ+1];
//	unsigned char	if_name[WAN_IFNAME_SZ+1];
	int		verbose;
	
	int		channel;
	unsigned long	channel_map;
//	char		channels[MAX_EC_CLIENT_CHANNELS_LEN];

	int		param_no;
	struct {
		char	key[MAX_EC_CLIENT_PARAM_LEN];
		char	value[MAX_EC_CLIENT_VALUE_LEN];
	} param[10];	
} wanec_client_t;

#endif /* __WAN_ECMAIN_H__ */
