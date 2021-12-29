#ifndef _WANPIPE_CFG_H_
#define _WANPIPE_CFG_H_

#if defined (__WINDOWS__)
 #if defined(WAN_KERNEL)
  #include <array_queue.h>
 #endif
 #include <stdio.h>
 #include <sdla_te1.h>
 #include <sdla_56k.h>
 #include <sdla_te3.h>
 #include <sdla_remora.h>
 #include <sdla_front_end.h>

#endif

/* DSL interface types */
#define WAN_INTERFACE 	0
#define LAN_INTERFACE	1


/* Defines for UDP PACKET TYPE */
#define UDP_PTPIPE_TYPE 	0x01
#define UDP_FPIPE_TYPE		0x02
#define UDP_CPIPE_TYPE		0x03
#define UDP_DRVSTATS_TYPE 	0x04
#define UDP_INVALID_TYPE  	0x05

#define UDPMGMT_UDP_PROTOCOL	0x11

/* Command return code */
#define WAN_CMD_OK		0	/* normal firmware return code */
#define WAN_CMD_TIMEOUT		0xFF	/* firmware command timed out */

/* UDP Packet Management */
#define UDP_PKT_FRM_STACK	0x00
#define UDP_PKT_FRM_NETWORK	0x01

#define WAN_UDP_FAILED_CMD  	0xCF
#define WAN_UDP_INVALID_CMD 	0xCE 
#define WAN_UDP_TIMEOUT_CMD 	0xAA 
#define WAN_UDP_INVALID_NET_CMD     0xCD

/* Maximum interrupt test counter */
#define MAX_INTR_TEST_COUNTER	100
#define MAX_NEW_INTR_TEST_COUNTER	5

/* Critical Values for RACE conditions*/
#define CRITICAL_IN_ISR		0xA1
#define CRITICAL_INTR_HANDLED	0xB1

/* Card Types */
#define WANOPT_S50X			1
#define WANOPT_S51X			2
#define WANOPT_ADSL			3
#define WANOPT_AFT			4
#define WANOPT_AFT104		5
#define WANOPT_AFT300		6
#define WANOPT_AFT_ANALOG	7
#define WANOPT_AFT108		8
#define WANOPT_AFT_X		9
#define WANOPT_AFT102		10

#if defined (__WINDOWS__)

/* Interface Operation Modes */
enum {
	WANPIPE,
	API,
	BRIDGE,
	BRIDGE_NODE,
	SWITCH,
	STACK,
	ANNEXG,
	TTY,
	TDM_VOICE,
	TDM_VOICE_DCHAN,//for applications like FreeSwitch
	TDM_VOICE_API,	//for applications like YATE
	LIB_SANGOMA_API	//for applications like FreeSwitch
};

//DavidR: added this macro
#define SDLA_DECODE_USEDBY_FIELD(usedby)				\
		(usedby == WANPIPE)		?	"WANPIPE" :			\
		(usedby == API)			?	"API" :				\
		(usedby == BRIDGE)		?	"BRIDGE" :			\
		(usedby == BRIDGE_NODE)	?	"BRIDGE_NODE" :		\
		(usedby == SWITCH)		?	"SWITCH" :			\
		(usedby == STACK)		?	"STACK" :			\
		(usedby == TDM_VOICE_API)?	"TDM VOICE API" :	\
		(usedby == TDM_VOICE_DCHAN)?"TDM VOICE DCHAN" :	\
		(usedby == LIB_SANGOMA_API)?"LIB SANGOMA API" :	\
		(usedby == ANNEXG)		?	"ANNEXG" :	"Unknown"

#define STATE_DECODE(state)					\
		((state == WAN_UNCONFIGURED) ? "Unconfigured" :	\
		 (state == WAN_DISCONNECTED) ? "Disconnected" : \
         (state == WAN_CONNECTING) ? "Connecting" : 	\
		 (state == WAN_CONNECTED) ? "Connected": 	\
		 (state == WAN_LIMIT) ? "Limit": 		\
		 (state == WAN_DUALPORT) ? "DualPort": 		\
		 (state == WAN_DISCONNECTING) ? "Disconnecting": \
		 (state == WAN_FT1_READY) ? "FT1 Ready": "Invalid")

#define CARD_WANOPT_DECODE(cardtype)				\
		((cardtype == WANOPT_S50X) ? "WANOPT_S50X" :	\
		 (cardtype == WANOPT_S51X) ? "WANOPT_S51X" :	\
	     (cardtype == WANOPT_ADSL) ? "WANOPT_ADSL" : 	\
		 (cardtype == WANOPT_AFT)  ? "WANOPT_AFT": 	\
		 (cardtype == WANOPT_AFT104) ? "WANOPT_AFT104": 		\
		 (cardtype == WANOPT_AFT108) ? "WANOPT_AFT108": 		\
		 (cardtype == WANOPT_AFT_ANALOG) ? "WANOPT_AFT_ANALOG": 		\
		 (cardtype == WANOPT_AFT300) ? "WANOPT_AFT300": "Invalid card")

#endif//if defined (__WINDOWS__)

/*
 * Configuration options defines.
 */
/* general options */
#define	WANOPT_OFF	0
#define	WANOPT_ON	1
#define	WANOPT_NO	0
#define	WANOPT_YES	1

/* intercace options */
#define	WANOPT_RS232	0
#define	WANOPT_V35	1

/* data encoding options */
#define	WANOPT_NRZ	0
#define	WANOPT_NRZI	1
#define	WANOPT_FM0	2
#define	WANOPT_FM1	3

/* line idle option */
#define WANOPT_IDLE_FLAG 0
#define WANOPT_IDLE_MARK 1

/* link type options */
#define	WANOPT_POINTTOPOINT	0	/* RTS always active */
#define	WANOPT_MULTIDROP	1	/* RTS is active when transmitting */

/* clocking options */
#define	WANOPT_EXTERNAL	0
#define	WANOPT_INTERNAL	1

/* station options */
#define	WANOPT_DTE		0
#define	WANOPT_DCE		1
#define	WANOPT_CPE		0
#define	WANOPT_NODE		1
#define	WANOPT_SECONDARY	0
#define	WANOPT_PRIMARY		1

/* connection options */
#define	WANOPT_PERMANENT	0	/* DTR always active */
#define	WANOPT_SWITCHED		1	/* use DTR to setup link (dial-up) */
#define	WANOPT_ONDEMAND		2	/* activate DTR only before sending */

