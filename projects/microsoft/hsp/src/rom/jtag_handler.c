// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "device_keys.h"
#include "jtag_handler.h"
#include "rom_logging.h"
#include "common/unused.h"
#include "logging/code_path_integrity.h"


/**
 * Length of the SOCID for the device.
 */
#define	JTAG_HANDLER_SOCID_LENGTH					16

/**
 * Length of the global seed data sent during provisioning.
 */
#define	JTAG_HANDLER_PROVISION_SEED_LENGTH			SP_MSG_512_SIZE

/**
 * Number of data bytes in each provisioning message.
 */
#define	JTAG_HANDLER_PROVISION_SEED_DATA_BYTES      \
	sizeof (((struct jtag_handler_msg_provision*) 0)->seed_data)

/**
 * The maximum offset value for global seed data.
 */
#define	JTAG_HANDLER_PROVISION_LAST_SEED_OFFSET     \
	(((JTAG_HANDLER_PROVISION_SEED_LENGTH + (JTAG_HANDLER_PROVISION_SEED_DATA_BYTES - 1)) / \
		JTAG_HANDLER_PROVISION_SEED_DATA_BYTES) - 1)

/**
 * The number of valid seed data bytes in the last provision message.
 */
#define	JTAG_HANDLER_PROVISION_LAST_SEED_LENGTH     \
	JTAG_HANDLER_PROVISION_SEED_DATA_BYTES - \
		((JTAG_HANDLER_PROVISION_SEED_DATA_BYTES * (JTAG_HANDLER_PROVISION_LAST_SEED_OFFSET + 1)) - \
			JTAG_HANDLER_PROVISION_SEED_LENGTH)

/**
 * Convert a fuse controller HW error to fuse error bits for the ext_status field.
 */
#define	JTAG_HANDLER_FUSE_ERROR_TO_EXT_STATUS(x)	((((x) & 0x60) << 1) | ((x) & 0x1f))


/**
 * Mark a message a done.  If there are more messages to process, wait for the next message to be
 * available.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param status Returned status of the message processing.
 * @param is_last Flag indicating if this is the last message to process.
 *
 * @return The overall completion status for the message.
 */
int jtag_handler_message_done (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	int status, bool is_last)
{
	jtag_handler_msg_set_done (msg);
	jtag->mailbox->write (jtag->mailbox, msg);

	if ((status == 0) && !is_last) {
		do {
			jtag->mailbox->read (jtag->mailbox, msg);
		} while (!jtag_handler_msg_is_new_request (msg));

		return JTAG_HANDLER_MORE;
	}
	else {
		return status;
	}
}

/**
 * Complete processing of a JTAG message, reporting status to the caller.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param status Returned status of the message processing.
 * @param ext_status Output for the extended status information in this message.
 * @param is_last Flag indicating if this is the last message to process.
 *
 * @return The overall completion status for the message.
 */
static int jtag_handler_complete_message_processing (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, int status, uint8_t *ext_status, bool is_last)
{
	if (status != 0) {
		if (FUSE_CONTROLLER_IS_HW_ERROR (status)) {
			*ext_status = JTAG_HANDLER_FUSE_ERROR_TO_EXT_STATUS ((uint8_t) status);
			status = 0;
		}
		else {
			*ext_status = 0;
		}
	}
	else {
		jtag_handler_msg_set_pass (msg);
	}

	return jtag_handler_message_done (jtag, msg, status, is_last);
}

/**
 * Handle a message to transition the device security state.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param current_state The current device security state.
 * @param to_state The target security state of the device.
 * @param allowed_state The security state allowed to execute the transition.
 *
 * @return 0 if the message is processed or an error code.
 */
static int jtag_handler_transition_security_state (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, enum hsp_security_state current_state,
	enum hsp_security_state to_state, enum hsp_security_state allowed_state)
{
	struct jtag_handler_msg_ss_transition *change_ss = (struct jtag_handler_msg_ss_transition*) msg;
	int status;

