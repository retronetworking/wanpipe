/*****************************************************************************
* libxmtp2.h	SS7 MTP2 API Library 
*
* Author(s):	Mike Mueller <ss7box@gmail.com
*               Nenad Corbic <ncorbic@sangoma.com>
*		
*
* Copyright:	(c) 2008 Xygnada Technologies
*                        Sangoma Technologies Inc.
*
* ============================================================================
*/

#ifndef _LIBXMTP2_
#define _LIBXMTP2_

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/if.h>

#define MARK			1
#define MTP2_SEC_MS(sec)  	(sec*1000000)

#define XMTP2_VERSION	1.6

/*=================================================================
 * Ioctl definitions
 *===============================================================*/

/* needed for the _IOW etc stuff used later */
#include <linux/ioctl.h> 

/* Use '7' as magic number */
#define XMTP2KM_IOC_MAGIC  '7'

#define XMTP2KM_IOCRESET    _IO(XMTP2KM_IOC_MAGIC, 0)

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */

#define XMTP2KM_IOCS_BINIT 	_IOW (XMTP2KM_IOC_MAGIC,  1, uint8_t *)
#define XMTP2KM_IOCS_OPSPARMS 	_IOW (XMTP2KM_IOC_MAGIC,  2, uint8_t *)
#define XMTP2KM_IOCS_PWR_ON 	_IOW (XMTP2KM_IOC_MAGIC,  3, uint8_t *)
#define XMTP2KM_IOCS_EMERGENCY 	_IOW (XMTP2KM_IOC_MAGIC,  4, uint8_t *)
#define XMTP2KM_IOCS_EMERGENCY_CEASES 	_IOW (XMTP2KM_IOC_MAGIC,  5, uint8_t *)
#define XMTP2KM_IOCS_STARTLINK 	_IOW (XMTP2KM_IOC_MAGIC,  6, uint8_t *)
#define XMTP2KM_IOCG_GETMSU 	_IOR (XMTP2KM_IOC_MAGIC,  7, uint8_t *)
#define XMTP2KM_IOCS_PUTMSU 	_IOW (XMTP2KM_IOC_MAGIC,  8, uint8_t *)
#define XMTP2KM_IOCX_GETTBQ 	_IOWR(XMTP2KM_IOC_MAGIC,  9, uint8_t *)
#define XMTP2KM_IOCS_LNKRCVY 	_IOW (XMTP2KM_IOC_MAGIC,  10, uint32_t)
#define XMTP2KM_IOCX_GETBSNT 	_IOWR(XMTP2KM_IOC_MAGIC,  11, uint8_t *)
#define XMTP2KM_IOCS_STOPLINK 	_IOW (XMTP2KM_IOC_MAGIC,  12, uint8_t *)
#define XMTP2KM_IOCG_GETOPM 	_IOR (XMTP2KM_IOC_MAGIC,  13, uint8_t *)
#define XMTP2KM_IOCS_STOP_FAC 	_IOR (XMTP2KM_IOC_MAGIC,  14, uint8_t *)

/*
 * The other entities only have "Tell" and "Query", because they're
 * not printed in the book, and there's no need to have all six.
 * (The previous stuff was only there to show different ways to do it.
 */
/* ... more to come */
#define XMTP2KM_IOCHARDRESET _IO(XMTP2KM_IOC_MAGIC, 15) /* debugging tool */

#define XMTP2KM_IOC_MAXNR 15

/*=================================================================
 * Config defines
 *===============================================================*/

enum	e_parser_controls
{
	NEW_LINE,
	CURRENT_LINE,
};

enum	e_protocol_types
{
	PT_MTP3		= 1,
	PT_M3UA		= 2,
};

enum	facility_types
{
	FACILITY_TYPE_NONE 	= 0,
	FACILITY_TYPE_T1 	= 1,
	FACILITY_TYPE_E1 	= 2,
	FACILITY_TYPE_DDS 	= 3,
	FACILITY_TYPE_UDP 	= 4,
	FACILITY_TYPE_ATMII 	= 5,
};



/*=================================================================
 * Facilities defines
 *===============================================================*/

