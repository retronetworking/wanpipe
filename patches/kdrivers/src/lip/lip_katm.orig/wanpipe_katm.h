
#ifndef _WAN_KATM_H_
#define _WAN_KATM_H_ 1 

#include "stddef.h"
#include <linux/types.h>
#include "wanpipe_abstr.h"
#include "wanpipe_cfg.h"

/* This is the shared header between the LIP
 * and this PROTOCOL */
#include "wanpipe_katm_iface.h"

#include <linux/atm.h>
#include <linux/atmdev.h>

#include <linux/wanpipe_lip_atm_iface.h>

#define WPIFNAMSIZ 20

/* RWM Added #defines to limit speed to T1 rates / connections */
/* This will have to increase for E1, specifically the CELL_RATE */
#define LOG2_NUM_VPIS			0
#define LOG2_NUM_VCIS_PER_VPI		14
#define AFT101_CELL_RATE			3622  /* (8000*24)/46.875 from Cisco's website */

/* Default AAL5 Maximum Transmission Unit (and length of AAL5 buffers) */
#define AAL5_MTU (10 + 1586 + 8) /* LLC/SNAP hdr + max-eth-frame-size + AAL5 trailer */
#define AAL5_BUFLEN (((AAL5_MTU + 47)/48)*48) /* Round up to n*48 bytes */
#define AAL5_CRC32_MASK	(0xDEBB20E3) /* CRC-32 Mask */


typedef struct _katm_ 
{
	unsigned long		critical;
	
	/* Used by lip layer */
	void 			*link_dev;
	void 			*dev;
	
	unsigned char		type;
	wplip_prot_reg_t	reg;
	wan_atm_conf_if_t	cfg;
	unsigned char 		name[WPIFNAMSIZ+1];
	unsigned char 		hwdevname[WPIFNAMSIZ+1];
	int				state;
	unsigned int 		refcnt;
	
	/* INSERT PROTOCOL SPECIFIC VARIABLES HERE */
	void				*atmdev;  //
	void				*sar_dev; 
} wp_katm_t;



typedef struct wp_katm_header 
{
	/* IMPLEMENT PROTOCOL HEADER HERE */

} wp_katm_header_t;

typedef struct wp_katm_channel
{
	/******************************
	* Stuff used by all channels *
     ******************************/
    struct atm_dev	*dev;        		/* Points back at device */
    int			chanid;     		/* Channel ID used by CPM */
    unchar		aal;        		/* ATM_AAL0 or ATM_AAL5 */
    unchar		traffic_class;      /* UBR, ABR, CBR, NONE, ANYCLASS */
    /********************************************************
     * Stuff used by all channels except the Raw Cell Queue *
     ********************************************************/
    struct atm_vcc		*vcc;          /* Ptr to socket layer per VC info */
    unchar			vpi;			/* VPI in use */
    ushort			vci;			/* VCI in use */

    /********************************************
     * Stuff used by rx and bidir channels only *
     ********************************************/
    int                   rx_ring_size; /* Num rx BDs and skbuffs for this channel */
    struct sk_buff**      rx_skbuff;    /* points to rx_ring_size sk_buff ptrs */
    int				 rx_tail; 	/* read next from rbase[rx_tail] */
#ifdef MULTI_BUFFER_FRAMES
    struct sk_buff*       lskb;		/* for AAL5 frames larger than AAL5_BUFLEN */
#endif

    /***********************************************
     * Stuff used by CBR tx or bidir channels only *
     ***********************************************/
    ulong			      tx_cellrate; /* Requested Transmission data rate (CBR only) */
    
    char name[WPIFNAMSIZ+1];
    
    /* Link back to the wp_katm_t device */
    wp_katm_t *atm_link;
    void *sar_vcc;

} wp_katm_channel_t;


/* PRIVATE Function Prototypes Go Here */
int wan_lip_katm_open(struct atm_vcc *vcc);
void wan_lip_katm_close(struct atm_vcc *vcc);
int wan_lip_katm_send(struct atm_vcc *vcc,struct sk_buff *skb);
int wan_lip_katm_setsockopt(struct atm_vcc *vcc, int level, int optname,
		      void *optval, int optlen);
int wan_lip_katm_getsockopt(struct atm_vcc *vcc, int level, int optname,
		      void *optval, int optlen);
int wan_lip_katm_sg_send(struct atm_vcc *vcc, unsigned long start,
		   unsigned long size);
//int wp_katm_activate_channel(struct atm_vcc *vcc, wp_katm_channel_t *chan);

/* ATM Callbacks */
int wplip_katm_prot_rx_up (void *lip_dev_ptr, void *skb, int type);
int wplip_katm_link_callback_tx_down (void *wplink_id, void *skb);
int wplip_katm_callback_tx_down (void *lip_dev_ptr, void *skb);
int wplip_katm_lipdev_prot_change_state(void *wplip_id,int state, 
		                          unsigned char *data, int len);
int wplip_katm_link_prot_change_state (void *wplip_id,int state, unsigned char *data, int len);

int wp_katm_data_transmit(wp_katm_t *prot, void *skb);
					  


/* PRIVATE Debug Functions */

#define wp_dev_hold(dev)    do{\
				wpabs_debug_test("%s:%d (dev_hold) Refnt %i!\n",\
							__FUNCTION__,__LINE__,dev->refcnt );\
				dev->refcnt++; \
			    }while(0)

#define wp_dev_put(dev)	    do{\
				wpabs_debug_test("%s:%d (wp_dev_put): Refnt %i!\n",\
							__FUNCTION__,__LINE__,dev->refcnt );\
				if (--dev->refcnt == 0){\
					wpabs_debug_test("%s:%d (wp_dev_put): Deallocating!\n",\
							__FUNCTION__,__LINE__);\
					wpabs_free(dev);\
					dev=NULL;\
				}\
		            }while(0)

#define FUNC_BEGIN() wpabs_debug_test("%s:%d ---Begin---\n",__FUNCTION__,__LINE__); 
#define FUNC_END() wpabs_debug_test("%s:%d ---End---\n\n",__FUNCTION__,__LINE__); 

#define FUNC_BEGIN1() wpabs_debug_event("%s:%d ---Begin---\n",__FUNCTION__,__LINE__); 
#define FUNC_END1() wpabs_debug_event("%s:%d ---End---\n\n",__FUNCTION__,__LINE__); 


#endif


