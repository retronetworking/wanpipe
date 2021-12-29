/*
 *************************************************************************************
 *                                                                                   *
 * FRNT_END.H - the 'C' header file for the Sangoma S508/S514 adapter front-end API. *
 *                                                                                   *
 *************************************************************************************
*/
#ifndef _SDLA_FRONT_END_H_
#define _SDLA_FRONT_END_H_

/*
*************************************************************************
*			  DEFINES AND MACROS				*	
*************************************************************************
*/
/* Front-End media type */
#define WAN_MEDIA_NONE		0x00
#define WAN_MEDIA_T1		0x01
#define WAN_MEDIA_E1		0x02
#define WAN_MEDIA_56K		0x03
#define WAN_MEDIA_DS3		0x04
#define WAN_MEDIA_E3		0x05
#define WAN_MEDIA_STS1		0x06
#define WAN_MEDIA_J1		0x07

/*The line code */
#define WAN_LCODE_AMI           0x01	/* T1/E1/DS3/E3 */
#define WAN_LCODE_B8ZS          0x02	/* T1 */
#define WAN_LCODE_HDB3          0x03	/* E1/E3 */
#define WAN_LCODE_B3ZS          0x04	/* DS3 */

/* Framing modes */
#define WAN_FR_ESF		0x01
#define WAN_FR_D4		0x02
#define WAN_FR_ESF_JAPAN	0x03
#define WAN_FR_CRC4		0x04
#define WAN_FR_NCRC4		0x05
#define WAN_FR_UNFRAMED		0x06
#define WAN_FR_E3_G751		0x07
#define WAN_FR_E3_G832		0x08
#define WAN_FR_DS3_Cbit		0x09
#define WAN_FR_DS3_M13		0x10

/* Clocking Master/Normal */
#define WAN_NORMAL_CLK		0x01
#define WAN_MASTER_CLK		0x02

/* Front-End UDP command */
#define WAN_FE_GET_STAT		(WAN_FE_UDP_CMD_START + 0)
#define WAN_FE_SET_LB_MODE	(WAN_FE_UDP_CMD_START + 1)
#define WAN_FE_FLUSH_PMON	(WAN_FE_UDP_CMD_START + 2)
#define WAN_FE_GET_CFG		(WAN_FE_UDP_CMD_START + 3)

#define IS_T1_MEDIA(media)	((media) == WAN_MEDIA_T1)
#define IS_E1_MEDIA(media)	((media) == WAN_MEDIA_E1)
#define IS_J1_MEDIA(media)	((media) == WAN_MEDIA_J1)
#define IS_TE1_MEDIA(media)	((media) == WAN_MEDIA_T1 ||	\
	       			 (media) == WAN_MEDIA_E1 ||	\
			       	 (media) == WAN_MEDIA_J1)
#define IS_56K_MEDIA(media)	((media) == WAN_MEDIA_56K)
#define IS_TE1_56K_MEDIA(media)	((media) == WAN_MEDIA_T1 || 	\
				 (media) == WAN_MEDIA_E1 ||	\
				 (media) == WAN_MEDIA_J1 ||	\
				 (media) == WAN_MEDIA_56K)
#define IS_DS3_MEDIA(media)	((media) == WAN_MEDIA_DS3)
#define IS_E3_MEDIA(media)	((media) == WAN_MEDIA_E3)

#define IS_TE1_56K(te_cfg)	((te_cfg).media == WAN_MEDIA_T1 || \
				 (te_cfg).media == WAN_MEDIA_E1 || \
				 (te_cfg).media == WAN_MEDIA_J1 || \
				 (te_cfg).media == WAN_MEDIA_56K)?1:0

#define MEDIA_DECODE(media)	((media) == WAN_MEDIA_T1) ? "T1" :	\
				((media) == WAN_MEDIA_E1) ? "E1" : 	\
				((media) == WAN_MEDIA_J1) ? "J1" : 	\
				((media) == WAN_MEDIA_56K) ? "56K" : 	\
				((media) == WAN_MEDIA_DS3) ? "DS3" : 	\
				((media) == WAN_MEDIA_E3) ? "E3" : "Unknown"

#define LCODE_DECODE(lcode)	((lcode) == WAN_LCODE_AMI)  ? "AMI" :	\
				((lcode) == WAN_LCODE_B8ZS) ? "B8ZS" :	\
				((lcode) == WAN_LCODE_HDB3) ? "HDB3" : 	\
				((lcode) == WAN_LCODE_B3ZS) ? "B3ZS" : "Unknown"

