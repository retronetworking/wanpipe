/*************************************************************
 * wanec_iface.c   WANPIPE Echo Canceller Layer (WANEC)
 *
 *
 *  
 * ===========================================================
 *
 * May 10 2006	Alex Feldman	Initial Versionr
 */

/*=============================================================
 * Includes
 */
#undef WAN_DEBUG_FUNC

#if defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe.h>
# include <wanpipe_ec_kernel.h>
# include <sdla_cdev.h>
#elif defined(__LINUX__)
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe.h>
# include <linux/if_wanpipe.h>
# include <linux/wanpipe_tdm_api.h>
# include <linux/wanpipe_ec_kernel.h>
#elif defined(__WINDOWS__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <wanpipe.h>
# include <wanpipe_tdm_api.h>
# include <wanpipe_ec_kernel.h>

# define DEBUG_HWEC_V1	DbgPrint
//#define DEBUG_HWEC_V1
#endif

#include "wanec_iface.h"

/*=============================================================
 * Definitions
 */
#define WAN_OCT6100_ENABLE_INTR_POLL

#define WAN_OCT6100_READ_LIMIT		0x10000

#if 0
# define DEBUG
#endif

/*=============================================================
 * Global Parameters
 */
#if defined(DEBUG)
static int	global_verbose = WAN_EC_VERBOSE_EXTRA1;
#else
static int	global_verbose = WAN_EC_VERBOSE_NONE;
#endif

WAN_LIST_HEAD(wan_ec_head_, wan_ec_) wan_ec_head = 
		WAN_LIST_HEAD_INITIALIZER(wan_ec_head);

wanec_iface_t	wanec_iface = 
{
	0,
	NULL,
	NULL,
	
	NULL,
	NULL,
	NULL,
	NULL
};

static int	wan_ec_no = 0;

static unsigned char wpec_fullname[]="WANPIPE(tm) WANEC Layer";
static unsigned char wpec_copyright[]="(c) 1995-2006 Sangoma Technologies Inc.";
/*=============================================================
 * Function definition
 */

#if defined(__LINUX__)
extern int wanec_create_dev(void);
extern int wanec_remove_dev(void);
#elif defined(__WINDOWS__)
extern int wanec_create_dev(void);
extern int wanec_remove_dev(void);
extern void* get_wan_ec_ptr(sdla_t *card);
extern void set_wan_ec_ptr(sdla_t *card, IN void *wan_ec_ptr);
#endif

int register_wanec_iface (wanec_iface_t *iface);
void unregister_wanec_iface (void);

extern int wanec_fe2ec_channel(wan_ec_dev_t*, int);

extern int wanec_ChipOpenPrep(wan_ec_dev_t*, wan_ec_api_t*);
extern int wanec_ChipOpen(wan_ec_dev_t*, int verbose);
extern int wanec_ChipOpen_OLD(wan_ec_dev_t*, wan_ec_api_t*);
extern int wanec_ChipClose(wan_ec_dev_t*, int verbose);
extern int wanec_ChipStats(wan_ec_dev_t*, wan_ec_api_t*, int);

extern int wanec_ChannelOpen(wan_ec_dev_t*, wan_ec_api_t *ec_api);
extern int wanec_ChannelClose(wan_ec_dev_t*, wan_ec_api_t *ec_api, int);
extern int wanec_ChannelModify(wan_ec_dev_t*, INT, UINT32, wan_ec_api_t*, int verbose);
extern int wanec_ChannelStats(wan_ec_dev_t*, INT channel, wan_ec_api_t *ec_api, int reset);

extern int wanec_TonesEnable(wan_ec_t *ec, int channel, unsigned char, int verbose);
extern int wanec_TonesDisable(wan_ec_t *ec, int channel, unsigned char, int verbose);

extern int wanec_DebugChannel(wan_ec_t *ec, INT channel, int verbose);
extern int wanec_DebugGetData(wan_ec_t *ec, wan_ec_api_t *ec_api);

