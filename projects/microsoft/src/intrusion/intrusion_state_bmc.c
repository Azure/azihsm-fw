// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <string.h>
#include "common/buffer_util.h"
#include "common/type_cast.h"
#include "intrusion/intrusion_logging.h"
#include "intrusion/intrusion_manager_async.h"
#include "intrusion/intrusion_state_bmc.h"
#include "msft_protocol/bmc_commands.h"
#include "msft_protocol/msft_mctp_protocol.h"


/**
 * The nonce length for sending intrusion data to BMC.
 */
#define INTRUSION_STATE_BMC_NONCE_LENGTH		BMC_CHASSIS_INTRUSION_NONCE_LEN


/**
 * Function to send BMC protocol request and wait for a response using the MSFT transport.
 * The response payload will be copied to the instance msg_buffer.
 *
 * @param intrusion_bmc BMC intrusion state instance to utilize.
 * @param request The request message to send.
 * @param response The response message structure to receive the response.
 *
 * @return 0 if successful or error code otherwise
 */
static int intrusion_state_bmc_send_request_and_get_response (
	struct intrusion_state_bmc *intrusion_bmc, struct cmd_interface_msg *request,
	struct cmd_interface_msg *response)
{
	int status = 0;
	uint32_t timeout_ms;

	intrusion_bmc->request_status = INTRUSION_STATE_BMC_REQUEST_WAITING;

	timeout_ms = device_manager_get_mctp_ctrl_timeout (intrusion_bmc->device_mgr);

	status = intrusion_bmc->msft_transport->send_request_message (intrusion_bmc->msft_transport,
		request, timeout_ms, response);
	if (status != 0) {
		return status;
	}

	if (sizeof (intrusion_bmc->msg_buffer) < response->payload_length) {
		return MSG_TRANSPORT_RESPONSE_TOO_LARGE;
	}

	memmove (intrusion_bmc->msg_buffer, response->payload, response->payload_length);

	return 0;
}

static int intrusion_state_bmc_set (struct intrusion_state *intrusion)
{
	struct intrusion_state_bmc *intrusion_bmc = (struct intrusion_state_bmc*) intrusion;
	int status;

	if (intrusion == NULL) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	// Clear the data from flash, which lets the platform know intrusion has been detected.
	status = intrusion_bmc->store->erase (intrusion_bmc->store, intrusion_bmc->id);
	if (status != 0) {
		return status;
	}

	return 0;
}

static int intrusion_state_bmc_clear (struct intrusion_state *intrusion)
{
	struct bmc_chassis_intrusion_store_data_response *rsp;
	struct cmd_interface_msg request;
	struct cmd_interface_msg response;
	struct intrusion_state_bmc *intrusion_bmc = (struct intrusion_state_bmc*) intrusion;
	int request_len = 0;
	int status = 0;
	uint8_t rng_buf[CERBERUS_INTRUSION_STATE_BMC_MAX_LENGTH_BYTES];

	if (intrusion == NULL) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	status = intrusion_bmc->rng->generate_random_buffer (intrusion_bmc->rng, intrusion_bmc->rng_len,
		rng_buf);
	if (status != 0) {
		return status;
	}

	memset (intrusion_bmc->msg_buffer, 0, sizeof (intrusion_bmc->msg_buffer));

	status = msg_transport_create_empty_request (intrusion_bmc->msft_transport,
		intrusion_bmc->msg_buffer, sizeof (intrusion_bmc->msg_buffer), intrusion_bmc->dest_eid,
		&request);
	if (status != 0) {
		return status;
	}

	request_len = bmc_chassis_intrusion_generate_store_data_request (rng_buf,
		intrusion_bmc->rng_len,	request.payload, request.payload_length);
	if (ROT_IS_ERROR (request_len)) {
		return request_len;
	}

	cmd_interface_msg_set_message_payload_length (&request, request_len);

	status = msg_transport_create_empty_response (intrusion_bmc->msg_buffer,
		sizeof (intrusion_bmc->msg_buffer), &response);
	if (status != 0) {
		return status;
	}

	status = intrusion_state_bmc_send_request_and_get_response (intrusion_bmc, &request, &response);
	if (status != 0) {
		return status;
	}

	rsp = (struct bmc_chassis_intrusion_store_data_response*) response.payload;

	if ((rsp->header.completion_code != 0) || (rsp->header.command !=
		BMC_CMD_CHASSIS_INTRUSION_STORE_DATA)) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INTRUSION,
			INTRUSION_LOGGING_STORE_DATA_FAIL, intrusion_bmc->dest_eid,
			rsp->header.completion_code);

		return INTRUSION_STATE_REMOTE_REQUEST_FAILED;
	}

	status = intrusion_bmc->store->write (intrusion_bmc->store, intrusion_bmc->id, rng_buf,
		intrusion_bmc->rng_len);
	if (status != 0) {
		return status;
	}

	return status;
}

