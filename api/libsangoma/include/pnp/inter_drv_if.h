/*++

Copyright (c) 1985-2005 Sangoma Technologies All Rights Reserved

Module Name:

    inter_drv_if.h

Abstract:

    This module contains the declarations for inter driver
    communication. Drivers are placed in a stack and call each
	other.

	Shared between: sdladrv.sys, sprotocol.sys and wanpipe.sys.

Author:

    David Rokhvarg October 20, 2004

Environment:

    kernel mode only

Notes:


Revision History:


--*/

#ifndef INTER_DRV_IF_H
#define INTER_DRV_IF_H

#include <wmilib.h>

#include <wanpipe_includes.h>
#include <wanrouter.h>
#include <wanpipe.h>
#include <wanpipe_cfg.h>			//for wanif_conf_t
#include <sdlapci.h>


#define BUSENUM_COMPATIBLE_IDS			L"CommsAdapter\\SangomaAdapter\0"
#define BUSENUM_COMPATIBLE_IDS_LENGTH	sizeof(BUSENUM_COMPATIBLE_IDS)

//the ID is the same as at 'sdladrv.sys' level - the way it should be
//in order not limit driver stack size
#define SPROTOCOL_COMPATIBLE_IDS		BUSENUM_COMPATIBLE_IDS
#define SPROTOCOL_COMPATIBLE_IDS_LENGTH BUSENUM_COMPATIBLE_IDS_LENGTH


//these are defined in sources file
#if defined( VIRTUAL_IF_DRV ) || defined( BUSENUM_DRV )
	#define SERIAL_NUMBER_RANGE_INCREMENT	500
#endif

#if defined( VIRTUAL_IF_DRV )

	#define HW_IF_NAME	"WANPIPE"

	#define BUSENUM_POOL_TAG (ULONG) 'fiVT'

#elif defined( BUSENUM_DRV )

	#define BUSENUM_POOL_TAG (ULONG) 'suBT'

#elif defined( NDIS_MINIPORT_DRIVER )

#elif defined( SPROTOCOL )

	#define SERIAL_NUMBER_RANGE_INCREMENT	500 //1 ??

	#define SPROTOCOL_POOL_TAG	(ULONG)'trpT'

#endif

#define MAX_NAME_LENGTH	256
#define TMP_STR_LEN		300

///////////////////////////////////////////////////////////////////////////////
//uncomment -DSANG_DBG in C_DEFINES in Sources to get debug output
//
#if defined(SANG_DBG)

//print driver name and a line
#define DbgOut(_f_, _x_) if(_f_) { DbgPrint("%s: ", DRIVER_NAME); DbgPrint _x_;}
//print a line. used when driver name is not needed
#define DbgOutL(_f_, _x_) if(_f_) { DbgPrint _x_;}

#define BUS_DEFAULT_DEBUG_OUTPUT_LEVEL 0x000FFFFF

#else //#if DBG

#define BUS_DEFAULT_DEBUG_OUTPUT_LEVEL 0x0

#define DbgOut(_f_, _x_)
#define DbgOutL(_f_, _x_)

#endif//#if DBG

extern ULONG BusEnumDebugLevel;

extern int HardwareInterfaceNumber;

///////////////////////////////////////////////////////////////////////////////
// These are the states a PDO or FDO transition upon
// receiving a specific PnP Irp. Refer to the PnP Device States
// diagram in DDK documentation for better understanding.
typedef enum _DEVICE_PNP_STATE {

    NotStarted = 0,         // Not started yet
    Started,                // Device has received the START_DEVICE IRP
    StopPending,            // Device has received the QUERY_STOP IRP
    Stopped,                // Device has received the STOP_DEVICE IRP
    RemovePending,          // Device has received the QUERY_REMOVE IRP
    SurpriseRemovePending,  // Device has received the SURPRISE_REMOVE IRP
    Deleted,                // Device has received the REMOVE_DEVICE IRP
    UnKnown                 // Unknown state

} DEVICE_PNP_STATE;

typedef struct _GLOBALS {
    // 
    // Path to the driver's Services Key in the registry
    //
    UNICODE_STRING RegistryPath;
	//DDDDD
	ULONG last_serial_number_range;

} GLOBALS;

extern GLOBALS Globals;

///////////////////////////////////////////////////////////////////////////////
// Structure for reporting data to WMI
typedef struct _TOASTER_BUS_WMI_STD_DATA {

    // The error Count
    UINT32   ErrorCount;

    // Debug Print Level
    UINT32  DebugPrintLevel;

} TOASTER_BUS_WMI_STD_DATA, * PTOASTER_BUS_WMI_STD_DATA;

