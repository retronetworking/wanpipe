/******************************************************************************//**
 * \file sample.c
 * \brief WANPIPE(tm) API C Sample Code
 *
 * Authors: David Rokhvarg <davidr@sangoma.com>
 *			Nenad Corbic <ncorbic@sangoma.com>
 *
 * Copyright (c) 2007 - 08, Sangoma Technologies
 * All rights reserved.
 *
 * * Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Sangoma Technologies nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Sangoma Technologies ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Sangoma Technologies BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * ===============================================================================
 *
 */

#include "libsangoma.h"
#include "lib_api.h"		

u_int32_t		poll_events_bitmap = 0;

/*!
  \def TEST_NUMBER_OF_OBJECTS
  \brief Number of wait objects to define in object array.

  Objects are used to wait on file descripotrs.
  Usually there is a one wait object per file descriptor.

  In this example there is one a single file descriptor
*/
#define TEST_NUMBER_OF_OBJECTS 32

sangoma_wait_obj_t sangoma_wait_objects[TEST_NUMBER_OF_OBJECTS];
wp_api_element_t Rx_data[TEST_NUMBER_OF_OBJECTS];
wp_api_element_t Tx_data[TEST_NUMBER_OF_OBJECTS];

/* IOCTL management structures and variables */
/* Warning: non-thread safe globals. Ok for an example but not in production. */
wan_udp_hdr_t	wan_udp;
wanpipe_api_t	tdm_api; 

unsigned char rx_rbs_bits = WAN_RBS_SIG_A;

FILE	*pRxFile;

/*****************************************************************
 * Prototypes
 *****************************************************************/

int __cdecl main(int argc, char* argv[]);
int open_sangoma_devices(int *open_device_counter);
void handle_span_chan(int open_device_counter);
int handle_tdm_event(uint32_t dev_index);
int handle_data(uint32_t	dev_index);
int read_data(uint32_t dev_index);
int write_data(uint32_t dev_index, wp_api_hdr_t *tx_hdr, void *tx_data);
int dtmf_event(sng_fd_t fd,unsigned char digit,unsigned char type,unsigned char port);
int rbs_event(sng_fd_t fd,unsigned char rbs_bits);
int rxhook_event(sng_fd_t fd,unsigned char hook_state);
int rxring_event(sng_fd_t fd,unsigned char ring_state);
int ringtrip_event (sng_fd_t fd, unsigned char ring_state);
void print_rx_data(unsigned char *data, int	datalen);
int write_data_to_file(unsigned char *data,	unsigned int data_length);
void cleanup(int interface_no);

#ifdef WIN32
BOOL TerminateHandler(DWORD dwCtrlType);
#else
void TerminateHandler(int);
#endif

/*****************************************************************
 * General Functions
 *****************************************************************/

/*!
  \fn void print_rx_data(unsigned char *data, int	datalen)
  \brief  Prints the contents of data packet
  \param data pointer to data buffer
  \param datalen size of data buffer
  \return void
*/
void print_rx_data(unsigned char *data, int	datalen)
{
	int i;

	printf("Data: (Len=%i)\n",datalen);
	for(i = 0; i < datalen; i++) {
		if((i % 20 == 0)){
			if(i){
				printf("\n");
			}
		}
		printf("%02X ", data[i]);
#if 0
		/* don't print too much!! */
		if(i > 100){
			printf("...\n");
			break;
		}
#endif
	}
	printf("\n");
}

