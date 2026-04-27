// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_CMD_HANDLER_OVERLAKE_STATIC_H_
#define HOST_CMD_HANDLER_OVERLAKE_STATIC_H_

#include "host_cmd_handler_overlake.h"


/* Internal functions declared to allow for static initialization. */
int host_cmd_handler_overlake_get_next_host_verification (const struct host_cmd_interface *cmd,
	enum host_processor_reset_actions *action);
int host_cmd_handler_overlake_get_flash_configuration (const struct host_cmd_interface *cmd,
	spi_filter_flash_mode *mode, spi_filter_cs *current_ro, spi_filter_cs *next_ro,
	enum host_read_only_activation *apply_next_ro);
int host_cmd_handler_overlake_set_flash_configuration (const struct host_cmd_interface *cmd,
	int8_t current_ro, int8_t next_ro, int8_t apply_next_ro);
int host_cmd_handler_overlake_get_status (const struct host_cmd_interface *cmd);


/**
 * Constant initializer for the Host command handler interface APIs.
 */
#define	HOST_CMD_HANDLER_OVERLAKE_API_INIT  { \
		.get_next_host_verification = host_cmd_handler_overlake_get_next_host_verification, \
		.get_flash_configuration = host_cmd_handler_overlake_get_flash_configuration, \
		.set_flash_configuration = host_cmd_handler_overlake_set_flash_configuration, \
		.get_status = host_cmd_handler_overlake_get_status, \
	}


/**
 * Initialize a static instance of a handler for Overlake host requests.  This can be a constant
 * instance.
 *
 * There is no validation done on the arguments.
 *
 * @param host_ptr The target host processor instance for the commands.
 */
#define	host_cmd_handler_overlake_static_init(host_ptr) { \
		.base = HOST_CMD_HANDLER_OVERLAKE_API_INIT, \
		.host = host_ptr, \
	}


#endif	/* HOST_CMD_HANDLER_OVERLAKE_STATIC_H_ */
