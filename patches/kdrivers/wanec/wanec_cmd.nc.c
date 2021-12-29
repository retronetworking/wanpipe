/*************************************************************
 * wanec_cmd.c   WANPIPE Echo Canceller Layer (WANEC_LIP)
 *
 *
 *
 * ===========================================================
 *
 * May 10 2006		Alex Feldman	Initial Version
 *
 * March 19, 2006	Alex Feldman	Enable Sout Adaptive Noise
 *					Reduction for all channel by
 *					default.
 */


/*=============================================================
 * Includes
 */

#if defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe.h>
#elif defined(__LINUX__)
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe.h>
# include <linux/if_wanpipe.h>
#elif defined(__WINDOWS__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe.h>
#endif

int verbose;

#include "oct6100_api.h"
#include "oct6100_version.h"

#include "wanec_iface.h"
#include "wanec_tones.h"

/*=============================================================
 * Definitions
 */
#define WANEC_MAX_PORT_RANGE		32
#define WANEC_MAX_BRI_PORT_RANGE	2
#define WANEC_READ_LIMIT		0x10000

#define WANEC_MAX_CONFBRIDGE_DEF	32
#define WANEC_MAC_PLAYOUT_BUFFERS	20
<<<<<<< wanec_cmd.c

//FIXME: Take this out
#warning "WAN_MEDIA_BRI is localy defined to 0: do not commit"
#define WAN_MEDIA_BRI	0
=======

>>>>>>> 1.52
/*=============================================================
 * Global Parameters
 */
UINT32 DetectedSoutToneNumbers[WAN_NUM_DTMF_TONES] = 
{
	SOUT_DTMF_0,
	SOUT_DTMF_1,
	SOUT_DTMF_2,
	SOUT_DTMF_3,
	SOUT_DTMF_4,
	SOUT_DTMF_5,
	SOUT_DTMF_6,
	SOUT_DTMF_7,
	SOUT_DTMF_8,
	SOUT_DTMF_9,
	SOUT_DTMF_A,
	SOUT_DTMF_B,
	SOUT_DTMF_C,
	SOUT_DTMF_D,
	SOUT_DTMF_STAR,
	SOUT_DTMF_POUND,
};
UINT32 DetectedRoutToneNumbers[WAN_NUM_DTMF_TONES] = 
{
	ROUT_DTMF_0,
	ROUT_DTMF_1,
	ROUT_DTMF_2,
	ROUT_DTMF_3,
	ROUT_DTMF_4,
	ROUT_DTMF_5,
	ROUT_DTMF_6,
	ROUT_DTMF_7,
	ROUT_DTMF_8,
	ROUT_DTMF_9,
	ROUT_DTMF_A,
	ROUT_DTMF_B,
	ROUT_DTMF_C,
	ROUT_DTMF_D,
	ROUT_DTMF_STAR,
	ROUT_DTMF_POUND
};

/*=============================================================
 * Function prototype
*/

int wanec_ChipOpenPrep(wan_ec_dev_t *ec_dev, char *devname, wanec_config_t *config, int verbose);
int wanec_ChipOpen(wan_ec_dev_t*, int);
int wanec_ChipClose(wan_ec_dev_t*, int verbose);
int wanec_ChipStats(wan_ec_dev_t *ec_dev, wanec_chip_stats_t *chip_stats, int reset, int verbose);

int wanec_ChannelOpen(wan_ec_dev_t*, int);
int wanec_ChannelClose(wan_ec_dev_t*, int);
int wanec_ChannelModify(wan_ec_dev_t*, INT, wanec_chan_modify_t*, int verbose);
int wanec_ChannelStats(wan_ec_dev_t*, INT channel, wanec_chan_stats_t *chan_stats, int reset);

int wanec_ChannelMute(wan_ec_dev_t*, INT channel, unsigned char port_mask, int);
int wanec_ChannelUnMute(wan_ec_dev_t*, INT channel, unsigned char port_mask, int);

int wanec_TonesEnable(wan_ec_t *ec, int channel, unsigned char port, int verbose);
int wanec_TonesDisable(wan_ec_t *ec, int channel, unsigned char port, int verbose);

int wanec_DebugChannel(wan_ec_dev_t*, INT channel, int verbose);
int wanec_DebugGetData(wan_ec_dev_t*, wanec_chan_monitor_t *chan_monitor, int verbose);

int wanec_BufferLoad(wan_ec_dev_t *ec_dev, wanec_tone_config_t *tone_config, int verbose);
int wanec_BufferUnload(wan_ec_dev_t *ec_dev, wanec_tone_config_t *tone_config, int verbose);
int wanec_BufferPlayoutAdd(wan_ec_t *ec, int channel, wanec_playout_t *playout, int verbose);
int wanec_BufferPlayoutStart(wan_ec_t *ec, int channel, wanec_playout_t *playout, int verbose);
int wanec_BufferPlayoutStop(wan_ec_t *ec, int channel, wanec_playout_t *playout, int verbose);

int wanec_ConfBridgeOpen(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int verbose);
int wanec_ConfBridgeClose(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int verbose);
int wanec_ConfBridgeChanAdd(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int verbose);
int wanec_ConfBridgeChanRemove(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int verbose);
int wanec_ConfBridgeChanMute(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int verbose);
int wanec_ConfBridgeChanUnMute(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int verbose);
int wanec_ConfBridgeDominantSpeakerSet(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int enable, int verbose);
int wanec_ConfBridgeMaskChange(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, UINT32 mask, int verbose);
int wanec_ConfBridgeGetStats(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int verbose);

int wanec_EventTone(wan_ec_t *ec, int verbose);
int wanec_ISR(wan_ec_t *ec, int verbose);

int wanec_fe2ec_channel(wan_ec_dev_t*, int);
static int wanec_hndl2ec_channel(wan_ec_t *ec, UINT32 ChannelHndl);
static int wanec_ec2fe_channel(wan_ec_t*, int, wan_ec_dev_t**);

extern int wanec_ChipParam(wan_ec_t*,tPOCT6100_CHIP_OPEN, wan_custom_conf_t*, int);
extern int wanec_ChanParam(wan_ec_t*,tPOCT6100_CHANNEL_MODIFY, wan_custom_conf_t*, int);
extern int wanec_ChanParamList(wan_ec_t *ec);

u32 wanec_req_write(void*, u32 write_addr, u16 write_data);
u32 wanec_req_write_smear(void*, u32 addr, u16 data, u32 len);
u32 wanec_req_write_burst(void*, u32 addr, u16 *data, u32 len);
u32 wanec_req_read(void*, u32 addr, u16 *data);
u32 wanec_req_read_burst(void*, u32 addr, u16 *data, u32 len);

extern int wan_ec_write_internal_dword(wan_ec_dev_t *ec_dev, u32 addr1, u32 data);
extern int wan_ec_read_internal_dword(wan_ec_dev_t *ec_dev, u32 addr1, u32 *data);

/*=============================================================
 * Function definition
*/
static int wanec_hndl2ec_channel(wan_ec_t *ec, UINT32 ChannelHndl)
{
	int	channel = 0;

	for(channel = 0; channel < ec->max_channels; channel++){
		if (ec->pEchoChannelHndl[channel] == ChannelHndl){
			return channel;
		}
	}
	return 0;
}
int wanec_fe2ec_channel(wan_ec_dev_t *ec_dev, int fe_channel)
{
	int	ec_channel = 0;

	if (ec_dev->fe_media == WAN_MEDIA_BRI){
		if (ec_dev->fe_lineno >= 12){
			ec_channel = WANEC_MAX_PORT_RANGE;
		}
		ec_channel += (ec_dev->fe_lineno * WANEC_MAX_BRI_PORT_RANGE + (fe_channel-1));
	}else if (ec_dev->fe_media == WAN_MEDIA_T1 || ec_dev->fe_media == WAN_MEDIA_FXOFXS){
		ec_channel = ec_dev->fe_lineno * WANEC_MAX_PORT_RANGE + (fe_channel-1);
	}else{
		/*ec_channel = ec_dev->fe_lineno * ec_dev->fe_max_channels + channel;*/
		ec_channel = ec_dev->fe_lineno * WANEC_MAX_PORT_RANGE + fe_channel;
	}
	return ec_channel;
}

static int wanec_ec2fe_channel(wan_ec_t *ec, int ec_chan, wan_ec_dev_t **ec_dev)
{
	int	fe_chan;

	*ec_dev = ec->pEcDevMap[ec_chan];
	if (*ec_dev == NULL) return 0;

	fe_chan = ec_chan % WANEC_MAX_PORT_RANGE;
	if ((*ec_dev)->fe_media == WAN_MEDIA_BRI){
		fe_chan = fe_chan % WANEC_MAX_BRI_PORT_RANGE;
		fe_chan++;
	}else{
		if ((*ec_dev)->fe_media == WAN_MEDIA_T1 ||
		    (*ec_dev)->fe_media == WAN_MEDIA_FXOFXS){
			fe_chan++; 
		}
	}
	return fe_chan;
}


