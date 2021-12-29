/*++

Copyright (c) 1985-2005 Sangoma Technologies All Rights Reserved

Module Name:

    busenum.h

Abstract:

    This module contains the common private declarations 
    for the Sangoma Bus enumerator.

Author:

    David Rokhvarg February 15, 2005

Environment:

    kernel mode only. not for NDIS drivers.

Notes:


Revision History:


--*/

#ifndef BUSENUM_H
#define BUSENUM_H

#include <inter_drv_if.h>//data types

/////////////////////////////////////////////////////////////////////
//S508 definitions
//
//port constants
//
//the offset from the card I/O port base address of the adapter
//segment and PC window select latch 
#define OFF_PC_ADDR_WS_SELECT 	0x01

// the offset from the card I/O port base address
// of the Z80 window select latch
#define OFF_Z80_WIN_SELECT 	0x02	

#define STANDARD_WINDOW     0x07

#define SANG_READ_PORT(device, offset)	\
	READ_PORT_UCHAR((PUCHAR)(device->portmem+offset))

#define SANG_WRITE_PORT(device, offset, value)	\
	WRITE_PORT_UCHAR((PUCHAR)(device->portmem+offset), value)

//End of S508 definitions
/////////////////////////////////////////////////////////////////////

#define MB_SIZE					16	//size of mail box without the data area
#define MAILBOX_DATA		    16	//offset of data area in the mail box structure
//#define MAILBOX_RCODE		    4
//#define MAILBOX_BUFFER_LENGTH	2
//#define MAILBOX_COMMAND			1
     
#define TIMEOUT_INTERVAL			20000
//#define Z80_TIMEOUT_ERROR			0x7f
#define INITIALIZE_TIMEOUT_ERROR	0x7e

/////////////////////////////////////////////////////////////////////
//
// Prototypes of functions in different files.
//

//
// Defined in common.c
//
unsigned long
parse_active_channel(
	char* val,
	unsigned char media_type
	);

ULONG
my_strtoul(
	PCHAR str,
	PCHAR* endstr,
	int base);

int 
active_channels_str_invalid_characters_check(
	char* active_ch_str
	);

unsigned long 
get_active_channels(
	int channel_flag,
	int start_channel,
	int stop_channel,
	unsigned char media_type
	);

int
check_channels(
	int channel_flag,
	unsigned int start_channel,
	unsigned int stop_channel,
	unsigned char media_type
	);

ULONG 
my_strtoul(
	PCHAR str,
	PCHAR* endstr,
	int base
	);

//
// Defined in DriverEntry.C
//
NTSTATUS 
DriverEntry (
	IN PDRIVER_OBJECT, 
	PUNICODE_STRING
	);

NTSTATUS
Bus_CreateClose (
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);

NTSTATUS
Bus_IoCtl (
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);

VOID
Bus_DriverUnload (
	IN PDRIVER_OBJECT DriverObject
	);

VOID
Bus_IncIoCount (
	PFDO_DEVICE_DATA   Data
	);

VOID
Bus_DecIoCount (
	PFDO_DEVICE_DATA   Data
	);

//
// Defined in PNP.C
//
PCHAR
PnPMinorFunctionString (
	UCHAR MinorFunction
	);

NTSTATUS
Bus_CompletionRoutine (
	IN PDEVICE_OBJECT   DeviceObject,
	IN PIRP             Pirp,
	IN PVOID            Context
	);

NTSTATUS
Bus_SendIrpSynchronously (
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);

NTSTATUS
Bus_PnP (
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);

NTSTATUS
Bus_AddDevice(
	IN PDRIVER_OBJECT DriverObject,
	IN PDEVICE_OBJECT BusDeviceObject
	);

NTSTATUS
Bus_InitializePdo (
	PDEVICE_OBJECT	Pdo,
	PFDO_DEVICE_DATA FdoData);

NTSTATUS
Bus_PlugInDevice (
	PBUSENUM_PLUGIN_HARDWARE	PlugIn,
	ULONG                       PlugInLength,
	PFDO_DEVICE_DATA            DeviceData,
	UCHAR						cpu,
	int							channel_group_number
	);


NTSTATUS
Bus_UnPlugDevice (
	PBUSENUM_UNPLUG_HARDWARE   UnPlug,
	PFDO_DEVICE_DATA            DeviceData
	);

NTSTATUS
Bus_EjectDevice (
	PBUSENUM_EJECT_HARDWARE     Eject,
	PFDO_DEVICE_DATA            FdoData
	);

void 
Bus_RemoveFdo (
	PFDO_DEVICE_DATA    FdoData
	);

void
card_shutdown (
	IN PFDO_DEVICE_DATA level2_fdo_pdx
    );

