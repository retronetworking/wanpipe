/*****************************************************************************
 * libsangoma.c	AFT T1/E1: HDLC API Code Library
 *
 * Author(s):	Anthony Minessale II <anthmct@yahoo.com>
 *              Nenad Corbic <ncorbic@sangoma.com>
 *              David Rokhvarg <davidr@sangoma.com>
 *              Michael Jerris <mike@jerris.com>
 *
 * Copyright:	(c) 2005 Anthony Minessale II
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 *
 * Aug 15, 2006  David Rokhvarg <davidr@sangoma.com>	Ported to MS Windows 2000/XP
 * Sep 24, 2006  Michael Jerris <mike@jerris.com>		Windows port, standardize api, cleanup
 * 
 */

#include "libsangoma.h"
#define DFT_CARD "wanpipe1"

#ifndef WP_TDM_FEATURE_FE_ALARM
#warning "Warning: TDM FE ALARM not supported by driver"
#endif

#ifndef WP_TDM_FEATURE_DTMF_EVENTS
#warning "Warning: TDM DTMF not supported by driver"
#endif

#ifndef WP_TDM_FEATURE_EVENTS
#warning "Warning: TDM EVENTS not supported by driver"
#endif

#ifndef WP_TDM_FEATURE_LINK_STATUS
#warning "Warning: TDM LINK STATUS not supported by driver"
#endif


#if defined(WIN32)
//extern int	verbose;

#define DEV_NAME_LEN	100
char device_name[DEV_NAME_LEN];

/* IOCTL management structures and variables*/
wan_udp_hdr_t	wan_udp;

#include "win_api_common.h"

static wan_cmd_api_t api_cmd;
static api_tx_hdr_t *tx_hdr = (api_tx_hdr_t *)api_cmd.data;

/* keeps the LAST (and single) event received */
static wp_tdm_api_rx_hdr_t last_tdm_api_event_buffer;

#endif	/* WIN32 */

void sangoma_socket_close(sng_fd_t *sp) 
{
#if defined(WIN32)
	if(	*sp != INVALID_HANDLE_VALUE){
		CloseHandle(*sp);
		*sp = INVALID_HANDLE_VALUE;
	}
#else
    if (*sp > -1) {
	close(*sp);
	*sp = -1;
    }
#endif
}

int sangoma_socket_waitfor(sng_fd_t fd, int timeout, int flags)
{
#if defined(WIN32)
	API_POLL_STRUCT	api_poll;

	memset(&api_poll, 0x00, sizeof(API_POLL_STRUCT));
	
	api_poll.user_flags_bitmap = flags;

	if(DoApiPollCommand(fd, &api_poll)){
		//failed
		return 0;
	}

	switch(api_poll.operation_status)
	{
		case SANG_STATUS_RX_DATA_AVAILABLE:
			break;

		default:
			prn(1, "Error: sangoma_socket_waitfor(): Unknown Operation Status: %d\n", 
				api_poll.operation_status);
			return 0;
	}//switch()

	if(api_poll.poll_events_bitmap == 0){
		prn(1, "Error: invalid Poll Events bitmap: 0x%X\n",
			api_poll.poll_events_bitmap);
	}
	return api_poll.poll_events_bitmap;
#else
    struct pollfd pfds[1];
    int res;

    memset(&pfds[0], 0, sizeof(pfds[0]));
    pfds[0].fd = fd;
    pfds[0].events = flags;
    res = poll(pfds, 1, timeout);
    if (res > 0) {
	if ((pfds[0].revents & POLLERR)) {
		res = -1;
	} else if((pfds[0].revents)) {
		res = 1;
	}
    }

    return res;
#endif
}


int sangoma_span_chan_toif(int span, int chan, char *interface_name)
{
 	sprintf(interface_name,"s%ic%i",span,chan);
	return 0;
}

