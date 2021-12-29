/*****************************************************************************
* chdlc_api.c	CHDLC API: Receive Module
*
* Author(s):	Gideon Hack & Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 1995-2001 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Description:
*
* 	The chdlc_api.c utility will bind to a socket to a chdlc network
* 	interface, and continously tx and rx packets to an from the sockets.
*
*	This example has been written for a single interface in mind, 
*	where the same process handles tx and rx data.
*
*	A real world example, should use different processes to handle
*	tx and rx spearately.  
*/



#include <stdlib.h>
#include <stdio.h>
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
#include <linux/wanpipe.h>
#include <linux/sdla_aft_te1.h>
#include "lib_api.h"

#define FALSE	0
#define TRUE	1

#define LGTH_CRC_BYTES	0
#define MAX_TX_DATA     5000	/* Size of tx data */  
#define MAX_RX_DATA 	MAX_TX_DATA


#define SOCK_TIMEOUT 1


/*===================================================
 * Golobal data
 *==================================================*/

unsigned char HDLC_streaming = FALSE;
unsigned short Rx_lgth;

unsigned char Rx_data[MAX_RX_DATA];
unsigned char Tx_data[MAX_TX_DATA];
int 	sock;


/*=================================================== 
 * Function Prototypes 
 *==================================================*/
int MakeConnection(void);
void handle_socket( void);




/*===================================================
 * MakeConnection
 *
 *   o Create a Socket
 *   o Bind a socket to a wanpipe network interface
 *       (Interface name is supplied by the user)
 *==================================================*/         

int MakeConnection(void) 
{
	struct wan_sockaddr_ll 	sa;

	memset(&sa,0,sizeof(struct wan_sockaddr_ll));
	errno = 0;
   	sock = socket(AF_WANPIPE, SOCK_RAW, 0);
   	if( sock < 0 ) {
      		perror("Socket");
      		return(FALSE);
   	} /* if */
  
	printf("\nConnecting to router %s, interface %s\n", card_name, if_name);

	strcpy( (char*)sa.sll_device, if_name);
	strcpy( (char*)sa.sll_card, card_name);
	sa.sll_protocol = htons(PVC_PROT);
	sa.sll_family=AF_WANPIPE;

        if(bind(sock, (struct sockaddr *)&sa, sizeof(struct wan_sockaddr_ll)) < 0){
                perror("bind");
		printf("Failed to bind a socket to %s interface\n",if_name);
                exit(0);
        }
	printf("Socket bound to %s\n\n",if_name);

	return(TRUE);

}


/*===========================================================
 * handle_socket 
 *
 *   o Tx/Rx data to and from the socket 
 *   o Cast received data to an api_rx_element_t data type 
 *   o The received packet contains 16 bytes header 
 *
 *	------------------------------------------
 *      |  16 bytes      |        X bytes        ...
 *	------------------------------------------
 * 	   Header              Data
 *
 * RX DATA:
 * --------
 * 	Each rx data packet contains the 16 byte header!
 * 	
 *   	o Rx 16 byte data structure:
 *
 *		typedef struct {
 *       	 	unsigned char   error_flag      PACKED;
 *        		unsigned short  time_stamp      PACKED;
 *        		unsigned char   reserved[13]    PACKED;
 *		} api_rx_hdr_t; 
 *
 *		typedef struct {
 *        		api_rx_hdr_t    api_rx_hdr      PACKED;
 *        		void *          data            PACKED;
 *		} api_rx_element_t;
 *
 * 	error_flag:
 * 		bit 0: 	incoming frame was aborted
 * 		bit 1: 	incoming frame has a CRC error
 * 		bit 2:  incoming frame has an overrun eror 
 *
 * 	time_stamp:
 * 		absolute time value in ms.
 *
 * TX_DATA:
 * --------
 *	Each tx data packet MUST contain a 16 byte header!
 *
 * 	o Tx 16 byte data structure
 * 	
 *		typedef struct {
 *			unsigned char 	attr		PACKED;
 *			unsigned char  	reserved[15]	PACKED;
 *		} api_tx_hdr_t;
 *
 *		typedef struct {
 *			api_tx_hdr_t 	api_tx_hdr	PACKED;
 *			void *		data		PACKED;
 *		} api_tx_element_t;
 *
 *	Currently the chdlc device driver doesn't use any of
 *	the above fields.  Thus, the user can set the 16 bytes
 *	to ZERO.
 * 
 */

