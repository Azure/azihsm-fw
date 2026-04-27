// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_cmd.h"
#include "hsp_top.h"
#include "common/buffer_util.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "drivers/crypto_hw.h"
#include "drivers/hs_sha.h"
#include "drivers/sram.h"
#include "splibs/inc/sptypes.h"


/**
 * Convert the hardware status register value to an execution status code.
 *
 * @param status The raw status register value immediately upon command completion.
 * @param error_code Error to return when the the command failure bit is set.
 *
 * @return The error code to return for the command execution.
 */
static int hs_sha_parse_command_status (uint32_t status, int error_code)
{
	if (status & SHA_REGS_SHA_STATUS_COMPLETE_FIELD_MASK) {
		return 0;
	}
	else if (status & SHA_REGS_SHA_STATUS_ERROR_CMD_FIELD_MASK) {
		/* For generic command failures, return a more specific error code. */
		return error_code;
	}
	else if (status == 0) {
		/* The command was never accepted by the HW. */
		return HS_SHA_CMD_NOT_STARTED;
	}
	else {
		/* For other errors, return the detailed error bits. */
		return HS_SHA_HW_ERROR (status);
	}
}

int hs_sha_execute_command_polling (const struct hs_sha *sha, int error_code)
{
	return crypto_hw_submit_command_polling (&sha->buffer->cmd, &sha->regs->SHA_CMD,
		&sha->regs->SHA_STATUS, SHA_REGS_SHA_STATUS_BUSY_FIELD_MASK, hs_sha_parse_command_status,
		error_code, HS_SHA_CMD_EXE_TIMEOUT);
}

bool hs_sha_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param)
{
	const struct hs_sha *sha = (const struct hs_sha*) handler;

	UNUSED (param);

	return crypto_hw_handle_interrupt (sha->irq,
		CRYPTO_HW_IRQ_BIT_MASK (CRYPTO_DONE_INTSTS, SHA_DONE), &sha->state->done);
}

int hs_sha_execute_command_interrupt (const struct hs_sha *sha, int error_code)
{
	return crypto_hw_submit_command_interrupt (sha->irq,
		CRYPTO_HW_IRQ_BIT_MASK (CRYPTO_DONE_INTSTS, SHA_DONE), &sha->state->done, &sha->buffer->cmd,
		&sha->regs->SHA_CMD, &sha->regs->SHA_STATUS, SHA_REGS_SHA_STATUS_BUSY_FIELD_MASK,
		hs_sha_parse_command_status, error_code, HS_SHA_CMD_EXE_TIMEOUT);
}

/**
 * Initialize a driver instance for the HS-SHA.
 *
 * @param sha The HS-SHA driver to initialize.
 * @param state Variable context for the HS-SHA driver instance.  The must be uninitialized.
 * @param regs Base address for the HS-SHA registers.
 * @param cmd_buffer Location in HSP shared RAM where HS-SHA commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buffer Location in HSP shared RAM that can be used to stage data for HS-SHA, when
 * necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for SHA-512 (128 bytes).
 *
 * @return 0 if the driver was initialized successfully or an error code.
 */
static int hs_sha_init (struct hs_sha *sha, struct hs_sha_state *state, struct Sha_regs *regs,
	struct hs_sha_cmd_buffer *cmd_buffer, uint8_t *msg_buffer, size_t buffer_length)
{
	if ((sha == NULL) || (state == NULL) || (regs == NULL) || (buffer_length < SHA512_BLOCK_SIZE)) {
		return HS_SHA_INVALID_ARGUMENT;
	}

	memset (sha, 0, sizeof (struct hs_sha));

	sha->state = state;
	sha->regs = regs;
	sha->buffer = cmd_buffer;
	sha->msg_buffer = msg_buffer;
	sha->max_msg = buffer_length;

	return hs_sha_init_state (sha);
}

/**
 * Initialize a driver instance for the HS-SHA.  Hash operations will enter a busy waiting loop,
 * actively polling the hardware to determine when they have finished.
 *
 * The interrupt handler will be null for instances initialized in this way.
 *
 * @param sha The HS-SHA driver to initialize.
 * @param state Variable context for the HS-SHA driver instance.  The must be uninitialized.
 * @param regs Base address for the HS-SHA registers.
 * @param cmd_buffer Location in HSP shared RAM where HS-SHA commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buffer Location in HSP shared RAM that can be used to stage data for HS-SHA, when
 * necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for SHA-512 (128 bytes).
 *
 * @return 0 if the driver was initialized successfully or an error code.
 */
int hs_sha_init_polling (struct hs_sha *sha, struct hs_sha_state *state, struct Sha_regs *regs,
	struct hs_sha_cmd_buffer *cmd_buffer, uint8_t *msg_buffer, size_t buffer_length)
{
	int status;

