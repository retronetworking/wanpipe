/*****************************************************************************
 * libhpsangoma_priv.c:  Sangoma High Performance TDM API - Span Based Library
 *
 * Author(s):	Nenad Corbic <ncorbic@sangoma.com>
 *
 * Copyright:	(c) 2008 Nenad Corbic <ncorbic@sangoma.com>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 *
 */

#include "libhpsangoma.h"
#include "libhpsangoma_priv.h"
#define DFT_CARD "wanpipe1"


/*!
  \brief Span read data from a hw interface
  \param fd socket file descriptor to span hw interface
  \param hdrbuf pointer to header buffer used for misc oob data - not used
  \param hdrlen size of hdr buffer - always standard 16 bytes
  \param databuf pointer to span data that includes all timeslots * chunk size
  \param datalen  size of span data: timeslots * chunk size
  \param flag  selects normal (0) or oob (MSG_OOB) read queue
  \return 0 = pass, non zero fail
*/
int sangoma_readmsg_hp_tdm(int fd, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag)
{
	int rx_len=0;

#if defined(WIN32)

#error "WINDOWS HP TDMAPI NOT DEFINED"

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

	rx_len = recvmsg(fd,&msg,0);

	if (rx_len <= sizeof(api_rx_hdr_t)){
		return -EINVAL;
	}

	rx_len-=sizeof(api_rx_hdr_t);
#endif

   	return rx_len;
}



/*!
  \brief Span write data into a hw interface
  \param fd socket file descriptor to span hw interface
  \param hdrbuf pointer to header buffer used for misc oob data - not used
  \param hdrlen size of hdr buffer - always standard 16 bytes
  \param databuf pointer to span data that includes all timeslots * chunk size
  \param datalen  size of span data: timeslots * chunk size
  \param flag  selects normal (0) or oob (MSG_OOB) read queue
  \return 0 = pass, non zero fail
*/
int sangoma_writemsg_hp_tdm(int fd, void *hdrbuf, int hdrlen, void *databuf, unsigned short datalen, int flag)
{
	int bsent;

#if defined(WIN32)

#error "Windows TX not implemented"

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

	bsent = sendmsg(fd,&msg,0);
	if (bsent > 0){
		bsent-=sizeof(api_tx_hdr_t);
	}
#endif
	return bsent;
}

/*!
  \brief Execute a management ioctl command
  \param span span object
  \return 0 = pass, non zero fail
 */
int do_wanpipemon_cmd(sangoma_hptdm_span_t *span)
{
	struct ifreq ifr;
	memset(&ifr,0,sizeof(ifr));
	ifr.ifr_data = (void*)&span->wan_udp;
	sprintf (ifr.ifr_name, "%s", span->if_name);
	return ioctl(span->sock,SIOC_WANPIPE_PIPEMON,&ifr);
}

/*!
  \brief Get full span hw configuration
  \param span span object
  \return 0 = pass, non zero fail
 */
unsigned char sangoma_get_cfg(sangoma_hptdm_span_t *span)
{
	span->wan_udp.wan_udphdr_command = READ_CONFIGURATION;
	span->wan_udp.wan_udphdr_data_len = 0;
	span->wan_udp.wan_udphdr_return_code = 0xaa;
	do_wanpipemon_cmd(span);
	if (span->wan_udp.wan_udphdr_return_code != 0){
		lib_printf(0,NULL,"Error: SPAN %i management command failed 0x02Xs (%s)",
			span->span_no+1, span->wan_udp.wan_udphdr_return_code,strerror(errno));
		return -1;
	}

	memcpy(&span->span_cfg,&span->wan_udp.wan_udphdr_data[0],sizeof(span->span_cfg));

	return 0;
}


/*!
  \brief Open socket to span hw interface
  \param span span object
  \return 0 = pass, non zero fail
*/

int sangoma_hptdm_span_open(sangoma_hptdm_span_t *span)
{
	struct wan_sockaddr_ll 	sa;
	char if_name[100];
	int sock=-1;
	int err;

	memset(&sa,0,sizeof(struct wan_sockaddr_ll));
   	sock = socket(AF_WANPIPE, SOCK_RAW, 0);
   	if( sock < 0 ) {
      		perror("Socket");
      		return -1;
   	} /* if */

	sprintf(if_name, "w%ig1", span->span_no+1);

	strcpy( (char*)sa.sll_device, if_name);
	strcpy( (char*)sa.sll_card, DFT_CARD);
	sa.sll_protocol = htons(PVC_PROT);
	sa.sll_family=AF_WANPIPE;

	if(bind(sock, (struct sockaddr *)&sa, sizeof(struct wan_sockaddr_ll)) < 0){
	lib_printf(0,NULL,"bind failed on span span %i %s\n",
		span->span_no+1,strerror(errno));
			return -1;
	}

	span->sock=sock;

	err= sangoma_get_cfg(span);
	if (err) {
		goto sangoma_hptdm_span_open_error;
	}

	lib_printf(0,NULL,"libhp: span %i opened\n",
			span->span_no+1);

	return 0;

sangoma_hptdm_span_open_error:

	if (span->sock) {
		close(span->sock);
		span->sock=-1;
	}

	return err;
}


/*!
  \brief Read oob event from hw and push oob event up to the user. Called after select()
  \param span span object
  \return 0 = pass, non zero fail
*/

int sangoma_hp_tdm_handle_oob_event(sangoma_hptdm_span_t *span)
{
	int err;
	hp_tdmapi_rx_event_t *rx_event;

	err = recv(span->sock, span->rx_data, sizeof(span->rx_data), MSG_OOB);
	if (err > 0) {
		rx_event=(hp_tdmapi_rx_event_t*)&span->rx_data;
		if (span->span_reg.rx_event && span->span_reg.p) {
			span->span_reg.rx_event(span->span_reg.p, rx_event);
		}
	}
	/* For now treat all err as socket reload */
	return 1;
}

