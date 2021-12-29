/*****************************************************************************
* sdla_aft_te1.h
*	
*		WANPIPE(tm) AFT Hardware Support
* 		
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003-2005 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jan 07, 2003	Nenad Corbic	Initial version.
*****************************************************************************/


#ifndef _SDLA_AFT_TE1_H
#define _SDLA_AFT_TE1_H


#ifdef WAN_KERNEL

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <aft_a104.h>
# include <aft_analog.h>
#else
# include <linux/wanpipe_tdm_api.h>
# include <linux/aft_a104.h>
# include <linux/aft_analog.h>
#endif

#define AFT_PORT0_OFFSET		0x00000
#define AFT_PORT1_OFFSET		0x04000
#define AFT_PORT2_OFFSET		0x08000
#define AFT_PORT3_OFFSET		0x0C000

#define AFT_PORT4_OFFSET		0x10000
#define AFT_PORT5_OFFSET		0x14000
#define AFT_PORT6_OFFSET		0x18000
#define AFT_PORT7_OFFSET		0x1C000

#define AFT_PORT_REG(card,reg)		(reg+(0x4000*card->wandev.comm_port))

/*======================================================
 * GLOBAL  (All Ports)
 * 
 * AFT Chip PCI Registers
 *
 * Global Configuration Area for All Ports
 *
 *=====================================================*/

#define AFT_CHIP_CFG_REG		0x40

#define AFT_ANALOG_MCPU_IFACE_RW	0x44

#define AFT_WDT_CTRL_REG		0x48 
#define AFT_WDT_1TO4_CTRL_REG		AFT_WDT_CTRL_REG

#define AFT_DATA_MUX_REG		0x4C 

#define AFT_FIFO_MARK_REG		0x50

#define AFT_MCPU_INTERFACE_RW		0x54

#define AFT_ANALOG_SPI_IFACE_RW		0x54

#define AFT_CHIP_STAT_REG		0x58

#define AFT_WDT_4TO8_CTRL_REG		0x5C


/*================================================= 
  A104 CHIP CFG REGISTERS  
*/

# define AFT_CHIPCFG_TE1_CFG_BIT	0
# define AFT_CHIPCFG_56K_CFG_BIT	0
# define AFT_CHIPCFG_ANALOG_CLOCK_SELECT_BIT 0
# define AFT_CHIPCFG_SFR_EX_BIT		1
# define AFT_CHIPCFG_SFR_IN_BIT		2
# define AFT_CHIPCFG_FE_INTR_CFG_BIT	3

# define AFT_CHIPCFG_A104D_EC_SEC_KEY_MASK	0x7
# define AFT_CHIPCFG_A104D_EC_SEC_KEY_SHIFT 	4

# define AFT_CHIPCFG_A200_EC_SEC_KEY_MASK	0x3
# define AFT_CHIPCFG_A200_EC_SEC_KEY_SHIFT  	16 

# define AFT_CHIPCFG_SPI_SLOW_BIT	5	/* Slow down SPI */
#if 0
# define AFT_CHIPCFG_EC_INTR_CFG_BIT	4	/* Shark or Analog */
# define AFT_CHIPCFG_SECURITY_CFG_BIT	6
#endif

# define AFT_CHIPCFG_RAM_READY_BIT	7
# define AFT_CHIPCFG_HDLC_CTRL_RDY_BIT  8

# define AFT_CHIPCFG_ANALOG_INTR_MASK	0x0F		/* Analog */
# define AFT_CHIPCFG_ANALOG_INTR_SHIFT	9

# define AFT_CHIPCFG_A108_EC_CLOCK_SRC_MASK	0x07	/* A108 */
# define AFT_CHIPCFG_A108_EC_CLOCK_SRC_SHIFT	9	

# define AFT_CHIPCFG_A104D_EC_SECURITY_BIT 12
# define AFT_CHIPCFG_A108_EC_INTR_ENABLE_BIT 12		/* A108 */

# define AFT_CHIPCFG_EC_INTR_STAT_BIT	13


/* A104 A200 A108 Differ Here 
 * By default any register without device name is
 * common to all all.
 */

# define AFT_CHIPCFG_P1_TDMV_INTR_BIT	14	

# define AFT_CHIPCFG_P2_TDMV_INTR_BIT	15	
# define AFT_CHIPCFG_A104_TDM_ACK_BIT 15

# define AFT_CHIPCFG_A200_EC_SECURITY_BIT 15		/* Analog */
# define AFT_CHIPCFG_A108_EC_SECURITY_BIT 15		/* A108 */

# define AFT_CHIPCFG_P3_TDMV_INTR_BIT	16	
# define AFT_CHIPCFG_A108_A104_TDM_FIFO_SYNC_BIT	16 	/* A108 Global Fifo Sync Bit */

# define AFT_CHIPCFG_P4_TDMV_INTR_BIT	17	
# define AFT_CHIPCFG_A108_A104_TDM_DMA_RINGBUF_BIT	17   	/* A108 Quad DMA Ring buf enable */

# define AFT_CHIPCFG_P1_WDT_INTR_BIT	18
# define AFT_CHIPCFG_P2_WDT_INTR_BIT	19
# define AFT_CHIPCFG_P3_WDT_INTR_BIT	20
# define AFT_CHIPCFG_P4_WDT_INTR_BIT	21

# define AFT_CHIPCFG_A108_EC_INTER_STAT_BIT	21	/* A108 */

# define AFT_CHIPCFG_FE_INTR_STAT_BIT	22
# define AFT_CHIPCFG_SECURITY_STAT_BIT	23


/* A104 & A104D IRQ Status Bits */

# define AFT_CHIPCFG_HDLC_INTR_MASK	0x0F
# define AFT_CHIPCFG_HDLC_INTR_SHIFT	24

# define AFT_CHIPCFG_DMA_INTR_MASK	0x0F
# define AFT_CHIPCFG_DMA_INTR_SHIFT	28

# define AFT_CHIPCFG_A108_TDM_GLOBAL_RX_INTR_ACK 30
# define AFT_CHIPCFG_A108_TDM_GLOBAL_TX_INTR_ACK 31

# define AFT_CHIPCFG_WDT_INTR_MASK	0x0F
# define AFT_CHIPCFG_WDT_INTR_SHIFT	18

# define AFT_CHIPCFG_TDMV_INTR_MASK	0x0F
# define AFT_CHIPCFG_TDMV_INTR_SHIFT	14

#  define AFT_CHIPCFG_WDT_FE_INTR_STAT	0
#  define AFT_CHIPCFG_WDT_TX_INTR_STAT  1
#  define AFT_CHIPCFG_WDT_RX_INTR_STAT	2



/* A104 & A104D Interrupt Status Funcitons */

static __inline u32
aft_chipcfg_get_hdlc_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_HDLC_INTR_SHIFT;
	reg&=AFT_CHIPCFG_HDLC_INTR_MASK;
	return reg;
}

static __inline u32
aft_chipcfg_get_analog_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_ANALOG_INTR_SHIFT;
	reg&=AFT_CHIPCFG_ANALOG_INTR_MASK;
	return reg;
}

static __inline void
aft_chipcfg_set_oct_clk_src(u32 *reg, u32 src)
{ 
 	*reg&=~(AFT_CHIPCFG_A108_EC_CLOCK_SRC_MASK<<AFT_CHIPCFG_A108_EC_CLOCK_SRC_SHIFT);
	src&=AFT_CHIPCFG_A108_EC_CLOCK_SRC_MASK;
	*reg|=(src<<AFT_CHIPCFG_A108_EC_CLOCK_SRC_SHIFT);       
}


static __inline u32
aft_chipcfg_get_dma_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_DMA_INTR_SHIFT;
	reg&=AFT_CHIPCFG_DMA_INTR_MASK;
	return reg;
}

