/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Thu Mar 01 00:28:22 2007
 */
/* Compiler settings for E:\source2\ToneDecoder\ToneDecoderOcx\ToneDecoder.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ToneDecoder_h__
#define __ToneDecoder_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ITonePagingParams_FWD_DEFINED__
#define __ITonePagingParams_FWD_DEFINED__
typedef interface ITonePagingParams ITonePagingParams;
#endif 	/* __ITonePagingParams_FWD_DEFINED__ */


#ifndef __IPhoneToneDecoder_FWD_DEFINED__
#define __IPhoneToneDecoder_FWD_DEFINED__
typedef interface IPhoneToneDecoder IPhoneToneDecoder;
#endif 	/* __IPhoneToneDecoder_FWD_DEFINED__ */


#ifndef __IMonitorTonesParams_FWD_DEFINED__
#define __IMonitorTonesParams_FWD_DEFINED__
typedef interface IMonitorTonesParams IMonitorTonesParams;
#endif 	/* __IMonitorTonesParams_FWD_DEFINED__ */


#ifndef __ICustomToneParams_FWD_DEFINED__
#define __ICustomToneParams_FWD_DEFINED__
typedef interface ICustomToneParams ICustomToneParams;
#endif 	/* __ICustomToneParams_FWD_DEFINED__ */


#ifndef __IDTMFParams_FWD_DEFINED__
#define __IDTMFParams_FWD_DEFINED__
typedef interface IDTMFParams IDTMFParams;
#endif 	/* __IDTMFParams_FWD_DEFINED__ */


#ifndef __ITTYParams_FWD_DEFINED__
#define __ITTYParams_FWD_DEFINED__
typedef interface ITTYParams ITTYParams;
#endif 	/* __ITTYParams_FWD_DEFINED__ */


#ifndef ___IPhoneToneDecoderEvents_FWD_DEFINED__
#define ___IPhoneToneDecoderEvents_FWD_DEFINED__
typedef interface _IPhoneToneDecoderEvents _IPhoneToneDecoderEvents;
#endif 	/* ___IPhoneToneDecoderEvents_FWD_DEFINED__ */


#ifndef __PhoneToneDecoder_FWD_DEFINED__
#define __PhoneToneDecoder_FWD_DEFINED__

#ifdef __cplusplus
typedef class PhoneToneDecoder PhoneToneDecoder;
#else
typedef struct PhoneToneDecoder PhoneToneDecoder;
#endif /* __cplusplus */

#endif 	/* __PhoneToneDecoder_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_ToneDecoder_0000 */
/* [local] */ 

