// Copyright (c) Microsoft Corporation. All rights reserved.

#include <limits.h>
#include <string.h>
#include "cmd_interface_msft.h"
#include "msft_base_commands.h"
#include "msft_protocol_logging.h"
#include "rot_commands.h"
#include "common/buffer_util.h"
#include "common/common_math.h"


/**
 * Sets a target feature bit inside a capabilities bitmask array.
 *
 * @param feature_flags The capabilities bitmask array.
 * @param features_len The length of the feature_flags array.
 * @param feature The target ROT_FEATURE_* feature to set.
 *
 * @return 0 if successful or an error code.
 */
int rot_set_rot_capabilities_feature (uint8_t *feature_flags, size_t features_len,
	enum rot_feature feature)
{
	if (feature >= ROT_FEATURE_COUNT) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	// This will do buffer/length validation
	return common_math_set_bit_in_array (feature_flags, features_len, feature);
}

/**
 * Process a capabilities request and generate a response with device capabilities.
 *
 * @param feature_flags Bitmask array of features that are supported by the device.  These flags use
 * the bit indexes defined in {@link enum rot_features}.
 * @param features_len The length of the feature_flags array.
 * @param request The message containing the request.  This will be updated with an appropriate
 * response.
 *
 * @return 0 if a response was created successfully or an error code.
 */
int rot_get_rot_capabilities (const uint8_t *feature_flags, size_t features_len,
	struct cmd_interface_msg *request)
{
	const struct rot_get_rot_capabilities_request *req;
	struct rot_get_rot_capabilities_response *resp;

