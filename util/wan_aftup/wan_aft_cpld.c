#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#if defined(__LINUX__)
# include <linux/if.h>
# include <linux/if_packet.h>
# include <linux/wanpipe_defines.h>
# include <linux/sdlasfm.h>
# include <linux/wanpipe_cfg.h>
#else
# include <net/if.h>
# include <net/wanpipe_defines.h>
# include <net/sdlasfm.h>
# include <net/wanpipe_cfg.h>
#endif

#include "wan_aft_cpld.h"

#define JP8_VALUE               0x02
#define JP7_VALUE               0x01
#define SW0_VALUE               0x04
#define SW1_VALUE               0x08   

#define WRITE_DEF_SECTOR_DSBL   0x01
#define FRONT_END_TYPE_MASK     0x38

#define AFT_BIT_DEV_ADDR_CLEAR	0x600
#define AFT_BIT_DEV_ADDR_CPLD	0x200

#define AFT4_BIT_DEV_ADDR_CLEAR	0x800 /* QUADR */
#define AFT4_BIT_DEV_ADDR_CPLD	0x800 /* QUADR */

#define BIT_A18_SECTOR_SA4_SA7	0x20
#define USER_SECTOR_START_ADDR	0x40000

/* Manufacturer code */
#define MCODE_ST	0x20

/* Device code */
#define DCODE_M29W040B	0xE3
#define DCODE_M29W800DT	0xD7
#define DCODE_M29W800DB	0x5B

#define M29W040B_FID	0
#define M29W800DT_FID	1
#define M29W800DB_FID	2

typedef struct {
	unsigned long	saddr;
	unsigned long	len;
	unsigned int	sector_type;
} block_addr_t;

static block_addr_t 
block_addr_M29W040B[19] = 
	{
		{ 0x00000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x10000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x20000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x30000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x40000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0x50000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0x60000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0x70000, 0xFFFF, USER_SECTOR_FLASH }
	};

static block_addr_t 
block_addr_M29W800DT[19] = 
	{
		{ 0x00000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x10000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x20000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x30000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x40000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x50000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x60000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x70000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x80000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0x90000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xA0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xB0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xC0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xD0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xE0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xF0000, 0x7FFF, USER_SECTOR_FLASH },
		{ 0xF8000, 0x1FFF, USER_SECTOR_FLASH },
		{ 0xFA000, 0x1FFF, USER_SECTOR_FLASH },
		{ 0xFC000, 0x3FFF, USER_SECTOR_FLASH }
	};

static block_addr_t 
block_addr_M29W800DB[19] = 
	{
		{ 0x00000, 0x3FFF, DEF_SECTOR_FLASH },
		{ 0x04000, 0x1FFF, DEF_SECTOR_FLASH },
		{ 0x06000, 0x1FFF, DEF_SECTOR_FLASH },
		{ 0x08000, 0x7FFF, DEF_SECTOR_FLASH },
		{ 0x10000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x20000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x30000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x40000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x50000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x60000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x70000, 0xFFFF, DEF_SECTOR_FLASH },
		{ 0x80000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0x90000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xA0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xB0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xC0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xD0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xE0000, 0xFFFF, USER_SECTOR_FLASH },
		{ 0xF0000, 0xFFFF, USER_SECTOR_FLASH },
	};


#define FLASH_NAME_LEN	20

struct flash_spec_t {
	char		name[FLASH_NAME_LEN];
	unsigned short	data_reg;
	unsigned short	addr1_reg;
	unsigned char	addr1_mask;
	unsigned short	addr2_reg;
	unsigned char	addr2_mask;
	unsigned short	addr3_reg;
	unsigned char	addr3_mask;
	unsigned long	def_start_adr;
	unsigned long	user_start_adr;
	unsigned short	user_mask_sector_flash;
	int		sectors_no;
	block_addr_t	*block_addr;
} flash_spec[] = 
	{
		{ 
			"M29W040B",
			0x04, 0x05, 0xFF, 0x06, 0xFF, 0x07, 0x3,
			0x00000, 0x40000, 0x04, 
			8, block_addr_M29W040B
		},
		{ 
			"M29W800DT",
			0x04, 0x05, 0xFF, 0x06, 0xFF, 0x07, 0x7,
			0x00000, 0x80000, 0x08, 
			19, block_addr_M29W800DT
		},
		{
			"M29W800DB",
			0x04, 0x05, 0xFF, 0x06, 0xFF, 0x07, 0x7,
			0x00000, 0x80000, 0x08, 
			19, block_addr_M29W800DB
		}
	};

