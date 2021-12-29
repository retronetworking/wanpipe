/******************************************************************************
** Copyright (c) 2005
**	Alex Feldman <al.feldman@sangoma.com>.  All rights reserved.
**
** ============================================================================
** Oct 13, 2005		Alex Feldman	Initial version.
**
** Jul 26, 2006		David Rokhvarg	<davidr@sangoma.com>	Ported to Windows.
******************************************************************************/

/******************************************************************************
**			   INCLUDE FILES
******************************************************************************/
#if !defined(__WINDOWS__)
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <syslog.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/un.h>
#include <netinet/in.h>
#if defined(__LINUX__)
# include <linux/if.h>
# include <linux/types.h>
# include <linux/if_packet.h>
#endif
#endif

#if defined(__LINUX__)
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_cfg.h>
# include <wanec_iface.h>
#elif defined(__WINDOWS__)
# include <windows.h>
# include <sang_status_defines.h>
# include <wanpipe_defines.h>
# include <wanpipe_cfg.h>
# include "wan_ecmain.h"
# include <wanec_iface.h>
#else
# include <wanpipe_defines.h>
# include <wanpipe_cfg.h>
# include <wanec_iface.h>
#endif

#include "wanec_api.h"

/******************************************************************************
**			  DEFINES AND MACROS
******************************************************************************/

/******************************************************************************
**			STRUCTURES AND TYPEDEFS
******************************************************************************/

/******************************************************************************
**			   GLOBAL VARIABLES
******************************************************************************/
wan_ec_api_t	ec_api;

/******************************************************************************
** 			FUNCTION PROTOTYPES
******************************************************************************/
extern int wanec_api_lib_config(wan_ec_api_t *ec_api, int verbose);
extern int wanec_api_lib_toneload(wan_ec_api_t *ec_api);
extern int wanec_api_lib_monitor(wan_ec_api_t *ec_api);
extern int wanec_api_lib_cmd(wan_ec_api_t *ec_api);



/******************************************************************************
** 			FUNCTION DEFINITIONS
******************************************************************************/