/*!
  \fn int read_data(uint32_t dev_index)
  \brief Read data buffer from a device
  \param dev_index device index number associated with device file descriptor
  \return 0 - Ok otherwise Error
 
*/
int read_data(uint32_t dev_index)
{
	wp_api_element_t	*rx_el	= &Rx_data[dev_index];
	wp_api_hdr_t		*rx_hdr = &rx_el->hdr;
	sng_fd_t			dev_fd	= sangoma_wait_objects[dev_index].fd;
	int					Rx_lgth = 0;
	static int			Rx_count= 0;

	memset(rx_hdr, 0, sizeof(wp_api_hdr_t));

	/* read the message */
	Rx_lgth = sangoma_readmsg(
					dev_fd,
					rx_hdr,						/* header buffer */
					sizeof(wp_api_hdr_t),		/* header size */
					rx_el->data,				/* data buffer */
					MAX_NO_DATA_BYTES_IN_FRAME,	/* data BUFFER size */
					0);   
	if(Rx_lgth <= 0) {
		printf("Span: %d, Chan: %d: Error receiving data!\n", 
			sangoma_wait_objects[dev_index].span, sangoma_wait_objects[dev_index].chan);
		return 1;
	}

	if (verbose){
		print_rx_data(rx_el->data, Rx_lgth);
	}
			
	/* use Rx_counter as "write" events trigger: */
	if(rbs_events == 1 && (Rx_count % 400) == 0){
		/*	bitmap - set as needed: WAN_RBS_SIG_A | WAN_RBS_SIG_B | WAN_RBS_SIG_C | WAN_RBS_SIG_D;
		
			In this example make bits A and B to change each time,
			so it's easy to see the change on the receiving side.
		*/
		if(rx_rbs_bits == WAN_RBS_SIG_A){
			rx_rbs_bits = WAN_RBS_SIG_B;
		}else{
			rx_rbs_bits = WAN_RBS_SIG_A;
		}
		printf("Writing RBS bits (0x%X)...\n", rx_rbs_bits);
		sangoma_tdm_write_rbs(dev_fd, &tdm_api, sangoma_wait_objects[dev_index].chan, rx_rbs_bits);
	}

	/* if user needs Rx data to be written into a file: */
	if(files_used & RX_FILE_USED){
		write_data_to_file(rx_el->data, Rx_lgth);
	}

	return 0;
}

/*!
  \fn int write_data(uint32_t dev_index, wp_api_hdr_t *tx_hdr, void *tx_data)
  \brief Transmit a data buffer to a device.
  \param dev_index device index number associated with device file descriptor
  \param tx_hdr pointer to a wp_api_hdr_t
  \param tx_data pointer to a data buffer 
  \return 0 - Ok otherwise Error
*/
int write_data(uint32_t dev_index, wp_api_hdr_t *tx_hdr, void *tx_data)
{
	sng_fd_t	dev_fd	= sangoma_wait_objects[dev_index].fd;
	int			err;
	static int	Tx_count = 0;

	/* write a message */
	err = sangoma_writemsg(
				dev_fd,
				tx_hdr,					/* header buffer */
				sizeof(wp_api_hdr_t),	/* header size */
				tx_data,				/* data buffer */
				tx_hdr->data_length,	/* DATA size */
				0);

	if (err <= 0){
		printf("Span: %d, Chan: %d: Failed to send!\n", 
			sangoma_wait_objects[dev_index].span, sangoma_wait_objects[dev_index].chan);
		return -1;
	}
	
	Tx_count++;
	if (verbose){
		printf("Packet sent: counter: %i, len: %i\n", Tx_count, err);
	}else{
		if(Tx_count && (!(Tx_count % 1000))){
			printf("Packet sent: counter: %i, len: %i\n", Tx_count, err);
		}
	}

#if 0
	if(Tx_count >= tx_cnt){
		write_enable=0;
		printf("Disabling POLLOUT...\n");
		/* No need for POLLOUT, turn it off!! If not turned off, and we
		 * have nothing for transmission, sangoma_socket_waitfor() will return
		 * immediately, creating a busy loop. */
		sangoma_wait_objects[dev_index].flags_in &= (~POLLOUT);
	}
#endif
	return 0;
}

/*!
  \fn int handle_data(uint32_t	dev_index)
  \brief Read data buffer from the device and transmit it back down.
  \param dev_index device index number associated with device file descriptor
  \return 0 - Ok  otherwise Error

   Read data buffer from a device. After a successful read, transmit
   the same data down the device.  This function will have an ECHO effect.
   Everything received will be transmitted down.
*/
int handle_data(uint32_t	dev_index)
{
	wp_api_element_t	*rx_el			= &Rx_data[dev_index];
	wp_api_element_t	*tx_el			= &Tx_data[dev_index];
	uint32_t			api_poll_status = sangoma_wait_objects[dev_index].flags_out;

	memset(rx_el, 0, sizeof(wp_api_element_t));

#if 0
	printf("%s(): span: %d, chan: %d\n", __FUNCTION__,
		sangoma_wait_objects[dev_index].span, sangoma_wait_objects[dev_index].chan);
#endif

	if(api_poll_status & POLLIN){

		if(read_data(dev_index) == 0){

			if(rx2tx){
				/* Send back received data (create a "software loopback"), just a test. */
				write_data(dev_index, &rx_el->hdr, rx_el->data);
			}
		}
	}

	if((api_poll_status & POLLOUT) && write_enable){
	
		wp_api_hdr_t *api_tx_hdr = &tx_el->hdr;
		uint16_t Tx_length = 128;/* 128 is just an example */
		static unsigned char tx_test_byte = 0;

		api_tx_hdr->data_length = Tx_length; 
		memset(tx_el->data, tx_test_byte, Tx_length);
					
		if(write_data(dev_index, api_tx_hdr, tx_el->data) == 0){

			tx_test_byte++;
		}

	}/* if() */
	return 0;
}

