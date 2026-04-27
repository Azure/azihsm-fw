// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cerberus_protocol_master_commands.h"
#include "cmd_interface/cerberus_protocol_optional_commands.h"
#include "cmd_interface/cerberus_protocol_required_commands.h"
#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/cmd_interface_overlake.h"
#include "cmd_interface/overlake_protocol.h"
#include "cmd_interface/overlake_protocol_commands.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "host_fw/overlake_host_id.h"
#include "manifest/pfm/pfm.h"
#include "tpm/tpm.h"


/**
 * Process get certificate packet.  Maintain old operation to keep compatibility with SoC fTPM FW.
 *
 * @param attestation Attestation manager instance to utilize
 * @param request Get certificate request to process
 *
 * @return 0 if request processed successfully or an error code.
 */
static int cmd_interface_overlake_get_certificate (struct attestation_responder *attestation,
	struct cmd_interface_msg *request)
{
	struct cerberus_protocol_get_certificate *rq =
		(struct cerberus_protocol_get_certificate*) request->data;
	struct cerberus_protocol_get_certificate_response *rsp =
		(struct cerberus_protocol_get_certificate_response*) request->data;
	struct der_cert cert;
	uint8_t slot_num;
	uint8_t cert_num;
	uint16_t offset;
	uint16_t length;
	int status;

	if (request->length != sizeof (struct cerberus_protocol_get_certificate)) {
		return CMD_HANDLER_BAD_LENGTH;
	}

	slot_num = rq->slot_num;
	cert_num = rq->cert_num;
	length = rq->length;
	offset = rq->offset;

	if (slot_num > ATTESTATION_MAX_SLOT_NUM) {
		return CMD_HANDLER_OUT_OF_RANGE;
	}

	status = attestation->get_certificate (attestation, slot_num, cert_num, &cert);
	if (status != 0) {
		return status;
	}

	if (offset < cert.length) {
		if ((length == 0) || (length > CERBERUS_PROTOCOL_MAX_CERT_DATA (request))) {
			length = CERBERUS_PROTOCOL_MAX_CERT_DATA (request);
		}

		length = min (length, cert.length - offset);
		memcpy (cerberus_protocol_certificate (rsp), &cert.cert[offset], length);
	}
	else {
		length = 0;
	}

	rsp->slot_num = slot_num;
	rsp->cert_num = cert_num;

	request->length = cerberus_protocol_get_certificate_response_length (length);

	return 0;
}

