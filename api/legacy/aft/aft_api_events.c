/*****************************************************************************
* aft_api.c	AFT T1/E1: HDLC API Sample Code
*
* Author(s):	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003-2004 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
*/


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_wanpipe.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/if.h>
#include <linux/wanpipe_defines.h>
#include <linux/wanpipe_cfg.h>
#include <linux/wanpipe.h>
#include <linux/sdla_aft_te1.h>
#include "lib_api.h"


#include <linux/wanpipe_events.h>
//#include "wanec_iface.h"
#include "wanec_api.h"


#define MAX_TX_DATA     5000	/* Size of tx data */  
#define MAX_FRAMES 	5000     /* Number of frames to transmit */  

#define MAX_RX_DATA	5000

unsigned short Rx_lgth;

unsigned char Rx_data[MAX_RX_DATA];
unsigned char Tx_data[MAX_TX_DATA + sizeof(api_tx_hdr_t)];

/* Prototypes */
int MakeConnection(void);
void handle_socket(int ,char**);
void sig_end(int sigid);

int sock;
FILE *tx_fd=NULL,*rx_fd=NULL;	


#define WP_API_EVENT_MAX_RETRY	10
u_int8_t	event_type;
int		channel, tone_id = 1;

/***************************************************
* MakeConnection
*
*   o Create a Socket
*   o Bind a socket to a wanpipe network interface
*       (Interface name is supplied by the user)
*/         

int MakeConnection(void) 
{
	struct wan_sockaddr_ll 	sa;

	memset(&sa,0,sizeof(struct wan_sockaddr_ll));
	errno = 0;
   	sock = socket(AF_WANPIPE, SOCK_RAW, 0);
   	if( sock < 0 ) {
      		perror("Socket");
      		return( WAN_FALSE );
   	} /* if */
  
	printf("\nConnecting to card %s, interface %s prot %x\n", card_name, if_name,htons(PVC_PROT));
	
	strcpy( sa.sll_device, if_name);
	strcpy( sa.sll_card, card_name);
	sa.sll_protocol = htons(PVC_PROT);
	sa.sll_family=AF_WANPIPE;
	
        if(bind(sock, (struct sockaddr *)&sa, sizeof(struct wan_sockaddr_ll)) < 0){
                perror("bind");
		printf("Failed to bind a socket to %s interface\n",if_name);
                exit(0);
        }

	{
		unsigned int customer_id;
		int err = ioctl(sock,SIOC_AFT_CUSTOMER_ID,&customer_id);
		if (err){
			perror("Customer ID: ");
		}else{
			printf("Customer ID 0x%X\n",customer_id);
		}
	
		
	}
	printf("Socket bound to %s\n\n",if_name);
	
	return( WAN_TRUE );

}

