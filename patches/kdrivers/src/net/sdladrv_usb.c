/*****************************************************************************
* sdladrv_usb.c 
* 		
* 		SDLA USB Support Interface
*
* Authors: 	Alex Feldman <alex@sangoma.com>
*
* Copyright:	(c) 2003-2006 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jan 30, 2008	Alex Feldman	Initial version.
*****************************************************************************/

/***************************************************************************
****		I N C L U D E  		F I L E S			****
***************************************************************************/
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe.h>
# include <wanpipe_abstr.h> 
# include <wanpipe_snmp.h> 
# include <if_wanpipe_common.h>    /* Socket Driver common area */
# include <sdlapci.h>
# include <aft_core.h>
# include <wanpipe_iface.h>
#else
# include <linux/wanpipe_includes.h>
# include <linux/tty.h>
# include <linux/usb.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe.h>
# include <linux/wanproc.h>
# include <linux/wanpipe_abstr.h>
# include <linux/if_wanpipe_common.h>    /* Socket Driver common area */
# include <linux/if_wanpipe.h>
# include <linux/sdlapci.h>
# include <linux/wanpipe_iface.h>
# include <linux/wanpipe_tdm_api.h>
#endif

#if defined(CONFIG_PRODUCT_WANPIPE_USB) 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
#  include <linux/usb/serial.h>
#else
#  define MAX_NUM_PORTS 8  //This is normally defined in /linux/usb/serial.h
#endif
/***************************************************************************
****                   M A C R O S     D E F I N E S                    ****
***************************************************************************/

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,19)
#define USB2420
#endif
#define LINUX26

#define WP_USB_RXTX_DATA_LEN		8
#define WP_USB_RXTX_DATA_COUNT		2	// Original 2
#define WP_USB_MAX_RW_COUNT		100

#define SDLA_USB_NAME			"sdlausb"

#define SDLA_USBFXO_VID			0x10C4
#define SDLA_USBFXO_PID			0x8461

#define SDLA_USBFXO_READ_DELAY		100
#define SDLA_USBFXO_WRITE_DELAY		10

/* Internal USB-FXO CPU registers */
#define SDLA_USB_CPU_REG_DEVICEID		0x00

#define SDLA_USB_CPU_REG_HARDWAREVER		0x01

#define SDLA_USB_CPU_REG_FIRMWAREVER		0x02

#define SDLA_USB_CPU_REG_CONRTOL		0x03
#define SDLA_USB_CPU_BIT_CONRTOL_TS1_EVENT_EN	0x02
#define SDLA_USB_CPU_BIT_CONRTOL_TS0_EVENT_EN	0x01

#define SDLA_USB_CPU_REG_FIFO_STATUS		0x04
#define SDLA_USB_CPU_BIT_FIFO_STATUS_TS1_TX_UF	0x80
#define SDLA_USB_CPU_BIT_FIFO_STATUS_TS1_TX_OF	0x40
#define SDLA_USB_CPU_BIT_FIFO_STATUS_TS0_TX_UF	0x20
#define SDLA_USB_CPU_BIT_FIFO_STATUS_TS0_TX_OF	0x10
#define SDLA_USB_CPU_BIT_FIFO_STATUS_TS1_RX_UF	0x08
#define SDLA_USB_CPU_BIT_FIFO_STATUS_TS1_RX_OF	0x04
#define SDLA_USB_CPU_BIT_FIFO_STATUS_TS0_RX_UF	0x02
#define SDLA_USB_CPU_BIT_FIFO_STATUS_TS0_RX_OF	0x01

#define SDLA_USB_CPU_REG_UART_STATUS		0x05
#define SDLA_USB_CPU_BIT_UART_STATUS_LOST_SYNC	0x10
#define SDLA_USB_CPU_BIT_UART_STATUS_CMD_UNKNOWN	0x10
#define SDLA_USB_CPU_BIT_UART_STATUS_RX_UF	0x08
#define SDLA_USB_CPU_BIT_UART_STATUS_RX_OF	0x04
#define SDLA_USB_CPU_BIT_UART_STATUS_TX_UF	0x02
#define SDLA_USB_CPU_BIT_UART_STATUS_TX_OF	0x01

#define SDLA_USB_CPU_REG_HOSTIF_STATUS		0x06
#define SDLA_USB_CPU_BIT_HOSTIF_STATUS_RX_UF	0x08
#define SDLA_USB_CPU_BIT_HOSTIF_STATUS_RX_OF	0x04
#define SDLA_USB_CPU_BIT_HOSTIF_STATUS_TX_UF	0x02
#define SDLA_USB_CPU_BIT_HOSTIF_STATUS_TX_OF	0x01

#define SDLA_USB_CPU_REG_LED_CONTROL		0x07
#define SDLA_USB_CPU_BIT_LED_CONTROL_TS1_GRN	0x08
#define SDLA_USB_CPU_BIT_LED_CONTROL_TS1_RED	0x04
#define SDLA_USB_CPU_BIT_LED_CONTROL_TS0_GRN	0x02
#define SDLA_USB_CPU_BIT_LED_CONTROL_TS0_RED	0x01

#define SDLA_USB_CPU_REG_DEBUG			0x08
#define SDLA_USB_CPU_BIT_DEBUG_WEN_ACK		0x08
#define SDLA_USB_CPU_BIT_DEBUG_DTMF		0x04
#define SDLA_USB_CPU_BIT_DEBUG_LOCAL_LB		0x02
#define SDLA_USB_CPU_BIT_DEBUG_LINE_LB		0x01


#define WP_USB_BAUD_RATE		500000

#define WP_USB_IDLE_PATTERN		0x7E
#define WP_USB_CTRL_IDLE_PATTERN	0x7E

#define WP_USB_MAX_RX_CMD_QLEN		20	// Original 10
#define WP_USB_MAX_TX_CMD_QLEN		20	

unsigned char readFXOcmd[]= { 0x70, 0xC1, 0xC8 , 0x88 , 0x12 , 0x03 , 0x48 , 0x08 , 0x14 , 0x05 , 0x88 , 0x08 , 0x56 , 0xC7 , 0xC8 , 0x88 , 0x58 , 0xC9 , 0xC8 , 0x88 , 0x5A , 0xCB , 0xC8 , 0x88 , 0x5C , 0xCD , 0xC8 , 0x88 , 0x5E , 0xCF , 0xC8 , 0x88 };

unsigned char idlebuf[]= 
	{
		0x77, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x77, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E,
		0x57, 0xCF, 0xCF, 0x8E
	};

#define WP_USB_STATUS_READY		1
#define WP_USB_STATUS_TX_READY		2
#define WP_USB_STATUS_RX_EVENT1_READY	3
#define WP_USB_STATUS_RX_EVENT2_READY	4
#define WP_USB_STATUS_RX_DATA_READY	5
#define WP_USB_STATUS_BH		6
#define WP_USB_STATUS_TX_CMD		7

#define WP_USB_CMD_TYPE_MASK		0x1C
#define WP_USB_CMD_TYPE_SHIFT		2
#define WP_USB_CMD_TYPE_READ_FXO	0x01
#define WP_USB_CMD_TYPE_WRITE_FXO	0x02
#define WP_USB_CMD_TYPE_EVENT		0x03
#define WP_USB_CMD_TYPE_READ_CPU	0x04
#define WP_USB_CMD_TYPE_WRITE_CPU	0x05
#define WP_USB_CMD_TYPE_DECODE(ctrl)	(((ctrl) & WP_USB_CMD_TYPE_MASK) >> WP_USB_CMD_TYPE_SHIFT)
#define WP_USB_CMD_TYPE_ENCODE(cmd)	((cmd) << WP_USB_CMD_TYPE_SHIFT)

#define WP_USB_CTRL_DECODE(buf)							\
		((buf)[0] & 0xC0) | (((buf)[1] & 0xC0) >> 2) | 			\
		(((buf)[2] & 0xC0) >> 4) | (((buf)[3] & 0xC0) >> 6)

#define WP_USB_CTRL_ENCODE(buf, ind)							\
	if (ind % 4 == 0 && ind % 32 == 0) *((buf)+ind) = (*((buf)+ind) & 0xCF) | 0x30;	\
	else if (ind % 4 == 0)  *((buf)+ind) = (*((buf)+ind) & 0xCF) | 0x10;

#define WP_USB_CTRL_ENCODE1(buf, ctrl)						\
	*(buf)     = (*(buf) & 0xCF)     | (ctrl);

#define WP_USB_DATA_ENCODE(buf, data)						\
	*(buf)     = (*(buf) & 0xF0)     | ((data) & 0x0F);			\
	*((buf)+2) = (*((buf)+2) & 0xF0) | (((data) >> 4) & 0x0F);
	
#define WP_USB_CMD_ENCODE(buf, cmd)						\
	*(buf)     = (*(buf) & 0x3F)     | ((cmd) & 0xC0);			\
	*((buf)+1) = (*((buf)+1) & 0x3F) | (((cmd) & 0x30) << 2);		\
	*((buf)+2) = (*((buf)+2) & 0x3F) | (((cmd) & 0x0C) << 4);		\
	*((buf)+3) = (*((buf)+3) & 0x3F) | (((cmd) & 0x03) << 6); 
#if 0
#define WP_USB_CTRL_ENCODE(buf, ctrl)						\
	*(buf) |= (ctrl) & 0xC0;						\
	*((buf)+1) |= ((ctrl) & 0x30) << 2;					\
	*((buf)+2) |= ((ctrl) & 0x0C) << 4;					\
	*((buf)+3) |= ((ctrl) & 0x03) << 6; 
#endif
/***************************************************************************
****               S T R U C T U R E S   T Y P E D E F S                ****
***************************************************************************/
struct sdla_usb_desc {
        char	*name;
        int	adptr_type;
};

/***************************************************************************
****               F U N C T I O N   P R O T O T Y P E S                ****
***************************************************************************/
extern sdlahw_t* sdla_find_adapter(wandev_conf_t* conf, char* devname);
extern sdlahw_cpu_t* sdla_hwcpu_search(u8, int, int, int, int);
extern sdlahw_t* sdla_hw_search(sdlahw_cpu_t*, int);
extern int sdla_hw_fe_test_and_set_bit(void *phw,int value);
extern int sdla_hw_fe_test_bit(void *phw,int value);
extern int sdla_hw_fe_set_bit(void *phw,int value);
extern int sdla_hw_fe_clear_bit(void *phw,int value);
extern int sdla_usb_create(struct usb_interface*, int);
extern int sdla_usb_remove(struct usb_interface*, int);

static int sdla_usb_probe(struct usb_interface*, const struct usb_device_id*);
static void sdla_usb_disconnect(struct usb_interface*);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10) 
static int sdla_usb_suspend (struct usb_interface*, pm_message_t);
#else
static int sdla_usb_suspend (struct usb_interface*, u32 msg);
#endif
static int sdla_usb_resume (struct usb_interface*);
#if 0
static void sdla_usb_prereset (struct usb_interface*);
static void sdla_usb_postreset (struct usb_interface*);
#endif

u_int8_t sdla_usb_fxo_read(void *phw, ...);
int sdla_usb_fxo_write(void *phw, ...);
int sdla_usb_cpu_read(void *phw, unsigned char off, unsigned char *data);
int sdla_usb_cpu_write(void *phw, unsigned char off, unsigned char data);
int sdla_usb_write_poll(void *phw, unsigned char off, unsigned char data);
int sdla_usb_read_poll(void *phw, unsigned char off, unsigned char *data);
int sdla_usb_rxevent_enable(void *phw, int mod_no, int enable);
int sdla_usb_rxevent(void *phw, int mod_no, u8 *regs, int);
int sdla_usb_rxtx_data_init(void *phw, int, unsigned char **, unsigned char **);
int sdla_usb_rxdata_enable(void *phw, int enable);
int sdla_usb_rxdata(void *phw, unsigned char*, int);
int sdla_usb_txdata(void *phw, unsigned char*, int);
int sdla_usb_txdata_ready(void *phw);
int sdla_usb_set_intrhand(void* phw, wan_pci_ifunc_t *isr_func, void* arg, int notused);
int sdla_usb_restore_intrhand(void* phw, int notused);
int sdla_usb_err_stats(void*,void*,int);
//int sdla_usb_rxdata_ready(void *phw, int mod_no);
//netskb_t* sdla_usb_rxdata(void *phw, int mod_no);
//int sdla_usb_txdata(void *phw, netskb_t*, in t mod_no);

