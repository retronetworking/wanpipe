#ifndef _WANPIPE_CFG_H_
#define _WANPIPE_CFG_H_


#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <net/sdla_56k.h>
# include <net/sdla_te1.h>
# include <net/sdla_te3.h>
# include <net/sdla_front_end.h>
#elif defined(__LINUX__)
# include <linux/sdla_56k.h>
# include <linux/sdla_te1.h>
# include <linux/sdla_te3.h>
# include <linux/sdla_front_end.h>
#else
# error "No OS Defined!"
#endif

/* DSL interface types */
#define WAN_INTERFACE 	0
#define LAN_INTERFACE	1

/* Miscellaneous */
#define	WAN_IFNAME_SZ	15	/* max length of the interface name */
#define	WAN_DRVNAME_SZ	15	/* max length of the link driver name */
#define	WAN_ADDRESS_SZ	31	/* max length of the WAN media address */
#define USED_BY_FIELD	10	/* max length of the used by field */

#define WAN_AUTHNAMELEN 64

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
/* FIXME: Remove these 2 defines (use WAN_x) */
#define CMD_OK		0	/* normal firmware return code */
#define CMD_TIMEOUT		0xFF	/* firmware command timed out */

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
#define WANOPT_S50X	1
#define WANOPT_S51X	2
#define WANOPT_ADSL	3
#define WANOPT_AFT	4
#define WANOPT_AFT104	5
#define WANOPT_AFT300	6

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


/* SS7 options */
#define WANOPT_SS7_FISU 0
#define WANOPT_SS7_LSSU 1

#define WANOPT_SS7_MODE_128 	0
#define WANOPT_SS7_MODE_4096	1

#define WANOPT_SS7_FISU_4096_SZ 6
#define WANOPT_SS7_FISU_128_SZ  3


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

/* Interface Operation Modes */
#define WANPIPE		0x00
#define API		0x01
#define BRIDGE		0x02
#define BRIDGE_NODE	0x03
#define SWITCH		0x04
#define STACK		0x05
#define ANNEXG		0x06
#define TDM_VOICE	0x07
#define TTY		0x08


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
	WAN_LIMIT,		/* for verification only */
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
		(card_type == WANOPT_AFT300) ?  "A300"  : "Unknown"

#define COMPORT_DECODE(port)	(port == WANOPT_PRI) ? "PRI" : "SEC"

#define STATE_DECODE(state)					\
		((state == WAN_UNCONFIGURED) ? "Unconfigured" :	\
		 (state == WAN_DISCONNECTED) ? "Disconnected" : \
	         (state == WAN_CONNECTING) ? "Connecting" : 	\
		 (state == WAN_CONNECTED) ? "Connected": 	\
		 (state == WAN_LIMIT) ? "Limit": 		\
		 (state == WAN_DUALPORT) ? "DualPort": 		\
		 (state == WAN_DISCONNECTING) ? "Disconnecting": \
		 (state == WAN_FT1_READY) ? "FT1 Ready": "Invalid")

#define CFG_DECODE(value)	(value) ? "YES" : "NO"

#define CLK_DECODE(clocking)	(clocking) ? "INT" : "EXT"

#define INT_DECODE(interface)					\
		(interface == WANOPT_RS232) ? "RS232" : "V35"

#define SIGNALLING_DECODE(sig)					\
		(sig == WANOPT_FR_ANSI) ? "ANSI" :	\
		(sig == WANOPT_FR_Q933) ? "Q333" :	\
		(sig == WANOPT_FR_LMI) ? "LMI" : "NO"
		
#define COMPORT_DECODE(port)	(port == WANOPT_PRI) ? "PRI" : "SEC"

#define IP_MODE_DECODE(ip_mode)					\
		(ip_mode == WANOPT_PPP_STATIC) ? "STATIC" :	\
		(ip_mode == WANOPT_PPP_PEER) ? "PEER" : "HOST"
		
