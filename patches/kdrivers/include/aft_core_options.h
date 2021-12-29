/*****************************************************************************
* aft_core_options.h - Compile options for Sangoma WANPIPE Driver.
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
* 		David Rokhvarg <davidr@sangoma.com>
*
* Copyright:	(c) 1995-2008 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
*****************************************************************************/

#ifndef _AFT_CORE_OPTIONS_H
#define _AFT_CORE_OPTIONS_H


#ifdef WAN_KERNEL

/*=================================================================
 * Feature Defines
 *================================================================*/

#if defined(__LINUX__) || defined(__WINDOWS__)
# ifndef AFT_API_SUPPORT
#  define AFT_API_SUPPORT 1
# endif
#else
#undef AFT_API_SUPPORT
#endif

#if defined(__LINUX__) || defined(__WINDOWS__)
# ifndef AFT_TDM_API_SUPPORT
#  define AFT_TDM_API_SUPPORT 1
# endif
#else
#undef AFT_TDM_API_SUPPORT
#endif


#if defined(__LINUX__)
#ifndef AFT_RTP_SUPPORT
# define AFT_RTP_SUPPORT 1
#endif
#else
# undef AFT_RTP_SUPPORT
#endif

#if 1
#undef WAN_NO_INTR
#else
#define WAN_NO_INTR 1
#pragma message ("Warning: WAN_NO_INTR Defined!!")
#endif

#if defined(__WINDOWS__)
 /********** compilation flags ************/
 /* compile protocols in the LIP layer */
 #define CONFIG_PRODUCT_WANPIPE_FR
 #define CONFIG_PRODUCT_WANPIPE_CHDLC
 #define CONFIG_PRODUCT_WANPIPE_PPP
 #define CONFIG_PRODUCT_WANPIPE_LAPB

  /* compile AFT 56k code */
 #define CONFIG_PRODUCT_WANPIPE_AFT_56K
 /* compile "old" AFT T1/E1 code */
 /*#define CONFIG_PRODUCT_WANPIPE_AFT*/
 /* compile "new/shark" AFT T1/E1 code */
 #define CONFIG_PRODUCT_WANPIPE_AFT_TE1
 /* compile AFT A200 Analog code */
 #define CONFIG_PRODUCT_WANPIPE_AFT_RM
 #define CONFIG_WANPIPE_PRODUCT_AFT_RM

 /* compile HWEC code */
 #define CONFIG_WANPIPE_HWEC
 /* compile ADSL code */
 //#define CONFIG_PRODUCT_WANPIPE_ADSL
 /* compile ISDN BRI code */
 #define CONFIG_PRODUCT_WANPIPE_AFT_BRI
 #define CONFIG_WANPIPE_PRODUCT_AFT_BRI

 /* compile AFT Serial code */
 #define CONFIG_PRODUCT_WANPIPE_AFT_SERIAL

 #define WAN_IS_TASKQ_SCHEDULE 1

 /* compile TDM Voice API in wanpipe_tdm_api.c */
 #define BUILD_TDMV_API

 #define SDLADRV_HW_IFACE

 /********** end of compilation flags ************/
#endif/* __WINDOWS__ */

#endif
#endif
