/*****************************************************************************
* sdla_ec_dev.c 
* 		
* 		WANPIPE(tm) EC Device
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003-2006 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jan 16, 2006	Nenad Corbic	Initial version.
*****************************************************************************/

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
# include <wanpipe_debug.h>
# include <wanpipe.h>
#endif

#include "oct6100_api.h"
#include "oct6100_version.h"

#include "wanec_iface.h"
#include "wanec_iface_api.h"

#if !defined(__WINDOWS__)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
# define WP_ECDEV_UDEV 1
# undef WP_CONFIG_DEVFS_FS
#else
# undef WP_ECDEV_UDEV
# ifdef CONFIG_DEVFS_FS
#  include <linux/devfs_fs_kernel.h>
#  define WP_CONFIG_DEVFS_FS
# else
#  undef WP_CONFIG_DEVFS_FS
#  warning "Error: Hardware EC Device requires DEVFS: HWEC Will not be supported!"
# endif
#endif

#define WP_ECDEV_MAJOR 242
#define WP_ECDEV_MINOR_OFFSET 0

#ifdef WP_ECDEV_UDEV

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
#define WP_CLASS_DEV_CREATE(class, devt, device, name) \
        class_device_create(class, NULL, devt, device, name)
#else
#define WP_CLASS_DEV_CREATE(class, devt, device, name) \
        class_device_create(class, devt, device, name)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13)
static struct class *wanec_dev_class = NULL;
#else
static struct class_simple *wanec_dev_class = NULL;
#define class_create class_simple_create
#define class_destroy class_simple_destroy
#define class_device_create class_simple_device_add
#define class_device_destroy(a, b) class_simple_device_remove(b)
#endif

#endif

#define	UNIT(file) MINOR(file->f_dentry->d_inode->i_rdev)

#define WP_ECDEV_MAX_CHIPS 255

#ifdef WP_CONFIG_DEVFS_FS 
devfs_handle_t devfs_handle;
#endif

#endif/*#if !defined(__WINDOWS__)*/


static int wanec_dev_open(struct inode*, struct file*);
static int wanec_dev_release(struct inode*, struct file*);
static int wanec_dev_ioctl(struct inode*, struct file*, unsigned int, unsigned long);

#if !defined(__WINDOWS__)
/*==============================================================
  Global Variables
 */
static struct file_operations wanec_dev_fops = {
	owner: THIS_MODULE,
	llseek: NULL,
	open: wanec_dev_open,
	release: wanec_dev_release,
	ioctl: wanec_dev_ioctl,
	read: NULL,
	write: NULL,
	poll: NULL,
	mmap: NULL,
	flush: NULL,
	fsync: NULL,
	fasync: NULL,
};

int wanec_create_dev(void)
{
	int err;

	DEBUG_EVENT("%s: Registering Wanpipe ECDEV Device!\n",__FUNCTION__);
#ifdef WP_ECDEV_UDEV  
	wanec_dev_class = class_create(THIS_MODULE, "wp_ec");
#endif
	
#ifdef WP_CONFIG_DEVFS_FS
	err=devfs_register_chrdev(WP_ECDEV_MAJOR, "wp_ec", &wanec_dev_fops);  
	if (err) {
              	DEBUG_EVENT("Unable to register tor device on %d\n", WP_ECDEV_MAJOR);
	    	return err;	
	}
#else
	if ((err = register_chrdev(WP_ECDEV_MAJOR, "wp_ec", &wanec_dev_fops))) {
		DEBUG_EVENT("Unable to register tor device on %d\n", WP_ECDEV_MAJOR);
		return err;
	}
#endif

#ifdef WP_ECDEV_UDEV	
	WP_CLASS_DEV_CREATE(	wanec_dev_class,
				MKDEV(WP_ECDEV_MAJOR, 0),
				NULL,
				WANEC_DEV_NAME);
#endif
	
#ifdef WP_CONFIG_DEVFS_FS 
	{
	umode_t mode = S_IFCHR|S_IRUGO|S_IWUGO;    
	devfs_handle = devfs_register(NULL, WANEC_DEV_NAME, DEVFS_FL_DEFAULT, WP_ECDEV_MAJOR, 
			       0, mode, &wanec_dev_fops, NULL);
	}
#endif	
	
	return err;
}
 
int wanec_remove_dev(void)
{
	DEBUG_EVENT("%s: Unregistering Wanpipe ECDEV Device!\n",__FUNCTION__);

#ifdef WP_ECDEV_UDEV	
	class_device_destroy(	wanec_dev_class,
				MKDEV(WP_ECDEV_MAJOR, 0));
#endif

#ifdef WP_CONFIG_DEVFS_FS
	devfs_unregister(devfs_handle);
#endif 
	
#ifdef WP_ECDEV_UDEV
	class_destroy(wanec_dev_class);
#endif

#ifdef WP_CONFIG_DEVFS_FS           
        devfs_unregister_chrdev(WP_ECDEV_MAJOR, "wp_ec");    
#else
	unregister_chrdev(WP_ECDEV_MAJOR, "wp_ec");
#endif
	return 0;
}

#else/*#if !defined(__WINDOWS__)*/

int wanec_create_dev(void)
{
	int err = 0;
	DEBUG_EVENT("%s: Registering Wanpipe ECDEV Device!\n",__FUNCTION__);
	return err;
}
 
int wanec_remove_dev(void)
{
	DEBUG_EVENT("%s: Unregistering Wanpipe ECDEV Device!\n",__FUNCTION__);
	return 0;
}
#endif/*#if !defined(__WINDOWS__)*/

static int wanec_dev_open(struct inode *inode, struct file *file)
{
	DEBUG_TEST ("%s: DRIVER OPEN EC DEV\n", __FUNCTION__);
	return 0;
}


static int wanec_dev_release(struct inode *inode, struct file *file)
{
	DEBUG_TEST ("%s: DRIVER CLOSE EC DEV\n", __FUNCTION__);
	return 0;
}

#if !defined(__WINDOWS__)
extern int wanec_ioctl(unsigned int, void*);
static int wanec_dev_ioctl(struct inode *inode, struct file *file, 
		      unsigned int cmd, unsigned long data)
{
	if (data == 0){
		return -EINVAL;
	}
	
	return wanec_ioctl(cmd,(void*)data);
}

#endif
