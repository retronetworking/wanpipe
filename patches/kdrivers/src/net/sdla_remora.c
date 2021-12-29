/***************************************************************************
 * sdla_remora.c	WANPIPE(tm) Multiprotocol WAN Link Driver. 
 *				AFT REMORA and FXO/FXS support module.
 *
 * Author: 	Alex Feldman   <al.feldman@sangoma.com>
 *
 * Copyright:	(c) 2005 Sangoma Technologies Inc.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 * Oct  6, 2005	Alex Feldman	Initial version.
 * Jul 27, 2006	David Rokhvarg	<davidr@sangoma.com>	Ported to Windows.
 ******************************************************************************
 */

/*******************************************************************************
**			   INCLUDE FILES
*******************************************************************************/
#if defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <wanpipe_abstr.h>
# include <wanpipe_common.h>
# include <wanpipe_events.h>
# include <wanpipe.h>
# include <sdla_remora.h>
# include <wanpipe_events.h>
#elif (defined __WINDOWS__)
# include <wanpipe_includes.h>
# include <wanpipe_defines.h>
# include <wanpipe.h>
# include <sdla_remora.h>
#else
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanpipe_common.h>
# include <linux/wanpipe_events.h>
# include <linux/wanpipe.h>
# include <linux/sdla_remora.h>
# include <linux/wanpipe_events.h>
#endif


/*******************************************************************************
**			  DEFINES AND MACROS
*******************************************************************************/
//#if defined(__WINDOWS__)
#if 1
# define AFT_TDM_API_SUPPORT
#else
# undef AFT_TDM_API_SUPPORT
#endif

#if 1
# define AFT_RM_INTR_SUPPORT
#else
# undef	AFT_RM_INTR_SUPPORT
#endif

#define AUDIO_RINGCHECK

#if 1
/* Current A200 designs (without interrupts) */
# define AFT_RM_VIRTUAL_INTR_SUPPORT
#else
# undef	AFT_RM_VIRTUAL_INTR_SUPPORT
#endif

#undef	SPI2STEP

/* TE1 critical flag */
#define WP_RM_TIMER_RUNNING	0x01
#define WP_RM_TIMER_KILL 	0x02
#define WP_RM_CONFIGURED 	0x03

#define WP_RM_POLL_TIMER	1000
#define WP_RM_POLL_EVENT_TIMER	10
#define WP_RM_POLL_TONE_TIMER	5000
#define WP_RM_POLL_RING_TIMER	10000
enum {
	WP_RM_POLL_TONE_DIAL	= 1,
	WP_RM_POLL_TONE_BUSY,
	WP_RM_POLL_TONE_RING,
	WP_RM_POLL_TONE_CONGESTION,
	WP_RM_POLL_TONE_DONE,
	WP_RM_POLL_RING,
	WP_RM_POLL_RING_STOP,
	WP_RM_POLL_TDMV,
	WP_RM_POLL_EVENT,
	WP_RM_POLL_INIT
};

/* tone_struct DialTone
** OSC1= 350 Hz OSC2= 440 Hz .0975 Volts -18 dBm */
#define	DIALTONE_IR13	0x7b30
#define	DIALTONE_IR14	0x0063	
#define	DIALTONE_IR16	0x7870	
#define	DIALTONE_IR17	0x007d	
#define	DIALTONE_DR32	6	
#define	DIALTONE_DR33	6	
#define	DIALTONE_DR36	0	
#define	DIALTONE_DR37	0	
#define	DIALTONE_DR38	0	
#define	DIALTONE_DR39	0	
#define	DIALTONE_DR40	0	
#define	DIALTONE_DR41	0	
#define	DIALTONE_DR42	0	
#define	DIALTONE_DR43	0	

/* tone_struct BusySignal
** OSC1= 480  OSC2 = 620 .0975 Voltz -18 dBm 8 */
#define	BUSYTONE_IR13	0x7700
#define	BUSYTONE_IR14	0x0089	
#define	BUSYTONE_IR16	0x7120	
#define	BUSYTONE_IR17	0x00B2	
#define	BUSYTONE_DR32	0x1E	
#define	BUSYTONE_DR33	0x1E	
#define	BUSYTONE_DR36	0xa0	
#define	BUSYTONE_DR37	0x0f	
#define	BUSYTONE_DR38	0xa0	
#define	BUSYTONE_DR39	0x0f	
#define	BUSYTONE_DR40	0xa0	
#define	BUSYTONE_DR41	0x0f	
#define	BUSYTONE_DR42	0xa0	
#define	BUSYTONE_DR43	0x0f	

/* tone_struct RingbackNormal
** OSC1 = 440 Hz OSC2 = 480 .0975 Volts -18 dBm */
#define	RINGBACKTONE_IR13	0x7870	
#define	RINGBACKTONE_IR14	0x007D	
#define	RINGBACKTONE_IR16	0x7700	
#define	RINGBACKTONE_IR17	0x0089	
#define	RINGBACKTONE_DR32	0x1E	
#define	RINGBACKTONE_DR33	0x1E	
#define	RINGBACKTONE_DR36	0x80	
#define	RINGBACKTONE_DR37	0x3E	
#define	RINGBACKTONE_DR38	0x0	
#define	RINGBACKTONE_DR39	0x7d	
#define	RINGBACKTONE_DR40	0x80	
#define	RINGBACKTONE_DR41	0x3E	
#define	RINGBACKTONE_DR42	0x0	
#define	RINGBACKTONE_DR43	0x7d

/* tone_struct CongestionTone
** OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBM */
#define	CONGESTIONTONE_IR13	0x7700
#define	CONGESTIONTONE_IR14	0x0089	
#define	CONGESTIONTONE_IR16	0x7120	
#define	CONGESTIONTONE_IR17	0x00B2	
#define	CONGESTIONTONE_DR32	0x1E	
#define	CONGESTIONTONE_DR33	0x1E	
#define	CONGESTIONTONE_DR36	0x40	
#define	CONGESTIONTONE_DR37	0x06	
#define	CONGESTIONTONE_DR38	0x60	
#define	CONGESTIONTONE_DR39	0x09	
#define	CONGESTIONTONE_DR40	0x40	
#define	CONGESTIONTONE_DR41	0x06	
#define	CONGESTIONTONE_DR42	0x60	
#define	CONGESTIONTONE_DR43	0x09	

#define WP_RM_CHUNKSIZE		8

#define RING_DEBOUNCE           16     /* Ringer Debounce (64 ms) */
#define DEFAULT_BATT_DEBOUNCE   16     /* Battery debounce (64 ms) */
#define POLARITY_DEBOUNCE       16     /* Polarity debounce (64 ms) */
#define DEFAULT_BATT_THRESH     3      /* Anything under this is "no battery" */

#define OHT_TIMER		6000	/* How long after RING to retain OHT */

#define MAX_ALARMS		10

#define WP_RM_WATCHDOG_TIMEOUT	100

static alpha  indirect_regs[] =
{
{0,255,"DTMF_ROW_0_PEAK",0x55C2},
{1,255,"DTMF_ROW_1_PEAK",0x51E6},
{2,255,"DTMF_ROW2_PEAK",0x4B85},
{3,255,"DTMF_ROW3_PEAK",0x4937},
{4,255,"DTMF_COL1_PEAK",0x3333},
{5,255,"DTMF_FWD_TWIST",0x0202},
{6,255,"DTMF_RVS_TWIST",0x0202},
{7,255,"DTMF_ROW_RATIO_TRES",0x0198},
{8,255,"DTMF_COL_RATIO_TRES",0x0198},
{9,255,"DTMF_ROW_2ND_ARM",0x0611},
{10,255,"DTMF_COL_2ND_ARM",0x0202},
{11,255,"DTMF_PWR_MIN_TRES",0x00E5},
{12,255,"DTMF_OT_LIM_TRES",0x0A1C},
{13,0,"OSC1_COEF",0x7B30},
{14,1,"OSC1X",0x0063},
{15,2,"OSC1Y",0x0000},
{16,3,"OSC2_COEF",0x7870},
{17,4,"OSC2X",0x007D},
{18,5,"OSC2Y",0x0000},
{19,6,"RING_V_OFF",0x0000},
{20,7,"RING_OSC",0x7EF0},
{21,8,"RING_X",0x0160},
{22,9,"RING_Y",0x0000},
{23,255,"PULSE_ENVEL",0x2000},
{24,255,"PULSE_X",0x2000},
{25,255,"PULSE_Y",0x0000},
/*{26,13,"RECV_DIGITAL_GAIN",0x4000},*/	/* playback volume set lower */
{26,13,"RECV_DIGITAL_GAIN",0x2000},	/* playback volume set lower */
{27,14,"XMIT_DIGITAL_GAIN",0x4000},
/*{27,14,"XMIT_DIGITAL_GAIN",0x2000}, */
{28,15,"LOOP_CLOSE_TRES",0x1000},
{29,16,"RING_TRIP_TRES",0x3600},
{30,17,"COMMON_MIN_TRES",0x1000},
{31,18,"COMMON_MAX_TRES",0x0200},
{32,19,"PWR_ALARM_Q1Q2",0x07C0},
{33,20,"PWR_ALARM_Q3Q4",0x2600},
{34,21,"PWR_ALARM_Q5Q6",0x1B80},
{35,22,"LOOP_CLOSURE_FILTER",0x8000},
{36,23,"RING_TRIP_FILTER",0x0320},
{37,24,"TERM_LP_POLE_Q1Q2",0x008C},
{38,25,"TERM_LP_POLE_Q3Q4",0x0100},
{39,26,"TERM_LP_POLE_Q5Q6",0x0010},
{40,27,"CM_BIAS_RINGING",0x0C00},
{41,64,"DCDC_MIN_V",0x0C00},
{42,255,"DCDC_XTRA",0x1000},
{43,66,"LOOP_CLOSE_TRES_LOW",0x1000},
};

