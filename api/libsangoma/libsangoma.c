/*******************************************************************************//**
 * \file libsangoma.c
 * \brief Wanpipe API Code Library for Sangoma AFT T1/E1/Analog/BRI/Serial hardware
 *
 * Author(s):	Nenad Corbic <ncorbic@sangoma.com>
 *              David Rokhvarg <davidr@sangoma.com>
 *              Michael Jerris <mike@jerris.com>
 * 				Anthony Minessale II <anthmct@yahoo.com>
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
 *******************************************************************************
 */


#include "libsangoma.h"

/*!
  \def DFT_CARD
  \brief Default card name. Deprecated not used
 */
#define DFT_CARD "wanpipe1"


#ifndef WP_API_FEATURE_FE_ALARM
#warning "Warning: SANGOMA API FE ALARM not supported by driver"
#endif

#ifndef WP_API_FEATURE_DTMF_EVENTS
#warning "Warning: SANGOMA API DTMF not supported by driver"
#endif

#ifndef WP_API_FEATURE_EVENTS
#warning "Warning: SANGOMA API EVENTS not supported by driver"
#endif

#ifndef WP_API_FEATURE_LINK_STATUS
#warning "Warning: SANGOMA API LINK STATUS not supported by driver"
#endif

#ifndef WP_API_FEATURE_POL_REV
#warning "Warning: SANGOMA API Polarity Reversal not supported by driver"
#endif


/*!
  \def DEV_NAME_LEN
  \brief String length of device name
 */
#define DEV_NAME_LEN	100


static void libsng_dbg(const char * fmt, ...)
{
	va_list args;
	char buf[1024];
	va_start(args, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, args);
#if defined(WIN32)
	OutputDebugString(buf);
#else
	printf(buf);
#endif
	va_end(args);
}

/*********************************************************************//**
 * WINDOWS Only Section
 *************************************************************************/

#define	DBG_POLL	if(0)libsng_dbg
#define	DBG_POLL1	if(0)libsng_dbg
#define	DBG_EVNT	if(0)libsng_dbg
#define	DBG_ERR		if(0)libsng_dbg
#define	DBG_INIT	if(0)libsng_dbg

#if defined(WIN32)

/*
  \fn static void DecodeLastError(LPSTR lpszFunction)
  \brief Decodes the Error in radable format.
  \param lpszFunction error string

  Private Windows Only Function
 */
static void DecodeLastError(LPSTR lpszFunction) 
{ 
	LPVOID lpMsgBuf;
	DWORD dwLastErr = GetLastError();
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwLastErr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);
	// Display the string.
	DBG_POLL("Last Error in %s(): %s (%d)\n", lpszFunction, lpMsgBuf, dwLastErr);
	// Free the buffer.
	LocalFree( lpMsgBuf );
} 

/*
  \fn static int handle_device_ioctl_result(int bResult)
  \brief Checks result code of ioctl
  \param bResult result of ioctl call

  Private Windows Only Function
 */
static u16 handle_device_ioctl_result(int bResult, char *caller_name)
{
	if(bResult == 0){
		//error
		DecodeLastError(caller_name);
		return 1;

	}else{
		return 0;
	}
}

/*
  \fn static int DoManagementCommand(HANDLE fd, wan_udp_hdr_t* wan_udp)
  \brief Executes Driver Management Command
  \param fd device file descriptor
  \param wan_udp managemet cmd structure

  Private Windows Function
 */
static int DoManagementCommand(HANDLE fd, wan_udp_hdr_t* wan_udp)
{
	DWORD ln, bIoResult;
	unsigned char id = 0;

	wan_udp->wan_udphdr_request_reply = 0x01;
	wan_udp->wan_udphdr_id = id;
 	wan_udp->wan_udphdr_return_code = WAN_UDP_TIMEOUT_CMD;

	bIoResult = DeviceIoControl(
			fd,
			IoctlManagementCommand,
			(LPVOID)wan_udp,
			sizeof(wan_udp_hdr_t),
			(LPVOID)wan_udp,
			sizeof(wan_udp_hdr_t),
			(LPDWORD)(&ln),
			(LPOVERLAPPED)NULL
			);

	return handle_device_ioctl_result(bIoResult, __FUNCTION__);
}

/*
  \fn static int DoTdmvApiCommand(HANDLE fd, wanpipe_tdm_api_cmd_t *api_cmd)
  \brief Executes Driver TDM API Command
  \param fd device file descriptor
  \param api_cmd tdm_api managemet cmd structure

  Private Windows Function
 */
static int DoTdmvApiCommand(HANDLE fd, wanpipe_tdm_api_cmd_t *api_cmd)
{
	DWORD ln, bIoResult;

	bIoResult = DeviceIoControl(
			fd,
			IoctlTdmApiCommand,
			(LPVOID)api_cmd,
			sizeof(wanpipe_tdm_api_cmd_t),
			(LPVOID)api_cmd,
			sizeof(wanpipe_tdm_api_cmd_t),
			(LPDWORD)(&ln),
			(LPOVERLAPPED)NULL
			);

	return handle_device_ioctl_result(bIoResult, __FUNCTION__);
}

/*
  \fn static int tdmv_api_ioctl(HANDLE fd, wanpipe_tdm_api_cmd_t *api_cmd)
  \brief Executes Driver TDM API Command Wrapper Function
  \param fd device file descriptor
  \param api_cmd tdm_api managemet cmd structure

  Private Windows Function
 */
static int tdmv_api_ioctl(HANDLE fd, wanpipe_tdm_api_cmd_t *api_cmd)
{
	if(DoTdmvApiCommand(fd, api_cmd)){
		return SANG_STATUS_GENERAL_ERROR;
	}

	return api_cmd->result;
}

/*
  \fn static USHORT DoReadCommand(HANDLE drv, RX_DATA_STRUCT * pRx)
  \brief  API READ Function
  \param drv device file descriptor
  \param pRx receive data structure

  Private Windows Function
  In Legacy API mode this fuction will Block if there is no data.
  In API mode no function is allowed to Block
 */
static USHORT DoReadCommand(HANDLE drv, RX_DATA_STRUCT * pRx)
{
	DWORD ln, bIoResult;

	bIoResult = DeviceIoControl(
			drv,
			IoctlReadCommand,
			(LPVOID)NULL,//NO input buffer!
			0,
			(LPVOID)pRx,
			sizeof(RX_DATA_STRUCT),
			(LPDWORD)(&ln),
			(LPOVERLAPPED)NULL);

	return handle_device_ioctl_result(bIoResult, __FUNCTION__);
}

/*
  \fn static UCHAR DoWriteCommand(HANDLE drv, TX_DATA_STRUCT * pTx)
  \brief API Write Function
  \param drv device file descriptor
  \param pRx receive data structure

  Private Windows Function
  In Legacy API mode this fuction will Block if data is busy.
  In API mode no function is allowed to Block
 */
static UCHAR DoWriteCommand(HANDLE drv,
							void *input_data_buffer, u32 size_of_input_data_buffer,
							void *output_data_buffer, u32 size_of_output_data_buffer
							)
{
	DWORD BytesReturned, bIoResult;

	bIoResult = DeviceIoControl(
			drv,
			IoctlWriteCommand,
			(LPVOID)input_data_buffer,
			size_of_input_data_buffer,
			(LPVOID)output_data_buffer,
			size_of_output_data_buffer,
			(LPDWORD)(&BytesReturned),
			(LPOVERLAPPED)NULL);

	return (UCHAR)handle_device_ioctl_result(bIoResult, __FUNCTION__);	
}

