/*****************************************************************************
* sdladrv.c	SDLA Support Module.  Main module.
*
*		This module is a library of common hardware-specific functions
*		used by all Sangoma drivers.
*
* Authors:	Alex Feldman, Nenad Corbic, Gideon Hack, David Rokhvarg
*
* Copyright:	(c) 1995-2003 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jan 14, 2008	Alex Feldman	Add AUTO PCI probe for FreeBSD
*				Support FreeBSD-7
* Jul 06, 2007	David Rokhvarg	Added detection of A500 - ISDN BRI modules.
* Apr 04, 2007	Alex Feldman	Add support T3/E3 SHART cards
* Mar 04, 2007	Alex Feldman	Add support ISDN/BRI cards
* Dec 15. 2003  Nenad Corbic	Redesigned hw abstraction layer to 
*               Alex Feldman	support both BSD and Linux as well
*                               as to abstract the HW layer from 
*                               layers above.
*                               Added support for ADSL,S51X,AFT cards.
* Oct 02. 2002  Nenad Corbic	sdla_exec() update
* 				Timeout using jiffies and nloops since
* 				jiffies don't work when irq's are turned
* 				off.
* Apr 25, 2001  Nenad Corbic	Fixed the 2.0.X kernel bug in init.
* Mar 20, 2001  Nenad Corbic	Added the auto_pci_cfg filed, to support
*                               the PCISLOT #0. 
* Apr 04, 2000  Nenad Corbic	Fixed the auto memory detection code.
*                               The memory test at address 0xC8000.
* Mar 09, 2000  Nenad Corbic 	Added Gideon's Bug Fix: clear pci
*                               interrupt flags on initial load.
* Jun 02, 1999  Gideon Hack     Added support for the S514 adapter.
*				Updates for Linux 2.2.X kernels.	
* Sep 17, 1998	Jaspreet Singh	Updates for linux 2.2.X kernels
* Dec 20, 1996	Gene Kozin	Version 3.0.0. Complete overhaul.
* Jul 12, 1996	Gene Kozin	Changes for Linux 2.0 compatibility.
* Jun 12, 1996	Gene Kozin 	Added support for S503 card.
* Apr 30, 1996	Gene Kozin	SDLA hardware interrupt is acknowledged before
*				calling protocolspecific ISR.
*				Register I/O ports with Linux kernel.
*				Miscellaneous bug fixes.
* Dec 20, 1995	Gene Kozin	Fixed a bug in interrupt routine.
* Oct 14, 1995	Gene Kozin	Initial version.
*****************************************************************************/

/*****************************************************************************
 * Notes:
 * ------
 * 1. This code is ment to be system-independent (as much as possible).  To
 *    achive this, various macros are used to hide system-specific interfaces.
 *    To compile this code, one of the following constants must be defined:
 *
 *	Platform	Define
 *	--------	------
 *	Linux		_LINUX_
 *	SCO Unix	_SCO_UNIX_
 *
 * 2. Supported adapter types:
 *
 *	S502A
 *	ES502A (S502E)
 *	S503
 *	S507
 *	S508 (S509)
 *
 * 3. S502A Notes:
 *
 *	There is no separate DPM window enable/disable control in S502A.  It
 *	opens immediately after a window number it written to the HMCR
 *	register.  To close the window, HMCR has to be written a value
 *	????1111b (e.g. 0x0F or 0xFF).
 *
 *	S502A DPM window cannot be located at offset E000 (e.g. 0xAE000).
 *
 *	There should be a delay of ??? before reading back S502A status
 *	register.
 *
 * 4. S502E Notes:
 *
 *	S502E has a h/w bug: although default IRQ line state is HIGH, enabling
 *	interrupts by setting bit 1 of the control register (BASE) to '1'
 *	causes it to go LOW! Therefore, disabling interrupts by setting that
 *	bit to '0' causes low-to-high transition on IRQ line (ghosty
 *	interrupt). The same occurs when disabling CPU by resetting bit 0 of
 *	CPU control register (BASE+3) - see the next note.
 *
 *	S502E CPU and DPM control is limited:
 *
 *	o CPU cannot be stopped independently. Resetting bit 0 of the CPUi
 *	  control register (BASE+3) shuts the board down entirely, including
 *	  DPM;
 *
 *	o DPM access cannot be controlled dynamically. Ones CPU is started,
 *	  bit 1 of the control register (BASE) is used to enable/disable IRQ,
 *	  so that access to shared memory cannot be disabled while CPU is
 *	  running.
 ****************************************************************************/


#define __SDLA_HW_LEVEL
#define __SDLADRV__

/*
****************************************************************************
****		For Debug purpose (only OpenBSD)			****
****************************************************************************
*/

#ifdef __OpenBSD__
# undef WANDEBUG	/* Uncomment this line for debug purpose */
#endif

/***************************************************************************
****		I N C L U D E  		F I L E S			****
***************************************************************************/
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# include <wanpipe_includes.h>
# include <wanpipe_version.h>
# include <wanpipe_defines.h>
# include <wanpipe_debug.h>
# include <wanpipe_common.h>
# include <wanpipe.h>
# include <sdlasfm.h>
# include <sdlapci.h>
# if defined(SDLA_AUTO_PROBE)
#  include <sdla_bsd.h>
# endif
# include <sdladrv.h>
#elif defined(__WINDOWS__)
# include <wanpipe_includes.h>
# include <wanpipe_version.h>
# include <sdlasfm.h>	/* SDLA firmware module definitions */
# include <sdlapci.h>	/* SDLA PCI hardware definitions */
# include <wanpipe.h>
# include <sdladrv.h>	/* API definitions */
#elif defined(__LINUX__)||defined(__KERNEL__)
# define _K22X_MODULE_FIX_
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_version.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanpipe_common.h>
# include <linux/sdlasfm.h>	/* SDLA firmware module definitions */
# include <linux/sdlapci.h>	/* SDLA PCI hardware definitions */
# include <linux/wanpipe.h>
# include <linux/sdladrv.h>	/* API definitions */
#else
# error "Unsupported Operating System!"
#endif

#if !defined(__WINDOWS__)
#if 1
#define AFT_FUNC_DEBUG()
#else
#define AFT_FUNC_DEBUG()  DEBUG_EVENT("%s:%d\n",__FUNCTION__,__LINE__)
#endif
#endif

/***************************************************************************
****			M A C R O S / D E F I N E S			****
***************************************************************************/
#define SDLA_HWPROBE_NAME	"sdladrv"
#define SDLA_HWEC_NAME		"wanec"

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# define EXPORT_SYMBOL(symbol)
#endif
#define	SDLA_IODELAY	100	/* I/O Rd/Wr delay, 10 works for 486DX2-66 */
#define	EXEC_DELAY	20	/* shared memory access delay, mks */
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# define EXEC_TIMEOUT	(HZ*2)
#endif
#define MAX_NLOOPS	(EXEC_DELAY*2000)
 				/* timeout used if jiffies are stopped
				 * EXEC_DELAY=20
				 * EXEC_TIMEOUT=EXEC_DELAY*2000 = 40000 
				 * 40000 ~= 80 jiffies = EXEC_TIMEOUT */
 
#define	EXEC_HZ_DIVISOR	8/10    /* We don't want to wait a full second on
				* sdla_exec timeout, thus use 
				* HZ * EXEC_HZ_DIVISOR to get the number of  
				* jiffies we would like to wait */
 

/* I/O port address range */
#define S502A_IORANGE	3
#define S502E_IORANGE	4
#define S503_IORANGE	3
#define S507_IORANGE	4
#define S508_IORANGE	4

/* Maximum amount of memory */
#define S502_MAXMEM	0x10000L
#define S503_MAXMEM	0x10000L
#define S507_MAXMEM	0x40000L
#define S508_MAXMEM	0x40000L

/* Minimum amount of memory */
#define S502_MINMEM	0x8000L
#define S503_MINMEM	0x8000L
#define S507_MINMEM	0x20000L
#define S508_MINMEM	0x20000L

#define IS_SUPPORTED_ADAPTER(hw) (hw->type==SDLA_S508 || hw->type==SDLA_S514 || hw->type==SDLA_ADSL)

#define IS_S514(hw)	(hw->type == SDLA_S514)
#define IS_S518(hw)	(hw->type == SDLA_ADSL)
#define IS_S508(hw)	(hw->type == SDLA_S508)

#define SDLA_TYPE(hw)	IS_S508(hw) ? "S508" : 	\
			IS_S514(hw) ? "S514" :	\
			IS_S518(hw) ? "S518 (ADSL)" : "Unknown"

#define SDLA_ISA_CARD		0
#define SDLA_PCI_CARD		1
#define SDLA_PCI_EXP_CARD	2

/****** Function Prototypes *************************************************/

/* Hardware-specific functions */
static int sdla_register_check (wandev_conf_t* conf, char* devname);
static int sdla_setup (void* phw, wandev_conf_t* conf);
static int sdla_load (void* phw, void* psfm, unsigned len);
static int sdla_down (void* phw);
static int sdla_halt (void* phw);
static int sdla_inten (sdlahw_t* hw);
static int sdla_intack (void* phw, u32 int_status);
static int sdla_read_int_stat (void* phw, u32* int_status);
#if 0
static int sdla_intde (sdlahw_t* hw);
static int sdla_intr (sdlahw_t* hw);
#endif
static int sdla_mapmem (void* phw, unsigned long addr);
static int sdla_check_mismatch(void* phw, unsigned char media);

static int sdla_getcfg(void* phw, int type, void*);
static int sdla_setcfg(void* phw, int type, void* value);

#if defined(WAN_ISA_SUPPORT)
static int sdla_isa_read_1(void* phw, unsigned int offset, u8*);
static int sdla_isa_write_1(void* phw, unsigned int offset, u8);
#endif
static int sdla_io_write_1(void* phw, unsigned int offset, u8);
static int sdla_io_read_1(void* phw, unsigned int offset, u8*);
int sdla_bus_write_1(void* phw, unsigned int offset, u8);
int sdla_bus_write_2(void* phw, unsigned int offset, u16);
int sdla_bus_write_4(void* phw, unsigned int offset, u32);
int sdla_bus_read_1(void* phw, unsigned int offset, u8*);
int sdla_bus_read_2(void* phw, unsigned int offset, u16*);
int sdla_bus_read_4(void* phw, unsigned int offset, u32*);
static int sdla_pci_write_config_byte(void*, int, u8);
static int sdla_pci_write_config_word(void*, int, u16);
static int sdla_pci_write_config_dword(void*, int, u32);
static int sdla_pci_read_config_byte(void*, int, u8*);
static int sdla_pci_read_config_word(void*, int, u16*);
static int sdla_pci_read_config_dword(void*, int, u32*);

int sdla_pci_bridge_write_config_dword(void*, int, u_int32_t);
static int sdla_pci_bridge_write_config_byte(void*, int, u_int8_t);
int sdla_pci_bridge_read_config_dword(void*, int, u_int32_t*);
static int sdla_pci_bridge_read_config_byte(void*, int, u_int8_t*);

static int sdla_cmd (void* phw, unsigned long offset, wan_mbox_t* mbox);

static int sdla_exec (sdlahw_t* hw, unsigned long offset);
static int sdla_peek (void* phw, unsigned long addr, void* pbuf, unsigned len);
static int sdla_poke (void* phw, unsigned long addr, void* pbuf, unsigned len);
static int sdla_poke_byte (void* phw, unsigned long addr, u8);
static int sdla_set_bit (void* phw, unsigned long addr, u8);
static int sdla_clear_bit (void* phw, unsigned long addr, u8);
static void sdla_peek_by_4 (sdlahw_t* hw, unsigned long offset, void* pbuf, unsigned int len);
static void sdla_poke_by_4 (sdlahw_t* hw, unsigned long offset, void* pbuf, unsigned int len);
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static int sdla_set_intrhand(void* phw, wan_pci_ifunc_t*, void* arg, int);
static int sdla_restore_intrhand(void* phw, int);
#endif
static int sdla_get_hwcard(void* phw, void** phwcard);
static int sdla_get_hwprobe(void* phw, int port, void** str);

static int sdla_memory_map(sdlahw_t* hw, int cpu_no);
static int sdla_memory_unmap(sdlahw_t* hw);

static int sdla_detect	(sdlahw_t* hw);
static int sdla_start(sdlahw_t* hw, unsigned addr);
/*ALEXstatic int sdla_load	(sdlahw_t* hw, sfm_t* sfm, unsigned len);*/
static int sdla_bootcfg	(sdlahw_t* hw, sfm_info_t* sfminfo);
static unsigned char sdla_make_config_byte (sdlahw_t* hw);

#if defined(WAN_ISA_SUPPORT)	
static int sdla_init_s502a	(sdlahw_t* hw);
static int sdla_init_s502e	(sdlahw_t* hw);
static int sdla_init_s503	(sdlahw_t* hw);
static int sdla_init_s507	(sdlahw_t* hw);
static int sdla_init_s508	(sdlahw_t* hw);      
static int sdla_detect_s502a	(sdlahw_t* hw);
static int sdla_detect_s502e	(sdlahw_t* hw);
static int sdla_detect_s503	(sdlahw_t* hw);
static int sdla_detect_s507	(sdlahw_t* hw);
static int sdla_detect_s508	(sdlahw_t* hw);

static int sdla_autodpm	(sdlahw_t* hw);
static int sdla_setdpm	(sdlahw_t* hw);
static int sdla_init	(sdlahw_t* hw);
static unsigned long sdla_memtest (sdlahw_t* hw);
static unsigned sdla_check_memregion (sdlahw_t* hw);
static int sdla_get_option_index (unsigned* optlist, unsigned optval);
#endif	

static int sdla_detect_s514	(sdlahw_t* hw);
static int sdla_detect_pulsar(sdlahw_t* hw);
static int sdla_detect_aft(sdlahw_t* hw);

static int sdla_is_te1(void* phw);
static int sdla_is_56k(void* phw);

static sdlahw_t* sdla_find_adapter(wandev_conf_t* conf, char* devname);

/* Miscellaneous functions */
static unsigned	sdla_test_memregion (sdlahw_t* hw, unsigned len);
static unsigned short sdla_checksum (unsigned char* buf, unsigned len);
static int sdla_init_pci_slot(sdlahw_t *);


static sdlahw_card_t* sdla_card_register(unsigned char hw_type, int slot_no, int bus_no, int ioport);
static int sdla_card_unregister (unsigned char hw_type, int slot_no, int bus_no, int ioport);
static sdlahw_card_t* sdla_card_search(unsigned char hw_type, int slot_no, int bus_no, int ioport);
static int sdla_card_info(sdlahw_card_t*);

static sdlahw_cpu_t* sdla_hwcpu_register(sdlahw_card_t* card, int cpu_no, int irq, void*);
static int sdla_hwcpu_unregister(sdlahw_cpu_t*);
static sdlahw_cpu_t* sdla_hwcpu_search(unsigned char hw_type, int slot_no, int bus_no, int ioport, int cpu_no);
static int sdla_hwcpu_info(sdlahw_cpu_t*);

static sdlahw_t* sdla_hw_register(sdlahw_cpu_t* hwcpu, int port_no);
static int sdla_hw_unregister(sdlahw_t*);
static sdlahw_t* sdla_hw_search(sdlahw_cpu_t* hwcpu, int port_no);
static int sdla_hw_info(sdlahw_t*);

static int sdla_hwport_unregister(sdlahw_cpu_t*);
static sdlahw_t* sdla_hwport_register(sdlahw_cpu_t* hwcpu, int);
static sdlahw_t* sdla_hwport_te1_register(sdlahw_cpu_t*, int);
static sdlahw_t* sdla_hwport_Remora_register(sdlahw_cpu_t*, int*);
static sdlahw_t* sdla_hwport_ISDN_register(sdlahw_cpu_t*, int*);

static int sdla_s514_hw_select (sdlahw_card_t*, int, int, void*);
static int sdla_adsl_hw_select (sdlahw_card_t*, int, int, void*);
static int sdla_aft_hw_select (sdlahw_card_t*, int, int, void*);
static void sdla_save_hw_probe (sdlahw_t* hw);

static int sdla_hw_lock(void *phw, wan_smp_flag_t *flag);
static int sdla_hw_unlock(void *phw, wan_smp_flag_t *flag);

static int sdla_hw_ec_trylock(void *phw, wan_smp_flag_t *flag);
static int sdla_hw_ec_lock(void *phw, wan_smp_flag_t *flag);
static int sdla_hw_ec_unlock(void *phw, wan_smp_flag_t *flag);

static wan_dma_descr_t *sdla_busdma_descr_alloc(void *phw, int num);
static void sdla_busdma_descr_free(void *phw, wan_dma_descr_t *dma_descr);
#if defined(__FreeBSD__)
static void sdla_busdma_callback(void *arg, bus_dma_segment_t *seg, int nseg, int error);
#endif
static int sdla_busdma_tag_create(void *phw, wan_dma_descr_t*, u32, u32,int);
static int sdla_busdma_tag_destroy(void *phw, wan_dma_descr_t*, int);
static int sdla_busdma_create(void *phw, wan_dma_descr_t*);
static int sdla_busdma_destroy(void *phw, wan_dma_descr_t*);
static int sdla_busdma_alloc(void *phw, wan_dma_descr_t*);
static void sdla_busdma_free(void *phw, wan_dma_descr_t*);
static int sdla_busdma_load(void *phw, wan_dma_descr_t*, u32);
static void sdla_busdma_unload(void *phw, wan_dma_descr_t*);
static void sdla_busdma_map(void *phw, wan_dma_descr_t*, void *buf, int len, int map_len, int dir);
static void sdla_busdma_unmap(void *phw, wan_dma_descr_t*, int dir);
static void sdla_busdma_sync(void *phw, wan_dma_descr_t*, int ndescr, int single, int dir);

static sdla_dma_addr_t sdla_pci_map_dma(void *phw, void *buf, int len, int ctrl);
static int sdla_pci_unmap_dma(void *phw, sdla_dma_addr_t buf, int len, int ctrl);

static int sdla_is_same_hwcard(void* phw1, void *phw2);
static int sdla_is_same_hwcpu(void* phw1, void *phw2);
int sdla_hw_fe_test_and_set_bit(void *phw,int value);
int sdla_hw_fe_test_bit(void *phw,int value);
int sdla_hw_fe_set_bit(void *phw,int value);
int sdla_hw_fe_clear_bit(void *phw,int value);

static int sdla_hw_read_cpld(void *phw, u16 off, u8 *data);
static int sdla_hw_write_cpld(void *phw, u16 off, u8 data);

extern int	sdla_te1_write_fe(void* phw, ...);
extern u_int8_t	sdla_te1_read_fe (void* phw, ...);

extern int	sdla_shark_te1_write_fe(void *phw, ...);
extern u_int8_t	__sdla_shark_te1_read_fe (void *phw, ...);
extern u_int8_t	sdla_shark_te1_read_fe (void *phw, ...);

extern int	sdla_shark_rm_write_fe (void* phw, ...);
extern u_int8_t	__sdla_shark_rm_read_fe (void* phw, ...);
extern u_int8_t	sdla_shark_rm_read_fe (void* phw, ...);

extern int	sdla_shark_bri_write_fe (void* phw, ...);
extern u_int8_t	sdla_shark_bri_read_fe (void* phw, ...);
static int	sdla_scan_isdn_bri_modules(sdlahw_t* hw, int *rm_mod_type,  u_int8_t rm_no);

extern int	sdla_shark_56k_write_fe(void *phw, ...);
extern u_int8_t	__sdla_shark_56k_read_fe (void *phw, ...);
extern u_int8_t	sdla_shark_56k_read_fe (void *phw, ...);

extern int	sdla_shark_serial_write_fe (void* phw, ...);
extern u_int8_t	sdla_shark_serial_read_fe (void* phw, ...);

extern int	sdla_te3_write_fe(void *phw, ...);
extern u_int8_t	sdla_te3_read_fe(void *phw, ...);

extern int	sdla_plxctrl_read8(void *phw, short, unsigned char*);
extern int	sdla_plxctrl_write8(void *phw, short, unsigned char);

#if defined(__LINUX__)

static int sdla_pci_probe(sdlahw_t*);
#endif

static int	sdla_is_pciexpress(void *phw);
static int	sdla_get_hwec_index(void *phw);

/****** Global Data **********************************************************
 * Note: All data must be explicitly initialized!!!
 */
#if defined(__FreeBSD__) && (__FreeBSD_version < 500000)
volatile extern int ticks;	/* This line will causes redundant 
				   redeclaration warning. Don't worry, 
				   otherwise loop in calibrate_delay() will
				   never finished (optimization) */
#endif

/* SDLA ISA/PCI varibles */
extern int 		Sangoma_cards_no;	/* total number of SDLA cards */
extern int 		Sangoma_devices_no;	/* Max number of Sangoma dev */
extern int		Sangoma_PCI_cards_no;  	/* total number of S514 cards */

#if !defined(SDLA_AUTO_PROBE)
# if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
extern sdladev_t 	sdladev[];		/* SDLA info structure 	      */
# endif
#endif

/* private data */
#if defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
char		*wan_drvname	= "wanpipe(lite)";
static char	*wan_fullname	= "WANPIPE(Lite) Hardware Support Module";
#else
static char	*wan_drvname	= "wanpipe";
static char	*wan_fullname	= "WANPIPE(tm) Hardware Support Module";
#endif

/* Array of already initialized PCI slots */
static int pci_slot_ar[MAX_S514_CARDS];

WAN_LIST_HEAD(NAME_PLACEHOLDER, sdlahw_card) sdlahw_card_head = 
			WAN_LIST_HEAD_INITIALIZER(&sdlahw_card_head);
WAN_LIST_HEAD(NAME_PLACEHOLDER1, sdlahw_cpu) sdlahw_cpu_head = 
			WAN_LIST_HEAD_INITIALIZER(&sdlahw_cpu_head);
WAN_LIST_HEAD(NAME_PLACEHOLDER2, sdlahw_port) sdlahw_head = 
			WAN_LIST_HEAD_INITIALIZER(&sdlahw_head);
WAN_LIST_HEAD(NAME_PLACEHOLDER3, sdla_hw_probe) sdlahw_probe_head = 
			WAN_LIST_HEAD_INITIALIZER(&sdlahw_probe_head);


static sdla_hw_type_cnt_t	sdla_adapter_cnt;
static int			sdla_hwec_no = 0;

#if defined(__LINUX__) || defined(__WINDOWS__)
static unsigned long EXEC_TIMEOUT;
#endif

#if defined(__WINDOWS__)
extern int get_usage_counter(sdla_t *card);
extern wan_spinlock_t* get_card_spin_lock(sdla_t *card);
extern u32 get_card_serial_number(sdla_t *card);
#endif

/* Hardware configuration options.
 * These are arrays of configuration options used by verification routines.
 * The first element of each array is its size (i.e. number of options).
 */
#if defined(WAN_ISA_SUPPORT)
static unsigned	s502_port_options[] =
	{ 4, 0x250, 0x300, 0x350, 0x360 }
;
static unsigned	s503_port_options[] =
	{ 8, 0x250, 0x254, 0x300, 0x304, 0x350, 0x354, 0x360, 0x364 }
;
static unsigned	s508_port_options[] =
	{ 8, 0x250, 0x270, 0x280, 0x300, 0x350, 0x360, 0x380, 0x390 }
;

static unsigned s502a_irq_options[] = { 0 };
static unsigned s502e_irq_options[] = { 4, 2, 3, 5, 7 };
static unsigned s503_irq_options[]  = { 5, 2, 3, 4, 5, 7 };
static unsigned s508_irq_options[]  = { 8, 3, 4, 5, 7, 10, 11, 12, 15 };

static unsigned s502a_dpmbase_options[] =
{
	28,
	0xA0000, 0xA2000, 0xA4000, 0xA6000, 0xA8000, 0xAA000, 0xAC000,
	0xC0000, 0xC2000, 0xC4000, 0xC6000, 0xC8000, 0xCA000, 0xCC000,
	0xD0000, 0xD2000, 0xD4000, 0xD6000, 0xD8000, 0xDA000, 0xDC000,
	0xE0000, 0xE2000, 0xE4000, 0xE6000, 0xE8000, 0xEA000, 0xEC000,
};
static unsigned s507_dpmbase_options[] =
{
	32,
	0xA0000, 0xA2000, 0xA4000, 0xA6000, 0xA8000, 0xAA000, 0xAC000, 0xAE000,
	0xB0000, 0xB2000, 0xB4000, 0xB6000, 0xB8000, 0xBA000, 0xBC000, 0xBE000,
	0xC0000, 0xC2000, 0xC4000, 0xC6000, 0xC8000, 0xCA000, 0xCC000, 0xCE000,
	0xE0000, 0xE2000, 0xE4000, 0xE6000, 0xE8000, 0xEA000, 0xEC000, 0xEE000,
};
static unsigned s508_dpmbase_options[] =	
{
	32,
	0xA0000, 0xA2000, 0xA4000, 0xA6000, 0xA8000, 0xAA000, 0xAC000, 0xAE000,
	0xC0000, 0xC2000, 0xC4000, 0xC6000, 0xC8000, 0xCA000, 0xCC000, 0xCE000,
	0xD0000, 0xD2000, 0xD4000, 0xD6000, 0xD8000, 0xDA000, 0xDC000, 0xDE000,
	0xE0000, 0xE2000, 0xE4000, 0xE6000, 0xE8000, 0xEA000, 0xEC000, 0xEE000,
};

/*
static unsigned	s508_dpmsize_options[] = { 1, 0x2000 };
*/

static unsigned	s502a_pclk_options[] = { 2, 3600, 7200 };
static unsigned	s502e_pclk_options[] = { 5, 3600, 5000, 7200, 8000, 10000 };
static unsigned	s503_pclk_options[]  = { 3, 7200, 8000, 10000 };
static unsigned	s507_pclk_options[]  = { 1, 12288 };
static unsigned	s508_pclk_options[]  = { 1, 16000 };

/* Host memory control register masks */
static unsigned char s502a_hmcr[] =
{
	0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C,	/* A0000 - AC000 */
	0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C,	/* C0000 - CC000 */
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C,	/* D0000 - DC000 */
	0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C,	/* E0000 - EC000 */
};
static unsigned char s502e_hmcr[] =
{
	0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,	/* A0000 - AE000 */
	0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,	/* C0000 - CE000 */
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,	/* D0000 - DE000 */
	0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,	/* E0000 - EE000 */
};
static unsigned char s507_hmcr[] =
{
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,	/* A0000 - AE000 */
	0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E,	/* B0000 - BE000 */
	0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E,	/* C0000 - CE000 */
	0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,	/* E0000 - EE000 */
};
static unsigned char s508_hmcr[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	/* A0000 - AE000 */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,	/* C0000 - CE000 */
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,	/* D0000 - DE000 */
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,	/* E0000 - EE000 */
};

static unsigned char s507_irqmask[] =
{
	0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
};
#endif /* WAN_ISA_SUPPORT */

/* Entry Point for Low-Level function */
int sdladrv_init(void*);
int sdladrv_exit(void*);
#if 0
int sdladrv_shutdown(void*);
int sdladrv_ready_unload(void*);
#endif

/*****************************************************************************/
/* Loadable kernel module function interface */
#if !defined(CONFIG_PRODUCT_WANPIPE_GENERIC) && !defined(__WINDOWS__)
# if !defined(__OpenBSD__) && !defined(__NetBSD__)
WAN_MODULE_DEFINE(
		sdladrv, "sdladrv",
		"Alex Feldman <al.feldman@sangoma.com>",
		"Sangoma WANPIPE: HW Layer",
		"GPL",
		sdladrv_init, sdladrv_exit,
		NULL);
WAN_MODULE_VERSION(sdladrv, SDLADRV_MAJOR_VER);
# endif 
#endif


/*****************************************************************************/
/* Memory Debug Code
*/

# if defined(WAN_DEBUG_MEM)

static int wan_debug_mem;

wan_spinlock_t wan_debug_mem_lock;
EXPORT_SYMBOL(wan_debug_mem_lock);

WAN_LIST_HEAD(NAME_PLACEHOLDER_MEM, sdla_memdbg_el) sdla_memdbg_head = 
			WAN_LIST_HEAD_INITIALIZER(&sdla_memdbg_head);

typedef struct sdla_memdbg_el
{
	unsigned int len;
	unsigned int line;
	char cmd_func[128];
	void *mem;
	WAN_LIST_ENTRY(sdla_memdbg_el)	next;
}sdla_memdbg_el_t;

int sdla_memdbg_init(void)
{
	wan_spin_lock_init(&wan_debug_mem_lock,"wan_debug_mem_lock");
	WAN_LIST_INIT(&sdla_memdbg_head);
	return 0;
}