#if defined(__LINUX__)
static void 	sdla_usb_bh (unsigned long);
#else
static void 	sdla_usb_bh (void*,int);
#endif
static int wp_usb_start_transfer(struct wan_urb*);

/***************************************************************************
****                   G L O B A L   V A R I A B L E                    ****
***************************************************************************/
static struct sdla_usb_desc sdlausb = { "Sangoma Wanpipe USB-FXO 2 Interfaces", U100_ADPTR };

static struct usb_device_id sdla_usb_dev_ids[] = {
          /* This needs to be a USB audio device, and it needs to be made by us and have the right device ID */
	{ 
		match_flags:		(USB_DEVICE_ID_MATCH_VENDOR|USB_DEVICE_ID_MATCH_DEVICE),
		bInterfaceClass:	USB_CLASS_AUDIO,
		bInterfaceSubClass:	1,
		idVendor:		SDLA_USBFXO_VID,
		idProduct:		SDLA_USBFXO_PID,
		driver_info:		(unsigned long)&sdlausb,
	},
	{ }     /* Terminating Entry */
};

static struct usb_driver sdla_usb_driver =
{
	name:		SDLA_USB_NAME,
	probe:		sdla_usb_probe,
	disconnect:	sdla_usb_disconnect,
	suspend:	sdla_usb_suspend,
	resume:		sdla_usb_resume,
#if 0
	pre_reset:	sdla_usb_prereset,
	post_reset:	sdla_usb_postreset,
#endif
	id_table:	sdla_usb_dev_ids,
};

int gl_usb_rw_fast = 0;
EXPORT_SYMBOL(gl_usb_rw_fast);

static const unsigned char MULAW_1[992] = {
22,44,153,136,137,163,23,10,14,44,167,161,199,45,71,158,142,146,223,15,6,13,65,147,141,155,
90,43,98,167,163,55,16,10,21,170,139,135,150,53,19,23,52,183,231,32,24,40,157,137,137,
158,27,9,12,36,168,156,175,55,59,166,144,146,189,18,7,11,49,148,140,150,206,40,54,178,
165,74,21,11,20,181,140,134,145,67,18,19,40,188,190,43,28,37,163,139,136,154,30,10,10,
29,171,153,164,75,54,176,149,147,176,22,7,10,40,150,139,145,185,38,40,206,169,226,26,13,
18,202,141,134,142,235,18,15,31,200,178,60,31,35,172,141,137,151,36,10,8,25,176,150,157,
207,52,197,155,149,170,26,9,9,33,152,138,142,172,38,31,66,174,195,31,15,18,87,143,134,
140,190,19,13,26,251,172,223,40,36,184,144,138,149,44,11,7,20,186,148,152,181,54,88,161,
152,166,30,10,9,29,156,138,140,163,40,27,45,184,186,40,18,18,59,146,134,139,175,20,12,
21,68,169,183,51,37,205,149,139,147,56,12,6,16,206,147,147,169,59,59,172,155,163,38,12,
9,25,159,138,138,157,43,24,34,209,182,50,22,20,47,150,135,138,167,22,11,16,51,168,169,
78,41,90,154,140,145,82,14,6,14,82,147,143,159,71,48,190,159,162,46,14,9,23,166,138,
136,153,48,21,28,72,182,72,27,22,42,154,136,137,160,25,10,13,41,168,160,194,46,65,159,
142,145,203,16,6,12,57,148,141,154,121,43,86,168,162,60,17,10,20,174,139,135,148,59,20,
23,49,184,215,34,24,38,159,138,136,156,28,10,11,33,170,156,173,57,57,168,145,145,183,19,
7,11,45,149,140,149,197,40,52,180,165,86,22,11,18,188,140,134,144,81,19,18,38,190,188,
44,28,35,166,140,136,153,32,10,9,28,173,153,163,81,53,179,150,147,173,23,8,10,37,151,
139,144,179,39,40,216,169,209,27,13,17,223,142,134,142,206,19,15,30,207,177,62,32,34,175,
142,137,150,40,11,8,23,181,150,156,200,53,203,155,149,168,28,9,9,30,154,138,141,169,40,
31,62,174,191,32,15,17,70,144,134,140,183,20,13,25,92,172,215,41,35,189,145,137,147,48,
12,7,19,192,149,151,177,55,79,162,152,164,32,11,9,27,157,138,139,161,42,27,44,187,185,
41,18,18,53,147,134,138,172,21,12,20,62,170,180,54,37,220,150,139,145,64,13,6,15,236,
148,146,167,61,57,173,155,161,41,13,9,24,162,138,137,156,45,24,33,221,182,53,23,19,44,
152,135,137,164,24,11,15,47,169,169,89,41,78,155,140,144,243,14,6,13,67,148,143,158,77,
47,194,160,161,49,15,9,21,170,139,136,151,53,22,27,66,182,78,28,21,39,156,137,136,158,
27,10,13,39,170,160,190,46,62,160,142,144,191,17,6,12,50,149,141,153,223,43,77,169,162,
65,18,10,19,179,140,135,147,67,20,22,47,185,206,35,24,36,161,138,136,154,30,10,11,31,
172,156,172,59,56,170,146,145,177,21,7,10,41,151,140,148,190,40,49,183,164,113,23,11,17,
198,141,134,143,254,20,17,37,193,186,46,28,34,169,140,136,151,36,10,9,26,176,153,161,89,
52,183,151,146,170,25,8,9,33,153,139,143,175,40,39,226,169,201,28,13,16,93,142,134,141,
192,20,14,29,217,176,67,33,33,179,142,137,148,43,11,8,22,186,151,155,195,53,211,156,149,
165,29,9,9,29,156,138,141,167,41,31,59,175,188,34,15,16,60,145,134,139,176,21,13,24,
79,172,206,42,34,197,146,138,146,55,12,7,17,206,149,150,175,56,74,163,152,162,35,11,8,
25,159,138,139,159,44,27,42,188,183,43,19,17,47,149,135,138,168,23,12,19,57,170,179,55,
37,118,151,139,144,77,14,6,14,86,149,146,165,64,56,174,155,160,44,13,9,22,165,138,137,
154,48,24,31,236,181,57,24,19,41,153,136,137,160,25,11,15,44,170,168,101,41,71,156,141,
143,206,15,6,13,59,149,143,157,86,47,199,160,160,55,15,9,19,173,139,136,150,58,22,26,
62,182,87,29,21,37,157,137,136,156,28,10,12,36,171,159,187,47,59,162,143,143,184,19,6,
11,45,150,141,152,207,43,72,170,161,75,19,10,17,186,140,135,145,78,21,21,45,187,202,36,
24,33,164,139,136,152,32,10,10,29,174,156,170,61,54,172/*,146,144,173,22,7,10,37,152*/
};


/***************************************************************************
****               F U N C T I O N   D E F I N I T I N S                ****
***************************************************************************/

/***************************************************************************
***************************************************************************/
/*****************************************************************************
*****************************************************************************/
int sdla_usb_init(void)
{
	int	ret;

	/* USB-FXO */
	ret = usb_register(&sdla_usb_driver);
	if (ret){
		DEBUG_EVENT("%s: Failed to register Sangoma USB driver!\n",
				SDLA_USB_NAME);
		return ret;
	}
	return 0;
}

int sdla_usb_exit(void)
{
	usb_deregister(&sdla_usb_driver);
	return 0;
}


static int sdla_usb_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device	*udev = interface_to_usbdev(intf);
	struct sdla_usb_desc 	*desc = (struct sdla_usb_desc *)id->driver_info;

	DEBUG_EVENT("%s: Probing %s (%X) on %d ...\n",
				SDLA_USB_NAME, desc->name, desc->adptr_type, udev->devnum);

	if (sdla_usb_create(intf, desc->adptr_type)){
		DEBUG_ERROR("ERROR: %s: Failed to creae hwcard structures\n",
				SDLA_USB_NAME);
		return -ENODEV;
	}
	return 0;
}

static void sdla_usb_disconnect(struct usb_interface *intf)
{
	struct usb_device	*dev = interface_to_usbdev(intf);
	sdlahw_card_t		*hwcard = NULL;
	int			force = 0;

	DEBUG_EVENT("%s: Disconnect USB from %d ...\n", 
				SDLA_USB_NAME, dev->devnum);
	if ((hwcard = usb_get_intfdata(intf)) != NULL){
		DEBUG_EVENT("WARNING: Force to remove usb device!\n");
		wan_clear_bit(WP_USB_STATUS_READY, &hwcard->u_usb.status);
		force = 1;
	}

	sdla_usb_remove(intf, force);
	return;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
static int sdla_usb_suspend (struct usb_interface *intf, pm_message_t message)
{
	struct usb_device	*dev = interface_to_usbdev(intf);
	DEBUG_EVENT("%s: Suspend USB device on %d (not implemented)!\n", 
				SDLA_USB_NAME, dev->devnum);
	return 0;
}
#else
static int sdla_usb_suspend (struct usb_interface *intf, u32 msg)
{
	struct usb_device	*dev = interface_to_usbdev(intf);
	DEBUG_EVENT("%s: Suspend USB device on %d (not implemented)!\n", 
				SDLA_USB_NAME, dev->devnum);
	return 0;
}
#endif

static int sdla_usb_resume (struct usb_interface *intf)
{
	struct usb_device	*dev = interface_to_usbdev(intf);
	DEBUG_EVENT("%s: Resume USB device on %d (not implemented)!\n", 
				SDLA_USB_NAME, dev->devnum);
	return 0;
}

#if 0
static void sdla_usb_prereset (struct usb_interface *intf)
{
	struct usb_device	*dev = interface_to_usbdev(intf);
	DEBUG_EVENT("%s: Pre-reset USB device on %d (not implemented)!\n",
				SDLA_USB_NAME, dev->devnum);
}


static void sdla_usb_postreset (struct usb_interface *intf)
{
	struct usb_device	*dev = interface_to_usbdev(intf);
	DEBUG_EVENT("%s: Post-Reset USB device on %d (not implemented)!\n",
				SDLA_USB_NAME, dev->devnum);
}
#endif

/***************************************************************************
***************************************************************************/
static void wait_just_a_bit(int foo, int fast)
{

#if defined(__FreeBSD__) || defined(__OpenBSD__)
	WP_SCHEDULE(foo, "A-USB");
#else
	wan_ticks_t	start_ticks;
	start_ticks = SYSTEM_TICKS + foo;
	while(SYSTEM_TICKS < start_ticks){
		WP_DELAY(1);
# if defined(__LINUX__)
		if (!fast) WP_SCHEDULE(foo, "A-USB");
# endif
	}
#endif
}

static u_int8_t __sdla_usb_fxo_read(void *phw, int mod_no, unsigned char off)
{ 
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	netskb_t	*skb;
	u8		data = 0xFF, *cmd_data;
	wan_smp_flag_t	flags;

	WAN_ASSERT_RC(hw == NULL, 0xFF);
	SDLA_MAGIC(hw);
	WAN_ASSERT_RC(hw->hwcpu == NULL, 0xFF);
	WAN_ASSERT_RC(hw->hwcpu->hwcard == NULL, 0xFF);
	hwcard = hw->hwcpu->hwcard;

	wan_spin_lock(&hwcard->u_usb.cmd_lock,&flags);
	DEBUG_TX("%s: Tx Read FXO register (%02X:%d)!\n", 
			hw->devname,
			WP_USB_CMD_TYPE_ENCODE(WP_USB_CMD_TYPE_READ_FXO) | mod_no, 
			(unsigned char)off);
	if (wan_test_and_set_bit(WP_USB_STATUS_TX_CMD, &hwcard->u_usb.status)){
		DEBUG_USB("%s: WARNING: USB FXO Read Command Overrun (Read command in process)!\n",
					hw->devname);
		hwcard->u_usb.stats.cmd_overrun++;
		goto fxo_read_done;
	}
	
	if (!wan_skb_queue_len(&hwcard->u_usb.tx_cmd_free_list)){
		DEBUG_USB("%s: WARNING: USB FXO Read Command Overrun (%d commands in process)!\n",
					hw->devname, wan_skb_queue_len(&hwcard->u_usb.tx_cmd_list));
		hwcard->u_usb.stats.cmd_overrun++;
		goto fxo_read_done;
	}	
	skb = wan_skb_dequeue(&hwcard->u_usb.tx_cmd_free_list);
	cmd_data = wan_skb_put(skb, 2);
	cmd_data[0] = WP_USB_CMD_TYPE_ENCODE(WP_USB_CMD_TYPE_READ_FXO) | mod_no;
	cmd_data[1] = off;
	wan_skb_queue_tail(&hwcard->u_usb.tx_cmd_list, skb);

	wait_just_a_bit(SDLA_USBFXO_READ_DELAY, gl_usb_rw_fast);	//WP_DELAY(10000);

	if (!wan_skb_queue_len(&hwcard->u_usb.rx_cmd_list)){
		DEBUG_USB("%s: WARNING: Timeout on Read USB-FXO Reg!\n",
				hw->devname);
		hwcard->u_usb.stats.cmd_timeout++;
		goto fxo_read_done;
	}	
	skb = wan_skb_dequeue(&hwcard->u_usb.rx_cmd_list);
	cmd_data = wan_skb_data(skb);
	if (cmd_data[1] != off){
		DEBUG_USB("%s: USB FXO Read response is out of order (%02X:%02X)!\n",
				hw->devname, cmd_data[1], off);
		hwcard->u_usb.stats.cmd_invalid++;
		goto fxo_read_done;
	}
	data = (unsigned char)cmd_data[2];
	wan_skb_init(skb, 0);
	wan_skb_queue_tail(&hwcard->u_usb.rx_cmd_free_list, skb);

fxo_read_done:
	wan_clear_bit(WP_USB_STATUS_TX_CMD, &hwcard->u_usb.status);
	wan_spin_unlock(&hwcard->u_usb.cmd_lock,&flags);
	return data;
}

u_int8_t sdla_usb_fxo_read(void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	int 		mod_no, off, data;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	if (sdla_hw_fe_test_and_set_bit(hw,0)){
		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT(
			"%s: %s:%d: Critical Error: Re-entry in FE!\n",
					hw->devname,
					__FUNCTION__,__LINE__);
		}
		return -EINVAL;
	}
	va_start(args, phw);
	mod_no = va_arg(args, int);
	off	= va_arg(args, int);
	data	= va_arg(args, int);
	va_end(args);

	data = __sdla_usb_fxo_read(hw, mod_no, (unsigned char)off);
	sdla_hw_fe_clear_bit(hw,0);
	return data;
}

