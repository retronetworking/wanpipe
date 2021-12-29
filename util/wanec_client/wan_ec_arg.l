%{

/* Any C code */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/ioctl.h>
#include "y.tab.h"
#include "wan_ecmain.h"
#include "wanec_api.h"

#define WAN_EC_VERSION	"1.3"

#define YYLMAX		512
#define token(x)	x
#define END(v)		(v-1+sizeof(v)/sizeof(v[0]))
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)			\
	{						\
		char c;					\
		if (targv < arglim){			\
			c = targv[0][offset++];		\
			if (c == '\n'){			\
				result = 0;		\
			}else if (c==' ' || c=='\0'){	\
				targv++;		\
				offset = 0;		\
				buf[0] = '\n';		\
				result = 1;		\
			}else{				\
				buf[0] = c;		\
				result = 1;		\
			}				\
		}else{					\
			result = 0;			\
		}					\
	}

char		**targv;
static char	**arglim;
unsigned	offset = 0;

wanec_client_t	*gl_ec_client;
int		action = WAN_EC_ACT_CMD;
char		idstr[YYLMAX];
int		gl_err = 0;

extern unsigned long convert_str(char* str, int type);
extern void yyerror(char*);
extern int yyparse();
void yy_reset(void);

static void s_lookup(int);
static int lookup_name(void);
static int wan_ec_read_param(void);
static int help(void);
static int help1(void);


%}

letter			[a-zA-Z_]
exp_letter		[a-zA-Z_\x80-\xFF]
digit			[0-9]
hex			[a-fA-F0-9]
power			[Ee][+-]?{digit}+
float_suffix		(f|F|l|L)
integer_suffix		("u"|"U"|"l"|"L"|"ul"|"lu"|"Ul"|"lU"|"uL"|"Lu"|"UL"|"LU")
letter_or_digit		[a-zA-Z_0-9]
exp_letter_or_digit	[a-zA-Z_0-9\x80-\xFF]
blank			[ \t]

%%

"-v"				{ gl_ec_client->verbose = 1; }
"-vv"				{ gl_ec_client->verbose = 2; }
"--h"				{ help(); exit(0); }
"--h1"				{ help(); help1(); exit(0); }

{digit}{digit}*{integer_suffix}?	{ /* decimal */ 
				s_lookup(token(DEC_CONSTANT));
				return token(DEC_CONSTANT);
				}

"t"{digit}{digit}*	{ /* decimal */ 
			s_lookup(token(DIAL_STRING));
			return token(DIAL_STRING);
			}

{letter}{letter_or_digit}*	{ return lookup_name(); }

"-"				{ return token('-'); }
"."				{ return token('.'); }

"--"				{ wan_ec_read_param(); }

.				{ 
					//return token(yytext[0]);
					s_lookup(token(CHAR_STRING));
					return token(CHAR_STRING);
				}

%%

static struct table_args_t {
	char*	name;
	int 	value;
	char*	descr;
} table_args[] = {
	{"all",		token(ALL_TOKEN),		"1" },
	{"bd",		token(BYPASS_DISABLE_TOKEN),	"2" },
	{"be",		token(BYPASS_ENABLE_TOKEN),	"3" },
	{"config",	token(CONFIG_TOKEN),		"4" },
	{"dd",		token(DTMF_DISABLE_TOKEN),	"5" },
	{"de",		token(DTMF_ENABLE_TOKEN),	"5" },
	{"disable",	token(DISABLE_TOKEN),		"5" },
	{"duration",	token(DURATION_TOKEN),		"5" },
	{"enable",	token(ENABLE_TOKEN),		"6" },
	{"help",	token(HELP_TOKEN),		"7" },
	{"help1",	token(HELP1_TOKEN),		"8" },
	{"kill",	token(KILL_TOKEN),		"10"},
	{"load",	token(LOAD_TOKEN),		"11"},
	{"mn",		token(MODE_NORMAL_TOKEN),	"12"},
	{"modify",	token(MODIFY_TOKEN),		"14"},
	{"monitor",	token(MONITOR_TOKEN),		"15"},
	{"mpd",		token(MODE_POWERDOWN_TOKEN),	"16"},
	{"playout_start",token(PLAYOUT_START_TOKEN),	"17"},
	{"playout_stop",token(PLAYOUT_STOP_TOKEN),	"17"},
	{"release",	token(RELEASE_TOKEN),		"18"},
	{"stats",	token(STATS_TOKEN),		"19"},
	{"stats_full",	token(STATS_FULL_TOKEN),	"20"},
	{"test",	token(TEST_TOKEN),		"21"},
	{"tone_load",	token(TONE_LOAD_TOKEN),		"17"},
	{"tone_unload",	token(TONE_UNLOAD_TOKEN),	"17"},
	{"unload",	token(UNLOAD_TOKEN),		"22"}
};

static int lookup_name()
{
	struct table_args_t	*first_arg = table_args,
				*last_arg = END(table_args),
				*mid_arg;
	int cmp = 0;

	while(first_arg <= last_arg){
		mid_arg = first_arg + (last_arg - first_arg) / 2;
		//printf("ADBG: %s (%s)\n", mid_arg->name, yytext);
		if ((cmp = strcmp(mid_arg->name, yytext)) == 0){
			return mid_arg->value;
		}else if (cmp < 0){
			first_arg = mid_arg + 1;
		}else{
			last_arg = mid_arg - 1;
		}
	}

	s_lookup(token(CHAR_STRING));
	return token(CHAR_STRING);
}