/* frame relay in-channel signalling */
#define WANOPT_FR_AUTO_SIG	0	/* Automatically find singalling */
#define	WANOPT_FR_ANSI		1	/* ANSI T1.617 Annex D */
#define	WANOPT_FR_Q933		2	/* ITU Q.933A */
#define	WANOPT_FR_LMI		3	/* LMI */

/* PPP IP Mode Options */
#define	WANOPT_PPP_STATIC	0
#define	WANOPT_PPP_HOST		1
#define	WANOPT_PPP_PEER		2

/* ASY Mode Options */
#define WANOPT_ONE 		1
#define WANOPT_TWO		2
#define WANOPT_ONE_AND_HALF	3

#define WANOPT_NONE	0
#define WANOPT_ODD      1
#define WANOPT_EVEN	2

/* ATM sync options */
#define WANOPT_AUTO	0
#define WANOPT_MANUAL	1

#define WANOPT_DSP_HPAD	0
#define WANOPT_DSP_TPAD	1

/* CHDLC Protocol Options */
/* DF Commmented out for now.

#define WANOPT_CHDLC_NO_DCD		IGNORE_DCD_FOR_LINK_STAT
#define WANOPT_CHDLC_NO_CTS		IGNORE_CTS_FOR_LINK_STAT
#define WANOPT_CHDLC_NO_KEEPALIVE	IGNORE_KPALV_FOR_LINK_STAT
*/

/* SS7 options */
#define WANOPT_SS7_ANSI		1
#define WANOPT_SS7_ITU		2	
#define WANOPT_SS7_NTT		3	

#define WANOPT_SS7_FISU 0
#define WANOPT_SS7_LSSU 1

#define WANOPT_SS7_MODE_128 	0
#define WANOPT_SS7_MODE_4096	1

#define WANOPT_SS7_FISU_128_SZ  3
#define WANOPT_SS7_FISU_4096_SZ 6


/* Port options */
#define WANOPT_PRI 0
#define WANOPT_SEC 1
/* read mode */
#define	WANOPT_INTR	0
#define WANOPT_POLL	1


#define WANOPT_TTY_SYNC  0
#define WANOPT_TTY_ASYNC 1

/* RBS Signalling Options */
#define WAN_RBS_SIG_A	0x01
#define WAN_RBS_SIG_B	0x02
#define WAN_RBS_SIG_C	0x04
#define WAN_RBS_SIG_D	0x08

/* Front End Ref Clock Options */
#define WANOPT_FE_OSC_CLOCK 	0x00
#define WANOPT_FE_LINE_CLOCK 	0x01

enum wan_codec_format{
	WP_NONE,
	WP_SLINEAR
};

/* POS protocols */
enum {
	IBM4680,
	NCR2127,
	NCR2126,
	NCR1255,
	NCR7000,
	ICL
};
/* 'state' defines */
enum wan_states
{
	WAN_UNCONFIGURED,	/* link/channel is not configured */
	WAN_DISCONNECTED,	/* link/channel is disconnected */
	WAN_CONNECTING,		/* connection is in progress */
	WAN_CONNECTED,		/* link/channel is operational */
	WAN_LIMIT,			/* for verification only */
	WAN_DUALPORT,		/* for Dual Port cards */
	WAN_DISCONNECTING,
	WAN_FT1_READY		/* FT1 Configurator Ready */
};

enum {
	WAN_LOCAL_IP,
	WAN_POINTOPOINT_IP,
	WAN_NETMASK_IP,
	WAN_BROADCAST_IP
};


/*      Standard Mode                          */
enum {
        WANOPT_ADSL_T1_413      = 0,
        WANOPT_ADSL_G_LITE      = 1,
        WANOPT_ADSL_G_DMT       = 2,
        WANOPT_ADSL_ALCATEL_1_4 = 3,
        WANOPT_ADSL_MULTIMODE   = 4,
        WANOPT_ADSL_ADI         = 5,
        WANOPT_ADSL_ALCATEL     = 6,
        WANOPT_ADSL_T1_413_AUTO = 9
};

/*      Trellis  Modes                         */
#define WANOPT_ADSL_TRELLIS_DISABLE             0x0000
#define WANOPT_ADSL_TRELLIS_ENABLE              0x8000

#define WANOPT_ADSL_TRELLIS_LITE_ONLY_DISABLE   0xF000

#define WANOPT_ADSL_0DB_CODING_GAIN             0x0000
#define WANOPT_ADSL_1DB_CODING_GAIN             0x1000
#define WANOPT_ADSL_2DB_CODING_GAIN             0x2000
#define WANOPT_ADSL_3DB_CODING_GAIN             0x3000
#define WANOPT_ADSL_4DB_CODING_GAIN             0x4000
#define WANOPT_ADSL_5DB_CODING_GAIN             0x5000
#define WANOPT_ADSL_6DB_CODING_GAIN             0x6000
#define WANOPT_ADSL_7DB_CODING_GAIN             0x7000
#define WANOPT_ADSL_AUTO_CODING_GAIN            0xFF00

#define WANOPT_ADSL_RX_BIN_ENABLE               0x01
#define WANOPT_ADSL_RX_BIN_DISABLE              0x00


#define WANOPT_ADSL_FRAMING_TYPE_0              0x0000
#define WANOPT_ADSL_FRAMING_TYPE_1              0x0001
#define WANOPT_ADSL_FRAMING_TYPE_2              0x0002
#define WANOPT_ADSL_FRAMING_TYPE_3              0x0003

#define WANOPT_ADSL_EXPANDED_EXCHANGE           0x8000
#define WANOPT_ADSL_SHORT_EXCHANGE              0x0000

#define WANOPT_ADSL_CLOCK_OSCILLATOR            0x00
#define WANOPT_ADSL_CLOCK_CRYSTAL               0x04
 

#define SDLA_DECODE_CARDTYPE(card_type)				\
		(card_type == WANOPT_S50X) ? "S50X" :		\
		(card_type == WANOPT_S51X) ? "S51X" :		\
		(card_type == WANOPT_ADSL) ? "ADSL" :		\
		(card_type == WANOPT_AFT)  ? "A101/2" :		\
		(card_type == WANOPT_AFT104) ? "A104" :		\
		(card_type == WANOPT_AFT108) ? "A108" :		\
		(card_type == WANOPT_AFT300) ?  "A300"  :	\
		(card_type == WANOPT_AFT_ANALOG) ? "A200" : "Unknown"

