/*****************************************************************************
* aftpipemon.c	AFT Debugger/Monitor
*
* Author:       Nenad Corbic <ncorbic@sangoma.com>	
*
* Copyright:	(c) 2004 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ----------------------------------------------------------------------------
* Jan 06, 2004	Nenad Corbic	Initial version based on aftpipemon
*****************************************************************************/

/******************************************************************************
 * 			INCLUDE FILES					      *
 *****************************************************************************/

#if defined(__WINDOWS__)
# include <conio.h>				/* for _kbhit */
# include "wanpipe_includes.h"
# include "wanpipe_defines.h"	/* for 'wan_udp_hdr_t' */
# include "wanpipe_time.h"		/* for 'struct wan_timeval' */
# include "wanpipe_common.h"		/* for 'wp_strcasecmp' */
#else
# include <stdio.h>
# include <stddef.h>	/* offsetof(), etc. */
# include <ctype.h>
# include <unistd.h>
# include <string.h>
# include <stdlib.h>
# include <time.h>
#if defined(__FreeBSD__)
# include <limits.h>
#endif
# include <sys/time.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <arpa/inet.h>
# include <net/if.h>
# include <netinet/in_systm.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/udp.h>
#if defined(__LINUX__)
# include <linux/version.h>
# include <linux/types.h>
# include <linux/if_packet.h>
# include <linux/if_wanpipe.h>
# include <linux/if_ether.h>
#endif
#endif

#include "wanpipe_api.h"

#include "fe_lib.h"
#include "wanpipemon.h"


/******************************************************************************
 * 			DEFINES/MACROS					      *
 *****************************************************************************/
#if LINUX_VERSION_CODE >= 0x020100
#define LINUX_2_1
#endif

#define TIMEOUT 1
#define MDATALEN MAX_LGTH_UDP_MGNT_PKT
#define MAX_TRACE_BUF ((MDATALEN+200)*2)

#define BANNER(str)  banner(str,0)
/******************************************************************************
 * 			TYPEDEF/STRUCTURE				      *
 *****************************************************************************/
/* Structures for data casting */



/******************************************************************************
 * 			GLOBAL VARIABLES				      *
 *****************************************************************************/
/* The ft1_lib needs these global variables */
extern wan_femedia_t	femedia;
extern char *csudsu_menu_te1_pmc[];
extern char *csudsu_menu_te1_dm[];
extern char *csudsu_menu_te3[];

/******************************************************************************
 * 			FUNCTION PROTOTYPES				      *
 *****************************************************************************/
/* Command routines */
static void modem( void );
#if 0
static void global_stats( void );
#endif
static void comm_err_stats( void );
static void read_code_version( void );
static void link_status( void );
static void operational_stats( void );
static void line_trace( int );
static void aft_router_up_time( void );
static void flush_operational_stats( void );
static void flush_comm_err_stats( void );
static void read_ft1_te1_56k_config( void );

static int aft_read_hwec_status(void);

static int aft_remora_tones(int);
static int aft_remora_ring(int);
static int aft_remora_regdump(int);
static int aft_fe_reg_bit(int, int, int);

static wp_trace_output_iface_t trace_iface;

static char *gui_main_menu[]={
"aft_card_stats_menu","Card Status",
"aft_stats_menu", "Card Statistics",
"aft_trace_menu", "Trace Data",
"csudsu_menu", "CSU DSU Config/Stats",
"aft_flush_menu","Flush Statistics",
"."
};


static char *aft_card_stats_menu[]={
"xm","Modem Status",
"xl","Link Status",
"xcv","Read Code Version",
"xru","Display Router UP time",
"."
};
	
static char *aft_stats_menu[]={
"sc","Communication Error Statistics",
"so","Operational Statistics",
"."
};

static char *aft_trace_menu[]={
"tr","Raw Hex trace",
"ti","Interpreted trace",
"."
};

static char *aft_flush_menu[]={
"fc","Flush Communication Error Statistics",
"fo","Flush Operational Statistics",
"fpm","Flush T1/E1 performance monitoring cnters",
"."
};

static struct cmd_menu_lookup_t gui_cmd_menu_lookup[]={
	{"aft_card_stats_menu",aft_card_stats_menu},
	{"aft_stats_menu",aft_stats_menu},
	{"aft_trace_menu",aft_trace_menu},
	{"csudsu_menu",csudsu_menu},
	{"aft_flush_menu",aft_flush_menu},
	{".",NULL}
};


char ** AFTget_main_menu(int *len)
{
	int i=0;
	while(strcmp(gui_main_menu[i],".") != 0){
		i++;
	}
	*len=i/2;
	return gui_main_menu;
}

char ** AFTget_cmd_menu(char *cmd_name,int *len)
{
	int i=0,j=0;
	char **cmd_menu=NULL;
	
	if (strcmp(cmd_name, "csudsu_menu") == 0){

		if (femedia.media == WAN_MEDIA_T1 || femedia.media == WAN_MEDIA_E1){  
			if (femedia.chip_id == WAN_TE_CHIP_PMC){
				cmd_menu = csudsu_menu_te1_pmc;
			}else if (femedia.chip_id == WAN_TE_CHIP_DM){
				cmd_menu = csudsu_menu_te1_dm;
			}else{
				cmd_menu = csudsu_menu;
			}
		}else if (femedia.media == WAN_MEDIA_DS3 || femedia.media == WAN_MEDIA_E3){  
			cmd_menu = csudsu_menu_te3;
		}else{
			cmd_menu = csudsu_menu;
		}
	}else{
		while(gui_cmd_menu_lookup[i].cmd_menu_ptr != NULL){
			if (strcmp(cmd_name,gui_cmd_menu_lookup[i].cmd_menu_name) == 0){
				cmd_menu=(char**)gui_cmd_menu_lookup[i].cmd_menu_ptr;
				break;
			}
			i++;
		}
	}
	if (cmd_menu){
		while (strcmp(cmd_menu[j],".") != 0){
			j++;
		}
	}
	*len=j/2;
	return cmd_menu;
}



/******************************************************************************
 * 			FUNCTION DEFINITION				      *
 *****************************************************************************/
int AFTConfig(void)
{
	char codeversion[10];
	unsigned char x=0;
   
	protocol_cb_size = sizeof(wan_mgmt_t) + 
			   sizeof(wan_cmd_t) + 
			   sizeof(wan_trace_info_t) + 1;
	wan_udp.wan_udphdr_command= READ_CONFIGURATION;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	while (++x < 4) {
		DO_COMMAND(wan_udp); 
		if (wan_udp.wan_udphdr_return_code == 0x00){
			break;
		}
		if (wan_udp.wan_udphdr_return_code == 0xaa){
			printf("Error: Command timeout occurred\n"); 
			return(WAN_FALSE);
		}

		if (wan_udp.wan_udphdr_return_code == 0xCC){ 
			return(WAN_FALSE);
		}
		wan_udp.wan_udphdr_return_code = 0xaa;
	}

	if (x >= 4) return(WAN_FALSE);

	is_508 = WAN_FALSE;
   
	wp_strlcpy(codeversion, "?.??",10);
   
	wan_udp.wan_udphdr_command = READ_CODE_VERSION;
	wan_udp.wan_udphdr_data_len = 0;
	wan_udp.wan_udphdr_return_code = 0xaa;
	DO_COMMAND(wan_udp);
	if (wan_udp.wan_udphdr_return_code == 0) {
		wan_udp.wan_udphdr_data[wan_udp.wan_udphdr_data_len] = 0;
		wp_strlcpy(codeversion, (char*)wan_udp.wan_udphdr_data,10);
	}
	
	return(WAN_TRUE);
}; 


#if 0
static void global_stats (void)
{
	GLOBAL_STATS_STRUCT* global_stats;
	wan_udp.wan_udphdr_command= READ_GLOBAL_STATISTICS; 
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code == 0 ) {
		BANNER("GLOBAL STATISTICS");
		global_stats = (GLOBAL_STATS_STRUCT *)&wan_udp.wan_udphdr_data[0];
		printf("Times application did not respond to IRQ: %u",
			global_stats->app_IRQ_timeout_count);
	} else {
		error();
	}
};  /* global stats */
#endif

static void modem( void )
{
#if 1
	unsigned char cts_dcd;
	wan_udp.wan_udphdr_command= AFT_MODEM_STATUS; 
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code == 0 ) {
		BANNER("MODEM STATUS");
		memcpy(&cts_dcd, &wan_udp.wan_udphdr_data[0],1);
		printf("DCD: ");
		(cts_dcd & 0x08) ? printf("High\n") : printf("Low\n");
		printf("CTS: ");
		(cts_dcd & 0x20) ? printf("High\n") : printf("Low\n");
	}
#endif
}; /* modem */

