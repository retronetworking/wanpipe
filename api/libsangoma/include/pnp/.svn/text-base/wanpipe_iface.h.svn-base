

#ifndef __WANPIPE_IFACE_H
# define __WANPIPE_IFACE_H

#if defined(__LINUX__)
# include <linux/hdlc.h>
#endif

#if defined(__NetBSD__) || defined (__FreeBSD__) || defined(__OpenBSD__)

# define IF_IFACE_V35		0x1001
# define IF_IFACE_T1		0x1002
# define IF_IFACE_E1		0x1003
# define IF_IFACE_SYNC_SERIAL	0x1004

# define IF_PROTO_HDLC		0x2001
# define IF_PROTO_PPP		0x2002
# define IF_PROTO_CISCO		0x2003
# define IF_PROTO_FR		0x2004
# define IF_PROTO_FR_ADD_PVC	0x2005
# define IF_PROTO_FR_DEL_PVC	0x2006
# define IF_PROTO_X25		0x2007
# define WAN_PROTO_X25		0x2007

# define IF_GET_PROTO		0x3001

# define te1_settings		void
# define sync_serial_settings	void

#elif defined(__LINUX__)

# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
//#  define ifs_size		data_length
//#  define SIOCWANDEV		SIOCDEVICE
# elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
//#  define ifs_size		size
# else
//#  define ifs_size		data_length
#ifndef SIOCWANDEV
#  define SIOCWANDEV		SIOCDEVICE
#endif
# endif

#elif defined(__WINDOWS__)

/************************************************
 *	SET COMMON KERNEL DEFINE					*
 ************************************************/
#if defined (__KERNEL__) || defined (KERNEL) || defined (_KERNEL)
# define WAN_KERNEL
#endif

#endif

#if defined(OLD_IFSETTINGS_STRUCT)
# define ifs_size		data_length
# define ifs_te1		data
# define ifs_sync              	data
# define ifs_cisco             	data
# define ifs_fr                	data
# define ifs_fr_pvc            	data
# define ifs_fr_pvc_info       	data
#else
# define ifs_size		size
# define ifs_te1               	ifs_ifsu.te1
# define ifs_sync              	ifs_ifsu.sync
# define ifs_cisco             	ifs_ifsu.cisco
# define ifs_fr                	ifs_ifsu.fr
# define ifs_fr_pvc            	ifs_ifsu.fr_pvc
# define ifs_fr_pvc_info       	ifs_ifsu.fr_pvc_info
#endif


#define WANHDLC_CONF_INTERFACE	0x0001
#define WANHDLC_CONF_CLOCKING	0x0002
#define WANHDLC_CONF_MTU	0x0004
#define WANHDLC_CONF_UDPPORT	0x0008
#define WANHDLC_CONF_TTL	0x0010

typedef struct {
	unsigned long	mask;
	char		interface;
	unsigned	bps;
	unsigned	mtu;
	unsigned 	udp_port;
	unsigned	ttl;
} wan_dev_conf_t;

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
typedef struct { int dummy; } cisco_proto, fr_proto, fr_proto_pvc;
struct if_settings
{
	unsigned int	type;
	unsigned int	data_length;
	void*		data;
};
#endif

#if 0
typedef struct {
	int		proto;
	int		iface;
	char		hwprobe[100];
	sdla_te_cfg_t	te_cfg;
	wan_dev_conf_t	devconf;
	union {
		cisco_proto	cisco;
		fr_proto	fr;
		fr_proto_pvc	fr_pvc;
	} protocol;
} wanlite_def_t;
#endif

/* WANPIPE Generic function interface */
# if defined(WAN_KERNEL)
typedef struct
{
	netdevice_t*(*alloc)(int);
	void(*free)(netdevice_t*);
	int(*attach)(netdevice_t*, char*, int);
	void(*detach)(netdevice_t*, int);
	int(*input)(netdevice_t*, netskb_t*);
	int(*set_proto)(netdevice_t*, struct ifreq*);
} wan_iface_t;

#if 0
/* ALEX Remove it !!! */
netdevice_t*	wan_iface_alloc (int);
void		wan_iface_free (netdevice_t* dev);
int		wan_iface_attach(netdevice_t*, char*, int);
void		wan_iface_detach(netdevice_t*, int);
int		wan_iface_open(netdevice_t* dev);
int		wan_iface_close(netdevice_t* dev);
int		wan_iface_input(netdevice_t*, netskb_t*);
int		wan_iface_tx_timeout(netdevice_t*);
int		wan_iface_set_proto(netdevice_t* dev, struct ifreq* ifr);
#endif

# endif
#endif /* __WANPIPE_IFACE_H */