static struct fxo_mode {
	char *name;
	/* FXO */
	int ohs;
	int ohs2;
	int rz;
	int rt;
	int ilim;
	int dcv;
	int mini;
	int acim;
	int ring_osc;
	int ring_x;
} fxo_modes[] =
{
	{ "FCC", 0, 0, 0, 1, 0, 0x3, 0, 0, }, 	/* US, Canada */
	{ "TBR21", 0, 0, 0, 0, 1, 0x3, 0, 0x2, 0x7e6c, 0x023a, },
		/* Austria, Belgium, Denmark, Finland, France, Germany, 
		   Greece, Iceland, Ireland, Italy, Luxembourg, Netherlands,
		   Norway, Portugal, Spain, Sweden, Switzerland, and UK */
	{ "ARGENTINA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "AUSTRALIA", 1, 0, 0, 0, 0, 0, 0x3, 0x3, },
	{ "AUSTRIA", 0, 1, 0, 0, 1, 0x3, 0, 0x3, },
	{ "BAHRAIN", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "BELGIUM", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "BRAZIL", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "BULGARIA", 0, 0, 0, 0, 1, 0x3, 0x0, 0x3, },
	{ "CANADA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "CHILE", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "CHINA", 0, 0, 0, 0, 0, 0, 0x3, 0xf, },
	{ "COLUMBIA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "CROATIA", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "CYRPUS", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "CZECH", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "DENMARK", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ECUADOR", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "EGYPT", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "ELSALVADOR", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "FINLAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "FRANCE", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "GERMANY", 0, 1, 0, 0, 1, 0x3, 0, 0x3, },
	{ "GREECE", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "GUAM", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "HONGKONG", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "HUNGARY", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "ICELAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "INDIA", 0, 0, 0, 0, 0, 0x3, 0, 0x4, },
	{ "INDONESIA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "IRELAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ISRAEL", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ITALY", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "JAPAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "JORDAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "KAZAKHSTAN", 0, 0, 0, 0, 0, 0x3, 0, },
	{ "KUWAIT", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "LATVIA", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "LEBANON", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "LUXEMBOURG", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "MACAO", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "MALAYSIA", 0, 0, 0, 0, 0, 0, 0x3, 0, },	/* Current loop >= 20ma */
	{ "MALTA", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "MEXICO", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "MOROCCO", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "NETHERLANDS", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "NEWZEALAND", 0, 0, 0, 0, 0, 0x3, 0, 0x4, },
	{ "NIGERIA", 0, 0, 0, 0, 0x1, 0x3, 0, 0x2, },
	{ "NORWAY", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "OMAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "PAKISTAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "PERU", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "PHILIPPINES", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "POLAND", 0, 0, 1, 1, 0, 0x3, 0, 0, },
	{ "PORTUGAL", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ROMANIA", 0, 0, 0, 0, 0, 3, 0, 0, },
	{ "RUSSIA", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "SAUDIARABIA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "SINGAPORE", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "SLOVAKIA", 0, 0, 0, 0, 0, 0x3, 0, 0x3, },
	{ "SLOVENIA", 0, 0, 0, 0, 0, 0x3, 0, 0x2, },
	{ "SOUTHAFRICA", 1, 0, 1, 0, 0, 0x3, 0, 0x3, },
	{ "SOUTHKOREA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "SPAIN", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "SWEDEN", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "SWITZERLAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "SYRIA", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "TAIWAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "THAILAND", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "UAE", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "UK", 0, 1, 0, 0, 1, 0x3, 0, 0x5, },
	{ "USA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "YEMEN", 0, 0, 0, 0, 0, 0x3, 0, 0, },
};

/*******************************************************************************
**			STRUCTURES AND TYPEDEFS
*******************************************************************************/

/*******************************************************************************
**			   GLOBAL VARIABLES
*******************************************************************************/
#if !defined(__WINDOWS__)
extern WAN_LIST_HEAD(, wan_tdmv_) wan_tdmv_head;
#endif
static int battdebounce = DEFAULT_BATT_DEBOUNCE;
static int battthresh = DEFAULT_BATT_THRESH;

/*******************************************************************************
**			  FUNCTION PROTOTYPES
*******************************************************************************/
int wp_init_proslic(sdla_fe_t *fe, int mod_no, int fast, int sane);
int wp_init_voicedaa(sdla_fe_t *fe, int mod_no, int fast, int sane);

static int wp_remora_config(void *pfe);
static int wp_remora_unconfig(void *pfe);
static int wp_remora_if_config(void *pfe, u32 mod_map, u8);
static int wp_remora_if_unconfig(void *pfe, u32 mod_map, u8);
static int wp_remora_disable_irq(void *pfe); 
static int wp_remora_intr(sdla_fe_t *); 
static int wp_remora_check_intr(sdla_fe_t *); 
static void wp_remora_enable_timer(sdla_fe_t*, int, unsigned char, unsigned int);
static int wp_remora_polling(sdla_fe_t*);
static int wp_remora_udp(sdla_fe_t*, void*, unsigned char*);
static unsigned int wp_remora_active_map(sdla_fe_t* fe);
static unsigned char wp_remora_fe_media(sdla_fe_t *fe);
static int wp_remora_set_dtmf(sdla_fe_t*, int, unsigned char);
static int wp_remora_intr_ctrl(sdla_fe_t*, int, int, int, unsigned int);
static int wp_remora_event_ctrl(sdla_fe_t*, wan_event_ctrl_t*);

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
static void wp_remora_timer(void*);
#elif defined(__WINDOWS__)
static void wp_remora_timer(IN PKDPC,void*,void*,void*);
#else
static void wp_remora_timer(unsigned long);
#endif

static int wp_remora_dialtone(sdla_fe_t*, int);
static int wp_remora_busytone(sdla_fe_t*, int);
static int wp_remora_ringtone(sdla_fe_t*, int);
static int wp_remora_congestiontone(sdla_fe_t*, int);
static int wp_remora_disabletone(sdla_fe_t*, int);

#if defined(AFT_TDM_API_SUPPORT)
static int wp_remora_watchdog(sdla_fe_t *fe);
static void wp_remora_voicedaa_check_hook(sdla_fe_t *fe, int mod_no);
#endif

/*******************************************************************************
**			  FUNCTION DEFINITIONS
*******************************************************************************/

static void wait_just_a_bit(int foo, int fast)
{

#if defined(__FreeBSD__) || defined(__OpenBSD__)
	WP_SCHEDULE(foo, "A200");
#else
	unsigned long	start_ticks;
	start_ticks = SYSTEM_TICKS + foo;
	while(SYSTEM_TICKS < start_ticks){
		WP_DELAY(100);
# if defined(__LINUX__)
		if (!fast) WP_SCHEDULE(foo, "A200");
# endif
	}
#endif
}

static void wp_remora_reset_spi(sdla_fe_t *fe)
{
	sdla_t	*card = (sdla_t*)fe->card;

	WAN_ASSERT1(card == NULL);
	card->hw_iface.bus_write_4(	card->hw,
					SPI_INTERFACE_REG,
					MOD_SPI_RESET);
	WP_DELAY(1000);
	card->hw_iface.bus_write_4(	card->hw,
					SPI_INTERFACE_REG,
					0x00000000);
	WP_DELAY(1000);
	return;
}


static int
wp_proslic_setreg_indirect(sdla_fe_t *fe, int mod_no, unsigned char address, unsigned short data)
{

	WRITE_RM_REG(mod_no, IDA_LO,(unsigned char)(data & 0xFF));
	WRITE_RM_REG(mod_no, IDA_HI,(unsigned char)((data & 0xFF00)>>8));
	WRITE_RM_REG(mod_no, IAA,address);
	return 0;
}

static int
wp_proslic_getreg_indirect(sdla_fe_t *fe, int mod_no, unsigned char address)
{ 
	int res = -1;
	unsigned char data1, data2;

	WRITE_RM_REG(mod_no, IAA, address);
	data1 = READ_RM_REG(mod_no, IDA_LO);
	data2 = READ_RM_REG(mod_no, IDA_HI);
	res = data1 | (data2 << 8);
	return res;
}

static int wp_proslic_init_indirect_regs(sdla_fe_t *fe, int mod_no)
{
	unsigned char i;

	for (i=0; i<sizeof(indirect_regs) / sizeof(indirect_regs[0]); i++){
		if(wp_proslic_setreg_indirect(fe, mod_no, indirect_regs[i].address,indirect_regs[i].initial))
			return -1;
	}

	return 0;
}

static int wp_proslic_verify_indirect_regs(sdla_fe_t *fe, int mod_no)
{ 
	int passed = 1;
	unsigned short i, initial;
	int j;

	for (i=0; i<sizeof(indirect_regs) / sizeof(indirect_regs[0]); i++){ 
		
		j = wp_proslic_getreg_indirect(
				fe,
				mod_no,
				(unsigned char) indirect_regs[i].address);
		if (j < 0){
			DEBUG_EVENT("%s: Module %d: Failed to read indirect register %d\n",
						fe->name, mod_no,
						i);
			return -1;
		}
		initial= indirect_regs[i].initial;

		if ( j != initial && indirect_regs[i].altaddr != 255){
			DEBUG_EVENT(
			"%s: Module %d: Internal Error: iReg=%s (%X) Value=%X (%X)\n",
						fe->name, mod_no,
						indirect_regs[i].name,
						indirect_regs[i].address,
						j, initial);
			 passed = 0;
		}	
	}

	if (!passed) {
		DEBUG_EVENT(
		"%s: Module %d: Init Indirect Registers UNSUCCESSFULLY.\n",
						fe->name, mod_no);
		return -1;
	}
	return 0;
}


static int wp_remora_chain_enable(sdla_fe_t *fe)
{
	int		mod_no;
	unsigned char	byte;

	for(mod_no = 0;mod_no < MAX_REMORA_MODULES; mod_no += 2){
		if (fe->rm_param.mod[mod_no].type == MOD_TYPE_NONE){
			byte = READ_RM_FXS_REG(mod_no, 0, 0);
			byte &= 0x0F;
			if (byte == 0x05){
				DEBUG_RM("%s: Module %d/%d FXS\n",
					fe->name, mod_no,mod_no+1);
				fe->rm_param.mod[mod_no].type	=
						MOD_TYPE_FXS;
				fe->rm_param.mod[mod_no+1].type	=
						MOD_TYPE_FXS;
			}else if (byte == 0x00){
				DEBUG_RM("%s: Module %d/%d TEST\n",
					fe->name, mod_no,mod_no+1);
				fe->rm_param.mod[mod_no].type	=
						MOD_TYPE_TEST;
				fe->rm_param.mod[mod_no+1].type	=
						MOD_TYPE_TEST;			
			}
		}
	}
	/* Reset SPI interface */
	wp_remora_reset_spi(fe);
	for(mod_no = 0;mod_no < MAX_REMORA_MODULES; mod_no += 2){
		if (fe->rm_param.mod[mod_no].type == MOD_TYPE_NONE){
			byte = READ_RM_FXO_REG(mod_no,1,2);
			if (byte == 0x03){
				DEBUG_RM("%s: Module %d/%d FXO\n",
						fe->name, mod_no,mod_no+1);
				fe->rm_param.mod[mod_no].type	=
							MOD_TYPE_FXO;
				fe->rm_param.mod[mod_no+1].type	=
							MOD_TYPE_FXO;
			}
		}
	}

	/* Reset SPI interface */
	wp_remora_reset_spi(fe);

	/* Now enable chain mode for only FXS modules (FXO by default chain) */
	for(mod_no = 0;mod_no < MAX_REMORA_MODULES; mod_no += 2){
		if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXS){
			WRITE_RM_FXS_REG(mod_no,0,0,0xC0);
			byte = READ_RM_FXS_REG(mod_no, 1, 0);
			if ((byte & 0x80) != 0x80){
				DEBUG_RM(
				"%s: Module %d: Failed to enable chain (%02X)!\n", 
						fe->name, mod_no, byte);
				return -EINVAL;
			}
		}else if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXO){
			byte = READ_RM_FXO_REG(mod_no,1,2);
			if (byte != 0x03){
				/* Should never happened */
				fe->rm_param.mod[mod_no].type	= MOD_TYPE_NONE;
				fe->rm_param.mod[mod_no+1].type	= MOD_TYPE_NONE;
				continue;
			}
		}else if (fe->rm_param.mod[mod_no].type == MOD_TYPE_TEST){
			/* Test module or nothing */
			continue;
		}else{
			DEBUG_RM(
			"%s: Module %d: Failed to detect FXO/FXS module!\n", 
						fe->name, mod_no);
			continue;
		}
		DEBUG_RM(
		"%s: Module %d/%d %s (chain)\n",
			fe->name, mod_no,mod_no+1,
			WP_REMORA_DECODE_TYPE(fe->rm_param.mod[mod_no].type));	
		fe->rm_param.mod[mod_no].chain	= MOD_CHAIN_ENABLED;
		fe->rm_param.mod[mod_no+1].chain= MOD_CHAIN_ENABLED;
	}
	return 0;
}

static int wp_init_proslic_insane(sdla_fe_t *fe, int mod_no)
{
	unsigned char	value;

	value = READ_RM_REG(mod_no, 0);
	if ((value & 0x30) >> 4){
		DEBUG_RM("%s: Proslic on module %d is not a Si3210 (%02X)!\n",
					fe->name,
					mod_no,
					value);
		return -1;
	}
	if (((value & 0x0F) == 0) || ((value & 0x0F) == 0x0F)){
		DEBUG_RM("%s: Proslic is not loaded!\n",
					fe->name);
		return -1;
	}
	if ((value & 0x0F) < 2){
		DEBUG_EVENT("%s: Proslic 3210 version %d is too old!\n",
					fe->name,
					value & 0x0F);
		return -1;
	}

	value = READ_RM_REG(mod_no, 8);
	if (value != 0x2) {
		DEBUG_EVENT(
		"%s: Proslic on module %d insane (1) %d should be 2!\n",
					fe->name, mod_no, value);
		return -1;
	}

	value = READ_RM_REG(mod_no, 64);
	if (value != 0x0) {
		DEBUG_EVENT(
		"%s: Proslic on modyle %d insane (2) %d should be 0!\n",
					fe->name, mod_no, value);
		return -1;
	} 

	value = READ_RM_REG(mod_no, 11);
	if (value != 0x33) {
		DEBUG_EVENT(
		"%s: Proslic on module %d insane (3) %02X should be 0x33\n",
					fe->name, mod_no, value);
		return -1;
	}
	WRITE_RM_REG(mod_no, 30, 0);
	return 0;
}


static int wp_powerup_proslic(sdla_fe_t *fe, int mod_no, int fast)
{
	wp_remora_fxs_t	*fxs;
	unsigned long	start_ticks;
	int		loopcurrent = 20, lim;
	unsigned char	vbat;
	
	fxs = &fe->rm_param.mod[mod_no].u.fxs;
	/* set the period of the DC-DC converter to 1/64 kHz  START OUT SLOW*/
	WRITE_RM_REG(mod_no, 92, 0xf5);

	start_ticks = SYSTEM_TICKS;
	WRITE_RM_REG(mod_no, 14, 0x0);	/* DIFF DEMO 0x10 */

	if (fast) return 0;

	/* powerup */ 
	WRITE_RM_REG(mod_no, 93, 0x1F);
	while((vbat = READ_RM_REG(mod_no, 82)) < 0xC0){
		/* Wait no more than 500ms */
		if ((SYSTEM_TICKS - start_ticks) > HZ/2){
			break;
		}
		wait_just_a_bit(HZ/10, fast);
	}

	if (vbat < 0xc0){
		if (fxs->proslic_power == PROSLIC_POWER_UNKNOWN){
			DEBUG_EVENT(
			"%s: Module %d: Failed to powerup within %d ms (%d mV only)!\n",
					fe->name,
					mod_no,
					(int)(((SYSTEM_TICKS - start_ticks) * 1000 / HZ)),
					vbat * 375);
			DEBUG_EVENT(
			"%s: Module %d: Did you remember to plug in the power cable?\n",
					fe->name,
					mod_no);

		}
		fxs->proslic_power = PROSLIC_POWER_WARNED;
		return -1;
	}
	fxs->proslic_power = PROSLIC_POWER_ON;
	DEBUG_RM("%s: Module %d: Current Battery1 %dV, Battery2 %dV\n",
					fe->name, mod_no,
					READ_RM_REG(mod_no, 82)*375/1000,
					READ_RM_REG(mod_no, 83)*375/1000);

        /* Proslic max allowed loop current, reg 71 LOOP_I_LIMIT */
        /* If out of range, just set it to the default value     */
        lim = (loopcurrent - 20) / 3;
        if ( loopcurrent > 41 ) {
                lim = 0;
		DEBUG_RM(
		"%s: Module %d: Loop current out of range (default 20mA)!\n",
					fe->name, mod_no);
        }else{
		DEBUG_RM("%s: Loop current set to %dmA!\n",
					fe->name,
					(lim*3)+20);
	}
        WRITE_RM_REG(mod_no, 71,lim);

	WRITE_RM_REG(mod_no, 93, 0x99);  /* DC-DC Calibration  */
#if 1
	/* Wait for DC-DC Calibration to complete */
	start_ticks = SYSTEM_TICKS;
	while(0x80 & READ_RM_REG(mod_no, 93)){
		if ((SYSTEM_TICKS - start_ticks) > 2*HZ){
			DEBUG_EVENT(
			"%s: Module %d: Timeout waiting for DC-DC calibration\n",
						fe->name,
						mod_no);
			return -EINVAL;
		}
		wait_just_a_bit(HZ/10, fast);
	}
#endif
	return 0;
}

static int wp_proslic_powerleak_test(sdla_fe_t *fe, int mod_no)
{
	unsigned long	start_ticks;
	unsigned char	vbat;

	/* powerleak */ 
	WRITE_RM_REG(mod_no, 64, 0);
	WRITE_RM_REG(mod_no, 14, 0x10);
	/* wait for 1 s */
	start_ticks = SYSTEM_TICKS;
	while((vbat = READ_RM_REG(mod_no, 82)) >= 0x6){
		if ((SYSTEM_TICKS - start_ticks) > (HZ/2)){
			break;
		}
		wait_just_a_bit(HZ/10, 0);
	}
	if (vbat < 0x6){
		DEBUG_EVENT(
		"%s: Module %d: Excessive leakage detected: %d volts (%02x) after %d ms\n",
					fe->name, mod_no,
					376 * vbat / 1000,
					vbat,
					(int)((SYSTEM_TICKS - start_ticks) * 1000 / HZ));
		return -1;
	}
	DEBUG_RM("%s: Module %d: Post-leakage voltage: %d volts\n",
					fe->name,
					mod_no,
					376 * vbat / 1000);

	return 0;
}


/* static */
int wp_init_proslic(sdla_fe_t *fe, int mod_no, int fast, int sane)
{
	volatile unsigned long	start_ticks;
	unsigned short	tmp[5];
	unsigned char	value;
	volatile int		i, x;

	/* By default, don't send on hook */
	if (fe->fe_cfg.cfg.remora.reversepolarity){
		fe->rm_param.mod[mod_no].u.fxs.idletxhookstate = 5;
	}else{
		fe->rm_param.mod[mod_no].u.fxs.idletxhookstate = 1;
	}

	/* Step 8 */
	if (!sane && wp_init_proslic_insane(fe, mod_no)){
		return -2;
	}

	if (sane){
		WRITE_RM_REG(mod_no, 14, 0x10);
	}

	/* Step 9 */
	if (wp_proslic_init_indirect_regs(fe, mod_no)) {
		DEBUG_EVENT(
		"%s: Module %d: Indirect Registers failed to initialize!\n",
							fe->name,
							mod_no);
		return -1;
	}
	wp_proslic_setreg_indirect(fe, mod_no, 97,0);

	/* Steo 10 */
	WRITE_RM_REG(mod_no, 8, 0);		/*DIGIUM*/
	WRITE_RM_REG(mod_no, 108, 0xeb);	/*DIGIUM*/
	WRITE_RM_REG(mod_no, 67, 0x17);
	WRITE_RM_REG(mod_no, 66, 1);

	/* Flush ProSLIC digital filters by setting to clear, while
	** saving old values */
	for (x=0;x<5;x++) {
		tmp[x] = (unsigned short)wp_proslic_getreg_indirect(fe, mod_no, x + 35);
		wp_proslic_setreg_indirect(fe, mod_no, x + 35, 0x8000);
	}

	/* Power up the DC-DC converter */
	if (wp_powerup_proslic(fe, mod_no, fast)) {
		DEBUG_EVENT(
		"%s: Module %d: Unable to do INITIAL ProSLIC powerup!\n",
					fe->name,
					mod_no);
		return -1;
	}

	if (!fast){

		if (wp_proslic_powerleak_test(fe, mod_no)){
			DEBUG_EVENT(
			"%s: Module %d: Proslic failed leakge the short circuit\n",
						fe->name,
						mod_no);
		}

		/* Step 12 */
		if (wp_powerup_proslic(fe, mod_no, fast)) {
			DEBUG_EVENT(
			"%s: Module %d: Unable to do INITIAL ProSLIC powerup!\n",
						fe->name,
						mod_no);
			return -1;
		}

		/* Step 13 */
		WRITE_RM_REG(mod_no, 64, 0);

		/* Step 14 */
		WRITE_RM_REG(mod_no, 97, 0x1E);
		WRITE_RM_REG(mod_no, 96, 0x47);

		/* Step 15 */
		start_ticks = SYSTEM_TICKS;
		while(READ_RM_REG(mod_no, 96) != 0){
			if ((SYSTEM_TICKS - start_ticks) > 400){
				DEBUG_EVENT(
				"%s: Module %d: Timeout on SLIC calibration (15)!\n",
						fe->name, mod_no);
				return -1;
			}
			wait_just_a_bit(HZ/10, fast);
		}

		/* Step 16 */
		/* Insert manual calibration for sangoma Si3210 */
		WRITE_RM_REG(mod_no, 98, 0x10);
		//WRITE_RM_REG(mod_no, 98, 0x1F/*0x10*/);
		for (i = 0x1f; i > 0; i--){

			WRITE_RM_REG(mod_no, 98, i);
			wait_just_a_bit(4, fast);
			if ((READ_RM_REG(mod_no, 88)) == 0){
				break;
			}
		}

		WRITE_RM_REG(mod_no, 99, 0x10);
		//WRITE_RM_REG(mod_no, 99, 0x1F/*0x10*/);
		for (i = 0x1f; i > 0; i--){

			WRITE_RM_REG(mod_no, 99, i);
			wait_just_a_bit(4, fast);
			if ((READ_RM_REG(mod_no, 89)) == 0){
				break;
			}
		}

		/* Step 17 */
		value = READ_RM_REG(mod_no, 23);
		WRITE_RM_REG(mod_no, 23, value | 0x04);

		/* Step 18 */
		/* DAC offset and without common mode calibration. */
		WRITE_RM_REG(mod_no, 97, 0x01/*0x18*/);	/* Manual after */
		/* Calibrate common mode and differential DAC mode DAC + ILIM */
		WRITE_RM_REG(mod_no, 96, 0x40/*0x47*/);

		/* Step 19 */
		start_ticks = SYSTEM_TICKS;
		while(READ_RM_REG(mod_no, 96) != 0){
			if ((SYSTEM_TICKS - start_ticks) > 2000/*400*/){
				DEBUG_EVENT(
				"%s: Module %d: Timeout on SLIC calibration (19:%02X)!\n",
						fe->name,
						mod_no,
						READ_RM_REG(mod_no, 96));
				return -1;
			}
			wait_just_a_bit(HZ/10, fast);
		}
		DEBUG_RM("%s: SLIC calibration complete (%ld)\n",
					fe->name, SYSTEM_TICKS-start_ticks);
		/* Save calibration vectors */
		for (x=0;x<NUM_CAL_REGS;x++){
			fe->rm_param.mod[mod_no].u.fxs.callregs.vals[x] =
					READ_RM_REG(mod_no, 96 + x);
		}

	}else{
		/* Restore calibration vectors */
		for (x=0;x<NUM_CAL_REGS;x++){
			WRITE_RM_REG(mod_no, 96 + x, 
				fe->rm_param.mod[mod_no].u.fxs.callregs.vals[x]);
		}
	}    

	/* Step 20 */
	wp_proslic_setreg_indirect(fe, mod_no, 88, 0);
	wp_proslic_setreg_indirect(fe, mod_no, 89, 0);
	wp_proslic_setreg_indirect(fe, mod_no, 90, 0);
	wp_proslic_setreg_indirect(fe, mod_no, 91, 0);
	wp_proslic_setreg_indirect(fe, mod_no, 92, 0);
	wp_proslic_setreg_indirect(fe, mod_no, 93, 0);
	wp_proslic_setreg_indirect(fe, mod_no, 94, 0);
	wp_proslic_setreg_indirect(fe, mod_no, 95, 0);

	if (!fast){
		/* Disable interrupt while full initialization */
		WRITE_RM_REG(mod_no, 21, 0);
		WRITE_RM_REG(mod_no, 22, 0);
		WRITE_RM_REG(mod_no, 23, 0);
		
#if defined(AFT_RM_INTR_SUPPORT)
		fe->rm_param.mod[mod_no].u.fxs.imask1 = 0x00;
		fe->rm_param.mod[mod_no].u.fxs.imask2 = 0x03;
		fe->rm_param.mod[mod_no].u.fxs.imask3 = 0x01;
#else		
		fe->rm_param.mod[mod_no].u.fxs.imask1 = 0x00;
		fe->rm_param.mod[mod_no].u.fxs.imask2 = 0x00;
		fe->rm_param.mod[mod_no].u.fxs.imask3 = 0x00;
#endif		
	}

	WRITE_RM_REG(mod_no, 64, 0);/* (0) */

	//Alex Apr 3 - WRITE_RM_REG(mod_no, 64, 0x1);

	value = READ_RM_REG(mod_no, 68); 
	/*
	** FIXME ???value = value & 0x03;
	** if (value & 4){
	**	printf("Module %d Timeout!\n", mod_no);
	**	return -1;
	** } */

#if 1
	WRITE_RM_REG(mod_no, 64, 0x00);

	/* this is a singular calibration bit for longitudinal calibration */
	WRITE_RM_REG(mod_no, 97, 0x01);
	WRITE_RM_REG(mod_no, 96, 0x40);

	value = READ_RM_REG(mod_no, 96); 

	WRITE_RM_REG(mod_no, 18,0xff);
	WRITE_RM_REG(mod_no, 19,0xff);
	WRITE_RM_REG(mod_no, 20,0xff);

	/* WRITE_RM_REG(mod_no, 64,0x1); */
#endif

	/* Perform DC-DC calibration */
	WRITE_RM_REG(mod_no,  93, 0x99);
	/*wait_just_a_bit(10, fast);*/
	value = READ_RM_REG(mod_no, 107);
	if ((value < 0x2) || (value > 0xd)) {
		DEBUG_EVENT(
		"%s: Module %d: DC-DC calibration has a surprising direct 107 of 0x%02x!\n",
					fe->name,
					mod_no,
					value);
		WRITE_RM_REG(mod_no,  107, 0x8);
	}

	for (x=0;x<5;x++) {
		wp_proslic_setreg_indirect(fe, mod_no, x + 35, tmp[x]);
	}

	if (wp_proslic_verify_indirect_regs(fe, mod_no)) {
		DEBUG_EVENT(
		"%s: Module %d: Indirect Registers failed verification.\n",
					fe->name,
					mod_no);
		return -1;
	}

	if (fe->fe_cfg.tdmv_law == WAN_TDMV_ALAW){
		WRITE_RM_REG(mod_no, 1, 0x20);
	}else if (fe->fe_cfg.tdmv_law == WAN_TDMV_MULAW){
		WRITE_RM_REG(mod_no, 1, 0x28);
	}
	/* U-Law 8-bit interface */
	/* Tx Start count low byte  0 */
	WRITE_RM_REG(mod_no, 2, mod_no * 8 + 1);
	/* Tx Start count high byte 0 */
	WRITE_RM_REG(mod_no, 3, 0);
	/* Rx Start count low byte  0 */
	WRITE_RM_REG(mod_no, 4, mod_no * 8 + 1);
	/* Rx Start count high byte 0 */
	WRITE_RM_REG(mod_no, 5, 0);
	/* Clear all interrupt */
	WRITE_RM_REG(mod_no, 18, 0xff);
	WRITE_RM_REG(mod_no, 19, 0xff);
	WRITE_RM_REG(mod_no, 20, 0xff);
	WRITE_RM_REG(mod_no, 73, 0x04);

#if 0
	/* Enable loopback */
	WRITE_RM_REG(mod_no, 8,  0x2);
	WRITE_RM_REG(mod_no, 14, 0x0);
	WRITE_RM_REG(mod_no, 64, 0x0);
	WRITE_RM_REG(mod_no, 1,  0x08);
#endif

	WRITE_RM_REG(mod_no, 64, 0x1);

	DEBUG_RM("%s: Module %d: Current Battery1 %dV, Battery2 %dV (%d)\n",
					fe->name, mod_no,
					READ_RM_REG(mod_no, 82)*375/1000,
					READ_RM_REG(mod_no, 83)*375/1000,
					__LINE__);

#if 0
	/* verify TIP/RING voltage */
	if (!fast){
		wait_just_a_bit(HZ, fast);
		start_ticks = SYSTEM_TICKS;
		while(READ_RM_REG(mod_no, 81) < 0x75){
			if ((SYSTEM_TICKS - start_ticks) > HZ*10){
				break;	
			}
			wait_just_a_bit(HZ, fast);
		}
		wait_just_a_bit(HZ, fast);
		if (READ_RM_REG(mod_no, 81) < 0x75){
			if (sane){
				DEBUG_EVENT(
				"%s: Module %d: TIP/RING is too low on FXS %d!\n",
						fe->name,
						mod_no,
						READ_RM_REG(mod_no, 81) * 375 / 1000);
			}
			return -1;
		}
	}
#endif
	DEBUG_RM("%s: Module %d: Current Battery1 %dV, Battery2 %dV (%d)\n",
					fe->name, mod_no,
					READ_RM_REG(mod_no, 82)*375/1000,
					READ_RM_REG(mod_no, 83)*375/1000,
					__LINE__);

	/* lowpower */
	//WRITE_RM_REG(mod_no, 72, 0x14);
	//todayWRITE_RM_REG(mod_no, 64, 0x1);
	return 0;
}

static int wp_voicedaa_insane(sdla_fe_t *fe, int mod_no)
{
	unsigned char	byte;

	byte = READ_RM_REG(mod_no, 2);
	if (byte != 0x3)
		return -2;
	byte = READ_RM_REG(mod_no, 11);
	DEBUG_TEST("%s: Module %d: VoiceDAA System: %02x\n",
				fe->name,
				mod_no,
				byte & 0xf);
	return 0;
}


int wp_init_voicedaa(sdla_fe_t *fe, int mod_no, int fast, int sane)
{
	unsigned char reg16=0, reg26=0, reg30=0, reg31=0;
	unsigned long	start_ticks;

	if (!sane && wp_voicedaa_insane(fe, mod_no)){
		return -2;
	}

	/* Software reset */
	WRITE_RM_REG(mod_no, 1, 0x80);

	/* Wait just a bit */
	if (fast) {
		WP_DELAY(10000);
	} else {
		wait_just_a_bit(HZ/10, fast);
	}

	/* Enable PCM, ulaw */
	if (fe->fe_cfg.tdmv_law == WAN_TDMV_ALAW){
		WRITE_RM_REG(mod_no, 33, 0x20);
	}else if (fe->fe_cfg.tdmv_law == WAN_TDMV_MULAW){
		WRITE_RM_REG(mod_no, 33, 0x28);
	}

	/* Set On-hook speed, Ringer impedence, and ringer threshold */
	reg16 |= (fxo_modes[fe->fe_cfg.cfg.remora.opermode].ohs << 6);
	reg16 |= (fxo_modes[fe->fe_cfg.cfg.remora.opermode].rz << 1);
	reg16 |= (fxo_modes[fe->fe_cfg.cfg.remora.opermode].rt);
	WRITE_RM_REG(mod_no, 16, reg16);
	
	/* Set DC Termination:
	**	Tip/Ring voltage adjust,
	**	minimum operational current,
	**	current limitation */
	reg26 |= (fxo_modes[fe->fe_cfg.cfg.remora.opermode].dcv << 6);
	reg26 |= (fxo_modes[fe->fe_cfg.cfg.remora.opermode].mini << 4);
	reg26 |= (fxo_modes[fe->fe_cfg.cfg.remora.opermode].ilim << 1);
	WRITE_RM_REG(mod_no, 26, reg26);

	/* Set AC Impedence */
	reg30 = (unsigned char)(fxo_modes[fe->fe_cfg.cfg.remora.opermode].acim);
	WRITE_RM_REG(mod_no, 30, reg30);

	/* Misc. DAA parameters */
	reg31 = 0xa3;
	reg31 |= (fxo_modes[fe->fe_cfg.cfg.remora.opermode].ohs2 << 3);
	WRITE_RM_REG(mod_no, 31, reg31);

	/* Set Transmit/Receive timeslot */
	WRITE_RM_REG(mod_no, 34, mod_no * 8 + 1);
	WRITE_RM_REG(mod_no, 35, 0x00);
	WRITE_RM_REG(mod_no, 36, mod_no * 8 + 1);
	WRITE_RM_REG(mod_no, 37, 0x00);

	/* Enable ISO-Cap */
	WRITE_RM_REG(mod_no, 6, 0x00);

if (!fast) {
	/* Wait 1000ms for ISO-cap to come up */
	start_ticks = SYSTEM_TICKS + 2*HZ;
	while(!(READ_RM_REG(mod_no, 11) & 0xf0)){
		if (SYSTEM_TICKS > start_ticks){
			break;
		}
		wait_just_a_bit(HZ/10, fast);
	}

	if (!(READ_RM_REG(mod_no, 11) & 0xf0)) {
		DEBUG_EVENT(
		"%s: Module %d: VoiceDAA did not bring up ISO link properly!\n",
					fe->name,
					mod_no);
		return -1;
	}
	DEBUG_TEST("%s: Module %d: ISO-Cap is now up, line side: %02x rev %02x\n", 
			fe->name,
			mod_no,
			READ_RM_REG(mod_no, 11) >> 4,
			(READ_RM_REG(mod_no, 13) >> 2) & 0xf);
} else {
	WP_DELAY(10000);
}

	/* Enable on-hook line monitor */
	WRITE_RM_REG(mod_no, 5, 0x08);
	WRITE_RM_REG(mod_no, 3, 0x00);
#if defined(AFT_RM_INTR_SUPPORT)
	WRITE_RM_REG(mod_no, 2, 0x87/*0x84*/);
	fe->rm_param.mod[mod_no].u.fxo.imask = 0xFF;
#else
	fe->rm_param.mod[mod_no].u.fxo.imask = 0x00;
#endif		

	/* NZ -- crank the tx gain up by 7 dB */
	if (!strcmp(fxo_modes[fe->fe_cfg.cfg.remora.opermode].name, "NEWZEALAND")) {
		DEBUG_EVENT("%s: Module %d: Adjusting gain\n",
					fe->name,
					mod_no);
		WRITE_RM_REG(mod_no, 38, 0x7);
	}

	return 0;
}


/******************************************************************************
** wp_remora_iface_init) - 
**
**	OK
*/
int wp_remora_iface_init(void *pfe_iface)
{
	sdla_fe_iface_t	*fe_iface = (sdla_fe_iface_t*)pfe_iface;

	fe_iface->config	= &wp_remora_config;
	fe_iface->unconfig	= &wp_remora_unconfig;
	fe_iface->if_config	= &wp_remora_if_config;
	fe_iface->if_unconfig	= &wp_remora_if_unconfig;
	fe_iface->active_map	= &wp_remora_active_map;
	fe_iface->isr		= &wp_remora_intr;
	fe_iface->disable_irq	= &wp_remora_disable_irq;
	fe_iface->check_isr	= &wp_remora_check_intr;
	fe_iface->polling	= &wp_remora_polling;
	fe_iface->process_udp	= &wp_remora_udp;
	fe_iface->get_fe_media	= &wp_remora_fe_media;
	fe_iface->set_dtmf	= &wp_remora_set_dtmf;
	fe_iface->intr_ctrl	= &wp_remora_intr_ctrl;
	fe_iface->event_ctrl	= &wp_remora_event_ctrl;
#if defined(AFT_TDM_API_SUPPORT)
	fe_iface->watchdog	= &wp_remora_watchdog;
#endif

	return 0;
}

/******************************************************************************
** wp_remora_opermode() - 
**
**	OK
*/
static int wp_remora_opermode(sdla_fe_t *fe)
{
	sdla_fe_cfg_t	*fe_cfg = &fe->fe_cfg;

	if (!strlen(fe_cfg->cfg.remora.opermode_name)){
		memcpy(fe_cfg->cfg.remora.opermode_name, "FCC", 3);
		fe_cfg->cfg.remora.opermode = 0;
	}else{
		int x;
		for (x=0;x<(sizeof(fxo_modes) / sizeof(fxo_modes[0])); x++) {
			if (!strcmp(fxo_modes[x].name, fe_cfg->cfg.remora.opermode_name))
				break;
		}
		if (x < sizeof(fxo_modes) / sizeof(fxo_modes[0])) {
			fe_cfg->cfg.remora.opermode = x;
		} else {
			DEBUG_EVENT(
			"%s: Invalid/unknown operating mode '%s' specified\n",
						fe->name,
						fe_cfg->cfg.remora.opermode_name);
			DEBUG_EVENT(
			"%s: Please choose one of:\n",
						fe->name);
			for (x=0;x<sizeof(fxo_modes) / sizeof(fxo_modes[0]); x++)
				DEBUG_EVENT("%s:    %s\n",
					fe->name, fxo_modes[x].name);
			return -ENODEV;
		}
	}
	
	return 0;
}

/******************************************************************************
** wp_remora_config() - 
**
**	OK
*/
static int wp_remora_config(void *pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	int		err=0, mod_no, mod_cnt = 0, err_cnt = 0, retry;
	int		sane=0;

	DEBUG_EVENT("%s: Configuring FXS/FXO Front End ...\n",
        		     	fe->name);
	fe->rm_param.max_fe_channels 	= MAX_REMORA_MODULES;
	fe->rm_param.module_map 	= 0;
	fe->rm_param.intcount		= 0;
	fe->rm_param.last_watchdog 	= SYSTEM_TICKS;

	if (wp_remora_opermode(fe)){
		return -EINVAL;
	}

	wait_just_a_bit(HZ, 0);
	/* Reset SPI interface */
	wp_remora_reset_spi(fe);

	/* Search for installed modules and enable chain for all modules */
	if (wp_remora_chain_enable(fe)){
		DEBUG_EVENT("%s: Failed enable chain mode for all modules!\n",
				fe->name);
		return -EINVAL;
	}

	/* Auto detect FXS and FXO modules */
	for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no++){
		sane = 0; err = 0; retry = 0;
retry_cfg:
		switch(fe->rm_param.mod[mod_no].type){
		case MOD_TYPE_FXS:
			if (!(err = wp_init_proslic(fe, mod_no, 0, sane))){
				DEBUG_EVENT(
				"%s: Module %d: Installed -- Auto FXS!\n",
					fe->name,
					mod_no);	
				wan_set_bit(mod_no, &fe->rm_param.module_map);
				fe->rm_param.mod[mod_no].u.fxs.oldrxhook = 1;	/* default (off-hook) */
				mod_cnt++;
#if 0
			}else{
				DEBUG_EVENT(
				"%s: Module %d: FXS failed!\n",
						fe->name,
						mod_no);
				err_cnt++;
#endif 
			}
			break;

		case MOD_TYPE_FXO:
			err = wp_init_voicedaa(fe, mod_no, 0, sane);
			if (!err){
				DEBUG_EVENT(
				"%s: Module %d: Installed -- Auto FXO (%s mode)!\n",
					fe->name,
					mod_no,
					fxo_modes[fe->fe_cfg.cfg.remora.opermode].name);
				wan_set_bit(mod_no, &fe->rm_param.module_map);
				mod_cnt++;
#if 0
			}else{
				DEBUG_EVENT(
				"%s: Module %d: FXO failed!\n",
						fe->name,
						mod_no);
				err_cnt++;
#endif
			}
			break;

		case MOD_TYPE_TEST:
			DEBUG_EVENT(
			"%s: Module %d: Installed -- FXS/FXO tester!\n",
					fe->name,
					mod_no);
			wan_set_bit(mod_no, &fe->rm_param.module_map);
			mod_cnt++;
			break;
		default:
			DEBUG_TDMV("%s: Module %d: Not Installed!\n",
						fe->name,
						mod_no);	
			break;
		}
		if (err/* && !sane*/){
			sane = 1;
			if (retry++ < 10) goto retry_cfg;
			DEBUG_EVENT("%s: Module %d: %s failed!\n",
				fe->name,
				mod_no,
				WP_REMORA_DECODE_TYPE(fe->rm_param.mod[mod_no].type));
			err_cnt++;
		}else{
#if defined(__WINDOWS__)
			//FIXME: should be fixed for linux too?
			//if not reset to zero, initialization will fail even
			//if "retry_cfg" was SUCCESSFUL!!
			err_cnt = 0;
#endif
		}
#if !defined(__WINDOWS__)
		WAN_LIST_INIT(&fe->rm_param.mod[mod_no].event_head);
#endif
	}

	if (err_cnt){
		DEBUG_EVENT("%s: %d FXO/FXS module(s) are failed to initialize!\n",
					fe->name, err_cnt);
		return -EINVAL;
	}
	
#if defined(__WINDOWS__)
	fe->remora_modules_counter = mod_cnt;
#endif

	if (mod_cnt == 0){
		DEBUG_EVENT("%s: No FXO/FXS modules are found!\n",
					fe->name);
	//	return -EINVAL;
	}
	/* Initialize and start T1/E1 timer */
	wan_set_bit(WP_RM_TIMER_KILL,(void*)&fe->rm_param.critical);

	wan_init_timer(
		&fe->rm_param.timer, 
		wp_remora_timer,
	       	(wan_timer_arg_t)fe);
	
	wan_clear_bit(WP_RM_TIMER_KILL,(void*)&fe->rm_param.critical);
	wan_clear_bit(WP_RM_TIMER_RUNNING,(void*)&fe->rm_param.critical);
	wan_set_bit(WP_RM_CONFIGURED,(void*)&fe->rm_param.critical);

#if 0
	wp_remora_event_ctrl(
			fe, 0,
			WAN_RM_EVENT_DTMF|WAN_RM_EVENT_LC|WAN_RM_EVENT_RT,
			WAN_RM_EVENT_ENABLE, 0x00);
#endif

	return 0;
}

/******************************************************************************
** wp_remora_unconfig() - 
**
**	OK
*/
static int wp_remora_unconfig(void *pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	int		mod_no;

	DEBUG_EVENT("%s: Unconfiguring FXS/FXO Front End...\n",
        		     	fe->name);

	/* Disable interrupt (should be done before ) */				
	wp_remora_disable_irq(fe);
					
	/* Clear and Kill TE timer poll command */
	wan_clear_bit(WP_RM_CONFIGURED,(void*)&fe->rm_param.critical);
	wan_set_bit(WP_RM_TIMER_KILL,(void*)&fe->rm_param.critical);

	if (wan_test_bit(WP_RM_TIMER_RUNNING,(void*)&fe->rm_param.critical)){
		wan_del_timer(&fe->rm_param.timer);
	}

	wan_clear_bit(WP_RM_TIMER_RUNNING,(void*)&fe->rm_param.critical);
	fe->rm_param.timer_cmd = 0x00;

	for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no++){
		if (wan_test_bit(mod_no, &fe->rm_param.module_map)) {
			wan_clear_bit(mod_no, &fe->rm_param.module_map);
		}
	}
	return 0;
}

/******************************************************************************
** wp_remora_if_config() - 
**
**	OK
*/
static int wp_remora_if_config(void *pfe, u32 mod_map, u8 usedby)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	int		mod_no;
	
	if (!wan_test_bit(WP_RM_CONFIGURED,(void*)&fe->rm_param.critical)){
		return -EINVAL;
	}

#if defined(AFT_RM_INTR_SUPPORT)
	/* Enable remora interrupts (only for TDM_API) */
	if (usedby == TDM_VOICE_API){
		for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no++){
			if (!wan_test_bit(mod_no, &fe->rm_param.module_map) ||
			    !wan_test_bit(mod_no, &mod_map)){
				continue;
			}
			wp_remora_intr_ctrl(	fe, 
						mod_no, 
						WAN_RM_INTR_GLOBAL, 
						WAN_FE_INTR_ENABLE, 
						0x00);
		}
	}
#endif	
	return 0;
}

/******************************************************************************
** wp_remora_if_unconfig() - 
**
**	OK
*/
static int wp_remora_if_unconfig(void *pfe, u32 mod_map, u8 usedby)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	int		mod_no;
	
	if (!wan_test_bit(WP_RM_CONFIGURED,(void*)&fe->rm_param.critical)){
		return -EINVAL;
	}

#if defined(AFT_RM_INTR_SUPPORT)
	for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no++){
		if (!wan_test_bit(mod_no, &fe->rm_param.module_map) ||
		    !wan_test_bit(mod_no, &mod_map)){
		    continue;
		}
		wp_remora_intr_ctrl(	fe, 
					mod_no, 
					WAN_RM_INTR_GLOBAL, 
					WAN_FE_INTR_MASK, 
					0x00);
	}
#endif
	return 0;

}