/*
  \fn static USHORT DoApiPollCommand(HANDLE drv, API_POLL_STRUCT *api_poll_ptr)
  \brief Non Blocking API Poll function used to find out if Rx Data, Events or
			Free Tx buffer available
  \param drv device file descriptor
  \param api_poll_ptr poll device that stores polling information read/write/event
  \param overlapped pointer to system overlapped io structure.

  Private Windows Function
 */
static USHORT DoApiPollCommand(HANDLE drv, API_POLL_STRUCT *api_poll_ptr)
{
	DWORD ln, bIoResult;

	bIoResult = DeviceIoControl(
			drv,
			IoctlApiPoll,
			(LPVOID)NULL,
			0L,
			(LPVOID)api_poll_ptr,
			sizeof(API_POLL_STRUCT),
			(LPDWORD)(&ln),
			(LPOVERLAPPED)NULL);

	return handle_device_ioctl_result(bIoResult, __FUNCTION__);
}

static int _SAPI_CALL sangoma_socket_poll(sangoma_wait_obj_t *sng_wait_obj)
{
	API_POLL_STRUCT	*api_poll = &sng_wait_obj->api_poll;
#if 0
	DBG_POLL("%s(): span: %d, chan: %d\n", __FUNCTION__, sng_wait_obj->span, sng_wait_obj->chan);
#endif
	memset(api_poll, 0x00, sizeof(API_POLL_STRUCT));

	api_poll->user_flags_bitmap = sng_wait_obj->flags_in;

	/* This call will return immediatly! 
	 * Caller of this function must implement the actual wait. */
	if(DoApiPollCommand(sng_wait_obj->fd, api_poll)){
		//failed
		return -1;
	}
	return 0;
}

static USHORT DoSetSharedEventCommand(HANDLE drv, PREGISTER_EVENT event)
{
	DWORD ln, bIoResult;

	bIoResult = DeviceIoControl(
			drv,
			IoctlSetSharedEvent,
			(LPVOID)event,
			sizeof(REGISTER_EVENT),
			(LPVOID)event,
			sizeof(REGISTER_EVENT),
			(LPDWORD)(&ln),
			(LPOVERLAPPED)NULL);

	return handle_device_ioctl_result(bIoResult, __FUNCTION__);
}

#endif	/* WIN32 */


/*********************************************************************//**
 * Common Linux & Windows Code
 *************************************************************************/



/************************************************************//**
 * Device POLL Functions
 ***************************************************************/ 


/*!
  \fn void sangoma_init_wait_obj(sangoma_wait_obj_t *sng_wait_obj, sng_fd_t fd, int span, int chan, int timeout, int flags_in, int object_type)
  \brief Initialize a wait object that will be used in sangoma_socket_waitfor_many() function  
  \param sng_wait_obj pointer a single device object 
  \param fd device file descriptor
  \param span span number starting from 1 to 255
  \param chan chan number starting from 1 to 32
  \param flags_in events to wait for (read/write/event)
  \param object_type defines whether file descriptor is a sangoma device or other sytem file descriptor.
  \return 0: success, non-zero: error
*/
int _SAPI_CALL sangoma_init_wait_obj(sangoma_wait_obj_t *sng_wait_obj, sng_fd_t fd, int span, int chan, int flags_in, int object_type)
{
	int err = 0;
	memset(sng_wait_obj, 0, sizeof(*sng_wait_obj));
	sng_wait_obj->fd		= fd;
	sng_wait_obj->flags_in	= flags_in;
	sng_wait_obj->span		= span;
	sng_wait_obj->chan		= chan;
	sng_wait_obj->object_type = object_type;

#if defined(WIN32)
	DBG_INIT("%s(): fd:0x%X, span:%d, chan:%d, flags_in:0x%X, object_type:%s", 
		__FUNCTION__, fd, span, chan, flags_in, (object_type==SANGOMA_WAIT_OBJ?"SANGOMA_WAIT_OBJ":"UNKNOWN_WAIT_OBJ"));

	sng_wait_obj->SharedEvent.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(NULL == sng_wait_obj->SharedEvent.hEvent){
		return 1;
	}

	if(SANGOMA_WAIT_OBJ == object_type){
		err = DoSetSharedEventCommand(fd, &sng_wait_obj->SharedEvent);
	}
		
	DBG_INIT("%s(): returning: %d", __FUNCTION__, err);
#else
	int filedes[2];
	sng_wait_obj->signal_read_fd = INVALID_HANDLE_VALUE;
	sng_wait_obj->signal_write_fd = INVALID_HANDLE_VALUE;
	/* if we want cross-process event notification we can use a named pipe with mkfifo() */
	if (pipe(filedes)) {
		return -1;
	}
	sng_wait_obj->signal_read_fd = filedes[0];
	sng_wait_obj->signal_write_fd = filedes[1];
#endif
	return err;
}

/*!
  \fn void sangoma_release_wait_obj(sangoma_wait_obj_t *sng_wait_obj)
  \brief De-allocate all resources in a wait object
  \param sng_wait_obj pointer a single device object 
  \return void
*/
void _SAPI_CALL sangoma_release_wait_obj(sangoma_wait_obj_t *sng_wait_obj)
{
	if(sng_wait_obj->fd != INVALID_HANDLE_VALUE){
    /* sangoma_close takes care of setting the fd value itself to invalid */
		sangoma_close(&sng_wait_obj->fd);
#ifndef WIN32
    sangoma_close(&sng_wait_obj->signal_read_fd);
    sangoma_close(&sng_wait_obj->signal_write_fd);
#endif
	}
	sng_wait_obj->object_type = UNKNOWN_WAIT_OBJ;
}

/*!
  \fn void sangoma_signal_wait_obj(sangoma_wait_obj_t *sng_wait_obj)
  \brief Set wait object to a signalled state
  \param sng_wait_obj pointer a single device object 
  \return 0 on success, non-zero on error
*/
int _SAPI_CALL sangoma_signal_wait_obj(sangoma_wait_obj_t *sng_wait_obj)
{
#if defined(WIN32)
	if(sng_wait_obj->SharedEvent.hEvent){
		if (!SetEvent(sng_wait_obj->SharedEvent.hEvent)) {
			return -1;
		}
		return 0;
	}
#else
	if (sng_wait_obj->signal_write_fd) {
		return write(sng_wait_obj->signal_write_fd, "s", 1);
	}
#endif
  /* error, this object cannot be signaled */
  return -1;
}

