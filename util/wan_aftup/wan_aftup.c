/*
**	History
**-----------------------------------------------------------------
**	  Date		Name		Description
**-----------------------------------------------------------------
**
**
**	24 Dec 2004	Alex Feldman	Initial version for Linux OS.
**					(version 1.2)
**	14 Jan 2005	Alex Feldman	Support OpenBSD OS.
**					(version 1.3)
**	24 Jan 2005	Alex Feldman	Support FreeBSD/NetBSD OS.
**					(version 1.4)
**	15 Aug 2005	Alex Feldman	New version will automatically
**					update all Sangoma card installed
**					on this computer with the latest
**					firmware from current directory.
**					For the auto update, use '-auto'
**					command line argument.
**					(version 1.5)
**	22 Dec 2006	Alex Feldman	* Add A400 support
**					* PLX update 
**					(version 1.6)
**	22 Jan 2007	Alex Feldman	* Add new option to command line
**					* Force Flash update with specific filename
**					(version 1.7)
**/

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#if defined(__LINUX__)
# include <linux/if.h>
# include <linux/types.h>
# include <linux/if_packet.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_common.h>
# include <linux/sdlapci.h>
# include <linux/sdlasfm.h>
# include <linux/if_wanpipe.h>
#else
# include <net/if.h>
# include <wanpipe_defines.h>
# include <wanpipe_common.h>
# include <sdlapci.h>
# include <sdlasfm.h>
# include <wanpipe.h>
#endif

#include "wan_aft_prg.h"
#include "wan_aftup.h"

/***********************************************************************
**			D E F I N E S / M A C R O S
***********************************************************************/
#define WAN_AFTUP_VERSION	"1.7"

#define WAN_AFTUP_NONE		0x00
#define WAN_AFTUP_AUTO		0x01
#define WAN_AFTUP_FORCE_FIRM	0x02
#define WAN_AFTUP_PCIEXPRESS	0x04

/***********************************************************************
**			G L O B A L  V A R I A B L E S
***********************************************************************/
static int		sock;
static wan_cmd_api_t	api_cmd;
static char		ifname_def[20];
static struct ifreq	req;
int			options = 0x00;
static char		aft_firmware_force[MAXPATHLEN];

extern aftup_flash_iface_t aftup_flash_iface;
extern aftup_flash_iface_t aftup_shark_flash_iface;

extern aftup_flash_t	aft_flash;
extern aftup_flash_t	aft4_flash;
extern aftup_flash_t	aft_shark_flash;
extern aftup_flash_t	aft_shark_flash_ds;

	
WAN_LIST_HEAD(wan_aftup_head_t, wan_aftup_) wan_aftup_head = 
			WAN_LIST_HEAD_INITIALIZER(wan_aftup_head);

aft_core_info_t aft_core_table[] = {
	{ A101_1TE1_SUBSYS_VENDOR, AFT_CHIP_OLD_X300, AFT_HDLC_CORE_ID, 0x01, 0x4F,
	  "A101_V", "A101_V*.BIN", AFT_CORE_SIZE },
	{ A101_1TE1_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_HDLC_CORE_ID, 0x01, 0x4F,
	  "A101N_V", "A101N_V*.BIN", AFT4_CORE_SIZE },
	{ A101_2TE1_SUBSYS_VENDOR, AFT_CHIP_OLD_X300, AFT_HDLC_CORE_ID, 0x01, 0x4F,
	  "A101_V", "A101_V*.BIN", AFT_CORE_SIZE },
	{ A101_2TE1_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_HDLC_CORE_ID, 0x01, 0x4F,
	  "A101N_V", "A101N_V*.BIN", AFT4_CORE_SIZE },
	{ A104_4TE1_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_HDLC_CORE_ID, 0x20, 0x53,
	  "A104_V", "A104_V*.BIN", AFT4_CORE_SIZE },
	{ AFT_4TE1_SHARK_SUBSYS_VENDOR, AFT_CHIP_X1000, AFT_PMC_FE_CORE_ID, 0x20, 0x5B,
	  "A104d_0100_V", "A104d_0100_V*.BIN", AFT_CORE_X1000_SIZE },
	{ AFT_4TE1_SHARK_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_PMC_FE_CORE_ID, 0x20, 0x5B,
	  "A104d_0040_V", "A104d_0040_V*.BIN", AFT_CORE_X400_SIZE },
	{ AFT_1TE1_SHARK_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_DS_FE_CORE_ID, 0x20, 0x5B,
	  "A101dm_0040_V", "A101dm_0040_V*.BIN", AFT_CORE_X400_SIZE },
	{ AFT_2TE1_SHARK_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_DS_FE_CORE_ID, 0x20, 0x5B,
	  "A102dm_0040_V", "A102dm_0040_V*.BIN", AFT_CORE_X400_SIZE },
	{ AFT_4TE1_SHARK_SUBSYS_VENDOR, AFT_CHIP_X1000, AFT_DS_FE_CORE_ID, 0x20, 0x5B,
	  "A104dm_0100_V", "A104dm_0100_V*.BIN", AFT_CORE_X1000_SIZE },
	{ AFT_8TE1_SHARK_SUBSYS_VENDOR, AFT_CHIP_X1000, AFT_DS_FE_CORE_ID, 0x20, 0x5B,
	  "A108dm_0100_V", "A108dm_0100_V*.BIN", AFT_CORE_X1000_SIZE },
	{ A200_REMORA_SHARK_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_ANALOG_FE_CORE_ID, 0x01, 0x4F,	
	  "A200_0040_V", "A200_0040_V*.BIN", AFT_CORE_X400_SIZE },
	{ A400_REMORA_SHARK_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_ANALOG_FE_CORE_ID, 0x01, 0x4F,	
	  "A400_0040_V", "A400_0040_V*.BIN", AFT_CORE_X400_SIZE },
	{ A200_REMORA_SHARK_SUBSYS_VENDOR, AFT_CHIP_X200, AFT_ANALOG_FE_CORE_ID, 0x20, 0x5B,	
	  "A200_0020_V", "A200_0020_V*.BIN", AFT_CORE_X200_SIZE },
	{ A200_REMORA_SHARK_SUBSYS_VENDOR, AFT_CHIP_X1000, AFT_ANALOG_FE_CORE_ID, 0x20, 0x5B,	
	  "A200_0100_V", "A200_0100_V*.BIN", AFT_CORE_X1000_SIZE },
	{ A300_UTE3_SHARK_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_HDLC_CORE_ID, 0x00, 0x00,
	  "A301_0040_V", "A301_0040_V*.BIN", AFT_CORE_X400_SIZE },
	{ A300_UTE3_SUBSYS_VENDOR, AFT_CHIP_X400, AFT_HDLC_CORE_ID, 0x00, 0x00,
	  "A301_V", "A301_V*.BIN", AFT_CORE_SIZE },
#if 0
	{ AFT_TE1_ATM_CORE_ID,	
	  NULL, NULL, 0x00,
	  "1/2 T1/E1 line(s) ATM core" },
	{ AFT_TE1_SS7_CORE_ID,
	  NULL, NULL, 0x00,
	  "1/2 T1/E1 line(s) SS7 core" },
#endif
	{ 0x00, 0x00, 0x00, 0x00, 0x00, NULL, NULL, 0x00 }
};

