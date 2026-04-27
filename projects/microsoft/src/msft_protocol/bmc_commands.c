// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <string.h>
#include "bmc_commands.h"
#include "cmd_interface_msft.h"
#include "msft_mctp_protocol.h"


/**
 * Construct chassis intrusion store data request.
 *
 * @param data Buffer with data to store.
 * @param num_bytes Number of bytes to store.
 * @param buf Output buffer for the generated request data.
 * @param buf_len Maximum size of buffer.
 *
 * @return Length of the generated request data if the request was successfully constructed or
 * an error code.
 */
int bmc_chassis_intrusion_generate_store_data_request (const uint8_t *data,	uint8_t num_bytes,
	uint8_t *buf, size_t buf_len)
{
	struct bmc_chassis_intrusion_store_data *rq = (struct bmc_chassis_intrusion_store_data*) buf;
	int status;

	if ((rq == NULL) || (data == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (buf_len < bmc_chassis_intrusion_store_data_length (num_bytes)) {
		return CMD_HANDLER_MSFT_BUF_TOO_SMALL;
	}

	status = msft_mctp_protocol_populate_header (&rq->header, BMC_CMD_CHASSIS_INTRUSION_STORE_DATA,
		MSFT_MCTP_PROTOCOL_COMMAND_SET_BMC, 0);
	if (status != 0) {
		return status;
	}

	rq->num_bytes = num_bytes;
	memcpy (bmc_chassis_intrusion_store_data_data (rq), data, num_bytes);

	return bmc_chassis_intrusion_store_data_length (num_bytes);
}

/**
 * Process a chassis intrusion store data response.
 *
 * @param response Response buffer to process.
 *
 * @return Response processing completion status, 0 if successful or error code otherwise.
 */
int bmc_chassis_intrusion_process_store_data_response (struct cmd_interface_msg *response)
{
	if (response == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (response->payload_length != sizeof (struct bmc_chassis_intrusion_store_data_response)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	return 0;
}

/**
 * Construct chassis intrusion challenge data request.
 *
 * @param hash_algo Hash algorithm to use in challenge response.
 * @param nonce Buffer with nonce to use in request. Nonce must be
 * BMC_CHASSIS_INTRUSION_NONCE_LEN bytes long.
 * @param buf Output buffer for the generated request data.
 * @param buf_len Maximum size of buffer.
 *
 * @return Length of the generated request data if the request was successfully constructed or
 * an error code.
 */
int bmc_chassis_intrusion_generate_challenge_data_request (enum hash_type hash_algo, uint8_t *nonce,
	uint8_t *buf, size_t buf_len)
{
	struct bmc_chassis_intrusion_challenge_data *rq =
		(struct bmc_chassis_intrusion_challenge_data*) buf;
	int status;

	if ((rq == NULL) || (nonce == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (buf_len < sizeof (struct bmc_chassis_intrusion_challenge_data)) {
		return CMD_HANDLER_MSFT_BUF_TOO_SMALL;
	}

	status = msft_mctp_protocol_populate_header (&rq->header,
		BMC_CMD_CHASSIS_INTRUSION_CHALLENGE_DATA, MSFT_MCTP_PROTOCOL_COMMAND_SET_BMC, 0);
	if (status != 0) {
		return status;
	}

	switch (hash_algo) {
		case HASH_TYPE_SHA256:
			rq->hash_algo = BMC_CHASSIS_INTRUSION_HASH_SHA256;
			break;

		case HASH_TYPE_SHA384:
			rq->hash_algo = BMC_CHASSIS_INTRUSION_HASH_SHA384;
			break;

		case HASH_TYPE_SHA512:
			rq->hash_algo = BMC_CHASSIS_INTRUSION_HASH_SHA512;
			break;

		default:
			return CMD_HANDLER_MSFT_OUT_OF_RANGE;
	}

	memcpy (rq->nonce, nonce, BMC_CHASSIS_INTRUSION_NONCE_LEN);

	return sizeof (struct bmc_chassis_intrusion_challenge_data);
}

/**
 * Process a chassis intrusion challenge data response.
 *
 * @param response Response buffer to process.
 *
 * @return Response processing completion status, 0 if successful or error code otherwise.
 */
int bmc_chassis_intrusion_process_challenge_data_response (
	struct cmd_interface_msg *response)
{
	struct bmc_chassis_intrusion_challenge_data_response *rsp;

	if (response == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	rsp = (struct bmc_chassis_intrusion_challenge_data_response*) response->payload;

	if ((response->payload_length <
		sizeof (struct bmc_chassis_intrusion_challenge_data_response)) ||
		(response->payload_length !=
		bmc_chassis_intrusion_challenge_data_response_length (rsp->hash_len))) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	return 0;
}

/**
 * Construct get system devices data request.
 *
 * @param start_index The device entry index that will be the first device listed in the
 * response message.
 * @param entry_count The maximum number of device entries to return.  If this is 0, all
 * device entries will be returned, up to the maximum message size.
 * @param filter_properties Filter results based on properties of the device
 * @param buf Output buffer for the generated request data.
 * @param buf_len Maximum size of buffer.
 *
 * @return Length of the generated request data if the request was successfully constructed or
 * an error code.
 */
int bmc_system_devices_generate_get_data_request (uint16_t start_index, uint16_t entry_count,
	uint16_t filter_properties, uint8_t *buf, size_t buf_len)
{
	struct bmc_system_devices_get_data *rq =
		(struct bmc_system_devices_get_data*) buf;
	int status;

	if (rq == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (buf_len < sizeof (struct bmc_system_devices_get_data)) {
		return CMD_HANDLER_MSFT_BUF_TOO_SMALL;
	}

	status = msft_mctp_protocol_populate_header (&rq->header, BMC_CMD_GET_SYSTEM_DEVICES,
		MSFT_MCTP_PROTOCOL_COMMAND_SET_BMC, 0);
	if (status != 0) {
		return status;
	}

	rq->start_index = start_index;
	rq->entry_count = entry_count;
	rq->filter_device_properties = filter_properties;

	return sizeof (struct bmc_system_devices_get_data);
}

/**
 * Process get system devices response.
 *
 * @param response Response buffer to process.
 *
 * @return Response processing completion status, 0 if successful or error code otherwise.
 */
int bmc_system_devices_process_get_data_response (struct cmd_interface_msg *response)
{
	struct bmc_system_devices_get_data_response *rsp;

	if (response == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	rsp = (struct bmc_system_devices_get_data_response*) (response->payload);
	if ((response->payload_length < sizeof (struct bmc_system_devices_get_data_response)) ||
		(response->payload_length !=
		bmc_system_devices_get_data_response_length (rsp->entry_count))) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	return 0;
}

/**
 * Construct get device string data request.
 *
 * @param vendor_id Vendor ID of the device.
 * @param device_id Device ID of the device.
 * @param subsystem_vendor_id Subsystem vendor ID of the device.
 * @param subsystem_id Subsystem ID of the device.
 * @param instance_id Instance ID of the device.
 * @param string_type Type of string to retrieve.
 *
 * @return Length of the generated request data if the request was successfully constructed or
 * an error code.
 */
int bmc_system_device_generate_string_identifier_get_data_request (uint16_t vendor_id,
	uint16_t device_id,	uint16_t subsystem_vendor_id, uint16_t subsystem_id, uint8_t instance_id,
	uint8_t string_type, uint8_t *buf, size_t buf_len)
{
	struct bmc_system_device_string_identifier_get_data *rq =
		(struct bmc_system_device_string_identifier_get_data*) buf;
	int status;

	if (rq == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (!((string_type == BMC_DEVICE_STRING_TYPE_DEVICE_INSTANCE) ||
		(string_type == BMC_DEVICE_STRING_TYPE_DEVICE_TYPE))) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (buf_len < sizeof (struct bmc_system_device_string_identifier_get_data)) {
		return CMD_HANDLER_MSFT_BUF_TOO_SMALL;
	}

	status = msft_mctp_protocol_populate_header (&rq->header, BMC_CMD_GET_DEVICE_STRING_IDENTIFIER,
		MSFT_MCTP_PROTOCOL_COMMAND_SET_BMC, 0);
	if (status != 0) {
		return status;
	}

	rq->vendor_id = vendor_id;
	rq->device_id = device_id;
	rq->subsystem_vendor_id = subsystem_vendor_id;
	rq->subsystem_id = subsystem_id;
	rq->instance_id = instance_id;
	rq->string_type = string_type;

	return sizeof (struct bmc_system_device_string_identifier_get_data);
}

/**
 * Process get device string data response.
 *
 * @param response Response buffer to process.
 *
 * @return Response processing completion status, 0 if successful or error code otherwise.
 */
int bmc_system_device_process_string_identifier_get_data_response (
	struct cmd_interface_msg *response)
{
	struct bmc_system_device_string_identifier_get_data_response *rsp;

	if (response == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	rsp = (struct bmc_system_device_string_identifier_get_data_response*) response->payload;
	if ((response->payload_length <
		sizeof (struct bmc_system_device_string_identifier_get_data_response)) ||
		(response->payload_length !=
		bmc_system_device_string_identifier_get_data_response_length (rsp->string_length))) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	return 0;
}

/**
 * Construct get device EID data request.
 *
 * @param vendor_id Vendor ID of the device.
 * @param device_id Device ID of the device.
 * @param subsystem_vendor_id Subsystem vendor ID of the device.
 * @param subsystem_id Subsystem ID of the device.
 * @param instance_id Instance ID of the device.
 * @param buf Output buffer for the generated request data.
 * @param buf_len Maximum size of buffer.
 *
 * @return Length of the generated request data if the request was successfully constructed or
 * an error code.
 */
int bmc_system_device_generate_eid_get_data_request (uint16_t vendor_id, uint16_t device_id,
	uint16_t subsystem_vendor_id, uint16_t subsystem_id, uint8_t instance_id, uint8_t *buf,
	size_t buf_len)
{
	struct bmc_system_device_eid_get_data *rq = (struct bmc_system_device_eid_get_data*) buf;
	int status;

	if (rq == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (buf_len < sizeof (struct bmc_system_device_eid_get_data)) {
		return CMD_HANDLER_MSFT_BUF_TOO_SMALL;
	}

	status = msft_mctp_protocol_populate_header (&rq->header, BMC_CMD_GET_DEVICE_EID,
		MSFT_MCTP_PROTOCOL_COMMAND_SET_BMC, 0);
	if (status != 0) {
		return status;
	}
	rq->vendor_id = vendor_id;
	rq->device_id = device_id;
	rq->subsystem_vendor_id = subsystem_vendor_id;
	rq->subsystem_id = subsystem_id;
	rq->instance_id = instance_id;

	return sizeof (struct bmc_system_device_eid_get_data);
}

/**
 * Process get device EID data response.
 *
 * @param response Response buffer to process.
 *
 * @return Response processing completion status, 0 if successful or error code otherwise.
 */
int bmc_system_device_process_eid_get_data_response (struct cmd_interface_msg *response)
{
	struct bmc_system_device_eid_get_data_response *rsp;

	if (response == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	rsp = (struct bmc_system_device_eid_get_data_response*) response->payload;
	if ((response->payload_length < sizeof (struct bmc_system_device_eid_get_data_response)) ||
		(response->payload_length !=
		bmc_system_device_eid_get_data_response_length (rsp->eid_count))) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	return 0;
}