	if (current_state != allowed_state) {
		return JTAG_HANDLER_UNSUPPORTED_MSG;
	}

	jtag_handler_msg_set_ack (msg);
	jtag->mailbox->write (jtag->mailbox, msg);

	status = jtag->fuses->change_security_state (jtag->fuses, to_state);

	return jtag_handler_complete_message_processing (jtag, msg, status, &change_ss->ext_status,
		jtag_handler_msg_ss_transition_is_last (change_ss));
}

/**
 * Handle a message for RNG calibration data.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param current_state The current device security state.
 * @param program True if the calibration should be programmed, false to read the data.
 *
 * @return 0 if the message is processed or an error code.
 */
static int jtag_handler_rng_calibration (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, enum hsp_security_state current_state, bool program)
{
	struct jtag_handler_msg_rng_calibration *rng = (struct jtag_handler_msg_rng_calibration*) msg;
	int status;

	if (current_state != HSP_SECURITY_STATE_PRODUCTION) {
		return JTAG_HANDLER_UNSUPPORTED_MSG;
	}

	jtag_handler_msg_set_ack (msg);
	jtag->mailbox->write (jtag->mailbox, msg);

	if (program) {
		status = jtag->fuses->program_rng_calibration (jtag->fuses, rng->calibration);
	}
	else {
		status = jtag->fuses->read_rng_calibration (jtag->fuses, rng->calibration);
	}

	return jtag_handler_complete_message_processing (jtag, msg, status, &rng->ext_status,
		program || jtag_handler_msg_rng_calibration_is_last (rng));
}

/**
 * Handle a message to blank check fuses.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param current_state The current device security state.
 *
 * @return 0 if the message is processed or an error code.
 */
static int jtag_handler_fuse_blank_check (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, enum hsp_security_state current_state)
{
	struct jtag_handler_msg_fuse_blank_check *check =
		(struct jtag_handler_msg_fuse_blank_check*) msg;
	int status;

	if (current_state == HSP_SECURITY_STATE_UNKNOWN) {
		return JTAG_HANDLER_UNSUPPORTED_MSG;
	}

	jtag_handler_msg_set_ack (msg);
	jtag->mailbox->write (jtag->mailbox, msg);

	status = jtag->fuses->blank_check (jtag->fuses, check->start_address, check->end_address,
		&check->first_not_blank_addr);
	if (status != 0) {
		if (status == FUSE_CONTROLLER_NOT_BLANK) {
			status = 0;
		}
	}
	else {
		jtag_handler_msg_set_pass (msg);
	}

	return jtag_handler_message_done (jtag, msg, status,
		(current_state == HSP_SECURITY_STATE_SECURE) ||
		jtag_handler_msg_fuse_blank_check_is_last (check));
}

/**
 * Handle a message to retrieve a device public key.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param current_state The current device security state.
 *
 * @return 0 if the message is processed or an error code.
 */
static int jtag_handler_get_public_key (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, enum hsp_security_state current_state)
{
	struct jtag_handler_msg_public_key *key = (struct jtag_handler_msg_public_key*) msg;
	size_t pos = 0;
	int i = jtag_handler_msg_public_key_get_key_index (key);
	size_t next_len;

	if ((current_state != HSP_SECURITY_STATE_SECURE) || (jtag->keys[i].key == NULL)) {
		return JTAG_HANDLER_UNSUPPORTED_MSG;
	}

	jtag_handler_msg_set_ack (msg);
	jtag->mailbox->write (jtag->mailbox, msg);

	do {
		jtag_handler_msg_set_done (msg);
		jtag_handler_msg_public_key_set_key_offset (key, pos / sizeof (key->key_data));

		next_len = jtag->keys[i].length - pos;
		if (next_len >= sizeof (key->key_data)) {
			next_len = sizeof (key->key_data);
		}
		else {
			/* Zero pad unused bytes in the last transaction.  We just clear the whole buffer rather
			 * than creating additional instructions to target only the bytes necessary because the
			 * buffer is very small. */
			memset (key->key_data, 0, sizeof (key->key_data));
		}
		memcpy (key->key_data, &jtag->keys[i].key[pos], next_len);
		pos += next_len;

		if (pos >= jtag->keys[i].length) {
			jtag_handler_msg_set_pass (msg);
		}
		jtag->mailbox->write (jtag->mailbox, msg);

		while ((pos < jtag->keys[i].length) && jtag_handler_msg_is_done (msg)) {
			jtag->mailbox->read (jtag->mailbox, msg);
		}
	} while (pos < jtag->keys[i].length);

	return 0;
}