#ifdef __WINDOWS__
int get_if_operational_stats()
{
	/*! API command structure used to execute API commands. 
	This command structure is used with libsangoma library */
	wanpipe_api_t wp_api;
	net_device_stats_t stats;
	int err;
	unsigned char firm_ver, cpld_ver;

	err=sangoma_get_stats(sock, &wp_api, &stats);
	if (err) {
		return 1;
	}

	printf( "******* OPERATIONAL_STATS *******\n");

	printf("\trx_packets\t: %u\n",			stats.rx_packets);
	printf("\ttx_packets\t: %u\n",			stats.tx_packets);
	printf("\trx_bytes\t: %u\n",			stats.rx_bytes);
	printf("\ttx_bytes\t: %u\n",			stats.tx_bytes);
	printf("\trx_errors\t: %u\n",			stats.rx_errors);
	printf("\ttx_errors\t: %u\n",			stats.tx_errors);
	printf("\trx_dropped\t: %u\n",			stats.rx_dropped);
	printf("\ttx_dropped\t: %u\n",			stats.tx_dropped);
	printf("\tmulticast\t: %u\n",			stats.multicast);
	printf("\tcollisions\t: %u\n",			stats.collisions);

	printf("\trx_length_errors: %u\n",		stats.rx_length_errors);
	printf("\trx_over_errors\t: %u\n",		stats.rx_over_errors);
	printf("\trx_crc_errors\t: %u\n",		stats.rx_crc_errors);
	printf("\trx_frame_errors\t: %u\n",	stats.rx_frame_errors);
	printf("\trx_fifo_errors\t: %u\n",		stats.rx_fifo_errors);
	printf("\trx_missed_errors: %u\n",		stats.rx_missed_errors);

	printf("\ttx_aborted_errors: %u\n",	stats.tx_aborted_errors);
	printf("\tTx Idle Data\t: %u\n",		stats.tx_carrier_errors);

	printf("\ttx_fifo_errors\t: %u\n",		stats.tx_fifo_errors);
	printf("\ttx_heartbeat_errors: %u\n",	stats.tx_heartbeat_errors);
	printf("\ttx_window_errors: %u\n",		stats.tx_window_errors);

	printf("\n\ttx_packets_in_q: %u\n",	stats.current_number_of_frames_in_tx_queue);
	printf("\ttx_queue_size: %u\n",		stats.max_tx_queue_length);

	printf("\n\trx_packets_in_q: %u\n",	stats.current_number_of_frames_in_rx_queue);
	printf("\trx_queue_size: %u\n",		stats.max_rx_queue_length);

	printf("\n\trx_events_in_q: %u\n",	stats.current_number_of_events_in_event_queue);
	printf("\trx_event_queue_size: %u\n",		stats.max_event_queue_length);
	printf("\trx_events: %u\n",	stats.rx_events);
	printf("\trx_events_dropped: %u\n",		stats.rx_events_dropped);

	printf("\tHWEC tone (DTMF) events counter: %u\n",		stats.rx_events_tone);
	printf( "*********************************\n");

	err=sangoma_get_driver_version(sock,&wp_api, NULL);
	if (err) {
		return 1;
	}
	printf("\tDriver Version: %u.%u.%u.%u\n",
				wp_api.wp_cmd.version.major,
				wp_api.wp_cmd.version.minor,
				wp_api.wp_cmd.version.minor1,
				wp_api.wp_cmd.version.minor2);

	err=sangoma_get_firmware_version(sock, &wp_api, &firm_ver);
	if (err) {
		return 1;
	}
	printf("\tFirmware Version: %X\n",
				firm_ver);

	err=sangoma_get_cpld_version(sock, &wp_api, &cpld_ver);
	if (err) {
		return 1;
	}
	printf("\tCPLD Version: %X\n",
				cpld_ver);
	return 0;
}

int flush_if_operational_stats()
{
	/*! API command structure used to execute API commands. 
	This command structure is used with libsangoma library */
	wanpipe_api_t wp_api;

    return sangoma_flush_stats(sock, &wp_api);
}
#endif

static void comm_err_stats (void)
{
#ifdef __WINDOWS__
	/* use libsangoma api to get statistics */
	get_if_operational_stats();
#else
	aft_comm_err_stats_t* comm_err_stats;
	wan_udp.wan_udphdr_command= READ_COMMS_ERROR_STATS; 
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code == 0 ) {

		BANNER("AFT COMMUNICATION ERROR STATISTICS");

		comm_err_stats = (aft_comm_err_stats_t *)&wan_udp.wan_udphdr_data[0];

		printf("RX Stats:\n");
		printf("                        Number of receiver overrun errors:  %u\n", 
				comm_err_stats->Rx_overrun_err_count);
		printf("                            Number of receiver CRC errors:  %u\n", 
				comm_err_stats->Rx_crc_err_count);
		printf("                          Number of receiver Abort errors:  %u\n", 
				comm_err_stats->Rx_abort_count);
		printf("                     Number of receiver corruption errors:  %u\n", 
				comm_err_stats->Rx_hdlc_corrupiton);
		printf("                            Number of receiver PCI errors:  %u\n", 
				comm_err_stats->Rx_pci_errors);
		printf("                 Number of receiver DMA descriptor errors:  %u\n", 
				comm_err_stats->Rx_dma_descr_err);
		printf("TX Stats:\n");
		printf("                         Number of transmitter PCI errors:  %u\n", 
				comm_err_stats->Tx_pci_errors);
		printf("               Number of transmitter PCI latency warnings:  %u\n", 
				comm_err_stats->Tx_pci_latency);
		printf("              Number of transmitter DMA descriptor errors:  %u\n", 
				comm_err_stats->Tx_dma_errors);
		printf("       Number of transmitter DMA descriptor length errors:  %u\n", 
				comm_err_stats->Tx_dma_len_nonzero);

#if 0
		printf("                        Number of times DCD changed state:  %u\n", 
				comm_err_stats->DCD_state_change_count);
		printf("                        Number of times CTS changed state:  %u\n", 
				comm_err_stats->CTS_state_change_count);
#endif
	}
#endif

}; /* comm_err_stats */


static void flush_comm_err_stats( void ) 
{
#ifdef __WINDOWS__
	/* use libsangoma api to get statistics */
	flush_if_operational_stats();
#else
	wan_udp.wan_udphdr_command=FLUSH_COMMS_ERROR_STATS;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);

#endif
}; /* flush_comm_err_stats */


static void read_code_version (void)
{
	wan_udp.wan_udphdr_command= READ_CODE_VERSION; 
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code == 0) {
		BANNER("AFT CODE VERSION");
		printf("Code version: HDLC rev.%X\n",
				wan_udp.wan_udphdr_data[0]);
	}else{
		printf("Error: Rc=%x\n",wan_udp.wan_udphdr_return_code);
	}
}; /* read code version */


static void link_status (void)
{
	wan_udp.wan_udphdr_command= AFT_LINK_STATUS; 
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);
	
	BANNER("AFT LINK STATUS");
	
	if (wan_udp.wan_udphdr_return_code == 0){
		printf("Device Link Status:	%s\n", 
			STATE_DECODE(wan_udp.wan_udphdr_data[0]));
	}
	
	return;
}; /* Link Status */		


static void operational_stats (void)
{
	aft_op_stats_t *stats;
	wan_udp.wan_udphdr_command= READ_OPERATIONAL_STATS; 
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code == 0) {
		BANNER("AFT OPERATIONAL STATISTICS");
		stats = (aft_op_stats_t *)&wan_udp.wan_udphdr_data[0];
 
		printf(    "             Number of frames transmitted:   %u",
				stats->Data_frames_Tx_count);
		printf(  "\n              Number of bytes transmitted:   %u",
				stats->Data_bytes_Tx_count);
		printf(  "\n                      Transmit Throughput:   %u",
				stats->Data_Tx_throughput);
		printf(  "\n Transmit frames discarded (length error):   %u",
				stats->Tx_Data_discard_lgth_err_count);
		printf(  "\n                Transmit frames realigned:   %u",
				stats->Data_frames_Tx_realign_count);


		printf("\n\n                Number of frames received:   %u",
				stats->Data_frames_Rx_count);
		printf(  "\n                 Number of bytes received:   %u",
				stats->Data_bytes_Rx_count);
		printf(  "\n                       Receive Throughput:   %u",
				stats->Data_Rx_throughput);
		printf(  "\n    Received frames discarded (too short):   %u",
				stats->Rx_Data_discard_short_count);
		printf(  "\n     Received frames discarded (too long):   %u",
				stats->Rx_Data_discard_long_count);
		printf(  "\nReceived frames discarded (link inactive):   %u",
				stats->Rx_Data_discard_inactive_count);


		printf("\n\nHDLC link active/inactive and loopback statistics");
		printf(  "\n                           Times that the link went active:   %u", stats->link_active_count);	
		printf(  "\n         Times that the link went inactive (modem failure):   %u", stats->link_inactive_modem_count);
		printf(  "\n     Times that the link went inactive (keepalive failure):   %u", stats->link_inactive_keepalive_count);
		printf(  "\n                                         link looped count:   %u", stats->link_looped_count);


	} 
}; /* Operational_stats */


static void flush_operational_stats( void ) 
{
#if 1
	wan_udp.wan_udphdr_command= FLUSH_OPERATIONAL_STATS;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);
#endif
}; /* flush_operational_stats */

static void wp_span_debugging( unsigned char toggle ) 
{
	wan_udp.wan_udphdr_command= WANPIPEMON_CHAN_SEQ_DEBUGGING;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 1;
	wan_udp.wan_udphdr_data[0] = toggle;
	DO_COMMAND(wan_udp);
}; /* flush_operational_stats */

static void wp_gen_fifo_debugging( unsigned char toggle )
{
	if (toggle) {
		wan_udp.wan_udphdr_command= WANPIPEMON_GEN_FIFO_ERR_RX;
	} else {
		wan_udp.wan_udphdr_command= WANPIPEMON_GEN_FIFO_ERR_TX;
	}
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 1;
	wan_udp.wan_udphdr_data[0] = 0;
	DO_COMMAND(wan_udp);
}; /* flush_operational_stats */

static void wp_get_fifo_sync_cnt(void)
{
	wan_udp.wan_udphdr_command= WANPIPEMON_GEN_FE_SYNC_ERR;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 1;
	wan_udp.wan_udphdr_data[0] = 0;
	DO_COMMAND(wan_udp);
}; /* flush_operational_stats */


int AFTDisableTrace(void)
{
	wan_udp.wan_udphdr_command= DISABLE_TRACING;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);
	return 0;
}

