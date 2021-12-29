////////////////////////////////////////////////////////////////////////////////////
//stelephony.cpp - Definitions of the main class of the Sangoma Telephony Library
//					(CStelephony). 
//
// Author	:	David Rokhvarg	<davidr@sangoma.com>
//
// Versions:
//	v 1,0,0,1	May  29	2008	David Rokhvarg	Initial Version
//		Implemented FSK Caller ID detection for Analog FXO.
//
////////////////////////////////////////////////////////////////////////////////////

#if defined(__WINDOWS__)
#include <conio.h>
#include "stdafx.h"
#include "ToneDecoder.h"
#elif defined(__LINUX__)
#include <string.h>
#include <stdio.h>
#endif

#include "stelephony.h"

#define DBG_STEL if(0)printf

CStelephony::CStelephony()
{ 
	STEL_FUNC();

	memset(&callback_functions, 0x00, sizeof(stelephony_callback_functions_t));
	InitializeCriticalSection(&m_CriticalSection);
	return; 
}

CStelephony::~CStelephony()
{ 
	STEL_FUNC();
	return; 
}

void CStelephony::SetCallbackFunctions(IN stelephony_callback_functions_t *cbf)
{
	memcpy(&callback_functions, cbf, sizeof(stelephony_callback_functions_t));
	
	if(cbf->Q931Event){
		DBG_STEL("%s(): cbf->Q931Event: 0x%p\n", __FUNCTION__, cbf->Q931Event);
		q931_event_decoder_callback_functions_t q931_cbf;
		q931_cbf.OnQ931Event = cbf->Q931Event;
		Q931EventsDecoderObj.SetCallbackFunctions(&q931_cbf);
	}
}

void CStelephony::GetCallbackFunctions(OUT stelephony_callback_functions_t *cbf)
{
	memcpy(cbf, &callback_functions, sizeof(stelephony_callback_functions_t));
}

void CStelephony::SetCallbackContext(void *cbc)
{
	callback_context = cbc;
	Q931EventsDecoderObj.SetCallbackObject(cbc);
}

void* CStelephony::GetCallbackContext(void)
{
	return callback_context;
}

stelephony_status_t CStelephony::Init()
{ 
	STEL_FUNC();
	stelephony_status_t status = STEL_STATUS_SUCCESS;

	status = InitDecoderLib();
	if (status != STEL_STATUS_SUCCESS) return status;
#if defined(__LINUX__)
	status = InitEncoderLib();
	if (status != STEL_STATUS_SUCCESS) return status;
#endif
	return STEL_STATUS_SUCCESS;
}

stelephony_status_t CStelephony::Cleanup()
{ 
	STEL_FUNC();
	stelephony_status_t status = STEL_STATUS_SUCCESS;

	status = CleanupDecoderLib();
	if (status != STEL_STATUS_SUCCESS) return status;

#if defined(__LINUX__)
	status = CleanupEncoderLib();
	if (status != STEL_STATUS_SUCCESS) return status;
#endif
	return STEL_STATUS_SUCCESS;
}