struct wan_aftup_plxctrl_data_ {
	unsigned char	off;
	unsigned char	value;
} plxctrl_data [] =
	{
		{ 0x00, 0x5A },
		{ 0x01, 0x01 },
		{ 0x02, 0x24 },
		{ 0x03, 0x00 },
		{ 0x04, 0x48 },
		{ 0x05, 0x00 },
		{ 0x06, 0x11 },
		{ 0x07, 0x2C },
		{ 0x08, 0x0E },
		{ 0x09, 0x00 },
		{ 0x0A, 0x0C },
		{ 0x0B, 0x10 },
		{ 0x0C, 0x10 },
		{ 0x0D, 0x80 },
		{ 0x0E, 0xFE },
		{ 0x0F, 0x0B },
		{ 0x10, 0x10 },
		{ 0x11, 0x00 },
		{ 0x12, 0x08 },
		{ 0x13, 0x00 },
		{ 0x14, 0x00 },
		{ 0x15, 0x00 },
		{ 0x16, 0x0C },
		{ 0x17, 0x00 },
		{ 0x18, 0x00 },
		{ 0x19, 0xFE },
		{ 0x1A, 0x01 },
		{ 0x1B, 0x00 },
		{ 0x1C, 0x48 },
		{ 0x1D, 0x10 },
		{ 0x1E, 0x20 },
		{ 0x1F, 0x00 },
		{ 0x20, 0x05 },
		{ 0x21, 0x00 },
		{ 0x22, 0x0C },
		{ 0x23, 0x00 },
		{ 0x24, 0x00 },
		{ 0x25, 0xFE },
		{ 0x26, 0x01 },
		{ 0x27, 0x00 },
		{ 0xFC, 0x19 },	/* Sangoma vendor ID		*/
		{ 0xFD, 0x23 },	/* Sangoma vendor ID		*/
		{ 0xFE, 0x00 },	/* Sangoma PLX config verion	*/
		{ 0xFF, 0x02 }	/* Sangoma PLX config verion	*/
	};

/******************************************************************************
			  FUNCTION PROTOTYPES
******************************************************************************/
int progress_bar(char*,int,int);
void main_menu(void);
void read_chip_control_menu(void);
void registers_and_internal_ram_test_menu(void);
void setup_chip_configuration_menu(void);
int exec_command(int cmd);
int exec_reload_pci_cmd(void);
int exec_read_cmd(void *, unsigned int off, unsigned int len, unsigned int *data);
int exec_write_cmd(void *, unsigned int off, unsigned int len, unsigned int data);
int exec_bridge_read_cmd(void *, unsigned int off, unsigned int len, unsigned int *data);
int exec_bridge_write_cmd(void *, unsigned int off, unsigned int len, unsigned int data);

void hit_any_key(void);

static int aft_a200_a400_warning(wan_aftup_t *aft);

extern int pmc_initialization (int sock);
extern int update_flash(wan_aft_cpld_t*, int, int, char*);
extern int board_reset(wan_aft_cpld_t *, int clear);

extern unsigned char wan_plxctrl_read_ebyte(void*, unsigned char,int);
extern void wan_plxctrl_write_ebyte(void*, unsigned char, unsigned char);

/******************************************************************************
*			  FUNCTION DEFINITION	
******************************************************************************/

int progress_bar(char *msg, int ci, int mi)
{
	static int index = 0;

	if (mi == 0){
		if (index++ == 0){
			printf("\r%s |", msg);
		}else if (index++ == 1){
			printf("\r%s /", msg);
		}else{
			printf("\r%s", msg);
			index = 0;
		}	
	}else{
		int	pos = (ci*100)/mi;
		printf("\r%s%d%%", msg, pos);
	}
	fflush(stdout);
	return 0;
}
static int MakeConnection(char *ifname) 
{
#if defined(__LINUX__)
	char			error_msg[100];
	struct ifreq		ifr;
	struct sockaddr_ll	sa;
	
	memset(&sa,0,sizeof(struct sockaddr_ll));
	memset(&ifr,0,sizeof(struct ifreq));
	errno = 0;
   	sock = socket(AF_PACKET, SOCK_DGRAM, 0);
   	if( sock < 0 ) {
      		perror("Socket");
      		return -EIO;
   	} /* if */
  
	strncpy(ifr.ifr_name, ifname, strlen(ifname));
	if (ioctl(sock,SIOCGIFINDEX,&ifr)){
		snprintf(error_msg, 100, "Get index: %s", ifname);
		perror(error_msg);
		close(sock);
		return -EIO;
	}

	sa.sll_protocol = htons(0x17);
	sa.sll_family=AF_PACKET;
	sa.sll_ifindex = ifr.ifr_ifindex;

        if(bind(sock, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll)) < 0){
		snprintf(error_msg, 100, "Bind: %s", ifname);
                perror(error_msg);
		close(sock);
                return -EINVAL;
        }
#elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)

 	sock = socket(AF_INET, SOCK_DGRAM, 0);
   	if( sock < 0 ) {
      		perror("Socket");
      		return -EIO;
   	} /* if */
#endif
	return 0;

}

static int CloseConnection(char *ifname)
{
	close(sock);
	return 0;
}

static int wan_aftup_ioctl(wan_cmd_api_t *api_cmd, int cmd, char *ifname)
{
	int ret;

	api_cmd->cmd = cmd;
	req.ifr_data = (void*)api_cmd;
	strncpy(req.ifr_name, ifname, strlen(ifname));
	if ((ret = ioctl(sock, SIOC_WAN_DEVEL_IOCTL, &req)) < 0){
		return -EINVAL;
	}
	return 0;
}

static int wan_aftup_gettype(wan_aftup_t *aft, char *type)
{
	
	if (strncmp(type,"AFT-A101",8) == 0){
		//strcpy(aft->prefix_fw, "A101");
		aft->cpld.adptr_type = A101_ADPTR_1TE1;
		aft->cpld.iface	= &aftup_flash_iface;
	}else if (strncmp(type,"AFT-A102",8) == 0){
		//strcpy(aft->prefix_fw, "A101");
		aft->cpld.adptr_type = A101_ADPTR_2TE1;
		aft->cpld.iface	= &aftup_flash_iface;
	}else if (strncmp(type,"AFT-A104",8) == 0){
		//strcpy(aft->prefix_fw, "A104");
		aft->cpld.adptr_type = A104_ADPTR_4TE1;
		aft->cpld.iface	= &aftup_flash_iface;
	}else if (strncmp(type,"AFT-A108",8) == 0){
		//strcpy(aft->prefix_fw, "A104");
		aft->cpld.adptr_type = A108_ADPTR_8TE1;
		aft->cpld.iface	= &aftup_flash_iface;
	}else if (strncmp(type,"AFT-A300",8) == 0){
		//strcpy(aft->prefix_fw, "A301");
		aft->cpld.adptr_type = A300_ADPTR_U_1TE3;
		aft->cpld.iface	= &aftup_flash_iface;
	}else if (strncmp(type,"AFT-A200",8) == 0){
		//strcpy(aft->prefix_fw, "AFT_RM");
		aft->cpld.adptr_type = A200_ADPTR_ANALOG;
		aft->cpld.iface	= &aftup_shark_flash_iface;
	}else if (strncmp(type,"AFT-A400",8) == 0){
		//strcpy(aft->prefix_fw, "AFT_RM");
		aft->cpld.adptr_type = A400_ADPTR_ANALOG;
		aft->cpld.iface	= &aftup_shark_flash_iface;
	}else if (strncmp(type,"AFT-A301",8) == 0){
		//strcpy(aft->prefix_fw, "AFT_RM");
		aft->cpld.adptr_type = A300_ADPTR_U_1TE3;
		aft->cpld.iface	= &aftup_shark_flash_iface;
	}else{
		return -EINVAL;
	}
	return 0;
}

static int wan_aftup_getfile(wan_aftup_t *aft)
{
	FILE 		*f;
	char		ver[10], sel_ver[10];
	DIR		*dir;
	struct dirent	*ent;
	int		versions_no = 0, ver_no = -1;

	if (options & WAN_AFTUP_FORCE_FIRM){
		memcpy(aft->cpld.core_info->firmware, 
				aft_firmware_force,
				strlen(aft_firmware_force));
		goto verify_firmware_file;
	}
	if (!(dir = opendir("."))){
		perror("opendir");
		return -EINVAL;
	}
	if (!(options & WAN_AFTUP_AUTO)){
		printf("List of available versions: \n");
	}
	while((ent = readdir(dir))){
		if (strncmp(ent->d_name, aft->cpld.core_info->fw_prefix, strlen(aft->cpld.core_info->fw_prefix)) ==0){
			int	i = strlen(aft->cpld.core_info->fw_prefix);
			int	new_ver;
			
			while(!isdigit(ent->d_name[i])) i++;
			new_ver = (ent->d_name[i]-'0') * 16 + (ent->d_name[i+1]-'0');
			if (ver_no == -1 || new_ver > ver_no){
				int j=0;
				ver_no = new_ver;
				strncpy(aft->cpld.core_info->firmware,
					ent->d_name,
					strlen(ent->d_name));
				while(isdigit(ent->d_name[i])){
					ver[j++] = ent->d_name[i++];
				}
				//ver[0] = ent->d_name[i];
				//ver[1] = ent->d_name[i+1];
			}
			if (!(options & WAN_AFTUP_AUTO)){
				printf("\t Version no. %X (filename=%s)\n", 
						new_ver, ent->d_name);
			}
			versions_no++;
		}
	}
	if (versions_no == 0){
		printf("\nYour current directory doesn't include update file (%s)!\n",
						aft->cpld.core_info->fw_template);
		printf("\nPlease download the update file from ftp.sangoma.com.\n");
		return -EINVAL;
	}

	if (!(options & WAN_AFTUP_AUTO)){

wan_aftup_getfile_again:
		printf("Please specify version number [def=%X; q=exit] > ",
					ver_no);
		if (scanf("%s", sel_ver)){
			if (strcmp(sel_ver, "q") == 0){
				return -EINVAL;
			}else if (strcmp(sel_ver, "def") == 0){
				snprintf(sel_ver, 10, "%X", ver_no);
			}
			if (strlen(sel_ver) == 2){
				strncpy(ver, sel_ver, strlen(sel_ver));
				ver[2] = '\0';
			}else if (strlen(sel_ver) == 1){
				ver[0] = '0';
				ver[1] = sel_ver[0];
				ver[2] = '\0';
			}else{
				goto wan_aftup_getfile_again;
			}
		}
		memset(aft->cpld.core_info->firmware, 0, MAXPATHLEN);
		strncpy(aft->cpld.core_info->firmware,
			aft->cpld.core_info->fw_prefix,
			strlen(aft->cpld.core_info->fw_prefix));
		/*strcat(aft->cpld.core_info->firmware, "_V");*/
		strncat(aft->cpld.core_info->firmware, ver, strlen(ver));
		strncat(aft->cpld.core_info->firmware, ".BIN", 4);
	} else {
		if (ver_no <= aft->flash_rev) {
			printf ("%s: Device already at latest rev=%X\n",
					aft->if_name,ver_no);
			return 1;
		}
	}
	
verify_firmware_file:
	f = fopen(aft->cpld.core_info->firmware, "rb");
	if (f == NULL){
		printf("Failed to open file %s for %s\n",
				aft->cpld.core_info->firmware, aft->if_name);
		return -EINVAL;
	}
	fclose(f);
	printf("\n");
	return 0;
}