void handle_socket(void) 
{
	unsigned int Rx_count,Tx_count,Tx_length;
	api_tx_element_t * api_tx_el;
	fd_set 	ready,write,oob;
	int err;
	struct timeval tv;
	tv.tv_usec = 0; 
	tv.tv_sec = SOCK_TIMEOUT;


        Rx_count = 0;
	Tx_count = 0;
	Tx_length = tx_size;

	printf("\n\nSocket Handler: Rx=%d Tx=%i TxCnt=%i TxLen=%i TxDelay=%i RxCnt=%i\n",
			read_enable,write_enable,tx_cnt,tx_size,tx_delay,rx_cnt);	
	
	/* If running HDLC_STREAMING then the received CRC bytes
         * will be passed to the application as part of the 
         * received data.  
	 */

	api_tx_el = (api_tx_element_t*)&Tx_data[0];
	memset(&Tx_data[0],0,MAX_TX_DATA);

	while (1) {
		err = ioctl(sock,SIOC_WANPIPE_SOCK_STATE,0);
        	printf("Interface %s state is %s (%d)\n",
		   if_name,
                  (err == 0) ? "CONNECTED" :
                  (err == 1) ? "DISCONNECTED" :
                     "CONNECTING",err);

		if (err < 0) {
			printf("Error interface down %s sock disconnected! (%s)\n",
					if_name,strerror(errno));
			return;
		}

		if (err > 0) {
			printf("Waiting for interface %s to come up!\n",if_name);
			sleep(10);
		} else {
			break;
		}
	}

	for(;;) {	
		FD_ZERO(&ready);
		FD_ZERO(&write);
		FD_ZERO(&oob);
		FD_SET(sock,&oob);
		FD_SET(sock,&ready);

		tv.tv_usec = 0; 
		tv.tv_sec = SOCK_TIMEOUT;

		if (write_enable){
		     FD_SET(sock,&write);
		}

		fflush(stdout);
  		err= select(sock + 1,&ready, NULL, &oob, NULL);
	
		if (err < 0) {
			printf("Error: inteface down: %s socket disconnected %s\n",
					if_name,strerror(errno));
			break;

		} else if (err == 0) {		
			/* Timeout do something */
	
  		} else {
	
		   	if (FD_ISSET(sock,&oob)){
		
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
					
				printf("Got OOB exception: Link Down !\n");
				break;
			}
		  
			
                   	if (FD_ISSET(sock,&ready)){

				err = recv(sock, Rx_data, MAX_RX_DATA, 0);

				/* err indicates bytes received */
				if (err > 0){
					Rx_count++;
					err = send(sock, Rx_data, err, 0);
					Tx_count++;
					printf("Rx Len=%i  Rx=%i  Tx=%i : Echo Ok\r",
							err-sizeof(api_rx_hdr_t), Tx_count,Rx_count);
				} else {
					printf("\nError receiving data\n");
					break;
				}

		   	}
		}
	}

	close (sock);
} 

/***************************************************************
 * Main:
 *
 *    o Make a socket connection to the driver.
 *    o Call handle_socket() to read/write the socket 
 *
 **************************************************************/


int main(int argc, char* argv[])
{
	int proceed;

	proceed=init_args(argc,argv);
	if (proceed != WAN_TRUE){
		usage((unsigned char*)argv[0]);
		return -1;
	}

	proceed = MakeConnection();
	if(proceed == WAN_TRUE){
		handle_socket();
		return 0;
	}

	return 1;
};