static int wanec_api_print_chip_stats(wan_ec_api_t *ec_api)
{
	tPOCT6100_CHIP_STATS		pChipStats;
	tPOCT6100_CHIP_IMAGE_INFO	pChipImageInfo;

	pChipStats = &ec_api->u_chip_stats.f_ChipStats;
	printf("****** Echo Canceller Chip Get Stats %s ******\n",
			ec_api->devname);
	printf("%10s: Number of channels currently open\t\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulNumberChannels);
	printf("%10s: Number of conference bridges currently open\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulNumberConfBridges);
	printf("%10s: Number of playout buffers currently loaded\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulNumberPlayoutBuffers);
	printf("%10s: Number of framing error on H.100 bus\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulH100OutOfSynchCount);
	printf("%10s: Number of errors on H.100 clock CT_C8_A\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulH100ClockABadCount);
	printf("%10s: Number of errors on H.100 frame CT_FRAME_A\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulH100FrameABadCount);
	printf("%10s: Number of errors on H.100 clock CT_C8_B\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulH100ClockBBadCount);
	printf("%10s: Number of internal read timeout errors\t\t%d\n",
			ec_api->devname,
			pChipStats->ulInternalReadTimeoutCount);
	printf("%10s: Number of SDRAM refresh too late errors\t\t%d\n",
			ec_api->devname,
			pChipStats->ulSdramRefreshTooLateCount);
	printf("%10s: Number of PLL jitter errors\t\t\t\t%d\n",
			ec_api->devname,
			pChipStats->ulPllJitterErrorCount);
	printf("%10s: Number of HW tone event buffer has overflowed\t%d\n",
			ec_api->devname,
			pChipStats->ulOverflowToneEventsCount);
	printf("%10s: Number of SW tone event buffer has overflowed\t%d\n",
			ec_api->devname,
			pChipStats->ulSoftOverflowToneEventsCount);
	printf("%10s: Number of SW Playout event buffer has overflowed\t%d\n",
			ec_api->devname,
			pChipStats->ulSoftOverflowBufferPlayoutEventsCount);
	printf("\n");
	
	if (ec_api->u_chip_stats.f_ChipImageInfo){
		pChipImageInfo = ec_api->u_chip_stats.f_ChipImageInfo;
		printf("****** Echo Canceller Chip Image Info %s ******\n",
					ec_api->devname);
		printf("%10s: Echo Canceller chip image build description:\n%s\n",
					ec_api->devname,
					pChipImageInfo->szVersionNumber);
		printf("%10s: Echo Canceller chip build ID\t\t\t%d\n",
					ec_api->devname,
					pChipImageInfo->ulBuildId);
		printf("%10s: Echo Canceller image type\t\t\t\t%d\n",
					ec_api->devname,
					pChipImageInfo->ulImageType);
		printf("%10s: Maximum number of channels supported by the image\t%d\n",
					ec_api->devname,
					pChipImageInfo->ulMaxChannels);
		printf("\n");
	}
	return 0;
}

static int wanec_api_print_full_chip_stats(wan_ec_api_t *ec_api)
{
	tPOCT6100_CHIP_STATS		pChipStats;
	tPOCT6100_CHIP_IMAGE_INFO	pChipImageInfo;
	tPOCT6100_CHIP_TONE_INFO	pChipToneInfo;
	unsigned int				i;

	pChipStats = &ec_api->u_chip_stats.f_ChipStats;
	printf("****** Echo Canceller Chip Get Stats %s ******\n",
			ec_api->devname);

	printf("%10s: Number of channels currently open\t\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulNumberChannels);
	printf("%10s: Number of TSI connections currently open\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulNumberTsiCncts);
	printf("%10s: Number of conference bridges currently open\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulNumberConfBridges);
	printf("%10s: Number of playout buffers currently loaded\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulNumberPlayoutBuffers);
	printf("%10s: Number of framing error on H.100 bus\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulH100OutOfSynchCount);
	printf("%10s: Number of errors on H.100 clock CT_C8_A\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulH100ClockABadCount);
	printf("%10s: Number of errors on H.100 frame CT_FRAME_A\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulH100FrameABadCount);
	printf("%10s: Number of errors on H.100 clock CT_C8_B\t\t%d\n", 
			ec_api->devname,
			pChipStats->ulH100ClockBBadCount);
	printf("%10s: Number of internal read timeout errors\t\t%d\n",
			ec_api->devname,
			pChipStats->ulInternalReadTimeoutCount);
	printf("%10s: Number of SDRAM refresh too late errors\t\t%d\n",
			ec_api->devname,
			pChipStats->ulSdramRefreshTooLateCount);
	printf("%10s: Number of PLL jitter errors\t\t\t\t%d\n",
			ec_api->devname,
			pChipStats->ulPllJitterErrorCount);
	printf("%10s: Number of HW tone event buffer has overflowed\t%d\n",
			ec_api->devname,
			pChipStats->ulOverflowToneEventsCount);
	printf("%10s: Number of SW tone event buffer has overflowed\t%d\n",
			ec_api->devname,
			pChipStats->ulSoftOverflowToneEventsCount);
	printf("%10s: Number of SW Playout event buffer has overflowed\t%d\n",
			ec_api->devname,
			pChipStats->ulSoftOverflowBufferPlayoutEventsCount);
	printf("\n");
	
	if (ec_api->u_chip_stats.f_ChipImageInfo){
		pChipImageInfo = ec_api->u_chip_stats.f_ChipImageInfo;
		printf("****** Echo Canceller Chip Image Info %s ******\n",
					ec_api->devname);
		printf("%10s: Echo Canceller chip image build description:\n%s\n",
					ec_api->devname,
					pChipImageInfo->szVersionNumber);
		printf("%10s: Echo Canceller chip build ID\t\t\t%d\n",
					ec_api->devname,
					pChipImageInfo->ulBuildId);
		printf("%10s: Echo Canceller image type\t\t\t\t%d\n",
					ec_api->devname,
					pChipImageInfo->ulImageType);
		printf("%10s: Maximum number of channels supported by the image\t%d\n",
					ec_api->devname,
					pChipImageInfo->ulMaxChannels);
		printf("%10s: Support Acoustic echo cancellation\t%s\n",
					ec_api->devname,
					(pChipImageInfo->fAcousticEcho == TRUE) ?
								"TRUE" : "FALSE");
		printf("%10s: Support configurable tail length for Aec\t%s\n",
					ec_api->devname,
					(pChipImageInfo->fAecTailLength == TRUE) ?
								"TRUE" : "FALSE");
		printf("%10s: Support configurable default ERL\t%s\n",
					ec_api->devname,
					(pChipImageInfo->fDefaultErl == TRUE) ?
								"TRUE" : "FALSE");
		printf("%10s: Support configurable non-linearity A\t%s\n",
					ec_api->devname,
					(pChipImageInfo->fNonLinearityBehaviorA == TRUE) ?
								"TRUE" : "FALSE");
		printf("%10s: Support configurable non-linearity B\t%s\n",
					ec_api->devname,
					(pChipImageInfo->fNonLinearityBehaviorB == TRUE) ?
								"TRUE" : "FALSE");
		printf("%10s: Tone profile number built in the image\t%d\n",
					ec_api->devname,
					pChipImageInfo->ulToneProfileNumber);
		printf("%10s: Number of tone available in the image\t%d\n",
					ec_api->devname,
					pChipImageInfo->ulNumTonesAvailable);
		for(i = 0; i < pChipImageInfo->ulNumTonesAvailable; i++){
			pChipToneInfo = &pChipImageInfo->aToneInfo[i];
			printf("%10s: \tDetection Port: %s\tToneId: 0x%X\n",
				ec_api->devname,
				(pChipToneInfo->ulDetectionPort == cOCT6100_CHANNEL_PORT_SIN)?"SIN":
				(pChipToneInfo->ulDetectionPort == cOCT6100_CHANNEL_PORT_ROUT)?"ROUT":
				(pChipToneInfo->ulDetectionPort == cOCT6100_CHANNEL_PORT_ROUT)?"SOUT":
				(pChipToneInfo->ulDetectionPort == cOCT6100_CHANNEL_PORT_ROUT_SOUT)?"ROUT/SOUT":
												"INV",
				pChipToneInfo->ulToneID);
		}
		printf("\n");
	}
	return 0;
}


static int wanec_api_print_chan_stats(wan_ec_api_t *ec_api, int channel)
{
	tPOCT6100_CHANNEL_STATS		pChannelStats;
	tPOCT6100_CHANNEL_STATS_VQE	pChannelStatsVqe;

	pChannelStats = (tPOCT6100_CHANNEL_STATS)&ec_api->u_chan_stats.f_ChannelStats;
	printf("%10s:%2d: Echo Channel Operation Mode\t\t\t: %s\n",
					ec_api->devname,
					channel,
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_NORMAL)?
								"NORMAL":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_HT_FREEZE)?
								"HT FREEZE":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_HT_RESET)?
								"HT RESET":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_POWER_DOWN)?
								"POWER DOWN":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_NO_ECHO)?
								"NO ECHO":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_SPEECH_RECOGNITION)?
								"SPEECH RECOGNITION":
								"Unknown");
	printf("%10s:%2d: Enable Tone Disabler\t\t\t\t: %s\n",
		ec_api->devname, channel,
		(pChannelStats->fEnableToneDisabler==TRUE) ? "TRUE" : "FALSE");
	printf("%10s:%2d: Mute Ports\t\t\t\t\t: %s\n",
					ec_api->devname, channel,
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_RIN) ?
							"RIN" :
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_ROUT) ?
							"ROUT" :
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_SIN) ?
							"SIN" :
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_SOUT) ?
							"SOUT" :
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_NONE) ?
							"NONE" : "Unknown");
	printf("%10s:%2d: Enable Extended Tone Detection\t\t\t: %s\n",
		ec_api->devname, channel,
		(pChannelStats->fEnableExtToneDetection==TRUE) ? "TRUE" : "FALSE");

	if (pChannelStats->ulToneDisablerStatus == cOCT6100_INVALID_STAT){
		printf("%10s:%2d: Tone Disabler Status\t\t\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Tone Disabler Status\t\t\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStats->ulToneDisablerStatus==cOCT6100_TONE_DISABLER_EC_DISABLED)?
								"Disabled":"Enabled"); 
	}
	printf("%10s:%2d: Voice activity is detected on SIN port\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStats->fSinVoiceDetected==TRUE)? "TRUE":
			(pChannelStats->fSinVoiceDetected==FALSE)? "FALSE": "Unknown");
	printf("%10s:%2d: Echo canceller has detected and converged\t: %s\n",
			ec_api->devname, channel,
			(pChannelStats->fEchoCancellerConverged==TRUE)? "TRUE":
			(pChannelStats->fEchoCancellerConverged==FALSE)? "FALSE": "Unknown");
	if (pChannelStats->lRinLevel == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Average power of signal level on RIN\t\t: Invalid\n",
				ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Average power of signal level on RIN\t\t: %d dBm0\n",
				ec_api->devname, channel,
				pChannelStats->lRinLevel);
	}
	if (pChannelStats->lSinLevel == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Average power of signal level on SIN\t\t: Invalid\n",
				ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Average power of signal level on SIN\t\t: %d dBm0\n",
				ec_api->devname, channel,
				pChannelStats->lSinLevel);
	}
	if (pChannelStats->lRinAppliedGain == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Current gain applied to signal level on RIN\t: Invalid\n",
				ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Current gain applied to signal level on RIN\t: %d dB\n",
				ec_api->devname, channel,
				pChannelStats->lRinAppliedGain);
	}
	if (pChannelStats->lSoutAppliedGain == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Current gain applied to signal level on SOUT\t: Invalid\n",
				ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Current gain applied to signal level on SOUT\t: %d dB\n",
				ec_api->devname, channel,
				pChannelStats->lSoutAppliedGain);
	}
	if (pChannelStats->lComfortNoiseLevel == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Average power of the comfort noise injected\t: Invalid\n",
				ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Average power of the comfort noise injected\t: %d dBm0\n",
				ec_api->devname, channel,
				pChannelStats->lComfortNoiseLevel);
	}

	pChannelStatsVqe = &pChannelStats->VqeConfig;
	printf("%10s:%2d: (VQE) NLP status\t\t\t\t\t: %s\n",
				ec_api->devname, channel,
				(pChannelStatsVqe->fEnableNlp == TRUE) ? "TRUE" : "FALSE");
	printf("%10s:%2d: (VQE) Enable Tail Displacement\t\t\t: %s\n",
				ec_api->devname, channel,
				(pChannelStatsVqe->fEnableTailDisplacement == TRUE) ? "TRUE" : "FALSE");
	printf("%10s:%2d: (VQE) Echo Cancellation offset windowd (ms)\t: %d\n",
				ec_api->devname, channel,
				pChannelStatsVqe->ulTailDisplacement);
	printf("%10s:%2d: (VQE) Comfort noise mode\t\t\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStatsVqe->ulComfortNoiseMode == cOCT6100_COMFORT_NOISE_NORMAL) ? "NORMAL" :
			(pChannelStatsVqe->ulComfortNoiseMode == cOCT6100_COMFORT_NOISE_FAST_LATCH) ? "FAST LATCH" :
			(pChannelStatsVqe->ulComfortNoiseMode == cOCT6100_COMFORT_NOISE_EXTENDED) ? "EXTENDED" :
			(pChannelStatsVqe->ulComfortNoiseMode == cOCT6100_COMFORT_NOISE_OFF) ? "OFF" : "UNKNOWN");
	printf("%10s:%2d: (VQE) Acoustic Echo\t\t\t\t: %s\n",
				ec_api->devname, channel,
				(pChannelStatsVqe->fAcousticEcho == TRUE) ? "TRUE" : "FALSE");

	printf("\n");

	return 0;
}