///////////////////////////////////////////////////////////////////////////////
//
// A common header for the device extensions of the PDOs and FDO
//
typedef struct _COMMON_DEVICE_DATA
{
    // A back pointer to the device object for which this is the extension
    PDEVICE_OBJECT  Self;

    // This flag helps distinguish between PDO and FDO
    BOOLEAN         IsFDO;

    // We track the state of the device with every PnP Irp
    // that affects the device through these two variables.
    DEVICE_PNP_STATE	DevicePnPState;

    DEVICE_PNP_STATE	PreviousPnPState;

    ULONG				DebugLevel;

    // Stores the current system power state
    SYSTEM_POWER_STATE  SystemPowerState;

    // Stores current device power state
    DEVICE_POWER_STATE  DevicePowerState;

} COMMON_DEVICE_DATA, *PCOMMON_DEVICE_DATA;

///////////////////////////////////////////////////////////////////////////////

//
// The device extension for the PDOs.
// That's of the toaster device which this bus driver enumerates.
//
typedef struct _PDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    // A back pointer to the bus
    PDEVICE_OBJECT  ParentFdo;

    // pointer to FDO attached to this PDO
    //void*  attachedFdoExtension;

    // An array of (zero terminated wide character strings).
    // The array itself also null terminated
    PWCHAR      HardwareIDs;

    // Unique serail number of the device on the bus
    ULONG		SerialNo;

    // Link point to hold all the PDOs for a single bus together
    LIST_ENTRY  Link;
    
    //
    // Present is set to TRUE when the PDO is exposed via PlugIn IOCTL,
    // and set to FALSE when a UnPlug IOCTL is received. 
    // We will delete the PDO in IRP_MN_REMOVE only after we have reported 
    // to the Plug and Play manager that it's missing.
    //
    BOOLEAN     Present;
    BOOLEAN     ReportedMissing;
    UCHAR       Reserved[2]; // for 4 byte alignment

    //
    // Used to track the intefaces handed out to other drivers.
    // If this value is non-zero, we fail query-remove.
    //
    ULONG       ToasterInterfaceRefCount;
    
    //
    // In order to reduce the complexity of the driver, I chose not 
    // to use any queues for holding IRPs when the system tries to 
    // rebalance resources to accommodate a new device, and stops our 
    // device temporarily. But in a real world driver this is required. 
    // If you hold Irps then you should also support Cancel and 
    // Cleanup functions. The function driver demonstrates these techniques.
    //    
    // The queue where the incoming requests are held when
    // the device is stopped for resource rebalance.

    //LIST_ENTRY          PendingQueue;     

    // The spin lock that protects access to  the queue

    //KSPIN_LOCK          PendingQueueLock;     

    //DDDDDDDDDDDDD
	//USHORT card_type;//S508 or S514
	PCM_PARTIAL_RESOURCE_LIST raw_res;
	PCM_PARTIAL_RESOURCE_LIST translated_res;

	UCHAR pdo_cpu;
	UCHAR pdo_port;

	int channel_group_number;

	int (*register_router_net_if)(	void* pdoData,	//this is PPDO_DEVICE_DATA!!
									sdla_wanpipe_interface_t* p_sdla_wanpipe_interface);

	int (*deregister_router_net_if)(	void* pdoData,	//this is PPDO_DEVICE_DATA!!
										sdla_wanpipe_interface_t* p_sdla_wanpipe_interface);

} PDO_DEVICE_DATA, *PPDO_DEVICE_DATA;

///////////////////////////////////////////////////////////////////////////////

typedef struct _PROTOCOL_DATA
{
	//global card configuration. used only at hardware level.
	wandev_conf_t wandev_conf;

	//per-interface configuration
	//Used for:
	//1. for AFT (hardware level): maximum NUM_OF_E1_CHANNELS (31) logic channels
	//   configuration of each logic channel (like 'active_ch',
	//   HDLC or transparent...)
	//2. for Protocol (LIP) layer - configuration of the interfaces (i.g. DLCIs)
	//   of the protocol. 
	wanif_conf_t if_cfg[MAX_NUMBER_OF_PROTOCOL_INTERFACES];

}PROTOCOL_DATA, *PPROTOCOL_DATA;

