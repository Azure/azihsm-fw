// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "security_manager_hsp.h"
#include "common/buffer_util.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "crypto/hash.h"
#include "logging/code_path_integrity.h"
#include "system/device_unlock_token.h"
#include "system/system_logging.h"


/**
 * KDF context to use for mutating the DICE CDI and Device ID key when an unlock policy has been
 * applied.
 */
static const SP_MSG_384 SECURITY_MANAGER_HSP_DICE_KDF = {
	.AsBytes = {
		0x69, 0x21, 0x4b, 0xa5, 0x0f, 0x0d, 0x1c, 0xac,
		0x5a, 0x23, 0x46, 0x95, 0x2b, 0x0f, 0x3e, 0x5a,
		0xf0, 0x43, 0xc8, 0x1f, 0x58, 0x7a, 0x22, 0xc4,
		0x99, 0x6b, 0x8d, 0x65, 0x0b, 0xe0, 0x36, 0x59,
		0x6a, 0x4e, 0x99, 0x72, 0xc2, 0xa3, 0xa3, 0x64,
		0x8b, 0x7a, 0x97, 0x01, 0xab, 0xdb, 0x0c, 0x7c
	}
};

/**
 * KDF context to use for deriving the HMAC signing key for unlock policies on flash.
 */
static const SP_MSG_384 SECURITY_MANAGER_HSP_HMAC_KDF = {
	.AsBytes = {
		0x09, 0x62, 0x74, 0x55, 0x53, 0x4b, 0xf5, 0xce,
		0xb0, 0x5f, 0xfe, 0x95, 0xa4, 0x29, 0xcb, 0xf2,
		0xf4, 0x6d, 0x59, 0xf5, 0xd0, 0x7b, 0xb5, 0x75,
		0xbe, 0xac, 0x62, 0x42, 0x51, 0x60, 0x98, 0x0a,
		0x34, 0xc3, 0xdf, 0x87, 0x66, 0xbd, 0xab, 0x8e,
		0x75, 0x27, 0x60, 0x5c, 0x81, 0x91, 0x75, 0x19
	}
};


/**
 * The value to use for a specific checkpoint step.  This uses the security manager module ID to
 * provide uniqueness.
 */
#define	SECURITY_MANAGER_HSP_CHKPT_VALUE(x)		((ROT_MODULE_SECURITY_MANAGER << 8) | (x))

/**
 * Checkpoint values used when applying memory protections.
 */
enum {
	SECURITY_MANAGER_HSP_CHKPT_APPLY_CONFIG_START =
		SECURITY_MANAGER_HSP_CHKPT_VALUE (0x01),	/**< Start applying the security configuration for the current policy. */
	SECURITY_MANAGER_HSP_CHKPT_AEB_FUSES =
		SECURITY_MANAGER_HSP_CHKPT_VALUE (0x02),	/**< Updated fuses for permanently disabled AEBs. */
	SECURITY_MANAGER_HSP_CHKPT_GET_AEB_CONFIG =
		SECURITY_MANAGER_HSP_CHKPT_VALUE (0x03),	/**< Get the AEB settings report by the current policy. */
	SECURITY_MANAGER_HSP_CHKPT_APPLY_AEB_CONFIG =
		SECURITY_MANAGER_HSP_CHKPT_VALUE (0x04),	/**< Configured AEB settings per the current policy. */
	SECURITY_MANAGER_HSP_CHKPT_APPLY_FENCING =
		SECURITY_MANAGER_HSP_CHKPT_VALUE (0x05),	/**< Configured SoC fencing per the current current policy. */
};


/**
 * Check an unlock counter value to determine if it represents a locked or unlocked state.
 *
 * @param hsp The security manager context for checking the unlock counter.
 * @param unlock_counter The unlock counter value to check.
 *
 * @return false if the counter is locked, true if unlocked.
 */
bool security_manager_hsp_is_counter_unlocked (const struct security_manager_hsp *hsp,
	const uint8_t *unlock_counter)
{
	int bits;

	bits = common_math_get_num_contiguous_bits_set_in_array (unlock_counter, hsp->counter_length);

	/* An even bit count is locked.  An odd count is unlocked. */
	return (bits & 0x1);
}

