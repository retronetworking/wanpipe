/*****************************************************************************
* libxmtp2.c	SS7 MTP2 API Library 
*
* Author(s):	Mike Mueller <ss7box@gmail.com
*               Nenad Corbic <ncorbic@sangoma.com>
*		
*
* Copyright:	(c) 2010 Xygnada Technologies
*                        Sangoma Technologies Inc.
*
* ============================================================================
* 1.6   Nenad Corbic <ncorbic@sangoma.com>
*	May 28 2010
*   Updated for 8 E1 worth of links.
*   Synched up with latest xmtp2 code
*
* 1.5   Nenad Corbic <ncorbic@sangoma.com>
*	Sep 02 2009
*   Updated for full t1/e1 config
*   update freame_per_packet to cfg->mtu
*
* 1.4   Nenad Corbic <ncorbic@sangoma.com>
*	Aug 19 2009
*	Updated for full t1/e1 config
* 
* 1.3   Nenad Corbic <ncorbic@sangoma.com>
*	Jun 3 2008
*	Fixed spelling mistake in function names
*	xmtp2_restart_facility
*	xmtp2_start_facility
*
* 1.2   Nenad Corbic <ncorbic@sangoma.com>
*	May 21 2008
*	Updated so mtp2 links can be configured and
*       reconfigured on the fly.
*
* 1.1 	Nenad Corbic <ncorbic@sangoma.com>
* 	May 1 2008
*	Updated MTP2 Timer configuration
*
* 1.0 	Nenad Corbic <ncorbic@sangoma.com>
* 	May 1 2008
*	Initial Version
*/

#include "libxmtp2.h"

static t_fac_Sangoma_v2 fac_Sangoma[MAX_FACILITIES];

/*===================================================================
 *
 *==================================================================*/
void	info_u (
		const char * object,
		const char * location, 
		int	group,
		const char * cause, 
		const unsigned int detail
		)
{
	printf ("I:%s:%s:%s:%u\n", 
		object,
		location,
		cause,
		detail);

}

/*===================================================================
 *
 *==================================================================*/
void xmtp2_link_util_report (
		const unsigned int ls,
		const unsigned int link,
		const unsigned int msu_octet_count,
		const unsigned int total_octet_count
		)
{
	int util;
	if (total_octet_count != 0)
	{
		util = 100 * (msu_octet_count / total_octet_count);
	}
	else
	{
		util = -1;
	}
	printf ("R:link util:ls %d:link %d:msu oc %u:tot oc %u:util %d\n", 
			ls,
			link,
			msu_octet_count,
			total_octet_count,
			util);
}

/*===================================================================
 *
 *==================================================================*/
int xmtp2_load(void)
{
	int r = system ("./xmtp2km_in");

	if (r < 0) {
		info_u (__FILE__, __FUNCTION__,0,
"failed to start xmtp2km:return value follows", r);
		return r;
	}

	return r;
}

/*===================================================================
 *
 *==================================================================*/
int xmtp2_unload(void)
{
	int rc=system ("modprobe -r xmtp2km");
	//printf("%s:%d: RC=%i\n",__FUNCTION__,__LINE__,rc);
	return rc;
}


/*===================================================================
 *
 *==================================================================*/
int xmtp2_init(void)
{
	memset(&fac_Sangoma,0,sizeof(fac_Sangoma));
	return 0;
}


/*===================================================================
 *
 *==================================================================*/
int xmtp2_open (void)
{
	/* install and open /dev/xmtp2km */
	int xmtp2km_fd;

        xmtp2km_fd = open ("/dev/xmtp2km0", O_RDWR, 0);
	if (xmtp2km_fd < 0) {
		info_u (__FILE__, __FUNCTION__,0,
"failed:open /dev/xmtp2km:xmtp2km_fd follows", xmtp2km_fd);
	}

	return xmtp2km_fd;
}


/*===================================================================
 *
 *==================================================================*/
int xmtp2_close(int fd)
{
	if (fd >= 0) {
		close(fd);
	}
	return 0;
}


/*===================================================================
 *
 *==================================================================*/