/******************************************************************************
** wp_remora_disable_irq() - 
**
**	OK
*/
static int wp_remora_disable_irq(void *pfe)
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	int		mod_no;

	if (!wan_test_bit(WP_RM_CONFIGURED,(void*)&fe->rm_param.critical)){
		return -EINVAL;
	}

	for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no++){
		if (wan_test_bit(mod_no, &fe->rm_param.module_map)) {
			wp_remora_intr_ctrl(	fe, 
						mod_no, 
						WAN_RM_INTR_GLOBAL, 
						WAN_FE_INTR_MASK, 
						0x00);
		}
	}
	return 0;
}

static unsigned int wp_remora_active_map(sdla_fe_t* fe)
{
	return fe->rm_param.module_map;
}

/******************************************************************************
 *				wp_remora_fe_status()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static unsigned char wp_remora_fe_media(sdla_fe_t *fe)
{
	return fe->fe_cfg.media;
}

/******************************************************************************
 *				wp_remora_set_dtmf()	
 *
 * Description:
 * Arguments:	
 * Returns:
 ******************************************************************************
 */
static int wp_remora_set_dtmf(sdla_fe_t *fe, int mod_no, unsigned char val)
{

	if (mod_no > MAX_REMORA_MODULES){
		DEBUG_EVENT("%s: Module %d: Module number out of range!\n",
					fe->name, mod_no);
		return -EINVAL;
	}	
	if (!wan_test_bit(mod_no-1, &fe->rm_param.module_map)){
		DEBUG_EVENT("%s: Module %d: Not configures yet!\n",
					fe->name, mod_no);
		return -EINVAL;
	}
	
	return -EINVAL;
}