static int __sdla_usb_fxo_write(void *phw, int mod_no, unsigned char off, unsigned char data)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	netskb_t	*skb;
	u8		*cmd_data;
	wan_smp_flag_t	flags; 

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	wan_spin_lock(&hwcard->u_usb.cmd_lock,&flags);
	DEBUG_TX("%s: Tx Write FXO register (%02X: %d <- 0x%02X)!\n", 
			hw->devname, 
			WP_USB_CMD_TYPE_ENCODE(WP_USB_CMD_TYPE_WRITE_FXO) | mod_no, 
			(unsigned char)off, (unsigned char)data);
	if (!wan_skb_queue_len(&hwcard->u_usb.tx_cmd_free_list)){
		DEBUG_USB("%s: WARNING: USB FXO Write Command Overrun (%d commands in process)!\n",
					hw->devname, wan_skb_queue_len(&hwcard->u_usb.tx_cmd_list));
		hwcard->u_usb.stats.cmd_overrun++;
		wan_spin_unlock(&hwcard->u_usb.cmd_lock,&flags);
		return 0;
	}	
	skb = wan_skb_dequeue(&hwcard->u_usb.tx_cmd_free_list);
	cmd_data = wan_skb_put(skb, 3);
	cmd_data[0] = WP_USB_CMD_TYPE_ENCODE(WP_USB_CMD_TYPE_WRITE_FXO) | mod_no;
	cmd_data[1] = off; 
	cmd_data[2] = data; 
	wan_skb_queue_tail(&hwcard->u_usb.tx_cmd_list, skb);

	/* update mirror registers */
	hwcard->u_usb.regs[mod_no][off]  = data;

	wan_spin_unlock(&hwcard->u_usb.cmd_lock,&flags);
	return 0;
}
int sdla_usb_fxo_write(void *phw, ...)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	va_list		args;
	int 		mod_no, off, err, data;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	if (sdla_hw_fe_test_and_set_bit(hw,0)){
		if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT(
			"%s: %s:%d: Critical Error: Re-entry in FE!\n",
					hw->devname,
					__FUNCTION__,__LINE__);
		}
		return -EINVAL;
	}
	va_start(args, phw);
	mod_no = va_arg(args, int);
	off	= va_arg(args, int);
	data	= va_arg(args, int);
	va_end(args);

	err = __sdla_usb_fxo_write(hw, mod_no, (unsigned char)off, (unsigned char)data);
	sdla_hw_fe_clear_bit(hw,0);
	return err;
}

int sdla_usb_cpu_read(void *phw, unsigned char off, unsigned char *data)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	netskb_t 	*skb;
	u8		*cmd_data;
	wan_smp_flag_t	flags; 

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);

	hwcard = hw->hwcpu->hwcard;
	*data = 0xFF;
	if (!wan_test_bit(WP_USB_STATUS_READY, &hwcard->u_usb.status)){

	}
	wan_spin_lock(&hwcard->u_usb.cmd_lock,&flags);
	DEBUG_TX("%s: Tx Read CPU register (0x%02X:ticks=%ld)!\n", 
			hw->devname,(unsigned char)off,SYSTEM_TICKS);
	if (wan_test_and_set_bit(WP_USB_STATUS_TX_CMD, &hwcard->u_usb.status)){
		DEBUG_USB("%s: WARNING: USB CPU Read Command Overrun (Read command in process)!\n",
					hw->devname);
		hwcard->u_usb.stats.cmd_overrun++;
		goto cpu_read_done;
	}
	
	if (!wan_skb_queue_len(&hwcard->u_usb.tx_cmd_free_list)){
		DEBUG_USB("%s: WARNING: USB CPU Read Command Overrun (%d commands in process)!\n",
					hw->devname, wan_skb_queue_len(&hwcard->u_usb.tx_cmd_list));
		hwcard->u_usb.stats.cmd_overrun++;
		goto cpu_read_done;
	}	
	skb = wan_skb_dequeue(&hwcard->u_usb.tx_cmd_free_list);
	cmd_data = wan_skb_put(skb, 2);
	cmd_data[0] = WP_USB_CMD_TYPE_ENCODE(WP_USB_CMD_TYPE_READ_CPU);
	cmd_data[1] = off;
	wan_skb_queue_tail(&hwcard->u_usb.tx_cmd_list, skb);

	//WP_DELAY(20000);
	wait_just_a_bit(SDLA_USBFXO_READ_DELAY, gl_usb_rw_fast);	//WP_DELAY(10000);

	if (!wan_skb_queue_len(&hwcard->u_usb.rx_cmd_list)){
		DEBUG_USB("WARNING: %s: Timeout on Read USB-CPU Reg!\n",
				hw->devname);
		hwcard->u_usb.stats.cmd_timeout++;
		goto cpu_read_done;
	}	
	skb = wan_skb_dequeue(&hwcard->u_usb.rx_cmd_list);
	cmd_data = wan_skb_data(skb);
	if (cmd_data[1] != off){
		DEBUG_USB("%s: USB Read response is out of order (%02X:%02X)!\n",
				hw->devname, cmd_data[1], off);
		hwcard->u_usb.stats.cmd_invalid++;
		wan_skb_init(skb, 0);
		wan_skb_queue_tail(&hwcard->u_usb.rx_cmd_free_list, skb);
		goto cpu_read_done;
	}
	*data = (unsigned char)cmd_data[2];
	wan_skb_init(skb, 0);
	wan_skb_queue_tail(&hwcard->u_usb.rx_cmd_free_list, skb);

cpu_read_done:
	wan_clear_bit(WP_USB_STATUS_TX_CMD, &hwcard->u_usb.status);
	wan_spin_unlock(&hwcard->u_usb.cmd_lock,&flags);
	return 0;
}

int sdla_usb_cpu_write(void *phw, unsigned char off, unsigned char data)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	netskb_t	*skb;
	u8		*cmd_data;
	wan_smp_flag_t	flags; 

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	wan_spin_lock(&hwcard->u_usb.cmd_lock,&flags);

	DEBUG_TX("%s: Tx Write CPU register (0x%02X <- 0x%02X)!\n", 
			hw->devname, (unsigned char)off, (unsigned char)data);
	if (!wan_skb_queue_len(&hwcard->u_usb.tx_cmd_free_list)){
		DEBUG_USB("%s: WARNING: USB CPU Write Command Overrun (%d commands in process)!\n",
					hw->devname, wan_skb_queue_len(&hwcard->u_usb.tx_cmd_list));
		hwcard->u_usb.stats.cmd_overrun++;
		wan_spin_unlock(&hwcard->u_usb.cmd_lock,&flags);
		return 0;
	}	
	skb = wan_skb_dequeue(&hwcard->u_usb.tx_cmd_free_list);
	cmd_data = wan_skb_put(skb, 3);
	cmd_data[0] = WP_USB_CMD_TYPE_ENCODE(WP_USB_CMD_TYPE_WRITE_CPU);
	cmd_data[1] = off; 
	cmd_data[2] = data; 
	wan_skb_queue_tail(&hwcard->u_usb.tx_cmd_list, skb);

	wan_spin_unlock(&hwcard->u_usb.cmd_lock,&flags);
	return 0;
}

int sdla_usb_write_poll(void *phw, unsigned char off, unsigned char data)
{
	return 0;
}

int sdla_usb_read_poll(void *phw, unsigned char off, unsigned char *data)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	netskb_t	*skb;
	u8		*cmd_data;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	if (!wan_skb_queue_len(&hwcard->u_usb.rx_cmd_list)){
		DEBUG_EVENT("WARNING: %s: Timeout on Read USB Reg!\n",
					hw->devname);
		return -EBUSY;
	}	
	skb = wan_skb_dequeue(&hwcard->u_usb.rx_cmd_list);
	cmd_data = wan_skb_data(skb);
	*data = (unsigned char)cmd_data[2];
	wan_skb_init(skb, 0);
	wan_skb_queue_tail(&hwcard->u_usb.rx_cmd_free_list, skb);
	return 0;
}

int sdla_usb_rxevent_enable(void *phw, int mod_no, int enable)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	int		event_bit;
	u8		mask, data;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	if (sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_CONRTOL, &data)){
		return -EBUSY;
	}		
	if (mod_no) mask = SDLA_USB_CPU_BIT_CONRTOL_TS1_EVENT_EN;
	else mask = SDLA_USB_CPU_BIT_CONRTOL_TS0_EVENT_EN;
	if (enable) data |= mask;
	else data &= ~mask; 
	sdla_usb_cpu_write(hw, SDLA_USB_CPU_REG_CONRTOL, data);

	event_bit = (mod_no) ? WP_USB_STATUS_RX_EVENT2_READY : WP_USB_STATUS_RX_EVENT1_READY;
	wan_set_bit(event_bit, &hwcard->u_usb.status);
	DEBUG_USB("%s: Module %d: %s RX Events (%02X:%ld)!\n",
				hw->devname, mod_no+1, 
				(enable) ? "Enable" : "Disable", data,
				(unsigned long)SYSTEM_TICKS);
	return 0;
}

