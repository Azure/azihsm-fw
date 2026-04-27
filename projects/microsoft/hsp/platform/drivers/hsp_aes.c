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
#include "drivers/hsp_aes.h"
#include "drivers/sram.h"
#include "splibs/inc/sptypes.h"


/**
 * Wait for the AES engine to be ready to accept a new command and erase the command buffer.
 *
 * @param aes The AES instance that will be executing the command.
 */
static void hsp_aes_start_new_command (const struct hsp_aes *aes)
{
	/* Make sure the AES HW is available for use. */
	platform_mutex_lock (&aes->state->lock);

	/* Spin here to make sure the AES HW is ready.  This is to clean up after possible FW errors
	 * with previous commands or other abnormal states.  Even when configured to use interrupts,
	 * this will still be a busy loop, for simplicity.  It should typically not get executed. */
	while (aes->regs->status & AES_REGS_STATUS_BUSY_FIELD_MASK) {
	}
	memset (aes->buffer, 0, sizeof (*aes->buffer));
}

/**
 * Convert the hardware status register value to an execution status code.
 *
 * @param status The raw status register value immediately upon command completion.
 * @param error_code Error to return when the the command failure bit is set.
 *
 * @return The error code to return for the command execution.
 */
static int hsp_aes_parse_command_status (uint32_t status, int error_code)
{
	if (status & AES_REGS_STATUS_COMPLETE_FIELD_MASK) {
		return 0;
	}
	else if (status & AES_REGS_STATUS_ERROR_CMD_FIELD_MASK) {
		/* For generic command failures, return a more specific error code. */
		return error_code;
	}
	else if (status == 0) {
		/* The command was never accepted by the HW. */
		return HSP_AES_CMD_NOT_STARTED;
	}
	else {
		/* For other errors, return the detailed error bits. */
		return HSP_AES_HW_ERROR (status);
	}
}

int hsp_aes_execute_command_polling (const struct hsp_aes *aes, int error_code)
{
	return crypto_hw_submit_command_polling (&aes->buffer->cmd, &aes->regs->cmd_addr,
		&aes->regs->status, AES_REGS_STATUS_BUSY_FIELD_MASK, hsp_aes_parse_command_status,
		error_code, HSP_AES_CMD_EXE_TIMEOUT);
}

bool hsp_aes_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param)
{
	const struct hsp_aes *aes = (const struct hsp_aes*) handler;

	UNUSED (param);

	return crypto_hw_handle_interrupt (aes->irq,
		CRYPTO_HW_IRQ_BIT_MASK (CRYPTO_DONE_INTSTS, AES_DONE), &aes->state->done);
}

int hsp_aes_execute_command_interrupt (const struct hsp_aes *aes, int error_code)
{
	return crypto_hw_submit_command_interrupt (aes->irq,
		CRYPTO_HW_IRQ_BIT_MASK (CRYPTO_DONE_INTSTS, AES_DONE), &aes->state->done, &aes->buffer->cmd,
		&aes->regs->cmd_addr, &aes->regs->status, AES_REGS_STATUS_BUSY_FIELD_MASK,
		hsp_aes_parse_command_status, error_code, HSP_AES_CMD_EXE_TIMEOUT);
}

/**
 * Perform AES encryption or decryption on a data buffer.
 *
 * @param aes The AES engine to use for the operation.
 * @param mode Cipher mode to use for operation.
 * @param encrypt True if the data should be encrypted, false to decrypt.
 * @param key KSU key slot that contains the encryption key.
 * @param iv Initial vector for the operation.
 * @param input Input buffer to the AES operation.
 * @param length Length of the input data.  This must be aligned to the AES block size (16 bytes).
 * @param output Output buffer for the AES operation.  It can be the same as the input buffer.
 * @param out_length Length of the output buffer.  It must be at least the same size as the input
 * buffer.
 * @param next_iv Optional output buffer for the IV that would be used to continue the operation
 * with additional data.
 *
 * @return 0 if the operation completed successfully or an error code.
 */
