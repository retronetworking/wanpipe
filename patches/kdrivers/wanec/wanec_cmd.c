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
#define MAX_EC_PORT_RANGE	32
#define WAN_OCT6100_READ_LIMIT	0x10000

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

int wanec_ChipOpenPrep(wan_ec_dev_t*, wan_ec_api_t*);
int wanec_ChipOpen(wan_ec_dev_t*, int);
int wanec_ChipOpen_OLD(wan_ec_dev_t*, wan_ec_api_t*);
int wanec_ChipClose(wan_ec_dev_t*, int verbose);
int wanec_ChipStats(wan_ec_dev_t*, wan_ec_api_t*, int reset);

int wanec_ChannelOpen(wan_ec_dev_t*, wan_ec_api_t*);
int wanec_ChannelClose(wan_ec_dev_t*, wan_ec_api_t*, int);
int wanec_ChannelModify(wan_ec_dev_t*, INT, UINT32, wan_ec_api_t*, int verbose);
int wanec_ChannelStats(wan_ec_dev_t*, INT channel, wan_ec_api_t*, int);

int wanec_TonesEnable(wan_ec_t *ec, int channel, unsigned char port, int verbose);
int wanec_TonesDisable(wan_ec_t *ec, int channel, unsigned char port, int verbose);

int wanec_DebugChannel(wan_ec_t *ec, INT channel, int verbose);
int wanec_DebugGetData(wan_ec_t *ec, wan_ec_api_t *ec_api);