int security_manager_hsp_lock_device (const struct security_manager *manager)
{
	const struct security_manager_hsp *hsp = (const struct security_manager_hsp*) manager;
	uint32_t counter_buffer[SECURITY_MANAGER_HSP_MAX_COUNTER_DWORDS];
	uint8_t *unlock_counter = (uint8_t*) counter_buffer;
	int status;

	if (hsp == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	if (hsp->unlock_nonce != NULL) {
		memset (hsp->unlock_nonce, 0, hsp->nonce_length);
	}

	/* The counter length is known to be valid based on checks done during init. */
	status = hsp->fuses->read_sw_fuses (hsp->fuses, hsp->counter_addr, unlock_counter,
		hsp->counter_length);
	if (status != 0) {
		return status;
	}

	if (security_manager_hsp_is_counter_unlocked (hsp, unlock_counter)) {
		common_math_set_next_bit_in_array_even_count (unlock_counter, hsp->counter_length);

		status = hsp->fuses->program_sw_fuses (hsp->fuses, hsp->counter_addr, counter_buffer,
			IN_DWORDS (hsp->counter_length));
		if (status != 0) {
			return status;
		}
	}

	status = hsp->flash->has_data_stored (hsp->flash, hsp->unlock_id);
	if (status == 1) {
		status = hsp->flash->erase (hsp->flash, hsp->unlock_id);
	}

	return status;
}

int security_manager_hsp_lock_device_unsupported (const struct security_manager *manager)
{
	UNUSED (manager);

	return SECURITY_MANAGER_UNSUPPORTED;
}

/**
 * Get the buffer to use for holding the unlock policy written to flash.  This needs to be large
 * enough to hold both the unlock policy and HMAC.
 *
 * @param hsp The HSP security manager to use for buffer allocation.
 * @param length Length of the buffer to allocate.
 * @param unlock_buffer Output for the buffer to hold the unlock data.
 *
 * @return 0 if the buffer was allocated successfully or an error code.
 */
static int security_manager_hsp_alloc_unlock_buffer (const struct security_manager_hsp *hsp,
	size_t length, uint8_t **unlock_buffer)
{
	if (hsp->unlock_buffer == NULL) {
		*unlock_buffer = platform_malloc (length);
		if (*unlock_buffer == NULL) {
			return SECURITY_MANAGER_NO_MEMORY;
		}
	}
	else {
		if (hsp->unlock_length < length) {
			return SECURITY_MANAGER_SMALL_POLICY_BUFFER;
		}

		*unlock_buffer = hsp->unlock_buffer;
	}

	return 0;
}

/**
 * Free the buffer used for holding the unlock policy.
 *
 * @param hsp The HSP security manager that allocated the buffer.
 * @param unlock_buffer The unlock buffer to free.
 */
static void security_manager_hsp_free_unlock_buffer (const struct security_manager_hsp *hsp,
	uint8_t *unlock_buffer)
{
	/* Only free the unlock buffer if it was dynamically allocated. */
	if (unlock_buffer != hsp->unlock_buffer) {
		platform_free (unlock_buffer);
	}
}

int security_manager_hsp_unlock_device (const struct security_manager *manager,
	const uint8_t *policy, size_t length)
{
	const struct security_manager_hsp *hsp = (const struct security_manager_hsp*) manager;
	SP_MSG_384 hmac;
	uint32_t key_attributes;
	size_t hmac_length;
	uint8_t *unlock_buffer;
	const uint8_t *policy_data;
	size_t data_length;
	int status;

	if ((hsp == NULL) || (policy == NULL)) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	/* HMAC the unlock data and write it to flash. */
	status = hsp->hash->calculate_sha384 (hsp->hash, policy, length, hmac.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	status = hsp->ccs->hmac (hsp->ccs, hsp->hmac_slot, hmac.AsBytes, SP_MSG_384_SIZE, &hmac,
		&key_attributes);
	if (status != 0) {
		return status;
	}

	if (key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		hmac_length = SHA384_HASH_LENGTH;
	}
	else {
		hmac_length = SHA256_HASH_LENGTH;
	}

	status = security_manager_hsp_alloc_unlock_buffer (hsp, length + hmac_length, &unlock_buffer);
	if (status != 0) {
		return status;
	}

	memcpy (unlock_buffer, policy, length);
	memcpy (&unlock_buffer[length], hmac.AsBytes, hmac_length);

	/* Update the device state to enable the stored unlock policy. */
	status = device_unlock_token_get_unlock_policy (policy, length, &policy_data, &data_length);
	if (status != 0) {
		goto exit;
	}

	status = hsp->policy->base.check_unlock_persistence (&hsp->policy->base, policy_data,
		data_length);
	switch (status) {
		case 0:
			/* One-time unlock caches the token nonce, if supported. */
			if (hsp->unlock_nonce != NULL) {
				SP_MSG_384 nonce_hmac;

				if (hsp->nonce_length < hmac_length) {
					status = SECURITY_MANAGER_SMALL_NONCE_BUFFER;
					goto exit;
				}

				status = device_unlock_token_get_nonce (policy, length, &policy_data, &data_length);
				if (status != 0) {
					goto exit;
				}

				/* Store an HMAC of the nonce to ensure one-time unlock policies can only be applied
				 * when the HMAC key is present in the KSU.  This prevents unlocked firmware from
				 * replaying one-time unlock policies by copying the nonce from that policy. */
				status = hsp->ccs->hmac (hsp->ccs, hsp->hmac_slot, policy_data, data_length,
					&nonce_hmac, NULL);
				if (status == 0) {
					memcpy (hsp->unlock_nonce, nonce_hmac.AsUINT32s, hmac_length);
				}
			}
			else {
				status = SECURITY_MANAGER_UNSUPPORTED;
			}

			break;

		case 1:
			/* Nothing more needs to be done for persistent unlock handling.  The policy has already
			 * been authenticated, so is assumed to be valid.  Fuses are not updated until
			 * application of the policy, so no device state needs to be changed at this time. */
			status = 0;
			break;

		default:
			break;
	}

	if (status == 0) {
		/* The policy has been consumed successfully by the device.  Commit the data to flash. */
		status = hsp->flash->write (hsp->flash, hsp->unlock_id, unlock_buffer,
			length + hmac_length);
		if ((status != 0) && (hsp->unlock_nonce != NULL)) {
			/* Always make sure the nonce is cleared on errors, if supported. */
			buffer_zeroize_dwords (hsp->unlock_nonce, IN_DWORDS (hsp->nonce_length));
		}
	}

exit:
	security_manager_hsp_free_unlock_buffer (hsp, unlock_buffer);

	return status;
}

int security_manager_hsp_unlock_device_unsupported (const struct security_manager *manager,
	const uint8_t *policy, size_t length)
{
	UNUSED (manager);
	UNUSED (policy);
	UNUSED (length);

	return SECURITY_MANAGER_UNSUPPORTED;
}

int security_manager_hsp_get_unlock_counter (const struct security_manager *manager,
	uint8_t *counter, size_t length)
{
	const struct security_manager_hsp *hsp = (const struct security_manager_hsp*) manager;
	int status;

	if ((hsp == NULL) || (counter == NULL)) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	if (length < hsp->counter_length) {
		return SECURITY_MANAGER_SMALL_COUNTER_BUFFER;
	}

	status = hsp->fuses->read_sw_fuses (hsp->fuses, hsp->counter_addr, counter,
		hsp->counter_length);
	if (status != 0) {
		return status;
	}

	return hsp->counter_length;
}

int security_manager_hsp_get_unlock_counter_unsupported (const struct security_manager *manager,
	uint8_t *counter, size_t length)
{
	UNUSED (manager);
	UNUSED (counter);
	UNUSED (length);

	return SECURITY_MANAGER_UNSUPPORTED;
}

int security_manager_hsp_has_unlock_policy (const struct security_manager *manager)
{
	const struct security_manager_hsp *hsp = (const struct security_manager_hsp*) manager;

	if (hsp == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	return hsp->state->is_unlocked;
}

int security_manager_hsp_has_unlock_policy_unsupported (const struct security_manager *manager)
{
	UNUSED (manager);

	return SECURITY_MANAGER_UNSUPPORTED;
}

/**
 * Read the current counter value from fuses.  Errors reading the fuses will be logged.
 *
 * @param hsp The HSP security manager to use for counter allocation.
 * @param counter_buffer Buffer that has been allocated for storing the unlock counter value.
 * @param unlock_counter Output for the current unlock counter value.  If the counter value could
 * not be read, this will be null.
 * @param is_unlocked Output for a flag indicating if the counter value represents an unlocked
 * state.  On error, this will indicate a locked state.
 */
void security_manager_hsp_get_current_unlock_counter (const struct security_manager_hsp *hsp,
	uint32_t counter_buffer[SECURITY_MANAGER_HSP_MAX_COUNTER_DWORDS], uint8_t **unlock_counter,
	bool *is_unlocked)
{
	int status;

	*unlock_counter = (uint8_t*) counter_buffer;
	*is_unlocked = false;

	/* Read the current value of the unlock counter in fuses to determine the unlock state of the
	 * device. */
	status = hsp->fuses->read_sw_fuses (hsp->fuses, hsp->counter_addr, *unlock_counter,
		hsp->counter_length);
	if (status == 0) {
		*is_unlocked = security_manager_hsp_is_counter_unlocked (hsp, *unlock_counter);
	}
	else {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_SYSTEM,
			SYSTEM_LOGGING_UNDETERMINED_UNLOCK, status, 0);

		*unlock_counter = NULL;
	}
}

/**
 * Determine the security policy (locked or unlocked) that should be used for the device and load
 * it.
 *
 * @param hsp The HSP security manager to use for security policy loading.
 * @param unlock_counter The current unlock counter for the device.  If this is null, the locked
 * security policy will be used.  If not null, this must be a 32-bit aligned buffer.
 * @param is_unlocked Flag indicating if the device is currently unlocked.
 *
 * @return 0 if the appropriate security policy was successfully loaded or an error code.
 */
int security_manager_hsp_determine_device_security_policy (const struct security_manager_hsp *hsp,
	uint8_t *unlock_counter, bool is_unlocked)
{
	int length;
	uint32_t key_attributes;
	SP_MSG_384 hmac;
	size_t hmac_length;
	uint8_t *unlock_buffer = NULL;
	const uint8_t *unlock_policy = NULL;
	size_t policy_length;
	uint32_t policy_type = 0;
	const uint8_t *token_check;
	size_t check_length;
	bool erase_policy = false;
	bool update_counter = true;
	int status;

	if (unlock_counter == NULL) {
		goto apply_policy;
	}

	if (!is_unlocked) {
		/* All counter operations need to be done against the unlocked value, so update the counter
		 * to the next value for an unlocked device. */
		common_math_set_next_bit_in_array_odd_count (unlock_counter, hsp->counter_length);
	}

	/* Check flash for any active unlock policy. */
	length = hsp->flash->get_data_length (hsp->flash, hsp->unlock_id);
	if (!ROT_IS_ERROR (length)) {
		status = security_manager_hsp_alloc_unlock_buffer (hsp, length, &unlock_buffer);
		if (status != 0) {
			goto policy_error;
		}

		length = hsp->flash->read (hsp->flash, hsp->unlock_id, unlock_buffer, length);
	}

	if (ROT_IS_ERROR (length)) {
		if (length != FLASH_STORE_NO_DATA) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_SYSTEM,
				SYSTEM_LOGGING_UNDETERMINED_UNLOCK, length, 0);

			if (length == FLASH_STORE_CORRUPT_DATA) {
				erase_policy = true;
			}
		}

		goto apply_policy;
	}

	/* Verify that the loaded data represents a valid unlock policy for this device by checking the
	 * HMAC that was added to the data.  The HMAC key attributes need to be inspected to determine
	 * the type of HMAC being performed. */
	status = hsp->ccs->get_key_attributes (hsp->ccs, hsp->hmac_slot, &key_attributes);
	if (status != 0) {
		goto policy_error;
	}

	if (key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		hmac_length = SHA384_HASH_LENGTH;
	}
	else {
		hmac_length = SHA256_HASH_LENGTH;
	}
	length -= hmac_length;

	status = hsp->hash->calculate_sha384 (hsp->hash, unlock_buffer, length, hmac.AsBytes,
		SP_MSG_384_SIZE);
	if (status != 0) {
		goto policy_error;
	}

	status = hsp->ccs->hmac (hsp->ccs, hsp->hmac_slot, hmac.AsBytes, SP_MSG_384_SIZE, &hmac, NULL);
	if (status != 0) {
		goto policy_error;
	}

	status = buffer_compare (&unlock_buffer[length], hmac.AsBytes, hmac_length);
	if (status != 0) {
		status = SECURITY_MANAGER_BAD_UNLOCK_POLICY;
		goto policy_error;
	}

	/* Extract the unlock policy data from the stored data and determine what type of unlock it
	 * represents to know how to further validate the unlock policy. */
	status = device_unlock_token_get_unlock_policy (unlock_buffer, length, &unlock_policy,
		&policy_length);
	if (status != 0) {
		goto policy_error;
	}

	status = hsp->policy->base.check_unlock_persistence (&hsp->policy->base, unlock_policy,
		policy_length);
	if (status == 1) {
		/* For persistent unlock policies, validate against the current unlock counter. */
		status = device_unlock_token_get_unlock_counter (unlock_buffer, length, &token_check,
			&check_length);
		if (status != 0) {
			goto policy_error;
		}

		if ((check_length != hsp->counter_length) ||
			(buffer_compare (token_check, unlock_counter, hsp->counter_length) != 0)) {
			status = SECURITY_MANAGER_COUNTER_MISMATCH;
			goto policy_error;
		}

		status = hsp->policy->base.parse_unlock_policy (&hsp->policy->base, unlock_policy,
			policy_length);
		if (status != 0) {
			goto policy_error;
		}

		if (!is_unlocked) {
			/* Update the counter in fuses to indicate that the device has been unlocked.  This is
			 * not necessary if the counter already is unlocked. */
			status = hsp->fuses->program_sw_fuses (hsp->fuses, hsp->counter_addr,
				(uint32_t*) unlock_counter, IN_DWORDS (hsp->counter_length));
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_SYSTEM,
					SYSTEM_LOGGING_LOCK_STATE_FAIL, status, 0);

				/* The fuses could not be updated.  Invalidate the unlock policy to avoid any
				 * persistent error condition. */
				unlock_policy = NULL;
				erase_policy = true;
				status = 0;
			}
		}

		/* The persistent unlock policy is valid.  Do not update the unlock counter. */
		update_counter = false;
		policy_type = SYSTEM_LOGGING_UNLOCK_PERSISTENT;
	}
	else if (status == 0) {
		/* For one-time unlock policies, validate against the stored nonce.  If there is no nonce
		 * buffer available to compare against, one-time unlock is not supported. */
		if (hsp->unlock_nonce != NULL) {
			SP_MSG_384 nonce_hmac;

			/* The nonce buffer length needs to be sufficient for the HMAC that would be generated
			 * for comparison. */
			if (hsp->nonce_length < hmac_length) {
				status = SECURITY_MANAGER_SMALL_NONCE_BUFFER;
				goto policy_error;
			}

			status = device_unlock_token_get_nonce (unlock_buffer, length, &token_check,
				&check_length);
			if (status != 0) {
				goto policy_error;
			}

			/* Generate an HMAC of the nonce for comparison to the expected value.  The length of
			 * the HMAC depends on the size of the HMAC key. */
			status = hsp->ccs->hmac (hsp->ccs, hsp->hmac_slot, token_check, check_length,
				&nonce_hmac, NULL);
			if (status != 0) {
				goto policy_error;
			}

			/* Nonce length is constant, so use that value rather than the one returned by the
			 * token parser. */
			status = buffer_compare_dwords (nonce_hmac.AsUINT32s, hsp->unlock_nonce,
				IN_DWORDS (hmac_length));
			if (status != 0) {
				status = SECURITY_MANAGER_NONCE_MISMATCH;
				goto policy_error;
			}

			status = hsp->policy->base.parse_unlock_policy (&hsp->policy->base, unlock_policy,
				policy_length);
			if (status != 0) {
				goto policy_error;
			}

			/* The one-time unlock policy is valid.  Erase it from flash. */
			erase_policy = true;
			policy_type = SYSTEM_LOGGING_UNLOCK_ONE_TIME;
		}
		else {
			status = SECURITY_MANAGER_UNSUPPORTED;
		}
	}