int sdla_memdbg_push(void *mem, char *func_name, int line, int len)
{
	sdla_memdbg_el_t *sdla_mem_el;
	wan_smp_flag_t flags;

	sdla_mem_el = kmalloc(sizeof(sdla_memdbg_el_t),GFP_ATOMIC);
	if (!sdla_mem_el) {
		DEBUG_EVENT("%s:%d Critical failed to allocate memory!\n",
			__FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	memset(sdla_mem_el,0,sizeof(sdla_memdbg_el_t));
		
	sdla_mem_el->len=len;
	sdla_mem_el->line=line;
	sdla_mem_el->mem=mem;
	strncpy(sdla_mem_el->cmd_func,func_name,sizeof(sdla_mem_el->cmd_func)-1);
	

	wan_spin_lock_irq(&wan_debug_mem_lock,&flags);
	wan_debug_mem+=sdla_mem_el->len;
	WAN_LIST_INSERT_HEAD(&sdla_memdbg_head, sdla_mem_el, next);
	wan_spin_unlock_irq(&wan_debug_mem_lock,&flags);

	DEBUG_EVENT("%s:%d: Alloc %p Len=%i Total=%i\n",
			sdla_mem_el->cmd_func,sdla_mem_el->line,
			 sdla_mem_el->mem, sdla_mem_el->len,wan_debug_mem);
	
	return 0;

}
EXPORT_SYMBOL(sdla_memdbg_push);

int sdla_memdbg_pull(void *mem, char *func_name, int line)
{
	sdla_memdbg_el_t *sdla_mem_el;
	wan_smp_flag_t flags;
	int err=-1;

	wan_spin_lock_irq(&wan_debug_mem_lock,&flags);

	WAN_LIST_FOREACH(sdla_mem_el, &sdla_memdbg_head, next){
		if (sdla_mem_el->mem == mem) {
			break;
		}
	}

	if (sdla_mem_el) {
		
		WAN_LIST_REMOVE(sdla_mem_el, next);
		wan_debug_mem-=sdla_mem_el->len;
		wan_spin_unlock_irq(&wan_debug_mem_lock,&flags);

		DEBUG_EVENT("%s:%d: DeAlloc %p Len=%i Total=%i (From %s:%d)\n",
			func_name,line,
			sdla_mem_el->mem, sdla_mem_el->len, wan_debug_mem,
			sdla_mem_el->cmd_func,sdla_mem_el->line);

		kfree(sdla_mem_el);
		err=0;
	} else {
		wan_spin_unlock_irq(&wan_debug_mem_lock,&flags);
	}

	if (err) {
		DEBUG_EVENT("%s:%d: Critical Error: Unknows Memeory %p\n",
			__FUNCTION__,__LINE__,mem);
	}

	return err;
}
EXPORT_SYMBOL(sdla_memdbg_pull);

int sdla_memdbg_free(void)
{
	sdla_memdbg_el_t *sdla_mem_el;
	int total=0;

	DEBUG_EVENT("sdladrv: Memory Still Allocated=%i \n",
			 wan_debug_mem);

	DEBUG_EVENT("=====================BEGIN================================\n");

	sdla_mem_el = WAN_LIST_FIRST(&sdla_memdbg_head);
	while(sdla_mem_el){
		sdla_memdbg_el_t *tmp = sdla_mem_el;

		DEBUG_EVENT("%s:%d: Mem Leak %p Len=%i \n",
			sdla_mem_el->cmd_func,sdla_mem_el->line,
			sdla_mem_el->mem, sdla_mem_el->len);
		total+=sdla_mem_el->len;

		sdla_mem_el = WAN_LIST_NEXT(sdla_mem_el, next);
		WAN_LIST_REMOVE(tmp, next);
		kfree(tmp);
	}

	DEBUG_EVENT("=====================END==================================\n");
	DEBUG_EVENT("sdladrv: Memory Still Allocated=%i  Leaks Found=%i Missing=%i\n",
			 wan_debug_mem,total,wan_debug_mem-total);

	return 0;
}

# endif



/*============================================================================
 * Module init point.
 */
int sdladrv_init(void* arg)
{
	int volatile i=0;

	if (WANPIPE_VERSION_BETA){
		DEBUG_EVENT("%s Beta %s.%s %s %s\n",
			wan_fullname, WANPIPE_VERSION, WANPIPE_SUB_VERSION, 
			WANPIPE_COPYRIGHT_DATES,WANPIPE_COMPANY);
	}else{
		DEBUG_EVENT("%s Stable %s.%s %s %s\n",
			wan_fullname, WANPIPE_VERSION, WANPIPE_SUB_VERSION, 
			WANPIPE_COPYRIGHT_DATES,WANPIPE_COMPANY);
	}

#if defined(__LINUX__)
	EXEC_TIMEOUT=HZ*EXEC_HZ_DIVISOR;
#endif
	/* Initialize the PCI Card array, which
         * will store flags, used to mark 
         * card initialization state */
	for (i=0; i<MAX_S514_CARDS; i++)
		pci_slot_ar[i] = 0xFF;

	memset(&sdla_adapter_cnt,0,sizeof(sdla_hw_type_cnt_t));

#ifdef WAN_DEBUG_MEM
	sdla_memdbg_init();
#endif

	return 0;
}

#if 0
int sdladrv_shutdown(void *arg)
{
	DEBUG_EVENT("Shutting down SDLADRV module ...\n");
	return 0;
}

int sdladrv_ready_unload(void *arg)
{
	DEBUG_EVENT("Is SDLADRV module ready to unload...\n");
	return 0;
}
#endif

/*============================================================================
 * Module deinit point.
 * o release all remaining system resources
 */
int sdladrv_exit (void *arg)
{
	sdla_hw_probe_t	*elm_hw_probe;
	sdlahw_cpu_t	*elm_hw_cpu;
	sdlahw_card_t	*elm_hw_card;
	
	DEBUG_MOD("Unloading SDLADRV module ...\n");

	elm_hw_cpu = WAN_LIST_FIRST(&sdlahw_cpu_head);
	while(elm_hw_cpu){
		sdlahw_cpu_t	*tmp = elm_hw_cpu;
		elm_hw_cpu = WAN_LIST_NEXT(elm_hw_cpu, next);
		if (sdla_hwport_unregister(tmp) == -EBUSY){
			return -EBUSY;
		}
		if (sdla_hwcpu_unregister(tmp) == -EBUSY){
			return -EBUSY;
		}
	}
	WAN_LIST_INIT(&sdlahw_cpu_head);

	elm_hw_card = WAN_LIST_FIRST(&sdlahw_card_head);
	while(elm_hw_card){
		sdlahw_card_t	*tmp = elm_hw_card;
		elm_hw_card = WAN_LIST_NEXT(elm_hw_card, next);
		if (sdla_card_unregister((u8)tmp->hw_type, 
					 tmp->slot_no, 
					 tmp->bus_no, 
					 tmp->ioport) == -EBUSY){
			return -EBUSY;
		}
	}
	WAN_LIST_INIT(&sdlahw_card_head);

	elm_hw_probe = WAN_LIST_FIRST(&sdlahw_probe_head);
	while(elm_hw_probe){
		sdla_hw_probe_t	*tmp = elm_hw_probe;
		elm_hw_probe = WAN_LIST_NEXT(elm_hw_probe, next);
		if (tmp->internal_used){
			DEBUG_EVENT("sdladrv: HW probe info is in used (%s)\n",
					tmp->hw_info);
			return -EBUSY;
		}
		WAN_LIST_REMOVE(tmp, next);
		wan_free(tmp);
	}

	
#if defined(WAN_DEBUG_MEM)
	sdla_memdbg_free();
#endif	
	
	return 0;
}

/*
*****************************************************************************
*****************************************************************************
G***		S A N G O M A  H A R D W A R E  P R O B E 		*****
*****************************************************************************
*****************************************************************************
*/
/*
*****************************************************************************
**			sdla_save_hw_probe
*****************************************************************************
*/
#define SDLA_HWPROBE_ISA_FORMAT				\
	"%-10s : IOPORT=0x%X : PORT=%s"
#define SDLA_HWPROBE_PCI_FORMAT				\
	"%-10s : SLOT=%d : BUS=%d : IRQ=%d : CPU=%c : PORT=%s"
#define SDLA_HWPROBE_AFT_FORMAT				\
	"%-10s : SLOT=%d : BUS=%d : IRQ=%d : CPU=%c : PORT=%d : V=%02X"
#define SDLA_HWPROBE_AFT_1_2_FORMAT				\
	"%-10s : SLOT=%d : BUS=%d : IRQ=%d : CPU=%c : PORT=%s : V=%02X"
#define SDLA_HWPROBE_AFT_SH_FORMAT			\
	"%-10s : SLOT=%d : BUS=%d : IRQ=%d : CPU=%c : PORT=%d : HWEC=%d : V=%02X" 
#define SDLA_HWPROBE_A200_SH_FORMAT			\
	"%-10s : SLOT=%d : BUS=%d : IRQ=%d : CPU=%c : PORT=%s : HWEC=%d : V=%02X"
#define SDLA_HWPROBE_A500_SH_FORMAT			\
	"%-10s : SLOT=%d : BUS=%d : IRQ=%d : PORT=%d : HWEC=%d : V=%02X"
static void 
sdla_save_hw_probe (sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	sdla_hw_probe_t	*hwprobe;

	WAN_ASSERT_VOID(hw == NULL);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	hwprobe = hw->hwprobe;

	if (hwcpu->hwcard->hw_type == SDLA_PCI_CARD){
		switch(hwcpu->hwcard->adptr_type){
		case A101_ADPTR_1TE1:
		case A101_ADPTR_2TE1:
			if (hwcpu->hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
				snprintf(hwprobe->hw_info, 
					sizeof(hwprobe->hw_info),
					SDLA_HWPROBE_AFT_SH_FORMAT, 
					hwcpu->hwcard->adptr_name,
					hwcpu->hwcard->slot_no, 
					hwcpu->hwcard->bus_no, 
					hwcpu->irq, 
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hw->port_no+1,			/* line_no */
					hwcpu->hwcard->hwec_chan_no,
					hwcpu->hwcard->core_rev);
			}else{
				/*sprintf(tmp_hw_probe->hw_info,*/
				snprintf(hwprobe->hw_info, 
					sizeof(hwprobe->hw_info),
					SDLA_HWPROBE_AFT_1_2_FORMAT,
					hwcpu->hwcard->adptr_name,
					hwcpu->hwcard->slot_no, 
					hwcpu->hwcard->bus_no, 
					hwcpu->irq, 
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hw->port_no ? "SEC" : "PRI",
					hwcpu->hwcard->core_rev);
			}
			break;
		
		case A104_ADPTR_4TE1:
		case A108_ADPTR_8TE1:
			if (hwcpu->hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
				snprintf(hwprobe->hw_info, 
					sizeof(hwprobe->hw_info),
					SDLA_HWPROBE_AFT_SH_FORMAT, 
					hwcpu->hwcard->adptr_name,
					hwcpu->hwcard->slot_no, 
					hwcpu->hwcard->bus_no, 
					hwcpu->irq, 
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hw->port_no+1,			/* line_no */
					hwcpu->hwcard->hwec_chan_no,
					hwcpu->hwcard->core_rev);
			}else{
				snprintf(hwprobe->hw_info, 
					sizeof(hwprobe->hw_info),
					SDLA_HWPROBE_AFT_FORMAT, 
					hwcpu->hwcard->adptr_name,
					hwcpu->hwcard->slot_no, 
					hwcpu->hwcard->bus_no, 
					hwcpu->irq, 
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hw->port_no+1,
					hwcpu->hwcard->core_rev
					);		/* line_no */
			}
			break;

		case A200_ADPTR_ANALOG:
		case A400_ADPTR_ANALOG:
			/*sprintf(tmp_hw_probe->hw_info,*/
			snprintf(hwprobe->hw_info, 
				sizeof(hwprobe->hw_info),
				SDLA_HWPROBE_A200_SH_FORMAT,
				hwcpu->hwcard->adptr_name,
				hwcpu->hwcard->slot_no, 
				hwcpu->hwcard->bus_no, 
				hwcpu->irq, 
				SDLA_GET_CPU(hwcpu->cpu_no), 
				hw->port_no ? "SEC" : "PRI",
				hwcpu->hwcard->hwec_chan_no,
				hwcpu->hwcard->core_rev);
			break;
		
		case A300_ADPTR_U_1TE3:
		case AFT_ADPTR_56K:
			if (hwcpu->hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
				snprintf(hwprobe->hw_info, 
					sizeof(hwprobe->hw_info),
					SDLA_HWPROBE_AFT_FORMAT, 
					hwcpu->hwcard->adptr_name,
					hwcpu->hwcard->slot_no, 
					hwcpu->hwcard->bus_no, 
					hwcpu->irq, 
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hw->port_no+1,
					hwcpu->hwcard->core_rev);		/* line_no */
			}else{
				snprintf(hwprobe->hw_info, 
					sizeof(hwprobe->hw_info),
					SDLA_HWPROBE_PCI_FORMAT,
					hwcpu->hwcard->adptr_name,
					hwcpu->hwcard->slot_no, 
					hwcpu->hwcard->bus_no, 
					hwcpu->irq, 
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hw->port_no ? "SEC" : "PRI");						
			}
			break;
			
		case AFT_ADPTR_ISDN:
			snprintf(hwprobe->hw_info, 
					sizeof(hwprobe->hw_info),
					SDLA_HWPROBE_A500_SH_FORMAT, 
					hwcpu->hwcard->adptr_name,
					hwcpu->hwcard->slot_no, 
					hwcpu->hwcard->bus_no, 
					hwcpu->irq, 
					hw->port_no+1,			/* Physical line number */
					hwcpu->hwcard->hwec_chan_no,
					hwcpu->hwcard->core_rev);
			break;

		case AFT_ADPTR_2SERIAL_V35X21:
		case AFT_ADPTR_4SERIAL_V35X21:
		case AFT_ADPTR_2SERIAL_RS232:
		case AFT_ADPTR_4SERIAL_RS232:
			snprintf(hwprobe->hw_info, 
					sizeof(hwprobe->hw_info),
					SDLA_HWPROBE_AFT_FORMAT, 
					hwcpu->hwcard->adptr_name,
					hwcpu->hwcard->slot_no, 
					hwcpu->hwcard->bus_no, 
					hwcpu->irq, 
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hw->port_no+1,
					hwcpu->hwcard->core_rev);		/* line_no */
			break;

		default:
			/*sprintf(tmp_hw_probe->hw_info,*/
			snprintf(hwprobe->hw_info, 
				sizeof(hwprobe->hw_info),
				SDLA_HWPROBE_PCI_FORMAT,
				hwcpu->hwcard->adptr_name,
				hwcpu->hwcard->slot_no, 
				hwcpu->hwcard->bus_no, 
				hwcpu->irq, 
				SDLA_GET_CPU(hwcpu->cpu_no), 
				hw->port_no ? "SEC" : "PRI");
			break;
		}
	}else{
		/*sprintf(tmp_hw_probe->hw_info, */
		snprintf(hwprobe->hw_info, sizeof(hwprobe->hw_info),
				SDLA_HWPROBE_ISA_FORMAT,
				"S508-ISA",
				hwcpu->hwcard->ioport, 
				hw->port_no ? "SEC" : "PRI");
	}
	return;
}

static void sdla_get_hwcard_name(sdlahw_card_t* hwcard)
{
	WAN_ASSERT_VOID(hwcard == NULL);
	sprintf(hwcard->adptr_name, "%s%s%s",
			SDLA_ADPTR_NAME(hwcard->adptr_type),
			AFT_SUBTYPE(hwcard->adptr_subtype),
			AFT_SECURITY(hwcard->adptr_security));
	return;
}


static sdlahw_t* sdla_hwport_register(sdlahw_cpu_t* hwcpu, int max_ports_no)
{
	sdlahw_t	*hw = NULL, *first_hw = NULL;
	int		port;

	for(port = 0; port < max_ports_no; port++){
		if ((hw = sdla_hw_register(hwcpu, port)) == NULL){
			sdla_hwport_unregister(hwcpu);
			return NULL;
		}
		if (first_hw == NULL){
			first_hw = hw;
		}
	}
	return first_hw;
}

static int sdla_hwport_unregister(sdlahw_cpu_t* hwcpu)
{
	sdlahw_t	*hw, *tmp_hw;

	WAN_ASSERT(hwcpu == NULL);
	hw = WAN_LIST_FIRST(&sdlahw_head);
	while(hw){

		tmp_hw = hw;
		hw = WAN_LIST_NEXT(hw, next);

		if (tmp_hw->hwcpu == hwcpu){
			if (sdla_hw_unregister(tmp_hw)){
				return -EBUSY;
			}
		}
	}
	return 0; 
}

static sdlahw_t* sdla_hwport_serial_register(sdlahw_cpu_t* hwcpu, int max_ports_no)
{
	sdlahw_t	*hw = NULL, *first_hw = NULL;
	int		port;
	unsigned char	id_str[50];

	memset(id_str, 0x00, 50);
	for(port = 0; port < max_ports_no; port++){
		switch(hwcpu->hwcard->adptr_type){
		case AFT_ADPTR_2SERIAL_V35X21: 
			strcpy(id_str, "AFT-A142 2 Port V.35/X.21");
			break;
		case AFT_ADPTR_4SERIAL_V35X21: 
			strcpy(id_str, "AFT-A144 4 Port V.35/X.21");
			break;
		case AFT_ADPTR_2SERIAL_RS232:
			strcpy(id_str, "AFT-A142 2 Port RS232");
			break;
		case AFT_ADPTR_4SERIAL_RS232:
			strcpy(id_str, "AFT-A144 4 Port RS232");
			break;
		}

		if ((hw = sdla_hw_register(hwcpu, port)) == NULL){
			sdla_hwport_unregister(hwcpu);
			return NULL;
		}
		if (first_hw == NULL){
			first_hw = hw;
		}
		sprintf(&hw->hwprobe->hw_info_verbose[0], "\n+%02d:%s:%s",
				port+1, id_str, 
				AFT_PCITYPE_DECODE(hwcpu->hwcard));
	}
	return first_hw;
}

static sdlahw_t* sdla_hwport_te1_register(sdlahw_cpu_t* hwcpu, int max_ports_no)
{
	sdlahw_t	*hw = NULL, *first_hw = NULL;
	int		port;
	unsigned char	id_str[50];

	memset(id_str, 0x00, 50);
	for(port = 0; port < max_ports_no; port++){
		if (hwcpu->hwcard->type == SDLA_S514){
			strcpy(id_str, "PMC4351");
		}else if (hwcpu->hwcard->type == SDLA_AFT){
			if (hwcpu->hwcard->cfg_type == WANOPT_AFT){
				strcpy(id_str, "PMC4351");
			}else if (hwcpu->hwcard->cfg_type == WANOPT_AFT101){
				strcpy(id_str, "DS26521");
			}else if (hwcpu->hwcard->cfg_type == WANOPT_AFT102){
				strcpy(id_str, "DS26521");
			}else if (hwcpu->hwcard->cfg_type == WANOPT_AFT104){
				if (hwcpu->hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
					strcpy(id_str, "DS26524"); 
				}else{
					strcpy(id_str, "PMC4354");
				}
			}else if (hwcpu->hwcard->cfg_type == WANOPT_AFT108){
				strcpy(id_str, "DS26528");
			}
		}else{
			continue;
		}

		if ((hw = sdla_hw_register(hwcpu, port)) == NULL){
			sdla_hwport_unregister(hwcpu);
			return NULL;
		}
		if (first_hw == NULL){
			first_hw = hw;
		}
		sprintf(&hw->hwprobe->hw_info_verbose[0], "\n+%02d:%s:%s",
				port+1, id_str, 
				AFT_PCITYPE_DECODE(hwcpu->hwcard));
	}
	return first_hw;
}

static sdlahw_t *sdla_hwport_Remora_register(sdlahw_cpu_t* hwcpu, int *max_ports_no)
{
	sdlahw_t	*hw;
	u32		reg;
	int		mod_no, off = 0, port_cnt = 0;
	int		rm_mod_type[MAX_REMORA_MODULES+2];
	u_int32_t	port_map = 0;
	unsigned char	value, str[50];
		
	WAN_ASSERT_RC(hwcpu == NULL, NULL);
	*max_ports_no = 0;
	if ((hw = sdla_hw_register(hwcpu, 0)) == NULL){
		return NULL;
	}

	if (sdla_memory_map(hw, SDLA_CPU_A)){
		sdla_hw_unregister(hw);
		return NULL;
	}

	/* A200 clear reset */
	sdla_bus_read_4(hw, 0x40,&reg);
	wan_set_bit(1,&reg);
	wan_set_bit(2,&reg);
	sdla_bus_write_4(hw,0x40,reg);

	WP_DELAY(10);

	wan_clear_bit(1,&reg);
	wan_clear_bit(2,&reg);
	sdla_bus_write_4(hw,0x40,reg);

	WP_DELAY(10);

	/* Reset SPI bus */
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,MOD_SPI_RESET);
	WP_DELAY(1000);
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,0x00000000);
	WP_DELAY(1000);

	for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no ++){
 		rm_mod_type[mod_no] = MOD_TYPE_NONE;		
       		//rm_mod_type[mod_no+1] = MOD_TYPE_NONE;
		value = sdla_shark_rm_read_fe(hw, mod_no, MOD_TYPE_FXS, 0, 0);
		if ((value & 0x0F) == 0x05){
			rm_mod_type[mod_no] = MOD_TYPE_FXS;		
		//	rm_mod_type[mod_no+1] = MOD_TYPE_FXS;
		}
	}
	
	/* Reset SPI bus */
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,MOD_SPI_RESET);
	WP_DELAY(1000);
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,0x00000000);
	WP_DELAY(1000);	
	for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no ++){
	
		if (rm_mod_type[mod_no] != MOD_TYPE_NONE) continue;
		value = sdla_shark_rm_read_fe(hw, mod_no, MOD_TYPE_FXO, 1, 2);
		if (value == 0x03){
			rm_mod_type[mod_no] = MOD_TYPE_FXO;		
			//rm_mod_type[mod_no+1] = MOD_TYPE_FXO;
		}
	}
	
	port_map = 0;
	for(mod_no = 0; mod_no < MAX_REMORA_MODULES; mod_no ++){
	
		if (rm_mod_type[mod_no] == MOD_TYPE_FXS){
			sprintf(str, "\n+%02d:FXS", mod_no+1);
			port_cnt++;
			port_map |= (1 << (mod_no+1));
		}else if (rm_mod_type[mod_no] == MOD_TYPE_FXO){
			sprintf(str, "\n+%02d:FXO", mod_no+1);
			port_cnt++;		
			port_map |= (1 << (mod_no+1));
		}else{
			sprintf(str, "\n+%02d:EMPTY", mod_no+1);		
		}
		memcpy(&hw->hwprobe->hw_info_verbose[off],
			str, strlen(str));
		off += strlen(str);
	}
	/* Reset SPI bus */
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,MOD_SPI_RESET);
	WP_DELAY(1000);
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,0x00000000);
	WP_DELAY(1000);
	
	WP_DELAY(10);

	wan_set_bit(1,&reg);
	wan_set_bit(2,&reg);
	sdla_bus_write_4(hw,0x40,reg);
	
	sdla_memory_unmap(hw);
	*max_ports_no = port_cnt;
	hwcpu->max_ports = port_cnt;	/* overwrite with real port number */
	hwcpu->port_map = port_map;

	return hw;
}

static sdlahw_t *sdla_hwport_ISDN_register(sdlahw_cpu_t* hwcpu, int *ports_no)
{
	sdlahw_t	*hw, *first_hw = NULL;
	int		port_cnt = 0, mod_no, rm_no;
	u32		reg, port_map=0;
	int		rm_mod_type[MAX_REMORA_MODULES+2];
	unsigned char	str[50];

	WAN_ASSERT_RC(hwcpu == NULL, NULL);

	if ((hw = sdla_hw_register(hwcpu, 0)) == NULL){
		return NULL;
	}

	if (sdla_memory_map(hw, SDLA_CPU_A)){
		sdla_hw_unregister(hw);
		return NULL;
	} 
	
	/* A500 clear reset. Disable Global Chip/FrontEnd/CPLD Reset. */
	sdla_bus_read_4(hw, 0x40,&reg);
	wan_set_bit(1,&reg);
	wan_set_bit(2,&reg);
	sdla_bus_write_4(hw,0x40,reg);

	WP_DELAY(10);

	wan_clear_bit(1,&reg);
	wan_clear_bit(2,&reg);
	sdla_bus_write_4(hw,0x40,reg);

	WP_DELAY(10);

	/* Reset SPI bus */
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,MOD_SPI_RESET);
	WP_DELAY(1000);
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,0x00000000);
	WP_DELAY(1000);

	for(mod_no = 0; mod_no < MAX_BRI_LINES; mod_no++){
 		rm_mod_type[mod_no] = MOD_TYPE_NONE;		
	}

	for(rm_no = 0; rm_no < MAX_BRI_REMORAS; rm_no++){
		port_cnt += sdla_scan_isdn_bri_modules(hw, rm_mod_type, (u8)rm_no);
	}

	/* The sdla_scan_isdn_bri_modules() call returns number of PHYSICAL modules.
	   But sdla_save_ISDN_hw_probe() must return number of LOGICAL modules,
	   which is actually number of BRI ports.
	*/
	port_cnt *= BRI_MAX_PORTS_PER_CHIP;

	/* A500 has a hwport for each BRI port */
	for(mod_no = 0; mod_no < MAX_BRI_LINES; mod_no ++){

		if (rm_mod_type[mod_no] == MOD_TYPE_TE){
			sprintf(str, "\n+%02d:TE", mod_no+1);
			port_map |= (1 << (mod_no+1));
		}else if (rm_mod_type[mod_no] == MOD_TYPE_NT){
			sprintf(str, "\n+%02d:NT", mod_no+1);
			port_map |= (1 << (mod_no+1));
		}else{
			/* EMPTY */		
			continue;
		}

		if (first_hw == NULL){
			sdla_hwport_unregister(hwcpu);
		}
		if ((hw = sdla_hw_register(hwcpu, mod_no)) == NULL){
			sdla_memory_unmap(hw);
			sdla_hwport_unregister(hwcpu);
			return NULL;
		}
		if (first_hw == NULL){
			first_hw = hw;
		}
		memcpy(&hw->hwprobe->hw_info_verbose[0],
			str, strlen(str));
	}

	/* Reset SPI bus */
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,MOD_SPI_RESET);
	WP_DELAY(1000);
	sdla_bus_write_4(hw,SPI_INTERFACE_REG,0x00000000);
	WP_DELAY(1000);
	
	WP_DELAY(10);

	wan_set_bit(1,&reg);
	wan_set_bit(2,&reg);
	sdla_bus_write_4(hw,0x40,reg);
	
	sdla_memory_unmap(hw);
	*ports_no = port_cnt;
	hwcpu->max_ports = port_cnt;	/* overwrite with real port number */
	hwcpu->port_map = port_map;
	return first_hw;
}

/******************************************************************************
 *				sdla_scan_isdn_bri_modules()	
 *
 * Description	: Scan for installed modules.
 *
 * Arguments	: pfe - pointer to Front End structure.	
 * Returns		: number of discovered modules.
 *******************************************************************************/

static int sdla_scan_isdn_bri_modules(sdlahw_t* hw, int *rm_mod_type,  u_int8_t rm_no)
{
	u_int8_t mod_no = 0x3;	/* to read remora status register ALWAYS put 0x3 into mod_addr. */
	u_int8_t value, ind, mod_counter = 0;
	u_int8_t mod_no_index;	/* index in the array of ALL modules (NOT lines) on ALL remoras. From 0 to 11 */

	/* format of remora status register: 
		bit 0 == 1	- module 1 active (exist)
		bit 1		- type of module 1 (0 - NT, 1 - TE)

		bit 2 == 1	- module 2 active (exist)
		bit 3		- type of module 1 (0 - NT, 1 - TE)

  		bit 4 == 1	- module 3 active (exist)
		bit 5		- type of module 1 (0 - NT, 1 - TE)

		bit 6,7		- has to be zeros for active remora. if non-zero, remora does not exist.
	*/

	value = sdla_shark_bri_read_fe(	hw, 
					mod_no,
					MOD_TYPE_NONE,	
					rm_no,
					0);

#define MODULE1	0
#define MODULE2	2
#define MODULE3	4

	DEBUG_TEST("remora number: %d, remora status register: 0x%02X\n", rm_no, value);

	if(((value >> 7) & 0x01) || ((value >> 8) & 0x01)){
		DEBUG_EVENT("%s: Remora number %d does not exist.\n", hw->devname, rm_no);
		return 0;
	}

	for(ind = 0; ind < 6; ind++){

		switch(ind){

		case MODULE1:
		case MODULE2:
		case MODULE3:
			DEBUG_TEST("module Number on REMORA (0-2): %d\n", ind / 2);

			/* 0-11, all (even and odd) numbers */
			mod_no_index = rm_no * MAX_BRI_MODULES_PER_REMORA + ind / 2;
			DEBUG_TEST("mod_no_index on CARD (should be 0-11): %d\n", mod_no_index);

			/* 0-23, only even numbers */
			mod_no_index = mod_no_index * 2;//????
			DEBUG_TEST("mod_no_index (line number) on CARD (should be 0-23): %d\n", mod_no_index);

			if(mod_no_index >= MAX_BRI_LINES){
					DEBUG_EVENT("%s: Error: Module %d/%d exceeds maximum (%d)\n",
						hw->devname, mod_no_index, mod_no_index, MAX_BRI_LINES);
				return 0;
			}

			if((value >> ind) & 0x01){

				mod_counter++;

				if((value >> (ind + 1)) & 0x01){
					
					DEBUG_TEST("%s: Module %d type is: TE\n",
						hw->devname, mod_no_index);

					rm_mod_type[mod_no_index] = MOD_TYPE_TE;
					rm_mod_type[mod_no_index+1] = MOD_TYPE_TE;

				}else{
					DEBUG_TEST("%s: Module %d type is: NT\n",
						hw->devname, mod_no_index);

					rm_mod_type[mod_no_index] = MOD_TYPE_NT;
					rm_mod_type[mod_no_index+1] = MOD_TYPE_NT;
				}

			}else{
				DEBUG_TEST("%s: Module %d is not installed\n",
						hw->devname, mod_no_index);
			}
			DEBUG_TEST("=================================\n\n");
			break;
		}/* switch() */
	}/* for() */

	return mod_counter;
}

#define AFT_CHIP_CFG_REG		0x40
#define AFT_CHIPCFG_SFR_IN_BIT		2
#define AFT_CHIPCFG_SFR_EX_BIT		1
#if !defined(__WINDOWS__)
static 
#endif
int sdla_get_hw_info(sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;
	unsigned int	reg, reg1;
	unsigned short	cpld_off;
	unsigned char	status = 0, tmp = 0, adptr_sec = 0;

	AFT_FUNC_DEBUG();

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if (sdla_memory_map(hw, SDLA_CPU_A)){
		return -EINVAL;
	}

	if (hwcard->adptr_subtype == AFT_SUBTYPE_NORMAL){
		switch(hwcard->adptr_type){
		case A101_ADPTR_1TE1:
		case A101_ADPTR_2TE1:
			cpld_off = AFT_SECURITY_CPLD_REG;
			sdla_hw_read_cpld(hw, cpld_off, &tmp);
			adptr_sec = AFT_GET_SECURITY(tmp);
			switch(adptr_sec){
			case AFT_SECURITY_1LINE_UNCH:
			case AFT_SECURITY_2LINE_UNCH:
				hwcard->adptr_security = AFT_SECURITY_UNCHAN;
				break;
			case AFT_SECURITY_1LINE_CH:
			case AFT_SECURITY_2LINE_CH:
				hwcard->adptr_security = AFT_SECURITY_CHAN;
				break;
			default:
				DEBUG_EVENT(
				"%s: AFT-A101-2 Critical error: Unknown Security ID (0x%02X)!\n",
						wan_drvname, adptr_sec);
				break;
			}
			break;
	
		case A104_ADPTR_4TE1:
			/* Enable memory access */	
			sdla_bus_read_4(hw, AFT_CHIP_CFG_REG, &reg1);
			reg = reg1;
			wan_clear_bit(AFT_CHIPCFG_SFR_IN_BIT, &reg);
			wan_clear_bit(AFT_CHIPCFG_SFR_EX_BIT, &reg);
			sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg);
	
			cpld_off = AFT_SECURITY_CPLD_REG;
			sdla_hw_read_cpld(hw, cpld_off, &tmp);
			adptr_sec = AFT_GET_SECURITY(tmp);
			if (adptr_sec == AFT_SECURITY_1LINE_UNCH){
				hwcard->adptr_security = AFT_SECURITY_UNCHAN;
			}else if (adptr_sec == AFT_SECURITY_1LINE_CH){
				hwcard->adptr_security = AFT_SECURITY_CHAN;
			}else if (adptr_sec == 0x02){
				/*FIXME: ALEX CHANGE HARDCODED VALUE FOR SHARK */
				hwcard->adptr_security = AFT_SECURITY_CHAN;
			}else{
				DEBUG_EVENT(
				"%s: AFT-A104 Critical error: Unknown Security ID (%02X)!\n",
						wan_drvname, adptr_sec);
			}
	
			/* Restore original value */	
			sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg1);
			break;
		}	
	
	}else if (hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
		switch(hwcard->core_id){
		case AFT_PMC_FE_CORE_ID:
			switch(hwcard->adptr_type){
			case A104_ADPTR_4TE1:
				/* Enable memory access */	
				sdla_bus_read_4(hw, AFT_CHIP_CFG_REG, &reg1);
				reg = reg1;
				wan_clear_bit(AFT_CHIPCFG_SFR_IN_BIT, &reg);
				wan_clear_bit(AFT_CHIPCFG_SFR_EX_BIT, &reg);
				sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg);
		
				cpld_off = AFT_SH_CPLD_BOARD_STATUS_REG;
				sdla_hw_read_cpld(hw, cpld_off, &status);
				hwcard->hwec_chan_no = A104_ECCHAN(AFT_SH_SECURITY(status));
		
				/* Restore original value */	
				sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg1);
				break;	
			case A300_ADPTR_U_1TE3:
				/* By default, AFT-A300 is unchannelized! */
				hwcard->adptr_security = AFT_SECURITY_UNCHAN;
				break;		
			}
			break;
		case AFT_DS_FE_CORE_ID:
			switch(hwcard->adptr_type){
			case A101_ADPTR_1TE1:
			case A101_ADPTR_2TE1:
			case A104_ADPTR_4TE1:
			case A108_ADPTR_8TE1:
				/* Enable memory access */	
				sdla_bus_read_4(hw, AFT_CHIP_CFG_REG, &reg1);
				reg = reg1;
				wan_clear_bit(AFT_CHIPCFG_SFR_IN_BIT, &reg);
				wan_clear_bit(AFT_CHIPCFG_SFR_EX_BIT, &reg);
				sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg);
		
				cpld_off = AFT_SH_CPLD_BOARD_STATUS_REG;
				sdla_hw_read_cpld(hw, cpld_off, &status);
				hwcard->hwec_chan_no = A108_ECCHAN(AFT_SH_SECURITY(status));
						
				/* Restore original value */	
				sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg1);
				break;			
			}	
			break;
		default:
			switch(hwcard->adptr_type){
			case A200_ADPTR_ANALOG:
			case A400_ADPTR_ANALOG:
				/* Enable memory access */	
				sdla_bus_read_4(hw, AFT_CHIP_CFG_REG, &reg1);
				reg = reg1;
				wan_clear_bit(AFT_CHIPCFG_SFR_IN_BIT, &reg);
				wan_clear_bit(AFT_CHIPCFG_SFR_EX_BIT, &reg);
				sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg);
		
				cpld_off = A200_SH_CPLD_BOARD_STATUS_REG;
				sdla_hw_read_cpld(hw, cpld_off, &status);
				hwcard->hwec_chan_no = AFT_RM_ECCHAN(AFT_SH_SECURITY(status));
			
				if (hwcard->hwec_chan_no){
		
					/* Check EC access */
					/* Clear octasic reset */
					cpld_off = 0x00;
					sdla_hw_write_cpld(hw, cpld_off, 0x01);
	
					/* Set octasic reset */
					cpld_off = 0x00;
					sdla_hw_write_cpld(hw, cpld_off, 0x00);
				}	
		
				/* Restore original value */	
				sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg1);
				break;

			case AFT_ADPTR_ISDN:
				AFT_FUNC_DEBUG();
				/* Enable memory access */	
				sdla_bus_read_4(hw, AFT_CHIP_CFG_REG, &reg1);
				reg = reg1;
				wan_clear_bit(AFT_CHIPCFG_SFR_IN_BIT, &reg);
				wan_clear_bit(AFT_CHIPCFG_SFR_EX_BIT, &reg);
				sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg);
		
				/* Restore original value */	
				//sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg1);
				cpld_off = A200_SH_CPLD_BOARD_STATUS_REG;
				sdla_hw_read_cpld(hw, cpld_off, &status);

				DEBUG_BRI("at A200_SH_CPLD_BOARD_STATUS_REG: 0x%X\n", status);

				hwcard->hwec_chan_no = A500_ECCHAN(AFT_SH_SECURITY(status));

				DEBUG_BRI("hwcard->hwec_chan_no: %d\n", hwcard->hwec_chan_no);

				if (hwcard->hwec_chan_no){
					/* Check EC access */
					/* Clear octasic reset */
					cpld_off = 0x00;
					sdla_hw_write_cpld(hw, cpld_off, 0x01);
	
					/* Set octasic reset */
					cpld_off = 0x00;
					sdla_hw_write_cpld(hw, cpld_off, 0x00);
				}	
				break;

			case AFT_ADPTR_56K:
				AFT_FUNC_DEBUG();
				/* Enable memory access */	
				sdla_bus_read_4(hw, AFT_CHIP_CFG_REG, &reg1);
				reg = reg1;
				wan_clear_bit(AFT_CHIPCFG_SFR_IN_BIT, &reg);
				wan_clear_bit(AFT_CHIPCFG_SFR_EX_BIT, &reg);
				sdla_bus_write_4(hw, AFT_CHIP_CFG_REG, reg);
		
				cpld_off = AFT_SH_CPLD_BOARD_STATUS_REG;
				sdla_hw_read_cpld(hw, cpld_off, &status);
				hwcard->hwec_chan_no = 0;
				break;
			case AFT_ADPTR_2SERIAL_V35X21:
			case AFT_ADPTR_4SERIAL_V35X21:
			case AFT_ADPTR_2SERIAL_RS232:
			case AFT_ADPTR_4SERIAL_RS232:
				/* AFT-SERIAL: Add code here */
				break;
			}
			break;
		}
		/* Only for those cards that have PLX PCI Express chip as master */
		if (hwcard->pci_bridge_dev){
			u_int16_t	vendor_id;
			u_int8_t	val;

			sdla_plxctrl_read8(hw, 0x00, &val);
			/* For now, all PLX with blank EEPROM is our new
			** cards from production. */
			if (val == PLX_EEPROM_ENABLE){

				sdla_plxctrl_read8(hw, PLX_EEPROM_VENDOR_OFF, &val);
				vendor_id = val << 8;	
				sdla_plxctrl_read8(hw, PLX_EEPROM_VENDOR_OFF+1, &val);
				vendor_id |= val;	
				if (vendor_id != SANGOMA_PCI_VENDOR){
					hwcard->pci_bridge_dev = NULL;
					hwcard->pci_bridge_bus = 0; 
					hwcard->pci_bridge_slot = 0;
				}
			}
		}
	}
	if (hwcard->hwec_chan_no){
		/* These card has Echo Cancellation module */
#if defined(__WINDOWS__)
		/* Special case for Windows driver:
		** Serial number of the physical card, NOT changing 
		** when a port is restarted. */
		hwcard->hwec_ind = get_card_serial_number(hw->p_sdla);
#else
		hwcard->hwec_ind = ++sdla_hwec_no;
#endif
	}
	sdla_memory_unmap(hw);
	return 0;
}

