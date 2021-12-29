/*****************************************************************************
 * call_signal.c -- Signal Specifics
 *
 * Author(s):	Anthony Minessale II <anthmct@yahoo.com>
 *              Nenad Corbic <ncorbic@sangoma.com>
 *
 * Copyright:	(c) 2005 Nenad Corbic 
 *		         Anthony Minessale II
 * 
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 */

#include <call_signal.h>

extern void __log_printf(int level, FILE *fp, char *file, const char *func, int line, char *fmt, ...);
#define clog_printf(level, fp, fmt, ...) __log_printf(level, fp, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)


struct call_signal_map {
	uint32_t event_id;
	char *name;
};

static struct call_signal_map call_signal_table[] = {
	{SIGBOOST_EVENT_CALL_START, "CALL_START"},
	{SIGBOOST_EVENT_CALL_START_ACK, "CALL_START_ACK"},
	{SIGBOOST_EVENT_CALL_START_NACK, "CALL_START_NACK"},
	{SIGBOOST_EVENT_CALL_START_NACK_ACK, "CALL_START_NACK_ACK"},
	{SIGBOOST_EVENT_CALL_ANSWERED, "CALL_ANSWERED"},
	{SIGBOOST_EVENT_CALL_STOPPED, "CALL_STOPPED"},
	{SIGBOOST_EVENT_CALL_STOPPED_ACK, "CALL_STOPPED_ACK"},
	{SIGBOOST_EVENT_SYSTEM_RESTART, "SYSTEM_RESTART"},
	{SIGBOOST_EVENT_SYSTEM_RESTART_ACK, "SYSTEM_RESTART_ACK"},
	{SIGBOOST_EVENT_HEARTBEAT, "HEARTBEAT"},
	{SIGBOOST_EVENT_INSERT_CHECK_LOOP, "LOOP START"},
	{SIGBOOST_EVENT_REMOVE_CHECK_LOOP, "LOOP STOP"} 
}; 

#define USE_SCTP 1

static int create_udp_socket(call_signal_connection_t *mcon, char *local_ip, int local_port, char *ip, int port)
{
	int rc;
	struct hostent *result, *local_result;
	char buf[512], local_buf[512];
	int err = 0;

	memset(&mcon->remote_hp, 0, sizeof(mcon->remote_hp));
	memset(&mcon->local_hp, 0, sizeof(mcon->local_hp));
#ifdef USE_SCTP
	mcon->socket = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
#else
	mcon->socket = socket(AF_INET, SOCK_DGRAM, 0);
#endif
 
	clog_printf(3,mcon->log,"Creating L=%s:%d R=%s:%d\n",
			local_ip,local_port,ip,port);

	if (mcon->socket >= 0) {
		int flag=1;
		gethostbyname_r(ip, &mcon->remote_hp, buf, sizeof(buf), &result, &err);
		gethostbyname_r(local_ip, &mcon->local_hp, local_buf, sizeof(local_buf), &local_result, &err);
		if (result && local_result) {
			mcon->remote_addr.sin_family = mcon->remote_hp.h_addrtype;
			memcpy((char *) &mcon->remote_addr.sin_addr.s_addr, mcon->remote_hp.h_addr_list[0], mcon->remote_hp.h_length);
			mcon->remote_addr.sin_port = htons(port);

			mcon->local_addr.sin_family = mcon->local_hp.h_addrtype;
			memcpy((char *) &mcon->local_addr.sin_addr.s_addr, mcon->local_hp.h_addr_list[0], mcon->local_hp.h_length);
			mcon->local_addr.sin_port = htons(local_port);

#ifdef USE_SCTP
			setsockopt(mcon->socket, IPPROTO_SCTP, SCTP_NODELAY, 
				   (char *)&flag, sizeof(int));
#endif

			if ((rc = bind(mcon->socket, 
				  (struct sockaddr *) &mcon->local_addr,
				   sizeof(mcon->local_addr))) < 0) {
				close(mcon->socket);
				mcon->socket = -1;
			} else {
#ifdef USE_SCTP
				rc=listen(mcon->socket,100);
				if (rc) {
					close(mcon->socket);
	                                mcon->socket = -1;
				}
#endif
			}
		}
	}

	return mcon->socket;
}

int call_signal_connection_close(call_signal_connection_t *mcon)
{
	close(mcon->socket);
	mcon->socket = -1;
	memset(mcon, 0, sizeof(*mcon));

	return 0;
}

int call_signal_connection_open(call_signal_connection_t *mcon, char *local_ip, int local_port, char *ip, int port)
{
	create_udp_socket(mcon, local_ip, local_port, ip, port);
	return mcon->socket;
}

