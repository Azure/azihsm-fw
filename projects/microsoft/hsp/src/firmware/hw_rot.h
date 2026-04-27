// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HW_ROT_H_
#define HW_ROT_H_

#include <stdbool.h>
#include <stdint.h>
#include "crypto/hash.h"
#include "status/msft_module_id.h"


/**
 * Interface to the information stored in hardware or securely chained from hardware that is used to
 * securely load a firmware image.
 */
struct hw_rot {
	/**
	 * Checks to see if there is a trusted root key within the RoT.
	 *
	 * @param rot The RoT to query.
	 *
	 * @return 0 if there is a trusted root key or an error code.  If there is no trusted root key,
	 * HW_ROT_NO_ROOT_KEY will be returned.  If it is not possible to have a trusted root key,
	 * HW_ROT_UNSUPPORTED will be returned.
	 */
	int (*has_root_key) (const struct hw_rot *rot);

	/**
	 * Get a hash based on the current root key of the device.
	 *
	 * This hash should not be used for root key verification purposes since this is not guaranteed
	 * to be exactly the hash of the raw root key data in all scenarios, depending on how the key
	 * is stored in hardware.
	 *
	 * @param rot The RoT to query.
	 * @param hash A hash engine to use if additional hashing is required.
	 * @param type The hash algorithm to use when generating the hash.
	 * @param digest Output for the digest calculated against the root key information in hardware.
	 * @param length Length of the digest output buffer.
	 *
	 * @return 0 if the hash was generated successfully or an error code.  If there is no trusted
	 * root key, HW_ROT_NO_ROOT_KEY will be returned.  If it is not possible to have a trusted root
	 * key, HW_ROT_UNSUPPORTED will be returned.
	 */
	int (*get_root_key_hash) (const struct hw_rot *rot, const struct hash_engine *hash,
		enum hash_type type, uint8_t *digest, size_t length);

	/**
	 * Verify that the provided root key is trusted based on the current state of the device.
	 *
	 * @param rot The RoT to check the root key against.
	 * @param root_key The raw key data that should be checked against the RoT state.
	 * @param length Length of the root key data.
	 * @param hash A hash engine that can be used to hash the root key, if necessary, for comparison
	 * to the RoT state.
	 *
	 * @return 0 if the root key is trusted by the device or an error code.
	 *    - If the root key is not valid, HW_ROT_BAD_ROOT_KEY will be returned.
	 *    - If no trusted root key has been configured, HW_ROT_NO_ROOT_KEY will be returned.
	 *    - If it is not possible to have a root key, HW_ROT_UNSUPPORTED will be returned.
	 */
	int (*verify_root_key) (const struct hw_rot *rot, const uint8_t *root_key, size_t length,
		const struct hash_engine *hash);

	/**
	 * Checks to see if there are root key slots that are still available to use.  As long as there
	 * are root key slots available, it is possible to change the root key used for image
	 * validation.
	 *
	 * @param rot The RoT to query.
	 *
	 * @return 0 if there are root key slots available or an error code.  If there are no more root
	 * key slots, HW_ROT_ROOT_KEYS_EXHAUSTED will be returned.
	 */
	int (*has_free_root_key_slots) (const struct hw_rot *rot);

	/**
	 * Change the root key used for image validation.  This process will invalidate the current
	 * root key.
	 *
	 * @param rot the RoT of to update.
	 * @param root_key The raw key data that should be used to update the RoT state.
	 * @param length Length of the root key data.
	 * @param hash A hash engine that can be used to hash the root key, if necessary, before
	 * updating the RoT state.
	 *
	 * @return 0 if the root key was updated successfully or an error code.  If there is no storage
	 * available for the new root key, HW_ROT_ROOT_KEYS_EXHAUSTED will be returned.
	 */
	int (*update_root_key) (const struct hw_rot *rot, const uint8_t *root_key, size_t length,
		const struct hash_engine *hash);

	/**
	 * Get the Security Version Number for the firmware image being loaded.  This value is used to
	 * enforce anti-rollback of the image.
	 *
	 * @param rot The RoT to query.
	 * @param svn Output for the current SVN value accepted by the device.
	 *
	 * @return 0 if the SVN was retrieved successfully or an error code.
	 */
	int (*get_svn) (const struct hw_rot *rot, uint64_t *svn);

	/**
	 * Determine the number of SVN bit that are supported by the RoT.
	 *
	 * @param The RoT to query.
	 *
	 * @return The number of supported SVN bits.
	 */
	int (*get_svn_bits) (const struct hw_rot *rot);

	/**
	 * Update the Security Version Number for the firmware image.  Once this is updated, any image
	 * with a lower SVN will no longer be valid.
	 *
	 * @param rot The RoT to update.
	 * @param svn The SVN value to set.
	 *
	 * @return 0 if the SVN was updated successfully or an error code.
	 */
	int (*update_svn) (const struct hw_rot *rot, uint64_t svn);

	/**
	 * Ensure that all internal copies of the Security Version Number match the expected value.  If
	 * any are wrong, update them to the expected value.
	 *
	 * @param rot The RoT to update.
	 * @param svn The expected SVN value.
	 *
	 * @return 0 if all copies of the SVN are correct or an error code.
	 */
	int (*refresh_svn) (const struct hw_rot *rot, uint64_t svn);

	/**
	 * Check to see if there is an active tenancy for the firmware image.
	 *
	 * @param rot The RoT to query.
	 *
	 * @return 0 if there is no active tenancy, 1 if there is an active tenancy, or an error code.
	 */
	int (*has_active_tenancy) (const struct hw_rot *rot);

