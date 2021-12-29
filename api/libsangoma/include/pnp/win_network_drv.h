
/***********************************************************************************
	win_network_drv.h - Header for Sangoma Network (NDIS) driver.

Versions:
	v 1,0,0,0	March 14 2005		David Rokhvarg	<davidr@sangoma.com>
		For use with A101/A102 cards, Wanpipe version 3.0.0.0

************************************************************************************/

#ifndef _WIN_NETWORK_DRV_
#define _WIN_NETWORK_DRV_

# undef BINARY_COMPATIBLE	
# define BINARY_COMPATIBLE 0	//compile for Win2000 and later

#include <ndis.h>
#include <stdarg.h>	//logging
#include <stdio.h>	//logging too

/*
extern NDIS_HANDLE		cpipeWrapperHandle;
extern PDEVICE_OBJECT	sdla_DeviceObject;
extern NDIS_HANDLE		sdla_NdisDeviceHandle;
extern void*			global_card;
*/

/*
__WINDOWS__ must be always defined in "Sources" file!
*/
#include <wanpipe_defines.h>		//for 'wan_udp_hdr_t'
#include <inter_drv_if.h>
#include <wanpipe_cfg.h>

typedef struct _ADAPTER_CONFIG{

    ULONG		Speed;
	UCHAR		LogErrorsAndInfo;
    ULONG		NodeID;       
    USHORT      UdpPort;
    UCHAR       UdpTTL;

} ADAPTER_CONFIG;

//
// Used when registering ourselves with NDIS.
//
//#define CPIPE_NDIS_MAJOR_VERSION	4
#define CPIPE_NDIS_MAJOR_VERSION	3
#define CPIPE_NDIS_MINOR_VERSION	0

#define MAX_DATA_LEN		1514L	//used to communicate to the TCP/IP stack
#define MAX_TX_RX_DATA_SIZE	2000	//temporary Tx/Rx buffers length. Just in case,
									//it must be bigger than MAX_DATA_LEN!!
////////////////////////////////////////////////////////////////////
// stuff from wanpipe_include.h

#define PROTOCOL_TYPE_IP		0x45

/*
ARP packet format (ether), RFC 826:
48-bit dest address
48-bit src address
16-bit packet type (0800 = IP, 0806 = ARP)
16-bit hardware address space (10M ethernet is 0001)
16-bit protocol space (IP = 0800)
8-bit UCHAR length hardware address (6 for ether)
8-bit UCHAR length protocol address (IP = 4)
16-bit opcode (request = 0001, reply = 0002)
48-bit source address
32-bit protocol source address
48-bit dest address
32-bit protocol dest address
(padding)
*/
#pragma pack(1)

typedef struct _ARP_RETURN 
{
    UCHAR DestAddr[6];
    UCHAR SrcAddr[6];
    USHORT EtherType;
    USHORT AddrSpace;
    USHORT ProtoSpace;
    UCHAR HardAddrBytes;
    UCHAR ProtoAddrBytes;
    USHORT OpCode;
    UCHAR ArpSrcAddr[6];
    UCHAR ArpProtoSrcAddr[4];
    UCHAR ArpDestAddr[6];
    UCHAR ArpProtoDestAddr[4];
    UCHAR Padding[18];
} ARP_RETURN;

// Ethernet address 'prefix'
// 6-byte local address is
//  ETHADD0:ETHADD1:ETHADD2:00:DLCI:00
// 6-byte remote address is always
//  ETHADD0:ETHADD1:ETHADD2:00:DLCI:02
// according to what we tell the ARP module via ARP replies
// note: 0xff:0xff:0xff did not work
// all this changed in v1.1 to:
//  ETHADD0:NODEID:00/02
#define ETHADD0			0x02
#define DEFAULT_NODEID  0x0000fefe

/* UDP/IP packet (for UDP management) layout */
typedef struct 
{
	unsigned char	protocol;
	unsigned char	service_type;
	unsigned short	ip_length;
	unsigned char	reserved2[4];
	unsigned char	ip_ttl;
	unsigned char	ip_protocol;
	unsigned short	ip_checksum;
	unsigned long	ip_src_address;
	unsigned long	ip_dst_address;
	unsigned short	udp_src_port;
	unsigned short	udp_dst_port;
	unsigned short	udp_length;
	unsigned short	udp_checksum;
} ip_packet_t;

typedef struct 
{
    UCHAR DestAddr[6];
    UCHAR SrcAddr[6];
    USHORT EtherType;
} ethernet2_header_t;

#pragma pack()

/* Adapter Data Space.
 * This structure is needed because we handle multiple cards, otherwise
 * static data would do it.
 */