static int wanec_api_print_full_chan_stats(wan_ec_api_t *ec_api, int channel)
{
	tPOCT6100_CHANNEL_STATS		pChannelStats;
	tPOCT6100_CHANNEL_STATS_VQE	pChannelStatsVqe;
	tPOCT6100_CHANNEL_STATS_CODEC	pChannelStatsCodec;

	pChannelStats = (tPOCT6100_CHANNEL_STATS)&ec_api->u_chan_stats.f_ChannelStats;
	printf("%10s:%2d: Echo Channel Operation Mode\t\t\t: %s\n",
					ec_api->devname,
					channel,
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_NORMAL)?
								"NORMAL":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_HT_FREEZE)?
								"HT FREEZE":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_HT_RESET)?
								"HT RESET":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_POWER_DOWN)?
								"POWER DOWN":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_NO_ECHO)?
								"NO ECHO":
		(pChannelStats->ulEchoOperationMode==cOCT6100_ECHO_OP_MODE_SPEECH_RECOGNITION)?
								"SPEECH RECOGNITION":
								"Unknown");
	printf("%10s:%2d: Enable Tone Disabler\t\t\t\t\t: %s\n",
		ec_api->devname, channel,
		(pChannelStats->fEnableToneDisabler==TRUE) ? "TRUE" : "FALSE");
	printf("%10s:%2d: Mute Ports\t\t\t\t\t: %s\n",
					ec_api->devname, channel,
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_RIN) ?
							"RIN" :
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_ROUT) ?
							"ROUT" :
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_SIN) ?
							"SIN" :
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_SOUT) ?
							"SOUT" :
		(pChannelStats->ulMutePortsMask==cOCT6100_CHANNEL_MUTE_PORT_NONE) ?
							"NONE" : "Unknown");
	printf("%10s:%2d: Enable Extended Tone Detection\t\t\t\t: %s\n",
		ec_api->devname, channel,
		(pChannelStats->fEnableExtToneDetection==TRUE) ? "TRUE" : "FALSE");
	if (pChannelStats->lCurrentERL == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Current Echo Return Loss\t\t\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Current Echo Return Loss\t\t\t\t: %d dB\n",
					ec_api->devname, channel,
					pChannelStats->lCurrentERL);
	}
	if (pChannelStats->lCurrentERLE == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Current Echo Return Loss Enhancement\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Current Echo Return Loss Enhancement\t\t: %d dB\n",
					ec_api->devname, channel,
					pChannelStats->lCurrentERLE);
	}
	if (pChannelStats->lMaxERL == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Maximum value of the ERL\t\t\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Maximum value of the ERL\t\t\t\t: %d dB\n",
					ec_api->devname, channel,
					pChannelStats->lMaxERL);
	}
	if (pChannelStats->lMaxERLE == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Maximum value of the ERLE\t\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Maximum value of the ERLE\t\t\t: %d dB\n",
					ec_api->devname, channel,
					pChannelStats->lMaxERLE);
	}
	if (pChannelStats->ulNumEchoPathChanges == cOCT6100_INVALID_STAT){
		printf("%10s:%2d: Number of Echo Path changes\t\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Number of Echo Path changes\t\t\t: %d\n",
					ec_api->devname, channel,
					pChannelStats->ulNumEchoPathChanges);
	}
	if (pChannelStats->ulCurrentEchoDelay == cOCT6100_INVALID_STAT){
		printf("%10s:%2d: Current Echo Delay\t\t\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Current Echo Delay\t\t\t\t: %d\n",
					ec_api->devname, channel,
					pChannelStats->ulCurrentEchoDelay);
	}
	if (pChannelStats->ulMaxEchoDelay == cOCT6100_INVALID_STAT){
		printf("%10s:%2d: Maximum Echo Delay\t\t\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Maximum Echo Delay\t\t\t\t: %d\n",
					ec_api->devname, channel,
					pChannelStats->ulMaxEchoDelay);
	}
	if (pChannelStats->ulToneDisablerStatus == cOCT6100_INVALID_STAT){
		printf("%10s:%2d: Tone Disabler Status\t\t\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Tone Disabler Status\t\t\t\t: %s\n",
					ec_api->devname, channel,
			(pChannelStats->ulToneDisablerStatus==cOCT6100_TONE_DISABLER_EC_DISABLED)?
								"Disabled":"Enabled"); 
	}
	printf("%10s:%2d: Voice activity is detected on SIN port\t\t: %s\n",
					ec_api->devname, channel,
		(pChannelStats->fSinVoiceDetected==TRUE)? "TRUE":
		(pChannelStats->fSinVoiceDetected==FALSE)? "FALSE": "Unknown");
	printf("%10s:%2d: Echo canceller has detected and converged\t: %s\n",
					ec_api->devname, channel,
		(pChannelStats->fEchoCancellerConverged==TRUE)? "TRUE":
		(pChannelStats->fEchoCancellerConverged==FALSE)? "FALSE": "Unknown");
	if (pChannelStats->lRinLevel == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Average power of signal level on RIN\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Average power of signal level on RIN\t\t: %d dBm0\n",
					ec_api->devname, channel,
					pChannelStats->lRinLevel);
	}
	if (pChannelStats->lSinLevel == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Average power of signal level on SIN\t\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Average power of signal level on SIN\t\t: %d dBm0\n",
					ec_api->devname, channel,
					pChannelStats->lSinLevel);
	}
	if (pChannelStats->lRinAppliedGain == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Current gain applied to signal level on RIN\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Current gain applied to signal level on RIN\t: %d dB\n",
					ec_api->devname, channel,
					pChannelStats->lRinAppliedGain);
	}
	if (pChannelStats->lSoutAppliedGain == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Current gain applied to signal level on SOUT\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Current gain applied to signal level on SOUT\t: %d dB\n",
					ec_api->devname, channel,
					pChannelStats->lSoutAppliedGain);
	}
	if (pChannelStats->lComfortNoiseLevel == cOCT6100_INVALID_SIGNED_STAT){
		printf("%10s:%2d: Average power of the comfort noise injected\t: Invalid\n",
					ec_api->devname, channel);
	}else{
		printf("%10s:%2d: Average power of the comfort noise injected\t: %d dBm0\n",
					ec_api->devname, channel,
					pChannelStats->lComfortNoiseLevel);
	}

	pChannelStatsVqe = &pChannelStats->VqeConfig;
	printf("%10s:%2d: (VQE) NLP status\t\t\t\t\t: %s\n",
				ec_api->devname, channel,
				(pChannelStatsVqe->fEnableNlp == TRUE) ? "TRUE" : "FALSE");
	printf("%10s:%2d: (VQE) Enable Tail Displacement\t\t\t: %s\n",
				ec_api->devname, channel,
				(pChannelStatsVqe->fEnableTailDisplacement == TRUE) ? "TRUE" : "FALSE");
	printf("%10s:%2d: (VQE) Maximum tail length\t\t\t: %d ms\n",
				ec_api->devname, channel,
				pChannelStatsVqe->ulTailLength);
	printf("%10s:%2d: (VQE) Rin Level control mode\t\t\t: %s\n",
				ec_api->devname, channel,
				(pChannelStatsVqe->fRinLevelControl == TRUE) ? "Enable" : "BYPASSED");
	printf("%10s:%2d: (VQE) Rin Control Signal gain\t\t\t: %d dB\n",
				ec_api->devname, channel,
				pChannelStatsVqe->lRinLevelControlGainDb);
	printf("%10s:%2d: (VQE) Sout Level control mode\t\t\t: %s\n",
				ec_api->devname, channel,
				(pChannelStatsVqe->fSoutLevelControl == TRUE) ? "Enable" : "BYPASSED");
	printf("%10s:%2d: (VQE) Sout Control Signal gain\t\t\t: %d dB\n",
				ec_api->devname, channel,
				pChannelStatsVqe->lSoutLevelControlGainDb);
	printf("%10s:%2d: (VQE) RIN Automatic Level Control\t\t: %s\n",
				ec_api->devname, channel,
				(pChannelStatsVqe->fRinAutomaticLevelControl == TRUE) ? "TRUE" : "FALSE");
	printf("%10s:%2d: (VQE) RIN Target Level Control\t\t\t: %d dBm0\n",
				ec_api->devname, channel,
				pChannelStatsVqe->lRinAutomaticLevelControlTargetDb);
	printf("%10s:%2d: (VQE) SOUT Automatic Level Control\t\t: %s\n",
				ec_api->devname, channel,
				(pChannelStatsVqe->fSoutAutomaticLevelControl == TRUE) ? "TRUE" : "FALSE");
	printf("%10s:%2d: (VQE) SOUT Target Level Control\t\t\t: %d dBm0\n",
				ec_api->devname, channel,
				pChannelStatsVqe->lSoutAutomaticLevelControlTargetDb);
	printf("%10s:%2d: (VQE) Comfort noise mode\t\t\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStatsVqe->ulComfortNoiseMode == cOCT6100_COMFORT_NOISE_NORMAL) ? "NORMAL" :
			(pChannelStatsVqe->ulComfortNoiseMode == cOCT6100_COMFORT_NOISE_FAST_LATCH) ? "FAST LATCH" :
			(pChannelStatsVqe->ulComfortNoiseMode == cOCT6100_COMFORT_NOISE_EXTENDED) ? "EXTENDED" :
			(pChannelStatsVqe->ulComfortNoiseMode == cOCT6100_COMFORT_NOISE_OFF) ? "OFF" : "UNKNOWN");
	printf("%10s:%2d: (VQE) Acoustic Echo\t\t\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStatsVqe->fAcousticEcho == TRUE) ? "TRUE" : "FALSE");

	printf("%10s:%2d: (VQE) Non Linearity Behavior A\t\t\t: %d\n",
			ec_api->devname, channel,
			pChannelStatsVqe->ulNonLinearityBehaviorA);
	printf("%10s:%2d: (VQE) Non Linearity Behavior B\t\t\t: %d\n",
			ec_api->devname, channel,
			pChannelStatsVqe->ulNonLinearityBehaviorB);
	printf("%10s:%2d: (VQE) Double Talk algorithm\t\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStatsVqe->ulDoubleTalkBehavior==cOCT6100_DOUBLE_TALK_BEH_NORMAL)?"NORMAL":
									"LESS AGGRESSIVE");
	printf("%10s:%2d: (VQE) Default ERL (not converged)\t\t: %d dB\n",
			ec_api->devname, channel,
			pChannelStatsVqe->lDefaultErlDb);
	printf("%10s:%2d: (VQE) Acoustic Echo Cancellation default ERL\t: %d dB\n",
			ec_api->devname, channel,
			pChannelStatsVqe->lAecDefaultErlDb);
	printf("%10s:%2d: (VQE) Maximum Acoustic Echo tail length\t\t: %d ms\n",
			ec_api->devname, channel,
			pChannelStatsVqe->ulAecTailLength);

	pChannelStatsCodec = &pChannelStats->CodecConfig;
	printf("%10s:%2d: (CODEC) Encoder channel port\t\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStatsCodec->ulEncoderPort == cOCT6100_CHANNEL_PORT_ROUT) ? "ROUT":
			(pChannelStatsCodec->ulEncoderPort == cOCT6100_CHANNEL_PORT_SOUT) ? "SOUT":"NO ENCODING");
	printf("%10s:%2d: (CODEC) Encoder rate\t\t\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G711_64KBPS) ? "G.711 64 kBps":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G726_40KBPS) ? "G.726 40 kBps":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G726_32KBPS) ? "G.726 32 kBps":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G726_24KBPS) ? "G.726 24 kBps":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G726_16KBPS) ? "G.726 16 kBps":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G727_40KBPS_4_1) ? "G.727 40 kBps (4:1)":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G727_40KBPS_3_2) ? "G.727 40 kBps (3:2)":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G727_40KBPS_2_3) ? "G.727 40 kBps (2:3)":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G727_32KBPS_4_0) ? "G.727 32 kBps (4:0)":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G727_32KBPS_3_1) ? "G.727 32 kBps (3:1)":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G727_32KBPS_2_2) ? "G.727 32 kBps (2:2)":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G727_24KBPS_3_0) ? "G.727 24 kBps (3:0)":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G727_24KBPS_2_1) ? "G.727 24 kBps (2:1)":
			(pChannelStatsCodec->ulEncodingRate == cOCT6100_G727_16KBPS_2_0) ? "G.727 16 kBps (2:0)":
											"UNKNOWN");
	printf("%10s:%2d: (CODEC) Decoder channel port\t\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStatsCodec->ulDecoderPort == cOCT6100_CHANNEL_PORT_RIN) ? "RIN":
			(pChannelStatsCodec->ulDecoderPort == cOCT6100_CHANNEL_PORT_SIN) ? "SIN":"NO DECODING");
	printf("%10s:%2d: (CODEC) Decoder rate\t\t\t\t: %s\n",
			ec_api->devname, channel,
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G711_64KBPS) ? "G.711 64 kBps":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G726_40KBPS) ? "G.726 40 kBps":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G726_32KBPS) ? "G.726 32 kBps":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G726_24KBPS) ? "G.726 24 kBps":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G726_16KBPS) ? "G.726 16 kBps":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G727_2C_ENCODED) ? "G.727 2C Encoded":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G727_3C_ENCODED) ? "G.727 3C Encoded":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G727_4C_ENCODED) ? "G.727 4C Encoded":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G726_ENCODED) ? "G.726 Encoded":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G711_G727_2C_ENCODED) ? "G.727 2C Encoded":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G711_G727_3C_ENCODED) ? "G.727 3C Encoded":
			(pChannelStatsCodec->ulDecodingRate == cOCT6100_G711_G727_4C_ENCODED) ? "G.727 4C Encoded":
											"UNKNOWN");
	printf("\n");
	return 0;
}

