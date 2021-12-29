/*****************************************************************************
* sdla_ss7.c	WANPIPE(tm) Multiprotocol WAN Link Driver. SS7 module.
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*		Gideon Hack  
*
* Copyright:	(c) 1995-2001 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Dec 20 2000	Nenad Corbic	Initial version based on ss7 driver.
*****************************************************************************/

#include <linux/wanpipe_includes.h>
#include <linux/wanpipe_defines.h>
#include <linux/wanrouter.h>	/* WAN router definitions */
#include <linux/wanpipe.h>	/* WANPIPE common user API definitions */
#include <linux/sdlapci.h>
#include <linux/sdla_ss7.h>		/* ss7 firmware API definitions */
#include <linux/ss7_linux.h>           	/* ss7 (async) API definitions */
#include <linux/if_wanpipe_common.h>    /* Socket Driver common area */
#include <linux/if_wanpipe.h>

/****** Defines & Macros ****************************************************/

/* reasons for enabling the timer interrupt on the adapter */
#define TMR_INT_ENABLED_UDP   		0x01
#define TMR_INT_ENABLED_UPDATE		0x02
#define TMR_INT_ENABLED_CONFIG		0x10
#define TMR_INT_ENABLED_L2_HIST		0x20
#define TMR_INT_ENABLED_TE   		0x40

//#define SHOW_L2_HIST	1

#define MAX_IP_ERRORS	100
 
#define	CHDLC_DFLT_DATA_LEN	1500		/* default MTU */
#define CHDLC_HDR_LEN		1

#define CHDLC_API 0x01

#define PORT(x)   (x == 0 ? "PRIMARY" : "SECONDARY" )

#ifdef SS7_IOCTL_INTERFACE
#define MAX_BH_BUFF	100
#else
#define MAX_BH_BUFF	10
#endif

#define MIN_LGTH_SS7_DATA_CFG 		100
#define PRI_MAX_NO_DATA_BYTES_IN_FRAME 	4000
#define SS7_DFLT_DATA_LEN 		1500

#define COMMAND_OK	0
#define SS7_HDR_LEN	1

#ifndef IS_TE1_CARD
#define IS_TE1_CARD(card)	IS_TE1(card->wandev.te_cfg)
#endif

#ifndef IS_56K_CARD
#define IS_56K_CARD(card)	IS_56K(card->wandev.te_cfg)
#endif

/* This value is used to delay API tx when the
 * ss7 buffers become full.  After this value
 * the driver will trigger the user API to 
 * continue sending. HZ = 1sec */

#define TX_INTR_TIMEOUT (HZ/30)
/******Data Structures*****************************************************/

/* This structure is placed in the private data area of the device structure.
 * The card structure used to occupy the private area but now the following 
 * structure will incorporate the card structure along with CHDLC specific data
 */

typedef struct ss7_private_area
{
	wanpipe_common_t common;
	char 		if_name[WAN_IFNAME_SZ+1];
	sdla_t		*card;
	int 		TracingEnabled;		/* For enabling Tracing */
	unsigned long 	curr_trace_addr;	/* Used for Tracing */
	unsigned long 	start_trace_addr;
	unsigned long 	end_trace_addr;
	unsigned long 	base_addr_trace_buffer;
	unsigned long 	end_addr_trace_buffer;
	unsigned short 	number_trace_elements;
	unsigned  	available_buffer_space;
	unsigned long 	router_start_time;
	unsigned char 	route_status;
	unsigned char 	route_removed;
	unsigned long 	tick_counter;		/* For 5s timeout counter */
	unsigned long 	router_up_time;
        u32             IP_address;		/* IP addressing */
        u32             IP_netmask;
	u32		ip_local;
	u32		ip_remote;
	u32 		ip_local_tmp;
	u32		ip_remote_tmp;
	u8		ip_error;
	u8		config_ss7;
	u8 		config_ss7_timeout;
	unsigned char  mc;			/* Mulitcast support on/off */
	unsigned short timer_int_enabled;
	char update_comms_stats;		/* updating comms stats */

	u8 interface_down;
	u8 gateway;
	u8 true_if_encoding;

	struct timer_list tx_timer;
	//FIXME: add driver stats as per frame relay!
	unsigned L2_hist_counter;	
	unsigned L2_state;

#ifdef SS7_IOCTL_INTERFACE
	unsigned char tx_data[MAX_LENGTH_MSU_SIF+10];
	unsigned int tx_len;
#if defined(LINUX_2_4)||defined(LINUX_2_6)
	wait_queue_head_t sleep;	
#else
	struct wait_queue *sleep;
#endif
#endif
	unsigned long ptr_first_MSU_bfr;
	unsigned int number_MSUs;

	unsigned char udp_pkt_data[MAX_LGTH_UDP_MGNT_PKT];

	wan_mbox_t    usr_cmd_mb;
	atomic_t udp_pkt_len;

} ss7_private_area_t;

/* Route Status options */
#define NO_ROUTE	0x00
#define ADD_ROUTE       0x01
#define ROUTE_ADDED	0x02
#define REMOVE_ROUTE	0x03

/* power off = 'PWR ON' */
/* initial alignment = 'ALIGN CPLT' and 'ALIGN NOT POSS' */
/* aligned ready */
/* aligned not ready */
/* in service */
char *act_LSC_0[] = {  "PWR ON", "START", "EMERG", "EMERG CEASES",
                       "LOC PROC OUT", "RESUME", "CLR BFRS", "ALIGN CPLT", 
		       "STOP", "LINK FAIL", "ALIGN NOT POSS",   "FISU/MSU RX", 
		       "SIPO", "SIO,SIOS", "T1 EXP", "SIO,SIN,SIE,SIOS"};

/* processor outage = 'RTB CLEARED' and 'CLEAR RTB' */
char *act_LSC_1[] = {  "PWR ON",           "RTB CLEARED",      "EMERG",            "EMERG CEASES",
                       "LOC PROC OUT",     "RESUME",           "CLR BFRS",         "ALIGN CPLT",
                       "STOP",             "LINK FAIL",			"CLR RTB",          "FISU/MSU RX",
                       "SIPO",             "SIO,SIOS",			"T1 EXP",           "SIO,SIN,SIE,SIOS"};

/* out of service = 'START', 'RETRIEVE BSNT' and 'RET REQ AND FSNC' */
char *act_LSC_2[] = {  "RETRIEVE BSNT",    "START",            "EMERG",            "EMERG CEASES", 
                       "LOC PROC OUT",     "RESUME",           "CLR BFRS",         "RET REQ AND FSNC",
                       "STOP",             "LINK FAIL",        "CLR RTB",          "FISU/MSU RX",
                       "SIPO",             "SIO,SIOS",         "T1 EXP",           "SIO,SIN,SIE,SIOS"};

char *state_LSC[] = {  "PWR OFF",          "OUT OF SERVICE",   "INIT ALIGN",       "ALIGNED READY",
						     "ALIGNED NOT READY","IN SERVICE",       "PROC OUT",         "EMERGENCY",
						     "LOC PROC OUT",     "REM PROC OUT",     "T1 RUNNING",       "*INVALID*",
						     "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"};

char *act_IAC[] = {    "EMERG",            "START",            "SIO,SIN",          "SIE",
                       "STOP",             "T2 EXP",           "SIN",              "T3 EXP",
                       "SIOS",             "CORRECT SU",       "T4 EXP",           "ABRT PRV", 
                       "SIO",              "*INVALID*",        "*INVALID*",        "*INVALID*"};

