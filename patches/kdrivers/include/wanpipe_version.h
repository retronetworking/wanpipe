#ifndef __WANPIPE_VERSION__
#define __WANPIPE_VERSION__


#define WANPIPE_COPYRIGHT_DATES "(c) 1994-2008"
#define WANPIPE_COMPANY         "Sangoma Technologies Inc"

/********** LINUX **********/
#define WANPIPE_VERSION			"3.3.6"
#define WANPIPE_SUB_VERSION		"0"
#define WANPIPE_VERSION_BETA		1
#define WANPIPE_LITE_VERSION		"1.1.1"

/********** FreeBSD **********/
#define WANPIPE_VERSION_FreeBSD		"3.2"
#define WANPIPE_SUB_VERSION_FreeBSD	"0"
#define WANPIPE_VERSION_BETA_FreeBSD	1
#define WANPIPE_LITE_VERSION_FreeBSD	"1.1.1"

/********** OpenBSD **********/
#define WANPIPE_VERSION_OpenBSD		"1.6.5"
#define WANPIPE_SUB_VERSION_OpenBSD	"8"
#define WANPIPE_VERSION_BETA_OpenBSD	1
#define WANPIPE_LITE_VERSION_OpenBSD	"1.1.1"

/********** NetBSD **********/
#define WANPIPE_VERSION_NetBSD		"1.1.1"
#define WANPIPE_SUB_VERSION_NetBSD	"5"
#define WANPIPE_VERSION_BETA_NetBSD	1

#if defined(__WINDOWS__)
/********** Windows **********/
typedef struct _DRIVER_VERSION {
	unsigned int major;
	unsigned int minor;
	unsigned int minor1; 
	unsigned int minor2; 
}DRIVER_VERSION, *PDRIVER_VERSION;

# define	WANPIPE_VERSION_MAJOR	6
# define	WANPIPE_VERSION_MINOR	0

#if 1
# define	WANPIPE_VERSION_MINOR1	4
# define	WANPIPE_VERSION_MINOR2	6
#else
# define	WANPIPE_VERSION_MINOR1	4
# define	WANPIPE_VERSION_MINOR2	4
#endif

static DRIVER_VERSION drv_version = {	WANPIPE_VERSION_MAJOR,
					WANPIPE_VERSION_MINOR, 
					WANPIPE_VERSION_MINOR1, 
					WANPIPE_VERSION_MINOR2
				};

#undef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTNAME_STR
#undef VER_COMPANYNAME_STR

#define VER_PRODUCTVERSION	6,0,4,6
#define VER_PRODUCTVERSION_STR	"6.0.4.6"
#define __BUILDDATE__		January 15, 2008

#define VER_COMPANYNAME_STR		"Sangoma Technologies Corporation"
#define VER_LEGALCOPYRIGHT_YEARS	"1984-2008"
#define VER_LEGALCOPYRIGHT_STR		"Copyright (c) Sangoma Technologies Corporation"
#define VER_PRODUCTNAME_STR		"Sangoma WANPIPE (TM)"

# undef WANPIPE_VERSION
# undef WANPIPE_VERSION_BETA
# undef WANPIPE_SUB_VERSION

# define WANPIPE_VERSION_Windows	"1.1.1"
# define WANPIPE_SUB_VERSION_Windows	"4"
# define WANPIPE_VERSION_BETA_Windows	1

# define WANPIPE_VERSION	WANPIPE_VERSION_Windows
# define WANPIPE_VERSION_BETA	WANPIPE_VERSION_BETA_Windows
# define WANPIPE_SUB_VERSION	WANPIPE_SUB_VERSION_Windows
#endif/* __WINDOWS__ */

#endif