#define X25_STATION_DECODE(station)				\
		(station == WANOPT_DTE) ? "DTE" : 		\
		(station == WANOPT_DCE) ? "DCE" : "DXE"

#define FR_STATION_DECODE(station)				\
		(station == WANOPT_CPE) ? "CPE" : "Node"

typedef char devname_t[WAN_DRVNAME_SZ+1];

typedef enum {
    RFC_MODE_BRIDGED_ETH_LLC    = 0,
    RFC_MODE_BRIDGED_ETH_VC     = 1,
    RFC_MODE_ROUTED_IP_LLC      = 2,
    RFC_MODE_ROUTED_IP_VC       = 3,
    RFC_MODE_RFC1577_ENCAP      = 4,
    RFC_MODE_PPP_LLC 	    	= 5,
    RFC_MODE_PPP_VC		= 6,
    RFC_MODE_STACK_VC		= 7
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


typedef struct wan_bitstrm_conf{
	/* Bitstreaming options */
	unsigned short sync_options;
	unsigned short rx_sync_char;
	unsigned char monosync_tx_time_fill_char;
	unsigned int  max_length_tx_data_block;
	unsigned int  rx_complete_length;
	unsigned int  rx_complete_timer;
	
	unsigned long rbs_map;

}wan_bitstrm_conf_t;


typedef struct wan_bitstrm_conf_if
{
	unsigned int	max_tx_queue_size;
	unsigned int	max_tx_up_size;
	unsigned char 	seven_bit_hdlc;
}wan_bitstrm_conf_if_t;

/*----------------------------------------------------------------------------
 * X.25-specific link-level configuration.
 */
typedef struct wan_x25_conf
{
	unsigned int lo_pvc;	/* lowest permanent circuit number */
	unsigned int hi_pvc;	/* highest permanent circuit number */
	unsigned int lo_svc;	/* lowest switched circuit number */
	unsigned int hi_svc;	/* highest switched circuit number */
	unsigned int hdlc_window; /* HDLC window size (1..7) */
	unsigned int pkt_window;  /* X.25 packet window size (1..7) */
	unsigned int t1;	  /* HDLC timer T1, sec (1..30) */
	unsigned int t2;	  /* HDLC timer T2, sec (0..29) */
	unsigned int t4;	/* HDLC supervisory frame timer = T4 * T1 */
	unsigned int n2;	/* HDLC retransmission limit (1..30) */
	unsigned int t10_t20;	/* X.25 RESTART timeout, sec (1..255) */
	unsigned int t11_t21;	/* X.25 CALL timeout, sec (1..255) */
	unsigned int t12_t22;	/* X.25 RESET timeout, sec (1..255) */
	unsigned int t13_t23;	/* X.25 CLEAR timeout, sec (1..255) */
	unsigned int t16_t26;	/* X.25 INTERRUPT timeout, sec (1..255) */
	unsigned int t28;	/* X.25 REGISTRATION timeout, sec (1..255) */
	unsigned int r10_r20;	/* RESTART retransmission limit (0..250) */
	unsigned int r12_r22;	/* RESET retransmission limit (0..250) */
	unsigned int r13_r23;	/* CLEAR retransmission limit (0..250) */
	unsigned int ccitt_compat;   /* compatibility mode: 1988/1984/1980 */
	unsigned int x25_conf_opt;   /* User defined x25 config optoins */
	unsigned char LAPB_hdlc_only; /* Run in HDLC only mode */
	unsigned char logging;        /* Control connection logging */  
	unsigned char oob_on_modem; /* Whether to send modem 
				       status to the user app */
	unsigned char local_station_address;  /*Local Station address */
	unsigned short defPktSize;
	unsigned short pktMTU;
	unsigned char cmd_retry_timeout; /* Value is seconds */
	unsigned char station;
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
	unsigned dlci[100];     /* List of all DLCIs */
	unsigned char issue_fs_on_startup;
	unsigned char station;  /* Node or CPE */
} wan_fr_conf_t;


typedef struct wan_xilinx_conf
{
	unsigned short dma_per_ch; 	/* DMA buffers per logic channel */
	unsigned short mru;		/* MRU of transparent channels */
	unsigned char  rbs;		/* Robbit signalling support */
	unsigned int   data_mux_map;	/* Data mux map */
	unsigned int   fe_ref_clock;	/* Front End Reference Clock */
	unsigned int   tdmv_span_no;
}wan_xilinx_conf_t;


typedef struct wan_xilinx_conf_if
{
	unsigned char 	station;   	/* Node or CPE */
	unsigned int  	signalling; 	/* local in-channel signalling type */
	unsigned char 	seven_bit_hdlc;
	unsigned int  	mru;
	unsigned int  	mtu;
	unsigned char  	idle_flag;
	unsigned char  	data_mux;
	unsigned char	ss7_enable;
	unsigned char 	ss7_mode;
	unsigned char	ss7_lssu_size;
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


#define WANOPT_TWO_WAY_ALTERNATE 	0
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

	unsigned char  station_configuration;		
	unsigned long  baud_rate;	
	unsigned short max_I_field_length;
	unsigned short general_operational_config_bits;
	unsigned short protocol_config_bits;
	unsigned short exception_condition_reporting_config;
	unsigned short modem_config_bits;
	unsigned short statistics_format;
	unsigned short pri_station_slow_poll_interval;
	unsigned short permitted_sec_station_response_TO;
	unsigned short no_consec_sec_TOs_in_NRM_before_SNRM_issued;
	unsigned short max_lgth_I_fld_pri_XID_frame;		
	unsigned short opening_flag_bit_delay_count;		
	unsigned short RTS_bit_delay_count;
	unsigned short CTS_timeout_1000ths_sec;	
	unsigned char  SDLA_configuration;	
	
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
	unsigned int station;
	unsigned char label[WAN_IF_LABEL_SZ+1];
	unsigned char virtual_addr[WAN_ADDRESS_SZ+1];
	unsigned char real_addr[WAN_ADDRESS_SZ+1];
}wan_lapb_if_conf_t;

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

	unsigned int  gateway;
	unsigned char ppp_prot;
}wan_sppp_if_conf_t;


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
	
	
#if 0
/*----------------------------------------------------------------------------
 * T1/E1 configuration structures.
 */
