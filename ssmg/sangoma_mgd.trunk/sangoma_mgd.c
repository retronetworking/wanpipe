/*********************************************************************************
 * sangoma_mgd.c --  Sangoma Media Gateway Daemon for Sangoma/Wanpipe Cards
 *
 * Copyright 05-08, Nenad Corbic <ncorbic@sangoma.com>
 *		    Anthony Minessale II <anthmct@yahoo.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License
 * 
 * =============================================
 * v1.31 Nenad Corbic  <ncorbic@sangoma.com>
 * Mar 14 2008
 *	Moved in use error to verbose level 3
 *      When using SMG agains 2 Asterisk Boxes
 *      Not a functional update.
 *
 * v1.30 Nenad Corbic  <ncorbic@sangoma.com>
 * Feb 08 2008
 *	The fix in v1.26 causes double stop.
 *      I took out the v1.26 fix and added
 *      a big warning if that condition is
 *      ever reached.
 *
 * v1.29 Nenad Corbic  <ncorbic@sangoma.com>
 * Feb 07 2008
 *	Added strip_cid_non_digits option
 *
 * v1.28 Nenad Corbic  <ncorbic@sangoma.com>
 * Feb 06 2008
 * 	Fixed a memory leak in clone event
 *      function. Added memory check subsystem  
 *
 * v1.27 Nenad Corbic  <ncorbic@sangoma.com>
 * Jan 24 2008
 *	Fixed a memory leak on incoming calls
 *	Removed the use of server listener which 
 *	was not used
 *
 * v1.26 Nenad Corbic  <ncorbic@sangoma.com>
 * Jan 18 2008
 *	Fixed hangup after invalid Answer or Ack Session 
 *	Can cause double use of setup id - now fixed
 *	Update on autoacm on accept check for acked.
 *
 * v1.25 Nenad Corbic  <ncorbic@sangoma.com>
 * Dec 31 2007
 *	Removed UDP Resync it can cause skb_over errors.
 * 	Moved RDNIS message to higher debug level
 *
 * v1.24 Nenad Corbic  <ncorbic@sangoma.com>
 * Nov 30 2007
 *	Bug fix on return code on ALL ckt busy
 *
 * v1.23 Nenad Corbic  <ncorbic@sangoma.com>
 *	Bug fix on socket open. Check for retun code >= 0
 *
 * v1.22 Nenad Corbic  <ncorbic@sangoma.com>
 * Nov 27 2007
 *	Updated DTMF Tx function
 *      Fixed - dtmf tx without voice
 *      Fxied - dtmf clipping.
 *
 * v1.21 Nenad Corbic  <ncorbic@sangoma.com>
 * Nov 25 2007
 *	Major unit testing of each state
 *      Numerous bug fixes for non autoacm mode.
 *      Changed "Channel-Name" to tg/cic
 *      Added compile option WANPIPE_CHAN_NAME to change Asterisk channel
 *      name of chan_woomera.so.  So one can use Dial(SS7/g1/${EXTE})
 *      instead of WOOMERA (for example) 
 *
 * v1.20 Nenad Corbic <ncorbic@sangoma.com>
 *	Added option for Auto ACM response mode.
 *
 * v1.19 Nenad Corbic <ncorbic@sangoma.com>
 *	Configurable DTMF
 *      Bug fix in release codes (all ckt busy)
 *
 * v1.18 Nenad Corbic <ncorbic@sangoma.com>
 *	Added new rel cause support based on
 *	digits instead of strings.
 *
 * v1.17 Nenad Corbic <ncorbic@sangoma.com>
 *	Added session support
 *
 * v1.16 Nenad Corbic <ncorbic@sangoma.com>
 *      Added hwec disable on loop ccr 
 *
 * v1.15 Nenad Corbic <ncorbic@sangoma.com>
 *      Updated DTMF Locking 
 *	Added delay between digits
 *
 * v1.14 Nenad Corbic <ncorbic@sangoma.com>
 *	Updated DTMF Library
 *	Fixed DTMF synchronization
 *
 * v1.13 Nenad Corbic <ncorbic@sangoma.com>
 *	Woomera OPAL Dialect
 *	Added Congestion control
 *	Added SCTP
 *	Added priority ISUP queue
 *	Fixed presentation
 *
 * v1.12 Nenad Corbic <ncorbic@sangoma.com>
 *	Fixed CCR 
 *	Removed socket shutdown on end call.
 *	Let Media thread shutodwn sockets.
 *
 * v1.11 Nenad Corbic <ncorbic@sangoma.com>
 * 	Fixed Remote asterisk/woomera connection
 *	Increased socket timeouts
 *
 * v1.10 Nenad Corbic <ncorbic@sangoma.com>
 *	Added Woomera OPAL dialect.
 *	Start montor thread in priority
 *
 * v1.9 Nenad Corbic <ncorbic@sangoma.com>
 * 	Added Loop mode for ccr
 *	Added remote debug enable
 *	Fixed syslog logging.
 *
 * v1.8 Nenad Corbic <ncorbic@sangoma.com>
 *	Added a ccr loop mode for each channel.
 *	Boost can set any channel in loop mode
 *
 * v1.7 Nenad Corbic <ncorbic@sangoma.com>
 *      Pass trunk group number to incoming call
 *      chan woomera will use it to append to context
 *      name. Added presentation feature.
 *
 * v1.6	Nenad Corbic <ncorbic@sangoma.com>
 *      Use only ALAW and MLAW not SLIN.
 *      This reduces the load quite a bit.
 *      Send out ALAW/ULAW format on HELLO message.
 *      RxTx Gain is done now in chan_woomera.
 *
 * v1.5	Nenad Corbic <ncorbic@sangoma.com>
 * 	Bug fix in START_NACK_ACK handling.
 * 	When we receive START_NACK we must alwasy pull tank before
 *      we send out NACK_ACK this way we will not try to send NACK 
 *      ourself.
 *********************************************************************************/

#include "sangoma_mgd.h"
#include "sangoma_mgd_memdbg.h"
#include "q931_cause.h"


#define USE_SYSLOG 1
#define CODEC_LAW_DEFAULT 1

#ifdef CODEC_LAW_DEFAULT
static uint32_t codec_sample=8;
#else
static uint32_t codec_sample=16;
#endif

static char ps_progname[]="sangoma_mgd";

static struct woomera_interface woomera_dead_dev;

#if 0
#define DOTRACE
#endif


#define SMG_VERSION	"v1.31"

/* enable early media */
#if 1
#define WOOMERA_EARLY_MEDIA 1
#endif


#define SMG_DTMF_ON 	60
#define SMG_DTMF_OFF 	10
#define SMG_DTMF_RATE  	8000

#if 0
#define MEDIA_SOCK_SHUTDOWN 1	
#endif

#ifdef DOTRACE
static int tc = 0;
#endif

#if 0
#warning "NENAD: HPTDM API"
#define WP_HPTDM_API 1
#else
#undef WP_HPTDM_API 
#endif

#ifdef  WP_HPTDM_API
hp_tdm_api_span_t *hptdmspan[WOOMERA_MAX_SPAN];
#endif

#undef SMG_CALLING_NAME

const char WELCOME_TEXT[] =
"================================================================================\n"
"Sangoma Media Gateway Daemon v1.31 \n"
"TDM Signal Media Gateway for Sangoma/Wanpipe Cards\n"
"Copyright 2005, 2006, 2007 \n"
"Nenad Corbic <ncorbic@sangoma.com>, Anthony Minessale II <anthmct@yahoo.com>\n"
"This program is free software, distributed under the terms of\n"
"the GNU General Public License\n"
"================================================================================\n"
"";

					
static int master_reset=0;

static int coredump=1;
static int autoacm=1;

static int launch_media_tdm_thread(struct woomera_interface *woomera);
static int launch_woomera_thread(struct woomera_interface *woomera);
static struct woomera_interface *alloc_woomera(void);

q931_cause_to_str_array_t q931_cause_to_str_array[255];


#if 0
static uint32_t string_to_release(char *code)
{
	if (code) {
		if (!strcasecmp(code, "CHANUNAVAIL")) {
			return SIGBOOST_RELEASE_CAUSE_NOANSWER;
		}

		if (!strcasecmp(code, "INVALID")) {
			return SIGBOOST_RELEASE_CAUSE_CALLED_NOT_EXIST;
		}

		if (!strcasecmp(code, "ERROR")) {
			return SIGBOOST_RELEASE_CAUSE_UNDEFINED;
		}

		if (!strcasecmp(code, "CONGESTION")) {
			//return SIGBOOST_RELEASE_CAUSE_BUSY;
			return SIGBOOST_RELEASE_CAUSE_CALLED_NOT_EXIST;
		}

		if (!strcasecmp(code, "BUSY")) {
			return SIGBOOST_RELEASE_CAUSE_BUSY;
		}

		if (!strcasecmp(code, "NOANSWER")) {
			return SIGBOOST_RELEASE_CAUSE_NOANSWER;
		}

		if (!strcasecmp(code, "ANSWER")) {
			return SIGBOOST_RELEASE_CAUSE_NORMAL;
		}

		if (!strcasecmp(code, "CANCEL")) {
			return SIGBOOST_RELEASE_CAUSE_BUSY;
		}

		if (!strcasecmp(code, "UNKNOWN")) {
			return SIGBOOST_RELEASE_CAUSE_UNDEFINED;
		} 

	}
	return SIGBOOST_RELEASE_CAUSE_NORMAL;
}

static char * release_to_string(uint32_t rel_cause)
{
	switch (rel_cause) {
	
		case SIGBOOST_RELEASE_CAUSE_UNDEFINED:
			return  "UNKNOWN";
		case SIGBOOST_RELEASE_CAUSE_NORMAL:
			return  "NORMAL";
		case SIGBOOST_RELEASE_CAUSE_BUSY:
			return  "BUSY";
		case SIGBOOST_RELEASE_CAUSE_CALLED_NOT_EXIST:
			return  "CHANUNAVAIL";
		case SIGBOOST_RELEASE_CAUSE_CIRCUIT_RESET:
			return  "CANCEL";
		case SIGBOOST_RELEASE_CAUSE_NOANSWER:
			return "NOANSWER";
		case SIGBOOST_CALL_SETUP_NACK_CKT_START_TIMEOUT:
			return "TIMEOUT";
		case SIGBOOST_CALL_SETUP_NACK_ALL_CKTS_BUSY:
			return "CONGESTION";
		case SIGBOOST_CALL_SETUP_NACK_CALLED_NUM_TOO_BIG:
			return "ERROR";
		case SIGBOOST_CALL_SETUP_NACK_CALLING_NUM_TOO_BIG:
			return "ERROR";
		case SIGBOOST_CALL_SETUP_NACK_CALLED_NUM_TOO_SMALL:
			return "ERROR";
		case SIGBOOST_CALL_SETUP_NACK_CALLING_NUM_TOO_SMALL:
			return "ERROR";
	}		
	
	return "NORMAL";
}
#endif



void __log_printf(int level, FILE *fp, char *file, const char *func, int line, char *fmt, ...)
{
    char *data;
    int ret = 0;
    va_list ap;

    if (socket < 0) {
		return;
    }

    if (fp == NULL) {
 	fp = server.log;
    } 

    if (level && level > server.debug) {
		return;
    }

    va_start(ap, fmt);
#ifdef SOLARIS
    data = (char *) smg_malloc(2048);
    vsnprintf(data, 2048, fmt, ap);
#else
    ret = vasprintf(&data, fmt, ap);
#endif
    va_end(ap);
    if (ret == -1) {
		fprintf(stderr, "Memory Error\n");
    } else {
		char date[80] = "";
		struct tm now;
		time_t epoch;

		if (time(&epoch) && localtime_r(&epoch, &now)) {
			strftime(date, sizeof(date), "%Y-%m-%d %T", &now);
		}

#ifdef USE_SYSLOG 
		syslog(LOG_DEBUG | LOG_LOCAL2, data);
#else
		fprintf(fp, "[%d] %s %s:%d %s() %s", getpid(), date, file, line, func, data);
#endif
		free(data);
    }
#ifndef USE_SYSLOG
    fflush(fp);
#endif
}




static int isup_exec_command(int span, int chan, int id, int cmd, int cause)
{
	call_signal_event_t oevent;
	int retry=5;
	
	call_signal_event_init(&oevent, cmd, chan, span);
	oevent.release_cause = cause;
	
	if (id >= 0) {
		oevent.call_setup_id = id;
	}
isup_exec_cmd_retry:
	if (call_signal_connection_write(&server.mcon, &oevent) <= 0){

		--retry;
		if (retry <= 0) {
			log_printf(0, server.log, 
			"Critical System Error: Failed to tx on ISUP socket: %s\n", 
				strerror(errno));
			return -1;
		} else {
			log_printf(0, server.log,
                        "System Warning: Failed to tx on ISUP socket: %s :retry %i\n",
                                strerror(errno),retry);
		}

		goto isup_exec_cmd_retry; 
	}
	
	return 0;
}

static int socket_printf(int socket, char *fmt, ...)
{
    char *data;
    int ret = 0;
    va_list ap;

    if (socket < 0) {
		return -1;
    }
	
    va_start(ap, fmt);
#ifdef SOLARIS
    data = (char *) smg_malloc(2048);
    vsnprintf(data, 2048, fmt, ap);
#else
    ret = vasprintf(&data, fmt, ap);
#endif
    va_end(ap);
    if (ret == -1) {
		fprintf(stderr, "Memory Error\n");
		log_printf(0, server.log, "Crtical ERROR: Memory Error!\n");
    } else {
		int err;
		int len = strlen(data);
		err=send(socket, data, strlen(data), 0);
		if (err != strlen(data)) {
			log_printf(2, server.log, "ERROR: Failed to send data to woomera socket(%i): err=%i  len=%d %s\n",
					   socket,err,len,strerror(errno));
			ret = err;
		} else {
			ret = 0;
		}
		
		free(data);
    }

	return ret;
}



static int woomera_next_pair(struct woomera_config *cfg, char **var, char **val)
{
    int ret = 0;
    char *p, *end;

    *var = *val = NULL;
	
    for(;;) {
		cfg->lineno++;

		if (!fgets(cfg->buf, sizeof(cfg->buf), cfg->file)) {
			ret = 0;
			break;
		}
		
		*var = cfg->buf;

		if (**var == '[' && (end = strchr(*var, ']'))) {
			*end = '\0';
			(*var)++;
			strncpy(cfg->category, *var, sizeof(cfg->category) - 1);
			continue;
		}
		
		if (**var == '#' || **var == '\n' || **var == '\r') {
			continue;
		}

		if ((end = strchr(*var, '#'))) {
			*end = '\0';
			end--;
		} else if ((end = strchr(*var, '\n'))) {
			if (*end - 1 == '\r') {
				end--;
			}
			*end = '\0';
		}
	
		p = *var;
		while ((*p == ' ' || *p == '\t') && p != end) {
			*p = '\0';
			p++;
		}
		*var = p;

		if (!(*val = strchr(*var, '='))) {
			ret = -1;
			log_printf(0, server.log, "Invalid syntax on %s: line %d\n", cfg->path, cfg->lineno);
			continue;
		} else {
			p = *val - 1;
			*(*val) = '\0';
			(*val)++;
			if (*(*val) == '>') {
				*(*val) = '\0';
				(*val)++;
			}

			while ((*p == ' ' || *p == '\t') && p != *var) {
				*p = '\0';
				p--;
			}

			p = *val;
			while ((*p == ' ' || *p == '\t') && p != end) {
				*p = '\0';
				p++;
			}
			*val = p;
			ret = 1;
			break;
		}
    }

    return ret;

}


#if 0
static void woomera_set_span_chan(struct woomera_interface *woomera, int span, int chan)
{
    pthread_mutex_lock(&woomera->vlock);
    woomera->span = span;
    woomera->chan = chan;
    pthread_mutex_unlock(&woomera->vlock);

}
#endif


static struct woomera_event *new_woomera_event_printf(struct woomera_event *ebuf, char *fmt, ...)
{
    struct woomera_event *event = NULL;
    int ret = 0;
    va_list ap;

    if (ebuf) {
		event = ebuf;
    } else if (!(event = new_woomera_event())) {
		log_printf(0, server.log, "Memory Error queuing event!\n");
		return NULL;
    } else {
    		return NULL;
    }

    va_start(ap, fmt);
#ifdef SOLARIS
    event->data = (char *) smg_malloc(2048);
    vsnprintf(event->data, 2048, fmt, ap);
#else
    ret = smg_vasprintf(&event->data, fmt, ap);
#endif
    va_end(ap);
    if (ret == -1) {
		log_printf(0, server.log, "Memory Error queuing event!\n");
		destroy_woomera_event(&event, EVENT_FREE_DATA);
		return NULL;
    }

    return event;
	
}

static struct woomera_event *woomera_clone_event(struct woomera_event *event)
{
    struct woomera_event *clone;

    if (!(clone = new_woomera_event())) {
		return NULL;
    }

    /* This overwrites the MALLOC in clone causing a memory leak */	
    memcpy(clone, event, sizeof(*event));

    /* We must set the malloc flag back so that this event
     * will be deleted */
    _woomera_set_flag(clone, WFLAG_MALLOC);
    clone->next = NULL;
    clone->data = smg_strdup(event->data);

    return clone;
}

static void enqueue_event(struct woomera_interface *woomera, 
                          struct woomera_event *event, 
			  event_args free_data)
{
    struct woomera_event *ptr, *clone = NULL;
	
    assert(woomera != NULL);
    assert(event != NULL);

    if (!(clone = woomera_clone_event(event))) {
		log_printf(0, server.log, "Error Cloning Event\n");
		return;
    }

    pthread_mutex_lock(&woomera->queue_lock);
	
    for (ptr = woomera->event_queue; ptr && ptr->next ; ptr = ptr->next);
	
    if (ptr) {
	ptr->next = clone;
    } else {
	woomera->event_queue = clone;
    }

    pthread_mutex_unlock(&woomera->queue_lock);

    woomera_set_flag(woomera, WFLAG_EVENT);
    
    if (free_data && event->data) {
    	/* The event has been duplicated, the original data
	 * should be freed */
    	smg_free(event->data);	
	event->data=NULL;
    }
}

static char *dequeue_event(struct woomera_interface *woomera) 
{
    struct woomera_event *event;
    char *data = NULL;

    if (!woomera) {
		return NULL;
    }
	
    pthread_mutex_lock(&woomera->queue_lock);
    if (woomera->event_queue) {
		event = woomera->event_queue;
		woomera->event_queue = event->next;
		data = event->data;
		pthread_mutex_unlock(&woomera->queue_lock);
	
		destroy_woomera_event(&event, EVENT_KEEP_DATA);
		return data;
    }
    pthread_mutex_unlock(&woomera->queue_lock);

    return data;
}


static int enqueue_event_on_listeners(struct woomera_event *event) 
{
    struct woomera_listener *ptr;
    int x = 0;

    assert(event != NULL);

    pthread_mutex_lock(&server.listen_lock);
    for (ptr = server.listeners ; ptr ; ptr = ptr->next) {
		enqueue_event(ptr->woomera, event, EVENT_KEEP_DATA);
		x++;
    }
    pthread_mutex_unlock(&server.listen_lock);
	
    return x;
}


static void del_listener(struct woomera_interface *woomera) 
{
    struct woomera_listener *ptr, *last = NULL;

    pthread_mutex_lock(&server.listen_lock);
    for (ptr = server.listeners ; ptr ; ptr = ptr->next) {
		if (ptr->woomera == woomera) {
			if (last) {
				last->next = ptr->next;
			} else {
				server.listeners = ptr->next;
			}
			smg_free(ptr);
			break;
		}
		last = ptr;
    }
    pthread_mutex_unlock(&server.listen_lock);
}

static void add_listener(struct woomera_interface *woomera) 
{
    struct woomera_listener *new;

    pthread_mutex_lock(&server.listen_lock);
	
    if ((new = smg_malloc(sizeof(*new)))) {
		memset(new, 0, sizeof(*new));
		new->woomera = woomera;
		new->next = server.listeners;
		server.listeners = new;
    } else {
		log_printf(0, server.log, "Memory Error adding listener!\n");
    }

    pthread_mutex_unlock(&server.listen_lock);
}