/******************************************************************
**
**
*******************************************************************/
tOCT6100_CHIP_IMAGE_INFO	f_ChipImageInfo;
#if 0
tOCT6100_GET_HW_REVISION	f_Revision;
#endif
int wanec_ChipStats(wan_ec_dev_t *ec_dev, wanec_chip_stats_t *chip_stats, int reset, int verbose)
{
	wan_ec_t		*ec;
	tOCT6100_CHIP_STATS	f_ChipStats;
	UINT32			ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	
	if (chip_stats){
		PRINT1(verbose, "%s: Reading chip statistics...\n",
					ec->name);
	}
	Oct6100ChipGetStatsDef( &f_ChipStats );
	f_ChipStats.fResetChipStats	= reset;
	ulResult = Oct6100ChipGetStats(
				ec->pChipInstance,
				&f_ChipStats);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"%s: Reading chip statistics...\tFailed (err=0x%X)\n",
					ec->name, ulResult);
		return -EINVAL;
	}
	if (chip_stats){
		memcpy(	&chip_stats->f_ChipStats,
			&f_ChipStats,
			sizeof(tOCT6100_CHIP_STATS));
	}

	if (chip_stats){
		PRINT1(verbose, "%s: Reading chip image info...\n",
					ec->name); 
	}

	Oct6100ChipGetImageInfoDef( &f_ChipImageInfo );
	ulResult = Oct6100ChipGetImageInfo(
				ec->pChipInstance,
				&f_ChipImageInfo);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"%s: Reading chip image info...\tFailed (err=0x%X)\n",
					ec->name, ulResult); 
		return -EINVAL;
	}
	if (chip_stats){
		if (chip_stats->f_ChipImageInfo){
			int err;
			err = WAN_COPY_TO_USER(
					&f_ChipImageInfo,
					chip_stats->f_ChipImageInfo,
					sizeof(tOCT6100_CHIP_IMAGE_INFO));
			if (err){
				DEBUG_EVENT(
				"%s: Failed to copy chip image info to user space [%s:%d]!\n",
						ec->name, __FUNCTION__,__LINE__);
				return -EINVAL;
			}
		}else{
			PRINT1(verbose,
			"%s: Echo Canceller image description:\n%s\n",
					ec->name, f_ChipImageInfo.szVersionNumber);
			PRINT1(verbose,
			"%s: Echo Canceller image build ID\t\t\t%08X\n",
					ec->name, f_ChipImageInfo.ulBuildId);
			PRINT1(verbose,
			"%s: Echo Canceller maximum number of channels\t%d\n",
					ec->name, f_ChipImageInfo.ulMaxChannels);	
#if 0	
			PRINT1(verbose,
			"%s: Echo Canceller maximum tail displacement\t\t%d\n",
					ec->name, f_ChipImageInfo.ulMaxTailDisplacement);	
			PRINT1(verbose,
			"%s: Echo Canceller per channel tail displacement\t%s\n",
					ec->name,
					(f_ChipImageInfo.fPerChannelTailDisplacement == TRUE)?"TRUE":"FALSE");
			PRINT1(verbose,
			"%s: Echo Canceller per channel tail length\t\t%s\n",
					ec->name,
					(f_ChipImageInfo.fPerChannelTailLength == TRUE)?"TRUE":"FALSE");
			PRINT1(verbose,
			"%s: Echo Canceller maximum tail length\t\t%d\n",
					ec->name, f_ChipImageInfo.ulMaxTailLength);	
			PRINT1(verbose,
			"%s: Echo Canceller buffer Playout support\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fBufferPlayout == TRUE)?"TRUE":"FALSE");	
			PRINT1(verbose,
			"%s: Echo Canceller adaptive noise reduction\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fAdaptiveNoiseReduction==TRUE)?"TRUE":"FALSE");	
			PRINT1(verbose,
			"%s: Echo Canceller SOUT noise bleaching\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fSoutNoiseBleaching==TRUE)?"TRUE":"FALSE");	
			PRINT1(verbose,
			"%s: Echo Canceller ROUT noise reduction\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fRoutNoiseReduction==TRUE)?"TRUE":"FALSE");	
			PRINT1(verbose,
			"%s: Echo Canceller ROUT noise reduction level\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fRoutNoiseReductionLevel==TRUE)?"TRUE":"FALSE");	
			PRINT1(verbose,
			"%s: Echo Canceller automatic level control\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fAutoLevelControl==TRUE)?"TRUE":"FALSE");	
			PRINT1(verbose,
			"%s: Echo Canceller acoustic echo cancellation\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fAcousticEcho==TRUE)?"TRUE":"FALSE");	
			PRINT1(verbose,
			"%s: Echo Canceller conferencing\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fConferencing==TRUE)?"TRUE":"FALSE");	
			PRINT1(verbose,
			"%s: Echo Canceller conferencing noise reduction\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fConferencingNoiseReduction==TRUE)?"TRUE":"FALSE");
#endif
		}
	}
	if (f_ChipImageInfo.ulMaxChannels < (unsigned int)ec->max_channels){
		ec->max_channels = f_ChipImageInfo.ulMaxChannels-1;
	}
	
//	DEBUG_EVENT(verbose, "%s: Reading hw revision...\n",
//					ec->name); 
#if 0
	DEBUG_EVENT(verbose, "%s: Reading hw revision...\n",
					ec->name); 
	ec->f_Revision.ulUserChipId = 0;
	ec->f_Revision.pProcessContext = &ec->f_Context;
	ulResult = Oct6100GetHwRevision(&ec->f_Revision);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"%s: Reading hw revision...\tFailed (err=0x%X)\n",
					ec->name, ulResult); 
		return EINVAL;
	}
#endif	
	return 0;
}

int wanec_ChipOpenPrep(wan_ec_dev_t *ec_dev, char *devname, wanec_config_t *config, int verbose)
{
	tOCT6100_GET_INSTANCE_SIZE	InstanceSize;
	wan_ec_t			*ec;
	UINT32				ulResult;
	
	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(config->imageData == NULL);
	ec	= ec_dev->ec;

	ulResult = Oct6100ChipOpenDef( &ec->f_OpenChip );

	/*==============================================================*/
	/* Configure clocks */

	/* upclk oscillator is at 33.33 Mhz */
	ec->f_OpenChip.ulUpclkFreq = cOCT6100_UPCLK_FREQ_33_33_MHZ;

	/* mclk will be generated by internal PLL at 133 Mhz */
	ec->f_OpenChip.fEnableMemClkOut = TRUE;
#if 1
	ec->f_OpenChip.ulMemClkFreq = cOCT6100_MCLK_FREQ_133_MHZ;
#else
	ec->f_OpenChip.ulMemClkFreq = cOCT6100_MCLK_FREQ_125_MHZ; /*125*/
#endif

	/*==============================================================*/

	/*==============================================================*/
	/* General parameters */

	/* Chip ID.*/	
	ec->f_OpenChip.ulUserChipId	= ec->chip_no;

	/* Set the max number of accesses to 1024 to speed things up */
	ec->f_OpenChip.ulMaxRwAccesses	= 1024;

	/* Set the maximums that the chip needs to support for this test */
	ec->f_OpenChip.ulMaxChannels	= config->max_channels;

	ec->f_OpenChip.ulMaxBiDirChannels	= 0;
	ec->f_OpenChip.ulMaxConfBridges		= 0;	//WANEC_MAX_CONFBRIDGE_DEF;
	ec->f_OpenChip.ulMaxPhasingTssts	= 0;
	ec->f_OpenChip.ulMaxTdmStreams		= 32;
	ec->f_OpenChip.ulMaxTsiCncts		= 2;

	/*==============================================================*/

	/*==============================================================*/
	/* External Memory Settings */

	/* Use DDR memory.*/
#if 1
	ec->f_OpenChip.ulMemoryType	= cOCT6100_MEM_TYPE_SDR;
#else
	ec->f_OpenChip.ulMemoryType	= cOCT6100_MEM_TYPE_DDR;
#endif
	
	ec->f_OpenChip.ulNumMemoryChips	= 2;

	/*
	**f_OpenChip.ulMemoryChipSize	= cOCT6100_MEMORY_CHIP_SIZE_8MB;
	**f_OpenChip.ulMemoryChipSize	= cOCT6100_MEMORY_CHIP_SIZE_16MB;*/
	ec->f_OpenChip.ulMemoryChipSize	= config->memory_chip_size;

	ec->f_OpenChip.fEnableChannelRecording	= TRUE;

#if defined(ENABLE_ACOUSTICECHO)
	ec->f_OpenChip.fEnableAcousticEcho	= TRUE;
#endif

#if defined(ENABLE_PRODBIST)
	/* Enable production bist mode */
	ec->f_OpenChip.fEnableProductionBist	= TRUE;
	ec->f_OpenChip.ulNumProductionBistLoops	= 0x1;
#endif
	
	ec->f_OpenChip.ulMaxPlayoutBuffers	= WANEC_MAC_PLAYOUT_BUFFERS;

	ec->f_OpenChip.pbyImageFile	= ec->pImageData;
	ec->f_OpenChip.ulImageSize	= ec->ImageSize;
		
	/* Assign board index (0). */
 	ec->f_Context.ulBoardId = ec->chip_no;
	
	/* Handle to driver */
	ec->f_Context.ec_dev = ec_dev;

	/* Interface name to driver */
	strlcpy(ec->f_Context.devname, devname, WAN_DRVNAME_SZ);

	ulResult = Oct6100GetInstanceSize(&ec->f_OpenChip, &InstanceSize);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to get EC chip instance size (err=0x%X)!\n",
					ec->name, ulResult);
		return -EINVAL;
	}

	/* Allocate memory needed for API instance. */
	ec->pChipInstance = 
		(tPOCT6100_INSTANCE_API)wan_vmalloc(InstanceSize.ulApiInstanceSize);
	if (ec->pChipInstance == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed to allocate memory for EC chip (%d bytes)!\n",
					ec->name,InstanceSize.ulApiInstanceSize);
		return -EINVAL;
	}

	/* Open the OCT6100 on the evaluation board. */
	ec->f_OpenChip.pProcessContext	= (PVOID)&ec->f_Context;

	/* parse advanced params (global custom configuration) */
	if (ec->custom_conf.param_no){
		wanec_ChipParam(ec, &ec->f_OpenChip, &ec->custom_conf, verbose);
	}
	/* parse advanced params (command line custom configuration) */
	if (config->custom_conf.param_no){
		wanec_ChipParam(ec, &ec->f_OpenChip, &config->custom_conf, verbose);
	}

	ec->ulDebugChannelHndl	= cOCT6100_INVALID_HANDLE;
	ec->ulDebugDataMode	= config->debug_data_mode;	
	return 0;
}

int wanec_ChipOpen(wan_ec_dev_t *ec_dev, int verbose)
{	
	wan_ec_t	*ec;
	UINT32		ulResult, i = 0;
	INT		ec_chan = 0;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec	= ec_dev->ec;
	
	PRINT1(verbose,
	"%s: Opening Echo Canceller Chip ...\n",
					ec->name);
	if (ec->f_OpenChip.pbyImageFile == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Invalid EC image pointer\n",
					ec->name);
		return -EINVAL;
	}
	ulResult = Oct6100ChipOpen( 
			ec->pChipInstance,	/* API instance memory. */
			&ec->f_OpenChip );		/* Open chip structure. */
	if ( ulResult != cOCT6100_ERR_OK ){
		if (ec->imageLast == WANOPT_YES || 
		    ulResult != cOCT6100_ERR_OPEN_INVALID_FIRMWARE_OR_CAPACITY_PINS){
			DEBUG_EVENT(
			"ERROR: %s: Failed to open Echo Canceller Chip (err=0x%X)\n",
					ec->name, ulResult);
		}
		return -EINVAL;
	}

	if (wanec_ChipStats(ec_dev, NULL, TRUE, verbose)){
		DEBUG_EVENT(
		"ERROR: %s: Failed to read EC chip statistics!\n",
					ec->name);
		wanec_ChipClose(ec_dev, verbose);
		return EINVAL;
	}

	ec->pToneBufferIndexes = 
			wan_malloc(sizeof(UINT32) * ec->f_OpenChip.ulMaxPlayoutBuffers);
	if (ec->pToneBufferIndexes == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed allocate memory for playout handles!\n",
					ec->name);
		wanec_ChipClose(ec_dev, verbose);
		return EINVAL;
	}
	i = 0;
	while(i < ec->f_OpenChip.ulMaxPlayoutBuffers){
		ec->pToneBufferIndexes[i++] = cOCT6100_INVALID_VALUE;
	}
	ec->pEchoChannelHndl = 
			wan_malloc(sizeof(UINT32) * ec->max_channels);
	if (ec->pEchoChannelHndl == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed allocate memory for channel handle!\n",
					ec->name);
		wan_free(ec->pToneBufferIndexes);
		wanec_ChipClose(ec_dev, verbose);
		return EINVAL;
	}
	ec->pEcDevMap = wan_malloc(sizeof(wan_ec_dev_t*) * ec->max_channels);
	if (ec->pEcDevMap == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed allocate memory for ec channel map!\n",
					ec->name);
		wan_free(ec->pToneBufferIndexes);
		wan_free(ec->pEchoChannelHndl);
		wanec_ChipClose(ec_dev, verbose);
		return -EINVAL;
	}
	for(ec_chan = 0; ec_chan < ec->max_channels; ec_chan++){
		ec->pEchoChannelHndl[ec_chan] = cOCT6100_INVALID_HANDLE;
		ec->pEcDevMap[ec_chan] = NULL;
	}
	return 0;
}