/*
*****************************************************************************
**			sdla_hw_select
*****************************************************************************
*/
static int
sdla_s514_hw_select (sdlahw_card_t* hwcard, int cpu_no, int irq, void* dev)
{
	sdlahw_cpu_t	*hwcpu=NULL;
	sdlahw_t	*hw=NULL;
	int 		number_of_cards = 0;
	
	hwcard->cfg_type = WANOPT_S51X;
	hwcard->type = SDLA_S514;
	sdla_get_hwcard_name(hwcard);
	sdla_adapter_cnt.s514x_adapters++;
	switch(hwcard->adptr_type){
	case S5144_ADPTR_1_CPU_T1E1:
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return -EINVAL;
		}
		if ((hw = sdla_hwport_te1_register(hwcpu, 2)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 2;
		DEBUG_EVENT(
		"%s: %s T1/E1 card found, cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			hwcard->bus_no, hwcard->slot_no, irq);

		break;

	case S5145_ADPTR_1_CPU_56K:
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return -EINVAL;
		}
		if ((hw = sdla_hwport_register(hwcpu, 2)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
 		number_of_cards += 2;
		DEBUG_EVENT(
		"%s: %s 56K card found, cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			hwcard->bus_no, hwcard->slot_no, irq);

		break;
			

	case S5142_ADPTR_2_CPU_SERIAL:
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return -EINVAL;
		}
		if ((hw = sdla_hwport_register(hwcpu, 2)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 2;
		/* Print the message only for CPU A.
		** BSD calls this function for both CPUs */
		if (cpu_no == SDLA_CPU_A){
			DEBUG_EVENT(
			"%s: %s V35/RS232 card found, cpu(s) 2, bus #%d, slot #%d, irq #%d\n",
				wan_drvname,
				hwcard->adptr_name,
				hwcard->bus_no, hwcard->slot_no, irq);
		}else{
#if !defined(SDLA_AUTO_PROBE)
# if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
			sdla_adapter_cnt.s514x_adapters--;
# endif
#endif
		}
			
#if defined(__LINUX__) || defined(SDLA_AUTO_PROBE)
		if ((hwcpu = sdla_hwcpu_register(hwcard, SDLA_CPU_B, irq, dev)) == NULL){
			return -EINVAL;
		}
		/* Aug 10, 2007
		** Do not overwrite first hw for this Serial card */
		if (sdla_hwport_register(hwcpu, 2) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 2;
#endif
		break;

	
	case S5143_ADPTR_1_CPU_FT1:
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return -EINVAL;
		}
		if ((hw = sdla_hwport_te1_register(hwcpu, 1)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 1;
		DEBUG_EVENT(
		"%s: %s FT1 card found, cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			hwcard->bus_no, hwcard->slot_no, irq);

		break;
			
	case S5147_ADPTR_2_CPU_T1E1:
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return -EINVAL;
		}
		if ((hw = sdla_hwport_te1_register(hwcpu, 1)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 1;
		/* Print the message only for CPU A.
		** BSD calls this function for both CPUs */
		if (cpu_no == SDLA_CPU_A){
			DEBUG_EVENT(
			"%s: %s T1/E1 card found, cpu(s) 2, bus #%d, slot #%d, irq #%d\n",
				wan_drvname,
				hwcard->adptr_name, 
				hwcard->bus_no, hwcard->slot_no, irq);
		}else{
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
			sdla_adapter_cnt.s514x_adapters--;
#endif
		}

#if defined(__LINUX__)
		if ((hwcpu = sdla_hwcpu_register(hwcard, SDLA_CPU_B, irq, dev)) == NULL){
			return -EINVAL;
		}
		/* Aug 10, 2007
		** Do not overwrite first hw for this Serial card */
		if (sdla_hwport_te1_register(hwcpu, 1) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 1;
#endif
		break;

	case S5148_ADPTR_1_CPU_T1E1:
		hwcard->adptr_type = S5144_ADPTR_1_CPU_T1E1;
		sdla_get_hwcard_name(hwcard);
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return -EINVAL;
		}
		if ((hw = sdla_hwport_te1_register(hwcpu, 1)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 1;
		DEBUG_EVENT(
		"%s: S514-8-PCI T1/E1 card found, cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname, hwcard->bus_no, hwcard->slot_no, irq);
		break;

	default:
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return -EINVAL;
		}
		if ((hw = sdla_hwport_register(hwcpu, 2)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 2;
		DEBUG_EVENT(
		"%s: S514-1-PCI V35/RS232/FT1 card found, cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
	        	wan_drvname, hwcard->bus_no, hwcard->slot_no, irq);
		break;		
	}
	while(hw){
		sdla_save_hw_probe(hw);
		hw = WAN_LIST_NEXT(hw, next);
	}
	return number_of_cards;
}

static int
sdla_adsl_hw_select (sdlahw_card_t* hwcard, int cpu_no, int irq, void* dev)
{
	sdlahw_cpu_t	*hwcpu = NULL;
	sdlahw_t	*hw=NULL;
	int 		number_of_cards = 0;
	
	hwcard->cfg_type = WANOPT_ADSL;
	hwcard->type = SDLA_ADSL;
	sdla_get_hwcard_name(hwcard);
	switch(hwcard->adptr_type){
	case S518_ADPTR_1_CPU_ADSL:
		sdla_adapter_cnt.s518_adapters++;
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return 0;
		}
		if ((hw = sdla_hwport_register(hwcpu, 1)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 1;
		DEBUG_EVENT(
		"%s: %s ADSL card found, cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname, 
			hwcard->adptr_name,
			hwcard->bus_no, hwcard->slot_no, irq);
		break;

	default:
		DEBUG_EVENT(
			"%s: Unknown GSI adapter (bus #%d, slot #%d, irq #%d)!\n",
                       	wan_drvname, hwcard->bus_no, hwcard->slot_no, irq);
		break;		
	}

	while(hw){
		sdla_save_hw_probe(hw);
		hw = WAN_LIST_NEXT(hw, next);
	}
	return number_of_cards;
}

static int sdla_aft_hw_select (sdlahw_card_t* hwcard, int cpu_no, int irq, void* dev)
{
	sdlahw_cpu_t	*hwcpu=NULL;
	sdlahw_t	*hw=NULL;
#if defined(__LINUX__) || defined(SDLA_AUTO_PROBE)
	sdlahw_cpu_t	*hwcpu2=NULL;
	sdlahw_t	*hw2= NULL;
#endif
	int		number_of_cards = 0, ports_no = 1;
	
	hwcard->type = SDLA_AFT;
	sdla_get_hwcard_name(hwcard);
	switch(hwcard->adptr_type){
	case A101_ADPTR_1TE1:
		hwcard->cfg_type = WANOPT_AFT;
		sdla_adapter_cnt.aft101_adapters++;
		if (hwcard->adptr_subtype == AFT_SUBTYPE_NORMAL){
			hwcard->cfg_type = WANOPT_AFT;
		}else if (hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
			hwcard->cfg_type = WANOPT_AFT101;
		}
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return 0;
		}
		if ((hw = sdla_hwport_te1_register(hwcpu, 1)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		sdla_get_hw_info(hw);
		sdla_get_hwcard_name(hwcard);
		number_of_cards += 1;
		DEBUG_EVENT(
		"%s: %s%s T1/E1 card found (%s rev.%X), cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			AFT_PCITYPE_DECODE(hwcard),
			AFT_CORE_ID_DECODE(hwcard->core_id),
			hwcard->core_rev,
			hwcard->bus_no, hwcard->slot_no, irq);
		break;

	case A101_ADPTR_2TE1:
		if (hwcard->adptr_subtype == AFT_SUBTYPE_NORMAL){
			hwcard->cfg_type = WANOPT_AFT;
			sdla_adapter_cnt.aft101_adapters++;
			if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
				return 0;
			}
			if ((hw = sdla_hwport_te1_register(hwcpu, 1)) == NULL){
				sdla_hwcpu_unregister(hwcpu);
				return 0;
			}
			sdla_get_hw_info(hw);
			sdla_get_hwcard_name(hwcard);
			number_of_cards += 1;
#if defined(__LINUX__)
			if (hwcard->pci_dev->resource[1].flags){
				if ((hwcpu2 = sdla_hwcpu_register(hwcard, SDLA_CPU_B, irq, dev)) == NULL){
					return 0;
				}
				if ((hw2 = sdla_hwport_te1_register(hwcpu2, 1)) == NULL){
					sdla_hwcpu_unregister(hwcpu);
					return 0;
				}
				number_of_cards += 1;
			}	
#elif defined(__FreeBSD__) && defined(SDLA_AUTO_PROBE)
			if ((hwcpu2 = sdla_hwcpu_register(hwcard, SDLA_CPU_B, irq, dev)) == NULL){
				return 0;
			}
			if ((hw2 = sdla_hwport_te1_register(hwcpu2, 1)) == NULL){
				sdla_hwcpu_unregister(hwcpu2);
				return 0;
			}
			number_of_cards += 1;
#endif
			if (cpu_no == SDLA_CPU_A){
				DEBUG_EVENT(
				"%s: %s%s T1/E1 card found (%s rev.%X), cpu(s) 2, bus #%d, slot #%d, irq #%d\n",
					wan_drvname,
					hwcard->adptr_name, 
					AFT_PCITYPE_DECODE(hwcard),
					AFT_CORE_ID_DECODE(hwcard->core_id),
					hwcard->core_rev,
					hwcard->bus_no, hwcard->slot_no, irq);
			}else{
#if !defined(SDLA_AUTO_PROBE)
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
				sdla_adapter_cnt.aft101_adapters--;
#endif
#endif
			}
		}else if (hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
			hwcard->cfg_type = WANOPT_AFT102;
			sdla_adapter_cnt.aft101_adapters++;
			if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
				return 0;
			}
			if ((hw = sdla_hwport_te1_register(hwcpu, 2)) == NULL){
				sdla_hwcpu_unregister(hwcpu);
				return 0;
			}
			sdla_get_hw_info(hw);
			number_of_cards += 2;
			DEBUG_EVENT(
			"%s: %s%s T1/E1 card found (%s rev.%X), cpu(s) 1, line(s) 2, bus #%d, slot #%d, irq #%d\n",
				wan_drvname,
				hwcard->adptr_name,
				AFT_PCITYPE_DECODE(hwcard),
				AFT_CORE_ID_DECODE(hwcard->core_id),
				hwcard->core_rev,
				hwcard->bus_no, hwcard->slot_no, irq);
		}
		break;

	case A104_ADPTR_4TE1:
		hwcard->cfg_type = WANOPT_AFT104;
		sdla_adapter_cnt.aft104_adapters++;
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return 0;
		}
		if ((hw = sdla_hwport_te1_register(hwcpu, 4)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		sdla_get_hw_info(hw);
		sdla_get_hwcard_name(hwcard);
		number_of_cards += 4;

		DEBUG_EVENT(
		"%s: %s%s T1/E1 card found (%s rev.%X), cpu(s) 1, line(s) 4, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			AFT_PCITYPE_DECODE(hwcard),
			AFT_CORE_ID_DECODE(hwcard->core_id),
			hwcard->core_rev,
			hwcard->bus_no, hwcard->slot_no, irq);
		break;

	case A108_ADPTR_8TE1:
		hwcard->cfg_type = WANOPT_AFT108;
		sdla_adapter_cnt.aft108_adapters++;
		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return 0;
		}
		if ((hw = sdla_hwport_te1_register(hwcpu, 8)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 8;
		sdla_get_hw_info(hw);
		DEBUG_EVENT(
		"%s: %s%s T1/E1 card found (%s rev.%X), cpu(s) 1, line(s) 8, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			AFT_PCITYPE_DECODE(hwcard),
			AFT_CORE_ID_DECODE(hwcard->core_id),
			hwcard->core_rev,
			hwcard->bus_no, hwcard->slot_no, irq);
		break;

	case A300_ADPTR_U_1TE3:
		hwcard->cfg_type = WANOPT_AFT300;
		sdla_adapter_cnt.aft300_adapters++;

		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return 0;
		}
		if ((hw = sdla_hw_register(hwcpu, 0)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 1;
		sdla_get_hw_info(hw);
		DEBUG_EVENT(
		"%s: %s%s T3/E3 card found (%s rev.%X), cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			AFT_PCITYPE_DECODE(hwcard),
			AFT_CORE_ID_DECODE(hwcard->core_id),
			hwcard->core_rev,
			hwcard->bus_no, hwcard->slot_no, irq);
		break;

	case A200_ADPTR_ANALOG:
	case A400_ADPTR_ANALOG:
		hwcard->cfg_type = WANOPT_AFT_ANALOG;
		sdla_adapter_cnt.aft200_adapters++;

		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return 0;
		}
		if ((hw = sdla_hwport_Remora_register(hwcpu, &ports_no)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards ++;
		sdla_get_hw_info(hw);
		
		DEBUG_EVENT(
		"%s: %s%s FXO/FXS card found (%s rev.%X), cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			AFT_PCITYPE_DECODE(hwcard),
			AFT_CORE_ID_DECODE(hwcard->core_id),
			hwcard->core_rev,
			hwcard->bus_no, hwcard->slot_no, irq);
		break;
		
	case AFT_ADPTR_ISDN:
		hwcard->cfg_type = WANOPT_AFT_ISDN;
		sdla_adapter_cnt.aft_isdn_adapters++;

		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return 0;
		}
		if ((hw = sdla_hwport_ISDN_register(hwcpu, &ports_no)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		sdla_get_hw_info(hw);

		/* Verify BRI TE/NT modules */
		number_of_cards += ports_no;
		
		DEBUG_EVENT(
		"%s: %s%s ISDN BRI card found (%s rev.%X), cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			AFT_PCITYPE_DECODE(hwcard),
			AFT_CORE_ID_DECODE(hwcard->core_id),
			hwcard->core_rev,
			hwcard->bus_no, hwcard->slot_no, irq);
		break;
		
	case AFT_ADPTR_56K:
		hwcard->cfg_type = WANOPT_AFT_56K;
		sdla_adapter_cnt.aft_56k_adapters++;

		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return 0;
		}
		if ((hw = sdla_hwport_register(hwcpu, 1)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += 1;
		sdla_get_hw_info(hw);
		
		DEBUG_EVENT(
		"%s: %s%s 56K card found (%s rev.%X), cpu(s) 1, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			AFT_PCITYPE_DECODE(hwcard),
			AFT_CORE_ID_DECODE(hwcard->core_id),
			hwcard->core_rev,
			hwcard->bus_no, hwcard->slot_no, irq);
		break;
		
	case AFT_ADPTR_2SERIAL_V35X21:
	case AFT_ADPTR_4SERIAL_V35X21:
	case AFT_ADPTR_2SERIAL_RS232:
	case AFT_ADPTR_4SERIAL_RS232:
		if (hwcard->adptr_type == AFT_ADPTR_2SERIAL_V35X21 || 
		    hwcard->adptr_type == AFT_ADPTR_2SERIAL_RS232){
			ports_no = 2;
		}else{
			ports_no = 4;
		}
		hwcard->cfg_type = WANOPT_AFT_SERIAL;
		sdla_adapter_cnt.aft_serial_adapters++;

		if ((hwcpu = sdla_hwcpu_register(hwcard, cpu_no, irq, dev)) == NULL){
			return 0;
		}
		if ((hw = sdla_hwport_serial_register(hwcpu, ports_no)) == NULL){
			sdla_hwcpu_unregister(hwcpu);
			return 0;
		}
		number_of_cards += ports_no;
		sdla_get_hw_info(hw);
		
		DEBUG_EVENT(
		"%s: %s%s Serial card found (%s rev.%X), cpu(s) %d, bus #%d, slot #%d, irq #%d\n",
			wan_drvname,
			hwcard->adptr_name,
			AFT_PCITYPE_DECODE(hwcard),
			AFT_CORE_ID_DECODE(hwcard->core_id),
			hwcard->core_rev, ports_no,
			hwcard->bus_no, hwcard->slot_no, irq);
		break;

	default:
		DEBUG_EVENT(
		"%s: Unknown adapter %04X (bus #%d, slot #%d, irq #%d)!\n",
			wan_drvname,
			hwcard->adptr_type,
			hwcard->bus_no,
			hwcard->slot_no,
			irq);
		break;		
	}
	while(hw){
		sdla_save_hw_probe(hw);
		hw = WAN_LIST_NEXT(hw, next);
	}
	return number_of_cards;
}

/*
*****************************************************************************
**			sdla_pci_probe
*****************************************************************************
*/
static int
sdla_pci_probe_S(sdlahw_t *hw, int slot, int bus, int irq)
{
	sdlahw_card_t	*hwcard = NULL, *tmp_hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	u16		PCI_subsys_vendor;
	u16		pci_subsystem_id;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	tmp_hwcard = hwcpu->hwcard;

	sdla_pci_read_config_word(hw,PCI_SUBSYS_VENDOR_WORD,&PCI_subsys_vendor);
		
	if(PCI_subsys_vendor != SANGOMA_SUBSYS_VENDOR) return 0;

	sdla_pci_read_config_word(hw,PCI_SUBSYS_ID_WORD,&pci_subsystem_id);
	
	hwcard = sdla_card_register(SDLA_PCI_CARD, slot, bus, 0);  
	if (hwcard == NULL) return 0;

	hwcard->adptr_type	= pci_subsystem_id & 0xFF;
	hwcard->pci_dev		= tmp_hwcard->pci_dev;

	/* A dual cpu card can support up to 4 physical connections,
	 * where a single cpu card can support up to 2 physical
	 * connections.  The FT1 card can only support a single 
	 * connection, however we cannot distinguish between a Single
	 * CPU card and an FT1 card. */
	return sdla_s514_hw_select(hwcard, SDLA_CPU_A, irq, NULL);
}

static int
sdla_pci_probe_adsl(sdlahw_t *hw, int slot, int bus, int irq)
{
	sdlahw_card_t	*hwcard = NULL, *tmp_hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	u16		pci_subsystem_id;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	tmp_hwcard = hwcpu->hwcard;

	sdla_pci_read_config_word(hw, PCI_SUBSYS_ID_WORD, &pci_subsystem_id);

#if 0
	if ((pci_subsystem_id & 0xFF) != S518_ADPTR_1_CPU_ADSL){
		continue;
	}
#else
	pci_subsystem_id=S518_ADPTR_1_CPU_ADSL;
#endif
		
	hwcard = sdla_card_register(SDLA_PCI_CARD, slot, bus, 0);
	if (hwcard == NULL) return 0;

	hwcard->adptr_type	= pci_subsystem_id & 0xFF;
	hwcard->pci_dev		= tmp_hwcard->pci_dev;
		
	return sdla_adsl_hw_select(hwcard, SDLA_CPU_A, irq, NULL);
}

static int
sdla_pci_probe_adsl_2(sdlahw_t *hw, int slot, int bus, int irq)
{
	sdlahw_card_t	*hwcard = NULL, *tmp_hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	u16		pci_subsystem_id;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	tmp_hwcard = hwcpu->hwcard;

	sdla_pci_read_config_word(hw, PCI_SUBSYS_ID_WORD, &pci_subsystem_id);

	pci_subsystem_id=S518_ADPTR_1_CPU_ADSL;
	
	hwcard = sdla_card_register(SDLA_PCI_CARD, slot, bus, 0); 
	if (hwcard == NULL){
		return 0;
	}
	hwcard->adptr_type	= pci_subsystem_id & 0xFF;
	hwcard->pci_dev		= tmp_hwcard->pci_dev;
	
	return sdla_adsl_hw_select(hwcard, SDLA_CPU_A, irq, NULL);
}

static int 
sdla_pci_probe_aft(sdlahw_t *hw, int slot_no, int bus_no, int irq)
{
	sdlahw_card_t	*hwcard = NULL, *tmp_hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	u16		pci_device_id;
	u16		PCI_subsys_vendor;
	u16		pci_subsystem_id;
#if defined(__LINUX__)
        struct pci_dev*	pci_bridge_dev = NULL;
	struct pci_bus*	bus = NULL;
#endif

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard->pci_dev == NULL);
	hwcpu = hw->hwcpu;
	tmp_hwcard = hwcpu->hwcard;

	/* ALEX 11/18 
	 * Supporting different aft cards */
 	sdla_pci_read_config_word(hw, 
				PCI_DEVICE_ID_WORD, 
				&pci_device_id);
	if (pci_device_id == SANGOMA_PCI_DEVICE || pci_device_id == SANGOMA_PCI_4_DEVICE){
		/* Old A-series cards, keep original sequence */
		return 0;
	}
	sdla_pci_read_config_word(hw, 
				PCI_SUBSYS_VENDOR_WORD, 
				&PCI_subsys_vendor);
	sdla_pci_read_config_word(hw, 
				PCI_SUBSYS_ID_WORD, 
				&pci_subsystem_id);

	hwcard = sdla_card_register(SDLA_PCI_CARD, slot_no, bus_no, 0); 
	if (hwcard == NULL){
		return 0;
	}

	switch(PCI_subsys_vendor){	
	case A101_1TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_1TE1;
		break;
	case A101_2TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_2TE1;
		break;
	case A104_4TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A104_ADPTR_4TE1;
		break;
	case AFT_1TE1_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_1TE1;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case AFT_2TE1_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_2TE1;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case AFT_4TE1_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= A104_ADPTR_4TE1;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case AFT_8TE1_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= A108_ADPTR_8TE1;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case A300_UTE3_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= A300_ADPTR_U_1TE3;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case A305_CTE3_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= A305_ADPTR_C_1TE3;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case A200_REMORA_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= A200_ADPTR_ANALOG;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case A400_REMORA_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= A400_ADPTR_ANALOG;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case AFT_ISDN_BRI_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= AFT_ADPTR_ISDN;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case AFT_56K_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= AFT_ADPTR_56K;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case AFT_2SERIAL_V35X21_SUBSYS_VENDOR:
		hwcard->adptr_type	= AFT_ADPTR_2SERIAL_V35X21;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case AFT_4SERIAL_V35X21_SUBSYS_VENDOR:
		hwcard->adptr_type	= AFT_ADPTR_4SERIAL_V35X21;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case AFT_2SERIAL_RS232_SUBSYS_VENDOR:
		hwcard->adptr_type	= AFT_ADPTR_2SERIAL_RS232;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	case AFT_4SERIAL_RS232_SUBSYS_VENDOR:
		hwcard->adptr_type	= AFT_ADPTR_4SERIAL_RS232;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	default:
		DEBUG_EVENT(
		"%s: Unsupported SubVendor ID:%04X (bus=%d, slot=%d)\n",
				wan_drvname,
				PCI_subsys_vendor, slot_no, bus_no);
		sdla_card_unregister(SDLA_PCI_CARD, slot_no, bus_no, 0); 
		return 0;
	}
#if defined(__LINUX__)
	/* Detect PCI Express cards (only valid for production test) */
	switch(PCI_subsys_vendor){	
	case A200_REMORA_SHARK_SUBSYS_VENDOR:
	case A400_REMORA_SHARK_SUBSYS_VENDOR:
	case AFT_1TE1_SHARK_SUBSYS_VENDOR:
	case AFT_2TE1_SHARK_SUBSYS_VENDOR:
	case AFT_4TE1_SHARK_SUBSYS_VENDOR:
	case AFT_8TE1_SHARK_SUBSYS_VENDOR:
	case AFT_ISDN_BRI_SHARK_SUBSYS_VENDOR:
	case AFT_56K_SHARK_SUBSYS_VENDOR:
	case AFT_2SERIAL_V35X21_SUBSYS_VENDOR:
	case AFT_4SERIAL_V35X21_SUBSYS_VENDOR:
	case AFT_2SERIAL_RS232_SUBSYS_VENDOR:
	case AFT_4SERIAL_RS232_SUBSYS_VENDOR:
		if (tmp_hwcard->pci_dev->bus == NULL) break;
		bus = tmp_hwcard->pci_dev->bus;
		if (bus->self == NULL) break;
		pci_bridge_dev = bus->self;
		if (pci_bridge_dev->vendor == PLX_VENDOR_ID && 
		    (pci_bridge_dev->device == PLX_DEVICE_ID ||
		     pci_bridge_dev->device == PLX2_DEVICE_ID)){
			hwcard->pci_bridge_dev = pci_bridge_dev;
			hwcard->pci_bridge_bus = bus_no; 
			hwcard->pci_bridge_slot = slot_no;
			DEBUG_TEST("%s: PCI-Express card (bus:%d, slot:%d)\n",
				wan_drvname, bus_no, slot_no);
		}
		break;
	}	
#endif
		
	hwcard->core_id	= AFT_CORE_ID(pci_subsystem_id);
	hwcard->core_rev= AFT_CORE_REV(pci_subsystem_id);
	if (hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
		switch(hwcard->core_id){
		case AFT_ANALOG_FE_CORE_ID:
		case AFT_PMC_FE_CORE_ID:
		case AFT_DS_FE_CORE_ID:
			break;
		default:
			DEBUG_EVENT("%s: Unsupported core id (%X)\n", 
					wan_drvname, hwcard->core_id);
			sdla_card_unregister(SDLA_PCI_CARD, slot_no, bus_no, 0); 
			return 0;
			break;
		}
	}
	hwcard->pci_dev	= tmp_hwcard->pci_dev;
	return sdla_aft_hw_select(hwcard, SDLA_CPU_A, irq, NULL);
}

static int
sdla_pci_probe_aft_v2(sdlahw_t *hw, int slot_no, int bus_no, int irq)
{
	sdlahw_card_t	*hwcard = NULL, *tmp_hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	u16		PCI_subsys_vendor;
	u16		pci_subsystem_id;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	tmp_hwcard = hwcpu->hwcard;

	/* ALEX 11/18 
	 * Supporting different aft cards */
	sdla_pci_read_config_word(hw, 
			PCI_SUBSYS_VENDOR_WORD, 
			&PCI_subsys_vendor);

	hwcard = sdla_card_register(SDLA_PCI_CARD, slot_no, bus_no, 0); 
	if (hwcard == NULL){
		return 0;
	}

	switch(PCI_subsys_vendor){	
	case A101_1TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_1TE1;
		break;
	case A101_2TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_2TE1;
		break;
	case A300_UTE3_SUBSYS_VENDOR:
		hwcard->adptr_type	= A300_ADPTR_U_1TE3;
		break;
	default:
		DEBUG_EVENT("%s: Unsupported subsystem vendor id %04X (bus=%d, slot=%d)\n",
				wan_drvname, PCI_subsys_vendor, bus_no, slot_no); 
		return 0;
	}

	sdla_pci_read_config_word(hw, 
				PCI_SUBSYS_ID_WORD, 
				&pci_subsystem_id);
	hwcard->core_id	= AFT_CORE_ID(pci_subsystem_id);
	hwcard->core_rev= AFT_CORE_REV(pci_subsystem_id);
	hwcard->pci_dev	= tmp_hwcard->pci_dev;
	return sdla_aft_hw_select(hwcard, SDLA_CPU_A, irq, NULL);
}

static int
sdla_pci_probe_aft_v1(sdlahw_t *hw, int slot_no, int bus_no, int irq)
{
	sdlahw_card_t	*hwcard = NULL, *tmp_hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	u16		PCI_subsys_vendor;
	u16		pci_subsystem_id;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	tmp_hwcard = hwcpu->hwcard;

	/* ALEX 11/18 
	 * Supporting different aft cards */
	sdla_pci_read_config_word(hw, 
				PCI_SUBSYS_VENDOR_WORD, 
				&PCI_subsys_vendor);
	sdla_pci_read_config_word(hw, 
				PCI_SUBSYS_ID_WORD, 
				&pci_subsystem_id);
	hwcard = sdla_card_register(SDLA_PCI_CARD, slot_no, bus_no, 0); 
	if (hwcard == NULL){
		return 0;
	}

	switch(PCI_subsys_vendor){	
	case A101_1TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_1TE1;
		break;
	case A101_2TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_2TE1;
		break;
	case A104_4TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A104_ADPTR_4TE1;
		break;
	case AFT_4TE1_SHARK_SUBSYS_VENDOR:
		hwcard->adptr_type	= A104_ADPTR_4TE1;
		hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		break;
	default:
		DEBUG_EVENT("%s: Unsupported subsystem vendor id %04X (bus=%d, slot=%d)\n",
				wan_drvname, PCI_subsys_vendor, bus_no, slot_no); 
		return 0;
	}

	hwcard->core_id = AFT_CORE_ID(pci_subsystem_id);
	hwcard->core_rev= AFT_CORE_REV(pci_subsystem_id);
	hwcard->pci_dev	= tmp_hwcard->pci_dev;
	return sdla_aft_hw_select(hwcard, SDLA_CPU_A, irq, NULL);
}

static int
sdla_pci_probe_aft_old(sdlahw_t *hw, int slot_no, int bus_no, int irq)
{
	sdlahw_card_t	*hwcard = NULL, *tmp_hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	u16		PCI_subsys_vendor;
	u16		pci_subsystem_id;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	tmp_hwcard = hwcpu->hwcard;

	/* ALEX 11/18 
	 * Supporting different aft cards */
	sdla_pci_read_config_word(hw, 
			PCI_SUBSYS_VENDOR_WORD, 
			&PCI_subsys_vendor);
	hwcard = sdla_card_register(SDLA_PCI_CARD, slot_no, bus_no, 0); 
	if (hwcard == NULL) return 0;

	switch(PCI_subsys_vendor){	
	case A101_1TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_1TE1;
		break;
	case A101_2TE1_SUBSYS_VENDOR:
		hwcard->adptr_type	= A101_ADPTR_2TE1;
		break;
	case A300_UTE3_SUBSYS_VENDOR:
		hwcard->adptr_type	= A300_ADPTR_U_1TE3;
		break;
	default:
		DEBUG_EVENT("%s: Unsupported subsystem vendor id %04X (bus=%d, slot=%d)\n",
				wan_drvname, PCI_subsys_vendor, bus_no, slot_no); 
		return 0;
	}

	sdla_pci_read_config_word(hw, 
				PCI_SUBSYS_ID_WORD, 
				&pci_subsystem_id);
	hwcard->core_id	= AFT_CORE_ID(pci_subsystem_id);
	hwcard->core_rev= AFT_CORE_REV(pci_subsystem_id);
	hwcard->pci_dev	= tmp_hwcard->pci_dev;
	return sdla_aft_hw_select(hwcard, SDLA_CPU_A, irq, NULL);
}

#if defined(__LINUX__)
static int sdla_pci_probe(sdlahw_t *hw)
{
//	sdlahw_card_t	*hwcard = NULL;
	sdlahw_card_t	*tmp_hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
        int		number_pci_cards = 0;
//	u16		pci_device_id;
//      u16		PCI_subsys_vendor;
//	u16		pci_subsystem_id;
        struct pci_dev	*pci_dev = NULL;
//        struct pci_dev	*pci_bridge_dev = NULL;
//	struct pci_bus*	bus = NULL;
 
	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	tmp_hwcard = hwcpu->hwcard;

	while ((pci_dev = pci_get_device(V3_VENDOR_ID, V3_DEVICE_ID, pci_dev))
        	!= NULL) {
		
		tmp_hwcard->pci_dev = pci_dev;
		number_pci_cards += sdla_pci_probe_S(
					hw,  
					((pci_dev->devfn >> 3) & PCI_DEV_SLOT_MASK),
					pci_dev->bus->number,
					pci_dev->irq);
        }

	/* Search for Pulsar PCI cards */
	pci_dev = NULL;
	while ((pci_dev = pci_get_device(PCI_VENDOR_ID_GSI, PCI_DEVICE_ID_GSI_ADSL, pci_dev))
        	!= NULL) {
        
		tmp_hwcard->pci_dev = pci_dev;
		number_pci_cards += sdla_pci_probe_adsl(
					hw,	
				 	((pci_dev->devfn >> 3) & PCI_DEV_SLOT_MASK),
				 	pci_dev->bus->number,
					pci_dev->irq);
        }

	/* Search for Pulsar PCI cards */
	pci_dev = NULL;
	while ((pci_dev = pci_get_device(PCI_VENDOR_ID_GSI, PCI_DEVICE_ID_GSI_ADSL_V2, pci_dev))
        	!= NULL) {
        
		tmp_hwcard->pci_dev = pci_dev;	
		number_pci_cards += sdla_pci_probe_adsl_2(
					hw,	
				 	((pci_dev->devfn >> 3) & PCI_DEV_SLOT_MASK),
				 	pci_dev->bus->number,
					pci_dev->irq);
        }

	pci_dev=NULL;
	while((pci_dev = pci_get_device(SANGOMA_PCI_VENDOR, PCI_ANY_ID, pci_dev)) != NULL){
		
		tmp_hwcard->pci_dev = pci_dev;	
		number_pci_cards += sdla_pci_probe_aft(
					hw,
					((pci_dev->devfn >> 3) & PCI_DEV_SLOT_MASK),
					pci_dev->bus->number,
					pci_dev->irq);
        }

	pci_dev=NULL;
	while((pci_dev = pci_get_device(SANGOMA_PCI_VENDOR, SANGOMA_PCI_DEVICE, pci_dev)) != NULL){
		
		tmp_hwcard->pci_dev = pci_dev;	
		number_pci_cards += sdla_pci_probe_aft_v2(
					hw,
					((pci_dev->devfn >> 3) & PCI_DEV_SLOT_MASK),
					pci_dev->bus->number,
					pci_dev->irq);
        }

	pci_dev=NULL;
	while((pci_dev = pci_get_device(SANGOMA_PCI_VENDOR, SANGOMA_PCI_4_DEVICE, pci_dev)) != NULL){
		
		tmp_hwcard->pci_dev = pci_dev;	
		number_pci_cards += sdla_pci_probe_aft_v1(
					hw,
					((pci_dev->devfn >> 3) & PCI_DEV_SLOT_MASK),
					pci_dev->bus->number,
					pci_dev->irq);
        }

	pci_dev=NULL;
	while ((pci_dev = pci_get_device(SANGOMA_PCI_VENDOR_OLD, SANGOMA_PCI_DEVICE, pci_dev))
        	!= NULL) {
		
		tmp_hwcard->pci_dev = pci_dev;
		number_pci_cards += sdla_pci_probe_aft_old(
					hw,
					((pci_dev->devfn >> 3) & PCI_DEV_SLOT_MASK),
					pci_dev->bus->number,
					pci_dev->irq);
        }

	return number_pci_cards;
}


EXPORT_SYMBOL(sdla_hw_bridge_probe);
unsigned int sdla_hw_bridge_probe(void)
{
	sdlahw_card_t*	hwcard = NULL;
        unsigned int	number_pci_x_bridges = 0;
        struct pci_dev*	pci_dev = NULL, *tmp;
	struct pci_bus*	bus = NULL;
 
	pci_dev = NULL;
	while ((pci_dev = pci_get_device(PLX_VENDOR_ID, PLX_DEVICE_ID, pci_dev))
        	!= NULL) {
		
                //sdla_pci_read_config_word(hw, 
		//			PCI_SUBSYS_VENDOR_WORD, 
		//			&PCI_subsys_vendor);
		
                //if(PCI_subsys_vendor != SANGOMA_SUBSYS_VENDOR)
                //	continue;

		//sdla_pci_read_config_word(hw, 
		//			PCI_SUBSYS_ID_WORD, 
		//			&pci_subsystem_id);

		bus = pci_bus_b(pci_dev->bus->children.next);
		if (bus) DEBUG_EVENT("ADBG> %s:%d:%X %X %X %X %d %d\n",
					__FUNCTION__,__LINE__,
					bus->self->vendor,
					bus->self->device,
					bus->self->subsystem_vendor,
					bus->self->subsystem_device,
       				 	((bus->self->devfn >> 3) & PCI_DEV_SLOT_MASK),
					bus->number);
		bus = pci_bus_b(pci_dev->bus->children.prev);
		if (bus) DEBUG_EVENT("ADBG> %s:%d: %X %X %X %X %d %d\n",
					__FUNCTION__,__LINE__,
					bus->self->vendor,
					bus->self->device,
					bus->self->subsystem_vendor,
					bus->self->subsystem_device,
       				 	((bus->self->devfn >> 3) & PCI_DEV_SLOT_MASK),
					bus->number);
		tmp = pci_dev_b(pci_dev->bus->devices.next);
		if (tmp) DEBUG_EVENT("ADBG> %s:%d: %X %X %X %X %d %d\n",
					__FUNCTION__,__LINE__,
					tmp->vendor,
					tmp->device,
					tmp->subsystem_vendor,
					tmp->subsystem_device,
       				 	((tmp->devfn >> 3) & PCI_DEV_SLOT_MASK),
					tmp->bus->number);
		tmp = pci_dev_b(pci_dev->bus->devices.prev);
		if (tmp) DEBUG_EVENT("ADBG> %s:%d: %X %X %X %X %d %d\n",
					__FUNCTION__,__LINE__,
					tmp->vendor,
					tmp->device,
					tmp->subsystem_vendor,
					tmp->subsystem_device,
       				 	((tmp->devfn >> 3) & PCI_DEV_SLOT_MASK),
					tmp->bus->number);
		break;
		hwcard = sdla_card_register(SDLA_PCI_EXP_CARD, 
					 ((pci_dev->devfn >> 3) & PCI_DEV_SLOT_MASK),
					 pci_dev->bus->number,
					 0);
		if (hwcard == NULL){
			continue;
		}

		//hwcard->adptr_type	= pci_subsystem_id & 0xFF;	
		//hwcard->adptr_type	= A104_ADPTR_X_4TE1;
		//hwcard->adptr_subtype	= AFT_SUBTYPE_SHARK;
		hwcard->pci_dev		= pci_dev;

		number_pci_x_bridges ++;
        }
	return number_pci_x_bridges;
}

#elif defined(__FreeBSD__) 
# if defined(SDLA_AUTO_PROBE)
static int sdla_pci_auto_probe(sdlahw_t *hw)
{
	struct pci_devinfo	*dinfo = NULL;
	sdlahw_card_t	*tmp_hwcard = NULL;
	sdlahw_cpu_t	*hwcpu = NULL;
	int		number_pci_cards = 0, cards = 0;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	tmp_hwcard = hwcpu->hwcard;

	STAILQ_FOREACH(dinfo, &pci_devq, pci_links){
	
		if (dinfo->cfg.vendor == V3_VENDOR_ID && dinfo->cfg.device == V3_DEVICE_ID){

			tmp_hwcard->pci_dev = dinfo->cfg.dev;
			cards = sdla_pci_probe_S(
					hw,  
					pci_get_slot(dinfo->cfg.dev), 
					pci_get_bus(dinfo->cfg.dev),
					dinfo->cfg.intline);
			if (cards){
				number_pci_cards += cards;
				continue;
			}
		}
		if (dinfo->cfg.vendor == PCI_VENDOR_ID_GSI && dinfo->cfg.device == PCI_DEVICE_ID_GSI_ADSL){

			tmp_hwcard->pci_dev = dinfo->cfg.dev;
			cards += sdla_pci_probe_adsl(
						hw,  
						pci_get_slot(dinfo->cfg.dev), 
						pci_get_bus(dinfo->cfg.dev),
						dinfo->cfg.intline);
			if (cards){
				number_pci_cards += cards;
				continue;
			}
		}
		if (dinfo->cfg.vendor == PCI_VENDOR_ID_GSI && dinfo->cfg.device == PCI_DEVICE_ID_GSI_ADSL_V2){

			tmp_hwcard->pci_dev = dinfo->cfg.dev;
			cards = sdla_pci_probe_adsl_2(
						hw,  
						pci_get_slot(dinfo->cfg.dev), 
						pci_get_bus(dinfo->cfg.dev),
						dinfo->cfg.intline);
			if (cards){
				number_pci_cards += cards;
				continue;
			}
		}
		if (dinfo->cfg.vendor == SANGOMA_PCI_VENDOR){

			tmp_hwcard->pci_dev = dinfo->cfg.dev;
			cards = sdla_pci_probe_aft(
						hw,  
						pci_get_slot(dinfo->cfg.dev), 
						pci_get_bus(dinfo->cfg.dev),
						dinfo->cfg.intline);
			if (cards){
				number_pci_cards += cards;
				continue;
			}
		}
		if (dinfo->cfg.vendor == SANGOMA_PCI_VENDOR && dinfo->cfg.device == SANGOMA_PCI_DEVICE){

			tmp_hwcard->pci_dev = dinfo->cfg.dev;
			cards = sdla_pci_probe_aft_v2(
						hw,  
						pci_get_slot(dinfo->cfg.dev), 
						pci_get_bus(dinfo->cfg.dev),
						dinfo->cfg.intline);
			if (cards){
				number_pci_cards += cards;
				continue;
			}
		}
		if (dinfo->cfg.vendor == SANGOMA_PCI_VENDOR && dinfo->cfg.device == SANGOMA_PCI_4_DEVICE){

			tmp_hwcard->pci_dev = dinfo->cfg.dev;
			cards = sdla_pci_probe_aft_v1(
						hw,  
						pci_get_slot(dinfo->cfg.dev), 
						pci_get_bus(dinfo->cfg.dev),
						dinfo->cfg.intline);
			if (cards){
				number_pci_cards += cards;
				continue;
			}
		}
		if (dinfo->cfg.vendor == SANGOMA_PCI_VENDOR_OLD && dinfo->cfg.device == SANGOMA_PCI_DEVICE){

			tmp_hwcard->pci_dev = dinfo->cfg.dev;
			cards = sdla_pci_probe_aft_old(
						hw,  
						pci_get_slot(dinfo->cfg.dev), 
						pci_get_bus(dinfo->cfg.dev),
						dinfo->cfg.intline);
			if (cards){
				number_pci_cards += cards;
				continue;
			}
		}
	}
	return number_pci_cards;
}
# endif
#endif

/*
*****************************************************************************
**			sdla_hw_probe
*****************************************************************************
*/
EXPORT_SYMBOL(sdla_hw_probe);

#if defined(__LINUX__)
unsigned int sdla_hw_probe(void)
{
	sdlahw_card_t*	hwcard;
	sdlahw_card_t 	tmp_hwcard;
	sdlahw_cpu_t 	tmp_hwcpu, *hwcpu;
	sdlahw_t 	tmp_hw;
	unsigned* 	opt = s508_port_options; 
	unsigned int	cardno=0;
	int i;
	
	//if (!WAN_LIST_EMPTY(&sdlahw_card_head)){
	//	DEBUG_EVENT("ADBG> SDLA_HW_PROBE: Number configured cards %d\n",
	//					cardno);
	//	return cardno;
	//}
	
	memset(&tmp_hw, 0, sizeof(tmp_hw));
	memset(&tmp_hwcpu, 0, sizeof(tmp_hwcpu));
	memset(&tmp_hwcard, 0, sizeof(tmp_hwcard));
	tmp_hwcpu.hwcard = &tmp_hwcard;
	tmp_hw.hwcpu = &tmp_hwcpu;
	tmp_hw.magic = SDLADRV_MAGIC;
	
	for (i = 1; i <= opt[0]; i++) {
		tmp_hwcard.hw_type = SDLA_ISA_CARD;
		tmp_hwcard.ioport = opt[i];
		if (!sdla_detect_s508(&tmp_hw)){
			DEBUG_EVENT("%s: S508-ISA card found, port 0x%x\n",
				wan_drvname, tmp_hwcard.ioport);
			hwcard = sdla_card_register(SDLA_ISA_CARD,
						  0,
						  0,
						  tmp_hwcard.ioport);
			if (hwcard == NULL){
				continue;
			}
			hwcard->adptr_type	= 0x00;
			hwcard->pci_dev		= NULL;
			hwcard->cfg_type 	= WANOPT_S50X;

			hwcpu = sdla_hwcpu_register(hwcard, SDLA_CPU_A, 0, NULL);
			if (hwcpu == NULL){
				sdla_card_unregister (
						SDLA_ISA_CARD,
						0,
						0,
						tmp_hwcard.ioport);
				continue;
			}
			sdla_hw_register(hwcpu, 1);
			sdla_hw_register(hwcpu, 2);
			
			/* S508 card can support up to two physical links */
			cardno += 2;

			sdla_adapter_cnt.s508_adapters++;
		}
		tmp_hwcard.ioport = 0x00;
	}

# ifdef CONFIG_PCI
	tmp_hwcard.hw_type = SDLA_PCI_CARD;
	tmp_hwcard.slot_no = 0;
	tmp_hwcard.bus_no = 0;
	cardno += sdla_pci_probe(&tmp_hw);
# else
	DEBUG_EVENT( "Warning, Kernel not compiled for PCI support!\n");
	DEBUG_EVENT( "PCI Hardware Probe Failed!\n");
# endif
	return cardno;
}
#elif defined(__WINDOWS__)
unsigned int sdla_hw_probe(void)
{
	sdlahw_card_t*	hwcard;
	sdlahw_card_t 	tmp_hwcard;
	sdlahw_cpu_t 	tmp_hwcpu, *hwcpu;
	sdlahw_t 	tmp_hw;
	unsigned int	cardno=0;
	int i;
	
	//if (!WAN_LIST_EMPTY(&sdlahw_card_head)){
	//	DEBUG_EVENT("ADBG> SDLA_HW_PROBE: Number configured cards %d\n",
	//					cardno);
	//	return cardno;
	//}
	
	memset(&tmp_hw, 0, sizeof(tmp_hw));
	memset(&tmp_hwcpu, 0, sizeof(tmp_hwcpu));
	memset(&tmp_hwcard, 0, sizeof(tmp_hwcard));
	tmp_hwcpu.hwcard = &tmp_hwcard;
	tmp_hw.hwcpu = &tmp_hwcpu;
	tmp_hw.magic = SDLADRV_MAGIC;
	
# ifdef CONFIG_PCI
	tmp_hwcard.hw_type = SDLA_PCI_CARD;
	tmp_hwcard.slot_no = 0;
	tmp_hwcard.bus_no = 0;
	cardno += sdla_pci_probe(&tmp_hw);
# else
	DEBUG_EVENT( "Warning, Kernel not compiled for PCI support!\n");
	DEBUG_EVENT( "PCI Hardware Probe Failed!\n");
# endif
	return cardno;
}
#else /* !LINUX */
unsigned int sdla_hw_probe(void)
{
#if !defined(SDLA_AUTO_PROBE)
	sdlahw_card_t 	*hwcard;
	int		cnt = 0;
#endif

	/* Probe ISA card */
#if defined(WAN_ISA_SUPPORT)
	for(cnt=0; cnt< Sangoma_cards_no; cnt++){
		sdladev_t	*dev = NULL;
		sdla_pci_dev_t	pci_dev = NULL;
		
		dev = &sdladev[cnt];
		if (dev->type == SDLA_S508){
			sdlahw_card_t	*hwcard;
			sdlahw_cpu_t	*hwcpu;
			sdlahw_t	*hw;
			DEBUG_EVENT( "%s: S508-ISA card found, port 0x%x\n",
					wan_drvname, sdladev_ioport(dev));
			hwcard = sdla_card_register(
						SDLA_ISA_CARD,
						0,
						0,
						sdladev_ioport(dev));
			if (hwcard == NULL) continue;

			hwcard->adptr_type	= 0x00;
			hwcard->pci_dev		= NULL;
			hwcard->cfg_type 	= WANOPT_S50X;

			hwcpu = sdla_hwcpu_register(hwcard, SDLA_CPU_A, 0, dev);
			if (hwcpu == NULL){
				sdla_card_unregister (
						SDLA_ISA_CARD,
						0,
						0,
						sdladev_ioport(dev));
				continue;
			}
	    		hwcpu->irq    	= sdladev_irq(dev);
	    		hwcpu->dpmbase 	= sdladev_maddr(dev);

#if defined(__NetBSD__) || defined(__OpenBSD__)
			hwcpu->ioh		= dev->u.isa.ioh;
    			hwcard->iot    	= dev->sc->ia.ia_iot;
	    		hwcard->memt   	= dev->sc->ia.ia_memt;
#endif /* __NetBSD__ || __OpenBSD__ */
			sdla_hw_register(hwcpu, 1);
			sdla_hw_register(hwcpu, 0);

			card_no += 2;
			sdla_adapter_cnt.s508_adapters++;
		}
	}
#endif


#if defined(SDLA_AUTO_PROBE)
	sdlahw_card_t 	tmp_hwcard;
	sdlahw_cpu_t 	tmp_hwcpu;
	sdlahw_t 	tmp_hw;
	unsigned int	cardno=0;

	memset(&tmp_hw, 0, sizeof(tmp_hw));
	tmp_hwcpu.hwcard = &tmp_hwcard;
	tmp_hw.hwcpu = &tmp_hwcpu;
	tmp_hw.magic = SDLADRV_MAGIC;

	tmp_hwcard.hw_type = SDLA_PCI_CARD;
	tmp_hwcard.slot_no = 0;
	tmp_hwcard.bus_no = 0;

	cardno += sdla_pci_auto_probe(&tmp_hw);
	return cardno;
#else
	/* Probe PCI/PCI-Exp cards */
	for(cnt=0; cnt<Sangoma_cards_no; cnt++){
		sdladev_t	*dev = NULL;
		sdla_pci_dev_t	pci_dev = NULL;
		
		dev = &sdladev[cnt];
# if defined(__FreeBSD__)
#  if (__FreeBSD_version > 400000)
		pci_dev = dev->sc->dev;
#  else
    		pci_dev = dev->u.pci.pci_dev;
#  endif /* __FreeBSD_version */
# else
	    	pci_dev = &dev->sc->pa;
# endif

		if (sdladev_cpu(dev) == SDLA_CPU_A){
			hwcard = sdla_card_register(SDLA_PCI_CARD, 
						  sdladev_slot(dev),
						  sdladev_bus(dev),
						  0);
		}else{
			hwcard = sdla_card_search(SDLA_PCI_CARD, 
						  sdladev_slot(dev),
						  sdladev_bus(dev),
						  0);
		}
		if (hwcard == NULL){
			/* This is only for cards that has more then
			** one CPU. */
			hwcard = sdla_card_search(SDLA_PCI_CARD,
						sdladev_slot(dev),
						sdladev_bus(dev),
						0);
		}
		hwcard->adptr_type	= dev->adapter_type;
		hwcard->adptr_subtype	= dev->adapter_subtype;
		hwcard->pci_dev		= pci_dev;
#if defined(__NetBSD__) || defined(__OpenBSD__)
	    	hwcard->memt		= pci_dev->pa_memt;
#endif
		switch(dev->vendor_id){
		case V3_VENDOR_ID:
			if (dev->subvendor_id == SANGOMA_SUBSYS_VENDOR){
				sdla_s514_hw_select(hwcard, 
						    sdladev_cpu(dev),
						    sdladev_irq(dev),
						    dev);
				
			}else if (dev->device_id == SANGOMA_PCI_DEVICE){
				hwcard->core_id	= AFT_CORE_ID(dev->subsystem_id);
				hwcard->core_rev= AFT_CORE_REV(dev->subsystem_id);
				sdla_aft_hw_select(hwcard,
						      sdladev_cpu(dev),
						      sdladev_irq(dev),
						      dev);
			}else{
				DEBUG_EVENT("%s: Unknown Sangoma device (%04X)\n",
					wan_drvname, dev->device_id);
			}
			break;
			
		case PCI_VENDOR_ID_GSI:
			sdla_adsl_hw_select(hwcard,
					    sdladev_cpu(dev),
					    sdladev_irq(dev),
					    dev);
			break;

		case SANGOMA_PCI_VENDOR:
			hwcard->core_id	= AFT_CORE_ID(dev->subsystem_id);
			hwcard->core_rev= AFT_CORE_REV(dev->subsystem_id);
			sdla_aft_hw_select(hwcard,
					      sdladev_cpu(dev),
					      sdladev_irq(dev),
					      dev);
			break;

		default:
			DEBUG_EVENT("%s: Unknown Sangoma card (%04X)\n",
					wan_drvname, dev->vendor_id);
		}
	}
	return Sangoma_devices_no;
#endif
}
#endif

/*
*****************************************************************************
**			sdla_get_hw_probe
*****************************************************************************
*/
EXPORT_SYMBOL(sdla_get_hw_probe);
void *sdla_get_hw_probe (void)
{
	return WAN_LIST_FIRST(&sdlahw_probe_head);
}	


/*
*****************************************************************************
**			sdla_get_hw_adptr_cnt
*****************************************************************************
*/
EXPORT_SYMBOL(sdla_get_hw_adptr_cnt);
void *sdla_get_hw_adptr_cnt (void)
{
	return &sdla_adapter_cnt;
}	


/*****************************************************************************
**	Hardware structure interface per Physical Card
*****************************************************************************/
static int sdla_card_info(sdlahw_card_t *hwcard)
{
	WAN_ASSERT(hwcard == NULL);
	if (hwcard->hw_type == SDLA_PCI_CARD){
		DEBUG_EVENT("%s: Card info: slot=%d,bus=%d!\n",
				__FUNCTION__,
				hwcard->slot_no, 
				hwcard->bus_no);
	}else{
		DEBUG_EVENT("%s: Card info: ioport=%d!\n",
				__FUNCTION__,
				hwcard->ioport);
	}
	return 0;
}

static sdlahw_card_t* sdla_card_register(	unsigned char	hw_type,
						int		slot_no,
						int		bus_no,
						int		ioport)
{
	sdlahw_card_t	*new_hwcard, *last_hwcard;

	new_hwcard = sdla_card_search(hw_type, slot_no, bus_no, ioport);
	if (new_hwcard){

		DEBUG_EVENT("%s: Card is already exists!\n", __FUNCTION__);
		sdla_card_info(new_hwcard);
		return NULL;
	}
	new_hwcard = wan_malloc(sizeof(sdlahw_card_t));
	if (!new_hwcard){
		return NULL;
	}

	memset(new_hwcard,0,sizeof(sdlahw_card_t));
	
	new_hwcard->hw_type	= hw_type;
	new_hwcard->slot_no	= slot_no;
	new_hwcard->bus_no  	= bus_no;
	new_hwcard->ioport 	= ioport;
	wan_spin_lock_init(&new_hwcard->pcard_lock,"wan_hwcard_lock");
	wan_spin_lock_init(&new_hwcard->pcard_ec_lock,"wan_hwcard_ec_lock");

	WAN_LIST_FOREACH(last_hwcard, &sdlahw_card_head, next){
		if (!WAN_LIST_NEXT(last_hwcard, next)){
			break;
		}
	}
	if (last_hwcard){
		WAN_LIST_INSERT_AFTER(last_hwcard, new_hwcard, next);
	}else{
		WAN_LIST_INSERT_HEAD(&sdlahw_card_head, new_hwcard, next);

	}
	return new_hwcard;
}

static int sdla_card_unregister(	unsigned char	hw_type,
					int		slot_no,
					int		bus_no,
					int		ioport)
{
	sdlahw_card_t*	tmp_card;

	WAN_LIST_FOREACH(tmp_card, &sdlahw_card_head, next){
		if (tmp_card->hw_type != hw_type){
			continue;
		}
		if (tmp_card->hw_type == SDLA_PCI_CARD &&
		    tmp_card->slot_no == slot_no && tmp_card->bus_no == bus_no){
				break;
		}else if (tmp_card->hw_type == SDLA_ISA_CARD &&
			  tmp_card->ioport == ioport){
				break;
		}
	}
	if (tmp_card == NULL){
		if  (hw_type == SDLA_PCI_CARD){
			DEBUG_EVENT("%s: Error: Card didn't find (slot=%d,bus=%d)\n",
					__FUNCTION__,
					slot_no, 
					bus_no);
		}else{
			DEBUG_EVENT("%s: Error: Card didn't find (ioport=%d)\n",
					__FUNCTION__,
					ioport);
		}
		return -EFAULT;
	}
	if (tmp_card->internal_used){
		DEBUG_EVENT("%s: Error: This card is still in used (used=%d)!\n",
					__FUNCTION__,
					tmp_card->internal_used);
		sdla_card_info(tmp_card);
		return -EBUSY;
	}
	WAN_LIST_REMOVE(tmp_card, next);
	wan_free(tmp_card);
	return 0;
}

static sdlahw_card_t* sdla_card_search(	unsigned char	hw_type,
					int		slot_no,
					int		bus_no,
					int		ioport)
{
	sdlahw_card_t*	tmp_card;
	
	WAN_LIST_FOREACH(tmp_card, &sdlahw_card_head, next){
		if (tmp_card->hw_type != hw_type){
			continue;
		}
		switch(tmp_card->hw_type){
		case SDLA_PCI_CARD:
			if (tmp_card->slot_no == slot_no && tmp_card->bus_no == bus_no){
				return tmp_card;
			}
			break;
		case SDLA_ISA_CARD:
			if (tmp_card->ioport == ioport){
				return tmp_card;
			}
			break;
		}
	}
	return NULL;
}

/*****************************************************************************
**	Hardware structure interface per CPU
*****************************************************************************/
static int sdla_hwcpu_info(sdlahw_cpu_t	*hwcpu)
{

	WAN_ASSERT(hwcpu == NULL);
	WAN_ASSERT(hwcpu->hwcard == NULL);
	if (hwcpu->hwcard->hw_type == SDLA_PCI_CARD){
		DEBUG_TEST("%s: HW Card Cpu info: slot=%d,bus=%d,cpu=%c!\n",
				__FUNCTION__,
				hwcpu->hwcard->slot_no,
				hwcpu->hwcard->bus_no, 
				SDLA_GET_CPU(hwcpu->cpu_no));
	}else{
		DEBUG_TEST("%s: HW Card Cpu info: ioport=%d!\n",
				__FUNCTION__,
				hwcpu->hwcard->ioport);
	}
	return 0;
}

static sdlahw_cpu_t* sdla_hwcpu_register(	sdlahw_card_t	*hwcard, 
						int		cpu_no,
						int		irq,
						void		*sdla_dev)
{
	sdlahw_cpu_t	*new_hwcpu, *last_hwcpu;

	new_hwcpu = sdla_hwcpu_search(	(u8)hwcard->hw_type,
					hwcard->slot_no,
					hwcard->bus_no,
					hwcard->ioport,
					cpu_no); 
	if (new_hwcpu){
		DEBUG_TEST("%s: HW Card CPU is already exists!\n",
					__FUNCTION__);
		sdla_hwcpu_info(new_hwcpu);
		return NULL;
	}
	new_hwcpu = wan_malloc(sizeof(sdlahw_cpu_t));
	if (!new_hwcpu)
		return NULL;

	memset(new_hwcpu,0,sizeof(sdlahw_cpu_t));
	new_hwcpu->cpu_no	= cpu_no;
	new_hwcpu->irq		= irq;
	new_hwcpu->hwcard	= hwcard;
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	new_hwcpu->sdla_dev	= sdla_dev;	/* Internal kernel BSD structure */
#endif
	new_hwcpu->magic	= SDLADRV_MAGIC;
	hwcard->internal_used++;

	
	WAN_LIST_FOREACH(last_hwcpu, &sdlahw_cpu_head, next){
		if (!WAN_LIST_NEXT(last_hwcpu, next)){
			break;
		}
	}
	if (last_hwcpu){
		WAN_LIST_INSERT_AFTER(last_hwcpu, new_hwcpu, next);
	}else{
		WAN_LIST_INSERT_HEAD(&sdlahw_cpu_head, new_hwcpu, next);
	}
	return new_hwcpu;
}

static int sdla_hwcpu_unregister(sdlahw_cpu_t *hwcpu)
{

	WAN_ASSERT(hwcpu == NULL);
	if (hwcpu->internal_used){
		DEBUG_EVENT("%s: Error: HW Card Cpu is still in used (used=%d)\n",
					__FUNCTION__,
					hwcpu->internal_used);
		sdla_hwcpu_info(hwcpu);
		return -EBUSY;
	}
	if (hwcpu == WAN_LIST_FIRST(&sdlahw_cpu_head)){
		WAN_LIST_FIRST(&sdlahw_cpu_head) = WAN_LIST_NEXT(hwcpu, next);
	}else{
		WAN_LIST_REMOVE(hwcpu, next);
	}

	hwcpu->hwcard->internal_used--;			/* Decrement card usage */
	hwcpu->hwcard = NULL;
	wan_free(hwcpu);
	return 0;
}

static sdlahw_cpu_t* sdla_hwcpu_search(	unsigned char	hw_type, 
					int		slot_no,
					int		bus_no,
					int		ioport,
					int		cpu_no)
{
	sdlahw_cpu_t*	tmp_hwcpu;
	
	WAN_LIST_FOREACH(tmp_hwcpu, &sdlahw_cpu_head, next){
		WAN_ASSERT_RC(tmp_hwcpu->hwcard == NULL, NULL);
		if (tmp_hwcpu->hwcard->hw_type != hw_type){
			continue;
		}
		switch(hw_type){
		case SDLA_PCI_CARD:
			if (tmp_hwcpu->hwcard->slot_no == slot_no && 
			    tmp_hwcpu->hwcard->bus_no == bus_no && 
			    tmp_hwcpu->cpu_no == cpu_no){ 
				return tmp_hwcpu;
			}
			break;
		case SDLA_ISA_CARD:
			if (tmp_hwcpu->hwcard->ioport == ioport){
				return tmp_hwcpu;
			}
			break;
		}
	}
	return NULL;
}

/*
*****************************************************************************
**	Hardware structure interface per Port
*****************************************************************************
*/
static int sdla_hw_info(sdlahw_t *hw)
{
	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);

	if (hw->hwcpu->hwcard->hw_type == SDLA_PCI_CARD){
		DEBUG_TEST("%s: HW Port info: slot=%d,bus=%d,cpu=%c,port=%d!\n",
				__FUNCTION__,
				hw->hwcpu->hwcard->slot_no,
				hw->hwcpu->hwcard->bus_no, 
				SDLA_GET_CPU(hw->hwcpu->cpu_no),
				hw->port_no);
	}else{
		DEBUG_TEST("%s: HW Port info: ioport=%d,port=%d!\n",
				__FUNCTION__,
				hw->hwcpu->hwcard->ioport,
				hw->port_no);
	}
	return 0;
}

static sdlahw_t* sdla_hw_register(	sdlahw_cpu_t	*hwcpu,
					int		port_no)
{
	sdlahw_t	*new_hw, *last_hw;
	sdla_hw_probe_t	*hwprobe, *tmp_hwprobe;

	new_hw = sdla_hw_search(hwcpu, port_no); 
	if (new_hw){
		DEBUG_TEST(
		"%s: Port is already exists (slot=%d,bus=%d,cpu=%c,port=%d)!\n",
					__FUNCTION__,
					hwcpu->hwcard->slot_no,
					hwcpu->hwcard->bus_no,
					hwcpu->cpu_no,
					port_no);
		sdla_hw_info(new_hw);
		return NULL;
	}
	new_hw = wan_malloc(sizeof(sdlahw_t));
	if (!new_hw)
		return NULL;
	
	hwprobe = wan_malloc(sizeof(sdla_hw_probe_t));
	if (!hwprobe){
		wan_free(new_hw);
		return NULL;
	}
	memset(hwprobe,0,sizeof(sdla_hw_probe_t));
	memset(hwprobe->hw_info, '\0', 100);
	memset(hwprobe->hw_info_verbose, '\0', 500);

	memset(new_hw,0,sizeof(sdlahw_t));
	new_hw->devname	= SDLA_HWPROBE_NAME;
	new_hw->hwcpu	= hwcpu;
	new_hw->magic	= SDLADRV_MAGIC;
	new_hw->port_no	= port_no;

	hwcpu->internal_used++;
	hwcpu->max_ports++;
	hwcpu->hwport[port_no] = new_hw;

	new_hw->hwprobe	= hwprobe;
	hwprobe->internal_used++;

	/* Link new hw port */
	WAN_LIST_FOREACH(last_hw, &sdlahw_head, next){
		if (!WAN_LIST_NEXT(last_hw, next)){
			break;
		}
	}
	if (last_hw){
		WAN_LIST_INSERT_AFTER(last_hw, new_hw, next);
	}else{
		WAN_LIST_INSERT_HEAD(&sdlahw_head, new_hw, next);
	}

	/* Link new hw probe */
	WAN_LIST_FOREACH(tmp_hwprobe, &sdlahw_probe_head, next){
		if (!WAN_LIST_NEXT(tmp_hwprobe, next)){
			break;
		}
	}
	if (tmp_hwprobe){
		WAN_LIST_INSERT_AFTER(tmp_hwprobe, hwprobe, next);
	}else{
		WAN_LIST_INSERT_HEAD(&sdlahw_probe_head, hwprobe, next);
	}

	return new_hw;
}

static int sdla_hw_unregister (sdlahw_t* hw)
{
	sdla_hw_probe_t	*hwprobe;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);

	if (hw->internal_used){
		DEBUG_EVENT(
		"%s: Error: HW Port is still in used (used=%d)\n",
					__FUNCTION__,
					hw->internal_used);
		sdla_hw_info(hw);
		return -EBUSY;
	}

	hwprobe = hw->hwprobe;
	hwprobe->internal_used--;
	hw->hwprobe = NULL;

	hw->hwcpu->hwport[hw->port_no] = NULL;

	hw->hwcpu->internal_used--;			/* Decrement card usage */
	hw->hwcpu = NULL;

	if (hw == WAN_LIST_FIRST(&sdlahw_head)){
		WAN_LIST_FIRST(&sdlahw_head) = WAN_LIST_NEXT(hw, next);
	}else{
		WAN_LIST_REMOVE(hw, next);
	}
	wan_free(hw);

	if (hwprobe->internal_used == 0){
		if (hwprobe == WAN_LIST_FIRST(&sdlahw_probe_head)){
			WAN_LIST_FIRST(&sdlahw_probe_head) = WAN_LIST_NEXT(hwprobe, next);
		}else{
			WAN_LIST_REMOVE(hwprobe, next);
		}
		wan_free(hwprobe);
	}else{
		DEBUG_EVENT("%s: Error: HWPROBE is still in used %d\n",
					__FUNCTION__, hwprobe->internal_used);
	}

	return 0;
}

static sdlahw_t* sdla_hw_search(sdlahw_cpu_t *hwcpu, int port_no)
{
	sdlahw_t*	tmp_hw;
	
	WAN_LIST_FOREACH(tmp_hw, &sdlahw_head, next){
		WAN_ASSERT_RC(tmp_hw->hwcpu == NULL, NULL);
		if (tmp_hw->hwcpu != hwcpu || tmp_hw->port_no != port_no){
			continue;
		}
		return tmp_hw;
	}
	return NULL;
}

/*
*****************************************************************************
*****************************************************************************
***	    S A N G O M A  H A R D W A R E  R E G I S T E R		*****
*****************************************************************************
*****************************************************************************
*/
/*
*****************************************************************************
**			sdla_register
*****************************************************************************
*/
EXPORT_SYMBOL(sdla_register);

void* sdla_register(sdlahw_iface_t* hw_iface, wandev_conf_t* conf, char* devname)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_t	*hw = NULL;

#if defined(__WINDOWS__)
	hw = hw_iface->hw;
#else
	if (sdla_register_check(conf, devname)){
		return NULL;
	}
	hw = sdla_find_adapter(conf, devname);
#endif
	if (hw == NULL || hw->used){
		return NULL;
	}
	WAN_ASSERT_RC(hw->hwcpu == NULL, NULL);
	WAN_ASSERT_RC(hw->hwcpu->hwcard == NULL, NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	hw_iface->setup		= sdla_setup;
	hw_iface->hw_down	= sdla_down;
#if defined(CONFIG_PRODUCT_WANPIPE_GENERIC)
	hw_iface->load		= sdla_load;
#endif
	hw_iface->intack	= sdla_intack;
	hw_iface->read_int_stat	= sdla_read_int_stat;
	hw_iface->mapmem	= sdla_mapmem;
	hw_iface->check_mismatch= sdla_check_mismatch;
	hw_iface->peek		= sdla_peek;
	hw_iface->poke		= sdla_poke;
	hw_iface->poke_byte	= sdla_poke_byte;
	hw_iface->getcfg	= sdla_getcfg;
	hw_iface->setcfg	= sdla_setcfg;
#if defined(WAN_ISA_SUPPORT)	
	hw_iface->isa_read_1	= sdla_isa_read_1;
	hw_iface->isa_write_1	= sdla_isa_write_1;
#endif
	hw_iface->io_read_1	= sdla_io_read_1;
	hw_iface->io_write_1	= sdla_io_write_1;
	hw_iface->bus_read_1	= sdla_bus_read_1;
	hw_iface->bus_read_2	= sdla_bus_read_2;
	hw_iface->bus_read_4	= sdla_bus_read_4;
	hw_iface->bus_write_1	= sdla_bus_write_1;
	hw_iface->bus_write_2	= sdla_bus_write_2;
	hw_iface->bus_write_4	= sdla_bus_write_4;
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	hw_iface->set_intrhand	= sdla_set_intrhand;
	hw_iface->restore_intrhand= sdla_restore_intrhand;
#endif
	hw_iface->is_te1	= sdla_is_te1;
	hw_iface->is_56k	= sdla_is_56k;
	hw_iface->get_hwcard	= sdla_get_hwcard;
	hw_iface->get_hwprobe	= sdla_get_hwprobe;
	hw_iface->hw_lock	= sdla_hw_lock;
	hw_iface->hw_unlock	= sdla_hw_unlock;
	hw_iface->hw_ec_trylock	= sdla_hw_ec_trylock;
	hw_iface->hw_ec_lock	= sdla_hw_ec_lock;
	hw_iface->hw_ec_unlock	= sdla_hw_ec_unlock;
	hw_iface->pci_map_dma	= sdla_pci_map_dma;
	hw_iface->pci_unmap_dma = sdla_pci_unmap_dma;
	hw_iface->hw_same	= sdla_is_same_hwcard;
	hw_iface->hwcpu_same	= sdla_is_same_hwcpu;
	hw_iface->fe_test_and_set_bit = sdla_hw_fe_test_and_set_bit;
	hw_iface->fe_set_bit = sdla_hw_fe_set_bit;
	hw_iface->fe_test_bit = sdla_hw_fe_test_bit;
	hw_iface->fe_clear_bit =  sdla_hw_fe_clear_bit;

	switch(hwcard->cfg_type){
	case WANOPT_S50X:
		hwcard->type		= SDLA_S508;
		hw_iface->cmd		= sdla_cmd;
		hw_iface->set_bit	= sdla_set_bit;
		hw_iface->clear_bit	= sdla_clear_bit;
		DEBUG_EVENT("%s: Found: %s card, IoPort=0x%X, Irq=%d\n",
					devname, 
					SDLA_DECODE_CARDTYPE(hwcard->cfg_type),
					hwcard->ioport,
					hwcpu->irq);
		break;
		
	case WANOPT_S51X:
		hwcard->type			= SDLA_S514;
		hw_iface->load			= sdla_load;	/* For Edukit */
		hw_iface->hw_halt		= sdla_halt;	/* For Edukit */
		hw_iface->start			= sdla_start;	/* For Edukit */
		hw_iface->cmd			= sdla_cmd;
		hw_iface->set_bit		= sdla_set_bit;
		hw_iface->clear_bit		= sdla_clear_bit;
		hw_iface->pci_read_config_byte 	= sdla_pci_read_config_byte;
		hw_iface->pci_read_config_word 	= sdla_pci_read_config_word;
		hw_iface->pci_read_config_dword = sdla_pci_read_config_dword;
		hw_iface->pci_write_config_byte = sdla_pci_write_config_byte;
		hw_iface->pci_write_config_word = sdla_pci_write_config_word;
		hw_iface->pci_write_config_dword = sdla_pci_write_config_dword;
		DEBUG_EVENT("%s: Found: %s card, CPU %c, PciSlot=%d, PciBus=%d\n",
				devname, 
				SDLA_DECODE_CARDTYPE(hwcard->cfg_type),
				SDLA_GET_CPU(hwcpu->cpu_no), 
				hwcard->slot_no, 
				hwcard->bus_no);
		break;
		
	case WANOPT_ADSL:
		hwcard->type			= SDLA_ADSL;
		hw_iface->pci_read_config_byte 	= sdla_pci_read_config_byte;
		hw_iface->pci_read_config_word 	= sdla_pci_read_config_word;
		hw_iface->pci_read_config_dword = sdla_pci_read_config_dword;
		hw_iface->pci_write_config_byte = sdla_pci_write_config_byte;
		hw_iface->pci_write_config_word = sdla_pci_write_config_word;
		hw_iface->pci_write_config_dword = sdla_pci_write_config_dword;
		DEBUG_EVENT("%s: Found: %s card, CPU %c, PciSlot=%d, PciBus=%d\n",
					devname, 
					SDLA_DECODE_CARDTYPE(hwcard->cfg_type),
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hwcard->slot_no, 
					hwcard->bus_no);
		break;
		
	case WANOPT_AFT:
	case WANOPT_AFT101:
	case WANOPT_AFT102:
	case WANOPT_AFT104:
	case WANOPT_AFT108:
	case WANOPT_AFT300:
	case WANOPT_AFT_ANALOG:
	case WANOPT_AFT_ISDN:
	case WANOPT_AFT_56K:
	case WANOPT_AFT_SERIAL:

		hwcard->type			= SDLA_AFT;
		hw_iface->set_bit		= sdla_set_bit;
		hw_iface->clear_bit		= sdla_clear_bit;
		hw_iface->pci_read_config_byte 	= sdla_pci_read_config_byte;
		hw_iface->pci_read_config_word 	= sdla_pci_read_config_word;
		hw_iface->pci_read_config_dword = sdla_pci_read_config_dword;
		hw_iface->pci_write_config_byte = sdla_pci_write_config_byte;
		hw_iface->pci_write_config_word = sdla_pci_write_config_word;
		hw_iface->pci_write_config_dword = sdla_pci_write_config_dword;
		
		hw_iface->pci_bridge_read_config_dword	= sdla_pci_bridge_read_config_dword;
		hw_iface->pci_bridge_read_config_byte	= sdla_pci_bridge_read_config_byte;
		hw_iface->pci_bridge_write_config_dword = sdla_pci_bridge_write_config_dword;
		hw_iface->pci_bridge_write_config_byte = sdla_pci_bridge_write_config_byte;

		hw_iface->read_cpld		= sdla_hw_read_cpld;
		hw_iface->write_cpld		= sdla_hw_write_cpld;

		/* PCI DMA interface access */
		hw_iface->busdma_descr_alloc	= sdla_busdma_descr_alloc;
		hw_iface->busdma_descr_free	= sdla_busdma_descr_free;
		hw_iface->busdma_tag_create	= sdla_busdma_tag_create;
		hw_iface->busdma_tag_destroy	= sdla_busdma_tag_destroy;
		hw_iface->busdma_create		= sdla_busdma_create;
		hw_iface->busdma_destroy	= sdla_busdma_destroy;
		hw_iface->busdma_alloc		= sdla_busdma_alloc;
		hw_iface->busdma_free		= sdla_busdma_free;
		hw_iface->busdma_load		= sdla_busdma_load;
		hw_iface->busdma_unload		= sdla_busdma_unload;
		hw_iface->busdma_map		= sdla_busdma_map;
		hw_iface->busdma_unmap		= sdla_busdma_unmap;
		hw_iface->busdma_sync		= sdla_busdma_sync;
		/* Old PCI DMA interface access */
		hw_iface->pci_map_dma		= sdla_pci_map_dma;
		hw_iface->pci_unmap_dma 	= sdla_pci_unmap_dma;

		hw_iface->get_hwec_index	= sdla_get_hwec_index;

		switch(hwcard->adptr_type){
		case A101_ADPTR_1TE1:
		case A101_ADPTR_2TE1:
			if (hwcard->adptr_subtype == AFT_SUBTYPE_NORMAL){
				hw_iface->fe_read = sdla_te1_read_fe;
				hw_iface->fe_write = sdla_te1_write_fe;
			}else{
				hw_iface->fe_read = sdla_shark_te1_read_fe;
				hw_iface->__fe_read = __sdla_shark_te1_read_fe; 
				hw_iface->fe_write = sdla_shark_te1_write_fe;
			}
			break;
		case A104_ADPTR_4TE1:
		case A108_ADPTR_8TE1:
			hw_iface->fe_read = sdla_shark_te1_read_fe;
			hw_iface->__fe_read = __sdla_shark_te1_read_fe; 
			hw_iface->fe_write = sdla_shark_te1_write_fe;
			break;
		case AFT_ADPTR_56K:
			hw_iface->fe_read = sdla_shark_56k_read_fe;
			hw_iface->__fe_read = __sdla_shark_56k_read_fe;
			hw_iface->fe_write = sdla_shark_56k_write_fe;
			break;
		case A200_ADPTR_ANALOG:
		case A400_ADPTR_ANALOG:
			hw_iface->fe_read = sdla_shark_rm_read_fe;
			hw_iface->__fe_read = __sdla_shark_rm_read_fe;
			hw_iface->fe_write = sdla_shark_rm_write_fe;
			break;
		case A300_ADPTR_U_1TE3:
			hw_iface->fe_read = sdla_te3_read_fe;
			hw_iface->fe_write = sdla_te3_write_fe;
			break;
#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)
		case AFT_ADPTR_ISDN:
			hw_iface->fe_read = sdla_shark_bri_read_fe;
			hw_iface->fe_write = sdla_shark_bri_write_fe;
			break;
#endif
		case AFT_ADPTR_2SERIAL_V35X21:
		case AFT_ADPTR_4SERIAL_V35X21:
		case AFT_ADPTR_2SERIAL_RS232:
		case AFT_ADPTR_4SERIAL_RS232:
			hw_iface->fe_read = sdla_shark_serial_read_fe;
			hw_iface->fe_write = sdla_shark_serial_write_fe;
			break;

		}	
		switch(hwcard->adptr_type){
		case A101_ADPTR_1TE1:
		case A101_ADPTR_2TE1:
		case A104_ADPTR_4TE1:
		case A108_ADPTR_8TE1:
		case A200_ADPTR_ANALOG:
		case A400_ADPTR_ANALOG:
		case AFT_ADPTR_ISDN:
		case AFT_ADPTR_56K:
		case AFT_ADPTR_2SERIAL_V35X21:
		case AFT_ADPTR_4SERIAL_V35X21:
		case AFT_ADPTR_2SERIAL_RS232:
		case AFT_ADPTR_4SERIAL_RS232:
			DEBUG_EVENT("%s: Found: %s card, CPU %c, PciSlot=%d, PciBus=%d, Port=%d\n",
					devname, 
					SDLA_DECODE_CARDTYPE(hwcard->cfg_type),
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hwcard->slot_no, 
					hwcard->bus_no,
					(conf) ? conf->comm_port : hw->used);
			break;
		default:
			DEBUG_EVENT("%s: Found: %s card, CPU %c, PciSlot=%d, PciBus=%d\n",
					devname, 
					SDLA_DECODE_CARDTYPE(hwcard->cfg_type),
					SDLA_GET_CPU(hwcpu->cpu_no), 
					hwcard->slot_no, 
					hwcard->bus_no);
			break;
		}
		break;

	default:
		DEBUG_EVENT("%s:%d: %s: (2) ERROR, invalid card type! 0x%X\n",
					__FUNCTION__,__LINE__,	
					devname, 
					hwcard->cfg_type);
		return NULL;
	}
	

	/* NC: 
	 * Increment the usage count when we know
	 * for sure that the hw has been taken */
	hw->used++;
	hw->hwcpu->used++;
	hw->devname = devname;

	/* ISDN-BRI logial used cnt */
       	if (hwcard->adptr_type == AFT_ADPTR_ISDN){
		int	port;
		/* Add special code for BRI */
		port = hw->port_no / 2;
		port *= 2;
		hw->hwcpu->reg_port[port]++;
		hw->hwcpu->reg_port[port+1]++;
	}else{
		hw->hwcpu->reg_port[0]++;
	}
	/* Set bit for specific port */
	wan_set_bit(hw->port_no, &hw->hwcpu->reg_port_map);
	return hw;
}

/*
*****************************************************************************
**			sdla_unregister
*****************************************************************************
*/
EXPORT_SYMBOL(sdla_unregister);

int sdla_unregister(void** p_hw, char* devname)
{
	sdlahw_t*	hw = *(sdlahw_t**)p_hw;
	sdlahw_cpu_t	*hwcpu;
	
	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;

	/* ISDN-BRI logial used cnt */
       	if (hwcpu->hwcard->adptr_type == AFT_ADPTR_ISDN){
		int	port;
		/* Add special code for BRI */
		port = hw->port_no / 2;
		port *= 2;
		hw->hwcpu->reg_port[port]--;
		hw->hwcpu->reg_port[port+1]--;
	}else{
		hw->hwcpu->reg_port[0]--;
	}
	/* Clear bit for specific port */
	wan_clear_bit(hw->port_no, &hw->hwcpu->reg_port_map);

	hw->used--;
	hw->hwcpu->used--;
	hw->devname = SDLA_HWPROBE_NAME;
	*p_hw = NULL;
	return 0;
}

/******* Kernel APIs ********************************************************/
static int sdla_register_check (wandev_conf_t* conf, char* devname)
{
	if (conf == NULL){
		return 0;
	}
	/* Create sdlahw_t now */
#if defined(__LINUX__)
	if (conf->card_type != WANOPT_S50X){
# ifdef CONFIG_PCI
		if(!pci_present()){
        	        DEBUG_EVENT("%s: PCI BIOS not present!\n", devname);
                	return 0;
	        }
# else
        	DEBUG_EVENT( "%s: Linux not compiled for PCI usage!\n", devname);
	        return 0;
# endif
	}
#endif

	if (conf->card_type==WANOPT_S50X){
		DEBUG_EVENT("%s: Locating: %s card, IoPort=0x%X, Irq=%d\n",
			devname, 
			SDLA_DECODE_CARDTYPE(conf->card_type),
			conf->ioport,
			conf->irq);
	}else if (conf->auto_pci_cfg){
		DEBUG_EVENT("%s: Locating: %s card, CPU %c, PciSlot=Auto, PciBus=Auto\n",
			devname, 
			SDLA_DECODE_CARDTYPE(conf->card_type),
			conf->S514_CPU_no[0]);
	}else{
		DEBUG_EVENT("%s: Locating: %s card, CPU %c, PciSlot=%d, PciBus=%d\n",
				devname, 
				SDLA_DECODE_CARDTYPE(conf->card_type),
				conf->S514_CPU_no[0],
				conf->PCI_slot_no, 
				conf->pci_bus_no);
	}
		
	switch(conf->card_type){

	case WANOPT_S50X: /* Sangoma ISA cards */
		break;

	case WANOPT_S51X:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.s514x_adapters > 1){
 			DEBUG_EVENT( "%s: HW Autodetect failed: Multiple S514X cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
				return -EINVAL;
		}
		break;
		
	case WANOPT_ADSL:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.s518_adapters > 1){
 			DEBUG_EVENT( "%s: HW Autodetect failed: Multiple S518 ADSL cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
				return -EINVAL;
		}
		break;

	case WANOPT_AFT:
	case WANOPT_AFT101:
	case WANOPT_AFT102:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.aft101_adapters > 1){
 			DEBUG_EVENT( "%s: HW Auto PCI failed: Multiple AFT-101/102 cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
			return -EINVAL;
		}
		break;
		
	case WANOPT_AFT104:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.aft104_adapters > 1){
			DEBUG_EVENT( "%s: HW Auto PCI failed: Multiple AFT-104 cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
			return -EINVAL;
		}
		break;
		
	case WANOPT_AFT108:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.aft108_adapters > 1){
			DEBUG_EVENT( "%s: HW Auto PCI failed: Multiple AFT-108 cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
			return -EINVAL;
		}
		break;
		
	case WANOPT_AFT_ANALOG:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.aft200_adapters > 1){
 			DEBUG_EVENT( "%s: HW Auto PCI failed: Multiple AFT-ANALOG cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
			return -EINVAL;
		}
		break;

	case WANOPT_AFT_ISDN:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.aft200_adapters > 1){
 			DEBUG_EVENT( "%s: HW Auto PCI failed: Multiple AFT-ISDN BRI cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
			return -EINVAL;
		}
		break;

	case WANOPT_AFT300:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.aft300_adapters > 1){
			DEBUG_EVENT( "%s: HW Auto PCI failed: Multiple AFT-300 cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
			return -EINVAL;
		}
		break;

		
	case WANOPT_AFT_56K:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.aft_56k_adapters > 1){
 			DEBUG_EVENT( "%s: HW Auto PCI failed: Multiple AFT-56K cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
			return -EINVAL;
		}
		break;

	case WANOPT_AFT_SERIAL:
		if (conf->auto_pci_cfg && sdla_adapter_cnt.aft_serial_adapters > 1){
 			DEBUG_EVENT( "%s: HW Auto PCI failed: Multiple AFT-SERIAL cards found! \n"
				     "%s:    Disable the Autodetect feature and supply\n"
				     "%s:    the PCI Slot and Bus numbers for each card.\n",
               			        devname,devname,devname);
			return -EINVAL;
		}
		break;

	default:
		DEBUG_EVENT("%s: Unsupported Sangoma Card (0x%X) requested by user!\n", 
				devname,conf->card_type);
		return -EINVAL;

	}
	return 0;
}

/*============================================================================
 * Set up adapter.
 * o detect adapter type
 * o verify hardware configuration options
 * o check for hardware conflicts
 * o set up adapter shared memory
 * o test adapter memory
 * o load firmware
 * Return:	0	ok.
 *		< 0	error
 */
/* ALEX int sdla_setup (sdlahw_t* hw, void* sfm, unsigned len)*/
static int sdla_setup (void* phw, wandev_conf_t* conf)
{
	sdlahw_card_t*	hwcard = NULL;
	sdlahw_cpu_t*	hwcpu = NULL;
	sdlahw_t*	hw = (sdlahw_t*)phw;
#if defined(WAN_ISA_SUPPORT)
	unsigned*	irq_opt	= NULL;	/* IRQ options */
	unsigned*	dpmbase_opt	= NULL;	/* DPM window base options */
	unsigned*	pclk_opt	= NULL;	/* CPU clock rate options */
#endif
	int		err=0;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	/* ALEX VVVV*/
	/* for an S514 adapter, pass the CPU number and the slot number read */
	/* from 'router.conf' to the 'sdla_setup()' function via the 'port' */
	/* parameter */
	switch(hwcard->type){
	case SDLA_S514:
		
		if (conf && !conf->S514_CPU_no[0]){
			DEBUG_EVENT("%s: ERROR, invalid S514 CPU [ A|B ]\n",
					hw->devname);
			return -EINVAL;
		}
#if 0				
		if (conf->S514_CPU_no[0] == 'A'){
			hw->cpu_no = SDLA_CPU_A;
		}else if (conf->S514_CPU_no[0] == 'B'){
			hw->cpu_no = SDLA_CPU_B;
		}
		hw->slot_no = conf->PCI_slot_no;
		hw->bus_no  = conf->pci_bus_no;
		hw->auto_pci_cfg = conf->auto_pci_cfg;

		if (hw->auto_pci_cfg == WANOPT_YES){
			DEBUG_EVENT("%s: Setting CPU to %c and Slot to Auto\n",
					hw->devname, 
					SDLA_GET_CPU(hw->cpu_no));
		}else{
			DEBUG_EVENT("%s: Setting CPU to %c and Slot to %i\n",
					hw->devname, 
					SDLA_GET_CPU(hw->cpu_no),
					hwcard->slot_no);
		}
		hw->dpmbase = (sdla_mem_handle_t)conf->maddr;
#endif
		break;

	case SDLA_S508:
		/* 508 Card io port and irq initialization */
#if 0
		hw->port = conf->ioport;
#endif
		hwcpu->irq = (conf && conf->irq == 9) ? 2 : conf->irq;
		if(conf && conf->maddr){
			hwcpu->dpmbase = (sdla_mem_handle_t)phys_to_virt(conf->maddr);
		}
		break;
			
	case SDLA_ADSL:
#if 0
		hw->cpu_no = SDLA_CPU_A;
		hw->slot_no = conf->PCI_slot_no;
		hw->auto_pci_cfg = conf->auto_pci_cfg;
		hw->bus_no  = conf->pci_bus_no;

		if (hw->auto_pci_cfg == WANOPT_YES){
			DEBUG_EVENT("%s: Setting Slot and Bus to Auto\n",
					hw->devname);
		}else{
			DEBUG_EVENT("%s: Setting Slot to %i Bus to %i\n",
					hw->devname, 
					hwcard->slot_no,
					hwcard->bus_no);
		}
		hw->dpmbase = (sdla_mem_handle_t)conf->maddr;
#endif
		hwcpu->fwid = SFID_ADSL; 
		break;
	
	case SDLA_AFT:

		switch(hwcard->adptr_type){
		case A101_ADPTR_1TE1:
		case A101_ADPTR_2TE1:
		case A104_ADPTR_4TE1:
		case A108_ADPTR_8TE1:
		case A200_ADPTR_ANALOG:
		case A400_ADPTR_ANALOG:
		case AFT_ADPTR_ISDN:
		case AFT_ADPTR_56K:
		case AFT_ADPTR_2SERIAL_V35X21:
		case AFT_ADPTR_4SERIAL_V35X21:
		case AFT_ADPTR_2SERIAL_RS232:
		case AFT_ADPTR_4SERIAL_RS232:
			if (hwcpu->used > 1){
				if (conf) conf->irq = hwcpu->irq;
				return 0;
			}
			break;
		}
		
		hwcpu->fwid = SFID_AFT; 
		break;
		
	default:
		DEBUG_EVENT("%s: Invalid card type %x\n",
				hw->devname, hwcard->type);
		return -EINVAL;
	}

	hwcpu->dpmsize = SDLA_WINDOWSIZE;
	hwcpu->pclk = (conf) ? conf->hw_opt[1] : 0;
	
	if (sdla_detect(hw) != 0) {
		return -ENODEV;
	}

	switch(hwcard->type){
	case SDLA_S502A:
        case SDLA_S502E:
        case SDLA_S503:
        case SDLA_S507:
	case SDLA_S508:
                DEBUG_EVENT("%s: found S%04u card at port 0x%X.\n",
                		hw->devname, hwcard->type, hwcard->ioport);

                hwcpu->dpmsize = SDLA_WINDOWSIZE;
		
#if defined(WAN_ISA_SUPPORT)
		switch(hwcard->type){
		case SDLA_S502A:
                        hwcpu->io_range    = S502A_IORANGE;
                        irq_opt         = s502a_irq_options;
                        dpmbase_opt     = s502a_dpmbase_options;
                        pclk_opt        = s502a_pclk_options;
                        break;

                case SDLA_S502E:
                        hwcpu->io_range    = S502E_IORANGE;
                        irq_opt         = s502e_irq_options;
                        dpmbase_opt     = s508_dpmbase_options;
                        pclk_opt        = s502e_pclk_options;
                        break;

                case SDLA_S503:
                        hwcpu->io_range    = S503_IORANGE;
                        irq_opt         = s503_irq_options;
                        dpmbase_opt     = s508_dpmbase_options;
                        pclk_opt        = s503_pclk_options;
                        break;

                case SDLA_S507:
                        hwcpu->io_range    = S507_IORANGE;
                        irq_opt         = s508_irq_options;
                        dpmbase_opt     = s507_dpmbase_options;
                        pclk_opt        = s507_pclk_options;
                        break;

                case SDLA_S508:
                        hwcpu->io_range    = S508_IORANGE;
                        irq_opt         = s508_irq_options;
                        dpmbase_opt     = s508_dpmbase_options;
                        pclk_opt        = s508_pclk_options;
                        break;
		default:
                        DEBUG_EVENT("%s: Unknown card type (%02X)!\n",
                        	hw->devname, hwcard->type);
                     	return -EINVAL;
		}

 		/* Verify IRQ configuration options */
                if (!sdla_get_option_index(irq_opt, hwcpu->irq)) {
                        DEBUG_EVENT("%s: IRQ %d is illegal!\n",
                        	hw->devname, hwcpu->irq);
                      return -EINVAL;
                } 

                /* Verify CPU clock rate configuration options */
                if (hwcpu->pclk == 0)
                        hwcpu->pclk = pclk_opt[1];  /* use default */
        
                else if (!sdla_get_option_index(pclk_opt, hwcpu->pclk)) {
                        DEBUG_EVENT("%s: CPU clock %u is illegal!\n",
				hw->devname, hwcpu->pclk);
                        return -EINVAL;
                } 
                DEBUG_EVENT("%s: assuming CPU clock rate of %u kHz.\n",
			hw->devname, hwcpu->pclk);

                /* Setup adapter dual-port memory window and test memory */
                if (hwcpu->dpmbase == 0) {
                        err = sdla_autodpm(hw);
                        if (err) {
				DEBUG_EVENT("%s: can't find available memory region!\n",
							hw->devname);
                                return err;
                        }
                }
                else if (!sdla_get_option_index(dpmbase_opt,
			virt_to_phys((void*)hwcpu->dpmbase))) {
			DEBUG_EVENT("%s: memory address 0x%lX is illegal!\n",
						hw->devname, 
						(unsigned long)virt_to_phys((void*)hwcpu->dpmbase));
                        return -EINVAL;
                }               
                else if (sdla_setdpm(hw)) {
			DEBUG_EVENT("%s: 8K memory region at 0x%lX is not available!\n",
						hw->devname, 
						(unsigned long)virt_to_phys((void*)hwcpu->dpmbase));
                        return -EINVAL;
                } 
		DEBUG_EVENT("%s: dual-port memory window is set at 0x%lX.\n",
				hw->devname, (unsigned long)virt_to_phys((void*)hwcpu->dpmbase));


		/* If we find memory in 0xE**** Memory region, 
                 * warn the user to disable the SHADOW RAM.  
                 * Since memory corruption can occur if SHADOW is
                 * enabled. This can causes random crashes ! */
		if (virt_to_phys((void*)hwcpu->dpmbase) >= 0xE0000){
			DEBUG_EVENT("\n(WARNING) %s: !!!!!!!!  WARNING !!!!!!!!\n",hw->devname);
			DEBUG_EVENT("(WANRINIG) %s: WANPIPE is using 0x%lX memory region !!!\n",
						hw->devname, 
						(unsigned long)virt_to_phys((void*)hwcpu->dpmbase));
			DEBUG_EVENT("(WARNING)         Please disable the SHADOW RAM, otherwise\n");
			DEBUG_EVENT("(WARNING)         your system might crash randomly from time to time !\n");
			DEBUG_EVENT("(WARNING) %s: !!!!!!!!  WARNING !!!!!!!!\n\n",hw->devname);
		}
#endif
		break;

	case SDLA_S514:

		if (conf) conf->irq = hwcpu->irq;
		if (conf && conf->config_id == WANCONFIG_DEBUG){
			hwcpu->memory = MAX_SIZEOF_S514_MEMORY;
			return 0;
		}
		hwcpu->memory = sdla_test_memregion(hw, MAX_SIZEOF_S514_MEMORY);
		if(hwcpu->memory < (256 * 1024)) {
			DEBUG_EVENT("%s: error in testing S514 memory (0x%lX)\n",
					hw->devname, hwcpu->memory);
			sdla_down(hw);
			return -EINVAL;
		}
		break;

	case SDLA_ADSL:
		if (conf) conf->irq = hwcpu->irq;
		return 0;
		break;

	case SDLA_AFT:
		if (conf) conf->irq = hwcpu->irq;
		return 0;
		break;

	default:
		DEBUG_EVENT("%s: Invalid card type %x\n",
				hw->devname, hwcard->type);
		return -EINVAL;
	}
    
	DEBUG_EVENT("%s: found %luK bytes of on-board memory\n",
		hw->devname, hwcpu->memory / 1024);

	/* Load firmware. If loader fails then shut down adapter */
	if (conf){
		err = sdla_load(hw, conf->data, conf->data_size);
	}
	if (err){ 
		sdla_down(hw);		/* shutdown adapter */
	}

	return err;
} 



/*============================================================================
 * Prepare configuration byte identifying adapter type and CPU clock rate.
 */
static unsigned char sdla_make_config_byte (sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	unsigned char byte = 0;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	switch (hwcpu->pclk) {
	case 5000:  byte = 0x01; break;
	case 7200:  byte = 0x02; break;
	case 8000:  byte = 0x03; break;
	case 10000: byte = 0x04; break;
	case 16000: byte = 0x05; break;
	}

	switch (hwcard->type) {
	case SDLA_S502E: byte |= 0x80; break;
	case SDLA_S503:  byte |= 0x40; break;
	}
	return byte;
}

/*============================================================================
 * Prepare boot-time firmware configuration data.
 * o position DPM window
 * o initialize configuration data area
 */
static int sdla_bootcfg (sdlahw_t* hw, sfm_info_t* sfminfo)
{
	sdlahw_card_t*	card = NULL;
	sdlahw_cpu_t	*hwcpu;
	unsigned int	offset = 0;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
	if (!sfminfo->datasize) return 0;	/* nothing to do */

	if (sdla_mapmem(hw, sfminfo->dataoffs) != 0)
		return -EIO;

	if (card->type == SDLA_S514){
		offset = sfminfo->dataoffs;
	}else{
		offset = sfminfo->dataoffs - (unsigned long)hwcpu->vector;
	}

	sdla_bus_set_region_1(hw, 0x00, 0x00, sfminfo->datasize);
	sdla_bus_write_1(hw, offset, sdla_make_config_byte(hw));

	switch (sfminfo->codeid) {
	case SFID_X25_502:
	case SFID_X25_508:
		sdla_bus_write_1(hw, offset + 0x01, 3);
                sdla_bus_write_1(hw, offset + 0x01, 3);        /* T1 timer */
                sdla_bus_write_1(hw, offset + 0x03, 10);       /* N2 */
                sdla_bus_write_1(hw, offset + 0x06, 7);        /* HDLC window size */
                sdla_bus_write_1(hw, offset + 0x0B, 1);        /* DTE */
                sdla_bus_write_1(hw, offset + 0x0C, 2);        /* X.25 packet window size */
                sdla_bus_write_2(hw, offset + 0x0D, 128);	/* default X.25 data size */
                sdla_bus_write_2(hw, offset + 0x0F, 128);	/* maximum X.25 data size */
		break;
	}
	return 0;
}

/*============================================================================
 * Start adapter's CPU.
 * o calculate a pointer to adapter's cold boot entry point
 * o position DPM window
 * o place boot instruction (jp addr) at cold boot entry point
 * o start CPU
 */
static int sdla_start (sdlahw_t* hw, unsigned addr)
{
	sdlahw_card_t*	hwcard = NULL;
	sdlahw_cpu_t*	hwcpu;
	unsigned int	offset = 0;
	int		err;
#if defined(WAN_ISA_SUPPORT)		
	int		i;
	u8		tmp;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	if (!hwcard->ioport && (hwcard->type != SDLA_S514)) return -EFAULT;

 	switch (hwcard->type) {
#if defined(WAN_ISA_SUPPORT)		
	case SDLA_S502A:
		offset = 0x66;
		break;

	case SDLA_S502E:
	case SDLA_S503:
	case SDLA_S507:
	case SDLA_S508:
#endif	
	case SDLA_S514:
		break;

	default:
		return -EINVAL;
	}

	err = sdla_mapmem(hw, 0);
	if (err) return err;

	sdla_bus_write_1(hw, offset, 0xC3);
	sdla_bus_write_2(hw, offset + 1, (u16)addr);

	switch (hwcard->type) {
#if defined(WAN_ISA_SUPPORT)	
	case SDLA_S502A:
		sdla_isa_write_1(hw, 0x00, 0x10);		/* issue NMI to CPU */
		hwcpu->regs[0] = 0x10;
		break;

	case SDLA_S502E:
		sdla_isa_write_1(hw, 0x03, 0x01);		/* start CPU */
		hwcpu->regs[3] = 0x01;
		for (i = 0; i < SDLA_IODELAY; ++i);
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (tmp & 0x01) {	/* verify */
			/*
			 * Enabling CPU changes functionality of the
			 * control register, so we have to reset its
			 * mirror.
			 */
			sdla_isa_write_1(hw, 0x00, 0);		/* disable interrupts */
			hwcpu->regs[0] = 0;
		}
		else return -EIO;
		break;

	case SDLA_S503:
		tmp = hwcpu->regs[0] | 0x09;	/* set bits 0 and 3 */
		sdla_isa_write_1(hw, 0x00, tmp);
		hwcpu->regs[0] = tmp;		/* update mirror */
		for (i = 0; i < SDLA_IODELAY; ++i);
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (!(tmp & 0x01))	/* verify */
			return -EIO;
		break;

	case SDLA_S507:
		tmp = hwcpu->regs[0] | 0x02;
		sdla_isa_write_1(hw, 0x00, tmp);
		hwcpu->regs[0] = tmp;		/* update mirror */
		for (i = 0; i < SDLA_IODELAY; ++i);
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (!(tmp & 0x04))	/* verify */
			return -EIO;
		break;

	case SDLA_S508:
		tmp = hwcpu->regs[0] | 0x02;
		sdla_isa_write_1(hw, 0x00, tmp);
		hwcpu->regs[0] = tmp;	/* update mirror */
		for (i = 0; i < SDLA_IODELAY; ++i);
		sdla_isa_read_1(hw, 0x01, &tmp);
		if (!(tmp & 0x02))	/* verify */
			return -EIO;
		break;
#endif
	case SDLA_S514:

		sdla_io_write_1(hw, 0x00, S514_CPU_START);
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

/*============================================================================
 * Load adapter from the memory image of the SDLA firmware module. 
 * o verify firmware integrity and compatibility
 * o start adapter up
 */
static int sdla_load (void* phw, void* psfm, unsigned len)
{
	sdlahw_card_t*	hwcard = NULL;
	sdlahw_cpu_t*	hwcpu = NULL;
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sfm_t*		sfm = (sfm_t*)psfm;
	unsigned char	test[256];
	int i;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	/* Verify firmware signature */
	if (strcmp(sfm->signature, SFM_SIGNATURE)) {
		DEBUG_EVENT("%s: not SDLA firmware!\n",
			hw->devname);
		return -EINVAL;
	}

	/* Verify firmware module format version */
	if (sfm->version != SFM_VERSION) {
		DEBUG_EVENT("%s: firmware format %u rejected! Expecting %u.\n",
			hw->devname, sfm->version, SFM_VERSION);
		return -EINVAL;
	}

	/* Verify firmware module length and checksum */
	if ((len - offsetof(sfm_t, image) != sfm->info.codesize) ||
		(sdla_checksum((void*)&sfm->info,
		sizeof(sfm_info_t) + sfm->info.codesize) != sfm->checksum)) {
		DEBUG_EVENT("%s: firmware corrupted!\n", hw->devname);
		return -EINVAL;
	}

	/* Announce */
	DEBUG_EVENT("%s: loading %s (ID=%u)...\n", hw->devname,
		(sfm->descr[0] != '\0') ? sfm->descr : "unknown firmware",
		sfm->info.codeid);

	if (hwcard->type == SDLA_S514){
		DEBUG_EVENT("%s: loading S514 adapter, CPU %c\n",
			hw->devname, SDLA_GET_CPU(hwcpu->cpu_no));
	}

	/* Scan through the list of compatible adapters and make sure our
	 * adapter type is listed.
	 */
	for (i = 0;
	     (i < SFM_MAX_SDLA) && (sfm->info.adapter[i] != hwcard->type);
	     ++i);
	
	if (i == SFM_MAX_SDLA){
		DEBUG_EVENT("%s: firmware is not compatible with S%u!\n",
			hw->devname, hwcard->type);
		return -EINVAL;
	}


	/* Make sure there is enough on-board memory */
	if (hwcpu->memory < sfm->info.memsize){
		DEBUG_EVENT("%s: firmware needs %u bytes of on-board memory!\n",
			hw->devname, sfm->info.memsize);
		return -EINVAL;
	}

	/* Move code onto adapter */
	if (sdla_poke(hw, sfm->info.codeoffs, sfm->image, sfm->info.codesize)) {
		DEBUG_EVENT("%s: failed to load code segment!\n",
			hw->devname);
		return -EIO;
	}
	if (sdla_peek(hw, sfm->info.codeoffs, test, 100)){
		DEBUG_EVENT("%s: Failed to read from card memory!\n",
				hw->devname);
		return -EIO;
	}
	if (strncmp(sfm->image, test, 100) != 0){
		DEBUG_EVENT("%s: failed to test code segment!\n",
			hw->devname);
		return -EIO;
	}

	/* Prepare boot-time configuration data and kick-off CPU */
	sdla_bootcfg(hw, &sfm->info);

	if (sfm->info.codeid != SFID_BSCMP514 && sfm->info.codeid != SFID_POS){
		if (sdla_start(hw, sfm->info.startoffs)) {
			DEBUG_EVENT("%s: Damn... Adapter won't start!\n",
				hw->devname);
			return -EIO;
		}
	}

	/* position DPM window over the mailbox and enable interrupts */
        if (sdla_mapmem(hw, sfm->info.winoffs) || sdla_inten(hw)) {
		DEBUG_EVENT("%s: adapter hardware failure!\n",
			hw->devname);
		return -EIO;
	}

	hwcpu->fwid = sfm->info.codeid;		/* set firmware ID */
	return 0;
}

/*
*****************************************************************************
**			sdla_halt
*****************************************************************************
*/
static int sdla_halt (void* phw)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu = NULL;
	sdlahw_t	*hw = (sdlahw_t*)phw;
#if defined(WAN_ISA_SUPPORT)		
	int		i;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

        if(!hwcard->ioport && (hwcard->type != SDLA_S514))
                return -EFAULT;

	switch (hwcard->type) {
#if defined(WAN_ISA_SUPPORT)		
	case SDLA_S502A:
		sdla_isa_write_1(hw, 0x00, 0x08);	/* halt CPU */
		sdla_isa_write_1(hw, 0x00, 0x08);
		sdla_isa_write_1(hw, 0x00, 0x08);
		hwcpu->regs[0] = 0x08;
		sdla_isa_write_1(hw, 0x01, 0xFF);	/* close memory window */
		hwcpu->regs[1] = 0xFF;
		break;

	case SDLA_S502E:
		sdla_isa_write_1(hw, 0x03, 0);		/* stop CPU */
		sdla_isa_write_1(hw, 0x00, 0);		/* reset board */
		for (i = 0; i < S502E_IORANGE; ++i)
			hwcpu->regs[i] = 0
		;
		break;

	case SDLA_S503:
	case SDLA_S507:
	case SDLA_S508:
		sdla_isa_write_1(hw, 0x00, 0);		/* reset board logic */
		hwcpu->regs[0] = 0;
		break;
#endif
	case SDLA_S514:
		/* halt the adapter */
		sdla_io_write_1(hw, 0x00, S514_CPU_HALT);
		/* Test and clear PCI memory */
		sdla_test_memregion(hw, MAX_SIZEOF_S514_MEMORY);
		break;
	}
	return 0;
}


/*============================================================================
 * Shut down SDLA: disable shared memory access and interrupts, stop CPU, etc.
 */
static int sdla_down (void* phw)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu = NULL;
	sdlahw_t	*hw = (sdlahw_t*)phw;
        unsigned int	CPU_no;
        u32		int_config, int_status;
#if defined(WAN_ISA_SUPPORT)	
	int 		i = 0;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	switch (hwcard->type) {
#if defined(WAN_ISA_SUPPORT)	
	case SDLA_S502A:

		if (!hwcard->ioport){
			return -EFAULT;
		}

		sdla_isa_write_1(hw, 0x00, 0x08);	/* halt CPU */
		sdla_isa_write_1(hw, 0x00, 0x08);
		sdla_isa_write_1(hw, 0x00, 0x08);
		hwcpu->regs[0] = 0x08;
		sdla_isa_write_1(hw, 0x01, 0xFF);	/* close memory window */
		hwcpu->regs[1] = 0xFF;
		break;

	case SDLA_S502E:

	 	if (!hwcard->ioport){
                        return -EFAULT;
                }

		sdla_isa_write_1(hw, 0x03, 0);		/* stop CPU */
		sdla_isa_write_1(hw, 0x00, 0);		/* reset board */
		for (i = 0; i < S502E_IORANGE; ++i)
			hwcpu->regs[i] = 0
		;
		break;

	case SDLA_S503:
	case SDLA_S507:
	case SDLA_S508:

	 	if (!hwcard->ioport){
                        return -EFAULT;
                }

		sdla_isa_write_1(hw, 0x00, 0);			/* reset board logic */
		hwcpu->regs[0] = 0;
		break;
#endif
	case SDLA_S514:
		/* halt the adapter */
		sdla_io_write_1(hw, 0x00, S514_CPU_HALT);
        	CPU_no = hwcpu->cpu_no;
	
		/* disable the PCI IRQ and disable memory access */
                sdla_pci_read_config_dword(hw, PCI_INT_CONFIG, &int_config);
	        int_config &= (hwcpu->cpu_no == SDLA_CPU_A) ? ~PCI_DISABLE_IRQ_CPU_A : ~PCI_DISABLE_IRQ_CPU_B;
                sdla_pci_write_config_dword(hw, PCI_INT_CONFIG, int_config);

		sdla_read_int_stat(hw, &int_status);
		sdla_intack(hw, int_status);
		
		if (hwcpu->cpu_no == SDLA_CPU_A){
                        sdla_pci_write_config_dword(hw, PCI_MAP0_DWORD, PCI_CPU_A_MEM_DISABLE);
		}else{
                        sdla_pci_write_config_dword(hw, PCI_MAP1_DWORD, PCI_CPU_B_MEM_DISABLE);
		}

		/* free up the allocated virtual memory */
		if (hwcpu->status & SDLA_MEM_MAPPED){
			sdla_bus_space_unmap(hw, hwcpu->dpmbase, MAX_SIZEOF_S514_MEMORY);
			hwcpu->status &= ~SDLA_MEM_MAPPED;
		}

		if (hwcpu->status & SDLA_IO_MAPPED){
			sdla_bus_space_unmap(hw, hwcpu->vector, 16);
			hwcpu->status &= ~SDLA_IO_MAPPED;
		}

 		break;

/* ALEX*/
	case SDLA_ADSL:

		sdla_memory_unmap(hw);
#if 0
		if (hw->status & SDLA_MEM_RESERVED){
			sdla_release_mem_region(hw, hw->mem_base_addr, GSI_PCI_MEMORY_SIZE);
			hw->status &= ~SDLA_MEM_RESERVED;
		}

		/* free up the allocated virtual memory */
		if (hw->status & SDLA_MEM_MAPPED){
			sdla_bus_space_unmap(hw, hw->dpmbase, GSI_PCI_MEMORY_SIZE);
			hw->status &= ~SDLA_MEM_MAPPED;
		}
#endif
		break;

	case SDLA_AFT:

		switch(hwcard->adptr_type){
		case A101_ADPTR_1TE1:
		case A101_ADPTR_2TE1:
		case A104_ADPTR_4TE1:
		case A108_ADPTR_8TE1:
		case A200_ADPTR_ANALOG:
		case A400_ADPTR_ANALOG:
		case AFT_ADPTR_ISDN:
		case AFT_ADPTR_56K:
		case AFT_ADPTR_2SERIAL_V35X21:
		case AFT_ADPTR_4SERIAL_V35X21:
		case AFT_ADPTR_2SERIAL_RS232:
		case AFT_ADPTR_4SERIAL_RS232:
			if (hwcpu->used > 1){
				break;
			}
			/* fall throught */
		default:
			sdla_memory_unmap(hw);
			break;
		}
                break;

	default:
		return -EINVAL;
	}
	return 0;
}

/*============================================================================
 * Enable interrupt generation.
 */
static int sdla_inten (sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu = NULL;
#if defined(WAN_ISA_SUPPORT)		
	int 		i;
	u8		tmp;
#endif

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	switch (hwcard->type) {
#if defined(WAN_ISA_SUPPORT)		
	case SDLA_S502E:
		/* Note thar interrupt control operations on S502E are allowed
		 * only if CPU is enabled (bit 0 of status register is set).
		 */
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (tmp & 0x01) {
			sdla_isa_write_1(hw, 0x00, 0x02);	/* bit1 = 1, bit2 = 0 */
			sdla_isa_write_1(hw, 0x00, 0x06);	/* bit1 = 1, bit2 = 1 */
			hwcpu->regs[0] = 0x06;
		}
		else return -EIO;
		break;

	case SDLA_S503:
		tmp = hwcpu->regs[0] | 0x04;
		sdla_isa_write_1(hw, 0x00, tmp);
		hwcpu->regs[0] = tmp;		/* update mirror */
		for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (!(tmp & 0x02))		/* verify */
			return -EIO;
		break;

	case SDLA_S508:
		tmp = hwcpu->regs[0] | 0x10;
		sdla_isa_write_1(hw, 0x00, tmp);
		hwcpu->regs[0] = tmp;		/* update mirror */
		for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
		sdla_isa_read_1(hw, 0x01, &tmp);
		if (!(tmp & 0x10))		/* verify */
			return -EIO;
		break;

	case SDLA_S502A:
	case SDLA_S507:
		break;
#endif
        case SDLA_S514:
        case SDLA_ADSL:
        case SDLA_AFT:
                break;

	default:
		return -EINVAL;

	}
	return 0;
}

/*============================================================================
 * Disable interrupt generation.
 */
#if 0
static int sdla_intde (sdlahw_t* hw)
{
	sdlahw_card_t*	card = NULL;
	int		i;
	u8		tmp;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcard == NULL);
	card = hw->hwcard;
	switch (card->type){
	case SDLA_S502E:
		/* Notes:
		 *  1) interrupt control operations are allowed only if CPU is
		 *     enabled (bit 0 of status register is set).
		 *  2) disabling interrupts using bit 1 of control register
		 *     causes IRQ line go high, therefore we are going to use
		 *     0x04 instead: lower it to inhibit interrupts to PC.
		 */
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (tmp & 0x01) {
			sdla_isa_write_1(hw, 0x00, hw->regs[0] & ~0x04);
			hw->regs[0] &= ~0x04;
		}
		else return -EIO;
		break;

	case SDLA_S503:
		tmp = hw->regs[0] & ~0x04;
		sdla_isa_write_1(hw, 0x00, tmp);
		hw->regs[0] = tmp;			/* update mirror */
		for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (tmp & 0x02)			/* verify */
			return -EIO;
		break;

	case SDLA_S508:
		tmp = hw->regs[0] & ~0x10;
		sdla_isa_write_1(hw, 0x00, tmp);
		hw->regs[0] = tmp;			/* update mirror */
		for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (tmp & 0x10)			/* verify */
			return -EIO;
		break;

	case SDLA_S502A:
	case SDLA_S507:
		break;

	default:
		return -EINVAL;
	}
	return 0;
}
#endif
/*============================================================================
 * Read the hardware interrupt status.
 */
static int sdla_read_int_stat (void* phw, u32* int_status)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu = NULL;
	sdlahw_t	*hw = (sdlahw_t*)phw;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	switch(hwcard->type){
	case SDLA_S514:
	case SDLA_ADSL:
	case SDLA_AFT:
		sdla_pci_read_config_dword(hw, PCI_INT_STATUS, int_status);
	}
	return 0;
}

/*============================================================================
 * Acknowledge SDLA hardware interrupt.
 */
static int sdla_intack (void* phw, u32 int_status)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu = NULL;
	sdlahw_t	*hw = (sdlahw_t*)phw;
#if defined(WAN_ISA_SUPPORT)		
	u8		tmp;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	switch (hwcard->type){
#if defined(WAN_ISA_SUPPORT)		
	case SDLA_S502E:
		/* To acknoledge hardware interrupt we have to toggle bit 3 of
		 * control register: \_/
		 * Note that interrupt control operations on S502E are allowed
		 * only if CPU is enabled (bit 1 of status register is set).
		 */
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (tmp & 0x01) {
			tmp = hwcpu->regs[0] & ~0x04;
			sdla_isa_write_1(hw, 0x00, tmp);
			tmp |= 0x04;
			sdla_isa_write_1(hw, 0x00, tmp);
			hwcpu->regs[0] = tmp;
		}
		else return -EIO;
		break;

	case SDLA_S503:
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (tmp & 0x04) {
			tmp = hwcpu->regs[0] & ~0x08;
			sdla_isa_write_1(hw, 0x00, tmp);
			tmp |= 0x08;
			sdla_isa_write_1(hw, 0x00, tmp);
			hwcpu->regs[0] = tmp;
		}
		break;

	case SDLA_S502A:
	case SDLA_S507:
	case SDLA_S508:
		break;
#endif
	case SDLA_S514:
	case SDLA_ADSL:
	case SDLA_AFT:
		sdla_pci_write_config_dword(hw, PCI_INT_STATUS, int_status);
		break;
			
	default:
		return -EINVAL;
	}
	return 0;
}

/*============================================================================
 * Generate an interrupt to adapter's CPU.
 */
#if 0
static int sdla_intr (sdlahw_t* hw)
{
	sdlahw_card_t*	card = NULL;
	u8		tmp;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcard == NULL);
	card = hw->hwcard;
	switch (card->type) {
	case SDLA_S502A:
		sdla_isa_read_1(hw, 0x00, &tmp);
		if (!(tmp & 0x40)) {
			sdla_isa_write_1(hw, 0x00, 0x10);		/* issue NMI to CPU */
			hw->regs[0] = 0x10;
		}
		else return -EIO;
		break;

	case SDLA_S507:
		sdla_isa_read_1(hw, 0x00, &tmp);
		if ((tmp & 0x06) == 0x06) {
			sdla_isa_write_1(hw, 0x03, 0);
		}
		else return -EIO;
		break;

	case SDLA_S508:
		sdla_isa_read_1(hw, 0x01, &tmp);
		if (tmp & 0x02) {
			sdla_isa_write_1(hw, 0x00, 0x08);
		}
		else return -EIO;
		break;

	case SDLA_S502E:
	case SDLA_S503:
	default:
		return -EINVAL;
	}
	return 0;
}
#endif

static int sdla_cmd (void* phw, unsigned long offset, wan_mbox_t* mbox)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t*	hwcpu = NULL;
	int		len = sizeof(wan_cmd_t);
	int		err = 0;
	u8		value;
	
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	len += mbox->wan_data_len;
		
	DEBUG_CMD("%s: Executing %02X command...\n", 
				hw->devname,mbox->wan_command);
	/* Sangoma ISA card -> sdla_bus_read_1(hw, offset, &value); */
	sdla_peek(hw, offset, (void*)&value, 1);
	if (value != 0x00){ 
		DEBUG_EVENT("%s: opp flag set on entry to sdla_exec!\n",
				hw->devname);
		return 0;
	}
	mbox->wan_opp_flag = 0x00;
	sdla_poke(hw, offset, (void*)mbox, len);
	
	err = sdla_exec(hw, offset);
	if (!err){
		DEBUG_EVENT("%s: Command 0x%02X failed!\n",
					hw->devname, mbox->wan_command);
		return WAN_CMD_TIMEOUT;
	}
	sdla_peek(hw, offset, (void*)mbox, sizeof(wan_cmd_t));
	if (mbox->wan_data_len < (sizeof(wan_mbox_t) - sizeof(wan_cmd_t))){
		sdla_peek(hw, offset+offsetof(wan_mbox_t, wan_data), 
			  mbox->wan_data, mbox->wan_data_len);
	}else{
		DEBUG_EVENT("%s: Error: sdla_cmd() data_len=%d\n",
				hw->devname,mbox->wan_data_len);
		return WAN_CMD_TIMEOUT;
	}
	
	DEBUG_CMD("%s: Executing %02X command...done!\n", 
				hw->devname,mbox->wan_command);
	return mbox->wan_return_code;	
}

/*============================================================================
 * Execute Adapter Command.
 * o Set exec flag.
 * o Busy-wait until flag is reset.
 * o Return number of loops made, or 0 if command timed out.
 */
static int sdla_exec (sdlahw_t* hw, unsigned long offset)
{
	volatile wan_ticks_t	tstop = 0;
	volatile int		nloops = 0;
	u8			value, buf[10];
	
#if 0
	/* Sangoma ISA card -> sdla_bus_read_1(hw, offset, &value); */
	sdla_peek(hw, offset, (void*)&value, 1);
	if (value != 0x00){ 
		DEBUG_EVENT("%s: opp flag set on entry to sdla_exec!\n",
						hw->devname);
		return 0;
	}
#endif	
	/* Sangoma ISA card -> sdla_bus_write_1(hw, offset, 0x01); */
	value = 0x01;
	sdla_poke(hw, offset, (void*)&value, 1);
	tstop = SYSTEM_TICKS + EXEC_TIMEOUT;

	/* Sangoma ISA card -> sdla_bus_read_1(hw, offset, &value); */
	sdla_peek(hw, offset, (void*)&value, 1); 
	for (nloops = 1; value == 0x01; ++ nloops){ 
		WP_DELAY(EXEC_DELAY);
		if (SYSTEM_TICKS > tstop || nloops > MAX_NLOOPS){ 
			DEBUG_EVENT(
			"%s: Timeout %ld (%lu>%lu) Ticks (max=%lu) Loops %u (max=%u)\n",
					__FUNCTION__,
				(unsigned long)(SYSTEM_TICKS-tstop+EXEC_TIMEOUT),
				(unsigned long)SYSTEM_TICKS,
				(unsigned long)tstop,
				(unsigned long)EXEC_TIMEOUT,
				nloops,
				MAX_NLOOPS);

			/* NC: May 26 2004
			 * Reset the opp flag, in order to attempt
			 * to recover from this critical error.
			 * Otherwise, every subsequen call will be
			 * blocked with opp flag set condition */
			value = 0x00;
			sdla_poke(hw, offset, (void*)&value, 1);

			return 0;		/* time is up! */ 
		}

		/* Sangoma ISA card -> sdla_bus_read_1(hw, offset, &value); */
		buf[0] = 0;
		sdla_peek(hw, offset, (void*)&buf[0], 1); 
		value = buf[0];
	}

	return nloops;
}
	

/*============================================================================
 * Read absolute adapter memory.
 * Transfer data from adapter's memory to data buffer.
 *
 * Note:
 * Care should be taken when crossing dual-port memory window boundary.
 * This function is not atomic, so caller must disable interrupt if
 * interrupt routines are accessing adapter shared memory.
 */
static int sdla_peek (void* phw, unsigned long addr, void* pbuf, unsigned len)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu = NULL;
	sdlahw_t*	hw = (sdlahw_t*)phw;
	unsigned char*  buf = (unsigned char*)pbuf;
	int err = 0;
	

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if (addr + len > hwcpu->memory)	/* verify arguments */
		return -EINVAL;

	switch(hwcard->type){

	case SDLA_S508:
		{
		unsigned long oldvec = (unsigned long)hwcpu->vector;
		unsigned long curvec;      /* current DPM window vector */
		unsigned winsize = hwcpu->dpmsize;
		unsigned curpos, curlen;   /* current offset and block size */

		while (len && !err) {
			curpos = addr % winsize;  /* current window offset */
			curvec = addr - curpos;   /* current window vector */
			curlen = (len > (winsize - curpos)) ?
						(winsize - curpos) : len;
                        /* Relocate window and copy block of data */
			err = sdla_mapmem(hw, curvec);
			sdla_peek_by_4 (hw, curpos, buf, curlen);
			addr       += curlen;
			buf 	   += curlen;
			len        -= curlen;
		}

		/* Restore DPM window position */
		sdla_mapmem(hw, oldvec);
		break;
		}
	
	case SDLA_S514:
	case SDLA_ADSL:
	case SDLA_AFT:
		sdla_peek_by_4(hw, addr, buf, len);
		break;

	default:
		DEBUG_EVENT("%s: Invalid card type 0x%X\n",
			__FUNCTION__,hwcard->type);
		err = -EINVAL;
		break;
        }
	return err;
}


/*============================================================================
 * Read data from adapter's memory to a data buffer in 4-byte chunks.
 * Note that we ensure that the SDLA memory address is on a 4-byte boundary
 * before we begin moving the data in 4-byte chunks.
*/
static void sdla_peek_by_4 (sdlahw_t* hw, unsigned long offset, void* pbuf, unsigned int len)
{
	unsigned char *buf=(unsigned char*)pbuf;
	u32* u32_buf;
	
	/* byte copy data until we get to a 4-byte boundary */
	while (len && (offset & 0x03)){ 
		sdla_bus_read_1(hw, offset++, buf);
		buf ++;
		len --;
	}

	/* copy data in 4-byte chunks */
	while (len >= 4){
		u32_buf=(u32*)buf;
		sdla_bus_read_4(hw, offset, u32_buf);
		buf += 4;
		offset += 4;
		len -= 4;
	}

	/* byte copy any remaining data */
	while (len){ 
		sdla_bus_read_1(hw, offset++, buf);
		buf ++;
		len --;
	}
}

/*============================================================================
 * Write Absolute Adapter Memory.
 * Transfer data from data buffer to adapter's memory.
 *
 * Note:
 * Care should be taken when crossing dual-port memory window boundary.
 * This function is not atomic, so caller must disable interrupt if
 * interrupt routines are accessing adapter shared memory.
 */
static int sdla_poke (void* phw, unsigned long addr, void* pbuf, unsigned len)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu = NULL;
	sdlahw_t*	hw = (sdlahw_t*)phw;
	unsigned char 	*buf = pbuf;
       	int err = 0;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if (addr + len > hwcpu->memory){	/* verify arguments */
		return -EINVAL;
	}
   
	switch (hwcard->type){

	case SDLA_S508:
		{
		unsigned long oldvec = (unsigned long)hwcpu->vector;
		unsigned long curvec;        /* current DPM window vector */
		unsigned winsize = hwcpu->dpmsize;
		unsigned curpos, curlen;     /* current offset and block size */

		while (len && !err) {
			curpos = addr % winsize;    /* current window offset */
			curvec = addr - curpos;     /* current window vector */
			curlen = (len > (winsize - curpos)) ?
				(winsize - curpos) : len;
			/* Relocate window and copy block of data */
			sdla_mapmem(hw, curvec);
			sdla_poke_by_4 (hw, curpos, buf, curlen);
			addr       += curlen;
			buf 	   += curlen;
			len        -= curlen;
		}

		/* Restore DPM window position */
		sdla_mapmem(hw, oldvec);
		break;
		}
	case SDLA_S514:
	case SDLA_ADSL:
	case SDLA_AFT:
		sdla_poke_by_4(hw, addr, buf, len);
		break;

	default:
		DEBUG_EVENT("%s: Invalid card type 0x%X\n",
			__FUNCTION__,hwcard->type);
		err = -EINVAL;
		break;
	}
	return err;
}


/*============================================================================
 * Write from a data buffer to adapter's memory in 4-byte chunks.
 * Note that we ensure that the SDLA memory address is on a 4-byte boundary
 * before we begin moving the data in 4-byte chunks.
*/
static void sdla_poke_by_4 (sdlahw_t* hw, unsigned long offset, void* pbuf, unsigned int len)
{
	u32* u32_buf;
	unsigned char *buf=(unsigned char*)pbuf;

	/* byte copy data until we get to a 4-byte boundary */
	while (len && (offset & 0x03)){ 
		sdla_bus_write_1(hw, offset++, *buf);
		buf ++;
		len --;
	}

	/* copy data in 4-byte chunks */
	while (len >= 4){ 
		u32_buf=(u32*)buf;
		sdla_bus_write_4(hw, offset, *u32_buf);
		offset += 4;
		buf += 4;
		len -= 4;
	}

	/* byte copy any remaining data */
	while (len){ 
		sdla_bus_write_1(hw, offset++, *buf);
		buf ++;
		len --;
	}
}

static int sdla_poke_byte (void* phw, unsigned long offset, u8 value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;

	SDLA_MAGIC(hw);
	/* Sangoma ISA card sdla_bus_write_1(hw, offset, value); */
	sdla_poke(hw, offset, (void*)&value, 1);
	return 0;
}

static int sdla_set_bit (void* phw, unsigned long offset, u8 value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	u8		tmp;

	SDLA_MAGIC(hw);
	/* Sangoma ISA card -> sdla_bus_read_1(hw, offset, &tmp); */
	sdla_peek(hw, offset, (void*)&tmp, 1);
	tmp |= value;
	/* Sangoma ISA card -> sdla_bus_write_1(hw, offset, tmp); */
	sdla_poke(hw, offset, (void*)&tmp, 1);
	return 0;
}


static int sdla_clear_bit (void* phw, unsigned long offset, u8 value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	u8		tmp;

	SDLA_MAGIC(hw);
	/* Sangoma ISA card -> sdla_bus_read_1(hw, offset, &tmp); */
	sdla_peek(hw, offset, (void*)&tmp, 1);
	tmp &= ~value;
	/* Sangoma ISA card -> sdla_bus_write_1(hw, offset, tmp); */
	sdla_poke(hw, offset, (void*)&tmp, 1);
	return 0;
}


/****** Hardware-Specific Functions *****************************************/

/*
*****************************************************************************
*****************************************************************************
***	    S A N G O M A  H A R D W A R E  D E T E C T I O N 		*****
*****************************************************************************
*****************************************************************************
*/
static int sdla_init_pci_slot(sdlahw_t *hw)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	u32 int_status;
	int volatile found=0;
	int i=0;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	/* Check if this is a very first load for a specific
         * pci card. If it is, clear the interrput bits, and
         * set the flag indicating that this card was initialized.
	 */
	
	for (i=0; (i<MAX_S514_CARDS) && !found; i++){
		if (pci_slot_ar[i] == hwcard->slot_no){
			found=1;
			break;
		}
		if (pci_slot_ar[i] == 0xFF){
			break;
		}
	}

	if (!found){
		sdla_read_int_stat(hw,&int_status);
		sdla_intack(hw,int_status);
		if (i == MAX_S514_CARDS){
			DEBUG_EVENT( "%s: Critical Error !!!\n",hw->devname);
			DEBUG_EVENT( 
				"%s: Number of Sangoma PCI cards exceeded maximum limit.\n",
					hw->devname);
			DEBUG_EVENT( "Please contact Sangoma Technologies\n");
			return 1;
		}
		pci_slot_ar[i] = hwcard->slot_no;
	}
	return 0;
}

#if defined(WAN_ISA_SUPPORT)
/* 
 * ============================================================================
 * Check memory region to see if it's available. 
 * Return:	0	ok.
 */
static unsigned int sdla_check_memregion(sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	unsigned int	offset = 0x0;
	unsigned int	len    = 0;
	u8		value;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	len = hwcpu->dpmsize;
	sdla_bus_read_1(hw, offset, &value); 
	for(; len && (value == 0xFF); --len){ 
		/* attempt to write 0 */
		sdla_bus_write_1(hw, offset, 0);
		sdla_bus_read_1(hw, offset, &value);
		if (value != 0xFF){ 	/*  still has to read 0xFF */
			/* restore orig. value */
			sdla_bus_write_1(hw, offset, 0xFF);
			break;			/* not good */
		}
		++offset;
		sdla_bus_read_1(hw, offset, &value); 
	}
	return len;

}
#endif

#if defined(WAN_ISA_SUPPORT)
/*============================================================================
 * Autoselect memory region. 
 * o try all available DMP address options from the top down until success.
 */
static int sdla_autodpm (sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;
	int i, err = -EINVAL;
	unsigned* opt;
 
	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	switch (hwcard->type){
	case SDLA_S502A:
		opt = s502a_dpmbase_options;
		break;

	case SDLA_S502E:
	case SDLA_S503:
	case SDLA_S508:
		opt = s508_dpmbase_options;
		break;

	case SDLA_S507:
		opt = s507_dpmbase_options;
		break;
	default:
		return -EINVAL;
	}

	/* Start testing from 8th position, address
         * 0xC8000 from the 508 address table. 
         * We don't want to test A**** addresses, since
         * they are usually used for Video */
	for (i = 8; i <= opt[0] && err; i++) {
		hwcpu->dpmbase = (sdla_mem_handle_t)phys_to_virt(opt[i]);
		err = sdla_setdpm(hw);
	}
	return err;
}
#endif

#if defined(WAN_ISA_SUPPORT)
/*============================================================================
 * Initialize SDLA hardware: setup memory window, IRQ, etc.
 */
static int sdla_init (sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	int i;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	for (i = 0; i < SDLA_MAXIORANGE; ++i){
		hwcpu->regs[i] = 0;
	}

	switch (hwcard->type) {
	case SDLA_S502A: return sdla_init_s502a(hw);
	case SDLA_S502E: return sdla_init_s502e(hw);
	case SDLA_S503:  return sdla_init_s503(hw);
	case SDLA_S507:  return sdla_init_s507(hw);
	case SDLA_S508:  return sdla_init_s508(hw);
	}
	return -EINVAL;
}
#endif

/*============================================================================
 * Map shared memory window into SDLA address space.
 */
static int sdla_mapmem (void* phw, unsigned long addr)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t*	hwcpu = NULL;
	sdlahw_t*	hw = (sdlahw_t*)phw;
#if defined(WAN_ISA_SUPPORT)		
	u8		tmp;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	switch (hwcard->type){
#if defined(WAN_ISA_SUPPORT)		
	case SDLA_S502A:
	case SDLA_S502E:
		if (addr < S502_MAXMEM)	{ /* verify parameter */
			tmp = addr >> 13;	/* convert to register mask */
			sdla_isa_write_1(hw, 0x02, tmp);
			hwcpu->regs[2] = tmp;
		}
		else return -EINVAL;
		break;

	case SDLA_S503:
		if (addr < S503_MAXMEM)	{ /* verify parameter */
			tmp = (hwcpu->regs[0] & 0x8F) | ((addr >> 9) & 0x70);
			sdla_isa_write_1(hw, 0x00, tmp);
			hwcpu->regs[0] = tmp;
		}
		else return -EINVAL;
		break;

	case SDLA_S507:
		if (addr < S507_MAXMEM) {
			sdla_isa_read_1(hw, 0x00, &tmp);
			if (!(tmp & 0x02))
				return -EIO;
			tmp = addr >> 13;	/* convert to register mask */
			sdla_isa_write_1(hw, 0x02, tmp);
			hwcpu->regs[2] = tmp;
		}
		else return -EINVAL;
		break;

	case SDLA_S508:
		if (addr < S508_MAXMEM) {
			tmp = addr >> 13;	/* convert to register mask */
			sdla_isa_write_1(hw, 0x02, tmp);
			hwcpu->regs[2] = tmp;
		}
		else return -EINVAL;
		break;
#endif
	case SDLA_S514:
	case SDLA_ADSL:
	case SDLA_AFT:
		return 0;

 	default:
		return -EINVAL;
	}
	hwcpu->vector = (sdla_mem_handle_t)(addr & 0xFFFFE000L);
	return 0;
}

/*============================================================================
 * Test memory region.
 * Return:	size of the region that passed the test.
 * Note:	Region size must be multiple of 2 !
 */
static unsigned sdla_test_memregion (sdlahw_t* hw, unsigned len)
{
	unsigned	len_w = len >> 1;	/* region len in words */
	unsigned	offset, i;		/* offset in char */
	u16		value;

    	for (i = 0, offset = 0; i < len_w; ++i, offset += 2){
		sdla_bus_write_2(hw, offset, 0xAA55);
    	}
        
	for (i = 0, offset = 0; i < len_w; ++i, offset += 2){
		sdla_bus_read_2(hw, offset, &value);
		if (value != 0xAA55){
	    		len_w = i;
            		break;
        	}
    	}

    	for (i = 0, offset = 0; i < len_w; ++i, offset += 2){
		sdla_bus_write_2(hw, offset, 0x55AA);
    	}
        
	for (i = 0, offset = 0; i < len_w; ++i, offset += 2){
		sdla_bus_read_2(hw, offset, &value);
		if (value != 0x55AA){ 
	    		len_w = i;
            		break;
        	}
    	}
        
    	for (i = 0, offset = 0; i < len_w; ++i, offset += 2){
		sdla_bus_write_2(hw, offset, 0x0);
    	}

	return len_w << 1;
#if 0
	volatile unsigned long w_ptr;
	unsigned len_w = len >> 1;	/* region len in words */
	unsigned i;

        for (i = 0, w_ptr = hw->dpmbase; i < len_w; ++i, ++w_ptr)
                writew (0xAA55, w_ptr);
        
	for (i = 0, w_ptr = hw->dpmbase; i < len_w; ++i, ++w_ptr)
                if (readw (w_ptr) != 0xAA55) {
                        len_w = i;
                        break;
                }

        for (i = 0, w_ptr = hw->dpmbase; i < len_w; ++i, ++w_ptr)
                writew (0x55AA, w_ptr);
        
        for (i = 0, w_ptr = hw->dpmbase; i < len_w; ++i, ++w_ptr)
                if (readw(w_ptr) != 0x55AA) {
                        len_w = i;
                        break;
                }
        
        for (i = 0, w_ptr = hw->dpmbase; i < len_w; ++i, ++w_ptr)
		writew (0, w_ptr);

        return len_w << 1;
#endif
}

#if defined(WAN_ISA_SUPPORT)
/*============================================================================
 * Test adapter on-board memory.
 * o slide DPM window from the bottom up and test adapter memory segment by
 *   segment.
 * Return adapter memory size.
 */
static unsigned long sdla_memtest (sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	unsigned long memsize;
	unsigned winsize;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	for (memsize = 0, winsize = hwcpu->dpmsize;
	     !sdla_mapmem(hw, memsize) &&
	     (sdla_test_memregion(hw, winsize) == winsize)
	     ;
	     memsize += winsize)
	;
	hwcpu->memory = memsize;
	return memsize;
}
#endif

#if defined(WAN_ISA_SUPPORT)
/*============================================================================
 * Set up adapter dual-port memory window. 
 * o shut down adapter
 * o make sure that no physical memory exists in this region, i.e entire
 *   region reads 0xFF and is not writable when adapter is shut down.
 * o initialize adapter hardware
 * o make sure that region is usable with SDLA card, i.e. we can write to it
 *   when adapter is configured.
 */
static int sdla_setdpm (sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	int err;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	/* Shut down card and verify memory region */
	sdla_down(hw);
	if (sdla_check_memregion(hw))
		return -EINVAL;

	/* Initialize adapter and test on-board memory segment by segment.
	 * If memory size appears to be less than shared memory window size,
	 * assume that memory region is unusable.
	 */
	err = sdla_init(hw);
	if (err) return err;

	if (sdla_memtest(hw) < hwcpu->dpmsize) {/* less than window size */
		sdla_down(hw);
		return -EIO;
	}

	sdla_mapmem(hw, 0L);	/* set window vector at bottom */
	return 0;
}
#endif

/*============================================================================
 * Detect S502A adapter.
 *	Following tests are used to detect S502A adapter:
 *	1. All registers other than status (BASE) should read 0xFF
 *	2. After writing 00001000b to control register, status register should
 *	   read 01000000b.
 *	3. After writing 0 to control register, status register should still
 *	   read  01000000b.
 *	4. After writing 00000100b to control register, status register should
 *	   read 01000100b.
 *	Return 1 if detected o.k. or 0 if failed.
 *	Note:	This test is destructive! Adapter will be left in shutdown
 *		state after the test.
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_detect_s502a (sdlahw_t* hw)
{
	sdlahw_card_t*	hwcard;
	sdlahw_cpu_t*	hwcpu;
	int		i, j;
	u8		tmp;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	if (!sdla_get_option_index(s502_port_options, hwcard->ioport))
		return 0;
	
	for (j = 1; j < SDLA_MAXIORANGE; ++j) {
		sdla_isa_read_1(hw, j, &tmp);
		if (tmp != 0xFF)
			return 0;
		for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	}

	sdla_isa_write_1(hw, 0x00, 0x08);			/* halt CPU */
	sdla_isa_write_1(hw, 0x00, 0x08);			/* halt CPU */
	sdla_isa_write_1(hw, 0x00, 0x08);			/* halt CPU */
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	if (tmp != 0x40)
		return 0;
	sdla_isa_write_1(hw, 0x00, 0x00);
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	if (tmp != 0x40)
		return 0;
	sdla_isa_write_1(hw, 0x00, 0x04);
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	if (tmp != 0x44)
		return 0;

	/* Reset adapter */
	sdla_isa_write_1(hw, 0x00, 0x08);			/* halt CPU */
	sdla_isa_write_1(hw, 0x00, 0x08);			/* halt CPU */
	sdla_isa_write_1(hw, 0x00, 0x08);			/* halt CPU */
	sdla_isa_write_1(hw, 0x01, 0xFF);
	return 1;
}
#endif


/*============================================================================
 * Initialize S502A adapter.
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_init_s502a (sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	int tmp, i;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	if (!sdla_detect_s502a(hw))
		return -ENODEV;

	hwcpu->regs[0] = 0x08;
	hwcpu->regs[1] = 0xFF;

	/* Verify configuration options */
	i = sdla_get_option_index(s502a_dpmbase_options, virt_to_phys((void*)hwcpu->dpmbase));
	if (i == 0)
		return -EINVAL;

	tmp = s502a_hmcr[i - 1];
	switch (hwcpu->dpmsize) {
	case 0x2000:
		tmp |= 0x01;
		break;

	case 0x10000L:
		break;

	default:
		return -EINVAL;
	}

	/* Setup dual-port memory window (this also enables memory access) */
	sdla_isa_write_1(hw, 0x01, tmp);
	hwcpu->regs[1] = tmp;
	hwcpu->regs[0] = 0x08;
	return 0;
}
#endif
/*============================================================================
 * Detect S502E adapter.
 *	Following tests are used to verify adapter presence:
 *	1. All registers other than status (BASE) should read 0xFF.
 *	2. After writing 0 to CPU control register (BASE+3), status register
 *	   (BASE) should read 11111000b.
 *	3. After writing 00000100b to port BASE (set bit 2), status register
 *	   (BASE) should read 11111100b.
 *	Return 1 if detected o.k. or 0 if failed.
 *	Note:	This test is destructive! Adapter will be left in shutdown
 *		state after the test.
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_detect_s502e (sdlahw_t* hw)
{
	sdlahw_card_t*	card;
	sdlahw_cpu_t*	hwcpu;
	int		i, j;
	u8		tmp;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
	if (!sdla_get_option_index(s502_port_options, card->ioport))
		return 0;
	for (j = 1; j < SDLA_MAXIORANGE; ++j) {
		sdla_isa_read_1(hw, j, &tmp);
		if (tmp != 0xFF)
			return 0;
		for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	}

	sdla_isa_write_1(hw, 0x03, 0x00);	/* CPU control reg. */
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);	/* read status */
	if (tmp != 0xF8)			/* read status */
		return 0;
	sdla_isa_write_1(hw, 0x00, 0x04);	/* set bit 2 */
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);	/* verify */
	if (tmp != 0xFC)			/* verify */
		return 0;

	/* Reset adapter */
	sdla_isa_write_1(hw, 0x00, 0x00);
	return 1;
}
#endif

/*============================================================================
 * Initialize S502E adapter.
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_init_s502e (sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	int	i;
	u8	tmp;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	if (!sdla_detect_s502e(hw))
		return -ENODEV;

	/* Verify configuration options */
	i = sdla_get_option_index(s508_dpmbase_options, virt_to_phys((void*)hwcpu->dpmbase));
	if (i == 0)
		return -EINVAL;

	tmp = s502e_hmcr[i - 1];
	switch (hwcpu->dpmsize) {
	case 0x2000:
		tmp |= 0x01;
		break;

	case 0x10000L:
		break;

	default:
		return -EINVAL;
	}

	/* Setup dual-port memory window */
	sdla_isa_write_1(hw, 0x01, tmp);
	hwcpu->regs[1] = tmp;

	/* Enable memory access */
	sdla_isa_write_1(hw, 0x00, 0x02);
	hwcpu->regs[0] = 0x02;
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	return (tmp & 0x02) ? 0 : -EIO;
}
#endif

/*============================================================================
 * Detect s503 adapter.
 *	Following tests are used to verify adapter presence:
 *	1. All registers other than status (BASE) should read 0xFF.
 *	2. After writing 0 to control register (BASE), status register (BASE)
 *	   should read 11110000b.
 *	3. After writing 00000100b (set bit 2) to control register (BASE),
 *	   status register should read 11110010b.
 *	Return 1 if detected o.k. or 0 if failed.
 *	Note:	This test is destructive! Adapter will be left in shutdown
 *		state after the test.
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_detect_s503 (sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;
	int		i, j;
	u8		tmp;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if (!sdla_get_option_index(s503_port_options, hwcard->ioport))
		return 0;
	for (j = 1; j < SDLA_MAXIORANGE; ++j) {
		sdla_isa_read_1(hw, j, &tmp);
		if (tmp != 0xFF)
			return 0;
		for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	}

	sdla_isa_write_1(hw, 0x00, 0x00);	/* reset control reg.*/
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	if (tmp != 0xF0)			/* read status */
		return 0;
	sdla_isa_write_1(hw, 0x00, 0x04);	/* set bit 2 */
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	if (tmp != 0xF2)			/* verify */
		return 0;

	/* Reset adapter */
	sdla_isa_write_1(hw, 0x00, 0x00);
	return 1;
}
#endif

