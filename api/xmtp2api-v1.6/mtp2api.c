/*****************************************************************************
* mpt2api.c	SS7 MTP2 API Example Code
*
* Author(s):	Nenad Corbic <ncorbic@sangoma.com>
*		Michael Mueller <mmueller@gmail.com>
*
* Copyright:	(c) 2003-2010 Sangoma Technologies Inc.
*                             Xygnada Technologies.
* ============================================================================
*
* 1.6   Nenad Corbic <ncorbic@sangoma.com>
*	May 28 2010
*   Updated for 8 E1 worth of links.
*   Synched up with latest xmtp2 code
*
* 1.5   Nenad Corbic <ncorbic@sangoma.com>
*	Sep 02 2009
*   Updated for full t1/e1 config
*
* 1.4   Nenad Corbic <ncorbic@sangoma.com>
*	Aug 19 2009
*	Updated for full t1/e1 config
* 
* 1.2   Nenad Corbic <ncorbic@sangoma.com>
*	May 21 2008
*	Updated so mtp2 links can be configured and
*       reconfigured on the fly.
*
* 1.1 	Nenad Corbic <ncorbic@sangoma.com>
* 	May 1 2008
*	Updated MTP2 Timer configuration
*
* 1.0 	Nenad Corbic <ncorbic@sangoma.com>
* 	May 1 2008
*	Initial Version
*/


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/if.h>
#include "libxmtp2.h"


/*===================================================================
 * Local Defines
 *==================================================================*/
static int mtp2api_exit=0;
static int mtp2api_reconfig=0;

#define MAX_MTP2_DEVS 0xFFFF

#define MTP2_DEV_KEY(_lset,_link) ((_lset&0xFF) << 8 | (_link&0xFF))

typedef struct mtp2_dev
{
	xmtp_cfg_link_info_t cfg;
	
	int state;
	int init;
	int fd;

	int (*rx_msu)(struct mtp2_dev *dev, t_su_buf * p_msu);
	int (*state_change)(struct mtp2_dev *dev, int state);

} mtp2_dev_t;

mtp2_dev_t mtp2_dev_idx[MAX_MTP2_DEVS];



/*===================================================================
 * state_change : callback function
 * 
 * Change of MTP2 State  
 * 
 *==================================================================*/
int mtp2_state_change (struct mtp2_dev *dev, int state)
{
	printf("MTP2 LinkSet %i Link %i State Change %i\n",
			dev->cfg.linkset, dev->cfg.link, state);


	/* FIXME: Do something with state change */

	dev->state = state;
	
	return 0;
}


/*===================================================================
 * rx_msu : callback function
 * 
 * Received MSU packet.  
 * Return 0: This function takes control over p_msu
 * Return Non 0: Lower layer will free p_msu
 *==================================================================*/
int mtp2_rx_msu (struct mtp2_dev *dev, t_su_buf * p_msu)
{
	int data_len;
	unsigned char *data;

	printf("MTP2 LinkSet %i Link %i Received MSU Len=%i\n",
			dev->cfg.linkset, dev->cfg.link,p_msu->hdr.su_octet_count);
	
	/* The su_octet_count contains 3 bytes header and 2 bytes crc 
	 * To get true data len we must decrement the octet_cout by 5 */
	data_len = p_msu->hdr.su_octet_count - 5;

	/* First 3 bytes are mtp2 header and last 2 are crc */
	data = (unsigned char*)&p_msu->su_buf[3];	

	/* FIXME: Do something with received MSU */	

	xmtp2_print_msu(p_msu);

	free(p_msu);
	
	return 0;
}

/*===================================================================
 * transfer_L2_to_L3
 * 
 * Read xmtp2 packets and determine the packet type
 *==================================================================*/