char *state_IAC[] = {  "IDLE",             "NOT ALIGN",        "ALIGNED",          "PROVING",
                       "FURTHER PRV",      "EMERG",            "T2 RUNNING",       "T3 RUNNING",
                       "T4 RUNNING",       "*INVALID*",        "*INVALID*",        "*INVALID*",
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *act_DAEDR[] = {  "START",            "7 CONSEC 1's",     "Nmax PLUS 1",      "16 OCTETS", 
							  "BITS RX",          "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *state_DAEDR[] = {"IDLE",             "IN SERVICE",       "OCTET COUNT MODE", "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *act_DAEDT[] = {  "START",            "SIGNAL UNIT",      "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *state_DAEDT[] = {"IDLE",             "IN SERVICE",       "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *act_TXC[] = {    "START",            "T6 EXP",           "T7 EXP",           "TX REQ",
                       "CLR TB",           "CLR RTB",          "SEND FISU",        "SEND MSU",
                       "MSG FOR TX",       "RET REQ AND FSNC", "SEND LSSU",        "FSNX VALUE",
                       "NACK TO BE TX",    "BSNR AND BIBR",    "SIB RX",           "*INVALID*"};

char *state_TXC[] = {  "IDLE",             "IN SERVICE",       "RTB FULL",         "LSSU AVAIL", 
						     "MSU INHIB",        "CLR RTB",          "SIB RX",           "FORCED RE-TX (PCR)",
							  "T6 RUNNING",       "T7 RUNNING",       "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *act_RXC[] = {    "START",            "RETRIEVE BSNT",    "FSNT VALUE",       "SIGNAL UNIT",
						     "REJ MSU/FISU",     "ACCEPT MSU/FISU",  "ALIGN FSNX",       "CLR RB", 
						     "STOP",             "CONGESTION DISC",  "CONGESTION ACC",   "NO CONGESTION",
					        "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"};

char *state_RXC[] = {  "IDLE",             "IN SERVICE",       "MSU/FISU ACCEPTED","ABNORMAL BSNR",
						     "ABNORMAL FIBR",    "CONGESTION DISC",  "CONGESTION ACC",   "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *act_AERM[] = {   "SET Ti TO Tin"     "START"             "SET Ti TO Tie"     "STOP"   
							  "SU IN ERROR",      "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *state_AERM[] = {  "IDLE"             "MONITORING"        "*INVALID*",        "*INVALID*", 
							   "*INVALID*",       "*INVALID*",        "*INVALID*",        "*INVALID*", 
							   "*INVALID*",       "*INVALID*",        "*INVALID*",        "*INVALID*", 
							   "*INVALID*",       "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *act_SUERM[] = {  "START"             "STOP"              "SU IN ERROR"       "CORRECT SU"
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *state_SUERM[] = {"IDLE"              "IN SERVICE"       "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *act_CC[] = {     "BUSY",             "NORMAL",           "STOP",             "T5 EXP",
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *state_CC[] = {   "IDLE",             "LVL 2 CONG",       "T5 RUNNING",       "*INVALID*",
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *act_EIM[] = {    "START",            "STOP"              "T8 EXP"            "CORRECT SU",
							  "SU IN ERROR",      "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 

char *state_EIM[] = {  "IDLE"              "MONITORING"        "INTERVAL ERROR"    "SU RECEIVED",
							  "T8 RUNNING",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*", 
							  "*INVALID*",        "*INVALID*",        "*INVALID*",        "*INVALID*"}; 



static void L2_read_hist_table(sdla_t *card);
static void prt_hist(unsigned short, char *[], char *);


/* variable for tracking how many interfaces to open for WANPIPE on the
   two ports */

extern void disable_irq(unsigned int);
extern void enable_irq(unsigned int);
extern int check_ss7_hw(sdlahw_t* hw);

/****** Function Prototypes *************************************************/
/* WAN link driver entry points. These are called by the WAN router module. */
static int update (wan_device_t* wandev);
static int new_if (wan_device_t* wandev, netdevice_t* dev,
	wanif_conf_t* conf);
static int del_if(wan_device_t *wandev, netdevice_t *dev);

/* Network device interface */
static int if_init   (netdevice_t* dev);
static int if_open   (netdevice_t* dev);
static int if_close  (netdevice_t* dev);

static int do_ioctl (netdevice_t *dev, struct ifreq *ifr, int cmd);

static struct net_device_stats* if_stats (netdevice_t* dev);

static int if_send (struct sk_buff* skb, netdevice_t* dev);

/* CHDLC Firmware interface functions */
static int ss7_configure 	(sdla_t* card, void* data);
static int ss7_comm_enable 	(sdla_t* card);
static int ss7_read_version 	(sdla_t* card, char* str);
static int ss7_set_intr_mode 	(sdla_t* card, unsigned mode);
static int ss7_send (sdla_t* card, void* data, unsigned len, unsigned char);
static int ss7_read_comm_err_stats (sdla_t* card);
static int ss7_read_op_stats (sdla_t* card);
static void tx_intr (unsigned long data);


static int ss7_disable_comm_shutdown (sdla_t *card);
static void if_tx_timeout (netdevice_t *dev);

/* Miscellaneous CHDLC Functions */
static int set_ss7_config (sdla_t* card);
static void init_ss7_tx_rx_buff( sdla_t* card);
static int ss7_error (sdla_t *card, int err, wan_mbox_t *mb);
static int process_ss7_exception(sdla_t *card);
static int process_global_exception(sdla_t *card);
static int update_comms_stats(sdla_t* card,
        ss7_private_area_t* ss7_priv_area);
static void port_set_state (sdla_t *card, int);
static int config_ss7 (sdla_t *card);
static void disable_comm (sdla_t *card);

/* Interrupt handlers */
static WAN_IRQ_RETVAL wpc_isr (sdla_t* card);
static void rx_intr (sdla_t* card);
static void timer_intr(sdla_t *);

/* Bottom half handlers */
static void ss7_bh (unsigned long data);
static void send_oob_msg (sdla_t *card, netdevice_t *dev);

/* Miscellaneous functions */
static int intr_test( sdla_t* card);
static int process_udp_mgmt_pkt(sdla_t* card, netdevice_t* dev,  
				ss7_private_area_t* ss7_priv_area);
static void s508_lock (sdla_t *card, unsigned long *smp_flags);
static void s508_unlock (sdla_t *card, unsigned long *smp_flags);

static void ss7_enable_timer (void* card_id);
static void ss7_handle_front_end_state(void* card_id);
static int set_adapter_config (sdla_t* card);



/****** Public Functions ****************************************************/

/*============================================================================
 * Cisco HDLC protocol initialization routine.
 *
 * This routine is called by the main WANPIPE module during setup.  At this
 * point adapter is completely initialized and firmware is running.
 *  o read firmware version (to make sure it's alive)
 *  o configure adapter
 *  o initialize protocol-specific fields of the adapter data space.
 *
 * Return:	0	o.k.
 *		< 0	failure.
 */
int wpss7_init (sdla_t* card, wandev_conf_t* conf)
{
	int err;
	union{
		char str[80];
	} u;
	volatile wan_mbox_t* mb;
	wan_mbox_t* mb1;
	unsigned long timeout;
	u8	pciextraver;

	/* Verify configuration ID */
	if (conf->config_id != WANCONFIG_SS7) {
		printk(KERN_INFO "%s: invalid configuration ID %u!\n",
				  card->devname, conf->config_id);
		return -EINVAL;
	}

	card->hw_iface.getcfg(card->hw, SDLA_PCIEXTRAVER, &pciextraver);
	if (pciextraver != SS7_USER_ID){
		printk(KERN_INFO "%s: ERROR: SS7 firmware on invalid hardware!\n",card->devname);
		return -EPERM;
	}

	card->u.s.comm_port = WANOPT_PRI;

	card->u.s.line_cfg_opt 		= conf->u.ss7.line_cfg_opt;
	card->u.s.modem_cfg_opt 	= conf->u.ss7.modem_cfg_opt;
	card->u.s.modem_status_timer 	= conf->u.ss7.modem_status_timer;
	card->u.s.api_options 		= conf->u.ss7.api_options;
	card->u.s.protocol_options 	= conf->u.ss7.protocol_options;
	card->u.s.stats_history_options	= conf->u.ss7.stats_history_options;
	card->u.s.max_length_msu_sif	= conf->u.ss7.max_length_msu_sif;
	card->u.s.max_unacked_tx_msus	= conf->u.ss7.max_unacked_tx_msus;
	card->u.s.link_inactivity_timer	= conf->u.ss7.link_inactivity_timer;
	card->u.s.t1_timer 		= conf->u.ss7.t1_timer;
	card->u.s.t2_timer 		= conf->u.ss7.t2_timer;
	card->u.s.t3_timer 		= conf->u.ss7.t3_timer;
	card->u.s.t4_timer_emergency 	= conf->u.ss7.t4_timer_emergency;	
	card->u.s.t4_timer_normal	= conf->u.ss7.t4_timer_normal;	
	card->u.s.t5_timer 		= conf->u.ss7.t5_timer;
	card->u.s.t6_timer 		= conf->u.ss7.t6_timer;
	card->u.s.t7_timer 		= conf->u.ss7.t7_timer;
	card->u.s.t8_timer 		= conf->u.ss7.t8_timer;
	card->u.s.n1 			= conf->u.ss7.n1;
	card->u.s.n2 			= conf->u.ss7.n2;
	card->u.s.tie 			= conf->u.ss7.tie;
	card->u.s.tin 			= conf->u.ss7.tin;
	card->u.s.suerm_error_threshold	= conf->u.ss7.suerm_error_threshold;
	card->u.s.suerm_number_octets	= conf->u.ss7.suerm_number_octets;
	card->u.s.suerm_number_sus	= conf->u.ss7.suerm_number_sus;
	card->u.s.sie_interval_timer	= conf->u.ss7.sie_interval_timer;
	card->u.s.sio_interval_timer	= conf->u.ss7.sio_interval_timer;
	card->u.s.sios_interval_timer	= conf->u.ss7.sios_interval_timer;
	card->u.s.fisu_interval_timer	= conf->u.ss7.fisu_interval_timer;

	switch(conf->u.ss7.protocol_specification) {
		case WANOPT_SS7_ANSI:
			card->u.s.protocol_specification = L2_PROTOCOL_ANSI;
			break;

		case WANOPT_SS7_ITU:
			card->u.s.protocol_specification = L2_PROTOCOL_ITU;
			break;


		case WANOPT_SS7_NTT:
			card->u.s.protocol_specification = L2_PROTOCOL_NTT;
			break;
	}	
	
	/* Initialize protocol-specific fields */
	/* Set a pointer to the actual mailbox in the allocated virtual 
	 * memory area */
	/* ALEX Apr 8 2004 Sangoma ISA card */
	card->mbox_off = PRI_BASE_ADDR_MB_STRUCT;

	card->wandev.ignore_front_end_status = conf->ignore_front_end_status;

	//ALEX_TODAY err=check_conf_hw_mismatch(card,conf->te_cfg.media);
	err = (card->hw_iface.check_mismatch) ? 
			card->hw_iface.check_mismatch(card->hw,conf->fe_cfg.media) : -EINVAL;
	if (err){
		return err;
	}
	
	/* Determine the board type user is trying to configure
	 * and make sure that the HW matches */
	if (IS_TE1_MEDIA(&conf->fe_cfg)){

		memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));
		sdla_te_iface_init(&card->wandev.fe_iface);
		card->fe.name		= card->devname;
		card->fe.card		= card;
		card->fe.write_fe_reg   = card->hw_iface.fe_write;
		card->fe.read_fe_reg	= card->hw_iface.fe_read;
		card->wandev.fe_enable_timer = ss7_enable_timer;
		card->wandev.te_link_state = ss7_handle_front_end_state;
		conf->electrical_interface = 
			(IS_T1_CARD(card)) ? WANOPT_V35 : WANOPT_RS232;
		conf->clocking = WANOPT_EXTERNAL;
		
        }else if (IS_56K_MEDIA(&conf->fe_cfg)){
                
		memcpy(&card->fe.fe_cfg, &conf->fe_cfg, sizeof(sdla_fe_cfg_t));
		sdla_56k_iface_init(&card->wandev.fe_iface);
		card->fe.name		= card->devname;
		card->fe.card		= card;
		card->fe.write_fe_reg   = card->hw_iface.fe_write;
		card->fe.read_fe_reg    = card->hw_iface.fe_read;
	
	}else{
		card->fe.fe_status = FE_CONNECTED;
	}

	if (card->wandev.ignore_front_end_status == WANOPT_NO){
		printk(KERN_INFO 
		  "%s: Enabling front end link monitor\n",
				card->devname);
	}else{
		printk(KERN_INFO 
		"%s: Disabling front end link monitor\n",
				card->devname);
	}

	
	mb = &card->wan_mbox;
	mb1 = &card->wan_mbox;

	if (!card->configured){

		unsigned char return_code;
		/* The board will place an 'I' in the return code to indicate that it is
	   	ready to accept commands.  We expect this to be completed in less
           	than 1 second. */

		timeout = jiffies;
		do {
			return_code = 0x00;
			card->hw_iface.peek(card->hw,
					    card->mbox_off+offsetof(wan_mbox_t, wan_return_code),
					    &return_code, 
					    sizeof(unsigned char));
			
			if ((jiffies - timeout) > 1*HZ){ 
				break;
			}
		
		}while(return_code != 'I');

		
		if (return_code != 'I') {
			printk(KERN_INFO
				"%s: Initialization not completed by adapter\n",
				card->devname);
			printk(KERN_INFO "Please contact Sangoma representative.\n");
			return -EIO;
		}

		if (set_adapter_config(card)) {
			return -EIO;
		}
	}

	/* Read firmware version.  Note that when adapter initializes, it
	 * clears the mailbox, so it may appear that the first command was
	 * executed successfully when in fact it was merely erased. To work
	 * around this, we execute the first command twice.
	 */

	if (ss7_read_version(card, u.str))
		return -EIO;

	
	printk(KERN_INFO "%s: Running SS7 firmware v%s\n",
		card->devname, u.str); 
	
	card->isr			= &wpc_isr;
	card->poll			= NULL;
	card->exec			= NULL;
	card->wandev.update		= &update;
 	card->wandev.new_if		= &new_if;
	card->wandev.del_if		= &del_if;
	card->wandev.udp_port   	= conf->udp_port;
	card->disable_comm		= &disable_comm;
	card->wandev.new_if_cnt = 0;

	card->wandev.ttl = conf->ttl;
	card->wandev.electrical_interface = conf->electrical_interface; 

	if ((card->u.s.comm_port == WANOPT_SEC && conf->electrical_interface == WANOPT_V35)&&
	    card->type != SDLA_S514){
		printk(KERN_INFO "%s: ERROR - V35 Interface not supported on S508 %s port \n",
			card->devname, PORT(card->u.s.comm_port));
		return -EIO;
	}

	card->wandev.clocking = conf->clocking;

	/* Setup Port Bps */
	if (card->wandev.clocking) {
		card->wandev.bps = conf->bps;
	}else{
        	card->wandev.bps = 0;
  	}

	card->wandev.mtu = MAX_LENGTH_MSU_SIF; 

	/* Set up the interrupt status area */
	/* Read the CHDLC Configuration and obtain: 
	 *	Ptr to shared memory infor struct
         * Use this pointer to calculate the value of card->u.s.flags !
 	 */
	mb1->wan_data_len = 0;
	mb1->wan_command = L2_READ_CONFIGURATION;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb1);
	if(err != COMMAND_OK) {
		ss7_error(card, err, mb1);
		return -EIO;
	}

	/* ALEX Apr 8 2004 Sangoma ISA card */
	card->flags_off = 
			((L2_CONFIGURATION_STRUCT *)mb1->wan_data)->
			ptr_shared_mem_info_struct;

    	card->intr_type_off = 
			card->flags_off +
			offsetof(SHARED_MEMORY_INFO_STRUCT, interrupt_info_struct) +
			offsetof(INTERRUPT_INFORMATION_STRUCT, interrupt_type);
	card->intr_perm_off = 
			card->flags_off +
			offsetof(SHARED_MEMORY_INFO_STRUCT, interrupt_info_struct) +
			offsetof(INTERRUPT_INFORMATION_STRUCT, interrupt_permission);
	card->fe_status_off = 
			card->flags_off + 
			offsetof(SHARED_MEMORY_INFO_STRUCT, FT1_info_struct) + 
			offsetof(FT1_INFORMATION_STRUCT, parallel_port_A_input);
	
	/* This is for the ports link state */
	card->wandev.state = WAN_DUALPORT;
	card->u.s.state = WAN_DISCONNECTED;

	if (!card->wandev.piggyback){	
		int err;

		/* Perform interrupt testing */
		err = intr_test(card);

		if(err || (card->timer_int_enabled < MAX_INTR_TEST_COUNTER)) { 
			printk(KERN_INFO "%s: Interrupt test failed (%i)\n",
					card->devname, card->timer_int_enabled);
			printk(KERN_INFO "%s: Please choose another interrupt\n",
					card->devname);
			return -EIO;
		}
		
		printk(KERN_INFO "%s: Interrupt test passed (%i)\n", 
				card->devname, card->timer_int_enabled);
		card->configured = 1;
	}

	 if (ss7_set_intr_mode(card, APP_INT_ON_TIMER)){
		printk (KERN_INFO "%s: Failed to set interrupt triggers!\n",
				card->devname);
		return -EIO;	
        }
	 
	/* Mask the Timer interrupt */
	card->hw_iface.clear_bit(card->hw, card->intr_perm_off, APP_INT_ON_TIMER);
	
	printk(KERN_INFO "\n");

	card->wandev.state = WAN_DISCONNECTED;
	return 0;
}

/******* WAN Device Driver Entry Points *************************************/

/*============================================================================
 * Update device status & statistics
 * This procedure is called when updating the PROC file system and returns
 * various communications statistics. These statistics are accumulated from 3 
 * different locations:
 * 	1) The 'if_stats' recorded for the device.
 * 	2) Communication error statistics on the adapter.
 *      3) CHDLC operational statistics on the adapter.
 * The board level statistics are read during a timer interrupt. Note that we 
 * read the error and operational statistics during consecitive timer ticks so
 * as to minimize the time that we are inside the interrupt handler.
 *
 */
static int update (wan_device_t* wandev)
{
	sdla_t* card = wandev->priv;
 	netdevice_t* dev;
        volatile ss7_private_area_t* ss7_priv_area;
	unsigned long timeout;

	/* sanity checks */
	if((wandev == NULL) || (wandev->priv == NULL))
		return -EFAULT;
	
	if(wandev->state == WAN_UNCONFIGURED)
		return -ENODEV;

	/* more sanity checks */
        if(!card->flags_off)
                return -ENODEV;

	if(test_bit(PERI_CRIT, (void*)&card->wandev.critical))
                return -EAGAIN;

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if(dev == NULL)
		return -ENODEV;

	if((ss7_priv_area=dev->priv) == NULL)
		return -ENODEV;

       	if(ss7_priv_area->update_comms_stats){
		return -EAGAIN;
	}


	/* TE1	Change the update_comms_stats variable to 3,
	 * 	only for T1/E1 card, otherwise 2 for regular
	 *	S514/S508 card.
	 *	Each timer interrupt will update only one type
	 *	of statistics.
	 */
	ss7_priv_area->update_comms_stats = 
		(IS_TE1_CARD(card) || IS_56K_CARD(card)) ? 3 : 2;
	card->hw_iface.set_bit(card->hw, card->intr_perm_off, APP_INT_ON_TIMER);
	ss7_priv_area->timer_int_enabled = TMR_INT_ENABLED_UPDATE;
  
	/* wait a maximum of 1 second for the statistics to be updated */ 
        timeout = jiffies;
        for(;;) {
		if(ss7_priv_area->update_comms_stats == 0)
			break;
                if ((jiffies - timeout) > (1 * HZ)){
    			ss7_priv_area->update_comms_stats = 0;
 			ss7_priv_area->timer_int_enabled &=
				~TMR_INT_ENABLED_UPDATE; 
 			return -EAGAIN;
		}
        }

	return 0;
}

static int do_ioctl (netdevice_t *dev, struct ifreq *ifr, int cmd)
{

	ss7_private_area_t* ss7_priv_area = dev->priv;
	sdla_t *card;
	wan_mbox_t* mb;
	wan_mbox_t* usr_mb;
	wan_udp_pkt_t *wan_udp_pkt;
	unsigned long flags;
	unsigned long txbuf_off;
	L2_MSU_TX_STATUS_EL_STRUCT txbuf;
	unsigned int cur_msu_buf;
	int err=0,i;
	unsigned long smp_flags;
#ifdef SS7_IOCTL_INTERFACE
	api_tx_hdr_t* api_tx_hdr;
	struct sk_buff *skb;
#endif

	if (!ss7_priv_area)
		return -ENODEV;

	card = ss7_priv_area->card;

	mb = &card->wan_mbox;
	
	DEBUG_TEST("%s: Cmd 0x%x\n",__FUNCTION__,cmd);
	
	switch (cmd){

		case SIOC_WANPIPE_BIND_SK:
			if (!ifr){
				err= -EINVAL;
				break;
			}
			
			spin_lock_irqsave(&card->wandev.lock,smp_flags);
			err=wan_bind_api_to_svc(ss7_priv_area,ifr->ifr_data);
			spin_unlock_irqrestore(&card->wandev.lock,smp_flags);

			port_set_state(card,WAN_CONNECTING);
			break;

		case SIOC_WANPIPE_UNBIND_SK:
			if (!ifr){
				err= -EINVAL;
				break;
			}

			spin_lock_irqsave(&card->wandev.lock,smp_flags);
			err=wan_unbind_api_from_svc(ss7_priv_area,ifr->ifr_data);
			spin_unlock_irqrestore(&card->wandev.lock,smp_flags);

			break;

		case SIOC_WANPIPE_CHECK_TX:
		case SIOC_ANNEXG_CHECK_TX:
			err=0;
			break;

		case SIOC_WANPIPE_DEV_STATE:
			err = ss7_priv_area->common.state;
			break;

		case SIOC_ANNEXG_KICK:
			err=0;
			break;

		case SIOC_WANPIPE_PIPEMON: 
			
			if (atomic_read(&ss7_priv_area->udp_pkt_len) != 0){
				return -EBUSY;
			}
	
			atomic_set(&ss7_priv_area->udp_pkt_len,MAX_LGTH_UDP_MGNT_PKT);
			
			/* For performance reasons test the critical
			 * here before spin lock */
			if (test_bit(0,&card->in_isr)){
				atomic_set(&ss7_priv_area->udp_pkt_len,0);
				return -EBUSY;
			}
			
			wan_udp_pkt=(wan_udp_pkt_t*)ss7_priv_area->udp_pkt_data;
			if (copy_from_user(&wan_udp_pkt->wan_udp_hdr,ifr->ifr_data,sizeof(wan_udp_hdr_t))){
				atomic_set(&ss7_priv_area->udp_pkt_len,0);
				return -EFAULT;
			}

			spin_lock_irqsave(&card->wandev.lock, smp_flags);

			/* We have to check here again because we don't know
			 * what happened during spin_lock */
			if (test_bit(0,&card->in_isr)) {
				printk(KERN_INFO "%s:%s Pipemon command failed, Driver busy: try again.\n",
						card->devname,dev->name);
				atomic_set(&ss7_priv_area->udp_pkt_len,0);
				spin_unlock_irqrestore(&card->wandev.lock, smp_flags);
				return -EBUSY;
			}
			
			process_udp_mgmt_pkt(card,dev,ss7_priv_area);
			
			spin_unlock_irqrestore(&card->wandev.lock, smp_flags);

			/* This area will still be critical to other
			 * PIPEMON commands due to udp_pkt_len
			 * thus we can release the irq */
			
			if (atomic_read(&ss7_priv_area->udp_pkt_len) > sizeof(wan_udp_pkt_t)){
				printk(KERN_INFO "%s: Error: Pipemon buf too bit on the way up! %i\n",
						card->devname,atomic_read(&ss7_priv_area->udp_pkt_len));
				atomic_set(&ss7_priv_area->udp_pkt_len,0);
				return -EINVAL;
			}

			if (copy_to_user(ifr->ifr_data,&wan_udp_pkt->wan_udp_hdr,sizeof(wan_udp_hdr_t))){
				atomic_set(&ss7_priv_area->udp_pkt_len,0);
				return -EFAULT;
			}
			
			atomic_set(&ss7_priv_area->udp_pkt_len,0);
			return 0;


		case SIOCS_GENERAL_CMD:

			if (!ifr){
				return -EINVAL;
			}
		
			usr_mb = (wan_mbox_t*)ifr->ifr_ifru.ifru_data;
			if (!usr_mb){
				printk (KERN_INFO 
				"%s: Ioctl command %x, has no Mbox attached\n",
				card->devname, cmd);
				return -EINVAL;
			}


			if (copy_from_user(&ss7_priv_area->usr_cmd_mb.wan_command, 
					   &usr_mb->wan_command,SS7_CMD_BLOCK_SZ)){
				printk(KERN_INFO "%s: SS7 Cmd: Failed to copy mb \n",card->devname);
				return -EFAULT;
			}
	
			if (ss7_priv_area->usr_cmd_mb.wan_data_len){
				if (copy_from_user(&ss7_priv_area->usr_cmd_mb.wan_data[0], 
						   &usr_mb->wan_data[0],
						   ss7_priv_area->usr_cmd_mb.wan_data_len)){
					printk(KERN_INFO "%s: SS7 Cmd: Failed to copy mb data: len= %i\n",
							card->devname, ss7_priv_area->usr_cmd_mb.wan_data_len);
					return -EFAULT;
				}
			}
			
			spin_lock_irqsave(&card->wandev.lock,flags);

			memcpy(&mb->wan_command,&ss7_priv_area->usr_cmd_mb.wan_command,SS7_CMD_BLOCK_SZ);
			if (ss7_priv_area->usr_cmd_mb.wan_data_len){
				memcpy(&mb->wan_data[0], 
				       &ss7_priv_area->usr_cmd_mb.wan_data[0],
				       ss7_priv_area->usr_cmd_mb.wan_data_len);
			}

			if (mb->wan_command == L2_START && 
			    card->fe.fe_status != FE_CONNECTED &&
			    (IS_56K_CARD(card) || IS_TE1_CARD(card))){
				err = usr_mb->wan_return_code = 0xFF;

				DEBUG_EVENT("%s: Failed L2 Start Front End Disconnected!\n",
						__FUNCTION__);
				
				spin_unlock_irqrestore(&card->wandev.lock,flags);
				break;
			}

			err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
			
			DEBUG_TEST("%s: Cmd 0x%x, Rc 0x%x\n",
					__FUNCTION__,mb->wan_command,err);
			
			if (err != COMMAND_OK) {
				
				if (mb->wan_command == L2_START && err==L2_INVALID_STATE_FOR_CMND){
					port_set_state(card,WAN_CONNECTED);
				}
				
				spin_unlock_irqrestore(&card->wandev.lock,flags);
				break;
			}
			
			if (err == COMMAND_OK){
				if (mb->wan_command == L2_START){
					set_bit(1,&card->comm_enabled);
					port_set_state(card,WAN_CONNECTING);
					
				}else if (mb->wan_command == L2_RETRIEVAL_REQ_AND_FSNC){
					L2_RETRIEVAL_STRUCT *l2_ret=(L2_RETRIEVAL_STRUCT *)mb->wan_data;
					ss7_priv_area->ptr_first_MSU_bfr = 
						l2_ret->ptr_first_MSU_bfr;
					ss7_priv_area->number_MSUs =
						l2_ret->number_MSUs;
					printk(KERN_INFO "%s: (Debug): L2 Retreive Ptr %u, Bufs %i\n",
							card->devname,
							(u32)ss7_priv_area->ptr_first_MSU_bfr,
							ss7_priv_area->number_MSUs);
				}
			}
			
			
			memcpy(&ss7_priv_area->usr_cmd_mb.wan_command,&mb->wan_command,SS7_CMD_BLOCK_SZ);
			if (ss7_priv_area->usr_cmd_mb.wan_data_len){
				memcpy(&ss7_priv_area->usr_cmd_mb.wan_data[0],
				       &mb->wan_data[0], 
				       ss7_priv_area->usr_cmd_mb.wan_data_len);
			}

			spin_unlock_irqrestore(&card->wandev.lock,flags);
			
			/* copy the result back to our buffer */
			if (copy_to_user(&usr_mb->wan_command, 
					 &ss7_priv_area->usr_cmd_mb.wan_command,
					 SS7_CMD_BLOCK_SZ)){
				err= -EFAULT;
				break;
			}
			
			if (ss7_priv_area->usr_cmd_mb.wan_data_len) {
				if (copy_to_user(&usr_mb->wan_data[0], 
						 &ss7_priv_area->usr_cmd_mb.wan_data[0], 
						 ss7_priv_area->usr_cmd_mb.wan_data_len)){
					err= -EFAULT;
					break;
				}
	      		}

			break;

		case SIOCS_CHECK_FRONT_STATE:
			
			if (card->fe.fe_status == FE_CONNECTED){
				return 0;
			}else{
				return 1;
			}
			break;

		case SIOC_RETRIEVE_MSU_BUFS:
	
			if (!ifr){
				return -EINVAL;
			}
			
			usr_mb = (wan_mbox_t*)ifr->ifr_ifru.ifru_data;
			if (!usr_mb){
				printk (KERN_INFO 
				"%s: Ioctl command %x, has no Mbox attached\n",
				card->devname, cmd);
				return -EINVAL;
			}

			if (copy_from_user(&ss7_priv_area->usr_cmd_mb.wan_command, 
					   &usr_mb->wan_command,SS7_CMD_BLOCK_SZ)){
				printk(KERN_INFO "%s: SS7 Cmd: Failed to copy mb \n",card->devname);
				return -EFAULT;
			}
	
			if (ss7_priv_area->usr_cmd_mb.wan_data_len){
				if (copy_from_user(&ss7_priv_area->usr_cmd_mb.wan_data[0], 
						   &usr_mb->wan_data[0],
						   ss7_priv_area->usr_cmd_mb.wan_data_len)){
					printk(KERN_INFO "%s: SS7 Cmd: Failed to copy mb data: len= %i\n",
							card->devname, ss7_priv_area->usr_cmd_mb.wan_data_len);
					return -EFAULT;
				}
			}else{
				printk(KERN_INFO "%s: SS7 MSU Read: Invalid buffer length 0!\n",
						card->devname);
				return  -EINVAL;
			}
			
			spin_lock_irqsave(&card->wandev.lock,flags);

			memcpy(&mb->wan_command,&ss7_priv_area->usr_cmd_mb.wan_command,SS7_CMD_BLOCK_SZ);
			if (ss7_priv_area->usr_cmd_mb.wan_data_len){
				memcpy(&mb->wan_data[0], 
				       &ss7_priv_area->usr_cmd_mb.wan_data[0],
				       ss7_priv_area->usr_cmd_mb.wan_data_len);
			}

			cur_msu_buf=mb->wan_data[0];
			if (cur_msu_buf >= ss7_priv_area->number_MSUs){
				printk(KERN_INFO "%s: SS7 MSU Read: Invalid buffer number %i!\n",
						card->devname,cur_msu_buf);
				err = -EINVAL;
				spin_unlock_irqrestore(&card->wandev.lock,flags);
				break;
			}

			if (!ss7_priv_area->ptr_first_MSU_bfr){
				printk(KERN_INFO "%s: SS7 MSU Read: MSU buf ptr not initialized!\n",
						card->devname);
				err = -EINVAL;
				spin_unlock_irqrestore(&card->wandev.lock,flags);
				break;
			}

			txbuf_off = ss7_priv_area->ptr_first_MSU_bfr;

			for (i=0;i<=cur_msu_buf; i++){
				txbuf_off += sizeof(L2_MSU_TX_STATUS_EL_STRUCT);
				if (txbuf_off > card->u.s.txbuf_last_off){
					txbuf_off = card->u.s.txbuf_base_off;
				}
			}
			
			card->hw_iface.peek(card->hw, txbuf_off, &txbuf, sizeof(txbuf));
			card->hw_iface.peek(card->hw,
				  txbuf.ptr_data_bfr,
				  &mb->wan_data[0],
				  txbuf.frame_length);
			
			mb->wan_data_len = txbuf.frame_length;

			memcpy(&ss7_priv_area->usr_cmd_mb.wan_command,&mb->wan_command,SS7_CMD_BLOCK_SZ);
			if (ss7_priv_area->usr_cmd_mb.wan_data_len){
				memcpy(&ss7_priv_area->usr_cmd_mb.wan_data[0],
				       &mb->wan_data[0], 
				       ss7_priv_area->usr_cmd_mb.wan_data_len);
			}

			spin_unlock_irqrestore(&card->wandev.lock,flags);
			
			/* copy the result back to our buffer */
			if (copy_to_user(&usr_mb->wan_command, 
					 &ss7_priv_area->usr_cmd_mb.wan_command,
					 SS7_CMD_BLOCK_SZ)){
				err= -EFAULT;
				break;
			}
			
			if (ss7_priv_area->usr_cmd_mb.wan_data_len) {
				if (copy_to_user(&usr_mb->wan_data[0], 
						 &ss7_priv_area->usr_cmd_mb.wan_data[0], 
						 ss7_priv_area->usr_cmd_mb.wan_data_len)){
					err= -EFAULT;
					break;
				}
	      		}
			break;
			
#ifdef SS7_IOCTL_INTERFACE	
		case SIOC_SEND:

			if (!ifr){
				return -EINVAL;
			}
		
			usr_mb = (wan_mbox_t*)ifr->ifr_ifru.ifru_data;
			if (!usr_mb){
				printk (KERN_INFO 
				"%s: Ioctl command %x, has no Mbox attached\n",
				card->devname, cmd);
				return -EINVAL;
			}

			
			if (test_and_set_bit(SEND_CRIT, (void*)&card->wandev.critical)){
				return -EBUSY;
			}else{
				unsigned long txbuf_off = card->u.s.txbuf_off;
				L2_MSU_TX_STATUS_EL_STRUCT txbuf;
				card->hw_iface.peek(card->hw, txbuf_off, &txbuf, sizeof(txbuf));
				if (test_bit(0,&txbuf.opp_flag)){
					clear_bit(SEND_CRIT, (void*)&card->wandev.critical);
					return -EBUSY;
				}
			}
				
			if (!usr_mb->wan_data_len || usr_mb->wan_data_len <= sizeof(api_tx_hdr_t)){
				printk(KERN_INFO "Tx: Buffer Length =%i\n", usr_mb->wan_data_len);
				clear_bit(SEND_CRIT, (void*)&card->wandev.critical);
				return -ENODATA;
			}
			
			if (usr_mb->wan_data_len > dev->mtu){
				clear_bit(SEND_CRIT, (void*)&card->wandev.critical);
				return -EOVERFLOW;
			}

			if (copy_from_user(ss7_priv_area->tx_data,usr_mb->wan_data,usr_mb->wan_data_len)){
				clear_bit(SEND_CRIT, (void*)&card->wandev.critical);
				return -EFAULT;
			}

			api_tx_hdr = (api_tx_hdr_t *)ss7_priv_area->tx_data;
			ss7_priv_area->tx_len =  usr_mb->buffer_length - sizeof(api_tx_hdr_t);

			if (ss7_send(card, (ss7_priv_area->tx_data+sizeof(api_tx_hdr_t)), 
					    ss7_priv_area->tx_len, api_tx_hdr->SIO)){
				clear_bit(SEND_CRIT, (void*)&card->wandev.critical);
				return -EBUSY;
			}
			
			++card->wandev.stats.tx_packets;
			card->wandev.stats.tx_bytes += ss7_priv_area->tx_len;
			clear_bit(SEND_CRIT, (void*)&card->wandev.critical);
			return 0;
		
		case SIOC_RECEIVE_AVAILABLE:
			return atomic_read(&ss7_priv_area->bh_buff_used);
		
			
		case SIOC_RECEIVE_WAIT:

			if (!ifr){
				return -EINVAL;
			}
		
			usr_mb = (wan_mbox_t*)ifr->ifr_ifru.ifru_data;
			if (!usr_mb){
				printk (KERN_INFO 
				"%s: Ioctl command %x, has no Mbox attached\n",
				card->devname, cmd);
				return -EINVAL;
			}

			if (test_and_set_bit(RX_CRIT, (void*)&card->wandev.critical)){
				return -EBUSY;
			}
			
			if (atomic_read(&ss7_priv_area->bh_buff_used) > 0){
				goto rx_ss7_data;
			}else{
			
				DECLARE_WAITQUEUE(wait, current);
				err=-ENODATA;
				
				add_wait_queue(&ss7_priv_area->sleep, &wait);
				current->state = TASK_INTERRUPTIBLE;
				for (;;) {

					if (atomic_read(&ss7_priv_area->bh_buff_used) > 0){
						err=0;
						break;
					}
					
					if (signal_pending(current)){
						err = -ERESTARTSYS;
						break;
					}
					
					schedule();
				}
				current->state = TASK_RUNNING;
				remove_wait_queue(&ss7_priv_area->sleep, &wait);

				if (err == 0){
					goto rx_ss7_data;
				}else{
					clear_bit(RX_CRIT, (void*)&card->wandev.critical);
					return err;
				}
			}
			break;
			
		case SIOC_RECEIVE:
		
			if (!ifr){
				return -EINVAL;
			}
		
			usr_mb = (wan_mbox_t*)ifr->ifr_ifru.ifru_data;
			if (!usr_mb){
				printk (KERN_INFO 
				"%s: Ioctl command %x, has no Mbox attached\n",
				card->devname, cmd);
				return -EINVAL;
			}
			
			if (atomic_read(&ss7_priv_area->bh_buff_used) == 0){
				return -ENODATA;
			}

			if (test_and_set_bit(RX_CRIT, (void*)&card->wandev.critical)){
				return -EBUSY;
			}

rx_ss7_data:
			skb  = ((bh_data_t *)&ss7_priv_area->bh_head[ss7_priv_area->bh_read])->skb;

			if (skb == NULL){
				++card->wandev.stats.rx_errors;
				ss7_bh_cleanup(dev);
				clear_bit(RX_CRIT, (void*)&card->wandev.critical);
				return -ENOBUFS;
			}

			err = skb->len;

			if (copy_to_user(&usr_mb->wan_data[0], &skb->data[0], skb->len)){
				++card->wandev.stats.rx_errors;
				
				wan_dev_kfree_skb(skb, FREE_READ);
				ss7_bh_cleanup(dev);
				
				clear_bit(RX_CRIT, (void*)&card->wandev.critical);
				return -EFAULT;
			}
				
			++card->wandev.stats.rx_packets;
			card->wandev.stats.rx_bytes +=err;
			
			wan_dev_kfree_skb(skb, FREE_READ);	
			ss7_bh_cleanup(dev);
			
			clear_bit(RX_CRIT, (void*)&card->wandev.critical);
			return err;
#endif
		default:
			err = -EOPNOTSUPP;
			break;
	}

	return err;
}

/*============================================================================
 * Create new logical channel.
 * This routine is called by the router when ROUTER_IFNEW IOCTL is being
 * handled.
 * o parse media- and hardware-specific configuration
 * o make sure that a new channel can be created
 * o allocate resources, if necessary
 * o prepare network device structure for registaration.
 *
 * Return:	0	o.k.
 *		< 0	failure (channel will not be created)
 */
static int new_if (wan_device_t* wandev, netdevice_t* dev, wanif_conf_t* conf)
{
	sdla_t* card = wandev->priv;
	ss7_private_area_t* ss7_priv_area;


	printk(KERN_INFO "%s: Configuring Interface: %s\n",
			card->devname, conf->name);
 
	if ((conf->name[0] == '\0') || (strlen(conf->name) > WAN_IFNAME_SZ)) {
		printk(KERN_INFO "%s: Invalid interface name!\n",
			card->devname);
		return -EINVAL;
	}
		
	/* allocate and initialize private data */
	ss7_priv_area = kmalloc(sizeof(ss7_private_area_t), GFP_KERNEL);
	
	if(ss7_priv_area == NULL) 
		return -ENOMEM;

	memset(ss7_priv_area, 0, sizeof(ss7_private_area_t));

	ss7_priv_area->card = card; 

	wan_reg_api(ss7_priv_area,dev,card->devname);
	WAN_TASKLET_INIT((&ss7_priv_area->common.bh_task),0,ss7_bh,(unsigned long)ss7_priv_area);

	/* initialize data */
	strcpy(ss7_priv_area->if_name,conf->name);

	if(card->wandev.new_if_cnt > 0) {
                kfree(ss7_priv_area);
		return -EEXIST;
	}

	card->wandev.new_if_cnt++;

	ss7_priv_area->TracingEnabled = 0;
	ss7_priv_area->route_status = NO_ROUTE;
	ss7_priv_area->route_removed = 0;

	if ((ss7_priv_area->true_if_encoding = conf->true_if_encoding) == WANOPT_YES){
		printk(KERN_INFO 
			"%s: Enabling, true interface type encoding.\n",
			card->devname);
	}
	
#ifdef SS7_IOCTL_INTERFACE
	printk(KERN_INFO "%s: Running in IOCTL API mode !\n",
			wandev->name);

#if defined(LINUX_2_4)||defined(LINUX_2_6)
	init_waitqueue_head(&ss7_priv_area->sleep);
#else
	ss7_priv_area->sleep=NULL;
#endif
#else
	
	if (strcmp(conf->usedby, "API") == 0){
		ss7_priv_area->common.usedby=API;
		printk(KERN_INFO "%s: Running in API mode !\n",
			wandev->name);
	}else{
		printk(KERN_INFO "%s: Invalid operation mode. SS7 can only support API mode!\n",
			wandev->name);
	}
#endif
	
	init_timer(&ss7_priv_area->tx_timer);
	ss7_priv_area->tx_timer.function=tx_intr;
	ss7_priv_area->tx_timer.data=(unsigned long)card;

	dev->init = &if_init;
	dev->priv = ss7_priv_area;
	
	printk(KERN_INFO "\n");

	return 0;
}



static int del_if (wan_device_t* wandev, netdevice_t* dev)
{
	ss7_private_area_t* 	ss7_priv_area = dev->priv;
	sdla_t*			card = wandev->priv;

	WAN_TASKLET_KILL(&ss7_priv_area->common.bh_task);
	wan_unreg_api(ss7_priv_area, card->devname);
	
	return 0;
}


/****** Network Device Interface ********************************************/

/*============================================================================
 * Initialize Linux network interface.
 *
 * This routine is called only once for each interface, during Linux network
 * interface registration.  Returning anything but zero will fail interface
 * registration.
 */
static int if_init (netdevice_t* dev)
	{
	ss7_private_area_t* ss7_priv_area = dev->priv;
	sdla_t* card = ss7_priv_area->card;
	wan_device_t* wandev = &card->wandev;

	/* Initialize device driver entry points */
	dev->open		= &if_open;
	dev->stop		= &if_close;
	dev->hard_start_xmit	= &if_send;
	dev->get_stats		= &if_stats;
	dev->do_ioctl		= &do_ioctl;
#if defined(LINUX_2_4)||defined(LINUX_2_6)
	dev->tx_timeout		= &if_tx_timeout;
	dev->watchdog_timeo	= TX_TIMEOUT;
#endif

	
	/* Initialize media-specific parameters */
	dev->flags		|= IFF_POINTOPOINT;
	dev->flags		|= IFF_NOARP;

	/* Enable Mulitcasting if user selected */
	if (ss7_priv_area->mc == WANOPT_YES){
		dev->flags 	|= IFF_MULTICAST;
	}
	
	if (ss7_priv_area->true_if_encoding){
		dev->type	= ARPHRD_HDLC; /* This breaks the tcpdump */
	}else{
		dev->type	= ARPHRD_PPP;
	}
	
	dev->mtu		= card->wandev.mtu;
	/* for API usage, add the API header size to the requested MTU size */
	if(ss7_priv_area->common.usedby == API) {
		dev->mtu += sizeof(api_tx_hdr_t);
	}
 
	dev->hard_header_len	= 0;

	/* Initialize hardware parameters */
	dev->irq	= wandev->irq;
	dev->dma	= wandev->dma;
	dev->base_addr	= wandev->ioport;
	card->hw_iface.getcfg(card->hw, SDLA_MEMBASE, &dev->mem_start); //ALEX_TODAY wandev->maddr;
	card->hw_iface.getcfg(card->hw, SDLA_MEMEND, &dev->mem_end); //ALEX_TODAY wandev->maddr + wandev->msize - 1;

	/* Set transmit buffer queue length 
	 * If too low packets will not be retransmitted 
         * by stack.
	 */
        dev->tx_queue_len = 100;
   
	return 0;
}

/*============================================================================
 * Open network interface.
 * o enable communications and interrupts.
 * o prevent module from unloading by incrementing use count
 *
 * Return 0 if O.k. or errno.
 */
static int if_open (netdevice_t* dev)
{
	ss7_private_area_t* ss7_priv_area = dev->priv;
	sdla_t* card = ss7_priv_area->card;
	struct timeval tv;
	int err = 0;

	/* Only one open per interface is allowed */

	if (open_dev_check(dev))
		return -EBUSY;

	do_gettimeofday(&tv);
	ss7_priv_area->router_start_time = tv.tv_sec;

	netif_start_queue(dev);

	wanpipe_open(card);

	ss7_priv_area->timer_int_enabled |= TMR_INT_ENABLED_CONFIG;
	card->hw_iface.set_bit(card->hw, card->intr_perm_off, APP_INT_ON_TIMER);
	return err;
}

/*============================================================================
 * Close network interface.
 * o if this is the last close, then disable communications and interrupts.
 * o reset flags.
 */
static int if_close (netdevice_t* dev)
{
	ss7_private_area_t* ss7_priv_area = dev->priv;
	sdla_t* card = ss7_priv_area->card;

	stop_net_queue(dev);

#if defined(LINUX_2_1)
	dev->start=0;
#endif
	wanpipe_close(card);
	del_timer(&ss7_priv_area->tx_timer);

	/* TE1 - Unconfiging */
	if (IS_TE1_CARD(card)) {
		if (card->wandev.fe_iface.pre_release){
			card->wandev.fe_iface.pre_release(&card->fe);
		}
		if (card->wandev.fe_iface.unconfig){
			card->wandev.fe_iface.unconfig(&card->fe);
		}
	}
	return 0;
}

static void disable_comm (sdla_t *card)
{

	if (test_bit(0,&card->comm_enabled)){
		ss7_disable_comm_shutdown (card);
	}else{
		card->hw_iface.poke_byte(card->hw, card->intr_perm_off, 0x00);
	}
	return;
}


/*============================================================================
 * Handle transmit timeout event from netif watchdog
 */
static void if_tx_timeout (netdevice_t *dev)
{
    	ss7_private_area_t* chan = dev->priv;
	sdla_t *card = chan->card;
	
	/* If our device stays busy for at least 5 seconds then we will
	 * kick start the device by making dev->tbusy = 0.  We expect
	 * that our device never stays busy more than 5 seconds. So this                 
	 * is only used as a last resort.
	 */

	++card->wandev.stats.collisions;

	printk (KERN_INFO "%s: Transmit timed out on %s\n", card->devname,dev->name);
	netif_start_queue (dev);
}



/*============================================================================
 * Send a packet on a network interface.
 * o set tbusy flag (marks start of the transmission) to block a timer-based
 *   transmit from overlapping.
 * o check link state. If link is not up, then drop the packet.
 * o execute adapter send command.
 * o free socket buffer
 *
 * Return:	0	complete (socket buffer must be freed)
 *		non-0	packet may be re-transmitted (tbusy must be set)
 *
 * Notes:
 * 1. This routine is called either by the protocol stack or by the "net
 *    bottom half" (with interrupts enabled).
 * 2. Setting tbusy flag will inhibit further transmit requests from the
 *    protocol stack and can be used for flow control with protocol layer.
 */
static int if_send (struct sk_buff* skb, netdevice_t* dev)
{
	ss7_private_area_t *ss7_priv_area = dev->priv;
	sdla_t *card = ss7_priv_area->card;
	unsigned long smp_flags;
	int err=0;

#ifdef SS7_IOCTL_INTERFACE
	++card->wandev.stats.tx_errors;
	wan_dev_kfree_skb(skb,FREE_WRITE);
	start_net_queue(dev);
	return 0;
#endif

	
#if defined(LINUX_2_4)||defined(LINUX_2_6)
	netif_stop_queue(dev);
#endif
	
	if (skb == NULL){
		/* If we get here, some higher layer thinks we've missed an
		 * tx-done interrupt.
		 */
		printk(KERN_INFO "%s: interface %s got kicked!\n",
			card->devname, dev->name);

		wake_net_dev(dev);
		return 0;
	}

#if defined(LINUX_2_1)
	if (dev->tbusy){

		/* If our device stays busy for at least 5 seconds then we will
		 * kick start the device by making dev->tbusy = 0.  We expect 
		 * that our device never stays busy more than 5 seconds. So this
		 * is only used as a last resort. 
		 */
                ++card->wandev.stats.collisions;
		if((jiffies - ss7_priv_area->tick_counter) < (5 * HZ)) {
			return 1;
		}

		if_tx_timeout(dev);
	}
#endif

	/* Lock the 508 Card: SMP is supported */
      	if(card->type != SDLA_S514){
		s508_lock(card,&smp_flags);
	} 

	err=0;

    	if(test_and_set_bit(SEND_CRIT, (void*)&card->wandev.critical)) {
	
		printk(KERN_INFO "%s: Critical in if_send: %lx\n",
					card->wandev.name,card->wandev.critical);
                ++card->wandev.stats.tx_dropped;
		start_net_queue(dev);
		goto if_send_exit_crit;
	}

	if(card->wandev.state != WAN_CONNECTED){
       		++card->wandev.stats.tx_dropped;
		start_net_queue(dev);
		
	}else if(!skb->protocol){
        	++card->wandev.stats.tx_errors;
		start_net_queue(dev);
		
	}else {
		void* data = skb->data;
		unsigned len = skb->len;
		unsigned char SIO;

		/* If it's an API packet pull off the API
		 * header. Also check that the packet size
		 * is larger than the API header
	         */
		api_tx_hdr_t* api_tx_hdr;

		/* discard the frame if we are configured for */
		/* 'receive only' mode or if there is no data */
		if (len <= sizeof(api_tx_hdr_t)) {
			
			++card->wandev.stats.tx_dropped;
			start_net_queue(dev);
			goto if_send_exit_crit;
		}
				
		api_tx_hdr = (api_tx_hdr_t *)data;
		SIO = api_tx_hdr->SIO;
		data += sizeof(api_tx_hdr_t);
		len -= sizeof(api_tx_hdr_t);

		if((err=ss7_send(card, data, len, SIO))) {
			err=1;
			stop_net_queue(dev);
		}else{
			++card->wandev.stats.tx_packets;
                        card->wandev.stats.tx_bytes += len;
			
			start_net_queue(dev);
			
#if defined(LINUX_2_4)||defined(LINUX_2_6)
		 	dev->trans_start = jiffies;
#endif
		}	
	}

if_send_exit_crit:
	
	if (err==0) {
		wan_dev_kfree_skb(skb, FREE_WRITE);
	}else{

		/* All buffers are full, 
		 * Delay TX_INTR_TIMEOUT before re-triggering
		 * user API to tx more data */
		del_timer(&ss7_priv_area->tx_timer);
		ss7_priv_area->tx_timer.expires=jiffies+TX_INTR_TIMEOUT;
		add_timer(&ss7_priv_area->tx_timer);
	}

	clear_bit(SEND_CRIT, (void*)&card->wandev.critical);
	if(card->type != SDLA_S514){
		s508_unlock(card,&smp_flags);
	}
	
	return err;
}


/*============================================================================
 * Get ethernet-style interface statistics.
 * Return a pointer to struct enet_statistics.
 */
static struct net_device_stats* if_stats (netdevice_t* dev)
{
	sdla_t *my_card;
	ss7_private_area_t* ss7_priv_area;

	if ((ss7_priv_area=dev->priv) == NULL)
		return NULL;

	my_card = ss7_priv_area->card;
	return &my_card->wandev.stats; 
}

/****** Cisco HDLC Firmware Interface Functions *******************************/

/*============================================================================
 * Read firmware code version.
 *	Put code version as ASCII string in str. 
 */
static int ss7_read_version (sdla_t* card, char* str)
{
	wan_mbox_t* mb = &card->wan_mbox;
	int len;
	char err;
	
	mb->wan_data_len = 0;
	mb->wan_command = READ_SS7_CODE_VERSION;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);

	if(err != COMMAND_OK) {
		ss7_error(card,err,mb);
	}
	else if (str) {  /* is not null */
		len = mb->wan_data_len;
		memcpy(str, mb->wan_data, len);
		str[len] = '\0';
	}
	return (err);
}

/*-----------------------------------------------------------------------------
 *  Configure CHDLC firmware.
 */
static int ss7_configure (sdla_t* card, void* data)
{
	int err;
	wan_mbox_t *mbox = &card->wan_mbox;
	int data_length = sizeof(L2_CONFIGURATION_STRUCT);

	mbox->wan_data_len = data_length;  
	memcpy(mbox->wan_data, data, data_length);
	mbox->wan_command = L2_SET_CONFIGURATION;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mbox);
	
	if (err != COMMAND_OK) {
		ss7_error (card, err, mbox);
		printk(KERN_INFO "Error offset: 0x%02X\n", mbox->wan_data_len);               
	} 
	return err;
}


/*============================================================================
 * Set interrupt mode -- HDLC Version.
 */

static int ss7_set_intr_mode (sdla_t* card, unsigned mode)
{
	wan_mbox_t* mb = &card->wan_mbox;
	L2_INT_TRIGGERS_STRUCT* int_data =
		 (L2_INT_TRIGGERS_STRUCT *)mb->wan_data;
	int err;

	int_data->L2_interrupt_triggers 	= mode;
	int_data->IRQ				= card->wandev.irq; // ALEX_TODAY card->hw.irq;
	int_data->interrupt_timer               = 1;
   
	mb->wan_data_len = sizeof(L2_INT_TRIGGERS_STRUCT);
	mb->wan_command = L2_SET_INTERRUPT_TRIGGERS;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
	if (err != COMMAND_OK)
		ss7_error (card, err, mb);
	return err;
}


/*===========================================================
 * ss7_disable_comm_shutdown
 *
 * Shutdown() disables the communications. We must
 * have a sparate functions, because we must not
 * call ss7_error() hander since the private
 * area has already been replaced */

static int ss7_disable_comm_shutdown (sdla_t *card)
{
	wan_mbox_t* mb = &card->wan_mbox;
	L2_INT_TRIGGERS_STRUCT* int_data =
		 (L2_INT_TRIGGERS_STRUCT *)mb->wan_data;
	int err;

	/* Disable Interrutps */
	int_data->L2_interrupt_triggers 	= 0;
	int_data->IRQ				= card->wandev.irq; // ALEX_TODAY card->hw.irq;
	int_data->interrupt_timer               = 1;
   
	mb->wan_data_len = sizeof(L2_INT_TRIGGERS_STRUCT);
	mb->wan_command = L2_SET_INTERRUPT_TRIGGERS;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);

	
	mb->wan_command = L2_STOP;
	mb->wan_data_len = 0;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
	
	clear_bit(0,&card->comm_enabled);	

	return 0;
}

static int ss7_stop(sdla_t *card)
{
	wan_mbox_t* mb = &card->wan_mbox;
	int err;

	if (!test_bit(1,&card->comm_enabled)){
		return 0;
	}

	mb->wan_command = L2_STOP;
	mb->wan_data_len = 0;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
	
	clear_bit(1,&card->comm_enabled);

	return err;
}



/*============================================================================
 * Enable communications.
 */

static int ss7_comm_enable (sdla_t* card)
{
	int err=0;
	wan_mbox_t* mb = &card->wan_mbox;

	if (test_bit(0,&card->comm_enabled))
		return 0;

	mb->wan_data_len = 0;
	mb->wan_command = L2_POWER_ON;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
	if (err != COMMAND_OK){
		ss7_error(card, err, mb);
		return err;
	}

	set_bit(0,&card->comm_enabled);
	
	return err;
}

/*============================================================================
 * Read communication error statistics.
 */
static int ss7_read_comm_err_stats (sdla_t* card)
{
        int err;
        wan_mbox_t* mb = &card->wan_mbox;

        mb->wan_data_len = 0;
        mb->wan_command = READ_COMMS_ERROR_STATS;
        err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
        if (err != COMMAND_OK)
                ss7_error(card,err,mb);
        return err;
}


/*============================================================================
 * Read CHDLC operational statistics.
 */
static int ss7_read_op_stats (sdla_t* card)
{
        int err;
        wan_mbox_t* mb = &card->wan_mbox;

        mb->wan_data_len = 0;
        mb->wan_command = L2_READ_OPERATIONAL_STATS;
        err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
        if (err != COMMAND_OK)
                ss7_error(card,err,mb);
        return err;
}


/*============================================================================
 * Update communications error and general packet statistics.
 */
static int update_comms_stats(sdla_t* card,
	ss7_private_area_t* ss7_priv_area)
{
        wan_mbox_t* mb = &card->wan_mbox;
  	COMMS_ERROR_STATS_STRUCT* err_stats;
        L2_OPERATIONAL_STATS_STRUCT *op_stats;

	/* on the first timer interrupt, read the comms error statistics */
	if(ss7_priv_area->update_comms_stats == 3) {

		/* 1. On the first timer interrupt, update T1/E1 alarms 
		 * and PMON counters (only for T1/E1 card) (TE1) 
		 */
		/* TE1 Update T1/E1 alarms */
		if (IS_TE1_CARD(card)) {	
			card->wandev.fe_iface.read_alarm(&card->fe, 0); 
			/* TE1 Update T1/E1 perfomance counters */
			card->wandev.fe_iface.read_pmon(&card->fe, 0); 
		}else if (IS_56K_CARD(card)) {
			/* 56K Update CSU/DSU alarms */
			card->wandev.fe_iface.read_alarm(&card->fe, 1); 
		}

	}else if(ss7_priv_area->update_comms_stats == 2) {
		if(ss7_read_comm_err_stats(card))
			return 1;
		err_stats = (COMMS_ERROR_STATS_STRUCT *)mb->wan_data;
		card->wandev.stats.rx_over_errors = 
				err_stats->Rx_overrun_err_count;
		card->wandev.stats.rx_crc_errors = 
				err_stats->CRC_err_count;
		card->wandev.stats.rx_frame_errors = 
				err_stats->Rx_abort_count;
		card->wandev.stats.rx_fifo_errors = 
				err_stats->Rx_dis_pri_bfrs_full_count; 
		card->wandev.stats.rx_missed_errors =
				card->wandev.stats.rx_fifo_errors;
	//????????/
	//	card->wandev.stats.tx_aborted_errors =
	//			err_stats->sec_Tx_abort_count;
	}
        /* on the second timer interrupt, read the operational statistics */
	else {
        	if(ss7_read_op_stats(card))
                	return 1;
		op_stats = (L2_OPERATIONAL_STATS_STRUCT *)mb->wan_data;
		card->wandev.stats.rx_length_errors=0;
#if 0
		card->wandev.stats.rx_length_errors =
			(op_stats->Rx_MSU_discard_short_count +
			op_stats->Rx_MSU_discard_long_count);
#endif
	}

	return 0;
}

/*============================================================================
 * Send packet.
 *	Return:	0 - o.k.
 *		1 - no transmit buffers available
 */
static int ss7_send (sdla_t* card, void* data, unsigned len, unsigned char SIO)
{
	unsigned long txbuf_off = card->u.s.txbuf_off;
	L2_MSU_TX_STATUS_EL_STRUCT txbuf;

	card->hw_iface.peek(card->hw, txbuf_off, &txbuf, sizeof(txbuf));
	if (txbuf.opp_flag)
		return 1;
	
	card->hw_iface.poke(card->hw, txbuf.ptr_data_bfr, data, len);

	txbuf.frame_length = len;
	txbuf.SIO = SIO;
	txbuf.opp_flag = 1;		/* start transmission */
	card->hw_iface.poke(card->hw, txbuf_off, &txbuf, sizeof(txbuf));
	
	/* Update transmit buffer control fields */
	card->u.s.txbuf_off += sizeof(L2_MSU_TX_STATUS_EL_STRUCT);
	
	if (card->u.s.txbuf_off > card->u.s.txbuf_last_off)
		card->u.s.txbuf_off = card->u.s.txbuf_base_off;
	return 0;
}





/****** Firmware Error Handler **********************************************/

/*============================================================================
 * Firmware error handler.
 *	This routine is called whenever firmware command returns non-zero
 *	return code.
 *
 * Return zero if previous command has to be cancelled.
 */
static int ss7_error (sdla_t *card, int err, wan_mbox_t *mb)
{
	unsigned cmd = mb->wan_command;

	switch (err) {

	case CMD_TIMEOUT:
		printk(KERN_INFO "%s: command 0x%02X timed out!\n",
			card->devname, cmd);
		break;

#if 0
	---- SECONDARY PORT NOT ENABLED -------
	case S514_BOTH_PORTS_SAME_CLK_MODE:
		if(cmd == L2_SET_CONFIGURATION) {
			printk(KERN_INFO
			 "%s: Configure both ports for the same clock source\n",
				card->devname);
			break;
		}
#endif
		
	default:
		printk(KERN_INFO "%s: command 0x%02X returned 0x%02X!\n",
			card->devname, cmd, err);
	}

	return 0;
}


/********** Bottom Half Handlers ********************************************/

/* NOTE: There is no API, BH support for Kernels lower than 2.2.X.
 *       DO NOT INSERT ANY CODE HERE, NOTICE THE 
 *       PREPROCESSOR STATEMENT ABOVE, UNLESS YOU KNOW WHAT YOU ARE
 *       DOING */

static void ss7_bh (unsigned long data)
{
	ss7_private_area_t* chan = (ss7_private_area_t*)data;
	sdla_t *card = chan->card;
	struct sk_buff *skb;
	int len=0;

	while ((skb=wan_api_dequeue_skb(chan)) != NULL){

		len = skb->len;
		if (wan_api_rx(chan,skb) == 0){
			card->wandev.stats.rx_packets++;
			card->wandev.stats.rx_bytes += len;
		}else{
			++card->wandev.stats.rx_dropped;
			wan_skb_free(skb);
		}
	}	
	
	WAN_TASKLET_END(&chan->common.bh_task);

	return;
}


/*===============================================================
 *  alloc_and_init_skb_buf 
 *
 *	Allocate and initialize an skb buffer. 
 *
 *===============================================================*/

static int alloc_and_init_skb_buf (sdla_t *card, struct sk_buff **skb, int len)
{
	struct sk_buff *new_skb = *skb;

	new_skb = dev_alloc_skb(len);
	if (new_skb == NULL){
		printk(KERN_INFO "%s: no socket buffers available!\n",
			card->devname);
		return 1;
	}

	if (skb_tailroom(new_skb) < len){
		/* No room for the packet. Call off the whole thing! */
                wan_dev_kfree_skb(new_skb, FREE_READ);
		printk(KERN_INFO "%s: Listen: unexpectedly long packet sequence\n"
			,card->devname);
		*skb = NULL;
		return 1;
	}

	*skb = new_skb;
	return 0;

}


/*===============================================================
 * send_oob_msg
 *
 *    Construct an NEM Message and pass it up the connected
 *    sock. If the sock is not bounded discard the NEM.
 *
 *===============================================================*/

static void send_oob_msg (sdla_t *card, netdevice_t *dev)
{
	wan_mbox_t *usr_mb;
	wan_mbox_t *mbox = &card->wan_mbox;
	ss7_private_area_t *chan = dev->priv;
	struct sk_buff *skb;
	int len=sizeof(wan_mbox_t)-SIZEOF_MB_DATA_BFR+mbox->wan_data_len;


#ifdef SS7_IOCTL_INTERFACE
	return;
#endif

	if (!chan->common.sk){
		return;
	}
	
	if (alloc_and_init_skb_buf(card,&skb,len)){
		DEBUG_EVENT("%s: OOB MSG: No sock buffers\n",card->devname);
		return;
	}

	usr_mb = (wan_mbox_t*)skb_put(skb,len); 

	memcpy(&usr_mb->wan_command,&mbox->wan_command,SS7_CMD_BLOCK_SZ);

	if (mbox->wan_data_len > 0){
		memcpy(&usr_mb->wan_data[0], &mbox->wan_data[0], mbox->wan_data_len);
	}

	usr_mb->wan_return_code = chan->L2_state;
	
	wan_skb_reset_mac_header(skb);
	skb->pkt_type = WAN_PACKET_ERR;
	skb->dev=chan->common.dev;
	skb->protocol=htons(SS7_PROT);
	
	if (wanpipe_api_sock_rx(skb,chan->common.dev,chan->common.sk) != 0){
		printk(KERN_INFO "%s: Socket OOB queue full, dropping OOB MSG!\n",
				card->devname);
               	wan_skb_free(skb);
	}

//	printk(KERN_INFO "%s: OOB MSG OK, %s, Cmd %x\n",
//			card->devname, dev->name, mbox->command);
}	


/****** Interrupt Handlers **************************************************/

/*============================================================================
 * Cisco HDLC interrupt service routine.
 */
static WAN_IRQ_RETVAL wpc_isr (sdla_t* card)
{
	netdevice_t* dev;
	SHARED_MEMORY_INFO_STRUCT flags;
	int i, interrupt_serviced = 0;
	//sdla_t *my_card;


	/* Check for which port the interrupt has been generated
	 * Since Secondary Port is piggybacking on the Primary
         * the check must be done here. 
	 */

#if 0
	flags = card->u.s.flags;
	if (!flags->interrupt_info_struct.interrupt_type){
		/* Check for a second port (piggybacking) */
		if((my_card = card->next)){
			flags = my_card->u.s.flags;
			if (flags->interrupt_info_struct.interrupt_type){
				card = my_card;
			}
		}
	}
#endif
	
	card->hw_iface.peek(card->hw, card->flags_off, &flags, sizeof(flags));
	card->in_isr = 1;
	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));

	/* If we get an interrupt with no network device, stop the interrupts
	 * and issue an error */
	if (!dev && 
	    flags.interrupt_info_struct.interrupt_type != COMMAND_COMPLETE_APP_INT_PEND){
		printk(KERN_INFO "%s: Error: Interrupt with no device!\n",
				card->devname);
		goto rx_intr_done;
	}
	
	/* if critical due to peripheral operations
	 * ie. update() or getstats() then reset the interrupt and
	 * wait for the board to retrigger.
	 */
	if(test_bit(PERI_CRIT, (void*)&card->wandev.critical)) {
		goto rx_intr_done;
	}


	/* On a 508 Card, if critical due to if_send 
         * Major Error !!! */
	if(card->type != SDLA_S514) {
		if(test_bit(SEND_CRIT, (void*)&card->wandev.critical)) {
			printk(KERN_INFO "%s: Critical while in ISR: %lx\n",
				card->devname, card->wandev.critical);
			card->in_isr = 0;
			card->hw_iface.poke_byte(card->hw, card->intr_type_off, 0x00);
			WAN_IRQ_RETURN(WAN_IRQ_HANDLED);
		}
	}

	switch(flags.interrupt_info_struct.interrupt_type) {

	case RX_APP_INT_PEND:	/* 0x01: receive interrupt */
		interrupt_serviced = 1;
		rx_intr(card);
		break;

	case TX_APP_INT_PEND:	/* 0x02: transmit interrupt */
		interrupt_serviced = 1;
		card->hw_iface.clear_bit(card->hw, card->intr_perm_off, APP_INT_ON_TX_FRAME);

		if (is_queue_stopped(dev)){
			ss7_private_area_t *chan=dev->priv;
			if (chan->common.usedby == API){
				start_net_queue(dev);
				wan_wakeup_api(chan);
			}else{
				wake_net_dev(dev);
			}
		}
		break;

	case COMMAND_COMPLETE_APP_INT_PEND:/* 0x04: cmd cplt */
		interrupt_serviced = 1;
		++ card->timer_int_enabled;
		break;

	case L2_EXCEP_COND_APP_INT_PEND:	/* 0x20 */
		interrupt_serviced = 1;
		process_ss7_exception(card);
		send_oob_msg(card,dev);
		break;

	case GLOBAL_EXCEP_COND_APP_INT_PEND:
		interrupt_serviced = 1;
		process_global_exception(card);

		/* Reset the 56k or T1/E1 front end exception condition */
		if(IS_56K_CARD(card) || IS_TE1_CARD(card)) {
			FRONT_END_STATUS_STRUCT FE_status;
			card->hw_iface.peek(card->hw, 
					    card->fe_status_off,
					    &FE_status, 
					    sizeof(FE_status));
			FE_status.opp_flag = 0x01;	
			card->hw_iface.poke(card->hw, 
					    card->fe_status_off,
				            &FE_status,
					    sizeof(FE_status));
			break;
		}
		break;

	case TIMER_APP_INT_PEND:
		interrupt_serviced = 1;
		timer_intr(card);
		break;

	default:
		break;
	}	

	if(!interrupt_serviced) {
		printk(KERN_INFO "%s: spurious interrupt 0x%02X!\n", 
			card->devname,
			flags.interrupt_info_struct.interrupt_type);
		printk(KERN_INFO "Code name: ");
		for(i = 0; i < 4; i ++)
			printk("%c",
				flags.global_info_struct.code_name[i]); 
		printk("\n");
		
		printk(KERN_INFO "Code version: ");
	 	for(i = 0; i < 4; i ++)
			printk("%c", 
				flags.global_info_struct.code_version[i]); 
		printk("\n");	
	}