/**
 * Handle a provisioning failure. This function will be called when a provisioning process fails.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param prov The provisioning-specific message that is being handled.
 * @param status The status code indicating the reason for the failure.
 *
 */
void jtag_handler_msg_provision_fail (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, int status)
{
	prov->ext_status = status;
	prov->ext_unused = 0;
	jtag_handler_msg_set_done (msg);
	jtag_handler_msg_clear_pass (msg);
	jtag->mailbox->write (jtag->mailbox, msg);
}

/**
 * Handle a provisioning success. This function will be called when a provisioning process completes successfully.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param prov The provisioning-specific message that is being handled.
 *
 */
void jtag_handler_msg_provision_complete (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov)
{
	jtag_handler_msg_provision_set_complete (prov);
	jtag_handler_msg_set_done (msg);
	jtag_handler_msg_set_pass (msg);
	jtag->mailbox->write (jtag->mailbox, msg);
}

/**
 * Handle a message to receive the wrapped global seed.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message that is being processed.
 * @param prov The provisioning-specific message that is being handled.
 * @param global_seed Pointer to a data structure containing the received global seed.
 *
 * @return 0 if the message is processed or an error code.
 */
int jtag_handler_receive_global_seed (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, SP_MSG_512 *global_seed)
{
	int offset;
	size_t received = 0;
	int status;

	jtag_handler_msg_set_ack (msg);
	jtag->mailbox->write (jtag->mailbox, msg);

	/* Receive the wrapped global seed.  The key must be wrapped using OBS(0) as the KEK, since this
	 * will be the key loaded into KSU key slots when the fuses are blank.
	 *
	 * Once we receive the last chunk of seed data, as indicated by the seed offset, proceed with
	 * device key provisioning. */
	offset = jtag_handler_msg_provision_get_seed_offset (prov);
	while (offset != JTAG_HANDLER_PROVISION_LAST_SEED_OFFSET) {
		/* If the seed offset is set out of range, just ignore that data and wait for the next
		 * message.  The alternative here is to just fail, but it shouldn't be a problem to keep
		 * looping waiting for correct data. */
		if (offset < (int) JTAG_HANDLER_PROVISION_LAST_SEED_OFFSET) {
			memcpy (&global_seed->AsBytes[offset * sizeof (prov->seed_data)], prov->seed_data,
				sizeof (prov->seed_data));
			received += sizeof (prov->seed_data);
		}

		jtag_handler_msg_set_done (msg);
		jtag->mailbox->write (jtag->mailbox, msg);

		do {
			jtag->mailbox->read (jtag->mailbox, msg);
		} while (jtag_handler_msg_is_done (msg));

		offset = jtag_handler_msg_provision_get_seed_offset (prov);
	}

	memcpy (&global_seed->AsBytes[JTAG_HANDLER_PROVISION_LAST_SEED_OFFSET *
		sizeof (prov->seed_data)], prov->seed_data, JTAG_HANDLER_PROVISION_LAST_SEED_LENGTH);
	received += JTAG_HANDLER_PROVISION_LAST_SEED_LENGTH;

	/* This is just a quick sanity check and doesn't account for all scenarios.  It mainly exists to
	 * provide meaningful errors to well-intentioned sources that are not functioning correctly.  It
	 * is possible to just send the same offset multiple times then the last offset to make it look
	 * like we got all the data, which would cause this check to pass.  In this scenario however,
	 * the unwrap operation would still fail, preventing that incorrect key from being used. */
	if (received < sizeof (*global_seed)) {
		status = JTAG_HANDLER_SHORT_SEED_DATA;
		jtag_handler_msg_provision_fail (jtag, msg, prov, status);

		return status;
	}

	jtag_handler_msg_provision_set_seed_received (prov);
	jtag->mailbox->write (jtag->mailbox, msg);

	return 0;
}

