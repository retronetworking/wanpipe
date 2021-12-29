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
#elif defined(__WINDOWS__)
# include <windows.h>
# include <winioctl.h>
# include <stdio.h>
# include <conio.h>
# include <stddef.h>		//for offsetof()
# include <stdlib.h>
# include <wanpipe_ctypes.h>
# include <sang_status_defines.h>
# include <sang_api.h>
# include <wanpipe_defines.h>
# include <wanpipe_cfg.h>
# include <sang_api.h>

#define sleep	Sleep
#define FUNC_DEBUG	if(1)printf("%s: line: %d\n", __FILE__, __LINE__);
#define DBGPRINT if(1)printf

//////////////////////////////////////////////////////////////////
//IOCTL management structures and variables.
//Note: this program is NOT multithreded, so it's ok to use global buffer.
wan_udp_hdr_t	wan_udp;

#else
# include <wanpipe_defines.h>
# include <wanpipe_cfg.h>
#endif

#include <wanec_iface_api.h>

/******************************************************************************
**			  DEFINES AND MACROS
******************************************************************************/

#define OCT6116_32S_IMAGE_NAME	"OCT6116-32S.ima"
#define OCT6116_64S_IMAGE_NAME	"OCT6116-64S.ima"
#define OCT6116_128S_IMAGE_NAME	"OCT6116-128S.ima"
#define OCT6116_256S_IMAGE_NAME	"OCT6116-256S.ima"

#if !defined(__WINDOWS__)

#define WAN_EC_NAME	"wan_ec"
#if defined(__LINUX__)
# define WAN_EC_DIR	"/etc/wanpipe/" WAN_EC_NAME
#else
# define WAN_EC_DIR	"/usr/local/etc/wanpipe/" WAN_EC_NAME
#endif
#define WAN_EC_BUFFERS	WAN_EC_DIR "/buffers"

#define OCT6116_32S_IMAGE_PATH	WAN_EC_DIR "/" OCT6116_32S_IMAGE_NAME
#define OCT6116_64S_IMAGE_PATH	WAN_EC_DIR "/" OCT6116_64S_IMAGE_NAME
#define OCT6116_128S_IMAGE_PATH	WAN_EC_DIR "/" OCT6116_128S_IMAGE_NAME
#define OCT6116_256S_IMAGE_PATH	WAN_EC_DIR "/" OCT6116_256S_IMAGE_NAME

#else

char WAN_EC_DIR[MAX_PATH];
char WAN_EC_BUFFERS[MAX_PATH];
char windows_dir[MAX_PATH];

char OCT6116_32S_IMAGE_PATH[MAX_PATH];
char OCT6116_64S_IMAGE_PATH[MAX_PATH];
char OCT6116_128S_IMAGE_PATH[MAX_PATH];
char OCT6116_256S_IMAGE_PATH[MAX_PATH];

#endif

#define MAX_OPEN_RETRY	10

/******************************************************************************
**			STRUCTURES AND TYPEDEFS
******************************************************************************/

/******************************************************************************
**			   GLOBAL VARIABLES
******************************************************************************/
#if 1
typedef struct {
	UINT32	max_channels;
	UINT32	memory_chip_size;
	UINT32	debug_data_mode;
	char	*image_name;	
	char	*image_path;
} wanec_image_info_t;

wanec_image_info_t	wanec_image_32 = 
		{
			32, cOCT6100_MEMORY_CHIP_SIZE_8MB,
			cOCT6100_DEBUG_GET_DATA_MODE_120S,
			OCT6116_32S_IMAGE_NAME, OCT6116_32S_IMAGE_PATH
		};
wanec_image_info_t	wanec_image_64 = 
		{
			64, cOCT6100_MEMORY_CHIP_SIZE_8MB,
			cOCT6100_DEBUG_GET_DATA_MODE_120S,
			OCT6116_64S_IMAGE_NAME, OCT6116_64S_IMAGE_PATH
		};
wanec_image_info_t	wanec_image_128 = 
		{
			128, cOCT6100_MEMORY_CHIP_SIZE_16MB,
			cOCT6100_DEBUG_GET_DATA_MODE_120S,
			OCT6116_128S_IMAGE_NAME, OCT6116_128S_IMAGE_PATH,
		};
