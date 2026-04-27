// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OMC_BACKGROUND_H_
#define OMC_BACKGROUND_H_

#include "cmd_interface/cmd_interface.h"


/**
 * Extension for running OMC commands in the background.
 */
struct omc_background {
	/**
	 * Process a request to erase the OMC flash.
	 *
	 * @param background The background context for executing the operation.
	 *
	 * @return 0 if the request was successful or an error code.
	 */
	int (*soc_flash_erase) (struct omc_background *background);

	/**
	 * Get the status of the last OMC flash erase operation.
	 *
	 * @param background The background context for executing the operation.
	 *
	 * @return 0 if the last flash erase operation was successful, 1 if it is still in progress, or an error code.
	 */
	int (*get_soc_flash_erase_status) (struct omc_background *background);

	/**
	 * Process a request to erase the OMC flash image partitions (active and backup).
	 *
	 * @param background The background context for executing the operation.
	 *
	 * @return 0 if the request was successful or an error code.
	 */
	int (*soc_image_partitions_erase) (struct omc_background *background);

	/**
	 * Get the status of the last OMC flash image partitions erase operation.
	 *
	 * @param background The background context for executing the operation.
	 *
	 * @return 0 if the last flash image partitions erase operation was successful, 1 if it is still in progress, or an error code.
	 */
	int (*get_soc_image_partitions_erase_status) (struct omc_background *background);
};


#endif	/* OMC_BACKGROUND_H_ */