rx_intr_done:

	card->in_isr = 0;
	card->hw_iface.poke_byte(card->hw, card->intr_type_off, 0x00);
	WAN_IRQ_RETURN(WAN_IRQ_HANDLED); 
}

/*============================================================================
 * Receive interrupt handler.
 */
static void rx_intr (sdla_t* card)
{
	netdevice_t *dev;
	ss7_private_area_t *ss7_priv_area;
	api_rx_hdr_t*	api_rx_hdr = NULL;
	SHARED_MEMORY_INFO_STRUCT flags;
	unsigned long rxbuf_off = card->u.s.rxmb_off;
	L2_MSU_RX_STATUS_EL_STRUCT rxbuf;
	unsigned long addr;
	struct sk_buff *skb;
	unsigned len;
	void *buf;
	int i;
	
	card->hw_iface.peek(card->hw, card->flags_off, &flags, sizeof(flags));
	card->hw_iface.peek(card->hw, rxbuf_off, &rxbuf, sizeof(rxbuf));
	addr = rxbuf.ptr_data_bfr;
	if (rxbuf.opp_flag != 0x01) {
		printk(KERN_INFO 
			"%s: corrupted Rx buffer @ 0x%lX, flag = 0x%02X!\n", 
			card->devname, rxbuf_off, rxbuf.opp_flag);
                printk(KERN_INFO "Code name: ");
                for(i = 0; i < 4; i ++)
                        printk(KERN_INFO "%c",
                                flags.global_info_struct.code_name[i]);
                printk(KERN_INFO "\nCode version: ");
                for(i = 0; i < 4; i ++)
                        printk(KERN_INFO "%c",
                                flags.global_info_struct.code_version[i]);
                printk(KERN_INFO "\n");


		/* Bug Fix: Mar 6 2000
                 * If we get a corrupted mailbox, it measn that driver 
                 * is out of sync with the firmware. There is no recovery.
                 * If we don't turn off all interrupts for this card
                 * the machine will crash. 
                 */
		printk(KERN_INFO "%s: Critical router failure ...!!!\n", card->devname);
		printk(KERN_INFO "Please contact Sangoma Technologies !\n");
		ss7_set_intr_mode(card,0);	
		return;
	}

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if (!dev){
		goto rx_exit;
	}

	if (!is_dev_running(dev))
		goto rx_exit;

	ss7_priv_area = dev->priv;

	len  = rxbuf.frame_length;
	
	/* Allocate socket buffer */
	skb = dev_alloc_skb(len+sizeof(api_rx_hdr_t));

	if (skb == NULL) {
		printk(KERN_INFO "%s: no socket buffers available!\n",
					card->devname);
		++card->wandev.stats.rx_dropped;
		goto rx_exit;
	}

	buf = skb_put(skb, len);
	card->hw_iface.peek(card->hw, addr, buf, len);

	skb_push(skb, sizeof(api_rx_hdr_t));
	api_rx_hdr = (api_rx_hdr_t*)&skb->data[0x00];
	api_rx_hdr->SIO = rxbuf.SIO;
	api_rx_hdr->time_stamp = rxbuf.time_stamp;

	skb->protocol = htons(PVC_PROT);
	wan_skb_reset_mac_header(skb);
	skb->dev      = dev;
	skb->pkt_type = WAN_PACKET_DATA;

	if (wan_api_enqueue_skb(ss7_priv_area,skb) < 0){
		wan_skb_free(skb);
		++card->wandev.stats.rx_dropped;
		goto rx_exit;
	}

#ifdef SS7_IOCTL_INTERFACE
	wake_up_interruptible(&ss7_priv_area->sleep);
#else
	WAN_TASKLET_SCHEDULE(&ss7_priv_area->common.bh_task);
#endif

rx_exit:
	/* Release buffer element and calculate a pointer to the next one */
	rxbuf.opp_flag = 0x00;
	card->hw_iface.poke(card->hw, rxbuf_off, &rxbuf, sizeof(rxbuf));
	card->u.s.rxmb_off += sizeof(L2_MSU_RX_STATUS_EL_STRUCT);
	if (card->u.s.rxmb_off > card->u.s.rxbuf_last_off){
		card->u.s.rxmb_off = card->u.s.rxbuf_base_off;
	}
}