wanec_image_info_t	wanec_image_256 = 
		{
			256, cOCT6100_MEMORY_CHIP_SIZE_16MB,
			cOCT6100_DEBUG_GET_DATA_MODE_120S,
			OCT6116_256S_IMAGE_NAME, OCT6116_256S_IMAGE_PATH
		};

typedef struct {
	u_int16_t		hwec_chan_no;
	int			images_no;
	wanec_image_info_t	*image_info[5];
} wanec_image_list_t;

wanec_image_list_t	wanec_image_list[] = 
	{
		{ 16,  1, { &wanec_image_32  } },
		{ 32,  2, { &wanec_image_64, &wanec_image_32 } },
		{ 64,  1, { &wanec_image_64  } },
		{ 128, 2, { &wanec_image_256, &wanec_image_128 } },
		{ 256, 1, { &wanec_image_256 } },
		{ 0, 0L }
	};
#else				
struct wan_ec_image
{
	int	hwec_chan_no;
	UINT32	max_channels;
	UINT32	memory_chip_size;
	UINT32	debug_data_mode;
	char	*image;	
	char	*image_path;
	int	hwec_opt_no;
	int	hwec_opt[5];
} wan_ec_image_list[] =
	{
		{ 	16,
			32, cOCT6100_MEMORY_CHIP_SIZE_8MB,
			cOCT6100_DEBUG_GET_DATA_MODE_16S,
			OCT6116_32S_IMAGE_NAME, OCT6116_32S_IMAGE_PATH
		},
		{ 	32,
			32, cOCT6100_MEMORY_CHIP_SIZE_8MB,
			cOCT6100_DEBUG_GET_DATA_MODE_16S,
			OCT6116_32S_IMAGE_NAME, OCT6116_32S_IMAGE_PATH,
			1, { 64 }
		},
		{ 	64,
			32, cOCT6100_MEMORY_CHIP_SIZE_8MB,
			cOCT6100_DEBUG_GET_DATA_MODE_16S,
			OCT6116_64S_IMAGE_NAME, OCT6116_64S_IMAGE_PATH
		},
		{	128,
			128, cOCT6100_MEMORY_CHIP_SIZE_16MB,
			cOCT6100_DEBUG_GET_DATA_MODE_120S,
			OCT6116_128S_IMAGE_NAME, OCT6116_128S_IMAGE_PATH,
			1, { 256 }
		},
		{	256,
			256, cOCT6100_MEMORY_CHIP_SIZE_16MB,
			cOCT6100_DEBUG_GET_DATA_MODE_16S,
			OCT6116_256S_IMAGE_NAME, OCT6116_256S_IMAGE_PATH
		},
		{	0, 0, 0, 0, NULL }
	};
#endif

#if !defined(__WINDOWS__)	
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
#endif

/******************************************************************************
** 			FUNCTION PROTOTYPES
******************************************************************************/
#if defined(__WINDOWS__)
HANDLE wanec_api_lib_open(wan_ec_api_t *ec_api);
int wanec_api_lib_close(wan_ec_api_t *ec_api, HANDLE dev);
int wanec_api_lib_ioctl(HANDLE dev, wan_ec_api_t *ec_api, int verbose);
int wanec_api_lib_chip_load(HANDLE dev, wan_ec_api_t *ec_api, u_int16_t, int verbose);
#else
static int wanec_api_lib_open(wan_ec_api_t *ec_api);
static int wanec_api_lib_close(wan_ec_api_t *ec_api, int dev);
static int wanec_api_lib_ioctl(int dev, wan_ec_api_t *ec_api, int verbose);
int wanec_api_lib_chip_load(int dev, wan_ec_api_t *ec_api, u_int16_t, int verbose);
#endif

int wanec_api_lib_config(wan_ec_api_t *ec_api, int verbose);
int wanec_api_lib_bufferload(wan_ec_api_t *ec_api);
int wanec_api_lib_monitor(wan_ec_api_t *ec_api);
int wanec_api_lib_cmd(wan_ec_api_t *ec_api);


