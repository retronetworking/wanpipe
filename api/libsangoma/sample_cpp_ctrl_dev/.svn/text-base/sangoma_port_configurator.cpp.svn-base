//////////////////////////////////////////////////////////////////////
// sangoma_port_configurator.cpp: implementation of the sangoma_port_configurator class.
//
// Author	:	David Rokhvarg	<davidr@sangoma.com>
//////////////////////////////////////////////////////////////////////

#include "sangoma_port_configurator.h"

#if defined(__WINDOWS__)
 #include <setupapi.h>	//for SetupDiXXX() functions
 #include <initguid.h>	//for GUID instantination
 #include <devguid.h>	//for DEFINE_GUID() 
 #include "public.h"	//for GUID_DEVCLASS_SANGOMA_ADAPTER
#endif

#define DBG_CFG		if(1)printf("PORTCFG:");if(1)printf
#define _DBG_CFG	if(1)printf

#define INFO_CFG	if(1)printf("PORTCFG:");if(1)printf
#define _INFO_CFG	if(1)printf

#define ERR_CFG		if(1)printf("PORTCFG:");if(1)printf
#define _ERR_CFG	if(1)printf

//////////////////////////////////////////////////////////////////////

//these are recommendations only:
#define MIN_RECOMMENDED_BITSTREAM_MTU	256
#define MIN_RECOMMENDED_HDLC_MTU		2048

/*!
  \brief Brief description
 *
 */
static int get_number_of_channels(unsigned int chan_bit_map)
{
	int chan_counter = 0, i;

	for(i = 0; i < (int)(sizeof(unsigned int)*8); i++){
		if(chan_bit_map & (1 << i)){
			chan_counter++;
		}
	}
	return chan_counter;
}

/*!
  \brief Brief description
 *
 */
static int get_recommended_bitstream_mtu(int num_of_channels)
{
	int recommended_mtu = num_of_channels * 32;//<--- will work well in most cases

	//
	//Smaller mtu will cause more interrupts.
	//Recommend at least MIN_RECOMMENDED_BITSTREAM_MTU, user will set to smaller, if timing is important.
	//
	while(recommended_mtu < MIN_RECOMMENDED_BITSTREAM_MTU){
		recommended_mtu = recommended_mtu * 2;
	}

	return recommended_mtu;
}

/*!
  \brief Brief description
 *
 */