#if defined(__WINDOWS__)
stelephony_status_t CStelephony::InitDecoderLib()
{
	CoInitialize(NULL);
	USES_CONVERSION;

	HRESULT hr = pToneDecoder.CoCreateInstance(CLSID_PhoneToneDecoder);
	switch(hr)
	{
	case S_OK:
		break;
	case REGDB_E_CLASSNOTREG:
		STEL_FUNC();
		return STEL_STATUS_DECODER_LIB_CO_CREATE_INSTANCE_REGDB_E_CLASSNOTREG_ERROR;
	case CLASS_E_NOAGGREGATION:
		STEL_FUNC();
		return STEL_STATUS_DECODER_LIB_CO_CREATE_INSTANCE_CLASS_E_NOAGGREGATION_ERROR;
	case E_NOINTERFACE:
		STEL_FUNC();
		return STEL_STATUS_DECODER_LIB_CO_CREATE_INSTANCE_E_NOINTERFACE_ERROR;
	default:
		STEL_FUNC();
		return STEL_STATUS_DECODER_LIB_CO_CREATE_INSTANCE_UNKNOWN_ERROR;
	}

    //check if this interface supports connectable objects
	IConnectionPointContainer	*pCPC;

    hr = pToneDecoder->QueryInterface(IID_IConnectionPointContainer,(void **)&pCPC);
    if ( !SUCCEEDED(hr) ){
		STEL_FUNC();
        return STEL_STATUS_DECODER_LIB_QUERY_INTERFACE_ERROR;
    }

    //ok it does, now get the correct connection point interface 
	//in our case DIID__IPhoneToneDecoderEvents
    hr = pCPC->FindConnectionPoint(DIID__IPhoneToneDecoderEvents,&pCP);
    if ( !SUCCEEDED(hr) ){
		STEL_FUNC();
		pCP = NULL;
        return STEL_STATUS_DECODER_LIB_FIND_CONNECTION_POINT_ERROR;
    }

	//we have done with the connection point container interface
	pCPC->Release();
    
	IUnknown *pSinkUnk;
	
	//create a notification object from our CSink class
    pSink = new CSink;
    if ( NULL == pSink ){
		STEL_FUNC();
		return STEL_STATUS_DECODER_LIB_MEMORY_ALLOCATION_ERROR;
    }

    //Get the pointer to CSink's IUnknown pointer

	hr = pSink->QueryInterface (IID_IUnknown,(void **)&pSinkUnk);

	//Pass it to the com through the com's  IPhoneToneDecoderEvents interface (pCP) retrieved
	//through the earlier FindConnectoinPoint call

	hr = pCP->Advise(pSinkUnk,&dwAdvise); 
	
	//The dwAdvise is the number returned, through which IConnectionPoint:UnAdvise is called 
	//to break the connection.

	//////////////////////////////////////////////////////////////////////////////////////
	//provide License number
	CComBSTR bstrStart(_T("Ds90LK-PPOWXgUBAmhkSBpNCd8qRhNkxpbmRhIFNvb2xlcHAsIFNhbmdvbWEgVGVjaG5vbG9naWVzIEluYy4="));

    BSTR bstrLicense = bstrStart;

	pToneDecoder->put_AboutLicenseKey(bstrLicense);

#if 0
	//////////////////////////////////////////////////////////////////////////////////////
	VARIANT var;
	var.vt=VT_UI4;
	if(m_ToneDecoderCodec == STEL_OPTION_ALAW){
		var.intVal=WFI_CCITT_ALaw_8kHzMono;
	}else{
		var.intVal=WFI_CCITT_uLaw_8kHzMono;
	}
	pToneDecoder->put_WaveFormatID(var);
				
	//////////////////////////////////////////////////////////////////////////////////////
	pToneDecoder->put_MonitorDTMF(TRUE);
	pToneDecoder->put_MonitorCallerID(TRUE);
	pToneDecoder->put_Multithreaded(TRUE);
	pToneDecoder->WaveStreamStart();
#endif

	return STEL_STATUS_SUCCESS;
}
#elif defined(__LINUX__)
stelephony_status_t CStelephony::InitDecoderLib()
{
	int result;
	
	pToneDecoder = new PhoneToneDecoder();
	if (pToneDecoder == NULL) {
		return STEL_STATUS_DECODER_LIB_MEMORY_ALLOCATION_ERROR;
	}

	result = pToneDecoder->DemodInit();
	if (result) {
		return STEL_STATUS_INVALID_INPUT_ERROR;
	}	
	return STEL_STATUS_SUCCESS;
}

stelephony_status_t CStelephony::InitEncoderLib()
{
	int result;

	pToneEncoder = new PhoneToneEncoder();
	if (pToneEncoder == NULL) {
		
		return STEL_STATUS_ENCODER_LIB_MEMORY_ALLOCATION_ERROR;
	}

	result = pToneEncoder->EncoderInit();

	if (result) {
		return STEL_STATUS_INVALID_INPUT_ERROR;
	}	
	return STEL_STATUS_SUCCESS;
}
#endif /* __LINUX__ */