typedef /* [public][public] */ 
enum __MIDL___MIDL_itf_ToneDecoder_0000_0001
    {	WFI_PCM_8kHz8BitMono	= 4,
	WFI_PCM_8kHz8BitStereo	= WFI_PCM_8kHz8BitMono + 1,
	WFI_PCM_8kHz16BitMono	= WFI_PCM_8kHz8BitStereo + 1,
	WFI_PCM_8kHz16BitStereo	= WFI_PCM_8kHz16BitMono + 1,
	WFI_PCM_11kHz8BitMono	= WFI_PCM_8kHz16BitStereo + 1,
	WFI_PCM_11kHz8BitStereo	= WFI_PCM_11kHz8BitMono + 1,
	WFI_PCM_11kHz16BitMono	= WFI_PCM_11kHz8BitStereo + 1,
	WFI_PCM_11kHz16BitStereo	= WFI_PCM_11kHz16BitMono + 1,
	WFI_PCM_12kHz8BitMono	= WFI_PCM_11kHz16BitStereo + 1,
	WFI_PCM_12kHz8BitStereo	= WFI_PCM_12kHz8BitMono + 1,
	WFI_PCM_12kHz16BitMono	= WFI_PCM_12kHz8BitStereo + 1,
	WFI_PCM_12kHz16BitStereo	= WFI_PCM_12kHz16BitMono + 1,
	WFI_PCM_16kHz8BitMono	= WFI_PCM_12kHz16BitStereo + 1,
	WFI_PCM_16kHz8BitStereo	= WFI_PCM_16kHz8BitMono + 1,
	WFI_PCM_16kHz16BitMono	= WFI_PCM_16kHz8BitStereo + 1,
	WFI_PCM_16kHz16BitStereo	= WFI_PCM_16kHz16BitMono + 1,
	WFI_PCM_22kHz8BitMono	= WFI_PCM_16kHz16BitStereo + 1,
	WFI_PCM_22kHz8BitStereo	= WFI_PCM_22kHz8BitMono + 1,
	WFI_PCM_22kHz16BitMono	= WFI_PCM_22kHz8BitStereo + 1,
	WFI_PCM_22kHz16BitStereo	= WFI_PCM_22kHz16BitMono + 1,
	WFI_PCM_24kHz8BitMono	= WFI_PCM_22kHz16BitStereo + 1,
	WFI_PCM_24kHz8BitStereo	= WFI_PCM_24kHz8BitMono + 1,
	WFI_PCM_24kHz16BitMono	= WFI_PCM_24kHz8BitStereo + 1,
	WFI_PCM_24kHz16BitStereo	= WFI_PCM_24kHz16BitMono + 1,
	WFI_PCM_32kHz8BitMono	= WFI_PCM_24kHz16BitStereo + 1,
	WFI_PCM_32kHz8BitStereo	= WFI_PCM_32kHz8BitMono + 1,
	WFI_PCM_32kHz16BitMono	= WFI_PCM_32kHz8BitStereo + 1,
	WFI_PCM_32kHz16BitStereo	= WFI_PCM_32kHz16BitMono + 1,
	WFI_PCM_44kHz8BitMono	= WFI_PCM_32kHz16BitStereo + 1,
	WFI_PCM_44kHz8BitStereo	= WFI_PCM_44kHz8BitMono + 1,
	WFI_PCM_44kHz16BitMono	= WFI_PCM_44kHz8BitStereo + 1,
	WFI_PCM_44kHz16BitStereo	= WFI_PCM_44kHz16BitMono + 1,
	WFI_PCM_48kHz8BitMono	= WFI_PCM_44kHz16BitStereo + 1,
	WFI_PCM_48kHz8BitStereo	= WFI_PCM_48kHz8BitMono + 1,
	WFI_PCM_48kHz16BitMono	= WFI_PCM_48kHz8BitStereo + 1,
	WFI_PCM_48kHz16BitStereo	= WFI_PCM_48kHz16BitMono + 1,
	WFI_TrueSpeech_8kHz1BitMono	= WFI_PCM_48kHz16BitStereo + 1,
	WFI_CCITT_ALaw_8kHzMono	= WFI_TrueSpeech_8kHz1BitMono + 1,
	WFI_CCITT_ALaw_8kHzStereo	= WFI_CCITT_ALaw_8kHzMono + 1,
	WFI_CCITT_ALaw_11kHzMono	= WFI_CCITT_ALaw_8kHzStereo + 1,
	WFI_CCITT_ALaw_11kHzStereo	= WFI_CCITT_ALaw_11kHzMono + 1,
	WFI_CCITT_ALaw_22kHzMono	= WFI_CCITT_ALaw_11kHzStereo + 1,
	WFI_CCITT_ALaw_22kHzStereo	= WFI_CCITT_ALaw_22kHzMono + 1,
	WFI_CCITT_ALaw_44kHzMono	= WFI_CCITT_ALaw_22kHzStereo + 1,
	WFI_CCITT_ALaw_44kHzStereo	= WFI_CCITT_ALaw_44kHzMono + 1,
	WFI_CCITT_uLaw_8kHzMono	= WFI_CCITT_ALaw_44kHzStereo + 1,
	WFI_CCITT_uLaw_8kHzStereo	= WFI_CCITT_uLaw_8kHzMono + 1,
	WFI_CCITT_uLaw_11kHzMono	= WFI_CCITT_uLaw_8kHzStereo + 1,
	WFI_CCITT_uLaw_11kHzStereo	= WFI_CCITT_uLaw_11kHzMono + 1,
	WFI_CCITT_uLaw_22kHzMono	= WFI_CCITT_uLaw_11kHzStereo + 1,
	WFI_CCITT_uLaw_22kHzStereo	= WFI_CCITT_uLaw_22kHzMono + 1,
	WFI_CCITT_uLaw_44kHzMono	= WFI_CCITT_uLaw_22kHzStereo + 1,
	WFI_CCITT_uLaw_44kHzStereo	= WFI_CCITT_uLaw_44kHzMono + 1,
	WFI_ADPCM_8kHzMono	= WFI_CCITT_uLaw_44kHzStereo + 1,
	WFI_ADPCM_8kHzStereo	= WFI_ADPCM_8kHzMono + 1,
	WFI_ADPCM_11kHzMono	= WFI_ADPCM_8kHzStereo + 1,
	WFI_ADPCM_11kHzStereo	= WFI_ADPCM_11kHzMono + 1,
	WFI_ADPCM_22kHzMono	= WFI_ADPCM_11kHzStereo + 1,
	WFI_ADPCM_22kHzStereo	= WFI_ADPCM_22kHzMono + 1,
	WFI_ADPCM_44kHzMono	= WFI_ADPCM_22kHzStereo + 1,
	WFI_ADPCM_44kHzStereo	= WFI_ADPCM_44kHzMono + 1,
	WFI_GSM610_8kHzMono	= WFI_ADPCM_44kHzStereo + 1,
	WFI_GSM610_11kHzMono	= WFI_GSM610_8kHzMono + 1,
	WFI_GSM610_22kHzMono	= WFI_GSM610_11kHzMono + 1,
	WFI_GSM610_44kHzMono	= WFI_GSM610_22kHzMono + 1
    }	WAVE_FORMAT_INDEX;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_ToneDecoder_0000_0002
    {	LEFT_CHANNEL	= 0,
	RIGHT_CHANNEL	= LEFT_CHANNEL + 1
    }	CHANNEL_OPT;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_ToneDecoder_0000_0003
    {	PT_EIA	= 0,
	PT_CCIR	= PT_EIA + 1,
	PT_ZVEI1	= PT_CCIR + 1,
	PT_ZVEI2	= PT_ZVEI1 + 1,
	PT_ZVEI3	= PT_ZVEI2 + 1,
	PT_PZVEI	= PT_ZVEI3 + 1,
	PT_EEA	= PT_PZVEI + 1
    }	PAGING_TYPE;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_ToneDecoder_0000_0004
    {	WINDOW_RECTANGLE	= 0,
	WINDOW_HAMMING	= WINDOW_RECTANGLE + 1,
	WINDOW_BLACKMAN	= WINDOW_HAMMING + 1,
	WINDOW_TRIANGLE	= WINDOW_BLACKMAN + 1,
	WINDOW_HANNING	= WINDOW_TRIANGLE + 1,
	NO_WINDOW	= WINDOW_HANNING + 1
    }	FFT_WINDOW_TYPE;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_ToneDecoder_0000_0005
    {	BR_45_45_Baud	= 0,
	BR_50_Baud	= BR_45_45_Baud + 1,
	BR_56_92_Baud	= BR_50_Baud + 1,
	BR_74_20_Baud	= BR_56_92_Baud + 1,
	BR_100_Baud	= BR_74_20_Baud + 1,
	BR_110_Baud	= BR_100_Baud + 1,
	BR_1200_BPS	= BR_110_Baud + 1
    }	TTY_RATE;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_ToneDecoder_0000_0006
    {	Baudot_Mode	= 0,
	ASCII7_Mode	= Baudot_Mode + 1,
	ASCII8_Mode	= ASCII7_Mode + 1
    }	TTY_MODE;

typedef /* [public][public] */ 
enum __MIDL___MIDL_itf_ToneDecoder_0000_0007
    {	VMWI_OFF	= 0,
	VMWI_ON	= VMWI_OFF + 1,
	VMWI_UNKNOWN	= VMWI_ON + 1
    }	VMWI_STATUS;







extern RPC_IF_HANDLE __MIDL_itf_ToneDecoder_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ToneDecoder_0000_v0_0_s_ifspec;

#ifndef __ITonePagingParams_INTERFACE_DEFINED__
#define __ITonePagingParams_INTERFACE_DEFINED__