static int hsp_aes_operation (const struct hsp_aes *aes, enum hsp_aes_mode mode, bool encrypt,
	uint8_t key, const SP_MSG_128 *iv, const uint8_t *input, size_t length, uint8_t *output,
	size_t out_length, SP_MSG_128 *next_iv)
{
	uint8_t key_length;
	uint8_t unit_size;
	int status;

	if ((aes == NULL) || (input == NULL) || (length == 0) || (output == 0) ||
		((mode != HSP_AES_MODE_ECB) && (iv == NULL))) {
		return HSP_AES_INVALID_ARGUMENT;
	}

	if (out_length < length) {
		return HSP_AES_OUTPUT_BUFFER_TOO_SMALL;
	}

	switch (mode) {
		case HSP_AES_MODE_ECB:
		case HSP_AES_MODE_CBC:
			/* Only AES-256 is supported. */
			key_length = HSP_CMD_AES_KEY_SIZE_ENUM_KEY_256;
			unit_size = 0;
			break;

		case HSP_AES_MODE_XTS:
			/* While these would fail in other ways, since the max would typically exceed available
			 * memory, this needs to be explicitly checked and should return a different error
			 * code. */
			if ((length < 16) || (length > ((1U << 20) * 16))) {
				return HSP_AES_INVALID_XTS_DATA_LENGTH;
			}

			/* Only support encrypting a single data unit at a time.  Only specific data unit
			 * lengths are supported by the HW. */
			switch (length) {
				case 16:
					unit_size = HSP_CMD_AES_XTS_SIZE_ENUM_LEN_16;
					break;

				case 512:
					unit_size = HSP_CMD_AES_XTS_SIZE_ENUM_LEN_512;
					break;

				case 1024:
					unit_size = HSP_CMD_AES_XTS_SIZE_ENUM_LEN_1024;
					break;

				case 2048:
					unit_size = HSP_CMD_AES_XTS_SIZE_ENUM_LEN_2048;
					break;

				case 4096:
					unit_size = HSP_CMD_AES_XTS_SIZE_ENUM_LEN_4096;
					break;

				default:
					return HSP_AES_UNSUPPORTED_XTS_DATA_LENGTH;
			}

			/* XTS only supports AES-128. */
			key_length = HSP_CMD_AES_KEY_SIZE_ENUM_KEY_128;
			break;

		default:
			/* No other AES mode is currently supported by the driver as there is no current
			 * use-case defined for them. */
			return HSP_AES_UNSUPPORTED_AES_MODE;
	}

	/* Some modes are not strict about aligning to AES block sizes, but we don't currently support
	 * any of them in this driver. */
	if (HSP_AES_BLOCK_ALIGN (length) != length) {
		return HSP_AES_NOT_BLOCK_ALIGNED;
	}

	if (key >= aes->key_slots) {
		return HSP_AES_UNSUPPORTED_KEY_SLOT;
	}

	hsp_aes_start_new_command (aes);

	aes->buffer->cmd.command_code =
		HSP_CMD_AES_CMD_CODE_ID_SET (0x02) | HSP_CMD_AES_CMD_CODE_CIPHER_MODE_SET (mode) |
		HSP_CMD_AES_CMD_CODE_KEY_LEN_SET (key_length) |
		HSP_CMD_AES_CMD_CODE_XTS_SIZE_SET (unit_size);
	if (mode != HSP_AES_MODE_ECB) {
		aes->buffer->cmd.command_code |= HSP_CMD_AES_CMD_CODE_WIV_FIELD_MASK;
	}
	if (encrypt) {
		aes->buffer->cmd.command_code |= HSP_CMD_AES_CMD_CODE_ED_MODE_FIELD_MASK;
	}

	/* Put the IV in shared SRAM.  Always copy the data since that buffer will also be used for
	 * intermediate IV values for larger messages.  The intermediate IV is configured to always be
	 * written out, so it is available if needed. */
	if (mode != HSP_AES_MODE_ECB) {
		memcpy (aes->buffer->iv.AsBytes, iv->AsBytes, SP_MSG_128_SIZE);
		aes->buffer->cmd.init_vector = (uint32_t) &aes->buffer->iv;
	}

	do {
		size_t temp_len;

		/* Set up the input data, ensuring the data is in shared SRAM.  If the input buffer is
		 * already is shared SRAM, just access it directly.  Otherwise, copy the data to the temp
		 * buffer in shared SRAM, ensuring we do not overrun it. */
		if (sram_is_buffer_in_shared_sram (input, length)) {
			temp_len = length;
			aes->buffer->cmd.message = (uint32_t) input;
		}
		else {
			temp_len = min (length, aes->max_msg);
			memcpy (aes->msg_buffer, input, temp_len);
			aes->buffer->cmd.message = (uint32_t) aes->msg_buffer;
		}

		/* Set up the output data, ensuring the destination is shared SRAM. If the output buffer is
		 * already in shared SRAM, just access it directly.  Otherwise, use the temp buffer in
		 * shared SRAM for the output, and it will be copied out later.  Be sure we don't try to
		 * process more data that the temp buffer can hold. */
		if (sram_is_buffer_in_shared_sram (output, length)) {
			aes->buffer->cmd.result = (uint32_t) output;
		}
		else {
			/* We need to check the length here to cover the scenario where the input buffer is in
			 * shared SRAM. */
			temp_len = min (temp_len, aes->max_msg);
			aes->buffer->cmd.result = (uint32_t) aes->msg_buffer;
		}

		if ((mode == HSP_AES_MODE_XTS) && (temp_len != length)) {
			/* XTS needs to operate on the entire data unit in one pass.  If this can't be the case,
			 * the operation can't be performed. */
			status = HSP_AES_INPUT_BUFFER_TOO_LARGE;
			break;
		}

		aes->buffer->cmd.byte_count = temp_len;

		/* Specify the KSU address for the encryption key.
		 *
		 * TODO:  It would probably be better to get the address calculation out of this driver and
		 * use a call to CCS to determine this value. */
		aes->buffer->cmd.key = (uint32_t) &aes->keys[key];

		status = aes->execute_command (aes,
			(encrypt) ? HSP_AES_ENCRYPT_FAILED : HSP_AES_DECRYPT_FAILED);
		if (status == 0) {
			/* If the output was stored in the temp buffer, copy it to the user buffer. */
			if (aes->buffer->cmd.result == (uint32_t) aes->msg_buffer) {
				memcpy (output, aes->msg_buffer, temp_len);
			}

			input += temp_len;
			output += temp_len;
			length -= temp_len;
		}
	} while ((length > 0) && (status == 0));

	/* Save the last intermediate IV if the user has requested it. */
	if ((mode != HSP_AES_MODE_ECB) && next_iv && (status == 0)) {
		memcpy (next_iv->AsBytes, aes->buffer->iv.AsBytes, SP_MSG_128_SIZE);
	}

	/* Clean up after the operation. */
	buffer_zeroize (aes->buffer, sizeof (*aes->buffer));
	buffer_zeroize (aes->msg_buffer, aes->max_msg);

	platform_mutex_unlock (&aes->state->lock);

	return status;
}