int sangoma_interface_toi(char *interface_name, int *span, int *chan)
{
	char *p=NULL, *sp = NULL, *ch = NULL;
	int ret = 0;
	char data[FNAME_LEN];

	strncpy(data, interface_name, FNAME_LEN);
	if ((data[0])) {
		for (p = data; *p; p++) {
			if (sp && *p == 'g') {
				*p = '\0';
				ch = (p + 1);
				break;
			} else if (*p == 'w') {
				sp = (p + 1);
			}
		}

		if(ch && sp) {
			*span = atoi(sp);
			*chan = atoi(ch);
			ret = 1;
		} else {
			*span = -1;
			*chan = -1;
		}
	}

	return ret;
}

int sangoma_span_chan_fromif(char *interface_name, int *span, int *chan)
{
	char *p = NULL, *sp = NULL, *ch = NULL;
	int ret = 0;
	char data[FNAME_LEN];

	strncpy(data, interface_name, FNAME_LEN);
	if ((data[0])) {
		for (p = data; *p; p++) {
			if (sp && *p == 'c') {
				*p = '\0';
				ch = (p + 1);
				break;
			} else if (*p == 's') {
				sp = (p + 1);
			}
		}

		if(ch && sp) {
			*span = atoi(sp);
			*chan = atoi(ch);
			ret = 1;
		} else {
			*span = -1;
			*chan = -1;
		}
	}

	return ret;
}

sng_fd_t sangoma_open_tdmapi_span_chan(int span, int chan) 
{
   	char fname[FNAME_LEN];
#if defined(WIN32)

	//NOTE: under Windows Interfaces are zero based but 'chan' is 1 based.
	//		Subtract 1 from 'chan'.
	_snprintf(fname , FNAME_LEN, "\\\\.\\WANPIPE%d_IF%d", span, chan - 1);

	//prn(verbose, "Opening device: %s...\n", fname);

	return CreateFile(	fname, 
						GENERIC_READ | GENERIC_WRITE, 
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						(LPSECURITY_ATTRIBUTES)NULL, 
						OPEN_EXISTING,
						FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
						(HANDLE)NULL
						);
#else
  	int fd=-1;

	sprintf(fname,"/dev/wptdm_s%dc%d",span,chan);

	fd = open(fname, O_RDWR);

	return fd;  
#endif
}            


sng_fd_t sangoma_open_tdmapi_ctrl(void)
{
	int fd=-1;

#if defined(WIN32)
#warning "sangoma_open_tdmapi_ctrl: Not support on Windows"
#else
        fd = open("/dev/wptdm_ctrl", O_RDWR);
#endif

        return fd;
}

sng_fd_t sangoma_create_socket_by_name(char *device, char *card) 
{
	int span,chan;
	sangoma_interface_toi(device,&span,&chan);
	
	return sangoma_open_tdmapi_span_chan(span,chan);
}

          
sng_fd_t sangoma_open_tdmapi_span(int span) 
{
    int i=0;
#if defined(WIN32)
	sng_fd_t fd = INVALID_HANDLE_VALUE;

	for(i = 1; i < 32; i++){
		if((fd = sangoma_open_tdmapi_span_chan(span, i)) == INVALID_HANDLE_VALUE){
			//prn(verbose, "Span: %d, chan: %d: is not running, consider 'busy'\n",
			//	span, i);
			continue;
		}

		//get the open handle counter
		wan_udp.wan_udphdr_command = GET_OPEN_HANDLES_COUNTER; 
		wan_udp.wan_udphdr_data_len = 0;

		DoManagementCommand(fd, &wan_udp);
		if(wan_udp.wan_udphdr_return_code){
			prn(1, "Error: command GET_OPEN_HANDLES_COUNTER failed! Span: %d, chan: %d\n",
				span, i);
			//don't forget to close!! otherwize counter will stay incremented.
			sangoma_socket_close(&fd);
			continue;
		}

		//prn(verbose, "open handles counter: %d\n", *(int*)&wan_udp.wan_udphdr_data[0]);
		if(*(int*)&wan_udp.wan_udphdr_data[0] == 1){
			//this is the only process using this chan/span, so it is 'free'
			//prn(verbose, "Found 'free' Span: %d, chan: %d\n",span, i);
			break;
		}
		//don't forget to close!! otherwize counter will stay incremented.
		sangoma_socket_close(&fd);
	}//for()

#else
        char fname[FNAME_LEN];
	int fd=0;
	for (i=1;i<32;i++){
		sprintf(fname,"/dev/wptdm_s%dc%d",span,i);
		fd = open(fname, O_RDWR);
		if (fd < 0){
         		continue;
		}
		break;
	}
#endif	
    return fd;  
}      