int sdla_usb_rxevent(void *phw, int mod_no, u8 *regs, int force)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	int		event_bit;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	event_bit = (mod_no) ? WP_USB_STATUS_RX_EVENT2_READY : WP_USB_STATUS_RX_EVENT1_READY;
	if(!wan_test_bit(event_bit, &hwcard->u_usb.status)){
		return -EINVAL;
	}
	if (force){
		hwcard->u_usb.regs[mod_no][5]  = __sdla_usb_fxo_read(hw, mod_no, 5);
		hwcard->u_usb.regs[mod_no][29] = __sdla_usb_fxo_read(hw, mod_no, 29);
		hwcard->u_usb.regs[mod_no][34] = __sdla_usb_fxo_read(hw, mod_no, 34);
		hwcard->u_usb.regs[mod_no][4]  = __sdla_usb_fxo_read(hw, mod_no, 4);
		DEBUG_USB("%s: Module %d: RX Event Init (%02X:%02X:%02X:%02X)...\n",
				hw->devname, mod_no+1,
				hwcard->u_usb.regs[mod_no][5], hwcard->u_usb.regs[mod_no][29],
				hwcard->u_usb.regs[mod_no][34], hwcard->u_usb.regs[mod_no][4]);
	}
	regs[0] = hwcard->u_usb.regs[mod_no][5];
	regs[1] = hwcard->u_usb.regs[mod_no][29];
	regs[2] = hwcard->u_usb.regs[mod_no][34];
	regs[3] = hwcard->u_usb.regs[mod_no][4];
	return 0;
}

int sdla_usb_rxtx_data_init(void *phw, int mod_no, unsigned char ** rxdata, unsigned char **txdata)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	*rxdata = &hwcard->u_usb.readchunk[mod_no][0];
	*txdata = &hwcard->u_usb.writechunk[mod_no][0];
	return 0;
}

int sdla_usb_rxdata_enable(void *phw, int enable)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	if (enable){
		wan_set_bit(WP_USB_STATUS_RX_DATA_READY, &hwcard->u_usb.status);
	}else{
		wan_clear_bit(WP_USB_STATUS_RX_DATA_READY, &hwcard->u_usb.status);
	}
	return 0;
}

int sdla_usb_rxdata(void *phw, unsigned char *data, int len)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	int		rx_len, rx_ind;
 
	WAN_ASSERT_RC(hw == NULL, 0);
	SDLA_MAGIC_RC(hw, 0);
	WAN_ASSERT_RC(hw->hwcpu == NULL, 0);
	WAN_ASSERT_RC(hw->hwcpu->hwcard == NULL, 0);
	hwcard = hw->hwcpu->hwcard;

	rx_ind = hwcard->u_usb.next_rx_ind;
	if (rx_ind > hwcard->u_usb.next_read_ind){
		rx_len = rx_ind - hwcard->u_usb.next_read_ind; 
	}else if (rx_ind < hwcard->u_usb.next_read_ind){
		rx_len = MAX_READ_BUF_LEN - hwcard->u_usb.next_read_ind + rx_ind;
	}else{
		DEBUG_USB("%s: No data available!\n",
					hw->devname);
		return 0;
	}
	if (rx_len < len){
		DEBUG_USB("%s: No enought data received (%d:%d)\n",
					hw->devname, rx_len, len);
		return 0;
	}

	memcpy(data, &hwcard->u_usb.readbuf[hwcard->u_usb.next_read_ind], len);
	hwcard->u_usb.next_read_ind += len;
	return len;
}

int sdla_usb_txdata(void *phw, unsigned char *data, int len)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	int		urb_write_ind;
 
	WAN_ASSERT_RC(hw == NULL, 0);
	SDLA_MAGIC_RC(hw, 0);
	WAN_ASSERT_RC(hw->hwcpu == NULL, 0);
	WAN_ASSERT_RC(hw->hwcpu->hwcard == NULL, 0);
	hwcard = hw->hwcpu->hwcard;

	if (len > MAX_USB_TX_LEN){
		len = MAX_USB_TX_LEN;
	}
	memcpy(&hwcard->u_usb.writebuf[hwcard->u_usb.next_tx_ind], data, len);

	/* Mark this urb as busy */
	urb_write_ind = (hwcard->u_usb.urb_write_ind + 1) % MAX_WRITE_URB_COUNT;
	hwcard->u_usb.datawrite[urb_write_ind].urb.transfer_buffer = &hwcard->u_usb.writebuf[hwcard->u_usb.next_tx_ind];
	hwcard->u_usb.datawrite[urb_write_ind].urb.transfer_buffer_length = len;
	hwcard->u_usb.next_tx_ind += len;
	hwcard->u_usb.next_tx_ind = hwcard->u_usb.next_tx_ind % MAX_WRITE_BUF_LEN;
	wan_clear_bit(WP_USB_STATUS_TX_READY, &hwcard->u_usb.status);
	if (wp_usb_start_transfer(&hwcard->u_usb.datawrite[urb_write_ind])){
		DEBUG_TX("%s: Failed to execute write cycle (%d:%d)\n",
 				hw->devname,
				hwcard->u_usb.next_tx_ind, hwcard->u_usb.next_write_ind);
		hwcard->u_usb.stats.tx_notready_cnt++;
		return 0;
	}else{
		hwcard->u_usb.urb_write_ind = urb_write_ind;
	}	

	return len;
}

int sdla_usb_txdata_ready(void *phw)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;
	int		urb_write_ind;
 
	WAN_ASSERT_RC(hw == NULL, -EBUSY);
	SDLA_MAGIC_RC(hw, -EBUSY);
	WAN_ASSERT_RC(hw->hwcpu == NULL, -EBUSY);
	WAN_ASSERT_RC(hw->hwcpu->hwcard == NULL, -EBUSY);
	hwcard = hw->hwcpu->hwcard;

	urb_write_ind = (hwcard->u_usb.urb_write_ind + 1) % MAX_WRITE_URB_COUNT;
	if (!wan_test_bit(1, &hwcard->u_usb.datawrite[urb_write_ind].ready)){
		DEBUG_TX("%s:%d: Tx Data is busy!\n",
				hw->devname, hwcard->u_usb.datawrite[urb_write_ind].id,
				(unsigned long)SYSTEM_TICKS);
		return -EBUSY;
	}
	if (!wan_test_bit(WP_USB_STATUS_TX_READY, &hwcard->u_usb.status)){
		DEBUG_TX("%s:%d: Tx Data is busy (%ld)!\n",
				hw->devname, hwcard->u_usb.datawrite[urb_write_ind].id,
				(unsigned long)SYSTEM_TICKS);
		return -EBUSY;
	}
	return 0;
}

int sdla_usb_set_intrhand(void* phw, wan_pci_ifunc_t *isr_func, void* arg, int notused)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	hwcard->u_usb.isr_func	= isr_func;
	hwcard->u_usb.isr_arg	= arg;
	return 0;
}

int sdla_usb_restore_intrhand(void* phw, int notused)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	hwcard->u_usb.isr_func	= NULL;
	hwcard->u_usb.isr_arg	= NULL;
	return 0;
}

int sdla_usb_err_stats(void *phw,void *buf,int len)
{
	sdlahw_t			*hw = (sdlahw_t*)phw;
	sdlahw_card_t			*hwcard = NULL;
	sdla_usb_comm_err_stats_t	*stats;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	if (len != sizeof(sdla_usb_comm_err_stats_t)){
		DEBUG_EVENT("%s: Invalid stats structure (%d:%d)!\n",
				hw->devname, len, sizeof(sdla_usb_comm_err_stats_t)); 
		return -EINVAL;
	}
	stats = &hwcard->u_usb.stats;
	sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_FIFO_STATUS, &stats->fifo_status);
	sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_UART_STATUS, &stats->uart_status);
	sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_HOSTIF_STATUS, &stats->hostif_status);

	memcpy(buf, stats,sizeof(sdla_usb_comm_err_stats_t));
	return 0; 
}

int sdla_usb_flush_err_stats(void *phw)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard = NULL;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	memset(&hwcard->u_usb.stats,0,sizeof(sdla_usb_comm_err_stats_t));
	return 0; 
}

static unsigned int	bhcount=0, rxcount=0, txcount=0;

static int sdla_usb_decode_rxcmd(sdlahw_t *hw, u8 *rx_cmd, int rx_cmd_len)
{
	sdlahw_card_t	*hwcard;
	netskb_t	*skb;
	u_int8_t	*cmd;
	u_int8_t	reg_no;
	int		mod_no;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	switch(WP_USB_CMD_TYPE_DECODE(rx_cmd[0])){
	case WP_USB_CMD_TYPE_WRITE_CPU:
		DEBUG_RX("%s: Rx Write CPU register (%02X:%02X)!\n", 
					hw->devname,
					(unsigned char)rx_cmd[1],
					(unsigned char)rx_cmd[2]);
		break;			
	case WP_USB_CMD_TYPE_READ_CPU:
		DEBUG_RX("%s: Rx Read CPU register (%02X:%02X)!\n", 
					hw->devname,
					(unsigned char)rx_cmd[1],
					(unsigned char)rx_cmd[2]);
		break;
	case WP_USB_CMD_TYPE_WRITE_FXO:
		DEBUG_RX("%s: Rx Write FXO register (%02X:%02X)!\n", 
					hw->devname,
					(unsigned char)rx_cmd[1],
					(unsigned char)rx_cmd[2]);
		break;
	case WP_USB_CMD_TYPE_READ_FXO:
		DEBUG_RX("%s: Rx Read FXO register (%d:%02X)!\n", 
					hw->devname,
					(unsigned char)rx_cmd[1],
					(unsigned char)rx_cmd[2]);
		break;
	case WP_USB_CMD_TYPE_EVENT:
		mod_no = rx_cmd[0] & 0x01;
		reg_no = rx_cmd[1];
		DEBUG_RX("%s: Module %d: Rx Event Info (Reg:%d = %02X:%ld)!\n", 
					hw->devname, mod_no+1, 
					reg_no, rx_cmd[2],
					(unsigned long)SYSTEM_TICKS);
		hwcard->u_usb.regs[mod_no][reg_no] = rx_cmd[2];
		return 0;
	default:
		DEBUG_USB("%s: Unknown command %X : %02X!\n",
				hw->devname,
				WP_USB_CMD_TYPE_DECODE(rx_cmd[0]), rx_cmd[0]);
		hwcard->u_usb.stats.rx_cmd_unknown++;
		return -EINVAL;
	}
	skb = wan_skb_dequeue(&hwcard->u_usb.rx_cmd_free_list);
	cmd = wan_skb_put(skb, rx_cmd_len);
	memcpy(cmd, &rx_cmd[0], rx_cmd_len);
	wan_skb_queue_tail(&hwcard->u_usb.rx_cmd_list, skb); 
	return 0;
}

