/*****************************************************************************
* wpusbdummy.c	WANPIPE USB Dummy Timer Interface
*
*
* Authors:	Alex Feldman
*
* Copyright:	(c) 2008 Sangoma Technologies Inc.
*
* ============================================================================
* June 10, 2008	Alex Feldman	Initial version.
*****************************************************************************/

/***************************************************************************
****		I N C L U D E  		F I L E S			****
***************************************************************************/
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe_version.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <wanpipe_common.h>
# include <sdladrv.h>
#elif defined(__LINUX__)||defined(__KERNEL__)
# define _K22X_MODULE_FIX_
# include <linux/wanpipe_includes.h>
# include <linux/tty.h>
# include <linux/usb.h>
# include <linux/usb/serial.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_version.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanpipe_common.h>
#else
# error "Unsupported Operating System!"
#endif

#include <zaptel.h>

#define LINUX26
#if 0
# define DBG_WUD		/* Remove it before release */
#endif
/***************************************************************************
****		   	  M A C R O S / D E F I N E S			****
***************************************************************************/
#define WUD_VENDOR_ID		0x10C4	
#define WUD_PRODUCT_ID		0x8460	
#define WUD_BAUD_RATE		115200

#define WUD_MOD_MAJOR_VER	1
#define WUD_MOD_MINOR_VER	1
#define WUD_MOD_NAME		"wpusbdumy"
#define WUD_MOD_DESC		"WANPIPE USB Dummy Timer Driver"

#define WUD_NAME		"wud"
#define WUD_NAME_LEN		20

#define WUD_CLOCK_SRC		"usbdummy"

#define WUD_STATUS_READY	1
#define WUD_STATUS_DOWN		2

#define MAX_WUD_RX_LEN		1

#if 0
/* Temporary modify for internal testing */

#undef  WUD_PRODUCT_ID
#define WUD_PRODUCT_ID		0xEA62	

#undef  WUD_BAUD_RATE
#define WUD_BAUD_RATE		500000

#undef  MAX_WUD_RX_LEN
#define MAX_WUD_RX_LEN		64
#endif

/***************************************************************************
****		     S T R U C T U R E S / T Y P E D E F S		****
***************************************************************************/
typedef struct wud_softc_
{
	char				name[WUD_NAME_LEN];

	struct usb_interface		*usb_intf;
	struct usb_device		*usb_dev;

	unsigned int			status;
        struct urb			rx_urb;
	char				rx_buf[MAX_WUD_RX_LEN+1];

	struct zt_span			span;
	struct zt_chan			chan;

	WAN_LIST_ENTRY(wud_softc_)	next;

} wud_softc_t;

struct wud_desc {
        char	*name;
        int	adptr_type;
};

/***************************************************************************
****			F U N C T I O N  P R O T O T Y P E S		****
***************************************************************************/
static int wpusbdummy_init(void* arg);
//int wpusbdummy_shutdown(void *arg);
//int wpusbdummy_ready_unload(void *arg);
static int wpusbdummy_exit (void *arg);

static int wud_probe(struct usb_interface*, const struct usb_device_id*);
static void wud_disconnect(struct usb_interface*);
static int wud_suspend (struct usb_interface*, pm_message_t);
static int wud_resume (struct usb_interface*);
static void wud_prereset (struct usb_interface*);
static void wud_postreset (struct usb_interface*);

static int wud_setup(struct usb_interface*);
static int wud_remove(struct usb_interface*);
static int wud_initialize(wud_softc_t*);
static int wud_start_transfer(wud_softc_t*, struct urb*);

/***************************************************************************
****			   G L O B A L  V A R I A B L E S			****
***************************************************************************/
static struct wud_desc wud_dinfo = { WUD_MOD_DESC };

WAN_LIST_HEAD(, wud_softc_) wud_softc_head = 
			WAN_LIST_HEAD_INITIALIZER(&wud_softc_head);

static struct usb_device_id wud_dev_ids[] = {

	{ 
		match_flags: (USB_DEVICE_ID_MATCH_VENDOR|USB_DEVICE_ID_MATCH_DEVICE),
		bInterfaceClass: USB_CLASS_AUDIO,
		bInterfaceSubClass: 1,
		idVendor:	WUD_VENDOR_ID,
		idProduct: 	WUD_PRODUCT_ID,
		driver_info:	(unsigned long)&wud_dinfo,
	},
	{ }     /* Terminating Entry */
};