static __inline u32
aft_chipcfg_get_wdt_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_WDT_INTR_SHIFT;
	reg&=AFT_CHIPCFG_WDT_INTR_MASK;
	return reg;
}

static __inline u32
aft_chipcfg_get_tdmv_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_TDMV_INTR_SHIFT;
	reg&=AFT_CHIPCFG_TDMV_INTR_MASK;
	return reg;
}




/* A108 IRQ Status Bits on CHIP_STAT_REG */

# define AFT_CHIPCFG_A108_WDT_INTR_MASK		0xFF
# define AFT_CHIPCFG_A108_WDT_INTR_SHIFT	0

# define AFT_CHIPCFG_A108_TDMV_INTR_MASK	0xFF
# define AFT_CHIPCFG_A108_TDMV_INTR_SHIFT	8

# define AFT_CHIPCFG_A108_FIFO_INTR_MASK	0xFF
# define AFT_CHIPCFG_A108_FIFO_INTR_SHIFT	16

# define AFT_CHIPCFG_A108_DMA_INTR_MASK		0xFF
# define AFT_CHIPCFG_A108_DMA_INTR_SHIFT	24

/* A108 Interrupt Status Functions */

static __inline u32
aft_chipcfg_a108_get_fifo_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_A108_FIFO_INTR_SHIFT;
	reg&=AFT_CHIPCFG_A108_FIFO_INTR_MASK;
	return reg;
}

static __inline u32
aft_chipcfg_a108_get_dma_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_A108_DMA_INTR_SHIFT;
	reg&=AFT_CHIPCFG_A108_DMA_INTR_MASK;
	return reg;
}

static __inline u32
aft_chipcfg_a108_get_wdt_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_A108_WDT_INTR_SHIFT;
	reg&=AFT_CHIPCFG_A108_WDT_INTR_MASK;
	return reg;
}

static __inline u32
aft_chipcfg_a108_get_tdmv_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_A108_TDMV_INTR_SHIFT;
	reg&=AFT_CHIPCFG_A108_TDMV_INTR_MASK;
	return reg;
}

/* 56k IRQ status bits */
# define AFT_CHIPCFG_A56K_WDT_INTR_BIT		0
# define AFT_CHIPCFG_A56K_DMA_INTR_BIT		24
# define AFT_CHIPCFG_A56K_FIFO_INTR_BIT		16

# define AFT_CHIPCFG_A56K_FE_MASK			0x7F
# define AFT_CHIPCFG_A56K_FE_SHIFT			9

static __inline u32
aft_chipcfg_a56k_read_fe(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_A56K_FE_SHIFT;
	reg&=AFT_CHIPCFG_A56K_FE_MASK;
	return reg;
}

static __inline u32
aft_chipcfg_a56k_write_fe(u32 reg, u32 val)
{
	val&=AFT_CHIPCFG_A56K_FE_MASK;

	reg &= ~(AFT_CHIPCFG_A56K_FE_MASK<<AFT_CHIPCFG_A56K_FE_SHIFT);

	reg |= (val<<AFT_CHIPCFG_A56K_FE_SHIFT);

	return reg;
}


static __inline u32
aft_chipcfg_get_ec_channels(u32 reg)
{
	switch ((reg>>AFT_CHIPCFG_A104D_EC_SEC_KEY_SHIFT)&AFT_CHIPCFG_A104D_EC_SEC_KEY_MASK){
    
	case 0x00:
		return 0;
	case 0x01:
		return 32;
	case 0x02:
		return 64;
	case 0x03:
		return 96;
	case 0x04:
		return 128;
	case 0x05:
		return 256;
   	default:
		return 0;
	}
	
	return 0;
}


static __inline u32
aft_chipcfg_get_a200_ec_channels(u32 reg)
{
	switch ((reg>>AFT_CHIPCFG_A200_EC_SEC_KEY_SHIFT)&AFT_CHIPCFG_A200_EC_SEC_KEY_MASK){
    
	case 0x00:
		return 0;
	case 0x01:
		return 16;
	case 0x02:
		return 32;
   	default:
		return 0;
	}
	
	return 0;
}        

# define AFT_WDTCTRL_MASK		0xFF
# define AFT_WDTCTRL_TIMEOUT 		75	/* ms */

static __inline void 
aft_wdt_ctrl_reset(u8 *reg)
{
	*reg=0xFF;
}
static __inline void 
aft_wdt_ctrl_set(u8 *reg, u8 timeout)
{
	timeout&=AFT_WDTCTRL_MASK;
	*reg=timeout;
}


#define AFT_FIFO_MARK_32_MASK	0x3F
#define AFT_FIFO_MARK_32_SHIFT	0

#define AFT_FIFO_MARK_64_MASK	0x3F
#define AFT_FIFO_MARK_64_SHIFT	6

#define AFT_FIFO_MARK_128_MASK	0x3F
#define AFT_FIFO_MARK_128_SHIFT	12

#define AFT_FIFO_MARK_256_MASK	0x3F
#define AFT_FIFO_MARK_256_SHIFT	18

#define AFT_FIFO_MARK_512_MASK	0x3F
#define AFT_FIFO_MARK_512_SHIFT	24

static __inline void
aft_fifo_mark_gset(u32 *reg, u8 mark)
{	
	mark&=AFT_FIFO_MARK_32_MASK;
	
	*reg=0;
	*reg=(mark<<AFT_FIFO_MARK_32_SHIFT)|
	     (mark<<AFT_FIFO_MARK_64_SHIFT)|
	     (mark<<AFT_FIFO_MARK_128_SHIFT)|
	     (mark<<AFT_FIFO_MARK_256_SHIFT)|
	     (mark<<AFT_FIFO_MARK_512_SHIFT);
}


/*======================================================
 * PER PORT 
 * 
 * AFT INTERRUPT Configuration Registers
 *
 *=====================================================*/

#define AFT_LINE_CFG_REG		0x100

#define AFT_TX_DMA_INTR_MASK_REG	0x104

#define AFT_TX_DMA_INTR_PENDING_REG	0x108

#define AFT_TX_FIFO_INTR_PENDING_REG	0x10C

#define AFT_RX_DMA_INTR_MASK_REG	0x110

#define AFT_RX_DMA_INTR_PENDING_REG	0x114

#define AFT_RX_FIFO_INTR_PENDING_REG	0x118


# define AFT_LCFG_FE_IFACE_RESET_BIT	0
# define AFT_LCFG_TX_FE_SYNC_STAT_BIT	1
# define AFT_LCFG_TX_FE_FIFO_STAT_BIT	2
# define AFT_LCFG_RX_FE_SYNC_STAT_BIT	3
# define AFT_LCFG_RX_FE_FIFO_STAT_BIT	4

# define AFT_LCFG_SS7_MODE_BIT		5
# define AFT_LCFG_SS7_LSSU_FRMS_SZ_BIT	6

# define AFT_LCFG_DMA_INTR_BIT		8
# define AFT_LCFG_FIFO_INTR_BIT		9

# define AFT_LCFG_TDMV_CH_NUM_MASK	0x1F
# define AFT_LCFG_TDMV_CH_NUM_SHIFT	10

# define AFT_LCFG_TDMV_INTR_BIT		15

# define AFT_LCFG_A108_CLK_ROUTE_MASK	0x0F
# define AFT_LCFG_A108_CLK_ROUTE_SHIFT	16

# define AFT_LCFG_CLR_CHNL_EN		26

# define AFT_LCFG_FE_CLK_ROUTE_BIT	27

# define AFT_LCFG_FE_CLK_SOURCE_MASK	0x03
# define AFT_LCFG_FE_CLK_SOURCE_SHIFT	28

# define AFT_LCFG_GREEN_LED_BIT		30
# define AFT_LCFG_A108_FE_CLOCK_MODE_BIT 31	/* A108 */

