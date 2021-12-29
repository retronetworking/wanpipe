/******************************************************************************
** Copyright (c) 2005
**	Alex Feldman <al.feldman@sangoma.com>.  All rights reserved.
**
** ============================================================================
** Aprl 29, 2005	Alex Feldman	Initial version.
** Feb  15, 2006	Alex Feldman	Version 2.1
**					1) 64-bit support
**					2) API 3.9 with GPL
******************************************************************************/

/******************************************************************************
**			   INCLUDE FILES
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/queue.h>
#include <netinet/in.h>

#include "wan_oct6100.h"
#include "wan_oct6100tones.h"

/******************************************************************************
**			  DEFINES AND MACROS
******************************************************************************/
#define WAN_ECD_VERSION		"2.1"
#define MAX_LOG_LEN		255
#undef	ENABLE_TONE_PLAY
#undef	ENABLE_PRODBIST
#undef	ENABLE_ACOUSTICECHO

#define MAX_EC_PORT_RANGE	32

#define OCT6116_32S_IMAGE_NAME	"OCT6116-32S.ima"
#define OCT6126_128S_IMAGE_NAME	"OCT6126-128S.ima"
#define OCT6116_256S_IMAGE_NAME	"OCT6116-256S.ima"
#define OCT6116_32S_IMAGE_PATH	WAN_EC_DIR "/" OCT6116_32S_IMAGE_NAME
#define OCT6126_128S_IMAGE_PATH	WAN_EC_DIR "/" OCT6126_128S_IMAGE_NAME
#define OCT6116_256S_IMAGE_PATH	WAN_EC_DIR "/" OCT6116_256S_IMAGE_NAME

/******************************************************************************
**			STRUCTURES AND TYPEDEFS
******************************************************************************/
/******************************************************************************
**			   GLOBAL VARIABLES
******************************************************************************/
struct wan_oct6100_image
{
	int	hwec_chan_no;
	UINT32	max_channels;
	UINT32	memory_chip_size;
	UINT32	debug_data_mode;
	char	*image;	
	char	*image_path;	
} wan_oct6100_image_list[] =
	{
		{ 	16,
			32, cOCT6100_MEMORY_CHIP_SIZE_8MB,
			cOCT6100_DEBUG_GET_DATA_MODE_16S,
			OCT6116_32S_IMAGE_NAME, OCT6116_32S_IMAGE_PATH },
		{ 	32,
			32, cOCT6100_MEMORY_CHIP_SIZE_8MB,
			cOCT6100_DEBUG_GET_DATA_MODE_16S,
			OCT6116_32S_IMAGE_NAME, OCT6116_32S_IMAGE_PATH },
		{	128,
			128, cOCT6100_MEMORY_CHIP_SIZE_16MB,
			cOCT6100_DEBUG_GET_DATA_MODE_120S,
			OCT6126_128S_IMAGE_NAME, OCT6126_128S_IMAGE_PATH },
		{	256, /* FIXME: Update this value for 256ch image */
			128, cOCT6100_MEMORY_CHIP_SIZE_16MB,
			cOCT6100_DEBUG_GET_DATA_MODE_16S,
			OCT6126_128S_IMAGE_NAME, OCT6126_128S_IMAGE_PATH },
		{	0, 0, 0, 0, NULL }
	};

WAN_LIST_HEAD(wan_ecd_head_, wan_ecd_) wan_ecd_head = 
		WAN_LIST_HEAD_INITIALIZER(wan_ecd_head);
int verbose = 0;
CHAR *ToneBufferPaths[WAN_NUM_PLAYOUT_TONES] = 
{
	WAN_EC_BUFFERS "/DTMF_0_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_1_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_2_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_3_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_4_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_5_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_6_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_7_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_8_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_9_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_A_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_B_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_C_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_D_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_STAR_ulaw.pcm",
	WAN_EC_BUFFERS "/DTMF_POUND_ulaw.pcm"
};
UINT32 DetectedToneNumbers[WAN_NUM_DTMF_TONES] = 
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

/******************************************************************************
** 			FUNCTION PROTOTYPES
******************************************************************************/
extern int exec_read_cmd(void *sc, unsigned int, unsigned int, unsigned int*);
extern int exec_write_cmd(void *sc, unsigned int, unsigned int, unsigned int);
extern INT wan_ecd_dev_open(char*);
extern INT wan_ecd_dev_close(INT f_Handle);
extern INT wan_ecd_dev_ioctl( int, wan_ec_api_t* );

extern int wan_ecd_ChipParam(wan_ecd_t*,tPOCT6100_CHIP_OPEN, wan_ec_msg_t*);
extern int wan_ecd_ChanParam(wan_ecd_t*,tPOCT6100_CHANNEL_MODIFY, wan_ec_msg_t*);
extern int wan_ecd_ChanParamList(wan_ecd_t*);

static int wan_ecd_ChipOpen(wan_ec_dev_t *ec_dev, wan_ec_msg_t *ec_msg, int);
static int wan_ecd_ChipClose(wan_ecd_t *ec);
static int wan_ecd_TonesEnable(wan_ecd_t *ec, int channel);
static int wan_ecd_TonesDisable(wan_ecd_t *ec, int channel);
static int wan_ecd_EventTone(wan_ecd_t *ec);

#if defined(ENABLE_PRODBIST)
static int wan_ecd_ProductionBist(wan_ecd_t *ec);
#endif

static int wan_ecd_fe2ec_channel(wan_ec_dev_t*, int);
static int wan_ecd_ec2fe_channel(wan_ecd_t*, int, wan_ec_dev_t**);

/******************************************************************************
** 			FUNCTION DEFINITIONS
******************************************************************************/
//#define DEBUG
#if defined(DEBUG)
static void *my_malloc(size_t size)
{
	void *ptr = malloc(size);
	printf("ADBG: Allocate %d bytes (%p)\n", size, ptr);
	return ptr;
}
static void my_free(void *ptr)
{
	printf("ADBG: Free %p pointer\n", ptr);
	free(ptr);
	return;
}

#define malloc(x) my_malloc(x)
#define free(x) my_free(x)
#endif

#if defined(ENABLE_TONE_PLAY)
static int wan_ecd_delay(int sec)
{
	struct timeval	timeout;

	timeout.tv_sec = sec;
	timeout.tv_usec = 0;
	return select(1,NULL,NULL,NULL, &timeout);
}
#endif

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:	ExampleLibLoadFile

Description:    This function opens the specified file, allocates memory to
		store it's content and transfer the file content into the 
		buffer.

IMPORTANT:				**
		The memory allocated by this function call must be freed 
		later
					**

-------------------------------------------------------------------------------
|	Argument		|	Description
-------------------------------------------------------------------------------

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
UINT32	LibLoadImageFile(
				wan_ecd_t	*ec, 
				IN	PSZ	f_pszFileName,
				OUT	PUINT8*	f_ppbyFileData,
				OUT	PUINT32	f_pulFileByteSize )
{
	UINT32	ulNumBytesRead;
	FILE*	pFile;
	
	/* Validate the parameters.*/
	if ( f_pszFileName == NULL || f_pulFileByteSize == NULL ){
		PRINT(verbose, "ERROR: %s: Invalid image filename!\n",
					ec->name);
		return EINVAL;
	}

	/*==================================================================*/
	/* Open the file.*/
	
	pFile = fopen( f_pszFileName, "rb" );
	if ( pFile == NULL ){
		PRINT(verbose, "ERROR: %s: Failed to open image file %s!\n",
					ec->name,
					f_pszFileName);
		return EINVAL;
	}

	/*====================================================================*/
	/* Extract the file length.*/

	fseek( pFile, 0, SEEK_END );
	*f_pulFileByteSize = (UINT32)ftell( pFile );
	fseek( pFile, 0, SEEK_SET );
	
	if ( *f_pulFileByteSize == 0 ){
		fclose( pFile );
		PRINT(verbose, "ERROR: %s: Image file is too short!\n",
					ec->name);
		return EINVAL;
	}

	/*==================================================================*/
	/* Allocate enough memory to store the file content.*/
	
	*f_ppbyFileData = (PUINT8)malloc( *f_pulFileByteSize * sizeof(BYTE) );
	if ( *f_ppbyFileData == NULL ){
		fclose( pFile );
		PRINT(verbose, "ERROR: %s: Failed allocate memory for image file!\n",
					ec->name);
		return EINVAL;
	}

	/*==================================================================*/
	/* Read the content of the file.*/

	ulNumBytesRead = fread( *f_ppbyFileData, sizeof(UINT8), *f_pulFileByteSize, pFile );
	if ( ulNumBytesRead != *f_pulFileByteSize ){
		free( *f_ppbyFileData );
		fclose( pFile );
		PRINT(verbose, "ERROR: %s: Failed to read image file!\n",
					ec->name);
		return EINVAL;
	}

	
	/* The content of the file should now be in the f_pbyFileData memory,
	 * we can close the file and return to the calling function.*/
	fclose( pFile );
	
	return 0;
}



static CHAR* wan_ecd_ToneId2Str(UINT32 f_ulToneId)
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

static unsigned int wan_ecd_ConvertToneId(UINT32 f_ulToneId)
{
	switch (f_ulToneId){ 
	/* DTMF Section */
	case ROUT_DTMF_0: return WAN_EC_ROUT_DTMF_0;
	case ROUT_DTMF_1: return WAN_EC_ROUT_DTMF_1;
	case ROUT_DTMF_2: return WAN_EC_ROUT_DTMF_2;
	case ROUT_DTMF_3: return WAN_EC_ROUT_DTMF_3;
	case ROUT_DTMF_4: return WAN_EC_ROUT_DTMF_4;
	case ROUT_DTMF_5: return WAN_EC_ROUT_DTMF_5;
	case ROUT_DTMF_6: return WAN_EC_ROUT_DTMF_6;
	case ROUT_DTMF_7: return WAN_EC_ROUT_DTMF_7;
	case ROUT_DTMF_8: return WAN_EC_ROUT_DTMF_8;
	case ROUT_DTMF_9: return WAN_EC_ROUT_DTMF_9;
	case ROUT_DTMF_A: return WAN_EC_ROUT_DTMF_A;
	case ROUT_DTMF_B: return WAN_EC_ROUT_DTMF_B;
	case ROUT_DTMF_C: return WAN_EC_ROUT_DTMF_C;
	case ROUT_DTMF_D: return WAN_EC_ROUT_DTMF_D;
	case ROUT_DTMF_STAR: return WAN_EC_ROUT_DTMF_STAR;
	case ROUT_DTMF_POUND: return WAN_EC_ROUT_DTMF_POUND;
	case SOUT_DTMF_0: return WAN_EC_SOUT_DTMF_0;
	case SOUT_DTMF_1: return WAN_EC_SOUT_DTMF_1;
	case SOUT_DTMF_2: return WAN_EC_SOUT_DTMF_2;
	case SOUT_DTMF_3: return WAN_EC_SOUT_DTMF_3;
	case SOUT_DTMF_4: return WAN_EC_SOUT_DTMF_4;
	case SOUT_DTMF_5: return WAN_EC_SOUT_DTMF_5;
	case SOUT_DTMF_6: return WAN_EC_SOUT_DTMF_6;
	case SOUT_DTMF_7: return WAN_EC_SOUT_DTMF_7;
	case SOUT_DTMF_8: return WAN_EC_SOUT_DTMF_8;
	case SOUT_DTMF_9: return WAN_EC_SOUT_DTMF_9;
	case SOUT_DTMF_A: return WAN_EC_SOUT_DTMF_A;
	case SOUT_DTMF_B: return WAN_EC_SOUT_DTMF_B;
	case SOUT_DTMF_C: return WAN_EC_SOUT_DTMF_C;
	case SOUT_DTMF_D: return WAN_EC_SOUT_DTMF_D;
	case SOUT_DTMF_STAR: return WAN_EC_SOUT_DTMF_STAR;
	case SOUT_DTMF_POUND: return WAN_EC_SOUT_DTMF_POUND;
	}
	return 0x00000000;
}

