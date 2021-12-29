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


#define MAX_STACK_SIZE 1024 * 240
#define MAX_SPANS 32
#define MAX_CHANS 32

#define _printf(a,...) 

static int max_spans = MAX_SPANS;
static int max_chans = MAX_CHANS;
static int server_running=0;
static int threads_running=0;
pthread_mutex_t threads_running_lock;

typedef struct {

	int init;
	int span;
	int chan;

	int poll_events_bitmap;
	sangoma_wait_obj_t sangoma_wait_object;
	pthread_t threadid;

	wp_api_element_t Rx_data;
	wp_api_element_t Tx_data;
	wanpipe_api_t	 tdm_api;
    wan_udp_hdr_t	wan_udp;
	unsigned char rx_rbs_bits;
	FILE	*pRxFile;
	FILE	*pTxFile;

	char name[100];

	unsigned int errors;
	unsigned int rx_errors;
	unsigned int rx_bytes;
	unsigned int rx_packets;

	unsigned int tx_bytes;
	unsigned int tx_errors;
	unsigned int tx_packets;
	unsigned int rx_events;
	unsigned int rx_events_tone;

	unsigned int loop_cnt;
	unsigned int tx_file_cnt;
                             
	unsigned char prev_dtmf[MAX_CHANS];
	unsigned int test_failed;

	struct timeval tv_start;
	int    elapsed_max;
	int    elapsed_min;

	unsigned char rx_sync;
	unsigned char tx_sync;
	struct timeval tv_sync_start;

	wanpipe_chan_stats_t stats;
}sangoma_channel_t;

sangoma_channel_t sangoma_channel_idx[MAX_SPANS+1][MAX_CHANS+1];

/*****************************************************************
 * Prototypes
 *****************************************************************/

int __cdecl main(int argc, char* argv[]);
int open_sangoma_device(sangoma_channel_t *schan);
static void *handle_span_chan(void *obj);
int handle_tdm_event(sangoma_channel_t *schan);
int handle_data(sangoma_channel_t *schan);
int read_data(sangoma_channel_t *schan);
int write_data(sangoma_channel_t *schan, wp_api_hdr_t *tx_hdr, void *tx_data);
int dtmf_event(sng_fd_t fd,unsigned char digit,unsigned char type,unsigned char port);
int rbs_event(sng_fd_t fd,unsigned char rbs_bits);
int rxhook_event(sng_fd_t fd,unsigned char hook_state);
int rxring_event(sng_fd_t fd,unsigned char ring_state);
int ringtrip_event (sng_fd_t fd, unsigned char ring_state);
void print_rx_data(unsigned char *data, int	datalen);
int write_data_to_file(sangoma_channel_t *schan, unsigned char *data, unsigned int data_length);
void close_sangoma_devices(sangoma_channel_t *schan);
static void sangoma_get_print_stats(sangoma_channel_t *schan);

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

#if 1
#ifdef __LINUX__
static int sample_time_test(sangoma_channel_t *schan){

	struct timeval last;
	int elapsed;
	int err;

	last=schan->tv_start;
	gettimeofday(&schan->tv_start, NULL);
	elapsed = abs((((last.tv_sec * 1000) + last.tv_usec / 1000) - ((schan->tv_start.tv_sec * 1000) + schan->tv_start.tv_usec / 1000)));
	err = abs(20-elapsed);
	if (err > 1 && err < 1000) {
		if (elapsed > schan->elapsed_max) {
			schan->elapsed_max = elapsed;
		}
		if (schan->elapsed_min > elapsed) {
         	schan->elapsed_min = elapsed;
		}
		printf("wanpipe%d: Elapsed %i  diff=%i max=%i min=%i\n", 
			schan->span, elapsed,abs(20-elapsed),schan->elapsed_max,schan->elapsed_min);
	}

	return 0;
}
#endif
#endif

/*!
  \fn int read_data(sangoma_channel_t *schan)
  \brief Read data buffer from a device
  \param dev_index device index number associated with device file descriptor
  \return 0 - Ok otherwise Error
 
*/
int read_data(sangoma_channel_t *schan)
{
	wp_api_element_t	*rx_el	= (wp_api_element_t*)&schan->Rx_data;
	wp_api_hdr_t		*rx_hdr = &rx_el->hdr;
	sng_fd_t			dev_fd	= schan->sangoma_wait_object.fd;
	int					Rx_lgth = 0;
	static int			Rx_count= 0;
	wanpipe_api_t		tdm_api;

	memset(&tdm_api,0,sizeof(tdm_api));
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
			schan->span, schan->chan);
		return 1;
	}
    