static int xmtp2_config (int xmtp2km_fd)
{
	/* pass /dev/xmtp2km its configuration parms */

        int r = ioctl (xmtp2km_fd, XMTP2KM_IOCS_OPSPARMS, &fac_Sangoma);
	if (r) {
		info_u (__FILE__, __FUNCTION__,0,
"failed:ioctl XMTP2KM_IOCS_OPSPARMS:retval follows", r);
	}

	return r;
}

/*===================================================================
 *
 *==================================================================*/
static int xmtp2_unconfig (int xmtp2km_fd)
{
	/* pass /dev/xmtp2km its configuration parms */

        int r = ioctl (xmtp2km_fd, XMTP2KM_IOCS_STOP_FAC, &fac_Sangoma);
	if (r) {
		info_u (__FILE__, __FUNCTION__,0,
"failed:ioctl XMTP2KM_IOCS_OPSPARMS:retval follows", r);
	}

	return r;
}


/*===================================================================
 *
 *==================================================================*/

int xmtp2_stop_link (int fd, xmtp_cfg_link_info_t *cfg)
{
	/* enter the fi and si into the mtp2_link_info table for use later during init */
	int fi = cfg->card-1;
	int si = cfg->slot;

	memset(fac_Sangoma,0,sizeof(fac_Sangoma));

	fac_Sangoma[fi].card_name_number=cfg->card;
	sprintf (fac_Sangoma[fi].card_name, "wanpipe%u", fac_Sangoma[fi].card_name_number);

	fac_Sangoma[fi].frames_in_packet= cfg->mtu ? cfg->mtu : 80;
	fac_Sangoma[fi].clear_channel= cfg->clear_ch;
	fac_Sangoma[fi].configured = MARK;

	/* derived interface name: wXgY where X is the numeral in the card name
	 * and Y is si + 1 */
	sprintf (fac_Sangoma[fi].link_cfg[si].if_name, "w%ug%u", fac_Sangoma[fi].card_name_number, si + 1);

	/* get linkset and link index */
	fac_Sangoma[fi].link_cfg[si].linkset = cfg->linkset;
	fac_Sangoma[fi].link_cfg[si].link    = cfg->link;

	/* read and apply timer values */
	fac_Sangoma[fi].link_cfg[si].cfg_T1 = cfg->cfg_T1 ? cfg->cfg_T1 : MTP2_SEC_MS(13.0);
	fac_Sangoma[fi].link_cfg[si].cfg_T2 = cfg->cfg_T2 ? cfg->cfg_T2 : MTP2_SEC_MS(11.5);
	fac_Sangoma[fi].link_cfg[si].cfg_T3 = cfg->cfg_T3 ? cfg->cfg_T3 : MTP2_SEC_MS(11.5);
	fac_Sangoma[fi].link_cfg[si].cfg_T4Pe = cfg->cfg_T4Pe ? cfg->cfg_T4Pe : MTP2_SEC_MS(0.6);
	fac_Sangoma[fi].link_cfg[si].cfg_T4Pn = cfg->cfg_T4Pn ? cfg->cfg_T4Pn : MTP2_SEC_MS(2.3);
	fac_Sangoma[fi].link_cfg[si].cfg_T5 = cfg->cfg_T5 ? cfg->cfg_T5 : MTP2_SEC_MS(0.12);
	fac_Sangoma[fi].link_cfg[si].cfg_T6 = cfg->cfg_T6 ? cfg->cfg_T6 : MTP2_SEC_MS(3.0);
	fac_Sangoma[fi].link_cfg[si].cfg_T7 = cfg->cfg_T7 ? cfg->cfg_T7 : MTP2_SEC_MS(1.0);

	fac_Sangoma[fi].link_cfg[si].configured = MARK;

	printf("Stopping fi=%i si=%i  linkset=%i link=%i\n",
			fi,si,cfg->linkset,cfg->link);
	
	cfg->facility=fi;

	return xmtp2_unconfig (fd);
}


