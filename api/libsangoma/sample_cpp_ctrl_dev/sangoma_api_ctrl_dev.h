#ifndef SANGOMA_API_CTRL_DEV_H
#define SANGOMA_API_CTRL_DEV_H

#include "sangoma_interface.h"

class sangoma_api_ctrl_dev :
	public sangoma_interface
{
public:
	
	sangoma_api_ctrl_dev(void);
	~sangoma_api_ctrl_dev(void);
	int init(callback_functions_t *callback_functions_ptr);
};

#endif