static int wanpipe_send_dtmf(struct woomera_interface *woomera, char *digits)
{	
	struct media_session *ms = woomera_get_ms(woomera);
	char *cur = NULL;
	int wrote = 0;
	int err;
	
	if (!ms) {
		return -EINVAL;
	}
	
	if (!ms->dtmf_buffer) {
		log_printf(3, woomera->log, "Allocate DTMF Buffer....");

		err=switch_buffer_create_dynamic(&ms->dtmf_buffer, 1024, server.dtmf_size, 0);

		if (err != 0) {
			log_printf(0, woomera->log, "Failed to allocate DTMF Buffer!\n");
			return -ENOMEM;
		} else {
			log_printf(3, woomera->log, "SUCCESS!\n");
		}
		
	}
	
	log_printf(3, woomera->log, "Sending DTMF %s\n",digits);
	for (cur = digits; *cur; cur++) {
		if ((wrote = teletone_mux_tones(&ms->tone_session,
					        &ms->tone_session.TONES[(int)*cur]))) {
			
			pthread_mutex_lock(&woomera->dtmf_lock);

			err=switch_buffer_write(ms->dtmf_buffer, ms->tone_session.buffer, wrote * 2);

			pthread_mutex_unlock(&woomera->dtmf_lock);

			log_printf(3, woomera->log, "Sending DTMF %s Wrote=%i (err=%i)\n",
					digits,wrote*2,err);
		} else {
			log_printf(0, woomera->log, "Error: Sending DTMF %s (err=%i)\n",
                                        digits,wrote);
		}
	}

	ms->skip_read_frames = 200;
	return 0;
}

static struct woomera_interface *alloc_woomera(void)
{
	struct woomera_interface *woomera = NULL;

	if ((woomera = smg_malloc(sizeof(struct woomera_interface)))) {

		memset(woomera, 0, sizeof(struct woomera_interface));
		
		woomera->chan = -1;
		woomera->span = -1;
		woomera->log = server.log;
		woomera->debug = server.debug;
		woomera->call_id = 1;
		woomera->event_queue = NULL;
		woomera->q931_rel_cause_topbx=SIGBOOST_RELEASE_CAUSE_NORMAL;
		woomera->q931_rel_cause_tosig=SIGBOOST_RELEASE_CAUSE_NORMAL;
    
		woomera_set_interface(woomera, "w-1g-1");

		

	}

	return woomera;

}


static struct woomera_interface *new_woomera_interface(int socket, struct sockaddr_in *sock_addr, int len) 
{
    	struct woomera_interface *woomera = NULL;
	
    	if (socket < 0) {
		log_printf(0, server.log, "Critical: Invalid Socket on new interface!\n");
		return NULL;
    	}

    	if ((woomera = alloc_woomera())) {
		if (socket >= 0) {
			no_nagle(socket);
			woomera->socket = socket;
		}

		if (sock_addr && len) {
			memcpy(&woomera->addr, sock_addr, len);
		}
	}

	return woomera;

}

static char *woomera_message_header(struct woomera_message *wmsg, char *key) 
{
    int x = 0;
    char *value = NULL;

    for (x = 0 ; x < wmsg->last ; x++) {
		if (!strcasecmp(wmsg->names[x], key)) {
			value = wmsg->values[x];
			break;
		}
    }

    return value;
}


#if 1

static int waitfor_socket(int fd, int timeout, int flags)
{
    struct pollfd pfds[1];
    int res;
 
    memset(&pfds[0], 0, sizeof(pfds[0]));
    pfds[0].fd = fd;
    pfds[0].events = flags;
    res = poll(pfds, 1, timeout);

    if (res > 0) {
	if(pfds[0].revents & POLLIN) {
		res = 1;
	} else if ((pfds[0].revents & POLLERR)) {
		res = -1;
	} else if ((pfds[0].revents & POLLNVAL)) {
		res = -2;
#if 0
		log_printf(0, server.log,"System Warning: Poll Event NVAL (0x%X) (fd=%i)!\n",
				pfds[0].revents, fd);
#endif
    	} else {
#if 0
		log_printf(0, server.log,"System Error: Poll Event Error no event (0x%X) (fd=%i)!\n",
				pfds[0].revents,fd);
#endif
		res = -1;
	}
    }

    return res;
}


static int waitfor_tx_socket(int fd, int timeout, int flags)
{
    struct pollfd pfds[1];
    int res;
 
    memset(&pfds[0], 0, sizeof(pfds[0]));
    pfds[0].fd = fd;
    pfds[0].events = flags;
    res = poll(pfds, 1, timeout);

    if (res > 0) {
	if (pfds[0].revents & POLLOUT) {
		res = 1;
	} else if ((pfds[0].revents & POLLERR)) {
		res = -1;
	} else if ((pfds[0].revents & POLLNVAL)) {
		res = -2;
#if 0
		log_printf(0, server.log,"System Warning: Poll Event NVAL (0x%X) (fd=%i)!\n",
				pfds[0].revents, fd);
#endif
    	} else {
#if 0
		log_printf(0, server.log,"System Error: Poll Event Error no event (0x%X) (fd=%i)!\n",
				pfds[0].revents,fd);
#endif
		res = -1;
	}
    }

    return res;
}


#else

static int waitfor_socket(int fd, int timeout, int flags)
{
    struct pollfd pfds[1];
    int res;
    int errflags = (POLLERR | POLLHUP | POLLNVAL);

    memset(&pfds[0], 0, sizeof(pfds[0]));
    pfds[0].fd = fd;
    pfds[0].events = flags | errflags;
    res = poll(pfds, 1, timeout);

    if (res > 0) {
	if(pfds[0].revents & POLLIN) {
		res = 1;
	} else if ((pfds[0].revents & errflags)) {
		res = -1;
    	} else {
#if 0
		log_printf(0, server.log,"System Error: Poll Event Error no event (0x%X)!\n",
				pfds[0].revents);
#endif
		res = -1;
	}
    }

    return res;
}

#endif

#if 1
static int waitfor_2sockets(int fda, int fdb, char *a, char *b, int timeout) 
{
    struct pollfd pfds[2];
    int res = 0;
    int errflags = (POLLERR | POLLHUP | POLLNVAL);

    if (fda < 0 || fdb < 0) {
		return -1;
    }

    *a=0;
    *b=0;

    memset(pfds, 0, sizeof(pfds));
    pfds[0].fd = fda;
    pfds[1].fd = fdb;
    pfds[0].events = POLLIN | errflags;
    pfds[1].events = POLLIN | errflags;
    if ((res = poll(pfds, 2, timeout)) > 0) {
		res = 1;
		if ((pfds[0].revents & errflags) || (pfds[1].revents & errflags)) {
			res = -1;
		} else { 
			if ((pfds[0].revents & POLLIN)) {
				*a=1;
				res++;
			}
			if ((pfds[1].revents & POLLIN)) {
				*b=1;		
				res++;
			}
		}

		if (res == 1) {
			/* No event found what to do */
			res=-1;
		}
    }
	
    return res;
}
#endif


static struct media_session *media_session_new(struct woomera_interface *woomera)
{
    	struct media_session *ms = NULL;
    	int x;
    	char *p;
    	int span,chan;
	
	span=woomera->span;
	chan=woomera->chan;

    	log_printf(2, server.log,"Starting new MEDIA session [%s] [%s]\n",
				woomera->interface,woomera->raw?woomera->raw:"N/A");

    	if ((ms = smg_malloc(sizeof(struct media_session)))) {
		memset(ms, 0, sizeof(struct media_session));
		
		if (woomera->loop_tdm != 1) {
			for(x = 0; x < strlen(woomera->raw) ; x++) {
				if (woomera->raw[x] == ':') {
					break;
				}
				if (woomera->raw[x] == '/') {
					break;
				}
			}
			
			ms->ip = smg_strndup(woomera->raw, x);
			time(&ms->started);
			p = woomera->raw + (x+1);
			ms->port = atoi(p);
		} 
		
		time(&ms->started);
		woomera_set_ms(woomera,ms);
		ms->woomera = woomera;
		
		/* Setup artificial DTMF stuff */
		memset(&ms->tone_session, 0, sizeof(ms->tone_session));
		if (teletone_init_session(&ms->tone_session, 0, NULL, NULL)) {
			log_printf(0, server.log, "ERROR: Failed to initialize TONE [w%ig%i]!\n",
			span+1,chan+1);			
		}

		ms->tone_session.rate = SMG_DTMF_RATE;
		ms->tone_session.duration = server.dtmf_on * (ms->tone_session.rate / 1000);
		ms->tone_session.wait = server.dtmf_off * (ms->tone_session.rate / 1000);
	
		teletone_dtmf_detect_init (&ms->dtmf_detect, SMG_DTMF_RATE);
			
    	} else {
		log_printf(0, server.log, "ERROR: Memory Alloc Failed [w%ig%i]!\n",
			span+1,chan+1);	
	}

   	return ms;
}

static void media_session_free(struct media_session *ms) 
{
    if (ms->ip) {
		smg_free(ms->ip);
    }
    
    teletone_destroy_session(&ms->tone_session);
    switch_buffer_destroy(&ms->dtmf_buffer);

    ms->woomera = NULL;

    smg_free(ms);
}