int xmtp2_conf_link (int fd, xmtp_cfg_link_info_t *cfg)
{
	/* enter the fi and si into the mtp2_link_info table for use later during init */
	int fi = cfg->card-1;
	int si = cfg->slot;

	memset(fac_Sangoma,0,sizeof(fac_Sangoma));

	fac_Sangoma[fi].card_name_number=cfg->card;
	sprintf (fac_Sangoma[fi].card_name, "wanpipe%u", fac_Sangoma[fi].card_name_number);

	fac_Sangoma[fi].frames_in_packet=cfg->mtu;
	fac_Sangoma[fi].clear_channel= cfg->clear_ch;
	fac_Sangoma[fi].configured = MARK;


	/* derived interface name: wXgY where X is the numeral in the card name
	 * and Y is si + 1 */
	sprintf (fac_Sangoma[fi].link_cfg[si].if_name, "w%ug%u", fac_Sangoma[fi].card_name_number, si + 1);

	/* get linkset and link index */
	fac_Sangoma[fi].link_cfg[si].linkset = cfg->linkset;
	fac_Sangoma[fi].link_cfg[si].link    = cfg->link;

	/* read and apply timer values */
	fac_Sangoma[fi].link_cfg[si].cfg_T1 = cfg->cfg_T1 ? cfg->cfg_T1 : MTP2_SEC_MS(13.0);
	fac_Sangoma[fi].link_cfg[si].cfg_T2 = cfg->cfg_T2 ? cfg->cfg_T2 : MTP2_SEC_MS(11.5);
	fac_Sangoma[fi].link_cfg[si].cfg_T3 = cfg->cfg_T3 ? cfg->cfg_T3 : MTP2_SEC_MS(11.5);
	fac_Sangoma[fi].link_cfg[si].cfg_T4Pe = cfg->cfg_T4Pe ? cfg->cfg_T4Pe : MTP2_SEC_MS(0.6);
	fac_Sangoma[fi].link_cfg[si].cfg_T4Pn = cfg->cfg_T4Pn ? cfg->cfg_T4Pn : MTP2_SEC_MS(2.3);
	fac_Sangoma[fi].link_cfg[si].cfg_T5 = cfg->cfg_T5 ? cfg->cfg_T5 : MTP2_SEC_MS(0.12);
	fac_Sangoma[fi].link_cfg[si].cfg_T6 = cfg->cfg_T6 ? cfg->cfg_T6 : MTP2_SEC_MS(3.0);
	fac_Sangoma[fi].link_cfg[si].cfg_T7 = cfg->cfg_T7 ? cfg->cfg_T7 : MTP2_SEC_MS(1.0);

	fac_Sangoma[fi].link_cfg[si].configured = MARK;

	printf("Configuring fi=%i si=%i  linkset=%i link=%i\n",
			fi,si,cfg->linkset,cfg->link);
	
	cfg->facility=fi;

	return xmtp2_config (fd);
}


/*===================================================================
 * xmtp2_cmd
 *==================================================================*/
int xmtp2_cmd (
	int xmtp2km_fd,
	const unsigned int linkset, 
	const unsigned int link, 
	const unsigned int cause, 
	const unsigned int caller)
{
	unsigned int cmd = 0;
	int r=-1;

	t_linkset_link	linkset_link;

	linkset_link.linkset = linkset;
	linkset_link.link = link;
	linkset_link.caller = caller;
	
	switch (cause)
	{
		case LSC_CAUSE_POWER_ON:
			cmd = XMTP2KM_IOCS_PWR_ON;
			break;
		case LSC_CAUSE_EMERGENCY:
			cmd = XMTP2KM_IOCS_EMERGENCY;
			break;
		case LSC_CAUSE_EMERGENCY_CEASES:
			cmd = XMTP2KM_IOCS_EMERGENCY_CEASES;
			break;
		case LSC_CAUSE_START:
			cmd = XMTP2KM_IOCS_STARTLINK;
			break;
		case LSC_CAUSE_STOP:
			cmd = XMTP2KM_IOCS_STOPLINK;
			break;
		default:
			info_u (__FILE__, __FUNCTION__,0, "lsc invalid cause ",cause);
			return 1;
	}
	
        r = ioctl (xmtp2km_fd, cmd, &linkset_link);
	if (r)
	{
		info_u (__FILE__, __FUNCTION__,0,
"ioctl failed:retval/cause follows", r);
		return r;
	}

	info_u (__FILE__, __FUNCTION__, 3,
"cause follows", cause);
	return r;
	
}