#if defined(__WINDOWS__)
stelephony_status_t CStelephony::CleanupDecoderLib()
{
	STEL_FUNC();

	if(pSink){
		pToneDecoder->WaveStreamEnd();
		if(pCP){
			pCP->Unadvise(dwAdvise);//call this when you need to disconnect from server										
			pCP->Release();
		}
		delete pSink;
		pToneDecoder.Release();
		pSink = NULL;
		pCP = NULL;
	}

	CoUninitialize();
	return STEL_STATUS_SUCCESS;
}
#elif defined(__LINUX__)
stelephony_status_t CStelephony::CleanupDecoderLib()
{
	pToneDecoder->WaveStreamEnd();
	delete pToneDecoder;
	pToneDecoder = NULL;
	return STEL_STATUS_SUCCESS;
}

stelephony_status_t CStelephony::CleanupEncoderLib()
{
	delete pToneEncoder;
	pToneEncoder =  NULL;
	return STEL_STATUS_SUCCESS;
}
#endif /* __LINUX__ */

int CStelephony::DecoderLibStreamInput(void *data, int dataLength)
{
	//If user enabled FSK CallerID or SoftwareDTMF, call ToneDecoder with (voice) StreamInput.
	if(callback_functions.FSKCallerIDEvent || callback_functions.DTMFEvent){
		int retvalue, rc;
#if defined(__LINUX__)
		rc = pToneDecoder->WaveStreamInputEx((char*)data, dataLength, &retvalue);
#else
		rc = pToneDecoder->WaveStreamInputEx((int)data, dataLength, &retvalue);
#endif
		//printf("retvalue: %d, rc: %d\n", retvalue, rc);
	}

	if(callback_functions.Q931Event){
		Q931EventsDecoderObj.Input(data,	dataLength);
	}

	return 0;
}

#if defined(__LINUX__)
stelephony_status_t CStelephony::EncoderLibGenerateFSKCallerID(stelephony_caller_id_t* cid_info)
{
	int retValue, rc;

	rc = pToneEncoder->BufferGetFSKCallerID(cid_info, &retValue);
	if (rc) {
		return STEL_STATUS_ENCODER_LIB_ERROR;
	}
	return STEL_STATUS_SUCCESS;
}
#endif

#if defined (__LINUX__)
stelephony_status_t CStelephony::EncoderLibGenerateSwDTMF(char dtmf_char)
{
	int retValue, rc;

	rc = pToneEncoder->BufferGetSwDTMF(dtmf_char, &retValue);
	if (rc) {
		return STEL_STATUS_ENCODER_LIB_ERROR;
	}
	return STEL_STATUS_SUCCESS;
}
#endif

//caller id is in Unincode because call can be any country
void OnCallerID(void *callback_obj, LPCTSTR Name, LPCTSTR CallerNumber, LPCTSTR CalledNumber, LPCTSTR DateTime)
{
	CStelephony* stelObj = (CStelephony*)callback_obj;
	stelephony_callback_functions_t cbf;

#if (__LINUX__)
	DBG_STEL("%s(): Name: %s, CallerNumber: %s, CalledNumber: %s, DateTime: %s\r\n", 
		__FUNCTION__, Name, CallerNumber, CalledNumber, DateTime);
#else
	DBG_STEL("%s(): Name: %S, CallerNumber: %S, CalledNumber: %S, DateTime: %S\r\n", 
		__FUNCTION__, Name, CallerNumber, CalledNumber, DateTime);
#endif

	stelObj->GetCallbackFunctions(&cbf);

	if(cbf.FSKCallerIDEvent){
		cbf.FSKCallerIDEvent(stelObj->GetCallbackContext(), Name, CallerNumber, CalledNumber, DateTime);
	}
}