static int wanec_api_verbose(int verbose)
{
	int	ec_verbose;
		
	ec_verbose = (verbose == 0) ? WAN_EC_VERBOSE_NONE :
			  (verbose == 1) ? WAN_EC_VERBOSE_EXTRA1 :
						WAN_EC_VERBOSE_EXTRA2;
	return ec_verbose;
}


int wanec_api_init(void)
{
	memset(&ec_api, 0, sizeof(ec_api));
	return 0;
}

int wanec_api_config(	char*		devname,
			int		verbose)
{
	
	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd	= WAN_EC_CMD_CONFIG;
	ec_api.verbose	= wanec_api_verbose(verbose);
	return wanec_api_lib_config(&ec_api, 1);
}

int wanec_api_release(	char*		devname,
			int		verbose)
{
		
	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd	= WAN_EC_CMD_RELEASE;
	ec_api.verbose	= wanec_api_verbose(verbose);
	return wanec_api_lib_cmd(&ec_api);
}

int wanec_api_enable(	char*		devname,
			unsigned long	channel_map,
			int		verbose)
{
		
	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd		= WAN_EC_CMD_ENABLE;
	ec_api.verbose		= wanec_api_verbose(verbose);
	ec_api.channel_map	= channel_map;
	return wanec_api_lib_cmd(&ec_api);
}