/*============================================================================
 * Initialize S503 adapter.
 * ---------------------------------------------------------------------------
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_init_s503 (sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	int tmp, i;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	if (!sdla_detect_s503(hw))
		return -ENODEV;

	/* Verify configuration options */
	i = sdla_get_option_index(s508_dpmbase_options, virt_to_phys((void*)hwcpu->dpmbase));
	if (i == 0)
		return -EINVAL;

	tmp = s502e_hmcr[i - 1];
	switch (hwcpu->dpmsize) {
	case 0x2000:
		tmp |= 0x01;
		break;

	case 0x10000L:
		break;

	default:
		return -EINVAL;
	}

	/* Setup dual-port memory window */
	sdla_isa_write_1(hw, 0x01, tmp);
	hwcpu->regs[1] = tmp;

	/* Enable memory access */
	sdla_isa_write_1(hw, 0x00, 0x02);
	hwcpu->regs[0] = 0x02;	/* update mirror */
	return 0;
}
#endif

/*============================================================================
 * Detect s507 adapter.
 *	Following tests are used to detect s507 adapter:
 *	1. All ports should read the same value.
 *	2. After writing 0x00 to control register, status register should read
 *	   ?011000?b.
 *	3. After writing 0x01 to control register, status register should read
 *	   ?011001?b.
 *	Return 1 if detected o.k. or 0 if failed.
 *	Note:	This test is destructive! Adapter will be left in shutdown
 *		state after the test.
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_detect_s507 (sdlahw_t* hw)
{
	sdlahw_card_t*	hwcard;
	sdlahw_cpu_t	*hwcpu;
	int		i, j;
	u8		tmp, tmp1;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if (!sdla_get_option_index(s508_port_options, hwcard->ioport))
		return 0;
	sdla_isa_read_1(hw, 0x00, &tmp);
	for (j = 1; j < S507_IORANGE; ++j) {
		sdla_isa_read_1(hw, j, &tmp1);
		if (tmp1 != tmp)
			return 0;
		for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	}

	sdla_isa_write_1(hw, 0x00, 0x00);
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	if ((tmp & 0x7E) != 0x30)
		return 0;
	sdla_isa_write_1(hw, 0x00, 0x01);
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	if ((tmp & 0x7E) != 0x32)
		return 0;

	/* Reset adapter */
	sdla_isa_write_1(hw, 0x00, 0x00);
	return 1;
}
#endif