/******************************************************************************
** 			FUNCTION DEFINITIONS
******************************************************************************/

#if !defined(__WINDOWS__)
static int wanec_api_lib_open(wan_ec_api_t *ec_api)
{
	int	dev;
	int	cnt = 0;

dev_open_again:

	dev = open(WANEC_DEV_DIR WANEC_DEV_NAME, O_RDONLY);
	if (dev < 0){
		if (cnt < MAX_OPEN_RETRY){
			fprintf(stderr, "--> Waiting for ec device %s....\n",
						WANEC_DEV_DIR WANEC_DEV_NAME);
			cnt++;
			sleep(1);
			goto dev_open_again;
		}
		printf("ERROR: Failed to open device %s%s (err=%d,%s)\n",
					WANEC_DEV_DIR, WANEC_DEV_NAME,
					errno, strerror(errno));
		return -EINVAL;
	}
	return dev;
}

static int wanec_api_lib_close(wan_ec_api_t *ec_api, int dev)
{
	if (dev < 0){
		return -EINVAL;
	}  	
	close(dev);
	return 0;
}

static int wanec_api_lib_ioctl(int dev, wan_ec_api_t *ec_api, int verbose)
{
	int err;
	
	ec_api->err = WAN_EC_API_RC_OK;
	err = ioctl(dev, ec_api->cmd, ec_api);
	if (err){
		printf("Failed (%s)!\n", strerror(errno));
		return err;
	}
	if (ec_api->err){
		switch(ec_api->err){
		case WAN_EC_API_RC_INVALID_STATE:
			printf("Failed (Invalid State:%s)!\n",
					WAN_EC_STATE_DECODE(ec_api->state));
			break;
		case WAN_EC_API_RC_FAILED:
		case WAN_EC_API_RC_INVALID_CMD:
		case WAN_EC_API_RC_INVALID_DEV:
		case WAN_EC_API_RC_BUSY:
		case WAN_EC_API_RC_INVALID_CHANNELS:
		case WAN_EC_API_RC_INVALID_PORT:
		default:
			printf("Failed (%s)!\n",
					WAN_EC_API_RC_DECODE(ec_api->err));
			break;
		}
		return -EINVAL;
	}
	return 0;
}

#else

HANDLE wanec_api_lib_open(wan_ec_api_t *ec_api)
{
	HANDLE	hGeneralCommands = INVALID_HANDLE_VALUE;
	char device_name[200];

	////////////////////////////////////////////////////////////////////////////
	//Open device for general commands
	printf("Opening Device: %s\n", ec_api->devname);
	_snprintf(device_name , 200, "\\\\.\\%s", ec_api->devname);

	hGeneralCommands = CreateFile(
							device_name, 
							GENERIC_READ | GENERIC_WRITE, 
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							(LPSECURITY_ATTRIBUTES)NULL, 
							OPEN_EXISTING,
							FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
							(HANDLE)NULL
							);
    if (hGeneralCommands == INVALID_HANDLE_VALUE){
		printf("Unable to open %s for general commands!\n", device_name);
		return INVALID_HANDLE_VALUE;
	}

	return hGeneralCommands;
}

int wanec_api_lib_close(wan_ec_api_t *ec_api, HANDLE f_Handle)
{
	if(f_Handle == INVALID_HANDLE_VALUE){
		return EINVAL;
	}  	
	CloseHandle(f_Handle);
	return 0;
}

