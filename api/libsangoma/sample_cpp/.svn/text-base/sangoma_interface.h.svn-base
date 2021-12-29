/*******************************************************************************//**
 * \file sangoma_interface.h
 * \brief Used with Sample.cpp Code
 * \brief Provides a C++ class that deals with channel IO (read/write/events)
 *
 * Author(s):	David Rokhvarg <davidr@sangoma.com>
 *              Nenad Corbic <ncorbic@sangoma.com>
 *
 *
 * Copyright:	(c) 2005-2009 Sangoma Technologies
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


#ifndef SANGOMA_INTERFACE_H
#define SANGOMA_INTERFACE_H

#include <stdio.h>
#include <stddef.h>		//for offsetof()
#include <stdlib.h>

#if defined(__WINDOWS__)
# include <windows.h>
# include <winioctl.h>
# include <conio.h>
# include "bit_win.h"
# include "wanpipe_time.h" //for usleep()

#elif defined(__LINUX__)

/* Include headers */
# include <stddef.h>
# include <errno.h>
# include <fcntl.h>
# include <string.h>
# include <ctype.h>
# include <sys/stat.h>
# include <sys/ioctl.h>
# include <sys/types.h>
# include <dirent.h>
# include <unistd.h>
# include <sys/socket.h>
# include <netdb.h>
# include <sys/un.h>
# include <sys/wait.h>
# include <unistd.h>
# include <signal.h>
# include <time.h>
#else
# error "sangoma_interface.h: undefined OS type"
#endif

/*
 * FSK CallerID and DTMF detection note:
 *  Stelephony.dll expects input BitStream (aLaw or uLaw audio) from a SINGLE timeslot.
 *  It is always the case for Analog cards, but not for the Digital cards, where
 *  API may provide input from a SINGLE or from MULTIPLE timeslots.
 */
#if defined(_WIN64)
/* temporary not available on 64bit */
# define USE_STELEPHONY_API 0
#else
# define USE_STELEPHONY_API 1
#endif

#include "wanpipe_api.h"
#include "sangoma_cthread.h"
#include "sample.h"

#include <libsangoma.h>

# if USE_STELEPHONY_API
#  include <libstelephony.h>
# endif

/*!
  \class sangoma_interface
  \brief Sangoma Interface Class that deals with span/chan IO (read/write/events)
*/
class sangoma_interface : public sangoma_cthread
{
	
	/*! wait object device for an IO device */
	sangoma_wait_obj_t	sangoma_wait_obj;

	//////////////////////////////////////////////////////////////////
	//receive stuff
	/*!< rx count statistic */
	ULONG		rx_frames_count;
 	/*! rx byte statistic */
	ULONG		rx_bytes_count;
	//for counting frames with CRC/Abort errors
	/*! corrupted frame count statistic */
	ULONG		bad_rx_frames_count;
	/*! Rx thread id used for launching threads that handle rx path */
	DWORD		dwRxThreadId; 

	//////////////////////////////////////////////////////////////////
	//transmit stuff
	/*! tx byte statistic */
	ULONG		tx_bytes_count;
	/*! tx frame statistic */
	ULONG		tx_frames_count;
	/*! tx test byte sent statistic */
	UCHAR		tx_test_byte;

	//////////////////////////////////////////////////////////////////
	//IOCTL management structures and variables
	/*! deprecated: ioctl management structure, used to execute management commands - one should use libsangoma directly */
	wan_udp_hdr_t	wan_udp;

	/*! deprecated: Helper function to get data byte out of wan_udp structure */
	unsigned char get_wan_udphdr_data_byte(unsigned char off);

	/*! deprecated: Helper function to get data pointer out of wan_udp structure */
	unsigned char *get_wan_udphdr_data_ptr(unsigned char off);

	/*! deprecated: Helper function to set data pointer out of wan_udp structure */
	unsigned char set_wan_udphdr_data_byte(unsigned char off, unsigned char data);