/* interface ITonePagingParams */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ITonePagingParams;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("54BD50C7-E15F-49CD-845A-3087AC56EAFF")
    ITonePagingParams : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PagingType( 
            /* [retval][out] */ PAGING_TYPE __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PagingType( 
            /* [in] */ PAGING_TYPE newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITonePagingParamsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITonePagingParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITonePagingParams __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITonePagingParams __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITonePagingParams __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITonePagingParams __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITonePagingParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITonePagingParams __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PagingType )( 
            ITonePagingParams __RPC_FAR * This,
            /* [retval][out] */ PAGING_TYPE __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PagingType )( 
            ITonePagingParams __RPC_FAR * This,
            /* [in] */ PAGING_TYPE newVal);
        
        END_INTERFACE
    } ITonePagingParamsVtbl;

    interface ITonePagingParams
    {
        CONST_VTBL struct ITonePagingParamsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITonePagingParams_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITonePagingParams_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITonePagingParams_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITonePagingParams_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITonePagingParams_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITonePagingParams_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITonePagingParams_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITonePagingParams_get_PagingType(This,pVal)	\
    (This)->lpVtbl -> get_PagingType(This,pVal)

#define ITonePagingParams_put_PagingType(This,newVal)	\
    (This)->lpVtbl -> put_PagingType(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITonePagingParams_get_PagingType_Proxy( 
    ITonePagingParams __RPC_FAR * This,
    /* [retval][out] */ PAGING_TYPE __RPC_FAR *pVal);


void __RPC_STUB ITonePagingParams_get_PagingType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITonePagingParams_put_PagingType_Proxy( 
    ITonePagingParams __RPC_FAR * This,
    /* [in] */ PAGING_TYPE newVal);


void __RPC_STUB ITonePagingParams_put_PagingType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITonePagingParams_INTERFACE_DEFINED__ */


#ifndef __IPhoneToneDecoder_INTERFACE_DEFINED__
#define __IPhoneToneDecoder_INTERFACE_DEFINED__

/* interface IPhoneToneDecoder */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IPhoneToneDecoder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("818DD928-DC2E-4BE6-8D15-52AC2A1A630E")
    IPhoneToneDecoder : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AboutBox( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetfrequencyDB( 
            double Frequency,
            /* [retval][out] */ double __RPC_FAR *retvalue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFreqsArrayDB( 
            /* [out] */ VARIANT __RPC_FAR *FreqArray,
            /* [retval][out] */ int __RPC_FAR *retvalue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorDTMF( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MonitorDTMF( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorCallerID( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MonitorCallerID( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableSpectrumConversion( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableSpectrumConversion( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorCustomTone( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MonitorCustomTone( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorTTY( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MonitorTTY( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WaveStreamInput( 
            /* [in] */ VARIANT Buffer,
            /* [retval][out] */ int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AboutUserName( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AboutUserName( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AboutLicenseKey( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AboutLicenseKey( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AboutLicenseType( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AboutLicenseType( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CustomToneParams( 
            /* [retval][out] */ ICustomToneParams __RPC_FAR *__RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MonitorTones_Add( 
            long AppSpecific,
            long MinPeriodOn,
            long MinPeriodOff,
            /* [defaultvalue][in] */ int Frequency1 = 0,
            /* [defaultvalue][in] */ int Frequency2 = 0,
            /* [defaultvalue][in] */ int Frequency3 = 0,
            /* [defaultvalue][in] */ int Frequency4 = 0,
            /* [defaultvalue][in] */ float MinAmpdB = 0,
            /* [defaultvalue][in] */ float MaxAmpdB = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MonitorTones_Clear( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DTMFParams( 
            /* [retval][out] */ IDTMFParams __RPC_FAR *__RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReceivedTTY( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReceivedDTMFs( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WaveStreamInputEx( 
            int BuffAddr,
            int cbSize,
            /* [retval][out] */ int __RPC_FAR *retvalue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TTYParams( 
            /* [retval][out] */ ITTYParams __RPC_FAR *__RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorTonesParams( 
            /* [retval][out] */ IMonitorTonesParams __RPC_FAR *__RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DecodeChannel( 
            /* [retval][out] */ CHANNEL_OPT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DecodeChannel( 
            /* [in] */ CHANNEL_OPT newVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Source( 
            /* [in] */ VARIANT newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReserveProp1( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReserveProp1( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorTonePaging( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MonitorTonePaging( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorMF( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MonitorMF( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WaveStreamStart( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TonePagingParams( 
            /* [retval][out] */ ITonePagingParams __RPC_FAR *__RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorSAME( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MonitorSAME( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReserveProp2( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReserveProp2( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Multithreaded( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Multithreaded( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorVMWI( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MonitorVMWI( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MonitorCIDCW( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MonitorCIDCW( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WaveFormatID( 
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WaveFormatID( 
            /* [in] */ VARIANT newVal) = 0;
        
        virtual /* [hidden][helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WaveFormatIndex( 
            /* [in] */ WAVE_FORMAT_INDEX newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCenterFreqs( 
            /* [out] */ double __RPC_FAR *Frequency,
            /* [out] */ double __RPC_FAR *secondFreq) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WaveStreamEnd( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPhoneToneDecoderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPhoneToneDecoder __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPhoneToneDecoder __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AboutBox )( 
            IPhoneToneDecoder __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetfrequencyDB )( 
            IPhoneToneDecoder __RPC_FAR * This,
            double Frequency,
            /* [retval][out] */ double __RPC_FAR *retvalue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFreqsArrayDB )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *FreqArray,
            /* [retval][out] */ int __RPC_FAR *retvalue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorDTMF )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonitorDTMF )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorCallerID )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonitorCallerID )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EnableSpectrumConversion )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EnableSpectrumConversion )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorCustomTone )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonitorCustomTone )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorTTY )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonitorTTY )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WaveStreamInput )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT Buffer,
            /* [retval][out] */ int __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AboutUserName )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AboutUserName )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AboutLicenseKey )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AboutLicenseKey )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AboutLicenseType )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AboutLicenseType )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CustomToneParams )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ ICustomToneParams __RPC_FAR *__RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MonitorTones_Add )( 
            IPhoneToneDecoder __RPC_FAR * This,
            long AppSpecific,
            long MinPeriodOn,
            long MinPeriodOff,
            /* [defaultvalue][in] */ int Frequency1,
            /* [defaultvalue][in] */ int Frequency2,
            /* [defaultvalue][in] */ int Frequency3,
            /* [defaultvalue][in] */ int Frequency4,
            /* [defaultvalue][in] */ float MinAmpdB,
            /* [defaultvalue][in] */ float MaxAmpdB);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MonitorTones_Clear )( 
            IPhoneToneDecoder __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DTMFParams )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ IDTMFParams __RPC_FAR *__RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReceivedTTY )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReceivedDTMFs )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WaveStreamInputEx )( 
            IPhoneToneDecoder __RPC_FAR * This,
            int BuffAddr,
            int cbSize,
            /* [retval][out] */ int __RPC_FAR *retvalue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TTYParams )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ ITTYParams __RPC_FAR *__RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorTonesParams )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ IMonitorTonesParams __RPC_FAR *__RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DecodeChannel )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ CHANNEL_OPT __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DecodeChannel )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ CHANNEL_OPT newVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Source )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReserveProp1 )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ReserveProp1 )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorTonePaging )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonitorTonePaging )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorMF )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonitorMF )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WaveStreamStart )( 
            IPhoneToneDecoder __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TonePagingParams )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ ITonePagingParams __RPC_FAR *__RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorSAME )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonitorSAME )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReserveProp2 )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ReserveProp2 )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Multithreaded )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Multithreaded )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorVMWI )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonitorVMWI )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MonitorCIDCW )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MonitorCIDCW )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_WaveFormatID )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_WaveFormatID )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ VARIANT newVal);
        
        /* [hidden][helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_WaveFormatIndex )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [in] */ WAVE_FORMAT_INDEX newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCenterFreqs )( 
            IPhoneToneDecoder __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *Frequency,
            /* [out] */ double __RPC_FAR *secondFreq);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WaveStreamEnd )( 
            IPhoneToneDecoder __RPC_FAR * This);
        
        END_INTERFACE
    } IPhoneToneDecoderVtbl;

    interface IPhoneToneDecoder
    {
        CONST_VTBL struct IPhoneToneDecoderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPhoneToneDecoder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPhoneToneDecoder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPhoneToneDecoder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPhoneToneDecoder_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPhoneToneDecoder_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPhoneToneDecoder_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPhoneToneDecoder_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPhoneToneDecoder_AboutBox(This)	\
    (This)->lpVtbl -> AboutBox(This)

#define IPhoneToneDecoder_GetfrequencyDB(This,Frequency,retvalue)	\
    (This)->lpVtbl -> GetfrequencyDB(This,Frequency,retvalue)

#define IPhoneToneDecoder_GetFreqsArrayDB(This,FreqArray,retvalue)	\
    (This)->lpVtbl -> GetFreqsArrayDB(This,FreqArray,retvalue)

#define IPhoneToneDecoder_get_MonitorDTMF(This,pVal)	\
    (This)->lpVtbl -> get_MonitorDTMF(This,pVal)

#define IPhoneToneDecoder_put_MonitorDTMF(This,newVal)	\
    (This)->lpVtbl -> put_MonitorDTMF(This,newVal)

#define IPhoneToneDecoder_get_MonitorCallerID(This,pVal)	\
    (This)->lpVtbl -> get_MonitorCallerID(This,pVal)

#define IPhoneToneDecoder_put_MonitorCallerID(This,newVal)	\
    (This)->lpVtbl -> put_MonitorCallerID(This,newVal)

#define IPhoneToneDecoder_get_EnableSpectrumConversion(This,pVal)	\
    (This)->lpVtbl -> get_EnableSpectrumConversion(This,pVal)

#define IPhoneToneDecoder_put_EnableSpectrumConversion(This,newVal)	\
    (This)->lpVtbl -> put_EnableSpectrumConversion(This,newVal)

#define IPhoneToneDecoder_get_MonitorCustomTone(This,pVal)	\
    (This)->lpVtbl -> get_MonitorCustomTone(This,pVal)

#define IPhoneToneDecoder_put_MonitorCustomTone(This,newVal)	\
    (This)->lpVtbl -> put_MonitorCustomTone(This,newVal)

#define IPhoneToneDecoder_get_MonitorTTY(This,pVal)	\
    (This)->lpVtbl -> get_MonitorTTY(This,pVal)

#define IPhoneToneDecoder_put_MonitorTTY(This,newVal)	\
    (This)->lpVtbl -> put_MonitorTTY(This,newVal)

#define IPhoneToneDecoder_WaveStreamInput(This,Buffer,pVal)	\
    (This)->lpVtbl -> WaveStreamInput(This,Buffer,pVal)

#define IPhoneToneDecoder_get_AboutUserName(This,pVal)	\
    (This)->lpVtbl -> get_AboutUserName(This,pVal)

#define IPhoneToneDecoder_put_AboutUserName(This,newVal)	\
    (This)->lpVtbl -> put_AboutUserName(This,newVal)

#define IPhoneToneDecoder_get_AboutLicenseKey(This,pVal)	\
    (This)->lpVtbl -> get_AboutLicenseKey(This,pVal)

#define IPhoneToneDecoder_put_AboutLicenseKey(This,newVal)	\
    (This)->lpVtbl -> put_AboutLicenseKey(This,newVal)

#define IPhoneToneDecoder_get_AboutLicenseType(This,pVal)	\
    (This)->lpVtbl -> get_AboutLicenseType(This,pVal)

#define IPhoneToneDecoder_put_AboutLicenseType(This,newVal)	\
    (This)->lpVtbl -> put_AboutLicenseType(This,newVal)

#define IPhoneToneDecoder_get_CustomToneParams(This,pVal)	\
    (This)->lpVtbl -> get_CustomToneParams(This,pVal)

#define IPhoneToneDecoder_MonitorTones_Add(This,AppSpecific,MinPeriodOn,MinPeriodOff,Frequency1,Frequency2,Frequency3,Frequency4,MinAmpdB,MaxAmpdB)	\
    (This)->lpVtbl -> MonitorTones_Add(This,AppSpecific,MinPeriodOn,MinPeriodOff,Frequency1,Frequency2,Frequency3,Frequency4,MinAmpdB,MaxAmpdB)

#define IPhoneToneDecoder_MonitorTones_Clear(This)	\
    (This)->lpVtbl -> MonitorTones_Clear(This)

#define IPhoneToneDecoder_get_DTMFParams(This,pVal)	\
    (This)->lpVtbl -> get_DTMFParams(This,pVal)

#define IPhoneToneDecoder_get_ReceivedTTY(This,pVal)	\
    (This)->lpVtbl -> get_ReceivedTTY(This,pVal)

#define IPhoneToneDecoder_get_ReceivedDTMFs(This,pVal)	\
    (This)->lpVtbl -> get_ReceivedDTMFs(This,pVal)

#define IPhoneToneDecoder_WaveStreamInputEx(This,BuffAddr,cbSize,retvalue)	\
    (This)->lpVtbl -> WaveStreamInputEx(This,BuffAddr,cbSize,retvalue)

#define IPhoneToneDecoder_get_TTYParams(This,pVal)	\
    (This)->lpVtbl -> get_TTYParams(This,pVal)

#define IPhoneToneDecoder_get_MonitorTonesParams(This,pVal)	\
    (This)->lpVtbl -> get_MonitorTonesParams(This,pVal)

#define IPhoneToneDecoder_get_DecodeChannel(This,pVal)	\
    (This)->lpVtbl -> get_DecodeChannel(This,pVal)

#define IPhoneToneDecoder_put_DecodeChannel(This,newVal)	\
    (This)->lpVtbl -> put_DecodeChannel(This,newVal)

#define IPhoneToneDecoder_put_Source(This,newVal)	\
    (This)->lpVtbl -> put_Source(This,newVal)

#define IPhoneToneDecoder_get_ReserveProp1(This,pVal)	\
    (This)->lpVtbl -> get_ReserveProp1(This,pVal)

#define IPhoneToneDecoder_put_ReserveProp1(This,newVal)	\
    (This)->lpVtbl -> put_ReserveProp1(This,newVal)

#define IPhoneToneDecoder_get_MonitorTonePaging(This,pVal)	\
    (This)->lpVtbl -> get_MonitorTonePaging(This,pVal)

#define IPhoneToneDecoder_put_MonitorTonePaging(This,newVal)	\
    (This)->lpVtbl -> put_MonitorTonePaging(This,newVal)

#define IPhoneToneDecoder_get_MonitorMF(This,pVal)	\
    (This)->lpVtbl -> get_MonitorMF(This,pVal)

#define IPhoneToneDecoder_put_MonitorMF(This,newVal)	\
    (This)->lpVtbl -> put_MonitorMF(This,newVal)

#define IPhoneToneDecoder_WaveStreamStart(This)	\
    (This)->lpVtbl -> WaveStreamStart(This)

#define IPhoneToneDecoder_get_TonePagingParams(This,pVal)	\
    (This)->lpVtbl -> get_TonePagingParams(This,pVal)

#define IPhoneToneDecoder_get_MonitorSAME(This,pVal)	\
    (This)->lpVtbl -> get_MonitorSAME(This,pVal)

#define IPhoneToneDecoder_put_MonitorSAME(This,newVal)	\
    (This)->lpVtbl -> put_MonitorSAME(This,newVal)

#define IPhoneToneDecoder_get_ReserveProp2(This,pVal)	\
    (This)->lpVtbl -> get_ReserveProp2(This,pVal)

#define IPhoneToneDecoder_put_ReserveProp2(This,newVal)	\
    (This)->lpVtbl -> put_ReserveProp2(This,newVal)

#define IPhoneToneDecoder_get_Multithreaded(This,pVal)	\
    (This)->lpVtbl -> get_Multithreaded(This,pVal)

#define IPhoneToneDecoder_put_Multithreaded(This,newVal)	\
    (This)->lpVtbl -> put_Multithreaded(This,newVal)

#define IPhoneToneDecoder_get_MonitorVMWI(This,pVal)	\
    (This)->lpVtbl -> get_MonitorVMWI(This,pVal)

#define IPhoneToneDecoder_put_MonitorVMWI(This,newVal)	\
    (This)->lpVtbl -> put_MonitorVMWI(This,newVal)

#define IPhoneToneDecoder_get_MonitorCIDCW(This,pVal)	\
    (This)->lpVtbl -> get_MonitorCIDCW(This,pVal)

#define IPhoneToneDecoder_put_MonitorCIDCW(This,newVal)	\
    (This)->lpVtbl -> put_MonitorCIDCW(This,newVal)

#define IPhoneToneDecoder_get_WaveFormatID(This,pVal)	\
    (This)->lpVtbl -> get_WaveFormatID(This,pVal)

#define IPhoneToneDecoder_put_WaveFormatID(This,newVal)	\
    (This)->lpVtbl -> put_WaveFormatID(This,newVal)

#define IPhoneToneDecoder_put_WaveFormatIndex(This,newVal)	\
    (This)->lpVtbl -> put_WaveFormatIndex(This,newVal)

#define IPhoneToneDecoder_GetCenterFreqs(This,Frequency,secondFreq)	\
    (This)->lpVtbl -> GetCenterFreqs(This,Frequency,secondFreq)

#define IPhoneToneDecoder_WaveStreamEnd(This)	\
    (This)->lpVtbl -> WaveStreamEnd(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_AboutBox_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This);


void __RPC_STUB IPhoneToneDecoder_AboutBox_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_GetfrequencyDB_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    double Frequency,
    /* [retval][out] */ double __RPC_FAR *retvalue);


void __RPC_STUB IPhoneToneDecoder_GetfrequencyDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_GetFreqsArrayDB_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *FreqArray,
    /* [retval][out] */ int __RPC_FAR *retvalue);


void __RPC_STUB IPhoneToneDecoder_GetFreqsArrayDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorDTMF_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorDTMF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_MonitorDTMF_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_MonitorDTMF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorCallerID_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorCallerID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_MonitorCallerID_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_MonitorCallerID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_EnableSpectrumConversion_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_EnableSpectrumConversion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_EnableSpectrumConversion_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_EnableSpectrumConversion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorCustomTone_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorCustomTone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_MonitorCustomTone_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_MonitorCustomTone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorTTY_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorTTY_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_MonitorTTY_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_MonitorTTY_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_WaveStreamInput_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT Buffer,
    /* [retval][out] */ int __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_WaveStreamInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_AboutUserName_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_AboutUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_AboutUserName_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IPhoneToneDecoder_put_AboutUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_AboutLicenseKey_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_AboutLicenseKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_AboutLicenseKey_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IPhoneToneDecoder_put_AboutLicenseKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_AboutLicenseType_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_AboutLicenseType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_AboutLicenseType_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IPhoneToneDecoder_put_AboutLicenseType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_CustomToneParams_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ ICustomToneParams __RPC_FAR *__RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_CustomToneParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_MonitorTones_Add_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    long AppSpecific,
    long MinPeriodOn,
    long MinPeriodOff,
    /* [defaultvalue][in] */ int Frequency1,
    /* [defaultvalue][in] */ int Frequency2,
    /* [defaultvalue][in] */ int Frequency3,
    /* [defaultvalue][in] */ int Frequency4,
    /* [defaultvalue][in] */ float MinAmpdB,
    /* [defaultvalue][in] */ float MaxAmpdB);


void __RPC_STUB IPhoneToneDecoder_MonitorTones_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_MonitorTones_Clear_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This);