typedef struct _sdla_wanpipe_t{

    // Handle given by NDIS when the adapter was registered.
    NDIS_HANDLE MiniportAdapterHandle;

	USHORT	instances;

    //
    // Will be true the first time that the hardware is initialized
    // by the driver initialization.
    //
    BOOLEAN	FirstInitialization;

    UINT	HardwareFailureCount;

    //
    // The current packet filter.
    //
	ULONG	CurrentPacketFilter;

    //
    // These hold adapter statistics.
    //
    ULONG LineSpeed;

    //
    // The burned-in network address from the hardware.
    //
    UCHAR	PermanentNetworkAddress[6];

    ULONG	xmit_ok, rcv_ok, xmit_error, rcv_error, rcv_no_buffer;

    UCHAR	SendPacketData[MAX_TX_RX_DATA_SIZE];// v1.3 change - was 1514 before

    UCHAR	ReceivePacketData[MAX_TX_RX_DATA_SIZE];

/*
    UCHAR	pending_tx_data[MAX_TX_RX_DATA_SIZE];
	ULONG	pending_tx_data_length;
	PNDIS_PACKET	pending_tx_packet;
	NDIS_SPIN_LOCK  txSpinLock;
	unsigned int	number_of_tx_pending_returned;
*/

    ARP_RETURN ArpReturn;
    // if 1 then throw away sent ARP and don't reply because we are already
    // replying to another one sent earlier, and we don't want to buffer
    // all of these
    CHAR ArpInProgress;

	UCHAR LogErrorsAndInfo;

	// this is the system time the adapter has been brought up 
	LARGE_INTEGER StartTime;

	UCHAR devname[WAN_IFNAME_SZ];

	//It is the physical device object on top which this driver runs.
	PPDO_DEVICE_DATA pdoData;
	//this is the Functional dev obj. of the above phys. obj.
	PFDO_DEVICE_DATA p_fdo;

	sdla_wanpipe_interface_t	sdla_wanpipe;

	TX_DATA_STRUCT				tx_data;

	////////////////////////////////////////////////////
	//configuration from registry
	ADAPTER_CONFIG		acfg;

} sdla_wanpipe_t;

//
// Given a MacContextHandle return the PSONIC_ADAPTER
// it represents.
//
#define PCPIPE_ADAPTER_FROM_CONTEXT_HANDLE(Handle) \
    ((sdla_wanpipe_t *)((PVOID)(Handle)))

//
// Definitions of sonic functions which are used by multiple
// source files.
//

//
// packet.c
//
extern
VOID
cpipeCopyFromPacketToBuffer(
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    IN UINT BytesToCopy,
    OUT PCHAR Buffer,
    OUT PUINT BytesCopied
    );

extern
VOID
cpipeCopyFromBufferToPacket(
    IN PCHAR Buffer,
    IN UINT BytesToCopy,
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    OUT PUINT BytesCopied
    );

extern
VOID
cpipeCopyFromPacketToPacket(
    IN PNDIS_PACKET Destination,
    IN UINT DestinationOffset,
    IN UINT BytesToCopy,
    IN PNDIS_PACKET Source,
    IN UINT SourceOffset,
    OUT PUINT BytesCopied
    );

//
// request.c
//
extern
NDIS_STATUS
cpipeQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );

extern
NDIS_STATUS
cpipeSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    );

//
// send.c
//
extern
NDIS_STATUS
cpipeSend(
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN PNDIS_PACKET Packet,
    IN UINT SendFlags
    );

extern
VOID
cpipeTimerHandler(
    IN PVOID s1,
    IN PVOID MiniportAdapterContext,
    IN PVOID s2,
    IN PVOID s3
    );

void
tx_complete_indicate(
	void* wanpipe_if
	);

// 
// cpipe.c
//
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

BOOLEAN
cpipeCheckForHang(
    IN PVOID MiniportAdapterContext
    );

VOID
cpipeHalt(
	IN NDIS_HANDLE MiniportAdapterContext  
	);

VOID
cpipeShutdown(
	IN PVOID  ShutdownContext 
	);

NDIS_STATUS
cpipeInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE ConfigurationHandle
    );

NDIS_STATUS
cpipeReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
cpipeRegisterAdapter(
    IN NDIS_HANDLE MiniportAdapterHandle,
	IN ADAPTER_CONFIG * fc
    );

//
// transfer.c
//
extern
NDIS_STATUS
cpipeTransferData(
    OUT PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer
    );

//
// COMMON.C
//

extern char	ThreadFunctionRunning;
extern char ThreadTerminate;

void OutputLogString(PUCHAR pszFormat, ...);
void init_globals();
void ThreadFunction(char * buf);

int start_log_thread();

//////////////////////////////////////////////////////////////////////////////
//
//macros, functions, structures for accessing the card
//
#define WANPIPE_WRITE_PORT(_Adapter, _Port, _Value) \
	NdisRawWritePortUchar((_Adapter)->PortAddress + _Port, (UCHAR)(_Value))

#define WANPIPE_READ_PORT(_Adapter, _Port, _Value) \
	NdisRawReadPortUchar((_Adapter)->PortAddress + _Port, (PUCHAR)(_Value))

///////////////////////////////////////////////////////////////////////////////
//
// PowerPC platform special macros.										-A-
// Used to avoid the misaligned memory references
//