int wanec_ChipClose(wan_ec_dev_t *ec_dev, int verbose)
{
	wan_ec_t		*ec;
	tOCT6100_CHIP_CLOSE	f_CloseChip;
	UINT32			ulResult;
	
	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	PRINT1(verbose,
	"%s: Closing Echo Canceller Chip ...\n",
				ec->name);
	Oct6100ChipCloseDef( &f_CloseChip );
	ulResult = Oct6100ChipClose( 
				ec->pChipInstance,
				&f_CloseChip );
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to close Echo Canceller chip (err=0x%X)!\n",
						ec->name, ulResult); 
		return -EINVAL;
	}
	if (ec->pChipInstance){
		wan_vfree(ec->pChipInstance);
		ec->pChipInstance = NULL;
	}
	if (ec->pToneBufferIndexes){
		wan_free(ec->pToneBufferIndexes);
		ec->pToneBufferIndexes = NULL;
	}
	if (ec->pEchoChannelHndl){
		wan_free(ec->pEchoChannelHndl);
		ec->pEchoChannelHndl = NULL;
	}
	if (ec->pEcDevMap){
		wan_free(ec->pEcDevMap);
		ec->pEcDevMap = NULL;
	}
	return 0;
}

int wanec_ChannelOpen(wan_ec_dev_t *ec_dev, int verbose)
{
	tOCT6100_CHANNEL_OPEN	EchoChannelOpen;
	wan_ec_t		*ec;
	sdla_t			*card;
	UINT32			ulResult;
	UINT32			stream = 0,timeslot=0;
	INT			channel, pcm_law_type;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_dev->card == NULL);
	ec = ec_dev->ec;
	card = (sdla_t*)ec_dev->card;
	
	pcm_law_type = (WAN_FE_TDMV_LAW(&card->fe) == WAN_TDMV_MULAW) ? 
				cOCT6100_PCM_U_LAW : cOCT6100_PCM_A_LAW;	
	PRINT1(verbose,
	"%s: Openning all Echo Canceller channels (%s)...\n",
				ec->name,
				(pcm_law_type == cOCT6100_PCM_U_LAW) ?
						"MULAW":"ALAW");

	for(channel = 0; channel < ec->max_channels; channel++){
		Oct6100ChannelOpenDef( &EchoChannelOpen );

		/* Assign the handle memory.*/
		EchoChannelOpen.pulChannelHndl = &ec->pEchoChannelHndl[channel];

		/* Make sure the channel does not perform echo cancellation */
#if defined(WANEC_BYDEFAULT_NORMAL)
		EchoChannelOpen.ulEchoOperationMode =
					cOCT6100_ECHO_OP_MODE_NORMAL;
#else	
		EchoChannelOpen.ulEchoOperationMode =
					cOCT6100_ECHO_OP_MODE_POWER_DOWN;
#endif
		EchoChannelOpen.fEnableToneDisabler = 	TRUE;

		stream = (channel % 2 == 1) ? 4 : 0;
		timeslot = channel / 2;

		/* Configure the TDM interface.*/
		EchoChannelOpen.TdmConfig.ulRinPcmLaw		= pcm_law_type; 
		EchoChannelOpen.TdmConfig.ulRoutPcmLaw		= pcm_law_type; 
		EchoChannelOpen.TdmConfig.ulSinPcmLaw		= pcm_law_type; 
		EchoChannelOpen.TdmConfig.ulSoutPcmLaw		= pcm_law_type; 

		EchoChannelOpen.TdmConfig.ulRinStream		= stream + 0;
		EchoChannelOpen.TdmConfig.ulRinTimeslot		= timeslot;
		EchoChannelOpen.TdmConfig.ulRoutStream		= stream + 1;
		EchoChannelOpen.TdmConfig.ulRoutTimeslot	= timeslot;
		EchoChannelOpen.TdmConfig.ulSinStream		= stream + 2;
		EchoChannelOpen.TdmConfig.ulSinTimeslot		= timeslot;
		EchoChannelOpen.TdmConfig.ulSoutStream		= stream + 3;
		EchoChannelOpen.TdmConfig.ulSoutTimeslot	= timeslot;

		/* Set the desired VQE features (TRUE/FALSE).*/
		EchoChannelOpen.VqeConfig.fEnableNlp		= TRUE;
		EchoChannelOpen.VqeConfig.fRinDcOffsetRemoval	= TRUE;
		EchoChannelOpen.VqeConfig.fSinDcOffsetRemoval	= TRUE;
#if defined(ENABLE_ACOUSTICECHO)
		EchoChannelOpen.VqeConfig.fAcousticEcho		= TRUE;
#endif

		EchoChannelOpen.VqeConfig.fSoutAdaptiveNoiseReduction = TRUE;
		EchoChannelOpen.VqeConfig.ulComfortNoiseMode	= 
					cOCT6100_COMFORT_NOISE_NORMAL;
				/*	cOCT6100_COMFORT_NOISE_NORMAL
				**	cOCT6100_COMFORT_NOISE_EXTENDED,
				**	cOCT6100_COMFORT_NOISE_OFF,
				**	cOCT6100_COMFORT_NOISE_FAST_LATCH */

		PRINT1(verbose,
		"%s: Openning Echo Canceller channel %d (%s)...\n",
				ec->name,
				channel,
				(pcm_law_type == cOCT6100_PCM_U_LAW) ?
						"MULAW":"ALAW");
		/* Open the channel.*/
		ulResult = Oct6100ChannelOpen(	ec->pChipInstance,
						&EchoChannelOpen );
		if (ulResult != cOCT6100_ERR_OK){
			DEBUG_EVENT(
			"ERROR: %s: Failed to open Echo Canceller channel %d (err=0x%X)!\n",
							ec->name,
							channel,
							ulResult);
			return ulResult;
		}
	}

	/* Init debug channel handle */
	ec->ulDebugChannelHndl = cOCT6100_INVALID_HANDLE;

	return 0;
}

int wanec_ChannelClose(wan_ec_dev_t *ec_dev, int verbose)
{
	wan_ec_t		*ec;
	tOCT6100_CHANNEL_CLOSE	EchoChannelClose;
	UINT32			ulResult, channel;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	PRINT1(verbose,
	"%s: Closing all Echo Canceller channels ...\n",
					ec->name);
	for(channel = 0; channel < (UINT32)ec->max_channels; channel++){
		Oct6100ChannelCloseDef( &EchoChannelClose );
		EchoChannelClose.ulChannelHndl =
					ec->pEchoChannelHndl[channel];
		ulResult = Oct6100ChannelClose(	ec->pChipInstance,
						&EchoChannelClose );
		if (ulResult != cOCT6100_ERR_OK){
			DEBUG_EVENT(
			"ERROR: %s: Failed to close Echo Canceller channel %d (err=0x%X)!\n",
							ec->name,
							channel,
							ulResult);
			return -EINVAL;
		}
		ec->pEchoChannelHndl[channel] = cOCT6100_INVALID_HANDLE;
	}
	return 0;
}

int wanec_ChannelModify(	wan_ec_dev_t		*ec_dev,
				INT			channel,
				wanec_chan_modify_t	*chan_modify,
				int			verbose)
{
	wan_ec_t		*ec;
	tOCT6100_CHANNEL_MODIFY	EchoChannelModify;
	UINT32			ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;

	Oct6100ChannelModifyDef( &EchoChannelModify );

	/* Assign the handle memory.*/
	EchoChannelModify.ulChannelHndl = ec->pEchoChannelHndl[channel];

	/* parse advianced params */
	if (chan_modify->custom_conf.param_no){
		int err;
		err = wanec_ChanParam(ec, &EchoChannelModify, &chan_modify->custom_conf, verbose);
		if (err){
			DEBUG_EVENT(
			"%s: WARNING: Unsupported parameter for channel %d!\n",
					ec->name, channel);
			return -EINVAL;
		}
	}else{

		/* Echo Channel Operation Mode */	
		EchoChannelModify.ulEchoOperationMode = chan_modify->opmode;
			
		if (chan_modify->mute != WANEC_IGNORE){
			EchoChannelModify.fVqeConfigModified = TRUE;
			if (chan_modify->mute){
				EchoChannelModify.VqeConfig.fDtmfToneRemoval = TRUE;
			}else{
				EchoChannelModify.VqeConfig.fDtmfToneRemoval = FALSE;
			}
		}
	}

	/* Open the channel.*/
	ulResult = Oct6100ChannelModify( 
					ec->pChipInstance,
					&EchoChannelModify );
	if (ulResult != cOCT6100_ERR_OK){
		PRINT1(verbose,
		"%s: Failed to modify Channel config parameters for channel %d (err=0x%X)\n",
				ec->name,
				channel,
				ulResult);
		return EINVAL;
	}
	return	0;
}

