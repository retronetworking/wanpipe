/*****************************************************************************
* wanpipe_version.h	WANPIPE(tm) Sangoma Device Driver.
******************************************************************************/

#ifndef __WANPIPE_VERSION__
#define __WANPIPE_VERSION__


#define WANPIPE_COPYRIGHT_DATES "(c) 1994-2010"
#define WANPIPE_COMPANY         "Sangoma Technologies Inc"

/********** LINUX **********/
#define WANPIPE_VERSION			"3.5.23"
#define WANPIPE_SUB_VERSION				"0"
#define WANPIPE_LITE_VERSION			"1.1.1"

#if defined(__LINUX__)
#define WANPIPE_VERSION_MAJOR			3
#define WANPIPE_VERSION_MINOR			5
#define WANPIPE_VERSION_MINOR1			22
#define WANPIPE_VERSION_MINOR2			1
#endif

/********** FreeBSD **********/
#define WANPIPE_VERSION_FreeBSD			"3.2"
#define WANPIPE_SUB_VERSION_FreeBSD		"2"
#define WANPIPE_VERSION_BETA_FreeBSD	1
#define WANPIPE_LITE_VERSION_FreeBSD	"1.1.1"

/********** OpenBSD **********/
#define WANPIPE_VERSION_OpenBSD			"1.6.5"
#define WANPIPE_SUB_VERSION_OpenBSD		"8"
#define WANPIPE_VERSION_BETA_OpenBSD	1
#define WANPIPE_LITE_VERSION_OpenBSD	"1.1.1"

/********** NetBSD **********/
#define WANPIPE_VERSION_NetBSD			"1.1.1"
#define WANPIPE_SUB_VERSION_NetBSD		"5"
#define WANPIPE_VERSION_BETA_NetBSD		1

#if defined(__WINDOWS__)

# define WP_BUILD_NBE41_HPBOA 0

# define	WANPIPE_VERSION_MAJOR	6	/* major upgrade */
# define	WANPIPE_VERSION_MINOR	0		
# if WP_BUILD_NBE41_HPBOA
#  define	WANPIPE_VERSION_MINOR1	42	/* frozen feature number */
#  define	WANPIPE_VERSION_MINOR2	5	/* patch number for WANPIPE_VERSION_MINOR1 */
# else
#  define	WANPIPE_VERSION_MINOR1	44	/* frozen feature number */
#  define	WANPIPE_VERSION_MINOR2	0	/* patch number for WANPIPE_VERSION_MINOR1 */
# endif

# undef	VER_PRODUCTVERSION
# undef VER_PRODUCTVERSION_STR
# undef VER_PRODUCTNAME_STR
# undef VER_COMPANYNAME_STR

# if WP_BUILD_NBE41_HPBOA
#  define VER_PRODUCTVERSION		6,0,42,5
#  define VER_PRODUCTVERSION_STR	"6.0.42.5"
# else
#  define VER_PRODUCTVERSION		6,0,44,0
#  define VER_PRODUCTVERSION_STR	"6.0.44.0"
# endif

# define __BUILDDATE__				May 10, 2011

# define VER_COMPANYNAME_STR		"Sangoma Technologies Corporation"
# define VER_LEGALCOPYRIGHT_YEARS	"1984-2011"
# define VER_LEGALCOPYRIGHT_STR		"Copyright (c) Sangoma Technologies Corp."
# define VER_PRODUCTNAME_STR		"Sangoma WANPIPE (TM)"

# undef WANPIPE_VERSION
# undef WANPIPE_VERSION_BETA
# undef WANPIPE_SUB_VERSION

# if WP_BUILD_NBE41_HPBOA
#  define WANPIPE_VERSION_Windows		"6.0.42"
#  define WANPIPE_SUB_VERSION_Windows	"5"
# else
#  define WANPIPE_VERSION_Windows		"6.0.44"
#  define WANPIPE_SUB_VERSION_Windows	"0"
# endif

# define WANPIPE_VERSION_BETA_Windows	0

# define WANPIPE_VERSION		WANPIPE_VERSION_Windows
# define WANPIPE_VERSION_BETA	WANPIPE_VERSION_BETA_Windows
# define WANPIPE_SUB_VERSION	WANPIPE_SUB_VERSION_Windows
#endif /* __WINDOWS__ */

#endif /* __WANPIPE_VERSION__ */