static int intrusion_state_bmc_check (struct intrusion_state *intrusion)
{
	int status;
	struct intrusion_state_bmc *intrusion_bmc = (struct intrusion_state_bmc*) intrusion;

	if (intrusion == NULL) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	status = platform_semaphore_post (&intrusion_bmc->run_check);
	if (status != 0) {
		return status;
	}

	return INTRUSION_STATE_CHECK_DEFERRED;
}

/**
 * Initialize the intrusion data flash and RNG.
 *
 * @param intrusion The intrusion state instance being tracked.
 * @param id Block ID of the intrusion data in memory.
 * @param rng_length The number of bytes the stored RNG data will be.
 * @param store The flash store where intrusion data will be saved once sent to BMC.
 * @param rng The RNG engine that will be used to create this data.
 * @param hash The hash engine used to hash the RNG data for comparisons.
 * @param hash_type_bmc The hash type the BMC will use to generate data.
 * @param channel The command channel that will be used for BMC communication.
 * @param source_eid The source's EID.
 * @param dest_eid The EID of the BMC device.
 * @param source_addr The I2C address of the source device.
 * @param dest_addr The I2C address of the BMC.
 * @param challenge_retry_delay The delay period before initiating challenge request retries, in ms.
 * @param msft_transport MSFT message transport to utilize to send and receive requests to BMC.
 * @param device_mgr Device manager to use for retrieving response timeout.
 *
 * @return 0 if initialization successful or an error code.
 */
int intrusion_state_bmc_init (struct intrusion_state_bmc *intrusion, int id, size_t rng_length,
	const struct flash_store *store, const struct rng_engine *rng, const struct hash_engine *hash,
	enum hash_type hash_type_bmc, const struct cmd_channel *channel, uint8_t source_eid,
	uint8_t dest_eid, uint8_t source_addr, uint8_t dest_addr, uint32_t challenge_retry_delay,
	const struct msg_transport *msft_transport, struct device_manager *device_mgr)
{
	int status;

	if ((intrusion == NULL) || (store == NULL) || (rng == NULL) || (hash == NULL) ||
		(channel == NULL) || (msft_transport == NULL) || (device_mgr == NULL) ||
		(rng_length < CERBERUS_INTRUSION_STATE_BMC_MIN_LENGTH_BYTES) ||
		(rng_length > CERBERUS_INTRUSION_STATE_BMC_MAX_LENGTH_BYTES) ||
		(hash_type_bmc < HASH_TYPE_SHA256) || (hash_type_bmc > HASH_TYPE_SHA512)) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	// Set all fields to a known value prior to assignment.
	memset (intrusion, 0, sizeof (struct intrusion_state_bmc));

	status = observable_init (&intrusion->observable);
	if (status != 0) {
		return status;
	}

	status = platform_semaphore_init (&intrusion->run_check);
	if (status != 0) {
		goto error;
	}

	intrusion->id = id;
	intrusion->rng_len = rng_length;
	intrusion->store = store;
	intrusion->rng = rng;
	intrusion->hash = hash;
	intrusion->hash_type_bmc = hash_type_bmc;
	intrusion->channel = channel;
	intrusion->source_addr = source_addr;
	intrusion->dest_addr = dest_addr;
	intrusion->source_eid = source_eid;
	intrusion->dest_eid = dest_eid;
	intrusion->device_mgr = device_mgr;
	intrusion->msft_transport = msft_transport;
	intrusion->challenge_retry_delay = challenge_retry_delay;

	intrusion->base.clear = intrusion_state_bmc_clear;
	intrusion->base.set = intrusion_state_bmc_set;
	intrusion->base.check = intrusion_state_bmc_check;

	return 0;

error:
	observable_release (&intrusion->observable);

	return status;
}

