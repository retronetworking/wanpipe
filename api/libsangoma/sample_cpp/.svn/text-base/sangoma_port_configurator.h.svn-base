//////////////////////////////////////////////////////////////////////
// sangoma_port_configurator.h: interface for the sangoma_port_configurator class.
//
// Author	:	David Rokhvarg	<davidr@sangoma.com>
//////////////////////////////////////////////////////////////////////

#if !defined(_SANGOMA_PORT_CONFIGURATOR_H)
#define _SANGOMA_PORT_CONFIGURATOR_H

#include "sangoma_port.h"

#define MAX_COMP_INSTID 2096
#define MAX_COMP_DESC   2096
#define MAX_FRIENDLY    2096	
#define TMP_BUFFER_LEN	200

#define T1_ALL_CHANNELS_BITMAP 0xFFFFFF /* channels 1-24 */

/*!
  \brief 
 *
 */

class sangoma_port_configurator : public sangoma_port
{

	TCHAR	name[MAX_FRIENDLY];
	char    szCompInstanceId[MAX_COMP_INSTID];
	TCHAR   szCompDescription[MAX_COMP_DESC];
	TCHAR   szFriendlyName[MAX_FRIENDLY];
	DWORD   dwRegType;

#ifdef __WINDOWS__
	HKEY	hPortRegistryKey;
#endif

	port_cfg_t	port_cfg;

	//SET new configuration of the API driver
	int SetPortVolatileConfigurationCommand(port_cfg_t *port_cfg);


public:
	sangoma_port_configurator();
	virtual ~sangoma_port_configurator();
	
	int open_port_registry_key(hardware_info_t *hardware_info);

	int get_configration(port_cfg_t *port_cfg);

	int set_default_configuration(port_cfg_t *port_cfg);

	//Function to check correctness of 'port_cfg_t' structure.
	int check_port_cfg_structure(port_cfg_t *port_cfg);

	//Function to print contents of 'port_cfg_t' structure.
	int print_port_cfg_structure(port_cfg_t *port_cfg);

	//Function to print contents of 'sdla_fe_cfg_t' structure.
	int print_sdla_fe_cfg_t_structure(sdla_fe_cfg_t *sdla_fe_cfg);

	//Function to print contents of 'wanif_conf_t' structure.
	int print_wanif_conf_t_structure(wanif_conf_t *wanif_conf);

	int set_volatile_configration(port_cfg_t *port_cfg);

	int initialize_t1_tdm_span_voice_api_configration_structure(port_cfg_t *port_cfg, hardware_info_t *hardware_info, int span);
	int initialize_e1_tdm_span_voice_api_configration_structure(port_cfg_t *port_cfg, hardware_info_t *hardware_info, int span);

	int write_configration_on_persistent_storage(port_cfg_t *port_cfg, hardware_info_t *hardware_info, int span);
};

#endif // !defined(_SANGOMA_PORT_CONFIGURATOR_H)