typedef struct _WANPIPE_DATA
{
	ULONG res_size;	//sizeof allocated resorces

	//this is for saving resources for configurator
	ULONG PNP_memory, PNP_io_port, PNP_interrupt;

	//these are untranslated resources
	ULONG				interruptnumber;

	PHYSICAL_ADDRESS	portbase;
	ULONG				nports;	// number of assigned ports
	PUCHAR		mapped_portbase;	// I/O port base address, on RISC platform only
	BOOLEAN		mappedport;			// true if we mapped port addr in StartDevice
	ULONG		sharedmemoryaddress;
	// physical memory *different* from what passed from PnP manager.
	ULONG		ForcedSharedMemoryAddress;

	PHYSICAL_ADDRESS	pci_control_memory_region;
	PHYSICAL_ADDRESS	main_memory_region;

	//interrupt related parameters
	ULONG		vector;
	KIRQL		irql;
	KAFFINITY	affinity;
	BOOLEAN		irqshare;
	KINTERRUPT_MODE	mode;
	
	//global card configuration
	wandev_conf_t wandev_conf;
	//per-interface configuration
	//for AFT card maximum NUM_OF_E1_CHANNELS (31) logic channels
	//configuration of each logic channel (like 'active_ch',
	//HDLC or transparent...)
	wanif_conf_t if_cfg[NUM_OF_E1_CHANNELS];

	PCI_COMMON_CONFIG	PciConfigInfo;

	sdlahw_card_t sdlahw_card_struct;

}WANPIPE_DATA, *PWANPIPE_DATA;

///////////////////////////////////////////////////////////////////////////////
typedef struct _lower_layer_functions{
	int (*get_usage_count)(void);
}lower_layer_functions_t;

///////////////////////////////////////////////////////////////////////////////

typedef struct _FDO_DEVICE_DATA
{
	//
	//must be the 1-st member of this sturcture!!!
	//
	unsigned char	device_type;

    COMMON_DEVICE_DATA;

	//DDDDDDDDDDD
	union{
		WANPIPE_DATA	wanpipe_data;
		PROTOCOL_DATA	protocol_data;
	};
	
	// device object this extension belongs to
    PDEVICE_OBJECT	UnderlyingPDO;
    
    // The underlying bus PDO and the actual device object to
	// which this FDO is attached.
	// (Next lower driver in same stack.)
    PDEVICE_OBJECT  NextLowerDriver;

    // List of PDOs created so far
    LIST_ENTRY      ListOfPDOs;
    
    // The PDOs currently enumerated.
    ULONG           NumPDOs;

    // A synchronization for access to the device extension.
    FAST_MUTEX      Mutex;

    // The number of IRPs sent from the bus to the underlying device object
    ULONG           OutstandingIO; // Biased to 1

    // On remove device plug & play request we must wait until all outstanding
    // requests have been completed before we can actually delete the device
    // object. This event is when the Outstanding IO count goes to zero
    KEVENT          RemoveEvent;

    // This event is set when the Outstanding IO count goes to 1.
    KEVENT          StopEvent;
    
    // The name returned from IoRegisterDeviceInterface,
    // which is used as a handle for IoSetDeviceInterfaceState.
    UNICODE_STRING	InterfaceName;

    //
    // WMI Information
    //
    WMILIB_CONTEXT				WmiLibInfo;
    TOASTER_BUS_WMI_STD_DATA	StdToasterBusData;

    //
    // Unique serial number of the child device to be enumerated.
    // Enumeration will be failed if another device on the 
    // bus has the same serail number.
    //
    ULONG SerialNo;

	//resource count
	ULONG res_count;

	//S514 PCI card resource pointers
	PCM_PARTIAL_RESOURCE_LIST cpu_A_raw_res;
	PCM_PARTIAL_RESOURCE_LIST cpu_A_translated_res;

	PCM_PARTIAL_RESOURCE_LIST cpu_B_raw_res;
	PCM_PARTIAL_RESOURCE_LIST cpu_B_translated_res;

	//S508 ISA card resource pointers
	PCM_PARTIAL_RESOURCE_LIST S508_raw_res;
	PCM_PARTIAL_RESOURCE_LIST S508_translated_res;

	//This counter is needed for the following:
	// 1.
	//Only the first Net interfce should do media and other
	//global configuration.
	//All others will do per DLCI (LCN) configuration.
	// 2.
	//Only the last unloading Net interface should do all
	//the global clean ups.
	ULONG registered_Network_interfaces_counter;

	ULONG serial_number_range_for_this_vir_adapter;
	ULONG last_serial_number_for_this_vir_adapter;

	///////////////////////////////////////////////////////////////////////
	//members needed for the HardwareInterface

	////////////////////////////
	//ISA only resources
	PUCHAR portmem;
    UCHAR SavedWindow;
	//end of ISA only resources
	////////////////////////////
							
	CHAR AdapterName[WAN_IFNAME_SZ*2];

	//end of members needed for the HardwareInterface
	///////////////////////////////////////////////////////////////////////

	//pointer to the Card
	sdla_t * p_sdla_card;

	///////////////////////////////////////////////////////////////////////
	//varialbles used to "talk" to driver above current one.
	void* upper_fdo_dev_data;
	int (*is_ready_to_stop)(void* fdo_dev_data);

#if defined(A104D_CODE)
	//initialize function pointers used to "talk" to driver below current one.
	void (*get_lower_layer_pointers)(lower_layer_functions_t* lower_layer_functions_ptr);

	void *wan_ec_ptr;//HW Echo Canceller pointer. Only ONE per Physical CARD.
	wan_spinlock_t	per_card_lock;
#endif

	///////////////////////////////////////////////////////////////////////
	//functions used by upper level to "talk" to lower level
	NTSTATUS (*ReadPciConfigSpace)(	IN  void* pdx,//PFDO_DEVICE_DATA pdx,
									PUCHAR pBuffer,
									ULONG Offset,
									ULONG Length);

	NTSTATUS (*WritePciConfigSpace)(IN  void* pdx,//PFDO_DEVICE_DATA pdx,
									PUCHAR pBuffer,
									ULONG Offset,
									ULONG Length);
	///////////////////////////////////////////////////////////////////////
	unsigned int driver_mode;//DRV_MODE_NORMAL, DRV_MODE_AFTUP

} FDO_DEVICE_DATA, *PFDO_DEVICE_DATA;

