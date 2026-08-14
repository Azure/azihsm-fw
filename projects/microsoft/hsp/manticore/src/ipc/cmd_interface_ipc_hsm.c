// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/type_cast.h"
#include "common/unused.h"
#include "ipc/cmd_interface_ipc_hsm.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_message.h"
#include "logging/manticore_logging.h"


/**
 * Process the Get Certificate Chain Length message request received over the IPC Channel.
 *
 * @param cmd_interface_ipc_hsm A pointer to the command interface to process the request.
 * @param request The message containing the request.  This will be updated with an appropriate
 * response.
 *
 * @return 0 if the request was successfully processed or an error code.
 */
static int cmd_interface_ipc_hsm_get_cert_chain_len (
	const struct cmd_interface_ipc_hsm *cmd_interface_ipc_hsm, struct cmd_interface_msg *request)
{
	struct ipc_message_get_cert_chain_len *resp =
		(struct ipc_message_get_cert_chain_len*) request->data;
	struct der_cert cert;
	uint8_t digests[SHA256_HASH_LENGTH * 4];	// There will be at most 4 certificates.
	int digests_length;
	size_t i;
	int status;

	digests_length =
		cmd_interface_ipc_hsm->attestation->get_digests (cmd_interface_ipc_hsm->attestation, 0,
		digests, sizeof (digests), &resp->payload.num_certs);
	if (ROT_IS_ERROR (digests_length)) {
		return digests_length;
	}

	if (resp->payload.num_certs < 2) {
		/* There must be at least 2 certs, Device ID and Alias. */
		return CMD_INTERFACE_IPC_HSM_CERT_CHAIN_TOO_SMALL;
	}

	resp->payload.num_certs--;	// Ignore the Alias certificate.
	if (resp->payload.num_certs > IPC_MESSAGE_GET_CERT_CHAIN_LEN_MAX_CERTS) {
		return CMD_INTERFACE_IPC_HSM_GET_CERT_CHAIN_LEN_TOO_MANY;
	}

	status = cmd_interface_ipc_hsm->hash->calculate_sha256 (cmd_interface_ipc_hsm->hash, digests,
		(digests_length - SHA256_HASH_LENGTH), resp->payload.digest, sizeof (resp->payload.digest));
	if (status != 0) {
		return status;
	}

	for (i = 0; i < resp->payload.num_certs; i++) {
		status =
			cmd_interface_ipc_hsm->attestation->get_certificate (cmd_interface_ipc_hsm->attestation,
			0, i, &cert);
		if (status != 0) {
			return status;
		}

		resp->payload.len[i] = cert.length;
	}

	resp->header.data_length = ipc_message_get_cert_chain_len_data_length (resp->payload.num_certs);

	return 0;
}

/**
 * Process the Get Certificate message request received over the IPC Channel.
 *
 * @param cmd_interface_ipc_hsm A pointer to the command interface to process the request.
 * @param request The message containing the request.  This will be updated with an appropriate
 * response.
 *
 * @return 0 if the request was successfully processed or an error code.
 */
static int cmd_interface_ipc_hsm_get_get_cert (
	const struct cmd_interface_ipc_hsm *cmd_interface_ipc_hsm, struct cmd_interface_msg *request)
{
	struct ipc_message_get_cert_payload *payload =
		(struct ipc_message_get_cert_payload*) request->payload;
	uint8_t *d_tcm_addr;
	uint8_t slot_num = 0;
	struct der_cert cert = {0, 0};
	int status = 0;

	status =
		cmd_interface_ipc_hsm->attestation->get_certificate (cmd_interface_ipc_hsm->attestation,
		slot_num, payload->cert_num, &cert);
	if (status != 0) {
		return status;
	}

	if (cert.length <= payload->buf_size) {
		/* Map the soc_address */
		status = cmd_interface_ipc_hsm->dmb->map_soc_address (cmd_interface_ipc_hsm->dmb,
			payload->buf_addr, cert.length, HSP_DMB_ACCESS_WRITE, (void**) &d_tcm_addr);
		if (status != 0) {
			return status;
		}

		memcpy (d_tcm_addr, cert.cert, cert.length);
		cmd_interface_ipc_hsm->dmb->unmap_soc_address (cmd_interface_ipc_hsm->dmb, d_tcm_addr);
	}
	else {
		status = CMD_INTERFACE_IPC_HSM_GET_CERT_LEN_BUF_TOO_SMALL;
	}

	/* Update the cert_size for the requester, regardless of the buf_size and dmb mapping */
	payload->cert_size = cert.length;

	return status;
}

