/*****************************************************************************
* wanconfig.c	WAN Multiprotocol Router Configuration Utility.
*
* Author:	Nenad Corbic	<ncorbic@sangoma.com>
*               Gideon Hack
*
* Copyright:	(c) 1995-2003 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ----------------------------------------------------------------------------
* Mar 29, 2007  David Rokhvarg  Added support for AFT-56K card.
* May 11, 2001  Alex Feldman    Added T1/E1 support (TE1).
* Apr 16, 2001  David Rokhvarg  Added X25_SRC_ADDR and ACCEPT_CALLS_FROM to 'chan_conftab'
* Nov 15, 2000  Nenad Corbic    Added true interface encoding type option
* Arp 13, 2000  Nenad Corbic	Added the command line argument, startup support.
*                               Thus, one can configure the router using
*                               the command line arguments.
* Jan 28, 2000  Nenad Corbic    Added support for ASYCN protocol.
* Sep 23, 1999  Nenad Corbic    Added support for HDLC STREAMING, Primary
*                               and secondary ports. 
* Jun 02, 1999 	Gideon Hack	Added support for the S514 PCI adapter.
* Jan 07, 1998	Jaspreet Singh	Made changes for 2.1.X.
*				Added support for WANPIPE and API integration.
* Jul 20, 1998	David Fong	Added Inverse ARP option to channel config.
* Jun 26, 1998	David Fong	Added IP_MODE to PPP Configuration.
*				Used for Dynamic IP Assignment.
* Jun 18, 1998	David Fong	Added Cisco HDLC definitions and structures
* Dec 16, 1997	Jaspreet Singh 	Moved IPX and NETWORK to 'chan_conftab'
* Dec 08, 1997	Jaspreet Singh	Added USERID, PASSWD and SYSNAME in 
*				'chan_conftab' 
* Dec 05, 1997	Jaspreet Singh  Added PAP and CHAP in 'chan_conftab'
*				Added AUTHENTICATOR in 'ppp_conftab' 
* Oct 12, 1997	Jaspreet Singh	Added IPX and NETWORK to 'common_conftab'
*				Added MULTICAST to 'chan_conftab'
* Oct 02, 1997  Jaspreet Singh	Took out DLCI from 'fr_conftab'. Made changes 
*				so that a list of DLCI is prepared to 
*				configuring them when emulating a NODE 
* Jul 07, 1997	Jaspreet Singh	Added 'ttl' to 'common_conftab'
* Apr 25, 1997  Farhan Thawar   Added 'udp_port' to 'common_conftab'
* Jan 06, 1997	Gene Kozin	Initial version based on WANPIPE configurator.
*****************************************************************************/

/*****************************************************************************
* Usage:
*   wanconfig [-f {conf_file}]	Configure WAN links and interfaces
*   wanconfig -d {device}	Shut down WAN link
*   wanconfig -h|?		Display help screen
*
* Where:
*   {conf_file}		configuration file name. Default is /etc/wanpipe1.conf
*   {device}		name of the WAN device in /proc/net/wanrouter directory
*
* Optional switches:
*   -v			verbose mode
*   -h or -?		display help screen
*****************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "lib/safe-read.h"
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_version.h>
# include <wanpipe_defines.h>
# include <wanpipe_cfg.h>
# include <wanproc.h>
# include <wanpipe.h>
#else
# include <linux/wanpipe_version.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_cfg.h>
# include <linux/wanpipe.h>
#endif

#define smemof(TYPE, MEMBER) offsetof(TYPE,MEMBER),(sizeof(((TYPE *)0)->MEMBER))



/****** Defines *************************************************************/

#ifndef	min
#define	min(a,b)	(((a)<(b))?(a):(b))
#endif

#define	is_digit(ch) (((ch)>=(unsigned)'0'&&(ch)<=(unsigned)'9')?1:0)

enum ErrCodes			/* Error codes */
{
	ERR_SYSTEM = 1,		/* system error */
	ERR_SYNTAX,		/* command line syntax error */
	ERR_CONFIG,		/* configuration file syntax error */
	ERR_MODULE,		/* module not loaded */
	ERR_LIMIT
};

/* Configuration stuff */
#define	MAX_CFGLINE	256
#define	MAX_CMDLINE	256
#define	MAX_TOKENS	32

/* Device directory and WAN device name */
#define WANDEV_NAME "/dev/wanrouter"

/****** Data Types **********************************************************/

typedef struct look_up		/* Look-up table entry */
{
	uint	val;		/* look-up value */
	void*	ptr;		/* pointer */
} look_up_t;

typedef struct key_word		/* Keyword table entry */
{
	char*	keyword;	/* -> keyword */
	uint	offset;		/* offset of the related parameter */
	int	size;		/* size of type */
	int	dtype;		/* data type */
} key_word_t;

typedef struct data_buf		/* General data buffer */
{
	unsigned size;
	void* data;
} data_buf_t;

/*
 * Data types for configuration structure description tables.
 */
#define	DTYPE_INT	1
#define	DTYPE_UINT	2
#define	DTYPE_LONG	3
#define	DTYPE_ULONG	4
#define	DTYPE_SHORT	5
#define	DTYPE_USHORT	6
#define	DTYPE_CHAR	7
#define	DTYPE_UCHAR	8
#define	DTYPE_PTR	9
#define	DTYPE_STR	10
#define	DTYPE_FILENAME	11

#define NO_ANNEXG	 0
#define ANNEXG_LAPB	 1
#define ANNEXG_X25	 2
#define ANNEXG_DSP	 3

#define ANNEXG_FR	 4
#define ANNEXG_PPP	 5
#define ANNEXG_CHDLC	 6
#define ANNEXG_LIP_LAPB  7
#define ANNEXG_LIP_XDLC  8
#define ANNEXG_LIP_TTY   9
#define ANNEXG_LIP_XMTP2 10
#define ANNEXG_LIP_X25   11
#define ANNEXG_LIP_ATM   12
#define ANNEXG_LIP_LAPD  13
#define ANNEXG_LIP_KATM  14

#define WANCONFIG_SOCKET "/etc/wanpipe/wanconfig_socket"
#define WANCONFIG_PID    "/var/run/wanconfig.pid"
#define WANCONFIG_PID_FILE "/proc/net/wanrouter/pid"

struct link_def;
typedef struct chan_def			/* Logical Channel definition */
{
	char name[WAN_IFNAME_SZ+1];	/* interface name for this channel */
	char* addr;			/* -> media address */
	char* conf;			/* -> configuration data */
	char* conf_profile;		/* -> protocol provile data */
	char* usedby;			/* used by WANPIPE or API */
	char  annexg;			/* -> anexg interface */
	char* label;			/* -> user defined label/comment */
	char* virtual_addr;		/* -> X.25 virtual addr */
	char* real_addr;		/* -> X.25 real addr */
	char* protocol;			/* -> protocol */
	char active_ch[50];		/* -> active channels */
	wanif_conf_t  *chanconf;	/* channel configuration structure */
	struct link_def	*link;		/* Point to WAN link definition */
	struct chan_def* next;		/* -> next channel definition */
} chan_def_t;

typedef struct link_def			/* WAN Link definition */
{
	char name[WAN_DRVNAME_SZ+1];	/* link device name */
	int config_id;			/* configuration ID */
	char* conf;			/* -> configuration data */
	char* descr;			/* -> description */
	unsigned long checksum;
	time_t modified;
	chan_def_t* chan;		/* list of channel definitions */
	wandev_conf_t *linkconf;	/* link configuration structure */
	struct link_def* next;		/* -> next link definition */
} link_def_t;


char master_lapb_dev[WAN_DRVNAME_SZ+1];
char master_x25_dev[WAN_DRVNAME_SZ+1];
char master_dsp_dev[WAN_DRVNAME_SZ+1];
char master_lip_dev[WAN_DRVNAME_SZ+1];

#define MAX_WAKEUI_WAIT 2
static char wakeup_ui=0;
static time_t time_ui=0;

#define MAX_FIRMW_SIZE	40000
static unsigned char firmware_file_buffer[MAX_FIRMW_SIZE];

#define show_error(x) { show_error_dbg(x, __LINE__);}

/****** Function Prototypes *************************************************/

int arg_proc (int argc, char* argv[]);
void show_error_dbg	(int err, int line);
int startup(void);
int wp_shutdown (void);
int configure (void);
int parse_conf_file (char* fname);
int build_linkdef_list (FILE* file);
int build_chandef_list (FILE* file);
char* read_conf_section (FILE* file, char* section);
int read_conf_record (FILE* file, char* key);
int configure_link (link_def_t* def, char init);
int configure_chan (int dev, chan_def_t* def, char init, int id);
int set_conf_param (char* key, char* val, key_word_t* dtab, void* conf);
void init_first_time_tokens(char **token);
void init_tokens(char **token);
int tokenize (char* str, char **tokens);
char* str_strip (char* str, char* s);
char* strupcase	(char* str);
void* lookup (int val, look_up_t* table);
int name2val (char* name, look_up_t* table);
int read_data_file (char* name, data_buf_t* databuf);
unsigned long filesize (FILE* file);
unsigned int dec_to_uint (unsigned char* str, int len);
unsigned int get_config_data (int, char**);
void show_help(void);
void show_usage(void);
void set_action(int new_action);
int gencat (char *filename); 

int show_status(void);
int show_config(void);
int show_hwprobe(void);
int debugging(void);
int debug_read(void);


int start_daemon(void);
int exec_link_cmd(int dev, link_def_t *def);
int exec_chan_cmd(int dev, chan_def_t *def);
void free_linkdefs(void);


/* Parsing the command line arguments, in order
 * to starting WANPIPE from the command line. 
 */ 
int get_devices (FILE *fp, int *argc, char ***argv, char**);
int get_interfaces (FILE *fp, int *argnum, char ***argv_ptr, char *device_name);
int get_hardware (FILE *fp, int *argc_ptr, char ***argv_ptr, char *device_name);
int get_intr_setup (FILE *fp, int *argc_ptr, char ***argv_ptr);
int router_down (char *devname, int ignore_error);
int router_ifdel (char *card_name, char *dev_name);
int conf_file_down (void);

unsigned int parse_active_channel(char* val);
unsigned int get_active_channels(int channel_flag, int start_channel, int stop_channel);
void read_adsl_vci_vpi_list(wan_adsl_vcivpi_t* vcivpi_list, unsigned short* vcivpi_num);
void update_adsl_vci_vpi_list(wan_adsl_vcivpi_t* vcivpi_list, unsigned short vcivpi_num);

void wakeup_java_ui(void);

#if defined(WAN_HWEC)
static int config_hwec(char *devname);
static int enable_hwec(char *devname, char *ifname, char *);
static int release_hwec(char *devname);
#endif
extern	int close (int);

char *time_string(time_t t,char *buf);
int has_config_changed(link_def_t *linkdef, char *name);
void free_device_link(char *devname);
int device_syncup(char *devname);
void sig_func (int sigio);
int start_link (void);
int stop_link(void);
int exec_command(unsigned char *rx_data);
void catch_signal(int signum,int nomask);

/****** Global Data *********************************************************/

char prognamed[20] =	"wanconfig";
char progname_sp[] = 	"         ";
char def_conf_file[] =	"/etc/wanpipe/wanpipe1.conf";	/* default name */
char def_adsl_file[] =	"/etc/wanpipe/wan_adsl.list";	/* default name */
char tmp_adsl_file[] =	"/etc/wanpipe/wan_adsl.tmp";	/* default name */
#if defined(__LINUX__)
char router_dir[] =	"/proc/net/wanrouter";	/* location of WAN devices */
#else
char router_dir[] =	"/var/lock/wanrouter";	/* location of WAN devices */
#endif
char conf_dir[] =	"/etc/wanpipe";
char banner[] =		"WAN Router Configurator"
			"(c) 1995-2003 Sangoma Technologies Inc.";

static unsigned char wan_version[100];


char usagetext[] = 
	"\n"
	"  wanconfig: Wanpipe device driver configuration tool\n\n"
	"  Usage: wanconfig [ -hvw ] [ -f <config-file> ] [ -U {arg options} ]\n"
       	"                   [ -y <verbose-log> ] [ -z <kernel-log> ]\n"
	"                   [ card <wan-device-name> [ dev <dev-name> | nodev ] ]\n"
	"                   [ help | start | stop | up | down | add | del | reload\n"
	"                     | restart | status | show | config ]\n"
       	"\n";	

char helptext[] =
	"Usage:\n"
	"------\n"
	"\twanconfig [ -hvw ] [ -f <config-file> ] [ -U {arg-options} ]\n"
       	"\t                   [ -y <verbose-log> ] [ -z <kernel-log> ]\n"
	"\t                   [ card <wan-dev-name> [ dev <dev-name> | nodev ] ]\n"
	"\t                   [ help | start | stop | up | down | add | del\n"
       	"\t                     | reload | restart | status | show | config ]\n"
	"\n"
	"\tWhere:\n"
	"\n"
	"\t          [-f {config-file}] Specify an overriding configuration file.\n"
	"\t          [-U {arg-options}] Configure WAN link using command line\n"
	"\t                             arguments.\n"
	"\t          [-v]               Verbose output to stdout.\n"
	"\t          [-h] | help        Show this help.\n"
	"\t          [-w]               Enable extra error messages about debug\n"
	"\t                             log locations.\n"
	"\t          [-y]               Give location of wanconfig verbose log\n"
       	"\t                             for -w switch.\n"
	"\t          [-z]               Give location of syslogd kernel logging\n"
       	"\t                             for -w switch.\n"
	"\t          card <wan-dev-name>\n"
	"\t                             Specify which WAN interface/card to\n"
	"\t                             operate on. Name given in\n"
       	"\t                             /proc/net/wanrouter directory.\n"
	"\t          dev <dev-name>     Specify which linux network interface\n"
	"\t                             to operate on.\n"
	"\t          nodev              Turns off creation of interface(s)\n"
	"\t                             when (re)starting card(s).\n"
	"\t          start | up | add   Configure WAN interface(s)/card(s)\n"
	"\t                             and/or create linux network interface(s).\n"
	"\t          stop | down | del  Shut down WAN interface(s)/card(s) and/or\n"
	"\t                             destroy linux network interface(s).\n"
	"\t          reload | restart   Restart WAN interface(s)/card(s) and/or\n"
       	"\t                             recreate linux network interface(s).\n"
	"\t          status | stat | show\n"
	"\t                             Display status information on all\n"
	"\t                             interfaces/cards or for a specific\n"
       	"\t                             WAN interface/card.\n"
	"\t          config             Display configuration information for all\n"
	"\t                             WAN interfaces/cards.\n"
	"\n"
	"\t{config-file}\tconfiguration file (default is\n"
       	"\t\t\t/etc/wanpipe/wanpipe#.conf or\n"
	"\t\t\t/etc/wanpipe/wanpipe.conf in that order)\n"
	"\t{arg-options}\tare as given below\n"
	"\nArg Options:\n"
	"	[devices] \\ \n"
	" 	 <devname> <protocol> \\ \n"
	"	[interfaces] \\ \n"
	"	 <if_name> <devname> <{addr/-}> <operation_mode> \\ \n"
	"	 <if_name> <devname> <{addr/-}> <operation_mode> \\ \n"
        "	 ...   \\ \n"
	"	[devname] \\ \n"
	"	 <hw_option> <hw_option_value> \\ \n"
	"	 <hw_option> <hw_option_value> \\ \n"
	"	 ...  \\ \n"
	"	[if_name] \\ \n"
	"	 <protocol_opton> <protocol_option_value> \\ \n"
	"	 <protocol_opton> <protocol_option_value> \\ \n"
	"	 ... \\ \n\n"
	"	devname		device name. ex:wanpipe# (#=1..16) \n"
	"	protocol	wan protocol. ex: WAN_FR,WAN_CHDLC,WAN_PPP,WAN_X25 \n"
	"	if_name		interface name. ex: wp1_fr16, wp1_ppp \n"
	"	addr/-		For Frame Relay: DLCI number ex: 16, 25\n"
	"			    X25 PVC :    LCN number ex: 1, 2, 4\n"
	"			    X25 SVC :    X25 address ex: @123, @4343 \n"
	"			For other protocol set to: '-'\n"
	"	operation_mode  Mode of operation: WANPIPE - for routing \n"
	"					   API     - raw api interface\n"
	"			The API only applies to WAN_CHDLC, WAN_FR and \n" 
	"			WAN_X25 protocols.\n"
	"	hw_option\n"
	"	protocol_option \n"
	"			Please refer to sample wanpipe#.conf files in \n"
	"			/etc/wanpipe/samples directory for the\n"
	"			appropriate HW/Protocol optoins. \n" 
	"			ex: ioprot, irq, s514_cpu, pci_slot ... \n"
	"	hw_option_value \n"
	"	protocol_option_value \n"
	"			Value of the above options. Refer to the above \n"
	"			sample files. ex: ioport = '0x360' multicast=YES  \n\n"
	"	Example 1: Bring up wanpipe1 device from the command line\n"
	"	wanconfig -U 	[devices] \\ \n"
        "			wanpipe1 WAN_CHDLC \\ \n"
        "			[interfaces] \\ \n"
        "			wp1_chdlc wanpipe1 - WANPIPE \\ \n" 
        "			[wanpipe1] \\ \n"
        "			IOPORT 0x360 \\ \n"
        "			IRQ 7 \\ \n"
        "			Firmware  /etc/wanpipe/firmware/cdual514.sfm \\ \n"
        "			CommPort PRI \\ \n"
        "			Receive_Only NO \\ \n"
        "			Interface V35 \\ \n"
        "			Clocking External \\ \n"
        "			BaudRate 1600000 \\ \n"
        "			MTU 1500 \\ \n"
        "			UDPPORT 9000 \\ \n" 
        "			TTL 255 \\ \n"
        "			[wp1_chdlc] \\ \n"
        "			MULTICAST  NO \\ \n"
        "			IGNORE_DCD YES \\ \n"
        "			IGNORE_CTS YES \\ \n"
        "			IGNORE_KEEPALIVE YES \\ \n" 
        "			HDLC_STREAMING YES \\ \n"
        "			KEEPALIVE_TX_TIMER 10000 \n\n"   
	"	Example 2: Shutdown wanpipe1 device from the command line \n"
	"	\n\t\t#> wanconfig card wanpipe1 stop\n\n"
	"	Example 3: Create fr17 linux network interface and \n"
	"                  start wanpipe1 device (if not already started)\n"
	"                  from the command line \n"
	"	\n\t\t#> wanconfig card wanpipe1 dev fr17 start\n\n"
	"	wanconfig card wanpipe1 dev fr17 start\n\n"
	"	Example 4: Shutdown all WAN devices from the command line \n"
	"	\n\t\t#> wanconfig stop\n"
	

;
char* err_messages[] =				/* Error messages */
{
	"Invalid command line syntax",		/* ERR_SYNTAX */
	"Invalid configuration file syntax",	/* ERR_CONFIG */
	"Wanpipe module not loaded",		/* ERR_MODULE */
	"Unknown error code",			/* ERR_LIMIT */
};
enum	/* modes */
{
	DO_UNDEF,
	DO_START,
	DO_STOP,
	DO_RESTART,
	DO_HELP,
	DO_ARG_CONFIG,
	DO_ARG_DOWN,
	DO_SHOW_STATUS,
	DO_SHOW_CONFIG,
	DO_SHOW_HWPROBE,
	DO_DEBUGGING,
	DO_DEBUG_READ,
} action;				/* what to do */

#define	SLEEP_TIME 1			/* Sleep time after executing ioctl */

int verbose = 0;			/* verbosity level */
char* conf_file = NULL;			/* configuration file */
char* adsl_file = NULL;			/* ADSL VCI/VPI list */
char *dev_name = NULL;
char *card_name = NULL;
static char *command=NULL;
char *krnl_log_file = "/var/log/messages";
char *verbose_log = "/var/log/wanrouter";
int  weanie_flag = 1;
int  nodev_flag = 0;
link_def_t* link_defs;			/* list of WAN link definitions */
union
{
	wandev_conf_t linkconf;		/* link configuration structure */
	wanif_conf_t chanconf;		/* channel configuration structure */
} u;
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
wan_conf_t	config;
#endif


static void SIZEOFASSERT (key_word_t* dtab, int type_size)
{
 	if (dtab->size != type_size) { 
		printf("\n==========CRITICAL ERROR============\n\n");  
		printf("Size Mismatch: Type Size %i != %i\n",dtab->size, type_size); 
		printf("======================================\n\n");  
		fprintf(stderr,"\n==========CRITICAL ERROR============\n\n"); 
	        fprintf(stderr,"Size Mismatch: Type Size %i != %i\n",dtab->size, type_size);
		fprintf(stderr,"======================================\n\n");	
		fprintf(stderr,"Plese email /var/log/wanrouter file to Sangoma Support\n");	
		fprintf(stderr,"Email to techdesk@sangoma.com\n");	
		fprintf(stderr,"======================================\n\n");	
       	}         
}   

/*
 * Configuration structure description tables.
 * WARNING:	These tables MUST be kept in sync with corresponding data
 *		structures defined in linux/wanrouter.h
 */
