// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "device_keys.h"
#include "jtag_handler_emc.h"
#include "rom_logging.h"
#include "common/unused.h"


/**
 * This function programs the fuses for a device during provisioning
 * such as global seed, SoC ID, and unique device key.
 *
 * @param jtag The JTAG handler managing the provisioning process.
 * @param msg The generic message being processed.
 * @param current_state The current device security state.
 *
 * @return 0 if the message is processed or an error code.
 */
static int jtag_handler_handle_provision_device (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, enum hsp_security_state current_state)
{
	struct jtag_handler_msg_provision *prov = (struct jtag_handler_msg_provision*) msg;
	struct jtag_handler_emc *jtag_emc = (struct jtag_handler_emc*) jtag;
	SP_MSG_512 global_seed = {0};
	SP_MSG_512 socid = {0};
	int status;

	if (current_state != HSP_SECURITY_STATE_PRODUCTION) {
		return JTAG_HANDLER_UNSUPPORTED_MSG;
	}

	status = jtag_handler_receive_global_seed (jtag, msg, prov, &global_seed);
	if (status != 0) {
		return status;
	}

	/* Unwrap the global seed and store it into fuses. */
	status = jtag_handler_unwrap_program_global_seed (jtag, msg, prov, &global_seed);
	if (status != 0) {
		return status;
	}

	/* Generate SOCID for this device and program it. */
	status = jtag_handler_generate_and_program_socid (jtag, msg, prov, &socid, &jtag_emc->socid_tag,
		sizeof (uint8_t));
	if (status != 0) {
		return status;
	}

	/* Generate random seeds for device-unique key slots and program it. */
	status = jtag_handler_generate_device_unique_key (jtag, msg, prov, JTAG_HANDLER_MAX_KEY_SLOTS);
	if (status != 0) {
		return status;
	}

	status = jtag_handler_program_device_unique_key (jtag, msg, prov, JTAG_HANDLER_MAX_KEY_SLOTS);
	if (status != 0) {
		return status;
	}

	/* Transition the device to the Secure state. */
	status = jtag_handler_transition_to_secure_state (jtag, msg, prov);
	if (status != 0) {
		return status;
	}

	jtag_handler_msg_provision_complete (jtag, msg, prov);

	return 0;
}

/**
 * Handle a message for EMC fuses.
 *
 * @param jtag The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param current_state The current device security state.
 * @param program True if the fuse should be programmed, false to read the data.
 *
 * @return 0 if the message is processed or an error code.
 */
static int jtag_handler_emc_fuse (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	enum hsp_security_state current_state, bool program)
{
	struct jtag_handler_msg_emc_fuse *emc = (struct jtag_handler_msg_emc_fuse*) msg;
	int status;

	if (current_state != HSP_SECURITY_STATE_TEST) {
		return JTAG_HANDLER_UNSUPPORTED_MSG;
	}

	jtag_handler_msg_set_ack (msg);
	jtag->mailbox->write (jtag->mailbox, msg);

	if (program) {
		status = jtag->fuses->program_emc_register (jtag->fuses, emc->fuse_address, emc->fuse_data);
	}
	else {
		status = jtag->fuses->read_emc_register (jtag->fuses, emc->fuse_address, &emc->fuse_data);
	}

	if (status != 0) {
		if (FUSE_CONTROLLER_IS_HW_ERROR (status)) {
			jtag_handler_msg_emc_fuse_set_ext_status (emc, status);
			status = 0;
		}
		else {
			jtag_handler_msg_emc_fuse_set_ext_status (emc, 0);
		}
	}
	else {
		jtag_handler_msg_set_pass (msg);
	}

	return jtag_handler_message_done (jtag, msg, status, jtag_handler_msg_emc_fuse_is_last (emc));
}

int jtag_handler_emc_handle_msg (const struct jtag_handler *jtag)
{
	union jtag_handler_msg msg;
	enum hsp_security_state state;
	int status = 0;
	int fail_id = 0;

	if (jtag == NULL) {
		return JTAG_HANDLER_INVALID_ARGUMENT;
	}

	status = jtag_handler_read_mailbox_msg (jtag, &msg, &state);

	if (status == 0) {
		switch (jtag_handler_msg_get_request (&msg)) {
			case JTAG_HANDLER_CMD_PROVISION:
				status = jtag_handler_handle_provision_device (jtag, &msg, state);
				fail_id = ROM_LOGGING_FAIL_TRANSITION_TO_SECURE;
				break;

			case JTAG_HANDLER_CMD_EMC_FUSE_PROGRAM:
				status = jtag_handler_emc_fuse (jtag, &msg, state, true);
				break;

			case JTAG_HANDLER_CMD_EMC_FUSE_READ:
				status = jtag_handler_emc_fuse (jtag, &msg, state, false);
				break;

			default:
				status = jtag_handler_handle_common_msg (jtag, &msg, state, &fail_id);
				break;
		}
	}

	return jtag_handler_finish_msg (jtag, &msg, status, fail_id);
}

/**
 * Initialize a handler for JTAG mailbox messages specific to systems with EMC support.
 *
 * This function sets up a JTAG handler instance for handling messages in systems
 * that support EMC. It initializes the base handler and then
 * overrides necessary definitions for handling messages specific to systems with EMC support.
 *
 * @param jtag The JTAG handler to initialize.
 * @param mailbox Interface to the mailbox containing the messages.
 * @param fuses Interface to the HSP fuses.
 * @param ccs Interface to the HSP CCS and KSU for secure key management.
 * @param rng Random number generator to use during message handling, such as for generating the
 * SOCID.
 * @param public_keys List of public keys that can be retrieved over JTAG. Each array entry maps to
 * an index in the request. Unsupported indices must have the key buffer set to null.
 * @param socid_tag Identifier to use as the first byte of the SOCID for the device.
 * @return 0 if the handler is successfully initialized or an error code.
 */
int jtag_handler_emc_init (struct jtag_handler_emc *jtag, const struct jtag_mailbox *mailbox,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs,
	const struct rng_engine *rng, const struct jtag_handler_public_key public_keys[4],
	uint8_t socid_tag)
{
	int status = 0;

	if (jtag == NULL) {
		return JTAG_HANDLER_INVALID_ARGUMENT;
	}

	memset (jtag, 0, sizeof (struct jtag_handler_emc));

	/* Initialize base jtag handler definitions. */
	status = jtag_handler_init (&jtag->base, mailbox, fuses, ccs, rng, public_keys);
	if (status != 0) {
		return status;
	}

	/* Override the necessary base definitions for jtag handler emc type. */
	jtag->base.handle_msg = jtag_handler_emc_handle_msg;

	/* ROM jtag handler emc definitions. */
	jtag->socid_tag = socid_tag;

	return 0;
}

/**
 * Release the resources used for a JTAG message handler.
 *
 * @param jtag The JTAG handler to release.
 */
void jtag_handler_emc_release (const struct jtag_handler_emc *jtag)
{
	jtag_handler_release (&jtag->base);
}
