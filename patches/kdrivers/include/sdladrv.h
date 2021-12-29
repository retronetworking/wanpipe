/*
 * Copyright (c) 1997, 1998, 1999
 *	Alex Feldman <al.feldman@sangoma.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Alex Feldman.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Alex Feldman AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Alex Feldman OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	$Id: sdladrv.h,v 1.97 2008-04-21 19:03:31 sangoma Exp $
 */

/*****************************************************************************
 * sdladrv.h	SDLA Support Module.  Kernel API Definitions.
 *
 * Author:	Alex Feldman 	<al.feldman@sangoma.com>
 *
 * ============================================================================
 * Nov 27,  2007 David Rokhvarg	Implemented functions/definitions for
 *                              Sangoma MS Windows Driver and API.
 *
 * Dec 06, 1999 Alex Feldman    Updated to FreeBSD 3.2
 *
 * Dec 06, 1995	Gene Kozin	Initial version.
 *****************************************************************************
 */

#ifndef	_SDLADRV_H
#   define	_SDLADRV_H
#ifdef __SDLADRV__
# define EXTERN 
#else
# define EXTERN extern
#endif

#if defined(__LINUX__)
#ifdef CONFIG_ISA
# define WAN_ISA_SUPPORT
#else
# undef WAN_ISA_SUPPORT
#endif
#endif

/*
******************************************************************
**			I N C L U D E S				**	
******************************************************************
*/


#if defined(__LINUX__)
# include <linux/version.h>
# include <linux/wanpipe_kernel.h>
# include <linux/sdlasfm.h>
#elif defined(__FreeBSD__)
# if defined(__SDLADRV__)
#  if defined(SDLA_AUTO_PROBE)
#   include <sdla_bsd.h>
#  else
#   include <i386/isa/sdla_dev.h>
#  endif
# endif
#elif defined(__OpenBSD__) || defined(__NetBSD__)
# ifdef __SDLADRV__
#  include <dev/ic/sdla_dev.h>
# endif
#elif defined(__WINDOWS__)
# include <wanpipe_kernel.h>
#endif

/*
******************************************************************
**			D E F I N E S				**	
******************************************************************
*/

#define	SDLADRV_MAGIC		0x414C4453L	/* signature: 'SDLA' reversed */

#define SDLADRV_MODE_WANPIPE	0
#define SDLADRV_MODE_LIMITED	1

#define SDLADRV_MAJOR_VER 	2
#define SDLADRV_MINOR_VER 	1

#define	SDLA_MAXIORANGE		4	/* maximum I/O port range */
#define	SDLA_WINDOWSIZE		0x2000	/* default dual-port memory window size */

/* */
#define SDLA_MEMBASE			0x01
#define SDLA_MEMEND			0x02
#define SDLA_MEMSIZE			0x03
#define SDLA_IRQ			0x04
#define SDLA_ADAPTERTYPE		0x05
#define SDLA_CPU			0x06
#define SDLA_SLOT			0x07
#define SDLA_IOPORT			0x08
#define SDLA_IORANGE			0x09
#define SDLA_CARDTYPE			0x0A
#define SDLA_DMATAG			0x0B
#define SDLA_MEMORY			0x0C
#define SDLA_PCIEXTRAVER		0x0D
#define SDLA_HWTYPE			0x0E
#define SDLA_BUS			0x0F
#define SDLA_BASEADDR			0x10
#define SDLA_COREREV			0x11
#define SDLA_HWCPU_USEDCNT		0x12
#define SDLA_ADAPTERSUBTYPE		0x13
#define SDLA_HWEC_NO			0x14
#define SDLA_COREID			0x15
#define SDLA_PCIEXPRESS			0x16
#define SDLA_PORTS_NO			0x17
#define SDLA_HWPORTUSED			0x18
#define SDLA_HWPORTREG			0x19
#define SDLA_PORT_MAP			0x1A
#define SDLA_RECOVERY_CLOCK_FLAG	0x1B
#define SDLA_HWPORTREGMAP		0x1C

#define SDLA_MAX_CPUS		2

/* Serial card - 2, A104 - 4, A108 - 8, A200/A400 - 1, A500-24  */
#define SDLA_MAX_HWDEVS		32
#define SDLA_MAX_HWPORTS	32

/* Status values */
#define SDLA_MEM_RESERVED	0x0001
#define SDLA_MEM_MAPPED		0x0002
#define SDLA_IO_MAPPED		0x0004
#define SDLA_PCI_ENABLE		0x0008