key_word_t common_conftab[] =	/* Common configuration parameters */
{
  { "IOPORT",     smemof(wandev_conf_t, ioport),     DTYPE_UINT },
  { "MEMADDR",    smemof(wandev_conf_t, maddr),       DTYPE_UINT  },
  { "MEMSIZE",    smemof(wandev_conf_t, msize),       DTYPE_UINT },
  { "IRQ",        smemof(wandev_conf_t, irq),         DTYPE_UINT },
  { "DMA",        smemof(wandev_conf_t, dma),         DTYPE_UINT },
  { "CARD_TYPE",  smemof(wandev_conf_t, card_type),	DTYPE_UCHAR },
  { "S514CPU",    smemof(wandev_conf_t, S514_CPU_no), DTYPE_STR },
  { "PCISLOT",    smemof(wandev_conf_t, PCI_slot_no), DTYPE_UINT },
  { "PCIBUS", 	  smemof(wandev_conf_t, pci_bus_no),	DTYPE_UINT },
  { "AUTO_PCISLOT",smemof(wandev_conf_t, auto_pci_cfg), DTYPE_UCHAR },
  { "COMMPORT",   smemof(wandev_conf_t, comm_port),   DTYPE_UINT },

  /* TE1 New hardware parameters for T1/E1 board */
//  { "MEDIA",    offsetof(wandev_conf_t, te_cfg)+smemof(sdla_te_cfg_t, media), DTYPE_UCHAR },
//  { "LCODE",    offsetof(wandev_conf_t, te_cfg)+smemof(sdla_te_cfg_t, lcode), DTYPE_UCHAR },
//  { "FRAME",    offsetof(wandev_conf_t, te_cfg)+smemof(sdla_te_cfg_t, frame), DTYPE_UCHAR },

  /* Front-End parameters */
  { "FE_MEDIA",    offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, media), DTYPE_UCHAR },
  { "FE_SUBMEDIA",    offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, sub_media), DTYPE_UCHAR },
  { "FE_LCODE",    offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, lcode), DTYPE_UCHAR },
  { "FE_FRAME",    offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, frame), DTYPE_UCHAR },
  { "FE_LINE",    offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, line_no),  DTYPE_UINT },
  { "FE_TXTRISTATE",    offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, tx_tristate_mode),  DTYPE_UCHAR },
  /* Front-End parameters (old style) */
  /* Front-End parameters (old style) */
  { "MEDIA",    offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, media), DTYPE_UCHAR },
  { "LCODE",    offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, lcode), DTYPE_UCHAR },
  { "FRAME",    offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, frame), DTYPE_UCHAR },
  /* T1/E1 Front-End parameters */
  { "TE_LBO",           offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, lbo), DTYPE_UCHAR },
  { "TE_CLOCK",         offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, te_clock), DTYPE_UCHAR },
  { "TE_ACTIVE_CH",	offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, active_ch), DTYPE_UINT },
  { "TE_RBS_CH",	offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, te_rbs_ch), DTYPE_UINT },
  { "TE_HIGHIMPEDANCE", offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, high_impedance_mode), DTYPE_UCHAR },
  { "TE_REF_CLOCK",     offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, te_ref_clock), DTYPE_UCHAR },
  { "TE_SIG_MODE",     offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, sig_mode), DTYPE_UCHAR },
  /* T1/E1 Front-End parameters (old style) */
  { "LBO",           offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, lbo), DTYPE_UCHAR },
  { "ACTIVE_CH",	offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, active_ch), DTYPE_UINT },
  { "HIGHIMPEDANCE", offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te_cfg_t, high_impedance_mode), DTYPE_UCHAR },
  /* T3/E3 Front-End parameters */
  { "TE3_FRACTIONAL",offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te3_cfg_t, fractional), DTYPE_UINT },
  { "TE3_RDEVICE",	offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te3_cfg_t, rdevice_type), DTYPE_UINT },
  { "TE3_FCS",	offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te3_cfg_t, fcs), DTYPE_UINT },
  { "TE3_RXEQ",	offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + offsetof(sdla_te3_cfg_t, liu_cfg) + smemof(sdla_te3_liu_cfg_t, rx_equal), DTYPE_UINT },
  { "TE3_TAOS",	offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + offsetof(sdla_te3_cfg_t, liu_cfg) + smemof(sdla_te3_liu_cfg_t, taos), DTYPE_UINT },
  { "TE3_LBMODE",	offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + offsetof(sdla_te3_cfg_t, liu_cfg) + smemof(sdla_te3_liu_cfg_t, lb_mode), DTYPE_UINT },
  { "TE3_TXLBO",	offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + offsetof(sdla_te3_cfg_t, liu_cfg) + smemof(sdla_te3_liu_cfg_t, tx_lbo), DTYPE_UINT },
  { "TE3_CLOCK",         offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_te3_cfg_t, clock), DTYPE_UINT },
  { "TDMV_LAW",         offsetof(wandev_conf_t, fe_cfg)+smemof(sdla_fe_cfg_t, tdmv_law), DTYPE_UINT },
  { "TDMV_OPERMODE",    offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_remora_cfg_t, opermode_name), DTYPE_STR },
  { "RM_BATTTHRESH",    offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_remora_cfg_t, battthresh), DTYPE_UINT },
  { "RM_BATTDEBOUNCE",  offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_remora_cfg_t, battdebounce), DTYPE_UINT },
  { "RM_NETWORK_SYNC",  offsetof(wandev_conf_t, fe_cfg)+offsetof(sdla_fe_cfg_t, cfg) + smemof(sdla_remora_cfg_t, network_sync), DTYPE_UINT },
  
  /* TDMV parameters */
  { "TDMV_SPAN",     offsetof(wandev_conf_t, tdmv_conf)+smemof(wan_tdmv_conf_t, span_no), DTYPE_UINT},
  { "TDMV_DCHAN",    offsetof(wandev_conf_t, tdmv_conf)+smemof(wan_tdmv_conf_t, dchan),   DTYPE_UINT},
  { "TDMV_HW_DTMF",  offsetof(wandev_conf_t, tdmv_conf)+smemof(wan_tdmv_conf_t, hw_dtmf), DTYPE_UCHAR},
  
  { "HWEC_CLKSRC",   offsetof(wandev_conf_t, tdmv_conf)+smemof(wan_hwec_conf_t, clk_src), DTYPE_UINT},  
  { "HWEC_PERSIST_DISABLE",  offsetof(wandev_conf_t, tdmv_conf)+smemof(wan_hwec_conf_t, persist_disable), DTYPE_UINT},  
  /* Keep backward compatibility */
  { "TDMV_HWEC_PERSIST_DISABLE",  offsetof(wandev_conf_t, tdmv_conf)+smemof(wan_hwec_conf_t, persist_disable), DTYPE_UINT},  
      
  { "BAUDRATE",   smemof(wandev_conf_t, bps),         DTYPE_UINT },
  { "MTU",        smemof(wandev_conf_t, mtu),         DTYPE_UINT },
  { "UDPPORT",    smemof(wandev_conf_t, udp_port),    DTYPE_UINT },
  { "TTL",	  smemof(wandev_conf_t, ttl),		DTYPE_UCHAR },
  { "INTERFACE",  smemof(wandev_conf_t, interface),   DTYPE_UCHAR },
  { "CLOCKING",   smemof(wandev_conf_t, clocking),    DTYPE_UCHAR },
  { "LINECODING", smemof(wandev_conf_t, line_coding), DTYPE_UCHAR },
  { "CONNECTION", smemof(wandev_conf_t, connection),  DTYPE_UCHAR },
  { "LINEIDLE",   smemof(wandev_conf_t, line_idle),   DTYPE_UCHAR },
  { "OPTION1",    smemof(wandev_conf_t, hw_opt[0]),   DTYPE_UINT },
  { "OPTION2",    smemof(wandev_conf_t, hw_opt[1]),   DTYPE_UINT },
  { "OPTION3",    smemof(wandev_conf_t, hw_opt[2]),   DTYPE_UINT },
  { "OPTION4",    smemof(wandev_conf_t, hw_opt[3]),   DTYPE_UINT },
  { "FIRMWARE",   smemof(wandev_conf_t, data_size),   DTYPE_FILENAME },
  { "RXMODE",     smemof(wandev_conf_t, read_mode),   DTYPE_CHAR },
  { "RECEIVE_ONLY", smemof(wandev_conf_t, receive_only), DTYPE_UCHAR},	  
  { "BACKUP",     smemof(wandev_conf_t, backup), DTYPE_UCHAR},
  { "TTY",        smemof(wandev_conf_t, tty), DTYPE_UCHAR},
  { "TTY_MAJOR",  smemof(wandev_conf_t, tty_major), DTYPE_UINT},
  { "TTY_MINOR",  smemof(wandev_conf_t, tty_minor), DTYPE_UINT},
  { "TTY_MODE",   smemof(wandev_conf_t, tty_mode), DTYPE_UINT},
  { "IGNORE_FRONT_END",  smemof(wandev_conf_t, ignore_front_end_status), DTYPE_UCHAR},

  { "MAX_RX_QUEUE", smemof(wandev_conf_t, max_rx_queue), DTYPE_UINT},
  { "LMI_TRACE_QUEUE", smemof(wandev_conf_t, max_rx_queue), DTYPE_UINT},
  
  /* ATM can be in the LIP layer but hardware setting stay in hardware section,
     so this the copy */
#if 0
  {"ATM_SYNC_MODE", offsetof(wandev_conf_t, u) + smemof(wan_atm_conf_t, atm_sync_mode), DTYPE_USHORT },
  {"ATM_SYNC_DATA", offsetof(wandev_conf_t, u) + smemof(wan_atm_conf_t,atm_sync_data), DTYPE_USHORT },
  {"ATM_SYNC_OFFSET", offsetof(wandev_conf_t, u) + smemof(wan_atm_conf_t,atm_sync_offset), DTYPE_USHORT },
  {"ATM_HUNT_TIMER", offsetof(wandev_conf_t, u) + smemof(wan_atm_conf_t,atm_hunt_timer), DTYPE_USHORT },
  {"ATM_CELL_CFG", offsetof(wandev_conf_t, u) + smemof(wan_atm_conf_t,atm_cell_cfg), DTYPE_UCHAR },
  {"ATM_CELL_PT", offsetof(wandev_conf_t, u) + smemof(wan_atm_conf_t,atm_cell_pt), DTYPE_UCHAR },
  {"ATM_CELL_CLP", offsetof(wandev_conf_t, u) + smemof(wan_atm_conf_t,atm_cell_clp), DTYPE_UCHAR },
  {"ATM_CELL_PAYLOAD", offsetof(wandev_conf_t, u) + smemof(wan_atm_conf_t,atm_cell_payload), DTYPE_UCHAR },
#endif

  { NULL, 0, 0 }
 };
 
key_word_t ppp_conftab[] =	/* PPP-specific configuration */
 {
  { "RESTARTTIMER",   smemof(wan_ppp_conf_t, restart_tmr),   DTYPE_UINT },
  { "AUTHRESTART",    smemof(wan_ppp_conf_t, auth_rsrt_tmr), DTYPE_UINT },
  { "AUTHWAIT",       smemof(wan_ppp_conf_t, auth_wait_tmr), DTYPE_UINT },
  
  { "DTRDROPTIMER",   smemof(wan_ppp_conf_t, dtr_drop_tmr),  DTYPE_UINT },
  { "CONNECTTIMEOUT", smemof(wan_ppp_conf_t, connect_tmout), DTYPE_UINT },
  { "CONFIGURERETRY", smemof(wan_ppp_conf_t, conf_retry),    DTYPE_UINT },
  { "TERMINATERETRY", smemof(wan_ppp_conf_t, term_retry),    DTYPE_UINT },
  { "MAXCONFREJECT",  smemof(wan_ppp_conf_t, fail_retry),    DTYPE_UINT },
  { "AUTHRETRY",      smemof(wan_ppp_conf_t, auth_retry),    DTYPE_UINT },
  { "AUTHENTICATOR",  smemof(wan_ppp_conf_t, authenticator), DTYPE_UCHAR},
  { "IP_MODE",        smemof(wan_ppp_conf_t, ip_mode),       DTYPE_UCHAR},
  { NULL, 0, 0 }
};




key_word_t chdlc_conftab[] =	/* Cisco HDLC-specific configuration */
 {
  { "IGNORE_DCD", smemof(wan_chdlc_conf_t, ignore_dcd), DTYPE_UCHAR},
  { "IGNORE_CTS", smemof(wan_chdlc_conf_t, ignore_cts), DTYPE_UCHAR},
  { "IGNORE_KEEPALIVE", smemof(wan_chdlc_conf_t, ignore_keepalive), DTYPE_UCHAR},
  { "HDLC_STREAMING", smemof(wan_chdlc_conf_t, hdlc_streaming), DTYPE_UCHAR},
  { "KEEPALIVE_TX_TIMER", smemof(wan_chdlc_conf_t, keepalive_tx_tmr), DTYPE_UINT },
  { "KEEPALIVE_RX_TIMER", smemof(wan_chdlc_conf_t, keepalive_rx_tmr), DTYPE_UINT },
  { "KEEPALIVE_ERR_MARGIN", smemof(wan_chdlc_conf_t, keepalive_err_margin),    DTYPE_UINT },
  { "SLARP_TIMER", smemof(wan_chdlc_conf_t, slarp_timer),    DTYPE_UINT },
  { "FAST_ISR", smemof(wan_chdlc_conf_t,fast_isr), DTYPE_UCHAR },
  { "PROTOCOL_OPTIONS",	  smemof(wan_chdlc_conf_t,protocol_options), DTYPE_UINT },
  { NULL, 0, 0 }
};


key_word_t sppp_conftab[] =	/* PPP-CHDLC specific configuration */
 {
  { "IP_MODE",        smemof(wan_sppp_if_conf_t, dynamic_ip),       DTYPE_UCHAR},
	
  { "AUTH_TIMER",     smemof(wan_sppp_if_conf_t, pp_auth_timer),    DTYPE_UINT},
  { "KEEPALIVE_TIMER",smemof(wan_sppp_if_conf_t, sppp_keepalive_timer),    DTYPE_UINT},
  { "PPP_TIMER",      smemof(wan_sppp_if_conf_t, pp_timer),    DTYPE_UINT},
 
  { "PAP",	      smemof(wan_sppp_if_conf_t, pap),    DTYPE_UCHAR},		
  { "CHAP",	      smemof(wan_sppp_if_conf_t, chap),    DTYPE_UCHAR},		
  { "CHAP",	      smemof(wan_sppp_if_conf_t, chap),    DTYPE_UCHAR},		

  { "USERID",         smemof(wan_sppp_if_conf_t, userid), 	DTYPE_STR},
  { "PASSWD",         smemof(wan_sppp_if_conf_t, passwd),	DTYPE_STR},
 
  /* note, the name is NOT "KEEPALIVE_ERR_MARGIN" intentionally, couse this name is defined
     inside wanif_conf_t, and data gets to wrong place */
  { "KEEPALIVE_ERROR_MARGIN", smemof(wan_sppp_if_conf_t, keepalive_err_margin),    DTYPE_UINT },

  { NULL, 0, 0 }
};

key_word_t fr_conftab[] =	/* Frame relay-specific configuration */
{
  { "SIGNALLING",    smemof(wan_fr_conf_t, signalling),     DTYPE_UINT },
  { "T391", 	     smemof(wan_fr_conf_t, t391),           DTYPE_UINT },
  { "T392",          smemof(wan_fr_conf_t, t392),           DTYPE_UINT },
  { "N391",	     smemof(wan_fr_conf_t, n391),           DTYPE_UINT },
  { "N392",          smemof(wan_fr_conf_t, n392),           DTYPE_UINT },
  { "N393",          smemof(wan_fr_conf_t, n393),           DTYPE_UINT },
  { "FR_ISSUE_FS",   smemof(wan_fr_conf_t, issue_fs_on_startup),DTYPE_UCHAR },
  { "NUMBER_OF_DLCI",    smemof(wan_fr_conf_t, dlci_num),       DTYPE_UINT },
  { "STATION",       smemof(wan_fr_conf_t, station),     DTYPE_UCHAR },
  { "EEK_CFG",       smemof(wan_fr_conf_t, eek_cfg),     DTYPE_UINT },
  { "EEK_TIMER",     smemof(wan_fr_conf_t, eek_timer),     DTYPE_UINT },   
  { NULL, 0, 0 }
};

key_word_t xilinx_conftab[] =	/* Xilinx specific configuration */
{
  { "MRU",     	     smemof(wan_xilinx_conf_t, mru),          DTYPE_USHORT },
  { "DMA_PER_CH",    smemof(wan_xilinx_conf_t, dma_per_ch),   DTYPE_USHORT },
  { "RBS",    	     smemof(wan_xilinx_conf_t, rbs),          DTYPE_UCHAR },
  { "DATA_MUX_MAP",  smemof(wan_xilinx_conf_t, data_mux_map), DTYPE_UINT },
//  { "HWEC_CLKSRC",   smemof(wan_xilinx_conf_t, ec_clk_src),   DTYPE_UINT},
//  { "TDMV_HWEC",     smemof(wan_xilinx_conf_t, tdmv_hwec),    DTYPE_UCHAR},
  { "RX_CRC_BYTES",  smemof(wan_xilinx_conf_t, rx_crc_bytes), DTYPE_UINT},
  { NULL, 0, 0 }
};

key_word_t bitstrm_conftab[] =	/* Bitstreaming specific configuration */
{
 /* Bit strm options */
  { "SYNC_OPTIONS",	       smemof(wan_bitstrm_conf_t, sync_options), DTYPE_USHORT },
  { "RX_SYNC_CHAR",	       smemof(wan_bitstrm_conf_t, rx_sync_char), DTYPE_USHORT},
  { "MONOSYNC_TX_TIME_FILL_CHAR", smemof(wan_bitstrm_conf_t, monosync_tx_time_fill_char), DTYPE_UCHAR},
  { "MAX_LENGTH_TX_DATA_BLOCK", smemof(wan_bitstrm_conf_t,max_length_tx_data_block), DTYPE_UINT},
  { "RX_COMPLETE_LENGTH",       smemof(wan_bitstrm_conf_t,rx_complete_length), DTYPE_UINT},
  { "RX_COMPLETE_TIMER",        smemof(wan_bitstrm_conf_t,rx_complete_timer), DTYPE_UINT},	
  { "RBS_CH_MAP",		smemof(wan_bitstrm_conf_t,rbs_map), DTYPE_UINT},
  { NULL, 0, 0 }
};

key_word_t sdlc_conftab[] =
{
  { "STATION_CONFIG",	smemof(wan_sdlc_conf_t,station_configuration), DTYPE_UCHAR},
  { "BAUD_RATE",	smemof(wan_sdlc_conf_t,baud_rate),  DTYPE_UINT},
  { "MAX_I_FIELD_LEN",	smemof(wan_sdlc_conf_t,max_I_field_length),  DTYPE_USHORT},
  { "GEN_OP_CFG_BITS",	smemof(wan_sdlc_conf_t,general_operational_config_bits),  DTYPE_USHORT},

  { "PROT_CFG_BITS",	smemof(wan_sdlc_conf_t,protocol_config_bits),  DTYPE_USHORT},
  { "EXCEP_REPORTING",	smemof(wan_sdlc_conf_t,exception_condition_reporting_config),  DTYPE_USHORT},
  { "MODEM_CFG_BITS",	smemof(wan_sdlc_conf_t,modem_config_bits),  DTYPE_USHORT},
  { "STATISTICS_FRMT",	smemof(wan_sdlc_conf_t,statistics_format),  DTYPE_USHORT},
  { "PRI_ST_SLOW_POLL",	smemof(wan_sdlc_conf_t,pri_station_slow_poll_interval),  DTYPE_USHORT},

  { "SEC_ST_RESP_TO",	smemof(wan_sdlc_conf_t,permitted_sec_station_response_TO),  DTYPE_USHORT},
  { "CNSC_NRM_B4_SNRM",	smemof(wan_sdlc_conf_t,no_consec_sec_TOs_in_NRM_before_SNRM_issued),  DTYPE_USHORT},
  { "MX_LN_IFLD_P_XID",	smemof(wan_sdlc_conf_t,max_lgth_I_fld_pri_XID_frame),  DTYPE_USHORT},
  { "O_FLG_BIT_DELAY",	smemof(wan_sdlc_conf_t,opening_flag_bit_delay_count),  DTYPE_USHORT},
  { "RTS_BIT_DELAY",	smemof(wan_sdlc_conf_t,RTS_bit_delay_count),  DTYPE_USHORT},
  { "CTS_TIMEOUT",	smemof(wan_sdlc_conf_t,CTS_timeout_1000ths_sec),  DTYPE_USHORT},
  
  { "SDLC_CONFIG",	smemof(wan_sdlc_conf_t,SDLA_configuration),  DTYPE_UCHAR},

  { NULL, 0, 0 }
};

key_word_t xdlc_conftab[] =
{

  { "STATION",		smemof(wan_xdlc_conf_t,station), DTYPE_UCHAR},
  { "MAX_I_FIELD_LEN",	smemof(wan_xdlc_conf_t,max_I_field_length),  DTYPE_UINT},
  
  { "WINDOW",		smemof(wan_xdlc_conf_t,window), 	 DTYPE_UINT},
  { "PROT_CONFIG",	smemof(wan_xdlc_conf_t,protocol_config),  DTYPE_UINT},
  { "ERROR_RESP_CONFIG", smemof(wan_xdlc_conf_t,error_response_config),  DTYPE_UINT},
  
  { "TWS_MAX_ACK_CNT",	smemof(wan_xdlc_conf_t,TWS_max_ack_count),  DTYPE_UINT},

  { "PRI_SLOW_POLL_TIMER",      smemof(wan_xdlc_conf_t,pri_slow_poll_timer),  DTYPE_UINT},
  { "PRI_NORMAL_POLL_TIMER",    smemof(wan_xdlc_conf_t,pri_normal_poll_timer),  DTYPE_UINT},
  { "PRI_FRAME_RESPONSE_TIMER", smemof(wan_xdlc_conf_t,pri_frame_response_timer),  DTYPE_UINT},
 
  { "MAX_NO_RESPONSE_CNT",	smemof(wan_xdlc_conf_t,max_no_response_cnt),  DTYPE_UINT},	
  { "MAX_FRAME_RETRANSMIT_CNT",	smemof(wan_xdlc_conf_t,max_frame_retransmit_cnt),  DTYPE_UINT},	
  { "MAX_RNR_CNT",	smemof(wan_xdlc_conf_t,max_rnr_cnt),  DTYPE_UINT},	
  
  { "SEC_NRM_TIMER",	smemof(wan_xdlc_conf_t,sec_nrm_timer),  DTYPE_UINT},
  
  { NULL, 0, 0 }
};

key_word_t tty_conftab[] =
{
  { NULL, 0, 0 }
};

key_word_t xmtp2_conftab[] =
{
  { NULL, 0, 0 }
};
key_word_t lapd_conftab[] =
{
  { NULL, 0, 0 }
};

/*
key_word_t atm_conftab[] =
{
  {"ATM_SYNC_MODE", smemof(wanif_conf_t, u) + offsetof(wan_atm_conf_t, atm_sync_mode), DTYPE_USHORT },
  {"ATM_SYNC_DATA", offsetof(wanif_conf_t, u) + offsetof(wan_atm_conf_t,atm_sync_data), DTYPE_USHORT },
  {"ATM_SYNC_OFFSET", offsetof(wanif_conf_t, u) + offsetof(wan_atm_conf_t,atm_sync_offset), DTYPE_USHORT },
  {"ATM_HUNT_TIMER", offsetof(wanif_conf_t, u) + offsetof(wan_atm_conf_t,atm_hunt_timer), DTYPE_USHORT },
  {"ATM_CELL_CFG", offsetof(wanif_conf_t, u) + offsetof(wan_atm_conf_t,atm_cell_cfg), DTYPE_UCHAR },
  {"ATM_CELL_PT", offsetof(wanif_conf_t, u) + offsetof(wan_atm_conf_t,atm_cell_pt), DTYPE_UCHAR },
  {"ATM_CELL_CLP", offsetof(wanif_conf_t, u) + offsetof(wan_atm_conf_t,atm_cell_clp), DTYPE_UCHAR },
  {"ATM_CELL_PAYLOAD", offsetof(wanif_conf_t, u) + offsetof(wan_atm_conf_t,atm_cell_payload), DTYPE_UCHAR },
  { NULL, 0, 0 }
};
*/

key_word_t atm_conftab[] =
{
  {"ATM_SYNC_MODE",	       smemof(wan_atm_conf_t,atm_sync_mode), DTYPE_USHORT },
  {"ATM_SYNC_DATA",	       smemof(wan_atm_conf_t,atm_sync_data), DTYPE_USHORT },
  {"ATM_SYNC_OFFSET", 	       smemof(wan_atm_conf_t,atm_sync_offset), DTYPE_USHORT },
  {"ATM_HUNT_TIMER", 	       smemof(wan_atm_conf_t,atm_hunt_timer), DTYPE_USHORT },
  {"ATM_CELL_CFG", 	       smemof(wan_atm_conf_t,atm_cell_cfg), DTYPE_UCHAR },
  {"ATM_CELL_PT", 	       smemof(wan_atm_conf_t,atm_cell_pt), DTYPE_UCHAR },
  {"ATM_CELL_CLP", 	       smemof(wan_atm_conf_t,atm_cell_clp), DTYPE_UCHAR },
  {"ATM_CELL_PAYLOAD", 	       smemof(wan_atm_conf_t,atm_cell_payload), DTYPE_UCHAR },
  { NULL, 0, 0 }
};


key_word_t atm_if_conftab[] =
{
	
  /* Per Interface Configuration */
  {"ENCAPMODE",        smemof(wan_atm_conf_if_t, encap_mode), DTYPE_UCHAR },
  {"VCI", 	       smemof(wan_atm_conf_if_t, vci), DTYPE_USHORT },
  {"VPI", 	       smemof(wan_atm_conf_if_t, vpi), DTYPE_USHORT },

  {"OAM_LOOPBACK",	smemof(wan_atm_conf_if_t,atm_oam_loopback),  DTYPE_UCHAR },
  {"OAM_LOOPBACK_INT",  smemof(wan_atm_conf_if_t,atm_oam_loopback_intr),  DTYPE_UCHAR },
  {"OAM_CC_CHECK",	smemof(wan_atm_conf_if_t,atm_oam_continuity),  DTYPE_UCHAR },
  {"OAM_CC_CHECK_INT",	smemof(wan_atm_conf_if_t,atm_oam_continuity_intr),  DTYPE_UCHAR },
  {"ATMARP",		smemof(wan_atm_conf_if_t,atm_arp),  DTYPE_UCHAR },
  {"ATMARP_INT",	smemof(wan_atm_conf_if_t,atm_arp_intr),  DTYPE_UCHAR },
  {"MTU",		smemof(wan_atm_conf_if_t,mtu),  DTYPE_USHORT },

  /* Profile Section */
  {"ATM_SYNC_MODE",	       smemof(wan_atm_conf_if_t,atm_sync_mode), DTYPE_USHORT },
  {"ATM_SYNC_DATA",	       smemof(wan_atm_conf_if_t,atm_sync_data), DTYPE_USHORT },
  {"ATM_SYNC_OFFSET", 	       smemof(wan_atm_conf_if_t,atm_sync_offset), DTYPE_USHORT },
  {"ATM_HUNT_TIMER", 	       smemof(wan_atm_conf_if_t,atm_hunt_timer), DTYPE_USHORT },
  {"ATM_CELL_CFG", 	       smemof(wan_atm_conf_if_t,atm_cell_cfg), DTYPE_UCHAR },
  {"ATM_CELL_PT", 	       smemof(wan_atm_conf_if_t,atm_cell_pt), DTYPE_UCHAR },
  {"ATM_CELL_CLP", 	       smemof(wan_atm_conf_if_t,atm_cell_clp), DTYPE_UCHAR },
  {"ATM_CELL_PAYLOAD", 	       smemof(wan_atm_conf_if_t,atm_cell_payload), DTYPE_UCHAR },

  { NULL, 0, 0 }
};

key_word_t xilinx_if_conftab[] =
{
  { "SIGNALLING",     smemof(wan_xilinx_conf_if_t, signalling),  DTYPE_UINT },
  { "STATION",        smemof(wan_xilinx_conf_if_t, station),     DTYPE_UCHAR },
  { "SEVEN_BIT_HDLC", smemof(wan_xilinx_conf_if_t, seven_bit_hdlc), DTYPE_CHAR },
  { "MRU",     	smemof(wan_xilinx_conf_if_t, mru),  DTYPE_UINT },
  { "MTU",     	smemof(wan_xilinx_conf_if_t, mtu),  DTYPE_UINT },
  { "IDLE_FLAG",     smemof(wan_xilinx_conf_if_t, idle_flag),  DTYPE_UCHAR},
  { "DATA_MUX",    smemof(wan_xilinx_conf_if_t, data_mux),  DTYPE_UCHAR}, 
  { "SS7_ENABLE",  smemof(wan_xilinx_conf_if_t, ss7_enable),  DTYPE_UCHAR},
  { "SS7_MODE",  smemof(wan_xilinx_conf_if_t, ss7_mode),  DTYPE_UCHAR},
  { "SS7_LSSU_SZ",  smemof(wan_xilinx_conf_if_t, ss7_lssu_size),  DTYPE_UCHAR},
//  { "TDMV_HWEC_MAP", smemof(wan_xilinx_conf_if_t, tdmv_hwec_map),    DTYPE_STR},
//  { "TDMV_HWEC",     smemof(wan_xilinx_conf_if_t, tdmv_hwec),    DTYPE_UCHAR},
  { "RBS_CAS_IDLE",  smemof(wan_xilinx_conf_if_t, rbs_cas_idle), DTYPE_UCHAR },
  { NULL, 0, 0 }
};