static int print_local_time (char *date_string, int max_len)
{
	
  	char tmp_time[50];
	time_t time_val;
	struct tm *time_tm;
	
	date_string[0]='\0';

	time_val=time(NULL);
		
	/* Parse time and date */
	time_tm=localtime(&time_val);

	strftime(tmp_time,sizeof(tmp_time),"%b",time_tm);
	snprintf(date_string, max_len, " %s ",tmp_time);

	strftime(tmp_time,sizeof(tmp_time),"%d",time_tm);
	snprintf(date_string+strlen(date_string), max_len-strlen(date_string), "%s ",tmp_time);

	strftime(tmp_time,sizeof(tmp_time),"%H",time_tm);
	snprintf(date_string+strlen(date_string), max_len-strlen(date_string), "%s:",tmp_time);

	strftime(tmp_time,sizeof(tmp_time),"%M",time_tm);
	snprintf(date_string+strlen(date_string), max_len-strlen(date_string), "%s:",tmp_time);

	strftime(tmp_time,sizeof(tmp_time),"%S",time_tm);
	snprintf(date_string+strlen(date_string), max_len-strlen(date_string), "%s",tmp_time);

	return 0;
}

static int loop_rx_data(int passnum)
{
	unsigned int num_frames;
	unsigned short curr_pos = 0;
	wan_trace_pkt_t *trace_pkt;
	unsigned int i;
	struct wan_timeval to;
	int timeout=0;
	char date_string[100];
	
	wp_gettimeofday(&to, NULL);
	to.tv_sec = 0;
	to.tv_usec = 0;
	
	print_local_time(date_string,100);
		
	printf("%s | Test %04i | ",
		date_string, passnum);
	
        for(;;) {
	
#ifdef __WINDOWS__
		if(to.tv_usec){
			wp_usleep(to.tv_usec);
		}
#else
		select(1,NULL, NULL, NULL, &to);
#endif
		
		wan_udp.wan_udphdr_command = GET_TRACE_INFO;
		wan_udp.wan_udphdr_return_code = 0xaa;
		wan_udp.wan_udphdr_data_len = 0;
		DO_COMMAND(wan_udp);
		
		if (wan_udp.wan_udphdr_return_code == 0 && wan_udp.wan_udphdr_data_len) { 
		     
			num_frames = wan_udp.wan_udphdr_aft_num_frames;

		     	for ( i = 0; i < num_frames; i++) {
				trace_pkt= (wan_trace_pkt_t *)&wan_udp.wan_udphdr_data[curr_pos];

				/*  frame type */
				if (trace_pkt->status & 0x01) {
					trace_iface.status |= WP_TRACE_OUTGOING;
				}else{
					if (trace_pkt->status & 0x10) { 
						trace_iface.status |= WP_TRACE_ABORT;
					} else if (trace_pkt->status & 0x20) {
						trace_iface.status |= WP_TRACE_CRC;
					} else if (trace_pkt->status & 0x40) {
						trace_iface.status |= WP_TRACE_OVERRUN;
					}
				}

				trace_iface.len = trace_pkt->real_length;
				trace_iface.timestamp=trace_pkt->time_stamp;
				trace_iface.sec = trace_pkt->sec;
				trace_iface.usec = trace_pkt->usec;
				
				curr_pos += sizeof(wan_trace_pkt_t);
		
				if (trace_pkt->real_length >= WAN_MAX_DATA_SIZE){
					printf("\t:the frame data is to big (%u)!",
									trace_pkt->real_length);
					fflush(stdout);
					continue;

				}else if (trace_pkt->data_avail == 0) {

					printf("\t: the frame data is not available" );
					fflush(stdout);
					continue;
				} 

				
				/* update curr_pos again */
				curr_pos += trace_pkt->real_length;
			
				trace_iface.trace_all_data=trace_all_data;
				trace_iface.data=(unsigned char*)&trace_pkt->data[0];

				
				if (trace_pkt->status & 0x01){ 
					continue;
				}
				
				if (trace_iface.data[0] == (0xFF-0) &&
				    trace_iface.data[1] == (0xFF-1) &&
				    trace_iface.data[2] == (0xFF-2) &&
				    trace_iface.data[3] == (0xFF-3)) {
				 	printf("Successful (%s)!\n",
						trace_iface.status & WP_TRACE_ABORT ? "Abort" :
						trace_iface.status & WP_TRACE_CRC ? "Crc" :
						trace_iface.status & WP_TRACE_OVERRUN ? "Overrun" : "Ok"
						);   
					return 0;
				} 
				
				

		   	} //for
		} //if
		curr_pos = 0;

		if (!wan_udp.wan_udphdr_chdlc_ismoredata){
			to.tv_sec = 0;
			to.tv_usec = WAN_TRACE_DELAY;
			timeout++;
			if (timeout > 100) {
				printf("Timeout!\n");
				break;
			}   
		}else{
			to.tv_sec = 0;
			to.tv_usec = 0;
		}
	}
	return 0;
}

		
static int aft_digital_loop_test( void )
{
	int passnum=0;

	/* Disable trace to ensure that the buffers are flushed */
	wan_udp.wan_udphdr_command= DISABLE_TRACING;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);
	
	wan_udp.wan_udphdr_command= ENABLE_TRACING;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 1;
	wan_udp.wan_udphdr_data[0]=0;
	
	DO_COMMAND(wan_udp);
	if (wan_udp.wan_udphdr_return_code != 0 && 
		wan_udp.wan_udphdr_return_code != 1) {
		printf("Error: Failed to start loop test: failed to start tracing!\n");
		return -1;
	}
	
	printf("Starting Loop Test (press ctrl-c to exit)!\n\n");
	
	while (1) {

		memset(&wan_udp, 0x00, sizeof(wan_udp));

		wan_udp.wan_udphdr_command= DIGITAL_LOOPTEST;
		wan_udp.wan_udphdr_return_code = 0xaa;
		wan_udp.wan_udphdr_data[0] = 0xFF-0;
		wan_udp.wan_udphdr_data[1] = 0xFF-1;
		wan_udp.wan_udphdr_data[2] = 0xFF-2;
		wan_udp.wan_udphdr_data[3] = 0xFF-3;
		wan_udp.wan_udphdr_data_len = 100;
		DO_COMMAND(wan_udp);	

		switch (wan_udp.wan_udphdr_return_code) {
			
		case 0:
			break;
		case 1:
			printf("Error: Failed to start loop test: dev not found\n");
			goto loop_rx_exit;
		case 2:
			printf("Error: Failed to start loop test: dev state not connected\n");
			goto loop_rx_exit;
		case 3:
			printf("Error: Failed to start loop test: memory error\n");
			goto loop_rx_exit;
		case 4:
			printf("Error: Failed to start loop test: invalid operation mode\n");
			goto loop_rx_exit;
			
		default:
#if defined(__WINDOWS__)
		printf("Error: Failed to start loop test: %s (%i)\n",
				SDLA_DECODE_SANG_STATUS(wan_udp.wan_udphdr_return_code),
				wan_udp.wan_udphdr_return_code);
#else
			printf("Error: Failed to start loop test: unknown error (%i)\n",
				wan_udp.wan_udphdr_return_code);
#endif
			goto loop_rx_exit;
		}
		
		loop_rx_data(++passnum);
		wp_usleep(500000);
		fflush(stdout);
		
		if (passnum >= 10) {
			break;
		}		
	}
	
loop_rx_exit:
	
	wan_udp.wan_udphdr_command= DISABLE_TRACING;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);
	
	return 0;
}

extern int mtp2_msu_only;
extern wanpipe_hdlc_engine_t *rx_hdlc_eng;  
wp_trace_output_iface_t hdlc_trace_iface;

static int trace_aft_hdlc_data(wanpipe_hdlc_engine_t *hdlc_eng, void *data, int len)
{
	char *frame = (char*)data;
	hdlc_trace_iface.data=data;
	hdlc_trace_iface.len=len;

	hdlc_trace_iface.status		= trace_iface.status;
	hdlc_trace_iface.timestamp 	= trace_iface.timestamp;
	hdlc_trace_iface.sec		= trace_iface.sec;
	hdlc_trace_iface.usec		= trace_iface.usec;

	if (mtp2_msu_only) {
		if (frame[2] < 3) {
			return 1;
		}
	}			

	if (pcap_output){
		hdlc_trace_iface.type=WP_OUT_TRACE_PCAP;
	} else {
		hdlc_trace_iface.type=WP_OUT_TRACE_RAW;
	}

	hdlc_trace_iface.link_type=wan_protocol;

	wp_trace_output(&hdlc_trace_iface);

	fflush(stdout);

	return 0;	
}

#ifdef __WINDOWS__
int wanpipe_hdlc_decode (wanpipe_hdlc_engine_t *hdlc_eng, 
			 	unsigned char *buf, int len)
{
	WIN_DBG_FUNC_NOT_IMPLEMENTED();
	return 1;
}
#endif