/*!
  \fn int decode_api_event (wp_api_event_t *wp_tdm_api_event, sangoma_wait_obj_t *sangoma_wait_obj)
  \brief Handle API Event
  \param wp_tdm_api_event
  \param sangoma_wait_obj
*/
static void decode_api_event(wp_api_event_t *wp_tdm_api_event, sangoma_wait_obj_t *sangoma_wait_obj)
{ 
	printf("decode_api_event(): span: %d, chan: %d\n", wp_tdm_api_event->span, wp_tdm_api_event->channel);

	switch(wp_tdm_api_event->wp_api_event_type)
	{
	case WP_API_EVENT_DTMF:/* DTMF detected by Hardware */
		printf("DTMF Event: Channel: %d, Digit: %c (Port: %s, Type:%s)!\n",
			wp_tdm_api_event->channel,
			wp_tdm_api_event->wp_api_event_dtmf_digit,
			(wp_tdm_api_event->wp_api_event_dtmf_port == WAN_EC_CHANNEL_PORT_ROUT)?"ROUT":"SOUT",
			(wp_tdm_api_event->wp_api_event_dtmf_type == WAN_EC_TONE_PRESENT)?"PRESENT":"STOP");
		break;

	case WP_API_EVENT_RXHOOK:
		printf("RXHOOK Event: Channel: %d, %s! (0x%X)\n", 
			wp_tdm_api_event->channel,
			WAN_EVENT_RXHOOK_DECODE(wp_tdm_api_event->wp_api_event_hook_state),
			wp_tdm_api_event->wp_api_event_hook_state);
		break;

	case WP_API_EVENT_RING_DETECT:
		printf("RING Event: %s! (0x%X)\n",
			WAN_EVENT_RING_DECODE(wp_tdm_api_event->wp_api_event_ring_state),
			wp_tdm_api_event->wp_api_event_ring_state);
		break;

	case WP_API_EVENT_RING_TRIP_DETECT:
		printf("RING TRIP Event: %s! (0x%X)\n", 
			WAN_EVENT_RING_TRIP_DECODE(wp_tdm_api_event->wp_api_event_ring_state),
			wp_tdm_api_event->wp_api_event_ring_state);
		break;

	case WP_API_EVENT_RBS:
		printf("RBS Event: Channel: %d, 0x%X!\n",
			wp_tdm_api_event->channel,
			wp_tdm_api_event->wp_api_event_rbs_bits);
		printf( "RX RBS: A:%1d B:%1d C:%1d D:%1d\n",
			(wp_tdm_api_event->wp_api_event_rbs_bits & WAN_RBS_SIG_A) ? 1 : 0,
			(wp_tdm_api_event->wp_api_event_rbs_bits & WAN_RBS_SIG_B) ? 1 : 0,
			(wp_tdm_api_event->wp_api_event_rbs_bits & WAN_RBS_SIG_C) ? 1 : 0,
			(wp_tdm_api_event->wp_api_event_rbs_bits & WAN_RBS_SIG_D) ? 1 : 0);
		break;

	case WP_API_EVENT_LINK_STATUS:
		printf("Link Status Event: %s! (0x%X)\n", 
			WAN_EVENT_LINK_STATUS_DECODE(wp_tdm_api_event->wp_api_event_link_status),
			wp_tdm_api_event->wp_api_event_link_status);
		break;

	case WP_API_EVENT_ALARM:
		printf("New Alarm State: %s! (0x%X)\n", (wp_tdm_api_event->wp_api_event_alarm == 0?"Off":"On"),
			wp_tdm_api_event->wp_api_event_alarm);
		break;

	case WP_API_EVENT_POLARITY_REVERSE:
		printf("Polarity Reversal Event : %s! (0x%X)\n", 
			WP_API_EVENT_POLARITY_REVERSE_DECODE(wp_tdm_api_event->wp_api_event_polarity_reverse),
			wp_tdm_api_event->wp_api_event_polarity_reverse);
		break;

	default:
		printf("Unknown TDM API Event: %d\n", wp_tdm_api_event->wp_api_event_type);
		break;
	}
}