#define DECODE_CARD_SUBTYPE(card_sub_type)									\
		(card_sub_type == A101_1TE1_SUBSYS_VENDOR)			? "A101" :		\
		(card_sub_type == AFT_1TE1_SHARK_SUBSYS_VENDOR)	? "A101D" :		\
		(card_sub_type == A101_2TE1_SUBSYS_VENDOR)			? "A102" :		\
		(card_sub_type == AFT_2TE1_SHARK_SUBSYS_VENDOR)		? "A102D" :		\
		(card_sub_type == A104_4TE1_SUBSYS_VENDOR)			? "A104" :		\
		(card_sub_type == AFT_4TE1_SHARK_SUBSYS_VENDOR)		? "A104D" :		\
		(card_sub_type == AFT_8TE1_SHARK_SUBSYS_VENDOR)		? "A108D"  :	\
		(card_sub_type == A200_REMORA_SHARK_SUBSYS_VENDOR)	? "A200"  : "Unknown"

#define COMPORT_DECODE(port)	(port == WANOPT_PRI) ? "PRI" : "SEC"

typedef enum {
    RFC_MODE_BRIDGED_ETH_LLC    = 0,
    RFC_MODE_BRIDGED_ETH_VC     = 1,
    RFC_MODE_ROUTED_IP_LLC      = 2,
    RFC_MODE_ROUTED_IP_VC       = 3,
    RFC_MODE_RFC1577_ENCAP      = 4,
    RFC_MODE_PPP_LLC 	    	= 5,
    RFC_MODE_PPP_VC		= 6
} RFC_MODE;

typedef struct wan_adsl_vcivpi
{
	unsigned short	vci;
	unsigned char	vpi;
} wan_adsl_vcivpi_t;


typedef struct wan_adsl_conf
{
#if 1
	unsigned char     EncapMode;
	unsigned short    Vci;
	unsigned short    Vpi;
#else
	unsigned char     interface;
	unsigned char     Rfc1483Mode;
	unsigned short    Rfc1483Vci;
	unsigned short    Rfc1483Vpi;
	unsigned char     Rfc2364Mode;
	unsigned short    Rfc2364Vci;
	unsigned short    Rfc2364Vpi;
#endif
	unsigned char     Verbose; 
	unsigned short    RxBufferCount;
	unsigned short    TxBufferCount;

	unsigned short    Standard;
	unsigned short    Trellis;
	unsigned short    TxPowerAtten;
	unsigned short    CodingGain;
	unsigned short    MaxBitsPerBin;
	unsigned short    TxStartBin;
	unsigned short    TxEndBin;
	unsigned short    RxStartBin;
	unsigned short    RxEndBin;
	unsigned short    RxBinAdjust;
	unsigned short    FramingStruct;
	unsigned short    ExpandedExchange;
	unsigned short    ClockType;
	unsigned short    MaxDownRate;

	unsigned char	  atm_autocfg;
	unsigned short	  vcivpi_num;	
	wan_adsl_vcivpi_t vcivpi_list[100];	
	unsigned char	  tty_minor;
	unsigned short	  mtu;

	unsigned char	  atm_watchdog;
}wan_adsl_conf_t;


typedef struct wan_atm_conf
{
	unsigned char	atm_sync_mode;
	unsigned short	atm_sync_data;
	unsigned char	atm_sync_offset;
	unsigned short  atm_hunt_timer;

	unsigned char	atm_cell_cfg;
	unsigned char	atm_cell_pt;
	unsigned char	atm_cell_clp;
	unsigned char	atm_cell_payload;
}wan_atm_conf_t;


typedef struct wan_atm_conf_if
{
	unsigned char     encap_mode;
	unsigned short    vci;
	unsigned short    vpi;

	unsigned char	  atm_oam_loopback;
	unsigned char	  atm_oam_loopback_intr;
	unsigned char	  atm_oam_continuity;
	unsigned char	  atm_oam_continuity_intr;
	unsigned char	  atm_arp;
	unsigned char	  atm_arp_intr;

}wan_atm_conf_if_t;

/*----------------------------------------------------------------------------
 * X.25-specific link-level configuration.
 */
typedef struct wan_x25_conf
{
	unsigned lo_pvc;	/* lowest permanent circuit number */
	unsigned hi_pvc;	/* highest permanent circuit number */
	unsigned lo_svc;	/* lowest switched circuit number */
	unsigned hi_svc;	/* highest switched circuit number */
	unsigned hdlc_window;	/* HDLC window size (1..7) */
	unsigned pkt_window;	/* X.25 packet window size (1..7) */
	unsigned t1;		/* HDLC timer T1, sec (1..30) */
	unsigned t2;		/* HDLC timer T2, sec (0..29) */
	unsigned t4;		/* HDLC supervisory frame timer = T4 * T1 */
	unsigned n2;		/* HDLC retransmission limit (1..30) */
	unsigned t10_t20;	/* X.25 RESTART timeout, sec (1..255) */
	unsigned t11_t21;	/* X.25 CALL timeout, sec (1..255) */
	unsigned t12_t22;	/* X.25 RESET timeout, sec (1..255) */
	unsigned t13_t23;	/* X.25 CLEAR timeout, sec (1..255) */
	unsigned t16_t26;	/* X.25 INTERRUPT timeout, sec (1..255) */
	unsigned t28;		/* X.25 REGISTRATION timeout, sec (1..255) */
	unsigned r10_r20;	/* RESTART retransmission limit (0..250) */
	unsigned r12_r22;	/* RESET retransmission limit (0..250) */
	unsigned r13_r23;	/* CLEAR retransmission limit (0..250) */
	unsigned ccitt_compat;	/* compatibility mode: 1988/1984/1980 */
	unsigned x25_conf_opt;   /* User defined x25 config optoins */
	unsigned char LAPB_hdlc_only; /* Run in HDLC only mode */
	unsigned char logging;   /* Control connection logging */  
	unsigned char oob_on_modem; /* Whether to send modem status to the user app */
	unsigned char local_station_address;  /*Local Station address */
	unsigned short defPktSize;
	unsigned short pktMTU;
	unsigned char cmd_retry_timeout; /* Value is seconds */
} wan_x25_conf_t;

/*----------------------------------------------------------------------------
 * Frame relay specific link-level configuration.
 */
typedef struct wan_fr_conf
{
	unsigned signalling;	/* local in-channel signalling type */
	unsigned t391;		/* link integrity verification timer */
	unsigned t392;		/* polling verification timer */
	unsigned n391;		/* full status polling cycle counter */
	unsigned n392;		/* error threshold counter */
	unsigned n393;		/* monitored events counter */
	unsigned dlci_num;	/* number of DLCs (access node) */

	unsigned  dlci[MAX_NUMBER_OF_PROTOCOL_INTERFACES];    /* List of all DLCIs */
	unsigned char issue_fs_on_startup;

	//Windows
	unsigned char auto_dlci;//1 - yes, 0 - no
    unsigned char station;	//Node/CPE
} wan_fr_conf_t;


