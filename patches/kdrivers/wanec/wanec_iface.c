/********************************************************************
 * wanec_iface.c   WANPIPE Echo Canceller Layer (WANEC)
 *
 *
 *  
 * ==================================================================
 *
 * May	10	2006	Alex Feldman	
 *			Initial Version
 *
 * January 9	2008	David Rokhvarg
 *			Added support for Sangoma MS Windows Driver
 *
 ********************************************************************/

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
/*# define DEBUG*/
#endif

#include "wanec_iface_api.h"

/*=============================================================
 * Definitions
 */
#define WAN_OCT6100_ENABLE_INTR_POLL

#define WAN_OCT6100_READ_LIMIT		0x10000

#if 0
# define WANEC_DEBUG
#endif

/*=============================================================
 * Global Parameters
 */
#if defined(WANEC_DEBUG)
static int	wanec_verbose = WAN_EC_VERBOSE_EXTRA2;
#else
static int	wanec_verbose = WAN_EC_VERBOSE_NONE;
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

extern int wanec_ChipOpenPrep(wan_ec_dev_t *ec_dev, char *devname, wanec_config_t *config, int);
extern int wanec_ChipOpen(wan_ec_dev_t*, int verbose);
extern int wanec_ChipClose(wan_ec_dev_t*, int verbose);
extern int wanec_ChipStats(wan_ec_dev_t *ec_dev, wanec_chip_stats_t *chip_stats, int reset, int verbose);
extern int wanec_ChipImage(wan_ec_dev_t *ec_dev, wanec_chip_image_t *chip_image, int verbose);

extern int wanec_ChannelOpen(wan_ec_dev_t*, int);
extern int wanec_ChannelClose(wan_ec_dev_t*, int);
extern int wanec_ChannelModifyOpmode(wan_ec_dev_t*, INT, UINT32, int verbose);
extern int wanec_ChannelModifyCustom(wan_ec_dev_t*, INT, wanec_chan_custom_t*, int verbose);
extern int wanec_ChannelStats(wan_ec_dev_t*, INT ec_chan, wanec_chan_stats_t *chan_stats, int reset);

extern int wanec_ChannelMute(wan_ec_dev_t*, INT ec_chan, wanec_chan_mute_t*, int);
extern int wanec_ChannelUnMute(wan_ec_dev_t*, INT ec_chan, wanec_chan_mute_t*, int);

extern int wanec_TonesEnable(wan_ec_t *ec, int ec_chan, wanec_dtmf_config_t*, int verbose);
extern int wanec_FaxTonesEnable(wan_ec_t *ec, int ec_chan, wanec_dtmf_config_t*, int verbose);
extern int wanec_TonesDisable(wan_ec_t *ec, int ec_chan, wanec_dtmf_config_t*, int verbose);

extern int wanec_DebugChannel(wan_ec_dev_t*, INT, int);
extern int wanec_DebugGetData(wan_ec_dev_t*, wanec_chan_monitor_t*, int);

extern int wanec_BufferLoad(wan_ec_dev_t *ec_dev, wanec_buffer_config_t *buffer_config, int verbose);
extern int wanec_BufferUnload(wan_ec_dev_t *ec_dev, wanec_buffer_config_t *buffer_config, int verbose);
extern int wanec_BufferPlayoutAdd(wan_ec_t *ec, int ec_chan, wanec_playout_t *playout, int verbose);
extern int wanec_BufferPlayoutStart(wan_ec_t *ec, int ec_chan, wanec_playout_t *playout, int verbose);
extern int wanec_BufferPlayoutStop(wan_ec_t *ec, int ec_chan, wanec_playout_t *playout, int verbose);

extern int wanec_EventTone(wan_ec_t *ec, int verbose);
extern int wanec_ISR(wan_ec_t *ec, int verbose);

static int wanec_channel_opmode_modify(wan_ec_dev_t *ec_dev, int fe_chan, UINT32 opmode, int verbose);
static int wanec_channel_dtmf(wan_ec_dev_t*, int, int, wanec_dtmf_config_t*, int);

static int wanec_bypass(wan_ec_dev_t *ec_dev, int fe_chan, int enable, int verbose);

