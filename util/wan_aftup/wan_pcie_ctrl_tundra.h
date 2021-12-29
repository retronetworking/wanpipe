#ifndef __WAN_PCIE_CTRL_TUNDRA_H
#define __WAN_PCIE_CTRL_TUNDRA_H


#define TUNDRA_VENDOR_ID		0x10E3
#define TUNDRA_DEVICE_ID		0x8111

#define TUNDRA_EECTRL_REG_OFF 		0x0AC

#define TUNDRA_EECTRL_BIT_CMD_VLD 		(1<<24)
#define	TUNDRA_EECTRL_BIT_BUSY			(1<<25)

#define TUNDRA_EECTRL_BIT_ADD_WIDTH		(1<<26)


#define TUNDRA_EECTRL_BIT_CMD_READ		(1<<28)
#define TUNDRA_EECTRL_BIT_CMD_WRITE		(2<<28)

#define TUNDRA_EECTRL_ADD_BIT_SHIFT(add)	(add<<8)


#endif /*__WAN_PCIE_CTRL_TUNDRA_H */