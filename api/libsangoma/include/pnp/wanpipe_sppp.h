/*
 * Defines for synchronous PPP/Cisco link level subroutines.
 *
 * Copyright (C) 1994 Cronyx Ltd.
 * Author: Serge Vakulenko, <vak@zebub.msk.su>
 *
 * This software is distributed with NO WARRANTIES, not even the implied
 * warranties for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors grant any other persons or organizations permission to use
 * or modify this software as long as this message is kept with the software,
 * all derivative works or modified versions.
 *
 * Version 1.7, Wed Jun  7 22:12:02 MSD 1995
 * 
 * Version 2.1, Wed Mar 26 10:03:00 EDT 2003
 *
 *
 */

/*
 *
 * Changes: v2.1 by David Rokhvarg <davidr@sangoma.com>
 *  
 * Changed for use by Sangoma WANPIPE driver. Added MD5 algorithm
 * declarations.
 * 	
 */

#ifndef _SYNCPPP_H_
#define _SYNCPPP_H_ 1 
/*
#include "stddef.h"
#include "wanpipe_abstr.h"
#include "wanpipe_cfg.h"
#include "wanpipe_sppp_iface.h"
*/

#include <wanpipe_includes.h>
#include <wanpipe_version.h>
#include <wanpipe_defines.h>

#include <wanpipe_abstr.h>
#include <wanpipe_cfg.h>
#include <wanpipe_sppp_iface.h>
#include <little_endian.h>


#define PPP_ALLSTATIONS 0xff            /* All-Stations broadcast address */
#define PPP_UI          0x03            /* Unnumbered Information */
#define PPP_IP          0x0021          /* Internet Protocol */
#define PPP_ISO         0x0023          /* ISO OSI Protocol */
#define PPP_XNS         0x0025          /* Xerox NS Protocol */
#define PPP_IPX         0x002b          /* Novell IPX Protocol */
#define PPP_LCP         0xc021          /* Link Control Protocol */
#define PPP_IPCP        0x8021          /* Internet Protocol Control Protocol */
#define PPP_PAP		0xc023		/* Password Authentication Protocol */
#define PPP_CHAP	0xc223		/* Challenge-Handshake Auth Protocol */

struct slcp {
	unsigned short	state;          /* state machine */
	unsigned int	magic;          /* local magic number */
	unsigned char	echoid;         /* id of last keepalive echo request */
	unsigned char	confid;         /* id of last configuration request */

	unsigned int	opts;		/* LCP options to send (bitfield) */
	unsigned int	mru;		/* our max receive unit */
	unsigned int	their_mru;	/* their max receive unit */
	unsigned int	protos;		/* bitmask of protos that are started */
	/* restart max values, see RFC 1661 */
	int	timeout;
	int	max_terminate;
	int	max_configure;
	int	max_failure;
};

struct sipcp {
	unsigned short	state;          /* state machine */
	unsigned char  confid;         /* id of last configuration request */
};

#define IDX_LCP 0		/* idx into state table */
#define IDX_IPCP 1		/* idx into state table */
#define IDX_IPV6CP 2		/* idx into state table */

#define AUTHNAMELEN	64
#define AUTHKEYLEN	16

struct sauth {
	unsigned short	proto;			/* authentication protocol to use */
	unsigned short	flags;
#define AUTHFLAG_NOCALLOUT	1	/* do not require authentication on */
					/* callouts */
#define AUTHFLAG_NORECHALLENGE	2	/* do not re-challenge CHAP */
	unsigned char	name[AUTHNAMELEN];	/* system identification name */
	unsigned char	secret[AUTHKEYLEN];	/* secret password */
	unsigned char	challenge[AUTHKEYLEN];	/* random challenge */
};

#define IDX_PAP		3
#define IDX_CHAP	4

#define IDX_COUNT (IDX_CHAP + 1) /* bump this when adding cp's! */

/*
 * Don't change the order of this.  Ordering the phases this way allows
 * for a comparision of ``pp_phase >= PHASE_AUTHENTICATE'' in order to
 * know whether LCP is up.
 */
enum ppp_phase {
	PHASE_DEAD, PHASE_ESTABLISH, PHASE_TERMINATE,
	PHASE_AUTHENTICATE, PHASE_NETWORK
};

#define PP_MTU          1500    /* default/minimal MRU */
#define PP_MAX_MRU	2048	/* maximal MRU we want to negotiate */