extern int	verbose;
extern int	card_type;

extern int	progress_bar(char*);
/*
 ******************************************************************************
			  FUNCTION PROTOTYPES
 ******************************************************************************
*/
		
extern int exec_read_cmd(void*,unsigned int, unsigned int, unsigned int*);
extern int exec_write_cmd(void*,unsigned int, unsigned int, unsigned int);
extern void hit_any_key(void);

static unsigned int
write_cpld(wan_aft_cpld_t *cpld, unsigned short cpld_off,unsigned short cpld_data)
{
	if (cpld->adptr_type == A104_ADPTR_4TE1){
		cpld_off |= AFT4_BIT_DEV_ADDR_CPLD; 
	}else{
		cpld_off &= ~AFT_BIT_DEV_ADDR_CLEAR; 
		cpld_off |= AFT_BIT_DEV_ADDR_CPLD; 
	}
	exec_write_cmd(cpld->private, 0x46, 2, cpld_off);
	exec_write_cmd(cpld->private, 0x44, 2, cpld_data);
	return 0;
}

static unsigned int
read_cpld(wan_aft_cpld_t *cpld, unsigned short cpld_off)
{
	unsigned int cpld_data;
		
	if (cpld->adptr_type == A104_ADPTR_4TE1){
		cpld_off |= AFT4_BIT_DEV_ADDR_CPLD; 
	}else{
		cpld_off &= ~AFT_BIT_DEV_ADDR_CLEAR; 
		cpld_off |= AFT_BIT_DEV_ADDR_CPLD; 
	}
	exec_write_cmd(cpld->private, 0x46, 2, cpld_off);
	if (exec_read_cmd(cpld->private, 0x44, 4, &cpld_data) == 0){
		return cpld_data;
	}else{
		return 0;
	}
}

static unsigned int
write_flash(wan_aft_cpld_t *cpld, int stype, int mtype, unsigned long off,unsigned char data)
{
	unsigned char	offset;

	//Writing flash address to cpld
	offset = off & 0xFF;
	write_cpld(cpld, 0x05, offset);
	offset = (off >> 8) & 0xFF;
	write_cpld(cpld, 0x06, offset);
	offset = (off >> 16) & 0x3;
	if (mtype == MEMORY_TYPE_SRAM){
		offset |= MASK_MEMORY_TYPE_SRAM;
	}else if (mtype == MEMORY_TYPE_FLASH){
		offset |= MASK_MEMORY_TYPE_FLASH;
	}else{
		return -EINVAL;
	}
	if (stype == USER_SECTOR_FLASH){
		offset |= MASK_USER_SECTOR_FLASH;
	}else if (stype == DEF_SECTOR_FLASH){
		;
	}else{
		return -EINVAL;
	}
	write_cpld(cpld, 0x07, offset);
	write_cpld(cpld, 0x04, data);
        write_cpld(cpld, 0x07, 0x00);  // disable CS signal for the Boot FLASH/SRAM

	return 0;
}

static unsigned char
read_flash(wan_aft_cpld_t *cpld, int stype, int mtype, unsigned long off)
{
	unsigned char offset;
        unsigned char data;

	//Writing flash address to cpld
	offset = off & 0xFF;
	write_cpld(cpld, 0x05, offset);
	offset = (off >> 8) & 0xFF;
	write_cpld(cpld, 0x06, offset);
	offset = (off >> 16) & 0x3;
        offset |= MASK_MEMORY_TYPE_FLASH;
//           
//	if (memory_type == MEMORY_TYPE_SRAM){
//		offset |= MASK_MEMORY_TYPE_SRAM;
//	}else if (memory_type == MEMORY_TYPE_FLASH){
//		offset |= MASK_MEMORY_TYPE_FLASH;
//	}else{
//		return -EINVAL;
//	}
	if (stype == USER_SECTOR_FLASH){
		offset |= MASK_USER_SECTOR_FLASH;
	}else if (stype == DEF_SECTOR_FLASH){
		;
	}else{
		return -EINVAL;
	}

	write_cpld(cpld, 0x07, offset);
        data = read_cpld(cpld, 0x04);
        write_cpld(cpld, 0x07, 0x00); // Disable CS for the Boot FLASH/SRAM
	return data;
}

