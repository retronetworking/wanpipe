#ifndef __WANPIPE_MFR__
#define __WANPIPE_MFR__

#define HDLC_PROT_ONLY
#include <linux/sdla_chdlc.h>
#include <linux/sdla_fr.h>


#undef  wan_udphdr_data
#define wan_udphdr_data	wan_udphdr_u.data

#undef  wan_udp_data
#define wan_udp_data  wan_udp_hdr.wan_udphdr_data

#undef MAX_FR_CHANNELS
#undef HIGHEST_VALID_DLCI

#define MAX_FR_CHANNELS 1023
#define HIGHEST_VALID_DLCI  MAX_FR_CHANNELS-1

typedef struct {
	unsigned ea1  : 1;
	unsigned cr   : 1;
	unsigned dlcih: 6;
  
	unsigned ea2  : 1;
	unsigned de   : 1;
	unsigned becn : 1;
	unsigned fecn : 1;
	unsigned dlcil: 4;
}__attribute__ ((packed)) fr_hdr;


#define	FR_HEADER_LEN	8

#define LINK_STATE_RELIABLE 0x01
#define LINK_STATE_REQUEST  0x02 /* full stat sent (DCE) / req pending (DTE) */
#define LINK_STATE_CHANGED  0x04 /* change in PVCs state, send full report */
#define LINK_STATE_FULLREP_SENT 0x08 /* full report sent */

#define FR_UI              0x03
#define FR_PAD             0x00

#define NLPID_IP           0xCC
#define NLPID_IPV6         0x8E
#define NLPID_SNAP         0x80
#define NLPID_PAD          0x00
#define NLPID_Q933         0x08



/* 'status' defines */
#define	FR_LINK_INOPER	0x00		/* for global status (DLCI == 0) */
#define	FR_LINK_OPER	0x01

#if 0
#define FR_DLCI_INOPER  0x00
#define	FR_DLCI_DELETED	0x01		/* for circuit status (DLCI != 0) */
#define	FR_DLCI_ACTIVE	0x02
#define	FR_DLCI_WAITING	0x04
#define	FR_DLCI_NEW	0x08
#define	FR_DLCI_REPORT	0x40
#endif

#define PVC_STATE_NEW       0x01
#define PVC_STATE_ACTIVE    0x02
#define PVC_STATE_FECN	    0x08 /* FECN condition */
#define PVC_STATE_BECN      0x10 /* BECN condition */


#define LMI_ANSI_DLCI	0
#define LMI_LMI_DLCI	1023

#define LMI_PROTO               0x08
#define LMI_CALLREF             0x00 /* Call Reference */
#define LMI_ANSI_LOCKSHIFT      0x95 /* ANSI lockshift */
#define LMI_REPTYPE                1 /* report type */
#define LMI_CCITT_REPTYPE       0x51
#define LMI_ALIVE                  3 /* keep alive */
#define LMI_CCITT_ALIVE         0x53
#define LMI_PVCSTAT                7 /* pvc status */
#define LMI_CCITT_PVCSTAT       0x57
#define LMI_FULLREP                0 /* full report  */
#define LMI_INTEGRITY              1 /* link integrity report */
#define LMI_SINGLE                 2 /* single pvc report */
#define LMI_STATUS_ENQUIRY      0x75
#define LMI_STATUS              0x7D /* reply */

#define LMI_REPT_LEN               1 /* report type element length */
#define LMI_INTEG_LEN              2 /* link integrity element length */

#define LMI_LENGTH                13 /* standard LMI frame length */
#define LMI_ANSI_LENGTH           14

#define HDLC_MAX_MTU 1500	/* Ethernet 1500 bytes */
#define HDLC_MAX_MRU (HDLC_MAX_MTU + 10) /* max 10 bytes for FR */

#define MAX_TRACE_QUEUE 	  100
#define TRACE_QUEUE_LIMIT	  1001
#define	MAX_TRACE_TIMEOUT	  (HZ*10)