/*!
  \brief Multiplex span rx data and push it up to each channel
  \param span span object
  \return 0 = pass, non zero fail
*/

int sangoma_hp_tdm_push_rx_data(sangoma_hptdm_span_t *span)
{
	int i=0,x=0,err=0;
	sangoma_hptdm_chan_t *chan=NULL;
	hp_tmd_chunk_t *chunk=NULL;
	int chan_no_hw;
	char *rx_data = &span->rx_data[sizeof(api_rx_hdr_t)];


	for (i=0;i<SMG_HP_TDM_MAX_CHANS;i++) {
		chan = &span->chan_idx[i].chan;
		if (!chan->init) {
			continue;
		}
		chan_no_hw = span->chan_idx[i].chan_no_hw;

		lib_printf(15,NULL, "SPAN %i Chan %i Chunk %i Channelizing\n",
				span->span_no+1, chan->chan_no+1, span->chunk_sz);

		chunk = &chan->rx_chunk;

		for (x=0;x<span->chunk_sz;x++) {
			chunk->data[x] = rx_data[(span->max_chans*x)+chan_no_hw];
		}

		chunk->len = span->chunk_sz;
		if (chan->chan_reg.p && chan->chan_reg.rx_data) {
			err=chan->chan_reg.rx_data(chan->chan_reg.p,chunk->data,chunk->len);
		} else {
			err=1;
		}

		if (err) {
			chan->init=0;
			chan->chan_reg.p=NULL;
		}

	}

	return err;


}

/*!
  \brief Read data from hw interface and pass it up to the rxdata handler:  called by run_span()
  \param span span object
  \return 0 = pass, non zero fail
*/

int sangoma_hp_tdm_handle_read_event(sangoma_hptdm_span_t *span)
{
	int err;
	hp_tdmapi_rx_event_t *rx_event = (hp_tdmapi_rx_event_t*)&span->rx_data[0];

	err = sangoma_readmsg_hp_tdm(span->sock,
                                     &span->rx_data[0],sizeof(api_rx_hdr_t),
				     &span->rx_data[sizeof(api_rx_hdr_t)],
				     sizeof(span->rx_data) - sizeof(api_rx_hdr_t),
				     0);

	if (err >  sizeof(api_rx_hdr_t)) {

		lib_printf(15,NULL, "SPAN %i Read Len = %i\n",span->span_no+1,err);

		if (rx_event->event_type) {

			if (span->span_reg.rx_event && span->span_reg.p) {
				span->span_reg.rx_event(span->span_reg.p, rx_event);
			}

		} else {

			if (err % span->chunk_sz) {
				lib_printf(0,NULL, "Error: SPAN %i Read Len = %i Block not chunk %i aligned! \n",span->span_no+1,err,span->chunk_sz);
				/* Received chunk size is not aligned drop it for now */
				return 0;
			}

			sangoma_hp_tdm_push_rx_data(span);
		}

		err=0;

	} else {
		if (errno == EAGAIN) {
			err = 0;
		} else {
			err=-1;
		}
	}

	return err;
}


/*!
  \brief Pull data from all chans and write data to hw interface: called by run_span()
  \param span span object
  \return 0 = pass, non zero fail
*/
int sangoma_hp_tdm_handle_write_event(sangoma_hptdm_span_t *span)
{
	int i=0,x=0,err=0;
	sangoma_hptdm_chan_t *chan;
	hp_tmd_chunk_t *chunk;
	int chan_no_hw=0;
	char *tx_data = &span->tx_data[sizeof(api_tx_hdr_t)];

	memset(&span->tx_data,span->idle,sizeof(span->tx_data));


	for (i=0;i<SMG_HP_TDM_MAX_CHANS;i++) {
		chan = &span->chan_idx[i].chan;
		if (!chan->init) {
			/* This channel is not open */
			continue;
		}
		chan_no_hw=span->chan_idx[i].chan_no_hw;

		chunk = &chan->tx_idx[chan->tx_idx_out];
		if (!chunk->init) {
			lib_printf(15,NULL,"span write s%ic%i tx chunk underrun out %i \n",
					chan->span_no+1,chan->chan_no+1,chan->tx_idx_out);
			/* There is no tx data for this channel */
			continue;
		}

		for (x=0;x<span->chunk_sz;x++) {

			tx_data[(span->max_chans*x)+chan_no_hw] = chunk->data[chunk->offset];
			chunk->offset++;

			if (chunk->offset >= chunk->len) {
				chunk->init=0;

				lib_printf(15,NULL,"span write s%ic%i tx chunk out %i \n",
					chan->span_no+1,chan->chan_no+1,chan->tx_idx_out);

				chan->tx_idx_out++;
				if (chan->tx_idx_out >= SMG_HP_TDM_CHUNK_IDX_SZ) {
					chan->tx_idx_out=0;
				}
				chunk=&chan->tx_idx[chan->tx_idx_out];
				if (!chunk->init) {
					/* We are out of tx data on this channel */
					break;
				}
			}
		}
	}


	err = sangoma_writemsg_hp_tdm(span->sock,
				     span->tx_data,sizeof(api_tx_hdr_t),
				     tx_data, span->tx_size,
				     0);

	lib_printf(15, NULL, "SPAN %i TX Len %i Expecting %i\n",
			span->span_no+1, err, span->tx_size+sizeof(api_tx_hdr_t));

	if (err < span->tx_size) {
		if (errno == EAGAIN) {
			return 0;
		} else {
			return -1;
		}
	} else {
		err=0;
	}

	return err;

}