/*!
  \fn int sangoma_socket_waitfor_many(sangoma_wait_obj_t sangoma_wait_objects[], int number_of_sangoma_wait_objects, uint32_t system_wait_timeout)
  \brief Wait for multiple file descriptors  - poll()
  \param sangoma_wait_objects pointer to array of file descriptors to wait for
  \param number_of_sangoma_wait_objects size of the array of file descriptors
  \param system_wait_timeout timeout in miliseconds in case of no event
  \return negative: error, 0: timeout, positive: event occured check in sangoma_wait_objects
*/
int _SAPI_CALL sangoma_socket_waitfor_many(sangoma_wait_obj_t sangoma_wait_objects[], int number_of_sangoma_wait_objects, int32_t system_wait_timeout)
{
	int	i;
#if defined(WIN32)
	HANDLE hEvents[MAXIMUM_WAIT_OBJECTS];
	int rc, at_least_one_poll_set_flags_out = 0;

	if(number_of_sangoma_wait_objects > MAXIMUM_WAIT_OBJECTS){
		DBG_EVNT("Error: %s(): 'number_of_sangoma_wait_objects': %d is greater than the Maximum of: %d\n", __FUNCTION__,
			number_of_sangoma_wait_objects, MAXIMUM_WAIT_OBJECTS);
		return -1;
	}

	if(number_of_sangoma_wait_objects < 1){
		DBG_EVNT("Error: %s(): 'number_of_sangoma_wait_objects': %d is less than the Minimum of: 1!\n", __FUNCTION__,
			number_of_sangoma_wait_objects);
		return -2;
	}

	DBG_POLL1("%s(): line: %d: number_of_sangoma_wait_objects: %d\n", __FUNCTION__, __LINE__, number_of_sangoma_wait_objects);

	for(i = 0; i < number_of_sangoma_wait_objects; i++){
		hEvents[i] = sangoma_wait_objects[i].SharedEvent.hEvent;
		sangoma_wait_objects[i].flags_out = 0;
	}

	/*	Note this loop is especially important for POLLOUT!	*/
	for(i = 0; i < number_of_sangoma_wait_objects; i++){

		if(sangoma_wait_objects[i].object_type == SANGOMA_WAIT_OBJ){

			if(sangoma_socket_poll(&sangoma_wait_objects[i])){
				return -3;
			}

			if(sangoma_wait_objects[i].api_poll.operation_status != SANG_STATUS_SUCCESS){

				DBG_EVNT("Error: %s(): Invalid Operation Status: %s(%d)\n", __FUNCTION__,
					SDLA_DECODE_SANG_STATUS(sangoma_wait_objects[i].api_poll.operation_status), 
					sangoma_wait_objects[i].api_poll.operation_status);
				return -4;
			}

			if(sangoma_wait_objects[i].api_poll.poll_events_bitmap){
				sangoma_wait_objects[i].flags_out = sangoma_wait_objects[i].api_poll.poll_events_bitmap;
				at_least_one_poll_set_flags_out = 1;
			}
		}
	}

	if(at_least_one_poll_set_flags_out){
		DBG_POLL1("%s(): line: %d: at_least_one_poll_set_flags_out is set\n", __FUNCTION__, __LINE__);
		return 1;
	}

	DBG_POLL1("%s(): line: %d: going into WaitForMultipleObjects()\n", __FUNCTION__, __LINE__);

	/* wait untill at least one of the events is signalled OR a 'system_wait_timeout' occured */
	if(WAIT_TIMEOUT == WaitForMultipleObjects(number_of_sangoma_wait_objects, &hEvents[0], FALSE, system_wait_timeout)){
		DBG_POLL1("%s(): line: %d: WaitForMultipleObjects() timedout\n", __FUNCTION__, __LINE__);
		return 0;
	}

	DBG_POLL1("%s(): line: %d: returned from WaitForMultipleObjects()\n", __FUNCTION__, __LINE__);

	/* WaitForMultipleObjects() could be waken by a Sangoma or by a non-Sangoma wait object */
	for(i = 0; i < number_of_sangoma_wait_objects; i++){

		if(sangoma_wait_objects[i].object_type == SANGOMA_WAIT_OBJ){

			if(sangoma_socket_poll(&sangoma_wait_objects[i])){
				return -5;
			}

			if(sangoma_wait_objects[i].api_poll.operation_status != SANG_STATUS_SUCCESS){

				DBG_EVNT("Error: %s(): Invalid Operation Status: %s(%d)\n", __FUNCTION__,
					SDLA_DECODE_SANG_STATUS(sangoma_wait_objects[i].api_poll.operation_status), 
					sangoma_wait_objects[i].api_poll.operation_status);
				return -6;
			}

			if(sangoma_wait_objects[i].api_poll.poll_events_bitmap){
				sangoma_wait_objects[i].flags_out = sangoma_wait_objects[i].api_poll.poll_events_bitmap;
			}
		}
	}

	return 2;
#else
	/* I dont like that much the current state of things, the user of libsangoma is expected to build the
	array of sangoma wait objects and then we create the poll array, ideally we should provide a 
	helper API (sangoma_add_waitable(waitable, array)) and then sangoma_socket_waitfor_many just receives
	this array (which of course should already have a built in poll array ready to go) */
	/* in order to implement sangoma_signal_wait_obj we need an additional fd for each sangoma obj */ 
	struct pollfd pfds[number_of_sangoma_wait_objects*2];
  char dummy_buf[1];
	int signal_count = 0;
	int res;

	memset(pfds, 0, sizeof(pfds));

	for(i = 0; i < number_of_sangoma_wait_objects; i++){
		pfds[i].fd = sangoma_wait_objects[i].fd;
		pfds[i].events = sangoma_wait_objects[i].flags_in;
		if (sangoma_wait_objects[i].signal_read_fd != INVALID_HANDLE_VALUE) {
			pfds[number_of_sangoma_wait_objects+signal_count].fd = sangoma_wait_objects[i].signal_read_fd;
			pfds[number_of_sangoma_wait_objects+signal_count].events = POLLIN;
			signal_count++;
		}
	}

	poll_try_again:

	res = poll(pfds, (number_of_sangoma_wait_objects + signal_count), system_wait_timeout);
	if (res > 0) {
		for(i = 0; i < number_of_sangoma_wait_objects; i++){
			sangoma_wait_objects[i].flags_out = pfds[i].revents;
			/* should we do something extra for signalled objects? */
		}
		for(i = 0; i < signal_count; i++){
			/* should we do something extra for signalled objects? */
			if (pfds[number_of_sangoma_wait_objects+i].revents & POLLIN) {
				/* read and discard the signal byte */
				read(pfds[number_of_sangoma_wait_objects+i].fd, &dummy_buf, 1);
			}
		}
	} else if (res < 0 && errno == EINTR) {
		/* TODO: decrement system_wait_timeout */
		goto poll_try_again;
	}

	return res;
#endif
}


/*!
  \fn int sangoma_socket_waitfor(sangoma_wait_obj_t *sangoma_wait_obj, int timeout, int flags_in, unsigned int *flags_out)
  \brief Wait for a single file descriptor - poll()
  \param sangoma_wait_obj pointer to array of file descriptors to wait for
  \param timeout timeout in miliseconds in case of no event
  \param flags_in events to wait for (read/write/event)
  \param flags_out events that occured (read/write/event)
  \return negative: error,  0: timeout, positive: event occured check in *flags_out
*/
int _SAPI_CALL sangoma_socket_waitfor(sangoma_wait_obj_t *sangoma_wait_obj, int timeout, int flags_in, unsigned int *flags_out)
{
	int err;

	sangoma_wait_obj->flags_in = flags_in;

	err = sangoma_socket_waitfor_many(sangoma_wait_obj, 1, timeout);

	if(err < 0 || err == 0){
		return err;
	}

	*flags_out = sangoma_wait_obj->flags_out;

	return 1;
}


/************************************************************//**
 * Device OPEN / CLOSE Functions
 ***************************************************************/


int _SAPI_CALL sangoma_span_chan_toif(int span, int chan, char *interface_name)
{
#if defined(WIN32)
/* FIXME: Not implemented */
	return -1;
#else
 	sprintf(interface_name,"s%ic%i",span,chan);
#endif
	return 0;
}

