/*******************************************************************************//**
 * \file sample.cpp
 * \brief  WANPIPE(tm) API C++ Sample Code
 *
 * Author(s):	David Rokhvarg <davidr@sangoma.com>
 *				Nenad Corbic <ncorbic@sangoma.com>
 *
 * Copyright:	(c) 2005-2009  Sangoma Technologies
 *
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
 *
 */

#include "sangoma_port.h"
#include "sangoma_port_configurator.h"	
#include "sangoma_interface.h"
#include "sangoma_api_ctrl_dev.h"

#if defined(__LINUX__)
#include "sample_linux_compat.h"
#endif

#ifndef MAX_PATH
#define MAX_PATH 100
#endif


/*****************************************************************
 * Global Variables
 *****************************************************************/

wp_program_settings_t	program_settings;
callback_functions_t 	callback_functions;



/*****************************************************************
 * Prototypes & Defines
 *****************************************************************/

static int got_rx_data(void *sang_if_ptr, void *rx_data);
static void got_TdmApiEvent(void *sang_if_ptr, void *event_data);

typedef struct{
	void				*sang_if_ptr;
	wp_api_event_t		event;
}TDM_API_EVENT_THREAD_PARAM;


#if USE_STELEPHONY_API
//Sangoma Telephony API (Stelephony.dll) provides the following telephony services:
//1. FSK Caller ID detection for Analog FXO.
//2. Software DTMF detection.
//3. Q931 decoding
static void FSKCallerIDEvent(void *callback_context, LPCTSTR Name, LPCTSTR CallerNumber, LPCTSTR CalledNumber, LPCTSTR DateTime);
static void DTMFEvent(void *callback_context, long Key);
static void Q931Event(void *callback_context, stelephony_q931_event *pQ931Event);
#if defined (__LINUX__)
static void FSKCallerIDTransmit (void *callback_context, void* buffer);
static void SwDTMFBuffer (void *callback_context, void* buffer);
#endif
#endif

//critical section for synchronizing access to 'stdout' between the threads
CRITICAL_SECTION	PrintCriticalSection;
//critical section for TDM events
CRITICAL_SECTION	TdmEventCriticalSection;

#if defined (__WINDOWS__)
DWORD TdmApiEventThreadFunc(LPDWORD lpdwParam);
#else
void *TdmApiEventThreadFunc(void *lpdwParam);
#endif


/*****************************************************************
 * Debugging Macros
 *****************************************************************/

#define DBG_MAIN	if(1)printf
#define ERR_MAIN	if(1)printf
#define INFO_MAIN	if(1)printf

static int set_port_configuration();

/*!
  \fn sangoma_interface* init(int wanpipe_number, int interface_number)
  \brief Create a sangoma_interface class and setup callback functions
  \param wanpipe_number wanpipe device number obtained from the hardware probe info, span
  \param interface_number wanpipe interface number, corresponds to a channel in a span
  \return sangoma_interface object pointer - ok  or NULL if error.
*/

sangoma_interface* init(int wanpipe_number, int interface_number)
{
	sangoma_interface	*sang_if = NULL;
	DBG_MAIN("init()\n");

	if(program_settings.use_ctrl_dev == 1){
		sang_if = new sangoma_api_ctrl_dev();
	}else{
		sang_if = new sangoma_interface(wanpipe_number, interface_number);
	}

	if(sang_if->init(&callback_functions)){
		delete sang_if;
		return NULL;
	}
	DBG_MAIN("init(): OK\n");
	return sang_if;
}

/*!
  \fn void cleanup(sangoma_interface	*sang_if)
  \brief Free Sangoma Interface Object
  \param sang_if Sangoma interface object pointer
  \return void
*/
void cleanup(sangoma_interface	*sang_if)
{
	DBG_MAIN("cleanup()\n");
	if(sang_if){
		delete sang_if;
	}
}

/*!
  \fn int start(sangoma_interface	*sang_if)
  \brief Run the main sangoma interface hanlder code
  \param sang_if Sangoma interface object pointer
  \return 0 - ok  Non Zero - Error
*/
int start(sangoma_interface	*sang_if)
{
	DBG_MAIN("start()\n");
	return sang_if->run();
}

/*!
  \fn void stop(sangoma_interface	*sang_if)
  \brief Stop the Sangoma Interface Object
  \param sang_if Sangoma interface object pointer
  \return void
*/
void stop(sangoma_interface	*sang_if)
{
	DBG_MAIN("stop()\n");
	sang_if->stop();
}

/*!
  \fn void PrintRxData(wp_api_element_t* pRx)
  \brief Debug function used to print Rx Data
  \param pRx API data element strcutre containt header + data 
  \return void
*/
void PrintRxData(wp_api_element_t* pRx)
{
	wp_api_hdr_t*	pri;
	USHORT			datlen;
	PUCHAR			data;
	static unsigned int	rx_counter = 0;

	pri = &pRx->hdr;

	//NOTE: if running in BitStream mode, there will be TOO MUCH to print 
	datlen = pri->data_length;
	data =  pRx->data;

	rx_counter++;
	if(program_settings.silent){
		if((rx_counter % 1000) == 0){
			INFO_MAIN("Rx counter:%d, Rx datlen : %d\n", rx_counter, datlen);
		}
		return;
	}else{
		INFO_MAIN("Rx counter:%d, Rx datlen : %d. Data :\n", rx_counter, datlen);
	}

#if 1
	for(int ln = 0; ln < datlen; ln++){
		if((ln % 20 == 0)){
			if(ln){
				INFO_MAIN("\n");
			}
			INFO_MAIN("%04d ", ln/20);
		}
		INFO_MAIN("%02X ", data[ln]);
	}
	INFO_MAIN("\n");
#endif
}


/*!
  \fn static int got_rx_data(void *sang_if_ptr, void *rx_data)
  \brief Callback function indicating data rx is pending
  \param sang_if_ptr sangoma interface pointer
  \param rx_data API data element strcutre containt header + data
  \return 0 - ok  non-zero - Error
*/
static int got_rx_data(void *sang_if_ptr, void *rx_data)
{
	sangoma_interface *sang_if = (sangoma_interface*)sang_if_ptr;

#if 0
#ifdef __LINUX__
	static struct timeval tv_start;
	static int elapsed_b4=0;
	struct timeval last;
	int elapsed;

	last=tv_start;
	gettimeofday(&tv_start, NULL);
	elapsed = abs(elapsed_b4);
	elapsed_b4 = abs((((last.tv_sec * 1000) + last.tv_usec / 1000) - ((tv_start.tv_sec * 1000) + tv_start.tv_usec / 1000)));
	if (abs(elapsed - elapsed_b4) > 1) {
		INFO_MAIN("wanpipe%d: Elapsed %i %i diff=%i\n", program_settings.wanpipe_number, elapsed,elapsed_b4,abs(elapsed-elapsed_b4));
	}
#endif
#endif
	//Do something with data received from Sangoma interface.
	//Fore example, transimit back everything what was received:

	if(program_settings.Rx_to_Tx_loopback == 1){
		sang_if->transmit((wp_api_element_t*)rx_data);
	}

	EnterCriticalSection(&PrintCriticalSection);
	PrintRxData((wp_api_element_t*)rx_data);
	LeaveCriticalSection(&PrintCriticalSection);
	return 0;
}