int sangoma_readmsg_tdm(sng_fd_t fd, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag)
{
	int rx_len=0;

#if defined(WIN32)
	static RX_DATA_STRUCT	rx_data;
	api_header_t			*pri;
	wp_tdm_api_rx_hdr_t		*tdm_api_rx_hdr;
	wp_tdm_api_rx_hdr_t		*user_buf = (wp_tdm_api_rx_hdr_t*)hdrbuf;

	if(hdrlen != sizeof(wp_tdm_api_rx_hdr_t)){
		//error
		prn(1, "Error: sangoma_readmsg_tdm(): invalid size of user's 'header buffer'.\
Should be 'sizeof(wp_tdm_api_rx_hdr_t)'.\n");
		return -1;
	}

	if(DoReadCommand(fd, &rx_data) ){
		//error
		prn(1, "Error: DoReadCommand() failed! Check messages log.\n");
		return -1;
	}

	//use our special buffer at rxdata to hold received data
	pri = &rx_data.api_header;
	tdm_api_rx_hdr = (wp_tdm_api_rx_hdr_t*)rx_data.data;

	user_buf->wp_tdm_api_event_type = pri->operation_status;

	switch(pri->operation_status)
	{
	case SANG_STATUS_RX_DATA_AVAILABLE:
		//prn(verbose, "SANG_STATUS_RX_DATA_AVAILABLE\n");

		if(pri->data_length > datalen){
			rx_len=0;
			break;
		}
		memcpy(databuf, rx_data.data, pri->data_length);
		rx_len = pri->data_length;
		break;

	case SANG_STATUS_TDM_EVENT_AVAILABLE:
		//prn(verbose, "SANG_STATUS_TDM_EVENT_AVAILABLE\n");

		//make event is accessable for the caller directly:
		memcpy(databuf, rx_data.data, pri->data_length);
		rx_len = pri->data_length;

		//make copy for use with sangoma_tdm_read_event() - indirect access.
		memcpy(	&last_tdm_api_event_buffer,	tdm_api_rx_hdr, sizeof(wp_tdm_api_rx_hdr_t));
		break;

	default:
		switch(pri->operation_status)
		{
		case SANG_STATUS_RX_DATA_TIMEOUT:
			//no data in READ_CMD_TIMEOUT, try again.
			prn(1, "Error: Timeout on read.\n");
			break;

		case SANG_STATUS_BUFFER_TOO_SMALL:
			//Recieved data longer than the pre-configured maximum.
			//Maximum length is set in 'Interface Properties',
			//in the 'Device Manager'.
			prn(1, "Error: Received data longer than buffer passed to API.\n");
			break;

		case SANG_STATUS_LINE_DISCONNECTED:
			//Front end monitoring is enabled and Line is
			//in disconnected state.
			//Check the T1/E1 line is in "Connected" state,
			//alse check the Alarms and the message log.
			prn(1, "Error: Line disconnected.\n");
			break;

		default:
			prn(1, "Rx:Unknown Operation Status: %d\n", pri->operation_status);
			break;
		}//switch()
		return 0;
	}//switch()

#else
	struct msghdr msg;
	struct iovec iov[2];

	memset(&msg,0,sizeof(struct msghdr));

	iov[0].iov_len=hdrlen;
	iov[0].iov_base=hdrbuf;

	iov[1].iov_len=datalen;
	iov[1].iov_base=databuf;

	msg.msg_iovlen=2;
	msg.msg_iov=iov;

	rx_len = read(fd,&msg,datalen+hdrlen);

	if (rx_len <= sizeof(wp_tdm_api_rx_hdr_t)){
		return -EINVAL;
	}

	rx_len-=sizeof(wp_tdm_api_rx_hdr_t);
#endif
    return rx_len;
}                    

