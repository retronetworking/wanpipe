
#include "sangoma_api_ctrl_dev.h"

sangoma_api_ctrl_dev::sangoma_api_ctrl_dev():sangoma_interface(0, 0)
{
	IFACE_FUNC();

	//Initialize Device Name (for debugging only).
	_snprintf(device_name, DEV_NAME_LEN, WP_CTRL_DEV_NAME);

	INFO_IFACE("Using Device Name: %s\n", device_name);
}

sangoma_api_ctrl_dev::~sangoma_api_ctrl_dev()
{
	IFACE_FUNC();
}

int sangoma_api_ctrl_dev::init(callback_functions_t *callback_functions_ptr)
{
	IFACE_FUNC();

	memcpy(&callback_functions, callback_functions_ptr, sizeof(callback_functions_t));

	////////////////////////////////////////////////////////////////////////////
	//open handle for reading and writing data, for events reception and other commands
	sng_fd_t tmp_dev_fd = sangoma_open_api_ctrl();
    if (tmp_dev_fd == INVALID_HANDLE_VALUE){
		ERR_IFACE("Unable to open %s for Ctrl Dev!\n", device_name);
		return 1;
	}

	if(sangoma_init_wait_obj(
			&sangoma_wait_obj,
			tmp_dev_fd,
			WanpipeNumber,
			InterfaceNumber,
			POLLPRI,
			SANGOMA_WAIT_OBJ)){
		ERR_IFACE("Failed to initialize 'sangoma_wait_object' for %s\n", device_name);
		return 1;
	}

	return 0;
}