/*!
  \fn static void got_TdmApiEvent(void *sang_if_ptr, void *event_data)
  \brief Callback function indicating Event is pending.
  \param sang_if_ptr sangoma interface pointer
  \param event_data  API event element strcutre containt header + data
  \return 0 - ok  non-zero - Error

   Currently Windows launches a thread to handle the event, where
   Linux handles the event directly.  Implementation is left to the user.
*/
static void got_TdmApiEvent(void *sang_if_ptr, void *event_data)
{
	TDM_API_EVENT_THREAD_PARAM	*param =
		(TDM_API_EVENT_THREAD_PARAM*)malloc(sizeof(TDM_API_EVENT_THREAD_PARAM));

	if(param == NULL){
		ERR_MAIN("Failed to allocate memory for 'Event Thread parameter'!!\n");
		return;
	}

	memcpy(&param->event, event_data, sizeof(wp_api_event_t));
	param->sang_if_ptr = sang_if_ptr;

	//////////////////////////////////////////////////////////////////////
	//Handling of Events must be done OUTSIDE of the REAL-TIME Rx thread//
	//because it may make take a lot of time.							//
	//Create a special thread for Event hadling.						//
	//////////////////////////////////////////////////////////////////////
#if defined(__WINDOWS__)
	DWORD	dwThreadId;

	if(CreateThread(
        NULL,                       /* no security attributes        */ 
        0,                          /* use default stack size        */ 
        (LPTHREAD_START_ROUTINE)TdmApiEventThreadFunc, /* thread function     */ 
        param,						/* argument to thread function   */ 
        0,                          /* use default creation flags    */ 
        &dwThreadId					/* returns the thread identifier */ 
		) == NULL){
		ERR_MAIN("Failed to create 'TdmApiEvent' thread!!\n");
	}
#else
	//FIXME: implement the thread. Consider using sangoma_cthread class.
	TdmApiEventThreadFunc(param);
#endif
}

/*!
  \fn void *TdmApiEventThreadFunc(void *lpdwParam)
  \brief Event handling Function
  \param lpdwParam pointer to the span/chan device
  \return void
*/
#if defined(__WINDOWS__)
DWORD TdmApiEventThreadFunc(LPDWORD lpdwParam)
#else
void *TdmApiEventThreadFunc(void *lpdwParam)
#endif
{ 
	TDM_API_EVENT_THREAD_PARAM	*param;
	sangoma_interface			*sang_if;
	wp_api_event_t			*wp_tdm_api_event;

	EnterCriticalSection(&TdmEventCriticalSection);

	param = (TDM_API_EVENT_THREAD_PARAM*)lpdwParam;

	wp_tdm_api_event = &param->event;
	sang_if = (sangoma_interface*)param->sang_if_ptr;

	DBG_MAIN( "TdmApiEventThreadFunc():ifname: %s: Channel: %d\n",
		sang_if->device_name, wp_tdm_api_event->channel);

	switch(wp_tdm_api_event->wp_api_event_type)
	{
	case WP_API_EVENT_DTMF:/* DTMF detected by Hardware */
		DBG_MAIN("DTMF Event: Digit: %c (Port: %s, Type:%s)!\n",
			wp_tdm_api_event->wp_api_event_dtmf_digit,
			(wp_tdm_api_event->wp_api_event_dtmf_port == WAN_EC_CHANNEL_PORT_ROUT)?"ROUT":"SOUT",
			(wp_tdm_api_event->wp_api_event_dtmf_type == WAN_EC_TONE_PRESENT)?"PRESENT":"STOP");

		break;

	case WP_API_EVENT_RXHOOK:
		DBG_MAIN("RXHOOK Event: %s! (0x%X)\n", 
			WAN_EVENT_RXHOOK_DECODE(wp_tdm_api_event->wp_api_event_hook_state),
			wp_tdm_api_event->wp_api_event_hook_state);
		break;

	case WP_API_EVENT_RING_DETECT:
		DBG_MAIN("RING Event: %s! (0x%X)\n",
			WAN_EVENT_RING_DECODE(wp_tdm_api_event->wp_api_event_ring_state),
			wp_tdm_api_event->wp_api_event_ring_state);
		break;

	case WP_API_EVENT_RING_TRIP_DETECT:
		DBG_MAIN("RING TRIP Event: %s! (0x%X)\n", 
			WAN_EVENT_RING_TRIP_DECODE(wp_tdm_api_event->wp_api_event_ring_state),
			wp_tdm_api_event->wp_api_event_ring_state);
		break;

	case WP_API_EVENT_RBS:
		DBG_MAIN("RBS Event: New bits: 0x%X!\n",	wp_tdm_api_event->wp_api_event_rbs_bits);
		DBG_MAIN( "RX RBS: A:%1d B:%1d C:%1d D:%1d\n",
			(wp_tdm_api_event->wp_api_event_rbs_bits & WAN_RBS_SIG_A) ? 1 : 0,
			(wp_tdm_api_event->wp_api_event_rbs_bits & WAN_RBS_SIG_B) ? 1 : 0,
			(wp_tdm_api_event->wp_api_event_rbs_bits & WAN_RBS_SIG_C) ? 1 : 0,
			(wp_tdm_api_event->wp_api_event_rbs_bits & WAN_RBS_SIG_D) ? 1 : 0);
		break;

	case WP_API_EVENT_LINK_STATUS:
		DBG_MAIN("Link Status Event: %s! (0x%X)\n", 
			WAN_EVENT_LINK_STATUS_DECODE(wp_tdm_api_event->wp_api_event_link_status),
			wp_tdm_api_event->wp_api_event_link_status);
		break;

	case WP_API_EVENT_ALARM:
		DBG_MAIN("New Alarm State: 0x%X\n", wp_tdm_api_event->wp_api_event_alarm);
		break;

	default:
		ERR_MAIN("Unknown TDM API Event: %d\n", wp_tdm_api_event->wp_api_event_type);
		break;
	}

	free(lpdwParam);
	LeaveCriticalSection(&TdmEventCriticalSection);
	//Done with the Event, exit the thread.
	return 0;
}