int _SAPI_CALL sangoma_interface_toi(char *interface_name, int *span, int *chan)
{
	char *p=NULL, *sp = NULL, *ch = NULL;
	int ret = 0;
	char data[FNAME_LEN];

	strncpy(data, interface_name, FNAME_LEN);
	if ((data[0])) {
		for (p = data; *p; p++) {
			if (sp && *p == 'g') {
				*p = '\0';
				ch = (p + 1);
				break;
			} else if (*p == 'w') {
				sp = (p + 1);
			}
		}

		if(ch && sp) {
			*span = atoi(sp);
			*chan = atoi(ch);
			ret = 1;
		} else {
			*span = -1;
			*chan = -1;
		}
	}

	return ret;
}

int _SAPI_CALL sangoma_interface_wait_up(int span, int chan, int sectimeout)
{
#ifdef WIN32
  /* Windows does not need to wait for interfaces to come up */
  return 0;
#else
	char interface_name[FNAME_LEN];
  struct stat statbuf;
  struct timeval endtime = {0,0};
  struct timeval curtime = {0,0};
  int counter;
  int rc;
  if (sectimeout >= 0 && gettimeofday(&endtime, NULL)) {
    return -1;
  }
	snprintf(interface_name, sizeof(interface_name), "/dev/wanpipe%d_if%d", span, chan);
  endtime.tv_sec += sectimeout;
  do {
    counter = 0;
    while ((rc = stat(interface_name, &statbuf)) && errno == ENOENT && counter != 10) {
      poll(0, 0, 100); // test in 100ms increments
      counter++;
    }
    if (!rc) break;
    if (gettimeofday(&curtime, NULL)) {
      fprintf(stderr, "two!\n");
      return -1;
    }
  } while (sectimeout < 0 || timercmp(&endtime, &curtime,>));
  return rc;
#endif
}

int _SAPI_CALL sangoma_span_chan_fromif(char *interface_name, int *span, int *chan)
{
	char *p = NULL, *sp = NULL, *ch = NULL;
	int ret = 0;
	char data[FNAME_LEN];

	strncpy(data, interface_name, FNAME_LEN);
	if ((data[0])) {
		for (p = data; *p; p++) {
			if (sp && *p == 'c') {
				*p = '\0';
				ch = (p + 1);
				break;
			} else if (*p == 's') {
				sp = (p + 1);
			}
		}

		if(ch && sp) {
			*span = atoi(sp);
			*chan = atoi(ch);
			ret = 1;
		} else {
			*span = -1;
			*chan = -1;
		}
	}

	return ret;
}

sng_fd_t _SAPI_CALL sangoma_open_api_span_chan(int span, int chan) 
{
	sng_fd_t fd = INVALID_HANDLE_VALUE;
	wanpipe_api_t tdm_api;
	int err;

	fd = __sangoma_open_api_span_chan(span, chan);

#if defined(__WINDOWS__)
	if(fd == INVALID_HANDLE_VALUE){
		//DBG_EVNT("Span: %d, chan: %d: is not running, consider 'busy'\n", span, chan);
		return fd;
	}
#else
	if (fd < 0) {
		return fd;
	}
#endif

	memset(&tdm_api,0,sizeof(tdm_api));
	tdm_api.wp_cmd.cmd = WP_API_CMD_OPEN_CNT;
	err=sangoma_cmd_exec(fd,&tdm_api);
	if (err){
		sangoma_close(&fd);
		return fd;
	}

	if (tdm_api.wp_cmd.open_cnt > 1) {
		/* this is NOT the first open request for this span/chan */
		sangoma_close(&fd);
		fd = INVALID_HANDLE_VALUE;/* fd is NOT valid anymore */
	}

	return fd;
}            

/* no checks done for multiple open */
sng_fd_t _SAPI_CALL __sangoma_open_api_span_chan(int span, int chan)
{
   	char fname[FNAME_LEN], tmp_fname[FNAME_LEN];

	/* Form the Interface Name from span and chan number (i.e. wanpipe1_if1). */
	_snprintf(tmp_fname, DEV_NAME_LEN, WP_INTERFACE_NAME_FORM, span, chan);

#if defined(WIN32)
	_snprintf(fname , FNAME_LEN, "\\\\.\\%s", tmp_fname);
	return CreateFile(	fname, 
						GENERIC_READ | GENERIC_WRITE, 
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						(LPSECURITY_ATTRIBUTES)NULL, 
						OPEN_EXISTING,
						FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
						(HANDLE)NULL
						);
#else
	//sprintf(fname,"/dev/wanpipe%d_if%d",span,chan);
	sprintf(fname,"/dev/%s", tmp_fname);

	return open(fname, O_RDWR);
#endif
}            

sng_fd_t _SAPI_CALL sangoma_open_api_ctrl(void)
{
#if defined(WIN32)
	sng_fd_t fd = INVALID_HANDLE_VALUE;
#pragma message("sangoma_open_api_ctrl: Not support on Windows")
#else
	sng_fd_t fd=-1;

	fd = open("/dev/wanpipe_ctrl", O_RDWR);
#endif

    return fd;
}


int _SAPI_CALL sangoma_get_open_cnt(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;
	tdm_api->wp_cmd.cmd = WP_API_CMD_OPEN_CNT;

	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return -1;
	}

	return tdm_api->wp_cmd.open_cnt;
}