/**
 * Release any memory associated with the intrusion state.
 *
 * @param intrusion The intrusion state instance being released.
 */
void intrusion_state_bmc_release (struct intrusion_state_bmc *intrusion)
{
	if (intrusion) {
		observable_release (&intrusion->observable);
		platform_semaphore_free (&intrusion->run_check);
	}
}

/**
 * Add an observer for intrusion state notifications.
 *
 * @param intrusion The intrusion state instance to register with.
 * @param observer The observer to add.
 *
 * @return 0 if the observer was successfully added or an error code.
 */
int intrusion_state_bmc_add_observer (struct intrusion_state_bmc *intrusion,
	struct intrusion_state_observer *observer)
{
	if (intrusion == NULL) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	return observable_add_observer (&intrusion->observable, observer);
}

/**
 * Remove an observer from intrusion state notifications.
 *
 * @param intrusion The intrusion state instance to deregister from.
 * @param observer The observer to remove.
 *
 * @return 0 if the observer was successfully removed or an error code.
 */
int intrusion_state_bmc_remove_observer (struct intrusion_state_bmc *intrusion,
	struct intrusion_state_observer *observer)
{
	if (intrusion == NULL) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	return observable_remove_observer (&intrusion->observable, observer);
}

/**
 * Check the intrusion state for the device.
 *
 * @param intrusion The intrusion state instance to use.
 *
 * @return 0 if no intrusion, 1 if intrusion detected, or an error code.
 */