void OnDTMF(void *callback_obj, long Key)
{
	CStelephony* stelObj = (CStelephony*)callback_obj;
	stelephony_callback_functions_t cbf;

	DBG_STEL("%s(): Key: %c\r\n", __FUNCTION__, (int) Key);

	stelObj->GetCallbackFunctions(&cbf);

	if(cbf.DTMFEvent){
		cbf.DTMFEvent(stelObj->GetCallbackContext(), Key);
	}
}

#if (__LINUX__)
void OnFSKCallerIDTransmit(void *callback_obj, buffer_t* buffer)
{
	CStelephony* stelObj = (CStelephony*)callback_obj;
	stelephony_callback_functions_t cbf;
	stelObj->GetCallbackFunctions(&cbf);

	if(cbf.FSKCallerIDTransmit){
		cbf.FSKCallerIDTransmit(stelObj->GetCallbackContext(), buffer);
	} else {
		printf("No CallerID Transmit callback function\n");
	}
}
#endif

#if (__LINUX__)
void OnSwDTMFTransmit(void *callback_obj, buffer_t* buffer)
{
	CStelephony* stelObj = (CStelephony*)callback_obj;
	stelephony_callback_functions_t cbf;
	stelObj->GetCallbackFunctions(&cbf);

	if(cbf.SwDTMFBuffer){
		cbf.SwDTMFBuffer(stelObj->GetCallbackContext(), buffer);
	} else {
		printf("No DTMF Transmit callback function\n");
	}
}
#endif

#if (__LINUX__)
stelephony_status_t CStelephony::ToneEncoderEventControl(stelephony_event_t Event, stelephony_control_code_t ControlCode, void *optionalData)
{
	//get current callback functions
	sink_callback_functions_t scf;
	pToneEncoder->GetCallbackFunctions(&scf);

	switch(Event)
	{
		case STEL_FEATURE_FSK_CALLER_ID:
		case STEL_FEATURE_SW_DTMF:
			switch(ControlCode)
			{
				case STEL_CTRL_CODE_ENABLE:
					switch(Event)
					{
						case STEL_FEATURE_FSK_CALLER_ID:
							scf.OnFSKCallerIDTransmit = OnFSKCallerIDTransmit;
							break;
						case STEL_FEATURE_SW_DTMF:
							scf.OnSwDTMFTransmit = OnSwDTMFTransmit;
							break;
						default:
							return STEL_STATUS_INVALID_EVENT_ERROR;
					}
					m_ToneEncoderCodec = *(stelephony_option_t*)optionalData;

					VARIANT var;
					var.vt=VT_UI4;
					if(m_ToneEncoderCodec == STEL_OPTION_ALAW){
						var.intVal=WFI_CCITT_ALaw_8kHzMono;
					}else{
						var.intVal=WFI_CCITT_uLaw_8kHzMono;
					}
					pToneEncoder->WaveStreamEnd(); 
					pToneEncoder->put_WaveFormatID(var);
					pToneEncoder->put_Multithreaded(TRUE);
					pToneEncoder->put_FeatureFSKCallerID(TRUE);
					pToneEncoder->put_FeatureSwDTMF(TRUE);
					pToneEncoder->SetCallbackFunctionsAndObject(&scf, this);
					pToneEncoder->WaveStreamStart(); 
					break;
				case STEL_CTRL_CODE_DISABLE:
					switch(Event)
					{
						case STEL_FEATURE_FSK_CALLER_ID:
							scf.OnFSKCallerIDTransmit = NULL;
							break;
	
						case STEL_FEATURE_SW_DTMF:
							scf.OnSwDTMFTransmit = NULL;
							break;
						default:
							return STEL_STATUS_INVALID_EVENT_ERROR;
					}
					pToneEncoder->WaveStreamEnd(); 
					pToneEncoder->SetCallbackFunctionsAndObject(&scf, this);
					break;
			}
			break;
		default:
			return STEL_STATUS_INVALID_EVENT_ERROR;
	}
	return STEL_STATUS_SUCCESS;
}
#endif