/**
 * Unwrap a global seed message. This function processes the global seed message and
 * unwrap the seed during provisioning.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message that is being processed.
 * @param prov The provisioning-specific message that is being handled.
 * @param global_seed Pointer to a data structure containing the global seed that needs to be unwrapped.
 *
 * @return 0 if the message is processed or an error code.
 */
int jtag_handler_unwrap_global_seed (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, SP_MSG_512 *global_seed)
{
	int status;

	status = jtag->ccs->unwrap_key (jtag->ccs, DEVICE_KEYS_GLOBAL_SEED, global_seed,
		DEVICE_KEYS_GLOBAL_SEED);
	if (status != 0) {
		jtag_handler_msg_provision_fail (jtag, msg, prov, status);

		return status;
	}

	return 0;
}

/**
 * Program the global seed. This function processes the global seed
 * and stores it during provisioning.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message is being processed.
 * @param prov The provisioning-specific message that is being handled.
 *
 * @return 0 if the message is processed or an error code.
 */
int jtag_handler_program_global_seed (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov)
{
	int status;

	status = jtag->ccs->burn_key (jtag->ccs, DEVICE_KEYS_GLOBAL_SEED);
	if (status != 0) {
		jtag_handler_msg_provision_fail (jtag, msg, prov, status);

		return status;
	}

	jtag_handler_msg_provision_set_seed_programmed (prov);
	jtag->mailbox->write (jtag->mailbox, msg);

	return 0;
}

/**
 * Handle the provision of a global seed.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message is being processed.
 * @param prov The provisioning-specific message that is being handled.
 *
 * @return 0 if the message is processed or an error code.
 */
int jtag_handler_unwrap_program_global_seed (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov, SP_MSG_512 *global_seed)
{
	int status;

	/* Unwrap the global seed and store it into fuses. */
	status = jtag->fuses->blank_check_key (jtag->fuses, DEVICE_KEYS_GLOBAL_SEED);
	if (status == 0) {
		status = jtag_handler_unwrap_global_seed (jtag, msg, prov, global_seed);
		if (status != 0) {
			return status;
		}

		status = jtag_handler_program_global_seed (jtag, msg, prov);
		if (status != 0) {
			return status;
		}
	}
	else if (status != FUSE_CONTROLLER_NOT_BLANK) {
		jtag_handler_msg_provision_fail (jtag, msg, prov, status);

		return status;
	}

	return 0;
}

/**
 * Generate a random SOCID for this device.
 *
 * This function generates a random SOCID for the device, storing it in the provided
 * SOCID structure. The random portion is generated starting after the prefix length.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message being processed.
 * @param prov The provisioning-specific message that is being handled.
 * @param socid Pointer to a data structure to store the random SOCID.
 * @param prefix_length The length of the prefix already stored in the SOCID structure.
 *
 * @return 0 if the SOCID is successfully generated, or an error code.
 */
int jtag_handler_generate_socid (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, SP_MSG_512 *socid, const uint8_t *socid_prefix,
	size_t prefix_length)
{
	int status;

	/* Copy the socid prefix to the socid */
	memcpy (socid, socid_prefix, prefix_length);

	status = jtag->rng->generate_random_buffer (jtag->rng,
		JTAG_HANDLER_SOCID_LENGTH - prefix_length, &socid->AsBytes[prefix_length]);
	if (status != 0) {
		jtag_handler_msg_provision_fail (jtag, msg, prov, status);

		return status;
	}

	return 0;
}

