#ifndef __FSK_TONE_DECODER_H__
#define __FSK_TONE_DECODER_H__


#pragma once
#include <stdio.h>
#include <StelephonyApi.h>

#include "Sink.h"
#include "g711.h"
#include "libteletone.h"

#define DBG_FSK		if(0)printf
#define DBG_DTMF	if(0)printf  


class PhoneToneDecoder
{
	CRITICAL_SECTION	m_CriticalSection;

	/* common variables */
	variant_t			variant;	
	sink_callback_functions_t sink_callback_functions;
	void				*callback_obj;

	/* fsk variables */
	BOOL 				fskInit;
	BOOL				fskEnable;
	fsk_data_state_t 	*fskData;
	unsigned char 		*fbuf;
	size_t 				bufSize;
	int 				rate;
	int WaveStreamInputExFSK(int16_t* slin_data, int dataLength, int *retvalue);
	
	/* dtmf variables */
	BOOL				dtmfInit;
	BOOL 				dtmfEnable;
	teletone_dtmf_detect_state_t *dtmfData;
	
	int WaveStreamInputExDtmf(int16_t* slin_data, int dataLength, int *retvalue);

public:
	PhoneToneDecoder(void);
	~PhoneToneDecoder(void);
		
	int  DemodInit(void);
	int  WaveStreamInputEx(char* data,	int dataLength, int *retvalue);
	void SetCallbackFunctions(sink_callback_functions_t *cbf);
	void GetCallbackFunctions(sink_callback_functions_t *cbf);
	void SetCallbackFunctionsAndObject(sink_callback_functions_t* cbf, void *cbo);
	void WaveStreamStart(void);
	void WaveStreamEnd(void);
	void put_WaveFormatID(variant_t var);
	void put_MonitorDTMF(int val);
	void put_MonitorCallerID(int val);
	void put_Multithreaded(int val);
};

#endif/* __FSK_TONE_DECODER_H__*/