/*
 ******************************************************************************
 *				sdla_remora_timer()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static void wp_remora_timer(void* pfe)
#elif defined(__WINDOWS__)
static void wp_remora_timer(IN PKDPC Dpc, void* pfe, void* arg2, void* arg3)
#else
static void wp_remora_timer(unsigned long pfe)
#endif
{
	sdla_fe_t	*fe = (sdla_fe_t*)pfe;
	sdla_t 		*card = (sdla_t*)fe->card;
	wan_device_t	*wandev = &card->wandev;

	if (wan_test_bit(WP_RM_TIMER_KILL,(void*)&fe->rm_param.critical)){
		wan_clear_bit(WP_RM_TIMER_RUNNING,(void*)&fe->rm_param.critical);
		return;
	}
	/*WAN_ASSERT1(wandev->fe_enable_timer == NULL); */
	DEBUG_TIMER("%s: RM timer!\n", fe->name);
	/* Enable hardware interrupt for Analog */

	if (wandev->fe_enable_timer){
		wandev->fe_enable_timer(fe->card);
	}else{
		wp_remora_polling(fe);
	}

	return;
}


/*
 ******************************************************************************
 *				wp_remora_enable_timer()	
 *
 * Description: Enable software timer interrupt in delay ms.
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static void
wp_remora_enable_timer(sdla_fe_t* fe, int mod_no, unsigned char cmd, unsigned int delay)
{
	sdla_t	*card = (sdla_t*)fe->card;

	WAN_ASSERT1(card == NULL);

	DEBUG_TEST("%s: %s:%d Cmd=0x%X\n",
			fe->name,__FUNCTION__,__LINE__,cmd);

#if defined (__WINDOWS__)
	{
		int rc=SILENT;
		/* make sure not called from ISR */
		VERIFY_DISPATCH_IRQL(rc);
		if(rc){
			DEBUG_EVENT("%s(): Error: invalid IRQL! current irql: %d\n",
				__FUNCTION__, KeGetCurrentIrql());
			return;
		}
	}
