#ifndef __WAN_AFTUP_H
# define __WAN_AFTUP_H


typedef struct wan_aftup_ {
	char	if_name[20];
	char	flash_rev;
	char	firmware[100];
	char	prefix_fw[100];
	char	hwinfo[100];

	wan_aft_cpld_t	cpld;

	WAN_LIST_ENTRY(wan_aftup_) next;
} wan_aftup_t;

#endif	/* __WAN_AFTUP_H */
