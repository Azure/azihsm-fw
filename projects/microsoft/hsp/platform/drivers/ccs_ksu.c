// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "common/buffer_util.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "crypto/rng_hsp.h"
#include "drivers/ccs_ksu.h"
#include "drivers/crypto_hw.h"
#include "drivers/sram.h"
#include "splibs/inc/sptypes.h"


/* Make sure the expected size of KSU key and PCR slots match the HSP configuration. */
_Static_assert ((sizeof (struct ksu_key_slot) == KSU_KSB_KEYS_KEY1_OFFSET),
	"KSU Key slot is not sized as expected.");
_Static_assert ((sizeof (struct ksu_pcr_slot) == KSU_KSB_PCRS_PCR1_OFFSET),
	"PCR slot is not sized as expected.");

/**
 * Get the address for a specified key slot.
 *
 * @param ccs The CCS to check against.
 * @param key_slot The desired key slot.
 * @param address Output for the key slot address.
 *
 * @return true if the key slot is valid, false if not.
 */
static bool ccs_ksu_get_key_slot_address (const struct ccs_ksu *ccs, uint8_t key_slot,
	uint32_t *address)
{
	if (key_slot < ccs->key_slots) {
		*address = (uint32_t) &ccs->keys[key_slot];

		return true;
	}
	else {
		return false;
	}
}

/**
 * Get the address for a specified PCR slot.
 *
 * @param ccs The CCS to check against.
 * @param pcr_slot The desired PCR slot.
 * @param address Output for the PCR slot address.
 *
 * @return true if the PCR slot is valid, false if not.
 */
bool ccs_ksu_get_pcr_slot_address (const struct ccs_ksu *ccs, uint8_t pcr_slot, uint32_t *address)
{
	if (pcr_slot < ccs->pcr_slots) {
		*address = (uint32_t) &ccs->pcrs[pcr_slot];

		return true;
	}
	else {
		return false;
	}
}

/**
 * Execute a CCS command.
 *
 * @param ccs The CCS that will execute the command.
 * @param cmd The command structure to execute.  This will be copied into the location used by the
 * CCS.  The address arguments should be populated with the following requirements.  For this to
 * work generally, it is assumed that 0x8effxxxx is a reserved address range within HSP.
 * 		- If the argument is a KSU key slot, set the value to (CCS_KSU_KEY_SLOT | slot).
 * 		- If the argument is a KSU PCR slot, set the value to (CCS_KSU_PCR_SLOT | slot).
 * 		- If the argument is data output, set the value to the CCS output buffer address.
 * 		- If the argument is data input, set the value to the data buffer address.  If this falls
 * 		  outside HSP shared SRAM or the data needs to be reversed, it will be copied to the input
 * 		  buffer.
 * @param input_len The amount of data that must be copied into the command input buffer.  If no
 * input data is required for this command, set this to 0.  Input data will be copied in reverse
 * order if the CCS_KSU_INPUT_REVERSE flag is set on this length.
 * @param output User buffer for output data from the command.  The output data will be copied to
 * this buffer when the command is successful.  If there is no output data, this must be null.
 * @param output_len Length of the output data.
 * @param error_code Error to return when the the command failure bit is set.
 * @param hw_flags Bitmask of ccs_ksu_cmd_hw flags indicating which external hardware engines are
 * used by the command.  For commands that require random data, command execution will not be
 * started until the RNG is ready.  Commands that use the DRBG in FW mode, such as commands to
 * derive ECC keys, will not wait for the RNG, since those commands don't need random numbers.
 * However, in those cases, the RNG will be reset and recalibrated after the command has been
 * completed.  Any hardware resources needed to executed the command will be notified to lock access
 * to the hardware until the command has completed.
 *
 * @return 0 if the command was completed successfully or an error code.
 */