	/*! deprecated: protocol command structure size  */
	int				protocol_cb_size;
	/*! device low level protocol (T1/E1...) currently running  */
	int 			wan_protocol;
    /*! device adapter type  */
	unsigned char	adapter_type;

	/*! deprecated: API command structure used to execute API commands  */
	wanpipe_api_cmd_t tdm_api_cmd;

	/*! API command structure used to execute API commands. This command structure is used with libsangoma library */
	wanpipe_api_t wp_api;
	
	/*! Rx Thread function */
	void RxThreadFunc();
#if defined(__LINUX__)
	/*! Tx Thread function */
	void TxThreadFunc();
#endif

	/*! Rx data call back function handler */
	int read_data();
	/*! Rx event call back function handler */
	int read_event();

	/*! Debug function to print rx data */
	void PrintRxData(wp_api_element_t *pRx);

	/*! Debug function to print rx event */
	void PrintTdmApiEvent(wp_api_element_t *pRx);

	/*! Shutdown function to cleanup the class */
	void cleanup();

	/*! deprecated: Developmnet API structure used to read write low level memory */
	wan_cmd_api_t		wanpipe_api_cmd;
	
	/*! Get device span configuration */
	int get_wan_config();

	/*! Get interface chan configuration */
	int get_interface_configuration(if_cfg_t *wanif_conf_ptr);

	/*! Get front end type (T1/E1...) */
	int get_fe_type(unsigned char* adapter_type);

	/*! deprecated: Execute API command IOCTL - one should use libsangoma directly */
	int wanpipe_api_ioctl(wan_cmd_api_t *api_cmd);

	/*! deprecated: Read data from device */ 
	int		DoReadCommand(sng_fd_t drv, wp_api_element_t * pRx);

	/*! deprecated: Write data to device */
	int		DoWriteCommand(sng_fd_t drv, wp_api_element_t * pTx);

	/*! deprecated: Set Tx Idle buffer */
	UCHAR	DoSetIdleTxBufferCommand(sng_fd_t drv, wp_api_element_t *pTx);

	/*! Bert test pattern */
	unsigned char wp_brt[256];

	/*! deprecated: Generate bit reversal table - not used with new sangoma cards */
	void generate_bit_rev_table();

	/*! Flag indicating that rbs is enabled */
	char is_rbs_monitoring_enabled;

#if USE_STELEPHONY_API
	/*! Stelephony provides Analog CallierID and DTMF encoding/decoding */
	stelephony_callback_functions_t scf;
	void *stelObj;
	void *dtmfBuffer;
#endif

	//////////////////////////////////////////////////////////////////
	//data
	char					terminate_tx_rx_threads;

	/*! Maximum Rx Data buffer to copy rx data into */
	wp_api_element_t			rx_data;
	/*! Maximum Tx Data buffer to copy tx data into */
	wp_api_element_t			tx_data;

	/*! Callback functions used to call the application on IO events */
	callback_functions_t	callback_functions;

	/*! Wanpipe Interface configuration structure to be populated on read configuration */
	if_cfg_t				wanif_conf_struct;

	/*! Wanpipe Device number based on hardware probe device enumeration (SPAN) */
	int WanpipeNumber;

	/*! Wanpipe Interface number based on a channel in a SPAN */
	int InterfaceNumber;

protected:
	virtual unsigned long threadFunction(struct ThreadParam& thParam);

public:
	char				device_name[DEV_NAME_LEN];

	//////////////////////////////////////////////////////////////////
	//methods
	sangoma_interface(int wanpipe_number, int interface_number);
	~sangoma_interface();

	int DoManagementCommand(sng_fd_t drv, wan_udp_hdr_t* packet);

	int init(callback_functions_t *callback_functions_ptr);
	int run();
	int stop();

	void bit_swap_a_buffer(unsigned char *data, int len);

