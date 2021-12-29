/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

File: octtypentdrv.h

Copyright (c) 2001 Octasic Inc. All rights reserved.

Description: This file defines the base storage types for the NT driver 
			 environment.

$Octasic_Confidentiality: $

$Octasic_Release: $

$Revision: 1.2 $

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#ifndef __OCTTYPENTDRV_H__
#define __OCTTYPENTDRV_H__


/* 32 bit integer*/
typedef unsigned long ULONG;
typedef signed long LONG;

/* 16 bit integer*/
typedef unsigned short USHORT;
typedef signed short SHORT;
typedef unsigned int UINT;
typedef int INT;

/* 8 bit integer*/
typedef unsigned char BYTE;
typedef unsigned char UCHAR;
typedef signed char SCHAR;

/* 8 bit integer*/
typedef BYTE	UINT8;

/* 32 bit integer */
//DAVIDR: i removed it - "macro redefinition"
//typedef ULONG	UINT32; //??

//DAVIDR: i added it - was "not defined"
typedef u32		BOOL;
typedef u32*	PBOOL;

#if 1
 #define OCT_FUNC_DEBUG()									\
	{														\
		DbgPrint("%s:%s(): File: %s, Line: %d.\n",			\
			"Oct", __FUNCTION__, __FILE__, __LINE__);	\
	}
#else
 #define OCT_FUNC_DEBUG()
#endif

#define OCT_DBG	if(1)DbgPrint

//Memory allocation inside ISR is illegal. Create temporary local buffers.
#define OCT_TMP_MEMORY_SIZE	500

#endif /*__OCTTYPENTDRV_H__*/