# define AFT_LCFG_RED_LED_BIT		31
# define AFT_LCFG_A108_FE_TE1_MODE_BIT	30	/* A108 */




static __inline void
aft_lcfg_fe_clk_source(u32 *reg, u32 src)
{
	*reg&=~(AFT_LCFG_FE_CLK_SOURCE_MASK<<AFT_LCFG_FE_CLK_SOURCE_SHIFT);
	*reg|=(src&AFT_LCFG_FE_CLK_SOURCE_MASK)<<AFT_LCFG_FE_CLK_SOURCE_SHIFT;
}

static __inline void
aft_lcfg_a108_fe_clk_source(u32 *reg, u32 src)
{
	*reg&=~(AFT_LCFG_A108_CLK_ROUTE_MASK<<AFT_LCFG_A108_CLK_ROUTE_SHIFT);
	*reg|=(src&AFT_LCFG_A108_CLK_ROUTE_MASK)<<AFT_LCFG_A108_CLK_ROUTE_SHIFT;
}


static __inline void
aft_lcfg_tdmv_cnt_inc(u32 *reg)
{
	u32 cnt = (*reg>>AFT_LCFG_TDMV_CH_NUM_SHIFT)&AFT_LCFG_TDMV_CH_NUM_MASK;
	if (cnt < 32){
		cnt++;
	}
	*reg&=~(AFT_LCFG_TDMV_CH_NUM_MASK<<AFT_LCFG_TDMV_CH_NUM_SHIFT);
	*reg|=(cnt&AFT_LCFG_TDMV_CH_NUM_MASK)<<AFT_LCFG_TDMV_CH_NUM_SHIFT;
}
static __inline void
aft_lcfg_tdmv_cnt_dec(u32 *reg)
{
	u32 cnt = (*reg>>AFT_LCFG_TDMV_CH_NUM_SHIFT)&AFT_LCFG_TDMV_CH_NUM_MASK;
	if (cnt > 0){
		cnt--;
	}
	*reg&=~(AFT_LCFG_TDMV_CH_NUM_MASK<<AFT_LCFG_TDMV_CH_NUM_SHIFT);
	*reg|=(cnt&AFT_LCFG_TDMV_CH_NUM_MASK)<<AFT_LCFG_TDMV_CH_NUM_SHIFT;
}

/* SS7 LCFG MODE
 * 0: 128
 * 1: 4096
 *
 * SS7 LSSU FRM SZ
 * 0: 4byte (if 128) || 7byte (if 4096)
 * 1: 5byte (if 128) || 8byte (if 4096)
 */

static __inline void 
aft_lcfg_ss7_mode128_cfg(u32 *reg, int lssu_size)
{
	wan_clear_bit(AFT_LCFG_SS7_MODE_BIT,reg);
	if (lssu_size == 4){
		wan_clear_bit(AFT_LCFG_SS7_LSSU_FRMS_SZ_BIT,reg);
	}else{
		wan_set_bit(AFT_LCFG_SS7_LSSU_FRMS_SZ_BIT,reg);
	}
}

static __inline void 
aft_lcfg_ss7_mode4096_cfg(u32 *reg, int lssu_size)
{
	wan_set_bit(AFT_LCFG_SS7_MODE_BIT,reg);
	if (lssu_size == 7){
		wan_clear_bit(AFT_LCFG_SS7_LSSU_FRMS_SZ_BIT,reg);
	}else{
		wan_set_bit(AFT_LCFG_SS7_LSSU_FRMS_SZ_BIT,reg);
	}
}


/*======================================================
 * PER PORT 
 * 
 * AFT General DMA Registers
 *
 *=====================================================*/

#define AFT_DMA_CTRL_REG		0x200

#define AFT_TX_DMA_CTRL_REG		0x204

#define AFT_RX_DMA_CTRL_REG		0x208

#define AFT_ANALOG_DATA_MUX_CTRL_REG	0x20C

#define AFT_ANALOG_EC_CTRL_REG		0x210

# define AFT_DMACTRL_TDMV_RX_TOGGLE	14
# define AFT_DMACTRL_TDMV_TX_TOGGLE	15

# define AFT_DMACTRL_MAX_LOGIC_CH_SHIFT 16
# define AFT_DMACTRL_MAX_LOGIC_CH_MASK  0x1F

# define AFT_DMACTRL_GLOBAL_INTR_BIT	31

static __inline void 
aft_dmactrl_set_max_logic_ch(u32 *reg, int max_logic_ch)
{
	*reg&=~(AFT_DMACTRL_MAX_LOGIC_CH_MASK<<AFT_DMACTRL_MAX_LOGIC_CH_SHIFT);
	max_logic_ch&=AFT_DMACTRL_MAX_LOGIC_CH_MASK;
	*reg|=(max_logic_ch<<AFT_DMACTRL_MAX_LOGIC_CH_SHIFT);
}

/*======================================================
 * PER PORT 
 * 
 * AFT Control RAM DMA Registers
 *
 *=====================================================*/

#define AFT_CONTROL_RAM_ACCESS_BASE_REG	0x1000


# define AFT_CTRLRAM_LOGIC_CH_NUM_MASK	0x1F
# define AFT_CTRLRAM_LOGIC_CH_NUM_SHIFT	0

# define AFT_CTRLRAM_HDLC_MODE_BIT	  7
# define AFT_CTRLRAM_HDLC_CRC_SIZE_BIT	  8
# define AFT_CTRLRAM_HDLC_TXCH_RESET_BIT  9
# define AFT_CTRLRAM_HDLC_RXCH_RESET_BIT 10

# define AFT_CTRLRAM_SYNC_FST_TSLOT_BIT	 11

# define AFT_CTRLRAM_SS7_ENABLE_BIT	 12

# define AFT_CTRLRAM_DATA_MUX_ENABLE_BIT 13
# define AFT_CTRLRAM_SS7_FORCE_RX_BIT	 14

# define AFT_CTRLRAM_FIFO_SIZE_SHIFT	 16
# define AFT_CTRLRAM_FIFO_SIZE_MASK	 0x1F

# define AFT_CTRLRAM_FIFO_BASE_SHIFT	 24
# define AFT_CTRLRAM_FIFO_BASE_MASK	 0x1F


static __inline void
aft_ctrlram_set_logic_ch(u32 *reg, int logic_ch)
{
	*reg&=~(AFT_CTRLRAM_LOGIC_CH_NUM_MASK<<AFT_CTRLRAM_LOGIC_CH_NUM_SHIFT);
	logic_ch&=AFT_CTRLRAM_LOGIC_CH_NUM_MASK;
	*reg|=logic_ch;
}

static __inline void
aft_ctrlram_set_fifo_size(u32 *reg, int size)
{
	*reg&=~(AFT_CTRLRAM_FIFO_SIZE_MASK<<AFT_CTRLRAM_FIFO_SIZE_SHIFT);
	size&=AFT_CTRLRAM_FIFO_SIZE_MASK;
	*reg|=(size<<AFT_CTRLRAM_FIFO_SIZE_SHIFT);
}

static __inline void
aft_ctrlram_set_fifo_base(u32 *reg, int base)
{
	*reg&=~(AFT_CTRLRAM_FIFO_BASE_MASK<<AFT_CTRLRAM_FIFO_BASE_SHIFT);
	base&=AFT_CTRLRAM_FIFO_BASE_MASK;
	*reg|=(base<<AFT_CTRLRAM_FIFO_BASE_SHIFT);
}


/*======================================================
 * PER PORT 
 * 
 * AFT DMA Chain RAM Registers
 *
 *=====================================================*/

#define AFT_DMA_CHAIN_RAM_BASE_REG 	0x1800

