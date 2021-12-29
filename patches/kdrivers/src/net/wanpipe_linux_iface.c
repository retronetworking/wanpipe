/****************************************************************************
* wanpipe_main.c	WANPIPE(tm) OpenSource WANPIPe Driver.
*
* Author:		Alex Feldman	<al.feldman@sangoma.com>
*		
*
* Copyright:		(c) 2004 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jan 28, 2004	Alex Feldman	Initial version.
*****************************************************************************/

#if defined (__LINUX__)

# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanrouter.h>	/* WAN router definitions */
# include <linux/sdladrv.h>
# include <linux/wanpipe.h>	/* WANPIPE common user API definitions */
# include <linux/sdlapci.h>
# include <linux/if_wanpipe.h>
# include <linux/if_wanpipe_common.h>
# include <linux/wanpipe_iface.h>

#else

# error "Only Linux OS support HDLC interface!"

#endif

/****** Defines & Macros ****************************************************/

#ifdef	_DEBUG_
#define	STATIC
#else
#define	STATIC		static
#endif

/****** Function Prototypes *************************************************/
static netdevice_t* wan_iface_alloc (int);
static void wan_iface_free (netdevice_t*);
static int wan_iface_attach (netdevice_t*, char*,int);
static void wan_iface_detach (netdevice_t*, int);
static int wan_iface_init(netdevice_t* dev);
static int wan_iface_input(netdevice_t*, netskb_t*);
static int wan_iface_set_proto(netdevice_t*, struct ifreq*);


static int wan_iface_open(netdevice_t*);
static int wan_iface_close(netdevice_t*);
static int wan_iface_send(netskb_t*, netdevice_t*);
static int wan_iface_ioctl(netdevice_t*, struct ifreq*, int);
static struct net_device_stats* wan_iface_get_stats (netdevice_t*);
static void wan_iface_tx_timeout (netdevice_t*);

#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC
static int wan_iface_hdlc_attach(hdlc_device*, unsigned short,unsigned short);
#endif

/****** Global Data *********************************************************/
wan_iface_t wan_iface = 
{
	wan_iface_alloc,	/* alloc */
	wan_iface_free,		/* free */
	wan_iface_attach,	/* attach */	
	wan_iface_detach,	/* detach */	
	wan_iface_input,	/* input */	
	wan_iface_set_proto	/* set_proto */	
};

/******* WAN Device Driver Entry Points *************************************/

static netdevice_t* wan_iface_alloc (int is_netdev)
{
	netdevice_t	*dev;
	int err;
	unsigned char 	tmp_name[]="wan";
#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC
	hdlc_device*	hdlc;
#endif

	/* Register in HDLC device */
	if (is_netdev){
		dev = wan_netif_alloc(tmp_name,&err);
		if (dev == NULL)
			return NULL;

	}else{
#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC	
		hdlc = (hdlc_device*)wan_malloc(sizeof(hdlc_device));
		if (hdlc == NULL)
			return NULL;
		dev = hdlc_to_dev(hdlc);
#else
		DEBUG_EVENT("%s: Critical Compile Error %i!\n",__FUNCTION__,__LINE__);
		dev = NULL;
#endif
	}
	return dev;
}

static void wan_iface_free(netdevice_t* dev)
{
	wan_netif_free(dev);
}

static int wan_iface_attach (netdevice_t* dev, char *ifname, int is_netdev)
{
	int err = 0;
	if (is_netdev){
		if (ifname){
			wan_netif_init(dev, ifname);
		}
		dev->init = &wan_iface_init;
		err=register_netdev(dev);
	}else{
#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC
		hdlc_device*	hdlc = dev_to_hdlc(dev);

		hdlc->attach 	= wan_iface_hdlc_attach;
		hdlc->xmit 	= wan_iface_send;

		err = register_hdlc_device(hdlc);
#else
		err = -EINVAL;
#endif
	}
	if (err){
		DEBUG_EVENT("%s: Failed to register interface (%d)\n",
					wan_netif_name(dev), err);
		dev->init = NULL;
		*(dev->name) = 0;
		wan_iface_free(dev);
		return -EINVAL;
	}
	return 0;
}