static void line_trace(int trace_mode) 
{
	unsigned int num_frames;
	unsigned short curr_pos = 0;
	wan_trace_pkt_t *trace_pkt;
	unsigned int i;
	int recv_buff = sizeof(wan_udp_hdr_t)+ 100;
	struct wan_timeval to;

#ifndef __WINDOWS__	
	fd_set ready;
	setsockopt( sock, SOL_SOCKET, SO_RCVBUF, &recv_buff, sizeof(int) );
#endif

	/* Disable trace to ensure that the buffers are flushed */
	wan_udp.wan_udphdr_command= DISABLE_TRACING;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);
	
	wan_udp.wan_udphdr_command= ENABLE_TRACING;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 1;
	wan_udp.wan_udphdr_data[0]=trace_mode;
	
	DO_COMMAND(wan_udp);
	
	if (wan_udp.wan_udphdr_return_code == 0) { 
		printf("Starting trace...(Press ENTER to exit)\n");
		fflush(stdout);
	} else if(wan_udp.wan_udphdr_return_code == 0xCD ) {
		printf("Cannot Enable Line Tracing from Underneath.\n");
		fflush(stdout);
		return;
	}else if (wan_udp.wan_udphdr_return_code == 0x01 ) {
		printf("Starting trace...(although it's already enabled!)\n");
		printf("Press ENTER to exit.\n");
		fflush(stdout);
	}else{
		printf("Failed to Enable Line Tracing. Return code: 0x%02X\n", 
			wan_udp.wan_udphdr_return_code );
		fflush(stdout);
		return;
	}
	
	to.tv_sec = 0;
	to.tv_usec = 0;
	for(;;) {
		
#ifdef __WINDOWS__
		if(to.tv_usec){
			wp_usleep(to.tv_usec);
		}
		if(_kbhit()){
			if(_getch() == 0x0D){
				/* Enter key was pressed */
				break;
			}else{
				printf("Invalid key... (Press ENTER to exit)\n");
				fflush(stdout);
			}
		}
#else
		FD_ZERO(&ready);
		FD_SET(0,&ready);

		if(select(1,&ready, NULL, NULL, &to)) {
			break;
		} /* if */
#endif
		wan_udp.wan_udphdr_command = GET_TRACE_INFO;
		wan_udp.wan_udphdr_return_code = 0xaa;
		wan_udp.wan_udphdr_data_len = 0;
		DO_COMMAND(wan_udp);

		if (wan_udp.wan_udphdr_return_code == 0 && wan_udp.wan_udphdr_data_len) { 
			
			num_frames = wan_udp.wan_udphdr_aft_num_frames;
			
			for ( i = 0; i < num_frames; i++) {
				trace_pkt= (wan_trace_pkt_t *)&wan_udp.wan_udphdr_data[curr_pos];
				
				/*  frame type */
				trace_iface.status=0;
				if (trace_pkt->status & 0x01) {
					trace_iface.status |= WP_TRACE_OUTGOING;
				}else{
					if (trace_pkt->status & 0x10) { 
						trace_iface.status |= WP_TRACE_ABORT;
					} else if (trace_pkt->status & 0x20) {
						trace_iface.status |= WP_TRACE_CRC;
					} else if (trace_pkt->status & 0x40) {
						trace_iface.status |= WP_TRACE_OVERRUN;
					}
				}
				
#if 0
				if (trace_pkt->real_length != 4800){
					printf("Trace Len = %i Num frames =%i  Current=%i status=0x%X\n",trace_pkt->real_length,num_frames,i, trace_pkt->status);
					continue;
				}
#endif
				
				trace_iface.len = trace_pkt->real_length;
				trace_iface.timestamp=trace_pkt->time_stamp;
				trace_iface.sec = trace_pkt->sec;
				trace_iface.usec = trace_pkt->usec;
				
				hdlc_trace_iface.status		= trace_iface.status;
				hdlc_trace_iface.timestamp 	= trace_iface.timestamp;
				hdlc_trace_iface.sec		= trace_iface.sec;
				hdlc_trace_iface.usec		= trace_iface.usec;
				
				curr_pos += sizeof(wan_trace_pkt_t);
				
				if (trace_pkt->real_length >= WAN_MAX_DATA_SIZE){
					printf("\t:the frame data is to big (%u)!",
						trace_pkt->real_length);
					fflush(stdout);
					continue;
					
				}else if (trace_pkt->data_avail == 0) {
					
					printf("\t: the frame data is not available" );
					fflush(stdout);
					continue;
				} 
				
				
				/* update curr_pos again */
				curr_pos += trace_pkt->real_length;
				
				trace_iface.trace_all_data=trace_all_data;
				trace_iface.data=(unsigned char*)&trace_pkt->data[0];
				
				hdlc_trace_iface.trace_all_data = trace_iface.trace_all_data;
				hdlc_trace_iface.data = trace_iface.data;
				hdlc_trace_iface.len = trace_iface.len;
				
				
				/*
				if (raw_data) {
				trace_iface.type=WP_OUT_TRACE_RAW;
				}else
				*/
				
				if (trace_iface.type == WP_OUT_TRACE_HDLC && rx_hdlc_eng) {
					rx_hdlc_eng->hdlc_data = trace_aft_hdlc_data;
					wanpipe_hdlc_decode(rx_hdlc_eng,trace_iface.data,trace_iface.len);
					continue;		
				} 
				
				if (pcap_output){
					trace_iface.type=WP_OUT_TRACE_PCAP;
				}
				if (trace_binary){
					trace_iface.type=WP_OUT_TRACE_BIN;
				}
				/*
				else{
				trace_iface.type=WP_OUT_TRACE_INTERP;
				}
				*/
				
				trace_iface.link_type=wan_protocol;
				
				wp_trace_output(&trace_iface);
				
				fflush(stdout);
				
			} //for
		} //if
		curr_pos = 0;
		
		if (!wan_udp.wan_udphdr_chdlc_ismoredata){
			to.tv_sec = 0;
			to.tv_usec = WAN_TRACE_DELAY;
		}else{
			to.tv_sec = 0;
			to.tv_usec = 0;
		}
	}
	
	
	wan_udp.wan_udphdr_command= DISABLE_TRACING;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	DO_COMMAND(wan_udp);
}; /* line_trace */

static int aft_read_hwec_status()
{
	u_int32_t	hwec_status;
	int		channel, found = 0;
	
	/* Disable trace to ensure that the buffers are flushed */
	wan_udp.wan_udphdr_command	= AFT_HWEC_STATUS;
	wan_udp.wan_udphdr_return_code	= 0xaa;
	wan_udp.wan_udphdr_data_len	= 0;
	DO_COMMAND(wan_udp);
	
	if (wan_udp.wan_udphdr_return_code) { 
		printf("Failed to get HW Echo Canceller status!\n"); 
		fflush(stdout);
		return 0;
	}
	hwec_status = *(u_int32_t*)&wan_udp.wan_udphdr_data[0];
	for(channel = 0; channel < 32; channel++){
		if (hwec_status & (1 << channel)){
			printf(
				"Sangoma HW Echo Canceller is enabled for channel %d\n",
				channel);
			found = 1;
		}	
	} 
	if (!found){
		printf(
			"Sangoma HW Echo Canceller is disabled for all channels!\n");
	}
	fflush(stdout);
	return 0;
}

static int aft_fe_reg_bit(int is_setbit, int reg, int bit)
 {
 	sdla_fe_debug_t	fe_debug;
 	unsigned char	value = 0;
  
 	fe_debug.type = WAN_FE_DEBUG_REG;
 	fe_debug.fe_debug_reg.reg  = reg;
 	fe_debug.fe_debug_reg.read = 1;
 	set_fe_debug_mode(&fe_debug);
 
 	value = fe_debug.fe_debug_reg.value;
 	if (is_setbit){
 		fe_debug.fe_debug_reg.value |= (1<<bit);
 	}else{
 		fe_debug.fe_debug_reg.value &= ~(1<<bit);
 	}
 	fe_debug.fe_debug_reg.read = 0;
 	set_fe_debug_mode(&fe_debug);
 
 	printf("\t%s bit %d in Register %04X (%02X:%02X)\n", 
 			(is_setbit) ? "Set" : "Clear",
 			bit, fe_debug.fe_debug_reg.reg,
 			value, (unsigned char)fe_debug.fe_debug_reg.value);
 	return 0;
}

static int aft_remora_tones(int mod_no)
{
	int	cnt = 0;
	char	ch;
	wan_remora_udp_t        *rm_udp;

	/* Disable trace to ensure that the buffers are flushed */
	wan_udp.wan_udphdr_command	= WAN_FE_TONES;
	wan_udp.wan_udphdr_return_code	= 0xaa;
	wan_udp.wan_udphdr_data_len	= 2;

	rm_udp = (wan_remora_udp_t *)get_wan_udphdr_data_ptr(0);
	rm_udp->mod_no 	= mod_no;
	rm_udp->type	= 1;

	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code) { 
		printf("Failed to start tone on Module %d!\n", mod_no); 
		fflush(stdout);
		return 0;
	}

	printf("Press enter to stop the tone ...");fflush(stdout);ch=getchar();
tone_stop_again:
	/* Disable A200/A400 Ring event */
	wan_udp.wan_udphdr_command	= WAN_FE_TONES;
	wan_udp.wan_udphdr_return_code	= 0xaa;
	wan_udp.wan_udphdr_data_len	= 2;

	rm_udp = (wan_remora_udp_t *)get_wan_udphdr_data_ptr(0);
	rm_udp->mod_no 	= mod_no;
	rm_udp->type	= 0;

	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code) {
		if (cnt++ > 10){
			wp_sleep(1);
			goto tone_stop_again;
		} 
		printf("Failed to stop tone on Module %d (timeout)!\n",
					mod_no); 
		fflush(stdout);
		return 0;
	}
	return 0;
}

static int aft_remora_ring(int mod_no)
{
	int	cnt=0;
	char	ch;
	wan_remora_udp_t        *rm_udp;

	/* Enable A200/A400 Ring event */
	wan_udp.wan_udphdr_command	= WAN_FE_RING;
	wan_udp.wan_udphdr_return_code	= 0xaa;
	wan_udp.wan_udphdr_data_len	= 2;

	rm_udp = (wan_remora_udp_t *)get_wan_udphdr_data_ptr(0);
	rm_udp->mod_no 	= mod_no;
	rm_udp->type	= 1;

	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code) { 
		printf("Failed to start ring for Module %d!\n", mod_no); 
		fflush(stdout);
		return 0;
	}
	fflush(stdout);

	printf("Press enter to stop the ring ...");fflush(stdout);ch=getchar();
ring_stop_again:
	/* Disable A200/A400 Ring event */
	wan_udp.wan_udphdr_command	= WAN_FE_RING;
	wan_udp.wan_udphdr_return_code	= 0xaa;
	wan_udp.wan_udphdr_data_len	= 2;

	rm_udp = (wan_remora_udp_t *)get_wan_udphdr_data_ptr(0);
	rm_udp->mod_no 	= mod_no;
	rm_udp->type	= 0;

	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code) { 
		if (cnt++ > 10){
			wp_sleep(1);
			goto ring_stop_again;
		} 
		printf("Failed to stop ring for Module %d (timeout)!\n", mod_no); 
		fflush(stdout);
		return 0;
	}
	fflush(stdout);

	return 0;
}

