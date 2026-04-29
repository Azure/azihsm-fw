// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "msft_base_commands.h"
#include "msft_protocol_logging.h"
#include "common/buffer_util.h"


/**
 * Populate the message payload with a MSFT error status response.  The completion and error codes
 * will be set accordingly.  No error data will be added.
 *
 * @param message The response message to populate with error details.  If this is null, nothing
 * will be done.
 * @param completion_code The completion code to assign to the response message.
 * @param error_code The detailed error code to assign to the status payload in the response.
 */
void msft_base_build_error_response (struct cmd_interface_msg *message, uint8_t completion_code,
	uint32_t error_code)
{
	struct msft_base_status_response *resp;

	if (message == NULL) {
		return;
	}

	resp = (struct msft_base_status_response*) message->payload;

	resp->header.completion_code = completion_code;
	buffer_unaligned_write32 (&resp->error_code, error_code);
	buffer_unaligned_write16 (&resp->data_length, 0);

	cmd_interface_msg_set_message_payload_length (message, sizeof (*resp));

	debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, MSFT_LOGGING_COMPONENT_MVDP,
		MSFT_PROTOCOL_LOGGING_ERROR,
		(resp->header.command_set << 16) | (resp->header.command << 8) | completion_code,
		error_code);
}

/**
 * Populate the message payload with a MSFT error status response, including additional error data.
 * The completion and error codes will be set accordingly.
 *
 * If no error data is provided or the data is too big to fit into the message, no error data will
 * be added to the response.
 *
 * @param message The response message to populate with error details.  If this is null, nothing
 * will be done.
 * @param completion_code The completion code to assign to the response message.
 * @param error_code The detailed error code to assign to the status payload in the response.
 * @param error_data Optional error data to add to the response.
 * @param length Length of the additional error data.
 */
void msft_base_build_error_response_with_data (struct cmd_interface_msg *message,
	uint8_t completion_code, uint32_t error_code, const uint8_t *error_data, size_t length)
{
	struct msft_base_status_response *resp;
	size_t max_data;

	if (message == NULL) {
		return;
	}

	msft_base_build_error_response (message, completion_code, error_code);

	resp = (struct msft_base_status_response*) message->payload;

	if ((error_data != NULL) && (length != 0)) {
		/* Check the amount of response space left before adding the error data. */
		max_data = cmd_interface_msg_get_max_response (message);
		if (max_data > message->payload_length) {
			max_data -= message->payload_length;
		}
		else {
			max_data = 0;
		}

		if (length <= max_data) {
			buffer_unaligned_write16 (&resp->data_length, length);
			cmd_interface_msg_add_payload_data (message, error_data, length);
		}
		else {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, MSFT_LOGGING_COMPONENT_MVDP,
				MSFT_PROTOCOL_LOGGING_SKIP_ERROR_DATA, length, max_data);
		}
	}
}

/**
 * Initializes a list entry for a supported MSFT protocol command set.
 *
 * @param entry The list entry to initialize.
 * @param set_id The command set identifier to associate with this entry.
 * @param protocol_versions List of protocol versions supported for the command set.
 * @param version_count The number of supported protocol versions.
 *
 * @return 0 if successful, else an error code.
 */
