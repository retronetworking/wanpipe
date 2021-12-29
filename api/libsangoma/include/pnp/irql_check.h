/*****************************************************************************
* irql_check.h WANPIPE(tm) Kernel Abstraction Layer.
*
* Author: 	David Rokhvarg <davidr@sangoma.com>
*
* Copyright:	(c) 2003 Sangoma Technologies Inc.
*
*	In many cases have to check IRQL of current execution thread,
*	this file contains set of macros to do that.
*
* ============================================================================
* September 15, 2006  David Rokhvarg	Initial version
* ============================================================================
*
*/

#ifndef _IRQL_CHECK_H
#define _IRQL_CHECK_H

#define SILENT	0xABCD

#define DBG_IRQL_CHECK	DbgPrint
//#define DBG_IRQL_CHECK

static __inline const char* decode_irql(KIRQL	irql)
{
	switch(irql)
	{
	case PASSIVE_LEVEL:
		return "PASSIVE_LEVEL";

	case APC_LEVEL:
		return "APC_LEVEL";

	case DISPATCH_LEVEL:
		return "DISPATCH_LEVEL";

	case PROFILE_LEVEL:
		return "PROFILE_LEVEL";

	default:
		return "Unknown High IRQL";
	}
}

//verify that current IRQL is NOT higher than PASSIVE_LEVEL
#define VERIFY_PASSIVE_IRQL(rc)					\
{												\
	KIRQL current_irql = KeGetCurrentIrql();	\
												\
	if(current_irql > PASSIVE_LEVEL){			\
		ERR_DBG_OUT(("Failed IRQL PASSIVE_LEVEL test! Current IRQL: 0x%X (%s).\n",	\
			current_irql, decode_irql(current_irql)));	\
		rc = 1;		\
	}else{			\
		rc = 0;		\
	}				\
}

//verify that current IRQL is NOT higher than DISPATCH_LEVEL
#define VERIFY_DISPATCH_IRQL(rc)				\
{												\
	KIRQL current_irql = KeGetCurrentIrql();	\
												\
	if(current_irql > DISPATCH_LEVEL){			\
		if(rc != SILENT){						\
			ERR_DBG_OUT(("Failed IRQL DISPATCH_LEVEL test! Current IRQL: 0x%X (%s).\n",	\
				current_irql, decode_irql(current_irql)));	\
		}			\
		rc = 1;		\
	}else{			\
		rc = 0;		\
	}				\
}

//verify that current IRQL is equal DISPATCH_LEVEL
#define VERIFY_IRQL_EQUAL_DISPATCH(rc)			\
{												\
	KIRQL current_irql = KeGetCurrentIrql();	\
												\
	if(current_irql != DISPATCH_LEVEL){			\
		ERR_DBG_OUT(("Failed VERIFY_IRQL_EQUAL_DISPATCH test! Current IRQL: 0x%X (%s).\n",	\
			current_irql, decode_irql(current_irql)));	\
		rc = 1;		\
	}else{			\
		rc = 0;		\
	}				\
}

//verify that current IRQL is HIGHER than DISPATCH_LEVEL
#define VERIFY_HIGHER_THAN_DISPATCH_IRQL(rc)				\
{												\
	KIRQL current_irql = KeGetCurrentIrql();	\
												\
	if(current_irql <= DISPATCH_LEVEL){			\
		ERR_DBG_OUT(("Failed VERIFY_HIGHER_THAN_DISPATCH_IRQL test! Current IRQL: 0x%X (%s).\n",	\
			current_irql, decode_irql(current_irql)));	\
		rc = 1;		\
	}else{			\
		rc = 0;		\
	}				\
}

//print current IRQL to debugger
#define PRINT_CURRENT_IRQL()						\
{													\
	KIRQL current_irql = KeGetCurrentIrql();		\
	DBG_IRQL_CHECK("%s(): Current IRQL: 0x%X (%s).\n",	\
		__FUNCTION__, current_irql, decode_irql(current_irql));	\
}

#endif//#ifndef _IRQL_CHECK_H