#define SDLA_NAME_SIZE		20

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# define PCI_DMA_FROMDEVICE	1
# define PCI_DMA_TODEVICE	2
#endif

#define SDLA_MAGIC(hw)		WAN_ASSERT((hw)->magic != SDLADRV_MAGIC)
#define SDLA_MAGIC_RC(hw,rc)	WAN_ASSERT_RC((hw)->magic != SDLADRV_MAGIC, (rc))
#define SDLA_MAGIC_VOID(hw)	WAN_ASSERT_VOID((hw)->magic != SDLADRV_MAGIC)

/*
******************************************************************
**			T Y P E D E F S				**	
******************************************************************
*/
#if defined(__FreeBSD__)
typedef void*			sdla_mem_handle_t;
# if (__FreeBSD_version < 400000)
typedef	bus_addr_t 		sdla_base_addr_t;
typedef bus_addr_t		sdla_dma_addr_t;
typedef	pcici_t			sdla_pci_dev_t;
# else
typedef	bus_addr_t		sdla_base_addr_t;
typedef bus_addr_t		sdla_dma_addr_t;
typedef	struct device*		sdla_pci_dev_t;
# endif
#elif defined(__OpenBSD__)
typedef bus_space_handle_t	sdla_mem_handle_t;
typedef	bus_addr_t 		sdla_base_addr_t;
typedef bus_addr_t		sdla_dma_addr_t;
typedef	struct pci_attach_args* sdla_pci_dev_t;
#elif defined(__NetBSD__)
typedef bus_space_handle_t	sdla_mem_handle_t;
typedef	bus_addr_t 		sdla_base_addr_t;
typedef bus_addr_t		sdla_dma_addr_t;
typedef	struct pci_attach_args* sdla_pci_dev_t;
#elif defined(__LINUX__)
#if defined(__iomem)
typedef void __iomem*		sdla_mem_handle_t;
#else
typedef void *		        sdla_mem_handle_t;
#endif
typedef	unsigned long 		sdla_base_addr_t;
typedef dma_addr_t		sdla_dma_addr_t;
typedef struct pci_dev*		sdla_pci_dev_t;
#elif defined(__WINDOWS__)
typedef void *		        sdla_mem_handle_t;
typedef	unsigned long 		sdla_base_addr_t;
/* Dma addresses are 32-bits wide!!! */
typedef u32			dma_addr_t;
typedef dma_addr_t		sdla_dma_addr_t;
typedef struct pci_dev*		sdla_pci_dev_t;
#else
# warning "Undefined types sdla_mem_handle_t/sdla_base_addr_t!"
#endif

/*
******************************************************************
**			M A C R O S				**
******************************************************************
*/
#if defined(__FreeBSD__)
# if defined(__i386__)
#  define virt_to_phys(x) kvtop((caddr_t)x)
# else
#  define virt_to_phys(x) (sdla_dma_addr_t)(x)
# endif
# define phys_to_virt(x) (sdla_mem_handle_t)x
#elif defined(__OpenBSD__)
# define virt_to_phys(x) kvtop((caddr_t)x)
# define phys_to_virt(x) (bus_space_handle_t)x
#elif defined(__NetBSD__)
# define virt_to_phys(x) kvtop((caddr_t)x)
# define phys_to_virt(x) (bus_space_handle_t)x
#elif defined(__LINUX__) 
#elif defined(__WINDOWS__)
# define phys_to_virt(x) NULL
#else
# warning "Undefined virt_to_phys/phys_to_virt macros!"
#endif

#define IS_SDLA_PCI(hw)		(hw->type == SDLA_S514 || hw->type == SDLA_ADSL)
#define IS_SDLA_ISA(hw)		(hw->type == SDLA_S508)

#define WAN_HWCALL(func, x)						\
	if (card->hw_iface.func){					\
		card->hw_iface.func x;					\
	}else{								\
		DEBUG_EVENT("%s: Unknown hw function pointer (%s:%d)!\n",\
				card->devname, __FUNCTION__,__LINE__);	\
	}
			
/*
******************************************************************
**			S T R U C T U R E S			**	
******************************************************************
*/
/* DMA structure */
#define SDLA_DMA_FLAG_READY	0
#define SDLA_DMA_FLAG_INIT	1
#define SDLA_DMA_FLAG_ALLOC	2