/******************************************************************************
**			CONFERENCE BRIDGE FUNCTIONS
******************************************************************************/
int wanec_ChannelMute(wan_ec_dev_t* ec_dev, INT channel, unsigned char port_mask, int verbose)
{
	wan_ec_t		*ec;
	tOCT6100_CHANNEL_MUTE	f_ChannelMute;
	UINT32			ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;

	PRINT1(verbose, "%s: Muting channel %d on port %X...\n",
				ec->name, channel, port_mask);
	Oct6100ChannelMuteDef( &f_ChannelMute );
	f_ChannelMute.ulChannelHndl	= ec->pEchoChannelHndl[channel];
	f_ChannelMute.ulPortMask	= cOCT6100_CHANNEL_MUTE_PORT_NONE;
	if (port_mask & WAN_EC_CHANNEL_PORT_SOUT)
		f_ChannelMute.ulPortMask |= cOCT6100_CHANNEL_MUTE_PORT_SOUT; 
	if (port_mask & WAN_EC_CHANNEL_PORT_SIN)
		f_ChannelMute.ulPortMask |= cOCT6100_CHANNEL_MUTE_PORT_SIN; 
	if (port_mask & WAN_EC_CHANNEL_PORT_ROUT)
		f_ChannelMute.ulPortMask |= cOCT6100_CHANNEL_MUTE_PORT_ROUT; 
	if (port_mask & WAN_EC_CHANNEL_PORT_RIN)
		f_ChannelMute.ulPortMask |= cOCT6100_CHANNEL_MUTE_PORT_RIN; 
	ulResult = Oct6100ChannelMute(
				ec->pChipInstance,
				&f_ChannelMute);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to mute channel %d on port %X (%08X)!\n",
						ec->name, channel, port_mask, ulResult);
		return EINVAL;
	}
	return 0;
}
int wanec_ChannelUnMute(wan_ec_dev_t *ec_dev, INT channel, unsigned char port_mask, int verbose)
{
	wan_ec_t		*ec;
	tOCT6100_CHANNEL_UNMUTE	f_ChannelUnMute;
	UINT32			ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;

	PRINT1(verbose, "%s: Un-muting channel %d on port %X...\n",
				ec->name, channel, port_mask);
	Oct6100ChannelUnMuteDef( &f_ChannelUnMute );
	f_ChannelUnMute.ulChannelHndl	= ec->pEchoChannelHndl[channel];
	f_ChannelUnMute.ulPortMask	= cOCT6100_CHANNEL_MUTE_PORT_NONE;
	if (port_mask & WAN_EC_CHANNEL_PORT_SOUT)
		f_ChannelUnMute.ulPortMask |= cOCT6100_CHANNEL_MUTE_PORT_SOUT; 
	if (port_mask & WAN_EC_CHANNEL_PORT_SIN)
		f_ChannelUnMute.ulPortMask |= cOCT6100_CHANNEL_MUTE_PORT_SIN; 
	if (port_mask & WAN_EC_CHANNEL_PORT_ROUT)
		f_ChannelUnMute.ulPortMask |= cOCT6100_CHANNEL_MUTE_PORT_ROUT; 
	if (port_mask & WAN_EC_CHANNEL_PORT_RIN)
		f_ChannelUnMute.ulPortMask |= cOCT6100_CHANNEL_MUTE_PORT_RIN; 
	ulResult = Oct6100ChannelUnMute(
				ec->pChipInstance,
				&f_ChannelUnMute);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to un-mute channel %d on port %X (%08X)!\n",
						ec->name, channel, port_mask, ulResult);
		return EINVAL;
	}
	return 0;
}

int wanec_ChannelStats(wan_ec_dev_t *ec_dev, INT channel, wanec_chan_stats_t *chan_stats, int reset)
{
	wan_ec_t		*ec;
	tOCT6100_CHANNEL_STATS	f_ChannelStats;
	UINT32			ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;

	PRINT1(verbose, "%s: Reading EC statistics for channel %d...\n",
				ec->name, channel);
	Oct6100ChannelGetStatsDef( &f_ChannelStats );
	f_ChannelStats.ulChannelHndl = ec->pEchoChannelHndl[channel];
	f_ChannelStats.fResetStats = reset;
	ulResult = Oct6100ChannelGetStats(
				ec->pChipInstance,
				&f_ChannelStats);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to read EC stats for channel %d (%08X)!\n",
						ec->name, channel, ulResult);
		return EINVAL;
	}
	if (chan_stats){
		memcpy(	&chan_stats->f_ChannelStats,
			&f_ChannelStats,
			sizeof(tOCT6100_CHANNEL_STATS));
	}
	return 0;
}

int wanec_TonesEnable(wan_ec_t *ec, int channel, unsigned char type, int verbose)
{
	tOCT6100_TONE_DETECTION_ENABLE	f_ToneDetectionEnable;
	UINT32				ulResult;
	int				i;

	PRINT1(verbose, "%s: Enable tone detection on ec chan %d ...\n",
					ec->name,
					channel);
					
	if (type & WAN_EC_CHANNEL_PORT_ROUT){
		for(i = 0; i < WAN_NUM_DTMF_TONES; i++){
	
			Oct6100ToneDetectionEnableDef( &f_ToneDetectionEnable );
			f_ToneDetectionEnable.ulChannelHndl = 
						ec->pEchoChannelHndl[channel];
			f_ToneDetectionEnable.ulToneNumber =
						DetectedRoutToneNumbers[i];
			ulResult = Oct6100ToneDetectionEnable (
						ec->pChipInstance,
						&f_ToneDetectionEnable);
			if ( ulResult == cOCT6100_ERR_OK ){
				continue;
			}else if (ulResult == cOCT6100_ERR_TONE_DETECTION_TONE_ACTIVATED){
				PRINT1(verbose,
				"%s: Tone detection is already enabled on channel %d for port ROUT!\n",
					ec->name, channel);
				continue;	/* already activated */
			}else{
				DEBUG_EVENT(
				"ERROR: %s: Failed to enable tone detection on ec chan %d!\n",
					ec->name, channel);
				DEBUG_EVENT(
				"ERROR: %s: (err=0x%X,i=%d)!\n",
					ec->name,
					(unsigned int)ulResult, i);
				return -EINVAL;
			}
		}
	}
	if (type & WAN_EC_CHANNEL_PORT_SOUT){
		for(i = 0; i < WAN_NUM_DTMF_TONES; i++){
	
			Oct6100ToneDetectionEnableDef( &f_ToneDetectionEnable );
			f_ToneDetectionEnable.ulChannelHndl = 
						ec->pEchoChannelHndl[channel];
			f_ToneDetectionEnable.ulToneNumber =
						DetectedSoutToneNumbers[i];
			ulResult = Oct6100ToneDetectionEnable (
						ec->pChipInstance,
						&f_ToneDetectionEnable);
			if ( ulResult == cOCT6100_ERR_OK ){
				continue;
			}else if (ulResult == cOCT6100_ERR_TONE_DETECTION_TONE_ACTIVATED){
				PRINT1(verbose,
				"%s: Tone detection is already enabled on channel %d for port SOUT!\n",
					ec->name, channel);
				continue;	/* already activated */
			}else{
				DEBUG_EVENT(
				"ERROR: %s: Failed to enable tone detection on channel %d!\n",
					ec->name, channel);
				DEBUG_EVENT(
				"ERROR: %s: (err=0x%X,i=%d)!\n",
					ec->name,
					(unsigned int)ulResult, i);
				return -EINVAL;
			}
		}
	}

	return 0;
}

int wanec_TonesDisable(wan_ec_t *ec, int channel, unsigned char type, int verbose)
{
	tOCT6100_TONE_DETECTION_DISABLE	f_ToneDetectionDisable;
	UINT32				ulResult;
	INT				i;

	PRINT1(verbose, "%s: Disable tone detection on channel %d ...\n",
					ec->name,
					channel);
	if (type & WAN_EC_CHANNEL_PORT_ROUT){
	
		for(i = 0; i < WAN_NUM_DTMF_TONES; i++){
		
			Oct6100ToneDetectionDisableDef( &f_ToneDetectionDisable );
			f_ToneDetectionDisable.ulChannelHndl = 
						ec->pEchoChannelHndl[channel];
			if (channel >= 0){
				f_ToneDetectionDisable.ulToneNumber =
						DetectedRoutToneNumbers[i];
			}else{
				f_ToneDetectionDisable.fDisableAll = TRUE;
			}
			ulResult = Oct6100ToneDetectionDisable (
							ec->pChipInstance,
							&f_ToneDetectionDisable);
			if ( ulResult != cOCT6100_ERR_OK ){
				DEBUG_EVENT(
				"ERROR: %s: Failed to disable tone detection for channel %d (err=0x%X,i=%d)!\n", 
						ec->name, channel,
						(unsigned int)ulResult, i);
				return -EINVAL;
			}
		}
	}
	if (type & WAN_EC_CHANNEL_PORT_SOUT){	
		
		for(i = 0; i < WAN_NUM_DTMF_TONES; i++){
		
			Oct6100ToneDetectionDisableDef( &f_ToneDetectionDisable );
			f_ToneDetectionDisable.ulChannelHndl = 
						ec->pEchoChannelHndl[channel];
			if (channel >= 0){
				f_ToneDetectionDisable.ulToneNumber =
							DetectedSoutToneNumbers[i];
			}else{
				f_ToneDetectionDisable.fDisableAll = TRUE;
			}
			ulResult = Oct6100ToneDetectionDisable (
							ec->pChipInstance,
							&f_ToneDetectionDisable);
			if ( ulResult != cOCT6100_ERR_OK ){
				DEBUG_EVENT(
				"ERROR: %s: Failed to disable tone detection for channel %d (err=0x%X,i=%d)!\n", 
						ec->name, channel,
						(unsigned int)ulResult, i);
				return -EINVAL;
			}
		}
	}
	return 0;
}