policy_error:
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_SYSTEM,
			SYSTEM_LOGGING_UNDETERMINED_UNLOCK, status, 0);

		unlock_policy = NULL;
		erase_policy = true;
	}

apply_policy:
	/* Always make sure the cached nonce buffer is cleared. */
	if (hsp->unlock_nonce != NULL) {
		buffer_zeroize_dwords (hsp->unlock_nonce, IN_DWORDS (hsp->nonce_length));
	}

	/* If necessary, erase the unlock policy data from flash. */
	if (erase_policy) {
		status = hsp->flash->erase (hsp->flash, hsp->unlock_id);
		if (status != 0) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_SYSTEM,
				SYSTEM_LOGGING_LOCK_STATE_FAIL, status, 0);
		}
	}

	/* If necessary, ensure the unlock counter to reflects a locked state. */
	if (update_counter && is_unlocked) {
		common_math_set_next_bit_in_array_even_count (unlock_counter, hsp->unlock_length);

		status = hsp->fuses->program_sw_fuses (hsp->fuses, hsp->counter_addr,
			(uint32_t*) unlock_counter, IN_DWORDS (hsp->counter_length));
		if (status != 0) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_SYSTEM,
				SYSTEM_LOGGING_LOCK_STATE_FAIL, status, 0);
		}
	}

	/* Finalize handling for the loaded unlock policy.  If there is no valid unlock policy, load the
	 * default policy for locked devices. */
	if (unlock_policy != NULL) {
		/* Clear the device keys to make sure they get modified for the unlocked execution. */
		status = security_manager_hsp_derive_unlocked_device_keys (hsp);
		if (status != 0) {
			goto exit;
		}

		hsp->state->is_unlocked = true;
		debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_SYSTEM,
			SYSTEM_LOGGING_DEVICE_UNLOCKED, policy_type, 0);
	}
	else {
		status = security_manager_hsp_load_locked_security_policy (hsp);
	}