int cpld_set_clock(wan_aft_cpld_t *cpld, unsigned char media)
{
	int i = 0;
	write_cpld(cpld, 0x00, 0x00);
	for(i=0;i<400;i++);
	switch(media){
	case WAN_MEDIA_T1:
		write_cpld(cpld, 0x00, 0x03);
		break;

	case WAN_MEDIA_E1:
		write_cpld(cpld, 0x00, 0x01);
		break;
	}
	for(i=0;i<400;i++);
	return 0;
}

int led_test(wan_aft_cpld_t *cpld)
{
	int i = 0;
	unsigned int j = 0, jj = 0;

	// Configuration LED
	write_cpld(cpld, 0x01, 0xc);

	for(i=0;i<20;i++){
		// Turn ON RED
		write_cpld(cpld, 0x01, 0xd);
		for(j=0;j<100000;j++)	
			for(jj=0;jj<100;jj++);	

		// Turn ON GREEN
		write_cpld(cpld, 0x01, 0xe);
		for(j=0;j<100000;j++)
			for(jj=0;jj<100;jj++);	
	}	
	// Turn OFF RED/GREEN
	write_cpld(cpld, 0x01, 0xf);

	return 0;
}

static int
erase_sector_flash(wan_aft_cpld_t *cpld, int stype, int verify)
{
	unsigned long offset = 0x00;
	unsigned char data = 0x00;
	int sector_no = 0;
	int cnt=0;
	unsigned char	val;

	// Checking write enable to the Default Boot Flash Sector 
        if(stype == DEF_SECTOR_FLASH){
		val = read_cpld(cpld, 0x09);
		if (val & WRITE_DEF_SECTOR_DSBL){
			printf("erase_sector: Default sector protected!\n");
			return -EINVAL;
		}
	}    

	if (stype == USER_SECTOR_FLASH){
                offset  = USER_SECTOR_START_ADDR;
       	}
	for(sector_no = 0; sector_no < 4; sector_no++){
                offset += (sector_no << 16); 
		write_flash(cpld, stype, MEMORY_TYPE_FLASH, 0x5555, 0xAA);
		write_flash(cpld, stype, MEMORY_TYPE_FLASH, 0x2AAA, 0x55);
		write_flash(cpld, stype, MEMORY_TYPE_FLASH, 0x5555, 0x80);
		write_flash(cpld, stype, MEMORY_TYPE_FLASH, 0x5555, 0xAA);
		write_flash(cpld, stype, MEMORY_TYPE_FLASH, 0x2AAA, 0x55);
		write_flash(cpld, stype, MEMORY_TYPE_FLASH, offset, 0x30);
		do{
//MF			for(i=0;i<100000;i++);
			data = read_flash(
					cpld, stype,
					MEMORY_TYPE_FLASH,
					offset);
			if (data & 0x80){
				break;
			}else if (data & 0x20){
				data = read_flash(
						cpld, stype, 
						MEMORY_TYPE_FLASH, 
						offset);
				if (data & 0x80){
					break;
				}else{
					printf("erase_sector_flash: Failed!\n");
					printf("erase_sector_flash: Sector=%d!\n",
					   (stype == USER_SECTOR_FLASH) ? 
							sector_no+4:sector_no);
					return -EINVAL;
				}
			}
		} while(1);

		progress_bar("\tErasing sectors\t\t\t");
	}
	printf("\r\tErasing sectors\t\t\tPassed\n");
	if (!verify) return 0;

	// Verify that flash is 0xFF
	offset = 0x00;
	if (stype == USER_SECTOR_FLASH){
		offset = USER_SECTOR_START_ADDR;
	}
//        stype = DEF_SECTOR_FLASH;    // M.F debug
//        offset = 0;  

	for(cnt = 0; cnt < 0x40000; cnt++){
//MF		for(i=0;i<10000;i++);
		data = read_flash(cpld, stype, MEMORY_TYPE_FLASH, offset+cnt);	
		if (data != 0xFF){
			printf(" Failed to compare! %05lx -> %02x \n",
						offset+cnt,data);
			return -EINVAL;
		}
		if ((cnt & 0x1FFF) == 0x1000){
			progress_bar("\tErasing sectors (verification)\t");
		}
	}
	printf("\r\tErasing sectors (verification)\tPassed\n");
	return 0;
}