static CHAR* wan_ecd_ToneType2Str(UINT32 f_ulToneType)
{
	switch (f_ulToneType){ 
	case cOCT6100_TONE_PRESENT: return "cOCT6100_TONE_PRESENT";
	case cOCT6100_TONE_STOP: return "cOCT6100_TONE_STOP";
	default: return "INVALID TONE TYPE!";
	}
}
static unsigned char wan_ecd_ConvertToneType(UINT32 f_ulToneType)
{
	switch (f_ulToneType){ 
	case cOCT6100_TONE_PRESENT: return WAN_EC_TONE_PRESENT;
	case cOCT6100_TONE_STOP: return WAN_EC_TONE_STOP;
	}
	return WAN_EC_TONE_NONE;
}


#if defined(ENABLE_TONE_PLAY)
static UINT32 wan_ecd_Char2ToneIndex(CHAR tone_ch)
{
	switch(tone_ch){
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'A': return 10;
	case 'a': return 10;
	case 'B': return 11;
	case 'b': return 11;
	case 'c': return 12;
	case 'C': return 12;
	case 'd': return 13;
	case 'D': return 13;
	case '*': return 14;
	case '#': return 15;
	default : return 0;
	}
	return 0;
}
#endif

static int wan_ecd_hndl2ec_channel(wan_ecd_t *ec, UINT32 ChannelHndl)
{
	int	channel = 0;

	for(channel = 0; channel < ec->max_channels; channel++){
		if (ec->pEchoChannelHndl[channel] == ChannelHndl){
			return channel;
		}
	}
	return 0;
}

static int wan_ecd_fe2ec_channel(wan_ec_dev_t *ec_dev, int fe_channel)
{
	/*ec_channel = ec_dev->fe_lineno * ec_dev->fe_max_channels + channel;*/
	return ec_dev->fe_lineno * MAX_EC_PORT_RANGE + fe_channel;
}

static int wan_ecd_ec2fe_channel(wan_ecd_t *ec, int ec_chan, wan_ec_dev_t **ec_dev)
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

tOCT6100_CHIP_IMAGE_INFO	f_ChipImageInfo;
#if 0
tOCT6100_GET_HW_REVISION	f_Revision;
#endif
static int wan_ecd_ChipStats(wan_ecd_t *ec, wan_ec_msg_t *ec_msg, int reset)
{
	tOCT6100_CHIP_STATS	f_ChipStats;
	UINT32			ulResult;

	PRINT1(verbose, "%s: Reading chip statistics...\n",
					ec->name);
	Oct6100ChipGetStatsDef( &f_ChipStats );
	f_ChipStats.fResetChipStats	= reset;
	ulResult = Oct6100ChipGetStats(
				ec->pChipInstance,
				&f_ChipStats);
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"%s: Reading chip statistics...\tFailed (err=0x%X)\n",
					ec->name, ulResult);
		return EINVAL;
	}
	if (ec_msg){
		memcpy(	&ec_msg->msg_chip_stat.f_ChipStats,
			&f_ChipStats,
			sizeof(tOCT6100_CHIP_STATS));
	}

	PRINT1(verbose, "%s: Reading chip image info...\n",
				ec->name); 
	Oct6100ChipGetImageInfoDef( &f_ChipImageInfo );
	ulResult = Oct6100ChipGetImageInfo(
				ec->pChipInstance,
				&f_ChipImageInfo);
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"%s: Reading chip image info...\tFailed (err=0x%X)\n",
					ec->name, ulResult); 
		return EINVAL;
	}
	if (ec_msg){
		memcpy(	&ec_msg->msg_chip_stat.f_ChipImageInfo,
			&f_ChipImageInfo,
			sizeof(tOCT6100_CHIP_IMAGE_INFO));
	}else{
		if (f_ChipImageInfo.ulMaxChannels < ec->max_channels){
			ec->max_channels = f_ChipImageInfo.ulMaxChannels-1;
		}
	}
	
#if 0
	PRINT1(verbose, "%s: Reading hw revision...\n",
					ec->name); 
	ec->f_Revision.ulUserChipId = 0;
	ec->f_Revision.pProcessContext = &ec->f_Context;
	ulResult = Oct6100GetHwRevision(&ec->f_Revision);
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"%s: Reading hw revision...\tFailed (err=0x%X)\n",
					ec->name, ulResult); 
		return EINVAL;
	}
#endif	
	return 0;
}

#if defined(ENABLE_TONE_PLAY)
static int wan_ecd_BufferPlayoutLoad(wan_ecd_t *ec, int tdmv_law)
{
	tOCT6100_BUFFER_LOAD		BufferLoad;
	UINT32				ulBufferSize, ulResult;
	PUINT8				pbyBufferData = NULL;
	INT				i;

	PRINT1(verbose, "%s: Loading Tones buffers into OCT6100 Chip ...\n",
					ec->name);
	for(i = 0; i < WAN_NUM_PLAYOUT_TONES; i++){
		ulResult = LibLoadImageFile(	ec,
						ToneBufferPaths[i],
						&pbyBufferData,
						&ulBufferSize);
		if ( ulResult ){
			PRINT(verbose,
			"ERROR: %s: Failed to download tone buffer (%d)!\n",
						ec->name, i);
			return EINVAL;
		}
		Oct6100BufferPlayoutLoadDef( &BufferLoad );
		BufferLoad.pulBufferIndex = &ec->pToneBufferIndexes[i];
		/* FIXME: Can be alaw/mulaw */
		BufferLoad.ulBufferPcmLaw = (tdmv_law == WAN_TDMV_MULAW) ?
							cOCT6100_PCM_U_LAW :
							cOCT6100_PCM_A_LAW;	
		BufferLoad.pbyBufferPattern = pbyBufferData;
		BufferLoad.ulBufferSize = ulBufferSize;
		ulResult = Oct6100BufferPlayoutLoad (
						ec->pChipInstance,
						&BufferLoad);
		if ( ulResult != cOCT6100_ERR_OK ){
			PRINT(verbose,
			"%s: Loading Tones buffers into OCT6100 Chip ...\tFailed (err=0x%X,i=%d)\n",
							ec->name, ulResult, i);
			return EINVAL;
		}
		free( pbyBufferData );
	}

	return 0;
}

static int wan_ecd_BufferPlayoutUnload(wan_ecd_t *ec)
{
	tOCT6100_BUFFER_UNLOAD	BufferUnload;
	UINT32			ulResult;
	INT			i;

	PRINT1(verbose, "%s: Unloading Tones buffers into EC chip ...\n",
					ec->name);
	for(i = 0; i < WAN_NUM_PLAYOUT_TONES; i++){

		Oct6100BufferPlayoutUnloadDef( &BufferUnload );
		BufferUnload.ulBufferIndex = ec->pToneBufferIndexes[i];
		ulResult = Oct6100BufferPlayoutUnload (
						ec->pChipInstance,
						&BufferUnload);
		if ( ulResult != cOCT6100_ERR_OK ){
			PRINT(verbose,
			"ERROR: %s: Unloading Tones buffers into EC Chip ...\tFailed(err=0x%X,i=%d)!\n",
					ec->name, (unsigned int)ulResult, i);
			return EINVAL;
		}
	}
	return 0;
}

static int wan_ecd_BufferPlayoutAdd(wan_ecd_t *ec, int channel, int tone_ch)
{
	tOCT6100_BUFFER_PLAYOUT_ADD	BufferAdd;
	UINT32				ulResult;

	PRINT1(verbose, "%s: Add Tone buffer (%c) to channel %d...\n",
					ec->name,
					tone_ch,
					channel);
	Oct6100BufferPlayoutAddDef( &BufferAdd );
	BufferAdd.fRepeat = TRUE;
	BufferAdd.ulPlayoutPort = cOCT6100_CHANNEL_PORT_ROUT;
	BufferAdd.ulMixingMode = cOCT6100_MIXING_MUTE;
	BufferAdd.ulChannelHndl = ec->pEchoChannelHndl[channel];
	BufferAdd.ulBufferIndex = 
		ec->pToneBufferIndexes[wan_ecd_Char2ToneIndex(tone_ch)];
	ulResult = Oct6100BufferPlayoutAdd(
					ec->pChipInstance,
					&BufferAdd);
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"ERROR: %s: Add Tone buffer (%c) to channel %d...\ttFailed (err=%08X)",
					ec->name,
					tone_ch,
					channel,
					ulResult);
		return EINVAL;
	}
	return 0;
}

static int wan_ecd_BufferPlayoutStart(wan_ecd_t *ec, int channel)
{
	tOCT6100_BUFFER_PLAYOUT_START	BufferStart;
	UINT32				ulResult;

	PRINT1(verbose, "%s: Active playout buffer on channel %d...\n",
					ec->name,
					channel);
	Oct6100BufferPlayoutStartDef( &BufferStart );
	BufferStart.ulChannelHndl = ec->pEchoChannelHndl[channel];
	BufferStart.ulPlayoutPort = cOCT6100_CHANNEL_PORT_ROUT;
	ulResult = Oct6100BufferPlayoutStart(
					ec->pChipInstance,
					&BufferStart);
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"%s: Active playout buffer on channel %d...\tFailed (err=%08X)\n",
					ec->name,
					channel,
					ulResult);
		return EINVAL;
	}
	return 0;
}

static int wan_ecd_BufferPlayoutStop(wan_ecd_t *ec, int channel)
{
	tOCT6100_BUFFER_PLAYOUT_STOP	BufferStop;
	UINT32				ulResult;

	PRINT1(verbose, "%s: Deactive playout buffer on channel %d...\n",
					ec->name,
					channel);
	Oct6100BufferPlayoutStopDef( &BufferStop );
	BufferStop.ulChannelHndl = ec->pEchoChannelHndl[channel];
	BufferStop.ulPlayoutPort = cOCT6100_CHANNEL_PORT_ROUT;
	ulResult = Oct6100BufferPlayoutStop(
					ec->pChipInstance,
					&BufferStop);
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"%s: Deactive playout buffer on channel %d...\tFailed (err=%08X)\n",
					ec->name,
					channel,
					ulResult);
		return EINVAL;
	}
	return 0;
}
#endif

#if defined(ENABLE_CONF_BRIDGE)
static int wan_ecd_ConfBridgeOpen(wan_ecd_t *ec)
{

}

static int wan_ecd_ConfBridgeClose(wan_ecd_t *ec, UINT32 )
{

}

#endif