int wanec_api_disable(	char*		devname,
			unsigned long	channel_map,
			int		verbose)
{
		
	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd		= WAN_EC_CMD_DISABLE;
	ec_api.verbose		= wanec_api_verbose(verbose);
	ec_api.channel_map	= channel_map;
	return wanec_api_lib_cmd(&ec_api);
}

int wanec_api_bypass(	char*		devname,
			int		enable,
			unsigned long	channel_map,
			int		verbose)
{
		
	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd		= (enable) ?
					WAN_EC_CMD_BYPASS_ENABLE : 
					WAN_EC_CMD_BYPASS_DISABLE;
	ec_api.verbose	= wanec_api_verbose(verbose);
	ec_api.channel_map	= channel_map;
	return wanec_api_lib_cmd(&ec_api);
}

int wanec_api_mode(	char*		devname,
			int		mode,
			unsigned long	channel_map,
			int		verbose)
{
		
	memcpy(ec_api.devname, devname, strlen(devname));
	switch(mode){
	case 0:
		ec_api.cmd = WAN_EC_CMD_MODE_POWERDOWN;
		break;
	case 1:
		ec_api.cmd = WAN_EC_CMD_MODE_NORMAL;
		break;
	case 2:
		ec_api.cmd = WAN_EC_CMD_MODIFY_CHANNEL;
		break;
	default:
		return -EINVAL;
	}
	ec_api.verbose	= wanec_api_verbose(verbose);
	ec_api.channel_map	= channel_map;
	return wanec_api_lib_cmd(&ec_api);
}