static int aft_remora_regdump(int mod_no)
{
	wan_remora_udp_t	*rm_udp;
	int			reg;

	rm_udp = (wan_remora_udp_t *)get_wan_udphdr_data_ptr(0);
	rm_udp->mod_no = mod_no;
	wan_udp.wan_udphdr_command	= WAN_FE_REGDUMP;
	wan_udp.wan_udphdr_return_code	= 0xaa;
	wan_udp.wan_udphdr_data_len	= sizeof(wan_remora_udp_t);
	DO_COMMAND(wan_udp);

	if (wan_udp.wan_udphdr_return_code || !wan_udp.wan_udphdr_data_len) { 
		printf("Failed to get register dump!\n"); 
		fflush(stdout);
		return 0;
	}
	rm_udp = (wan_remora_udp_t *)get_wan_udphdr_data_ptr(0);
	printf("\t------- Direct registers (%s,port %d) -------\n",
					WP_REMORA_DECODE_TYPE(rm_udp->type),
					rm_udp->mod_no);
	if (rm_udp->type == MOD_TYPE_FXS){

		for(reg = 0; reg < WAN_FXS_NUM_REGS; reg++){
			if (reg % 8 == 0) printf("\n\t");
			printf("%3d. %02X ", reg, rm_udp->u.regs_fxs.direct[reg]);
		}
		printf("\n\t-----------------------------\n");
		printf("\n");
		printf("\t------- Indirect registers (port %d) -------\n",
							rm_udp->mod_no);
		for (reg=0; reg < WAN_FXS_NUM_INDIRECT_REGS; reg++){
			if (reg % 6 == 0) printf("\n\t");
			printf("%3d. %04X ", reg, rm_udp->u.regs_fxs.indirect[reg]);
		}
		printf("\n\t-----------------------------\n");
		printf("\n");
		printf("TIP\t: -%d Volts\n", (rm_udp->u.regs_fxs.direct[80]*376)/1000);
		printf("RING\t: -%d Volts\n", (rm_udp->u.regs_fxs.direct[81]*376)/1000);
		printf("VBAT\t: -%d Volts\n", (rm_udp->u.regs_fxs.direct[82]*376)/1000);
	} else if (rm_udp->type == MOD_TYPE_FXO){

		for(reg = 0; reg < WAN_FXO_NUM_REGS; reg++){
			if (reg % 8 == 0) printf("\n\t");
			printf("%3d. %02X ", reg, rm_udp->u.regs_fxo.direct[reg]);
		}
		printf("\n\t-----------------------------\n");
		printf("\n");
	}

	fflush(stdout);
	return 0;
}

static int aft_remora_stats(int mod_no)
{
	wan_remora_udp_t	*rm_udp;

	rm_udp = (wan_remora_udp_t *)get_wan_udphdr_data_ptr(0);
	rm_udp->mod_no = mod_no;
	wan_udp.wan_udphdr_command	= WAN_FE_STATS;
	wan_udp.wan_udphdr_return_code	= 0xaa;
	wan_udp.wan_udphdr_data_len	= sizeof(wan_remora_udp_t);

	DO_COMMAND(wan_udp);
	
	if (wan_udp.wan_udphdr_return_code || !wan_udp.wan_udphdr_data_len) { 
		printf("Failed to get voltage stats!\n"); 
		fflush(stdout);
		return 0;
	}

	rm_udp = (wan_remora_udp_t *)get_wan_udphdr_data_ptr(0);
	if (rm_udp->type == MOD_TYPE_FXS){
		printf("\t------- Voltage Status  (%s,port %d) -------\n\n",
					WP_REMORA_DECODE_TYPE(rm_udp->type),
					rm_udp->mod_no);
		printf("TIP\t: -%7.4f Volts\n", (float)(rm_udp->u.stats.tip_volt*376)/1000);
		printf("RING\t: -%7.4f Volts\n", (float)(rm_udp->u.stats.ring_volt*376)/1000);
		printf("VBAT\t: -%7.4f Volts\n", (float)(rm_udp->u.stats.bat_volt*376)/1000);
	}else if (rm_udp->type == MOD_TYPE_FXO){
		unsigned char	volt = rm_udp->u.stats.volt;
		printf("\t------- Voltage Status  (%s,port %d) -------\n\n",
					WP_REMORA_DECODE_TYPE(rm_udp->type),
					rm_udp->mod_no);
		if (volt & 0x80){
			volt = ~volt + 1;
		}
		printf("VOLTAGE\t: %d Volts\n", volt);
		printf("\n");
		printf("\t------- Line Status  (%s,port %d) -------\n\n",
					WP_REMORA_DECODE_TYPE(rm_udp->type),
					rm_udp->mod_no);
		printf("Line\t: %s\n", FE_STATUS_DECODE(rm_udp->u.stats.status));
		printf("\n");
	}
	fflush(stdout);
	return 0;
}


static int aft_remora_hook(int mod_no, int offhook)
{
	sdla_fe_debug_t	fe_debug;

	fe_debug.type			= WAN_FE_DEBUG_HOOK;
	fe_debug.mod_no			= mod_no;
	fe_debug.fe_debug_hook.offhook	= offhook;
	aft_remora_debug_mode(&fe_debug);
	return 0;
}

//CORBA
int AFTUsage(void)
{
	printf("wanpipemon: Wanpipe AFT Hardware Level Debugging Utility\n\n");
	printf("Usage:\n");
	printf("-----\n\n");
#ifdef __WINDOWS__
	printf("wanpipemon -i <interface name> -c <command>\n\n");
	printf("\tOption -i: \n");
	printf("\t\tWanpipe interface name (ex: wanpipe1_if1)\n");   
#else
	printf("wanpipemon -i <ip-address or interface name> -u <port> -c <command>\n\n");
	printf("\tOption -i: \n");
	printf("\t\tWanpipe remote IP address must be supplied\n");
	printf("\t\t<or> Wanpipe network interface name (ex: wp1_chdlc)\n");   
	printf("\tOption -u: (Optional, default: 9000)\n");
	printf("\t\tWanpipe UDPPORT specified in /etc/wanpipe#.conf\n");
#endif
	printf("\tOption -full: (Optional, trace option)\n");
	printf("\t\tDisplay raw packets in full: default trace pkt len=25bytes\n");
	printf("\tOption -c: \n");
	printf("\t\tCommand is split into two parts:\n"); 
	printf("\t\t\tFirst letter is a command and the rest are options:\n"); 
	printf("\t\t\tex: xm = View Modem Status\n\n");
	printf("\tSupported Commands: x=status : s=statistics : t=trace \n");
	printf("\t                    c=config : T=FT1 stats  : f=flush\n\n");
	printf("\tCommand:  Options:   Description \n");	
	printf("\t-------------------------------- \n\n");    
	printf("\tCard Status\n");
	printf("\t   x         m       Modem Status\n");
	printf("\t             l       Link Status\n");
	printf("\t             cv      Read Code Version\n");
	printf("\t             ru      Display Router UP time\n");
	printf("\tCard Statistics\n");
	printf("\t   s         c       Communication Error Statistics\n");
	printf("\t             o       Operational Statistics\n");
	printf("\tTrace Data \n");
#ifdef __WINDOWS__
	printf("\t   t         r       Trace data frames. Do not attempt interpret the\n");
	printf("\t                     data.\n");
#else
	printf("\tt         i       Trace and Interpret ALL frames\n");
	printf("\t             r       Trace ALL frames, in RAW format\n");
#endif
	printf("\tT1/E1 Configuration/Statistics\n");
	printf("\t   T  a      Read T1/E1 alarms.\n"); 
	printf("\n");
	printf("\t   ============================================\n");
	printf("\t   ========= T1/E1 Loop Back commands =========\n");
	printf("\n");  
	printf("\t      allb   Activate   Line (Remote) Loopback.\n");  
	printf("\t             (T1/E1; NORMAL Clock)\n");  
	printf("\t      dllb   Deactivate Line (Remote) Loopback.\n");  
	printf("\t             (T1/E1; NORMAL Clock)\n");  
	printf("\n");
	printf("\t      adlb   Activate   Diagnostic (Local) Loopback.\n");  
	printf("\t             (T1/E1; MASTER Clock\n");  
	printf("\t      ddlb   Deactivate Diagnostic (Local) Loopback.\n");  
	printf("\t             (T1/E1; MASTER Clock\n");  
	printf("\n");
	printf("\t      lb     Read LoopBack status (T1/E1). This is the LoopBack\n");
	printf("\t             status of the LOCAL device, not a status of LoopBack\n");
	printf("\t             COMMAND which was transmitted to REMOTE device.\n");
	printf("\t             The status (return code) of any LoopBack COMMAND is\n");  
	printf("\t             displayed on command completion.\n");  
	printf("\n");
	printf("\t   ============================================\n");
	printf("\t   ======= T1 FDL/BOC Loop Back commands ======\n");
	printf("\n"); 
	printf("\t      salb   Send Activate   Loopback (Remote) Code.\n"); 
	printf("\t             (T1; MASTER Clock)\n"); 
	printf("\t      sdlb   Send Deactivate Loopback (Remote) Code.\n");
	printf("\t             (T1; MASTER Clock)\n"); 
	printf("\n");
	printf("\t      saplb  Send Activate   Payload Loopback (Remote) Code.\n");  
	printf("\t             (T1; MASTER Clock)\n"); 
	printf("\t      sdplb  Send Deactivate Payload Loopback (Remote) Code.\n");  
	printf("\t             (T1; MASTER Clock)\n"); 
	printf("\n");
	printf("\t   ============================================\n");
	printf("\t   =============== BERT Commands ==============\n");
	printf("\n");
	printf("\t      sw_bert <command>  BERT test (AFT only)\n");  
	printf("\t             command:\n");  
	printf("\t               --random - start BERT, use random data\n");  
	printf("\t               --ascendant - start BERT, use ascendant data\n");  
	printf("\t               --descendant - start BERT, use descendant data\n");  
	printf("\t               --stop - stop BERT\n");  
	printf("\t               --status - print BERT status/statistics\n");  
	printf("\n");

	printf("\t   ***** DS3/E3 Test modes *****\n");  
	printf("\t      allb3  Activate Analog Loopback mode (DS3/E3 cards)\n");  
	printf("\t      dllb3  Deactivate Analog Loopback mode (DS3/E3 cards)\n");  
	printf("\t      arlb3  Activate Remote Loopback mode (DS3/E3 cards)\n");  
	printf("\t      drlb3  Deactivate Remote Loopback mode (DS3/E3 cards)\n");  
	printf("\t      adlb3  Activate Digital Loopback mode (DS3/E3 cards)\n");  
	printf("\t      ddlb3  Deactivate Digital Loopback mode (DS3/E3 cards)\n");  
	printf("\t   ***** End of DS3/E3 Test modes *****\n");  
	printf("\n");
	printf("\t      txe    Enable TX (AFT card only)\n");  
	printf("\t      txd    Disable TX (AFT card only)\n");  
	printf("\n");

	printf("\tFlush Statistics\n");
	printf("\t   f         c       Flush Communication Error Statistics\n");
	printf("\t             o       Flush Operational Statistics\n");
	printf("\t             pm      Flush T1/E1 performance monitoring counters\n");
	printf("\n");
	printf("\tEcho Canceller Statistics\n");
	printf("\t   e         hw      Read HW Echo Canceller Status\n");
	printf("\tAFT Remora Statistics\n");
	printf("\t   a         tone    Play a tones ( -m <mod_no> - Module number)\n");
	printf("\t   a         ring    Rings phone ( -m <mod_no> - Module number)\n");
	printf("\t   a         regdump Dumps FXS/FXO registers.\n");
	printf("\t                     ( -m <mod_no> - Module number)\n");
	printf("\t   a         stats   Voltage status ( -m <mod_no> - Module number)\n");
	printf("\tAFT Debugging\n");
	printf("\t   d         err     Eanble RX RBS debugging\n");
	printf("\t   d         drr     Disable RX RBS debugging\n");
	printf("\t   d         ert     Eanble TX RBS debugging\n");
	printf("\t   d         drt     Disable TX RBS debugging\n");
	printf("\t   d         rr      Read RX/TX RBS status\n");
	printf("\t   d         pr      Print current RX/TX RBS status\n");
	printf("\t   d         sr      Set TX RBS status\n");
	printf("\n");
	printf("\t   d         e_span_seq      Enable span seq debugging\n");
	printf("\t   d         d_span_seq      Disable span seq debugging\n");
	printf("\t   d         rx_fifo_gen     Generate rx fifo error\n");
	printf("\t   d         tx_fifo_gen     Generate tx fifo error\n");
	printf("\t   d         fifo_sync_cnt   Increment fifo sync cnt\n"); 
	printf("\n");
	printf("\t--------\n\n");
	printf("\tExamples:\n");
#ifdef __WINDOWS__
	printf("\t View T1/E1 Alarams:\n");
	printf("\t\t wanpipemon -i wanpipe1_if1 -c Ta\n");
	printf("\t View Communications/Error statistics:\n");
	printf("\t\t wanpipemon -i wanpipe1_if1 -c sc\n");
	printf("\t Flush (Reset) Communications/Error statistics:\n");
	printf("\t\t wanpipemon -i wanpipe1_if1 -c fc\n");
	printf("\t Trace frames (Tx and Rx):\n");
	printf("\t\t wanpipemon -i wanpipe1_if1 -c tr\n\n");
#else
	printf("\tex: wanpipemon -i w1g1 -u 9000 -c xm :View Modem Status \n");
	printf("\tex: wanpipemon -i 201.1.1.2 -u 9000 -c ti  :Trace and Interpret ALL frames\n\n");
#endif
	return 0;

}