void __RPC_STUB IPhoneToneDecoder_MonitorTones_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_DTMFParams_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ IDTMFParams __RPC_FAR *__RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_DTMFParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_ReceivedTTY_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_ReceivedTTY_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_ReceivedDTMFs_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_ReceivedDTMFs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_WaveStreamInputEx_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    int BuffAddr,
    int cbSize,
    /* [retval][out] */ int __RPC_FAR *retvalue);


void __RPC_STUB IPhoneToneDecoder_WaveStreamInputEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_TTYParams_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ ITTYParams __RPC_FAR *__RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_TTYParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorTonesParams_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ IMonitorTonesParams __RPC_FAR *__RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorTonesParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_DecodeChannel_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ CHANNEL_OPT __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_DecodeChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_DecodeChannel_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ CHANNEL_OPT newVal);


void __RPC_STUB IPhoneToneDecoder_put_DecodeChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_Source_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT newVal);


void __RPC_STUB IPhoneToneDecoder_put_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_ReserveProp1_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_ReserveProp1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_ReserveProp1_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IPhoneToneDecoder_put_ReserveProp1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorTonePaging_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorTonePaging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_MonitorTonePaging_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_MonitorTonePaging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorMF_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorMF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_MonitorMF_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_MonitorMF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_WaveStreamStart_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This);