sng_fd_t _SAPI_CALL sangoma_create_socket_by_name(char *device, char *card) 
{
	int span,chan;
	sangoma_interface_toi(device,&span,&chan);
	
	return sangoma_open_api_span_chan(span,chan);
}

          
sng_fd_t _SAPI_CALL sangoma_open_api_span(int span) 
{
    int i=0;
	sng_fd_t fd = INVALID_HANDLE_VALUE;

	for(i = 1; i < 32; i++){

		fd = sangoma_open_api_span_chan(span, i);

#if defined(WIN32)
		if(fd != INVALID_HANDLE_VALUE){
#else
		if (fd >= 0) {
#endif

			//found free chan
			break;
		}

	}//for()

    return fd;  
}


/*!
  \fn void sangoma_close(sng_fd_t *fd)
  \brief Close device file descriptor
  \param fd device file descriptor
  \return void
*/
void _SAPI_CALL sangoma_close(sng_fd_t *fd)
{
#if defined(WIN32)
	if(	*fd != INVALID_HANDLE_VALUE){
		CloseHandle(*fd);
		*fd = INVALID_HANDLE_VALUE;
	}
#else
    if (*fd >= 0) {
		close(*fd);
		*fd = -1;
    }
#endif
}



/************************************************************//**
 * Device READ / WRITE Functions
 ***************************************************************/ 

int _SAPI_CALL sangoma_readmsg(sng_fd_t fd, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag)
{
	int rx_len=0;

#if defined(WIN32)
	wp_api_hdr_t	*rx_hdr = (wp_api_hdr_t*)hdrbuf;
	wp_api_element_t wp_api_element;

	if(hdrlen != sizeof(wp_api_hdr_t)){
		//error
		DBG_EVNT("Error: %s(): invalid size of user's 'header buffer'. Should be 'sizeof(wp_api_hdr_t)'.\n", __FUNCTION__);
		return -1;
	}

	if(DoReadCommand(fd, &wp_api_element)){
		//error
		DBG_EVNT("Error: %s(): DoReadCommand() failed! Check messages log.\n", __FUNCTION__);
		return -4;
	}

	memcpy(rx_hdr, &wp_api_element.hdr, sizeof(wp_api_hdr_t));

	switch(rx_hdr->operation_status)
	{
	case SANG_STATUS_RX_DATA_AVAILABLE:
		/* ok */
		if(rx_hdr->data_length <= datalen){
			memcpy(databuf, wp_api_element.data, rx_hdr->data_length);
		}else{
			rx_hdr->operation_status = SANG_STATUS_BUFFER_TOO_SMALL;
		}
		break;
	default:
		/* note that SANG_STATUS_DEVICE_BUSY is NOT an error! */
		if(0)DBG_EVNT("Error: %s(): Operation Status: %s(%d)\n", __FUNCTION__, 
			SDLA_DECODE_SANG_STATUS(rx_hdr->operation_status), rx_hdr->operation_status);
		return -5;
	}

	rx_len = rx_hdr->data_length;
#else
	struct msghdr msg;
	struct iovec iov[2];

	memset(&msg,0,sizeof(struct msghdr));

	iov[0].iov_len=hdrlen;
	iov[0].iov_base=hdrbuf;

	iov[1].iov_len=datalen;
	iov[1].iov_base=databuf;

	msg.msg_iovlen=2;
	msg.msg_iov=iov;

	rx_len = read(fd,&msg,datalen+hdrlen);

	if (rx_len <= sizeof(wp_api_hdr_t)){
		return -EINVAL;
	}

	rx_len-=sizeof(wp_api_hdr_t);
#endif
    return rx_len;
}                    

int _SAPI_CALL sangoma_writemsg(sng_fd_t fd, void *hdrbuf, int hdrlen, void *databuf, unsigned short datalen, int flag)
{
	int bsent=-1;
	wp_api_hdr_t *wp_api_hdr = hdrbuf;

	if (hdrlen != sizeof(wp_api_hdr_t)) {
		DBG_ERR("Error: sangoma_writemsg() failed! hdrlen (%i) != sizeof(wp_api_hdr_t) (%i)\n",
						hdrlen,sizeof(wp_api_hdr_t));
		wp_api_hdr->operation_status = SANG_STATUS_TX_HDR_TOO_SHORT;
		return -1;
	}

#if defined(WIN32)
	//queue data for transmission
	if(DoWriteCommand(fd, databuf, datalen, hdrbuf, hdrlen)){
		//error
		DBG_EVNT("Error: DoWriteCommand() failed!! Check messages log.\n");
		return -1;
	}

	bsent=0;
	//check that frame was transmitted
	switch(wp_api_hdr->operation_status)
	{
	case SANG_STATUS_SUCCESS:
		bsent = datalen;
		break;
	default:
		DBG_EVNT("Error: %s(): Operation Status: %s(%d)\n", __FUNCTION__, 
			SDLA_DECODE_SANG_STATUS(wp_api_hdr->operation_status), wp_api_hdr->operation_status);
		break;
	}//switch()
#else
	struct msghdr msg;
	struct iovec iov[2];
	wp_api_hdr_t	*tx_el=hdrbuf;

	memset(&msg,0,sizeof(struct msghdr));

	iov[0].iov_len=hdrlen;
	iov[0].iov_base=hdrbuf;

	iov[1].iov_len=datalen;
	iov[1].iov_base=databuf;

	msg.msg_iovlen=2;
	msg.msg_iov=iov;

	bsent = write(fd,&msg,datalen+hdrlen);

	if (bsent == (datalen+hdrlen)){
		tx_el->wp_api_hdr_operation_status=SANG_STATUS_SUCCESS;
		bsent-=sizeof(wp_api_hdr_t);
	} else if (errno == EBUSY){
		tx_el->wp_api_hdr_operation_status=SANG_STATUS_DEVICE_BUSY;
	} else {
		tx_el->wp_api_hdr_operation_status=SANG_STATUS_IO_ERROR;
	}
	tx_el->wp_api_hdr_data_length=bsent;

	//FIXME - THIS SHOULD BE DONE IN KERNEL
        tx_el->wp_api_tx_hdr_max_queue_length=16;
        tx_el->wp_api_tx_hdr_number_of_frames_in_queue=0;

#endif
	return bsent;
}


#ifdef WANPIPE_TDM_API


/************************************************************//**
 * Device API COMMAND Functions
 ***************************************************************/ 



/*========================================================
 * Execute TDM command
 *
 */
int _SAPI_CALL sangoma_cmd_exec(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

#if defined(WIN32)
	err = tdmv_api_ioctl(fd, &tdm_api->wp_cmd);
#else
	err = ioctl(fd,WANPIPE_IOCTL_API_CMD,&tdm_api->wp_cmd);
	if (err < 0){
		char tmp[50];
		sprintf(tmp,"TDM API: CMD: %i\n",tdm_api->wp_cmd.cmd);
		perror(tmp);
		return -1;
	}
#endif
	return err;
}

/*========================================================
 * Get Full TDM API configuration per channel
 *
 */
int _SAPI_CALL sangoma_get_full_cfg(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_FULL_CFG;

	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

#if 0
	printf("TDM API CFG:\n");
	printf("\thw_tdm_coding:\t%d\n",tdm_api->wp_cmd.hw_tdm_coding);
	printf("\thw_mtu_mru:\t%d\n",tdm_api->wp_cmd.hw_mtu_mru);
	printf("\tusr_period:\t%d\n",tdm_api->wp_cmd.usr_period);
	printf("\ttdm_codec:\t%d\n",tdm_api->wp_cmd.tdm_codec);
	printf("\tpower_level:\t%d\n",tdm_api->wp_cmd.power_level);
	printf("\trx_disable:\t%d\n",tdm_api->wp_cmd.rx_disable);
	printf("\ttx_disable:\t%d\n",tdm_api->wp_cmd.tx_disable);
	printf("\tusr_mtu_mru:\t%d\n",tdm_api->wp_cmd.usr_mtu_mru);
	printf("\tidle flag:\t0x%02X\n",tdm_api->wp_cmd.idle_flag);

#ifdef WP_API_FEATURE_FE_ALARM
	printf("\tfe alarms:\t0x%02X\n",tdm_api->wp_cmd.fe_alarms);
#endif
	
	printf("\trx pkt\t%d\ttx pkt\t%d\n",tdm_api->wp_cmd.stats.rx_packets,
				tdm_api->wp_cmd.stats.tx_packets);
	printf("\trx err\t%d\ttx err\t%d\n",
				tdm_api->wp_cmd.stats.rx_errors,
				tdm_api->wp_cmd.stats.tx_errors);
#ifndef __WINDOWS__
	printf("\trx ovr\t%d\ttx idl\t%d\n",
				tdm_api->wp_cmd.stats.rx_fifo_errors,
				tdm_api->wp_cmd.stats.tx_carrier_errors);
#endif		
#endif		
	
	return 0;
}

/*========================================================
 * SET Codec on a particular Channel.
 * 
 * Available codecs are defined in 
 * /usr/src/linux/include/linux/wanpipe_cfg.h
 * 
 * enum wan_codec_format {
 *  	WP_NONE,
 *	WP_SLINEAR
 * }
 *
 */
int _SAPI_CALL sangoma_tdm_set_codec(sng_fd_t fd, wanpipe_api_t *tdm_api, int codec)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_CODEC;
	tdm_api->wp_cmd.tdm_codec = codec;

	err=sangoma_cmd_exec(fd,tdm_api);

	return err;
}