/*============================================================================
 * Timer interrupt handler.
 * The timer interrupt is used for two purposes:
 *    1) Processing udp calls from 'cpipemon'.
 *    2) Reading board-level statistics for updating the proc file system.
 */
void timer_intr(sdla_t *card)
{
        netdevice_t* dev;
        ss7_private_area_t* ss7_priv_area = NULL;

        dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if (!dev) return;
        ss7_priv_area = dev->priv;

	if (ss7_priv_area->timer_int_enabled & TMR_INT_ENABLED_CONFIG) {
		config_ss7(card);
		ss7_priv_area->timer_int_enabled &= ~TMR_INT_ENABLED_CONFIG;
#ifdef SHOW_L2_HIST
		ss7_priv_area->timer_int_enabled |= TMR_INT_ENABLED_L2_HIST;
		ss7_priv_area->L2_hist_counter = 0;
#endif
	}

	/* process a udp call if pending */
       	if(ss7_priv_area->timer_int_enabled & TMR_INT_ENABLED_UDP) {
               	process_udp_mgmt_pkt(card, dev,
                       ss7_priv_area);
		ss7_priv_area->timer_int_enabled &= ~TMR_INT_ENABLED_UDP;
        }

	/* read the communications statistics if required */
	if(ss7_priv_area->timer_int_enabled & TMR_INT_ENABLED_UPDATE) {
		update_comms_stats(card, ss7_priv_area);
                if(!(-- ss7_priv_area->update_comms_stats)) {
			ss7_priv_area->timer_int_enabled &= 
				~TMR_INT_ENABLED_UPDATE;
		}
        }

	if(ss7_priv_area->timer_int_enabled & TMR_INT_ENABLED_L2_HIST) {
		if((++ ss7_priv_area->L2_hist_counter) == 50) {
			ss7_priv_area->L2_hist_counter = 0; 
			L2_read_hist_table(card);
		}
	}

	/* TE timer interrupt */
	if (ss7_priv_area->timer_int_enabled & TMR_INT_ENABLED_TE) {
		card->wandev.fe_iface.polling(&card->fe);
		ss7_priv_area->timer_int_enabled &= ~TMR_INT_ENABLED_TE;
	}

	/* only disable the timer interrupt if there are no udp or statistic */
	/* updates pending */
        if(!ss7_priv_area->timer_int_enabled) {
		card->hw_iface.clear_bit(card->hw, card->intr_perm_off, APP_INT_ON_TIMER);
        }
}