int sangoma_writemsg_tdm(sng_fd_t fd, void *hdrbuf, int hdrlen, void *databuf, unsigned short datalen, int flag)
{
	int bsent;

#if defined(WIN32)
	static TX_DATA_STRUCT	local_tx_data;
	api_header_t			*pri;

	pri = &local_tx_data.api_header;

	pri->data_length = datalen;
	memcpy(local_tx_data.data, databuf, pri->data_length);

	//queue data for transmission
	if(	DoWriteCommand(fd, &local_tx_data)){
		//error
		prn(1, "Error: DoWriteCommand() failed!! Check messages log.\n");
		return -1;
	}

	bsent=0;
	//check that frame was transmitted
	switch(local_tx_data.api_header.operation_status)
	{
	case SANG_STATUS_SUCCESS:
		bsent = datalen;
		break;
				
	case SANG_STATUS_TX_TIMEOUT:
		//error
		prn(1, "****** Error: SANG_STATUS_TX_TIMEOUT ******\n");
		//Check messages log or look at statistics.
		break;
				
	case SANG_STATUS_TX_DATA_TOO_LONG:
		//Attempt to transmit data longer than the pre-configured maximum.
		//Maximum length is set in 'Interface Properties',
		//in the 'Device Manager'.
		prn(1, "****** SANG_STATUS_TX_DATA_TOO_LONG ******\n");
		break;
				
	case SANG_STATUS_TX_DATA_TOO_SHORT:
		//Minimum is 1 byte  for Primary   port,
		//			 2 bytes for Secondary port
		prn(1, "****** SANG_STATUS_TX_DATA_TOO_SHORT ******\n");
		break;

	case SANG_STATUS_LINE_DISCONNECTED:
		//Front end monitoring is enabled and Line is
		//in disconnected state.
		//Check the T1/E1 line is in "Connected" state,
		//alse check the Alarms and the message log.
		prn(1, "****** SANG_STATUS_LINE_DISCONNECTED ******\n");
		break;

	default:
		prn(1, "Unknown return code (0x%X) on transmission!\n",
			local_tx_data.api_header.operation_status);
		break;
	}//switch()
#else
	struct msghdr msg;
	struct iovec iov[2];

	memset(&msg,0,sizeof(struct msghdr));

	iov[0].iov_len=hdrlen;
	iov[0].iov_base=hdrbuf;

	iov[1].iov_len=datalen;
	iov[1].iov_base=databuf;

	msg.msg_iovlen=2;
	msg.msg_iov=iov;

	bsent = write(fd,&msg,datalen+hdrlen);
	if (bsent > 0){
		bsent-=sizeof(wp_tdm_api_tx_hdr_t);
	}
#endif
	return bsent;
}


#ifdef WANPIPE_TDM_API

/*========================================================
 * Execute TDM command
 *
 */
static int sangoma_tdm_cmd_exec(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

#if defined(WIN32)
	err = tdmv_api_ioctl(fd, &tdm_api->wp_tdm_cmd);
#else
	err = ioctl(fd,SIOC_WANPIPE_TDM_API,&tdm_api->wp_tdm_cmd);
	if (err < 0){
		char tmp[50];
		sprintf(tmp,"TDM API: CMD: %i\n",tdm_api->wp_tdm_cmd.cmd);
		perror(tmp);
		return -1;
	}
#endif
	return err;
}

/*========================================================
 * Get Full TDM API configuration per channel
 *
 */
