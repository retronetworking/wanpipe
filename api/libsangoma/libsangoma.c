/*****************************************************************************
 * libsangoma.c	AFT T1/E1: HDLC API Code Library
 *
 * Author(s):	Anthony Minessale II <anthmct@yahoo.com>
 *              Nenad Corbic <ncorbic@sangoma.com>
 *
 * Copyright:	(c) 2005 Anthony Minessale II
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 */

#include "libsangoma.h"

#if 1
/* Enable Sangoma Scatter Gather Socket Calls */
#define SANGOMA_SC_SOCKET 1
#else
/* Disable Sangoma Scatter Gather Socket Calls */
#undef SANGOMA_SC_SOCKET 
#endif


int sangoma_create_socket_intr(int span, int intr) 
{
	struct wan_sockaddr_ll  sa;
	int sock=-1;
	
	memset(&sa, 0, sizeof(struct wan_sockaddr_ll));
	sock = socket(AF_WANPIPE, SOCK_RAW, 0);
	if ( sock >= 0 ) {
		sprintf(sa.sll_device, "w%dg%d", span, intr);
        strcpy( sa.sll_card, "wanpipe1");
        sa.sll_protocol = htons(PVC_PROT);
        sa.sll_family=AF_WANPIPE;

        if (bind(sock, (struct sockaddr *)&sa, sizeof(struct wan_sockaddr_ll)) < 0) {
			close(sock);
			sock = -1;
			printf("Fail %d %s [%s]\n", sock, sa.sll_device, strerror(errno));
        }

	} 


	return sock;
}


int sangoma_create_socket(int span) 
{
	int sock=-1;
	int i;

	for (i=1; i<32; i++) {
		if (i == 16 || i == 24) {
			continue;
		}
		if ((sock=sangoma_create_socket_intr(span, i) >= 0)) {
			break;
		}
	}

	return sock;

}


int sangoma_write_socket(int sock, void *data, int len)
{

	return send(sock,data,len,0);
}


/*==============================================================
 * sangoma_readmsg_socket
 * 
 * Transmits wanpipe header and data using two separate buffers
 *
 * INPUTS: 
 *   
 *  hdrbuf  = header buffer must be size of sizeof(api_rx_hdr_t) 16bytes
 *  hdrlen  = lenght of the header buffer sizeof(api_rx_hdr_t) 16bytes
 *   
 *  databuf = data buffer that will contain the actual payload
 *  datalen = length of the data buffer. Always set it to MAX value.
 *  
 * OUTPUT:
 * 
 *  hdrbuf  = will contain Sangoma 16 byte header
 *  databuf = will contain data payload
 *  
 *  Function returns length of received databuf WITHOUT the header.
 *  
 */


int sangoma_sendmsg_socket(int sock, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag)
{
	int bsent;
	struct msghdr msg;
	struct iovec iov[2];

	memset(&msg,0,sizeof(struct msghdr));

	iov[0].iov_len=hdrlen;
	iov[0].iov_base=hdrbuf;

	iov[1].iov_len=datalen;
	iov[1].iov_base=databuf;

	msg.msg_iovlen=2;
	msg.msg_iov=iov;

	bsent = sendmsg(sock,&msg,flag);
	if (bsent > 0){
		bsent-=sizeof(sangoma_api_hdr_t);
	}

	return bsent;
}



int sangoma_read_socket(int sock, void *data, int len)
{
	api_rx_element_t *api_rx_el;
	int rx_len = recv(sock,data,len,0);

	if (rx_len <= sizeof(api_rx_hdr_t)){
		return -EINVAL;
	}

	api_rx_el = (api_rx_element_t*)data;


	if (api_rx_el->api_rx_hdr.error_flag){
		return -EINVAL;
	}

	return rx_len;
}

/*==============================================================
 * sangoma_readmsg_socket
 *
 * Receives wanpipe header and data in two separate buffers
 *
 * INPUTS: 
 * 
 * hdrbuf  = header buffer must be size of sizeof(api_rx_hdr_t) 16bytes
 * hdrlen  = lenght of the header buffer sizeof(api_rx_hdr_t) 16bytes
 *
 * databuf = data buffer that will contain the actual payload
 * datalen = length of the data buffer. Always set it to MAX value.
 *
 * OUTPUT:
 *
 * hdrbuf  = will contain Sangoma 16 byte header
 * databuf = will contain data payload
 *
 * Function returns length of received databuf WITHOUT the header.
 *
 */

int sangoma_readmsg_socket(int sock, void *hdrbuf, int hdrlen, void *databuf, int datalen, int flag)
{
	int rx_len;
	struct msghdr msg;
	struct iovec iov[2];

	memset(&msg,0,sizeof(struct msghdr));

	iov[0].iov_len=hdrlen;
	iov[0].iov_base=hdrbuf;

	iov[1].iov_len=datalen;
	iov[1].iov_base=databuf;

	msg.msg_iovlen=2;
	msg.msg_iov=iov;

	rx_len = recvmsg(sock,&msg,flag);

	if (rx_len <= sizeof(sangoma_api_hdr_t)){
		return -EINVAL;
	}

	rx_len-=sizeof(sangoma_api_hdr_t);

        return rx_len;
}