int hsp_aes_encrypt (const struct hsp_aes *aes, enum hsp_aes_mode mode, uint8_t key,
	const SP_MSG_128 *iv, const uint8_t *plaintext, size_t length, uint8_t *ciphertext,
	size_t out_length, SP_MSG_128 *next_iv)
{
	return hsp_aes_operation (aes, mode, true, key, iv, plaintext, length, ciphertext, out_length,
		next_iv);
}

int hsp_aes_decrypt (const struct hsp_aes *aes, enum hsp_aes_mode mode, uint8_t key,
	const SP_MSG_128 *iv, const uint8_t *ciphertext, size_t length, uint8_t *plaintext,
	size_t out_length, SP_MSG_128 *next_iv)
{
	return hsp_aes_operation (aes, mode, false, key, iv, ciphertext, length, plaintext, out_length,
		next_iv);
}

/**
 * Initialize a driver instance for the HSP AES hardware engine.
 *
 * @param aes The driver to initialize.
 * @param state Variable context for the AES driver.  This must be uninitialized.
 * @param regs Base address for the AES registers.
 * @param cmd_buffer Location in HSP shared RAM where AES commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buffer Location in HSP shared RAM that can be used to stage data for AES operations,
 * when necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for AES (16 bytes).
 * @param keys Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 *
 * @return 0 if the AES driver was initialized successfully or an error code.
 */