int sangoma_get_full_cfg(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_FULL_CFG;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

#if 0
	printf("TDM API CFG:\n");
	printf("\thw_tdm_coding:\t%d\n",tdm_api->wp_tdm_cmd.hw_tdm_coding);
	printf("\tusr_mtu_mru:\t%d\n",tdm_api->wp_tdm_cmd.hw_mtu_mru);
	printf("\tusr_period:\t%d\n",tdm_api->wp_tdm_cmd.usr_period);
	printf("\ttdm_codec:\t%d\n",tdm_api->wp_tdm_cmd.tdm_codec);
	printf("\tpower_level:\t%d\n",tdm_api->wp_tdm_cmd.power_level);
	printf("\trx_disable:\t%d\n",tdm_api->wp_tdm_cmd.rx_disable);
	printf("\ttx_disable:\t%d\n",tdm_api->wp_tdm_cmd.tx_disable);
	printf("\tusr_mtu_mru:\t%d\n",tdm_api->wp_tdm_cmd.usr_mtu_mru);
	printf("\tidle flag:\t0x%02X\n",tdm_api->wp_tdm_cmd.idle_flag);

#ifdef WP_TDM_FEATURE_FE_ALARM
	printf("\tfe alarms:\t0x%02X\n",tdm_api->wp_tdm_cmd.fe_alarms);
#endif
	
	printf("\trx pkt\t%d\ttx pkt\t%d\n",tdm_api->wp_tdm_cmd.stats.rx_packets,
				tdm_api->wp_tdm_cmd.stats.tx_packets);
	printf("\trx err\t%d\ttx err\t%d\n",
				tdm_api->wp_tdm_cmd.stats.rx_errors,
				tdm_api->wp_tdm_cmd.stats.tx_errors);
#ifndef __WINDOWS__
	printf("\trx ovr\t%d\ttx idl\t%d\n",
				tdm_api->wp_tdm_cmd.stats.rx_fifo_errors,
				tdm_api->wp_tdm_cmd.stats.tx_carrier_errors);
#endif		
#endif		
	
	return 0;
}

/*========================================================
 * SET Codec on a particular Channel.
 * 
 * Available codecs are defined in 
 * /usr/src/linux/include/linux/wanpipe_cfg.h
 * 
 * enum wan_codec_format {
 *  	WP_NONE,
 *	WP_SLINEAR
 * }
 *
 */
int sangoma_tdm_set_codec(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api, int codec)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_CODEC;
	tdm_api->wp_tdm_cmd.tdm_codec = codec;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);

	return err;
}

/*========================================================
 * GET Codec from a particular Channel.
 * 
 * Available codecs are defined in 
 * /usr/src/linux/include/linux/wanpipe_cfg.h
 * 
 * enum wan_codec_format {
 *  	WP_NONE,
 *	WP_SLINEAR
 * }
 *
 */
int sangoma_tdm_get_codec(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_CODEC;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.tdm_codec;	
}

/*========================================================
 * SET Rx/Tx Hardware Period in milliseconds.
 * 
 * Available options are:
 *  10,20,30,40,50 ms      
 *
 */
int sangoma_tdm_set_usr_period(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api, int period)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_USR_PERIOD;
	tdm_api->wp_tdm_cmd.usr_period = period;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);

	return err;
}

/*========================================================
 * GET Rx/Tx Hardware Period in milliseconds.
 * 
 * Available options are:
 *  10,20,30,40,50 ms      
 *
 */
int sangoma_tdm_get_usr_period(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_USR_PERIOD;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.usr_period;
}

/*========================================================
 * GET Current User Hardware Coding Format
 *
 * Coding Format will be ULAW/ALAW based on T1/E1 
 */

int sangoma_tdm_get_hw_coding(int fd, wanpipe_tdm_api_t *tdm_api)
{
        int err;
        tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_HW_CODING;
        err=sangoma_tdm_cmd_exec(fd,tdm_api);
        if (err){
                return err;
        }
        return tdm_api->wp_tdm_cmd.hw_tdm_coding;
}

#ifdef WP_TDM_FEATURE_DTMF_EVENTS
/*========================================================
 * GET Current User Hardware DTMF Enabled/Disabled
 *
 * Will return true if HW DTMF is enabled on Octasic
 */

int sangoma_tdm_get_hw_dtmf(int fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_HW_DTMF;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}
	return tdm_api->wp_tdm_cmd.hw_dtmf;
}
#endif

/*========================================================
 * GET Current User MTU/MRU values in bytes.
 * 
 * The USER MTU/MRU values will change each time a PERIOD
 * or CODEC is adjusted.
 */
int sangoma_tdm_get_usr_mtu_mru(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_USR_MTU_MRU;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.usr_mtu_mru;
}

/*========================================================
 * SET TDM Power Level
 * 
 * This option is not implemented yet
 *
 */