#define MAX_CHANNELS_IN_FRAME		32
#define T1_CHANNELS_IN_FRAME 		24
#define E1_CHANNELS_IN_FRAME 		31
#define DDS_CHANNELS_IN_FRAME 		1

#define MAX_RX_DATA 1024 * 16

#define CARD_IF_NAME_LEN 31

#define NUM_CHAN_T1			24
#define NUM_CHAN_E1			31
#define MAX_TRUNK_GROUPS		16
#define MAX_SPAN_PER_TG			16
#define MAX_CHANS_PER_SPAN		NUM_CHAN_E1
#define MAX_CHAN_INDEX			MAX_CHANS_PER_SPAN - 1
#define MAX_SPANS			16
#define MAX_SPAN_INDEX			MAX_SPANS - 1
#define MAX_CHANS			MAX_CHANS_PER_SPAN * MAX_SPANS

//#define MAX_NON_VOICE_CHAN_PER_SPAN 	10
//#define NUM_CHAN_T3			672
//#define NUM_CHAN_E3			512
//#define MAX_SPANS			MAX_SPAN_PER_TG * MAX_TRUNK_GROUPS
//#define DOUBLE_MAX_CHAN			MAX_CHAN * 2

enum    ss7box_system_limits
{
	MAX_FACILITIES      = 8,
	MAX_MTP2LINKS       = 248, /* 8 E1 fully loaded with signalling */
	MAX_MTP3LINKSETS    = 248,
	MAX_PATHS       = 248,
	MAX_MTP3SLS     = 32,
	MAX_MTP3_LINKS_IN_LSET  = 16,
	MAX_ROUTEPATHS      = 248,
	MAX_ROUTES      = 248,
	MAX_UDPLINKS        = 2,
};

/* for each link define 128 buffers for rx, 128 for tx, and 128 for retransmission */
#define NUM_BFRS_PER_LINK 3 * 128 
/* double the buffer pool size: one pool each for MTP3 and MTP2 */
#define	B_POOL_SIZE 	 2 * MAX_MTP2LINKS * NUM_BFRS_PER_LINK



/*=================================================================
 * Facilities definitions
 *===============================================================*/

typedef struct
{
	uint32_t	configured;
	uint32_t	linkset;
	uint32_t	link;
	char		if_name[CARD_IF_NAME_LEN];
	uint32_t	cfg_T1;
	uint32_t	cfg_T2;
	uint32_t	cfg_T3;
	uint32_t	cfg_T4Pn;
	uint32_t	cfg_T4Pe;
	uint32_t	cfg_T5;
	uint32_t	cfg_T6;
	uint32_t	cfg_T7;
} t_link_cfg_v2;

typedef struct
{
	int	linkset;
	int	link;
	int	caller;
} t_linkset_link;

typedef struct
{
	int enabled;
	void *dev;
	int (*frame)(void *ptr, int slot, int dir, unsigned char *data, int len);
}t_fac_trace_tap;

typedef struct
{
	int		configured;
	int		status;
	int		delay;
	int		distress_counter;
	int		type;
	int		frames_in_packet;
	//int		channels_in_frame;
	int		clear_channel;
	int		card_name_number;
	char		card_name[CARD_IF_NAME_LEN];
	t_link_cfg_v2	link_cfg[E1_CHANNELS_IN_FRAME];
	unsigned int	bs_block_lost_warning_limit;
	t_fac_trace_tap     tap;		/* For internal use not to be used by api */
	int         pcr_error_correction;	/* Enable pcr error correction */
} t_fac_Sangoma_v2;

enum e_link_recovery_commands
{
	LNKRCVY_ON  = -1,
	LNKRCVY_OFF =  0
};

enum e_bsnt_retrieved_indicators
{
	BSNT_RETRIEVED		=2,
	BSNT_NOT_RETRIEVED	=3
};

#pragma pack(1)
/* PACKED STRUCTURE */
typedef struct
{
	uint8_t		cmd	;
	uint8_t		fi	;
	uint8_t		si	;
	uint8_t		spare	;
} t_lnkrcvy;

