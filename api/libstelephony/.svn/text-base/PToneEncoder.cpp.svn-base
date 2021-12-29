#if defined (__LINUX__)
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#endif
#include "PToneEncoder.h"

#define NOT_IMPL printf("FUNCTION_NOT_IMPLEMENTED (%s:%d)\n",__FUNCTION__,__LINE__);

#define MX_CID_LEN 15

#define SMG_DTMF_ON 	250
#define SMG_DTMF_OFF 	50

PhoneToneEncoder::PhoneToneEncoder(void)
{
	bufSize = 256;
	rate = 8000;
	
	fskData = (fsk_data_state_t*) malloc(sizeof(fsk_data_state_t));
	if (fskData == NULL) {
		printf("Error: Failed to allocate memory (%s:%d)\n", __FUNCTION__,__LINE__);
		return;
	}

	fbuf = (unsigned char*) malloc(bufSize);
	if (fbuf == NULL) {
		printf("Error: Failed to allocate memory (%s:%d)\n", __FUNCTION__,__LINE__);
		return;
	}

	fskTrans = (fsk_modulator_t*) malloc(sizeof(fsk_modulator_t));
	if (fskTrans == NULL) {
		printf("Error: Failed to allocate memory (%s:%d)\n", __FUNCTION__,__LINE__);
		return;
	}

	userData = (helper_t*) malloc(sizeof(helper_t));
	if (userData == NULL) {
		printf("Error: Failed to allocate memory (%s:%d)\n", __FUNCTION__,__LINE__);
		return;
	}

	dtmfSession = (teletone_generation_session_t*) malloc(sizeof(teletone_generation_session_t));
	if (dtmfSession == NULL) {
		printf("Error: Failed to allocate memory (%s:%d)\n", __FUNCTION__,__LINE__);
		return;
	}
	InitializeCriticalSection(&m_CriticalSection);	
}

PhoneToneEncoder::~PhoneToneEncoder(void)
{
	if (fskData) {
		DBG_FSK("Freeing fskData\n");
		free(fskData);
	}
	if (fbuf) {
		DBG_FSK("Freeing fbuf\n");
		free(fbuf);
	}

	if (userData) {
		DBG_FSK("Freeing userData\n");
		free(userData);
	}

	if (fskTrans) {
		DBG_FSK("Freeing fskTrans\n");
		free(fskTrans);
	}

	if (dtmfSession) {
		DBG_DTMF("Freeing dtmfSession\n");
		free(dtmfSession);
	}
}

int PhoneToneEncoder::EncoderInit(void) 
{
	EnterCriticalSection(&m_CriticalSection);
	if (fskData) {
		memset(fskData, 0, sizeof(fsk_data_state_t));
	} else {
		printf("Error: Tone Encoder was not initialized (%s:%d)\n", __FUNCTION__,__LINE__);
		return -1;
	}
	if (fbuf) {
		memset(fbuf, 0, bufSize);
	} else {
		printf("Error: Tone Encoder was not initialized (%s:%d)\n", __FUNCTION__,__LINE__);
		return -1;
	}

	fsk_data_init(fskData, fbuf, bufSize);
	fskInit = true;

	if (dtmfSession) {
		memset(dtmfSession, 0, sizeof(teletone_generation_session_t));
	}
	LeaveCriticalSection(&m_CriticalSection);
	return 0;
}

void PhoneToneEncoder::WaveStreamStart(void)
{
	
	int dtmf_size;
	EnterCriticalSection(&m_CriticalSection);
	dtmfInit = true;
	if (teletone_init_session(dtmfSession, 0, NULL, NULL)) {
		printf("Failed to initialize SW DTMF Tone session\n");
		return;
	}

	dtmfSession->rate = rate;
	dtmfSession->duration = SMG_DTMF_ON * (dtmfSession->rate/1000);
	dtmfSession->wait = SMG_DTMF_OFF * (dtmfSession->rate/1000);

	dtmf_size=(SMG_DTMF_ON+SMG_DTMF_OFF)*10*2;

	buffer_create_dynamic(&(userData->dtmf_buffer), 1024, dtmf_size, 0);

	if (sink_callback_functions.OnSwDTMFTransmit) {
		sink_callback_functions.OnSwDTMFTransmit(this->callback_obj, userData->dtmf_buffer);	
	} else {
		printf("No OnSwDTMFTransmit callback function\n");
	}
	LeaveCriticalSection(&m_CriticalSection);
}

void PhoneToneEncoder::WaveStreamEnd(void)
{
	EnterCriticalSection(&m_CriticalSection);
	dtmfInit = false;
	if (userData->dtmf_buffer) {
		buffer_zero(userData->dtmf_buffer);
	}
	LeaveCriticalSection(&m_CriticalSection);
	return;
}