static int hsp_aes_init (struct hsp_aes *aes, struct hsp_aes_state *state, struct Aes_regs *regs,
	struct hsp_aes_cmd_buffer *cmd_buffer, uint8_t *msg_buffer, size_t buffer_length,
	const struct ksu_key_slot *keys, size_t num_keys)
{
	if ((aes == NULL) || (state == NULL) || (regs == NULL) || (buffer_length < SP_MSG_128_SIZE) ||
		(keys == NULL)) {
		return HSP_AES_INVALID_ARGUMENT;
	}

	memset (aes, 0, sizeof (struct hsp_aes));

	aes->encrypt = hsp_aes_encrypt;
	aes->decrypt = hsp_aes_decrypt;

	aes->state = state;
	aes->regs = regs;
	aes->buffer = cmd_buffer;
	aes->msg_buffer = msg_buffer;
	aes->max_msg = HSP_AES_BLOCK_ALIGN (buffer_length);
	aes->keys = keys;
	aes->key_slots = num_keys;

	return hsp_aes_init_state (aes);
}

/**
 * Initialize a driver instance for the HSP AES hardware engine.  AES operations will enter a busy
 * waiting loop, actively polling the hardware to determine when they have finished.
 *
 * The interrupt handler will be null for instances initialized in this way.
 *
 * @param aes The driver to initialize.
 * @param state Variable context for the AES driver.  This must be uninitialized.
 * @param regs Base address for the AES registers.
 * @param cmd_buffer Location in HSP shared RAM where AES commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buffer Location in HSP shared RAM that can be used to stage data for AES operations,
 * when necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for AES (16 bytes).
 * @param keys Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 *
 * @return 0 if the AES driver was initialized successfully or an error code.
 */
int hsp_aes_init_polling (struct hsp_aes *aes, struct hsp_aes_state *state, struct Aes_regs *regs,
	struct hsp_aes_cmd_buffer *cmd_buffer, uint8_t *msg_buffer, size_t buffer_length,
	const struct ksu_key_slot *keys, size_t num_keys)
{
	int status;

	status = hsp_aes_init (aes, state, regs, cmd_buffer, msg_buffer, buffer_length, keys, num_keys);
	if (status == 0) {
		aes->execute_command = hsp_aes_execute_command_polling;
	}

	return status;
}

/**
 * Initialize a driver instance for the HSP AES hardware engine.  AES operations will block, waiting
 * for an interrupt to indicate when the hardware has finished.
 *
 * @param aes The driver to initialize.
 * @param state Variable context for the AES driver.  This must be uninitialized.
 * @param regs Base address for the AES registers.
 * @param cmd_buffer Location in HSP shared RAM where AES commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buffer Location in HSP shared RAM that can be used to stage data for AES operations,
 * when necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for AES (16 bytes).
 * @param keys Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 *
 * @return 0 if the AES driver was initialized successfully or an error code.
 */
int hsp_aes_init_interrupt (struct hsp_aes *aes, struct hsp_aes_state *state, struct Aes_regs *regs,
	struct Creg_regs_creg_crypto_group *irq_regs, struct hsp_aes_cmd_buffer *cmd_buffer,
	uint8_t *msg_buffer, size_t buffer_length, const struct ksu_key_slot *key_buffer,
	size_t num_keys)
{
	int status;

	if (irq_regs == NULL) {
		return HSP_AES_INVALID_ARGUMENT;
	}

	status = hsp_aes_init (aes, state, regs, cmd_buffer, msg_buffer, buffer_length, key_buffer,
		num_keys);
	if (status == 0) {
		aes->base.handle_interrupt = hsp_aes_handle_interrupt;
		aes->execute_command = hsp_aes_execute_command_interrupt;

		aes->irq = irq_regs;
	}

	return status;
}