int sangoma_tdm_set_power_level(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api, int power)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_POWER_LEVEL;
	tdm_api->wp_tdm_cmd.power_level = power;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);

	return err;
}

/*========================================================
 * GET TDM Power Level
 * 
 * This option is not implemented yet
 *
 */
int sangoma_tdm_get_power_level(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_POWER_LEVEL;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.power_level;
}

int sangoma_tdm_flush_bufs(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
#if 0
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_FLUSH_BUFFERS;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}
#endif
	return 0;
}

int sangoma_tdm_enable_rbs_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api, int poll_in_sec) {
	
	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_ENABLE_RBS_EVENTS;
	tdm_api->wp_tdm_cmd.rbs_poll=poll_in_sec; 
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.rbs_poll;
}


int sangoma_tdm_disable_rbs_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {

	int err;
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_DISABLE_RBS_EVENTS;
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_write_rbs(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api, unsigned char rbs) 
{
	
	int err;
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_WRITE_RBS_BITS;
	tdm_api->wp_tdm_cmd.rbs_tx_bits=rbs; 
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}        

int sangoma_tdm_read_event(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) 
{

#ifdef WP_TDM_FEATURE_EVENTS

	wp_tdm_api_event_t *rx_event;

#if defined(WIN32)	
	rx_event = &last_tdm_api_event_buffer;
#else
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_READ_EVENT;
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	rx_event = &tdm_api->wp_tdm_cmd.event;
#endif
	

	switch (rx_event->wp_tdm_api_event_type){
	
	case WP_TDMAPI_EVENT_RBS:
		if (tdm_api->wp_tdm_event.wp_rbs_event) {
			tdm_api->wp_tdm_event.wp_rbs_event(fd,rx_event->wp_tdm_api_event_rbs_bits);
		}
		
		break;
	
#ifdef WP_TDM_FEATURE_DTMF_EVENTS	
	case WP_TDMAPI_EVENT_DTMF:
		if (tdm_api->wp_tdm_event.wp_dtmf_event) {
			tdm_api->wp_tdm_event.wp_dtmf_event(fd,
						rx_event->wp_tdm_api_event_dtmf_digit,
						rx_event->wp_tdm_api_event_dtmf_type,
						rx_event->wp_tdm_api_event_dtmf_port);
		}
		break;
#endif
		
	case WP_TDMAPI_EVENT_RXHOOK:
		if (tdm_api->wp_tdm_event.wp_rxhook_event) {
			tdm_api->wp_tdm_event.wp_rxhook_event(fd,
						rx_event->wp_tdm_api_event_hook_state);
		}
		break;

	case WP_TDMAPI_EVENT_RING_DETECT:
		if (tdm_api->wp_tdm_event.wp_ring_detect_event) {
			tdm_api->wp_tdm_event.wp_ring_detect_event(fd,
						rx_event->wp_tdm_api_event_ring_state);
		}
		break;

	case WP_TDMAPI_EVENT_RING_TRIP_DETECT:
		if (tdm_api->wp_tdm_event.wp_ring_trip_detect_event) {
			tdm_api->wp_tdm_event.wp_ring_trip_detect_event(fd,
						rx_event->wp_tdm_api_event_ring_state);
		}
		break;

#ifdef WP_TDM_FEATURE_FE_ALARM
	case WP_TDMAPI_EVENT_ALARM:
		if (tdm_api->wp_tdm_event.wp_fe_alarm_event) {
			tdm_api->wp_tdm_event.wp_fe_alarm_event(fd,
                              rx_event->wp_tdm_api_event_alarm);
		}   
		break; 
#endif

#ifdef WP_TDM_FEATURE_LINK_STATUS	
	/* Link Status */	
	case WP_TDMAPI_EVENT_LINK_STATUS:
		if(tdm_api->wp_tdm_event.wp_link_status_event){
			tdm_api->wp_tdm_event.wp_link_status_event(fd,
						rx_event->wp_tdm_api_event_link_status);
		}
		
		break;
#endif	
	default:
		printf("%d: Unknown TDM event!", (int)fd);
		break;
	}
	
	return 0;
#else
	printf("Error: Read Event not supported!\n");
	return -1;
#endif
}        

#ifdef WP_TDM_FEATURE_DTMF_EVENTS
int sangoma_tdm_enable_dtmf_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_DTMF;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_disable_dtmf_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) 
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_DTMF;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_DISABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_enable_rm_dtmf_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) 
{
	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RM_DTMF;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_disable_rm_dtmf_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) 
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RM_DTMF;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_DISABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_enable_rxhook_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) 
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RXHOOK;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_disable_rxhook_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) 
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RXHOOK;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_DISABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_enable_ring_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {
	
	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RING;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.rbs_poll;
}