//#if defined(WAN_DEBUG_HWEC)
#if 1
static CHAR* wanec_ToneId2Str(UINT32 f_ulToneId)
{
	switch (f_ulToneId){ 
	/* DTMF Section */
	case ROUT_DTMF_0: return "ROUT_DTMF_0";
	case ROUT_DTMF_1: return "ROUT_DTMF_1";
	case ROUT_DTMF_2: return "ROUT_DTMF_2";
	case ROUT_DTMF_3: return "ROUT_DTMF_3";
	case ROUT_DTMF_4: return "ROUT_DTMF_4";
	case ROUT_DTMF_5: return "ROUT_DTMF_5";
	case ROUT_DTMF_6: return "ROUT_DTMF_6";
	case ROUT_DTMF_7: return "ROUT_DTMF_7";
	case ROUT_DTMF_8: return "ROUT_DTMF_8";
	case ROUT_DTMF_9: return "ROUT_DTMF_9";
	case ROUT_DTMF_A: return "ROUT_DTMF_A";
	case ROUT_DTMF_B: return "ROUT_DTMF_B";
	case ROUT_DTMF_C: return "ROUT_DTMF_C";
	case ROUT_DTMF_D: return "ROUT_DTMF_D";
	case ROUT_DTMF_STAR: return "ROUT_DTMF_STAR";
	case ROUT_DTMF_POUND: return "ROUT_DTMF_POUND";
	case SOUT_DTMF_0: return "SOUT_DTMF_0";
	case SOUT_DTMF_1: return "SOUT_DTMF_1";
	case SOUT_DTMF_2: return "SOUT_DTMF_2";
	case SOUT_DTMF_3: return "SOUT_DTMF_3";
	case SOUT_DTMF_4: return "SOUT_DTMF_4";
	case SOUT_DTMF_5: return "SOUT_DTMF_5";
	case SOUT_DTMF_6: return "SOUT_DTMF_6";
	case SOUT_DTMF_7: return "SOUT_DTMF_7";
	case SOUT_DTMF_8: return "SOUT_DTMF_8";
	case SOUT_DTMF_9: return "SOUT_DTMF_9";
	case SOUT_DTMF_A: return "SOUT_DTMF_A";
	case SOUT_DTMF_B: return "SOUT_DTMF_B";
	case SOUT_DTMF_C: return "SOUT_DTMF_C";
	case SOUT_DTMF_D: return "SOUT_DTMF_D";
	case SOUT_DTMF_STAR: return "SOUT_DTMF_STAR";
	case SOUT_DTMF_POUND: return "SOUT_DTMF_POUND";

	/* System 5/6/7 Section */
	case SIN_SYSTEM5_2400: return "SIN_SYSTEM5_2400";
	case SIN_SYSTEM5_2600: return "SIN_SYSTEM5_2600";
	case SIN_SYSTEM5_2400_2600: return "SIN_SYSTEM5_2400_2600";
	case SIN_SYSTEM7_2000: return "SIN_SYSTEM7_2000";
	case SIN_SYSTEM7_1780: return "SIN_SYSTEM7_1780";

	default: return "INVALID TONE ID!";
	}
}
#endif

static unsigned char wanec_ConvertToneId(UINT32 f_ulToneId, unsigned char *ec_dtmf_port)
{
	switch (f_ulToneId){ 
	/* DTMF Section */
	case ROUT_DTMF_0:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '0';
	case ROUT_DTMF_1:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '1';
	case ROUT_DTMF_2:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '2';
	case ROUT_DTMF_3:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '3';
	case ROUT_DTMF_4:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '4';
	case ROUT_DTMF_5:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '5';
	case ROUT_DTMF_6:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '6';
	case ROUT_DTMF_7:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '7';
	case ROUT_DTMF_8:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '8';
	case ROUT_DTMF_9:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '9';
	case ROUT_DTMF_A:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return 'A';
	case ROUT_DTMF_B:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return 'B';
	case ROUT_DTMF_C:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return 'C';
	case ROUT_DTMF_D:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return 'D';
	case ROUT_DTMF_STAR:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '*';
	case ROUT_DTMF_POUND:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_ROUT; return '#';
	case SOUT_DTMF_0:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '0';
	case SOUT_DTMF_1:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '1';
	case SOUT_DTMF_2:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '2';
	case SOUT_DTMF_3:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '3';
	case SOUT_DTMF_4:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '4';
	case SOUT_DTMF_5:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '5';
	case SOUT_DTMF_6:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '6';
	case SOUT_DTMF_7:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '7';
	case SOUT_DTMF_8:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '8';
	case SOUT_DTMF_9:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '9';
	case SOUT_DTMF_A:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return 'A';
	case SOUT_DTMF_B:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return 'B';
	case SOUT_DTMF_C:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return 'C';
	case SOUT_DTMF_D:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return 'D';
	case SOUT_DTMF_STAR:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '*';
	case SOUT_DTMF_POUND:	*ec_dtmf_port = WAN_EC_CHANNEL_PORT_SOUT; return '#';
	}
	return 0x00000000;
}

#if 1
//#if defined(WAN_DEBUG_HWEC)
static CHAR* wanec_ToneType2Str(UINT32 f_ulToneType)
{
	switch (f_ulToneType){ 
	case cOCT6100_TONE_PRESENT:	return "cOCT6100_TONE_PRESENT";
	case cOCT6100_TONE_STOP:	return "cOCT6100_TONE_STOP";
	default: return "INVALID TONE TYPE!";
	}
}
#endif
static unsigned char wanec_ConvertToneType(UINT32 f_ulToneType)
{
	switch (f_ulToneType){ 
	case cOCT6100_TONE_PRESENT:	return WAN_EC_TONE_PRESENT;
	case cOCT6100_TONE_STOP:	return WAN_EC_TONE_STOP;
	}
	return 0x00;
}


/*
**				wanec_EventTone()
**
** Return:	0  - on success
**		<0 - on error
**		1  - pending dtmf events
**/
int wanec_EventTone(wan_ec_t *ec, int verbose)
{
	tOCT6100_EVENT_GET_TONE	f_GetToneEvent;
	tOCT6100_TONE_EVENT	ToneEvent[32];
	UINT32			ulResult;
	wan_ec_dev_t		*ec_dev;
	sdla_t			*card;
	UINT32			i;
	int			ec_chan,fe_chan;

	PRINT1(verbose, "%s: Getting Tone events ...\n",
					ec->name);
	Oct6100EventGetToneDef( &f_GetToneEvent );
	f_GetToneEvent.fResetBufs = FALSE;
	f_GetToneEvent.ulMaxToneEvent = 32;
	f_GetToneEvent.pToneEvent = ToneEvent;
	ulResult = Oct6100EventGetTone(
					ec->pChipInstance,
					&f_GetToneEvent);
	if ( ulResult != cOCT6100_ERR_OK ){
		if ( ulResult != cOCT6100_ERR_EVENTS_TONE_BUF_EMPTY ){
			PRINT1(verbose, "%s: There are not tone events!\n",
							ec->name);
			return 0;
		}
		DEBUG_EVENT(
		"ERROR: %s: Failed to get tone events (err=0x%X)!\n",
						ec->name, ulResult);
		return -EINVAL;
	}

	/* No dtmf tone event returned */
	if (!f_GetToneEvent.ulNumValidToneEvent) return 0;

	for(i = 0; i < f_GetToneEvent.ulNumValidToneEvent; i++){
		ec_chan = wanec_hndl2ec_channel(ec, ToneEvent[i].ulChannelHndl);
		ec_dev = ec->pEcDevMap[ec_chan];
		fe_chan = wanec_ec2fe_channel(ec, ec_chan, &ec_dev);
		if (ec_dev == NULL || ec_dev->card == NULL){
			DEBUG_EVENT(
			"%s: Internal Error: Failed to find fe channel (ec_chan=%d)\n",
						ec->name, ec_chan);
			continue;
		}

		PRINT1(verbose,
		"%s: Tone event %s %s on fe_chan=%d ec_chan=%d\n", 
			ec_dev->devname,
			wanec_ToneId2Str(ToneEvent[i].ulToneDetected),
			wanec_ToneType2Str(f_GetToneEvent.pToneEvent[i].ulEventType),
			fe_chan, ec_chan);

		card = (sdla_t*)ec_dev->card;
		if (card->wandev.event_callback.dtmf){
			wan_event_t	event;
			unsigned char	dtmf_port = WAN_EC_CHANNEL_PORT_ROUT, dtmf_type;
			
			event.type	= WAN_EVENT_EC_DTMF;
			event.channel	= fe_chan;
			event.digit	= wanec_ConvertToneId(
						ToneEvent[i].ulToneDetected,
						&dtmf_port);
			dtmf_type	= wanec_ConvertToneType(ToneEvent[i].ulEventType);
			event.dtmf_type = dtmf_type;
			event.dtmf_port = dtmf_port;
			card->wandev.event_callback.dtmf(card, &event);
		}
	}

	/* Return 1 if more dtmf event are present, otherwise - 0 */
	return (f_GetToneEvent.fMoreEvents == TRUE) ? 1 : 0;
}