/**
 * Initialize only the variable state for an AES driver.  The rest of the driver is assumed to have
 * already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param aes The AES driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int hsp_aes_init_state (const struct hsp_aes *aes)
{
	int status;

	if ((aes == NULL) || (aes->state == NULL) || (aes->regs == NULL) ||
		(aes->max_msg < SP_MSG_128_SIZE) || (aes->keys == NULL)) {
		return HSP_AES_INVALID_ARGUMENT;
	}

	if ((aes->base.handle_interrupt != NULL) && (aes->irq == NULL)) {
		return HSP_AES_INVALID_ARGUMENT;
	}

	if (!sram_is_buffer_in_shared_sram (aes->buffer, sizeof (*aes->buffer))) {
		return HSP_AES_INVALID_ADDRESS;
	}

	if ((uintptr_t) aes->buffer & 0x3) {
		return HSP_AES_ADDRESS_NOT_ALIGNED;
	}

	if (!sram_is_buffer_in_shared_sram (aes->msg_buffer, aes->max_msg)) {
		return HSP_AES_INVALID_ADDRESS;
	}

	memset (aes->state, 0, sizeof (struct hsp_aes_state));

	status = platform_semaphore_init (&aes->state->done);
	if (status != 0) {
		return status;
	}

	status = platform_mutex_init (&aes->state->lock);
	if (status != 0) {
		platform_semaphore_free (&aes->state->done);
	}

	return status;
}

/**
 * Release a HSP AES driver instance.
 *
 * @param aes The AES driver to release.
 */
void hsp_aes_release (const struct hsp_aes *aes)
{
	if (aes) {
		platform_mutex_free (&aes->state->lock);
		platform_semaphore_free (&aes->state->done);
	}
}

/**
 * Mark the AES as being used by another HW block, such as CCS.  This prevents AES access from FW
 * while being used for other HW purposes.
 *
 * This will block until the AES is available to use.
 *
 * This must be followed by a call to hsp_aes_mark_as_available for the AES to be used again by FW.
 *
 * @param aes The AES that will be used by HW.
 */
void hsp_aes_mark_as_in_use (const struct hsp_aes *aes)
{
	if (aes) {
		platform_mutex_lock (&aes->state->lock);
	}
}

/**
 * Mark the AES as no longer being used by another HW block.
 *
 * @param aes The AES that is now available for use.
 */
void hsp_aes_mark_as_available (const struct hsp_aes *aes)
{
	if (aes) {
		platform_mutex_unlock (&aes->state->lock);
	}
}

/**
 * Encryption key to use for the AES known-answer test.
 */
SECTION (".cryptotest.HSP_AES_KAT_KEY")
static const SP_MSG_256 HSP_AES_KAT_KEY = {
	.AsBytes = {
		0x04, 0x93, 0xff, 0x63, 0x71, 0x08, 0xaf, 0x6a,
		0x5b, 0x8e, 0x90, 0xac, 0x1f, 0xdf, 0x03, 0x5a,
		0x3d, 0x4b, 0xaf, 0xd1, 0xaf, 0xb5, 0x73, 0xbe,
		0x7a, 0xde, 0x9e, 0x86, 0x82, 0xe6, 0x63, 0xe5
	}
};


/**
 * IV for the AES known-answer test.
 */
SECTION (".cryptotest.HSP_AES_KAT_IV")
static const SP_MSG_128 HSP_AES_KAT_IV = {
	.AsBytes = {
		0xc0, 0xcd, 0x2b, 0xeb, 0xcc, 0xbb, 0x6c, 0x49,
		0x92, 0x0b, 0xd5, 0x48, 0x2a, 0xc7, 0x56, 0xe8
	}
};


/**
 * Plaintext data to use for the AES known-answer test.
 */