static int
prg_flash_byte(wan_aft_cpld_t *cpld, int stype, unsigned long off32, unsigned char data)
{
	unsigned char data1 = 0x00;
	unsigned char	val;

	// Checking write enable to the Default Boot Flash Sector 
        if(stype == DEF_SECTOR_FLASH){
		val = read_cpld(cpld, 0x09);
		if (val & WRITE_DEF_SECTOR_DSBL){
			printf("prg_flash_byte: Default sector protected!\n");
			return -EINVAL;
		}
	}    
	write_flash(cpld, stype, MEMORY_TYPE_FLASH, 0x5555, 0xAA);
	write_flash(cpld, stype, MEMORY_TYPE_FLASH, 0x2AAA, 0x55);
	write_flash(cpld, stype, MEMORY_TYPE_FLASH, 0x5555, 0xA0);
	write_flash(cpld, stype, MEMORY_TYPE_FLASH, off32, data);
	do{
//MF		for(i=0;i<1000;i++);
		data1 = read_flash(cpld, stype, MEMORY_TYPE_FLASH, off32);
		if ((data1 & 0x80) == (data & 0x80)){
			break;
		}else if (data1 & 0x20){
			data1 = read_flash(
					cpld, stype, 
					MEMORY_TYPE_FLASH, 
					off32);
			if ((data1 & 0x80) == (data & 0x80)){
				break;
			}else{
				printf("prg_flash_byte: Failed!\n");
				return -EINVAL;
			}
		}
	} while(1);
	return 0;
}

static int reset_flash(wan_aft_cpld_t *cpld)
{
	write_flash(cpld, DEF_SECTOR_FLASH, MEMORY_TYPE_FLASH, 0x00, 0xF0);
	return 0;
}

static unsigned long filesize(FILE* f)
{
	unsigned long size = 0;
	unsigned long cur_pos = 0;

	cur_pos = ftell(f);
	if ((cur_pos != -1l) && !fseek(f, 0, SEEK_END)){
		size = ftell(f);
		fseek(f, cur_pos, SEEK_SET);
	}
	return size;
}

#define UNKNOWN_FILE	0
#define HEX_FILE	1
#define BIN_FILE	2
static int get_file_type(char *filename)
{
	char	*ext;
	int	type = UNKNOWN_FILE;

	ext = strstr(filename, ".");
	if (ext != NULL){
		ext++;
		if (!strncasecmp(ext, "BIN", 3)){
			type = BIN_FILE;
		}else if (!strncasecmp(ext, "HEX", 3)){
			type = HEX_FILE;
		}else if (!strncasecmp(ext, "MCS", 3)){
			type = HEX_FILE;
		}else{
			type = BIN_FILE;
		}
	}else{
		/* By default, file format is Binary */
		type = BIN_FILE;
	}
	return type;

}
static long read_bin_data_file(char* filename, char** data)
{
	FILE		*f = NULL;
	char		*buf = NULL;
	unsigned long	fsize = 0;

	f = fopen(filename, "rb");
	if (f == NULL){
		printf("read_bin_data_file: Can't open data file %s!\n",
					filename);
		return -EINVAL;
	}
	fsize = filesize(f);
	if (!fsize){
		printf("read_bin_data_file: Data file %s is empty!\n",
					filename);
		return -EINVAL;
	}
	buf = malloc(fsize);
	if (buf == NULL){
		printf("read_bin_data_file: Can't allocate memory for data!\n");
		return -ENOMEM;
	}
	if (fread(buf, 1, fsize, f) < fsize){
		printf("read_bin_data_file: Can't copy all data to local buffer!\n");
		free(buf);
		return -EINVAL;
	}
	*data = buf;
	fclose(f);
	return fsize;
	
}