int intrusion_state_bmc_background_check (struct intrusion_state_bmc *intrusion)
{
	struct bmc_chassis_intrusion_challenge_data_response *challenge_rsp;
	struct cmd_interface_msg request;
	struct cmd_interface_msg response;
	int data_len_to_hash;
	int hash_len_calculated;
	int request_len = 0;
	int status = 0;
	int retry = 3;
	uint8_t hash_calculated[SHA512_HASH_LENGTH];
	// Use 1 buffer for both, since this makes hashing easier.
	uint8_t nonce_and_stored_data[INTRUSION_STATE_BMC_NONCE_LENGTH +
		CERBERUS_INTRUSION_STATE_BMC_MAX_LENGTH_BYTES];
	uint8_t *received_hash;

	if (intrusion == NULL) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	/* Block until the intrusion task initiates the check flow. */
	status = platform_semaphore_wait (&intrusion->run_check, 0);
	if (status != 0) {
		goto exit;
	}

	/* Run intrusion check with retries on failures not deemed intrusions.
	 * The check reads flash data, generates a nonce, and hashes the flash data. Then, a request is
	 * made to the BMC for a hash of its intrusion data. If the two hashes match, there has been no
	 * detected intrusion. */
	do {
		if (ROT_IS_ERROR (status)) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INTRUSION,
				INTRUSION_LOGGING_CHECK_FAILED, status, 0);

			observable_notify_observers (&intrusion->observable,
				offsetof (struct intrusion_state_observer, on_error));
		}

		status = intrusion->store->read (intrusion->store, intrusion->id,
			&nonce_and_stored_data[INTRUSION_STATE_BMC_NONCE_LENGTH], intrusion->rng_len);
		if (ROT_IS_ERROR (status)) {
			if ((status == FLASH_STORE_CORRUPT_DATA) || (status == FLASH_STORE_NO_DATA)) {
				// These error codes signify an intrusion detection event.
				status = 1;
				goto exit;
			}
			else {
				// This is an error, but not an intrusion.
				continue;
			}
		}
		else if (status != (int) intrusion->rng_len) {
			// Bad data read length during check signals an intrusion event.
			status = 1;
			goto exit;
		}

		status = intrusion->rng->generate_random_buffer (intrusion->rng,
			INTRUSION_STATE_BMC_NONCE_LENGTH, nonce_and_stored_data);
		if (status != 0) {
			continue;
		}

		data_len_to_hash = INTRUSION_STATE_BMC_NONCE_LENGTH + intrusion->rng_len;
		hash_len_calculated = hash_calculate (intrusion->hash, intrusion->hash_type_bmc,
			nonce_and_stored_data, data_len_to_hash, hash_calculated, sizeof (hash_calculated));
		if (ROT_IS_ERROR (hash_len_calculated)) {
			status = hash_len_calculated;
			continue;
		}

		memset (intrusion->msg_buffer, 0, sizeof (intrusion->msg_buffer));

		status = msg_transport_create_empty_request (intrusion->msft_transport,
			intrusion->msg_buffer, sizeof (intrusion->msg_buffer), intrusion->dest_eid, &request);
		if (status != 0) {
			return status;
		}

		request_len =
			bmc_chassis_intrusion_generate_challenge_data_request (intrusion->hash_type_bmc,
			nonce_and_stored_data, request.payload, request.payload_length);
		if (ROT_IS_ERROR (request_len)) {
			return request_len;
		}

		cmd_interface_msg_set_message_payload_length (&request, request_len);

		status = msg_transport_create_empty_response (intrusion->msg_buffer,
			sizeof (intrusion->msg_buffer), &response);
		if (status != 0) {
			return status;
		}

		status = intrusion_state_bmc_send_request_and_get_response (intrusion, &request, &response);
		if (status != 0) {
			platform_msleep (intrusion->challenge_retry_delay);
			continue;
		}

		/* Process challenge data response directly */
		challenge_rsp =
			(struct bmc_chassis_intrusion_challenge_data_response*) intrusion->msg_buffer;

		if ((challenge_rsp->header.completion_code != 0) || (challenge_rsp->header.command !=
			BMC_CMD_CHASSIS_INTRUSION_CHALLENGE_DATA)) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INTRUSION,
				INTRUSION_LOGGING_CHALLENGE_DATA_FAIL, intrusion->dest_eid,
				challenge_rsp->header.completion_code);

			status = INTRUSION_STATE_REMOTE_REQUEST_FAILED;
			continue;
		}

		if (challenge_rsp->hash_len != hash_len_calculated) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INTRUSION,
				INTRUSION_LOGGING_CHALLENGE_DATA_INVALID_HASH_LEN, intrusion->dest_eid,
				challenge_rsp->hash_len);
			status = INTRUSION_STATE_REMOTE_REQUEST_FAILED;
			continue;
		}

		received_hash = bmc_chassis_intrusion_challenge_data_hash (challenge_rsp);

		if (buffer_compare (received_hash, hash_calculated, hash_len_calculated) != 0) {
			// Hashes do not match. Intrusion detected.
			status = 1;
		}
	} while (ROT_IS_ERROR (status) && (--retry));

exit:
	if (status == 1) {
		observable_notify_observers (&intrusion->observable,
			offsetof (struct intrusion_state_observer, on_intrusion));
	}
	else if (status == 0) {
		observable_notify_observers (&intrusion->observable,
			offsetof (struct intrusion_state_observer, on_no_intrusion));
	}
	else {
		observable_notify_observers (&intrusion->observable,
			offsetof (struct intrusion_state_observer, on_error));
	}

	return status;
}
