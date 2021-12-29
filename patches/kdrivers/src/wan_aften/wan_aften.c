

#if defined(__LINUX__)
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe.h>
# include <linux/wanpipe_cfg.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <net/wanpipe_includes.h>
# include <net/wanpipe.h>
# include <net/wanpipe_cfg.h>
#else
# error "This Operating System is not supported!"
#endif

#include "wan_aften.h"

/*#define DEBUG_REG*/

static char	*wan_drvname = "wan_aften";
static char 	wan_fullname[]	= "WANPIPE(tm) Sangoma AFT (lite) Driver";
static char	wan_copyright[]	= "(c) 1995-2005 Sangoma Technologies Inc.";
static int	ncards; 

static sdla_t* card_list = NULL;	/* adapter data space */
extern wan_iface_t wan_iface;

static int wan_aften_init(void*);
static int wan_aften_exit(void*);
static int wan_aften_setup(sdla_t *card, netdevice_t *dev);
static int wan_aften_shutdown(sdla_t *card);
static int wan_aften_update_ports(void);
static int wan_aften_open(netdevice_t *dev);
static int wan_aften_close(netdevice_t *dev);
static int wan_aften_ioctl (netdevice_t *dev, struct ifreq *ifr, int cmd);

#if defined(__OpenBSD__)
struct cdevsw wan_aften_devsw = {
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
WAN_MODULE_DEFINE(
	wan_aften,"wan_aften", 
	"Alex Feldman <al.feldman@sangoma.com>",
	"WAN AFT Enable", 
	"GPL",
	wan_aften_init, wan_aften_exit,
	&wan_aften_devsw);
#else
WAN_MODULE_DEFINE(
	wan_aften,"wan_aften", 
	"Alex Feldman <al.feldman@sangoma.com>",
	"WAN AFT Enable", 
	"GPL",
	wan_aften_init, wan_aften_exit,
	NULL);
#endif
WAN_MODULE_DEPEND(wan_aften, sdladrv, 1, SDLADRV_MAJOR_VER, SDLADRV_MAJOR_VER);
WAN_MODULE_VERSION(wan_aften, WAN_AFTEN_VER);

static int wan_aften_init(void *arg)
{
	struct wan_aften_priv	*priv = NULL;
	struct wan_dev_le	*devle;
	sdla_t			*card, *tmp;
	static int		if_index = 0;
	int 			err = 0, i;

#if defined(__OpenBSD__) || defined(__NetBSD__)
	sdladrv_init();
#endif
        DEBUG_EVENT("%s v%d %s\n", 
				wan_fullname, 
				WAN_AFTEN_VER, 
				wan_copyright); 

	
	ncards = sdla_hw_probe();
	if (!ncards){
		DEBUG_EVENT("No Sangoma cards found, unloading modules!\n");
		return -ENODEV;
	}

	for (i=0;i<ncards;i++){
		wan_device_t*	wandev;
		netdevice_t*	dev;

		card=wan_malloc(sizeof(sdla_t));
		if (!card){
			DEBUG_EVENT("%s: Failed allocate new card!\n", 
					wan_drvname);
			goto wanpipe_init_done;
		}
		memset(card, 0, sizeof(sdla_t));
		/* Allocate HDLC device */
		if (!wan_iface.alloc || (dev = wan_iface.alloc(1)) == NULL){
			wan_free(card);
			goto wanpipe_init_done;
		}

		devle = wan_malloc(sizeof(struct wan_dev_le));
		if (devle == NULL){
			DEBUG_EVENT("%s: Failed allocate memory!\n",
					wan_drvname);
			wan_free(card);
			goto wanpipe_init_done;
		}
		if ((priv = wan_malloc(sizeof(struct wan_aften_priv))) == NULL){
			DEBUG_EVENT("%s: Failed to allocate priv structure!\n",
				wan_drvname);
			wan_free(card);
			wan_free(devle);
			goto wanpipe_init_done;
		}
		priv->common.is_netdev		= 1;
		priv->common.iface.open		= &wan_aften_open;
		priv->common.iface.close	= &wan_aften_close;
		priv->common.iface.ioctl	= &wan_aften_ioctl;

		priv->common.card		= card;
		wan_netif_set_priv(dev, priv);

#if defined(__LINUX__)
		/*sprintf(card->devname, "hdlc%d", if_index++);*/
		sprintf(card->devname, "wp%daft1", ++if_index);
#else
		sprintf(card->devname, "wp%caft1", 
			'a' + if_index++);

#endif
		/* Register in HDLC device */
		if (!wan_iface.attach || wan_iface.attach(dev, card->devname, 1)){
			wan_free(devle);
			if (wan_iface.free) wan_iface.free(dev);
			goto wanpipe_init_done;
		}

		wandev 			= &card->wandev;
		wandev->magic   	= ROUTER_MAGIC;
		wandev->name    	= card->devname;
		wandev->private 	= card;
		devle->dev		= dev;

		/* Set device pointer */
		WAN_LIST_INIT(&wandev->dev_head);
		WAN_LIST_INSERT_HEAD(&wandev->dev_head, devle, dev_link);
		card->list = NULL;
		if (card_list){
			for(tmp = card_list;tmp->list;tmp = tmp->list);
			tmp->list = card;
		}else{
			card_list = card;
		}
		//card->list	= card_list;
		//card_list	= card;
		if (wan_aften_setup(card, dev)){
			DEBUG_EVENT("%s: Failed setup new device!\n", 
					card->devname);
			WAN_LIST_REMOVE(devle, dev_link);
			wan_free(devle);
			if (wan_iface.detach) wan_iface.detach(dev, 1);
			if (wan_iface.free) wan_iface.free(dev);
			if (card_list == card){
				card_list = NULL;
			}else{
				for(tmp = card_list;
					tmp->list != card; tmp = tmp->list);
				tmp->list = card->list;
				card->list = NULL;
			}
			wan_free(card);
		}
	}
	wan_aften_update_ports();

wanpipe_init_done:
	if (err) wan_aften_exit(arg);
	return err;
}

static int wan_aften_exit(void *arg)
{
	struct wan_dev_le	*devle;
	sdla_t			*card;
	int			err = 0;

	for (card=card_list;card_list;){
		devle = WAN_LIST_FIRST(&card->wandev.dev_head);
		if (devle && devle->dev){
			struct wan_aften_priv	*priv = wan_netif_priv(devle->dev);
			DEBUG_EVENT("%s: Unregistering interface...\n", 
					wan_netif_name(devle->dev));
			if (wan_iface.detach) wan_iface.detach(devle->dev, 1);
			wan_free(priv);
			if (wan_iface.free) wan_iface.free(devle->dev);
			WAN_LIST_REMOVE(devle, dev_link);
			wan_free(devle);
		}
		DEBUG_EVENT("%s: Shutdown device\n", card->devname);
		wan_aften_shutdown(card);
		card_list = card->list;
		wan_free(card);
		card = card_list;
	}

	card_list=NULL;
#if defined(__OpenBSD__) || defined(__NetBSD__)
	sdladrv_exit();
#endif
	DEBUG_EVENT("\n");
	DEBUG_EVENT("%s Unloaded.\n", wan_fullname);

#if defined(WAN_DEBUG_MEM)
	DEBUG_EVENT("%s: Total Mem %d\n",
			wan_drvname, wan_atomic_read(&wan_debug_mem));
#endif	
	return err;
}

static int wan_aften_setup(sdla_t *card, netdevice_t *dev)
{
	struct wan_aften_priv	*priv = wan_netif_priv(dev);
	int			err;

	card->hw = sdla_register(&card->hw_iface, NULL, card->devname);
	if (card->hw == NULL){
		DEBUG_EVENT("%s: Failed to register hw device\n",
				card->devname);
		goto wan_aften_setup_error;
	}

	err = card->hw_iface.setup(card->hw, NULL);
	if (err){
		DEBUG_EVENT("%s: Hardware setup Failed %d\n",
				card->devname,err);
		sdla_unregister(&card->hw, card->devname);
		goto wan_aften_setup_error;
	}


	WAN_HWCALL(getcfg, (card->hw, SDLA_BUS, &priv->bus));
	WAN_HWCALL(getcfg, (card->hw, SDLA_SLOT, &priv->slot));
	WAN_HWCALL(getcfg, (card->hw, SDLA_IRQ, &priv->irq));
	WAN_HWCALL(pci_read_config_dword, 
			(card->hw, 0x04, &priv->base_class));
	WAN_HWCALL(pci_read_config_dword, 
			(card->hw, PCI_IO_BASE_DWORD, &priv->base_addr0));
	WAN_HWCALL(pci_read_config_dword, 
			(card->hw, PCI_MEM_BASE0_DWORD, &priv->base_addr1));

	DEBUG_TEST("%s: BaseClass %X BaseAddr 0x%X IRQ %d\n", 
			wan_netif_name(dev),
			priv->base_class,
			priv->base_addr0,
			priv->irq);
	return 0;

wan_aften_setup_error:
	return -EINVAL;

}

static int wan_aften_shutdown(sdla_t *card)
{
	
	if (card->hw_iface.down){
		card->hw_iface.down(card->hw);
	}
	sdla_unregister(&card->hw, card->devname);

	return 0;
}

static int wan_aften_update_ports(void)
{
	sdla_t		*card, *prev = NULL;
#if 0
	unsigned char	*hwprobe;
	int		err;
#endif
	
	DEBUG_TEST("\n\nList of available Sangoma devices:\n");
	for (card=card_list; card; card = card->list){
		netdevice_t	*dev;
		dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
		if (dev){
			if (prev && prev->hw == card->hw){
				card->wandev.comm_port = 
					prev->wandev.comm_port + 1;
			}
#if 0
			err = card->hw_iface.get_hwprobe(
						card->hw, 
						card->wandev.comm_port, 
						(void**)&hwprobe); 
			if (!err){
				hwprobe[strlen(hwprobe)] = '\0';
				DEBUG_EVENT("%s: %s\n",
						wan_netif_name(dev),
						hwprobe);
			}
#endif
		}
		prev = card;
	}
	return 0;
}


static int wan_aften_read_reg(sdla_t *card, wan_cmd_api_t *api_cmd, int idata)
{

	if (api_cmd->len == 1){
		if (api_cmd->offset <= 0x3C){
			card->hw_iface.pci_read_config_byte(
					card->hw,
					api_cmd->offset,
					(unsigned char*)&api_cmd->data[idata]);
		}else{
			card->hw_iface.bus_read_1(
					card->hw,
				       	api_cmd->offset,
			       		(unsigned char*)&api_cmd->data[idata]);
		}
#if defined(DEBUG_REG)
		DEBUG_EVENT("%s: Reading Off=0x%08X Len=%i Data=0x%02X\n",
				card->devname,
				api_cmd->offset,
				api_cmd->len,
				*(unsigned char*)&api_cmd->data[idata]);
#endif
	}else if (api_cmd->len == 2){
		if (api_cmd->offset <= 0x3C){
			card->hw_iface.pci_read_config_word(
					card->hw,
					api_cmd->offset,
					(unsigned short*)&api_cmd->data[idata]);
		}else{
			card->hw_iface.bus_read_2(
					card->hw,
			       		api_cmd->offset,
				       	(unsigned short*)&api_cmd->data[idata]);
		}
#if defined(DEBUG_REG)
		DEBUG_EVENT("%s: Reading Off=0x%08X Len=%i Data=0x%04X\n",
				card->devname,
				api_cmd->offset,
				api_cmd->len,
				*(unsigned short*)&api_cmd->data[idata]);
#endif
	}else if (api_cmd->len == 4){
		if (api_cmd->offset <= 0x3C){
			card->hw_iface.pci_read_config_dword(
					card->hw,
					api_cmd->offset,
					(unsigned int*)&api_cmd->data[idata]);
		}else{
			card->hw_iface.bus_read_4(
					card->hw,
				       	api_cmd->offset,
				       	(unsigned int*)&api_cmd->data[idata]);
		}
#if defined(DEBUG_REG)
		DEBUG_EVENT("ADEBUG: %s: Reading Off=0x%08X Len=%i Data=0x%04X (idata=%d)\n",
				card->devname,
				api_cmd->offset,
				api_cmd->len,
				*(unsigned int*)&api_cmd->data[idata],
				idata);
#endif
	}else{
		card->hw_iface.peek(
				card->hw,
				api_cmd->offset,
				(unsigned char*)&api_cmd->data[idata],
				api_cmd->len);
#if 0
		memcpy_fromio((unsigned char*)&api_cmd.data[0],
				(unsigned char*)vector, api_cmd.len);
#endif

#if defined(DEBUG_REG)
		DEBUG_EVENT("%s: Reading Off=0x%08X Len=%i\n",
				card->devname,
				api_cmd->offset,
				api_cmd->len);
#endif
	}
	
	return 0;	
}

static int wan_aften_all_read_reg(sdla_t *card_head, wan_cmd_api_t *api_cmd)
{
	sdla_t		*card, *prev = NULL;
	int 		idata = 0;
	
	for (card=card_list; card; card = card->list){
		//if (prev == NULL || prev->hw != card->hw){
		if (prev == NULL||!card->hw_iface.hw_same(prev->hw, card->hw)){
			wan_aften_read_reg(card, api_cmd, idata);
			idata += api_cmd->len;
		}
		prev = card;
	}

	return 0;	
}

static int wan_aften_write_reg(sdla_t *card, wan_cmd_api_t *api_cmd)
{
	if (api_cmd->len == 1){
		card->hw_iface.bus_write_1(
				card->hw,
				api_cmd->offset,
				*(unsigned char*)&api_cmd->data[0]);
#if defined(DEBUG_REG)
		DEBUG_EVENT("%s: Write  Offset=0x%08X Data=0x%02X\n",
			card->devname,api_cmd->offset,
			*(unsigned char*)&api_cmd->data[0]);
#endif
	}else if (api_cmd->len == 2){
		card->hw_iface.bus_write_2(
				card->hw,
				api_cmd->offset,
				*(unsigned short*)&api_cmd->data[0]);
#if defined(DEBUG_REG)
		DEBUG_EVENT("%s: Write  Offset=0x%08X Data=0x%04X\n",
			card->devname,api_cmd->offset,
			*(unsigned short*)&api_cmd->data[0]);
#endif
	}else if (api_cmd->len == 4){
		card->hw_iface.bus_write_4(
				card->hw,
				api_cmd->offset,
				*(unsigned int*)&api_cmd->data[0]);
#if defined(DEBUG_REG)
		DEBUG_EVENT("ADEBUG: %s: Write  Offset=0x%08X Data=0x%08X\n",
			card->devname,api_cmd->offset,
			*(unsigned int*)&api_cmd->data[0]);
#endif
	}else{
		card->hw_iface.poke(
				card->hw,
				api_cmd->offset,
				(unsigned char*)&api_cmd->data[0],
				api_cmd->len);
#if 0
		memcpy_toio((unsigned char*)vector,
			(unsigned char*)&api_cmd->data[0], api_cmd->len);
#endif
	}
	return 0;
}

static int wan_aften_all_write_reg(sdla_t *card_head, wan_cmd_api_t *api_cmd)
{
	sdla_t		*card, *prev = NULL;

	for (card=card_list; card; card = card->list){
		//if (prev == NULL || prev->hw != card->hw){
		if (prev == NULL||!card->hw_iface.hw_same(prev->hw, card->hw)){
			wan_aften_write_reg(card, api_cmd);
		}
		prev = card;
	}

	return 0;	
}


static int wan_aften_set_pci_bios(sdla_t *card)
{
	netdevice_t		*dev = NULL;
	struct wan_aften_priv	*priv;

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if (dev == NULL){
		return -EINVAL;
	}
	priv= wan_netif_priv(dev);
	DEBUG_TEST("%s: Set PCI BaseClass %X BaseAddr0 %X BaseAddr1 %X IRQ %d\n",
				wan_netif_name(dev),
			       	priv->base_class,
			       	priv->base_addr0,
			       	priv->base_addr1,
				priv->irq);

	WP_DELAY(200);
	card->hw_iface.pci_write_config_dword(
			card->hw, 0x04, priv->base_class);
	card->hw_iface.pci_write_config_dword(
			card->hw, PCI_IO_BASE_DWORD, priv->base_addr0);
	card->hw_iface.pci_write_config_dword(
			card->hw, PCI_MEM_BASE0_DWORD, priv->base_addr1);
	card->hw_iface.pci_write_config_byte(
			card->hw, PCI_INT_LINE_BYTE, priv->irq);
	return 0;
}

static int wan_aften_all_set_pci_bios(sdla_t *card_head)
{
	sdla_t		*card, *prev = NULL;

	for (card=card_list; card; card = card->list){
		if (prev == NULL||!card->hw_iface.hw_same(prev->hw, card->hw)){
		//if (prev == NULL || prev->hw != card->hw){
			if (wan_aften_set_pci_bios(card)){
				return -EINVAL;
			}
		}
		prev = card;
	}

	return 0;	
}

static int wan_aften_hwprobe(sdla_t *card, wan_cmd_api_t *api_cmd)
{
	netdevice_t	*dev;
	unsigned char	*str;
	int		err;

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if (dev && card->hw_iface.get_hwprobe && card->hw_iface.getcfg){
		char	ver;
		memcpy(&api_cmd->data[api_cmd->len], 
				wan_netif_name(dev), 
				strlen(wan_netif_name(dev)));
		api_cmd->len += strlen(wan_netif_name(dev));
		api_cmd->data[api_cmd->len++] = ':';
		api_cmd->data[api_cmd->len++] = ' ';
		err = card->hw_iface.get_hwprobe(
				card->hw, card->wandev.comm_port, (void**)&str); 
		if (err){
			return err;
		}
		str[strlen(str)] = '\0';
		memcpy(&api_cmd->data[api_cmd->len], str, strlen(str));
		api_cmd->len += strlen(str);	/* set to number of cards */
		api_cmd->data[api_cmd->len++] = ' ';
		api_cmd->data[api_cmd->len++] = '(';
		api_cmd->data[api_cmd->len++] = 'V';
		api_cmd->data[api_cmd->len++] = 'e';
		api_cmd->data[api_cmd->len++] = 'r';
		api_cmd->data[api_cmd->len++] = '.';
		err = card->hw_iface.getcfg(
				card->hw, SDLA_COREREV, &ver);
		if (err){
			return err;
		}
		sprintf(&api_cmd->data[api_cmd->len], "%02X", ver);
		api_cmd->len += 2;
		api_cmd->data[api_cmd->len++] = ')';
		api_cmd->data[api_cmd->len++] = '\n';
		DEBUG_TEST("%s: Read card hwprobe (%s)!\n",
					wan_netif_name(dev),
					api_cmd->data);
	}else{
		return -EINVAL;
	}
	return 0;
}

static int wan_aften_all_hwprobe(sdla_t *card_head, wan_cmd_api_t *api_cmd)
{
	sdla_t	*card, *prev=NULL;
#if 0
	int 	bus = -1, slot = -1;
#endif
	int	err = 0;

	for (card=card_list; card; card = card->list){
		if (prev == NULL||!card->hw_iface.hw_same(prev->hw, card->hw)){
			if ((err = wan_aften_hwprobe(card, api_cmd))){
				return err;
			}
		}
		prev = card;
#if 0
		netdevice_t		*dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
		struct wan_aften_priv	*priv = wan_netif_priv(dev);
		if (bus == -1 || 
		    !(bus == priv->bus && slot == priv->slot)){
			if ((err = wan_aften_hwprobe(card, api_cmd))){
				return err;
			}
		}
		bus = priv->bus;
		slot = priv->slot;
#endif
	}

	return 0;	
}

static int wan_aften_open(netdevice_t *dev)
{
	WAN_NETIF_START_QUEUE(dev);
	return 0;
}

static int wan_aften_close(netdevice_t *dev)
{
	WAN_NETIF_STOP_QUEUE(dev);
	return 0;
}

static int wan_aften_ioctl (netdevice_t *dev, struct ifreq *ifr, int cmd)
{
	sdla_t			*card;
	struct wan_aften_priv	*priv= wan_netif_priv(dev);
	wan_cmd_api_t		api_cmd;
	int			err=-EINVAL;
		
	if (!priv || !priv->common.card){
		DEBUG_EVENT("%s: Invalid structures!\n", wan_netif_name(dev));
		return -ENODEV;
	}

	DEBUG_TEST("%s: CMD=0x%X\n",__FUNCTION__,cmd);

	switch (cmd){
	case SIOC_WAN_DEVEL_IOCTL:
		break;
	default:
		DEBUG_IOCTL("%s: Unsupported IOCTL call!\n", 
					wan_netif_name(dev));
		return -EINVAL;
	}
	if (!ifr->ifr_data){
		DEBUG_EVENT("%s: No API data!\n", wan_netif_name(dev));
		return -EINVAL;
	}

	card = priv->common.card;
	if (WAN_COPY_FROM_USER(&api_cmd,ifr->ifr_data,sizeof(wan_cmd_api_t))){
		return -EFAULT;
	}

	/* Hardcode bar access FELD */
	switch (api_cmd.cmd){
	case SIOC_WAN_READ_REG:
		err = wan_aften_read_reg(card, &api_cmd, 0);
		break;
	
	case SIOC_WAN_ALL_READ_REG:
		err = wan_aften_all_read_reg(card_list, &api_cmd);
		break;

	case SIOC_WAN_WRITE_REG:
		err = wan_aften_write_reg(card, &api_cmd);
		break;

	case SIOC_WAN_ALL_WRITE_REG:
		err = wan_aften_all_write_reg(card_list, &api_cmd);
		break;
		
	case SIOC_WAN_SET_PCI_BIOS:
		err = wan_aften_set_pci_bios(card);
		break;

	case SIOC_WAN_ALL_SET_PCI_BIOS:
		err = wan_aften_all_set_pci_bios(card_list);
		break;
		
	case SIOC_WAN_HWPROBE:
		DEBUG_TEST("%s: Read Sangoma device hwprobe!\n",
					wan_netif_name(dev));
		memset(&api_cmd.data[0], 0, WAN_MAX_DATA_SIZE);
		api_cmd.len = 0;
		err = wan_aften_hwprobe(card, &api_cmd);
		break;
		
	case SIOC_WAN_ALL_HWPROBE:
		DEBUG_TEST("%s: Read list of Sangoma devices!\n",
					wan_netif_name(dev));
		memset(&api_cmd.data[0], 0, WAN_MAX_DATA_SIZE);
		api_cmd.len = 0;
		err = wan_aften_all_hwprobe(card, &api_cmd);
		break;

	case SIOC_WAN_COREREV:
		if (card->hw_iface.getcfg){
			err = card->hw_iface.getcfg(
					card->hw,
				       	SDLA_COREREV,
					&api_cmd.data[0]);
			api_cmd.len = 1;
		}
		DEBUG_TEST("%s: Get core revision (rev %X)!\n", 
				wan_netif_name(dev), api_cmd.data[0]);
		break;
	}

	if (WAN_COPY_TO_USER(ifr->ifr_data,&api_cmd,sizeof(wan_cmd_api_t))){
		return -EFAULT;
	}
	
	return err;
}

