/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

File: octtypentdrv.h

Copyright (c) 2001 Octasic Inc. All rights reserved.

Description: This file defines the base storage types for the NT driver 
			 environment.

$Octasic_Confidentiality: $

$Octasic_Release: $

$Revision: 1.1 $

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#ifndef __OCTTYPENTDRV_H__
#define __OCTTYPENTDRV_H__

#if defined(__KERNEL__)

#include <ntddk.h>//for DbgPrint

#if 0
 #define OCT_FUNC_DEBUG()									\
	{														\
		DbgPrint("%s:%s(): File: %s, Line: %d.\n",			\
			"Oct", __FUNCTION__, __FILE__, __LINE__);	\
	}
#else
 #define OCT_FUNC_DEBUG()
#endif

#define OCT_DBG	if(0)DbgPrint

//Memory allocation inside ISR is illegal. Create temporary local buffers.
#define OCT_TMP_MEMORY_SIZE	500

#endif	/*__KERNEL__*/
#endif	/*__OCTTYPENTDRV_H__*/