/*============================================================================
 * Enable timer interrupt  
 */
static void ss7_enable_timer (void* card_id)
{
	sdla_t* 		card = (sdla_t*)card_id;
	netdevice_t* 		dev = NULL;
        ss7_private_area_t* 	ss7_priv_area = NULL;

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if (dev == NULL || ((ss7_priv_area=dev->priv) == NULL))
		return;

	ss7_priv_area->timer_int_enabled |= TMR_INT_ENABLED_TE;
	card->hw_iface.set_bit(card->hw, card->intr_perm_off, APP_INT_ON_TIMER);
	return;
}


/*------------------------------------------------------------------------------
  Miscellaneous Functions
	- set_ss7_config() used to set configuration options on the board
------------------------------------------------------------------------------*/

static int set_ss7_config(sdla_t* card)
{
	L2_CONFIGURATION_STRUCT cfg;

	memset(&cfg, 0, sizeof(L2_CONFIGURATION_STRUCT));

	if(card->wandev.clocking){
		cfg.baud_rate = card->wandev.bps;
	}
		
	cfg.line_config_options = (card->wandev.electrical_interface == WANOPT_RS232) ?
		INTERFACE_LEVEL_RS232 : INTERFACE_LEVEL_V35;

	cfg.modem_config_options	= card->u.s.modem_cfg_opt;
	cfg.modem_status_timer		= card->u.s.modem_status_timer;
	cfg.L2_API_options		= card->u.s.api_options;

	cfg.L2_protocol_options		= card->u.s.protocol_options;
	cfg.L2_protocol_options 	= 0; //AUTO_START_WHEN_OUT_OF_SERVICE;

	cfg.L2_protocol_specification	= card->u.s.protocol_specification;
	cfg.L2_stats_history_options	= card->u.s.stats_history_options;
	cfg.max_length_MSU_SIF		= card->u.s.max_length_msu_sif; 
	cfg.max_unacked_Tx_MSUs		= card->u.s.max_unacked_tx_msus; 
	cfg.link_inactivity_timer       = card->u.s.link_inactivity_timer;
	cfg.T1_timer			= card->u.s.t1_timer;
	cfg.T2_timer 			= card->u.s.t2_timer;
	cfg.T3_timer 			= card->u.s.t3_timer;
	cfg.T4_timer_emergency		= card->u.s.t4_timer_emergency;
	cfg.T4_timer_normal		= card->u.s.t4_timer_normal;
	cfg.T5_timer			= card->u.s.t5_timer;
	cfg.T6_timer			= card->u.s.t6_timer;
	cfg.T7_timer 			= card->u.s.t7_timer;
	cfg.T8_timer			= card->u.s.t8_timer;
	cfg.N1				= card->u.s.n1;
	cfg.N2				= card->u.s.n2;
	cfg.Tin				= card->u.s.tin;
	cfg.Tie				= card->u.s.tie;
	cfg.SUERM_error_threshold	= card->u.s.suerm_error_threshold;
	cfg.SUERM_number_octets		= card->u.s.suerm_number_octets;
	cfg.SUERM_number_SUs		= card->u.s.suerm_number_sus;
	cfg.SIE_interval_timer		= card->u.s.sie_interval_timer;
	cfg.SIO_interval_timer		= card->u.s.sio_interval_timer;
	cfg.SIOS_interval_timer		= card->u.s.sios_interval_timer;
	cfg.FISU_interval_timer		= card->u.s.fisu_interval_timer;

#if 0
	{
		int i;
	printk(KERN_INFO "SS7 Config:\n");
	for (i=0;i<sizeof(L2_CONFIGURATION_STRUCT);i++){
		printk(KERN_INFO "0x%02X: 0x%02X\n",i,*((unsigned char*)&cfg+i));
	}
	}
#endif	
	
	return ss7_configure(card, &cfg);
}