/*========================================================
 * GET Codec from a particular Channel.
 * 
 * Available codecs are defined in 
 * /usr/src/linux/include/linux/wanpipe_cfg.h
 * 
 * enum wan_codec_format {
 *  	WP_NONE,
 *	WP_SLINEAR
 * }
 *
 */
int _SAPI_CALL sangoma_tdm_get_codec(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_CODEC;

	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_cmd.tdm_codec;	
}

/*========================================================
 * SET Rx/Tx Hardware Period in milliseconds.
 * 
 * Available options are:
 *  10,20,30,40,50 ms      
 *
 */
int _SAPI_CALL sangoma_tdm_set_usr_period(sng_fd_t fd, wanpipe_api_t *tdm_api, int period)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_USR_PERIOD;
	tdm_api->wp_cmd.usr_period = period;

	err=sangoma_cmd_exec(fd,tdm_api);

	return err;
}

/*========================================================
 * GET Rx/Tx Hardware Period in milliseconds.
 * 
 * Available options are:
 *  10,20,30,40,50 ms      
 *
 */
int _SAPI_CALL sangoma_tdm_get_usr_period(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_USR_PERIOD;

	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_cmd.usr_period;
}

/*========================================================
 * GET Current User Hardware Coding Format
 *
 * Coding Format will be ULAW/ALAW based on T1/E1 
 */

int _SAPI_CALL sangoma_get_hw_coding(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
        int err;
        tdm_api->wp_cmd.cmd = WP_API_CMD_GET_HW_CODING;
        err=sangoma_cmd_exec(fd,tdm_api);
        if (err){
                return err;
        }
        return tdm_api->wp_cmd.hw_tdm_coding;
}

#ifdef WP_API_FEATURE_DTMF_EVENTS
/*========================================================
 * GET Current User Hardware DTMF Enabled/Disabled
 *
 * Will return true if HW DTMF is enabled on Octasic
 */

int _SAPI_CALL sangoma_tdm_get_hw_dtmf(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;
	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_HW_DTMF;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}
	return tdm_api->wp_cmd.hw_dtmf;
}
#endif

/*========================================================
 * GET Current User MTU/MRU values in bytes.
 * 
 * The USER MTU/MRU values will change each time a PERIOD
 * or CODEC is adjusted.
 */
int _SAPI_CALL sangoma_tdm_get_usr_mtu_mru(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_USR_MTU_MRU;

	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_cmd.usr_mtu_mru;
}

/*========================================================
 * SET TDM Power Level
 * 
 * This option is not implemented yet
 *
 */
int _SAPI_CALL sangoma_tdm_set_power_level(sng_fd_t fd, wanpipe_api_t *tdm_api, int power)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_POWER_LEVEL;
	tdm_api->wp_cmd.power_level = power;

	err=sangoma_cmd_exec(fd,tdm_api);

	return err;
}

/*========================================================
 * GET TDM Power Level
 * 
 * This option is not implemented yet
 *
 */
int _SAPI_CALL sangoma_tdm_get_power_level(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_POWER_LEVEL;

	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_cmd.power_level;
}