#define FRAME_DECODE(frame)	((frame) == WAN_FR_ESF)		? "ESF"  : \
				((frame) == WAN_FR_D4) 		? "D4"   : \
				((frame) == WAN_FR_CRC4)	? "CRC4" : \
				((frame) == WAN_FR_NCRC4)	? "non-CRC4" :	\
				((frame) == WAN_FR_UNFRAMED)	? "Unframed" : 	\
				((frame) == WAN_FR_E3_G751)	? "G.751" : 	\
				((frame) == WAN_FR_E3_G832)	? "G.832" : 	\
				((frame) == WAN_FR_DS3_Cbit)	? "C-bit" : 	\
				((frame) == WAN_FR_DS3_M13)	? "M13" : "Unknown"


/* front-end configuration and access interface commands */
#define READ_FRONT_END_REGISTER		(WAN_FE_CMD_START+0)	/* 0x90 read from front-end register */
#define WRITE_FRONT_END_REGISTER	(WAN_FE_CMD_START+1)	/* 0x91 write to front-end register */
#define READ_FRONT_END_STATISTICS	(WAN_FE_CMD_START+2)	/* 0x92 read the front-end statistics */
#define FLUSH_FRONT_END_STATISTICS	(WAN_FE_CMD_START+3)	/* 0x93 flush the front-end statistics */


typedef struct {
	unsigned char	media;
	unsigned char	sub_media;
	unsigned char	lcode;
	unsigned char	frame;
	unsigned int	line_no;
	union {
		sdla_te_cfg_t	te_cfg;
		sdla_te3_cfg_t	te3_cfg;
	} cfg;
} sdla_fe_cfg_t;

typedef struct {
	union {
		sdla_te_pmon_t	te_pmon;
		sdla_te3_pmon_t	te3_pmon;
	} u;
} sdla_fe_pmon_t;



#ifdef WAN_KERNEL

#define FE_ASSERT(val)	if (val) return;
#define FE_ASSERT1(val)	if (val) return 1;

#define WAN_FECALL(dev, func, x)					\
	((dev)->fe_iface.func) ? (dev)->fe_iface.func x : -EINVAL

/* Front-End status */
#if 0
enum fe_status {
	FE_UNITIALIZED = 0x00,
	FE_DISCONNECTED,
	FE_CONNECTED
};
#endif

/* adapter configuration interface commands */
#define SET_ADAPTER_CONFIGURATION	(WAN_INTERFACE_CMD_START+0)	/* 0xA0 set adapter configuration */
#define READ_ADAPTER_CONFIGURATION	(WAN_INTERFACE_CMD_START+1)	/* 0xA1 read adapter configuration */

/* front-end command */
#define WAN_FE_GET_STAT			(WAN_FE_UDP_CMD_START + 0)
#define WAN_FE_SET_LB_MODE		(WAN_FE_UDP_CMD_START + 1)
#define WAN_FE_FLUSH_PMON		(WAN_FE_UDP_CMD_START + 2)
#define WAN_FE_GET_CFG			(WAN_FE_UDP_CMD_START + 3)

/* return codes from interface commands */
#define LGTH_FE_CFG_DATA_INVALID       0x91 /* the length of the FE_RX_DISC_TX_IDLE_STRUCT is invalid */
#define LGTH_ADAPTER_CFG_DATA_INVALID  0x91 /* the length of the passed configuration data is invalid */
#define INVALID_FE_CFG_DATA            0x92 /* the passed SET_FE_RX_DISC_TX_IDLE_CFG data is invalid */
#define ADPTR_OPERATING_FREQ_INVALID   0x92 /* an invalid adapter operating frequency was selected */
#define PROT_CFG_BEFORE_FE_CFG         0x93 /* set the protocol-level configuration before setting the FE configuration */

#define SET_FE_RX_DISC_TX_IDLE_CFG      0x98 /* set the front-end Rx discard/Tx idle configuration */
#define READ_FE_RX_DISC_TX_IDLE_CFG     0x99 /* read the front-end Rx discard/Tx idle configuration */
#define SET_TE1_SIGNALING_CFG		0x9A /* set the T1/E1 signaling configuration */
#define READ_TE1_SIGNALING_CFG	0x9B /* read the T1/E1 signaling configuration */