/**
 * Process the IPC Message and validate the received IPC message.
 *
 * @param cmd_interface_ipc_hsm The command interface that will process the request.
 * @param request The request data to process. This will be updated to contain a response, if
 * necessary.
 */
static void cmd_interface_ipc_hsm_process_ipc_message (
	const struct cmd_interface_ipc_hsm *cmd_interface_ipc_hsm, struct cmd_interface_msg *request)
{
	struct ipc_message_header *header;
	int status = 0;

	header = (struct ipc_message_header*) request->data;

	switch (header->opcode)	{
		case IPC_MESSAGE_OPCODE_GET_CERT_CHAIN_LEN:
			status = cmd_interface_ipc_hsm_get_cert_chain_len (cmd_interface_ipc_hsm, request);
			break;

		case IPC_MESSAGE_OPCODE_GET_CERT:
			status = cmd_interface_ipc_hsm_get_get_cert (cmd_interface_ipc_hsm, request);
			break;

		default:
			status = CMD_INTERFACE_IPC_HSM_INVALID_IPC_OPCODE;
			break;
	}

	ipc_message_build_response (request, MANTICORE_LOGGING_IPC_HSM_INTERFACE, status);
}

int cmd_interface_ipc_hsm_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_ipc_hsm *cmd_interface_ipc_hsm = TO_DERIVED_TYPE (intf,
		const struct cmd_interface_ipc_hsm, base);

	if ((intf == NULL) || (request == NULL)) {
		return CMD_INTERFACE_IPC_HSM_INVALID_ARGUMENT;
	}

	cmd_interface_ipc_hsm_process_ipc_message (cmd_interface_ipc_hsm, request);

	return 0;
}

/**
 * Initialize an instance of Command Interface for HSM core
 *
 * @param cmd_interface_hsm - A pointer to the initialized struct cmd_interface_ipc_hsm
 * @param dmb - A pointer to struct hsp_dmb used to map and unmap memory regions
 * @param attestation A pointer to an instance of struct struct attestation_responder
 * @param hash Hash engine to use for certificate handling.
 *
 * @return 0 if the request was successfully processed or an error code.
 */
int cmd_interface_ipc_hsm_init (struct cmd_interface_ipc_hsm *cmd_interface_hsm,
	const struct hsp_dmb *dmb, struct attestation_responder *attestation,
	const struct hash_engine *hash)
{
	if ((cmd_interface_hsm == NULL) || (dmb == NULL) || (attestation == NULL) || (hash == NULL)) {
		return CMD_INTERFACE_IPC_HSM_INVALID_ARGUMENT;
	}

	memset (cmd_interface_hsm, 0, sizeof (struct cmd_interface_ipc_hsm));

	cmd_interface_hsm->base.process_request = cmd_interface_ipc_hsm_process_request;

	cmd_interface_hsm->dmb = dmb;
	cmd_interface_hsm->attestation = attestation;
	cmd_interface_hsm->hash = hash;

	return 0;
}

/**
 * Release a previously initialized IPC HSM Command Interface and free any associated resources
 *
 * @param cmd_interface_hsm - A pointer to an un-initialized IPC HSM Command Interface object
 *
 * @return None.
 */
void cmd_interface_ipc_hsm_release (const struct cmd_interface_ipc_hsm *cmd_interface_hsm)
{
	UNUSED (cmd_interface_hsm);
}