#if 0
static int
update_man(wan_aft_cpld_t *cpld, int memory_type, char* filename)
{
	char		*data = NULL;
	unsigned long	offset = 0, findex = 0;
	long		fsize = 0;
	char		val = 0x00;
        int		stype =   DEF_SECTOR_FLASH; 

	// Checking write enable to the Default Boot Flash Sector 
	fsize = read_bin_data_file(filename, &data);
	if (fsize < 0){
		return -EINVAL;
	}
	if (fsize > 0x80000){
		printf("update_man: Data file is too big!\n");
		free(data);
		return -EINVAL;
	}

        stype = DEF_SECTOR_FLASH;
	erase_sector_flash(cpld, stype, 1);

	stype = USER_SECTOR_FLASH;
	erase_sector_flash(cpld, stype, 1);

        stype = DEF_SECTOR_FLASH;
	offset = 0;
	printf("update_man: Programming      BOOT    ");
	fflush(stdout);
	while(findex < fsize/2){
		prg_flash_byte(cpld, stype, offset+findex, data[findex]);
		findex++;
		if ((findex & 0x1FFF) == 0x1000){
			printf(".");
			fflush(stdout);
		}
	}
	printf("Passed\n");

	stype = USER_SECTOR_FLASH;
	offset = USER_SECTOR_START_ADDR;

	printf("update_man: Programming      USER    ");
	while(findex < fsize){
		prg_flash_byte(cpld, stype, offset+findex, data[findex]);
		findex++;
		if ((findex & 0x1FFF) == 0x1000){
			printf(".");
			fflush(stdout);
		}
	}
	printf("Passed\n");
	reset_flash(cpld);

        stype = DEF_SECTOR_FLASH;
	offset = 0;
	findex = 0;
	printf("update_man: Verification     BOOT    ");
	fflush(stdout);
	while(findex < fsize/2){
		val = read_flash(cpld, stype, MEMORY_TYPE_FLASH, offset+findex);
		if (val != data[findex]){
			printf("update_man: Failed to compare data (%lx)!\n",
					findex);
			free(data);
			return -EINVAL;
		}
		if ((findex & 0x1FFF) == 0x1000){
			printf(".");
			fflush(stdout);
		}
		findex++;
	}
	printf("Passed\n");

	stype = USER_SECTOR_FLASH;
	offset = USER_SECTOR_START_ADDR;
	printf("update_man: Verification     USER    ");

	while(findex < fsize){
		val = read_flash(cpld, stype, MEMORY_TYPE_FLASH, offset+findex);
		if (val != data[findex]){
			printf("update: Failed to compare data (%lx)!\n",
					findex);
			free(data);
			return -EINVAL;
		}
		if ((findex & 0x1FFF) == 0x1000){
			printf(".");
			fflush(stdout);
		}
		findex++;
	}
	printf("Passed\n");
	fflush(stdout);
	free(data);
	return 0;
}
#endif

#define CMD_FLASH_UNKNOWN		0x00
#define CMD_FLASH_VERIFY		0x01
#define CMD_FLASH_PRG			0x02

#define MIN_HEX_LINE_LEN		11
#define MAX_HEX_LINE_LEN		200
#define MAX_HEXNUM_LEN			5