#ifndef YYSTYPE
static char* strsave(char* str)
{
	long	len = strlen(str);
	char*	new_str = NULL;
	
	if (len > YYLMAX){
		yyerror("Line is to long!");
		return NULL;
	}
	new_str = calloc(len, sizeof(char));
	strlcpy(new_str, str, len);
	return new_str;
}
#endif

static void s_lookup(int yylex)
{
#ifdef YYSTYPE
	switch(yylex){
	case DEC_CONSTANT:
	case HEX_CONSTANT:
		memset(idstr, 0, YYLMAX*sizeof(char));
		strlcpy(idstr, yytext, YYLMAX);
		yylval.val = convert_str(idstr, yylex);
		//yylval.str = idstr;
		break;
	case CHAR_STRING:
		memset(idstr, 0, YYLMAX*sizeof(char));
		strlcpy(idstr, yytext, YYLMAX);
		yylval.str = idstr;
		break;
	case DIAL_STRING:
		memset(idstr, 0, YYLMAX*sizeof(char));
		strlcpy(idstr, yytext, YYLMAX);
		yylval.str = idstr;
		break;
	}	
#else
	yylval = strsave(yytext);
#endif
}

void yy_reset()
{
	yy_start = 0;
	yyleng = 0;
}

int yywrap(void) { return 1; }

static int help(void)
{
	printf("\nWanpipe Echo Canceller setup utility (ver. %s)\n\n",
					WAN_EC_VERSION);
	printf("Usage:\n");
	printf(" wan_ec_client <devname> <commands>\n");
	printf("\n");
	printf(" Commands are:\n");
	printf("\tconfig\t\t\t- Configure all Echo Canceller channels\n");
	printf("\t\t\t\t  in Power Down mode!\n");
	printf("\trelease\t\t\t- Close/Release all Echo Canceller channels\n");
	printf("\tenable <FE_channel>\t- Enable Echo canceller on specified channel(s)\n");
	printf("\tdisable <FE_channel>\t- Disable Echo canceller on specified channel(s)\n");
	printf("\tde <FE_channel>\t\t- Enable DTMF detection on specified channel(s)\n");
	printf("\tdd <FE_channel>\t\t- Disable DTMF detection on specified channel(s)\n");
	printf("\tstats\t\t\t- Read Echo Canceller Chip/Image statistis\n");
	printf("\tstats <FE_channel>\t- Read Echo Canceller channel statistis\n");
	printf("\tmonitor <FE_channel>\t- Enable Debug monitoring for specified channel\n");
	printf("\tmonitor\t\t\t- Get debug data for previously specified channel\n");
	printf("\n");
	printf(" where:\n");
	printf("\t  FE_channel values:\n");
	printf("\t\t1,2,..,24\t- for T1 line\n");
	printf("\t\t1,2,..,31\t- for E1 line\n");
	printf("\t\tall\t\t- all channels\n");
	printf("\n");
	return 0;
}

static int help1(void)
{
	printf(" Special commands are:\n");
	printf("\tbe  <FE_channel>\t- Enable Echo canceller bypass mode\n");
	printf("\tbd  <FE_channel>\t- Disable Echo canceller bypass mode\n");
	printf("\tmn  <FE_channel>\t- Set Echo canceller in Normal mode\n");
	printf("\tmpd <FE_channel>\t- set Echo canceller int Power-Down mode\n");
	printf("\ttone_load <tone_name>\t- Load specific tone (tone_name.pcm)\n");
	printf("\ttone_unload <tone_id>\t- Unload specific tone (tone_id \n");
	printf("\t\t\t\t  returns by tone_load command)\n");
	printf("\tplayout_start <fe_chan> <tone_id>\t- Start playout specific tone\n");
	printf("\tplayout_stop <fe_chan> <tone_id>\t- Stop playout specific tone\n");
	printf("\tstats_full <FE_channel>\t- Read full Echo Canceller channel statistis\n");
	printf("\tmodify <FE_channel>\t- Modify channel configuration parameters\n");
	printf("\n");
	return 0;
}

static int wan_ec_read_param()
{
	char	str[MAX_EC_CLIENT_PARAM_LEN], *ptr;

	int i = 0;

	while((str[i] = input()) == '-');
	while(str[i] != EOF && str[i] != ' ' && str[i] != '\n' && str[i] != '\0'){
		str[++i] = input();
	}
	if (i == 0){
		return 0;
	}
	str[i] = '\0';
	ptr = strchr(str, '=');
	if (ptr == NULL){
		printf("Error: Invalid parameter format (%s)!\n", str);
		return 0;
	}
	*ptr = '\0'; ptr++;

	wanec_api_param(str, ptr);
#if 0	
	strlcpy(gl_ec_client->param[gl_ec_client->param_no].key,
					str, MAX_EC_CLIENT_PARAM_LEN); 
	strlcpy(gl_ec_client->param[gl_ec_client->param_no].value,
					ptr, MAX_EC_CLIENT_VALUE_LEN); 
	gl_ec_client->param_no++;
#endif
	return 0;
}

int wan_ec_args_parse_and_run(wanec_client_t *ec_client, int argc, char* argv[])
{
	if (argc == 1){
		help();
		return 0;
	}
	gl_ec_client = ec_client;
	targv = argv+1;
	arglim = argv+argc;
	yyparse();
	return gl_err;
}