#endif
	if (wan_test_bit(WP_RM_TIMER_KILL,(void*)&fe->rm_param.critical)){
		wan_clear_bit(
				WP_RM_TIMER_RUNNING,
				(void*)&fe->rm_param.critical);
		return;
	}
	
	if (wan_test_bit(WP_RM_TIMER_RUNNING,(void*)&fe->rm_param.critical)){
		if (fe->rm_param.timer_cmd == cmd){
			/* Just ignore current request */
			return;
		}
		DEBUG_TEST("%s: WP_RM_TIMER_RUNNING: new_cmd=%X curr_cmd=%X\n",
					fe->name,
					cmd,
					fe->rm_param.timer_cmd);
		return;
	}

	wan_set_bit(WP_RM_TIMER_RUNNING,(void*)&fe->rm_param.critical);
	
	fe->rm_param.timer_cmd		= cmd;
	fe->rm_param.timer_mod_no	= mod_no;
	wan_add_timer(&fe->rm_param.timer, delay * HZ / 1000);
	return;	
}

static int wp_remora_polling_event(sdla_fe_t* fe, int mod_no)
{
	wan_event_ctrl_t	*ectrl;

#if defined(__WINDOWS__)
	ectrl = fe->rm_param.mod[mod_no].current_control_event_ptr;
	if(ectrl == NULL){
		TDM_FUNC_DBG
		return 0;
	}
#else
	if (WAN_LIST_EMPTY(&fe->rm_param.mod[mod_no].event_head)){
		return 0;
	}
	ectrl = WAN_LIST_FIRST(&fe->rm_param.mod[mod_no].event_head);
	WAN_LIST_REMOVE(ectrl, next);
#endif

	DEBUG_EVENT("%s: Module %d: %s %s events!\n",
			fe->name, mod_no,
			WAN_EVENT_MODE_DECODE(ectrl->mode),
			WAN_EVENT_TYPE_DECODE(ectrl->type));
	if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXS){	
		unsigned char		imask;
		
		switch(ectrl->type){
		case WAN_EVENT_RM_POWER:
			imask = READ_RM_REG(mod_no, 22);
			if (ectrl->mode == WAN_EVENT_ENABLE){
				imask |= 0xFC;
				wan_set_bit(WAN_RM_EVENT_POWER,&fe->rm_param.mod[mod_no].events);
			}else{
				imask &= ~0xFC;
				wan_clear_bit(WAN_RM_EVENT_POWER,&fe->rm_param.mod[mod_no].events);
			}
			WRITE_RM_REG(mod_no, 22, imask);
			break;
		case WAN_EVENT_RM_LC:
			imask = READ_RM_REG(mod_no, 22);
			if (ectrl->mode == WAN_EVENT_ENABLE){
				imask |= 0x02;
				wan_set_bit(WAN_RM_EVENT_LC, &fe->rm_param.mod[mod_no].events);
			}else{
				imask &= ~0x02;
				wan_clear_bit(WAN_RM_EVENT_LC, &fe->rm_param.mod[mod_no].events);
			}
			WRITE_RM_REG(mod_no, 22, imask);
			break;
		case WAN_EVENT_RM_RING_TRIP:
			imask = READ_RM_REG(mod_no, 22);
			if (ectrl->mode == WAN_EVENT_ENABLE){
				imask |= 0x01;
				wan_set_bit(WAN_RM_EVENT_RING_TRIP, &fe->rm_param.mod[mod_no].events);
			}else{
				imask &= ~0x01;
				wan_clear_bit(WAN_RM_EVENT_RING_TRIP, &fe->rm_param.mod[mod_no].events);
			}			
			WRITE_RM_REG(mod_no, 22, imask);
			break;
		case WAN_EVENT_RM_DTMF:
			imask = READ_RM_REG(mod_no, 23);
			if (ectrl->mode == WAN_EVENT_ENABLE){
				imask |= 0x01;
				wan_set_bit(WAN_RM_EVENT_DTMF, &fe->rm_param.mod[mod_no].events);
			}else{
				imask &= ~0x01;
				wan_clear_bit(WAN_RM_EVENT_DTMF, &fe->rm_param.mod[mod_no].events);
			}
			WRITE_RM_REG(mod_no, 23, imask);
			break;	
		case WAN_EVENT_RM_RING:
			if (ectrl->mode == WAN_EVENT_ENABLE){
				WRITE_RM_REG(fe->rm_param.timer_mod_no, 64, 0x04);
				wan_set_bit(WAN_RM_EVENT_RING, &fe->rm_param.mod[mod_no].events);
			}else{
				WRITE_RM_REG(fe->rm_param.timer_mod_no, 64, 0x01);
				wan_clear_bit(WAN_RM_EVENT_RING, &fe->rm_param.mod[mod_no].events);
			}
			break;
		case WAN_EVENT_RM_TONE:
			if (ectrl->mode == WAN_EVENT_ENABLE){
				DEBUG_EVENT("%s: Module %d: %s %s events (%d)!\n",
						fe->name, mod_no,
						WAN_EVENT_MODE_DECODE(ectrl->mode),
						WAN_EVENT_TYPE_DECODE(ectrl->type),
						ectrl->tone);
				switch(ectrl->tone){
				case WAN_EVENT_TONE_DIAL:
					wp_remora_dialtone(fe, mod_no);
					break;
				case WAN_EVENT_TONE_BUSY:
					wp_remora_busytone(fe, mod_no);	
					break;
				case WAN_EVENT_TONE_RING:
					wp_remora_ringtone(fe, mod_no);	
					break;
				case WAN_EVENT_TONE_CONGESTION:
					wp_remora_congestiontone(fe, mod_no);	
					break;	
				}
				wan_set_bit(WAN_RM_EVENT_TONE, &fe->rm_param.mod[mod_no].events);
			}else{
				wp_remora_disabletone(fe, mod_no);
				wan_clear_bit(WAN_RM_EVENT_TONE, &fe->rm_param.mod[mod_no].events);
			}
			break;
		case WAN_EVENT_RM_TXSIG_KEWL:
			fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 0;
			WRITE_RM_REG(mod_no, 64, fe->rm_param.mod[mod_no].u.fxs.lasttxhook);
			break;
		case WAN_EVENT_RM_TXSIG_START:
			DEBUG_RM("%s: Module %d: txsig START.\n",
					fe->name, mod_no);
			fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 4;
			WRITE_RM_REG(mod_no, 64, fe->rm_param.mod[mod_no].u.fxs.lasttxhook);
			break;
		case WAN_EVENT_RM_TXSIG_OFFHOOK:
			DEBUG_RM("%s: Module %d: goes off-hook.\n",
					fe->name, mod_no+1);
			switch(fe->rm_param.mod[mod_no].sig) {
			case WAN_RM_SIG_EM:
				fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 5;
				break;
			default:
				fe->rm_param.mod[mod_no].u.fxs.lasttxhook =
					fe->rm_param.mod[mod_no].u.fxs.idletxhookstate;
				break;
			}
			WRITE_RM_REG(mod_no, 64, fe->rm_param.mod[mod_no].u.fxs.lasttxhook);
			break;
		case WAN_EVENT_RM_TXSIG_ONHOOK:
			DEBUG_RM("%s: Module %d: goes on-hook.\n",
					fe->name, mod_no+1);
			switch(fe->rm_param.mod[mod_no].sig) {
			case WAN_RM_SIG_EM:
			case WAN_RM_SIG_FXOKS:
			case WAN_RM_SIG_FXOLS:
				fe->rm_param.mod[mod_no].u.fxs.lasttxhook =
					fe->rm_param.mod[mod_no].u.fxs.idletxhookstate;
				break;
			case WAN_RM_SIG_FXOGS:
				fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 3;
				break;
			}
			WRITE_RM_REG(mod_no, 64, fe->rm_param.mod[mod_no].u.fxs.lasttxhook);
			break;
		case WAN_EVENT_RM_ONHOOKTRANSFER:
			fe->rm_param.mod[mod_no].u.fxs.ohttimer = ectrl->ohttimer << 3;
			if (fe->fe_cfg.cfg.remora.reversepolarity){
				/* OHT mode when idle */
				fe->rm_param.mod[mod_no].u.fxs.idletxhookstate  = 0x6;
			}else{
				fe->rm_param.mod[mod_no].u.fxs.idletxhookstate  = 0x2;
			}
			if (fe->rm_param.mod[mod_no].u.fxs.lasttxhook == 0x1) {
				/* Apply the change if appropriate */
				if (fe->fe_cfg.cfg.remora.reversepolarity){
					fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 0x6;
				}else{
					fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 0x2;
				}
				WRITE_RM_REG(mod_no, 64, fe->rm_param.mod[mod_no].u.fxs.lasttxhook);
			}
			break;
		case WAN_EVENT_RM_SETPOLARITY:
			/* Can't change polarity while ringing or when open */
			if ((fe->rm_param.mod[mod_no].u.fxs.lasttxhook == 0x04) ||
			    (fe->rm_param.mod[mod_no].u.fxs.lasttxhook == 0x00)){
				return -EINVAL;
			}
	
			if ((ectrl->mode && !fe->fe_cfg.cfg.remora.reversepolarity) ||
			    (!ectrl->mode && fe->fe_cfg.cfg.remora.reversepolarity)){
				fe->rm_param.mod[mod_no].u.fxs.lasttxhook |= 0x04;
			}else{
				fe->rm_param.mod[mod_no].u.fxs.lasttxhook &= ~0x04;
			}
			WRITE_RM_REG(mod_no, 64, fe->rm_param.mod[mod_no].u.fxs.lasttxhook);
			break;
			
		default:
			DEBUG_RM("%s: Module %d: Unknown FXS event type %X.\n",
					fe->name, mod_no, ectrl->type);
			break;		
		}
		DEBUG_RM("%s: INTR mask %02X %02X %02X\n",
				fe->name,
				READ_RM_REG(mod_no, 21),
				READ_RM_REG(mod_no, 22),
				READ_RM_REG(mod_no, 23));
#if !defined(__WINDOWS__)
		wan_free(ectrl);
#endif
	}else if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXO){
	
		unsigned char		imask;
		
		switch(ectrl->type){
		case WAN_EVENT_RM_RING_DETECT:
			imask = READ_RM_REG(mod_no, 3);
			if (ectrl->mode == WAN_EVENT_ENABLE){
				imask |= 0x80;
				wan_set_bit(WAN_RM_EVENT_RING_DETECT, &fe->rm_param.mod[mod_no].events);
			}else{
				imask &= ~0x80;
				wan_clear_bit(WAN_RM_EVENT_RING_DETECT, &fe->rm_param.mod[mod_no].events);
			}
			WRITE_RM_REG(mod_no, 3, imask);
			break;
		case WAN_EVENT_RM_TXSIG_START:
		case WAN_EVENT_RM_TXSIG_OFFHOOK:
			DEBUG_TDMV("%s: Module %d: goes off-hook.\n", 
					fe->name, mod_no+1);
			fe->rm_param.mod[mod_no].u.fxo.offhook = 1;
			WRITE_RM_REG(mod_no, 5, 0x9);		
			break;
		case WAN_EVENT_RM_TXSIG_ONHOOK:
			DEBUG_TDMV("%s: Module %d: goes on-hook.\n", 
					fe->name, mod_no+1);
			fe->rm_param.mod[mod_no].u.fxo.offhook = 0;
			WRITE_RM_REG(mod_no, 5, 0x8);
			break;
		default:
			DEBUG_RM("%s: Module %d: Unknown FXO event type %X.\n",
					fe->name, mod_no, ectrl->type);
			break;		
		}
	}

#if defined(__WINDOWS__)
	//done with the event, reset the pointer.
	fe->rm_param.mod[mod_no].current_control_event_ptr = NULL;
#endif
	return 0;
}


static int wp_remora_polling(sdla_fe_t* fe)
{
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
	sdla_t	*card = (sdla_t*)fe->card;
	int	err = 0;
#endif
	int	mod_no = fe->rm_param.timer_mod_no;

	WAN_ASSERT(fe->write_fe_reg == NULL);
	WAN_ASSERT(fe->read_fe_reg == NULL);

	DEBUG_RM("%s: RM Polling State=%s Cmd=0x%X!\n", 
			fe->name, fe->fe_status==FE_CONNECTED?"Con":"Disconn",
			fe->rm_param.timer_cmd);

	wan_clear_bit(WP_RM_TIMER_RUNNING,(void*)&fe->rm_param.critical);
	
	switch(fe->rm_param.timer_cmd){

	case WP_RM_POLL_EVENT:
		wp_remora_polling_event(fe, mod_no);	
		break;
#if defined(CONFIG_PRODUCT_WANPIPE_TDM_VOICE)
	case WP_RM_POLL_TDMV:
#if defined(__WINDOWS__)
		//FIXME: implement
		TDM_FUNC_DBG
#else
		WAN_TDMV_CALL(polling, (card), err);
#endif
		break;
#endif
	case WP_RM_POLL_INIT:
		wp_init_proslic(fe, mod_no, 1, 1);
		break;
	}
	return 0;
}

