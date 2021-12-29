/*******************************************************************************//**
 * \file libsangoma-pvt.h
 * \brief LibSangoma private definitios, this file should not be used by applications nor installed -
 *
 * Author(s):	Moises Silva <moy@sangoma.com>
 *
 * Copyright:	(c) 2009 Sangoma Technologies
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
 */

#ifndef _LIBSNAGOMA_PVT_H
#define _LIBSNAGOMA_PVT_H

#include "libsangoma.h"

#ifdef __cplusplus
extern "C" {	/* for C++ users */
#endif

/*!
  \def DFT_CARD
  \brief Default card name. Deprecated not used
 */
#define DFT_CARD "wanpipe1"


#ifndef WP_API_FEATURE_FE_ALARM
#warning "Warning: SANGOMA API FE ALARM not supported by driver"
#endif

#ifndef WP_API_FEATURE_DTMF_EVENTS
#ifdef __WINDOWS__
# pragma message("Warning: SANGOMA API DTMF not supported by driver")
#else
# warning "Warning: SANGOMA API DTMF not supported by driver"
#endif
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

#define SANGOMA_OBJ_IS_SIGNALABLE(object) ((object)->object_type == SANGOMA_GENERIC_WAIT_OBJ \
		                       || (object)->object_type == SANGOMA_DEVICE_WAIT_OBJ_SIG)

#define SANGOMA_OBJ_HAS_DEVICE(object) ((object)->object_type == SANGOMA_DEVICE_WAIT_OBJ \
		                       || (object)->object_type == SANGOMA_DEVICE_WAIT_OBJ_SIG)

#define LIBSNG_MAGIC_NO	0x90547419

/* libsangoma.h defines sangoma_wait_obj_t for the sake of clean prototype definitions, time to undefine it since from now on 
 * the real typedef will be used */
#undef sangoma_wait_obj_t
/*!
  \struct sangoma_wait_obj
  \brief Sangoma wait object structure. Used to setup a poll on a specific device

  This structure is used by sangoma_socket_waitfor_many() function.
  This structure is initialized by sangoma_wait_obj_init() function.

  \typedef sangoma_wait_obj_t
  \brief Sangoma wait object structure. Used to setup a poll on a specific device
*/
typedef struct sangoma_wait_obj
{
	/*****************************************
	 * Private Variables
	 *****************************************/
	int init_flag;

	/*! file descriptor of a device */
	sng_fd_t		fd;

	/*!\brief context provided by the user */	
	void *context;

	/*! type of the object to wait on */
	sangoma_wait_obj_type_t object_type;

#if defined(__WINDOWS__)
	/*! signallable object to wait on events or data from the Sangoma device driver or can be signalled by sangoma_wait_obj_signal(). */
	HANDLE signal_object;
#else
	/*! LINUX: To be used by a pipe to asynchronously break a poll() */
	sng_fd_t signal_write_fd;
	sng_fd_t signal_read_fd;
#endif

} sangoma_wait_obj_t;

/*!
  \def DEV_NAME_LEN
  \brief String length of device name
 */
#define DEV_NAME_LEN	100


#endif	/* _LIBSNAGOMA_H */