static int create_udp_socket(struct media_session *ms, char *local_ip, int local_port, char *ip, int port)
{
    int rc;
    struct hostent *result, *local_result;
    char buf[512], local_buf[512];
    int err = 0;

    log_printf(5,server.log,"LocalIP %s:%d IP %s:%d \n",local_ip, local_port, ip, port);

    memset(&ms->remote_hp, 0, sizeof(ms->remote_hp));
    memset(&ms->local_hp, 0, sizeof(ms->local_hp));
    if ((ms->socket = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
		gethostbyname_r(ip, &ms->remote_hp, buf, sizeof(buf), &result, &err);
		gethostbyname_r(local_ip, &ms->local_hp, local_buf, sizeof(local_buf), &local_result, &err);
		if (result && local_result) {
			ms->remote_addr.sin_family = ms->remote_hp.h_addrtype;
			memcpy((char *) &ms->remote_addr.sin_addr.s_addr, ms->remote_hp.h_addr_list[0], ms->remote_hp.h_length);
			ms->remote_addr.sin_port = htons(port);

			ms->local_addr.sin_family = ms->local_hp.h_addrtype;
			memcpy((char *) &ms->local_addr.sin_addr.s_addr, ms->local_hp.h_addr_list[0], ms->local_hp.h_length);
			ms->local_addr.sin_port = htons(local_port);
    
			rc = bind(ms->socket, (struct sockaddr *) &ms->local_addr, sizeof(ms->local_addr));
			if (rc < 0) {
				close(ms->socket);
				ms->socket = -1;
    			
				log_printf(5,server.log,
					"Failed to bind LocalIP %s:%d IP %s:%d (%s)\n",
						local_ip, local_port, ip, port,strerror(errno));
			} 

			/* OK */

		} else {
    			log_printf(0,server.log,
				"Failed to get hostbyname LocalIP %s:%d IP %s:%d (%s)\n",
					local_ip, local_port, ip, port,strerror(errno));
		}
    } else {
    	log_printf(0,server.log,
		"Failed to create/allocate UDP socket\n");
    }

    return ms->socket;
}

static int next_media_port(void)
{
    int port;
    
    pthread_mutex_lock(&server.media_udp_port_lock);
    port = ++server.next_media_port;
    if (port > WOOMERA_MAX_MEDIA_PORT) {
    		server.next_media_port = WOOMERA_MIN_MEDIA_PORT;
		port = WOOMERA_MIN_MEDIA_PORT;
    }
    pthread_mutex_unlock(&server.media_udp_port_lock);
    
    return port;
}



static int woomera_dtmf_transmit(struct media_session *ms, int mtu)
{
	struct woomera_interface *woomera = ms->woomera;
	int bread;
	unsigned char dtmf[1024];
	unsigned char dtmf_law[1024];
	sangoma_api_hdr_t hdrframe;
	int i;
	int slin_len = mtu*2;
	short *data;
	int used;
	int res;
	int err;
	int txdtmf=0;
	memset(&hdrframe,0,sizeof(hdrframe));

	if (!ms->dtmf_buffer) {
		return -1;
	}
	
	for (;;) {
		
		if ((used=switch_buffer_inuse(ms->dtmf_buffer)) <= 0) {
			break;
		}

		res = waitfor_tx_socket(ms->sangoma_sock, -1,  POLLERR | POLLOUT);
		if (res <= 0) {
			break;
		}

#ifdef CODEC_LAW_DEFAULT

		pthread_mutex_lock(&woomera->dtmf_lock);
		if ((used=switch_buffer_inuse(ms->dtmf_buffer)) <= 0) {
			pthread_mutex_unlock(&woomera->dtmf_lock);
			break;
		}

		bread = switch_buffer_read(ms->dtmf_buffer, dtmf, slin_len);
		pthread_mutex_unlock(&woomera->dtmf_lock);

		if (bread <= 0) {
			break;
		}
		
		log_printf(3,woomera->log,"%s: Write DTMF Got %d bytes MTU=%i Coding=%i Used=%i\n",
				woomera->interface,bread,mtu,ms->hw_coding,used);
		
		data=(short*)dtmf;
		for (i=0;i<mtu;i++) {
			if (ms->hw_coding) {
				/* ALAW */
				dtmf_law[i] = linear_to_alaw((int)data[i]);
			} else {
				/* ULAW */
				dtmf_law[i] = linear_to_ulaw((int)data[i]);
			}			
		}
	
		err=sangoma_sendmsg_socket(ms->sangoma_sock,
					 &hdrframe, 
					 sizeof(hdrframe), 
					 dtmf_law, mtu, 0);
				
		if (err != mtu) {
			log_printf(0, woomera->log, "Error: Failed to TX to TDM API on DTMF (err=%i mtu=%i)!\n",err,mtu);
		}

		txdtmf++;	 
		ms->skip_write_frames++;
#else
...
		pthread_mutex_lock(&woomera->dtmf_lock);		
		bread = switch_buffer_read(ms->dtmf_buffer, dtmf, mtu);
		pthread_mutex_unlock(&woomera->dtmf_lock);

		log_printf(3,woomera->log,"%s: Write DTMF Got %d bytes\n",
				woomera->interface,bread);

		sangoma_sendmsg_socket(ms->sangoma_sock,
					 &hdrframe, 
					 sizeof(hdrframe), 
					 dtmf, mtu, 0);
		txdtmf++;	 
		ms->skip_write_frames++;
#endif
		
	} 

	if (txdtmf) {
		return 0;
	} else {	
		return -1;
	}
}

static void media_loop_run(struct media_session *ms)
{
	struct woomera_interface *woomera = ms->woomera;
	int sangoma_frame_len = 160;
	int errs=0;
	int res=0;
	wanpipe_tdm_api_t tdm_api;
	unsigned char circuit_frame[1024];
	char filename[100];
	FILE *filed=NULL;
	int loops=0;
	
	sangoma_api_hdr_t hdrframe;
	memset(&hdrframe,0,sizeof(hdrframe));
	memset(circuit_frame,0,sizeof(circuit_frame));

	ms->sangoma_sock = sangoma_open_tdmapi_span_chan(woomera->span+1, woomera->chan+1);
	
	log_printf(1, server.log, "Media Loop Started %s fd=%i\n", 
			woomera->interface,ms->sangoma_sock);

	if (ms->sangoma_sock < 0) {
		log_printf(0, server.log, "WANPIPE MEDIA Socket Error (%s) if=[%s]  [w%ig%i]\n", 
			strerror(errno), woomera->interface, woomera->span+1, woomera->chan+1);
		errs++;
	} else {
			

		if (sangoma_tdm_set_codec(ms->sangoma_sock, &tdm_api, WP_NONE) < 0 ) {
			errs++;	
		}

		if (sangoma_tdm_flush_bufs(ms->sangoma_sock, &tdm_api)) {	
			errs++;
		}
			
		if (sangoma_tdm_set_usr_period(ms->sangoma_sock, &tdm_api, 20) < 0 ) {
			errs++;	
		}

		sangoma_frame_len = sangoma_tdm_get_usr_mtu_mru(ms->sangoma_sock,&tdm_api);

		sangoma_tdm_disable_hwec(ms->sangoma_sock,&tdm_api);
		
	}
	
	if (errs) {

		log_printf(0, server.log, "Media Loop: failed to open tdm device %s\n", 
					woomera->interface);
		return;
	}

	if (server.loop_trace) {
		sprintf(filename,"/smg/w%ig%i-loop.trace",woomera->span+1,woomera->chan+1);	
		unlink(filename);
		filed = safe_fopen(filename, "w");
	}
	
	while ( woomera_test_flag(&server.master_connection, WFLAG_RUNNING) &&
		!woomera_test_flag(woomera, WFLAG_MEDIA_END) && 
		((res = waitfor_socket(ms->sangoma_sock, 1000,  POLLERR | POLLIN)) >= -2)) {
		
		if (res == 0) {
			//log_printf(4, server.log, "%s: TDM UDP Timeout !!!\n",
			//		woomera->interface);
			/* NENAD Timeout thus just continue */
			continue; 
		}

		if (res == -2) {
			close_socket(&ms->sangoma_sock);
			ms->sangoma_sock = sangoma_open_tdmapi_span_chan(woomera->span+1, woomera->chan+1);
			log_printf(0, server.log, "Media Loop Restart %s\n",
				woomera->interface);
			continue;
		}

		if (res  < 0 ){
			log_printf(0, server.log, "Media Loop Socket error %s\n",
				woomera->interface);
			break;
		}
	
		res = sangoma_readmsg_socket(ms->sangoma_sock, 
		                             &hdrframe, 
					     sizeof(hdrframe), 
					     circuit_frame, 
					     sizeof(circuit_frame), 0);
		if (res < 0) {
			log_printf(0, server.log, "TDM Loop ReadMsg Error: %s\n", 
				strerror(errno), woomera->interface);
			break;
		}

		if (server.loop_trace && filed != NULL) {
			int i;
			for (i=0;i<res;i++) {
				fprintf(filed,"%02X ", circuit_frame[i]);
				if (i && (i % 16) == 0) {
					fprintf(filed,"\n");
				}
			}
		      	fprintf(filed,"\n");
		}
			
		sangoma_sendmsg_socket(ms->sangoma_sock, 
		                             &hdrframe, 
					     sizeof(hdrframe), 
					     circuit_frame, 
					     res, 0);
		
		res=0;

		loops++;			     
	} 

	
	if (res < 0) {
		log_printf(2, server.log, "Media Loop: socket error %s  (fd=%i)!\n",
				woomera->interface, ms->sangoma_sock);
	}


	if (server.loop_trace && filed != NULL) {
		fclose(filed);
	}

	sangoma_tdm_enable_hwec(ms->sangoma_sock,&tdm_api);

	sleep(1);

	close_socket(&ms->sangoma_sock);

	log_printf(1, server.log, "Media Loop Finished %s Master=%i MediaEnd=%i Loops=%i\n", 
				woomera->interface, 
				woomera_test_flag(&server.master_connection, WFLAG_RUNNING), 
				woomera_test_flag(woomera, WFLAG_MEDIA_END),loops); 
	
	return;
	
}

#ifdef WP_HPTDM_API
static int media_rx_ready(void *p, unsigned char *data, int len)
{
	struct media_session *ms = (struct media_session *)p;
		
	if (ms->udp_sock < 0) {
		return -1;
	}
	
	return sendto(ms->udp_sock, 
	       	      data,len, 0, 
	      	      (struct sockaddr *) &ms->remote_addr, 
	       	       sizeof(ms->remote_addr));
	       
	
}
#endif

static void *media_thread_run(void *obj)
{
	struct media_session *ms = obj;
	struct woomera_interface *woomera = ms->woomera;
	int sangoma_frame_len = 160;
	int local_port, x = 0, errs = 0, res = 0, packet_len = 0;
	//int udp_cnt=0;
	struct woomera_event wevent;
	wanpipe_tdm_api_t tdm_api;
	FILE *tx_fd=NULL;
	int sock_timeout=200;
	

	if (woomera_test_flag(woomera, WFLAG_MEDIA_END) || 
	    !woomera->interface ||
	    woomera_test_flag(woomera, WFLAG_HANGUP)) {
		log_printf(2, server.log, 
			"MEDIA session for [%s] Cancelled! (ptr=%p)\n", 
				woomera->interface,woomera);
		/* In this case the call will be closed via woomera_thread_run
		* function. And the process table will be cleard there */
		woomera_set_flag(woomera, WFLAG_MEDIA_END);
		woomera_clear_flag(woomera, WFLAG_MEDIA_RUNNING);
		media_session_free(ms);
		pthread_exit(NULL);
		return NULL;
	}


    	log_printf(2, server.log, "MEDIA session for [%s] started (ptr=%p loop=%i)\n", 
			woomera->interface,woomera,woomera->loop_tdm);

	if (woomera->loop_tdm) {   
		media_loop_run(ms);
		ms->udp_sock=-1;
		woomera_set_flag(woomera, WFLAG_HANGUP);
		woomera_clear_flag(woomera, WFLAG_MEDIA_TDM_RUNNING);
		goto media_thread_exit;	
	} 	

    	for(x = 0; x < 1000 ; x++) {
		local_port = next_media_port();
		if ((ms->udp_sock = create_udp_socket(ms, server.media_ip, local_port, ms->ip, ms->port)) > -1) {
			break;
		}
    	}

    	if (ms->udp_sock < 0) {
		log_printf(0, server.log, "UDP Socket Error (%s) [%s] LocalPort=%d\n", 
				strerror(errno), woomera->interface, local_port);

		errs++;
    	} else {
	
#ifdef WP_HPTDM_API
		hp_tdm_api_span_t *span=hptdmspan[woomera->span+1];
		if (!span || !span->init) {
			errs++;
		} else {		
			hp_tdm_api_usr_callback_t usr_callback;
			memset(&usr_callback,0,sizeof(usr_callback));
			usr_callback.p = ms;
			usr_callback.rx_avail = media_rx_ready;
			if (span->open_chan(span, &usr_callback, &ms->tdmchan,woomera->chan+1)) {
				errs++;	
			}
		}
#else
		if ((ms->sangoma_sock = sangoma_create_socket_by_name(woomera->interface, NULL)) < 0) {
			log_printf(0, server.log, "WANPIPE Socket Error (%s) if=[%s]  [w%ig%i]\n", 
				strerror(errno), woomera->interface, woomera->span+1, woomera->chan+1);
			errs++;
		} else {
			
# ifdef CODEC_LAW_DEFAULT
			if (sangoma_tdm_set_codec(ms->sangoma_sock, &tdm_api, WP_NONE) < 0 ) {
				errs++;	
			}
# else
			if (sangoma_tdm_set_codec(ms->sangoma_sock, &tdm_api, WP_SLINEAR) < 0 ) {
				errs++;	
			}
# endif
			if (sangoma_tdm_flush_bufs(ms->sangoma_sock, &tdm_api)) {	
				errs++;
			}
			
			if (sangoma_tdm_set_usr_period(ms->sangoma_sock, &tdm_api, 20) < 0 ) {
				errs++;	
			}

# ifdef CODEC_LAW_DEFAULT			
#  ifdef LIBSANGOMA_GET_HWCODING
			ms->hw_coding=sangoma_tdm_get_hw_coding(ms->sangoma_sock, &tdm_api);
			if (ms->hw_coding < 0) {
				errs++;
			}
#  else
#   error "libsangoma missing hwcoding feature: not up to date!"
#  endif
# endif

			sangoma_frame_len = sangoma_tdm_get_usr_mtu_mru(ms->sangoma_sock,&tdm_api);

		}
#endif	 	
	}

		

#ifdef WP_HPTDM_API
	/* No tdm thread */
#else
	if (!errs && 
	    launch_media_tdm_thread(woomera)) {
		errs++;
	}
#endif

    	if (!errs) {
	
		unsigned char udp_frame[4096];
		sangoma_api_hdr_t hdrframe;
		memset(&hdrframe,0,sizeof(hdrframe));
		memset(udp_frame,0,sizeof(udp_frame));
#ifdef DOTRACE
		int fdin, fdout;
		char path_in[512], path_out[512];
#endif

		

#if 0
		new_woomera_event_printf(&wevent, 
					"EVENT MEDIA %s AUDIO%s"
					"Raw-Audio: %s:%d%s"
					"Call-ID: %s%s"
					"Raw-Format: %s%s"
					,
					woomera->interface,
					WOOMERA_LINE_SEPERATOR,
					server.media_ip,
					local_port,
					WOOMERA_LINE_SEPERATOR,
					woomera->interface,
					WOOMERA_LINE_SEPERATOR,
					ms->hw_coding ?"ALAW":"ULAW",
					WOOMERA_RECORD_SEPERATOR
					);
#else
		new_woomera_event_printf(&wevent, 
					"EVENT MEDIA %s AUDIO%s"
					"Unique-Call-Id: %s%s"
					"Raw-Audio: %s:%d%s"
					"Call-ID: %s%s"
					"Raw-Format: PCM-16%s"
					,
					woomera->interface,
					WOOMERA_LINE_SEPERATOR,
					woomera->session,
					WOOMERA_LINE_SEPERATOR,
					server.media_ip,
					local_port,
					WOOMERA_LINE_SEPERATOR,
					woomera->interface,
                                        WOOMERA_LINE_SEPERATOR,
					WOOMERA_RECORD_SEPERATOR
					);
#endif


		enqueue_event(woomera, &wevent, EVENT_FREE_DATA);
		
#ifdef DOTRACE
		sprintf(path_in, "/tmp/debug-in.%d.raw", tc);
		sprintf(path_out, "/tmp/debug-out.%d.raw", tc++);
		fdin = open(path_in, O_WRONLY | O_CREAT, O_TRUNC, 0600);
		fdout = open(path_out, O_WRONLY | O_CREAT, O_TRUNC, 0600);
#endif

		if (server.out_tx_test) {
			tx_fd=fopen("/smg/sound.raw","rb");
			if (!tx_fd){
				log_printf(0,server.log, "FAILED TO OPEN Sound file!\n");
			}	
		}
		
		
		while ( woomera_test_flag(&server.master_connection, WFLAG_RUNNING) &&
		       !woomera_test_flag(woomera, WFLAG_MEDIA_END) && 
		       !woomera_test_flag(woomera, WFLAG_HANGUP) && 
		       (res = waitfor_socket(ms->udp_sock, sock_timeout, POLLERR | POLLIN)) >= 0) {

			unsigned int fromlen = sizeof(struct sockaddr_in);
		

			if (res == 0) {
				
				if (woomera_dtmf_transmit(ms,sangoma_frame_len) == 0) {
					sock_timeout=(sangoma_frame_len/codec_sample);
				} else {
					sock_timeout=200;
				}
		
				if (ms->skip_write_frames > 0) {
					ms->skip_write_frames--;
				}

				log_printf(4, server.log, "%s: UDP Sock Timeout !!!\n",
						woomera->interface);
				/* NENAD Timeout thus just continue */
				continue;
			}
	
			if ((packet_len = recvfrom(ms->udp_sock, udp_frame, sizeof(udp_frame), 
				MSG_DONTWAIT, (struct sockaddr *) &ms->local_addr, &fromlen)) < 1) {
				log_printf(2, server.log, "UDP Recv Error: %s\n",strerror(errno));
				break;
			}
		
#if 0	
			log_printf(6, server.log, "%s: UDP Receive %i !!!\n",
						woomera->interface,packet_len);
#endif
			
			if (packet_len > 0) {

#if 0
/* NC: This can cause skb_over panic must be retested */
				if (packet_len != sangoma_frame_len && ms->udp_sync_cnt <= 5) {
					/* Assume that we will always receive SLINEAR here */
					sangoma_tdm_set_usr_period(ms->sangoma_sock, 
								   &tdm_api, packet_len/codec_sample);
					sangoma_frame_len =
						 sangoma_tdm_get_usr_mtu_mru(ms->sangoma_sock,&tdm_api);
				
					log_printf(3, server.log, 
						"%s: UDP TDM Period ReSync to Len=%i %ims (udp=%i) \n",
						woomera->interface,sangoma_frame_len,
						sangoma_frame_len/codec_sample,packet_len);


					if (++ms->udp_sync_cnt >= 6) {
				       		sangoma_tdm_set_usr_period(ms->sangoma_sock,
				       			&tdm_api, 20);
				       		sangoma_frame_len =
				       			sangoma_tdm_get_usr_mtu_mru(ms->sangoma_sock,&tdm_api);
						log_printf(0, server.log, 
								"%s: UDP TDM Period Force ReSync to 20ms \n", 
								woomera->interface); 
					}
					  
				}
#endif				
				if (!server.out_tx_test) {

					if (woomera_dtmf_transmit(ms,sangoma_frame_len) == 0) {
						sock_timeout=(sangoma_frame_len/codec_sample);
						/* For sanity sake if we are doing the out test
						* dont take any chances force tx udp data */
						if (ms->skip_write_frames > 0) {
							ms->skip_write_frames--;
						}
						continue;
					} else {
						sock_timeout=200;
					}
	
					if (ms->skip_write_frames > 0) {
						ms->skip_write_frames--;
						continue;
					}

				} 		

				if (server.out_tx_test && tx_fd && 
				    fread((void*)udp_frame,
				                   sizeof(char),
					           packet_len,tx_fd) <= 0) {
						   
					sangoma_get_full_cfg(ms->sangoma_sock,&tdm_api);		   
					fclose(tx_fd);
					tx_fd=NULL;
				}

#ifdef WP_HPTDM_API
				if (ms->tdmchan->push) {
					ms->tdmchan->push(ms->tdmchan,udp_frame,packet_len);
				}
#else
				sangoma_sendmsg_socket(ms->sangoma_sock, 
							&hdrframe, 
							sizeof(hdrframe), 
							udp_frame, 
							packet_len, 0);
#endif

			}

#if 0					
			if (woomera->span == 1 && woomera->chan == 1) {
				udp_cnt++;
				if (udp_cnt && udp_cnt % 1000 == 0) { 
					log_printf(0, server.log, "%s: MEDIA UDP TX RX CNT %i %i\n",
						woomera->interface,udp_cnt,packet_len);
				}
			}
#endif			
		}
	
		if (res < 0) {
			log_printf(2, server.log, "Media Thread: socket error !\n");
		}
    	}



    	new_woomera_event_printf(&wevent, 
					"EVENT HANGUP %s%s"
					"Unique-Call-Id: %s%s"
					"Start-Time: %ld%s"
					"End-Time: %ld%s"
					"Answer-Time: %ld%s"
					"Call-ID: %s%s"
					"Cause: %s%s"
					"Q931-Cause-Code: %d%s",
					woomera->interface,
					WOOMERA_LINE_SEPERATOR,
				
					woomera->session,
					WOOMERA_LINE_SEPERATOR,

					time(&ms->started),
					WOOMERA_LINE_SEPERATOR,

					time(NULL),
					WOOMERA_LINE_SEPERATOR,

					time(&ms->answered),
					WOOMERA_LINE_SEPERATOR,

					woomera->interface,
					WOOMERA_LINE_SEPERATOR,
					
					q931_rel_to_str(woomera->q931_rel_cause_topbx),
					WOOMERA_LINE_SEPERATOR,

					woomera->q931_rel_cause_topbx,
					WOOMERA_RECORD_SEPERATOR
					);

    	enqueue_event(woomera, &wevent,EVENT_FREE_DATA);

	
media_thread_exit:
	
 	if (woomera_test_flag(woomera, WFLAG_MEDIA_TDM_RUNNING)) {
		woomera_set_flag(woomera, WFLAG_MEDIA_END);
		while(woomera_test_flag(woomera, WFLAG_MEDIA_TDM_RUNNING)) {
			usleep(1000);
			sched_yield();
		}
    	}

	
	close_socket(&ms->udp_sock);
	close_socket(&ms->sangoma_sock);

	if (tx_fd){
		fclose(tx_fd);
		tx_fd=NULL;
	}
	
	
	woomera_set_flag(woomera, WFLAG_MEDIA_END);
	
	woomera_set_ms(woomera,NULL);	
	woomera_clear_flag(woomera, WFLAG_MEDIA_RUNNING);

	media_session_free(ms);
	
	log_printf(2, server.log, "MEDIA session for [%s] ended (ptr=%p)\n", 
			woomera->interface,woomera);
			
	
	pthread_exit(NULL);		
	return NULL;
}




static void *media_tdm_thread_run(void *obj)
{
	struct media_session *ms = obj;
	struct woomera_interface *woomera = ms->woomera;
	int res = 0;
	//int tdm_cnt=0;
	//wanpipe_tdm_api_t tdm_api;
	unsigned char circuit_frame[1024];
	sangoma_api_hdr_t hdrframe;

	memset(&hdrframe,0,sizeof(hdrframe));
	memset(circuit_frame,0,sizeof(circuit_frame));
    
	if (woomera_test_flag(woomera, WFLAG_MEDIA_END) || !woomera->interface) {
		log_printf(2, server.log, "MEDIA TDM session for [%s] Cancelled! (ptr=%p)\n",
			 woomera->interface,woomera);
		/* In this case the call will be closed via woomera_thread_run
		* function. And the process table will be cleard there */
		woomera_set_flag(woomera, WFLAG_MEDIA_END);
		woomera_clear_flag(woomera, WFLAG_MEDIA_TDM_RUNNING);
		pthread_exit(NULL);
		return NULL;
	}

   	log_printf(2, server.log, "MEDIA TDM session for [%s] started (ptr=%p)\n",
	 		woomera->interface,woomera);


	while ( woomera_test_flag(&server.master_connection, WFLAG_RUNNING) &&
		!woomera_test_flag(woomera, WFLAG_MEDIA_END) && 
		(res = waitfor_socket(ms->sangoma_sock, 1000, POLLERR | POLLIN)) >= -2) {
		
		if (res == 0) {
			//log_printf(4, server.log, "%s: TDM UDP Timeout !!!\n",
			//		woomera->interface);
			/* NENAD Timeout thus just continue */
			continue;
		} else 	if (res == -2) {
			close_socket(&ms->sangoma_sock);
			ms->sangoma_sock = sangoma_open_tdmapi_span_chan(woomera->span+1, woomera->chan+1);
			if (ms->sangoma_sock < 0) {
				log_printf(0, server.log, "Media TDM Restart Failed%s\n",
						woomera->interface);
				break;
			} 
			log_printf(0, server.log, "Media TDM Restart %s\n",
				woomera->interface);
 			continue;
 		} else 	if (res < 0) {
			log_printf(0, server.log, "Media TDM Sangoma Socket Error %s\n",
				woomera->interface);
			break;
		}
		
		res = sangoma_readmsg_socket(ms->sangoma_sock, 
		                             &hdrframe, 
					     sizeof(hdrframe), 
					     circuit_frame, 
					     sizeof(circuit_frame), 0);
		if (res < 0) {
			log_printf(0, server.log, "TDM ReadMsg Error: %s\n", strerror(errno));
			break;
		}
			
		res = sendto(ms->udp_sock, 
		             circuit_frame, 
			     res, 0, 
			     (struct sockaddr *) &ms->remote_addr, 
			     sizeof(ms->remote_addr));
			
		if (res < 0) {
			log_printf(2, server.log, "UDP Sento Error: %s\n", strerror(errno));
			break;
		}	
#if 0
		if (woomera->span == 1 && woomera->chan == 1) {
			tdm_cnt++;
			if (tdm_cnt && tdm_cnt % 1000 == 0) { 
				log_printf(0, server.log, "%s: MEDIA TDM TX RX CNT %i %i\n",
					woomera->interface,tdm_cnt,res);
			}
		}
#endif		
	}

	if (res < 0) {
		log_printf(2, server.log, "Media TDM Thread: socket error !\n");
	}

    	log_printf(2, server.log, "MEDIA TDM session for [%s] ended (ptr=%p)\n", 
		    woomera->interface,woomera);
    
    	woomera_set_flag(woomera, WFLAG_MEDIA_END);
	woomera_clear_flag(woomera, WFLAG_MEDIA_TDM_RUNNING);
	
	pthread_exit(NULL);
    	return NULL;
	
}


/* This function must be called with process_lock 
 * because it modifies shared process_table */

static int launch_media_thread(struct woomera_interface *woomera) 
{
    pthread_attr_t attr;
    int result = -1;
    struct media_session *ms;

    if ((ms = media_session_new(woomera))) {
		result = pthread_attr_init(&attr);
    		//pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		//pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_attr_setstacksize(&attr, MGD_STACK_SIZE);

		woomera_set_flag(woomera, WFLAG_MEDIA_RUNNING);
		result = pthread_create(&ms->thread, &attr, media_thread_run, ms);
		if (result) {
			log_printf(0, server.log, "%s: Error: Creating Thread! %s\n",
				 __FUNCTION__,strerror(errno));
			woomera_clear_flag(woomera, WFLAG_MEDIA_RUNNING);
			media_session_free(woomera->ms);
			
    		} 
		pthread_attr_destroy(&attr);
	
    } else {
		log_printf(0, server.log, "Failed to start new media session\n");
    }
    
    return result;

}

static int launch_media_tdm_thread(struct woomera_interface *woomera) 
{
	pthread_attr_t attr;
	int result = -1;
	struct media_session *ms = woomera_get_ms(woomera);

	if (!ms) {
		return result;
	}
	
	result = pthread_attr_init(&attr);
        //pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	//pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, MGD_STACK_SIZE);

	woomera_set_flag(woomera, WFLAG_MEDIA_TDM_RUNNING);
	result = pthread_create(&ms->thread, &attr, media_tdm_thread_run, ms);
	if (result) {
		log_printf(0, server.log, "%s: Error: Creating Thread! %s\n",
				 __FUNCTION__,strerror(errno));
		woomera_clear_flag(woomera, WFLAG_MEDIA_TDM_RUNNING);
    	} 
	pthread_attr_destroy(&attr);

   	return result;
}


static struct woomera_interface * launch_woomera_loop_thread(call_signal_event_t *event)
{

	struct woomera_interface *woomera = NULL;
	char callid[20];
	
	sprintf(callid, "w%dg%d", event->span+1,event->chan+1);
	
    	if ((woomera = alloc_woomera())) {
		
		woomera->chan = event->chan;
		woomera->span = event->span;
		woomera->log = server.log;
		woomera->debug = server.debug;
		woomera->call_id = 1;
		woomera->event_queue = NULL;
		woomera->loop_tdm=1;

	} else {
		log_printf(0, server.log, "Critical ERROR: memory/socket error\n");
		return NULL;
	}

	woomera_set_interface(woomera,callid);

	pthread_mutex_lock(&server.process_lock);
        server.process_table[event->span][event->chan].dev = woomera;
        pthread_mutex_unlock(&server.process_lock);
    		
	if (launch_woomera_thread(woomera)) {
		pthread_mutex_lock(&server.process_lock);
		server.process_table[event->span][event->chan].dev = NULL;
		memset(server.process_table[event->span][event->chan].session,0,SMG_SESSION_NAME_SZ);
    		pthread_mutex_unlock(&server.process_lock); 
		smg_free(woomera);
		log_printf(0, server.log, "Critical ERROR: memory/socket error\n");
		return NULL;
	}
	
	return woomera;
}

static int woomera_message_parse(struct woomera_interface *woomera, struct woomera_message *wmsg, int timeout) 
{
    char *cur, *cr, *next = NULL, *eor = NULL;
    char buf[2048];
    int res = 0, bytes = 0, sanity = 0;
    struct timeval started, ended;
    int elapsed, loops = 0;
    int failto = 0;
    int packet = 0;

    memset(wmsg, 0, sizeof(*wmsg));

    if (woomera->socket < 0 ) {
   	 log_printf(2, woomera->log, WOOMERA_DEBUG_PREFIX "%s Invalid Socket! %d\n", 
			 woomera->interface,woomera->socket);
		return -1;
    }

	if (woomera_test_flag(woomera, WFLAG_MEDIA_END) || 
            woomera_test_flag(woomera, WFLAG_HANGUP)) {
		log_printf(5, woomera->log, WOOMERA_DEBUG_PREFIX 
			"%s MEDIA END or HANGUP !\n", 
			woomera->interface);
		return -1;
	}

    gettimeofday(&started, NULL);
    memset(buf, 0, sizeof(buf));

    if (timeout < 0) {
		timeout = abs(timeout);
		failto = 1;
    } else if (timeout == 0) {
		timeout = -1;
    }
	

    while (!(eor = strstr(buf, WOOMERA_RECORD_SEPERATOR))) {
		if (sanity > 1000) {
			log_printf(2, woomera->log, WOOMERA_DEBUG_PREFIX "%s Failed Sanity Check!\n[%s]\n\n", woomera->interface, buf);
			return -1;
		}

		if ((res = waitfor_socket(woomera->socket, 1000, POLLERR | POLLIN) > 0)) {
			res = recv(woomera->socket, buf, sizeof(buf), MSG_PEEK);

			if (res > 1) {
				packet++;
			}
			if (!strncmp(buf, WOOMERA_LINE_SEPERATOR, 2)) {
				res = read(woomera->socket, buf, 2);
				return 0;
			}
			if (res == 0) {
				sanity++;
				/* Looks Like it's time to go! */
				if (!woomera_test_flag(&server.master_connection, WFLAG_RUNNING) ||
				    woomera_test_flag(woomera, WFLAG_MEDIA_END) || 
				    woomera_test_flag(woomera, WFLAG_HANGUP)) {
					log_printf(5, woomera->log, WOOMERA_DEBUG_PREFIX 
						"%s MEDIA END or HANGUP \n", woomera->interface);
					return -1;
				}
				ysleep(1000);
				continue;

			} else if (res < 0) {
				log_printf(3, woomera->log, WOOMERA_DEBUG_PREFIX 
					"%s error during packet retry (err=%i) Loops#%d (%s)\n", 
						woomera->interface, res, loops,
						strerror(errno));
				return res;
			} else if (loops) {
				ysleep(100000);
			}
		}
		
		gettimeofday(&ended, NULL);
		elapsed = (((ended.tv_sec * 1000) + ended.tv_usec / 1000) - ((started.tv_sec * 1000) + started.tv_usec / 1000));

		if (res < 0) {
			log_printf(2, woomera->log, WOOMERA_DEBUG_PREFIX "%s Bad RECV\n", 
				woomera->interface);
			return res;
		} else if (res == 0) {
			sanity++;
			/* Looks Like it's time to go! */
			if (!woomera_test_flag(&server.master_connection, WFLAG_RUNNING) ||
			    woomera_test_flag(woomera, WFLAG_MEDIA_END) || 
			    woomera_test_flag(woomera, WFLAG_HANGUP)) {
				log_printf(5, woomera->log, WOOMERA_DEBUG_PREFIX 
					"%s MEDIA END or HANGUP \n", woomera->interface);
					return -1;
			}
			ysleep(1000);
			continue;
		}

		if (packet && loops > 150) {
			log_printf(1, woomera->log, WOOMERA_DEBUG_PREFIX 
					"%s Timeout waiting for packet.\n", 
						woomera->interface);
			return -1;
		}

		if (timeout > 0 && (elapsed > timeout)) {
			log_printf(1, woomera->log, WOOMERA_DEBUG_PREFIX 
					"%s Timeout [%d] reached\n", 
						woomera->interface, timeout);
			return failto ? -1 : 0;
		}

		if (woomera_test_flag(woomera, WFLAG_EVENT)) {
			/* BRB! we have an Event to deliver....*/
			return 0;
		}

		/* what're we still doing here? */
		if (!woomera_test_flag(&server.master_connection, WFLAG_RUNNING) || 
		    !woomera_test_flag(woomera, WFLAG_RUNNING)) {
			log_printf(2, woomera->log, WOOMERA_DEBUG_PREFIX 
				"%s MASTER RUNNING or RUNNING!\n", woomera->interface);
			return -1;
		}
		loops++;
    }

    *eor = '\0';
    bytes = strlen(buf) + 4;
	
    memset(buf, 0, sizeof(buf));
    res = read(woomera->socket, buf, bytes);
    next = buf;

    if (woomera->debug > 1) {
		log_printf(3, woomera->log, "%s:WOOMERA RX MSG: %s\n",woomera->interface,buf);

    }
	
    while ((cur = next)) {

		if ((cr = strstr(cur, WOOMERA_LINE_SEPERATOR))) {
			*cr = '\0';
			next = cr + (sizeof(WOOMERA_LINE_SEPERATOR) - 1);
			if (!strcmp(next, WOOMERA_RECORD_SEPERATOR)) {
				break;
			}
		} 
		if (!cur || !*cur) {
			break;
		}

		if (!wmsg->last) {
			char *cmd, *id, *args;
			woomera_set_flag(wmsg, MFLAG_EXISTS);
			cmd = cur;

			if ((id = strchr(cmd, ' '))) {
				*id = '\0';
				id++;
				if ((args = strchr(id, ' '))) {
					*args = '\0';
					args++;
					strncpy(wmsg->command_args, args, sizeof(wmsg->command_args)-1);
				}
				strncpy(wmsg->callid, id, sizeof(wmsg->callid)-1);
			}

			strncpy(wmsg->command, cmd, sizeof(wmsg->command)-1);
		} else {
			char *name, *val;
			name = cur;

			if ((val = strchr(name, ':'))) {
				*val = '\0';
				val++;
				while (*val == ' ') {
					*val = '\0';
					val++;
				}
				strncpy(wmsg->values[wmsg->last-1], val, WOOMERA_STRLEN);
			}
			strncpy(wmsg->names[wmsg->last-1], name, WOOMERA_STRLEN);
			if (name && val && !strcasecmp(name, "content-type")) {
				woomera_set_flag(wmsg, MFLAG_CONTENT);
				bytes = atoi(val);
			}
			if (name && val && !strcasecmp(name, "content-length")) {
				woomera_set_flag(wmsg, MFLAG_CONTENT);
				bytes = atoi(val);
			}


		}
		wmsg->last++;
    }

    wmsg->last--;

    if (bytes && woomera_test_flag(wmsg, MFLAG_CONTENT)) {
		read(woomera->socket, wmsg->body, 
		     (bytes > sizeof(wmsg->body)) ? sizeof(wmsg->body) : bytes);
    }

    return woomera_test_flag(wmsg, MFLAG_EXISTS);

}

static struct woomera_interface *pull_from_holding_tank(int index, int span , int chan)
{
	struct woomera_interface *woomera = NULL;
	
	if (index < 1 || index >= CORE_TANK_LEN) {
		if (index != 0) {
			log_printf(0, server.log, "%s Error on invalid TANK INDEX = %i\n",
				__FUNCTION__,index);
		}
		return NULL;
	}

	pthread_mutex_lock(&server.ht_lock);
	if (server.holding_tank[index] && 
	    server.holding_tank[index] != &woomera_dead_dev) {
			woomera = server.holding_tank[index];
	
			/* Block this index until the call is completed */
			server.holding_tank[index] = &woomera_dead_dev;
			
			woomera->timeout = 0;
			woomera->index = 0;
			woomera->span=span;
			woomera->chan=chan;
	}
	pthread_mutex_unlock(&server.ht_lock);
	
	return woomera;
}

static void clear_from_holding_tank(int index, struct woomera_interface *woomera)
{

	if (index < 1 || index >= CORE_TANK_LEN) {
		if (index != 0) {
			log_printf(0, server.log, "%s Error on invalid TANK INDEX = %i\n",
				__FUNCTION__,index);
		}
		return;
	}
	
	pthread_mutex_lock(&server.ht_lock);
	if (server.holding_tank[index] == &woomera_dead_dev) {
		server.holding_tank[index] = NULL;
	} else if (woomera && server.holding_tank[index] == woomera) {
		server.holding_tank[index] = NULL;
	} else if (server.holding_tank[index]) {
		log_printf(0, server.log, "Error: Holding tank index %i not cleared %p !\n",
				index, server.holding_tank[index]);
	}
	pthread_mutex_unlock(&server.ht_lock);
	
	return;
}

static struct woomera_interface *peek_from_holding_tank(int index)
{
	struct woomera_interface *woomera = NULL;
	
	if (index < 1 || index >= CORE_TANK_LEN) {
		if (index != 0) {
			log_printf(0, server.log, "%s Error on invalid TANK INDEX = %i\n",
				__FUNCTION__,index);
		}
		return NULL;
	}
	
	pthread_mutex_lock(&server.ht_lock);
	if (server.holding_tank[index] && 
		server.holding_tank[index] != &woomera_dead_dev) {
			woomera = server.holding_tank[index];
	}
	pthread_mutex_unlock(&server.ht_lock);
	
	return woomera;
}

static int add_to_holding_tank(struct woomera_interface *woomera)
{
	int next, i, found=0;
	
	pthread_mutex_lock(&server.ht_lock);
	
	for (i=0;i<CORE_TANK_LEN;i++) {    
		next = ++server.holding_tank_index;
		if (server.holding_tank_index >= CORE_TANK_LEN) {
			next = server.holding_tank_index = 1;
		} 
	
		if (next == 0) {
			log_printf(0, server.log, "\nCritical Error on TANK INDEX == 0\n");
			continue;
		}
	
		if (server.holding_tank[next]) {
			continue;
		}
	
		found=1;
		break;
	}
	
	if (!found) {
		/* This means all tank vales are busy
		* should never happend */
		pthread_mutex_unlock(&server.ht_lock);
		log_printf(0, server.log, "\nCritical Error failed to obtain a TANK INDEX\n");
		return 0;
	}

	server.holding_tank[next] = woomera;
	woomera->timeout = time(NULL) + 100;
		
	pthread_mutex_unlock(&server.ht_lock);
	return next;
}

static int handle_woomera_media_accept_answer(struct woomera_interface *woomera, 
				      struct woomera_message *wmsg, 
				      int media, int answer, int accept)
{
	char *raw = woomera_message_header(wmsg, "raw-audio");
	
	log_printf(4, woomera->log, "WOOMERA CMD: %s [%s]\n",
		wmsg->command, woomera->interface);	
		
		
	if (woomera_test_flag(woomera, WFLAG_HANGUP) || 
	    !woomera_test_flag(woomera, WFLAG_RUNNING) ||
	    woomera_test_flag(woomera, WFLAG_MEDIA_END)) {
	
		log_printf(2, server.log,
			"ERROR! call was cancelled MEDIA on HANGUP or MEDIA END!\n");

		woomera->timeout=0;
		return -1;
	} 
		
	log_printf(3, server.log,"WOOMERA: GOT %s EVENT: [%s]  RAW=%s\n",
			wmsg->command,wmsg->callid,raw);

			
	if (raw &&
	    woomera->raw == NULL && 
	    !woomera_test_flag(woomera, WFLAG_RAW_MEDIA_STARTED)) {
		
		woomera_set_flag(woomera, WFLAG_RAW_MEDIA_STARTED);
		
		woomera_set_raw(woomera, raw);
				
		if (launch_media_thread(woomera)) {
			struct woomera_event wevent;
			
			log_printf(4, server.log,"ERROR: Failed to Launch Call [%s]\n",
				woomera->interface);

#if 0
			new_woomera_event_printf(&wevent, "501 call was cancelled!%s"
							"Unique-Call-Id: %s%s",
							WOOMERA_LINE_SEPERATOR,
							woomera->session,
							WOOMERA_RECORD_SEPERATOR);

			enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
#endif
			
			new_woomera_event_printf(&wevent, "EVENT HANGUP %s%s"
								"Unique-Call-Id: %s%s"
								"Cause: %s%s"
								"Q931-Cause-Code: %d%s",
								wmsg->callid,
								WOOMERA_LINE_SEPERATOR,
								woomera->session,
								WOOMERA_LINE_SEPERATOR,
								q931_rel_to_str(21),
								WOOMERA_LINE_SEPERATOR,
								21,
								WOOMERA_RECORD_SEPERATOR
								);
			

			enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
				
			woomera_set_flag(woomera, WFLAG_MEDIA_END);
			woomera_clear_flag(woomera, WFLAG_RUNNING);
			
			woomera->timeout=0;
			return -1;
		}
	}

	if (!woomera_test_flag(&server.master_connection, WFLAG_MONITOR_RUNNING)) {
		log_printf(0, server.log,"ERROR! Monitor Thread not running!\n");
		
	} else {

		if (accept) {
			if (!autoacm && !woomera_test_flag(woomera,WFLAG_CALL_ACKED)) {
				woomera_set_flag(woomera,WFLAG_CALL_ACKED);
				isup_exec_command(woomera->span, 
					 	  woomera->chan, 
					  	  -1,
					  	  SIGBOOST_EVENT_CALL_START_ACK,
					  	  0);
			}
		}

		if (answer) {
			struct media_session *ms;
			
			pthread_mutex_lock(&woomera->ms_lock);
			if ((ms=woomera->ms)) {
				time(&woomera->ms->answered);
			}
			pthread_mutex_unlock(&woomera->ms_lock);
			
			if (ms) {

				if (!autoacm && !woomera_test_flag(woomera,WFLAG_CALL_ACKED)) {
					woomera_set_flag(woomera,WFLAG_CALL_ACKED);
					isup_exec_command(woomera->span, 
							woomera->chan, 
							-1,
							SIGBOOST_EVENT_CALL_START_ACK,
							0);
				}

				isup_exec_command(woomera->span, 
						  woomera->chan, 
						  -1,
						  SIGBOOST_EVENT_CALL_ANSWERED,
						  0);
				log_printf(2, server.log,
				"Sent SIGBOOST_EVENT_CALL_ANSWERED [w%dg%d]\n",
					woomera->span+1,woomera->chan+1);
			} else {
				struct woomera_event wevent;
				log_printf(0, server.log,
				  "WOOMERA ANSWER: FAILED [%s] no Media \n",
					wmsg->command,wmsg->callid);	


				new_woomera_event_printf(&wevent, "EVENT HANGUP %s%s"
								  "Unique-Call-Id: %s%s"
							  	  "Cause: %s%s"
								  "Q931-Cause-Code: %d%s",
								wmsg->callid,
								WOOMERA_LINE_SEPERATOR,
								woomera->session,
								WOOMERA_LINE_SEPERATOR,
								q931_rel_to_str(21),
								WOOMERA_LINE_SEPERATOR,
								21,
								WOOMERA_RECORD_SEPERATOR
								);
			
				enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
					
				woomera_set_flag(woomera, WFLAG_MEDIA_END);
				woomera_clear_flag(woomera, WFLAG_RUNNING);
				woomera->timeout=0;
				return -1;
			}
		}
	}

	{
		struct woomera_event wevent;
		new_woomera_event_printf(&wevent, "200 %s OK%s"
						"Unique-Call-Id: %s%s", 
				answer ? "ANSWER" : 
				accept ? "ACCEPT" : "MEDIA", 
				WOOMERA_LINE_SEPERATOR,
				woomera->session,
				WOOMERA_RECORD_SEPERATOR);
	
		enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
       }

       return 0;
}

static int handle_woomera_call_start (struct woomera_interface *woomera,
                                     struct woomera_message *wmsg)
{
       char *raw = woomera_message_header(wmsg, "raw-audio");
       call_signal_event_t event;
       char *calling = woomera_message_header(wmsg, "local-number");
#ifdef SMG_CALLING_NAME
       char *calling_name = woomera_message_header(wmsg, "local-name");
#endif
	char *presentation = woomera_message_header(wmsg, "Presentation");
	char *screening = woomera_message_header(wmsg, "Screening");
	char *rdnis = woomera_message_header(wmsg, "RDNIS");
	char *called = wmsg->callid;
	char *grp = wmsg->callid;
	char *p;
	int cause = 34;
	int tg = 0;

	if (smg_check_all_busy()) {
                socket_printf(woomera->socket, "EVENT HANGUP %s"
                                                  "Cause: %s%s"
                                                  "Q931-Cause-Code: %d%s",
                                                 WOOMERA_LINE_SEPERATOR,
                                                 q931_rel_to_str(cause),
                                                 WOOMERA_LINE_SEPERATOR,
                                                 cause,
                                                 WOOMERA_RECORD_SEPERATOR);
		socket_printf(woomera->socket, 
			"405 SMG Server All Ckt Busy!%s", WOOMERA_RECORD_SEPERATOR);
		log_printf(3, woomera->log, "SMG Server Full %d (ckt busy cnt=%i)\n", 
			server.call_count, server.all_ckt_busy);
		return -1;
	}

	log_printf(2, woomera->log, "New Call %d/%d\n", server.call_count, server.max_calls);

	if ((p = strchr(called, '/'))) {
		*p = '\0';
		called = p+1;
		tg = atoi(grp+1) - 1;
		if (tg < 0) {
			tg=0;
		}
	}

	woomera->trunk_group=tg;

	if (raw) {
		woomera_set_raw(woomera, raw);
	}

	woomera->index =  add_to_holding_tank(woomera);
	if (woomera->index < 1) {
		 socket_printf(woomera->socket, "EVENT HANGUP %s"
                                                  "Cause: %s%s"
                                                  "Q931-Cause-Code: %d%s",
                                                 WOOMERA_LINE_SEPERATOR,
                                                 q931_rel_to_str(cause),
                                                 WOOMERA_LINE_SEPERATOR,
                                                 cause,
                                                 WOOMERA_RECORD_SEPERATOR);
		socket_printf(woomera->socket, 
				"405 SMG Server All Tanks Busy!%s", 
				WOOMERA_RECORD_SEPERATOR);
		log_printf(0, woomera->log, "Error: Call Tank Full (Call Cnt=%i)\n", 
			server.call_count);
		return -1;				
	} 


	woomera->index_hold = woomera->index;
	
	call_signal_call_init(&event, calling, called, woomera->index);

	if (presentation) {
		event.calling_number_presentation = atoi(presentation);
	} else {
		event.calling_number_presentation = 0;
	}

	if (screening) {
		event.calling_number_screening_ind = atoi(screening);
	} else {
		event.calling_number_screening_ind = 0;
	}

	if (rdnis && strlen(rdnis)) {
		strncpy((char*)event.redirection_string,rdnis,
				sizeof(event.redirection_string)-1);
		log_printf(3,server.log,"RDNIS %s\n", rdnis);

	}

#ifdef SMG_CALLING_NAME
	if (calling_name) {
		strncpy((char*)event.calling_name,calling_name,
				sizeof(event.calling_name)-1);
	}
#endif

	event.trunk_group = tg;
	

	if (call_signal_connection_write(&server.mcon, &event) <= 0) {
		log_printf(0, server.log, 
		"Critical System Error: Failed to tx on ISUP socket [%s]: %s\n", 
			strerror(errno));

		 socket_printf(woomera->socket, "EVENT HANGUP %s"
                                                  "Cause: %s%s"
                                                  "Q931-Cause-Code: %d%s",
                                                 WOOMERA_LINE_SEPERATOR,
                                                 q931_rel_to_str(cause),
                                                 WOOMERA_LINE_SEPERATOR,
                                                 cause,
                                                 WOOMERA_RECORD_SEPERATOR);

		socket_printf(woomera->socket, 
				"405 SMG Signalling Contestion!%s", 
				WOOMERA_RECORD_SEPERATOR);
		return -1;
	}

	socket_printf(woomera->socket, "100 Trying%s", WOOMERA_RECORD_SEPERATOR);

	log_printf(2, server.log, "Call Called Event [Setup ID: %d] TG=%d\n",
			woomera->index,tg);
	
	return 0;
}


static void interpret_command(struct woomera_interface *woomera, struct woomera_message *wmsg)
{
    	int answer = 0, media = 0, accept=0;
	char *unique_id;
	int cause=0;



	if (!strcasecmp(wmsg->command, "call")) {
		int err;
		if (strlen(woomera->session) != 0) {
			/* Call has already been placed */
			socket_printf(woomera->socket, "400 Error Call already in progress %s",
						WOOMERA_RECORD_SEPERATOR);
			log_printf(0,server.log,"Woomera RX Call Even while call in progress!\n");
			return;
		}
	
		err=handle_woomera_call_start(woomera,wmsg);
		if (err) {
			woomera_set_flag(woomera, WFLAG_HANGUP);
		}

		return;

	} else if (!strcasecmp(wmsg->command, "bye") || !strcasecmp(wmsg->command, "quit")) {
		char *cause = woomera_message_header(wmsg, "cause");
		char *q931cause = woomera_message_header(wmsg, "Q931-Cause-Code");
		
		if (cause) {
			log_printf(3, woomera->log, "Bye Cause Received: [%s]\n", cause);
		}
		if (q931cause && atoi(q931cause)) {
			woomera_set_cause_tosig(woomera,atoi(q931cause));
		}

		log_printf(2, woomera->log, "WOOMERA CMD: Bye Received: [%s]\n", woomera->interface);
		
		woomera_clear_flag(woomera, WFLAG_RUNNING);
		socket_printf(woomera->socket, "200 Connection closed%s"
						"Unique-Call-Id: %s%s",
						WOOMERA_LINE_SEPERATOR, 
						woomera->session,
						WOOMERA_RECORD_SEPERATOR);
		return;				
	
	} else if (!strcasecmp(wmsg->command, "listen")) {

		if (woomera_test_flag(woomera, WFLAG_LISTENING)) {
			socket_printf(woomera->socket, "405 Listener already started%s"
							"Unique-Call-Id: %s%s",
							WOOMERA_LINE_SEPERATOR, 
							woomera->session,
					 		WOOMERA_RECORD_SEPERATOR);
		} else {
		
			woomera_set_flag(woomera, WFLAG_LISTENING);
			add_listener(woomera);

			if (!strcmp(wmsg->callid,"MASTER")) {
				woomera_set_flag(woomera, WFLAG_MASTER_DEV);
				log_printf(0,woomera->log, "Starting MASTER Listen Device!\n");
				master_reset=0;
			} 
		

			socket_printf(woomera->socket, "%s", 
					WOOMERA_RECORD_SEPERATOR);
			
			socket_printf(woomera->socket, "200 Listener enabled%s"
						       "Unique-Call-Id: %s%s",
							WOOMERA_LINE_SEPERATOR, 
							woomera->session,
							WOOMERA_RECORD_SEPERATOR);
		}
		return;

	} else if ((media = !strcasecmp(wmsg->command, "debug"))) {

		int debug_level=atoi(wmsg->callid);

		if (debug_level < 10) {
			server.debug=debug_level;
			log_printf(0,server.log,"SMG Debugging set to %i (window=%i)\n",server.debug,server.mcon.txwindow);
		}

		return;
	}


	unique_id = woomera_message_header(wmsg, "Unique-Call-Id");	
    	if (!unique_id) {

		cause=111;
                socket_printf(woomera->socket, "EVENT HANGUP %s"
                                                  "Cause: %s%s"
                                                  "Q931-Cause-Code: %d%s",
                                                 WOOMERA_LINE_SEPERATOR,
                                                 q931_rel_to_str(cause),
                                                 WOOMERA_LINE_SEPERATOR,
                                                 cause,
                                                 WOOMERA_RECORD_SEPERATOR);
		socket_printf(woomera->socket, "400 Woomera cmd without uniquie id%s"
						WOOMERA_RECORD_SEPERATOR);

		log_printf(2,server.log,"Woomera RX Event (%s) without unique id!\n",wmsg->command);
		woomera_set_flag(woomera, WFLAG_HANGUP);
		return;
	}

	if (strlen(woomera->session) == 0) {
		struct woomera_interface *session_woomera=NULL;
		char *session=NULL;
		int span, chan;
		char ifname[100];
		/* If session does not exist this is an incoming call */
		sscanf(unique_id, "w%dg%d",  &span, &chan);
		span--;
		chan--;
	
		log_printf(3, woomera->log, 
			"WOOMERA Got CMD %s Span=%d Chan=%d from session %s\n", 	
				wmsg->command,span,chan,unique_id);

		if (smg_validate_span_chan(span,chan) != 0) {

                        cause=34;
                        socket_printf(woomera->socket, "EVENT HANGUP %s"
                                                  "Cause: %s%s"
                                                  "Q931-Cause-Code: %d%s",
                                                 WOOMERA_LINE_SEPERATOR,
                                                 q931_rel_to_str(cause),
                                                 WOOMERA_LINE_SEPERATOR,
                                                 cause,
                                                 WOOMERA_RECORD_SEPERATOR);
			socket_printf(woomera->socket, "404 Invalid span/chan in session%s"
						WOOMERA_RECORD_SEPERATOR);

			log_printf(2, woomera->log, 
				"WOOMERA Warning invalid span chan in session %s %s\n",
				wmsg->command,unique_id);
			woomera_set_flag(woomera, WFLAG_HANGUP);
			return;
		}

		pthread_mutex_lock(&server.process_lock);
		session = server.process_table[span][chan].session;
		session_woomera = server.process_table[span][chan].dev;

		if (session_woomera) {
			pthread_mutex_unlock(&server.process_lock);

                        cause=34;
                        socket_printf(woomera->socket, "EVENT HANGUP %s"
                                                  "Cause: %s%s"
                                                  "Q931-Cause-Code: %d%s",
                                                 WOOMERA_LINE_SEPERATOR,
                                                 q931_rel_to_str(cause),
                                                 WOOMERA_LINE_SEPERATOR,
                                                 cause,
                                                 WOOMERA_RECORD_SEPERATOR);
			socket_printf(woomera->socket, "404 Session not found%s"
						WOOMERA_RECORD_SEPERATOR);

			
			log_printf(3, woomera->log, "WOOMERA Error channel in use %s %s\n",
				wmsg->command,unique_id);
			woomera_set_flag(woomera, WFLAG_HANGUP);
			return;
		}

		if (!session || strlen(session) == 0 ||
		    strncmp(session,unique_id,sizeof(woomera->session))){
			pthread_mutex_unlock(&server.process_lock);

			cause=34;
			socket_printf(woomera->socket, "EVENT HANGUP %s"
                                                  "Cause: %s%s"
                                                  "Q931-Cause-Code: %d%s",
                                                 WOOMERA_LINE_SEPERATOR,
                                                 q931_rel_to_str(cause),
                                                 WOOMERA_LINE_SEPERATOR,
                                                 cause,
                                                 WOOMERA_RECORD_SEPERATOR);

			socket_printf(woomera->socket, "404 Invalid/Expired Session%s"
						WOOMERA_RECORD_SEPERATOR);

			log_printf(3, woomera->log, 
				"WOOMERA Warning: Cmd=%s with invalid session %s (orig=%s)\n", 	
				wmsg->command,unique_id,session?session:"N/A");
			woomera_set_flag(woomera, WFLAG_HANGUP);
			return;
		}

		server.process_table[span][chan].dev=woomera;
		strncpy(woomera->session,unique_id,sizeof(woomera->session));
		sprintf(ifname,"w%dg%d",span+1,chan+1);
		woomera_set_interface(woomera, ifname);

		woomera->span=span;
		woomera->chan=chan;

		pthread_mutex_unlock(&server.process_lock);
		

		log_printf(3, woomera->log, "WOOMERA Got New If=%s Session %s\n", 	
			woomera->interface, woomera->session);
	
	
	} else if (strncmp(woomera->session,unique_id,sizeof(woomera->session))) {

		 cause=34;
                 socket_printf(woomera->socket, "EVENT HANGUP %s"
                                                  "Cause: %s%s"
                                                  "Q931-Cause-Code: %d%s",
                                                 WOOMERA_LINE_SEPERATOR,
                                                 q931_rel_to_str(cause),
                                                 WOOMERA_LINE_SEPERATOR,
                                                 cause,
                                                 WOOMERA_RECORD_SEPERATOR);

		socket_printf(woomera->socket, "404 Session Mis-match%s"
						WOOMERA_RECORD_SEPERATOR);
		 woomera_set_flag(woomera, WFLAG_HANGUP);
		return;
	}


    	if (!strcasecmp(wmsg->command, "dtmf")) {
		
		log_printf(3, woomera->log, "WOOMERA CMD: DTMF Received: [%s]  Digit %s Body %s\n", 	
			woomera->interface, wmsg->command_args, wmsg->body);
	
		wanpipe_send_dtmf(woomera,wmsg->body);
	
		socket_printf(woomera->socket, "200 DTMF OK%s"
						"Unique-Call-Id: %s%s",
						WOOMERA_LINE_SEPERATOR, 
						woomera->session,
						WOOMERA_RECORD_SEPERATOR);

		
    	} else if (!strcasecmp(wmsg->command, "hangup")) {

		int chan = -1, span = -1;
		char *cause = woomera_message_header(wmsg, "cause");
		char *q931cause = woomera_message_header(wmsg, "Q931-Cause-Code");


		if (q931cause && atoi(q931cause)) {
			woomera_set_cause_tosig(woomera,atoi(q931cause));
		}
	
		span=woomera->span;
		chan=woomera->chan;
			
		log_printf(3, woomera->log, "WOOMERA CMD: Hangup Received: [%s] MEDIA EXIST Cause=%s\n",
				 	woomera->interface,cause);
		
		if (smg_validate_span_chan(span,chan) != 0) {
			
			socket_printf(woomera->socket, "405 No Such Channel%s"
							"Unique-Call-Id: %s%s",
							WOOMERA_LINE_SEPERATOR, 
							woomera->session,
							WOOMERA_RECORD_SEPERATOR);
			return;
		}

	
		log_printf(2, woomera->log, "Hangup Received: [w%dg%d]\n", 	
				span+1,chan+1);	

		
		woomera_set_flag(woomera, WFLAG_MEDIA_END);
		woomera_clear_flag(woomera, WFLAG_RUNNING);

			
		socket_printf(woomera->socket, "200 HANGUP OK%s"
						"Unique-Call-Id: %s%s",
						WOOMERA_LINE_SEPERATOR, 
						woomera->session,
						WOOMERA_RECORD_SEPERATOR);
 
	
     } else if (!strcasecmp(wmsg->command, "proceed")) {
		
		log_printf(3, woomera->log, "WOOMERA CMD: %s [%s]\n",
			wmsg->command, woomera->interface);
		
		socket_printf(woomera->socket, 
				"200 %s PROCEED OK%s"
				"Unique-Call-Id: %s%s", 
				wmsg->callid, 
				WOOMERA_LINE_SEPERATOR,
				woomera->session,
				WOOMERA_RECORD_SEPERATOR);
        			
    } else if ((media = !strcasecmp(wmsg->command, "media")) || 
               (answer = !strcasecmp(wmsg->command, "answer")) ||
	       (accept = !strcasecmp(wmsg->command, "accept"))) {
	       
		handle_woomera_media_accept_answer(woomera, wmsg, media,answer,accept);

    

    } else {
	    log_printf(0, server.log,"WOOMERA INVALID EVENT:  %s  [%s] \n",
					wmsg->command,wmsg->callid);
		socket_printf(woomera->socket, "501 Command '%s' not implemented%s", 
				wmsg->command, WOOMERA_RECORD_SEPERATOR);
    }
}


/*
  EVENT INCOMING 1
  Remote-Address: 10.3.3.104
  Remote-Number:
  Remote-Name: Anthony Minessale!8668630501
  Protocol: H.323
  User-Agent: Post Increment Woomera		1.0alpha1 (OpenH323 v1.17.2)	9/61
  H323-Call-Id: 887b1ff8-bb1f-da11-85c0-0007e98988c4
  Local-Number: 996
  Start-Time: Fri, 09 Sep 2005 12:25:14 -0400
  Local-Name: root
*/


static void handle_call_answer(call_signal_event_t *event)
{
    	struct woomera_interface *woomera = NULL;
	int kill = 0;

    	pthread_mutex_lock(&server.process_lock);
    	woomera = server.process_table[event->span][event->chan].dev;
    	pthread_mutex_unlock(&server.process_lock);	
    
    	if (woomera) {
		char callid[80];
		struct woomera_event wevent;

		if (!woomera->raw) {
			log_printf(1, server.log, "Refusing to answer call with no media!\n");
			kill++;
			goto handle_call_answer_end;
		}

		if (woomera_test_flag(woomera, WFLAG_ANSWER)) {
			log_printf(1, server.log, "Refusing to double-answer a call!\n");
			kill++;
			goto handle_call_answer_end;
		}

		woomera_set_flag(woomera, WFLAG_ANSWER);

		if (woomera->span != event->span || woomera->chan != event->chan) {
			log_printf(1, server.log, "Refusing to start media on a different channel from the one we agreed on.!\n");
			kill++;
			goto handle_call_answer_end;
		}

		if (woomera_test_flag(woomera, WFLAG_HANGUP)) {
			log_printf(1, server.log, "Refusing to answer a dead call!\n");
			kill++;
		} else {
			int err;
			err=0;
			sprintf(callid, "w%dg%d", event->span + 1, event->chan + 1);
			woomera_set_interface(woomera, callid);
#ifndef WOOMERA_EARLY_MEDIA
			err=launch_media_thread(woomera);
			if (err) {
				log_printf(0, server.log,"ERROR: Failed to Launch Call [%s]\n",
					woomera->interface);
#if 0
				new_woomera_event_printf(&wevent, "501 call was cancelled!%s"
								"Unique-Call-Id: %s%s",
								 WOOMERA_LINE_SEPERATOR,
								 woomera->session,
								 WOOMERA_RECORD_SEPERATOR);

				enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
#endif
							 
				new_woomera_event_printf(&wevent, "EVENT HANGUP %s%s"
								  "Unique-Call-Id: %s%s"
							          "Q931-Cause-Code: %d%s"
								  "Cause: %s%s",
						 	 woomera->interface,
							 WOOMERA_LINE_SEPERATOR,
							 woomera->session,
							 WOOMERA_LINE_SEPERATOR,
							 21,
							 WOOMERA_LINE_SEPERATOR,
							 q931_rel_to_str(21),
							 WOOMERA_RECORD_SEPERATOR
							 );
					
				enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
				
				
				
				kill++;
			} 
#endif

			if (!kill) {
				new_woomera_event_printf(&wevent, "EVENT CONNECT w%dg%d%s"
								  "Unique-Call-Id: %s%s",
									 event->span+1,
									 event->chan+1,
									 WOOMERA_LINE_SEPERATOR,
									 woomera->session,
									 WOOMERA_RECORD_SEPERATOR
									 );
				enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
			}
		}
        } else {
		log_printf(1, server.log, "Answer requested on non-existant session. [w%dg%d]\n",
					event->span+1, event->chan+1);
		kill++;
	}

handle_call_answer_end:

	if (kill) {
		if (woomera) {
			woomera_set_flag(woomera,WFLAG_MEDIA_END);
		} else {

/* This can casuse a double STOP 
   must be debugged further */
#if 0
			isup_exec_command(event->span, 
					event->chan, 
					-1,
					SIGBOOST_EVENT_CALL_STOPPED,
					SIGBOOST_RELEASE_CAUSE_NORMAL);
									
			log_printf(1, server.log, "Sent CALL STOP to Answer without session [w%dg%d]\n", 
					event->span+1, event->chan+1);
#endif
			log_printf(0, server.log, "WARNING: Received Answer with no session [w%dg%d]\n", 
					event->span+1, event->chan+1);
		}
	}
}

static void handle_call_start_ack(call_signal_event_t *event)
{
    	struct woomera_interface *woomera = NULL;
    	struct woomera_event wevent;
	int kill = 0;

   	if ((woomera = peek_from_holding_tank(event->call_setup_id))) {
		char callid[80];
	
		if (woomera_test_flag(woomera, WFLAG_HANGUP)) {
			log_printf(1, server.log, "Refusing to ack a dead call!\n");
			kill++;
		} else {
			int err, span, chan;
			
			pull_from_holding_tank(event->call_setup_id,event->span,event->chan);
			sprintf(callid, "w%dg%d", event->span + 1, event->chan + 1);
			
			span = event->span;
			chan = event->chan;

			woomera_set_flag(woomera,WFLAG_CALL_ACKED);
			woomera_set_interface(woomera, callid);

			pthread_mutex_lock(&server.process_lock);
			server.process_table[span][chan].dev = woomera;
			sprintf(woomera->session,"%s-%i-%i",callid,rand(),rand());
			sprintf(server.process_table[span][chan].session,"%s-%s",
						callid,woomera->session);
			pthread_mutex_unlock(&server.process_lock);
					
			
			
#ifdef WOOMERA_EARLY_MEDIA
			err=launch_media_thread(woomera);
			if (err) {
				log_printf(0, server.log,"ERROR: Failed to Launch Call [%s]\n",
					woomera->interface);
					

#if 0
				new_woomera_event_printf(&wevent, "501 call was cancelled!%s"
								 "Unique-Call-Id: %s%s",
								 WOOMERA_LINE_SEPERATOR,
								 woomera->session,
				 				 WOOMERA_RECORD_SEPERATOR);

				enqueue_event(woomera, &wevent,EVENT_FREE_DATA);	
#endif							
	 
				new_woomera_event_printf(&wevent, "EVENT HANGUP %s%s"
								  "Unique-Call-Id:%s%s"
							          "Q931-Cause-Code: %d%s"
								  "Cause: %s%s",
								 woomera->interface,
								 WOOMERA_LINE_SEPERATOR,
								 woomera->session,
								 WOOMERA_LINE_SEPERATOR,
								 21,
								 WOOMERA_LINE_SEPERATOR,
								 q931_rel_to_str(21),
								 WOOMERA_RECORD_SEPERATOR
								 );
					
				enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
			
				kill++;
			}
#endif
			if (!kill) {
				new_woomera_event_printf(&wevent, "201 Accepted%s"
								"Unique-Call-Id: %s%s",
								WOOMERA_LINE_SEPERATOR,
								woomera->session,
								WOOMERA_RECORD_SEPERATOR);

				enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
								
				new_woomera_event_printf(&wevent, "EVENT PROCEED w%dg%d%s"
								  "Channel-Name: g%d/%d%s"
								  "Unique-Call-Id: %s%s",
								event->span+1,
								event->chan+1,
								WOOMERA_LINE_SEPERATOR,
								woomera->trunk_group+1,
								(event->span*31)+event->chan+1,
								WOOMERA_LINE_SEPERATOR,
								woomera->session,
								WOOMERA_RECORD_SEPERATOR
								);
								
				enqueue_event(woomera, &wevent,EVENT_FREE_DATA);
	
				log_printf(2, server.log, "Call Answered Event ID = %d  Device = w%dg%d!\n",
						event->call_setup_id,woomera->span+1,woomera->chan+1);
			}
		}
	} else {
		log_printf(1, server.log, 
			"Event (START ACK) %d referrs to a non-existant session (%d) [w%dg%d]!\n",
				event->event_id, event->call_setup_id,event->span+1, event->chan+1);
		kill++;
	}

	if (kill) {
		if (woomera) {
			woomera_set_flag(woomera,WFLAG_MEDIA_END);
		} else {
			isup_exec_command(event->span, 
					event->chan, 
					-1,
					SIGBOOST_EVENT_CALL_STOPPED,
					SIGBOOST_RELEASE_CAUSE_NORMAL);
							
			log_printf(1, server.log, "Sent CALL STOP to CALL START ACK without session [w%dg%d]\n", 
					event->span+1, event->chan+1);
		}
	}
}

static void handle_call_start_nack(call_signal_event_t *event)
{
   	struct woomera_interface *woomera = NULL;
	int span=-1, chan=-1;
	int ack=0;
	
	/* Always ACK the incoming NACK 
	 * Send out the NACK ACK before pulling the TANK, because
	 * if we send after the pull, the outgoing call could send
	 * a message to boost with the pulled TANK value before
	 * we send a NACK ACK */
	 
	if (smg_validate_span_chan(event->span,event->chan) == 0) {
		span=event->span;
		chan=event->chan;
	}

	if (event->call_setup_id > 0) {
		woomera=peek_from_holding_tank(event->call_setup_id);
	}
	
	if (woomera) {
	
		struct woomera_event wevent;

		if (woomera_test_flag(woomera, WFLAG_HANGUP)) {
			log_printf(0, server.log, "Event CALL START NACK on hungup call [%d]!\n",
				event->call_setup_id);
			ack++;
		} else {

			woomera_set_cause_topbx(woomera,event->release_cause);	
			woomera_set_flag(woomera, (WFLAG_HANGUP|WFLAG_HANGUP_NACK_ACK)); 

			new_woomera_event_printf(&wevent, "EVENT HANGUP w%dg%d%s"
							  "Unique-Call-Id: %s%s"
							  "Cause: %s%s"
							  "Q931-Cause-Code: %d%s",
							 event->span+1,
							 event->chan+1,
							 WOOMERA_LINE_SEPERATOR,
							 woomera->session,
							 WOOMERA_LINE_SEPERATOR,
							 q931_rel_to_str(woomera->q931_rel_cause_topbx),
							 WOOMERA_LINE_SEPERATOR,
							 woomera->q931_rel_cause_topbx,
							 WOOMERA_RECORD_SEPERATOR
								 );
			enqueue_event(woomera, &wevent,EVENT_FREE_DATA);

			
			woomera_set_flag(woomera, WFLAG_HANGUP);
			woomera_clear_flag(woomera, WFLAG_RUNNING);
			
			/* Do not ack here, let woomera thread ack it */
			ack=0;
		}
	
		
	} else if (event->call_setup_id == 0 && 
		   smg_validate_span_chan(event->span,event->chan) == 0) {
	
		pthread_mutex_lock(&server.process_lock);
		woomera = server.process_table[event->span][event->chan].dev;

		if (woomera) {
			if (!woomera_test_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK_SENT) && 
		            !woomera_test_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK_SENT)) {
				/* Only if we are not already waiting for hangup */
				server.process_table[event->span][event->chan].dev=NULL;
			} else if (woomera_test_flag(woomera, WFLAG_HANGUP)) {
				/* At this point call is already hang up */
				woomera=NULL;
			} else {
				/* At this point call is already hang up */
				woomera=NULL;
			}
		}

		memset(server.process_table[event->span][event->chan].session,
			0,SMG_SESSION_NAME_SZ);			
		pthread_mutex_unlock(&server.process_lock);

		
		if (woomera) {
			
			log_printf(2, server.log, "Event START NACK on w%dg%d ptr=%p ms=%p\n",
					woomera->span+1,woomera->chan+1,woomera,woomera->ms);

			if (!woomera_test_flag(woomera,WFLAG_HANGUP)){
				woomera_set_cause_topbx(woomera,event->release_cause);
			}

			woomera_set_flag(woomera, 
				(WFLAG_HANGUP|WFLAG_MEDIA_END|WFLAG_HANGUP_NACK_ACK)); 

			/* Nack Ack will be sent by the woomera thread */
			ack=0;		
		} else {
			/* Valid state when we are not in autoacm mode */
			ack++;
			log_printf(3, server.log, "Event: NACK no woomera on span chan [w%dg%d]!\n",
				event->span+1, event->chan+1);
		}
  				
	} else {
		log_printf(0, server.log, 
			"Error: Start Nack Invalid State Should not happen [%d] [w%dg%d]!\n",
				event->call_setup_id, event->span+1, event->chan+1);
		ack++;
	}


	if (event->release_cause == SIGBOOST_CALL_SETUP_NACK_ALL_CKTS_BUSY) {
		log_printf(0, server.log, "WARNING: All ckt busy!\n");	
		smg_all_ckt_busy();
	}