void __RPC_STUB IPhoneToneDecoder_WaveStreamStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_TonePagingParams_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ ITonePagingParams __RPC_FAR *__RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_TonePagingParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorSAME_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorSAME_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_MonitorSAME_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_MonitorSAME_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_ReserveProp2_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_ReserveProp2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_ReserveProp2_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IPhoneToneDecoder_put_ReserveProp2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_Multithreaded_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_Multithreaded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_Multithreaded_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_Multithreaded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorVMWI_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorVMWI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_MonitorVMWI_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_MonitorVMWI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_MonitorCIDCW_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_MonitorCIDCW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_MonitorCIDCW_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IPhoneToneDecoder_put_MonitorCIDCW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_get_WaveFormatID_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IPhoneToneDecoder_get_WaveFormatID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_WaveFormatID_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ VARIANT newVal);


void __RPC_STUB IPhoneToneDecoder_put_WaveFormatID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_put_WaveFormatIndex_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [in] */ WAVE_FORMAT_INDEX newVal);


void __RPC_STUB IPhoneToneDecoder_put_WaveFormatIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_GetCenterFreqs_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *Frequency,
    /* [out] */ double __RPC_FAR *secondFreq);


void __RPC_STUB IPhoneToneDecoder_GetCenterFreqs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPhoneToneDecoder_WaveStreamEnd_Proxy( 
    IPhoneToneDecoder __RPC_FAR * This);


