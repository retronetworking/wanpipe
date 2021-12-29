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

#include "libsangoma-pvt.h"

static void libsng_dbg(const char * fmt, ...)
{
	va_list args;
	char buf[1024];
	va_start(args, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, args);
#if defined(__WINDOWS__)
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
#define	DBG_EVNT	if(0)libsng_dbg
#define	DBG_ERR		if(0)libsng_dbg("Error: %s() line: %d : ", __FUNCTION__, __LINE__);if(0)libsng_dbg
#define	DBG_INIT	if(0)libsng_dbg

#if defined(__WINDOWS__)

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
  This function will NOT block because using IoctlReadCommandNonBlocking.
 */
static USHORT DoReadCommand(HANDLE drv, RX_DATA_STRUCT * pRx)
{
	DWORD ln, bIoResult;

	bIoResult = DeviceIoControl(
			drv,
			IoctlReadCommandNonBlocking,
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
  \fn static USHORT sangoma_poll_fd(HANDLE drv, API_POLL_STRUCT *api_poll_ptr)
  \brief Non Blocking API Poll function used to find out if Rx Data, Events or
			Free Tx buffer available
  \param drv device file descriptor
  \param api_poll_ptr poll device that stores polling information read/write/event
  \param overlapped pointer to system overlapped io structure.

  Private Windows Function
 */
static USHORT sangoma_poll_fd(sng_fd_t fd, API_POLL_STRUCT *api_poll_ptr)
{
	DWORD ln, bIoResult;

	bIoResult = DeviceIoControl(
			fd,
			IoctlApiPoll,
			(LPVOID)NULL,
			0L,
			(LPVOID)api_poll_ptr,
			sizeof(API_POLL_STRUCT),
			(LPDWORD)(&ln),
			(LPOVERLAPPED)NULL);

	return handle_device_ioctl_result(bIoResult, __FUNCTION__);
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

static int init_sangoma_event_object(sangoma_wait_obj_t *sng_wait_obj, int flags_in)
{
	int event_index = -1;
	
	if(flags_in & POLLIN){
		event_index = LIBSNG_EVENT_INDEX_POLLIN;
	}

	if(flags_in & POLLOUT){
		event_index = LIBSNG_EVENT_INDEX_POLLOUT;
	}

	if(flags_in & POLLPRI){
		event_index = LIBSNG_EVENT_INDEX_POLLPRI;
	}

	if(event_index == -1){
		/* invalid 'flags_in', this should be an assert */
		return SANG_STATUS_GENERAL_ERROR;
	}

	sng_wait_obj->sng_event_objects[event_index].hEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(!sng_wait_obj->sng_event_objects[event_index].hEvent){
		//error
		return SANG_STATUS_GENERAL_ERROR;
	}

	sng_wait_obj->sng_event_objects[event_index].user_flags_bitmap = flags_in;
	if(DoSetSharedEventCommand(sng_wait_obj->fd, &sng_wait_obj->sng_event_objects[event_index])){
		//error
		return SANG_STATUS_GENERAL_ERROR;
	}

	return sng_wait_obj->sng_event_objects[event_index].operation_status;
}

static void sangoma_reset_wait_obj(sangoma_wait_obj_t *sng_wait_obj, int flags_in)
{
	if(flags_in & POLLIN){
		if(sng_wait_obj->sng_event_objects[LIBSNG_EVENT_INDEX_POLLIN].hEvent){
			ResetEvent(sng_wait_obj->sng_event_objects[LIBSNG_EVENT_INDEX_POLLIN].hEvent);
		}
	}

	if(flags_in & POLLOUT){
		if(sng_wait_obj->sng_event_objects[LIBSNG_EVENT_INDEX_POLLOUT].hEvent){
			ResetEvent(sng_wait_obj->sng_event_objects[LIBSNG_EVENT_INDEX_POLLOUT].hEvent);
		}
	}

	if(flags_in & POLLPRI){
		if(sng_wait_obj->sng_event_objects[LIBSNG_EVENT_INDEX_POLLPRI].hEvent){
			ResetEvent(sng_wait_obj->sng_event_objects[LIBSNG_EVENT_INDEX_POLLPRI].hEvent);
		}
	}
}

static sangoma_status_t _SAPI_CALL sangoma_wait_obj_poll(sangoma_wait_obj_t *sangoma_wait_object, int flags_in, int *flags_out)
{
	int err;
	sangoma_wait_obj_t *sng_wait_obj = sangoma_wait_object;
	 /*! api structure used by windows IoctlApiPoll call */
	API_POLL_STRUCT	api_poll;

	*flags_out = 0;

	memset(&api_poll, 0x00, sizeof(API_POLL_STRUCT));
	api_poll.user_flags_bitmap = flags_in;

	/* This call is non-blocking - it will return immediatly. */
	if(sangoma_poll_fd(sng_wait_obj->fd, &api_poll)){
		/* error - ioctl failed */
		return SANG_STATUS_IO_ERROR;
	}

	if(api_poll.operation_status == SANG_STATUS_SUCCESS){
		*flags_out = api_poll.poll_events_bitmap;
		err = 0;
	}else{
		/* error - command failed */
		err = api_poll.operation_status;
	}

	if(*flags_out == 0){
		DBG_POLL("======%s(): *flags_out: 0x%X, flags_in: 0x%X\n", __FUNCTION__, *flags_out, flags_in);
	}
	return err;
}

static int check_number_of_wait_objects(uint32_t number_of_objects, const char *caller_function, int lineno)
{
	if(number_of_objects >= MAXIMUM_WAIT_OBJECTS){
		DBG_ERR("Caller: %s(): Line: %d: 'number_of_objects': %d is greater than the Maximum of: %d\n", 
			caller_function, lineno, number_of_objects, MAXIMUM_WAIT_OBJECTS);
		return 1;
	}
	return 0;
}

static sangoma_status_t get_out_flags(IN sangoma_wait_obj_t *sng_wait_objects[],
									  IN uint32_t in_flags[], OUT uint32_t out_flags[],
									  IN uint32_t number_of_sangoma_wait_objects,
									  IN BOOL	reset_events_if_out_flags_set,
									  OUT BOOL	*at_least_one_poll_set_flags_out)
{
	uint32_t i, j;

	if(at_least_one_poll_set_flags_out){
		*at_least_one_poll_set_flags_out = FALSE;
	}

	for(i = 0; i < number_of_sangoma_wait_objects; i++) {

		sangoma_wait_obj_t *sangoma_wait_object = sng_wait_objects[i];
		if (!SANGOMA_OBJ_HAS_DEVICE(sangoma_wait_object)) {
			continue;
		}

		for(j = 0; j < LIBSNG_NUMBER_OF_EVENT_OBJECTS; j++){

			if(!sangoma_wait_object->sng_event_objects[j].hEvent) {
				continue;
			}

			if(sangoma_wait_obj_poll(sangoma_wait_object, in_flags[i], &out_flags[i])){
				return SANG_STATUS_GENERAL_ERROR;
			}

			if(	out_flags[i] & in_flags[i] ){
				if(TRUE == reset_events_if_out_flags_set){
					sangoma_reset_wait_obj(sangoma_wait_object, out_flags[i]);/* since we are NOT going to wait on this event, reset it 'manually' */
				}
				if(at_least_one_poll_set_flags_out){
					*at_least_one_poll_set_flags_out = TRUE;
				}
			}
		}
	}

	return SANG_STATUS_SUCCESS;
}
#endif	/* __WINDOWS__ */


/*********************************************************************//**
 * Common Linux & Windows Code
 *************************************************************************/



/************************************************************//**
 * Device POLL Functions
 ***************************************************************/ 

/*!
  \fn sangoma_status_t sangoma_wait_obj_create(sangoma_wait_obj_t **sangoma_wait_object, sng_fd_t fd, sangoma_wait_obj_type_t object_type)
  \brief Create a wait object that will be used with sangoma_waitfor_many() API
  \param sangoma_wait_object pointer a single device object 
  \param fd device file descriptor
  \param object_type type of the wait object. see sangoma_wait_obj_type_t for types
  \see sangoma_wait_obj_type_t
  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _SAPI_CALL sangoma_wait_obj_create(sangoma_wait_obj_t **sangoma_wait_object, sng_fd_t fd, sangoma_wait_obj_type_t object_type)
{
	int err = 0;
	sangoma_wait_obj_t *sng_wait_obj;

	if (!sangoma_wait_object) { 
		return SANG_STATUS_INVALID_DEVICE;
	}
	*sangoma_wait_object = NULL;
	sng_wait_obj = malloc(sizeof(**sangoma_wait_object));
	if (!sng_wait_obj) {
		return SANG_STATUS_FAILED_ALLOCATE_MEMORY;
	}

	memset(sng_wait_obj, 0x00, sizeof(*sng_wait_obj));
	/* it is a first initialization of the object */
	sng_wait_obj->init_flag = LIBSNG_MAGIC_NO;

	sng_wait_obj->fd			= fd;
	sng_wait_obj->object_type	= object_type;

#if defined(__WINDOWS__)
	DBG_INIT("%s(): sng_wait_obj ptr: 0x%p\n", __FUNCTION__, sng_wait_obj);
	DBG_INIT("%s(): fd: 0x%X, object_type: %s\n", __FUNCTION__, fd, DECODE_SANGOMA_WAIT_OBJECT_TYPE(object_type));
	DBG_INIT("%s(): sizeof(**sangoma_wait_object): %d\n", __FUNCTION__, sizeof(**sangoma_wait_object));

	if (SANGOMA_OBJ_IS_SIGNALABLE(sng_wait_obj)) {
		sng_wait_obj->generic_event_object.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
		if(!sng_wait_obj->generic_event_object.hEvent){
			return SANG_STATUS_GENERAL_ERROR;
		}
	}

	if(SANGOMA_GENERIC_WAIT_OBJ == object_type){
		/* everything is done for the generic wait object */
		*sangoma_wait_object = sng_wait_obj;
		return SANG_STATUS_SUCCESS;
	}

	err = init_sangoma_event_object(sng_wait_obj, POLLIN /* must be a SINGLE bit because there is a signaling object for each bit */);
	if(SANG_STATUS_SUCCESS != err){
		return err;
	}

	err = init_sangoma_event_object(sng_wait_obj, POLLOUT /* must be a SINGLE bit because there is a signaling object for each bit */);
	if(SANG_STATUS_SUCCESS != err){
		return err;
	}

	err = init_sangoma_event_object(sng_wait_obj, POLLPRI /* must be a SINGLE bit because there is a signaling object for each bit */);
	if(SANG_STATUS_SUCCESS != err) {
		return err;
	}
		
	DBG_INIT("%s(): returning: %d", __FUNCTION__, err);
#else
	int filedes[2];
	if (SANGOMA_OBJ_IS_SIGNALABLE(sng_wait_obj)) {
		sng_wait_obj->signal_read_fd = INVALID_HANDLE_VALUE;
		sng_wait_obj->signal_write_fd = INVALID_HANDLE_VALUE;
		/* if we want cross-process event notification we can use a named pipe with mkfifo() */
		if (pipe(filedes)) {
			return -1;
		}
		sng_wait_obj->signal_read_fd = filedes[0];
		sng_wait_obj->signal_write_fd = filedes[1];
	}
#endif
	*sangoma_wait_object = sng_wait_obj;
	return err;
}

/*!
  \fn sangoma_status_t sangoma_wait_obj_delete(sangoma_wait_obj_t **sangoma_wait_object)
  \brief De-allocate all resources in a wait object
  \param sangoma_wait_object pointer to a pointer to a single device object
  \return SANG_STATUS_SUCCESS on success or some sangoma status error
*/
sangoma_status_t _SAPI_CALL sangoma_wait_obj_delete(sangoma_wait_obj_t **sangoma_wait_object)
{
	sangoma_wait_obj_t *sng_wait_obj = *sangoma_wait_object;
#if defined(__WINDOWS__)
	int index = 0;
#endif

	if(sng_wait_obj->init_flag != LIBSNG_MAGIC_NO){
		/* error. object was not initialized by sangoma_wait_obj_init() */
		return SANG_STATUS_INVALID_DEVICE;
	}

#if defined(__WINDOWS__)
	if (SANGOMA_OBJ_IS_SIGNALABLE(sng_wait_obj)) {
		sangoma_close(&sng_wait_obj->generic_event_object.hEvent);
	}
	if (SANGOMA_OBJ_HAS_DEVICE(sng_wait_obj)) {
		for(index = 0; index < LIBSNG_NUMBER_OF_EVENT_OBJECTS; index++){
			sangoma_close(&sng_wait_obj->sng_event_objects[index].hEvent);
		}
	}
#else
	if (SANGOMA_OBJ_IS_SIGNALABLE(sng_wait_obj)) {
		sangoma_close(&sng_wait_obj->signal_read_fd);
		sangoma_close(&sng_wait_obj->signal_write_fd);
	}
#endif
	sng_wait_obj->init_flag = 0;
	sng_wait_obj->object_type = UNKNOWN_WAIT_OBJ;
	*sangoma_wait_object = NULL;
	return SANG_STATUS_SUCCESS;
}

/*!
  \fn void sangoma_wait_obj_signal(void *sangoma_wait_object)
  \brief Set wait object to a signaled state
  \param sangoma_wait_object pointer a single device object 
  \return 0 on success, non-zero on error
*/
int _SAPI_CALL sangoma_wait_obj_signal(sangoma_wait_obj_t *sng_wait_obj)
{
	if (!SANGOMA_OBJ_IS_SIGNALABLE(sng_wait_obj)) {
		/* even when Windows objects are always signalable for the sake of providing
		 * a consistent interface to the user we downgrade the capabilities of Windows
		 * objects unless the sangoma wait object is explicitly initialized as signalable
		 * */
		return SANG_STATUS_INVALID_DEVICE;
	}
#if defined(__WINDOWS__)
	if(sng_wait_obj->generic_event_object.hEvent){
		if (!SetEvent(sng_wait_obj->generic_event_object.hEvent)) {
			return SANG_STATUS_GENERAL_ERROR;
		}
	}
#else
	/* at this point we know is a signalable object and has a signal_write_fd */
	if (write(sng_wait_obj->signal_write_fd, "s", 1) < 1) {
		return SANG_STATUS_GENERAL_ERROR;
	}
#endif
	return SANG_STATUS_SUCCESS;
}

/*!
  \fn sng_fd_t sangoma_wait_obj_get_fd(void *sangoma_wait_object)
  \brief Retrieve fd device file descriptor which was the 'fd' parameter for sangoma_wait_obj_init()
  \param sangoma_wait_object pointer a single device object 
  \return fd - device file descriptor
*/
sng_fd_t _SAPI_CALL sangoma_wait_obj_get_fd(sangoma_wait_obj_t *sng_wait_obj)
{
	return sng_wait_obj->fd;
}

/*!
  \fn void sangoma_wait_obj_set_context(sangoma_wait_obj_t *sangoma_wait_object)
  \brief Store the given context into provided sangoma wait object.
  \brief This function is useful to associate a context with a sangoma wait object.
  \param sangoma_wait_object pointer a single device object 
  \param context void pointer to user context
  \return void
*/
void _SAPI_CALL sangoma_wait_obj_set_context(sangoma_wait_obj_t *sng_wait_obj, void *context)
{
	sng_wait_obj->context = context;
}

/*!
  \fn void *sangoma_wait_obj_get_context(sangoma_wait_obj_t *sangoma_wait_object)
  \brief Retrieve the user context (if any) that was set via sangoma_wait_obj_set_context.
  \param sangoma_wait_object pointer a single device object 
  \return void*
*/
void* _SAPI_CALL sangoma_wait_obj_get_context(sangoma_wait_obj_t *sng_wait_obj)
{
	return sng_wait_obj->context;
}

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
sangoma_status_t _SAPI_CALL sangoma_waitfor_many(sangoma_wait_obj_t *sng_wait_objects[], uint32_t in_flags[], uint32_t out_flags[],
		uint32_t number_of_sangoma_wait_objects, int32_t system_wait_timeout)
{
#if defined(__WINDOWS__)
	HANDLE hEvents[MAXIMUM_WAIT_OBJECTS];
	int at_least_one_poll_set_flags_out, number_of_internal_signaling_objects, err;
#endif
	uint32_t i = 0, j = 0;

	memset(out_flags, 0x00, number_of_sangoma_wait_objects * sizeof(out_flags[0]));
#if defined(__WINDOWS__)
	/* This loop will calculate 'number_of_internal_signaling_objects' and will initialize 'hEvents'
	 * based on 'number_of_sangoma_wait_objects' and 'in_flags'.  */
	number_of_internal_signaling_objects = 0;
	for(i = 0; i < number_of_sangoma_wait_objects; i++){
		sangoma_wait_obj_t *sangoma_wait_object = sng_wait_objects[i];

		/* if SANGOMA_OBJ_IS_SIGNALABLE add the generic_event_object.hEvent to the hEvents */
		if(sangoma_wait_object->generic_event_object.hEvent){
			if(check_number_of_wait_objects(number_of_internal_signaling_objects, __FUNCTION__, __LINE__)){
				return SANG_STATUS_NO_FREE_BUFFERS;
			}
			hEvents[number_of_internal_signaling_objects] = sangoma_wait_object->generic_event_object.hEvent;
			number_of_internal_signaling_objects++;
		}

		for(j = 0; j < LIBSNG_NUMBER_OF_EVENT_OBJECTS; j++){
			if(sangoma_wait_object->sng_event_objects[j].hEvent){
				if(	((j == LIBSNG_EVENT_INDEX_POLLIN)	&& (in_flags[i] & POLLIN))	||
					((j == LIBSNG_EVENT_INDEX_POLLOUT)	&& (in_flags[i] & POLLOUT))	||
					((j == LIBSNG_EVENT_INDEX_POLLPRI)	&& (in_flags[i] & POLLPRI))	){

					if(check_number_of_wait_objects(number_of_internal_signaling_objects, __FUNCTION__, __LINE__)){
						return SANG_STATUS_NO_FREE_BUFFERS;
					}
					hEvents[number_of_internal_signaling_objects] = sangoma_wait_object->sng_event_objects[j].hEvent;
					number_of_internal_signaling_objects++;
				}
			}/* if () */
		}/* for() */
	}/* for() */

	if(number_of_internal_signaling_objects < 1){
		DBG_ERR("'number_of_internal_signaling_objects': %d is less than the Minimum of: 1!\n",
			number_of_internal_signaling_objects);
		/* error - most likely the user did not initialize sng_wait_objects[] */
		return SANG_STATUS_INVALID_PARAMETER;
	}

	at_least_one_poll_set_flags_out = FALSE;

	/* It is important to get 'out flags' BEFORE the WaitForMultipleObjects()
	 * because it allows to keep API driver's transmit queue full. */
	err = get_out_flags(sng_wait_objects, in_flags, out_flags, number_of_sangoma_wait_objects, TRUE, &at_least_one_poll_set_flags_out);
	if(SANG_ERROR(err)){
		return err;
	}

	if(TRUE == at_least_one_poll_set_flags_out){
		return SANG_STATUS_SUCCESS;
	}

	/* wait untill at least one of the events is signaled OR a 'system_wait_timeout' occured */
	if (WAIT_TIMEOUT == WaitForMultipleObjects(number_of_internal_signaling_objects, &hEvents[0], FALSE, system_wait_timeout)){
		return SANG_STATUS_APIPOLL_TIMEOUT;
	}

	/* WaitForMultipleObjects() was waken by a Sangoma or by a non-Sangoma wait object. */
	err = get_out_flags(sng_wait_objects, in_flags, out_flags, number_of_sangoma_wait_objects, TRUE, NULL);
	if(SANG_ERROR(err)){
		return err;
	}

	return SANG_STATUS_SUCCESS;
#else
	struct pollfd pfds[number_of_sangoma_wait_objects*2]; /* we need twice as many polls because of the sangoma signalable objects */
	char dummy_buf[1];
	int res;
	j = 0;

	memset(pfds, 0, sizeof(pfds));

	for(i = 0; i < number_of_sangoma_wait_objects; i++){

		if (SANGOMA_OBJ_HAS_DEVICE(sng_wait_objects[i])) {
			pfds[i].fd = sng_wait_objects[i]->fd;
			pfds[i].events = in_flags[i];
		}

		if (SANGOMA_OBJ_IS_SIGNALABLE(sng_wait_objects[i])) {
			pfds[number_of_sangoma_wait_objects+j].fd = sng_wait_objects[i]->signal_read_fd;
			pfds[number_of_sangoma_wait_objects+j].events = POLLIN;
			j++;
		}
	}

	poll_try_again:

	res = poll(pfds, (number_of_sangoma_wait_objects + j), system_wait_timeout);
	if (res > 0) {
		for(i = 0; i < number_of_sangoma_wait_objects; i++){
			out_flags[i] = pfds[i].revents;
		}
		for(i = 0; i < j; i++){
			/* TODO: we must do something extra for signalled objects like setting a flag.
			 * current user of the SANGOMA_OBJ_IS_SIGNALABLE feature is Netborder and they dont
			 * need to know which object was signaled. If we want to know which object was signaled
			 * we need a new libsangoma API sangoma_wait_obj_is_signaled() where in Windows we can
			 * use WaitForSingleObject to test the signaled state and in Linux we can set a flag in
			 * sng_wait_obj
			 * */
			if (pfds[number_of_sangoma_wait_objects+i].revents & POLLIN) {
				/* read and discard the signal byte */
				read(pfds[number_of_sangoma_wait_objects+i].fd, &dummy_buf, 1);
			}
		}
	} else if (res < 0 && errno == EINTR) {
		/* TODO: decrement system_wait_timeout */
		goto poll_try_again;
	}
	if (res < 0) {
		return SANG_STATUS_GENERAL_ERROR;
	}
	if (res == 0) {
		return SANG_STATUS_APIPOLL_TIMEOUT;
	}
	return SANG_STATUS_SUCCESS;
#endif
}


/*!
  \fn sangoma_status_t _SAPI_CALL sangoma_waitfor(sangoma_wait_obj_t *sangoma_wait_obj, int32_t inflags, int32_t *outflags, int32_t timeout)
  \brief Wait for a single waitable object
  \param sangoma_wait_obj pointer to array of file descriptors to wait for
  \param timeout timeout in miliseconds in case of no event
  \return SANG_STATUS_APIPOLL_TIMEOUT: timeout, SANG_STATUS_SUCCESS: event occured use sangoma_wait_obj_get_out_flags to check or error status
*/
sangoma_status_t _SAPI_CALL sangoma_waitfor(sangoma_wait_obj_t *sangoma_wait_obj, uint32_t inflags, uint32_t *outflags, int32_t timeout)
{
	return sangoma_waitfor_many(&sangoma_wait_obj, &inflags, outflags, 1, timeout);
}


/************************************************************//**
 * Device OPEN / CLOSE Functions
 ***************************************************************/


int _SAPI_CALL sangoma_span_chan_toif(int span, int chan, char *interface_name)
{
#if defined(__WINDOWS__)
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
#if defined(__WINDOWS__)
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
  snprintf(interface_name, sizeof(interface_name), "/dev/" WP_INTERFACE_NAME_FORM, span, chan);
  endtime.tv_sec += sectimeout;
  do {
    counter = 0;
    while ((rc = stat(interface_name, &statbuf)) && errno == ENOENT && counter != 10) {
      poll(0, 0, 100); // test in 100ms increments
      counter++;
    }
    if (!rc || errno != ENOENT) break;
    if (gettimeofday(&curtime, NULL)) {
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

#if defined(__WINDOWS__)
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
	sprintf(fname,"/dev/%s", tmp_fname);

	return open(fname, O_RDWR);
#endif
}            

sng_fd_t _SAPI_CALL sangoma_open_api_ctrl(void)
{
   	char fname[FNAME_LEN], tmp_fname[FNAME_LEN];

	/* Form the Ctrl Device Name. */
	_snprintf(tmp_fname, DEV_NAME_LEN, WP_CTRL_DEV_NAME);

#if defined(__WINDOWS__)
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
	sprintf(fname,"/dev/%s", tmp_fname);

	return open(fname, O_RDWR);
#endif
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

#if defined(__WINDOWS__)
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
#if defined(__WINDOWS__)
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

#if defined(__WINDOWS__)
	wp_api_hdr_t	*rx_hdr = (wp_api_hdr_t*)hdrbuf;
	wp_api_element_t wp_api_element;

	if(hdrlen != sizeof(wp_api_hdr_t)){
		//error
		DBG_ERR("hdrlen (%i) != sizeof(wp_api_hdr_t) (%i)\n", hdrlen, sizeof(wp_api_hdr_t));
		return -1;
	}

	if(DoReadCommand(fd, &wp_api_element)){
		//error
		DBG_ERR("DoReadCommand() failed! Check messages log.\n");
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
		/* note that SANG_STATUS_NO_DATA_AVAILABLE is NOT an error! */
		if(0)DBG_ERR("Operation Status: %s(%d)\n",
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

	rx_len = read(fd,&msg,sizeof(msg));

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
		/* error. Possible cause is a mismatch between versions of API header files. */
		DBG_ERR("hdrlen (%i) != sizeof(wp_api_hdr_t) (%i)\n", hdrlen, sizeof(wp_api_hdr_t));
		return -1;
	}

#if defined(__WINDOWS__)
	//queue data for transmission
	if(DoWriteCommand(fd, databuf, datalen, hdrbuf, hdrlen)){
		//error
		DBG_ERR("DoWriteCommand() failed!! Check messages log.\n");
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
		DBG_ERR("Operation Status: %s(%d)\n",
			SDLA_DECODE_SANG_STATUS(wp_api_hdr->operation_status), wp_api_hdr->operation_status);
		break;
	}//switch()
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

	bsent = write(fd,&msg,datalen+hdrlen);

	if (bsent == (datalen+hdrlen)){
		wp_api_hdr->wp_api_hdr_operation_status=SANG_STATUS_SUCCESS;
		bsent-=sizeof(wp_api_hdr_t);
	} else if (errno == EBUSY){
		wp_api_hdr->wp_api_hdr_operation_status=SANG_STATUS_DEVICE_BUSY;
	} else {
		wp_api_hdr->wp_api_hdr_operation_status=SANG_STATUS_IO_ERROR;
	}
	wp_api_hdr->wp_api_hdr_data_length=bsent;

	//FIXME - THIS SHOULD BE DONE IN KERNEL
        wp_api_hdr->wp_api_tx_hdr_max_queue_length=16;
        wp_api_hdr->wp_api_tx_hdr_number_of_frames_in_queue=0;

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

#if defined(__WINDOWS__)
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

/* open wanpipe configuration device */
sng_fd_t _SAPI_CALL sangoma_open_driver_ctrl(int port_no)
{
	char fname[FNAME_LEN], tmp_fname[FNAME_LEN];

#if defined(__WINDOWS__)
	/* Form the Config Device Name (i.e. wanpipe1, wanpipe2,...). */
	_snprintf(tmp_fname, DEV_NAME_LEN, WP_PORT_NAME_FORM, port_no);
	_snprintf(fname, FNAME_LEN, "\\\\.\\%s", tmp_fname);

	return CreateFile(	fname, 
						GENERIC_READ | GENERIC_WRITE, 
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						(LPSECURITY_ATTRIBUTES)NULL, 
						OPEN_EXISTING,
						FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
						(HANDLE)NULL
						);
#else
	/* Form the Config Device Name. ("/dev/wanpipe") */
	_snprintf(tmp_fname, DEV_NAME_LEN, WP_CONFIG_DEV_NAME);

	sprintf(fname,"/dev/%s", tmp_fname);

	return open(fname, O_RDWR);
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

	if (port_mgmnt->operation_status != SANG_STATUS_SUCCESS){

		if(port_mgmnt->operation_status == SANG_STATUS_CAN_NOT_STOP_DEVICE_WHEN_ALREADY_STOPPED) {
			err = 0;
		}else{
			err = port_mgmnt->operation_status;
		}
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
