//////////////////////////////////////////////////////////////////////
// Sink.h: interface for the CSink class.
//
// Author	:	David Rokhvarg	<davidr@sangoma.com>
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SINK_H__3618F855_A175_41A3_AF69_3C0DEA0152A7__INCLUDED_)
#define AFX_SINK_H__3618F855_A175_41A3_AF69_3C0DEA0152A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#if defined(__WINDOWS__)
#include "ToneDecoder.h"
#elif defined(__LINUX__)
#include "libstelephony_linux_compat.h"
#include "libstelephony_tone.h"
#endif
//#import "../../ToneDecoder.dll" named_guids raw_interfaces_only 


#define DBG_SINK	if(0)printf
#define DBGW_SINK	if(0)wprintf

typedef struct{
	void	(*OnCallerID)(void *callback_obj, LPCTSTR Name, LPCTSTR CallerNumber, LPCTSTR CalledNumber, LPCTSTR DateTime);
	void	(*OnDTMF)(void *callback_obj, long Key);
#if (__LINUX__)
	void 	(*OnFSKCallerIDTransmit)(void *callback_obj, buffer_t* buffer);
	void 	(*OnSwDTMFTransmit)(void *callback_obj, buffer_t* buffer);
#endif
}sink_callback_functions_t;

#if defined(__WINDOWS__)
class CSink  : public _IPhoneToneDecoderEvents
{
private:
    DWORD						m_dwRefCount;
	sink_callback_functions_t	sink_callback_functions;
	void						*callback_obj;

public:
	CSink();
	virtual ~CSink();

	STDMETHODIMP OnCallerID(LPCTSTR Name, LPCTSTR CallerNumber, LPCTSTR CalledNumber, LPCTSTR DateTime, long Position);
	STDMETHODIMP OnDebug(LPCTSTR sMsg, long RefCode);
	STDMETHODIMP OnDTMF(long Key, long Position);
	STDMETHODIMP OnFrequencyDetected(double Frequency, double secondFreq, long duration, long StartPos);
	STDMETHODIMP OnMF(long Key, long Position);
	STDMETHODIMP OnPaging(long Key, long Position);
	STDMETHODIMP OnSAME(LPCTSTR RecvData, long Position);
	STDMETHODIMP OnSilenceEnd(long EndPos, long duration);
	STDMETHODIMP OnSilenceStart(long StartPos);
	STDMETHODIMP OnSpectrumData();
	STDMETHODIMP OnToneDetected(long AppSpecific, long Position);
	STDMETHODIMP OnTTY(long RecvChar, long Position);
	STDMETHODIMP OnVMWI(long Status, long ParamsData, long Position);

	void SetCallbackFunctionsAndObject(IN sink_callback_functions_t *scf, IN void *cbo);
	void GetCallbackFunctions(OUT sink_callback_functions_t *scf);

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject)
    {
      if (iid == DIID__IPhoneToneDecoderEvents)
        {
            m_dwRefCount++;
            *ppvObject = (void *)this;
            return S_OK;
        }

        if (iid == IID_IUnknown)
        {
            m_dwRefCount++;            
            *ppvObject = (void *)this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }
	ULONG STDMETHODCALLTYPE AddRef()
    {
        m_dwRefCount++;
        return m_dwRefCount;
    }
    
	ULONG STDMETHODCALLTYPE Release()
    {
        ULONG l;
        
        l  = m_dwRefCount--;

        if ( 0 == m_dwRefCount)
        {
            delete this;
        }
        
        return l;
    }

  STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
  {
	return E_NOTIMPL;
  }
	   
  STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
  {
	return E_NOTIMPL;
  }
	   
  STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
		   LCID lcid, DISPID* rgdispid)
  {
	return E_NOTIMPL;
  }
	   
  STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		   LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		   EXCEPINFO* pexcepinfo, UINT* puArgErr);  
 
};
#endif /* __WINDOWS__ */
#endif // !defined(AFX_SINK_H__3618F855_A175_41A3_AF69_3C0DEA0152A7__INCLUDED_)
