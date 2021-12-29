/*******************************************************************************//**
 * \file libsangoma_hwec.c
 * \brief Hardware Echo Canceller API Code Library for
 *		Sangoma AFT T1/E1/Analog/BRI hardware.
 *
 * Author(s):	David Rokhvarg <davidr@sangoma.com>
 *
 * Copyright:	(c) 2005-2010 Sangoma Technologies Corporation
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
#include "libsangoma-pvt.h"
#include "wanpipe_includes.h"

#ifdef WP_API_FEATURE_LIBSNG_HWEC

#include "wanpipe_events.h"
#include "wanec_api.h"

#if defined (__WINDOWS__)
# include "wanpipe_time.h"	/* wp_sleep() */
# pragma comment( lib, "waneclib" )	/* import functions from waneclib.dll */
#endif/* __WINDOWS__) */

/* Fast sequence of commands to HWEC may cause the chip
 * enter fatal error state, the workaround is to have
 * a guaranteed delay after eache command. */
#define HWEC_CMD_DELAY() wp_usleep(20000)	/* 20ms */

static int libsng_hwec_verbosity_level = 0;

/************************************************************//**
 * Private Functions. (Not exported)
 ***************************************************************/ 

static sangoma_status_t sangoma_hwec_bypass(char *device_name, int enable, unsigned int fe_chan_map)
{
	sangoma_status_t rc;
	wanec_api_hwec_t hwec;

	memset(&hwec, 0, sizeof(wanec_api_hwec_t));

	hwec.enable = enable;
	hwec.fe_chan_map = fe_chan_map;

	/* WAN_EC_API_CMD_HWEC_ENABLE/WAN_EC_API_CMD_HWEC_DISABLE - Controls the "bypass" mode.)*/
	rc = wanec_api_hwec(device_name, libsng_hwec_verbosity_level, &hwec);
	if( rc ) {
		return rc;
	}
	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}


/************************************************************//**
 * Public Functions. (Exported)
 ***************************************************************/ 

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_init(char *device_name)

  \brief Load Firmware image onto EC chip.

  \param device_name Sangoma API device name. 
		Windows: wanpipe1_if1, wanpipe2_if1...
		Linux: wanpipe1, wanpipe2...

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_init(char *device_name)
{
	sangoma_status_t rc;
	wan_custom_param_t custom_parms;
	wanec_api_config_t config;

	memset(&config, 0x00, sizeof(config));
	memset(&custom_parms, 0x0, sizeof(custom_parms));

#if 1
	/* enable acoustic echo cancellation by default */
	strcpy( custom_parms.name, "WANEC_EnableAcousticEcho" );
	strcpy( custom_parms.sValue, "TRUE" );
	
	config.conf.param_no = 1;
	config.conf.params = &custom_parms;
#endif

	/* Load firmware on EC chip */
	rc = wanec_api_config( device_name, libsng_hwec_verbosity_level, &config );
	if( rc ) {
		return rc;
	}
	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}


/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_release(char *device_name)

  \brief Reset internal state of HWEC API.

  \param device_name Sangoma API device name. 
		Windows: wanpipe1_if1, wanpipe2_if1...
		Linux: wanpipe1, wanpipe2...

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_release(char *device_name)
{
	sangoma_status_t rc;
	wanec_api_release_t	release;

	memset(&release, 0, sizeof(wanec_api_release_t));

	rc = wanec_api_release( device_name, libsng_hwec_verbosity_level, &release );
	if( rc ) {
		return rc;
	}
	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}