#if defined(__FreeBSD__)
# define SDLA_DMA_POSTREAD	BUS_DMASYNC_POSTREAD
# define SDLA_DMA_PREREAD	BUS_DMASYNC_PREREAD
# define SDLA_DMA_POSTWRITE	BUS_DMASYNC_POSTWRITE
# define SDLA_DMA_PREWRITE	BUS_DMASYNC_PREWRITE
#elif defined(__LINUX__)
# define SDLA_DMA_POSTREAD	PCI_DMA_FROMDEVICE 
# define SDLA_DMA_PREREAD	PCI_DMA_FROMDEVICE
# define SDLA_DMA_POSTWRITE	PCI_DMA_TODEVICE
# define SDLA_DMA_PREWRITE	PCI_DMA_TODEVICE
#else
# define SDLA_DMA_POSTREAD	1
# define SDLA_DMA_PREREAD	2
# define SDLA_DMA_POSTWRITE	3
# define SDLA_DMA_PREWRITE	4
#endif
typedef struct wan_dma_descr_
{
	unsigned long		init;
	unsigned long		flag;
	u32			alignment;
	u32			max_len;

	sdla_dma_addr_t		dma_addr;	/* physical address */
	u32			dma_len;	/* total dma data len */
	u32			dma_map_len;	/* actual dma mapped len (address range) */
	netskb_t 		*skb;
	u32			index;

	u32			dma_descr;
	u32			len_align;
	u32			reg;

	u8			pkt_error;
	void*			dma_virt;
	u32			dma_offset;
	u32			dma_toggle;
#if defined(__FreeBSD__)
	bus_dma_tag_t		dmat;
	bus_dmamap_t		dmam;
#elif defined(__OpenBSD__)
	bus_dma_tag_t		dmat;
	bus_dma_segment_t	dmaseg;
	int			rsegs;
#elif defined(__NetBSD__)
	bus_dma_tag_t		dmat;
	bus_dma_segment_t	dmaseg;
	int			rsegs;
#elif defined(__WINDOWS__)
	void*			vAddr;
	PHYSICAL_ADDRESS	physicalAddr;
#endif
} wan_dma_descr_t;


typedef struct sdla_hw_probe
{
	int				internal_used;
	int 				used;
	unsigned char			hw_info[100];
	unsigned char			hw_info_verbose[500];
	unsigned char			hw_info_dump[1000];
	WAN_LIST_ENTRY(sdla_hw_probe)	next;
} sdla_hw_probe_t;


/*
 * This structure keeps common parameters per physical card.
 */
typedef struct sdlahw_card {
	int 				internal_used;
	unsigned int		hw_type;	/* ISA/PCI */
	unsigned int		type;		/* S50x/S514/ADSL/SDLA_AFT */
	unsigned char		cfg_type;	/* Config card type WANOPT_XXX */
	unsigned int		adptr_type;	/* Adapter type (subvendor ID) */
	unsigned char		adptr_subtype;	/* Adapter Subtype (Normal|Shark) */
	unsigned char		adptr_name[SDLA_NAME_SIZE];
	unsigned char		core_id;	/* SubSystem ID [0..7] */
	unsigned char		core_rev;	/* SubSystem ID [8..15] */
	unsigned char 		pci_extra_ver;
	unsigned int		slot_no;
	unsigned int		bus_no;
	unsigned int		ioport;
	unsigned int		recovery_clock_flag; /* 1 - already used, 0 - not used */
#if defined(__LINUX__)
	sdla_pci_dev_t		pci_dev;	/* PCI config header info */
#else
#if defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_tag_t		iot;		/* adapter I/O port tag */
	bus_space_tag_t		memt;
#endif
	sdla_pci_dev_t		pci_dev;	/* PCI config header info */
#endif
	sdla_pci_dev_t		pci_bridge_dev;	/* PCI Bridge config header info */
	unsigned int		pci_bridge_slot;/* PCI Bridge slot number */
	unsigned int		pci_bridge_bus;	/* PCI Bridge bus number */
	wan_spinlock_t		pcard_lock;	/* lock per physical card for FE */
	wan_spinlock_t		pcard_ec_lock;	/* lock per physical card for EC */
	wan_smp_flag_t		fe_rw_flag;
	unsigned char		adptr_security;	/* Adapter security (AFT cards) */
	u16			hwec_chan_no;	/* max hwec channels number */
	int			hwec_ind;	/* hwec index */
	WAN_LIST_ENTRY(sdlahw_card)	next;
	unsigned char		cpld_rev;
} sdlahw_card_t;