static struct usb_driver wud_driver =
{
	name: 		WUD_MOD_NAME,
	probe: 		wud_probe,
	disconnect:	wud_disconnect,
	suspend:	wud_suspend,
	resume:		wud_resume,
	pre_reset:	wud_prereset,
	post_reset:	wud_postreset,
	id_table:	wud_dev_ids,
};


#if defined(__LINUX__) || defined(__FreeBSD__)
WAN_MODULE_DEFINE(
		wpusbdummy, "wpusbdummy",
		"Alex Feldman <alex@sangoma.com>",
		WUD_MOD_DESC,
		"GPL",
		wpusbdummy_init, wpusbdummy_exit,
		NULL);
WAN_MODULE_VERSION(wpusbdummy, WUD_MOD_MAJOR_VER);
#endif


/***************************************************************************
****			 F U N C T I O N  D E F I N I T I O N		****
***************************************************************************/


int wpusbdummy_init(void* arg)
{
	int	ret;

	DEBUG_EVENT("%s: Loading %s \n",
			WUD_MOD_NAME, WUD_MOD_DESC);
	WAN_LIST_INIT(&wud_softc_head);

	/* USB-FXO */
	ret = usb_register(&wud_driver);
	if (ret){
		DEBUG_EVENT("%s: Failed to register %s \n",
				WUD_MOD_NAME, WUD_MOD_DESC);
		return ret;
	}

	return 0;
}

#if 0
int wpusbdummy_shutdown(void *arg)
{
	DEBUG_EVENT("%s: Shutting down %s \n",
			WUD_MOD_NAME, WUD_MOD_DESC);
	return 0;
}

int wpusbdummy_ready_unload(void *arg)
{
	DEBUG_EVENT("%s: Is %s ready to unload \n",
			WUD_MOD_NAME, WUD_MOD_DESC);
	return 0;
}
#endif

/*============================================================================
 * Module deinit point.
 * o release all remaining system resources
 */
int wpusbdummy_exit (void *arg)
{
	wud_softc_t	*sc = NULL;

	DEBUG_MOD("%s: Unloading %s \n",
			WUD_MOD_NAME, WUD_MOD_DESC);

	/* Mark all devices down */
	WAN_LIST_FOREACH(sc, &wud_softc_head, next){
		wan_set_bit(WUD_STATUS_DOWN, &sc->status);
	}
	
	usb_deregister(&wud_driver);
	return 0;
}


/****************************************************************************
****************************************************************************/
static int
wud_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device	*dev = interface_to_usbdev(intf);
	struct wud_desc 	*dinfo = (struct wud_desc *)id->driver_info;
	wud_softc_t		*sc = NULL;

	DEBUG_EVENT("%s: Probing %s on %d ...\n",
				WUD_MOD_NAME, 
				dinfo->name, dev->devnum);

	if ((sc= usb_get_intfdata(intf)) != NULL){
		DEBUG_EVENT("%s: Force to add usb device!\n",
				WUD_MOD_NAME);
	}

	if (wud_setup(intf)){
		DEBUG_EVENT("%s: Failed to setup private structure!\n",
				WUD_MOD_NAME);
		return -ENODEV;
	}

	return 0;
}

static void wud_disconnect(struct usb_interface *intf)
{
	struct usb_device	*dev = interface_to_usbdev(intf);

	DEBUG_EVENT("%s: Disconnecting %s from %d ...\n",
				WUD_MOD_NAME, 
				WUD_MOD_DESC, dev->devnum);
	wud_remove(intf);
	return;
}

static int wud_suspend (struct usb_interface *intf, pm_message_t message)
{
	struct usb_device	*dev = interface_to_usbdev(intf);

	DEBUG_EVENT("%s: WARNING: Suspend USB device on %d \n",
				WUD_MOD_NAME, dev->devnum);
	return 0;
}

static int wud_resume (struct usb_interface *intf)
{
	struct usb_device	*dev = interface_to_usbdev(intf);

	DEBUG_EVENT("%s: WARNING: Resume USB device on %d \n",
				WUD_MOD_NAME, dev->devnum);
	return 0;
}

static void wud_prereset (struct usb_interface *intf)
{
	struct usb_device	*dev = interface_to_usbdev(intf);

	DEBUG_EVENT("%s: WARNING: Pre-reset USB device on %d \n",
				WUD_MOD_NAME, dev->devnum);

	return;
}