/*
 * This is a cut down struct sppp (see below) that can easily be
 * exported to/ imported from userland without the need to include
 * dozens of kernel-internal header files.  It is used by the
 * SPPPIO[GS]DEFS ioctl commands below.
 */
struct sppp_parms {
	enum ppp_phase pp_phase;	/* phase we're currently in */
	int	enable_vj;		/* VJ header compression enabled */
	int	enable_ipv6;		/*
					 * Enable IPv6 negotiations -- only
					 * needed since each IPv4 i/f auto-
					 * matically gets an IPv6 address
					 * assigned, so we can't use this as
					 * a decision.
					 */
	struct slcp lcp;		/* LCP params */
	struct sipcp ipcp;		/* IPCP params */
	struct sipcp ipv6cp;		/* IPv6CP params */
	struct sauth myauth;		/* auth params, i'm peer */
	struct sauth hisauth;		/* auth params, i'm authenticator */
};

/*
 * Definitions to pass struct sppp_parms data down into the kernel
 * using the SIOC[SG]IFGENERIC ioctl interface.
 *
 * In order to use this, create a struct spppreq, fill in the cmd
 * field with SPPPIOGDEFS, and put the address of this structure into
 * the ifr_data portion of a struct ifreq.  Pass this struct to a
 * SIOCGIFGENERIC ioctl.  Then replace the cmd field by SPPPIOSDEFS,
 * modify the defs field as desired, and pass the struct ifreq now
 * to a SIOCSIFGENERIC ioctl.
 */

#define SPPPIOGDEFS  ((caddr_t)(('S' << 24) + (1 << 16) +\
	sizeof(struct sppp_parms)))
#define SPPPIOSDEFS  ((caddr_t)(('S' << 24) + (2 << 16) +\
	sizeof(struct sppp_parms)))

struct spppreq {
	int	cmd;
	struct sppp_parms defs;
};

/* bits for pp_flags */
#define PP_KEEPALIVE    0x01    /* use keepalive protocol */
				/* 0x04 was PP_TIMO */
#define PP_CALLIN	0x08	/* we are being called */
#define PP_NEEDAUTH	0x10	/* remote requested authentication */

typedef struct sppp 
{
	unsigned long		critical;
	void 			*link_dev;
	void 			*dev;
	
	unsigned int		pp_flags;	/* use Cisco protocol instead of PPP */
	unsigned short		pp_alivecnt;	/* keepalive packets counter */
	unsigned short		pp_loopcnt;	/* loopback detection counter */
	struct slcp		lcp;		/* LCP params */
	struct sipcp		ipcp;		/* IPCP params */
	unsigned int		ibytes,obytes;	/* Bytes in/out */
	unsigned int		ipkts,opkts;	/* Packets in/out */
	unsigned int 		pp_timer;
	char			pp_link_state;	/* Link status */
	
	unsigned int  		pp_seq[IDX_COUNT];      /* local sequence number */
    unsigned int  		pp_rseq[IDX_COUNT];     /* remote sequence number */
    enum ppp_phase 		pp_phase;        /* phase we're currently in */
	int     		state[IDX_COUNT];       /* state machine */
	unsigned char  		confid[IDX_COUNT];      /* id of last configuration request */
	int     		rst_counter[IDX_COUNT]; /* restart counter */
	int     		fail_counter[IDX_COUNT]; /* negotiation failure counter */
	int     		confflags;      /* administrative configuration flags */
#define CONF_ENABLE_VJ    0x01  /* VJ header compression enabled */
#define CONF_ENABLE_IPV6  0x02  /* IPv6 administratively enabled */
	unsigned int  		pp_last_recv;   /* time last packet has been received */
	unsigned int  		pp_last_sent;   /* time last packet has been sent */
	
	struct sauth 		myauth;		/* auth params, i'm peer */
	struct sauth 		hisauth;		/* auth params, i'm authenticator */
	
	unsigned int		pp_auth_timer;
	//david - changed to abstructed timer
	//wan_timer_t			pp_auth_timer;
	unsigned int		pp_auth_flags;
	
	unsigned char 		dynamic_ip;
	unsigned char 		dynamic_event;
	unsigned int 		dynamic_failures;
	unsigned long 		local_ip;
	unsigned long 		remote_ip;
	unsigned char 		gateway;
	
	unsigned long 		task_working;
	
	unsigned char 		name[IFNAMSIZ+1];
	unsigned char 		hwdevname[IFNAMSIZ+1];

	//FIXME: all these is repeated in 'cfg' member.
	//Remove it from here, use 'cfg'.
	unsigned int		sppp_keepalive_timer;
	unsigned int  		sppp_max_keepalive_count;
	
	wplip_prot_reg_t	reg;
	
	wan_sppp_if_conf_t	cfg;

	ppp_pkt_stats_t		packet_stats;	
	ppp_lcp_stats_t		lcp_stats;
	ppp_lpbk_stats_t	lpbk_stats;
	ppp_prot_stats_t	prot_stats;
	ppp_pap_stats_t		pap_stats;
	ppp_chap_stats_t	chap_stats;
	ppp_conn_info_t		conn_info;
	chdlc_stat_t		chdlc_stats;
	
	unsigned int 		refcnt;

	unsigned char		type;

}wp_sppp_t;