static int wp_remora_dialtone(sdla_fe_t *fe, int mod_no)
{

	DEBUG_EVENT("%s: Module %d: Enable Dial tone\n",
				fe->name, mod_no);
	wp_proslic_setreg_indirect(fe, mod_no, 13,DIALTONE_IR13);
	wp_proslic_setreg_indirect(fe, mod_no, 14,DIALTONE_IR14);
	wp_proslic_setreg_indirect(fe, mod_no, 16,DIALTONE_IR16);
	wp_proslic_setreg_indirect(fe, mod_no, 17,DIALTONE_IR17);

	WRITE_RM_REG(mod_no, 36,  DIALTONE_DR36);
	WRITE_RM_REG(mod_no, 37,  DIALTONE_DR37);
	WRITE_RM_REG(mod_no, 38,  DIALTONE_DR38);
	WRITE_RM_REG(mod_no, 39,  DIALTONE_DR39);
	WRITE_RM_REG(mod_no, 40,  DIALTONE_DR40);
	WRITE_RM_REG(mod_no, 41,  DIALTONE_DR41);
	WRITE_RM_REG(mod_no, 42,  DIALTONE_DR42);
	WRITE_RM_REG(mod_no, 43,  DIALTONE_DR43);

	WRITE_RM_REG(mod_no, 32,  DIALTONE_DR32);
	WRITE_RM_REG(mod_no, 33,  DIALTONE_DR33);
	return 0;
}

static int wp_remora_busytone(sdla_fe_t *fe, int mod_no)
{

	DEBUG_EVENT("%s: Module %d: Enable Busy tone\n",
				fe->name, mod_no);
	wp_proslic_setreg_indirect(fe, mod_no, 13,BUSYTONE_IR13);
	wp_proslic_setreg_indirect(fe, mod_no, 14,BUSYTONE_IR14);
	wp_proslic_setreg_indirect(fe, mod_no, 16,BUSYTONE_IR16);
	wp_proslic_setreg_indirect(fe, mod_no, 17,BUSYTONE_IR17);

	WRITE_RM_REG(mod_no, 6,  BUSYTONE_DR36);
	WRITE_RM_REG(mod_no, 37,  BUSYTONE_DR37);
	WRITE_RM_REG(mod_no, 38,  BUSYTONE_DR38);
	WRITE_RM_REG(mod_no, 39,  BUSYTONE_DR39);
	WRITE_RM_REG(mod_no, 40,  BUSYTONE_DR40);
	WRITE_RM_REG(mod_no, 41,  BUSYTONE_DR41);
	WRITE_RM_REG(mod_no, 42,  BUSYTONE_DR42);
	WRITE_RM_REG(mod_no, 43,  BUSYTONE_DR43);
  
	WRITE_RM_REG(mod_no, 32,  BUSYTONE_DR32);
	WRITE_RM_REG(mod_no, 33,  BUSYTONE_DR33);
	return 0;
}

static int wp_remora_ringtone(sdla_fe_t *fe, int mod_no)
{

	DEBUG_EVENT("%s: Module %d: Enable Ring tone\n",
				fe->name, mod_no);
	wp_proslic_setreg_indirect(fe, mod_no, 13,RINGBACKTONE_IR13);
	wp_proslic_setreg_indirect(fe, mod_no, 14,RINGBACKTONE_IR14);
	wp_proslic_setreg_indirect(fe, mod_no, 16,RINGBACKTONE_IR16);
	wp_proslic_setreg_indirect(fe, mod_no, 17,RINGBACKTONE_IR17);
	
	WRITE_RM_REG(mod_no, 36,  RINGBACKTONE_DR36);
	WRITE_RM_REG(mod_no, 37,  RINGBACKTONE_DR37);
	WRITE_RM_REG(mod_no, 38,  RINGBACKTONE_DR38);
	WRITE_RM_REG(mod_no, 39,  RINGBACKTONE_DR39);
	WRITE_RM_REG(mod_no, 40,  RINGBACKTONE_DR40);
	WRITE_RM_REG(mod_no, 41,  RINGBACKTONE_DR41);
	WRITE_RM_REG(mod_no, 42,  RINGBACKTONE_DR42);
	WRITE_RM_REG(mod_no, 43,  RINGBACKTONE_DR43);
  
	WRITE_RM_REG(mod_no, 32,  RINGBACKTONE_DR32);
	WRITE_RM_REG(mod_no, 33,  RINGBACKTONE_DR33);
	return 0;
}

static int wp_remora_congestiontone(sdla_fe_t *fe, int mod_no)
{

	DEBUG_EVENT("%s: Module %d: Enable Congestion tone\n",
				fe->name, mod_no);
	wp_proslic_setreg_indirect(fe, mod_no, 13,CONGESTIONTONE_IR13);
	wp_proslic_setreg_indirect(fe, mod_no, 14,CONGESTIONTONE_IR14);
	wp_proslic_setreg_indirect(fe, mod_no, 16,CONGESTIONTONE_IR16);
	wp_proslic_setreg_indirect(fe, mod_no, 17,CONGESTIONTONE_IR17);
	
	WRITE_RM_REG(mod_no, 36,  CONGESTIONTONE_DR36);
	WRITE_RM_REG(mod_no, 37,  CONGESTIONTONE_DR37);
	WRITE_RM_REG(mod_no, 38,  CONGESTIONTONE_DR38);
	WRITE_RM_REG(mod_no, 39,  CONGESTIONTONE_DR39);
	WRITE_RM_REG(mod_no, 40,  CONGESTIONTONE_DR40);
	WRITE_RM_REG(mod_no, 41,  CONGESTIONTONE_DR41);
	WRITE_RM_REG(mod_no, 42,  CONGESTIONTONE_DR42);
	WRITE_RM_REG(mod_no, 43,  CONGESTIONTONE_DR43);

	WRITE_RM_REG(mod_no, 32,  CONGESTIONTONE_DR32);
	WRITE_RM_REG(mod_no, 33,  CONGESTIONTONE_DR33);
	return 0;
}

static int wp_remora_disabletone(sdla_fe_t* fe, int mod_no)
{
	WRITE_RM_REG(mod_no, 32, 0);
	WRITE_RM_REG(mod_no, 33, 0);
	WRITE_RM_REG(mod_no, 36, 0);
	WRITE_RM_REG(mod_no, 37, 0);
	WRITE_RM_REG(mod_no, 38, 0);
	WRITE_RM_REG(mod_no, 39, 0);
	WRITE_RM_REG(mod_no, 40, 0);
	WRITE_RM_REG(mod_no, 41, 0);
	WRITE_RM_REG(mod_no, 42, 0);
	WRITE_RM_REG(mod_no, 43, 0);

	return 0;
}


static int wp_remora_regdump(sdla_fe_t* fe, unsigned char *data)
{
	wan_remora_udp_t	*rm_udp = (wan_remora_udp_t*)data;
	wan_remora_fxs_regs_t	*regs_fxs = NULL;
	wan_remora_fxo_regs_t	*regs_fxo = NULL;
	int			mod_no = 0, reg = 0;

	mod_no = rm_udp->mod_no;
	if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXS){

		rm_udp->type = MOD_TYPE_FXS;
		regs_fxs = &rm_udp->u.regs_fxs;
		for(reg = 0; reg < WAN_FXS_NUM_REGS; reg++){
			regs_fxs->direct[reg] = READ_RM_REG(mod_no, reg);
		}

		for (reg=0; reg < WAN_FXS_NUM_INDIRECT_REGS; reg++){
			regs_fxs->indirect[reg] =
				(unsigned short)wp_proslic_getreg_indirect(fe, mod_no, (unsigned char)reg);
		}
	}else if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXO){

		rm_udp->type = MOD_TYPE_FXO;
		regs_fxo = &rm_udp->u.regs_fxo;
		for(reg = 0; reg < WAN_FXO_NUM_REGS; reg++){
			regs_fxo->direct[reg] = READ_RM_REG(mod_no, reg);
		}
	}else{
		return 0;
	}
	return sizeof(wan_remora_udp_t);
}

static int wp_remora_stats_voltage(sdla_fe_t* fe, unsigned char *data)
{
	wan_remora_udp_t	*rm_udp = (wan_remora_udp_t*)data;
	int			mod_no = 0;

	mod_no = rm_udp->mod_no;



	if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXS){
		rm_udp->type = MOD_TYPE_FXS;
		rm_udp->u.stats.tip_volt = READ_RM_REG(mod_no, 80);
		rm_udp->u.stats.ring_volt = READ_RM_REG(mod_no, 81);
		rm_udp->u.stats.bat_volt = READ_RM_REG(mod_no, 82);
	} else if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXO){
		rm_udp->type = MOD_TYPE_FXO;
		rm_udp->u.stats.volt = READ_RM_REG(mod_no, 29);
	} else{
		return 0;
	}

	return sizeof(wan_remora_udp_t);
}

/*
 ******************************************************************************
 *				wp_remora_udp()	
 *
 * Description:
 * Arguments:
 * Returns:
 ******************************************************************************
 */
static int wp_remora_udp(sdla_fe_t *fe, void* p_udp_cmd, unsigned char* data)
{
	wan_cmd_t	*udp_cmd = (wan_cmd_t*)p_udp_cmd;
	sdla_fe_debug_t	*fe_debug;	
	int		err = -EINVAL;

	switch(udp_cmd->wan_cmd_command){
#if 0
/* FIXME: Update wanpipemon to use API ioctl call */
	case WAN_FE_TONES:
		fe->rm_param.timer_mod_no = data[0]; 
		wp_remora_enable_timer(fe, WP_RM_POLL_TONE_DIAL, WP_RM_POLL_TIMER);
		udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		udp_cmd->wan_cmd_data_len = 0; 
		break;

	case WAN_FE_RING:
		fe->rm_param.timer_mod_no = data[0]; 
		wp_remora_enable_timer(fe, WP_RM_POLL_RING, WP_RM_POLL_TIMER);
		udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
		udp_cmd->wan_cmd_data_len = 0; 
		break;
#endif
	case WAN_FE_REGDUMP:
		err = wp_remora_regdump(fe, data);
		if (err){
			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			udp_cmd->wan_cmd_data_len = (unsigned short)err; 
		}
		break;

	case WAN_FE_STATS:
		err = wp_remora_stats_voltage(fe, data);
		if (err){
			udp_cmd->wan_cmd_return_code = WAN_CMD_OK;
			udp_cmd->wan_cmd_data_len = (unsigned short)err; 
		}
		break;
		
	case WAN_FE_SET_DEBUG_MODE:
		fe_debug = (sdla_fe_debug_t*)&data[0];
		switch(fe_debug->type){
		case WAN_FE_DEBUG_VOLTAGE:
			/* FIXME: Add code */
			break;
		}
		break;
		
	default:
		udp_cmd->wan_cmd_return_code = WAN_UDP_INVALID_CMD;
	    	udp_cmd->wan_cmd_data_len = 0;
		break;
	}
	return 0;
}


/******************************************************************************
*				wp_remora_event_ctrl()	
*
* Description: Enable/Disable event types
* Arguments: mod_no -  Module number (1,2,3,... MAX_REMORA_MODULES)
* Returns:
******************************************************************************/
//wp_remora_event_ctrl(sdla_fe_t *fe, int mod_no, int etype, int mode, unsigned long ts_map)
static int
wp_remora_event_ctrl(sdla_fe_t *fe, wan_event_ctrl_t *ectrl)
{
	int	mod_no, err = 0;

	WAN_ASSERT(ectrl == NULL);
	mod_no = ectrl->mod_no;

	if (mod_no >= MAX_REMORA_MODULES){
		DEBUG_EVENT("%s: Module %d: Module number is out of range!\n",
					fe->name, mod_no);
#if !defined(__WINDOWS__)
		wan_free(ectrl);
#endif
		return -EINVAL;
	}
	if (!wan_test_bit(mod_no, &fe->rm_param.module_map)) {
		DEBUG_EVENT("%s: Module %d: Unconfigured module!\n",
					fe->name, mod_no);
#if !defined(__WINDOWS__)
		wan_free(ectrl);
#endif
		return -EINVAL;
	}

	DEBUG_RM("%s: Module %d: Scheduling %s event type!\n",
				fe->name, mod_no,
				WAN_EVENT_TYPE_DECODE(ectrl->type));
			
#if defined(__WINDOWS__)
	TDM_FUNC_DBG
	if(	fe->rm_param.mod[mod_no].current_control_event_ptr != NULL){
		TDM_FUNC_DBG
		return -EBUSY;
	}
	fe->rm_param.mod[mod_no].current_control_event_ptr =
		&fe->rm_param.mod[mod_no].current_control_event;
	memcpy(	fe->rm_param.mod[mod_no].current_control_event_ptr,
			ectrl,
			sizeof(wan_event_ctrl_t));
#else
	if (WAN_LIST_EMPTY(&fe->rm_param.mod[mod_no].event_head)){
		WAN_LIST_INSERT_HEAD(&fe->rm_param.mod[mod_no].event_head, ectrl, next);
	}else{
		wan_free(ectrl);
		return -EBUSY;
	}
#endif
	wp_remora_enable_timer(fe, mod_no, WP_RM_POLL_EVENT, WP_RM_POLL_EVENT_TIMER);
	return err;
}


static int wp_init_proslic_recheck_sanity(sdla_fe_t *fe, int mod_no)
{
	int	res;
	
	res = READ_RM_REG(mod_no, 64);
	if (!res && (res != fe->rm_param.mod[mod_no].u.fxs.lasttxhook)) {
		res = READ_RM_REG(mod_no, 8);
		if (res) {
			DEBUG_EVENT(
			"%s: Module %d: Ouch, part reset, quickly restoring reality\n",
					fe->name, mod_no);
			wp_remora_enable_timer(fe, mod_no, WP_RM_POLL_INIT, WP_RM_POLL_TIMER);
			return 1;
			//wp_init_proslic(fe, mod_no, 1, 1);
		} else {
			if (fe->rm_param.mod[mod_no].u.fxs.palarms++ < MAX_ALARMS) {
				DEBUG_EVENT(
				"%s: Module %d: Power alarm, resetting!\n",
					fe->name, mod_no + 1);
				if (fe->rm_param.mod[mod_no].u.fxs.lasttxhook == 4){
					fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 1;
				}
				WRITE_RM_REG(mod_no, 64, fe->rm_param.mod[mod_no].u.fxs.lasttxhook);
			} else {
				if (fe->rm_param.mod[mod_no].u.fxs.palarms == MAX_ALARMS){
					DEBUG_EVENT(
					"%s: Module %d: Too many power alarms, NOT resetting!\n",
						fe->name, mod_no + 1);
				}
			}
		}
	}
	return 0;
}