int wanec_api_dtmf(	char*		devname,
			int		enable,
			unsigned long	channel_map,
			unsigned char	port,
			unsigned char	type,
			int		verbose)
{
		
	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd		= (enable) ?
					WAN_EC_CMD_DTMF_ENABLE : 
					WAN_EC_CMD_DTMF_DISABLE;
	ec_api.verbose		= wanec_api_verbose(verbose);
	ec_api.channel_map	= channel_map;
	ec_api.u_dtmf_config.port = port;	//WAN_EC_CHANNEL_PORT_SOUT;
	ec_api.u_dtmf_config.type = type;	//WAN_EC_TONE_PRESENT;
	return wanec_api_lib_cmd(&ec_api);
}

int wanec_api_stats(	char*		devname,
			int		full,
			int		channel,
			int		reset,
			int		verbose)
{
	int	err;
	
	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd			= (full) ?
						WAN_EC_CMD_STATS_FULL : 
						WAN_EC_CMD_STATS;
	ec_api.verbose			= wanec_api_verbose(verbose);
	if (channel){
		ec_api.channel_map		= (1 << channel);
		ec_api.u_chan_stats.reset 	= reset;
	}else{
		ec_api.channel_map		= 0;
		ec_api.u_chip_stats.reset 	= reset;
#if 0
		if (full){
			ec_api.u_chip_stats.f_ChipImageInfo =
				malloc(sizeof(tOCT6100_CHIP_IMAGE_INFO));
			memset(ec_api.u_chip_stats.f_ChipImageInfo,
					0, sizeof(tOCT6100_CHIP_IMAGE_INFO));
		}
#endif		
	}
	err = wanec_api_lib_cmd(&ec_api);
	if (err) return err;
	if (ec_api.err) return 0;

	if (channel){
		if (ec_api.cmd == WAN_EC_CMD_STATS){
			err = wanec_api_print_chan_stats(&ec_api, channel);
		}else{
			err = wanec_api_print_full_chan_stats(&ec_api, channel);
		}
	}else{
		if (ec_api.cmd == WAN_EC_CMD_STATS){
			err = wanec_api_print_chip_stats(&ec_api);
		}else{
			err = wanec_api_print_full_chip_stats(&ec_api);
		}
	}
	return err;

}

