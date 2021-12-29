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
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <netinet/in.h>
#if defined(__LINUX__)
# include <linux/if.h>
# include <linux/types.h>
# include <linux/if_packet.h>
# include <linux/wanpipe_cfg.h>
#else
# include <net/if.h>
# include <net/wanpipe_cfg.h>
#endif
#include "wan_oct6100.h"

/******************************************************************************
**			  DEFINES AND MACROS
******************************************************************************/
#define MAX_OPEN_RETRY	10

/******************************************************************************
**			STRUCTURES AND TYPEDEFS
******************************************************************************/


/******************************************************************************
**			   GLOBAL VARIABLES
******************************************************************************/
/* Driver version string */
extern int	verbose;

/******************************************************************************
** 			FUNCTION DEFINITIONS
******************************************************************************/
INT wan_ecd_dev_open(char* devname);
INT wan_ecd_dev_close(INT);
int wan_ecd_dev_ioctl(int dev, wan_ec_api_t *ec_api);

/*===========================================================================*\
    OctPciDrvApiDriverOpen:  
\*===========================================================================*/

INT wan_ecd_dev_open(char* devname)
{
	INT	f_Handle;
	char	str[100], *ptr = devname;
	int	cnt = 0;

	while(!isdigit(*ptr)) ptr++;
	snprintf(str, 100, "/dev/wp%dec", (int)strtod(ptr, NULL));
	PRINT(verbose, "%s: Opening dev socket %s\n",
				devname, str);
dev_open_again:
	f_Handle = open(str, O_RDONLY);
	if (f_Handle < 0){
		if (cnt < MAX_OPEN_RETRY){
			cnt++;
			sleep(1);
			goto dev_open_again;
		}
		PRINT(verbose,
		"ERROR: %s: Failed to open device %s (err=%d,%s)\n",
					devname, str,
					errno, strerror(errno));
		return -EINVAL;
	}
	return f_Handle;
}

/*===========================================================================*\
    OctPciDrvApiDriverClose:  
\*===========================================================================*/

INT wan_ecd_dev_close( INT f_Handle )
{
	if( f_Handle < 0 ){
		return EINVAL;
	}  	
	close( f_Handle );
	return 0;
}

/*===========================================================================*\
    wan_ecd_dev_ioctl  
\*===========================================================================*/
int wan_ecd_dev_ioctl(int sock, wan_ec_api_t *ec_api)
{
	return ioctl(sock, ec_api->cmd, ec_api);
}

