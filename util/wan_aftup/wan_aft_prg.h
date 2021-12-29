#ifndef __WAN_AFT_PRG_H
# define __WAN_AFT_PRG_H


#define MEMORY_TYPE_SRAM	0x00
#define MEMORY_TYPE_FLASH	0x01
#define MASK_MEMORY_TYPE_SRAM	0x10
#define MASK_MEMORY_TYPE_FLASH	0x20

#define DEF_SECTOR_FLASH	0x00
#define USER_SECTOR_FLASH	0x01

#define CMD_FLASH_UNKNOWN	0x00
#define CMD_FLASH_VERIFY	0x01
#define CMD_FLASH_PRG		0x02

#define SECTOR_FLASH_DECODE(sector_type)				\
		(sector_type == USER_SECTOR_FLASH) ? "USER" :		\
		(sector_type == DEF_SECTOR_FLASH) ? "BOOT" : "UNKNOWN"
		
#ifndef AFT_CHIP_UNKNOWN
# define AFT_CHIP_UNKNOWN	0x0000
# define AFT_CHIP_OLD_X300	0x0300
# define AFT_CHIP_OLD_X400	0x0400
# define AFT_CHIP_X200		0x0020
# define AFT_CHIP_X300		0x0030
# define AFT_CHIP_X400		0x0040
# define AFT_CHIP_X1000		0x0100
#endif

//#define AFT_CORE_SIZE		234456
///#define AFT4_CORE_SIZE		212392
//#define AFT_SHARK_CORE_SIZE	402936
#define AFT_CORE_SIZE		234456
#define AFT4_CORE_SIZE		212392
#define AFT_CORE_X200_SIZE	130952
#define AFT_CORE_X400_SIZE	212392
#define AFT_CORE_X1000_SIZE	402936


struct wan_aft_cpld_;
typedef struct {
	int	(*reset)(struct wan_aft_cpld_*);
	int	(*is_protected)(struct wan_aft_cpld_*,int stype);
	int	(*flash_id)(struct wan_aft_cpld_*, int mtype, int stype, int*);
	int 	(*reload)(struct wan_aft_cpld_*, int stype);
	int	(*prg_byte)(struct wan_aft_cpld_*, int, unsigned long, unsigned char);
	unsigned char	(*read_byte)(struct wan_aft_cpld_*, int, int, unsigned long);
	int	(*erase)(struct wan_aft_cpld_*, int stype, int verify);
} aftup_flash_iface_t;

typedef struct {
	int	id;
	int	size;
} aftup_flash_t;

typedef struct {
	int		board_id;
	int		chip_id;
	unsigned char	core_id;
	unsigned char	man_id;
	unsigned char	dev_id;
	char*		fw_prefix;
	char*		fw_template;
	int		flash_size;
	char		firmware[MAXPATHLEN];
//#endif
} aft_core_info_t;

typedef struct wan_aft_cpld_ {
	void	*private;
	int	adptr_type;
//	int	board_id;
//	int	chip_id;
	int	adptr_subtype;
	int	flash_index;

//	aftup_flash_t		*flash;
	aft_core_info_t		*core_info;
	aftup_flash_iface_t	*iface;
} wan_aft_cpld_t;


#endif /* __WAN_AFT_PRG_H*/