#ifdef __LINUX__
	sample_time_test(schan);
#endif

	schan->rx_packets++;
	schan->rx_bytes+=Rx_lgth;

	if (verbose){
		print_rx_data(rx_el->data, Rx_lgth);
	}
			
	/* use Rx_counter as "write" events trigger: */
	if(rbs_events == 1 && (Rx_count % 400) == 0){
		/*	bitmap - set as needed: WAN_RBS_SIG_A | WAN_RBS_SIG_B | WAN_RBS_SIG_C | WAN_RBS_SIG_D;
		
			In this example make bits A and B to change each time,
			so it's easy to see the change on the receiving side.
		*/
		if(schan->rx_rbs_bits == WAN_RBS_SIG_A){
			schan->rx_rbs_bits = WAN_RBS_SIG_B;
		}else{
			schan->rx_rbs_bits = WAN_RBS_SIG_A;
		}
		printf("Writing RBS bits (0x%X)...\n", schan->rx_rbs_bits);
		sangoma_tdm_write_rbs(dev_fd, &tdm_api, schan->chan, schan->rx_rbs_bits);
	}

	if (schan->tx_sync && !schan->rx_sync) {
		int i;
		for (i=0;i<Rx_lgth;i++) {
    		if (rx_el->data[i]==tx_data) {
				struct timeval last;
				int elapsed;
				gettimeofday(&last, NULL);
				elapsed = abs((((last.tv_sec * 1000) + last.tv_usec / 1000) - ((schan->tv_sync_start.tv_sec * 1000) + schan->tv_sync_start.tv_usec / 1000)));


				printf("wanpipe%d: Rx Sync Time %i ms Data=0x%X\n", 
						schan->span, elapsed, tx_data);
            	schan->rx_sync=1;
				break;
			}
		}
	}

	/* if user needs Rx data to be written into a file: */
	if(files_used & RX_FILE_USED){
		write_data_to_file(schan, rx_el->data, Rx_lgth);
	}

	return Rx_lgth;
}