typedef struct sdla_te_cfg {
	unsigned char media;
	unsigned char lcode;
	unsigned char frame;
	unsigned char lbo;
	unsigned char te_clock;
	unsigned long active_ch;
	unsigned char high_impedance_mode;
} sdla_te_cfg_t;

/* Performamce monitor counters */
typedef struct pmc_pmon {
	unsigned long pmon1;
	unsigned long pmon2;
	unsigned long pmon3;
	unsigned long pmon4;
} pmc_pmon_t;
#endif

/*----------------------------------------------------------------------------
 * WAN device configuration. Passed to ROUTER_SETUP IOCTL.
 */
typedef struct wandev_conf
{
	unsigned magic;		/* magic number (for verification) */
	unsigned config_id;	/* configuration structure identifier */
				/****** hardware configuration ******/
	unsigned ioport;	/* adapter I/O port base */
	unsigned long maddr;	/* dual-port memory address */
	unsigned msize;		/* dual-port memory size */
	int irq;		/* interrupt request level */
	int dma;		/* DMA request level */
	char S514_CPU_no[1];	/* S514 PCI adapter CPU number ('A' or 'B') */
        unsigned PCI_slot_no;	/* S514 PCI adapter slot number */
	char auto_pci_cfg;	/* S515 PCI automatic slot detection */
	int comm_port;		/* Communication Port (PRI=0, SEC=1) */ 
	unsigned bps;		/* data transfer rate */
	unsigned mtu;		/* maximum transmit unit size */
        unsigned udp_port;      /* UDP port for management */
	unsigned char ttl;	/* Time To Live for UDP security */
	unsigned char ft1;	/* FT1 Configurator Option */
        char interface;		/* RS-232/V.35, etc. */
	char clocking;		/* external/internal */
	char line_coding;	/* NRZ/NRZI/FM0/FM1, etc. */
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

	union{			/****** protocol-specific ************/
	
		wan_x25_conf_t 		x25;	/* X.25 configuration */
		wan_ppp_conf_t 		ppp;	/* PPP configuration */
		wan_fr_conf_t 		fr;	/* frame relay configuration */
		wan_chdlc_conf_t	chdlc;	/* Cisco HDLC configuration */
		wan_ss7_conf_t 		ss7;	/* SS7 Configuration */
		wan_sdlc_conf_t 	sdlc;	/* SDLC COnfiguration */
		wan_bscstrm_conf_t 	bscstrm; /* Bisync Streaming Configuration */
		wan_adsl_conf_t 	adsl;	/* ADSL Configuration */
		wan_atm_conf_t 		atm;
		wan_xilinx_conf_t 	aft;
		wan_bitstrm_conf_t	bitstrm;
		wan_xdlc_conf_t 	xdlc;
	} u;

	/* No new variables are allowed above */
	
	char card_type;		/* Supported Sangoma Card type */
	unsigned pci_bus_no;	/* S514 PCI bus number */

	sdla_fe_cfg_t	fe_cfg;	/* Front end configurations */

	unsigned char line_idle; /* IDLE FLAG/ IDLE MARK */
	unsigned char ignore_front_end_status;
	unsigned int  max_trace_queue;
	unsigned int  max_rx_queue;

#if 0
	/* Bitstreaming options */
	unsigned int  sync_options;
	unsigned char rx_sync_char;
	unsigned char monosync_tx_time_fill_char;
	unsigned int  max_length_tx_data_block;
	unsigned int  rx_complete_length;
	unsigned int  rx_complete_timer;

	unsigned int  max_trace_queue;
	unsigned int  max_rx_queue;
#endif
	
} wandev_conf_t;