int sangoma_tdm_disable_ring_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RING;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_DISABLE;
	
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_enable_ring_detect_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {
	
	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RING_DETECT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.rbs_poll;
}


int sangoma_tdm_disable_ring_detect_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RING_DETECT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_DISABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_enable_ring_trip_detect_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {
	
	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RING_TRIP_DETECT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.rbs_poll;
}


int sangoma_tdm_disable_ring_trip_detect_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_RING_DETECT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_DISABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_txsig_kewl(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_TXSIG_KEWL;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_txsig_start(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_TXSIG_START;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_txsig_onhook(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_TXSIG_ONHOOK;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}

int sangoma_tdm_txsig_offhook(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {

	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_TXSIG_OFFHOOK;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return 0;
}


int sangoma_tdm_enable_tone_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api, int tone_id) {
	
	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_TONE;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_ENABLE;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_tone_type = tone_id;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.rbs_poll;
}

int sangoma_tdm_disable_tone_events(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api) {
	
	int err;
	
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_EVENT;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_type = WP_TDMAPI_EVENT_TONE;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_mode = WP_TDMAPI_EVENT_DISABLE;
	tdm_api->wp_tdm_cmd.event.wp_tdm_api_event_tone_type = 0x00;
	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.rbs_poll;
}

#endif

int sangoma_tdm_enable_hwec(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
        int err;

        tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_ENABLE_HWEC;
        err=sangoma_tdm_cmd_exec(fd,tdm_api);
        if (err){
                return err;
        }

        return 0;
}

int sangoma_tdm_disable_hwec(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
        int err;

        tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_DISABLE_HWEC;
        err=sangoma_tdm_cmd_exec(fd,tdm_api);
        if (err){
                return err;
        }

        return 0;
}


/*========================================================
 * GET Front End Alarms
 * 
 */                  
#ifdef WP_TDM_FEATURE_FE_ALARM
int sangoma_tdm_get_fe_alarms(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_FE_ALARMS;

	err=sangoma_tdm_cmd_exec(fd,tdm_api);
	if (err){
		return err;
	}

	return tdm_api->wp_tdm_cmd.fe_alarms;
}         

/* get current Line Connection state - Connected/Disconnected */
int sangoma_tdm_get_fe_status(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api, unsigned char *current_status)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_FE_STATUS;
	err = sangoma_tdm_cmd_exec(fd, tdm_api);
	*current_status = tdm_api->wp_tdm_cmd.fe_status;

	return err;
}
#endif

/* get current Line Connection state - Connected/Disconnected */
#ifdef WP_TDM_FEATURE_LINK_STATUS
int sangoma_tdm_get_link_status(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api, unsigned char *current_status)
{
	int err;

	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_GET_LINK_STATUS;
	err = sangoma_tdm_cmd_exec(fd, tdm_api);
	*current_status = tdm_api->wp_tdm_cmd.fe_status;

	return err;
}

/* set current Line Connection state - Connected/Disconnected. valid only for ISDN BRI */
int sangoma_tdm_set_fe_status(sng_fd_t fd, wanpipe_tdm_api_t *tdm_api, unsigned char new_status)
{
	tdm_api->wp_tdm_cmd.cmd = SIOC_WP_TDM_SET_FE_STATUS;
	tdm_api->wp_tdm_cmd.fe_status = new_status;

	return sangoma_tdm_cmd_exec(fd, tdm_api);
}
#endif

#endif /* WANPIPE_TDM_API */