#if defined(ENABLE_PRODBIST)
static int
wan_ecd_ProductionBist(wan_ecd_t *ec)
{
	tOCT6100_PRODUCTION_BIST	f_ProductionBist;
	UINT32				ulResult;

	PRINT(verbose, "%s: Reading Production Bist result...\n",
				ec->name);
	Oct6100ProductionBistDef( &f_ProductionBist );
	ulResult = Oct6100ProductionBist(
				ec->pChipInstance,
				&f_ProductionBist);
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"ERROR: %s: Failed to read Production Bist result (%08X)!\n",
						ec->name, ulResult);
		return EINVAL;
	}
	if (f_ProductionBist.ulBistStatus == cOCT6100_BIST_IN_PROGRESS){
		PRINT(verbose,
		"%s: The Bist is in progress (Loop=%08X, Addr=%08X)!\n",
				ec->name,
				f_ProductionBist.ulCurrentLoop,
				f_ProductionBist.ulCurrentAddress);
		return EAGAIN;
	}
	if (f_ProductionBist.ulBistStatus == cOCT6100_BIST_CONFIGURATION_FAILED){
		PRINT(verbose,
		"%s: The initial configuration of the internal processors failed!\n",
				ec->name);
	}else if (f_ProductionBist.ulBistStatus == cOCT6100_BIST_STATUS_CRC_FAILED){
		PRINT(verbose,
		"%s: CRC didn't match the computed value!!\n",
				ec->name);
	}else if (f_ProductionBist.ulBistStatus == cOCT6100_BIST_MEMORY_FAILED){
		PRINT(verbose,
		"%s: The external memory Bist failed at location %08X value %08X (%08X)!\n",
				ec->name, 
				f_ProductionBist.ulFailedAddress,
				f_ProductionBist.ulReadValue,
				f_ProductionBist.ulExpectedValue);
	}else if (f_ProductionBist.ulBistStatus == cOCT6100_BIST_SUCCESS){
		PRINT(verbose,
		"%s: The Bist completed successfully!\n",
				ec->name); 
	}

	return 0;
}
#endif

/* Oct6100 image path */
static struct wan_oct6100_image *wan_ecd_image_search(int hwec_chan_no)
{
	struct wan_oct6100_image *image_info = &wan_oct6100_image_list[0];

	while(image_info && image_info->hwec_chan_no){
		if (image_info->hwec_chan_no == hwec_chan_no){
			return image_info;
		}
		image_info++;
	}	
	return NULL;
}

//static int wan_ecd_ChipOpen(wan_ecd_t *ec, char *devname, char *ifname)
static int
wan_ecd_ChipOpen(wan_ec_dev_t *ec_dev, wan_ec_msg_t *ec_msg, int tdmv_law)
{
	tOCT6100_CHIP_OPEN		f_OpenChip;
	tOCT6100_GET_INSTANCE_SIZE	InstanceSize;
	wan_ecd_t			*ec;
	char				*devname, *ifname;
	UINT32				ulImageByteSize;
	PUINT8				pbyImageData = NULL;
	UINT32				ulResult;
	INT				ec_chan = 0;
	struct wan_oct6100_image	*image_info;
	
	ec	= ec_dev->ec;
	devname = ec_dev->devname;
	ifname	= ec_dev->if_name;
	if (ec == NULL || devname == NULL || ifname == NULL){
		PRINT(verbose,
		"ERROR: Internal error (%s:%d)!\n",
					__FUNCTION__,__LINE__);
		return EINVAL;
	} 
	image_info = wan_ecd_image_search(ec->max_channels);
	if (image_info == NULL){
		PRINT(verbose,
		"ERROR: %s: Failed to find image file to EC chip (max=%d)!\n",
					ec->name,
					ec->max_channels);
		return EINVAL;
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
	f_OpenChip.ulMaxChannels	= image_info->max_channels;

#if defined(ENABLE_TONE_PLAY)
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
	f_OpenChip.ulMemoryChipSize	= image_info->memory_chip_size;

	f_OpenChip.fEnableChannelRecording	= TRUE;

#if defined(ENABLE_ACOUSTICECHO)
	f_OpenChip.fEnableAcousticEcho		= TRUE;
#endif

#if defined(ENABLE_PRODBIST)
	/* Enable production bist mode */
	f_OpenChip.fEnableProductionBist	= TRUE;
	f_OpenChip.ulNumProductionBistLoops	= 0x1;
#endif
	
	/*==============================================================*/
	/* Load the image file */
	ulResult = LibLoadImageFile(	ec,
					/*cOCT6116_32S_IMAGE_PATH,
					**cOCT6100_IMAGE_PATH,*/
					image_info->image_path,
					&pbyImageData,
					&ulImageByteSize);
	if ( ulResult ){
		PRINT(verbose,
		"ERROR: %s: Failed to download image to EC chip (err=0x%X)!\n",
					ec->name, ulResult);
		return EINVAL;
	}
	PRINT(verbose, "%s: Loading %s image to EC module...\n",
					ec->name,
					image_info->image);
	f_OpenChip.pbyImageFile = pbyImageData;
	f_OpenChip.ulImageSize  = ulImageByteSize;

	/* Assign evaluation board type. */
	ec->f_Context.ulBoardType = 0x00;//cOCTPCIDRV_BOARD_TYPE_T012;
	
	/* Assign board index (0). */
 	ec->f_Context.ulBoardId = ec->chip_no;
	
	/* We are the main process. */
	ec->f_Context.fMainProcess = 1;

	/* Handle to driver */
	ec->f_Context.hDriver = ec->dev;

	/* Interface name to driver */
	strlcpy(ec->f_Context.devname, devname, WAN_DRVNAME_SZ);
	strlcpy(ec->f_Context.ifname, ifname, WAN_IFNAME_SZ);

	ulResult = Oct6100GetInstanceSize(&f_OpenChip, &InstanceSize);
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"ERROR: %s: Failed to get EC chip instance size (err=0x%X)!\n",
					ec->name, ulResult);
		return EINVAL;
	}

	/* Allocate memory needed for API instance. */
	ec->pChipInstance = 
		(tPOCT6100_INSTANCE_API)malloc(InstanceSize.ulApiInstanceSize);
	if (ec->pChipInstance == NULL){
		PRINT(verbose,
		"ERROR: %s: Failed to allocate memory for EC chip!\n",
					ec->name);
		return EINVAL;
	}
	ec->ulApiInstanceSize = InstanceSize.ulApiInstanceSize;

	/* Open the OCT6100 on the evaluation board. */
	f_OpenChip.pProcessContext = (PVOID)&ec->f_Context;

	/* parse advianced params */
	if (ec_msg->param_no){
		wan_ecd_ChipParam(ec, &f_OpenChip, ec_msg);
	}

	PRINT1(verbose, "%s: Opening Echo Canceller Chip ...",
					ec->name);
	ulResult = Oct6100ChipOpen( 
			ec->pChipInstance,	/* API instance memory. */
			&f_OpenChip );		/* Open chip structure. */
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"ERROR: %s: Failed to open Echo Canceller Chip (err=0x%X)\n",
					ec->name, ulResult);
		return EINVAL;
	}
	free( f_OpenChip.pbyImageFile );

	if (wan_ecd_ChipStats(ec, NULL, TRUE)){
		wan_ecd_ChipClose(ec);
		return EINVAL;
	}

	ec->pEchoChannelHndl = 
			malloc(sizeof(UINT32) * ec->max_channels);
	if (ec->pEchoChannelHndl == NULL){
		PRINT(verbose,
		"ERROR: %s: Failed allocate memory for channel handle!\n",
					ec->name);
		wan_ecd_ChipClose(ec);
		return EINVAL;
	}
	ec->pEcDevMap = malloc(sizeof(wan_ec_dev_t*) * ec->max_channels);
	if (ec->pEcDevMap == NULL){
		PRINT(verbose,
		"ERROR: %s: Failed allocate memory for ec channel map!\n",
					ec->name);
		free(ec->pEchoChannelHndl);
		wan_ecd_ChipClose(ec);
		return EINVAL;
	}
	for(ec_chan = 0; ec_chan < ec->max_channels; ec_chan++){
		ec->pEchoChannelHndl[ec_chan] = cOCT6100_INVALID_HANDLE;
		ec->pEcDevMap[ec_chan] = NULL;
	}


	ec->ulDebugChannelHndl	= cOCT6100_INVALID_HANDLE;
	ec->ulDebugDataMode	= image_info->debug_data_mode;

#if defined(ENABLE_TONE_PLAY)
	if (wan_ecd_BufferPlayoutLoad(ec, tdmv_law)){
		free(ec->pEcDevMap);
		free(ec->pEchoChannelHndl);
		wan_ecd_ChipClose(ec);
		return EINVAL;
	}
#endif

	return 0;
}


static int wan_ecd_EventTone(wan_ecd_t *ec)
{
	tOCT6100_EVENT_GET_TONE	f_GetToneEvent;
	tOCT6100_TONE_EVENT	ToneEvent[32];
	UINT32			ulResult, ulToneEventCount = 0;
	INT			i;
	wan_ec_dev_t		*ec_dev;
	wan_ec_api_t		ec_api;
	int			err,ec_chan,fe_chan;

	PRINT1(verbose, "%s: Getting Tone events ...",
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
		PRINT(verbose, "ERROR: %s: Failed to get tone events (err=0x%X)!\n",
						ec->name, ulResult);
		return EINVAL;
	}

	for(i = 0; i < f_GetToneEvent.ulNumValidToneEvent; i++){
		ec_chan = wan_ecd_hndl2ec_channel(ec, ToneEvent[i].ulChannelHndl);
		ec_dev = ec->pEcDevMap[ec_chan];
		fe_chan = wan_ecd_ec2fe_channel(ec, ec_chan, &ec_dev);
		if (ec_dev == NULL){
			PRINT(verbose,
			"%s: Internal Error: Failed to find fe channel (ec_chan=%d)\n",
						ec->name, ec_chan);
			continue;
		}
		PRINT1(verbose, "%s:%s: Tone event %d %s %s on fe_chan=%d ec_chan=%d\n", 
			ec->name, ec_dev->if_name,
			ulToneEventCount++,
			wan_ecd_ToneId2Str(ToneEvent[i].ulToneDetected),
			wan_ecd_ToneType2Str(f_GetToneEvent.pToneEvent[i].ulEventType),
			fe_chan, ec_chan);
	
		memset(&ec_api, 0, sizeof(ec_api));
		strlcpy(ec_api.if_name, ec_dev->if_name, WAN_DRVNAME_SZ);
		strlcpy(ec_api.devname, ec_dev->devname, WAN_IFNAME_SZ);
		ec_api.cmd = WAN_OCT6100_CMD_TONE_DETECTION; 
		ec_api.u_tone.fe_chan = fe_chan;
		ec_api.u_tone.type = wan_ecd_ConvertToneType(ToneEvent[i].ulEventType);
		ec_api.u_tone.id = wan_ecd_ConvertToneId(ToneEvent[i].ulToneDetected);
		err = wan_ecd_dev_ioctl(ec->dev, &ec_api);	
		if (err){
			PRINT(verbose,
			"ERROR: %s: Failed to send tone event for channel %d!\n",
				ec_dev->if_name, fe_chan);
			return EINVAL;
		}
	}

	return 0;
}


