/*****************************************************************************
* sdla_cdev.c 
* 		
* 		WANPIPE(tm) Character Device
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*		Alex Feldman <al.feldman@sangoma.com>
*
* Copyright:	(c) 2003-2006 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jan 16, 2006	Nenad Corbic	Initial version.
* Mar 16, 2006	Alex Feldman	Add support for FreeBSD
*****************************************************************************/

/******************************************************************************
*			   INCLUDE FILES
******************************************************************************/
#if defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_version.h>
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <wanpipe_common.h>
# include <wanpipe.h>
# include <sdla_cdev.h>
#elif defined(__LINUX__)
# error "Unsupported Operating System"
#else
# error "Unsupported Operating System"
#endif

/******************************************************************************
*			  DEFINES AND MACROS
******************************************************************************/
#define WP_CDEV_MAX_NUM	255
#define WP_CDEV_NAME	"wp_cdev"

#define WP_CDEV_FLAG_NONE	0x00
#define WP_CDEV_FLAG_READY	0x01
#define WP_CDEV_FLAG_OPEN	0x02

/******************************************************************************
*			  STRUCTURES AND TYPEDEFS
******************************************************************************/
typedef struct __wp_cdev_element {
	char			cdev_name[WAN_DRVNAME_SZ+1];
	unsigned char		flag;
	void			*softc;
	wp_cdev_iface_ioctl_t	*ioctl;
	int			open;
	wan_cdev_t		cdev;
	struct cdevsw		*cdevsw;
} wp_cdev_element_t;

static wan_d_open_t	wp_cdev_open;
static wan_d_close_t	wp_cdev_close;
static wan_d_ioctl_t	wp_cdev_ioctl;

/******************************************************************************
*			  GLOBAL VERIABLES
******************************************************************************/
struct cdevsw wp_cdevsw = {
	.d_version	= D_VERSION,
	.d_flags	= D_NEEDGIANT,
	.d_open	= wp_cdev_open,
	.d_close	= wp_cdev_close,
	.d_ioctl	= wp_cdev_ioctl,
	.d_name	= "wanpipe",
#if (__FreeBSD_version >= 600000)
#elif (__FreeBSD_version >= 503000)
	.d_maj	= WAN_CDEV_MAJOR,
#endif
};

static wp_cdev_element_t	wp_cdev_hash[WP_CDEV_MAX_NUM];
static int			wp_cdev_global_cnt=0;
#if 0
static wan_cdev_t		wp_cdev;
static wan_spinlock_t		wp_cdev_hash_lock;
#endif

/******************************************************************************
*			  FUNCTION PROTOTYPES
******************************************************************************/

/******************************************************************************
*			  FUNCTION DEFINITIONS
******************************************************************************/
static int wp_cdev_search_by_name(char *cdev_name, int *index)
{
	int	i;

	for (i = 0; i < WP_CDEV_MAX_NUM; i++){
		if (!wp_cdev_hash[i].flag){
			continue;
		}
		if (strcmp(wp_cdev_hash[i].cdev_name, cdev_name) == 0){
			*index = i;
			return 0;
		}
	}
	return -EFAULT;
}

static int wp_cdev_search_by_dev(wan_cdev_t dev, int *index)
{
	int	i;

	for (i = 0; i < WP_CDEV_MAX_NUM; i++){
		if (!wp_cdev_hash[i].flag){
			continue;
		}
		if (wp_cdev_hash[i].cdev == dev){
			*index = i;
			return 0;
		}
	}
	return -EFAULT;
}

#if 0
static int wp_cdev_global_reg(void)
{
	wp_cdev = make_dev(	&wp_cdevsw,
				0,
				UID_ROOT,
				GID_WHEEL,
				0600,
				WP_CDEV_NAME);
}

static int wp_cdev_global_unreg(void)
{
	destroy_dev(wp_cdev);
}
#endif

