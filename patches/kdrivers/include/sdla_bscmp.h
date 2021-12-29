
#ifndef _BSC_HEADER_
#define _BSC_HEADER_


#ifndef	PACKED
#define	PACKED __attribute__((packed))
#endif	/* PACKED */


/*========== MAILBOX COMMANDS AND RETURN CODES ==========*/
#define BSC_READ                       0x00
#define BSC_WRITE                      0x01
#define OPEN_LINK                      0x02
#define CLOSE_LINK                     0x03
#define CAM_WRITE                      0x04
#define CAM_READ                       0x05
#define LINK_STATUS                    0x06
#define READ_OPERATIONAL_STATISTICS    0x07
#define FLUSH_OPERATIONAL_STATISTICS   0x08
#define READ_COMMS_ERROR_STATISTICS    0x09
#define FLUSH_COMMS_ERROR_STATISTICS   0x0A
#define READ_BSC_ERROR_STATISTICS      0x0B
#define FLUSH_BSC_ERROR_STATISTICS     0x0C
#define FLUSH_BSC_TEXT_BUFFERS         0x0D
#define SET_CONFIGURATION              0x0E
#define READ_CONFIGURATION             0x0F
#define SET_MODEM_STATUS               0x10
#define READ_MODEM_STATUS              0x11
#define READ_CODE_VERSION              0x12
#define ADD_STATION						0x20
#define DELETE_STATION					0x21
#define DELETE_ALL_STATIONS				0x22
#define LIST_STATIONS					0x23
#define SET_GENERAL_OR_SPECIFIC_POLL	0x24
#define SET_STATION_STATUS				0x25

#define READ_STATE_DIAGNOSTICS         0x30

#define	UNUSED_CMD_FOR_EVENTS		0x7e

#define	Z80_TIMEOUT_ERROR	0x0a
#define	DATA_LENGTH_TOO_BIG	0x03

#define BSC_SENDBOX     0xF000  /* send mailbox */

#define	MDATALEN	4000
#define MBOX_HEADER_SZ	15

/* for point-to-point, ignore station_number and address fields in CBLOCK */

/* note: structure must be packed on 1-byte boundaries
   and for a block this size, it is not wise to allocate it on
   the stack - should be a static global
*/
/* control block */
typedef struct {
	unsigned char 	opp_flag 		PACKED;
	unsigned char 	command			PACKED;
	unsigned short 	buffer_length		PACKED;
	unsigned char 	return_code		PACKED;
	unsigned char 	misc_tx_rx_bits		PACKED;
	unsigned short 	heading_length		PACKED;
	unsigned short 	notify			PACKED;
	unsigned char 	station			PACKED;
	unsigned char 	poll_address		PACKED;
	unsigned char 	select_address		PACKED;
	unsigned char 	device_address		PACKED;
	unsigned char 	notify_extended		PACKED;
	unsigned char 	reserved		PACKED;
	unsigned char 	data[MDATALEN]		PACKED;
} BSC_MAILBOX_STRUCT;



typedef struct {
	unsigned char 	line_speed_number	PACKED;
	unsigned short 	max_data_frame_size	PACKED;
	unsigned char 	secondary_station	PACKED;
	unsigned char 	num_consec_PAD_eof	PACKED;
	unsigned char 	num_add_lead_SYN	PACKED;
	unsigned char 	conversational_mode	PACKED;
	unsigned char 	pp_dial_up_operation	PACKED;
	unsigned char 	switched_CTS_RTS	PACKED;
	unsigned char 	EBCDIC_encoding		PACKED;
	unsigned char 	auto_open		PACKED;
	unsigned char 	misc_bits		PACKED;
	unsigned char 	protocol_options1	PACKED;
	unsigned char 	protocol_options2	PACKED;
	unsigned short 	reserved_pp		PACKED;
	unsigned char 	max_retransmissions	PACKED;
	unsigned short 	fast_poll_retries	PACKED;
	unsigned short 	TTD_retries		PACKED;
	unsigned short 	restart_timer		PACKED;
	unsigned short 	pp_slow_restart_timer	PACKED;
	unsigned short 	TTD_timer		PACKED;
	unsigned short 	pp_delay_between_EOT_ENQ PACKED;
	unsigned short 	response_timer		PACKED;
	unsigned short 	rx_data_timer		PACKED;
	unsigned short 	NAK_retrans_delay_timer PACKED;
	unsigned short 	wait_CTS_timer		PACKED;
	unsigned char 	mp_max_consec_ETX	PACKED;
	unsigned char 	mp_general_poll_address PACKED;
	unsigned short 	sec_poll_timeout	PACKED;
	unsigned char 	pri_poll_skips_inactive PACKED;
	unsigned char 	sec_additional_stn_send_gpoll PACKED;
	unsigned char 	pri_select_retries 	PACKED;
	unsigned char 	mp_multipoint_options	PACKED;
	unsigned short 	reserved		PACKED;
} BSC_CONFIG_STRUCT;


typedef struct {
	unsigned char max_tx_queue	PACKED;
	unsigned char max_rx_queue	PACKED;
	unsigned char station_flags	PACKED;
}ADD_STATION_STRUCT;

typedef struct {
	unsigned char	station		PACKED;
	unsigned short	time_stamp	PACKED;
	unsigned char	reserved[13]	PACKED;
} api_rx_hdr_t;

typedef struct {
        api_rx_hdr_t	api_rx_hdr      PACKED;
        void *   	data    	PACKED;
} api_rx_element_t;

typedef struct {
	unsigned char 	station		PACKED;
	unsigned char   misc_tx_rx_bits PACKED;
	unsigned char  	reserved[14]	PACKED;
} api_tx_hdr_t;

typedef struct {
	api_tx_hdr_t 	api_tx_hdr	PACKED;
	void *		data		PACKED;
} api_tx_element_t;

#define SIOC_WANPIPE_EXEC_CMD	SIOC_WANPIPE_DEVPRIVATE

#endif
