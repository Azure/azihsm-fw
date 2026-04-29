// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/buffer_util.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "crypto/mbedtls_compat.h"
#include "ipc/cmd_interface_ipc_hsm.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_message.h"
#include "logging/manticore_logging.h"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"

/**
 * Convert the supported ephemeral key types to key size in bits.
 */
#define CMD_INTERFACE_IPC_HSM_SUPPORTED_EPHEMERAL_KEY_SIZE(key_type) ((key_type + 2) * 1024)

/**
 * Generate the RSA key context from the DER data and store RSA key component data in little-endian
 * format.
 *
 * @param key_der  A pointer for input RSA key buffer in DER format.
 * @param key_length Input RSA key length in bytes.
 * @param rsa_key_component A pointer to output RSA key components. RSA key will be copied in RAW
 * little-endian format as defined by struct ipc_message_rsa_key_component
 *
 * @return 0 if the request was successfully processed or an error code.
 */
static int cmd_interface_ipc_hsm_rsa_get_key_context (uint8_t *key_der, size_t key_length,
	struct ipc_message_rsa_key_component *rsa_key_component)
{
	mbedtls_rsa_context *rsa;
	mbedtls_pk_context pk;
	int status;

	if ((key_der == NULL) || (rsa_key_component == NULL)) {
		return CMD_INTERFACE_IPC_HSM_INVALID_ARGUMENT;
	}

	// Initialize the PK context
	mbedtls_pk_init (&pk);

#if MBEDTLS_IS_VERSION_3
	/* Parse the private key in DER format from the buffer.  No RNG is needed for RSA keys, despite
	 * the API description saying it's required.  If one is desired (or becomes required), an
	 * rng_engine can be provided to this instance during init and used along with
	 * rng_mbedtls_rng_callback(). */
	status = mbedtls_pk_parse_key (&pk, key_der, key_length, NULL, 0, NULL, NULL);
#else
	status = mbedtls_pk_parse_key (&pk, key_der, key_length, NULL, 0);
#endif
	if (status != 0) {
		goto err_rsa_free;
	}

	if (mbedtls_pk_get_type (&pk) != MBEDTLS_PK_RSA) {
		status = CMD_INTERFACE_IPC_HSM_KEY_NOT_RSA_TYPE;
		goto err_rsa_free;
	}

	// Get the RSA context from the PK context
	rsa = mbedtls_pk_rsa (pk);
	if (!rsa) {
		status = CMD_INTERFACE_IPC_HSM_FAILED_TO_CONVERT_RSA_CONTEXT;
		goto err_rsa_free;
	}

	/* This function exports core parameters of an RSA key in raw big-endian binary format */
	status = mbedtls_rsa_export_raw (rsa, rsa_key_component->n, IPC_MESSAGE_RSA_2K_MODULUS_LEN,
		NULL, 0, NULL, 0, rsa_key_component->d, IPC_MESSAGE_RSA_2K_PRIV_EXPONENT_LEN,
		rsa_key_component->e, IPC_MESSAGE_RSA_2K_PUB_EXPONENT_LEN);
	if (status != 0) {
		goto err_rsa_free;
	}

	/* Reverse the RSA components buffer and covert in little-endian format */
	buffer_reverse (rsa_key_component->d, IPC_MESSAGE_RSA_2K_PRIV_EXPONENT_LEN);
	buffer_reverse (rsa_key_component->n, IPC_MESSAGE_RSA_2K_MODULUS_LEN);
	buffer_reverse (rsa_key_component->e, IPC_MESSAGE_RSA_2K_PUB_EXPONENT_LEN);

err_rsa_free:
	mbedtls_pk_free (&pk);

	return status;
}

/**
 * Process the RSA Key Generation message request received over the IPC Channel.
 *
 * @param cmd_interface_ipc_hsm A pointer to the command interface to process the request.
 * @param request The message containing the request.  This will be updated with an appropriate
 * response.
 *
 * @return 0 if the request was successfully processed or an error code.
 */