/*!
	Modify channel operation mode.
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_operation_mode(char *device_name, int mode, unsigned int fe_chan_map)
{
	sangoma_status_t rc;
	wanec_api_opmode_t	opmode;

	memset(&opmode, 0, sizeof(wanec_api_opmode_t));

	opmode.mode = mode;
	opmode.fe_chan_map = fe_chan_map;

	/* modes are:
	WANEC_API_OPMODE_NORMAL,
	WANEC_API_OPMODE_HT_FREEZE,
	WANEC_API_OPMODE_HT_RESET, 
	WANEC_API_OPMODE_POWER_DOWN,
	WANEC_API_OPMODE_NO_ECHO,
	WANEC_API_OPMODE_SPEECH_RECOGNITION.
	*/
	rc = wanec_api_opmode(device_name, libsng_hwec_verbosity_level, &opmode);
	if( rc ) {
		return rc;
	}
	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_power_on (char *device_name,  unsigned int fe_chan_map)

  \brief Set the channel state in the echo canceller to NORMAL/POWER ON.
		 This enables echo cancelation logic inside the chip. 
         The action is internal to EC chip itself, not related to AFT FPGA.
		 This call is slow and should be used only on startup.

  \param device_name Sangoma API device name. 
		Windows: wanpipe1_if1, wanpipe2_if1...
		Linux: wanpipe1, wanpipe2...

  \param fe_chan_map Bitmap of channels (timeslots for Digital,
				lines for Analog) where the call will take effect.

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_power_on(char *device_name,  unsigned int fe_chan_map)
{
	return sangoma_hwec_config_operation_mode(device_name, WANEC_API_OPMODE_NORMAL, fe_chan_map);
}

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_power_off (char *device_name,  unsigned int fe_chan_map)

  \brief  Set the channel state in the echo canceller to POWER OFF.
          This disables echo cancellatio logic inside the chip and
          data passes unmodified through the ec chip.
          The action is internal to EC chip itself, not related
		  to AFT FPGA. This call is slow and should be used only on startup.

  \param device_name Sangoma API device name. 
		Windows: wanpipe1_if1, wanpipe2_if1...
		Linux: wanpipe1, wanpipe2...

  \param fe_chan_map Bitmap of channels (timeslots for Digital,
				lines for Analog) where the call will take effect.

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_power_off(char *device_name,  unsigned int fe_chan_map)
{
	return sangoma_hwec_config_operation_mode(device_name, WANEC_API_OPMODE_POWER_DOWN, fe_chan_map);
}

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_enable(char *device_name,  unsigned int fe_chan_map)

  \brief Redirect audio stream from AFT FPGA to EC chip. 
        This command effectively enables echo cancellation since
		data is now forced through the EC chip by the FPGA. 
		Data will be modified by the echo canceller.
		This command is recommened for fast enabling of Echo Cancellation.
        Note 1: Chip must be configured and in POWER ON state for echo
				Chancellation to take place.
		Note 2: sangoma_tdm_enable_hwec() function can be use to achive
				the same funcitnality based on file descriptor versus
				channel map.

  \param device_name Sangoma API device name. 
		Windows: wanpipe1_if1, wanpipe2_if1...
		Linux: wanpipe1, wanpipe2...

  \param fe_chan_map Bitmap of channels (timeslots for Digital, lines for Analog) where 
		the call will take effect.

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_enable(char *device_name,  unsigned int fe_chan_map)
{
	return sangoma_hwec_bypass(device_name, 1 /* enable */, fe_chan_map);
}