key_word_t bitstrm_if_conftab[] =
{
  {"MAX_TX_QUEUE", smemof(wan_bitstrm_conf_if_t,max_tx_queue_size), DTYPE_UINT },
  {"MAX_TX_UP_SIZE", smemof(wan_bitstrm_conf_if_t,max_tx_up_size), DTYPE_UINT },
  {"SEVEN_BIT_HDLC", smemof(wan_bitstrm_conf_if_t,seven_bit_hdlc), DTYPE_CHAR },
  { NULL, 0, 0 }
};


key_word_t adsl_conftab[] =
{
  {"ENCAPMODE", 	smemof(wan_adsl_conf_t,EncapMode), DTYPE_UCHAR },
  /*Backward compatibility*/
  {"RFC1483MODE", 	smemof(wan_adsl_conf_t,EncapMode), DTYPE_UCHAR },
  {"RFC2364MODE", 	smemof(wan_adsl_conf_t,EncapMode), DTYPE_UCHAR },
  
  {"VCI", 		smemof(wan_adsl_conf_t,Vci), DTYPE_USHORT },
  /*Backward compatibility*/
  {"RFC1483VCI",	smemof(wan_adsl_conf_t,Vci), DTYPE_USHORT },
  {"RFC2364VCI",	smemof(wan_adsl_conf_t,Vci), DTYPE_USHORT },
  
  {"VPI", 		smemof(wan_adsl_conf_t,Vpi), DTYPE_USHORT },
  /*Backward compatibility*/
  {"RFC1483VPI", 	smemof(wan_adsl_conf_t,Vpi), DTYPE_USHORT },
  {"RFC2364VPI", 	smemof(wan_adsl_conf_t,Vpi), DTYPE_USHORT },
  
  {"VERBOSE", 		smemof(wan_adsl_conf_t,Verbose), DTYPE_UCHAR },
  /*Backward compatibility*/
  {"DSL_INTERFACE", 	smemof(wan_adsl_conf_t,Verbose), DTYPE_UCHAR },
  
  {"RXBUFFERCOUNT", 	smemof(wan_adsl_conf_t,RxBufferCount), DTYPE_USHORT },
  {"TXBUFFERCOUNT", 	smemof(wan_adsl_conf_t,TxBufferCount), DTYPE_USHORT },
  
  {"ADSLSTANDARD",      smemof(wan_adsl_conf_t,Standard), DTYPE_USHORT },
  {"ADSLTRELLIS",       smemof(wan_adsl_conf_t,Trellis), DTYPE_USHORT },
  {"ADSLTXPOWERATTEN",  smemof(wan_adsl_conf_t,TxPowerAtten), DTYPE_USHORT },
  {"ADSLCODINGGAIN",    smemof(wan_adsl_conf_t,CodingGain), DTYPE_USHORT },
  {"ADSLMAXBITSPERBIN", smemof(wan_adsl_conf_t,MaxBitsPerBin), DTYPE_USHORT },
  {"ADSLTXSTARTBIN",    smemof(wan_adsl_conf_t,TxStartBin), DTYPE_USHORT },
  {"ADSLTXENDBIN",      smemof(wan_adsl_conf_t,TxEndBin), DTYPE_USHORT },
  {"ADSLRXSTARTBIN",    smemof(wan_adsl_conf_t,RxStartBin), DTYPE_USHORT },
  {"ADSLRXENDBIN",      smemof(wan_adsl_conf_t,RxEndBin), DTYPE_USHORT },
  {"ADSLRXBINADJUST",   smemof(wan_adsl_conf_t,RxBinAdjust), DTYPE_USHORT },
  {"ADSLFRAMINGSTRUCT", smemof(wan_adsl_conf_t,FramingStruct), DTYPE_USHORT },
  {"ADSLEXPANDEDEXCHANGE",      smemof(wan_adsl_conf_t,ExpandedExchange), DTYPE_USHORT },
  {"ADSLCLOCKTYPE",     smemof(wan_adsl_conf_t,ClockType), DTYPE_USHORT },
  {"ADSLMAXDOWNRATE",   smemof(wan_adsl_conf_t,MaxDownRate), DTYPE_USHORT },

  /*Backward compatibility*/
  {"GTISTANDARD",      smemof(wan_adsl_conf_t,Standard), DTYPE_USHORT },
  {"GTITRELLIS",       smemof(wan_adsl_conf_t,Trellis), DTYPE_USHORT },
  {"GTITXPOWERATTEN",  smemof(wan_adsl_conf_t,TxPowerAtten), DTYPE_USHORT },
  {"GTICODINGGAIN",    smemof(wan_adsl_conf_t,CodingGain), DTYPE_USHORT },
  {"GTIMAXBITSPERBIN", smemof(wan_adsl_conf_t,MaxBitsPerBin), DTYPE_USHORT },
  {"GTITXSTARTBIN",    smemof(wan_adsl_conf_t,TxStartBin), DTYPE_USHORT },
  {"GTITXENDBIN",      smemof(wan_adsl_conf_t,TxEndBin), DTYPE_USHORT },
  {"GTIRXSTARTBIN",    smemof(wan_adsl_conf_t,RxStartBin), DTYPE_USHORT },
  {"GTIRXENDBIN",      smemof(wan_adsl_conf_t,RxEndBin), DTYPE_USHORT },
  {"GTIRXBINADJUST",   smemof(wan_adsl_conf_t,RxBinAdjust), DTYPE_USHORT },
  {"GTIFRAMINGSTRUCT", smemof(wan_adsl_conf_t,FramingStruct), DTYPE_USHORT },
  {"GTIEXPANDEDEXCHANGE",      smemof(wan_adsl_conf_t,ExpandedExchange), DTYPE_USHORT },
  {"GTICLOCKTYPE",     smemof(wan_adsl_conf_t,ClockType), DTYPE_USHORT },
  {"GTIMAXDOWNRATE",   smemof(wan_adsl_conf_t,MaxDownRate), DTYPE_USHORT },

  {"ATM_AUTOCFG", 	smemof(wan_adsl_conf_t,atm_autocfg), DTYPE_UCHAR },
  {"ADSL_WATCHDOG",	smemof(wan_adsl_conf_t,atm_watchdog), DTYPE_UCHAR },
  { NULL, 0, 0 }
};

key_word_t bscstrm_conftab[]=
{
  {"BSCSTRM_ADAPTER_FR" , smemof(wan_bscstrm_conf_t,adapter_frequency),DTYPE_UINT},
  {"BSCSTRM_MTU", 	smemof(wan_bscstrm_conf_t,max_data_length),DTYPE_USHORT},
  {"BSCSTRM_EBCDIC" ,  smemof(wan_bscstrm_conf_t,EBCDIC_encoding),DTYPE_USHORT},
  {"BSCSTRM_RB_BLOCK_TYPE", smemof(wan_bscstrm_conf_t,Rx_block_type),DTYPE_USHORT},
  {"BSCSTRM_NO_CONSEC_PAD_EOB", smemof(wan_bscstrm_conf_t,no_consec_PADs_EOB), DTYPE_USHORT},
  {"BSCSTRM_NO_ADD_LEAD_TX_SYN_CHARS", smemof(wan_bscstrm_conf_t,no_add_lead_Tx_SYN_chars),DTYPE_USHORT},
  {"BSCSTRM_NO_BITS_PER_CHAR", smemof(wan_bscstrm_conf_t,no_bits_per_char),DTYPE_USHORT},
  {"BSCSTRM_PARITY", smemof(wan_bscstrm_conf_t,parity),DTYPE_USHORT},
  {"BSCSTRM_MISC_CONFIG_OPTIONS",  smemof(wan_bscstrm_conf_t,misc_config_options),DTYPE_USHORT},
  {"BSCSTRM_STATISTICS_OPTIONS", smemof(wan_bscstrm_conf_t,statistics_options),  DTYPE_USHORT},
  {"BSCSTRM_MODEM_CONFIG_OPTIONS", smemof(wan_bscstrm_conf_t,modem_config_options), DTYPE_USHORT},
  { NULL, 0, 0 }
};


key_word_t ss7_conftab[] =
{
  {"LINE_CONFIG_OPTIONS", smemof(wan_ss7_conf_t,line_cfg_opt),  DTYPE_UINT },
  {"MODEM_CONFIG_OPTIONS",smemof(wan_ss7_conf_t,modem_cfg_opt), DTYPE_UINT },
  {"MODEM_STATUS_TIMER",  smemof(wan_ss7_conf_t,modem_status_timer), DTYPE_UINT },
  {"API_OPTIONS",	  smemof(wan_ss7_conf_t,api_options),	  DTYPE_UINT },
  {"PROTOCOL_OPTIONS",	  smemof(wan_ss7_conf_t,protocol_options), DTYPE_UINT },
  {"PROTOCOL_SPECIFICATION", smemof(wan_ss7_conf_t,protocol_specification), DTYPE_UINT },
  {"STATS_HISTORY_OPTIONS", smemof(wan_ss7_conf_t,stats_history_options), DTYPE_UINT },
  {"MAX_LENGTH_MSU_SIF", smemof(wan_ss7_conf_t,max_length_msu_sif), DTYPE_UINT },
  {"MAX_UNACKED_TX_MSUS", smemof(wan_ss7_conf_t,max_unacked_tx_msus), DTYPE_UINT },
  {"LINK_INACTIVITY_TIMER", smemof(wan_ss7_conf_t,link_inactivity_timer), DTYPE_UINT }, 
  {"T1_TIMER",		  smemof(wan_ss7_conf_t,t1_timer),	  DTYPE_UINT },
  {"T2_TIMER",		  smemof(wan_ss7_conf_t,t2_timer),	  DTYPE_UINT },
  {"T3_TIMER",		  smemof(wan_ss7_conf_t,t3_timer),	  DTYPE_UINT },
  {"T4_TIMER_EMERGENCY",  smemof(wan_ss7_conf_t,t4_timer_emergency), DTYPE_UINT },
  {"T4_TIMER_NORMAL",  smemof(wan_ss7_conf_t,t4_timer_normal), DTYPE_UINT },
  {"T5_TIMER",		  smemof(wan_ss7_conf_t,t5_timer),	  DTYPE_UINT },
  {"T6_TIMER",		  smemof(wan_ss7_conf_t,t6_timer),	  DTYPE_UINT },
  {"T7_TIMER",		  smemof(wan_ss7_conf_t,t7_timer),	  DTYPE_UINT },
  {"T8_TIMER",		  smemof(wan_ss7_conf_t,t8_timer),	  DTYPE_UINT },
  {"N1",		  smemof(wan_ss7_conf_t,n1),	  DTYPE_UINT },
  {"N2",		  smemof(wan_ss7_conf_t,n2),	  DTYPE_UINT },
  {"TIN",		  smemof(wan_ss7_conf_t,tin),	  DTYPE_UINT },
  {"TIE",		  smemof(wan_ss7_conf_t,tie),	  DTYPE_UINT },
  {"SUERM_ERROR_THRESHOLD", smemof(wan_ss7_conf_t,suerm_error_threshold), DTYPE_UINT},
  {"SUERM_NUMBER_OCTETS", smemof(wan_ss7_conf_t,suerm_number_octets), DTYPE_UINT},
  {"SUERM_NUMBER_SUS", smemof(wan_ss7_conf_t,suerm_number_sus), DTYPE_UINT},
  {"SIE_INTERVAL_TIMER", smemof(wan_ss7_conf_t,sie_interval_timer), DTYPE_UINT},
  {"SIO_INTERVAL_TIMER", smemof(wan_ss7_conf_t,sio_interval_timer), DTYPE_UINT},
  {"SIOS_INTERVAL_TIMER", smemof(wan_ss7_conf_t,sios_interval_timer), DTYPE_UINT},
  {"FISU_INTERVAL_TIMER", smemof(wan_ss7_conf_t,fisu_interval_timer), DTYPE_UINT},
  { NULL, 0, 0 }
};



key_word_t x25_conftab[] =	/* X.25-specific configuration */
{
  { "LOWESTPVC",    smemof(wan_x25_conf_t, lo_pvc),       DTYPE_UINT },
  { "HIGHESTPVC",   smemof(wan_x25_conf_t, hi_pvc),       DTYPE_UINT },
  { "LOWESTSVC",    smemof(wan_x25_conf_t, lo_svc),       DTYPE_UINT },
  { "HIGHESTSVC",   smemof(wan_x25_conf_t, hi_svc),       DTYPE_UINT },
  { "HDLCWINDOW",   smemof(wan_x25_conf_t, hdlc_window),  DTYPE_UINT },
  { "PACKETWINDOW", smemof(wan_x25_conf_t, pkt_window),   DTYPE_UINT },
  { "T1", 	    smemof(wan_x25_conf_t, t1), DTYPE_UINT },
  { "T2", 	    smemof(wan_x25_conf_t, t2), DTYPE_UINT },	
  { "T4", 	    smemof(wan_x25_conf_t, t4), DTYPE_UINT },
  { "N2", 	    smemof(wan_x25_conf_t, n2), DTYPE_UINT },
  { "T10_T20", 	    smemof(wan_x25_conf_t, t10_t20), DTYPE_UINT },
  { "T11_T21", 	    smemof(wan_x25_conf_t, t11_t21), DTYPE_UINT },	
  { "T12_T22", 	    smemof(wan_x25_conf_t, t12_t22), DTYPE_UINT },
  { "T13_T23", 	    smemof(wan_x25_conf_t, t13_t23), DTYPE_UINT },
  { "T16_T26", 	    smemof(wan_x25_conf_t, t16_t26), DTYPE_UINT },
  { "T28", 	    smemof(wan_x25_conf_t, t28), DTYPE_UINT },
  { "R10_R20", 	    smemof(wan_x25_conf_t, r10_r20), DTYPE_UINT },
  { "R12_R22", 	    smemof(wan_x25_conf_t, r12_r22), DTYPE_UINT },
  { "R13_R23", 	    smemof(wan_x25_conf_t, r13_r23), DTYPE_UINT },
  { "CCITTCOMPAT",  smemof(wan_x25_conf_t, ccitt_compat), DTYPE_UINT },
  { "X25CONFIG",    smemof(wan_x25_conf_t, x25_conf_opt), DTYPE_UINT }, 	
  { "LAPB_HDLC_ONLY", smemof(wan_x25_conf_t, LAPB_hdlc_only), DTYPE_UCHAR },
  { "CALL_SETUP_LOG", smemof(wan_x25_conf_t, logging), DTYPE_UCHAR },
  { "OOB_ON_MODEM",   smemof(wan_x25_conf_t, oob_on_modem), DTYPE_UCHAR},
  { "STATION_ADDR", smemof(wan_x25_conf_t, local_station_address), DTYPE_UCHAR},
  { "DEF_PKT_SIZE", smemof(wan_x25_conf_t, defPktSize),  DTYPE_USHORT },
  { "MAX_PKT_SIZE", smemof(wan_x25_conf_t, pktMTU),  DTYPE_USHORT },
  { "STATION",      smemof(wan_x25_conf_t, station),  DTYPE_UCHAR },
  { NULL, 0, 0 }
};

key_word_t lapb_annexg_conftab[] =
{
  //LAPB STUFF
  { "T1", smemof(wan_lapb_if_conf_t, t1),    DTYPE_UINT },
  { "T2", smemof(wan_lapb_if_conf_t, t2),    DTYPE_UINT },
  { "T3", smemof(wan_lapb_if_conf_t, t3),    DTYPE_UINT },
  { "T4", smemof(wan_lapb_if_conf_t, t4),    DTYPE_UINT },
  { "N2", smemof(wan_lapb_if_conf_t, n2),    DTYPE_UINT },
  { "LAPB_MODE", 	smemof(wan_lapb_if_conf_t, mode),    DTYPE_UINT },
  { "LAP_MODE", 	smemof(wan_lapb_if_conf_t, mode),    DTYPE_UINT },
  { "MODE", 		smemof(wan_lapb_if_conf_t, mode),    DTYPE_UINT },
  { "LAPB_WINDOW", 	smemof(wan_lapb_if_conf_t, window),    DTYPE_UINT },
  { "LAP_WINDOW", 	smemof(wan_lapb_if_conf_t, window),    DTYPE_UINT },
  { "WINDOW", 		smemof(wan_lapb_if_conf_t, window),    DTYPE_UINT },
  { "LAP_STATION",     	smemof(wan_lapb_if_conf_t, station),    DTYPE_UINT },

  { "LABEL",		smemof(wan_lapb_if_conf_t,label), DTYPE_STR},
  { "VIRTUAL_ADDR",     smemof(wan_lapb_if_conf_t,virtual_addr), DTYPE_STR},
  { "REAL_ADDR",        smemof(wan_lapb_if_conf_t,real_addr), DTYPE_STR},

  { "MAX_PKT_SIZE", 	smemof(wan_lapb_if_conf_t,mtu), DTYPE_UINT},

  { NULL, 0, 0, 0 }
};

key_word_t x25_annexg_conftab[] =
{
 //X25 STUFF
  { "PACKETWINDOW", smemof(wan_x25_if_conf_t, packet_window),   DTYPE_USHORT },
  { "CCITTCOMPAT",  smemof(wan_x25_if_conf_t, CCITT_compatibility), DTYPE_USHORT },
  { "T10_T20", 	    smemof(wan_x25_if_conf_t, T10_T20), DTYPE_USHORT },
  { "T11_T21", 	    smemof(wan_x25_if_conf_t, T11_T21), DTYPE_USHORT },	
  { "T12_T22", 	    smemof(wan_x25_if_conf_t, T12_T22), DTYPE_USHORT },
  { "T13_T23", 	    smemof(wan_x25_if_conf_t, T13_T23), DTYPE_USHORT },
  { "T16_T26", 	    smemof(wan_x25_if_conf_t, T16_T26), DTYPE_USHORT },
  { "T28", 	    smemof(wan_x25_if_conf_t, T28),     DTYPE_USHORT },
  { "R10_R20", 	    smemof(wan_x25_if_conf_t, R10_R20), DTYPE_USHORT },
  { "R12_R22", 	    smemof(wan_x25_if_conf_t, R12_R22), DTYPE_USHORT },
  { "R13_R23", 	    smemof(wan_x25_if_conf_t, R13_R23), DTYPE_USHORT },
  
  { "X25_API_OPTIONS", smemof(wan_x25_if_conf_t, X25_API_options), DTYPE_USHORT },
  { "X25_PROTOCOL_OPTIONS", smemof(wan_x25_if_conf_t, X25_protocol_options), DTYPE_USHORT },
  { "X25_RESPONSE_OPTIONS", smemof(wan_x25_if_conf_t, X25_response_options), DTYPE_USHORT },
  { "X25_STATISTICS_OPTIONS", smemof(wan_x25_if_conf_t, X25_statistics_options), DTYPE_USHORT },
 
  { "GEN_FACILITY_1", smemof(wan_x25_if_conf_t, genl_facilities_supported_1), DTYPE_USHORT },
  { "GEN_FACILITY_2", smemof(wan_x25_if_conf_t, genl_facilities_supported_2), DTYPE_USHORT },
  { "CCITT_FACILITY", smemof(wan_x25_if_conf_t, CCITT_facilities_supported), DTYPE_USHORT },
  { "NON_X25_FACILITY",	smemof(wan_x25_if_conf_t,non_X25_facilities_supported),DTYPE_USHORT },

  { "DFLT_PKT_SIZE", smemof(wan_x25_if_conf_t,default_packet_size), DTYPE_USHORT },
  { "MAX_PKT_SIZE",  smemof(wan_x25_if_conf_t,maximum_packet_size), DTYPE_USHORT },

  { "LOWESTSVC",   smemof(wan_x25_if_conf_t,lowest_two_way_channel), DTYPE_USHORT },
  { "HIGHESTSVC",  smemof(wan_x25_if_conf_t,highest_two_way_channel), DTYPE_USHORT},
  
  { "LOWESTPVC",   smemof(wan_x25_if_conf_t,lowest_PVC), DTYPE_USHORT },
  { "HIGHESTPVC",  smemof(wan_x25_if_conf_t,highest_PVC), DTYPE_USHORT},

  { "X25_MODE", smemof(wan_x25_if_conf_t, mode), DTYPE_UCHAR},
  { "CALL_BACKOFF", smemof(wan_x25_if_conf_t, call_backoff_timeout), DTYPE_UINT },
  { "CALL_LOGGING", smemof(wan_x25_if_conf_t, call_logging), DTYPE_UCHAR },
 
  { "X25_CALL_STRING",      smemof(wan_x25_if_conf_t, call_string),     DTYPE_STR},
  { "X25_ACCEPT_DST_ADDR",  smemof(wan_x25_if_conf_t, accept_called),   DTYPE_STR},
  { "X25_ACCEPT_SRC_ADDR",  smemof(wan_x25_if_conf_t, accept_calling),  DTYPE_STR},
  { "X25_ACCEPT_FCL_DATA",  smemof(wan_x25_if_conf_t, accept_facil),    DTYPE_STR},
  { "X25_ACCEPT_USR_DATA",  smemof(wan_x25_if_conf_t, accept_udata),    DTYPE_STR},

  { "LABEL",		smemof(wan_x25_if_conf_t,label),        DTYPE_STR},
  { "VIRTUAL_ADDR",     smemof(wan_x25_if_conf_t,virtual_addr), DTYPE_STR},
  { "REAL_ADDR",        smemof(wan_x25_if_conf_t,real_addr),    DTYPE_STR},
  
  { NULL, 0, 0, 0 }
};

key_word_t dsp_annexg_conftab[] =
{
  //DSP_20 DSP STUFF
  { "PAD",		smemof(wan_dsp_if_conf_t, pad_type),		DTYPE_UCHAR },
  { "T1_DSP",  		smemof(wan_dsp_if_conf_t, T1), 		DTYPE_UINT },
  { "T2_DSP",  		smemof(wan_dsp_if_conf_t, T2), 		DTYPE_UINT },
  { "T3_DSP",  		smemof(wan_dsp_if_conf_t, T3), 		DTYPE_UINT },
  { "DSP_AUTO_CE",  	smemof(wan_dsp_if_conf_t, auto_ce), 		DTYPE_UCHAR },
  { "DSP_AUTO_CALL_REQ",smemof(wan_dsp_if_conf_t, auto_call_req), 	DTYPE_UCHAR },
  { "DSP_AUTO_ACK",  	smemof(wan_dsp_if_conf_t, auto_ack), 		DTYPE_UCHAR },
  { "DSP_MTU",  	smemof(wan_dsp_if_conf_t, dsp_mtu), 		DTYPE_USHORT },
  { NULL, 0, 0, 0 }
};