int _SAPI_CALL sangoma_flush_bufs(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;
	tdm_api->wp_cmd.cmd = WP_API_CMD_FLUSH_BUFFERS;

	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_enable_rbs_events(sng_fd_t fd, wanpipe_api_t *tdm_api, int poll_in_sec) {
	
	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_ENABLE_RBS_EVENTS;
	tdm_api->wp_cmd.rbs_poll=poll_in_sec;
	
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}


int _SAPI_CALL sangoma_tdm_disable_rbs_events(sng_fd_t fd, wanpipe_api_t *tdm_api) {

	int err;
	tdm_api->wp_cmd.cmd = WP_API_CMD_DISABLE_RBS_EVENTS;
	
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_write_rbs(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel, unsigned char rbs)
{
	
	int err;
	tdm_api->wp_cmd.cmd = WP_API_CMD_WRITE_RBS_BITS;
	tdm_api->wp_cmd.chan = (unsigned char)channel;
	tdm_api->wp_cmd.rbs_tx_bits=rbs;
	
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}        


int _SAPI_CALL sangoma_tdm_read_rbs(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel, unsigned char *rbs)
{
	
	int err;
	tdm_api->wp_cmd.cmd = WP_API_CMD_READ_RBS_BITS;
	tdm_api->wp_cmd.chan = (unsigned char)channel;
	tdm_api->wp_cmd.rbs_tx_bits=0;
	
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	*rbs=(unsigned char)tdm_api->wp_cmd.rbs_rx_bits;

	return 0;
}        

int _SAPI_CALL sangoma_read_event(sng_fd_t fd, wanpipe_api_t *tdm_api)
{

#ifdef WP_API_FEATURE_EVENTS
	wp_api_event_t *rx_event;
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_READ_EVENT;
	
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	rx_event = &tdm_api->wp_cmd.event;


   	/*
	 The use of callbacks here is purely optional and is left
     here for backward compatibility purposes.  By default user
 	 should handle events outside this funciton. This function
	 should only be used to read the event
    */

	switch (rx_event->wp_api_event_type){
	
	case WP_API_EVENT_RBS:
		if (tdm_api->wp_callback.wp_rbs_event) {
			tdm_api->wp_callback.wp_rbs_event(fd,rx_event->wp_api_event_rbs_bits);
		}
		
		break;
	
#ifdef WP_API_FEATURE_DTMF_EVENTS
	case WP_API_EVENT_DTMF:
		if (tdm_api->wp_callback.wp_dtmf_event) {
			tdm_api->wp_callback.wp_dtmf_event(fd,
						rx_event->wp_api_event_dtmf_digit,
						rx_event->wp_api_event_dtmf_type,
						rx_event->wp_api_event_dtmf_port);
		}
		break;
#endif
		
	case WP_API_EVENT_RXHOOK:
		if (tdm_api->wp_callback.wp_rxhook_event) {
			tdm_api->wp_callback.wp_rxhook_event(fd,
						rx_event->wp_api_event_hook_state);
		}
		break;

	case WP_API_EVENT_RING_DETECT:
		if (tdm_api->wp_callback.wp_ring_detect_event) {
			tdm_api->wp_callback.wp_ring_detect_event(fd,
						rx_event->wp_api_event_ring_state);
		}
		break;

	case WP_API_EVENT_RING_TRIP_DETECT:
		if (tdm_api->wp_callback.wp_ring_trip_detect_event) {
			tdm_api->wp_callback.wp_ring_trip_detect_event(fd,
						rx_event->wp_api_event_ring_state);
		}
		break;

#ifdef WP_API_FEATURE_FE_ALARM
	case WP_API_EVENT_ALARM:
		if (tdm_api->wp_callback.wp_fe_alarm_event) {
			tdm_api->wp_callback.wp_fe_alarm_event(fd,
						rx_event->wp_api_event_alarm);
		}   
		break; 
#endif

#ifdef WP_API_FEATURE_LINK_STATUS
	/* Link Status */	
	case WP_API_EVENT_LINK_STATUS:
		if(tdm_api->wp_callback.wp_link_status_event){
			tdm_api->wp_callback.wp_link_status_event(fd,
						rx_event->wp_api_event_link_status);
		}
		
		break;
#endif

#ifdef WP_API_FEATURE_POL_REV
    case WP_API_EVENT_POLARITY_REVERSE:
        break;
#endif	
	default:
#ifdef __WINDOWS__
		printf("libsangoma: %s fd=0x%p: Unknown TDM event!", __FUNCTION__,fd);
#else
		printf("libsangoma: %s fd=%d: Unknown TDM event!", __FUNCTION__, fd);
#endif
		break;
	}
	
	return 0;
#else
	printf("Error: Read Event not supported!\n");
	return -1;
#endif
}        

#ifdef WP_API_FEATURE_DTMF_EVENTS
int _SAPI_CALL sangoma_tdm_enable_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_DTMF;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_disable_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_DTMF;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_DISABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_enable_rm_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RM_DTMF;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_disable_rm_dtmf_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RM_DTMF;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_DISABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_enable_rxhook_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RXHOOK;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_disable_rxhook_events(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RXHOOK;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_DISABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_enable_ring_events(sng_fd_t fd, wanpipe_api_t *tdm_api) {
	
	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RING;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}


int _SAPI_CALL sangoma_tdm_disable_ring_events(sng_fd_t fd, wanpipe_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RING;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_DISABLE;
	
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_enable_ring_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api) {
	
	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RING_DETECT;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return err;
}


int _SAPI_CALL sangoma_tdm_disable_ring_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RING_DETECT;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_DISABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_enable_ring_trip_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api) {
	
	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RING_TRIP_DETECT;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_cmd.rbs_poll;
}


int _SAPI_CALL sangoma_tdm_disable_ring_trip_detect_events(sng_fd_t fd, wanpipe_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_RING_DETECT;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_DISABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_txsig_kewl(sng_fd_t fd, wanpipe_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_TXSIG_KEWL;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_txsig_start(sng_fd_t fd, wanpipe_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_TXSIG_START;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_txsig_onhook(sng_fd_t fd, wanpipe_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_TXSIG_ONHOOK;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int _SAPI_CALL sangoma_tdm_txsig_offhook(sng_fd_t fd, wanpipe_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_TXSIG_OFFHOOK;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}


int _SAPI_CALL sangoma_tdm_enable_tone_events(sng_fd_t fd, wanpipe_api_t *tdm_api, uint16_t tone_id) {
	
	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_TONE;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	tdm_api->wp_cmd.event.wp_api_event_tone_type = tone_id;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_cmd.rbs_poll;
}

int _SAPI_CALL sangoma_tdm_disable_tone_events(sng_fd_t fd, wanpipe_api_t *tdm_api) {
	
	int err;
	
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_TONE;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_DISABLE;
	tdm_api->wp_cmd.event.wp_api_event_tone_type = 0x00;
	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_cmd.rbs_poll;
}

#endif

int _SAPI_CALL sangoma_tdm_enable_hwec(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
        int err;

        tdm_api->wp_cmd.cmd = WP_API_CMD_ENABLE_HWEC;
        err=sangoma_cmd_exec(fd,tdm_api);
        if (err){
                return err;
        }

        return 0;
}

int _SAPI_CALL sangoma_tdm_disable_hwec(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
        int err;

        tdm_api->wp_cmd.cmd = WP_API_CMD_DISABLE_HWEC;
        err=sangoma_cmd_exec(fd,tdm_api);
        if (err){
                return err;
        }

        return 0;
}


/*========================================================
 * GET Front End Alarms
 * 
 */                  
#ifdef WP_API_FEATURE_FE_ALARM
int _SAPI_CALL sangoma_tdm_get_fe_alarms(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned int *alarms)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_FE_ALARMS;

	err=sangoma_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	*alarms=tdm_api->wp_cmd.fe_alarms;

	return 0;
}         

/* get current Line Connection state - Connected/Disconnected */
int _SAPI_CALL sangoma_get_fe_status(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *current_status)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_FE_STATUS;
	err = sangoma_cmd_exec(fd, tdm_api);
	*current_status = tdm_api->wp_cmd.fe_status;

	return err;
}
#endif

/* get current Line Connection state - Connected/Disconnected */
#ifdef WP_API_FEATURE_LINK_STATUS
int _SAPI_CALL sangoma_get_link_status(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *current_status)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_FE_STATUS;
	err = sangoma_cmd_exec(fd, tdm_api);
	*current_status = tdm_api->wp_cmd.fe_status;

	return err;
}

/* set current Line Connection state - Connected/Disconnected. valid only for ISDN BRI */
int _SAPI_CALL sangoma_set_fe_status(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char new_status)
{
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_FE_STATUS;
	tdm_api->wp_cmd.fe_status = new_status;

	return sangoma_cmd_exec(fd, tdm_api);
}
#endif

int _SAPI_CALL sangoma_disable_bri_bchan_loopback(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel)
{
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.channel	= (unsigned char)channel;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_BRI_CHAN_LOOPBACK;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_DISABLE;
	return sangoma_cmd_exec(fd, tdm_api);
}

int _SAPI_CALL sangoma_enable_bri_bchan_loopback(sng_fd_t fd, wanpipe_api_t *tdm_api, int channel)
{
	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_EVENT;
	tdm_api->wp_cmd.event.channel	= (unsigned char)channel;
	tdm_api->wp_cmd.event.wp_api_event_type = WP_API_EVENT_BRI_CHAN_LOOPBACK;
	tdm_api->wp_cmd.event.wp_api_event_mode = WP_API_EVENT_ENABLE;
	return sangoma_cmd_exec(fd, tdm_api);
}


int _SAPI_CALL sangoma_get_tx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_TX_Q_SIZE;
	tdm_api->wp_cmd.tx_queue_sz = 0;
	
	err=sangoma_cmd_exec(fd, tdm_api);
	if (err < 0) {
		return err;
	}

	return tdm_api->wp_cmd.tx_queue_sz;
}

int _SAPI_CALL sangoma_set_tx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api, int size)
{
	if (size < 0) {
		return -1;
	}

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_TX_Q_SIZE;
	tdm_api->wp_cmd.tx_queue_sz = size;
	
	return sangoma_cmd_exec(fd, tdm_api);
}

int _SAPI_CALL sangoma_get_rx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_RX_Q_SIZE;
	tdm_api->wp_cmd.rx_queue_sz = 0;
	
	err=sangoma_cmd_exec(fd, tdm_api);
	if (err < 0) {
		return err;
	}

	return tdm_api->wp_cmd.rx_queue_sz;

}

int _SAPI_CALL sangoma_set_rx_queue_sz(sng_fd_t fd, wanpipe_api_t *tdm_api, int size)
{
	if (size < 0) {
		return -1;
	}

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_RX_Q_SIZE;
	tdm_api->wp_cmd.rx_queue_sz = size;
	
	return sangoma_cmd_exec(fd, tdm_api);

}

int _SAPI_CALL sangoma_get_driver_version(sng_fd_t fd, wanpipe_api_t *tdm_api, wan_driver_version_t *drv_ver)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_DRIVER_VERSION;

	err = sangoma_cmd_exec(fd, tdm_api);
	if (err == 0) {
		if (tdm_api->wp_cmd.data_len == sizeof(wan_driver_version_t)) {
			if (drv_ver) {
				memcpy(drv_ver,&tdm_api->wp_cmd.version,sizeof(wan_driver_version_t));
			}
		} else {
			return -1;
		}
	}

	return err;
}

int _SAPI_CALL sangoma_get_firmware_version(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *ver)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_FIRMWARE_VERSION;

	err = sangoma_cmd_exec(fd, tdm_api);
	if (err == 0) {
		if (tdm_api->wp_cmd.data_len == sizeof(unsigned char)) {
			*ver = tdm_api->wp_cmd.data[0];
		} else {
			return -1;
		}
	}

	return err;
}