	if ((feature_flags == NULL) || (features_len != ROT_CAPABILITIES_SIZE) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct rot_get_rot_capabilities_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct rot_get_rot_capabilities_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	resp = (struct rot_get_rot_capabilities_response*) request->payload;

	resp->header.completion_code = 0;
	memcpy (resp->feature_flags, feature_flags, features_len);

	cmd_interface_msg_set_message_payload_length (request,
		sizeof (struct rot_get_rot_capabilities_response));

	return 0;
}

/**
 * Process a reset RoT request and create a response with the result.
 *
 * @param background The background command handler to use for scheduling the device reset.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.
 */
int rot_reset (const struct cmd_background *background, struct cmd_interface_msg *request)
{
	const struct rot_reset_request *req;
	struct rot_reset_response *resp;
	int status;

	if ((background == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct rot_reset_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct rot_reset_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = background->reboot_device (background);
	if (status != 0) {
		return status;
	}

	resp = (struct rot_reset_response*) request->payload;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Process a get unlock token request and create a response with the result.
 *
 * @param unlock The device handler for unlock requests.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int rot_get_unlock_token (const struct secure_device_unlock *unlock,
	struct cmd_interface_msg *request)
{
	const struct rot_get_unlock_token_request *req;
	struct rot_get_unlock_token_response *resp;
	size_t max_response;
	int status;

	if ((unlock == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct rot_get_unlock_token_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct rot_get_unlock_token_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	resp = (struct rot_get_unlock_token_response*) request->payload;

	max_response = cmd_interface_msg_get_max_response (request);
	if (max_response < sizeof (struct rot_get_unlock_token_response)) {
		return CMD_HANDLER_MSFT_BAD_MAX_RESPONSE;
	}

	status = unlock->get_unlock_token (unlock, &resp->token,
		rot_get_unlock_token_max_token_length (max_response));
	if (ROT_IS_ERROR (status)) {
		switch (status) {
			case SECURE_DEVICE_UNLOCK_NOT_LOCKED:
			case SECURE_DEVICE_UNLOCK_COUNTER_EXHAUSTED:
				/* These errors indicate a bad device state, preventing the token from being
				 * generated. */
				msft_base_build_error_response (request, ROT_CC_INVALID_DEVICE_STATE, status);
				status = 0;
				break;

			default:
				/* Any other error, just return the error code. */
				break;
		}

		return status;
	}

	cmd_interface_msg_set_message_payload_length (request,
		rot_get_unlock_token_response_length (status));

	resp->header.completion_code = 0;

	return 0;
}

/**
 * Check the option in the command for how to apply the lock/unlock policy.
 *
 * @param background The background command handler that will be used for device resets.  A null
 * handler indicates forced resets are not supported.
 * @param option The option value specified in the request.
 * @param reset Output to indicate if the specified option indicates a reset.  It will only get
 * changed if a reset is needed.
 * @param request The request structure that will be updated with an error response in the case of
 * an unsupported operation.
 *
 * @return 0 if the option is valid or an error code.
 */
static int rot_check_unlock_option (const struct cmd_background *background, uint8_t option,
	bool *reset, struct cmd_interface_msg *request)
{
	switch (option) {
		case ROT_UNLOCK_POLICY_OPTION_NEXT_RESET:
			return 0;

		case ROT_UNLOCK_POLICY_OPTION_FORCE_RESET:
			if (background != NULL) {
				*reset = true;

				return 0;
			}

		/* When there is no background handler, resetting is not supported. */
		/* fall through */ /* no break */

		case ROT_UNLOCK_POLICY_OPTION_IMMEDIATELY:
			/* Immediately applying unlock policies is not supported. */
			msft_base_build_error_response (request, MSFT_BASE_CC_UNSUPPORTED_PARAM,
				CMD_HANDLER_MSFT_UNSUPPORTED_OPERATION);

			return CMD_HANDLER_MSFT_UNSUPPORTED_OPERATION;

		default:
			return CMD_HANDLER_MSFT_OUT_OF_RANGE;
	}
}

/**
 * Schedule a device warm reset if requested as part of the command.  Failures to execute the reset
 * will be logged.
 *
 * @param background The background command handler to use for the warm reset.
 * @param reset Flag indicating if the reset should be performed.
 * @param header Header information for the request being processed.
 */
static void rot_unlock_do_reset (const struct cmd_background *background, bool reset,
	const struct msft_mctp_protocol_header *header)
{
	int status;

	if (reset) {
		status = background->reboot_device (background);
		if (status != 0) {
			/* Don't fail the operation just because the reset couldn't be executed.  Log the
			 * failure for telemetry purposes. */
			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, MSFT_LOGGING_COMPONENT_MVDP,
				MSFT_PROTOCOL_LOGGING_REBOOT_FAILED, (header->command_set << 8) | header->command,
				status);
		}
	}
}

/**
 * Process an apply unlock token request and create a response with the result.
 *
 * @param unlock The device handler for unlock requests.
 * @param background Optional background command handler to use when triggering device resets.  If
 * this is null, commands that request device resets are unsupported.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int rot_apply_unlock_policy (const struct secure_device_unlock *unlock,
	const struct cmd_background *background, struct cmd_interface_msg *request)
{
	const struct rot_apply_unlock_policy_request *req;
	struct rot_apply_unlock_policy_response *resp;
	int status;
	bool reset = false;

	if ((unlock == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct rot_apply_unlock_policy_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length < sizeof (struct rot_apply_unlock_policy_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = rot_check_unlock_option (background, req->option, &reset, request);
	if (status == CMD_HANDLER_MSFT_UNSUPPORTED_OPERATION) {
		/* An error response was generated for this case. */
		return 0;
	}
	else if (status != 0) {
		return status;
	}

	status = unlock->apply_unlock_policy (unlock, &req->unlock_policy,
		rot_apply_unlock_policy_get_policy_length (request->payload_length));
	if (status != 0) {
		return status;
	}

	rot_unlock_do_reset (background, reset, &req->header);

	resp = (struct rot_apply_unlock_policy_response*) request->payload;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Process a clear unlock token request and create a response with the result.
 *
 * @param unlock The device handler for unlock requests.
 * @param background Optional background command handler to use when triggering device resets.  If
 * this is null, commands that request device resets are unsupported.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int rot_clear_unlock_policy (const struct secure_device_unlock *unlock,
	const struct cmd_background *background, struct cmd_interface_msg *request)
{
	const struct rot_clear_unlock_policy_request *req;
	struct rot_clear_unlock_policy_response *resp;
	int status;
	bool reset = false;

	if ((unlock == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct rot_clear_unlock_policy_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct rot_clear_unlock_policy_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = rot_check_unlock_option (background, req->option, &reset, request);
	if (status == CMD_HANDLER_MSFT_UNSUPPORTED_OPERATION) {
		/* An error response was generated for this case. */
		return 0;
	}
	else if (status != 0) {
		return status;
	}

	status = unlock->clear_unlock_policy (unlock);
	if (status != 0) {
		return status;
	}

	rot_unlock_do_reset (background, reset, &req->header);

	resp = (struct rot_clear_unlock_policy_response*) request->payload;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Process a get time request and create a response with the result.
 *
 * @param rtc The real time clock handler for time requests.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int rot_get_time (const struct real_time_clock *rtc, struct cmd_interface_msg *request)
{
	const struct rot_get_time_request *req;
	struct rot_get_time_response *resp;
	uint64_t timestamp;
	int status;

	if ((rtc == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct rot_get_time_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct rot_get_time_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	timestamp = 0;
	status = rtc->get_time (rtc, &timestamp);
	if (status != 0) {
		return status;
	}

	timestamp /= 1000;
	if (timestamp > UINT32_MAX) {
		return CMD_HANDLER_MSFT_OUT_OF_RANGE;
	}

	resp = (struct rot_get_time_response*) request->payload;

	buffer_unaligned_write32 (&resp->secs, (uint32_t) timestamp);

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Process a set time request and create a response with the result.
 *
 * @param rtc The real time clock handler for time requests.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int rot_set_time (const struct real_time_clock *rtc, struct cmd_interface_msg *request)
{
	const struct rot_set_time_request *req;
	struct rot_set_time_response *resp;
	uint32_t secs;
	int status;

	if ((rtc == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct rot_set_time_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct rot_set_time_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	secs = buffer_unaligned_read32 (&req->secs);
	status = rtc->set_time (rtc, (uint64_t) secs * 1000);
	if (status != 0) {
		return status;
	}

	resp = (struct rot_set_time_response*) request->payload;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Get the current state of the intrusion detection signal and create a response
 * with the result.
 *
 * @param intrusion The intrusion state handler for intrusion requests.
 * @param notifier The MCTP notifier interface handler for listener registration requests.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code. A valid response could be
 * an error message.
 */
int rot_get_intrusion_detection (const struct intrusion_state *intrusion,
	const struct mctp_notifier_interface *notifier, struct cmd_interface_msg *request)
{
	const struct rot_get_intrusion_detection_request *req;
	struct rot_get_intrusion_detection_response *resp;
	uint32_t active_state;
	int status;

	if ((request == NULL) || (intrusion == NULL) || (notifier == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct rot_get_intrusion_detection_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct rot_get_intrusion_detection_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = intrusion->is_active ((struct intrusion_state*) intrusion, &active_state);
	if (status != 0) {
		return status;
	}

	if (req->intrusion_event_control.control.event_register == 1) {
		if (req->intrusion_event_control.control.force_send_event == 1) {
			status = notifier->force_register_listener (notifier, request->source_eid);
		}
		else if (req->intrusion_event_control.control.start_event_reporting == 1) {
			status = notifier->register_listener (notifier, request->source_eid);
		}
		else {
			status = notifier->deregister_listener (notifier, request->source_eid);
		}

		if (status != 0) {
			return status;
		}
	}

	resp = (struct rot_get_intrusion_detection_response*) request->payload;
	resp->intrusion_detection = (uint8_t) active_state;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Process a get intrusion count request and create a response with the result.
 *
 * @param intrusion The intrusion state handler for intrusion requests.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code. A valid response could be
 * an error message.
 */
int rot_get_intrusion_count (const struct intrusion_state *intrusion,
	struct cmd_interface_msg *request)
{
	const struct rot_get_intrusion_count_request *req;
	struct rot_get_intrusion_count_response *resp;
	uint32_t count;
	int status;

	if ((request == NULL) || (intrusion == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct rot_get_intrusion_count_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct rot_get_intrusion_count_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = intrusion->get_intrusion_count ((struct intrusion_state*) intrusion, &count);
	if (status != 0) {
		return status;
	}

	resp = (struct rot_get_intrusion_count_response*) request->payload;
	resp->intrusion_count = count;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Build an intrusion event request message.
 *
 * @param event The intrusion event. 0 means no active intrusion detected;
 * otherwise, active intrusion detected.
 * @param payload Output to be loaded with the intrusion event message.
 * @param payload_len The length of the intrusion event message
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int rot_build_intrusion_event_request (uint8_t event, uint8_t *payload, size_t payload_len)
{
	struct rot_intrusion_event_request *req = (struct rot_intrusion_event_request*) payload;
	int status;

	if (payload == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (payload_len < sizeof (struct rot_intrusion_event_request)) {
		return CMD_HANDLER_MSFT_BUF_TOO_SMALL;
	}

	status = msft_mctp_protocol_populate_header (&req->header, ROT_CMD_INTRUSION_EVENT,
		MSFT_MCTP_PROTOCOL_COMMAND_SET_ROT, 0x0);
	if (status != 0) {
		return status;
	}

	req->intrusion_event = event;

	return 0;
}

/**
 * Process a read log command request and create a response with the result.
 *
 * @param log The log interface handle.
 * @param hash The hashing engine for hash of log data.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.
 */
int rot_read_log_command (const struct rot_log_interface *log, const struct hash_engine *hash,
	struct cmd_interface_msg *request)
{
	struct rot_read_log_command_request *req;
	struct rot_read_log_command_response *resp;
	int log_length = 0;
	uint8_t type = 0;
	uint16_t log_id = 0;
	uint32_t offset = 0;
	int status = 0;

	if ((log == NULL) || (hash == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (struct rot_read_log_command_request*) request->payload;

	if (req->header.protocol_version != ROT_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct rot_read_log_command_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	type = req->type;
	log_id = buffer_unaligned_read16 (&req->log_id);
	offset = buffer_unaligned_read32 (&req->offset);

	resp = (struct rot_read_log_command_response*) request->payload;

	resp->type = type;
	buffer_unaligned_write16 (&resp->log_id, log_id);

	log_length = log->get_log (log, type, log_id, offset, rot_log_response_data (resp),
		ROT_READ_LOG_MAX_LOG_DATA (request), NULL);
	if (ROT_IS_ERROR (log_length)) {
		return log_length;
	}

	status = hash->calculate_sha256 (hash, rot_log_response_data (resp), log_length, resp->sha256,
		SHA256_HASH_LENGTH);
	if (status != 0) {
		return status;
	}

	resp->hash_type = ROT_LOG_INTERFACE_HASH_TYPE_SHA256;
	buffer_unaligned_write32 (&resp->offset, offset);

	cmd_interface_msg_set_message_payload_length (request,
		rot_read_log_response_log_length (log_length));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Build an send log command request.
 *
 * @param log The log interface handle.
 * @param payload Output to be loaded with the send log message.
 * @param payload_len The length of the send log message
 *
 * @return Length of the generated request data if the request was successfully constructed
 * or an error code. Don't return 0
 *
 */
int rot_build_send_log_command_request (const struct rot_log_interface *log, uint8_t log_type,
	uint8_t *payload, size_t payload_len)
{
	struct rot_send_log_command_request *req = (struct rot_send_log_command_request*) payload;
	struct rot_log_interface_entry_info *info = NULL;
	int log_length = 0;
	size_t max_length = 0;
	int status = 0;
	bool more_entry_data = false;

	if ((log == NULL) || (payload == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (payload_len < sizeof (struct rot_send_log_command_request)) {
		return CMD_HANDLER_MSFT_BUF_TOO_SMALL;
	}

	status = msft_mctp_protocol_populate_header (&req->header, ROT_CMD_SEND_LOG_COMMAND,
		MSFT_MCTP_PROTOCOL_COMMAND_SET_ROT, ROT_PROTOCOL_VERSION);
	if (status != 0) {
		return status;
	}

	req->type = log_type;
	req->flag = 0;

	max_length = payload_len - sizeof (struct rot_send_log_command_request);

	log_length = log->get_log (log, req->type, 0, 0, rot_log_request_data (req), max_length,
		&more_entry_data);

	if (ROT_IS_ERROR (log_length)) {
		return log_length;
	}

	if (more_entry_data) {
		info = (struct rot_log_interface_entry_info*) rot_log_request_data (req);
		max_length = max_length - sizeof (struct rot_log_interface_entry_info);

		log_length = log->get_entry_read_info (log, req->type, info,
			rot_log_entry_info_digest_data (req), max_length);
		if (ROT_IS_ERROR (log_length)) {
			return log_length;
		}

		req->flag |= ROT_SEND_LOG_FLAG_READ_REQUEST;
	}

	return rot_send_log_request_log_length (log_length);
}