static __inline__ u16 status_to_dlci(u8 *status, u8 *state)
{
	*state &= ~(PVC_STATE_ACTIVE | PVC_STATE_NEW);
	if (status[2] & 0x08)
		*state |= PVC_STATE_NEW;
	else if (status[2] & 0x02)
		*state |= PVC_STATE_ACTIVE;

	return ((status[0] & 0x3F)<<4) | ((status[1] & 0x78)>>3);
}



static __inline__ u16 q922_to_dlci(u8 *hdr)
{
        return ((hdr[0] & 0xFC)<<2) | ((hdr[1] & 0xF0)>>4);
}


static inline u8 fr_lmi_nextseq(u8 x)
{
	x++;
	return x ? x : 1;
}

static __inline__ void dlci_to_q922(u8 *hdr, u16 dlci)
{
        hdr[0] = (dlci>>2) & 0xFC;
        hdr[1] = ((dlci<<4) & 0xF0) | 0x01;
}



#if 0
/* Special UDP drivers management commands */
#define FPIPE_ENABLE_TRACING          	0x41
#define FPIPE_DISABLE_TRACING		0x42
#define FPIPE_GET_TRACE_INFO            0x43
#define FPIPE_FT1_READ_STATUS           0x44
#define FPIPE_DRIVER_STAT_IFSEND        0x45
#define FPIPE_DRIVER_STAT_INTR          0x46
#define FPIPE_DRIVER_STAT_GEN           0x47
#define FPIPE_FLUSH_DRIVER_STATS        0x48
#define FPIPE_ROUTER_UP_TIME            0x49
#define FPIPE_TE1_56K_STAT   		0x50	/* TE1_56K */
#define FPIPE_GET_MEDIA_TYPE 		0x51	/* TE1_56K */
#define FPIPE_FLUSH_TE1_PMON 		0x52	/* TE1     */
#define FPIPE_READ_REGISTER 		0x53	/* TE1_56K */
#define FPIPE_TE1_CFG 			0x54	/* TE1     */


/* UDP management packet layout (data area of ip packet) */

typedef struct {
        unsigned char   control                 PACKED;
        unsigned char   NLPID                   PACKED;
} fr_encap_hdr_t;

typedef struct {
	ip_pkt_t 		ip_pkt		PACKED;
	udp_pkt_t		udp_pkt		PACKED;
	wp_mgmt_t 		wp_mgmt       	PACKED;
        cblock_t                cblock          PACKED;
        unsigned char           data[4080]      PACKED;
} fr_udp_pkt_t;


/*----------------------------------------------------------------------------
 * Global Statistics Block.
 *	This structure is returned by the FR_READ_STATISTICS command when
 *	dcli == 0.
 */
typedef struct	fr_link_stat
{
	unsigned short rx_too_long	PACKED;	/* 00h:  */
	unsigned short rx_dropped	PACKED;	/* 02h:  */
	unsigned short rx_dropped2	PACKED;	/* 04h:  */
	unsigned short rx_bad_dlci	PACKED;	/* 06h:  */
	unsigned short rx_bad_format	PACKED;	/* 08h:  */
	unsigned short retransmitted	PACKED;	/* 0Ah:  */
	unsigned short cpe_tx_FSE	PACKED;	/* 0Ch:  */
	unsigned short cpe_tx_LIV	PACKED;	/* 0Eh:  */
	unsigned short cpe_rx_FSR	PACKED;	/* 10h:  */
	unsigned short cpe_rx_LIV	PACKED;	/* 12h:  */
	unsigned short node_rx_FSE	PACKED;	/* 14h:  */
	unsigned short node_rx_LIV	PACKED;	/* 16h:  */
	unsigned short node_tx_FSR	PACKED;	/* 18h:  */
	unsigned short node_tx_LIV	PACKED;	/* 1Ah:  */
	unsigned short rx_ISF_err	PACKED;	/* 1Ch:  */
	unsigned short rx_unsolicited	PACKED;	/* 1Eh:  */
	unsigned short rx_SSN_err	PACKED;	/* 20h:  */
	unsigned short rx_RSN_err	PACKED;	/* 22h:  */
	unsigned short T391_timeouts	PACKED;	/* 24h:  */
	unsigned short T392_timeouts	PACKED;	/* 26h:  */
	unsigned short N392_reached	PACKED;	/* 28h:  */
	unsigned short cpe_SSN_RSN	PACKED;	/* 2Ah:  */
	unsigned short current_SSN	PACKED;	/* 2Ch:  */
	unsigned short current_RSN	PACKED;	/* 2Eh:  */
	unsigned short curreny_T391	PACKED;	/* 30h:  */
	unsigned short current_T392	PACKED;	/* 32h:  */
	unsigned short current_N392	PACKED;	/* 34h:  */
	unsigned short current_N393	PACKED;	/* 36h:  */
} fr_link_stat_t;

