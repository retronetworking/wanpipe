/******************************************************************************
 * sdla_cdev.h	
 *
 * Author: 	Alex Feldman  <al.feldman@sangoma.com>
 *
 * Copyright:	(c) 1995-2001 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 ******************************************************************************
 */


/******************************************************************************
*			  STRUCTURES AND TYPEDEFS
******************************************************************************/
typedef int wp_cdev_iface_ioctl_t(void *softc, void *data);

/******************************************************************************
*			  FUNCTION PROTOTYPES
******************************************************************************/
extern int wp_cdev_reg(void *softc, char *cdev_name, wp_cdev_iface_ioctl_t *ioctl);  
extern int wp_cdev_unreg(char *cdev_name);