/*----------------------------------------------------------------------------
 * Adapter hardware configuration. Pointer to this structure is passed to all
 * APIs.
 */
struct sdlahw_;
typedef struct sdlahw_cpu
{
	int			internal_used;
	int			used;
	u16			status;
	unsigned 		fwid;		/* firmware ID */
#if defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_handle_t	ioh;		/* adapter I/O port handle */
	bus_dma_tag_t		dmat;
#endif
	int 			irq;		/* interrupt request level */
#if (defined(__FreeBSD__) && __FreeBSD_version >= 450000)
	void*			irqh[SDLA_MAX_HWDEVS];
#endif
	unsigned int		cpu_no;		/* PCI CPU Number */
	char 			auto_pci_cfg;	/* Auto PCI configuration */
	sdla_mem_handle_t	dpmbase;	/* dual-port memory base */
	sdla_base_addr_t 	mem_base_addr;
	unsigned 		dpmsize;	/* dual-port memory size */
	unsigned 		pclk;		/* CPU clock rate, kHz */
	unsigned long 		memory;		/* memory size */
	sdla_mem_handle_t	vector;		/* local offset of the DPM window */
	unsigned 		io_range;	/* I/O port range */
	unsigned char 		regs[SDLA_MAXIORANGE];	/* was written to registers */

	unsigned 		reserved[5];
	//unsigned char		hw_info[100];

	/* */
	unsigned		magic;
# if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	void			*sdla_dev;		/* used only for FreeBSD/OpenBSD */
# endif
	u16			configured;

	int			max_ports;	/* number of ports */

	u_int32_t		port_map;	/* installed modules for A200/A400/ISDN */
	int			reg_port[SDLA_MAX_HWDEVS];

	u_int32_t		reg_port_map;	/* configured port */
	struct sdlahw_dev	*hwdev[SDLA_MAX_HWDEVS];
		
	void			*port_ptr_isr_array[SDLA_MAX_HWDEVS];

	sdlahw_card_t		*hwcard;
	WAN_LIST_ENTRY(sdlahw_cpu)	next;
#if defined(__WINDOWS__)
	PHYSICAL_ADDRESS unmapped_memory_base;/* memory BEFORE it was mapped in to virtual space */
	ulong_t	unmapped_memory_length;/* actual memory aperture size */
#endif
} sdlahw_cpu_t;

typedef struct sdlahw_port_
{
	int			used;
	char			*devname;
	sdla_hw_probe_t		*hwprobe;
} sdlahw_port_t;

typedef struct sdlahw_dev
{
	int			internal_used;
	int			used;
	char			*devname;
	unsigned		magic;
	unsigned int		port_no;	/* Port number per CPU (line) */
//	sdla_hw_probe_t		*hwprobe;
	sdlahw_cpu_t*		hwcpu;
	int			max_port_no;
	sdlahw_port_t		hwport[SDLA_MAX_HWPORTS];

	WAN_LIST_ENTRY(sdlahw_dev)	next;
#if defined(__WINDOWS__)
	void *level1_dev_pdx;
#endif
} sdlahw_t;