/*===================================================================
 *
 *==================================================================*/
int xmtp2_power_on_links (int fd) 
{
	int i;
	int err=-1;
	for (i = 0; i < MAX_FACILITIES; i++)
       	{
		if (!fac_Sangoma[i].configured) continue;

		info_u (__FILE__, __FUNCTION__, 0,
"facility configured:facility index follows", i);
		int j;
		for (j=0; j<E1_CHANNELS_IN_FRAME; j++)
		{
			if (fac_Sangoma[i].link_cfg[j].configured)
			{
				int ls  = fac_Sangoma[i].link_cfg[j].linkset;
				int lnk = fac_Sangoma[i].link_cfg[j].link;
				err = xmtp2_cmd (fd, ls, lnk, LSC_CAUSE_POWER_ON, MGMT);
				if (err) {
					printf("Error: Failed power on i=%i j=%i ls=%i lnk=%i\n",
						i,j,ls,lnk);
					return err;
				}
				printf("Power on i=%i j=%i ls=%i lnk=%i\n",
						i, j, ls,lnk);
#if 0
				info_u (__FILE__, __FUNCTION__, 0,
"link being powered on:facility/channel/linkset/link follow", lnk);
#endif
			}
		}
	}
	return err;
}


/*===================================================================
 *
 *==================================================================*/
int  xmtp2_sangoma_start (int fi)
{
	char cmd_string_start[64]="wanrouter start ";
	strcat (cmd_string_start, fac_Sangoma[fi].card_name);
	return system (cmd_string_start);
}



/*===================================================================
 *
 *==================================================================*/
int xmtp2_start_facilities (void)
{
	int i;
	int err;
	for (i = 0; i < MAX_FACILITIES; i++)
       	{
		if (! fac_Sangoma[i].configured) {
			 continue;
		}

		printf("Starting Facility %i\n",
				i);

		err=xmtp2_sangoma_start (i);
		if (err) {
			return err;
		}
	}

	return 0;
}


int xmtp2_restart_facility (int card, int slot)
{	
	char cmd_string_start[64];
	
	/* Interface not running */
	sprintf(cmd_string_start,"wanrouter stop w%ig%i",card,slot+1);
	system (cmd_string_start);

	sprintf(cmd_string_start,"wanrouter start w%ig%i",card,slot+1);
	return system (cmd_string_start);
}

int xmtp2_start_facility (int card, int slot)
{	
	char cmd_string_ifstatus[64];
	char cmd_string_start[64];
	int err;

	sprintf(cmd_string_ifstatus,"ifconfig w%ig%i 2> /dev/null > /dev/null",card,slot+1);
	err=system (cmd_string_ifstatus);
	if (err) {
		/* Interface not running */
		sprintf(cmd_string_start,"wanrouter start wanpipe%i w%ig%i",card,card,slot+1);
		system (cmd_string_start);

		return system (cmd_string_ifstatus);
	} else {
		/* Interface already running */
		return 0;
	}
}

/*===================================================================
 *
 *==================================================================*/
void xmtp2_sangoma_stop (int fi)
{
char	cmd_string_stop[64]="wanrouter stop ";
	strcat (cmd_string_stop, fac_Sangoma[fi].card_name);
	system (cmd_string_stop);
}


/*===================================================================
 *
 *==================================================================*/
void xmtp2_stop_facilities (void)
{
	int i;
	for (i = 0; i < MAX_FACILITIES; i++)
       	{
		if (! fac_Sangoma[i].configured) continue;
		xmtp2_sangoma_stop (i);
	}
}


/*===================================================================
 *
 *==================================================================*/
int xmtp2_report_link_load (int xmtp2km_fd, t_link_opm *opm_table) 
{
	opm_table[0].ls = -1;

	int r = ioctl (xmtp2km_fd, XMTP2KM_IOCG_GETOPM, (uint8_t *)opm_table);
	if (r < 0)
	{
		info_u (__FILE__, __FUNCTION__,0,
"failed:ioctl XMTP2KM_IOCG_GETOPM,:retval follows", r);
		return r;
	}

	int i;
	for (i=0; i < MAX_MTP2LINKS; i++)
	{
		if (opm_table[i].ls < 0) break;
		xmtp2_link_util_report (
			opm_table[i].ls,
			opm_table[i].link,
			opm_table[i].msu_octet_count,
			opm_table[i].total_octet_count);
	}

	return 0;
}