exit:
	security_manager_hsp_free_unlock_buffer (hsp, unlock_buffer);

	return status;
}

/**
 * Load the security policy for a locked device.
 *
 * @param hsp The HSP security manager to load with the locked security policy.
 *
 * @return 0 if the security policy was loaded successfully or an error code.
 */
int security_manager_hsp_load_locked_security_policy (const struct security_manager_hsp *hsp)
{
	int status;

	status = hsp->policy->base.parse_unlock_policy (&hsp->policy->base, hsp->locked_data,
		hsp->locked_length);
	if (status == 0) {
		hsp->state->is_unlocked = false;
	}

	return status;
}

int security_manager_hsp_load_security_policy (const struct security_manager *manager)
{
	const struct security_manager_hsp *hsp = (const struct security_manager_hsp*) manager;
	uint32_t counter_buffer[SECURITY_MANAGER_HSP_MAX_COUNTER_DWORDS];
	uint8_t *unlock_counter;
	bool is_unlocked;
	int status;

	if (hsp == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	security_manager_hsp_get_current_unlock_counter (hsp, counter_buffer, &unlock_counter,
		&is_unlocked);

	status = security_manager_hsp_determine_device_security_policy (hsp, unlock_counter,
		is_unlocked);

	return status;
}

int security_manager_hsp_load_security_policy_unsupported (const struct security_manager *manager)
{
	UNUSED (manager);

	return SECURITY_MANAGER_UNSUPPORTED;
}

/**
 * Check the current value of the AEB fuses.  If the required disable bits are not set, program the
 * updated value into fuses.
 *
 * @param hsp The HSP security manager running the fuse check.
 *
 * @return 0 if the fuses have the expected disable bits set or an error code.
 */
static int security_manager_hsp_check_aeb_fuses (const struct security_manager_hsp *hsp)
{
	uint32_t aeb_disable;
	uint32_t fuse_disable;
	uint32_t fuse_value;
	uint8_t mask;
	int i;
	int status;

	status = hsp->policy->get_fuse_disabled_aebs (hsp->policy, &aeb_disable);
	if (status != 0) {
		return status;
	}

	/* 32 fused-backed AEBs map to 4 fuse words of data. */
	for (i = 0; i < 4; i++) {
		uint8_t aeb = (aeb_disable >> (8 * i)) & 0xff;
		uint16_t addr = hsp->aeb_addr + (4 * i);

		/* If no AEB is disabled, there is nothing to do. */
		if (aeb != 0) {
			/* Convert the AEB disable bit mask to a fuse word value.  Each AEB uses 4 fuse bits,
			 * with the disable bit in bit 1.  Set bit 1 of each nibble that corresponds to a bit
			 * set in the AEB disable list. */
			for (mask = 0x80, fuse_disable = 0; mask != 0; mask >>= 1) {
				fuse_disable <<= 4;

				if (aeb & mask) {
					fuse_disable |= 0x02;
				}
			}

			status = hsp->fuses->read_aeb_register (hsp->fuses, addr, &fuse_value);
			if (status != 0) {
				return status;
			}

			/* Mask out any other fuse bits that may be set and only check if the desired disabled
			 * bits are set. */
			if ((fuse_value & fuse_disable) != fuse_disable) {
				/* Add the required disable bits to any already set bits. */
				status = hsp->fuses->program_aeb_register (hsp->fuses, addr,
					fuse_disable | fuse_value);
				if (status != 0) {
					return status;
				}
			}
		}
	}

	return 0;
}

int security_manager_hsp_apply_device_config (const struct security_manager *manager)
{
	const struct security_manager_hsp *hsp = (const struct security_manager_hsp*) manager;
	uint32_t enable_aeb[HSP_AEB_MAX_GROUPS];
	uint32_t disable_aeb[HSP_AEB_MAX_GROUPS];
	uint32_t lock_aeb[HSP_AEB_MAX_GROUPS];
	int status;

	if (hsp == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	code_path_integrity_secure_message_no_trace (SECURITY_MANAGER_HSP_CHKPT_APPLY_CONFIG_START);

	/* Permanently disable any necessary AEBs through fuses.
	 *
	 * This step needs to be done first, before any AEBs that would block AEB fuse access are
	 * applied.  It's required of the caller to make sure that AEB fuses are made accessible.  This
	 * would generally require enabling the associated AEB for that fuse slot. */
	status = security_manager_hsp_check_aeb_fuses (hsp);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (SECURITY_MANAGER_HSP_CHKPT_AEB_FUSES ^ status);

	/* Apply the AEB configuration as specified by the current security policy. */
	status = hsp->policy->get_enabled_aebs (hsp->policy, enable_aeb, HSP_AEB_MAX_GROUPS);
	if (status != 0) {
		return status;
	}

	status = hsp->policy->get_disabled_aebs (hsp->policy, disable_aeb, HSP_AEB_MAX_GROUPS);
	if (status != 0) {
		return status;
	}

	status = hsp->policy->get_locked_aebs (hsp->policy, lock_aeb, HSP_AEB_MAX_GROUPS);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (SECURITY_MANAGER_HSP_CHKPT_GET_AEB_CONFIG ^
		status);

	status = hsp->aeb->configure_multiple_aeb (hsp->aeb, enable_aeb, disable_aeb, lock_aeb,
		HSP_AEB_MAX_GROUPS);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (SECURITY_MANAGER_HSP_CHKPT_APPLY_AEB_CONFIG ^
		status);

	/* Enable SoC memory fencing if the current policy requires it. */
	status = hsp->policy->enforce_memory_fencing (hsp->policy);
	if (status == 1) {
		status = hsp->mem_protect->configure_soc_fences (hsp->mem_protect);
	}

	code_path_integrity_secure_message_no_trace (SECURITY_MANAGER_HSP_CHKPT_APPLY_FENCING ^ status);

	return status;
}

int security_manager_hsp_apply_device_config_unsupported (const struct security_manager *manager)
{
	UNUSED (manager);

	return SECURITY_MANAGER_UNSUPPORTED;
}

int security_manager_hsp_get_security_policy (const struct security_manager *manager,
	const struct security_policy **policy)
{
	const struct security_manager_hsp *hsp = (const struct security_manager_hsp*) manager;

	if ((manager == NULL) || (policy == NULL)) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	*policy = &hsp->policy->base;

	return 0;
}

/**
 * Initialize a manager for the security configuration of HSP-based devices.
 *
 * @param manager The security manager to initialize.
 * @param state Variable context for the security manager.  This must be uninitialized.
 * @param policy The security policy handler for the device.
 * @param locked_policy The policy data that should loaded by the security policy when the device is
 * locked.  This must be in a format compatible with a call to security_policy.parse_unlock_policy.
 * @param policy_length Length of the locked policy data.
 * @param aeb Driver to configure HSP AEBs.
 * @param fuses Interface to the HSP fuses.
 * @param aeb_addr Fuse address for the AEB fuses.  It's assumed there are 4 fuse words to
 * accommodate 32 fuse-backed AEBs.  This address must be 32-bit aligned.
 * @param counter_addr Fuse address for the unlock anti-replay counter.  This address must be 32-bit
 * aligned.
 * @param counter_length Length of the unlock counter.  This must be 32-bit aligned.
 * @param mem_protect Handler for configuring hardware memory protections.
 * @param hash Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param cdi_slot KSU key slot that contains the DICE CDI.
 * @param devid_slot KSU key slot that contains the DICE Device ID.
 * @param hmac_slot KSU key slot that contains the HMAC key used to sign unlock policies stored on
 * flash.
 * @param hmac_buffer A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length Length of the unlock policy HMAC buffer.  This must be large enough to hold
 * the entire authorized unlock data.
 * @param flash Flash storage used for device unlock policies.
 * @param unlock_id Block ID in flash storage that will be used for storing unlock policies.
 * @param unlock_nonce Buffer that contains the nonce for one-time unlock policies.  This memory
 * location must not be altered during device resets and must be 32-bit aligned.  This can be null
 * if one-time unlock policies are not supported.
 * @param nonce_length Length of the buffer for the one-time unlock nonce.  This must be at least
 * large enough to hold an HMAC of the HSP unlock token nonce.  The length of the HMAC depends on
 * the length of the HMAC key when the one-time unlock policy has been received.
 *
 * @return 0 if the manager was initialized successfully or an error code.
 */
int security_manager_hsp_init (struct security_manager_hsp *manager,
	struct security_manager_hsp_state *state, const struct security_policy_hsp *policy,
	const uint8_t *locked_policy, size_t policy_length, const struct hsp_aeb *aeb,
	const struct fuse_controller_interface *fuses, uint16_t aeb_addr, uint16_t counter_addr,
	size_t counter_length, const struct memory_protection *mem_protect,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t cdi_slot,
	uint8_t devid_slot, uint8_t hmac_slot, uint8_t *hmac_buffer, size_t buffer_length,
	const struct flash_store *flash, int unlock_id, uint32_t *unlock_nonce, size_t nonce_length)
{
	if (manager == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	memset (manager, 0, sizeof (struct security_manager_hsp));

	manager->base.lock_device = security_manager_hsp_lock_device;
	manager->base.unlock_device = security_manager_hsp_unlock_device;
	manager->base.get_unlock_counter = security_manager_hsp_get_unlock_counter;
	manager->base.has_unlock_policy = security_manager_hsp_has_unlock_policy;
	manager->base.load_security_policy = security_manager_hsp_load_security_policy;
	manager->base.apply_device_config = security_manager_hsp_apply_device_config;

	manager->base.internal.get_security_policy = security_manager_hsp_get_security_policy;

	manager->state = state;
	manager->policy = policy;
	manager->locked_data = locked_policy;
	manager->locked_length = policy_length;
	manager->aeb = aeb;
	manager->fuses = fuses;
	manager->mem_protect = mem_protect;
	manager->hash = hash;
	manager->ccs = ccs;
	manager->flash = flash;
	manager->aeb_addr = aeb_addr;
	manager->counter_addr = counter_addr;
	manager->counter_length = counter_length;
	manager->cdi_slot = cdi_slot;
	manager->devid_slot = devid_slot;
	manager->hmac_slot = hmac_slot;
	manager->unlock_buffer = hmac_buffer;
	manager->unlock_length = buffer_length;
	manager->unlock_id = unlock_id;
	manager->unlock_nonce = unlock_nonce;
	manager->nonce_length = nonce_length;
	manager->support_policy_load = true;

	return security_manager_hsp_init_state (manager);
}

/**
 * Initialize a manager for the security configuration of HSP-based devices.  Only APIs that are
 * used to lock or unlock the device are supported.  Applying the unlocked policy is not supported.
 * This type of manager would typically be used by run-time firmware.
 *
 * These API calls are supported:
 * - lock_device
 * - unlock_device
 * - get_unlock_counter
 *
 * @param manager The security manager to initialize.
 * @param state Variable context for the security manager.  This must be uninitialized.
 * @param policy The security policy handler for the device.
 * @param fuses Interface to the HSP fuses.
 * @param counter_addr Fuse address for the unlock anti-replay counter.  This address must be 32-bit
 * aligned.
 * @param counter_length Length of the unlock counter.  This must be 32-bit aligned.
 * @param hash Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param hmac_slot KSU key slot that contains the HMAC key used to sign unlock policies stored on
 * flash.
 * @param hmac_buffer A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length Length of the unlock policy HMAC buffer.  This must be large enough to hold
 * the entire authorized unlock data.
 * @param flash Flash storage used for device unlock policies.
 * @param unlock_id Block ID in flash storage that will be used for storing unlock policies.
 * @param unlock_nonce Buffer that contains the nonce for one-time unlock policies.  This memory
 * location must not be altered during device resets and must be 32-bit aligned.  This can be null
 * if one-time unlock policies are not supported.
 * @param nonce_length Length of the buffer for the one-time unlock nonce.  This must be at least
 * large enough to hold an HMAC of the HSP unlock token nonce.  The length of the HMAC depends on
 * the length of the HMAC key when the one-time unlock policy has been received.
 *
 * @return 0 if the manager was initialized successfully or an error code.
 */
int security_manager_hsp_init_only_config_unlock (struct security_manager_hsp *manager,
	struct security_manager_hsp_state *state, const struct security_policy_hsp *policy,
	const struct fuse_controller_interface *fuses, uint16_t counter_addr, size_t counter_length,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t hmac_slot,
	uint8_t *hmac_buffer, size_t buffer_length, const struct flash_store *flash, int unlock_id,
	uint32_t *unlock_nonce, size_t nonce_length)
{
	if (manager == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	memset (manager, 0, sizeof (struct security_manager_hsp));

	manager->base.lock_device = security_manager_hsp_lock_device;
	manager->base.unlock_device = security_manager_hsp_unlock_device;
	manager->base.get_unlock_counter = security_manager_hsp_get_unlock_counter;
	manager->base.has_unlock_policy = security_manager_hsp_has_unlock_policy_unsupported;
	manager->base.load_security_policy = security_manager_hsp_load_security_policy_unsupported;
	manager->base.apply_device_config = security_manager_hsp_apply_device_config_unsupported;

	manager->base.internal.get_security_policy = security_manager_hsp_get_security_policy;

	manager->state = state;
	manager->policy = policy;
	manager->fuses = fuses;
	manager->hash = hash;
	manager->ccs = ccs;
	manager->flash = flash;
	manager->counter_addr = counter_addr;
	manager->counter_length = counter_length;
	manager->hmac_slot = hmac_slot;
	manager->unlock_buffer = hmac_buffer;
	manager->unlock_length = buffer_length;
	manager->unlock_id = unlock_id;
	manager->unlock_nonce = unlock_nonce;
	manager->nonce_length = nonce_length;

	return security_manager_hsp_init_state (manager);
}

/**
 * Initialize a manager for the security configuration of HSP-based devices.  Only APIs that are
 * used to load and apply an unlock policy are supported.  Locking or unlocking the device is not
 * supported.  This type of manager would typically be used at boot-time (1SP).
 *
 * These API calls are supported:
 * - has_unlock_policy
 * - load_security_policy
 * - apply_device_config
 *
 * @param manager The security manager to initialize.
 * @param state Variable context for the security manager.  This must be uninitialized.
 * @param policy The security policy handler for the device.
 * @param locked_policy The policy data that should loaded by the security policy when the device is
 * locked.  This must be in a format compatible with a call to security_policy.parse_unlock_policy.
 * @param policy_length Length of the locked policy data.
 * @param aeb Driver to configure HSP AEBs.
 * @param fuses Interface to the HSP fuses.
 * @param aeb_addr Fuse address for the AEB fuses.  It's assumed there are 4 fuse words to
 * accommodate 32 fuse-backed AEBs.  This address must be 32-bit aligned.
 * @param counter_addr Fuse address for the unlock anti-replay counter.  This address must be 32-bit
 * aligned.
 * @param counter_length Length of the unlock counter.  This must be 32-bit aligned.
 * @param mem_protect Handler for configuring hardware memory protections.
 * @param hash Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param cdi_slot KSU key slot that contains the DICE CDI.
 * @param devid_slot KSU key slot that contains the DICE Device ID.
 * @param hmac_slot KSU key slot that contains the HMAC key used to sign unlock policies stored on
 * flash.
 * @param hmac_buffer A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length Length of the unlock policy HMAC buffer.  This must be large enough to hold
 * the entire authorized unlock data.
 * @param flash Flash storage used for device unlock policies.
 * @param unlock_id Block ID in flash storage that will be used for storing unlock policies.
 * @param unlock_nonce Buffer that contains the nonce for one-time unlock policies.  This memory
 * location must not be altered during device resets and must be 32-bit aligned.  This can be null
 * if one-time unlock policies are not supported.
 * @param nonce_length Length of the buffer for the one-time unlock nonce.  This must be at least
 * large enough to hold an HMAC of the HSP unlock token nonce.  The length of the HMAC depends on
 * the length of the HMAC key when the one-time unlock policy has been received.
 *
 * @return 0 if the manager was initialized successfully or an error code.
 */
int security_manager_hsp_init_only_apply_unlock (struct security_manager_hsp *manager,
	struct security_manager_hsp_state *state, const struct security_policy_hsp *policy,
	const uint8_t *locked_policy, size_t policy_length, const struct hsp_aeb *aeb,
	const struct fuse_controller_interface *fuses, uint16_t aeb_addr, uint16_t counter_addr,
	size_t counter_length, const struct memory_protection *mem_protect,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t cdi_slot,
	uint8_t devid_slot, uint8_t hmac_slot, uint8_t *hmac_buffer, size_t buffer_length,
	const struct flash_store *flash, int unlock_id, uint32_t *unlock_nonce, size_t nonce_length)
{
	if (manager == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	memset (manager, 0, sizeof (struct security_manager_hsp));

	manager->base.lock_device = security_manager_hsp_lock_device_unsupported;
	manager->base.unlock_device = security_manager_hsp_unlock_device_unsupported;
	manager->base.get_unlock_counter = security_manager_hsp_get_unlock_counter_unsupported;
	manager->base.has_unlock_policy = security_manager_hsp_has_unlock_policy;
	manager->base.load_security_policy = security_manager_hsp_load_security_policy;
	manager->base.apply_device_config = security_manager_hsp_apply_device_config;

	manager->base.internal.get_security_policy = security_manager_hsp_get_security_policy;

	manager->state = state;
	manager->policy = policy;
	manager->locked_data = locked_policy;
	manager->locked_length = policy_length;
	manager->aeb = aeb;
	manager->fuses = fuses;
	manager->mem_protect = mem_protect;
	manager->hash = hash;
	manager->ccs = ccs;
	manager->flash = flash;
	manager->aeb_addr = aeb_addr;
	manager->counter_addr = counter_addr;
	manager->counter_length = counter_length;
	manager->cdi_slot = cdi_slot;
	manager->devid_slot = devid_slot;
	manager->hmac_slot = hmac_slot;
	manager->unlock_buffer = hmac_buffer;
	manager->unlock_length = buffer_length;
	manager->unlock_id = unlock_id;
	manager->unlock_nonce = unlock_nonce;
	manager->nonce_length = nonce_length;
	manager->support_policy_load = true;

	return security_manager_hsp_init_state (manager);
}

/**
 * Initialize only the variable state for an HSP security manager.  The rest of the manager
 * structure is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param manager The security manager that contains the state to initialize.
 *
 * @return 0 if the state was initialized successfully or an error code.
 */
int security_manager_hsp_init_state (const struct security_manager_hsp *manager)
{
	if ((manager == NULL) || (manager->state == NULL) || (manager->policy == NULL) ||
		(manager->fuses == NULL) || (manager->hash == NULL) || (manager->ccs == NULL) ||
		(manager->flash == NULL)) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	/* If loading unlock policies is supported, additional parameters are required. */
	if (manager->support_policy_load &&
		((manager->locked_data == NULL) || (manager->locked_length == 0) ||
		(manager->aeb == NULL) || (manager->mem_protect == NULL))) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	/* Ensure 32-bit alignment for fuse addresses and buffers for fuse data. */
	if (((manager->aeb_addr & 0x3) != 0) || ((manager->counter_addr & 0x3) != 0)) {
		return SECURITY_MANAGER_UNALIGNED_ADDRESS;
	}

	if ((manager->counter_length & 0x3) != 0) {
		return SECURITY_MANAGER_UNALIGNED_BUFFER;
	}

	/* Temporary counter buffers are allocated on the stack so must be no larger than the supported
	 * maximum length. */
	if (manager->counter_length > SECURITY_MANAGER_HSP_MAX_COUNTER_LENGTH) {
		return SECURITY_MANAGER_SMALL_COUNTER_BUFFER;
	}

	memset (manager->state, 0, sizeof (struct security_manager_hsp_state));

	return 0;
}

/**
 * Release the resources used by an HSP security manager.
 *
 * @param manager The security manager to release.
 */
void security_manager_hsp_release (const struct security_manager_hsp *manager)
{
	UNUSED (manager);
}

/**
 * Initialize the HMAC CCS key used to sign and verify device unlock policies stored on flash.  This
 * must only be called once after a device reset.  This key must be generated before attempting to
 * load or store any unlock policy on flash.
 *
 * @param manager The security manager that will derive the unlock key.
 * @param src_key_slot KSU key slot for the source key that will be used to derive the unlock HMAC
 * key.
 *
 * @return 0 if the HMAC key was initialized successfully or an error code.
 */
int security_manager_hsp_derive_hmac_key (const struct security_manager_hsp *manager,
	uint8_t src_key_slot)
{
	if (manager == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	/* If the security manager does not support loading unlock policies, do not allow it to derive
	 * the HMAC key. */
	if (!manager->support_policy_load) {
		return SECURITY_MANAGER_UNSUPPORTED;
	}

	/* Always set IsDeviceSecret and KeySize384 when deriving the key.  If the source key doesn't
	 * have these set, the HW will automatically clear them. */
	return manager->ccs->derive_key (manager->ccs, src_key_slot, &SECURITY_MANAGER_HSP_HMAC_KDF,
		manager->hmac_slot, CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED |
		CCS_KSU_ATTR_KEY_SIZE_384);
}

/**
 * Update DICE keys for an unlocked device and wipe the unlock HMAC key.
 *
 * This will be called as part of loading an unlock policy, so it's not required to be called
 * externally.  However, it's available to force this workflow in scenarios where there is no unlock
 * policy, but trusted keys should still not be used.
 *
 * @param manager The security manager to use for updating the device keys.
 *
 * @return 0 if all keys were updated successfully or an error code.
 */
int security_manager_hsp_derive_unlocked_device_keys (const struct security_manager_hsp *manager)
{
	uint32_t key_attributes;
	SP_MSG_384 zero;
	int status;

	if (manager == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	/* If the security manager does not support loading unlock policies, it doesn't have access to
	 * the DICE keys, so this operation cannot be executed. */
	if (!manager->support_policy_load) {
		return SECURITY_MANAGER_UNSUPPORTED;
	}

	/* Change the DICE keys so an unlocked device will have a different identity.  This will not
	 * be endorsed by DME. */
	status = manager->ccs->get_key_attributes (manager->ccs, manager->cdi_slot, &key_attributes);
	if (status != 0) {
		return status;
	}

	status = manager->ccs->derive_key (manager->ccs, manager->cdi_slot,
		&SECURITY_MANAGER_HSP_DICE_KDF, manager->cdi_slot, key_attributes);
	if (status != 0) {
		return status;
	}

	status = manager->ccs->get_key_attributes (manager->ccs, manager->devid_slot, &key_attributes);
	if (status != 0) {
		return status;
	}

	status = manager->ccs->derive_ecc_key (manager->ccs, manager->cdi_slot, manager->devid_slot,
		key_attributes);
	if (status != 0) {
		return status;
	}

	/* Clear the HMAC key used to sign and verify unlock policies.  Only locked devices can
	 * apply an unlock policy. */
	memset (zero.AsBytes, 0, SP_MSG_384_SIZE);

	return manager->ccs->set_key (manager->ccs, &zero, manager->hmac_slot, 0);
}