static int dtmf_event_ctrl(u_int8_t mode, int channel)
{
	api_tx_hdr_t	api_tx_hdr;
	int		err, cnt=0;

	/* Configure DTMF events */
	printf("%s DTMF event on channel %d\n",
			(mode==WP_API_EVENT_ENABLE) ? "Configure" : "Clear",
			channel);
	
	err = wanec_api_dtmf(	card_name,
				(mode==WP_API_EVENT_ENABLE) ? 1 : 0,
				(1 << channel),
				WAN_EC_CHANNEL_PORT_SOUT,
				WAN_EC_TONE_PRESENT,
				1);
	if (err){
		printf("ERROR: %s: Failed to execute DTMF configuration request!\n",
					card_name);
		return -EINVAL;
	}			
	
	/* Enable DTMF events */
	memset(&api_tx_hdr, 0, sizeof(api_tx_hdr_t));
	printf("%s DTMF Event on channel %d...\n",
			(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable",
			channel);
	api_tx_hdr.wp_api_tx_hdr_event_type	= WP_API_EVENT_DTMF;
	api_tx_hdr.wp_api_tx_hdr_event_mode	= mode;
	api_tx_hdr.wp_api_tx_hdr_event_channel	= channel;
dtmf_try_again:	
	err = ioctl(sock,SIOC_WANPIPE_API,&api_tx_hdr);
	if (err < 0){
		usleep(10000);
		if (cnt++ < WP_API_EVENT_MAX_RETRY) goto dtmf_try_again;
		printf("ERROR: Failed to %s DTMF events!\n",
				(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable");
		return -EINVAL;
	}

	return 0;
}

static int rxhook_event_ctrl(u_int8_t mode, int channel)
{
	api_tx_hdr_t	api_tx_hdr;
	int		err, cnt=0;
	
	/* Enable Rx-hook events */
	memset(&api_tx_hdr, 0, sizeof(api_tx_hdr_t));
	printf("%s Rx-hook Event...\n",
		(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable");
	
	api_tx_hdr.wp_api_tx_hdr_event_type	= WP_API_EVENT_RXHOOK;
	api_tx_hdr.wp_api_tx_hdr_event_mode	= mode;
	api_tx_hdr.wp_api_tx_hdr_event_channel	= channel;
rxhook_try_again:	
	err = ioctl(sock,SIOC_WANPIPE_API,&api_tx_hdr);
	if (err < 0){
		usleep(10000);
		if (cnt++ < WP_API_EVENT_MAX_RETRY) goto rxhook_try_again;
		printf("ERROR: Failed to %s Rx-hook events (%d)!\n",
		       	(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable",
			cnt);
		return -EINVAL;
	}

	return 0;
}

static int ring_event_ctrl(u_int8_t mode, int channel)
{
	api_tx_hdr_t	api_tx_hdr;
	int		err, cnt=0;
	
	/* Enable Ring events */
	memset(&api_tx_hdr, 0, sizeof(api_tx_hdr_t));
	printf("%s Ring Event...\n",
		(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable");
	api_tx_hdr.wp_api_tx_hdr_event_type	= WP_API_EVENT_RING;
	api_tx_hdr.wp_api_tx_hdr_event_mode	= mode;
	api_tx_hdr.wp_api_tx_hdr_event_channel	= channel;
ring_try_again:	
	err = ioctl(sock,SIOC_WANPIPE_API,&api_tx_hdr);
	if (err < 0){
		usleep(10000);
		if (cnt++ < WP_API_EVENT_MAX_RETRY) goto ring_try_again;
		printf("ERROR: Failed to %s Ring events!\n",
				(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable");
		return -EINVAL;
	}

	return 0;
}

static int tone_event_ctrl(u_int8_t mode, int channel, int tone)
{
	api_tx_hdr_t	api_tx_hdr;
	int		err, cnt=0;
	
	/* Enable Tone events */
	memset(&api_tx_hdr, 0, sizeof(api_tx_hdr_t));
	printf("%s Tone Event...\n",
		(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable");
	api_tx_hdr.wp_api_tx_hdr_event_type	= WP_API_EVENT_TONE;
	api_tx_hdr.wp_api_tx_hdr_event_mode	= mode;
	api_tx_hdr.wp_api_tx_hdr_event_tone	= tone;
	api_tx_hdr.wp_api_tx_hdr_event_channel	= channel;
tone_try_again:	
	err = ioctl(sock,SIOC_WANPIPE_API,&api_tx_hdr);
	if (err < 0){
		usleep(10000);
		if (cnt++ < WP_API_EVENT_MAX_RETRY) goto tone_try_again;
		printf("ERROR: Failed to %s Tone events!\n",
				(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable");
		return -EINVAL;
	}

	return 0;
}

static int ringdetect_event_ctrl(u_int8_t mode, int channel, int tone)
{
	api_tx_hdr_t	api_tx_hdr;
	int		err, cnt=0;
	
	/* Enable Tone events */
	memset(&api_tx_hdr, 0, sizeof(api_tx_hdr_t));
	printf("%s Ring Detect Event...\n",
		(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable");
	api_tx_hdr.wp_api_tx_hdr_event_type	= WP_API_EVENT_RING_DETECT;
	api_tx_hdr.wp_api_tx_hdr_event_mode	= mode;
	api_tx_hdr.wp_api_tx_hdr_event_channel	= channel;
ringdetect_try_again:	
	err = ioctl(sock,SIOC_WANPIPE_API,&api_tx_hdr);
	if (err < 0){
		usleep(10000);
		if (cnt++ < WP_API_EVENT_MAX_RETRY) goto ringdetect_try_again;
		printf("ERROR: Failed to %s Tone events!\n",
				(mode==WP_API_EVENT_ENABLE) ? "Enable" : "Disable");
		return -EINVAL;
	}

	return 0;
}

static int event_decode(api_rx_hdr_t *rx_hdr)
{
	static int	event_cnt = 0;
	
	switch(rx_hdr->event_type){
	case WP_API_EVENT_DTMF:
		printf("%04d --> DTMF event: %c (%s:%s)\n",
			event_cnt++,
			rx_hdr->wp_api_rx_hdr_event_dtmf_digit,
			(rx_hdr->wp_api_rx_hdr_event_dtmf_port == WAN_EC_CHANNEL_PORT_SOUT) ?
						"SOUT" : "ROUT",
			(rx_hdr->wp_api_rx_hdr_event_dtmf_type == WAN_EC_TONE_PRESENT) ?
						"PRESENT" : "STOP");
		break;
	case WP_API_EVENT_RXHOOK:
		printf("%04d --> Rx-hook: %s\n",
			event_cnt++,
			(rx_hdr->wp_api_rx_hdr_event_rxhook_state == WP_API_EVENT_RXHOOK_OFF) ?
						"Off-hook" : 
			(rx_hdr->wp_api_rx_hdr_event_rxhook_state == WP_API_EVENT_RXHOOK_ON) ?
						"On-hook" : "Unknown");
		break;
	case WP_API_EVENT_RING_DETECT:
		printf("%04d --> Ring Detect: %s\n",
			event_cnt++,
			WAN_EVENT_RING_DECODE(rx_hdr->wp_api_rx_hdr_event_ringdetect_state));
		break;
	default:
		printf("%04d --> Unknown event\n",
			event_cnt++);
		break;		
	}
	return 0;
}

static int event_ctrl(u_int8_t mode, int argc, char* argv[])
{
	int i=0;
	
	for (i = 0; i < argc; i++){
		if (!strcmp(argv[i],"-tone")){

			if (i+2 > argc-1){
				printf("ERROR: Invalid Interface Name!\n");
				return WAN_FALSE;
			}
			if(!isdigit(argv[i+1][0])){
				printf("ERROR: Invalid tone id!\n");
				return WAN_FALSE;			
			}
			event_type = WP_API_EVENT_TONE;			
			tone_id = atoi(argv[i+1]);
			channel = atoi(argv[i+2]);
			if (tone_event_ctrl(mode, channel, tone_id)){
				return -1;
			}
		}
		if (!strcmp(argv[i],"-ring")){

			if (i+1 > argc-1){
				printf("ERROR: Invalid Interface Name!\n");
				return WAN_FALSE;
			}
			if(!isdigit(argv[i+1][0])){
				printf("ERROR: Invalid ring command!\n");
				return WAN_FALSE;
			}
			event_type = WP_API_EVENT_RING;
			channel = atoi(argv[i+1]);
			if (ring_event_ctrl(mode, channel)) return -1;
		}
		if (!strcmp(argv[i],"-rxhook")){
			if (i+1 > argc-1){
				printf("ERROR: Invalid Interface Name!\n");
				return WAN_FALSE;
			}
			if(!isdigit(argv[i+1][0])){
				printf("ERROR: Invalid ring command!\n");
				return WAN_FALSE;
			}
			event_type = WP_API_EVENT_RXHOOK;
			channel = atoi(argv[i+1]);
			if (rxhook_event_ctrl(mode, channel)) return -1;
		}
		if (!strcmp(argv[i],"-dtmf")){

			if (i+1 > argc-1){
				printf("ERROR: Invalid Interface Name!\n");
				return WAN_FALSE;
			}
			if(!isdigit(argv[i+1][0])){
				printf("ERROR: Invalid ring command!\n");
				return WAN_FALSE;
			}
			event_type = WP_API_EVENT_DTMF;
			channel = atoi(argv[i+1]);
			if (dtmf_event_ctrl(mode, channel)) return -1;
		}
	}
	return 0;
}



/***************************************************
* HANDLE SOCKET 
*
*   o Read a socket 
*   o Cast data received to api_rx_element_t data type 
*   o The received packet contains 16 bytes header 
*
*	------------------------------------------
*      |  16 bytes      |        X bytes        ...
*	------------------------------------------
* 	   Header              Data
*
*   o Data structures:
*
*   typedef struct {
*	unsigned char	error_flag;
*	unsigned short	time_stamp;
*	unsigned char	reserved[13];
*   } api_rx_hdr_t;
*
*   typedef struct {
*       api_rx_hdr_t	api_rx_hdr;
*       unsigned char  	data[1];
*   } api_rx_element_t;
*
*/

void handle_socket(int argc, char* argv[]) 
{
	unsigned int Rx_count,Tx_count,Tx_length;
	api_rx_element_t* api_rx_el;
	api_tx_element_t * api_tx_el;
	fd_set 	ready,write,oob;
	int err,i;
	int rlen;
#if 0
	int stream_sync=0;
#endif
        Rx_count = 0;
	Tx_count = 0;
	Tx_length = tx_size;

	printf("\n\nSocket Handler: Rx=%d Tx=%i TxCnt=%i TxLen=%i TxDelay=%i\n",
			read_enable,write_enable,tx_cnt,tx_size,tx_delay);	

	/* Initialize the Tx Data buffer */
	memset(&Tx_data[0],0,MAX_TX_DATA + sizeof(api_tx_hdr_t));

	/* If rx file is specified, open
	 * it and prepare it for writing */
	if (files_used & RX_FILE_USED){
		rx_fd=fopen(rx_file,"wb");
		if (!rx_fd){
			printf("Failed to open file %s\n",rx_file);
			perror("File: ");
			return;
		}
		printf("Opening %s rx file\n",rx_file);
	}

	/* Cast the Tx data packet with the tx element
	 * structure.  We must insert a 16 byte
	 * driver header, which driver will remove 
	 * before passing packet out the physical port */
	api_tx_el = (api_tx_element_t*)&Tx_data[0];
	
	if (files_used & TX_FILE_USED){

		/* TX file is used to transmit data */
		
		tx_fd=fopen(tx_file,"rb");
		if (!tx_fd){
			printf("Failed to open file %s\n",tx_file);
			perror("File: ");
			return;
		}
		
		printf("Opening %s tx file\n",rx_file);
		
		rlen=fread((void*)&Tx_data[sizeof(api_tx_hdr_t)],
				   sizeof(char),
				   Tx_length,tx_fd);
		if (!rlen){
			printf("%s: File empty!\n",
				tx_file);
			return;
		}
	}else{

		/* Create a Tx packet based on user info, or
		 * by deafult incrementing number starting from 0 */
		for (i=0;i<Tx_length;i++){
			if (tx_data == -1){
				api_tx_el->data[i] = (unsigned char)i;
			}else{
#if 0
				api_tx_el->data[i] = (unsigned char)tx_data+(i%4);
#else
				api_tx_el->data[i] = (unsigned char)tx_data;
#endif
			}
		}
	}

	write_enable = 1;
	read_enable = 1;
	if (event_ctrl(WP_API_EVENT_ENABLE, argc, argv)){
		event_ctrl(WP_API_EVENT_DISABLE, argc, argv);
		return;
	}
	
	/* Main Rx Tx OOB routine */
	for(;;) {	

		/* Initialize all select() descriptors */
		FD_ZERO(&ready);
		FD_ZERO(&write);
		FD_ZERO(&oob);
		FD_SET(sock,&oob);
		FD_SET(sock,&ready);

		if (write_enable){
		     FD_SET(sock,&write);
		}

		/* Select will block, until:
		 * 	1: OOB event, link level change
		 * 	2: Rx data available
		 * 	3: Interface able to Tx */
		
  		if(select(sock + 1,&ready, &write, &oob, NULL)){

			fflush(stdout);	
		   	if (FD_ISSET(sock,&oob)){
				api_rx_hdr_t	*rx_hdr;
				/* An OOB event is pending, usually indicating
				 * a link level change */
				
				err = recv(sock, Rx_data, MAX_RX_DATA, MSG_OOB);

				if(err < 0 ) {
					printf("Failed to receive OOB %i , %i\n", Rx_count, err);
					err = ioctl(sock,SIOC_WANPIPE_SOCK_STATE,0);
					printf("Sock state is %s\n",
							(err == 0) ? "CONNECTED" : 
							(err == 1) ? "DISCONNECTED" :
							 "CONNECTING");
					break;
				}

				printf("GOT OOB EXCEPTION CMD Exiting\n");
				rx_hdr = (api_rx_hdr_t*)Rx_data;
				event_decode(rx_hdr);
				break;
			}

                   	if (FD_ISSET(sock,&ready)){

				/* An Rx packet is pending
				 * 	1: Read the rx packet into the Rx_data
				 * 	   buffer. Confirm len > 0
				 *
				 * 	2: Cast Rx_data to the api_rx_element.
				 * 	   Thus, removing a 16 byte header
				 * 	   attached by the driver.
				 *
				 * 	3. Check error_flag:
				 * 		CRC,Abort..etc 
				 */
				err = recv(sock, Rx_data, MAX_RX_DATA, 0);

				if (!read_enable){
					goto bitstrm_skip_read;
				}
				
				/* err indicates bytes received */
				if(err > 0) {

					api_rx_el = (api_rx_element_t*)&Rx_data[0];

		                	/* Check the packet length */
                                	Rx_lgth = err - sizeof(api_rx_hdr_t);
                                	if(Rx_lgth<=0) {
                                        	printf("\nShort frame received (%d)\n",
                                                	Rx_lgth);
                                        	return;
                                	}
#if 0
					if (api_rx_el->api_rx_hdr.error_flag){
						printf("Data: ");
                                                for(i=0;i<Rx_lgth; i ++) {
                                                        printf("0x%02X ", Rx_data[i+16]);
                                                }
                                                printf("\n");
					}

					if (api_rx_el->api_rx_hdr.error_flag & (1<<WP_FIFO_ERROR_BIT)){
						printf("\nPacket with fifo overflow err=0x%X len=%i\n",
							api_rx_el->api_rx_hdr.error_flag,Rx_lgth);
						continue;
					}

					if (api_rx_el->api_rx_hdr.error_flag & (1<<WP_CRC_ERROR_BIT)){
                                                printf("\nPacket with invalid crc!  err=0x%X len=%i\n",
                                                        api_rx_el->api_rx_hdr.error_flag,Rx_lgth);
                                                continue;
                                        }

					if (api_rx_el->api_rx_hdr.error_flag & (1<<WP_ABORT_ERROR_BIT)){
                                                printf("\nPacket with abort!  err=0x%X len=%i\n",
                                                        api_rx_el->api_rx_hdr.error_flag,Rx_lgth);
                                                continue;
                                        }
#endif

					if (api_rx_el->api_rx_hdr.event_type){
						event_decode(&api_rx_el->api_rx_hdr);
					}
#if 0
					if (api_rx_el->data[0] == tx_data && api_rx_el->data[1] == (tx_data+1)){
						if (!stream_sync){
							printf("GOT SYNC %x\n",api_rx_el->data[0]);
						}
						stream_sync=1;
					}else{
						if (stream_sync){
							printf("OUT OF SYNC: %x\n",api_rx_el->data[0]);
						}
					}
#endif					

					if ((files_used & RX_FILE_USED) && rx_fd){
						fwrite((void*)&Rx_data[sizeof(api_rx_hdr_t)],
						       sizeof(char),
						       Rx_lgth,
						       rx_fd);	
					}
				
					++Rx_count;

					if (verbose){
						printf("Received %i Olen=%i Length = %i\n", 
								Rx_count, err,Rx_lgth);
#if 1
						printf("Data: ");
                                                for(i=0;i<Rx_lgth; i ++) {
                                                        printf("0x%02X ", api_rx_el->data[i]);
                                                }
                                                printf("\n");
#endif
					}else{
						//putchar('R');
					}

					if (rx_cnt > 0  && Rx_count >= rx_cnt){
						break;
					}
	
				} else {
					printf("\nError receiving data\n");
					break;
				}

			
bitstrm_skip_read:
;

		   	}
		
		   	if (FD_ISSET(sock,&write)){

				//err = send(sock,Tx_data, Tx_length + sizeof(api_tx_hdr_t), 0);
				err=Tx_length + sizeof(api_tx_hdr_t);
				if (err <= 0){
					if (errno == EBUSY){
						if (verbose){
							printf("Sock busy try again!\n");
						}
						/* Socket busy try sending again !*/
					}else{
				 		/* Check socket state */
						err = ioctl(sock,SIOC_WANPIPE_SOCK_STATE,0);
						printf("Sock state is %s\n",
							(err == 0) ? "CONNECTED" : 
							(err == 1) ? "DISCONNECTED" :
							 "CONNECTING");

						printf("Faild to send %i \n",errno);
						perror("Send: ");
						break;
					}
				}else{

					++Tx_count;
					
					if (verbose){
						printf("Packet sent: Sent %i : %i\n",
							err,Tx_count);
					}else{
						//putchar('T');
					}
					
					if ((files_used & TX_FILE_USED) && tx_fd){
						rlen=fread((void*)&Tx_data[sizeof(api_tx_hdr_t)],
							   sizeof(char),
							   Tx_length,tx_fd);
						if (!rlen){
							printf("\nTx of file %s is done!\n",
								tx_file);	
							break;	
						}
						if (Tx_length != rlen){
							Tx_length = rlen;
						}
					}
				}
		   	}

			if (tx_delay){
				usleep(tx_delay);
			}

			if (tx_cnt && tx_size && Tx_count >= tx_cnt && !(files_used & TX_FILE_USED)){
			
				write_enable=0;
				if (rx_cnt > 0){
					/* Dont break let rx finish */
				}else{
					break;
				}
			}
		}
	}

	printf("Press any key...");
	getchar();getchar();

	event_ctrl(WP_API_EVENT_DISABLE, argc, argv);
	
	if (tx_fd){
		fclose(tx_fd);
	}
	if (rx_fd){
		fclose(rx_fd);
	}
	close (sock);
} 


/***************************************************************
 * Main:
 *
 *    o Make a socket connection to the driver.
 *    o Call handle_socket() to read the socket 
 *
 **************************************************************/


int main(int argc, char* argv[])
{
	int proceed;

	proceed=init_args(argc,argv);
	if (proceed != WAN_TRUE){
		usage(argv[0]);
		return -1;
	}
	
	signal(SIGINT,&sig_end);
	
	proceed = MakeConnection();
	if (proceed == WAN_TRUE){
		
		handle_socket(argc, argv);
		return 0;
	}

	return 0;
};


void sig_end(int sigid)
{

	printf("Got Signal %i\n",sigid);

	if (tx_fd){
		fclose(tx_fd);
	}
	if (rx_fd){
		fclose(rx_fd);
	}

	if (sock){
		close (sock);
	}

	exit(1);
}

