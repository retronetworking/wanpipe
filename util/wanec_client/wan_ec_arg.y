
%{

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/ioctl.h>

#include "wan_ecmain.h"
#include <wanpipe_events.h>
#include "wanec_api.h"

extern wanec_client_t	*gl_ec_client;
extern int		action;
extern char		yytext[];
extern char		**targv;
extern unsigned 	offset;
static int		start_channel = 0, range = 0;
extern int		gl_err;

extern int yylex(void);

unsigned long convert_str(char* str, int type);
static int is_channel(unsigned long);
void yyerror(char* msg);

%}

%union {
#define YYSTYPE YYSTYPE
	char*		str;
	unsigned long	val;
}


%token LOAD_TOKEN
%token UNLOAD_TOKEN
%token CONFIG_TOKEN
%token RELEASE_TOKEN
%token KILL_TOKEN
%token ENABLE_TOKEN
%token DISABLE_TOKEN
%token BYPASS_ENABLE_TOKEN
%token BYPASS_DISABLE_TOKEN
%token MODE_NORMAL_TOKEN
%token MODE_POWERDOWN_TOKEN
%token DTMF_ENABLE_TOKEN
%token DTMF_DISABLE_TOKEN
%token STATS_TOKEN
%token STATS_FULL_TOKEN
%token ALL_TOKEN
%token HELP_TOKEN
%token HELP1_TOKEN
%token MONITOR_TOKEN
%token MODIFY_TOKEN
%token TONE_LOAD_TOKEN
%token TONE_UNLOAD_TOKEN
%token PLAYOUT_START_TOKEN
%token PLAYOUT_STOP_TOKEN
%token DURATION_TOKEN
%token TEST_TOKEN

%token CHAR_STRING
%token DEC_CONSTANT
%token HEX_CONSTANT
%token DIAL_STRING

%%

start_args	: TEST_TOKEN
		  { action = WAN_EC_ACT_TEST; }
		| CHAR_STRING
		  { memcpy(gl_ec_client->devname, $<str>1, strlen($<str>1));
		    wanec_api_init(); }
				command
		;

command		: CONFIG_TOKEN
		  { gl_err = wanec_api_config(	gl_ec_client->devname,
						gl_ec_client->verbose); }
		| RELEASE_TOKEN
		  { wanec_api_release(	gl_ec_client->devname,
					gl_ec_client->verbose); }
		| KILL_TOKEN
		  { wanec_api_release(	gl_ec_client->devname,
					gl_ec_client->verbose); }
		| ENABLE_TOKEN		channel_map
		  { wanec_api_enable(	gl_ec_client->devname,
					gl_ec_client->channel_map,
					gl_ec_client->verbose); }
		| DISABLE_TOKEN		channel_map
		  { wanec_api_disable(	gl_ec_client->devname,
					gl_ec_client->channel_map,
					gl_ec_client->verbose); }
		| BYPASS_ENABLE_TOKEN	channel_map
		  { wanec_api_bypass(	gl_ec_client->devname,
					1,
					gl_ec_client->channel_map,
					gl_ec_client->verbose); }
		| BYPASS_DISABLE_TOKEN	channel_map
		  { wanec_api_bypass(	gl_ec_client->devname,
					0,
					gl_ec_client->channel_map,
					gl_ec_client->verbose); }
		| MODE_NORMAL_TOKEN	channel_map
		  { wanec_api_mode(	gl_ec_client->devname,
					1,
					gl_ec_client->channel_map,
					gl_ec_client->verbose); }
		| MODE_POWERDOWN_TOKEN	channel_map
		  { wanec_api_mode(	gl_ec_client->devname,
					0,
					gl_ec_client->channel_map,
					gl_ec_client->verbose); }
		| DTMF_ENABLE_TOKEN	channel_map
		  { wanec_api_dtmf(	gl_ec_client->devname,
					1,
					gl_ec_client->channel_map,
					WAN_EC_CHANNEL_PORT_SOUT,
					WAN_EC_TONE_PRESENT,
					gl_ec_client->verbose); }
		| DTMF_DISABLE_TOKEN	channel_map
		  { wanec_api_dtmf(	gl_ec_client->devname,
					0,
					gl_ec_client->channel_map,
					WAN_EC_CHANNEL_PORT_SOUT,
					WAN_EC_TONE_PRESENT,
					gl_ec_client->verbose); }
		| STATS_TOKEN		stats_debug_args
		  { wanec_api_stats(	gl_ec_client->devname,
					0,
					gl_ec_client->channel,
					0,
					gl_ec_client->verbose); }
		| STATS_FULL_TOKEN	stats_debug_args
		  { wanec_api_stats(	gl_ec_client->devname,
					1,
					gl_ec_client->channel,
					0,
					gl_ec_client->verbose); }
		| TONE_LOAD_TOKEN	CHAR_STRING
		  {  wanec_api_tone_load(	gl_ec_client->devname,
						$<str>2,
						gl_ec_client->verbose); }
		| TONE_UNLOAD_TOKEN	DEC_CONSTANT
		  { wanec_api_tone_unload(	gl_ec_client->devname,
						$<val>2,
						gl_ec_client->verbose); }
		| PLAYOUT_START_TOKEN	DEC_CONSTANT
		  { gl_ec_client->channel = $<val>2; }
					DEC_CONSTANT
		  { wanec_api_playout(	gl_ec_client->devname,
					1,
					gl_ec_client->channel,
					$<val>4,
					gl_ec_client->verbose); }	
						playout_args
		| PLAYOUT_STOP_TOKEN	DEC_CONSTANT
		  { gl_ec_client->channel = $<val>2; }
					DEC_CONSTANT
		  { wanec_api_playout(	gl_ec_client->devname,
					0,
					gl_ec_client->channel,
					$<val>4,
					gl_ec_client->verbose); }	
		| MONITOR_TOKEN		stats_debug_args
		  { wanec_api_monitor(	gl_ec_client->devname,
					gl_ec_client->channel,
					gl_ec_client->verbose); }
		| MODIFY_TOKEN		channel_map
		  { wanec_api_mode(	gl_ec_client->devname,
					2,
					gl_ec_client->channel_map,
					gl_ec_client->verbose); }
		;

