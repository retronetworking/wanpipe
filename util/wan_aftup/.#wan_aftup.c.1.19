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
**/

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
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
# include <linux/wanpipe_common.h>
# include <linux/sdlasfm.h>
# include <linux/if_wanpipe.h>
#else
# include <net/if.h>
# include <net/wanpipe_defines.h>
# include <net/wanpipe_common.h>
# include <net/sdlasfm.h>
# include <net/wanpipe.h>
#endif

#include "wan_aft_cpld.h"
#include "wan_aftup.h"

#define WAN_AFTUP_VERSION	"1.5"

#define WAN_AFTUP_NONE		0x00
#define WAN_AFTUP_AUTO		0x01

static int		sock;
static wan_cmd_api_t	api_cmd;
static unsigned char	ifname_def[20];
static struct ifreq	req;
int			options = 0x00;


WAN_LIST_HEAD(wan_aftup_head_t, wan_aftup_) wan_aftup_head = 
			WAN_LIST_HEAD_INITIALIZER(wan_aftup_head);

int progress_bar(char*);
void main_menu(void);
void read_chip_control_menu(void);
void registers_and_internal_ram_test_menu(void);
void setup_chip_configuration_menu(void);
int exec_command(int cmd);
int exec_reload_pci_cmd(void);
int exec_read_cmd(void *, unsigned int off, unsigned int len, unsigned int *data);
int exec_write_cmd(void *, unsigned int off, unsigned int len, unsigned int data);

void hit_any_key(void);

extern int pmc_initialization (int sock);
//extern int cpld_initialization (int sock);
extern int check_flash_id(wan_aft_cpld_t*, int, int, int*);
extern int reload_flash(wan_aft_cpld_t*, int);
extern int update_flash(wan_aft_cpld_t*, int, int, char*);
extern int release_board(wan_aft_cpld_t *);

//struct cmd_list_struct cmd_list[]={
//	{"READ", SIOC_WAN_READ_REG},
//	{"WRITE",SIOC_WAN_WRITE_REG},
//	{".",0xFF}
//};