/*============================================================================
 * Process global exception condition
 */
static int process_global_exception(sdla_t *card)
{
	netdevice_t	*dev;
	wan_mbox_t	*mbox = &card->wan_mbox;
	int		err;

	mbox->wan_data_len = 0;
	mbox->wan_command = READ_GLOBAL_EXCEPTION_CONDITION;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mbox);

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	send_oob_msg(card, dev);

	if(err != CMD_TIMEOUT ){
	
		switch(mbox->wan_return_code) {
         
	      	case EXCEP_MODEM_STATUS_CHANGE:

			
			if (IS_56K_CARD(card)) {

				FRONT_END_STATUS_STRUCT FE_status;
			       
				card->hw_iface.peek(card->hw,
					            card->fe_status_off, 
						    &FE_status,
						    sizeof(FE_status));

				card->fe.fe_param.k56_param.RR8_reg_56k = 
					FE_status.FE_U.stat_56k.RR8_56k;	
				card->fe.fe_param.k56_param.RRA_reg_56k = 
					FE_status.FE_U.stat_56k.RRA_56k;	
				card->fe.fe_param.k56_param.RRC_reg_56k = 
					FE_status.FE_U.stat_56k.RRC_56k;	
				card->wandev.fe_iface.read_alarm(&card->fe, 0); 

				ss7_handle_front_end_state(card);
				break;
			
			}
			
			if (IS_TE1_CARD(card)) {

				/* TE1 T1/E1 interrupt */
				card->wandev.fe_iface.isr(&card->fe);
				ss7_handle_front_end_state(card);
				break;
			}	


			if ((mbox->wan_data[0] & (DCD_HIGH | CTS_HIGH)) == (DCD_HIGH | CTS_HIGH)){
				card->fe.fe_status = FE_CONNECTED;
			}else{
				card->fe.fe_status = FE_DISCONNECTED;
			}

			printk(KERN_INFO "%s: Modem status change\n",
				card->devname);

			switch(mbox->wan_data[0] & (DCD_HIGH | CTS_HIGH)) {
				case (DCD_HIGH):
					printk(KERN_INFO "%s: DCD high, CTS low\n",card->devname);
					break;
					
				case (CTS_HIGH):
                                        printk(KERN_INFO "%s: DCD low, CTS high\n",card->devname);
					break;
					
                                case ((DCD_HIGH | CTS_HIGH)):
                                        printk(KERN_INFO "%s: DCD high, CTS high\n",card->devname);
                                        break;
					
				default:
                                        printk(KERN_INFO "%s: DCD low, CTS low\n",card->devname);
                                        break;
			}
			
			ss7_handle_front_end_state(card);
			break;
			
                default:
                        printk(KERN_INFO "%s: Global exception %x\n",
				card->devname, mbox->wan_return_code);
                        break;
                }
	}

	return 0;
}


/*============================================================================
 * Process ss7 exception condition
 */
static int process_ss7_exception(sdla_t *card)
{
	wan_mbox_t		*mb = &card->wan_mbox;
	netdevice_t		*dev;
	ss7_private_area_t	*ss7_priv_area;
	int err;

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if (!dev) return -EINVAL;	
	ss7_priv_area = dev->priv;
	mb->wan_data_len = 0;
	mb->wan_command = L2_READ_EXCEPTION_CONDITION;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
	
	ss7_priv_area->L2_state = mb->wan_return_code;
	
	if(err != CMD_TIMEOUT) {
	
		switch (err) {

		case L2_EXCEP_IN_SERVICE:
			card->u.s.state=WAN_CONNECTED;
			if (card->wandev.ignore_front_end_status == WANOPT_YES ||
			    card->fe.fe_status == FE_CONNECTED){
				port_set_state(card, WAN_CONNECTED);
			}
			break;

		case L2_EXCEP_OUT_OF_SERVICE:
			/*NC: Jul 30 2003 
			 *  The sock state should be controled
			 *    by the user not the driver 
			 *port_set_state(card, WAN_DISCONNECTED); */
			DEBUG_EVENT("%s: Out of service Qualifier = 0x%02X\n",
					card->devname,mb->wan_data[0]);
			break;

		case L2_EXCEP_REMOTE_PROC_OUTAGE:
			printk(KERN_INFO "%s: Remote processor outage\n",
						card->devname);
			break;

		case L2_EXCEP_REMOTE_PROC_RECOVERED:
			printk(KERN_INFO "%s: Remote processor recovered\n",
						card->devname);
			break;

		case L2_EXCEP_RTB_CLEARED:
			printk(KERN_INFO "%s: RTB cleared\n",
						card->devname);
			break;

		case L2_EXCEP_RB_CLEARED:
			printk(KERN_INFO "%s: RB cleared\n", card->devname);
			break;

		case L2_EXCEP_BSNT:
			printk(KERN_INFO "%s: BSNT\n", card->devname);
			break;

		case L2_EXCEP_RETRIEVAL_COMPLETE:
			printk(KERN_INFO "%s: Retrieval complete\n", 
						card->devname);
			break;

		case L2_EXCEP_CONG_INBOUND:
			printk(KERN_INFO "%s: Congestion inbound\n", 
						card->devname);
			break;
#if 0
		case L2_EXCEP_CONG_INBOUND_CEASED:
			printk(KERN_INFO "%s: Congestion inbound ceased\n",
						card->devname);
			break;
#endif
		case L2_EXCEP_CONG_OUTBOUND:
			printk(KERN_INFO "%s: Congestion outbound\n",
						card->devname);
			break;
#if 0
		case L2_EXCEP_CONG_OUTBOUND_CEASED:
			printk(KERN_INFO "%s: Congestion outbound ceased\n",
						card->devname);
			break;
#endif
		case NO_L2_EXCEP_COND_TO_REPORT:
			printk(KERN_INFO "%s: No exceptions reported.\n",
						card->devname);
			break;
		default:
			printk(KERN_INFO "%s: Exception condition %x\n",
					card->devname, err);
			break;
		}
	}

	return 0;
}


/*=============================================================================
 * Process UDP management packet.
 */