#warning "Ignoring CALL GAP"
#if 0
	if (event->release_cause == SIGBOOST_CALL_SETUP_NACK_AUTO_CALL_GAP) {
		log_printf(0, server.log, "WARNING: Call Gapping Detected!\n");	
		smg_all_ckt_gap();
 	} 
#endif
	
	if (ack) {
		span=0;
		chan=0;
		if (smg_validate_span_chan(event->span,event->chan) == 0) {
			span=event->span;
			chan=event->chan;	
		}

		isup_exec_command(span,
				  chan,
				  event->call_setup_id,
				  SIGBOOST_EVENT_CALL_START_NACK_ACK,
				  0);
		
		if (!woomera) {	
		log_printf(2, server.log, "Event (NACK ACK) %d referrs to a non-existant session (%d) [w%dg%d]!\n",
				event->event_id,event->call_setup_id, event->span+1, event->chan+1);
		}
	}
}

static void handle_call_loop_start(call_signal_event_t *event)
{
	struct woomera_interface *woomera;
    	char callid[20];
	char *session;
	
	pthread_mutex_lock(&server.process_lock);
    	if (server.process_table[event->span][event->chan].dev) {

		isup_exec_command(event->span, 
				  event->chan, 
				  -1,
				  SIGBOOST_EVENT_CALL_START_NACK,
				  SIGBOOST_RELEASE_CAUSE_BUSY);

	
		log_printf(1, server.log, 
			"Sent (From Handle Loop START) Call Busy SIGBOOST_EVENT_CALL_START_NACK  [w%dg] ptr=%d\n", 
				event->span+1, event->chan+1, server.process_table[event->span][event->chan].dev);
	
		pthread_mutex_unlock(&server.process_lock);
		return;
				
    	}	

 	sprintf(callid, "w%dg%d", event->span+1,event->chan+1);
        sprintf(server.process_table[event->span][event->chan].session,
                                         "%s-%i-%i",callid,rand(),rand());
        session=server.process_table[event->span][event->chan].session;
        server.process_table[event->span][event->chan].dev = NULL;
        pthread_mutex_unlock(&server.process_lock);


	woomera=launch_woomera_loop_thread(event);
	if (woomera == NULL) {
		isup_exec_command(event->span, 
			  event->chan, 
			  -1,
			  SIGBOOST_EVENT_CALL_START_NACK,
			  SIGBOOST_RELEASE_CAUSE_BUSY);
		log_printf(1, server.log, 
		"Sent (From Handle Loop START) Call Busy SIGBOOST_EVENT_CALL_START_NACK  [w%dg] ptr=%d\n", 
			event->span+1, event->chan+1, server.process_table[event->span][event->chan].dev);
	}
		
	woomera_set_flag(woomera,WFLAG_CALL_ACKED);

	return;
}