static void wan_iface_detach (netdevice_t* dev, int is_netdev)
{
	DEBUG_EVENT("%s: Unregister interface!\n", 
				wan_netif_name(dev));
	if (is_netdev){
		if (dev->priv){
			dev->priv=NULL;
		}
		unregister_netdev(dev);
	}else{
#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC
		unregister_hdlc_device(dev_to_hdlc(dev));
#else
		DEBUG_EVENT("%s: Compilation Assertion Error! %d\n",__FUNCTION__,__LINE__);
#endif
	}
	return;
}


static int wan_iface_init(netdevice_t* dev)
{
//	dev->priv = NULL;	/* We need 'priv', hdlc doesn't */
	dev->get_stats		= &wan_iface_get_stats;
	dev->do_ioctl		= &wan_iface_ioctl;
	dev->open		= &wan_iface_open;
	dev->stop		= &wan_iface_close;

	dev->hard_header	= NULL;
	dev->rebuild_header	= NULL;
	dev->hard_start_xmit	= &wan_iface_send;
	dev->get_stats		= &wan_iface_get_stats;
	dev->tx_timeout		= &wan_iface_tx_timeout;
	dev->watchdog_timeo	= HZ*2;
	dev->hard_header_len	= 16;
	dev->set_config		= NULL;
	
	/* Initialize media-specific parameters */
	dev->flags		|= IFF_POINTOPOINT;
	dev->flags		|= IFF_NOARP;

	dev->mtu		= 1500;
	dev->tx_queue_len	= 100;
	dev->type		= ARPHRD_PPP;
	dev->addr_len		= 0;
	*(u8*)dev->dev_addr	= 0; 

	dev->trans_start	= SYSTEM_TICKS;

	/* Initialize socket buffers */
	dev_init_buffers(dev);

	DEBUG_TEST("%s: %s:%d %p\n",
			dev->name,
			__FUNCTION__,__LINE__,
			dev->priv);

	return 0;
}

static int wan_iface_open(netdevice_t* dev)
{
	wanpipe_common_t	*common;
	int			err;
#ifdef  CONFIG_PRODUCT_WANPIPE_GENERIC
	void			*org_priv;
#endif

	WAN_ASSERT(wan_netif_priv(dev) == NULL);
	common = wan_netif_priv(dev);

#ifdef  CONFIG_PRODUCT_WANPIPE_GENERIC
	/* Save original private pointer and restore it later after returning
	 * from hdlc_open function call (Linux-2.6 overwrite private pointer. */
	org_priv = wan_netif_priv(dev);
	hdlc_open(dev_to_hdlc(dev));
	wan_netif_set_priv(dev, org_priv);
#endif

	DEBUG_TEST("%s: OPEN \n",dev->name);
	
	if (common->iface.open){
		err = common->iface.open(dev);
#ifdef  CONFIG_PRODUCT_WANPIPE_GENERIC
		if (err){
			hdlc_close(dev_to_hdlc(dev));
		}
#endif
		return err;
	}
	return -EINVAL;
}

static int wan_iface_close(netdevice_t* dev)
{
	wanpipe_common_t	*common;
	int			err;

	WAN_ASSERT(wan_netif_priv(dev) == NULL);
	common = wan_netif_priv(dev);
	if (common->iface.close){
		err = common->iface.close(dev);
#ifdef  CONFIG_PRODUCT_WANPIPE_GENERIC
		hdlc_close(dev_to_hdlc(dev));
#endif
		return err;
	}
	return -EINVAL;
}

static int wan_iface_send(netskb_t* skb, netdevice_t* dev)
{
	wanpipe_common_t	*common;

	WAN_ASSERT(wan_netif_priv(dev) == NULL);
	common = wan_netif_priv(dev);
	if (common->iface.send){
		return common->iface.send(skb, dev);
	}else{
		wan_skb_free(skb);
	}
	return 0;
}