static int wan_ecd_ISR(wan_ecd_t *ec)
{
	tOCT6100_INTERRUPT_FLAGS	f_InterruptFlag;
	UINT32				ulResult;

	Oct6100InterruptServiceRoutineDef(&f_InterruptFlag);

	ulResult = Oct6100InterruptServiceRoutine( 
					ec->pChipInstance,
					&f_InterruptFlag );
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"ERROR: %s: Failed to execute interrupt Service Routine (err=%08X)!\n",
					ec->name,
					ulResult); 
		return EINVAL;
	}
	if (f_InterruptFlag.fFatalGeneral == TRUE){
		PRINT(verbose,
		"%s: An internal fatal chip error detected (0x%X)!\n",
				ec->name,
				f_InterruptFlag.ulFatalGeneralFlags);
	}
	if (f_InterruptFlag.fFatalReadTimeout == TRUE){
		PRINT(verbose,
		"%s: A read to the external memory has failed!\n",
				ec->name);
	}
	if (f_InterruptFlag.fErrorRefreshTooLate == TRUE){
		PRINT(verbose,
		"%s: Error Refresh Too Late!\n",
				ec->name);
	}
	if (f_InterruptFlag.fErrorPllJitter == TRUE){
		PRINT(verbose,
		"%s: Error Pll Jitter\n",
				ec->name);
	}
	if (f_InterruptFlag.fErrorOverflowToneEvents == TRUE){
		PRINT(verbose,
		"%s: Error Overflow Tone Events\n",
				ec->name);
	}
	if (f_InterruptFlag.fErrorH100OutOfSync == TRUE){
		PRINT(verbose,
		"%s: The H100 slave has lost its framing on the bus!\n",
				ec->name);
	}
	if (f_InterruptFlag.fErrorH100ClkA == TRUE){
		PRINT(verbose,
		"%s: The CT_C8_A clock behavior does not conform to the H.100 spec!\n",
				ec->name);
	}
	if (f_InterruptFlag.fErrorH100FrameA == TRUE){
		PRINT(verbose,
		"%s: The CT_FRAME_A clock behavior does not comform to the H.100 spec!\n",
				ec->name);
	}
	if (f_InterruptFlag.fErrorH100ClkB == TRUE){
		PRINT(verbose,
		"%s: The CT_C8_B clock is not running a 16.384 MHz!\n",
				ec->name);
	}
	if (f_InterruptFlag.fToneEventsPending == TRUE){
		PRINT(verbose, "Tone Event pending....\n");
		wan_ecd_EventTone(ec);
	}
	if (f_InterruptFlag.fBufferPlayoutEventsPending == TRUE){
		PRINT1(verbose,
		"%s: Buffer Playout Events Pending\n",
				ec->name);
	}
	if (f_InterruptFlag.fApiSynch == TRUE){
		PRINT1(verbose,
		"%s: The chip interrupted the API for purpose of maintaining sync!\n",
				ec->name);
	}
	return 0;
}

static int wan_ecd_ChipClose(wan_ecd_t *ec)
{
	tOCT6100_CHIP_CLOSE	f_CloseChip;
	UINT32			ulResult;
	
#if defined(ENABLE_TONE_PLAY)
	wan_ecd_BufferPlayoutUnload(ec);
#endif

	Oct6100ChipCloseDef( &f_CloseChip );
	PRINT1(verbose, "%s: Closing Echo Canceller Chip ...\n",
				ec->name);
	ulResult = Oct6100ChipClose( 
				ec->pChipInstance,
				&f_CloseChip );
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"ERROR: %s: Failed to close Echo Canceller chip (err=0x%X)!\n",
						ec->name, ulResult); 
		return EINVAL;
	}
	return 0;
}