static void handle_call_start(call_signal_event_t *event)
{
    	struct woomera_event wevent;
    	char callid[20];
	char *session;

    	pthread_mutex_lock(&server.process_lock);

    	if (server.process_table[event->span][event->chan].dev) {
    	
		isup_exec_command(event->span, 
				  event->chan, 
				  -1,
				  SIGBOOST_EVENT_CALL_START_NACK,
				  SIGBOOST_RELEASE_CAUSE_BUSY);

	
		log_printf(0, server.log, 
			"Sent (From Handle START) Call Busy SIGBOOST_EVENT_CALL_START_NACK  [w%dg%d]\n", 
				event->span+1, event->chan+1);
	
		pthread_mutex_unlock(&server.process_lock);
		return;
    	}	
	
	sprintf(callid, "w%dg%d", event->span+1,event->chan+1);
	sprintf(server.process_table[event->span][event->chan].session,
					 "%s-%i-%i",callid,rand(),rand()); 
	session=server.process_table[event->span][event->chan].session;
    	server.process_table[event->span][event->chan].dev = NULL;
    	pthread_mutex_unlock(&server.process_lock);
    	
	if (autoacm) {
		isup_exec_command(event->span, 
				event->chan, 
				-1,
				SIGBOOST_EVENT_CALL_START_ACK,
				0);
	}	
    
    	new_woomera_event_printf(&wevent, "EVENT INCOMING w%dg%d%s"
					  		 "Unique-Call-Id: %s%s"
							 "Remote-Number: %s%s"
							 "Remote-Name: %s%s"
							 "Protocol: SS7%s"
							 "User-Agent: sangoma_mgd%s"
							 "Local-Number: %s%s"
							 "Channel-Name: g%d/%d%s"
							 "Trunk-Group: %d%s"
							 "Presentation: %d%s"
							 "Screening: %d%s"
							 "RDNIS: %s%s"
							 ,
							 event->span+1,
							 event->chan+1,
							 WOOMERA_LINE_SEPERATOR,
							 session,
							 WOOMERA_LINE_SEPERATOR,
							 event->calling_number_digits,
							 WOOMERA_LINE_SEPERATOR,
#ifdef SMG_CALLING_NAME
							 event->calling_name,
#else
							 "",
#endif
							 WOOMERA_LINE_SEPERATOR,
							 WOOMERA_LINE_SEPERATOR,
							 WOOMERA_LINE_SEPERATOR,
							 event->called_number_digits,
							 WOOMERA_LINE_SEPERATOR,
							 event->trunk_group+1,
							 (event->span*31)+event->chan+1,
							 WOOMERA_LINE_SEPERATOR,
							 event->trunk_group+1,
							 WOOMERA_LINE_SEPERATOR,
							 event->calling_number_presentation,
							 WOOMERA_LINE_SEPERATOR,
							 event->calling_number_screening_ind,
							 WOOMERA_LINE_SEPERATOR,
							 event->redirection_string,
							 WOOMERA_RECORD_SEPERATOR
						 );

    	if (!enqueue_event_on_listeners(&wevent)) {
		
		pthread_mutex_lock(&server.process_lock);
		server.process_table[event->span][event->chan].dev = NULL;
		memset(server.process_table[event->span][event->chan].session,0,SMG_SESSION_NAME_SZ);
    		pthread_mutex_unlock(&server.process_lock);  
	
		if (autoacm) {
			isup_exec_command(event->span, 
					event->chan, 
					-1,
					SIGBOOST_EVENT_CALL_STOPPED,
					SIGBOOST_RELEASE_CAUSE_BUSY);
		} else {
			isup_exec_command(event->span, 
					  event->chan, 
					  -1,
					  SIGBOOST_EVENT_CALL_START_NACK,
					  SIGBOOST_RELEASE_CAUSE_BUSY);
		}
	
		log_printf(0, server.log, 
			"CALL INCOMING: Enqueue Error Sent SIGBOOST_EVENT_CALL_STOPPED  [w%dg%d]\n", 
				event->span+1, event->chan+1);
    	}
	
	destroy_woomera_event_data(&wevent);
       
}

