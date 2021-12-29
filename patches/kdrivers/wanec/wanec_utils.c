/******************************************************************************
** Copyright (c) 2005
**	Alex Feldman <al.feldman@sangoma.com>.  All rights reserved.
**
** ============================================================================
** Aprl 29, 2001	Alex Feldma	Initial version.
******************************************************************************/

/******************************************************************************
**			   INCLUDE FILES
******************************************************************************/

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

#include "oct6100_api.h"
#include "oct6100_version.h"

#include "wanec_iface.h"

/******************************************************************************
**			  DEFINES AND MACROS
******************************************************************************/
#define	DTYPE_BOOL	1
#define	DTYPE_UINT32	2
#define	DTYPE_INT32	3

#if !defined(offsetof)
# define offsetof(s, e) ((size_t)&((s*)0)->e)
#endif
/******************************************************************************
**			STRUCTURES AND TYPEDEFS
******************************************************************************/
typedef struct key_word		/* Keyword table entry */
{
	u_int8_t	*key;		/* -> keyword */
	u_int16_t	offset;		/* offset of the related parameter */
	u_int16_t	dtype;		/* data type */
} key_word_t;

/******************************************************************************
**			   GLOBAL VARIABLES
******************************************************************************/
/* Driver version string */
extern int	verbose;

key_word_t chip_param[] =
{
	{ "fEnableAcousticEcho",
	  offsetof(tOCT6100_CHIP_OPEN, fEnableAcousticEcho),
	  DTYPE_BOOL },
	{ NULL, 0, 0 }
};

key_word_t ChannelModifyParam[] =
{
	{ "ulEchoOperationMode",
	  offsetof(tOCT6100_CHANNEL_MODIFY, ulEchoOperationMode),
	  DTYPE_UINT32 },
	{ "fEnableToneDisabler",
	  offsetof(tOCT6100_CHANNEL_MODIFY, fEnableToneDisabler),
	  DTYPE_BOOL },
	{ "fApplyToAllChannels",
	  offsetof(tOCT6100_CHANNEL_MODIFY, fApplyToAllChannels),
	  DTYPE_BOOL },
	{ "fEnableNlp",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fEnableNlp),
	  DTYPE_BOOL },
	{ "fEnableTailDisplacement",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fEnableTailDisplacement),
	  DTYPE_BOOL },
	{ "ulTailDisplacement",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,ulTailDisplacement),
	  DTYPE_UINT32 },
	{ "fRinLevelControl",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fRinLevelControl),
	  DTYPE_BOOL },
	{ "lRinLevelControlGainDb",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,lRinLevelControlGainDb),
	  DTYPE_INT32 },
	{ "fSoutLevelControl",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fSoutLevelControl),
	  DTYPE_BOOL },
	{ "lSoutLevelControlGainDb",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,lSoutLevelControlGainDb),
	  DTYPE_INT32 },
	{ "fRinAutomaticLevelControl",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fRinAutomaticLevelControl),
	  DTYPE_BOOL },
	{ "lRinAutomaticLevelControlTargetDb",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,lRinAutomaticLevelControlTargetDb),
	  DTYPE_INT32 },
	{ "fSoutAutomaticLevelControl",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fSoutAutomaticLevelControl),
	  DTYPE_BOOL },
	{ "lSoutAutomaticLevelControlTargetDb",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,lSoutAutomaticLevelControlTargetDb),
	  DTYPE_INT32 },
	{ "ulAlcNoiseBleedOutTime",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,ulAlcNoiseBleedOutTime),
	  DTYPE_UINT32 },
	{ "fRinHighLevelCompensation",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fRinHighLevelCompensation),
	  DTYPE_BOOL },
	{ "lRinHighLevelCompensationThresholdDb",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,lRinHighLevelCompensationThresholdDb),
	  DTYPE_INT32 },
	{ "fSoutAdaptiveNoiseReduction",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fSoutAdaptiveNoiseReduction),
	  DTYPE_BOOL },
	{ "fRoutNoiseReduction",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fRoutNoiseReduction),
	  DTYPE_BOOL },
	{ "ulComfortNoiseMode",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,ulComfortNoiseMode),
	  DTYPE_UINT32 },
	{ "fAcousticEcho",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fAcousticEcho),
	  DTYPE_BOOL },
	{ "fSoutNoiseBleaching",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,fSoutNoiseBleaching),
	  DTYPE_BOOL },
	{ "ulNonLinearityBehaviorA",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,ulNonLinearityBehaviorA),
	  DTYPE_UINT32 },
	{ "ulNonLinearityBehaviorB",
	  offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)+offsetof(tOCT6100_CHANNEL_MODIFY_VQE,ulNonLinearityBehaviorB),
	  DTYPE_UINT32 },
	{ NULL, 0, 0 }
};