call_signal_event_t *call_signal_connection_read(call_signal_connection_t *mcon, int iteration)
{
	unsigned int fromlen = sizeof(struct sockaddr_in);
#if 0
	call_signal_event_t *event = &mcon->event;
#endif
	int bytes = 0;

	bytes = recvfrom(mcon->socket, &mcon->event, sizeof(mcon->event), MSG_DONTWAIT, 
			(struct sockaddr *) &mcon->local_addr, &fromlen);

	if (bytes == sizeof(mcon->event) || 
            bytes == (sizeof(mcon->event)-sizeof(uint32_t))) {

		if (mcon->rxseq_reset) {
			if (mcon->event.event_id == SIGBOOST_EVENT_SYSTEM_RESTART_ACK) {
				clog_printf(0,mcon->log,"Rx sync ok\n");
				mcon->rxseq=mcon->event.fseqno;
				return &mcon->event;
			}
			errno=EAGAIN;
			clog_printf(0,mcon->log,"Waiting for rx sync...\n");
			return NULL;
		}
		
		mcon->txwindow = mcon->txseq - mcon->event.bseqno;
		mcon->rxseq++;

		if (mcon->rxseq != mcon->event.fseqno) {
			clog_printf(0, mcon->log, 
				"------------------------------------------\n");
			clog_printf(0, mcon->log, 
				"Critical Error: Invalid Sequence Number Expect=%i Rx=%i\n",
				mcon->rxseq,mcon->event.fseqno);
			clog_printf(0, mcon->log, 
				"------------------------------------------\n");
		}

#if 0
/* Debugging only not to be used in production because span/chan can be invalid */
	   	if (mcon->event.span < 0 || mcon->event.chan < 0 || mcon->event.span > 16 || mcon->event.chan > 31) {
                	clog_printf(0, mcon->log,
                        	"------------------------------------------\n");
                	clog_printf(0, mcon->log,
                        	"Critical Error: RX Cmd=%s Invalid Span=%i Chan=%i\n",
                        	call_signal_event_id_name(event->event_id), event->span,event->chan);
                	clog_printf(0, mcon->log,
                        	"------------------------------------------\n");
		
			errno=EAGAIN;
                	return NULL;
        	}
#endif

 


		return &mcon->event;
	} else {
		if (iteration == 0) {
                	clog_printf(0, mcon->log,
                        	"------------------------------------------\n");
                	clog_printf(0, mcon->log,
                        	"Critical Error: Invalid Event lenght from boost rxlen=%i evsz=%i\n",
					bytes, sizeof(mcon->event));
                	clog_printf(0, mcon->log,
                        	"------------------------------------------\n");
		}
	}

	return NULL;
}

call_signal_event_t *call_signal_connection_readp(call_signal_connection_t *mcon, int iteration)
{
	unsigned int fromlen = sizeof(struct sockaddr_in);
#if 0
	call_signal_event_t *event = &mcon->event;
#endif
	int bytes = 0;

	bytes = recvfrom(mcon->socket, &mcon->event, sizeof(mcon->event), MSG_DONTWAIT, 
			(struct sockaddr *) &mcon->local_addr, &fromlen);

	if (bytes == sizeof(mcon->event) || 
            bytes == (sizeof(mcon->event)-sizeof(uint32_t))) {

#if 0
	/* Debugging only not to be used in production because span/chan can be invalid */
               if (mcon->event.span < 0 || mcon->event.chan < 0 || mcon->event.span > 16 || mcon->event.chan > 31) {
                        clog_printf(0, mcon->log,
                                "------------------------------------------\n");
                        clog_printf(0, mcon->log,
                                "Critical Error: RXp Cmd=%s Invalid Span=%i Chan=%i\n",
                                call_signal_event_id_name(event->event_id), event->span,event->chan);
                        clog_printf(0, mcon->log,
                                "------------------------------------------\n");

                        errno=EAGAIN;
                        return NULL;
                }
#endif

		return &mcon->event;
	} else {
		if (iteration == 0) {
                	clog_printf(0, mcon->log,
                        	"------------------------------------------\n");
                	clog_printf(0, mcon->log,
                        	"Critical Error: PQ Invalid Event lenght from boost rxlen=%i evsz=%i\n",
					bytes, sizeof(mcon->event));
                	clog_printf(0, mcon->log,
                        	"------------------------------------------\n");
		}
	}

	return NULL;
}