/*!
  \fn int write_data(sangoma_channel_t *schan, wp_api_hdr_t *tx_hdr, void *tx_data)
  \brief Transmit a data buffer to a device.
  \param dev_index device index number associated with device file descriptor
  \param tx_hdr pointer to a wp_api_hdr_t
  \param tx_data pointer to a data buffer 
  \return 0 - Ok otherwise Error
*/
int write_data(sangoma_channel_t *schan, wp_api_hdr_t *tx_hdr, void *tx_data)
{
	sng_fd_t	dev_fd	= schan->sangoma_wait_object.fd;
	int			err;
	int			tx_len = tx_hdr->data_length;

	/* write a message */
	err = sangoma_writemsg(
				dev_fd,
				tx_hdr,					/* header buffer */
				sizeof(wp_api_hdr_t),	/* header size */
				tx_data,				/* data buffer */
				tx_len,	/* DATA size */
				0);

	if (err <= 0){
		printf("Span: %d, Chan: %d: Failed to send! (%s) len=%i\n", 
			schan->span, schan->chan, strerror(errno), tx_len);
		schan->tx_errors++;
		return -1;
	}

	schan->tx_packets++;
	schan->tx_bytes+=err;

	if (verbose){
		printf("Packet sent: counter: %i, len: %i\n", schan->tx_packets, err);
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
  \fn int handle_data(sangoma_channel_t *schan)
  \brief Read data buffer from the device and transmit it back down.
  \param dev_index device index number associated with device file descriptor
  \return 0 - Ok  otherwise Error

   Read data buffer from a device. After a successful read, transmit
   the same data down the device.  This function will have an ECHO effect.
   Everything received will be transmitted down.
*/
int handle_data(sangoma_channel_t *schan)
{
	wp_api_element_t	*rx_el			= &schan->Rx_data;
	wp_api_element_t	*tx_el			= &schan->Tx_data;
	uint32_t			api_poll_status = schan->sangoma_wait_object.flags_out;
	wanpipe_api_t		tdm_api;

	memset(&tdm_api,0,sizeof(tdm_api));
	memset(rx_el, 0, sizeof(wp_api_element_t));

#if 0
	printf("%s(): span: %d, chan: %d\n", __FUNCTION__,
		sangoma_wait_objects[dev_index].span, sangoma_wait_objects[dev_index].chan);
#endif

	if(api_poll_status & POLLIN){
		int err;
               
		err = read_data(schan);
		if(err > 0) {
			if(rx2tx){
				/* Send back received data (create a "software loopback"), just a test. */
				if (tx_data) {
					memset(rx_el->data, tx_data, err);
				}
				if (!schan->tx_sync) {
					schan->tx_sync=1;
					sangoma_flush_bufs(schan->sangoma_wait_object.fd,&tdm_api);
					sangoma_flush_stats(schan->sangoma_wait_object.fd,&tdm_api);
					gettimeofday(&schan->tv_sync_start, NULL);
				}
				err=write_data(schan, &rx_el->hdr, rx_el->data);
				if (err) {
					return err;
				}
			}
		}
	}

#if 0
	if((api_poll_status & POLLOUT) && write_enable){

		int err;
		wp_api_hdr_t *api_tx_hdr = &tx_el->hdr;
		uint16_t Tx_length = sangoma_tdm_get_usr_mtu_mru(schan->sangoma_wait_object.fd,&tdm_api);
		unsigned char tx_test_byte = tx_data;

		api_tx_hdr->data_length = Tx_length;

		if (schan->pTxFile == NULL) {
			memset(tx_el->data, tx_test_byte, Tx_length);
		} else {
			memset(tx_el->data, 0xFF, Tx_length);
			err=fread(tx_el->data, Tx_length, 1, schan->pTxFile);
			if (err == 0) {
				_printf("%s: Tx Whole File\n",schan->name);
				schan->tx_file_cnt++;
				rewind(schan->pTxFile);
			}
		}

		if (!schan->tx_sync) {
			schan->tx_sync=1;
			sangoma_flush_bufs(schan->sangoma_wait_object.fd,&tdm_api);
			sangoma_flush_stats(schan->sangoma_wait_object.fd,&tdm_api);
			gettimeofday(&schan->tv_sync_start, NULL);
		}

		err=write_data(schan, api_tx_hdr, tx_el->data);
		if (err != 0) {
			return err;
		}

	}/* if() */
#endif
	return 0;
}

static unsigned char get_next_expected_digit(unsigned char current_digit)
{
	switch(current_digit)
	{
	case '0':
		return '1';
	case '1':
		return '2';
	case '2':
		return '3';
	case '3':
		return '4';
	case '4':
		return '5';
	case '5':
		return '6';
	case '6':
		return '7';
	case '7':
		return '8';
	case '8':
		return '9';
	case '9':
		return 'A';
	case 'A':
		return 'B';
	case 'B':
		return 'C';
	case 'C':
		return 'D';
	case 'D':
		return '#';
	case '#':
		return '*';
	case '*':
		return '0';
	default:
		return '?';
	}
}



/*!
  \fn int decode_api_event(sangoma_channel_t *schan, wanpipe_api_t *tdm_api)
  \brief Handle API Event
  \param wp_tdm_api_event
  \param sangoma_wait_obj
*/
static int decode_api_event(sangoma_channel_t *schan, wanpipe_api_t *tdm_api)
{
	wp_api_event_t *wp_tdm_api_event = &tdm_api->wp_cmd.event;

	schan->rx_events++;

	//printf("decode_api_event(): span: %d, chan: %d\n", wp_tdm_api_event->span, wp_tdm_api_event->channel);

	switch(wp_tdm_api_event->wp_api_event_type)
	{
	case WP_API_EVENT_DTMF:/* DTMF detected by Hardware */

		schan->rx_events_tone++;

		if (dtmf_seq_check) {
			unsigned char digit = get_next_expected_digit(schan->prev_dtmf[wp_tdm_api_event->channel]);

			if (wp_tdm_api_event->wp_api_event_dtmf_type == WAN_EC_TONE_PRESENT) {
				digit = get_next_expected_digit(schan->prev_dtmf[wp_tdm_api_event->channel]);
			} else {
				digit =  schan->prev_dtmf[wp_tdm_api_event->channel];
			}

			if (digit != '?') {
				if (digit != wp_tdm_api_event->wp_api_event_dtmf_digit) {
					printf("%s: Error: DTMF Sequence Failed Expecting %c Got %c Events=%i Tone=%i\n",
					schan->name,digit,wp_tdm_api_event->wp_api_event_dtmf_digit,schan->rx_events, schan->rx_events_tone);
					
					return -1;
				}
			}

			if (wp_tdm_api_event->wp_api_event_dtmf_type == WAN_EC_TONE_PRESENT) {
				schan->prev_dtmf[wp_tdm_api_event->channel]=wp_tdm_api_event->wp_api_event_dtmf_digit;
			}
		}

		_printf("%s: DTMF Event: Span: %d Channel: %d, Digit: %c (Port: %s, Type:%s)!\n",
			schan->name,
			wp_tdm_api_event->span,
			wp_tdm_api_event->channel,
			wp_tdm_api_event->wp_api_event_dtmf_digit,
			(wp_tdm_api_event->wp_api_event_dtmf_port == WAN_EC_CHANNEL_PORT_ROUT)?"ROUT":"SOUT",
			(wp_tdm_api_event->wp_api_event_dtmf_type == WAN_EC_TONE_PRESENT)?"PRESENT":"STOP");
		break;

	case WP_API_EVENT_RXHOOK:
		printf("%s: RXHOOK Event: Channel: %d, %s! (0x%X)\n",
			schan->name,
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

	return 0;
}

/*!
  \fn int handle_tdm_event(sangoma_channel_t *schan)
  \brief Read Event buffer from the device
  \param dev_index device index number associated with device file descriptor
  \return 0 - Ok  otherwise Error

   An EVENT has occoured. Execute a system call to read the EVENT
   on a device.
*/
int handle_tdm_event(sangoma_channel_t *schan)
{
	sng_fd_t	dev_fd = schan->sangoma_wait_object.fd;
	wanpipe_api_t tdm_api;

	memset(&tdm_api,0,sizeof(tdm_api));

#if 0
	printf("sangoma_wait_objects[%d].flags_out:", dev_index);
	print_poll_event_bitmap(sangoma_wait_objects[dev_index].flags_out);
	printf("\n");
#endif

	if(sangoma_read_event(dev_fd, &tdm_api)){
		printf("%s: Error: Failed to read event %s\n",schan->name,strerror(errno));
		return 1;
	}

	return decode_api_event(schan, &tdm_api);

}


static void sangoma_get_print_stats(sangoma_channel_t *schan)
{
	wanpipe_api_t tdm_api;
	wanpipe_chan_stats_t *stats = &schan->stats;

	memset(&tdm_api,0,sizeof(tdm_api));

	sangoma_get_stats(schan->sangoma_wait_object.fd,&tdm_api,&schan->stats);

	printf(" Device %s Statistics\n",schan->name);

	printf("rx_packets              =%i\n", stats->rx_packets);
	printf("tx_packets              =%i\n", stats->tx_packets);
	printf("rx_bytes                =%i\n", stats->rx_bytes);
	printf("tx_bytes                =%i\n", stats->tx_bytes);

	printf("rx_errors               =%i\n", stats->rx_errors);
	printf("tx_errors               =%i\n", stats->tx_errors);

	printf("rx_dropped              =%i\n", stats->rx_dropped);
	printf("tx_dropped              =%i\n", stats->tx_dropped);
	printf("multicast               =%i\n", stats->multicast);
	printf("collisions              =%i\n", stats->collisions);

	printf("rx_length_errors        =%i\n", stats->rx_length_errors);
	printf("rx_over_errors          =%i\n", stats->rx_over_errors);
	printf("rx_crc_errors           =%i\n", stats->rx_crc_errors);
	printf("rx_frame_errors         =%i\n", stats->rx_frame_errors);
	printf("rx_fifo_errors          =%i\n", stats->rx_fifo_errors);
	printf("rx_missed_errors        =%i\n", stats->rx_missed_errors);

	printf("tx_aborted_errors       =%i\n", stats->tx_aborted_errors);
	printf("tx_carrier_errors       =%i\n", stats->tx_carrier_errors);
	printf("tx_fifo_errors          =%i\n", stats->tx_fifo_errors);
	printf("tx_heartbeat_errors     =%i\n", stats->tx_heartbeat_errors);
	printf("tx_window_errors        =%i\n", stats->tx_window_errors);
	printf("tx_idle_packets         =%i\n", stats->tx_idle_packets);

	printf("errors                  =%i\n", stats->errors);

	printf("frames_in_tx_queue      =%i\n", stats->current_number_of_frames_in_tx_queue);
	printf("max_tx_queue_length     =%i\n", stats->max_tx_queue_length);
	printf("frames_in_rx_queue      =%i\n", stats->current_number_of_frames_in_rx_queue);
	printf("max_rx_queue_length     =%i\n", stats->max_rx_queue_length);

	printf("events_in_event_queue   =%i\n", stats->current_number_of_events_in_event_queue);
	printf("max_event_queue_length  =%i\n", stats->max_event_queue_length);
	
	printf("rx_events               =%i\n", stats->rx_events);
	printf("rx_events_dropped       =%i\n", stats->rx_events_dropped);
	printf("rx_events_tone          =%i\n", stats->rx_events_tone);

}


/*!
  \fn static void *handle_span_chan(void *obj)
  \brief Write data buffer into a file
  \param obj schan pointer 
  \return void

  This function will wait on all opened devices.
  This example will wait for RX and EVENT signals.
  In case of POLLIN - rx data available
  In case of POLLPRI - event is available
*/
static void *handle_span_chan(void *obj)
{
	int	iResult=0;
	sangoma_channel_t *schan = (sangoma_channel_t*)obj;
	wanpipe_api_t tdm_api;
	
	memset(&tdm_api,0,sizeof(tdm_api));

	printf("\n\n%s: Span/Chan Handler: RxEnable=%s, TxEnable=%s, TxCnt=%i, TxLen=%i, Verbose=%i Stats=%i Flush=%i\n",
		schan->name,(read_enable? "Yes":"No"), (write_enable?"Yes":"No"),tx_cnt,tx_size, verbose,stats_period,flush_period);

	
	sangoma_flush_bufs(schan->sangoma_wait_object.fd,&tdm_api);
	sangoma_flush_stats(schan->sangoma_wait_object.fd,&tdm_api);

	/* init to some hight value */
	schan->elapsed_min=100000;

	/* Main Rx/Tx/Event loop */
	while(server_running)
	{	
		iResult = sangoma_socket_waitfor_many(&schan->sangoma_wait_object,
												1 /* number of wait objects */,
												1000/* 2 sec wait */);
			
		if(iResult < 0){
			/* error */
			printf("Error: iResult: %d\n", iResult);
			schan->test_failed++;
			break;
		}

		if (iResult == 0){
			/* timeout */
			printf("%s: Timeout fd=%i Events=0x%X\n",
				schan->name,schan->sangoma_wait_object.fd,schan->sangoma_wait_object.flags_in);
			sangoma_get_full_cfg(schan->sangoma_wait_object.fd, &tdm_api);
			sangoma_get_print_stats(schan);
			schan->test_failed++;
			break;
		}

		schan->loop_cnt++;

		if (schan->sangoma_wait_object.flags_out & POLLPRI){
			/* got tdm api event */
			if(handle_tdm_event(schan)){
				sangoma_get_print_stats(schan);
				schan->test_failed++;
				break;
			}
		}

		if (schan->sangoma_wait_object.flags_out & (POLLIN | POLLOUT)){
			/* got data */
			if(handle_data(schan)){
				sangoma_get_print_stats(schan);
				schan->test_failed++;
				break;
			}
		}

		if (stats_period &&
			schan->loop_cnt % stats_period == 0) {
			sangoma_get_print_stats(schan);
		}

		if (flush_period &&
			schan->loop_cnt % flush_period == 0) {
			printf("%s: Flushing Buffers\n",schan->name);
			//sangoma_flush_bufs(schan->sangoma_wait_object.fd,&tdm_api);
			tdm_api.wp_cmd.cmd = WP_API_CMD_GEN_FIFO_ERR;
			sangoma_cmd_exec(schan->sangoma_wait_object.fd,&tdm_api);
			schan->tx_sync=0;
			schan->rx_sync=0;
			//sangoma_get_print_stats(schan);
		} 

	}/* for() */

	sangoma_get_stats(schan->sangoma_wait_object.fd,&tdm_api,&schan->stats);

	/* FIXME LOCK */
	pthread_mutex_lock(&threads_running_lock);
	threads_running--;
	pthread_mutex_unlock(&threads_running_lock);

	return NULL;
}

/*!
  \fn int write_data_to_file(sangoma_channel_t *schan, unsigned char *data, unsigned int data_length)
  \brief Write data buffer into a file
  \param data data buffer
  \param data_length length of a data buffer
  \return data_length = ok  otherwise error

*/
int write_data_to_file(sangoma_channel_t *schan, unsigned char *data, unsigned int data_length)
{
	if(schan->pRxFile == NULL){
		return 1;
	}

	return fwrite(data, 1, data_length, schan->pRxFile);
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

	printf("\nProcess terminated by user request. Threads=%i\n",threads_running);

	server_running=0;

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

	printf("\nProcess terminated by user request Threads=%i.\n",threads_running);

	server_running=0;


	return;
}

#endif

/*!
  \fn int open_sangoma_device(sangoma_channel_t *schan)
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

int open_sangoma_device(sangoma_channel_t *schan)
{
	int err = -1;
	sng_fd_t	dev_fd = INVALID_HANDLE_VALUE;
	wanpipe_api_t tdm_api;

	memset(&tdm_api,0,sizeof(tdm_api));

	/* span and chan are 1-based */
	dev_fd = sangoma_open_api_span_chan(schan->span, schan->chan);
	if( dev_fd == INVALID_HANDLE_VALUE){
		printf("%s: Warning: Failed to open span %d, chan %d\n", schan->name, schan->span , schan->chan);
		return 1;
	}else{
		printf("%s: Successfuly opened span %d, chan %d fd=%i\n", schan->name, schan->span , schan->chan, dev_fd);
	}


	do{

		printf("%s: Setup Span=%i Chan=%i Handler\n",
					 schan->name, schan->span, schan->chan);

		/* Front End connect/disconnect, and other events, such as DTMF... */
		schan->poll_events_bitmap |= (POLLHUP | POLLPRI);

		if(read_enable	== 1){
			schan->poll_events_bitmap |= POLLIN;
		}
		
		if(write_enable == 1){
			schan->poll_events_bitmap |= POLLOUT;
		}

		if((err=sangoma_get_full_cfg(dev_fd, &tdm_api))){
			break;
		}

		if(usr_period){
			printf("%s: Setting user period: %d\n", schan->name,usr_period);
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
			schan->poll_events_bitmap |= POLLPRI;
			/* enable dtmf detection on Octasic chip */
			if((err=sangoma_tdm_enable_dtmf_events(dev_fd, &tdm_api))){
				break;
			}
		}

		if(dtmf_enable_remora == 1){
			schan->poll_events_bitmap |= POLLPRI;
			/* enable dtmf detection on Sangoma's Remora SLIC chip (A200 ONLY) */
			if((err=sangoma_tdm_enable_rm_dtmf_events(dev_fd, &tdm_api))){
				break;
			}
		}

		if(remora_hook == 1){
			schan->poll_events_bitmap |= POLLPRI;
			if((err=sangoma_tdm_enable_rxhook_events(dev_fd, &tdm_api))){
				break;
			}
		}

		if(rbs_events == 1){
			schan->poll_events_bitmap |= POLLPRI;
    		if((err=sangoma_tdm_enable_rbs_events(dev_fd, &tdm_api, 20))){
				break;
			}
		}

		printf("%s: Device Config RxQ=%i TxQ=%i \n",
				schan->name,
				sangoma_get_rx_queue_sz(dev_fd,&tdm_api),
				sangoma_get_rx_queue_sz(dev_fd,&tdm_api));

		err=0;
		
	}while(0);

	if(sangoma_init_wait_obj(&schan->sangoma_wait_object, dev_fd, schan->span, schan->chan, schan->poll_events_bitmap, SANGOMA_WAIT_OBJ)){
		printf("%s: Error: Failed to initialize 'sangoma_wait_object'\n", schan->name);
		return 1;
	}

	if (tx_data) {
		printf("%s: Setting Tx Data to 0x%X MaxSize=%i\n",schan->name, tx_data, sizeof(schan->Tx_data.data));
		memset(schan->Tx_data.data,tx_data,sizeof(schan->Tx_data.data));
	}	

	printf("%s: Enabling Poll Events: RX=%d TX=%d EVENT=%d\n", schan->name,
			schan->poll_events_bitmap&POLLIN,
			schan->poll_events_bitmap&POLLOUT,
			schan->poll_events_bitmap&POLLPRI);


	printf("********************************\n");
	printf("files_used: 0x%x\n", files_used);
	printf("********************************\n");
	if(files_used & RX_FILE_USED){
		char file_name[100];
		sprintf(file_name, "sangoma_s%dc%d.bin",schan->span,schan->chan);
		schan->pRxFile = fopen( (const char*)file_name, "wb" );
		if(schan->pRxFile == NULL){
			printf("%s: Can't open Rx file: [%s]!!\n", schan->name, file_name);
			return 1;
		}else{
			printf("%s: Open Rx file: %s. OK.\n", schan->name , file_name);
		}
	}

	if(files_used & TX_FILE_USED){
		schan->pTxFile = fopen( (const char*)&tx_file[0], "rb" );
		if(schan->pTxFile == NULL){
			printf("%s: Can't open Tx file: [%s]!!\n", schan->name, tx_file);
			return 1;
		}else{
			printf("%s: Open Tx file: %s. OK.\n",schan->name, tx_file);
		}
	}

	return err;
}

/*!
  \fn void close_sangoma_devices(void)
  \brief Close all opened devices 
*/
void close_sangoma_devices(sangoma_channel_t *schan)
{
	wanpipe_api_t tdm_api;
	wanpipe_chan_stats_t *stats = &schan->stats;
	int test_failed;
	memset(&tdm_api,0,sizeof(tdm_api));

	test_failed = schan->test_failed;
	if (schan->rx_events_tone == 0) {
		test_failed++;
	}

	printf("%-10s: Errors: All=%03i Rx=%03i Tx=%03i Event=%03i | RX: Event=%03i Tone=%03i | App: R_Ev=%03i R_Tone=%03i T_Ev=%03i | Diff: %i %i  | Test: %s\n",schan->name,
			stats->errors,
			stats->rx_errors,
			stats->tx_errors,
			stats->rx_events_dropped,
			stats->rx_events,
			stats->rx_events_tone,
			schan->rx_events,
			schan->rx_events_tone,
			schan->tx_file_cnt,
			abs(stats->rx_events-schan->rx_events),
			abs(stats->rx_events_tone-schan->rx_events_tone),
			test_failed?"FAILED":"Passed"
			);

	if(dtmf_enable_octasic == 1){
		/* Disable dtmf detection on Octasic chip */
		sangoma_tdm_disable_dtmf_events(schan->sangoma_wait_object.fd, &tdm_api);
	}

	if(dtmf_enable_remora == 1){
		/* Disable dtmf detection on Sangoma's Remora SLIC chip */
		sangoma_tdm_disable_rm_dtmf_events(schan->sangoma_wait_object.fd, &tdm_api);
	}

	if(remora_hook == 1){
		sangoma_tdm_disable_rxhook_events(schan->sangoma_wait_object.fd, &tdm_api);
	}

	if(rbs_events == 1){
    	sangoma_tdm_disable_rbs_events(schan->sangoma_wait_object.fd, &tdm_api);
	}

	/* call sangoma_close() for EACH open Device Handle */
	sangoma_close(&schan->sangoma_wait_object.fd);

	return;
}

#ifdef __LINUX__
static int launch_handler_thread(sangoma_channel_t *schan)
{
	pthread_attr_t attr;
	int result = -1;

	result = pthread_attr_init(&attr);
	//pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	//pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, MAX_STACK_SIZE);

	result = pthread_create(&schan->threadid, &attr, handle_span_chan, schan);
	if (result) {
		printf("%s: %s(): Error: Creating Thread! %s\n",
				 schan->name,__FUNCTION__,strerror(errno));
   	} else {
		//FIXME LOCK
		pthread_mutex_lock(&threads_running_lock);
		threads_running++;
		pthread_mutex_unlock(&threads_running_lock);
	}
	pthread_attr_destroy(&attr);

   	return result;
}
#endif




static char api_usage[]="\n"
"\n"
"<options>:\n"
"	-p  <max span numbers>	Default: 1\n"
"	-pm <min span numbers>	Default: 1\n"
"	-i  <max span numbers>	Default: 1\n"
"	-r					#read enable\n"
"	-w					#write eable\n"
"   -dtmf_seq_check     #check dtmf seq\n"
"\n"
"	example 1: sangoma_c -p 1 -i 1 -r\n"
"	  in this example Wanpipe 1, Interface 1 will be used for reading data\n"
"	example 1: sangoma_c -p 5 -i 32 -r\n"
"	  in this example Wanpipes 1-5, Interfaces 1-32 will be used for reading data\n"
"\n"
"<extra options>\n"
"	-txcnt   <digit>	#number of tx packets  (Dflt: 1)\n"
"	-txsize  <digit>	#tx packet size        (Dflt: 10)\n"
"	-txdelay <digit>	#delay in sec after each tx packet (Dflt: 0)\n"
"	-txdata  <digit>	#data to tx <1-255>\n"
"	-rx2tx	 			#transmit all received data\n"
"\n"
"	-dtmf_octasic		#Enable DTMF detection on Octasic chip\n"
"	-dtmf_remora		#Enable DTMF detection on A200 (SLIC) chip\n"
"	-remora_hook		#Enable On/Off hook events on A200\n"
"	-set_codec_slinear	#Enable SLINEAR codec\n"
"	-set_codec_none		#Disable codec\n"
"	-rbs_events			#Enable RBS change detection\n"
"\n"
"	-rxcnt   <digit>	#number of rx packets before exit\n"
"						#this number overwrites the txcnt\n"
"						#Thus, app will only exit after it\n"
"						#receives the rxcnt number of packets.\n"
"	\n"
"	-verbose			#Enable verbose mode\n"
"\n";




/*!
  \fn int __cdecl main(int argc, char* argv[])
  \brief Main function that starts the sample code
  \param argc  number of arguments
  \param argv  argument list
*/
int __cdecl main(int argc, char* argv[])
{
	int proceed;
	int span,chan,err;
	int min_span;

	proceed=init_args(argc,argv);
	if (proceed != WAN_TRUE){
		printf("%s",api_usage);
		return -1;
	}

	min_span=wanpipe_min_port_no;
	if (min_span==0){
     	min_span=1;
	}
	max_spans = wanpipe_port_no;
	max_chans = wanpipe_if_no;

#if defined(__LINUX__)
	nice(-10);
#endif

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
	signal(SIGINT,TerminateHandler);
#endif

	server_running = 1;
	pthread_mutex_init(&threads_running_lock,NULL);


	printf("Connecting to Port/Span: %d, Interface/Chan: %d\n",
		wanpipe_port_no, wanpipe_if_no);

	for (span=min_span;span<=max_spans;span++) {
		for (chan=1;chan<=max_chans;chan++) {
			sangoma_channel_t *schan = &sangoma_channel_idx[span][chan];
			schan->span=span;
			schan->chan=chan;
			sprintf(schan->name, "s%dc%d",span,chan);
			err=open_sangoma_device(schan);
			if (err == 0) {
				schan->init=1;
				err=launch_handler_thread(schan);
				if (err) {
					printf("Critical Error Failed to Launch Thread!\n");
					server_running=0;
					goto server_shutdown;
				}
			} else {
				close_sangoma_devices(schan);
			}
		}
	}

	while (server_running) {
		usleep(5000);
	}



server_shutdown:

	printf("Waiting for theads to stop !\n");
	while (threads_running) {
		usleep(5000);
	}

	printf("All threads stopped, shutting down !\n");

	for (span=1;span<=max_spans;span++) {
		for (chan=1;chan<=max_chans;chan++) {
			sangoma_channel_t *schan = &sangoma_channel_idx[span][chan];
			if (schan->init) {
				close_sangoma_devices(schan);
			}
		}
	}
	
	pthread_mutex_destroy(&threads_running_lock);

	return 0;
}