/*!
  \fn int handle_tdm_event(uint32_t dev_index)
  \brief Read Event buffer from the device
  \param dev_index device index number associated with device file descriptor
  \return 0 - Ok  otherwise Error

   An EVENT has occoured. Execute a system call to read the EVENT
   on a device.
*/
int handle_tdm_event(uint32_t dev_index)
{
	sng_fd_t	dev_fd = sangoma_wait_objects[dev_index].fd;
#if 0
	printf("sangoma_wait_objects[%d].flags_out:", dev_index);
	print_poll_event_bitmap(sangoma_wait_objects[dev_index].flags_out);
	printf("\n");
#endif

	if(sangoma_read_event(dev_fd, &tdm_api)){
		return 1;
	}

	decode_api_event(&tdm_api.wp_cmd.event, &sangoma_wait_objects[dev_index]);

	return 0;
}

/*!
  \fn void handle_span_chan(int open_device_counter)
  \brief Write data buffer into a file
  \param open_device_counter number of opened devices
  \return void

  This function will wait on all opened devices.
  This example will wait for RX and EVENT signals.
  In case of POLLIN - rx data available
  In case of POLLPRI - event is available
*/
void handle_span_chan(int open_device_counter)
{
	int	iResult, i;

	printf("\n\nSpan/Chan Handler: RxEnable=%s, TxEnable=%s, TxCnt=%i, TxLen=%i\n",
		(read_enable? "Yes":"No"), (write_enable?"Yes":"No"),tx_cnt,tx_size);	

	/* Main Rx/Tx/Event loop */
	for(;;) 
	{	
		iResult = sangoma_socket_waitfor_many(sangoma_wait_objects, 
							open_device_counter /* number of wait objects */,
							2000/* 2 sec wait */);
			
		if(iResult < 0){
			/* error */
			printf("Error: iResult: %d\n", iResult);
			break;
		}

		if(iResult == 0){
			/* timeout */
			printf("Timeout\n");
			continue;
		}

		for(i = 0; i < open_device_counter; i++){

			if(sangoma_wait_objects[i].flags_out & POLLPRI){
				/* got tdm api event */
				if(handle_tdm_event(i)){
					printf("Error in handle_tdm_event()!\n");
				}
			}
				
			if(sangoma_wait_objects[i].flags_out & POLLIN){
				/* got data */
				if(handle_data(i)){
					printf("Error in handle_data()!\n");
				}
			}

		}/* for() */

	}/* for() */
}

/*!
  \fn int write_data_to_file(unsigned char *data, unsigned int data_length)
  \brief Write data buffer into a file
  \param data data buffer
  \param data_length length of a data buffer
  \return data_length = ok  otherwise error

*/
int write_data_to_file(unsigned char *data, unsigned int data_length)
{
	if(pRxFile == NULL){
		return 1;
	}

	return fwrite(data, 1, data_length, pRxFile);
}

#ifdef WIN32
/*
 * TerminateHandler() - this handler is called by the system whenever user tries to terminate
 *						the process with Ctrl+C, Ctrl+Break or closes the console window.
 *						Perform a clean-up here.
 */
BOOL TerminateHandler(DWORD dwCtrlType)
{
	int i;

	printf("\nProcess terminated by user request.\n");

	/* do the cleanup before exiting: */
	for(i = 0; i < TEST_NUMBER_OF_OBJECTS; i++){
		cleanup(i);
	}

	/* return FALSE so the system will call the dafult handler which will terminate the process. */
	return FALSE;
}
#else
/*!
  \fn void TerminateHandler (int sig)
  \brief Signal handler for graceful shutdown
  \param sig signal 
*/
void TerminateHandler (int sig)
{
	int i;

	printf("\nProcess terminated by user request.\n");

	/* do the cleanup before exiting: */
	for(i = 0; i < TEST_NUMBER_OF_OBJECTS; i++){
		cleanup(i);
	}

	return;
}

