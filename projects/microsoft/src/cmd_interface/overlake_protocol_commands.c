// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "attestation/attestation_responder.h"
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cerberus_protocol_master_commands.h"
#include "cmd_interface/cerberus_protocol_optional_commands.h"
#include "cmd_interface/cmd_interface_overlake.h"
#include "cmd_interface/cmd_logging.h"
#include "cmd_interface/overlake_protocol.h"
#include "cmd_interface/overlake_protocol_commands.h"
#include "common/common_math.h"
#include "crypto/ecdsa.h"
#include "host_fw/host_fw_cmd_interface.h"
#include "host_fw/overlake_board_id.h"
#include "host_fw/overlake_control.h"
#include "host_fw/overlake_flash_manager.h"
#include "host_fw/overlake_flash_manager_fpga.h"
#include "host_fw/overlake_host_id.h"
#include "tpm/tpm.h"


/**
 * Process get storage packet
 *
 * @param tpm TPM instance to utilize
 * @param mask_data_errors flag to mask errors and return empty buffer when storage read fails.
 * @param request Request to process
 *
 * @return 0 if request processing completed successfully or an error code.
 */
int overlake_protocol_get_storage (struct tpm *tpm, bool mask_data_errors,
	struct cmd_interface_msg *request)
{
	CERBERUS_PROTOCOL_CMD (rq, struct overlake_protocol_get_storage_request_packet*, request);
	CERBERUS_PROTOCOL_CMD (rsp, struct overlake_protocol_get_storage_response_packet*, request);
	int status;
	uint16_t tpm_segment_size;