#define COMMAND_INVALID_FOR_ADAPTER    0x9F /* the command is invalid for the adapter type */

 
/* ---------------------------------------------------------------------------------
 * Constants for the SET_FE_RX_DISC_TX_IDLE_CFG/READ_FE_RX_DISC_TX_IDLE_CFG commands
 * --------------------------------------------------------------------------------*/

#define NO_ACTIVE_RX_TIME_SLOTS_T1   24 /* T1 - no active time slots used for reception */
#define NO_ACTIVE_TX_TIME_SLOTS_T1   24 /* T1 - no active time slots used for transmission */
#define NO_ACTIVE_RX_TIME_SLOTS_E1   32 /* E1 - no active time slots used for reception */
#define NO_ACTIVE_TX_TIME_SLOTS_E1   31 /* E1 - no active time slots used for transmission (channel 0 reserved for framing) */

/* Read/Write to front-end register */
#if 0
#define READ_REG(reg)		card->wandev.read_front_end_reg(card, reg)
#define WRITE_REG(reg, value)	card->wandev.write_front_end_reg(card, reg, (unsigned char)(value))
#endif
#define WRITE_REG(reg,val)					\
		fe->write_fe_reg(fe->card, reg + (fe->fe_cfg.line_no*PMC4_LINE_DELTA), (unsigned char)val) 
#define WRITE_REG_LINE(fe_line_no, reg,val)					\
		fe->write_fe_reg(fe->card, reg + fe_line_no*PMC4_LINE_DELTA, (unsigned char)val) 
#define READ_REG(reg)						\
		fe->read_fe_reg(fe->card, reg + (fe->fe_cfg.line_no*PMC4_LINE_DELTA))
#define READ_REG_LINE(fe_line_no, reg)						\
		fe->read_fe_reg(fe->card, reg + fe_line_no*PMC4_LINE_DELTA)

/* the structure used for the SET_FE_RX_DISC_TX_IDLE_CFG/READ_FE_RX_DISC_TX_IDLE_CFG command */
#pragma pack(1)
typedef struct {
        unsigned short lgth_Rx_disc_bfr; /* the length of the Rx discard buffer */
        unsigned short lgth_Tx_idle_bfr; /* the length of the Tx idle buffer */
                                                /* the transmit idle data buffer */
        unsigned char Tx_idle_data_bfr[NO_ACTIVE_TX_TIME_SLOTS_E1];
} FE_RX_DISC_TX_IDLE_STRUCT;
#pragma pack()
                                         

/* ----------------------------------------------------------------------------
 *                       Constants for front-end access
 * --------------------------------------------------------------------------*/

/* the structure used for the READ_FRONT_END_REGISTER/WRITE_FRONT_END_REGISTER command */
#pragma pack(1)
typedef struct {
	unsigned short register_number; /* the register number to be read from or written to */
	unsigned char register_value;	/* the register value read/written */
} FRONT_END_REG_STRUCT;
#pragma pack()

#pragma pack(1)
typedef struct {
	unsigned char opp_flag;	/* opp flag */
	
	union {
		struct {
			unsigned char RR8_56k;	/* register #8 value - 56K CSU/DSU */
			unsigned char RR9_56k;	/* register #9 value - 56K CSU/DSU */
			unsigned char RRA_56k;	/* register #A value - 56K CSU/DSU */	
			unsigned char RRB_56k;	/* register #B value - 56K CSU/DSU */
			unsigned char RRC_56k;	/* register #C value - 56K CSU/DSU */
		} stat_56k;
	} FE_U;

} FRONT_END_STATUS_STRUCT;
#pragma pack()


/* -----------------------------------------------------------------------------
 *            Constants for the READ_FRONT_END_STATISTICS command
 * ---------------------------------------------------------------------------*/

/* the front-end statistics structure */
#pragma pack(1)
typedef struct {
	unsigned long FE_interrupt_count;   /* the number of front-end interrupts generated */
	unsigned long FE_app_timeout_count; /* the number of front-end interrupt application timeouts */
} FE_STATISTICS_STRUCT;
#pragma pack()



/* --------------------------------------------------------------------------------
 * Constants for the SET_ADAPTER_CONFIGURATION/READ_ADAPTER_CONFIGURATION commands
 * -------------------------------------------------------------------------------*/

/* the adapter configuration structure */
#pragma pack(1)
typedef struct {
	unsigned short adapter_type;	/* type of adapter */
	unsigned short adapter_config;	/* miscellaneous adapter configuration options */
	unsigned long operating_frequency;	/* adapter operating frequency */
} ADAPTER_CONFIGURATION_STRUCT;
#pragma pack()