NTSTATUS
Bus_DestroyPdo (
	PDEVICE_OBJECT      Device,
	PPDO_DEVICE_DATA    PdoData
	);

NTSTATUS
Bus_FDO_PnP (
	IN PDEVICE_OBJECT       DeviceObject,
	IN PIRP                 Irp,
	IN PIO_STACK_LOCATION   IrpStack,
	IN PFDO_DEVICE_DATA     DeviceData
	);


NTSTATUS
Bus_StartFdo (
	IN  PFDO_DEVICE_DATA            FdoData,
	IN  PIRP   Irp );

PCHAR 
DbgDeviceIDString(
	BUS_QUERY_ID_TYPE Type
	);

PCHAR 
DbgDeviceRelationString(
	IN DEVICE_RELATION_TYPE Type
	);

int 
get_usage_counter(
	sdla_t *card
	);

void 
set_usage_counter(
	sdla_t *card,
	int new_counter
	);

void* 
get_wan_ec_ptr(
	sdla_t *card
	);

void
set_wan_ec_ptr(
	sdla_t *card, 
	IN void *wan_ec_ptr
	);

//
// Defined in Power.c
//
NTSTATUS
Bus_FDO_Power (
	PFDO_DEVICE_DATA    FdoData,
	PIRP                Irp
	);

NTSTATUS
Bus_PDO_Power (
	PPDO_DEVICE_DATA    PdoData,
	PIRP                Irp
	);

NTSTATUS
Bus_Power (
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);

PCHAR
PowerMinorFunctionString (
	UCHAR MinorFunction
	);

PCHAR 
DbgSystemPowerString(
	IN SYSTEM_POWER_STATE Type
	);

PCHAR 
DbgDevicePowerString(
	IN DEVICE_POWER_STATE Type
	);

//
// Defined in BusPDO.c
//
NTSTATUS
Bus_PDO_PnP (
	IN PDEVICE_OBJECT       DeviceObject,
	IN PIRP                 Irp,
	IN PIO_STACK_LOCATION   IrpStack,
	IN PPDO_DEVICE_DATA     DeviceData
	);

NTSTATUS
Bus_PDO_QueryDeviceCaps(
	IN PPDO_DEVICE_DATA     DeviceData,
	IN  PIRP   Irp );

NTSTATUS
Bus_PDO_QueryDeviceId(
	IN PPDO_DEVICE_DATA     DeviceData,
	IN  PIRP   Irp );


NTSTATUS
Bus_PDO_QueryDeviceText(
	IN PPDO_DEVICE_DATA     DeviceData,
	IN  PIRP   Irp );

NTSTATUS
Bus_PDO_QueryResources(
	IN PPDO_DEVICE_DATA     DeviceData,
	IN  PIRP   Irp );

NTSTATUS
Bus_PDO_QueryResourceRequirements(
	IN PPDO_DEVICE_DATA     DeviceData,
	IN  PIRP   Irp );

NTSTATUS
Bus_PDO_QueryDeviceRelations(
	IN PPDO_DEVICE_DATA     DeviceData,
	IN  PIRP   Irp );

NTSTATUS
Bus_PDO_QueryBusInformation(
	IN PPDO_DEVICE_DATA     DeviceData,
	IN  PIRP   Irp );

NTSTATUS
Bus_GetDeviceCapabilities(
	IN  PDEVICE_OBJECT          DeviceObject,
	IN  PDEVICE_CAPABILITIES    DeviceCapabilities);

NTSTATUS
Bus_PDO_QueryInterface(
	IN PPDO_DEVICE_DATA     DeviceData,
	IN  PIRP   Irp );

BOOLEAN
Bus_GetCrispinessLevel(
	IN   PVOID Context,
	OUT  PUCHAR Level);

BOOLEAN
Bus_SetCrispinessLevel(
	IN   PVOID Context,
	OUT  UCHAR Level);

BOOLEAN
Bus_IsSafetyLockEnabled(
	IN PVOID Context);

VOID
Bus_InterfaceReference (
   IN PVOID Context);

VOID
Bus_InterfaceDereference (
   IN PVOID Context);

//
// Defined in WMI.C
//
NTSTATUS
Bus_WmiRegistration(
	IN PFDO_DEVICE_DATA		FdoData);

NTSTATUS
Bus_WmiDeRegistration (
	IN PFDO_DEVICE_DATA     FdoData);

NTSTATUS
Bus_SystemControl (
	IN  PDEVICE_OBJECT  DeviceObject,
	IN  PIRP            Irp);

NTSTATUS
Bus_SetWmiDataItem(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN ULONG GuidIndex,
	IN ULONG InstanceIndex,
	IN ULONG DataItemId,
	IN ULONG BufferSize,
	IN PUCHAR Buffer
	);

