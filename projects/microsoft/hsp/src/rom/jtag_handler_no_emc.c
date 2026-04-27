// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "device_keys.h"
#include "jtag_handler_no_emc.h"
#include "rom_logging.h"
#include "common/unused.h"


/**
 * Handle provisioning canary fuse program
 *
 * @param jtag_no_emc The JTAG handler processing the message.
 * @param msg Message that is being processed.
 * @param prov The provisioning-specific message that is being handled.
 *
 * @return 0 if the message is processed or an error code.
 */
int jtag_handler_no_emc_program_canary (struct jtag_handler_no_emc *jtag_no_emc,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov)
{
	int status;
	uint64_t svn = 0;

	/* Canary fuse program */
	status = jtag_no_emc->rot->get_svn (jtag_no_emc->rot, &svn);
	if (status != 0) {
		jtag_handler_msg_provision_fail (&jtag_no_emc->base, msg, prov, status);

		return status;
	}

	if (svn == 0) {
		svn |= 0x01;
		status = jtag_no_emc->rot->update_svn (jtag_no_emc->rot, svn);
		if (status != 0) {
			jtag_handler_msg_provision_fail (&jtag_no_emc->base, msg, prov, status);
		}
	}

	return status;
}

/**
 * This function programs the fuses for a device during provisioning
 * such as global seed, SoC ID, unique device key and program canary bit.
 *
 * @param jtag The JTAG handler managing the provisioning process.
 * @param msg The generic message being processed.
 * @param current_state The current device security state.
 *
 * @return 0 if the message is processed or an error code.
 */
static int jtag_handler_handle_no_emc_provision_device (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, enum hsp_security_state current_state)
{
	struct jtag_handler_no_emc *jtag_no_emc = (struct jtag_handler_no_emc*) jtag;
	struct jtag_handler_msg_provision *prov = (struct jtag_handler_msg_provision*) msg;
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

	/* Generate a random SOCID for this device and program it. */
	status = jtag_handler_generate_and_program_socid (jtag, msg, prov, &socid,
		jtag_no_emc->socid_prefix, jtag_no_emc->socid_prefix_len);
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

	/* Program the canary fuse. */
	status = jtag_handler_no_emc_program_canary (jtag_no_emc, msg, prov);
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

int jtag_handler_no_emc_handle_msg (const struct jtag_handler *jtag)
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
				status = jtag_handler_handle_no_emc_provision_device (jtag, &msg, state);
				fail_id = ROM_LOGGING_FAIL_TRANSITION_TO_SECURE;
				break;

			default:
				status = jtag_handler_handle_common_msg (jtag, &msg, state, &fail_id);
				break;
		}
	}

	return jtag_handler_finish_msg (jtag, &msg, status, fail_id);
}

/**
 * Initialize a handler for JTAG mailbox messages specific to systems without EMC support but with Canary programming.
 *
 * This function sets up a JTAG handler instance for handling messages in systems
 * that support Canary programming and do not support EMC. It initializes the base handler and then
 * overrides necessary definitions for handling messages specific to systems without EMC support.
 *
 * @param jtag The JTAG handler to initialize.
 * @param mailbox Interface to the mailbox containing the messages.
 * @param fuses Interface to the HSP fuses.
 * @param ccs Interface to the HSP CCS and KSU for secure key management.
 * @param rng Random number generator to use during message handling, such as for generating the
 * SOCID.
 * @param public_keys List of public keys that can be retrieved over JTAG. Each array entry maps to
 * an index in the request. Unsupported indices must have the key buffer set to null.
 * @param socid_prefix Pointer to a prefix for the SOCID.
 * @param socid_prefix_len Length of prefix SOCID.
 * @param rot Interface to the ROT.
 * @return 0 if the handler was successfully initialized or an error code.
 */
int jtag_handler_no_emc_init (struct jtag_handler_no_emc *jtag, const struct jtag_mailbox *mailbox,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs,
	const struct rng_engine *rng, const struct jtag_handler_public_key public_keys[4],
	const uint8_t *socid_prefix, size_t socid_prefix_len, const struct hw_rot *rot)
{
	int status = 0;

	if ((jtag == NULL) || (socid_prefix == NULL) || (rot == NULL)) {
		return JTAG_HANDLER_INVALID_ARGUMENT;
	}

	memset (jtag, 0, sizeof (struct jtag_handler_no_emc));

	/* Initialize base jtag handler definitions. */
	status = jtag_handler_init (&jtag->base, mailbox, fuses, ccs, rng, public_keys);
	if (status != 0) {
		return status;
	}

	/* Override the necessary base definitions for jtag handler no emc type. */
	jtag->base.handle_msg = jtag_handler_no_emc_handle_msg;

	/* ROM jtag handler no emc definitions. */
	jtag->socid_prefix = socid_prefix;
	jtag->socid_prefix_len = socid_prefix_len;
	jtag->rot = rot;

	return 0;
}

/**
 * Release the resources used for a JTAG message handler.
 *
 * @param jtag The JTAG handler to release.
 */
void jtag_handler_no_emc_release (const struct jtag_handler_no_emc *jtag)
{
	jtag_handler_release (&jtag->base);
}