static int wan_aftup_program_card(wan_aftup_t *aft)
{
	int	flash_id = 0, tmp = 0, err = -EINVAL;
	
	if (wan_aftup_getfile(aft)){
		return -EINVAL;
	}

	/* Release board internal reset (AFT-T1/E1/T3/E3 */
	if (board_reset(&aft->cpld, 0)){
		printf("ERROR: %s: Failed to set board in reset!\n",
					aft->if_name);
		return -EINVAL;
	}
	if (board_reset(&aft->cpld, 1)){
		printf("ERROR: %s: Failed to clear board reset!\n",
					aft->if_name);
		return -EINVAL;
	}

	/* Check Flash ID */
	if (aft->cpld.iface){
		err = aft->cpld.iface->flash_id(&aft->cpld,
					MEMORY_TYPE_FLASH,
					USER_SECTOR_FLASH,
					&flash_id);
	}
	if (err){
		return -EINVAL;
	}
	printf("%s: Current Sangoma Flash: Revision=%X ID=0x%04X\n",
					aft->if_name,
					aft->flash_rev,
					flash_id);

	err = update_flash(
			&aft->cpld,
			USER_SECTOR_FLASH,
			MEMORY_TYPE_FLASH,
			aft->cpld.core_info->firmware);
	if (err){
		printf("ERROR: %s: Failed to re-program flash!\n",
					aft->if_name);
		return -EINVAL;
	}
	
	printf("%s: Sangoma Flash update\t\t\tDONE\n", aft->if_name);
#if 0
	if (!(options & WAN_AFTUP_AUTO)){
		char	ch;
		printf(
		"\n%s: You have to reload flash in order to new flash code will take place\n",
						aft->if_name);
		printf("Do you want to reload now (Y/N)?");
		ch = getchar();
		ch = getchar();
		if (ch != 'y' && ch != 'Y'){
			printf("Please, reboot your computer before using Sangoma AFT Series card\n");
			printf("New flash code will reload while rebooting computer!\n");
			return 0;
		}
		printf("\n");
	}
#else
	printf("\n");
#endif
	if (aft->cpld.iface){
		printf("%s: Reloading Sangoma flash\t\tDONE\n", aft->if_name);
		aft->cpld.iface->reload(&aft->cpld, USER_SECTOR_FLASH);
	}

	usleep(1000000);

	/* Read flash revision */
	if (wan_aftup_ioctl(&api_cmd, SIOC_WAN_SET_PCI_BIOS, aft->if_name)){
		printf("%s: Failed to restore PCI registers!\n",
					aft->if_name);
		return -EINVAL;
	}

	/* Release board internal reset (AFT-T1/E1/T3/E3 */
	if (board_reset(&aft->cpld, 0)){
		printf("ERROR: %s: Failed to set board in reset!\n",
					aft->if_name);
		return -EINVAL;
	}
	if (board_reset(&aft->cpld, 1)){
		printf("ERROR: %s: Failed to clear board reset!\n",
					aft->if_name);
		return -EINVAL;
	}

	/* Check Flash ID */
	if (aft->cpld.iface){
		aft->cpld.iface->flash_id(	&aft->cpld,
						MEMORY_TYPE_FLASH,
						USER_SECTOR_FLASH,
						&tmp);
	}
	if (tmp != flash_id){
		printf("%s: Failed to read Flash ID (new flash id %04X)\n",
					aft->if_name, tmp);
		return -EINVAL;
	}
	
#if 0
	/* Read new Flash revision */
	if (wan_aftup_ioctl(&api_cmd, SIOC_WAN_COREREV, aft->if_name)){
		printf("%s: Failed to read new Flash revision!\n",
						aft->if_name);
		return -EINVAL;
	}
	printf("%s: New Sangoma Flash revision %X\n",
				aft->if_name, api_cmd.data[0]);
	if (api_cmd.data[0] < aft->flash_rev){
		printf("%s: New flash revision is %d (previous flash revision %d)\n",
				aft->if_name, api_cmd.data[0], aft->flash_rev);
		printf("You re-program flash with older version!\n");
		printf("Please, contact Sangoma Technologies (905.474.1990)!\n");
		return -EINVAL;
	}
#endif

	printf("%s: Sangoma Flash updated successfully\n\n", 
					aft->if_name);
	return 0;	
}