static int cmd_interface_overlake_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_overlake *interface = (const struct cmd_interface_overlake*) intf;
	bool reject_secure_commands = false;
	uint8_t command_id;
	uint8_t command_set;
	int status;

	status = cmd_interface_process_cerberus_protocol_message (intf, request, &command_id,
		&command_set, true, true);
	if (status != 0) {
		return status;
	}

	if (!request->is_encrypted) {
		status = interface->base.session->is_session_established (interface->base.session,
			request->source_eid);
		if (ROT_IS_ERROR (status)) {
			return status;
		}
		else if (status) {
			reject_secure_commands = true;
		}
	}

	if (!reject_secure_commands) {
		status = interface->base.session->get_pairing_state (interface->base.session,
			request->source_eid);
		if (ROT_IS_ERROR (status)) {
			return status;
		}

		if (status == SESSION_PAIRING_STATE_NOT_PAIRED) {
			reject_secure_commands = true;
		}
	}

	switch (command_id) {
		case CERBERUS_PROTOCOL_GET_DEVICE_CAPABILITIES:
			status = cerberus_protocol_get_device_capabilities (interface->device_manager, request);
			break;

		case OVERLAKE_PROTOCOL_GET_STORAGE: {
			bool mask_data_error = (interface->board_type == OVERLAKE_GLACIER_PEAK) ? false : true;

			if (reject_secure_commands) {
				return CMD_HANDLER_CMD_SHOULD_BE_ENCRYPTED;
			}

			status = overlake_protocol_get_storage (interface->tpm, mask_data_error, request);
			break;
		}

		case OVERLAKE_PROTOCOL_SET_STORAGE:
			if (reject_secure_commands) {
				return CMD_HANDLER_CMD_SHOULD_BE_ENCRYPTED;
			}

			status = overlake_protocol_set_storage (interface->tpm, request);
			break;

		case OVERLAKE_PROTOCOL_READ_DATA:
			if (reject_secure_commands) {
				return CMD_HANDLER_CMD_SHOULD_BE_ENCRYPTED;
			}

			status = overlake_protocol_read_data (interface->flash, request);
			break;

		case OVERLAKE_PROTOCOL_CLEAR_DATA:
			if (reject_secure_commands) {
				return CMD_HANDLER_CMD_SHOULD_BE_ENCRYPTED;
			}

			status = overlake_protocol_clear_data (interface->flash, request);
			break;

		case OVERLAKE_PROTOCOL_STORE_DATA:
			if (reject_secure_commands) {
				return CMD_HANDLER_CMD_SHOULD_BE_ENCRYPTED;
			}

			status = overlake_protocol_store_data (interface->flash, request);
			break;

		case OVERLAKE_PROTOCOL_SIGN_DATA:
			if (reject_secure_commands) {
				return CMD_HANDLER_CMD_SHOULD_BE_ENCRYPTED;
			}

			status = overlake_protocol_sign_data (interface->ecc, interface->riot, interface->hash,
				request);
			request->crypto_timeout = true;
			break;

		case OVERLAKE_PROTOCOL_SOC_INIT_FW_UPDATE: {
			const struct host_fw_cmd_interface *host_fw_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->boot, interface->nitro, interface->fpga
			};

			status = overlake_protocol_host_fw_init (host_fw_cmd, OVERLAKE_HOST_NUM_PORTS, false,
				CMD_INTERFACE_OVERLAKE_CHANNEL_SPI_SOC, request);
			break;
		}

		case OVERLAKE_PROTOCOL_SOC_UPDATE_FW: {
			const struct host_fw_cmd_interface *host_fw_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->boot, interface->nitro, interface->fpga
			};

			status = overlake_protocol_host_fw_update (host_fw_cmd, OVERLAKE_HOST_NUM_PORTS,
				request);
			break;
		}

		case OVERLAKE_PROTOCOL_GET_SOC_UPDATE_STATUS: {
			const struct host_fw_cmd_interface *host_fw_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->boot, interface->nitro, interface->fpga
			};

			status = overlake_protocol_get_host_fw_update_status (host_fw_cmd,
				OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case OVERLAKE_PROTOCOL_TPM_CLEAR:
			if (reject_secure_commands) {
				return CMD_HANDLER_CMD_SHOULD_BE_ENCRYPTED;
			}

			status = overlake_protocol_tpm_clear (interface->tpm, request);
			break;

		case CERBERUS_PROTOCOL_INIT_PFM_UPDATE: {
			const struct manifest_cmd_interface *pfm_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_0, interface->pfm_1, interface->pfm_2
			};

			status = cerberus_protocol_pfm_update_init (pfm_cmd, OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case CERBERUS_PROTOCOL_PFM_UPDATE: {
			const struct manifest_cmd_interface *pfm_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_0, interface->pfm_1, interface->pfm_2
			};

			status = cerberus_protocol_pfm_update (pfm_cmd, OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE: {
			const struct manifest_cmd_interface *pfm_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_0, interface->pfm_1, interface->pfm_2
			};

			status = cerberus_protocol_pfm_update_complete (pfm_cmd, OVERLAKE_HOST_NUM_PORTS,
				request);
			break;
		}

		case CERBERUS_PROTOCOL_GET_PFM_ID: {
			const struct pfm_manager *pfm_mgr[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_manager_0, interface->pfm_manager_1, interface->pfm_manager_2
			};

			status = cerberus_protocol_get_pfm_id (pfm_mgr, OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW: {
			const struct pfm_manager *pfm_mgr[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_manager_0, interface->pfm_manager_1, interface->pfm_manager_2
			};

			status = cerberus_protocol_get_pfm_fw (pfm_mgr, OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case CERBERUS_PROTOCOL_GET_UPDATE_STATUS: {
			const struct manifest_cmd_interface *pfm_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_0, interface->pfm_1, interface->pfm_2
			};

			status = cerberus_protocol_get_update_status (NULL, OVERLAKE_HOST_NUM_PORTS, pfm_cmd,
				NULL, NULL, NULL, NULL, NULL, NULL, request);
			break;
		}

		case CERBERUS_PROTOCOL_GET_DIGEST:
			status = cerberus_protocol_get_certificate_digest (interface->attestation,
				interface->base.session, request);
			break;

		case CERBERUS_PROTOCOL_GET_CERTIFICATE:
			status = cmd_interface_overlake_get_certificate (interface->attestation, request);
			break;

		case CERBERUS_PROTOCOL_ATTESTATION_CHALLENGE:
			status = cerberus_protocol_get_challenge_response (interface->attestation,
				interface->base.session, request);
			break;

		case CERBERUS_PROTOCOL_EXCHANGE_KEYS:
			status = cerberus_protocol_key_exchange (interface->base.session, request,
				request->is_encrypted);
			break;

		case CERBERUS_PROTOCOL_SESSION_SYNC:
			status = cerberus_protocol_session_sync (interface->base.session, request,
				request->is_encrypted);
			break;

		case OVERLAKE_PROTOCOL_GET_PUBLIC_KEY:
			if (reject_secure_commands) {
				return CMD_HANDLER_CMD_SHOULD_BE_ENCRYPTED;
			}

			status = overlake_protocol_get_public_key (interface->attestation, request,
				interface->x509);
			break;

		case OVERLAKE_PROTOCOL_DECRYPT_PAYLOAD:
			if (reject_secure_commands) {
				return CMD_HANDLER_CMD_SHOULD_BE_ENCRYPTED;
			}

			status = interface->overlake_bgnd->decrypt_payload (interface->overlake_bgnd, request);
			request->crypto_timeout = true;
			break;

		default:
			return CMD_HANDLER_UNKNOWN_REQUEST;
	}

	if (status == 0) {
		status = cmd_interface_prepare_response (&interface->base, request);
	}

	return status;
}

/**
 * Initialize Overlake command interface instance
 *
 * @param intf The Overlake command interface instance to initialize
 * @param device_manager Device manager
 * @param tpm TPM instance tied to interface
 * @param boot_fw Command handler for SoC boot firmware
 * @param nitro_fw Command handler for SoC Nitro firmware
 * @param fpga_fw Command handler for Cyclone V FPGA firmware.
 * @param pfm_0 Command handler for PFM port 0
 * @param pfm_1 Command handler for PFM port 1
 * @param pfm_2 Command handler for PFM port 2.
 * @param pfm_manager_0 PFM manager instance for port 0
 * @param pfm_manager_1 PFM manager instance for port 1
 * @param pfm_manager_2 PFM manager instance for port 2.
 * @param attestation Slave attestation manager
 * @param background Context for executing long-running operations in the background.
 * @param x509 X.509 engine for certificate parsing.
 * @param pcr Manager for system PCRs.
 * @param responder_attestation Slave attestation manager.
 * @param session Session manager for channel encryption.
 * @param hash Hash engine for data signing.
 * @param ecc ECC engine for data signing.
 * @param riot Manager for RIoT device keys.
 * @param flash The flash block storage to store generic data.
 * @param board_type The Overlake board type.
 *
 * @return Initialization status, 0 if success or an error code.
 */
int cmd_interface_overlake_init (struct cmd_interface_overlake *intf,
	struct device_manager *device_manager, struct tpm *tpm,
	const struct host_fw_cmd_interface *boot_fw, const struct host_fw_cmd_interface *nitro_fw,
	const struct host_fw_cmd_interface *fpga_fw, const struct manifest_cmd_interface *pfm_0,
	const struct manifest_cmd_interface *pfm_1, const struct manifest_cmd_interface *pfm_2,
	const struct pfm_manager *pfm_manager_0, const struct pfm_manager *pfm_manager_1,
	const struct pfm_manager *pfm_manager_2, struct attestation_responder *attestation,
	const struct cmd_background *background, struct overlake_background *overlake_bgnd,
	const struct x509_engine *x509, struct pcr_store *pcr, struct session_manager *session,
	const struct hash_engine *hash, const struct ecc_engine *ecc,
	const struct riot_key_manager *riot, const struct flash_store *flash,
	enum overlake_board_type board_type)
{
	if ((intf == NULL) || (boot_fw == NULL) || (nitro_fw == NULL) ||
		(pfm_0 == NULL) || (pfm_manager_0 == NULL) || (attestation == NULL) ||
		(background == NULL) || (overlake_bgnd == NULL) || (x509 == NULL) || (pcr == NULL) ||
		(device_manager == NULL) || (hash == NULL) || (ecc == NULL) || (riot == NULL)) {
		return CMD_HANDLER_INVALID_ARGUMENT;
	}

	if ((board_type != OVERLAKE_CASTLE_PEAK) &&
		((flash == NULL) || (session == NULL) || (tpm == NULL))) {
		return CMD_HANDLER_INVALID_ARGUMENT;
	}

	if ((board_type == OVERLAKE_CELESTIAL_PEAK) &&
		((nitro_fw == NULL) || (pfm_1 == NULL) || (pfm_manager_1 == NULL))) {
		return CMD_HANDLER_INVALID_ARGUMENT;
	}

	if ((board_type == OVERLAKE_GLACIER_PEAK) &&
		((fpga_fw == NULL) || (pfm_2 == NULL) || (pfm_manager_2 == NULL))) {
		return CMD_HANDLER_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_overlake));

	intf->base.process_request = cmd_interface_overlake_process_request;

	intf->base.session = session;

	intf->tpm = tpm;
	intf->boot = boot_fw;
	intf->nitro = nitro_fw;
	intf->pfm_0 = pfm_0;
	intf->pfm_1 = pfm_1;
	intf->pfm_manager_0 = pfm_manager_0;
	intf->pfm_manager_1 = pfm_manager_1;
	intf->attestation = attestation;
	intf->background = background;
	intf->overlake_bgnd = overlake_bgnd;
	intf->x509 = x509;
	intf->pcr = pcr;
	intf->device_manager = device_manager;
	intf->hash = hash;
	intf->ecc = ecc;
	intf->riot = riot;
	intf->flash = flash;
	intf->fpga = fpga_fw;
	intf->pfm_2 = pfm_2;
	intf->pfm_manager_2 = pfm_manager_2;
	intf->board_type = board_type;

	return 0;
}

/**
 * Deinitialize Overlake command interface instance
 *
 * @param intf The Overlake command interface instance to deinitialize
 */
void cmd_interface_overlake_deinit (struct cmd_interface_overlake *intf)
{
	if (intf != NULL) {
		memset (intf, 0, sizeof (struct cmd_interface_overlake));
	}
}