	status = hs_sha_init (sha, state, regs, cmd_buffer, msg_buffer, buffer_length);
	if (status == 0) {
		sha->execute_command = hs_sha_execute_command_polling;
	}

	return status;
}

/**
 * Initialize a driver instance for the HS-SHA.  Hash operations will block, waiting for an
 * interrupt to indicate when the hardware has finished.
 *
 * @param sha The HS-SHA driver to initialize.
 * @param state Variable context for the HS-SHA driver instance.  The must be uninitialized.
 * @param regs Base address for the HS-SHA registers.
 * @param irq_regs Base address for the CREG registers to control HS-SHA interrupts.
 * @param cmd_buffer Location in HSP shared RAM where HS-SHA commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buffer Location in HSP shared RAM that can be used to stage data for HS-SHA, when
 * necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for SHA-512 (128 bytes).
 *
 * @return 0 if the driver was initialized successfully or an error code.
 */
int hs_sha_init_interrupt (struct hs_sha *sha, struct hs_sha_state *state, struct Sha_regs *regs,
	struct Creg_regs_creg_crypto_group *irq_regs, struct hs_sha_cmd_buffer *cmd_buffer,
	uint8_t *msg_buffer, size_t buffer_length)
{
	int status;

	if (irq_regs == NULL) {
		return HS_SHA_INVALID_ARGUMENT;
	}

	status = hs_sha_init (sha, state, regs, cmd_buffer, msg_buffer, buffer_length);
	if (status == 0) {
		sha->base.handle_interrupt = hs_sha_handle_interrupt;
		sha->execute_command = hs_sha_execute_command_interrupt;

		sha->irq = irq_regs;
	}

	return status;
}

/**
 * Initialize only the variable state for an HS-SHA driver.  The rest of the driver is assumed to
 * have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param sha The HS-SHA driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int hs_sha_init_state (const struct hs_sha *sha)
{
	int status;

	if ((sha == NULL) || (sha->state == NULL) || (sha->regs == NULL) ||
		(sha->max_msg < SHA512_BLOCK_SIZE)) {
		return HS_SHA_INVALID_ARGUMENT;
	}

	if ((sha->base.handle_interrupt != NULL) && (sha->irq == NULL)) {
		return HS_SHA_INVALID_ARGUMENT;
	}

	if (!sram_is_buffer_in_shared_sram (sha->buffer, sizeof (*sha->buffer))) {
		return HS_SHA_INVALID_ADDRESS;
	}

	if ((uintptr_t) sha->buffer & 0x3) {
		return HS_SHA_ADDRESS_NOT_ALIGNED;
	}

	if (!sram_is_buffer_in_shared_sram (sha->msg_buffer, sha->max_msg)) {
		return HS_SHA_INVALID_ADDRESS;
	}

	memset (sha->state, 0, sizeof (struct hs_sha_state));

	status = platform_semaphore_init (&sha->state->done);
	if (status != 0) {
		return status;
	}

	status = platform_mutex_init (&sha->state->lock);
	if (status != 0) {
		platform_semaphore_free (&sha->state->done);
	}

	return status;
}

/**
 * Release an HS-SHA driver instance.
 *
 * @param sha The HS-SHA driver to release.
 */
void hs_sha_release (const struct hs_sha *sha)
{
	if (sha) {
		platform_mutex_free (&sha->state->lock);
		platform_semaphore_free (&sha->state->done);
	}
}

/**
 * Wait for the HS-SHA to be ready to accept a new command and erase the command buffer.
 *
 * @param sha The HS-SHA instance that will be executing the command.
 */
static void hs_sha_start_new_command (const struct hs_sha *sha)
{
	/* Make sure the HS-SHA HW is available for use. */
	platform_mutex_lock (&sha->state->lock);

	/* Spin here to make sure the HS-SHA HW is ready.  This is to clean up after possible FW errors
	 * with previous commands or other abnormal states.  Even when configured to use interrupts,
	 * this will still be a busy loop, for simplicity.  It should typically not get executed. */
	while (sha->regs->SHA_STATUS & SHA_REGS_SHA_STATUS_BUSY_FIELD_MASK) {
	}
	memset (sha->buffer, 0, sizeof (*sha->buffer));
}

/**
 * Calculate the digest for a data buffer.
 *
 * @param sha The HS-SHA instance to use for the digest calculation.
 * @param data Buffer containing the data to hash.
 * @param length Length of the data.
 * @param total_length Total number bytes that were part of the digest calculation.  If the final
 * digest is not being calculated, this value is ignored.
 * @param current Input context to continue calculation of an existing digest.
 * @param type The hash algorithm to use to generate the digest.
 * @param final Flag to indicate if the final digest should be calculated.  If this is false, the
 * input data must be aligned to the hash algorithm block size and must contained some data (i.e.
 * null/0 length is not supported).
 * @param error_code The error code to report for HS-SHA command failures.
 * @param digest Output buffer for the calculated digest.
 * @param digest_length Length of the digest output buffer.
 *
 * @return Length of the calculated digest or an error code.
 */