#define HEX_START_LINE			':'
#define HEX_DATA_RECORD			0x00
#define HEX_END_FILE_RECORD		0x01
#define HEX_EXT_SEG_ADDR_RECORD		0x02
#define HEX_START_SEG_ADDR_RECORD	0x03
#define HEX_EXT_LIN_ADDR_RECORD		0x04
#define HEX_START_LIN_ADDR_RECORD	0x05
int parse_hex_line(char *line, int data[], int *addr, int *num, int *type)
{
	int	sum, len, cksum;
	char	*ptr = line;
	
	*num = 0;
	if (*ptr++ != HEX_START_LINE){
		printf("Wrong HEX file format!\n");
		return -EINVAL;
	}
	if (strlen(line) < MIN_HEX_LINE_LEN){
		printf("Wrong HEX line length (too small)!\n");
		return -EINVAL;
	}
	if (!sscanf(ptr, "%02x", &len)){
		printf("Failed to read record-length field\n");
	       	return -EINVAL;
	}
	ptr += 2;
	if (strlen(line) < (MIN_HEX_LINE_LEN + (len * 2))){
		printf("Wrong HEX line length (too small)!\n");
	       	return -EINVAL;
	}
	if (!sscanf(ptr, "%04x", addr)){
		printf("Failed to read record-addr field\n");
	       	return -EINVAL;
	}
	ptr += 4;
	if (!sscanf(ptr, "%02x", type)){
		printf("Failed to read record-type field\n");
		return -EINVAL;
	}
	ptr += 2;
	sum = (len & 255) + ((*addr >> 8) & 255) + (*addr & 255) + (*type & 255);
	while(*num != len){
		if (!sscanf(ptr, "%02x", &data[*num])){
			printf("Failed to read record-data field (%d)\n",
					*num);
		       	return -EINVAL;
		}
		ptr += 2;
		sum += data[*num] & 255;
		(*num)++;
		if (*num >= MAX_HEX_LINE_LEN){
			printf("Wrong HEX line length (too long)!\n");
		       	return -EINVAL;
		}
	}
	if (!sscanf(ptr, "%02x", &cksum)) return -EINVAL;
	if (((sum & 255) + (cksum & 255)) & 255){
	       	printf("Wrong hex checksum value!\n");
	       	return -EINVAL;
	}
	return 0;
}


static int
aft_flash_hex_file(wan_aft_cpld_t *cpld, int stype, char *filename, int cmd)
{
	FILE		*f;
	char		line[MAX_HEX_LINE_LEN];
	int		data[MAX_HEX_LINE_LEN];
	int		offset = 0, seg = 0, addr;
	int		len, type, i, err;
	unsigned char	val;
	
	if (cmd == CMD_FLASH_PRG){
		erase_sector_flash(cpld, stype, 0);
	}
	f = fopen(filename, "r");
	if (f == NULL){
		printf("Can't open data file %s!\n",
					filename);
		return -EINVAL;
	}
	if (stype == USER_SECTOR_FLASH){
		offset = USER_SECTOR_START_ADDR;
	}
	while(!feof(f)){
		if (!fgets(line, MAX_HEX_LINE_LEN, f)){
			if (feof(f)){
				break;
			}
			fclose(f);
			return -EINVAL;
		}
		if (parse_hex_line(line, data, &addr, &len, &type)){
			printf("Failed to parse HEX line!\n");
			fclose(f);
			return -EINVAL;
		}
		switch(type){
		case HEX_EXT_LIN_ADDR_RECORD:
			/* Save extended address */
			seg = data[0] << 24;
			seg |= data[1] << 16;
			continue;
		case HEX_EXT_SEG_ADDR_RECORD:
			seg = data[0] << 12;
			seg = data[1] << 4;
			continue;
		}
		addr += offset;
		i = 0;
		while(i < len){
			if (cmd == CMD_FLASH_PRG){
				err = prg_flash_byte(
						cpld,
						stype,
						seg+addr+i,
						data[i]);
				if (err){
					printf("\r\tUpdating flash\t\t\tFailed(%x)\n",
								addr+i);
					fclose(f);
					return -EINVAL;
				}
			}else{
				val = read_flash(
						cpld, stype, 
						MEMORY_TYPE_FLASH,
						seg+addr+i);
				if (val != (unsigned char)data[i]){
					printf("\r\tVerification\t\t\tFailed(%x)\n",
								addr+i);
					fclose(f);
					return -EINVAL;
				}
			}
			i++;
		}
		if (cmd == CMD_FLASH_PRG){
			progress_bar("\tUpdating flash\t\t\t");
		}else{
			progress_bar("\tVerification\t\t\t");
		}
	}
	if (cmd == CMD_FLASH_PRG){
		printf("\r\tUpdating flash\t\t\tPassed\n");
	}else{
		printf("\r\tVerification\t\t\tPassed\n");
	}
	fflush(stdout);
	fclose(f);
	return 0;
}

