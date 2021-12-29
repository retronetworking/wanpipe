/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Thu Mar 01 00:28:22 2007
 */
/* Compiler settings for E:\source2\ToneDecoder\ToneDecoderOcx\ToneDecoder.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_ITonePagingParams = {0x54BD50C7,0xE15F,0x49CD,{0x84,0x5A,0x30,0x87,0xAC,0x56,0xEA,0xFF}};


const IID IID_IPhoneToneDecoder = {0x818DD928,0xDC2E,0x4BE6,{0x8D,0x15,0x52,0xAC,0x2A,0x1A,0x63,0x0E}};


const IID LIBID_TONEDECODERLib = {0x91166F70,0x45F7,0x4E49,{0x9F,0x21,0x49,0xC2,0x73,0x4E,0xA1,0xC3}};


const IID IID_IMonitorTonesParams = {0x54BD50C7,0xE15F,0x49CD,{0x84,0x5A,0x30,0x87,0xAC,0x55,0xEA,0xFF}};


const IID IID_ICustomToneParams = {0x54BD50C7,0xE15F,0x49CD,{0x84,0x5A,0x30,0x87,0xAC,0x52,0xEA,0xFF}};


const IID IID_IDTMFParams = {0x54BD50C7,0xE15F,0x49CD,{0x84,0x5A,0x30,0x87,0xAC,0x53,0xEA,0xFF}};


const IID IID_ITTYParams = {0x54BD50C7,0xE15F,0x49CD,{0x84,0x5A,0x30,0x87,0xAC,0x54,0xEA,0xFF}};


const IID DIID__IPhoneToneDecoderEvents = {0xD2ED31A6,0xE080,0x43F3,{0x80,0xDF,0xDE,0x27,0xE8,0xD8,0xC7,0x15}};


const CLSID CLSID_PhoneToneDecoder = {0x131FE04C,0x1D39,0x4205,{0xAB,0x75,0x5F,0x98,0x34,0xE7,0x54,0xA6}};


#ifdef __cplusplus
}
#endif