/* PACKED STRUCTURE */
typedef struct
{
	uint8_t		ls		;
	uint8_t		link		;
	uint8_t		bsnt		;
	uint8_t		bsnt_retrieved	;
} t_bsnt;
#pragma pack()



/*=================================================================
 * SU Buffer definitions
 *===============================================================*/

#define SU_BFR_SIZE 300

typedef struct
{
	unsigned int	facility;
	unsigned int	slot;
	unsigned int	linkset;
	unsigned int	link;
	unsigned int 	clear_ch;
	unsigned int 	card;
	unsigned int	mtu;
	uint32_t	cfg_T1;
	uint32_t	cfg_T2;
	uint32_t	cfg_T3;
	uint32_t	cfg_T4Pn;
	uint32_t	cfg_T4Pe;
	uint32_t	cfg_T5;
	uint32_t	cfg_T6;
	uint32_t	cfg_T7;
}xmtp_cfg_link_info_t;


typedef struct
{
	/* if anything is changed from here to... */
	void *		p_next; /* buffer pool linked list pointer */
	int		inuse;
	int		from_crosslink;
	char		ac[48];
	unsigned int	facility;
	unsigned int	slot;
	unsigned int	linkset;
	unsigned int	link;
	unsigned int	called;
	unsigned int	caller;
	unsigned int	msg_type; /* see e_mtp2_mtp3_interface_causes below */
	unsigned int	su_octet_count;	/* includes the BSN, FSN , LI and 
						CHKSUM (2 bytes) = 5 bytes */
} t_su_buf_hdr;

typedef struct
{
	t_su_buf_hdr	hdr;
	uint8_t		su_buf[SU_BFR_SIZE];
} t_su_buf;

enum 	e_functional_entities
{
	MGMT 		= -1,
	L3 		= 1,
	LSC 		= 2,
	IAC 		= 3,
	RXC 		= 4,
	TXC 		= 5,
	DAEDR 		= 6,
	DAEDT 		= 7,
	SUERM 		= 8,
	AERM 		= 9,
	TIMER 		= 10,
	FAC_SANGOMA 	= 11,
	TXC_1		= 12,
	FAC_ADAX    = 13,
	ATM_SAAL    = 14
};

enum e_mtp2_mtp3_interface_causes
{
	/* MTP3->MTP2 Generic values */
	TX_MESSAGE			=100,
	EMERGENCY			=101,
	EMERGENCY_CEASE			=102,
	START				=103,
	STOP				=104,
	RETRIEVE_BSNT			=105,
	RETRIEVAL_REQUEST_AND_FSNC	=106,
	RESUME				=107,
	CLEAR_BUFFERS			=108,

	/* MTP2->MTP3 Generic values */
	IN_SERVICE			=300,
	OUT_OF_SERVICE			=301,
	REMOTE_PROCESSOR_OUTAGE		=302,
	REMOTE_PROCESSOR_RECOVERED	=303,
	RTB_CLEARED			=304,
	RX_MESSAGE			=305,
	RX_BUF_CLEARED			=306,
	BSNT				=307,
	RETRIEVED_MESSAGES		=308,
	RETRIEVAL_COMPLETE		=309,
	LINK_CONGESTION_ONSET_TX	=310,
	LINK_CONGESTION_ONSET_RX	=311,
	LINK_CONGESTION_CEASE_TX	=312,
	LINK_CONGESTION_CEASE_RX	=313,
	LINK_CONNECTED			=314,
	LINK_NOT_CONNECTED		=315,
};