static int sdla_usb_rx_bh (sdlahw_t *hw)
{
	sdlahw_card_t	*hwcard = NULL;
	int		rx_len, rx_ind = 0;
	int		off, x = 0, mod_no=0, ind=0, high=0, shift=0;
	unsigned char	data, cmd, start_bit, start_fr_bit;
	u_int8_t	*rx_data[2];
	u_int8_t	rx_cmd[10];
	int	rx_cmd_len= 0;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	rx_ind = hwcard->u_usb.next_rx_ind;
	if (rx_ind > hwcard->u_usb.next_read_ind){
		rx_len = rx_ind - hwcard->u_usb.next_read_ind; 
	}else if (rx_ind < hwcard->u_usb.next_read_ind){
		rx_len = MAX_READ_BUF_LEN - hwcard->u_usb.next_read_ind + rx_ind;
	}else{
		DEBUG_USB("%s: INTERNAL ERROR:%d: No data available!\n",
					hw->devname, (u32)SYSTEM_TICKS);
		hwcard->u_usb.stats.rx_underrun_cnt++;
		return -EINVAL;
	}
	if (rx_len < MAX_USB_RX_LEN){
		DEBUG_USB("%s: ERROR: Not enough data (%d:%d:%d:%d)!\n",
				hw->devname, rx_len,
				hwcard->u_usb.next_read_ind,
				hwcard->u_usb.next_rx_ind,
				MAX_READ_BUF_LEN);
		hwcard->u_usb.stats.rx_underrun_cnt++;
		return 0;
	}
	if (rx_len > MAX_USB_RX_LEN) rx_len = MAX_USB_RX_LEN;

	DEBUG_RX("[RX-BH]%s:%d: RX:%d (%d:%d)\n",
			__FUNCTION__,__LINE__,rx_len, 
			hwcard->u_usb.next_read_ind,
			hwcard->u_usb.next_rx_ind);

	off = hwcard->u_usb.next_read_ind;
	x = 0;

	rx_data[0] = hwcard->u_usb.readchunk[0];
	rx_data[1] = hwcard->u_usb.readchunk[1];
	//memset(&hwcard->u_usb.readchunk[0][0], 0, WP_USB_MAX_CHUNKSIZE);
	//memset(&hwcard->u_usb.readchunk[1][0], 0, WP_USB_MAX_CHUNKSIZE);
	memset(&rx_cmd[0],0, 10);

	while(rx_len){

		DEBUG_RX("  RX: %02X\n", (unsigned char)hwcard->u_usb.readbuf[off]);
		data		= hwcard->u_usb.readbuf[off] & 0x0F;
		start_bit	= hwcard->u_usb.readbuf[off] & 0x10;
		start_fr_bit	= hwcard->u_usb.readbuf[off] & 0x20;

		ind	= x / 4;
		mod_no	= x % 2;
		high	= (x % 4) / 2;
		/* decode voice traffic */
		if (mod_no == 0 && high == 0 && (ind % WP_USB_CHUNKSIZE) == 0){
			if (!start_fr_bit){
				DEBUG_USB(
				"%s: ERROR:%d: Start Frame bit missing (%d:%d:%02X:%ld)\n",
						hw->devname,
						bhcount,off,ind,
						(unsigned char)hwcard->u_usb.readbuf[off],
						(unsigned long)SYSTEM_TICKS);
				hwcard->u_usb.stats.rx_start_fr_err_cnt++;
 				/* Skip the current packet */
				off = (off + rx_len) % MAX_READ_BUF_LEN;
				break;
			}
		}
		if (x % 4 == 0){
			if (!start_bit){
				DEBUG_USB(
				"%s: ERROR:%d: Start bit missing (%d:%02X:%ld)\n",
						hw->devname,
						bhcount,off,
						(unsigned char)hwcard->u_usb.readbuf[off],
						(unsigned long)SYSTEM_TICKS);
				hwcard->u_usb.stats.rx_start_err_cnt++;
 				/* Skip the current packet */
				off = (off + rx_len) % MAX_READ_BUF_LEN;
				break;
			}
		}
		DEBUG_RX("x:%d mod:%d ind:%d high:%d data:%02X\n",
					x, mod_no, ind, high,
					(unsigned char)hwcard->u_usb.readbuf[off]);
		if (high){
			rx_data[mod_no][ind] |= (data << 4);	//sc->readchunk[mod_no][ind] |= (data << 4);
		}else{
			rx_data[mod_no][ind] = data;	//sc->readchunk[mod_no][ind] = data;
		}

		/* decode control command (2 bits) */
		cmd = hwcard->u_usb.readbuf[off] & 0xC0;

		ind = x / 4;
		shift = (x % 4) * 2;
		cmd = cmd >> shift;
		if (ind && shift == 0){
			if (rx_cmd[rx_cmd_len] == hwcard->u_usb.ctrl_idle_pattern){
				DEBUG_RX("RX_CMD: x:%d ind:%d shift:%d len:%d ctrl_idle_pattern\n",
							x, ind, shift, rx_cmd_len);
				rx_cmd[rx_cmd_len] = 0x00;
				if (rx_cmd_len){
					if (!wan_skb_queue_len(&hwcard->u_usb.rx_cmd_free_list)){
						DEBUG_USB("%s: INTERNAL ERROR:%d: Received too many commands!\n",
								hw->devname, (u32)SYSTEM_TICKS);
						hwcard->u_usb.stats.rx_cmd_drop_cnt++;
						rx_cmd_len = 0;
					}
					if (rx_cmd_len >= 3){
						sdla_usb_decode_rxcmd(hw, rx_cmd, rx_cmd_len);
					}else{
						hwcard->u_usb.stats.rx_cmd_reset_cnt++;
						if (rx_cmd_len == 1){
							DEBUG_USB("%s: Reset RX Cmd (%d:%02X:%02X)!\n",	
								hw->devname, rx_cmd_len,
								WP_USB_CMD_TYPE_DECODE(rx_cmd[0]), rx_cmd[0]);
						}else{
							DEBUG_USB("%s: Reset RX Cmd (%d:%02X:%02X:%02X)!\n",	
								hw->devname, rx_cmd_len,
								WP_USB_CMD_TYPE_DECODE(rx_cmd[0]), rx_cmd[0],rx_cmd[1]);
						}
					}
					memset(&rx_cmd[0],0,rx_cmd_len);
					rx_cmd_len = 0;
				}
			}else{
				DEBUG_RX("RX_CMD: x:%d ind:%d shift:%d cmd:%02X\n",
					x, ind, shift, (unsigned char)rx_cmd[rx_cmd_len]);
				rx_cmd_len++;
			}
		}
		rx_cmd[rx_cmd_len] |= cmd;
		DEBUG_RX("RX_CMD: x:%d ind:%d shift:%d cmd:%02X:%02X\n",
					x, ind, shift, (unsigned char)cmd,
					(unsigned char)rx_cmd[rx_cmd_len]);
		off = (off + 1) % MAX_READ_BUF_LEN;
		rx_len --;
		x++;
	}
	hwcard->u_usb.next_read_ind = off;

	if (wan_test_bit(WP_USB_STATUS_RX_DATA_READY, &hwcard->u_usb.status) && hwcard->u_usb.isr_func){
		hwcard->u_usb.isr_func(hwcard->u_usb.isr_arg);
	}	
	return 0;
}

static int sdla_usb_tx_bh (sdlahw_t *hw)
{	
	sdlahw_card_t	*hwcard = NULL;
	int		next_write_ind;
	int		off, x = 0, ind, mod_no, tx_idle = 0;
	unsigned char	data, start_bit;
	unsigned char	*txdata[2];

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	DEBUG_TX("%s:%d: TX:%d (%d:%d)\n",
			__FUNCTION__,__LINE__,MAX_USB_TX_LEN,
			hwcard->u_usb.next_tx_ind, hwcard->u_usb.next_write_ind);
	txdata[0] = hwcard->u_usb.writechunk[0];
	txdata[1] = hwcard->u_usb.writechunk[1];
	next_write_ind = hwcard->u_usb.next_write_ind; 
	memcpy(&hwcard->u_usb.writebuf[next_write_ind], &hwcard->u_usb.idlebuf[0], MAX_USB_TX_LEN); 
	for (x = 0; x < WP_USB_RXTX_CHUNKSIZE; x++) {
		for (mod_no = 0; mod_no < 2; mod_no++) {
			start_bit = 0x00;
			if (tx_idle){
				data = WP_USB_IDLE_PATTERN;
			}else{
				data = *(txdata[mod_no] + x);
			}
	
			ind = x * 4 + mod_no;
			WP_USB_CTRL_ENCODE(&hwcard->u_usb.writebuf[next_write_ind], ind);
			/* write loq nibble */
			WP_USB_DATA_ENCODE(&hwcard->u_usb.writebuf[next_write_ind+ind], data);
			DEBUG_TX("TX: x:%d mod:%d ind:%d:%d data:%02X:%02X:%02X\n",
						x, mod_no, ind, ind+2,
						(unsigned char)data,
						(unsigned char)hwcard->u_usb.writebuf[next_write_ind+ind],
						(unsigned char)hwcard->u_usb.writebuf[next_write_ind+ind+2]);
		}
	}	

	if (wan_skb_queue_len(&hwcard->u_usb.tx_cmd_list)){
		u8		*cmd;
		int		cmd_len;
		netskb_t	*skb;
	
		skb = wan_skb_dequeue(&hwcard->u_usb.tx_cmd_list);
		cmd = wan_skb_data(skb);
		cmd_len = wan_skb_len(skb);
		off = 0;

		off++;
		for (x=0; x < cmd_len; x++,off++) {
			WP_USB_CMD_ENCODE(&hwcard->u_usb.writebuf[next_write_ind+off*4], cmd[x]);
			DEBUG_TX("TX_CMD: off:%d byte:%02X %02X:%02X:%02X:%02X\n",
					off, cmd[x],
					(unsigned char)hwcard->u_usb.writebuf[next_write_ind+off*4],
					(unsigned char)hwcard->u_usb.writebuf[next_write_ind+off*4+1],
					(unsigned char)hwcard->u_usb.writebuf[next_write_ind+off*4+2],
					(unsigned char)hwcard->u_usb.writebuf[next_write_ind+off*4+3]);
		}
		wan_skb_init(skb, 0);
		wan_skb_queue_tail(&hwcard->u_usb.tx_cmd_free_list, skb);
	}
	
	next_write_ind = (next_write_ind + MAX_USB_TX_LEN) % MAX_WRITE_BUF_LEN;  
	if (next_write_ind == hwcard->u_usb.next_tx_ind){
		DEBUG_USB("ERROR:%d: TX BH is too fast (%d:%ld)\n",
					bhcount,
					next_write_ind, 
					(unsigned long)SYSTEM_TICKS);
		hwcard->u_usb.stats.tx_overrun_cnt++;
	}else{
		hwcard->u_usb.next_write_ind = next_write_ind;
	}

#if 1
{
	int urb_write_ind = (hwcard->u_usb.urb_write_ind + 1) % MAX_WRITE_URB_COUNT;
	if (wan_test_bit(1, &hwcard->u_usb.datawrite[urb_write_ind].ready)){
		/* Update tx urb */
		hwcard->u_usb.datawrite[urb_write_ind].urb.transfer_buffer = &hwcard->u_usb.writebuf[hwcard->u_usb.next_tx_ind];
		hwcard->u_usb.next_tx_ind += MAX_USB_TX_LEN;
		hwcard->u_usb.next_tx_ind = hwcard->u_usb.next_tx_ind % MAX_WRITE_BUF_LEN;
	}else{
		/* Use previous urb for now and increment errors */
		DEBUG_USB("%s: [BH:%d:%d:%d]: TX is not ready (%ld)!\n",
				hw->devname, bhcount,rxcount,txcount,(unsigned long)SYSTEM_TICKS);
		hwcard->u_usb.stats.tx_overrun_cnt++;
	}
	wan_clear_bit(WP_USB_STATUS_TX_READY, &hwcard->u_usb.status);	//sc->tx_ready = 0;
	if (wp_usb_start_transfer(&hwcard->u_usb.datawrite[urb_write_ind])){
		DEBUG_EVENT("%s: Failed to program transmitter\n", hw->devname);
		hwcard->u_usb.stats.tx_notready_cnt++;
	}else{
		hwcard->u_usb.urb_write_ind = urb_write_ind;
	}	
}

#else
	if (!wan_test_bit(WP_USB_STATUS_TX_READY, &hwcard->u_usb.status)){
		DEBUG_USB("%s: [BH:%d:%d:%d]: TX is not ready (%ld)!\n",
				hw->devname, bhcount,rxcount,txcount,(unsigned long)SYSTEM_TICKS);
		hwcard->u_usb.stats.tx_overrun_cnt++;
		return -EINVAL;
	}

	hwcard->u_usb.datawrite[0].urb.transfer_buffer = &hwcard->u_usb.writebuf[hwcard->u_usb.next_tx_ind];
	hwcard->u_usb.next_tx_ind += MAX_USB_TX_LEN;
	hwcard->u_usb.next_tx_ind = hwcard->u_usb.next_tx_ind % MAX_WRITE_BUF_LEN;
	wan_clear_bit(WP_USB_STATUS_TX_READY, &hwcard->u_usb.status);	//sc->tx_ready = 0;
	if (wp_usb_start_transfer(&hwcard->u_usb.datawrite[0])){
		DEBUG_USB("%s: [BH:%d]: Write cycle failed (%d:%d)\n",
				hw->devname, bhcount,
				hwcard->u_usb.next_tx_ind, hwcard->u_usb.next_write_ind);
		hwcard->u_usb.stats.tx_notready_cnt++;
	}
#endif
	DEBUG_USB("%s: [BH:%d:%d:%d]: Tx is programmed!\n",
			hw->devname, bhcount, rxcount, txcount);
	return 0;
}

