#ifndef __WAN_AFTUP_H
# define __WAN_AFTUP_H

#define MAX_IFNAME_LEN	20
#define MAX_HWINFO_LEN	100

typedef struct wan_aftup_ {

	char		if_name[MAX_IFNAME_LEN];
	int		board_id;
	int		chip_id;
	unsigned char	core_rev;
	unsigned char	core_id;
	int		revision_id;
	char		flash_rev;
#if 0
	char	firmware[100];
	char	prefix_fw[100];
#endif
	char		hwinfo[MAX_HWINFO_LEN];

	wan_aft_cpld_t	cpld;

	WAN_LIST_ENTRY(wan_aftup_) next;
} wan_aftup_t;


#endif	/* __WAN_AFTUP_H */
