/*****************************************************************************
* sdla_xilinx.h	WANPIPE(tm) S51XX Xilinx Hardware Support
* 		
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*
* Copyright:	(c) 2003 Sangoma Technologies Inc.
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

#define AFT_PORT0_OFFSET		0x0000
#define AFT_PORT1_OFFSET		0x4000
#define AFT_PORT2_OFFSET		0x8000
#define AFT_PORT3_OFFSET		0xC000

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

#define AFT_MCPU_INTERFACE_RW		0x54

#define AFT_WDT_CTRL_REG		0x48 

#define AFT_DATA_MUX_REG		0x4C 

#define AFT_FIFO_MARK_REG		0x50

# define AFT_CHIPCFG_TE1_CFG_BIT	0
# define AFT_CHIPCFG_SFR_EX_BIT		1
# define AFT_CHIPCFG_SFR_IN_BIT		2
# define AFT_CHIPCFG_FE_INTR_CFG_BIT	3
# define AFT_CHIPCFG_SECURITY_CFG_BIT	6
# define AFT_CHIPCFG_RAM_READY_BIT	7
# define AFT_CHIPCFG_HDLC_CTRL_RDY_BIT  8

# define AFT_CHIPCFG_P1_TDMV_INTR_BIT	14	
# define AFT_CHIPCFG_P2_TDMV_INTR_BIT	15	
# define AFT_CHIPCFG_P3_TDMV_INTR_BIT	16	
# define AFT_CHIPCFG_P4_TDMV_INTR_BIT	17	

# define AFT_CHIPCFG_P1_WDT_INTR_BIT	18
# define AFT_CHIPCFG_P2_WDT_INTR_BIT	19
# define AFT_CHIPCFG_P3_WDT_INTR_BIT	20
# define AFT_CHIPCFG_P4_WDT_INTR_BIT	21

# define AFT_CHIPCFG_FE_INTR_STAT_BIT	22
# define AFT_CHIPCFG_SECURITY_STAT_BIT	23

# define AFT_CHIPCFG_HDLC_INTR_MASK	0x0F
# define AFT_CHIPCFG_HDLC_INTR_SHIFT	24

# define AFT_CHIPCFG_DMA_INTR_MASK	0x0F
# define AFT_CHIPCFG_DMA_INTR_SHIFT	28

# define AFT_CHIPCFG_WDT_INTR_MASK	0x0F
# define AFT_CHIPCFG_WDT_INTR_SHIFT	18

# define AFT_CHIPCFG_TDMV_INTR_MASK	0x0F
# define AFT_CHIPCFG_TDMV_INTR_SHIFT	14

#  define AFT_CHIPCFG_WDT_FE_INTR_STAT	0
#  define AFT_CHIPCFG_WDT_TX_INTR_STAT  1
#  define AFT_CHIPCFG_WDT_RX_INTR_STAT	2

static __inline u32
aft_chipcfg_get_hdlc_intr_stats(u32 reg)
{
	reg=reg>>AFT_CHIPCFG_HDLC_INTR_SHIFT;
	reg&=AFT_CHIPCFG_HDLC_INTR_MASK;
	return reg;
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

# define AFT_LCFG_FE_CLK_ROUTE_BIT	27

# define AFT_LCFG_FE_CLK_SOURCE_MASK	0x03
# define AFT_LCFG_FE_CLK_SOURCE_SHIFT	28

# define AFT_LCFG_GREEN_LED_BIT		30
# define AFT_LCFG_RED_LED_BIT		31


static __inline void
aft_lcfg_fe_clk_source(u32 *reg, u32 src)
{
	*reg&=~(AFT_LCFG_FE_CLK_SOURCE_MASK<<AFT_LCFG_FE_CLK_SOURCE_SHIFT);
	*reg|=(src&AFT_LCFG_FE_CLK_SOURCE_MASK)<<AFT_LCFG_FE_CLK_SOURCE_SHIFT;
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
	case 32:
		size=0x02;
		break;
	case 64:
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

# define AFT_TXDMA_HI_DMA_LENGTH_MASK	0x3FF
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

# define AFT_RXDMA_HI_DMA_LENGTH_MASK	0x3FF
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


#define SECURITY_CPLD_REG       0x09
#define CUSTOMER_CPLD_ID_REG	0x0A

#define SECURITY_CPLD_MASK	0x03
#define SECURITY_CPLD_SHIFT	0x02

#define SECURITY_1LINE_UNCH	0x00
#define SECURITY_1LINE_CH	0x01
#define SECURITY_2LINE_UNCH	0x02
#define SECURITY_2LINE_CH	0x03

#define WAN_AFT_SECURITY(sec)					\
	((sec) == SECURITY_1LINE_UNCH) ? "Unchannelized!" :	\
	((sec) == SECURITY_1LINE_CH)   ? "Channelized!" :	\
	((sec) == SECURITY_2LINE_UNCH) ? "Unchannelized!" :	\
	((sec) == SECURITY_2LINE_CH)   ? "Channelized!" :	\
						"Unknown"

/* -------------------------------------- */

#define WRITE_DEF_SECTOR_DSBL   0x01
#define FRONT_END_TYPE_MASK     0x38

/* Moved to sdlasfm.h 
#define AFT_BIT_DEV_ADDR_CLEAR	0x600
#define AFT_BIT_DEV_ADDR_CPLD	0x200

#define AFT4_BIT_DEV_ADDR_CLEAR	0x800
#define AFT4_BIT_DEV_ADDR_CPLD	0x800
*/

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
		return 128;
	}else if (mtu <= 256){
		return 256;
	}else if (mtu <= 512){
		return 512;
	}else if (mtu <= 1024){
		return 1024;
	}else if (mtu <= 2048){
		return 2048;
	}else if (mtu <= 4096){
		return 4096;
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
	AFT_MODEM_STATUS
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
	SIOC_AFT_SS7_FORCE_RX
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


typedef struct {
	unsigned char	error_flag;
	unsigned short	time_stamp;
	unsigned char	reserved[13];
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
	union {
		api_tx_ss7_hdr_t ss7;
		unsigned char 	 reserved[16];
	}u;
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



#endif