#endif

/*!
  \fn void cleanup(int dev_no)
  \brief Protperly shutdown single device
  \param dev_no device index number
  \return void

*/
void cleanup(int dev_no)
{
	printf("cleanup()...\n");

	if(dtmf_enable_octasic == 1){
		/* Disable dtmf detection on Octasic chip */
		sangoma_tdm_disable_dtmf_events(sangoma_wait_objects[dev_no].fd, &tdm_api);
	}

	if(dtmf_enable_remora == 1){
		/* Disable dtmf detection on Sangoma's Remora SLIC chip */
		sangoma_tdm_disable_rm_dtmf_events(sangoma_wait_objects[dev_no].fd, &tdm_api);
	}

	if(remora_hook == 1){
		sangoma_tdm_disable_rxhook_events(sangoma_wait_objects[dev_no].fd, &tdm_api);
	}

	if(rbs_events == 1){
    	sangoma_tdm_disable_rbs_events(sangoma_wait_objects[dev_no].fd, &tdm_api);
	}

	/* call sangoma_close() for EACH open Device Handle */
	sangoma_close(&sangoma_wait_objects[dev_no].fd);
}


/*!
  \fn int open_sangoma_devices(int *open_device_counter)
  \brief Open a single span chan device and return number of devices opened
  \param open_device_counter returns the number of devices opened
  \return 0 ok otherise error.

  This function will open a single span chan.

  However it can be rewritten to iterate for all spans and chans and  try to
  open all existing wanpipe devices.

  For each opened device, a wait object will be initialized.
  For each device, configure the chunk size for tx/rx
                   enable events such as DTMF/RBS ...etc
*/
int open_sangoma_devices(int *open_device_counter)
{
	int i, span, chan, err = -1, open_dev_cnt = 0;
	sng_fd_t	dev_fd = INVALID_HANDLE_VALUE;

	*open_device_counter = 0;

	span = wanpipe_port_no;
	chan = wanpipe_if_no;

	/* span and chan are 1-based */
	dev_fd = sangoma_open_api_span_chan(span, chan);
	if( dev_fd == INVALID_HANDLE_VALUE){
		printf("Warning: Failed to open span %d, chan %d\n", span , chan);
		return 1;
	}else{
		printf("Successfuly opened span %d, chan %d\n", span , chan);
	}

	if(sangoma_init_wait_obj(&sangoma_wait_objects[open_dev_cnt], dev_fd, span, chan, poll_events_bitmap, SANGOMA_WAIT_OBJ)){
		printf("Error: Failed to initialize 'sangoma_wait_object' for span %d, chan %d\n", span, chan);
		return 1;
	}
	open_dev_cnt++;

	*open_device_counter = open_dev_cnt;
	if(0 == open_dev_cnt){
		return 1;
	}

	for(i = 0; i < open_dev_cnt; i++){

		printf("HANDLING SPAN %i CHAN %i\n", sangoma_wait_objects[i].span, sangoma_wait_objects[i].chan);

		dev_fd = sangoma_wait_objects[i].fd;

		if((err=sangoma_get_full_cfg(dev_fd, &tdm_api))){
			break;
		}

		if(set_codec_slinear){
			printf("Setting SLINEAR codec\n");
			if((err=sangoma_tdm_set_codec(dev_fd, &tdm_api, WP_SLINEAR))){
				break;
			}
		}

		if(set_codec_none){
			printf("Disabling codec\n");
			if((err=sangoma_tdm_set_codec(dev_fd, &tdm_api, WP_NONE))){
				break;
			}
		}

		if(usr_period){
			printf("Setting user period: %d\n", usr_period);
			if((err=sangoma_tdm_set_usr_period(dev_fd, &tdm_api, usr_period))){
				break;
			}
		}

		if(set_codec_slinear || usr_period || set_codec_none){
			/* display new configuration AFTER it was changed */
			if((err=sangoma_get_full_cfg(dev_fd, &tdm_api))){
				break;
			}
		}

		if(dtmf_enable_octasic == 1){
			poll_events_bitmap |= POLLPRI;
			/* enable dtmf detection on Octasic chip */
			if((err=sangoma_tdm_enable_dtmf_events(dev_fd, &tdm_api))){
				break;
			}
		}

		if(dtmf_enable_remora == 1){
			poll_events_bitmap |= POLLPRI;
			/* enable dtmf detection on Sangoma's Remora SLIC chip (A200 ONLY) */
			if((err=sangoma_tdm_enable_rm_dtmf_events(dev_fd, &tdm_api))){
				break;
			}
		}

		if(remora_hook == 1){
			poll_events_bitmap |= POLLPRI;
			if((err=sangoma_tdm_enable_rxhook_events(dev_fd, &tdm_api))){
				break;
			}
		}

		if(rbs_events == 1){
			poll_events_bitmap |= POLLPRI;
    		if((err=sangoma_tdm_enable_rbs_events(dev_fd, &tdm_api, 20))){
				break;
			}
		}

		printf("Device Config RxQ=%i TxQ=%i \n",
				sangoma_get_rx_queue_sz(dev_fd,&tdm_api),
				sangoma_get_rx_queue_sz(dev_fd,&tdm_api));

		sangoma_set_rx_queue_sz(dev_fd,&tdm_api,20);
		sangoma_set_tx_queue_sz(dev_fd,&tdm_api,30);

		printf("Device Config RxQ=%i TxQ=%i \n",
				sangoma_get_rx_queue_sz(dev_fd,&tdm_api),
				sangoma_get_tx_queue_sz(dev_fd,&tdm_api));
	}
 
	printf("Enabling Poll Events:\n");
#ifdef WIN32
	print_poll_event_bitmap(poll_events_bitmap);
#endif
	return err;
}