#if defined(__LINUX__)
static void sdla_usb_bh (unsigned long data)
#else
static void sdla_usb_bh (void *data, int pending)
#endif
{
	sdlahw_t	*hw = (sdlahw_t*)data;
	sdlahw_card_t	*hwcard = NULL;
	wan_smp_flag_t flags;
	
	WAN_ASSERT_VOID(hw == NULL);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	WAN_ASSERT_VOID(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

 	DEBUG_TEST("%s:%d: ------------ BEGIN --------------: %ld\n",
			__FUNCTION__,__LINE__,(unsigned long)SYSTEM_TICKS);
	wan_spin_lock_irq(&hwcard->u_usb.lock,&flags);
	if (!wan_test_bit(WP_USB_STATUS_READY, &hwcard->u_usb.status)){
		goto usb_bh_done;
	}
	if (wan_test_and_set_bit(WP_USB_STATUS_BH, &hwcard->u_usb.status)){
		DEBUG_EVENT("%s: [BH:%ld]: Re-entry in USB BH!\n",
				hw->devname, (unsigned long)SYSTEM_TICKS);
		goto usb_bh_done;
	}
	bhcount++;

	/* receive path */
	sdla_usb_rx_bh(hw);
	/* transmit path */
	sdla_usb_tx_bh(hw);

	wan_clear_bit(WP_USB_STATUS_BH, &hwcard->u_usb.status);

usb_bh_done:
	wan_spin_unlock_irq(&hwcard->u_usb.lock,&flags);
	WAN_TASKLET_END((&hwcard->u_usb.bh_task));

	DEBUG_TEST("%s: ------------ END -----------------: %ld\n",
                        __FUNCTION__,(unsigned long)SYSTEM_TICKS);
	return;
}

#if defined(LINUX26)  && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
void sdlausb_read_complete(struct urb *q, struct pt_regs *regs)
#else
void sdlausb_read_complete(struct urb *q)
#endif
{
	struct wan_urb	*wurb = (struct wan_urb*)q->context;
	sdlahw_t	*hw = (sdlahw_t*)wurb->pvt;
	sdlahw_card_t	*hwcard = NULL;
	int		next_rx_ind = 0, next_rx_off = 0;
	int		len = 0;	// actual_length = MAX_USB_RX_LEN;
	wan_smp_flag_t	flags; 

	WAN_ASSERT_VOID(hw == NULL);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	WAN_ASSERT_VOID(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	wan_spin_lock_irq(&hwcard->u_usb.lock,&flags);
	rxcount++;
	if (!wan_test_bit(WP_USB_STATUS_READY, &hwcard->u_usb.status)){
		DEBUG_RX("%s: WARNING: RX USB core is not ready (%d:%ld)!\n",
					hw->devname, rxcount,
					(unsigned long)SYSTEM_TICKS);
		hwcard->u_usb.stats.core_notready_cnt++;
		wan_spin_unlock_irq(&hwcard->u_usb.lock,&flags);
		return;
	}
#if 0
	if (q->actual_length < MAX_USB_RX_LEN){
		DEBUG_EVENT("%s: [RX:%d]: WARNING: Invalid Rx length (%d:%d)\n",
					hw->devname, rxcount,
					q->actual_length,MAX_USB_RX_LEN);
		actual_length = q->actual_length; 
	}
#endif
	DEBUG_RX("%s:%d: [RX:%d]: RX:%d:%02X %d:%02X %d:%02X (%ld)\n", 
			hw->devname, wurb->id, rxcount, 
			q->actual_length, ((unsigned char*)q->transfer_buffer)[0], 
			hwcard->u_usb.next_read_ind, (unsigned char)hwcard->u_usb.readbuf[hwcard->u_usb.next_read_ind],
			hwcard->u_usb.next_rx_ind, (unsigned char)hwcard->u_usb.readbuf[hwcard->u_usb.next_rx_ind],
			(unsigned long)SYSTEM_TICKS);

	if (!q->actual_length) goto read_complete_done;

	next_rx_ind = hwcard->u_usb.next_rx_ind + q->actual_length;
	next_rx_off = hwcard->u_usb.next_rx_ind + q->actual_length * hwcard->u_usb.urbcount_read;

	if (hwcard->u_usb.next_rx_ind < hwcard->u_usb.next_read_ind){

		if (next_rx_off > hwcard->u_usb.next_read_ind){
			DEBUG_USB("%s: ERROR:%ld:%d: RX BH is too slow %d:%d:%d (Reset:1)\n",
					hw->devname,		
					(unsigned long)SYSTEM_TICKS, rxcount,
					hwcard->u_usb.next_read_ind,hwcard->u_usb.next_rx_ind,
					q->actual_length*hwcard->u_usb.urbcount_read);
			hwcard->u_usb.stats.rx_overrun_cnt++;
			WAN_TASKLET_SCHEDULE((&hwcard->u_usb.bh_task));
			goto read_complete_done;
		}
		next_rx_ind = next_rx_ind % MAX_READ_BUF_LEN;
		next_rx_off = next_rx_off % MAX_READ_BUF_LEN;

	}else if (hwcard->u_usb.next_rx_ind > hwcard->u_usb.next_read_ind){

		if (next_rx_ind > MAX_READ_BUF_LEN){

			memcpy(	&hwcard->u_usb.readbuf[0], 
				&hwcard->u_usb.readbuf[MAX_READ_BUF_LEN],
				next_rx_ind - MAX_READ_BUF_LEN);
			next_rx_off = next_rx_off % MAX_READ_BUF_LEN;
			if (next_rx_off > hwcard->u_usb.next_read_ind){
				DEBUG_USB("%s: ERROR:%ld:%d: RX BH is too slow %d:%d:%d (Reset:2)\n",
						hw->devname,
						(unsigned long)SYSTEM_TICKS, rxcount,
						hwcard->u_usb.next_read_ind,hwcard->u_usb.next_rx_ind,
						q->actual_length*hwcard->u_usb.urbcount_read);
				hwcard->u_usb.stats.rx_overrun_cnt++;
				WAN_TASKLET_SCHEDULE((&hwcard->u_usb.bh_task));
				goto read_complete_done;
			}
			next_rx_ind = next_rx_ind % MAX_READ_BUF_LEN;
		}else{
			next_rx_ind = next_rx_ind % MAX_READ_BUF_LEN;
			next_rx_off = next_rx_off % MAX_READ_BUF_LEN;
		}
	}else{
		next_rx_ind = next_rx_ind % MAX_READ_BUF_LEN;
		next_rx_off = next_rx_off % MAX_READ_BUF_LEN;
	}
	hwcard->u_usb.next_rx_ind = next_rx_ind;
	wurb->next_off = next_rx_off;	

	/* analyze data */
	if (hwcard->u_usb.next_rx_ind >= hwcard->u_usb.next_read_ind){
		len = hwcard->u_usb.next_rx_ind - hwcard->u_usb.next_read_ind; 
	}else{
		len = MAX_READ_BUF_LEN - hwcard->u_usb.next_read_ind + hwcard->u_usb.next_rx_ind;
	}

	if (hwcard->u_usb.opmode == SDLA_USB_OPMODE_VOICE){
		DEBUG_RX("[RX-] %s: RX:%d %d:%d\n", 
				hw->devname, len, hwcard->u_usb.next_read_ind, hwcard->u_usb.next_rx_ind);
		if (len > 4){
			unsigned char data;
			if (!hwcard->u_usb.rx_sync){
	
				DEBUG_USB("%s: Searching for sync from %d...\n", 
								hw->devname, hwcard->u_usb.next_read_ind);
				do {
					if ((hwcard->u_usb.readbuf[hwcard->u_usb.next_read_ind] & 0x30) != 0x30){
						hwcard->u_usb.next_read_ind = (hwcard->u_usb.next_read_ind+1) % MAX_READ_BUF_LEN;
						len--;
						continue;
					}
					/* Found first sync condition */
					DEBUG_USB("%s: Got first sync condition at %d:%d!\n",
								hw->devname,
								hwcard->u_usb.next_read_ind,
								hwcard->u_usb.next_rx_ind);
					if (len < 4) break;
					data = WP_USB_CTRL_DECODE(&hwcard->u_usb.readbuf[hwcard->u_usb.next_read_ind]);
					if (data == 0x7E){
						/* Found second sync condition */
						DEBUG_EVENT("%s: USB device is connected!\n",
									hw->devname);
						DEBUG_USB("%s: Got in sync at %d:%d!\n",
								hw->devname,
								hwcard->u_usb.next_read_ind,
								hwcard->u_usb.next_rx_ind);
						hwcard->u_usb.rx_sync = 1;
						break;
					}
					hwcard->u_usb.next_read_ind = (hwcard->u_usb.next_read_ind+4) % MAX_READ_BUF_LEN;
					len -= 4;
				}while(len);
				//if (sc->next_read_ind >= MAX_READ_BUF_LEN){
				//	sc->next_read_ind = sc->next_read_ind % MAX_READ_BUF_LEN;
				//}
			}else{
				if ((hwcard->u_usb.readbuf[hwcard->u_usb.next_read_ind] & 0x30) != 0x30){
					DEBUG_EVENT("%s: USB device is disconnected!\n",
									hw->devname);
					DEBUG_USB("%s: WARNING: Got out of sync 1 at %d:%d (%02X)!\n",
							hw->devname,
							hwcard->u_usb.next_read_ind, hwcard->u_usb.next_rx_ind,
							(unsigned char)hwcard->u_usb.readbuf[hwcard->u_usb.next_read_ind]);
					hwcard->u_usb.stats.rx_sync_err_cnt++;
	
					hwcard->u_usb.rx_sync = 0;
					//sc->next_read_ind++;
					hwcard->u_usb.next_read_ind = (hwcard->u_usb.next_read_ind+1) % MAX_READ_BUF_LEN;
					goto read_complete_done;
				}
				data = WP_USB_CTRL_DECODE(&hwcard->u_usb.readbuf[hwcard->u_usb.next_read_ind]);
				if (data != 0x7E){
					DEBUG_EVENT("%s: USB device is disconnected!\n",
									hw->devname);
					DEBUG_USB("%s: WARNING: Got out of sync 2 at %d:%d!\n",
							hw->devname,
							hwcard->u_usb.next_read_ind,
							hwcard->u_usb.next_rx_ind);
					hwcard->u_usb.stats.rx_sync_err_cnt++;
					hwcard->u_usb.rx_sync = 0;
					//sc->next_read_ind++;
					hwcard->u_usb.next_read_ind = (hwcard->u_usb.next_read_ind+1) % MAX_READ_BUF_LEN;
					goto read_complete_done;
				}
			}
		}
	
		if (len >= MAX_USB_RX_LEN){
			WAN_TASKLET_SCHEDULE((&hwcard->u_usb.bh_task));
		}	
	}

read_complete_done:
	DEBUG_TEST("%s: q->dev=%p buf:%p\n", 
				SDLA_USB_NAME, q->dev, q->transfer_buffer);
	//q->dev = sc->dev;
	
	q->transfer_buffer = &hwcard->u_usb.readbuf[wurb->next_off];
	if (wp_usb_start_transfer(wurb)){
		DEBUG_EVENT("%s: Failed to program receiver\n", 
				hw->devname);
	}

	DEBUG_RX("%s: [RX:%d:%d]: %s: %d:%d:%d!\n",
			hw->devname,wurb->id,rxcount,
			hwcard->u_usb.next_read_ind, hwcard->u_usb.next_rx_ind, next_rx_off);
	wan_spin_unlock_irq(&hwcard->u_usb.lock,&flags);
	return;
}


#if defined(LINUX26) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
void sdlausb_write_complete(struct urb *q, struct pt_regs *regs)
#else
void sdlausb_write_complete(struct urb *q)
#endif
{
	struct wan_urb	*wurb = (struct wan_urb*)q->context;
	sdlahw_t	*hw = (sdlahw_t*)wurb->pvt;
	sdlahw_card_t	*hwcard = NULL;
	int		actual_length = MAX_USB_TX_LEN;
	wan_smp_flag_t	flags;

	WAN_ASSERT_VOID(hw == NULL);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	WAN_ASSERT_VOID(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	txcount++;
	wan_spin_lock_irq(&hwcard->u_usb.lock,&flags);
	if (!wan_test_bit(WP_USB_STATUS_READY, &hwcard->u_usb.status)){
		DEBUG_USB("%s: WARNING: TX USB core is not ready (%d:%ld)!\n",
					hw->devname, txcount,
					(unsigned long)SYSTEM_TICKS);
		hwcard->u_usb.stats.core_notready_cnt++;
		wan_spin_unlock_irq(&hwcard->u_usb.lock,&flags);
		return;
	}

	if (hwcard->u_usb.opmode == SDLA_USB_OPMODE_VOICE){
		if (q->actual_length < MAX_USB_TX_LEN){
			DEBUG_TEST("[TX]: WARNING: Invalid Tx length (%d:%d)\n",
						q->actual_length,MAX_USB_TX_LEN);
			actual_length = q->actual_length; 
		}
	}

	/* Mark this urb as ready */
	wan_set_bit(1, &wurb->ready);
	wan_set_bit(WP_USB_STATUS_TX_READY, &hwcard->u_usb.status);	//sc->tx_ready = 1;
	DEBUG_TX("%s:%d: [TX:%d:%d] %d:%d (%d:%d)\n", 
			hw->devname, wurb->id, txcount, txcount,
			MAX_USB_TX_LEN,q->transfer_buffer_length,
			hwcard->u_usb.next_tx_ind, hwcard->u_usb.next_write_ind);
	wan_spin_unlock_irq(&hwcard->u_usb.lock,&flags);
	return;
}


#define usb_endpoint_dir_in(epd)					\
		(((epd)->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
#define usb_endpoint_dir_out(epd)					\
		(((epd)->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)
#define usb_endpoint_xfer_bulk(epd)					\
		(((epd)->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==	\
		USB_ENDPOINT_XFER_BULK)
#define usb_endpoint_is_bulk_in(epd)					\
		(usb_endpoint_xfer_bulk((epd)) && usb_endpoint_dir_in(epd))
#define usb_endpoint_is_bulk_out(epd)					\
		(usb_endpoint_xfer_bulk((epd)) && usb_endpoint_dir_out(epd))

static int sdla_prepare_transfer_urbs(sdlahw_t * hw, struct usb_interface *intf)
{
	sdlahw_card_t			*hwcard = NULL;
	struct usb_host_interface	*iface_desc;
	struct usb_endpoint_descriptor	*endpoint;
	struct usb_endpoint_descriptor	*bulk_in_endpoint[MAX_NUM_PORTS];
	struct usb_endpoint_descriptor	*bulk_out_endpoint[MAX_NUM_PORTS];	
	int				num_bulk_in = 0, num_bulk_out = 0;
	int				x, i = 0;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	DEBUG_USB("%s: Preparing Transfer URBs...\n", hw->devname); 
	/* descriptor matches, let's find the endpoints needed */
	/* check out the endpoints */
	iface_desc = intf->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (usb_endpoint_is_bulk_in(endpoint)) {
			/* we found a bulk in endpoint */
			DEBUG_USB("%s: Found bulk in on endpoint %d\n", hw->devname, i);
			bulk_in_endpoint[num_bulk_in] = endpoint;
			++num_bulk_in;
		}

		if (usb_endpoint_is_bulk_out(endpoint)) {
			/* we found a bulk out endpoint */
			DEBUG_USB("%s: Found bulk out on endpoint %d\n", hw->devname, i);
			bulk_out_endpoint[num_bulk_out] = endpoint;
			++num_bulk_out;
		}
	}

	for (x = 0; x < hwcard->u_usb.urbcount_read; x++) {
		/* set up the endpoint information */
		for (i = 0; i < num_bulk_in; ++i) {
			endpoint = bulk_in_endpoint[i];
			usb_init_urb(&hwcard->u_usb.dataread[x].urb);
			usb_fill_bulk_urb (
					&hwcard->u_usb.dataread[x].urb,
					hwcard->u_usb.usb_dev,
					usb_rcvbulkpipe (hwcard->u_usb.usb_dev,
							   endpoint->bEndpointAddress),
					&hwcard->u_usb.readbuf[MAX_USB_RX_LEN*x], 
					64,	//MAX_USB_RX_LEN,
					sdlausb_read_complete,
					&hwcard->u_usb.dataread[x]);	//p
			hwcard->u_usb.dataread[x].next_off = MAX_USB_RX_LEN*x;
			DEBUG_USB("%s: %d: Bulk Int ENAddress:%X Len:%d (%d:%p)\n",
					hw->devname, x,
					endpoint->bEndpointAddress,MAX_USB_RX_LEN,
					hwcard->u_usb.dataread[x].next_off,
					&hwcard->u_usb.readbuf[MAX_USB_RX_LEN*x]);
		}
	}
	
	for (x = 0; x < hwcard->u_usb.urbcount_write; x++) {
		for (i = 0; i < num_bulk_out; ++i) {
			endpoint = bulk_out_endpoint[i];
			usb_init_urb(&hwcard->u_usb.datawrite[x].urb);
			usb_fill_bulk_urb (
					&hwcard->u_usb.datawrite[x].urb,
					hwcard->u_usb.usb_dev,
					usb_sndbulkpipe (hwcard->u_usb.usb_dev,
							endpoint->bEndpointAddress),
					&hwcard->u_usb.writebuf[MAX_USB_TX_LEN*x], 
					MAX_USB_TX_LEN,
					sdlausb_write_complete,
					&hwcard->u_usb.datawrite[x]);	//p
			hwcard->u_usb.datawrite[x].next_off = MAX_USB_TX_LEN*x;
			DEBUG_USB("%s: %d: Bulk Out ENAddress:%X Len:%d (%d:%p)\n",
					hw->devname, x,
					endpoint->bEndpointAddress,MAX_USB_TX_LEN,
					hwcard->u_usb.datawrite[x].next_off,
					&hwcard->u_usb.writebuf[MAX_USB_TX_LEN*x]);
		}
	}	

	return 0;
}

static int wp_usb_start_transfer(struct wan_urb *wurb)
{
	int	ret;

#ifdef LINUX26
	ret = usb_submit_urb(&wurb->urb, GFP_KERNEL); 
#else
	ret = usb_submit_urb(q); 
#endif
	if (ret){
		return -EINVAL;
	}
	wan_clear_bit(1, &wurb->ready);
	/* Start checking for interrupts */
	//sdlausb_check_interrupt(p);
	return 0;
}

#define REQTYPE_HOST_TO_DEVICE	0x41
#define REQTYPE_DEVICE_TO_HOST	0xc1

/* Config SET requests. To GET, add 1 to the request number */
#define CP2101_UART 		0x00	/* Enable / Disable */
#define CP2101_BAUDRATE		0x01	/* (BAUD_RATE_GEN_FREQ / baudrate) */
#define CP2101_BITS		0x03	/* 0x(0)(databits)(parity)(stopbits) */
#define CP2101_BREAK		0x05	/* On / Off */
#define CP2101_CONTROL		0x07	/* Flow control line states */
#define CP2101_MODEMCTL		0x13	/* Modem controls */
#define CP2101_CONFIG_6		0x19	/* 6 bytes of config data ??? */

/* CP2101_UART */
#define UART_ENABLE		0x0001
#define UART_DISABLE		0x0000

/* CP2101_BAUDRATE */
#define BAUD_RATE_GEN_FREQ	0x384000

/* CP2101_BITS */
#define BITS_DATA_MASK		0X0f00
#define BITS_DATA_5		0X0500
#define BITS_DATA_6		0X0600
#define BITS_DATA_7		0X0700
#define BITS_DATA_8		0X0800
#define BITS_DATA_9		0X0900

#define BITS_PARITY_MASK	0x00f0
#define BITS_PARITY_NONE	0x0000
#define BITS_PARITY_ODD		0x0010
#define BITS_PARITY_EVEN	0x0020
#define BITS_PARITY_MARK	0x0030
#define BITS_PARITY_SPACE	0x0040

#define BITS_STOP_MASK		0x000f
#define BITS_STOP_1		0x0000
#define BITS_STOP_1_5		0x0001
#define BITS_STOP_2		0x0002


static int 
sdla_usb_set_config(sdlahw_t *hw, u8 request, unsigned int *data, int size)
{
	sdlahw_card_t	*hwcard;
	__le32		*buf;
	int		result, i, length;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	WAN_ASSERT(hwcard->u_usb.usb_dev == NULL);

	/* Number of integers required to contain the array */
	length = (((size - 1) | 3) + 1)/4;

	buf = kmalloc(length * sizeof(__le32), GFP_KERNEL);
	if (!buf) {
		DEBUG_ERROR("ERROR [%s:%d]: Out of memory!\n",
				__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	/* Array of integers into bytes */
	for (i = 0; i < length; i++)
		buf[i] = cpu_to_le32(data[i]);

	if (size > 2) {
		result = usb_control_msg (hwcard->u_usb.usb_dev,
				usb_sndctrlpipe(hwcard->u_usb.usb_dev, 0),
				request, REQTYPE_HOST_TO_DEVICE, 0x0000,
				0, buf, size, 300);
	} else {
		result = usb_control_msg (hwcard->u_usb.usb_dev,
				usb_sndctrlpipe(hwcard->u_usb.usb_dev, 0),
				request, REQTYPE_HOST_TO_DEVICE, data[0],
				0, NULL, 0, 300);
	}

	kfree(buf);

	if ((size > 2 && result != size) || result < 0) {
		DEBUG_ERROR("ERROR [%s:%d]: Unable to send request: request=0x%x size=%d result=%d!\n",
				__FUNCTION__,__LINE__, request, size, result);
		return -EPROTO;
	}

	/* Single data value */
	result = usb_control_msg(	hwcard->u_usb.usb_dev,
					usb_sndctrlpipe(hwcard->u_usb.usb_dev, 0),
					request, REQTYPE_HOST_TO_DEVICE, data[0],
					0, NULL, 0, 300);
	return 0;
}


/*
 * cp2101_get_config
 * Reads from the CP2101 configuration registers
 * 'size' is specified in bytes.
 * 'data' is a pointer to a pre-allocated array of integers large
 * enough to hold 'size' bytes (with 4 bytes to each integer)
 */
static int 
sdla_usb_get_config(sdlahw_t *hw, u8 request, unsigned int *data, int size)
{
	sdlahw_card_t	*hwcard = NULL;
	__le32 *buf;
	int result, i, length;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;
	WAN_ASSERT(hwcard->u_usb.usb_dev == NULL);

	/* Number of integers required to contain the array */
	length = (((size - 1) | 3) + 1)/4;

	buf = kcalloc(length, sizeof(__le32), GFP_KERNEL);
	if (!buf) {
		DEBUG_ERROR("ERROR [%s:%d]: Out of memory.\n", __FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	/* For get requests, the request number must be incremented */
	request++;

	/* Issue the request, attempting to read 'size' bytes */
	result = usb_control_msg (	hwcard->u_usb.usb_dev,
					usb_rcvctrlpipe (hwcard->u_usb.usb_dev, 0),
					request, REQTYPE_DEVICE_TO_HOST, 0x0000,
					0, buf, size, 300);

	/* Convert data into an array of integers */
	for (i=0; i<length; i++)
		data[i] = le32_to_cpu(buf[i]);

	kfree(buf);

	if (result != size) {
		DEBUG_EVENT(
		"ERROR [%s:%d]: Unable to send config request, request=0x%x size=%d result=%d\n",
				__FUNCTION__, __LINE__, request, size, result);
		return -EPROTO;
	}

	return 0;
}

/*
 * cp2101_set_config_single
 * Convenience function for calling cp2101_set_config on single data values
 * without requiring an integer pointer
 */
static inline int 
sdla_usb_set_config_single(sdlahw_t *hw, u8 request, unsigned int data)
{
	return sdla_usb_set_config(hw, request, &data, 2);
}



int sdla_usb_setup(sdlahw_t *hw)
{
	sdlahw_card_t	*hwcard;
	netskb_t	*skb;
	int		err, bits, x = 0, mod_no = 0;
	u8		data8;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	DEBUG_USB("%s: Creating private data for usb%s...\n", 
				hw->devname,hwcard->u_usb.bus_id);

	WAN_TASKLET_INIT((&hwcard->u_usb.bh_task),0,sdla_usb_bh,hw);

	WAN_IFQ_INIT(&hwcard->u_usb.rx_cmd_free_list, WP_USB_MAX_RX_CMD_QLEN);
	WAN_IFQ_INIT(&hwcard->u_usb.rx_cmd_list, WP_USB_MAX_RX_CMD_QLEN);
	for(x = 0; x < WP_USB_MAX_RX_CMD_QLEN; x++){
		skb = wan_skb_alloc(10);
		if (!skb){
			DEBUG_ERROR("%s: ERROR: Failed to allocate RX cmd buffer!\n",
						hw->devname);
			goto cleanup;
		}
		wan_skb_queue_tail(&hwcard->u_usb.rx_cmd_free_list, skb);

	}
	WAN_IFQ_INIT(&hwcard->u_usb.tx_cmd_free_list, WP_USB_MAX_TX_CMD_QLEN);
	WAN_IFQ_INIT(&hwcard->u_usb.tx_cmd_list, WP_USB_MAX_TX_CMD_QLEN);
	for(x = 0; x < WP_USB_MAX_RX_CMD_QLEN; x++){
		skb = wan_skb_alloc(10);
		if (!skb){
			DEBUG_ERROR("%s: ERROR: Failed to allocate TX cmd buffer!\n",
						hw->devname);
			goto cleanup;
		}
		wan_skb_queue_tail(&hwcard->u_usb.tx_cmd_free_list, skb);
	}

	hwcard->u_usb.rxtx_len		= WP_USB_RXTX_DATA_LEN;
	hwcard->u_usb.rxtx_count	= WP_USB_RXTX_DATA_COUNT;
	hwcard->u_usb.rxtx_total_len	= hwcard->u_usb.rxtx_len * hwcard->u_usb.rxtx_count;
 	
	hwcard->u_usb.rxtx_buf_len	= hwcard->u_usb.rxtx_total_len * 4;
	hwcard->u_usb.read_buf_len	= hwcard->u_usb.rxtx_buf_len * (WP_USB_MAX_RW_COUNT + 2);
	hwcard->u_usb.write_buf_len	= hwcard->u_usb.rxtx_buf_len * (WP_USB_MAX_RW_COUNT + 2);
 
	wan_spin_lock_init(&hwcard->u_usb.cmd_lock,"usb_cmd");
	wan_spin_lock_irq_init(&hwcard->u_usb.lock,"usb_bh_lock");
	hwcard->u_usb.ctrl_idle_pattern  = WP_USB_CTRL_IDLE_PATTERN;
	if (sdla_usb_set_config_single(hw, CP2101_UART, UART_ENABLE)) {
		DEBUG_ERROR("%s: ERROR: Unable to enable UART!\n",
					hw->devname);
		goto cleanup;
	}

	if (sdla_usb_set_config_single(hw, CP2101_BAUDRATE, (BAUD_RATE_GEN_FREQ / WP_USB_BAUD_RATE))){
		DEBUG_EVENT(
		"%s: ERROR: Baud rate requested not supported by device (%d bps)!\n",
				hw->devname, WP_USB_BAUD_RATE);
		goto cleanup;
	}

	sdla_usb_get_config(hw, CP2101_BITS, &bits, 2);
	bits &= ~BITS_DATA_MASK;
	bits |= BITS_DATA_8;
	if (sdla_usb_set_config(hw, CP2101_BITS, &bits, 2)){
		DEBUG_EVENT(
		"%s: ERROR: Number of data bits requested not supported by device (%02X)!\n",
				hw->devname, bits);
		goto cleanup;
	}

	hwcard->u_usb.urbcount_read = MAX_READ_URB_COUNT;	//4
	hwcard->u_usb.urb_read_ind = 0;
	for (x = 0; x < hwcard->u_usb.urbcount_read; x++){
		hwcard->u_usb.dataread[x].id = x;
		hwcard->u_usb.dataread[x].pvt = hw;
		wan_set_bit(1, &hwcard->u_usb.datawrite[x].ready);
	}

	hwcard->u_usb.urbcount_write = MAX_WRITE_URB_COUNT;	//4
	hwcard->u_usb.urb_write_ind = 0;
	for (x = 0; x < hwcard->u_usb.urbcount_write; x++){
		hwcard->u_usb.datawrite[x].id = x;
		hwcard->u_usb.datawrite[x].pvt = hw;
		wan_set_bit(1, &hwcard->u_usb.datawrite[x].ready);
	}
	if (sdla_prepare_transfer_urbs(hw, hwcard->u_usb.usb_intf)) {
		DEBUG_EVENT(
		"%s: Failed to prepare the urbs for transfer!\n",
				hw->devname);
		goto cleanup;
	}

	hwcard->u_usb.next_rx_ind	= 0;
	hwcard->u_usb.next_read_ind	= 0;
	hwcard->u_usb.next_tx_ind	= 0;
	hwcard->u_usb.next_write_ind	= 0;

	/* Create dummy voice */
	for(mod_no = 0; mod_no < 2; mod_no++){
		memset(&hwcard->u_usb.regs[mod_no][0], 0xFF, 110); 
	}

	/* init initial and idle buffer */
	for (x=0; x < MAX_USB_TX_LEN;x++){
		hwcard->u_usb.writebuf[x] = idlebuf[x];
		hwcard->u_usb.idlebuf[x] = idlebuf[x];
	}
	
	usb_set_intfdata(hwcard->u_usb.usb_intf, hw);
	wan_set_bit(WP_USB_STATUS_READY, &hwcard->u_usb.status);

	/* Verifing sync with USB-FXO device */
	hwcard->u_usb.opmode = SDLA_USB_OPMODE_VOICE;
	for (x = 0; x < hwcard->u_usb.urbcount_read; x++){
		DEBUG_EVENT("%s: Start read sequence...\n",
					hw->devname);
		if (wp_usb_start_transfer(&hwcard->u_usb.dataread[x])) {
			DEBUG_EVENT(
			"%s: Failed to start RX:%d transfer!\n", 
					hw->devname, x);
			goto cleanup;
		}
	}

	err = 0;
	do{
		if (err++ > 10) break;
		wait_just_a_bit(SDLA_USBFXO_READ_DELAY, 0);
	} while(!hwcard->u_usb.rx_sync);

	if (hwcard->u_usb.rx_sync){

		hwcard->u_usb.opmode = SDLA_USB_OPMODE_VOICE;
	
		/* Read hardware ID */
		err = sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_DEVICEID, &data8);
		if (err){
			DEBUG_EVENT("%s: Failed to read USB Device ID (err=%d)!\n",
					hw->devname, err);
			goto cleanup;
		}
		if (data8 != 0x55){
			DEBUG_EVENT("%s: Failed to verify USB Device ID (%X:55)!\n",
					hw->devname, data8);
			goto cleanup;
		}
		/* Read hardware version */ 
		err = sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_HARDWAREVER, &data8);
		if (err){
			DEBUG_EVENT("%s: Failed to read USB Hardware Version (err=%d)!\n",
					hw->devname, err);
			goto cleanup;
		}
		DEBUG_EVENT("%s: USB-FXO Hardware Version %d.%d!\n",
					hw->devname, (data8 >> 4) & 0x0F, data8 & 0x0F);
	
		/* Read firmware version */
		err = sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_FIRMWAREVER, &data8);
		if (err){
			DEBUG_EVENT("%s: Failed to read USB Firmware Version (err=%d)!\n",
					hw->devname, err);
			goto cleanup;
		}
		DEBUG_EVENT("%s: USB-FXO Firmware Version %d.%d!\n",
					hw->devname, (data8 >> 4) & 0x0F, data8 & 0x0F);
	}else{
		DEBUG_EVENT("%s: USB-FXO Device in Firmware Update mode...\n",
				  hw->devname);
		hwcard->u_usb.opmode = SDLA_USB_OPMODE_API;

	}

	sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_FIFO_STATUS, &data8);
	sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_UART_STATUS, &data8);
	sdla_usb_cpu_read(hw, SDLA_USB_CPU_REG_HOSTIF_STATUS, &data8);
	return 0;

cleanup:

	wan_clear_bit(WP_USB_STATUS_READY, &hwcard->u_usb.status);
	WAN_IFQ_PURGE(&hwcard->u_usb.tx_cmd_free_list);
	WAN_IFQ_DESTROY(&hwcard->u_usb.tx_cmd_free_list);
	WAN_IFQ_PURGE(&hwcard->u_usb.rx_cmd_free_list);
	WAN_IFQ_DESTROY(&hwcard->u_usb.rx_cmd_free_list);
	WAN_TASKLET_KILL(&hwcard->u_usb.bh_task);
	usb_set_intfdata(hwcard->u_usb.usb_intf, NULL);
	return -ENODEV;
}

int sdla_usb_down(sdlahw_t *hw, int force)
{
	sdlahw_card_t	*hwcard;
	int		x = 0;

	DEBUG_EVENT("%s: Releasing private data...\n", hw->devname);
	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcard = hw->hwcpu->hwcard;

	wan_clear_bit(WP_USB_STATUS_READY, &hwcard->u_usb.status);

	/* Do not clear the structures if force=1 */
	if (force) return 0;
	WAN_IFQ_PURGE(&hwcard->u_usb.tx_cmd_free_list);
	WAN_IFQ_DESTROY(&hwcard->u_usb.tx_cmd_free_list);
	WAN_IFQ_PURGE(&hwcard->u_usb.rx_cmd_free_list);
	WAN_IFQ_DESTROY(&hwcard->u_usb.rx_cmd_free_list);
	WAN_IFQ_PURGE(&hwcard->u_usb.tx_cmd_list);
	WAN_IFQ_DESTROY(&hwcard->u_usb.tx_cmd_list);
	WAN_IFQ_PURGE(&hwcard->u_usb.rx_cmd_list);
	WAN_IFQ_DESTROY(&hwcard->u_usb.rx_cmd_list);

	for (x = 0; x < hwcard->u_usb.urbcount_read; x++){
		usb_unlink_urb(&hwcard->u_usb.dataread[x].urb);
		usb_kill_urb(&hwcard->u_usb.dataread[x].urb);
	}
	for (x = 0; x < hwcard->u_usb.urbcount_write; x++){
		usb_unlink_urb(&hwcard->u_usb.datawrite[x].urb);
		usb_kill_urb(&hwcard->u_usb.datawrite[x].urb);
	}

	WAN_TASKLET_KILL(&hwcard->u_usb.bh_task);
	usb_set_intfdata(hwcard->u_usb.usb_intf, NULL);
	return 0;
}

#endif   /* #if defined(CONFIG_PRODUCT_WANPIPE_USB)        */