/*===================================================================
 *
 *==================================================================*/
int xmtp2_retrieve_bsnt (
	int xmtp2km_fd,
	const unsigned int linkset, 
	const unsigned int link, 
	const unsigned int caller) 
{ 
	t_bsnt retrieve_bsnt_msg;

	retrieve_bsnt_msg.ls = linkset;
	retrieve_bsnt_msg.link = link;
	
	int r = ioctl (xmtp2km_fd, XMTP2KM_IOCX_GETBSNT, (uint8_t *)&retrieve_bsnt_msg);
	if (r < 0)
	{
		info_u (__FILE__, __FUNCTION__,0,
"failed:ioctl XMTP2KM_IOCX_GETBSNT:retval follows", r);
		return r;
	}

	if (retrieve_bsnt_msg.bsnt_retrieved == BSNT_RETRIEVED) {
		/* local bsnt retrieved */

	} else {
		/* local bsnt canceled */

	}

	/* retrieve_bsnt_msg.bsnt; */

	return retrieve_bsnt_msg.bsnt_retrieved;
}


/*===================================================================
 *
 *==================================================================*/
int xmtp2_send_msu (int xmtp2km_fd, t_su_buf * p_msu) 
{
        int r = ioctl (xmtp2km_fd, XMTP2KM_IOCS_PUTMSU, p_msu);
	if (r < 0)
	{
		info_u (__FILE__, __FUNCTION__,0,
"failed:ioctl XMTP2KM_IOCS_PUTMSU:retval follows", r);
		return r;
	}

	return 0;
}


/*===================================================================
 *
 *==================================================================*/
int xmtp2_read_msu(int xmtp2km_fd,  t_su_buf * p_msu)
{
        int r = ioctl (xmtp2km_fd, XMTP2KM_IOCG_GETMSU, p_msu);
	if (r < 0){
		info_u (__FILE__, __FUNCTION__,0,
"failed:ioctl XMTP2KM_IOCG_GETMSU:retval and p_msu follow", r);
		return -1;
	}

	return r;
}


/*===================================================================
 *
 *==================================================================*/
void xmtp2_print_mem_hex_ascii (FILE * p_fd, uint8_t *buff, int len)
{
  int i=0, j=0, c=0, printnext = 1;

  fprintf (p_fd, "\tMARK < >\n");
  fprintf (p_fd, "-------------------------------------------------------------------------\n");
  if (len)
  {
    if (len % 16) c = len + (16 - len % 16);
    else c = len;
  }
  else c = len;

  for (i = 0; i < c; i++)
  {
    if (printnext) { printnext--; fprintf (p_fd, "%.4x ", i & 0xffff); }

    if (i < len) fprintf (p_fd, "%3.2x", buff[i]&0xff);
    else fprintf (p_fd, "   ");

    if (!((i+1)%8))
    {
      if ((i+1)%16) fprintf (p_fd, " -");
      else
      {
        fprintf (p_fd, "  ");

        for(j = i - 15; j <= i; j++)
        {
          if (j < len)
          {
            if ( (buff[j] & 0xff) >= 0x20 && (buff[j] & 0xff) <= 0x7e)
              fprintf (p_fd, "%c", buff[j]&0xff);
            else fprintf (p_fd, ".");
          }
          else fprintf (p_fd, " ");
        }

        fprintf (p_fd, "\n");
        printnext = 1;
      }
    }
  }
  fprintf (p_fd, "-------------------------------------------------------------------------\n\n");
}


/*===================================================================
 *
 *==================================================================*/
void xmtp2_print_msu (t_su_buf * p_msu2)
{
	xmtp2_print_mem_hex_ascii (stdout, p_msu2->su_buf, p_msu2->hdr.su_octet_count);
	fflush (stdout);
}