static void wud_postreset (struct usb_interface *intf)
{
	struct usb_device	*dev = interface_to_usbdev(intf);

	DEBUG_EVENT("%s: WARNING: Post-Reset USB device on %d \n",
				WUD_MOD_NAME, dev->devnum);
	
	return;
}

/****************************************************************************
****************************************************************************/

#if defined(DBG_WUD)
static int rxcount = 0, rxcount_bytes = 0;
static unsigned int last_ticks;
#endif

#if defined(LINUX26)  && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
void wud_read_complete(struct urb *q, struct pt_regs *regs)
#else
void wud_read_complete(struct urb *q)
#endif
{
	wud_softc_t	*sc = (wud_softc_t*)q->context;

	WAN_ASSERT_VOID(sc == NULL);
#if defined(DBG_WUD)
	rxcount++;
#endif
	if (wan_test_bit(WUD_STATUS_DOWN, &sc->status)){
		return;
	}
	if (!wan_test_bit(WUD_STATUS_READY, &sc->status)){
		DEBUG_EVENT("%s: ERROR: RX USB core is not ready!\n",
					sc->name);
		return;
	}
	if (q->actual_length < MAX_WUD_RX_LEN){
		DEBUG_TEST("%s: WARNING: Invalid Rx length (%d:%d)\n",
					sc->name, 
					q->actual_length,
					MAX_WUD_RX_LEN);
	}
	
#if defined(DBG_WUD)
	rxcount_bytes += q->actual_length;
	if ((rxcount_bytes % 1000) == 0){
		DEBUG_EVENT("%s: RX INTR:%d: %d %ld (%ld)\n",
			sc->name, rxcount, rxcount_bytes,
			(unsigned long)SYSTEM_TICKS,
			(unsigned long)(SYSTEM_TICKS-last_ticks)); 
		last_ticks = SYSTEM_TICKS;
	}
	DEBUG_EVENT("%s: RX INTR:%d: %ld\n",
			sc->name, rxcount, (unsigned long)SYSTEM_TICKS);
#endif

	zt_receive(&sc->span);
	zt_transmit(&sc->span);

	if (wud_start_transfer(sc, q)){
		DEBUG_RX(
		"%s: Failed to re-start RX transfer request!\n",
					sc->name);
		return;
	}
	return;
}


#if 0
#if defined(LINUX26) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
void wud_write_complete(struct urb *q, struct pt_regs *regs)
#else
void wud_write_complete(struct urb *q)
#endif
{
	wud_softc_t	*sc = (wud_softc_t*)q->context;


	return;
}
#endif

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



static int wud_setup_transfer_urbs(wud_softc_t *sc, struct usb_interface *intf)
{
	struct usb_host_interface	*iface_desc;
	struct usb_endpoint_descriptor	*endpoint;
	struct usb_endpoint_descriptor	*bulk_in_endpoint[MAX_NUM_PORTS];
	struct usb_endpoint_descriptor	*bulk_out_endpoint[MAX_NUM_PORTS];	
	int				num_bulk_in = 0, num_bulk_out = 0;
	int				i = 0;

	/* descriptor matches, let's find the endpoints needed */
	/* check out the endpoints */
	iface_desc = intf->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (usb_endpoint_is_bulk_in(endpoint)) {
			/* we found a bulk in endpoint */
			DEBUG_TEST("%s: Found bulk in on endpoint %d\n", 
						sc->name, i);
			bulk_in_endpoint[num_bulk_in] = endpoint;
			++num_bulk_in;
		}

		if (usb_endpoint_is_bulk_out(endpoint)) {
			/* we found a bulk out endpoint */
			DEBUG_TEST("%s: Found bulk out on endpoint %d\n",
						sc->name, i);
			bulk_out_endpoint[num_bulk_out] = endpoint;
			++num_bulk_out;
		}
	}

	/* set up the endpoint information */
	for (i = 0; i < num_bulk_in; ++i) {
		endpoint = bulk_in_endpoint[i];
		usb_init_urb(&sc->rx_urb);
		usb_fill_bulk_urb (
				&sc->rx_urb,
				sc->usb_dev,
				usb_rcvbulkpipe (sc->usb_dev,
						   endpoint->bEndpointAddress),
				sc->rx_buf, 
				MAX_WUD_RX_LEN,
				wud_read_complete,
				sc);	//p
		DEBUG_TEST("%s: Bulk Int ENAddress %X\n",
					sc->name,
					endpoint->bEndpointAddress);
	}
	