/**
 * Handle the programming of a SOCID.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message that is being processed.
 * @param prov The provisioning-specific message that is being handled.
 * @param socid Pointer to a data structure containing the SOCID to program the device.
 *
 * @return 0 if the message is processed or an error code.
 */
int jtag_handler_program_socid (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, SP_MSG_512 *socid)
{
	int status;

	status = jtag->fuses->program_socid (jtag->fuses, socid->AsBytes, JTAG_HANDLER_SOCID_LENGTH);
	if (status != 0) {
		jtag_handler_msg_provision_fail (jtag, msg, prov, status);

		return status;
	}

	return 0;
}

/**
 * Generate and program SOCID for the device.
 *
 * This function checks if the SOCID fuses are blank. If fuses are blank, it generates a random SOCID
 * and then programs it into the device. If the SOCID fuses are not blank and the error is not
 * FUSE_CONTROLLER_NOT_BLANK, it handles the error accordingly.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message that is being processed.
 * @param prov The provisioning-specific message that is being handled.
 * @param socid Pointer to a data structure to store the generated SOCID.
 * @param socid_prefix Pointer to a prefix for the SOCID.
 * @param prefix_length Length of the SOCID prefix.
 *
 * @return 0 if the SOCID is successfully generated and programmed, or an error code.
 */
int jtag_handler_generate_and_program_socid (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov, SP_MSG_512 *socid,
	const uint8_t *socid_prefix, size_t prefix_length)
{
	int status;

	status = jtag->fuses->blank_check_socid (jtag->fuses);
	if (status == 0) {
		/* Generate SOCID for this device and program it. */
		status = jtag_handler_generate_socid (jtag, msg, prov, socid, socid_prefix, prefix_length);
		if (status != 0) {
			return status;
		}

		status = jtag_handler_program_socid (jtag, msg, prov, socid);
		if (status != 0) {
			return status;
		}
	}
	else if (status != FUSE_CONTROLLER_NOT_BLANK) {
		jtag_handler_msg_provision_fail (jtag, msg, prov, status);

		return status;
	}

	return 0;
}

/**
 * Generate random bytes for provisioning device unique key.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The  message is being processed.
 * @param prov The provisioning-specific message that is being handled.
 * @param int Number of the key slots
 *
 * @return 0 if the message is processed or an error code.
 */
int jtag_handler_generate_device_unique_key (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov, size_t num_slots)
{
	int status;
	size_t i;

	if (num_slots > JTAG_HANDLER_MAX_KEY_SLOTS) {
		return JTAG_HANDLER_TOO_MANY_KEY_SLOTS;
	}

	for (i = 0; i < num_slots; i++) {
		status = jtag->fuses->blank_check_key (jtag->fuses, i);
		if (status == 0) {
			status = jtag->ccs->generate_random_key (jtag->ccs, i,
				CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED |
				CCS_KSU_ATTR_KEY_SIZE_384);
			if (status != 0) {
				jtag_handler_msg_provision_fail (jtag, msg, prov, status);

				return status;
			}
		}
		else if (status != FUSE_CONTROLLER_NOT_BLANK) {
			jtag_handler_msg_provision_fail (jtag, msg, prov, status);

			return status;
		}
	}

	return 0;
}

/**
 * Program unique keys to the device key slots.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The  message is being processed.
 * @param prov The provisioning-specific message that is being handled.
 * @param int Number of the key slots
 *
 * @return 0 if the message is processed or an error code.
 */