extern int wanec_BufferLoad(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
extern int wanec_BufferUnload(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
extern int wanec_BufferPlayoutAdd(wan_ec_t *ec, int channel, wan_ec_api_t *ec_api);
extern int wanec_BufferPlayoutStart(wan_ec_t *ec, int channel, wan_ec_api_t *ec_api);
extern int wanec_BufferPlayoutStop(wan_ec_t *ec, int channel, wan_ec_api_t *ec_api);

extern int wanec_EventTone(wan_ec_t *ec, int verbose);
extern int wanec_ISR(wan_ec_t *ec, int verbose);

static int wanec_config(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_release(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api, int verbose);
static int wanec_channel_open(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_modify_channel(wan_ec_dev_t *ec_dev, int fe_chan, u32 cmd, int verbose);
static int wanec_modify(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_modify_mode(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_modify_bypass(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_bypass(wan_ec_dev_t *ec_dev, int fe_channel, int enable, int verbose);

static wan_ec_dev_t *wanec_search(char *devname);

static int wanec_enable(void *pcard, int enable, int channel);
static int wanec_poll(void *arg, void *pcard);

#if defined(__FreeBSD__) || defined(__OpenBSD__)
int wanec_ioctl(void *sc, void *data);
#elif defined(__LINUX__)
int wanec_ioctl(unsigned int cmd, void *data);
#endif

int wan_ec_write_internal_dword(wan_ec_dev_t *ec_dev, u32 addr1, u32 data);
int wan_ec_read_internal_dword(wan_ec_dev_t *ec_dev, u32 addr1, u32 *data);

int wanec_init(void*);
int wanec_exit(void*);


/*****************************************************************************/

static u32 convert_addr(u32 addr)
{
	addr &= 0xFFFF;
	switch(addr){
	case 0x0000:
		return 0x60;
	case 0x0002:
		return 0x64;
	case 0x0004:
		return 0x68;
	case 0x0008:
		return 0x70;
	case 0x000A:
		return 0x74;
	}
	return 0x00;
}

/*===========================================================================*\
  ReadInternalDword
\*===========================================================================*/
int wan_ec_read_internal_dword(wan_ec_dev_t *ec_dev, u32 addr1, u32 *data)
{
	sdla_t	*card = NULL;
	u32	addr;
	int err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	card = ec_dev->card;
	addr = convert_addr(addr1);
	if (addr == 0x00){
		DEBUG_EVENT("%s: %s:%d: Internal Error (EC off %X)\n",
				card->devname,
				__FUNCTION__,__LINE__,
				addr1);
		return -EINVAL;
	}

	err = card->hw_iface.bus_read_4(card->hw, addr, data);
	
	WP_DELAY(5);
	return err;
}


/*===========================================================================*\
  WriteInternalDword
\*===========================================================================*/
int wan_ec_write_internal_dword(wan_ec_dev_t *ec_dev, u32 addr1, u32 data)
{
	sdla_t	*card = NULL;
	u32	addr;
	int	err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	card = ec_dev->card;
	addr = convert_addr(addr1);
	if (addr == 0x00){
		DEBUG_EVENT("%s: %s:%d: Internal Error (EC off %X)\n",
				card->devname,
				__FUNCTION__,__LINE__,
				addr1);
		return -EINVAL;
	}


	err = card->hw_iface.bus_write_4(card->hw, addr, data);	

	WP_DELAY(5);
	return err;
}


static int wanec_reset(wan_ec_dev_t *ec_dev, int reset)
{
	sdla_t		*card;
	wan_ec_t	*ec;
	int		err = -EINVAL;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	ec	= ec_dev->ec;
	card	= ec_dev->card;
	if (card->wandev.hwec_reset){
		/* reset=1 - Set HW EC reset
		** reset=0 - Clear HW EC reset */
		err = card->wandev.hwec_reset(card, reset);
		if (!err){
			if (reset){
				ec->state = WAN_OCT6100_STATE_RESET;
			}else{
				ec->state = WAN_OCT6100_STATE_READY;
			}
		}
	}
	return err;
}

static int wanec_enable(void *pcard, int enable, int fe_channel)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_ec_dev_t	*ec_dev = NULL;
	int err;

	ec_dev = 
#if defined(__WINDOWS__)
		card->ec_dev_ptr;
#else
		wanec_search(card->devname);
#endif
	WAN_ASSERT(ec_dev == NULL);

#if defined(WANEC_BYDEFAULT_NORMAL)
	WAN_ASSERT(ec_dev->ec == NULL);
	wan_spin_lock(&ec_dev->ec->lock);
	err=wanec_bypass(ec_dev, fe_channel, enable, 0);
	wan_spin_unlock(&ec_dev->ec->lock);

	return err;
#else
	return wanec_modify_channel(
			ec_dev,
			fe_channel,
			(enable) ? WAN_EC_CMD_ENABLE : WAN_EC_CMD_DISABLE, 
			0);
#endif
}

static int wanec_bypass(wan_ec_dev_t *ec_dev, int fe_channel, int enable, int verbose)
{
	wan_ec_t	*ec = NULL;
	sdla_t		*card = NULL;
	int		err = -ENODEV;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	ec = ec_dev->ec;	
	card = ec_dev->card;

	PRINT1(verbose,
	"%s: %s bypass mode for channel %d (%lX)!\n",
				card->devname,
				(enable) ? "Enable" : "Disable",
				fe_channel,
				card->wandev.ec_map);

	if (card->wandev.hwec_enable == NULL){
		DEBUG_EVENT("%s: Undefined HW EC callback function!\n",
					ec->name);
		return -ENODEV;
	}
	if (enable){
		if (!wan_test_bit(fe_channel, &card->wandev.ec_map)){
			if (ec->ec_active >= ec->max_channels){
				DEBUG_EVENT(
				"%s: Exceeded maximum number of Echo Canceller channels (max=%d)!\n",
					ec->name,
					ec->max_channels);
				return -ENODEV;
			}	
		}else{
			/* Already enabled */
			return 0;
		}
	}else{
		if (!wan_test_bit(fe_channel, &card->wandev.ec_map)){
			/* Already disabled */
			return 0;
		}
	
	}
	err = card->wandev.hwec_enable(card, enable, fe_channel);
	if (!err){
		if (enable){
			ec->ec_active++;
		}else{
			if (ec->ec_active) ec->ec_active--;
		}
	}else{
		PRINT1(verbose, 
		"%s: HWEC option is not enable for the channel %d (%lX)!\n",
				ec->name, fe_channel, card->wandev.ec_enable_map);
		return err;
	}
	return err;
}

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static void wanec_timer(void* p_ec_dev)
#elif defined(__WINDOWS__)
static void wanec_timer(IN PKDPC Dpc, void* p_ec_dev, void* arg2, void* arg3)
#else
static void wanec_timer(unsigned long p_ec_dev)
#endif
{
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)p_ec_dev;
	sdla_t		*card = NULL;

	WAN_ASSERT1(ec_dev == NULL);
	WAN_ASSERT1(ec_dev->card == NULL);
	card = (sdla_t*)ec_dev->card;

	if (wan_test_bit(WAN_EC_BIT_TIMER_KILL,(void*)&ec_dev->critical)){
		wan_clear_bit(WAN_EC_BIT_TIMER_RUNNING,(void*)&ec_dev->critical);
		return;
	}
	/*WAN_ASSERT1(wandev->te_enable_timer == NULL); */
	/* Enable hardware interrupt for TE1 */

	if (card->wandev.ec_enable_timer){
		card->wandev.ec_enable_timer(card);
	}else{
		wanec_poll(ec_dev, card);
	}
	return;
}

/*
 ******************************************************************************
 *				wanec_enable_timer()	
 *
 * Description: Enable software timer interrupt in delay ms.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static void wanec_enable_timer(wan_ec_dev_t* ec_dev, u_int8_t cmd, u_int32_t delay)
{
	sdla_t	*card = NULL;

	WAN_ASSERT1(ec_dev == NULL);
	WAN_ASSERT1(ec_dev->card == NULL);
	card = (sdla_t*)ec_dev->card;

#if defined (__WINDOWS__)
	if(KeGetCurrentIrql() > DISPATCH_LEVEL){
		/*	May get here on AFT card because front end interrupt
			is handled inside ISR not in DPC as on S514.
			The KeSetTimer() function is illegal inside ISR,
			so queue 'front_end_dpc_obj' DPC and this routine
			will be called again from xilinx_front_end_dpc().
		*/
		card->xilinx_fe_dpc.te_timer_delay = delay;
		ec_dev->poll_cmd = (u_int8_t)cmd;

		if(KeInsertQueueDpc(&card->front_end_dpc_obj, NULL,
			(PVOID)ENABLE_HWEC_TIMER) == FALSE){

			DEBUG_HWEC("Failed to queue 'front_end_dpc_obj'!\n");
		}else{
			DEBUG_HWEC("Successfully queued 'front_end_dpc_obj'.\n");
		}
		return;
	}/* if() */
#endif
	if (wan_test_bit(WAN_EC_BIT_TIMER_KILL,(void*)&ec_dev->critical)){
		wan_clear_bit(WAN_EC_BIT_TIMER_RUNNING, (void*)&ec_dev->critical);
		return;
	}
	
	if (wan_test_bit(WAN_EC_BIT_TIMER_RUNNING,(void*)&ec_dev->critical)){
		if (ec_dev->poll_cmd == cmd){
			/* Just ignore current request */
			return;
		}
		DEBUG_EVENT("%s: WAN_EC_BIT_TIMER_RUNNING: new_cmd=%X curr_cmd=%X\n",
					ec_dev->name,
					cmd,
					ec_dev->poll_cmd);
		return;
	}

	wan_set_bit(WAN_EC_BIT_TIMER_RUNNING,(void*)&ec_dev->critical);
	ec_dev->poll_cmd=cmd;
	wan_add_timer(&ec_dev->timer, delay * HZ / 1000);
	return;	
}

wan_ec_dev_t *wanec_search(char *devname)
{
	wan_ec_t	*ec;
	wan_ec_dev_t	*ec_dev = NULL;

	WAN_LIST_FOREACH(ec, &wan_ec_head, next){
		WAN_LIST_FOREACH(ec_dev, &ec->ec_dev_head, next){
			if (strcmp(ec_dev->devname, devname) == 0){
				return ec_dev;
			}
		}
	}
	return NULL;
}

static int wanec_config(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	switch(ec->state){
	case WAN_OCT6100_STATE_RESET:
	case WAN_OCT6100_STATE_READY:
		break;
	case WAN_OCT6100_STATE_CHIP_OPEN:
	case WAN_OCT6100_STATE_CHIP_OPEN_PENDING:
	case WAN_OCT6100_STATE_CHIP_READY:
		DEBUG_HWEC(
		"%s: Echo Canceller %s chip is %s!\n",
				ec_api->devname, ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		break;
	default:
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_api->devname, ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		ec_api->err	= WAN_EC_API_RC_INVALID_STATE;
		return 0;
	}

	if (ec->state == WAN_OCT6100_STATE_RESET){
		err = wanec_reset(ec_dev, 0);
		if (err) return err;
	}

	if (ec->state == WAN_OCT6100_STATE_READY){

		if (wanec_ChipOpenPrep(ec_dev, ec_api)){
			wanec_reset(ec_dev, 1);
			return -EINVAL;
		}
		ec->imageLast	= ec_api->u_config.imageLast;
		ec->state	= WAN_OCT6100_STATE_CHIP_OPEN_PENDING;
		wanec_enable_timer(ec_dev, WAN_EC_POLL_CHIPOPENPENDING, 10);
	}
	ec_dev->state = ec->state;
	return 0;
}

static int wanec_channel_open(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t		*ec = NULL;
	unsigned int	ec_chan, fe_chan;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	switch(ec->state){
	case WAN_OCT6100_STATE_CHIP_OPEN:
		break;
	case WAN_OCT6100_STATE_CHIP_READY:
		DEBUG_HWEC(
		"%s: Echo Canceller %s chip is %s!\n",
				ec_api->devname, ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		break;
	default:
		PRINT1(ec_api->verbose,
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_api->devname, ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		ec_api->err	= WAN_EC_API_RC_INVALID_STATE;
		return 0;
	}

	if (ec->state == WAN_OCT6100_STATE_CHIP_OPEN){

		/* Open all channels */	
		if (wanec_ChannelOpen(ec_dev, ec_api)){
			wanec_ChipClose(ec_dev, ec_api->verbose);
			wanec_reset(ec_dev, 1);
			return -EINVAL;
		}
		ec->state = WAN_OCT6100_STATE_CHIP_READY;
	}
	ec_dev->state = ec->state;

	/* EC_DEV_MAP */
	for(fe_chan=0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
		ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
		ec->pEcDevMap[ec_chan] = ec_dev;
	}
	return 0;
}

int wanec_release(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api, int verbose)
{
	wan_ec_t	*ec = NULL;
	wan_ec_dev_t	*ec_dev_tmp = NULL;
	unsigned int		fe_chan, ec_chan, err = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	
	switch(ec->state){
	case WAN_OCT6100_STATE_READY:
	case WAN_OCT6100_STATE_CHIP_OPEN:
	case WAN_OCT6100_STATE_CHIP_OPEN_PENDING:
	case WAN_OCT6100_STATE_CHIP_READY:
		break;
	case WAN_OCT6100_STATE_RESET:
		return 0;
	default:
		PRINT1(verbose,
		"%s: WARNING: Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return 0;
	}

#if defined(__WINDOWS__)
	//for TDMV API there is only only one device created.
	//So 'release' request should simply go ahead.
	FUNC_DEBUG();
#else
	WAN_LIST_FOREACH(ec_dev_tmp, &ec->ec_dev_head, next){
		if (ec_dev_tmp != ec_dev){
			if (ec_dev_tmp->state == WAN_OCT6100_STATE_CHIP_READY){
				/* This EC device is still connected */
				ec->f_Context.ec_dev	= ec_dev_tmp;
				strlcpy(ec->f_Context.devname, ec_dev_tmp->devname, WAN_DRVNAME_SZ);
				break;
			}
		} 
	}

#endif

	for(fe_chan = 0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
		ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
		if (ec->pEcDevMap){
			ec->pEcDevMap[ec_chan] = NULL;
		}
	}
	ec_dev->state = WAN_OCT6100_STATE_RESET;
	if (ec_dev_tmp){
		/* EC device is still in used */
		return 0;
	}

	/* EC device is not in used anymore.
	** Close all channels and chip */
	if (ec->state == WAN_OCT6100_STATE_CHIP_READY){
		if (wanec_ChannelClose(ec_dev, ec_api, verbose)){
			return EINVAL;
		}
		ec->state = WAN_OCT6100_STATE_CHIP_OPEN;
	}
	if (ec->state == WAN_OCT6100_STATE_CHIP_OPEN){
		if (wanec_ChipClose(ec_dev, verbose)){
			return EINVAL;
		}
		ec->state = WAN_OCT6100_STATE_READY;
	}

	if (ec->state == WAN_OCT6100_STATE_CHIP_OPEN_PENDING){
		ec->state = WAN_OCT6100_STATE_READY;
	}
	if (ec->state == WAN_OCT6100_STATE_READY){
		err = wanec_reset(ec_dev, 1);
		if (err){
			return EINVAL;
		}
	}
	return 0;
}

static int wanec_modify_channel(wan_ec_dev_t *ec_dev, int fe_chan, u32 cmd, int verbose)
{
	wan_ec_t	*ec = NULL;
	sdla_t		*card = NULL;
	u_int32_t	ec_chan = 0; 
	int		err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	ec = ec_dev->ec;
	card = ec_dev->card;

	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT1(verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	/* Enable Echo cancelation on Oct6100 */
	PRINT1(verbose,
	"%s: %s Echo Canceller on channel %d ...\n",
			ec_dev->devname,
			(cmd == WAN_EC_CMD_ENABLE) ? "Enable" : "Disable",
			fe_chan);
	ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
	if (cmd == WAN_EC_CMD_ENABLE){
		err = wanec_ChannelModify(
					ec_dev,
					ec_chan,
					cOCT6100_ECHO_OP_MODE_NORMAL,
					NULL,
					verbose);
		if (err){
			return WAN_EC_API_RC_FAILED;
		}	
			
		/* Change rx/tx traffic through Oct6100 */
		if (wanec_bypass(ec_dev, fe_chan, 1, verbose)){
			return WAN_EC_API_RC_FAILED;
		}
	}else{
		/* Change rx/tx traffic through Oct6100 */
		if (wanec_bypass(ec_dev, fe_chan, 0, verbose)){
			return WAN_EC_API_RC_FAILED;
		}
			
		err = wanec_ChannelModify(
					ec_dev,
					ec_chan,
					cOCT6100_ECHO_OP_MODE_POWER_DOWN,
					NULL,
					verbose);
		if (err){
			return WAN_EC_API_RC_FAILED;
		}
	}

	return 0;
}


static int wanec_modify(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	sdla_t		*card = NULL;
	u_int32_t	cmd = ec_api->cmd;
	u_int32_t	fe_chan = 0; 
#if 0
	u_int32_t	ec_chan = 0; 
#endif
	int		err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	ec = ec_dev->ec;
	card = ec_dev->card;

	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT1(ec_api->verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_api->channel_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->channel_map = 0l;
		for (fe_chan = 0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
			ec_api->channel_map |= (1 << fe_chan);
		}
	}else{
		if (ec_dev->fe_media == WAN_MEDIA_T1 ||
		    ec_dev->fe_media == WAN_MEDIA_FXOFXS){
			ec_api->channel_map = ec_api->channel_map >> 1;
		}
	}

	/* Enable Echo cancelation on Oct6100 */
	PRINT1(ec_api->verbose,
	"%s: %s Echo Canceller on channel(s) map=0x%08lX ...\n",
			ec_dev->devname,
			(cmd == WAN_EC_CMD_ENABLE) ? "Enable" : "Disable",
			ec_api->channel_map);
	/*for(chan = fe_first; chan <= fe_last; chan++){*/
	for(fe_chan=0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
		if (!(ec_api->channel_map & (1 << fe_chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0) continue;
#if 1
		err = wanec_modify_channel(ec_dev, fe_chan, cmd, ec_api->verbose);
#else
		ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
		if (cmd == WAN_EC_CMD_ENABLE){
			err = wanec_ChannelModify(
						ec_dev,
						ec_chan,
						cOCT6100_ECHO_OP_MODE_NORMAL,
						ec_api);
			if (err){
				return WAN_EC_API_RC_FAILED;
			}	

			/* Change rx/tx traffic through Oct6100 */
			if (wanec_bypass(ec_dev, fe_chan, 1, ec_api->verbose)){
				return WAN_EC_API_RC_FAILED;
			}
		}else{
			/* Change rx/tx traffic through Oct6100 */
			if (wanec_bypass(ec_dev, fe_chan, 0, ec_api->verbose)){
				return WAN_EC_API_RC_FAILED;
			}

			err = wanec_ChannelModify(
						ec_dev,
						ec_chan,
						cOCT6100_ECHO_OP_MODE_POWER_DOWN,
						ec_api);
			if (err){
				return WAN_EC_API_RC_FAILED;
			}
		}
#endif
	}

	return 0;
}

static int wanec_modify_mode(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	u_int32_t	cmd = ec_api->cmd;
	u_int32_t	chan, ec_channel;
	int		err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT1(ec_api->verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_api->channel_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->channel_map = 0l;
		for (chan = 0; chan < ec_dev->fe_max_channels; chan++){
			ec_api->channel_map |= (1 << chan);
		}
	}else{
		if (ec_dev->fe_media == WAN_MEDIA_T1 ||
		    ec_dev->fe_media == WAN_MEDIA_FXOFXS){
			ec_api->channel_map = ec_api->channel_map >> 1;
		}
	}
	/* Enable/Disable Normal mode on Oct6100 */
	PRINT1(ec_api->verbose,
	"%s: %s Echo Canceller mode on channel(s) map=0x%08lX ...\n",
			ec_dev->devname,
			(cmd == WAN_EC_CMD_MODE_NORMAL) ? "Enable" :
			(cmd == WAN_EC_CMD_MODE_POWERDOWN) ? "Disable" : "Modify",
			ec_api->channel_map);
	for(chan=0; chan < ec_dev->fe_max_channels; chan++){
		if (!(ec_api->channel_map & (1 << chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && chan == 0) continue;
		ec_channel = wanec_fe2ec_channel(ec_dev, chan);
		switch(cmd){
		case WAN_EC_CMD_MODE_NORMAL:
			err = wanec_ChannelModify(
						ec_dev,
						ec_channel,
						cOCT6100_ECHO_OP_MODE_NORMAL,
						ec_api,
						ec_api->verbose);
			break;
		case WAN_EC_CMD_MODE_POWERDOWN:
			err = wanec_ChannelModify(
						ec_dev,
						ec_channel,
						cOCT6100_ECHO_OP_MODE_POWER_DOWN,
						ec_api,
						ec_api->verbose);
			break;
		default:
			err = wanec_ChannelModify(
						ec_dev,
						ec_channel,
						cOCT6100_KEEP_PREVIOUS_SETTING,
						ec_api,
						ec_api->verbose);
			break;
		}
		if (err){
			return WAN_EC_API_RC_FAILED;
		}	
	}
	return 0;
}

static int wanec_modify_bypass(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	sdla_t		*card = NULL;
	unsigned int		chan, fe_chan = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	ec = ec_dev->ec;
	card = ec_dev->card;

	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT1(ec_api->verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}
	if (ec_api->channel_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->channel_map = 0l;
		for (chan = 0; chan < ec_dev->fe_max_channels; chan++){
			ec_api->channel_map |= (1 << chan);
		}
	}else{
		if (ec_dev->fe_media == WAN_MEDIA_T1 ||
		    ec_dev->fe_media == WAN_MEDIA_FXOFXS){
			ec_api->channel_map = ec_api->channel_map >> 1;
		}
	}
	/* Enable/Disable bypass mode on Oct6100 */
	PRINT1(ec_api->verbose,
	"%s: %s Bypass mode on channel(s) map=0x%08lX ...\n",
			ec_dev->devname,
			(ec_api->cmd == WAN_EC_CMD_BYPASS_ENABLE) ? "Enable" : "Disable",
			ec_api->channel_map);	
	for(chan = 0; chan < ec_dev->fe_max_channels; chan++){
		if (!(ec_api->channel_map & (1 << chan))){
			continue;
		}
		fe_chan = chan;
		if (ec_api->cmd == WAN_EC_CMD_BYPASS_ENABLE){
			/* Change rx/tx traffic through Oct6100 */
			if (wanec_bypass(ec_dev, fe_chan, 1, ec_api->verbose)){
				return WAN_EC_API_RC_FAILED;
			}
		}else{
			/* Change rx/tx traffic through Oct6100 */
			if (wanec_bypass(ec_dev, fe_chan, 0, ec_api->verbose)){
				return WAN_EC_API_RC_FAILED;
			}
		}
	}
	return 0;
}


static int wanec_modify_dtmf(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	sdla_t		*card = NULL;
	unsigned int		fe_chan, ec_channel;
	int		err = WAN_EC_API_RC_OK;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	ec = ec_dev->ec;
	card = ec_dev->card;
	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT1(ec_api->verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_api->channel_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->channel_map = 0l;
		for (fe_chan = 0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
			ec_api->channel_map |= (1 << fe_chan);
		}
	}else{
		if (ec_dev->fe_media == WAN_MEDIA_T1 ||
		    ec_dev->fe_media == WAN_MEDIA_FXOFXS){
			ec_api->channel_map = ec_api->channel_map >> 1;
		}
	}
	
	if (!ec_api->channel_map){
		return WAN_EC_API_RC_NOACTION;
	}
	/* Enable/Disable Normal mode on Oct6100 */
	PRINT1(ec_api->verbose,
	"%s: %s Echo Canceller DTMF on channel(s) map=0x%08lX ...\n",
			ec_dev->devname,
			(ec_api->cmd == WAN_EC_CMD_DTMF_ENABLE) ? "Enable" :
			(ec_api->cmd == WAN_EC_CMD_DTMF_DISABLE) ? "Disable" :
								"Unknown",
			ec_api->channel_map);
	for(fe_chan=0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
		if (!(ec_api->channel_map & (1 << fe_chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0) continue;
		ec_channel = wanec_fe2ec_channel(ec_dev, fe_chan);
		switch(ec_api->cmd){
		case WAN_EC_CMD_DTMF_ENABLE:
			if (wanec_bypass(ec_dev, fe_chan, 1, ec_api->verbose)){
				return WAN_EC_API_RC_FAILED;
			}
			err = wanec_TonesEnable(	
					ec,
					ec_channel,
					ec_api->u_dtmf_config.port,
					ec_api->verbose);
			break;
		case WAN_EC_CMD_DTMF_DISABLE:
			err = wanec_TonesDisable(
					ec,
					ec_channel,
					ec_api->u_dtmf_config.port,
					ec_api->verbose);
			if (wanec_bypass(ec_dev, fe_chan, 0, ec_api->verbose)){
				return WAN_EC_API_RC_FAILED;
			}
			break;
		default:
			err = WAN_EC_API_RC_INVALID_CMD;
			break;
		}
		if (err){
			return WAN_EC_API_RC_FAILED;
		}	
	}
	return err;
}

static int wanec_stats(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	unsigned int		fe_chan, ec_channel, err = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT1(ec_api->verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_dev->fe_media == WAN_MEDIA_T1 || ec_dev->fe_media == WAN_MEDIA_FXOFXS){
		ec_api->channel_map = ec_api->channel_map >> 1; 
	}
	PRINT1(ec_api->verbose,
	"%s: Read Echo Canceller stats on channel(s) map=0x%08lX reset %d...\n",
			ec_dev->devname,
			ec_api->channel_map,
			(ec_api->channel_map) ? 
				ec_api->u_chan_stats.reset:ec_api->u_chip_stats.reset);
	if (wanec_ISR(ec, ec_api->verbose)){
		return WAN_EC_API_RC_FAILED;
	}
	if (ec_api->channel_map){
		for(fe_chan=0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
			if (!(ec_api->channel_map & (1 << fe_chan))){
				continue;
			}
			if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0){
				continue;
			}
			ec_channel = wanec_fe2ec_channel(ec_dev, fe_chan);
			err = wanec_ChannelStats(
						ec_dev,
						ec_channel,
						ec_api,
						ec_api->u_chan_stats.reset);
			if (err){
				return WAN_EC_API_RC_FAILED;
			}
		}
	}else{
		wanec_ChipStats(ec_dev, ec_api, ec_api->u_chip_stats.reset);	
	}
			
	return 0;
}

static int wanec_monitor(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	unsigned int		channel = ec_api->channel,
			ec_channel;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT1(ec_api->	verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	/* Sanity check from channel selection */
	if (((ec_dev->fe_media == WAN_MEDIA_T1) && (channel > ec_dev->fe_max_channels)) || 
	    ((ec_dev->fe_media == WAN_MEDIA_E1) && (channel >= ec_dev->fe_max_channels))){ 
		DEBUG_EVENT(
		"ERROR: %s: Channel number %d out of range!\n",
				ec_dev->devname,
				channel);
		return WAN_EC_API_RC_INVALID_CHANNELS;
	}

	if (channel){
		if (ec_dev->fe_media == WAN_MEDIA_T1 || ec_dev->fe_media == WAN_MEDIA_FXOFXS) channel--; 
		ec_channel = wanec_fe2ec_channel(ec_dev, channel);
		if (wanec_DebugChannel(ec, ec_channel, ec_api->verbose)){
			return WAN_EC_API_RC_FAILED;
		}
	}else{
		wanec_DebugGetData(ec, ec_api);
	}
	return 0;
}

static int wanec_tone(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT1(ec_api->verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}
	if (ec_api->cmd == WAN_EC_CMD_TONE_LOAD){
		err = wanec_BufferLoad(ec_dev,  ec_api);
	}else{
		err = wanec_BufferUnload(ec_dev, ec_api);
	}
	if (err){
		return WAN_EC_API_RC_FAILED;
	}
	return 0;
}

static int wanec_playout(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		ec_channel;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT1(ec_api->verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_dev->fe_media == WAN_MEDIA_E1 && ec_api->channel == 0){
		return WAN_EC_API_RC_NOACTION;
	}
	if (((ec_dev->fe_media == WAN_MEDIA_T1) && ((unsigned int)ec_api->channel > ec_dev->fe_max_channels)) || 
	    ((ec_dev->fe_media == WAN_MEDIA_E1) && ((unsigned int)ec_api->channel >= ec_dev->fe_max_channels))){ 
		DEBUG_EVENT(
		"ERROR: %s: Channel number %d out of range!\n",
				ec_dev->devname,
				ec_api->channel);
		return WAN_EC_API_RC_INVALID_CHANNELS;
	}

	if (ec_dev->fe_media == WAN_MEDIA_T1 || ec_dev->fe_media == WAN_MEDIA_FXOFXS)
		ec_api->channel--; 
	ec_channel = wanec_fe2ec_channel(ec_dev, ec_api->channel);

	switch(ec_api->cmd){	
	case WAN_EC_CMD_PLAYOUT_START:
		if (wanec_BufferPlayoutAdd(ec, ec_channel, ec_api)){
			return WAN_EC_API_RC_FAILED;
		}
		if (wanec_BufferPlayoutStart(ec, ec_channel, ec_api)){
			wanec_BufferPlayoutStop(ec, ec_channel, ec_api);
			return WAN_EC_API_RC_FAILED;
		}
		break;
	case WAN_EC_CMD_PLAYOUT_STOP:
		wanec_BufferPlayoutStop(ec, ec_channel, ec_api);
		break;
	}
		
	return 0;
}

#if defined(__FreeBSD__) || defined(__OpenBSD__)
int wanec_ioctl(void *sc, void *data)
#elif defined(__LINUX__)
int wanec_ioctl(unsigned int cmd, void *data)
#elif defined(__WINDOWS__)
int wanec_ioctl(void *data, void *pcard)
#endif
{
	wan_ec_api_t	*ec_api = NULL;
	wan_ec_t	*ec = NULL;
	wan_ec_dev_t	*ec_dev = NULL;
	int		err = 0;
#if defined(__WINDOWS__)
	sdla_t	*card = (sdla_t*)pcard;
#endif

	WAN_DEBUG_FUNC_START;
	
#if defined(__LINUX__)
	ec_api = wan_malloc(sizeof(wan_ec_api_t));
	if (ec_api == NULL){
		DEBUG_EVENT(
    		"wanec: Failed allocate memory (%d) [%s:%d]!\n",
				sizeof(wan_ec_api_t),
				__FUNCTION__,__LINE__);
		return -EINVAL;
	}
	err = WAN_COPY_FROM_USER(
				ec_api,
				data,
				sizeof(wan_ec_api_t));
	if (err){
		DEBUG_EVENT(
       		"wanec: Failed to copy data from user space [%s:%d]!\n",
				__FUNCTION__,__LINE__);
		wan_free(ec_api);
		return -EINVAL;
	}
#else
	ec_api = (wan_ec_api_t*)data;
#endif

#if defined(DEBUG)
	ec_api->verbose |= (WAN_EC_VERBOSE_EXTRA1|WAN_EC_VERBOSE_EXTRA2);
#endif

	ec_dev = 
#if defined(__WINDOWS__)	
		card->ec_dev_ptr;
#else
		wanec_search(ec_api->devname);
#endif
	if (ec_dev == NULL){
		PRINT1(ec_api->verbose,
		"%s: Failed to find device [%s:%d]!\n",
				ec_api->devname, __FUNCTION__,__LINE__);
		ec_api->err = WAN_EC_API_RC_INVALID_DEV;
		goto wanec_ioctl_done;
	}
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	wan_spin_lock(&ec->lock);

	if (wan_test_bit(WAN_EC_BIT_CRIT_DOWN, &ec_dev->critical)){
		PRINT1(ec_api->verbose,
		"%s: Echo Canceller device is down!\n",
				ec_api->devname);
		ec_api->err = WAN_EC_API_RC_INVALID_DEV;
		goto wanec_ioctl_done;	
	}
	if (wan_test_and_set_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical)){
		PRINT1(ec_api->verbose,
		"%s: Echo Canceller is busy!\n",
				ec_api->devname);
		ec_api->err = WAN_EC_API_RC_BUSY;
		goto wanec_ioctl_done;	
	}
	PRINT2(ec_api->verbose,
	"%s: WPEC_LIP IOCTL: %s\n",
			ec_api->devname, WAN_EC_CMD_DECODE(ec_api->cmd));
	ec_api->err = WAN_EC_API_RC_OK;
	switch(ec_api->cmd){
	case WAN_EC_CMD_GETINFO:
		ec_api->u_info.max_channels	= ec->max_channels;
		ec_api->state			= ec->state;
		break;
	case WAN_EC_CMD_CONFIG:
		err = wanec_config(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_RELEASE:
		err = wanec_release(ec_dev, ec_api, ec_api->verbose);
		break;
	case WAN_EC_CMD_CHANNEL_OPEN:
		err = wanec_channel_open(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_ENABLE:
	case WAN_EC_CMD_DISABLE:
		err = wanec_modify(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_BYPASS_ENABLE:
	case WAN_EC_CMD_BYPASS_DISABLE:
		err = wanec_modify_bypass(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_MODE_NORMAL:
	case WAN_EC_CMD_MODE_POWERDOWN:
	case WAN_EC_CMD_MODIFY_CHANNEL:
		err = wanec_modify_mode(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_DTMF_ENABLE:
	case WAN_EC_CMD_DTMF_DISABLE:
		ec_api->err = wanec_modify_dtmf(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_STATS:
	case WAN_EC_CMD_STATS_FULL:
		err = wanec_stats(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_TONE_LOAD:
	case WAN_EC_CMD_TONE_UNLOAD:
		err = wanec_tone(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_PLAYOUT_START:
	case WAN_EC_CMD_PLAYOUT_STOP:
		err = wanec_playout(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_MONITOR:
		err = wanec_monitor(ec_dev, ec_api);
		break;
	case WAN_EC_CMD_RELEASE_ALL:
		break;
	}
	if (err){
		PRINT2(ec_api->verbose,
		"%s: %s return error (Command: %s)\n",
			ec_api->devname, __FUNCTION__, WAN_EC_CMD_DECODE(ec_api->cmd));
		ec_api->err = err;
	}
	if (ec_api->err == WAN_EC_API_RC_INVALID_STATE){
		ec_api->state	= ec->state;
	}
	wan_clear_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical);

wanec_ioctl_done:
	wan_spin_unlock(&ec->lock);
#if defined(__LINUX__)
	err = WAN_COPY_TO_USER(
			data,
			ec_api,
			sizeof(wan_ec_api_t));
	if (err){
		DEBUG_EVENT(
		"%s: Failed to copy data to user space [%s:%d]!\n",
			ec_api->devname,
			__FUNCTION__,__LINE__);
		wan_free(ec_api);
		return -EINVAL;
	}
#endif
	PRINT2(ec_api->verbose,
	"%s: WPEC_LIP IOCTL: %s returns %d\n",
			ec_api->devname,
			WAN_EC_CMD_DECODE(ec_api->cmd),
			ec_api->err);
			
#if defined(__LINUX__)
	wan_free(ec_api);
#endif			
	WAN_DEBUG_FUNC_END;
	return 0;
}


/*****************************************************************************/
#define my_isdigit(c)	((c) >= '0' && (c) <= '9')
static int wan_ec_devnum(char *ptr)
{
	int	num = 0, base = 1;
	int	i = 0;

	while(!my_isdigit(*ptr)) ptr++;
	while(my_isdigit(ptr[i])){
		i++;
		base = base * 10;
	}
	if (i){
		i=0;
		do {
			base = base / 10;
			num = num + (ptr[i]-'0') * base;
			i++;
		}while(my_isdigit(ptr[i]));
	}
	return num;
}

static void* wanec_register(void *pcard, int max_channels)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_ec_t	*ec = NULL;
	wan_ec_dev_t	*ec_dev = NULL, *ec_dev_new = NULL;

	WAN_DEBUG_FUNC_START;


#if defined(__WINDOWS__)	
	ec = get_wan_ec_ptr(card);
#else
	WAN_LIST_FOREACH(ec, &wan_ec_head, next){
		WAN_LIST_FOREACH(ec_dev, &ec->ec_dev_head, next){
			if (ec_dev->card == NULL || ec_dev->card == card){
				DEBUG_EVENT("%s: Internal Error (%s:%d)\n",
						card->devname,
						__FUNCTION__,__LINE__);
				return NULL;
			}
		}
	}
	WAN_LIST_FOREACH(ec, &wan_ec_head, next){
		WAN_LIST_FOREACH(ec_dev, &ec->ec_dev_head, next){
			if (card->hw_iface.hw_same(ec_dev->card->hw, card->hw)){
				/* Current OCT6100 chip is already in use */
				break;
			}
		}
		if (ec_dev){
			break;
		}
	}
#endif
	
	if (ec && ec_dev){
		if(wan_test_bit(WAN_EC_BIT_CRIT_ERROR, &ec_dev->ec->critical)){
			DEBUG_EVENT("%s: Echo Canceller chip has Critical Error flag set!\n",
					card->devname);
			return NULL;	
		}
	}

	ec_dev_new = wan_malloc(sizeof(wan_ec_dev_t));
	if (ec_dev_new == NULL){
		DEBUG_EVENT("%s: ERROR: Failed to allocate memory (%s:%d)!\n",
					card->devname,
					__FUNCTION__,__LINE__);
		return NULL;
	}
	memset(ec_dev_new, 0, sizeof(wan_ec_dev_t));
#if defined(__WINDOWS__)
	ec_dev = ec_dev_new;
	if (ec == NULL){		
#else
	if (ec_dev == NULL){
#endif
		/* First device for current Oct6100 chip */
		ec = wan_malloc(sizeof(wan_ec_t));
		if (ec == NULL){
			DEBUG_EVENT("%s: ERROR: Failed to allocate memory (%s:%d)!\n",
						card->devname,
						__FUNCTION__,__LINE__);
			return NULL;
		}

		memset(ec, 0, sizeof(wan_ec_t));
		ec->chip_no		= ++wan_ec_no;
		ec->state		= WAN_OCT6100_STATE_RESET;
		ec->ec_active		= 0;
		ec->max_channels	= max_channels;
		wan_spin_lock_init(&ec->lock);
		sprintf(ec->name, "%s%d", WANEC_DEV_NAME, ec->chip_no);
		Oct6100InterruptServiceRoutineDef(&ec->f_InterruptFlag);

#if defined(__WINDOWS__)
		set_wan_ec_ptr(card, ec);
#else
		WAN_LIST_INIT(&ec->ec_dev_head);
		WAN_LIST_INSERT_HEAD(&wan_ec_head, ec, next);
#endif
	}else{
#if !defined(__WINDOWS__)
		ec = ec_dev->ec;
#endif
	}
	ec->usage++;
	ec_dev_new->ecdev_no = wan_ec_devnum(card->devname);
	ec_dev_new->ec			= ec;
	ec_dev_new->name		= ec->name;
	ec_dev_new->card		= card;
	
	ec_dev_new->fe_media		= WAN_FE_MEDIA(&card->fe);
	ec_dev_new->fe_lineno		= WAN_FE_LINENO(&card->fe);
	ec_dev_new->fe_max_channels	= WAN_FE_MAX_CHANNELS(&card->fe);
	if (!WAN_FE_TDMV_LAW(&card->fe)){
		if (WAN_FE_MEDIA(&card->fe) == WAN_MEDIA_T1){
			WAN_FE_TDMV_LAW(&card->fe) = WAN_TDMV_MULAW;
		}else if (WAN_FE_MEDIA(&card->fe) == WAN_MEDIA_E1){
			WAN_FE_TDMV_LAW(&card->fe) = WAN_TDMV_ALAW;		
		}else{
			DEBUG_EVENT("%s: ERROR: Undefines MULAW/ALAW type!\n",
						card->devname);
		}
	}
	ec_dev_new->fe_tdmv_law		= WAN_FE_TDMV_LAW(&card->fe);
	ec_dev_new->state		= WAN_OCT6100_STATE_RESET;
		
	/* Initialize hwec_bypass pointer */
	card->wandev.ec_enable	= wanec_enable;
	card->wandev.ec_map	= 0;

	memcpy(ec_dev_new->devname, card->devname, sizeof(card->devname));
	sprintf(ec_dev_new->ecdev_name, "wp%dec", ec_dev_new->ecdev_no);

	wan_init_timer(	&ec_dev_new->timer,
			wanec_timer,
			(wan_timer_arg_t)ec_dev_new);

#if defined(__WINDOWS__)
	card->ec_dev_ptr = ec_dev_new;
#else
	WAN_LIST_INSERT_HEAD(&ec->ec_dev_head, ec_dev_new, next);
#endif

	DEBUG_EVENT("%s: Register EC interface %s (usage %d, max ec chans %d)!\n",
					ec_dev_new->devname,
					ec->name,
					ec->usage,
					ec->max_channels);
	return (void*)ec_dev_new;
}

static int wanec_unregister(void *arg, void *pcard)
{
	sdla_t		*card = (sdla_t*)pcard;
	wan_ec_t	*ec = NULL;
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_DEBUG_FUNC_START;

	ec = ec_dev->ec;
	wan_spin_lock(&ec->lock);
	DEBUG_EVENT("%s: Unregister interface from %s (chip id %d, usage %d)!\n",
					card->devname,
					ec->name,
					ec->chip_no,
					ec->usage);
	
	wan_set_bit(WAN_EC_BIT_TIMER_KILL,(void*)&ec_dev->critical);
	wan_set_bit(WAN_EC_BIT_CRIT_DOWN,(void*)&ec_dev->critical);
	wan_set_bit(WAN_EC_BIT_CRIT,(void*)&ec_dev->critical);
	wan_del_timer(&ec_dev->timer);				
	
	if (ec_dev->state != WAN_OCT6100_STATE_RESET){
		PRINT1(global_verbose,
		"%s: Forcing EC device release\n",
					card->devname);
		wanec_release(ec_dev, NULL, global_verbose);
	}
	ec_dev->card = NULL;
	ec->usage--;

#if !defined(__WINDOWS__)
	if (WAN_LIST_FIRST(&ec->ec_dev_head) == ec_dev){
		WAN_LIST_FIRST(&ec->ec_dev_head) =
				WAN_LIST_NEXT(ec_dev, next);
		WAN_LIST_NEXT(ec_dev, next) = NULL;
	}else{
		WAN_LIST_REMOVE(ec_dev, next);
	}
#endif

	card->wandev.ec_enable	= NULL;
#if defined(__WINDOWS__)
	card->ec_dev_ptr = NULL;
#endif

	/* FIXME: Remove character device */
	if (!ec->usage){
		ec_dev->ec = NULL;
		wan_free(ec_dev);
#if !defined(__WINDOWS__)
		if (WAN_LIST_FIRST(&wan_ec_head) == ec){
			WAN_LIST_FIRST(&wan_ec_head) =
					WAN_LIST_NEXT(ec, next);
			WAN_LIST_NEXT(ec, next) = NULL;
		}else{
			WAN_LIST_REMOVE(ec, next);
		}
#endif
		
		wan_free(ec);

#if defined(__WINDOWS__)
		set_wan_ec_ptr(card, NULL);
#endif

	}else{
		ec_dev->ec = NULL;
		wan_free(ec_dev);
	}
	wan_spin_unlock(&ec->lock);
	WAN_DEBUG_FUNC_END;
	return 0;
}

#if 0
static int wanec_isr(void *arg, void *pcard)
{
	wan_ec_t	*ec = NULL;
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;

	WAN_ASSERT2(ec_dev == NULL, 0);
	WAN_ASSERT2(ec_dev->ec == NULL, 0);
	ec = ec_dev->ec;

#if !defined(__WINDOWS__)
	if (WAN_LIST_FIRST(&ec->ec_dev_head) != ec_dev){
		return 0;
	}
#endif

	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		return 0;
	}
	if (wan_test_bit(WAN_EC_BIT_CRIT_DOWN, &ec_dev->critical)){
		return 0;
	}
	if (wan_test_bit(WAN_EC_BIT_CRIT_ERROR, &ec_dev->critical)){
		return 0;
	}	
	if (wan_test_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical)){
		return 0;
	}
	if (ec_dev->poll_cmd != WAN_EC_POLL_NONE){
		/* I'm still busy, return now */
		return 0;
	}

	ec->intcount++;
	
	/* Execute interrupt routine */
	if (wanec_ISR(ec, global_verbose)){
		wan_set_bit(WAN_EC_BIT_CRIT_ERROR, &ec->critical);
		wan_set_bit(WAN_EC_BIT_CRIT,(void*)&ec_dev->critical);
		return 0;
	}
	
	PRINT1(global_verbose, 
	"%s: HW EC ISR-POLL (%d:%d)\n",
		ec_dev->devname, ec->intcount,
		(ec->f_InterruptFlag.fToneEventsPending == TRUE)?1:0);

	if (ec->f_InterruptFlag.fToneEventsPending == TRUE &&
	    wan_test_bit(WAN_EC_BIT_EVENT_DTMF, &ec_dev->events)){
		ec_dev->poll_cmd = WAN_EC_POLL_INTR;
		/* Schedule poll */
		return 1;
	}
	return 0;
}
#endif 

static int wanec_poll(void *arg, void *pcard)
{
	wan_ec_t	*ec = NULL;
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;
	int		err = 0;
	
	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	
	WAN_DEBUG_FUNC_START;
		
	wan_spin_lock(&ec->lock);
	wan_clear_bit(WAN_EC_BIT_TIMER_RUNNING,(void*)&ec_dev->critical);
	if (wan_test_bit(WAN_EC_BIT_CRIT_DOWN, &ec_dev->critical)){
		ec_dev->poll_cmd = WAN_EC_POLL_NONE;
		wan_spin_unlock(&ec->lock);
		return -EINVAL;
	}
	switch(ec_dev->poll_cmd){
	case WAN_EC_POLL_CHIPOPENPENDING:
		/* Chip open */
		if (wanec_ChipOpen(ec_dev, WAN_EC_VERBOSE_NONE)){
			/* Chip state is Ready state */
			ec->state = WAN_OCT6100_STATE_READY;
			ec_dev->state = ec->state;
			ec_dev->poll_cmd = WAN_EC_POLL_NONE;
			err = -EINVAL;
			goto wanec_poll_done;
		}	
		ec->state = WAN_OCT6100_STATE_CHIP_OPEN;
		ec_dev->state = ec->state;
		break;
	
	case WAN_EC_POLL_INTR:
	default:	/* by default, can be only schedule from interrupt */
		if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
			break;
		}

		if ((wan_test_bit(WAN_EC_BIT_CRIT_DOWN, &ec_dev->critical)) || 
		    (wan_test_bit(WAN_EC_BIT_CRIT_ERROR, &ec_dev->critical)) ||
		    (wan_test_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical))) {
			err = -EINVAL;
			break;
		}
		
		/* Execute interrupt routine */
		if (wanec_ISR(ec, global_verbose)){
			wan_set_bit(WAN_EC_BIT_CRIT_ERROR, &ec->critical);
			wan_set_bit(WAN_EC_BIT_CRIT,(void*)&ec_dev->critical);
			err = -EINVAL;
			break;
		}	
		break;
	}
	ec_dev->poll_cmd = WAN_EC_POLL_NONE;
	
wanec_poll_done:	
	wan_spin_unlock(&ec->lock);
	WAN_DEBUG_FUNC_END;	
	return err;
}

static int wanec_event_ctrl(void *arg, void *pcard, wan_event_ctrl_t *event_ctrl)
{
	wan_ec_t	*ec = NULL;
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;
	int		err = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(event_ctrl  == NULL);
	ec = ec_dev->ec;
	
	if (wan_test_and_set_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical)){
		return -EBUSY;
	}

	switch(event_ctrl->type){
	case WAN_EVENT_EC_DTMF:
		DEBUG_EVENT("%s: %s DTMF events\n",
				ec_dev->devname,
				WAN_EVENT_MODE_DECODE(event_ctrl->mode));
		if (event_ctrl->mode == WAN_EVENT_ENABLE){
			wan_set_bit(WAN_EC_BIT_EVENT_DTMF, &ec_dev->events);
		}else{
			wan_clear_bit(WAN_EC_BIT_EVENT_DTMF, &ec_dev->events);
		}
		break;
	default:
		err = -EINVAL;
		break;		
	}
	if (!err){
#if !defined(__WINDOWS__)
		wan_free(event_ctrl);
#endif
	}
	wan_clear_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical);
	return err;
}

int wanec_init(void *arg)
{
#if defined(__WINDOWS__)
	if(wanec_iface.reg != NULL){
		DEBUG_EVENT("%s(): Warning: Initialization already done!\n", __FUNCTION__);
		return 0;
	}
#endif

	if (WANPIPE_VERSION_BETA){
		DEBUG_EVENT("%s Beta %s.%s %s\n",
				wpec_fullname,
				WANPIPE_VERSION,
				WANPIPE_SUB_VERSION,
				wpec_copyright);
	}else{
		DEBUG_EVENT("%s Stable %s.%s %s\n",
				wpec_fullname,
				WANPIPE_VERSION,
				WANPIPE_SUB_VERSION,
				wpec_copyright);
	}
	
	/* Initialize WAN EC lip interface */
	wanec_iface.reg		= wanec_register;
	wanec_iface.unreg	= wanec_unregister;
	wanec_iface.ioctl	= NULL;
	wanec_iface.isr		= NULL;			// wanec_isr;
	wanec_iface.poll	= wanec_poll;
	wanec_iface.event_ctrl	= wanec_event_ctrl;
	
	register_wanec_iface (&wanec_iface);

#if defined(__FreeBSD__) || defined(__OpenBSD__)
	wp_cdev_reg(NULL, WANEC_DEV_NAME, wanec_ioctl);
#elif defined(__LINUX__) || defined(__WINDOWS__)
	wanec_create_dev();
#endif
	return 0;
}

int wanec_exit (void *arg)
{
#if defined(__FreeBSD__) || defined(__OpenBSD__)
	wp_cdev_unreg(WANEC_DEV_NAME);
#elif defined(__LINUX__) || defined(__WINDOWS__)
	wanec_remove_dev();
#endif
	unregister_wanec_iface();
	DEBUG_EVENT("WANEC Layer: Unloaded\n");
	return 0;
}

#if !defined(__WINDOWS__)
WAN_MODULE_DEFINE(
		wanec,"wanec", 
		"Alex Feldman <al.feldman@sangoma.com>",
		"Wanpipe Echo Canceller Layer - Sangoma Tech. Copyright 2006", 
		"GPL",
		wanec_init, wanec_exit, NULL);

WAN_MODULE_DEPEND(wanec, wanrouter, 1,
			WANROUTER_MAJOR_VER, WANROUTER_MAJOR_VER);

WAN_MODULE_DEPEND(wanec, wanpipe, 1,
			WANPIPE_MAJOR_VER, WANPIPE_MAJOR_VER);
#endif