/* 'config_id' definitions */
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
#define WANCONFIG_AFT    	117	/* AFT Original Hardware Support */
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
#define WANCONFIG_AFT_TE1    	128	/* AFT Quad Hardware Support */

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
	unsigned 	magic;			/* magic number */
	unsigned 	config_id;		/* configuration identifier */
	char 		name[WAN_IFNAME_SZ+1];	/* interface name, ASCIIZ */
	char 		addr[WAN_ADDRESS_SZ+1];	/* media address, ASCIIZ */
	char 		usedby[USED_BY_FIELD];	/* used by API or WANPIPE */
	unsigned 	idle_timeout;		/* sec, before disconnecting */
	unsigned 	hold_timeout;		/* sec, before re-connecting */
	unsigned 	cir;			/* Committed Information Rate fwd,bwd*/
	unsigned 	bc;			/* Committed Burst Size fwd, bwd */
	unsigned 	be;			/* Excess Burst Size fwd, bwd */ 
#ifdef ENABLE_IPV6
	unsigned char 	enable_IPv4;	/* Enable or Disable IPv4 */
	unsigned char 	enable_IPv6;	/* Enable or Disable IPv6 */
#endif
	unsigned char 	enable_IPX;	/* Enable or Disable IPX */
	unsigned char 	inarp;		/* Send Inverse ARP requests Y/N */
	unsigned 	inarp_interval;	/* sec, between InARP requests */
	unsigned long 	network_number;	/* Network Number for IPX */
	char 		mc;			/* Multicast on or off */
	char 		local_addr[WAN_ADDRESS_SZ+1];/* local media address, ASCIIZ */
        unsigned char 	port;             /* board port */
        unsigned char 	protocol;         /* prococol used in this channel (TCPOX25 or X25) */
	char 		pap;			/* PAP enabled or disabled */
	char 		chap;			/* CHAP enabled or disabled */