int wanec_BufferLoad(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
int wanec_BufferUnload(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
int wanec_BufferPlayoutAdd(wan_ec_t *ec, int channel, wan_ec_api_t *ec_api);
int wanec_BufferPlayoutStart(wan_ec_t *ec, int channel, wan_ec_api_t *ec_api);
int wanec_BufferPlayoutStop(wan_ec_t *ec, int channel, wan_ec_api_t *ec_api);

int wanec_ConfBridgeOpen(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);
int wanec_ConfBridgeClose(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api);

int wanec_EventTone(wan_ec_t *ec, int verbose);
int wanec_ISR(wan_ec_t *ec, int verbose);

int wanec_fe2ec_channel(wan_ec_dev_t*, int);
static int wanec_hndl2ec_channel(wan_ec_t *ec, UINT32 ChannelHndl);
static int wanec_ec2fe_channel(wan_ec_t*, int, wan_ec_dev_t**);

extern int wanec_ChipParam(wan_ec_t*,tPOCT6100_CHIP_OPEN, wan_ec_api_t*);
extern int wanec_ChanParam(wan_ec_t*,tPOCT6100_CHANNEL_MODIFY, wan_ec_api_t*);
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
	/*ec_channel = ec_dev->fe_lineno * ec_dev->fe_max_channels + channel;*/
	return ec_dev->fe_lineno * MAX_EC_PORT_RANGE + fe_channel;
}

static int wanec_ec2fe_channel(wan_ec_t *ec, int ec_chan, wan_ec_dev_t **ec_dev)
{
	int	fe_chan;

	*ec_dev = ec->pEcDevMap[ec_chan];
	if (*ec_dev == NULL) return 0;

	fe_chan = ec_chan % MAX_EC_PORT_RANGE;

	if ((*ec_dev)->fe_media == WAN_MEDIA_T1 ||
	    (*ec_dev)->fe_media == WAN_MEDIA_FXOFXS){
		fe_chan++; 
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
int wanec_ChipStats(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api, int reset)
{
	wan_ec_t		*ec;
	tOCT6100_CHIP_STATS	f_ChipStats;
	UINT32			ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	
	if (ec_api){
		PRINT1(ec_api->verbose,
		"%s: Reading chip statistics...\n",
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
	if (ec_api){
		memcpy(	&ec_api->u_chip_stats.f_ChipStats,
			&f_ChipStats,
			sizeof(tOCT6100_CHIP_STATS));
	}

	if (ec_api){
		PRINT1(ec_api->verbose,
		"%s: Reading chip image info...\n",
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
	if (ec_api){
		if (ec_api->u_chip_stats.f_ChipImageInfo){
			int err;
			err = WAN_COPY_TO_USER(
					&f_ChipImageInfo,
					ec_api->u_chip_stats.f_ChipImageInfo,
					sizeof(tOCT6100_CHIP_IMAGE_INFO));
			if (err){
				DEBUG_EVENT(
				"%s: Failed to copy chip image info to user space [%s:%d]!\n",
					ec_api->devname,
					__FUNCTION__,__LINE__);
				return -EINVAL;
			}
		}else{
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller image description:\n%s\n",
					ec->name, f_ChipImageInfo.szVersionNumber);
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller image build ID\t\t\t%08X\n",
					ec->name, f_ChipImageInfo.ulBuildId);
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller maximum number of channels\t%d\n",
					ec->name, f_ChipImageInfo.ulMaxChannels);	
#if 0	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller maximum tail displacement\t\t%d\n",
					ec->name, f_ChipImageInfo.ulMaxTailDisplacement);	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller per channel tail displacement\t%s\n",
					ec->name,
					(f_ChipImageInfo.fPerChannelTailDisplacement == TRUE)?"TRUE":"FALSE");
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller per channel tail length\t\t%s\n",
					ec->name,
					(f_ChipImageInfo.fPerChannelTailLength == TRUE)?"TRUE":"FALSE");
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller maximum tail length\t\t%d\n",
					ec->name, f_ChipImageInfo.ulMaxTailLength);	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller buffer Playout support\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fBufferPlayout == TRUE)?"TRUE":"FALSE");	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller adaptive noise reduction\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fAdaptiveNoiseReduction==TRUE)?"TRUE":"FALSE");	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller SOUT noise bleaching\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fSoutNoiseBleaching==TRUE)?"TRUE":"FALSE");	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller ROUT noise reduction\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fRoutNoiseReduction==TRUE)?"TRUE":"FALSE");	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller ROUT noise reduction level\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fRoutNoiseReductionLevel==TRUE)?"TRUE":"FALSE");	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller automatic level control\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fAutoLevelControl==TRUE)?"TRUE":"FALSE");	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller acoustic echo cancellation\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fAcousticEcho==TRUE)?"TRUE":"FALSE");	
			PRINT1(ec_api->verbose,
			"%s: Echo Canceller conferencing\t\t%s\n",
					ec->name, 
					(f_ChipImageInfo.fConferencing==TRUE)?"TRUE":"FALSE");	
			PRINT1(ec_api->verbose,
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

int wanec_ChipOpenPrep(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	tOCT6100_GET_INSTANCE_SIZE	InstanceSize;
	wan_ec_t			*ec;
	UINT32				ulResult;
	INT				err;
	
	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_api->u_config.imageData == NULL);
	ec	= ec_dev->ec;

	ec->pImageData = wan_vmalloc(ec_api->u_config.imageSize * sizeof(UINT8));
	if (ec->pImageData == NULL){
		DEBUG_EVENT(
		"ERROR: Failed to allocate memory for EC image %ld bytes [%s:%d]!\n",
				(unsigned long)ec_api->u_config.imageSize*sizeof(UINT8),
				__FUNCTION__,__LINE__);
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
		return -EINVAL;
	}
		
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
	ec->f_OpenChip.ulUserChipId = 1;

	/* Set the max number of accesses to 1024 to speed things up */
	ec->f_OpenChip.ulMaxRwAccesses = 1024;

	/* Set the maximums that the chip needs to support for this test */
	ec->f_OpenChip.ulMaxChannels	= ec_api->u_config.max_channels;

	ec->f_OpenChip.ulMaxBiDirChannels	= 0;
	ec->f_OpenChip.ulMaxConfBridges	= 0;
	ec->f_OpenChip.ulMaxPhasingTssts	= 0;
	ec->f_OpenChip.ulMaxTdmStreams	= 32;
	ec->f_OpenChip.ulMaxTsiCncts	= 2;

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
	ec->f_OpenChip.ulMemoryChipSize	= ec_api->u_config.memory_chip_size;

	ec->f_OpenChip.fEnableChannelRecording	= TRUE;

#if defined(ENABLE_ACOUSTICECHO)
	ec->f_OpenChip.fEnableAcousticEcho		= TRUE;
#endif

#if defined(ENABLE_PRODBIST)
	/* Enable production bist mode */
	ec->f_OpenChip.fEnableProductionBist	= TRUE;
	ec->f_OpenChip.ulNumProductionBistLoops	= 0x1;
#endif
	
	ec->f_OpenChip.pbyImageFile = ec->pImageData;
	ec->f_OpenChip.ulImageSize  = ec_api->u_config.imageSize;

	ec->f_OpenChip.ulMaxPlayoutBuffers	= cOCT6100_MAX_PLAYOUT_BUFFERS;
		
	/* Assign board index (0). */
 	ec->f_Context.ulBoardId = ec->chip_no;
	
	/* Handle to driver */
	ec->f_Context.ec_dev = ec_dev;

	/* Interface name to driver */
	strlcpy(ec->f_Context.devname, ec_api->devname, WAN_DRVNAME_SZ);

	ulResult = Oct6100GetInstanceSize(&ec->f_OpenChip, &InstanceSize);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to get EC chip instance size (err=0x%X)!\n",
					ec->name, ulResult);
		wan_vfree(ec->pImageData);
		return -EINVAL;
	}

	/* Allocate memory needed for API instance. */
	ec->pChipInstance = 
		(tPOCT6100_INSTANCE_API)wan_vmalloc(InstanceSize.ulApiInstanceSize);
	if (ec->pChipInstance == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed to allocate memory for EC chip (%d bytes)!\n",
					ec->name,InstanceSize.ulApiInstanceSize);
		wan_vfree(ec->pImageData);
		return -EINVAL;
	}

	/* Open the OCT6100 on the evaluation board. */
	ec->f_OpenChip.pProcessContext = (PVOID)&ec->f_Context;

	/* parse advianced params */
	if (ec_api->param_no){
		wanec_ChipParam(ec, &ec->f_OpenChip, ec_api);
	}

	/* Chip Open parameter verification */	
	
	ec->ulDebugChannelHndl	= cOCT6100_INVALID_HANDLE;
	ec->ulDebugDataMode	= ec_api->u_config.debug_data_mode;	
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
		wan_vfree(ec->pImageData);
		return -EINVAL;
	}
	wan_vfree(ec->pImageData);

	if (wanec_ChipStats(ec_dev, NULL, TRUE)){
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

int wanec_ChipOpen_OLD(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	tOCT6100_CHIP_OPEN		f_OpenChip;
	tOCT6100_GET_INSTANCE_SIZE	InstanceSize;
	wan_ec_t			*ec;
	/*PUINT8				pImageData = NULL;*/
	UINT32				ulResult, i = 0;
	INT				ec_chan = 0, err;
	
	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	WAN_ASSERT(ec_api->u_config.imageData == NULL);
	ec	= ec_dev->ec;

	ec->pImageData = wan_vmalloc(ec_api->u_config.imageSize * sizeof(UINT8));
	if (ec->pImageData == NULL){
		DEBUG_EVENT(
		"ERROR: Failed to allocate memory for EC image %ld bytes [%s:%d]!\n",
				(unsigned long)ec_api->u_config.imageSize*sizeof(UINT8),
				__FUNCTION__,__LINE__);
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
		return -EINVAL;
	}
		
	ulResult = Oct6100ChipOpenDef( &f_OpenChip );

	/*==============================================================*/
	/* Configure clocks */

	/* upclk oscillator is at 33.33 Mhz */
	f_OpenChip.ulUpclkFreq = cOCT6100_UPCLK_FREQ_33_33_MHZ;

	/* mclk will be generated by internal PLL at 133 Mhz */
	f_OpenChip.fEnableMemClkOut = TRUE;
#if 1
	f_OpenChip.ulMemClkFreq = cOCT6100_MCLK_FREQ_133_MHZ;
#else
	f_OpenChip.ulMemClkFreq = cOCT6100_MCLK_FREQ_125_MHZ; /*125*/
#endif

	/*==============================================================*/

	/*==============================================================*/
	/* General parameters */

	/* Chip ID.*/	
	f_OpenChip.ulUserChipId = 1;

	/* Set the max number of accesses to 1024 to speed things up */
	f_OpenChip.ulMaxRwAccesses = 1024;

	/* Set the maximums that the chip needs to support for this test */
	f_OpenChip.ulMaxChannels	= ec_api->u_config.max_channels;

#if 1
	f_OpenChip.ulMaxPlayoutBuffers	= WAN_NUM_PLAYOUT_TONES;
#else
	f_OpenChip.ulMaxPlayoutBuffers	= 2;
#endif

	f_OpenChip.ulMaxBiDirChannels	= 0;
	f_OpenChip.ulMaxConfBridges	= 0;
	f_OpenChip.ulMaxPhasingTssts	= 0;
	f_OpenChip.ulMaxTdmStreams	= 32;
	f_OpenChip.ulMaxTsiCncts	= 2;

	/*==============================================================*/

	/*==============================================================*/
	/* External Memory Settings */

	/* Use DDR memory.*/
#if 1
	f_OpenChip.ulMemoryType	= cOCT6100_MEM_TYPE_SDR;
#else
	f_OpenChip.ulMemoryType	= cOCT6100_MEM_TYPE_DDR;
#endif
	
	f_OpenChip.ulNumMemoryChips	= 2;

	/*
	**f_OpenChip.ulMemoryChipSize	= cOCT6100_MEMORY_CHIP_SIZE_8MB;
	**f_OpenChip.ulMemoryChipSize	= cOCT6100_MEMORY_CHIP_SIZE_16MB;*/
	f_OpenChip.ulMemoryChipSize	= ec_api->u_config.memory_chip_size;

	f_OpenChip.fEnableChannelRecording	= TRUE;

#if defined(ENABLE_ACOUSTICECHO)
	f_OpenChip.fEnableAcousticEcho		= TRUE;
#endif

#if defined(ENABLE_PRODBIST)
	/* Enable production bist mode */
	f_OpenChip.fEnableProductionBist	= TRUE;
	f_OpenChip.ulNumProductionBistLoops	= 0x1;
#endif
	
	f_OpenChip.pbyImageFile = ec->pImageData;
	f_OpenChip.ulImageSize  = ec_api->u_config.imageSize;

	/* Assign board index (0). */
 	ec->f_Context.ulBoardId = ec->chip_no;
	
	/* Handle to driver */
	ec->f_Context.ec_dev = ec_dev;

	/* Interface name to driver */
	strlcpy(ec->f_Context.devname, ec_api->devname, WAN_DRVNAME_SZ);

	ulResult = Oct6100GetInstanceSize(&f_OpenChip, &InstanceSize);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to get EC chip instance size (err=0x%X)!\n",
					ec->name, ulResult);
		wan_vfree(ec->pImageData);
		return -EINVAL;
	}

	/* Allocate memory needed for API instance. */
	ec->pChipInstance = 
		(tPOCT6100_INSTANCE_API)wan_vmalloc(InstanceSize.ulApiInstanceSize);
	if (ec->pChipInstance == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed to allocate memory for EC chip (%d bytes)!\n",
					ec->name,InstanceSize.ulApiInstanceSize);
		wan_vfree(ec->pImageData);
		return -EINVAL;
	}

	/* Open the OCT6100 on the evaluation board. */
	f_OpenChip.pProcessContext = (PVOID)&ec->f_Context;

	/* parse advianced params */
	if (ec_api->param_no){
		wanec_ChipParam(ec, &f_OpenChip, ec_api);
	}

	PRINT1(ec_api->verbose,
	"%s: Opening Echo Canceller Chip ...\n",
					ec->name);
	ulResult = Oct6100ChipOpen( 
			ec->pChipInstance,	/* API instance memory. */
			&f_OpenChip );		/* Open chip structure. */
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to open Echo Canceller Chip (err=0x%X)\n",
					ec->name, ulResult);
		wan_vfree(ec->pImageData);
		return -EINVAL;
	}
	wan_vfree(ec->pImageData);

	if (wanec_ChipStats(ec_dev, NULL, TRUE)){
		wanec_ChipClose(ec_dev, ec_api->verbose);
		return EINVAL;
	}

	ec->pToneBufferIndexes = 
			wan_malloc(sizeof(UINT32) * ec->f_OpenChip.ulMaxPlayoutBuffers);
	if (ec->pToneBufferIndexes == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed allocate memory for playout handles!\n",
					ec->name);
		wanec_ChipClose(ec_dev, ec_api->verbose);
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
		wanec_ChipClose(ec_dev, ec_api->verbose);
		return EINVAL;
	}
	ec->pEcDevMap = wan_malloc(sizeof(wan_ec_dev_t*) * ec->max_channels);
	if (ec->pEcDevMap == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed allocate memory for ec channel map!\n",
					ec->name);
		wan_free(ec->pToneBufferIndexes);
		wan_free(ec->pEchoChannelHndl);
		wanec_ChipClose(ec_dev, ec_api->verbose);
		return -EINVAL;
	}
	for(ec_chan = 0; ec_chan < ec->max_channels; ec_chan++){
		ec->pEchoChannelHndl[ec_chan] = cOCT6100_INVALID_HANDLE;
		ec->pEcDevMap[ec_chan] = NULL;
	}

	ec->ulDebugChannelHndl	= cOCT6100_INVALID_HANDLE;
	ec->ulDebugDataMode	= ec_api->u_config.debug_data_mode;

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