int _SAPI_CALL sangoma_get_cpld_version(sng_fd_t fd, wanpipe_api_t *tdm_api, unsigned char *ver)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_CPLD_VERSION;

	err = sangoma_cmd_exec(fd, tdm_api);
	if (err == 0) {
		if (tdm_api->wp_cmd.data_len == sizeof(unsigned char)) {
			*ver = tdm_api->wp_cmd.data[0];
		} else {
			return -1;
		}
	}

	return err;
}

int _SAPI_CALL sangoma_get_stats(sng_fd_t fd, wanpipe_api_t *tdm_api, wanpipe_chan_stats_t *stats)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_GET_STATS;

	err = sangoma_cmd_exec(fd, tdm_api);
	if (err == 0) {
		if (stats) {
			memcpy(stats,&tdm_api->wp_cmd.stats,sizeof(wanpipe_chan_stats_t));
		}
	}

	return err;
}

int _SAPI_CALL sangoma_flush_stats(sng_fd_t fd, wanpipe_api_t *tdm_api)
{
	tdm_api->wp_cmd.cmd = WP_API_CMD_RESET_STATS;
	return sangoma_cmd_exec(fd, tdm_api);
}

int _SAPI_CALL sangoma_set_rm_rxflashtime(sng_fd_t fd, wanpipe_api_t *tdm_api, int rxflashtime)
{
	int err;

	tdm_api->wp_cmd.cmd = WP_API_CMD_SET_RM_RXFLASHTIME;
	tdm_api->wp_cmd.rxflashtime=rxflashtime;
	err = sangoma_cmd_exec(fd, tdm_api);
	return err;
}


#ifndef LIBSANGOMA_LIGHT 


/************************************************************//**
 * Device PORT Control Functions
 ***************************************************************/

static int sangoma_port_mgmnt_ioctl(sng_fd_t fd, port_management_struct_t *port_management)
{

#if defined(__WINDOWS__)
	DWORD ln;
	if(DeviceIoControl(
			fd,
			IoctlPortManagementCommand,
			(LPVOID)port_management,
			sizeof(port_management_struct_t),
			(LPVOID)port_management,
			sizeof(port_management_struct_t),
			(LPDWORD)(&ln),
			(LPOVERLAPPED)NULL
						) == FALSE){
		//check messages log
		printf("%s():Error: IoctlPortManagementCommand failed!!\n", __FUNCTION__);
		return -1;
	}else {
		return 0;
	}
#else
	int err;
	err=ioctl(fd,WANPIPE_IOCTL_PORT_MGMT,port_management);
	if (err) {
		return -1;
	} else {
		return 0;
	}
#endif
}

static int sangoma_port_cfg_ioctl(sng_fd_t fd, port_cfg_t *port_cfg)
{

#if defined(__WINDOWS__)
	DWORD ln;
	if(DeviceIoControl(
			fd,
			IoctlPortConfigurationCommand,
			(LPVOID)port_cfg,
			sizeof(port_cfg_t),
			(LPVOID)port_cfg,
			sizeof(port_cfg_t),
			(LPDWORD)(&ln),
			(LPOVERLAPPED)NULL
						) == FALSE){
		//check messages log
		printf("%s():Error: IoctlSetDriverConfiguration failed!!\n", __FUNCTION__);
		return 1;
	}else{
		return 0;
	}
#else
	int err;
	err=ioctl(fd,WANPIPE_IOCTL_PORT_CONFIG,port_cfg);
	if (err) {
		return -1;
	} else {
		return 0;
	}
#endif
}

sng_fd_t _SAPI_CALL sangoma_open_driver_ctrl(int port_no)
{
#if defined(WIN32)
   	char fname[FNAME_LEN];
	_snprintf(fname , FNAME_LEN, "\\\\.\\wanpipe%d", port_no);
	return CreateFile(	fname, 
						GENERIC_READ | GENERIC_WRITE, 
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						(LPSECURITY_ATTRIBUTES)NULL, 
						OPEN_EXISTING,
						FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
						(HANDLE)NULL
						);
#else
	return open("/dev/wanpipe", O_RDWR);
#endif
}


int _SAPI_CALL sangoma_mgmt_cmd(sng_fd_t fd, wan_udp_hdr_t* wan_udp)
{
#if defined(__WINDOWS__)
	if(DoManagementCommand(fd, wan_udp)){
		return 1;
	}
#else
	unsigned char id = 0;
	int err=0;
	wan_udp->wan_udphdr_request_reply = 0x01;
	wan_udp->wan_udphdr_id = id;
	wan_udp->wan_udphdr_return_code = WAN_UDP_TIMEOUT_CMD;

	err=ioctl(fd,WANPIPE_IOCTL_PIPEMON,wan_udp);
	if (err < 0) {
		return 1;
	}
#endif

	if(wan_udp->wan_udphdr_return_code != WAN_CMD_OK){
		return 2;
	}
	return 0;
}

int _SAPI_CALL sangoma_driver_port_start(sng_fd_t fd, port_management_struct_t *port_mgmnt, unsigned short port_no)
{
	int err;
	port_mgmnt->command_code = START_PORT_VOLATILE_CONFIG;
	port_mgmnt->port_no	= port_no;

	err = sangoma_port_mgmnt_ioctl(fd, port_mgmnt);
	if (err) {
		return err;
	}

	if (port_mgmnt->operation_status != SANG_STATUS_SUCCESS ) {
		err=port_mgmnt->operation_status;
	}

	return err;
}

int _SAPI_CALL sangoma_driver_port_stop(sng_fd_t fd, port_management_struct_t *port_mgmnt, unsigned short port_no)
{
	int err;
	port_mgmnt->command_code = STOP_PORT;
	port_mgmnt->port_no	= port_no;

	err = sangoma_port_mgmnt_ioctl(fd, port_mgmnt);
	if (err) {
		return err;
	}

	if (port_mgmnt->operation_status != SANG_STATUS_SUCCESS ) {
		err=port_mgmnt->operation_status;
	}

	return err;
}

int _SAPI_CALL sangoma_driver_get_hw_info(sng_fd_t fd, port_management_struct_t *port_mgmnt, unsigned short port_no)
{
        int err;
        port_mgmnt->command_code = GET_HARDWARE_INFO;
        port_mgmnt->port_no     = port_no;

        err = sangoma_port_mgmnt_ioctl(fd, port_mgmnt);
        if (err) {
				port_mgmnt->operation_status = SANG_STATUS_INVALID_DEVICE;
                return err;
        }

        if (port_mgmnt->operation_status != SANG_STATUS_SUCCESS ) {
                err=port_mgmnt->operation_status;
        }

        return err;
}

int _SAPI_CALL sangoma_driver_port_set_config(sng_fd_t fd, port_cfg_t *port_cfg, unsigned short port_no)
{
	port_cfg->command_code = SET_PORT_VOLATILE_CONFIG;
	port_cfg->port_no	= port_no;

	return sangoma_port_cfg_ioctl(fd, port_cfg);
}

int _SAPI_CALL sangoma_driver_port_get_config(sng_fd_t fd, port_cfg_t *port_cfg, unsigned short port_no)
{

	port_cfg->command_code = GET_PORT_VOLATILE_CONFIG;
	port_cfg->port_no	= port_no;

	return sangoma_port_cfg_ioctl(fd, port_cfg);
}

#endif


#endif /* WANPIPE_TDM_API */