#if defined(AFT_TDM_API_SUPPORT)
static void wp_remora_voicedaa_check_hook(sdla_fe_t *fe, int mod_no)
{
	sdla_t		*card = NULL;
	wan_event_t	event;
#ifndef AUDIO_RINGCHECK
	unsigned char res;
#endif	
	signed char b;
	int poopy = 0;

	WAN_ASSERT1(fe->card == NULL);
	card	= (sdla_t*)fe->card;

	/* Try to track issues that plague slot one FXO's */
	b = READ_RM_REG(mod_no, 5);
	if ((b & 0x2) || !(b & 0x8)) {
		/* Not good -- don't look at anything else */
		DEBUG_RM("%s: Module %d: Poopy (%02x)!\n",
				fe->name, mod_no + 1, b); 
		poopy++;
	}
	b &= 0x9b;
	if (fe->rm_param.mod[mod_no].u.fxo.offhook) {
		if (b != 0x9){
			WRITE_RM_REG(mod_no, 5, 0x9);
		}
	} else {
		if (b != 0x8){
			WRITE_RM_REG(mod_no, 5, 0x8);
		}
	}
	if (poopy)
		return;
#ifndef AUDIO_RINGCHECK
	if (!fe->rm_param.mod[mod_no].u.fxo.offhook) {
		res = READ_RM_REG(mod_no, 5);
		if ((res & 0x60) && fe->rm_param.mod[mod_no].u.fxo.battery) {
			fe->rm_param.mod[mod_no].u.fxo.ringdebounce += (WP_RM_CHUNKSIZE * 16);
			if (fe->rm_param.mod[mod_no].u.fxo.ringdebounce >= WP_RM_CHUNKSIZE * 64) {
				if (!fe->rm_param.mod[mod_no].u.fxo.wasringing) {
					fe->rm_param.mod[mod_no].u.fxo.wasringing = 1;
					DEBUG_RM("%s: Module %d: RING!\n",
							fe->name,
							mod_no + 1);
					event.type	= WAN_EVENT_RM_RING_DETECT;
					event.channel	= mod_no+1;
					event.ring_mode	= WAN_EVENT_RING_PRESENT;
					if (card->wandev.event_callback.ringdetect){
						card->wandev.event_callback.ringdetect(card, &event);
					}	
								
				}
				fe->rm_param.mod[mod_no].u.fxo.ringdebounce = WP_RM_CHUNKSIZE * 64;
			}
		} else {
			fe->rm_param.mod[mod_no].u.fxo.ringdebounce -= WP_RM_CHUNKSIZE * 4;
			if (fe->rm_param.mod[mod_no].u.fxo.ringdebounce <= 0) {
				if (fe->rm_param.mod[mod_no].u.fxo.wasringing) {
					fe->rm_param.mod[mod_no].u.fxo.wasringing = 0;
					DEBUG_RM("%s: Module %d: NO RING!\n",
							fe->name,
							mod_no + 1);
					event.type	= WAN_EVENT_RM_RING_DETECT;
					event.channel	= mod_no+1;
					event.ring_mode	= WAN_EVENT_RING_STOP;
					if (card->wandev.event_callback.ringdetect){
						card->wandev.event_callback.ringdetect(card, &event);
					}	
				}
				fe->rm_param.mod[mod_no].u.fxo.ringdebounce = 0;
			}
		}
	}
#endif

	b = READ_RM_REG(mod_no, 29);
	if (abs(b) < battthresh) {
		fe->rm_param.mod[mod_no].u.fxo.nobatttimer++;
#if 0
		if (wr->mod[mod_no].fxo.battery)
			printk("Battery loss: %d (%d debounce)\n", b, wr->mod[mod_no].fxo.battdebounce);
#endif
		if (fe->rm_param.mod[mod_no].u.fxo.battery &&
		    !fe->rm_param.mod[mod_no].u.fxo.battdebounce) {
			DEBUG_RM("%s Module %d: NO BATTERY!\n",
						fe->name,
						mod_no + 1);
			fe->rm_param.mod[mod_no].u.fxo.battery =  0;
#ifdef	JAPAN
			if ((!fe->rm_param.mod[mod_no].u.fxo.ohdebounce) &&
			     fe->rm_param.mod[mod_no].u.fxo.offhook) {
				DEBUG_RM("%s: Module %d: On-Hook status!\n",
							fe->name,
							mod_no + 1);
				event.type	= WAN_EVENT_RM_LC;
				event.channel	= mod_no+1;
				event.rxhook	= WAN_EVENT_RXHOOK_ON;
				if (card->wandev.event_callback.hook){
					card->wandev.event_callback.hook(
								card, &event);
				}
							
#ifdef	ZERO_BATT_RING
				fe->rm_param.mod[mod_no].u.fxo.onhook++;
#endif
			}
#else
			event.type	= WAN_EVENT_RM_LC;
			event.channel	= mod_no+1;
			event.rxhook	= WAN_EVENT_RXHOOK_ON;
			if (card->wandev.event_callback.hook){
				card->wandev.event_callback.hook(card, &event);
			}
#endif
			fe->rm_param.mod[mod_no].u.fxo.battdebounce = battdebounce;
		} else if (!fe->rm_param.mod[mod_no].u.fxo.battery)
			fe->rm_param.mod[mod_no].u.fxo.battdebounce = battdebounce;
	} else if (abs(b) > battthresh) {
		if (!fe->rm_param.mod[mod_no].u.fxo.battery &&
		    !fe->rm_param.mod[mod_no].u.fxo.battdebounce) {
			DEBUG_RM("%s: Module %d: BATTERY (%s)!\n",
						fe->name,
						mod_no + 1,
						(b < 0) ? "-" : "+");			    
#ifdef	ZERO_BATT_RING
			if (fe->rm_param.mod[mod_no].u.fxo.onhook) {
				fe->rm_param.mod[mod_no].u.fxo.onhook = 0;
				DEBUG_RM("%s: Module %d: Off-Hook status!\n",
							fe->name,
							mod_no + 1);
				event.type	= WAN_EVENT_RM_LC;
				event.channel	= mod_no+1;
				event.rxhook	= WAN_EVENT_RXHOOK_OFF;
				if (card->wandev.event_callback.hook){
					card->wandev.event_callback.hook(card, &event);
				}		
			}
#else
			DEBUG_RM("%s: Module %d: Off-Hook status!\n",
						fe->name,
						mod_no + 1);
			event.type	= WAN_EVENT_RM_LC;
			event.channel	= mod_no+1;
			event.rxhook	= WAN_EVENT_RXHOOK_OFF;
			if (card->wandev.event_callback.hook){
				card->wandev.event_callback.hook(card, &event);
			}
#endif
			fe->rm_param.mod[mod_no].u.fxo.battery = 1;
			fe->rm_param.mod[mod_no].u.fxo.nobatttimer = 0;
			fe->rm_param.mod[mod_no].u.fxo.battdebounce = battdebounce;
		} else if (fe->rm_param.mod[mod_no].u.fxo.battery)
			fe->rm_param.mod[mod_no].u.fxo.battdebounce = battdebounce;

		if (fe->rm_param.mod[mod_no].u.fxo.lastpol >= 0) {
		    if (b < 0) {
			fe->rm_param.mod[mod_no].u.fxo.lastpol = -1;
			fe->rm_param.mod[mod_no].u.fxo.polaritydebounce = POLARITY_DEBOUNCE;
		    }
		} 
		if (fe->rm_param.mod[mod_no].u.fxo.lastpol <= 0) {
		    if (b > 0) {
			fe->rm_param.mod[mod_no].u.fxo.lastpol = 1;
			fe->rm_param.mod[mod_no].u.fxo.polaritydebounce = POLARITY_DEBOUNCE;
		    }
		}
	} else {
		/* It's something else... */
		fe->rm_param.mod[mod_no].u.fxo.battdebounce = battdebounce;
	}
	if (fe->rm_param.mod[mod_no].u.fxo.battdebounce)
		fe->rm_param.mod[mod_no].u.fxo.battdebounce--;
	if (fe->rm_param.mod[mod_no].u.fxo.polaritydebounce) {
	        fe->rm_param.mod[mod_no].u.fxo.polaritydebounce--;
		if (fe->rm_param.mod[mod_no].u.fxo.polaritydebounce < 1) {
			if (fe->rm_param.mod[mod_no].u.fxo.lastpol !=
					fe->rm_param.mod[mod_no].u.fxo.polarity) {
				DEBUG_RM(
				"%s: Module %d: %lu Polarity reversed (%d -> %d)\n",
						fe->name,
						mod_no + 1,
						SYSTEM_TICKS, 
						fe->rm_param.mod[mod_no].u.fxo.polarity, 
						fe->rm_param.mod[mod_no].u.fxo.lastpol);
				if (fe->rm_param.mod[mod_no].u.fxo.polarity){
					/* FIXME: Add revers polarity event */
				}
				fe->rm_param.mod[mod_no].u.fxo.polarity =
						fe->rm_param.mod[mod_no].u.fxo.lastpol;
			}
		}
	}
	return;
}
#endif

#if !defined(AFT_RM_INTR_SUPPORT)
static void wp_remora_proslic_check_hook(sdla_fe_t *fe, int mod_no)
{
	sdla_t		*card = NULL;	
	wan_event_t	event;
	int		hook;
	char		res;

	WAN_ASSERT1(fe->card == NULL);
	card	= fe->card;
	/* For some reason we have to debounce the
	   hook detector.  */

	res = READ_RM_REG(mod_no, 68);
	hook = (res & 1);
	if (hook != fe->rm_param.mod[mod_no].u.fxs.lastrxhook) {
		/* Reset the debounce (must be multiple of 4ms) */
		fe->rm_param.mod[mod_no].u.fxs.debounce = 4 * (4 * 8);
		DEBUG_EVENT(
		"%s: Module %d: Resetting debounce hook %d, %d\n",
				fe->name, mod_no + 1, hook,
				fe->rm_param.mod[mod_no].u.fxs.debounce);
	} else {
		if (fe->rm_param.mod[mod_no].u.fxs.debounce > 0) {
			fe->rm_param.mod[mod_no].u.fxs.debounce-= 16 * WP_RM_CHUNKSIZE;
			DEBUG_EVENT(
			"%s: Module %d: Sustaining hook %d, %d\n",
					fe->name, mod_no + 1,
					hook, fe->rm_param.mod[mod_no].u.fxs.debounce);
			if (!fe->rm_param.mod[mod_no].u.fxs.debounce) {
				DEBUG_EVENT(
				"%s: Module %d: Counted down debounce, newhook: %d\n",
							fe->name,
							mod_no + 1,
							hook);
				fe->rm_param.mod[mod_no].u.fxs.debouncehook = hook;
			}
			if (!fe->rm_param.mod[mod_no].u.fxs.oldrxhook &&
			    fe->rm_param.mod[mod_no].u.fxs.debouncehook) {
				/* Off hook */
				DEBUG_RM("%s: Module %d: Going off hook\n",
							fe->name, mod_no + 1);
				event.type	= WAN_EVENT_RM_LC;
				event.channel	= mod_no+1;
				event.rxhook	= WAN_EVENT_RXHOOK_OFF;
				if (card->wandev.event_callback.hook){
					card->wandev.event_callback.hook(card, &event);
				}		
				//zt_hooksig(&wr->chans[mod_no], ZT_RXSIG_OFFHOOK);
#if 0
				if (robust)
					wp_init_proslic(wc, card, 1, 0, 1);
#endif
				fe->rm_param.mod[mod_no].u.fxs.oldrxhook = 1;
			
			} else if (fe->rm_param.mod[mod_no].u.fxs.oldrxhook &&
				   !fe->rm_param.mod[mod_no].u.fxs.debouncehook) {
				/* On hook */
				DEBUG_RM("%s: Module %d: Going on hook\n",
							fe->name, mod_no + 1);
				event.type	= WAN_EVENT_RM_LC;
				event.channel	= mod_no+1;
				event.rxhook	= WAN_EVENT_RXHOOK_OFF;
				if (card->wandev.event_callback.hook){
					card->wandev.event_callback.hook(card, &event);
				}		
				//zt_hooksig(&wr->chans[mod_no], ZT_RXSIG_ONHOOK);
				fe->rm_param.mod[mod_no].u.fxs.oldrxhook = 0;
			}
		}
	}
	fe->rm_param.mod[mod_no].u.fxs.lastrxhook = hook;
}
#endif