int progress_bar(char *msg)
{
	static int index = 0;

	if (index++ == 0){
		printf("\r%s |", msg);
	}else if (index++ == 1){
		printf("\r%s /", msg);
	}else{
		printf("\r%s -", msg);
		index = 0;
	}
	fflush(stdout);
	return 0;
}
int MakeConnection(char *ifname) 
{
	char			error_msg[100];
	struct ifreq		ifr;
#if defined(__LINUX__)
	struct sockaddr_ll	sa;
	
	memset(&sa,0,sizeof(struct sockaddr_ll));
	errno = 0;
   	sock = socket(AF_PACKET, SOCK_DGRAM, 0);
   	if( sock < 0 ) {
      		perror("Socket");
      		return -EIO;
   	} /* if */
  
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(sock,SIOCGIFINDEX,&ifr)){
		sprintf(error_msg, "Get index: %s", ifname);
		perror(error_msg);
		close(sock);
		return -EIO;
	}

	sa.sll_protocol = htons(0x17);
	sa.sll_family=AF_PACKET;
	sa.sll_ifindex = ifr.ifr_ifindex;

        if(bind(sock, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll)) < 0){
		sprintf(error_msg, "Bind: %s", ifname);
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

int CloseConnection(char *ifname)
{
	close(sock);
	return 0;
}

int wan_aftup_ioctl(wan_cmd_api_t *api_cmd, int cmd, char *ifname)
{
	int ret;

	api_cmd->cmd = cmd;
	req.ifr_data = (void*)api_cmd;
	strcpy(req.ifr_name, ifname);
	if ((ret = ioctl(sock, SIOC_WAN_DEVEL_IOCTL, &req)) < 0){
		return -EINVAL;
	}
	return 0;
}

static int wan_aftup_gettype(wan_aftup_t *iface, char *type)
{
	
	if (strncmp(type,"AFT-A101",8) == 0){
		strcpy(iface->prefix_fw, "A101");
		iface->cpld.adptr_type = A101_ADPTR_1TE1;	//WAN_AFTUP_A101;
	}else if (strncmp(type,"AFT-A102",8) == 0){
		strcpy(iface->prefix_fw, "A101");
		iface->cpld.adptr_type = A101_ADPTR_2TE1;	//WAN_AFTUP_A101;
	}else if (strncmp(type,"AFT-A104",8) == 0){
		strcpy(iface->prefix_fw, "A104");
		iface->cpld.adptr_type = A104_ADPTR_4TE1;
	}else if (strncmp(type,"AFT-A300",8) == 0){
		strcpy(iface->prefix_fw, "A301");
		iface->cpld.adptr_type = A300_ADPTR_U_1TE3;	//WAN_AFTUP_A300;
	}else{
		return -EINVAL;
	}
	return 0;
}

static int wan_aftup_getfile(wan_aftup_t *iface)
{
	FILE 		*f;
	char		ver[10], sel_ver[10];
	DIR		*dir;
	struct dirent	*ent;
	int		versions_no = 0, ver_no = -1;

	if (!(dir = opendir("."))){
		perror("opendir");
		return -EINVAL;
	}
	if (!(options & WAN_AFTUP_AUTO)){
		printf("List of available versions: \n");
	}
	while((ent = readdir(dir))){
		if (strncmp(ent->d_name, iface->prefix_fw, strlen(iface->prefix_fw)) ==0){
			int	i = strlen(iface->prefix_fw);
			int	new_ver;
			
			while(!isdigit(ent->d_name[i])) i++;
			new_ver = (ent->d_name[i]-'0') * 16 + (ent->d_name[i+1]-'0');
			if (ver_no == -1 || new_ver > ver_no){
				int j=0;
				ver_no = new_ver;
				strcpy(iface->firmware, ent->d_name);
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
		printf("\nYour current directory doesn't include update file!\n");
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
				sprintf(sel_ver, "%X", ver_no);
			}
			if (strlen(sel_ver) == 2){
				strcpy(ver, sel_ver);
			}else if (strlen(sel_ver) == 1){
				ver[0] = '0';
				ver[1] = sel_ver[0];
				ver[2] = '\0';
			}else{
				goto wan_aftup_getfile_again;
			}
		}
		strcpy(iface->firmware, iface->prefix_fw);
		strcat(iface->firmware, "_V");
		strcat(iface->firmware, ver);
		strcat(iface->firmware, ".BIN");
	}
	
	f = fopen(iface->firmware, "rb");
	if (f == NULL){
		printf("Failed to open file %s for %s\n",
				iface->firmware, iface->if_name);
		return -EINVAL;
	}
	fclose(f);
	printf("\n");
	return 0;
}

static int wan_aftup_program_card(wan_aftup_t *iface)
{
	int	flash_id, tmp, err;
	char	ch;
	
	if (wan_aftup_getfile(iface)){
		return -EINVAL;
	}

	/* Release board internal reset (AFT-T1/E1/T3/E3 */
	if (release_board(&iface->cpld)){
		printf("ERROR: %s: Failed access to the board!\n",
					iface->if_name);
		return -EINVAL;
	}

	/* Check Flash ID */
	check_flash_id(
			&iface->cpld,
			MEMORY_TYPE_FLASH,
			USER_SECTOR_FLASH,
			&flash_id);
	if (flash_id != 0x014F && flash_id != 0x20E3){
		printf("ERROR: %s: Failed to read Flash id (%04X)\n",
				iface->if_name, flash_id);
		return -EINVAL;
	}
	printf("%s: Current Sangoma Flash: Revision=%X ID=0x%04X\n",
					iface->if_name,
					iface->flash_rev,
					flash_id);

	err = update_flash(
			&iface->cpld,
			USER_SECTOR_FLASH,
			MEMORY_TYPE_FLASH,
			iface->firmware);
	if (err){
		printf("ERROR: %s: Failed to re-program flash!\n",
					iface->if_name);
		return -EINVAL;
	}
	
	printf("%s: Sangoma Flash update\t\t\tDONE\n", iface->if_name);
	if (!(options & WAN_AFTUP_AUTO)){
		printf("\n%s: You have to reload flash in order to new flash code will take place\n",
						iface->if_name);
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
	printf("%s: Reload Sangoma flash\t\t\tDONE\n", iface->if_name);
	reload_flash(&iface->cpld, USER_SECTOR_FLASH);

	usleep(1000000);

	/* Read flash revision */
	if (wan_aftup_ioctl(&api_cmd, SIOC_WAN_SET_PCI_BIOS, iface->if_name)){
		printf("%s: Failed to restore PCI registers!\n",
					iface->if_name);
		return -EINVAL;
	}

	/* Release board internal reset (AFT-T1/E1/T3/E3 */
	if (release_board(&iface->cpld)){
		printf("%s: Failed access to the board!\n", 
					iface->if_name);
		return -EINVAL;
	}

	/* Check Flash ID */
	check_flash_id(&iface->cpld, MEMORY_TYPE_FLASH, USER_SECTOR_FLASH, &tmp);
	if (tmp != flash_id){
		printf("%s: Failed to read Flash ID (new flash id %04X)\n",
					iface->if_name, tmp);
		return -EINVAL;
	}
	
#if 0
	/* Read new Flash revision */
	if (wan_aftup_ioctl(&api_cmd, SIOC_WAN_COREREV, iface->if_name)){
		printf("%s: Failed to read new Flash revision!\n",
						iface->if_name);
		return -EINVAL;
	}
	printf("%s: New Sangoma Flash revision %X\n",
				iface->if_name, api_cmd.data[0]);
	if (api_cmd.data[0] < iface->flash_rev){
		printf("%s: New flash revision is %d (previous flash revision %d)\n",
				iface->if_name, api_cmd.data[0], iface->flash_rev);
		printf("You re-program flash with older version!\n");
		printf("Please, contact Sangoma Technologies (905.474.1990)!\n");
		return -EINVAL;
	}
#endif

	printf("%s: Sangoma Flash updated successfully\n\n", 
					iface->if_name);
	return 0;	
}

static int wan_aftup_program(struct wan_aftup_head_t *head)
{
	wan_aftup_t	*iface = NULL;
	int		err;

	WAN_LIST_FOREACH(iface, head, next){
		if (MakeConnection(iface->if_name)){
			printf("%s: Failed to create socket to the driver!\n",
						 iface->if_name);
			continue;
		}

		/* Read revision */
		if (wan_aftup_ioctl(&api_cmd, SIOC_WAN_COREREV,iface->if_name)){
			printf("%s: Failed to read flash revision!\n",
						iface->if_name);
			err = -EINVAL;
			goto program_done;
		}
		iface->flash_rev = api_cmd.data[0];
		if (wan_aftup_program_card(iface)){
			printf("\n%s: Failed to re-program flash!\n",
						iface->if_name);
			err = -EINVAL;
			goto program_done;
		}

program_done:
		CloseConnection(iface->if_name);
	}

	return 0;
}
static int wan_aftup_parse_hwprobe(wan_cmd_api_t *api_cmd)
{
	wan_aftup_t	*iface = NULL, *iface_prev = NULL;
	char		sel_name[20], *tmp = NULL;
	int		j, cnt = 0;
	
	tmp = strtok(api_cmd->data, "\n");
	while (tmp){
		/* Create new interface structure */
		iface = malloc(sizeof(wan_aftup_t));
		if (iface == NULL){
			printf("ERROR: Failed allocate memory!\n");
			return 0;
		}
		iface->cpld.private = iface;
		strcpy(iface->hwinfo, tmp);

		if (iface_prev == NULL){
			WAN_LIST_INSERT_HEAD(
					&wan_aftup_head,
					iface,
					next);
		}else{
			WAN_LIST_INSERT_AFTER(
					iface_prev,
					iface,
					next);
		}
		iface_prev = iface;
		tmp = strtok(NULL, "\n");
	}

#if 0
	WAN_LIST_FOREACH(iface, &wan_aftup_head, next){
		printf("<%s>\n", iface->hwinfo);
	}
#endif
	if (!(options & WAN_AFTUP_AUTO)){
		printf("Sangoma AFT card list:\n");
	}
	iface = WAN_LIST_FIRST(&wan_aftup_head);
	while(iface){
	
		/* Use api_cmd structure to parse hwprobe info */
		strcpy(api_cmd->data, iface->hwinfo);
		tmp = strtok(api_cmd->data, ":");
		if (tmp == NULL){
			printf("ERROR:%d: Internal error (hwinfo:%s)\n",
						__LINE__, iface->hwinfo);
			iface_prev = iface;
			iface = WAN_LIST_NEXT(iface_prev, next);
			if (iface_prev == WAN_LIST_FIRST(&wan_aftup_head)){
				WAN_LIST_FIRST(&wan_aftup_head) = iface;
			}else{
				WAN_LIST_REMOVE(iface_prev, next);
			}
			free(iface_prev);
			continue;
		}
		/* got interface name */
		strcpy(iface->if_name, tmp);
		tmp = strtok(NULL, ":");
		while(*tmp == ' ') tmp++;
		j = 0;
		while(tmp[j] != ' ' && tmp[j] != '\0') j++;
		tmp[j]='\0';
		if (wan_aftup_gettype(iface, tmp)){
			iface_prev = iface;
			iface = WAN_LIST_NEXT(iface_prev, next);
			if (iface_prev == WAN_LIST_FIRST(&wan_aftup_head)){
				WAN_LIST_FIRST(&wan_aftup_head) = iface;
			}else{
				WAN_LIST_REMOVE(iface_prev, next);
			}
			free(iface_prev);
			continue;
		}
		if (!(options & WAN_AFTUP_AUTO)){
			printf(" %s\n", iface->hwinfo);		
		}
		iface = WAN_LIST_NEXT(iface, next);
	}

#if 0
	WAN_LIST_FOREACH(iface, &wan_aftup_head, next){
		printf("<%s:%d:%s>\n",
				iface->if_name,
				iface->cpld.adptr_type,
				iface->prefix_fw);
	}
#endif
	if (!(options & WAN_AFTUP_AUTO) && !WAN_LIST_EMPTY(&wan_aftup_head)){
		iface = WAN_LIST_FIRST(&wan_aftup_head);
		printf("\n");
		printf("Please select card interface [def=%s; q=exit] > ",
					iface->if_name);
		scanf("%s", sel_name);
		printf("\n");

		while(iface){
			if (strcmp(iface->if_name, sel_name) == 0){
				/* This interface is selected */
				iface = WAN_LIST_NEXT(iface, next);
			}else{
				iface_prev = iface;
				iface = WAN_LIST_NEXT(iface_prev, next);
				if (iface_prev == WAN_LIST_FIRST(&wan_aftup_head)){
					WAN_LIST_FIRST(&wan_aftup_head) = iface;
				}else{
					WAN_LIST_REMOVE(iface_prev, next);
				}
				free(iface_prev);
				continue;
			}
		}
	}

	WAN_LIST_FOREACH(iface, &wan_aftup_head, next){
		cnt ++;
	}
	return cnt;
}

static int wan_aftup_start(void)
{
	wan_aftup_t	*iface;
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

main_done:
	iface = WAN_LIST_FIRST(&wan_aftup_head);
	while(iface){
		if (iface == WAN_LIST_FIRST(&wan_aftup_head)){
			WAN_LIST_FIRST(&wan_aftup_head) = WAN_LIST_NEXT(iface, next);
		}
		WAN_LIST_REMOVE(iface, next);
		free(iface);
		iface = WAN_LIST_FIRST(&wan_aftup_head);
	}

	return err;
}

static unsigned char title_info[]=
"Sangoma AFT Series card update flash software";

static unsigned char usage_info[]="\n"
"Usage:\n"
"	wan_aftup -h	: Print help message\n"
"	wan_aftup -auto	: Auto Flash update for Sangoma card\n"
"	wan_aftup -v	: Print utility version\n";

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
	strcpy(ifname_def, "wp1aft1");
#elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
	strcpy(ifname_def, "wpaaft1");
#endif
	for (i = 0; i < argc; i++){

		if (!strcmp(argv[i],"-h")){
			wan_aftup_usage();
			return 0;
		}else if (!strcmp(argv[i],"-v")){
			wan_aftup_version();
			return 0;
		}else if (!strcmp(argv[i],"-auto")){
			options |= WAN_AFTUP_AUTO;
#if 0
		}else if (!strcmp(argv[i],"-i")){
			strcpy(if_name, argv[i+1]);
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
	wan_aftup_t	*iface = (wan_aftup_t*)arg;
	int		err;
	struct ifreq	ifr;

	memset(api_cmd.data, 0, WAN_MAX_DATA_SIZE);
	strcpy(ifr.ifr_name, iface->if_name);
	ifr.ifr_data = (char*)&api_cmd;

	api_cmd.cmd=SIOC_WAN_READ_REG;
	api_cmd.bar=0;
	api_cmd.len=len;
	api_cmd.offset=off;

	err = ioctl(sock,SIOC_WAN_DEVEL_IOCTL,&ifr);
	if (err){
		perror("Read Cmd Exec: ");
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
	wan_aftup_t	*iface = (wan_aftup_t*)arg;
	int		err;
	struct ifreq	ifr;

	memset(api_cmd.data, 0, WAN_MAX_DATA_SIZE);
	strcpy(ifr.ifr_name, iface->if_name);
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
		perror("Write Cmd Exec: ");
	}

	return err;
}