/******************************************************************************
** 			FUNCTION PROTOTYPES
******************************************************************************/
int wanec_ChipParam(wan_ec_t*,tPOCT6100_CHIP_OPEN, wan_ec_api_t*);
int wanec_ChanParam(wan_ec_t*,tPOCT6100_CHANNEL_MODIFY, wan_ec_api_t*);
int wanec_ChanParamList(wan_ec_t *ec);

/******************************************************************************
** 			FUNCTION DEFINITIONS
******************************************************************************/
static int
set_conf_param (	wan_ec_t	*ec,
			wanec_param_t	*param,
			key_word_t	*dtab,
			void		*conf)
{
	/* Search a keyword in configuration definition table */
	for (; dtab->key && strcmp(dtab->key, param->key); ++dtab);
	
	if (dtab->key == NULL){
		DEBUG_EVENT("%s: param=%s\n", ec->name, param->key);
		return -EINVAL;	/* keyword not found */
	}

	DEBUG_EVENT(" * Setting %s to %s\n", param->key, param->value);

	switch (dtab->dtype) {
	case DTYPE_BOOL:
		if (memcmp(param->value, "TRUE", 4) == 0){
			DEBUG_EVENT(
			"%s: *** Modify %s to %s\n",
					ec->name, param->key, param->value);
			*(BOOL*)((char*)conf + dtab->offset) = TRUE;
		}else if (memcmp(param->value, "FALSE", 5) == 0){
			DEBUG_EVENT(
			"%s: *** Modify %s to %s\n",
					ec->name, param->key, param->value);
			*(BOOL*)((char*)conf + dtab->offset) = FALSE;
		}else{
			DEBUG_EVENT(
			"%s: Invalid parameter type (%d)\n",
				ec->name, dtab->dtype);
			return -EINVAL;
		}
		break;
	case DTYPE_UINT32:
		DEBUG_EVENT("%s: *** Modify %s to %d\n",
						ec->name, param->key, param->value1);
		*(UINT32*)((char*)conf + dtab->offset) = param->value1;
		break;
	case DTYPE_INT32:
		DEBUG_EVENT("%s: *** Modify %s to %d\n",
						ec->name, param->key, param->value1);
		*(INT32*)((char*)conf + dtab->offset) = param->value1;
		break;
	default:
		return -EINVAL;
	}
	return dtab->offset;
}

int wanec_ChipParam(	wan_ec_t		*ec,
				tPOCT6100_CHIP_OPEN	f_pOpenChip,
				wan_ec_api_t		*ec_api)
{
	int	iParam=0;

	DEBUG_EVENT(
	"%s: Parsing advanced Chip params\n", ec->name);
	while(iParam < ec_api->param_no){
		set_conf_param(	ec,
				&ec_api->param[iParam],
				chip_param,
				f_pOpenChip);
		iParam++;
	}
	return 0;
}


int wanec_ChanParam(	wan_ec_t			*ec,
				tPOCT6100_CHANNEL_MODIFY	f_pChannelModify,
				wan_ec_api_t			*ec_api)
{
	int	iParam=0, err;

	DEBUG_EVENT(
	"%s: Parsing advanced Channel params\n", ec->name);
	while(iParam < ec_api->param_no){
		err = set_conf_param(	ec,
					&ec_api->param[iParam],
					ChannelModifyParam,
					f_pChannelModify);
		if (err < 0) return -EINVAL;
		if (err < offsetof(tOCT6100_CHANNEL_MODIFY, TdmConfig)){
			/* no extra action */
		}else if (err < offsetof(tOCT6100_CHANNEL_MODIFY, VqeConfig)){
			f_pChannelModify->fTdmConfigModified = TRUE;
		}else if (err < offsetof(tOCT6100_CHANNEL_MODIFY, CodecConfig)){
			f_pChannelModify->fVqeConfigModified = TRUE;
		}else{
			f_pChannelModify->fCodecConfigModified = TRUE;
		}
		iParam++;
	}
	return 0;
}


int wanec_ChanParamList(wan_ec_t *ec)
{
	key_word_t	*dtab = ChannelModifyParam;

	for (; dtab->key; ++dtab){
		
		switch (dtab->dtype) {

		case DTYPE_UINT32:
			DEBUG_EVENT("%s: Channel paramater %s  = UINT32 value\n",
						ec->name, dtab->key);
			break;
		case DTYPE_INT32:
			DEBUG_EVENT("%s: Channel paramater %s  = INT32 value\n",
						ec->name, dtab->key);
			break;
		case DTYPE_BOOL:
			DEBUG_EVENT("%s: Channel paramater %s  = TRUE/FALSE value\n",
						ec->name, dtab->key);
			break;
		default:
			DEBUG_EVENT("%s: Channel paramater %s  = UNKNOWN value\n",
						ec->name, dtab->key);
			break;
		}
	}
	return 0;
}