	/**
	 * Get the current value of the tenancy counter.
	 *
	 * @param rot The RoT to query.
	 * @param counter Output buffer for the tenancy counter value.
	 * @param length Length of the tenancy counter buffer.
	 *
	 * @return Length of the tenancy counter or an error code.  If not enough buffer was provided to
	 * hold the full tenancy counter, HW_ROT_TENANCY_COUNTER_TOO_LONG will be returned.
	 */
	int (*get_tenancy_counter) (const struct hw_rot *rot, uint8_t *counter, size_t length);

	/**
	 * Get the current tenancy grant token for the device.
	 *
	 * @param rot The RoT to query.
	 * @param tenant_key Public key that will be used by the tenant for firmware signing.
	 * @param key_length Length of the tenant public key.
	 * @param token Output buffer for the tenancy grant token.
	 * @param length Length of the token buffer.
	 *
	 * @return Length of the grant token or an error code.  Use ROT_IS_ERROR to check the return
	 * value.
	 */
	int (*get_tenancy_grant_token) (const struct hw_rot *rot, const uint8_t *tenant_key,
		size_t key_length, uint8_t *token, size_t length);

	/**
	 * Grant or revoke a tenancy for the firmware image.  When a tenancy transfer is executed, the
	 * device identity keys change to reflect the change in device state.  Only keys valid for the
	 * current tenancy state will be used to validate the firmware image.
	 *
	 * Attempting a transfer while already in the desired state will result in a successful
	 * transfer, but no state will change.
	 *
	 * @param rot the RoT to update.
	 * @param grant True to activate a tenancy on the device or false to revoke a tenancy.
	 *
	 * @return 0 if the transfer was completed successfully or an error code.
	 */
	int (*tenancy_transfer) (const struct hw_rot *rot, bool grant);
};


#define	HW_ROT_ERROR(code)		ROT_ERROR (MSFT_MODULE_HW_ROT, code)

/**
 * Error codes that can be generated by RoT state.
 */
enum {
	HW_ROT_INVALID_ARGUMENT = HW_ROT_ERROR (0x00),			/**< Input parameter is null or not valid. */
	HW_ROT_NO_MEMORY = HW_ROT_ERROR (0x01),					/**< Memory allocation failed. */
	HW_ROT_CHECK_ROOT_FAILED = HW_ROT_ERROR (0x02),			/**< Failed to check for a trusted root key. */
	HW_ROT_GET_ROOT_HASH_FAILED = HW_ROT_ERROR (0x03),		/**< Failed to get a hash of the root key. */
	HW_ROT_VERIFY_ROOT_FAILED = HW_ROT_ERROR (0x04),		/**< The root key could not be verified. */
	HW_ROT_CHECK_FREE_ROOT_FAILED = HW_ROT_ERROR (0x05),	/**< Failed to check for free root key slots. */
	HW_ROT_UPDATE_ROOT_FAILED = HW_ROT_ERROR (0x06),		/**< Failed to change the root key. */
	HW_ROT_GET_SVN_FAILED = HW_ROT_ERROR (0x07),			/**< Failed to get the current SVN value. */
	HW_ROT_UPDATE_SVN_FAILED = HW_ROT_ERROR (0x08),			/**< Failed to update the SVN value. */
	HW_ROT_REFRESH_SVN_FAILED = HW_ROT_ERROR (0x09),		/**< Failed to refresh the current SVN value. */
	HW_ROT_CHECK_TENANCY_FAILED = HW_ROT_ERROR (0x0a),		/**< Failed to check for an active tenancy. */
	HW_ROT_TENANCY_COUNTER_FAILED = HW_ROT_ERROR (0x0b),	/**< Failed to retrieve the current tenancy counter. */
	HW_ROT_GRANT_TOKEN_FAILED = HW_ROT_ERROR (0x0c),		/**< Failed to get the current tenancy grant token. */
	HW_ROT_TENANCY_TRANSFER_FAILED = HW_ROT_ERROR (0x0d),	/**< Failed to grant or revoke a tenancy transfer. */
	HW_ROT_BAD_ROOT_KEY = HW_ROT_ERROR (0x0e),				/**< The root key is not valid. */
	HW_ROT_ROOT_KEYS_EXHAUSTED = HW_ROT_ERROR (0x0f),		/**< All root key slots have been used. */
	HW_ROT_TENANCY_COUNTER_TOO_LONG = HW_ROT_ERROR (0x10),	/**< The tenancy counter is larger than the provided buffer. */
	HW_ROT_NOT_INIT = HW_ROT_ERROR (0x11),					/**< RoT has not been initialized. */
	HW_ROT_UNSUPPORTED = HW_ROT_ERROR (0x12),				/**< RoT does not support the operation in the current state. */
	HW_ROT_NO_ROOT_KEY = HW_ROT_ERROR (0x13),				/**< The RoT does not current have a trusted root key. */
	HW_ROT_TENANCY_EXHAUSTED = HW_ROT_ERROR (0x14),			/**< No more tenancy transfers can be supported by the device. */
	HW_ROT_ROOT_KEY_LOAD_FAILURE = HW_ROT_ERROR (0x15),		/**< The current root key was not loaded successfully from fuses. */
	HW_ROT_GRANT_TOKEN_TOO_LONG = HW_ROT_ERROR (0x16),		/**< The tenancy grant token is larger than the provided buffer. */
	HW_ROT_ADDRESS_NOT_SUPPORTED = HW_ROT_ERROR (0x17),		/**< A address pointer provided is not valid for the context. */
	HW_ROT_UNSUPPORTED_TENANT_KEY = HW_ROT_ERROR (0x18),	/**< The provided tenant key is not supported. */
	HW_ROT_ROOT_HASH_TOO_LONG = HW_ROT_ERROR (0x19),		/**< The root key hash requested is longer than the provided buffer. */
};


#endif	/* HW_ROT_H_ */