static int
aft_flash_bin_file(wan_aft_cpld_t *cpld, int stype, char *filename, int cmd)
{
	char		*data = NULL;
	unsigned long	offset = 0, findex = 0;
	long		fsize = 0;
	char	val;

	fsize = read_bin_data_file(filename, &data);
	if (fsize < 0){
		return -EINVAL;
	}
	if (fsize > 0x40000){
		printf("Flash data file is too big!\n");
		free(data);
		return -EINVAL;
	}

	if (stype == USER_SECTOR_FLASH){
		offset = USER_SECTOR_START_ADDR;
	}
	if (cmd == CMD_FLASH_PRG){
		erase_sector_flash(cpld, stype, 0);
	}
	while(findex < fsize){
		if (cmd == CMD_FLASH_PRG){
			prg_flash_byte(cpld, stype, offset+findex, data[findex]);
		}else{
			val = read_flash(
					cpld, stype,
					MEMORY_TYPE_FLASH,
					offset+findex);
			if (val != data[findex]){
				printf("\r\tVerification\t\t\tFailed(%lx)\n",
						findex);
				free(data);
				return -EINVAL;
			}
		}
		findex++;
		if ((findex & 0x1FFF) == 0x1000){
			if (cmd == CMD_FLASH_PRG){
				progress_bar("\tUpdating flash\t\t\t");
			}else{
				progress_bar("\tVerification\t\t\t");
			}
		}
	}
	if (cmd == CMD_FLASH_PRG){
		printf("\r\tUpdating flash\t\t\tPassed\n");
	}else{
		printf("\r\tVerification\t\t\tPassed\n");
	}
	fflush(stdout);
	free(data);
	return 0;
}

static int prg_flash_data(wan_aft_cpld_t *cpld, int stype, char *filename, int type)
{
	switch(type){
	case HEX_FILE:
		return aft_flash_hex_file(cpld, stype, filename, CMD_FLASH_PRG);
	case BIN_FILE:
		return aft_flash_bin_file(cpld, stype, filename, CMD_FLASH_PRG);
	}
	return -EINVAL;
}

static int verify_flash_data(wan_aft_cpld_t *cpld, int stype, char *filename, int type)
{
	switch(type){
	case HEX_FILE:
		return aft_flash_hex_file(cpld, stype, filename, CMD_FLASH_VERIFY);
	case BIN_FILE:
		return aft_flash_bin_file(cpld, stype, filename, CMD_FLASH_VERIFY);
	}
	return -EINVAL;
}

int update_flash(wan_aft_cpld_t *cpld, int stype, int mtype, char* filename)
{
#if 0
	char		*data = NULL;
	unsigned long	offset = 0, findex = 0;
	long		fsize = 0;
#endif
	char		val = 0x00;
	int		type, err;

	reset_flash(cpld);
	// Checking write enable to the Default Boot Flash Sector 
        if(stype == DEF_SECTOR_FLASH){
		val = read_cpld(cpld, 0x09);
		if (val & WRITE_DEF_SECTOR_DSBL){
			printf("update_flash: Default sector protected!\n");
			return -EINVAL;
		}
	}    

#if 0
	fsize = read_bin_data_file(filename, &data);
	if (fsize < 0){
		return -EINVAL;
	}
	if (fsize > 0x40000){
		printf("update_flash: Data file is too big!\n");
		free(data);
		return -EINVAL;
	}

	if (stype == USER_SECTOR_FLASH){
		offset = USER_SECTOR_START_ADDR;
	}
	erase_sector_flash(cpld, stype, 0);
	while(findex < fsize){
		prg_flash_byte(cpld, stype, offset+findex, data[findex]);
		findex++;
		if ((findex & 0x1FFF) == 0x1000){
			progress_bar("Updating flash\t\t\t");
		}
	}
	printf("\rUpdating flash\t\t\tPassed\n");
	fflush(stdout);

	reset_flash(cpld);
	findex = 0;
	while(findex < fsize){
		val = read_flash(cpld, stype, MEMORY_TYPE_FLASH, offset+findex);
		if (val != data[findex]){
			printf("update_flash: Failed to compare data (%lx)!\n",
					findex);
			free(data);
			return -EINVAL;
		}
		if ((findex & 0x1FFF) == 0x1000){
			progress_bar("Updating flash (verification)\t");
		}
		findex++;
	}
	printf("\rUpdating flash (verification)\tPassed\n");
	fflush(stdout);
#endif
	type = get_file_type(filename);
	if (!(err = prg_flash_data(cpld, stype, filename, type))){
		reset_flash(cpld);
		err = verify_flash_data(cpld, stype, filename, type);
	}
	return err;
}

