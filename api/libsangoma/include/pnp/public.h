/*++
Copyright (c) 1990-2000    Microsoft Corporation All Rights Reserved

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Author:

     Eliyas Yakub   Sep 16, 1998
     
Environment:

    user and kernel
Notes:


Revision History:


--*/

//
// Define an Interface Guid for bus enumerator class.
// This GUID is used to register (IoRegisterDeviceInterface) 
// an instance of an interface so that enumerator application 
// can send an ioctl to the bus driver.
//

DEFINE_GUID (GUID_TOASTER_BUS_ENUMERATOR_INTERFACE_CLASS,
        0xD35F7840, 0x6A0C, 0x11d2, 0xB8, 0x41, 0x00, 0xC0, 0x4F, 0xAD, 0x51, 0x71);
//  {D35F7840-6A0C-11d2-B841-00C04FAD5171}
  
//
// Define an Interface Guid for toaster device class.
// This GUID is used to register (IoRegisterDeviceInterface) 
// an instance of an interface so that user application 
// can control the toaster device.
//

DEFINE_GUID (GUID_TOASTER_DEVICE_INTERFACE_CLASS, 
        0x781EF630, 0x72B2, 0x11d2, 0xB8, 0x52, 0x00, 0xC0, 0x4F, 0xAD, 0x51, 0x71);
//{781EF630-72B2-11d2-B852-00C04FAD5171}

//
// Define a Setup Class GUID for Toaster Class. This is same
// as the TOASTSER CLASS guid in the INF files.
//

//DDDDDDDD
DEFINE_GUID (GUID_TOASTER_SETUP_CLASS, 
        0xB85B7C50, 0x6A01, 0x11d2, 0xB8, 0x41, 0x00, 0xC0, 0x4F, 0xAD, 0x51, 0x71);
//{B85B7C50-6A01-11d2-B841-00C04FAD5171}

//Net class
//DEFINE_GUID (GUID_TOASTER_SETUP_CLASS, 
//        0x4d36e972, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18);
//{4d36e972-e325-11ce-bfc1-08002be10318}

//
// GUID definition are required to be outside of header inclusion pragma to avoid
// error during precompiled headers.
//

#ifndef __PUBLIC_H
#define __PUBLIC_H

//DDDDDDDDDDDDDDDDDDD
//#define BUS_HARDWARE_IDS L"Toaster\\MsToaster\0"
//#define BUS_HARDWARE_IDS L"PCI\\VEN_11B0&DEV_0002&SUBSYS_00114753&REV_05\0"
//#define BUS_HARDWARE_IDS           L"CommsAdapter\\SangomaAdapter\0"
//#define BUS_HARDWARE_IDS_LENGTH    sizeof(BUS_HARDWARE_IDS)

/////////////////////////////////////////////////////////////////////////////////////
//
//it is simpler to allocte new devices if all hardware IDs of the SAME length
//
//IDs of virtual Adapters
										//for S5141, S5142(CPU A), S5143 (FT1)
#define BUS_HARDWARE_IDS_CPU_A_S5141    L"CommsAdapter\\SangomaAdapterCPUA_S5141\0"
										//CPU B on S5142 card
#define BUS_HARDWARE_IDS_CPU_B_S5142    L"CommsAdapter\\SangomaAdapterCPUB_S5142\0"

										//for S5147 (dual TE1), CPU A
#define BUS_HARDWARE_IDS_CPU_A_S5147    L"CommsAdapter\\SangomaAdapterCPUA_S5147\0"
										//for S5147 (dual TE1), CPU B
#define BUS_HARDWARE_IDS_CPU_B_S5147    L"CommsAdapter\\SangomaAdapterCPUB_S5147\0"

										//for S5144 (single TE1)
#define BUS_HARDWARE_IDS_CPU_A_S5144    L"CommsAdapter\\SangomaAdapterCPUA_S5144\0"
										//for S5145 (56K)
#define BUS_HARDWARE_IDS_CPU_A_S5145    L"CommsAdapter\\SangomaAdapterCPUA_S5145\0"
										//for S508
#define BUS_HARDWARE_IDS__S508_S5080    L"CommsAdapter\\SangomaAdapterS508_S5080\0"


#define BUS_HARDWARE_IDS_AFT_LINE1		L"CommsAdapter\\SangomaAdapter_AFT_LINE1\0"
#define BUS_HARDWARE_IDS_AFT_LINE2		L"CommsAdapter\\SangomaAdapter_AFT_LINE2\0"
#define BUS_HARDWARE_IDS_AFT_LINE3		L"CommsAdapter\\SangomaAdapter_AFT_LINE3\0"
#define BUS_HARDWARE_IDS_AFT_LINE4		L"CommsAdapter\\SangomaAdapter_AFT_LINE4\0"
#define BUS_HARDWARE_IDS_AFT_LINE5		L"CommsAdapter\\SangomaAdapter_AFT_LINE5\0"
#define BUS_HARDWARE_IDS_AFT_LINE6		L"CommsAdapter\\SangomaAdapter_AFT_LINE6\0"
#define BUS_HARDWARE_IDS_AFT_LINE7		L"CommsAdapter\\SangomaAdapter_AFT_LINE7\0"
#define BUS_HARDWARE_IDS_AFT_LINE8		L"CommsAdapter\\SangomaAdapter_AFT_LINE8\0"

#define BUS_HARDWARE_IDS_AFT_A200		L"CommsAdapter\\SangomaAdapter_AFT__A200\0"