int wanec_ChannelOpen(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
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
	PRINT1(ec_api->verbose,
	"%s: Openning all Echo Canceller channels (%s)...\n",
				ec->name,
				(pcm_law_type == cOCT6100_PCM_U_LAW) ?
						"MULAW":"ALAW");

	DEBUG_EVENT("%s: Opening HW Echo Canceller (NoiseRed=%s)\n",
			ec->name,card->hwec_conf.noise_reduction?"On":"Off");
 
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

		if (card->hwec_conf.noise_reduction) {
			EchoChannelOpen.VqeConfig.fSoutAdaptiveNoiseReduction = TRUE;
		} else {
			EchoChannelOpen.VqeConfig.fSoutAdaptiveNoiseReduction = FALSE;
		}

		EchoChannelOpen.VqeConfig.ulComfortNoiseMode	= 
					cOCT6100_COMFORT_NOISE_NORMAL;
				/*	cOCT6100_COMFORT_NOISE_NORMAL
				**	cOCT6100_COMFORT_NOISE_EXTENDED,
				**	cOCT6100_COMFORT_NOISE_OFF,
				**	cOCT6100_COMFORT_NOISE_FAST_LATCH */

		PRINT1(ec_api->verbose,
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

int wanec_ChannelClose(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api, int verbose)
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

int wanec_ChannelModify(	wan_ec_dev_t	*ec_dev,
				INT		channel,
				UINT32		mode,
				wan_ec_api_t	*ec_api,
				int		verbose)
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

	/* Enable echo cancellation */
	EchoChannelModify.ulEchoOperationMode = mode;

	/* parse advianced params */
	if (ec_api && ec_api->param_no){
		int err;
		err = wanec_ChanParam(ec, &EchoChannelModify, ec_api);
		if (err){
			DEBUG_EVENT(
			"%s: WARNING: Unsupported parameter for channel %d!\n",
					ec->name,
					channel);
			return -EINVAL;
		}
	}else if (mode == cOCT6100_KEEP_PREVIOUS_SETTING){
		wanec_ChanParamList(ec);
		return 0;
	}

	if (mode == cOCT6100_KEEP_PREVIOUS_SETTING){
		PRINT1(verbose,
		"%s: Modify Channel configuration for channel %d...\n",
				ec->name,
				channel);
	}else{
		PRINT1(verbose,
		"%s: Modify EC mode for channel %d to %s ...\n",
				ec->name,
				channel,
				(mode == cOCT6100_ECHO_OP_MODE_POWER_DOWN) ?
							"POWER_DOWN" :
				(mode == cOCT6100_ECHO_OP_MODE_NORMAL) ?
							"NORMAL" : "UNKNOWN");
	}

	/* Open the channel.*/
	ulResult = Oct6100ChannelModify( 
					ec->pChipInstance,
					&EchoChannelModify );
	if (ulResult != cOCT6100_ERR_OK){
		if (mode == cOCT6100_KEEP_PREVIOUS_SETTING){
			PRINT1(verbose,
			"%s: Failed to modify Channel config parameters for channel %d (err=0x%X)\n",
					ec->name,
					channel,
					ulResult);
		}else{
			DEBUG_EVENT(
			"ERROR: %s: Failed to modify EC mode %s for channel %d (err=0x%X)\n",
					ec->name,
					(mode == cOCT6100_ECHO_OP_MODE_POWER_DOWN) ?
								"POWER_DOWN" :
					(mode == cOCT6100_ECHO_OP_MODE_NORMAL) ?
								"NORMAL" : "UNKNOWN",
					channel,
					ulResult);
		}
		return EINVAL;
	}
	return	0;
}

int wanec_ChannelStats(wan_ec_dev_t *ec_dev, INT channel, wan_ec_api_t *ec_api, int reset)
{
	wan_ec_t		*ec;
	tOCT6100_CHANNEL_STATS	f_ChannelStats;
	UINT32			ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;

	PRINT1(ec_api->verbose, "%s: Reading EC statistics for channel %d...\n",
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
	if (ec_api){
		memcpy(	&ec_api->u_chan_stats.f_ChannelStats,
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

	PRINT1(verbose, "%s: Enable tone detection on channel %d ...\n",
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
		return EINVAL;
	}

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
			unsigned char	dtmf_port=0, dtmf_type=0;
			
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

	return 0;
}

int wanec_ISR(wan_ec_t *ec, int verbose)
{
	UINT32	ulResult;

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
		PRINT1(verbose,
		"%s: Error Overflow Tone Events\n",
				ec->name);
	}
	if (ec->f_InterruptFlag.fToneEventsPending == TRUE){
		PRINT1(verbose, "%s: Tone Event pending....\n",
				ec->name);
		wanec_EventTone(ec, verbose);
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
	return 0;
}

int wanec_DebugChannel(wan_ec_t *ec, INT channel, int verbose)
{
	tOCT6100_DEBUG_SELECT_CHANNEL	DebugSelectChannel;
	UINT32				ulResult;

	if (ec->ulDebugChannelHndl != cOCT6100_INVALID_HANDLE){
		DEBUG_EVENT(
		"ERROR: %s: Echo Canceller daemon can monitor only one channel (%d)!\n",
				ec->name,
				ec->DebugChannel);
		return -EINVAL;	
	}
	Oct6100DebugSelectChannelDef( &DebugSelectChannel );

	PRINT1(verbose, "%s: Select channel %d for monitoring...\n",
			ec->name,
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
		"ERROR: %s: Failed to select debug channel %d for monitoring (err=0x%X)\n",
				ec->name,
				channel,
				ulResult);
		return -EINVAL;
	}
	return	0;
}

int wanec_DebugGetData(wan_ec_t *ec, wan_ec_api_t *ec_api)
{
	tOCT6100_DEBUG_GET_DATA	fDebugGetData;
	UINT32			ulResult;

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

	PRINT1(ec_api->verbose,
	"%s: Retrieves debug data for channel %d...\n",
			ec->name,
			ec->DebugChannel);

	memset(&ec_api->u_chan_monitor.data[0], 0,
				sizeof(UINT8) * (MAX_MONITOR_DATA_LEN+1));
	/* Set selected debug channel */
	fDebugGetData.ulGetDataMode	= ec->ulDebugDataMode;
	fDebugGetData.ulMaxBytes	= ec_api->u_chan_monitor.max_len;
	fDebugGetData.pbyData		= &ec_api->u_chan_monitor.data[0];

	/* Select Debug channel */
	ulResult = Oct6100DebugGetData( 
			ec->pChipInstance,
			&fDebugGetData );
	if (ulResult != cOCT6100_ERR_OK){
		PRINT(
#if !defined(__WINDOWS__)
			verbose,
#endif
		"ERROR: %s: Failed to get debug data for channel %d (err=0x%X)\n",
				ec->name,
				ec->DebugChannel,
				ulResult);
		return -EINVAL;
	}
	ec_api->u_chan_monitor.data_len		= fDebugGetData.ulValidNumBytes;
	ec_api->u_chan_monitor.remain_len	= fDebugGetData.ulRemainingNumBytes;
	ec_api->channel				= ec->DebugChannel;
	
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

int wanec_BufferLoad(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t		*ec;
	tOCT6100_BUFFER_LOAD	BufferLoad;
	UINT32			size, ulResult;
	PUINT8			pData = NULL;
	int			err;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	PRINT1(ec_api->verbose,
	"%s: Loading Tone buffer (%s) into OCT6100 Chip ...\n",
					ec->name, ec_api->u_tone_config.tone);
	size = ec_api->u_tone_config.size * sizeof(INT8);
	pData = wan_vmalloc(size);
	if (pData == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Failed to allocate memory for tone buffer!\n",
					ec->name);
		return -EINVAL;
	}
	err = WAN_COPY_FROM_USER(
				pData,
				ec_api->u_tone_config.data,
				size);
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
	ec_api->u_tone_config.buffer_index = *BufferLoad.pulBufferIndex;
	PRINT1(ec_api->verbose,
	"%s: Current tone index is %d\n",
			ec->name, ec_api->u_tone_config.buffer_index);
	return 0;
}

int wanec_BufferUnload(wan_ec_dev_t *ec_dev, wan_ec_api_t *ec_api)
{
	wan_ec_t		*ec;
	tOCT6100_BUFFER_UNLOAD	BufferUnload;
	PUINT32			pBufferIndex = NULL;
	UINT32			index = 0, ulResult;

	WAN_ASSERT(ec_dev == NULL);
	WAN_ASSERT(ec_dev->ec == NULL);
	ec = ec_dev->ec;
	PRINT1(ec_api->verbose,
	"%s: Unloading Tone buffer from EC chip ...\n",
					ec->name);

try_next_index:					
	Oct6100BufferPlayoutUnloadDef( &BufferUnload );
	if (ec_api->u_tone_config.buffer_index != cOCT6100_INVALID_VALUE){
		pBufferIndex = wanec_search_bufferindex(ec, ec_api->u_tone_config.buffer_index);
		if (pBufferIndex == NULL){
			DEBUG_EVENT(
			"ERROR: %s: Invalid tone buffer index %X!\n",
					ec->name, ec_api->u_tone_config.buffer_index);
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
	if (!ec_api->u_tone_config.buffer_index){
		index++;
		goto try_next_index;	
	}

buffer_unload_done:	
	return 0;
}

int wanec_BufferPlayoutAdd(wan_ec_t *ec, int channel, wan_ec_api_t *ec_api)
{
	tOCT6100_BUFFER_PLAYOUT_ADD	BufferPlayoutAdd;
	UINT32				ulResult;

	PRINT1(ec_api->verbose,
	"%s: Add Tone buffer to channel %d...\n",
					ec->name, channel);
	if (ec_api->u_playout.index == cOCT6100_INVALID_VALUE||
	    wanec_search_bufferindex(ec, ec_api->u_playout.index) == NULL){
		DEBUG_EVENT(
		"ERROR: %s: Invalid playout buffer index for channel %d!\n",
					ec->name, channel);
		return -EINVAL;
	}
	Oct6100BufferPlayoutAddDef( &BufferPlayoutAdd );
	BufferPlayoutAdd.fRepeat	= ec_api->u_playout.repeat;
	BufferPlayoutAdd.ulPlayoutPort	= cOCT6100_CHANNEL_PORT_ROUT;
	BufferPlayoutAdd.ulMixingMode	= cOCT6100_MIXING_MUTE;
	BufferPlayoutAdd.ulChannelHndl	= ec->pEchoChannelHndl[channel];
	BufferPlayoutAdd.ulBufferIndex	= ec_api->u_playout.index;
	BufferPlayoutAdd.ulDuration	= (ec_api->u_playout.duration) ?
						ec_api->u_playout.duration : 5000;
	BufferPlayoutAdd.ulBufferLength	= (ec_api->u_playout.buffer_length) ?
						ec_api->u_playout.buffer_length : 
						cOCT6100_AUTO_SELECT;
	ulResult = Oct6100BufferPlayoutAdd(
					ec->pChipInstance,
					&BufferPlayoutAdd);
	if ( ulResult != cOCT6100_ERR_OK ){
		DEBUG_EVENT(
		"ERROR: %s: Failed to add playout buffer to channel %d (err=%08X)\n",
					ec->name, channel, ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_BufferPlayoutStart(wan_ec_t *ec, int channel, wan_ec_api_t *ec_api)
{
	tOCT6100_BUFFER_PLAYOUT_START	BufferPlayoutStart;
	UINT32				ulResult;

	PRINT1(ec_api->verbose,
	"%s: Active playout buffer on channel %d...\n",
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
		"ERROR: %s: Failed to active playout buffer on channel %d (err=%08X)\n",
					ec->name,
					channel,
					ulResult);
		return -EINVAL;
	}
	return 0;
}

int wanec_BufferPlayoutStop(wan_ec_t *ec, int channel, wan_ec_api_t *ec_api)
{
	tOCT6100_BUFFER_PLAYOUT_STOP	BufferPlayoutStop;
	UINT32				ulResult;

	PRINT1(ec_api->verbose,
	"%s: Deactive playout buffer on channel %d...\n",
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
		"ERROR: %s: Failed to deactive playout buffer on channel %d (err=%08X)\n",
					ec->name,
					channel,
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
	for ( i = 0; i < WAN_OCT6100_READ_LIMIT; i++ )
	{
		ulResult = wan_ec_read( ec_dev, 0, &usData );
		if ( ulResult ) 
			return ulResult;

		if ( ( ( usData >> 8 ) & 0x1 ) == 0x0 ) 
			break;
	}

	if ( i == WAN_OCT6100_READ_LIMIT ){
		DEBUG_EVENT("%s: EC write command reached limit!\n",
				ec_dev->name);
		return WAN_OCT6100_RC_CPU_INTERFACE_NO_RESPONSE;
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
	u32		i, ulResult = WAN_OCT6100_RC_OK;

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
	u32		i, ulResult = WAN_OCT6100_RC_OK;

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
	for ( i = 0; i < WAN_OCT6100_READ_LIMIT; i++ )
	{
		ulResult = wan_ec_read( ec_dev, 0x0, &usData );
		if (ulResult) 
			return ulResult;

		if ( ( ( usData >> 8 ) & 0x1 ) == 0x0 )
			break;
	}
	if ( i == WAN_OCT6100_READ_LIMIT ){
		DEBUG_EVENT("%s: EC read command reached limit!\n",
				ec_dev->name);
		return WAN_OCT6100_RC_CPU_INTERFACE_NO_RESPONSE;
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
	u32		i, ulResult = WAN_OCT6100_RC_OK;
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
