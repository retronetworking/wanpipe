// Sink.cpp: implementation of the CSink class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Sink.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSink::CSink()
{
	DBG_SINK("%s(): this ptr: 0x%p\n", __FUNCTION__, this);

	m_dwRefCount = 0;
	memset(&sink_callback_functions, 0x00, sizeof(sink_callback_functions_t));
	callback_obj = NULL;
}

CSink::~CSink()
{
	DBG_SINK("%s(): this ptr: 0x%p\n", __FUNCTION__, this);
}

void CSink::SetCallbackFunctionsAndObject(IN sink_callback_functions_t *scf, IN void *cbo)
{
	memcpy(&sink_callback_functions, scf, sizeof(sink_callback_functions_t));
	callback_obj = cbo;
}

void CSink::GetCallbackFunctions(OUT sink_callback_functions_t *scf)
{
	memcpy(scf, &sink_callback_functions, sizeof(sink_callback_functions_t));
}

STDMETHODIMP CSink::OnCallerID(LPCTSTR Name, LPCTSTR CallerNumber, LPCTSTR CalledNumber, LPCTSTR DateTime, long Position) 
{
	DBG_SINK("%s(): Name: %S, CallerNumber: %S, CalledNumber: %S, DateTime: %S, Position: %u\r\n", 
		__FUNCTION__, Name, CallerNumber, CalledNumber, DateTime, Position);
	if(sink_callback_functions.OnCallerID && callback_obj){
		sink_callback_functions.OnCallerID(callback_obj, Name, CallerNumber, CalledNumber, DateTime);
	}
	return S_OK;
}

STDMETHODIMP CSink::OnDebug(LPCTSTR sMsg, long RefCode) 
{
	// TODO: Add your control notification handler code here
	DBGW_SINK(sMsg);
	DBGW_SINK(L"\r\n");
	return S_OK;
}

STDMETHODIMP CSink::OnDTMF(long Key, long Position) 
{
	DBG_SINK("%s(): Key: %c, Position: %u\n", __FUNCTION__, Key, Position);
	if(sink_callback_functions.OnDTMF && callback_obj){
		sink_callback_functions.OnDTMF(callback_obj, Key);
	}
	return S_OK;
}

STDMETHODIMP CSink::OnFrequencyDetected(double Frequency, double secondFreq, long duration, long StartPos) 
{
	// TODO: Add your control notification handler code here
	return S_OK;
	
}

STDMETHODIMP CSink::OnMF(long Key, long Position) 
{
	// TODO: Add your control notification handler code here
	return S_OK;
}

STDMETHODIMP CSink::OnPaging(long Key, long Position) 
{
	// TODO: Add your control notification handler code here
	return S_OK;
}

STDMETHODIMP CSink::OnSAME(LPCTSTR RecvData, long Position) 
{
	// TODO: Add your control notification handler code here
	return S_OK;
}

STDMETHODIMP CSink::OnSilenceEnd(long EndPos, long duration) 
{
	// TODO: Add your control notification handler code here
	return S_OK;
}

STDMETHODIMP CSink::OnSilenceStart(long StartPos) 
{
	// TODO: Add your control notification handler code here
	return S_OK;
}

STDMETHODIMP CSink::OnSpectrumData() 
{
	// TODO: Add your control notification handler code here
	return S_OK;
}

STDMETHODIMP CSink::OnToneDetected(long AppSpecific, long Position) 
{
	// TODO: Add your control notification handler code here
	return S_OK;
}

STDMETHODIMP CSink::OnTTY(long RecvChar, long Position) 
{
	// TODO: Add your control notification handler code here
	return S_OK;
}

STDMETHODIMP CSink::OnVMWI(long Status, long ParamsData, long Position) 
{
	// TODO: Add your control notification handler code here
	return S_OK;
}

STDMETHODIMP CSink::Invoke(DISPID dispidMember, REFIID riid,
		   LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		   EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	VARIANTARG __RPC_FAR *rgvarg=pdispparams->rgvarg;
	switch(dispidMember) {
	case 0x1:
		return OnDTMF(V_I4(&rgvarg[1]),V_I4(&rgvarg[0]));
	case 0x2:
		return OnSilenceStart(V_I4(&rgvarg[0]));
	case 0x3:
		return OnSilenceEnd(V_I4(&rgvarg[1]),V_I4(&rgvarg[0]));
	case 0x4:
		return OnFrequencyDetected(V_R8(&rgvarg[3]),V_R8(&rgvarg[2]),V_I4(&rgvarg[1]),V_I4(&rgvarg[0]));
	case 0x5:
		return OnSpectrumData();
	case 0x6:
		return OnTTY(V_I4(&rgvarg[1]),V_I4(&rgvarg[0]));
	case 0x7:
		return OnCallerID(V_BSTR(&rgvarg[4]),V_BSTR(&rgvarg[3]),V_BSTR(&rgvarg[2]),V_BSTR(&rgvarg[1]),
					V_I4(&rgvarg[0]));
	case 0x8:
		return OnDebug(V_BSTR(&rgvarg[1]),V_I4(&rgvarg[0]));
	case 0x9:
		return OnToneDetected(V_I4(&rgvarg[1]),V_I4(&rgvarg[0]));
	case 0xa:
		return OnMF(V_I4(&rgvarg[1]),V_I4(&rgvarg[0]));
	case 0xb:
		return OnPaging(V_I4(&rgvarg[1]),V_I4(&rgvarg[0]));
	case 0xc:
		return OnSAME(V_BSTR(&rgvarg[1]),V_I4(&rgvarg[0]));
	case 0xd:
		return OnVMWI(V_I4(&rgvarg[2]),V_I4(&rgvarg[1]),V_I4(&rgvarg[0]));

	}
	return E_FAIL;
}