int jtag_handler_program_device_unique_key (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov, size_t num_slots)
{
	int status;
	size_t i;

	if (num_slots > JTAG_HANDLER_MAX_KEY_SLOTS) {
		return JTAG_HANDLER_TOO_MANY_KEY_SLOTS;
	}

	for (i = 0; i < num_slots; i++) {
		status = jtag->fuses->blank_check_key (jtag->fuses, i);
		if (status == 0) {
			status = jtag->ccs->burn_key (jtag->ccs, i);
			if (status != 0) {
				jtag_handler_msg_provision_fail (jtag, msg, prov, status);

				return status;
			}
		}
		else if (status != FUSE_CONTROLLER_NOT_BLANK) {
			jtag_handler_msg_provision_fail (jtag, msg, prov, status);

			return status;
		}
	}

	jtag_handler_msg_provision_set_keys_programmed (prov);
	jtag->mailbox->write (jtag->mailbox, msg);

	return 0;
}

/**
 * Handle the device transition to the Secure state.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param prov The provisioning-specific message that is being handled.
 *
 * @return 0 if the message is processed or an error code.
 */
int jtag_handler_transition_to_secure_state (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov)
{
	int status;

	/* Transition the device to the Secure state. */
	status = jtag->fuses->change_security_state (jtag->fuses, HSP_SECURITY_STATE_SECURE);
	if (status != 0) {
		jtag_handler_msg_provision_fail (jtag, msg, prov, status);

		return status;
	}

	return 0;
}

/**
 * Read a message from the JTAG mailbox and update the current security state.
 * If the message is not a new request or is a null message, it indicates no pending message.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message buffer to read into.
 * @param current_state Pointer to the variable storing the current security state.
 *
 * @return 0 if the message is successfully read and processed.
 * JTAG_HANDLER_NO_MESSAGE if there is no new request or the message is a null message.
 */
int jtag_handler_read_mailbox_msg (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	enum hsp_security_state *current_state)
{
	jtag->mailbox->read (jtag->mailbox, msg);
	if (!jtag_handler_msg_is_new_request (msg) ||
		(jtag_handler_msg_get_request (msg) == JTAG_HANDLER_CMD_NULL_MESSAGE)) {
		return JTAG_HANDLER_NO_MESSAGE;
	}

	code_path_integrity_message_trace (ROM_LOGGING_TRACE_JTAG_MESSAGE);

	*current_state = jtag->fuses->get_security_state (jtag->fuses);

	return 0;
}

/**
 * Handle common JTAG messages. This function processes common JTAG messages.
 *
 * This function processes common JTAG messages, such as transitioning to different security states,
 * requesting public keys, performing fuse blank checks, and handling RNG calibration operations.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message to be processed.
 * @param state The current security state.
 * @param fail_id Pointer to an integer to store the failure identifier, if applicable.
 *
 * @return The status of the message processing.
 *         - 0 if the message is successfully processed.
 *         - JTAG_HANDLER_UNSUPPORTED_MSG if the message is unsupported.
 *         - JTAG_HANDLER_HALT if the message indicates halting further processing.
 *         - An error code if an error occurred during message processing.
 */
int jtag_handler_handle_common_msg (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	enum hsp_security_state state, int *fail_id)
{
	int status = 0;

	switch (jtag_handler_msg_get_request (msg)) {
		case JTAG_HANDLER_CMD_TRANSITION_TO_TEST:
			status = jtag_handler_transition_security_state (jtag, msg, state,
				HSP_SECURITY_STATE_TEST, HSP_SECURITY_STATE_BLANK);
			*fail_id = ROM_LOGGING_FAIL_TRANSITION_TO_TEST;
			break;

		case JTAG_HANDLER_CMD_TRANSITION_TO_PROD:
			status = jtag_handler_transition_security_state (jtag, msg, state,
				HSP_SECURITY_STATE_PRODUCTION, HSP_SECURITY_STATE_TEST);
			*fail_id = ROM_LOGGING_FAIL_TRANSITION_TO_PROD;
			break;

		case JTAG_HANDLER_CMD_REQUEST_PUBLIC_KEYS:
			status = jtag_handler_get_public_key (jtag, msg, state);
			break;

		case JTAG_HANDLER_CMD_FUSE_BLANK_CHECK:
			status = jtag_handler_fuse_blank_check (jtag, msg, state);
			break;

		case JTAG_HANDLER_CMD_RNG_CALIBRATION_PROGRAM:
			status = jtag_handler_rng_calibration (jtag, msg, state, true);
			break;

		case JTAG_HANDLER_CMD_RNG_CALIBRATION_READ:
			status = jtag_handler_rng_calibration (jtag, msg, state, false);
			break;

		case JTAG_HANDLER_CMD_HALT_ROM:
			if ((state != HSP_SECURITY_STATE_SECURE) && (state != HSP_SECURITY_STATE_UNKNOWN)) {
				jtag_handler_msg_set_all_status (msg);
				jtag->mailbox->write (jtag->mailbox, msg);

				status = JTAG_HANDLER_HALT;
			}
			else {
				status = JTAG_HANDLER_UNSUPPORTED_MSG;
			}
			break;

		default:
			status = JTAG_HANDLER_UNSUPPORTED_MSG;
			break;
	}

	return status;
}