# define AFT_DMACHAIN_TX_ADDR_CNT_MASK	0x0F
# define AFT_DMACHAIN_TX_ADDR_CNT_SHIFT 0

# define AFT_DMACHAIN_RX_ADDR_CNT_MASK	0x0F
# define AFT_DMACHAIN_RX_ADDR_CNT_SHIFT 8

# define AFT_DMACHAIN_FIFO_SIZE_MASK	0x1F
# define AFT_DMACHAIN_FIFO_SIZE_SHIFT	16

/* AFT_DMACHAIN_TDMV_SIZE Bit Map
 * 00=  8byte size
 * 01= 16byte size
 * 10= 32byte size
 * 11= 64byte size 
 */
# define AFT_DMACHAIN_TDMV_SIZE_MASK	0x03
# define AFT_DMACHAIN_TDMV_SIZE_SHIFT	21

# define AFT_DMACHAIN_TDMV_ENABLE_BIT	23

# define AFT_DMACHAIN_FIFO_BASE_MASK	0x1F
# define AFT_DMACHAIN_FIFO_BASE_SHIFT	24

# define AFT_DMACHAIN_SS7_FORCE_RX	29

# define AFT_DMACHAIN_SS7_ENABLE_BIT	30

# define AFT_DMACHAIN_RX_SYNC_BIT 	31

static __inline u32
aft_dmachain_get_tx_dma_addr(u32 reg)
{
	reg=reg>>AFT_DMACHAIN_TX_ADDR_CNT_SHIFT;
	reg&=AFT_DMACHAIN_TX_ADDR_CNT_MASK;
	return reg;
}
static __inline void
aft_dmachain_set_tx_dma_addr(u32 *reg, int addr)
{
	*reg&=~(AFT_DMACHAIN_TX_ADDR_CNT_MASK<<AFT_DMACHAIN_TX_ADDR_CNT_SHIFT);
	addr&=AFT_DMACHAIN_TX_ADDR_CNT_MASK;
	*reg|=addr<<AFT_DMACHAIN_TX_ADDR_CNT_SHIFT;
	return;
}

static __inline u32
aft_dmachain_get_rx_dma_addr(u32 reg)
{
	reg=reg>>AFT_DMACHAIN_RX_ADDR_CNT_SHIFT;
	reg&=AFT_DMACHAIN_RX_ADDR_CNT_MASK;
	return reg;
}


static __inline void
aft_dmachain_set_rx_dma_addr(u32 *reg, int addr)
{
	*reg&=~(AFT_DMACHAIN_RX_ADDR_CNT_MASK<<AFT_DMACHAIN_RX_ADDR_CNT_SHIFT);
	addr&=AFT_DMACHAIN_RX_ADDR_CNT_MASK;
	*reg|=addr<<AFT_DMACHAIN_RX_ADDR_CNT_SHIFT;
	return;
}


static __inline void
aft_dmachain_set_fifo_size(u32 *reg, int size)
{
	*reg&=~(AFT_DMACHAIN_FIFO_SIZE_MASK<<AFT_DMACHAIN_FIFO_SIZE_SHIFT);
	size&=AFT_DMACHAIN_FIFO_SIZE_MASK;
	*reg|=(size<<AFT_DMACHAIN_FIFO_SIZE_SHIFT);
}

static __inline void
aft_dmachain_set_fifo_base(u32 *reg, int base)
{
	*reg&=~(AFT_DMACHAIN_FIFO_BASE_MASK<<AFT_DMACHAIN_FIFO_BASE_SHIFT);
	base&=AFT_DMACHAIN_FIFO_BASE_MASK;
	*reg|=(base<<AFT_DMACHAIN_FIFO_BASE_SHIFT);
}

static __inline void
aft_dmachain_enable_tdmv_and_mtu_size(u32 *reg, int size)
{
	*reg&=~(AFT_DMACHAIN_TDMV_SIZE_MASK<<AFT_DMACHAIN_TDMV_SIZE_SHIFT);

	switch(size){
	case 8:
		size=0x00;
		break;
	case 16:
		size=0x01;
		break;
	case 40:
		size=0x02;
		break;
	case 80:
		size=0x03;
		break;
	default:
		size=0x00;
		break;
	}
	
	size&=AFT_DMACHAIN_TDMV_SIZE_MASK;
	*reg|=(size<<AFT_DMACHAIN_TDMV_SIZE_SHIFT);

	wan_set_bit(AFT_DMACHAIN_TDMV_ENABLE_BIT,reg);
}


/*======================================================
 * PER PORT 
 * 
 * AFT DMA Descr RAM Registers
 *
 *=====================================================*/

#define AFT_TX_DMA_LO_DESCR_BASE_REG	0x2000
#define AFT_TX_DMA_HI_DESCR_BASE_REG	0x2004

#define AFT_RX_DMA_LO_DESCR_BASE_REG	0x2008
#define AFT_RX_DMA_HI_DESCR_BASE_REG	0x200C


# define AFT_TXDMA_LO_ALIGN_MASK	0x03
# define AFT_TXDMA_LO_ALIGN_SHIFT	0

# define AFT_TXDMA_HI_DMA_LENGTH_MASK	0xFFF
# define AFT_TXDMA_HI_DMA_LENGTH_SHIFT	0

# define AFT_TXDMA_HI_DMA_STATUS_MASK	0x0F
# define AFT_TXDMA_HI_DMA_STATUS_SHIFT	11

#  define AFT_TXDMA_HIDMASTATUS_PCI_M_ABRT	0
#  define AFT_TXDMA_HIDMASTATUS_PCI_T_ABRT	1
#  define AFT_TXDMA_HIDMASTATUS_PCI_DS_TOUT	2
#  define AFT_TXDMA_HIDMASTATUS_PCI_RETRY	3

# define AFT_TXDMA_HI_SS7_FI_LS_TX_STATUS_BIT	19
# define AFT_TXDMA_HI_SS7_FISU_OR_LSSU_BIT	20
# define AFT_TXDMA_HI_SS7_FI_LS_FORCE_TX_BIT	21

# define AFT_TXDMA_HI_LAST_DESC_BIT	24
# define AFT_TXDMA_HI_INTR_DISABLE_BIT	27
# define AFT_TXDMA_HI_DMA_CMD_BIT	28
# define AFT_TXDMA_HI_EOF_BIT		29
# define AFT_TXDMA_HI_START_BIT		30
# define AFT_TXDMA_HI_GO_BIT		31

# define AFT_RXDMA_LO_ALIGN_MASK	0x03
# define AFT_RXDMA_LO_ALIGN_SHIFT	0

# define AFT_RXDMA_HI_DMA_LENGTH_MASK	0xFFF
# define AFT_RXDMA_HI_DMA_LENGTH_SHIFT	0

# define AFT_RXDMA_HI_DMA_STATUS_MASK	0x0F
# define AFT_RXDMA_HI_DMA_STATUS_SHIFT	11

#  define AFT_RXDMA_HIDMASTATUS_PCI_M_ABRT	0
#  define AFT_RXDMA_HIDMASTATUS_PCI_T_ABRT	1
#  define AFT_RXDMA_HIDMASTATUS_PCI_DS_TOUT	2
#  define AFT_RXDMA_HIDMASTATUS_PCI_RETRY	3

# define AFT_RXDMA_HI_IFT_INTR_ENB_BIT	15
# define AFT_RXDMA_HI_LAST_DESC_BIT	24
# define AFT_RXDMA_HI_FCS_ERR_BIT	25
# define AFT_RXDMA_HI_FRM_ABORT_BIT	26
# define AFT_RXDMA_HI_INTR_DISABLE_BIT	27
# define AFT_RXDMA_HI_DMA_CMD_BIT	28
# define AFT_RXDMA_HI_EOF_BIT		29
# define AFT_RXDMA_HI_START_BIT		30
# define AFT_RXDMA_HI_GO_BIT		31