void __RPC_STUB IPhoneToneDecoder_WaveStreamEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPhoneToneDecoder_INTERFACE_DEFINED__ */



#ifndef __TONEDECODERLib_LIBRARY_DEFINED__
#define __TONEDECODERLib_LIBRARY_DEFINED__

/* library TONEDECODERLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_TONEDECODERLib;

#ifndef __IMonitorTonesParams_INTERFACE_DEFINED__
#define __IMonitorTonesParams_INTERFACE_DEFINED__

/* interface IMonitorTonesParams */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IMonitorTonesParams;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("54BD50C7-E15F-49CD-845A-3087AC55EAFF")
    IMonitorTonesParams : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Tolerance( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Tolerance( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NoiseThreshold( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NoiseThreshold( 
            /* [in] */ float newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMonitorTonesParamsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMonitorTonesParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMonitorTonesParams __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMonitorTonesParams __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMonitorTonesParams __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMonitorTonesParams __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMonitorTonesParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMonitorTonesParams __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Tolerance )( 
            IMonitorTonesParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Tolerance )( 
            IMonitorTonesParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NoiseThreshold )( 
            IMonitorTonesParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_NoiseThreshold )( 
            IMonitorTonesParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        END_INTERFACE
    } IMonitorTonesParamsVtbl;

    interface IMonitorTonesParams
    {
        CONST_VTBL struct IMonitorTonesParamsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMonitorTonesParams_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMonitorTonesParams_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMonitorTonesParams_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMonitorTonesParams_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMonitorTonesParams_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMonitorTonesParams_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMonitorTonesParams_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMonitorTonesParams_get_Tolerance(This,pVal)	\
    (This)->lpVtbl -> get_Tolerance(This,pVal)

#define IMonitorTonesParams_put_Tolerance(This,newVal)	\
    (This)->lpVtbl -> put_Tolerance(This,newVal)

#define IMonitorTonesParams_get_NoiseThreshold(This,pVal)	\
    (This)->lpVtbl -> get_NoiseThreshold(This,pVal)

#define IMonitorTonesParams_put_NoiseThreshold(This,newVal)	\
    (This)->lpVtbl -> put_NoiseThreshold(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMonitorTonesParams_get_Tolerance_Proxy( 
    IMonitorTonesParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB IMonitorTonesParams_get_Tolerance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMonitorTonesParams_put_Tolerance_Proxy( 
    IMonitorTonesParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB IMonitorTonesParams_put_Tolerance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMonitorTonesParams_get_NoiseThreshold_Proxy( 
    IMonitorTonesParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB IMonitorTonesParams_get_NoiseThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IMonitorTonesParams_put_NoiseThreshold_Proxy( 
    IMonitorTonesParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB IMonitorTonesParams_put_NoiseThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMonitorTonesParams_INTERFACE_DEFINED__ */


#ifndef __ICustomToneParams_INTERFACE_DEFINED__
#define __ICustomToneParams_INTERFACE_DEFINED__

/* interface ICustomToneParams */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ICustomToneParams;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("54BD50C7-E15F-49CD-845A-3087AC52EAFF")
    ICustomToneParams : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SilenceThreshold( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SilenceThreshold( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FFT_Window( 
            /* [retval][out] */ FFT_WINDOW_TYPE __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FFT_Window( 
            /* [in] */ FFT_WINDOW_TYPE newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_minSilenceDuration( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_minSilenceDuration( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AmplitudeThreshold( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AmplitudeThreshold( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MaxTonesDuration( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MaxTonesDuration( 
            /* [in] */ long newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICustomToneParamsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICustomToneParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICustomToneParams __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICustomToneParams __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICustomToneParams __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICustomToneParams __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICustomToneParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICustomToneParams __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SilenceThreshold )( 
            ICustomToneParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SilenceThreshold )( 
            ICustomToneParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FFT_Window )( 
            ICustomToneParams __RPC_FAR * This,
            /* [retval][out] */ FFT_WINDOW_TYPE __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FFT_Window )( 
            ICustomToneParams __RPC_FAR * This,
            /* [in] */ FFT_WINDOW_TYPE newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_minSilenceDuration )( 
            ICustomToneParams __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_minSilenceDuration )( 
            ICustomToneParams __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AmplitudeThreshold )( 
            ICustomToneParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AmplitudeThreshold )( 
            ICustomToneParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MaxTonesDuration )( 
            ICustomToneParams __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MaxTonesDuration )( 
            ICustomToneParams __RPC_FAR * This,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } ICustomToneParamsVtbl;

    interface ICustomToneParams
    {
        CONST_VTBL struct ICustomToneParamsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICustomToneParams_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICustomToneParams_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICustomToneParams_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICustomToneParams_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICustomToneParams_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICustomToneParams_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICustomToneParams_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICustomToneParams_get_SilenceThreshold(This,pVal)	\
    (This)->lpVtbl -> get_SilenceThreshold(This,pVal)

#define ICustomToneParams_put_SilenceThreshold(This,newVal)	\
    (This)->lpVtbl -> put_SilenceThreshold(This,newVal)

#define ICustomToneParams_get_FFT_Window(This,pVal)	\
    (This)->lpVtbl -> get_FFT_Window(This,pVal)

#define ICustomToneParams_put_FFT_Window(This,newVal)	\
    (This)->lpVtbl -> put_FFT_Window(This,newVal)

#define ICustomToneParams_get_minSilenceDuration(This,pVal)	\
    (This)->lpVtbl -> get_minSilenceDuration(This,pVal)

#define ICustomToneParams_put_minSilenceDuration(This,newVal)	\
    (This)->lpVtbl -> put_minSilenceDuration(This,newVal)

#define ICustomToneParams_get_AmplitudeThreshold(This,pVal)	\
    (This)->lpVtbl -> get_AmplitudeThreshold(This,pVal)

#define ICustomToneParams_put_AmplitudeThreshold(This,newVal)	\
    (This)->lpVtbl -> put_AmplitudeThreshold(This,newVal)

#define ICustomToneParams_get_MaxTonesDuration(This,pVal)	\
    (This)->lpVtbl -> get_MaxTonesDuration(This,pVal)

#define ICustomToneParams_put_MaxTonesDuration(This,newVal)	\
    (This)->lpVtbl -> put_MaxTonesDuration(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_get_SilenceThreshold_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB ICustomToneParams_get_SilenceThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_put_SilenceThreshold_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB ICustomToneParams_put_SilenceThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_get_FFT_Window_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [retval][out] */ FFT_WINDOW_TYPE __RPC_FAR *pVal);


void __RPC_STUB ICustomToneParams_get_FFT_Window_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_put_FFT_Window_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [in] */ FFT_WINDOW_TYPE newVal);


void __RPC_STUB ICustomToneParams_put_FFT_Window_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_get_minSilenceDuration_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB ICustomToneParams_get_minSilenceDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_put_minSilenceDuration_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB ICustomToneParams_put_minSilenceDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_get_AmplitudeThreshold_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB ICustomToneParams_get_AmplitudeThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_put_AmplitudeThreshold_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB ICustomToneParams_put_AmplitudeThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_get_MaxTonesDuration_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB ICustomToneParams_get_MaxTonesDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ICustomToneParams_put_MaxTonesDuration_Proxy( 
    ICustomToneParams __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB ICustomToneParams_put_MaxTonesDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICustomToneParams_INTERFACE_DEFINED__ */


#ifndef __IDTMFParams_INTERFACE_DEFINED__
#define __IDTMFParams_INTERFACE_DEFINED__

/* interface IDTMFParams */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDTMFParams;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("54BD50C7-E15F-49CD-845A-3087AC53EAFF")
    IDTMFParams : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_minDuration( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_minDuration( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_minIntervalDuration( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_minIntervalDuration( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Harmonic2nd_col( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Harmonic2nd_col( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Harmonic2nd_row( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Harmonic2nd_row( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnergyThreshold( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnergyThreshold( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Normal_twist( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Normal_twist( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Reverse_twist( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Reverse_twist( 
            /* [in] */ float newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDTMFParamsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDTMFParams __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDTMFParams __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IDTMFParams __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_minDuration )( 
            IDTMFParams __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_minDuration )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_minIntervalDuration )( 
            IDTMFParams __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_minIntervalDuration )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Harmonic2nd_col )( 
            IDTMFParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Harmonic2nd_col )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Harmonic2nd_row )( 
            IDTMFParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Harmonic2nd_row )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EnergyThreshold )( 
            IDTMFParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EnergyThreshold )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Normal_twist )( 
            IDTMFParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Normal_twist )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Reverse_twist )( 
            IDTMFParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Reverse_twist )( 
            IDTMFParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        END_INTERFACE
    } IDTMFParamsVtbl;

    interface IDTMFParams
    {
        CONST_VTBL struct IDTMFParamsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDTMFParams_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDTMFParams_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDTMFParams_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDTMFParams_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDTMFParams_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDTMFParams_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDTMFParams_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDTMFParams_get_minDuration(This,pVal)	\
    (This)->lpVtbl -> get_minDuration(This,pVal)

#define IDTMFParams_put_minDuration(This,newVal)	\
    (This)->lpVtbl -> put_minDuration(This,newVal)

#define IDTMFParams_get_minIntervalDuration(This,pVal)	\
    (This)->lpVtbl -> get_minIntervalDuration(This,pVal)

#define IDTMFParams_put_minIntervalDuration(This,newVal)	\
    (This)->lpVtbl -> put_minIntervalDuration(This,newVal)

#define IDTMFParams_get_Harmonic2nd_col(This,pVal)	\
    (This)->lpVtbl -> get_Harmonic2nd_col(This,pVal)

#define IDTMFParams_put_Harmonic2nd_col(This,newVal)	\
    (This)->lpVtbl -> put_Harmonic2nd_col(This,newVal)

#define IDTMFParams_get_Harmonic2nd_row(This,pVal)	\
    (This)->lpVtbl -> get_Harmonic2nd_row(This,pVal)

#define IDTMFParams_put_Harmonic2nd_row(This,newVal)	\
    (This)->lpVtbl -> put_Harmonic2nd_row(This,newVal)

#define IDTMFParams_get_EnergyThreshold(This,pVal)	\
    (This)->lpVtbl -> get_EnergyThreshold(This,pVal)

#define IDTMFParams_put_EnergyThreshold(This,newVal)	\
    (This)->lpVtbl -> put_EnergyThreshold(This,newVal)

#define IDTMFParams_get_Normal_twist(This,pVal)	\
    (This)->lpVtbl -> get_Normal_twist(This,pVal)

#define IDTMFParams_put_Normal_twist(This,newVal)	\
    (This)->lpVtbl -> put_Normal_twist(This,newVal)

#define IDTMFParams_get_Reverse_twist(This,pVal)	\
    (This)->lpVtbl -> get_Reverse_twist(This,pVal)

#define IDTMFParams_put_Reverse_twist(This,newVal)	\
    (This)->lpVtbl -> put_Reverse_twist(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IDTMFParams_get_minDuration_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IDTMFParams_get_minDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IDTMFParams_put_minDuration_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IDTMFParams_put_minDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IDTMFParams_get_minIntervalDuration_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IDTMFParams_get_minIntervalDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IDTMFParams_put_minIntervalDuration_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IDTMFParams_put_minIntervalDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IDTMFParams_get_Harmonic2nd_col_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB IDTMFParams_get_Harmonic2nd_col_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IDTMFParams_put_Harmonic2nd_col_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB IDTMFParams_put_Harmonic2nd_col_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IDTMFParams_get_Harmonic2nd_row_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB IDTMFParams_get_Harmonic2nd_row_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IDTMFParams_put_Harmonic2nd_row_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB IDTMFParams_put_Harmonic2nd_row_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IDTMFParams_get_EnergyThreshold_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB IDTMFParams_get_EnergyThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IDTMFParams_put_EnergyThreshold_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB IDTMFParams_put_EnergyThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IDTMFParams_get_Normal_twist_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB IDTMFParams_get_Normal_twist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IDTMFParams_put_Normal_twist_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB IDTMFParams_put_Normal_twist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IDTMFParams_get_Reverse_twist_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB IDTMFParams_get_Reverse_twist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IDTMFParams_put_Reverse_twist_Proxy( 
    IDTMFParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB IDTMFParams_put_Reverse_twist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDTMFParams_INTERFACE_DEFINED__ */


#ifndef __ITTYParams_INTERFACE_DEFINED__
#define __ITTYParams_INTERFACE_DEFINED__

/* interface ITTYParams */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ITTYParams;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("54BD50C7-E15F-49CD-845A-3087AC54EAFF")
    ITTYParams : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rate( 
            /* [retval][out] */ TTY_RATE __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Rate( 
            /* [in] */ TTY_RATE newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Mode( 
            /* [retval][out] */ TTY_MODE __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Mode( 
            /* [in] */ TTY_MODE newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MarkFreq( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MarkFreq( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SpaceFreq( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SpaceFreq( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SignalThreshold( 
            /* [retval][out] */ float __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SignalThreshold( 
            /* [in] */ float newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITTYParamsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITTYParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITTYParams __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITTYParams __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITTYParams __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITTYParams __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITTYParams __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITTYParams __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rate )( 
            ITTYParams __RPC_FAR * This,
            /* [retval][out] */ TTY_RATE __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Rate )( 
            ITTYParams __RPC_FAR * This,
            /* [in] */ TTY_RATE newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Mode )( 
            ITTYParams __RPC_FAR * This,
            /* [retval][out] */ TTY_MODE __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Mode )( 
            ITTYParams __RPC_FAR * This,
            /* [in] */ TTY_MODE newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MarkFreq )( 
            ITTYParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MarkFreq )( 
            ITTYParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SpaceFreq )( 
            ITTYParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SpaceFreq )( 
            ITTYParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SignalThreshold )( 
            ITTYParams __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SignalThreshold )( 
            ITTYParams __RPC_FAR * This,
            /* [in] */ float newVal);
        
        END_INTERFACE
    } ITTYParamsVtbl;

    interface ITTYParams
    {
        CONST_VTBL struct ITTYParamsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTYParams_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTYParams_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTYParams_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTYParams_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTYParams_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTYParams_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTYParams_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTYParams_get_Rate(This,pVal)	\
    (This)->lpVtbl -> get_Rate(This,pVal)

#define ITTYParams_put_Rate(This,newVal)	\
    (This)->lpVtbl -> put_Rate(This,newVal)

#define ITTYParams_get_Mode(This,pVal)	\
    (This)->lpVtbl -> get_Mode(This,pVal)

#define ITTYParams_put_Mode(This,newVal)	\
    (This)->lpVtbl -> put_Mode(This,newVal)

#define ITTYParams_get_MarkFreq(This,pVal)	\
    (This)->lpVtbl -> get_MarkFreq(This,pVal)

#define ITTYParams_put_MarkFreq(This,newVal)	\
    (This)->lpVtbl -> put_MarkFreq(This,newVal)

#define ITTYParams_get_SpaceFreq(This,pVal)	\
    (This)->lpVtbl -> get_SpaceFreq(This,pVal)

#define ITTYParams_put_SpaceFreq(This,newVal)	\
    (This)->lpVtbl -> put_SpaceFreq(This,newVal)

#define ITTYParams_get_SignalThreshold(This,pVal)	\
    (This)->lpVtbl -> get_SignalThreshold(This,pVal)

#define ITTYParams_put_SignalThreshold(This,newVal)	\
    (This)->lpVtbl -> put_SignalThreshold(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITTYParams_get_Rate_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [retval][out] */ TTY_RATE __RPC_FAR *pVal);


void __RPC_STUB ITTYParams_get_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITTYParams_put_Rate_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [in] */ TTY_RATE newVal);


void __RPC_STUB ITTYParams_put_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITTYParams_get_Mode_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [retval][out] */ TTY_MODE __RPC_FAR *pVal);


void __RPC_STUB ITTYParams_get_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITTYParams_put_Mode_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [in] */ TTY_MODE newVal);


void __RPC_STUB ITTYParams_put_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITTYParams_get_MarkFreq_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB ITTYParams_get_MarkFreq_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITTYParams_put_MarkFreq_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB ITTYParams_put_MarkFreq_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITTYParams_get_SpaceFreq_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB ITTYParams_get_SpaceFreq_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITTYParams_put_SpaceFreq_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB ITTYParams_put_SpaceFreq_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ITTYParams_get_SignalThreshold_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pVal);


void __RPC_STUB ITTYParams_get_SignalThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ITTYParams_put_SignalThreshold_Proxy( 
    ITTYParams __RPC_FAR * This,
    /* [in] */ float newVal);


void __RPC_STUB ITTYParams_put_SignalThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITTYParams_INTERFACE_DEFINED__ */


#ifndef ___IPhoneToneDecoderEvents_DISPINTERFACE_DEFINED__
#define ___IPhoneToneDecoderEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IPhoneToneDecoderEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IPhoneToneDecoderEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("D2ED31A6-E080-43F3-80DF-DE27E8D8C715")
    _IPhoneToneDecoderEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IPhoneToneDecoderEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IPhoneToneDecoderEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IPhoneToneDecoderEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IPhoneToneDecoderEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _IPhoneToneDecoderEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _IPhoneToneDecoderEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _IPhoneToneDecoderEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _IPhoneToneDecoderEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _IPhoneToneDecoderEventsVtbl;

    interface _IPhoneToneDecoderEvents
    {
        CONST_VTBL struct _IPhoneToneDecoderEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IPhoneToneDecoderEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IPhoneToneDecoderEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IPhoneToneDecoderEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IPhoneToneDecoderEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IPhoneToneDecoderEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IPhoneToneDecoderEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IPhoneToneDecoderEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IPhoneToneDecoderEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_PhoneToneDecoder;

#ifdef __cplusplus

class DECLSPEC_UUID("131FE04C-1D39-4205-AB75-5F9834E754A6")
PhoneToneDecoder;
#endif
#endif /* __TONEDECODERLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