int ccs_ksu_execute_command (const struct ccs_ksu *ccs, struct ccs_command *cmd, size_t input_len,
	uint8_t *output, size_t output_len, int error_code, uint8_t hw_flags)
{
	int i;
	uint32_t status = 0;
	int rng_status = 0;
	bool input_arg_done = false;

	if (hw_flags & CCS_KSU_CMD_HW_LOCK_CCS) {
		platform_mutex_lock (&ccs->state->lock);
	}

	/* Before we start, make sure the CCS is available and wipe out any contents in the command
	 * buffer.  This is to clean up after possible FW errors with previous commands or other
	 * abnormal states.  Even when configured to use interrupts, this will still be a busy loop, for
	 * simplicity.  It should typically not get executed. */
	while (ccs->regs->status & CCS_REGS_STATUS_BUSY_FIELD_MASK) {
	}
	memset (ccs->buffer, 0, sizeof (*ccs->buffer));

	/* Validate and initialize all command arguments. */
	for (i = 0; i < 4; i++) {
		if (cmd->address[i] != 0) {
			if ((cmd->address[i] & 0xffffff00) == CCS_KSU_KEY_SLOT) {
				if (!ccs_ksu_get_key_slot_address (ccs, cmd->address[i] & 0xff, &cmd->address[i])) {
					status = CCS_KSU_UNSUPPORTED_KEY_SLOT;
					break;
				}
			}
			else if ((cmd->address[i] & 0xffffff00) == CCS_KSU_PCR_SLOT) {
				if (!ccs_ksu_get_pcr_slot_address (ccs, cmd->address[i] & 0xff, &cmd->address[i])) {
					status = CCS_KSU_UNSUPPORTED_PCR_SLOT;
					break;
				}
			}
			else if (!input_arg_done && (cmd->address[i] != (uint32_t) &ccs->buffer->output) &&
				(input_len != 0) && !(input_len & CCS_KSU_INPUT_NO_PROCESSING)) {
				/* This is the input data argument for the command, and it must be prepared for use
				 * by the CCS.  The data needs to be available in shared SRAM, and in some cases the
				 * byte order also needs to be reversed. */
				if (input_len & CCS_KSU_INPUT_REVERSE) {
					/* Reverse the byte order while copying the data into shared SRAM.  Since we
					 * need to change the input data, this copy happens regardless of where the
					 * source data is located. */
					buffer_reverse_copy (ccs->buffer->input.bytes, (uint8_t*) cmd->address[i],
						input_len & ~CCS_KSU_INPUT_REVERSE);
					cmd->address[i] = (uint32_t) &ccs->buffer->input;
				}
				else if (!sram_is_shared_address ((void*) cmd->address[i])) {
					/* The data doesn't need to be reversed, but it is not in shared SRAM. */
					memcpy (&ccs->buffer->input, (void*) cmd->address[i], input_len);
					cmd->address[i] = (uint32_t) &ccs->buffer->input;
				}

				/* Commands will not have more than one input argument that needs to be processed.
				 * Any additional arguments should be ignored for input processing. */
				input_arg_done = true;
			}
		}
	}

	if (status != 0) {
		goto exit;
	}

	/* Wait for all required hardware blocks to be available. */
	if (hw_flags & CCS_KSU_CMD_HW_SHA) {
		hs_sha_mark_as_in_use (ccs->sha);
	}

	if (hw_flags & CCS_KSU_CMD_HW_AES) {
		hsp_aes_mark_as_in_use (ccs->aes);
	}

	/* Always lock PKA access before RNG to prevent deadlock. */
	if (hw_flags & CCS_KSU_CMD_HW_PKA) {
		ecc_hw_pka_mark_as_in_use (ccs->pka);
	}

	if (hw_flags & CCS_KSU_CMD_HW_RNG) {
		hsp_rng_hw_mark_as_in_use (ccs->rng);

		/* Make sure the RNG has random data ready if the command requires it.  Otherwise, bus
		 * watchdog faults could occur during command execution. */
		if (hw_flags & CCS_KSU_CMD_HW_RNG_NORMAL_MODE) {
			hsp_rng_hw_wait_for_reseed (ccs->rng);
		}

		if (hw_flags & CCS_KSU_CMD_HW_RNG_FW_MODE) {
			hsp_rng_hw_wait_for_entropy_read_done (ccs->rng);
		}
	}

	/* Load the command structure into shared SRAM and submit it for execution. */
	ccs->buffer->cmd = *cmd;
	status = ccs->submit_command (ccs, error_code);

	/* Release the RNG hardware since the command has finished.  If the command used FW mode of the
	 * RNG, ensure the calibration values are correct.  Some versions of CCS have a bug that
	 * overwrites calibration values when switching out of FW mode. */
	if (hw_flags & CCS_KSU_CMD_HW_RNG) {
		if (hw_flags & CCS_KSU_CMD_HW_RNG_FW_MODE) {
			rng_status = hsp_rng_hw_recalibrate (ccs->rng);
		}

		hsp_rng_hw_mark_as_available (ccs->rng);
	}

	/* Release the rest of the external hardware that was used. */
	if (hw_flags & CCS_KSU_CMD_HW_PKA) {
		ecc_hw_pka_mark_as_available (ccs->pka);
	}

	if (hw_flags & CCS_KSU_CMD_HW_AES) {
		hsp_aes_mark_as_available (ccs->aes);
	}

	if (hw_flags & CCS_KSU_CMD_HW_SHA) {
		hs_sha_mark_as_available (ccs->sha);
	}

	/* A failure with the RNG should be extremely rare or impossible.  If it does happen, we want to
	 * fail the overall operation since the RNG could be left in a bad state for future use. */
	if ((status == 0) && (rng_status != 0)) {
		status = rng_status;
	}

	/* Get the command output before releasing the lock. */
	if ((output != NULL) && (status == 0)) {
		memcpy (output, &ccs->buffer->output, output_len);
	}

exit:
	if (hw_flags & CCS_KSU_CMD_HW_LOCK_CCS) {
		buffer_zeroize (ccs->buffer, sizeof (*ccs->buffer));
		platform_mutex_unlock (&ccs->state->lock);
	}

	return status;
}

/**
 * Convert the hardware status register value to an execution status code.
 *
 * @param status The raw status register value immediately upon command completion.
 * @param error_code Error to return when the the command failure bit is set.
 *
 * @return The error code to return for the command execution.
 */
static int ccs_ksu_parse_command_status (uint32_t status, int error_code)
{
	if (status & CCS_REGS_STATUS_COMPLETED_FIELD_MASK) {
		return 0;
	}
	else if (status & CCS_REGS_STATUS_COMMAND_ERROR_FIELD_MASK) {
		/* For generic command failures, return a more specific error code. */
		return error_code;
	}
	else if (status == 0) {
		/* The command was never accepted by the HW. */
		return CCS_KSU_CMD_NOT_STARTED;
	}
	else {
		/* For other errors, return the detailed error bits. */
		return CCS_KSU_HW_ERROR (status);
	}
}