/*!
  \fn int tx_file(sangoma_interface *sang_if)
  \brief Transmit a file on a sangoma interface / device.
  \param sang_if_ptr sangoma interface pointer
  \return 0 - ok  non-zero - Error

*/
int tx_file(sangoma_interface *sang_if)
{
	FILE			*pFile;
	unsigned int	tx_counter=0, bytes_read_from_file, total_bytes_read_from_file=0;
	wp_api_element_t	local_tx_data;
	int err=0;

	pFile = fopen( program_settings.szTxFileName, "rb" );
	if( pFile == NULL){
		ERR_MAIN( "Can't open file: [%s]\n", program_settings.szTxFileName );
		return 1;
	}

	do
	{
		//read tx data from the file. if 'bytes_read_from_file != txlength', end of file is reached
		bytes_read_from_file = fread( local_tx_data.data, 1, program_settings.txlength /* MTU size */, pFile );
		total_bytes_read_from_file += bytes_read_from_file;

		local_tx_data.hdr.data_length = program_settings.txlength;//ALWAYS transmit MTU size over the BitStream/Voice interface
		local_tx_data.hdr.operation_status = SANG_STATUS_TX_TIMEOUT;
		
retry_tx:
		err=sang_if->transmit(&local_tx_data);
		if (err == SANG_STATUS_DEVICE_BUSY) {
			INFO_MAIN("Tx busy retry\n");
			usleep(100);	
			goto retry_tx;
		}

		if(err != SANG_STATUS_SUCCESS){
			//error
			break;
		}
		
		tx_counter++;
		
		//DBG_MAIN("tx_counter: %u\r",tx_counter);

	}while(bytes_read_from_file == program_settings.txlength);

	INFO_MAIN("Finished transmitting file \"%s\" (tx_counter: %u, total_bytes_read_from_file: %d)\n",
		program_settings.szTxFileName, tx_counter, total_bytes_read_from_file);
	fclose( pFile );

	return 0;
}

/*!
  \fn static int get_user_decimal_number()
  \brief Get user DECIMAL input in integer format
  \return user inputed integer
*/
static int get_user_decimal_number()
{
	int result = 1;
	int retry_counter = 0;

	while(scanf("%d", &result) == 0){
		fflush( stdin );
		INFO_MAIN("\nError: Not a numerical input!!\n");
		if(retry_counter++ > 10){
			INFO_MAIN("giving up...\n");
			result = 1;
			break;
		}
	}//while()

	INFO_MAIN("User input: %d\n", result);
	return result;
}

/*!
  \fn static int get_user_hex_number()
  \brief Get user HEX input in integer format
  \return user inputed integer
*/
static int get_user_hex_number()
{
	int result = 1;
	int retry_counter = 0;

	while(scanf("%x", &result) == 0){
		fflush( stdin );
		INFO_MAIN("\nError: Not a HEX input!!\n");
		if(retry_counter++ > 10){
			INFO_MAIN("giving up...\n");
			result = 1;
			break;
		}
	}//while()

	INFO_MAIN("User input: 0x%X\n", result);
	return result;
}

/*!
  \fn static int parse_command_line_args(int argc, char* argv[])
  \brief Parse input arguments
  \param argc number of arguments
  \param argv argument list
  \return 0 - ok  non-zero - Error

*/
static int parse_command_line_args(int argc, char* argv[])
{
	int i;
	const char *USAGE_STR =
"\n"
"Usage: sample [-c] [-i] [-silent]\n"
"\n"
"Options:\n"
"\t-c		number	Wanpipe number: 1,2,3...\n"
"\t-i		number	Interface number 0,1,2,3,....\n"
"\t-driver_config	configure start/stop driver using volatile....\n"
"\t-silent			Disable display of Rx data\n"
"\t-rx2tx			All received data automatically transmitted on the SAME interface\n"
"\t-txlength\tnumber\tLength of data frames to be transmitted when 't' key is pressed\n"
"\t-txcount\tnumber	Number of test data frames to be transmitted when 't' key is pressed\n"
"\t-tx_file_name\tstring\tFile to be transmitted when 't' key is pressed\n"
#if USE_STELEPHONY_API
"\t-decode_fsk_cid\t\tDecode FSK Caller ID on an Analog line. For Voice data only.\n"
"\t-encode_fsk_cid\t\tEncode FSK Caller ID on an Analog line. For Voice data only.\n"
"\t-encode_sw_dtmf\t\tEncode SW DTMF on an line. For Voice data only.\n"
"\t-sw_dtmf			Enable Sangoma Software DTMF decoder. For Voice data only.\n"
"\t-decode_q931		Enable Sangoma Q931 decoder. For HDLC (Dchannel) data only.\n"
#endif
"\t-use_ctrl_dev	Use the global 'wptdm_ctrl' device to Get/Set events.\n"
"\n"
"Example: sample -c 1 -i 1\n";

	memset(&program_settings, 0, sizeof(wp_program_settings_t));
	program_settings.wanpipe_number = 1;
	program_settings.interface_number = 1;
	program_settings.txlength = 128;
	program_settings.txcount = 1;

	
	for(i = 0; i < argc; i++){
	
		if(_stricmp(argv[i], "-silent") == 0){
			INFO_MAIN("disabling Rx data display...\n");
			program_settings.silent = 1;
		}else if(_stricmp(argv[i], "help") == 0 || _stricmp(argv[i], "?") == 0 || _stricmp(argv[i], "/?") == 0){
			INFO_MAIN(USAGE_STR);
			return 1;
		}else if(_stricmp(argv[i], "-c") == 0){
			if (i+1 > argc-1){
				INFO_MAIN("No Wanpipe number was provided!\n");
				return 1;
			}
			program_settings.wanpipe_number = (uint16_t)atoi(argv[i+1]);
			INFO_MAIN("Using wanpipe number %d\n", program_settings.wanpipe_number);
		}else if(_stricmp(argv[i], "-i") == 0){
			if (i+1 > argc-1){
				INFO_MAIN("No Interface number was provided!\n");
				return 1;
			}
			program_settings.interface_number = (uint16_t)atoi(argv[i+1]);
			INFO_MAIN("Using interface number %d\n", program_settings.interface_number);
			if(program_settings.interface_number < 1){
				ERR_MAIN("Invalid interface number %d!!\n", program_settings.interface_number);
				return 1;
			}
		}else if(strcmp(argv[i], "-rx2tx") == 0){
			INFO_MAIN("enabling Rx to Tx loopback...\n");
			program_settings.Rx_to_Tx_loopback = 1;
		}else if(strcmp(argv[i], "-driver_config") == 0){
			INFO_MAIN("enabling driver config start/stop\n");
			program_settings.driver_config = 1;

		}else if(_stricmp(argv[i], "-txlength") == 0){
			if (i+1 > argc-1){
				INFO_MAIN("No txlength provided!\n");
				return 1;
			}
			program_settings.txlength = (uint16_t)atoi(argv[i+1]);
			INFO_MAIN("Setting txlength to %d bytes.\n", program_settings.txlength);
		}else if(_stricmp(argv[i], "-txcount") == 0){
			if (i+1 > argc-1){
				INFO_MAIN("No txcount provided!\n");
				return 1;
			}
			program_settings.txcount = atoi(argv[i+1]);
			INFO_MAIN("Setting txcount to %d.\n", program_settings.txcount);
#if USE_STELEPHONY_API
		}else if(_stricmp(argv[i], "-decode_fsk_cid") == 0){
			INFO_MAIN("enabling FSK Caller ID decoder...\n");
			program_settings.decode_fsk_cid = 1;
			callback_functions.FSKCallerIDEvent = FSKCallerIDEvent;
		}else if(_stricmp(argv[i], "-sw_dtmf") == 0){
			INFO_MAIN("enabling Software DTMF decoder...\n");
			program_settings.sw_dtmf = 1;
			callback_functions.DTMFEvent = DTMFEvent;
		}else if(_stricmp(argv[i], "-decode_q931") == 0){
			INFO_MAIN("enabling Q931 decoder...\n");
			program_settings.decode_q931 = 1;
			callback_functions.Q931Event = Q931Event;
#if defined (__LINUX__)
		}else if(_stricmp(argv[i], "-encode_fsk_cid") == 0){
			INFO_MAIN("enabling FSK Caller ID encoder...\n");
			program_settings.encode_fsk_cid = 1;
			callback_functions.FSKCallerIDTransmit = FSKCallerIDTransmit;
		}else if(_stricmp(argv[i], "-encode_sw_dtmf") == 0){
			INFO_MAIN("enabling Software DTMF encoder...\n");
			program_settings.encode_sw_dtmf = 1;
			callback_functions.SwDTMFBuffer = SwDTMFBuffer;
#endif
		}else if(_stricmp(argv[i], "-alaw") == 0){
			INFO_MAIN("enabling ALaw codec...\n");
			program_settings.voice_codec_alaw = 1;
#endif//USE_STELEPHONY_API
		}else if(_stricmp(argv[i], "-tx_file_name") == 0){
			if (i+1 > argc-1){
				INFO_MAIN("No TxFileName provided!\n");
				return 1;
			}
			strcpy(program_settings.szTxFileName, argv[i+1]);
			INFO_MAIN("Setting szTxFileName to '%s'.\n", program_settings.szTxFileName);
		}else if(_stricmp(argv[i], "-use_ctrl_dev") == 0){
			INFO_MAIN("Using ctrl_dev...\n");
			program_settings.use_ctrl_dev = 1;
		}
	}
	return 0;
}