int msft_base_supported_command_set_init (struct msft_base_supported_command_set *entry,
	uint8_t set_id, const uint16_t *protocol_versions, size_t version_count)
{
	if ((entry == NULL) || (protocol_versions == NULL) || (version_count == 0)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (entry, 0, sizeof (*entry));

	entry->set_id = set_id;
	entry->versions = protocol_versions;
	entry->version_count = version_count;

	return 0;
}

/**
 * Process a request for MSFT command sets supported by the device and generate a response with a
 * list of supported command sets.
 *
 * @param cmd_sets List of command sets supported by the device.
 * @param count Number of supported command sets in the list.
 * @param request The message containing the request.  This will be updated with an appropriate
 * response.
 *
 * @return 0 if a response was created successfully or an error code.
 */
int msft_base_command_set_support (const struct msft_base_supported_command_set *cmd_sets,
	size_t count, struct cmd_interface_msg *request)
{
	const struct msft_base_cmd_set_support_request *req;
	struct msft_base_cmd_set_support_response *resp;
	struct msft_base_cmd_set_support_entry *entry;
	size_t total_length;
	size_t entry_length;
	size_t remaining;

	if ((cmd_sets == NULL) || (count == 0) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct msft_base_cmd_set_support_request*) request->payload;

	/* This check works while only protocol v0 is supported.  If the protocol version ever changes,
	 * this check will need to change since v0 must always be supported for this command. */
	if (req->header.protocol_version != MSFT_BASE_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct msft_base_cmd_set_support_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	if (req->list_entry_start >= count) {
		return CMD_HANDLER_MSFT_UNSUPPORTED_INDEX;
	}

	remaining = cmd_interface_msg_get_max_response (request);
	if (remaining < (sizeof (*resp) - sizeof (*entry))) {
		/* There needs to at least be enough room for the entry list header, but this represents
		 * misuse of the message structure rather than an issue with the request. */
		return CMD_HANDLER_MSFT_BAD_MAX_RESPONSE;
	}

	resp = (struct msft_base_cmd_set_support_response*) request->payload;

	resp->next_list_entry = req->list_entry_start;
	resp->entry_count = 0;

	entry = &resp->entry;
	total_length = 0;
	remaining -= (sizeof (*resp) - sizeof (*entry));

	while ((resp->next_list_entry < count) && (remaining != 0)) {
		size_t i = resp->next_list_entry;
		size_t j;

		entry_length = msft_base_cmd_set_support_get_entry_length (cmd_sets[i].version_count);
		if (entry_length <= remaining) {
			entry->command_set = cmd_sets[i].set_id;
			entry->version_count = cmd_sets[i].version_count;

			for (j = 0; j < cmd_sets[i].version_count; j++) {
				buffer_unaligned_write16 (&(&entry->version)[j], cmd_sets[i].versions[j]);
			}

			/* Update response to indicate another supported command set has been reported. */
			resp->next_list_entry++;
			resp->entry_count++;

			entry = msft_base_cmd_set_support_get_next_entry (entry);
			total_length += entry_length;
			remaining -= entry_length;
		}
		else {
			remaining = 0;
		}
	}

	/* If all the supported command sets are in the response, mark the next entry appropriately. */
	if (resp->next_list_entry >= count) {
		resp->next_list_entry = MSFT_BASE_CMD_SET_SUPPORT_NO_MORE_ENTRIES;
	}

	cmd_interface_msg_set_message_payload_length (request,
		msft_base_cmd_set_support_response_length (total_length));

	resp->header.completion_code = 0;

	return 0;
}

/**
 * Process a capabilities negotiation request and generate a response with device capabilities.
 *
 * @param device_mgr The manager of known devices.
 * @param feature_flags  Bitmask of features that are supported by the device.  These flags use the
 * values defined in {@link enum msft_base_features}.
 * @param request The message containing the request.  This will be updated with an appropriate
 * response.
 *
 * @return 0 if a response was created successfully or an error code.
 */
int msft_base_capabilities_negotiation (struct device_manager *device_mgr, uint8_t feature_flags,
	struct cmd_interface_msg *request)
{
	const struct msft_base_caps_negotiation_request *req;
	struct msft_base_caps_negotiation_response *resp;
	struct device_manager_capabilities capabilities = {0};
	uint16_t max_msg;
	uint16_t max_pkt;
	int device_num;

	if ((device_mgr == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct msft_base_caps_negotiation_request*) request->payload;

	if (req->header.protocol_version != MSFT_BASE_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct msft_base_caps_negotiation_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	max_msg = buffer_unaligned_read16 (&req->max_msg_size);
	max_pkt = buffer_unaligned_read16 (&req->max_pkt_size);
	if ((max_msg < MCTP_BASE_PROTOCOL_MIN_TRANSMISSION_UNIT) ||
		(max_pkt < MCTP_BASE_PROTOCOL_MIN_TRANSMISSION_UNIT)) {
		/* All devices are required to support at least minimum message and packet lengths. */
		return CMD_HANDLER_MSFT_OUT_OF_RANGE;
	}

	device_num = device_manager_get_device_num (device_mgr, request->source_eid);
	if (ROT_IS_ERROR (device_num)) {
		return device_num;
	}

	buffer_unaligned_write16 (&capabilities.max_message_size, max_msg);
	buffer_unaligned_write16 (&capabilities.max_packet_size, max_pkt);

	/* Since the device number is known to be valid, this call can't fail. */
	device_manager_update_device_capabilities_request (device_mgr, device_num, &capabilities);

	resp = (struct msft_base_caps_negotiation_response*) request->payload;

	buffer_unaligned_write16 (&resp->max_msg_size, MCTP_BASE_PROTOCOL_MAX_MESSAGE_BODY);
	buffer_unaligned_write16 (&resp->max_pkt_size, MCTP_BASE_PROTOCOL_MAX_TRANSMISSION_UNIT);
	buffer_unaligned_write16 (&resp->msg_timeout,
		device_manager_set_timeout_ms (MCTP_BASE_PROTOCOL_MAX_CRYPTO_TIMEOUT_MS));
	resp->feature_flags = feature_flags;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));

	resp->header.completion_code = 0;

	return 0;
}

/**
 * Process a temperature sensor read request and create the response containing temperature
 * readings.
 *
 * @param cluster The temperature sensor cluster to obtain readings from.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was created successfully or an error code.
 */
int msft_base_get_temperature (const struct temperature_sensor_cluster *cluster,
	struct cmd_interface_msg *request)
{
	const struct msft_base_get_temperature_request *req;
	struct msft_base_get_temperature_response *resp;
	size_t max_sensors;
	size_t max_response;
	int count;

	if ((cluster == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct msft_base_get_temperature_request*) request->payload;
	resp = (struct msft_base_get_temperature_response*) request->payload;

	if (req->header.protocol_version != MSFT_BASE_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct msft_base_get_temperature_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	max_response = cmd_interface_msg_get_max_response (request);
	if (max_response < sizeof (struct msft_base_get_temperature_response)) {
		return CMD_HANDLER_MSFT_BAD_MAX_RESPONSE;
	}

	if (req->read_single == 1) {
		max_sensors = 1;
	}
	else {
		max_sensors = temperature_sensor_cluster_get_sensor_count (cluster);

		max_response =
			(max_response - sizeof (*resp)) / sizeof (struct msft_base_temperature_reading);
		max_response++;	// Add another sensor since the response message always supports one.

		if (max_response < max_sensors) {
			max_sensors = max_response;
		}
	}

	count = temperature_sensor_cluster_sensor_range_get_temps (cluster, req->sensor_id,
		&resp->sensor, max_sensors);
	if (ROT_IS_ERROR (count)) {
		msft_base_build_error_response (request, MSFT_BASE_CC_UNSUPPORTED_PARAM, count);

		return 0;
	}

	resp->total_sensors = temperature_sensor_cluster_get_sensor_count (cluster);
	resp->sensors_reporting = count;

	cmd_interface_msg_set_message_payload_length (request,
		msft_base_get_temperature_response_length (resp));

	resp->header.completion_code = 0;

	return 0;
}

/**
 * Process a heartbeat control request and update the heartbeat listener list.
 *
 * @param notifier MCTP notifier instance to use.
 * @param request The message containing the request. This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was created successfully or an error code.
 */
int msft_base_heartbeat_control (const struct mctp_notifier_interface *notifier,
	struct cmd_interface_msg *request)
{
	const struct msft_base_heartbeat_ctrl_request *req;
	struct msft_base_heartbeat_ctrl_response *resp;
	int status;

	if ((notifier == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct msft_base_heartbeat_ctrl_request*) request->payload;
	resp = (struct msft_base_heartbeat_ctrl_response*) request->payload;

	if (req->header.protocol_version != MSFT_BASE_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct msft_base_heartbeat_ctrl_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	if (req->enable == 1) {
		status = notifier->register_listener (notifier, request->source_eid);
	}
	else {
		status = notifier->deregister_listener (notifier, request->source_eid);
	}

	if (ROT_IS_ERROR (status)) {
		return status;
	}

	cmd_interface_msg_set_message_payload_length (request,
		sizeof (struct msft_base_heartbeat_ctrl_response));

	resp->header.completion_code = 0;

	return 0;
}

/**
 * Build a heartbeat request notification payload.
 *
 * TODO:  Currently only supports a single CPU. Work need to be done to define new CPU health
 * interface with support for multiple CPUs.
 *
 * @param timeout_secs The maximum timeout, in seconds, before the next heartbeat should arrive.
 * @param core_id Identifier for the reporting core.
 * @param health_status Platform-defined health status value for the reporting core.
 * @param payload Output buffer for the heartbeat request payload.
 * @param payload_len Length of the output buffer.
 *
 * @return  Length of the generated request if the request was successfully constructed or an
 * error code.
 */
int msft_base_build_heartbeat_request (uint16_t timeout_secs, uint16_t core_id,
	uint16_t health_status, uint8_t *payload, size_t payload_len)
{
	struct msft_base_heartbeat_request *req =
		(struct msft_base_heartbeat_request*) payload;
	int status;

	if (payload == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (payload_len < sizeof (struct msft_base_heartbeat_request)) {
		return CMD_HANDLER_MSFT_BUF_TOO_SMALL;
	}

	status = msft_mctp_protocol_populate_header (&req->header, MSFT_BASE_CMD_HEARTBEAT,
		MSFT_MCTP_PROTOCOL_COMMAND_SET_BASE, MSFT_BASE_PROTOCOL_VERSION);
	if (status != 0) {
		return status;
	}

	buffer_unaligned_write16 (&req->timeout_secs, timeout_secs);
	req->cpu_count = 0;
	buffer_unaligned_write16 (&req->cpu.core_id, core_id);
	buffer_unaligned_write16 (&req->cpu.health_status, health_status);

	return sizeof (struct msft_base_heartbeat_request);
}