int wanec_api_lib_ioctl(HANDLE dev, wan_ec_api_t *ec_api, int verbose)
{
	int err;
	DWORD ln;
    unsigned char id = 0;

	wan_udp.wan_udphdr_request_reply = 0x01;
	wan_udp.wan_udphdr_id = id;
   	wan_udp.wan_udphdr_return_code = WAN_UDP_TIMEOUT_CMD;

	wan_udp.wan_udphdr_command	= WAN_EC_IOCTL;
	wan_udp.wan_udphdr_data_len	= sizeof(wan_ec_api_t);

	ec_api->err = WAN_EC_API_RC_OK;

	memcpy(	wan_udp.wan_udphdr_data, 
			(void*)ec_api,
			sizeof(wan_ec_api_t));

	if(DeviceIoControl(
			dev,
			IoctlManagementCommand,
			(LPVOID)&wan_udp,
			sizeof(wan_udp_hdr_t),
			(LPVOID)&wan_udp,
			sizeof(wan_udp_hdr_t),
			(LPDWORD)(&ln),
			(LPOVERLAPPED)NULL
			) == FALSE){
		err = 1;
		printf("DeviceIoControl() failed!!\n");
	}else{
		err = 0;

		//copy data from driver to caller's buffer
		memcpy(	(void*)ec_api,
				wan_udp.wan_udphdr_data, 
				sizeof(wan_ec_api_t));

		if (ec_api->err){
			switch(ec_api->err){
			case WAN_EC_API_RC_INVALID_STATE:
				printf("Failed (Invalid State:%s)!\n",
						WAN_EC_STATE_DECODE(ec_api->state));
				break;
			case WAN_EC_API_RC_FAILED:
			case WAN_EC_API_RC_INVALID_CMD:
			case WAN_EC_API_RC_INVALID_DEV:
			case WAN_EC_API_RC_BUSY:
			default:
				printf("Failed (%s)!\n",
						WAN_EC_API_RC_DECODE(ec_api->err));
				break;
			}
			return -EINVAL;
		}
	}

	return err;
}

#endif


/*
**	WAN EC Config
**/
static int
//wan_ec_loadImageFile(wan_ec_api_t *ec_api, struct wan_ec_image *image_info)
wanec_api_lib_loadImageFile(	wan_ec_api_t 	*ec_api,
				char		*path,
				PUINT8		*pData,
				UINT32		*size)
{
	UINT32	ulNumBytesRead;
	FILE*	pFile;
	PUINT8	ptr;
	
	/* Validate the parameters.*/
	if (path == NULL){
		printf("ERROR: %s: Invalid image filename!\n",
					ec_api->devname);
		return -EINVAL;
	}

	/*==================================================================*/
	/* Open the file.*/
	
	pFile = fopen(path, "rb" );
	if ( pFile == NULL ){
		printf("ERROR: %s: Failed to open file (%s)!\n",
					ec_api->devname, path);
		return -EINVAL;
	}

	/*====================================================================*/
	/* Extract the file length.*/

	fseek( pFile, 0, SEEK_END );
	*size = (UINT32)ftell( pFile );
	fseek( pFile, 0, SEEK_SET );
	
	if (*size == 0 ){
		fclose( pFile );
		printf("ERROR: %s: Image file is too short!\n",
					ec_api->devname);
		return -EINVAL;
	}

	/*==================================================================*/
	/* Allocate enough memory to store the file content.*/
	
	ptr = (PUINT8)malloc(*size * sizeof(UINT8));
	if (ptr == NULL ){
		fclose( pFile );
		printf("ERROR: %s: Failed allocate memory for image file!\n",
					ec_api->devname);
		return -EINVAL;
	}

	/*==================================================================*/
	/* Read the content of the file.*/

	ulNumBytesRead = fread(	ptr,
				sizeof(UINT8),
				*size,
				pFile );
	if ( ulNumBytesRead != *size ){
		free( ptr );
		fclose( pFile );
		printf("ERROR: %s: Failed to read image file!\n",
					ec_api->devname);
		return -EINVAL;
	}

	/* The content of the file should now be in the f_pbyFileData memory,
	 * we can close the file and return to the calling function.*/
	fclose( pFile );
	*pData = ptr;	
	return 0;
}

/* Oct6100 image path */
static wanec_image_list_t *wanec_api_lib_image_search(u_int16_t hwec_chan_no)
{
	wanec_image_list_t *image_list = &wanec_image_list[0];

	while(image_list && image_list->hwec_chan_no){
		if (image_list->hwec_chan_no == hwec_chan_no){
			return image_list;
		}
		image_list++;
	}	
	return NULL;
}