static void aft_router_up_time( void )
{
	u_int32_t time;
	
	wan_udp.wan_udphdr_command= ROUTER_UP_TIME;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	wan_udp.wan_udphdr_data[0] = 0;
	DO_COMMAND(wan_udp);
	
	time = *(u_int32_t*)&wan_udp.wan_udphdr_data[0];
	
	BANNER("ROUTER UP TIME");
	
	print_router_up_time(time);
}

static void aft_perf_stats( void )
{
#ifdef WANPIPE_PERFORMANCE_DEBUG
	aft_driver_performance_stats_t *aft_perf;
	
	wan_udp.wan_udphdr_command= WANPIPEMON_READ_PEFORMANCE_STATS;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	wan_udp.wan_udphdr_data[0] = 0;
	DO_COMMAND(wan_udp);
	
	aft_perf = (aft_driver_performance_stats_t*)&wan_udp.wan_udphdr_data[0];
	
	BANNER("WANPIPE PERFORMANCE STATS");

	printf("                  ISR\n");
	printf("===========================================\n");
	printf("ISR    Latency=%02lu    Max=%02lu    Min=%02lu    Avg=%02lu    AbvAvg=%04lu BelAvg=%04lu Limit=%02lu\n",
			aft_perf->aft_isr_latency.latency,
			aft_perf->aft_isr_latency.max_latency,
			aft_perf->aft_isr_latency.min_latency,
			aft_perf->aft_isr_latency.latency_avg,
			aft_perf->aft_isr_latency.above_avg,
			aft_perf->aft_isr_latency.below_avg,
			aft_perf->aft_isr_latency.limit);
	
	printf("Kernel Latency=%05lu Max=%05lu Min=%05lu Avg=%05lu AbvAvg=%04lu BelAvg=%04lu Limit=%02lu\n",
			aft_perf->kernel_isr_latency.latency,
			aft_perf->kernel_isr_latency.max_latency,
			aft_perf->kernel_isr_latency.min_latency,
			aft_perf->kernel_isr_latency.latency_avg,
			aft_perf->kernel_isr_latency.above_avg,
			aft_perf->kernel_isr_latency.below_avg,
			aft_perf->kernel_isr_latency.limit);
	printf("-------------------------------------------\n");
	printf("ALL ..............................%lu\n",aft_perf->isr.all);
	printf("AFT ..............................%lu\n",aft_perf->isr.aft);
	printf("Non AFT  .........................%lu\n",aft_perf->isr.non_aft);
	printf("-------------------------------------------\n");
	printf("FE ...............................%lu\n",aft_perf->isr.fe);
	printf("FE Run............................%lu\n",aft_perf->isr.fe_run);
	printf("-------------------------------------------\n");
	printf("TDM ..............................%lu\n",aft_perf->isr.tdm);
	printf("TDM Run...........................%lu\n",aft_perf->isr.tdm_run);
	printf("-------------------------------------------\n");
	printf("DMA ..............................%lu\n",aft_perf->isr.dma);
	printf("DMA Rx............................%lu\n",aft_perf->isr.dma_rx);
	printf("DMA Tx ...........................%lu\n",aft_perf->isr.dma_tx);
	printf("-------------------------------------------\n");
	printf("FIFO..............................%lu\n",aft_perf->isr.fifo);
	printf("FIFO RX...........................%lu\n",aft_perf->isr.fifo_rx);
	printf("FIFO TX...........................%lu\n",aft_perf->isr.fifo_tx);
	printf("-------------------------------------------\n");
	printf("Free Run Timer....................%lu\n",aft_perf->isr.free_run);
	printf("WDT ..............................%lu\n",aft_perf->isr.wdt);
	printf("WDT Soft..........................%lu\n",aft_perf->isr.wdt_software);
	printf("-------------------------------------------\n");
	printf("Serial............................%lu\n",aft_perf->isr.serial);
	printf("                  BH\n");
	printf("===========================================\n");
	printf("All ..............................%lu\n",aft_perf->bh.all);
	printf("Rx ...............................%lu\n",aft_perf->bh.rx);
	printf("Rx Stack .........................%lu\n",aft_perf->bh.rx_stack);
	printf("Rx Bri Dchan .....................%lu\n",aft_perf->bh.rx_bri_dchan);
	printf("Tx Post ..........................%lu\n",aft_perf->bh.tx_post);
	printf("               Port Task\n");
	printf("===========================================\n");
	printf("All ..............................%lu\n",aft_perf->port_task.all);
	printf("FE ISR............................%lu\n",aft_perf->port_task.fe_isr);
	printf("FE Poll ..........................%lu\n",aft_perf->port_task.fe_poll);
	printf("EC ...............................%lu\n",aft_perf->port_task.ec);
	printf("EC Poll ..........................%lu\n",aft_perf->port_task.ec_poll);
	printf("LED ..............................%lu\n",aft_perf->port_task.led);
	printf("RBS ..... ........................%lu\n",aft_perf->port_task.rbs);
	printf("Serial Status ....................%lu\n",aft_perf->port_task.serial_status);
	printf("TAP Queue ........................%lu\n",aft_perf->port_task.tap_q);
	printf("Restart ..... ....................%lu\n",aft_perf->port_task.restart);
#else
	printf("Error: Performance Stats Feature has not been compiled in!\n");
#endif
	
}

static void flush_aft_perf_stats( void )
{
#ifdef WANPIPE_PERFORMANCE_DEBUG
	wan_udp.wan_udphdr_command= WANPIPEMON_FLUSH_PEFORMANCE_STATS;
	wan_udp.wan_udphdr_return_code = 0xaa;
	wan_udp.wan_udphdr_data_len = 0;
	wan_udp.wan_udphdr_data[0] = 0;
	DO_COMMAND(wan_udp);
#else
	printf("Error: Performance Stats Feature has not been compiled in!\n");
#endif
}

