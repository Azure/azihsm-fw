// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_msft_base.h"
#include "msft_base_commands.h"
#include "common/unused.h"


int cmd_interface_msft_base_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_msft_base *msft = (const struct cmd_interface_msft_base*) intf;
	const struct msft_mctp_protocol_header *header;
	int status;

	if ((msft == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	/* There is no need to check the header length or validate contents in this handler.  That would
	 * have been already done by an earlier layer of the stack before passing the message here. */
	header = (const struct msft_mctp_protocol_header*) request->payload;

	switch (header->command) {
		case MSFT_BASE_CMD_CMD_SET_SUPPORT:
			status = msft_base_command_set_support (msft->cmd_sets, msft->set_count, request);
			break;

		case MSFT_BASE_CMD_CAPABILITIES_NEGOTIATION:
			status = msft_base_capabilities_negotiation (msft->device_mgr,
				((msft->temp_sensors != NULL) ? MSFT_BASE_FEATURES_0_TEMP_SENSOR : 0) |
				((msft->hb_notifier != NULL) ? MSFT_BASE_FEATURES_0_HEARTBEAT : 0), request);
			break;

		case MSFT_BASE_CMD_GET_TEMPERATURE:
			if (msft->temp_sensors == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = msft_base_get_temperature (msft->temp_sensors, request);
			break;

		case MSFT_BASE_CMD_HEARTBEAT_CTRL:
			if (msft->hb_notifier == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = msft_base_heartbeat_control (msft->hb_notifier, request);
			break;

		case MSFT_BASE_CMD_HEARTBEAT:
			status = CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			break;

		default:
			status = CMD_HANDLER_MSFT_UNKNOWN_COMMAND;
	}

	return status;
}

/**
 * Initialize a command handler for MSFT Base command set requests.
 *
 * @param intf The command handler to initialize.
 * @param cmd_sets The list of all command sets supported by the device.
 * @param set_count Number of supported command sets in the list.
 * @param device_mgr The manager for information about external devices.
 * @param cluster The manager for temperature sensors in the device.  This can be null if the device
 * does not support reading temperature sensors.
 * @param hb_notifier The instance of MCTP notifier used to send heartbeat notifications.
 * This can be null if the device doesn't support heartbeat notifications.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_msft_base_init (struct cmd_interface_msft_base *intf,
	const struct msft_base_supported_command_set *cmd_sets, size_t set_count,
	struct device_manager *device_mgr, const struct temperature_sensor_cluster *cluster,
	const struct mctp_notifier_interface *hb_notifier)
{
	if ((intf == NULL) || (cmd_sets == NULL) || (set_count == 0) || (device_mgr == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_msft_base));

	intf->base.process_request = cmd_interface_msft_base_process_request;

	intf->cmd_sets = cmd_sets;
	intf->set_count = set_count;
	intf->device_mgr = device_mgr;
	intf->temp_sensors = cluster;
	intf->hb_notifier = hb_notifier;

	return 0;
}

/**
 * Release the resources used by a MSFT Base command set handler.
 *
 * @param intf The command handler to release.
 */
void cmd_interface_msft_base_release (const struct cmd_interface_msft_base *intf)
{
	UNUSED (intf);
}