stelephony_status_t CStelephony::ToneDecoderEventControl(stelephony_event_t Event, stelephony_control_code_t ControlCode, void *optionalData)
{
#if defined (__WINDOWS__)
	if(!pSink){
		return STEL_STATUS_REQUEST_INVALID_FOR_CURRENT_OBJECT_STATE_ERROR;
	}
#endif	

	//get current callback functions
	sink_callback_functions_t scf;

#if defined (__WINDOWS__)	
	pSink->GetCallbackFunctions(&scf);
#else
	pToneDecoder->GetCallbackFunctions(&scf);
#endif
	
	//add/remove a function
	switch(Event)
	{
	case STEL_EVENT_FSK_CALLER_ID:
	case STEL_EVENT_DTMF:
		switch(ControlCode)
		{
		case STEL_CTRL_CODE_ENABLE:
			switch(Event)
			{
			case STEL_EVENT_Q931:
				printf("Q931 event not handled\n");
				break;
			case STEL_EVENT_FSK_CALLER_ID:
				scf.OnCallerID = OnCallerID;				
				break;
			case STEL_EVENT_DTMF:
				scf.OnDTMF = OnDTMF;
				break;
			default:
				return STEL_STATUS_INVALID_EVENT_ERROR;
			}
			m_ToneDecoderCodec = *(stelephony_option_t*)optionalData;

			pToneDecoder->WaveStreamEnd();//stop stream, just in case it was running before.
			
			VARIANT var;
			var.vt=VT_UI4;
			if(m_ToneDecoderCodec == STEL_OPTION_ALAW){
				var.intVal=WFI_CCITT_ALaw_8kHzMono;
			}else{
				var.intVal=WFI_CCITT_uLaw_8kHzMono;
			}
			pToneDecoder->put_WaveFormatID(var);
				
			pToneDecoder->put_MonitorDTMF(TRUE);
			pToneDecoder->put_MonitorCallerID(TRUE);
			pToneDecoder->put_Multithreaded(TRUE);
			pToneDecoder->WaveStreamStart();
			break;

		case STEL_CTRL_CODE_DISABLE:
			switch(Event)
			{
			case STEL_EVENT_Q931:
				printf("Q931 event not handled\n");
				break;
			case STEL_EVENT_FSK_CALLER_ID:
				scf.OnCallerID = NULL;
				break;
			case STEL_EVENT_DTMF:
				scf.OnDTMF = NULL;
				break;
			default:
				return STEL_STATUS_INVALID_EVENT_ERROR;
			}
			break;
		}
		break;
	default:
		return STEL_STATUS_INVALID_EVENT_ERROR;
	}

#if defined (__WINDOWS__)
	pSink->SetCallbackFunctionsAndObject(&scf, this);
#else
	pToneDecoder->SetCallbackFunctionsAndObject(&scf, this);
#endif

	return STEL_STATUS_SUCCESS;
}

stelephony_status_t CStelephony::EventControl(stelephony_event_t Event, stelephony_control_code_t ControlCode, void *optionalData)
{
	switch(Event)
	{
	case STEL_EVENT_FSK_CALLER_ID:
	case STEL_EVENT_DTMF:
		return ToneDecoderEventControl(Event, ControlCode, optionalData);
#if defined (__LINUX__)
	case STEL_FEATURE_FSK_CALLER_ID:
	case STEL_FEATURE_SW_DTMF:
		return ToneEncoderEventControl(Event, ControlCode, optionalData);
#endif
	case STEL_EVENT_Q931:
		return Q931EventsDecoderObj.EventControl(Event, ControlCode, optionalData);

	default:
		return STEL_STATUS_INVALID_EVENT_ERROR;
	}
}

void CStelephony::EnterStelCriticalSection()
{
	EnterCriticalSection(&m_CriticalSection);
}

void CStelephony::LeaveStelCriticalSection()
{
	LeaveCriticalSection(&m_CriticalSection);
}