/*============================================================================
 * Initialize S507 adapter.
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_init_s507 (sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	int	i;
	u8	tmp;


	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	if (!sdla_detect_s507(hw))
		return -ENODEV;

	/* Verify configuration options */
	i = sdla_get_option_index(s507_dpmbase_options, virt_to_phys((void*)hwcpu->dpmbase));
	if (i == 0)
		return -EINVAL;

	tmp = s507_hmcr[i - 1];
	switch (hwcpu->dpmsize) {
	case 0x2000:
		tmp |= 0x01;
		break;

	case 0x10000L:
		break;

	default:
		return -EINVAL;
	}

	/* Enable adapter's logic */
	sdla_isa_write_1(hw, 0x00, 0x01);
	hwcpu->regs[0] = 0x01;
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	if (!(tmp & 0x20))
		return -EIO;

	/* Setup dual-port memory window */
	sdla_isa_write_1(hw, 0x01, tmp);
	hwcpu->regs[1] = tmp;

	/* Enable memory access */
	tmp = hwcpu->regs[0] | 0x04;
	if (hwcpu->irq) {
		i = sdla_get_option_index(s508_irq_options, hwcpu->irq);
		if (i) tmp |= s507_irqmask[i - 1];
	}
	sdla_isa_write_1(hw, 0x00, tmp);
	hwcpu->regs[0] = tmp;		/* update mirror */
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x00, &tmp);
	return (tmp & 0x08) ? 0 : -EIO;
}
#endif