static __inline void
aft_txdma_lo_set_alignment(u32 *reg, int align)
{
	*reg&=~(AFT_TXDMA_LO_ALIGN_MASK<<AFT_TXDMA_LO_ALIGN_SHIFT);
	align&=AFT_TXDMA_LO_ALIGN_MASK;
	*reg|=(align<<AFT_TXDMA_LO_ALIGN_SHIFT);
}
static __inline u32
aft_txdma_lo_get_alignment(u32 reg)
{
	reg=reg>>AFT_TXDMA_LO_ALIGN_SHIFT;
	reg&=AFT_TXDMA_LO_ALIGN_MASK;
	return reg;
}

static __inline void
aft_txdma_hi_set_dma_length(u32 *reg, int len, int align)
{
	*reg&=~(AFT_TXDMA_HI_DMA_LENGTH_MASK<<AFT_TXDMA_HI_DMA_LENGTH_SHIFT);
	len=(len>>2)+align;
	len&=AFT_TXDMA_HI_DMA_LENGTH_MASK;
	*reg|=(len<<AFT_TXDMA_HI_DMA_LENGTH_SHIFT);
}

static __inline u32
aft_txdma_hi_get_dma_length(u32 reg)
{
	reg=reg>>AFT_TXDMA_HI_DMA_LENGTH_SHIFT;
	reg&=AFT_TXDMA_HI_DMA_LENGTH_MASK;
	reg=reg<<2;
	return reg;	
}

static __inline u32
aft_txdma_hi_get_dma_status(u32 reg)
{
	reg=reg>>AFT_TXDMA_HI_DMA_STATUS_SHIFT;
	reg&=AFT_TXDMA_HI_DMA_STATUS_MASK;
	return reg;	
}

static __inline void
aft_rxdma_lo_set_alignment(u32 *reg, int align)
{
	*reg&=~(AFT_RXDMA_LO_ALIGN_MASK<<AFT_RXDMA_LO_ALIGN_SHIFT);
	align&=AFT_RXDMA_LO_ALIGN_MASK;
	*reg|=(align<<AFT_RXDMA_LO_ALIGN_SHIFT);
}
static __inline u32
aft_rxdma_lo_get_alignment(u32 reg)
{
	reg=reg>>AFT_RXDMA_LO_ALIGN_SHIFT;
	reg&=AFT_RXDMA_LO_ALIGN_MASK;
	return reg;
}

static __inline void
aft_rxdma_hi_set_dma_length(u32 *reg, int len, int align)
{
	*reg&=~(AFT_RXDMA_HI_DMA_LENGTH_MASK<<AFT_RXDMA_HI_DMA_LENGTH_SHIFT);
	len=(len>>2)-align;
	len&=AFT_RXDMA_HI_DMA_LENGTH_MASK;
	*reg|=(len<<AFT_RXDMA_HI_DMA_LENGTH_SHIFT);
}

static __inline u32
aft_rxdma_hi_get_dma_length(u32 reg)
{
	reg=reg>>AFT_RXDMA_HI_DMA_LENGTH_SHIFT;
	reg&=AFT_RXDMA_HI_DMA_LENGTH_MASK;
	reg=reg<<2;
	return reg;	
}

static __inline u32
aft_rxdma_hi_get_dma_status(u32 reg)
{
	reg=reg>>AFT_RXDMA_HI_DMA_STATUS_SHIFT;
	reg&=AFT_RXDMA_HI_DMA_STATUS_MASK;
	return reg;	
}


#define FIFO_32B			0x00
#define FIFO_64B			0x01
#define FIFO_128B			0x03
#define FIFO_256B			0x07
#define FIFO_512B			0x0F
#define FIFO_1024B			0x1F



/*===============================================

*/

/* Default Active DMA channel used by the
 * DMA Engine */
#define AFT_DEFLT_ACTIVE_CH  0

#define MAX_AFT_TX_DMA_SIZE  0xFFFF

#define MIN_WP_PRI_MTU		8	
#define DEFAULT_WP_PRI_MTU 	1500

/* Maximum MTU for AFT card
 * 8192-4=8188.  This is a hardware
 * limitation. 
 */
#define MAX_WP_PRI_MTU		8188

#define MAX_DMA_PER_CH		20
#define MIN_DMA_PER_CH		2

#define WP_MAX_FIFO_FRAMES      7

#define AFT_DEFAULT_DATA_MUX_MAP 0x01234567
enum {
	WAN_AFT_RED,
	WAN_AFT_GREEN
};

enum {
	WAN_AFT_OFF,
	WAN_AFT_ON
};


/*==========================================
 * Board CPLD Interface Section
 *
 *=========================================*/


#define PMC_CONTROL_REG		0x00

/* Used to reset the pcm 
 * front end 
 *    0: Reset Enable 
 *    1: Normal Operation 
 */
#define PMC_RESET_BIT		0	

/* Used to set the pmc clock
 * source:
 *   0 = E1
 *   1 = T1 
 */
#define PMC_CLOCK_SELECT	1

#define LED_CONTROL_REG		0x01

#define JP8_VALUE               0x02
#define JP7_VALUE               0x01
#define SW0_VALUE               0x04
#define SW1_VALUE               0x08

#define CUSTOMER_CPLD_ID_REG	0x0A

/* -------------------------------------- */

#define WRITE_DEF_SECTOR_DSBL   0x01
#define FRONT_END_TYPE_MASK     0x38


#define MEMORY_TYPE_SRAM        0x00
#define MEMORY_TYPE_FLASH       0x01
#define MASK_MEMORY_TYPE_SRAM   0x10
#define MASK_MEMORY_TYPE_FLASH  0x20

#define BIT_A18_SECTOR_SA4_SA7  0x20
#define USER_SECTOR_START_ADDR  0x40000

#define MAX_TRACE_QUEUE         100

#define	HDLC_FREE_LOGIC_CH	0x1F					
#define AFT_DEFLT_ACTIVE_CH	0
						
static __inline unsigned short aft_valid_mtu(unsigned short mtu)
{
	if (mtu <= 128){
		return 256;
	}else if (mtu <= 256){
		return 512;
	}else if (mtu <= 512){
		return 1024;
	}else if (mtu <= 1024){
		return 2048;
	}else if (mtu <= 2048){
		return 4096;
	}else if (mtu <= 4096){
		return 8188;
	}else if (mtu <= 8188){
		return 8188;
	}else{
		return 0;
	}	
}

static __inline unsigned short aft_dma_buf_bits(unsigned short dma_bufs)
{
	if (dma_bufs < 2){
		return 0;
	}else if (dma_bufs < 3){
		return 1;
	}else if (dma_bufs < 5){
		return 2;
	}else if (dma_bufs < 9){
		return 3;
	}else if (dma_bufs < 17){
		return 4;
	}else{
		return 0;
	}	
}


static __inline void
aft_set_led(unsigned int color, int led_pos, int on, u32 *reg)
{
	if (color == WAN_AFT_RED){
		if (on == WAN_AFT_OFF){
			wan_clear_bit(AFT_LCFG_RED_LED_BIT,reg);
		}else{
			wan_set_bit(AFT_LCFG_RED_LED_BIT,reg);
		}
	}else{
		if (on == WAN_AFT_OFF){
			wan_clear_bit(AFT_LCFG_GREEN_LED_BIT,reg);
		}else{
			wan_set_bit(AFT_LCFG_GREEN_LED_BIT,reg);
		}
	}
}

static __inline int
aft_get_num_of_slots(u32 total_slots, u32 chan_slots)
{	
	int num_of_slots=0;
	int i;
	for (i=0;i<total_slots;i++){
		if (wan_test_bit(i,&chan_slots)){
			num_of_slots++;
		}
	}

	return num_of_slots;
}


