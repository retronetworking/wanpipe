/*****************************************************************************
 * call_signal.h -- Signal Specifics
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

#ifndef _CALL_SIGNAL_H
#define _CALL_SIGNAL_H

#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <netdb.h>
#include <sigboost.h>
#include <pthread.h>
#include <sys/time.h>




#define call_signal_test_flag(p,flag) 		({ \
					((p)->flags & (flag)); \
					})

#define call_signal_set_flag(p,flag) 		do { \
					((p)->flags |= (flag)); \
					} while (0)

#define call_signal_clear_flag(p,flag) 		do { \
					((p)->flags &= ~(flag)); \
					} while (0)

#define call_signal_copy_flags(dest,src,flagz)	do { \
					(dest)->flags &= ~(flagz); \
					(dest)->flags |= ((src)->flags & (flagz)); \
					} while (0)

typedef  t_sigboost call_signal_event_t;
typedef uint32_t call_signal_event_id_t;

struct call_signal_connection {
	int socket;
	struct sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	call_signal_event_t event;
	struct hostent remote_hp;
	struct hostent local_hp;
	unsigned int flags;
	pthread_mutex_t lock;
	FILE *log;
};

typedef enum {
	MSU_FLAG_EVENT = (1 << 0)
} call_signal_flag_t;

typedef struct call_signal_connection call_signal_connection_t;

int call_signal_connection_close(call_signal_connection_t *mcon);
int call_signal_connection_open(call_signal_connection_t *mcon, char *local_ip, int local_port, char *ip, int port);
call_signal_event_t *call_signal_connection_read(call_signal_connection_t *mcon, int iteration);
int call_signal_connection_write(call_signal_connection_t *mcon, call_signal_event_t *event);
void call_signal_event_init(call_signal_event_t *event, call_signal_event_id_t event_id, int chan, int span);
void call_signal_call_init(call_signal_event_t *event, char *calling, char *called, int setup_id);
char *call_signal_event_id_name(uint32_t event_id);

#endif