/*
 * ============================================================================
 * Initialize S508 adapter.
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_init_s508 (sdlahw_t* hw)
{
	sdlahw_cpu_t	*hwcpu;
	int		i;
	u8		tmp;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	if (sdla_detect_s508(hw)){
		return -ENODEV;
	}

	/* Verify configuration options */
	i = sdla_get_option_index(s508_dpmbase_options, virt_to_phys((void*)hwcpu->dpmbase));
	if (i == 0){
		return -EINVAL;
	}

	/* Setup memory configuration */
	tmp = s508_hmcr[i - 1];
	sdla_isa_write_1(hw, 0x01, tmp);
	hwcpu->regs[1] = tmp;

	/* Enable memory access */
	sdla_isa_write_1(hw, 0x00, 0x04);
	hwcpu->regs[0] = 0x04;		/* update mirror */
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x01, &tmp);
	return (tmp & 0x04) ? 0 : -EIO;
}
#endif

/* 
 * ============================================================================
 * Detect s508 adapter.
 *	Following tests are used to detect s508 adapter:
 *	1. After writing 0x00 to control register, status register should read
 *	   ??000000b.
 *	2. After writing 0x10 to control register, status register should read
 *	   ??010000b
 *	Return 1 if detected o.k. or 0 if failed.
 *	Note:	This test is destructive! Adapter will be left in shutdown
 *		state after the test.
 */
#if defined(WAN_ISA_SUPPORT)	
static int sdla_detect_s508 (sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;
	int		i;
	u8		tmp;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	/* Sangoma ISa card */
	hwcpu->status |= SDLA_MEM_MAPPED;
	hwcpu->status |= SDLA_IO_MAPPED;
	if (!sdla_get_option_index(s508_port_options, hwcard->ioport)){
		return -EINVAL;
	}
	sdla_isa_write_1(hw, 0x0, 0x00);
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x1, &tmp);
	if ((tmp & 0x3F) != 0x00){
		return -EINVAL;
	}
	sdla_isa_write_1(hw, 0x0, 0x10);
	for (i = 0; i < SDLA_IODELAY; ++i);	/* delay */
	sdla_isa_read_1(hw, 0x01, &tmp);
	if ((tmp & 0x3F) != 0x10){
		return -EINVAL;
	}
	/* Reset adapter */
	sdla_isa_write_1(hw, 0x0, 0x00);
	return 0;
}
#endif

/*============================================================================
 * Detect s514 PCI adapter.
 *      Return 1 if detected o.k. or 0 if failed.
 *      Note:   This test is destructive! Adapter will be left in shutdown
 *              state after the test.
 */
static int sdla_detect_s514 (sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;
	u32		ut_u32;
	u8		ut_u8;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	sdla_pci_read_config_dword(hw,
		   (hwcpu->cpu_no == SDLA_CPU_A) ? PCI_MEM_BASE0_DWORD : 
			    	PCI_MEM_BASE1_DWORD, (u32*)&hwcpu->mem_base_addr);
	if (!hwcpu->mem_base_addr){
		if(hwcpu->cpu_no == SDLA_CPU_B){
			DEBUG_EVENT( "%s: CPU #B not present on the card\n",
					hw->devname);
		}else{
			DEBUG_EVENT( "%s: No PCI memory allocated to card\n",
					hw->devname);
		}
		return -EINVAL;
	}
	DEBUG_EVENT( "%s: S514 PCI memory at 0x%lX\n",
				hw->devname, (unsigned long)hwcpu->mem_base_addr);

	/* enable the PCI memory */
	sdla_pci_read_config_dword(hw, 
			(hwcpu->cpu_no == SDLA_CPU_A) ? PCI_MAP0_DWORD : PCI_MAP1_DWORD, &ut_u32);
	sdla_pci_write_config_dword(hw, 
			(hwcpu->cpu_no == SDLA_CPU_A) ? PCI_MAP0_DWORD : PCI_MAP1_DWORD,
			(ut_u32 | PCI_MEMORY_ENABLE));

	/* check the IRQ allocated and enable IRQ usage */
	/* the INTPIN must not be 0 - if it is, then the S514 adapter is not */
	/* configured for IRQ usage */
	
	sdla_pci_read_config_byte(hw, PCI_INT_PIN_BYTE, &ut_u8);
        if(!ut_u8) {
                DEBUG_EVENT("%s: invalid setting for INTPIN on S514 card\n",
			       hw->devname);
                DEBUG_EVENT("Please contact your Sangoma representative\n");
                return 0;
        }

	sdla_pci_read_config_byte(hw, PCI_INT_LINE_BYTE, (u8*)&hwcpu->irq);
        if(hwcpu->irq == PCI_IRQ_NOT_ALLOCATED) {
                DEBUG_EVENT("%s: IRQ not allocated to S514 adapter\n",
			hw->devname);
                return -EINVAL;
        }
#if defined(__LINUX__)
	hwcpu->irq = hwcard->pci_dev->irq;
#endif

	/* BUG FIX : Mar 6 2000
 	 * On a initial loading of the card, we must check
         * and clear PCI interrupt bits, due to a reset
         * problem on some other boards.  i.e. An interrupt
         * might be pending, even after system bootup, 
         * in which case, when starting wanrouter the machine
         * would crash. 
	 */
	if (sdla_init_pci_slot(hw)){
		return 0;
	}

	sdla_pci_read_config_dword(hw, PCI_INT_CONFIG, &ut_u32);
	ut_u32 |= (hwcpu->cpu_no == SDLA_CPU_A) ?
		PCI_ENABLE_IRQ_CPU_A : PCI_ENABLE_IRQ_CPU_B;
        sdla_pci_write_config_dword(hw, PCI_INT_CONFIG, ut_u32);

	if (sdla_pci_enable_device(hw)){
		DEBUG_EVENT( "%s: Error: S514 PCI enable failed\n",
			hw->devname);
		return -EINVAL;
    	}

	sdla_pci_set_master(hw);

	/* Linux uses new irq routing mechanism, where
	 * the value of the irq is not valid until pci_enable_device()
	 * is executied */
#if defined(__LINUX__)
	hwcpu->irq = hwcard->pci_dev->irq;
#endif

	DEBUG_EVENT( "%s: IRQ %d allocated to the S514 card\n",
		hw->devname, hwcpu->irq);
	
	hwcpu->status |= SDLA_PCI_ENABLE;

	/* map the physical PCI memory to virtual memory */
	sdla_bus_space_map(hw, 0x0, MAX_SIZEOF_S514_MEMORY, &hwcpu->dpmbase);
	if (!hwcpu->dpmbase){ 
		DEBUG_EVENT("%s: PCI virtual memory allocation failed\n", 
					hw->devname);
		return -EINVAL;
	}

	/* map the physical control register memory to virtual memory */
	sdla_bus_space_map(hw, S514_CTRL_REG_BYTE, 16, &hwcpu->vector);
	if (!hwcpu->vector){ 
		sdla_bus_space_unmap(hw, hwcpu->dpmbase, MAX_SIZEOF_S514_MEMORY);
		DEBUG_EVENT("%s: PCI virtual memory allocation failed\n", 
					hw->devname);
		hwcpu->dpmbase=0;
		return -EINVAL;
	}

	hwcpu->status |= SDLA_MEM_MAPPED;

		
#if defined(__NetBSD__) || defined(__OpenBSD__)
	hwcpu->ioh = hw->vector;
#endif
	hwcpu->status |= SDLA_IO_MAPPED;

	/* halt the adapter */
	sdla_io_write_1(hw, 0x0, S514_CPU_HALT);

	return 0;
}



/*============================================================================
 * Find the S514 PCI adapter in the PCI bus.
 *      Return the number of S514 adapters found (0 if no adapter found).
 */
static int sdla_detect_pulsar(sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;
	int		err;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if ((err = sdla_memory_map(hw, SDLA_CPU_A))){
		return err;
	}
#if 0
	sdla_pci_read_config_dword(hw, PCI_IO_BASE_DWORD, (u32*)&hw->mem_base_addr);
	if(!hw->mem_base_addr) {
		DEBUG_EVENT( "%s: No PCI memory allocated to card!\n",
					hw->devname);
		return -EINVAL;
	}
#endif
	DEBUG_EVENT( "%s: ADSL PCI memory at 0x%lX\n",
				hw->devname, (unsigned long)hwcpu->mem_base_addr);

	sdla_pci_read_config_byte(hw, PCI_INT_LINE_BYTE, (u8*)&hwcpu->irq);
        if(hwcpu->irq == PCI_IRQ_NOT_ALLOCATED) {
                DEBUG_EVENT( "%s: IRQ not allocated to S514 adapter\n",
			hw->devname);
                return -EINVAL;
        }

#if defined(__LINUX__)
	hwcpu->irq = hwcard->pci_dev->irq;
#endif
	DEBUG_EVENT( "%s: IRQ %d allocated to the ADSL card\n",
				hw->devname, hwcpu->irq);

#if 0
	if (sdla_pci_enable_device(hw)){
		DEBUG_EVENT( "%s: Error: S518 ADSL PCI enable failed\n",
			hw->devname);
		return -EINVAL;
    	}
	hw->status |= SDLA_PCI_ENABLE;

	sdla_pci_set_master(hw);

	hw->status |= SDLA_MEM_MAPPED;

	if (sdla_request_mem_region(hw, hw->mem_base_addr, 
				    GSI_PCI_MEMORY_SIZE, "WANPIPE ADSL", &presource)){ 
   
		DEBUG_EVENT("%s: Error: Unable to reserve S518 ADSL pci bar0 memory\n",
				hw->devname);
		return -EINVAL;
    	}
	
	hw->status |= SDLA_MEM_RESERVED;
	
	hw->memory=GSI_PCI_MEMORY_SIZE;
	
	/* map the physical PCI memory to virtual memory */
	sdla_bus_space_map(hw, 0x0, GSI_PCI_MEMORY_SIZE, &hw->dpmbase);
        if(!hw->dpmbase) {
		DEBUG_EVENT( "%s: Error: S518 ADSL PCI virtual memory ioremap failed\n",
			hw->devname);
                return -EINVAL;
	}
#endif
	return 0;

}

/*============================================================================
 * Map memory for AFT HDLC PCI adapter in the PCI bus.
 */
static int sdla_memory_map(sdlahw_t* hw, int cpu_no)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;
	void		*presource;
	int		err=-EINVAL;
	unsigned char   reserve_name[50];

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;
	presource =NULL;

	switch (hwcard->type){ 
#if defined(WAN_ISA_SUPPORT)	
	case SDLA_S508:
		err=0;
		break;
#endif

	case SDLA_ADSL:
		hwcpu->memory=GSI_PCI_MEMORY_SIZE;
		cpu_no=SDLA_CPU_A;
		sprintf(reserve_name,"WANPIPE ADSL");
		break;

	case SDLA_AFT:
		sprintf(reserve_name,"WANPIPE AFT");
		switch(hwcard->adptr_type){
		case A101_ADPTR_1TE1:
		case A101_ADPTR_2TE1:
			if (hwcard->adptr_subtype == AFT_SUBTYPE_NORMAL){
				hwcpu->memory = AFT_PCI_MEM_SIZE; 
			}else{
				hwcpu->memory = AFT2_PCI_MEM_SIZE; 
			}
			break;
		case A104_ADPTR_4TE1:
		case A200_ADPTR_ANALOG:
		case A400_ADPTR_ANALOG:
		case AFT_ADPTR_ISDN:
			hwcpu->memory = AFT4_PCI_MEM_SIZE; 
			break;
		case AFT_ADPTR_2SERIAL_V35X21:
		case AFT_ADPTR_4SERIAL_V35X21:
		case AFT_ADPTR_2SERIAL_RS232:
		case AFT_ADPTR_4SERIAL_RS232:
		case A108_ADPTR_8TE1:
			hwcpu->memory = AFT8_PCI_MEM_SIZE; 
			break;
		case AFT_ADPTR_56K:
			hwcpu->memory = AFT2_PCI_MEM_SIZE; 
			break;
		default:
			hwcpu->memory = AFT_PCI_MEM_SIZE; 
			break;
		}
		break;
	
	default:
		DEBUG_EVENT("%s:%d Error: Invalid hw adapter type (0x%X)\n",
			__FUNCTION__,__LINE__,hwcard->type);
		return -EINVAL;
	}

	DEBUG_TEST("NCDEBUG: MEMORY MAPPING AFT\n");

	if (sdla_pci_enable_device(hw)){
		DEBUG_EVENT( "%s: Error: AFT PCI enable failed\n",
			hw->devname);
		return -EINVAL;
	}
	hwcpu->status |= SDLA_PCI_ENABLE;
	
	if (!(hwcpu->status & SDLA_MEM_RESERVED)){
#if defined(__LINUX__)
		err = pci_request_region(hwcard->pci_dev, (cpu_no == SDLA_CPU_A)?0:1 ,reserve_name);
#else
		err = sdla_request_mem_region(
				hw, 
				hwcpu->mem_base_addr, 
				hwcpu->memory,
				reserve_name,
				&presource);
#endif
		if (err){

			DEBUG_EVENT("%s: Error: Unable to reserve AFT pci bar%i memory\n",
				hw->devname,(cpu_no == SDLA_CPU_A)?0:1);

			err = -EINVAL;
			goto aft_pci_error;
		}
		hwcpu->status |= SDLA_MEM_RESERVED;
	}

#if defined(__LINUX__) && defined(DMA_32BIT_MASK)
	if((err = pci_set_dma_mask(hwcard->pci_dev, DMA_32BIT_MASK))) {
		DEBUG_EVENT("%s: Error: No usable DMA configuration, aborting.\n",
				hw->devname);
		err = -EINVAL;
		goto aft_pci_error;
	}
#endif
	sdla_get_pci_base_resource_addr(hw,
				(cpu_no == SDLA_CPU_A) ? 
					PCI_IO_BASE_DWORD : 
					PCI_MEM_BASE0_DWORD,
				&hwcpu->mem_base_addr);

	if (!hwcpu->mem_base_addr){
		if(cpu_no == SDLA_CPU_B){
			DEBUG_EVENT( "%s: Error: CPU #B not present on adapter\n",
				hw->devname);
		}else{
			DEBUG_EVENT( "%s: Error: Failed to allocate PCI memory on AFT/ADSL card\n",
				hw->devname);
		}
		err = -EINVAL;
		goto aft_pci_error;
	}

	if (!(hwcpu->status & SDLA_MEM_MAPPED)){
		/* map the physical PCI memory to virtual memory */
		sdla_bus_space_map(hw, 
				0x0, 
				hwcpu->memory,
				&hwcpu->dpmbase);
		if (!hwcpu->dpmbase){
			DEBUG_EVENT( "%s: Error: AFT PCI virtual memory ioremap failed\n",
				hw->devname);
			err = -EINVAL;
			goto aft_pci_error;
		}	
		hwcpu->status |= SDLA_MEM_MAPPED;
	}
	
	sdla_pci_set_master(hw);
	err=0;

	return err;


aft_pci_error:
	sdla_memory_unmap(hw);
	return err;
}

/*============================================================================
 * Map memory for AFT HDLC PCI adapter in the PCI bus.
 */
static int sdla_memory_unmap(sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

#if defined(__WINDOWS__)
	return 0;
#endif

	switch (hwcard->type){ 
#if defined(WAN_ISA_SUPPORT)	
	case SDLA_S508:
		break;
#endif

	case SDLA_ADSL:
	case SDLA_AFT:

		DEBUG_TEST("NCDEBUG: MEMORY UNMAPPING AFT\n");

		/* free up the allocated virtual memory */
		if (hwcpu->status & SDLA_MEM_MAPPED){
			sdla_bus_space_unmap(
					hw, 
					hwcpu->dpmbase, 
					hwcpu->memory);
			hwcpu->status &= ~SDLA_MEM_MAPPED;
		}

		if (hwcpu->status & SDLA_MEM_RESERVED){
#if defined(__LINUX__)
			pci_release_region(hwcard->pci_dev,(hwcpu->cpu_no == SDLA_CPU_A)?0:1);
#else
			sdla_release_mem_region(
					hw,
					hwcpu->mem_base_addr,
					hwcpu->memory);
#endif
			hwcpu->status &= ~SDLA_MEM_RESERVED;
		}

		if (hwcpu->status & SDLA_PCI_ENABLE){
			/* FIXME: No allowed to do this because bar1 might 
                         *        still be used.  Need a pci dev counter */
			/* sdla_pci_disable_device(hw); */
			hwcpu->status &= ~SDLA_PCI_ENABLE;
		}
		break;
	}
	return 0;
}
/*============================================================================
 * Find the AFT HDLC PCI adapter in the PCI bus.
 * Return the number of AFT adapters found (0 if no adapter found).
 */
static int sdla_detect_aft(sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;
	int		err;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if ((err = sdla_memory_map(hw, hwcpu->cpu_no))){
		return err;
	}

	DEBUG_EVENT( "%s: AFT PCI memory at 0x%lX\n",
				hw->devname, (unsigned long)hwcpu->mem_base_addr);

#if defined(__LINUX__)
	hwcpu->irq = hwcard->pci_dev->irq;
#else
	sdla_pci_read_config_byte(hw, PCI_INT_LINE_BYTE, (u8*)&hwcpu->irq);
#endif
        if(hwcpu->irq == PCI_IRQ_NOT_ALLOCATED) {
		sdla_memory_unmap(hw);
                DEBUG_EVENT( "%s: IRQ not allocated to AFT adapter\n",
			hw->devname);
                return -EINVAL;
        }

	DEBUG_EVENT( "%s: IRQ %d allocated to the AFT PCI card\n",
				hw->devname, hwcpu->irq);

	/* Set PCI Latency of 0xFF*/
	sdla_pci_write_config_dword(hw, XILINX_PCI_LATENCY_REG, XILINX_PCI_LATENCY);
	return 0;
}