static int wan_ecd_ChannelOpen(wan_ecd_t *ec, wan_ec_msg_t *ec_msg, int tdmv_law)
{
	tOCT6100_CHANNEL_OPEN	EchoChannelOpen;
	UINT32			ulResult;
	UINT32			stream = 0,timeslot=0;
	INT			channel, pcm_law_type;

	pcm_law_type = (tdmv_law == WAN_TDMV_MULAW) ?
				cOCT6100_PCM_U_LAW : cOCT6100_PCM_A_LAW;	
	PRINT1(verbose, "%s: Openning all Echo Canceller channels (%s)...\n",
				ec->name,
				(pcm_law_type == cOCT6100_PCM_U_LAW) ?
						"MULAW":"ALAW");

	for(channel = 0; channel < ec->max_channels; channel++){
		Oct6100ChannelOpenDef( &EchoChannelOpen );

		/* Assign the handle memory.*/
		EchoChannelOpen.pulChannelHndl = &ec->pEchoChannelHndl[channel];

		/* Make sure the channel does not perform echo cancellation */
		EchoChannelOpen.ulEchoOperationMode =
					cOCT6100_ECHO_OP_MODE_POWER_DOWN;

		EchoChannelOpen.fEnableToneDisabler = TRUE;

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

		EchoChannelOpen.VqeConfig.ulComfortNoiseMode	= 
					cOCT6100_COMFORT_NOISE_NORMAL;
				/*	cOCT6100_COMFORT_NOISE_NORMAL
				**	cOCT6100_COMFORT_NOISE_EXTENDED,
				**	cOCT6100_COMFORT_NOISE_OFF,
				**	cOCT6100_COMFORT_NOISE_FAST_LATCH */

		PRINT1(verbose, "%s: Openning Echo Canceller channel %d (%s)...\n",
				ec->name,
				channel,
				(pcm_law_type == cOCT6100_PCM_U_LAW) ?
						"MULAW":"ALAW");
		/* Open the channel.*/
		ulResult = Oct6100ChannelOpen(	ec->pChipInstance,
						&EchoChannelOpen );
		if (ulResult != cOCT6100_ERR_OK){
			PRINT(verbose,
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

static int wan_ecd_ChannelClose(wan_ecd_t *ec)
{
	tOCT6100_CHANNEL_CLOSE	EchoChannelClose;
	UINT32			ulResult, channel;

	PRINT1(verbose, "%s: Closing all Echo Canceller channels ...\n",
					ec->name);
	for(channel = 0; channel < ec->max_channels; channel++){
		Oct6100ChannelCloseDef( &EchoChannelClose );
		EchoChannelClose.ulChannelHndl =
					ec->pEchoChannelHndl[channel];
		ulResult = Oct6100ChannelClose(	ec->pChipInstance,
						&EchoChannelClose );
		if (ulResult != cOCT6100_ERR_OK){
			PRINT(verbose,
			"ERROR: %s: Failed to close Echo Canceller channel %d (err=0x%X)!\n",
							ec->name,
							channel,
							ulResult);
			return EINVAL;
		}
		ec->pEchoChannelHndl[channel] = cOCT6100_INVALID_HANDLE;
	}
	return 0;
}

static int
wan_ecd_ChannelStats(wan_ecd_t *ec, INT channel, wan_ec_msg_t *ec_msg, BOOL reset)
{
	tOCT6100_CHANNEL_STATS	f_ChannelStats;
	UINT32			ulResult;

	PRINT1(verbose, "%s: Reading EC statistics for channel %d...\n",
				ec->name, channel);
	Oct6100ChannelGetStatsDef( &f_ChannelStats );
	f_ChannelStats.ulChannelHndl = ec->pEchoChannelHndl[channel];
	f_ChannelStats.fResetStats = reset;
	ulResult = Oct6100ChannelGetStats(
				ec->pChipInstance,
				&f_ChannelStats);
	if ( ulResult != cOCT6100_ERR_OK ){
		PRINT(verbose,
		"ERROR: %s: Failed to read EC stats for channel %d (%08X)!\n",
						ec->name, channel, ulResult);
		return EINVAL;
	}
	if (ec_msg){
		memcpy(	&ec_msg->msg_chan_stat.f_ChannelStats,
			&f_ChannelStats,
			sizeof(tOCT6100_CHANNEL_STATS));
	}
	return 0;
}

static int
wan_ecd_ChannelModify(wan_ecd_t *ec, INT channel, UINT32 mode, wan_ec_msg_t *ec_msg)
{
	tOCT6100_CHANNEL_MODIFY		EchoChannelModify;
	UINT32				ulResult;

	Oct6100ChannelModifyDef( &EchoChannelModify );

	/* Assign the handle memory.*/
	EchoChannelModify.ulChannelHndl = ec->pEchoChannelHndl[channel];

	/* Enable echo cancellation */
	EchoChannelModify.ulEchoOperationMode = mode;

	/* parse advianced params */
	if (ec_msg->param_no){
		int err;
		err = wan_ecd_ChanParam(ec, &EchoChannelModify, ec_msg);
		if (err){
			PRINT(verbose,
			"%s: WARNING: Unsupported parameter for channel %d!\n",
				ec->name,
				channel);
		}
	}else if (mode == cOCT6100_KEEP_PREVIOUS_SETTING){
		wan_ecd_ChanParamList(ec);
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
			PRINT(verbose,
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

static int
wan_ecd_DebugChannel(wan_ecd_t *ec, INT channel)
{
	tOCT6100_DEBUG_SELECT_CHANNEL	DebugSelectChannel;
	UINT32				ulResult;

	if (ec->ulDebugChannelHndl != cOCT6100_INVALID_HANDLE){
		PRINT(verbose,
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
		PRINT(verbose,
		"ERROR: %s: Failed to select debug channel %d for monitoring (err=0x%X)\n",
				ec->name,
				channel,
				ulResult);
		return EINVAL;
	}
	return	0;
}

#if 1
static int
wan_ecd_DebugGetData(wan_ecd_t *ec, wan_ec_msg_t *ec_msg)
{
	tOCT6100_DEBUG_GET_DATA	fDebugGetData;
	UINT32			ulResult;

	if (ec->ulDebugChannelHndl == cOCT6100_INVALID_HANDLE){
		PRINT(verbose,
		"ERROR: %s: No Debug channel was selected!\n",
					ec->name);
		return EINVAL;
	}
		
	Oct6100DebugGetDataDef( &fDebugGetData );

	PRINT1(verbose, "%s: Retrieves debug data for channel %d...\n",
			ec->name,
			ec->DebugChannel);

	memset(&ec_msg->msg_chan_monitor.data[0], 0,
				sizeof(UINT8) * (MAX_MONITOR_DATA_LEN+1));
	/* Set selected debug channel */
	fDebugGetData.ulGetDataMode	= ec->ulDebugDataMode;
	fDebugGetData.ulMaxBytes	= ec_msg->msg_chan_monitor.max_len;
	fDebugGetData.pbyData		= &ec_msg->msg_chan_monitor.data[0];

	/* Select Debug channel */
	ulResult = Oct6100DebugGetData( 
			ec->pChipInstance,
			&fDebugGetData );
	if (ulResult != cOCT6100_ERR_OK){
		PRINT(verbose,
		"ERROR: %s: Failed to get debug data for channel %d (err=0x%X)\n",
				ec->name,
				ec->DebugChannel,
				ulResult);
		return EINVAL;
	}
	ec_msg->msg_chan_monitor.data_len	= fDebugGetData.ulValidNumBytes;
	ec_msg->msg_chan_monitor.remain_len	= fDebugGetData.ulRemainingNumBytes;
	ec_msg->msg_event.channel		= ec->DebugChannel;
	
	if (fDebugGetData.ulRemainingNumBytes == 0){
		/* Last read */
		ec->ulDebugChannelHndl = cOCT6100_INVALID_HANDLE;
	}

	return	0;
}

#else
#define MAX_DEBUG_DATA_LEN	1024
static int
wan_ecd_DebugGetData(wan_ecd_t *ec)
{
	tOCT6100_DEBUG_GET_DATA	fDebugGetData;
	UINT32			ulResult;
	size_t			len;
	UINT8			*pData;
	unsigned char		filename[MAXPATHLEN];
	FILE			*output;
	struct timeval		tv;
	struct tm		t;

pid_t pid;
pid=fork();
if (pid < 0){
PRINT(verbose,
"ERROR: %s: Failed to run fork!\n",
			ec->name);
return -EINVAL;
} else if (pid > 0){
return 0;
}

	pData = malloc(sizeof(UINT8) * (MAX_DEBUG_DATA_LEN+1));
	if (pData == NULL){
		PRINT(verbose,
		"ERROR: %s: Failed to allocate memory for debug channel %d!\n",
					ec->name,
					ec->DebugChannel);
		return EINVAL;
	}

	if (ec->ulDebugChannelHndl == cOCT6100_INVALID_HANDLE){
		PRINT(verbose,
		"ERROR: %s: No Debug channel was selected!\n",
					ec->name);
		free(pData);
exit(0);
		return EINVAL;
	}
		
	memset(filename, 0, MAXPATHLEN);
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &t);
	snprintf(filename, MAXPATHLEN, "%s_chan%d_%d.%d.%d_%d.%d.%d.bin",
					WAN_EC_DEBUG,
					ec->DebugChannel,
					t.tm_mon,t.tm_mday,1900+t.tm_year,
					t.tm_hour,t.tm_min,t.tm_sec);
	output = fopen(filename, "wb");
	if (output == NULL){
		PRINT(verbose,
		"ERROR: %s: Failed to open binary file %s!\n",
					ec->name,
					filename);
		free(pData);
exit(0);
		return EINVAL;
	}
	
	Oct6100DebugGetDataDef( &fDebugGetData );

	PRINT1(verbose, "%s: Retrieves debug data for channel %d...\n",
			ec->name,
			ec->DebugChannel);
	do {
		memset(pData, 0, sizeof(UINT8) * (MAX_DEBUG_DATA_LEN+1));
		/* Set selected debug channel */
		fDebugGetData.ulGetDataMode	= ec->ulDebugDataMode;
		fDebugGetData.ulMaxBytes	= MAX_DEBUG_DATA_LEN;
		fDebugGetData.pbyData		= pData;

		/* Select Debug channel */
		ulResult = Oct6100DebugGetData( 
				ec->pChipInstance,
				&fDebugGetData );
		if (ulResult != cOCT6100_ERR_OK){
			PRINT(verbose,
			"ERROR: %s: Failed to get debug data for channel %d (err=0x%X)\n",
					ec->name,
					ec->DebugChannel,
					ulResult);
exit(0);
			return EINVAL;
		}
		len = fwrite(fDebugGetData.pbyData, sizeof(UINT8), fDebugGetData.ulValidNumBytes, output);
		if (len != fDebugGetData.ulValidNumBytes){
			PRINT(verbose,
			"ERROR: %s: Failed to write data to binary file %s!\n",
					ec->name,
					filename);
			break;
		}
	}while(fDebugGetData.ulRemainingNumBytes);

	if (pData){
		free(pData);
	}
	fclose(output);
exit(0);
	return	0;
}
#endif

static int wan_ecd_TonesEnable(wan_ecd_t *ec, int channel)
{
	tOCT6100_TONE_DETECTION_ENABLE	f_ToneDetectionEnable;
	UINT32				ulResult;
	int				i;

	PRINT1(verbose, "%s: Enable tone detection on channel %d ...",
					ec->name,
					channel);
	for(i = 0; i < WAN_NUM_DTMF_TONES; i++){
	
		Oct6100ToneDetectionEnableDef( &f_ToneDetectionEnable );
		f_ToneDetectionEnable.ulChannelHndl = 
					ec->pEchoChannelHndl[channel];
		f_ToneDetectionEnable.ulToneNumber =
					DetectedToneNumbers[i];
		ulResult = Oct6100ToneDetectionEnable (
						ec->pChipInstance,
						&f_ToneDetectionEnable);
		if ( ulResult != cOCT6100_ERR_OK ){
			PRINT(verbose,
			"ERROR: %s: Failed to enable tone detection on channel %d (err=0x%X,i=%d)!\n",
					ec->name,
					channel,
					(unsigned int)ulResult, i);
			return EINVAL;
		}
	}
	return 0;
}

static int wan_ecd_TonesDisable(wan_ecd_t *ec, int channel)
{
	tOCT6100_TONE_DETECTION_DISABLE	f_ToneDetectionDisable;
	UINT32				ulResult;
	INT				i;

	PRINT1(verbose, "%s: Disable tone detection on channel %d ...",
					ec->name,
					channel);
	for(i = 0; i < WAN_NUM_DTMF_TONES; i++){
	
		Oct6100ToneDetectionDisableDef( &f_ToneDetectionDisable );
		f_ToneDetectionDisable.ulChannelHndl = 
					ec->pEchoChannelHndl[channel];
		if (channel >= 0){
			f_ToneDetectionDisable.ulToneNumber =
						DetectedToneNumbers[i];
		}else{
			f_ToneDetectionDisable.fDisableAll = TRUE;
		}
		ulResult = Oct6100ToneDetectionDisable (
						ec->pChipInstance,
						&f_ToneDetectionDisable);
		if ( ulResult != cOCT6100_ERR_OK ){
			PRINT(verbose,
			"ERROR: %s: Failed to disable tone detection for channel %d (err=0x%X,i=%d)!\n", 
					ec->name, channel,
					(unsigned int)ulResult, i);
			return EINVAL;
		}
	}
	return 0;
}

static int
wan_ecd_reset(wan_ecd_t *ec, wan_ec_dev_t *ec_dev, int reset)
{
	wan_ec_api_t	ec_api;
	int		err;

	PRINT1(verbose, "%s: %s Echo Canceller chip reset (%s)!\n",
				ec_dev->if_name, 
				(reset) ? "Set" : "Clear",
				ec_dev->devname);
	memset(&ec_api, 0, sizeof(ec_api));
	strlcpy(ec_api.devname, ec_dev->devname, WAN_DRVNAME_SZ);
	strlcpy(ec_api.if_name, ec_dev->if_name, WAN_IFNAME_SZ);
	ec_api.cmd = (reset) ? 
				WAN_OCT6100_CMD_SET_RESET :
				WAN_OCT6100_CMD_CLEAR_RESET;
	err = wan_ecd_dev_ioctl(ec->dev, &ec_api);	
	if (err){
		PRINT(verbose,
		"ERROR: %s: Failed to %s reset for Echo Canceller chip!\n",
				ec_dev->if_name,
				(reset) ? "set" : "clear");
		return EINVAL;
	}

	ec->state = (reset) ? 
			WAN_OCT6100_STATE_RESET :
			WAN_OCT6100_STATE_READY;
	return 0;
}

static int
wan_ecd_bypass(wan_ec_dev_t *ec_dev, int fe_channel, int enable)
{
	wan_ecd_t	*ec = ec_dev->ec;
	wan_ec_api_t	ec_api;
	int		err;

	PRINT1(verbose, "%s: %s bypass mode for channel %d!\n",
				ec_dev->if_name,
				(enable) ? "Enable" : "Disable",
				fe_channel);
	memset(&ec_api, 0, sizeof(ec_api));
	strlcpy(ec_api.if_name, ec_dev->if_name, WAN_DRVNAME_SZ);
	strlcpy(ec_api.devname, ec_dev->devname, WAN_IFNAME_SZ);
	ec_api.u_ecd.channel = fe_channel;
	ec_api.cmd = (enable) ? 
				WAN_OCT6100_CMD_BYPASS_ENABLE :
				WAN_OCT6100_CMD_BYPASS_DISABLE;
	err = wan_ecd_dev_ioctl(ec->dev, &ec_api);	
	if (err){
		PRINT(verbose,
		"ERROR: %s: Failed to %s bypass mode for channel %d!\n",
				ec_dev->if_name,
				(enable) ? "enable" : "disable",
				fe_channel);
		return EINVAL;
	}

	return 0;
}

wan_ec_dev_t *wan_ecd_search(wan_ec_msg_t *ec_msg)
{
	wan_ecd_t	*ec;
	wan_ec_dev_t	*ec_dev = NULL;

	WAN_LIST_FOREACH(ec, &wan_ecd_head, next){
		WAN_LIST_FOREACH(ec_dev, &ec->ec_dev_head, next){
			int	cmp1 = 0, cmp2 = 0;
			cmp1 = strcmp(ec_dev->devname, ec_msg->devname);
#if 0
			/* allow to select any interface (devname is enough) */
			cmp2 = strcmp(ec_dev->if_name, ec_msg->if_name);
#endif
			if (cmp1 == 0 && cmp2 == 0){
				return ec_dev;
			}else if (cmp1 && cmp2){
				continue;
			}else{
				continue;
			}
		}
	}
	PRINT2(verbose, "%s: Searching for device...\tFailed!\n",
				ec_msg->devname);
	return NULL;
}

static wan_ec_dev_t *wan_ecd_open(char *if_name, char *devname)
{
	wan_ecd_t	*ec;
	wan_ec_dev_t 	*ec_dev;
	wan_ec_api_t	ec_api;
	int		dev, err;

	dev = wan_ecd_dev_open(devname);
	if (dev < 0){
		PRINT(verbose,
		"ERROR: %s: Failed to open Echo Canceller API device!\n",
			devname);
		return NULL;
	}

	PRINT2(verbose, "%s: Opening API device (%d) to the driver!\n",
				devname, dev);

	memset(&ec_api, 0, sizeof(ec_api));
	strlcpy(ec_api.devname, devname, WAN_IFNAME_SZ);
	strlcpy(ec_api.if_name, if_name, WAN_DRVNAME_SZ);
	ec_api.cmd = WAN_OCT6100_CMD_GET_INFO;
	err = wan_ecd_dev_ioctl(dev, &ec_api);	
	if (err < 0){
		PRINT(verbose,
		"ERROR: %s: Failed to get device info (%d,%s)!\n",
				if_name,errno,strerror(errno));
		wan_ecd_dev_close(dev);
		return NULL;
	}

	WAN_LIST_FOREACH(ec, &wan_ecd_head, next){
		if (strcmp(ec->name, ec_api.name) == 0){
			break;
		}
	}

	if (ec == NULL){
		PRINT2(verbose,
		"%s: Creating new Echo Canceller device %s...\n",
					if_name,
					ec_api.name);
		ec = malloc(sizeof(wan_ecd_t));
		if (ec == NULL){
			PRINT(verbose,
			"ERROR: %s: Failed to allocate memory for new EC device!\n",
						ec_api.name);
			wan_ecd_dev_close(dev);
			return NULL;
		}
		memset(ec, 0, sizeof(wan_ecd_t));
		memcpy(ec->name, ec_api.name, strlen(ec_api.name));
		ec->dev			= dev;

		ec->usage		= 0;
		ec->state		= ec_api.u_ecd.state;
		ec->chip_no		= ec_api.u_ecd.chip_no;
		ec->max_channels	= ec_api.u_ecd.max_channels;
		ec->ulApiInstanceSize	= ec_api.u_ecd.ulApiInstanceSize;
		WAN_LIST_INIT(&ec->ec_dev_head);
		WAN_LIST_INSERT_HEAD(&wan_ecd_head, ec, next);
	}

	PRINT2(verbose,
	"%s: Register new Echo Canceller API device to %s...\n",
					if_name,
					ec->name);
	ec_dev = malloc(sizeof(wan_ec_dev_t));
	if (ec_dev == NULL){
		PRINT(verbose,
		"ERROR: %s: Failed to allocate memory for new EC device!\n",
						if_name);
		return NULL;
	}
	memset(ec_dev, 0, sizeof(wan_ec_dev_t));
	memcpy(ec_dev->if_name, if_name, strlen(if_name));
	memcpy(ec_dev->devname, devname, strlen(devname));
	ec_dev->dev		= dev;
	ec_dev->fe_media	= ec_api.u_ecd.fe_media;
	ec_dev->fe_lineno	= ec_api.u_ecd.fe_lineno;
	/* Dec 01, 2005 T1:1-24, E1:1-32 (disable EC on chan 0), RM:1-16 */
	ec_dev->fe_max_channels	= ec_api.u_ecd.fe_max_channels;
	ec_dev->fe_tdmv_law	= ec_api.u_ecd.fe_tdmv_law;
	ec_dev->ec		= ec;
	ec->usage++;

	if (WAN_LIST_EMPTY(&ec->ec_dev_head)){
		WAN_LIST_INSERT_HEAD(&ec->ec_dev_head, ec_dev, next);
	}else{
		wan_ec_dev_t 	*ec_dev_tmp;
		ec_dev_tmp = WAN_LIST_FIRST(&ec->ec_dev_head);
		while(WAN_LIST_NEXT(ec_dev_tmp, next)){
			ec_dev_tmp = WAN_LIST_NEXT(ec_dev_tmp, next);
		}
		WAN_LIST_INSERT_AFTER(ec_dev_tmp, ec_dev, next);
	}
	return ec_dev;
}

int wan_ecd_close(wan_ecd_t *ec, wan_ec_dev_t *ec_dev)
{
	int	fe_chan, ec_chan;
	PRINT(verbose, "Closing the socket device %s\n", ec_dev->devname);
	/* EC_DEV_MAP */
	for(fe_chan = 0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
		ec_chan = wan_ecd_fe2ec_channel(ec_dev, fe_chan);
		ec->pEcDevMap[ec_chan] = NULL;
	}
	wan_ecd_dev_close(ec_dev->dev);
	return 0;
}

static int wan_ecd_create(wan_ec_dev_t *ec_dev, wan_ec_msg_t *ec_msg)
{
	wan_ecd_t	*ec = NULL;
	int		err, ec_chan, fe_chan;

	if (ec_dev == NULL){
		ec_dev = wan_ecd_open(ec_msg->if_name, ec_msg->devname);
		if (ec_dev == NULL){
			return EINVAL;
		}
	}
	ec = ec_dev->ec;
	switch(ec->state){
	case WAN_OCT6100_STATE_RESET:
	case WAN_OCT6100_STATE_READY:
		break;
	case WAN_OCT6100_STATE_CHIP_READY:
		PRINT2(verbose, "%s: Echo Canceller %s chip is READY!\n",
						ec_dev->if_name,
						ec->name);
		break;
	default:
		PRINT(verbose,
		"ERROR: %s: Invalid Echo Canceller %s API state (%s)\n",
					ec_dev->if_name,
					ec->name,
					WAN_OCT6100_STATE_DECODE(ec->state));
		return EINVAL;
	}

	if (ec->state == WAN_OCT6100_STATE_RESET){
		err = wan_ecd_reset(ec, ec_dev, 0);
		if (err) return err;
	}

	if (ec->state == WAN_OCT6100_STATE_READY){

		/* Chip open */
		if (wan_ecd_ChipOpen(ec_dev, ec_msg, ec_dev->fe_tdmv_law)){
			wan_ecd_reset(ec, ec_dev, 1);
			return EINVAL;
		}
	
		/* Open all channels */
		if (wan_ecd_ChannelOpen(ec, ec_msg, ec_dev->fe_tdmv_law)){
			wan_ecd_ChipClose(ec);
			wan_ecd_reset(ec, ec_dev, 1);
			return EINVAL;
		}
		ec->state = WAN_OCT6100_STATE_CHIP_READY;
	}
	/* EC_DEV_MAP */
	for(fe_chan=0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
		ec_chan = wan_ecd_fe2ec_channel(ec_dev, fe_chan);
		ec->pEcDevMap[ec_chan] = ec_dev;
	}
	return 0;
}


int wan_ecd_release(wan_ec_dev_t *ec_dev)
{
	wan_ecd_t	*ec = ec_dev->ec;
	int		err = 0;

	switch(ec->state){
	case WAN_OCT6100_STATE_READY:
	case WAN_OCT6100_STATE_CHIP_READY:
		break;
	default:
		PRINT2(verbose, "%s: WARNING: Echo Canceller %s API state (%s)\n",
				ec_dev->if_name,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return 0;
	}

	if (WAN_LIST_FIRST(&ec->ec_dev_head) == ec_dev){
		wan_ec_dev_t	*ec_dev_head;

		WAN_LIST_FIRST(&ec->ec_dev_head) =
				WAN_LIST_NEXT(ec_dev, next);
		ec_dev_head = WAN_LIST_FIRST(&ec->ec_dev_head);
				
		if (ec_dev_head){
			ec->dev			= ec_dev_head->dev;
			ec->f_Context.hDriver	= ec->dev;
			strlcpy(ec->f_Context.devname, ec_dev_head->devname, WAN_DRVNAME_SZ);
			strlcpy(ec->f_Context.ifname, ec_dev_head->if_name, WAN_IFNAME_SZ);
		}
	}else{
		WAN_LIST_REMOVE(ec_dev, next);
	}
	ec_dev->ec = NULL;
	ec->usage--;
	if (ec->usage){
		/* Other device are still connected */
		wan_ecd_close(ec, ec_dev);
		goto wan_oct6100_release_done;
	}
	if (ec->state == WAN_OCT6100_STATE_CHIP_READY){

		if (wan_ecd_ChannelClose(ec)){
			return EINVAL;
		}
		if (wan_ecd_ChipClose(ec)){
			return EINVAL;
		}
		ec->state = WAN_OCT6100_STATE_READY;
		ec->ulApiInstanceSize = 0;
		ec->max_channels = 0;
		if (ec->pChipInstance){
			free(ec->pChipInstance);
			ec->pChipInstance = NULL;
		}
		if (ec->pEchoChannelHndl){
			free(ec->pEchoChannelHndl);
			ec->pEchoChannelHndl = NULL;
		}
	}

	if (ec->state == WAN_OCT6100_STATE_READY){
		err = wan_ecd_reset(ec, ec_dev, 1);
		if (err){
			return EINVAL;
		}
	}

	PRINT2(verbose, "%s: Release Echo Canceller device\n",
				ec->name);
	wan_ecd_close(ec, ec_dev);
	ec->dev = -1;
	if (WAN_LIST_FIRST(&wan_ecd_head) == ec){
		WAN_LIST_FIRST(&wan_ecd_head) =
				WAN_LIST_NEXT(ec, next);
	}else{
		WAN_LIST_REMOVE(ec, next);
	}
	if (ec->pEcDevMap){
		free(ec->pEcDevMap);
		ec->pEcDevMap = NULL;
	}
	free(ec);

wan_oct6100_release_done:
	PRINT2(verbose, "%s: Release Echo Canceller API device\n",
				ec_dev->if_name);
	free(ec_dev);
	return 0;
}

int wan_ecd_release_all(wan_ec_dev_t *ec_dev)
{
	wan_ecd_t	*ec;

	if (ec_dev){
		ec = ec_dev->ec;
		while(ec_dev){
			WAN_LIST_FIRST(&ec->ec_dev_head) = 
					WAN_LIST_NEXT(ec_dev, next);
			wan_ecd_release(ec_dev);
			ec_dev = WAN_LIST_FIRST(&ec->ec_dev_head);
		}	
	}else{
		WAN_LIST_FOREACH(ec, &wan_ecd_head, next){
			ec_dev = WAN_LIST_FIRST(&ec->ec_dev_head);
			while(ec_dev){
				WAN_LIST_FIRST(&ec->ec_dev_head) = 
						WAN_LIST_NEXT(ec_dev, next);
				wan_ecd_release(ec_dev);
				ec_dev = WAN_LIST_FIRST(&ec->ec_dev_head);
			}
		}
	}
	return 0;
}

int wan_ecd_modify(wan_ec_dev_t *ec_dev, wan_ec_msg_t *ec_msg)
{
	wan_ecd_t	*ec = ec_dev->ec;
	wan_ecd_event_t	*event = &ec_msg->msg_event; 
	int		cmd = ec_msg->cmd;
	int		fe_chan = 0, ec_chan = 0; 
	int		err;

	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT(verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->if_name,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return EINVAL;
	}

	if (event->channel_map == 0xFFFFFFFF){
		/* All channels selected */
		event->channel_map = 0l;
		for (fe_chan = 0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
			event->channel_map |= (1 << fe_chan);
		}
	}else{
		if (ec_dev->fe_media == WAN_MEDIA_T1 ||
		    ec_dev->fe_media == WAN_MEDIA_FXOFXS){
			event->channel_map = event->channel_map >> 1;
		}
	}

	/* Enable Echo cancelation on Oct6100 */
	PRINT2(verbose,
	"%s: %s Echo Canceller on channel(s) map=0x%X...\n",
			ec_dev->if_name,
			(cmd == WAN_EC_CMD_ENABLE) ? "Enable" : "Disable",
			event->channel_map);
	//for(chan = fe_first; chan <= fe_last; chan++){
	for(fe_chan=0; fe_chan < ec_dev->fe_max_channels; fe_chan++){
		if (!(event->channel_map & (1 << fe_chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && fe_chan == 0) continue;
		ec_chan = wan_ecd_fe2ec_channel(ec_dev, fe_chan);
		if (cmd == WAN_EC_CMD_ENABLE){
			err = wan_ecd_ChannelModify(
						ec,
						ec_chan,
						cOCT6100_ECHO_OP_MODE_NORMAL,
						ec_msg);
			if (err){
				return EINVAL;
			}	

			/* Change rx/tx traffic through Oct6100 */
			if (wan_ecd_bypass(ec_dev, fe_chan, 1)){
				return EINVAL;
			}
		}else{
			/* Change rx/tx traffic through Oct6100 */
			if (wan_ecd_bypass(ec_dev, fe_chan, 0)){
				return EINVAL;
			}

			err = wan_ecd_ChannelModify(
						ec,
						ec_chan,
						cOCT6100_ECHO_OP_MODE_POWER_DOWN,
						ec_msg);
			if (err){
				return EINVAL;
			}
		}
	}

	return 0;
}

int wan_ecd_modify_mode(wan_ec_dev_t *ec_dev, wan_ec_msg_t *ec_msg)
{
	wan_ecd_t	*ec = ec_dev->ec;
	wan_ecd_event_t	*event = &ec_msg->msg_event;
	int		cmd = ec_msg->cmd;
	int		chan, ec_channel;
	int		err;

	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT(verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->if_name,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return EINVAL;
	}

	if (event->channel_map == 0xFFFFFFFF){
		/* All channels selected */
		event->channel_map = 0l;
		for (chan = 0; chan < ec_dev->fe_max_channels; chan++){
			event->channel_map |= (1 << chan);
		}
	}else{
		if (ec_dev->fe_media == WAN_MEDIA_T1 ||
		    ec_dev->fe_media == WAN_MEDIA_FXOFXS){
			event->channel_map = event->channel_map >> 1;
		}
	}
	/* Enable/Disable Normal mode on Oct6100 */
	PRINT1(verbose,
	"%s: %s Echo Canceller mode on channel(s) map=0x%X ...",
			ec_dev->if_name,
			(cmd == WAN_EC_CMD_MODE_NORMAL) ? "Enable" :
			(cmd == WAN_EC_CMD_MODE_POWERDOWN) ? "Disable" : "Modify",
			event->channel_map);
	for(chan=0; chan < ec_dev->fe_max_channels; chan++){
		if (!(event->channel_map & (1 << chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && chan == 0) continue;
		ec_channel = wan_ecd_fe2ec_channel(ec_dev, chan);
		switch(cmd){
		case WAN_EC_CMD_MODE_NORMAL:
			err = wan_ecd_ChannelModify(
						ec,
						ec_channel,
						cOCT6100_ECHO_OP_MODE_NORMAL,
						ec_msg);
			break;
		case WAN_EC_CMD_MODE_POWERDOWN:
			err = wan_ecd_ChannelModify(
						ec,
						ec_channel,
						cOCT6100_ECHO_OP_MODE_POWER_DOWN,
						ec_msg);
			break;
		default:
			err = wan_ecd_ChannelModify(
						ec,
						ec_channel,
						cOCT6100_KEEP_PREVIOUS_SETTING,
						ec_msg);
			break;
		}
		if (err){
			return EINVAL;
		}	
	}

	return 0;
}

int wan_ecd_modify_bypass(wan_ec_dev_t *ec_dev, wan_ecd_event_t *event, int cmd)
{
	wan_ecd_t	*ec = ec_dev->ec;
	int		chan, fe_channel = 0;

	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT(verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->if_name,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return EINVAL;
	}

	if (event->channel_map == 0xFFFFFFFF){
		/* All channels selected */
		event->channel_map = 0l;
		for (chan = 0; chan < ec_dev->fe_max_channels; chan++){
			event->channel_map |= (1 << chan);
		}
	}else{
		if (ec_dev->fe_media == WAN_MEDIA_T1 ||
		    ec_dev->fe_media == WAN_MEDIA_FXOFXS){
			event->channel_map = event->channel_map >> 1;
		}
	}
	/* Enable/Disable bypass mode on Oct6100 */
	for(chan = 0; chan < ec_dev->fe_max_channels; chan++){
		if (!(event->channel_map & (1 << chan))){
			continue;
		}
		fe_channel = chan;
		if (cmd == WAN_EC_CMD_BYPASS_ENABLE){
			/* Change rx/tx traffic through Oct6100 */
			if (wan_ecd_bypass(ec_dev, fe_channel, 1)){
				return EINVAL;
			}
		}else{
			/* Change rx/tx traffic through Oct6100 */
			if (wan_ecd_bypass(ec_dev, fe_channel, 0)){
				return EINVAL;
			}
		}
	}

	return 0;
}

int wan_ecd_modify_dtmf(wan_ec_dev_t *ec_dev, wan_ec_msg_t *ec_msg)
{
	wan_ecd_t	*ec = ec_dev->ec;
	wan_ecd_event_t	*event = &ec_msg->msg_event;
	int		cmd = ec_msg->cmd;
	int		chan, ec_channel;
	int		err;

	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT(verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->if_name,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return EINVAL;
	}

	if (event->channel_map == 0xFFFFFFFF){
		/* All channels selected */
		event->channel_map = 0l;
		for (chan = 0; chan < ec_dev->fe_max_channels; chan++){
			event->channel_map |= (1 << chan);
		}
	}else{
		if (ec_dev->fe_media == WAN_MEDIA_T1 ||
		    ec_dev->fe_media == WAN_MEDIA_FXOFXS){
			event->channel_map = event->channel_map >> 1;
		}
	}
	/* Enable/Disable Normal mode on Oct6100 */
	PRINT1(verbose,
	"%s: %s Echo Canceller mode on channel(s) map=0x%X ...",
			ec_dev->if_name,
			(cmd == WAN_EC_CMD_DTMF_ENABLE) ? "Enable" :
			(cmd == WAN_EC_CMD_DTMF_DISABLE) ? "Disable" :
								"Unknown",
			event->channel_map);
	for(chan=0; chan < ec_dev->fe_max_channels; chan++){
		if (!(event->channel_map & (1 << chan))){
			continue;
		}
		if (ec_dev->fe_media == WAN_MEDIA_E1 && chan == 0) continue;
		ec_channel = wan_ecd_fe2ec_channel(ec_dev, chan);
		switch(cmd){
		case WAN_EC_CMD_DTMF_ENABLE:
			if (wan_ecd_bypass(ec_dev, chan, 1)){
				return EINVAL;
			}
			err = wan_ecd_TonesEnable(ec, chan);
			break;
		case WAN_EC_CMD_DTMF_DISABLE:
			err = wan_ecd_TonesDisable(ec, chan);
			if (wan_ecd_bypass(ec_dev, chan, 0)){
				return EINVAL;
			}
			break;
		default:
			err = -EINVAL;
			break;
		}
		if (err){
			return EINVAL;
		}	
	}
	return 0;
}


int wan_ecd_stats(wan_ec_dev_t *ec_dev, wan_ec_msg_t *ec_msg)
{
	wan_ecd_t	*ec = NULL;
	int		channel = ec_msg->msg_event.channel,
			reset = (ec_msg->msg_event.reset) ? TRUE : FALSE,
			ec_channel;

	ec = ec_dev->ec;
	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT(verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->if_name,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return EINVAL;
	}

	/* Sanity check from channel selection */
	if (((ec_dev->fe_media == WAN_MEDIA_T1) && (channel > ec_dev->fe_max_channels)) || 
	    ((ec_dev->fe_media == WAN_MEDIA_E1) && (channel >= ec_dev->fe_max_channels))){ 
		PRINT(verbose,
			"ERROR: %s: Channel number %d out of range!\n",
				ec_dev->if_name,
				channel);
			return EINVAL;
	}

	if (wan_ecd_ISR(ec)){
		return EINVAL;
	}
	if (channel){
		if (ec_dev->fe_media == WAN_MEDIA_T1 || ec_dev->fe_media == WAN_MEDIA_FXOFXS) channel--; 
		ec_channel = wan_ecd_fe2ec_channel(ec_dev, channel);
		if (wan_ecd_ChannelStats(ec, ec_channel, ec_msg, reset)){
			return EINVAL;
		}
	}else{
		wan_ecd_ChipStats(ec, ec_msg, reset);
	}
	return 0;
}

int wan_ecd_monitor(wan_ec_dev_t *ec_dev, wan_ec_msg_t *ec_msg)
{
	wan_ecd_t	*ec = NULL;
	int		channel = ec_msg->msg_event.channel,
			ec_channel;

	ec = ec_dev->ec;
	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT(verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->if_name,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return EINVAL;
	}

	/* Sanity check from channel selection */
	if (((ec_dev->fe_media == WAN_MEDIA_T1) && (channel > ec_dev->fe_max_channels)) || 
	    ((ec_dev->fe_media == WAN_MEDIA_E1) && (channel >= ec_dev->fe_max_channels))){ 
		PRINT(verbose,
			"ERROR: %s: Channel number %d out of range!\n",
				ec_dev->if_name,
				channel);
			return EINVAL;
	}

	if (channel){
		if (ec_dev->fe_media == WAN_MEDIA_T1) channel--; 
		ec_channel = wan_ecd_fe2ec_channel(ec_dev, channel);
		if (wan_ecd_DebugChannel(ec, ec_channel)){
			return EINVAL;
		}
	}else{
		wan_ecd_DebugGetData(ec, ec_msg);
	}
	return 0;
}

#if defined(ENABLE_TONE_PLAY)
int wan_ecd_playout_tone(wan_ecd_t *ec, int ec_channel, char tone_ch, int delay)
{

	if (wan_ecd_BufferPlayoutAdd(ec, ec_channel, tone_ch)){
		return EINVAL;
	}
	if (wan_ecd_BufferPlayoutStart(ec, ec_channel)){
		return EINVAL;
	}
	wan_ecd_delay(delay);
	if (wan_ecd_BufferPlayoutStop(ec, ec_channel)){
		return EINVAL;
	}
	return 0;
}

int wan_ecd_playout_test(wan_ecd_t *ec, int channel, int delay)
{

	if (wan_ecd_playout_tone(ec, channel, '0', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '1', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '2', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '3', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '4', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '5', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '6', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '7', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '8', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '9', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, 'a', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, 'b', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, 'c', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, 'd', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '*', delay)){
		return EINVAL;
	}
	if (wan_ecd_playout_tone(ec, channel, '#', delay)){
		return EINVAL;
	}
	return 0;
}

int wan_ecd_playout(wan_ec_dev_t *ec_dev, wan_ecd_event_playout_t *playout)
{
	wan_ecd_t	*ec = NULL;
	int		ec_channel;
	int		err;

	ec = ec_dev->ec;
	if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
		PRINT(verbose,
		"WARNING: %s: Invalid Echo Canceller %s API state (%s)\n",
				ec_dev->if_name,
				ec->name,
				WAN_OCT6100_STATE_DECODE(ec->state));
		return EINVAL;
	}

	if (ec_dev->fe_media == WAN_MEDIA_E1 && playout->channel == 0){
		return 0;
	}
	if (((ec_dev->fe_media == WAN_MEDIA_T1) && (playout->channel > ec_dev->fe_max_channels)) || 
	    ((ec_dev->fe_media == WAN_MEDIA_E1) && (playout->channel >= ec_dev->fe_max_channels))){ 
		PRINT(verbose,
			"ERROR: %s: Channel number %d out of range!\n",
				ec_dev->if_name,
				playout->channel);
			return EINVAL;
	}

	if (ec_dev->fe_media == WAN_MEDIA_T1) playout->channel--; 
	ec_channel = wan_ecd_fe2ec_channel(ec_dev, playout->channel);

	if (strlen(playout->str)){
		int	i;

		for(i = 0; i < strlen(playout->str); i++){
			err = wan_ecd_playout_tone(
						ec, ec_channel,
						playout->str[i],
						playout->delay);
			if (err) return EINVAL;
		}
	}else{
		return wan_ecd_playout_test(ec, ec_channel, playout->delay);
	}
	return 0;
}
#endif

static int wan_ecd_ISRPoll(wan_ec_msg_t *ec_msg)
{
	wan_ecd_t	*ec;
	wan_ec_dev_t	*ec_dev;
	int		cnt = 0;
#if defined(ENABLE_PRODBIST)
	static int	bist_done = 0;
#endif

	WAN_LIST_FOREACH(ec, &wan_ecd_head, next){
		if (ec->state != WAN_OCT6100_STATE_CHIP_READY){
			continue;
		}
		ec_dev = WAN_LIST_FIRST(&ec->ec_dev_head);

#if defined(ENABLE_PRODBIST)
		if (!bist_done && !wan_ecd_ProductionBist(ec)){
			bist_done = 1;
		}
		continue;
#endif

		if (wan_ecd_ISR(ec)){
			return EINVAL;
		}

		cnt ++;
	}
	return 0;
}

#if defined(WAN_OCT6100_DAEMON)
#define MAX_PROGNAME_LEN	20
unsigned char prognamed[MAX_PROGNAME_LEN];
void sig_handler(int sigio)
{

	if (sigio == SIGUSR1){
		PRINT1(verbose, "%s: Rx signal USR1, User prog ok\n",
						prognamed);
		exit(0);
		return;
	}else if (sigio == SIGUSR2){
		time_t time_var;
		time(&time_var); 
		PRINT1(verbose, "%s: Rx signal USR2, User prog ok!\n",
						prognamed);
		return;
	}else if (sigio == SIGHUP){
		PRINT1(verbose, "%s: Rx signal HUP: Terminal closed!\n",
						prognamed);
		return;
	}
		
	PRINT1(verbose, "%s: Rx TERM/INT (%i) singal\n",
						prognamed,sigio);
	unlink(WAN_EC_SOCKET);
	unlink(WAN_EC_PID);
	exit(1);
}
void catch_signal(int signum,int nomask)
{
	struct sigaction sa;

	memset(&sa,0,sizeof(sa));
	sa.sa_handler = sig_handler;
	sigemptyset(&sa.sa_mask);
	if (nomask){
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
		sa.sa_flags |= SA_NODEFER;
#else
		sa.sa_flags |= SA_NOMASK;
#endif
	}
	
	if (sigaction(signum,&sa,NULL)){
		PRINT(verbose, "%s: sigaction: %d (%s)\n",
					prognamed,
					signum,
					strerror(errno));
	}
}

#define WAN_ECD_POLL_NAME	"wan_ecd_poll"
static int wan_ecd_poll(void)
{
	struct sockaddr_un	address;
	struct timeval		timeout;
	fd_set			readset;
	size_t			addrLength;
	wan_ec_msg_t		ec_msg;
	int			sock, select_cnt, err, serr;

	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
	serr=select(1,NULL,NULL,NULL, &timeout);
		
	memset(&address,0,sizeof(struct sockaddr_un));
	address.sun_family=AF_UNIX;
	strlcpy(address.sun_path, WAN_EC_SOCKET, sizeof(address.sun_path));
	addrLength =	sizeof(address) -
			sizeof(address.sun_path) +
			strlen(address.sun_path);
	while(1){

		timeout.tv_sec = 1;	/* original 10 */
		timeout.tv_usec = 0;
		serr=select(1,NULL,NULL,NULL, &timeout);
		
		if ((sock=socket(PF_UNIX,SOCK_STREAM,0)) < 0){
			PRINT(verbose,
			"%s: ERROR: Failed to open poll socket (%s)!\n",
					WAN_ECD_POLL_NAME, strerror(errno));
			break;
		}
		if(connect(sock,(struct sockaddr*)&address,addrLength)){
			PRINT(verbose,
			"%s: ERROR: Failed to connect poll socket!\n",
					WAN_ECD_POLL_NAME);
			close(sock);
			break;	/*continue;*/
		}

		PRINT2(verbose, "%s: Sending POLL command to EC daemon...\n",
						WAN_ECD_POLL_NAME);
		memset(&ec_msg, 0, sizeof(wan_ec_msg_t));
		ec_msg.cmd = WAN_EC_CMD_POLL;
		err=send(sock,&ec_msg,sizeof(wan_ec_msg_t),0);
		if (err <= 0 && err != sizeof(wan_ec_msg_t)){
			PRINT(verbose,
			"%s: ERROR: Failed to Send POLL command (send,err=%s)\n",
					WAN_ECD_POLL_NAME, strerror(err));
			close(sock);
			continue;
		}

		select_cnt = 0;
select_again:
		FD_ZERO(&readset);
		FD_SET(sock,&readset);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		err = select(sock+1,&readset,NULL,NULL,&timeout);
		if (err == 0){
			if (select_cnt++ < 20){
				goto select_again;
			}
			PRINT(verbose, "%s: ERROR: POLL command is timed out\n",
						WAN_ECD_POLL_NAME);
			close(sock);
			continue;
		}

		if (FD_ISSET(sock,&readset)){
			err=recv(sock,&ec_msg,sizeof(wan_ec_msg_t),0);
			if (err <= 0){
				PRINT(verbose,
				"%s: ERROR: POLL command returns err=%d (recv)!\n",
						WAN_ECD_POLL_NAME, err);
				close(sock);
				continue;
			}
			if (ec_msg.err){
				PRINT(verbose,
				"%s: ERROR: POLL command returns NO ACK (%d)!\n",
						WAN_ECD_POLL_NAME, err);
			}
		}
		close(sock);
		continue;
	}

	exit(0);
}

int main(int argc, char *argv[])
{
	struct sockaddr_un	address;
	pid_t			main_pid, pid, sid, poll_pid = 0;
	int			sock,conn;
	size_t			addrLength;
	wan_ec_msg_t		*ec_msg;
	int			err, nopoll = 0;
	int			fp;

	main_pid = getpid();
	catch_signal(SIGUSR1,0);
	pid=fork();
	if (pid < 0){
		exit(0);
	} else if (pid > 0){
		waitpid(pid,&err,0);
		exit(0);
	}
	pid = getpid();
	
	if (argc > 1){
		int i = 0;
		for(i = 0; i < argc; i++){
			if (strcmp(argv[i], "-v1") == 0){
				verbose = WAN_EC_VERBOSE_MASK_EXTRA1;
			}else if (strcmp(argv[i], "-v2") == 0){
				verbose = WAN_EC_VERBOSE_MASK_EXTRA2;
			}else if (strcmp(argv[i], "-nopoll") == 0){
				nopoll = 1;
			}
		}
	}	

	snprintf(prognamed,MAX_PROGNAME_LEN, "wan_ecd[%d]", getpid());

	/* Initialize poll routine */
	if (nopoll == 0){
		poll_pid=fork();
		if (poll_pid < 0){
			exit(0);
		} else if (poll_pid == 0){
			wan_ecd_poll();
		}
	}

	sid = setsid();
	if (sid < 0){
		exit (EINVAL);
	}
	setuid(0);		/* set real UID = root */
	setgid(getegid());
	
	PRINT(verbose, "\n");
	PRINT(verbose, "%s: Loading EC daemon API v%s (%s)...\n",
					prognamed,
					WAN_ECD_VERSION,
					cOCT6100_API_VERSION);
	if (verbose){
		PRINT(verbose, "Setting verbose to %d\n",
					verbose);
	}

	ec_msg = malloc(sizeof(wan_ec_msg_t));
	if (ec_msg == NULL){
		PRINT(verbose,
		"ERROR: Failed allocate structure (%d bytes)!\n",
					sizeof(wan_ec_msg_t));
		goto daemon_done;
	}
	memset((void*)ec_msg,0,sizeof(wan_ec_msg_t));
	memset(&address,0,sizeof(struct sockaddr_un));

	catch_signal(SIGTERM,0);
	catch_signal(SIGINT,0);
//	catch_signal(SIGUSR1,0);
//	catch_signal(SIGUSR2,0);
	catch_signal(SIGHUP,0);

	if ((sock=socket(PF_UNIX,SOCK_STREAM,0)) < 0){
		PRINT(verbose,
		"ERROR: Failed to create EC daemon socket (err=%d,%s)\n",
					errno, strerror(errno));
		goto daemon_done;
	}

	address.sun_family=AF_UNIX;
	strlcpy(address.sun_path, WAN_EC_SOCKET, sizeof(address.sun_path));
	addrLength =	sizeof(address) -
			sizeof(address.sun_path) +
			strlen(address.sun_path);
	if (bind(sock,(struct sockaddr*) &address, addrLength) < 0){
		PRINT(verbose,
		"ERROR: Failed to bind EC daemon socket (err=%d,%s)\n",
					errno, strerror(errno));
		goto daemon_done;
	}

	if (listen(sock,10)){
		PRINT(verbose,
		"ERROR: Failed to listen EC daemon socket (err=%d,%s)\n",
					errno, strerror(errno));
		goto daemon_exit;
	}

	fp=open(WAN_EC_PID,(O_CREAT|O_WRONLY),0644);
	if (fp){
		char pid_str[10];
		snprintf(pid_str,10,"%d",getpid());
		write(fp,&pid_str,strlen(pid_str));
		close(fp);
	}

	kill(main_pid, SIGUSR1);
	PRINT2(verbose, "Ready and listening...\n\n");
	while ((conn=accept(sock,(struct sockaddr*)&address,&addrLength))>=0){

		wan_ec_dev_t	*ec_dev;

		memset((void*)ec_msg,0,sizeof(wan_ec_msg_t));

		/* Wait for a command from client */
		PRINT2(verbose, "%s: Receiving from %d...\n",
					prognamed, conn);
		err=recv(conn, ec_msg, sizeof(wan_ec_msg_t),0);
		if (err <= 0){
			PRINT(verbose,
			"ERROR: Failed to receive msg from %d (err=%d,%s)!\n",
						conn,err,strerror(err));
			goto daemon_continue;
		}

		/* Reply to client the result of the command */
		if (ec_msg->cmd == WAN_EC_CMD_POLL){
			err = wan_ecd_ISRPoll(ec_msg);
			goto daemon_reply;
		}else if (ec_msg->cmd == WAN_EC_CMD_ACK){
			err = 0;
			goto daemon_reply;		
		}
		ec_dev = wan_ecd_search(ec_msg);
		if (ec_dev == NULL && ec_msg->cmd == WAN_EC_CMD_RELEASE_ALL){
			/* Unload all Echo Canceller devices */
			err = wan_ecd_release_all(NULL);
			err = 0;
			goto daemon_reply;
		}else if (ec_dev == NULL && ec_msg->cmd != WAN_EC_CMD_CONFIG){
			err = ENXIO;	
			goto daemon_reply;
#if 0
		}else{
			/* Something goes wrong... 
			** (device is stopped without release HWEC */
			PRINT(verbose, "%s: Internal Error (%s:%d)\n",
						ec_dev->if_name,
						__FUNCTION__,__LINE__);
			wan_ecd_release(ec_dev);
			err = EINVAL;
			goto daemon_reply;
#endif
		}

		if (ec_msg->verbose){
			PRINT(verbose, "Setting verbose to %d\n",
						ec_msg->verbose);
			verbose = ec_msg->verbose;
		}
		PRINT2(verbose, "%s: Execute %s command!\n",
					ec_msg->if_name,
					WAN_EC_CMD_DECODE(ec_msg->cmd));
		err = -EINVAL;
		switch(ec_msg->cmd){
		case WAN_EC_CMD_CONFIG:
			err = wan_ecd_create(ec_dev, ec_msg);
			break;
		case WAN_EC_CMD_RELEASE:
			err = wan_ecd_release(ec_dev);
			break;
		case WAN_EC_CMD_ENABLE:
		case WAN_EC_CMD_DISABLE:
			err = wan_ecd_modify(ec_dev, ec_msg);
			break;
		case WAN_EC_CMD_BYPASS_ENABLE:
		case WAN_EC_CMD_BYPASS_DISABLE:
			err = wan_ecd_modify_bypass(
						ec_dev,
						&ec_msg->msg_event,
						ec_msg->cmd);
			break;
		case WAN_EC_CMD_MODE_NORMAL:
		case WAN_EC_CMD_MODE_POWERDOWN:
		case WAN_EC_CMD_MODIFY_CHANNEL:
			err = wan_ecd_modify_mode(ec_dev, ec_msg);
			break;
		case WAN_EC_CMD_DTMF_ENABLE:
		case WAN_EC_CMD_DTMF_DISABLE:
			err = wan_ecd_modify_dtmf(ec_dev, ec_msg);
			break;
		case WAN_EC_CMD_STATS:
		case WAN_EC_CMD_STATS_FULL:
			err = wan_ecd_stats(ec_dev, ec_msg);
			break;
#if defined(ENABLE_TONE_PLAY)
		case WAN_EC_CMD_PLAYOUT:
			err = wan_ecd_playout(ec_dev, &ec_msg->msg_playout);
			break;
#endif
		case WAN_EC_CMD_MONITOR:
			err = wan_ecd_monitor(ec_dev, ec_msg);
			break;
		case WAN_EC_CMD_RELEASE_ALL:
			err = wan_ecd_release_all(ec_dev);
			break;
		}

daemon_reply:
		ec_msg->err = err;
		err=send(conn, ec_msg, sizeof(wan_ec_msg_t),0);
		if (err != sizeof(wan_ec_msg_t)){
			PRINT(verbose,
			"ERROR: Failed to send reply msg to %d (err=%d,%s)!\n",
						conn,err, strerror(errno));
			goto daemon_continue;
		}
daemon_continue:
		close(conn);
		if (ec_msg->cmd == WAN_EC_CMD_RELEASE_ALL){
			break;
		}
		fflush(stdout);
		PRINT2(verbose, "Listening...\n\n");
	}

daemon_exit:
	close(sock);
	unlink(WAN_EC_SOCKET);
	unlink(WAN_EC_PID);

daemon_done:
	if (ec_msg){
		free(ec_msg);
	}
	if (poll_pid){
		kill(poll_pid, SIGTERM);
	}
	PRINT(verbose, "%s: Unloading EC daemon API...\n",
						prognamed);
	return 0;
}
#endif
