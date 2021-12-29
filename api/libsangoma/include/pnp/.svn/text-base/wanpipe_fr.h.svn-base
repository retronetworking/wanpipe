/***********************************************************
 * wanpipe_fr.h  Wanpipe Multi Protocol Module
 *
 *
 *
 *
 */

#ifndef _WANPIPE_HDLC_PROT_H_
#define _WANPIPE_HDLC_PROT_H_

#if !defined(__WINDOWS__)
 #include "stddef.h"
 #include "wanpipe_snmp.h"
 #include "wanpipe_abstr.h"
 #include "wanpipe_cfg.h"
 #include "wanpipe_fr_iface.h"
#else
 #if defined (__KERNEL__)
  #include <wanpipe_includes.h>
  #include <wanpipe_version.h>
  #include <wanpipe_defines.h>
  #include <wanpipe_cfg.h>
  #include <wanpipe_abstr.h>
  #include <wanpipe_fr_iface.h>
 #endif
#endif


#undef MAX_FR_CHANNELS
#undef HIGHEST_VALID_DLCI

#if !defined(__WINDOWS__)
#define MAX_FR_CHANNELS		1023
#else
#define MAX_FR_CHANNELS		MAX_NUMBER_OF_PROTOCOL_INTERFACES//1023
#endif

#define HIGHEST_VALID_DLCI  MAX_FR_CHANNELS-1
#define LOWEST_VALID_DLCI	16

#define MIN_T391	5
#define MAX_T391	30
#define MIN_T392	5
#define MAX_T392	30
#define MIN_N391	1
#define MAX_N391	255
#define MIN_N392	1
#define MAX_N392	10
#define MIN_N393	1
#define MAX_N393	10


#define MODE_FR_CCITT 0

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

/* CISCO data frame encapsulation */
#define CISCO_UI	0x08
#define CISCO_IP	0x00
#define CISCO_INV	0x20


#define ETH_P_ARP       0x0806          /* Address Resolution packet    */

/* 'status' defines */
#define	FR_LINK_INOPER	0x00		/* for global status (DLCI == 0) */
#define	FR_LINK_OPER	0x01

#define PVC_STATE_NEW       0x01
#define PVC_STATE_ACTIVE    0x02
#define PVC_STATE_FECN	    0x08 /* FECN condition */
#define PVC_STATE_BECN      0x10 /* BECN condition */


#define LMI_ANSI_DLCI	0
#define LMI_LMI_DLCI	1023

#define LMI_PROTO               0x09
#define LMI_ANSI_PROTO          0x08
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

#pragma pack(1)
typedef struct {
	unsigned char ea1  : 1;
	unsigned char cr   : 1;
	unsigned char dlcih: 6;
  
	unsigned char ea2  : 1;
	unsigned char de   : 1;
	unsigned char becn : 1;
	unsigned char fecn : 1;
	unsigned char dlcil: 4;

	unsigned char	ui;
	unsigned char   prot;
}fr_hdr;
#pragma pack()


#if defined (__KERNEL__)

typedef struct fr_channel
{
	unsigned short 	dlci;
	unsigned int	dlci_type;
	
	unsigned char	dlci_state;
	unsigned char   inarp;
	unsigned char   route_flag;
	unsigned short  newstate;

	unsigned long	rx_DE_set;
	unsigned long	tx_DE_set;
	unsigned long	rx_FECN;
	unsigned long	rx_BECN;

	void *dev;
	void *prot;
	struct fr_channel *next;

	signed int cisco_hdr;
	
	unsigned char name[20];

	fr_dlci_stat_t	dlci_stats;
	
	unsigned int down;
	signed int refcnt;

	unsigned char type;
}fr_channel_t;