key_word_t chan_conftab[] =	/* Channel configuration parameters */
{
  { "IDLETIMEOUT",   	smemof(wanif_conf_t, idle_timeout), 	DTYPE_UINT },
  { "HOLDTIMEOUT",   	smemof(wanif_conf_t, hold_timeout), 	DTYPE_UINT },
  { "X25_SRC_ADDR",   	smemof(wanif_conf_t, x25_src_addr), 	DTYPE_STR},
  { "X25_ACCEPT_DST_ADDR",  smemof(wanif_conf_t, accept_dest_addr), DTYPE_STR},
  { "X25_ACCEPT_SRC_ADDR",  smemof(wanif_conf_t, accept_src_addr),  DTYPE_STR},
  { "X25_ACCEPT_USR_DATA",  smemof(wanif_conf_t, accept_usr_data),  DTYPE_STR},
  { "CIR",           	smemof(wanif_conf_t, cir), 	   	DTYPE_UINT },
  { "BC",            	smemof(wanif_conf_t, bc),		DTYPE_UINT },
  { "BE", 	     	smemof(wanif_conf_t, be),		DTYPE_UINT },
  { "MULTICAST",     	smemof(wanif_conf_t, mc),		DTYPE_UCHAR},
  { "IPX",	     	smemof(wanif_conf_t, enable_IPX),	DTYPE_UCHAR},
  { "NETWORK",       	smemof(wanif_conf_t, network_number),	DTYPE_UINT}, 
  { "PAP",     	     	smemof(wanif_conf_t, pap),		DTYPE_UCHAR},
  { "CHAP",          	smemof(wanif_conf_t, chap),		DTYPE_UCHAR},
  { "USERID",        	smemof(wanif_conf_t, userid),	 	DTYPE_STR},
  { "PASSWD",        	smemof(wanif_conf_t, passwd),		DTYPE_STR},
  { "SYSNAME",       	smemof(wanif_conf_t, sysname),		DTYPE_STR},
  { "INARP", 	     	smemof(wanif_conf_t, inarp),          	DTYPE_UCHAR},
  { "INARPINTERVAL", 	smemof(wanif_conf_t, inarp_interval), 	DTYPE_UINT },
  { "INARP_RX",      	smemof(wanif_conf_t, inarp_rx),          	DTYPE_UCHAR},
  { "IGNORE_DCD",  	smemof(wanif_conf_t, ignore_dcd),        	DTYPE_UCHAR},
  { "IGNORE_CTS",    	smemof(wanif_conf_t, ignore_cts),        	DTYPE_UCHAR},
  { "IGNORE_KEEPALIVE", smemof(wanif_conf_t, ignore_keepalive), 	DTYPE_UCHAR},
  { "HDLC_STREAMING", 	smemof(wanif_conf_t, hdlc_streaming), 	DTYPE_UCHAR},

  { "KEEPALIVE_TX_TIMER",	smemof(wanif_conf_t, keepalive_tx_tmr), 	DTYPE_UINT },
  { "KEEPALIVE_RX_TIMER",	smemof(wanif_conf_t, keepalive_rx_tmr), 	DTYPE_UINT },
  { "KEEPALIVE_ERR_MARGIN",	smemof(wanif_conf_t, keepalive_err_margin),	DTYPE_UINT },
  { "SLARP_TIMER", 	smemof(wanif_conf_t, slarp_timer),    		DTYPE_UINT },

  { "TTL",        	smemof(wanif_conf_t, ttl),         DTYPE_UCHAR },

 // { "INTERFACE",  	smemof(wanif_conf_t, interface),   DTYPE_UCHAR },
 // { "CLOCKING",   	smemof(wanif_conf_t, clocking),    DTYPE_UCHAR },
 // { "BAUDRATE",   	smemof(wanif_conf_t, bps),         DTYPE_UINT },
 
  { "STATION" ,          smemof(wanif_conf_t, station),     DTYPE_UCHAR },
  { "DYN_INTR_CFG",  	smemof(wanif_conf_t, if_down),     DTYPE_UCHAR },
  { "GATEWAY",  	smemof(wanif_conf_t, gateway),     DTYPE_UCHAR },
  { "TRUE_ENCODING_TYPE", smemof(wanif_conf_t,true_if_encoding), DTYPE_UCHAR },

  /* ASYNC Options */
  { "ASYNC_MODE",    	       smemof(wanif_conf_t, async_mode), DTYPE_UCHAR},	
  { "ASY_DATA_TRANSPARENT",    smemof(wanif_conf_t, asy_data_trans), DTYPE_UCHAR}, 
  { "RTS_HS_FOR_RECEIVE",      smemof(wanif_conf_t, rts_hs_for_receive), DTYPE_UCHAR},
  { "XON_XOFF_HS_FOR_RECEIVE", smemof(wanif_conf_t, xon_xoff_hs_for_receive), DTYPE_UCHAR},
  { "XON_XOFF_HS_FOR_TRANSMIT",smemof(wanif_conf_t, xon_xoff_hs_for_transmit), DTYPE_UCHAR},
  { "DCD_HS_FOR_TRANSMIT",     smemof(wanif_conf_t, dcd_hs_for_transmit), DTYPE_UCHAR},	
  { "CTS_HS_FOR_TRANSMIT",     smemof(wanif_conf_t, cts_hs_for_transmit), DTYPE_UCHAR},
  { "TX_BITS_PER_CHAR",        smemof(wanif_conf_t, tx_bits_per_char),    DTYPE_UINT },
  { "RX_BITS_PER_CHAR",        smemof(wanif_conf_t, rx_bits_per_char),    DTYPE_UINT },
  { "STOP_BITS",               smemof(wanif_conf_t, stop_bits),    DTYPE_UINT },
  { "PARITY",                  smemof(wanif_conf_t, parity),    DTYPE_UCHAR },
  { "BREAK_TIMER",             smemof(wanif_conf_t, break_timer),    DTYPE_UINT },	
  { "INTER_CHAR_TIMER",        smemof(wanif_conf_t, inter_char_timer),    DTYPE_UINT },
  { "RX_COMPLETE_LENGTH",      smemof(wanif_conf_t, rx_complete_length),    DTYPE_UINT },
  { "XON_CHAR",                smemof(wanif_conf_t, xon_char),    DTYPE_UINT },	
  { "XOFF_CHAR",               smemof(wanif_conf_t, xoff_char),    DTYPE_UINT },	   
  { "MPPP_PROT",	       smemof(wanif_conf_t, protocol),  DTYPE_UCHAR},
  { "PROTOCOL",	       	       smemof(wanif_conf_t, protocol),  DTYPE_UCHAR},
  { "ACTIVE_CH",	       smemof(wanif_conf_t, active_ch),  DTYPE_UINT},
  { "SW_DEV_NAME",	       smemof(wanif_conf_t, sw_dev_name),  DTYPE_STR},

  { "LIP_MTU",        	       smemof(wanif_conf_t, mtu),  DTYPE_UINT},

  { "DLCI_TRACE_QUEUE", smemof(wanif_conf_t, max_trace_queue), DTYPE_UINT},
  { "MAX_TRACE_QUEUE", smemof(wanif_conf_t, max_trace_queue), DTYPE_UINT},

  { "TDMV_ECHO_OFF", offsetof(wanif_conf_t, tdmv)+smemof(wan_tdmv_if_conf_t, tdmv_echo_off), DTYPE_UCHAR},
  { "TDMV_CODEC",    offsetof(wanif_conf_t, tdmv)+smemof(wan_tdmv_if_conf_t, tdmv_codec),    DTYPE_UCHAR},
  { "TDMV_HWEC",     offsetof(wanif_conf_t, hwec)+smemof(wan_hwec_if_conf_t, enable),    DTYPE_UCHAR},
//  { "TDMV_ECHO_OFF",	smemof(wanif_conf_t, tdmv_echo_off), DTYPE_UCHAR},
//  { "TDMV_CODEC",	smemof(wanif_conf_t, tdmv_codec), DTYPE_UCHAR},

  { "SINGLE_TX_BUF",    smemof(wanif_conf_t, single_tx_buf), DTYPE_UCHAR},

  { NULL, 0, 0, 0 }
};

look_up_t conf_def_tables[] =
{
	{ WANCONFIG_PPP,	ppp_conftab	},
	{ WANCONFIG_FR,		fr_conftab	},
	{ WANCONFIG_X25,	x25_conftab	},
	{ WANCONFIG_ADCCP,	x25_conftab	},
	{ WANCONFIG_CHDLC,	chdlc_conftab	},
	{ WANCONFIG_ASYHDLC,	chdlc_conftab	},
	{ WANCONFIG_MPPP,	chdlc_conftab	},
	{ WANCONFIG_LIP_ATM,	atm_if_conftab	},
	{ WANCONFIG_LIP_KATM,	atm_if_conftab	},
	{ WANCONFIG_MFR,	fr_conftab	},
	{ WANCONFIG_SS7,	ss7_conftab 	},
	{ WANCONFIG_ADSL,	adsl_conftab 	},
	{ WANCONFIG_BSCSTRM,	bscstrm_conftab	},
	{ WANCONFIG_ATM,	atm_conftab	},
	{ WANCONFIG_MLINK_PPP,	chdlc_conftab	},
	{ WANCONFIG_AFT,        xilinx_conftab  },
	{ WANCONFIG_AFT_TE1,    xilinx_conftab  },
	{ WANCONFIG_AFT_ANALOG, xilinx_conftab  },
	{ WANCONFIG_AFT_TE3,    xilinx_conftab  },
	{ WANCONFIG_AFT_56K,    xilinx_conftab  },
	{ WANCONFIG_BITSTRM,    bitstrm_conftab },
	{ WANCONFIG_SDLC,	sdlc_conftab 	},
	{ 0,			NULL		}
};

look_up_t conf_if_def_tables[] =
{
	{ WANCONFIG_ATM,	atm_if_conftab	},
	{ WANCONFIG_LIP_ATM,	atm_if_conftab	},
	{ WANCONFIG_LIP_KATM,	atm_if_conftab	},
	{ WANCONFIG_BITSTRM,    bitstrm_if_conftab },
	{ WANCONFIG_AFT,	xilinx_if_conftab },
	{ WANCONFIG_AFT_TE1,	xilinx_if_conftab },
	{ WANCONFIG_AFT_TE3,    xilinx_if_conftab },
	{ WANCONFIG_AFT_ANALOG, xilinx_if_conftab },
	{ WANCONFIG_AFT_56K,	xilinx_if_conftab },
	{ WANCONFIG_ASYHDLC,	chdlc_conftab	},
	{ 0,			NULL		}
};

look_up_t conf_annexg_def_tables[] =
{
	{ ANNEXG_LAPB,		lapb_annexg_conftab	},
	{ ANNEXG_LIP_LAPB,	lapb_annexg_conftab	},
	{ ANNEXG_LIP_XDLC,	xdlc_conftab	},
	{ ANNEXG_LIP_TTY,	tty_conftab	},
	{ ANNEXG_LIP_XMTP2,	xmtp2_conftab	},
	{ ANNEXG_LIP_LAPD,	lapb_annexg_conftab	},
	{ ANNEXG_X25,		x25_annexg_conftab	},
	{ ANNEXG_DSP,		dsp_annexg_conftab	},
	{ ANNEXG_FR,		fr_conftab	},
	{ ANNEXG_CHDLC,		sppp_conftab	},
	{ ANNEXG_PPP,		sppp_conftab	},
	{ ANNEXG_LIP_X25,	x25_annexg_conftab	},
	{ ANNEXG_LIP_ATM,	atm_if_conftab	},
	{ ANNEXG_LIP_KATM,	atm_if_conftab	},
	{ 0,			NULL		}
};

look_up_t	config_id_str[] =
{
	{ WANCONFIG_PPP,	"WAN_PPP"	},
	{ WANCONFIG_FR,		"WAN_FR"	},
	{ WANCONFIG_X25,	"WAN_X25"	},
	{ WANCONFIG_CHDLC,	"WAN_CHDLC"	},
	{ WANCONFIG_ASYHDLC,	"WAN_ASYHDLC"	},
        { WANCONFIG_BSC,        "WAN_BSC"       },
        { WANCONFIG_HDLC,       "WAN_HDLC"      },
	{ WANCONFIG_MPPP,       "WAN_MULTPPP"   },
	{ WANCONFIG_MPROT,      "WAN_MULTPROT"  },
	{ WANCONFIG_LIP_ATM,    "WAN_LIP_ATM"   },
	{ WANCONFIG_LIP_KATM,   "WAN_LIP_KATM"   },
	{ WANCONFIG_BITSTRM, 	"WAN_BITSTRM"	},
	{ WANCONFIG_EDUKIT, 	"WAN_EDU_KIT"	},
	{ WANCONFIG_SS7,        "WAN_SS7"       },
	{ WANCONFIG_BSCSTRM,    "WAN_BSCSTRM"   },
	{ WANCONFIG_ADSL,	"WAN_ADSL"	},
	{ WANCONFIG_ADSL,	"WAN_ETH"	},
	{ WANCONFIG_SDLC,	"WAN_SDLC"	},
	{ WANCONFIG_ATM,	"WAN_ATM"	},
	{ WANCONFIG_POS,	"WAN_POS"	},
	{ WANCONFIG_AFT,	"WAN_AFT"	},
	{ WANCONFIG_AFT_TE1,	"WAN_AFT_TE1"	},
	{ WANCONFIG_AFT_ANALOG,	"WAN_AFT_ANALOG" },
	{ WANCONFIG_AFT_TE3,	"WAN_AFT_TE3"	},
	{ WANCONFIG_AFT_56K,	"WAN_AFT_56K"	},
	{ WANCONFIG_AFT,	"WAN_XILINX"	},
	{ WANCONFIG_MFR,    	"WAN_MFR"   	},
	{ WANCONFIG_DEBUG,    	"WAN_DEBUG"   	},
	{ WANCONFIG_ADCCP,    	"WAN_ADCCP"   	},
	{ WANCONFIG_MLINK_PPP, 	"WAN_MLINK_PPP" },
	{ 0,			NULL,		}
};

/*
 * Configuration options values and their symbolic names.
 */
look_up_t	sym_table[] =
{
	/*----- General -----------------------*/
	{ WANOPT_OFF,		"OFF"		}, 
	{ WANOPT_ON,		"ON"		}, 
	{ WANOPT_NO,		"NO"		}, 
	{ WANOPT_YES,		"YES"		}, 
	/*----- Interface type ----------------*/
	{ WANOPT_RS232,		"RS232"		},
	{ WANOPT_V35,		"V35"		},
	/*----- Data encoding -----------------*/
	{ WANOPT_NRZ,		"NRZ"		}, 
	{ WANOPT_NRZI,		"NRZI"		}, 
	{ WANOPT_FM0,		"FM0"		}, 
	{ WANOPT_FM1,		"FM1"		}, 

	/*----- Idle Line ----------------------*/
	{ WANOPT_IDLE_FLAG,	"FLAG"	}, 
	{ WANOPT_IDLE_MARK,	"MARK"	},

	/*----- Link type ---------------------*/
	{ WANOPT_POINTTOPOINT,	"POINTTOPOINT"	},
	{ WANOPT_MULTIDROP,	"MULTIDROP"	},
	/*----- Clocking ----------------------*/
	{ WANOPT_EXTERNAL,	"EXTERNAL"	}, 
	{ WANOPT_INTERNAL,	"INTERNAL"	}, 
	/*----- Station -----------------------*/
	{ WANOPT_DTE,		"DTE"		}, 
	{ WANOPT_DCE,		"DCE"		}, 
	{ WANOPT_CPE,		"CPE"		}, 
	{ WANOPT_NODE,		"NODE"		}, 
	{ WANOPT_SECONDARY,	"SECONDARY"	}, 
	{ WANOPT_PRIMARY,	"PRIMARY"	}, 
	/*----- Connection options ------------*/
	{ WANOPT_PERMANENT,	"PERMANENT"	}, 
	{ WANOPT_SWITCHED,	"SWITCHED"	}, 
	{ WANOPT_ONDEMAND,	"ONDEMAND"	}, 
	/*----- Frame relay in-channel signalling */
	{ WANOPT_FR_ANSI,	"ANSI"		}, 
	{ WANOPT_FR_Q933,	"Q933"		}, 
	{ WANOPT_FR_LMI,	"LMI"		}, 
	{ WANOPT_FR_NO_LMI,	"NOLMI"		}, 
	/*----- PPP IP Mode Options -----------*/
	{ WANOPT_PPP_STATIC,	"STATIC"	}, 
	{ WANOPT_PPP_HOST,	"HOST"		}, 
	{ WANOPT_PPP_PEER,	"PEER"		}, 
	/*----- CHDLC Protocol Options --------*/
/* DF for now	{ WANOPT_CHDLC_NO_DCD,	"IGNORE_DCD"	},
	{ WANOPT_CHDLC_NO_CTS,	"IGNORE_CTS"	}, 
	{ WANOPT_CHDLC_NO_KEEP,	"IGNORE_KEEPALIVE"}, 
*/
	{ WANOPT_PRI,           "PRI"           },
	{ WANOPT_SEC,           "SEC"           },
	
        /*------Read Mode ---------------------*/
        { WANOPT_INTR,          "INT"           },
        { WANOPT_POLL,          "POLL"          },
	/*------- Async Options ---------------*/
	{ WANOPT_ONE,           "ONE"          	},
	{ WANOPT_TWO,           "TWO"          	},
	{ WANOPT_ONE_AND_HALF,  "ONE_AND_HALF" 	},
	{ WANOPT_NONE,          "NONE"    	},
	{ WANOPT_EVEN,          "EVEN"    	},
	{ WANOPT_ODD,           "ODD"    	},

	{ WANOPT_TTY_SYNC,	"SYNC"		},
	{ WANOPT_TTY_ASYNC,     "ASYNC"		},

	
	{ WANOPT_FR_EEK_REQUEST, "EEK_REQ"	},
	{ WANOPT_FR_EEK_REPLY,	"EEK_REPLY"	}, 
                                                         


	/* TE1 T1/E1 defines */
        /*------TE Cofngiuration --------------*/
        { WAN_MEDIA_T1,      "T1"            },
        { WAN_MEDIA_E1,      "E1"            },
        { WAN_MEDIA_J1,      "J1"            },
	{ WAN_MEDIA_56K,     "56K"           },
	{ WAN_MEDIA_DS3,     "DS3"           },
	{ WAN_MEDIA_STS1,    "STS-1"         },
	{ WAN_MEDIA_E3,      "E3"            },
	{ WAN_MEDIA_FXOFXS,  "FXO/FXS"       },
        { WAN_LCODE_AMI, 	"AMI"           },
        { WAN_LCODE_B8ZS,       "B8ZS"          },
        { WAN_LCODE_HDB3,       "HDB3"          },
        { WAN_LCODE_B3ZS,       "B3ZS"          },
        { WAN_FR_D4,         "D4"            },
        { WAN_FR_ESF,        "ESF"           },
        { WAN_FR_NCRC4,      "NCRC4"         },
        { WAN_FR_CRC4,       "CRC4"          },
        { WAN_FR_UNFRAMED,   "UNFRAMED"      },
        { WAN_FR_E3_G751,    "G.751"         },
        { WAN_FR_E3_G832,    "G.832"         },
        { WAN_FR_DS3_Cbit,   "C-BIT"  	},
        { WAN_FR_DS3_M13,    "M13"           },
        { WAN_T1_LBO_0_DB,  	"0DB"           },
        { WAN_T1_LBO_75_DB,  "7.5DB"         },
        { WAN_T1_LBO_15_DB,  "15DB"          },
        { WAN_T1_LBO_225_DB, "22.5DB"        },
        { WAN_T1_0_110,  	"0-110FT"       },
        { WAN_T1_110_220,  	"110-220FT"     },
        { WAN_T1_220_330,  	"220-330FT"     },
        { WAN_T1_330_440,  	"330-440FT"     },
        { WAN_T1_440_550,  	"440-550FT"     },
        { WAN_T1_550_660,  	"550-660FT"     },
        { WAN_T1_0_133,  	"0-133FT"       },
        { WAN_T1_133_266,  	"133-266FT"     },
        { WAN_T1_266_399,  	"266-399FT"     },
        { WAN_T1_399_533,  	"399-533FT"     },
        { WAN_T1_533_655,  	"533-655FT"     },
        { WAN_E1_120,  		"120OH"     	},
        { WAN_E1_75,  		"75OH"     	},
        { WAN_NORMAL_CLK,   	"NORMAL"        },
        { WAN_MASTER_CLK,   	"MASTER"        },
	{ WANOPT_FE_OSC_CLOCK, 	"OSC"     	},
        { WANOPT_FE_LINE_CLOCK, "LINE"          },
        { WAN_TE1_SIG_CAS,	"CAS"           },
        { WAN_TE1_SIG_CCS,	"CCS"		},

	/* T3/E3 configuration */
        { WAN_TE3_RDEVICE_ADTRAN,	"ADTRAN"        },
        { WAN_TE3_RDEVICE_DIGITALLINK,	"DIGITAL-LINK"  },
        { WAN_TE3_RDEVICE_KENTROX,	"KENTROX"       },
        { WAN_TE3_RDEVICE_LARSCOM,	"LARSCOM"       },
        { WAN_TE3_RDEVICE_VERILINK,	"VERILINK"      },
        { WAN_TE3_LIU_LB_NORMAL,	"LB_NORMAL"     },
        { WAN_TE3_LIU_LB_ANALOG,	"LB_ANALOG"     },
        { WAN_TE3_LIU_LB_REMOTE,	"LB_REMOTE"     },
        { WAN_TE3_LIU_LB_DIGITAL,	"LB_DIGITAL"    },
	
	{ WANCONFIG_PPP,	"MP_PPP"	}, 
	{ WANCONFIG_CHDLC,	"MP_CHDLC"	},
	{ WANCONFIG_FR, 	"MP_FR"		},
	{ WANCONFIG_LAPB, 	"MP_LAPB"	},
	{ WANCONFIG_XDLC, 	"MP_XDLC"	},
	{ WANCONFIG_TTY, 	"MP_TTY"	},
	{ WANCONFIG_X25, 	"MP_X25"	},
	{ WANCONFIG_LIP_ATM, 	"MP_ATM"	},
	{ WANCONFIG_LIP_KATM, 	"MP_KATM"	},
	{ WANCONFIG_XMTP2, 	"MP_XMTP2"	},
	{ WANCONFIG_LAPD, 	"MP_LAPD"	},
	{ WANOPT_NO,		"RAW"		},
	{ WANOPT_NO,		"HDLC"		},
	{ WANCONFIG_PPP,	"PPP"		}, 
	{ WANCONFIG_CHDLC,	"CHDLC"		},
	{ WANCONFIG_ASYHDLC,	"ASYHDLC"	},
	
	/*-------------SS7 options--------------*/
	{ WANOPT_SS7_ANSI,      "ANSI"          },
	{ WANOPT_SS7_ITU,       "ITU"           },
	{ WANOPT_SS7_NTT,       "NTT"           },

	{ WANOPT_SS7_FISU,	"FISU"		},
	{ WANOPT_SS7_LSSU,	"LSSU"		},

	{ WANOPT_SS7_MODE_128, 	"SS7_128"	},
	{ WANOPT_SS7_MODE_4096,	"SS7_4096"	},
	
	{ WANOPT_S50X,		"S50X"		},
	{ WANOPT_S51X,		"S51X"		},
	{ WANOPT_ADSL,		"ADSL"		},
	{ WANOPT_ADSL,		"S518"		},
	{ WANOPT_AFT,           "AFT"           },

	/*-------------ADSL options--------------*/
	{ RFC_MODE_BRIDGED_ETH_LLC,	"ETH_LLC_OA" },
	{ RFC_MODE_BRIDGED_ETH_VC,	"ETH_VC_OA"  },
	{ RFC_MODE_ROUTED_IP_LLC,	"IP_LLC_OA"  },//ok
	{ RFC_MODE_ROUTED_IP_VC,	"IP_VC_OA"   },
	{ RFC_MODE_PPP_LLC,		"PPP_LLC_OA" },
	{ RFC_MODE_PPP_VC,		"PPP_VC_OA"  },
	{ RFC_MODE_STACK_VC,		"STACK_VC_OA"  },
	
	/* Backward compatible */
	{ RFC_MODE_BRIDGED_ETH_LLC,	"BLLC_OA" },
	{ RFC_MODE_ROUTED_IP_LLC,	"CIP_OA" },
	{ LAN_INTERFACE,		"LAN" },
	{ WAN_INTERFACE,		"WAN" },

        { WANOPT_ADSL_T1_413,                   "ADSL_T1_413"                   },
        { WANOPT_ADSL_G_LITE,                   "ADSL_G_LITE"                   },
	{ WANOPT_ADSL_G_DMT,                    "ADSL_G_DMT"                    },
	{ WANOPT_ADSL_ALCATEL_1_4,              "ADSL_ALCATEL_1_4"              },
	{ WANOPT_ADSL_ALCATEL,                  "ADSL_ALCATEL"                  },
	{ WANOPT_ADSL_MULTIMODE,                "ADSL_MULTIMODE"                },
	{ WANOPT_ADSL_T1_413_AUTO,              "ADSL_T1_413_AUTO"              },
	{ WANOPT_ADSL_TRELLIS_ENABLE,           "ADSL_TRELLIS_ENABLE"           },
	{ WANOPT_ADSL_TRELLIS_DISABLE,          "ADSL_TRELLIS_DISABLE"          },
	{ WANOPT_ADSL_TRELLIS_LITE_ONLY_DISABLE,"ADSL_TRELLIS_LITE_ONLY_DISABLE"},
	{ WANOPT_ADSL_0DB_CODING_GAIN,          "ADSL_0DB_CODING_GAIN"          },
	{ WANOPT_ADSL_1DB_CODING_GAIN,          "ADSL_1DB_CODING_GAIN"          },
	{ WANOPT_ADSL_2DB_CODING_GAIN,          "ADSL_2DB_CODING_GAIN"          },
	{ WANOPT_ADSL_3DB_CODING_GAIN,          "ADSL_3DB_CODING_GAIN"          },
	{ WANOPT_ADSL_4DB_CODING_GAIN,          "ADSL_4DB_CODING_GAIN"          },
	{ WANOPT_ADSL_5DB_CODING_GAIN,          "ADSL_5DB_CODING_GAIN"          },
	{ WANOPT_ADSL_6DB_CODING_GAIN,          "ADSL_6DB_CODING_GAIN"          },
	{ WANOPT_ADSL_7DB_CODING_GAIN,          "ADSL_7DB_CODING_GAIN"          },
	{ WANOPT_ADSL_AUTO_CODING_GAIN,         "ADSL_AUTO_CODING_GAIN"         },
	{ WANOPT_ADSL_RX_BIN_DISABLE,           "ADSL_RX_BIN_DISABLE"           },
	{ WANOPT_ADSL_RX_BIN_ENABLE,            "ADSL_RX_BIN_ENABLE"            },
	{ WANOPT_ADSL_FRAMING_TYPE_0,           "ADSL_FRAMING_TYPE_0"           },
        { WANOPT_ADSL_FRAMING_TYPE_1,           "ADSL_FRAMING_TYPE_1"           },
	{ WANOPT_ADSL_FRAMING_TYPE_2,           "ADSL_FRAMING_TYPE_2"           },
	{ WANOPT_ADSL_FRAMING_TYPE_3,           "ADSL_FRAMING_TYPE_3"           },
	{ WANOPT_ADSL_EXPANDED_EXCHANGE,        "ADSL_EXPANDED_EXCHANGE"        },
	{ WANOPT_ADSL_SHORT_EXCHANGE,           "ADSL_SHORT_EXCHANGE"           },
	{ WANOPT_ADSL_CLOCK_CRYSTAL,            "ADSL_CLOCK_CRYSTAL"            },
	{ WANOPT_ADSL_CLOCK_OSCILLATOR,         "ADSL_CLOCK_OSCILLATOR"         },

	{ IBM4680,	"IBM4680" },
	{ IBM4680,	"IBM4680" },
  	{ NCR2126,	"NCR2126" },
	{ NCR2127,	"NCR2127" },
	{ NCR1255,	"NCR1255" },
	{ NCR7000,	"NCR7000" },
	{ ICL,		"ICL" },

	
	// DSP_20
	/*------- DSP Options -----------------*/
	{ WANOPT_DSP_HPAD,      "HPAD"         	},
	{ WANOPT_DSP_TPAD,      "TPAD"         	},

	{ WANOPT_AUTO,  "AUTO" },
	{ WANOPT_MANUAL,"MANUAL" },

	/* XDLC Options -----------------------*/
	{ WANOPT_TWO_WAY_ALTERNATE, "TWO_WAY_ALTERNATE" },
	{ WANOPT_TWO_WAY_SIMULTANEOUS, "TWO_WAY_SIMULTANEOUS" },
	{ WANOPT_PRI_DISC_ON_NO_RESP,     "PRI_DISC_ON_NO_RESP" },
	{ WANOPT_PRI_SNRM_ON_NO_RESP,	"PRI_SNRM_ON_NO_RESP" },

	{ WP_NONE,	"WP_NONE" },
	{ WP_SLINEAR,   "WP_SLINEAR" },
		
	{ WAN_TDMV_ALAW,	"ALAW" },
	{ WAN_TDMV_MULAW,	"MULAW" },
		
	/*----- End ---------------------------*/
	{ 0,			NULL		}, 
};