int transfer_L2_to_L3 (int xmtp2km_fd)
{
	t_su_buf * p_msu;
	int r;
	mtp2_dev_t *mtp2dev;

X4_MTP2_TO_MTP3:

	p_msu = malloc(sizeof(t_su_buf));
	if (!p_msu) {
		return -1;
	}
	memset(p_msu,0,sizeof(t_su_buf));

	r = xmtp2_read_msu(xmtp2km_fd,p_msu);
	if (r < 0){
		info_u (__FILE__, __FUNCTION__,0,
"failed:ioctl XMTP2KM_IOCG_GETMSU:retval and p_msu follow", r);
		return -1;
	}

	if (r == 0){
		free (p_msu);
		return r;
	}

	mtp2dev = &mtp2_dev_idx[MTP2_DEV_KEY(p_msu->hdr.linkset,p_msu->hdr.link)];
	
	if (!mtp2dev->init) {
		printf("Error: Device for Linkset %i and Link %i Not inialized!\n",
				p_msu->hdr.linkset,p_msu->hdr.link);	
		free (p_msu);
		return r;
	}

	/* distribute messages and indicators from L2 to L3 */
	switch (p_msu->hdr.msg_type)
	{
		case LINK_CONNECTED:
			info_u (__FILE__, __FUNCTION__,0,
"facility LINK_CONNECTED:facility/slot follow", p_msu->hdr.facility);
			
			if (mtp2dev->state_change) {
				mtp2dev->state_change(mtp2dev,p_msu->hdr.msg_type);
			}
			xmtp2_cmd (xmtp2km_fd, p_msu->hdr.linkset, p_msu->hdr.link, LSC_CAUSE_START, LSAC);
			break;

		case LINK_NOT_CONNECTED:
			info_u (__FILE__, __FUNCTION__,0, 
"facility LINK_NOT_CONNECTED:facility/slot follow", p_msu->hdr.facility);
			if (mtp2dev->state_change) {
				mtp2dev->state_change(mtp2dev,p_msu->hdr.msg_type);
			}
			break;

		case IN_SERVICE:
			info_u (__FILE__, __FUNCTION__,0,
"facility IN_SERVICE:facility/slot follow", p_msu->hdr.facility);
			if (mtp2dev->state_change) {
				mtp2dev->state_change(mtp2dev,p_msu->hdr.msg_type);
			}
			break;

		case OUT_OF_SERVICE:
			info_u (__FILE__, __FUNCTION__,0,
"facility OUT_OF_SERVICE:facility/slot follow", p_msu->hdr.facility);
			if (mtp2dev->state_change) {
				mtp2dev->state_change(mtp2dev,p_msu->hdr.msg_type);
			}
			xmtp2_cmd (xmtp2km_fd, p_msu->hdr.linkset, p_msu->hdr.link, LSC_CAUSE_START, LSAC);
			break;

		case REMOTE_PROCESSOR_OUTAGE:
			info_u (__FILE__, __FUNCTION__,0,
"facility REMOTE_PROCESSOR_OUTAGE:facility/slot follow", p_msu->hdr.facility);
			if (mtp2dev->state_change) {
				mtp2dev->state_change(mtp2dev,p_msu->hdr.msg_type);
			}
			break;

		case REMOTE_PROCESSOR_RECOVERED:
			info_u (__FILE__, __FUNCTION__,0,
"facility REMOTE_PROCESSOR_RECOVERED:facility/slot follow", p_msu->hdr.facility);
			if (mtp2dev->state_change) {
				mtp2dev->state_change(mtp2dev,p_msu->hdr.msg_type);
			}
			break;

		case RTB_CLEARED:
			info_u ( __FILE__, __FUNCTION__,0,
"RTB_CLEARED msg :fac/slot/ls/link follow", 
				p_msu->hdr.facility);
			break;

		case RX_MESSAGE:
			info_u ( __FILE__, __FUNCTION__,0,
"RX_MESSAGE msg :fac/slot/ls/link follow", 
				p_msu->hdr.facility);

			if (mtp2dev->rx_msu) {
				int err=mtp2dev->rx_msu(mtp2dev,p_msu);
				if (err == 0) {
					/* The higher layer has the p_msu */
					p_msu=NULL;
				}
			} 
			break;

		case RX_BUF_CLEARED:
			info_u ( __FILE__, __FUNCTION__,0,
"RX_BUF_CLEARED msg :fac/slot/ls/link follow", 
				p_msu->hdr.facility);

			break;
		case BSNT:
			info_u ( __FILE__, __FUNCTION__,0,
"BSNT msg not handled with reply msg:fac/slot/ls/link follow", 
				p_msu->hdr.facility);
			/* ERROR */
			break;
		case RETRIEVED_MESSAGES:
			info_u ( __FILE__, __FUNCTION__,0,
"RETRIEVED_MESSAGES msg not handled with reply msg:fac/slot/ls/link follow", 
				p_msu->hdr.facility);
			/* ERROR */
			break;

		case RETRIEVAL_COMPLETE:
			info_u ( __FILE__, __FUNCTION__,0,
"RETRIEVAL_COMPLETE msg :fac/slot/ls/link follow", 
				p_msu->hdr.facility);
			break;

		case LINK_CONGESTION_ONSET_TX:
			info_u ( __FILE__, __FUNCTION__,0,
"LINK_CONGESTION_ONSET_TX msg :fac/slot/ls/link follow", 
				p_msu->hdr.facility);
			break;

		case LINK_CONGESTION_ONSET_RX:
			info_u ( __FILE__, __FUNCTION__,0,
"LINK_CONGESTION_ONSET_RX msg :fac/slot/ls/link follow", 
				p_msu->hdr.facility);
			break;

		case LINK_CONGESTION_CEASE_TX:
			info_u ( __FILE__, __FUNCTION__,0,
"LINK_CONGESTION_CEASE_TX msg :fac/slot/ls/link follow", 
				p_msu->hdr.facility);
			break;

		case LINK_CONGESTION_CEASE_RX:
			info_u ( __FILE__, __FUNCTION__,0,
"LINK_CONGESTION_CEASE_RX msg :fac/slot/ls/link follow", 
				p_msu->hdr.facility);
			break;
		default:
			info_u ( __FILE__, __FUNCTION__,0,
"msg invalid msg :fac/slot/ls/link follow", 
				p_msu->hdr.msg_type);
			/* ERROR */
			break;
	} /* switch */

	if (p_msu) {
		free (p_msu);
	}
	goto X4_MTP2_TO_MTP3;
}