static int cmd_interface_ipc_hsm_rsa_key_gen (
	const struct cmd_interface_ipc_hsm *cmd_interface_ipc_hsm, struct cmd_interface_msg *request)
{
	struct ipc_message_rsa_key_gen_payload *payload;
	struct ipc_message_rsa_key_component *rsa_key_component;
	size_t key_length;
	size_t key_size;
	int status;

	payload = (struct ipc_message_rsa_key_gen_payload*) request->payload;

	/* Validate the requested key type matches the configured key size */
	key_size = ephemeral_key_manager_get_key_size (cmd_interface_ipc_hsm->key_manager);
	if (key_size !=
		(size_t) CMD_INTERFACE_IPC_HSM_SUPPORTED_EPHEMERAL_KEY_SIZE (payload->key_type)) {
		return CMD_INTERFACE_IPC_HSM_INVALID_KEY_TYPE;
	}

	/* Map the soc_address */
	status = cmd_interface_ipc_hsm->dmb->map_soc_address (cmd_interface_ipc_hsm->dmb,
		(uint64_t) payload->key_address, sizeof (struct ipc_message_rsa_key_component),
		HSP_DMB_ACCESS_WRITE, (void**) &rsa_key_component);
	if (status != 0) {
		return status;
	}

	/* Request for the ephemeral key */
	status = ephemeral_key_manager_get_key (cmd_interface_ipc_hsm->key_manager,
		payload->function_id, cmd_interface_ipc_hsm->key, cmd_interface_ipc_hsm->key_size,
		&key_length);
	if (status != 0) {
		goto err_unmap_hsm_dmb;
	}

	memset (rsa_key_component, 0x00, sizeof (*rsa_key_component));

	/* Convert read data in RSA context */
	status = cmd_interface_ipc_hsm_rsa_get_key_context (cmd_interface_ipc_hsm->key, key_length,
		rsa_key_component);

	/* Zeroize key buffer after use */
	buffer_zeroize (cmd_interface_ipc_hsm->key, cmd_interface_ipc_hsm->key_size);

err_unmap_hsm_dmb:
	cmd_interface_ipc_hsm->dmb->unmap_soc_address (cmd_interface_ipc_hsm->dmb, rsa_key_component);

	return status;
}

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

	/* Validate IPC message opcode */
	switch (header->opcode)	{
		case IPC_MESSAGE_OPCODE_RSA_KEY_GEN:
			status = cmd_interface_ipc_hsm_rsa_key_gen (cmd_interface_ipc_hsm, request);
			break;

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
 * @param key_manager - A pointer to an implementation of struct ephemeral_key_manager
 * @param key A pointer to a buffer to read the key from the flash.
 * @param key_size Size of the key buffer.
 * @param attestation A pointer to an instance of struct struct attestation_responder
 * @param hash Hash engine to use for certificate handling.
 *
 * @return 0 if the request was successfully processed or an error code.
 */
int cmd_interface_ipc_hsm_init (struct cmd_interface_ipc_hsm *cmd_interface_hsm,
	const struct hsp_dmb *dmb, const struct ephemeral_key_manager *key_manager, uint8_t *key,
	size_t key_size, struct attestation_responder *attestation, const struct hash_engine *hash)
{
	if ((cmd_interface_hsm == NULL) || (dmb == NULL) || (key_manager == NULL) || (key == NULL) ||
		(attestation == NULL) || (hash == NULL)) {
		return CMD_INTERFACE_IPC_HSM_INVALID_ARGUMENT;
	}

	memset (cmd_interface_hsm, 0, sizeof (struct cmd_interface_ipc_hsm));

	cmd_interface_hsm->base.process_request = cmd_interface_ipc_hsm_process_request;

	cmd_interface_hsm->dmb = dmb;
	cmd_interface_hsm->key_manager = key_manager;
	cmd_interface_hsm->key = key;
	cmd_interface_hsm->key_size = key_size;
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