typedef struct wan_xilinx_conf
{
#if defined(__WINDOWS__)
	unsigned short num_of_ch;	// Number of logical channels
#endif
	unsigned short dma_per_ch; 	/* DMA buffers per logic channel */
	unsigned short mru;		/* MRU of transparent channels */
	unsigned char  rbs;		/* Robbit signalling support */

	unsigned int   rx_crc_bytes;
	unsigned int   data_mux_map;	/* Data mux map */
	unsigned int   fe_ref_clock;	/* Front End Reference Clock */
	unsigned int   tdmv_span_no;
	unsigned int   tdmv_dchan;	/* hwHDLC: PRI SIG */

	unsigned char  tdmv_hwec;	/* Congiure HW EC */
	unsigned int   ec_clk_src;	/* Octasic Clock Source Port */
}wan_xilinx_conf_t;


//per-logic chan cfg
typedef struct wan_xilinx_conf_if
{
	unsigned char 	station;   	/* Node or CPE */
	unsigned int  	signalling; 	/* local in-channel signalling type */
	unsigned char 	seven_bit_hdlc;
	unsigned int  	mru;	// MRU of transparent channels
	unsigned int  	mtu;	// MTU of a logic channel
	unsigned char  	idle_flag;
	//A104 additions
	unsigned char  	data_mux;
	unsigned char	ss7_enable;
	unsigned char 	ss7_mode;
	unsigned char	ss7_lssu_size;
	unsigned char	tdmv_master_if;
	unsigned char   tdmv_hwec;	/* Enable/Disable HW EC */
	unsigned char   rbs_cas_idle;	/* Initial RBS/CAS value */
	unsigned char   dtmf_hw;	/* Enable/Disable HW DTMF */
/*	unsigned char   tdmv_hwec_map[50];*/	/* Enable/Disable HW EC */
}wan_xilinx_conf_if_t;


typedef struct wan_ss7_conf
{
	unsigned int line_cfg_opt;
	unsigned int modem_cfg_opt;
	unsigned int modem_status_timer;
	unsigned int api_options;
	unsigned int protocol_options;
	unsigned int protocol_specification;
	unsigned int stats_history_options;
	unsigned int max_length_msu_sif;
	unsigned int max_unacked_tx_msus;
	unsigned int link_inactivity_timer;
	unsigned int t1_timer;
	unsigned int t2_timer;
	unsigned int t3_timer;
	unsigned int t4_timer_emergency;
	unsigned int t4_timer_normal;
	unsigned int t5_timer;
	unsigned int t6_timer;
	unsigned int t7_timer;
	unsigned int t8_timer;
	unsigned int n1;
	unsigned int n2;
	unsigned int tin;
	unsigned int tie;
	unsigned int suerm_error_threshold;
	unsigned int suerm_number_octets;
	unsigned int suerm_number_sus;
	unsigned int sie_interval_timer;
	unsigned int sio_interval_timer;
	unsigned int sios_interval_timer;
	unsigned int fisu_interval_timer;
} wan_ss7_conf_t;

#define WANOPT_TWO_WAY_SIMULTANEOUS	1

#define WANOPT_PRI_DISC_ON_NO_RESP	0
#define WANOPT_PRI_SNRM_ON_NO_RESP	1

typedef struct wan_xdlc_conf {

	unsigned char  station;		
	unsigned int   address;
	unsigned int   max_I_field_length;
	
	unsigned int protocol_config;
	unsigned int error_response_config;
	unsigned int TWS_max_ack_count;	
	
	unsigned int pri_slow_poll_timer;
	unsigned int pri_normal_poll_timer;
	unsigned int pri_frame_response_timer;
	unsigned int sec_nrm_timer;
	
	unsigned int max_no_response_cnt;
	unsigned int max_frame_retransmit_cnt;
	unsigned int max_rnr_cnt;
	
	unsigned int window;
	
}wan_xdlc_conf_t;


typedef struct wan_sdlc_conf {
	
	char station_configuration;		
	unsigned long baud_rate;
	unsigned max_I_field_length;
	unsigned general_operational_config_bits;
	unsigned protocol_config_bits;
	unsigned exception_condition_reporting_config;
	unsigned modem_config_bits;
	unsigned statistics_format;
	unsigned pri_station_slow_poll_interval;
	unsigned permitted_sec_station_response_TO;
	unsigned no_consec_sec_TOs_in_NRM_before_SNRM_issued;
	unsigned max_lgth_I_fld_pri_XID_frame;		
	unsigned opening_flag_bit_delay_count;		
	unsigned RTS_bit_delay_count;
	unsigned CTS_timeout_1000ths_sec;	
	char SDLA_configuration;			
	
}wan_sdlc_conf_t;

typedef struct wan_bscstrm_conf {
   unsigned long baud_rate;                 /* the baud rate */
   unsigned long adapter_frequency;         /* the adapter frequecy */
   unsigned short max_data_length;          /* the maximum length of a BSC data block */
   unsigned short EBCDIC_encoding;          /* EBCDIC/ASCII encoding */
   unsigned short Rx_block_type;            /* the type of BSC block to be received */
   unsigned short no_consec_PADs_EOB;       /* the number of consecutive PADs indicating the end of the block */
   unsigned short no_add_lead_Tx_SYN_chars; /* the number of additional leading transmit SYN characters */
   unsigned short no_bits_per_char;         /* the number of bits per character */
   unsigned short parity;                   /* parity */
   unsigned short misc_config_options;      /* miscellaneous configuration options */
   unsigned short statistics_options;       /* statistic options */
   unsigned short modem_config_options;     /* modem configuration options */
}wan_bscstrm_conf_t;


/*----------------------------------------------------------------------------
 * PPP-specific link-level configuration.
 */
typedef struct wan_ppp_conf
{
	unsigned restart_tmr;	/* restart timer */
	unsigned auth_rsrt_tmr;	/* authentication timer */
	unsigned auth_wait_tmr;	/* authentication timer */
	unsigned mdm_fail_tmr;	/* modem failure timer */
	unsigned dtr_drop_tmr;	/* DTR drop timer */
	unsigned connect_tmout;	/* connection timeout */
	unsigned conf_retry;	/* max. retry */
	unsigned term_retry;	/* max. retry */
	unsigned fail_retry;	/* max. retry */
	unsigned auth_retry;	/* max. retry */
	unsigned auth_options;	/* authentication opt. */
	unsigned ip_options;	/* IP options */
	char	authenticator;	/* AUTHENTICATOR or not */
	char	ip_mode;	/* Static/Host/Peer */
} wan_ppp_conf_t;