static int process_udp_mgmt_pkt(sdla_t* card, netdevice_t* dev,
				ss7_private_area_t* ss7_priv_area ) 
{
	unsigned int frames;
	unsigned short buffer_length, real_len;
	unsigned long data_ptr;
	unsigned data_length;
	wan_mbox_t *mb = &card->wan_mbox;
	SHARED_MEMORY_INFO_STRUCT flags;
	struct timeval tv;
	int err;
	char ut_char;
	unsigned long trace_status_cfg_addr = 0;
	TRACE_STATUS_EL_CFG_STRUCT trace_cfg_struct;
	TRACE_STATUS_ELEMENT_STRUCT trace_element_struct;

	wan_udp_pkt_t *wan_udp_pkt = (wan_udp_pkt_t *) ss7_priv_area->udp_pkt_data;
	card->hw_iface.peek(card->hw, card->flags_off, &flags, sizeof(flags));

	switch(wan_udp_pkt->wan_udp_command) {

	case SPIPE_ENABLE_TRACING:

	     if (!ss7_priv_area->TracingEnabled) {

		/* OPERATE_DATALINE_MONITOR */

		mb->wan_data_len = sizeof(LINE_TRACE_CONFIG_STRUCT);
		mb->wan_command = SET_TRACE_CONFIGURATION;

		((LINE_TRACE_CONFIG_STRUCT *)mb->wan_data)->
			trace_config = TRACE_ACTIVE;
		/* Trace delay mode is not used because it slows
		   down transfer and results in a standoff situation
		   when there is a lot of data */
		((LINE_TRACE_CONFIG_STRUCT *)mb->wan_data)->
			trace_config |= wan_udp_pkt->wan_udp_data[0];

		DEBUG_EVENT("%s: Enableing SS7 Trace: ",card->devname);
		if (wan_udp_pkt->wan_udp_data[0] & TRACE_FISU){
			printk("FISU ");
		}
		
		if (wan_udp_pkt->wan_udp_data[0] & TRACE_LSSU){
			printk("LSSU ");
		}
		
		if (wan_udp_pkt->wan_udp_data[0] & TRACE_MSU){
			printk("MSU");
		}

		printk("\n");

		((LINE_TRACE_CONFIG_STRUCT *)mb->wan_data)->
		   trace_deactivation_timer = 3000;

		err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
		if (err != COMMAND_OK) {
			ss7_error(card,err,mb);
			card->TracingEnabled = 0;
			wan_udp_pkt->wan_udp_return_code = err;
			mb->wan_data_len = 0;
			break;
		} 

		/* Get the base address of the trace element list */
		mb->wan_data_len = 0;
		mb->wan_command = READ_TRACE_CONFIGURATION;
		err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);

		if (err != COMMAND_OK) {
			ss7_error(card,err,mb);
			ss7_priv_area->TracingEnabled = 0;
			wan_udp_pkt->wan_udp_return_code = err;
			mb->wan_data_len = 0;
			break;
		} 	

		trace_status_cfg_addr =((LINE_TRACE_CONFIG_STRUCT *)
			mb->wan_data) -> ptr_trace_stat_el_cfg_struct;

		card->hw_iface.peek(card->hw, trace_status_cfg_addr, 
					&trace_cfg_struct, sizeof(trace_cfg_struct));
	    
		ss7_priv_area->start_trace_addr = trace_cfg_struct.
			base_addr_trace_status_elements;

		ss7_priv_area->number_trace_elements = 
				trace_cfg_struct.number_trace_status_elements;

		ss7_priv_area->end_trace_addr = (unsigned long)
				((TRACE_STATUS_ELEMENT_STRUCT *)
				 ss7_priv_area->start_trace_addr + 
				 (ss7_priv_area->number_trace_elements - 1));

		ss7_priv_area->base_addr_trace_buffer = 
				trace_cfg_struct.base_addr_trace_buffer;

		ss7_priv_area->end_addr_trace_buffer = 
				trace_cfg_struct.end_addr_trace_buffer;

		ss7_priv_area->curr_trace_addr = 
				trace_cfg_struct.next_trace_element_to_use;

		ss7_priv_area->available_buffer_space = 2000 - 
							  sizeof(struct iphdr) -
							  sizeof(struct udphdr) -
						      	  sizeof(wan_mgmt_t)-
							  sizeof(wan_cmd_t)-
							  sizeof(wan_trace_info_t);
	     }
	     wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
	     mb->wan_data_len = 0;
	     ss7_priv_area->TracingEnabled = 1;
	     break;
   

	case SPIPE_DISABLE_TRACING:
	     if (ss7_priv_area->TracingEnabled) {

		/* OPERATE_DATALINE_MONITOR */
		mb->wan_data_len = sizeof(LINE_TRACE_CONFIG_STRUCT);
		mb->wan_command = SET_TRACE_CONFIGURATION;
		((LINE_TRACE_CONFIG_STRUCT *)mb->wan_data)->
			trace_config = TRACE_INACTIVE;
		err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
	     }		

	     ss7_priv_area->TracingEnabled = 0;
	     wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
	     mb->wan_data_len = 0;
	     break;
   

	case SPIPE_GET_TRACE_INFO:

	     if (!ss7_priv_area->TracingEnabled) {
		wan_udp_pkt->wan_udp_return_code = 1;
		mb->wan_data_len = 0;
		break;
	     }

	     wan_udp_pkt->wan_udp_ss7_ismoredata = 0x00;
	    
	     DEBUG_TEST("%s: Trace Elements %i Base 0x%lX  Cur 0x%lX\n",
			     card->devname,
			     ss7_priv_area->number_trace_elements,
			     ss7_priv_area->base_addr_trace_buffer,
			     ss7_priv_area->curr_trace_addr);
	   

	     buffer_length = 0;	/* offset of packet already occupied */

	     for (frames=0; frames < ss7_priv_area->number_trace_elements; frames++){

		trace_pkt_t *trace_pkt = (trace_pkt_t *)
			&wan_udp_pkt->wan_udp_data[buffer_length];

		card->hw_iface.peek(card->hw, ss7_priv_area->curr_trace_addr,
			  (unsigned char *)&trace_element_struct,
			  sizeof(TRACE_STATUS_ELEMENT_STRUCT));

		if (trace_element_struct.opp_flag == 0x00) {
			break;
		}

		/* get pointer to real data */
		data_ptr = trace_element_struct.ptr_data_bfr;

		/* See if there is actual data on the trace buffer */
		if (data_ptr){
			data_length = trace_element_struct.trace_length;
		}else{
			data_length = 0;
			wan_udp_pkt->wan_udp_ss7_ismoredata = 0x01;
		}

		if( (ss7_priv_area->available_buffer_space - buffer_length)
			< ( sizeof(trace_pkt_t) + data_length) ) {

		    	/* indicate there are more frames on board & exit */
			wan_udp_pkt->wan_udp_ss7_ismoredata = 0x01;
			break;
		}

		trace_pkt->status = trace_element_struct.trace_type;

		trace_pkt->time_stamp =
			trace_element_struct.trace_time_stamp;

		trace_pkt->real_length =
			trace_element_struct.trace_length;

		/* see if we can fit the frame into the user buffer */
		real_len = trace_pkt->real_length;

		if (data_ptr == 0) {
			trace_pkt->data_avail = 0x00;
		} else {
			unsigned tmp = 0;

			/* get the data from circular buffer
			    must check for end of buffer */
			trace_pkt->data_avail = 0x01;

			if ((data_ptr + real_len) >
				     ss7_priv_area->end_addr_trace_buffer + 1){

				tmp = ss7_priv_area->end_addr_trace_buffer - data_ptr + 1;
				card->hw_iface.peek(card->hw, data_ptr,
					  trace_pkt->data,tmp);
				data_ptr = ss7_priv_area->base_addr_trace_buffer;
			}

			card->hw_iface.peek(card->hw, data_ptr,
				  &trace_pkt->data[tmp], real_len - tmp);
		}	

		/* zero the opp flag to show we got the frame */
		ut_char = 0x00;
		card->hw_iface.poke(card->hw, ss7_priv_area->curr_trace_addr, &ut_char, 1);

		/* now move onto the next frame */
		ss7_priv_area->curr_trace_addr += sizeof(TRACE_STATUS_ELEMENT_STRUCT);

		/* check if we went over the last address */
		if ( ss7_priv_area->curr_trace_addr > ss7_priv_area->end_trace_addr ) {
			ss7_priv_area->curr_trace_addr = ss7_priv_area->start_trace_addr;
		}

		if(trace_pkt->data_avail == 0x01) {
			buffer_length += real_len - 1;
		}
 
		/* for the header */
		buffer_length += sizeof(trace_pkt_t);

	     }  /* For Loop */

	     if (frames == ss7_priv_area->number_trace_elements){
		wan_udp_pkt->wan_udp_ss7_ismoredata = 0x01;
	     }
	
	     wan_udp_pkt->wan_udp_ss7_num_frames = frames;
	     
	 
	     mb->wan_data_len = buffer_length;
	     wan_udp_pkt->wan_udp_data_len = buffer_length; 
	 
	     wan_udp_pkt->wan_udp_return_code = COMMAND_OK; 

	     break;


	case SPIPE_FT1_READ_STATUS:
		((unsigned char *)wan_udp_pkt->wan_udp_data )[0] =
			flags.FT1_info_struct.parallel_port_A_input;

		((unsigned char *)wan_udp_pkt->wan_udp_data )[1] =
			flags.FT1_info_struct.parallel_port_B_input;
		wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
		wan_udp_pkt->wan_udp_data_len = 2;
		mb->wan_data_len = 2;
		break;

	case SPIPE_ROUTER_UP_TIME:
		do_gettimeofday( &tv );
		ss7_priv_area->router_up_time = tv.tv_sec - 
				ss7_priv_area->router_start_time;
		*(unsigned long *)&wan_udp_pkt->wan_udp_data = 
				ss7_priv_area->router_up_time;	
		mb->wan_data_len = sizeof(unsigned long);
		wan_udp_pkt->wan_udp_data_len = sizeof(unsigned long);
		wan_udp_pkt->wan_udp_return_code = COMMAND_OK;
		break;

	case WAN_GET_MEDIA_TYPE:
	case WAN_FE_GET_STAT:
	case WAN_FE_SET_LB_MODE:
	case WAN_FE_FLUSH_PMON:
	case WAN_FE_GET_CFG:
		if (IS_TE1_CARD(card)){
			card->wandev.fe_iface.process_udp(
					&card->fe, 
					&wan_udp_pkt->wan_udp_cmd,
					&wan_udp_pkt->wan_udp_data[0]);
		}else if (IS_56K_CARD(card)){
			card->wandev.fe_iface.process_udp(
					&card->fe, 
					&wan_udp_pkt->wan_udp_cmd,
					&wan_udp_pkt->wan_udp_data[0]);
		}else{
			if (wan_udp_pkt->wan_udp_command == WAN_GET_MEDIA_TYPE){
				wan_udp_pkt->wan_udp_data_len = sizeof(unsigned char); 
				wan_udp_pkt->wan_udp_return_code = CMD_OK;
			}else{
				wan_udp_pkt->wan_udp_return_code = WAN_UDP_INVALID_CMD;
			}
		}
		mb->wan_data_len = wan_udp_pkt->wan_udp_data_len;
		break;
		
	case WAN_GET_PROTOCOL:
		wan_udp_pkt->wan_udp_ss7_num_frames = card->wandev.config_id;
		wan_udp_pkt->wan_udp_return_code = CMD_OK;
		mb->wan_data_len = wan_udp_pkt->wan_udp_data_len = 1;
		break;

	case WAN_GET_PLATFORM:
		wan_udp_pkt->wan_udp_data[0] = WAN_LINUX_PLATFORM;
		wan_udp_pkt->wan_udp_return_code = CMD_OK;
		mb->wan_data_len = wan_udp_pkt->wan_udp_data_len = 1;
		break;
	default:
		/* it's a board command */
		mb->wan_command = wan_udp_pkt->wan_udp_command;
		mb->wan_data_len = wan_udp_pkt->wan_udp_data_len;
		if (mb->wan_data_len){
			memcpy(&mb->wan_data, (unsigned char *) wan_udp_pkt->
						wan_udp_data, mb->wan_data_len);
      		} 
		/* run the command on the board */
		err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
		if (err != COMMAND_OK) {
			break;
		}

		/* copy the result back to our buffer */
		memcpy(&wan_udp_pkt->wan_udp_hdr.wan_cmd, mb, sizeof(wan_cmd_t)); 
		
		if (mb->wan_data_len) {
			memcpy(&wan_udp_pkt->wan_udp_data, &mb->wan_data, 
							mb->wan_data_len); 
		}

	} /* end of switch */


	wan_udp_pkt->wan_udp_request_reply = UDPMGMT_REPLY;
	return 1;
}

/*============================================================================
 * Initialize Receive and Transmit Buffers.
 */

static void init_ss7_tx_rx_buff( sdla_t* card)
{
	wan_mbox_t* mb = &card->wan_mbox;
	unsigned long tx_config_off;
	unsigned long rx_config_off;
	L2_TX_STATUS_EL_CFG_STRUCT tx_config;
	L2_RX_STATUS_EL_CFG_STRUCT rx_config;
	char err;
	
	mb->wan_data_len = 0;
	mb->wan_command = L2_READ_CONFIGURATION;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);

	if(err != COMMAND_OK) {
		ss7_error(card,err,mb);
		return;
	}

	/* ALEX Apr 8 2004 Sangoma ISA card */
	tx_config_off = ((L2_CONFIGURATION_STRUCT *)mb->wan_data)->
                       		ptr_L2_Tx_stat_el_cfg_struct;
	rx_config_off = ((L2_CONFIGURATION_STRUCT *)mb->wan_data)->
                       		ptr_L2_Rx_stat_el_cfg_struct;
	card->hw_iface.peek(card->hw, tx_config_off, &tx_config, sizeof(tx_config));
	card->hw_iface.peek(card->hw, rx_config_off, &rx_config, sizeof(rx_config));

	/* Setup Head and Tails for buffers */
       	card->u.s.txbuf_base_off = tx_config.base_addr_Tx_status_elements;
       	card->u.s.txbuf_last_off = 
       		card->u.s.txbuf_base_off +
		(tx_config.number_Tx_status_elements - 1) *
					sizeof(L2_MSU_TX_STATUS_EL_STRUCT);

       	card->u.s.rxbuf_base_off = rx_config.base_addr_Rx_status_elements;
       	card->u.s.rxbuf_last_off =
       		card->u.s.rxbuf_base_off +
		(rx_config.number_Rx_status_elements - 1) * 
					sizeof(L2_MSU_RX_STATUS_EL_STRUCT);

	/* Set up next pointer to be used */
      	card->u.s.txbuf_off = tx_config.next_Tx_status_element_to_use;
       	card->u.s.rxmb_off = rx_config.next_Rx_status_element_to_use;
	
        /* Setup Actual Buffer Start and end addresses */
        card->u.s.rx_base_off = rx_config.base_addr_Rx_status_elements;

}

/*=============================================================================
 * Perform Interrupt Test by running READ_SS7_CODE_VERSION command MAX_INTR
 * _TEST_COUNTER times.
 */
static int intr_test( sdla_t* card)
{
	wan_mbox_t* mb = &card->wan_mbox;
	int err,i;

	card->timer_int_enabled = 0;
	
	err = ss7_set_intr_mode(card, APP_INT_ON_COMMAND_COMPLETE);

	if (err == CMD_OK) { 
		for (i = 0; i < MAX_INTR_TEST_COUNTER; i ++) {	
			mb->wan_data_len  = 0;
			mb->wan_command = READ_SS7_CODE_VERSION;
			err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
			if (err != CMD_OK) 
				ss7_error(card, err, mb);
		}
	}
	else {
		return err;
	}

	err = ss7_set_intr_mode(card, 0);

	if (err != CMD_OK)
		return err;

	return 0;
}