	void get_te1_56k_stat(void);
	void set_lb_modes(unsigned char type, unsigned char mode);
	int loopback_command(u_int8_t type, u_int8_t mode, u_int32_t chan_map);

	int get_operational_stats(wanpipe_chan_stats_t *stats);
	int flush_operational_stats (void);

	int enable_rbs_monitoring();
	char get_rbs(rbs_management_t *rbs_management_ptr);
	char set_rbs(rbs_management_t *rbs_management_ptr);

	int tdm_enable_rbs_events(int polls_per_second);
	int tdm_disable_rbs_events();

	int set_tx_idle_flag(unsigned char new_idle_flag);
	int get_open_handles_counter();

	//remove all data from API driver's transmit queue
	int flush_tx_buffers (void);

	sng_fd_t open_api_device();

	unsigned char get_adapter_type();
	unsigned int get_sub_media();
	void set_fe_debug_mode(sdla_fe_debug_t *fe_debug);

	void get_api_driver_version(PDRIVER_VERSION version);

	void get_card_customer_id(u_int8_t *customer_id);

	int transmit(wp_api_element_t *pTx);
	int receive (wp_api_element_t *pRx);

#if USE_STELEPHONY_API
#if defined (__LINUX__)
	int resetFSKCID(void);
	int sendCallerID(LPCTSTR name, LPCTSTR number);
	int sendSwDTMF(char dtmf_char);
	int setDTMFBuffer(void* pBuffer);
#endif
#endif

	//////////////////////////////////////////////////////////////////
	//TDM API calls
	int tdm_enable_rxhook_events();
	int tdm_disable_rxhook_events();

	/* DTMF Detection on A200 Analog card (SLIC) chip */
	int tdm_enable_rm_dtmf_events();
	int tdm_disable_rm_dtmf_events();

	/* DTMF Detection on Octasic chip */
	int tdm_enable_dtmf_events(uint8_t channel);
	int tdm_disable_dtmf_events(uint8_t channel);

	int tdm_enable_ring_detect_events();
	int tdm_disable_ring_detect_events();

	int tdm_enable_ring_trip_detect_events();
	int tdm_disable_ring_trip_detect_events();

	int tdm_enable_ring_events();
	int tdm_disable_ring_events();

	int tdm_txsig_onhook();
	int tdm_txsig_offhook();

	int tdm_enable_tone_events(uint16_t tone_id);
	int tdm_disable_tone_events();

	int tdm_front_end_activate();
	int tdm_front_end_deactivate();

	int tdm_control_flash_events(int rxflashtime);

	/* get current state of the line - is it Connected or Disconnected */
	int tdm_get_front_end_status(unsigned char *status);

	int tdm_set_user_period(unsigned int usr_period);
	//////////////////////////////////////////////////////////////////

	int tdmv_api_ioctl(wanpipe_api_cmd_t *api_cmd);

	int reset_interface_state();

	int start_ring_tone();
	int stop_ring_tone();

	int start_congestion_tone();
	int stop_congestion_tone();

	int start_busy_tone();
	int stop_busy_tone();

	int stop_all_tones();

	int start_dial_tone();
	int stop_dial_tone();

	int start_ringing_phone();
	int stop_ringing_phone();

	int fxo_go_off_hook();
	int fxo_go_on_hook();

	//BRI only:
	int tdm_enable_bri_bchan_loopback(u_int8_t channel);
	int tdm_disable_bri_bchan_loopback(u_int8_t channel);

};


#if defined(__WINDOWS__)

#define HANDLE_DEVICE_IOCTL_RESULT(bResult)\
{ \
	if(bResult == 0){ \
		/* check message log */ \
		printf("%s(): Line: %d: Error!!\n", __FUNCTION__, __LINE__); \
		DecodeLastError(__FUNCTION__); \
		return 1; \
	} \
}

#endif /*__WINDOWS__*/

#endif//SANGOMA_INTERFACE_H