#define PP_KEEPALIVE    0x01    /* use keepalive protocol */
#define PP_CISCO        0x02    /* use Cisco protocol instead of PPP */
#define PP_TIMO         0x04    /* cp_timeout routine active */
#define PP_DEBUG	0x08
#define PP_TIMO1        0x20    /* one of the authentication timeout routines is active */
	      
#define PPP_MTU          1500    /* max. transmit unit */

#define LCP_STATE_CLOSED        0       /* LCP state: closed (conf-req sent) */
#define LCP_STATE_ACK_RCVD      1       /* LCP state: conf-ack received */
#define LCP_STATE_ACK_SENT      2       /* LCP state: conf-ack sent */
#define LCP_STATE_OPENED        3       /* LCP state: opened */

#define IPCP_STATE_CLOSED       0       /* IPCP state: closed (conf-req sent) */
#define IPCP_STATE_ACK_RCVD     1       /* IPCP state: conf-ack received */
#define IPCP_STATE_ACK_SENT     2       /* IPCP state: conf-ack sent */
#define IPCP_STATE_OPENED       3       /* IPCP state: opened */

#define SPPP_LINK_DOWN		0	/* link down - no keepalive */
#define SPPP_LINK_UP		1	/* link is up - keepalive ok */

#ifndef wp_min
#define wp_min(x,y) \
	({ unsigned int __x = (x); unsigned int __y = (y); __x < __y ? __x: __y; })
#endif

#ifndef wp_max
#define wp_max(x,y) \
	({ unsigned int __x = (x); unsigned int __y = (y); __x > __y ? __x: __y; })
#endif
	
#endif



#define MAXALIVECNT     6               /* max. alive packets */

#define LCP_CONF_REQ    1               /* PPP LCP configure request */
#define LCP_CONF_ACK    2               /* PPP LCP configure acknowledge */
#define LCP_CONF_NAK    3               /* PPP LCP configure negative ack */
#define LCP_CONF_REJ    4               /* PPP LCP configure reject */
#define LCP_TERM_REQ    5               /* PPP LCP terminate request */
#define LCP_TERM_ACK    6               /* PPP LCP terminate acknowledge */
#define LCP_CODE_REJ    7               /* PPP LCP code reject */
#define LCP_PROTO_REJ   8               /* PPP LCP protocol reject */
#define LCP_ECHO_REQ    9               /* PPP LCP echo request */
#define LCP_ECHO_REPLY  10              /* PPP LCP echo reply */
#define LCP_DISC_REQ    11              /* PPP LCP discard request */

#define LCP_OPT_MRU             1       /* maximum receive unit */
#define LCP_OPT_ASYNC_MAP       2       /* async control character map */
#define LCP_OPT_AUTH_PROTO      3       /* authentication protocol */
#define LCP_OPT_QUAL_PROTO      4       /* quality protocol */
#define LCP_OPT_MAGIC           5       /* magic number */
#define LCP_OPT_RESERVED        6       /* reserved */
#define LCP_OPT_PROTO_COMP      7       /* protocol field compression */
#define LCP_OPT_ADDR_COMP       8       /* address/control field compression */