/*============================================================================
 * Set PORT state.
 */
static void port_set_state (sdla_t *card, int state)
{
	netdevice_t		*dev;
	ss7_private_area_t	*ss7_priv_area;

	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	ss7_priv_area = dev->priv;
        if (card->wandev.state != state){

                switch (state){
			
                case WAN_CONNECTED:
                        printk (KERN_INFO "%s: SS7 L2 Link in service!\n",
                                card->devname);
                      	break;

                case WAN_CONNECTING:
                        printk (KERN_INFO "%s: SS7 L2 Link connecting...\n",
                                card->devname);
                        break;

                case WAN_DISCONNECTED:
                        printk (KERN_INFO "%s: SS7 L2 Link out of service!\n",
                                card->devname);
                        break;
                }

                card->wandev.state = state;
		ss7_priv_area->common.state = state;
		wan_update_api_state(ss7_priv_area);
	
		if (state == WAN_CONNECTED){
			start_net_queue(dev);
		}
        }
}

/*===========================================================================
 * config_ss7
 *
 *	Configure the ss7 protocol and enable communications.		
 *
 *   	The if_open() function binds this function to the poll routine.
 *      Therefore, this function will run every time the ss7 interface
 *      is brought up. We cannot run this function from the if_open 
 *      because if_open does not have access to the remote IP address.
 *      
 *	If the communications are not enabled, proceed to configure
 *      the card and enable communications.
 *
 *      If the communications are enabled, it means that the interface
 *      was shutdown by ether the user or driver. In this case, we 
 *      have to check that the IP addresses have not changed.  If
 *      the IP addresses have changed, we have to reconfigure the firmware
 *      and update the changed IP addresses.  Otherwise, just exit.
 *
 */

static int config_ss7 (sdla_t *card)
{

	if (test_bit(0,&card->comm_enabled)){
		return 0;
	}

	/* Setup the Board for CHDLC */
	if (set_ss7_config(card)) {
		printk (KERN_INFO "%s: Failed SS7 configuration!\n",
			card->devname);
		return -EINVAL;
	}

	card->hw_iface.poke_byte(card->hw, card->intr_perm_off, 0x00);
	card->hw_iface.poke_byte(card->hw, card->intr_type_off, 0x00);

	if (IS_TE1_CARD(card)) {
		int	err = -EINVAL;
		printk(KERN_INFO "%s: Configuring onboard %s CSU/DSU\n",
			card->devname, 
			(IS_T1_CARD(card))?"T1":"E1");
		if (card->wandev.fe_iface.config){
			err = card->wandev.fe_iface.config(&card->fe);
		}
		if (err){
			printk(KERN_INFO "%s: Failed %s configuratoin!\n",
					card->devname,
					(IS_T1_CARD(card))?"T1":"E1");
			return -EINVAL;
		}
		if (card->wandev.fe_iface.post_init){
			err=card->wandev.fe_iface.post_init(&card->fe);	
		}
	}

	 
	if (IS_56K_CARD(card)) {
		int	err = -EINVAL;
		printk(KERN_INFO "%s: Configuring 56K onboard CSU/DSU\n",
			card->devname);

		if (card->wandev.fe_iface.config){
			err = card->wandev.fe_iface.config(&card->fe);
		}
		if (err){
			printk (KERN_INFO "%s: Failed 56K configuration!\n",
				card->devname);
			return -EINVAL;
		}
		if (card->wandev.fe_iface.post_init){
			err=card->wandev.fe_iface.post_init(&card->fe);
		}
	}

	/* Set interrupt mode and mask */
        if (ss7_set_intr_mode(card, APP_INT_ON_RX_FRAME |
                		APP_INT_ON_GLOBAL_EXCEP_COND |
                		APP_INT_ON_TX_FRAME |
                		APP_INT_ON_L2_EXCEP_COND | APP_INT_ON_TIMER)){
		printk (KERN_INFO "%s: Failed to set interrupt triggers!\n",
				card->devname);
		return -EINVAL;	
        }


	/* Mask All interrupts  */
	card->hw_iface.clear_bit(card->hw, card->intr_perm_off, 
			(APP_INT_ON_RX_FRAME |
                	 APP_INT_ON_GLOBAL_EXCEP_COND |
                	 APP_INT_ON_TX_FRAME |
                	 APP_INT_ON_L2_EXCEP_COND | APP_INT_ON_TIMER));
	
	if (ss7_comm_enable(card) != 0) {
		printk(KERN_INFO "%s: Failed to enable SS7 communications!\n",
				card->devname);
		card->hw_iface.poke_byte(card->hw, card->intr_perm_off, 0x00);
		ss7_set_intr_mode(card,0);
		return -EINVAL;
	}

	/* Initialize Rx/Tx buffer control fields */
	init_ss7_tx_rx_buff(card);

	/* Manually poll the 56K CSU/DSU to get the status */
	if (IS_56K_CARD(card)) {
		/* 56K Update CSU/DSU alarms */
		card->wandev.fe_iface.read_alarm(&card->fe, 1); 
	}

	/* Unmask all interrupts */
	card->hw_iface.set_bit(card->hw, card->intr_perm_off, 
		       (APP_INT_ON_RX_FRAME |
               		APP_INT_ON_GLOBAL_EXCEP_COND |
               		APP_INT_ON_TX_FRAME |
               		APP_INT_ON_L2_EXCEP_COND | APP_INT_ON_TIMER));
#ifdef SHOW_L2_HIST
	card->hw_iface.clear_bit(card->hw, card->intr_perm_off, APP_INT_ON_TX_FRAME);
#else
	card->hw_iface.clear_bit(card->hw, card->intr_perm_off, (APP_INT_ON_TX_FRAME | APP_INT_ON_TIMER));
#endif
	card->hw_iface.poke_byte(card->hw, card->intr_type_off, 0x00);
	
	port_set_state(card,WAN_CONNECTING);
	return 0; 
}


void s508_lock (sdla_t *card, unsigned long *smp_flags)
{
	spin_lock_irqsave(&card->wandev.lock, *smp_flags);
        if (card->next){
        	spin_lock(&card->next->wandev.lock);
	}
}

void s508_unlock (sdla_t *card, unsigned long *smp_flags)
{
        if (card->next){
        	spin_unlock(&card->next->wandev.lock);
        }
        spin_unlock_irqrestore(&card->wandev.lock, *smp_flags);
}

static void tx_intr (unsigned long data)
{
	sdla_t			*card = (sdla_t*)data;
	netdevice_t		*dev;
	ss7_private_area_t	*ss7_priv_area;
	
	dev = WAN_DEVLE2DEV(WAN_LIST_FIRST(&card->wandev.dev_head));
	if (!dev){
		return;
	}
	
	if ((ss7_priv_area=dev->priv) == NULL){
		return;
	}

	// printk(KERN_INFO "WAKING UP IN TX INTR\n");
	
	start_net_queue(dev);
	wan_wakeup_api(ss7_priv_area);
}




static void L2_read_hist_table(sdla_t *card)
{
	wan_mbox_t* mb = &card->wan_mbox;
        L2_HISTORY_STRUCT* phist;
	int err;
        int i, j;
        //SHARED_MEMORY_INFO_STRUCT* flags = NULL;
	char outs[400];
        
	mb->wan_data_len = 0x00;
        mb->wan_command = L2_READ_HISTORY_TABLE;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);
        if ((err != COMMAND_OK) || !mb->wan_data_len) {
                return;
        }                        

        i = 0;
        j = 1;


//????????????????????aaaaaaaaaaaaaaa
        for(;;) {
		memset(outs, 0, 400);
		sprintf(outs, "%s: ", card->devname);
                phist = (L2_HISTORY_STRUCT *)&mb->wan_data[i];
		switch(phist->function) {
                	case L2_HISTORY_LSC:
				sprintf(outs+strlen(outs),"Link State Control");
				break;
			case L2_HISTORY_IAC:
                        	sprintf(outs+strlen(outs),"Initial Alignment Control");
				break;
			case L2_HISTORY_DAEDR:
                                sprintf(outs+strlen(outs),"DAEDR");
				break;
			case L2_HISTORY_DAEDT:
                                sprintf(outs+strlen(outs),"DAEDT");
				break;
			case L2_HISTORY_TXC:
                                sprintf(outs+strlen(outs),"Transmission Control");
				break;
			case L2_HISTORY_RXC:
                                sprintf(outs+strlen(outs),"Reception Control");
				break;
			case L2_HISTORY_AERM:
                                sprintf(outs+strlen(outs),"AERM");
				break;
			case L2_HISTORY_SUERM:
                                sprintf(outs+strlen(outs),"SUERM");
				break;
			case L2_HISTORY_CC:
                                sprintf(outs+strlen(outs),"Congestion Control");
				break;
			case L2_HISTORY_EIM:
                                sprintf(outs+strlen(outs),"EIM");
				break;
		}
		sprintf(outs+strlen(outs), " (Time: %u)", phist->time);
		printk(KERN_INFO "%s\n", outs);

		memset(outs, 0, 400);
                sprintf(outs, "  Action: 0x%04X - ", phist->action);

		switch(phist->function) {
			case L2_HISTORY_LSC:
				if(phist->status_before_action & 
				(LSC_STAT_POWER_OFF | LSC_STAT_INIT_ALIGN | LSC_STAT_ALIGNED_READY | LSC_STAT_ALIGNED_NOT_READY | LSC_STAT_IN_SERVICE)) {
                                	prt_hist(phist->action, act_LSC_0, outs);
				}
				else if(phist->status_before_action & LSC_STAT_PROCESSOR_OUTAGE) {
                                	prt_hist(phist->action, act_LSC_1, outs);
				}
				else {
                               		prt_hist(phist->action, act_LSC_2, outs);
				}
				break;
			case L2_HISTORY_IAC:
                       		prt_hist(phist->action, act_IAC, outs);
				break;
			case L2_HISTORY_DAEDR:
                                prt_hist(phist->action, act_DAEDR, outs);
				break;
			case L2_HISTORY_DAEDT:
                                prt_hist(phist->action, act_DAEDT, outs);
				break;
			case L2_HISTORY_TXC:
                              	prt_hist(phist->action, act_TXC, outs);
				break;
			case L2_HISTORY_RXC:
                                prt_hist(phist->action, act_RXC, outs);
				break;
			case L2_HISTORY_AERM:
                                prt_hist(phist->action, act_AERM, outs);
				break;
			case L2_HISTORY_SUERM:
                                prt_hist(phist->action, act_SUERM, outs);
				break;
			case L2_HISTORY_CC:
                                prt_hist(phist->action, act_CC, outs);
				break;
			case L2_HISTORY_EIM:
                                prt_hist(phist->action, act_EIM, outs);
				break;
		}

		printk(KERN_INFO "%s\n", outs);

		memset(outs, 0, 400);
		sprintf(outs, "  State : 0x%04X - ", phist->status_before_action);

		switch(phist->function) {
			case L2_HISTORY_LSC:
                       		prt_hist(phist->status_before_action, state_LSC, outs);
				break;
			case L2_HISTORY_IAC:
                       		prt_hist(phist->status_before_action, state_IAC, outs);
				break;
			case L2_HISTORY_DAEDR:
                       		prt_hist(phist->status_before_action, state_DAEDR, outs);
				break;
			case L2_HISTORY_DAEDT:
                                prt_hist(phist->status_before_action, state_DAEDT, outs);
				break;
			case L2_HISTORY_TXC:
                                prt_hist(phist->status_before_action, state_TXC, outs);
				break;
			case L2_HISTORY_RXC:
                                prt_hist(phist->status_before_action, state_RXC, outs);
				break;
			case L2_HISTORY_AERM:
                                prt_hist(phist->status_before_action, state_AERM, outs);
				break;
			case L2_HISTORY_SUERM:
                                prt_hist(phist->status_before_action, state_SUERM, outs);
				break;
			case L2_HISTORY_CC:
                                prt_hist(phist->status_before_action, state_CC, outs);
				break;
			case L2_HISTORY_EIM:
                              	prt_hist(phist->status_before_action, state_EIM, outs);
				break;
		}

		printk(KERN_INFO "%s\n", outs);

		if((phist->function == L2_HISTORY_TXC) || (phist->function == L2_HISTORY_RXC)) {
			
			memset(outs, 0, 400);
			if(phist->function == L2_HISTORY_TXC) {
                        	sprintf(outs, "Tx LSSU SF: 0x%02X ", phist->LSSU_SF);
			}

			if(phist->function == L2_HISTORY_RXC) {
                       		sprintf(outs, "Rx LSSU SF: 0x%02X ", phist->LSSU_SF);
			}

			switch(phist->LSSU_SF) {
				case LSSU_SIO:
                                	sprintf(outs+strlen(outs), "(out of alignment)");
					break;
				case LSSU_SIN:
                                        sprintf(outs+strlen(outs), "(normal alignment)");
					break;
				case LSSU_SIE:
                                        sprintf(outs+strlen(outs), "(emergency alignment)");
					break;
				case LSSU_SIOS:
                                        sprintf(outs+strlen(outs), "(out of service)");
					break;
				case LSSU_SIPO:
                                        sprintf(outs+strlen(outs), "(processor outage)");
					break;
				case LSSU_SIB:
                                        sprintf(outs+strlen(outs), "(busy)");
					break;
				case NO_LSSU:
                                        sprintf(outs+strlen(outs), "(none)");
					break;
			}
			printk(KERN_INFO "%s\n", outs);
		}
		i += sizeof(L2_HISTORY_STRUCT);
		mb->wan_data_len -= sizeof(L2_HISTORY_STRUCT);
		if(!mb->wan_data_len) {
			break;
		}
	}

 //       flags = card->u.s.flags;
   //     flags->interrupt_info_struct.interrupt_permission &=
     //                   ~APP_INT_ON_TIMER;
}



static void prt_hist(unsigned short bit_map, char *string_bfr[], char* outs)
{
int i, j;

	j = 0;
	for(i = 0; i < 16; i ++) {
		if(bit_map & (0x01 << i)) {
			if(j) {
                                sprintf(outs+strlen(outs),  "/");
			}
                        sprintf(outs+strlen(outs), "%s", string_bfr[i]);
			j ++;
		}
	}
}

static void ss7_handle_front_end_state(void* card_id)
{
	sdla_t*	card = (sdla_t*)card_id;
	
	if (card->wandev.ignore_front_end_status == WANOPT_YES){
		return;
	}

	if (card->tty_opt){
		return;
	}

	if (card->fe.fe_status == FE_CONNECTED){
		if (card->u.s.state == WAN_CONNECTED){
			port_set_state(card,WAN_CONNECTED);
		}
	}else{
		ss7_stop(card);
		port_set_state(card,WAN_DISCONNECTED);
	}
}

static int set_adapter_config (sdla_t* card)
{
	wan_mbox_t*			mb = &card->wan_mbox;
	ADAPTER_CONFIGURATION_STRUCT*	cfg = (ADAPTER_CONFIGURATION_STRUCT*)mb->wan_data;
	int				err;

	card->hw_iface.getcfg(card->hw, SDLA_ADAPTERTYPE, &cfg->adapter_type); 
	cfg->adapter_config = 0x00; 
	cfg->operating_frequency = 00; 
	mb->wan_data_len = sizeof(ADAPTER_CONFIGURATION_STRUCT);
	mb->wan_command = SET_ADAPTER_CONFIGURATION;
	err = card->hw_iface.cmd(card->hw, card->mbox_off, mb);

	if(err != COMMAND_OK) {
		ss7_error(card,err,mb);
	}
	return (err);
}


/****** End ****************************************************************/