/*===================================================================
 *
 *==================================================================*/
int	wait_for_input (int fd, int ms)
{
	struct timeval  t;
	
	//FD_ZERO (&read_socks);
	//FD_ZERO (&write_socks);
	//FD_ZERO (&oob_socks);

	/* blocking select */
	
        t.tv_sec = 0;
        t.tv_usec = ms*1000;
  	//select(maxsock + 1, &read_socks, &write_socks, &oob_socks, &t);
  	select(fd + 1, NULL, NULL, NULL, &t);

	return 0;
}

/*===================================================================
 * tx_msu
 *
 * Example of how to send msu packet
 *==================================================================*/
int tx_msu(struct mtp2_dev *dev) 
{
	int i;
	t_su_buf t_msu;
	int data_len=10;
	memset(&t_msu,0,sizeof(t_msu));


	if (dev->state != IN_SERVICE) {
		return -1;
	}

	/* Specify where the msu is going */
	t_msu.hdr.facility	=dev->cfg.facility;
	t_msu.hdr.slot		=dev->cfg.slot; 
	t_msu.hdr.linkset	=dev->cfg.linkset; 
	t_msu.hdr.link		=dev->cfg.link; 
	
	/* Specify msu len and data 
	 * First 3 bytes will be overwritten by lower layer
         * start data on the 4th octet */
	t_msu.hdr.su_octet_count=data_len;
	for (i=0;i<t_msu.hdr.su_octet_count;i++) {	
		t_msu.su_buf[i+3]=i;	
	}

	/* We must add 5 bytes to octet count to account for
	 * 3 byte header and 2 byte crc tail -> this is explained in libxmtp2.h */
	t_msu.hdr.su_octet_count+=5;

	printf("LINK TX MSU f=%i,slot=%i,ls=%i,l=%i,mt=%i\n",
		t_msu.hdr.facility,t_msu.hdr.slot,t_msu.hdr.linkset,
		t_msu.hdr.link,t_msu.hdr.msg_type);
	

	/* Transmit MSU */
	return xmtp2_send_msu(dev->fd,&t_msu);
}


/*===================================================================
 * Signal Handlers
 *
 * Example of how to send msu packet
 *==================================================================*/
static int do_shut(int sig)
{
	mtp2api_exit=1;
	return 0;
}

static int do_ignore(int sig)
{
	mtp2api_reconfig=1;
	return 0;
}


/*===================================================================
 * Main Code
 *==================================================================*/