static int wan_aftup_program(struct wan_aftup_head_t *head)
{
	wan_aftup_t	*aft = NULL;
	int		tmp = 0x00;
	int		err, i;

	WAN_LIST_FOREACH(aft, head, next){
		if (MakeConnection(aft->if_name)){
			printf("%s: Failed to create socket to the driver!\n",
						 aft->if_name);
			continue;
		}

		/* Read core revision */
		if (wan_aftup_ioctl(&api_cmd, SIOC_WAN_COREREV,aft->if_name)){
			printf("%s: Failed to read flash revision!\n",
						aft->if_name);
			err = -EINVAL;
			goto program_done;
		}
		aft->flash_rev = api_cmd.data[0];

		/* Read subsystem vendor ID */
		exec_read_cmd(aft, 
			PCI_DEVICE_ID_WORD, 2, (unsigned int*)&aft->chip_id);
		exec_read_cmd(aft, 0x08, 1, (unsigned int*)&aft->revision_id);
#if defined(__OpenBSD__)
		exec_read_cmd(aft, 
			PCI_SUBSYS_VENDOR_WORD, 4, &aft->board_id);
		aft->board_id &= 0xFFFF;
#else
		exec_read_cmd(aft, 
			PCI_SUBSYS_VENDOR_WORD, 2, (unsigned int*)&aft->board_id);
#endif
		exec_read_cmd(aft, PCI_SUBSYS_ID_WORD, 2, (unsigned int*)&tmp);
		aft->core_rev = AFT_CORE_REV(tmp);
		aft->core_id = AFT_CORE_ID(tmp);
		
#if 1
                err=aft_a200_a400_warning(aft);
		if (err) {
			goto program_done;
		}     
		
		for(i = 0; aft_core_table[i].board_id; i++){

#if 0		
			printf("DEBUG: %s: %04X:%04X:%04X (%04X:%04X:%04X)\n",
					aft->if_name,
					aft_core_table[i].board_id,
					aft_core_table[i].chip_id,
					aft_core_table[i].core_id,
					aft->board_id, aft->chip_id, aft->core_id);
#endif					
			if (aft_core_table[i].board_id != aft->board_id){
				continue;
			}
				
			if ((aft_core_table[i].chip_id == aft->chip_id) &&
			    (aft_core_table[i].core_id == aft->core_id)){
				aft->cpld.core_info = &aft_core_table[i];
				break;
			}
			/* For the old cards ... */
			if (aft->board_id == A104_4TE1_SUBSYS_VENDOR){
				aft->cpld.core_info = &aft_core_table[i];
				break;
			}

			/* Special case for the new A101/2 */
			if ((aft->cpld.adptr_type == A101_ADPTR_1TE1) ||
			    (aft->cpld.adptr_type == A101_ADPTR_2TE1)){
				if (aft->chip_id == AFT_CHIP_OLD_X400){
					i++;
				}else if (aft->revision_id){
					/* Special case for the A101/2 new cards */
					i++;
				}
				aft->cpld.core_info = &aft_core_table[i];
				break;
			}
		}

		if (!aft_core_table[i].board_id){
			printf("%s: Unsupported board type for firmware update (%04X:%04X)!\n",
						aft->if_name,
						aft->board_id,
						aft->chip_id);
			err = -EINVAL;
			goto program_done;
		}
		
		switch(aft->board_id){
		case A101_1TE1_SUBSYS_VENDOR:
		case A101_2TE1_SUBSYS_VENDOR:
			aft->cpld.iface	= &aftup_flash_iface;
			break;
		case A104_4TE1_SUBSYS_VENDOR:
			aft->cpld.iface	= &aftup_flash_iface;
			break;
		case A300_UTE3_SUBSYS_VENDOR:
		case A305_CT3_SUBSYS_VENDOR:
			aft->cpld.iface	= &aftup_flash_iface;
			break;
		case AFT_1TE1_SHARK_SUBSYS_VENDOR:
		case AFT_2TE1_SHARK_SUBSYS_VENDOR:
			aft->cpld.iface	= &aftup_shark_flash_iface;
			break;
		case AFT_4TE1_SHARK_SUBSYS_VENDOR:
			aft->cpld.iface	= &aftup_shark_flash_iface;
			break;
		case AFT_8TE1_SHARK_SUBSYS_VENDOR:
			aft->cpld.iface	= &aftup_shark_flash_iface;
			break;
		case A200_REMORA_SHARK_SUBSYS_VENDOR:
			aft->cpld.iface	= &aftup_shark_flash_iface;
			break;
		case A400_REMORA_SHARK_SUBSYS_VENDOR:
			aft->cpld.iface	= &aftup_shark_flash_iface;
			break;
		case A300_UTE3_SHARK_SUBSYS_VENDOR:
			aft->cpld.iface	= &aftup_shark_flash_iface;
			break;
		default:
			printf("\n%s: These board are not supported (subvendor_id=%04X)!\n",
						aft->if_name,
						aft->board_id);
			err = -EINVAL;
			goto program_done;
			break;
		}

#else

		switch(aft->cpld.board_id){
		case A101_1TE1_SUBSYS_VENDOR:
		case A101_2TE1_SUBSYS_VENDOR:
			aft->cpld.chip_id = AFT_CHIP_X300;
			/* Read revision ID */
			exec_read_cmd(aft, 
					0x08, 1, (unsigned int*)&aft->cpld.adptr_subtype);
			if (aft->cpld.adptr_subtype == 0x01){
				/* A101/A102 new cards */
				strncpy(aft->prefix_fw, "A101N", 5);
				aft->cpld.chip_id = AFT_CHIP_X400;
			}
			aft->cpld.flash	= &aft_shark_flash;
			break;
		case A104_4TE1_SUBSYS_VENDOR:
			aft->cpld.chip_id = AFT_CHIP_X400;
			aft->cpld.flash	= &aft4_flash;
			break;
		case A300_UTE3_SUBSYS_VENDOR:
		case A305_CT3_SUBSYS_VENDOR:
			aft->cpld.chip_id = AFT_CHIP_X300;
			aft->cpld.flash	= &aft_shark_flash;
			break;
		case AFT_4TE1_SHARK_SUBSYS_VENDOR:
			aft->cpld.chip_id = AFT_CHIP_X1000;
			aft->cpld.flash	= &aft_shark_flash;
			break;
		case A200_REMORA_SHARK_SUBSYS_VENDOR:
		case A400_REMORA_SHARK_SUBSYS_VENDOR:
			aft->cpld.chip_id = AFT_CHIP_X1000;
			aft->cpld.flash	= &aft_shark_flash;
			break;
		case AFT_1TE1_SHARK_SUBSYS_VENDOR:
		case AFT_2TE1_SHARK_SUBSYS_VENDOR:
		case AFT_8TE1_SHARK_SUBSYS_VENDOR:
		case A300_UTE3_SHARK_SUBSYS_VENDOR:
		case A305_CTE3_SHARK_SUBSYS_VENDOR:
			printf("\n%s: These board are not supported (subvendor_id=%04X)!\n",
						aft->if_name,
						aft->cpld.board_id);
			goto program_done;
			break;
		}
#endif

		if (wan_aftup_program_card(aft)){
			printf("\n%s: Failed to re-program flash!\n",
						aft->if_name);
			err = -EINVAL;
			goto program_done;
		}

program_done:
		CloseConnection(aft->if_name);
	}

	return 0;
}