#ifdef ENABLE_IPV6
	unsigned char 	chap_userid[WAN_AUTHNAMELEN];	/* List of User Id */
	unsigned char 	chap_passwd[WAN_AUTHNAMELEN];	/* List of passwords */
	unsigned char 	pap_userid[WAN_AUTHNAMELEN];	/* List of User Id */
	unsigned char 	pap_passwd[WAN_AUTHNAMELEN];	/* List of passwords */
#else
	unsigned char 	userid[WAN_AUTHNAMELEN];	/* List of User Id */
	unsigned char 	passwd[WAN_AUTHNAMELEN];	/* List of passwords */
#endif
	unsigned char 	sysname[31];		/* Name of the system */
	unsigned char 	ignore_dcd;		/* Protocol options: */
	unsigned char 	ignore_cts;	 	/*  Ignore these to determine */
	unsigned char 	ignore_keepalive; 	/*  link status (Yes or No) */
	unsigned char 	hdlc_streaming;	  	/*  Hdlc streaming mode (Y/N) */
	unsigned 	keepalive_tx_tmr;	/* transmit keepalive timer */
	unsigned 	keepalive_rx_tmr;	/* receive  keepalive timer */
	unsigned 	keepalive_err_margin;	/* keepalive_error_tolerance */
	unsigned 	slarp_timer;		/* SLARP request timer */
	unsigned char 	ttl;			/* Time To Live for UDP security */
	char 		interface;		/* RS-232/V.35, etc. */
	char 		clocking;		/* external/internal */
	unsigned 	bps;			/* data transfer rate */
	unsigned 	mtu;			/* maximum transmit unit size */
	unsigned char 	if_down;	/* brind down interface when disconnected */
	unsigned char 	gateway;	/* Is this interface a gateway */
	unsigned char 	true_if_encoding; /* Set the dev->type to true board protocol */

	unsigned char 	asy_data_trans;     /* async API options */
        unsigned char 	rts_hs_for_receive; /* async Protocol options */
        unsigned char 	xon_xoff_hs_for_receive;
	unsigned char 	xon_xoff_hs_for_transmit;
	unsigned char 	dcd_hs_for_transmit;
	unsigned char 	cts_hs_for_transmit;
	unsigned char 	async_mode;
	unsigned 	tx_bits_per_char;
	unsigned 	rx_bits_per_char;
	unsigned 	stop_bits;  
	unsigned char 	parity;
 	unsigned 	break_timer;
        unsigned 	inter_char_timer;
	unsigned 	rx_complete_length;
	unsigned 	xon_char;
	unsigned 	xoff_char;
	unsigned char 	receive_only;	/*  no transmit buffering (Y/N) */
	
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
	
	unsigned char inarp_rx;		/* Receive Inverse ARP requests Y/N */
	unsigned long active_ch;
	unsigned int  max_trace_queue;
	unsigned char sw_dev_name[WAN_IFNAME_SZ+1];
	unsigned char auto_cfg;

	unsigned char call_logging;
	unsigned char master[WAN_IFNAME_SZ+1];
	unsigned char station;
	unsigned char label[WAN_IF_LABEL_SZ+1];

	unsigned int	spanno;		/* TDMV span device number */
	unsigned char	tdmv_echo_off;  /* TDMV echo disable */

	unsigned char lip_prot;
	union {
		wan_atm_conf_if_t 	atm;	
		wan_x25_if_conf_t 	x25;
		wan_lapb_if_conf_t 	lapb;
		wan_dsp_if_conf_t 	dsp;
		wan_fr_conf_t 		fr;
		wan_bitstrm_conf_if_t 	bitstrm;
		wan_xilinx_conf_if_t 	aft;
		wan_xdlc_conf_t 	xdlc;
		wan_sppp_if_conf_t	ppp;
	}u;

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

#endif