/**
 * Finish handling a JTAG message. If the status indicates an unsupported message,
 * the function marks the message as done and writes it to the JTAG mailbox. Otherwise,
 * if the status indicates an error other than unsupported message, more, no message, or halt,
 * it logs the error using ROM logging.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg The message that is being processed.
 * @param status Returned status of the message processing.
 * @param fail_id The identifier for the specific failure.
 *
 * @return The status of the message processing.
 *         - 0 if the message is successfully processed.
 *         - JTAG_HANDLER_UNSUPPORTED_MSG if the message is unsupported.
 *         - JTAG_HANDLER_MORE if further processing is required.
 *         - JTAG_HANDLER_NO_MESSAGE if there is no pending message.
 *         - JTAG_HANDLER_HALT if the message indicates halting further processing.
 *         - An error code if an error occurred during message processing.
 */
int jtag_handler_finish_msg (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	int status, uint8_t fail_id)
{
	if (status == JTAG_HANDLER_UNSUPPORTED_MSG) {
		jtag_handler_msg_set_done (msg);
		jtag->mailbox->write (jtag->mailbox, msg);
	}
	else if ((status != 0) && (status != JTAG_HANDLER_MORE) &&
		(status != JTAG_HANDLER_NO_MESSAGE) && (status != JTAG_HANDLER_HALT)) {
		rom_logging_error ((fail_id == 0) ? ROM_LOGGING_FAIL_JTAG_HANDLER : fail_id, status);
	}

	return status;
}

/**
 * Initialize a handler for JTAG mailbox messages.
 *
 * @param jtag The JTAG handler to initialize.
 * @param mailbox Interface to the mailbox containing the messages.
 * @param fuses Interface to the HSP fuses.
 * @param ccs Interface to the HSP CCS and KSU for secure key management.
 * @param rng Random number generator to use during message handling, such as for generating the
 * SOCID.
 * @param public_keys List of public keys that can be retrieved over JTAG.  Each array entry maps to
 * an index in the request.  Unsupported indicies must have the key buffer set to null.
 *
 * @return 0 if the handler is successfully initialized or an error code.
 */
int jtag_handler_init (struct jtag_handler *jtag, const struct jtag_mailbox *mailbox,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs,
	const struct rng_engine *rng, const struct jtag_handler_public_key public_keys[4])
{
	if ((jtag == NULL) || (mailbox == NULL) || (fuses == NULL) || (ccs == NULL) || (rng == NULL) ||
		(public_keys == NULL)) {
		return JTAG_HANDLER_INVALID_ARGUMENT;
	}

	memset (jtag, 0, sizeof (struct jtag_handler));

	jtag->mailbox = mailbox;
	jtag->fuses = fuses;
	jtag->ccs = ccs;
	jtag->rng = rng;
	jtag->keys = public_keys;

	return 0;
}

/**
 * Release the resources used for a JTAG message handler.
 *
 * @param jtag The JTAG handler to release.
 */
void jtag_handler_release (const struct jtag_handler *jtag)
{
	UNUSED (jtag);
}