#if defined(__WINDOWS__)
int wanec_api_lib_chip_load(HANDLE dev, wan_ec_api_t *ec_api, u_int16_t max_channels,int verbose)
#else
int wanec_api_lib_chip_load(int dev, wan_ec_api_t *ec_api, u_int16_t max_channels, int verbose)
#endif
{
	static wanec_image_list_t	*image_list;
	static wanec_image_info_t 	*image_info;
	int				err, image_no = 0;

#if defined(__WINDOWS__)
	//////////////////////////////////////////////////////////////////////////////////////////
	//initialize globals
	GetSystemWindowsDirectory(windows_dir, MAX_PATH);
	_snprintf(WAN_EC_DIR, MAX_PATH, "%s\\%s", windows_dir, "sang_ec_files");

	_snprintf(OCT6116_32S_IMAGE_PATH,	MAX_PATH, "%s\\%s", WAN_EC_DIR, OCT6116_32S_IMAGE_NAME);
	_snprintf(OCT6116_64S_IMAGE_PATH,	MAX_PATH, "%s\\%s", WAN_EC_DIR, OCT6116_64S_IMAGE_NAME);
	_snprintf(OCT6116_128S_IMAGE_PATH,	MAX_PATH, "%s\\%s", WAN_EC_DIR, OCT6116_128S_IMAGE_NAME);
	_snprintf(OCT6116_256S_IMAGE_PATH,	MAX_PATH, "%s\\%s", WAN_EC_DIR, OCT6116_256S_IMAGE_NAME);
#endif

	if (max_channels == 0){
		printf("ERROR: %s: this card does NOT have Echo Canceller chip!\n",
					ec_api->devname);
		return -EINVAL;
	}
	printf("Searching image for %d channels...\n", max_channels);
	
	image_list = wanec_api_lib_image_search(max_channels);
	while(image_no < image_list->images_no){
		image_info = image_list->image_info[image_no];
		if (image_info == NULL){
			printf("Failed!\n\n");
			printf(
			"ERROR: %s: Failed to find image file to EC chip (max=%d)!\n",
					ec_api->devname,
					max_channels);
			return -EINVAL;
		}
		
		fprintf(stderr, "--> Loading ec image %s...\n",
						image_info->image_name);

#if defined(__WINDOWS__)
		printf("Found Image path: %s\n", image_info->image_path);
#endif

		ec_api->u_config.max_channels		= max_channels;
		ec_api->u_config.memory_chip_size	= image_info->memory_chip_size;
		ec_api->u_config.debug_data_mode	= image_info->debug_data_mode;
		if (image_no+1 == image_list->images_no){
			ec_api->u_config.imageLast	= WANOPT_YES;
		}
		/* Load the image file */
		err = wanec_api_lib_loadImageFile(	ec_api,
						image_info->image_path,
						&ec_api->u_config.imageData,
						&ec_api->u_config.imageSize);
		if (err){
			return -EINVAL;
		}
		ec_api->cmd = WAN_EC_API_CMD_CONFIG;
		err = wanec_api_lib_ioctl(dev, ec_api, verbose);
		if (err || ec_api->err){
			free(ec_api->u_config.imageData);
			return (err) ? -EINVAL : 0;
		}
		free(ec_api->u_config.imageData);
		ec_api->u_config_poll.cnt = 0;
config_poll:
		ec_api->cmd = WAN_EC_API_CMD_CONFIG_POLL;
		err = wanec_api_lib_ioctl(dev, ec_api, verbose);
		if (err || ec_api->err){
			wanec_api_lib_close(ec_api, dev);
			return (err) ? -EINVAL : 0;
		}
		if (ec_api->state == WAN_EC_STATE_CHIP_OPEN_PENDING){
			sleep(5);
			printf(".");
			if (ec_api->u_config_poll.cnt++ < WANEC_API_MAX_CONFIG_POLL){
				goto config_poll;
			}
		}
		if (ec_api->state != WAN_EC_STATE_CHIP_OPEN &&
	    	    ec_api->state != WAN_EC_STATE_CHIP_READY){
		    	image_no ++;
			continue;
		}
		break;
	}
	if (ec_api->state != WAN_EC_STATE_CHIP_OPEN &&
	    ec_api->state != WAN_EC_STATE_CHIP_READY){
		printf("Timeout!\n");
		return -EINVAL;
	}

	return 0;
}