static int wan_aftup_verify_pciexpress(struct wan_aftup_head_t *head)
{
	wan_aftup_t	*aft = NULL;
	unsigned char	tmp = 0x00;
	int		i,regs_no = 0, update = 0;

	regs_no = sizeof(plxctrl_data)/sizeof(struct wan_aftup_plxctrl_data_);
	WAN_LIST_FOREACH(aft, head, next){
		switch(aft->board_id){
		case AFT_1TE1_SHARK_SUBSYS_VENDOR:
		case AFT_2TE1_SHARK_SUBSYS_VENDOR:
		case AFT_4TE1_SHARK_SUBSYS_VENDOR:
		case AFT_8TE1_SHARK_SUBSYS_VENDOR:
			break;
		case A200_REMORA_SHARK_SUBSYS_VENDOR:
		case A400_REMORA_SHARK_SUBSYS_VENDOR:
			break;
		case A300_UTE3_SHARK_SUBSYS_VENDOR:
			break;
		default:
			continue;
		}
		switch(aft->core_id){
		case AFT_DS_FE_CORE_ID:
		case AFT_ANALOG_FE_CORE_ID:
			break;
		default:
			continue;
		}
		if (MakeConnection(aft->if_name)){
			printf(
			"%s: Failed to create socket to the driver!\n",
						 aft->if_name);
			continue;
		}

		tmp = wan_plxctrl_read_ebyte(aft, (unsigned char)0x00,1);
		if (tmp != 0x5A){
			continue;
		}
		for(i = 0; i < regs_no; i++){		
			tmp = wan_plxctrl_read_ebyte(
						aft,
						plxctrl_data[i].off, 0);
			if (tmp == plxctrl_data[i].value){
				continue;		
			}
			if (!update){
				printf(
       				"Updating PCI Express Bridge settings");
			}
			wan_plxctrl_write_ebyte(aft, 
						plxctrl_data[i].off, 
						plxctrl_data[i].value);
			tmp = wan_plxctrl_read_ebyte(
						aft,
						plxctrl_data[i].off,0);
			printf(".");
			if (tmp != plxctrl_data[i].value){
				printf("\tFailed (verification:%X:%X:%X)!\n\n",
						plxctrl_data[i].off,tmp,plxctrl_data[i].value);
				printf("\tPlease call Sangoma Technical Support at 905.474.1990!\n\n");
				return -EINVAL;
			}
			update = 1;
		}			
		CloseConnection(aft->if_name);
	}
	return update;
}

static int wan_aftup_parse_hwprobe(wan_cmd_api_t *api_cmd)
{
	wan_aftup_t	*aft = NULL, *aft_prev = NULL;
	char		sel_name[20], *tmp = NULL;
	int		j, cnt = 0;
	
	tmp = strtok((char*)api_cmd->data, "\n");
	while (tmp){
		/* Create new interface structure */
		aft = malloc(sizeof(wan_aftup_t));
		if (aft == NULL){
			printf("ERROR: Failed allocate memory!\n");
			return 0;
		}
		aft->cpld.private = aft;
		strncpy(aft->hwinfo, tmp, strlen(tmp));

		if (aft_prev == NULL){
			WAN_LIST_INSERT_HEAD(
					&wan_aftup_head,
					aft,
					next);
		}else{
			WAN_LIST_INSERT_AFTER(
					aft_prev,
					aft,
					next);
		}
		aft_prev = aft;
		tmp = strtok(NULL, "\n");
	}

#if 0
	WAN_LIST_FOREACH(aft, &wan_aftup_head, next){
		printf("<%s>\n", aft->hwinfo);
	}
#endif
	if (!(options & WAN_AFTUP_AUTO)){
		printf("Sangoma AFT card list:\n");
	}
	aft = WAN_LIST_FIRST(&wan_aftup_head);
	while(aft){
	
		/* Use api_cmd structure to parse hwprobe info */
		strncpy((char*)api_cmd->data, aft->hwinfo, strlen(aft->hwinfo));
		tmp = strtok((char*)api_cmd->data, ":");
		if (tmp == NULL){
			printf("ERROR:%d: Internal error (hwinfo:%s)\n",
						__LINE__, aft->hwinfo);
			aft_prev = aft;
			aft = WAN_LIST_NEXT(aft_prev, next);
			if (aft_prev == WAN_LIST_FIRST(&wan_aftup_head)){
				WAN_LIST_FIRST(&wan_aftup_head) = aft;
			}else{
				WAN_LIST_REMOVE(aft_prev, next);
			}
			free(aft_prev);
			continue;
		}
		/* got interface name */
		strncpy(aft->if_name, tmp, strlen(tmp));
		tmp = strtok(NULL, ":");
		while(*tmp == ' ') tmp++;
		j = 0;
		while(tmp[j] != ' ' && tmp[j] != '\0') j++;
		tmp[j]='\0';
		if (wan_aftup_gettype(aft, tmp)){
			aft_prev = aft;
			aft = WAN_LIST_NEXT(aft_prev, next);
			if (aft_prev == WAN_LIST_FIRST(&wan_aftup_head)){
				WAN_LIST_FIRST(&wan_aftup_head) = aft;
			}else{
				WAN_LIST_REMOVE(aft_prev, next);
			}
			free(aft_prev);
			continue;
		}
		if (!(options & WAN_AFTUP_AUTO)){
			printf(" %s\n", aft->hwinfo);		
		}
		aft = WAN_LIST_NEXT(aft, next);
	}

#if 0
	WAN_LIST_FOREACH(aft, &wan_aftup_head, next){
		printf("<%s:%d:%s>\n",
				aft->if_name,
				aft->cpld.adptr_type,
				aft->prefix_fw);
	}
#endif
	if (!(options & WAN_AFTUP_AUTO) && !WAN_LIST_EMPTY(&wan_aftup_head)){
		aft = WAN_LIST_FIRST(&wan_aftup_head);
		printf("\n");
		printf("Please select card interface [def=%s; q=exit] > ",
					aft->if_name);
		scanf("%s", sel_name);
		printf("\n");

		while(aft){
			if (strcmp(aft->if_name, sel_name) == 0){
				/* This interface is selected */
				aft = WAN_LIST_NEXT(aft, next);
			}else{
				aft_prev = aft;
				aft = WAN_LIST_NEXT(aft_prev, next);
				if (aft_prev == WAN_LIST_FIRST(&wan_aftup_head)){
					WAN_LIST_FIRST(&wan_aftup_head) = aft;
				}else{
					WAN_LIST_REMOVE(aft_prev, next);
				}
				free(aft_prev);
				continue;
			}
		}
	}

	WAN_LIST_FOREACH(aft, &wan_aftup_head, next){
		cnt ++;
	}
	return cnt;
}