void init_first_time_tokens(char **token)
{
	int i;

	for (i=0;i<MAX_TOKENS;i++){
		token[i]=NULL;
	}
}

/****** Entry Point *********************************************************/

int main (int argc, char *argv[])
{
	int err = 0;	/* return code */
	int c;

	if (WANPIPE_VERSION_BETA){
		sprintf(wan_version,"Beta %s.%s",
				WANPIPE_VERSION, WANPIPE_SUB_VERSION);
	}else{
		sprintf(wan_version,"Stable %s.%s",
				WANPIPE_VERSION, WANPIPE_SUB_VERSION);
	}


	/* Process command line switches */
	action = DO_SHOW_STATUS;
	while(1) {
		c = getopt(argc, argv, "D:hvwUf:a:y:zd:y:z;V;x:r:");

		if( c == -1)
			break;

		switch(c) {
	

		case 'x':
			return start_daemon();
			
		case 'd':
			conf_file = optarg;
			set_action(DO_STOP);
			break;

		case 'a':
			adsl_file = optarg;
			break;

		case 'f':
			conf_file = optarg;
			set_action(DO_START);
			break;

		case 'v':
			verbose = 1;
			break;
		
		case 'w':
			weanie_flag = 1;
			break;
			
		case 'U':
			set_action(DO_ARG_CONFIG);
			break;
				
		case 'y':
			verbose_log = optarg;
			break;
			
		case 'z':
			krnl_log_file = optarg;
			break;
			
		case 'h':
			show_help();
			break;

		case 'D':
			dev_name = optarg;
			set_action(DO_DEBUGGING);
			break;

		case 'r':
			dev_name = optarg;
			set_action(DO_DEBUG_READ);
			break;
			
		case 'V':

			if (WANPIPE_VERSION_BETA){
				printf("wanconfig: Beta%s-%s %s %s\n",
					WANPIPE_SUB_VERSION, WANPIPE_VERSION,
					WANPIPE_COPYRIGHT_DATES,WANPIPE_COMPANY);
			}else{
				printf("wanconfig: Stable %s-%s %s %s\n",
					WANPIPE_VERSION, WANPIPE_SUB_VERSION,
					WANPIPE_COPYRIGHT_DATES,WANPIPE_COMPANY);
			}

			printf("\n");

			show_usage();
			break;

		case '?':
		case ':':
		default:
			show_usage();
			break;
		}	
	}
	
	/* Check that the no action is given with non-swtiched arguments 
	 */
	
	if( action != DO_SHOW_STATUS && action != DO_ARG_CONFIG ) {
		if(optind < argc)
			show_usage();
		
	}

	/* Process interface control command arguments */
	if( action == DO_SHOW_STATUS && optind  < argc ) {
		
		for( ; optind < argc; optind++ ){
			
			if( strcmp( argv[optind], "wan" ) == 0 \
			 	|| strcmp( argv[optind], "card") == 0 ) {
				
				/* Check that card name arg is given */
				if((optind+1) >= argc ) show_usage();
			
				card_name = argv[optind+1];
				optind++;
			}
			
			else if( strcmp( argv[optind], "dev" ) == 0 ) {
				
				/* Make sure nodev argument is not given */
				if(nodev_flag) show_usage();
				
				/* Make sure card argument is given */
				if(card_name == NULL) show_usage();
				
				/* Check that interface name arg is given */
				if((optind+1) >= argc ) show_usage(); 

				dev_name = argv[optind+1];
				optind++;
			}
			
			else if( strcmp( argv[optind], "nodev" ) == 0 ) {
				
				/* Make sure dev argument is not given */
				if(dev_name != NULL) show_usage();
				
				nodev_flag = 1;
			}
			
			else if( strcmp( argv[optind], "start" ) == 0 
				|| strcmp( argv[optind], "up") == 0
				|| strcmp( argv[optind], "add") == 0 ) {
				set_action(DO_START);
			}
			
			else if( strcmp( argv[optind], "stop" ) == 0
				|| strcmp( argv[optind], "del" ) == 0	
				|| strcmp( argv[optind], "delete" ) == 0	
				|| strcmp( argv[optind], "down") == 0 ) {
				set_action(DO_STOP);
			}
			
			else if( strcmp( argv[optind], "restart" ) == 0 
				|| strcmp( argv[optind], "reload") == 0 ) {
				set_action(DO_RESTART);
			}
			
			else if( strcmp( argv[optind], "status" ) == 0
				|| strcmp( argv[optind], "stat" ) == 0
				|| strcmp( argv[optind], "show" ) == 0 ) {
				set_action(DO_SHOW_STATUS);
			}
			
			else if( strcmp( argv[optind], "config" ) == 0 ) {
				set_action(DO_SHOW_CONFIG);
			}
		
			else if( strcmp( argv[optind], "hwprobe" ) == 0 ) {
				set_action(DO_SHOW_HWPROBE);
			}
			
			else if( strcmp( argv[optind], "help" ) == 0 ) {
				show_help();
			}
			
			else {
				show_usage();
			}
			
		}
	}

	/* Check length of names */
	if( card_name != NULL && strlen(card_name) > WAN_DRVNAME_SZ) {
	       	fprintf(stderr, "%s: WAN device names must be %d characters long or less\n",
			prognamed, WAN_DRVNAME_SZ);
		exit(1);
	}
		
	if( dev_name != NULL && strlen(dev_name) > WAN_IFNAME_SZ) {
	       	fprintf(stderr, "%s: Interface names must be %d characters long or less\n",
			prognamed, WAN_IFNAME_SZ);
		exit(1);
	}
 		
	/* Perform requested action */
	if (verbose) puts(banner);
	if (!err)
	switch (action) {

	case DO_START:
		err = startup();
		break;

	case DO_STOP:
		err = wp_shutdown();
		break;

	case DO_RESTART:
		err = wp_shutdown();
		if(err)
			break;
		sleep(SLEEP_TIME);
		err = startup();
		break;
		
	case DO_ARG_CONFIG:
		conf_file = "wanpipe_temp.conf";
		argc = argc - optind;
		argv = &argv[optind];
		err = get_config_data(argc,argv);
		if (err)
			break;
		err = configure();
		remove(conf_file);
		break;

	case DO_ARG_DOWN:
		printf("DO_ARG_DOWN\n");
		wp_shutdown();
		break;

	case DO_HELP:
		show_help();
		break;
	
	case DO_SHOW_STATUS:
		return show_status();
		break;

	case DO_SHOW_CONFIG:
		return show_config();
		break;

	case DO_SHOW_HWPROBE:
		return show_hwprobe();
		break;

	case DO_DEBUGGING:
		return debugging();
		break;

	case DO_DEBUG_READ:
		return debug_read();
		break;

	default:
		err = ERR_SYNTAX;
		break;
	}

	
	/* Always a good idea to sleep a bit - easier on system and link
	 * we could be operating 2 cards and 1st one improperly configured
	 * so dwell a little to make things easier before wanconfig reinvoked.
	 */
	if( !err || err == ERR_SYSTEM ) usleep(SLEEP_TIME);
	return err;
}

/*============================================================================
 * Show help text
 */
void show_help(void) {
	puts(helptext);
	exit(1);
}

/*============================================================================
 * Show usage text
 */
void show_usage(void) {
	fprintf(stderr, usagetext);
	exit(1);
}

/*============================================================================
 * set action
 */
void set_action(int new_action) {
	if(action == DO_SHOW_STATUS ) 
		action = new_action;
	else{
		show_usage();
	}
}

/*============================================================================
 * Show error message.
 */
void show_error_dbg (int err, int line)
{
	switch(err){
	case ERR_SYSTEM: 
		fprintf(stderr, "%s:%d: SYSTEM ERROR %d: %s!\n",
	 		prognamed, line, errno, strerror(errno));
		break;
	case ERR_MODULE:
		fprintf(stderr, "%s:%d: ERROR: %s!\n",
			prognamed, line, err_messages[min(err, ERR_LIMIT) - 2]);
		break;
	default:
		fprintf(stderr, "%s:%d: ERROR: %s : %s!\n", prognamed,
			line, err_messages[min(err, ERR_LIMIT) - 2], conf_file);
	}
}

/*============================================================================
 * cat a given  file to stdout - this is used for status and config
 */
int gencat (char *filename) 
{
	FILE *file;
	char buf[256];
	
	file = fopen(filename, "r");
	if( file == NULL ) {
		fprintf( stderr, "%s: cannot open %s\n",
				prognamed, filename);
		show_error(ERR_SYSTEM);
		exit(ERR_SYSTEM);
	}
	
	while(fgets(buf, sizeof(buf) -1, file)) {
		printf(buf);
	}	

	fclose(file);
	return(0);
}

/*============================================================================
 * Start up - This is a shell function to select the configuration source
 */
int startup(void)
{
	char buf[256];
	int err = 0;
	int res = 0;
        int found = 0;	
	DIR *dir;
	struct dirent *dentry;
	struct stat file_stat;

	if(conf_file != NULL ) {
		/* Method I. Observe given config file name
		 */
		return(configure());
		
	}else{
     		/* Method II. Scan /proc/net/wanrouter for device names
     		 */
		
     		/* Open directory */
                dir = opendir(router_dir);

                if(dir == NULL) {
	        	err = ERR_SYSTEM;
	                show_error(err);
	                return(err);
	        }

		while( (dentry = readdir(dir)) ) {

			if( strlen(dentry->d_name) > WAN_DRVNAME_SZ)
				continue;

			/* skip directory dots */
			if( strcmp(dentry->d_name, ".") == 0 \
				|| strcmp(dentry->d_name, "..") == 0)
				continue;

			/* skip /prroc/net/wanrouter/{status,config} */
			if( strcmp(dentry->d_name, "status") == 0 \
				|| strcmp(dentry->d_name, "config") == 0)
				continue;
			
			/* skip interfaces we are not configuring */
			if( card_name != NULL && strcmp(dentry->d_name, card_name) != 0 )
				continue;
			
			snprintf(buf, sizeof(buf), "%s/%s.conf", conf_dir, dentry->d_name);
			
			/* Stat config file to see if it is there */
			res = stat(buf, &file_stat);
			if( res < 0 ) {
				if( errno == ENOENT )
					continue;
				
				show_error(ERR_SYSTEM);
				closedir(dir);
				exit(ERR_SYSTEM);
			}			

			found = 1;
			conf_file = buf;	
			err = configure();
		}
		closedir(dir);
	}

	if (!found){
		/* Method III. Use default configuration file */
		conf_file = def_conf_file;
		err = configure();
	}
	
	return(err);
}

/*============================================================================
* Shut down link.
*/
int wp_shutdown (void)
{
	int err = 0;
	DIR *dir;
	struct dirent *dentry;

	if (conf_file){
		err = conf_file_down();

	}else if( card_name != NULL && dev_name == NULL ) {
		err = router_down(card_name, 0);

	}else if( card_name != NULL && dev_name != NULL ) {
		err = router_ifdel(card_name, dev_name);
	
	}else{
		/* Open directory */
		dir = opendir(router_dir);

		if(dir == NULL) {
			err = ERR_SYSTEM;
			show_error(err);
			return(err);
		}

		while( (dentry = readdir(dir)) ) {
			int res = 0;
			
			if( strlen(dentry->d_name) > WAN_DRVNAME_SZ)
				continue;

			if( strcmp(dentry->d_name, ".") == 0 \
				|| strcmp(dentry->d_name, "..") == 0)
				continue;

			res = router_down(dentry->d_name, 1);	
			if(res){ 
				err = res;
			}
		}
        	
		closedir(dir);
	}

	return err;
}