#define MAX_AFT_HW_DEV 20

typedef struct aft_hw_dev{

	int init;

	int (*aft_global_chip_config)(sdla_t *card);
	int (*aft_global_chip_unconfig)(sdla_t *card);

	int (*aft_chip_config)(sdla_t *card);
	int (*aft_chip_unconfig)(sdla_t *card);

	int (*aft_chan_config)(sdla_t *card, void *chan);
	int (*aft_chan_unconfig)(sdla_t *card, void *chan);

	int (*aft_led_ctrl)(sdla_t *card, int, int, int);
	int (*aft_test_sync)(sdla_t *card, int);

	unsigned char (*aft_read_fe)(sdla_t* card, unsigned short off);
	int (*aft_write_fe)(sdla_t *card, unsigned short off, unsigned char value);

	unsigned char (*aft_read_cpld)(sdla_t *card, unsigned short cpld_off);
	int (*aft_write_cpld)(sdla_t *card, unsigned short cpld_off, u_int16_t cpld_data);

	void (*aft_fifo_adjust)(sdla_t *card, u32 level);

	int (*aft_check_ec_security)(sdla_t *card);

}aft_hw_dev_t;

extern aft_hw_dev_t aft_hwdev[MAX_AFT_HW_DEV];

#define ASSERT_AFT_HWDEV(type) \
	if (type >= MAX_AFT_HW_DEV){ \
		DEBUG_EVENT("%s:%d: Critical Invalid AFT HW DEV Type %d\n", \
				__FUNCTION__,__LINE__,type); \
		return -EINVAL;	\
	}\
	if (!wan_test_bit(0,&aft_hwdev[type].init)) { \
		DEBUG_EVENT("%s:%d: Critical AFT HW DEV Type %d not initialized\n", \
				__FUNCTION__,__LINE__,type); \
		return -EINVAL; \
	}

#define ASSERT_AFT_HWDEV_VOID(type) \
	if (type >= MAX_AFT_HW_DEV){ \
		DEBUG_EVENT("%s:%d: Critical Invalid AFT HW DEV Type %d\n", \
				__FUNCTION__,__LINE__,type); \
		return;	\
	}\
	if (!aft_hwdev[type] || !wan_test_bit(0,&aft_hwdev[type].init)) { \
		DEBUG_EVENT("%s:%d: Critical AFT HW DEV Type %d not initialized\n", \
				__FUNCTION__,__LINE__,type); \
		return; \
	}


#endif /* WAN_KERNEL */

/*================================================================
 * DRIVER SPECIFIC DEFINES
 * 
 *================================================================*/
						

#undef  wan_udphdr_data
#define wan_udphdr_data	wan_udphdr_u.aft.data

						
#define MAX_TRACE_BUFFER	(MAX_LGTH_UDP_MGNT_PKT - 	\
				 sizeof(iphdr_t) - 		\
	 			 sizeof(udphdr_t) - 		\
				 sizeof(wan_mgmt_t) - 		\
				 sizeof(wan_trace_info_t) - 	\
		 		 sizeof(wan_cmd_t))
	
enum {
	TX_DMA_BUF_INIT =0,		
	TX_DMA_BUF_USED
};
 
enum {
	ROUTER_UP_TIME = 0x50,
	ENABLE_TRACING,	
	DISABLE_TRACING,
	GET_TRACE_INFO,
	READ_CODE_VERSION,
	FLUSH_OPERATIONAL_STATS,
	OPERATIONAL_STATS,
	READ_OPERATIONAL_STATS,
	READ_CONFIGURATION,
	READ_COMMS_ERROR_STATS,
	FLUSH_COMMS_ERROR_STATS,
	AFT_LINK_STATUS,
	AFT_MODEM_STATUS,
	AFT_HWEC_STATUS,
	DIGITAL_LOOPTEST
};

#define UDPMGMT_SIGNATURE		"AFTPIPEA"


/* the line trace status element presented by the frame relay code */
typedef struct {
        unsigned char 	flag	; /* ready flag */
        unsigned short 	length   ; /* trace length */
        unsigned char 	rsrv0[2]  ; /* reserved */
        unsigned char 	attr      ; /* trace attributes */
        unsigned short 	tmstamp  ; /* time stamp */
        unsigned char 	rsrv1[4]  ; /* reserved */
        unsigned int	offset    ; /* buffer absolute address */
}aft_trc_el_t;

typedef struct wp_rx_element
{
	unsigned int dma_addr;
	unsigned int reg;
	unsigned int align;
	unsigned char  pkt_error;
}wp_rx_element_t;


typedef struct aft_config
{
	unsigned int aft_chip_cfg_reg;
	unsigned int aft_dma_control_reg; 
}aft_config_t;



#if defined(__LINUX__)
enum {
	SIOC_AFT_CUSTOMER_ID = SIOC_WANPIPE_DEVPRIVATE,
	SIOC_AFT_SS7_FORCE_RX,
        SIOC_WANPIPE_API 	
};
#endif

#pragma pack(1)

/* the operational statistics structure */
typedef struct {

	/* Data frame transmission statistics */
	unsigned long Data_frames_Tx_count ;	
	/* # of frames transmitted */
	unsigned long Data_bytes_Tx_count ; 	
	/* # of bytes transmitted */
	unsigned long Data_Tx_throughput ;	
	/* transmit throughput */
	unsigned long no_ms_for_Data_Tx_thruput_comp ;	
	/* millisecond time used for the Tx throughput computation */
	unsigned long Tx_Data_discard_lgth_err_count ;	

	/* Data frame reception statistics */
	unsigned long Data_frames_Rx_count ;	
	/* number of frames received */
	unsigned long Data_bytes_Rx_count ;	
	/* number of bytes received */
	unsigned long Data_Rx_throughput ;	
	/* receive throughput */
	unsigned long no_ms_for_Data_Rx_thruput_comp ;	
	/* millisecond time used for the Rx throughput computation */
	unsigned long Rx_Data_discard_short_count ;	
	/* received Data frames discarded (too short) */
	unsigned long Rx_Data_discard_long_count ;	
	/* received Data frames discarded (too long) */
	unsigned long Rx_Data_discard_inactive_count ;	
	/* received Data frames discarded (link inactive) */

	/* Incomming frames with a format error statistics */
	unsigned short Rx_frm_incomp_CHDLC_hdr_count ;	
	/* frames received of with incomplete Cisco HDLC header */
	unsigned short Rx_frms_too_long_count ;		
	/* frames received of excessive length count */

	/* CHDLC link active/inactive and loopback statistics */
	unsigned short link_active_count ;		
	/* number of times that the link went active */
	unsigned short link_inactive_modem_count ;	
	/* number of times that the link went inactive (modem failure) */
	unsigned short link_inactive_keepalive_count ;	
	/* number of times that the link went inactive (keepalive failure) */
	unsigned short link_looped_count ;		
	/* link looped count */

	unsigned long Data_frames_Tx_realign_count; 	

} aft_op_stats_t;

typedef struct {
	unsigned short Rx_overrun_err_count;
	unsigned short Rx_crc_err_count ;		/* receiver CRC error count */
	unsigned short Rx_abort_count ; 	/* abort frames recvd count */
	unsigned short Rx_hdlc_corrupiton;	/* receiver disabled */
	unsigned short Rx_pci_errors;		/* missed tx underrun interrupt count */
        unsigned short Rx_dma_descr_err;   	/*secondary-abort frames tx count */
	unsigned short DCD_state_change_count ; /* DCD state change */
	unsigned short CTS_state_change_count ; /* CTS state change */
	
	unsigned short Tx_pci_errors;		/* missed tx underrun interrupt count */
	unsigned short Tx_dma_errors;		/* missed tx underrun interrupt count */

	unsigned int Tx_pci_latency;		/* missed tx underrun interrupt count */
	unsigned int Tx_dma_len_nonzero;	/* Tx dma descriptor len not zero */

} aft_comm_err_stats_t;