static int wan_aftup_start(void)
{
	wan_aftup_t	*aft;
	int		err = 0;

	if (options & WAN_AFTUP_AUTO){
		printf("Starting auto Sangoma Flash update for all card\n");
	}

	if (MakeConnection(ifname_def)){
		printf("%s: Failed to create connection\n", ifname_def);
		return -EINVAL;
	}
	/* Print hardware configuration */
	if (wan_aftup_ioctl(&api_cmd, SIOC_WAN_ALL_HWPROBE, ifname_def)){
		printf("Failed to read list of Sangoma adapters!\n");
		return -EINVAL;
	}
	CloseConnection(ifname_def);
	
	if (wan_aftup_parse_hwprobe(&api_cmd) == 0){
		printf("Exiting from update program ...\n");
		return -EINVAL;
	}

	if ((err = wan_aftup_program(&wan_aftup_head))){
		goto main_done;
	}

	/* Extra code here */
	if (options & WAN_AFTUP_PCIEXPRESS){
		err = wan_aftup_verify_pciexpress(&wan_aftup_head);
		if (err < 0){
			goto main_done;
		}
		if (err){
			printf("\n\n");
			printf(
			"\tPlease shutdown your computer in order to apply the current changes!\n\n");
			err = 0;
		}
	}
	
main_done:
	aft = WAN_LIST_FIRST(&wan_aftup_head);
	while(aft){
		if (aft == WAN_LIST_FIRST(&wan_aftup_head)){
			WAN_LIST_FIRST(&wan_aftup_head) = WAN_LIST_NEXT(aft, next);
		}
		WAN_LIST_REMOVE(aft, next);
		free(aft);
		aft = WAN_LIST_FIRST(&wan_aftup_head);
	}

	return err;
}

static unsigned char title_info[]=
"Sangoma AFT Series card update flash software";

static unsigned char usage_info[]="\n"
"Usage:\n"
"	wan_aftup -auto    : Auto Flash update for Sangoma card\n"
"	wan_aftup -h	   : Print help message\n"
"	wan_aftup -v	   : Print utility version\n";

static void wan_aftup_usage (void)
{
	printf("%s\n",title_info);
	printf("%s\n",usage_info);
}

static void wan_aftup_version (void)
{
	printf("\n%s",title_info);
	printf(" (version %s)\n\n", WAN_AFTUP_VERSION);
}

void hit_any_key(void)
{
	printf("Hit any key to continue... \n");
	getchar();
	getchar();
}

int main(int argc, char* argv[])
{
	int i=0;
	int err;

	memset(&api_cmd,0,sizeof(wan_cmd_api_t));
	api_cmd.cmd=0xFF;

	WAN_LIST_INIT(&wan_aftup_head);

#if defined(__LINUX__)
	strncpy(ifname_def, "w1g1", 7);
#elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
	strncpy(ifname_def, "wag1", 7);
#endif
	for (i = 0; i < argc; i++){

		if (!strcmp(argv[i],"-h")){
			wan_aftup_usage();
			return 0;
		}else if (!strcmp(argv[i],"-v")){
			wan_aftup_version();
			return 0;
		}else if (!strcmp(argv[i],"-firmware")){
			options |= WAN_AFTUP_FORCE_FIRM;
			if (i+1 >= argc){
				printf("\n\tERROR: Invalid command argument!\n\n");
				return -EINVAL;
			}
			memcpy(aft_firmware_force,
				argv[i+1], strlen(argv[i+1]));
			i++;
		}else if (!strcmp(argv[i],"-pcie")){
			options |= WAN_AFTUP_PCIEXPRESS;
		}else if (!strcmp(argv[i],"-auto")){
			options |= WAN_AFTUP_AUTO;
#if 0
		}else if (!strcmp(argv[i],"-i")){
			strncpy(if_name, argv[i+1], strlen(argv[i+1]));
			i++;
#endif
		}
	}

	wan_aftup_version();
	err = wan_aftup_start();
	return err;
}

int exec_read_cmd(void *arg, unsigned int off, unsigned int len, unsigned int *data)
{
	wan_aftup_t	*aft = (wan_aftup_t*)arg;
	int		err;
	struct ifreq	ifr;
	char		msg[100];

	memset(api_cmd.data, 0, WAN_MAX_DATA_SIZE);
	memset(ifr.ifr_name, 0, IFNAMSIZ);
	strncpy(ifr.ifr_name, aft->if_name, strlen(aft->if_name));
	ifr.ifr_data = (char*)&api_cmd;

	api_cmd.cmd=SIOC_WAN_READ_REG;
	api_cmd.bar=0;
	api_cmd.len=len;
	api_cmd.offset=off;

	err = ioctl(sock,SIOC_WAN_DEVEL_IOCTL,&ifr);
	if (err){
		snprintf(msg, 100, "Read Cmd Exec: %s: ",
					ifr.ifr_name);
		perror(msg);
	}

	if (len==1){
		*(unsigned char*)data = *(unsigned char*)api_cmd.data;
	}else if (len==2){
		*(unsigned short*)data = *(unsigned short*)api_cmd.data;
	}else{
		*(unsigned int*)data = *(unsigned int*)api_cmd.data;
	}

	return err;

}