#if defined(AFT_TDM_API_SUPPORT)
/******************************************************************************
*			wp_remora_watchdog()	
*
* Description:
* Arguments: mod_no -  Module number (1,2,3,... MAX_REMORA_MODULES)
* Returns:
******************************************************************************/
static int wp_remora_watchdog(sdla_fe_t *fe)
{
	int	mod_no;
	
	if (SYSTEM_TICKS - fe->rm_param.last_watchdog  < WP_RM_WATCHDOG_TIMEOUT) {
		return 0;
	}
	fe->rm_param.last_watchdog = SYSTEM_TICKS;

	for (mod_no = 0; mod_no < fe->rm_param.max_fe_channels; mod_no++) {
		if (!wan_test_bit(mod_no, &fe->rm_param.module_map)) {
			continue;
		}
		if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXS){

			if (fe->rm_param.mod[mod_no].u.fxs.lasttxhook == 0x4) {
				/* RINGing, prepare for OHT */
				fe->rm_param.mod[mod_no].u.fxs.ohttimer = OHT_TIMER << 3;
				if (fe->fe_cfg.cfg.remora.reversepolarity){
					/* OHT mode when idle */
					fe->rm_param.mod[mod_no].u.fxs.idletxhookstate = 0x6;
				}else{
					fe->rm_param.mod[mod_no].u.fxs.idletxhookstate = 0x2; 
				}
			} else {
				if (fe->rm_param.mod[mod_no].u.fxs.ohttimer) {
					fe->rm_param.mod[mod_no].u.fxs.ohttimer-= WP_RM_CHUNKSIZE;
					if (!fe->rm_param.mod[mod_no].u.fxs.ohttimer) {
						if (fe->fe_cfg.cfg.remora.reversepolarity){
							/* Switch to active */
							fe->rm_param.mod[mod_no].u.fxs.idletxhookstate = 0x5;
						}else{
							fe->rm_param.mod[mod_no].u.fxs.idletxhookstate = 0x1;
						}
						if ((fe->rm_param.mod[mod_no].u.fxs.lasttxhook == 0x2) ||
						    (fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 0x6)) {
							/* Apply the change if appropriate */
							if (fe->fe_cfg.cfg.remora.reversepolarity){ 
								fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 0x5;
							}else{
								fe->rm_param.mod[mod_no].u.fxs.lasttxhook = 0x1;
							}
							WRITE_RM_REG(mod_no, 64, fe->rm_param.mod[mod_no].u.fxs.lasttxhook);
						}
					}
				}
			}

		} else if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXO) {

#if 0
			if (wr->mod[x].fxo.echotune){
				DEBUG_EVENT("%s: Module %d: Setting echo registers: \n",
							fe->name, x);

				/* Set the ACIM register */
				WRITE_RM_REG(x, 30, wr->mod[x].fxo.echoregs.acim);

				/* Set the digital echo canceller registers */
				WRITE_RM_REG(x, 45, wr->mod[x].fxo.echoregs.coef1);
				WRITE_RM_REG(x, 46, wr->mod[x].fxo.echoregs.coef2);
				WRITE_RM_REG(x, 47, wr->mod[x].fxo.echoregs.coef3);
				WRITE_RM_REG(x, 48, wr->mod[x].fxo.echoregs.coef4);
				WRITE_RM_REG(x, 49, wr->mod[x].fxo.echoregs.coef5);
				WRITE_RM_REG(x, 50, wr->mod[x].fxo.echoregs.coef6);
				WRITE_RM_REG(x, 51, wr->mod[x].fxo.echoregs.coef7);
				WRITE_RM_REG(x, 52, wr->mod[x].fxo.echoregs.coef8);

				DEBUG_EVENT("%s: Module %d: Set echo registers successfully\n",
						fe->name, x);
				wr->mod[x].fxo.echotune = 0;
			}
#endif			
			wp_remora_voicedaa_check_hook(fe, mod_no);
		}
	}
#if defined(AFT_RM_INTR_SUPPORT) && defined(AFT_RM_VIRTUAL_INTR_SUPPORT)
	if (wp_remora_check_intr(fe)){
		wp_remora_intr(fe);
	}
#endif

	return 0;
}
#endif

/******************************************************************************
*				wp_remora_intr_ctrl()	
*
* Description: Enable/Disable extra interrupt types
* Arguments: mod_no -  Module number (1,2,3,... MAX_REMORA_MODULES)
* Returns:
******************************************************************************/
static int
wp_remora_intr_ctrl(sdla_fe_t *fe, int mod_no, int type, int mode, unsigned int ts_map)
{
	int		err = 0;

	if (mod_no >= MAX_REMORA_MODULES){
		DEBUG_EVENT(
		"%s: Module %d: Module number is out of range!\n",
					fe->name, mod_no);
		return -EINVAL;
	}
	if (!wan_test_bit(mod_no, &fe->rm_param.module_map)) {
		DEBUG_EVENT("%s: Module %d: Unconfigured module!\n",
					fe->name, mod_no);
		return -EINVAL;
	}
	if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXS){

		switch(type){
		case WAN_RM_INTR_GLOBAL:
			if (mode == WAN_FE_INTR_ENABLE){
				WRITE_RM_REG(mod_no, 21, fe->rm_param.mod[mod_no].u.fxs.imask1);
				WRITE_RM_REG(mod_no, 22, fe->rm_param.mod[mod_no].u.fxs.imask2);
				WRITE_RM_REG(mod_no, 23, fe->rm_param.mod[mod_no].u.fxs.imask3);
			}else{
				WRITE_RM_REG(mod_no, 21, 0x00);
				WRITE_RM_REG(mod_no, 22, 0x00);
				WRITE_RM_REG(mod_no, 23, 0x00);
			}
			break;
		default:
			DEBUG_EVENT(
			"%s: Module %d: Unsupported FXS interrupt type (%X)!\n",
					fe->name, mod_no, type);
			err = -EINVAL;
			break;
		}

	}else if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXO){
		switch(type){
		case WAN_RM_INTR_GLOBAL:
			if (mode == WAN_FE_INTR_ENABLE){
				WRITE_RM_REG(mod_no, 3, fe->rm_param.mod[mod_no].u.fxo.imask);
			}else{
				WRITE_RM_REG(mod_no, 3, 0x00);
			}
			break;
		default:
			DEBUG_EVENT(
			"%s: Module %d: Unsupported FXO interrupt type (%X)!\n",
					fe->name, mod_no, type);
			err = -EINVAL;
			break;
		}
	}
	return err;

}

static int wp_remora_check_intr_fxs(sdla_fe_t *fe, int mod_no)
{
	unsigned char	stat1 = 0x0, stat2 = 0x00, stat3 = 0x00;

#if 1	
	if (wp_init_proslic_recheck_sanity(fe, mod_no)){
		return 0;
	}
#else
	if (__READ_RM_REG(mod_no, 8)){
		DEBUG_EVENT(
		"%s: Module %d: Oops, part reset, quickly restoring reality\n",
				fe->name, mod_no);
#if 0
		wp_init_proslic(fe, mod_no, 1, 1);
#else
		wp_remora_enable_timer(fe, mod_no, WP_RM_POLL_INIT, WP_RM_POLL_TIMER);
#endif
		return 0;
	}
#endif
	stat1 = __READ_RM_REG(mod_no, 18);
	stat2 = __READ_RM_REG(mod_no, 19);
	stat3 = __READ_RM_REG(mod_no, 20);

	if (stat1 || stat2 || stat3) return 1;
	return 0;	
}

static int wp_remora_check_intr_fxo(sdla_fe_t *fe, int mod_no)
{
	u_int8_t	status;
	
	status = READ_RM_REG(mod_no, 4);
	
	return (status) ? 1 : 0;
}

static int wp_remora_check_intr(sdla_fe_t *fe)
{
	int	mod_no = 0, pending = 0;

	fe->rm_param.intcount++;
	//mod_no = fe->rm_param.intcount % MAX_REMORA_MODULES;
	
	for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no++){
		if (wan_test_bit(mod_no, &fe->rm_param.module_map)) {
			if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXS){
				pending = wp_remora_check_intr_fxs(fe, mod_no);
			}else if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXO){
				pending = wp_remora_check_intr_fxo(fe, mod_no);
			}
			if (pending){
				return pending;
			}
		}
	}
#if 0
	if (card->wan_tdmv.sc && fe->rm_param.intcount % 100 == 0){
		wp_remora_enable_timer(fe, mod_no, WP_RM_POLL_TDMV, WP_RM_POLL_TIMER);
	}
#endif
	return 0;
} 

static int wp_remora_read_dtmf(sdla_fe_t *fe, int mod_no)
{
	sdla_t		*card;
	unsigned char	status;

	WAN_ASSERT(fe->card == NULL);
	card		= fe->card;
	status = READ_RM_REG(mod_no, 24);
	if (status & 0x10){
		unsigned char	digit = 0xFF;
		status &= 0xF;
		switch(status){
		case 0x01: digit = '1'; break;
		case 0x02: digit = '2'; break;
		case 0x03: digit = '3'; break;
		case 0x04: digit = '4'; break;
		case 0x05: digit = '5'; break;
		case 0x06: digit = '6'; break;
		case 0x07: digit = '7'; break;
		case 0x08: digit = '8'; break;
		case 0x09: digit = '9'; break;
		case 0x0A: digit = '0'; break;
		case 0x0B: digit = '*'; break;
		case 0x0C: digit = '#'; break;
		case 0x0D: digit = 'A'; break;
		case 0x0E: digit = 'B'; break;
		case 0x0F: digit = 'C'; break;
		case 0x00: digit = 'D'; break;
		}
		DEBUG_RM("%s: Module %d: TDMV digit %c!\n",
				fe->name,
				mod_no + 1,
				digit);
		WRITE_RM_REG(mod_no, 20, 0x1);
		if (card->wandev.event_callback.dtmf){
			wan_event_t	event;
			event.type	= WAN_EVENT_RM_DTMF;
			event.digit	= digit;
			event.channel	= mod_no+1;
			event.dtmf_port = WAN_EC_CHANNEL_PORT_SOUT;
			event.dtmf_type = WAN_EC_TONE_STOP;
			card->wandev.event_callback.dtmf(card, &event);
		}
	}
	return 0;
}

static int wp_remora_intr_fxs(sdla_fe_t *fe, int mod_no)
{
	sdla_t		*card = NULL;
	wan_event_t	event;
	unsigned char	stat1 = 0x0, stat2 = 0x00, stat3 = 0x00;

	WAN_ASSERT(fe->card == NULL);
	card		= fe->card;

	DEBUG_TDMAPI("mod_no: %d\n", mod_no);

	stat1 = READ_RM_REG(mod_no, 18);
	if (stat1){
		/* Ack interrupts for now */
		WRITE_RM_REG(mod_no, 18, stat1);
	}

	stat2 = READ_RM_REG(mod_no, 19);
	if (stat2){
		unsigned char	status = READ_RM_REG(mod_no, 68);

		if (stat2 & 0x02){
			DEBUG_RM(
			"%s: Module %d: LCIP interrupt pending!\n",
				fe->name, mod_no);
#if 0
			if (card->wandev.fe_notify_iface.hook_state){
				card->wandev.fe_notify_iface.hook_state(
						fe, mod_no, status & 0x01);
			}
#endif
			event.type	= WAN_EVENT_RM_LC;
			event.channel	= mod_no+1;
			if (status & 0x01){
				DEBUG_RM(
				"%s: Module %d: Off-hook status!\n",
						fe->name, mod_no);
				fe->rm_param.mod[mod_no].u.fxs.oldrxhook = 1;
				event.rxhook	= WAN_EVENT_RXHOOK_OFF;
			}else/* if (fe->rm_param.mod[mod_no].u.fxs.oldrxhook)*/{
				DEBUG_RM(
				"%s: Module %d: On-hook status!\n",
						fe->name, mod_no);
				fe->rm_param.mod[mod_no].u.fxs.oldrxhook = 0;
				event.rxhook	= WAN_EVENT_RXHOOK_ON;
			}
			if (card->wandev.event_callback.hook){
				card->wandev.event_callback.hook(
							card,
						       	&event);
			}
		}
		if (stat2 & 0x01){
			DEBUG_RM(
			"%s: Module %d: Ring TRIP interrupt pending!\n",
				fe->name, mod_no);
#if 0
			if (card->wandev.fe_notify_iface.hook_state){
				card->wandev.fe_notify_iface.hook_state(
						fe, mod_no, status & 0x01);
			}
#endif
			/*
			A ring trip event signals that the terminal equipment has gone
			off-hook during the ringing state.
			At this point the application should stop the ring because the
			call was answered.
			*/
			event.type	= WAN_EVENT_RM_RING_TRIP;
			event.channel	= mod_no+1;
			if (status & 0x02){
				DEBUG_RM(
				"%s: Module %d: Ring Trip detect occured!\n",
						fe->name, mod_no);
				event.rxhook	= WAN_EVENT_RING_TRIP_PRESENT;
			}else{
				DEBUG_RM(
				"%s: Module %d: Ring Trip detect not occured!\n",
						fe->name, mod_no);
				event.rxhook	= WAN_EVENT_RING_TRIP_STOP;
			}
			if (card->wandev.event_callback.ringtrip){
				card->wandev.event_callback.ringtrip(card, &event);
			}
		}
		DEBUG_RM(
		"%s: Module %d: Reg[64]=%02X Reg[68]=%02X\n",
			fe->name, mod_no,
			READ_RM_REG(mod_no,64),
			status);
		WRITE_RM_REG(mod_no, 19, stat2);
	}else{
#if defined(__WINDOWS__)
		TDM_FUNC_DBG
#endif
	}

	stat3 = READ_RM_REG(mod_no, 20);
	if (stat3){
		if (stat3 & 0x1){
			wp_remora_read_dtmf(fe, mod_no);
		}
		WRITE_RM_REG(mod_no, 20, stat3);
	}
	return 0;
}

static int wp_remora_intr_fxo(sdla_fe_t *fe, int mod_no)
{
	sdla_t		*card = NULL;
	wan_event_t	event;
	u_int8_t	status;

	WAN_ASSERT(fe->card == NULL);
	card		= fe->card;
	
	status = READ_RM_REG(mod_no, 4);
	if (status & 0x80){
		u_int8_t	mode;
		mode = READ_RM_REG(mod_no, 2);
		event.type	= WAN_EVENT_RM_RING_DETECT;
		event.channel	= mod_no+1;
		if (mode & 0x04){
			if (fe->rm_param.mod[mod_no].u.fxo.ring_detect){
				DEBUG_RM(
				"%s: Module %d: End of ring burst\n",
					fe->name, mod_no+1);
				fe->rm_param.mod[mod_no].u.fxo.ring_detect = 0;			
				event.ring_mode	= WAN_EVENT_RING_STOP;
			}else{
				DEBUG_RM(
				"%s: Module %d: Beginning of ring burst\n",
					fe->name, mod_no+1);
				fe->rm_param.mod[mod_no].u.fxo.ring_detect = 1;
				event.ring_mode	= WAN_EVENT_RING_PRESENT;
			}
		}else{
			DEBUG_RM(
			"%s: Module %d: Beginning of ring burst\n",
				fe->name, mod_no+1);
			fe->rm_param.mod[mod_no].u.fxo.ring_detect = 1;
			event.ring_mode	= WAN_EVENT_RING_PRESENT;
		}
		if (card->wandev.event_callback.ringdetect){
			card->wandev.event_callback.ringdetect(card, &event);
		}	
	}else if (status){
		DEBUG_RM(
		"%s: Module %d: Receive interrupt %02X!\n",
			fe->name, mod_no+1, status);
	}
	WRITE_RM_REG(mod_no, 4, 0x00);

	return 0;
}

static int wp_remora_intr(sdla_fe_t *fe)
{
	int	mod_no = 0;

	for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no++){
		if (!wan_test_bit(mod_no, &fe->rm_param.module_map)) {
			continue;
		}
		if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXS){
			wp_remora_intr_fxs(fe, mod_no);
		}else if (fe->rm_param.mod[mod_no].type == MOD_TYPE_FXO){
			wp_remora_intr_fxo(fe, mod_no);
		}
	}

	return 0;
}