typedef struct sppp_parms_struct {

	unsigned char dynamic_ip;
	unsigned int  local_ip;
	unsigned int  remote_ip;
	
	unsigned int  pp_auth_timer;

 	unsigned int  sppp_keepalive_timer;
	unsigned int  pp_timer;

	unsigned char pap;
	unsigned char chap;
	unsigned char userid[WAN_AUTHNAMELEN];	
	unsigned char passwd[WAN_AUTHNAMELEN];	

#if defined(__WINDOWS__)
	unsigned char sysname[WAN_AUTHNAMELEN];	
	/////////////////////////////////////////////////////////////
	//added it because in original code it is hardcoded
	unsigned int sppp_max_keepalive_count;
	/////////////////////////////////////////////////////////////
#endif
	unsigned int  gateway;
	unsigned char ppp_prot;

}wan_sppp_if_conf_t;

/*----------------------------------------------------------------------------
 * CHDLC-specific link-level configuration.
 */
typedef struct wan_chdlc_conf
{
	unsigned char ignore_dcd;	/* Protocol options:		*/
	unsigned char ignore_cts;	/* Ignore these to determine	*/
	unsigned char ignore_keepalive;	/* link status (Yes or No)	*/
	unsigned char hdlc_streaming;	/* hdlc_streaming mode (Y/N) */
	unsigned char receive_only;	/* no transmit buffering (Y/N) */
	unsigned keepalive_tx_tmr;	/* transmit keepalive timer */
	unsigned keepalive_rx_tmr;	/* receive  keepalive timer */
	unsigned keepalive_err_margin;	/* keepalive_error_tolerance */
	unsigned slarp_timer;		/* SLARP request timer */
	unsigned char fast_isr;		/* Fast interrupt option */
} wan_chdlc_conf_t;

#define X25_CALL_STR_SZ 512
#define WAN_IF_LABEL_SZ 15

typedef struct lapb_parms_struct {
	unsigned int t1;
	unsigned int t1timer;
	unsigned int t2;
	unsigned int t2timer;
	unsigned int n2;
	unsigned int n2count;
	unsigned int t3;
	unsigned int t4;
	unsigned int window;
	unsigned int state;
	unsigned int mode;
	unsigned int mtu;
	unsigned char label[WAN_IF_LABEL_SZ+1];
	unsigned char virtual_addr[WAN_ADDRESS_SZ+1];
	unsigned char real_addr[WAN_ADDRESS_SZ+1];
}wan_lapb_if_conf_t;


typedef struct x25_parms_struct {

	unsigned short X25_API_options;
	unsigned short X25_protocol_options;
	unsigned short X25_response_options;
	unsigned short X25_statistics_options;
	unsigned short packet_window;
	unsigned short default_packet_size;
	unsigned short maximum_packet_size; 
	unsigned short lowest_PVC;
	unsigned short highest_PVC;
	unsigned short lowest_incoming_channel;	
	unsigned short highest_incoming_channel;
	unsigned short lowest_two_way_channel;	
	unsigned short highest_two_way_channel;
	unsigned short lowest_outgoing_channel;		
	unsigned short highest_outgoing_channel;		
	unsigned short genl_facilities_supported_1;	
	unsigned short genl_facilities_supported_2;	
	unsigned short CCITT_facilities_supported;	
	unsigned short non_X25_facilities_supported;	
	unsigned short CCITT_compatibility;				
	unsigned short T10_T20;
	unsigned short T11_T21;
	unsigned short T12_T22;
	unsigned short T13_T23;	
	unsigned short T16_T26;	
	unsigned short T28;
	unsigned short R10_R20;		
	unsigned short R12_R22;			
	unsigned short R13_R23;			
	unsigned char  dte;
	unsigned char  mode;

	unsigned char call_string   [X25_CALL_STR_SZ+1];	
	
	/* Accept Call Information */
	unsigned char accept_called [WAN_ADDRESS_SZ+1];
	unsigned char accept_calling[WAN_ADDRESS_SZ+1];
	unsigned char accept_facil  [WAN_ADDRESS_SZ+1];
	unsigned char accept_udata  [WAN_ADDRESS_SZ+1];

	unsigned char label [WAN_IF_LABEL_SZ+1];

	unsigned int  call_backoff_timeout;
	unsigned char call_logging;
	
	/* X25_SW X.25 switching */
	unsigned char virtual_addr[WAN_ADDRESS_SZ+1];
	unsigned char real_addr[WAN_ADDRESS_SZ+1];

	unsigned char addr[WAN_ADDRESS_SZ+1];	/* PVC LCN number */

} wan_x25_if_conf_t;

typedef struct dsp_parms {
	
	unsigned char	pad_type;	/* PAD type: HOST or TERM */
	unsigned int T1;			/* Service Timeout perioud */
	unsigned int T2;			/* PAD protocol timeout period */
	unsigned int T3;			/* Timeout between two packets
								* of the same M_DATA */
	unsigned char auto_ce;		/* Automaticaly send Circuit Enabled 
								* with ACCEPT CALL packet */
	unsigned char auto_call_req;	/* Automaticaly re-send CALL REQUEST 
									* if it was failed before (T1) */
	unsigned char auto_ack;		/* Automaticaly send ACK for data */

	unsigned short dsp_mtu;		/* MTU size for DSP level */
	
}wan_dsp_if_conf_t;
	
/*----------------------------------------------------------------------------
 * WAN device configuration. Passed to ROUTER_SETUP IOCTL.
 */