typedef unsigned char (WRITE_FRONT_END_REG_T)(void*, unsigned short, unsigned char);
typedef unsigned char (READ_FRONT_END_REG_T)(void*, unsigned short);


enum {
   AFT_LED_ON,
   AFT_LED_OFF,
   AFT_LED_TOGGLE
};

typedef struct {
	char		*name;
	void		*card;
	sdla_fe_cfg_t	fe_cfg;
	/* FIXME: Remove the following parameters from wandev_t */
	unsigned char	fe_status;
	unsigned long	fe_alarm;
	/* ^^^ */
	union {
		sdla_te_param_t		te_param;
		sdla_56k_param_t	k56_param;
		sdla_te3_param_t	te3_param;
	} fe_param;
	sdla_fe_pmon_t	fe_pmon;
	int		(*write_cpld)(void*, unsigned short, unsigned char);
	int		(*read_cpld)(void*, unsigned short, unsigned char);
	int 		(*write_framer)(void*,unsigned short,unsigned short);
	unsigned int 	(*read_framer)(void*,unsigned short);
	WRITE_FRONT_END_REG_T	*write_fe_reg;
	READ_FRONT_END_REG_T	*read_fe_reg;
} sdla_fe_t;

/*
** Sangoma Front-End interface structure 
*/
#if 0
typedef struct {
	/* In-Service or Not (T1/E1/56K) */
	unsigned long	(*get_fe_service_status)(void*);
	/* Print Front-End alarm (T1/E1/56K) */
	void		(*print_fe_alarm)(void*,unsigned long);
	/* Print Front-End alarm (T1/E1/56K) */
	char*		(*print_fe_act_channels)(void*);
	/* Set Front-End alarm (T1/E1) */
	void		(*set_fe_alarm)(void*,unsigned long);
	/* Set extra interrupt type (after link get connected)) */
	int		(*set_fe_sigcfg)(void*, unsigned long);
} sdla_fe_iface_t;
#endif

/* 
** Sangoma Front-End interface structure (new version)
** FIXME: replace sdla_fe_iface_t with the new version! */
typedef struct {
	int		(*polling)(sdla_fe_t *fe);
	int		(*isr)(sdla_fe_t *fe);
	int		(*process_udp)(sdla_fe_t *fe, void*, unsigned char*);
	unsigned long	(*read_alarm)(sdla_fe_t *fe, int);
	int		(*read_pmon)(sdla_fe_t *fe);
	int		(*flush_pmon)(sdla_fe_t *fe);
	/* Set Front-End alarm (T1/E1) */
	int		(*set_fe_alarm)(sdla_fe_t *fe, unsigned long);
	/* Set extra interrupt type (after link get connected)) */
	int		(*set_fe_sigcfg)(sdla_fe_t*, unsigned long);
	/* Print Front-End alarm (T1/E1/56K) */
	char*		(*print_fe_act_channels)(sdla_fe_t*);
	/* Print Front-End alarm (T1/E1/56K) */
	int		(*print_fe_alarm)(sdla_fe_t*,unsigned long);
	/* Get front end status */
	int		(*get_fe_status)(sdla_fe_t *fe, unsigned char*);
	/* Get front end media type */
	unsigned char	(*get_fe_media)(sdla_fe_t *fe);
	/* Get front end media type string */
	char*		(*get_fe_media_string)(void);
	/* Set Line-loopback modes */
	int		(*set_fe_lbmode)(sdla_fe_t*, unsigned char, unsigned char);
	/* Update Alarm Status for proc file system */
	int		(*update_alarm_info)(sdla_fe_t*, struct seq_file* m, int* stop_cnt);
	/* Update PMON Status for proc file system */
	int		(*update_pmon_info)(sdla_fe_t*, struct seq_file* m, int* stop_cnt);
	/* AFT T1/E1 cards only */
	int		(*led_ctrl)(sdla_fe_t*, int mode);
	/* Read RBS bits (T1/E1 cards) */
	int		(*read_rbsbits)(sdla_fe_t*, int);
	/* Set RBS bits (T1/E1 cards, voice) */
	int		(*set_rbsbits)(sdla_fe_t*, int, unsigned char);
	/* Get Front-End SNMP data */
	int		(*get_snmp_data)(sdla_fe_t*, void*, void*);
	/* Check if the interrupt is for this port */
	int		(*check_isr)(sdla_fe_t *fe);
} sdla_fe_iface_t;

#endif	/* WAN_KERNEL */

#endif