typedef struct sdlahw_iface
{
	int		(*setup)(void*,wandev_conf_t*);
	int		(*load)(void*,void*, unsigned);
	int		(*hw_down)(void*);
	int 		(*start)(sdlahw_t* hw, unsigned addr);
	int		(*hw_halt)(void*);
	int		(*intack)(void*, uint32_t);
	int		(*read_int_stat)(void*, uint32_t*);
	int		(*mapmem)(void*, unsigned long);
	int		(*check_mismatch)(void*, unsigned char);
	int 		(*getcfg)(void*, int, void*);
	int 		(*setcfg)(void*, int, void*);
	int		(*isa_read_1)(void* hw, unsigned int offset, u8*);
	int		(*isa_write_1)(void* hw, unsigned int offset, u8);
	int 		(*io_write_1)(void* hw, unsigned int offset, u8);
	int		(*io_read_1)(void* hw, unsigned int offset, u8*);
	int		(*bus_write_1)(void* hw, unsigned int offset, u8);
	int		(*bus_write_2)(void* hw, unsigned int offset, u16);
	int		(*bus_write_4)(void* hw, unsigned int offset, u32);
	int		(*bus_read_1)(void* hw, unsigned int offset, u8*);
	int		(*bus_read_2)(void* hw, unsigned int offset, u16*);
	int		(*bus_read_4)(void* hw, unsigned int offset, u32*);
	int 		(*pci_write_config_byte)(void* hw, int reg, u8 value);
	int 		(*pci_write_config_word)(void* hw, int reg, u16 value);
	int 		(*pci_write_config_dword)(void* hw, int reg, u32 value);
	int		(*pci_read_config_byte)(void* hw, int reg, u8* value);
	int		(*pci_read_config_word)(void* hw, int reg, u16* value);
	int		(*pci_read_config_dword)(void* hw, int reg, u32* value);
	int 		(*pci_bridge_write_config_dword)(void* hw, int reg, u32 value);
	int 		(*pci_bridge_write_config_byte)(void* hw, int reg, u_int8_t value);
	int		(*pci_bridge_read_config_dword)(void* hw, int reg, u32* value);
	int		(*pci_bridge_read_config_byte)(void* hw, int reg, u_int8_t* value);
	int		(*cmd)(void* phw, unsigned long offset, wan_mbox_t* mbox);
	int		(*peek)(void*,unsigned long, void*,unsigned);
	int		(*poke)(void*,unsigned long, void*,unsigned);
	int		(*poke_byte)(void*,unsigned long, u8);
	int		(*set_bit)(void*,unsigned long, u8);
	int		(*clear_bit)(void*,unsigned long,u8);
	int		(*set_intrhand)(void*, void (*isr_func)(void*), void*,int);
	int		(*restore_intrhand)(void*,int);
	int		(*is_te1)(void*);
	int		(*is_56k)(void*);
	int		(*get_hwcard)(void*, void**);
	int		(*get_hwprobe)(void*, int comm_port, void**);
	int 		(*hw_unlock)(void *phw, wan_smp_flag_t *flag);
	int 		(*hw_lock)(void *phw, wan_smp_flag_t *flag);
	int 		(*hw_ec_unlock)(void *phw, wan_smp_flag_t *flag);
	int 		(*hw_ec_lock)(void *phw, wan_smp_flag_t *flag);
	int 		(*hw_ec_trylock)(void *phw, wan_smp_flag_t *flag);
	int 		(*hw_same)(void *phw1, void *phw2);
	int 		(*hwcpu_same)(void *phw1, void *phw2);
	int 		(*fe_test_and_set_bit)(void *phw, int value);
	int 		(*fe_test_bit)(void *phw,int value);
	int 		(*fe_set_bit)(void *phw,int value);
	int 		(*fe_clear_bit)(void *phw,int value);
	sdla_dma_addr_t (*pci_map_dma)(void *phw, void *buf, int len, int ctrl);
	int    		(*pci_unmap_dma)(void *phw, sdla_dma_addr_t buf, int len, int ctrl);
	int		(*read_cpld)(void *phw, u16, u8*);
	int		(*write_cpld)(void *phw, u16, u8);
	int		(*fe_write)(void*, ...);
	u_int8_t	(*fe_read)(void*, ...);
	u_int8_t	(*__fe_read)(void*, ...);
	wan_dma_descr_t* (*busdma_descr_alloc)(void *phw, int);
	void		(*busdma_descr_free)(void *phw, wan_dma_descr_t*);
	int		(*busdma_tag_create)(void *phw, wan_dma_descr_t*, u32, u32, int);
	int		(*busdma_tag_destroy)(void *phw, wan_dma_descr_t*, int);
	int		(*busdma_create)(void *phw, wan_dma_descr_t*);
	int		(*busdma_destroy)(void *phw, wan_dma_descr_t*);
	int		(*busdma_alloc)(void *phw, wan_dma_descr_t*);
	void		(*busdma_free)(void *phw, wan_dma_descr_t*);
	int		(*busdma_load)(void *phw, wan_dma_descr_t*, u32);
	void		(*busdma_unload)(void *phw, wan_dma_descr_t*);
	void		(*busdma_map)(void *phw, wan_dma_descr_t*, void *buf, int len, int map_len, int dir);
	void		(*busdma_unmap)(void *phw, wan_dma_descr_t*, int dir);
	void		(*busdma_sync)(void *phw, wan_dma_descr_t*, int ndescr, int single, int dir);
	char*		(*hwec_name)(void *phw);
	int		(*get_hwec_index)(void *phw);
	void		(*reset_fe)(void*);
#if defined(__WINDOWS__)
	sdlahw_t *hw;
#endif
} sdlahw_iface_t;