int wanec_api_lib_config(wan_ec_api_t *ec_api, int verbose)
{
#if !defined(__WINDOWS__)
	int		dev;
#else
	HANDLE		dev;
#endif
	u_int16_t	hwec_max_channels = 0;
	int		err;

	printf("%s: Configuring Echo Canceller device...\t",
				ec_api->devname);

	dev = wanec_api_lib_open(ec_api);
#if !defined(__WINDOWS__)
	if (dev < 0){
#else
	if (dev == INVALID_HANDLE_VALUE){
#endif
		printf("Failed (Device open)!\n");
		return -ENXIO;
	}

	ec_api->cmd = WAN_EC_API_CMD_GETINFO;
	err = wanec_api_lib_ioctl(dev, ec_api, verbose);
	if (err || ec_api->err){
		wanec_api_lib_close(ec_api, dev);
		return (err) ? -EINVAL : 0;
	}
	hwec_max_channels = ec_api->u_info.max_channels;

	if (ec_api->state == WAN_EC_STATE_RESET){
		if (wanec_api_lib_chip_load(dev, ec_api, hwec_max_channels, verbose)){
			wanec_api_lib_close(ec_api, dev);
			return -EINVAL;
		}
	}
	if (ec_api->state != WAN_EC_STATE_CHIP_OPEN &&
	    ec_api->state != WAN_EC_STATE_CHIP_READY){
		printf("%s: WARNING: Incorrect Echo Canceller state (%s)!\n",
				ec_api->devname,
				WAN_EC_STATE_DECODE(ec_api->state));
		return -EINVAL;
	}	

	/* Open all channels */	
	ec_api->cmd = WAN_EC_API_CMD_CHANNEL_OPEN;
	err = wanec_api_lib_ioctl(dev, ec_api, verbose);
	if (err || ec_api->err){
		wanec_api_lib_close(ec_api, dev);
		return (err) ? -EINVAL : 0;
	}

	printf("Done!\n");
	wanec_api_lib_close(ec_api, dev);
	return err;
}


int wanec_api_lib_bufferload(wan_ec_api_t *ec_api)
{	
	char	buffer_path[100];
	int	err = 0;
#if !defined(__WINDOWS__)
	int	dev;
#else
	HANDLE	dev;
#endif

	printf("%s: Start Tone loading...\t",
			ec_api->devname);
	dev = wanec_api_lib_open(ec_api);
#if !defined(__WINDOWS__)
	if (dev < 0){
#else
	if (dev == INVALID_HANDLE_VALUE){
#endif
		printf("Failed (Device open)!\n");
		return -ENXIO;
	}
#if !defined(__WINDOWS__)
	sprintf(buffer_path, "%s/%s.pcm", WAN_EC_BUFFERS, ec_api->u_buffer_config.buffer);
#else
	GetSystemWindowsDirectory(windows_dir, MAX_PATH);
	_snprintf(WAN_EC_BUFFERS, MAX_PATH, "%s\\%s", windows_dir, "sang_ec_files");
	sprintf(buffer_path, "%s\\%s.pcm", WAN_EC_BUFFERS, ec_api->u_buffer_config.buffer);
#endif
	err = wanec_api_lib_loadImageFile(	ec_api,
						buffer_path,
						&ec_api->u_buffer_config.data,
						&ec_api->u_buffer_config.size);
	if (err){
		printf("Failed!\n");
		goto buffer_load_done;
	}	
	
	err = wanec_api_lib_ioctl(dev, ec_api, ec_api->verbose);
	if (err || ec_api->err){
		goto buffer_load_done;
	}	
	printf("Done!\n");
	
buffer_load_done:
	wanec_api_lib_close(ec_api, dev);
	if (ec_api->u_buffer_config.data){
		free(ec_api->u_buffer_config.data);
	}

	return err;
}

int wanec_api_lib_monitor(wan_ec_api_t *ec_api)
{
	int	dev, err=0;
#if !defined(__WINDOWS__)
	if (ec_api->fe_chan){

		dev = wanec_api_lib_open(ec_api);
		if (dev < 0){
			return ENXIO;
		}

		printf("%s: Start monitorting the channel %d ...",
				ec_api->devname,
				ec_api->fe_chan);
		err = wanec_api_lib_ioctl(dev, ec_api, 1);
		if (err || ec_api->err){
			wanec_api_lib_close(ec_api, dev);
			return (err) ? -EINVAL : 0;
		}
		printf("Done!\n");
		wanec_api_lib_close(ec_api, dev);
	}else{

		FILE		*output = NULL;
		size_t		len;
		int		cnt = 0;
		char		filename[MAXPATHLEN];
		
		printf("%s: Reading Monitor Data ..",
					ec_api->devname);
		do {
			dev = wanec_api_lib_open(ec_api);
			if (dev < 0){
				return ENXIO;
			}
			ec_api->fe_chan = 0;
			ec_api->u_chan_monitor.remain_len = 0;
			ec_api->u_chan_monitor.remain_len = 0;
			ec_api->u_chan_monitor.data_len = 0;
			ec_api->u_chan_monitor.max_len = MAX_MONITOR_DATA_LEN;
			err = wanec_api_lib_ioctl(dev, ec_api, 0);
			if (err || ec_api->err){
				err = (err) ? -EINVAL : 0;
				goto monitor_done;
			}
			if (output == NULL){
				struct timeval	tv;
				struct tm	t;
				/* Firsr data */
				memset(filename, 0, MAXPATHLEN);
				gettimeofday(&tv, NULL);
				localtime_r((time_t*)&tv.tv_sec, &t);
				snprintf(filename, MAXPATHLEN,
				"%s_%s_chan%d_%d.%d.%d_%d.%d.%d.bin",
					WAN_EC_NAME,
					ec_api->devname, ec_api->fe_chan,
					t.tm_mon,t.tm_mday,1900+t.tm_year,
					t.tm_hour,t.tm_min,t.tm_sec);
				output = fopen(filename, "wb");
				if (output == NULL){
					printf("Failed!\n");
					printf("ERROR: %s: Failed to open binary file (%s)\n",
							ec_api->devname, filename);
					err = -EINVAL;
					goto monitor_done;
				}
			}
			len = fwrite(&ec_api->u_chan_monitor.data[0], sizeof(UINT8),
					ec_api->u_chan_monitor.data_len, output);
			if (len != ec_api->u_chan_monitor.data_len){
				printf("Failed!\n");
				printf("ERROR: %s: Failed to write to binary file (%s)!\n",
							ec_api->devname, filename);
				err = -EINVAL;
				goto monitor_done;
			}
			if (++cnt % 100 == 0){
				printf(".");
			}
			wanec_api_lib_close(ec_api, dev);
		}while(ec_api->u_chan_monitor.remain_len);
		if (output) fclose(output);
		printf("Done!\n");
		printf("Binary dump information is stored in %s file.\n", filename);
	}
monitor_done:
#endif
	return err;
}

int wanec_api_lib_cmd(wan_ec_api_t *ec_api)
{
	int		err;
#if !defined(__WINDOWS__)
	int		dev;
#else
	HANDLE	dev;
#endif
	
	switch(ec_api->cmd){
	case WAN_EC_API_CMD_CONFIG:
		return wanec_api_lib_config(ec_api, 1);
	case WAN_EC_API_CMD_MONITOR:
		return wanec_api_lib_monitor(ec_api);
	default:
		dev = wanec_api_lib_open(ec_api);
#if !defined(__WINDOWS__)
		if (dev < 0){
#else
		if (dev == INVALID_HANDLE_VALUE){
#endif
			printf("Failed (Device open)!\n");
			return -ENXIO;
		}
		printf("%s: Running %s command to Echo Canceller device...\t",
				ec_api->devname,
				WAN_EC_API_CMD_DECODE(ec_api->cmd));
		err = wanec_api_lib_ioctl(dev, ec_api, 1);
		if (err || ec_api->err){
			wanec_api_lib_close(ec_api, dev);
			return (err) ? -EINVAL : 0;
		}
		printf("Done!\n\n");
		wanec_api_lib_close(ec_api, dev);
		break;
	}
	return 0;
}