int PhoneToneEncoder::BufferGetSwDTMF(char dtmf_char, int *retValue)
{
	int wrote;
	int err;

	if (dtmfInit == false && dtmfEnable == false) {
		printf("DTMF Encoder not ready\n");
		return -1;
	}

	DBG_DTMF("Generating DTMF %c\n", dtmf_char);

	if (dtmf_char >= (int) (sizeof(dtmfSession->TONES)/sizeof(dtmfSession->TONES[0]))) {
		printf("Unable to generate DTMF for %c\n", dtmf_char);
		return -1;
	}

	wrote = teletone_mux_tones(dtmfSession, &dtmfSession->TONES[(int) dtmf_char]);
	if (wrote) {
		err = buffer_write(userData->dtmf_buffer, (char*) dtmfSession->buffer, wrote); 
	} else {
		printf("Failed to generate tones\n");
		return -1;
	}
	
	return 0;
}


int PhoneToneEncoder::BufferGetFSKCallerID(stelephony_caller_id_t *cid_info, int *retValue)
{
	int nameStrLen;
	int numberStrLen;
	int dateStrLen;

	if (fskInit == false && fskEnable == false) {
		printf("FSK Encoder not ready\n");
		return -1;
	}

	
	
	nameStrLen = strlen(cid_info->calling_name);
	numberStrLen = strlen(cid_info->calling_number);

	if (cid_info->auto_datetime) {
		struct tm tm;
        time_t now;
	
		time(&now);
        localtime_r(&now, &tm);

		dateStrLen = strftime(cid_info->datetime, sizeof(cid_info->datetime), "%m%d%H%M", &tm);
	} else {
		dateStrLen = strlen(cid_info->datetime);
	}

	DBG_FSK("Generating CallerID Name:[%s] Number:[%s] DateTime:[%s]\n", cid_info->calling_name, cid_info->calling_number, cid_info->datetime);

	if (dateStrLen) {
		fsk_data_add_mdmf(fskData, MDMF_DATETIME, (uint8_t *)cid_info->datetime, dateStrLen);
	}

	if (numberStrLen) {
		fsk_data_add_mdmf(fskData, MDMF_PHONE_NUM, (uint8_t *)cid_info->calling_number, numberStrLen);
	}
	
	if (nameStrLen) {
		fsk_data_add_mdmf(fskData, MDMF_PHONE_NAME, (uint8_t *)cid_info->calling_name, nameStrLen);
	}
	
	buffer_create(&(userData->fsk_buffer), 128, 128, 0);

	fsk_data_add_checksum(fskData);

	fsk_modulator_init(fskTrans, FSK_BELL202, rate, fskData, -14, 180, 5, 300, fsk_write_sample, (void*) userData);

	fsk_modulator_send_all(fskTrans);
 	
	if (sink_callback_functions.OnFSKCallerIDTransmit) {
		sink_callback_functions.OnFSKCallerIDTransmit(this->callback_obj, userData->fsk_buffer);	
	} else {
		printf("No OnFSKCallerIDTransmit callback function\n");
	}
	return 0;
}


void PhoneToneEncoder::SetCallbackFunctions(sink_callback_functions_t *cbf)
{
	memcpy(&sink_callback_functions, cbf, sizeof(sink_callback_functions_t));
	return;
}

void PhoneToneEncoder::GetCallbackFunctions(sink_callback_functions_t *cbf)
{
	memcpy(cbf, &sink_callback_functions, sizeof(sink_callback_functions_t));
	return;
}

void PhoneToneEncoder::SetCallbackFunctionsAndObject(sink_callback_functions_t* cbf, void *cbo)
{
	memcpy(&sink_callback_functions, cbf, sizeof(sink_callback_functions_t));

	callback_obj = cbo;
	return;
}

void PhoneToneEncoder::put_WaveFormatID(variant_t _var)
{
	DBG_FSK("Wave format:%s\n", 
				(_var.intVal==WFI_CCITT_uLaw_8kHzMono) ?"Alaw" :
				(_var.intVal==WFI_CCITT_uLaw_8kHzMono) ?"Ulaw" :
				"slinear");
	memcpy(&variant, &_var, sizeof(variant_t));
	return;
}

void PhoneToneEncoder::put_FeatureFSKCallerID(int val)
{
	DBG_FSK("FSK enabled\n");
	EnterCriticalSection(&m_CriticalSection);
	fskEnable = true;
	LeaveCriticalSection(&m_CriticalSection);
}

void PhoneToneEncoder::put_FeatureSwDTMF(int val)
{
	EnterCriticalSection(&m_CriticalSection);
	if (val == false) {
		DBG_DTMF("DTMF disabled\n");
		dtmfEnable = false;
	} else {
		DBG_DTMF("DTMF enabled\n");
		dtmfEnable = true;
	}
	LeaveCriticalSection(&m_CriticalSection);


}

void PhoneToneEncoder::put_Multithreaded(int val)
{
	/* 
		Leave empty. Function needed for 
		compatibility with ToneDecoder Library 
	*/
	return;
}