enum wanpipe_aft_api_events {
	WP_API_EVENT_NONE,
	WP_API_EVENT_DTMF,
	WP_API_EVENT_RM_DTMF,
	WP_API_EVENT_RXHOOK,
	WP_API_EVENT_RING,
	WP_API_EVENT_TONE,
	WP_API_EVENT_RING_DETECT,
	WP_API_EVENT_TXSIG_KEWL,
	WP_API_EVENT_TXSIG_START,
	WP_API_EVENT_TXSIG_OFFHOOK,
	WP_API_EVENT_TXSIG_ONHOOK,
	WP_API_EVENT_ONHOOKTRANSFER,
	WP_API_EVENT_SETPOLARITY

};

#define WP_API_EVENT_ENABLE		0x01
#define WP_API_EVENT_DISABLE		0x02
#define WP_API_EVENT_MODE_DECODE(mode)					\
		((mode) == WP_API_EVENT_ENABLE) ? "Enable" :		\
		((mode) == WP_API_EVENT_DISABLE) ? "Disable" :		\
						"(Unknown mode)"

#define WP_API_EVENT_RXHOOK_OFF		0x01
#define WP_API_EVENT_RXHOOK_ON		0x02

#define WP_API_EVENT_RING_PRESENT	0x01
#define WP_API_EVENT_RING_STOP		0x02

/* tone type */
#define	WP_API_EVENT_TONE_DIAL		0x01
#define	WP_API_EVENT_TONE_BUSY		0x02
#define	WP_API_EVENT_TONE_RING		0x03
#define	WP_API_EVENT_TONE_CONGESTION	0x04

typedef struct {
	unsigned char	error_flag;
	unsigned short	time_stamp;
	unsigned char	event_type;
	union {
		struct {
			u_int16_t	channel; 
			union {
				struct {
					unsigned char	rbs_bits;
				} rbs;
				struct {
					unsigned char	state;
				} rxhook;
				struct {
					unsigned char	state;
				} ring;
				struct {
					unsigned char	digit;	/* DTMF: digit  */
					unsigned char	port;	/* DTMF: SOUT/ROUT */
					unsigned char	type;	/* DTMF: PRESET/STOP */
				} dtmf;
			} u_event;
		} wp_api_event;
		unsigned char	reserved[12];
	}hdr_u;
#define wp_api_rx_hdr_event_channel 		hdr_u.wp_api_event.channel
#define wp_api_rx_hdr_event_rxhook_state 	hdr_u.wp_api_event.u_event.rxhook.state
#define wp_api_rx_hdr_event_ring_state 		hdr_u.wp_api_event.u_event.ring.state
#define wp_api_rx_hdr_event_dtmf_digit 		hdr_u.wp_api_event.u_event.dtmf.digit
#define wp_api_rx_hdr_event_dtmf_type 		hdr_u.wp_api_event.u_event.dtmf.type
#define wp_api_rx_hdr_event_dtmf_port 		hdr_u.wp_api_event.u_event.dtmf.port
#define wp_api_rx_hdr_event_ringdetect_state 	hdr_u.wp_api_event.u_event.ring.state
} api_rx_hdr_t;

typedef struct {
        api_rx_hdr_t	api_rx_hdr;
        unsigned char  	data[1];
} api_rx_element_t;

typedef struct {
	unsigned char	type;
	unsigned char	force_tx;
	unsigned char	data[8];
} api_tx_ss7_hdr_t;

typedef struct {
	u_int8_t	type;
	u_int8_t	mode; 
	u_int8_t	tone;
	u_int16_t	channel;
	u_int16_t	polarity;
	u_int16_t	ohttimer;
	
} api_tdm_event_hdr_t;

typedef struct {
	union {
		api_tx_ss7_hdr_t 	ss7;
		api_tdm_event_hdr_t	event;
		unsigned char		reserved[16];
	}u;
#define wp_api_tx_hdr_event_type	u.event.type
#define wp_api_tx_hdr_event_mode 	u.event.mode
#define wp_api_tx_hdr_event_tone 	u.event.tone
#define wp_api_tx_hdr_event_channel 	u.event.channel
#define wp_api_tx_hdr_event_ohttimer 	u.event.ohttimer
#define wp_api_tx_hdr_event_polarity 	u.event.polarity
} api_tx_hdr_t;

typedef struct {
	api_tx_hdr_t 	api_tx_hdr;
	unsigned char	data[1];
} api_tx_element_t;




#pragma pack()

/* Possible RX packet errors */ 
enum {
	WP_FIFO_ERROR_BIT,
	WP_CRC_ERROR_BIT,
	WP_ABORT_ERROR_BIT,
};


#ifdef WAN_KERNEL

#define AFT_MIN_FRMW_VER 0x11
#define AFT_TDMV_FRM_VER 0x11
#define AFT_TDMV_FRM_CLK_SYNC_VER 0x14
#define AFT_TDMV_SHARK_FRM_CLK_SYNC_VER 0x17
#define AFT_TDMV_SHARK_A108_FRM_CLK_SYNC_VER 0x25
#define AFT_56K_MIN_FRMW_VER	0x00

#define AFT_MIN_ANALOG_FRMW_VER 0x05




typedef struct aft_dma_chain
{
	unsigned long	init;
	sdla_dma_addr_t	dma_addr;
	u32		dma_len;
	u32		dma_map_len;
	netskb_t 	*skb;
	u32		index;

	u32		dma_descr;
	u32		len_align;
	u32		reg;

	u8		pkt_error;
	void*		dma_virt;
	u32		dma_offset;
	u32		dma_toggle;
}aft_dma_chain_t;

typedef struct dma_history{
	u8	end;
	u8	cur;
	u8	begin;
	u8	status;
	u8 	loc;
}dma_history_t;

#define MAX_DMA_HIST_SIZE 	10
#define MAX_AFT_DMA_CHAINS 	16
#define MAX_TX_BUF		MAX_AFT_DMA_CHAINS*2+1
#define MAX_RX_BUF		MAX_AFT_DMA_CHAINS*4+1
#define AFT_DMA_INDEX_OFFSET	0x200


typedef struct aft_dma_ring
{
	unsigned char rxdata[128];
	unsigned char txdata[128];
}aft_dma_ring_t;

#define AFT_DMA_RING_MAX 4

typedef struct aft_dma_swring {
	int tx_toggle;
	int rx_toggle;
	aft_dma_ring_t  rbuf[AFT_DMA_RING_MAX];
}aft_dma_swring_t;


