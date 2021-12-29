/*************************************************************************
* wanpipe_abstr_types.h			WANPIPE(tm)			 *
*									 *
*	Wanpipe Kernel Abstraction type definitions	 		 *
*									 *
*									 *
* Author:	Alex Feldman <al.feldman@sangoma.com>			 *
*========================================================================*
* Jan 24, 2008	Alex Feldman	Initial version			 	 *
*************************************************************************/

#ifndef __WANPIPE_ABSTR_TYPES_H
# define __WANPIPE_ABSTR_TYPES_H



#if defined(__FreeBSD__)
/******************* F R E E B S D ******************************/
typedef int					wan_ticks_t; 
typedef unsigned long		ulong_ptr_t	
#elif defined(__OpenBSD__)
/******************* O P E N B S D ******************************/
typedef int					wan_ticks_t; 
typedef unsigned long		ulong_ptr_t	
#elif defined(__NetBSD__)
/******************* N E T B S D ******************************/
typedef int					wan_ticks_t; 
typedef unsigned long		ulong_ptr_t	
#elif defined(__LINUX__)
/*********************** L I N U X ******************************/
typedef unsigned long		wan_ticks_t; 
typedef unsigned long		ulong_ptr_t;	
typedef unsigned long		wan_time_t; 
typedef unsigned long		wan_suseconds_t; 
#elif defined(__WINDOWS__)
/******************* W I N D O W S ******************************/
typedef unsigned long		wan_ticks_t; 
typedef unsigned long		wan_time_t; 
typedef unsigned long		wan_suseconds_t; 
#endif




#endif	/* __WANPIPE_ABSTR_TYPES_H */
