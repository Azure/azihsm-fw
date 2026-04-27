// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "host_cmd_handler_overlake.h"
#include "common/type_cast.h"
#include "common/unused.h"


int host_cmd_handler_overlake_get_next_host_verification (const struct host_cmd_interface *cmd,
	enum host_processor_reset_actions *action)
{
	const struct host_cmd_handler_overlake *overlake =
		TO_DERIVED_TYPE (cmd, const struct host_cmd_handler_overlake, base);
	int status;

	if ((cmd == NULL) || (action == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	status = overlake->host->get_next_reset_verification_actions (overlake->host);
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	*action = status;

	return 0;
}

int host_cmd_handler_overlake_get_flash_configuration (const struct host_cmd_interface *cmd,
	spi_filter_flash_mode *mode, spi_filter_cs *current_ro, spi_filter_cs *next_ro,
	enum host_read_only_activation *apply_next_ro)
{
	if ((cmd == NULL) || (mode == NULL) || (current_ro == NULL) || (next_ro == NULL) ||
		(apply_next_ro == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	return HOST_PROCESSOR_FLASH_CONFIG_UNSUPPORTED;
}

int host_cmd_handler_overlake_set_flash_configuration (const struct host_cmd_interface *cmd,
	int8_t current_ro, int8_t next_ro, int8_t apply_next_ro)
{
	if (cmd == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	return HOST_PROCESSOR_FLASH_CONFIG_UNSUPPORTED;
}

int host_cmd_handler_overlake_get_status (const struct host_cmd_interface *cmd)
{
	if (cmd == NULL) {
		return HOST_CMD_STATUS_UNKNOWN;
	}

	return HOST_CMD_STATUS_NONE_STARTED;
}

/**
 * Initialize a handler for Overlake host requests.
 *
 * @param handler The handler to initialize.
 * @param host The target host processor instance for the commands.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int host_cmd_handler_overlake_init (struct host_cmd_handler_overlake *handler,
	const struct host_processor *host)
{
	if ((handler == NULL) || (host == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (*handler));

	handler->base.get_next_host_verification = host_cmd_handler_overlake_get_next_host_verification;
	handler->base.get_flash_configuration = host_cmd_handler_overlake_get_flash_configuration;
	handler->base.set_flash_configuration = host_cmd_handler_overlake_set_flash_configuration;
	handler->base.get_status = host_cmd_handler_overlake_get_status;

	handler->host = host;

	return 0;
}

/**
 * Release the resources used by Overlake host command handling.
 *
 * @param handler The handler to release.
 */
void host_cmd_handler_overlake_release (const struct host_cmd_handler_overlake *handler)
{
	UNUSED (handler);
}