static int hs_sha_run_digest (const struct hs_sha *sha, const uint8_t *data, size_t length,
	size_t total_length, const SP_MSG_512 *current, enum hash_type type, bool final, int error_code,
	uint8_t *digest, size_t digest_length)
{
	size_t out_length;
	size_t block_size;
	uint32_t sha_mode;
	uint32_t load = 0;
	int status;

	if ((sha == NULL) || (digest == NULL) || ((data == NULL) && ((length != 0) || !final)) ||
		((length == 0) && !final)) {
		return HS_SHA_INVALID_ARGUMENT;
	}

	switch (type) {
		case HASH_TYPE_SHA1:
			out_length = SHA1_HASH_LENGTH;
			block_size = SHA1_BLOCK_SIZE;
			sha_mode = HSP_CMD_SHA_MODE_ENUM_SHA_1;
			break;

		case HASH_TYPE_SHA256:
			out_length = SHA256_HASH_LENGTH;
			block_size = SHA256_BLOCK_SIZE;
			sha_mode = HSP_CMD_SHA_MODE_ENUM_SHA_256;
			break;

		case HASH_TYPE_SHA384:
			out_length = SHA384_HASH_LENGTH;
			block_size = SHA384_BLOCK_SIZE;
			sha_mode = HSP_CMD_SHA_MODE_ENUM_SHA_384;
			break;

		case HASH_TYPE_SHA512:
			out_length = SHA512_HASH_LENGTH;
			block_size = SHA512_BLOCK_SIZE;
			sha_mode = HSP_CMD_SHA_MODE_ENUM_SHA_512;
			break;

		default:
			return HS_SHA_UNSUPPORTED_HASH_TYPE;
	}

	if (!final && (length != HS_SHA_BLOCK_ALIGN (length, block_size))) {
		return HS_SHA_NOT_BLOCK_ALIGNED;
	}

	if (!final) {
		out_length = SP_MSG_512_SIZE;
	}

	if (digest_length < out_length) {
		return HS_SHA_DIGEST_BUFFER_TOO_SMALL;
	}

	hs_sha_start_new_command (sha);

	/* Be sure to incorporate any incoming context as part of the hash calculation.  It is fine to
	 * set the initial_digest pointer in all cases here because:
	 *	1. The pointer is only used when the load_digest flag is set on the command.
	 *	2. It gets used when the data to hash needs to be copied to shared SRAM in chunks. */
	if (current != NULL) {
		memcpy (sha->buffer->digest.AsBytes, current->AsBytes, SP_MSG_512_SIZE);
		load = 1;
	}
	sha->buffer->cmd.initial_digest = (uint32_t) &sha->buffer->digest;

	do {
		size_t temp_len;
		uint32_t flags = 0;

		sha->buffer->cmd.command_code =
			HSP_CMD_HSSHA_CMD_CODE_ID_SET (HSP_CMD_HSSHA_CMD_CODE_ID_RESET) |
			HSP_CMD_HSSHA_CMD_CODE_SHA_MODE_SET (sha_mode) |
			HSP_CMD_HSSHA_CMD_CODE_LOAD_DIGEST_SET (load);

		/* If the input buffer is already is shared SRAM, use it directly.  Otherwise, we need to
		 * make a copy.  When there is no additional input data, just use the shared buffer.  This
		 * ensures a valid address when the provided data pointer is null. */
		if ((length != 0) && sram_is_shared_address (data)) {
			temp_len = length;
			sha->buffer->cmd.message_buff = (uint32_t) data;
		}
		else {
			/* When copying into the shared temp buffer, we need to be sure to not overflow the
			 * space.  This may require multiple commands to be sent to the HS-SHA for each chunk of
			 * data. */
			temp_len = min (length, sha->max_msg);
			memcpy (sha->msg_buffer, data, temp_len);
			sha->buffer->cmd.message_buff = (uint32_t) sha->msg_buffer;
		}

		/* If this is the last chunk of data to be hashed, enable EOM padding.  If there is more
		 * data, get the intermediate digest to use with the next chunk. */
		if (final && (temp_len == length)) {
			flags = HSP_CMD_HSSHA_CMD_CODE_AUTO_PAD_SET (1);
		}
		else {
			/* Every chunk, except tha last one, needs to be aligned to the hash algorithm block
			 * size, so adjust the length here to make sure it is appropriate. */
			temp_len = HS_SHA_BLOCK_ALIGN (temp_len, block_size);

			flags = HSP_CMD_HSSHA_CMD_CODE_DONT_TRUNCATE_SET (1) |
				HSP_CMD_HSSHA_CMD_CODE_DIGEST_BYTE_SWAP_SET (1);
			load = 1;
		}

		sha->buffer->cmd.command_code |= flags;
		sha->buffer->cmd.byte_count = total_length;
		sha->buffer->cmd.message_bytes = temp_len;

		/* If the output buffer is already in shared SRAM, use it directly.  Otherwise, we need to
		 * use a temp buffer and copy.  Also use the temp buffer if this command only represents an
		 * intermediate step of the calculation. */
		if (!sram_is_shared_address (digest) ||
			(flags & HSP_CMD_HSSHA_CMD_CODE_DONT_TRUNCATE_FIELD_MASK)) {
			sha->buffer->cmd.digest = (uint32_t) &sha->buffer->digest;
		}
		else {
			sha->buffer->cmd.digest = (uint32_t) digest;
		}

		status = sha->execute_command (sha, error_code);
		if (status == 0) {
			length -= temp_len;
			data += temp_len;
		}
	} while ((length > 0) && (status == 0));

	/* After the digest has been completed for all the provided data, ensure the caller's output
	 * buffer contains the calculated digest and report the length of the output data. */
	if (status == 0) {
		if (sha->buffer->cmd.digest == (uint32_t) &sha->buffer->digest) {
			memcpy (digest, sha->buffer->digest.AsBytes, out_length);
		}

		status = out_length;
	}

	/* Clean up after the operation. */
	buffer_zeroize (sha->buffer, sizeof (*sha->buffer));
	buffer_zeroize (sha->msg_buffer, sha->max_msg);

	platform_mutex_unlock (&sha->state->lock);

	return status;
}

