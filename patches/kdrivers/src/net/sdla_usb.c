/*****************************************************************************
* sdla_usb.c 
* 		
* 		WANPIPE(tm) AFT TE1 Hardware Support
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
* Jan 07, 2003	Nenad Corbic	Initial version.
* Oct 25, 2004  Nenad Corbic	Support for QuadPort TE1
*****************************************************************************/



# include "wanpipe_includes.h"
# include "wanpipe_defines.h"
# include "wanpipe_debug.h"
# include "wanproc.h"
# include "wanpipe_snmp.h"
# include "wanpipe.h"	/* WANPIPE common user API definitions */
# include "aft_core.h"
# include "wanpipe_iface.h"
#include "if_wanpipe_common.h"

#if defined(__LINUX__)
#include "if_wanpipe.h"
#endif	