static int check_mtu(int mtu, int hdlc_streaming, unsigned int channel_bitmap)
{
	DBG_CFG("%s(): channel_bitmap: 0x%X\n", __FUNCTION__, channel_bitmap);

	if(mtu % 4){
		ERR_CFG("Invalid MTU (%d). Must be divizible by 4.\n", mtu);
		return 1;
	}

	//if BitStream, check MTU is also divisible by the number of channels (timeslots) in the group
	if(hdlc_streaming == WANOPT_NO){

		int num_of_channels = get_number_of_channels(channel_bitmap);

		DBG_CFG("num_of_channels in bitmap: %d\n", num_of_channels);

		if(mtu % num_of_channels){
			ERR_CFG("Invalid MTU. For 'BitStream' Must be divisible by 4 AND by the number of\n\
						\tchannels in the group (%d). Recommended MTU: %d.\n",
						num_of_channels, get_recommended_bitstream_mtu(num_of_channels));
			return 2;
		}
	}//if()

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*!
  \brief Brief description
 *
 */
sangoma_port_configurator::sangoma_port_configurator()
{
#if defined(__WINDOWS__)
	hPortRegistryKey = (struct HKEY__ *)INVALID_HANDLE_VALUE;
#endif
}

/*!
  \brief Brief description
 *
 */
sangoma_port_configurator::~sangoma_port_configurator()
{
#if defined(__WINDOWS__)
	if(hPortRegistryKey != (struct HKEY__ *)INVALID_HANDLE_VALUE){
		RegCloseKey(hPortRegistryKey);
	}
#endif
}


/*!
  \brief SET new configuration of the API driver.
 *
 */
int sangoma_port_configurator::SetPortVolatileConfigurationCommand(port_cfg_t *port_cfg)
{
	int err = sangoma_driver_port_set_config(wp_handle, port_cfg, wp_number);
	if (err) {
		err = 1;
	}
	return err;
}

/*!
  \brief Brief description
 *
 */
int sangoma_port_configurator::get_configration(port_cfg_t *port_cfg)
{
	int err = sangoma_driver_port_get_config(wp_handle, port_cfg, wp_number);
	if (err) {
		err = 1;
	}
	return err;
}


/*!
  \brief Function to check correctness of 'port_cfg_t' structure.
 *
 */
int sangoma_port_configurator::check_port_cfg_structure(port_cfg_t *port_cfg)
{
	unsigned int i, rc = 0;
	unsigned long tmp_active_ch = 0;
	unsigned long cumulutive_active_ch = 0;

	wandev_conf_t		*wandev_conf = &port_cfg->wandev_conf;
	sdla_fe_cfg_t		*sdla_fe_cfg = &wandev_conf->fe_cfg;
	wanif_conf_t		*wanif_conf;

	DBG_CFG("%s()\n", __FUNCTION__);

	INFO_CFG("Running Driver Configration structure check...\n");
	if(port_cfg->num_of_ifs < 1 || port_cfg->num_of_ifs > NUM_OF_E1_TIMESLOTS){
			_INFO_CFG("Invalid number of Timeslot Groups - %d! Should be between %d and %d (including).\n",
				port_cfg->num_of_ifs, 1, NUM_OF_E1_TIMESLOTS);
			return 1;
	}

	for(i = 0; i < port_cfg->num_of_ifs; i++){

		//check all logic channels were configured
		wanif_conf = &port_cfg->if_cfg[i];
		if(wanif_conf->active_ch == 0){
			_INFO_CFG("Group %d - no T1/E1 channels specified! Expecting a bitmap of channels.\n", i + 1);
			rc = 1; break;
		}

		//check the channels are valid for media type.
		if(sdla_fe_cfg->media == WAN_MEDIA_T1){
			unsigned int t1_invalid_channels = ~T1_ALL_CHANNELS_BITMAP;

			if(wanif_conf->active_ch & (t1_invalid_channels)){
				_INFO_CFG("Group %d - Invalid channels in Channel BitMap (0x%08X)!\n", i + 1,
					wanif_conf->active_ch & (t1_invalid_channels));
				rc = 2; break;
			}
		}

		//check channels are NOT in use already by another Group
		if(wanif_conf->active_ch & cumulutive_active_ch){
			_INFO_CFG("Group %d - one or more \"T1/E1 channels\" already used by another group!\n",	i + 1);
			rc = 3; break;
		}

		//update cumulative channels for the next iteration
		cumulutive_active_ch |= wanif_conf->active_ch;

		tmp_active_ch = wanif_conf->active_ch;
		if(sdla_fe_cfg->media == WAN_MEDIA_E1 && sdla_fe_cfg->frame != WAN_FR_UNFRAMED){
			//do not count the bit for channel 0! (valid only for Unframed E1)
			tmp_active_ch &= (~0x1);
		}

		if(check_mtu(wanif_conf->mtu, wanif_conf->hdlc_streaming, tmp_active_ch)){
			rc = 4; break;
		}
	}//for()

	INFO_CFG("Driver Configration structure check - %s.\n", (rc == 0 ? "OK":"FAILED"));
	return rc;
}

/*!
  \brief Brief description
 *
 */
int sangoma_port_configurator::print_port_cfg_structure(port_cfg_t *port_cfg)
{
	wandev_conf_t		*wandev_conf = &port_cfg->wandev_conf;
	sdla_fe_cfg_t		*sdla_fe_cfg = &wandev_conf->fe_cfg;
	wanif_conf_t		*wanif_conf = &port_cfg->if_cfg[0];
	unsigned int		i;

	DBG_CFG("%s()\n", __FUNCTION__);

	_INFO_CFG("\n================================================\n");

	INFO_CFG("Card Type\t: %s(%d)\n", SDLA_DECODE_CARDTYPE(wandev_conf->card_type),
		wandev_conf->card_type);/* Sangoma Card type - S514, S518 or AFT.*/

	INFO_CFG("Number of TimeSlot Groups: %d\n", port_cfg->num_of_ifs);

	INFO_CFG("MTU\t\t: %d\n",	wandev_conf->mtu);

	print_sdla_fe_cfg_t_structure(sdla_fe_cfg);

	for(i = 0; i < port_cfg->num_of_ifs; i++){
		_INFO_CFG("\n************************************************\n");
		_INFO_CFG("Configration of Group Number %d:\n", i);
		print_wanif_conf_t_structure(&wanif_conf[i]);
	}
	return 0;
}

/*!
  \brief Brief description
 *
 */
int sangoma_port_configurator::print_sdla_fe_cfg_t_structure(sdla_fe_cfg_t *sdla_fe_cfg)
{
	_INFO_CFG("\n################################################\n");
	INFO_CFG("MEDIA\t\t: %s\n", MEDIA_DECODE(sdla_fe_cfg));
	if(FE_MEDIA(sdla_fe_cfg) == WAN_MEDIA_T1 || FE_MEDIA(sdla_fe_cfg) == WAN_MEDIA_E1){
		INFO_CFG("Line CODE\t: %s\n", LCODE_DECODE(sdla_fe_cfg));
		INFO_CFG("Framing\t\t: %s\n", FRAME_DECODE(sdla_fe_cfg));
		INFO_CFG("Clock Mode\t: %s\n", TECLK_DECODE(sdla_fe_cfg));
		INFO_CFG("Clock Reference port: %d (0 - not used)\n", FE_REFCLK(sdla_fe_cfg));
		INFO_CFG("Signalling Insertion Mode: %s\n", TE1SIG_DECODE(sdla_fe_cfg));

		INFO_CFG("High Impedance Mode: %s\n",
			(FE_HIMPEDANCE_MODE(sdla_fe_cfg) == WANOPT_YES ? "Yes":"No"));
		INFO_CFG("FE_RX_SLEVEL: %d\n", FE_RX_SLEVEL(sdla_fe_cfg));
	}//if()
	INFO_CFG("TDMV LAW\t: %s\n", (FE_TDMV_LAW(sdla_fe_cfg) == WAN_TDMV_MULAW ?"MuLaw":"ALaw"));
	INFO_CFG("TDMV SYNC\t: %d\n", FE_NETWORK_SYNC(sdla_fe_cfg));

	switch(FE_MEDIA(sdla_fe_cfg))
	{
	case WAN_MEDIA_T1:
		INFO_CFG("LBO\t\t: %s\n", LBO_DECODE(sdla_fe_cfg));
		break;
	case WAN_MEDIA_E1:
		break;
	default:
		break;
	}
	return 0;
}


/*!
  \brief Brief port description
 *
 */

int sangoma_port_configurator::print_wanif_conf_t_structure(wanif_conf_t *wanif_conf)
{
	INFO_CFG("Operation Mode\t: %s\n", wanif_conf->usedby);
	INFO_CFG("Timeslot BitMap\t: 0x%08X\n", wanif_conf->active_ch);
	INFO_CFG("Line Mode\t: %s\n",
				(wanif_conf->hdlc_streaming == WANOPT_YES ? "HDLC":"BitStream"));
	INFO_CFG("MTU\\MRU\t\t: %d\n", wanif_conf->mtu);

	return 0;
}


/*!
  \brief Brief port description
 *
 */
int sangoma_port_configurator::set_default_configuration(port_cfg_t *port_cfg)
{
	return set_volatile_configration(port_cfg);
}


/*!
  \brief Brief port description
 *
 */

int sangoma_port_configurator::set_volatile_configration(port_cfg_t *port_cfg)
{
	int rc;

	DBG_CFG("%s()\n", __FUNCTION__);

	//////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
	if(check_port_cfg_structure(port_cfg)){
		ERR_CFG("Error(s) found in 'port_cfg_t' structure!\n");
		return 1;
	}
#endif

	rc = SetPortVolatileConfigurationCommand(port_cfg);
	if(rc){
		//configration failed
		INFO_CFG("%s(): Line: %d: return code: %s (%d)\n", __FUNCTION__, __LINE__, SDLA_DECODE_SANG_STATUS(rc), rc);
		return 2;
	}

	INFO_CFG("%s(): return code: %s (%d)\n", __FUNCTION__,
		SDLA_DECODE_SANG_STATUS(port_cfg->operation_status), port_cfg->operation_status);
	switch(port_cfg->operation_status)
	{
	case SANG_STATUS_DEVICE_BUSY:
		INFO_CFG("Error: open handles exist for '%s_IF\?\?' interfaces!\n",	wanpipe_name_str);
		return port_cfg->operation_status;
	case SANG_STATUS_SUCCESS:
		//OK
		break;
	default:
		//error
		return 5;
	}

	return 0;
}


/*!
  \brief Brief port description
 *
 */

int sangoma_port_configurator::set_t1_tdm_span_voice_api_configration(port_cfg_t *port_cfg, hardware_info_t *hardware_info, int span)
{
    wandev_conf_t    *wandev_conf = &port_cfg->wandev_conf;
    sdla_fe_cfg_t    *sdla_fe_cfg = &wandev_conf->fe_cfg;
    wan_tdmv_conf_t  *tdmv_cfg = &wandev_conf->tdmv_conf;
    wanif_conf_t     *wanif_cfg = &port_cfg->if_cfg[0];

    // Load media parameters in the registry
    FE_MEDIA(sdla_fe_cfg) = WAN_MEDIA_T1;
    FE_LCODE(sdla_fe_cfg) = WAN_LCODE_B8ZS;
    FE_FRAME(sdla_fe_cfg) = WAN_FR_ESF;
	FE_LINENO(sdla_fe_cfg) =  hardware_info->port_number;
    sdla_fe_cfg->cfg.te_cfg.lbo = WAN_T1_0_110;

    port_cfg->num_of_ifs = 1;

    wandev_conf->config_id = WANCONFIG_AFT_TE1;
    wandev_conf->magic = ROUTER_MAGIC;
	
    wandev_conf->mtu = 2048;
	wandev_conf->PCI_slot_no = hardware_info->pci_slot_number;
	wandev_conf->pci_bus_no = hardware_info->pci_bus_number;
    wandev_conf->card_type = WANOPT_AFT; //m_DeviceInfoData.card_model;

	wanif_cfg->magic = ROUTER_MAGIC;
    wanif_cfg->active_ch = 0xFFFFFFFF;
    sprintf(wanif_cfg->usedby,"TDM_SPAN_VOICE_API");
    wanif_cfg->u.aft.idle_flag=0xFF;
    wanif_cfg->mtu = 160;
    wanif_cfg->u.aft.mtu = 160;
	wanif_cfg->u.aft.mru = 160;
	sprintf(wanif_cfg->name,"w%dg1",span);

    if (hardware_info->max_hw_ec_chans) {
        tdmv_cfg->hw_dtmf=1;
		wanif_cfg->hwec.enable = 1;
	}

    tdmv_cfg->span_no = span;

	/* DCHAN Configuration */
    switch(FE_MEDIA(sdla_fe_cfg))
    {
	case WAN_MEDIA_T1:
		tdmv_cfg->dchan = (1<<23);/* channel 24 */
        break;
	default:
		printf("%s(): Error: invalid media type!\n", __FUNCTION__);
		return 1;
    }

	printf("T1: tdmv_cfg->dchan: 0x%X\n", tdmv_cfg->dchan);

	/* Use Master clock in loopback mode or as telco simulator */
#if 0
 	sdla_fe_cfg->cfg.te_cfg.te_clock = WAN_NORMAL_CLK;
#else
	sdla_fe_cfg->cfg.te_cfg.te_clock = WAN_MASTER_CLK;
#endif

	return 0;
}


int sangoma_port_configurator::set_e1_tdm_span_voice_api_configration(port_cfg_t *port_cfg, hardware_info_t *hardware_info, int span)
{
    wandev_conf_t    *wandev_conf = &port_cfg->wandev_conf;
    sdla_fe_cfg_t    *sdla_fe_cfg = &wandev_conf->fe_cfg;
    wan_tdmv_conf_t  *tdmv_cfg = &wandev_conf->tdmv_conf;
    wanif_conf_t     *wanif_cfg = &port_cfg->if_cfg[0];

	// Load media parameters in the registry
    FE_MEDIA(sdla_fe_cfg) = WAN_MEDIA_E1;
    FE_LCODE(sdla_fe_cfg) = WAN_LCODE_HDB3;
    FE_FRAME(sdla_fe_cfg) = WAN_FR_CRC4;
	FE_LINENO(sdla_fe_cfg) = hardware_info->port_number;
    sdla_fe_cfg->cfg.te_cfg.lbo = WAN_E1_120;

    port_cfg->num_of_ifs = 1;

    wandev_conf->config_id = WANCONFIG_AFT_TE1;
    wandev_conf->magic = ROUTER_MAGIC;
	
    wandev_conf->mtu = 2048;
	wandev_conf->PCI_slot_no = hardware_info->pci_slot_number;
	wandev_conf->pci_bus_no = hardware_info->pci_bus_number;
    wandev_conf->card_type = WANOPT_AFT; //m_DeviceInfoData.card_model;

	wanif_cfg->magic = ROUTER_MAGIC;
    wanif_cfg->active_ch = 0xFFFFFFFF;
    sprintf(wanif_cfg->usedby,"TDM_SPAN_VOICE_API");
    wanif_cfg->u.aft.idle_flag=0xFF;
    wanif_cfg->mtu = 160;
    wanif_cfg->u.aft.mtu = 160;
	wanif_cfg->u.aft.mru = 160;
	sprintf(wanif_cfg->name,"w%dg1",span);

    if (hardware_info->max_hw_ec_chans) {
        tdmv_cfg->hw_dtmf=1;
		wanif_cfg->hwec.enable = 1;
	}

    tdmv_cfg->span_no = span;

	/* DCHAN Configuration */
    switch(FE_MEDIA(sdla_fe_cfg))
    {
	case WAN_MEDIA_E1:
		tdmv_cfg->dchan = (1<<15);/* channel 16 */
        break;
	default:
		printf("%s(): Error: invalid media type!\n", __FUNCTION__);
		return 1;
    }
	
	printf("E1: tdmv_cfg->dchan: 0x%X\n", tdmv_cfg->dchan);

    sdla_fe_cfg->cfg.te_cfg.te_clock = WAN_NORMAL_CLK;

	return 0;
}

//////////////////////////////////////////////////////////////////////
#if defined(__WINDOWS__)

/*!
  \brief Brief description
 *
 */
void GetRegistryStringValue(HKEY hkey, LPTSTR szKeyname, OUT LPSTR szValue, OUT DWORD *pdwSize)
{
	//reading twice to set dwTemp to needed value
	RegQueryValueEx(hkey, szKeyname, NULL, NULL, (unsigned char *)szValue, pdwSize);
	RegQueryValueEx(hkey, szKeyname, NULL, NULL, (unsigned char *)szValue, pdwSize);
	DBG_CFG("GetRegistryStringValue(): %s: %s\n", szKeyname, szValue);
}

/*!
  \brief Brief description
 *
 */
void SetRegistryStringValue(HKEY hkey, LPTSTR szKeyname, IN LPSTR szValue)
{
	DWORD	dwSize;

	dwSize = strlen(szValue) + 1;
    RegSetValueEx(hkey, szKeyname, 0, REG_SZ, (unsigned char *)szValue, dwSize);
	DBG_CFG("SetRegistryStringValue(): %s: %s\n", szKeyname, szValue);
}

/*!
  \brief Convert an integer (iValue) to string and write it to registry
 *
 */
void SetRegistryStringValue(HKEY hkey, LPTSTR szKeyname, IN int iValue)
{
	DWORD	dwSize;
	char	szTemp[TMP_BUFFER_LEN];

	sprintf(szTemp, "%u", iValue);

	dwSize = strlen(szTemp) + 1;
    RegSetValueEx(hkey, szKeyname, 0, REG_SZ, (unsigned char *)szTemp, dwSize);
	DBG_CFG("SetRegistryStringValue(): %s: %s\n", szKeyname, szTemp);
}


/*!
  \brief Go through the list of ALL "Sangoma Hardware Abstraction Driver" ports installed on the system.
 *
 * Read Bus/Slot/Port information for a port and copare with what is searched for.
 */
int sangoma_port_configurator::open_port_registry_key(hardware_info_t *hardware_info)
{
	int				i;
	SP_DEVINFO_DATA deid={sizeof(SP_DEVINFO_DATA)};
	HDEVINFO		hdi = SetupDiGetClassDevs((struct _GUID *)&GUID_DEVCLASS_SANGOMA_ADAPTER, NULL,NULL, DIGCF_PRESENT);
	char			DeviceName[TMP_BUFFER_LEN], PCI_Bus[TMP_BUFFER_LEN], PCI_Slot[TMP_BUFFER_LEN], Port_Number[TMP_BUFFER_LEN];
	DWORD			dwTemp;
	HKEY			hKeyTmp = (struct HKEY__ *)INVALID_HANDLE_VALUE;

	hPortRegistryKey = (struct HKEY__ *)INVALID_HANDLE_VALUE;

	//Possible Port Names:
	//Sangoma Hardware Abstraction Driver (Port 1)
	//Sangoma Hardware Abstraction Driver (Port 2)
	//Sangoma Hardware Abstraction Driver (Port 3)
	//Sangoma Hardware Abstraction Driver (Port 4)
	//Sangoma Hardware Abstraction Driver (Port 5)
	//Sangoma Hardware Abstraction Driver (Port 6)
	//Sangoma Hardware Abstraction Driver (Port 7)
	//Sangoma Hardware Abstraction Driver (Port 8)

	//this version will search for all ports:
	sprintf(name,"  Sangoma Hardware Abstraction Driver");

	for (i = 0; SetupDiEnumDeviceInfo(hdi, i, &deid); i++)
	{
		BOOL fSuccess = SetupDiGetDeviceInstanceId(hdi, &deid, (PSTR)szCompInstanceId,MAX_COMP_INSTID, NULL);
		if (!fSuccess){
			continue;
		}

		fSuccess = SetupDiGetDeviceRegistryProperty(hdi, &deid,SPDRP_DEVICEDESC,&dwRegType, (BYTE*)szCompDescription, MAX_COMP_DESC, NULL);
		if (!fSuccess){
			continue;
		}

		if (strncmp(szCompDescription, name, strlen(name)) != 0) {	// Windows can add #2 etc - do NOT consider
			//not a Sangoma device
			continue;
		}

		printf("* %s\n", szCompDescription);

		hKeyTmp = SetupDiOpenDevRegKey(hdi, &deid, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ | KEY_SET_VALUE );
		if(hKeyTmp == INVALID_HANDLE_VALUE){
			printf("Error: Failed to open Registry key!!\n");
			continue;
		}

		PCI_Bus[0] = '\0';
		GetRegistryStringValue(hKeyTmp, "PCI_Bus", PCI_Bus, &dwTemp);

		PCI_Slot[0] = '\0';
		GetRegistryStringValue(hKeyTmp, "PCI_Slot", PCI_Slot, &dwTemp);

		Port_Number[0] = '\0';
		GetRegistryStringValue(hKeyTmp, "Port_Number", Port_Number, &dwTemp);

		if(	atoi(PCI_Bus)		== hardware_info->pci_bus_number	&&
			atoi(PCI_Slot)		== hardware_info->pci_slot_number	&&
			atoi(Port_Number)	== hardware_info->port_number		){

			hPortRegistryKey = hKeyTmp;

			DeviceName[0] = '\0';
			GetRegistryStringValue(hPortRegistryKey, "DeviceName", DeviceName, &dwTemp);

			printf("Found Port's Registry key: DeviceName: %s, PCI_Bus: %s, PCI_Slot: %s, Port_Number: %s\n",
				DeviceName, PCI_Bus, PCI_Slot, Port_Number);
			break;
		}//if()
	}//for()

	SetupDiDestroyDeviceInfoList(hdi);

	if(hPortRegistryKey == (struct HKEY__ *)INVALID_HANDLE_VALUE){
		return SANG_STATUS_REGISTRY_ERROR;
	}else{
		return SANG_STATUS_SUCCESS;
	}
}


/*!
  \brief Brief description
 *
 */
int sangoma_port_configurator::set_registry_configration(port_cfg_t *port_cfg)
{
	wandev_conf_t		*wandev_conf = &port_cfg->wandev_conf;
	sdla_fe_cfg_t		*sdla_fe_cfg = &wandev_conf->fe_cfg;
	int rc;

	DBG_CFG("%s()\n", __FUNCTION__);

	if(hPortRegistryKey == (struct HKEY__ *)INVALID_HANDLE_VALUE){
		ERR_CFG("Error: registry key is CLOSED for port!\n");
		return 1;
	}

	if((rc = stop_port())){
		//failed to stop port for re-configuration
		INFO_CFG("%s(): return code: %s (%d)\n", __FUNCTION__, SDLA_DECODE_SANG_STATUS(rc), rc);
		return 2;
	}


	// set common default values: set 1 group, HDLC, MTU 2048, API.
	// One group
	SetRegistryStringValue(hPortRegistryKey, "aft_number_of_logic_channels", 1);
	// Line mode is HDLC
	SetRegistryStringValue(hPortRegistryKey, "aft_logic_channel_0_line_mode", MODE_OPTION_HDLC);
	// MTU
	SetRegistryStringValue(hPortRegistryKey, "aft_logic_channel_0_mtu", 2048);
	// Operational mode
	SetRegistryStringValue(hPortRegistryKey, "aft_logic_channel_0_operational_mode", SDLA_DECODE_USEDBY_FIELD(API));

	// set Media specific default values.
	if(sdla_fe_cfg->media == WAN_MEDIA_T1){

		SetRegistryStringValue(hPortRegistryKey, "Media", WAN_MEDIA_T1);
		SetRegistryStringValue(hPortRegistryKey, "LDecoding", WAN_LCODE_B8ZS);
		SetRegistryStringValue(hPortRegistryKey, "Framing", WAN_FR_ESF);
		SetRegistryStringValue(hPortRegistryKey, "LBO", WAN_T1_LBO_0_DB);
		SetRegistryStringValue(hPortRegistryKey, "aft_logic_channel_0_active_ch", "1-24");
		SetRegistryStringValue(hPortRegistryKey, "ClkMode", WAN_NORMAL_CLK);
		SetRegistryStringValue(hPortRegistryKey, "HighImpedanceMode", WANOPT_NO);
		SetRegistryStringValue(hPortRegistryKey, "TE_RX_SLEVEL", WAN_TE1_RX_SLEVEL_12_DB);

	}else if(sdla_fe_cfg->media == WAN_MEDIA_E1){

		SetRegistryStringValue(hPortRegistryKey, "Media", WAN_MEDIA_E1);
		SetRegistryStringValue(hPortRegistryKey, "LDecoding", WAN_LCODE_HDB3);
		SetRegistryStringValue(hPortRegistryKey, "Framing", WAN_FR_NCRC4);
		SetRegistryStringValue(hPortRegistryKey, "LBO", WAN_E1_120);
		SetRegistryStringValue(hPortRegistryKey, "E1Signalling", WAN_TE1_SIG_CCS);
		SetRegistryStringValue(hPortRegistryKey, "aft_logic_channel_0_active_ch", "1-31");
		SetRegistryStringValue(hPortRegistryKey, "ClkMode", WAN_NORMAL_CLK);
		SetRegistryStringValue(hPortRegistryKey, "HighImpedanceMode", WANOPT_NO);
		SetRegistryStringValue(hPortRegistryKey, "TE_RX_SLEVEL", WAN_TE1_RX_SLEVEL_12_DB);

	}else{
		//example shows only how to change between T1 and E1
		ERR_CFG("%s(): Error: invalid Media Type %d!\n", __FUNCTION__, sdla_fe_cfg->media);
		return 3;
	}

	//After all configuration was written to the registry, start the driver:
	if((rc = start_port())){
		//failed to start port after re-configuration
		INFO_CFG("%s(): return code: %s (%d)\n", __FUNCTION__, SDLA_DECODE_SANG_STATUS(rc), rc);
		return 1;
	}

	INFO_CFG("%s(): return code: %s (%d)\n", __FUNCTION__, SDLA_DECODE_SANG_STATUS(rc), rc);
	return 0;
}
#endif//__WINDOWS__