int router_down (char *devname, int ignore_error)
{
	char filename[sizeof(router_dir) + WAN_DRVNAME_SZ + 2];
        int dev, err=0;
	link_def_t def;

	def.linkconf = malloc(sizeof(wandev_conf_t));
	if (!def.linkconf){
		printf("%s: Error failed to allocated memory on router down\n", 
				devname);
		return -ENOMEM;
	}

	if (verbose)
        	printf(" * Shutting down WAN device %s ...\n", devname);                
	
	if (strlen(devname) > WAN_DRVNAME_SZ) {
                show_error(ERR_SYNTAX);
                return -EINVAL;
	}

#if defined(WAN_HWEC)
	//ADBG : We don't know if wanpipe has HWEC enabled or not....
	//release_hwec(devname, NULL);
#endif

#if defined(__LINUX__)
        snprintf(filename, sizeof(filename), "%s/%s", router_dir, devname);
#else
        snprintf(filename, sizeof(filename), "%s", WANDEV_NAME);
#endif

	dev = open(filename, O_RDONLY);
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	strcpy(config.devname, &devname[0]);
    	def.linkconf->magic = ROUTER_MAGIC;
	config.arg = (void*)def.linkconf;
        if ((dev < 0) || (ioctl(dev, ROUTER_DOWN, &config) < 0 
				&& ( !ignore_error || errno != ENOTTY))) {
#else
        if ((dev < 0) || (ioctl(dev, ROUTER_DOWN, def.linkconf) < 0 
				&& ( !ignore_error || errno != ENOTTY))) {
#endif
		err = ERR_SYSTEM;
		fprintf(stderr, "\n\n\t%s: WAN device %s did not shutdown\n", prognamed, devname);
		fprintf(stderr, "\t%s: ioctl(%s,ROUTER_DOWN) failed:\n", 
				progname_sp, devname);
		fprintf(stderr, "\t%s:\t%d - %s\n", progname_sp, errno, strerror(errno));
		if( weanie_flag ) {
			fprintf(stderr, "\n");
			fprintf(stderr, "\n\tIf you router was not running ignore this message\n !!");
			fprintf(stderr, "\tOtherwise, check the %s and \n", verbose_log);
			fprintf(stderr, "\t%s for errors\n", krnl_log_file);
		}
 		fprintf( stderr, "\n" );
 		if(dev >=0){
			close (dev);
		}
 		return errno;
       	}
 

	if (def.linkconf->config_id == WANCONFIG_ADSL){
		update_adsl_vci_vpi_list(&def.linkconf->u.adsl.vcivpi_list[0], 	
					 def.linkconf->u.adsl.vcivpi_num);
	}

	free(def.linkconf);
	def.linkconf=NULL;

        if (dev >= 0){ 
		close(dev);
	}
	return 0;

}


int router_ifdel (char *card_name, char *dev_name)
{
	char filename[sizeof(router_dir) + WAN_DRVNAME_SZ + 2];
	char devicename[WAN_IFNAME_SZ +2];
	int dev, err=0;

	if (verbose)
        	printf(" * Shutting down WAN device %s ...\n", card_name);                

	if (strlen(card_name) > WAN_DRVNAME_SZ) {
                show_error(ERR_SYNTAX);
                return -EINVAL;
	}

#if defined(__LINUX__)
 	snprintf(filename, sizeof(filename), "%s/%s", router_dir, card_name);
#else
        snprintf(filename, sizeof(filename), "%s", WANDEV_NAME);
#endif
	if (strlen(dev_name) > WAN_IFNAME_SZ) {
		show_error(ERR_SYNTAX);
		return -EINVAL;
	}

#if defined(WAN_HWEC)
	//ADBG : We don't know if wanpipe has HWEC enabled or not....
	//release_hwec(card_name, dev_name);
#endif

#if defined(__LINUX__)
	snprintf(devicename, sizeof(devicename), "%s", dev_name);
#else
	strcpy(config.devname, dev_name);
    	config.arg = NULL;
#endif
	
        dev = open(filename, O_RDONLY);

#if defined(__LINUX__)
        if ((dev < 0) || (ioctl(dev, ROUTER_IFDEL, devicename) < 0)) {
#else
        if ((dev < 0) || (ioctl(dev, ROUTER_IFDEL, &config) < 0)) {
#endif
		err = errno;
		fprintf(stderr, "\n\n\t%s: Interface %s not destroyed\n", prognamed, devicename);
		fprintf(stderr, "\t%s: ioctl(%s,ROUTER_IFDEL,%s) failed:\n", 
				progname_sp, card_name, devicename);
		fprintf(stderr, "\t%s:\t%d - %s\n", progname_sp, errno, strerror(errno));
		if( weanie_flag ) {
			fprintf(stderr, "\n");
			fprintf(stderr, "\n\tIf you router was not running ignore this message\n !!");
			fprintf(stderr, "\tOtherwise, check the %s and \n", verbose_log);
			fprintf(stderr, "\t%s for errors\n", krnl_log_file);
		}
 		fprintf( stderr, "\n" );
 		if(dev >=0){ 
			close (dev);
		}
                return errno;
       	}


        if (dev >= 0){
		close(dev);
	}
	return 0;

}

/*============================================================================
 * Configure router.
 * o parse configuration file
 * o configure links
 * o configure logical channels (interfaces)
 */
int configure (void)
{
	int err = 0;
	int res;

	/* Parse configuration file */
	if (verbose)
		printf(" * Parsing configuration file %s ...\n", conf_file);

	err = parse_conf_file(conf_file);
	if (err) return err;

	if (link_defs == NULL) {
		if (verbose) printf(" * No link definitions found...\n");
		return 0;
	}

	if( card_name != NULL && strcmp(card_name, link_defs->name) != 0 )
		return ERR_CONFIG;

	if (verbose) printf(
		" * Configuring device %s (%s)\n",
		link_defs->name,
		"no description");
	res = configure_link(link_defs,0);
	if (res) {
		return res;
	}

	free_linkdefs();

	return err;
}

/*============================================================================
 * Parse configuration file.
 *	Read configuration file and create lists of link and channel
 *	definitions.
 */
int parse_conf_file (char* fname)
{
	int err = 0;
	FILE* file;

	file = fopen(fname, "r");
	if (file == NULL) {
		fprintf(stderr, "\nError: %s not found in %s directory\n",
				fname, conf_dir);
		show_error(ERR_SYSTEM);
		return ERR_SYSTEM;
	}

	/* Build a list of link and channel definitions */
	err = build_linkdef_list(file);
	if (!err && link_defs)
		err = build_chandef_list(file);

	fclose(file);
	return err;
}

/*============================================================================
 * Build a list of link definitions.
 */
int build_linkdef_list (FILE* file)
{
	int err = 0;
	char* conf;		/* -> section buffer */
	char* key;		/* -> current configuration record */
	int len;		/* record length */
	char *token[MAX_TOKENS];


	/* Read [devices] section */
	conf = read_conf_section(file, "devices");
	if (conf == NULL) return ERR_CONFIG;

	/* For each record in [devices] section create a link definition
	 * structure and link it to the list of link definitions.
	 */
	for (key = conf; !err && *key; key += len) {
		int toknum;
		
		link_def_t* linkdef;	/* -> link definition */
		int config_id = 0;

		len = strlen(key) + 1;
		toknum = tokenize(key, token);
		if (toknum < 2) continue;

		strupcase(token[1]);
		config_id = name2val(token[1], config_id_str);
		if (!config_id) {
			if (verbose) printf(
				" * Media ID %s is invalid!\n", token[1]);
			err = ERR_CONFIG;
			show_error(err);
			break;
		}
		linkdef = calloc(1, sizeof(link_def_t));
		if (linkdef == NULL) {
			err = ERR_SYSTEM;
			show_error(err);
			break;
		}

		memset(linkdef,0,sizeof(link_def_t));

	 	strncpy(linkdef->name, token[0], WAN_DRVNAME_SZ);
		linkdef->config_id = config_id;
	if ((toknum > 2) && token[2])
		linkdef->descr = strdup(token[2]);

		linkdef->conf = read_conf_section(file, linkdef->name);
                if (link_defs) {
			linkdef->next=link_defs;
			link_defs=linkdef;
			//link_def_t* last;
			//for (last = link_defs; last->next; last = last->next);
			//last->next = linkdef;
		}
		else link_defs = linkdef;
	}
	free(conf);
	return err;
}

/*============================================================================
 * Build a list of channel definitions.
 */
int build_chandef_list (FILE* file)
{
	int err = 0;
	char* conf;		/* -> section buffer */
	char* key;		/* -> current configuration record */
	int len;		/* record length */
	char *token[MAX_TOKENS];

	link_def_t* linkdef = link_defs;

	if (linkdef == NULL)
		return ERR_CONFIG;


	/* Read [interfaces] section */
	conf = read_conf_section(file, "interfaces");
	if (conf == NULL) 
		return 0;

	/* For each record in [interfaces] section create channel definition
	 * structure and link it to the list of channel definitions for
	 * appropriate link.
	 */
	for (key = conf; !err && *key; key += len) {
		int toknum;
		char *channel_sect;
		
		chan_def_t* chandef;		/* -> channel definition */

		len = strlen(key) + 1;

		toknum = tokenize(key, token);
		if (toknum < 4) continue;

		/* allocate and initialize channel definition structure */
		chandef = calloc(1, sizeof(chan_def_t));
		if (chandef == NULL) {
			err = ERR_SYSTEM;
			show_error(err);
			break;
		}
		memset(chandef,0,sizeof(chan_def_t));

	 	strncpy(chandef->name, token[0], WAN_IFNAME_SZ);
		chandef->addr   = strdup(token[2]);
		chandef->usedby = strdup(token[3]);
		chandef->annexg = NO_ANNEXG;

		/* These are Anexg Connections */
		if (toknum > 5){

			if (!strcmp(token[4],"lapb")){
				chandef->annexg = ANNEXG_LAPB;
			}else if (!strcmp(token[4],"x25")){
				chandef->annexg = ANNEXG_X25;
			}else if (!strcmp(token[4],"dsp")){
				chandef->annexg = ANNEXG_DSP;
				
			}else if (!strcmp(token[4],"fr")){
				chandef->annexg = ANNEXG_FR;
				chandef->protocol = strdup("MP_FR");	
			}else if (!strcmp(token[4],"ppp")){
				chandef->annexg = ANNEXG_PPP;
				chandef->protocol = strdup("MP_PPP");
			}else if (!strcmp(token[4],"chdlc")){
				chandef->annexg = ANNEXG_CHDLC;	
				chandef->protocol = strdup("MP_CHDLC");

			}else if (!strcmp(token[4],"lip_atm")){
				printf("%s: CONIGURING FOR LIP ATM\n",chandef->name);
				chandef->annexg = ANNEXG_LIP_ATM;	
				chandef->protocol = strdup("MP_ATM");

			}else if (!strcmp(token[4],"lip_katm")){
				printf("%s: CONIGURING FOR LIP KATM\n",chandef->name);
				chandef->annexg = ANNEXG_LIP_KATM;	
				chandef->protocol = strdup("MP_KATM");

			}else if (!strcmp(token[4],"lip_x25")){
				chandef->annexg = ANNEXG_LIP_X25;	
				chandef->protocol = strdup("MP_X25");
			
			}else if (!strcmp(token[4],"lip_lapb")){
				chandef->annexg = ANNEXG_LIP_LAPB;
				chandef->protocol = strdup("MP_LAPB");
			}else if (!strcmp(token[4],"lip_xmtp2")){
				chandef->annexg = ANNEXG_LIP_XMTP2;
				chandef->protocol = strdup("MP_XMTP2");
			}else if (!strcmp(token[4],"lip_lapd")){
				chandef->annexg = ANNEXG_LIP_LAPD;
				chandef->protocol = strdup("MP_LAPD");
			}else if (!strcmp(token[4],"lip_xdlc")){
				chandef->annexg = ANNEXG_LIP_XDLC;
				chandef->protocol = strdup("MP_XDLC");
			}else if (!strcmp(token[4],"tty")){
				chandef->annexg = ANNEXG_LIP_XDLC;
				chandef->protocol = strdup("MP_TTY");
			}else{
				return ERR_CONFIG;
			}

		}
		
		if (!(chandef->usedby) ||(( strcmp(chandef->usedby, "WANPIPE")     != 0 ) &&
					  ( strcmp(chandef->usedby, "API")         != 0 ) && 
					  ( strcmp(chandef->usedby, "BRIDGE")      != 0 ) &&
					  ( strcmp(chandef->usedby, "SWITCH")      != 0 ) &&
					  ( strcmp(chandef->usedby, "PPPoE")       != 0 ) &&
					  ( strcmp(chandef->usedby, "STACK")       != 0 ) &&
					  ( strcmp(chandef->usedby, "TTY")       != 0 ) &&
					  ( strcmp(chandef->usedby, "BRIDGE_NODE") != 0 ) &&
					  ( strcmp(chandef->usedby, "TDM_VOICE") 	   != 0 ) &&
					  ( strcmp(chandef->usedby, "TDM_VOICE_API") 	   != 0 ) &&
					  ( strcmp(chandef->usedby, "TDM_API") 	   != 0 ) &&
					  ( strcmp(chandef->usedby, "TRUNK") 	   != 0 ) &&
					  ( strcmp(chandef->usedby, "ANNEXG")      != 0 )))
		{

			if (verbose) printf(" * %s to be used by an Invalid entity: %s\n", 
									chandef->name, chandef->usedby);
			return ERR_CONFIG;
		}
	
		if (verbose)
			printf(" * %s to used by %s\n",
				chandef->name, chandef->usedby);
		
		channel_sect=strdup(token[0]);
		
		if (toknum > 5){
			char *section=strdup(token[5]);
			/* X25_SW */
			if (toknum > 7){
				if (toknum > 6)
					chandef->virtual_addr = strdup(token[6]);
				if (toknum > 7)
					chandef->real_addr = strdup(token[7]);
				if (toknum > 8)
					chandef->label = strdup(token[8]);
			}else{
				if (toknum > 6)
					chandef->label = strdup(token[6]);
			}

			chandef->conf_profile = read_conf_section(file, section);
			free(section);
			
		}else{
			chandef->conf_profile = NULL;
		}
		
		
		chandef->conf = read_conf_section(file, channel_sect);	
		free(channel_sect);
		
		/* append channel definition structure to the list */
		if (linkdef->chan) {
			chan_def_t* last;

			for (last = linkdef->chan;
			     last->next;
			     last = last->next)
			;
			last->next = chandef;
		}
		else linkdef->chan = chandef;
		/* Alex */
		chandef->link = linkdef;

	}
	free(conf);
	return err;
}


/*============================================================================
 * Configure WAN link.
 */
int configure_link (link_def_t* def, char init)
{
	int err = 0;
	int len = 0;
	int i;
	key_word_t* conf_table = lookup(def->config_id, conf_def_tables);
	char filename[sizeof(router_dir) + WAN_DRVNAME_SZ + 2];
	chan_def_t* chandef;
	char* conf_rec;
	int dev=-1;
	unsigned short hi_DLCI = 15;
	char *token[MAX_TOKENS];

	if (def->conf == NULL) {
		show_error(ERR_CONFIG);
		return ERR_CONFIG;
	}

	/* Clear configuration structure */
	if (def->linkconf){
		if (def->linkconf->data) free(def->linkconf->data);
		free(def->linkconf);
		def->linkconf=NULL;
	}

	def->linkconf = malloc(sizeof(wandev_conf_t));
	if (!def->linkconf){
		printf("%s: Failed to allocated linkconf!\n",prognamed);
		return -ENOMEM;
	}
	memset(def->linkconf, 0, sizeof(wandev_conf_t));
	def->linkconf->magic = ROUTER_MAGIC;
	def->linkconf->config_id = def->config_id;

	/* Parse link configuration */
	for (conf_rec = def->conf; *conf_rec; conf_rec += len) {
		int toknum;

		len = strlen(conf_rec) + 1;
		toknum = tokenize(conf_rec, token);
		if (toknum < 2) {
			show_error(ERR_CONFIG);
			return ERR_CONFIG;
		}

		/* Look up a keyword first in common, then in media-specific
		 * configuration definition tables.
		 */
		strupcase(token[0]);
		err = set_conf_param(
			token[0], token[1], common_conftab, def->linkconf);
		if ((err < 0) && (conf_table != NULL))
			err = set_conf_param(
				token[0], token[1], conf_table, &def->linkconf->u);
		if (err < 0) {
			printf(" * Unknown parameter %s\n", token[0]);
			fprintf(stderr, "\n\n\tERROR in %s !!\n",conf_file);
			if(weanie_flag) fprintf(stderr, "\tPlease check %s for errors\n", verbose_log);
			fprintf(stderr, "\n");
			return ERR_CONFIG;
		}
		if (err){ 
			fprintf(stderr,"\n\tError parsing %s configuration file\n",conf_file);
			if(weanie_flag) fprintf(stderr,"\tPlease check %s for errors\n", verbose_log);
			fprintf(stderr, "\n");
			return err;
		}
	}

	/* Open SDLA device and perform link configuration */
	sprintf(filename, "%s/%s", router_dir, def->name);
	
	/* prepare a list of DLCI(s) and place it in the wandev_conf_t structure
	 * This is done so that we have a list of DLCI(s) available when we 
	 * call the wpf_init() routine in sdla_fr.c (triggered by the ROUTER_
	 * SETUP ioctl call) for running global SET_DLCI_CONFIGURATION cmd. This
	 * is only relevant for a station being a NODE, for CPE it polls the 
	 * NODE (auto config DLCI)
	 */
	
	if ((def->linkconf->config_id == WANCONFIG_FR)){
		i = 0;

		if(err)
			return err;
		
		for (chandef = def->chan; chandef; chandef = chandef->next){
		
			if (strcmp(chandef->addr,"auto")==0){
				continue;
			}

			if (verbose) 
				printf(" * Reading DLCI(s) Included : %s\n",
					chandef->addr ? chandef->addr : "not specified");

			if (chandef->annexg != NO_ANNEXG) 
				continue;

			def->linkconf->u.fr.dlci[i] = dec_to_uint(chandef->addr,0);
			if(def->linkconf->u.fr.dlci[i] > hi_DLCI) {
				hi_DLCI = def->linkconf->u.fr.dlci[i];
				i++;
			}else {
				fprintf(stderr,"\n\tDLCI(s) specified in %s are not in order\n",conf_file); 
				fprintf(stderr,"\n\tor duplicate DLCI(s) assigned!\n");
				return ERR_SYSTEM;
			}
		}
	}else{
		/* Read standard VCI/VPI list and send it */
		if ((def->linkconf->config_id == WANCONFIG_ADSL)){
			read_adsl_vci_vpi_list(&def->linkconf->u.adsl.vcivpi_list[0], 
					       &def->linkconf->u.adsl.vcivpi_num);
		}
	}

#if defined(__LINUX__)
	dev = open(filename, O_RDONLY);
#else
	dev = open(WANDEV_NAME, O_RDONLY);
#endif

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	strcpy(config.devname, def->name);
    	config.arg = NULL;
#endif

	if (conf_file && strstr(conf_file, "ft1.conf") != NULL){
		def->linkconf->ft1 = 1;
	}else{
		def->linkconf->ft1 = 0;
	}

 	if (!init){
		/* copy firmware onto S514 card */
		def->linkconf->data = firmware_file_buffer;/* must initialize it again here!!! */
		err=exec_link_cmd(dev,def);
 	}

	/* Configure logical channels */
	if( !err && !nodev_flag ){ 
		for (chandef = def->chan; chandef; chandef = chandef->next) {

			switch (chandef->annexg){
				case NO_ANNEXG:
					strncpy(master_lapb_dev,chandef->name, WAN_IFNAME_SZ);
					strncpy(master_lip_dev,chandef->name, WAN_IFNAME_SZ);
					break;
				case ANNEXG_LAPB:
					strncpy(master_x25_dev,chandef->name, WAN_IFNAME_SZ);
					break;
				case ANNEXG_X25:
					strncpy(master_dsp_dev,chandef->name, WAN_IFNAME_SZ);
					break;
			}

			if (!init){
				if(dev_name != NULL && strcmp(dev_name, chandef->name) != 0 )
					continue;
			}
			
			if (verbose) printf(
				" * Configuring channel %s (%s). Media address: %s\n",
				chandef->name,
				"no description",
				chandef->addr ? chandef->addr : "not specified");

			if (chandef->chanconf){
				free(chandef->chanconf);
				chandef->chanconf=NULL;
			}
			chandef->chanconf=malloc(sizeof(wanif_conf_t));
			if (!chandef->chanconf){
				printf("%s: Failed to allocate chandef\n",prognamed);
				close(dev);
				return -ENOMEM;
			}
			memset(chandef->chanconf, 0, sizeof(wanif_conf_t));
			chandef->chanconf->magic = ROUTER_MAGIC;
			chandef->chanconf->config_id = def->config_id;
			err=configure_chan(dev, chandef, init, def->config_id);
			if (err){
				break;
			}

		        if (strcmp(chandef->usedby, "STACK") == 0){
				strncpy(master_lip_dev,chandef->name, WAN_IFNAME_SZ);
			}
		}
	}
	/* clean up */
	close(dev);
	return err;
}

int exec_link_cmd(int dev, link_def_t *def)
{
	if (!def->linkconf){
		printf("%s: Error: Device %s has no config structure\n",
				prognamed,def->name);
		return -EFAULT;
	}

	{
		char wanpipe_version[50];
		memset(wanpipe_version,0,sizeof(wanpipe_version));

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
		config.arg = wanpipe_version;
		if ((dev < 0) || (ioctl(dev, ROUTER_VER, &config))){
#else
		if ((dev < 0) || (ioctl(dev, ROUTER_VER, wanpipe_version))){
#endif

			fprintf(stderr, "\n\n\t%s: WAN device %s driver load failed !!\n", 
							prognamed, def->name);
			fprintf(stderr, "\t%s: ioctl(%s,ROUTER_VER) failed:\n", 
				progname_sp, def->name);
			fprintf(stderr, "\t%s:\t%d - %s\n", progname_sp, errno, strerror(errno));


			fprintf(stderr, "\n");

			switch (errno){

			case EPERM:
				fprintf(stderr, "\n\t%s: Wanpipe Driver Security Check Failure!\n",
					progname_sp);
				fprintf(stderr, "\n\t%s:   Linux headers used to compile Wanpipe\n",
					progname_sp);
				fprintf(stderr, "\n\t%s:   Drivers, do not match the Kernel Image\n",
					progname_sp);
				fprintf(stderr, "\n\t%s:   Please call Sangoma Tech Support\n",
					progname_sp);
		
				break;

			default:
				fprintf(stderr, "\n\t%s: Wanpipe Module Installation Failure!\n",
					progname_sp);
				fprintf(stderr, "\n\t%s:   New Wanpipe modules failed to install during\n",
					progname_sp);
				fprintf(stderr, "\n\t%s:   wanpipe installation.  Current wanpipe modules\n",
					progname_sp);
				fprintf(stderr, "\n\t%s:   (lsmod) are obselete.\n",
					progname_sp);
				fprintf(stderr, "\n\t%s:   Please check linux source name vs the name\n",
					progname_sp);
				fprintf(stderr, "\n\t%s:   of the currently running image.\n",
					progname_sp);
				fprintf(stderr, "\n\t%s:   Then proceed to install Wanpipe again!\n",
					progname_sp);
				break;	
			}

			
			fprintf(stderr, "\n");
			return -EINVAL;

		}

		if (strstr(wanpipe_version, wan_version) == NULL){
			fprintf(stderr, "\n\n\t%s: WAN device %s driver load failed !!\n", 
							prognamed, def->name);
			fprintf(stderr, "\n\t%s: Wanpipe version mismatch between utilites\n",
					progname_sp);
			fprintf(stderr, "\t%s: and kernel drivers:\n",
					progname_sp);
			fprintf(stderr, "\t%s: \tUtil Ver=%s, Driver Ver=%s\n",
					progname_sp,wan_version,wanpipe_version);
		
			fprintf(stderr, "\n");
			return -EINVAL;
		}
	}

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	config.arg = def->linkconf;
	if ((dev < 0) || (ioctl(dev, ROUTER_SETUP, &config) < 0
				&& ( dev_name == NULL || errno != EBUSY ))) {
#else
	if ((dev < 0) || (ioctl(dev, ROUTER_SETUP, def->linkconf) < 0
						&& ( dev_name == NULL || errno != EBUSY ))) {
#endif
		fprintf(stderr, "\n\n\t%s: WAN device %s driver load failed !!\n", prognamed, def->name);
		fprintf(stderr, "\t%s: ioctl(%s,ROUTER_SETUP) failed:\n", 
				progname_sp, def->name);
		fprintf(stderr, "\t%s:\t%d - %s\n", progname_sp, errno, strerror(errno));
		if( weanie_flag ) {
			fprintf(stderr, "\n");
			fprintf(stderr, "\n\tWanpipe driver did not load properly\n");
			fprintf(stderr, "\tPlease check %s and\n", verbose_log);
			fprintf(stderr, "\t%s for errors\n", krnl_log_file); 
		}
		fprintf(stderr, "\n");
		return errno;
	}
	
	return 0;
}

/*============================================================================
 * Configure WAN logical channel.
 */
int configure_chan (int dev, chan_def_t* def, char init, int id)
{
	int err = 0;
	int len = 0;
	char* conf_rec;
	char *token[MAX_TOKENS];
	key_word_t* conf_annexg_table=NULL;
	key_word_t* conf_table = lookup(id, conf_if_def_tables);
	
	if (def->annexg && def->protocol){
		conf_annexg_table=lookup(def->annexg, conf_annexg_def_tables);
		if (conf_annexg_table){
			set_conf_param("PROTOCOL", def->protocol, chan_conftab, def->chanconf);	
		}
	}
	
	/* Prepare configuration data */
	strncpy(def->chanconf->name, def->name, WAN_IFNAME_SZ);
	if (def->addr)
		strncpy(def->chanconf->addr, def->addr, WAN_ADDRESS_SZ);

	if (def->usedby)
		strncpy(def->chanconf->usedby, def->usedby, USED_BY_FIELD);

	if (def->label){
		if (conf_annexg_table){
			set_conf_param("LABEL", def->label, conf_annexg_table, &def->chanconf->u);
		}else{
			memcpy(def->chanconf->label,def->label,WAN_IF_LABEL_SZ);
		}
	}
		
	if (def->virtual_addr){
		if (conf_annexg_table){
			set_conf_param("VIRTUAL_ADDR",def->virtual_addr, 
					conf_annexg_table, &def->chanconf->u);
		}
	}	
	if (def->real_addr){
		if (def->annexg){
			if (conf_annexg_table){
				set_conf_param("REAL_ADDR",def->real_addr, 
						conf_annexg_table, &def->chanconf->u);
			}	
		}
	}
				
	if (def->conf) for (conf_rec = def->conf; *conf_rec; conf_rec += len) {
		int toknum;
		
		len = strlen(conf_rec) + 1;
		toknum = tokenize(conf_rec, token);
		if (toknum < 2) {
			show_error(ERR_CONFIG);
			return -EINVAL;
		}

		/* Look up a keyword first in common, then media-specific
		 * configuration definition tables.
		 */
		strupcase(token[0]);
		if (set_conf_param(token[0], token[1], chan_conftab, def->chanconf)) {

			if (def->annexg && conf_annexg_table){

				if (!conf_annexg_table || 
				    set_conf_param(token[0], token[1], 
						  conf_annexg_table, &def->chanconf->u)) {
				
					printf("Invalid Annexg/Lip parameter %s\n", token[0]);
					show_error(ERR_CONFIG);
					return ERR_CONFIG;
				}
				
			}else if (conf_table){
				if (set_conf_param(token[0], token[1], conf_table, &def->chanconf->u)) {
					printf("Invalid Iface parameter %s\n", token[0]);
					show_error(ERR_CONFIG);
					return ERR_CONFIG;
				}
			}else{
				printf("Invalid config table !!! parameter %s\n", token[0]);
				printf("Conf_table %p  %p\n",
						conf_table,
						bitstrm_if_conftab);
				show_error(ERR_CONFIG);
				return ERR_CONFIG;
			}
		}else{

			if (strcmp(token[0], "ACTIVE_CH") == 0){
				strcpy(def->active_ch, token[1]);
			}
		}
	}

	len=0;
	
	if (def->annexg){

		conf_annexg_table=lookup(def->annexg, conf_annexg_def_tables);

		if (def->conf_profile) for (conf_rec = def->conf_profile; *conf_rec; conf_rec += len) {
			int toknum;

			len = strlen(conf_rec) + 1;
			toknum = tokenize(conf_rec, token);
			if (toknum < 2) {
				show_error(ERR_CONFIG);
				return -EINVAL;
			}

			/* Look up a keyword first in common, then media-specific
			 * configuration definition tables.
			 */
			strupcase(token[0]);

			if (!conf_annexg_table ||
			     set_conf_param(token[0], token[1], conf_annexg_table, &def->chanconf->u)) {
				if (set_conf_param(token[0], token[1], chan_conftab, def->chanconf)) {
					printf("Invalid parameter %s\n", token[0]);
					show_error(ERR_CONFIG);
					return ERR_CONFIG;
				}
			}

			
		}
	}
	err=0;

	if (init){
		return err;
	}
	
	return exec_chan_cmd(dev,def);
}


int exec_chan_cmd(int dev, chan_def_t *def)
{
	if (!def->chanconf){
		printf("%s: Error: Device %s has no config structure\n",
				prognamed,def->name);
		return -EFAULT;
	}

	fflush(stdout);

	switch (def->annexg){

	case NO_ANNEXG:
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
		config.arg = (void*)def->chanconf;
		if (ioctl(dev, ROUTER_IFNEW, &config) < 0) {
#else
		if (ioctl(dev, ROUTER_IFNEW, def->chanconf) < 0) {
#endif
			fprintf(stderr, "\n\n\t%s: Interface %s setup failed\n", prognamed, def->name);
			fprintf(stderr, "\t%s: ioctl(ROUTER_IFNEW,%s) failed:\n", 
					progname_sp, def->name);
			fprintf(stderr, "\t%s:\t%d - %s\n", progname_sp, errno, strerror(errno));
			if( weanie_flag ) {
				fprintf(stderr, "\n\tWanpipe drivers could not setup network interface\n");
				fprintf(stderr, "\tPlease check %s and\n", verbose_log);
				fprintf(stderr, "\t%s for errors\n", krnl_log_file); 
			}
			fprintf(stderr, "\n");
			return errno;
		}

#if defined(WAN_HWEC)
		{ /* ALEX */
			struct link_def	*linkdef = def->link;
			int		err;

			if ((linkdef->config_id == WANCONFIG_AFT_TE1 || 
			     linkdef->config_id == WANCONFIG_AFT_ANALOG) &&
					def->chanconf->hwec.enable){

				err = config_hwec(linkdef->name);
				if (err){
					return err;
				}

				err = enable_hwec(linkdef->name,
						  def->name,
						  def->active_ch);
				if (err){
					release_hwec(linkdef->name);
					return err;
				}
			}
		}
#endif
		break;

#if defined(__LINUX__)
	case ANNEXG_LAPB:
		strncpy(def->chanconf->master,master_lapb_dev, WAN_IFNAME_SZ);

		if (ioctl(dev, ROUTER_IFNEW_LAPB, def->chanconf) < 0) {
			fprintf(stderr, "\n\n\t%s: Interface %s setup failed\n", prognamed, def->name);
			fprintf(stderr, "\t%s: ioctl(ROUTER_IFNEW_LAPB,%s) failed:\n", 
					progname_sp, def->name);
			fprintf(stderr, "\t%s:\t%d - %s\n", progname_sp, errno, strerror(errno));
			if( weanie_flag ) {
				fprintf(stderr, "\n\tWanpipe drivers could not setup network interface\n");
				fprintf(stderr, "\tPlease check %s and\n", verbose_log);
				fprintf(stderr, "\t%s for errors\n", krnl_log_file); 
			}
			fprintf(stderr, "\n");
			return errno;
		}
		break;

	case ANNEXG_X25:
		strncpy(def->chanconf->master, master_x25_dev, WAN_IFNAME_SZ);

		if (ioctl(dev, ROUTER_IFNEW_X25, def->chanconf) < 0) {
			fprintf(stderr, "\n\n\t%s: Interface %s setup failed\n", prognamed, def->name);
			fprintf(stderr, "\t%s: ioctl(ROUTER_IFNEW_X25,%s) failed:\n", 
					progname_sp, def->name);
			fprintf(stderr, "\t%s:\t%d - %s\n", progname_sp, errno, strerror(errno));
			if( weanie_flag ) {
				fprintf(stderr, "\n\tWanpipe drivers could not setup network interface\n");
				fprintf(stderr, "\tPlease check %s and\n", verbose_log);
				fprintf(stderr, "\t%s for errors\n", krnl_log_file); 
			}
			fprintf(stderr, "\n");
			return errno;
		}
		break;

	// DSP_20
	case ANNEXG_DSP:
		strncpy(def->chanconf->master, master_dsp_dev, WAN_IFNAME_SZ);
		if (ioctl(dev, ROUTER_IFNEW_DSP, def->chanconf) < 0) {
			fprintf(stderr, "\n\n\t%s: Interface %s setup failed\n", prognamed, def->name);
			fprintf(stderr, "\t%s: ioctl(ROUTER_IFNEW_DSP,%s) failed:\n", 
					progname_sp, def->name);
			fprintf(stderr, "\t%s:\t%d - %s\n", progname_sp, errno, strerror(errno));
			if( weanie_flag ) {
				fprintf(stderr, "\n\tWanpipe drivers could not setup network interface\n");
				fprintf(stderr, "\tPlease check %s and\n", verbose_log);
				fprintf(stderr, "\t%s for errors\n", krnl_log_file); 
			}
			fprintf(stderr, "\n");
			return errno;
		}
		break;
#endif

	case ANNEXG_FR:
	case ANNEXG_PPP:
	case ANNEXG_CHDLC:		
	case ANNEXG_LIP_LAPB:
	case ANNEXG_LIP_XDLC:
	case ANNEXG_LIP_TTY:
	case ANNEXG_LIP_XMTP2:
	case ANNEXG_LIP_LAPD:
	case ANNEXG_LIP_X25:
	case ANNEXG_LIP_ATM:
	case ANNEXG_LIP_KATM:
		strncpy(def->chanconf->master, master_lip_dev, WAN_IFNAME_SZ);

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
		config.arg = (void*)def->chanconf;
		if (ioctl(dev, ROUTER_IFNEW_LIP, &config) < 0) {
#else
		if (ioctl(dev, ROUTER_IFNEW_LIP, def->chanconf) < 0) {
#endif
			fprintf(stderr, "\n\n\t%s: Interface %s setup failed\n", prognamed, def->name);
			fprintf(stderr, "\t%s: ioctl(ROUTER_IFNEW_LIP,%s) failed:\n", 
					progname_sp, def->name);
			fprintf(stderr, "\t%s:\t%d - %s\n", progname_sp, errno, strerror(errno));
			if( weanie_flag ) {
				fprintf(stderr, "\n\tWanpipe drivers could not setup network interface\n");
				fprintf(stderr, "\tPlease check %s and\n", verbose_log);
				fprintf(stderr, "\t%s for errors\n", krnl_log_file); 
			}
			fprintf(stderr, "\n");
			return errno;
		}
		break;
	default:
		show_error(ERR_CONFIG);
		return -EINVAL;
	}
	return 0;
}



#if 0

/*============================================================================
 * Set configuration parameter.
 *	Look up a keyword in configuration description table to find offset
 *	and data type of the configuration parameter into configuration data
 *	buffer.  Convert parameter to apporriate data type and store it the
 *	configuration data buffer.
 *
 *	Return:	0 - success
 *		1 - error
 *		-1 - not found
 */
int set_conf_param (char* key, char* val, key_word_t* dtab, void* conf)
{
	ulong tmp = 0;

	/* Search a keyword in configuration definition table */
	for (; dtab->keyword && strcmp(dtab->keyword, key); ++dtab);
	if (dtab->keyword == NULL) return -1;	/* keyword not found */

	/* Interpert parameter value according to its data type */

	if (dtab->dtype == DTYPE_FILENAME) {
		return read_data_file(
			val, (void*)((char*)conf + dtab->offset));
	}

	if (verbose) printf(" * Setting %s to %s offset %i\n", key, val, dtab->offset);
	if (dtab->dtype == DTYPE_STR) {
		//wanif_conf_t *chanconf = (wanif_conf_t *)conf;
		strcpy((char*)conf + dtab->offset, val);

		//???????????????

		//if (!strcmp(key,"X25_CALL_STRING"))
		//	printf ("Setting STRING: %s to %s offset %i\n",
		//			key,chanconf->x25_call_string,dtab->offset);
		return 0;
	}

	if (!isdigit(*val)) {
		look_up_t* sym;

		strupcase(val);
		for (sym = sym_table;
		     sym->ptr && strcmp(sym->ptr, val);
		     ++sym)
		;
		if (sym->ptr == NULL) {
			if (verbose) printf(" * invalid term %s ...\n", val);
			return 1;
		}
		else tmp = sym->val;
	}
	else tmp = strtoul(val, NULL, 0);

	switch (dtab->dtype) {

	case DTYPE_INT:
		*(int*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_UINT:
		*(uint*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_LONG:
		*(long*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_ULONG:
		*(ulong*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_SHORT:
		*(short*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_USHORT:
		*(ushort*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_CHAR:
		*(char*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_UCHAR:
		*(unsigned char*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_PTR:
		*(void**)((char*)conf + dtab->offset) = (void*)tmp;
		break;
	}
	return 0;
}
#endif

/*============================================================================
 * Set configuration parameter.
 *	Look up a keyword in configuration description table to find offset
 *	and data type of the configuration parameter into configuration data
 *	buffer.  Convert parameter to apporriate data type and store it the
 *	configuration data buffer.
 *
 *	Return:	0 - success
 *		1 - error
 *		-1 - not found
 */
int set_conf_param (char* key, char* val, key_word_t* dtab, void* conf)
{
	unsigned int tmp = 0;

	/* Search a keyword in configuration definition table */
	for (; dtab->keyword && strcmp(dtab->keyword, key); ++dtab);
	
	if (dtab->keyword == NULL) return -1;	/* keyword not found */

	/* Interpert parameter value according to its data type */

	if (dtab->dtype == DTYPE_FILENAME) {
		return read_data_file(
			val, (void*)((char*)conf + dtab->offset));
	}

	if (verbose) printf(" * Setting %s to %s\n", key, val);

	if (dtab->dtype == DTYPE_STR) {
		strcpy((char*)conf + dtab->offset, val);
		return 0;
	}

	if (!isdigit(*val) || 
	    strcmp(key, "TE_ACTIVE_CH") == 0 || strcmp(key, "ACTIVE_CH") == 0 || 
	    strcmp(key, "TE_LBO") == 0 || strcmp(key, "LBO") == 0 ||
	    strcmp(key, "FE_MEDIA") == 0 || strcmp(key, "MEDIA") == 0 || 
	    strcmp(key, "RBS_CH_MAP") == 0 || strcmp(key, "TE_RBS_CH") == 0 ||
	    strcmp(key, "TDMV_DCHAN") == 0){
		look_up_t* sym;
		unsigned int tmp_ch;

		strupcase(val);
		for (sym = sym_table;
		     sym->ptr && strcmp(sym->ptr, val);
		     ++sym)
		;
		if (sym->ptr == NULL) {
			int ok_zero=0;

			/* TE1 */
			if (strcmp(key, "TE_ACTIVE_CH") && strcmp(key, "ACTIVE_CH") && 
			    strcmp(key, "RBS_CH_MAP") && strcmp(key, "TE_RBS_CH") &&
			    strcmp(key, "TDMV_DCHAN")) {	
				if (verbose) printf(" * invalid term %s ...\n", val);
				return 1;
			}

			if (strcmp(key, "TDMV_DCHAN") == 0) {
                         	ok_zero=1;
			}

			/* TE1 Convert active channel string to UINT */
		 	tmp_ch = parse_active_channel(val);	
			if (tmp_ch == 0 && !ok_zero){
				if (verbose) printf("Illegal active channel range! %s ...\n", val);
				fprintf(stderr, "Illegal active channel range! min=1 max=31\n");
				return -1;
			}
			tmp = (unsigned int)tmp_ch;
		}
		else tmp = sym->val;
	}
	else tmp = strtoul(val, NULL, 0);

	/* SANITY CHECK */
	switch (dtab->dtype) {

	case DTYPE_INT:
		SIZEOFASSERT(dtab, sizeof(int));
		*(int*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_UINT:
		SIZEOFASSERT(dtab, sizeof(unsigned int));
		*(unsigned int*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_LONG:
		SIZEOFASSERT(dtab, sizeof(long));
		*(long*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_ULONG:
		SIZEOFASSERT(dtab, sizeof(unsigned long));
		*(unsigned long*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_SHORT:
		SIZEOFASSERT(dtab, sizeof(short));
		*(short*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_USHORT:
		SIZEOFASSERT(dtab, sizeof(unsigned short));
		*(ushort*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_CHAR:
		SIZEOFASSERT(dtab, sizeof(char));
		*(char*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_UCHAR:
		SIZEOFASSERT(dtab, sizeof(unsigned char));
		*(unsigned char*)((char*)conf + dtab->offset) = tmp;
		break;

	case DTYPE_PTR:
		*(void**)((char*)conf + dtab->offset) = (void*)tmp;
		break;
	}
	return 0;
}


/*============================================================================
 * Read configuration file section.
 *	Return a pointer to memory filled with key entries read from given
 *	section or NULL if section is not found. Each key is a zero-terminated
 *	ASCII string with no leading spaces and comments. The last key is
 *	followed by an empty string.
 *	Note:	memory is allocated dynamically and must be released by the
 *		caller.
 */
char* read_conf_section (FILE* file, char* section)
{
	char key[MAX_CFGLINE];		/* key buffer */
	char* buf = NULL;
	int found = 0, offs = 0, len;
	
	rewind(file);
	while ((len = read_conf_record(file, key)) > 0) {
		char* tmp;

		if (found) {
			if (*key == '[') break;	/* end of section */

			if (buf) tmp = realloc(buf, offs + len + 1);
			else tmp = malloc(len + 1);
			if (tmp) {
				buf = tmp;
				strcpy(&buf[offs], key);
				offs += len;
				buf[offs] = '\0';
			}
			else {	/* allocation failed */
				show_error(ERR_SYSTEM);
 				break;
			}
		}
		else if (*key == '[') {
			tmp = strchr(key, ']');
			if (tmp != NULL) {
				*tmp = '\0';
				if (strcmp(&key[1], section) == 0) {
 					if (verbose) printf(
						" * Reading section [%s]...\n",
						section);
					found = 1;
				}
			}
		}
	}
 	if (verbose && !found) printf(
		" * section [%s] not found!\n", section);
	return buf;
 }

/*============================================================================
 * Read a single record from the configuration file.
 *	Read configuration file stripping comments and leading spaces until
 *	non-empty line is read.  Copy it to the destination buffer.
 *
 * Return string length (incl. terminating zero) or 0 if end of file has been
 * reached.
 */
int read_conf_record (FILE* file, char* key)
{
	char buf[MAX_CFGLINE];		/* line buffer */

	while (fgets(buf, MAX_CFGLINE, file)) {
		char* str;
		int len;

		/* Strip leading spaces and comments */
		for (str = buf; *str && strchr(" \t", *str); ++str);
		len = strcspn(str, "#;\n\r");
		if (len) {
			str[len] = '\0';
			strcpy(key, str);
			return len + 1;
		}
	}
	return 0;
}

/*============================================================================
 * Tokenize string.
 *	Parse a string of the following syntax:
 *		<tag>=<arg1>,<arg2>,...
 *	and fill array of tokens with pointers to string elements.
 *
 *	Return number of tokens.
 */

void init_tokens(char **token)
{
	int i;

	for (i=0;i<MAX_TOKENS;i++){
		token[i]=NULL;
	}
}


/* Bug Fix by Ren�Scharfe <l.s.r@web.de>
 * removed strtok 
 */
int tokenize (char *str, char **tokens)
{
       	int cnt = 0;
       	char *tok;

	init_tokens(tokens);
	
       	if (!str){
		return 0;
	}

       	tok = strchr(str, '=');
        if (!tok)
		return 0;


	*tok='\0';

       	tokens[cnt] = str;
	str=++tok;
	
       	while (tokens[cnt] && (cnt < MAX_TOKENS-1)) {
	      
		tokens[cnt] = str_strip(tokens[cnt], " \t");
		
               	if ((tok = strchr(str, ',')) == NULL){
			tokens[++cnt] = str_strip(str, " \t");
			goto end_tokenize;
		}

		*tok='\0';

		tokens[++cnt] = str_strip(str, " \t");
		str=++tok;
	
       	}
end_tokenize:
       	return ++cnt;
 }

/*============================================================================
 * Strip leading and trailing spaces off the string str.
 */
char* str_strip (char* str, char* s)
{
	char* eos = str + strlen(str);		/* -> end of string */

	while (*str && strchr(s, *str))
		++str;				/* strip leading spaces */
	while ((eos > str) && strchr(s, *(eos - 1)))
		--eos;				/* strip trailing spaces */
	*eos = '\0';
	return str;
}

/*============================================================================
 * Uppercase string.
 */
char* strupcase (char* str)
{
	char* s;

	for(s = str; *s; ++s) *s = toupper(*s);
	return str;
}

/*============================================================================
 * Get a pointer from a look-up table.
 */
void* lookup (int val, look_up_t* table)
{
	while (table->val && (table->val != val)) table++;
	return table->ptr;
}

/*============================================================================
 * Look up a symbolic name in name table.
 *	Return a numeric value associated with that name or zero, if name was
 *	not found.
 */
int name2val (char* name, look_up_t* table)
{
	while (table->ptr && strcmp(name, table->ptr)) table++;
	return table->val;
}

/*============================================================================
 * Create memory image of a file.
 */
int read_data_file (char* name, data_buf_t* databuf)
{
	int err = 0;
	FILE* file;
	unsigned long fsize;

	databuf->data = NULL;
	databuf->size = 0;
	file = fopen(name, "rb");
	if (file == NULL) {
		if (verbose) printf(" * Can't open data file %s!\n", name);
		fprintf(stderr, "%s: Can't open data file %s!\n", prognamed, name);
		show_error(ERR_SYSTEM);
		return ERR_SYSTEM;
	}

	fsize = filesize(file);
	if (!fsize) {
		if (verbose) printf(" * Data file %s is empty!\n", name);
		err = ERR_CONFIG;
		show_error(err);
	}
	else {

		if (verbose){
			printf(" * Reading %lu bytes from %s ...\n",
				fsize, name);
		}
		if (fread(firmware_file_buffer, 1, fsize, file) != fsize) {
			err = ERR_SYSTEM;
			show_error(err);
			goto done;
		}
		databuf->data = firmware_file_buffer;
		databuf->size = fsize;
	}
done:
	fclose(file);
	return err;
}

/*============================================================================
 * Get file size
 *	Return file length or 0 if error.
 */
unsigned long filesize (FILE* file)
{
	unsigned long size = 0;
	unsigned long cur_pos;

	cur_pos = ftell(file);
	if ((cur_pos != -1L) && !fseek(file, 0, SEEK_END)) {
		size = ftell(file);
		fseek(file, cur_pos, SEEK_SET);
	}
	return size;
}

/*============================================================================
 * Convert decimal string to unsigned integer.
 * If len != 0 then only 'len' characters of the string are converted.
 */
unsigned int dec_to_uint (unsigned char* str, int len)
{
	unsigned val;

	if (!len) len = strlen(str);
	for (val = 0; len && is_digit(*str); ++str, --len)
		val = (val * 10) + (*str - (unsigned)'0')
	;
	return val;
}

unsigned int get_config_data (int argc, char **argv)
{

	FILE *fp;
	char *device_name=NULL;

	fp = fopen (conf_file,"w");
	if (fp == NULL){
		printf("Could not open file\n");
		return 1;
	}

	get_devices(fp,&argc,&argv,&device_name);
	get_interfaces(fp,&argc,&argv,device_name);
	get_hardware(fp,&argc,&argv,device_name);
	get_intr_setup (fp,&argc,&argv);

	fclose(fp);	
	return 0;

}

int get_devices (FILE *fp, int *argc_ptr, char ***argv_ptr, char **device_name){

	int i;
	int 
						start=0, stop=0;
	int argc = *argc_ptr;
	char **argv = *argv_ptr;

	for (i=0;i<argc;i++){
		if (!strcmp(argv[i],"[devices]")){
			start = i;
			continue;
		}
		if (strstr(argv[i],"[") != NULL){
			stop = i;
			break;
		}	
	}

	if (!stop){
		printf("ERROR: No devices found, Incomplete Argument List\n");
		return 1;
	}

	if (stop == 3){
		fprintf(fp,"%s\n",argv[start]);
		*device_name = argv[start+1];					
		fprintf(fp,"%s = %s\n",argv[start+1],argv[start+2]);	
	}else if (stop > 3) {
		fprintf(fp,"%s\n",argv[start]);
		fprintf(fp,"%s = %s, %s\n",argv[start+1],argv[start+2],argv[start+3]);		
	}else{
		printf("ERROR: No devices found, Too many devices arguments\n");
		return 1;
	}		
	
	*argv_ptr += stop;
	*argc_ptr -= stop;

	//???????????????
	//printf("Start is %i and Stop is %i\n",start,stop);
	return 0;

}

int get_interfaces (FILE *fp, int *argc_ptr, char ***argv_ptr, char *device_name){

	char **argv = *argv_ptr;
	int argc = *argc_ptr;
	int i, start=0, stop=0;
	
	for (i=0;i<argc;i++){
		
		//?????????????????????
		//printf("INTR: Argv %i is %s\n",i,argv[i]);
		if (!strcmp(argv[i],"[interfaces]")){
			start = i;
			continue;
		}
		if (strstr(argv[i],"[") != NULL){
			stop = i;
			break;
		}	
	}
	
	if (!stop){
		printf("ERROR: No interfaces found, Incomplete Argument List\n");
		return 1;
	}

	if ((stop-1)%4){
		printf("ERROR: Insuficient/Too many Interface arguments\n");
		return 1;
	}

	fprintf(fp, "\n%s\n", argv[start]);
	for (i=(start+1);i<stop;i+=4){
		fprintf(fp, "%s = %s, ",argv[i],argv[i+1]);
		if (!strcmp(argv[i+2],"-")){
			fprintf(fp, " ,%s\n",argv[i+3]);
		}else{
			fprintf(fp, "%s, %s\n",argv[i+2],argv[i+3]);
		}
	}

	*argv_ptr += stop;
	*argc_ptr -= stop;

	//???????????????
	//printf("Interface Start is %i and Stop is %i\n",start,stop);
	return 0;
}

int get_hardware (FILE *fp, int *argc_ptr, char ***argv_ptr, char *device_name)
{

	char **argv = *argv_ptr;
	int argc = *argc_ptr;
	int i, start=0, stop=0;

	for (i=0;i<argc;i++){
		
		//?????????????????????
		//printf("HRDW: Argv %i is %s\n",i,argv[i]);
		if (strstr(argv[i],"[wanpipe") != NULL){
			start = i;
			continue;
		}
		if (strstr(argv[i],"[") != NULL){
			stop = i;
			break;
		}	
	}

	if (!stop){
		stop = argc;
	}

	if ((stop-1)%2){
		printf("ERROR: Insuficient/Too many Hardware arguments\n");
		return 1;
	}

	fprintf(fp, "\n%s\n", argv[start]);

	for (i=(start+1);i<stop;i+=2){
		fprintf(fp, "%s = %s\n",argv[i],argv[i+1]);
	}

	*argv_ptr += stop;
	*argc_ptr -= stop;

	//?????????????????
	//printf("Hardware Start is %i and Stop is %i\n",start,stop);
	return 0;
}

int get_intr_setup (FILE *fp, int *argc_ptr, char ***argv_ptr)
{

	char **argv = *argv_ptr;
	int argc = *argc_ptr;
	int i, start=0, stop=0, sfound=0;

	for (i=0;i<argc;i++){
		
		//?????????????????????
		//printf("INTR SETUP: Argv %i is %s\n",i,argv[i]);
		if ((strstr(argv[i],"[") != NULL) && !sfound){
			start = i;
			sfound = 1;
			continue;
		}
		if (strstr(argv[i],"[") != NULL){
			stop = i;
			break;
		}	
	}

	if (!stop){
		stop=argc;
	}

	if ((stop-1)%2){
		printf("ERROR: Insuficient/Too many Interface Setup arguments\n");
		return 1;
	}

	fprintf(fp, "\n%s\n", argv[start]);

	for (i=(start+1);i<stop;i+=2){
		fprintf(fp, "%s = %s\n",argv[i],argv[i+1]);
	}

	*argv_ptr += stop;
	*argc_ptr -= stop;

	//?????????????????
	//printf("Interface SETUP Start is %i and Stop is %i\n",start,stop);

	if (stop != argc){
		get_intr_setup (fp, argc_ptr, argv_ptr);
	}

	return 0;
}

int conf_file_down (void)
{
	FILE* file;
	char* conf;             /* -> section buffer */
	char devname[WAN_DRVNAME_SZ];
	char *token[MAX_TOKENS];
	int toknum, err;

	//printf("Shutting down Device %s\n",conf_file);
        file = fopen(conf_file, "r");
        if (file == NULL) {
                show_error(ERR_SYSTEM);
                return ERR_SYSTEM;
        }

	/* Read [devices] section */
        conf = read_conf_section(file, "devices");
        if (conf == NULL){
		fclose(file);
		return ERR_CONFIG;
	}
	
        toknum = tokenize(conf, token);
        if (toknum < 2){
		fclose(file);
		free(conf);
		return 1;
	}
	
        strncpy(devname, token[0], WAN_DRVNAME_SZ);

	err = router_down (devname,0);
	if (!err && verbose)
		printf ("Done\n");

	free(conf);
	fclose(file);

	return err;
}

/* ========================================================================
 * Part of the Dameon code
 */

#if 0
#define SET_BINARY(f) (void)0

static unsigned long sysv_sum_file (const char *file)
{
	int fd;
	unsigned char buf[8192];
	register unsigned long checksum = 0;
	int bytes_read;

	fd = open (file, O_RDONLY);
	if (fd < 0){
		//perror("checksum file: ");
	  	return 0;
	}

	/* Need binary I/O, or else byte counts and checksums are incorrect.  */
	SET_BINARY (fd);

	while ((bytes_read = safe_read (fd, buf, sizeof buf)) > 0){
		register int i;

		for (i = 0; i < bytes_read; i++){
			checksum += buf[i];
		}
	}

	if (bytes_read < 0){
		perror("bytes read: ");
		close(fd);
		return 0;
	}

	close(fd);
	return checksum;
}
#endif

#define TIME_STRING_BUF 50

char *time_string(time_t t,char *buf)
{
	struct tm *local;
	local = localtime(&t);
	strftime(buf,TIME_STRING_BUF,"%d",local);
	return buf;
}

int has_config_changed(link_def_t *linkdef, char *name)
{
	unsigned char filename[50];
	//unsigned long checksum;
	time_t modified;
	struct stat statbuf;
	unsigned char timeBuf[TIME_STRING_BUF];

	sprintf(filename,"/etc/wanpipe/%s.conf",name);

	if (lstat(filename,&statbuf)){
		return -1;
	}
	
//	printf("Access Time  : %s %i\n",time_string(statbuf.st_atime,timeBuf),statbuf.st_atime);
//	printf("Modified Time: %s %i\n",time_string(statbuf.st_mtime,timeBuf),statbuf.st_mtime);
//	printf("Creation Time: %s %i\n",time_string(statbuf.st_ctime,timeBuf),statbuf.st_ctime);
	
	modified = statbuf.st_mtime;
//	checksum = sysv_sum_file(filename);

	//printf("%s Check sum %u\n",name,checksum);

	if (!linkdef){
		link_def_t *def;
		for (def=link_defs;def;def=def->next){
			if (!strcmp(def->name,name)){
				linkdef=def;
				break;
			}
		}
		if (!linkdef){
			return -1;
		}
	}

//	if (linkdef->checksum != checksum){
//		
//		linkdef->checksum = checksum;
//		printf("%s: Configuration file %s changed\n",prognamed,filename);
//		return 1;
//	}
	if (linkdef->modified != modified){
		
		linkdef->modified = modified;
		printf("%s: Configuration file %s changed: %s \n",
				prognamed,filename,time_string(modified,timeBuf));
		return 1;
	}

	return 0;
}

void free_device_link(char *devname)
{
	link_def_t *linkdef,*prev_lnk=link_defs;

	for (linkdef = link_defs; linkdef; linkdef = linkdef->next){
		if (!strcmp(linkdef->name, devname)){
		
			while (linkdef->chan) {
				chan_def_t* chandef = linkdef->chan;
		
				if (chandef->conf) free(chandef->conf);
				if (chandef->conf_profile) free(chandef->conf_profile);
				if (chandef->addr) free(chandef->addr);
				if (chandef->usedby) free(chandef->usedby);
				if (chandef->protocol) free(chandef->protocol);
				if (chandef->label) free(chandef->label);
				if (chandef->virtual_addr) free(chandef->virtual_addr);
				if (chandef->real_addr) free(chandef->real_addr);
				linkdef->chan = chandef->next;

				if (chandef->chanconf){
					free(chandef->chanconf);
				}
				free(chandef);
			}
			if (linkdef->conf) free(linkdef->conf);
			if (linkdef->descr) free(linkdef->descr);
		
			if (linkdef == link_defs){
				link_defs = linkdef->next;
			}else{
				prev_lnk->next = linkdef->next;	
			}
			printf("%s: Freeing Link %s\n",prognamed,linkdef->name);
			if (linkdef->linkconf){
				free(linkdef->linkconf);
			}
			free(linkdef);
			return;
		}

		prev_lnk=linkdef;
	}
}

void free_linkdefs(void)
{
	/* Clear definition list */
	while (link_defs != NULL) {
		link_def_t* linkdef = link_defs;

		while (linkdef->chan) {
			chan_def_t* chandef = linkdef->chan;
		
			if (chandef->conf) free(chandef->conf);
			if (chandef->conf_profile) free(chandef->conf_profile);
			if (chandef->addr) free(chandef->addr);
			if (chandef->usedby) free(chandef->usedby);
			if (chandef->protocol) free(chandef->protocol);
			if (chandef->label) free(chandef->label);
			if (chandef->virtual_addr) free(chandef->virtual_addr);
			if (chandef->real_addr) free(chandef->real_addr);
			linkdef->chan = chandef->next;

			if (chandef->chanconf){
				free(chandef->chanconf);
			}
			free(chandef);
		}
		if (linkdef->conf) free(linkdef->conf);
		if (linkdef->descr) free(linkdef->descr);
		

		if (linkdef->linkconf){
			free(linkdef->linkconf);
		}

		link_defs = linkdef->next;
		free(linkdef);
	}
}

int device_syncup(char *devname)
{
	unsigned char filename[100];
	int err;

	free_device_link(devname);

	sprintf(filename,"%s/%s.conf",conf_dir,devname);

	printf("%s: Parsing configuration file %s\n",prognamed,filename);
	err = parse_conf_file(filename);
	if (err){
		return -EINVAL;
	}
	
	has_config_changed(link_defs,link_defs->name);
	
	err = configure_link(link_defs,1);
	if (err){
		return -EINVAL;
	}

	return 0;
}

void sig_func (int sigio)
{
	if (sigio == SIGUSR1){
		printf("%s: Rx signal USR1, User prog ok\n",prognamed);
		return;
	}

	if (sigio == SIGUSR2){
		time_t time_var;
		time(&time_var);
		if ((time_var-time_ui) > MAX_WAKEUI_WAIT){
			time_ui=time_var;
			wakeup_ui=1;
		}
		//else{
		//	printf("%s: Ignoring Wakeup UI %i\n",
		//			prognamed,time_var-time_ui);
		//}
		return;
	}

	if (sigio == SIGHUP){
		printf("%s: Rx signal HUP: Terminal closed!\n",prognamed);
		return;
	}
		
	printf("%s: Rx TERM/INT (%i) singal, freeing links\n",prognamed,sigio);
	free_linkdefs();
	unlink(WANCONFIG_SOCKET);
	unlink(WANCONFIG_PID);
	exit(1);
}


int start_chan (int dev, link_def_t *def)
{
	int err=ERR_SYSTEM;
	chan_def_t *chandef;

	for (chandef = def->chan; chandef; chandef = chandef->next) {

		switch (chandef->annexg){
			case NO_ANNEXG:
				strncpy(master_lapb_dev,chandef->name, WAN_IFNAME_SZ);
				strncpy(master_lip_dev,chandef->name, WAN_IFNAME_SZ);
				break;
			case ANNEXG_LAPB:
				strncpy(master_x25_dev,chandef->name, WAN_IFNAME_SZ);
				break;
			case ANNEXG_X25:
				strncpy(master_dsp_dev,chandef->name, WAN_IFNAME_SZ);
				break;
		}

		if (!dev_name || !strcmp(chandef->name,dev_name)){

			if (verbose){
				printf(
				" * Configuring channel %s (%s). Media address: %s\n",
				chandef->name,
				"no description",
				chandef->addr ? chandef->addr : "not specified");
			}
		
			err=exec_chan_cmd(dev,chandef);
			if (err){
				return err;
			}
			
			if (!dev_name){
				continue;
			}else{
				return err;
			}
		}
	}
	return err;
}

int start_link (void)
{
	link_def_t* linkdef; 
	int err=-EINVAL;
	int dev=-1;
	unsigned char filename[100];

	if (!link_defs){
		if (card_name){
			err=device_syncup(card_name);
			if (err){
				return err;
			}
		}else{
			printf("%s: No wanpipe links initialized!\n",prognamed);
			return err;
		}
	}else{
		if (card_name){
			if (has_config_changed(NULL,card_name)!=0){
				err=device_syncup(card_name);
				if (err){
					return err;
				}
			}
		}else{
start_cfg_chk:			
			for (linkdef = link_defs; linkdef; linkdef = linkdef->next){
				if (has_config_changed(linkdef,linkdef->name)!=0){
					err=device_syncup(linkdef->name);
					if (err){
						return err;
					}
					goto start_cfg_chk;
				}
			}
		}
	}
	
	printf("\n%s: Starting Card %s\n",prognamed,card_name?card_name:"All Cards");
	
	for (linkdef = link_defs; linkdef; linkdef = linkdef->next){

		if (!card_name || !strcmp(linkdef->name,card_name)){

			if (verbose){ 
				printf(" * Configuring device %s (%s)\n",
						linkdef->name,
						card_name?card_name:linkdef->name);
			}
			fflush(stdout);
			
			sprintf(filename, "%s/%s", router_dir, linkdef->name);
			dev = open(filename, O_RDONLY);
			if (dev<0){
				printf("%s: Failed to open file %s\n",prognamed,filename);
				return -EIO;
			}

			linkdef->linkconf->ft1 = 0;

			err=exec_link_cmd(dev,linkdef);
			if (err){
				close(dev);
				return err;
			}
		
			err=start_chan(dev,linkdef);
			if (err){
				close(dev);
				return err;
			}
			
			close(dev);
			if (!card_name){
				continue;
			}else{
				break;
			}
		}
	}

	return err;
}


int stop_link(void)
{
	link_def_t* linkdef; 
	int err=3;

	if (card_name){
		if (!dev_name){
			return router_down(card_name,0);
		}else{
			return router_ifdel(card_name,dev_name);
		}
	}

	if (!link_defs){
		printf("%s: Stop Link Error: No links initialized!\n",prognamed);
		return err;
	}
	
	for (linkdef = link_defs; linkdef; linkdef = linkdef->next){

		if (verbose){ 
			printf(" * Configuring device %s (%s)\n",
					linkdef->name,
					dev_name?dev_name:"card");
		}
		err=router_down(linkdef->name,0);			
		if (err){
			return err;
		}
	}

	return err;
}


/*======================================================
 * exec_command
 *
 * cmd=[sync|start|stop|restart]
 * card=<device name>		# wanpipe# {#=1,2,3...}
 * dev=<interface name>  	# wp1_fr16
 *
 */

int exec_command(unsigned char *rx_data)
{
	int toknum;
	char *card_str=NULL;
	char *dev_str=NULL;
	int err=-ENOEXEC;
	char *token[MAX_TOKENS];

	toknum = tokenize(rx_data, token);
        if (toknum < 2){ 
		printf("%s: Invalid client cmd = %s\n",prognamed,rx_data);
		return -ENOEXEC;
	}

	//printf("FIRST TOKNUM = %i\n",toknum);
	
	if (!strcmp(token[0],"cmd")){
		//printf("Command is %s\n",token[1]);
		command=strdup(token[1]);
	}else{
		printf("%s: Invalid client command syntax : %s!\n",prognamed,rx_data);
		goto exec_cmd_exit;
	}

	if (toknum > 2){
		card_str=strdup(token[2]);
	}
	if (toknum > 3){
		dev_str=strdup(token[3]);
	}
	
	if (card_str){
		toknum = tokenize(card_str, token);
		if (toknum < 2){ 
			printf("%s: Invalid client command syntax : %s!\n",prognamed,card_str);
			goto exec_cmd_exit;
		}	

		if (!strcmp(token[0],"card")){
			card_name=strdup(token[1]);
			//printf("Card is %s\n",card_name);
		}

		if (dev_str){
			toknum = tokenize(dev_str, token);
			if (toknum < 2){ 
				printf("%s: Invalid client command syntax : %s!\n",prognamed,dev_str);
				goto exec_cmd_exit;
			}	
			if (!strcmp(token[0],"dev")){
				dev_name=strdup(token[1]);
			}
		}
	}

	err=-EINVAL;
	
	if (!strcmp(command,"start")){
		err=start_link();
	}else if (!strcmp(command,"stop")){
		err=stop_link();
	}else if (!strcmp(command,"restart")){
		err=stop_link();
		err=start_link();
	}else if (!strcmp(command,"init")){
		if (card_name){
			err=device_syncup(card_name);
		}else{
			printf("%s: Error: cmd=init : invalid card name!\n",prognamed);
		}
	}else{
		printf("%s: Error invalid client cmd=%s!\n",prognamed,command);
	}
	
exec_cmd_exit:	
	
	fflush(stdout);

	if (card_str){
		free(card_str);
		card_str=NULL;
	}

	if (dev_str){
		free(dev_str);
		dev_str=NULL;
	}
	
	if (command){ 
		free(command);
		command=NULL;
	}
	if (card_name){
		free(card_name);
		card_name=NULL;
	}
	if (dev_name){
		free(dev_name);
		dev_name=NULL;
	}
	return err;
}

void catch_signal(int signum,int nomask)
{
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));

	sa.sa_handler=sig_func;

	if (nomask){
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
		sa.sa_flags |= SA_NODEFER;
#else
		sa.sa_flags |= SA_NOMASK;
#endif
	}
	
	if (sigaction(signum,&sa,NULL))
		perror("sigaction");
}


int start_daemon(void)
{
	struct sockaddr_un address;
	int sock,conn;
	socklen_t addrLength;
	unsigned char rx_data[100];
	int err;
	int fp;
	struct stat file_stat;
	
	verbose=1;

	if (fork() !=0){
		exit(0);
	}
	
	setsid();
	setuid(0);		/* set real UID = root */
	setgid(getegid());
	
	sprintf(prognamed,"wanconfigd[%i]",getpid());

	memset(&rx_data[0],0,100);
	memset(&address,0,sizeof(struct sockaddr_un));
#if 0
	signal(SIGTERM,sig_func);
	signal(SIGINT,sig_func);
	signal(SIGUSR1,sig_func);
	signal(SIGUSR2,sig_func);
#endif
	catch_signal(SIGTERM,0);
	catch_signal(SIGINT,0);
	catch_signal(SIGUSR1,0);
	catch_signal(SIGUSR2,0);
	catch_signal(SIGHUP,0);

	if ((sock=socket(PF_UNIX,SOCK_STREAM,0)) < 0){
		perror("socket: ");
		return sock;
	}

	address.sun_family=AF_UNIX;
	strcpy(address.sun_path, WANCONFIG_SOCKET);

	addrLength = sizeof(address.sun_family) + strlen(address.sun_path);

	if (bind(sock,(struct sockaddr*) &address, addrLength)){
		perror("bind: ");
		return -1;
	}

	if (listen(sock,10)){
		perror("listen: ");
		return -1;
	}

	fp=open(WANCONFIG_PID,(O_CREAT|O_WRONLY),0644);
	if (fp){
		char pid_str[10];
		sprintf(pid_str,"%i",getpid());
		write(fp,&pid_str,strlen(pid_str));
		close(fp);
	}


	err = stat(WANCONFIG_PID_FILE, &file_stat);
	if (err<0){
		printf("\n\nWarning: Failed pid write: rc=%i\n\n",err);
	}else{
		sprintf(rx_data,"echo %i > %s",getpid(),WANCONFIG_PID_FILE);
		if ((err=system(rx_data)) != 0){
			printf("\n\nWarning: Failed pid write: rc=%i\n\n",err);
		}
	}
	
	memset(&rx_data[0],0,100);
	
	printf("%s: ready and listening:\n\n",prognamed);

tryagain:
	while ((conn=accept(sock,(struct sockaddr*)&address,&addrLength))>=0){

		memset(rx_data,0,100);

		/* Wait for a command from client */
		err=recv(conn,rx_data,100,0);
		if (err>0){
			printf("\n%s: rx cmdstr: %s\n",
					prognamed,rx_data);
		}else{
			printf("%s: Error: received %i from new descriptor %i !\n",
						prognamed,conn,err);
			perror("recv: ");	
			close(conn);
			continue;
		}

		fflush(stdout);

		err=exec_command(rx_data);

		err=abs(err);
		if (err){
			printf("%s: Cmd Error err=%i: %s\n\n\n",
					prognamed,err,strerror(err));
		}else{
			printf("%s: Cmd OK err=%i\n\n\n",prognamed,err);
		}	
		
		
		/* Reply to client the result of the command */
		*((short*)&rx_data[0])=err;
		err=send(conn,rx_data,2,0);
		if (err != 2){
			perror("send: ");
			close(conn);
			fflush(stdout);
			continue;
		}
		
		close(conn);

		if (wakeup_ui){
			wakeup_ui=0;
			wakeup_java_ui();	
		}
		fflush(stdout);
	}

	if (conn<0){
		if (errno == EINTR){
			if (wakeup_ui){
				wakeup_ui=0;
				wakeup_java_ui();	
			}
			goto tryagain;
		}
		printf("%s: Accept err = %i\n",prognamed,errno);
		perror("Accept: ");
	}

	printf("%s: Warning: Tried to exit! conn=%i  errno=%i\n",
			prognamed,conn,errno);
	goto tryagain;
	
	close(sock);
	unlink(WANCONFIG_SOCKET);
	unlink(WANCONFIG_PID);
	return 0;
}

#define UI_JAVA_PID_FILE "/var/run/ui.pid"
static time_t ui_checksum=0;
static pid_t ui_pid=0;

void wakeup_java_ui(void)
{
	time_t checksum;
	FILE *file;
	char buf[50];
	struct stat statbuf;

	if (lstat(UI_JAVA_PID_FILE,&statbuf)){
		return;
	}

	checksum = statbuf.st_mtime;	
	if (ui_checksum != checksum){
		
		ui_checksum = checksum;
		
		file = fopen(UI_JAVA_PID_FILE, "r");
		if( file == NULL ) {
			fprintf( stderr, "%s: cannot open %s\n",
					prognamed, UI_JAVA_PID_FILE);
			return;
		}
		
		fgets(buf, sizeof(buf)-1, file);

		ui_pid=atoi(buf);

		fclose(file);
	}

	if (ui_pid){
		if (kill(ui_pid,SIGUSR2) == 0){
			printf("%s: Kicked Java UI: %u\n",prognamed,ui_pid);
		}else{
			printf("%s: Failed to kick UI: %u : %s\n",prognamed,ui_pid,strerror(errno));
		}
	}
	return;
}

int show_config(void)
{
	int 	err = 0;

#if defined(__LINUX__)
	char statbuf[80];
	snprintf(statbuf, sizeof(statbuf), "%s/%s", router_dir, "config");
	gencat(statbuf);
#else
	int 		dev;
	wan_procfs_t	procfs;

	dev = open(WANDEV_NAME, O_RDONLY);
	if (dev < 0){
		show_error(ERR_MODULE);
		return errno;
	}
	config.arg = &procfs;
	memset(&procfs, 0, sizeof(wan_procfs_t));
	procfs.magic 	= ROUTER_MAGIC;
	procfs.max_len 	= 2048;
	procfs.cmd	= WANPIPE_PROCFS_CONFIG;
	procfs.data	= malloc(2048);
	if (procfs.data == NULL){
		show_error(ERR_SYSTEM);
		return -EINVAL;
	}
	if (ioctl(dev, ROUTER_PROCFS, &config) < 0){
		show_error(ERR_SYSTEM);
		goto show_config_end;
	}else{
		if (procfs.offs){
			int i = 0;
			for (i=0;i<procfs.offs;i++){
				putchar(*((unsigned char *)procfs.data + i));
				fflush(stdout);
			}
		}
	}
show_config_end:
	if (dev >= 0) close(dev);
	free(procfs.data);

#endif
	return err;
}

int show_status(void)
{
	int 	err = 0;

#if defined(__LINUX__)
	char statbuf[80];
	if (card_name != NULL){
		snprintf(statbuf, sizeof(statbuf), "%s/%s", router_dir, card_name);
		gencat(statbuf);
	}else{
		snprintf(statbuf, sizeof(statbuf), "%s/%s", router_dir, "status");
		gencat(statbuf);
	}
#else
	int 		dev;
	wan_procfs_t	procfs;

	dev = open(WANDEV_NAME, O_RDONLY);
	if (dev < 0){
		show_error(ERR_MODULE);
		return -ENODEV;
	}
	config.arg = &procfs;
	memset(&procfs, 0, sizeof(wan_procfs_t));
	procfs.magic 	= ROUTER_MAGIC;
	procfs.max_len 	= 2048;
	procfs.cmd	= WANPIPE_PROCFS_STATUS;
	procfs.data	= malloc(2048);
	if (procfs.data == NULL){
		show_error(ERR_SYSTEM);
		return -EINVAL;
	}
	if (ioctl(dev, ROUTER_PROCFS, &config) < 0){
		show_error(ERR_SYSTEM);
		goto show_status_end;
	}else{
		if (procfs.offs){
			int i = 0;
			for (i=0;i<procfs.offs;i++){
				putchar(*((unsigned char *)procfs.data + i));
				fflush(stdout);
			}
		}
	}
show_status_end:
	if (dev >= 0) close(dev);
	free(procfs.data);
#endif
	return err;
}

int show_hwprobe(void)
{
	int 	err = 0;

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	int 		dev;
	wan_procfs_t	procfs;

	dev = open(WANDEV_NAME, O_RDONLY);
	if (dev < 0){
		show_error(ERR_MODULE);
		return -ENODEV;
	}
	config.arg = &procfs;
	memset(&procfs, 0, sizeof(wan_procfs_t));
	procfs.magic 	= ROUTER_MAGIC;
	procfs.max_len 	= 2048;
	procfs.cmd	= WANPIPE_PROCFS_HWPROBE;
	procfs.data	= malloc(2048);
	if (procfs.data == NULL){
		 show_error(ERR_SYSTEM);
		 err=-EINVAL;
		goto show_probe_end;
	}
	if (ioctl(dev, ROUTER_PROCFS, &config) < 0){
		show_error(ERR_SYSTEM);
		err=-EINVAL;
		goto show_probe_end;
	}else{
		if (procfs.offs){
			int i = 0;
			for (i=0;i<procfs.offs;i++){
				putchar(*((unsigned char *)procfs.data + i));
				fflush(stdout);
			}
		}
	}
show_probe_end:
	if (dev >= 0) close(dev);
	free(procfs.data);
#endif
	return err;
}

int debugging(void)
{
    	int	dev;
	int	err = 0;
	char filename[sizeof(router_dir) + WAN_DRVNAME_SZ + 2];

	if (dev_name == NULL){
		fprintf(stderr, "\n\n\tPlease specify device name!\n"); 
		show_error(ERR_SYSTEM);
		return -EINVAL;
	}
#if defined(__LINUX__)
	sprintf(filename, "%s/%s", router_dir, dev_name);
#else
	sprintf(filename, "%s", WANDEV_NAME);
#endif
	
        dev = open(filename, O_RDONLY); 
        if (dev < 0){
		fprintf(stderr, "\n\n\tFAILED open device %s!\n", 
					WANDEV_NAME);
		show_error(ERR_MODULE);
		return -EINVAL;
	}
 	/* Clear configuration structure */
	memset(&u.linkconf, 0, sizeof(wandev_conf_t));
	u.linkconf.magic = ROUTER_MAGIC;
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        strcpy(config.devname, dev_name);
    	config.arg = NULL;
	if (ioctl(dev, ROUTER_DEBUGGING, &config) < 0){ 
	//if (ioctl(dev, ROUTER_DEBUGGING, NULL) < 0){ 

		fprintf(stderr, "\n\n\tROUTER DEBUGGING failed!!\n");
		show_error(ERR_SYSTEM);
		err=-EINVAL;
        }
#else
	/* Linux currently doesn't support this option */
	err=-EINVAL;	
#endif
        if (dev >= 0){ 
		close(dev);
       	}
	return err;
}


int debug_read(void)
{
    	int			dev;
	int			err = 0;
	char 			filename[sizeof(router_dir) + WAN_DRVNAME_SZ + 2];
	wan_kernel_msg_t	wan_kernel_msg;

	if (dev_name == NULL){
		fprintf(stderr, "\n\n\tPlease specify device name!\n"); 
		show_error(ERR_SYSTEM);
		return -EINVAL;
	}
#if defined(__LINUX__)
	sprintf(filename, "%s/%s", router_dir, dev_name);
#else
	sprintf(filename, "%s", WANDEV_NAME);
#endif
	
        dev = open(filename, O_RDONLY); 
        if (dev < 0){
		fprintf(stderr, "\n\n\tFAILED open device %s!\n", 
					WANDEV_NAME);
		show_error(ERR_MODULE);
		return -EINVAL;
	}
 	/* Clear configuration structure */
	memset(&u.linkconf, 0, sizeof(wandev_conf_t));
	u.linkconf.magic = ROUTER_MAGIC;
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        strcpy(config.devname, dev_name);
    	config.arg = &wan_kernel_msg;
#endif
debug_read_again:
	memset(&wan_kernel_msg, 0, sizeof(wan_kernel_msg_t));
	wan_kernel_msg.magic 	= ROUTER_MAGIC;
	wan_kernel_msg.max_len 	= 2048;
	wan_kernel_msg.data	= malloc(2048);
	if (wan_kernel_msg.data == NULL){
		 show_error(ERR_SYSTEM);
		 err=-EINVAL;
		goto debug_read_end;
	}
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	if (ioctl(dev, ROUTER_DEBUG_READ, &config) < 0){ 
#else
	if (ioctl(dev, ROUTER_DEBUG_READ, &wan_kernel_msg) < 0){
#endif

		fprintf(stderr, "\n\n\tROUTER DEBUG_READ failed!!\n");
		show_error(ERR_SYSTEM);
		err=-EINVAL;
        }else{
		if (wan_kernel_msg.len){
			int i = 0;
			for (i=0;i<wan_kernel_msg.len;i++){
				putchar(*((unsigned char *)wan_kernel_msg.data + i));
				fflush(stdout);
			}
			if (wan_kernel_msg.is_more){
				goto debug_read_again;
			}
		}
	}
debug_read_end:
        if (dev >= 0){ 
		close(dev);
       	}
	free(wan_kernel_msg.data);
	return err;
}



void update_adsl_vci_vpi_list(wan_adsl_vcivpi_t* vcivpi_list, unsigned short vcivpi_num)
{
	FILE*		file = NULL, *tmp_file = NULL;
	char		buf[256];
	int 		x = 0;

	file = fopen(adsl_file, "r");
	tmp_file = fopen(tmp_adsl_file, "w");
	if (file == NULL || tmp_file == NULL){
		fprintf( stderr, "%s: cannot open %s or %s\n",
				prognamed, adsl_file, tmp_adsl_file);
		if (file) fclose(file);
		if (tmp_file) fclose(tmp_file);
		show_error(ERR_SYSTEM);
		exit(ERR_SYSTEM);
	}

	while(fgets(buf, sizeof(buf) -1, file)){
		if (buf[0] != '#'){
			break;
		}else{
			fprintf(tmp_file, buf);
		}
	}	
	for(x = 0; x < vcivpi_num; x++){
		fprintf(tmp_file, "%d %d\n", 
				vcivpi_list[x].vci, 
				vcivpi_list[x].vpi);
	}
	fclose(file);
	fclose(tmp_file);
	rename(tmp_adsl_file, adsl_file);
	return;
}


void read_adsl_vci_vpi_list(wan_adsl_vcivpi_t* vcivpi_list, unsigned short* vcivpi_num)
{
	FILE*		file = NULL;
	char		buf[256];
	int		num = 0;
	int 	vci, vpi;

	file = fopen(adsl_file, "r");
	if (file == NULL){
		fprintf( stderr, "%s: cannot open %s\n",
				prognamed, adsl_file);
		show_error(ERR_SYSTEM);
		exit(ERR_SYSTEM);
	}
	
	while(fgets(buf, sizeof(buf) -1, file)){
		if (buf[0] != '#'){
			sscanf(buf, "%d %d", &vci, &vpi);
			vcivpi_list[num].vci = (unsigned short)vci;
			vcivpi_list[num].vpi = (unsigned char)vpi;
			num++;
			if (num >= 100){
				break;
			}
		}
	}	
	*vcivpi_num = num;
	fclose(file);
	return;
}


/*============================================================================
 * TE1
 * Parse active channel string.
 *
 * Return ULONG value, that include 1 in position `i` if channels i is active.
 */
unsigned int parse_active_channel(char* val)
{
#define SINGLE_CHANNEL	0x2
#define RANGE_CHANNEL	0x1
	int channel_flag = 0;
	char* ptr = val;
	int channel = 0, start_channel = 0;
	unsigned int tmp = 0;

	if (strcmp(val,"ALL") == 0)
		return ENABLE_ALL_CHANNELS;
	
	if (strcmp(val,"0") == 0)
		return 0;

	while(*ptr != '\0') {
       		printf("\nMAP DIGIT %c\n", *ptr);
		if (isdigit(*ptr)) {
			channel = strtoul(ptr, &ptr, 10);
			channel_flag |= SINGLE_CHANNEL;
		} else {
			if (*ptr == '-') {
				channel_flag |= RANGE_CHANNEL;
				start_channel = channel;
			} else {
				tmp |= get_active_channels(channel_flag, start_channel, channel);
				channel_flag = 0;
			}
			ptr++;
		}
	}
	if (channel_flag){
		tmp |= get_active_channels(channel_flag, start_channel, channel);
	}

	return tmp;
}

/*============================================================================
 * TE1
 */
unsigned int get_active_channels(int channel_flag, int start_channel, int stop_channel)
{
	int i = 0;
	unsigned int tmp = 0, mask = 0;

	/* If the channel map is set to 0 then
	 * stop_channel will be zero. In this case just return
	 * 0 */
	if (stop_channel < 1) {
        	return 0;
	}

	if ((channel_flag & (SINGLE_CHANNEL | RANGE_CHANNEL)) == 0)
		return tmp;
	if (channel_flag & RANGE_CHANNEL) { /* Range of channels */
		for(i = start_channel; i <= stop_channel; i++) {
			mask = 1 << (i - 1);
			tmp |=mask;
		}
	} else { /* Single channel */ 
		mask = 1 << (stop_channel - 1);
		tmp |= mask; 
	}
	return tmp;
}

#if defined(WAN_HWEC)	

#define WAN_EC_PID	"/etc/wanpipe/wan_ec/wan_ec_pid"
#define WAN_EC_DIR      "/etc/wanpipe/wan_ec"

static int config_hwec(char *devname)
{
	int	status;
	char	cmd[100];
	DIR 	*dir;

	dir = opendir(WAN_EC_DIR);

        if(dir == NULL) {
        	return 0;
        }

	closedir(dir);

	/*HW_EC*/
	sprintf(cmd, "wan_ec_client %s config", devname);
	status = system(cmd);

	if (WEXITSTATUS(status) != 0){
		fprintf(stderr,
		"wanconfig: Failed to configure EC device %s (err=%d)!\n",
				devname,WEXITSTATUS(status));
		return -EINVAL;
	}

	return 0;
}

static int enable_hwec(char *devname, char *ifname, char *channel_list)
{
	int	status;
	char	cmd[100];
	DIR 	*dir;

return 0;
	
	dir = opendir(WAN_EC_DIR);

        if(dir == NULL) {
        	return 0;
        }
	closedir(dir);
	
#if 1
	if (strcasecmp(channel_list, "all") == 0){
		sprintf(cmd, "wan_ec_client %s mn all",
					devname);
	}else{
		sprintf(cmd, "wan_ec_client %s mn %s",
					devname,channel_list);
	}
#else
	if (strcasecmp(channel_list, "all") == 0){
		sprintf(cmd, "wan_ec_client %s enable all",
					devname);
	}else{
		sprintf(cmd, "wan_ec_client %s enable %s",
					devname,channel_list);
	}
#endif

	status = system(cmd);

	if (WEXITSTATUS(status) != 0){
		fprintf(stderr,
		"wanconfig: Failed to enable EC device %s : channels %s (err=%d)!\n",
				devname, channel_list,WEXITSTATUS(status));
		return -EINVAL;
	}
	return 0;
}

static int release_hwec(char *devname)
{
	int	status;
	char	cmd[100];
	
       	sprintf(cmd, "wan_ec_client %s release",
				devname);
	status = system(cmd);
	if (WEXITSTATUS(status) != 0){
		fprintf(stderr,
		"wanconfig: Failed to release EC device %s (err=%d)!\n",
				devname,WEXITSTATUS(status));
		return -EINVAL;
	}
	return 0;
}
#endif

//****** End *****************************************************************/

