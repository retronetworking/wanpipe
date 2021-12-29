/*
	sample.h - common definitions used in this application.
*/
#ifndef _SAMPLE_H
#define _SAMPLE_H

#include <libsangoma.h>
#if USE_STELEPHONY_API
#include <libstelephony.h>
#endif

#define SAMPLE_CPP_MAX_PATH 1024

typedef struct{
	unsigned int	wanpipe_number;	
	unsigned int	interface_number;	
	unsigned char	silent;
	uint16_t		txlength;
	unsigned char	Rx_to_Tx_loopback;
	unsigned char	decode_fsk_cid;
	unsigned char 	encode_fsk_cid;
	unsigned char	sw_dtmf;
	unsigned char 	encode_sw_dtmf;
	unsigned char	voice_codec_alaw;
	unsigned char	decode_q931;
	char		szTxFileName[SAMPLE_CPP_MAX_PATH];
	unsigned int	txcount;
	unsigned char   driver_config;
	unsigned char	use_ctrl_dev;
}wp_program_settings_t;

#define DEV_NAME_LEN			100

extern void cli_out(unsigned int dbg_flag, void *pszFormat, ...);
extern unsigned int verbosity_level;

typedef struct{
	//Recieved data
	int		(*got_rx_data)(void *sang_if_ptr, void *rx_data);
	//TDM events
	void	(*got_TdmApiEvent)(void *sang_if_ptr, void *event_data);
#if USE_STELEPHONY_API
	//FSK Caller ID
	void	(*FSKCallerIDEvent)(void *sang_if_ptr, LPCTSTR Name, LPCTSTR CallerNumber, LPCTSTR CalledNumber, LPCTSTR DateTime);
	//DTMF detected in SOFTWARE
	void	(*DTMFEvent)(void *sang_if_ptr, long Key);
	//Q931 events
	void	(*Q931Event)(void *callback_context, stelephony_q931_event *pQ931Event);
	//FSK Caller ID buffer ready events
	void	(*FSKCallerIDTransmit)(void *callback_context, void* buffer);
	//FSK DTMF buffer ready events
	void	(*SwDTMFBuffer)(void *callback_context, void* buffer);
#endif
}callback_functions_t;

static void DecodeLastError(LPSTR lpszFunction) 
{
#if defined (__WINDOWS__)
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
	printf("Last Error: %s (GetLastError() returned: %d)\n", lpMsgBuf, dwLastErr);
	// Free the buffer.
	LocalFree( lpMsgBuf );
#endif
} 

#endif//_SAMPLE_H