/*----------------------------------------------------------------------------
 * DLCI statistics.
 *	This structure is returned by the FR_READ_STATISTICS command when
 *	dlci != 0.
 */
typedef struct	fr_dlci_stat
{
	unsigned long tx_frames		PACKED;	/* 00h:  */
	unsigned long tx_bytes		PACKED;	/* 04h:  */
	unsigned long rx_frames		PACKED;	/* 08h:  */
	unsigned long rx_bytes		PACKED;	/* 0Ch:  */
	unsigned long rx_dropped	PACKED;	/* 10h:  */
	unsigned long rx_inactive	PACKED;	/* 14h:  */
	unsigned long rx_exceed_CIR	PACKED;	/* 18h:  */
	unsigned long rx_DE_set		PACKED;	/* 1Ch:  */
	unsigned long tx_throughput	PACKED;	/* 20h:  */
	unsigned long tx_calc_timer	PACKED;	/* 24h:  */
	unsigned long rx_throughput	PACKED;	/* 28h:  */
	unsigned long rx_calc_timer	PACKED;	/* 2Ch:  */
} fr_dlci_stat_t;

/*----------------------------------------------------------------------------
 * Communications error statistics.
 *	This structure is returned by the FR_READ_ERROR_STATS command.
 */
typedef struct	fr_comm_stat
{
	unsigned char rx_overruns	PACKED;	/* 00h:  */
	unsigned char rx_bad_crc	PACKED;	/* 01h:  */
	unsigned char rx_aborts		PACKED;	/* 02h:  */
	unsigned char rx_too_long	PACKED;	/* 03h:  */
	unsigned char tx_aborts		PACKED;	/* 04h:  */
	unsigned char tx_underruns	PACKED;	/* 05h:  */
	unsigned char tx_missed_undr	PACKED;	/* 06h:  */
	unsigned char dcd_dropped	PACKED;	/* 07h:  */
	unsigned char cts_dropped	PACKED;	/* 08h:  */
} fr_comm_stat_t;

#endif

typedef struct {

	struct tasklet_struct wanpipe_task;
	unsigned long 	tq_working;
	netdevice_t	*dlci_to_dev_map[MAX_FR_CHANNELS];
	unsigned char   global_dlci_map[MAX_FR_CHANNELS];
	wan_fr_conf_t 	cfg;	
	unsigned char 	station;
	struct sk_buff_head rx_free;
	struct sk_buff_head rx_used;
	struct sk_buff_head lmi_queue;
	struct sk_buff_head trace_queue;
	unsigned long 	last_rx_poll;
	unsigned int 	txseq, rxseq;
	unsigned char 	state;
	unsigned long 	n391cnt;
	unsigned int 	last_errors;
	struct timer_list timer;
	unsigned short  lmi_dlci;
	netdevice_t 	*tx_dev;

	fr_link_stat_t  link_stats;
	void 		*update_dlci;
	unsigned long	tracing_enabled;
	int		max_trace_queue;
	unsigned long   trace_timeout;

	unsigned int	max_rx_queue;
} fr_prot_t;

#endif