// Macro GET_ULONG_FROM_UCHAR_ARRAY( to, from_arr, from_pos )
// The macro should be used if you want to read some 4 UCHAR piece of UCHAR array
// as a ULONG value.
//
// Example of usage:
// 
// Instead of writing 
//		to = *(ULONG*)&from_arr[from_pos];
// write
//		GET_ULONG_FROM_UCHAR_ARRAY( to, from_arr, from_pos );
//
#define GET_ULONG_FROM_UCHAR_ARRAY( to, from_arr, from_pos )	\
{																\
	UINT qwer_abcde;											\
	PUCHAR to_ptr = (PUCHAR)&(to);								\
	for(qwer_abcde=0; qwer_abcde<4; qwer_abcde++)				\
	to_ptr[qwer_abcde] = from_arr[(from_pos)+qwer_abcde];		\
}
	
	
// Macro GET_USHORT_FROM_UCHAR_ARRAY( to, from_arr, from_pos )
// The macro should be used if you want to read some 2 UCHAR piece of UCHAR array
// as a USHORT value.
//
// Example of usage:
// 
// Instead of writing 
//		to = *(USHORT*)&from_arr[from_pos];
// write
//		GET_USHORT_FROM_UCHAR_ARRAY( to, from_arr, from_pos );
//
#define GET_USHORT_FROM_UCHAR_ARRAY( to, from_arr, from_pos )	\
{																\
	PUCHAR to_ptr = (PUCHAR)&(to);								\
	to_ptr[0] = from_arr[(from_pos)];							\
	to_ptr[1] = from_arr[(from_pos)+1];							\
}
	
// Macro PUT_USHORT_TO_UCHAR_ARRAY( from, to_arr, to_pos )
// The macro should be used if you want to write to some 2 UCHAR piece of UCHAR array
// a USHORT value.
//
// Example of usage:
// 
// Instead of writing 
//		*(USHORT*)&to_arr[to_pos] = from;
// write
//		PUT_USHORT_TO_UCHAR_ARRAY( from, to_arr, to_pos );
//
#define PUT_USHORT_TO_UCHAR_ARRAY( from, to_arr, to_pos )	\
{															\
	PUCHAR from_ptr = (PUCHAR)&(from);						\
	to_arr[(to_pos)] = from_ptr[0];							\
	to_arr[(to_pos)+1] = from_ptr[1];						\
}
		
//																		-A-
//
// Macros used for memory allocation and deallocation.
//
// Note that for regular memory we put no limit on the physical
// address, but for contiguous and noncached we limit it to
// 32 bits since that is all the card can handle (presumably
// such memory will be DMAed to/from by the card).
//

#define WANPIPE_ALLOC_MEMORY(_Status, _Address, _Length)				\
{																		\
	NDIS_PHYSICAL_ADDRESS Temp = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);	\
	*(_Status) = NdisAllocateMemory(									\
	(PVOID)(_Address),									\
	(_Length),											\
	0,													\
	Temp												\
	);													\
}

#define WANPIPE_FREE_MEMORY(_Address, _Length)		\
	NdisFreeMemory(									\
	(PVOID)(_Address),								\
	(_Length),										\
	0												\
)

#define WANPIPE_ALLOC_CONTIGUOUS_MEMORY(_Status, _Address, _Length)		\
{																		\
	NDIS_PHYSICAL_ADDRESS Temp = NDIS_PHYSICAL_ADDRESS_CONST(-1, 0);	\
	*(_Status) = NdisAllocateMemory(									\
	(PVOID)(_Address),									\
	(_Length),											\
	NDIS_MEMORY_CONTIGUOUS,								\
	Temp												\
	);													\
}

#define WANPIPE_FREE_CONTIGUOUS_MEMORY(_Address, _Length)	\
	NdisFreeMemory(											\
	(PVOID)(_Address),										\
	(_Length),												\
	NDIS_MEMORY_CONTIGUOUS									\
)


#define WANPIPE_ALLOC_NONCACHED_MEMORY(_Status, _Address, _Length)	\
{																	\
	NDIS_PHYSICAL_ADDRESS Temp = NDIS_PHYSICAL_ADDRESS_CONST(-1, 0);\
	*(_Status) = NdisAllocateMemory(								\
	(PVOID)(_Address),									\
	(_Length),											\
	NDIS_MEMORY_CONTIGUOUS | NDIS_MEMORY_NONCACHED,		\
	Temp												\
	);													\
}

#define WANPIPE_FREE_NONCACHED_MEMORY(_Address, _Length)	\
	NdisFreeMemory(											\
	(PVOID)(_Address),										\
	(_Length),												\
	NDIS_MEMORY_CONTIGUOUS | NDIS_MEMORY_NONCACHED			\
)

//
// Macros to move and zero memory.
//
#define WANPIPE_MOVE_MEMORY(Destination,Source,Length)			\
	NdisMoveMemory((PVOID)(Destination),(PVOID)(Source),Length)

#define WANPIPE_ZERO_MEMORY(Destination,Length) NdisZeroMemory(Destination,Length)

#endif // _WIN_NETWORK_DRV_