/*!
  \fn int main(int argc, char* argv[])
  \brief Main function start of the applicatoin
  \param argc number of arguments
  \param argv argument list
  \return 0 - ok  non-zero - Error

  Get user Input
  Set program settings based on user input
  Create SangomaInterface Class based on user span/chan input.
  Bind callback functions read/event to SangomaInteface class.
  Execute the SangomaInterface handling function -> start()
  The start function will read/write/event data.
  In Main thread prompt the user for commands.
*/
int __cdecl main(int argc, char* argv[])
{
	int		rc, user_selection,err;
	sangoma_interface	*sang_if = NULL;
	wp_api_element_t		local_tx_data;
	UCHAR				tx_test_byte = 0;

	////////////////////////////////////////////////////////////////////////////
	memset(&callback_functions, 0x00, sizeof(callback_functions_t));
	callback_functions.got_rx_data = got_rx_data;
	callback_functions.got_TdmApiEvent = got_TdmApiEvent;

	////////////////////////////////////////////////////////////////////////////
	if(parse_command_line_args(argc, argv)){
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////
	//An OPTIONAL step of setting the port configuration to different values from
	//what is set in "Device Manager"-->"Sangoma Hardware Abstraction Driver".

	//set port configration and exit
	if (program_settings.driver_config) {
		err=set_port_configuration();
		if (err) {
			return err;
		}
	}

	////////////////////////////////////////////////////////////////////////////
	//initialize critical section objects
	InitializeCriticalSection(&PrintCriticalSection);
	InitializeCriticalSection(&TdmEventCriticalSection);

	////////////////////////////////////////////////////////////////////////////
	//User may provide Wanpipe Number and Interface Number as a command line arguments:
	INFO_MAIN("Using wanpipe_number: %d, interface_number: %d\n", program_settings.wanpipe_number, program_settings.interface_number);

	sang_if = init(program_settings.wanpipe_number, program_settings.interface_number);

	if(sang_if == NULL){
		return 1;
	}

	rc = start(sang_if);
	if(rc){
		cleanup(sang_if);
		return rc;
	}


#if 1
	do{
		EnterCriticalSection(&PrintCriticalSection);
		INFO_MAIN("Press 'q' to quit the program.\n");
		INFO_MAIN("Press 't' to transmit data.\n");
		INFO_MAIN("Press 's' to get Operational Statistics.\n");
		INFO_MAIN("Press 'f' to reset (flush) Operational Statistics.\n");
		INFO_MAIN("Press 'v' to get API driver version.\n");

		if(sang_if->get_adapter_type() == WAN_MEDIA_T1 || sang_if->get_adapter_type() == WAN_MEDIA_E1){
			INFO_MAIN("Press 'a' to get T1/E1 alarms.\n");
			//RBS (CAS) commands
			INFO_MAIN("Press 'g' to get RBS bits.\n");
			INFO_MAIN("Press 'r' to set RBS bits.\n");
			INFO_MAIN("Press '1' to read FE register. Warning: used by Sangoma Techsupport only!\n");
			INFO_MAIN("Press '2' to write FE register.  Warning: used by Sangoma Techsupport only!\n");
		}
		INFO_MAIN("Press 'i' to set Tx idle data buffer (BitStream only).\n");
		switch(sang_if->get_adapter_type())
		{
		case WAN_MEDIA_T1:
			//those commands valid only for T1
			INFO_MAIN("Press 'l' to send 'activate remote loop back' signal.\n");
			INFO_MAIN("Press 'd' to send 'deactivate remote loop back' signal.\n");
			break;
		case WAN_MEDIA_FXOFXS:
			switch(sang_if->get_sub_media())
			{
			case MOD_TYPE_FXS:
				INFO_MAIN("Press 'e' to listen to test tones on a phone connected to the A200-FXS\n");
				INFO_MAIN("Press 'c' to ring/stop ring phone connected to the A200-FXS\n");
				INFO_MAIN("Press 'n' to enable/disable reception of ON/OFF Hook events on A200-FXS\n");
				INFO_MAIN("Press 'm' to enable DTMF events (on SLIC chip) on A200-FXS\n");
				INFO_MAIN("Press 'j' to enable/disable reception of Ring Trip events on A200-FXS\n");
				break;

			case MOD_TYPE_FXO:
				INFO_MAIN("Press 'u' to enable/disable reception of Ring Detect events on A200-FXO\n");
				INFO_MAIN("Press 'h' to transmit ON/OFF hook signals on A200-FXO\n");
				INFO_MAIN("Press 'a' to get Line Status (Connected/Disconnected)\n");
				break;
			}
			break;
		case WAN_MEDIA_BRI:
			INFO_MAIN("Press 'k' to Activate/Deactivate ISDN BRI line\n");
			INFO_MAIN("Press 'l' to enable	bri bchan loopback\n");
			INFO_MAIN("Press 'd' to disable bri bchan loopback\n");
			break;
		}
		INFO_MAIN("Press 'o' to enable DTMF events (on Octasic chip)\n");
		if (program_settings.encode_sw_dtmf) {
			INFO_MAIN("Press 'x' to send software DTMF\n");
		}
		if (program_settings.encode_fsk_cid) {
			INFO_MAIN("Press 'z' to send software FSK Caller ID\n");
		}
		if (program_settings.decode_fsk_cid) {
			INFO_MAIN("Press 'y' to reset software FSK Caller ID\n");
		}

		LeaveCriticalSection(&PrintCriticalSection);

		user_selection = tolower(_getch());
		switch(user_selection)
		{
		case 'q':
			break;
		case 't':
			for(u_int32_t cnt = 0; cnt < program_settings.txcount; cnt++){
				if(program_settings.szTxFileName[0]){
					tx_file(sang_if);
				}else{
					local_tx_data.hdr.data_length = program_settings.txlength;
					local_tx_data.hdr.operation_status = SANG_STATUS_TX_TIMEOUT;
					//set the actual data for transmission
					memset(local_tx_data.data, tx_test_byte, program_settings.txlength);
					sang_if->transmit(&local_tx_data);
					tx_test_byte++;
				}
			}
			break;
		case 's':
			{
				wanpipe_chan_stats_t stats;
				sang_if->get_operational_stats(&stats);
			}
			break;
		case 'f':
			sang_if->flush_operational_stats();
			break;
		case 'v':
			{
				DRIVER_VERSION version;
				//read API driver version
				sang_if->get_api_driver_version(&version);
				INFO_MAIN("\nAPI version\t: %d,%d,%d,%d\n",
					version.major, version.minor, version.minor1, version.minor2);

				u_int8_t customer_id = 0;
				sang_if->get_card_customer_id(&customer_id);
				INFO_MAIN("\ncustomer_id\t: 0x%02X\n", customer_id);
			}
			break;
		case 'a':
			unsigned char cFeStatus;

			switch(sang_if->get_adapter_type())
			{
			case WAN_MEDIA_T1:
			case WAN_MEDIA_E1:
				//read T1/E1/56k alarms
				sang_if->get_te1_56k_stat();
				break;

			case WAN_MEDIA_FXOFXS:
				switch(sang_if->get_sub_media())
				{
				case MOD_TYPE_FXO:
					cFeStatus = 0;
					sang_if->tdm_get_front_end_status(&cFeStatus);
					INFO_MAIN("cFeStatus: %s (%d)\n", FE_STATUS_DECODE(cFeStatus), cFeStatus);
					break;
				}
			}
			break;
		case 'l':
			switch(sang_if->get_adapter_type())
			{
			case WAN_MEDIA_T1:
			case WAN_MEDIA_E1:
				//Activate Line/Remote Loopback mode:
				sang_if->set_lb_modes(WAN_TE1_LINELB_MODE, WAN_TE1_LB_ENABLE);
				//Activate Diagnostic Digital Loopback mode:
				//sang_if->set_lb_modes(WAN_TE1_DDLB_MODE, WAN_TE1_LB_ENABLE);
				//sang_if->set_lb_modes(WAN_TE1_PAYLB_MODE, WAN_TE1_LB_ENABLE);
				break;
			case WAN_MEDIA_BRI:
				sang_if->tdm_enable_bri_bchan_loopback(WAN_BRI_BCHAN1);
				break;
			}
			break;
		case 'd':
			switch(sang_if->get_adapter_type())
			{
			case WAN_MEDIA_T1:
			case WAN_MEDIA_E1:
				//Deactivate Line/Remote Loopback mode:
				sang_if->set_lb_modes(WAN_TE1_LINELB_MODE, WAN_TE1_LB_DISABLE);
				//Deactivate Diagnostic Digital Loopback mode:
				//sang_if->set_lb_modes(WAN_TE1_DDLB_MODE, WAN_TE1_LB_DISABLE);
				//sang_if->set_lb_modes(WAN_TE1_PAYLB_MODE, WAN_TE1_LB_DISABLE);
				break;
			case WAN_MEDIA_BRI:
				sang_if->tdm_disable_bri_bchan_loopback(WAN_BRI_BCHAN1);
				break;
			}
			break;


		case 'g'://read RBS bits
			{
				rbs_management_t rbs_management_struct = {0,0};

				sang_if->enable_rbs_monitoring();

				INFO_MAIN("Type Channel number and press <Enter>:\n");
				rbs_management_struct.channel = get_user_decimal_number();//channels (Time Slots). Valid values: 1 to 24.
				if(rbs_management_struct.channel < 1 || rbs_management_struct.channel > 24){
					INFO_MAIN("Invalid RBS Channel number!\n");
					break;
				}
				sang_if->get_rbs(&rbs_management_struct);
			}
			break;
		case 'r'://set RBS bits
			{
				static rbs_management_t rbs_management_struct = {0,0};

				sang_if->enable_rbs_monitoring();

				INFO_MAIN("Type Channel number and press <Enter>:\n");
				rbs_management_struct.channel = get_user_decimal_number();//channels (Time Slots). Valid values: 1 to 24.
				if(rbs_management_struct.channel < 1 || rbs_management_struct.channel > 24){
					INFO_MAIN("Invalid RBS Channel number!\n");
					break;
				}
				/*	bitmap - set as needed: WAN_RBS_SIG_A |	WAN_RBS_SIG_B | WAN_RBS_SIG_C | WAN_RBS_SIG_D;

					In this example make bits A and B to change each time,
					so it's easy to see the change on the receiving side.
				*/
				if(rbs_management_struct.ABCD_bits == WAN_RBS_SIG_A){
					rbs_management_struct.ABCD_bits = WAN_RBS_SIG_B;
				}else{
					rbs_management_struct.ABCD_bits = WAN_RBS_SIG_A;
				}
				sang_if->set_rbs(&rbs_management_struct);
			}
			break;
		case 'i':
			{
			INFO_MAIN("Type Idle Flag (HEX, for example: FE) and press <Enter>:\n");
			unsigned char new_idle_flag = (unsigned char)get_user_hex_number();
			sang_if->set_tx_idle_flag(new_idle_flag);
			}
			break;
		case 'c':
user_retry_ring_e_d:
			INFO_MAIN("Press 'e' to START ring, 'd' to STOP ring, 't' to Toggle\n");
			INFO_MAIN("\n");
			user_selection = tolower(_getch());
			switch(user_selection)
			{
			case 'e':
				INFO_MAIN("Starting Ring ...%c\n",user_selection);
				sang_if->start_ringing_phone();//start
				break;
			case 'd':
				INFO_MAIN("Stopping Ring ... %c\n",user_selection);
				sang_if->stop_ringing_phone();//stop
				break;
			case 't':
				{
					int x;
					for (x=0;x<500;x++) {
						sang_if->start_ringing_phone();
						sang_if->start_ringing_phone();
						//sangoma_msleep(500);
						sang_if->stop_ringing_phone();//stop
						sang_if->stop_ringing_phone();//stop
						//sangoma_msleep(500);
						sang_if->start_busy_tone();
						sangoma_msleep(50);
						sang_if->stop_all_tones();
						sangoma_msleep(50);
					}
				}
				break;
			default:
				goto user_retry_ring_e_d;
				break;
			}
			break;
		case 'e':
			INFO_MAIN("Press 'e' to START a Tone, 'd' to STOP a Tone.\n");
			INFO_MAIN("\n");

			switch(tolower(_getch()))
			{
			case 'e':
				INFO_MAIN("Press 'r' for Ring Tone, 'd' for Dial Tone, 'b' for Busy Tone, 'c' for Congestion Tone.\n");
				INFO_MAIN("\n");
				switch(tolower(_getch()))
				{
				case 'r':
					sang_if->start_ring_tone();
					break;
				case 'd':
					sang_if->start_dial_tone();
					break;
				case 'b':
					sang_if->start_busy_tone();
					break;
				case 'c':
				default:
					sang_if->start_congestion_tone();
					break;
				}
				break;

			case 'd':
			default:
				sang_if->stop_all_tones();//stop all tones
			}
			break;
		case 'n':
			INFO_MAIN("Press 'e' to ENABLE Rx Hook Events, 'd' to DISABLE Rx Hook Events.\n");
			INFO_MAIN("\n");
			switch(tolower(_getch()))
			{
			case 'e':
				sang_if->tdm_enable_rxhook_events();
				break;
			case 'd':
			default:
				sang_if->tdm_disable_rxhook_events();
			}
			break;
		case 'm':
			//Enable/Disable DTMF events on SLIC chip.
			//On Analog (A200) card only.
			INFO_MAIN("Press 'e' to ENABLE Remora DTMF Events, 'd' to DISABLE Remora DTMF Events.\n");
			INFO_MAIN("\n");
			switch(tolower(_getch()))
			{
			case 'e':
				sang_if->tdm_enable_rm_dtmf_events();
				break;
			case 'd':
			default:
				sang_if->tdm_disable_rm_dtmf_events();
			}
			break;
		case 'o':
			{
				//Enable DTMF events on Octasic chip. 
				//For both Analog (A200) and T1/E1 (A104D) cards, but only if the chip is present.
				INFO_MAIN("Press 'e' to ENABLE Octasic DTMF Events, 'd' to DISABLE Octasic DTMF Events.\n");
				uint8_t channel;

				INFO_MAIN("\n");
				switch(tolower(_getch()))
				{
				case 'e':
					INFO_MAIN("Type Channel number and press <Enter>:\n");
					channel = (uint8_t)get_user_decimal_number();//channels (Time Slots). Valid values: 1 to 31.

					sang_if->tdm_enable_dtmf_events(channel);
					break;
				case 'd':
				default:
					INFO_MAIN("Type Channel number and press <Enter>:\n");
					channel = (uint8_t)get_user_decimal_number();//channels (Time Slots). Valid values: 1 to 31.

					sang_if->tdm_disable_dtmf_events(channel);
				}
			}
			break;
		case 'u':
			//Enable/Disable Ring Detect events on FXO. 
			INFO_MAIN("Press 'e' to ENABLE Rx Ring Detect Events, 'd' to DISABLE Rx Ring Detect Events.\n");
			INFO_MAIN("\n");
			switch(tolower(_getch()))
			{
			case 'e':
				sang_if->tdm_enable_ring_detect_events();
				break;
			case 'd':
			default:
				sang_if->tdm_disable_ring_detect_events();
			}
			break;
		case 'j':
			//Enable/Disable Ring Trip events on FXS. 
			INFO_MAIN("Press 'e' to ENABLE Rx Ring Trip Events, 'd' to DISABLE Rx Ring Trip Events.\n");
			INFO_MAIN("\n");
			switch(tolower(_getch()))
			{
			case 'e':
				sang_if->tdm_enable_ring_trip_detect_events();
				break;
			case 'd':
			default:
				sang_if->tdm_disable_ring_trip_detect_events();
			}
			break;
		case 'h':
			INFO_MAIN("Press 'e' to transmit OFF hook signal, 'd' to transmit ON hook signal.\n");
			INFO_MAIN("\n");
			switch(tolower(_getch()))
			{
			case 'e':
				sang_if->fxo_go_off_hook();
				break;
			case 'd':
			default:
				sang_if->fxo_go_on_hook();
			}
			break;
		case 'k':
			INFO_MAIN("Press 'e' to Activate, 'd' to De-Activate line.\n");
			INFO_MAIN("\n");
			switch(tolower(_getch()))
			{
			case 'e':
				sang_if->tdm_front_end_activate();
				break;
			case 'd':
			default:
				sang_if->tdm_front_end_deactivate();
			}
			break;
		case 'p':
			{
				int user_period;//Milliseconds interval between receive of Voice Data
				INFO_MAIN("Type User Period and press <Enter>. Valid values are: 10, 20, 40.\n");
				user_period = get_user_decimal_number();
				switch(user_period)
				{
				case 10:
				case 20:
				case 40:
					sang_if->tdm_set_user_period(user_period);
					break;
				default:
					INFO_MAIN("Invalid User Period value! Valid values are: 10, 20, 40.\n");
					break;
				}
			}
			break;
#if USE_STELEPHONY_API
#if defined (__LINUX__)
		case 'x':
			{	
				int user_char = _getch();
 				switch(tolower(user_char)) {
					case '1': case '2': case '3':
					case '4': case '5': case '6':
					case '7': case '8': case '9':
					case '0': case 'a': case 'b':
					case 'c':
						INFO_MAIN("Sending DTMF (%c).\n", user_char);
						sang_if->sendSwDTMF((char)user_char);
						break;
					default:
						INFO_MAIN("Invalid DTMF Char! Valid values are: Valid values are 0-9, A-C\n");
					break;
				}
			}
			break;
		case 'z':
			{
				INFO_MAIN("Sending CallerID.\n");
				sang_if->sendCallerID("Sangoma Rocks", "9054741990");
			}
			break;
		case 'y':
			{
				if (program_settings.decode_fsk_cid) {
					INFO_MAIN("Resetting FSK Caller ID\n");
					sang_if->resetFSKCID();
				}
				
			}
			break;
#endif
#endif
		case '1':/* read FE register */
			{
				int	value;
				sdla_fe_debug_t fe_debug;

				fe_debug.type = WAN_FE_DEBUG_REG;

				printf("Type Register number (hex) i.g. F8 and press Enter:");
				value = get_user_hex_number();

				fe_debug.fe_debug_reg.reg  = value;
				fe_debug.fe_debug_reg.read = 1;

				sang_if->set_fe_debug_mode(&fe_debug);
			}
			break;

		case '2':/* write FE register */
			{
				int	value;
				sdla_fe_debug_t fe_debug;
				fe_debug.type = WAN_FE_DEBUG_REG;

				printf("WRITE: Type Register number (hex) i.g. F8 and press Enter:");
				value = get_user_hex_number();

				fe_debug.fe_debug_reg.reg  = value;
				fe_debug.fe_debug_reg.read = 1;

				printf("WRITE: Type value (hex) i.g. 1A and press Enter:");
				value = get_user_hex_number();

				fe_debug.fe_debug_reg.read = 0;
				fe_debug.fe_debug_reg.value = value;

				sang_if->set_fe_debug_mode(&fe_debug);
			}
			break;

		default:
			INFO_MAIN("Invalid command.\n");
		}		
	}while(user_selection != 'q');
#endif
	stop(sang_if);
	cleanup(sang_if);

	return 0;
}//main()

static int set_port_configuration()
{
	int				rc = 0, is_te1_card = 0, user_selection;
	hardware_info_t	hardware_info;
	port_cfg_t		port_cfg;

	sangoma_port_configurator *sng_port_cfg_obj;

	sng_port_cfg_obj = new sangoma_port_configurator();
	if(sng_port_cfg_obj == NULL || sng_port_cfg_obj->init((unsigned short)program_settings.wanpipe_number)){
		return 2;
	}

	rc = sng_port_cfg_obj->get_hardware_info(&hardware_info);

	if(rc == SANG_STATUS_SUCCESS){

		INFO_MAIN("card_model		: %s (0x%08X)\n",
			SDLA_ADPTR_NAME(hardware_info.card_model), hardware_info.card_model);
		INFO_MAIN("firmware_version\t: 0x%02X\n", hardware_info.firmware_version);
		INFO_MAIN("pci_bus_number\t\t: %d\n", hardware_info.pci_bus_number);
		INFO_MAIN("pci_slot_number\t\t: %d\n", hardware_info.pci_slot_number);
		INFO_MAIN("max_hw_ec_chans\t\t: %d\n", hardware_info.max_hw_ec_chans);
		INFO_MAIN("port_number\t\t: %d\n", hardware_info.port_number);

	}else{
		delete sng_port_cfg_obj;
		return 3;
	}

#if 0
defined(__WINDOWS__)
	rc = sng_port_cfg_obj->open_port_registry_key(&hardware_info);
	if(rc != SANG_STATUS_SUCCESS){
		delete sng_port_cfg_obj;
		return 3;
	}
#endif

	memset(&port_cfg, 0x00, sizeof(port_cfg_t));

	switch(hardware_info.card_model)
	{
	case A101_ADPTR_1TE1:
	case A101_ADPTR_2TE1:
	case A104_ADPTR_4TE1:
	case A108_ADPTR_8TE1:
		is_te1_card = 1;
		break;
	}

	if(is_te1_card){

		INFO_MAIN("\n");
		INFO_MAIN("Press 't' to set T1 configration.\n");
		INFO_MAIN("Press 'e' to set E1 configration.\n");

try_again:
		user_selection = tolower(_getch());

		switch(user_selection)
		{
		case 't'://T1

			rc=sng_port_cfg_obj->set_t1_tdm_span_voice_api_configration(&port_cfg,&hardware_info,program_settings.wanpipe_number);
			break;
		case 'e'://E1

			rc=sng_port_cfg_obj->set_e1_tdm_span_voice_api_configration(&port_cfg,&hardware_info,program_settings.wanpipe_number);
			break;

		default:
			INFO_MAIN("Invalid command %c.\n",user_selection);
			goto try_again;
			break;
		}//switch(user_selection)

	} else { //if(is_te1_card)
		INFO_MAIN("Unsupported Card %i\n",hardware_info.card_model);

		rc=1;
#if 0
		//print the current configuration:
		sng_port_cfg_obj->print_port_cfg_structure(&port_cfg);

		//as an EXAMPLE, set the same configration as the current one:
		rc = sng_port_cfg_obj->set_default_configuration(&port_cfg);
#endif

	}

	if (rc==0) {
		INFO_MAIN("Stopping PORT!\n");
		rc=sng_port_cfg_obj->stop_port();
		if (rc == 0) {
			INFO_MAIN("Configuring PORT!\n");
			rc=sng_port_cfg_obj->set_volatile_configration(&port_cfg);
			if (rc == 0) {
				INFO_MAIN("Starting PORT!\n");
				rc=sng_port_cfg_obj->start_port();
				if (rc) {
					INFO_MAIN("Error: Failed to Start Port!\n");
				}
			} else {
				INFO_MAIN("Error: Failed to Configure Port!\n");
			}
		} else {
			INFO_MAIN("Error: Failed to Stop Port!\n");
		}

	} else {
		INFO_MAIN("Error: Failed to Set Configuratoin Port!\n");
	}

	if(sng_port_cfg_obj != NULL){
		delete sng_port_cfg_obj;
	}

	sangoma_msleep(2000);

	return rc;
}


#if USE_STELEPHONY_API
static void FSKCallerIDEvent(void *callback_context,
							 LPCTSTR Name, LPCTSTR CallerNumber,
							 LPCTSTR CalledNumber, LPCTSTR DateTime)
{
	//The "sangoma_interface" object was registered as the callback context in StelSetup() call.
	sangoma_interface	*sang_if = (sangoma_interface*)callback_context;

	INFO_MAIN("\n%s: %s() - Start\n", sang_if->device_name, __FUNCTION__);

#if defined (__LINUX__)
	if(Name){
		INFO_MAIN("Name: %s\n", Name);
	}
	if(CallerNumber){
		INFO_MAIN("CallerNumber: %s\n", CallerNumber);
	}
	if(CalledNumber){
		INFO_MAIN("CalledNumber: %s\n", CalledNumber);
	}
	if(DateTime){
		INFO_MAIN("DateTime: %s\n", DateTime);
	}
#else
	if(Name){
		INFO_MAIN("Name: %S\n", Name);
	}
	if(CallerNumber){
		INFO_MAIN("CallerNumber: %S\n", (wchar_t*)CallerNumber);
	}
	if(CalledNumber){
		INFO_MAIN("CalledNumber: %S\n", CalledNumber);
	}
	if(DateTime){
		INFO_MAIN("DateTime: %S\n", DateTime);
	}
#endif
	INFO_MAIN("%s() - End\n\n", __FUNCTION__);
}

static void DTMFEvent(void *callback_context, long Key)
{
	//The "sangoma_interface" object was registered as the callback context in StelSetup() call.
	sangoma_interface	*sang_if = (sangoma_interface*)callback_context;

	INFO_MAIN("\n%s: %s() - Start\n", sang_if->device_name, __FUNCTION__);

	INFO_MAIN("Key: %c\n", (char) Key);

	INFO_MAIN("%s() - End\n\n", __FUNCTION__);
}

static void Q931Event(void *callback_context, stelephony_q931_event *pQ931Event)
{
	//The "sangoma_interface" object was registered as the callback context in StelSetup() call.
	sangoma_interface	*sang_if = (sangoma_interface*)callback_context;
	
	INFO_MAIN("\n%s: %s() - Start\n", sang_if->device_name, __FUNCTION__);
#if 0
	INFO_MAIN("\nFound %d bytes of data: ", pQ931Event->dataLength);
	for (int i=0; i < pQ931Event->dataLength;i++){
		INFO_MAIN("%02X ",pQ931Event->data[i]);
	}
	INFO_MAIN("\n");
#endif


	INFO_MAIN("Message Received on: %02d/%02d/%02d @ %02d:%02d:%02d\n",pQ931Event->tv.wMonth,pQ931Event->tv.wDay,pQ931Event->tv.wYear,
		pQ931Event->tv.wHour,pQ931Event->tv.wMinute,pQ931Event->tv.wSecond);
	
	INFO_MAIN("Message Type is: %s\n",pQ931Event->msg_type);
	INFO_MAIN("Length of Call Reference Field is: %d\n", pQ931Event->len_callRef);
	INFO_MAIN("Message Call Reference is : 0X%s\n",pQ931Event->callRef);

	if (pQ931Event->cause_code > 0){
		INFO_MAIN("Cause code found = %d \n", pQ931Event->cause_code);
	}

	if (pQ931Event->chan > 0){
		INFO_MAIN("B-channel used = %d \n", pQ931Event->chan);
	}

	if (pQ931Event->calling_num_digits_count > 0 ){
		INFO_MAIN("Found %d digits for calling number \n", pQ931Event->calling_num_digits_count);
		INFO_MAIN("Presentation indicator is = %d \n",pQ931Event->calling_num_presentation);
		INFO_MAIN("Screening indicator is = %d \n",pQ931Event->calling_num_screening_ind);
		INFO_MAIN("Calling number is = %s\n",pQ931Event->calling_num_digits);
	}

	if (pQ931Event->called_num_digits_count > 0 ){
		INFO_MAIN("Found %d digits for called number \n", pQ931Event->called_num_digits_count);
		INFO_MAIN("Called number is = %s\n",pQ931Event->called_num_digits);
	}

	if (pQ931Event->rdnis_digits_count > 0 ){
		INFO_MAIN("Found %d digits for RDNIS\n", pQ931Event->rdnis_digits_count);
		INFO_MAIN("RDNIS is = %s\n",pQ931Event->rdnis_string);
	}
	//INFO_MAIN("%s() - End\n\n", __FUNCTION__);
}

#if 0
#warning "REMOVE LATER"
int slin2ulaw(void* data, size_t max, size_t *datalen)
{
	int16_t sln_buf[512] = {0}, *sln = sln_buf;
	uint8_t *lp = (uint8_t*)data;
	uint32_t i;
	size_t len = *datalen;

	if (max > len) {
		max = len;
	}

	memcpy(sln, data, max);
	
	for(i = 0; i < max; i++) {
		*lp++ = linear_to_ulaw(*sln++);
	}

	*datalen = max / 2;

	return 0;
}

#endif

#if defined (__LINUX__)
static void SwDTMFBuffer (void *callback_context, void* buffer)
{	
	sangoma_interface	*sang_if;
	sang_if = (sangoma_interface*)callback_context;

	sang_if->setDTMFBuffer(buffer);
}

static void FSKCallerIDTransmit (void *callback_context, void* buffer)
{
	int end = 0;
	int cnt = 0;
	int datalen;
	
	sangoma_interface	*sang_if;
	sang_if = (sangoma_interface*)callback_context;

	datalen = program_settings.txlength*2;

	while(!end) {
		if (stelephony_buffer_inuse(buffer) && buffer) {
			wp_api_element_t	local_tx_data;
			int dlen = datalen;
			unsigned char buf[1024];
			size_t br, max = sizeof(buf);

#if 1
			/* Data can be read as slinear, or ulaw, alaw */
			br = stelephony_buffer_read_ulaw(buffer, buf, &dlen, max);
			if (br < (size_t) dlen) {
				memset(buf+br, 0, dlen - br);
			}
#else
			dlen*=2;

			len = dlen;
			
			br = stelephony_buffer_read(buffer, buf, len);
			if (br < dlen) {
				memset(buf+br, 0, dlen - br);
			}

			slin2ulaw(buf, max, (size_t*) &dlen);
#endif
			local_tx_data.hdr.data_length = dlen; 
			local_tx_data.hdr.operation_status = SANG_STATUS_TX_TIMEOUT;

			memcpy (local_tx_data.data, buf, dlen);

			if(sang_if->transmit(&local_tx_data) != SANG_STATUS_SUCCESS){
				printf("Failed to TX dlen:%d\n", dlen);
			} else {
				//printf("TX OK (cnt:%d)\n", cnt);
				cnt++;
				usleep(20000);
			}

		} else {
			printf("Buffer Transmit complete... (transmitted:%d)\n", cnt);
			end = 1;
		}
	}
}
#endif
#endif