int ccs_ksu_submit_command_polling (const struct ccs_ksu *ccs, int error_code)
{
	return crypto_hw_submit_command_polling (ccs->buffer, &ccs->regs->command, &ccs->regs->status,
		CCS_REGS_STATUS_BUSY_FIELD_MASK, ccs_ksu_parse_command_status, error_code,
		CCS_KSU_CMD_EXE_TIMEOUT);
}

bool ccs_ksu_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param)
{
	const struct ccs_ksu *ccs = TO_DERIVED_TYPE (handler, const struct ccs_ksu, base_irq);

	UNUSED (param);

	return crypto_hw_handle_interrupt (ccs->irq,
		CRYPTO_HW_IRQ_BIT_MASK (CRYPTO_DONE_INTSTS, CCS_DONE), &ccs->state->done);
}

int ccs_ksu_submit_command_interrupt (const struct ccs_ksu *ccs, int error_code)
{
	return crypto_hw_submit_command_interrupt (ccs->irq,
		CRYPTO_HW_IRQ_BIT_MASK (CRYPTO_DONE_INTSTS, CCS_DONE), &ccs->state->done, ccs->buffer,
		&ccs->regs->command, &ccs->regs->status, CCS_REGS_STATUS_BUSY_FIELD_MASK,
		ccs_ksu_parse_command_status, error_code, CCS_KSU_CMD_EXE_TIMEOUT);
}

/**
 * Determine the key size based on key attributes.
 *
 * @param attr Key attributes that determine the key length.
 *
 * @return Length of the key.
 */
#define	ccs_ksu_key_size(attr)  \
	((attr & CCS_KSU_ATTR_KEY_SIZE_384) ? SP_MSG_384_SIZE : SP_MSG_256_SIZE)

/**
 * Determine the signature size based on key attributes.
 *
 * @param attr Key attributes that determine the key length.
 *
 * @return Length of the key.
 */
#define	ccs_ksu_signature_size(attr)    \
	((attr & CCS_KSU_ATTR_KEY_SIZE_384) ? \
		SP_ECDSA_P384_SIGNATURE_SIZE : SP_ECDSA_P256_SIGNATURE_SIZE)

/**
 * Get the attributes for a KSU key slot.
 *
 * @param ccs The CCS instance to query.
 * @param key_slot The key slot to query.
 *
 * @return Attributes for the key slot.  If the slot is not valid, this will be 0.
 */
static uint32_t ccs_ksu_get_slot_attributes (const struct ccs_ksu *ccs, uint8_t key_slot)
{
	struct ksu_key_slot *key;
	bool valid;

	valid = ccs_ksu_get_key_slot_address (ccs, key_slot, (uint32_t*) &key);
	if (valid) {
		return key->attributes;
	}
	else {
		return 0;
	}
}

int ccs_ksu_is_key_slot_valid (const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;

	if (ccs_hw == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (key_slot < ccs_hw->key_slots) {
		return 0;
	}
	else {
		return CCS_KSU_UNSUPPORTED_KEY_SLOT;
	}
}

int ccs_ksu_get_key_attributes (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t *key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;

	if ((ccs_hw == NULL) || (key_attributes == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (key_slot >= ccs_hw->key_slots) {
		return CCS_KSU_UNSUPPORTED_KEY_SLOT;
	}

	*key_attributes = ccs_ksu_get_slot_attributes (ccs_hw, key_slot);

	return 0;
}

int ccs_ksu_set_key (const struct ccs_ksu_interface *ccs, const SP_MSG_384 *key, uint8_t key_slot,
	uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_SET_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[1] = (uint32_t) key;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd,
		ccs_ksu_reverse_key (key_attributes) | ccs_ksu_key_size (key_attributes), NULL, 0,
		CCS_KSU_SET_KEY_FAILED, CCS_KSU_CMD_HW_LOCK_CCS);
}

#ifdef CCS_KSU_ENABLE_SEND_KEY
int ccs_ksu_send_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot, uint32_t dest_addr)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_SEND_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[1] = dest_addr;

	return ccs_ksu_execute_command (ccs_hw, &cmd, NULL, NULL, 0, CCS_KSU_SEND_KEY_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS);
}
#endif

int ccs_ksu_generate_random_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if (ccs_hw == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_GEN_RANDOM_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd, 0, NULL, 0, CCS_KSU_RANDOM_KEY_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_RNG_NORMAL_MODE);
}

int ccs_ksu_derive_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	const SP_MSG_384 *context, uint8_t key_slot, uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (context == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_KDF_KEY2;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_in;
	cmd.address[1] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[2] = (uint32_t) context;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd, SP_MSG_384_SIZE, NULL, 0,
		CCS_KSU_DERIVE_KEY_FAILED, CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_SHA);
}

int ccs_ksu_derive_key_using_pcr (const struct ccs_ksu_interface *ccs, uint8_t key_in, uint8_t pcr,
	uint8_t key_slot, uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if (ccs_hw == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_KDF_PCR2;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_in;
	cmd.address[1] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[2] = CCS_KSU_PCR_SLOT | pcr;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd, 0, NULL, 0, CCS_KSU_DERIVE_KEY_PCR_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_SHA);
}