/*
**				wanec_ISR()
**
** Return:	0  - on success
**		<0 - on error
**		1  - pending dtmf events
**/
int wanec_ISR(wan_ec_t *ec, int verbose)
{
	UINT32	ulResult;
	int	ret = 0;

	WAN_ASSERT(ec == NULL);

	Oct6100InterruptServiceRoutineDef(&ec->f_InterruptFlag);

	ulResult = Oct6100InterruptServiceRoutine( 
					ec->pChipInstance,
					&ec->f_InterruptFlag );
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to execute interrupt Service Routine (err=%08X)!\n",
					ec->name,
					ulResult); 
		return -EINVAL;
	}
	/* Critical errors */
	if (ec->f_InterruptFlag.fFatalGeneral == TRUE){
		DEBUG_EVENT(
		"%s: An internal fatal chip error detected (0x%X)!\n",
				ec->name,
				ec->f_InterruptFlag.ulFatalGeneralFlags);
	}
	if (ec->f_InterruptFlag.fFatalReadTimeout == TRUE){
		DEBUG_EVENT(
		"%s: A read to the external memory has failed!\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fErrorRefreshTooLate == TRUE){
		DEBUG_EVENT(
		"%s: Error Refresh Too Late!\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fErrorPllJitter == TRUE){
		DEBUG_EVENT(
		"%s: Error Pll Jitter\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fErrorH100OutOfSync == TRUE){
		DEBUG_EVENT(
		"%s: The H100 slave has lost its framing on the bus!\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fErrorH100ClkA == TRUE){
		DEBUG_EVENT(
		"%s: The CT_C8_A clock behavior does not conform to the H.100 spec!\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fErrorH100FrameA == TRUE){
		DEBUG_EVENT(
		"%s: The CT_FRAME_A clock behavior does not comform to the H.100 spec!\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fErrorH100ClkB == TRUE){
		DEBUG_EVENT(
		"%s: The CT_C8_B clock is not running a 16.384 MHz!\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fErrorOverflowToneEvents == TRUE){
		DEBUG_EVENT(
		"%s: Error: Tone Event buffer has overflowed\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fToneEventsPending == TRUE){
		PRINT1(verbose, "%s: Tone Event pending....\n",
				ec->name);
		ret = wanec_EventTone(ec, verbose);
	}
	if (ec->f_InterruptFlag.fBufferPlayoutEventsPending == TRUE){
		PRINT1(verbose, 
		"%s: Buffer Playout Events Pending\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fApiSynch == TRUE){
		PRINT1(verbose, 
		"%s: The chip interrupted the API for purpose of maintaining sync!\n",
				ec->name);
	}
	return ret;
}

int wanec_DebugChannel(wan_ec_dev_t *ec_dev, INT channel, int verbose)
{
	wan_ec_t			*ec = NULL;
	tOCT6100_DEBUG_SELECT_CHANNEL	DebugSelectChannel;
	wanec_chan_stats_t		chan_stats;
	UINT32				ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	/* Verify Echo Canceller Channel operation mode */
	ulResult = wanec_ChannelStats(ec_dev, channel, &chan_stats, 0);
	if (chan_stats.f_ChannelStats.ulEchoOperationMode == cOCT6100_ECHO_OP_MODE_POWER_DOWN){
		DEBUG_EVENT(
		"ERROR: %s: Invalid Echo Channel %d operation mode (POWER DOWN)!\n",
				ec_dev->name, channel);
		return -EINVAL;
	}

	if (ec_dev->ec->ulDebugChannelHndl != cOCT6100_INVALID_HANDLE){
		DEBUG_EVENT(
		"ERROR: %s: Echo Canceller daemon can monitor only one ec channel (%d)!\n",
				ec_dev->name, channel);
		return -EINVAL;	
	}
	Oct6100DebugSelectChannelDef( &DebugSelectChannel );

	PRINT1(verbose, "%s: Select ec channel %d for monitoring...\n",
			ec_dev->name,
			channel);
	/* Set selected debug channel */
	ec->DebugChannel		= channel;
	ec->ulDebugChannelHndl		= ec->pEchoChannelHndl[channel];
	DebugSelectChannel.ulChannelHndl= ec->pEchoChannelHndl[channel];

	/* Select Debug channel */
	ulResult = Oct6100DebugSelectChannel( 
					ec->pChipInstance,
					&DebugSelectChannel );
	if (ulResult != cOCT6100_ERR_OK){
		DEBUG_EVENT(
		"ERROR: %s: Failed to select debug ec channel %d for monitoring (err=0x%X)\n",
				ec->name,
				channel,
				ulResult);
		return -EINVAL;
	}
	return	0;
}

int wanec_DebugGetData(wan_ec_dev_t *ec_dev, wanec_chan_monitor_t *chan_monitor, int verbose)
{
	wan_ec_t			*ec = NULL;
	tOCT6100_DEBUG_GET_DATA	fDebugGetData;
	UINT32			ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	if (ec->ulDebugChannelHndl == cOCT6100_INVALID_HANDLE){
		PRINT(
#if !defined(__WINDOWS__)
			verbose,
#endif
		"ERROR: %s: No Debug channel was selected!\n",
					ec->name);
		return -EINVAL;
	}
		
	Oct6100DebugGetDataDef( &fDebugGetData );

	PRINT1(verbose,
	"%s: Retrieves debug data for ec channel %d...\n",
			ec->name,
			ec->DebugChannel);

	memset(&chan_monitor->data[0], 0,
				sizeof(UINT8) * (MAX_MONITOR_DATA_LEN+1));
	/* Set selected debug channel */
	fDebugGetData.ulGetDataMode	= ec->ulDebugDataMode;
	fDebugGetData.ulMaxBytes	= chan_monitor->max_len;
	fDebugGetData.pbyData		= &chan_monitor->data[0];

	/* Select Debug channel */
	ulResult = Oct6100DebugGetData( 
			ec->pChipInstance,
			&fDebugGetData );
	if (ulResult != cOCT6100_ERR_OK){
		PRINT(
#if !defined(__WINDOWS__)
			verbose,
#endif
		"ERROR: %s: Failed to get debug data for ec channel %d (err=0x%X)\n",
				ec->name,
				ec->DebugChannel,
				ulResult);
		return -EINVAL;
	}
	chan_monitor->data_len		= fDebugGetData.ulValidNumBytes;
	chan_monitor->remain_len	= fDebugGetData.ulRemainingNumBytes;
	chan_monitor->channel		= ec->DebugChannel;
	
	if (fDebugGetData.ulRemainingNumBytes == 0){
		/* Last read */
		ec->ulDebugChannelHndl = cOCT6100_INVALID_HANDLE;
	}

	return	0;
}

static PUINT32 wanec_search_bufferindex(wan_ec_t *ec, UINT32 index)
{
	UINT32	i = 0;
	
	while(i < ec->f_OpenChip.ulMaxPlayoutBuffers){
		if (ec->pToneBufferIndexes[i] == index){
			return &ec->pToneBufferIndexes[i];
		}
		i++;
	}
	return NULL;
}

int wanec_BufferLoad(wan_ec_dev_t *ec_dev, wanec_tone_config_t *tone_config, int verbose)
{
	wan_ec_t		*ec;
	tOCT6100_BUFFER_LOAD	BufferLoad;
	UINT32			size, ulResult;
	PUINT8			pData = NULL;
	int			err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	PRINT1(verbose,
	"%s: Loading Tone buffer (%s) into OCT6100 Chip ...\n",
					ec->name, tone_config->tone);
	size = tone_config->size * sizeof(INT8);
	pData = wan_vmalloc(size);
	if (pData == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed to allocate memory for tone buffer!\n",
					ec->name);
		return -EINVAL;
	}
	err = WAN_COPY_FROM_USER(pData, tone_config->data, size);
	if (err){
		DEBUG_EVENT(
		"ERROR: %s: Failed to copy EC tone buffer from user space [%s:%d]!\n",
				ec->name,
				__FUNCTION__,__LINE__);
		wan_vfree(pData);
		return -EINVAL;
	}
	
	Oct6100BufferPlayoutLoadDef( &BufferLoad );
	BufferLoad.pulBufferIndex = wanec_search_bufferindex(ec, cOCT6100_INVALID_VALUE);
	/* FIXME: Can be alaw/mulaw */
	BufferLoad.ulBufferPcmLaw = 
			(ec_dev->fe_tdmv_law == WAN_TDMV_MULAW) ?
							cOCT6100_PCM_U_LAW :
							cOCT6100_PCM_A_LAW;
	BufferLoad.pbyBufferPattern = pData;
	BufferLoad.ulBufferSize = size;
	ulResult = Oct6100BufferPlayoutLoad (
					ec->pChipInstance,
					&BufferLoad);
	if ( ulResult != cOCT6100_ERR_OK ){
		if (ulResult == cOCT6100_ERR_BUFFER_PLAYOUT_ALL_BUFFERS_OPEN){
			goto buffer_load_done;
		}
		DEBUG_EVENT(
		"%s: ERROR: Failed to load tone buffer into EC Chip (err=0x%X)\n",
						ec->name, ulResult);
		wan_vfree(pData);
		return -EINVAL;
	}
buffer_load_done:
	wan_vfree(pData);
	tone_config->buffer_index = *BufferLoad.pulBufferIndex;
	PRINT1(verbose,
	"%s: Current tone index is %d\n",
			ec->name, tone_config->buffer_index);
	return 0;
}

int wanec_BufferUnload(wan_ec_dev_t *ec_dev, wanec_tone_config_t *tone_config, int verbose)
{
	wan_ec_t		*ec;
	tOCT6100_BUFFER_UNLOAD	BufferUnload;
	PUINT32			pBufferIndex = NULL;
	UINT32			index = 0, ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	PRINT1(verbose,
	"%s: Unloading Tone buffer from EC chip ...\n",
					ec->name);

try_next_index:					
	Oct6100BufferPlayoutUnloadDef( &BufferUnload );
	if (tone_config->buffer_index != cOCT6100_INVALID_VALUE){
		pBufferIndex = wanec_search_bufferindex(ec, tone_config->buffer_index);
		if (pBufferIndex == NULL){
			DEBUG_EVENT(
			"ERROR: %s: Invalid tone buffer index %X!\n",
					ec->name, tone_config->buffer_index);
			return EINVAL;
		}
	}else{
		if (index > ec->f_OpenChip.ulMaxPlayoutBuffers){
			goto buffer_unload_done;
		}
		if (ec->pToneBufferIndexes[index] == cOCT6100_INVALID_VALUE){
			index++;
			goto try_next_index;
		}
		pBufferIndex = &ec->pToneBufferIndexes[index];
	}
	
	BufferUnload.ulBufferIndex = *pBufferIndex;
	ulResult = Oct6100BufferPlayoutUnload (
					ec->pChipInstance,
					&BufferUnload);
	if ( ulResult != cOCT6100_ERR_OK ){
		if (ulResult == cOCT6100_ERR_BUFFER_PLAYOUT_NOT_OPEN){
			goto buffer_unload_done;
		}
		DEBUG_EVENT(
		"ERROR: %s: Failed to unload tone buffer from EC Chip (err=0x%X)!\n",
				ec->name, (unsigned int)ulResult);
		return EINVAL;
	}
	*pBufferIndex = 0;
	if (!tone_config->buffer_index){
		index++;
		goto try_next_index;	
	}

buffer_unload_done:	
	return 0;
}

int wanec_BufferPlayoutAdd(wan_ec_t *ec, int channel, wanec_playout_t *playout, int verbose)
{
	tOCT6100_BUFFER_PLAYOUT_ADD	BufferPlayoutAdd;
	UINT32				ulResult;

	PRINT1(verbose,
	"%s: Add Tone buffer to ec channel %d...\n",
					ec->name, channel);
	if (playout->index == cOCT6100_INVALID_VALUE||
	    wanec_search_bufferindex(ec, playout->index) == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Invalid playout buffer index for ec channel %d!\n",
					ec->name, channel);
		return -EINVAL;
	}
	Oct6100BufferPlayoutAddDef( &BufferPlayoutAdd );
	BufferPlayoutAdd.fRepeat	= playout->repeat;
	BufferPlayoutAdd.ulPlayoutPort	= cOCT6100_CHANNEL_PORT_ROUT;
	BufferPlayoutAdd.ulMixingMode	= cOCT6100_MIXING_MUTE;
	BufferPlayoutAdd.ulChannelHndl	= ec->pEchoChannelHndl[channel];
	BufferPlayoutAdd.ulBufferIndex	= playout->index;
	BufferPlayoutAdd.ulDuration	= (playout->duration) ?
						playout->duration : 5000;
	BufferPlayoutAdd.ulBufferLength	= (playout->buffer_length) ?
						playout->buffer_length : 
						cOCT6100_AUTO_SELECT;
	ulResult = Oct6100BufferPlayoutAdd(
					ec->pChipInstance,
					&BufferPlayoutAdd);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to add playout buffer to ec channel %d (err=%08X)\n",
					ec->name, channel, ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_BufferPlayoutStart(wan_ec_t *ec, int channel, wanec_playout_t *playout, int verbose)
{
	tOCT6100_BUFFER_PLAYOUT_START	BufferPlayoutStart;
	UINT32				ulResult;

	PRINT1(verbose,
	"%s: Active playout buffer on ec channel %d...\n",
					ec->name,
					channel);
	Oct6100BufferPlayoutStartDef( &BufferPlayoutStart );
	BufferPlayoutStart.ulChannelHndl = ec->pEchoChannelHndl[channel];
	BufferPlayoutStart.ulPlayoutPort = cOCT6100_CHANNEL_PORT_ROUT;
	ulResult = Oct6100BufferPlayoutStart(
					ec->pChipInstance,
					&BufferPlayoutStart);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to active playout buffer on ec channel %d (err=%08X)\n",
					ec->name,
					channel,
					ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_BufferPlayoutStop(wan_ec_t *ec, int channel, wanec_playout_t *playout, int verbose)
{
	tOCT6100_BUFFER_PLAYOUT_STOP	BufferPlayoutStop;
	UINT32				ulResult;

	PRINT1(verbose,
	"%s: Deactive playout buffer on ec channel %d...\n",
					ec->name,
					channel);
	Oct6100BufferPlayoutStopDef( &BufferPlayoutStop );
	BufferPlayoutStop.ulChannelHndl = ec->pEchoChannelHndl[channel];
	BufferPlayoutStop.ulPlayoutPort = cOCT6100_CHANNEL_PORT_ROUT;
	ulResult = Oct6100BufferPlayoutStop(
					ec->pChipInstance,
					&BufferPlayoutStop);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to deactive playout buffer on ec channel %d (err=%08X)\n",
					ec->name,
					channel,
					ulResult);
		return -EINVAL;
	}
	return 0;
}


/******************************************************************************
**			CONFERENCE BRIDGE FUNCTIONS
******************************************************************************/
int wanec_ConfBridgeOpen(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int verbose)
{
	tOCT6100_CONF_BRIDGE_OPEN	ConfBridgeOpen;
	UINT32				ulResult;

	PRINT1(verbose,
	"%s: Opening Conference Bridge...\n", ec->name);
	
	if (ec->confbridges_no >= ec->f_OpenChip.ulMaxConfBridges){
		DEBUG_EVENT(
		"ERROR: %s: Trying to open too many conference bridges (%d:%d)\n",
					ec->name,
					ec->confbridges_no, 
					ec->f_OpenChip.ulMaxConfBridges);
		return -EINVAL;
	}

	Oct6100ConfBridgeOpenDef( &ConfBridgeOpen );
	ConfBridgeOpen.pulConfBridgeHndl	= &confbridge->ulHndl;
	ConfBridgeOpen.fFlexibleConferencing	= FALSE;
	ulResult = Oct6100ConfBridgeOpen(
				ec->pChipInstance,
				&ConfBridgeOpen);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to open new conference bridge (err=%08X)\n",
					ec->name,
					ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_ConfBridgeClose(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int verbose)
{
	tOCT6100_CONF_BRIDGE_CLOSE	ConfBridgeClose;
	UINT32				ulResult;

	PRINT1(verbose,
	"%s: Closing Conference Bridge...\n", ec->name);
	
	Oct6100ConfBridgeCloseDef( &ConfBridgeClose );
	ConfBridgeClose.ulConfBridgeHndl = confbridge->ulHndl;
	ulResult = Oct6100ConfBridgeClose(
				ec->pChipInstance,
				&ConfBridgeClose);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to close conference bridge (%X, err=%08X)\n",
					ec->name,
					confbridge->ulHndl,
					ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_ConfBridgeChanAdd(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int verbose)
{
	tOCT6100_CONF_BRIDGE_CHAN_ADD	ConfBridgeChanAdd;
	UINT32				ulResult;

	PRINT1(verbose,
	"%s: Add channel %d to Conference Bridge %X...\n",
					ec->name, channel, confbridge->ulHndl);
	
	Oct6100ConfBridgeChanAddDef( &ConfBridgeChanAdd );
	ConfBridgeChanAdd.ulConfBridgeHndl	= confbridge->ulHndl;
	ConfBridgeChanAdd.ulChannelHndl		= ec->pEchoChannelHndl[channel];
	ulResult = Oct6100ConfBridgeChanAdd(
				ec->pChipInstance,
				&ConfBridgeChanAdd);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to add channel %d to conference bridge (%X, err=%08X)\n",
					ec->name, channel,
					confbridge->ulHndl,
					ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_ConfBridgeChanRemove(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int verbose)
{
	tOCT6100_CONF_BRIDGE_CHAN_REMOVE	ConfBridgeChanRemove;
	UINT32					ulResult;

	PRINT1(verbose,
	"%s: Remove channel %d from Conference Bridge %X...\n",
					ec->name, channel, confbridge->ulHndl);
	
	Oct6100ConfBridgeChanRemoveDef( &ConfBridgeChanRemove );
	ConfBridgeChanRemove.ulConfBridgeHndl	= confbridge->ulHndl;
	ConfBridgeChanRemove.ulChannelHndl	= ec->pEchoChannelHndl[channel];
	ulResult = Oct6100ConfBridgeChanRemove(
				ec->pChipInstance,
				&ConfBridgeChanRemove);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to remove channel %d from conference bridge (%X, err=%08X)\n",
					ec->name, channel,
					confbridge->ulHndl,
					ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_ConfBridgeChanMute(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int verbose)
{
	tOCT6100_CONF_BRIDGE_CHAN_MUTE	ConfBridgeChanMute;
	UINT32				ulResult;

	PRINT1(verbose,
	"%s: Mute channel %d on a conference bridge %X...\n", 
				ec->name, channel, confbridge->ulHndl);
	
	Oct6100ConfBridgeChanMuteDef( &ConfBridgeChanMute );
	ConfBridgeChanMute.ulChannelHndl = ec->pEchoChannelHndl[channel];
	ulResult = Oct6100ConfBridgeChanMute(
				ec->pChipInstance,
				&ConfBridgeChanMute);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to mute channel %d on a conference bridge (%X, err=%08X)\n",
					ec->name, channel,
					confbridge->ulHndl,
					ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_ConfBridgeChanUnMute(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int verbose)
{
	tOCT6100_CONF_BRIDGE_CHAN_UNMUTE	ConfBridgeChanUnMute;
	UINT32					ulResult;

	PRINT1(verbose,
	"%s: UnMute channel %d on a Conference Bridge %X...\n", 
					ec->name, channel, confbridge->ulHndl);
	
	Oct6100ConfBridgeChanUnMuteDef( &ConfBridgeChanUnMute );
	ConfBridgeChanUnMute.ulChannelHndl = ec->pEchoChannelHndl[channel];
	ulResult = Oct6100ConfBridgeChanUnMute(
				ec->pChipInstance,
				&ConfBridgeChanUnMute);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to unmute channel %d from conference bridge (%X, err=%08X)\n",
					ec->name, channel,
					confbridge->ulHndl,
					ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_ConfBridgeDominantSpeakerSet(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, int enable, int verbose)
{
	tOCT6100_CONF_BRIDGE_DOMINANT_SPEAKER_SET	ConfBridgeDominantSpeaker;
	UINT32						ulResult;

	PRINT1(verbose,
	"%s: %s Dominant speaker (channel %d) to a Conference Bridge %X...\n", 
					ec->name,
					(enable) ? "Enable":"Disable", 
					channel, 
					confbridge->ulHndl);
	
	Oct6100ConfBridgeDominantSpeakerSetDef( &ConfBridgeDominantSpeaker );
	ConfBridgeDominantSpeaker.ulConfBridgeHndl	= confbridge->ulHndl;
	if (enable){
		ConfBridgeDominantSpeaker.ulChannelHndl = ec->pEchoChannelHndl[channel];
	}else{
		ConfBridgeDominantSpeaker.ulChannelHndl = cOCT6100_CONF_NO_DOMINANT_SPEAKER_HNDL;
	}
	ulResult = Oct6100ConfBridgeDominantSpeakerSet(
					ec->pChipInstance,
					&ConfBridgeDominantSpeaker);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to %s dominant speaker to conference bridge (%X, err=%08X)\n",
					ec->name, 
					(enable) ? "enable" : "disable",
					confbridge->ulHndl,
					ulResult);
		return -EINVAL;
	}
	return 0;
}


int wanec_ConfBridgeMaskChange(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int channel, UINT32 mask, int verbose)
{
	tOCT6100_CONF_BRIDGE_MASK_CHANGE	ConfBridgeMaskChange;
	UINT32					ulResult;

	PRINT1(verbose,
	"%s: Changing the listener (channel=%d) mask of bridge participant (%d)...\n", 
					ec->name,
					channel, confbridge->ulHndl);
	
	Oct6100ConfBridgeMaskChangeDef( &ConfBridgeMaskChange );
	ConfBridgeMaskChange.ulChannelHndl	= ec->pEchoChannelHndl[channel];
	ConfBridgeMaskChange.ulNewListenerMask	= mask;
	ulResult = Oct6100ConfBridgeMaskChange(
					ec->pChipInstance,
					&ConfBridgeMaskChange);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to change the listener mask of bridge participant %d (err=%X)!\n",
					ec->name, 
					channel,
					ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_ConfBridgeGetStats(wan_ec_t *ec, wan_ec_confbridge_t *confbridge, int verbose)
{
	tOCT6100_CONF_BRIDGE_STATS	ConfBridgeStats;
	UINT32				ulResult;

	PRINT1(verbose,
	"%s: Getting bridge statistics %X...\n", 
					ec->name, confbridge->ulHndl);
	
	Oct6100ConfBridgeGetStatsDef( &ConfBridgeStats );
	ConfBridgeStats.ulConfBridgeHndl	= confbridge->ulHndl;
	ulResult = Oct6100ConfBridgeGetStats(
					ec->pChipInstance,
					&ConfBridgeStats);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to get conference bridge statistics (err=%X)!\n",
					ec->name, 
					ulResult);
		return -EINVAL;
	}
	return 0;
}

/*===========================================================================*\
  Oct6100Read
\*===========================================================================*/

static int
wan_ec_read(wan_ec_dev_t *ec_dev, u32 read_addr, u16 *read_data)
{
	u32	data;
	wan_ec_read_internal_dword(ec_dev, read_addr, &data);
	*read_data = (u16)(data & 0xFFFF);
	return 0;
}

/*===========================================================================*\
  Oct6100Write
\*===========================================================================*/

static int
wan_ec_write(wan_ec_dev_t *ec_dev, u32 write_addr, u32 write_data)
{
	return wan_ec_write_internal_dword(ec_dev, write_addr, write_data);
}


/*===========================================================================*\
  Oct6100WriteSequenceSub
\*===========================================================================*/
static u32
wan_ec_write_seq(wan_ec_dev_t *ec_dev, u32 write_addr, u16 write_data)
{
	u32	ulResult;
	u32	ulData;
	u16	usData;
	u32	ulWordAddress;
	u32	i;

	/* Create the word address from the provided byte address. */
	ulWordAddress = write_addr >> 1;

	/* 16-bit indirect access. */

	/* First write to the address indirection registers. */
	ulData = ( ulWordAddress >> 19 ) & 0x1FFF;
	ulResult = wan_ec_write( ec_dev, 0x8, ulData );
	if (ulResult) 
		return ulResult;

	ulData = ( ulWordAddress >> 3 ) & 0xFFFF;
	ulResult = wan_ec_write( ec_dev, 0xA, ulData );
	if (ulResult) 
		return ulResult;

	/* Next, write data word to read/write data registers. */
	ulData = write_data & 0xFFFF;
	ulResult = wan_ec_write( ec_dev, 0x4, ulData );
	if ( ulResult ) 
		return ulResult;


	/* Write the parities and write enables, as well as last three bits
	** of wadd and request the write access. */
	ulData = ( ( 0x0 & 0x3 ) << 14 ) | ( ( 0x3 & 0x3 ) << 12 ) | ( ( ulWordAddress & 0x7 ) << 9 ) | 0x0100;
	ulResult = wan_ec_write( ec_dev, 0x0, ulData );
	if ( ulResult ) 
		return ulResult;

	/* Keep polling register contol0 for the access_req bit to go low. */
	for ( i = 0; i < WANEC_READ_LIMIT; i++ )
	{
		ulResult = wan_ec_read( ec_dev, 0, &usData );
		if ( ulResult ) 
			return ulResult;

		if ( ( ( usData >> 8 ) & 0x1 ) == 0x0 ) 
			break;
	}

	if ( i == WANEC_READ_LIMIT ){
		DEBUG_EVENT("%s: EC write command reached limit!\n",
				ec_dev->name);
		return WAN_EC_RC_CPU_INTERFACE_NO_RESPONSE;
	}
	return 0;
}


/*===========================================================================*\
  HandleReqWriteOct6100
\*===========================================================================*/
u32 wanec_req_write(void *arg, u32 write_addr, u16 write_data)
{
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;
	u32		ulResult;

	DEBUG_TEST("%s: EC WRITE API addr=%X data=%X\n",
				ec_dev->ec->name, write_addr, write_data);
	ulResult = wan_ec_write_seq(ec_dev, write_addr, write_data);
	if (ulResult){
		DEBUG_EVENT("%s: Failed to write %04X to addr %08X\n",
						ec_dev->name,
						write_addr,
						write_data);
	}
	return ulResult;
}


/*===========================================================================*\
  HandleReqWriteSmearOct6100
\*===========================================================================*/
u32 wanec_req_write_smear(void *arg, u32 addr, u16 data, u32 len)
{
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;
	u32		i, ulResult = WAN_EC_RC_OK;

	WAN_ASSERT(ec_dev == NULL);
	for ( i = 0; i < len; i++ ){
		ulResult = wan_ec_write_seq(ec_dev, addr + (i*2), data);
		if (ulResult){
			DEBUG_EVENT("%s: Failed to write %04X to addr %08X\n",
						ec_dev->name,
						addr + (i*2),
						data);
			break;
		}
	}
	return ulResult;
}


/*===========================================================================*\
  HandleReqWriteBurstOct6100
\*===========================================================================*/
u32 wanec_req_write_burst(void *arg, u32 addr, u16 *data, u32 len)
{
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;
	u32		i, ulResult = WAN_EC_RC_OK;

	WAN_ASSERT(ec_dev == NULL);

	for ( i = 0; i < len; i++ ){
		ulResult = wan_ec_write_seq(ec_dev, addr + (i * 2), data[i]);
		if (ulResult){
			DEBUG_EVENT("%s: Failed to write %04X to addr %08X\n",
						ec_dev->name,
						addr + (i*2),
						data[i]);
			break;
		}
	}
	return ulResult;
}


/*===========================================================================*\
  Oct6100ReadSequenceSub
\*===========================================================================*/
static u32
wan_ec_read_seq(wan_ec_dev_t *ec_dev, u32 read_addr, u16 *read_data, u32 read_len)
{
	u32	ulResult;
	u32	ulData;
	u32	ulWordAddress;
	u32	ulReadBurstLength;
	u16	usData;
	u32	i;

	/* Create the word address from the provided byte address. */
	ulWordAddress = read_addr >> 1;

	/* Indirect accesses. */

	/* First write to the address indirection registers. */
	ulData = ( ulWordAddress >> 19 ) & 0x1FFF;
	ulResult = wan_ec_write( ec_dev, 0x8, ulData );
	if (ulResult) 
		return ulResult;

	ulData = ( ulWordAddress >> 3 ) & 0xFFFF;
	ulResult = wan_ec_write( ec_dev, 0xA, ulData );
	if (ulResult)
		return ulResult;

	/* Request access. */
	if ( read_len >= 128 )
	{
		ulData = 0x100 | ( ( ulWordAddress & 0x7 ) << 9);
		ulReadBurstLength = 0;
	}
	else
	{
		ulData = 0x100 | ( ( ulWordAddress & 0x7 ) << 9) | read_len;
		ulReadBurstLength = read_len;
	}
	ulResult = wan_ec_write( ec_dev, 0x0, ulData );
	if (ulResult)
		return ulResult;

	/* Keep polling register contol0 for the access_req bit to go low. */
	for ( i = 0; i < WANEC_READ_LIMIT; i++ )
	{
		ulResult = wan_ec_read( ec_dev, 0x0, &usData );
		if (ulResult) 
			return ulResult;

		if ( ( ( usData >> 8 ) & 0x1 ) == 0x0 )
			break;
	}
	if ( i == WANEC_READ_LIMIT ){
		DEBUG_EVENT("%s: EC read command reached limit!\n",
				ec_dev->name);
		return WAN_EC_RC_CPU_INTERFACE_NO_RESPONSE;
	}

	if ( ( usData & 0xFF ) == 0x1 )
	{
		i = 0;
	}

	/* Retrieve read data. */
	ulResult = wan_ec_read( ec_dev, 0x4, &usData );
	if (ulResult)
		return ulResult;

	if ( ( usData & 0xFF ) == 0x1 )
	{
		i = 0;
	}

	*read_data = usData;
	return 0;
}

u32 wanec_req_read(void *arg, u32 addr, u16 *data)
{
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;
	u32		ulResult;

	WAN_ASSERT(ec_dev == NULL);
	DEBUG_TEST("%s: EC READ API addr=%X data=????\n",
			ec_dev->ec->name, addr);
	ulResult = wan_ec_read_seq(
				ec_dev,
				addr,
				data,
				1);
	if (ulResult){
		DEBUG_EVENT("%s: Failed to read data from addr %08X\n",
					ec_dev->name,
					addr);
		if (ec_dev->ec){
			wan_set_bit(WAN_EC_BIT_CRIT_ERROR, &ec_dev->ec->critical);
		}
	}
	DEBUG_TEST("%s: EC READ API addr=%X data=%X\n",
			ec_dev->ec->name,
			addr, *data);
	return ulResult;
}

u32 wanec_req_read_burst(void *arg, u32 addr, u16 *data, u32 len)
{
	wan_ec_dev_t	*ec_dev = (wan_ec_dev_t*)arg;
	u32		i, ulResult = WAN_EC_RC_OK;
	u16		read_data;

	for ( i = 0; i < len; i++ ){
		ulResult = wan_ec_read_seq(ec_dev, addr, &read_data, 1);
		if (ulResult){
			DEBUG_EVENT("%s: Failed to read from addr %X\n",
						ec_dev->name,
						addr);
			if (ec_dev->ec){
				wan_set_bit(WAN_EC_BIT_CRIT_ERROR, &ec_dev->ec->critical);
			}
			break;
		}
		data[i]  = (u16)read_data;
		addr += 2;
	}
	return ulResult;
}