static void handle_gap_abate(call_signal_event_t *event)
{
	log_printf(0, server.log, "NOTICE: GAP Cleared!\n", 
			event->span+1, event->chan+1);
	smg_clear_ckt_gap();
}

static void handle_restart_ack(call_signal_connection_t *mcon,call_signal_event_t *event)
{
	mcon->rxseq_reset	=0;
}

static void handle_call_stop(call_signal_event_t *event)
{
    	struct woomera_interface *woomera;
	int ack=0;

    	pthread_mutex_lock(&server.process_lock);
    	woomera = server.process_table[event->span][event->chan].dev;
	if (woomera) {
		if (!woomera_test_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK_SENT) && 
            	    !woomera_test_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK_SENT)) {
			/* Only if we are not already waiting for hangup */
			server.process_table[event->span][event->chan].dev=NULL;
		} else if (woomera_test_flag(woomera, WFLAG_HANGUP)) {
			/* At this point call is already hangup */
			woomera=NULL;
		} else {
			/* At this point call is already hangup */
			woomera=NULL;
		}
	}
	memset(server.process_table[event->span][event->chan].session,0,SMG_SESSION_NAME_SZ);
	pthread_mutex_unlock(&server.process_lock);
    
        if (woomera) {
	
		woomera_set_cause_topbx(woomera,event->release_cause);
	
		woomera_set_flag(woomera,
				(WFLAG_MEDIA_END|WFLAG_HANGUP|WFLAG_HANGUP_ACK));

		/* We have to close the socket because
	           At this point we are release span chan */

		log_printf(3, server.log, "Event CALL STOP on w%dg%d ptr=%p ms=%p\n",
				woomera->span+1,woomera->chan+1,woomera,woomera->ms);

	} else {
		ack++;
	}


	if (ack) {	
		/* At this point we have already sent our STOP so its safe to ACK */
		isup_exec_command(event->span, 
		  	  event->chan, 
		  	  -1,
		  	  SIGBOOST_EVENT_CALL_STOPPED_ACK,
		  	  0);
	}
		
	if (!woomera){
		/* This is allowed on incoming call if remote app does not answer it */
		log_printf(3, server.log, "Event CALL STOP referrs to a non-existant session [w%dg%d]!\n",
			event->span+1, event->chan+1);
	}
}

static void handle_heartbeat(call_signal_event_t *event)
{
	if (!woomera_test_flag(&server.master_connection, WFLAG_MONITOR_RUNNING)) {
		log_printf(0, server.log,"ERROR! Monitor Thread not running!\n");
	} else {	
		int err=call_signal_connection_write(&server.mcon, event);
		if (err <= 0) {
			log_printf(0, server.log,
                                   "Critical System Error: Failed to tx on ISUP socket [%s]: %s\n",
                                         strerror(errno));
		}
	}
	return;
}

static void handle_call_stop_ack(call_signal_event_t *event)
{

	struct woomera_interface *woomera = NULL;
	
	pthread_mutex_lock(&server.process_lock);
    	woomera = server.process_table[event->span][event->chan].dev;
	server.process_table[event->span][event->chan].dev=NULL;
	memset(server.process_table[event->span][event->chan].session,0,SMG_SESSION_NAME_SZ);
    	pthread_mutex_unlock(&server.process_lock);

	
	if (woomera) {
		log_printf(2, server.log, "Stop Ack on [w%dg%d] [Setup ID: %d] [%s]!\n",
				event->span+1, event->chan+1, event->call_setup_id,
				woomera->interface);

		woomera_clear_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK);

	} else {
		log_printf(2, server.log, "Event CALL_STOP_ACK(%d) referrs to a non-existant session [w%dg%d] [Setup ID: %d]!\n",
				event->event_id, event->span+1, event->chan+1, event->call_setup_id);
	}
	
	/* No need for us to do any thing here */
	return;
}


static void handle_call_start_nack_ack(call_signal_event_t *event)
{

   	struct woomera_interface *woomera = NULL;
	
	if ((woomera=pull_from_holding_tank(event->call_setup_id,-1,-1))) {
	
		woomera_clear_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK);

	} else if (event->call_setup_id == 0 &&
		   smg_validate_span_chan(event->span,event->chan) == 0) {

		pthread_mutex_lock(&server.process_lock);
		woomera = server.process_table[event->span][event->chan].dev;
		server.process_table[event->span][event->chan].dev=NULL;
		memset(server.process_table[event->span][event->chan].session,
			0,SMG_SESSION_NAME_SZ);			
		pthread_mutex_unlock(&server.process_lock);

		if (woomera) {
			woomera_clear_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK);
		} else {
			log_printf(0, server.log, 
			"Event NACK ACK [w%dg%d] with valid span/chan no dev!\n",
				event->span+1, event->chan+1);
		}

	} else {
		log_printf(2, server.log, 
			"Event NACK ACK referrs to a non-existant session [w%dg%d] [Setup ID: %d]!\n",
				event->span+1, event->chan+1, event->call_setup_id);
	}
	
	if (event->call_setup_id > 0) {
		clear_from_holding_tank(event->call_setup_id, NULL);	
	}

	/* No need for us to do any thing here */
	return;
}

static void validate_number(unsigned char *s)
{
	unsigned char *p;
	for (p = s; *p; p++) {
		if (*p < 48 || *p > 57) {
			log_printf(2, server.log, "Encountered a non-numeric character [%c]!\n", *p);
			*p = '\0';
			break;
		}
	}
}