typedef struct wandev_conf
{
	unsigned magic;		/* magic number (for verification) */
	unsigned config_id;	/* configuration structure identifier */

	/****** hardware configuration ******/

	/* ISA only hardware settings */
	unsigned ioport;	/* adapter I/O port base */
	unsigned long maddr;/* dual-port memory address */
	unsigned msize;		/* dual-port memory size */
	/* end of ISA hardware settings */
	
	int irq;		/* interrupt request level */
	int dma;		/* DMA request level */
	char S514_CPU_no[1];	/* S514 PCI adapter CPU number ('A' or 'B') */
    unsigned PCI_slot_no;	/* S514 PCI adapter slot number */
	char auto_pci_cfg;	/* S515 PCI automatic slot detection */
	char comm_port;		/* Communication Port (PRI=0, SEC=1) */ 
	unsigned bps;		/* data transfer rate */
	unsigned mtu;		/* maximum transmit unit size */
    unsigned udp_port;      /* UDP port for management */
	unsigned char ttl;	/* Time To Live for UDP security */
	unsigned char ft1;	/* FT1 Configurator Option */
    char electrical_interface;	/* RS-232/V.35, etc. */
	char clocking;		/* external/internal */
	char line_coding;	/* NRZ/NRZI/FM0/FM1, etc. */
	char station;		/* DTE/DCE, primary/secondary, etc. */
	char connection;	/* permanent/switched/on-demand */
	char read_mode;		/* read mode: Polling or interrupt */
	char receive_only;	/* disable tx buffers */
	char tty;		/* Create a fake tty device */
	unsigned tty_major;	/* Major number for wanpipe tty device */
	unsigned tty_minor; 	/* Minor number for wanpipe tty device */
	unsigned tty_mode;	/* TTY operation mode SYNC or ASYNC */
	char backup;		/* Backup Mode */
	unsigned hw_opt[4];	/* other hardware options */
	unsigned reserved[4];

	/****** arbitrary data ***************/
	unsigned data_size;	/* data buffer size */
	void* data;		/* data buffer, e.g. firmware */
	union			/****** protocol-specific ************/
	{
		wan_x25_conf_t x25;	/* X.25 configuration */

		wan_fr_conf_t fr;	/* frame relay configuration */
		
		//wan_chdlc_conf_t chdlc;	/* Cisco HDLC configuration */
		//Windows - changed to be the same as in 'wanif_conf'
		wan_sppp_if_conf_t	chdlc;
		//wan_ppp_conf_t		ppp;	/* PPP configuration */
		wan_sppp_if_conf_t	ppp;

		wan_ss7_conf_t ss7;	/* SS7 Configuration */
		wan_sdlc_conf_t sdlc;	/* SDLC COnfiguration */
		wan_bscstrm_conf_t bscstrm; /* Bisync Streaming Configuration */
		wan_adsl_conf_t adsl;	/* ADSL Configuration */
		wan_atm_conf_t atm;
		//FIXME: remove 'xilinx', leave only aft
		//wan_xilinx_conf_t xilinx;
		wan_xilinx_conf_t aft;

		wan_xdlc_conf_t 	xdlc;
	} u;

	/* No new variables are allowed above */
	
	unsigned short card_type;/* Supported Sangoma Card type - S508 or S514 or XILINX...*/
	unsigned short S514_adapter_type;	//S5141, S5142, S5143...

	unsigned pci_bus_no;	/* S514 PCI bus number */

	sdla_fe_cfg_t	fe_cfg;	/* Front end configurations */

	unsigned char line_idle; /* IDLE FLAG/ IDLE MARK */

	/* Bitstreaming options */
	unsigned int  sync_options;
	unsigned char rx_sync_char;
	unsigned char monosync_tx_time_fill_char;
	unsigned int  max_length_tx_data_block;
	unsigned int  rx_complete_length;
	unsigned int  rx_complete_timer;

	unsigned char ignore_front_end_status;

	unsigned int  max_trace_queue;
	unsigned int  max_rx_queue;

} wandev_conf_t;

/* 'config_id' definitions */
#if defined(__WINDOWS__) && defined(__KERNEL__)

typedef struct _sdla_wanpipe_interface
{
	//pointer to the wanpipe interface, passed when calling 'receive_indicate()'
	void* wanpipe_if;
	//pointer to receive function, called by VirIfEnum.sys
	void (*receive_indicate)(void* wanpipe_if, TX_DATA_STRUCT* rx_data);

	//to indicate to network layer we've completed TX and we can TX again.
	//called by VirIfEnum.sys
	void (*tx_complete_indicate)(void* wanpipe_if);

	//pointer to 'netdevice_t', passed when calling 'transmit_indicate()'
	void* p_netdevice;
	//pointer to transmit function, called by cpipe.sys
	int (*transmit_indicate)(void* p_netdevice, TX_DATA_STRUCT* tx_data);

	//pointer to Management input routine of underlying layer
	int (*management_indicate)(	void* p_called_netdevice,
								void* wan_udp				//ptr to 'wan_udp_hdr_t'
								);

	//pointer to Management Complete routine. Called by underlying layer
	//to indicate completion of Management call.
	int (*management_complete)(	void* p_netdevice);

	//this name used by wanpipe.sys for passing it to wanpipemon,
	//when used on a Network interface.
	unsigned char	underlying_dev_name[WAN_IFNAME_SZ];

}sdla_wanpipe_interface_t;

#endif

#if defined(__WINDOWS__)
//these needed to compile protocols in the LIP layer:
#define CONFIG_PRODUCT_WANPIPE_FR
#define CONFIG_PRODUCT_WANPIPE_CHDLC
#define CONFIG_PRODUCT_WANPIPE_PPP

//needed to compile A200 code:
#define CONFIG_PRODUCT_WANPIPE_TDM_VOICE
#define CONFIG_WANPIPE_HWEC

//cpecial id - when driver installed, it is "not configured".
//user has to select the protocol.
#define WANCONFIG_NONE					10
//special id - for AFT firmware update
#define WANCONFIG_AFT_FIRMWARE_UPDATE	11
//to compile TDM API
#define AFT_TDM_API_SUPPORT

#endif

#define	WANCONFIG_X25		101	/* X.25 link */
#define	WANCONFIG_FR		102	/* frame relay link */
#define	WANCONFIG_PPP		103	/* synchronous PPP link */
#define WANCONFIG_CHDLC		104	/* Cisco HDLC Link */
#define WANCONFIG_BSC		105	/* BiSync Streaming */
#define WANCONFIG_HDLC		106	/* HDLC Support */
#define WANCONFIG_MPPP  	107	/* Multi Port PPP over RAW CHDLC */
#define WANCONFIG_MPROT 	WANCONFIG_MPPP	/* Multi Port Driver HDLC,PPP,CHDLC over RAW CHDLC */
#define WANCONFIG_BITSTRM 	108	/* Bit-Stream protocol */
#define WANCONFIG_EDUKIT 	109	/* Educational Kit support */
#define WANCONFIG_SS7	 	110	/* SS7 Support */
#define WANCONFIG_BSCSTRM 	111	/* Bisync Streaming Nasdaq */
#define WANCONFIG_MFR    	112	/* Mulit-Port Frame Relay over RAW HDLC */
#define WANCONFIG_ADSL	 	113	/* LLC Ethernet Support */
#define WANCONFIG_SDLC	 	114	/* SDLC Support */
#define WANCONFIG_ATM	 	115	/* ATM Supprot */
#define WANCONFIG_POS	 	116	/* POS Support */
#define WANCONFIG_AFT    	117	/* AFT Hardware Support */
#define WANCONFIG_DEBUG  	118	/* Real Time Debugging */
#define WANCONFIG_ADCCP	 	119	/* Special HDLC LAPB Driver */
#define WANCONFIG_MLINK_PPP 	120	/* Multi-Link PPP */
#define WANCONFIG_GENERIC   	121	/* WANPIPE Generic driver */
#define WANCONFIG_AFT_TE3   	122 	/* AFT TE3 Hardware Support */
#define WANCONFIG_MPCHDLC   	123 	/* Multi Port CHDLC */
#define WANCONFIG_AFT_TE1_SS7   124 	/* AFT TE1 SS7 Hardware Support */
#define WANCONFIG_LAPB		125	/* LIP LAPB Protocol Support */
#define WANCONFIG_XDLC		126	/* LIP XDLC Protocol Support */
#define WANCONFIG_TTY		127	/* LIP TTY Support */
#define WANCONFIG_AFT_TE1  	128	/* AFT Quad Hardware Support */
#define WANCONFIG_XMTP2    	129	/* LIP XMTP2 Protocol Support */
#define WANCONFIG_ASYHDLC	130	/* S514 ASY HDLC API Support */
#define WANCONFIG_AFT_ANALOG	132	/* AFT Analog Driver */