typedef struct private_area
{
	wanpipe_common_t 	common;
	sdla_t			*card;

	wan_xilinx_conf_if_t 	cfg;

	wan_skb_queue_t 	wp_tx_pending_list;
	wan_skb_queue_t 	wp_tx_complete_list;
	netskb_t 		*tx_dma_skb;
	u8			tx_dma_cnt;

	wan_skb_queue_t 	wp_rx_free_list;
	wan_skb_queue_t 	wp_rx_complete_list;

	wan_skb_queue_t 	wp_rx_stack_complete_list;
	wan_skb_queue_t 	wp_rx_zap_complete_list;

	unsigned long 		time_slot_map;
	unsigned char 		num_of_time_slots;
	long          		logic_ch_num;

	unsigned char		hdlc_eng;
	unsigned long		dma_status;
	unsigned char		protocol;

	struct net_device_stats	if_stats;

	int 		tracing_enabled;	/* For enabling Tracing */
	unsigned long 	router_start_time;
	unsigned long   trace_timeout;

	unsigned long 	tick_counter;		/* For 5s timeout counter */
	unsigned long 	router_up_time;

	unsigned char  	mc;			/* Mulitcast support on/off */

	unsigned char 	interface_down;

	/* Polling task queue. Each interface
         * has its own task queue, which is used
         * to defer events from the interrupt */
	wan_taskq_t 		poll_task;
	wan_timer_info_t 	poll_delay_timer;

	u8 		gateway;
	u8 		true_if_encoding;

#if defined(__LINUX__)
	/* Entry in proc fs per each interface */
	struct proc_dir_entry	*dent;
#endif
	unsigned char 	udp_pkt_data[sizeof(wan_udp_pkt_t)+10];
	atomic_t 	udp_pkt_len;

	char 		if_name[WAN_IFNAME_SZ+1];

	u8		idle_flag;
	u16		max_idle_size;
	u8		idle_start;

	u8		pkt_error;
	u8		rx_fifo_err_cnt;

	int		first_time_slot;
	int		last_time_slot;
	
	netskb_t	*tx_idle_skb;
	unsigned char	rx_dma;
	unsigned char   pci_retry;
	
	unsigned char	fifo_size_code;
	unsigned char	fifo_base_addr;
	unsigned char 	fifo_size;

	int		dma_mru;
	int		mru,mtu;

	void *		prot_ch;
	int		prot_state;

	wan_trace_t	trace_info;

	/* TE1 Specific Dma Chains */
	unsigned char	tx_chain_indx,tx_pending_chain_indx;
	aft_dma_chain_t tx_dma_chain_table[MAX_AFT_DMA_CHAINS];

	unsigned char	rx_chain_indx,rx_pending_chain_indx;
	aft_dma_chain_t rx_dma_chain_table[MAX_AFT_DMA_CHAINS];
	int		rx_no_data_cnt;

	unsigned long	dma_chain_status;
	unsigned long 	up;
	int		tx_attempts;
	
	aft_op_stats_t  	opstats;
	aft_comm_err_stats_t	errstats;

	unsigned char   *tx_realign_buf;
	unsigned char 	single_dma_chain;
	unsigned char	tslot_sync;

	dma_history_t 	dma_history[MAX_DMA_HIST_SIZE];
	unsigned int	dma_index;

	/* Used by ss7 mangle code */
	api_tx_hdr_t 	tx_api_hdr;
	unsigned char   *tx_ss7_realign_buf;

	int		tdmv_chan;
	unsigned int	tdmv_irq_cfg;

	unsigned char	channelized_cfg;
	unsigned char	tdmv_zaptel_cfg;

	unsigned int	tdmv_rx_delay;
	unsigned char	tdmv_rx_delay_cfg;
	unsigned short	max_tx_bufs;
	unsigned short	max_tx_bufs_orig;

	unsigned int	ss7_force_rx;
	
	unsigned char	lip_atm;

#if defined(__LINUX__)
	wanpipe_tdm_api_dev_t	wp_tdm_api_dev;
#endif
	int	rx_api_crc_bytes;

	netskb_t *rx_rtp_skb;
	netskb_t *tx_rtp_skb;
	u32	  tdm_call_status;
	struct private_area *next;

	aft_dma_swring_t swring;

}private_area_t;


static __inline int aft_decode_dma_status(sdla_t *card, private_area_t *chan, u32 reg)
{
	u32 dma_status=aft_rxdma_hi_get_dma_status(reg);
	u32 data_error=0;

	if (dma_status){

		if (wan_test_bit(AFT_RXDMA_HIDMASTATUS_PCI_M_ABRT,&dma_status)){
			if (WAN_NET_RATELIMIT()){
                	DEBUG_EVENT("%s:%s: Rx Error: Abort from Master: pci fatal error 0x%X!\n",
                                   card->devname,chan->if_name,reg);
			}
                }
		if (wan_test_bit(AFT_RXDMA_HIDMASTATUS_PCI_T_ABRT,&dma_status)){
                        if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s:%s: Rx Error: Abort from Target: pci fatal error 0x%X!\n",
                                   card->devname,chan->if_name,reg);
			}
                }
		if (wan_test_bit(AFT_RXDMA_HIDMASTATUS_PCI_DS_TOUT,&dma_status)){
                        if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s:%s: Rx Error: No 'DeviceSelect' from target: pci fatal error 0x%X!\n",
                                    card->devname,chan->if_name,reg);
			}
                }
		if (wan_test_bit(AFT_RXDMA_HIDMASTATUS_PCI_RETRY,&dma_status)){
                        if (WAN_NET_RATELIMIT()){
			DEBUG_EVENT("%s:%s: Rx Error: 'Retry' exceeds maximum (64k): pci fatal error 0x%X!\n",
                                    card->devname,chan->if_name,reg);
			}
                }

		chan->errstats.Rx_pci_errors++;
		chan->if_stats.rx_errors++;
		card->wandev.stats.rx_errors++;
		return 1;
	}

	if (chan->hdlc_eng){
 
		/* Checking Rx DMA Frame start bit. (information for api) */
		if (!wan_test_bit(AFT_RXDMA_HI_START_BIT,&reg)){
			DEBUG_TEST("%s:%s RxDMA Intr: Start flag missing: MTU Mismatch! Reg=0x%X\n",
					card->devname,chan->if_name,reg);
			chan->if_stats.rx_frame_errors++;
			chan->opstats.Rx_Data_discard_long_count++;
			chan->errstats.Rx_hdlc_corrupiton++;
			card->wandev.stats.rx_errors++;
			return 1;
		}
    
		/* Checking Rx DMA Frame end bit. (information for api) */
		if (!wan_test_bit(AFT_RXDMA_HI_EOF_BIT,&reg)){
			DEBUG_TEST("%s:%s: RxDMA Intr: End flag missing: MTU Mismatch! Reg=0x%X\n",
					card->devname,chan->if_name,reg);
			chan->if_stats.rx_frame_errors++;
			chan->opstats.Rx_Data_discard_long_count++;
			chan->errstats.Rx_hdlc_corrupiton++;
			card->wandev.stats.rx_errors++;
			return 1;
		
       	 	} else {  /* Check CRC error flag only if this is the end of Frame */
        	
			if (wan_test_bit(AFT_RXDMA_HI_FCS_ERR_BIT,&reg)){
                   		DEBUG_TEST("%s:%s: RxDMA Intr: CRC Error! Reg=0x%X Len=%d\n",
                                		card->devname,chan->if_name,reg,
						(reg&AFT_RXDMA_HI_DMA_LENGTH_MASK)>>2);
				chan->if_stats.rx_frame_errors++;
				chan->errstats.Rx_crc_err_count++;
				card->wandev.stats.rx_crc_errors++;
                   		data_error = 1;
               		}

			/* Check if this frame is an abort, if it is
                 	 * drop it and continue receiving */
			if (wan_test_bit(AFT_RXDMA_HI_FRM_ABORT_BIT,&reg)){
				DEBUG_TEST("%s:%s: RxDMA Intr: Abort! Reg=0x%X\n",
						card->devname,chan->if_name,reg);
				chan->if_stats.rx_frame_errors++;
				chan->errstats.Rx_hdlc_corrupiton++;
				card->wandev.stats.rx_frame_errors++;
				data_error = 1;
			}
		
#if 0	
			if (chan->common.usedby != API && data_error){
				return 1;
			}
#else
			if (data_error) {
				return 1;
			}
#endif	

		}
	}

	return 0;
}



void 	aft_free_logical_channel_num (sdla_t *card, int logic_ch);
void 	aft_dma_max_logic_ch(sdla_t *card);
void 	aft_fe_intr_ctrl(sdla_t *card, int status);
void 	__aft_fe_intr_ctrl(sdla_t *card, int status);
void 	aft_wdt_set(sdla_t *card, unsigned char val);
void 	aft_wdt_reset(sdla_t *card);



#endif


#endif