int exec_write_cmd(void *arg, unsigned int off, unsigned int len, unsigned int data)
{
	wan_aftup_t	*aft = (wan_aftup_t*)arg;
	int		err;
	struct ifreq	ifr;
	char		msg[100];

	memset(api_cmd.data, 0, WAN_MAX_DATA_SIZE);
	memset(ifr.ifr_name, 0, IFNAMSIZ);
	strncpy(ifr.ifr_name, aft->if_name, strlen(aft->if_name));
	ifr.ifr_data = (char*)&api_cmd;

	api_cmd.cmd=SIOC_WAN_WRITE_REG;
	api_cmd.bar=0;
	api_cmd.len=len;
	api_cmd.offset=off;

	if (len==1){
		*(unsigned char*)api_cmd.data=(unsigned char)data;
	}else if (len==2){
		*(unsigned short*)api_cmd.data=(unsigned short)data;
	}else{
		*(unsigned int*)api_cmd.data=(unsigned int)data;
	}

	err = ioctl(sock,SIOC_WAN_DEVEL_IOCTL,&ifr);
	if (err){
		snprintf(msg, 100, "Write Cmd Exec: %s: ",
					ifr.ifr_name);
		perror(msg);
	}

	return err;
}

int exec_bridge_read_cmd(void *arg, unsigned int off, unsigned int len, unsigned int *data)
{
	wan_aftup_t	*aft = (wan_aftup_t*)arg;
	int		err;
	struct ifreq	ifr;
	char		msg[100];

	memset(api_cmd.data, 0, WAN_MAX_DATA_SIZE);
	memset(ifr.ifr_name, 0, IFNAMSIZ);
	strncpy(ifr.ifr_name, aft->if_name, strlen(aft->if_name));
	ifr.ifr_data = (char*)&api_cmd;

	api_cmd.cmd=SIOC_WAN_READ_PCIBRIDGE_REG;
	api_cmd.bar=0;
	api_cmd.len=len;
	api_cmd.offset=off;

	err = ioctl(sock,SIOC_WAN_DEVEL_IOCTL,&ifr);
	if (err){
		snprintf(msg, 100, "Read Cmd Exec: %s: ",
					ifr.ifr_name);
		perror(msg);
	}

	if (len==1){
		*(unsigned char*)data = *(unsigned char*)api_cmd.data;
	}else if (len==2){
		*(unsigned short*)data = *(unsigned short*)api_cmd.data;
	}else{
		*(unsigned int*)data = *(unsigned int*)api_cmd.data;
	}

	return err;

}

int exec_bridge_write_cmd(void *arg, unsigned int off, unsigned int len, unsigned int data)
{
	wan_aftup_t	*aft = (wan_aftup_t*)arg;
	int		err;
	struct ifreq	ifr;
	char		msg[100];

	memset(api_cmd.data, 0, WAN_MAX_DATA_SIZE);
	memset(ifr.ifr_name, 0, IFNAMSIZ);
	strncpy(ifr.ifr_name, aft->if_name, strlen(aft->if_name));
	ifr.ifr_data = (char*)&api_cmd;

	api_cmd.cmd=SIOC_WAN_WRITE_PCIBRIDGE_REG;
	api_cmd.bar=0;
	api_cmd.len=len;
	api_cmd.offset=off;

	if (len==1){
		*(unsigned char*)api_cmd.data=(unsigned char)data;
	}else if (len==2){
		*(unsigned short*)api_cmd.data=(unsigned short)data;
	}else{
		*(unsigned int*)api_cmd.data=(unsigned int)data;
	}

	err = ioctl(sock,SIOC_WAN_DEVEL_IOCTL,&ifr);
	if (err){
		snprintf(msg, 100, "Write Cmd Exec: %s: ",
					ifr.ifr_name);
		perror(msg);
	}

	return err;
}

static int aft_a200_a400_warning(wan_aftup_t *aft)
{
	char   	sel[20];
        int	warning=0;
	
        if (aft->board_id == A200_REMORA_SHARK_SUBSYS_VENDOR &&
	    (aft->flash_rev == 7 || 
	     aft->flash_rev == 8)) {
       		warning=1; 	
       	}      

	if (!warning) {
		return 0;
	}

	
        printf ("\n");
        printf ("WARNING: User Confirmation Required!\n");
        printf ("------------------------------------\n");
	printf ("Please confirm your hardware type, an\n");
	printf ("incorrect choice will corrupt the firmware.\n");
	printf ("If firmware gets corrupted, a firmware recovery procedure\n");
	printf ("using a jumper will have to be invoked to recover the card. \n");
	printf ("For recovery procedures refer to: wiki.sangoma.com \n");
        printf ("------------------------------------\n\n");
        printf ("Please confirm hardware type:\n");
	printf (" 1. A200 (front end connector RJ45)\n");
	printf (" 2. A400 (front end connector DB25)\n");
	printf (" Select: [1|2|q]:");

	if (scanf("%s", sel)){
	       	if (strcmp(sel, "q") == 0){
	       		return -EINVAL;
		} else if (strcmp(sel, "1") == 0) {
			return 0;
		} else if (strcmp(sel, "2") == 0) {
			aft->board_id=A400_REMORA_SHARK_SUBSYS_VENDOR;
			return 0;
		} else {
                 	return aft_a200_a400_warning(aft);
		}
	}

	printf ("\nError: Invalid Selection %s\n",sel);
	return -EINVAL;	
}