#if defined(__WINDOWS__)

//convert integer definition of a protocol to string
static char * get_protocol_string(int protocol)
{
#define MAX_PROT_NAME_LENGTH	256

#ifndef snprintf
#define snprintf _snprintf
#endif

	static char protocol_name[MAX_PROT_NAME_LENGTH];

	switch(protocol)
	{
	case WANCONFIG_X25:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "X25");
		break;

	case WANCONFIG_FR:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Frame Relay");
		break;

	case WANCONFIG_PPP:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "PPP");
		break;

	case WANCONFIG_CHDLC:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "CHDLC");
		break;

	case WANCONFIG_BSC:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "BiSync Streaming");
		break;

	case WANCONFIG_HDLC:
		//used with CHDLC firmware
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "HDLC Streaming");
		break;

	case WANCONFIG_MPPP://and WANCONFIG_MPROT too
		//snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Multi Port PPP");
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "PPP");
		break;

	case WANCONFIG_BITSTRM:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Bit Stream");
		break;

	case WANCONFIG_EDUKIT:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "WAN EduKit");
		break;

	case WANCONFIG_SS7:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "SS7");
		break;

	case WANCONFIG_BSCSTRM:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Bisync Streaming Nasdaq");
		break;

	case WANCONFIG_MFR:
		//snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Multi-Port Frame Relay");
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Frame Relay");
		break;

	case WANCONFIG_ADSL:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "LLC Ethernet (ADSL)");
		break;

	case WANCONFIG_SDLC:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "SDLC");
		break;

	case WANCONFIG_ATM:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "ATM");
		break;

	case WANCONFIG_POS:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Point-of-Sale");
		break;

	case WANCONFIG_AFT:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "AFT");
		break;

	case WANCONFIG_AFT_TE3:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "AFT");
		break;

	case WANCONFIG_DEBUG:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Real Time Debugging");
		break;

	case WANCONFIG_ADCCP:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Special HDLC LAPB");
		break;

	case WANCONFIG_MLINK_PPP:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Multi-Link PPP");
		break;

	case WANCONFIG_GENERIC:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "WANPIPE Generic driver");
		break;

	case WANCONFIG_MPCHDLC:
		//snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Multi-Port CHDLC");
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "CHDLC");
		break;

	case WANCONFIG_TTY:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "TTY");
		break;
/*
	case PROTOCOL_TDM_VOICE:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "TDM Voice");
	 break;
*/
	default:
		snprintf((char*)protocol_name, MAX_PROT_NAME_LENGTH, "Invalid Protocol");
		break;
	}

	return (char*)protocol_name;
}

typedef struct _DEVICE_CONFIGURATION
{
	UCHAR return_code;

	ULONG maximum_length_of_rx_queue;
	USHORT maximum_rx_data_length;		//including CRC bytes

	ULONG maximum_length_of_tx_queue;
	USHORT maximum_tx_data_length;		//not including CRC bytes

}DEVICE_CONFIGURATION, *PDEVICE_CONFIGURATION;

#define MAXIMUM_PROTOCOL_LENGTH_OF_DATA	2048

#endif

/*FIXME: This should be taken out, I just
//used it so I don't break the apps that are
//using the WANCONFIG_ETH. Once those apps are
//changed, remove this definition
*/
#define WANCONFIG_ETH WANCONFIG_ADSL

/*----------------------------------------------------------------------------
 * WAN interface (logical channel) configuration (for ROUTER_IFNEW IOCTL).
 */