int wanec_api_tone_load(char*	devname,
			char*	tone,
			int	verbose)
{
	int	err;

	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd	= WAN_EC_CMD_TONE_LOAD;
	ec_api.verbose	= wanec_api_verbose(verbose);
	memcpy(ec_api.u_tone_config.tone, tone, strlen(tone));	
	
	err = wanec_api_lib_toneload(&ec_api);
	if (err) return err;
	if (ec_api.err) return 0;
	return 0;
}

int wanec_api_tone_unload(	char*		devname,
				unsigned int	tone_id,
				int		verbose)
{
	int	err;

	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd				= WAN_EC_CMD_TONE_LOAD;
	ec_api.verbose	= wanec_api_verbose(verbose);
	ec_api.u_tone_config.buffer_index	= (UINT32)tone_id;
	err = wanec_api_lib_cmd(&ec_api);
	if (err) return err;
	if (ec_api.err) return 0;
	return 0;
}

int wanec_api_playout(	char*		devname,
			int		start,
			int		channel,
			unsigned int	tone_id,
			int		verbose)
{
	int	err;

	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd	= (start) ? 
				WAN_EC_CMD_PLAYOUT_START :
				WAN_EC_CMD_PLAYOUT_STOP;
	ec_api.verbose	= wanec_api_verbose(verbose);
	ec_api.channel		= channel;
	ec_api.channel_map	= (1 << channel);
	ec_api.u_playout.index	= (UINT32)tone_id;
	ec_api.u_playout.duration	= 5000;
	err = wanec_api_lib_cmd(&ec_api);
	if (err) return err;
	if (ec_api.err) return 0;
	return 0;
}

int wanec_api_monitor(	char*	devname,
			int	channel,
			int	verbose)
{
	int	err;

	memcpy(ec_api.devname, devname, strlen(devname));
	ec_api.cmd			= WAN_EC_CMD_MONITOR;
	ec_api.verbose	= wanec_api_verbose(verbose);
	ec_api.channel			= channel;
	ec_api.channel_map		= (1 << channel);
	
	err = wanec_api_lib_monitor(&ec_api);
	if (err) return err;
	if (ec_api.err) return 0;
	return 0;
}

int wanec_api_param(char *key, char *value)
{
	strlcpy(ec_api.param[ec_api.param_no].key,
					key, MAX_PARAM_LEN); 
	strlcpy(ec_api.param[ec_api.param_no].value,
					value, MAX_VALUE_LEN); 
	ec_api.param_no++;
	return 0;
}