///////////////////////////////////////////////////////////////////////////////

#define FDO_FROM_PDO(pdoData) \
          ((PFDO_DEVICE_DATA) (pdoData)->ParentFdo->DeviceExtension)

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;\


#define LEVEL1_FDO_FROM_LEVEL2_FDO(level2_fdo_pdx)	\
		((PPDO_DEVICE_DATA)level2_fdo_pdx->UnderlyingPDO->DeviceExtension)->	\
			ParentFdo->DeviceExtension;

///////////////////////////////////////////////////////////////////////////////

enum DEV_TYPE {
	DEV_TYPE_LEVEL2_BUS_FDO = 5,
	DEV_TYPE_SDLA_CARD_FDO,
	DEV_TYPE_NET_DEVICE,
	DEV_TYPE_PROTOCOL_FDO
};

#define DECODE_DEV_TYPE(device_type)			\
	(device_type == DEV_TYPE_LEVEL2_BUS_FDO) ?	\
	"DEV_TYPE_LEVEL2_BUS_FDO"	:				\
	(device_type == DEV_TYPE_SDLA_CARD_FDO)	?	\
	"DEV_TYPE_SDLA_CARD_FDO"	:				\
	(device_type == DEV_TYPE_NET_DEVICE)	?	\
	"DEV_TYPE_NET_DEVICE"	:					\
	(device_type == DEV_TYPE_PROTOCOL_FDO)	?	\
	"DEV_TYPE_PROTOCOL_FDO"	:					\
	"Unknown device type!!"

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;\


#define LEVEL1_FDO_FROM_LEVEL2_FDO(level2_fdo_pdx)	\
		((PPDO_DEVICE_DATA)level2_fdo_pdx->UnderlyingPDO->DeviceExtension)->	\
			ParentFdo->DeviceExtension;


////////////////////////////////////////////////////////////////////////////////////
//functions used for communicating between drivers in the stack
//Shared between: sdladrv.sys, sprotocol.sys and wanpipe.sys.
//
int register_with_level_below(sdla_t* card);
int deregister_from_level_below(sdla_t* card);

void receive_indicate_from_below(sdla_t* card, RX_DATA_STRUCT* rx_data);
void tx_complete_indicate_from_below(sdla_t* card);
int transmit_indicate_from_above(netdevice_t*	sdla_net_device, TX_DATA_STRUCT* tx_data);

//card is 'sdla_t' in sdladrv.sys and sprotocol.sys, 'sdla_wanpipe_t' in wanpipe.sys
int management_complete_indicate_from_below(void* card);


#endif//#ifndef INTER_DRV_IF_H