static int parse_ss7_event(call_signal_connection_t *mcon, call_signal_event_t *event)
{
    	int ret = 0;

	if (server.strip_cid_non_digits) {	
		validate_number((unsigned char*)event->called_number_digits);
		validate_number((unsigned char*)event->calling_number_digits);
	}

#if 1
 	log_printf(2, server.log,
                           "RX EVENT: %s:(%X) [w%dg%d] Rc=%i CSid=%i Seq=%i Cd=[%s] Ci=[%s]\n",
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
	log_printf(2, server.log, "RX EVENT\n");
        log_printf(2, server.log, "===================================\n");
        log_printf(2, server.log, "       rType: %s (%0x HEX)\n",
                               call_signal_event_id_name(event->event_id),event->event_id);
        log_printf(2, server.log, "       rSpan: [%d]\n",event->span+1);
        log_printf(2, server.log, "       rChan: [%d]\n",event->chan+1);
        log_printf(2, server.log, "  rCalledNum: %s\n",
                       (event->called_number_digits_count ? (char *) event->called_number_digits : "N/A"));
        log_printf(2, server.log, " rCallingNum: %s\n",
                     (event->calling_number_digits_count ? (char *) event->calling_number_digits : "N/A"));
       log_printf(2, server.log, "      rCause: %d\n",event->release_cause);
       log_printf(2, server.log, "  rInterface: [w%dg%d]\n",event->span+1,event->chan+1);
       log_printf(2, server.log, "   rEvent ID: [%d]\n",event->event_id);
       log_printf(2, server.log, "   rSetup ID: [%d]\n",event->call_setup_id);
       log_printf(2, server.log, "        rSeq: [%d]\n",event->fseqno);
       log_printf(2, server.log, "===================================\n");

#endif

#if 0
	log_printf(2, server.log,
                           "\nRX EVENT\n"
                           "===================================\n"
                           "           rType: %s (%0x HEX)\n"
                           "           rSpan: [%d]\n"
                           "           rChan: [%d]\n"
                           "  rCalledNum: %s\n"
                           " rCallingNum: %s\n"
                           "      rCause: %s\n"
                           " rInterface : [w%dg%d]\n"
                           "  rEvent ID : [%d]\n"
                           "   rSetup ID: [%d]\n"
                           "        rSeq: [%d]\n"
                           "===================================\n"
                           "\n",
                           call_signal_event_id_name(event->event_id),
                           event->event_id,
                           event->span+1,
                           event->chan+1,
                           (event->called_number_digits_count ? (char *) event->called_number_digits : "N/A"),
                           (event->calling_number_digits_count ? (char *) event->calling_number_digits : "N/A"),
			   release_to_string(event->release_cause),
                           event->span+1,
                           event->chan+1,
                           event->event_id,
                           event->call_setup_id,
                           event->fseqno
                           );
#endif
	

	switch(event->event_id) {
	
	case SIGBOOST_EVENT_CALL_START:
			handle_call_start(event);
			break;
	case SIGBOOST_EVENT_CALL_STOPPED:
			handle_call_stop(event);
			break;
	case SIGBOOST_EVENT_CALL_START_ACK:
			handle_call_start_ack(event);
			break;
	case SIGBOOST_EVENT_CALL_START_NACK:
			handle_call_start_nack(event);
			break;
	case SIGBOOST_EVENT_CALL_ANSWERED:
			handle_call_answer(event);
			break;
	case SIGBOOST_EVENT_HEARTBEAT:
			handle_heartbeat(event);
			break;
	case SIGBOOST_EVENT_CALL_START_NACK_ACK:
			handle_call_start_nack_ack(event);
			break;
	case SIGBOOST_EVENT_CALL_STOPPED_ACK:
			handle_call_stop_ack(event);
			break;
	case SIGBOOST_EVENT_INSERT_CHECK_LOOP:
			handle_call_loop_start(event);
			break;
	case SIGBOOST_EVENT_REMOVE_CHECK_LOOP:
			handle_call_stop(event);
			break;
	case SIGBOOST_EVENT_SYSTEM_RESTART_ACK:
			handle_restart_ack(mcon,event);
			break;
	case SIGBOOST_EVENT_AUTO_CALL_GAP_ABATE:
			handle_gap_abate(event);
			break;
	default:
			log_printf(0, server.log, "Warning no handler implemented for [%s]\n", 
				call_signal_event_id_name(event->event_id));
			break;
	}
		
	return ret;
}


static void *monitor_thread_run(void *obj)
{
	int ss = 0;
	int policy=0,priority=0;
	char a=0,b=0;
	call_signal_connection_t *mcon=&server.mcon;
	call_signal_connection_t *mconp=&server.mconp;

    	pthread_mutex_lock(&server.thread_count_lock);
	server.thread_count++;
    	pthread_mutex_unlock(&server.thread_count_lock);
	
	woomera_set_flag(&server.master_connection, WFLAG_MONITOR_RUNNING);
	
	if (call_signal_connection_open(mcon, 
					mcon->cfg.local_ip, 
					mcon->cfg.local_port,
					mcon->cfg.remote_ip,
					mcon->cfg.remote_port) < 0) {
			log_printf(0, server.log, "Error: Opening MCON Socket [%d] %s\n", 
					mcon->socket,strerror(errno));
			exit(-1);
	}

	if (call_signal_connection_open(mconp,
					mconp->cfg.local_ip, 
					mconp->cfg.local_port,
					mconp->cfg.remote_ip,
					mconp->cfg.remote_port) < 0) {
			log_printf(0, server.log, "Error: Opening MCONP Socket [%d] %s\n", 
					mconp->socket,strerror(errno));
			exit(-1);
	}
	
	mcon->log = server.log;
	mconp->log = server.log;
	
	isup_exec_command(0, 
			  0, 
			  -1,
			  SIGBOOST_EVENT_SYSTEM_RESTART,
			  0);

	mcon->rxseq_reset=1;

	smg_get_current_priority(&policy,&priority);
	
	log_printf(1, server.log, "Open udp socket [%d] [%d]\n",
				 mcon->socket,mconp->socket);
	log_printf(1, server.log, "Monitor Thread Started (%i:%i)\n",policy,priority);
				
	
	while (woomera_test_flag(&server.master_connection, WFLAG_RUNNING) &&
		woomera_test_flag(&server.master_connection, WFLAG_MONITOR_RUNNING)) {
#if 0
		ss = waitfor_socket(server.mcon.socket, 1000, POLLERR | POLLIN);
#else
		ss = waitfor_2sockets(mcon->socket,
				      mconp->socket,
				      &a, &b, 1000);
#endif	
		
		if (ss > 0) {

			call_signal_event_t *event=NULL;
			int i=0;
		
			if (b) { 
mcon_retry_priority:
			    if ((event = call_signal_connection_readp(mconp,i))) {
				struct timeval current;
                               	struct timeval difftime;
                               	gettimeofday(&current,NULL);
                               	timersub (&current, &event->tv, &difftime);
                               	log_printf(3, server.log, "Socket Event P [%s] T=%d:%d\n",
                               	call_signal_event_id_name(event->event_id),
                                       	difftime.tv_sec, difftime.tv_usec);
                               	parse_ss7_event(mconp,event);
				if (++i < 10) {
					goto mcon_retry_priority;
				}
				
			    } else if (errno != EAGAIN) {
				ss=-1;
				log_printf(0, server.log, 
					"Error: Reading from Boost P Socket! (%i) %s\n",
					errno,strerror(errno));
				break;
			    }
			}
		
			i=0;

			if (a) {
mcon_retry:
			    if ((event = call_signal_connection_read(mcon,i))) {
      				struct timeval current;
                               	struct timeval difftime;
                               	gettimeofday(&current,NULL);
                               	timersub (&current, &event->tv, &difftime);
                               	log_printf(3, server.log, "Socket Event [%s] T=%d:%d\n",
                               			call_signal_event_id_name(event->event_id),
                                       		difftime.tv_sec, difftime.tv_usec);
                               	parse_ss7_event(mcon,event);

				if (++i < 50) {
					goto mcon_retry;
				}
			    } else if (errno != EAGAIN) {
				ss=-1;
				log_printf(0, server.log, 
					"Error: Reading from Boost Socket! (%i) %s\n",
					errno,strerror(errno));
				break;
			    }
			} 

		} 

		if (ss < 0){
			log_printf(0, server.log, "Thread Run: Select Socket Error!\n");
			break;
		}
		
	}
 
	log_printf(1, server.log, "Close udp socket [%d] [%d]\n", 
					mcon->socket,mconp->socket);
	call_signal_connection_close(&server.mcon);
	call_signal_connection_close(&server.mconp);

    	pthread_mutex_lock(&server.thread_count_lock);
	server.thread_count--;
    	pthread_mutex_unlock(&server.thread_count_lock);
	
	woomera_clear_flag(&server.master_connection, WFLAG_MONITOR_RUNNING);
	log_printf(0, server.log, "Monitor Thread Ended\n");
	
	return NULL;
}

static void woomera_loop_thread_run(struct woomera_interface *woomera)
{
	int err=launch_media_thread(woomera);
	if (err) {
		log_printf(0, server.log, "Failed to start loop media thread\n");
		woomera_set_flag(woomera, 
				(WFLAG_HANGUP|WFLAG_MEDIA_END));
		woomera_clear_flag(woomera, WFLAG_RUNNING);
		return;
	}

	while (woomera_test_flag(&server.master_connection, WFLAG_RUNNING) &&
               woomera_test_flag(&server.master_connection, WFLAG_MONITOR_RUNNING) &&
	       !woomera_test_flag(woomera, WFLAG_MEDIA_END) &&
	       !woomera_test_flag(woomera, WFLAG_HANGUP)) {
	
		sleep(1);				
		continue;
		
	}
		
	woomera_clear_flag(woomera, WFLAG_RUNNING);

	log_printf(2, server.log, "Woomera Session: For Loop Test exiting %s\n",woomera->interface);
	


	return;
}

static void *woomera_thread_run(void *obj)
{
    struct woomera_interface *woomera = obj;
    struct woomera_message wmsg;
    struct woomera_event wevent;
    char *event_string;
    int mwi;
    int err;
    int policy=0, priority=0;
    int span = -1, chan = -1;

    woomera_message_init(&wmsg);
	
    smg_get_current_priority(&policy,&priority);

    log_printf(2, server.log, "WOOMERA session started (ptr=%p : loop=%i)(%i:%i) Index=%i\n", 
    			woomera,woomera->loop_tdm,policy,priority, woomera->index);

    pthread_mutex_lock(&server.thread_count_lock);
    server.thread_count++;
    pthread_mutex_unlock(&server.thread_count_lock);

    pthread_mutex_init(&woomera->queue_lock, NULL);	
    pthread_mutex_init(&woomera->ms_lock, NULL);	
    pthread_mutex_init(&woomera->dtmf_lock, NULL);	
    pthread_mutex_init(&woomera->vlock, NULL);	
    pthread_mutex_init(&woomera->flags_lock, NULL);	

    if (woomera->loop_tdm) {
    	woomera_loop_thread_run(woomera);
    	goto woomera_session_close;
    }
    
    err=socket_printf(woomera->socket,
				  "EVENT HELLO Sangoma Media Gateway%s"
				  "Supported-Protocols: TDM%s"
				  "Version: %s%s"
				  "Remote-Address: %s%s"
				  "Remote-Port: %d%s"
				  "Raw-Format: %s%s",
				  WOOMERA_LINE_SEPERATOR,
				  WOOMERA_LINE_SEPERATOR,
				  SMG_VERSION, WOOMERA_LINE_SEPERATOR,
				  inet_ntoa(woomera->addr.sin_addr), WOOMERA_LINE_SEPERATOR,
				  ntohs(woomera->addr.sin_port), WOOMERA_LINE_SEPERATOR,
				  server.hw_coding?"ALAW":"ULAW", WOOMERA_RECORD_SEPERATOR
				  );

    if (err) {
    	log_printf(0, server.log, "Woomera session socket failure! (ptr=%p)\n",
			woomera);
	woomera_clear_flag(woomera, WFLAG_RUNNING);
	goto woomera_session_close;
    }
	
    while (woomera_test_flag(&server.master_connection, WFLAG_RUNNING) &&
           woomera_test_flag(&server.master_connection, WFLAG_MONITOR_RUNNING) &&
	   woomera_test_flag(woomera, WFLAG_RUNNING) &&
	   !woomera_test_flag(woomera, WFLAG_MEDIA_END) &&
	   !woomera_test_flag(woomera, WFLAG_HANGUP) && 
	   !master_reset) {
					
	   
	   mwi = woomera_message_parse(woomera, &wmsg, WOOMERA_HARD_TIMEOUT);
	   if (mwi >= 0) {

		if (mwi) {
			interpret_command(woomera, &wmsg);			
		} else if (woomera_test_flag(woomera, WFLAG_EVENT)){
			while ((event_string = dequeue_event(woomera))) {
				if (socket_printf(woomera->socket, "%s", event_string)) {
					woomera_set_flag(woomera, WFLAG_MEDIA_END);	
					woomera_clear_flag(woomera, WFLAG_RUNNING);
					smg_free(event_string);
					log_printf(4, server.log, 
						"WOOMERA session (ptr=%p) print string error\n",
							woomera);
					break;
				}
				smg_free(event_string);
			}
			woomera_clear_flag(woomera, WFLAG_EVENT);
		}

		if (woomera->timeout > 0 && time(NULL) >= woomera->timeout) {
		
			/* Sent the hangup only after we sent a NACK */	

			log_printf(2, server.log, 
				"WOOMERA session (ptr=%p) Call Timedout ! [%s] (Timeout=%d)\n",
					 woomera,woomera->interface,woomera->timeout);
		
			/* Let the Index check below send a NACK */
			if (woomera->index) {
				woomera_set_flag(woomera, WFLAG_HANGUP);
			}
			break;
		}

	  } else  {
	  	 log_printf(3, server.log, "WOOMERA session (ptr=%p) [%s] READ MSG Error %i \n", 
    			woomera,woomera->interface,mwi);
		 break;
	  }
		
   }
    
woomera_session_close:
	
    	log_printf(2, server.log, "WOOMERA session (ptr=%p) is dying [%s]:  SR=%d WR=%d WF=0x%04X\n", 
    			woomera,woomera->interface,
			woomera_test_flag(&server.master_connection, WFLAG_RUNNING),
			woomera_test_flag(woomera, WFLAG_RUNNING),
			woomera->flags);


    	if (woomera_test_flag(woomera, WFLAG_MEDIA_RUNNING)) {
		woomera_set_flag(woomera, WFLAG_MEDIA_END);
		while(woomera_test_flag(woomera, WFLAG_MEDIA_RUNNING)) {
			usleep(100);
			sched_yield();
		}
    	}


	/***********************************************
         * Identify the SPAN CHAN to be used below
	 ***********************************************/

        chan = woomera->chan;
        span = woomera->span;


	if (!woomera_test_flag(woomera, WFLAG_HANGUP)) {
		
		/* The call was not HUNGUP. This is the last check,
		   If the call is valid, hungup the call if the call
		   was never up the keep going */
		

		if (smg_validate_span_chan(span,chan) == 0) {	

			if (!woomera->index) {

				if (autoacm || woomera_test_flag(woomera,WFLAG_CALL_ACKED)) {

					woomera_set_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK);
					isup_exec_command(span, 
							chan, 
							-1,
							SIGBOOST_EVENT_CALL_STOPPED,
							woomera->q931_rel_cause_tosig);
					woomera_set_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK_SENT);
				
	
					log_printf(3, woomera->log, "Woomera Sent SIGBOOST_EVENT_CALL_STOPPED [w%dg%d] [%s] ptr=%p\n", 
							span+1, chan+1,woomera->interface,woomera);
				} else {
					
					woomera_set_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK);
					isup_exec_command(span, 
							chan, 
							-1,
							SIGBOOST_EVENT_CALL_START_NACK,
							woomera->q931_rel_cause_tosig);
					woomera_set_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK_SENT);
		
					log_printf(3, woomera->log, "Woomera Sent SIGBOOST_EVENT_CALL_START_NACK [w%dg%d] [%s] ptr=%p\n", 
							span+1, chan+1,woomera->interface,woomera);
				}
			} else {
				log_printf(0, woomera->log, "Woomera Not Sent CALL STOPPED - Instead NACK [w%dg%d] [%s] ptr=%p\n", 
						span+1, chan+1,woomera->interface,woomera);
				
			}
		}else{
			/* This can happend if an outgoing call times out
			   or gets hungup before it gets acked. Its not a
			   failure */
			if (!woomera->index) {

				/* In this case we really failed to tx STOP */
				log_printf(0, woomera->log, "FAILED: Woomera (R) SIGBOOST_EVENT_CALL_STOPPED [w%dg%d] [%s] Index=%d ptr=%p\n", 
					span+1, chan+1, woomera->interface, woomera->index, woomera);
			}
		}

		woomera_set_flag(woomera, WFLAG_HANGUP);

	}

woo_re_hangup:

	/* We must send a STOP ACK to boost telling it that we are done */
	if (woomera_test_flag(woomera, WFLAG_HANGUP_ACK)) {
	
		/* SMG received a HANGUP from boost.
 		   We must now send back the ACK to the HANGUP.
		   Boost will not release channel until we
		   ACK the hangup */
	
		if (smg_validate_span_chan(span,chan) == 0) {	

			isup_exec_command(span, 
			  		  chan, 
			  		  -1,
			  		  SIGBOOST_EVENT_CALL_STOPPED_ACK,
					  woomera->q931_rel_cause_tosig);
			
			log_printf(3, woomera->log, 
				"Sent (Ack) to SIGBOOST_EVENT_CALL_STOPPED_ACK [w%dg%d] [%s] ptr=%p\n", 
						span+1,chan+1,woomera->interface,woomera);

		}else{
			/* This should never happen! If it does
			   we broke protocol */
			log_printf(0, woomera->log, 
				"FAILED: Woomera (R) SIGBOOST_EVENT_CALL_STOPPED_ACK [w%dg%d] [%s] Index=%d ptr=%p\n", 
					span+1, chan+1, woomera->interface, woomera->index, woomera);
		}

		woomera_clear_flag(woomera, WFLAG_HANGUP_ACK);
		woomera_set_flag(woomera, WFLAG_HANGUP);
	}
	
	if (woomera_test_flag(woomera, WFLAG_HANGUP_NACK_ACK)) {
	
		/* SMG received a NACK from boost during call startup.
 		   We must now send back the ACK to the NACK.
		   Boost will not release channel until we
		   ACK the NACK */

		if (smg_validate_span_chan(span,chan) == 0) {	
		
			isup_exec_command(span, 
			  		  chan, 
			  		  -1,
			  		  SIGBOOST_EVENT_CALL_START_NACK_ACK,
					  woomera->q931_rel_cause_tosig);
			
			log_printf(3, woomera->log, 
				"Sent (Nack Ack) to SIGBOOST_EVENT_CALL_START_NACK_ACK [w%dg%d] [%s] ptr=%p\n", 
						span+1,chan+1,woomera->interface,woomera);

			woomera->index=0;

		} else if (woomera->index) {
			isup_exec_command(0, 
			  		  0, 
			  		  woomera->index,
			  		  SIGBOOST_EVENT_CALL_START_NACK_ACK,
					  woomera->q931_rel_cause_tosig);

			woomera->index=0;

		} else {
			log_printf(0, woomera->log, 
				"FAILED: Sent (Nack Ack) SIGBOOST_EVENT_CALL_START_NACK_ACK [w%dg%d] [%s] Index=%d ptr=%p\n", 
					span+1, chan+1, woomera->interface, woomera->index, woomera);
		}

		woomera_clear_flag(woomera, WFLAG_HANGUP_NACK_ACK);
		
	} 
	
	if (woomera->index) { 
	
		int index = woomera->index;
		

		new_woomera_event_printf(&wevent, "EVENT HANGUP %s%s"
                                                  "Unique-Call-Id: %s%s"
                                                  "Timeout: %ld%s"
                                                  "Cause: %s%s"
						  "Q931-Cause-Code: %d%s",
                                                 woomera->interface,
                                                 WOOMERA_LINE_SEPERATOR,
						 woomera->session,
                                                 WOOMERA_LINE_SEPERATOR,
                                                 woomera->timeout,
                                                 WOOMERA_LINE_SEPERATOR,
                                                 q931_rel_to_str(18),
                                                 WOOMERA_LINE_SEPERATOR,
						 18,
                                                 WOOMERA_RECORD_SEPERATOR
                                                 );
                enqueue_event(woomera, &wevent,EVENT_FREE_DATA);

		while ((event_string = dequeue_event(woomera))) {
                        socket_printf(woomera->socket, "%s", event_string);
                        smg_free(event_string);
                }

		if (peek_from_holding_tank(index)) {
			
			woomera_set_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK);
			isup_exec_command(0, 
					0, 
					index,
					SIGBOOST_EVENT_CALL_START_NACK,
					0);
			woomera_set_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK_SENT);		
	
			log_printf(2, woomera->log, 
				"Sent SIGBOOST_EVENT_CALL_START_NACK [Setup ID: %d] .. WAITING FOR NACK ACK\n", 
			 	index);
		} else {
			log_printf(1, woomera->log, 
				"Error Failed to Sent SIGBOOST_EVENT_CALL_START_NACK [Setup ID: %d] - index stale!\n", 
			 	index);
		}
	}		
	
	if (woomera_test_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK)) {
		int timeout_cnt=0;
		int overall_cnt=0;
		
		/* SMG sent NACK to boost, however we have to wait
		   for boost to give us the ACK back before we
		   release resources. */
		
		while (woomera_test_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK)) {
			timeout_cnt++;
			if (timeout_cnt > 4000) {  //30sec timeout
				timeout_cnt=0;
				overall_cnt++;
				log_printf(0, woomera->log, 
				"Waiting for NACK ACK [Setup ID: %d] ... \n", 
					woomera->index);
			}		

			if (overall_cnt > 10) { //300sec timeotu
				woomera_clear_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK);
				break;
			}

			if (!woomera_test_flag(&server.master_connection, WFLAG_RUNNING)) {
				break;
			}

			usleep(5000);
			sched_yield();
		}

		woomera_clear_flag(woomera, WFLAG_WAIT_FOR_NACK_ACK);
		if (overall_cnt > 10) {		
			log_printf(0, woomera->log, 
				"Waiting for NACK ACK [Setup ID: %d] .. TIMEOUT on NACK ACK\n", 
				index);
		
		} else {
			log_printf(2, woomera->log, 
				"Waiting for NACK ACK [Setup ID: %d] .. GOT NACK ACK\n", 
				index);

		}
	}
		
    	if (woomera_test_flag(woomera, WFLAG_EVENT)){
		while ((event_string = dequeue_event(woomera))) {
			socket_printf(woomera->socket, "%s", event_string);
			smg_free(event_string);
		}
		woomera_clear_flag(woomera, WFLAG_EVENT);
    	}


       if (woomera_test_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK)) {
                int timeout_cnt=0;	
		int overall_cnt=0;

		/* SMG sent HANGUP to boost, however we have to wait
		   for boost to give us the ACK back before we
		   release resources. */

                log_printf(2, woomera->log,
                              "Waiting for STOPPED ACK [%s] [id=%i]... \n",
                                         woomera->interface,woomera->index_hold);

                while (woomera_test_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK)) {
                        timeout_cnt++;
                        if (timeout_cnt > 4000) {  //10sec timeout
				timeout_cnt=0;
				overall_cnt++;
                                log_printf(0, woomera->log,
                                       "Waiting for STOPPED ACK [%s] [id=%i] ... \n",
                                                woomera->interface,woomera->index_hold);
                        }

			if (overall_cnt > 10) { //100sec 
                		woomera_clear_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK);
				break;
			}


			if (!woomera_test_flag(&server.master_connection, WFLAG_RUNNING) || 
			    server.process_table[span][chan].dev != woomera) {
                		woomera_clear_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK);
				break;
			}
		
			usleep(5000);
                        sched_yield();
                }
                		
		woomera_clear_flag(woomera, WFLAG_WAIT_FOR_STOPPED_ACK);

		if (overall_cnt > 10) {
			log_printf(0, woomera->log,
                             		"Wait TIMEDOUT on STOPPED ACK [%s] [id=%i]... \n",
                                       	 woomera->interface,woomera->index_hold);

		} else {
			log_printf(2, woomera->log,
                          		"Wait GOT STOPPED ACK [%s] [id=%i]... \n",
                                       	woomera->interface,woomera->index_hold);
		}
        }
	
	/***************************************************** 
	 * We must wait for WFLAG_WAIT_FOR_STOPPED_ACK here
  	 * so that STOP_ACK can access the woomera 
	 *****************************************************/
		
	if (smg_validate_span_chan(span,chan) == 0) {	
                log_printf(2, woomera->log,
                              "WOOMERA Clearing Processs Table ... \n",
                                         woomera->interface);
		pthread_mutex_lock(&server.process_lock);
    		if (server.process_table[span][chan].dev == woomera){
			server.process_table[span][chan].dev = NULL;
			memset(server.process_table[span][chan].session,0,SMG_SESSION_NAME_SZ);
		}
		pthread_mutex_unlock(&server.process_lock);
	}

#if 0	
//Used for testing
	if (1) {
		int chan = woomera->chan;
		int span = woomera->span;
		if (smg_validate_span_chan(span,chan) == 0) {	
			pthread_mutex_lock(&server.process_lock);
			/* This is possible in case media thread dies on startup */
			
    			if (server.process_table[span][chan]){
				log_printf(0, server.log, 
				"Sanity Span Chan Still in use: [w%dg%d] [%s] Index=%d ptr=%p\n", 
					span+1, chan+1, woomera->interface, woomera->index, woomera);
				//server.process_table[span][chan] = NULL;
			}
			pthread_mutex_unlock(&server.process_lock);
		}
	}
#endif
	
    	usleep(3000000);
	
	/* Sanity Check */
	if (woomera_test_flag(woomera, WFLAG_HANGUP_ACK)) {
    		log_printf(0, woomera->log, 
			"Woomera MISSED HANGUP ACK: Retry HANGUP ACK\n");
		goto woo_re_hangup;
	}
	if (woomera_test_flag(woomera, WFLAG_HANGUP_NACK_ACK)) {
    		log_printf(0, woomera->log, 
			"Woomera MISSED HANGUP ACK: Retry HANGUP NACK ACK\n");
		goto woo_re_hangup;
	}
	
	/* This is where we actually pull the index
	 * out of the tank. We had to keep the tank
	 * value until the end of the call. Tank is only
	 * used on outgoing calls. */
	if (woomera->index_hold >= 1) {
		clear_from_holding_tank(woomera->index_hold, woomera);
		woomera->index_hold=0;
	}
	
    	log_printf(2, woomera->log, "Woomera Thread Finished %u\n", (unsigned long) woomera->thread);
   	close_socket(&woomera->socket);
   	woomera->socket=-1;

    	/* delete queue */
   	while ((event_string = dequeue_event(woomera))) {
		smg_free(event_string);
    	}
	
	if (woomera_test_flag(woomera, WFLAG_LISTENING)) {
		del_listener(woomera);
    	}

    	log_printf(2, server.log, "WOOMERA session for [%s] stopped (ptr=%p)\n", 
    			woomera->interface,woomera);

	if (woomera_test_flag(woomera, WFLAG_MASTER_DEV)) {
		log_printf(0,server.log,"MASTER Thread Stopped (ptr=%p)\n",woomera);
		master_reset=0;
	}

    
   	pthread_mutex_destroy(&woomera->queue_lock);
    	pthread_mutex_destroy(&woomera->ms_lock);
	pthread_mutex_destroy(&woomera->dtmf_lock);
	pthread_mutex_destroy(&woomera->vlock);
    	pthread_mutex_destroy(&woomera->flags_lock);
    	woomera_set_raw(woomera, NULL);
    	woomera_set_interface(woomera, NULL);

	woomera_message_clear(&wmsg);

    	smg_free(woomera);
    	pthread_mutex_lock(&server.thread_count_lock);
    	server.call_count--;
    	server.thread_count--;
    	pthread_mutex_unlock(&server.thread_count_lock);

	pthread_exit(NULL);
    	return NULL;
}