SECTION (".cryptotest.HSP_AES_KAT_PLAINTEXT")
static const uint8_t HSP_AES_KAT_PLAINTEXT[] = {
	0x8b, 0x37, 0xf9, 0x14, 0x8d, 0xf4, 0xbb, 0x25, 0x95, 0x6b, 0xe6, 0x31, 0x0c, 0x73, 0xc8, 0xdc,
	0x58, 0xea, 0x97, 0x14, 0xff, 0x49, 0xb6, 0x43, 0x10, 0x7b, 0x34, 0xc9, 0xbf, 0xf0, 0x96, 0xa9,
	0x4f, 0xed, 0xd6, 0x82, 0x35, 0x26, 0xab, 0xc2, 0x7a, 0x8e, 0x0b, 0x16, 0x61, 0x6e, 0xee, 0x25,
	0x4a, 0xb4, 0x56, 0x7d, 0xd6, 0x8e, 0x8c, 0xcd, 0x4c, 0x38, 0xac, 0x56, 0x3b, 0x13, 0x63, 0x9c
};


/**
 * Ciphertext data to use for the AES known-answer test.
 */
SECTION (".cryptotest.HSP_AES_KAT_CIPHERTEXT")
static const uint8_t HSP_AES_KAT_CIPHERTEXT[] = {
	0x05, 0xd5, 0xc7, 0x77, 0x29, 0x42, 0x1b, 0x08, 0xb7, 0x37, 0xe4, 0x11, 0x19, 0xfa, 0x44, 0x38,
	0xd1, 0xf5, 0x70, 0xcc, 0x77, 0x2a, 0x4d, 0x6c, 0x3d, 0xf7, 0xff, 0xed, 0xa0, 0x38, 0x4e, 0xf8,
	0x42, 0x88, 0xce, 0x37, 0xfc, 0x4c, 0x4c, 0x7d, 0x11, 0x25, 0xa4, 0x99, 0xb0, 0x51, 0x36, 0x4c,
	0x38, 0x9f, 0xd6, 0x39, 0xbd, 0xda, 0x64, 0x7d, 0xaa, 0x3b, 0xda, 0xda, 0xb2, 0xeb, 0x55, 0x94
};


/**
 * Test the AES hardware with AES-256-CBC operations to ensure that it is working properly.
 *
 * @param aes The AES hardware to test.
 * @param ccs Driver interface to the KSU where the AES key will be stored.
 * @param key_slot A key slot in the KSU that will be populated with the key for the test.  Any
 * existing key in this slot will be lost.
 *
 * @return 0 if the tests passed or an error code.
 */
int hsp_aes_run_self_test_aes256_cbc (const struct hsp_aes *aes,
	const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	uint8_t output[sizeof (HSP_AES_KAT_PLAINTEXT)];
	int status;

	if ((aes == NULL) || (ccs == NULL)) {
		return HSP_AES_INVALID_ARGUMENT;
	}

	status = ccs->set_key (ccs, (SP_MSG_384*) &HSP_AES_KAT_KEY, key_slot,
		CCS_KSU_ATTR_AES_ENCRYPT_ALLOWED | CCS_KSU_ATTR_AES_DECRYPT_ALLOWED);
	if (status != 0) {
		return status;
	}

	/* CBC KAT for AES encryption/decryption. */
	status = hsp_aes_encrypt (aes, HSP_AES_MODE_CBC, key_slot, &HSP_AES_KAT_IV,
		HSP_AES_KAT_PLAINTEXT, sizeof (HSP_AES_KAT_PLAINTEXT), output, sizeof (output), NULL);
	if (status != 0) {
		return status;
	}

	status = buffer_compare (HSP_AES_KAT_CIPHERTEXT, output, sizeof (HSP_AES_KAT_CIPHERTEXT));
	if (status != 0) {
		return HSP_AES_SELF_TEST_FAILED;
	}

	status = hsp_aes_decrypt (aes, HSP_AES_MODE_CBC, key_slot, &HSP_AES_KAT_IV,
		HSP_AES_KAT_CIPHERTEXT, sizeof (HSP_AES_KAT_CIPHERTEXT), output, sizeof (output), NULL);
	if (status != 0) {
		return status;
	}

	status = buffer_compare (HSP_AES_KAT_PLAINTEXT, output, sizeof (HSP_AES_KAT_PLAINTEXT));
	if (status != 0) {
		return HSP_AES_SELF_TEST_FAILED;
	}

	return 0;
}