static int wan_iface_ioctl(netdevice_t* dev, struct ifreq* ifr, int cmd)
{
	wanpipe_common_t	*common;
	int 			err = -EOPNOTSUPP;

	WAN_ASSERT(wan_netif_priv(dev) == NULL);

#if 0
	DEBUG_EVENT("%s: %s:%d\n",dev->name,__FUNCTION__,__LINE__);
	return err;
#endif	
	common = wan_netif_priv(dev);
	switch(cmd){

	default:
		if (common->iface.ioctl){
			err = common->iface.ioctl(dev, ifr, cmd);
		}
		break;
	}
	if (err == 1){
#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC
		err = hdlc_ioctl(dev, ifr, cmd);
#else
		err = -EINVAL;
#endif
	}	
	return err;
}

/* Get ethernet-style interface statistics.
 * Return a pointer to struct enet_statistics.
 */
static struct net_device_stats	gif_stats;
static struct net_device_stats* wan_iface_get_stats (netdevice_t* dev)
{
	wanpipe_common_t	*common;

#if 0
NC: This is not an error
On shutdown, this is possible
	WAN_ASSERT2(wan_netif_priv(dev) == NULL, NULL);
#endif

	common = wan_netif_priv(dev);
	if (common && common->iface.get_stats){
		return common->iface.get_stats(dev);
	}else{
		return &gif_stats;
	}
	return NULL; 
}

/*============================================================================
 * Handle transmit timeout event from netif watchdog
 */
static void wan_iface_tx_timeout (netdevice_t *dev)
{
	wanpipe_common_t	*common;

	WAN_ASSERT1(wan_netif_priv(dev) == NULL);
	common = wan_netif_priv(dev);
	/* If our device stays busy for at least 5 seconds then we will
	 * kick start the device by making dev->tbusy = 0.  We expect
	 * that our device never stays busy more than 5 seconds. So this 
	 * is only used as a last resort.
	 */
	if (common->iface.tx_timeout){
		common->iface.tx_timeout(dev);
	}
	return;
}

static int wan_iface_input(netdevice_t* dev, netskb_t* skb)
{
	wanpipe_common_t	*common;

	WAN_ASSERT(wan_netif_priv(dev) == NULL);
	common = wan_netif_priv(dev);

#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC
	if (!common->is_netdev){
		skb->protocol = htons(ETH_P_HDLC);
		skb->dev = dev;
		skb->mac.raw  = skb->data;
	}
#endif

	netif_rx(skb);
	return 0;
}

#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC
static int 
wan_iface_hdlc_attach(hdlc_device * hdlc, unsigned short encoding, unsigned short parity)
{
	return 0;
}
#endif

static int wan_iface_set_proto(netdevice_t* dev, struct ifreq* ifr)
{
	wanpipe_common_t*	common;
	int			err = 0;


	if ((common = wan_netif_priv(dev)) == NULL){
		DEBUG_EVENT("%s: Private structure is null!\n",
				wan_netif_name(dev));
		return -EINVAL;
	}

#ifdef CONFIG_PRODUCT_WANPIPE_GENERIC
	{
	struct if_settings*	ifsettings;
	ifsettings = (struct if_settings*)ifr->ifr_data;

		
	case IF_PROTO_PPP:
	case IF_PROTO_CISCO:
	case IF_PROTO_FR:
		common->protocol = 
			(ifsettings->type == IF_PROTO_PPP) ? WANCONFIG_PPP :
			(ifsettings->type == IF_PROTO_CISCO) ? WANCONFIG_CHDLC :
								WANCONFIG_FR;	
		if (common->protocol == WANCONFIG_PPP){
			hdlc_device* hdlc = dev_to_hdlc(dev);
			((wanpipe_common_t*)dev->priv)->prot_ptr = 
					(netdevice_t*)&hdlc->state.ppp.pppdev;
#if defined(LINUX2_6)
			hdlc->state.ppp.syncppp_ptr = (struct ppp_device*)
					((wanpipe_common_t*)dev->priv)->prot_ptr;
#endif
		}


		err = hdlc_ioctl(dev, ifr, SIOCWANDEV);
		break;
	default:
		err = -EINVAL;
		break;
	}
	}

#else
	err = -EINVAL;
#endif
	return err;
}

/************************************ END **********************************/