#define IPCP_CONF_REQ   LCP_CONF_REQ    /* PPP IPCP configure request */
#define IPCP_CONF_ACK   LCP_CONF_ACK    /* PPP IPCP configure acknowledge */
#define IPCP_CONF_NAK   LCP_CONF_NAK    /* PPP IPCP configure negative ack */
#define IPCP_CONF_REJ   LCP_CONF_REJ    /* PPP IPCP configure reject */
#define IPCP_TERM_REQ   LCP_TERM_REQ    /* PPP IPCP terminate request */
#define IPCP_TERM_ACK   LCP_TERM_ACK    /* PPP IPCP terminate acknowledge */
#define IPCP_CODE_REJ   LCP_CODE_REJ    /* PPP IPCP code reject */

#define CISCO_MULTICAST         0x8f    /* Cisco multicast address */
#define CISCO_UNICAST           0x0f    /* Cisco unicast address */
#define CISCO_KEEPALIVE         0x8035  /* Cisco keepalive protocol */
#define CISCO_IP		0x0800  /* Cisco IP packet */
#define CISCO_ADDR_REQ          0       /* Cisco address request */
#define CISCO_ADDR_REPLY        1       /* Cisco address reply */
#define CISCO_KEEPALIVE_REQ     2       /* Cisco keepalive request */

/* states are named and numbered according to RFC 1661 */
#define STATE_INITIAL	0
#define STATE_STARTING	1
#define STATE_CLOSED	2
#define STATE_STOPPED	3
#define STATE_CLOSING	4
#define STATE_STOPPING	5
#define STATE_REQ_SENT	6
#define STATE_ACK_RCVD	7
#define STATE_ACK_SENT	8
#define STATE_OPENED	9

#define PAP_REQ			1	/* PAP name/password request */
#define PAP_ACK			2	/* PAP acknowledge */
#define PAP_NAK			3	/* PAP fail */

#define CHAP_CHALLENGE		1	/* CHAP challenge request */
#define CHAP_RESPONSE		2	/* CHAP challenge response */
#define CHAP_SUCCESS		3	/* CHAP response ok */
#define CHAP_FAILURE		4	/* CHAP response failed */

#define CHAP_MD5		5	/* hash algorithm - MD5 */



struct ppp_header {
	unsigned char address;
	unsigned char control;
	unsigned short protocol;
};
#define PPP_HEADER_LEN          sizeof (struct ppp_header)

struct lcp_header {
	unsigned char type;
	unsigned char ident;
	unsigned short len;
};

struct ipcp_header {
	unsigned char type;
	unsigned char len;
	unsigned char data[1];
};


#define LCP_HEADER_LEN          sizeof (struct lcp_header)

struct cisco_packet {
	unsigned int type;
	unsigned int par1;
	unsigned int par2;
	unsigned short rel;
	unsigned short time0;
	unsigned short time1;
};
#define CISCO_PACKET_LEN 18
#define CISCO_BIG_PACKET_LEN 20

#define MAX_AUTHENTICATE_PACKET_LEN	256


#define	SPP_FMT		"%s: "
#define	SPP_ARGS(ifp)	(ifp)->name

# define UNTIMEOUT( arg ) 	wpsppp_auth_clear_timeout(arg)
# define TIMEOUT(fun, arg, time_in_seconds) wpsppp_auth_timeout(fun, arg, time_in_seconds)
# define IOCTL_CMD_T	int


/////////////////////////////

/*
 * We follow the spelling and capitalization of RFC 1661 here, to make
 * it easier comparing with the standard.  Please refer to this RFC in
 * case you can't make sense out of these abbreviation; it will also
 * explain the semantics related to the various events and actions.
 */
struct cp {
	unsigned short	proto;		/* PPP control protocol number */
	unsigned char protoidx;	/* index into state table in struct sppp */
	unsigned char flags;
#define CP_LCP		0x01	/* this is the LCP */
#define CP_AUTH		0x02	/* this is an authentication protocol */
#define CP_NCP		0x04	/* this is a NCP */
#define CP_QUAL		0x08	/* this is a quality reporting protocol */
	const char *name;	/* name of this control protocol */
	/* event handlers */
	void	(*Up)(struct sppp *sp);
	void	(*Down)(struct sppp *sp);
	void	(*Open)(struct sppp *sp);
	void	(*Close)(struct sppp *sp);
	void	(*TO)(void *sp);
	int	(*RCR)(struct sppp *sp, struct lcp_header *h, int len);
	void	(*RCN_rej)(struct sppp *sp, struct lcp_header *h, int len);
	void	(*RCN_nak)(struct sppp *sp, struct lcp_header *h, int len);
	/* actions */
	void	(*tlu)(struct sppp *sp);
	void	(*tld)(struct sppp *sp);
	void	(*tls)(struct sppp *sp);
	void	(*tlf)(struct sppp *sp);
	void	(*scr)(struct sppp *sp);
};

	

