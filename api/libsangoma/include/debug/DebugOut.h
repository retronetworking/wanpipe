//////////////////////////////////////////////////////////////////////////////
//																			//
// Set of macros and routines for debugging purposes						//
//																			//
// Author: 	David Rohvarg  <davidr@sangoma.com>								//
//																			//
// ======================================================================== //
// Aprl 29, 2001	David Rokhvarg	Initial version.						//
// ************************************************************************ //
//////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG_OUT_H
#define _DEBUG_OUT_H

#include <stdio.h>
#include <stdarg.h>

/*
Note : PROGRAM_NAME or DRIVER_NAME should be defined before including this file.
e.g. #define PROGRAM_NAME "MyProgram : "
*/

#if defined(_DEBUG) || defined(_MYDEBUG)
////////////////////////////////////////////////////////////////////////////////////////////////////
//Old-style debugging macros to compile legacy code. Do NOT use in any new code.
//
#define Debug1( DBG, _s1 )			{											\
										if(DBG)									\
										{	OutputDebugString( PROGRAM_NAME );	\
											OutputDebugString( _s1 );			\
											OutputDebugString( "\n" );			\
										}										\
									}

#define Debug2( DBG, _s1, _s2 )		{												\
										if(DBG)										\
										{	char	szTempDebugString[MAX_PATH];	\
											OutputDebugString( PROGRAM_NAME );		\
											sprintf( szTempDebugString, _s1, _s2 );	\
											OutputDebugString( szTempDebugString );	\
											OutputDebugString( "\n" );				\
										}											\
									}

#define Debug3( DBG, _s1, _s2, _s3 ){														\
										if(DBG)												\
										{	char	szTempDebugString[MAX_PATH];			\
											OutputDebugString( PROGRAM_NAME );				\
											sprintf( szTempDebugString, _s1, _s2, _s3 );	\
											OutputDebugString( szTempDebugString );			\
											OutputDebugString( "\n" );						\
										}													\
									}

#define Debug4( DBG, _s1, _s2, _s3, _s4 ){														\
										if(DBG)													\
										{	char	szTempDebugString[MAX_PATH];				\
											OutputDebugString( PROGRAM_NAME );					\
											sprintf( szTempDebugString, _s1, _s2, _s3, _s4  );	\
											OutputDebugString( szTempDebugString );				\
											OutputDebugString( "\n" );							\
										}														\
									}

#define Debug5( DBG, _s1, _s2, _s3, _s4, _s5 ){													\
									if(DBG)														\
									{	char	szTempDebugString[MAX_PATH];					\
										OutputDebugString( PROGRAM_NAME );						\
										sprintf( szTempDebugString, _s1, _s2, _s3, _s4, _s5 );	\
										OutputDebugString( szTempDebugString );					\
										OutputDebugString( "\n" );								\
									}															\
								}
#else

#define Debug1( DBG, _s1 )
#define Debug2( DBG, _s1, _s2 )
#define Debug3( DBG, _s1, _s2, _s3 )
#define Debug4( DBG, _s1, _s2, _s3, _s4 )
#define Debug5( DBG, _s1, _s2, _s3, _s4, _s5 )

#endif//#ifdef _DEBUG

#if !defined(__KERNEL__)
////////////////////////////////////////////////////////////////////////////////////////////////////
//redirect debug output to a file, only for user mode applications
static void print_to_file(char *file_name, char *log_string)
{
	char file_path[MAX_PATH];
	FILE *stream;

	GetSystemDirectory(file_path, MAX_PATH);

	_snprintf(&file_path[strlen(file_path)], MAX_PATH - strlen(file_path), "\\%s", 
		file_name);

	stream = fopen(file_path, "a");
	if(stream == NULL){
		OutputDebugString("Failed to open log file!!\n");
		return;
	}

	fprintf(stream, "%s", log_string );
	fflush(stream);
	fclose (stream);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_DEBUG_STRING 500

//Print "program name" and a line of text.
static void Debug(unsigned int dbg_flag, void *pszFormat, ...)
{
	if(dbg_flag)
	{	static unsigned char str[MAX_DEBUG_STRING];
		va_list	vaArg;

		va_start (vaArg, pszFormat);
		_vsnprintf((char *)str, MAX_DEBUG_STRING, (char *)pszFormat, vaArg);
		va_end (vaArg);

#if defined(__KERNEL__)
		DbgPrint(DRIVER_NAME);		
		DbgPrint(str);
#else
 #if defined(REDIRECT_TO_FILE)
//#define FILE_NAME	"sangomalog.txt"
	print_to_file(FILE_NAME, (char *)str);

 #else
	OutputDebugString(  PROGRAM_NAME );
	OutputDebugString( (char *)str );
 #endif
#endif
	}
}

//Print a Line of text only, no "program name".
static void DebugL(unsigned char dbg_flag, void *pszFormat, ...)
{
	if(dbg_flag)
	{	static unsigned char	str[MAX_DEBUG_STRING];
		va_list	vaArg;

		va_start (vaArg, pszFormat);
		_vsnprintf((char *)str, MAX_DEBUG_STRING, (char *)pszFormat, vaArg);
		va_end (vaArg);

#if defined(__KERNEL__)
		DbgPrint(str);
#else
		OutputDebugString( (char *)str );
#endif
	}
}

#if !defined(__KERNEL__)

static void TIME_STAMP(UCHAR dbg_flag)
{	SYSTEMTIME st;	
	static char Time_Msg[400];
					
	GetLocalTime(&st);
	_snprintf(Time_Msg, 400, "\n\nDate: Y:%d, M:%d, D:%d. Time: H:%d, M:%d, S:%d\n",
		st.wYear, 
		st.wMonth, 
		st.wDay,
		st.wHour, 
		st.wMinute,
		st.wSecond);
	Debug(dbg_flag, Time_Msg);
}		

//print to console a line WITH program name
static void prn(unsigned int dbg_flag, void *pszFormat, ...)
{
	if(dbg_flag)
	{	static unsigned char str[MAX_DEBUG_STRING];
		va_list	vaArg;

		va_start (vaArg, pszFormat);
		_vsnprintf((char *)str, MAX_DEBUG_STRING, (char *)pszFormat, vaArg);
		va_end (vaArg);

		printf(PROGRAM_NAME);
		printf((char *)str);
	}
}

//print to console a line WITHOUT program name
static void prnl(unsigned int dbg_flag, void *pszFormat, ...)
{
	if(dbg_flag)
	{	static unsigned char str[MAX_DEBUG_STRING];
		va_list	vaArg;

		va_start (vaArg, pszFormat);
		_vsnprintf((char *)str, MAX_DEBUG_STRING, (char *)pszFormat, vaArg);
		va_end (vaArg);

		printf((char *)str);
	}
}

#endif	//#if !defined(__KERNEL__)

#endif	//_DEBUG_OUT_H