NTSTATUS
Bus_SetWmiDataBlock(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN ULONG GuidIndex,
	IN ULONG InstanceIndex,
	IN ULONG BufferSize,
	IN PUCHAR Buffer
	);

NTSTATUS
Bus_QueryWmiDataBlock(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN ULONG GuidIndex,
	IN ULONG InstanceIndex,
	IN ULONG InstanceCount,
	IN OUT PULONG InstanceLengthArray,
	IN ULONG BufferAvail,
	OUT PUCHAR Buffer
	);

NTSTATUS
Bus_QueryWmiRegInfo(
	IN PDEVICE_OBJECT DeviceObject,
	OUT ULONG *RegFlags,
	OUT PUNICODE_STRING InstanceName,
	OUT PUNICODE_STRING *RegistryPath,
	OUT PUNICODE_STRING MofResourceName,
	OUT PDEVICE_OBJECT *Pdo
	);


/////////////////////////////////////////////////////////////////////////////////////////
#define DATA_TYPE_STRING	0
#define DATA_TYPE_INT		1

BOOLEAN
SdlaGetDeviceConfiguration(
	IN PDEVICE_OBJECT		DeviceObject,	// A pointer to the device object
	IN OUT PUNICODE_STRING	StringData,		// Receives the cfg data as a string
	IN OUT PULONG			IntegerData,	// Receives the cfg data as a integer
	IN PUNICODE_STRING		ValueName,		// The name of cfg data as a string
	IN ULONG				DataType		// Indicates data typ of cfg data
	);

BOOLEAN
SdlaSetDeviceConfiguration(
			IN PDEVICE_OBJECT		DeviceObject,	// A pointer to the device object
			IN PUNICODE_STRING	    StringData,		// Receives the cfg data as a string
			IN PUNICODE_STRING		ValueName		// The name of cfg data as a string
		   );
/////////////////////////////////////////////////////////////////////////////////////////

void
ShowAndUpdateResources(
	IN  PFDO_DEVICE_DATA		FdoData,
	IN PCM_PARTIAL_RESOURCE_LIST list);

void
ShowResources(
	IN PCM_PARTIAL_RESOURCE_LIST list);

NTSTATUS
CreateNewVirtualAdapter(
	IN PFDO_DEVICE_DATA FdoData,
	IN PIRP				Irp,
	IN UCHAR			Protocol,
	IN int				channel_group_number);

//
// Defined in init_hardware.c
//
NTSTATUS
CreateHardwareInterface(
	sdla_t * card
	);

NTSTATUS 
AllocateResources(	
	IN sdla_t *	card,
	IN PCM_PARTIAL_RESOURCE_LIST	raw, 
	IN PCM_PARTIAL_RESOURCE_LIST	translated
	);

BOOLEAN
Map_S514_PCI_Memory(
	sdla_t * card
	);

BOOLEAN
Map_AFT_PCI_Memory(
	sdla_t * card
	);

BOOLEAN
DoMemoryTest(
	sdla_t *	card
	);

BOOLEAN
Load_S514(
	sdla_t *	card
	);

BOOLEAN
ReadConfiguration(
	sdla_t * card
	);

unsigned char 
read_firmware_file_into_memory(
	sdla_t * card
	);

void
deallocate_firmware_file_memory_buffer(
	sdla_t * card
	);

/////////////////////////////////////////////////////////////////////
#define MAX_LOGSTRING	400

//needed by NotePad to read the log file correctly
#define WIN_NEW_LINE "\r\n"

int init_log();

/////////////////////////////////////////////////////////////////////

//these are used for resource parsing passed to the driver.
//S514
#define S514_1CPU_RES_COUNT								5
#define S514_2CPU_RES_COUNT								7
#define BASE_REGISTER_ADDRESS_RESOURCE_NUMBER			0
#define BASE_MEMORY_ADDRESS_CPU_A_RESOURCE_NUMBER		2
#define BASE_MEMORY_ADDRESS_CPU_B_RESOURCE_NUMBER		4//resource 6 is IRQ
//S508
#define S508_RES_COUNT									3
//AFT
#define A101_1CPU_RES_COUNT								3
#define A102_2CPU_RES_COUNT								5
#define BASE_REGISTER_ADDRESS_RESOURCE_NUMBER			0
#define BASE_MEMORY_ADDRESS_CPU_A_RESOURCE_NUMBER		2
#define BASE_MEMORY_ADDRESS_CPU_B_RESOURCE_NUMBER		4//resource 6 is IRQ

//number of resources after removing "unknown resources" passed by OS
#define A101_1CPU_REDUCED_RES_COUNT						2
//A104 has one Memory aperture and one Interrupt
#define A104_RES_COUNT									3//2

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

#endif//#ifndef BUSENUM_H