/*============================================================================
** Compare current hw adapter for auto pci configuration.
**/
static int sdla_cmp_adapter_auto(sdlahw_t	 *hw, wandev_conf_t* conf)
{
	sdlahw_cpu_t	*hwcpu = NULL;
	int		cpu_no = SDLA_CPU_A;

	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	switch(conf->card_type){
	case WANOPT_S51X:
	case WANOPT_ADSL:
	case WANOPT_AFT:
	case WANOPT_AFT300:
	case WANOPT_AFT_ANALOG:
	case WANOPT_AFT_ISDN:
	case WANOPT_AFT_56K:
	case WANOPT_AFT101:
	case WANOPT_AFT102:
	case WANOPT_AFT104:
	case WANOPT_AFT108:

		if (hwcpu->cpu_no == cpu_no &&
		    conf->card_type == WANOPT_AFT &&
		    hwcpu->hwcard->cfg_type == WANOPT_AFT101) {
			/* Remap the card type to standard
			   A104 Shark style.  We are allowing
			   and old config file for A101/2-SH */
			conf->config_id = WANCONFIG_AFT_TE1;
			conf->card_type = WANOPT_AFT104;
			conf->fe_cfg.line_no=1;
			return 0;
		}

		/* Allow old A102 config for A102 SHARK */
		if (hwcpu->cpu_no == cpu_no &&
		    conf->card_type == WANOPT_AFT &&
		    hwcpu->hwcard->cfg_type == WANOPT_AFT102) {
			/* Remap the card type to standard
			   A104 Shark style.  We are allowing
			   and old config file for A101/2-SH */
			conf->config_id = WANCONFIG_AFT_TE1;
			conf->card_type = WANOPT_AFT104;
			if (cpu_no == SDLA_CPU_A) {
				conf->fe_cfg.line_no=1;
			} else {
				conf->fe_cfg.line_no=2;		
			}
			return 0;
		}

		if (conf->config_id == WANCONFIG_AFT_TE1){
			if (hwcpu->cpu_no == cpu_no &&
			    hw->port_no == conf->fe_cfg.line_no-1 &&
       	          (hwcpu->hwcard->cfg_type == WANOPT_AFT101 ||
			    hwcpu->hwcard->cfg_type == WANOPT_AFT102 ||
			    hwcpu->hwcard->cfg_type == WANOPT_AFT104 ||
			    hwcpu->hwcard->cfg_type == WANOPT_AFT108)) {
				return 0;
			}
		}else{
			if (hwcpu->cpu_no == cpu_no &&
			    hwcpu->hwcard->cfg_type == conf->card_type){
				return 0;
			}
		}
		break;
	}
	return 1;
}

/*============================================================================
 * Find the S514 PCI adapter in the PCI bus.
 * 	If conf argument is NULL, return fisrt not used adapter (CONFIG_PRODUCT_WANPIPE_GENERIC)
 *      Return the number of S514 adapters found (0 if no adapter found).
 */
static sdlahw_t* sdla_find_adapter(wandev_conf_t* conf, char* devname)
{
	sdlahw_t	*hw = NULL;
	sdlahw_cpu_t	*hwcpu = NULL;
	int		cpu_no = SDLA_CPU_A;
	
	if (conf && conf->S514_CPU_no[0] == 'B'){
		cpu_no = SDLA_CPU_B;
	}else if (conf && conf->S514_CPU_no[0] == 'A'){
		cpu_no = SDLA_CPU_A;
	}
	
	WAN_LIST_FOREACH(hw, &sdlahw_head, next){
	
		WAN_ASSERT_RC(hw->hwcpu == NULL, NULL);
		hwcpu = hw->hwcpu;
		if (conf == NULL){
			if (hw->used == 0/*hwcpu->used < hwcpu->max_ports*/){
				return hw;
			}
			continue;
		}
		if (conf->auto_pci_cfg){
			if (!sdla_cmp_adapter_auto(hw, conf)){
				goto adapter_found;
			}
		}
		switch(conf->card_type){
#if defined(WAN_ISA_SUPPORT)
			case WANOPT_S50X:
				if (hwcpu->hwcard->ioport == conf->ioport){ 
					goto adapter_found;
				}
				break;
#endif				
			case WANOPT_ADSL:
			case WANOPT_AFT300:
			case WANOPT_AFT_ANALOG:
			case WANOPT_AFT_56K:

				if ((hwcpu->hwcard->slot_no == conf->PCI_slot_no) && 
			    	    (hwcpu->hwcard->bus_no == conf->pci_bus_no) &&
				    (hwcpu->hwcard->cfg_type == conf->card_type)){
					goto adapter_found;
				}
				break;

			case WANOPT_AFT_SERIAL:

				if ((hwcpu->hwcard->slot_no == conf->PCI_slot_no) && 
			    	    (hwcpu->hwcard->bus_no == conf->pci_bus_no) &&
				    (hwcpu->hwcard->cfg_type == conf->card_type) &&
				    (hw->port_no == conf->fe_cfg.line_no-1)){
					goto adapter_found;
				}
				break;

			case WANOPT_S51X:
				if (IS_56K_MEDIA(&conf->fe_cfg) && 
				    hwcpu->hwcard->slot_no == conf->PCI_slot_no && 
			    	    hwcpu->hwcard->bus_no == conf->pci_bus_no &&
				    hwcpu->hwcard->cfg_type == WANOPT_AFT_56K) {
					/* Remap the old 56K card type to standard
					   AFT 56K Shark style.  We are allowing
					   and old config file for 56K */
					conf->card_type = WANOPT_AFT_56K;
					conf->config_id = WANCONFIG_AFT_56K;
					conf->fe_cfg.line_no=1;
                                       	goto adapter_found;
				}else if ((hwcpu->hwcard->slot_no == conf->PCI_slot_no) && 
					  (hwcpu->hwcard->bus_no == conf->pci_bus_no) &&
					  (hwcpu->cpu_no == cpu_no) &&
    					  (hw->port_no == conf->comm_port) &&
					  (hwcpu->hwcard->cfg_type == conf->card_type)){
						goto adapter_found;
				}
				break;

			case WANOPT_AFT:
				/* Allow old A101 config for A101 SHARK */
				if (conf->card_type == WANOPT_AFT &&
				    hwcpu->hwcard->slot_no == conf->PCI_slot_no && 
			    	    hwcpu->hwcard->bus_no == conf->pci_bus_no &&
				    hwcpu->hwcard->cfg_type == WANOPT_AFT101) {
					/* Remap the card type to standard
					   A104 Shark style.  We are allowing
					   and old config file for A101/2-SH */
					conf->config_id = WANCONFIG_AFT_TE1;
					conf->card_type = WANOPT_AFT104;
					conf->fe_cfg.line_no=1;
                                       	goto adapter_found;
				}

				/* Allow old A102 config for A102 SHARK */
				if (conf->card_type == WANOPT_AFT &&
				    hwcpu->hwcard->slot_no == conf->PCI_slot_no && 
			    	    hwcpu->hwcard->bus_no == conf->pci_bus_no &&
				    hwcpu->hwcard->cfg_type == WANOPT_AFT102) {
					/* Remap the card type to standard
					   A104 Shark style.  We are allowing
					   and old config file for A101/2-SH */
					conf->config_id = WANCONFIG_AFT_TE1;
					conf->card_type = WANOPT_AFT104;
					if (cpu_no == SDLA_CPU_A) {
						conf->fe_cfg.line_no=1;
					} else {
						conf->fe_cfg.line_no=2;		
					}
                                       	goto adapter_found;
				}

				if (conf->card_type == WANOPT_S51X &&
				    IS_56K_MEDIA(&conf->fe_cfg) && 
				    hwcpu->hwcard->slot_no == conf->PCI_slot_no && 
				    hwcpu->hwcard->bus_no == conf->pci_bus_no &&
				    hwcpu->hwcard->cfg_type == WANOPT_AFT_56K) {
					/* Remap the old 56K card type to standard
					   AFT 56K Shark style.  We are allowing
					   and old config file for 56K */
					conf->card_type = WANOPT_AFT_56K;
					conf->config_id = WANCONFIG_AFT_56K;
					conf->fe_cfg.line_no=1;
					goto adapter_found;
					      
				}

				if ((hwcpu->hwcard->slot_no == conf->PCI_slot_no) && 
			    	    (hwcpu->hwcard->bus_no == conf->pci_bus_no) &&
			    	    (hwcpu->cpu_no == cpu_no) &&
				    (hwcpu->hwcard->cfg_type == conf->card_type)){
					goto adapter_found;
				}
				break;
			
			case WANOPT_AFT_ISDN:
				if ((hwcpu->hwcard->slot_no == conf->PCI_slot_no) && 
			    	    (hwcpu->hwcard->bus_no == conf->pci_bus_no) &&
				    (hw->port_no == conf->fe_cfg.line_no-1) &&
				    (hwcpu->hwcard->cfg_type == conf->card_type)){
					goto adapter_found;
				}
				break;

			case WANOPT_AFT101:
			case WANOPT_AFT102:
			case WANOPT_AFT104:
			case WANOPT_AFT108:
				if ((hwcpu->hwcard->slot_no == conf->PCI_slot_no) && 
			    	    (hwcpu->hwcard->bus_no == conf->pci_bus_no) &&
				    (hw->port_no == conf->fe_cfg.line_no-1)){
					goto adapter_found;
				}
				break;

			default:
				DEBUG_EVENT("%s: Unknown card type (%x) requested by user!\n",
						devname, conf->card_type);
				return NULL;
	       }
	}/* WAN_LIST_FOREACH */

adapter_found:
	if (hw && hwcpu){
		switch(hwcpu->hwcard->adptr_type){
		case S5144_ADPTR_1_CPU_T1E1:
		case S5147_ADPTR_2_CPU_T1E1:
		case A101_ADPTR_1TE1:
		case A200_ADPTR_ANALOG:
		case A400_ADPTR_ANALOG:
		case AFT_ADPTR_56K:
			conf->comm_port = 0;
			conf->fe_cfg.line_no = 0;
			break;
#if defined(CONFIG_PRODUCT_WANPIPE_AFT_BRI)
		case AFT_ADPTR_ISDN:
			if (conf->fe_cfg.line_no < 1 || conf->fe_cfg.line_no > MAX_BRI_LINES){
				DEBUG_EVENT("%s: Error, Invalid port selected %d (Min=1 Max=%d)\n",
						devname, conf->fe_cfg.line_no, MAX_BRI_LINES);
				return NULL;
			}
			conf->fe_cfg.line_no--;
			conf->comm_port = conf->fe_cfg.line_no;
			break;
#endif
		case A101_ADPTR_2TE1:
			if (hwcpu->hwcard->adptr_subtype == AFT_SUBTYPE_NORMAL){
				conf->comm_port = 0;
				conf->fe_cfg.line_no = 0;
				break;
			}
			if (conf->fe_cfg.line_no < 1 || conf->fe_cfg.line_no > 2){
				DEBUG_EVENT("%s: Error, Invalid port selected %d (Min=1 Max=2)\n",
						devname, conf->fe_cfg.line_no);
				return NULL;
			}
			conf->fe_cfg.line_no--;
			conf->comm_port = conf->fe_cfg.line_no;			
			break;
		case A104_ADPTR_4TE1:
			if (conf->fe_cfg.line_no < 1 || conf->fe_cfg.line_no > 4){
				DEBUG_EVENT("%s: Error, Invalid port selected %d (Min=1 Max=4)\n",
						devname, conf->fe_cfg.line_no);
				return NULL;
			}
			conf->fe_cfg.line_no--;
			conf->comm_port = conf->fe_cfg.line_no;
			break;
		case A108_ADPTR_8TE1:
			if (conf->fe_cfg.line_no < 1 || conf->fe_cfg.line_no > 8){
				DEBUG_EVENT("%s: Error, Invalid port selected %d (Min=1 Max=8)\n",
						devname, conf->fe_cfg.line_no);
				return NULL;
			}
			conf->fe_cfg.line_no--;
			conf->comm_port = conf->fe_cfg.line_no;
			break;
		case AFT_ADPTR_2SERIAL_V35X21:
		case AFT_ADPTR_2SERIAL_RS232:
			if (conf->fe_cfg.line_no < 1 || conf->fe_cfg.line_no > 2){
				DEBUG_EVENT("%s: Error, Invalid port selected %d (Min=1 Max=2)\n",
						devname, conf->fe_cfg.line_no);
				return NULL;
			}
			conf->fe_cfg.line_no--;
			conf->comm_port = conf->fe_cfg.line_no;
			break;
		case AFT_ADPTR_4SERIAL_V35X21:
		case AFT_ADPTR_4SERIAL_RS232:
			if (conf->fe_cfg.line_no < 1 || conf->fe_cfg.line_no > 4){
				DEBUG_EVENT("%s: Error, Invalid port selected %d (Min=1 Max=4)\n",
						devname, conf->fe_cfg.line_no);
				return NULL;
			}
			conf->fe_cfg.line_no--;
			conf->comm_port = conf->fe_cfg.line_no;
			break;
		}
		if (hw->used == 0){
			return hw;
		}
		switch(conf->card_type){
#if defined(WAN_ISA_SUPPORT)
		case WANOPT_S50X:
	                DEBUG_EVENT(
			"%s: Error, Sangoma ISA resource busy: (ioport #%x, %s)\n",
                	        devname, conf->ioport, COMPORT_DECODE(conf->comm_port));
			break;
#endif

		case WANOPT_S51X:
		case WANOPT_ADSL:
		case WANOPT_AFT:
		case WANOPT_AFT101:
		case WANOPT_AFT102:
		case WANOPT_AFT104:
		case WANOPT_AFT108:
		case WANOPT_AFT300:
		case WANOPT_AFT_ANALOG:
		case WANOPT_AFT_ISDN:
		case WANOPT_AFT_56K:
		case WANOPT_AFT_SERIAL:

			switch(hwcpu->hwcard->adptr_type){
			case A101_ADPTR_1TE1:
			case A101_ADPTR_2TE1:
				if (hwcpu->hwcard->adptr_subtype == AFT_SUBTYPE_NORMAL){
					DEBUG_EVENT(
					"%s: Error, %s resources busy: (bus #%d, slot #%d, cpu %c)\n",
						devname, 
						SDLA_DECODE_CARDTYPE(conf->card_type),
						conf->pci_bus_no, 
						conf->PCI_slot_no,
						conf->S514_CPU_no[0]);				
				}else{
					DEBUG_EVENT(
					"%s: Error, %s resources busy: (bus #%d, slot #%d, cpu %c, line %d)\n",
						devname, 
						SDLA_DECODE_CARDTYPE(conf->card_type),
						conf->pci_bus_no, 
						conf->PCI_slot_no,
						conf->S514_CPU_no[0],
						conf->fe_cfg.line_no);				
				}
				break;
			case A104_ADPTR_4TE1:
			case A108_ADPTR_8TE1:
				DEBUG_EVENT(
				"%s: Error, %s resources busy: (bus #%d, slot #%d, cpu %c, line %d)\n",
        	        	       	devname, 
					SDLA_DECODE_CARDTYPE(conf->card_type),
					conf->pci_bus_no, 
					conf->PCI_slot_no,
					conf->S514_CPU_no[0],
					conf->fe_cfg.line_no);
				break;

			default:
				DEBUG_EVENT(
				"%s: Error, %s resources busy: (bus #%d, slot #%d, cpu %c)\n",
        	        	       	devname, 
					SDLA_DECODE_CARDTYPE(conf->card_type),
					conf->pci_bus_no, 
					conf->PCI_slot_no,
					conf->S514_CPU_no[0]);
				break;
			}
			break;
		default:
			DEBUG_EVENT(
			"%s: Error, %s resources busy: (bus #%d, slot #%d, cpu %c)\n",
        	       		       	devname, 
					SDLA_DECODE_CARDTYPE(conf->card_type),
					conf->pci_bus_no, 
					conf->PCI_slot_no,
					conf->S514_CPU_no[0]);
			break;
		}
		return NULL;
	}/* if (hw) */

	if (conf){
		switch(conf->card_type){
#if defined(WAN_ISA_SUPPORT)
		case WANOPT_S50X:
	                DEBUG_EVENT(
			"%s: Error, Sangoma ISA card not found on ioport #%x, %s\n",
                	        devname, conf->ioport, COMPORT_DECODE(conf->comm_port));
			break;
#endif
		case WANOPT_S51X:
		         DEBUG_EVENT(
			"%s: Error, %s card not found on bus #%d, slot #%d, cpu %c, %s\n",
                	        	devname, 
					SDLA_DECODE_CARDTYPE(conf->card_type),
					conf->pci_bus_no, 
					conf->PCI_slot_no, 
					conf->S514_CPU_no[0],
					COMPORT_DECODE(conf->comm_port));
			break;
		case WANOPT_ADSL:
			DEBUG_EVENT(
			"%s: Error, %s card not found on bus #%d, slot #%d\n",
                	        	devname, 
					SDLA_DECODE_CARDTYPE(conf->card_type),
					conf->pci_bus_no, 
					conf->PCI_slot_no); 
			break;
		case WANOPT_AFT:
		case WANOPT_AFT101:
		case WANOPT_AFT102:
		case WANOPT_AFT104:
		case WANOPT_AFT108:
		case WANOPT_AFT300:
		case WANOPT_AFT_ANALOG:
		case WANOPT_AFT_ISDN:
		case WANOPT_AFT_56K:
		case WANOPT_AFT_SERIAL:

			DEBUG_EVENT(
			"%s: Error, %s card not found on bus #%d, slot #%d, cpu %c, line %d\n",
                	        	devname, 
					SDLA_DECODE_CARDTYPE(conf->card_type),
					conf->pci_bus_no, 
					conf->PCI_slot_no,
					conf->S514_CPU_no[0],
					conf->fe_cfg.line_no);
			break;
		default:
			DEBUG_EVENT(
			"%s: Error, %s card not found!\n",
					devname,
					SDLA_DECODE_CARDTYPE(conf->card_type));
			break;
        	}
	}else{
		DEBUG_EVENT("%s: Error, Failed to find new Sangoma card!\n",
					devname);
	}/* if (conf) */

	return NULL;
}

/* 
 * ============================================================================
 * Detect adapter type.
 * o if adapter type is specified then call detection routine for that adapter
 *   type.  Otherwise call detection routines for every adapter types until
 *   adapter is detected.
 *
 * Notes:
 * 1) Detection tests are destructive! Adapter will be left in shutdown state
 *    after the test.
 */
static int sdla_detect (sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard = NULL;
	sdlahw_cpu_t	*hwcpu;
	int		err = 0;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	switch (hwcard->type){ 
#if defined(WAN_ISA_SUPPORT)
	case SDLA_S508:
		err = sdla_detect_s508(hw);
		break;
#endif
	case SDLA_S514:
		err = sdla_detect_s514(hw);
		break;

	case SDLA_ADSL:
		err = sdla_detect_pulsar(hw);
		break;

	case SDLA_AFT:
		err = sdla_detect_aft(hw);
		break;
	}
	if (err){
		sdla_down(hw);
	}
	return err;
}




/*
*****************************************************************************
*****************************************************************************
***		H A R D W A R E    C O N F I G U R A T I O N		*****
*****************************************************************************
*****************************************************************************
*/

static int sdla_is_te1(void* phw)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard = NULL;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	switch(hwcard->adptr_type){
	case S5144_ADPTR_1_CPU_T1E1:
	case S5147_ADPTR_2_CPU_T1E1:
	case S5148_ADPTR_1_CPU_T1E1:
	case A101_ADPTR_1TE1:
	case A101_ADPTR_2TE1:
	case A104_ADPTR_4TE1:
	case A108_ADPTR_8TE1:
		return 0;
	}
	return -EINVAL;


}
static int sdla_is_56k(void* phw)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard = NULL;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	if (hwcard->adptr_type == S5145_ADPTR_1_CPU_56K){
		return 0;
	}
	return -EINVAL;
}

/*FIXME: Take it out of here and put it in sdladrv
** Args: HW structure not card
*/
static int sdla_check_mismatch(void* phw, unsigned char media)
{
	sdlahw_card_t*	hwcard = NULL;
	sdlahw_cpu_t*	hwcpu = NULL;
	sdlahw_t*	hw = (sdlahw_t*)phw;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if (media == WAN_MEDIA_T1 || 
	    media == WAN_MEDIA_E1){
		if (hwcard->adptr_type != S5144_ADPTR_1_CPU_T1E1 &&
		    hwcard->adptr_type != S5147_ADPTR_2_CPU_T1E1 &&
		    hwcard->adptr_type != S5148_ADPTR_1_CPU_T1E1){
			DEBUG_EVENT("%s: Error: Card type mismatch: User=T1/E1 Actual=%s\n",
				hw->devname,
				hwcard->adptr_name);
			return -EIO;
		}
		hwcard->adptr_type = S5144_ADPTR_1_CPU_T1E1;
		
	}else if (media == WAN_MEDIA_56K){
		if (hwcard->adptr_type != S5145_ADPTR_1_CPU_56K){
			DEBUG_EVENT("%s: Error: Card type mismatch: User=56K Actual=%s\n",
				hw->devname,
				hwcard->adptr_name);
			return -EIO;
		}
	}else{
		if (hwcard->adptr_type == S5145_ADPTR_1_CPU_56K ||
		    hwcard->adptr_type == S5144_ADPTR_1_CPU_T1E1 ||
		    hwcard->adptr_type == S5147_ADPTR_2_CPU_T1E1 ||
		    hwcard->adptr_type == S5148_ADPTR_1_CPU_T1E1){
			DEBUG_EVENT("%s: Error: Card type mismatch: User=S514(1/2/3) Actual=%s\n",
				hw->devname,
				hwcard->adptr_name);
			return -EIO;
		}
	}
	return 0;
}

static int sdla_is_same_hwcard(void* phw1, void *phw2)
{
	sdlahw_t*	hw1 = (sdlahw_t*)phw1;
	sdlahw_t*	hw2 = (sdlahw_t*)phw2;
	sdlahw_cpu_t	*hwcpu1, *hwcpu2;

	WAN_ASSERT_RC(hw1->hwcpu == NULL, 0);
	WAN_ASSERT_RC(hw2->hwcpu == NULL, 0);
	hwcpu1 = hw1->hwcpu;
	hwcpu2 = hw2->hwcpu;

	if (hwcpu1->hwcard == hwcpu2->hwcard){
		return 1;
	}
	return 0;
}

static int sdla_is_same_hwcpu(void* phw1, void *phw2)
{
	sdlahw_t*	hw1 = (sdlahw_t*)phw1;
	sdlahw_t*	hw2 = (sdlahw_t*)phw2;

	WAN_ASSERT_RC(hw1 == NULL, 0);
	WAN_ASSERT_RC(hw2 == NULL, 0);
	if (hw1->hwcpu == hw2->hwcpu){
		return 1;
	}
	return 0;
}

/*
 *****************************************************************************
 * sdla_set_intrhand() - Change interrupt handler for ISA devices. 
 * o For S508, use local structure for store pointer to real interrupt handler.
 * o For S514, register new interrupt handler for each CPU. 
 */
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static int
sdla_set_intrhand(void* phw, wan_pci_ifunc_t *isr_func, void* arg, int line_no)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_t*	hw = (sdlahw_t*)phw;
#if defined(SDLA_AUTO_PROBE)
	struct sdla_softc	*adapter;
#else
    	sdladev_t*	adapter = NULL;
#endif
#if defined(__FreeBSD__) && (__FreeBSD_version >= 450000)
	int 		error = 0;
#endif

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	SDLA_MAGIC(hw);

#if defined(SDLA_AUTO_PROBE)
	adapter = device_get_softc(hwcard->pci_dev);
# if __FreeBSD_version < 700031
     	error = bus_setup_intr(
			hwcard->pci_dev,
			adapter->irq_res, 
			INTR_TYPE_NET/*|INTR_MPSAFE*/,
			isr_func, arg, &hwcpu->irqh[line_no]);
# else
     	error = bus_setup_intr(
			hwcard->pci_dev,
			adapter->irq_res, 
			INTR_TYPE_NET/*|INTR_MPSAFE*/,NULL, 
			isr_func, arg, &hwcpu->irqh[line_no]);
# endif
    	if (error){
		DEBUG_EVENT(
		"%s: Failed set interrupt handler for Port %d (er=%d)!\n",
				device_get_name(hwcard->pci_dev), line_no,
				error);
		return error;
	}
#else
	adapter = (sdladev_t*)hwcpu->sdla_dev;
	WAN_ASSERT(adapter == NULL);
	WAN_ASSERT(adapter->sc == NULL);

# if defined(__FreeBSD__) && (__FreeBSD_version >= 450000)

#  if __FreeBSD_version < 700031
     	error = bus_setup_intr(
			adapter->dev,
			adapter->sc->irq_res, 
			INTR_TYPE_NET, isr_func, arg, 
			&hwcpu->irqh[line_no]);
#  else
     	error = bus_setup_intr(
			adapter->dev,
			adapter->sc->irq_res, 
			INTR_TYPE_NET, NULL, isr_func, arg, 
			&hwcpu->irqh[line_no]);
#  endif
    	if (error){
		DEBUG_EVENT(
		"%s: Failed set interrupt handler for Port %d (er=%d)!\n",
					device_get_name(adapter->dev), line_no,
					error);
		return error;
    	}

# else
	if (adapter->intr_arg[line_no].ready){
  		adapter->intr_arg[line_no].isr  = isr_func;
       		adapter->intr_arg[line_no].arg  = arg;
	}else{
		return -EINVAL;
	}
# endif
#endif
   	return 0;
}
#endif

/*
 *****************************************************************************
 * restore_intrhand() - Restore interrupt handler for ISA/PCI devices.
 * o For S508, set to NULL pointer to interrupt handler in local structure.
 * o For S514, call pci_unmap_int().
 */
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static int sdla_restore_intrhand(void* phw, int line_no)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;
#if defined(SDLA_AUTO_PROBE)
	struct sdla_softc	*adapter;
#else
	sdladev_t	*adapter = NULL;
#endif
#if defined(__FreeBSD__) && (__FreeBSD_version >= 450000)
	int		error;
#endif
	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard= hwcpu->hwcard;	
	SDLA_MAGIC(hw);

#if defined(SDLA_AUTO_PROBE)
	adapter = device_get_softc(hwcard->pci_dev);
	error = bus_teardown_intr(
			hwcard->pci_dev,adapter->irq_res,hwcpu->irqh[line_no]);
	if (error){
		DEBUG_EVENT(
		"%s: Failed unregister interrupt handler for Port %d (%d)!\n",
				device_get_name(hwcard->pci_dev), line_no,
				error);
		return -EINVAL;
	}
	hwcpu->irqh[line_no] = NULL;

#else

	adapter = (sdladev_t*)hwcpu->sdla_dev;
	WAN_ASSERT(adapter == NULL);
	WAN_ASSERT(adapter->sc == NULL);

# if defined(__FreeBSD__) && (__FreeBSD_version >= 450000)
	error = bus_teardown_intr(adapter->dev, adapter->sc->irq_res, hwcpu->irqh[line_no]);
	if (error){
		DEBUG_EVENT(
		"%s: Failed unregister interrupt handler for Port %d (%d)!\n",
					device_get_name(adapter->dev), line_no,
					error);
		return -EINVAL;
	}
# else
    	/* Restore local vector */
    	adapter->intr_arg[line_no].isr  = NULL;
    	adapter->intr_arg[line_no].arg  = NULL;
# endif
#endif
    	return 0;
}
#endif



static int sdla_getcfg(void* phw, int type, void* value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t*	hwcpu;
	sdlahw_card_t*	hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	switch(type){
	case SDLA_HWTYPE:
		*(u16*)value = (u16)hwcard->hw_type;
		break;
	case SDLA_CARDTYPE:
		*(u16*)value = (u16)hwcard->type;
		break;
	case SDLA_MEMBASE:
		*(sdla_mem_handle_t*)value = hwcpu->dpmbase;
		break;
	case SDLA_MEMEND:
		*(u32*)value = ((unsigned long)hwcpu->dpmbase + hwcpu->dpmsize - 1);
		break;
	case SDLA_MEMSIZE:
		*(u16*)value = (u16)hwcpu->dpmsize;
		break;
	case SDLA_MEMORY:
		*(u32*)value = hwcpu->memory;
		break;
	case SDLA_IRQ:
		*(u16*)value = (u16)hwcpu->irq;
		break;
	case SDLA_IOPORT:
		*(u16*)value = (u16)hwcard->ioport;
		break;
	case SDLA_IORANGE:
		*(u16*)value = (u16)hwcpu->io_range;
		break;
	case SDLA_ADAPTERTYPE:
		*(u16*)value = (u16)hwcard->adptr_type;
		break;
	case SDLA_CPU:
		*(u16*)value = (u16)hwcpu->cpu_no;
		break;
	case SDLA_SLOT:
		*(u16*)value = (u16)hwcard->slot_no;
		break;
	case SDLA_BUS:
		*(u16*)value = (u16)hwcard->bus_no;
		break;
	case SDLA_DMATAG:
#if defined(__NetBSD__) || defined(__OpenBSD__)
		if (hwcard->pci_dev){
			*(bus_dma_tag_t*)value = hwcard->pci_dev->pa_dmat;
		}
#endif
		break;
	case SDLA_PCIEXTRAVER:
		*(u8*)value = hwcard->pci_extra_ver;
		break;
	case SDLA_BASEADDR:
		*(u32*)value = hwcpu->mem_base_addr;
		break;
	case SDLA_COREREV:
		*(u8*)value = hwcard->core_rev;
		break;
	case SDLA_COREID:
		*(u8*)value = hwcard->core_id;
		break;
	case SDLA_HWCPU_USEDCNT:
#if defined(__WINDOWS__)
		*(u32*)value = get_usage_counter(hw->p_sdla);
#else
		*(u32*)value = hwcpu->used;
#endif
		break;
	case SDLA_ADAPTERSUBTYPE:
		*(u8*)value = hwcard->adptr_subtype;
		break;
	case SDLA_HWEC_NO:
		*(u16*)value = hwcard->hwec_chan_no;
		break;
	case SDLA_PCIEXPRESS:
		*(u8*)value = (u8)sdla_is_pciexpress(hw);
		break;
	case SDLA_PORTS_NO:
		*(u16*)value = (u16)hwcpu->max_ports;
		break;
	case SDLA_PORT_MAP:
		*(u32*)value = hwcpu->port_map;
		break;
	case SDLA_HWPORTUSED:
		*(u16*)value = (u16)hw->used;
		DEBUG_BRI("SDLA_HWPORTUSED: %d\n", hw->used);
		break;
	case SDLA_HWPORTREG:
		*(u16*)value = (u16)hw->hwcpu->reg_port[hw->port_no];
		break;
	case SDLA_RECOVERY_CLOCK_FLAG:
		*(u32*)value = hwcard->recovery_clock_flag;
		break;
	case SDLA_HWPORTREGMAP:
		*(u32*)value = (u32)hw->hwcpu->reg_port_map;
		break;
	}
	return 0;
}


static int sdla_setcfg(void* phw, int type, void* value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t*	hwcpu;
	sdlahw_card_t*	hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	switch(type){
	case SDLA_RECOVERY_CLOCK_FLAG:
		hwcard->recovery_clock_flag = *(u32*)value;
		break;
	}
	return 0;
}

static int sdla_get_hwcard(void* phw, void** phwcard)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	SDLA_MAGIC(hw);

	*phwcard = hwcpu->hwcard;
	return 0;
}


static int sdla_get_hwprobe(void* phw, int port, void** hwinfo)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);

	if (hw->hwprobe){
		*hwinfo = hw->hwprobe->hw_info;
	}
	return 0;
}

/*
*****************************************************************************
*****************************************************************************
***			M I S C E L L A N E O U S			*****
*****************************************************************************
*****************************************************************************
*/

#if defined(WAN_ISA_SUPPORT)
/*============================================================================
 * Get option's index into the options list.
 *	Return option's index (1 .. N) or zero if option is invalid.
 */
static int sdla_get_option_index (unsigned* optlist, unsigned optval)
{
	int i;

	for (i = 1; i <= optlist[0]; ++i)
		if ( optlist[i] == optval)
			return i;
	return 0;
}
#endif

/*============================================================================
 * Calculate 16-bit CRC using CCITT polynomial.
 */
static unsigned short sdla_checksum (unsigned char* buf, unsigned len)
{
	unsigned short crc = 0;
	unsigned mask, flag;

	for (; len; --len, ++buf) {
		for (mask = 0x80; mask; mask >>= 1) {
			flag = (crc & 0x8000);
			crc <<= 1;
			crc |= ((*buf & mask) ? 1 : 0);
			if (flag) crc ^= 0x1021;
		}
	}
	return crc;
}




static int sdla_io_read_1(void* phw, unsigned int offset, u8* value) 
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT2(hw == NULL, 0);
	WAN_ASSERT2(hw->hwcpu == NULL, 0);
	hwcpu = hw->hwcpu;

	if (!(hwcpu->status & SDLA_IO_MAPPED)) return 0;
#if defined(__FreeBSD__)
	*value = readb ((u8*)hwcpu->vector + offset);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	*value = bus_space_read_1(hwcpu->hwcard->memt, hwcpu->vector, offset);
#elif defined(__LINUX__)
	*value = wp_readb((hwcpu->vector + offset));
#elif defined(__WINDOWS__)
	*value = READ_REGISTER_UCHAR((PUCHAR)((PUCHAR)hwcpu->vector + offset));
#else
# warning "sdla_io_read_1: Not supported yet!"
#endif
	return 0;
}

static int sdla_io_write_1(void* phw, unsigned int offset, u8 value) 
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	if (!(hwcpu->status & SDLA_IO_MAPPED)) return 0;
#if defined(__FreeBSD__)
	writeb ((u8*)hwcpu->vector+offset, value);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_write_1(hwcpu->hwcard->memt, hwcpu->vector, offset, value);
#elif defined(__LINUX__)
	wp_writeb(value, hwcpu->vector + offset);
#elif defined(__WINDOWS__)
	WRITE_REGISTER_UCHAR((PUCHAR)((PUCHAR)hwcpu->vector + offset), value);
#else
# warning "sdla_io_write_1: Not supported yet!"
#endif
	return 0;
}

int sdla_bus_write_1(void* phw, unsigned int offset, u8 value) 
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	
	WAN_ASSERT(hw == NULL);	
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	if (!(hwcpu->status & SDLA_MEM_MAPPED)) return 0;
#if defined(__FreeBSD__)
	writeb(((u8*)hwcpu->dpmbase + offset), value);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_write_1(hwcpu->hwcard->memt, hwcpu->dpmbase, offset, value);
#elif defined(__LINUX__)
	/*DEBUG_BRI("%s(): offset: 0x%X, value: 0x%X\n", __FUNCTION__, offset, value);*/
	wp_writeb(value, hwcpu->dpmbase + offset);
#elif defined(__WINDOWS__)
	WRITE_REGISTER_UCHAR((PUCHAR)((PUCHAR)hwcpu->dpmbase + offset), value);
#else
# warning "sdla_bus_write_1: Not supported yet!"
#endif
	return 0;
}

int sdla_bus_write_2(void* phw, unsigned int offset, u16 value) 
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	
	WAN_ASSERT(hw == NULL);	
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	if (!(hwcpu->status & SDLA_MEM_MAPPED)) return 0;
#if defined(__FreeBSD__)
	writew(((u8*)hwcpu->dpmbase + offset), value);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_write_2(hwcpu->hwcard->memt, hwcpu->dpmbase, offset, value);
#elif defined(__LINUX__)
	/*DEBUG_BRI("%s(): offset: 0x%X, value: 0x%X\n", __FUNCTION__, offset, value);*/
	wp_writew(value,hwcpu->dpmbase+offset);
#elif defined(__WINDOWS__)
	WRITE_REGISTER_USHORT((PUSHORT)((PUCHAR)hwcpu->dpmbase + offset), value);
#else
# warning "sdla_bus_write_2: Not supported yet!"
#endif
	return 0;
}

int sdla_bus_write_4(void* phw, unsigned int offset, u32 value) 
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	
	WAN_ASSERT(hw == NULL);	
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	if (!(hwcpu->status & SDLA_MEM_MAPPED)) return 0;

	DEBUG_TEST("%s(): Offset=0x%04X Data=0x%08X\n", __FUNCTION__,
			offset, value);

#if defined(__FreeBSD__)
	writel(((u8*)hwcpu->dpmbase + offset), value);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_write_4(hwcpu->hwcard->memt, hwcpu->dpmbase, offset, value);
#elif defined(__LINUX__)
	/*DEBUG_BRI("%s(): offset: 0x%X, value: 0x%X\n", __FUNCTION__, offset, value);*/
	wp_writel(value,(u8*)hwcpu->dpmbase + offset);
#elif defined(__WINDOWS__)
	WRITE_REGISTER_ULONG((PULONG)((PUCHAR)hwcpu->dpmbase + offset), value);
#else
# warning "sdla_bus_write_4: Not supported yet!"
#endif
	return 0;
}

int sdla_bus_read_1(void* phw, unsigned int offset, u8* value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	
	WAN_ASSERT2(hw == NULL, 0);	
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	if (!(hwcpu->status & SDLA_MEM_MAPPED)) return 0;
#if defined(__FreeBSD__)
	*value = readb(((u8*)hwcpu->dpmbase + offset));
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	*value = bus_space_read_1(hwcpu->hwcard->memt, hwcpu->dpmbase, offset); 
#elif defined(__LINUX__)
	*value = wp_readb((hwcpu->dpmbase + offset));
#elif defined(__WINDOWS__)
	*value = READ_REGISTER_UCHAR((PUCHAR)((PUCHAR)hwcpu->dpmbase + offset));
#else
# warning "sdla_bus_read_1: Not supported yet!"
#endif
	return 0;
}

int sdla_bus_read_2(void* phw, unsigned int offset, u16* value)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	
	WAN_ASSERT2(hw == NULL, 0);	
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	if (!(hwcpu->status & SDLA_MEM_MAPPED)) return 0;
#if defined(__FreeBSD__)
	*value = readw(((u8*)hwcpu->dpmbase + offset));
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	*value = bus_space_read_2(hwcpu->hwcard->memt, hwcpu->dpmbase, offset); 
#elif defined(__LINUX__)
	*value = readw(((unsigned char*)hwcpu->dpmbase+offset));
#elif defined(__WINDOWS__)
	*value = READ_REGISTER_USHORT((PUSHORT)((PUCHAR)hwcpu->dpmbase + offset));
#else
# warning "sdla_bus_read_2: Not supported yet!"
#endif
	return 0;
}

int sdla_bus_read_4(void* phw, unsigned int offset, u32* value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	int retry=5;
	
	WAN_ASSERT2(hw == NULL, 0);	
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT2(hw->hwcpu->dpmbase == 0, 0);	
	hwcpu = hw->hwcpu;

	if (!(hwcpu->status & SDLA_MEM_MAPPED)) return 0;

	do {

#if defined(__FreeBSD__)
	*value = readl(((u8*)hw->hwcpu->dpmbase + offset));
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	*value = bus_space_read_4(hw->hwcard->memt, hwcpu->hwcpu->dpmbase, offset); 
#elif defined(__LINUX__)
	*value = wp_readl((unsigned char*)hw->hwcpu->dpmbase + offset);
#elif defined(__WINDOWS__)
	*value = READ_REGISTER_ULONG((PULONG)((PUCHAR)hwcpu->dpmbase + offset));
#else
	*value = 0;
# warning "sdla_bus_read_4: Not supported yet!"
#endif
#if 1
		if (offset == 0x40 && *value == (u32)-1) {
			if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s:%d: wanpipe PCI Error: Illegal Register read: 0x%04X = 0x%08X\n",
				__FUNCTION__,__LINE__,offset,*value);
			}
		} else {
			/* only check for register 0x40 */
			break;
		}
#endif
	}while(--retry);

	return 0;
}

static int sdla_pci_read_config_dword(void* phw, int reg, u32* value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	card;
	sdlahw_cpu_t*	hwcpu;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	*value = pci_read_config(card->pci_dev, reg, 4);
# else
	*value = ci_cfgread(card->pci_dev, reg, 4);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	*value = pci_conf_read(card->pci_dev->pa_pc, card->pci_dev->pa_tag, reg);
#elif defined(__LINUX__)
	pci_read_config_dword(card->pci_dev, reg, value);
#elif defined(__WINDOWS__)
	pci_read_config_dword(card->pci_dev, reg, value);
#else
# warning "sdla_pci_read_config_dword: Not supported yet!"
#endif
	return 0;
}