int main(int argc, char* argv[])
{
	int fd;
	int err;
	int cnt=0;
	int i;
	mtp2_dev_t *mtp2dev;

	/* FIXME: Used to obain full link status reports */
	t_link_opm opm_table[MAX_MTP2LINKS];
	memset(opm_table,0,sizeof(opm_table));
	
	(void) signal(SIGINT,(void *) do_shut);
    	(void) signal(SIGPIPE,(void *) do_ignore);
    	(void) signal(SIGUSR1,(void *) do_ignore);
    	(void) signal(SIGHUP,(void *) do_shut);


	xmtp2_init();

	err=xmtp2_load();
	if (err < 0) {
		exit (-1);
	}

	fd = xmtp2_open();
	if (fd < 0) {
		exit (-1);
	}

reconfig:

	/* MTP2 Configuration based on wancfg_smg configurator */
	
#if 1
	/* Configuration per T1/E1 timeslot, where a mtp2 link exists
       on a singel timeslot of a T1/E1 port.  In this example
       we are configuring 16 mtp2 links. However, this example
       can be rewritten per user config
	   Sample wanpipe config file:  wanpipe/wanpipe1.conf.16_channels_e1
	*/
	for (i=0;i<16;i++) {

		/* Initialize Local MTP2 Device Structure: Likset 0 Link 0 */
		mtp2dev = &mtp2_dev_idx[MTP2_DEV_KEY(0,i)];

		memset(mtp2dev,0,sizeof(mtp2_dev_t));

		mtp2dev->init=1;
		mtp2dev->state=LINK_NOT_CONNECTED;
		mtp2dev->cfg.card=1;		/* Number corresponding to wanpipe config file number: wanpipe1.conf */
		mtp2dev->cfg.slot=i;		/* T1/E1 timeslot/interface number -> corresponds to interface number
									   in wanpipe interface name - w1g1 in wanpipe1.conf */
		mtp2dev->cfg.clear_ch=1;	/* 1=8bit hdlc  0=7bit hdlc */
		mtp2dev->cfg.linkset=0;		/* LinkSet number */	
		mtp2dev->cfg.link=i;		/* Link in LinkSet */
		mtp2dev->cfg.mtu=80*1;		/* Block size per timeslot configured */

		/* Configure MTP2 Timters */
		mtp2dev->cfg.cfg_T1 = MTP2_SEC_MS(13.0);	 
		mtp2dev->cfg.cfg_T2 = MTP2_SEC_MS(11.5);
		mtp2dev->cfg.cfg_T3 = MTP2_SEC_MS(11.5);
		mtp2dev->cfg.cfg_T4Pe = MTP2_SEC_MS(0.6);
		mtp2dev->cfg.cfg_T4Pn = MTP2_SEC_MS(2.30);
		mtp2dev->cfg.cfg_T5 = MTP2_SEC_MS(0.12);
		mtp2dev->cfg.cfg_T6 = MTP2_SEC_MS(3.0);
		mtp2dev->cfg.cfg_T7 = MTP2_SEC_MS(1.0);

		mtp2dev->rx_msu=mtp2_rx_msu;
		mtp2dev->state_change=mtp2_state_change;
		mtp2dev->fd=fd;
		
		/* Stop current MTP2 Link */
		err=xmtp2_stop_link (fd, &mtp2dev->cfg);
		if (err) {
			exit(-1);
		}

		/* Configure MTP2 Link */
		err=xmtp2_conf_link (fd, &mtp2dev->cfg);
		if (err) {
			exit(-1);
		}

		/* Power ON MTP2 Link */
		err=xmtp2_cmd (fd, mtp2dev->cfg.linkset, mtp2dev->cfg.link, LSC_CAUSE_POWER_ON, MGMT);
		if (err) {
			exit(-1);
		}

		/* Start physical T1/E1 channel for the configured MTP2 Link 
		   If the port is already running this command will be skipped,
		   The /etc/wanpipe/wanpipe#.conf file must be properly configured   */ 
		err=xmtp2_start_facility(mtp2dev->cfg.card,mtp2dev->cfg.slot);
		if (err) {
			exit (-1);
		}

		mtp2dev=NULL;
	}

#else

	/* Configuration for FULL T1/E1 port, where a mtp2 link exists
       on all timeslot of a T1/E1 port.  In this example
       we are configuring 1 mtp2 link over full T1/E1.
       Sample wanpipe config file:  wanpipe/wanpipe1.conf.full_e1
     */
	for (i=0;i<1;i++) {

		/* Initialize Local MTP2 Device Structure: Likset 0 Link 0 */
		mtp2dev = &mtp2_dev_idx[MTP2_DEV_KEY(0,i)];

		memset(mtp2dev,0,sizeof(mtp2_dev_t));

		mtp2dev->init=1;
		mtp2dev->state=LINK_NOT_CONNECTED;
		mtp2dev->cfg.card=1;		/* Number corresponding to wanpipe config file number: wanpipe1.conf */
		mtp2dev->cfg.slot=i;		/* T1/E1 timeslot/interface number -> corresponds to interface number
									   in wanpipe interface name - w1g1 in wanpipe1.conf */
		mtp2dev->cfg.clear_ch=1;	/* 1=8bit hdlc  0=7bit hdlc */
		mtp2dev->cfg.linkset=0;		/* LinkSet number */	
		mtp2dev->cfg.link=i;		/* Link in LinkSet */
		mtp2dev->cfg.mtu=80*31;		/* Block size per timeslot configured.
									   In this example its all 31 timeslosts
									   IMPORTANTA: wanpipe1.conf must be configured with ACTIVE_CH=ALL */

		/* Configure MTP2 Timters */
		mtp2dev->cfg.cfg_T1 = MTP2_SEC_MS(13.0);	 
		mtp2dev->cfg.cfg_T2 = MTP2_SEC_MS(11.5);
		mtp2dev->cfg.cfg_T3 = MTP2_SEC_MS(11.5);
		mtp2dev->cfg.cfg_T4Pe = MTP2_SEC_MS(0.6);
		mtp2dev->cfg.cfg_T4Pn = MTP2_SEC_MS(2.30);
		mtp2dev->cfg.cfg_T5 = MTP2_SEC_MS(0.12);
		mtp2dev->cfg.cfg_T6 = MTP2_SEC_MS(3.0);
		mtp2dev->cfg.cfg_T7 = MTP2_SEC_MS(1.0);

		mtp2dev->rx_msu=mtp2_rx_msu;
		mtp2dev->state_change=mtp2_state_change;
		mtp2dev->fd=fd;
		
		/* Stop current MTP2 Link */
		err=xmtp2_stop_link (fd, &mtp2dev->cfg);
		if (err) {
			exit(-1);
		}

		/* Configure MTP2 Link */
		err=xmtp2_conf_link (fd, &mtp2dev->cfg);
		if (err) {
			exit(-1);
		}

		/* Power ON MTP2 Link */
		err=xmtp2_cmd (fd, mtp2dev->cfg.linkset, mtp2dev->cfg.link, LSC_CAUSE_POWER_ON, MGMT);
		if (err) {
			exit(-1);
		}

		/* Start physical T1/E1 channel for the configured MTP2 Link 
		   If the port is already running this command will be skipped,
		   The /etc/wanpipe/wanpipe#.conf file must be properly configured   */ 
		err=xmtp2_start_facility(mtp2dev->cfg.card,mtp2dev->cfg.slot);
		if (err) {
			exit (-1);
		}

		mtp2dev=NULL;
	}

#endif


	/* Print out full link Report */
	err=xmtp2_report_link_load(fd,opm_table);	
	if (err) {
		exit (-1);
	}

	/* Start the Main Loop */

	for (;;) {	

		/* Check for any MTP2 events */
		err=transfer_L2_to_L3(fd);
		if (err < 0) {
			exit(1);
		}
	
		/* Delay 100ms */
		wait_for_input (fd,100);
	
		/* Print out link report every 10 sec */	
		if (++cnt % 100 == 0) {
			cnt=0;
			xmtp2_report_link_load(fd,opm_table);
	
			/* FIXME: This is just an example one would spawn a tx thread 
			or put the tx_msu function as part of higher state machine */
	
			mtp2dev = &mtp2_dev_idx[MTP2_DEV_KEY(0,0)];
			if (mtp2dev->init) {
				tx_msu(mtp2dev);
			}
			mtp2dev = &mtp2_dev_idx[MTP2_DEV_KEY(1,0)];
			if (mtp2dev->init) {
				tx_msu(mtp2dev);
			}
		}

#if 0
		/* Transmit each time */
		mtp2dev = &mtp2_dev_idx[MTP2_DEV_KEY(0,0)];
			if (mtp2dev->init) {
				tx_msu(mtp2dev);
			}
#endif
	
		if (mtp2api_exit){
			break;
		}

		if (mtp2api_reconfig){
			mtp2api_reconfig=0;
			goto reconfig;
		}
		
	}

	xmtp2_close(fd);
#if 0
	xmtp2_stop_facilities();
	xmtp2_unload();
#endif	
	return 0;
}

