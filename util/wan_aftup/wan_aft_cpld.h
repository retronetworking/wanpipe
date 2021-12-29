#ifndef __WAN_AFT_CPLD_H
# define __WAN_AFT_CPLD_H


#define MEMORY_TYPE_SRAM	0x00
#define MEMORY_TYPE_FLASH	0x01
#define MASK_MEMORY_TYPE_SRAM	0x10
#define MASK_MEMORY_TYPE_FLASH	0x20

#define DEF_SECTOR_FLASH	0x00
#define USER_SECTOR_FLASH	0x01
#define MASK_DEF_SECTOR_FLASH	0x00
#define MASK_USER_SECTOR_FLASH	0x04

#define AFT_CORE_SIZE		234456
#define AFT4_CORE_SIZE		212392
#define AFT_SHARK_CORE_SIZE	402936

typedef struct {
	void	*private;
	int	adptr_type;
//	int	board_id;
//	int	chip_id;
	int	adptr_subtype;
} wan_aft_cpld_t;

#endif /* __WAN_AFT_CPLD_H*/
