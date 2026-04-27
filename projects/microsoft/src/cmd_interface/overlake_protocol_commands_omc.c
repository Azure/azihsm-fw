// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "cmd_interface_system_omc.h"
#include "attestation/attestation_responder.h"
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cerberus_protocol_master_commands.h"
#include "cmd_interface/cerberus_protocol_optional_commands.h"
#include "cmd_interface/cmd_logging.h"
#include "cmd_interface/overlake_protocol.h"
#include "cmd_interface/overlake_protocol_commands_omc.h"
#include "common/common_math.h"
#include "crypto/ecdsa.h"
#include "host_fw/host_fw_cmd_interface.h"
#include "host_fw/omc_flash_manager.h"
#include "host_fw/omc_host_id.h"
#include "host_fw/overlake_board_id.h"
#include "host_fw/overlake_control.h"


/**
 * Process Overlake host FW update init request.
 *
 * @param host_fw_cmd List of host firmware command interfaces for all available ports.
 * @param num_ports Number of available ports.
 * @param request FW update request to process.
 * @param prioritize_update Flag to indicate the priority of update.
 * @param max_channel_id Maximum channel ID that is to be supported.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_host_fw_init_omc (const struct host_fw_cmd_interface *host_fw_cmd[],
	uint8_t num_ports, bool prioritize_update, uint8_t max_channel_id,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_prepare_fw_update_request *rq =
		(struct overlake_protocol_prepare_fw_update_request*) request->data;
	size_t max_request_length = sizeof (struct overlake_protocol_prepare_fw_update_request);
	uint8_t skip_shmoo_erase;

	if ((request->length > max_request_length) ||
		(request->length < (max_request_length - 1))) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	if (request->channel_id > max_channel_id) {
		return CMD_HANDLER_UNSUPPORTED_CHANNEL;
	}

	if (rq->port_id >= num_ports) {
		return CMD_HANDLER_OUT_OF_RANGE;
	}

	if (host_fw_cmd[rq->port_id] == NULL) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	skip_shmoo_erase = (request->length == max_request_length - 1) ? 0 : rq->ctrl_flag;

	request->length = 0;

	return host_fw_cmd[rq->port_id]->prepare_update (host_fw_cmd[rq->port_id], rq->size,
		prioritize_update, skip_shmoo_erase);
}

/**
 * Process Overlake host FW update request.
 *
 * @param host_fw_cmd List of host firmware command interfaces for all available ports.
 * @param num_ports Number of available ports.
 * @param request FW update request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_host_fw_update_omc (const struct host_fw_cmd_interface *host_fw_cmd[],
	uint8_t num_ports, struct cmd_interface_msg *request)
{
	struct overlake_protocol_fw_update_request *rq =
		(struct overlake_protocol_fw_update_request*) request->data;
	int status;

	if (request->length < sizeof (struct overlake_protocol_fw_update_request)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	if (rq->port_id >= num_ports) {
		return CMD_HANDLER_OUT_OF_RANGE;
	}

	if (host_fw_cmd[rq->port_id] == NULL) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	status = host_fw_cmd[rq->port_id]->write_update (host_fw_cmd[rq->port_id], &rq->payload,
		OVERLAKE_PROTOCOL_SOC_FW_UPDATE_LENGTH (request));

	request->length = 0;

	return status;
}

/**
 * Process Overlake host FW update status request.
 *
 * @param host_fw_cmd List of host firmware command interfaces for all available ports.
 * @param num_ports Number of available ports.
 * @param request FW update request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_get_host_fw_update_status_omc (
	const struct host_fw_cmd_interface *host_fw_cmd[], uint8_t num_ports,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_get_fw_update_status *rq =
		(struct overlake_protocol_get_fw_update_status*) request->data;
	struct overlake_protocol_get_fw_update_status_response *rsp =
		(struct overlake_protocol_get_fw_update_status_response*) request->data;
	uint8_t port_num;

	if (request->length != sizeof (struct overlake_protocol_get_fw_update_status)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	if (rq->port_id >= num_ports) {
		return CMD_HANDLER_OUT_OF_RANGE;
	}

	if (host_fw_cmd[rq->port_id] == NULL) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	port_num = rq->port_id;

	rsp->update_status = host_fw_cmd[port_num]->get_status (host_fw_cmd[port_num]);

	rsp->remaining_len = host_fw_cmd[port_num]->get_update_remaining (host_fw_cmd[port_num]);

	cmd_interface_msg_set_message_payload_length (request,
		sizeof (struct overlake_protocol_get_fw_update_status_response));

	return 0;
}

/**
 * Process a request to hard reset the SoC/ Cyclone V FPGA.
 *
 * @param flash Overlake SoC flash manager to utilize.
 * @param flash_fpga Overlake FPGA flash manager to utilize
 * @param soc_control SoC control instance to utilize.
 * @param request SoC reset request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_soc_reset_omc (struct omc_flash_manager *flash,
	struct overlake_control *soc_control, struct cmd_interface_msg *request)
{
	struct overlake_protocol_soc_reset *rq = (struct overlake_protocol_soc_reset*) request->data;
	int status;

	if (request->length != sizeof (struct overlake_protocol_soc_reset)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	if ((rq->id != OMC_RESET_PORT_SOC) && (rq->id != OMC_RESET_PORT_HOLD_FLASH) &&
		(rq->id != OMC_RESET_PORT_RELEASE_FLASH) && (rq->id != OMC_RESET_PORT_TAKE_FLASH)) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	/* Only soc reset issued OOB is allowed to cancel SoC fw updates in progress. */
	if (request->channel_id == CMD_INTERFACE_OMC_CHANNEL_I2C_BMC) {
		status = flash->cancel_active_updates (flash);
		if (status != 0) {
			return status;
		}
	}

	status = soc_control->soc_reset (soc_control, rq->id);
	if (status != 0) {
		return status;
	}

	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_CMD_INTERFACE,
		CMD_LOGGING_SOC_RESET_TRIGGERED, status, rq->id);

	request->length = 0;

	return 0;
}