enum	e_mtp2_lsc_causes
{
	LSC_CAUSE_POWER_ON = 0,
	LSC_CAUSE_START = 1,
	LSC_CAUSE_STOP = 2,
	LSC_CAUSE_RETRIEVE_BSNT = 3,
	LSC_CAUSE_RETRIEVAL_REQUEST_AND_FSNC = 4,
	LSC_CAUSE_EMERGENCY = 5,
	LSC_CAUSE_EMERGENCY_CEASES = 6,
	LSC_CAUSE_LOCAL_PROCESSOR_OUTAGE = 7,
	LSC_CAUSE_RESUME = 8,
	LSC_CAUSE_CLEAR_BUFFERS = 9,
	LSC_CAUSE_ALIGNMENT_COMPLETE = 10,
	LSC_CAUSE_ALIGNMENT_NOT_POSSIBLE = 11,
	LSC_CAUSE_LINK_FAILURE = 12,
	LSC_CAUSE_FISU_MSU_RECEIVED = 13,
	LSC_CAUSE_SIPO = 14,
	LSC_CAUSE_SIO = 15,
	LSC_CAUSE_SIN = 16,
	LSC_CAUSE_SIE = 17,
	LSC_CAUSE_SIOS = 18,
	LSC_CAUSE_T1_EXPIRY = 19,
	LSC_CAUSE_CLEAR_RTB = 20,
	LSC_CAUSE_CLEAR_RTB_COMPLETE = 21,
};

typedef struct
{
	int32_t		ls;
	int32_t		link;
	uint32_t	total_octet_count;
	uint32_t	msu_octet_count;
} t_link_opm;


enum	e_mtp3_entities
{
	/* SMH */
	HMDC = 0 + 100,
	HMDT = 1 + 100,
	HMRT = 2 + 100,
	HMCG = 3 + 100,
	/* STM */
	TSFC = 4 + 100,
	TCRC = 5 + 100,
	TFRC = 6 + 100,
	TRCC = 7 + 100,
	TCBC = 8 + 100,
	TCOC = 9 + 100,
	TPRC = 10 + 100,
	TSRC = 11 + 100,
	TLAC = 12 + 100,
	/* SLM */
	LLSC = 13 + 100,
	LSAC = 14 + 100,
#if 0
	LSLA = 15 + 100, /* not used in this implementation */
	LSLR = 16 + 100, /* not used in this implementation */
	LSLD = 17 + 100, /* not used in this implementation */
	LSDA = 18 + 100, /* not used in this implementation */
	LSTA = 19 + 100, /* not used in this implementation */
#endif
	/* SRM */
	RTRC = 20 + 100,
	RTPC = 21 + 100,
	RSRT = 22 + 100,
	RTAC = 23 + 100,
	/* SLTC */
	SLTC = 24 + 100,
	/* Level 4 */
	LVL4 = 25 + 100,
	/* MTP3 timer facility */
	MTP3_TIMER = 26 + 100,
	/* Crosslink */
	CROSSLINK = 27 + 100,
};


typedef struct
{
	int	fi;
	int	si;
} mtp2link_info_t;


/*=================================================================
 * Prototypes
 *===============================================================*/

void	info_u (
		const char * object,
		const char * location, 
		int	group,
		const char * cause, 
		const unsigned int detail
		);
void xmtp2_link_util_report (
		const unsigned int ls,
		const unsigned int link,
		const unsigned int msu_octet_count,
		const unsigned int total_octet_count
		);
int xmtp2_init(void);
int xmtp2_load(void);
int xmtp2_open (void);
int xmtp2_conf_link (int fd, xmtp_cfg_link_info_t *cfg);
int xmtp2_stop_link (int fd, xmtp_cfg_link_info_t *cfg);

int xmtp2_cmd (
	int xmtp2km_fd,
	const unsigned int linkset, 
	const unsigned int link, 
	const unsigned int cause, 
	const unsigned int caller);
int xmtp2_power_on_links (int fd);
int xmtp2_sangoma_start (int fi);
int xmtp2_start_facilities (void);
void xmtp2_sangoma_stop (int fi);
void xmtp2_stop_facilities (void);
int xmtp2_start_facility (int card, int slot);
int xmtp2_restart_facility (int card, int slot);

int xmtp2_report_link_load (int xmtp2km_fd, t_link_opm *opm_table);

int xmtp2_retrieve_bsnt (int xmtp2km_fd, const unsigned int linkset, 
		       const unsigned int link, const unsigned int caller); 
int xmtp2_send_msu (int xmtp2km_fd, t_su_buf * p_msu); 
int xmtp2_read_msu(int xmtp2km_fd,  t_su_buf * p_msu);
void xmtp2_print_msu (t_su_buf * p_msu2);
int xmtp2_unload(void);
int xmtp2_close(int fd);

#endif