#if 0
	for (i = 0; i < num_bulk_out; ++i) {
		endpoint = bulk_out_endpoint[i];
		usb_init_urb(&sc->tx_urb);
		usb_fill_bulk_urb (
				&sc->tx_urb,
				sc->usb_dev,
				usb_sndbulkpipe (sc->usb_dev,
						endpoint->bEndpointAddress),
				sc->tx_buf, 
				MAX_WUD_TX_LEN,
				wud_write_complete,
				sc);	//p
		DEBUG_TEST("%s: Bulk Out ENAddress %X\n",
					sc->name,
					endpoint->bEndpointAddress);
	}
#endif
	return 0;
}

static int wud_start_transfer(wud_softc_t *sc, struct urb *q)
{
	int	ret;

	if (wan_test_bit(WUD_STATUS_DOWN, &sc->status)){
		return -EINVAL; 
	}

#ifdef LINUX26
	ret = usb_submit_urb(q, GFP_KERNEL); 
#else
	ret = usb_submit_urb(q); 
#endif
	if (ret){
		DEBUG_TEST(
		"%s: Failed to issue transfer request (%d)!\n",
					sc->name, ret);
		return -EINVAL;
	}
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
wud_set_config(wud_softc_t *sc, u8 request, unsigned int *data, int size)
{
//struct sdla_usb_pvt *p;
	__le32 *buf;
	int result, i, length;

	WAN_ASSERT(sc->usb_dev == NULL);
	/* Number of integers required to contain the array */
	length = (((size - 1) | 3) + 1)/4;

	buf = kmalloc(length * sizeof(__le32), GFP_KERNEL);
	if (!buf) {
		DEBUG_EVENT("%s: ERROR: Failed to allocate memory (%d bytes)!\n",
					sc->name, (length*sizeof(__le32)));
		return -ENOMEM;
	}

	/* Array of integers into bytes */
	for (i = 0; i < length; i++)
		buf[i] = cpu_to_le32(data[i]);

	if (size > 2) {
		result = usb_control_msg (sc->usb_dev,
				usb_sndctrlpipe(sc->usb_dev, 0),
				request, REQTYPE_HOST_TO_DEVICE, 0x0000,
				0, buf, size, 300);
	} else {
		result = usb_control_msg (sc->usb_dev,
				usb_sndctrlpipe(sc->usb_dev, 0),
				request, REQTYPE_HOST_TO_DEVICE, data[0],
				0, NULL, 0, 300);
	}

	kfree(buf);

	if ((size > 2 && result != size) || result < 0) {
		DEBUG_EVENT("%s: ERROR: Failed to send request (%X:%d:%d)!\n",
					sc->name, request, size, result);
		return -EPROTO;
	}

	/* Single data value */
	result = usb_control_msg(sc->usb_dev,
				usb_sndctrlpipe(sc->usb_dev, 0),
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
wud_get_config(wud_softc_t *sc, u8 request, unsigned int *data, int size)
{
//struct sdla_usb_pvt *p;
	__le32 *buf;
	int result, i, length;

	WAN_ASSERT(sc->usb_dev == NULL);
	/* Number of integers required to contain the array */
	length = (((size - 1) | 3) + 1)/4;

	buf = kcalloc(length, sizeof(__le32), GFP_KERNEL);
	if (!buf) {
		DEBUG_EVENT("%s: ERROR: Failed allocate memory (%d bytes)!\n",
					sc->name, (length*sizeof(__le32)));
		return -ENOMEM;
	}

	/* For get requests, the request number must be incremented */
	request++;

	/* Issue the request, attempting to read 'size' bytes */
	result = usb_control_msg (	sc->usb_dev,
					usb_rcvctrlpipe (sc->usb_dev, 0),
					request, REQTYPE_DEVICE_TO_HOST, 0x0000,
					0, buf, size, 300);

	/* Convert data into an array of integers */
	for (i=0; i<length; i++)
		data[i] = le32_to_cpu(buf[i]);

	kfree(buf);

	if (result != size) {
		DEBUG_EVENT("%s: ERROR: Unable to send config request (%X:%d:%d)!\n",
					sc->name, request, size, result);
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
wud_set_config_single(wud_softc_t *sc, u8 request, unsigned int data)
{
	return wud_set_config(sc, request, &data, 2);
}



static int wud_setup(struct usb_interface *intf)
{
	struct usb_device	*dev = interface_to_usbdev(intf);
	wud_softc_t		*sc = NULL;
	unsigned int		baud, bits;

	sc = wan_malloc(sizeof(wud_softc_t));
	if (!sc) {
		DEBUG_EVENT("%s: Failed allocate private data!\n",
				WUD_MOD_NAME);
		return -ENODEV;
	}
	memset(sc, 0, sizeof(wud_softc_t));
	sprintf(sc->name, "%s0", WUD_NAME);
	sc->usb_intf	= intf;
	sc->usb_dev	= dev;
	DEBUG_TEST("%s: Creating private data...\n",
				sc->name);
 
	if (wud_set_config_single(sc, CP2101_UART, UART_ENABLE)) {
		DEBUG_EVENT("%s: ERROR: Failed to enable UART!\n",
					sc->name);
		goto wud_setup_error;
	}

	baud = WUD_BAUD_RATE;
	if (wud_set_config_single(sc, CP2101_BAUDRATE, (BAUD_RATE_GEN_FREQ / baud))){
		DEBUG_EVENT("%s: ERROR: Failed to set new baud rate (%d bps)!\n",
					sc->name, baud);
		goto wud_setup_error;
	}

	wud_get_config(sc, CP2101_BITS, &bits, 2);
	bits &= ~BITS_DATA_MASK;
	bits |= BITS_DATA_8;
	if (wud_set_config(sc, CP2101_BITS, &bits, 2)){
		DEBUG_EVENT("%s: ERROR: Failed set new data bits (%X)!\n",
					sc->name, bits);
		goto wud_setup_error;
	}

	if (wud_setup_transfer_urbs(sc, intf)) {
		DEBUG_EVENT("%s: ERROR: Failed to prepare the urbs for transfer!\n",
					sc->name);
		goto wud_setup_error;
	}

	usb_set_intfdata(sc->usb_intf, sc);
	WAN_LIST_INSERT_HEAD(&wud_softc_head, sc, next);
	wan_set_bit(WUD_STATUS_READY, &sc->status);

	if (wud_initialize(sc)){
		DEBUG_EVENT("%s: Failed to register into Zaptel driver!\n",
					sc->name);
		WAN_LIST_REMOVE(sc, next);
		usb_set_intfdata(sc->usb_intf, NULL);
		goto wud_setup_error;
	}

	if (wud_start_transfer(sc, &sc->rx_urb)) {
		DEBUG_EVENT("%s: Failed to start RX transfer!\n",
					sc->name);
		zt_unregister(&sc->span);
		WAN_LIST_REMOVE(sc, next);
		usb_set_intfdata(sc->usb_intf, NULL);
		goto wud_setup_error;
	}


	return 0;

wud_setup_error:

	wan_clear_bit(WUD_STATUS_READY, &sc->status);
	usb_set_intfdata(sc->usb_intf, NULL);
	if (sc) wan_free(sc);

	return -ENODEV;
}

static int wud_remove(struct usb_interface *intf)
{
	wud_softc_t	*sc;

	DEBUG_TEST("Releasing private data...\n");
	if ((sc = usb_get_intfdata(intf)) != NULL){

		wan_clear_bit(WUD_STATUS_READY, &sc->status);

		zt_unregister(&sc->span);

		WAN_LIST_REMOVE(sc, next);
		usb_unlink_urb(&sc->rx_urb);
		usb_kill_urb(&sc->rx_urb);

		usb_set_intfdata(sc->usb_intf, NULL);

		if (sc) wan_free(sc);
	}else{

		DEBUG_EVENT("%s: Internal Error: [%s:%d]\n",
				WUD_MOD_NAME, __FUNCTION__,__LINE__);
	}
	return 0;
}


static int wud_initialize(wud_softc_t *sc)
{
	/* Zapata stuff */
	sprintf(sc->span.name, "WPUSBDUMMY/1");
	snprintf(sc->span.desc, sizeof(sc->span.desc) - 1, "%s (source: " WUD_CLOCK_SRC ") %d", sc->span.name, 1);
	sprintf(sc->chan.name, "WPUSBDUMMY/%d/%d", 1, 0);
	strncpy(sc->span.devicetype, WUD_MOD_DESC, sizeof(sc->span.devicetype) - 1);
	sc->chan.chanpos = 1;
	sc->span.chans = &sc->chan;
	sc->span.channels = 0;		/* no channels on our span */
	sc->span.deflaw = ZT_LAW_MULAW;
	init_waitqueue_head(&sc->span.maintq);
	sc->span.pvt = sc;
	sc->chan.pvt = sc;
	if (zt_register(&sc->span, 0)) {
		return -1;
	}
	return 0;
}