int call_signal_connection_write(call_signal_connection_t *mcon, call_signal_event_t *event)
{
	int err;
	if (!event) {
		clog_printf(0, mcon->log, "Critical Error: No Event Device\n");
		return -EINVAL;
	}

	if (event->span < 0 || event->chan < 0 || event->span > 16 || event->chan > 31) {
		clog_printf(0, mcon->log, 
			"------------------------------------------\n");
		clog_printf(0, mcon->log, 
			"Critical Error: TX Cmd=%s Invalid Span=%i Chan=%i\n",
			call_signal_event_id_name(event->event_id), event->span,event->chan);
		clog_printf(0, mcon->log, 
			"------------------------------------------\n");

		return -1;
	}

	gettimeofday(&event->tv,NULL);
	
	pthread_mutex_lock(&mcon->lock);
	event->fseqno=mcon->txseq++;
	event->bseqno=mcon->rxseq;
	err=sendto(mcon->socket, event, sizeof(call_signal_event_t), 0, 
		   (struct sockaddr *) &mcon->remote_addr, sizeof(mcon->remote_addr));
	pthread_mutex_unlock(&mcon->lock);

	if (err != sizeof(call_signal_event_t)) {
		err = -1;
	}
	
#if 0
	clog_printf(2, mcon->log, "TX EVENT\n");
	clog_printf(2, mcon->log, "===================================\n");
	clog_printf(2, mcon->log, "       tType: %s (%0x HEX)\n",
				call_signal_event_id_name(event->event_id),event->event_id);
	clog_printf(2, mcon->log, "       tSpan: [%d]\n",event->span+1);
	clog_printf(2, mcon->log, "       tChan: [%d]\n",event->chan+1);
	clog_printf(2, mcon->log, "  tCalledNum: %s\n",
			(event->called_number_digits_count ? (char *) event->called_number_digits : "N/A"));
	clog_printf(2, mcon->log, " tCallingNum: %s\n",
			(event->calling_number_digits_count ? (char *) event->calling_number_digits : "N/A"));
	clog_printf(2, mcon->log, "      tCause: %d\n",event->release_cause);
	clog_printf(2, mcon->log, "  tInterface: [w%dg%d]\n",event->span+1,event->chan+1);
	clog_printf(2, mcon->log, "   tEvent ID: [%d]\n",event->event_id);
	clog_printf(2, mcon->log, "   tSetup ID: [%d]\n",event->call_setup_id);
	clog_printf(2, mcon->log, "        tSeq: [%d]\n",event->fseqno);
	clog_printf(2, mcon->log, "===================================\n");
#endif

#if 1
 	clog_printf(2, mcon->log,
                           "TX EVENT: %s:(%X) [w%dg%d] Rc=%i CSid=%i Seq=%i Cd=[%s] Ci=[%s]\n",
                           call_signal_event_id_name(event->event_id),
                           event->event_id,
                           event->span+1,
                           event->chan+1,
                           event->release_cause,
                           event->call_setup_id,
                           event->fseqno,
                           (event->called_number_digits_count ? (char *) event->called_number_digits : "N/A"),
                           (event->calling_number_digits_count ? (char *) event->calling_number_digits : "N/A")
                           );
#endif


#if 0

	clog_printf(2, mcon->log,
                           "\nTX EVENT\n"
                           "===================================\n"
                           "           tType: %s (%0x HEX)\n"
                           "           tSpan: [%d]\n"
                           "           tChan: [%d]\n"
                           "  tCalledNum: %s\n"
                           " tCallingNum: %s\n"
                           "      tCause: %d\n"
                           "  tInterface: [w%dg%d]\n"
                           "   tEvent ID: [%d]\n"
                           "   tSetup ID: [%d]\n"
                           "        tSeq: [%d]\n"
                           "===================================\n"
                           "\n",
                           call_signal_event_id_name(event->event_id),
                           event->event_id,
                           event->span+1,
                           event->chan+1,
                           (event->called_number_digits_count ? (char *) event->called_number_digits : "N/A"),
                           (event->calling_number_digits_count ? (char *) event->calling_number_digits : "N/A"),
                           event->release_cause,
                           event->span+1,
                           event->chan+1,
                           event->event_id,
                           event->call_setup_id,
                           event->fseqno
                           );
#endif


	return err;
}

void call_signal_call_init(call_signal_event_t *event, char *calling, char *called, int setup_id)
{
	memset(event, 0, sizeof(call_signal_event_t));
	event->event_id = SIGBOOST_EVENT_CALL_START;

	if (calling) {
		strncpy((char*)event->calling_number_digits, calling, sizeof(event->calling_number_digits)-1);
		event->calling_number_digits_count = strlen(calling);
	}

	if (called) {
		strncpy((char*)event->called_number_digits, called, sizeof(event->called_number_digits)-1);
		event->called_number_digits_count = strlen(called);
	}
		
	event->call_setup_id = setup_id;
	
}

void call_signal_event_init(call_signal_event_t *event, call_signal_event_id_t event_id, int chan, int span) 
{
	memset(event, 0, sizeof(call_signal_event_t));
	event->event_id = event_id;
	event->chan = chan;
	event->span = span;
}

char *call_signal_event_id_name(uint32_t event_id)
{
	int x;
	char *ret = NULL;

	for (x = 0 ; x < sizeof(call_signal_table)/sizeof(struct call_signal_map); x++) {
		if (call_signal_table[x].event_id == event_id) {
			ret = call_signal_table[x].name;
			break;
		}
	}

	return ret;
}