/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_disable(char *device_name, unsigned int fe_chan_map)

  \brief Force AFT FPGA to bypass the echo canceller.  
         This command effectively disables echo cancellation since
		 data will not flowing through the ec chip. 
         Data will not be modified by the echo canceller.
         This command is recommened for fast disabling of Echo Cancelation.
		 Note: sangoma_tdm_disable_hwec() function can be use to achive
				the same functionality based on file descriptor versus
				channel map.

  \param device_name Sangoma API device name. 
		Windows: wanpipe1_if1, wanpipe2_if1...
		Linux: wanpipe1, wanpipe2...

  \param fe_chan_map Bitmap of channels (timeslots for Digital, lines for Analog) where 
		the call will take effect.

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_disable(char *device_name, unsigned int fe_chan_map)
{
	return sangoma_hwec_bypass(device_name, 0 /* disable */, fe_chan_map);;
}

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_channel_parameters(char *device_name,	char *parameter, char *parameter_value, unsigned int channel_map)

  \brief Modify channel configuration parameters.
	This is list of Echo Cancellation channel parameters:

		Channel parameter					Channel parameter value
		=================					=======================
		WANEC_EnableNlp						TRUE | FALSE
		WANEC_EnableTailDisplacement		TRUE | FALSE
		WANEC_TailDisplacement				0-896
		WANEC_SoutLevelControl				TRUE | FALSE
		WANEC_RinAutomaticLevelControl		TRUE | FALSE
		WANEC_SoutAutomaticLevelControl		TRUE | FALSE
		WANEC_SoutAdaptiveNoiseReduction	TRUE | FALSE
		WANEC_RoutNoiseReduction			TRUE | FALSE
		WANEC_ComfortNoiseMode				COMFORT_NOISE_NORMAL
											COMFORT_NOISE_FAST_LATCH
											COMFORT_NOISE_EXTENDED
											COMFORT_NOISE_OFF
		WANEC_DtmfToneRemoval				TRUE | FALSE
		WANEC_AcousticEcho					TRUE | FALSE
		WANEC_NonLinearityBehaviorA			0-13
		WANEC_NonLinearityBehaviorB			0-8
		WANEC_DoubleTalkBehavior			DT_BEH_NORMAL
											DT_BEH_LESS_AGGRESSIVE

  \param fe_chan_map Bitmap of channels (timeslots for Digital, lines for Analog) where 
		the call will take effect.

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_channel_parameters(char *device_name,	char *parameter, char *parameter_value, unsigned int channel_map)
{
	sangoma_status_t rc;
	wanec_api_modify_t channelModify;
	wan_custom_param_t aParms;

	memset(&channelModify, 0x00, sizeof(channelModify));
	memset(&aParms, 0x00, sizeof(aParms));

	channelModify.fe_chan_map = channel_map;
	channelModify.conf.param_no = 1;
	channelModify.conf.params = &aParms;

	strcpy( aParms.name, parameter);
	strcpy( aParms.sValue, parameter_value);

	rc = wanec_api_modify( device_name, libsng_hwec_verbosity_level, &channelModify );
	if( rc ){
		return rc;
	}
	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_tone_detection(char *device_name, int tone_id, int enable, unsigned int fe_chan_map, unsigned char port_map)
  
  \brief Enable/Disable tone detection (such as DTMF) of channels from channel map. 

  \param device_name Sangoma API device name. 
		Windows: wanpipe1_if1, wanpipe2_if1...
		Linux: wanpipe1, wanpipe2...

  \param tone_id See wanpipe_api_iface.h for list of valid tones

  \param enable A flag, if 1 - the specified tone will be detected,
				if 0 - specified tone will not be detected.

  \param fe_chan_map Bitmap of channels (timeslots for Digital, lines for Analog) where 
		the call will take effect.

  \param port_map Port\Direction of tone detection - Rx, Tx. See wanpipe_events.h for
			list of valid ports (WAN_EC_CHANNEL_PORT_SOUT...).

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_tone_detection(char *device_name, int tone_id, int enable, unsigned int fe_chan_map, unsigned char port_map)
{
	sangoma_status_t rc;
	wanec_api_tone_t tone;

	memset(&tone, 0, sizeof(wanec_api_tone_t));

	tone.id		= tone_id;
	tone.enable	= enable;
	tone.fe_chan_map = fe_chan_map;
	tone.port_map	= port_map;
	tone.type_map	= WAN_EC_TONE_PRESENT | WAN_EC_TONE_STOP;

	rc = wanec_api_tone( device_name, libsng_hwec_verbosity_level, &tone );
	if( rc ) {
		return rc;
	}
	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_print_statistics(char *device_name, int full, unsigned int fe_chan_map)

  \brief Read and print Chip/Channel statistics from EC chip. 

  \param device_name Sangoma API device name. 
		Windows: wanpipe1_if1, wanpipe2_if1...
		Linux: wanpipe1, wanpipe2...

  \param full Flag to read full statistics, if set to 1.

  \param fe_chan_map Bitmap of channels (timeslots for Digital, lines for Analog) where 
		the call will read statistics.

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_print_statistics(char *device_name, int full, unsigned int fe_chan_map)
{
	sangoma_status_t rc;
	wanec_api_stats_t stats;

	memset(&stats, 0, sizeof(wanec_api_stats_t));

	stats.full	= full;
	stats.fe_chan = fe_chan_map;
	stats.reset = 0;	/* do not reset */

	rc = wanec_api_stats( device_name, libsng_hwec_verbosity_level, &stats );
	if( rc ) {
		return rc;
	}
	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_audio_buffer_load(char *device_name, char *filename, char pcmlaw, int *out_buffer_id)

  \brief Load audio buffer to EC chip. The buffer can be played out using the sangoma_hwec_audio_buffer_playout() function. 

  \param filename name of the audio file (without the extension). 
				Actual file must have .pcm extension.
				Location:
					Windows: %SystemRoot%\sang_ec_files (ex: c:\WINDOWS\sang_ec_files)
				Linux: /etc/wanpipe/buffers

 \param out_buffer_id when the buffer is loaded on the chip, it is assigned an ID. This ID should
			be used when requesting to play out the buffer.
 
  \return SANG_STATUS_SUCCESS: success, or error status

 */
sangoma_status_t _LIBSNG_CALL sangoma_hwec_audio_buffer_load(char *device_name, char *filename, char pcmlaw, int *out_buffer_id)
{
	sangoma_status_t rc;
	wanec_api_bufferload_t bufferload;

	memset(&bufferload, 0, sizeof(wanec_api_bufferload_t));
	*out_buffer_id = -1;

	bufferload.buffer = filename;
	bufferload.pcmlaw = pcmlaw;

	rc = wanec_api_buffer_load( device_name, libsng_hwec_verbosity_level, &bufferload );
	if( rc ) {
		return rc;
	}

	*out_buffer_id = bufferload.buffer_id;

	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_audio_bufferunload(char *device_name, int in_buffer_id)

  \brief Unload/remove an audio buffer from the HWEC chip.

  \param device_name Sangoma wanpipe device name. (ex: wanpipe1 - Linux; wanpipe1_if1 - Windows).

  \param in_buffer_id	ID of the buffer which will be unloaded. The ID must be initialized by sangoma_hwec_audio_bufferload().

  \return SANG_STATUS_SUCCESS - buffer was successfully unloaded/removed, SANG_STATUS_GENERAL_ERROR - error occured
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_audio_buffer_unload(char *device_name, int in_buffer_id)
{
	sangoma_status_t rc;
	wanec_api_bufferunload_t bufferunload;

	memset(&bufferunload, 0, sizeof(wanec_api_bufferunload_t));

	bufferunload.buffer_id	= (unsigned int)in_buffer_id;

	rc = wanec_api_buffer_unload( device_name, libsng_hwec_verbosity_level, &bufferunload);
	if( rc ) {
		return rc;
	}
	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}

/*!
  \fn sangoma_status_t _LIBSNG_CALL sangoma_hwec_audio_buffer_playout(char *device_name, unsigned int fe_chan_map, 
										unsigned char port_map, int buffer_id, int start, int repeat_cnt, int duration)

  \brief Start\Stop playing out an audio buffer previously loaded by sangoma_hwec_audio_buffer_load().

  \param fe_chan_map Bitmap of channels (timeslots for Digital,
				lines for Analog) where the call will take effect.

  \param port_map Port\Direction where the buffer will be played out.
					This is the channel port on which the buffer will be
					played (WAN_EC_CHANNEL_PORT_SOUT or WAN_EC_CHANNEL_PORT_ROUT)
  
  \param in_buffer_id	ID of the buffer which will be unloaded. The ID must be initialized by sangoma_hwec_audio_bufferload().
  
  \param start	If 1 - start the play out, 0 - stop the play out
		
  \param repeat_cnt Number of times to play out the same buffer
  
  \param duration	Maximum duration of the playout, in milliseconds. If it takes less then 'duration' to
					play out the whole buffer this paramter is ignored.

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_audio_buffer_playout(char *device_name, unsigned int fe_chan_map, 
																unsigned char port, int in_buffer_id, int start,
																int repeat_cnt,	int duration)
{
	sangoma_status_t rc;
	wanec_api_playout_t	playout;

	memset(&playout, 0, sizeof(wanec_api_playout_t));

	playout.start		= start;
	playout.fe_chan		= fe_chan_map;
	playout.buffer_id	= in_buffer_id;
	playout.port		= port;
	playout.notifyonstop	= 1;
	playout.user_event_id	= 0xA5;	/* dummy value */
	playout.repeat_cnt	= repeat_cnt;
	playout.duration	= (duration) ? duration : 5000;	/* default is 5s */

	rc = wanec_api_playout( device_name, libsng_hwec_verbosity_level, &playout);
	if( rc ) {
		return rc;
	}
	HWEC_CMD_DELAY();
	return SANG_STATUS_SUCCESS;
}

/*!
  \fn void _LIBSNG_CALL sangoma_hwec_config_verbosity(int verbosity_level)

  \brief Set Verbosity level of EC API. The level controls amount of data 
			printed to stdout and wanpipelog.txt for diagnostic purposes.

  \param verbosity_level Valid values are from 0 to 3.

  \return SANG_STATUS_SUCCESS: success, or error status
*/
sangoma_status_t _LIBSNG_CALL sangoma_hwec_config_verbosity(int verbosity_level)
{
	if (verbosity_level >= 0 || verbosity_level <= 3) {
		libsng_hwec_verbosity_level = verbosity_level;
		return SANG_STATUS_SUCCESS;
	}
	return SANG_STATUS_INVALID_PARAMETER;
}

#endif /* WP_API_FEATURE_LIBSNG_HWEC */