static int launch_woomera_thread(struct woomera_interface *woomera) 
{
    int result = 0;
    pthread_attr_t attr;

    result = pthread_attr_init(&attr);
    //pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    //pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, MGD_STACK_SIZE);

    woomera_set_flag(woomera, WFLAG_RUNNING);
    result = pthread_create(&woomera->thread, &attr, woomera_thread_run, woomera);
    if (result) {
	log_printf(0, server.log, "%s: Error: Creating Thread! (%i) %s\n",
			 __FUNCTION__,result,strerror(errno));
    	woomera_clear_flag(woomera, WFLAG_RUNNING);
    }
    pthread_attr_destroy(&attr);
    
    return result;
}

static int launch_monitor_thread(void) 
{
    pthread_attr_t attr;
    int result = 0;
    struct sched_param param;

    param.sched_priority = 10;
    result = pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, MGD_STACK_SIZE);

    result = pthread_attr_setschedparam (&attr, &param);
    
    log_printf(0,server.log,"%s: Old Priority =%i res=%i \n",__FUNCTION__,
			 param.sched_priority,result);


    woomera_set_flag(&server.master_connection, WFLAG_MONITOR_RUNNING);
    result = pthread_create(&server.monitor_thread, &attr, monitor_thread_run, NULL);
    if (result) {
	log_printf(0, server.log, "%s: Error: Creating Thread! %s\n",
			 __FUNCTION__,strerror(errno));
	 woomera_clear_flag(&server.master_connection, WFLAG_MONITOR_RUNNING);
    } 
    pthread_attr_destroy(&attr);

    return result;
}


#ifdef WP_HPTDM_API
static void *hp_tdmapi_span_run(void *obj)
{
    hp_tdm_api_span_t *span = obj;
    int err; 
    
    log_printf(0,server.log,"Starting %s span!\n",span->ifname);
    
    while (woomera_test_flag(&server.master_connection, WFLAG_RUNNING) &&
           woomera_test_flag(&server.master_connection, WFLAG_MONITOR_RUNNING)) {
    	
	if (!span->run_span) {
		break;
	}
	
	err = span->run_span(span);
	if (err) {
		break;
	}
	
    }

    if (span->close_span) {
    	span->close_span(span);
    }
        
    sleep(3);
    log_printf(0,server.log,"Stopping %s span!\n",span->ifname);
    
    pthread_exit(NULL);	    
}


static int launch_hptdm_api_span_thread(int span)
{
    pthread_attr_t attr;
    int result = 0;
    struct sched_param param;

    param.sched_priority = 5;
    result = pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, MGD_STACK_SIZE);

    result = pthread_attr_setschedparam (&attr, &param);
    
    result = pthread_create(&server.monitor_thread, &attr, hp_tdmapi_span_run, &hptdmspan[span]);
    if (result) {
	 log_printf(0, server.log, "%s: Error: Creating Thread! %s\n",
			 __FUNCTION__,strerror(errno));
    } 
    pthread_attr_destroy(&attr);

    return result;
}
#endif

static int configure_server(void)
{
    struct woomera_config cfg;
    char *var, *val;
    int cnt = 0;

    server.dtmf_intr_ch = -1;

    if (!woomera_open_file(&cfg, server.config_file)) {
		log_printf(0, server.log, "open of %s failed\n", server.config_file);
		return 0;
    }
	
    while (woomera_next_pair(&cfg, &var, &val)) {
		if (!strcasecmp(var, "boost_local_ip")) {
			strncpy(server.mcon.cfg.local_ip, val,
				 sizeof(server.mcon.cfg.local_ip) -1);
			strncpy(server.mconp.cfg.local_ip, val,
				 sizeof(server.mconp.cfg.local_ip) -1);
			cnt++;
		} else if (!strcasecmp(var, "boost_local_port")) {
			server.mcon.cfg.local_port = atoi(val);
			server.mconp.cfg.local_port = 
				server.mcon.cfg.local_port+1;
			cnt++;
		} else if (!strcasecmp(var, "boost_remote_ip")) {
			strncpy(server.mcon.cfg.remote_ip, val,
				 sizeof(server.mcon.cfg.remote_ip) -1);
			strncpy(server.mconp.cfg.remote_ip, val,
				 sizeof(server.mconp.cfg.remote_ip) -1);
			cnt++;
		} else if (!strcasecmp(var, "boost_remote_port")) {
			server.mcon.cfg.remote_port = atoi(val);
			server.mconp.cfg.remote_port = 
				server.mcon.cfg.remote_port+1;
			cnt++;
		} else if (!strcasecmp(var, "logfile_path")) {
			if (!server.logfile_path) {
				server.logfile_path = smg_strdup(val);
			}
		} else if (!strcasecmp(var, "woomera_port")) {
			server.port = atoi(val);
		} else if (!strcasecmp(var, "debug_level")) {
			server.debug = atoi(val);
		} else if (!strcasecmp(var, "out_tx_test")) {
		        server.out_tx_test = atoi(val);
		} else if (!strcasecmp(var, "loop_trace")) {
		        server.loop_trace = atoi(val);
		} else if (!strcasecmp(var, "rxgain")) {
		        server.rxgain = atoi(val);
		} else if (!strcasecmp(var, "txgain")) {
		        server.txgain = atoi(val);
		} else if (!strcasecmp(var, "dtmf_on_duration")){
			server.dtmf_on = atoi(val);
		} else if (!strcasecmp(var, "dtmf_off_duration")){
			server.dtmf_off = atoi(val);
		} else if (!strcasecmp(var, "dtmf_inter_ch_duration")){
			server.dtmf_intr_ch = atoi(val);
		} else if (!strcasecmp(var, "strip_cid_non_digits")){
			server.strip_cid_non_digits = atoi(val);
		} else if (!strcasecmp(var, "max_calls")) {
			int max = atoi(val);
			if (max > 0) {
				server.max_calls = max;
			}
		} else if (!strcasecmp(var, "autoacm")) {
			int max = atoi(val);
			if (max >= 0) {
				autoacm=max;
			}			

		} else if (!strcasecmp(var, "media_ip")) {
			strncpy(server.media_ip, val, sizeof(server.media_ip) -1);
		} else {
			log_printf(0, server.log, "Invalid Option %s at line %d!\n", var, cfg.lineno);
		}
    }

    /* Post initialize */
    if (server.dtmf_on == 0){ 
	server.dtmf_on=SMG_DTMF_ON;
    }
    if (server.dtmf_off == 0) {
	server.dtmf_off=SMG_DTMF_OFF;
    }
    if (server.dtmf_intr_ch == -1) {
	server.dtmf_intr_ch = 0;
    }
    server.dtmf_size=(server.dtmf_on+server.dtmf_off)*10*2;

    log_printf(0,server.log, "DTMF On=%i Off=%i IntrCh=%i Size=%i\n",
		server.dtmf_on,server.dtmf_off,server.dtmf_intr_ch,server.dtmf_size);

    woomera_close_file(&cfg);
    return cnt == 4 ? 1 : 0;
}



static int main_thread(void)
{

    struct sockaddr_in sock_addr, client_addr;
    struct woomera_interface *new_woomera;
    int client_sock = -1, pid = 0;
    unsigned int len = 0;
    FILE *tmp;

    if ((server.master_connection.socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		log_printf(0, server.log, "socket() failed\n");
		return 1;
    }
	
    memset(&sock_addr, 0, sizeof(sock_addr));	/* Zero out structure */
    sock_addr.sin_family = AF_INET;				   /* Internet address family */
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    sock_addr.sin_port = htons(server.port);	  /* Local port */

    /* Bind to the local address */
    if (bind(server.master_connection.socket, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
		log_printf(0, server.log, "bind(%d) failed\n", server.port);
		return 1;
    }
	 
    /* Mark the socket so it will listen for incoming connections */
    if (listen(server.master_connection.socket, MAXPENDING) < 0) {
		log_printf(0, server.log, "listen() failed\n");
		return 1;
    }

    if ((pid = get_pid_from_file(PIDFILE))) {
		log_printf(0, stderr, "pid %d already exists.\n", pid);
		exit(0);
    }

    if (!(tmp = safe_fopen(PIDFILE, "w"))) {
		log_printf(0, stderr, "Error creating pidfile %s\n", PIDFILE);
		return 1;
    } else {
		fprintf(tmp, "%d", getpid());
		fclose(tmp);
		tmp = NULL;
    }

    no_nagle(server.master_connection.socket);

#if 0
    if (1) {
	int span,chan;
   	call_signal_event_t event;
#if 0	
	span=1;
	chan=30;
	event.span=span;
	event.chan=chan;
    	launch_woomera_loop_thread(&event);
#else
	for (span=0;span<8;span++) {
		for (chan=0;chan<31;chan++) {
			event.span=span;
			event.chan=chan;
    			launch_woomera_loop_thread(&event);
		}
	}
#endif
    }
#endif

#ifdef WP_HPTDM_API
    if (1) {
	int span;
	for (span=0;span<16;span++) {
		hptdmspan[span] = sangoma_hptdm_api_span_init(span);
		if (!hptdmspan[span]) {
			break;
		} else {
			log_printf(0, server.log, "HP TDM API Span: %d configured...\n", 
					span);
			launch_hptdm_api_span_thread(span);
		}
	}
    }	
#endif
    
    log_printf(1, server.log, "Main Process Started: Woomera Ready port: %d\n", server.port);

    while (woomera_test_flag(&server.master_connection, WFLAG_RUNNING) &&
           woomera_test_flag(&server.master_connection, WFLAG_MONITOR_RUNNING)) { 

		/* Set the size of the in-out parameter */
		len = sizeof(client_addr);

		/* Wait for a client to connect */
		if ((client_sock = accept(server.master_connection.socket, (struct sockaddr *) &client_addr, &len)) < 0) {
			log_printf(0, server.log, "accpet() failed\n");
			return 1;
		}

		if ((new_woomera = new_woomera_interface(client_sock, &client_addr, len))) {
			log_printf(2, server.log, "Starting Thread for New Connection %s:%d Sock=%d\n",
					   inet_ntoa(new_woomera->addr.sin_addr),
					   ntohs(new_woomera->addr.sin_port),
					   client_sock);
    			pthread_mutex_lock(&server.thread_count_lock);
			server.call_count++;
    			pthread_mutex_unlock(&server.thread_count_lock);
			if (launch_woomera_thread(new_woomera)) {
			         socket_printf(new_woomera->socket, 
						"501 call was cancelled!%s", 
						WOOMERA_RECORD_SEPERATOR);
				
				 close_socket(&new_woomera->socket);
   				 new_woomera->socket=-1;
				 smg_free(new_woomera);
			}
		} else {
			log_printf(0, server.log, "Critical ERROR: memory/socket error\n");
		}
    }

    log_printf(1, server.log, "Main Process End\n");

    return 0;
}

static int do_ignore(int sig)
{
	sdla_memdbg_free(0);
    	return 0;
}

static int do_shut(int sig)
{
    woomera_clear_flag(&server.master_connection, WFLAG_RUNNING);
    close_socket(&server.master_connection.socket);
    log_printf(1, server.log, "Caught SIG %d, Closing Master Socket!\n", sig);
    return 0;
}

static int sangoma_tdm_init (int span)
{
#ifdef LIBSANGOMA_GET_HWCODING
    wanpipe_tdm_api_t tdm_api;
    int fd=sangoma_open_tdmapi_span(span);
    if (fd < 0 ){
        return -1;
    } else {
        server.hw_coding=sangoma_tdm_get_hw_coding(fd,&tdm_api);
        close_socket(&fd);
    }
#else
#error "libsangoma missing hwcoding feature: not up to date!"
#endif
    return 0;
}


static int woomera_startup(int argc, char **argv)
{
    int x = 0, pid = 0, bg = 0;
    char *cfg=NULL, *debug=NULL, *arg=NULL;

    while((arg = argv[x++])) {

		if (!strcasecmp(arg, "-hup")) {
			if (! (pid = get_pid_from_file(PIDFILE))) {
				log_printf(0, stderr, "Error reading pidfile %s\n", PIDFILE);
				exit(1);
			} else {
				log_printf(0, stderr, "Killing PID %d\n", pid);
				kill(pid, SIGHUP);
				sleep(1);
				exit(0);
			}
		
		 } else if (!strcasecmp(arg, "-term") || !strcasecmp(arg, "--term")) {
                        if (! (pid = get_pid_from_file(PIDFILE))) {
                                log_printf(0, stderr, "Error reading pidfile %s\n", PIDFILE);
                                exit(1);
                        } else {
                                log_printf(0, stderr, "Killing PID %d\n", pid);
                                kill(pid, SIGTERM);
                                unlink(PIDFILE);
				sleep(1);
                                exit(0);
                        }	

		} else if (!strcasecmp(arg, "-version")) {
                        fprintf(stdout, "\nSangoma Media Gateway: Version %s\n\n", SMG_VERSION);
			exit(0);
			
		} else if (!strcasecmp(arg, "-help")) {
			fprintf(stdout, "%s\n%s [-help] | [ -version] | [-hup] | [-wipe] | [[-bg] | [-debug <level>] | [-cfg <path>] | [-log <path>]]\n\n", WELCOME_TEXT, argv[0]);
			exit(0);
		} else if (!strcasecmp(arg, "-wipe")) {
			unlink(PIDFILE);
		} else if (!strcasecmp(arg, "-bg")) {
			bg = 1;

		} else if (!strcasecmp(arg, "-g")) {
                        coredump = 1;

		} else if (!strcasecmp(arg, "-debug")) {
			if (argv[x] && *(argv[x]) != '-') {
				debug = argv[x++];
			}
		} else if (!strcasecmp(arg, "-cfg")) {
			if (argv[x] && *(argv[x]) != '-') {
				cfg = argv[x++];
			}
		} else if (!strcasecmp(arg, "-log")) {
			if (argv[x] && *(argv[x]) != '-') {
				server.logfile_path = smg_strdup(argv[x++]);
			}
		} else if (*arg == '-') {
			log_printf(0, stderr, "Unknown Option %s\n", arg);
			fprintf(stdout, "%s\n%s [-help] | [-hup] | [-wipe] | [[-bg] | [-debug <level>] | [-cfg <path>] | [-log <path>]]\n\n", WELCOME_TEXT, argv[0]);
			exit(1);
		}
    }

    if (1){
	int spn; 
    	for (spn=1;spn<=WOOMERA_MAX_SPAN;spn++) {
    		if (sangoma_tdm_init(spn) == 0) {
			break;
		}
    	}
	if (spn>WOOMERA_MAX_SPAN) {
        	printf("\nError: Failed to access a channel on spans 1-16\n");
        	printf("         Please start Wanpipe TDM API drivers\n");
		return 0;
	}
    }

    if (bg && (pid = fork())) {
		log_printf(0, stderr, "Backgrounding!\n");
		return 0;
    }

    q931_cause_setup();

    server.port = 42420;
    server.debug = 0;
    strcpy(server.media_ip, "127.0.0.1");
    server.next_media_port = WOOMERA_MIN_MEDIA_PORT;
    server.log = stdout;
    server.master_connection.socket = -1;
    server.master_connection.event_queue = NULL;
    server.config_file = cfg ? cfg : "/etc/sangoma_mgd.conf";
    pthread_mutex_init(&server.listen_lock, NULL);	
    pthread_mutex_init(&server.ht_lock, NULL);	
    pthread_mutex_init(&server.process_lock, NULL);
    pthread_mutex_init(&server.media_udp_port_lock, NULL);	
    pthread_mutex_init(&server.thread_count_lock, NULL);	
    pthread_mutex_init(&server.master_connection.queue_lock, NULL);	
    pthread_mutex_init(&server.master_connection.flags_lock, NULL);	
    pthread_mutex_init(&server.mcon.lock, NULL);	
	server.master_connection.chan = -1;
	server.master_connection.span = -1;

    if (!configure_server()) {
		log_printf(0, server.log, "configuration failed!\n");
		return 0;
    }

#ifndef USE_SYSLOG
    if (server.logfile_path) {
		if (!(server.log = safe_fopen(server.logfile_path, "a"))) {
			log_printf(0, stderr, "Error setting logfile %s!\n", server.logfile_path);
			server.log = stderr;
			return 0;
		}
    }
#endif
	
	
    if (debug) {
		server.debug = atoi(debug);
    }

    if (coredump) {
	struct rlimit l;
	memset(&l, 0, sizeof(l));
	l.rlim_cur = RLIM_INFINITY;
	l.rlim_max = RLIM_INFINITY;
	if (setrlimit(RLIMIT_CORE, &l)) {
		log_printf(0, stderr,  "Warning: Failed to disable core size limit: %s\n", 
			strerror(errno));
	}
    }

#ifdef __LINUX__
    if (geteuid() && coredump) {
    	if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) < 0) {
		log_printf(0, stderr,  "Warning: Failed to disable core size limit for non-root: %s\n", 
			strerror(errno));
        }
    }
#endif



    (void) signal(SIGINT,(void *) do_shut);
    (void) signal(SIGPIPE,(void *) do_ignore);
    (void) signal(SIGUSR1,(void *) do_ignore);	
    (void) signal(SIGHUP,(void *) do_shut);

   
    woomera_set_flag(&server.master_connection, WFLAG_RUNNING);
    if (launch_monitor_thread()) {
    	woomera_clear_flag(&server.master_connection, WFLAG_RUNNING);
	return 0;
    }
	
    fprintf(stderr, "%s", WELCOME_TEXT);
    log_printf(0, stderr, "Woomera STARTUP Complete. [AutoACM=%i SDigit=%i]\n",
		autoacm,server.strip_cid_non_digits);

    return 1;
}

static int woomera_shutdown(void) 
{
    char *event_string;
    int told = 0, loops = 0;

    close_socket(&server.master_connection.socket);
    pthread_mutex_destroy(&server.listen_lock);
    pthread_mutex_destroy(&server.ht_lock);	
    pthread_mutex_destroy(&server.process_lock);
    pthread_mutex_destroy(&server.media_udp_port_lock);
    pthread_mutex_destroy(&server.thread_count_lock);
    pthread_mutex_destroy(&server.master_connection.queue_lock);
    pthread_mutex_destroy(&server.master_connection.flags_lock);
    pthread_mutex_destroy(&server.mcon.lock);
    woomera_clear_flag(&server.master_connection, WFLAG_RUNNING);


    if (server.logfile_path) {
		smg_free(server.logfile_path);
		server.logfile_path = NULL;
    }

    /* delete queue */
    while ((event_string = dequeue_event(&server.master_connection))) {
		smg_free(event_string);
    }

    while(server.thread_count > 0) {
		loops++;

		if (loops % 1000 == 0) {
			told = 0;
		}

		if (loops > 10000) {
			log_printf(0, server.log, "Red Alert! threads did not stop\n");
			assert(server.thread_count == 0);
		}

		if (told != server.thread_count) {
			log_printf(1, server.log, "Waiting For %d thread%s.\n", 
					server.thread_count, server.thread_count == 1 ? "" : "s");
			told = server.thread_count;
		}
		ysleep(10000);
    }
    unlink(PIDFILE);
    log_printf(0, stderr, "Woomera SHUTDOWN Complete.\n");
    return 0;
}

int main(int argc, char *argv[]) 
{
    int ret = 0;
    
    mlockall(MCL_FUTURE);
    
    sdla_memdbg_init();

    server.hw_coding=0;

    openlog (ps_progname ,LOG_PID, LOG_LOCAL2);  
	
    if (! (ret = woomera_startup(argc, argv))) {
		exit(0);
    }
    ret = main_thread();

    woomera_shutdown();

    sdla_memdbg_free(1);

    return ret;
}

/** EMACS **
 * Local variables:
 * mode: C
 * c-basic-offset: 4
 * End:
 */