int ccs_ksu_generate_random_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if (ccs_hw == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_GEN_RANDOM_ECC_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd, 0, NULL, 0, CCS_KSU_RANDOM_ECC_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_RNG_NORMAL_MODE);
}

int ccs_ksu_derive_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in, uint8_t key_slot,
	uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if (ccs_hw == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_DERIVE_ECC_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_in;
	cmd.address[1] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd, 0, NULL, 0, CCS_KSU_DERIVE_ECC_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_RNG_FW_MODE);
}

int ccs_ksu_derive_fw_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in, SP_MSG_384 *key,
	uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};
	int status;

	if ((ccs_hw == NULL) || (key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_DERIVE_ECC_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_in;
	cmd.address[1] = (uint32_t) &ccs_hw->buffer->output;
	cmd.attributes = key_attributes;

	status = ccs_ksu_execute_command (ccs_hw, &cmd, 0, (uint8_t*) key, SP_MSG_384_SIZE,
		CCS_KSU_DERIVE_FW_ECC_FAILED, CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_RNG_FW_MODE);
	if (status == 0) {
		buffer_reverse (key->AsBytes, ccs_ksu_key_size (key_attributes));
	}

	return status;
}

int ccs_ksu_export_fw_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	SP_MSG_384 *key, uint32_t *key_attributes)
{
	if ((ccs == NULL) || (key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	UNUSED (key_slot);
	UNUSED (key_attributes);

	/* Private keys cannot be read from KSU memory by firmware. */
	return CCS_KSU_PRIVATE_KEY_PROTECTED;
}

int ccs_ksu_get_ecc_public_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	SP_ECDSA_P384_PUBLIC *public_key, uint32_t *key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};
	uint32_t attributes;
	int status;

	if ((ccs_hw == NULL) || (public_key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_DERIVE_ECC_PUBLIC;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[1] = (uint32_t) &ccs_hw->buffer->output;

	platform_mutex_lock (&ccs_hw->state->lock);
	attributes = ccs_ksu_get_slot_attributes (ccs_hw, key_slot);

	status = ccs_ksu_execute_command (ccs_hw, &cmd, 0, public_key->AsBytes,
		SP_ECDSA_P384_PUBLIC_KEY_SIZE, CCS_KSU_ECC_PUBLIC_FAILED,
		CCS_KSU_CMD_HW_PKA | CCS_KSU_CMD_HW_RNG_NORMAL_MODE);
	if (status == 0) {
		union ccs_ksu_ecc_public_key *ecc = (union ccs_ksu_ecc_public_key*) public_key;

		if (!(attributes & CCS_KSU_ATTR_KEY_SIZE_384)) {
			buffer_reverse (ecc->p256.Parts.X.AsBytes, SP_MSG_256_SIZE);
			buffer_reverse (ecc->p256.Parts.Y.AsBytes, SP_MSG_256_SIZE);
		}
		else {
			buffer_reverse (ecc->p384.Parts.X.AsBytes, SP_MSG_384_SIZE);
			buffer_reverse (ecc->p384.Parts.Y.AsBytes, SP_MSG_384_SIZE);
		}

		if (key_attributes) {
			*key_attributes = attributes;
		}
	}

	buffer_zeroize (ccs_hw->buffer, sizeof (*ccs_hw->buffer));
	platform_mutex_unlock (&ccs_hw->state->lock);

	return status;
}

int ccs_ksu_certify_ecc_public_key (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	uint8_t pcr, uint8_t key_slot, const SP_MSG_384 *sign_data, SP_ECDSA_P384_PUBLIC *public_key,
	SP_ECDSA_P384_SIGNATURE *signature, uint32_t *public_key_attributes,
	uint32_t *signing_key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};
	uint32_t pub_attributes;
	uint32_t sig_attributes;
	int status;

	if ((ccs_hw == NULL) || (sign_data == NULL) || (public_key == NULL) || (signature == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_CERTIFY_ECC_PUBLIC_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | signing_key;
	cmd.address[1] = CCS_KSU_PCR_SLOT | pcr;
	cmd.address[2] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[3] = (uint32_t) sign_data;
	cmd.address4 = (uint32_t) &ccs_hw->buffer->output;

	platform_mutex_lock (&ccs_hw->state->lock);

	pub_attributes = ccs_ksu_get_slot_attributes (ccs_hw, key_slot);
	sig_attributes = ccs_ksu_get_slot_attributes (ccs_hw, signing_key);

	/* Do not have the command execution copy anything.  The output buffer needs to be copied to
	 * the user buffers based on the types of keys that were used. */
	status = ccs_ksu_execute_command (ccs_hw, &cmd, SP_MSG_384_SIZE, NULL, 0,
		CCS_KSU_CERTIFY_ECC_FAILED,
		CCS_KSU_CMD_HW_SHA | CCS_KSU_CMD_HW_PKA | CCS_KSU_CMD_HW_RNG_NORMAL_MODE);
	if (status == 0) {
		union {
			struct {
				SP_ECDSA_P256_PUBLIC key;
				union ccs_ksu_ecc_signature sig;
			} p256;
			struct {
				SP_ECDSA_P384_PUBLIC key;
				SP_ECDSA_P384_SIGNATURE sig;
			} p384;
		} *tmp_output = (void*) &ccs_hw->buffer->output;
		union ccs_ksu_ecc_public_key *ecc = (union ccs_ksu_ecc_public_key*) public_key;
		union ccs_ksu_ecc_signature *ecdsa = (union ccs_ksu_ecc_signature*) signature;

		if (!(pub_attributes & CCS_KSU_ATTR_KEY_SIZE_384)) {
			memcpy (ecc->p256.AsBytes, tmp_output->p256.key.AsBytes, SP_ECDSA_P256_PUBLIC_KEY_SIZE);
			buffer_reverse (ecc->p256.Parts.X.AsBytes, SP_MSG_256_SIZE);
			buffer_reverse (ecc->p256.Parts.Y.AsBytes, SP_MSG_256_SIZE);

			if (!(sig_attributes & CCS_KSU_ATTR_KEY_SIZE_384)) {
				memcpy (ecdsa->p256.AsBytes, tmp_output->p256.sig.p256.AsBytes,
					SP_ECDSA_P256_SIGNATURE_SIZE);
				buffer_reverse (ecdsa->p256.Parts.R.AsBytes, SP_MSG_256_SIZE);
				buffer_reverse (ecdsa->p256.Parts.S.AsBytes, SP_MSG_256_SIZE);
			}
			else {
				memcpy (ecdsa->p384.AsBytes, tmp_output->p256.sig.p384.AsBytes,
					SP_ECDSA_P384_SIGNATURE_SIZE);
				buffer_reverse (ecdsa->p384.Parts.R.AsBytes, SP_MSG_384_SIZE);
				buffer_reverse (ecdsa->p384.Parts.S.AsBytes, SP_MSG_384_SIZE);
			}
		}
		else {
			memcpy (ecc->p384.AsBytes, tmp_output->p384.key.AsBytes, SP_ECDSA_P384_PUBLIC_KEY_SIZE);
			buffer_reverse (ecc->p384.Parts.X.AsBytes, SP_MSG_384_SIZE);
			buffer_reverse (ecc->p384.Parts.Y.AsBytes, SP_MSG_384_SIZE);

			memcpy (ecdsa->p384.AsBytes, tmp_output->p384.sig.AsBytes,
				SP_ECDSA_P384_SIGNATURE_SIZE);
			buffer_reverse (ecdsa->p384.Parts.R.AsBytes, SP_MSG_384_SIZE);
			buffer_reverse (ecdsa->p384.Parts.S.AsBytes, SP_MSG_384_SIZE);
		}

		if (public_key_attributes) {
			*public_key_attributes = pub_attributes;
		}
		if (signing_key_attributes) {
			*signing_key_attributes = sig_attributes;
		}
	}

	buffer_zeroize (ccs_hw->buffer, sizeof (*ccs_hw->buffer));
	platform_mutex_unlock (&ccs_hw->state->lock);

	return status;
}

int ccs_ksu_ecdh_key_exchange (const struct ccs_ksu_interface *ccs, uint8_t key_in,	uint8_t key_out,
	const uint8_t *partner_public_key_and_hash, size_t input_len, uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (partner_public_key_and_hash == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (!sram_is_buffer_in_shared_sram (partner_public_key_and_hash, input_len)) {
		if (input_len > (sizeof (ccs_hw->buffer->input) + sizeof (ccs_hw->buffer->output))) {
			return CCS_KSU_INVALID_ADDRESS;
		}
	}

	if (input_len != (SP_ECDSA_P384_PUBLIC_KEY_SIZE + SP_MSG_384_SIZE)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_ECDH_KEY_EXCHANGE2;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_in;
	cmd.address[1] = CCS_KSU_KEY_SLOT | key_out;
	cmd.address[2] = (uint32_t) partner_public_key_and_hash;
	cmd.address[3] = NULL;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd, SP_ECDSA_P384_PUBLIC_KEY_SIZE + SP_MSG_384_SIZE,
		NULL, 0, CCS_KSU_ECDH_KEY_EXCHANGE_FAILED, CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_PKA);
}

int ccs_ksu_ecc_sign (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const SP_MSG_384 *digest, const struct rng_engine *rng, SP_ECDSA_P384_SIGNATURE *signature,
	uint32_t *key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};
	uint32_t attributes;
	int status;

	if ((ccs_hw == NULL) || (digest == NULL) || (signature == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (rng != NULL) {
		/* CCS hardware cannot support signing with a different random number source. */
		return CCS_KSU_SIGN_RNG_UNSUPPORTED;
	}

	cmd.command_code = CCS_CMD_CODE_ECC_SIGN;
	cmd.address[0] = CCS_KSU_KEY_SLOT | signing_key;
	cmd.address[1] = (uint32_t) &ccs_hw->buffer->output;
	cmd.address[2] = (uint32_t) digest;

	platform_mutex_lock (&ccs_hw->state->lock);
	attributes = ccs_ksu_get_slot_attributes (ccs_hw, signing_key);

	status = ccs_ksu_execute_command (ccs_hw, &cmd,
		CCS_KSU_INPUT_REVERSE | ccs_ksu_key_size (attributes), (uint8_t*) signature,
		ccs_ksu_signature_size (attributes), CCS_KSU_ECC_SIGN_FAILED,
		CCS_KSU_CMD_HW_PKA | CCS_KSU_CMD_HW_RNG_NORMAL_MODE);
	if (status == 0) {
		union ccs_ksu_ecc_signature *ecdsa = (union ccs_ksu_ecc_signature*) signature;

		if (!(attributes & CCS_KSU_ATTR_KEY_SIZE_384)) {
			buffer_reverse (ecdsa->p256.Parts.R.AsBytes, SP_MSG_256_SIZE);
			buffer_reverse (ecdsa->p256.Parts.S.AsBytes, SP_MSG_256_SIZE);
		}
		else {
			buffer_reverse (ecdsa->p384.Parts.R.AsBytes, SP_MSG_384_SIZE);
			buffer_reverse (ecdsa->p384.Parts.S.AsBytes, SP_MSG_384_SIZE);
		}

		if (key_attributes) {
			*key_attributes = attributes;
		}
	}

	buffer_zeroize (ccs_hw->buffer, sizeof (*ccs_hw->buffer));
	platform_mutex_unlock (&ccs_hw->state->lock);

	return status;
}

int ccs_ksu_ecdsa_sign_message (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const uint8_t *message, size_t length, const struct hash_engine *hash, enum hash_type hash_algo,
	SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes)
{
	UNUSED (ccs);
	UNUSED (signing_key);
	UNUSED (message);
	UNUSED (length);
	UNUSED (hash);
	UNUSED (hash_algo);
	UNUSED (signature);
	UNUSED (key_attributes);

	/* CCS hardware cannot support ECDSA sign in a FIPS compliant way. */
	return CCS_KSU_UNSUPPORTED_CMD;
}

int ccs_ksu_ecdsa_sign_hash (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes)
{
	UNUSED (ccs);
	UNUSED (signing_key);
	UNUSED (hash);
	UNUSED (signature);
	UNUSED (key_attributes);

	/* CCS hardware cannot support ECDSA sign in a FIPS compliant way. */
	return CCS_KSU_UNSUPPORTED_CMD;
}

int ccs_ksu_ecdsa_sign_hash_and_finish (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes)
{
	UNUSED (ccs);
	UNUSED (signing_key);
	UNUSED (hash);
	UNUSED (signature);
	UNUSED (key_attributes);

	/* CCS hardware cannot support ECDSA sign in a FIPS compliant way. */
	return CCS_KSU_UNSUPPORTED_CMD;
}

int ccs_ksu_wrap_key_buffer (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_384 *key, SP_MSG_512 *wrapped_key, uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (key == NULL) || (wrapped_key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_WRAP_INPUT;
	cmd.address[0] = CCS_KSU_KEY_SLOT | kek_slot;
	cmd.address[1] = (uint32_t) key;
	cmd.address[2] = (uint32_t) &ccs_hw->buffer->output;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd,
		ccs_ksu_reverse_key (key_attributes) | SP_MSG_384_SIZE, (uint8_t*) wrapped_key,
		SP_MSG_512_SIZE, CCS_KSU_WRAP_BUFFER_FAILED, CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_AES);
}

int ccs_ksu_unwrap_key (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_512 *wrapped_key, uint8_t key_slot)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (wrapped_key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_UNWRAP_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | kek_slot;
	cmd.address[1] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[2] = (uint32_t) wrapped_key;

	return ccs_ksu_execute_command (ccs_hw, &cmd, SP_MSG_512_SIZE, NULL, 0, CCS_KSU_UNWRAP_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_AES);
}

int ccs_ksu_burn_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if (ccs_hw == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_BURN_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_slot;

	/* The BurnKey command has an external dependency on the GFC.  However, this dependency is not
	 * handled by the driver because BurnKey will only get executed in ROM and/or other highly
	 * controlled, single threaded environments. */
	return ccs_ksu_execute_command (ccs_hw, &cmd, 0, NULL, 0, CCS_KSU_BURN_KEY_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS);
}

int ccs_ksu_reset_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if (ccs_hw == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_REINIT_PCR;
	cmd.address[0] = CCS_KSU_PCR_SLOT | pcr;

	return ccs_ksu_execute_command (ccs_hw, &cmd, 0, NULL, 0, CCS_KSU_RESET_PCR_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS);
}

int ccs_ksu_extend_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr, const SP_MSG_384 *digest)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (digest == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_EXTEND_PCR_384;
	cmd.address[0] = CCS_KSU_PCR_SLOT | pcr;
	cmd.address[1] = (uint32_t) digest;

	return ccs_ksu_execute_command (ccs_hw, &cmd, SP_MSG_384_SIZE, NULL, 0,
		CCS_KSU_EXTEND_PCR_FAILED, CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_SHA);
}

int ccs_ksu_get_pcr_value (const struct ccs_ksu_interface *ccs, uint8_t pcr, SP_MSG_384 *value)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	uint32_t address;
	SP_MSG_384 tmp;

	if ((ccs == NULL) || (value == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (!ccs_ksu_get_pcr_slot_address (ccs_hw, pcr, &address)) {
		return CCS_KSU_UNSUPPORTED_PCR_SLOT;
	}

	/* The PCR memory is not byte addressable, so copy it to a temporary buffer whose alignment is
	 * known. */
	memcpy (tmp.AsBytes, (void*) (uintptr_t) address, SP_MSG_384_SIZE);

	/* Then copy to the output buffer. */
	memcpy (value->AsBytes, tmp.AsBytes, SP_MSG_384_SIZE);

	return 0;
}

int ccs_ksu_hmac (const struct ccs_ksu_interface *ccs, uint8_t key_slot, const uint8_t *data,
	size_t length, SP_MSG_384 *hmac, uint32_t *key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};
	size_t input_len = CCS_KSU_INPUT_NO_PROCESSING;
	int status;

	if ((ccs_hw == NULL) || (data == NULL) || (hmac == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (!sram_is_buffer_in_shared_sram (data, length)) {
		if (length > (sizeof (ccs_hw->buffer->input) + sizeof (ccs_hw->buffer->output))) {
			return CCS_KSU_INVALID_ADDRESS;
		}

		/* If the data buffer is not in shared memory but can fit into the command buffer, the data
		 * will be copied into shared SRAM rather than failing the command.  Both the input and
		 * output buffers can be used for this since the data will be fully consumed before the HMAC
		 * digest is written out. */
		input_len = length;
	}

	cmd.command_code = CCS_CMD_CODE_HMAC;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[1] = (uint32_t) &ccs_hw->buffer->output;
	cmd.address[2] = (uint32_t) data;
	cmd.address[3] = length;

	platform_mutex_lock (&ccs_hw->state->lock);

	status = ccs_ksu_execute_command (ccs_hw, &cmd, input_len, hmac->AsBytes, SP_MSG_384_SIZE,
		CCS_KSU_HMAC_FAILED, CCS_KSU_CMD_HW_SHA);

	if ((status == 0) && key_attributes) {
		*key_attributes = ccs_ksu_get_slot_attributes (ccs_hw, key_slot);
	}

	buffer_zeroize (ccs_hw->buffer, sizeof (*ccs_hw->buffer));
	platform_mutex_unlock (&ccs_hw->state->lock);

	return status;
}

/**
 * Initialize a driver for interfacing the the CCS and KSU using SHACK2.
 *
 * @param ccs The CCS driver instance to initialize.
 * @param state Variable context for the CCS driver instance.  The must be uninitialized.
 * @param regs Base address for the CCS registers.
 * @param sha Interface to the HS-SHA used by this CCS instance.
 * @param aes Interface to the AES used by this CCS instance.
 * @param pka Interface to the PKA used by this CCS instance.
 * @param rng Interface to the RNG used by this CCS instance.
 * @param cmd_buffer Location in HSP shared RAM where CCS commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param keys Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 * @param pcrs Address for PCR storage in the KSU.
 * @param num_pcrs The number of PCRs available in the KSU.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
static int ccs_ksu_init (struct ccs_ksu *ccs, struct ccs_ksu_state *state, struct Ccs_regs *regs,
	const struct hs_sha *sha, const struct hsp_aes *aes, const struct ecc_hw_pka *pka,
	const struct hsp_rng_hw *rng, struct ccs_cmd_buffer *cmd_buffer,
	const struct ksu_key_slot *keys, size_t num_keys, const struct ksu_pcr_slot *pcrs,
	size_t num_pcrs)
{
	if ((ccs == NULL) || (state == NULL) || (regs == NULL) || (sha == NULL) || (aes == NULL) ||
		(pka == NULL) || (rng == NULL) || (keys == NULL) || (pcrs == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	memset (ccs, 0, sizeof (struct ccs_ksu));

	ccs->base.is_key_slot_valid = ccs_ksu_is_key_slot_valid;
	ccs->base.get_key_attributes = ccs_ksu_get_key_attributes;
	ccs->base.set_key = ccs_ksu_set_key;
#ifdef CCS_KSU_ENABLE_SEND_KEY
	ccs->base.send_key = ccs_ksu_send_key;
#endif
	ccs->base.generate_random_key = ccs_ksu_generate_random_key;
	ccs->base.derive_key = ccs_ksu_derive_key;
	ccs->base.derive_key_using_pcr = ccs_ksu_derive_key_using_pcr;
	ccs->base.generate_random_ecc_key = ccs_ksu_generate_random_ecc_key;
	ccs->base.derive_ecc_key = ccs_ksu_derive_ecc_key;
	ccs->base.derive_fw_ecc_key = ccs_ksu_derive_fw_ecc_key;
	ccs->base.export_fw_ecc_key = ccs_ksu_export_fw_ecc_key;
	ccs->base.get_ecc_public_key = ccs_ksu_get_ecc_public_key;
	ccs->base.certify_ecc_public_key = ccs_ksu_certify_ecc_public_key;
	ccs->base.ecdh_key_exchange = ccs_ksu_ecdh_key_exchange;
	ccs->base.ecc_sign = ccs_ksu_ecc_sign;
	ccs->base.ecdsa_sign_message = ccs_ksu_ecdsa_sign_message;
	ccs->base.ecdsa_sign_hash = ccs_ksu_ecdsa_sign_hash;
	ccs->base.ecdsa_sign_hash_and_finish = ccs_ksu_ecdsa_sign_hash_and_finish;
	ccs->base.wrap_key_buffer = ccs_ksu_wrap_key_buffer;
	ccs->base.unwrap_key = ccs_ksu_unwrap_key;
	ccs->base.burn_key = ccs_ksu_burn_key;
	ccs->base.reset_pcr = ccs_ksu_reset_pcr;
	ccs->base.extend_pcr = ccs_ksu_extend_pcr;
	ccs->base.get_pcr_value = ccs_ksu_get_pcr_value;
	ccs->base.hmac = ccs_ksu_hmac;

	ccs->state = state;
	ccs->regs = regs;
	ccs->sha = sha;
	ccs->aes = aes;
	ccs->pka = pka;
	ccs->rng = rng;
	ccs->buffer = cmd_buffer;
	ccs->keys = keys;
	ccs->key_slots = num_keys;
	ccs->pcrs = pcrs;
	ccs->pcr_slots = num_pcrs;

	return ccs_ksu_init_state (ccs);
}

/**
 * Initialize a driver for interfacing the the CCS and KSU using SHACK2.  CCS operations will enter
 * a busy waiting loop, actively polling the hardware to determine when they have finished.
 *
 * The interrupt handler will be null for instances initialized in this way.
 *
 * @param ccs The CCS driver instance to initialize.
 * @param state Variable context for the CCS driver instance.  The must be uninitialized.
 * @param regs Base address for the CCS registers.
 * @param sha Interface to the HS-SHA used by this CCS instance.
 * @param aes Interface to the AES used by this CCS instance.
 * @param pka Interface to the PKA used by this CCS instance.
 * @param rng Interface to the RNG used by this CCS instance.
 * @param cmd_buffer Location in HSP shared RAM where CCS commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param keys Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 * @param pcrs Address for PCR storage in the KSU.
 * @param num_pcrs The number of PCRs available in the KSU.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int ccs_ksu_init_polling (struct ccs_ksu *ccs, struct ccs_ksu_state *state, struct Ccs_regs *regs,
	const struct hs_sha *sha, const struct hsp_aes *aes, const struct ecc_hw_pka *pka,
	const struct hsp_rng_hw *rng, struct ccs_cmd_buffer *cmd_buffer,
	const struct ksu_key_slot *keys, size_t num_keys, const struct ksu_pcr_slot *pcrs,
	size_t num_pcrs)
{
	int status;

	status = ccs_ksu_init (ccs, state, regs, sha, aes, pka, rng, cmd_buffer, keys, num_keys, pcrs,
		num_pcrs);
	if (status == 0) {
		ccs->submit_command = ccs_ksu_submit_command_polling;
	}

	return status;
}

/**
 * Initialize a driver for interfacing the the CCS and KSU using SHACK2.  CCS operations will block,
 * waiting for an interrupt to indicate when the hardware has finished.
 *
 * @param ccs The CCS driver instance to initialize.
 * @param state Variable context for the CCS driver instance.  The must be uninitialized.
 * @param regs Base address for the CCS registers.
 * @param sha Interface to the HS-SHA used by this CCS instance.
 * @param aes Interface to the AES used by this CCS instance.
 * @param pka Interface to the PKA used by this CCS instance.
 * @param rng Interface to the RNG used by this CCS instance.
 * @param cmd_buffer Location in HSP shared RAM where CCS commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param keys Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 * @param pcrs Address for PCR storage in the KSU.
 * @param num_pcrs The number of PCRs available in the KSU.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int ccs_ksu_init_interrupt (struct ccs_ksu *ccs, struct ccs_ksu_state *state, struct Ccs_regs *regs,
	struct Creg_regs_creg_crypto_group *irq_regs, const struct hs_sha *sha,
	const struct hsp_aes *aes, const struct ecc_hw_pka *pka, const struct hsp_rng_hw *rng,
	struct ccs_cmd_buffer *cmd_buffer, const struct ksu_key_slot *keys, size_t num_keys,
	const struct ksu_pcr_slot *pcrs, size_t num_pcrs)
{
	int status;

	if (irq_regs == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	status = ccs_ksu_init (ccs, state, regs, sha, aes, pka, rng, cmd_buffer, keys, num_keys, pcrs,
		num_pcrs);
	if (status == 0) {
		ccs->base_irq.handle_interrupt = ccs_ksu_handle_interrupt;
		ccs->submit_command = ccs_ksu_submit_command_interrupt;

		ccs->irq = irq_regs;
	}

	return status;
}

/**
 * Initialize only the variable state for a CCS/KSU driver using either SHACK1 or SHACK2.  The rest
 * of the driver is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param ccs The CCS driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int ccs_ksu_init_state (const struct ccs_ksu *ccs)
{
	int status;

	if ((ccs == NULL) || (ccs->state == NULL) || (ccs->regs == NULL) || (ccs->sha == NULL) ||
		(ccs->aes == NULL) || (ccs->pka == NULL) || (ccs->rng == NULL) || (ccs->keys == NULL) ||
		(ccs->pcrs == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if ((ccs->base_irq.handle_interrupt != NULL) && (ccs->irq == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (!sram_is_buffer_in_shared_sram (ccs->buffer, sizeof (*ccs->buffer))) {
		return CCS_KSU_INVALID_ADDRESS;
	}

	if ((uintptr_t) ccs->buffer & 0x3) {
		return CCS_KSU_ADDRESS_NOT_ALIGNED;
	}

	memset (ccs->state, 0, sizeof (struct ccs_ksu_state));

	status = platform_semaphore_init (&ccs->state->done);
	if (status != 0) {
		return status;
	}

	status = platform_mutex_init (&ccs->state->lock);
	if (status != 0) {
		platform_semaphore_free (&ccs->state->done);
	}

	return status;
}

/**
 * Release the resources used by a CCS/KSU driver.
 *
 * @param ccs The CCS driver to release.
 */
void ccs_ksu_release (const struct ccs_ksu *ccs)
{
	if (ccs) {
		platform_mutex_free (&ccs->state->lock);
		platform_semaphore_free (&ccs->state->done);
	}
}