int wp_cdev_reg(void *softc, char *cdev_name, wp_cdev_iface_ioctl_t *ioctl)  
{
	int		cdev_no;

	if (!wp_cdev_search_by_name(cdev_name, &cdev_no)){
		DEBUG_EVENT("%s: Char device already configured!\n",
					cdev_name);
		return -EINVAL; 
	}
#if 0
	if (wp_cdev_global_cnt == 0){
		wp_cdev_global_reg();
	}
#endif
	wp_cdev_global_cnt++;

	for (cdev_no = 0; cdev_no < WP_CDEV_MAX_NUM; cdev_no++){
		if (!wp_cdev_hash[cdev_no].flag){
			break;
		}
	}
	if (cdev_no == WP_CDEV_MAX_NUM){
		DEBUG_EVENT("%s: Trying to register to many cdev!\n",
					cdev_name);
		return -EINVAL;
	}

	DEBUG_EVENT("Registering new character device /dev/%s...\n", cdev_name);
	wp_cdev_hash[cdev_no].softc		= softc;
	memcpy(wp_cdev_hash[cdev_no].cdev_name, cdev_name, strlen(cdev_name));
	wp_cdev_hash[cdev_no].ioctl		= ioctl;
	wp_cdev_hash[cdev_no].flag		= WP_CDEV_FLAG_READY;
	wp_cdev_hash[cdev_no].cdevsw	= &wp_cdevsw;

#if defined(__FreeBSD__)
	wp_cdev_hash[cdev_no].cdev =
			make_dev(&wp_cdevsw,0,UID_ROOT,GID_WHEEL,0600,"%s/%s",
					wp_cdevsw.d_name, cdev_name);
	make_dev_alias(wp_cdev_hash[cdev_no].cdev, "%s", cdev_name);
#endif
	return 0;
}

int wp_cdev_unreg(char *cdev_name)
{
	int	cdev_no;

	if (wp_cdev_search_by_name(cdev_name, &cdev_no)){
		DEBUG_EVENT("%s: Char device is not configured!\n",
					cdev_name);
		return -EINVAL; 
	}
	DEBUG_EVENT("Unregistering character device /dev/%s...\n",
				wp_cdev_hash[cdev_no].cdev_name);
#if defined(__FreeBSD__)
	destroy_dev(wp_cdev_hash[cdev_no].cdev);
#endif
	wp_cdev_hash[cdev_no].softc	= NULL;
	memset(wp_cdev_hash[cdev_no].cdev_name, 0, WAN_DRVNAME_SZ);
	wp_cdev_hash[cdev_no].ioctl	= NULL;
	wp_cdev_hash[cdev_no].flag	= WP_CDEV_FLAG_NONE;
	wp_cdev_global_cnt--;
#if 0
	if (wp_cdev_global_cnt == 0){
		wp_cdev_global_unreg();
	}
#endif
	return 0;
}

static int
wp_cdev_open(wan_cdev_t dev, int flag, int otyp, wan_proc_ptr_t procp)
{
	int	index = 0;

	if (wp_cdev_search_by_dev(dev, &index)){
		DEBUG_EVENT("ERROR: Failed to find device\n"); 
		return -EINVAL;
	}
	if (!(wp_cdev_hash[index].flag & WP_CDEV_FLAG_READY)){
		DEBUG_EVENT("ERROR: Device is not ready!\n"); 
		return -EINVAL;
	
	}
	wp_cdev_hash[index].flag	|= WP_CDEV_FLAG_OPEN;
	return (0);
}
static int
wp_cdev_close(wan_cdev_t dev, int flag, int otyp, wan_proc_ptr_t procp)
{   
	int	index = 0;

	if (wp_cdev_search_by_dev(dev, &index)){
		DEBUG_EVENT("ERROR: Failed to find device\n"); 
		return -EINVAL;
	}
	if (!(wp_cdev_hash[index].flag & WP_CDEV_FLAG_READY) && !(wp_cdev_hash[index].flag & WP_CDEV_FLAG_OPEN)){
		DEBUG_EVENT("ERROR: Device is not ready/open!\n"); 
		return -EINVAL;
	}
	wp_cdev_hash[index].flag	&= ~WP_CDEV_FLAG_OPEN;
	return (0);
}

#if defined(__FreeBSD__)
static int
wp_cdev_ioctl(wan_cdev_t dev, u_long cmd, caddr_t data,
				      int fflag, struct thread *td)
#elif defined(__OpenBSD__)
static int
wp_cdev_ioctl(wan_cdev_t dev, u_long cmd, caddr_t data,
				      int fflag, struct proc *p)
#endif
{
	int	index, err;

	if (wp_cdev_search_by_dev(dev, &index)){
		DEBUG_EVENT("ERROR: Failed to find device\n"); 
		return -EINVAL;
	}
	if (wp_cdev_hash[index].ioctl == NULL){
		DEBUG_EVENT("ERROR: %s: Ioctl function pointer is NULL\n", 
				wp_cdev_hash[index].cdev_name);
		return -EINVAL;
		
	}
	err = wp_cdev_hash[index].ioctl(
				wp_cdev_hash[index].softc,
				data);
	return err;
}