static int wanec_api_config(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_release(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api, int verbose);
static int wanec_api_channel_open(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_modify(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_chan_opmode(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_chan_custom(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_modify_bypass(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_dtmf(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_stats(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_stats_image(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_buffer(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_playout(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
static int wanec_api_monitor(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);

static wan_ec_dev_t *wanec_search(char *devname);

static int wanec_enable(void *pcard, int enable, int fe_chan);
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
#if defined(__FreeBSD__)
int wanec_shutdown(void*);
int wanec_ready_unload(void*);
#endif

/*****************************************************************************/
# if defined(WAN_DEBUG_MEM)
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# define EXPORT_SYMBOL(symbol)
#endif
static int wan_debug_mem;

static wan_spinlock_t wan_debug_mem_lock;

WAN_LIST_HEAD(NAME_PLACEHOLDER_MEM, sdla_memdbg_el) sdla_memdbg_head = 
			WAN_LIST_HEAD_INITIALIZER(&sdla_memdbg_head);

typedef struct sdla_memdbg_el
{
	unsigned int len;
	unsigned int line;
	char cmd_func[128];
	void *mem;
	WAN_LIST_ENTRY(sdla_memdbg_el)	next;
}sdla_memdbg_el_t;

static int wanec_memdbg_init(void);
static int wanec_memdbg_free(void);

static int wanec_memdbg_init(void)
{
	wan_spin_lock_init(&wan_debug_mem_lock,"wan_debug_mem_lock");
	WAN_LIST_INIT(&sdla_memdbg_head);
	return 0;
}


int sdla_memdbg_push(void *mem, const char *func_name, const int line, int len)
{
	sdla_memdbg_el_t *sdla_mem_el = NULL;
	wan_smp_flag_t flags;

#if defined(__LINUX__)
	sdla_mem_el = kmalloc(sizeof(sdla_memdbg_el_t),GFP_ATOMIC);
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	sdla_mem_el = malloc(sizeof(sdla_memdbg_el_t), M_DEVBUF, M_NOWAIT); 
#endif
	if (!sdla_mem_el) {
		DEBUG_EVENT("%s:%d Critical failed to allocate memory!\n",
			__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	memset(sdla_mem_el,0,sizeof(sdla_memdbg_el_t));
		
	sdla_mem_el->len=len;
	sdla_mem_el->line=line;
	sdla_mem_el->mem=mem;
	strncpy(sdla_mem_el->cmd_func,func_name,sizeof(sdla_mem_el->cmd_func)-1);
	
	wan_spin_lock_irq(&wan_debug_mem_lock,&flags);
	wan_debug_mem+=sdla_mem_el->len;
	WAN_LIST_INSERT_HEAD(&sdla_memdbg_head, sdla_mem_el, next);
	wan_spin_unlock_irq(&wan_debug_mem_lock,&flags);

	DEBUG_EVENT("%s:%d: Alloc %p Len=%i Total=%i\n",
			sdla_mem_el->cmd_func,sdla_mem_el->line,
			 sdla_mem_el->mem, sdla_mem_el->len,wan_debug_mem);
	return 0;

}
EXPORT_SYMBOL(sdla_memdbg_push);

int sdla_memdbg_pull(void *mem, const char *func_name, const int line)
{
	sdla_memdbg_el_t *sdla_mem_el;
	wan_smp_flag_t flags;
	int err=-1;

	wan_spin_lock_irq(&wan_debug_mem_lock,&flags);

	WAN_LIST_FOREACH(sdla_mem_el, &sdla_memdbg_head, next){
		if (sdla_mem_el->mem == mem) {
			break;
		}
	}

	if (sdla_mem_el) {
		
		WAN_LIST_REMOVE(sdla_mem_el, next);
		wan_debug_mem-=sdla_mem_el->len;
		wan_spin_unlock_irq(&wan_debug_mem_lock,&flags);

		DEBUG_EVENT("%s:%d: DeAlloc %p Len=%i Total=%i (From %s:%d)\n",
			func_name,line,
			sdla_mem_el->mem, sdla_mem_el->len, wan_debug_mem,
			sdla_mem_el->cmd_func,sdla_mem_el->line);
#if defined(__LINUX__)
		kfree(sdla_mem_el);
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
		free(sdla_mem_el, M_DEVBUF); 
#endif
		err=0;
	} else {
		wan_spin_unlock_irq(&wan_debug_mem_lock,&flags);
	}

	if (err) {
		DEBUG_EVENT("%s:%d: Critical Error: Unknows Memeory %p\n",
			__FUNCTION__,__LINE__,mem);
	}

	return err;
}
EXPORT_SYMBOL(sdla_memdbg_pull);

static int wanec_memdbg_free(void)
{
	sdla_memdbg_el_t *sdla_mem_el;
	int total=0;

	DEBUG_EVENT("wanec: Memory Still Allocated=%i \n",
			 wan_debug_mem);

	DEBUG_EVENT("=====================BEGIN================================\n");

	sdla_mem_el = WAN_LIST_FIRST(&sdla_memdbg_head);
	while(sdla_mem_el){
		sdla_memdbg_el_t *tmp = sdla_mem_el;

		DEBUG_EVENT("%s:%d: Mem Leak %p Len=%i \n",
			sdla_mem_el->cmd_func,sdla_mem_el->line,
			sdla_mem_el->mem, sdla_mem_el->len);
		total+=sdla_mem_el->len;

		sdla_mem_el = WAN_LIST_NEXT(sdla_mem_el, next);
		WAN_LIST_REMOVE(tmp, next);
#if defined(__LINUX__)
		kfree(tmp);
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
		free(tmp, M_DEVBUF); 
#endif
	}

	DEBUG_EVENT("=====================END==================================\n");
	DEBUG_EVENT("wanec: Memory Still Allocated=%i  Leaks Found=%i Missing=%i\n",
			 wan_debug_mem,total,wan_debug_mem-total);

	return 0;
}

# endif

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

	if (IS_A600_CARD(card)) {
		addr+=0x1000;
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

	if (IS_A600_CARD(card)) {
		addr+=0x1000;
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
				ec->state = WAN_EC_STATE_RESET;
			}else{
				ec->state = WAN_EC_STATE_READY;
			}
		}
	}
	return err;
}

static int wanec_enable(void *pcard, int enable, int fe_chan)
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
	err = wanec_bypass(ec_dev, fe_chan, enable, 0);
	wan_spin_unlock(&ec_dev->ec->lock);

	return err;
#else
	return wanec_channel_opmode_modify(
			ec_dev,
			fe_chan,
			(enable) ? cOCT6100_ECHO_OP_MODE_NORMAL : cOCT6100_ECHO_OP_MODE_POWER_DOWN, 
			0);
#endif
}

static int 
wanec_bypass(wan_ec_dev_t *ec_dev, int fe_chan, int enable, int verbose)
{
	wan_ec_t	*ec = NULL;
	sdla_t		*card = NULL;
	int		ec_chan = 0, err = -ENODEV;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	ec = ec_dev->ec;	
	card = ec_dev->card;

	PRINT1(verbose,
	"%s: %s bypass mode for fe_chan:%d (ec map:%lX, fe map:%X)!\n",
				card->devname,
				(enable) ? "Enable" : "Disable",
				fe_chan,
				card->wandev.fe_ec_map, ec_dev->fe_channel_map);

	if (card->wandev.hwec_enable == NULL){
		DEBUG_EVENT( "%s: Undefined HW EC Bypass callback function!\n",
					ec->name);
		return -ENODEV;
	}
	if (!wan_test_bit(fe_chan, &ec_dev->fe_channel_map)){
		PRINT1(verbose, "%s: FE channel %d is not available (fe_chan_map=%X)!\n",
					ec->name, fe_chan, ec_dev->fe_channel_map);
		return 0;
	}
	ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
	if (enable){
		if (wan_test_bit(fe_chan, &card->wandev.fe_ec_map)){
			/* Already enabled */
                        PRINT2(verbose,
			"%s: Enable bypass mode overrun detected for ec_chan %d!\n",
                                card->devname, ec_chan);
			return 0;
		}
	}else{
		if (!wan_test_bit(fe_chan, &card->wandev.fe_ec_map)){
			/* Already disabled */
                        PRINT2(verbose,
			"%s: Disble bypass mode overrun detected for ec_channel %d!\n",
                                card->devname, ec_chan);
			return 0;
		}
	}

	err=wan_ec_update_and_check(ec,enable);
        if (err) {
                DEBUG_EVENT("%s: Error: Maximum EC Channels Reached! MaxEC=%i\n",
                                ec->name,ec->max_channels);
                return err;
        }

	err = card->wandev.hwec_enable(card, enable, fe_chan);
	if (!err){
		if (enable){
			wan_set_bit(fe_chan, &card->wandev.fe_ec_map);
		}else{
			wan_clear_bit(fe_chan, &card->wandev.fe_ec_map);
		}
	}else if (err < 0){

		/* If ec hwec_enable function fails, undo the action of above
		   wan_ec_update_and_check fucntion by passing in an inverted 
		   enable variable */
		wan_ec_update_and_check(ec,!enable);

		DEBUG_EVENT("ERROR: %s: Failed to %s Bypass mode on fe_chan:%d!\n",
				ec->name, 
				(enable) ? "Enable" : "Disable",
				fe_chan);
		return err;
	}else{
		/* no action made */
		err = 0;
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


static int wanec_channel_opmode_modify(wan_ec_dev_t *ec_dev, int fe_chan, UINT32 opmode, int verbose)
{
	wan_ec_t	*ec = NULL;
	u_int32_t	ec_chan = 0; 

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	ec = ec_dev->ec;

	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return -EINVAL;
	}

	switch(opmode){
	case cOCT6100_ECHO_OP_MODE_NORMAL:
	case cOCT6100_ECHO_OP_MODE_HT_FREEZE:
	case cOCT6100_ECHO_OP_MODE_HT_RESET:
	case cOCT6100_ECHO_OP_MODE_NO_ECHO:
	case cOCT6100_ECHO_OP_MODE_POWER_DOWN:
	case cOCT6100_ECHO_OP_MODE_SPEECH_RECOGNITION:
		break;
	default:
		DEBUG_EVENT(
		"%s: Invalid Echo Channel Operation Mode (opmode=%X)\n",
				ec_dev->devname, opmode);
		return -EINVAL;
	}

	/* Enable Echo cancelation on Oct6100 */
	PRINT1(verbose,
	"%s: Modify Echo Channel OpMode %s on fe_chan:%d ...\n",
			ec_dev->devname,
			(opmode == cOCT6100_ECHO_OP_MODE_NORMAL) ? "Normal" :
			(opmode == cOCT6100_ECHO_OP_MODE_POWER_DOWN) ? "Power Down" : 
			(opmode == cOCT6100_ECHO_OP_MODE_HT_FREEZE) ? "HT Freeze" : 
			(opmode == cOCT6100_ECHO_OP_MODE_HT_RESET) ? "HT Reset" : 
			(opmode == cOCT6100_ECHO_OP_MODE_NO_ECHO) ? "NO Echo" : 
			(opmode == cOCT6100_ECHO_OP_MODE_SPEECH_RECOGNITION) ? "Speech Recognition" : "Unknown", 
			fe_chan);
	ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
	return wanec_ChannelModifyOpmode(ec_dev, ec_chan, opmode, verbose);
}

static int wanec_channel_dtmf(	wan_ec_dev_t		*ec_dev, 
				int			fe_chan, 
				int			cmd, 
				wanec_dtmf_config_t	*dtmf, 
				int			verbose)
{
	wan_ec_t	*ec = NULL;
	int		ec_chan, err;
	sdla_t 		*card;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	
	ec = ec_dev->ec;
	card = ec_dev->card;

	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return -EINVAL;
	}

	if (dtmf){
		if ((dtmf->port_map & (WAN_EC_CHANNEL_PORT_SOUT|WAN_EC_CHANNEL_PORT_ROUT)) != dtmf->port_map){ 
 
			DEBUG_EVENT(
			"ERROR: %s: Invalid Echo Canceller port value (%X)!\n",
					ec_dev->devname, 
					dtmf->port_map);
			return -EINVAL;
		}
	}
	/* Enable/Disable Normal mode on Oct6100 */
	PRINT1(verbose, "%s: %s EC DTMF detection on fe_chan:%d ...\n",
			ec_dev->devname, (cmd==WAN_TRUE) ? "Enable" : "Disable",
			fe_chan);

	ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
	if (cmd == WAN_TRUE){
		err = wanec_TonesEnable(ec, ec_chan, dtmf, verbose);
		if (err == 0 && card->tdmv_conf.hw_fax_detect) {
			err = wanec_FaxTonesEnable(ec, ec_chan, dtmf, verbose);
		}
	}else{
		err = wanec_TonesDisable(ec, ec_chan, dtmf, verbose);
	}
	if (err == WAN_EC_API_RC_OK){
		if (cmd == WAN_TRUE){
			wan_set_bit(WAN_EC_BIT_EVENT_DTMF, &ec_dev->events);
			ec->tone_verbose = verbose;
		}else{
			/* FIXME: Once the DTMF event was enabled, do not 
			** disable it otherwise dtmf events for other 
			** channels will be delayed 
			** wan_clear_bit(WAN_EC_BIT_EVENT_DTMF, &ec_dev->events);
			** ec->tone_verbose = 0;	*/
		}
	}
	return err;
}


static int wanec_channel_mute(	wan_ec_dev_t		*ec_dev, 
				int			fe_chan,
				int			cmd, 
				wanec_chan_mute_t 	*mute, 
				int 			verbose)
{
	wan_ec_t	*ec = NULL;
	int		ec_chan, err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;

	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return -EINVAL;
	}

	/* Enable/Disable Normal mode on Oct6100 */
	PRINT1(verbose,
	"%s: %s EC channel on fe_chan:%d ...\n",
			ec_dev->devname,
			(cmd == WAN_TRUE) ? "Mute" : "Un-mute",
			fe_chan);

	ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
	if (cmd == WAN_TRUE){
		err = wanec_ChannelMute(ec_dev, ec_chan, mute, verbose);
	}else{
		err = wanec_ChannelUnMute(ec_dev, ec_chan, mute, verbose);
	}
	return err;
}


/******************************************************************************
**		WANPIPE EC API INTERFACE FUNCTION
******************************************************************************/

static int wanec_api_config(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	switch(ec->state){
	case WAN_EC_STATE_RESET:
	case WAN_EC_STATE_READY:
		break;
	case WAN_EC_STATE_CHIP_OPEN:
	case WAN_EC_STATE_CHIP_OPEN_PENDING:
	case WAN_EC_STATE_CHIP_READY:
		DEBUG_HWEC(
		"%s: Echo Canceller %s chip is %s!\n",
				ec_api->devname, ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		break;
	default:
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_api->devname, ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		ec_api->err	= WAN_EC_API_RC_INVALID_STATE;
		return 0;
	}

	if (ec->state == WAN_EC_STATE_RESET){
		err = wanec_reset(ec_dev, 0);
		if (err) return err;
	}

	if (ec->state == WAN_EC_STATE_READY){

		ec->pImageData = wan_vmalloc(ec_api->u_config.imageSize * sizeof(UINT8));
		if (ec->pImageData == NULL){
			DEBUG_EVENT(
			"ERROR: Failed to allocate memory for EC image %ld bytes [%s:%d]!\n",
					(unsigned long)ec_api->u_config.imageSize*sizeof(UINT8),
					__FUNCTION__,__LINE__);
			err = wanec_reset(ec_dev, 0);
			return -EINVAL;	
		}
		err = WAN_COPY_FROM_USER(
					ec->pImageData,
					ec_api->u_config.imageData,
					ec_api->u_config.imageSize * sizeof(UINT8));
		if (err){
			DEBUG_EVENT(
			"ERROR: Failed to copy EC image from user space [%s:%d]!\n",
					__FUNCTION__,__LINE__);
			wan_vfree(ec->pImageData);
			err = wanec_reset(ec_dev, 0);
			return -EINVAL;
		}
		ec->ImageSize = ec_api->u_config.imageSize;

		/* Copyin custom configuration (if exists) */
		if (ec_api->custom_conf.param_no){
			ec_api->u_config.custom_conf.params = 
					wan_malloc(ec_api->custom_conf.param_no * sizeof(wan_custom_param_t));
			if (ec_api->u_config.custom_conf.params){
				int	err = 0;
				err = WAN_COPY_FROM_USER(
						ec_api->u_config.custom_conf.params,
						ec_api->custom_conf.params,
						ec_api->custom_conf.param_no * sizeof(wan_custom_param_t));
				ec_api->u_config.custom_conf.param_no = ec_api->custom_conf.param_no;
			}
		}

		if (wanec_ChipOpenPrep(ec_dev, ec_api->devname, &ec_api->u_config, ec_api->verbose)){
			wan_vfree(ec->pImageData);
			if (ec_api->u_config.custom_conf.params){
				wan_free(ec_api->u_config.custom_conf.params);
			}
			wanec_reset(ec_dev, 1);
			return -EINVAL;
		}
		if (ec_api->u_config.custom_conf.params){
			wan_free(ec_api->u_config.custom_conf.params);
		}
		ec->imageLast	= ec_api->u_config.imageLast;
		ec->state	= WAN_EC_STATE_CHIP_OPEN_PENDING;
		wanec_enable_timer(ec_dev, WAN_EC_POLL_CHIPOPENPENDING, 10);
	}
	ec_dev->state = ec->state;
	return 0;
}

static int wanec_api_channel_open(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		ec_chan, fe_chan;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	switch(ec->state){
	case WAN_EC_STATE_CHIP_OPEN:
		break;
	case WAN_EC_STATE_CHIP_READY:
		PRINT1(ec_api->verbose,
		"%s: Echo Canceller %s chip is %s!\n",
				ec_api->devname, ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		break;
	default:
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_api->devname, ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		ec_api->err	= WAN_EC_API_RC_INVALID_STATE;
		return 0;
	}

	if (ec->state == WAN_EC_STATE_CHIP_OPEN){

		/* Open all channels */	
		if (wanec_ChannelOpen(ec_dev, ec_api->verbose)){
			wanec_ChipClose(ec_dev, ec_api->verbose);
			wanec_reset(ec_dev, 1);
			return -EINVAL;
		}
		ec->state = WAN_EC_STATE_CHIP_READY;
	}
	ec_dev->state = ec->state;

	/* EC_DEV_MAP */
	for(fe_chan = ec_dev->fe_start_chan; fe_chan <= ec_dev->fe_stop_chan; fe_chan++){
		ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
		ec->pEcDevMap[ec_chan] = ec_dev;
	}
	return 0;
}

int wanec_api_release(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api, int verbose)
{
	wan_ec_t	*ec = NULL;
	wan_ec_dev_t	*ec_dev_tmp = NULL;
	int		fe_chan, ec_chan, err = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	
	switch(ec->state){
	case WAN_EC_STATE_READY:
	case WAN_EC_STATE_CHIP_OPEN:
	case WAN_EC_STATE_CHIP_OPEN_PENDING:
	case WAN_EC_STATE_CHIP_READY:
		break;
	case WAN_EC_STATE_RESET:
		return 0;
	default:
		PRINT1(verbose,
		"%s: WARNING: Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return 0;
	}

#if defined(__WINDOWS__)
	//for TDMV API there is only only one device created.
	//So 'release' request should simply go ahead.
		EC_FUNC_DEBUG();
#else
	WAN_LIST_FOREACH(ec_dev_tmp, &ec->ec_dev_head, next){
		if (ec_dev_tmp != ec_dev){
			if (ec_dev_tmp->state == WAN_EC_STATE_CHIP_READY){
				/* This EC device is still connected */
				ec->f_Context.ec_dev	= ec_dev_tmp;
				strlcpy(ec->f_Context.devname, ec_dev_tmp->devname, WAN_DRVNAME_SZ);
				break;
			}
		} 
	}

#endif

	for(fe_chan = ec_dev->fe_start_chan; fe_chan <= ec_dev->fe_stop_chan; fe_chan++){
		ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
		if (ec->pEcDevMap){
			ec->pEcDevMap[ec_chan] = NULL;
		}
	}
	ec_dev->state = WAN_EC_STATE_RESET;
	if (ec_dev_tmp){
		/* EC device is still in used */
		return 0;
	}

	/* EC device is not in used anymore.
	** Close all channels and chip */
	if (ec->state == WAN_EC_STATE_CHIP_READY){
		if (wanec_ChannelClose(ec_dev, verbose)){
			return EINVAL;
		}
		ec->state = WAN_EC_STATE_CHIP_OPEN;
	}
	if (ec->state == WAN_EC_STATE_CHIP_OPEN){
		if (wanec_ChipClose(ec_dev, verbose)){
			return EINVAL;
		}
		ec->state = WAN_EC_STATE_READY;
	}

	if (ec->state == WAN_EC_STATE_CHIP_OPEN_PENDING){
		ec->state = WAN_EC_STATE_READY;
	}
	if (ec->state == WAN_EC_STATE_READY){
		err = wanec_reset(ec_dev, 1);
		if (err){
			return EINVAL;
		}
	}
	return 0;
}


static int wanec_api_modify(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	u_int32_t	cmd = ec_api->cmd;
	int		fe_chan = 0; 
#if 0
	u_int32_t	ec_chan = 0; 
#endif
	int		err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;

	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_api->fe_chan_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->fe_chan_map = ec_dev->fe_channel_map;
	}

	/* Enable Echo cancelation on Oct6100 */
	PRINT1(ec_api->verbose,
	"%s: %s Echo Canceller on channel(s) chan_map=0x%08lX ...\n",
			ec_dev->devname,
			(cmd == WAN_EC_API_CMD_ENABLE) ? "Enable" : "Disable",
			ec_api->fe_chan_map);
	/*for(chan = fe_first; chan <= fe_last; chan++){*/
	for(fe_chan = ec_dev->fe_start_chan; fe_chan <= ec_dev->fe_stop_chan; fe_chan++){
		if (!(ec_api->fe_chan_map & (1 << fe_chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0) continue;
		if (cmd == WAN_EC_API_CMD_ENABLE){
			err = wanec_channel_opmode_modify(
					ec_dev, fe_chan,
					cOCT6100_ECHO_OP_MODE_NORMAL,
					ec_api->verbose);
			if (err) return WAN_EC_API_RC_FAILED;
			err = wanec_bypass(ec_dev, fe_chan, 1, ec_api->verbose);
		}else{
			wanec_bypass(ec_dev, fe_chan, 0, ec_api->verbose);
			err = wanec_channel_opmode_modify(
					ec_dev, fe_chan,
					cOCT6100_ECHO_OP_MODE_POWER_DOWN,
					ec_api->verbose);
		}
		if (err){
			return WAN_EC_API_RC_FAILED;
		}
	}

	return 0;
}

static int wanec_api_chan_opmode(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		fe_chan, err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_api->fe_chan_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->fe_chan_map = ec_dev->fe_channel_map;
	}
	/* Enable/Disable Normal mode on Oct6100 */
	PRINT1(ec_api->verbose,
	"%s: Modify Echo Canceller opmode on channel(s) chan_map=0x%08lX ...\n",
			ec_dev->devname, ec_api->fe_chan_map);
	for(fe_chan = ec_dev->fe_start_chan; fe_chan <= ec_dev->fe_stop_chan; fe_chan++){
		if (!(ec_api->fe_chan_map & (1 << fe_chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0) continue;
		err = wanec_channel_opmode_modify(
				ec_dev, fe_chan, ec_api->u_chan_opmode.opmode, ec_api->verbose);
		if (err){
			return WAN_EC_API_RC_FAILED;
		}	
	}
	return 0;
}

static int wanec_api_chan_custom(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t		*ec = NULL;
	wanec_chan_custom_t	*chan_custom;
	int			fe_chan, ec_chan, err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_api->fe_chan_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->fe_chan_map = ec_dev->fe_channel_map;
	}
	/* Enable/Disable Normal mode on Oct6100 */
	PRINT1(ec_api->verbose,
	"%s: Modify EC Channel config (parms:%d) on channel(s) chan_map=0x%08lX ...\n",
			ec_dev->devname, ec_api->custom_conf.param_no, 
			ec_api->fe_chan_map);
	if (ec_api->custom_conf.param_no == 0){
		/* nothing to do */
		return 0;
	}
	chan_custom = &ec_api->u_chan_custom;
	chan_custom->custom_conf.params = 
			wan_malloc(ec_api->custom_conf.param_no * sizeof(wan_custom_param_t));
	if (chan_custom->custom_conf.params){
		int	err = 0;
		err = WAN_COPY_FROM_USER(
				chan_custom->custom_conf.params,
				ec_api->custom_conf.params,
				ec_api->custom_conf.param_no * sizeof(wan_custom_param_t));
		chan_custom->custom_conf.param_no = ec_api->custom_conf.param_no;
	}else{
		DEBUG_EVENT(
		"%s: WARNING: Skipping custom OCT6100 configuration (allocation failed)!\n",
					ec_dev->devname);
		return WAN_EC_API_RC_FAILED;
	}

	for(fe_chan = ec_dev->fe_start_chan; fe_chan <= ec_dev->fe_stop_chan; fe_chan++){
		if (!(ec_api->fe_chan_map & (1 << fe_chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0) continue;
		ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
		err = wanec_ChannelModifyCustom(ec_dev, ec_chan, chan_custom, ec_api->verbose);
		if (err){
			wan_free(chan_custom->custom_conf.params);
			chan_custom->custom_conf.params = NULL;
			return WAN_EC_API_RC_FAILED;
		}	
	}
	wan_free(chan_custom->custom_conf.params);
	chan_custom->custom_conf.params = NULL;
	return 0;
}

static int wanec_api_modify_bypass(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	unsigned int	fe_chan = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;

	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}
	if (ec_api->fe_chan_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->fe_chan_map = ec_dev->fe_channel_map;
	}
	/* Enable/Disable bypass mode on Oct6100 */
	PRINT1(ec_api->verbose,
	"%s: %s Bypass mode on channel(s) chan_map=0x%08lX ...\n",
			ec_dev->devname,
			(ec_api->cmd == WAN_EC_API_CMD_BYPASS_ENABLE) ? "Enable" : "Disable",
			ec_api->fe_chan_map);	
	for(fe_chan = ec_dev->fe_start_chan; fe_chan <= (unsigned int)ec_dev->fe_stop_chan; fe_chan++){
		if (!(ec_api->fe_chan_map & (1 << fe_chan))){
			continue;
		}
		if (ec_api->cmd == WAN_EC_API_CMD_BYPASS_ENABLE){
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


static int wanec_api_channel_mute(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		fe_chan;
	int		err = WAN_EC_API_RC_OK;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}
	if (ec_api->fe_chan_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->fe_chan_map = ec_dev->fe_channel_map;
	}
	
	if (!ec_api->fe_chan_map){
		return WAN_EC_API_RC_NOACTION;
	}
	for(fe_chan = ec_dev->fe_start_chan; fe_chan <= ec_dev->fe_stop_chan; fe_chan++){
		if (!(ec_api->fe_chan_map & (1 << fe_chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0) continue;
		err = wanec_channel_mute(
					ec_dev, 
					fe_chan,
					(ec_api->cmd == WAN_EC_API_CMD_CHANNEL_MUTE) ? WAN_TRUE: WAN_FALSE,
					&ec_api->u_chan_mute,
					ec_api->verbose);
		if (err){
			return WAN_EC_API_RC_FAILED;
		}	
	}
	return err;
}

static int wanec_api_dtmf(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		fe_chan, err = WAN_EC_API_RC_OK;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_api->fe_chan_map == 0xFFFFFFFF){
		/* All channels selected */
		ec_api->fe_chan_map = ec_dev->fe_channel_map;
	}
	
	if (!ec_api->fe_chan_map){
		return WAN_EC_API_RC_NOACTION;
	}
	/* Enable/Disable Normal mode on Oct6100 */
	PRINT1(ec_api->verbose,
	"%s: %s Echo Canceller DTMF on channel(s) chan_map=0x%08lX ...\n",
			ec_dev->devname,
			(ec_api->cmd == WAN_EC_API_CMD_DTMF_ENABLE) ? "Enable" :
			(ec_api->cmd == WAN_EC_API_CMD_DTMF_DISABLE) ? "Disable" :
								"Unknown",
			ec_api->fe_chan_map);
	for(fe_chan = ec_dev->fe_start_chan; fe_chan <= ec_dev->fe_stop_chan; fe_chan++){
		if (!(ec_api->fe_chan_map & (1 << fe_chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0) continue;
		err = wanec_channel_dtmf(
				ec_dev, 
				fe_chan,
				(ec_api->cmd == WAN_EC_API_CMD_DTMF_ENABLE) ? WAN_TRUE : WAN_FALSE,
				&ec_api->u_dtmf_config,
				ec_api->verbose);
		if (err){
			return WAN_EC_API_RC_FAILED;
		}
	}
	return err;
}

static int wanec_api_stats(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		fe_chan, ec_chan, err = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	PRINT1(ec_api->verbose,
	"%s: Read EC stats on channel(s) chan_map=0x%08lX reset:%d...\n",
			ec_dev->devname,
			ec_api->fe_chan_map,
			(ec_api->fe_chan_map) ? 
				ec_api->u_chan_stats.reset:ec_api->u_chip_stats.reset);
	if (wanec_ISR(ec, ec_api->verbose)){
		return WAN_EC_API_RC_FAILED;
	}
	if (ec_api->fe_chan_map){
		int	ready = 0;
		for(fe_chan = ec_dev->fe_start_chan; fe_chan <= ec_dev->fe_stop_chan; fe_chan++){
			if (!(ec_api->fe_chan_map & (1 << fe_chan))){
				continue;
			}
			if (!wan_test_bit(fe_chan, &ec_dev->fe_channel_map)){
				continue;
			}
			if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0){
				continue;
			}
			ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
			err = wanec_ChannelStats(
						ec_dev,
						ec_chan,
						&ec_api->u_chan_stats,
						ec_api->verbose);
			if (err){
				return WAN_EC_API_RC_FAILED;
			}
			ready = 1;
			break;
		}
		if (!ready){
			return WAN_EC_API_RC_INVALID_CHANNELS;
		}
	}else{
		wanec_ChipStats(ec_dev, &ec_api->u_chip_stats, ec_api->u_chip_stats.reset, ec_api->verbose);	
	}
			
	return 0;
}

static int wanec_api_stats_image(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	PRINT1(ec_api->verbose,
	"%s: Read EC Image stats ...\n",
			ec_dev->devname);
	wanec_ChipImage(ec_dev, &ec_api->u_chip_image, ec_api->verbose);
	return 0;
}

static int wanec_api_monitor(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		fe_chan = ec_api->fe_chan,
			ec_chan = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	/* Sanity check from channel selection */
	if (fe_chan > ec_dev->fe_stop_chan){
		DEBUG_EVENT(
		"ERROR: %s: Front-End channel number %d is out of range!\n",
				ec_dev->devname, fe_chan);
		return WAN_EC_API_RC_INVALID_CHANNELS;
	}

	if (fe_chan){
		if (!(ec_dev->fe_channel_map & (1 << fe_chan))){
			return WAN_EC_API_RC_INVALID_CHANNELS;
		}
		ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);
		if (wanec_DebugChannel(ec_dev, ec_chan, ec_api->verbose)){
			return WAN_EC_API_RC_FAILED;
		}
	}else{
		if (wanec_DebugGetData(ec_dev, &ec_api->u_chan_monitor, ec_api->verbose)){
			return WAN_EC_API_RC_FAILED;
		}
		ec_api->fe_chan = ec_api->u_chan_monitor.fe_chan;
	}
	return 0;
}

static int wanec_api_buffer(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}
	if (ec_api->cmd == WAN_EC_API_CMD_BUFFER_LOAD){
		err = wanec_BufferLoad(ec_dev,  &ec_api->u_buffer_config, ec_api->verbose);
	}else{
		err = wanec_BufferUnload(ec_dev, &ec_api->u_buffer_config, ec_api->verbose);
	}
	if (err){
		return WAN_EC_API_RC_FAILED;
	}
	return 0;
}

static int wanec_api_playout(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t	*ec = NULL;
	int		fe_chan = ec_api->fe_chan,
			ec_chan = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->state != WAN_EC_STATE_CHIP_READY){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->devname,
				ec->name,
				WAN_EC_STATE_DECODE(ec->state));
		return WAN_EC_API_RC_INVALID_STATE;
	}

	if (ec_dev->fe_media == WAN_MEDIA_E1 && ec_api->fe_chan == 0){
		return WAN_EC_API_RC_NOACTION;
	}
	if (fe_chan > ec_dev->fe_stop_chan){ 
		DEBUG_EVENT(
		"ERROR: %s: Front-End channel number %d is out of range!\n",
				ec_dev->devname, fe_chan);
		return WAN_EC_API_RC_INVALID_CHANNELS;
	}
	if (ec_api->u_playout.port != WAN_EC_CHANNEL_PORT_SOUT && 
	    ec_api->u_playout.port != WAN_EC_CHANNEL_PORT_ROUT){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Canceller port value (%s)!\n",
				ec_dev->devname, 
				WAN_EC_DECODE_CHANNEL_PORT(ec_api->u_playout.port));
		return WAN_EC_API_RC_INVALID_PORT;
	}
	ec_chan = wanec_fe2ec_channel(ec_dev, fe_chan);

	PRINT1(ec_api->verbose,
	"%s: Buffer Playout %s on fe_chan:%d ...\n",
				ec_dev->devname, 
				(ec_api->cmd == WAN_EC_API_CMD_PLAYOUT_START) ? 
							"Start" : "Stop",
				fe_chan);
	switch(ec_api->cmd){	
	case WAN_EC_API_CMD_PLAYOUT_START:
		if (wanec_BufferPlayoutAdd(ec, ec_chan, &ec_api->u_playout, ec_api->verbose)){
			return WAN_EC_API_RC_FAILED;
		}
		if (wanec_BufferPlayoutStart(ec, ec_chan, &ec_api->u_playout, ec_api->verbose)){
			wanec_BufferPlayoutStop(ec, ec_chan, &ec_api->u_playout, ec_api->verbose);
			return WAN_EC_API_RC_FAILED;
		}
		ec->playout_verbose = ec_api->verbose;
		if (ec_api->u_playout.notifyonstop){
			wan_set_bit(WAN_EC_BIT_EVENT_PLAYOUT, &ec_dev->events);
		}
		break;
	case WAN_EC_API_CMD_PLAYOUT_STOP:
		wanec_BufferPlayoutStop(ec, ec_chan, &ec_api->u_playout, ec_api->verbose);
		/* FIXME: Once the Playout event was enabled, do not 
		** disable it otherwise playout events for other 
		** channels will be delayed 
		**ec->playout_verbose = 0;
		**wan_clear_bit(WAN_EC_BIT_EVENT_PLAYOUT, &ec_dev->events);*/
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
				(int)sizeof(wan_ec_api_t),
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
		DEBUG_EVENT(
		"%s: Failed to find device [%s:%d]!\n",
				ec_api->devname, __FUNCTION__,__LINE__);
		ec_api->err = WAN_EC_API_RC_INVALID_DEV;
		goto wanec_ioctl_exit;
	}
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;

	PRINT2(ec_api->verbose,
	"%s: WPEC_LIP IOCTL: %s\n",
			ec_api->devname, WAN_EC_API_CMD_DECODE(ec_api->cmd));
	ec_api->err = WAN_EC_API_RC_OK;
	if (ec_api->cmd == WAN_EC_API_CMD_GETINFO){
		ec_api->u_info.max_channels	= ec->max_channels;
		ec_api->state			= ec->state;
		goto wanec_ioctl_exit;
	}else if (ec_api->cmd == WAN_EC_API_CMD_CONFIG_POLL){
		ec_api->state			= ec->state;
		goto wanec_ioctl_exit;
	}

#if !defined(__WINDOWS__)
	/* Windows: can not copy to/from user when locked */
	wan_spin_lock(&ec->lock);
#endif
	if (wan_test_bit(WAN_EC_BIT_CRIT_DOWN, &ec_dev->critical)){
		DEBUG_EVENT(
		"%s: Echo Canceller device is down!\n",
				ec_api->devname);
		ec_api->err = WAN_EC_API_RC_INVALID_DEV;
		goto wanec_ioctl_done;	
	}
	if (wan_test_and_set_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical)){
		DEBUG_EVENT(
		"%s: Echo Canceller is busy!\n",
				ec_api->devname);
		ec_api->err = WAN_EC_API_RC_BUSY;
		goto wanec_ioctl_done;	
	}
	switch(ec_api->cmd){
	case WAN_EC_API_CMD_GETINFO:
		ec_api->u_info.max_channels	= ec->max_channels;
		ec_api->state			= ec->state;
		break;
	case WAN_EC_API_CMD_CONFIG:
		err = wanec_api_config(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_CONFIG_POLL:
		ec_api->state			= ec->state;
		break;
	case WAN_EC_API_CMD_RELEASE:
		err = wanec_api_release(ec_dev, ec_api, ec_api->verbose);
		break;
	case WAN_EC_API_CMD_CHANNEL_OPEN:
		err = wanec_api_channel_open(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_ENABLE:
	case WAN_EC_API_CMD_DISABLE:
		err = wanec_api_modify(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_BYPASS_ENABLE:
	case WAN_EC_API_CMD_BYPASS_DISABLE:
		err = wanec_api_modify_bypass(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_OPMODE:
		err = wanec_api_chan_opmode(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_CHANNEL_MUTE:
	case WAN_EC_API_CMD_CHANNEL_UNMUTE:
		err = wanec_api_channel_mute(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_MODIFY_CHANNEL:
		err = wanec_api_chan_custom(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_DTMF_ENABLE:
	case WAN_EC_API_CMD_DTMF_DISABLE:
		ec_api->err = wanec_api_dtmf(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_STATS:
	case WAN_EC_API_CMD_STATS_FULL:
		err = wanec_api_stats(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_STATS_IMAGE:
		err = wanec_api_stats_image(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_BUFFER_LOAD:
	case WAN_EC_API_CMD_BUFFER_UNLOAD:
		err = wanec_api_buffer(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_PLAYOUT_START:
	case WAN_EC_API_CMD_PLAYOUT_STOP:
		err = wanec_api_playout(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_MONITOR:
		err = wanec_api_monitor(ec_dev, ec_api);
		break;
	case WAN_EC_API_CMD_RELEASE_ALL:
		break;
	}
	if (err){
		PRINT2(ec_api->verbose,
		"%s: %s return error (Command: %s)\n",
			ec_api->devname, __FUNCTION__, WAN_EC_API_CMD_DECODE(ec_api->cmd));
		ec_api->err = err;
	}
	if (ec_api->err == WAN_EC_API_RC_INVALID_STATE){
		ec_api->state	= ec->state;
	}
	wan_clear_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical);

wanec_ioctl_done:
#if !defined(__WINDOWS__)
	/* Windows: can not copy to/from user when locked */
	wan_spin_unlock(&ec->lock);
#endif

wanec_ioctl_exit:
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
			WAN_EC_API_CMD_DECODE(ec_api->cmd),
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

static void* 
wanec_register(void *pcard, u_int32_t fe_port_mask, int max_line_no, int max_channels, void *pconf)
{
	sdla_t			*card = (sdla_t*)pcard;
	wan_custom_conf_t	*conf = (wan_custom_conf_t*)pconf;
	wan_ec_t		*ec = NULL;
	wan_ec_dev_t		*ec_dev = NULL, *ec_dev_new = NULL;

	WAN_DEBUG_FUNC_START;
#if defined(__WINDOWS__)	
	ec = get_wan_ec_ptr(card);
#else
	WAN_LIST_FOREACH(ec, &wan_ec_head, next){
		WAN_LIST_FOREACH(ec_dev, &ec->ec_dev_head, next){
			if (ec_dev->card == NULL || ec_dev->card == card){
				DEBUG_EVENT("%s: Internal Error (%s:%d)\n",
					card->devname, __FUNCTION__,__LINE__);
				return NULL;
			}
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

		ec->chip_no	= card->hw_iface.get_hwec_index(card->hw);
		if (ec->chip_no < 0){
			DEBUG_EVENT("%s: ERROR: Failed to get EC chip number!\n",
						card->devname);
			wan_free(ec);
			return NULL;
		}
		ec->state		= WAN_EC_STATE_RESET;
		ec->max_channels	= (u_int16_t)max_channels;
		wan_spin_lock_init(&ec->lock, "wan_ec_lock");
		sprintf(ec->name, "%s%d", WANEC_DEV_NAME, ec->chip_no);

		/* Copy EC chip custom configuration */
		if (conf->param_no){
			/* Copy custom oct parameter from user space */
			ec->custom_conf.params = wan_malloc(conf->param_no * sizeof(wan_custom_param_t));
			if (ec->custom_conf.params){
				int	err = 0;
				err = WAN_COPY_FROM_USER(
							ec->custom_conf.params,
							conf->params,
							conf->param_no * sizeof(wan_custom_param_t));
				ec->custom_conf.param_no = conf->param_no; 
			}else{
				DEBUG_EVENT(
				"%s: WARNING: Skipping custom OCT6100 configuration (allocation failed)!\n",
							card->devname);
			}
		}

		Oct6100InterruptServiceRoutineDef(&ec->f_InterruptFlag);

#if defined(__WINDOWS__)
		set_wan_ec_ptr(card, ec);
#else
		WAN_LIST_INIT(&ec->ec_dev_head);
		WAN_LIST_INIT(&ec->ec_confbridge_head);
		WAN_LIST_INSERT_HEAD(&wan_ec_head, ec, next);
#endif
	}else{
#if !defined(__WINDOWS__)
		ec = ec_dev->ec;
#endif
	}
	ec->usage++;
	ec_dev_new->ecdev_no		= wan_ec_devnum(card->devname);
	ec_dev_new->ec			= ec;
	ec_dev_new->name		= ec->name;
	ec_dev_new->card		= card;
	
	ec_dev_new->fe_media		= WAN_FE_MEDIA(&card->fe);
	ec_dev_new->fe_lineno		= WAN_FE_LINENO(&card->fe);
	ec_dev_new->fe_start_chan	= WAN_FE_START_CHANNEL(&card->fe);	
	
	if (IS_A600_CARD(card)) {
		ec_dev_new->fe_max_chans	= 5;	//max_line_no;	//
	} else {
		ec_dev_new->fe_max_chans	= WAN_FE_MAX_CHANNELS(&card->fe);	//max_line_no;	//
	}

	ec_dev_new->fe_stop_chan	= ec_dev_new->fe_start_chan + ec_dev_new->fe_max_chans - 1;
	/* Feb 14, 2008
	** Ignore fe_port_mask for BRI cards. fe_port_mask is for full card, 
	** but ec_dev created per module. In this case, we have always 
	** 2 channels (1 and 2). Create fe_channel_map manually */
	if (fe_port_mask && ec_dev_new->fe_media != WAN_MEDIA_BRI){
		ec_dev_new->fe_channel_map	= fe_port_mask;
	}else{
		int	fe_chan = 0;
		for(fe_chan = ec_dev_new->fe_start_chan; fe_chan <= ec_dev_new->fe_stop_chan; fe_chan++){
			ec_dev_new->fe_channel_map |= (1 << fe_chan);
		}
	}
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
	ec_dev_new->state		= WAN_EC_STATE_RESET;
		
	/* Initialize hwec_bypass pointer */
	card->wandev.ec_enable	= wanec_enable;
	card->wandev.fe_ec_map	= 0;

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
	DEBUG_EVENT("%s: Unregister interface from %s (usage %d)!\n",
					card->devname,
					ec->name,
					ec->usage);
	
	wan_set_bit(WAN_EC_BIT_TIMER_KILL,(void*)&ec_dev->critical);
	wan_set_bit(WAN_EC_BIT_CRIT_DOWN,(void*)&ec_dev->critical);
	wan_set_bit(WAN_EC_BIT_CRIT,(void*)&ec_dev->critical);
	wan_del_timer(&ec_dev->timer);				
	
	if (ec_dev->state != WAN_EC_STATE_RESET){
		DEBUG_EVENT(
		"%s: Releasing EC device (force)!\n",
					card->devname);
		wanec_api_release(ec_dev, NULL, wanec_verbose);
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

	ec_dev->ec = NULL;
	wan_free(ec_dev);
	wan_spin_unlock(&ec->lock);

	/* FIXME: Remove character device */
	if (!ec->usage){
#if !defined(__WINDOWS__)
		if (WAN_LIST_FIRST(&wan_ec_head) == ec){
			WAN_LIST_FIRST(&wan_ec_head) =
					WAN_LIST_NEXT(ec, next);
			WAN_LIST_NEXT(ec, next) = NULL;
		}else{
			WAN_LIST_REMOVE(ec, next);
		}
#endif
		
		/* Free all custom configuration parameters */ 
		if (ec->custom_conf.params){
			wan_free(ec->custom_conf.params);
		}

		wan_free(ec);

#if defined(__WINDOWS__)
		set_wan_ec_ptr(card, NULL);
#endif

	}
	WAN_DEBUG_FUNC_END;
	return 0;
}

#define WAN_EC_IRQ_TIMEOUT		(HZ*60)
#define WAN_EC_DTMF_IRQ_TIMEOUT		(HZ/32)
#define WAN_EC_PLAYOUT_IRQ_TIMEOUT	(HZ/32)
static int wanec_isr(void *arg)
{
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;
	
	if (ec_dev == NULL || ec_dev->ec == NULL) return 0;
	if (ec_dev->ec->state != WAN_EC_STATE_CHIP_READY){
		return 0;
	}
	if (wan_test_bit(WAN_EC_BIT_EVENT_DTMF, &ec_dev->events)){
		/* DTMF event is enabled */
		if ((SYSTEM_TICKS - ec_dev->ec->lastint_ticks) > WAN_EC_DTMF_IRQ_TIMEOUT){
			ec_dev->ec->lastint_ticks = SYSTEM_TICKS;
			return 1; 
		}
		return 0;
	}
	if (wan_test_bit(WAN_EC_BIT_EVENT_PLAYOUT, &ec_dev->events)){
		/* Playout event is enabled */
		if ((SYSTEM_TICKS - ec_dev->ec->lastint_ticks) > WAN_EC_PLAYOUT_IRQ_TIMEOUT){
			ec_dev->ec->lastint_ticks = SYSTEM_TICKS;
			return 1; 
		}
		return 0;
	}

	if ((SYSTEM_TICKS - ec_dev->ec->lastint_ticks) > WAN_EC_IRQ_TIMEOUT){
		ec_dev->ec->lastint_ticks = SYSTEM_TICKS;
		return 1; 
	}
	return 0;
}

static int wanec_poll(void *arg, void *pcard)
{
	wan_ec_t	*ec = NULL;
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;
	int		err = 0;
	
	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	
	WAN_DEBUG_FUNC_START;

#if defined(__WINDOWS__)
	wan_spin_lock(&ec->lock);
#else
	if (!wan_spin_trylock(&ec->lock)){	
		return -EBUSY;
	}
#endif

	wan_clear_bit(WAN_EC_BIT_TIMER_RUNNING,(void*)&ec_dev->critical);
	if (wan_test_bit(WAN_EC_BIT_CRIT_DOWN, &ec_dev->critical) || 
	    wan_test_bit(WAN_EC_BIT_CRIT_ERROR, &ec_dev->critical)){
		ec_dev->poll_cmd = WAN_EC_POLL_NONE;
		wan_spin_unlock(&ec->lock);
		return -EINVAL;
	}
	switch(ec_dev->poll_cmd){
	case WAN_EC_POLL_CHIPOPENPENDING:
		/* Chip open */
		if (ec->state != WAN_EC_STATE_CHIP_OPEN_PENDING){
			DEBUG_EVENT(
			"%s: Invalid EC state at ChipOpenPending poll command (%02X)\n",
						ec->name, ec->state);
			ec->state = WAN_EC_STATE_READY;
			ec_dev->state = ec->state;
			ec_dev->poll_cmd = WAN_EC_POLL_NONE;
			err = -EINVAL;
			goto wanec_poll_done;
		}
		if (wanec_ChipOpen(ec_dev, WAN_EC_VERBOSE_NONE)){
			ec->f_OpenChip.pbyImageFile = NULL;
			if (ec->pImageData) wan_vfree(ec->pImageData);
			ec->pImageData = NULL;
			/* Chip state is Ready state */
			ec->state = WAN_EC_STATE_READY;
			ec_dev->state = ec->state;
			ec_dev->poll_cmd = WAN_EC_POLL_NONE;
			err = -EINVAL;
			goto wanec_poll_done;
		}	
		ec->state = WAN_EC_STATE_CHIP_OPEN;
		ec_dev->state = ec->state;

		ec->f_OpenChip.pbyImageFile = NULL;
		if (ec->pImageData) wan_vfree(ec->pImageData);
		ec->pImageData = NULL;
		break;
	
	case WAN_EC_POLL_INTR:
	default:	/* by default, can be only schedule from interrupt */
		if (ec->state != WAN_EC_STATE_CHIP_READY){
			break;
		}

		if (wan_test_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical)) {
			err = -EINVAL;
			break;
		}
		
		/* Execute interrupt routine */
		err = wanec_ISR(ec, wanec_verbose);
		if (err < 0){
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
	
	wan_spin_lock(&ec->lock);
	if (wan_test_and_set_bit(WAN_EC_BIT_CRIT_CMD, &ec->critical)){
		wan_spin_unlock(&ec->lock);
		return -EBUSY;
	}

	switch(event_ctrl->type){
	case WAN_EVENT_EC_DTMF:
		if (event_ctrl->mode == WAN_EVENT_ENABLE){
			wanec_channel_dtmf(ec_dev, event_ctrl->channel, WAN_TRUE, NULL, WAN_EC_VERBOSE_EXTRA2/*wanec_verbose*/);
		}else{
			wanec_channel_dtmf(ec_dev, event_ctrl->channel, WAN_FALSE, NULL, wanec_verbose);
		}
		break;
	case WAN_EVENT_EC_H100_REPORT:
		if (event_ctrl->mode == WAN_EVENT_DISABLE){
			ec->ignore_H100 = 1;
		}else{
			ec->ignore_H100 = 0;
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
	wan_spin_unlock(&ec->lock);
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

#if defined(WAN_DEBUG_MEM)
	wanec_memdbg_init();
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
	wanec_iface.isr		= wanec_isr;
	wanec_iface.poll	= wanec_poll;
	wanec_iface.event_ctrl	= wanec_event_ctrl;
	
#if defined(CONFIG_WANPIPE_HWEC)
	register_wanec_iface (&wanec_iface);
# if defined(__FreeBSD__) || defined(__OpenBSD__)
	wp_cdev_reg(NULL, WANEC_DEV_NAME, wanec_ioctl);
# elif defined(__LINUX__) || defined(__WINDOWS__)
	wanec_create_dev();
# endif
#endif
	return 0;
}

int wanec_exit (void *arg)
{
#if defined(CONFIG_WANPIPE_HWEC)
# if defined(__FreeBSD__) || defined(__OpenBSD__)
	wp_cdev_unreg(WANEC_DEV_NAME);
# elif defined(__LINUX__) || defined(__WINDOWS__)
	wanec_remove_dev();
# endif
	unregister_wanec_iface();
#endif

#if defined(WAN_DEBUG_MEM)
	wanec_memdbg_free();
#endif
	DEBUG_EVENT("WANEC Layer: Unloaded\n");
	return 0;
}

#if 0
int wanec_shutdown(void *arg)
{
	DEBUG_EVENT("Shutting down WANEC module ...\n");
	return 0;
}

int wanec_ready_unload(void *arg)
{
	DEBUG_EVENT("Is WANEC module ready to unload...\n");
	return 0;
}
#endif

#if !defined(__WINDOWS__)
WAN_MODULE_DEFINE(
		wanec,"wanec", 
		"Alex Feldman <al.feldman@sangoma.com>",
		"Wanpipe Echo Canceller Layer - Sangoma Tech. Copyright 2006", 
		"GPL",
		wanec_init, wanec_exit, /*wanec_shutdown, wanec_ready_unload,*/
		NULL);

WAN_MODULE_DEPEND(wanec, wanrouter, 1,
			WANROUTER_MAJOR_VER, WANROUTER_MAJOR_VER);

WAN_MODULE_DEPEND(wanec, wanpipe, 1,
			WANPIPE_MAJOR_VER, WANPIPE_MAJOR_VER);
#endif