static void read_ft1_te1_56k_config (void)
{
	unsigned char	adapter_type = 0x00;

	/* Read Adapter Type */
	if (get_fe_type(&adapter_type)){
		return;
	}
#if 0
	printf("Adapter type %i\n",adapter_type);
#endif	
	switch(adapter_type){
	case WAN_MEDIA_NONE:
		printf("CSU/DSU Read Configuration Failed");
		break;

	case WAN_MEDIA_T1:
	case WAN_MEDIA_E1:
	case WAN_MEDIA_DS3:
	case WAN_MEDIA_56K:
		read_te1_56k_config();
		break;

	default:
		printf("Unknown Front End Adapter Type: 0x%X",adapter_type);
		break;
		
	}
	return;
}

static u_int32_t parse_channel_arg(int argc, char* argv[])
{
	u_int32_t	chan_map = 0x00;
	int		argi = 0, chan;

	for(argi = 1; argi < argc; argi++){

		char *parg = argv[argi], *param;

		if (strcmp(parg, "--chan") == 0){

			if (argi + 1 >= argc ){
				printf("ERROR: Channel argument is missing!\n");
				return ENABLE_ALL_CHANNELS;
			}

			param 	= argv[argi+1];
			chan_map= 0x00;
			if (wp_strcasecmp(param,"all") == 0){
				return ENABLE_ALL_CHANNELS;
			}else{
				char	chan[10];
				int	i, j = 0, len=strlen(param);
				int	start_ch = 0, stop_ch = 0, range = 0;
			
				for(i = 0; i < len; i++){
					if (param[i] == '-'){
						range = 1;
						start_ch = atoi(chan);
						j = 0;
						continue;
					}
					chan[j++] = param[i];
				}
				if (!range){
					start_ch = atoi(chan);
				}
				stop_ch = atoi(chan);
				chan_map = 0x00;
				for(i = stop_ch; i >= start_ch; i--){
					chan_map |= (0x01 << i);
				}
			}
			return chan_map;
		}
	}
	return ENABLE_ALL_CHANNELS;
}