typedef struct sdla_hw_type_cnt
{
	unsigned char s508_adapters;
	unsigned char s514x_adapters;
	unsigned char s518_adapters;
	unsigned char aft101_adapters;
	unsigned char aft104_adapters;
	unsigned char aft300_adapters;
	unsigned char aft200_adapters;
	unsigned char aft108_adapters;
	unsigned char aft_isdn_adapters;
	unsigned char aft_56k_adapters;
	unsigned char aft_serial_adapters;
	unsigned char aft_a600_adapters;	
	unsigned char aft_x_adapters;
}sdla_hw_type_cnt_t;


/****** Function Prototypes *************************************************/

EXTERN int sdladrv_hw_mode(int);
EXTERN unsigned int sdla_hw_probe(void);
EXTERN unsigned int sdla_hw_bridge_probe(void);
EXTERN void *sdla_get_hw_probe (void);
EXTERN void *sdla_get_hw_adptr_cnt(void);
EXTERN void* sdla_register	(sdlahw_iface_t* hw_iface, wandev_conf_t*,char*);
EXTERN int sdla_unregister	(void**, char*);

#ifdef __SDLADRV__

static __inline unsigned int sdla_get_pci_bus(sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	return pci_get_bus(hwcard->pci_dev);
# else
	return hwcard->pci_dev->bus;
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	return hwcard->pci_dev->pa_bus;
#elif defined(__LINUX__)
	return ((struct pci_bus*)hwcard->pci_dev->bus)->number;
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "sdla_get_pci_bus: Not supported yet!"
#endif
	return 0;
}

static __inline unsigned int sdla_get_pci_slot(sdlahw_t* hw)
{
	sdlahw_card_t	*hwcard;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	hwcard = hwcpu->hwcard;
#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	return pci_get_slot(hwcard->pci_dev);
# else
	return hwcard->pci_dev->slot;
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	return hwcard->pci_dev->pa_device;
#elif defined(__LINUX__)
	return ((hwcard->pci_dev->devfn >> 3) & PCI_DEV_SLOT_MASK);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "sdla_get_pci_slot: Not supported yet!"
#endif
	return 0;
}

static __inline int 
sdla_bus_space_map(sdlahw_t* hw, int reg, int size, sdla_mem_handle_t* handle)
{
	sdlahw_cpu_t	*hwcpu;
	int		err = 0;
	
	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
#if defined(__FreeBSD__)
	*handle = (sdla_mem_handle_t)pmap_mapdev(hwcpu->mem_base_addr + reg, size);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	err = bus_space_map(hwcpu->hwcard->memt, 
			    hwcpu->mem_base_addr+reg, 
			    size, 
			    0, 
			    handle);
#elif defined(__LINUX__)
	*handle = (sdla_mem_handle_t)ioremap(hwcpu->mem_base_addr+reg, size);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
	/*
	*handle = (sdla_mem_handle_t)MmMapIoSpace(
		((PHYSICAL_ADDRESS)(hwcpu->mem_base_addr+reg)), (ULONG)size, MmNonCached);
	if(*handle == NULL){
		err = 1;
	}
	*/
#else
# warning "sdla_bus_space_map: Not supported yet!"
#endif
	return err;

}

static __inline void 
sdla_bus_space_unmap(sdlahw_t* hw, sdla_mem_handle_t offset, int size)
{
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT_VOID(hw == NULL);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT_VOID(hwcpu->hwcard == NULL);
#if defined(__FreeBSD__)
	int i = 0;
	for (i = 0; i < size; i += PAGE_SIZE){
	    pmap_kremove((vm_offset_t)offset + i);
	}
	kmem_free(kernel_map, (vm_offset_t)offset, size);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_unmap(hwcpu->hwcard->memt, offset, size);
#elif defined(__LINUX__)
	iounmap((void*)offset);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
	/*MmUnmapIoSpace((void*)offset, size);*/
#else
# warning "sdla_bus_space_unmap: Not supported yet!"
#endif

}


static __inline void 
sdla_bus_set_region_1(sdlahw_t* hw, unsigned int offset, unsigned char val, unsigned int cnt)
{
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT_VOID(hw == NULL);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT_VOID(hwcpu->hwcard == NULL);
#if defined(__FreeBSD__)
	return;
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_set_region_1(hwcpu->hwcard->memt, hw->dpmbase, offset, val, cnt);
#elif defined(__LINUX__)
	wp_memset_io(hwcpu->dpmbase + offset, val, cnt);
#elif defined(__WINDOWS__)
	wp_memset_io(((u8*)hwcpu->dpmbase) + offset, val, cnt);
#else
# warning "sdla_bus_set_region_1: Not supported yet!"
#endif
}