/**
 * Calculate an intermediate digest for a partial message.  This can either extend a previously
 * calculated digest with additional data or start a new calculation.
 *
 * @param sha The HS-SHA instance to use for the digest calculation.
 * @param data Buffer containing the data to hash.
 * @param length Length of the data.  This must be a multiple of the hash algorithm block size.
 * @param current Input context for the current hash being executed.  Set to null to start a new
 * hash.
 * @param type The hash algorithm to use to generate the digest.
 * @param updated Output for the updated hash context.  This can be the same buffer as the current
 * context, but must not be null.
 *
 * @return 0 if the digest was updated successfully or an error code.
 */
int hs_sha_update_digest (const struct hs_sha *sha, const uint8_t *data, size_t length,
	const SP_MSG_512 *current, enum hash_type type, SP_MSG_512 *updated)
{
	int status;

	status = hs_sha_run_digest (sha, data, length, 0, current, type, false, HS_SHA_UPDATE_FAILED,
		updated->AsBytes, SP_MSG_512_SIZE);
	if (!ROT_IS_ERROR (status)) {
		status = 0;
	}

	return status;
}

/**
 * Calculate the final digest for a message.  This can either complete a previously started digest
 * or calculate the digest for a complete message.
 *
 * @param sha The HS-SHA instance to use for the digest calculation.
 * @param data Buffer containing the data to hash.  If there is no additional data, this can be
 * null.  If this is null, length must be 0.
 * @param length Length of the data buffer.
 * @param total_length Total length of the message that was hashed.  This must include the length of
 * any previous data used to generate an intermediate digest.
 * @param current Input context for the current hash being executed.  Set to null to start a new
 * hash.
 * @param type The hash algorithm to use to generate the digest.
 * @param digest Output for the calculated digest.  This can be the same buffer as the current
 * context, but must not be null.
 * @param digest_length Length of the output buffer.
 *
 * @return Length of the calculated digest or an error code.  Use ROT_IS_ERROR to check the return
 * value.
 */
int hs_sha_finish_digest (const struct hs_sha *sha, const uint8_t *data, size_t length,
	size_t total_length, const SP_MSG_512 *current, enum hash_type type, uint8_t *digest,
	size_t digest_length)
{
	return hs_sha_run_digest (sha, data, length, total_length, current, type, true,
		HS_SHA_FINISH_FAILED, digest, digest_length);
}

/**
 * Mark the HS-SHA as being used by another HW block, such as CCS.  This prevents HS-SHA access from
 * FW while being used for other HW purposes.
 *
 * This will block until the HS-SHA is available to use.
 *
 * This must be followed by a call to hs_sha_mark_as_available for the HS-SHA to be used again by
 * FW.
 *
 * @param sha The HS-SHA that will be used by HW.
 */
void hs_sha_mark_as_in_use (const struct hs_sha *sha)
{
	if (sha) {
		platform_mutex_lock (&sha->state->lock);
	}
}

/**
 * Mark the HS-SHA as no longer being used by another HW block.
 *
 * @param sha The HS-SHA that is now available for use.
 */
void hs_sha_mark_as_available (const struct hs_sha *sha)
{
	if (sha) {
		platform_mutex_unlock (&sha->state->lock);
	}
}