typedef struct wanif_conf
{
	unsigned magic;			/* magic number */
	unsigned config_id;		/* configuration identifier */

#if !defined(__WINDOWS__)
	char name[WAN_IFNAME_SZ+1];	/* interface name, ASCIIZ */

	/* x25 media source address, ASCIIZ */
	unsigned char x25_src_addr[WAN_ADDRESS_SZ+1];	 
	/* pattern match string in -d<string>
	 * for accepting calls, ASCIIZ */
	unsigned char accept_dest_addr[WAN_ADDRESS_SZ+1];   	
	/* pattern match string in -s<string> 
	 * for accepting calls, ASCIIZ */
	unsigned char accept_src_addr[WAN_ADDRESS_SZ+1];    
	/* pattern match string in -u<string> 
	 * for accepting calls, ASCIIZ */
	unsigned char accept_usr_data[WAN_ADDRESS_SZ+1];

	#ifdef ENABLE_IPV6
	unsigned char chap_userid[511];	/* List of User Id */
	unsigned char chap_passwd[511];	/* List of passwords */
	unsigned char pap_userid[511];	/* List of User Id */
	unsigned char pap_passwd[511];	/* List of passwords */
#else
	unsigned char userid[511];	/* List of User Id */
	unsigned char passwd[511];	/* List of passwords */
#endif

	unsigned char master[WAN_IFNAME_SZ+1];
	unsigned char label[WAN_IF_LABEL_SZ+1];
	unsigned char local_addr[WAN_ADDRESS_SZ+1];/* local media address, ASCIIZ */
	unsigned char sw_dev_name[WAN_IFNAME_SZ+1];
#endif//#if !defined(__WINDOWS__)

	char addr[WAN_ADDRESS_SZ+1];	/* media address, ASCIIZ */ //i.g. DLCI number
	char usedby[USED_BY_FIELD];	/* used by API or WANPIPE */
	unsigned idle_timeout;		/* sec, before disconnecting */
	unsigned hold_timeout;		/* sec, before re-connecting */
	unsigned cir;			/* Committed Information Rate fwd,bwd*/
	unsigned bc;			/* Committed Burst Size fwd, bwd */
	unsigned be;			/* Excess Burst Size fwd, bwd */ 
#ifdef ENABLE_IPV6
	unsigned char enable_IPv4;	/* Enable or Disable IPv4 */
	unsigned char enable_IPv6;	/* Enable or Disable IPv6 */
#endif
	unsigned char enable_IPX;	/* Enable or Disable IPX */
	unsigned char inarp;		/* Send Inverse ARP requests Y/N */
	unsigned inarp_interval;	/* sec, between InARP requests */
	unsigned long network_number;	/* Network Number for IPX */
	char mc;			/* Multicast on or off */
	
    unsigned char port;             /* board port */
    unsigned char protocol;         /* prococol used in this channel (TCPOX25 or X25) */
	char pap;			/* PAP enabled or disabled */
	char chap;			/* CHAP enabled or disabled */

	unsigned char sysname[31];	/* Name of the system */
	unsigned char ignore_dcd;	/* Protocol options: */
	unsigned char ignore_cts;	/*  Ignore these to determine */
	unsigned char ignore_keepalive;	/*  link status (Yes or No) */
	unsigned char hdlc_streaming;	/*  Hdlc streaming mode (Y/N) */
	unsigned keepalive_tx_tmr;	/* transmit keepalive timer */
	unsigned keepalive_rx_tmr;	/* receive  keepalive timer */
	unsigned keepalive_err_margin;	/* keepalive_error_tolerance */
	unsigned slarp_timer;		/* SLARP request timer */
	unsigned char ttl;		/* Time To Live for UDP security */
	char electrical_interface;			/* RS-232/V.35, etc. */
	char clocking;			/* external/internal */
	unsigned bps;			/* data transfer rate */
	unsigned mtu;			/* maximum transmit unit size */
	unsigned char if_down;		/* brind down interface when disconnected */
	unsigned char gateway;		/* Is this interface a gateway */
	unsigned char true_if_encoding;	/* Set the dev->type to true board protocol */

	unsigned char idle_char;     /* default Idle char for BitStream */
    unsigned char rts_hs_for_receive; /* async Protocol options */
    unsigned char xon_xoff_hs_for_receive;
	unsigned char xon_xoff_hs_for_transmit;
	unsigned char dcd_hs_for_transmit;
	unsigned char cts_hs_for_transmit;
	unsigned char async_mode;
	unsigned tx_bits_per_char;
	unsigned rx_bits_per_char;
	unsigned stop_bits;  
	unsigned char parity;
 	unsigned break_timer;
    unsigned inter_char_timer;
	unsigned rx_complete_length;
	unsigned xon_char;
	unsigned xoff_char;
	unsigned char receive_only;	/*  no transmit buffering (Y/N) */
	

	unsigned char inarp_rx;		/* Receive Inverse ARP requests Y/N */
	unsigned long active_ch;
	unsigned int  max_trace_queue;

	unsigned char auto_cfg;

	unsigned char call_logging;

	unsigned char station;
	
	union {
		wan_atm_conf_if_t	atm;	
		wan_x25_if_conf_t	x25;
		wan_lapb_if_conf_t	lapb;
		wan_dsp_if_conf_t	dsp;
		wan_fr_conf_t		fr;

		//wan_bitstrm_conf_if_t bitstrm;
		wan_xilinx_conf_if_t aft;

		wan_xdlc_conf_t 	xdlc;
		wan_sppp_if_conf_t	ppp;
		//Windows
		wan_sppp_if_conf_t	chdlc;	//the same typedef used for both CHDLC and PPP,
									//have both 'ppp' and 'chdlc' for clarity.
	}u;

	unsigned char seven_bit_hdlc;

	DEVICE_CONFIGURATION device_cfg;

} wanif_conf_t;

#define ATM_CELL_SIZE 	53
/*
**		TYPEDEF
*/
typedef struct atm_stats {
	unsigned int	rx_valid;
	unsigned int	rx_empty;
	unsigned int	rx_invalid_atm_hdr;	
	unsigned int	rx_invalid_prot_hdr;
	unsigned int	rx_atm_pdu_size;
	unsigned int	rx_chip;
	unsigned int	tx_valid;
	unsigned int	tx_chip;
	unsigned int	rx_congestion;
	unsigned int    rx_clp;
} atm_stats_t;

enum {
	ATM_CONNECTED,
	ATM_DISCONNECTED,
	ATM_AIS
};

typedef struct {
	unsigned short fr_active;
	unsigned short fr_inactive;
	unsigned short lapb_active;
	unsigned short lapb_inactive;
	unsigned short x25_link_active;
	unsigned short x25_link_inactive;
	unsigned short x25_active;
	unsigned short x25_inactive;
	unsigned short dsp_active;
	unsigned short dsp_inactive;
}wp_stack_stats_t;

/* ALEX_DEBUG*/
typedef struct wan_debug {
	unsigned long 	magic;	/* for verification */
	unsigned long	len;
	unsigned long	num;
	unsigned long	max_len;
	unsigned long	offs;
	int		is_more;
	char*		data;
} wan_kernel_msg_t;

typedef struct wanpipe_debug_msg_hdr_t {
	int len;
	unsigned long time;
} wanpipe_kernel_msg_hdr_t;


typedef struct wplip_prot_reg
{
	int (*prot_set_state) (void *, int, unsigned char *, int);
	int (*chan_set_state) (void *, int, unsigned char *, int);
	int (*tx_link_down)   (void *, void *);
	int (*tx_chan_down)   (void *, void *);
	int (*rx_up) 	      (void *, void *, int type);
	unsigned int (*get_ipv4_addr)(void *, int type);
	int (*set_ipv4_addr)(void *, 
			     unsigned int,
			     unsigned int,
			     unsigned int,
			     unsigned int);
	int mtu;
}wplip_prot_reg_t;

enum {
	WPLIP_RAW,
	WPLIP_IP,
	WPLIP_IPV6,
	WPLIP_IPX,
	WPLIP_FR_ARP
};

#define PACKET_TYPE_DECODE(type)					\
		((type == WPLIP_RAW)	? "WPLIP_RAW" :		\
		 (type == WPLIP_IP)		? "WPLIP_IP" :		\
         (type == WPLIP_IPV6)	? "WPLIP_IPV6" : 	\
		 (type == WPLIP_IPX)	? "WPLIP_IPX": 		\
		 (type == WPLIP_FR_ARP)	? "WPLIP_FR_ARP": "Unknown Packet type")


#endif