typedef struct fr_prot
{
	wan_fr_conf_t 	cfg;

	fr_link_stat_t  link_stats;
	
	void		*dlci_to_dev_map[MAX_FR_CHANNELS];
	unsigned char   global_dlci_map[MAX_FR_CHANNELS];
	
	unsigned long 	last_rx_poll;
	unsigned int 	txseq, rxseq;
	unsigned char 	state;
	unsigned long 	n391cnt;
	unsigned int 	last_errors;
	unsigned short  lmi_dlci;
	unsigned int	singalling;
	
	void 		*update_dlci;

	unsigned int chan_cnt;
	unsigned int max_rx_queue;
	fr_channel_t* chan_list;

	unsigned char name[50];
	struct fr_prot *next;	

	wplip_prot_reg_t reg;

	void *dev;
	unsigned int down;
	signed int refcnt;

}fr_prot_t;
#endif

static __inline unsigned short status_to_dlci(unsigned char signalling, unsigned char *status, unsigned char *state)
{
	*state &= ~(PVC_STATE_ACTIVE | PVC_STATE_NEW);
	if (status[2] & 0x08)
		*state |= PVC_STATE_NEW;
	else if (status[2] & 0x02)
		*state |= PVC_STATE_ACTIVE;

	if (signalling == WANOPT_FR_ANSI){
		return ((status[0] & 0x3F)<<4) | ((status[1] & 0x78)>>3);
	}else{
		return ((status[0] << 8) | status[1]);
	}
}

static __inline unsigned short q922_to_dlci(unsigned char *hdr)
{
    return ((hdr[0] & 0xFC)<<2) | ((hdr[1] & 0xF0)>>4);
}


static __inline unsigned char fr_lmi_nextseq(unsigned char x)
{
	x++;
	return x ? x : 1;
}

static __inline void dlci_to_q922(unsigned char *hdr, unsigned short dlci)
{
        hdr[0] = (dlci>>2) & 0xFC;
        hdr[1] = ((dlci<<4) & 0xF0) | 0x01;
}


#if defined (__KERNEL__)

static __inline void dlci_to_status(fr_prot_t *fr_prot, unsigned short dlci, unsigned char *status,
				      unsigned char state)
{

	if (fr_prot->cfg.signalling == WANOPT_FR_ANSI){
		status[0] = (dlci>>4) & 0x3F;
		status[1] = ((dlci<<3) & 0x78) | 0x80;
	}else{
		status[0] = dlci>>8;
		status[1] = dlci&0xFF;
	}
	
	status[2] = 0x80;

	if (state & PVC_STATE_NEW){
		status[2] |= 0x08;
	}else if (state & PVC_STATE_ACTIVE){
		status[2] |= 0x02;
	}
}

/*============================================================================
 * Find network device by its channel number.
 *
 * We need this critical flag because we change
 * the dlci_to_dev_map outside the interrupt.
 *
 * NOTE: del_if() functions updates this array, it uses
 *       the spin locks to avoid corruption.
 */
static __inline void* find_channel (fr_prot_t* fr_prot, unsigned dlci)
{
	if(dlci > HIGHEST_VALID_DLCI)
		return NULL;

	return(fr_prot->dlci_to_dev_map[dlci]);
}

static __inline void init_global_dlci_state(fr_prot_t *fr_prot)
{
	wpabs_memset(fr_prot->global_dlci_map,0,sizeof(fr_prot->global_dlci_map));
	return;
}

static __inline void set_global_dlci_state(fr_prot_t *fr_prot, unsigned short dlci, unsigned char state)
{
	if (dlci > HIGHEST_VALID_DLCI){
		return;
	}
	fr_prot->global_dlci_map[dlci] = state;
}

static __inline void fr_log_dlci_active(fr_channel_t *chan)
{
	wpabs_debug_event("%s: DLCI %d: %s active %s\n", 
	       chan->name,
	       chan->dlci,
	       chan->dlci_state & PVC_STATE_ACTIVE ? " " : "in",
	       chan->dlci_state & PVC_STATE_NEW ? " new" : " ");
}

#define wp_dev_hold(dev)    dev->refcnt++
#define wp_dev_put(dev)	    do{\
				if (--dev->refcnt == 0){\
					wpabs_debug_cfg("%s:%d (wp_dev_put): Deallocating!\n",\
							__FUNCTION__,__LINE__);\
					wpabs_free(dev);\
					dev=NULL;\
				}\
			}while(0)

#endif//WAN_KERNEL
#endif