#define AUTHNAMELEN	64
#define AUTHKEYLEN	16

typedef unsigned int UWORD32;
typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef unsigned short UINT16;

#define md5byte unsigned char

struct wp_MD5Context {
	UWORD32 buf[4];
	UWORD32 bytes[2];
	UWORD32 in[16];
};

void wp_MD5Init(struct wp_MD5Context *context);
void wp_MD5Update(struct wp_MD5Context *context, md5byte *buf, unsigned len);
void wp_MD5Final(unsigned char digest[16], struct wp_MD5Context *context);
void wp_MD5Transform(UWORD32 buf[4], UWORD32 const in[16]);



void wpsppp_keepalive (struct sppp *sp);
void wpsppp_cp_send (struct sppp *sp, unsigned short proto, unsigned char type,
			  unsigned char ident, unsigned short len, void *data);
void wpsppp_cisco_send (struct sppp *sp, int type, long par1, long par2);
void wpsppp_lcp_open (struct sppp *sp);
void wpsppp_ipcp_open (struct sppp *sp);
int  wpsppp_lcp_conf_parse_options (struct sppp *sp, struct lcp_header *h,
					 int len, unsigned int *magic);
void wpsppp_cp_timeout (wp_sppp_t *sp);
char *wpsppp_lcp_type_name (unsigned char type);
char *wpsppp_ipcp_type_name (unsigned char type);
void wpsppp_print_bytes (unsigned char *p, unsigned short len);

int wpsppp_strnlen(unsigned char *p, int max);
const char * wpsppp_auth_type_name(unsigned short proto, unsigned char type);
void wpsppp_print_string(const char *p, unsigned short len);
void wpsppp_cp_change_state(const struct cp *cp, struct sppp *sp,
				 int newstate);
void wpsppp_auth_send(struct sppp *sp,
               unsigned int protocol, unsigned int type, unsigned int id,
	       int len, unsigned char *data_buf);

void wpsppp_auth_timeout(void * function, struct sppp *p, unsigned int seconds);
void wpsppp_auth_clear_timeout(struct sppp *p);
void wpsppp_phase_network(struct sppp *sp);
const char * wpsppp_phase_name(enum ppp_phase phase);

const char * wpsppp_proto_name(unsigned short proto);
const char * wpsppp_state_name(int state);
void wpsppp_null(struct sppp *unused);

void wpsppp_pap_scr(struct sppp *sp);

void wpsppp_chap_input(struct sppp *sp, void *skb);
void wpsppp_chap_init(struct sppp *sp);
void wpsppp_chap_open(struct sppp *sp);
void wpsppp_chap_close(struct sppp *sp);
void wpsppp_chap_TO(void *sp);
void wpsppp_chap_tlu(struct sppp *sp);
void wpsppp_chap_tld(struct sppp *sp);
void wpsppp_chap_scr(struct sppp *sp);

wp_sppp_t *wpsppp_create_cb(void);
void wpsppp_free_cb(struct sppp *sp);
void wp_sppp_attach(struct sppp *sp);
void wp_sppp_detach (void *sp_ptr);
void wpsppp_clear_timeout(struct sppp *p);
void wpsppp_set_timeout(struct sppp *p,int s);
void wp_sppp_input (void *sppp_ptr, void *skb);
int wpsppp_data_indication(wp_sppp_t *sppp, void *skb);
int wpsppp_data_transmit(wp_sppp_t *sppp, void *skb);
void wpsppp_disconnect_indication(wp_sppp_t *sppp, int reason);
void wpsppp_connect_indication(wp_sppp_t *sppp, int reason);
void byteSwap(unsigned int *buf, unsigned words);

unsigned int wpsppp_get_ipv4addr(wp_sppp_t *sppp, int type);
int wpsppp_set_ipv4addr(wp_sppp_t *sppp,
		 	unsigned int local,
			unsigned int remote,
			unsigned int netmask,
			unsigned int dns);

int wpsppp_set_gateway(wp_sppp_t *sppp);	
void wpsppp_bh (struct sppp *sp);

char *decode_lcp_state(int state);
char *decode_ipcp_state(int state);

extern unsigned int  debug;


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

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]



