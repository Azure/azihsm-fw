// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_msft_base.h"
#include "cmd_interface_msft_rot.h"
#include "rot_commands.h"
#include "common/unused.h"


int cmd_interface_msft_rot_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_msft_rot *msft = (const struct cmd_interface_msft_rot*) intf;
	const struct msft_mctp_protocol_header *header;
	int status;

	if ((msft == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	/* There is no need to check the header length or validate contents in this handler.  That would
	 * have been already done by an earlier layer of the stack before passing the message here. */
	header = (const struct msft_mctp_protocol_header*) request->payload;

	switch (header->command) {
		case ROT_CMD_GET_ROT_CAPABILITIES: {
			uint8_t feature_flags[ROT_CAPABILITIES_SIZE] = {0};

			if (msft->reboot != NULL) {
				rot_set_rot_capabilities_feature (feature_flags, ROT_CAPABILITIES_SIZE,
					ROT_FEATURE_ROT_RESET);
			}

			if (msft->unlock != NULL) {
				rot_set_rot_capabilities_feature (feature_flags, ROT_CAPABILITIES_SIZE,
					ROT_FEATURE_DEBUG_UNLOCK);
			}

			if (msft->rtc != NULL) {
				rot_set_rot_capabilities_feature (feature_flags, ROT_CAPABILITIES_SIZE,
					ROT_FEATURE_TIME);
			}

			if ((msft->intrusion != NULL) && (msft->notifier != NULL)) {
				rot_set_rot_capabilities_feature (feature_flags, ROT_CAPABILITIES_SIZE,
					ROT_FEATURE_INTRUSION_DETECTION);
			}

			if (msft->cmd_log != NULL) {
				rot_set_rot_capabilities_feature (feature_flags, ROT_CAPABILITIES_SIZE,
					ROT_FEATURE_LOG);
			}

			status = rot_get_rot_capabilities (feature_flags, ROT_CAPABILITIES_SIZE, request);
			break;
		}

		case ROT_CMD_RESET_ROT:
			if (msft->reboot == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = rot_reset (msft->reboot, request);
			break;

		case ROT_CMD_GET_TENANCY_GRANT_TOKEN:
			status = CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			break;

		case ROT_CMD_GET_UNLOCK_TOKEN:
			if (msft->unlock == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = rot_get_unlock_token (msft->unlock, request);
			break;

		case ROT_CMD_APPLY_UNLOCK_POLICY:
			if (msft->unlock == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = rot_apply_unlock_policy (msft->unlock, msft->reboot, request);
			break;

		case ROT_CMD_CLEAR_UNLOCK_POLICY:
			if (msft->unlock == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = rot_clear_unlock_policy (msft->unlock, msft->reboot, request);
			break;

		case ROT_CMD_GET_CRASH_DUMP_COUNT:
		case ROT_CMD_GET_CRASH_DUMP_INFO:
		case ROT_CMD_READ_CRASH_DUMP:
		case ROT_CMD_CLEAR_CRASH_DUMP:
			status = CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			break;

		case ROT_CMD_GET_TIME:
			if (msft->rtc == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = rot_get_time (msft->rtc, request);
			break;

		case ROT_CMD_SET_TIME:
			if (msft->rtc == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = rot_set_time (msft->rtc, request);
			break;

		case ROT_CMD_GET_INTRUSION_DETECTION:
			if ((msft->intrusion == NULL) || (msft->notifier == NULL)) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = rot_get_intrusion_detection (msft->intrusion, msft->notifier, request);
			break;

		case ROT_CMD_GET_INTRUSION_COUNT:
			if (msft->intrusion == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = rot_get_intrusion_count (msft->intrusion, request);
			break;

		case ROT_CMD_INTRUSION_EVENT:
		case ROT_CMD_WARM_RESET_EVENT_CTRL:
			status = CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			break;

		case ROT_CMD_SEND_LOG_COMMAND:
		case ROT_CMD_READ_LOG_COMMAND:
			if (msft->cmd_log == NULL) {
				return CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			}

			status = msft->cmd_log->process_request (msft->cmd_log, request);
			break;

		default:
			status = CMD_HANDLER_MSFT_UNKNOWN_COMMAND;
	}

	return status;
}

/**
 * Initialize a command handler for MSFT RoT command set requests.
 *
 * @param intf The command handler to initialize.
 * @param unlock The manager for device unlock requests.  This can be null if the device does not
 * support unlock operations.
 * @param rtc The real time clock for time requests.  This can be null if the device does not
 * support time operations.
 * @param reboot The background command handler for triggering device resets.  This can be null if
 * the device does not support commands that cause a device reset.
 * @param intrusion The intrusion state for intrusion requests. This can be null if the device does
 * not support intrusion operations.
 * @param notifier The MCTP notifier interface for intrusion event registration requests. This can
 * be null if the device does not support intrusion event registration operations.
 * @param cmd_log The RoT log interface for log command requests. This can be null if the device
 * does not support log operations.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_msft_rot_init (struct cmd_interface_msft_rot *intf,
	const struct secure_device_unlock *unlock, const struct real_time_clock *rtc,
	const struct cmd_background *reboot, const struct intrusion_state *intrusion,
	const struct mctp_notifier_interface *notifier,	const struct cmd_interface *cmd_log)
{
	if (intf == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_msft_rot));

	intf->base.process_request = cmd_interface_msft_rot_process_request;

	intf->unlock = unlock;
	intf->rtc = rtc;
	intf->reboot = reboot;
	intf->intrusion = intrusion;
	intf->notifier = notifier;
	intf->cmd_log = cmd_log;

	return 0;
}

/**
 * Release the resources used by a MSFT RoT command set handler.
 *
 * @param intf The command handler to release.
 */
void cmd_interface_msft_rot_release (const struct cmd_interface_msft_rot *intf)
{
	UNUSED (intf);
}