int AFTMain(char *command,int argc, char* argv[])
{
	char		*opt=&command[1];
	int		mod_no = 0, i, err=0;
	u_int32_t	chan_map;
	sdla_fe_debug_t	fe_debug;
			
	switch(command[0]){

		case 'x':
			if (!strcmp(opt,"m")){	
				modem();
			}else if (!strcmp(opt,"l")){
				link_status();
			}else if (!strcmp(opt, "cv")){
				read_code_version();
			}else if (!strcmp(opt,"ru")){
				aft_router_up_time();
			}else if (!strcmp(opt,"perf")) {
				aft_perf_stats();
			}else{
				printf("ERROR: Invalid Status Command 'x', Type wanpipemon <cr> for help\n\n");
			}
			break;	

		case 's':
			if (!strcmp(opt,"c")){
				comm_err_stats();
			}else if (!strcmp(opt,"o")){
				operational_stats();
			}else{
				printf("ERROR: Invalid Statistics Command 's', Type wanpipemon <cr> for help\n\n");
				printf("command: %s\n", command);
			}
			break;
			
		case 't':
	    		memset(&trace_iface,0,sizeof(wp_trace_output_iface_t));

			if (!strcmp(opt, "r")){
				raw_data = WAN_TRUE;
				trace_iface.type=WP_OUT_TRACE_RAW;
				line_trace(0);
                        }else if (!strcmp(opt, "rh")){
				raw_data = WAN_TRUE;
				trace_iface.type=WP_OUT_TRACE_HDLC;
				line_trace(0);
			}else if (!strcmp(opt, "rd")){
				raw_data = WAN_TRUE;
				trace_iface.type=WP_OUT_TRACE_RAW;
				line_trace(1);
			}else if (!strcmp(opt, "i")){
				raw_data = WAN_FALSE;
				trace_iface.type=WP_OUT_TRACE_INTERP;
				line_trace(0);
			}else if (!strcmp(opt, "i4")){
				raw_data = WAN_FALSE;
				trace_iface.type=WP_OUT_TRACE_INTERP;
				trace_iface.sub_type = WP_OUT_TRACE_INTERP_IPV4;
				line_trace(0);
			}else{
				printf("ERROR: Invalid Trace Command 't', Type wanpipemon <cr> for help\n\n");
			}
			break;

		case 'f':
			if (!strcmp(opt, "o")){
				flush_operational_stats();
				printf("Operational statistics flushed\n");
				/*operational_stats();*/
			}else if (!strcmp(opt, "c")){
				flush_comm_err_stats();
				printf("Communication statistics flushed\n");
			/*	comm_err_stats();*/
			}else if (!strcmp(opt, "pm")){
				flush_te1_pmon();
				printf("Performance monitoring counters flushed\n");
			}else if (!strcmp(opt, "perf")) {
				flush_aft_perf_stats();
			} else{
				printf("ERROR: Invalid Flush Command 'f', Type wanpipemon <cr> for help\n\n");
			}
			break;

		case 'T':
			if (!strcmp(opt,"read")){
				read_ft1_te1_56k_config();
				/* T1/E1 begins */
			}else if (!strcmp(opt,"lt")){
 				aft_digital_loop_test();
			}else if (!strcmp(opt,"lb")){
				get_lb_modes(0);

			/* DavidR: 'apclb' and 'dpclb' commands don't work.
			 *	Not removing to keep backward compatibility. */
			}else if (!strcmp(opt,"apclb")){
				chan_map = parse_channel_arg(argc, argv);
				set_lb_modes(WAN_TE1_LINELB_MODE, WAN_TE1_LB_ENABLE);
				set_lb_modes_per_chan(WAN_TE1_PCLB_MODE, WAN_TE1_LB_ENABLE, chan_map);
			}else if (!strcmp(opt,"dpclb")){
				chan_map = parse_channel_arg(argc, argv);
				set_lb_modes_per_chan(WAN_TE1_PCLB_MODE, WAN_TE1_LB_DISABLE, chan_map);

			/* DavidR: Warning: The LLB command should NOT be used Locally.
			 *	These commands must be controlled from Remote side. 
			 *	Not removing to keep backward compatibility. */
			}else if (!strcmp(opt,"allb")){
				set_lb_modes(WAN_TE1_LINELB_MODE, WAN_TE1_LB_ENABLE);
			}else if (!strcmp(opt,"dllb")){
				set_lb_modes(WAN_TE1_LINELB_MODE, WAN_TE1_LB_DISABLE);

			}else if (!strcmp(opt,"aplb")){
				set_lb_modes(WAN_TE1_PAYLB_MODE, WAN_TE1_LB_ENABLE);
			}else if (!strcmp(opt,"dplb")){
				set_lb_modes(WAN_TE1_PAYLB_MODE, WAN_TE1_LB_DISABLE);
			}else if (!strcmp(opt,"adlb")){
				set_lb_modes(WAN_TE1_DDLB_MODE, WAN_TE1_LB_ENABLE);
			}else if (!strcmp(opt,"ddlb")){
				set_lb_modes(WAN_TE1_DDLB_MODE, WAN_TE1_LB_DISABLE);
			}else if (!strcmp(opt,"salb")){
				set_lb_modes(WAN_TE1_TX_LINELB_MODE, WAN_TE1_LB_ENABLE);
			}else if (!strcmp(opt,"sdlb")){
				set_lb_modes(WAN_TE1_TX_LINELB_MODE, WAN_TE1_LB_DISABLE);
			}else if (!strcmp(opt,"saplb")){
				set_lb_modes(WAN_TE1_TX_PAYLB_MODE, WAN_TE1_LB_ENABLE);
			}else if (!strcmp(opt,"sdplb")){
				set_lb_modes(WAN_TE1_TX_PAYLB_MODE, WAN_TE1_LB_DISABLE);
			}else if (!strcmp(opt,"alalb")){
				/* T1/E1 LIU LBs */
				set_lb_modes(WAN_TE1_LIU_ALB_MODE, WAN_TE1_LB_ENABLE);
			}else if (!strcmp(opt,"dlalb")){
				set_lb_modes(WAN_TE1_LIU_ALB_MODE, WAN_TE1_LB_DISABLE);
			}else if (!strcmp(opt,"alllb")){
				set_lb_modes(WAN_TE1_LIU_LLB_MODE, WAN_TE1_LB_ENABLE);
			}else if (!strcmp(opt,"dlllb")){
				set_lb_modes(WAN_TE1_LIU_LLB_MODE, WAN_TE1_LB_DISABLE);
			}else if (!strcmp(opt,"alrlb")){
				set_lb_modes(WAN_TE1_LIU_RLB_MODE, WAN_TE1_LB_ENABLE);
			}else if (!strcmp(opt,"dlrlb")){
				set_lb_modes(WAN_TE1_LIU_RLB_MODE, WAN_TE1_LB_DISABLE);
			}else if (!strcmp(opt,"aldlb")){
				set_lb_modes(WAN_TE1_LIU_DLB_MODE, WAN_TE1_LB_ENABLE);
			}else if (!strcmp(opt,"dldlb")){
				set_lb_modes(WAN_TE1_LIU_DLB_MODE, WAN_TE1_LB_DISABLE);
				/* T3 begins */
			}else if (!strcmp(opt,"allb3")){
				set_lb_modes(WAN_TE3_LIU_LB_ANALOG, WAN_TE3_LB_ENABLE);
			}else if (!strcmp(opt,"dllb3")){
				set_lb_modes(WAN_TE3_LIU_LB_ANALOG, WAN_TE3_LB_DISABLE);
			}else if (!strcmp(opt,"arlb3")){
				set_lb_modes(WAN_TE3_LIU_LB_REMOTE, WAN_TE3_LB_ENABLE);
			}else if (!strcmp(opt,"drlb3")){
				set_lb_modes(WAN_TE3_LIU_LB_REMOTE, WAN_TE3_LB_DISABLE);
			}else if (!strcmp(opt,"adlb3")){
				set_lb_modes(WAN_TE3_LIU_LB_DIGITAL, WAN_TE3_LB_ENABLE);
			}else if (!strcmp(opt,"ddlb3")){
				set_lb_modes(WAN_TE3_LIU_LB_DIGITAL, WAN_TE3_LB_DISABLE);
				/* 56k begins */
			}else if (!strcmp(opt,"a")){
				read_te1_56k_stat(0);
			}else if (!strcmp(opt,"af")){
				read_te1_56k_stat(1);
			}else if (!strcmp(opt,"txe")){
				set_fe_tx_mode(WAN_FE_TXMODE_ENABLE);
			}else if (!strcmp(opt,"txd")){
				set_fe_tx_mode(WAN_FE_TXMODE_DISABLE);
			}else if (!strcmp(opt,"bert")){
				err = set_fe_bert(argc, argv);
			}else if (!strcmp(opt,"sw_bert")){
				err = set_sw_bert(argc, argv);
			}else{
				printf("ERROR: Invalid FT1 Command 'T', Type wanpipemon <cr> for help\n\n");
			} 
			break;

		case 'd':
			if (!strcmp(opt,"err")){
				set_debug_mode(WAN_FE_DEBUG_RBS, WAN_FE_DEBUG_RBS_RX_ENABLE);
			}else if (!strcmp(opt,"ert")){
				set_debug_mode(WAN_FE_DEBUG_RBS, WAN_FE_DEBUG_RBS_TX_ENABLE);
			}else if (!strcmp(opt,"drr")){
				set_debug_mode(WAN_FE_DEBUG_RBS, WAN_FE_DEBUG_RBS_RX_DISABLE);
			}else if (!strcmp(opt,"drt")){
				set_debug_mode(WAN_FE_DEBUG_RBS, WAN_FE_DEBUG_RBS_TX_DISABLE);
			}else if (!strcmp(opt,"rr")){
				set_debug_mode(WAN_FE_DEBUG_RBS, WAN_FE_DEBUG_RBS_READ);
			}else if (!strcmp(opt,"pr")){
				set_debug_mode(WAN_FE_DEBUG_RBS, WAN_FE_DEBUG_RBS_PRINT);
			}else if (!strcmp(opt,"sr")){
				int		i=0;
				if (argc < 6){
					printf("ERROR: Invalid command argument!\n");
					break;				
				}
				fe_debug.fe_debug_rbs.abcd	= 0x00;
				fe_debug.fe_debug_rbs.channel= atoi(argv[5]);
				if (fe_debug.fe_debug_rbs.channel < 1 || fe_debug.fe_debug_rbs.channel > 31){
					printf("ERROR: T1/E1 channel number of out range (%d)!\n",
								fe_debug.fe_debug_rbs.channel);
					break;
				}
				if (argc > 6){
					for(i = 0; i < strlen(argv[6]); i++){
						switch(argv[6][i]){
						case 'A': case 'a':
							fe_debug.fe_debug_rbs.abcd |= WAN_RBS_SIG_A;
							break;
						case 'B': case 'b':
							fe_debug.fe_debug_rbs.abcd |= WAN_RBS_SIG_B;
							break;
						case 'C': case 'c':
							fe_debug.fe_debug_rbs.abcd |= WAN_RBS_SIG_C;
							break;
						case 'D': case 'd':
							fe_debug.fe_debug_rbs.abcd |= WAN_RBS_SIG_D;
							break;
						}
					}
				}
				fe_debug.type = WAN_FE_DEBUG_RBS;
				fe_debug.mode = WAN_FE_DEBUG_RBS_SET;
				set_fe_debug_mode(&fe_debug);
			}else if (!strcmp(opt,"eais")){
				set_debug_mode(WAN_FE_DEBUG_ALARM, WAN_FE_DEBUG_ALARM_AIS_ENABLE);
			}else if (!strcmp(opt,"dais")){
				set_debug_mode(WAN_FE_DEBUG_ALARM, WAN_FE_DEBUG_ALARM_AIS_DISABLE);
			}else if (!strcmp(opt,"cfg")){
				fe_debug.type = WAN_FE_DEBUG_RECONFIG;
				set_fe_debug_mode(&fe_debug);
			}else if (!strcmp(opt,"reg")){
				long	value;
				fe_debug.type = WAN_FE_DEBUG_REG;
				if (argc < 6){
					printf("ERROR: Invalid command argument!\n");
					break;				
				}
				value = strtol(argv[5],(char**)NULL, 16);
				if (value == LONG_MIN || value == LONG_MAX){
					printf("ERROR: Invalid argument 5: %s!\n",
								argv[5]);
					break;				
				}
				fe_debug.fe_debug_reg.reg  = value;
				fe_debug.fe_debug_reg.read = 1;
				if (argc > 6){
					value = strtol(argv[6],(char**)NULL, 16);
					if (value == LONG_MIN || value == LONG_MAX){
						printf("ERROR: Invalid argument 6: %s!\n",
									argv[6]);
						break;
					}
					fe_debug.fe_debug_reg.read = 0;
					fe_debug.fe_debug_reg.value = value;
				}
				set_fe_debug_mode(&fe_debug);

            }else if (!strcmp(opt,"regsb") || !strcmp(opt,"regcb")){
 
 				long	value, bit;
 				fe_debug.type = WAN_FE_DEBUG_REG;
 				if (argc < 7){
 					printf("ERROR: Invalid command argument!\n");
 					break;				
 				}
 				value = strtol(argv[5],(char**)NULL, 16);
 				if (value == LONG_MIN || value == LONG_MAX){
 					printf("ERROR: Invalid argument 5: %s!\n",
 								argv[5]);
 					break;				
 				}
 
 				bit = strtol(argv[6],(char**)NULL, 10);
 				if (bit == LONG_MIN || bit == LONG_MAX){
 					printf("ERROR: Invalid argument 6: %s!\n",
 								argv[6]);
 					break;
 				}
 				aft_fe_reg_bit((!strcmp(opt,"regsb")) ? 1 : 0, value, bit);  

			}else if (!strcmp(opt,"e_span_seq")){
				wp_span_debugging(1);
			}else if (!strcmp(opt,"d_span_seq")){
				wp_span_debugging(0);
			}else if (!strcmp(opt,"rx_fifo_gen")) {
				wp_gen_fifo_debugging(1);
			} else if  (!strcmp(opt,"tx_fifo_gen")) {
				wp_gen_fifo_debugging(0);
			} else if (!strcmp(opt, "fifo_sync_cnt")) {
				wp_get_fifo_sync_cnt();
			}else{
				printf("ERROR: Invalid Status Command 'd', Type wanpipemon <cr> for help\n\n");
			}
			break;	
		case 'e':
			if (strcmp(opt,"hw") == 0){	
				aft_read_hwec_status();
			}else{
				printf("ERROR: Invalid Status Command 'e', Type wanpipemon <cr> for help\n\n");
			}
			break;

		case 'a':
			err = 0;
			for(i=0;i<argc;i++){
				if (!wp_strcasecmp(argv[i], "-m") && argv[i+1]){
					err = sscanf(argv[i+1], "%d", &mod_no);
					if (err){
						if (mod_no < 1 || mod_no > 16){
							printf("ERROR: Invalid Module number!\n\n");
						   	fflush(stdout);
					   		return 0;
						}
						mod_no--;
					}else{
						printf("ERROR: Invalid Module number!\n\n");
					   	fflush(stdout);
				   		return 0;
					}
					break;
				}
			}

			if (strcmp(opt,"tone") == 0){	
				aft_remora_tones(mod_no);
			}else if (strcmp(opt,"ring") == 0){	
				aft_remora_ring(mod_no);
			}else if (strcmp(opt,"regdump") == 0){	
				aft_remora_regdump(mod_no);
			}else if (!strcmp(opt,"reg")){
				long	value;
				fe_debug.type = WAN_FE_DEBUG_REG;
				if (argc < 6){
					printf("ERROR: Invalid command argument!\n");
					break;				
				}
				value = strtol(argv[5],(char**)NULL, 10);
				if (value == LONG_MIN || value == LONG_MAX){
					printf("ERROR: Invalid argument 5: %s!\n",
								argv[5]);
					break;				
				}
				fe_debug.fe_debug_reg.reg  = value;
				fe_debug.fe_debug_reg.read = 1;
				if (strcmp(argv[6], "-m") && argc > 6){
					value = strtol(argv[6],(char**)NULL, 16);
					if (value == LONG_MIN || value == LONG_MAX){
						printf("ERROR: Invalid argument 6: %s!\n",
									argv[6]);
						break;
					}
					fe_debug.fe_debug_reg.read = 0;
					fe_debug.fe_debug_reg.value = value;
				}
				fe_debug.mod_no = mod_no;
				aft_remora_debug_mode(&fe_debug);
			}else if (strcmp(opt,"stats") == 0){	
				aft_remora_stats(mod_no);
			}else if (strcmp(opt,"offhook") == 0){	
				aft_remora_hook(mod_no, 1);
			}else if (strcmp(opt,"onhook") == 0){	
				aft_remora_hook(mod_no, 0);
			}else{
				printf("ERROR: Invalid Status Command 'a', Type wanpipemon <cr> for help\n\n");
			}
			break;

		default:
			printf("ERROR: Invalid Command, Type wanpipemon <cr> for help\n\n");
			break;
	}//switch 
   	printf("\n");
   	fflush(stdout);
   	return err;
}; //main