#if 0
static int verify_flash(wan_aft_cpld_t *cpld, int stype, int mtype, char* filename)
{
#if 0
	char		*data = NULL;
	unsigned long	offset = 0, findex = 0;
	char		val = 0x00;
	long		fsize = 0;
#endif
	int		type, err;


	type = get_file_type(filename);
#if 0
	fsize = read_bin_data_file(filename, &data);
	if (fsize < 0){
		return -EINVAL;
	}
	if (type == BIN_FILE && fsize > 0x40000){
		printf("verify_flash: Data file is too big!\n");
		free(data);
		return -EINVAL;
	}

	if (stype == USER_SECTOR_FLASH){
		offset = USER_SECTOR_START_ADDR;
	}
	findex = 0;
	while(findex < fsize){
		val = read_flash(cpld, stype, MEMORY_TYPE_FLASH, offset+findex);
		if (val != data[findex]){
			printf("verify_flash: Failed to compare data (%lx)!\n",
					findex);
			free(data);
			return -EINVAL;
		}
		if ((findex & 0x1FFF) == 0x1000){
			progress_bar("Verify flash");
		}
		findex++;
	}
	printf("\rVerify flash                    Passed\n");
	fflush(stdout);
	free(data);
#endif
	err = verify_flash_data(cpld, stype, filename, type);

	return err;
}
#endif

int check_flash_id(wan_aft_cpld_t *cpld, int mtype, int stype, int *flash_id)
{
	unsigned char	man_code, device_code;

	write_flash(cpld, stype, mtype, 0x5555, 0xAA);
	write_flash(cpld, stype, mtype, 0x2AAA, 0x55);
	write_flash(cpld, stype, mtype, 0x5555, 0x90);
	man_code = read_flash(cpld, stype, mtype, 0x00);
	*flash_id = man_code << 8;
	device_code = read_flash(cpld, stype, mtype, 0x01);
	*flash_id |= device_code;
	reset_flash(cpld);
	return 0;
}

int reload_flash(wan_aft_cpld_t *cpld, int sector_type)
{
	if (sector_type == USER_SECTOR_FLASH){
		/* Reload new code in to Xilinx from
                ** Flash User sector */  
		write_cpld(cpld, 0x07,0xC0); 
	}else{
		/* Reload new code in to Xilinx from
		** Flash Default sector */   
		write_cpld(cpld, 0x07,0x80);   
	}
	return 0;
}

int release_board(wan_aft_cpld_t *cpld)
{
	unsigned int	data;

	/* Release board internal reset (AFT-T1/E1/T3/E3 */
	if (exec_read_cmd(cpld->private, 0x40, 4, &data)){
		printf("Failed access (read) to the board!\n");
		return -EINVAL;
	}

	switch(cpld->adptr_type){
	case A104_ADPTR_4TE1:
		data &= ~(0x06);
		break;
	case A101_ADPTR_1TE1:
	case A101_ADPTR_2TE1:
		data &= ~(0x20);
		break;
	case A300_ADPTR_U_1TE3:
		data &= ~(0x20);
		break;
	default:
		return -EINVAL;
	}

	if (exec_write_cmd(cpld->private, 0x40,4,data)){
		printf("Failed access (write) to the board!\n");
		return -EINVAL;
	}
	return 0;
}