static int sdla_pci_read_config_word(void* phw, int reg, u16* value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	card;
	sdlahw_cpu_t*	hwcpu;
#if defined(__NetBSD__) || defined(__OpenBSD__)
	u32		tmp = 0x00;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	*value = pci_read_config(card->pci_dev, reg, 2);
# else
	*value = pci_cfgread(card->pci_dev, reg, 2);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	tmp = pci_conf_read(card->pci_dev->pa_pc, card->pci_dev->pa_tag, reg);
	*value = (u16)((tmp >> 16) & 0xFFFF);
#elif defined(__LINUX__)
	pci_read_config_word(card->pci_dev, reg, value);
#elif defined(__WINDOWS__)
	pci_read_config_word(card->pci_dev, reg, value);
#else
# warning "sdla_pci_read_config_word: Not supported yet!"
#endif
	return 0;
}

static int sdla_pci_read_config_byte(void* phw, int reg, u8* value)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;
#if defined(__NetBSD__) || defined(__OpenBSD__)
	u32		tmp = 0x00;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	*value = pci_read_config(hwcard->pci_dev, reg, 1);
# else
	*value = pci_cfgread(hwcard->pci_dev, reg, 1);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	tmp = pci_conf_read(hwcard->pci_dev->pa_pc, hwcard->pci_dev->pa_tag, reg);
	*value = (u8)(tmp & 0xFF);
#elif defined(__LINUX__)
	pci_read_config_byte(hwcard->pci_dev, reg, value);
#elif defined(__WINDOWS__)
	pci_read_config_byte(hwcard->pci_dev, reg, value);
#else
# warning "sdla_pci_read_config_byte: Not supported yet!"
#endif
	return 0;
}

static int sdla_pci_write_config_dword(void* phw, int reg, u32 value)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	pci_write_config(hwcard->pci_dev, reg, value, 4);
# else
	pci_conf_write(hwcard->pci_dev, reg, 4);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	pci_conf_write(hwcard->pci_dev->pa_pc, hwcard->pci_dev->pa_tag, reg, value); 
#elif defined(__LINUX__)
	pci_write_config_dword(hwcard->pci_dev, reg, value);
#elif defined(__WINDOWS__)
	pci_write_config_dword(hwcard->pci_dev, reg, value);
#else
# warning "sdla_pci_write_config_dword: Not supported yet!"
#endif
	return 0;
}

static int sdla_pci_write_config_word(void* phw, int reg, u16 value)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	pci_write_config(hwcard->pci_dev, reg, value, 2);
# else
	pci_conf_write(hwcard->pci_dev, reg, value, 2);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	pci_conf_write(hwcard->pci_dev->pa_pc, hwcard->pci_dev->pa_tag, reg, value); 
#elif defined(__LINUX__)
	pci_write_config_word(hwcard->pci_dev, reg, value);
#elif defined(__WINDOWS__)
	pci_write_config_word(hwcard->pci_dev, reg, value);
#else
# warning "sdla_pci_write_config_word: Not supported yet!"
#endif
	return 0;
}

static int sdla_pci_write_config_byte(void* phw, int reg, u8 value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	card;
	sdlahw_cpu_t*	hwcpu;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	pci_write_config(card->pci_dev, reg, value, 1);
# else
	pci_conf_write(card->pci_dev, reg, value, 1);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	pci_conf_write(card->pci_dev->pa_pc, card->pci_dev->pa_tag, reg, value); 
#elif defined(__LINUX__)
	pci_write_config_byte(card->pci_dev, reg, value);
#elif defined(__WINDOWS__)
	pci_write_config_byte(card->pci_dev, reg, value);
#else
# warning "sdla_pci_write_config_byte: Not supported yet!"
#endif
	return 0;
}

int sdla_pci_bridge_read_config_dword(void* phw, int reg, u_int32_t* value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	card;
	sdlahw_cpu_t*	hwcpu;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	
	if (!sdla_is_pciexpress(hw)){
		*value = 0xFFFFFFFF;
		return 0;	
	}
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	*value = pci_read_config(card->pci_bridge_dev, reg, 4);
# else
	*value = ci_cfgread(card->pci_bridge_dev, reg, 4);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	*value = pci_conf_read(card->pci_bridge_dev->pa_pc, card->pci_bridge_dev->pa_tag, reg);
#elif defined(__LINUX__)
	pci_read_config_dword(card->pci_bridge_dev, reg, value);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "sdla_pci_bridge_read_config_dword: Not supported yet!"
#endif
	return 0;
}

static int sdla_pci_bridge_read_config_byte(void* phw, int reg, u_int8_t* value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	card;
	sdlahw_cpu_t*	hwcpu;
#if defined(__NetBSD__) || defined(__OpenBSD__)
	u32		tmp = 0x00;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	
	if (!sdla_is_pciexpress(hw)){
		*value = 0xFF;
		return 0;	
	}
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	*value = pci_read_config(card->pci_bridge_dev, reg, 1);
# else
	*value = ci_cfgread(card->pci_bridge_dev, reg, 1);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	tmp = pci_conf_read(card->pci_bridge_dev->pa_pc, card->pci_bridge_dev->pa_tag, reg);
	*value = tmp & 0xFF;
#elif defined(__LINUX__)
	pci_read_config_byte(card->pci_bridge_dev, reg, value);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "sdla_pci_bridge_read_config_byte: Not supported yet!"
#endif
	return 0;
}

int sdla_pci_bridge_write_config_dword(void* phw, int reg, u_int32_t value)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if (!sdla_is_pciexpress(hw)){
		return 0;	
	}
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	pci_write_config(hwcard->pci_bridge_dev, reg, value, 4);
# else
	pci_conf_write(hwcard->pci_bridge_dev, reg, 4);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	pci_conf_write(hwcard->pci_bridge_dev->pa_pc, hwcard->pci_bridge_dev->pa_tag, reg, value); 
#elif defined(__LINUX__)
	pci_write_config_dword(hwcard->pci_bridge_dev, reg, value);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "sdla_pci_bridge_write_config_dword: Not supported yet!"
#endif
	return 0;
}

static int sdla_pci_bridge_write_config_byte(void* phw, int reg, u_int8_t value)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	
	if (!sdla_is_pciexpress(hw)){
		return 0;	
	}
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	pci_write_config(hwcard->pci_bridge_dev, reg, value, 1);
# else
	pci_conf_write(hwcard->pci_bridge_dev, reg, 1);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	pci_conf_write(hwcard->pci_bridge_dev->pa_pc, hwcard->pci_bridge_dev->pa_tag, reg, value); 
#elif defined(__LINUX__)
	pci_write_config_byte(hwcard->pci_bridge_dev, reg, value);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "sdla_pci_bridge_write_config_byte: Not supported yet!"
#endif
	return 0;
}

#if defined(WAN_ISA_SUPPORT)	
static int sdla_isa_read_1(void* phw, unsigned int offset, u8* value) 
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
#if defined(__FreeBSD__)
	*value = inb (hwcard->ioport + offset);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	*value = bus_space_read_1(hwcard->iot, hw->ioh, offset);
#elif defined(__LINUX__)
	*value = inb(hwcard->ioport + offset);
#else
# warning "sdla_isa_read_1: Not supported yet!"
#endif
	return 0;
}
#endif

#if defined(WAN_ISA_SUPPORT)	
static int sdla_isa_write_1(void* phw, unsigned int offset, u8 value) 
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	hwcpu->regs[offset] = value;
#if defined(__FreeBSD__)
	outb (hwcard->ioport + offset, value);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_write_1(hwcard->iot, hwcpu->ioh, offset, value);
#elif defined(__LINUX__)
	outb(value, hwcard->ioport + offset);
#else
# warning "sdla_isa_write_1: Not supported yet!"
#endif
	return 0;
}
#endif

static int sdla_hw_lock(void *phw, wan_smp_flag_t *flag)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
#if defined(__WINDOWS__)
	{
		sdla_t *card = hw->p_sdla;

		DBG_FE_LOCK("%s():%s: p_sdla: 0x%p, card lock: 0x%p\n",
			__FUNCTION__, card->devname, hw->p_sdla, get_card_spin_lock(hw->p_sdla));
	}
	wan_spin_lock(get_card_spin_lock(hw->p_sdla));
#else
	wan_spin_lock(&hwcard->pcard_lock);
#endif
	return 0;
}

static int sdla_hw_unlock(void *phw, wan_smp_flag_t *flag)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
#if defined(__WINDOWS__)
	{
		sdla_t *card = hw->p_sdla;

		DBG_FE_LOCK("%s():%s: p_sdla: 0x%p, card lock: 0x%p\n",
			__FUNCTION__, card->devname, hw->p_sdla, get_card_spin_lock(hw->p_sdla));
	}
	wan_spin_unlock(get_card_spin_lock(hw->p_sdla));
#else
	wan_spin_unlock(&hwcard->pcard_lock);
#endif
	return 0;
}

static int sdla_hw_ec_trylock(void *phw, wan_smp_flag_t *flag)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	return wan_spin_trylock(&hwcard->pcard_ec_lock);
}

static int sdla_hw_ec_lock(void *phw, wan_smp_flag_t *flag)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	wan_spin_lock(&hwcard->pcard_ec_lock);
	return 0;
}

static int sdla_hw_ec_unlock(void *phw, wan_smp_flag_t *flag)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;
	wan_spin_unlock(&hwcard->pcard_ec_lock);
	return 0;
}

/*****************************************************************************
**			BUS DMA Access
*****************************************************************************/
static wan_dma_descr_t *sdla_busdma_descr_alloc(void *phw, int num)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	wan_dma_descr_t *dma_descr = NULL;

	WAN_ASSERT_RC(hw == NULL, NULL);
	SDLA_MAGIC_RC(hw, NULL);

	return dma_descr;
}
static void sdla_busdma_descr_free(void *phw, wan_dma_descr_t *dma_descr)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;

	WAN_ASSERT_VOID(hw == NULL);
	SDLA_MAGIC_VOID(hw);
	if (dma_descr) wan_free(dma_descr);
	return;
}
#if defined(__FreeBSD__)
static void
sdla_busdma_callback(void *arg, bus_dma_segment_t *seg, int nseg, int error)
{
	wan_dma_descr_t	*dma_descr;
	sdla_dma_addr_t	paddr;
	bus_size_t	plen;

	WAN_ASSERT_VOID(arg == NULL);
	if (error){
		DEBUG_EVENT(
		"ERROR [%s:%d]: Failed to load DMA buffer (nseg=%d:err=%d)!\n",
					__FUNCTION__,__LINE__, nseg, error);
		return;
	}
	dma_descr = (wan_dma_descr_t*)arg;
	
	paddr	= seg->ds_addr;
	plen	= seg->ds_len;	
	if (paddr & (dma_descr->alignment-1)){
               	dma_descr->dma_offset = 
				dma_descr->alignment - 
				(paddr & (dma_descr->alignment-1));
		DEBUG_EVENT(
		"ERROR [%s:%d]: Invalid Phys DMA Addr %lX Shift dma paddr %d\n",
				__FUNCTION__,__LINE__,
				(unsigned long)paddr,dma_descr->dma_offset);
		error = bus_dmamap_load(
				dma_descr->dmat,
				dma_descr->dmam,
				(char*)dma_descr->dma_virt+dma_descr->dma_offset,
				plen,
				sdla_busdma_callback, dma_descr,
				BUS_DMA_NOWAIT);
		return;
	}
	dma_descr->dma_addr	= paddr;
	dma_descr->dma_len	= plen;
	wan_set_bit(SDLA_DMA_FLAG_READY, &dma_descr->flag);
	DEBUG_DMA("%s:%d DMA Loaded paddr %X plen %d (dma_descr:%p)\n",
				__FUNCTION__,__LINE__,
				dma_descr->dma_addr,
				dma_descr->dma_len,
				dma_descr);
	return;
}
#endif
/*
 *
 * dma_alignment - value should be multiple 2,4
 */
static int 
sdla_busdma_tag_create(	void		*phw,
			wan_dma_descr_t *dma_descr, 
			u32		dma_alignment, 
			u32		dma_max_len, 
			int		ndescr)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	wan_dma_descr_t	*dma_descr_next = NULL;
	int		i;
#if defined(__FreeBSD__)
	int	err;
#endif		

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(dma_alignment == 0);
	SDLA_MAGIC(hw);
#if defined(__FreeBSD__)
	err = bus_dma_tag_create(	
			NULL,				/* NULL */
			dma_alignment,			/*alignemnt*/
			512,				/*boundary*/
#ifdef __amd64__
			BUS_SPACE_MAXADDR,	/*lowaddr*/
#else
			BUS_SPACE_MAXADDR_32BIT,	/*lowaddr*/
#endif
			BUS_SPACE_MAXADDR,		/*highaddr*/
			NULL,				/*filter*/
			NULL,				/*filterarg*/
			dma_max_len,			/*maxsize*/
# if (__FreeBSD_version >= 502000)
			10,				/*nsegments*/
# else
			BUS_SPACE_UNRESTRICTED,		/*nsegments*/
# endif
			dma_max_len,			/*maxsegsz*/
			BUS_DMA_ALLOCNOW,		/*flags*/
# if (__FreeBSD_version >= 502000)
			busdma_lock_mutex, 		/*lockfunc*/
			&Giant,				/*lockfuncarg*/
# endif
			&dma_descr->dmat);
	if (err) {
		DEBUG_EVENT("%s: Failed to create DMA tag (%d:%d:err=%d)!\n",
					hw->devname,
					dma_max_len,dma_alignment, err);
		return -EINVAL;
	}
#elif defined(__LINUX__)
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA TAG Create is not defined!"
#endif
	dma_descr_next = dma_descr;
	for(i = 0; i < ndescr; i++){
#if defined(__FreeBSD__)
		dma_descr_next->dmat		= dma_descr->dmat;
#endif
		dma_descr_next->alignment	= dma_alignment;
		dma_descr_next->max_len		= dma_max_len;
		wan_set_bit(SDLA_DMA_FLAG_INIT, &dma_descr_next->flag);
		dma_descr_next++;
	}
	return 0;
}
static int
sdla_busdma_tag_destroy(void *phw, wan_dma_descr_t *dma_descr, int ndescr)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	wan_dma_descr_t	*dma_descr_next = NULL;
	int		i;
#if defined(__FreeBSD__)
	int	err;
#endif	

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	if (!wan_test_bit(SDLA_DMA_FLAG_INIT, &dma_descr->flag)){
		DEBUG_DMA("%s: Internal Error: %s:%d!\n",
					hw->devname,
					__FUNCTION__,__LINE__);
		return -EINVAL;
	}
#if defined(__FreeBSD__)
	err = bus_dma_tag_destroy(dma_descr->dmat);
	if (err){
		DEBUG_EVENT("%s: Failed to destroy DMA tag (err=%d)!\n",
					hw->devname, err);
		return -EINVAL;
	}
#elif defined(__LINUX__)
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA TAG Destroy is not defined!"
#endif
	dma_descr_next = dma_descr;
	for(i = 0; i < ndescr; i++){
#if defined(__FreeBSD__)
		dma_descr_next->dmat		= 0;
#endif
		dma_descr_next->alignment	= 0;
		dma_descr_next->max_len		= 0;
		wan_clear_bit(SDLA_DMA_FLAG_INIT, &dma_descr_next->flag);
		dma_descr_next++;
	}
	return 0;
}
/* FIXME: This function is not in used (alignment is not support) */
static int sdla_busdma_create(void *phw, wan_dma_descr_t *dma_descr)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
#if defined(__FreeBSD__)
	int		err;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
#if defined(__FreeBSD__)
	err = bus_dmamap_create(dma_descr->dmat,0,&dma_descr->dmam);
	if (err) {
		DEBUG_EVENT(
		"%s: Failed to allocate and initialize DMA map (err=%d))!\n",
					hw->devname,err);
		return -EINVAL;
	}
#elif defined(__LINUX__)
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA Create is not defined!"
#endif
	return 0;
}
/* FIXME: This function is not in used (alignment is not support) */
static int sdla_busdma_destroy(void *phw, wan_dma_descr_t *dma_descr)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
#if defined(__FreeBSD__)
	int		err;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
#if defined(__FreeBSD__)
	err = bus_dmamap_destroy(dma_descr->dmat,dma_descr->dmam);
	if (err) {
		DEBUG_EVENT("%s: Failed to free all DMA resources (err=%d))!\n",
					hw->devname,err);
		return -EINVAL;
	}
#elif defined(__LINUX__)
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA Destroy is not defined!"
#endif
	return 0;
}

/* Allocate virtual buffer for DMA with dma_max_len */ 
static int sdla_busdma_alloc(void *phw, wan_dma_descr_t *dma_descr)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
#if defined(__FreeBSD__)
	int		err;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	if (!wan_test_bit(SDLA_DMA_FLAG_INIT, &dma_descr->flag)){
		DEBUG_DMA("%s: Internal Error: %s:%d!\n",
					hw->devname, __FUNCTION__,__LINE__);
		return -EINVAL;
	}
#if defined(__FreeBSD__)
	err = bus_dmamem_alloc(	dma_descr->dmat,
				&dma_descr->dma_virt,
				BUS_DMA_NOWAIT|BUS_DMA_ZERO/*BUS_DMA_NOWAIT*/,
				&dma_descr->dmam);
	if (err || dma_descr->dma_virt == NULL){
		DEBUG_EVENT(
		"%s: Unable to allocate DMA virtual memory (err=%d)!\n",
					hw->devname, err);
		return -ENOMEM;
	}

	err = bus_dmamap_load(	dma_descr->dmat,
				dma_descr->dmam,
				dma_descr->dma_virt,
				dma_descr->max_len,
				sdla_busdma_callback, dma_descr,
				BUS_DMA_NOWAIT);
	if (err){
		DEBUG_EVENT("%s: Unable to load DMA buffer (%d:err=%d)!\n",
				hw->devname, dma_descr->max_len, err);
		bus_dmamem_free(
				dma_descr->dmat,
				dma_descr->dma_virt,
				dma_descr->dmam);
		return -EINVAL;
	}
	wan_set_bit(SDLA_DMA_FLAG_ALLOC, &dma_descr->flag);
#elif defined(__LINUX__)
# if 0 
	dma_descr->dma_virt = pci_alloc_consistent(	
					NULL,
					dma_descr->max_len,
					(dma_addr_t*)&dma_descr->dma_addr);
	if (dma_descr->dma_virt == NULL){
		DEBUG_EVENT(
		"%s: Unable to allocate DMA virtual memory (err=%d)!\n",
					hw->devname, err);
		return -ENOMEM;
	}
	if (dma_descr->dma_addr & dma_descr->alignment){
		dma_descr->dma_offset = 
				dma_descr->alignment - 
				(dma_descr->dma_addr & dma_descr->alignment) +
				1;
		(u8*)dma_descr->dma_virt += dma_descr->dma_offset;
		dma_descr->dma_addr += dma_descr->dma_offset;
	}else{
		dma_descr->dma_offset = 0;
	}
# endif		
	wan_set_bit(SDLA_DMA_FLAG_ALLOC, &dma_descr->flag);
	wan_set_bit(SDLA_DMA_FLAG_READY, &dma_descr->flag);
#elif defined(__OpenBSD__)
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA Alloc is not defined!"
#endif
	return 0;
}
static void sdla_busdma_free(void *phw, wan_dma_descr_t *dma_descr)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;

	WAN_ASSERT_VOID(hw == NULL);
	SDLA_MAGIC_VOID(hw);
	
	if (!wan_test_bit(SDLA_DMA_FLAG_ALLOC, &dma_descr->flag)){
		DEBUG_DMA("%s: Internal Error: %s:%d!\n",
					hw->devname, __FUNCTION__,__LINE__);
		return;
	}
	wan_clear_bit(SDLA_DMA_FLAG_READY, &dma_descr->flag);
#if defined(__FreeBSD__)
	bus_dmamap_unload(dma_descr->dmat,dma_descr->dmam);
	bus_dmamem_free(dma_descr->dmat,dma_descr->dma_virt,dma_descr->dmam);
#elif defined(__LINUX__)
# if 0
	if (dma_descr->dma_offset){
		(u8*)dma_descr->dma_virt-= dma_descr->dma_offset;
		dma_descr->dma_addr	-= dma_descr->dma_offset;
	}
	pci_free_consistent(	NULL,
				dma_descr->max_len, 
				dma_descrdma_descr->dma_virt,
				dma_descr->dma_addr);
# endif
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA Free is not defined!"
#endif
	wan_clear_bit(SDLA_DMA_FLAG_ALLOC, &dma_descr->flag);
	dma_descr->dma_virt = NULL;
	dma_descr->dma_addr = 0;
	return;
}

/* FIXME: This function is not in used */
static int sdla_busdma_load(void *phw, wan_dma_descr_t *dma_descr, u32 len)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
#if defined(__FreeBSD__)
	int		err;
#endif

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
#if defined(__FreeBSD__)
	err = bus_dmamap_load(	dma_descr->dmat,
				dma_descr->dmam,
				dma_descr->dma_virt,
				len,
				sdla_busdma_callback, dma_descr,
				BUS_DMA_NOWAIT);
	if (err){
		DEBUG_EVENT("%s: Unable to load DMA buffer (%d:err=%d)!\n",
				hw->devname, len, err);
		return -EINVAL;
	}
#elif defined(__LINUX__)
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA Load is not defined!"
#endif
	return 0;
}

/* FIXME: This function is not in used */
static void sdla_busdma_unload(void *phw, wan_dma_descr_t *dma_descr)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;

	WAN_ASSERT_VOID(hw == NULL);
	SDLA_MAGIC_VOID(hw);
#if defined(__FreeBSD__)
	bus_dmamap_unload(dma_descr->dmat, dma_descr->dmam);
#elif defined(__LINUX__)
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA Unload is not defined!"
#endif
	return;
}
static void 
sdla_busdma_map(void *phw, wan_dma_descr_t *dma_descr, void *buf, int buflen, int map_len, int dir)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT_VOID(hw == NULL);
	SDLA_MAGIC_VOID(hw);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT_VOID(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;

	if (!wan_test_bit(SDLA_DMA_FLAG_READY, &dma_descr->flag)){
		DEBUG_DMA("%s: Internal Error: %s:%d: DMA is not ready!\n",
					hw->devname, __FUNCTION__,__LINE__);
		return;
	}
#if defined(__FreeBSD__)
	if (dir == SDLA_DMA_PREWRITE){
		bcopy(buf, dma_descr->dma_virt, buflen);
	}
#elif defined(__LINUX__)
	dma_descr->dma_offset	= 0;
	dma_descr->dma_addr = 
		cpu_to_le32(pci_map_single(hwcard->pci_dev,buf,map_len,dir));
	if (dma_descr->dma_addr & (dma_descr->alignment-1)){
		dma_descr->dma_offset = 
			dma_descr->alignment - 
			(dma_descr->dma_addr & (dma_descr->alignment-1));
		dma_descr->dma_virt = buf + dma_descr->dma_offset;
		dma_descr->dma_addr += dma_descr->dma_offset;
	}else{
		dma_descr->dma_virt	= buf;
		dma_descr->dma_offset	= 0;
	}
#elif defined(__OpenBSD__)
	dma_descr->dma_addr = virt_to_phys(buf);
	if (dma_descr->dma_addr & (dma_descr->alignment-1)){
		dma_descr->dma_offset = 
			dma_descr->alignment - 
			(dma_descr->dma_addr & (dma_descr->alignment-1));
		dma_descr->dma_virt = buf + dma_descr->dma_offset;
		dma_descr->dma_addr = virt_to_phys(dma_descrt->dma_virt));
	} else {
		dma_descr->dma_virt	= buf; 
		dma_descr->dma_offset	=0; 
	}
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA map is not defined!"
#endif
	dma_descr->dma_len = buflen;
	dma_descr->dma_map_len	= map_len;
	return;
}
static void sdla_busdma_unmap(void *phw, wan_dma_descr_t *dma_descr, int dir)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT_VOID(hw == NULL);
	SDLA_MAGIC_VOID(hw);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT_VOID(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;

	if (!wan_test_bit(SDLA_DMA_FLAG_READY, &dma_descr->flag)){
		DEBUG_DMA("%s: Internal Error: %s:%d: DMA is not ready!\n",
					hw->devname, __FUNCTION__,__LINE__);
		return;
	}
#if defined(__FreeBSD__)
	if (dma_descr->skb && dir == SDLA_DMA_POSTREAD){
		caddr_t	data = NULL;

		/* Apr 19, 2007 new line :
		** data = wan_skb_put(dma_descr->skb, dma_descr->dma_len);*/
		/* Nov 2, 2007 do not update anything in mbuf. The code 
		** who called me will update all length of mbuf */
		data = wan_skb_tail(dma_descr->skb);
		bcopy(dma_descr->dma_virt, data, dma_descr->dma_len);
	}
#elif defined(__LINUX__)
	if (dma_descr->dma_addr){
		dma_descr->dma_addr -= dma_descr->dma_offset;
		pci_unmap_single(	hwcard->pci_dev, 
					dma_descr->dma_addr, 
					dma_descr->dma_map_len, 
					dir);
	}
	dma_descr->dma_addr	= 0;
#elif defined(__OpenBSD__)
	dma_descr->dma_addr	= 0;
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA Unmap is not defined!"
#endif
	dma_descr->dma_len	= 0;
	dma_descr->dma_map_len	= 0;
	return;
}
static void 
sdla_busdma_sync(void *phw, wan_dma_descr_t *dma_descr, int ndescr, int single, int dir)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
#if defined(__FreeBSD__)
	int		cnt = 0;
#endif

	WAN_ASSERT_VOID(hw == NULL);
	SDLA_MAGIC_VOID(hw);
	if (!wan_test_bit(SDLA_DMA_FLAG_READY, &dma_descr->flag)){
		DEBUG_DMA("%s: Internal Error: %s:%d: DMA is not ready!\n",
					hw->devname, __FUNCTION__,__LINE__);
		return;
	}
#if defined(__FreeBSD__)
	for (cnt=0;cnt<ndescr;cnt++){
		bus_dmamap_sync(dma_descr->dmat, dma_descr->dmam, dir);	
		if (single) break;
		dma_descr++;
	}
#elif defined(__LINUX__)
#elif defined(__OpenBSD__)
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "BUSDMA Sync is not defined!"
#endif
	return;
}


static sdla_dma_addr_t sdla_pci_map_dma(void *phw, void *buf, int len, int ctrl)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;

#if defined(__LINUX__)
	return cpu_to_le32(pci_map_single(hwcard->pci_dev, buf, len, ctrl));
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
	return (sdla_dma_addr_t)NULL;
#else	
	return virt_to_phys(buf);
#endif
}

static int sdla_pci_unmap_dma(void *phw, sdla_dma_addr_t buf, int len, int ctrl)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	hwcard;
	sdlahw_cpu_t*	hwcpu;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;

#if defined(__LINUX__)
	pci_unmap_single(hwcard->pci_dev, buf, len, ctrl);
#endif
	return 0;
}
/*****************************************************************************/


int sdla_hw_fe_test_and_set_bit(void *phw, int value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	hwcard;
	sdlahw_cpu_t*	hwcpu;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;

#if defined(__WINDOWS__)
	{
		sdla_t *card = hw->p_sdla;
		DBG_FE_LOCK("%s():%s: bitNum: %i, &hwcard->fe_rw_flag: 0x%p, fe_rw_flag: 0x%X\n",
			__FUNCTION__, card->devname, value, &hwcard->fe_rw_flag, hwcard->fe_rw_flag);
	}
#endif
	return wan_test_and_set_bit(value, &hwcard->fe_rw_flag);
}

int sdla_hw_fe_test_bit(void *phw, int value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	hwcard;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;
#if defined(__WINDOWS__)
	{
		sdla_t *card = hw->p_sdla;
		DBG_FE_LOCK("%s():%s: bitNum: %i, &hwcard->fe_rw_flag: 0x%p, fe_rw_flag: 0x%X\n",
			__FUNCTION__, card->devname, value, &hwcard->fe_rw_flag, hwcard->fe_rw_flag);
	}
#endif
	return wan_test_bit(value, &hwcard->fe_rw_flag);
}   

int sdla_hw_fe_clear_bit(void *phw, int value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	hwcard;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;
	wan_clear_bit(value, &hwcard->fe_rw_flag);
#if defined(__WINDOWS__)
	{
		sdla_t *card = hw->p_sdla;
		DBG_FE_LOCK("%s():%s: bitNum: %i, &hwcard->fe_rw_flag: 0x%p, fe_rw_flag: 0x%X\n",
			__FUNCTION__, card->devname, value, &hwcard->fe_rw_flag, hwcard->fe_rw_flag);
	}
#endif
	return 0;
}

int sdla_hw_fe_set_bit(void *phw, int value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_card_t*	hwcard;
	sdlahw_cpu_t	*hwcpu;
	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;
	wan_set_bit(value, &hwcard->fe_rw_flag);
#if defined(__WINDOWS__)
	{
		sdla_t *card = hw->p_sdla;
		DBG_FE_LOCK("%s():%s: bitNum: %i, &hwcard->fe_rw_flag: 0x%p, fe_rw_flag: 0x%X\n",
			__FUNCTION__, card->devname, value, &hwcard->fe_rw_flag, hwcard->fe_rw_flag);
	}
#endif
	return 0;
}      


static int sdla_hw_read_cpld(void *phw, u16 off, u8 *data)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	u16		org_off;

	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;

	if (hwcpu->hwcard->adptr_subtype == AFT_SUBTYPE_NORMAL){
		switch(hwcpu->hwcard->adptr_type){
		case A101_ADPTR_1TE1:
		case A101_ADPTR_2TE1:
			off &= ~AFT_BIT_DEV_ADDR_CLEAR;
			off |= AFT_BIT_DEV_ADDR_CPLD;
			/* Save current original address */
			sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
			sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
			sdla_bus_read_1(hw, AFT_MCPU_INTERFACE, data);
			/* Restore the original address */
			sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
			break;
		case A104_ADPTR_4TE1:
			off &= ~AFT4_BIT_DEV_ADDR_CLEAR;
			off |= AFT4_BIT_DEV_ADDR_CPLD;
			/* Save current original address */
			sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
			WP_DELAY(5);
			sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
			WP_DELAY(5);
			sdla_bus_read_1(hw, AFT_MCPU_INTERFACE, data);
			/* Restore the original address */
			sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
			break;
		default:
			DEBUG_EVENT("%s: ERROR: Invalid read access to cpld (Normal)!\n",
						hw->devname);
			return -EINVAL;
		}
		
	}else if (hwcpu->hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
		switch(hwcpu->hwcard->core_id){
		case AFT_PMC_FE_CORE_ID:
			switch(hwcpu->hwcard->adptr_type){
			case A104_ADPTR_4TE1:
				off &= ~AFT4_BIT_DEV_ADDR_CLEAR;
				off |= AFT4_BIT_DEV_ADDR_CPLD;
				/* Save current original address */
				sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
				WP_DELAY(5);
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
				WP_DELAY(5);
				sdla_bus_read_1(hw, AFT_MCPU_INTERFACE, data);
				/* Restore the original address */
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
				break;
			default:
				DEBUG_EVENT("%s: ERROR: Invalid read access to cpld (PMC)!\n",
							hw->devname);
				return -EINVAL;
			}
			break;
		case AFT_DS_FE_CORE_ID:
			switch(hwcpu->hwcard->adptr_type){
			case A101_ADPTR_1TE1:
			case A101_ADPTR_2TE1:
			case A104_ADPTR_4TE1:
			case A108_ADPTR_8TE1:
				off &= ~AFT8_BIT_DEV_ADDR_CLEAR;
				off |= AFT8_BIT_DEV_ADDR_CPLD;
		
				/* Save current original address */
				sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
				WP_DELAY(5);
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
				WP_DELAY(5);
				sdla_bus_read_1(hw, AFT_MCPU_INTERFACE, data);
				/* Restore the original address */
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
				break;
			default:
				DEBUG_EVENT("%s: ERROR: Invalid read access to cpld (DS)!\n",
							hw->devname);
				return -EINVAL;
			}
			break;
		default:
			switch(hwcpu->hwcard->adptr_type){
			case A200_ADPTR_ANALOG:
			case A400_ADPTR_ANALOG:
			case AFT_ADPTR_ISDN:
				off &= ~AFT4_BIT_DEV_ADDR_CLEAR;
				off |= AFT4_BIT_DEV_ADDR_CPLD;
				/* Save current original address */
				sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
				WP_DELAY(5);
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
				WP_DELAY(5);
				sdla_bus_read_1(hw, AFT_MCPU_INTERFACE, data);
				/* Restore the original address */
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
				break;				
			case AFT_ADPTR_56K:

				off |= AFT56K_BIT_DEV_ADDR_CPLD; 

   				sdla_bus_write_2(hw, AFT56K_MCPU_INTERFACE_ADDR, off);
   				sdla_bus_read_1(hw, AFT56K_MCPU_INTERFACE, data);
				break;
			case AFT_ADPTR_2SERIAL_V35X21:
			case AFT_ADPTR_2SERIAL_RS232:
			case AFT_ADPTR_4SERIAL_V35X21:
			case AFT_ADPTR_4SERIAL_RS232:

				
				off &= ~AFT_SERIAL_BIT_DEV_ADDR_CLEAR;
				off |= AFT_SERIAL_BIT_DEV_ADDR_CPLD;
		
				/* Save current original address */
				sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
				WP_DELAY(5);
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
				WP_DELAY(5);
				sdla_bus_read_1(hw, AFT_MCPU_INTERFACE, data);
				/* Restore the original address */
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);


				break;
			default:
				DEBUG_EVENT("%s: ERROR: Invalid read access to cpld (SHARK)!\n",
						hw->devname);
				return -EINVAL;		
			}
			break;
		}
	}else{
		DEBUG_EVENT("%s: ERROR: Invalid read access to cpld!\n",
						hw->devname);
		return -EINVAL;	
	}
	return 0;
}

static int sdla_hw_write_cpld(void *phw, u16 off, u8 data)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;
	u16		org_off;

	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if (hwcard->adptr_subtype == AFT_SUBTYPE_NORMAL){
		switch(hwcard->adptr_type){
		case A101_ADPTR_1TE1:
		case A101_ADPTR_2TE1:
			off &= ~AFT_BIT_DEV_ADDR_CLEAR;
			off |= AFT_BIT_DEV_ADDR_CPLD;
			/* Save current original address */
			sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
			sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
			sdla_bus_write_1(hw, AFT_MCPU_INTERFACE, data);
			/* Restore the original address */
			sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
			break;
		case A104_ADPTR_4TE1:
			off &= ~AFT4_BIT_DEV_ADDR_CLEAR;
			off |= AFT4_BIT_DEV_ADDR_CPLD;
			/* Save current original address */
			sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
			WP_DELAY(5);
			sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
			WP_DELAY(5);
			sdla_bus_write_1(hw, AFT_MCPU_INTERFACE, data);
			/* Restore the original address */
			sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
			break;
		default:
			DEBUG_EVENT("%s: (line: %d) ERROR: Invalid write access to cpld!\n",
						hw->devname, __LINE__);
			return -EINVAL;
		}
		
	}else if (hwcard->adptr_subtype == AFT_SUBTYPE_SHARK){
		switch(hwcard->core_id){
		case AFT_PMC_FE_CORE_ID:
			switch(hwcard->adptr_type){
			case A104_ADPTR_4TE1:
				off &= ~AFT4_BIT_DEV_ADDR_CLEAR;
				off |= AFT4_BIT_DEV_ADDR_CPLD;
				/* Save current original address */
				sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
				WP_DELAY(5);
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
				WP_DELAY(5);
				sdla_bus_write_1(hw, AFT_MCPU_INTERFACE, data);
				/* Restore the original address */
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
				break;
			default:
				DEBUG_EVENT("%s: (%d)ERROR: Invalid write access to cpld!\n",
							hw->devname, __LINE__);
				return -EINVAL;
			}
			break;

		case AFT_DS_FE_CORE_ID:
			switch(hwcard->adptr_type){
			case A101_ADPTR_1TE1:
			case A101_ADPTR_2TE1:
		        case A104_ADPTR_4TE1:
			case A108_ADPTR_8TE1:
				off &= ~AFT8_BIT_DEV_ADDR_CLEAR;
				off |= AFT8_BIT_DEV_ADDR_CPLD;
				/* Save current original address */
				sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
				WP_DELAY(5);
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
				WP_DELAY(5);
				sdla_bus_write_1(hw, AFT_MCPU_INTERFACE, data);
				/* Restore the original address */
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
				break;

			default:
				DEBUG_EVENT("%s: (1)ERROR: Invalid write access to cpld!\n",
							hw->devname);
				return -EINVAL;
			}
			break;

		default:
			switch(hwcard->adptr_type){
			case A200_ADPTR_ANALOG:
			case A400_ADPTR_ANALOG:
			case AFT_ADPTR_ISDN://????
				off &= ~AFT4_BIT_DEV_ADDR_CLEAR;
				off |= AFT4_BIT_DEV_ADDR_CPLD;
				/* Save current original address */
				sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
				WP_DELAY(5);
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
				WP_DELAY(5);
				sdla_bus_write_1(hw, AFT_MCPU_INTERFACE, data);
				/* Restore the original address */
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
				break;		
			case AFT_ADPTR_56K:
				off |= AFT56K_BIT_DEV_ADDR_CPLD; 

				sdla_bus_write_2(hw, AFT56K_MCPU_INTERFACE_ADDR, off);
   				sdla_bus_write_1(hw, AFT56K_MCPU_INTERFACE, data);
				break;
			case AFT_ADPTR_2SERIAL_V35X21:
			case AFT_ADPTR_2SERIAL_RS232:
			case AFT_ADPTR_4SERIAL_V35X21:
			case AFT_ADPTR_4SERIAL_RS232:

				off &= ~AFT_SERIAL_BIT_DEV_ADDR_CLEAR;
				off |= AFT_SERIAL_BIT_DEV_ADDR_CPLD;
				/* Save current original address */
				sdla_bus_read_2(hw, AFT_MCPU_INTERFACE_ADDR, &org_off);
				WP_DELAY(5);
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, off);
				WP_DELAY(5);
				sdla_bus_write_1(hw, AFT_MCPU_INTERFACE, data);
				/* Restore the original address */
				sdla_bus_write_2(hw, AFT_MCPU_INTERFACE_ADDR, org_off);
				break;

			default:
				DEBUG_EVENT("%s: (line: %d)ERROR: Invalid write access to cpld!\n",
						hw->devname, __LINE__);
				return -EINVAL;		
			}
			break;
		}
	}else{
		DEBUG_EVENT("%s: (line: %d)ERROR: Invalid write access to cpld!\n",
						hw->devname, __LINE__);
		return -EINVAL;	
	}
	return 0;
}

static int sdla_is_pciexpress(void *phw)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	sdlahw_card_t	*hwcard;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);
	hwcpu = hw->hwcpu;
	hwcard = hwcpu->hwcard;

	if (hwcard->pci_bridge_dev == NULL) return 0;
	return 1;
}


static int sdla_get_hwec_index(void *phw)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;

	WAN_ASSERT(hw == NULL);
	SDLA_MAGIC(hw);
	WAN_ASSERT(hw->hwcpu == NULL);
	WAN_ASSERT(hw->hwcpu->hwcard == NULL);

	if (hw->hwcpu->hwcard->hwec_chan_no == 0) return -EINVAL;
	return hw->hwcpu->hwcard->hwec_ind;
}