playout_args	: /* empty */
		| playout_args playout_arg
		;
		
playout_arg	: DURATION_TOKEN DEC_CONSTANT
		;
		
channel_map	: ALL_TOKEN
		  { gl_ec_client->channel_map = 0xFFFFFFFF; }
		| channel_list
		;

channel_list	: /* empty */
		  { gl_ec_client->channel_map = 0xFFFFFFFF; } 
		| DEC_CONSTANT
		  { is_channel($<val>1);
		    if (range){
			int	i=0;
			for(i=start_channel;i<=$<val>1;i++){
				gl_ec_client->channel_map |= (1<<i);
			}
			start_channel=0;
		        range = 0;
		    }else{
		  	gl_ec_client->channel_map |= (1 << $<val>1);
		    }
		  }
		| DEC_CONSTANT '-'
		  { is_channel($<val>1);
		    range = 1; start_channel = $<val>1; }
					channel_list
		| DEC_CONSTANT '.'
		  { is_channel($<val>1);
		    if (range){
			int	i=0;
			for(i=start_channel;i<=$<val>1;i++){
				gl_ec_client->channel_map |= (1<<i);
			}
			start_channel=0;
		        range = 0;
		    }else{
		  	gl_ec_client->channel_map |= (1 << $<val>1);
		    }
		  }
					channel_list
		;
		
stats_debug_args:
		  { gl_ec_client->channel = 0; gl_ec_client->channel_map = 0x00000000; }
		| DEC_CONSTANT
		  { is_channel($<val>1);
		    gl_ec_client->channel = $<val>1; gl_ec_client->channel_map = (1<<$<val>1); }
		;

%%

unsigned long convert_str(char* str, int type)
{
	unsigned long value = 0;
	switch(type){
	case DEC_CONSTANT:
		sscanf(str, "%lu", &value);
		break;
	case HEX_CONSTANT:
		sscanf(str, "%lx", &value);
		break;
	}
	return value;
}

static int is_channel(unsigned long channel)
{
	if (channel > 31){
		printf("ERROR: Channel number %ld is out of range !\n\n",
					channel);
		exit(1);
	}
	return 0;
}

void yyerror(char* msg)
{
	printf("> %s (argv=%s,offset=%d)\n", msg, *targv, offset);
	exit(1);
}