static __inline int
sdla_request_mem_region(sdlahw_t* hw, sdla_base_addr_t base_addr, unsigned int len, unsigned char* name, void** pres)
{
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	/* We don't need for BSD */
	return 0;
#elif defined(__LINUX__)
	void*	resource;
	if ((resource = request_mem_region(base_addr, len, name)) == NULL){
		return -EINVAL;
	}
	*pres = resource; 
	return 0;
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
	return 0;
#else
# warning "sdla_request_mem_region: Not supported yet!"
	return 0;
#endif
}

static __inline void 
sdla_release_mem_region(sdlahw_t* hw, sdla_base_addr_t base_addr, unsigned int len)
{
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	return;
#elif defined(__LINUX__)
	release_mem_region(base_addr, len);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "sdla_release_mem_region: Not supported yet!"
#endif
}

static __inline int 
sdla_pci_enable_device(sdlahw_t* hw)
{
	sdlahw_card_t*	card;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
#if defined(__FreeBSD__)
	return pci_enable_io(card->pci_dev, SYS_RES_MEMORY);
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	return 0;
#elif defined(__LINUX__)
	return pci_enable_device(card->pci_dev);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
	return 0;
#else
# warning "sdla_release_mem_region: Not supported yet!"
	return -EINVAL;
#endif
}

static __inline int 
sdla_pci_disable_device(sdlahw_t* hw)
{
	sdlahw_card_t*	card;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
#if defined(__FreeBSD__)
	return pci_disable_io(card->pci_dev, SYS_RES_MEMORY);
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	return 0;
#elif defined(__LINUX__)
	pci_disable_device(card->pci_dev);
	return 0;
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
	return 0;
#else
# warning "sdla_release_mem_region: Not supported yet!"
	return -EINVAL;
#endif
}



static __inline void 
sdla_pci_set_master(sdlahw_t* hw)
{
	sdlahw_card_t*	card;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT1(hw == NULL);
	WAN_ASSERT1(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT1(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;
#if defined(__FreeBSD__)
	pci_enable_busmaster(card->pci_dev);
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	return;
#elif defined(__LINUX__)
	pci_set_master(card->pci_dev);
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "sdla_pci_set_master: Not supported yet!"
#endif
}


static __inline void
sdla_get_pci_base_resource_addr(sdlahw_t* hw, int pci_offset, sdla_base_addr_t *base_addr)
{
	sdlahw_card_t*	card;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT1(hw == NULL);
	WAN_ASSERT1(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT1(hwcpu->hwcard == NULL);
	card = hwcpu->hwcard;

#if defined(__FreeBSD__)
# if (__FreeBSD_version > 400000)
	*base_addr = (sdla_base_addr_t)pci_read_config(card->pci_dev, pci_offset, 4);
# else
	*base_addr = (sdla_base_addr_t)ci_cfgread(card->pci_dev, pci_offset, 4);
# endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	*base_addr = (sdla_base_addr_t)pci_conf_read(card->pci_dev->pa_pc, card->pci_dev->pa_tag, pci_offset);
#elif defined(__LINUX__)
	if (pci_offset == PCI_IO_BASE_DWORD){
		*base_addr = (sdla_base_addr_t)pci_resource_start(card->pci_dev,0);
	}else{
		*base_addr = (sdla_base_addr_t)pci_resource_start(card->pci_dev,1);
	}
#elif defined(__WINDOWS__)
	FUNC_NOT_IMPL
#else
# warning "sdla_get_pci_base_resource_addr: Not supported yet!"
#endif
}

#endif /* __SDLADRV__ */


static __inline int __sdla_bus_read_4(void* phw, unsigned int offset, u32* value)
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;
	unsigned int	retry=3;

	WAN_ASSERT2(hw == NULL, 0);
	WAN_ASSERT2(hw->hwcpu == NULL, 0);
	hwcpu = hw->hwcpu;
	WAN_ASSERT2(hwcpu->hwcard == NULL, 0);
	WAN_ASSERT2(hwcpu->dpmbase == 0, 0);	
	SDLA_MAGIC(hwcpu);

	if (!(hwcpu->status & SDLA_MEM_MAPPED)) return 0;

	do {

#if defined(__FreeBSD__)
	*value = readl(((u8*)hw->hwcpu->dpmbase + offset));
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	*value = bus_space_read_4(hw->hwcard->memt, hwcpu->hwcpu->dpmbase, offset); 
#elif defined(__LINUX__)
	*value = wp_readl((unsigned char*)hw->hwcpu->dpmbase + offset);
#elif defined(__WINDOWS__)
	*value = READ_REGISTER_ULONG((PULONG)((PUCHAR)hw->hwcpu->dpmbase + offset));
#else
	*value = 0;
# warning "__sdla_bus_read_4: Not supported yet!"
#endif

		if (offset == 0x40 && *value == (u32)-1) {
			if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s:%d: wanpipe PCI Error: Illegal Register read: 0x%04X = 0x%08X\n",
				__FUNCTION__,__LINE__,offset,*value);
			}
		} else {
			/* only check for register 0x40 */
			break;
		}

	}while(--retry);

	return 0;
}