/*
//old version
#define BUS_HARDWARE_IDS_CPU_A_A101     L"CommsAdapter\\SangomaAdapterCPUA_A1010\0"
#define BUS_HARDWARE_IDS_CPU_B_A101     L"CommsAdapter\\SangomaAdapterCPUB_A1010\0"
*/
#define BUS_HARDWARE_IDS_CPU_A_A101     BUS_HARDWARE_IDS_AFT_LINE1
#define BUS_HARDWARE_IDS_CPU_B_A101     BUS_HARDWARE_IDS_AFT_LINE2

//NOTE: all IDs above MUST be of the SAME length!!
#define BUS_HARDWARE_IDS_LENGTH    sizeof(BUS_HARDWARE_IDS_CPU_A_S5141)

/////////////////////////////////////////////////////////////////////////////////////

//IDs of virtual Net Interfaces
//Frame Relay
#define FRAME_REL_HARDWARE_IDS_S514     L"SangomaAdapter\\FrameRelayS514\0"
#define FRAME_REL_HARDWARE_IDS_S508     L"SangomaAdapter\\FrameRelayS508\0"
#define FRAME_REL_HARDWARE_IDS_LENGTH   sizeof(FRAME_REL_HARDWARE_IDS_S514)
//PPP
#define PPP_HARDWARE_IDS_S514           L"SangomaAdapter\\PPP_S514\0"
#define PPP_HARDWARE_IDS_S508           L"SangomaAdapter\\PPP_S508\0"
#define PPP_HARDWARE_IDS_LENGTH			sizeof(PPP_HARDWARE_IDS_S514)
//X25
#define X25_HARDWARE_IDS_S514           L"SangomaAdapter\\X25_S514\0"
#define X25_HARDWARE_IDS_S508           L"SangomaAdapter\\X25_S508\0"
#define X25_HARDWARE_IDS_LENGTH			sizeof(X25_HARDWARE_IDS_S514)
//CHDLC
#define CHDLC_HARDWARE_IDS_S514         L"SangomaAdapter\\CHDLC_S514\0"
#define CHDLC_HARDWARE_IDS_S508         L"SangomaAdapter\\CHDLC_S508\0"
#define CHDLC_HARDWARE_IDS_LENGTH		sizeof(CHDLC_HARDWARE_IDS_S514)

//
//Exposing DIFFERENT ID at each level will allow user
//simply "Search for the driver" instead of installing the
//driver manually.
//
#if defined(VIRTUAL_IF_DRV)
//sdladrv.sys will expose this ID for upper drivers:
 #define AFT_IF_HARDWARE_IDS			L"SangomaAdapter\\AFT_IFACE\0"
 #define AFT_IF_HARDWARE_IDS_LENGTH		sizeof(AFT_IF_HARDWARE_IDS)
#elif defined( SPROTOCOL )
//sprotocol.sys will expose this ID for upper drivers:
 #define SPROTOCOL_IF_IDS				L"SangomaAdapter\\SPROTOCOL_IFACE\0"
 #define SPROTOCOL_IF_IDS_LENGTH		sizeof(SPROTOCOL_IF_IDS)
#endif

/////////////////////////////////////////////////////////////////////////////////////
//DDDDDDDDDDDDDDDDDDD
#define FILE_DEVICE_BUSENUM         FILE_DEVICE_BUS_EXTENDER

#define BUSENUM_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_BUSENUM, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BUSENUM_PLUGIN_HARDWARE               BUSENUM_IOCTL (0x0)
#define IOCTL_BUSENUM_UNPLUG_HARDWARE               BUSENUM_IOCTL (0x1)
#define IOCTL_BUSENUM_EJECT_HARDWARE                BUSENUM_IOCTL (0x2)
#define IOCTL_TOASTER_DONT_DISPLAY_IN_UI_DEVICE     BUSENUM_IOCTL (0x3)

//
//  Data structure used in PlugIn and UnPlug ioctls
//
typedef struct _BUSENUM_PLUGIN_HARDWARE
{
    //
    // sizeof (struct _BUSENUM_HARDWARE)
    //
    IN ULONG Size;                          
    
    //
    // Unique serial number of the device to be enumerated.
    // Enumeration will be failed if another device on the 
    // bus has the same serail number.
    //

    IN ULONG SerialNo;

    
    //
    // An array of (zero terminated wide character strings). The array itself
    //  also null terminated (ie, MULTI_SZ)
    //
    IN  WCHAR   HardwareIDs[]; 
                                                                        
} BUSENUM_PLUGIN_HARDWARE, *PBUSENUM_PLUGIN_HARDWARE;

typedef struct _BUSENUM_UNPLUG_HARDWARE
{
    //
    // sizeof (struct _REMOVE_HARDWARE)
    //

    IN ULONG Size;                                    

    //
    // Serial number of the device to be plugged out    
    //

    ULONG   SerialNo;
    
    ULONG Reserved[2];    

} BUSENUM_UNPLUG_HARDWARE, *PBUSENUM_UNPLUG_HARDWARE;

typedef struct _BUSENUM_EJECT_HARDWARE
{
    //
    // sizeof (struct _EJECT_HARDWARE)
    //

    IN ULONG Size;                                    

    //
    // Serial number of the device to be ejected
    //

    ULONG   SerialNo;
    
    ULONG Reserved[2];    

} BUSENUM_EJECT_HARDWARE, *PBUSENUM_EJECT_HARDWARE;

#endif