	if (request->length !=
		CERBERUS_PROTOCOL_CMD_LEN (struct overlake_protocol_get_storage_request_packet)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	status = tpm_get_segment_storage_size (tpm, &tpm_segment_size);
	if (status != 0) {
		return status;
	}

	if (tpm_segment_size > cmd_interface_msg_get_max_response (request)) {
		return CMD_HANDLER_BUF_TOO_SMALL;
	}

	status = tpm_get_storage (tpm, rq->index, &rsp->segment_data, tpm_segment_size,
		mask_data_errors);
	if (status != 0) {
		return status;
	}

	request->length =
		CERBERUS_PROTOCOL_CMD_LEN (struct overlake_protocol_get_storage_response_packet) -
		sizeof (uint8_t) + tpm_segment_size;

	return 0;
}

/**
 * Process set storage packet
 *
 * @param tpm TPM instance to utilize
 * @param request Request to process
 *
 * @return 0 if request processing completed successfully or an error code.
 */
int overlake_protocol_set_storage (struct tpm *tpm, struct cmd_interface_msg *request)
{
	CERBERUS_PROTOCOL_CMD (rq, struct overlake_protocol_set_storage_request_packet*, request);
	int status;
	uint16_t tpm_segment_size;

	status = tpm_get_segment_storage_size (tpm, &tpm_segment_size);
	if (status != 0) {
		return status;
	}

	if (request->length !=
		(CERBERUS_PROTOCOL_CMD_LEN (struct overlake_protocol_set_storage_request_packet) -
		sizeof (uint8_t) + tpm_segment_size)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	status = tpm_set_storage (tpm, rq->index, &rq->segment_data, tpm_segment_size);
	if (status != 0) {
		return status;
	}

	request->length = 0;

	return 0;
}

/**
 * Process read data packet
 *
 * @param flash The flash block storage to read data from
 * @param request Request to process
 *
 * @return 0 if request processing completed successfully or an error code.
 */
int overlake_protocol_read_data (const struct flash_store *flash, struct cmd_interface_msg *request)
{
	struct overlake_protocol_read_data *rq = (struct overlake_protocol_read_data*) request->data;
	struct overlake_protocol_read_data_response *rsp =
		(struct overlake_protocol_read_data_response*) request->data;
	int status;

	if (request->length != sizeof (struct overlake_protocol_read_data)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	status = flash->read (flash, rq->id, &rsp->payload, OVERLAKE_PROTOCOL_MAX_READ_DATA (request));
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	request->length = overlake_protocol_read_data_response_length (status);

	return 0;
}

/**
 * Process clear data packet
 *
 * @param request Request to process
 *
 * @return 0 if request processing completed successfully or an error code.
 */
int overlake_protocol_clear_data (const struct flash_store *flash,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_clear_data *rq = (struct overlake_protocol_clear_data*) request->data;
	int status;

	if (request->length != sizeof (struct overlake_protocol_clear_data)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	status = flash->erase (flash, rq->id);
	if (status != 0) {
		return status;
	}

	request->length = 0;

	return 0;
}

/**
 * Process store data packet
 *
 * @param flash The flash block storage to store data
 * @param request Request to process
 *
 * @return 0 if request processing completed successfully or an error code.
 */
int overlake_protocol_store_data (const struct flash_store *flash,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_store_data *rq = (struct overlake_protocol_store_data*) request->data;
	int status;

	if (request->length < sizeof (struct overlake_protocol_store_data)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	status = flash->write (flash, rq->id, &rq->payload,
		overlake_protocol_store_data_length (request));
	if (status != 0) {
		return status;
	}

	request->length = 0;

	return 0;
}

/**
 * Process get signature packet
 *
 * @param ecc ECC engine for data signing.
 * @param riot Manager for the ECC private key for signing data.
 * @param hash Hash engine for data signing.
 * @param request Request to process
 *
 * @return 0 if request processing completed successfully or an error code.
 */
int overlake_protocol_sign_data (const struct ecc_engine *ecc, const struct riot_key_manager *riot,
	const struct hash_engine *hash, struct cmd_interface_msg *request)
{
	struct overlake_protocol_sign_data *rq = (struct overlake_protocol_sign_data*) request->data;
	struct overlake_protocol_sign_data_response *rsp =
		(struct overlake_protocol_sign_data_response*) request->data;
	const struct riot_keys *keys;
	int status;

	if (request->length < sizeof (struct overlake_protocol_sign_data)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	keys = riot_key_manager_get_riot_keys (riot);
	status = ecdsa_sign_message (ecc, hash, HASH_TYPE_SHA256, NULL, keys->alias_key,
		keys->alias_key_length, &rq->payload, overlake_protocol_sign_data_length (request),
		&rsp->signature, OVERLAKE_PROTOCOL_MAX_SIGNATURE_DATA (request));
	riot_key_manager_release_riot_keys (riot, keys);

	if (ROT_IS_ERROR (status)) {
		return status;
	}

	request->length = overlake_protocol_sign_data_response_length (status);

	return 0;
}

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
int overlake_protocol_host_fw_init (const struct host_fw_cmd_interface *host_fw_cmd[],
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
int overlake_protocol_host_fw_update (const struct host_fw_cmd_interface *host_fw_cmd[],
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
int overlake_protocol_get_host_fw_update_status (const struct host_fw_cmd_interface *host_fw_cmd[],
	uint8_t num_ports, struct cmd_interface_msg *request)
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
 * Process TPM clear packet
 *
 * @param tpm TPM instance to utilize
 * @param request Request to process
 *
 * @return 0 if request processing completed successfully or an error code.
 */
int overlake_protocol_tpm_clear (struct tpm *tpm, struct cmd_interface_msg *request)
{
	if ((request->length !=
		CERBERUS_PROTOCOL_CMD_LEN (struct overlake_protocol_tpm_clear_request_packet)) &&
		(request->length != CERBERUS_PROTOCOL_MIN_MSG_LEN)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	request->length = 0;

	return tpm_schedule_clear (tpm);
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
int overlake_protocol_soc_reset (struct overlake_flash_manager *flash,
	struct overlake_flash_manager *flash_fpga, struct overlake_control *soc_control,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_soc_reset *rq = (struct overlake_protocol_soc_reset*) request->data;
	int status;
	struct overlake_flash_manager *flash_mgr;

	if (request->length != sizeof (struct overlake_protocol_soc_reset)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	if (rq->id >= OVERLAKE_HOST_NUM_PORTS) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	flash_mgr = (rq->id == OVERLAKE_HOST_PORT_FPGA_C5) ? flash_fpga : flash;
	if (flash_mgr == NULL) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	/* Only soc reset issued OOB is allowed to cancel SoC fw updates in progress. */
	if (request->channel_id == CMD_INTERFACE_OVERLAKE_CHANNEL_I2C_BMC) {
		status = flash_mgr->cancel_active_updates (flash_mgr);
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

/**
 * Process a request to trigger an NMI in the SoC.
 *
 * @param soc_control SoC control instance to utilize.
 * @param request NMI trigger request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_nmi_trigger (struct overlake_control *soc_control,
	struct cmd_interface_msg *request)
{
	int status;

	if (request->length != CERBERUS_PROTOCOL_MIN_MSG_LEN) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	status = soc_control->soc_trigger_nmi (soc_control, true);
	if (status != 0) {
		return status;
	}

	platform_msleep (10);

	status = soc_control->soc_trigger_nmi (soc_control, false);
	if (status != 0) {
		return status;
	}

	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_CMD_INTERFACE,
		CMD_LOGGING_SOC_NMI_TRIGGERED, status, 0);

	request->length = 0;

	return 0;
}

/**
 * Process a request to set the boot mode for the SoC.
 *
 * @param flash Overlake flash manager to utilize.
 * @param request SoC boot mode request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_set_soc_boot (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_boot_mode_payload *rq =
		(struct overlake_protocol_boot_mode_payload*) request->data;

	if (request->length != sizeof (struct overlake_protocol_boot_mode_payload)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	if (rq->mode >= OVERLAKE_SOC_BOOT_INVALID) {
		return CMD_HANDLER_OUT_OF_RANGE;
	}

	request->length = 0;

	return flash->set_soc_boot_mode (flash, rq->mode);
}

/**
 * Process a request to get the current SoC boot mode.
 *
 * @param flash Overlake flash manager to utilize.
 * @param board_type The Overlake board type.
 * @param request SoC boot mode request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_get_soc_boot (struct overlake_flash_manager *flash,
	enum overlake_board_type board_type, struct cmd_interface_msg *request)
{
	struct overlake_protocol_boot_mode_payload *rsp =
		(struct overlake_protocol_boot_mode_payload*) request->data;
	int status;
	uint8_t mode;

	if (board_type == OVERLAKE_CASTLE_PEAK) {
		return CMD_HANDLER_UNSUPPORTED_OPERATION;
	}

	if (flash == NULL) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	if (request->length != CERBERUS_PROTOCOL_MIN_MSG_LEN) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	status = flash->get_soc_boot_mode (flash, &mode);
	if (status != 0) {
		return status;
	}

	rsp->mode = mode;

	request->length = sizeof (struct overlake_protocol_boot_mode_payload);

	return 0;
}

/**
 * Process a request to set the boot mode for the FPGA.
 *
 * @param flash Overlake flash manager to utilize.
 * @param board_type The Overlake board type.
 * @param request FPGA boot mode request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_set_fpga_boot (struct overlake_flash_manager *flash,
	enum overlake_board_type board_type, struct cmd_interface_msg *request)
{
	struct overlake_protocol_boot_mode_payload *rq =
		(struct overlake_protocol_boot_mode_payload*) request->data;

	if (board_type == OVERLAKE_CASTLE_PEAK) {
		return CMD_HANDLER_UNSUPPORTED_OPERATION;
	}

	if (flash == NULL) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	if (request->length != sizeof (struct overlake_protocol_boot_mode_payload)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	if ((rq->mode & ~(OVERLAKE_FPGA_BOOT_MODE_MASK)) > 0) {
		return CMD_HANDLER_OUT_OF_RANGE;
	}

	request->length = 0;

	return flash->set_soc_boot_mode (flash, rq->mode);
}

/**
 * Process a request to get the SoC MAC address.
 *
 * @param flash Overlake flash manager to utilize.
 * @param request SoC mac address request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_get_soc_mac_address (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request)
{
	int status;

	if (request->length != CERBERUS_PROTOCOL_MIN_MSG_LEN) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	status = flash->get_soc_mac_address (flash, &request->data[CERBERUS_PROTOCOL_MIN_MSG_LEN]);
	if (status != 0) {
		return status;
	}

	request->length = CERBERUS_PROTOCOL_MIN_MSG_LEN + 6;

	return 0;
}

/**
 * Process a request to set the debug level for the SoC.
 *
 * @param flash Overlake flash manager to utilize.
 * @param request SoC debug level request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_set_soc_debug_level (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_soc_debug_level *rq =
		(struct overlake_protocol_soc_debug_level*) request->data;

	if (request->length != sizeof (struct overlake_protocol_soc_debug_level)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	request->length = 0;

	return flash->set_soc_debug_level (flash, rq->debug_level);
}

/**
 * Process a request to get the current SoC debug level.
 *
 * @param flash Overlake flash manager to utilize.
 * @param request SoC debug level request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_get_soc_debug_level (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_soc_debug_level *resp =
		(struct overlake_protocol_soc_debug_level*) request->data;
	int status;

	if (request->length != sizeof (struct overlake_protocol_soc_get_debug_level_request)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	status = flash->get_soc_debug_level (flash, &resp->debug_level);
	if (status != 0) {
		return status;
	}

	request->length = sizeof (struct overlake_protocol_soc_debug_level);

	return 0;
}

/**
 * Process a request to get the fwversion of the requested port.
 *
 * @param flash Overlake flash manager to utilize.
 * @param request SoC fwversion request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_get_soc_fwversion (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_soc_fw_version *rq =
		(struct overlake_protocol_soc_fw_version*) request->data;
	struct overlake_protocol_soc_fw_version_response *resp =
		(struct overlake_protocol_soc_fw_version_response*) request->data;
	struct overlake_soc_firmware *fw_mgr = NULL;
	char *version = (char*) &resp->version;
	int status;

	if (request->length != sizeof (struct overlake_protocol_soc_fw_version)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	switch (rq->port_id) {
		case 0:
			fw_mgr = flash->get_boot_image (flash);
			break;

		case 1:
			fw_mgr = flash->get_nitro_image (flash);
			break;

		default:
			return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	if (fw_mgr == NULL) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	status = fw_mgr->get_fw_version (fw_mgr, version,
		OVERLAKE_PROTOCOL_MAX_SOC_FW_VERSION_LENGTH (request));
	if (status != 0) {
		return status;
	}

	request->length = overlake_protocol_soc_fw_version_response_length ((strlen (version) + 1));

	return 0;
}

/**
 * Process get public key packet
 *
 * @param attestation Attestation responder instance to utilize
 * @param request Get public key request to process
 * @param x509 X.509 engine to use for certificate parsing
 *
 * @return 0 if request processed successfully or an error code.
 */
int overlake_protocol_get_public_key (struct attestation_responder *attestation,
	struct cmd_interface_msg *request, const struct x509_engine *x509)
{
	CERBERUS_PROTOCOL_CMD (rq, struct overlake_protocol_get_public_key_request_packet*, request);
	CERBERUS_PROTOCOL_CMD (hdr, struct overlake_protocol_get_public_key_response_header*, request);
	struct x509_certificate x509_cert;
	struct der_cert cert;
	uint8_t *key = NULL;
	uint8_t slot_num;
	uint8_t cert_num;
	uint16_t max_length;
	uint16_t offset;
	uint16_t length;
	size_t key_len;
	int status;

	if (request->length !=
		CERBERUS_PROTOCOL_CMD_LEN (struct overlake_protocol_get_public_key_request_packet)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	slot_num = rq->slot_num;
	cert_num = rq->cert_num;
	length = rq->length;
	offset = rq->offset;

	if (slot_num > ATTESTATION_AUX_SLOT_NUM) {
		return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	/* TODO: Use the message max provided with the request. */
	max_length = MCTP_BASE_PROTOCOL_MAX_MESSAGE_BODY -
		CERBERUS_PROTOCOL_CMD_LEN (struct overlake_protocol_get_public_key_response_header);

	status = attestation->get_certificate (attestation, slot_num, cert_num, &cert);
	if (status != 0) {
		return status;
	}

	status = x509->load_certificate (x509, &x509_cert, cert.cert, cert.length);
	if (status != 0) {
		return status;
	}

	status = x509->get_public_key (x509, &x509_cert, &key, &key_len);
	x509->release_certificate (x509, &x509_cert);

	if (status != 0) {
		return status;
	}

	if (offset >= key_len) {
		status = CMD_HANDLER_INVALID_ARGUMENT;
		goto exit;
	}

	if ((length == 0) || (length > max_length)) {
		length = max_length;
	}

	length = min (length, key_len - offset);

	hdr->slot_num = slot_num;
	hdr->cert_num = cert_num;

	memcpy (request->data +
		CERBERUS_PROTOCOL_CMD_LEN (struct overlake_protocol_get_public_key_response_header),
		&key[offset], length);

	request->length =
		CERBERUS_PROTOCOL_CMD_LEN (struct overlake_protocol_get_public_key_response_header) +
		length;

exit:
	platform_free (key);

	return status;
}

/**
 * Process decrypt packet
 *
 * @param attestation Attestation responder instance to utilize
 * @param request Decrypt request to process
 *
 * @return 0 if request processed successfully or an error code.
 */
int overlake_protocol_decrypt_payload (struct attestation_responder *attestation,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_decrypt_payload *rq =
		(struct overlake_protocol_decrypt_payload*) request->data;
	struct overlake_protocol_decrypt_payload_response *rsp =
		(struct overlake_protocol_decrypt_payload_response*) request->data;
	uint8_t *label = NULL;
	uint8_t output[AUX_ATTESTATION_KEY_BYTES];
	enum hash_type pad_hash;
	bool rsa = true;

	int status;

	if ((request->length <= sizeof (struct overlake_protocol_decrypt_payload)) ||
		(request->length != overlake_protocol_decrypt_payload_total_length (rq))) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	switch (rq->algorithm) {
		case OVERLAKE_DECRYPTION_ALGORITHM_RSA_OAEP_SHA1:
			pad_hash = HASH_TYPE_SHA1;
			break;

		case OVERLAKE_DECRYPTION_ALGORITHM_RSA_OAEP_SHA256:
			pad_hash = HASH_TYPE_SHA256;
			break;

		case OVERLAKE_DECRYPTION_ALGORITHM_ECDH:
			rsa = false;
			pad_hash = HASH_TYPE_SHA1;
			break;

		case OVERLAKE_DECRYPTION_ALGORITHM_ECDH_SHA256:
			rsa = false;
			pad_hash = HASH_TYPE_SHA256;
			break;

		default:
			return CMD_HANDLER_UNSUPPORTED_INDEX;
	}

	if (rq->label_len != 0) {
		label = overlake_protocol_decrypt_label (rq);
	}

	if (rsa) {
		status = attestation->aux_decrypt (attestation, overlake_protocol_decrypt_data (rq),
			rq->decrypt_len, label, rq->label_len, pad_hash, output, sizeof (output));
	}
	else {
		status = attestation->generate_ecdh_seed (attestation, overlake_protocol_decrypt_data (rq),
			rq->decrypt_len, (pad_hash == HASH_TYPE_SHA256), output, sizeof (output));
	}
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	memcpy (&rsp->data, output, status);
	request->length = overlake_protocol_decrypted_response_length (status);

	return 0;
}

/* Process a request to get the CRMU Logs.
 *
 * @param flash Overlake flash manager to utilize.
 * @param request SoC crmu log request to process.
 *
 * @return 0 if the request was processed successfully or an error code.
 */
int overlake_protocol_get_soc_crmu_log (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request)
{
	struct overlake_protocol_get_debug_log *rq =
		(struct overlake_protocol_get_debug_log*) request->data;
	struct overlake_protocol_get_debug_log_response *rsp =
		(struct overlake_protocol_get_debug_log_response*) request->data;
	int read_len;

	if (request->length != sizeof (struct overlake_protocol_get_debug_log)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	if ((rq->length == 0) || (rq->length > OVERLAKE_PROTOCOL_MAX_DEBUG_LOG_DATA (request))) {
		rq->length = OVERLAKE_PROTOCOL_MAX_DEBUG_LOG_DATA (request);
	}

	read_len = flash->get_soc_crmu_log (flash, rq->offset, overlake_protocol_debug_log (rsp),
		rq->length);
	if (ROT_IS_ERROR (read_len)) {
		return read_len;
	}

	request->length = overlake_protocol_debug_log_length (read_len);

	return 0;
}