static __inline int __sdla_bus_write_4(void* phw, unsigned int offset, u32 value) 
{
	sdlahw_t*	hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT(hw == NULL);
	WAN_ASSERT(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	WAN_ASSERT(hwcpu->hwcard == NULL);	
	SDLA_MAGIC(hwcpu);
	if (!(hwcpu->status & SDLA_MEM_MAPPED)) return 0;

#if defined(__FreeBSD__)
	writel(((u8*)hwcpu->dpmbase + offset), value);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	bus_space_write_4(hwcpu->hwcard->memt, hwcpu->dpmbase, offset, value);
#elif defined(__LINUX__)
	wp_writel(value,(u8*)hwcpu->dpmbase + offset);
#elif defined(__WINDOWS__)
	WRITE_REGISTER_ULONG((PULONG)((PUCHAR)hwcpu->dpmbase + offset), value);
#else
# warning "__sdla_bus_write_4: Not supported yet!"
#endif
	return 0;
}


static __inline u32 SDLA_REG_OFF(sdlahw_card_t	*hwcard, u32 reg)
{
	if (hwcard == NULL) {
		DEBUG_EVENT("sdladrv: Critical error: hw or hw->cpu is NULL\n");
		return reg;
	}
	
	if (hwcard->adptr_type == AFT_ADPTR_A600) {
		if (reg < 0x100) {
			return (reg+0x1000);
		} else {
			return (reg+0x2000);
		}
	}
	return reg;
}

static __inline int __sdla_is_same_hwcard(void* phw1, void *phw2)
{
	sdlahw_cpu_t	*hwcpu1, *hwcpu2;

	WAN_ASSERT_RC(phw1 == NULL, 0);
	WAN_ASSERT_RC(phw2 == NULL, 0);
	WAN_ASSERT_RC(((sdlahw_t*)phw1)->hwcpu == NULL, 0);
	WAN_ASSERT_RC(((sdlahw_t*)phw2)->hwcpu == NULL, 0);
	hwcpu1 = ((sdlahw_t*)phw1)->hwcpu;
	hwcpu2 = ((sdlahw_t*)phw2)->hwcpu;
	if (hwcpu1->hwcard == hwcpu2->hwcard){
		return 1;
	}
	return 0;
}    

static __inline void **__sdla_get_ptr_isr_array(void *phw)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT_RC(hw == NULL, 0);
	WAN_ASSERT_RC(hw->hwcpu == NULL, 0);
	hwcpu = hw->hwcpu;
 	return &hwcpu->port_ptr_isr_array[0];
}

static __inline void __sdla_push_ptr_isr_array(void *phw, void *card, int line)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT_VOID(hw == NULL);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	if (line >= SDLA_MAX_HWDEVS) {
		return;
	}
 	hwcpu->port_ptr_isr_array[line]=card;
} 

static __inline void __sdla_pull_ptr_isr_array(void *phw, void *card, int line)
{
	sdlahw_t	*hw = (sdlahw_t*)phw;
	sdlahw_cpu_t	*hwcpu;

	WAN_ASSERT_VOID(hw == NULL);
	WAN_ASSERT_VOID(hw->hwcpu == NULL);
	hwcpu = hw->hwcpu;
	if (line >= SDLA_MAX_HWDEVS) {
        	return;
	}	
	hwcpu->port_ptr_isr_array[line]=NULL;
	return;
}  


#undef EXTERN 
#endif	/* _SDLADRV_H */
