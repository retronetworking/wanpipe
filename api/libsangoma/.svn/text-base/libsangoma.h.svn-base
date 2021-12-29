/*******************************************************************************//**
 * \file libsangoma.h
 * \brief Wanpipe API Library header for Sangoma AFT T1/E1/Analog/BRI/Serial Hardware -
 * \brief Provides User a Unified/OS Agnostic API to Wanpipe/Sangoma Drivers and Hardware
 *
 * Author(s):	Nenad Corbic <ncorbic@sangoma.com>
 *              David Rokhvarg <davidr@sangoma.com>
 *              Michael Jerris <mike@jerris.com>
 *		Anthony Minessale II <anthmct@yahoo.com>
 *
 * Copyright:	(c) 2005-2008 Nenad Corbic <ncorbic@sangoma.com>
 *
 * * Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Sangoma Technologies nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Sangoma Technologies ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Sangoma Technologies BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * ===============================================================================
 * v.2.0.0 Nenad Corbic
 * Jan 30 2009
 *		Added sangoma_get_driver_version, sangoma_get_firmware_version,
 *      sangoma_get_cpld_version functions,sangoma_get_stats,sangoma_flush_stats
 */

#ifndef _LIBSNAGOMA_H
#define _LIBSNAGOMA_H

#ifdef __cplusplus
extern "C" {	/* for C++ users */
#endif

#include <stdio.h>


/*!
  \def WANPIPE_TDM_API
  \brief Used by compiler and driver to enable TDM API
*/
#define WANPIPE_TDM_API 1

/*TODO: LIBSANGOMA_VERSION_CODE should be generated out of LIBSANGOMA_LT_CURRENT and friends in configure.in */

/*!
  \def LIBSANGOMA_VERSION
  \brief LibSangoma Macro to check the Version Number
*/
#define LIBSANGOMA_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

/*!
  \def LIBSANGOMA_VERSION_CODE
  \brief LibSangoma Current Version Number to be checked against the LIBSANGOMA_VERSION Macro 
*/
#define LIBSANGOMA_VERSION_CODE LIBSANGOMA_VERSION(3,0,0)

/*!
  \def LIBSANGOMA_VERSION_STR
  \brief LibSangoma Version in string format
*/
#define LIBSANGOMA_VERSION_STR "3.0.0"

#ifdef __COMPILING_LIBSANGOMA__
 struct sangoma_wait_obj;
 #define sangoma_wait_obj_t struct sangoma_wait_obj
#else
 typedef void sangoma_wait_obj_t;
#endif

#if defined(WIN32) || defined(WIN64)
#ifndef __WINDOWS__
#define __WINDOWS__
#endif
#include <windows.h>
#include <winioctl.h>
#include <conio.h>
#include <stddef.h>	
#include <stdlib.h>	

/*!
  \def _SAPI_CALL
  \brief libsangoma.dll functions exported as __cdecl calling convention
*/
#define _SAPI_CALL	__cdecl

/*!
  \def SANGOMA_INFINITE_API_POLL_WAIT (deprecated, use SANGOMA_WAIT_INFINITE instead)
  \brief Infinite poll timeout value 
*/
#define SANGOMA_INFINITE_API_POLL_WAIT INFINITE
#define SANGOMA_WAIT_INFINITE INFINITE

/*!
  \def sangoma_msleep(x)
  \brief milisecond sleep function
*/
#define sangoma_msleep(x)	Sleep(x)

#else
/* L I N U X */
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <poll.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>

/*!
  \def _SAPI_CALL
  \brief Not used in Linux
*/
#define _SAPI_CALL

/*!
  \def INVALID_HANDLE_VALUE
  \brief Invalid file handle value -1, Ported from Windows
*/
#define INVALID_HANDLE_VALUE -1

/*!
  \def SANGOMA_INFINITE_API_POLL_WAIT (deprecated, use SANGOMA_WAIT_INFINITE instead)
  \brief Infinite poll timeout value -1, Ported from Windows
*/
#define SANGOMA_INFINITE_API_POLL_WAIT -1
#define SANGOMA_WAIT_INFINITE -1

/*!
  \def __cdecl
  \brief Ported from Windows
  \typedef BOOL
  \brief Boolean type int, Ported from Windows
  \typedef DWORD
  \brief DWORD type is int, Ported from Windows
  \def FALSE
  \brief FALSE value is 0, Ported from Windows
  \def TRUE
  \brief TRUE value is 1, Ported from Windows
  \def sangoma_msleep(x)
  \brief milisecond sleep function
  \def _getch
  \brief get character, Ported from Windows
  \def Sleep
  \brief milisecond sleep function
  \typedef HANDLE
  \brief file handle type int, Ported from Windows
  \typedef TCHAR
  \brief TCHAN type mapped to char, Ported from Windows
  \typedef ULONG
  \brief ULONG type mapped to unsigned long, Ported from Windows
  \typedef UCHAR
  \brief ULONG type mapped to unsigned char, Ported from Windows
  \typedef USHORT
  \brief USHORT type mapped to unsigned short, Ported from Windows
  \typedef LPSTR
  \brief LPSTR type mapped to unsigned char *, Ported from Windows
  \typedef PUCHAR
  \brief PUCHAR type mapped to unsigned char *, Ported from Windows
  \typedef LPTHREAD_START_ROUTINE
  \brief LPTHREAD_START_ROUTINE type mapped to unsigned char *, Ported from Windows
  \def _stricmp
  \brief _stricmp type mapped to _stricmp, Ported from Windows
  \def _snprintf
  \brief _snprintf type mapped to snprintf, Ported from Windows
*/

#define __cdecl

#ifndef FALSE
#define FALSE		0
#endif

#ifndef TRUE
#define TRUE		1
#endif

#define sangoma_msleep(x) usleep(x*1000)
#define _getch		getchar
#define Sleep 		sangoma_msleep
#define _stricmp   	strcmp
#define _snprintf	snprintf
#define _vsnprintf  	vsnprintf

typedef int HANDLE;
typedef int BOOL;
typedef int DWORD;
typedef char TCHAR;
typedef unsigned char UCHAR;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char * LPSTR;
typedef unsigned char * PUCHAR;
typedef void * LPTHREAD_START_ROUTINE;
typedef pthread_mutex_t CRITICAL_SECTION;

#define EnterCriticalSection(arg) 	pthread_mutex_lock(arg)
#define LeaveCriticalSection(arg) 	pthread_mutex_unlock(arg)
#define InitializeCriticalSection(arg) pthread_mutex_init(arg, NULL);

typedef struct tm SYSTEMTIME;
typedef char * LPCTSTR;
               
#endif/* WIN32 */


/*!
  LIBSANGOMA_LIGHT can be used to enable only IO and EVENT
  libsangoma functions.  The DRIVER configuration/start/stop
  functions are not compiled.

  LIBSANGOMA_LIGHT depends only on 3 header files. Instead
  of all wanpipe header files needed for DRIVER management

  LIBSANGMOA_LIGHT is NOT enabled by default.
*/

#ifdef LIBSANGOMA_LIGHT
#include "wanpipe_api_iface.h"
#include "wanpipe_api_hdr.h"
#include "sdla_te1.h"
#include "wanpipe_events.h"
#include "wanpipe_api_deprecated.h"
#else
#include "wanpipe_api.h"
#endif

/*!
 * As of now this typedef maps exactly to SANG_STATUS_T, however that
 * is a kernel type, ugly, ugly, uglyyyyy, we should have strictly
 * minimum set of shared data structures between kernel and user
 * many return codes specified in SANG_STATUS_T are kernel specific
 * like FAILED_TO_LOCK_USER_MEMORY or INVALID_IRQL, the libsangoma
 * user does not need that much information, and even if ever needs
 * it we should provide simpler defaults
 * \brief return status from sangoma APIs
 */
typedef int32_t sangoma_status_t;

/*!
  \def FNAME_LEN
  \brief string length of a file name
  \def FUNC_DBG(x)
  \brief function debug print function
  \def DBG_PRINT
  \brief debug print function
*/
#define FNAME_LEN		100
#define FUNC_DBG(x)		if(0)printf("%s():%d\n", x, __LINE__)
#define DBG_PRINT		if(1)printf

/*!
  \typedef sangoma_api_hdr_t
  \brief Backward comaptible define of wp_api_hdr_t
*/
typedef wp_api_hdr_t sangoma_api_hdr_t;

/*!
  \enum _sangoma_wait_obj_type_t
  \brief Wait object type definition
  \typedef sangoma_wait_obj_type_t
  \brief Wait object type definition
*/
typedef enum _sangoma_wait_obj_type
{
	/*! \brief deprecated, use SANGOMA_GENERIC_WAIT_OBJ */
	UNKNOWN_WAIT_OBJ = 0, 
	/*! \brief Generic object that can be signaled but is not associated to any sangoma device */
	SANGOMA_GENERIC_WAIT_OBJ = 0, 
	/*!  \brief Sangoma object associated to some device which cannot be signaled (cannot call sangoma_wait_obj_signal on it) */
	SANGOMA_DEVICE_WAIT_OBJ, 
	/*!  \brief Sangoma object that is associated to a device AND can be signaled */
	SANGOMA_DEVICE_WAIT_OBJ_SIG, 
} sangoma_wait_obj_type_t;

#define DECODE_SANGOMA_WAIT_OBJECT_TYPE(type)\
	type == SANGOMA_GENERIC_WAIT_OBJ	? "SANGOMA_GENERIC_WAIT_OBJ"	:\
	type == SANGOMA_DEVICE_WAIT_OBJ		? "SANGOMA_DEVICE_WAIT_OBJ"		:\
	type == SANGOMA_DEVICE_WAIT_OBJ_SIG ? "SANGOMA_DEVICE_WAIT_OBJ_SIG"	:\
	"Invalid Wait Object type!"

/************************************************************//**
 * Device OPEN / CLOSE Functions
 ***************************************************************/

/*!
  \fn sng_fd_t sangoma_open_api_span_chan(int span, int chan)
  \brief Open a Device based on Span/Chan values
  \param span span number starting from 1 to 255
  \param chan chan number starting from 1 to 32
  \return File Descriptor: -1 error, 0 or positive integer: valid file descriptor

  Restriced open, device will allowed to be open only once.
*/
sng_fd_t _SAPI_CALL sangoma_open_api_span_chan(int span, int chan);


/*!
  \fn sng_fd_t __sangoma_open_api_span_chan(int span, int chan)
  \brief Open a Device based on Span/Chan values
  \param span span number starting from 1 to 255
  \param chan chan number starting from 1 to 32
  \return File Descriptor: -1 error, 0 or positive integer: valid file descriptor

  Unrestriced open, allows mutiple open calls on a single device 
*/
sng_fd_t _SAPI_CALL __sangoma_open_api_span_chan(int span, int chan);
#define __sangoma_open_tdmapi_span_chan __sangoma_open_api_span_chan

/*!
  \fn sng_fd_t sangoma_open_tdmapi_span(int span)
  \brief Open a first available device on a Span
  \param span span number starting from 1 to 255
  \return File Descriptor: -1 error, 0 or positive integer: valid file descriptor

  Unrestriced open, allows mutiple open calls on a single device 
*/
sng_fd_t _SAPI_CALL sangoma_open_api_span(int span);


/*!
  \def sangoma_create_socket_intr
  \brief Backward compatible open span chan call
*/



/*!
  \def LIBSANGOMA_TDMAPI_CTRL
  \brief Global control device feature
*/
#ifndef LIBSANGOMA_TDMAPI_CTRL
#define LIBSANGOMA_TDMAPI_CTRL 1
#endif

/*!
  \fn sng_fd_t sangoma_open_api_ctrl(void)
  \brief Open a Global Control Device 
  \return File Descriptor - negative=error  0 or greater = fd

  The global control device receives events for all devices
  configured.
*/
sng_fd_t _SAPI_CALL sangoma_open_api_ctrl(void);


/*!
  \fn sng_fd_t sangoma_open_driver_ctrl(int port_no)
  \brief Open a Global Driver Control Device
  \return File Descriptor - negative=error  0 or greater = fd

  The global control device receives events for all devices
  configured.
*/
sng_fd_t _SAPI_CALL sangoma_open_driver_ctrl(int port_no);



/*!
  \fn void sangoma_close(sng_fd_t *fd)
  \brief Close device file descriptor
  \param fd device file descriptor
  \return void

*/
void _SAPI_CALL sangoma_close(sng_fd_t *fd);



/*!
  \fn int sangoma_get_open_cnt(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Get device open count
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return negative or 0: error,  greater than 1 : open count
*/

int _SAPI_CALL sangoma_get_open_cnt(sng_fd_t fd, wanpipe_api_t *tdm_api);


/************************************************************//**
 * Device READ / WRITE Functions
 ***************************************************************/

/*!
  \fn int sangoma_writemsg(sng_fd_t fd, void *hdrbuf, int hdrlen, void *databuf, unsigned short datalen, int flag)
  \brief Write Data to device
  \param fd device file descriptor 
  \param hdrbuf pointer to header structure wp_api_hdr_t 
  \param hdrlen size of wp_api_hdr_t
  \param databuf pointer to data buffer to be transmitted
  \param datalen length of data buffer
  \param flag currently not used, set to 0
  \return transmit size, must be equal to datalen, anything else is error

   In case of error return code, one must check the header operation_status
   variable to identify the reason of an error. Please refer to the error codes.
*/

int _SAPI_CALL sangoma_writemsg(sng_fd_t fd, void *hdrbuf, int hdrlen,
						 void *databuf, unsigned short datalen, int flag);


/*!
  \fn int sangoma_readmsg(sng_fd_t fd, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag)
  \brief Read Data from device
  \param fd device file descriptor 
  \param hdrbuf pointer to header structure wp_api_hdr_t 
  \param hdrlen size of wp_api_hdr_t
  \param databuf pointer to data buffer to be received
  \param datalen length of data buffer
  \param flag currently not used, set to 0
  \return received size, must be equal to datalen, anything else is error or busy
   
  In case of error return code, one must check the header operation_status
  variable to identify the reason of error. Please refer to the error codes.
*/
int _SAPI_CALL sangoma_readmsg(sng_fd_t fd, void *hdrbuf, int hdrlen,
						void *databuf, int datalen, int flag);




/************************************************************//**
 * Device POLL Functions
 ***************************************************************/ 

/*!
  \fn sangoma_status_t _SAPI_CALL sangoma_waitfor(sangoma_wait_obj_t *sangoma_wait_obj, int32_t inflags, int32_t *outflags, int32_t timeout)
  \brief Wait for a single waitable object
  \param sangoma_wait_obj pointer to array of file descriptors to wait for
  \param timeout timeout in miliseconds in case of no event
  \return SANG_STATUS_APIPOLL_TIMEOUT: timeout, SANG_STATUS_SUCCESS: event occured use sangoma_wait_obj_get_out_flags to check or error status
*/
sangoma_status_t _SAPI_CALL sangoma_waitfor(sangoma_wait_obj_t *sangoma_wait_obj, uint32_t inflags, uint32_t *outflags, int32_t timeout);

/*!
  \fn sangoma_status_t _SAPI_CALL sangoma_waitfor_many(sangoma_wait_obj_t *sangoma_wait_objects[], int32_t in_flags[], int32_t out_flags[] uint32_t number_of_sangoma_wait_objects, int32_t system_wait_timeout);
  \brief Wait for multiple sangoma waitable objects 
  \param sangoma_wait_objects pointer to array of wait objects previously created with sangoma_wait_obj_create
  \param in_flags array of flags corresponding to the flags the user is interested on for each waitable object
  \param out_flags array of flags corresponding to the flags that are ready in the waitable objects 
  \param number_of_sangoma_wait_objects size of the array of waitable objects and flags
  \param system_wait_timeout timeout in miliseconds in case of no event
  \return negative: SANG_STATUS_APIPOLL_TIMEOUT: timeout, SANG_STATUS_SUCCESS: event occured check in sangoma_wait_objects, any other return code is some error
*/
sangoma_status_t _SAPI_CALL sangoma_waitfor_many(sangoma_wait_obj_t *sangoma_wait_objects[], uint32_t in_flags[], uint32_t out_flags[],
		uint32_t number_of_sangoma_wait_objects, int32_t system_wait_timeout);

/*!
  Users should be careful when using this function. You should just call this function once per sangoma device. If you attempt to use this function
  twice on the same device (even if different sng_fd_t are used) you will end up with a waitable object that no longer gets notified about events
  This is a fair limitation though, since there is no point on creating more than one waitable object per sangoma device. Even if you have one thread
  for signaling, one for reading and one for writing, the 3 threads can use the same sangoma waitable object. However you can delete the waitable
  object and create a new one if you need it.
  \fn sangoma_status_t sangoma_wait_obj_create(sangoma_wait_obj_t **sangoma_wait_object, sng_fd_t fd, sangoma_wait_obj_type_t object_type)
  \brief Create a wait object that will be used with sangoma_waitfor_many() API
  \param sangoma_wait_object pointer a single device object 
  \param fd device file descriptor
  \param object_type type of the wait object. see sangoma_wait_obj_type_t for types
  \see sangoma_wait_obj_type_t
  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _SAPI_CALL sangoma_wait_obj_create(sangoma_wait_obj_t **sangoma_wait_object, sng_fd_t fd, sangoma_wait_obj_type_t object_type);

/*!
  \fn sangoma_status_t sangoma_wait_obj_delete(sangoma_wait_obj_t **sangoma_wait_object)
  \brief De-allocate all resources inside a wait object which were allocated by sangoma_wait_obj_init().
  \param sangoma_wait_object pointer to a pointer to a single device object
  \return SANG_STATUS_SUCCESS on success or some sangoma status error
*/
sangoma_status_t _SAPI_CALL sangoma_wait_obj_delete(sangoma_wait_obj_t **sangoma_wait_object);

/*!
  \fn void sangoma_wait_obj_signal(sangoma_wait_obj_t *sangoma_wait_object)
  \brief Set wait object to a signaled state
  \param sangoma_wait_object pointer a single device object that can be signaled
  \return sangoma_status_t
*/
sangoma_status_t _SAPI_CALL sangoma_wait_obj_signal(sangoma_wait_obj_t *sangoma_wait_object);

/*!
  \fn sng_fd_t sangoma_wait_obj_get_fd(sangoma_wait_obj_t *sangoma_wait_object)
  \brief Get fd device file descriptor which was the 'fd' parameter for sangoma_wait_obj_create(), not useful for generic objects
  \param sangoma_wait_object pointer a single device object 
  \return sng_fd_t - device file descriptor
*/
sng_fd_t _SAPI_CALL sangoma_wait_obj_get_fd(sangoma_wait_obj_t *sangoma_wait_object);

/*!
  \fn void sangoma_wait_obj_set_context(sangoma_wait_obj_t *sangoma_wait_object)
  \brief Store the given context into provided sangoma wait object.
  \brief This function is useful to associate a context with a sangoma wait object.
  \param sangoma_wait_object pointer a single device object 
  \param context void pointer to user context
  \return void
*/
void _SAPI_CALL sangoma_wait_obj_set_context(sangoma_wait_obj_t *sangoma_wait_object, void *context);

/*!
  \fn void *sangoma_wait_obj_get_context(sangoma_wait_obj_t *sangoma_wait_object)
  \brief Retrieve the user context (if any) that was set via sangoma_wait_obj_set_context.
  \param sangoma_wait_object pointer a single device object 
  \return void*
*/
void* _SAPI_CALL sangoma_wait_obj_get_context(sangoma_wait_obj_t *sangoma_wait_object);


/************************************************************//**
 * Device API COMMAND Functions
 ***************************************************************/ 

/*!
  \fn int sangoma_cmd_exec(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Execute Sangoma API Command
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_cmd_exec(sng_fd_t fd, wanpipe_api_t *tdm_api);


/*!
  \fn int sangoma_get_full_cfg(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Read tdm api device configuration
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_get_full_cfg(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_set_usr_period(sng_fd_t fd, wanpipe_api_t *tdm_api, int period)
  \brief Set Tx/Rx Period in Milliseconds 
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param period value in miliseconds (1,2,5,10)
  \return non-zero: error, 0: ok
  
  Only valid in CHAN Operation Mode
*/
int _SAPI_CALL sangoma_tdm_set_usr_period(sng_fd_t fd, wanpipe_api_t *tdm_api, int period);

/*!
  \fn int sangoma_tdm_get_usr_period(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Get Tx/Rx Period in Milliseconds
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return negative: error or configured period value
*/
int _SAPI_CALL sangoma_tdm_get_usr_period(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_get_usr_mtu_mru(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Get Tx/Rx MTU/MRU in bytes
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return negative: error or configured mtu/mru in bytes
*/
int _SAPI_CALL sangoma_tdm_get_usr_mtu_mru(sng_fd_t fd, wanpipe_api_t *tdm_api);


/*!
  \fn int sangoma_flush_bufs(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Flush buffers from current channel
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
*/
int _SAPI_CALL sangoma_flush_bufs(sng_fd_t fd, wanpipe_api_t *tdm_api);


/*!
  \fn int sangoma_tdm_enable_rbs_events(sng_fd_t fd, wanpipe_api_t *tdm_api, int poll_in_sec)
  \brief Enable RBS Events on a device
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param poll_in_sec driver poll period for rbs events
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_tdm_enable_rbs_events(sng_fd_t fd, wanpipe_api_t *tdm_api, int poll_in_sec);

/*!
  \fn int sangoma_tdm_disable_rbs_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Disable RBS Events for a device
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_tdm_disable_rbs_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_write_rbs(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel, unsigned char rbs)
  \brief Write RBS Bits on a device
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param channel t1/e1 timeslot
  \param rbs rbs bits (ABCD)
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_tdm_write_rbs(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel, unsigned char rbs);

/*!
  \fn int sangoma_tdm_read_rbs(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel, unsigned char *rbs)
  \brief Read RBS Bits on a device
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param channel t1/e1 timeslot
  \param rbs pointer to rbs bits (ABCD) 
  \return non-zero: error, 0: ok
*/

int _SAPI_CALL sangoma_tdm_read_rbs(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel, unsigned char *rbs);

/*!
  \fn int sangoma_tdm_enable_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Enable DTMF Detection on Octasic chip (if hw supports it)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on cards that have HWEC
*/
int _SAPI_CALL sangoma_tdm_enable_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_disable_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Disable DTMF Detection on Octasic chip (if hw supports it)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on cards that have HWEC
*/
int _SAPI_CALL sangoma_tdm_disable_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api);


/*!
  \fn int sangoma_tdm_enable_rm_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Enable DTMF Detection on Analog/Remora SLIC Chip
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_enable_rm_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_disable_rm_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Disable DTMF Detection on Analog/Remora SLIC Chip
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_disable_rm_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_enable_rxhook_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Enable RX HOOK Events (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_enable_rxhook_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_disable_rxhook_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Disable RX HOOK Events (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_disable_rxhook_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_enable_ring_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Enable RING Events (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_enable_ring_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_disable_ring_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Disable RING Events (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_disable_ring_events(sng_fd_t fd, wanpipe_api_t *tdm_api);


/*!
  \fn int sangoma_tdm_enable_ring_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Enable RING DETECT Events (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_enable_ring_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_disable_ring_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Disable RING DETECT Events (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_disable_ring_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_enable_ring_trip_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Enable RING TRIP Events (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_enable_ring_trip_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_disable_ring_trip_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Disable RING TRIP Events (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_disable_ring_trip_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_enable_tone_events(sng_fd_t fd, wanpipe_api_t *tdm_api, uint16_t tone_id)
  \brief Transmit a TONE on this device (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param tone_id tone type to transmit 
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_enable_tone_events(sng_fd_t fd, wanpipe_api_t *tdm_api, uint16_t tone_id);

/*!
  \fn int sangoma_tdm_disable_tone_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Enable TONE Events (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_disable_tone_events(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_txsig_onhook(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Tranmsmit TX SIG ON HOOK (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_txsig_onhook(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_txsig_offhook(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Tranmsmit TX SIG OFF HOOK (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_txsig_offhook(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_txsig_start(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Tranmsmit TX SIG START (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_txsig_start(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_txsig_kewl(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Tranmsmit TX SIG KEWL START (Analog Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on Analog Cards
*/
int _SAPI_CALL sangoma_tdm_txsig_kewl(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_enable_hwec(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Enable HWEC on this channel 
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on cards that have HWEC
*/
int _SAPI_CALL sangoma_tdm_enable_hwec(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_tdm_disable_hwec(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Disable HWEC on this channel
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  Supported only on cards that have HWEC
*/
int _SAPI_CALL sangoma_tdm_disable_hwec(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int _SAPI_CALL sangoma_tdm_get_fe_alarms(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned int *alarms);
  \brief Get Front End Alarms (T1/E1 Only)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param alarms bit map status of T1/E1 alarms 
  \return non-zero: error, 0: ok
  
  Supported only on T1/E1 Cards
*/
int _SAPI_CALL sangoma_tdm_get_fe_alarms(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned int *alarms);



#ifdef WP_API_FEATURE_LINK_STATUS
# ifndef LIBSANGOMA_GET_LINKSTATUS
/*!
  \def LIBSANGOMA_GET_LINKSTATUS
  \brief Get Link Status feature 
*/
# define LIBSANGOMA_GET_LINKSTATUS 1
# endif

/*!
  \fn int sangoma_get_link_status(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *current_status)
  \brief Get Device Link Status (Connected/Disconnected)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param current_status pointer where result will be filled: 0=Link UP 1=Link Down
  \return non-zero: error, 0: ok -> check current_status 

*/
int _SAPI_CALL sangoma_get_link_status(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *current_status);

#endif

/* set current Line Connection state - Connected/Disconnected */
#ifndef LIBSANGOMA_GET_FESTATUS
/*!
  \def LIBSANGOMA_GET_FESTATUS
  \brief Get Front End Status feature
*/
#define LIBSANGOMA_GET_FESTATUS 1
#endif

/*!
  \fn int sangoma_set_fe_status(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char new_status)
  \brief Set Device Link Status (Connected/Disconnected) 
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param new_status new status  0=Link UP  1=Link Down
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_set_fe_status(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char new_status);


/*!
  \fn int _SAPI_CALL sangoma_enable_bri_bchan_loopback(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel)
  \brief Enable BRI Bchannel loopback - used when debugging bri device
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param channel bri bchannel 1 or 2
  \return non-zero: error, 0: ok
 
*/
int _SAPI_CALL sangoma_enable_bri_bchan_loopback(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel);

/*!
  \fn int _SAPI_CALL sangoma_disable_bri_bchan_loopback(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel)
  \brief Disable BRI Bchannel loopback - used when debugging bri device
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param channel bri bchannel 1 or 2
  \return non-zero: error, 0: ok
 
*/
int _SAPI_CALL sangoma_disable_bri_bchan_loopback(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel);


/*!
  \fn int sangoma_get_tx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Get Tx Queue Size for this channel
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_get_tx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api);


/*!
  \fn int sangoma_set_tx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api, int size)
  \brief Get Tx Queue Size for this channel
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param size tx queue size (minimum value of 1) 
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_set_tx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api, int size);

/*!
  \fn int sangoma_get_rx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Get Rx Queue Size for this channel
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_get_rx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_set_rx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api, int size)
  \brief Get Tx Queue Size for this channel
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param size rx queue size (minimum value of 1)
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_set_rx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api, int size);


#ifndef LIBSANGOMA_GET_HWCODING
/*!
  \def LIBSANGOMA_GET_HWCODING
  \brief Get HW Coding Feature
*/
#define LIBSANGOMA_GET_HWCODING 1
#endif

/*!
  \fn int sangoma_get_hw_coding(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Get HW Voice Coding (ulaw/alaw)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok

  This function will return the low level voice coding
  depending on configuration.  (ulaw or alaw)
*/
int _SAPI_CALL sangoma_get_hw_coding(sng_fd_t fd, wanpipe_api_t *tdm_api);



#ifndef LIBSANGOMA_GET_HWDTMF
/*!
  \def LIBSANGOMA_GET_HWDTMF
  \brief HW DTMF Feature
*/
#define LIBSANGOMA_GET_HWDTMF 1
#endif
/*!
  \fn int sangoma_tdm_get_hw_dtmf(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Check if hwdtmf support is available
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok

  This function will check if hw supports HW DTMF.
*/
int _SAPI_CALL sangoma_tdm_get_hw_dtmf(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_span_chan_toif(int span, int chan, char *interface_name)
  \brief Convert Span & Chan to interface name
  \param span span number starting from 1 to 255
  \param chan chan number starting from 1 to 32
  \param interface_name pointer to string where interface name will be written
  \return non-zero = error, 0 = ok
*/
int _SAPI_CALL sangoma_span_chan_toif(int span, int chan, char *interface_name);

/*!
  \fn int sangoma_span_chan_fromif(char *interface_name, int *span, int *chan)
  \brief Convert Interace Name to Span & Chan 
  \param interface_name pointer to string containing interface name
  \param span integer pointer where to write span value
  \param chan integer pointer where to write chan value
  \return non-zero = error, 0 = ok
*/
int _SAPI_CALL sangoma_span_chan_fromif(char *interface_name, int *span, int *chan);


/*!
  \fn int sangoma_interface_wait_up(int span, int chan, int sectimeout)
  \brief Wait for a sangoma device to come up (ie: Linux wait for /dev/wanpipex_1 to come up)
  \param span span number of the device to wait
  \param chan chan number of the device to wait
  \param sectimeout how many seconds to wait for the device to come up, -1 to wait forever
  \return non-zero = error, 0 = ok
*/
int _SAPI_CALL sangoma_interface_wait_up(int span, int chan, int sectimeout);

/*!
  \fn int sangoma_get_driver_version(sng_fd_t fd, wanpipe_api_t *tdm_api, wan_driver_version_t *drv_ver)
  \brief Get Device Driver Version Number
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param drv_ver driver version structure that will contain the driver version
  \return non-zero = error, 0 = ok
*/
int _SAPI_CALL sangoma_get_driver_version(sng_fd_t fd, wanpipe_api_t *tdm_api, wan_driver_version_t *drv_ver);

/*!
  \fn int sangoma_get_firmware_version(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *ver)
  \brief Get Hardware/Firmware Version
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param ver hardware/firmware version number
  \return non-zero = error, 0 = ok
*/
int _SAPI_CALL sangoma_get_firmware_version(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *ver);

/*!
  \fn int sangoma_get_cpld_version(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *ver)
  \brief Get Hardare/CPLD Version
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param ver hardware/cpld version number
  \return non-zero = error, 0 = ok
*/
int _SAPI_CALL sangoma_get_cpld_version(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *ver);


/*!
  \fn int sangoma_get_stats(sng_fd_t fd, wanpipe_api_t *tdm_api, wanpipe_chan_stats_t *stats)
  \brief Get Device Statistics. Statistics will be available in tdm_api->wp_cmd.stats structure.
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param stats stats structure will be filled with device stats. (Optional, can be left NULL)
  \return non-zero = error, 0 = ok
*/
int _SAPI_CALL sangoma_get_stats(sng_fd_t fd, wanpipe_api_t *tdm_api, wanpipe_chan_stats_t *stats);

/*!
  \fn int _SAPI_CALL sangoma_flush_stats(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Flush/Reset device statistics
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero = error, 0 = ok
*/
int _SAPI_CALL sangoma_flush_stats(sng_fd_t fd, wanpipe_api_t *tdm_api);

/*!
  \fn int sangoma_set_rm_rxflashtime(sng_fd_t fd, wanpipe_api_t *tdm_api, int rxflashtime)
  \brief Set rxflashtime for FXS module Wink-Flash Event
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param rxflashtime time value 
  \return non-zero = error, 0 = ok
*/
int _SAPI_CALL sangoma_set_rm_rxflashtime(sng_fd_t fd, wanpipe_api_t *tdm_api, int rxflashtime);



/************************************************************//**
 * Device EVENT Function
 ***************************************************************/ 


/*!
  \fn int sangoma_read_event(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Read API Events
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return non-zero: error, 0: ok
  
  The TDM API structure will be populated with a TDM API or WAN Event.
  This function usually used after wait() function indicated that event
  has occured.
*/
int _SAPI_CALL sangoma_read_event(sng_fd_t fd, wanpipe_api_t *tdm_api);



#ifndef LIBSANGOMA_LIGHT

/************************************************************//**
 * Device PORT Control Functions
 ***************************************************************/ 

int _SAPI_CALL sangoma_driver_port_start(sng_fd_t fd, port_management_struct_t *port_mgmnt, unsigned short port_no);
int _SAPI_CALL sangoma_driver_port_stop(sng_fd_t fd, port_management_struct_t *port_mgmnt, unsigned short port_no);
int _SAPI_CALL sangoma_driver_port_set_config(sng_fd_t fd, port_cfg_t *port_cfg, unsigned short port_no);
int _SAPI_CALL sangoma_driver_port_get_config(sng_fd_t fd, port_cfg_t *port_cfg, unsigned short port_no);
int _SAPI_CALL sangoma_driver_get_hw_info(sng_fd_t fd, port_management_struct_t *port_mgmnt, unsigned short port_no);


/************************************************************//**
 * Device MANAGEMENT Functions
 ***************************************************************/ 

/*!
  \fn int sangoma_mgmt_cmd(sng_fd_t fd, wan_udp_hdr_t* wan_udp)
  \brief Execute Sangoma Management Command
  \param fd device file descriptor
  \param wan_udp management command structure
  \return non-zero: error, 0: ok
*/
int _SAPI_CALL sangoma_mgmt_cmd(sng_fd_t fd, wan_udp_hdr_t* wan_udp);


#endif  /* LIBSANGOMA_LIGHT */


/*================================================================
 * DEPRECATED Function Calls - Not to be used any more
 * Here for backward compatibility 
 *================================================================*/


#ifndef LIBSANGOMA_SET_FESTATUS
/*!
  \def LIBSANGOMA_SET_FESTATUS
  \brief Set Front End Status Feature 
*/
#define LIBSANGOMA_SET_FESTATUS 1
#endif

/*!
  \fn int sangoma_get_fe_status(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *current_status)
  \brief Get Device Link Status (Connected/Disconnected)
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param current_status pointer where result will be filled: 0=Link UP 1=Link Down
  \return non-zero: error, 0: ok -> check current_status 

  Deprecated - replaced by sangoma_tdm_get_link_status function 
*/
int _SAPI_CALL sangoma_get_fe_status(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *current_status);



/*!
  \fn int sangoma_tdm_set_codec(sng_fd_t fd, wanpipe_api_t *tdm_api, int codec)
  \brief Set TDM Codec per chan
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param codec codec to set (ulaw/alaw/slinear)
  \return non-zero: error, 0: ok
  
  Deprecated Function - Here for backward compatibility
  Only valid in CHAN Operation Mode
*/
int _SAPI_CALL sangoma_tdm_set_codec(sng_fd_t fd, wanpipe_api_t *tdm_api, int codec);

/*!
  \fn int sangoma_tdm_get_codec(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Get Configured TDM Codec per chan
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return negative: error or configured codec value
  
  Deprecated Function - Here for backward compatibility
  Only valid in CHAN Operation Mode
*/
int _SAPI_CALL sangoma_tdm_get_codec(sng_fd_t fd, wanpipe_api_t *tdm_api);


/*!
  \fn sng_fd_t sangoma_create_socket_by_name(char *device, char *card)
  \brief Open a device based on a interface and card name
  \param device interface name
  \param card card name
  \return File Descriptor: -1 error, 0 or positive integer: valid file descriptor

   Deprecated - here for backward compatibility
*/
sng_fd_t _SAPI_CALL sangoma_create_socket_by_name(char *device, char *card);

/*!
  \fn int sangoma_interface_toi(char *interface_name, int *span, int *chan)
  \brief Convert Span & Chan to interface name
  \param interface_name pointer to string where interface name will be written
  \param span span number starting from 1 to 255
  \param chan chan number starting from 1 to 32
  \return non-zero = error, 0 = ok
  Deprecated - here for backward compatibility
*/
int _SAPI_CALL sangoma_interface_toi(char *interface_name, int *span, int *chan);


/*!
  \fn int sangoma_tdm_set_power_level(sng_fd_t fd, wanpipe_api_t *tdm_api, int power)
  \brief Set Power Level - so only data matching the power level would be passed up.
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \param power value of power 
  \return non-zero: error, 0: ok
  
  Deprecated - not used/implemented
*/
int _SAPI_CALL sangoma_tdm_set_power_level(sng_fd_t fd, wanpipe_api_t *tdm_api, int power);

/*!
  \fn int sangoma_tdm_get_power_level(sng_fd_t fd, wanpipe_api_t *tdm_api)
  \brief Get Configured Power Level
  \param fd device file descriptor
  \param tdm_api tdm api command structure
  \return negative: error or configured power level
  
  Deprecated - not used/implemented
*/
int _SAPI_CALL sangoma_tdm_get_power_level(sng_fd_t fd, wanpipe_api_t *tdm_api);


#ifdef __cplusplus
}
#endif

/*! Backward compabile defines */
#if !defined(__WINDOWS__)
#define sangoma_open_tdmapi_span_chan sangoma_open_api_span_chan
#define sangoma_open_tdmapi_span sangoma_open_api_span
#define sangoma_open_tdmapi_ctrl sangoma_open_api_ctrl
#define sangoma_tdm_get_fe_status sangoma_get_fe_status
#define sangoma_socket_close sangoma_close
#define sangoma_tdm_get_hw_coding sangoma_get_hw_coding
#define sangoma_tdm_set_fe_status sangoma_set_fe_status
#define sangoma_tdm_get_link_status sangoma_get_link_status
#define sangoma_tdm_flush_bufs sangoma_flush_bufs
#define sangoma_tdm_cmd_exec sangoma_cmd_exec
#define sangoma_tdm_read_event sangoma_read_event
#define sangoma_readmsg_tdm sangoma_readmsg
#define sangoma_readmsg_socket sangoma_readmsg
#define sangoma_sendmsg_socket sangoma_writemsg
#define sangoma_writemsg_tdm sangoma_writemsg
#define sangoma_create_socket_intr sangoma_open_api_span_chan
#endif

#endif	/* _LIBSNAGOMA_H */