/*!
  \fn void close_sangoma_devices(void)
  \brief Close all opened devices 
*/
void close_sangoma_devices(void)
{
	int i;

	for(i = 0; i < TEST_NUMBER_OF_OBJECTS; i++){
		if(sangoma_wait_objects[i].fd != INVALID_HANDLE_VALUE){
			cleanup(i);
		}
	}
}

/*!
  \fn int __cdecl main(int argc, char* argv[])
  \brief Main function that starts the sample code
  \param argc  number of arguments
  \param argv  argument list
*/
int __cdecl main(int argc, char* argv[])
{
	int proceed, i, open_device_counter;

	proceed=init_args(argc,argv);
	if (proceed != WAN_TRUE){
		usage(argv[0]);
		return -1;
	}

	/* register Ctrl+C handler - we want a clean termination */
#if defined(__WINDOWS__)
	if (!SetConsoleCtrlHandler(TerminateHandler, TRUE)) {
		printf("ERROR : Unable to register terminate handler ( %d ).\nProcess terminated.\n", 
			GetLastError());
		return -1;
	}
#else
	signal(SIGHUP,TerminateHandler);
	signal(SIGTERM,TerminateHandler);
#endif

	for(i = 0; i < TEST_NUMBER_OF_OBJECTS; i++){
		sangoma_wait_objects[i].fd = INVALID_HANDLE_VALUE;
	}

	poll_events_bitmap = 0;
	if(read_enable	== 1){
		poll_events_bitmap |= POLLIN;
	}
	
	if(write_enable == 1){
		poll_events_bitmap |= POLLOUT;
	}

	/* Front End connect/disconnect, and other events, such as DTMF... */
	poll_events_bitmap |= (POLLHUP | POLLPRI);

	printf("Connecting to Port/Span: %d, Interface/Chan: %d\n",
		wanpipe_port_no, wanpipe_if_no);

	memset(&tdm_api,0,sizeof(tdm_api));

	if(open_sangoma_devices(&open_device_counter)){
		return -1;
	}

	printf("********************************\n");
	printf("files_used: 0x%x\n", files_used);
	printf("********************************\n");
	if(files_used & RX_FILE_USED){
		pRxFile = fopen( (const char*)&rx_file[0], "wb" );
		if(pRxFile == NULL){
			printf("Can't open Rx file: [%s]!!\n", rx_file);
		}else{
			printf("Open Rx file: %s. OK.\n", rx_file);
		}
	}

	handle_span_chan(open_device_counter);

	/* returned from main loop, do the cleanup before exiting: */
	close_sangoma_devices();

	printf("\nSample application exiting.(press any key)\n");
	_getch();
	return 0;
}
